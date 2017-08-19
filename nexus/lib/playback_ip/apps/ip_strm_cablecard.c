/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <termios.h>

#include "bstd.h"
#include "bkni.h"

#include "mpod.h"
#include "mpod_util.h"
#include "mpod_apinfo.h"
#include "mpod_diag.h"
#include "mpod_feature.h"
#include "mpod_dsg.h"
#include "mpod_ext_chan.h"
#include "mpod_mmi.h"
#include "mpod_link.h"
#include "mpod_download.h"
#include "mpod_feature.h"
#include "mpod_homing.h"
#include "mpod_hc.h"
#include "mpod_sas.h"
#include "mpod_diag.h"
#include "mpod_mmi.h"
#include "mpod_systime.h"
#include "mpod_ca.h"
#include "mpod_cp.h"
#include "mpod_res.h"
#include "mpod_snmp.h"
#include "test_cp.h"
#include "mpod_resrc_mgr.h"
#include "mpod_headend_comm.h"
#include "mpod_host_property.h"

#if NEXUS_HAS_SECURITY
#include "nexus_security.h"
#endif

#ifdef CDL_SUPPORT
#include "b_cdl_lib.h"
#include "cdl_adpu.h"
#include "cdl_dsg.h"
#endif

#ifdef ESTB_CFG_SUPPORT
#include "b_estb_cfg_lib.h"
#endif

#include "b_playback_ip_lib.h"
#include "ip_streamer_lib.h"
#include "ip_streamer.h"
#include "ip_strm_cablecard.h"
#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_frontend.h"
#include "nexus_recpump.h"
#include "nexus_playpump.h"
#include "ts_psi.h"
#include "ts_pmt.h"

#include <signal.h>
#include <termios.h>
struct termios OriginalTerminalIO;

#define MAX_TUNER           (NEXUS_MAX_FRONTENDS-1)
#define CABLECARD_DEVICE        "/dev/pod"
#define MAX_MPEG_SECTION_LEN    4096

#ifdef LINUX
static pthread_mutex_t g_cablecard_mutex = PTHREAD_MUTEX_INITIALIZER;
#define cablecard_lock() pthread_mutex_lock(&g_cablecard_mutex)
#define cablecard_unlock() pthread_mutex_unlock(&g_cablecard_mutex)
#else
error "Not Supported"
#endif

static Cablecard g_cablecard;
static B_MPOD_SAS_HANDLE SasConnections[32]; /* max 32 connections allowed by the Host 2.0 spec */
static uint8_t SasData[8] = {0xde,0xad, 0xbe, 0xef, 0x01, 0x23, 0x45, 0x67};
static cablecard_program_info pg_info[MAX_CABLECARD_ROUTE];
#if NEXUS_HAS_SECURITY
static NEXUS_KeySlotHandle keyslot[MAX_CABLECARD_ROUTE];
#endif

/* generic CabelCARD information */
static CableCARDInfo info;
extern int gExitThread;
#if 0
static pthread_t gCablecardMenuThreadId;
#endif
static NEXUS_PlaypumpHandle switchPlaypump[2];
static NEXUS_RecpumpHandle switchRecpump[2];
static uint32_t switchMap[2] = {NEXUS_ParserBand_eMax,NEXUS_ParserBand_eMax};
static NEXUS_PidChannelHandle videoPidChannels[2];
static NEXUS_PidChannelHandle audioPidChannels[2];
static NEXUS_PidChannelHandle pcrPidChannels[2];
static NEXUS_PidChannelHandle patPidChannels[2];
static NEXUS_PidChannelHandle pmtPidChannels[2]; 
static NEXUS_PidChannelHandle caPidChannels[2];
static NEXUS_MpodInputHandle mpodPlaypumpInput[2];
static NEXUS_FrontendHandle oobTuner = 0;

BDBG_MODULE(ip_strm_cablecard);

/* external call to DSGCC lib*/
int InitializeDsgCcApplication(void);
 void CableCardCallbackSetDSGMode( unsigned char * data, unsigned short len );
 void CableCardCallbackDSG_Directory( unsigned char *data, unsigned short len );
 void CableCardCallbackDSGPacketError( unsigned char err_status );
 void CableCardCallbackConfig_Advanced_DSG( unsigned char *pkt_obj_ptr, unsigned short len );
 void CableCardCallbackDSGFlowID( unsigned long flow_id );
 unsigned long CableCardCallbackIPUDhcp( unsigned long flowid, unsigned char *mac,
        unsigned char opt_len, unsigned char *opt_data, unsigned char *ret_option, int *ropt_len );
 unsigned char CableCardCallbackSocketFlowConfig(unsigned long id, unsigned char *opt_data,
                                                        unsigned long opt_len);
 unsigned char CableCardCallbackDeleteFlowReq(unsigned long flowid);
 void CableCardCallbackSendIPUData( unsigned char *pdata, unsigned long len );
 void CableCardCallbackSendSocketFlowUsData( unsigned long id, unsigned char *opt_data, unsigned long opt_len );
 void CableCardRemovedCallbackCleanUp( void );

static void cablecard_in(void);
static void cablecard_out(void);
static void cablecard_error(void);
static void cablecard_reset(void);
static void OOB_Tx_Docsis(void);
static void page_cleanup(void);
static int bcm_3255_tristate_oob_pins(void);
static int bcm_3255_enable_oob_pins(void);
static NEXUS_FrontendHandle find_oob_tuner(void);

void removeKeyCb(
    uint16_t programNumber,
    uint8_t ltsid
    );

static NEXUS_FrontendHandle find_oob_tuner()
{
	if(oobTuner)
	{
		return oobTuner;
	}
	else
	{
		NEXUS_PlatformConfiguration platformConfig;
		NEXUS_FrontendCapabilities capabilities;
		int i;

		NEXUS_Platform_GetConfiguration(&platformConfig);

		/* OOB tuner, find it and remember*/
		for ( i = MAX_TUNER; i>0; i-- )
		{
			if (platformConfig.frontend[i])
			{
				NEXUS_Frontend_GetCapabilities(platformConfig.frontend[i], &capabilities);
				if (capabilities.outOfBand /*&& capabilities.upstream*/)
				{
					oobTuner = platformConfig.frontend[i];
					BDBG_MSG(("OOB tuner found at index %d", i));
					break;
				}
			}
		}

	}

	if(oobTuner == NULL)
	{
		BDBG_ERR(("OOB tuner not found ! null oobTuner handle will crash the system !!!!"));
	}

	return oobTuner;

}

static void default_lock_callback(
    void *context,
    int param
    )
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
}

static void b_reset_mpod_route(
    NEXUS_MpodHandle mpod
    )
{
    NEXUS_MpodInputSettings settings;
    unsigned pr_band;
    NEXUS_MpodInputHandle mpod_input;

    NEXUS_MpodInput_GetDefaultSettings(&settings);
    settings.bandType = NEXUS_MpodBandType_eParserBand;
    settings.allPass = false;
    settings.mpodPreFilter = false;

    for ( pr_band = 0; pr_band < MAX_CABLECARD_ROUTE; pr_band++)
    {
        settings.band.parserBand = (NEXUS_ParserBand)pr_band;
        mpod_input = NEXUS_MpodInput_Open(mpod, &settings);
        NEXUS_MpodInput_Close(mpod_input);
    }
}

static uint32_t dialogs[8];
#define MARK_AS_VALID(number)   (dialogs[(number) >> 5] |=  (1 << ((number) & 0x1f)))
#define MARK_AS_INVALID(number) (dialogs[(number) >> 5] &= ~(1 << ((number) & 0x1f)))
#define CHECK_IF_VALID(number)  (dialogs[(number) >> 5] &   (1 << ((number) & 0x1f)))

#if 0
int GetInputChar(
    void
    )
{
    struct termios oldattr, tattr;
    int in;

    tcgetattr(0, &tattr);
    oldattr = tattr;
    tattr.c_lflag &= ~(ICANON);
    /* tattr.c_lflag &= ~(ECHO); */
    tattr.c_cc[VTIME] = 0;
    tattr.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &tattr);

    in = fgetc(stdin);

    tcsetattr(0, TCSANOW, &oldattr);

    return in;
}

#define MAX_CABLECARD_APP 10
void *cablecardMenuThread(
    void *data
    )
{
    char *appUrl, *appName, cachedUrl[MAX_CABLECARD_APP][256];
    uint8_t type, numApps, i, appLen, urlLen, cachedUrlLen[MAX_CABLECARD_APP], dialogNb, selection;
    uint16_t version, manuId;
    MPOD_RET_CODE err;
    char cmd[256];

    BSTD_UNUSED(data);
    BKNI_Sleep(15000);
    
    while (!gExitThread) {

        err  = B_Mpod_AppInfoGetManuID(&manuId);
        err |= B_Mpod_AppInfoGetVersionNum(&version);
        err |= B_Mpod_AppInfoGetNumApps(&numApps);

        if(err)
        {
            printf("\n!!!!ERROR retrieving application dialogs\n\n");
        }
        else
        {
            for(i = 0; i < numApps; i++)
            {
                err = B_Mpod_AppInfoGetType(i, &type);
                if(err) printf("ERROR B_Mpod_AppInfoGetType\n");
                err |= B_Mpod_AppInfoGetName(i, &appName, &appLen);
                if(err) printf("ERROR B_Mpod_AppInfoGetName\n");
                err |= B_Mpod_AppInfoGetURL(i, &appUrl, &urlLen);
                if(err) printf("ERROR B_Mpod_AppInfoGetURL\n");

                /* cache the URL's */
                if (i < MAX_CABLECARD_APP)
                {
                    BKNI_Memcpy(&cachedUrl[i][0], appUrl, 256);
                    cachedUrlLen[i] = urlLen;
                }
                printf("%d: %s\n", i, appName);
            }
        }    

        printf("\nEnter Option:");
        cmd[0] = GetInputChar();
        printf("\n");
        
        if((cmd[0] < '0') || (cmd[0] > '9')) continue;
        selection = cmd[0] - '0';
        if(selection >= numApps) continue;
        
        err = B_Mpod_MMIHostOpenDialog(cachedUrl[selection], cachedUrlLen[selection], &dialogNb);
        if(err)
        {
            printf("ERROR Opening Menu Page\n");
        }
        else
        {
            MARK_AS_VALID(dialogNb);
        }
        BKNI_Sleep(5000);
    }

    pthread_exit(NULL);
}
#endif

#ifdef MPODXSIX
static pthread_t dataCbThread;

void *dataready_thread(
    void *data
    )
{
    BSTD_UNUSED(data);

    const void *data_buffer;
    size_t data_buffer_size, desc_size;
    int i;
    void *desc;

    while(1)
    {
        BKNI_Sleep(15);

        for(i=0; i<2; i++)
        {
            if(switchRecpump[i])
            {
                if (NEXUS_Recpump_GetDataBuffer(switchRecpump[i], &data_buffer, &data_buffer_size))
                    continue;
                NEXUS_Playpump_GetBuffer(switchPlaypump[i], &desc, &desc_size);
                if(desc_size)
                {
                    ((NEXUS_PlaypumpDesc *)desc)->addr = (void *)data_buffer;
                    ((NEXUS_PlaypumpDesc *)desc)->length = data_buffer_size;
                    NEXUS_Playpump_ReadComplete(switchPlaypump[i], 0, sizeof(NEXUS_PlaypumpDesc));
                }
                else
                {
                    data_buffer_size = 0;
                    printf("Failed to get playpump descriptor\n");
                }
                    
                NEXUS_Recpump_DataWriteComplete(switchRecpump[i], data_buffer_size);
            }
        }
    }
}
#endif

static void dataready_callback(
    void *context,
    int param
    )
{
    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
}

static void overflow_callback(
    void *context,
    int param
    )
{
    BSTD_UNUSED(param);
    BDBG_ERR(("overflow %s", (const char *)context));
}

cablecard_t cablecard_open(
    cablecard_setting_t setting
    )
{
    int pod, i;
    cablecard_t cablecard = &g_cablecard;
    B_MPOD_IF_SETTINGS IfSettings;
    NEXUS_MpodOpenSettings Mpodsetting;
    pthread_t dsgccThread;
    
#if 0
    /* moved to main() before any thread is launched */
    SetSignalMask();
#endif

    tcgetattr(STDIN_FILENO, &OriginalTerminalIO);
    signal(SIGCHLD, SIG_IGN);
    pthread_create(&dsgccThread, NULL, (void *(*)(void *))(InitializeDsgCcApplication), NULL);

    memset(cablecard, 0, sizeof(struct Cablecard));
    memset(keyslot, 0, sizeof(keyslot));
    
    cablecard->oob_mode = setting->oob_mode;
    cablecard->us_mode = setting->us_mode;

#ifdef ESTB_CFG_SUPPORT
    /*
    *  load the estb_cfg
    */
    B_Estb_cfg_Init("/perm");
    B_Estb_cfg_Init("/dyn");
    B_Estb_cfg_Init("/sys");
    B_Estb_cfg_Open("/perm", "./perm.bin");
    B_Estb_cfg_Open("/dyn", "./dyn.bin");
    B_Estb_cfg_Open("/sys", "./sys.bin");
#endif


    /* open POD driver device*/
    pod = open(CABLECARD_DEVICE, O_RDWR);
    if (pod < 0) {
        BDBG_ERR(("Unable to open cablecard device: %s\n", CABLECARD_DEVICE));
        return NULL;
    }
    cablecard->pod = pod;

    /* open MPOD XPT module*/
    NEXUS_Mpod_GetDefaultOpenSettings(&Mpodsetting);
    Mpodsetting.mpodMode = NEXUS_MpodMode_eMpod;
    Mpodsetting.bandType = NEXUS_MpodBandType_eParserBand;

    Mpodsetting.byteSync = 1;
    Mpodsetting.clockDelay = 1;
    Mpodsetting.invertClock = 0;

    cablecard->mpod = NEXUS_Mpod_Open(0, &Mpodsetting);
    if ( cablecard->mpod == NULL)
    {
        BDBG_ERR(("Unable to open NEXUS mpod "));
        return NULL;
    }

    b_reset_mpod_route(cablecard->mpod );

    BKNI_Memset(pg_info, 0, sizeof(pg_info));

    IfSettings.cardInsertedCb = cablecard_in;
    IfSettings.cardRemovedCb = cablecard_out;
    IfSettings.cardErrorCb = cablecard_error;
    IfSettings.cardResetCb = cablecard_reset;
    B_Mpod_Init(&IfSettings);

    /* Initializing MPOD stack */
    cablecard_init();
    B_Mpod_TestCpInit();

    /* clear all Card related Info*/
    memset(&info, 0, sizeof(info));

    CableCardRemovedCallbackCleanUp();
    bcm_3255_tristate_oob_pins();

#ifdef MPODXSIX
    /* configure record and playback channels */
    for (i=0;i<2;i++)
    {
        NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
        NEXUS_PlaypumpSettings playpumpSettings;
        NEXUS_RecpumpOpenSettings recpumpOpenSettings;
        NEXUS_RecpumpSettings recpumpSettings;
        NEXUS_MpodInputSettings settings;

        switchMap[i] = NEXUS_ParserBand_eMax;

        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
        playpumpOpenSettings.numDescriptors = 2000;

        switchPlaypump[i] = NEXUS_Playpump_Open((NEXUS_NUM_PLAYPUMPS-1)-i, &playpumpOpenSettings);
        
        if (!switchPlaypump[i])
        {
            BDBG_ERR(("NEXUS Error at %d, Exiting...\n", __LINE__));
        }

        BDBG_ERR(("Open playpump %p", switchPlaypump[i]));
        
        NEXUS_Playpump_GetSettings(switchPlaypump[i], &playpumpSettings);
        playpumpSettings.mode = NEXUS_PlaypumpMode_eScatterGather;
        playpumpSettings.allPass = false;

        NEXUS_Playpump_SetSettings(switchPlaypump[i], &playpumpSettings);
        NEXUS_MpodInput_GetDefaultSettings(&settings);
        settings.bandType = NEXUS_MpodBandType_ePlaypump;
        settings.allPass = false;
        settings.mpodPreFilter = true;
        settings.band.playpump = switchPlaypump[i];

        mpodPlaypumpInput[i] = NEXUS_MpodInput_Open(cablecard->mpod, &settings);

        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        recpumpOpenSettings.data.bufferSize = (188*4096) + 68;
        /* threshold is the least common mulitple of a 188 byte transport packet and 4096 atomSize */
        recpumpOpenSettings.data.dataReadyThreshold = (188*2048);
        
        switchRecpump[i] = NEXUS_Recpump_Open((NEXUS_NUM_RECPUMPS-1)-i, &recpumpOpenSettings);
        if (!switchRecpump[i])
        {
            BDBG_ERR(("NEXUS Error at %d, Exiting...\n", __LINE__));
        }
        
        BDBG_ERR(("Open recpump %p", switchRecpump[i]));

        NEXUS_Recpump_GetSettings(switchRecpump[i], &recpumpSettings);
        recpumpSettings.data.dataReady.callback = dataready_callback;
        recpumpSettings.data.dataReady.context = (void *) i;
        recpumpSettings.index.dataReady.callback = dataready_callback;
        recpumpSettings.index.dataReady.context = (void *) i; 
        recpumpSettings.data.overflow.callback = overflow_callback;
        recpumpSettings.data.overflow.context = (void *)"data";
        recpumpSettings.index.overflow.callback = overflow_callback;
        recpumpSettings.index.overflow.context = (void *)"index";
        NEXUS_Recpump_SetSettings(switchRecpump[i], &recpumpSettings);
    }

    if (pthread_create(&dataCbThread, NULL, dataready_thread, (void *)NULL))
    {
        BDBG_ERR(("Failed to create CableCARD pthread, errno %d", errno));
        BDBG_ASSERT(0);
    }
#endif

#if 0
    if (pthread_create(&gCablecardMenuThreadId, NULL, cablecardMenuThread, (void *)NULL))
    {
        BDBG_ERR(("Failed to create CableCARD pthread, errno %d", errno));
        BDBG_ASSERT(0);
    }
#endif

    return cablecard;
}

void cablecard_close(
    void
    )
{
    int i;

    cablecard_t cablecard = cablecard_get_instance();

#ifdef MPODXSIX
    for (i=0;i<2;i++)
    {
        if(mpodPlaypumpInput[i])
            NEXUS_MpodInput_Close(mpodPlaypumpInput[i]);
        if(switchPlaypump[i])
            NEXUS_Playpump_Close(switchPlaypump[i]);
        if(switchRecpump[i])
            NEXUS_Recpump_Close(switchRecpump[i]);
    }
    
    pthread_join(dataCbThread, NULL);
#endif
#if 0
    pthread_join(gCablecardMenuThreadId, NULL);*/
#endif

    if (cablecard->mpod)
    {
        for (i=0; i<MAX_CABLECARD_ROUTE; i++)
        {
            if ( NULL != cablecard->mpod_input[i])
                NEXUS_MpodInput_Close(cablecard->mpod_input[i]);

            if(pg_info[i].valid) removeKeyCb(pg_info[i].program_number, pg_info[i].ltsid);
        }
        NEXUS_Mpod_Close(cablecard->mpod);
    }

    if (cablecard->pod) {
        B_Mpod_Done(cablecard->pod);
        B_Mpod_TestCpDone();
        B_Mpod_AppInfoShutdown();
        B_Mpod_MmiShutdown();
        B_Mpod_SasShutdown();
        B_Mpod_DownloadShutdown();
        B_Mpod_FeatureShutdown();
        B_Mpod_DsgShutdown();
        B_Mpod_HostControlShutdown();
        B_Mpod_DiagsShutdown();
        B_Mpod_CaShutdown();
        B_Mpod_CpShutdown();
        B_Mpod_HeadendCommShutdown();
        B_Mpod_SnmpShutdown();
        B_Mpod_HostPropertiesShutdown();
        B_Mpod_SystimeShutdown();
        B_Mpod_HomingShutdown();
        B_Mpod_ExtendedChShutdown();

        close(cablecard->pod);
    }

    if (cablecard->mpeg_section)
        BKNI_Free(cablecard->mpeg_section);

    cablecard->mpeg_section = NULL;
    cablecard->cablecard_in = false;

    CableCardRemovedCallbackCleanUp();
    bcm_3255_tristate_oob_pins();

    /* set terminal back to main app*/
    tcsetattr(STDIN_FILENO, TCSANOW, &OriginalTerminalIO);
    signal(SIGINT, SIG_IGN);
    
#ifdef ESTB_CFG_SUPPORT
    /*
    *  load the estb_cfg
    */
    B_Estb_cfg_Close("/perm");
    B_Estb_cfg_Close("/dyn");
    B_Estb_cfg_Uninit("/perm");
    B_Estb_cfg_Uninit("/dyn");
#endif

}


#ifdef MPODXSIX
void cablecard_unswitch_parser_band(
    NEXUS_ParserBand parserBand,
    NEXUS_PidChannelHandle *videoPidChannel,
    NEXUS_PidChannelHandle *audioPidChannel,
    NEXUS_PidChannelHandle *pcrPidChannel,
    NEXUS_PidChannelHandle *patPidChannel,
    NEXUS_PidChannelHandle *pmtPidChannel, 
    NEXUS_PidChannelHandle *caPidChannel
    )
{
    cablecard_t cablecard = cablecard_get_instance();
    NEXUS_RecpumpHandle recpump;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_MpodInputSettings settings;
    
    if((switchMap[0] == parserBand) || (switchMap[1] == parserBand))
    {
        uint32_t index = (parserBand == switchMap[0]) ? 0 : 1;
        
        switchMap[index] = NEXUS_ParserBand_eMax;
        recpump = switchRecpump[index];
        playpump = switchPlaypump[index];

        BDBG_MSG(("Unswitch parser band %d playpump %p recpump %p", parserBand, playpump, recpump));
        
        NEXUS_Playpump_Stop(playpump);
        NEXUS_Recpump_Stop(recpump); 
        NEXUS_Recpump_RemoveAllPidChannels(recpump);
        NEXUS_Playpump_CloseAllPidChannels(playpump);


        *videoPidChannel = videoPidChannels[index];
        *audioPidChannel = audioPidChannels[index];
        *pcrPidChannel = pcrPidChannels[index];
        *patPidChannel = patPidChannels[index];
        *pmtPidChannel = pmtPidChannels[index];
        *caPidChannel = caPidChannels[index];

        if (NULL == cablecard->mpod_input[parserBand])
        {
            NEXUS_MpodInput_GetDefaultSettings(&settings);
            
            settings.bandType = NEXUS_MpodBandType_eParserBand;
            settings.allPass = false;
#ifdef MPODXSIX
            settings.mpodPreFilter = true;
#else
            settings.mpodPreFilter = false;
#endif
            settings.band.parserBand = parserBand;
            cablecard->mpod_input[parserBand] = NEXUS_MpodInput_Open(cablecard->mpod, &settings);
        }
    }
}

NEXUS_PlaypumpHandle cablecard_switch_parser_band(
    NEXUS_ParserBand parserBand
    )
{

    cablecard_t cablecard = cablecard_get_instance();
    NEXUS_PlaypumpHandle playpump;

    if(switchMap[0] != NEXUS_ParserBand_eMax)
    {
        if(switchMap[1] != NEXUS_ParserBand_eMax)
        {
            BDBG_MSG(("Do not switch parser band %d", parserBand));
            return NULL;
        }
        else
        {
            switchMap[1] = parserBand;
            playpump = switchPlaypump[1];
        }
    }
    else
    {
        switchMap[0] = parserBand;
        playpump = switchPlaypump[0];
    }

    BDBG_ERR(("Switch parser band %d for playpump %p", parserBand, playpump));
    
    if (NULL != cablecard->mpod_input[parserBand])
    {
        NEXUS_MpodInput_Close(cablecard->mpod_input[parserBand]);
        cablecard->mpod_input[parserBand] = NULL;
    }
    
    return playpump;
}

void cablecard_add_pid_channels(
    NEXUS_PlaypumpHandle playpumpHandle,
    NEXUS_PidChannelHandle videoPidChannel,
    NEXUS_PidChannelHandle audioPidChannel,
    NEXUS_PidChannelHandle pcrPidChannel,
    NEXUS_PidChannelHandle patPidChannel,
    NEXUS_PidChannelHandle pmtPidChannel, 
    NEXUS_PidChannelHandle caPidChannel
    )
{
    uint32_t index = (playpumpHandle == switchPlaypump[0]) ? 0 : 1;
    NEXUS_RecpumpHandle recpump = switchRecpump[index];

    if(videoPidChannel) NEXUS_Recpump_AddPidChannel(recpump, videoPidChannel, NULL);
    if(audioPidChannel) NEXUS_Recpump_AddPidChannel(recpump, audioPidChannel, NULL);
    if(pcrPidChannel)   NEXUS_Recpump_AddPidChannel(recpump, pcrPidChannel, NULL);
    if(patPidChannel)   NEXUS_Recpump_AddPidChannel(recpump, patPidChannel, NULL);
    if(pmtPidChannel)   NEXUS_Recpump_AddPidChannel(recpump, pmtPidChannel, NULL);
    if(caPidChannel)    NEXUS_Recpump_AddPidChannel(recpump, caPidChannel, NULL);
    
    videoPidChannels[index] = videoPidChannel;
    audioPidChannels[index] = audioPidChannel;
    pcrPidChannels[index] = pcrPidChannel;
    patPidChannels[index] = patPidChannel;
    pmtPidChannels[index] = pmtPidChannel;
    caPidChannels[index] = caPidChannel;

    NEXUS_Playpump_Start(playpumpHandle);
    NEXUS_Recpump_Start(recpump); 
    BDBG_MSG(("Starting playpump %p recpump %p", playpumpHandle, recpump));
}

#endif

/*
Summary:
   Enable routing tuner's transport output to CableCARD.
Description:
*/
int cablecard_route_add_tuner(
   cablecard_t cablecard,
   NEXUS_FrontendHandle frontend
   )
{
    NEXUS_ParserBand pr_band;
    NEXUS_MpodInputSettings settings;
    NEXUS_FrontendUserParameters userParams;

    if (cablecard == NULL || frontend == NULL) return -1;

    BDBG_MSG(("Routing Tuners to MPOD"));

    NEXUS_MpodInput_GetDefaultSettings(&settings);
    settings.bandType = NEXUS_MpodBandType_eParserBand;
    settings.allPass = false;
#ifdef MPODXSIX
    settings.mpodPreFilter = true;
#else
    settings.mpodPreFilter = false;
#endif
    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    pr_band = userParams.param1&0x7;
    settings.band.parserBand = pr_band;

    if (NULL == cablecard->mpod_input[pr_band])
    {
        cablecard->mpod_input[pr_band] = NEXUS_MpodInput_Open(cablecard->mpod, &settings);
        if (cablecard->mpod_input[pr_band] == NULL) return -1;
    }
    else
    {
        BDBG_WRN((" tuner has already been routed to MPOD"));
    }

    return 0;
}

/*
Summary:
   Disable routing tuner's transport output to CableCARD.
Description:
*/
int cablecard_route_remove_tuner(
   cablecard_t cablecard,
   NEXUS_FrontendHandle frontend
   )
{
    NEXUS_ParserBand pr_band;
    NEXUS_FrontendUserParameters userParams;

    if (cablecard == NULL || frontend == NULL) return -1;

    BDBG_MSG(("Routing Tuners away from MPOD"));

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    pr_band = userParams.param1&0x7;

    if (NULL != cablecard->mpod_input[pr_band])
    {
        NEXUS_MpodInput_Close(cablecard->mpod_input[pr_band]);
        cablecard->mpod_input[pr_band] = NULL;
    }
    else
    {
        BDBG_MSG((" tuner has already been disconnected from MPOD"));
    }
    return 0;
}

int cablecard_go(
    void
    )
{
    cablecard_t cablecard = cablecard_get_instance();
    
    B_Mpod_Go(cablecard->pod);
    return 0;
}

cablecard_t cablecard_get_instance(
    void
    )
{
    return (&g_cablecard);
}

int cablecard_get(
    cablecard_t cablecard,
    cablecard_setting_t setting
    )
{
    if (cablecard == NULL || setting == NULL) return -1;
    setting->oob_mode = cablecard->oob_mode;
    setting->us_mode = cablecard->us_mode;
    return 0;
}

int cablecard_set(
    cablecard_t cablecard,
    cablecard_setting_t setting
    )
{
    if (cablecard == NULL|| setting == NULL) return -1;
    cablecard->oob_mode = setting->oob_mode;
    cablecard->us_mode = setting->us_mode;
    return 0;
}

#define ISO936_CODE_LENGTH 3
#define MAX_PROGRAM_CC_SERVICE	32
typedef struct
{
    uint16_t pid;
    uint8_t streamType;
    uint16_t ca_pid;
    unsigned char iso639[ISO936_CODE_LENGTH];
} EPID;

typedef struct
{
    uint8_t ccType;
    uint8_t ccService;
    unsigned char iso639[ISO936_CODE_LENGTH];
} ECC;

#define MAX_PROGRAM_MAP_PIDS	12
typedef struct
{
    uint16_t program_number;
    uint16_t map_pid;
    uint8_t  version;
    uint8_t  broadcast_flag;
    uint8_t  num_cc;
    ECC      cc[MAX_PROGRAM_CC_SERVICE];
    uint16_t pcr_pid;
    uint16_t ca_pid;
    uint8_t  num_video_pids;
    EPID     video_pids[MAX_PROGRAM_MAP_PIDS];
    uint8_t  num_audio_pids;
    EPID     audio_pids[MAX_PROGRAM_MAP_PIDS];
    uint8_t  num_other_pids;
    EPID     other_pids[MAX_PROGRAM_MAP_PIDS];
    uint32_t pmt_size;
    uint8_t *pmt;
} PROGRAM_INFO_T;

static char atsc_id[4] = "GA94";
static char scte_id[4] = "SCTE";
void tsPsi_procProgDescriptors(
    const uint8_t *p_bfr,
    unsigned bfrSize,
    PROGRAM_INFO_T *progInfo
    )
{
    int i,j;
    char *cc_ptr;
    TS_PSI_descriptor descriptor;

    for( i = 0, descriptor = TS_PMT_getDescriptor( p_bfr, bfrSize, i );
        descriptor != NULL;
        i++, descriptor = TS_PMT_getDescriptor( p_bfr, bfrSize, i ) )
    {
        switch (descriptor[0])
        {
        case TS_PSI_DT_CA:
            progInfo->ca_pid = ((descriptor[4] & 0x1F) << 8) + descriptor[5];
            break;
        case TS_PSI_DT_Registration:
            if (memcmp(atsc_id, &descriptor[2], 4) ==0)
                BDBG_WRN((" ATSC MPEG2 registration descriptor"));
            else if (memcmp(scte_id, &descriptor[2], 4) == 0)
                BDBG_WRN((" SCTE MPEG2 registration descriptor"));
            else
                BDBG_WRN((" unknown MPEG2 registration descriptor"));
            break;
        case TS_PSI_DT_ATSC_RedistributionControl:
            progInfo->broadcast_flag = true;
            break;
        case TS_PSI_DT_ATSC_CaptionService:
            progInfo->num_cc = descriptor[2]&0x1f;
            cc_ptr = (char*)&descriptor[3];
            for (j=0;j<progInfo->num_cc;j++)
            {
                memcpy(progInfo->cc[j].iso639,cc_ptr,3);
                cc_ptr +=3;
                progInfo->cc[j].ccType = *cc_ptr>>7;
                progInfo->cc[j].ccService = *cc_ptr&0x3f;
                cc_ptr += 3;
            }
            break;
        case TS_PSI_DT_ATSC_ContentAdvisory:
            break;
        default:
            break;
        }
    }
}

void tsPsi_procStreamDescriptors(
    const uint8_t *p_bfr,
    unsigned bfrSize,
    int streamNum,
    EPID *ePidData
    )
{
    int i;
    TS_PSI_descriptor descriptor;

    for( i = 0, descriptor = TS_PMT_getStreamDescriptor( p_bfr, bfrSize, streamNum, i );
        descriptor != NULL;
        i++, descriptor = TS_PMT_getStreamDescriptor( p_bfr, bfrSize, streamNum, i ) )
    {
        switch (descriptor[0])
        {
        case TS_PSI_DT_CA:
            ePidData->ca_pid = ((descriptor[4] & 0x1F) << 8) + descriptor[5];
            break;
        case TS_PSI_DT_ATSC_ComponentName:
            break;
        case TS_PSI_DT_ATSC_AC3_Audio:
            break;
        default:
            break;
        }
    }
}

#define ADD_VIDEO_PID(P_INFO, PID, TYPE, PMT, PMTSIZE, INDEX) \
    do { \
    if( (P_INFO)->num_video_pids < MAX_PROGRAM_MAP_PIDS ) \
    { \
    BDBG_MSG(("  vpid[%d] 0x%x, type 0x%x", (P_INFO)->num_video_pids, (PID), (TYPE))); \
    (P_INFO)->video_pids[(P_INFO)->num_video_pids].pid = (PID); \
    (P_INFO)->video_pids[(P_INFO)->num_video_pids].streamType = (TYPE); \
    tsPsi_procStreamDescriptors((PMT), (PMTSIZE), (INDEX), &(P_INFO)->video_pids[(P_INFO)->num_video_pids] ); \
    (P_INFO)->num_video_pids++; \
    } \
    } while (0)
#define ADD_AUDIO_PID(P_INFO, PID, TYPE, PMT, PMTSIZE, INDEX) \
    do { \
    if( (P_INFO)->num_audio_pids < MAX_PROGRAM_MAP_PIDS ) \
    { \
    BDBG_MSG(("  apid[%d] 0x%x, type 0x%x", (P_INFO)->num_audio_pids, (PID), (TYPE))); \
    (P_INFO)->audio_pids[(P_INFO)->num_audio_pids].pid = (PID); \
    (P_INFO)->audio_pids[(P_INFO)->num_audio_pids].streamType = (TYPE); \
    tsPsi_procStreamDescriptors((PMT), (PMTSIZE), (INDEX), &(P_INFO)->audio_pids[(P_INFO)->num_audio_pids] ); \
    (P_INFO)->num_audio_pids++; \
    } \
    } while (0)
#define ADD_OTHER_PID(P_INFO, PID, TYPE, PMT, PMTSIZE, INDEX) \
    do { \
    if( (P_INFO)->num_other_pids < MAX_PROGRAM_MAP_PIDS ) \
    { \
    BDBG_MSG(("  opid[%d] 0x%x, type 0x%x", (P_INFO)->num_audio_pids, (PID), (TYPE))); \
    (P_INFO)->other_pids[(P_INFO)->num_other_pids].pid = (PID); \
    (P_INFO)->other_pids[(P_INFO)->num_other_pids].streamType = (TYPE); \
    tsPsi_procStreamDescriptors((PMT), (PMTSIZE), (INDEX), &(P_INFO)->other_pids[(P_INFO)->num_other_pids] ); \
    (P_INFO)->num_other_pids++; \
    } \
    } while (0)

void parsePMT(
    const void *pmt,
    unsigned pmtSize,
    PROGRAM_INFO_T *p_programInfo
    )
{
    int i;
    TS_PMT_stream pmt_stream;
    TS_PSI_header header;

    TS_PSI_getSectionHeader(pmt, &header );

    /* Store the main information about the program */
    p_programInfo->program_number   = header.table_id_extension;
    p_programInfo->version          = header.version_number;
    p_programInfo->pcr_pid          = TS_PMT_getPcrPid(pmt, pmtSize);

    /* find and process Program descriptors */
    tsPsi_procProgDescriptors(pmt, pmtSize, p_programInfo );

    /* Find the video and audio pids... */
    p_programInfo->num_video_pids   = 0;
    p_programInfo->num_audio_pids   = 0;
    p_programInfo->num_other_pids   = 0;

    for( i = 0; i < TS_PMT_getNumStreams(pmt, pmtSize); i++ )
    {
        int descIdx = 0;
        if (TS_PMT_getStream(pmt, pmtSize, i, &pmt_stream )) {
            BDBG_WRN(("Invalid PMT data detected"));
            continue;
        }

        switch( pmt_stream.stream_type )
        {
        /* video formats */
        case TS_PSI_ST_11172_2_Video:  /* MPEG-1 */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg1, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_ATSC_Video:   /* ATSC MPEG-2 */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg2, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_13818_2_Video: /* MPEG-2 */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg2, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_14496_2_Video: /* MPEG-4 Part 2 */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg4Part2, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_14496_10_Video: /* H.264/AVC */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eH264, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_AVS_Video: /* AVS */
            ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eAvs, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_SMPTE_VC1:      /* VC-1 */
            /* need to parse descriptor and then subdescriptor to determine profile */
            for (;;) {
                TS_PSI_descriptor desc = TS_PMT_getStreamDescriptor(pmt, pmtSize, i, descIdx);
                if (desc == NULL) break;
                descIdx++;

                switch(desc[0]) {
                case TS_PSI_DT_Registration:
                    /* calculate and check format_identifier */
                    {
                    uint32_t format_identifier = (desc[2] << 24) + (desc[3] << 16) + (desc[4] << 8) + desc[5];
                    if (format_identifier == 0x56432D31) {
                        /* check that proper sub-descriptor exists */
                        int subdescriptor_tag = desc[6];
                        if (subdescriptor_tag == 0x01) {
                            int profile_level = desc[7];
                            if (profile_level >= 0x90)  /* Advanced Profile ES */
                                ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eVc1, pmt, pmtSize, i);
                            else /* Simple/Main Profile ES */
                                ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eVc1SimpleMain, pmt, pmtSize, i);
                        }
                    }
                    }
                    break;
                default:
                    break;
                }
            }
            break;

        /* audio formats */
        case TS_PSI_ST_11172_3_Audio: /* MPEG-1 */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eMpeg, pmt, pmtSize, i);  /* Same baudio_format for MPEG-1 or MPEG-2 audio */
            break;
        case TS_PSI_ST_13818_3_Audio: /* MPEG-2 */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eMpeg, pmt, pmtSize, i);  /* Same baudio_format for MPEG-1 or MPEG-2 audio */
            break;
        case TS_PSI_ST_13818_7_AAC:  /* MPEG-2 AAC */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAac, pmt, pmtSize, i);    /* Note baudio_format_aac = MPEG-2 AAC */
            break;
        case TS_PSI_ST_14496_3_Audio: /* MPEG-4 AAC */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAacPlus, pmt, pmtSize, i);   /* Note baudio_format_aac_plus = MPEG-4 AAC */
            break;
        case TS_PSI_ST_ATSC_AC3:      /* ATSC AC-3 */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAc3, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_ATSC_EAC3:     /* ATSC Enhanced AC-3 */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAc3Plus, pmt, pmtSize, i);
            break;
        case TS_PSI_ST_AVS_Audio:     /* AVS */
            ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAvs, pmt, pmtSize, i);
            break;


        /* video or audio */
        case TS_PSI_ST_13818_1_PrivatePES:  /* examine descriptors to handle private data */
            for (;;) {
                TS_PSI_descriptor desc = TS_PMT_getStreamDescriptor(pmt, pmtSize, i, descIdx);
                if (desc == NULL) break;
                descIdx++;

                switch(desc[0]) {
                /* video formats */
                case TS_PSI_DT_VideoStream:
                    /* MPEG_1_only_flag is bit 2 of desc[2], this determines MPEG-1/2 */
                    if ((desc[2] & 0x04) == 1)
                        ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg1, pmt, pmtSize, i);
                    else
                        ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg2, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_MPEG4_Video:
                    ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eMpeg4Part2, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_AVC:
                    ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eH264, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_AVS_Video:
                    ADD_VIDEO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_VideoCodec_eAvs, pmt, pmtSize, i);
                    break;

                /* audio formats */
                case TS_PSI_DT_AudioStream:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eMpeg, pmt, pmtSize, i);   /* Same baudio_format for MPEG-1 or MPEG-2 audio */
                    break;
                case TS_PSI_DT_MPEG2_AAC:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAac, pmt, pmtSize, i);   /* Note baudio_format_aac = MPEG-2 AAC */
                    break;
                case TS_PSI_DT_MPEG4_Audio:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAacPlus, pmt, pmtSize, i); /* Note baudio_format_aac_plus = MPEG-4 AAC */
                    break;
                case TS_PSI_DT_DVB_AAC:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAacPlus, pmt, pmtSize, i); /* Note baudio_format_aac_plus = MPEG-4 AAC */
                    break;
                case TS_PSI_DT_DVB_AC3:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAc3, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_DVB_EnhancedAC3:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eAc3Plus, pmt, pmtSize, i);
                    break;
                case TS_PSI_DT_DVB_DTS:
                    ADD_AUDIO_PID(p_programInfo, pmt_stream.elementary_PID, NEXUS_AudioCodec_eDts, pmt, pmtSize, i);
                    break;
                default:
                    break;
                }
            }
            break;
        default:
            if( p_programInfo->num_other_pids < MAX_PROGRAM_MAP_PIDS )
            {
                ADD_OTHER_PID(p_programInfo, pmt_stream.elementary_PID, pmt_stream.stream_type, pmt, pmtSize, i);
            }
            break;
        }
        /* If we get any data our status is complete! */
    } /* EFOR Program map loop */
}


static uint32_t cablecard_get_index(
    uint8_t ltsid
    )
{
    uint32_t i;

    /* assumption: this will not be called unless there is at least one available slot */

    /* already assigned? */
    for (i=0;i<MAX_CABLECARD_ROUTE;i++)
    {
        if ((pg_info[i].ltsid == ltsid) && pg_info[i].valid)
            return i;
    }

    /* otherwise pick first available */
    for (i=0;i<MAX_CABLECARD_ROUTE;i++)
    {
        if (!(pg_info[i].valid))
            return i;
    }

    return 0xFFFF;
}

int cablecard_inquire_program(
    cablecard_t cablecard,
    cablecard_program_info_t info
    )
{
    MPOD_RET_CODE	ret;

    if (cablecard == NULL) return -1;
    BDBG_MSG((" inquire program ltsid %d PG num %d video_pid 0x%x", info->ltsid, info->program_number, info->video_pid));
    ret = B_Mpod_CaSendPmt((unsigned char *)info->pmt, B_MPOD_CA_QUERY, info->program_index, info->ltsid, info->source_id
                );
    return (ret != MPOD_SUCCESS);
}

void removeKeyCb(
    uint16_t programNumber,
    uint8_t ltsid
    )
{
#if NEXUS_HAS_SECURITY
    int i;
    NEXUS_Error rc;
    
    for (i=0;i<MAX_CABLECARD_ROUTE;i++)
    {
        if ((pg_info[i].program_number == programNumber) && (pg_info[i].ltsid == ltsid) && pg_info[i].valid) break;
    }
    
    if (i >= MAX_CABLECARD_ROUTE) {
        BDBG_ERR((" can't match based on ltsid and PG number"));
        return;
    }

    BDBG_MSG(("Removing CPKey for program number %d, ltsid %d\n", programNumber, ltsid));

    if (keyslot[i])
    {
        if (pg_info[i].audio_pid[0] != 0 && pg_info[i].audio_pid[0] != 0x1fff)
        {
            rc = NEXUS_Security_RemovePidChannelFromKeySlot(keyslot[i], pg_info[i].audio_pidchannel[0]);
            if (rc != 0) 
            {
                BDBG_ERR(("NEXUS_Security_RemovePidChannelFromKeySlot: failed "));
            }
        }

        if (pg_info[i].video_pid != 0 && pg_info[i].video_pid != 0x1fff)
        {
            rc = NEXUS_Security_RemovePidChannelFromKeySlot(keyslot[i], pg_info[i].video_pidchannel);
            if (rc != 0)
            {
                BDBG_ERR(("NEXUS_Security_RemovePidChannelFromKeySlot: failed "));
            }
        }

        NEXUS_Security_FreeKeySlot(keyslot[i]);
        BDBG_MSG(("Free keyslot %p ", keyslot[i]));
        keyslot[i] = NULL;        
    }
        
#else
    BSTD_UNUSED(programNumber);
    BSTD_UNUSED(ltsid);
#endif
}

void progKeyCb(
    uint8_t *desABAKey,
    uint16_t programNumber,
    uint8_t ltsid
    )
{
#if NEXUS_HAS_SECURITY
    int i;
    NEXUS_Error rc;
    NEXUS_SecurityClearKey ClearKey;

    for (i=0;i<MAX_CABLECARD_ROUTE;i++)
    {
        if (pg_info[i].program_number == programNumber && pg_info[i].ltsid == ltsid) break;
    }
    
    if (i >= MAX_CABLECARD_ROUTE) {
        BDBG_ERR((" can't match based on ltsid and PG number"));
        return;
    }

    BDBG_MSG((" CP: ltsid 0x%x ecmpid 0x%x Vpid 0x%x Apid 0x%x", ltsid, pg_info[i].ca_pid, pg_info[i].video_pid,
        pg_info[i].audio_pid[0]));

    if (!keyslot[i])
    {
        NEXUS_SecurityAlgorithmSettings config_algorithm;
        NEXUS_SecurityKeySlotSettings Settings;
        
        NEXUS_Security_GetDefaultAlgorithmSettings(&config_algorithm);
        config_algorithm.terminationMode = NEXUS_SecurityTerminationMode_eCipherStealing;
        config_algorithm.algorithm = NEXUS_SecurityAlgorithm_e3DesAba;
        config_algorithm.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
        
        NEXUS_Security_GetDefaultKeySlotSettings(&Settings);
        Settings.keySlotEngine = NEXUS_SecurityEngine_eCa;
        BDBG_MSG(("Allocating new key slot"));
        keyslot[i] = NEXUS_Security_AllocateKeySlot(&Settings);
        if (keyslot[i]== NULL) goto err;
        rc = NEXUS_Security_ConfigAlgorithm(keyslot[i], &config_algorithm);
        if (rc != 0) goto err;
    }

    /* Route a clear key */
    ClearKey.keySize = 16; /* 16 bytes (128 bits) */
    BKNI_Memcpy(ClearKey.keyData, desABAKey, ClearKey.keySize);
    
    NEXUS_Security_GetDefaultClearKey(&ClearKey);
    ClearKey.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    rc = NEXUS_Security_LoadClearKey( keyslot[i], &ClearKey);
    if (rc != 0) goto err;
    ClearKey.keyEntryType = NEXUS_SecurityKeyType_eEven;
    rc = NEXUS_Security_LoadClearKey( keyslot[i], &ClearKey);
    if (rc != 0) goto err;

    if (pg_info[i].video_pid != 0 && pg_info[i].video_pid != 0x1fff)
    {
        BDBG_MSG(("Video Key, pid ch %d", pg_info[i].video_pidchannel));
        rc = NEXUS_Security_AddPidChannelToKeySlot(keyslot[i], pg_info[i].video_pidchannel);
        if (rc != 0) goto err;
    }

    if (pg_info[i].audio_pid[0] != 0 && pg_info[i].audio_pid[0] != 0x1fff)
    {
        BDBG_MSG(("Audio Key, pid ch %d", pg_info[i].audio_pidchannel[0]));
        rc = NEXUS_Security_AddPidChannelToKeySlot(keyslot[i], pg_info[i].audio_pidchannel[0]);
        if (rc != 0) goto err;
    }

    return;
err:
    BDBG_ERR(("Error in enabling 3des decryption\n"));
    return;
#else
    BSTD_UNUSED(desABAKey);
    BSTD_UNUSED(programNumber);
    BSTD_UNUSED(ltsid);
#endif
}

int cablecard_enable_program(
    uint8_t* pmt,
    uint32_t pmtSize,
    NEXUS_ParserBand parserBand,
    NEXUS_PidChannelHandle videoPidChannel,
    NEXUS_PidChannelHandle audioPidChannel,
    uint32_t source_id,
    uint16_t *caPid
    )
{
    MPOD_RET_CODE   ret;
    cablecard_program_info info;
    PROGRAM_INFO_T p_programInfo;
    uint32_t j;
    uint8_t ltsid;
    NEXUS_PidChannelStatus status;
    NEXUS_PlaypumpHandle playpump;

    if(pmtSize == 0 || videoPidChannel == NULL || audioPidChannel == NULL) return -1;

    printf("cablecard_enable_program\n");
    NEXUS_PidChannel_GetStatus(videoPidChannel, &status);
    info.video_pidchannel = status.pidChannelIndex;
    NEXUS_PidChannel_GetStatus(audioPidChannel, &status);
    info.audio_pidchannel[0] = status.pidChannelIndex;
    
    parsePMT((const void *)pmt, pmtSize, &p_programInfo);
    
    /* retrieve ltsid. Input parser band or playback parser band? */
    ltsid = parserBand; /* if input parser band, ltsid = parser band index */
    if((switchMap[0] == parserBand) || (switchMap[1] == parserBand))
    {
        /* playback parser band */
        NEXUS_PlaypumpStatus playpumpStatus;
        
        playpump = (switchMap[0] == parserBand) ? switchPlaypump[0] : switchPlaypump[1];
        NEXUS_Playpump_GetStatus(playpump, &playpumpStatus);
        ltsid = playpumpStatus.index + 0x10; /* ltsid = playback parser band index + 0x10 */
    }
    
    info.program_index = cablecard_get_index(ltsid);
    BDBG_ASSERT((info.program_index < MAX_CABLECARD_ROUTE));

    /* convert p_programInfo to cablecard_program_info_t */
    info.source_id = source_id;
    info.program_number = p_programInfo.program_number;
    info.video_pid = p_programInfo.video_pids[0].pid;
    info.ca_pid = 0;
    for (j=0;j<MAX_AUDIO_STREAM;j++)
    {
        info.audio_pid[j] = p_programInfo.audio_pids[j].pid;
    }
    if (p_programInfo.num_video_pids && p_programInfo.video_pids[0].ca_pid)
    {
        info.ca_pid = p_programInfo.video_pids[0].ca_pid;
    }
    else if (p_programInfo.num_audio_pids && p_programInfo.audio_pids[0].ca_pid)
    {
        info.ca_pid = p_programInfo.audio_pids[0].ca_pid;
    }
    if (p_programInfo.ca_pid)
    {
        info.ca_pid = p_programInfo.ca_pid;
    }
    if (!info.ca_pid)
    {
        BDBG_MSG((" NO CA pid found!"));
    }
    *caPid = info.ca_pid;
    info.ltsid = ltsid;
    info.valid = true;
    BKNI_Memcpy(info.pmt, pmt, pmtSize);

    BDBG_MSG((" enable program ltsid %d PG num %d video_pid 0x%x ca_pid 0x%x", ltsid, info.program_number, info.video_pid, info.ca_pid));

    ret = B_Mpod_CaSendPmt((unsigned char *)pmt, B_MPOD_CA_OK_DESCRAMBLE, info.program_index, ltsid, source_id);
    
    pg_info[info.program_index] = info;

    if (ret == MPOD_SUCCESS) {
        B_Mpod_CpAddProgram(info.program_index,info.program_number, info.ca_pid, ltsid);
    }

    return (ret != MPOD_SUCCESS);
}

int cablecard_disable_program(
    NEXUS_ParserBand parserBand
    )
{
    MPOD_RET_CODE	ret;
    cablecard_program_info info;
    uint8_t ltsid, index;
    NEXUS_PlaypumpHandle playpump;
    
    /* retrieve ltsid. Input parser band or playback parser band? */
    ltsid = parserBand; /* if input parser band, ltsid = parser band index */
    if((switchMap[0] == parserBand) || (switchMap[1] == parserBand))
    {
        /* playback parser band */
        NEXUS_PlaypumpStatus playpumpStatus;
        
        playpump = (switchMap[0] == parserBand) ? switchPlaypump[0] : switchPlaypump[1];
        NEXUS_Playpump_GetStatus(playpump, &playpumpStatus);
        ltsid = playpumpStatus.index + 0x10; /* ltsid = playback parser band index + 0x10 */
    }
    
    index = cablecard_get_index(ltsid);
    BDBG_ASSERT((index < MAX_CABLECARD_ROUTE));
    
    info = pg_info[index];
    ret = B_Mpod_CaSendPmt((unsigned char *)info.pmt, B_MPOD_CA_NOT_SELECTED, info.program_index, ltsid, info.source_id);

    B_Mpod_CpRemoveProgram(index);

    info.valid = false;
    
    BDBG_MSG((" disable program ltsid %d PG num %d video_pid 0x%x ca_pid 0x%x", ltsid, info.program_number, info.video_pid, info.ca_pid));

    return 0;
}

int cablecard_set_mpeg_section_callback(
    cablecard_t cablecard,
    cablecard_mpeg_callback callback
    )
{
    if (cablecard == NULL) return -1;

    if (!cablecard->mpeg_section) {
        cablecard->mpeg_section = (unsigned char *)BKNI_Malloc(MAX_MPEG_SECTION_LEN);
        cablecard->mpeg_section_len = 0;
    }
    cablecard->si_callback = callback;

    return 0;
}

int cablecard_get_mpeg_section(
    cablecard_t cablecard,
    void *buffer,
    size_t size
    )
{
    if (cablecard == NULL) return -1;

    if (cablecard->mpeg_section) {
        cablecard_lock();
        size = (cablecard->mpeg_section_len > size)? size : cablecard->mpeg_section_len;
        BKNI_Memcpy(buffer, cablecard->mpeg_section, size);
        cablecard_unlock();
        return size;
    }

    return -1;
}

static int bcm_3255_tristate_oob_pins(
    void
    )
{
#if 1
    NEXUS_3255DeviceGpioPinSettings settings;
    settings.mode = NEXUS_GpioMode_eOutputOpenDrain;
    return NEXUS_Frontend_Set3255DeviceGpioPinSettings(find_oob_tuner(), NEXUS_3255DeviceGpioPin_eOob, &settings);
#else
    NEXUS_3255GpioPinSettings settings;
    NEXUS_PlatformConfiguration platformConfig;
    int rc;

    settings.mode = NEXUS_GpioMode_eOutputOpenDrain;
    NEXUS_Platform_GetConfiguration(&platformConfig);
    rc = NEXUS_Frontend_3255_SetGpioPinSettings(platformConfig.frontend[MAX_TUNER], NEXUS_3255GpioPin_eOob, &settings);
    return rc;
#endif
}

static int bcm_3255_enable_oob_pins(
    void
    )
{
#if 1
    NEXUS_3255DeviceGpioPinSettings settings;
    settings.mode = NEXUS_GpioMode_eOutputPushPull;
    return NEXUS_Frontend_Set3255DeviceGpioPinSettings(find_oob_tuner(), NEXUS_3255DeviceGpioPin_eOob, &settings);
#else
    NEXUS_3255GpioPinSettings settings;
    NEXUS_PlatformConfiguration platformConfig;

    settings.mode = NEXUS_GpioMode_eOutputPushPull;
    NEXUS_Platform_GetConfiguration(&platformConfig);
    return NEXUS_Frontend_3255_SetGpioPinSettings(platformConfig.frontend[MAX_TUNER], NEXUS_3255GpioPin_eOob, &settings);
#endif
}

void cablecard_in(
    void
    )
{
    cablecard_t cablecard = cablecard_get_instance();
    NEXUS_PlatformConfiguration platformConfig;
    int i;

    BDBG_WRN((" CableCard is in\n"));
    bcm_3255_enable_oob_pins();
    cablecard->cablecard_in = true;
    page_cleanup();
    NEXUS_Platform_GetConfiguration(&platformConfig);
    for (i=0;i<5;i++)
    {
        if (platformConfig.frontend[i])
            cablecard_route_add_tuner(cablecard, platformConfig.frontend[i]);
    }
}

void cablecard_out(
    void
    )
{
    cablecard_t cablecard = cablecard_get_instance();
    NEXUS_PlatformConfiguration platformConfig;
    int i;

    BDBG_WRN((" CableCard is out\n"));
    cablecard->cablecard_in = false;
    cablecard_reset();
    bcm_3255_tristate_oob_pins();
    NEXUS_Platform_GetConfiguration(&platformConfig);
    for (i=0;i<5;i++)
    {
        if (platformConfig.frontend[i])
            cablecard_route_remove_tuner(cablecard, platformConfig.frontend[i]);
    }
}

void cablecard_error(
    void
    )
{
    BDBG_WRN((" CableCard Error\n"));
}

void cablecard_reset(
    void
    )
{
#ifdef ESTB_CFG_SUPPORT
    unsigned int  cdl_group_id = 0;
#endif
    CableCardRemovedCallbackCleanUp();
    memset(&info, 0, sizeof(info));
#if 0
    channel_mgr_reset_map();
#endif
    page_cleanup();
#ifdef ESTB_CFG_SUPPORT
    B_Estb_cfg_Set_uint32("/dyn/estb/group_id", cdl_group_id);
#endif
}

int cablecard_set_dsgtunnel_handler(
    cablecard_t cablecard,
    cablecard_dsg_handler  *dsg_handler
    )
{
    if (cablecard->num_of_dsg_handler >= MAX_DSGTUNNEL_HANDLER) return -1;
    cablecard->dsg_handler[cablecard->num_of_dsg_handler++] = *dsg_handler;
    return 0;
}

unsigned char BcmSendDSGTunnelDataToHost(
    unsigned char *pBufData,
    unsigned int pktlen,
    unsigned long client_type,
    unsigned long client_id
    )
{

    cablecard_t cablecard = cablecard_get_instance();
    unsigned short     ether_type, GutsLen;
    unsigned char     *pFirstBuffData = pBufData;
    int i;

    if (cablecard == NULL) return -1;

   /*
     * Only forward these types (0800,0806) to TCPIP all others return.
     */
    ether_type = ((unsigned char *)pFirstBuffData)[12];
    ether_type <<= 8;
    ether_type |= ((unsigned char *)pFirstBuffData)[13];

    /*
     * discard non-IP ethernet packet data
     */
    if (ether_type != 0x0800) {
        printf("err1, ether_type=%04x(%04x)\n", ether_type, 0x0800);
        return -1;
    }

    /*
     * Passed IP checking, now the protocol type is farther in to the packet.
     * Evaluate it to make checking for UDP.
     */
    if (((unsigned char *)pFirstBuffData)[14 + 9] != 17/*IPPROTO_UDP*/) {
        printf("err2, Not UDP packet=%02x(%02x)\n", ((unsigned char *)pFirstBuffData)[14 + 9], 17/*IPPROTO_UDP*/ );
        return -1;
    }

    /*
     * get the UDP packet length, subtract 8 for UDP header
     */
    GutsLen = ((unsigned char *)pFirstBuffData)[14 + 20 + 4];
    GutsLen <<= 8;
    GutsLen |= ((unsigned char *)pFirstBuffData)[14 + 20 + 5];
    pktlen = GutsLen - 8;

    /*
     * offset to start of UDP data
     */
    pBufData = pFirstBuffData + 14 + 20 + 8;

    /* we have NOT seen any HE to send out new Broadcasting tunnels with BT header*/
    if (getenv("BT_HEADER") != NULL) {
        if (client_type == DSG_CLIENT_TYPE_BROADCAST) {
        /* TODO:: process BT header. Skip now*/
            pBufData += 4;
            pktlen -= 4;
        }
    }

    if (pktlen <= 0) {
        printf(" error in DSG tunnel data %d\n", pktlen);
        return -1;
    }

    /* first dispatch the DSG tunnel to registered external handlers*/
    for (i=0;i<cablecard->num_of_dsg_handler;i++)
    {
        if (cablecard->dsg_handler[i].callback != NULL
            && client_id == cablecard->dsg_handler[i].client_id
            && client_type == cablecard->dsg_handler[i].client_type)
        {
            (*cablecard->dsg_handler[i].callback)(pBufData, pktlen);
            return 0;
        }
    }

    if (client_type == DSG_CLIENT_TYPE_BROADCAST) {
        BDBG_MSG((" Got Broadcast DSG tunnel!\n"));

        switch ( client_id ) {
            case DSG_CLIENT_BROADCAST_TYPE_SCTE65:
                BDBG_MSG((" Got SCTE 65 DSG tunnel! \n"));
                if (cablecard->mpeg_section) {
                    cablecard_lock();
                    cablecard->mpeg_section_len = (pktlen > MAX_MPEG_SECTION_LEN)? MAX_MPEG_SECTION_LEN : pktlen;
                    BKNI_Memcpy(cablecard->mpeg_section, pBufData, cablecard->mpeg_section_len);
                    cablecard_unlock();
                }
                /* This callback will pass SI data (MPEG2 section) to SI parsing library*/
                if (cablecard->si_callback)
                    (*cablecard->si_callback)(cablecard, cablecard->mpeg_section_len);
                break;
            case DSG_CLIENT_BROADCAST_TYPE_SCTE18:
                BDBG_MSG((" Got SCTE 18 DSG tunnel! \n"));
                break;
            case DSG_CLIENT_BROADCAST_TYPE_XAIT_CVT:
                BDBG_MSG((" Got CVT or XAIT DSG tunnel! ID 0x%2x\n", pBufData[0]));
#ifdef CDL_SUPPORT
                /* CVT: table ID is 0xd9; XAIT: table ID is 0x74*/
                /*fixme: how to tell it's CVT or XAIT? */
                /* debug only, use a cvt captured in file */
#if 0
                {
                    int fp;
                    char fn_cvt[128] ="./signed_cvt_capture.bin";
                    if ((fp  = open(fn_cvt, O_RDONLY)) < 0) {
                        BDBG_ERR(("cannot open %s", fn_cvt));
                    } else {
                        pktlen = read(fp, pBufData, pktlen);
                        BDBG_ERR(("cvt pktlen %d bytes", pktlen));
                        close(fp);
                    }
                }
#endif
                if (pBufData[0] == 0xd9) {
                    BDBG_ERR(("pktlen %d", pktlen));
                    cdl_dsg_cvt_process(NULL, pBufData, pktlen);
                }
#endif
                break;
        case DSG_CLIENT_BROADCAST_TYPE_OCCD:
                BDBG_MSG((" Got OCAP Comon Download DSG tunnel!"));
                break;
        case DSG_CLIENT_BROADCAST_TYPE_OBJECT_CAROUSEL:
                BDBG_MSG((" Got Object Carousel DSG tunnel!"));
                break;
            default:
                BDBG_MSG((" Got unknown type Broadcast tunnel!"));
                break;
        }
    } else if (client_type ==  DSG_CLIENT_TYPE_APPLICATION ){
        BDBG_MSG((" Got Application type DSG tunnel!\n"));
    } else if (client_type ==  DSG_CLIENT_TYPE_WELLKNOWN_MACADDR){
        BDBG_MSG((" Got Well-Known Macaddr  DSG tunnel!  \n"));
    } else if (client_type ==  DSG_CLIENT_TYPE_CAS){
        BDBG_WRN((" Got CA DSG tunnel! Should NOT be terminated at Host!\n"));
    } else {
        BDBG_WRN((" Got unknown type DSG tunnel!\n"));
    }

    return 0;
}

unsigned char BcmSendDSGTunnelDataToPOD(
    unsigned char *pBufData,
    unsigned int pktlen,
    unsigned long flow_id
    )
{
    unsigned char *ptr=NULL, ret_val;

    BDBG_MSG((" BcmSendDSGTunnelDataToPOD len %d  flowID 0x%x", pktlen, flow_id));

    if (pktlen>=65535)
    {
        printf("too big %d\n", pktlen);
        return 1; /* Change the return value */
    }

    /* Need to stick 2 bytes of length count infront of the packet per Opencable spec...
     The buffer will be Queued and sent to cablecard in link layer Send thread
     The buffer will be freed in link layer.
     So, have to allocate buffer and copy the entire packet.
     Not the best solution in town, will optimize later.*/
    ptr = (unsigned char*)BKNI_Malloc(pktlen+4);
    if( ptr )
        BKNI_Memcpy(ptr+2, pBufData, pktlen);
    else
        BDBG_ASSERT((0));

    /* add 2 byte length */
    *ptr = pktlen >> 8;
    *(ptr+1) = pktlen & 0xff;

    ret_val = (unsigned char) B_Mpod_LinkH2CSendExtData(flow_id, pktlen+2, ptr);
    if( ret_val != 0 )
        printf("\n!CableCard Not Rdy! RX DSG tunnel packet with MacAddr=0x%08lx_%04x, pktlen=%d\n", *(unsigned long *)(pBufData), *(unsigned short *)(pBufData+4), pktlen );

    /* free(ptr); free at link layer  */

    return ret_val;

}

/*for IP-U and socket flow*/
unsigned char BcmSendDataToPOD(
    unsigned char *pBufData,
    unsigned int pktlen,
    unsigned long flow_id
    )
{

    unsigned char *ptr=NULL, ret_val;

    if (pktlen>=65535)
    {
        printf("too big %d\n", pktlen);
        return 1;
    }

    BDBG_MSG((" BcmSendDataToPOD len %d  flowID 0x%x", pktlen, flow_id));

/*  // Need to have a check here if the CableCard is present/absent,
    // if cablecard is not present, no need to waste time copy packet, just return
    // if( CableCard is not present )
    // 	return POD_NOT_READY;

    // No Need to stick 2 bytes of length count infront of the packet per Opencable spec...
    // The buffer will be Queued and sent to cablecard in link layer Send thread
    // The buffer will be freed in link layer.
    // So, have to allocate buffer and copy the entire packet.
    // Not the best solution in town, will optimize later.*/
    ptr =(unsigned char*)BKNI_Malloc(pktlen);
    if( ptr )
        BKNI_Memcpy (ptr, pBufData, pktlen);
    else
        return -1;

    /*printf("************** BcmSendDataToPOD **************\n"); */

    ret_val = (unsigned char) B_Mpod_LinkH2CSendExtData(flow_id, pktlen, ptr);
    if( ret_val )
        printf("\n!CableCard Not Rdy! RX DSG tunnel packet with MacAddr=0x%08lx_%04x, pktlen=%d\n", *(unsigned long *)(pBufData), *(unsigned short *)(pBufData+4), pktlen );

    /* free(ptr); free at link layer  */
    return ret_val;

}

void POD_Api_Lost_Flow_Ind(
    unsigned long id,
    unsigned char status
    )
{
    printf("\nsent to Cablecard: lost_flow_ind id=0x%x, status=%d!\n", (unsigned int)id, (int)status);
    B_Mpod_ExtChLostFlow(id, (B_MPOD_EXT_LOST_FLOW_REAS)status);
}

void POD_Api_Send_DCD_Info(
    void *dcd_ptr,
    unsigned short dcd_len
    )
{
    /* skip first three bytes to comform latest CCIF spec*/
    /* TODO:: wait for DSG-CC final fix*/
    if (info.extSession > 4)
    {
        B_Mpod_DsgSendDcdInfo((uint8_t*)dcd_ptr, (uint32_t)dcd_len);
    } else 	if (info.extSession) {
        B_Mpod_ExtChSendDcdInfo((uint8_t*)dcd_ptr, (uint32_t)dcd_len);
    } else
        BDBG_ERR((" both DSG and extended channel have NOT opened yet"));
}

void POD_Api_Send_DSG_Message(
    void *dsg_message_ptr,
    unsigned short dsg_message_len
    )
{
    if (info.extSession > 4)
    {
        B_Mpod_DsgSendDSGMessage((uint8_t *)dsg_message_ptr,(uint32_t)dsg_message_len);
    } else if (info.extSession) {
        B_Mpod_ExtChSendDSGMessage((uint8_t *)dsg_message_ptr,(uint32_t)dsg_message_len);
    } else
        BDBG_ERR((" both DSG and extended channel have NOT opened yet"));
}

/* For the DSG and Ext Channel Settings */
void CableCardCallback_DSG_Packet_Error(
    uint8_t *data,
    uint32_t len
    )
{
    BSTD_UNUSED(len);
    CableCardCallbackDSGPacketError((unsigned char)*data);
}

void CableCardCallback_ExtSet_DSG_Mode(
    uint8_t *data,
    uint32_t len
    )
{
    info.mode = (B_MPOD_EXT_OP_MODE)data[0];
    info.extSession = 4;

    /* HPNX pro send us basic DSG mode without mac_addr*/
    if ( (data[0] == B_MPOD_EXT_DSG || data[0] == B_MPOD_EXT_DSG_ONE_WAY) && len == 1) return;

    /* if switch to DSG mode, we need to switch docsis Upstream mode*/
    if ( data[0] != B_MPOD_EXT_OOB) OOB_Tx_Docsis();

    CableCardCallbackSetDSGMode((unsigned char *)data, (unsigned short)len);
}

void CableCardCallback_DSGSet_DSG_Mode(
    uint8_t *data,
    uint32_t len
    )
{
    info.mode = (B_MPOD_EXT_OP_MODE)data[0];
    info.extSession = 5;

    /* HPNX pro send us basic DSG mode without mac_addr*/
    if ( (data[0] == B_MPOD_EXT_DSG || data[0] == B_MPOD_EXT_DSG_ONE_WAY) && len == 1) return;

    /* if switch to DSG mode, we need to switch docsis Upstream mode*/
    if ( data[0] != B_MPOD_EXT_OOB) OOB_Tx_Docsis();

    CableCardCallbackSetDSGMode((unsigned char *)data, (unsigned short)len);
}

void CableCardCallback_DSG_Directory(
    uint8_t *data,
    uint32_t len
    )
{
    int vct_id_included, vct_id = 0;
    unsigned char *ptr = data;

    /* get VCT id if possible*/
    vct_id_included = (*ptr++ & 0x01);
    if (vct_id_included) {
        vct_id = (data[len-2]<<8)|data[len-1];
        BDBG_MSG((" VCI_ID included in DSG_directory %d", vct_id));
        info.vctId = vct_id;
        /*TODO:: notify application for SI data filtering*/
    }
    CableCardCallbackDSG_Directory((unsigned char *)data, (unsigned short)len);
}

void CableCardCallback_IPU_Dhcp(
    uint32_t flowId,
    uint8_t *data,
    uint32_t len
    )
{
    unsigned char status = 0, flowtype = 0x00, flags = 1;
    unsigned short max_pdu_size = 1500;
    unsigned long ipaddr = 0xc0a80001;
    unsigned char *ropt_data = NULL;
    int ropt_len = 128;	/*Guessing: max return option length*/
    uint8_t *a = &(data[1]);
    unsigned int optLen = data[7];
    unsigned char *optBytes = &(data[8]);

    BDBG_MSG(("MacAddr: %02x:%02x:%02x:%02x:%02x:%02x, option length: %d",
                  a[0],a[1],a[2],a[3],a[4],a[5], optLen));

    memcpy(info.macaddr, a, 6);
    if( len == (optLen + 8) && optLen && optBytes )
    {
        printf("\n dhcp option field (hex):");
        for(ipaddr=0; ipaddr<optLen; ipaddr++ )
        {
            if( (ipaddr % 32) == 0 )
                printf("\n");
            printf(" %02x", optBytes[ipaddr] );
        }
        printf("\n");
    }
    else {
        optLen = 0;
        printf(" No dhcp option field!");
    }

    ropt_data = (unsigned char *)malloc( ropt_len );
    if( ropt_data == NULL )
    {
        printf("Error! Cannot allocate space for dhcp option data\n");
        return;
    }

    /*Call Host to send DHCP and WAIT for the response*/
    ipaddr = CableCardCallbackIPUDhcp( flowId, a, optLen,
                                        optBytes, ropt_data, &ropt_len );

    ipaddr = htonl(ipaddr);
    printf("DHCP reply with IPAddress 0x%08lx with option-field data len=%d\n\n", ipaddr, ropt_len);

    /* Network is unavailable*/
    if (ipaddr == 0) {
        status =  B_MPOD_EXT_DEL_FLOW_NETUNAVAIL;
        ropt_len = 0;
    }
    memcpy(info.ipaddr, &ipaddr, 4); /*TODO:: ipv6*/

    B_Mpod_ExtChIpUnicastFlowCnf( flowId, (B_MPOD_EXT_NEW_FLOW_CNF_STAT)status, (unsigned char *)&ipaddr, flowtype, flags, max_pdu_size, ropt_len, ropt_data);

    /*When this function return, the ropt_data pointer can be freed.*/

    free(ropt_data);
}

B_MPOD_EXT_NEW_FLOW_CNF_STAT CableCardCallback_Req_Ipm_Flow(
    uint32_t flowId,
    uint32_t mgid
    )
{
    BSTD_UNUSED(flowId);
    BSTD_UNUSED(mgid);
    return 0;
}

B_MPOD_EXT_NEW_FLOW_CNF_STAT CableCardCallback_DSG_Flow_Id(
    uint32_t flowId
    )
{
    CableCardCallbackDSGFlowID(flowId); /* For DSG flow */
    return B_MPOD_EXT_NEW_FLOW_GRANTED;
}

void CableCardCallback_Socket_Flow_Config(
    uint32_t flowId,
    uint8_t *opt_data,
    uint32_t opt_len
    )
{
    unsigned short max_pdu_size = 1500;
    unsigned char status;

    status = CableCardCallbackSocketFlowConfig(flowId, opt_data, opt_len);
    BDBG_MSG(("-----New_flow_req type Socket: id=%d, opt_len=%d", flowId, opt_len));

    B_Mpod_ExtChSocketFlowCnf(flowId, (B_MPOD_EXT_NEW_FLOW_CNF_STAT)status, max_pdu_size, opt_len, opt_data);

}

static void CableCardCallback_Flow_Req_Failed(
    B_MPOD_EXT_SERV_TYPE serviceType,
    B_MPOD_EXT_NEW_FLOW_CNF_STAT status
    )
{
    BSTD_UNUSED(serviceType);
    BSTD_UNUSED(status);
}

static void CableCardCallback_New_Flow_Cnf_Cb(
    uint32_t flowId,
    B_MPOD_EXT_SERV_TYPE serviceType,
    uint16_t pid
    )
{
    BDBG_MSG(("received %s flowId=%d serviceType=%d pid=%d\n", BSTD_FUNCTION, flowId, (uint32_t) serviceType, pid));
}

B_MPOD_EXT_DEL_FLOW_CNF_STAT CableCardCallback_Delete_Flow_Req(
    uint32_t flowId,
    B_MPOD_EXT_SERV_TYPE serviceType
    )
{
    if (serviceType == B_MPOD_EXT_FLOW_IP_U || serviceType == 	B_MPOD_EXT_FLOW_IP_M || serviceType == B_MPOD_EXT_FLOW_SOCKET)
    {
        CableCardCallbackDeleteFlowReq(flowId);
        return B_MPOD_EXT_DEL_FLOW_GRANTED;
    } else
        return B_MPOD_EXT_DEL_FLOW_UNAUTH;
}

static void CableCardCallback_Del_Flow_Cnf_Cb(
    uint32_t flowId,
    B_MPOD_EXT_SERV_TYPE serviceType
    )
{
    BSTD_UNUSED(flowId);
    BSTD_UNUSED(serviceType);
}

static void CableCardCallback_Lost_Flow_Cb(
    uint32_t flowId,
    B_MPOD_EXT_SERV_TYPE serviceType,
    B_MPOD_EXT_LOST_FLOW_REAS reason
    )
{
    BSTD_UNUSED(flowId);
    BSTD_UNUSED(serviceType);
    BSTD_UNUSED(reason);
}

void CableCardCallback_Config_Advanced_DSG(
    uint8_t *data,
    uint32_t len
    )
{
    CableCardCallbackConfig_Advanced_DSG(data, len);
}

void CableCardCallback_Rcv_Flow_Data(
    uint32_t flowId,
    B_MPOD_EXT_SERV_TYPE serviceType,
    uint8_t *data,
    uint32_t len
    )
{
    if(serviceType == B_MPOD_EXT_FLOW_IP_U) {
        CableCardCallbackSendIPUData(data, len );
    }
    else if(serviceType == B_MPOD_EXT_FLOW_SOCKET) {
        CableCardCallbackSendSocketFlowUsData( flowId, data, len );
    }
    else if (serviceType == B_MPOD_EXT_FLOW_MPEG) {
        cablecard_t cablecard = cablecard_get_instance();

        if (cablecard->mpeg_section) {
            cablecard_lock();
            cablecard->mpeg_section_len = (len  > MAX_MPEG_SECTION_LEN)? MAX_MPEG_SECTION_LEN : len;
            BKNI_Memcpy(cablecard->mpeg_section, data, cablecard->mpeg_section_len);
            cablecard_unlock();
        }
        /* This callback will notify SI parser thread to process new SI data*/
        if (cablecard->si_callback)
            (*cablecard->si_callback)(cablecard, cablecard->mpeg_section_len);
    }
    else {
        /* do nothing */
        BDBG_WRN((" unhandled service type flow data %d", serviceType ));
    }
}

static void apInfoInfo_Changed_Cb(
    void
    )
{
    int id;
    cablecard_t cablecard = cablecard_get_instance();
    cablecard_setting s;

    BDBG_MSG(("apInfoInfoRdyCb Callback Called\n"));
    B_Mpod_AppInfoGetManuID(&info.vendorId);
    id = ((info.vendorId>>8)&0xff);
    B_Mpod_AppInfoGetVersionNum(&info.version);
    B_Mpod_AppInfoGetSerialNum(info.serialNum, &info.serialLen);
    B_Mpod_AppInfoGetMacAddr(info.macaddr);
    if (info.serialLen == 0)
    {
        memset(info.serialNum, 0, sizeof(info.serialNum));
        memset(info.macaddr, 0, sizeof(info.macaddr));
    }
    /* set correct OOB upstream mode according to card vendor ID*/
    BDBG_MSG((" CableCARD manufacture_id 0x%x",info.vendorId));
    /* use symbolrate to automatically set correct OOB mode*/
    cablecard_get(cablecard, &s);
    s.us_mode = (id == 0) ? NEXUS_FrontendUpstreamMode_ePodDvs178: NEXUS_FrontendUpstreamMode_ePodDvs167;
    s.oob_mode = (id == 0) ? NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk: NEXUS_FrontendOutOfBandMode_ePod_Dvs167Qpsk;
    BDBG_WRN((" set OOB mode to %s", ((id==0)? "DVS178" : "DVS167")));
    cablecard_set(cablecard,&s);

}

static uint32_t dialogs[8];
#define MARK_AS_VALID(number)   (dialogs[(number) >> 5] |=  (1 << ((number) & 0x1f)))
#define MARK_AS_INVALID(number) (dialogs[(number) >> 5] &= ~(1 << ((number) & 0x1f)))
#define CHECK_IF_VALID(number)  (dialogs[(number) >> 5] &   (1 << ((number) & 0x1f)))

typedef struct cablecard_message_t {
    char diag_message[NUM_CABLECARD_LINES][NUM_CABLECARD_CHARS]; /* for the app info recieved when we send the URLs */
    unsigned int diag_message_filled; 	/* no: of lines on the page filled */
}cablecard_message ;

static cablecard_message cablecard_page;

static void page_cleanup(
    void
    )
{
    int j=0;
    for(j=0;j<NUM_CABLECARD_LINES;j++) {
        memset(cablecard_page.diag_message[j], 0, NUM_CABLECARD_CHARS);
    }
}

/* No of lines to be updated */
unsigned int get_num_lines_filled(
    void
    )
{
    return cablecard_page.diag_message_filled;
}

char *get_cablecard_info(
    int line_num
    )
{
    return cablecard_page.diag_message[line_num];
}

/* To recieve the various App info thro a callback for the URLs sent  */
void cablecard_receive_app_info_query(
    uint8_t *html,
    uint16_t len,
    uint8_t dialogNb,
    uint8_t fileStatus
    )
{
    int i=0, cnt=0, j=0;
    BSTD_UNUSED(dialogNb);
    BSTD_UNUSED(fileStatus);

    for (i=0;i<NUM_CABLECARD_LINES;i++)
    {
        while (j < NUM_CABLECARD_CHARS-1 && cnt <len)
        {
            if ((html[cnt] != '<') && (html[cnt] != '&') && (html[cnt] != '\n') && (html[cnt] != '\0'))
            {
                cablecard_page.diag_message[i][j++] = html[cnt++];
            } else if (html[cnt] == '<'){
                /* check the next three characters*/
                if(((html[cnt+1] == 'b')||(html[cnt+1] == 'B')) && ((html[cnt+2] == 'r')|| (html[cnt+2] == 'R'))
                    && (html[cnt+3] == '>')){
                    cnt = cnt + 4;
                    cablecard_page.diag_message[i][j] = 0;
                    goto next_line;
                }
                else{
                    while (html[cnt] != '>'){
                        cnt++;
                    }
                    cnt++;
                }
            } else if(html[cnt] == '&') {
                cnt+=6;
            } else
                cnt++;
        }
        if (cnt >=len) break;
next_line:
        j = 0;
    }


    cablecard_page.diag_message_filled = i;
}

int cablecard_get_page(
    int app_num
    )
{
    unsigned short length = 0;
    char url_query[100];
    char *url_ptr;
    uint8_t dialogNb;
    uint8_t urlLen;
    uint8_t max_num_pages;
    MPOD_RET_CODE err;
    cablecard_t cablecard = cablecard_get_instance();

    err = B_Mpod_AppInfoGetNumApps(&max_num_pages);
    if(err || app_num >= max_num_pages ){
        BDBG_ERR(("\n!!!!ERROR retrieving application dialogs\n\n"));
        return -1;
    }

    err = B_Mpod_AppInfoGetURL(app_num, &url_ptr, &urlLen);
    if (err) return -1;
    strcpy(url_query, url_ptr);

    length = strlen(url_query);
    cablecard_page.diag_message_filled = 0;
    cablecard_set_html_callback(cablecard, cablecard_receive_app_info_query);
    err = B_Mpod_MMIHostOpenDialog(url_query, length, &dialogNb);

    return (err? -1 : 0);
}

/* poorman's HTML parser*/
int cablecard_receive_MMI(
    char *mmi_message,
    int len
    )
{
    int length;
    int i = 30;
    int j = 0;
    int cnt;
    char  *new_mmi_message;

    if (len <=0) return -1;
    new_mmi_message = (char *)malloc(len);
    memset(new_mmi_message, 0, len);
    length = len;
    for (cnt=0 ; cnt < (length) ; )
    {
        if (mmi_message[cnt] != '<' && mmi_message[cnt] != '&'
        && (mmi_message[cnt] != '\n') && (mmi_message[cnt] != '\0') )
        {
            new_mmi_message[j++] = mmi_message[cnt++];
        }
        else if (mmi_message[cnt] == '<')
        {
            while (mmi_message[cnt] != '>')
            {
                cnt++;
            }
            cnt++;
            new_mmi_message[j++] = ' ';
        }
        else if (mmi_message[cnt] == '&')
        {
            cnt+=6;
        }
        else
            cnt++;
    }

    while (i <= j)
    {
        if (new_mmi_message[i] == ' ')
        {
            new_mmi_message[i] = '\n';
            i+=30;
        }
        else
        {
            i+=1;
        }
    }
    new_mmi_message[j+1] = '\0';
  free(new_mmi_message);
  return 0;
}

int cablecard_set_html_callback(
    cablecard_t cablecard,
    cablecard_html_callback callback
    )
{
    if (cablecard == NULL) return -1;
    cablecard->html_callback = callback;
    return 0;
}

static void mmiHtmlRdyCb(
    uint8_t *html,
    uint16_t len,
    uint8_t dialogNb,
    bool hostDialog,
    uint8_t fileStatus
    )
{
    BSTD_UNUSED(fileStatus);
    /* add NULL termination */
    html[len - 1] = '\0';
    BDBG_MSG(("mmiHtmlRdyCb Dialog %d diaglog %d len %d", hostDialog, dialogNb, len));
    printf("%s\n",html);
}

static B_MPOD_MMI_OPEN_STATUS mmiDialogRequestCb(
    B_MPOD_MMI_DISPLAY_TYPE displayType,
    uint8_t dialogNb
    )
{
    BDBG_MSG(("mmiDialogRequestCb Callback Called with dialog number %d for display type %d\n", dialogNb, displayType));
    return B_MPOD_MMI_OK;
}

static void mmiDialogCloseCb(
    uint8_t dialogNb
    )
{
    BDBG_MSG(("mmiDialogCloseCb Callback Called for dialog %d\n", dialogNb));
    MARK_AS_INVALID(dialogNb);
}

static void mmiSessionOpenCb(
    uint16_t sessionNb
    )
{
    BSTD_UNUSED(sessionNb);
    BDBG_MSG(("mmiSessionOpenCb Callback Called\n"));
}

static void mmiSessionCloseCb(
    uint16_t sessionNb
    )
{
    BSTD_UNUSED(sessionNb);
    BDBG_MSG(("mmiSessionCloseCb Callback Called\n"));
}

static void dlNewCodeVersionTableType1Cb(
    uint8_t * data,
    uint32_t len,
    B_MPOD_DL_CVT_HOST_RESPONSE *cvtResponse
    )
{
    #ifdef CDL_SUPPORT
		*cvtResponse = (B_MPOD_DL_CVT_HOST_RESPONSE)cdl_adpu_cvt_type1_process(NULL, data, len);
    #else
        BSTD_UNUSED(data);
        BSTD_UNUSED(len);
        BDBG_MSG(("MPOD_TEST-DL: Got new code version table callback"));
        *cvtResponse = B_MPOD_DL_ACT_NO_ERROR;
    #endif
}
    
static void dlNewCodeVersionTableType2Cb(
    uint8_t * data,
	uint32_t len,
    B_MPOD_DL_CVT_HOST_RESPONSE *cvtResponse
    )
{
    #ifdef CDL_SUPPORT
        *cvtResponse = (B_MPOD_DL_CVT_HOST_RESPONSE)cdl_adpu_cvt_type2_process(NULL, data, len);
    #else
        BDBG_MSG(("MPOD_TEST-DL: Got new code version table callback"));
        BSTD_UNUSED(data);
        BSTD_UNUSED(len);
        *cvtResponse = B_MPOD_DL_ACT_NO_ERROR;
    #endif
}

uint8_t descriptorBlock[] =
/* tag length                      data                       */
{
    0,  12,      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
    1,  11,     12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    2,  10,     23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    3,   9,     33, 34, 35, 36, 37, 38, 39, 40, 41,
    4,   8,     42, 43, 44, 45, 46, 47, 48, 49,
    5,   7,     50, 51, 52, 53, 54, 55, 56,
    6,   6,     57, 58, 59, 60, 61, 62,
    7,   5,     63, 64, 65, 66, 67,
    8,   4,     68, 69, 70, 71,
    9,   3,     72, 73, 74,
    10,  2,     75, 76,
    11,  1,     77
};

static void dlHostInfoCb(
    B_MPOD_DL_SUPPORTED_DL_TYPE supportedDlType,
    B_MPOD_DL_HOST_INFO *hostInfo
    )
{
    #ifdef CDL_SUPPORT
        cdl_adpu_get_host_info((void*)supportedDlType, hostInfo);
    #else
        BSTD_UNUSED(supportedDlType);
        printf("MPOD_TEST-DL: Got Host Info callback");
        hostInfo->vendorId = 0x3a3a3a;
        hostInfo->hardwareVersionId = 0x8c8c8c8c;
        hostInfo->numDescriptors = 12;
        hostInfo->descriptors = descriptorBlock;
    #endif
}

static const B_MPOD_FEATURE_ID TestFeatureList[] =
{
    B_MPOD_FEATURE_ID_RF_OUTPUT_CHANNEL,
    B_MPOD_FEATURE_ID_PARENTIAL_CTL_PIN,
    B_MPOD_FEATURE_ID_PARENTIAL_CTL_SETTING,
    B_MPOD_FEATURE_ID_IPPV_PIN,
    B_MPOD_FEATURE_ID_TIME_ZONE,
    B_MPOD_FEATURE_ID_DAYLIGHT_SAVING,
    B_MPOD_FEATURE_ID_AC_OUTLET,
    B_MPOD_FEATURE_ID_LANGUAGE,
    B_MPOD_FEATURE_ID_RATING_REGION,
    B_MPOD_FEATURE_ID_RESET_PINS,
    B_MPOD_FEATURE_ID_CABLE_URL,
    B_MPOD_FEATURE_ID_EAS_LOCATION_CODE,
    B_MPOD_FEATURE_ID_VCT_ID,
    B_MPOD_FEATURE_ID_TURN_ON_CHANNEL,
    B_MPOD_FEATURE_ID_TERMINAL_ASSOC,
    B_MPOD_FEATURE_ID_DOWNLOAD_GRP_ID,
    B_MPOD_FEATURE_ID_ZIP_CODE
};

static void featureReqHostListCb(
    B_MPOD_FEATURE_ID *hostFeatures,
    uint8_t *hostNumFeatures
    )
{
    *hostNumFeatures = sizeof(TestFeatureList)/sizeof(TestFeatureList[0]);
    BKNI_Memcpy(hostFeatures, TestFeatureList, sizeof(TestFeatureList));
}

static B_MPOD_FEATURE_PARAM TestParams[B_MPOD_FEATURE_ID_MAX]; /* adjust for 0 based below */
static B_MPOD_FEATURE_PARAM *RfOutput          = &TestParams[B_MPOD_FEATURE_ID_RF_OUTPUT_CHANNEL-1];
static B_MPOD_FEATURE_PARAM *ParentalPin       = &TestParams[B_MPOD_FEATURE_ID_PARENTIAL_CTL_PIN-1];
static B_MPOD_FEATURE_PARAM *ParentalSettings  = &TestParams[B_MPOD_FEATURE_ID_PARENTIAL_CTL_SETTING-1];
static B_MPOD_FEATURE_PARAM *PurchasePin       = &TestParams[B_MPOD_FEATURE_ID_IPPV_PIN-1];
static B_MPOD_FEATURE_PARAM *TimeZone          = &TestParams[B_MPOD_FEATURE_ID_TIME_ZONE-1];
static B_MPOD_FEATURE_PARAM *DaylightSavings   = &TestParams[B_MPOD_FEATURE_ID_DAYLIGHT_SAVING-1];
static B_MPOD_FEATURE_PARAM *AcOutlet          = &TestParams[B_MPOD_FEATURE_ID_AC_OUTLET-1];
static B_MPOD_FEATURE_PARAM *Language          = &TestParams[B_MPOD_FEATURE_ID_LANGUAGE-1];
static B_MPOD_FEATURE_PARAM *RatingRegion      = &TestParams[B_MPOD_FEATURE_ID_RATING_REGION-1];
static B_MPOD_FEATURE_PARAM *ResetPin          = &TestParams[B_MPOD_FEATURE_ID_RESET_PINS-1];
static B_MPOD_FEATURE_PARAM *CableUrls         = &TestParams[B_MPOD_FEATURE_ID_CABLE_URL-1];
static B_MPOD_FEATURE_PARAM *EmergencyAlertLoc = &TestParams[B_MPOD_FEATURE_ID_EAS_LOCATION_CODE-1];
static B_MPOD_FEATURE_PARAM *VirtualChannel    = &TestParams[B_MPOD_FEATURE_ID_VCT_ID-1];
static B_MPOD_FEATURE_PARAM *TurnOnChan        = &TestParams[B_MPOD_FEATURE_ID_TURN_ON_CHANNEL-1];
static B_MPOD_FEATURE_PARAM *TerminalAssoc     = &TestParams[B_MPOD_FEATURE_ID_TERMINAL_ASSOC-1];
static B_MPOD_FEATURE_PARAM *CommonDownload    = &TestParams[B_MPOD_FEATURE_ID_DOWNLOAD_GRP_ID-1];
static B_MPOD_FEATURE_PARAM *ZipCode           = &TestParams[B_MPOD_FEATURE_ID_ZIP_CODE-1];

static B_MPOD_FEATURE_VIRTUAL_CHANNEL VirtualChannels[10];
static B_MPOD_FEATURE_CABLE_URL Urls[3];

static void featureReqParamsCb(
    B_MPOD_FEATURE_PARAM **featParams,
    uint8_t *numFeatures
    )
{
    uint32_t i;
#ifdef ESTB_CFG_SUPPORT
    int len;
#endif
    *featParams = TestParams;
    *numFeatures = 17;

    /* set up the ID's */
    for (i = 0; i < (int)B_MPOD_FEATURE_ID_MAX; i++) TestParams[i].feature = (B_MPOD_FEATURE_ID)(i+1);
    RfOutput->param.rfOutput.channel = 0x3;
    RfOutput->param.rfOutput.channelUi = B_MPOD_FEATURE_ENABLE_RF_CH_UI;

    ParentalPin->param.parentalPin.chr = "09080706";
    ParentalPin->param.parentalPin.length = 8;

    ParentalSettings->param.parentalSettings.factoryReset = 0;
    ParentalSettings->param.parentalSettings.chanCount = 10;
    ParentalSettings->param.parentalSettings.virtualChannels = VirtualChannels;

    for (i = 0; i < ParentalSettings->param.parentalSettings.chanCount; i++)
    {
        ParentalSettings->param.parentalSettings.virtualChannels[i].channelMajorMinor[2] = i;
        ParentalSettings->param.parentalSettings.virtualChannels[i].channelMajorMinor[1] = i * 5;
        ParentalSettings->param.parentalSettings.virtualChannels[i].channelMajorMinor[0] = i * 10;
    }

    PurchasePin->param.purchasePin.chr = "0504030";
    PurchasePin->param.purchasePin.length = 7;

    TimeZone->param.timeZone.offset = 27;

    DaylightSavings->param.daylightSavings.ctrl = B_MPOD_FEATURE_USE_DST; /* use daylight savings */
    DaylightSavings->param.daylightSavings.delta = 128;
    DaylightSavings->param.daylightSavings.entry = 0xffff0000;
    DaylightSavings->param.daylightSavings.exit =0x0000ffff;

    AcOutlet->param.acOutlet.ctrl = B_MPOD_FEATURE_AC_USER_SETTING;

#ifdef ESTB_CFG_SUPPORT
    B_Estb_cfg_Get_bin("/dyn/estb/lang_code", Language->param.language.ctrl, &len );
#else
    Language->param.language.ctrl[0] = 'e';
    Language->param.language.ctrl[1] = 'n';
    Language->param.language.ctrl[2] = 'g';
#endif

    RatingRegion->param.ratingRegion.region = B_MPOD_FEATURE_REGION_US;

    ResetPin->param.resetPin.ctrl = B_MPOD_FEATURE_RESET_P_C_PURCHASED_PINS;

    CableUrls->param.cableUrls.numberOfUrls = 3;
    CableUrls->param.cableUrls.urls = Urls;

    CableUrls->param.cableUrls.urls[0].length = 23;
    CableUrls->param.cableUrls.urls[0].type = B_MPOD_FEATURE_WEB_PORTAL; /* Web Portal URL */
    CableUrls->param.cableUrls.urls[0].url = "http://www.broadcom.com";

    CableUrls->param.cableUrls.urls[1].length = 24;
    CableUrls->param.cableUrls.urls[1].type = B_MPOD_FEATURE_EPG; /* EPG URL */
    CableUrls->param.cableUrls.urls[1].url = "http://epg.broadcomm.com";

    CableUrls->param.cableUrls.urls[2].length = 25;
    CableUrls->param.cableUrls.urls[2].type = B_MPOD_FEATURE_VOD; /* VOD URL */
    CableUrls->param.cableUrls.urls[2].url = "http://vod.broadcommm.com";

    EmergencyAlertLoc->param.emergencyAlertLoc.stateCode = 101;
    EmergencyAlertLoc->param.emergencyAlertLoc.countySubdivision = 102;
    EmergencyAlertLoc->param.emergencyAlertLoc.countyCode = 103;

    VirtualChannel->param.virtualChannel.vctId = 234;

    TurnOnChan->param.turnOnChan.virtualChannel = 88;
    TurnOnChan->param.turnOnChan.defined  = 1;

    TerminalAssoc->param.terminalAssoc.length = 19;
    TerminalAssoc->param.terminalAssoc.identifier = "This Is My Terminal";

#ifdef ESTB_CFG_SUPPORT
    {
        unsigned int group_id = 0;
        B_Estb_cfg_Get_uint32("/dyn/estb/group_id", &group_id);
        CommonDownload->param.commonDownload.groupId = group_id;
    }
#else
    CommonDownload->param.commonDownload.groupId = 56;
#endif

    ZipCode->param.zipCode.chr = "95118-9446";
    ZipCode->param.zipCode.length = 10;
}

static const char *FeatureString[] =
{
    "dummy", /* features are 1-based */
    "rf_output_channel",
    "parental_control_pin",
    "parental_control_settings",
    "purchase_pin",
    "time_zone",
    "daylight_savings",
    "ac_outlet",
    "language",
    "rating_region",
    "reset_pin",
    "cable_URLs",
    "EAS_location_code",
    "VCT_ID",
    "turn_on_channel",
    "terminal_association",
    "download_group_id",
    "zip_code"
};

static void featureRcvCardListCb(
    B_MPOD_FEATURE_ID *cardFeatures,
    uint8_t cardNumFeatures
    )
{
    uint32_t i;

    printf("%s list of Card supported features\n\n", BSTD_FUNCTION);
    printf("%d\n", cardNumFeatures);
    for(i = 0; i < cardNumFeatures; i++) printf("Feature %d = %s\n", i, FeatureString[cardFeatures[i]]);
    printf("\n\n\n");
}

static void featureRcvParamsCb(
    B_MPOD_FEATURE_PARAM *featureParams,
    uint8_t numFeatures
    )
{
    uint32_t i, j;
#ifdef ESTB_CFG_SUPPORT
    unsigned int group_id;
#endif

    printf("Received the following feature params from the CableCard\n\n");

    for(i = 0; i < numFeatures; i++)
    {
        switch(featureParams[i].feature)
        {
            case B_MPOD_FEATURE_ID_RF_OUTPUT_CHANNEL:
                printf("RF_OUTPUT_CHANNEL\n\n");
                printf("Channel = %d, Channel UI is %s\n",
                featureParams[i].param.rfOutput.channel,
                (featureParams[i].param.rfOutput.channelUi) == 0x1 ? "enabled" : "disabled");
            break;

            case B_MPOD_FEATURE_ID_PARENTIAL_CTL_PIN:
                printf("PARENTIAL_CTL_PIN\n\n");
                featureParams[i].param.parentalPin.chr[featureParams[i].param.parentalPin.length + 1] = '\0';
                printf("Pin is %s\n", featureParams[i].param.parentalPin.chr);
            break;

            case B_MPOD_FEATURE_ID_PARENTIAL_CTL_SETTING:
                printf("PARENTIAL_CTL_SETTING\n\n");
                printf("%s Factory Reset, Channel Count = %d\n",
                (featureParams[i].param.parentalSettings.factoryReset == 0xa7) ? "Perform" : "Don't Perform",
                featureParams[i].param.parentalSettings.chanCount);
                for(j = 0; j < featureParams[i].param.parentalSettings.chanCount; j++)
                {
                    uint16_t major, minor;

                    major = ((featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[0] & 0xf) << 6) |
                            ((featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[1] & 0xfc) >> 2);
                    minor = ((featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[1] & 0x3) << 8) |
                            featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[2];

                    printf("Virtual Channel %d %d included\n", major, minor);
                }
            break;

            case B_MPOD_FEATURE_ID_IPPV_PIN:
                printf("IPPV_PIN\n\n");
                featureParams[i].param.purchasePin.chr[featureParams[i].param.purchasePin.length + 1] = '\0';
                printf("Pin is %s\n", featureParams[i].param.purchasePin.chr);
            break;

            case B_MPOD_FEATURE_ID_TIME_ZONE:
                printf("TIME_ZONE\n\n");
                printf("Time Zone Offset = %d\n", featureParams[i].param.timeZone.offset);
                info.timezoneOffset = featureParams[i].param.timeZone.offset;
            break;

            case B_MPOD_FEATURE_ID_DAYLIGHT_SAVING:
                printf("DAYLIGHT_SAVING\n\n");
                printf("%s use Daylight Savings\n",
                    (featureParams[i].param.daylightSavings.ctrl == B_MPOD_FEATURE_USE_DST) ? "Do" : "Don't");
                if(featureParams[i].param.daylightSavings.ctrl == B_MPOD_FEATURE_USE_DST)
                {
                    printf("Delta = %d, Entry = %d, Exit = %d\n",
                    featureParams[i].param.daylightSavings.delta,
                    featureParams[i].param.daylightSavings.entry,
                    featureParams[i].param.daylightSavings.exit);
                    info.DSTimeDelta = featureParams[i].param.daylightSavings.delta;
                    info.DSTimeEntry = featureParams[i].param.daylightSavings.entry;
                    info.DSTimeExit = featureParams[i].param.daylightSavings.exit;
                }
            break;

            case B_MPOD_FEATURE_ID_AC_OUTLET:
            {
                char *ACOutletStrings[] = {"Use User Setting",
                                           "Switched AC Outlet",
                                           "Unswitched AC Outlet",
                                           "Reserved"};
                printf("AC_OUTLET\n\n");
                printf("AC Outlet setting %s\n", ACOutletStrings[featureParams[i].param.acOutlet.ctrl & 0x3]);
            }
            break;

            case B_MPOD_FEATURE_ID_LANGUAGE:
                printf("LANGUAGE\n\n");
                printf("Language code is %d %d %d\n",
                featureParams[i].param.language.ctrl[0],
                featureParams[i].param.language.ctrl[1],
                featureParams[i].param.language.ctrl[2]);
            break;

            case B_MPOD_FEATURE_ID_RATING_REGION:
                printf("RATING_REGION\n\n");
                printf("Rating Region is %d\n",
                featureParams[i].param.ratingRegion.region);
            break;

            case B_MPOD_FEATURE_ID_RESET_PINS:
            {
                char * resetPinsString[] = {"Don't reset any pin",
                                            "Reset parental control pin",
                                            "Reset purchase pin",
                                            "Reset parental control and purchase pin"};
                printf("RESET_PINS\n\n");
                printf("Reset Pin Setting is %s\n",
                resetPinsString[featureParams[i].param.resetPin.ctrl & 0x3]);
            }
            break;

            case B_MPOD_FEATURE_ID_CABLE_URL:
            {
                printf("CABLE_URL\n\n");

                /* populate the array with type, length and the pointer to the url */
                for (j = 0; j < featureParams[i].param.cableUrls.numberOfUrls; j++)
                {
                    char *urlTypeString[] = {"undefined", "Web Portal URL", "EPG URL", "VOD URL"};

                    featureParams[i].param.cableUrls.urls[j].url[featureParams[i].param.cableUrls.urls[j].length + 1] = '\0';

                    printf("Type %s, URL = %s\n",
                    urlTypeString[featureParams[i].param.cableUrls.urls[j].type & 0x3],
                    featureParams[i].param.cableUrls.urls[j].url);
                }
            }
            break;

            case B_MPOD_FEATURE_ID_EAS_LOCATION_CODE:
                printf("EAS_LOCATION_CODE\n\n");
                printf("State Code = %d, County Subdvsn = %d, County Code = %d\n",
                featureParams[i].param.emergencyAlertLoc.stateCode,
                featureParams[i].param.emergencyAlertLoc.countySubdivision,
                featureParams[i].param.emergencyAlertLoc.countyCode);
                info.EALocCode[0] = featureParams[i].param.emergencyAlertLoc.stateCode;
                info.EALocCode[1] = (featureParams[i].param.emergencyAlertLoc.countySubdivision<<4)
                                    |(featureParams[i].param.emergencyAlertLoc.countyCode>>8);
                info.EALocCode[2] = featureParams[i].param.emergencyAlertLoc.countyCode&0xff;
            break;

            case B_MPOD_FEATURE_ID_VCT_ID:
                printf("VCT_ID\n\n");
                printf("Virtual Channel ID = %d\n",
                    featureParams[i].param.virtualChannel.vctId);
                info.vctId = featureParams[i].param.virtualChannel.vctId;
            break;

            case B_MPOD_FEATURE_ID_TURN_ON_CHANNEL:
                printf("TURN_ON_CHANNEL\n\n");
                printf("Turn-On Channel %d is %s\n", featureParams[i].param.turnOnChan.virtualChannel,
                featureParams[i].param.turnOnChan.defined ? "defined" : "not defined");
            break;

            case B_MPOD_FEATURE_ID_TERMINAL_ASSOC:
                printf("TERMINAL_ASSOCIATION\n\n");
                featureParams[i].param.terminalAssoc.identifier[featureParams[i].param.terminalAssoc.length + 1] = '\0';
                printf("Terminal Association ID = %s\n", featureParams[i].param.terminalAssoc.identifier);
            break;

            case B_MPOD_FEATURE_ID_DOWNLOAD_GRP_ID:
                printf("DOWNLOAD_GROUP_ID\n\n");
                printf("Download Group ID is %d\n", featureParams[i].param.commonDownload.groupId);
#ifdef ESTB_CFG_SUPPORT
                group_id = featureParams[i].param.commonDownload.groupId;
                B_Estb_cfg_Set_uint32("/dyn/estb/group_id", group_id);
#endif
            break;

            case B_MPOD_FEATURE_ID_ZIP_CODE:
                printf("ZIP_CODE\n\n");
                featureParams[i].param.zipCode.chr[featureParams[i].param.zipCode.length + 1] = '\0';
                printf("Zip Code is %s\n", featureParams[i].param.zipCode.chr);
            break;

            default:
                printf("ERROR: Invalid feature selected \n\n");
            break;
        }

        featureParams[i].featureStatus = 0; /* feature param accepted */

        printf("\n\n\n");

    }
}

static void featureParamDeniedCb(
    B_MPOD_FEATURE_ID feature,
    B_MPOD_FEATURE_STATUS status
    )
{
    BSTD_UNUSED(feature);
    BSTD_UNUSED(status);

    printf("Param %s denied with status %d",FeatureString[feature], status );
}

static B_MPOD_HC_IB_TUNE_STATUS inbandTuneCb(
    uint32_t freqHz,
    B_MPOD_HC_IB_MOD_TYPE modulation,
    uint8_t *ltsid
    )
{
    BSTD_UNUSED(freqHz);
    BSTD_UNUSED(modulation);
    BSTD_UNUSED(ltsid);

    return B_MPOD_HC_IB_TUNE_ACCEPTED;
}

static B_MPOD_HC_OOB_TX_TUNE_STATUS oobTxTuneCb(
    uint32_t freqHz,
    uint32_t powerLevel,
    uint32_t rate
    )
{
#if 1/*!(VMS93380_SUPPORT || SMS93380_SUPPORT)*/
    NEXUS_FrontendUpstreamSettings setting;
    NEXUS_Error rc;
    NEXUS_PlatformConfiguration platformConfig;
    cablecard_t cablecard = cablecard_get_instance();

    NEXUS_Frontend_GetDefaultUpstreamSettings(&setting);
    setting.frequency = freqHz;
    setting.mode = (freqHz==0|| rate==0) ? NEXUS_FrontendUpstreamMode_eDocsis : cablecard->us_mode;
    setting.powerLevel = powerLevel*50; /* in hundredth of dBmV*/
    setting.symbolRate = rate;

    BDBG_MSG(("Freq: %dHz  PowerLevel: %d.%ddBmV  Rate: %dbaud\n",
            freqHz, powerLevel >> 1, (powerLevel & 0x1) ? 5 : 0, rate));

    NEXUS_Platform_GetConfiguration(&platformConfig);
    rc = NEXUS_Frontend_TuneUpstream(platformConfig.frontend[MAX_TUNER], &setting);

    return (rc)?  B_MPOD_HC_OOB_TX_OTHER_ERROR : B_MPOD_HC_OOB_TX_TUNE_GRANTED ;
#else
    BSTD_UNUSED(freqHz);
    BSTD_UNUSED(powerLevel);
    BSTD_UNUSED(rate);
    
    /* still waiting for OOB US support */
    BKNI_Sleep(500);
    return B_MPOD_HC_OOB_TX_TUNE_GRANTED ;
#endif
}

/* switch frontend OOB TX mode to Docsis UPstream mode*/
static void OOB_Tx_Docsis(
    void
    )
{
    oobTxTuneCb(0, 0, 0);
}

static B_MPOD_HC_OOB_RX_TUNE_STATUS oobRxTuneCb(
    uint32_t freqHz,
    uint32_t rate,
    bool spectralInv
    )
{
    int  rc;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendOutOfBandSettings oobSettings;
    cablecard_t cablecard = cablecard_get_instance();

    if (rate == 1024000)
    {
        cablecard->us_mode = NEXUS_FrontendUpstreamMode_ePodDvs178;
        cablecard->oob_mode = NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk;
        BDBG_MSG(("Set to DVS178 OOB mode"));
    } else if (rate == 1544000/2 || rate == 3088000/2)
    {
        cablecard->us_mode = NEXUS_FrontendUpstreamMode_ePodDvs167;
        cablecard->oob_mode = NEXUS_FrontendOutOfBandMode_ePod_Dvs167Qpsk;
        BDBG_MSG(("Set to DVS167 OOB mode"));
    } else
        BDBG_WRN((" wrong OOB symbolrate!"));

    BDBG_MSG(("Freq: %dHz  Rate: %dbps  Spectral Inversion: %s\n",
            freqHz, rate, spectralInv ? "true" : "false"));
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Frontend_GetDefaultOutOfBandSettings(&oobSettings);
    oobSettings.mode = cablecard->oob_mode;
    oobSettings.frequency = freqHz;
    oobSettings.spectrum =  (spectralInv) ?
        NEXUS_FrontendOutOfBandSpectrum_eInverted : NEXUS_FrontendOutOfBandSpectrum_eNonInverted;
    oobSettings.symbolRate = rate;
    oobSettings.lockCallback.callback = default_lock_callback;
    oobSettings.lockCallback.context = platformConfig.frontend[MAX_TUNER];

    rc = NEXUS_Frontend_TuneOutOfBand(platformConfig.frontend[MAX_TUNER], &oobSettings);
    NEXUS_StartCallbacks(platformConfig.frontend[MAX_TUNER]);

   	BKNI_Sleep(500);
    return (rc) ?  B_MPOD_HC_OOB_RX_OTHER_ERROR : B_MPOD_HC_OOB_RX_TUNE_GRANTED ;
}

static void sourceIdToFreqCb(
    uint16_t sourceId,
    uint32_t *freqHz,
    B_MPOD_HC_IB_MOD_TYPE *modulation
    )
{
    BSTD_UNUSED(sourceId);
    BSTD_UNUSED(freqHz);
    BSTD_UNUSED(modulation);
}

static void delayedDownloadReqCb(
    void
    )
{
    printf("received %s\n", BSTD_FUNCTION);
}

static void homingCompleteCb(
    void
    )
{
    printf("received %s\n", BSTD_FUNCTION);
}

/* the host should NOT interrupt the download */
static void downloadStartingCb(
    B_MPOD_HOMING_DOWNLOAD_INFO *downloadInfo
    )
{
    char notifyString[257];
    char *sourceStrings[] = {"unknown", "QAM Inband", "QPSK OOB", "reserved"};
    char *timeoutTypeStrings[] = {"both timeouts", "transport timeout",
                                            "download timeout", "no_timeout"};

    printf("received %s\n", BSTD_FUNCTION);

    if(downloadInfo->notifyTextLength)
    {
        strncpy(notifyString, downloadInfo->notifyText, downloadInfo->notifyTextLength);
        notifyString[downloadInfo->notifyTextLength] = '\0';
        printf("Notify Message: %s\n", notifyString);
    }

    printf("Upgrade Source: %s,  Download Time %d\n",
            sourceStrings[downloadInfo->source & 0x3],
            downloadInfo->downloadTime);

    printf("Timeout Type: %s,  Timeout Period: %d\n",
            timeoutTypeStrings[downloadInfo->timeoutType & 0x3],
            downloadInfo->downloadTimeoutPeriod);
}

static void downloadCompleteCb(
    B_MPOD_HOMING_UPGRADE_RESET_REQUEST resetType
    )
{
    char *resetTypeStrings[] = {"PCMCIA Reset", "Card Reset", "No Reset", "Reserved"};

    printf("received %s\n", BSTD_FUNCTION);
    printf("requested %s\n", resetTypeStrings[resetType & 0x3]);
}

static void homingTimeoutCb(
    void
    )
{
    printf("received %s\n", BSTD_FUNCTION);
    printf("Resetting CableCard\n");
}

/* tells the app that the requested connection for privateAppId has been established */
static void sasConnectCnfCallback(
    uint8_t *privateAppId,
    B_MPOD_SAS_HANDLE newSasConnection
    )
{
    int i;

    for(i = 0; i < 32; i++)
    {
        if(SasConnections[i] == 0)
        {
            SasConnections[i] = newSasConnection;
            printf("SAS connection %d assigned for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    i, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
                    privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]);
            return;
        }
    }

    printf("Unable to open new SAS Connection. Connection limit has been reached\n");
}

/* tells the app that the card is ready for syncrhonous communication for this privateAppId */
static void sasConnectionRdyCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId
    )
{

    printf("Recieved connection rdy for connection %p assigned for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
        (void *)sasConnection, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    );
}

/* delivers data to the application from the card */
static void sasSynchDataRcvCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId,
    uint8_t *msg,
    uint32_t len
    )
{
    uint32_t i = 0;
    BSTD_UNUSED(sasConnection);

    printf("Received new data through synchronous transmission from private app %02x %02x %02x %02x %02x %02x %02x %02x\n",
        privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    );

    printf("msg: \n");
    while(len--) printf("%x ", msg[i++]);
    printf("\n");
}

/* delivers data to the application from the card */
static void sasAsynchDataRcvCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId,
    uint8_t *msg,
    uint32_t len)
{
    uint32_t i = 0;
    BSTD_UNUSED(sasConnection);

    printf("Received new data through asynchronous transmission from private app %02x %02x %02x %02x %02x %02x %02x %02x\n",
        privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    );

    printf("msg: \n");
    while(len--) printf("%x ", msg[i++]);
    printf("\n");

}

/* retrieves data from the app to be sent to the card (app previously requested a syncrhonous transfer of data) */
static void sasGetSynchDataCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId,
    uint8_t transactionNb,
    uint8_t **sasMsg,
    uint32_t *sasLen
    )
{
    BSTD_UNUSED(sasConnection);
    
    printf("Received request for data for transaction number %d for private app %02x %02x %02x %02x %02x %02x %02x %02x\n",
        transactionNb, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    );

    /* Preliminary handshake is over, allow the data to be transmitted */
    *sasMsg = SasData;
    *sasLen = 8;
}

/* tells the app that the card has closed the connection for privateAppId */
static void sasConnectionClosedCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId
    )
{
    int i;

    for(i = 0; i < 32; i++)
    {
        if(SasConnections[i] == sasConnection)
        {
            SasConnections[i] = 0;
            printf("Closing SAS connection %p assigned for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    (void *)sasConnection, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
                    privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
            );
            return;
        }
    }

    printf("Unable to close SAS Connection. Connection not found\n");
}

void diagReqCb(
    uint8_t numDiags,
    B_MPOD_DIAG_DATA *diagReqests
    )
{
    uint8_t i;

    for(i = 0; i < numDiags; i++)
    {
        BDBG_MSG((" NUM: %d  Diag_id %d", i, diagReqests[i].id));
        diagReqests[i].status = B_MPOD_DIAG_DENIED_OTHER;
    }
}

void caInfoCb(
    uint16_t *caSystemId,
    uint32_t numSystemId
    )
{
    uint32_t i;
    for(i = 0; i < numSystemId; i++)
        printf("CA System Id [%d]: %x\n", i, caSystemId[i]);
    if (numSystemId) info.CAId = caSystemId[0];
    info.CPId = CP_SYSTEM2_ID;
}

void caPmtUpdateReply(
    B_MPOD_CA_PMT_UPDATE_INFO *replyInfo
    )
{
    uint32_t i;
    uint16_t pid;
    bool caEnableFlag;
    int caEnable;

    printf("\nProg Idx: %d  Prog Num: %d, Src Id: %d\n",
            replyInfo->progIndex, replyInfo->progNum, replyInfo->srcId);
    printf("Trans Id: %d  LTSID: %d\n", replyInfo->transId, replyInfo->ltsid);
    if(replyInfo->caEnaFlag)
        printf("Program Level CA Enable: %d\n", replyInfo->caEna);
    else
        printf("No Program Level CA Enable\n");

    for(i = 0; i < replyInfo->numElem; i++)
    {
        B_MPOD_CA_GET_PMT_ES_INFO(replyInfo->esInfo, i, pid, caEnableFlag, caEnable);
        if(caEnableFlag)
            printf("Es Level CA Enable:  pid:%02x  CA Enable:%d\n", pid, (uint8_t)caEnable);
        else
            printf("No Es Level CA Enable for pid %02x", pid);
    }

}

void caPmtUpdate(
    B_MPOD_CA_PMT_UPDATE_INFO *updateInfo
    )
{
    printf("%s\n", BSTD_FUNCTION);
    caPmtUpdateReply(updateInfo);
}

void caPmtReply(
    B_MPOD_CA_PMT_REPLY_INFO *replyInfo
    )
{
    printf("%s\n", BSTD_FUNCTION);
    caPmtUpdateReply(replyInfo);
}

void getAuthKeyCb(
    uint8_t *authKey,
    bool *authKeyExists
    )
{
    unsigned char hostId[5];
    B_Mpod_TestCpGetAuthKey(authKey, authKeyExists);
    info.authKeyStatus = true;

    if (*authKeyExists)
    {
        B_Mpod_TestCpGetID(hostId, info.cardId);
        BDBG_MSG((" Get host ID and card ID"));
    }
}

void cardAuthMsgCb(
    uint8_t *cardDevCert,
    uint8_t *cardManCert,
    uint8_t *dhPubKeyC,
    uint8_t *signC,
    uint8_t *hostDevCert,
    uint8_t *hostManCert,
    uint8_t *dhPubKeyH,
    uint8_t *signH
    )
{
    int ret;
    printf("Getting Host CP Authentication Parameters\n");
    B_Mpod_TestCpGetHostAuthParams(hostDevCert, hostManCert, dhPubKeyH, signH);
    printf("Checking Card CP Authorization, generating AuthKey\n");
    ret = B_Mpod_TestCpGenAuthKeyH(cardDevCert, cardManCert, dhPubKeyC, signC);
    if (ret&cablecard_cardcert_error)
    {
        info.certCheck = true;
        BDBG_ERR((" card certificate error"));
    }
    if (ret&cablecard_hostId_error)
    {
        BDBG_ERR((" HOST ID error"));
    }
    if (ret&cablecard_cardId_error)
    {
        BDBG_ERR((" card ID error"));
    }
}

void getNonceCb(
    uint8_t *nonce
    )
{
    B_Mpod_TestCpGenerateNonce(nonce);
}

void getIDCb(
    uint8_t * hostId,
    uint8_t * cardId)
{
    B_Mpod_TestCpGetID(hostId, cardId);
    memcpy(info.cardId, cardId, 8);
}

void cpkeyGenCb(
    uint8_t *nHost,
    uint8_t *nCard,
    uint8_t *cpKeyA,
    uint8_t *cpKeyB
    )
{
    printf("Generating New CPKey");

    B_Mpod_TestCpGenCPKey(nHost, nCard, cpKeyA, cpKeyB);
    info.CPkeyCount++;
}

void calcCciAckCb(
    uint8_t cpVersion,
    uint8_t cci,
    uint8_t *cpKeyA,
    uint8_t *cpKeyB,
    uint8_t *cciNCard,
    uint8_t *cciNHost,
    uint16_t programNumber,
    uint8_t ltsid,
    uint16_t ecmpid,
    uint8_t *cciAuth,
    uint8_t *cciAck
    )
{
    printf("Calculating new CP cci ack value\n");
    B_Mpod_TestCpGenCciAck(cpVersion, cci, cpKeyA, cpKeyB, cciNCard,
                cciNHost, programNumber, ltsid, ecmpid, cciAuth, cciAck);
    info.cciCount++;
}

void enforceCciCb(
    uint8_t cci_data, uint16_t prog_num, uint8_t cci_ltsid
    )
{
    BSTD_UNUSED(cci_data);
    BSTD_UNUSED(prog_num);
    BSTD_UNUSED(cci_ltsid);

}

void newValidationStatusCb(
    B_MPOD_CP_VALIDATION_STATUS validationStatus
    )
{

    printf("Received new CP validation status from the card, status is %d\n", validationStatus);
    info.status = validationStatus;
}

void resProfileInfoCb(
    uint8_t numStreams,
    uint8_t numProgs,
    uint8_t numEs
    )
{
   printf("%s streams=%d  progs=%d  elem streams=%d\n", BSTD_FUNCTION, numStreams, numProgs, numEs);
}

static void getRootOID( uint8_t *data, uint32_t len)
{
    uint32_t i;
    printf(" SNMP MIBs root OID\n");
    for (i=0;i<len;i++)
    {
        if (i) printf(".");
        printf("%d", data[i]);
    }
    printf("\n");
    if (info.rootoid) free(info.rootoid);
    info.rootoid = (char*)malloc(len);
    memcpy(info.rootoid, data, len);
    info.rootoidLen = len;
}

static void getSnmpReply( uint8_t *data,    uint32_t len)
{
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);
    /* will be passed back to SNMP proxy
    // by calling MPOD_RET_CODE B_Mpod_SnmpReq(uint8_t *snmpMsg, uint32_t len)*/

}

void
closeNexusPidChannelsCableCard(
    IpStreamerCtx *ipStreamerCtx
    )
{
    cablecard_disable_program(ipStreamerCtx->parserBandPtr->parserBand);
#ifdef MPODXSIX
    /* unsteal i/p parser band pid channels */
    cablecard_unswitch_parser_band
        (ipStreamerCtx->parserBandPtr->parserBand,
         &(ipStreamerCtx->videoPidChannel),
         &(ipStreamerCtx->audioPidChannel),
         &(ipStreamerCtx->pcrPidChannel),
         &(ipStreamerCtx->patPidChannel),
         &(ipStreamerCtx->pmtPidChannel),
         &(ipStreamerCtx->caPidChannel)
        );
    /* don't forget the one we opened for the ca pid */
    if (ipStreamerCtx->caPidChannel)
        NEXUS_PidChannel_Close(ipStreamerCtx->caPidChannel);
#endif
}

int
openNexusPidChannelsCableCard(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    if(ipStreamerCfg->srcType == IpStreamerSrc_eQam) /* Only QAM tuner i/p routed through cablecard */
    {
        uint16_t caPid;
#ifdef MPODXSIX
        /* MCARD only specced for 200mbps (i.e. 5xQAM256 streams). Need to pid filter if we wan to use 6xQAM256 streams
           MCARD i/f fifo cannot handle worst case back to back transport rate for 6xQAM256 streams. Need to send 2 streams
           on roundtrip to memory (rec->pb) for flow control */

        NEXUS_PlaypumpHandle playpumpHandle;
        NEXUS_PidChannelHandle stolenVideoPidChannel, stolenAudioPidChannel, stolenPcrPidChannel,
                               stolenPatPidChannel,   stolenPmtPidChannel,   stolenCaPidChannel;

        /* 2 playback parser bands are reserved for round trip to memory. If one is available then insert
           roundtrip to memory 'behind the scenes' by stealing all the input parser band pid channels
           and replacing with playback parser band pid channels */
        if((playpumpHandle = cablecard_switch_parser_band(ipStreamerCtx->parserBandPtr->parserBand))!= NULL)
        {
            /* steal i/p parser band pid channels... */
            stolenVideoPidChannel = ipStreamerCtx->videoPidChannel;
            stolenAudioPidChannel = ipStreamerCtx->audioPidChannel;
            stolenPcrPidChannel   = ipStreamerCtx->pcrPidChannel;
            stolenPatPidChannel   = ipStreamerCtx->patPidChannel;
            stolenPmtPidChannel   = ipStreamerCtx->pmtPidChannel;

            /* ... replace with new playback parser pid channels */
            ipStreamerCtx->videoPidChannel = ipStreamerCtx->audioPidChannel = ipStreamerCtx->pcrPidChannel = NULL;
            if (psi->videoPid)
                ipStreamerCtx->videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpumpHandle, psi->videoPid, NULL);

            if (psi->audioPid)
                ipStreamerCtx->audioPidChannel = NEXUS_Playpump_OpenPidChannel(playpumpHandle, psi->audioPid, NULL);

            if (psi->pcrPid != psi->videoPid)
                ipStreamerCtx->pcrPidChannel = NEXUS_Playpump_OpenPidChannel(playpumpHandle, psi->pcrPid, NULL);

            ipStreamerCtx->patPidChannel = NEXUS_Playpump_OpenPidChannel(playpumpHandle, 0, NULL);
            ipStreamerCtx->pmtPidChannel = NEXUS_Playpump_OpenPidChannel(playpumpHandle, psi->pmtPid, NULL);
        }
#endif
        /* tell cablecard about the pids and parser bands, retrieve the ca pid */
        cablecard_enable_program(psi->pmtBuffer, psi->pmtBufferSize,
                ipStreamerCtx->parserBandPtr->parserBand, ipStreamerCtx->videoPidChannel,
                ipStreamerCtx->audioPidChannel, ipStreamerCtx->cfg.sourceId, &caPid);

#ifdef MPODXSIX
        /* there may be no CA pid, in which case do not allocate a pid channel for it */
        ipStreamerCtx->caPidChannel = NULL;
        if(caPid)
        {   /* open both the i/p parser band and playback parser band pid channel */
            ipStreamerCtx->caPidChannel = NEXUS_PidChannel_Open(ipStreamerCtx->parserBandPtr->parserBand, caPid, NULL);
            if(playpumpHandle)
            {
                stolenCaPidChannel = ipStreamerCtx->caPidChannel;
                ipStreamerCtx->caPidChannel = NEXUS_Playpump_OpenPidChannel(playpumpHandle, caPid, NULL);
            }
        }

        if(playpumpHandle)
            /* send the i/p parser band pid channels on roundtrip to memory */
            cablecard_add_pid_channels(
                    playpumpHandle,
                    stolenVideoPidChannel,
                    stolenAudioPidChannel,
                    stolenPcrPidChannel,
                    stolenPatPidChannel,
                    stolenPmtPidChannel,
                    stolenCaPidChannel
                    );
#endif
    }
    return 0;
}

/*****************
** Host Properties
*****************/
static void hostPropertiesReplyCb(
    B_MPOD_PROP_HOST_PROPS *hostProperties
    )
{
    int i, j;

    printf("%s\n\n", BSTD_FUNCTION);
    printf("%d properties sent from the card\n\n", hostProperties->numOfProperties);

    for(i = 0; i < hostProperties->numOfProperties; i++)
    {
        printf("Key: ");
        for(j = 0; j < hostProperties->properties[i].keyLength; j++)
            printf("%02x ", hostProperties->properties[i].keyByte[j]);

        printf("= ");
        for(j = 0; j < hostProperties->properties[i].valueLength; j++)
            printf("%02x ", hostProperties->properties[i].valueByte[j]);

        printf("\n");
    }
}

/**************
** Headend Comm
**************/
static void rcvHostResetVectorCb(
    B_MPOD_HEADEND_HOST_RESET_VECTOR *hostResetVector
    )
{
    printf("%s\n\n", BSTD_FUNCTION);

    printf("Delay = %d\n", hostResetVector->delay);

    printf("resetEcm = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_ECM) ? "true" : "false");
    printf("resetSecurityElem = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_SECURITY_ELEM) ? "true" : "false");
    printf("resetHost = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_HOST) ? "true" : "false");
    printf("resetExternalDevices = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_EXTERNAL_DEVICES) ? "true" : "false");
    printf("resetAll = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_ALL) ? "true" : "false");

    printf("restartOcapStack = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESTART_FIELD(hostResetVector, B_MPOD_HEADEND_RESTART_OCAP_STACK) ? "true" : "false");
    printf("restartAll = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESTART_FIELD(hostResetVector, B_MPOD_HEADEND_RESTART_ALL) ? "true" : "false");

    printf("reloadAllOcapApps = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RELOAD_APP_FIELD(hostResetVector, B_MPOD_HEADEND_RELOAD_ALL_OCAP_APPS) ? "true" : "false");
    printf("reloadOcapStack = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RELOAD_APP_FIELD(hostResetVector, B_MPOD_HEADEND_RELOAD_OCAP_STACK) ? "true" : "false");

    printf("reloadHostFirmware = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RELOAD_FW_FIELD(hostResetVector, B_MPOD_HEADEND_RELOAD_HOST_FIRMWARE) ? "true" : "false");

    printf("clearPersistentGetFeatParams = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_PERSISTENT_GEN_FEAT_PARAMS) ? "true" : "false");
    printf("clearOrgDvbPersistentFs = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_ORG_DVB_PERSISTENT_FS) ? "true" : "false");
    printf("clearCachedUnboundApps = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_CACHED_UNBOUND_APPS) ? "true" : "false");
    printf("clearRegisteredLibs = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_REGISTERED_LIBS) ? "true" : "false");
    printf("clearPersistentHostMem = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_PERSISTENT_HOST_MEM) ? "true" : "false");
    printf("clearSecElemPassedValues = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_SEC_ELEM_PASSED_VALUES) ? "true" : "false");
    printf("clearNonAsdDvrContent = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_NON_ASD_DVR_CONTENT) ? "true" : "false");
    printf("clearAsdDvrContent = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_ASD_DVR_CONTENT) ? "true" : "false");
    printf("clearNetworkDvrContent = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_NETWORK_DVR_CONTENT) ? "true" : "false");
    printf("clearMediaVolInternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_MEDIA_VOL_INTERNAL_HDD) ? "true" : "false");
    printf("clearMediaVolExternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_MEDIA_VOL_EXTERNAL_HDD) ? "true" : "false");
    printf("clearGpfsInternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_GPFS_INTERNAL_HDD) ? "true" : "false");
    printf("clearGpfsExternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_GPFS_EXTERNAL_HDD) ? "true" : "false");
    printf("clearAllStorage = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_ALL_STORAGE) ? "true" : "false");

    printf("\n");
}

void cablecard_init(
    void
    )
{
    B_MPOD_APINFO_SETTINGS apInfoSettings;
    B_MPOD_DSG_SETTINGS dsgSettings;
    B_MPOD_EXT_CH_SETTINGS extChSettings;
    B_MPOD_RES_SETTINGS resSettings = {5 /* max simul xpt streams*/,&resProfileInfoCb};
    B_MPOD_MMI_SETTINGS mmiSettings = {&mmiDialogRequestCb, &mmiDialogCloseCb,
        &mmiHtmlRdyCb, (B_MPOD_MMI_CB_EXTERN_RDY)NULL, (B_MPOD_MMI_CB_EXTERN_RCV)NULL, &mmiSessionOpenCb,
        &mmiSessionCloseCb};

    B_MPOD_SAS_SETTINGS sasSettings = {&sasConnectCnfCallback, &sasConnectionRdyCallback,
        &sasSynchDataRcvCallback, &sasAsynchDataRcvCallback, &sasGetSynchDataCallback,
        NULL, NULL, &sasConnectionClosedCallback};

    B_MPOD_DL_SETTINGS dlSettings = {&dlHostInfoCb,&dlNewCodeVersionTableType1Cb, &dlNewCodeVersionTableType2Cb};

    B_MPOD_FEATURE_SETTINGS featureSettings = {&featureReqHostListCb, &featureReqParamsCb,
                &featureRcvCardListCb, &featureRcvParamsCb, &featureParamDeniedCb};

    B_MPOD_HOST_CONTROL_SETTNINGS hostControlSettings = {&inbandTuneCb, &oobTxTuneCb,
                                                &oobRxTuneCb, &sourceIdToFreqCb};

    B_MPOD_HOMING_SETTINGS homingSettings = {&homingTimeoutCb, &delayedDownloadReqCb,
                    &homingCompleteCb, &downloadStartingCb, &downloadCompleteCb};

    B_MPOD_DIAG_SETTINGS diagSettings = {&diagReqCb};
    B_MPOD_SYSTIME_SETTINGS systimeSettings = {NULL};
    B_MPOD_CA_SETTINGS caSettings = {&caInfoCb, &caPmtUpdate, &caPmtReply};
    B_MPOD_CP_SETTINGS cpSettings = {&getAuthKeyCb, &cardAuthMsgCb,
        &getNonceCb, &cpkeyGenCb, &removeKeyCb, &progKeyCb, &calcCciAckCb,
        &enforceCciCb, &newValidationStatusCb, &getIDCb};

    B_MPOD_SNMP_SETTINGS snmpSettings = { &getRootOID, &getSnmpReply};
    B_MPOD_HEADEND_COMM_SETTINGS headendCommSettings = {&rcvHostResetVectorCb};

    B_MPOD_PROP_HOST_PROPS_SETTINGS hostPropertySettings = {&hostPropertiesReplyCb};


    BKNI_Memset(SasConnections, 0, sizeof(SasConnections));

/* Initialize the resource manager */
    if(B_Mpod_ResrcMgrInit())
    {
        printf("Unable to initialize resource manager\n");
        exit(1);
    }


    if(B_Mpod_SasInit(&sasSettings))
    {
        printf("Unable to initialize SAS resource\n");
        exit(1);
    }


/* DSG Settings */
    dsgSettings.dsgErrorCb = &CableCardCallback_DSG_Packet_Error;
    dsgSettings.dsgRcvSetModeCb = &CableCardCallback_DSGSet_DSG_Mode;
    dsgSettings.dsgRcvDirectoryCb = &CableCardCallback_DSG_Directory;
    if(B_Mpod_DsgInit(&dsgSettings))
    {
        printf("Unable to initialize DSG resource\n");
        exit(1);
    }

/* Ext Channel Settings */
    extChSettings.reqIpUnicastFlowCbThread = &CableCardCallback_IPU_Dhcp;
    extChSettings.reqIpMulticastFlowCb = &CableCardCallback_Req_Ipm_Flow;
    extChSettings.reqDsgFlowCb = &CableCardCallback_DSG_Flow_Id;
    extChSettings.reqSocketFlowCbThread = &CableCardCallback_Socket_Flow_Config;
    extChSettings.flowReqFailedCb = &CableCardCallback_Flow_Req_Failed;
    extChSettings.newFlowCnfCb = &CableCardCallback_New_Flow_Cnf_Cb;
    extChSettings.delFlowReqCb = &CableCardCallback_Delete_Flow_Req;
    extChSettings.delFlowCnfCb = &CableCardCallback_Del_Flow_Cnf_Cb;
    extChSettings.lostFlowIndCb = &CableCardCallback_Lost_Flow_Cb;
    extChSettings.rcvSetDsgModeCb = &CableCardCallback_ExtSet_DSG_Mode;
    extChSettings.dsgErrorCb = &CableCardCallback_DSG_Packet_Error;
    extChSettings.configAdvDsgCb = &CableCardCallback_Config_Advanced_DSG;
    extChSettings.rcvFlowDataCb = &CableCardCallback_Rcv_Flow_Data;
    if(B_Mpod_ExtendedChInit(&extChSettings))
    {
        printf("Unable to initialize Extended Channel resource\n");
        exit(1);
    }

/* App Info */
    B_Mpod_AppInfoGetDefaultCapabilities(&apInfoSettings);

    apInfoSettings.apInfoExternalHandlerRcvCb   = NULL;
    apInfoSettings.apInfoChangedCb              = &apInfoInfo_Changed_Cb;
    if(B_Mpod_AppInfoInit(&apInfoSettings))
    {
        printf("Unable to initialize ApInfo resource\n");
        exit(1);
    }

    if(B_Mpod_MmiInit(&mmiSettings))
    {
        printf("Unable to initialize MMI resource\n");
        exit(1);
    }

    if(B_Mpod_DownloadInit(&dlSettings))
    {
        printf("Unable to initialize Download resource\n");
        exit(1);
    }

    if(B_Mpod_FeatureInit(&featureSettings))
    {
        printf("Unable to initialize Feature resource\n");
        exit(1);
    }

    if(B_Mpod_HostControlInit(&hostControlSettings))
    {
        printf("Unable to initialize Host Control resource\n");
        exit(1);
    }

    if(B_Mpod_HomingInit(&homingSettings))
    {
        printf("Unable to initialize Homing resource\n");
        exit(1);
    }

    if(B_Mpod_DiagsInit(&diagSettings))
    {
        printf("Unable to initialize General Diagnostic resource\n");
        exit(1);
    }

    if(B_Mpod_SystimeInit(&systimeSettings))
    {
        printf("Unable to initialize Systime resource\n");
        exit(1);
    }

    if(B_Mpod_CaInit(&caSettings))
    {
        printf("Unable to initialize Systime resource\n");
        exit(1);
    }

    if(B_Mpod_CpInit(&cpSettings))
    {
        printf("Unable to initialize Systime resource\n");
        exit(1);
    }

    if(B_Mpod_ResInit(&resSettings))
    {
        printf("Unable to initialize Res resource\n");
        exit(1);
    }

    if(B_Mpod_SnmpInit(&snmpSettings))
    {
        printf("Unable to initialize SNMP resource\n");
        exit(1);
    }

    if(B_Mpod_HostPropertiesInit(&hostPropertySettings))
    {
        printf("Unable to initialize Host Property resource\n");
        exit(1);
    }

    if(B_Mpod_HeadendCommInit(&headendCommSettings))
    {
        printf("Unable to initialize HE resource\n");
        exit(1);
    }

}
