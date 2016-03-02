/**************************************************************************
**     Copyright (c) 2005-2010, Widevine Technologies
**
**     WVPlaybackAPI test application
***************************************************************************/

#include "WVPlaybackAPI.h"
#include "WVStreamControlAPI.h"

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>


#ifdef BCMCARD_7630
#include <bdvd.h>
#include <signal.h>
#include "vdvd_types.h"
#include "WVPlayback_BCMCARD.h"
#include "pe_app.h"

extern PE_HANDLE gPE;
static PE_HANDLE localPE;
extern int init_pe(void);

#endif

#if defined(BCMCARD_3549) || defined(BCMCARD_35230) || defined(BCMCARD_7425)
extern "C" {

bool BcmNexus_Platform_Init();
};

#include "nexus_platform.h"
#include "drm_widevine.h"
#endif

#define MAX_URLS 10
#if defined(PANASONIC_FUNAITV)
#define FILE_NAME "http://edge.cinemanow.com.edgesuite.net/adaptive/studio/yes_man_c338331d_wv_SD_Trailer.vob"
#endif
#define DRM_SERVER_URL "https://license.uat.widevine.com/getlicense/widevine_test"
#define DRM_ACK_SERVER_URL ""

#define DRM_HEARTBEAT_URL "https://license.uat.widevine.com/classsic/heartbeat"
#define DRM_HEARTBEAT_PERIOD 120

#define DRM_CLIENT_IP ""
#define DRM_DEVICE_ID ""
#define DRM_STREAM_ID ""
#define DRM_USER_DATA ""

#define DEFAULT_PORTAL ""
#define DEFAULT_STOREFRONT ""

#if defined(BCMCARD_35230)
// workarond for msising symbol in BRCM libs
char buildversion[64];
#endif

bool     endOfFile    = false;

void usage() {
    printf("\n");
    printf(" app_wvplayback is a test application to exercize\n");
    printf(" the WVPlaybackAPI interface.\n");
    printf("\n");
    printf(" Usage:\n");
    printf(" app_wvplayback -url <url> [options...] [-url <url2> [options...]]\n");
    printf(" -url <url>  up to %d URLs may be specified\n", MAX_URLS);
    printf("\n      [drm_server_url=<val>]  Widevine DRM server URL");
    printf("\n      [drm_client_ip=<val>]  Widevine DRM IP address of client");
    printf("\n      [drm_device_id=<val>]  Widevine DRM device ID");
    printf("\n      [drm_stream_id=<val>]  Widevine DRM stream ID");
    printf("\n      [drm_user_data=<val>]  Widevine DRMuser data");
    printf("\n      [drm_ack_server_url=<val>]  Widevine DRM ack server URL");
    printf("\n      [drm_heartbeat_url=<val>]  Widevine DRM heartbeat URL");
#if defined(BCMCARD_3549) || defined(BCMCARD_35230) || defined(BCMCARD_7425)
    printf("\n      [drm_key_bin=<val>]  Path to Widevine DRM .bin key file");
#endif
    printf("\n      [proxy_server_ip=x.x.x.x|hostname]  Proxy server IP address");
    printf("\n      [proxy_server_port=<val>]  Proxy server IP port");
    printf("\n      [proxy_server_userid=string]  Proxy server user ID if authentication required");
    printf("\n      [proxy_server_password=string] Proxy server password if authentication required");
    printf("\n");
};


void runUsage() {
    printf("\n");
    printf(" n play next (or restart first) file\n");
    printf(" p pause/play toggle\n");
    printf(" f fast forward: doubles current rate\n");
    printf(" r rewind: doubles current rate\n");
    printf(" s seek to 00:01:00\n");
#if defined(MEMORYCHUNK_MANAGED_HEAP_MEMORY) && (MEMORYCHUNK_MANAGED_HEAP_MEMORY == PIL_MANAGED_HEAP)
    printf(" m dump PilManagedHeap stats\n");
#endif
    printf(" x terminate playback\n");
    printf(" ? print this list\n");
};

void QualityCallback(size_t numLevels, size_t currentLevel, bool isHD, void *userData)
{
    printf("CALLBACK: quality: %d levels, current=%d, isHD=%s, userData=%p\n", numLevels, currentLevel, (isHD ? "yes" : "no"), userData);
}

static bool bandwidthCheckDone = false;

void BandwidthCallback(unsigned long bytesPerSecond, void *userData)
{
    printf("CALLBACK: bandwidth: %ld bytesPerSecond, userData=%p\n", bytesPerSecond, userData);
    bandwidthCheckDone = true;
}

void EventCallback(WVPlaybackAPI::WVEventType eventType, const std::string &time, void *userData)
{
    const char *event = "unknown";
    switch(eventType) {
        case WVPlaybackAPI::WV_Event_Playing: event = "WV_Event_Playing"; break;
        case WVPlaybackAPI::WV_Event_FastForward: event = "WV_Event_FastForward"; break;
        case WVPlaybackAPI::WV_Event_Rewind: event = "WV_Event_Rewind"; break;
        case WVPlaybackAPI::WV_Event_Paused: event = "WV_Event_Paused"; break;
        case WVPlaybackAPI::WV_Event_Stopped: event = "WV_Event_Stopped"; break;
        case WVPlaybackAPI::WV_Event_Seek: event = "WV_Event_Seek"; break;
        case WVPlaybackAPI::WV_Event_Underflow: event = "WV_Event_Underflow"; break;
        case WVPlaybackAPI::WV_Event_EndOfStream: event = "WV_Event_EndOfStream"; endOfFile = true; break;
    }
    printf("CALLBACK: event: type=%s, time=%s, userData=%p\n", event, time.c_str(), userData);
}

void ErrorCallback(WVStatus errorCode, void *userData)
{
    printf("CALLBACK: error: code=%d, userData=%p\n", errorCode, userData);
}


void TimeCallback(const std::string &time, void *userData)
{
    printf("CALLBACK: time: %s, userData=%p\n", time.c_str(), userData);
}

struct filearg {
    char name[256];
    char drm_server_url[256];
    char drm_client_ip[100];
    char drm_device_id[128];
    char drm_stream_id[256];
    char drm_user_data[100];
    char drm_ack_server_url[256];
    char drm_heartbeat_url[256];
    char portal[256];
    char storefront[256];
};

static char proxy_server_userid[32] = {0};
static char proxy_server_password[32] = {0};
static char proxy_server_ip[32] = {0};
static int proxy_server_port;
static char drm_key_bin_path[256] = {0};

int main(int argc, char **argv)
{
    int  activeFile        = 0;
    int  fileCnt           = 0;
    filearg file[MAX_URLS] = {};

    if ( argc <= 2 )
    {
        usage();
        return(EXIT_FAILURE);
    }

    for (size_t i = 0; i < MAX_URLS; i++)
    {
	strcpy(file[i].drm_server_url, DRM_SERVER_URL);
	strcpy(file[i].drm_ack_server_url, DRM_ACK_SERVER_URL);
	strcpy(file[i].drm_client_ip, DRM_CLIENT_IP);
	strcpy(file[i].drm_device_id, DRM_DEVICE_ID);
	strcpy(file[i].drm_stream_id, DRM_STREAM_ID);
	strcpy(file[i].drm_user_data, DRM_USER_DATA);
	strcpy(file[i].drm_heartbeat_url, DRM_HEARTBEAT_URL);
	strcpy(file[i].portal, DEFAULT_PORTAL);
	strcpy(file[i].storefront, DEFAULT_STOREFRONT);
    }

    for (int i = 1; i < argc; i++)
    {
        if (!strncmp(argv[i], "-url", 4))
        {
            if( fileCnt < MAX_URLS - 1 )
            {
                fileCnt++;
                strcpy(file[fileCnt - 1].name, argv[++i]);
            }
            else
            {
                printf("***********************\n");
                printf("* Max URL count is %d *\n", MAX_URLS);
                printf("***********************\n");
            }
        }
	else if (!strncmp(argv[i], "portal=", 7))
	{
	    sscanf(argv[i], "portal=%s", file[fileCnt - 1].portal);
	}
	else if (!strncmp(argv[i], "storefront=", 11))
	{
	    sscanf(argv[i], "storefront=%s", file[fileCnt - 1].storefront);
	}
	else if (!strncmp(argv[i], "drm_server_url=", 15))
	{
	    sscanf(argv[i], "drm_server_url=%s", file[fileCnt - 1].drm_server_url);
	}
	else if (!strncmp(argv[i], "drm_client_ip=", 14))
	{
	    sscanf(argv[i], "drm_client_ip=%s", file[fileCnt - 1].drm_client_ip);
	}
	else if (!strncmp(argv[i], "drm_device_id=", 14))
	{
	    sscanf(argv[i], "drm_device_id=%s", file[fileCnt - 1].drm_device_id);
	}
	else if (!strncmp(argv[i], "drm_stream_id=", 14))
	{
	    sscanf(argv[i], "drm_stream_id=%s", file[fileCnt - 1].drm_stream_id);
	}
	else if (!strncmp(argv[i], "drm_user_data=", 14))
	{
	    sscanf(argv[i], "drm_user_data=%s", file[fileCnt - 1].drm_user_data);
	}
	else if (!strncmp(argv[i], "drm_ack_server_url=", 19))
	{
	    sscanf(argv[i], "drm_ack_server_url=%s", file[fileCnt - 1].drm_ack_server_url);
	}
	else if (!strncmp(argv[i], "drm_heartbeat_url=", 18))
	{
	    sscanf(argv[i], "drm_heartbeat_url=%s", file[fileCnt - 1].drm_heartbeat_url);
	}
	else if (!strncmp(argv[i], "drm_key_bin=", 12))
	{
	    sscanf(argv[i], "drm_key_bin=%s", drm_key_bin_path);
	}
	else if (!strncmp(argv[i], "proxy_server_ip=", 16))
	{
	    sscanf(argv[i], "proxy_server_ip=%s", proxy_server_ip);
	}
	else if (!strncmp(argv[i], "proxy_server_port=", 18))
	{
	    sscanf(argv[i], "proxy_server_port=%d", &proxy_server_port);
	}
	else if (!strncmp(argv[i], "proxy_server_userid=", 20))
	{
	    sscanf(argv[i], "proxy_server_userid=%s", proxy_server_userid);
	}
	else if (!strncmp(argv[i], "proxy_server_password=", 22))
	{
	    sscanf(argv[i], "proxy_server_password=%s", proxy_server_password);
	}
        else if ( !strcmp(argv[i], "-?") || !strcmp(argv[i],  "?") ||
                  !strcmp(argv[i], "-h") || !strcmp(argv[i], "-H")  )
        {
            usage();
            return(EXIT_FAILURE);
        }
        else
        {
            printf("Error: Invalid Arguement >%s< \n", argv[i]);
            return(EXIT_FAILURE);
        }
    }

    for(int i=0; i<fileCnt; i++)
    {
        printf("........File[%d] is >%s<\n", i, file[i].name);
    }

#ifdef BCMCARD_7630
    if( init_pe() < 0 )
    {
         printf("........initPE Failed!!!!!\n");
         exit(-1);
    }
#endif

    bool     endOfFile    = false;
    bool     jump = false;
    int      flags   = fcntl(STDIN_FILENO, F_GETFL, 0);
    char     cmd;
    int      readStatus;

    WVCredentials credentials;

    credentials.deviceID = file[activeFile].drm_device_id;
    credentials.streamID = file[activeFile].drm_stream_id;
    credentials.clientIP = file[activeFile].drm_client_ip;
    credentials.drmServerURL = file[activeFile].drm_server_url;
    credentials.userData = file[activeFile].drm_user_data;
    credentials.drmAckServerURL = file[activeFile].drm_ack_server_url;
    credentials.heartbeatURL = file[activeFile].drm_heartbeat_url;
    credentials.heartbeatPeriod  = DRM_HEARTBEAT_PERIOD;
	credentials.portal = file[activeFile].portal;

printf("\n came till here 1");
#if defined(BCMCARD_3549)
    Initialize_Widevine(drm_key_bin_path);
#endif

    WVPlaybackAPI *pAPI = WVPlaybackAPI::Instance();
    pAPI->ConfigureCredentials(credentials);


    WVProxySettings proxySettings;
    if (proxy_server_ip[0]) {
        proxySettings.enable = true;
	proxySettings.ipAddr = proxy_server_ip;
	proxySettings.ipPort = (unsigned short)proxy_server_port;
	proxySettings.userId = proxy_server_userid;
	proxySettings.password = proxy_server_password;
    } else {
        proxySettings.enable = false;
    }

    pAPI->ConfigureProxy(proxySettings);

#ifdef RUN_BANDWIDTH_CHECK
    pAPI->ConfigureBandwidthCheck("http://edge.cinemanow.com.edgesuite.net/adaptive/studio/yes_man_c338331d_wv_SD_Trailer.vob", 10);
#endif
    pAPI->ConfigureQualityCallback(QualityCallback, (void *)0x12341001);
    pAPI->ConfigureBandwidthCallback(BandwidthCallback, (void *)0x12341002);
    pAPI->ConfigureEventCallback(EventCallback, (void *)0x12341003);
    pAPI->ConfigureErrorCallback(ErrorCallback, 10, (void *)0x12341004);
    pAPI->ConfigureTimeCallback(TimeCallback, "npt", (void *)0x12341005);

#if (defined(BCMCARD_3549)  || defined(BCMCARD_35230) || defined(BCMCARD_7425)) && defined(BROADCOM_HARDWARE_CRYPTO)
    BcmNexus_Platform_Init();

  #if defined(BCMCARD_3549)
    Initialize_Widevine(drm_key_bin_path);
  #elif defined(BCMCARD_35230) || defined(BCMCARD_7425)
    //DRM_Widevine_Initialize(drm_key_bin_path);
	 printf("\n calling DRM_Widevine_Initialize");
	 DRM_Widevine_Init_t wvSetup = {drm_key_bin_path, DRM_WidevineDecryptMode_eVod};
		DRM_Widevine_Initialize(wvSetup);

  #endif
#endif
#ifdef BCMCARD_7630
    bdvd_init(BDVD_INIT_BASE_MODULES);
#endif
printf("\n calling pAPI->init()...............");
    pAPI->Initialize();
	printf("\n *****************************pAPI->init() done********************");
	printf("\n *****************************pAPI->init() done********************");
	printf("\n *****************************pAPI->init() done********************");



#ifdef RUN_BANDWIDTH_CHECK
    while (!bandwidthCheckDone)
        usleep(100);
#endif

    while( activeFile < fileCnt )
    {
	WVSession *session;
printf("\n *****************************pAPI->open() ********************");

	WVStatus status = pAPI->OpenURL(session, file[activeFile].name);
	printf("\n *****************************pAPI->open() done********************");

	if (status != WV_Status_OK) {
	    printf("OpenURL status: %d\n", status);
	    exit(-1);
	} else {
	    printf("Opened URL %s\n", file[activeFile].name);
	    runUsage();
	}

        endOfFile  = false;
        jump       = false;

        float speed = 0.0;

        while( endOfFile == false )
        {
            // check for command
            cmd = 0;
            fcntl(STDIN_FILENO, F_SETFL, flags|O_NONBLOCK);
            readStatus = read(STDIN_FILENO, &cmd, 1);
            fcntl(STDIN_FILENO, F_SETFL, flags);
            if(readStatus > 0 )
            {
                switch(cmd)
                {
                    case 'n':
                    {
                        jump = true;
                        endOfFile = true;
                        printf("next file %d\n", (activeFile + 1));
                        break;
                    }
                    case 'p':
                    {
			float speedUsed;
			// play
			if (speed == 0)
			  speed = 1;  // resume normal play from pause
			else
			  speed = 0;  // pause from play or trick mode
			printf("play speed %f\n", speed);
			if (speed == 0) {
			    status = pAPI->Pause(session);
			} else {
			    status = pAPI->Play(session, speed, &speedUsed);
			}
			if (status != WV_Status_OK)
			    printf("Play status=%d\n", status);
			else
			    printf("Actual speed: %f\n", speedUsed);
                        break;
                    }
                    case 'f':
                    {
			float speedUsed;

			// fast forward
			if (speed < 1)
			    speed = 2;
			else if (speed < 64)
			    speed = 2 * speed;

			printf("play speed %f\n", speed);
			status = pAPI->Play(session, speed, &speedUsed);
			if (status != WV_Status_OK)
			    printf("Play status=%d\n", status);
			else
			    printf("Actual speed: %f\n", speedUsed);
                        break;
                    }
                    case 'r':
                    {
			float speedUsed;

			// rewind
			if (speed > -1)
			    speed = -2.0;
			else if (speed > -64)
			    speed = 2 * speed;

			printf("play speed %f\n", speed);
			status = pAPI->Play(session, speed, &speedUsed);
			if (status != WV_Status_OK)
			    printf("Play status=%d\n", status);
			else
			    printf("Actual speed: %f\n", speedUsed);
                        break;
                    }
                    case 's':
                    {
			float speedUsed;
			status = pAPI->Seek(session, "00:01:00.4"); // change this to prompt for seek time
			printf("seek status=%d\n", (int)status);
			printf("play speed %f\n", speed);
			status = pAPI->Play(session, speed, &speedUsed);
			if (status != WV_Status_OK)
			    printf("Play status=%d\n", status);
			else
			    printf("Actual speed: %f\n", speedUsed);
                        break;
                    }
                    case 'x':
                    {
                        printf(".........terminating\n");
                        endOfFile = true;
                        activeFile = 100;
                        break;
                    }

                    case '?':
                    {
                        runUsage();
                        break;
                    }
                    default:
                        break;
                }
            }
            else
              usleep(100000);
        }   // while( endOfFile == false )

        printf("Closing device\n");
        status = pAPI->Close(session);

        activeFile++;
	if( jump && activeFile >= fileCnt )
	    activeFile = 0;
    }

#ifdef BCMCARD_7630
    bdvd_uninit();
#endif
    return(EXIT_SUCCESS);
}

#ifdef BCMCARD_7630
int init_pe(void)
{
    int ret = 0;
    //bdvd_dfb_ext_layer_settings_t dfb_ext_settings;

    /* Create the PE */
    fprintf( stdout, "%s(%d): Creating the PE\n", __FUNCTION__, __LINE__);
    PECreate(&localPE);
    if (localPE == NULL)
    {
        fprintf( stderr, "%s(%d): Could not create the PE!\n", __FUNCTION__, __LINE__);
        return(-1);
    }

    /* Create the PE iConfigure interface */
    if (PECreateiConfigure(localPE) != PE_SUCCESS)
    {
        fprintf( stderr, "%s(%d): Could not create the PE iConfigure interface!\n", __FUNCTION__, __LINE__);
        return(-1);
    }

    /* Create PE iStreamCtrl */
    fprintf( stdout, "%s(%d): Creating the PE iStreamCtrl interface\n", __FUNCTION__, __LINE__);
    if (PECreateFakeiStreamCtrl(localPE) != PE_SUCCESS)
    {
        fprintf( stderr, "%s(%d): Could not create the PE iStreamCtrl interface!\n", __FUNCTION__, __LINE__);
        return(-1);
    }

    fprintf( stderr, "%s:%d Calling....PEiConfigureSetVideoPlaneVisible\n\t", __FILE__, __LINE__ );
    PEiConfigureSetVideoPlaneVisible(localPE, TRUE, TRUE);

    /* Configure the PE video output */
    fprintf( stdout, "%s(%d): Configuring the PE video output\n", __FUNCTION__, __LINE__);
    if (PE_SUCCESS != PEiConfigureSetVideoOutput(localPE, VIDEO_FORMAT_1080I))
    {
        fprintf( stderr, "%s(%d): Could not configure the PE video output\n", __FUNCTION__, __LINE__);
        return(-1);
    }

    gPE = localPE;

    return(0);
}
#endif
