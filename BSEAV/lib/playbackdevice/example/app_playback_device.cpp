/**************************************************************************
 *    (c)2005-2011 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 * USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
***************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define SUPPORT_ENCRYPTED_DATA
#ifdef SUPPORT_ENCRYPTED_DATA
#include "drm_widevine.h"
#include "drm_init.h"
DrmRC widevine_security_config();
#endif

//#include "bkni.h"
//#include "bkni_multi.h"


#include "PlaybackDevice.h"
#include "data_reader.h"

#if 0
#include "b_nrm_lib.h"
#endif

typedef int32_t int32;
typedef uint64_t uint64;

#define PLAY_RATE_PAUSE 0
#define PLAY_RATE_NORMAL 1000

static CARD_StreamType              streamType   = CARD_StreamType_MPEG2_PS;

static CARD_PlaybackVideoParams     videoParams  = { 0x00, 0xE1, 0x00, CARD_VideoCodec_H264}; 
static CARD_PlaybackAudioParams     audioParams  = { 0x00, 0xC2, 0x00, 44100, 2, CARD_AudioCodec_AAC}; 
static CARD_PlaybackSecurity        security     = { 0, 0, false, false, false, CARD_OutputSecurity_NONE};
static uint32_t                     bufferDepth  = 0;
static bool                         parserMode   = false;
static bool                         loopMode     = false;
static int                          eosDelay     = 10000;  // 5 minutes

static bool                         initCurl     = false;
#ifdef SUPPORT_ENCRYPTED_DATA
static CARD_EncryptionType			streamEncType = CARD_EncryptType_NONE;//CARD_EncryptType_WV_VOD; 
#endif 

#define USE_BKNI_MEMORY

void MyFree(void *ptr)
{
#ifdef USE_BKNI_MEMORY
	BKNI_Free(ptr);
#else
	free(ptr);
#endif
}

static void Sleep(unsigned long nanoseconds) 
{
    struct timespec timeout;
    timeout.tv_sec  = 0;
    timeout.tv_nsec = nanoseconds;

    int ret = ::nanosleep(&timeout, NULL);

    ret = ret;
}

void usage() {
    printf("\n");
    printf(" app_card_playback_device is a test application to exersize\n");
    printf(" the CARD_PlaybackDevice object.\n");
    printf("\n");
    printf(" Usage:\n");
    printf(" app_card_playback_device -f filename <args>\n");
    printf("\n");
    printf(" Optional arguements:\n");
    printf(" -f      <val>  up to 10 files may be specified\n"); 
    printf(" -stype  <val>  [%5d] Input Stream Type  \n", streamType   ); 
    printf("                        ts=1, ps=2 \n"); 
    printf(" -vpid   <val>  [%5d] Video PID, if Transport Stream\n", videoParams.pid); 
    printf(" -vpesid <val>  [%5d] Video Stream ID, if Program Stream\n",  videoParams.pes_id); 
    printf(" -vsubid <val>  [%5d] Video Sub stream ID; optional for PS\n",  videoParams.substream_id); 
    printf(" -vcodec <val>  [%5d] Video Codec \n",  videoParams.codec); 
    printf("                       MPEG1=1, MPEG2=2, MPEG4P2=3, AVC=27, VC-1=234.\n"); 
    printf(" -apid   <val>  [%5d] Audio PID, if Transport Stream\n", audioParams.pid); 
    printf(" -apesid <val>  [%5d] Audio Stream ID, if Program Stream\n",  audioParams.pes_id); 
    printf(" -asubid <val>  [%5d] Audio Sub stream ID; optional for PS\n",  audioParams.substream_id); 
    printf(" -acodec <val>  [%5d] Audio Codec \n",  audioParams.codec); 
    printf("                        MP3=1, WMA=4, AAC=15, AC3=129 etc.\n"); 
    printf(" -asr    <val>  [%5d] Audio Sample Rate\n",  audioParams.sample_rate); 
    printf(" -achan  <val>  [%5d] Number of Audio Channels\n", audioParams.num_channels); 
    printf(" -depth  <val>  [%5d] Media Buffer Depth \n", bufferDepth); 
    printf(" -macro  <val>  [%5d] macrovision analog protection system \n",security.macrovision);
    printf(" -copy   <val>  [%5d] copyManagement copy management \n",security.copyManagement);
    printf(" -dot    <val>  [%5d] dot AACS digital only token \n",security.dot);
    printf(" -ict    <val>  [%5d] ict AACS image constraint token \n",security.ict);
    printf(" -scale  <val>  [%5d] PreventUpscaling upscaling prohibited on analog outputs by CSS \n",security.preventUpscaling);
    printf(" -etype  <val>  [%5d] encryption_type encrption type of current title \n",security.securityMode);
    printf("\n");
    printf(" -p      <val>  [%5d] parser mode 0 disabled, 1 enabled\n", (int) parserMode); 
    printf(" -l      <val>  [%5d] loop   mode 0 disabled, 1 enabled\n", (int) loopMode); 
    printf(" -eos    <val>  [%5d] endOfStream delay \n", (int) eosDelay); 
}


void runUsage() {
    printf("\n");
    printf(" n play next (or restart first) file\n"); 
    printf(" p pause/play toggle \n");
    printf(" t get current pts \n");
    printf(" f flush \n");
    printf(" x terminate playback\n");
    printf(" ? print this list\n");
}



//extern void CARD_SetPEHandle(PE_HANDLE pe);
static int  decOrhex_atoi(const char *str);
static CARD_PlaybackStatus waitForEvent(CARD_PlaybackDevice *pPlaybackDevice, 
		                                CARD_PlaybackEventID event, int wait);

#define READ_SIZE (4096*4)
#define READ_SIZE 188
#ifdef SUPPORT_ENCRYPTED_DATA
#define READ_SIZE 0xffff
#else
#define READ_SIZE (4096*1)
#endif


#ifdef SUPPORT_ENCRYPTED_DATA

DrmRC widevine_security_config()
{
	DRM_Widevine_Init_t inputWvInitStruct;
	DrmRC drm_rc = Drm_Success;
	uint8_t ecm_even[32] = {0xe6, 0x56, 0x07, 0xde, 0x61, 0xf2, 0x05, 0x2c,
    						0x71, 0x40, 0xc5, 0x0a, 0xe1, 0x56, 0x8f, 0x5f,
    						0x95, 0x4c, 0x20, 0xda, 0x4c, 0x7e, 0xa9, 0x2f,
    						0xe7, 0x98, 0xa5, 0xde, 0xd2, 0x5b, 0x53, 0x4b};

#if 0 

    uint8_t ecm_odd[32] = {0x18, 0x99, 0x35, 0x79, 0x07, 0x83, 0x40, 0x42,
    						0x2f, 0x6e, 0xe3, 0x9e, 0xf8, 0x3b, 0x88, 0xca,
							0x28, 0x20, 0x31, 0x26, 0x03, 0x73, 0x49, 0x2a,
							0xe5, 0x0c, 0x08, 0xbe, 0xa8, 0xf8, 0x5f, 0xe9 };
#else

	uint8_t ecm_odd[32] = {0x42, 0xb2, 0x3a, 0xc5, 0xe7, 0x08, 0xa0, 0x11,
		               0x38, 0xab, 0x35, 0x34, 0xd5, 0x05, 0xce, 0x17,
		               0xa6, 0xdd, 0x03, 0x3e, 0xfd, 0xe1, 0xe2, 0xf8,
		               0xc8, 0xd0, 0x43, 0x9b, 0x0c, 0x96, 0xaf, 0xd2};
#endif

#if 0
    uint8_t id[16] = {0x00};
#else
	  uint8_t id[4] = {0};
#endif

#if 0
    uint8_t common_key[16] = {0x31, 0x9f, 0xf5, 0xa5, 0xb2, 0x74, 0x6d, 0x9b, 0x74, 0xa6, 0x34, 0x3c, 0xaa, 0xf0, 0x1e, 0xf1};
#else
	uint8_t common_key[16] = {0x7e, 0xca, 0xd3, 0x2e, 0xec, 0x06, 0x5f, 0xf4, 0xeb, 0xa6, 0xc6, 0x32, 0x11, 0x5e, 0x0c, 0x0c};
#endif

#if 0
	unsigned char iv0[16] = { 0x00};
#else
uint8_t iv0[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
#endif

	printf("\n Entered %s",__FUNCTION__);
	inputWvInitStruct.key_file = "WIDEVINE_SETTOP_KEYBOX_00000000.bin";
	inputWvInitStruct.mode = DRM_WidevineDecryptMode_eVod;
    DRM_Widevine_Initialize(inputWvInitStruct);

	
    drm_rc = DRM_Widevine_LoadCwFixed(common_key, ecm_odd, id, DrmSecurityKeyType_eEven);
    if(drm_rc != Drm_Success)
	{
        printf("\tMAIN - DRM_Widevine_LoadCw (even) failed, rc %d\n", drm_rc);
        goto ErrorExit;
    }
	else
	{
		printf("\n\tMAIN - CW ID: 0x%02x 0x%02x 0x%02x 0x%02x\n\n", id[0], id[1], id[2], id[3]);
	}

    drm_rc = DRM_Widevine_LoadCwFixed(common_key, ecm_odd, id, DrmSecurityKeyType_eOdd);
    if(drm_rc != Drm_Success)
	{
        printf("\tMAIN - DRM_Widevine_LoadCw (odd) failed, rc %d\n", drm_rc);
        goto ErrorExit;
    }
	else
	{
		printf("\n\tMAIN - CW ID: 0x%02x 0x%02x 0x%02x 0x%02x\n\n", id[0], id[1], id[2], id[3]);
	}

ErrorExit:
	printf("\nExiting %s",__FUNCTION__);
	return drm_rc;

}


void *pHeader=NULL;
unsigned long hdrSz=0;

#endif



int main(int argc, char **argv)
{

    CARD_PlaybackStatus status;
    int  activeFile        = 0;
    int  fileCnt           = 0;
    char content_file[10][256] = {{0}};

    if ( argc <= 2 ) 
    { 
        usage();
        return(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) 
    {
        if ( !strcmp(argv[i], "-f") ) 
        {
            if( fileCnt < 10 )
            {
                strcpy(content_file[fileCnt], argv[++i]);
                fileCnt++;
            }
            else
            {
                printf("*********************\n");
                printf("* Max fileCnt is 10 *\n");
                printf("*********************\n");
            }
        } 
        else if ( !strcmp(argv[i], "-p") ) 
        {
            parserMode = (bool) decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-l") ) 
        {
            loopMode = (bool) decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-eos") ) 
        {
            eosDelay =  decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-stype") ) 
        {
            streamType   = (CARD_StreamType) decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-vpid") ) 
        {
            videoParams.pid = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-vpesid") ) 
        {
            videoParams.pes_id = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-vsupid") ) 
        {
            videoParams.substream_id = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-vcodec") ) 
        {
            videoParams.codec = (CARD_VideoCodec) decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-apid") ) 
        {
            audioParams.pid = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-apesid") ) 
        {
            audioParams.pes_id = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-asupid") ) 
        {
            audioParams.substream_id = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-acodec") ) 
        {
            audioParams.codec = (CARD_AudioCodec) decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-asr") ) 
        {
            audioParams.sample_rate = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-achan") ) 
        {
            audioParams.num_channels = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-depth") ) 
        {
            bufferDepth = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-macro") ) 
        {
            security.macrovision = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-copy") ) 
        {
            security.copyManagement = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-dot") ) 
        {
            security.dot = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-ict") ) 
        {
            security.ict = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-scale") ) 
        {
            security.preventUpscaling = decOrhex_atoi(argv[++i]);
        } 
        else if ( !strcmp(argv[i], "-etype") ) 
        {
            security.securityMode = (CARD_OutputSecurity) decOrhex_atoi(argv[++i]);
        } 
#ifdef SUPPORT_ENCRYPTED_DATA
		else if ( !strcmp(argv[i], "-enctype") ) 
        {
            streamEncType = (CARD_EncryptionType) decOrhex_atoi(argv[++i]);
        } 
#endif
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
        printf("........File[%d] is >%s<\n", i, content_file[i]);
    }

    uint32_t   feedCnt    = 0;
    uint32_t   bufferSize = 0;
    void     *pBuffer     = NULL;
    bool     endOfFile    = false;
    bool     jump         = false;
    int32    speed        = PLAY_RATE_PAUSE;
    
    int      flags   = fcntl(STDIN_FILENO, F_GETFL, 0);
    char     cmd;
    int      readStatus;

#ifdef LOADER_USED
    LOADER_HANDLE loaderHandle = NULL;
#endif
    cDataReader  *pFile        = NULL;

    while( activeFile < fileCnt )
    {
        if( pFile != NULL )
        {
            pFile->Close();
            delete pFile;
        }

        if( isHttp(content_file[activeFile]) )
        {
            if( initCurl == false )
            {
                if( curl_global_init(CURL_GLOBAL_SSL) != CURLE_OK) 
                {
                    printf("%s:  curl_global_init Failed!!!\n",__FUNCTION__);
                    return(EXIT_FAILURE);
                }
                initCurl = true;
            }
            pFile = new cHttpReader();
        }
        else
        {
#ifdef LOADER_USED
            if( loaderHandle == NULL )
            {
                loaderHandle = LoaderCreate(LOADER_ATAPI, 0);

                if( loaderHandle == NULL )
                {
                    printf("%s: LoaderCreate Failed!!!\n",__FUNCTION__);
                    return(EXIT_FAILURE);
                }
            }
        
            pFile  = new cFileReader(loaderHandle);
#else
#ifdef SUPPORT_ENCRYPTED_DATA
			if(streamEncType!=CARD_EncryptType_NONE)
				pFile = new cEncrptedPesReader();
				
			else
#endif
				pFile  = new cFileReader();
#endif

        }

        if( pFile == NULL )
        {
            printf("%s:  new FileReader Failed!!!\n",__FUNCTION__);
            return(EXIT_FAILURE);
        }

        if( pFile->Open(content_file[activeFile]) != READER_SUCCESS )
        {
            printf(" Hosed! open of file >%s<failed!!!\n", content_file[activeFile]);
            return(EXIT_FAILURE);
        }

        bool playbackStart = false;
     
        CARD_PlaybackDevice *pPlaybackDevice = NULL;
        CARD_PlaybackEventID eventID; 
        uint64               eventParam1;
    
        printf("create playback device\n");
    
        pPlaybackDevice = new CARD_PlaybackDevice;
    
        if ( parserMode ) 
        {
            printf("open playback device in parserMode\n");
			
			
			status = pPlaybackDevice->open(CARD_EncryptType_NONE, bufferDepth, security, &MyFree);

        }
        else
        {
            printf("open playback device in NONparserMode\n");
            std::vector<CARD_PlaybackVideoParams> VideoParams;
            std::vector<CARD_PlaybackAudioParams> AudioParams;

            VideoParams.push_back(videoParams);
            AudioParams.push_back(audioParams);

#ifdef SUPPORT_ENCRYPTED_DATA
			if(streamEncType!=CARD_EncryptType_NONE)
			{
				printf("open playback device with encryption mode\n");
				status = pPlaybackDevice->open(streamType, streamEncType,
                                           VideoParams, AudioParams,
                                           bufferDepth, security,&MyFree);
				if (widevine_security_config()!=Drm_Success)
					 return(EXIT_FAILURE);
			}
			else
#endif
            status = pPlaybackDevice->open(streamType, CARD_EncryptType_NONE,
                                       VideoParams, AudioParams,
                                           bufferDepth, security, &MyFree);

       } 
    
        if(status != CARD_PlaybackSuccess )
        {
            printf("Hosed! open failed=%d\n", status);
            return(EXIT_FAILURE);
        }

#if 0
// this is specifically for DTV platform to get DMA handle for decryption
// will be used by widevide themselves
unsigned long clientId;
NEXUS_DmaHandle dmaHandle = NULL;

B_NRM_RegisterClient((char *) __FUNCTION__, __LINE__, NULL, NULL, &clientId);

printf("%s(): clientId = 0x%08lx\n", __FUNCTION__, clientId);

dmaHandle = (NEXUS_DmaHandle) B_NRM_CheckoutHandleSync(B_NRM_eNEXUS_DmaHandle, B_NRM_eName_dma, clientId);

printf("%s(): dmaHandle = %p\n", __FUNCTION__, (void *) dmaHandle);

B_NRM_CheckinHandle(dmaHandle, clientId);
dmaHandle = NULL;

B_NRM_UnregisterClient(clientId);
#endif

        int errCode;
        bufferSize = 0;
        pBuffer    = NULL;
        endOfFile  = false;
        jump       = false;
        speed      = PLAY_RATE_PAUSE;
		unsigned int read_sz=0;
		bool sendPayLoad=false;

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
                        jump      = true;
                        endOfFile = true;
                        if( (activeFile + 1)  == fileCnt )
                        {
                            activeFile = -1;
                        }
                        printf("jump to file %d\n", (activeFile + 1));
                        break;
                    }
                    case 'p':
                    {
                        if( speed  == PLAY_RATE_NORMAL )
                        {
                            printf("setSpeed PAUSE\n");
                            speed  = PLAY_RATE_PAUSE;
                        }
                        else
                        {
                            printf("setSpeed NORMAL\n");
                            speed  = PLAY_RATE_NORMAL;
                        }
                        status = pPlaybackDevice->setSpeed(speed);
                        if( status != CARD_PlaybackSuccess )
                        {
                            printf("Hosed! setSpeed failed=%d\n", status);
                            return(EXIT_FAILURE);
                        }
                        break;
                    }
                    case 't':
                    {
                        TIME45k  currentTime;
                        status = pPlaybackDevice->getPts(currentTime);
                        if( status != CARD_PlaybackSuccess )
                        {
                            printf("Hosed! getPts failed=%d\n", status);
                            return(EXIT_FAILURE);
                        }
    
                        // convert to millisecs
                        currentTime /= 45;

                        // convert to seconds
                        currentTime /= 1000;

                        int minutes = currentTime/60;

                        int seconds = currentTime - minutes*60;

                        printf("........time=%d min %d secs\n", minutes, seconds);
    
                        break;
                    }
                    case 'f':
                    {
                        printf(".........flush\n");
                        status = pPlaybackDevice->flush();
                        if( status != CARD_PlaybackSuccess )
                        {
                            printf("Hosed! flush failed=%d\n", status);
                            return(EXIT_FAILURE);
                        }
                        speed  = PLAY_RATE_PAUSE;

                        if( parserMode )
                        {
							printf("\n pFile->Seek(0,0)\n");
                            pFile->Seek(0,0);
                        }

                        printf(".........type 'p' to restart\n");
                        break;
                    }
                    case 'x':
                    {
                        printf(".........terminating\n");
                        endOfFile = true;
                        activeFile= 100;
                        break;
                    }
                    case '?':
                    {
                        runUsage();
                        break;
                    }
                    default:
                    {
                        //printf("%s:%d default(%c)\n", __FUNCTION__, __LINE__, cmd);
                        break;
                    }
                }
            }

            if( pBuffer == NULL )
            {
                // allocate memory for a new data chunk
#ifdef USE_BKNI_MEMORY
				pBuffer = BKNI_Malloc(READ_SIZE);
#else
				pBuffer =  malloc(READ_SIZE);
#endif
			
    
                if( pBuffer == NULL )
                {
                    printf("Hosed! malloc failed\n");
                    break;
                }
				//printf("\n memory allocated at 0x%x",pBuffer);
        
#ifdef SUPPORT_ENCRYPTED_DATA

				if(streamEncType!=CARD_EncryptType_NONE){
											
						pHeader=BKNI_Malloc(16);
						errCode =  pFile->ReadPacket((ULONG) READ_SIZE,pHeader,&hdrSz,pBuffer,(ULONG *) &bufferSize);
						read_sz=hdrSz;
				}

				else
#endif	
				{ 
						errCode = pFile->Read((ULONG) READ_SIZE, pBuffer, (ULONG *) &bufferSize);
						read_sz=READ_SIZE;
				}

				
#ifdef SUPPORT_ENCRYPTED_DATA
                if( (bufferSize <read_sz)||  errCode==READER_FAILURE)
#else
				 if( bufferSize < read_sz )
#endif

                {
                    printf("end of file\n");
					
					endOfFile = true;
					
                    if( playbackStart == false )
                    {
                        playbackStart = true;
                        status = pPlaybackDevice->setSpeed(PLAY_RATE_NORMAL);
                        speed  = PLAY_RATE_NORMAL;
                        if( status != CARD_PlaybackSuccess )
                        {
                            printf("Hosed! setSpeed failed=%d\n", status);
                            return(EXIT_FAILURE);
                        }
                    }
                }
            }
    
            feedCnt = 0;
			
            while( feedCnt < 2 )
            {
                feedCnt++;
#ifdef SUPPORT_ENCRYPTED_DATA
				if(streamEncType!=CARD_EncryptType_NONE){
					//first send the header
					//printf("\n sendPayLoad is %s",sendPayLoad?"TRUE":"False");
					if(!endOfFile){
						if(!sendPayLoad){ 
							//printf("\n send Header:\n");
							status = pPlaybackDevice->feedData(pHeader, hdrSz);
							
						}				
						else{
								//printf("\n send Payload:\n");
								status = pPlaybackDevice->feedData(pBuffer, bufferSize,CARD_EncryptKeyType_EVEN);
								//printf("\n send Payload DONE\n");
					
					}
					
				}
				else
						status=CARD_PlaybackSuccess;
			}
			else
#endif
			{
				status = pPlaybackDevice->feedData(pBuffer, bufferSize);

			}
			
			if( status == CARD_PlaybackSuccess )
            {
#ifdef SUPPORT_ENCRYPTED_DATA
				if(streamEncType!=CARD_EncryptType_NONE){
                   if(sendPayLoad)
				   {
					pBuffer = NULL;
					sendPayLoad=false;
					//printf("\n payload sent sucessfully, now send next header");
				   }
				   else{
					sendPayLoad=true;
					//printf("\n header sent sucessfully, now send payload\nsenPayload is %s",sendPayLoad?"TRUE":"FALSE");
				   }
			}else
#endif
				pBuffer = NULL;
//#endif
					
                    // keep the event queue empty
                    if( playbackStart == true )
                    {
                        status = pPlaybackDevice->getEvent(eventID, eventParam1);
                        if( status == CARD_PlaybackSuccess)
                        {
                            //printf("getEvent returned event =%d\n", eventID);
                        }
                    }
                    break;
                }
                else if( status == CARD_PlaybackFull )
                {
					//printf("\n CARD_PlaybackFull!!!!!\n" );
					if( playbackStart == false )
                    {
                        playbackStart = true;
                        status = pPlaybackDevice->setSpeed(PLAY_RATE_NORMAL);
                        speed  = PLAY_RATE_NORMAL;
                        if( status != CARD_PlaybackSuccess )
                        {
                            printf("Hosed! setSpeed failed=%d\n", status);
                            return(EXIT_FAILURE);
                        }
                    }
                    Sleep(100*1000);
                }
                else
                {
                    printf("Hosed! feedData failed=%d\n", status);
                    return(EXIT_FAILURE);
                }
            }   // end feed Data
        
        }   //while( endOfFile == false )
    
        if( jump == false )
        {
            // send the end of stream message
            printf("Calling endOfStream\n");
            status = pPlaybackDevice->endOfStream();
            if( status != CARD_PlaybackSuccess )
            {
                printf("Hosed! setSpeed failed=%d\n", status);
                return(EXIT_FAILURE);
            }
    
            // wait till end of content is received
            printf("waiting for endOfStream with eosDelay %d\n",eosDelay);
            waitForEvent(pPlaybackDevice, CARD_PlaybackEvent_END_OF_STREAM, eosDelay);
        }
    
        printf("Closing device\n");
        pPlaybackDevice->close();
     
        printf("delete playback device\n");
        delete pPlaybackDevice;

        activeFile++;
		if( loopMode )
		{
			if( activeFile >= fileCnt )
			{
				activeFile = 0;
			}
		}
    }

    if( initCurl == true )
    {
        curl_global_cleanup();
    }
    
    return(EXIT_SUCCESS);
}

static int decOrhex_atoi(const char *str)
{
  unsigned int i, hexDigitValue, value=0;

  if ((strlen(str) > 2) && str[0]=='0' && str[1]=='x') {
    str += 2;
    for (i=0; i<strlen(str); i++) {
      if (str[i] >= 'a' && str[i] <= 'f') {
        hexDigitValue = str[i] - 'a' + 10;
      } else if (str[i] >= 'A' && str[i] <= 'F') {
        hexDigitValue = str[i] - 'A' + 10;
      } else {
        hexDigitValue = str[i] - '0';
      }
      value <<= 4; value |= hexDigitValue;
    }
  } else {
    value = atoi(str);
  }

  return value;
} 

static CARD_PlaybackStatus waitForEvent(CARD_PlaybackDevice *pPlaybackDevice, 
		                                CARD_PlaybackEventID event, int wait)
{
    CARD_PlaybackStatus  status = CARD_PlaybackFailure;
    CARD_PlaybackEventID eventID;
    uint64               eventParam1;
	int                  delay = 0;
    int                  emptyCount = 0;

	
	// convert to millisecs
	wait *= 1000;
	
    // wait till end of content is received
    //printf("waiting for event%d\n", (int) event);
    while( delay <= wait )
    {
		
        status = pPlaybackDevice->getEvent(eventID, eventParam1);
        if( status == CARD_PlaybackSuccess)
        {
            if( event == eventID)
            {
                status = CARD_PlaybackSuccess;
				printf("\nRecd EOS event !!");
                break;
            }
        }
        else if( status == CARD_PlaybackFailure )
        {
            emptyCount++;
            if( (emptyCount > wait) && (event == CARD_PlaybackEvent_END_OF_STREAM) )
            {
                printf("timestamps ended assume end of stream\n");
                status = CARD_PlaybackSuccess;
                break;
            }
            Sleep(100*1000);
			delay += 100;
        }
        else
        {
            printf("Hosed! getEvent failed=%d\n", status);
            break;
        }
    }
    return(status);
}
      
