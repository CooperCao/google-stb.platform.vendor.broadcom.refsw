/******************************************************************************
 *    (c)2008-2011 Broadcom Corporation
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
 * Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *****************************************************************************/

#include "nexusMgr.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "blst_slist.h"
#include "bchp_gio.h"
#include "nexus_platform.h"
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif
#include "nexus_display.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_mixer.h"
#include "nexus_core_utils.h"
#include "nexus_dma.h"
#include "nexus_ir_input.h"
#include "IrEventCode.h"
#include "avio_config.h"
#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)

BDBG_MODULE(nexusMgr);

#include "bchp_common.h"
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

#ifndef _SC_DIRECT_NEXUS_
#include "dfb_platform.h"
#endif

typedef struct BcmNexus_TTY {
    int                 fd;
    struct termios      savedTermIos;
    bool                exit;
    bool                post_exit;
    int                 signal;
    pthread_t           thread;
    NEXUS_CallbackDesc  callback;
} BcmNexus_TTY;

#define BCM_TTY_DEV         "/dev/tty"
#define BCM_CONSOLE_DEV     "/dev/console"


BDBG_OBJECT_ID(BcmNexus_Input_Resources);
typedef struct BcmNexus_Input_Resources_Impl {
    BcmNexus_Input_Resources            public;
    NEXUS_IrInputHandle                 irInput;
    const KeyMapElement *               keymap;
    NEXUS_IrInputMode                   mode;
    NEXUS_CallbackDesc                  callback;
    BcmNexus_TTY                        tty;
    BDBG_OBJECT(BcmNexus_Input_Resources)
} BcmNexus_Input_Resources_Impl;


static struct {
    bool                                initialized;
    bool                                autoInitialized;
#ifdef NEXUS_HAS_PICTURE_DECODER
    NEXUS_PictureDecoderHandle          pictureDecoder;
#endif
#ifdef _SC_DIRECT_NEXUS_
    BKNI_MutexHandle                    graphics2d_sync_lock;   /* Mutex for graphics checkpoint */
    BKNI_EventHandle                    graphics2d_sync_event;  /* Event set when graphics checkpoint complete */
#endif
    BKNI_MutexHandle                    resource_lock;
    NEXUS_DisplayHandle                 display[2];
    NEXUS_Graphics2DHandle              graphics2d;
    void (*graphics2d_sync)(void);

    /* mixer doesn't support changes in the configuration, so have to open all mixer sources ahead of time and connect them to the mixer */
    NEXUS_AudioMixerHandle              mixer;
    NEXUS_AudioPlaybackHandle           pcmPlayback[NEXUS_NUM_AUDIO_PLAYBACKS];
    NEXUS_AudioDecoderHandle            audioDecoder[1];

    /* HDMI hotplug state variables */
    bool                                hdmiAudioConnected;
    bool                                hdmiVideoConnected;
    bool                                hdmiCableConnected;
    NEXUS_CallbackDesc                  hdmiHotplugCallbackOriginal;

    BcmNexus_StreamPlayer_Resources *   streamResource;
    BcmNexus_Input_Resources_Impl *     inputResource;
    struct BcmNexus_Graphics_Resources_Impl *   graphics_resources;

    /* For starting/stopping audio decoders during HDMI hotplug */
    NEXUS_AudioDecoderStartSettings     audioSetting;
    NEXUS_AudioPlaybackStartSettings    pcmSetting[NEXUS_NUM_AUDIO_PLAYBACKS];
    bool                                pcmStarted[NEXUS_NUM_AUDIO_PLAYBACKS];		
    bool                                audioStarted[1];

    struct timeval                      last_input_timestamp;   /* Last time a user input was made */
    BKNI_MutexHandle                    key_up_lock;

    uint32_t                            FLevent;
    uint8_t                             current;                /* Current display frame buffer */
    bool                                doublebuffer;           /* Use double buffering flag */
    NEXUS_SurfaceHandle                 displayFramebuffer[2];  /* Display frame buffer handles */
} BcmNexus_State;
bool gAppEnabledHDCP=false;

#define BCM_NEXUS_PCM_FIFO_SIZE (B_AUDIO_FRAME_SIZE * B_AUDIO_BUFFERED_FRAMES * 4)

static void BcmNexus_Lock(void)
{
   if (!BcmNexus_State.initialized) {
        BcmNexus_State.autoInitialized = true;
        if(!BcmNexus_Platform_Init()) {
            fprintf(stderr,"can't initialize the system\n");
            exit(-1);
        }

        TestHarnessStart();

   }
   BDBG_MSG_TRACE(("BcmNexus_Lock"));
   BKNI_AcquireMutex(BcmNexus_State.resource_lock);
   return;
}

static void BcmNexus_Unlock(void)
{
   BDBG_MSG_TRACE(("BcmNexus_Unlock"));
   BKNI_ReleaseMutex(BcmNexus_State.resource_lock);
}

typedef struct BcmNexus_Resource {
    bool used[4];
    unsigned size;
} BcmNexus_Resource;

#define RESOURCES_INITALIZER(size) {{false,false,false,false}, (size)}

static int BcmNexus_Resource_Acquire(BcmNexus_Resource *resource)
{
    unsigned i;
    BDBG_ASSERT(resource->size <= sizeof(resource->used)/sizeof(resource->used[0]));
    for(i=0;i<resource->size;i++) {
        if(!resource->used[i]) {
            resource->used[i] = true;
            return (int)i;
        }
    }
    return -1;
}

static void BcmNexus_Resource_Release(BcmNexus_Resource *resource, int id)
{
    BDBG_ASSERT(id >= 0 && id < (int)resource->size); 
    BDBG_ASSERT(resource->used[id]);
    resource->used[id] = false;
    return;
}

static bool
BcmNexus_Platform_Init_Audio(void)
{
	NEXUS_PlatformConfiguration	platformConfig;
    NEXUS_Error rc;
    unsigned i;
    NEXUS_AudioPlaybackOpenSettings pcmOpenSettings;
    
    BDBG_MSG_TRACE(("BcmNexus_Platform_Init_Audio"));

	NEXUS_Platform_GetConfiguration(&platformConfig);       	

    BcmNexus_State.mixer = NEXUS_AudioMixer_Open(NULL);
    if (BcmNexus_State.mixer==NULL) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_mixer;}

    /* Connect DAC to mixer */
    if(platformConfig.outputs.audioDacs[0])  {
        rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), NEXUS_AudioMixer_GetConnector(BcmNexus_State.mixer));
        if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_mixer_output;}
    }

    NEXUS_AudioPlayback_GetDefaultOpenSettings(&pcmOpenSettings);
    pcmOpenSettings.fifoSize = BCM_NEXUS_PCM_FIFO_SIZE;
    for(i=0;i<sizeof(BcmNexus_State.pcmPlayback)/sizeof(BcmNexus_State.pcmPlayback[0]);i++) {
        BcmNexus_State.pcmPlayback[i] = NEXUS_AudioPlayback_Open(i, &pcmOpenSettings);
        if (BcmNexus_State.pcmPlayback[i]==NULL) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_pcm_playback;}
        rc = NEXUS_AudioMixer_AddInput(BcmNexus_State.mixer, NEXUS_AudioPlayback_GetConnector(BcmNexus_State.pcmPlayback[i]));
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_audio_pcm_connect;}
    }
    for(i=0;i<sizeof(BcmNexus_State.audioDecoder)/sizeof(BcmNexus_State.audioDecoder[0]);i++) {
        BcmNexus_State.audioDecoder[i] = NEXUS_AudioDecoder_Open(i, NULL);
        if(!BcmNexus_State.audioDecoder[i]) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_audioDecoder; }
        rc = NEXUS_AudioMixer_AddInput(BcmNexus_State.mixer, NEXUS_AudioDecoder_GetConnector(BcmNexus_State.audioDecoder[i], NEXUS_AudioDecoderConnectorType_eStereo));
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_audio_dec_connect;}
    }
    return true;

err_audio_dec_connect:
err_audioDecoder:
err_audio_pcm_connect:
err_pcm_playback:
err_mixer_output:
err_mixer:
    return false;
}

static void
BcmNexus_Platform_Uninit_Audio(void)
{
    unsigned i;
	NEXUS_PlatformConfiguration	platformConfig;
    BDBG_MSG_TRACE(("BcmNexus_Platform_Uninit_Audio"));

	NEXUS_Platform_GetConfiguration(&platformConfig);       	

    for(i=0;i<sizeof(BcmNexus_State.pcmPlayback)/sizeof(BcmNexus_State.pcmPlayback[0]);i++) {
        NEXUS_AudioMixer_RemoveInput(BcmNexus_State.mixer, NEXUS_AudioPlayback_GetConnector(BcmNexus_State.pcmPlayback[i]));
        NEXUS_AudioInput_Shutdown(NEXUS_AudioPlayback_GetConnector(BcmNexus_State.pcmPlayback[i]));
        NEXUS_AudioPlayback_Close(BcmNexus_State.pcmPlayback[i]);
    }
    for(i=0;i<sizeof(BcmNexus_State.audioDecoder)/sizeof(BcmNexus_State.audioDecoder[0]);i++) {
        NEXUS_AudioMixer_RemoveInput(BcmNexus_State.mixer, NEXUS_AudioDecoder_GetConnector(BcmNexus_State.audioDecoder[i], NEXUS_AudioDecoderConnectorType_eStereo));
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(BcmNexus_State.audioDecoder[i], NEXUS_AudioDecoderConnectorType_eStereo));
        NEXUS_AudioDecoder_Close(BcmNexus_State.audioDecoder[i]);
    }

    if(platformConfig.outputs.audioDacs[0])  {
        NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), NEXUS_AudioMixer_GetConnector(BcmNexus_State.mixer));
    }
#if NEXUS_NUM_HDMI_OUTPUTS
    if (BcmNexus_State.hdmiAudioConnected) {
        NEXUS_AudioOutput_RemoveInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), NEXUS_AudioMixer_GetConnector(BcmNexus_State.mixer));
    }
#endif

    NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(BcmNexus_State.mixer));
    NEXUS_AudioMixer_Close(BcmNexus_State.mixer);
    return;
}

#if NEXUS_NUM_HDMI_OUTPUTS

static void
BcmNexus_StopAll_AudioDecoders(void)
{
	unsigned i;
		
	for(i=0;i<sizeof(BcmNexus_State.audioDecoder)/sizeof(BcmNexus_State.audioDecoder[0]);i++) {
		if(BcmNexus_State.audioDecoder[i]) {
			if (BcmNexus_State.audioStarted[i]) {
				
				BDBG_MSG_TRACE(("Stop AudioDecoder"));
				NEXUS_AudioDecoder_Stop(BcmNexus_State.audioDecoder[i]);
			}
		}
	}
	for(i=0;i<sizeof(BcmNexus_State.pcmPlayback)/sizeof(BcmNexus_State.pcmPlayback[0]);i++) {
		if(BcmNexus_State.pcmPlayback[i]) {
			if (BcmNexus_State.pcmStarted[i]) {
				BDBG_MSG_TRACE(("Stop PCM Playback"));
				NEXUS_AudioPlayback_Stop(BcmNexus_State.pcmPlayback[i]);
			}
		}
	}
}

static void
BcmNexus_StartAll_AudioDecoders(void)
{
	unsigned i;
	for(i=0;i<sizeof(BcmNexus_State.audioDecoder)/sizeof(BcmNexus_State.audioDecoder[0]);i++) {
		if(BcmNexus_State.audioDecoder[i]) {
			if (BcmNexus_State.audioStarted[i]) {
				BDBG_MSG_TRACE(("Start AudioDecoder"));
				NEXUS_AudioDecoder_Start(BcmNexus_State.audioDecoder[i], &BcmNexus_State.audioSetting);
			}
		}
	}
	for(i=0;i<sizeof(BcmNexus_State.pcmPlayback)/sizeof(BcmNexus_State.pcmPlayback[0]);i++) {
		if(BcmNexus_State.pcmPlayback[i]) {
			if (BcmNexus_State.pcmStarted[i]) {
				BDBG_MSG_TRACE(("Start PCM Playback"));
				NEXUS_AudioPlayback_Start(BcmNexus_State.pcmPlayback[i], &BcmNexus_State.pcmSetting[i]);
			}
		}
	}
}
/*#define BRCM_COPY_PROTECTION_ENABLED 0*/
#if BRCM_COPY_PROTECTION_ENABLED 

/*vgd*/
/*****************/
/* For HDCP TESTING  */
/*    1) insert the Production Key Set set generated by BCrypt  */
/*    2) set the USE_PRODUCTION_KEYS macro to to 1 */
/*****************/

#define USE_PRODUCTION_KEYS 0

#if USE_PRODUCTION_KEYS

/*****************************/
/* INSERT PRODUCTION KeySet HERE */
/* Google needs to obtain production keys and place them at a specified in Flash , */
/*we need to add code to read the production keys from the specified loaction.*/
/*****************************/

#else


/**************************************/
/* HDCP Specification Test Key Set    */
/*                                    */
/* NOTE: the default declared Test    */
/* KeySet below is from the HDCP Spec */ 
/* and it *IS NOT* compatible with    */
/* production devices                 */
/**************************************/


static NEXUS_HdmiOutputHdcpKsv hdcpTxAksv =
{	{0x14, 0xF7, 0x61, 0x03, 0xB7} };

static NEXUS_HdmiOutputHdcpKey encryptedTxKeySet[NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS] =
{
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x691e138f, 0x58a44d00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x0950e658, 0x35821f00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x0d98b9ab, 0x476a8a00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xcac5cb52, 0x1b18f300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xb4d89668, 0x7f14fb00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x818f4878, 0xc98be000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x412c11c8, 0x64d0a000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x44202428, 0x5a9db300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6b56adbd, 0xb228b900},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf6e46c4a, 0x7ba49100},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x589d5e20, 0xf8005600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xa03fee06, 0xb77f8c00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x28bc7c9d, 0x8c2dc000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x059f4be5, 0x61125600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xcbc1ca8c, 0xdef07400},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6adbfc0e, 0xf6b83b00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xd72fb216, 0xbb2ba000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x98547846, 0x8e2f4800},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x38472762, 0x25ae6600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf2dd23a3, 0x52493d00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x543a7b76, 0x31d2e200},	
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x2561e6ed, 0x1a584d00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf7227bbf, 0x82603200},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6bce3035, 0x461bf600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6b97d7f0, 0x09043600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf9498d61, 0x05e1a100}, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x063405d1, 0x9d8ec900}, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x90614294, 0x67c32000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xc34facce, 0x51449600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x8a8ce104, 0x45903e00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xfc2d9c57, 0x10002900},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x80b1e569, 0x3b94d700},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x437bdd5b, 0xeac75400},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xba90c787, 0x58fb7400}, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xe01d4e36, 0xfa5c9300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xae119a15, 0x5e070300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x01fb788a, 0x40d30500},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xb34da0d7, 0xa5590000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x409e2c4a, 0x633b3700},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x412056b4, 0xbb732500}
} ;

 #endif

/* 
from HDCP Spec:
Table 51 gives the format of the HDCP SRM. All values are stored in big endian format.

Specify KSVs here in big endian;
*/
#define NUM_REVOKED_KSVS 3
static uint8_t NumRevokedKsvs = NUM_REVOKED_KSVS ;
static const NEXUS_HdmiOutputHdcpKsv RevokedKsvs[NUM_REVOKED_KSVS] =  
{
    /* MSB ... LSB */
    {{0xa5, 0x1f, 0xb0, 0xc3, 0x72}},
    {{0x65, 0xbf, 0x04, 0x8a, 0x7c}},
    {{0x65, 0x65, 0x1e, 0xd5, 0x64}}
} ;
static bool hdmiHdcpEnabled = false ;
static void hdcp_callback(void *pContext, int param)
{
    bool success = (bool)param;
    NEXUS_HdmiOutputHandle handle = pContext;
    NEXUS_HdmiOutputHdcpStatus hdcpStatus;

    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdcpStatus);

	NEXUS_PlatformConfiguration platformConfig;
	NEXUS_Platform_GetConfiguration(&platformConfig);

    if ( success )
    {
        /*fprintf(stderr, "\nHDCP Authentication Successful\n");*/
		BDBG_MSG_TRACE(("\nHDCP Authentication Successful\n"));

		hdmiHdcpEnabled = true ;
    }
    else
    {
		BDBG_MSG_TRACE(("\nHDCP Authentication Failed.  Current State %d\n", hdcpStatus.hdcpState));

        /* always retry */        
        NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);		
    }
}


static void initializeHdcpSettings(void) 
{
    NEXUS_HdmiOutputHdcpSettings *pHdcpSettings;
	NEXUS_PlatformConfiguration platformConfig;
	NEXUS_Platform_GetConfiguration(&platformConfig);
    pHdcpSettings = BKNI_Malloc(sizeof(*pHdcpSettings));
    if ( !pHdcpSettings )
    {
        BDBG_ERR(("Out of memory")) ;
        return ;
    }
		BDBG_MSG_TRACE(("\n Inside %s",__FUNCTION__));
    NEXUS_HdmiOutput_GetHdcpSettings(platformConfig.outputs.hdmi[0], pHdcpSettings);

        /* copy the encrypted key set and its Aksv here  */
        BKNI_Memcpy(pHdcpSettings->encryptedKeySet, encryptedTxKeySet, 
            NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS * sizeof(NEXUS_HdmiOutputHdcpKey)) ;
        BKNI_Memcpy(&pHdcpSettings->aksv, &hdcpTxAksv, NEXUS_HDMI_OUTPUT_HDCP_KSV_LENGTH) ;

        /* install HDCP success  callback */
        pHdcpSettings->successCallback.callback = hdcp_callback;
        pHdcpSettings->successCallback.context = platformConfig.outputs.hdmi[0];
        pHdcpSettings->successCallback.param = true;

        /* install HDCP failure callback */
        pHdcpSettings->failureCallback.callback = hdcp_callback;
        pHdcpSettings->failureCallback.context = platformConfig.outputs.hdmi[0];
        pHdcpSettings->failureCallback.param = false;

    NEXUS_HdmiOutput_SetHdcpSettings(platformConfig.outputs.hdmi[0], pHdcpSettings);

    /* install list of revoked KSVs from SRMs (System Renewability Message) if available */
    NEXUS_HdmiOutput_SetHdcpRevokedKsvs(platformConfig.outputs.hdmi[0], 
        RevokedKsvs, NumRevokedKsvs) ;
     
    BKNI_Free(pHdcpSettings);
}

/*vgd*/

#endif /*BRCM_COPY_PROTECTION_ENABLED*/

static void 
BcmNexus_Hdmi_Hotplug(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;
    NEXUS_DisplaySettings displaySettings;
	NEXUS_Error	rc;
	#if BRCM_COPY_PROTECTION_ENABLED
	NEXUS_PlatformConfiguration platformConfig;
	NEXUS_Platform_GetConfiguration(&platformConfig);
	#endif

	BDBG_ASSERT(hdmi);
	BDBG_ASSERT(display);
    NEXUS_HdmiOutput_GetStatus(hdmi, &status);

	if (BcmNexus_State.hdmiCableConnected != status.connected) {
        if (BcmNexus_State.hdmiCableConnected) {
            BDBG_MSG_TRACE(("Hotplug - Disconnecting HDMI"));
			/* Remove Audio output */
			if (BcmNexus_State.hdmiAudioConnected) {
				BcmNexus_StopAll_AudioDecoders();
				NEXUS_AudioOutput_RemoveInput(NEXUS_HdmiOutput_GetAudioConnector(hdmi), NEXUS_AudioMixer_GetConnector(BcmNexus_State.mixer));
				BcmNexus_State.hdmiAudioConnected = false;
				BcmNexus_StartAll_AudioDecoders();
			}
			/* Remove Video output */
			if (BcmNexus_State.hdmiVideoConnected) {
				NEXUS_Display_RemoveOutput(display, NEXUS_HdmiOutput_GetVideoConnector(hdmi));
				BcmNexus_State.hdmiVideoConnected = false;
			}
        } else {
            NEXUS_Display_GetSettings(display, &displaySettings);
            if (!status.videoFormatSupported[displaySettings.format]) {
                BDBG_MSG_TRACE(("Hotplug - Current format not supported by attached HDMI monitor"));
                displaySettings.format = status.preferredVideoFormat;
                NEXUS_Display_SetSettings(display, &displaySettings);
			} else {
				BDBG_MSG_TRACE(("Hotplug - Connecting HDMI"));
				BDBG_ASSERT(!BcmNexus_State.hdmiAudioConnected);
				BDBG_ASSERT(!BcmNexus_State.hdmiVideoConnected);

				/* Attach HDMI audio */
				
				BcmNexus_StopAll_AudioDecoders();
				
				rc = NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(hdmi), NEXUS_AudioMixer_GetConnector(BcmNexus_State.mixer));
				if (rc==BERR_SUCCESS) {BcmNexus_State.hdmiAudioConnected = true;} else {rc=BERR_TRACE(rc);}
				BcmNexus_StartAll_AudioDecoders();
				

				/* Attach Video output */
				rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(hdmi));
				if (rc==BERR_SUCCESS) {BcmNexus_State.hdmiVideoConnected = true;} else {rc=BERR_TRACE(rc);}
				

	#if BRCM_COPY_PROTECTION_ENABLED 
				/* restart HDCP if it was previously enabled */
				if (gAppEnabledHDCP)
				{
					BDBG_MSG_TRACE(("\n%s, calling NEXUS_HdmiOutput_StartHdcpAuthentication",__FUNCTION__ ));
					NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
				}
	#endif 			
			}
        }
        BcmNexus_State.hdmiCableConnected = status.connected;
    }
}
#endif /* NEXUS_NUM_HDMI_OUTPUTS */

#ifdef _SC_DIRECT_NEXUS_

/* Graphics 2D Engine Synch Routines */
/*_SC_DIRECT_NEXUS_*/
static void BcmNexus_graphics2d_complete(void *data, int unused)
{
	BSTD_UNUSED(unused);
	BKNI_SetEvent((BKNI_EventHandle)data);
}

/*_SC_DIRECT_NEXUS_*/
static void BcmNexus_graphics2d_sync(void)
{
    NEXUS_Error rc;

    /*BDBG_MSG_TRACE((">>platform_nexus_graphics2d_sync"));*/
    BKNI_AcquireMutex(BcmNexus_State.graphics2d_sync_lock);
    rc = NEXUS_Graphics2D_Checkpoint(BcmNexus_State.graphics2d, NULL);
    if(rc==NEXUS_GRAPHICS2D_QUEUED) {
        rc=BKNI_WaitForEvent(BcmNexus_State.graphics2d_sync_event, 500); /* wait 500ms */
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);} /* ignore an error */
    } else if(rc!=NEXUS_SUCCESS) {
        rc=BERR_TRACE(rc); /* ignore an error */
    }
    BKNI_ReleaseMutex(BcmNexus_State.graphics2d_sync_lock);
    /*BDBG_MSG_TRACE(("<<platform_nexus_graphics2d_sync"));*/
    return;
}

/*static void BcmNexus_graphics2d_sync(void);
static void BcmNexus_graphics2d_complete(void *data, int unused);
*/

/*_SC_DIRECT_NEXUS_*/
static NEXUS_VideoFormat
config_get_res(void)
{
	const char *pEnvVar = getenv("res");
	NEXUS_VideoFormat format = NEXUS_VideoFormat_e480p;
	

	if (pEnvVar) {
		if (!strcmp(pEnvVar,"1080p"))
			format = NEXUS_VideoFormat_e1080p;
		else if (!strcmp(pEnvVar,"1080i"))
			format = NEXUS_VideoFormat_e1080i;
		else if (!strcmp(pEnvVar,"720p"))
			format = NEXUS_VideoFormat_e720p;
		else if (!strcmp(pEnvVar,"480i"))
			format = NEXUS_VideoFormat_eNtsc;
	}
	return format;
}

/*_SC_DIRECT_NEXUS_*/
bool
BcmNexus_Platform_Init(void)
{
	NEXUS_PlatformSettings		platformSettings;
	NEXUS_Error					rc;
	NEXUS_Graphics2DOpenSettings graphics2dSettings;
	NEXUS_DisplaySettings		displayCfg;
	NEXUS_PlatformConfiguration	platformConfig;
	NEXUS_DisplayHandle		    display0;
    NEXUS_Graphics2DSettings    graphics2dConfig;

    BDBG_MSG_TRACE(("BcmNexus_Platform_Init (Nexus)"));

    if (BcmNexus_State.initialized) {
        return false;
    }
	
    BcmNexus_State.initialized = true;
    BcmNexus_State.graphics2d_sync = BcmNexus_graphics2d_sync;
	/* Bring up all modules for a platform in a default configuration for this platform */
	NEXUS_Platform_GetDefaultSettings(&platformSettings); 
	platformSettings.openFrontend = false;

#if NEXUS_DTV_PLATFORM
    if(AVIO_CONFIG_IS_PANEL_JEIDA_MODE_JEIDA)
        platformSettings.displayModuleSettings.panel.lvdsMappingRule = NEXUS_PanelOutputMappingRule_eJeidaRule;
    else if(AVIO_CONFIG_IS_PANEL_JEIDA_MODE_OPENLDI)
        platformSettings.displayModuleSettings.panel.lvdsMappingRule = NEXUS_PanelOutputMappingRule_eOpenLdiRule;

    if(AVIO_CONFIG_IS_PANEL_COLOR_MODE_6)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsColorMode = NEXUS_LvdsColorMode_e6Bit;
    else if(AVIO_CONFIG_IS_PANEL_COLOR_MODE_8)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsColorMode = NEXUS_LvdsColorMode_e8Bit;
    else if(AVIO_CONFIG_IS_PANEL_COLOR_MODE_10)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsColorMode = NEXUS_LvdsColorMode_e10Bit;
    else if(AVIO_CONFIG_IS_PANEL_COLOR_MODE_12)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsColorMode = NEXUS_LvdsColorMode_e12Bit;

    if(AVIO_CONFIG_IS_PANEL_LINK_MAP_1_0)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsLinkMap1 = NEXUS_LvdsLinkMap_eInternalLink1;
    else if(AVIO_CONFIG_IS_PANEL_LINK_MAP_1_1)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsLinkMap1 = NEXUS_LvdsLinkMap_eInternalLink2;
    else if(AVIO_CONFIG_IS_PANEL_LINK_MAP_1_2)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsLinkMap1 = NEXUS_LvdsLinkMap_eInternalLink3;
    else if(AVIO_CONFIG_IS_PANEL_LINK_MAP_1_3)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsLinkMap1 = NEXUS_LvdsLinkMap_eInternalLink4;

    if(AVIO_CONFIG_IS_PANEL_LINK_MAP_2_0)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsLinkMap2 = NEXUS_LvdsLinkMap_eInternalLink1;
    else if(AVIO_CONFIG_IS_PANEL_LINK_MAP_2_1)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsLinkMap2 = NEXUS_LvdsLinkMap_eInternalLink2;
    else if(AVIO_CONFIG_IS_PANEL_LINK_MAP_2_2)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsLinkMap2 = NEXUS_LvdsLinkMap_eInternalLink3;
    else if(AVIO_CONFIG_IS_PANEL_LINK_MAP_2_3)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsLinkMap2 = NEXUS_LvdsLinkMap_eInternalLink4;

    if(AVIO_CONFIG_IS_PANEL_LVDS_MODE_SINGLE)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsMode = NEXUS_LvdsMode_eSingleLink;
    else if(AVIO_CONFIG_IS_PANEL_LVDS_MODE_DUAL)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsMode = NEXUS_LvdsMode_eDualLink;
    else if(AVIO_CONFIG_IS_PANEL_LVDS_MODE_QUAD)
        platformSettings.displayModuleSettings.panel.customLinkSettings.lvdsMode = NEXUS_LvdsMode_eQuadLink;

    if( AVIO_CONFIG_IS_PANEL_COLOR_MODE_6 ||
        AVIO_CONFIG_IS_PANEL_COLOR_MODE_8 ||
        AVIO_CONFIG_IS_PANEL_COLOR_MODE_10 ||
        AVIO_CONFIG_IS_PANEL_COLOR_MODE_12 ||
        AVIO_CONFIG_IS_PANEL_LINK_MAP_1_0 ||
        AVIO_CONFIG_IS_PANEL_LINK_MAP_1_1 ||
        AVIO_CONFIG_IS_PANEL_LINK_MAP_1_2 ||
        AVIO_CONFIG_IS_PANEL_LINK_MAP_1_3 ||
        AVIO_CONFIG_IS_PANEL_LINK_MAP_2_0 ||
        AVIO_CONFIG_IS_PANEL_LINK_MAP_2_1 ||
        AVIO_CONFIG_IS_PANEL_LINK_MAP_2_2 ||
        AVIO_CONFIG_IS_PANEL_LINK_MAP_2_3 ||
        AVIO_CONFIG_IS_PANEL_LVDS_MODE_SINGLE ||
        AVIO_CONFIG_IS_PANEL_LVDS_MODE_DUAL ||
        AVIO_CONFIG_IS_PANEL_LVDS_MODE_QUAD)
    {
        platformSettings.displayModuleSettings.panel.dvoLinkMode = NEXUS_PanelOutputLinkMode_eCustom;
    }
#endif /* NEXUS_DTV_PLATFORM */

	rc = NEXUS_Platform_Init(&platformSettings); 
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_nexus;}
    rc = BKNI_CreateMutex(&BcmNexus_State.resource_lock);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_resource_lock;}

	rc = BKNI_CreateEvent(&BcmNexus_State.graphics2d_sync_event);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_sync_event;}
    rc = BKNI_CreateMutex(&BcmNexus_State.graphics2d_sync_lock);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_sync_lock;}

    rc = BKNI_CreateMutex(&BcmNexus_State.key_up_lock);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_key_up_lock;}

	NEXUS_Platform_GetConfiguration(&platformConfig);       	
	NEXUS_Display_GetDefaultSettings(&displayCfg);

#if NEXUS_DTV_PLATFORM
	if (AVIO_CONFIG_IS_PANEL_OUTPUT) {
		rc = NEXUS_BoardCfg_ConfigurePanel(true, true, true);
		BDBG_ASSERT(!rc);
		display0 = NEXUS_Display_Open(0, &displayCfg);
		BDBG_ASSERT(display0);
		rc = NEXUS_Display_AddOutput(display0, NEXUS_PanelOutput_GetConnector(platformConfig.outputs.panel[0]));
		BDBG_ASSERT(!rc);

	} else 
#endif
    {
		displayCfg.format = config_get_res();
		displayCfg.displayType = NEXUS_DisplayType_eAuto;
		display0 = NEXUS_Display_Open(0, &displayCfg);
		BDBG_ASSERT(display0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
		rc = NEXUS_Display_AddOutput(display0, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
		BDBG_ASSERT(!rc);
#endif
#if NEXUS_NUM_SCART_INPUTS
		rc = NEXUS_Display_AddOutput(display0, NEXUS_ScartInput_GetVideoOutputConnector(platformConfig.inputs.scart[0]));
		BDBG_ASSERT(!rc);
#if NEXUS_NUM_SCART_INPUTS>1
		rc = NEXUS_Display_AddOutput(display0, NEXUS_ScartInput_GetVideoOutputConnector(platformConfig.inputs.scart[1]));
		BDBG_ASSERT(!rc);
#endif
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
		if (displayCfg.format == NEXUS_VideoFormat_eNtsc && platformConfig.outputs.composite[0]) {
			rc = NEXUS_Display_AddOutput(display0, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
			BDBG_ASSERT(!rc);
		}
#endif
#if NEXUS_NUM_RFM_OUTPUTS
		if (displayCfg.format == NEXUS_VideoFormat_eNtsc && platformConfig.outputs.rfm[0]) {
			rc = NEXUS_Display_AddOutput(display0, NEXUS_Rfm_GetVideoConnector(platformConfig.outputs.rfm[0]));
			BDBG_ASSERT(!rc);
		}
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
		{
		NEXUS_HdmiOutputSettings hdmiSettings;
		BcmNexus_State.hdmiAudioConnected = false;
		BcmNexus_State.hdmiVideoConnected = false;
		BcmNexus_State.hdmiCableConnected = false;
		NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
		BcmNexus_State.hdmiHotplugCallbackOriginal.callback = hdmiSettings.hotplugCallback.callback;
		hdmiSettings.hotplugCallback.callback = BcmNexus_Hdmi_Hotplug;
		hdmiSettings.hotplugCallback.context = platformConfig.outputs.hdmi[0];
		hdmiSettings.hotplugCallback.param = (int)display0;
		NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
		}

		 /* initalize HDCP settings, keys, etc. */
#if BRCM_COPY_PROTECTION_ENABLED
		if(gAppEnabledHDCP)
			initializeHdcpSettings() ;
#endif

#endif
	}
    BcmNexus_State.display[0] = display0;
    BcmNexus_State.display[1] = NULL;

	NEXUS_Graphics2D_GetDefaultOpenSettings(&graphics2dSettings);
	graphics2dSettings.preAllocPacketMemory = false;
	graphics2dSettings.maxOperations = 1;
	BcmNexus_State.graphics2d = NEXUS_Graphics2D_Open(0, &graphics2dSettings);
    if(!BcmNexus_State.graphics2d) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_graphics2d; }
    NEXUS_Graphics2D_GetSettings(BcmNexus_State.graphics2d, &graphics2dConfig);
    graphics2dConfig.checkpointCallback.callback = BcmNexus_graphics2d_complete;
    graphics2dConfig.checkpointCallback.context = BcmNexus_State.graphics2d_sync_event;
	if (AVIO_CONFIG_IS_BVN_USAGE_CONFIG9) {
		graphics2dConfig.blockedSync = true;
	}

	rc = NEXUS_Graphics2D_SetSettings(BcmNexus_State.graphics2d, &graphics2dConfig);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_graphics2d_config;}

    if(!BcmNexus_Platform_Init_Audio()) { goto err_audio;}

#if NEXUS_NUM_HDMI_OUTPUTS

	/* Force a hotplug to check HDMI status */
	BcmNexus_Hdmi_Hotplug(platformConfig.outputs.hdmi[0], (int)display0);
#endif
	
	/* config9 is always double buffered */
	if (AVIO_CONFIG_IS_BVN_USAGE_CONFIG9) {
		BcmNexus_State.doublebuffer = true;
	}

	if (getenv("double_buffer") && !strcmp(getenv("double_buffer"), "y")) {
		BcmNexus_State.doublebuffer = true;
	}
    return true;

err_audio:
err_graphics2d_config:
err_graphics2d:
err_sync_lock:
err_sync_event:
err_key_up_lock:
err_resource_lock:
err_nexus:
    return false;
}

/*_SC_DIRECT_NEXUS_*/
void 
BcmNexus_Platform_Uninit(void)
{
    BDBG_ASSERT(BcmNexus_State.initialized);
    BDBG_MSG_TRACE(("BcmNexus_Platform_Uninit"));
	

	#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Platform_GetConfiguration(&platformConfig);
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = BcmNexus_State.hdmiHotplugCallbackOriginal.callback;
    NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);

#if BRCM_COPY_PROTECTION_ENABLED 	
	NEXUS_HdmiOutput_DisableHdcpAuthentication(platformConfig.outputs.hdmi[0]);
      gAppEnabledHDCP=false;      
#endif
#endif

	NEXUS_Graphics2D_Close(BcmNexus_State.graphics2d); 
	NEXUS_Display_RemoveAllOutputs(BcmNexus_State.display[0]);	
	NEXUS_Display_Close(BcmNexus_State.display[0]); 
    BKNI_DestroyEvent(BcmNexus_State.graphics2d_sync_event);
    BKNI_DestroyMutex(BcmNexus_State.graphics2d_sync_lock);
    BKNI_DestroyMutex(BcmNexus_State.key_up_lock);
    BKNI_DestroyMutex(BcmNexus_State.resource_lock);
    BcmNexus_Platform_Uninit_Audio();
	NEXUS_Platform_Uninit();
    BcmNexus_State.initialized = false;


    return;
}

/*_SC_DIRECT_NEXUS_*/
BDBG_OBJECT_ID(BcmNexus_Graphics_Resources);
BDBG_OBJECT_ID(BcmNexus_Graphics_Window);
struct BcmNexus_Graphics_Window_Impl {
    BcmNexus_Graphics_Window public;
	NEXUS_Rect position;
    BDBG_OBJECT(BcmNexus_Graphics_Window)
	BLST_S_ENTRY(BcmNexus_Graphics_Window_Impl) link; /* field that is used to link windows together */
    struct BcmNexus_Graphics_Resources_Impl *parent;
};

/*_SC_DIRECT_NEXUS_*/
struct BcmNexus_Graphics_Resources_Impl {
    BcmNexus_Graphics_Resources public;
    BDBG_OBJECT(BcmNexus_Graphics_Resources)
	BLST_S_HEAD(BcmNexus_Graphics_Resources_Impl_List, BcmNexus_Graphics_Window_Impl) windows;
    struct {
        NEXUS_SurfaceHandle surface;
        uint16_t pitch;
        uint16_t width, height;
        void *buffer;
    } framebuffer;
};

/*_SC_DIRECT_NEXUS_*/
unsigned int checksum(const BcmNexus_Graphics_Window *window_, const NEXUS_Rect *rect)
{
    unsigned int * pFrameBufferLine;
    unsigned int chk = 0;
    int x,y;

    struct BcmNexus_Graphics_Window_Impl *window =  (struct BcmNexus_Graphics_Window_Impl *)window_;

    for (y = 0; y < rect->height; y++) {
        pFrameBufferLine = (unsigned int *)window->parent->framebuffer.buffer + (window->position.x + rect->x) + 
                                            ((window->position.y + rect->y + y) * (window->parent->framebuffer.pitch >> 2));

        for (x = 0; x < rect->width; x++) {
            chk += pFrameBufferLine[x];
        }
    }

    return chk;
}

/*_SC_DIRECT_NEXUS_*/
void screen_capture(void)
{
    unsigned int * pFrameBufferLine;
    int y;
    FILE * output;

    char header[0x36] = {0x42,0x4d,
                         0x00,0x00,0x00,0x00, /*total size*/
                         0x00,0x00,
                         0x00,0x00,
                         0x36,0x00,0x00,0x00,
                         0x28,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,/*width*/
                         0x00,0x00,0x00,0x00,/*height*/
                         0x01,0x00,
                         0x20,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00, /*image size*/
                         0x13,0x0b,0x00,0x00,
                         0x13,0x0b,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00};

    *(int*)(header+0x12) = BcmNexus_State.graphics_resources->framebuffer.width;
    *(int*)(header+0x16) = BcmNexus_State.graphics_resources->framebuffer.height;
    *(int*)(header+0x22) = *(int*)(header+0x12) * *(int*)(header+0x16);
    *(int*)(header+0x2) = sizeof(header) + *(int*)(header+0x22);

    output = fopen("/tmp/screencap.bmp","wb");

    if (!output) {
	    BDBG_MSG_TRACE(("Unable to save 'screencap.bmp'"));
        return;
    }

    fwrite(header,sizeof(header),1,output);

    y = BcmNexus_State.graphics_resources->framebuffer.height;
    while (y--) {

        pFrameBufferLine = (unsigned int *)BcmNexus_State.graphics_resources->framebuffer.buffer +
                                     (y * (BcmNexus_State.graphics_resources->framebuffer.pitch >> 2));

        fwrite(pFrameBufferLine, BcmNexus_State.graphics_resources->framebuffer.width*4, 1, output);
    }

    fclose(output);
}

/*_SC_DIRECT_NEXUS_*/
static void
BcmNexus_Graphics_Update_Window(const BcmNexus_Graphics_Window *window_, const NEXUS_Rect *rect)
{
    static unsigned int counter = 1;
    static unsigned int mismatch = 0;
    static unsigned int previous_checksum = 0xDEADBEEF;
    static NEXUS_Rect previous_rect = {0,0,0,0};
    unsigned int current_checksum;
    unsigned int key = 0xDEADBEEF;
    unsigned int direction = 0xDEADBEEF;

    struct BcmNexus_Graphics_Window_Impl *window =  (struct BcmNexus_Graphics_Window_Impl *)window_;
	/*BDBG_MSG_TRACE(("BcmNexus_Graphics_Update_Window: %#lx %d,%d,%u,%u", (unsigned long)window, rect->x, rect->y, rect->width, rect->height));*/
	BDBG_OBJECT_ASSERT(window, BcmNexus_Graphics_Window);
    /* Nothing to do here window goes straight to the display framebuffer */
	BSTD_UNUSED(rect);
    /* Window update window postion wrt framebuffer */
	/*BDBG_MSG_TRACE(("BcmNexus_Graphics_Update_Window: framebuffer 0x%x (%d,%d,%d,%d)", (unsigned long)window, 
		(window->position.x + rect->x), (window->position.y + rect->y), rect->width, rect->height));*/

    if (TestHarnessGetMode() != TestHarnessModeNone) {

        /* We generate a checksum in input training mode since we need to know when to
           discard duplicate blits that come from Stagecraft. */

        current_checksum = checksum(window_, rect);

        if ((current_checksum == previous_checksum) &&
            (rect->width == previous_rect.width) && (rect->height == previous_rect.height) &&
            (rect->x == previous_rect.x) && (rect->y == previous_rect.y)) {
            return;
        } else {
            previous_checksum    = current_checksum;
            previous_rect.width  = rect->width;
            previous_rect.height = rect->height;
            previous_rect.x      = rect->x;
            previous_rect.y      = rect->y;
        }

        if ((TestHarnessGetMode() == TestHarnessModeInputCapture) ||
            (TestHarnessGetMode() == TestHarnessModeOutputCapture)) {
            TestHarnessAddFrameHash(current_checksum);
        }

        if (TestHarnessGetMode() == TestHarnessModePlayback) {
            if (current_checksum != TestHarnessGetFrameHash(counter)) {
                TestHarnessFail(counter);
                mismatch++;
            }
        }

        if ((TestHarnessGetMode() == TestHarnessModeOutputCapture) ||
            (TestHarnessGetMode() == TestHarnessModePlayback)) {
            while (TestHarnessGetKey(counter, &key, &direction)) {
                if (direction == KEY_DIRECTION_DOWN) {
                    BcmNexus_State.inputResource->public.config.key_down_callback.callback(BcmNexus_State.inputResource->public.config.key_down_callback.context, key );
                } else {
                    BcmNexus_State.inputResource->public.config.key_up_callback.callback(BcmNexus_State.inputResource->public.config.key_up_callback.context, key );
                }
            }
        }

        if (TestHarnessGetMode() == TestHarnessModePlayback) {
            if ((key == 0x00400100) && (!mismatch)) {
                TestHarnessPass();
            }
        }

        counter++;
    }

    /* Nothing to do here window goes straight to the display framebuffer */
    return;
}

/*_SC_DIRECT_NEXUS_*/
static void
BcmNexus_Flip(void *fp, int unused)
{
	NEXUS_Error rc;
	NEXUS_Graphics2DBlitSettings blitSettings;
	struct BcmNexus_Graphics_Resources_Impl	*resources = (struct BcmNexus_Graphics_Resources_Impl *) fp;
	BSTD_UNUSED(unused);

	/*BDBG_MSG_TRACE(("Flip currentbuffer=%s", BcmNexus_State.current?"back":"front"));*/
	BcmNexus_State.current = (BcmNexus_State.current+1)%2;
	/* Blit the full framebuffer onto the compressed surface */
	NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
	blitSettings.source.surface = resources->framebuffer.surface;
	blitSettings.output.surface = BcmNexus_State.displayFramebuffer[BcmNexus_State.current];
	rc = NEXUS_Graphics2D_Blit(BcmNexus_State.graphics2d, &blitSettings);
	if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}	
	BcmNexus_graphics2d_sync();

	/* Set this surface as the framebuffer */
    rc = NEXUS_Display_SetGraphicsFramebuffer(BcmNexus_State.display[0], BcmNexus_State.displayFramebuffer[BcmNexus_State.current]);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
}

/*_SC_DIRECT_NEXUS_*/
static void
BcmNexus_Graphics_Destroy_Window(const BcmNexus_Graphics_Window *window_)
{
    struct BcmNexus_Graphics_Window_Impl *window =  (struct BcmNexus_Graphics_Window_Impl *)window_;
	BDBG_MSG_TRACE(("BcmNexus_Graphics_Destroy_Window"));
	BDBG_OBJECT_ASSERT(window, BcmNexus_Graphics_Window);
    BcmNexus_Lock();
	BLST_S_REMOVE(&window->parent->windows, window, BcmNexus_Graphics_Window_Impl, link);
    BcmNexus_Unlock();
	if (window->public.surface) {
		NEXUS_Surface_Destroy(window->public.surface);
	}
	BDBG_OBJECT_DESTROY(window, BcmNexus_Graphics_Window);
    BKNI_Free(window);
    return;
}

/*_SC_DIRECT_NEXUS_*/
static const BcmNexus_Graphics_Window *
BcmNexus_Graphics_Create_Window(const struct BcmNexus_Graphics_Resources *resources_, NEXUS_Rect *rect, bool defaultPosition, NEXUS_PixelFormat format, const char *title)
{
	struct BcmNexus_Graphics_Resources_Impl *resources = (struct BcmNexus_Graphics_Resources_Impl *)resources_;
	NEXUS_SurfaceCreateSettings	createSettings;
    NEXUS_Rect                  position;
    struct BcmNexus_Graphics_Window_Impl *window;

	BDBG_MSG_TRACE(("BcmNexus_Graphics_Create_Window"));
	BDBG_OBJECT_ASSERT(resources, BcmNexus_Graphics_Resources);
	BDBG_ASSERT(rect);
	BSTD_UNUSED(title);
    
    window = BKNI_Malloc(sizeof(*window));
    BDBG_OBJECT_INIT(window, BcmNexus_Graphics_Window);
    window->parent = resources;

	NEXUS_Surface_GetDefaultCreateSettings(&createSettings);

	if (AVIO_CONFIG_IS_BVN_USAGE_CONFIG9) {
		/* Use width=1920 for calculating the position of the output window */
		resources->framebuffer.width *= 2; 
	}
	if ((rect->x >= resources->framebuffer.width) || (rect->y >= resources->framebuffer.height)) {
		BDBG_WRN(("Window(%d,%d,%d,%d) outside the framebuffer(%dx%d)", rect->x, rect->y, rect->width, rect->height,
			resources->framebuffer.width, resources->framebuffer.height));
		goto err_create_surface;
	}
	/* Create windows from the framebuffer, make sure the size is within the framebuffer size */
	if (resources->framebuffer.width < (rect->x+rect->width)) {
		createSettings.width = resources->framebuffer.width - rect->x;
	} else {
		createSettings.width = rect->width;
	}
	if (resources->framebuffer.height < (rect->y+rect->height)) {
		createSettings.height = resources->framebuffer.height - rect->y;
	} else {
		createSettings.height = rect->height;
	}
	createSettings.pixelFormat = format;

	if (defaultPosition) {
		/* position in the center of the screen */
		if (resources->framebuffer.width >= rect->width) {
			position.x = (resources->framebuffer.width - rect->width)/2;
		} else {
			position.x = 0;
		}
		if (resources->framebuffer.height >= rect->height) {
			position.y = (resources->framebuffer.height - rect->height)/2;
		} else {
			position.y = 0;
		}
	} else {
		position.x = rect->x;
		position.y = rect->y;
	}

	if (AVIO_CONFIG_IS_BVN_USAGE_CONFIG9) {
		/* Revert width back to the original framebuffer for calculating the output window */
		resources->framebuffer.width /= 2; 
		createSettings.width /= 2;
		position.x /= 2;
	}

 	BDBG_MSG_TRACE(("BcmNexus_Graphics_Create_Window: Window x=%d y=%d width=%u height=%u   Primary width=%d height=%d ",
 		position.x, position.y, createSettings.width, createSettings.height, resources->framebuffer.width, resources->framebuffer.height));

	/* Set the pitch and memory from the framebuffer */
	createSettings.pitch = resources->framebuffer.pitch;
	createSettings.pMemory = (uint8_t *)resources->framebuffer.buffer + (position.y*resources->framebuffer.pitch) + (position.x * (resources->framebuffer.pitch/resources->framebuffer.width));

#if BCHP_CHIP == 7420
	{
		NEXUS_PlatformConfiguration platformConfig;
		NEXUS_Platform_GetConfiguration(&platformConfig);
		/* Specify heap1 as nexus defaults to heap0 */
		createSettings.heap = platformConfig.heap[1];
	}
#endif

	window->public.surface = NEXUS_Surface_Create(&createSettings);
	if (!window->public.surface) {	BDBG_ASSERT(window->public.surface); goto err_create_surface; }
    BcmNexus_Lock();
	BLST_S_INSERT_HEAD(&resources->windows, window, link);
    BcmNexus_Unlock();
    /* update window position */
    rect->x = position.x;
    rect->y = position.y;
    rect->width = createSettings.width;
    rect->height = createSettings.height;
	window->position = *rect;
	return &window->public;

err_create_surface:
    BKNI_Free(window);
	return NULL;
}

/*_SC_DIRECT_NEXUS_*/
const BcmNexus_Graphics_Resources *
BcmNexus_Graphics_Resources_Acquire(BcmNexus_Graphics_Resources_Config *config)
{
	NEXUS_DisplayHandle					display0;
	NEXUS_DisplaySettings		        displaySettings;
    NEXUS_GraphicsSettings              graphicsSettings;
	NEXUS_Error							rc;
	NEXUS_SurfaceMemory					mem;
	NEXUS_SurfaceCreateSettings			createSettings;
	NEXUS_VideoFormatInfo				formatInfo;
	NEXUS_SurfaceHandle					surface, framebuffer;
    struct BcmNexus_Graphics_Resources_Impl	*resources;

    BDBG_ASSERT(config);
	
    BcmNexus_Lock();
    BDBG_MSG_TRACE((">>BcmNexus_Graphics_Resources_Acquire"));
    display0 = BcmNexus_State.display[0];

	resources =  BKNI_Malloc(sizeof(*resources));
    if(!resources) { goto err_alloc; }
    BDBG_OBJECT_INIT(resources, BcmNexus_Graphics_Resources);
	BLST_S_INIT(&resources->windows);
    resources->public.config = *config;
	resources->framebuffer.surface = NULL;
	BLST_S_INIT(&resources->windows);

	/* Create Fullscreen primary surface */
    NEXUS_Display_GetSettings(display0, &displaySettings);
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    NEXUS_VideoFormat_GetInfo(displaySettings.format ,&formatInfo);

	if (AVIO_CONFIG_IS_BVN_USAGE_CONFIG9) {
		/* Width is half of 1080p, height is the same as 1080p */
		createSettings.width = 960;
		createSettings.height = 1080;
	} else {
		createSettings.width = formatInfo.width;
		createSettings.height = formatInfo.height;
	}
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
	createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    surface = NEXUS_Surface_Create(&createSettings);
    if (surface == NULL) {
        goto err_frameBuffer;
    }
	framebuffer = surface;

    /* fill with black */
	NEXUS_Surface_GetMemory(surface, &mem);
	BKNI_Memset(mem.buffer, 0x0, createSettings.height * mem.pitch);
    NEXUS_Surface_Flush(surface);

	/* Create a commpressed frame buffer of the same size */
	if (AVIO_CONFIG_IS_BVN_USAGE_CONFIG9)
		createSettings.bitsPerPixel = 42;
	else if (AVIO_CONFIG_IS_BVN_USAGE_CONFIG11)
		createSettings.bitsPerPixel = 41;
	else
		createSettings.bitsPerPixel = 0;

	if (BcmNexus_State.doublebuffer) {
		BcmNexus_State.displayFramebuffer[0] = NEXUS_Surface_Create(&createSettings);
		if (BcmNexus_State.displayFramebuffer[0] == NULL) {
			goto err_display_framebuffer0;
		}
		framebuffer = BcmNexus_State.displayFramebuffer[0];

		BcmNexus_State.displayFramebuffer[1] = NEXUS_Surface_Create(&createSettings);
		if (BcmNexus_State.displayFramebuffer[1] == NULL) {
			goto err_display_framebuffer1;
		}
	}

    NEXUS_Display_GetGraphicsSettings(display0, &graphicsSettings);
    graphicsSettings.enabled = true;
	graphicsSettings.position.x = 0;
	graphicsSettings.position.y = 0;
	if (AVIO_CONFIG_IS_BVN_USAGE_CONFIG9) {
		graphicsSettings.position.width = formatInfo.width;
	} else {
		graphicsSettings.position.width = createSettings.width;
	}
	graphicsSettings.position.height = createSettings.height;
	graphicsSettings.clip.width = createSettings.width;
	graphicsSettings.clip.height = createSettings.height;
	if (BcmNexus_State.doublebuffer) {
		graphicsSettings.frameBufferCallback.callback = BcmNexus_Flip;
		graphicsSettings.frameBufferCallback.context = resources;
	}

	resources->framebuffer.surface = surface;
	resources->framebuffer.width = createSettings.width;
	resources->framebuffer.height = createSettings.height;
	resources->framebuffer.pitch = mem.pitch;
	resources->framebuffer.buffer = mem.buffer;
    BcmNexus_State.graphics_resources = resources;

	rc = NEXUS_Display_SetGraphicsSettings(display0, &graphicsSettings);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_setGraphicsSettings;}
    rc = NEXUS_Display_SetGraphicsFramebuffer(display0, framebuffer);
    if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_setGraphicsFramebuffer;}

	resources->public.graphics2d = BcmNexus_State.graphics2d;
    resources->public.graphicsSync = BcmNexus_graphics2d_sync;
    resources->public.createWindow = BcmNexus_Graphics_Create_Window;
    resources->public.windowUpdated = BcmNexus_Graphics_Update_Window;
    resources->public.destroyWindow = BcmNexus_Graphics_Destroy_Window;
    BDBG_MSG_TRACE(("<<BcmNexus_Graphics_Resources_Acquire:%#lx", (unsigned long)resources));
    BcmNexus_Unlock();
	return &resources->public;

err_display_framebuffer1:
	if (BcmNexus_State.doublebuffer) {
		NEXUS_Surface_Destroy(BcmNexus_State.displayFramebuffer[0]);
	}
err_display_framebuffer0:
err_setGraphicsFramebuffer:
err_setGraphicsSettings:
    NEXUS_Surface_Destroy(surface);
err_frameBuffer:
    BKNI_Free(resources);
err_alloc:
    BcmNexus_Unlock();
    return NULL;
}

/*_SC_DIRECT_NEXUS_*/
void BcmNexus_Graphics_Resources_Release(const BcmNexus_Graphics_Resources *resources_)
{	
    struct BcmNexus_Graphics_Resources_Impl *resources = (struct BcmNexus_Graphics_Resources_Impl *)resources_;
    struct BcmNexus_Graphics_Window_Impl *window;
    NEXUS_GraphicsSettings              graphicsSettings;

	BDBG_MSG_TRACE(("BcmNexus_Graphics_Resources_Release"));
    BDBG_OBJECT_ASSERT(resources, BcmNexus_Graphics_Resources);
    BcmNexus_Lock();

	while(NULL!=(window=BLST_S_FIRST(&resources->windows))) {
        BcmNexus_Graphics_Destroy_Window(&window->public);
    }
    NEXUS_Display_GetGraphicsSettings(BcmNexus_State.display[0], &graphicsSettings);
    graphicsSettings.enabled = false;
    NEXUS_Display_SetGraphicsSettings(BcmNexus_State.display[0], &graphicsSettings);
    NEXUS_Surface_Destroy(resources->framebuffer.surface);

    BDBG_OBJECT_DESTROY(resources, BcmNexus_Graphics_Resources);
    BKNI_Free(resources);
    BcmNexus_Unlock();

	/* Platform is shutdown at this point */
    if (BcmNexus_State.autoInitialized) {
        BcmNexus_State.autoInitialized = false;
	    BcmNexus_Platform_Uninit();
        TestHarnessTerminate();
    }
    return;
}

#else /*_SC_DIRECT_NEXUS_*/

/*_SC_DIRECRFB_*/
bool
BcmNexus_Platform_Init(void)
{
    NEXUS_Error rc;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_PlatformConfiguration platformConfig;
#endif
    NEXUS_DisplayHandle	display0;
	DFB_PlatformSettings platformSettings;
    BDBG_MSG_TRACE(("BcmNexus_Platform_Init (DirectFB)"));

    if (BcmNexus_State.initialized) {
        return false;
    }

	/* Get DirectFB Platform settings */
    DFB_Platform_GetSettings(&platformSettings);

	BKNI_Memset(&BcmNexus_State, 0, sizeof(BcmNexus_State));
    BcmNexus_State.initialized = true;
	BcmNexus_State.display[0] = platformSettings.display[0].handle;
    BcmNexus_State.graphics2d = platformSettings.graphics2d.handle;
    BcmNexus_State.graphics2d_sync = NULL;

	/* Bring up all modules for a platform in a default configuration for this platform */
    rc = BKNI_CreateMutex(&BcmNexus_State.resource_lock);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_resource_lock;}
#ifdef NEXUS_HAS_PICTURE_DECODER
    BcmNexus_State.pictureDecoder = platformSettings.pictureDecoder.handle; /*nexus_picturedecoder_handle*/
#endif
    if(!BcmNexus_Platform_Init_Audio()) { goto err_audio; }

    display0 = BcmNexus_State.display[0];

#if NEXUS_NUM_HDMI_OUTPUTS
    BcmNexus_State.hdmiAudioConnected = false;
    BcmNexus_State.hdmiVideoConnected = false;
    BcmNexus_State.hdmiCableConnected = false;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
    BcmNexus_State.hdmiHotplugCallbackOriginal.callback = hdmiSettings.hotplugCallback.callback;
    hdmiSettings.hotplugCallback.callback = BcmNexus_Hdmi_Hotplug;
    hdmiSettings.hotplugCallback.context = platformConfig.outputs.hdmi[0];
    hdmiSettings.hotplugCallback.param = (int)display0;
    NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);

    BcmNexus_Hdmi_Hotplug(platformConfig.outputs.hdmi[0], (int)display0);
#endif
#ifdef _DFB_TEST_HARNESS_
	TestHarnessStart();
#endif
    return true;
err_audio:
err_resource_lock:
    return false;
}

/*_SC_DIRECRFB_*/
void 
BcmNexus_Platform_Uninit(void)
{

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Platform_GetConfiguration(&platformConfig);
#endif

    BDBG_MSG_TRACE(("BcmNexus_Platform_Uninit (DirectFB)"));
    BDBG_ASSERT(BcmNexus_State.initialized);

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = BcmNexus_State.hdmiHotplugCallbackOriginal.callback;
    NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
#endif

    BKNI_DestroyMutex(BcmNexus_State.resource_lock);
    BcmNexus_Platform_Uninit_Audio();
    BcmNexus_State.initialized = false;
#ifdef _DFB_TEST_HARNESS_
	TestHarnessTerminate();
#endif
}

/*_SC_DIRECRFB_*/
const BcmNexus_Graphics_Resources *
BcmNexus_Graphics_Resources_Acquire(BcmNexus_Graphics_Resources_Config *config)
{
	BcmNexus_Graphics_Resources			*resources;
	BDBG_ASSERT(config);

	BcmNexus_Lock();
	resources =  BKNI_Malloc(sizeof(*resources));
	if(!resources) { goto err_alloc; }

	resources->graphics2d = BcmNexus_State.graphics2d;
	resources->graphicsSync = BcmNexus_State.graphics2d_sync;

    BDBG_MSG_TRACE(("BcmNexus_Graphics_Resources_Acquire (Hybrid):%#lx", (unsigned long)resources));
    BcmNexus_Unlock();
	return resources;

err_alloc:
    BcmNexus_Unlock();
    return NULL;
}

/*_SC_DIRECRFB_*/
void BcmNexus_Graphics_Resources_Release(const BcmNexus_Graphics_Resources *resources_)
{
	struct BcmNexus_Graphics_Resources_Impl *resources = (struct BcmNexus_Graphics_Resources_Impl *)resources_;
	BDBG_MSG_TRACE(("BcmNexus_Graphics_Resources_Release (Hybrid)"));

    BKNI_Free(resources);

    return;
}

#endif /*_SC_DIRECT_NEXUS_*/

bool get_automaster(void)
{
    /* "false": The device output frame rate is fixed and disregards video being decoded. */
    /* "true":  The device output frame rate may change based on the video being decoded. */
	const char *pEnvVar = getenv("automaster");
	if (pEnvVar) {
		if (!strcmp(pEnvVar,"n")) {
			return false;
		}
	}

	return true;
}

static BcmNexus_Resource BcmNexus_Resource_SoundOutput = RESOURCES_INITALIZER(1/*NEXUS_NUM_AUDIO_PLAYBACKS*/);
BDBG_OBJECT_ID(BcmNexus_SoundOutput_Resources);
typedef struct BcmNexus_SoundOutput_Resources_Impl {
    BcmNexus_SoundOutput_Resources public;
    BDBG_OBJECT(BcmNexus_SoundOutput_Resources)
    int id;
} BcmNexus_SoundOutput_Resources_Impl;

NEXUS_Error 
BcmNexus_SoundOutput_SetVolume(BcmNexus_SoundOutput_Volume volume, unsigned level)
{
    int32_t audioVolume = (0x800000/100)*level; /* Linear scaling - values are in a 4.23 format, 0x800000 = unity.  */
    NEXUS_Error rc;
	BDBG_MSG_TRACE(("BcmNexus_SoundOutput_SetVolume level=%d", level));
    switch(volume) {
    case BcmNexus_SoundOutput_Volume_ePcm:
        {
            NEXUS_AudioPlaybackSettings settings;
            NEXUS_AudioPlayback_GetSettings(BcmNexus_State.pcmPlayback[0], &settings);
            settings.leftVolume = audioVolume;
            settings.rightVolume = audioVolume;
            rc = NEXUS_AudioPlayback_SetSettings(BcmNexus_State.pcmPlayback[0], &settings);
            if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
            break;
        }
    case BcmNexus_SoundOutput_Volume_eDecoder:
        {
            NEXUS_AudioDecoderSettings settings;

            NEXUS_AudioDecoder_GetSettings(BcmNexus_State.audioDecoder[0], &settings);
            settings.volumeMatrix[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft] = audioVolume;
            settings.volumeMatrix[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = audioVolume;
            settings.volumeMatrix[NEXUS_AudioChannel_eCenter][NEXUS_AudioChannel_eCenter] = audioVolume;
            rc=NEXUS_AudioDecoder_SetSettings(BcmNexus_State.audioDecoder[0], &settings);
            if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
            break;
        }
    default:
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return NEXUS_SUCCESS;
}

/* The sound driver updates the BcmNexus_State decoder state */
void
BcmNexus_Pcm_Status_Update(const BcmNexus_SoundOutput_Resources *resources_, bool started)
{
	BcmNexus_SoundOutput_Resources_Impl *resources = (BcmNexus_SoundOutput_Resources_Impl *)resources_;
	BcmNexus_State.pcmStarted[resources->id] = started;
}

const BcmNexus_SoundOutput_Resources *
BcmNexus_SoundOutput_Resources_Acquire(const BcmNexus_SoundOutput_Resources_Config  *config)
{
    NEXUS_Error rc;
    BcmNexus_SoundOutput_Resources_Impl *resources;

    BDBG_MSG_TRACE(("BcmNexus_SoundOutput_Resources_Acquire"));

    BDBG_ASSERT(config);
    if(config->fifoSize > BCM_NEXUS_PCM_FIFO_SIZE) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_config; }

    resources = BKNI_Malloc(sizeof(*resources));
    if(!resources) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_alloc; }

    BDBG_OBJECT_INIT(resources, BcmNexus_SoundOutput_Resources);
    resources->public.config = *config;

    BcmNexus_Lock();
    resources->id = BcmNexus_Resource_Acquire(&BcmNexus_Resource_SoundOutput);
    BcmNexus_Unlock();
    if(resources->id<0) {BDBG_WRN(("SoundOuputResource Already Accquired")); goto err_resource; }

    BDBG_MSG(("BcmNexus_SoundOutput_Resources_Acquire:%#lx using pcm playback %d", (unsigned long)resources, resources->id ));

	resources->public.pcmStatusUpdate = BcmNexus_Pcm_Status_Update;
    resources->public.pcmPlayback = BcmNexus_State.pcmPlayback[resources->id];
    if (resources->public.pcmPlayback==NULL) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_pcm_playback;}
	BcmNexus_State.pcmSetting[resources->id] = config->startSettings;

    BDBG_MSG_TRACE(("BcmNexus_SoundOutput_Resources_Acquire: return %#lx", (unsigned long)resources));
    return &resources->public;

err_pcm_playback:
    BcmNexus_Resource_Release(&BcmNexus_Resource_SoundOutput, resources->id);

err_resource:
    BKNI_Free(resources);
err_alloc:
err_config:
    return NULL;
}

void 
BcmNexus_SoundOutput_Resources_Release(const BcmNexus_SoundOutput_Resources *resources_)
{
    BcmNexus_SoundOutput_Resources_Impl *resources = (BcmNexus_SoundOutput_Resources_Impl *)resources_;

    BDBG_MSG_TRACE(("BcmNexus_SoundOutput_Resources_Release: %#lx", (unsigned long)resources));

    BDBG_OBJECT_ASSERT(resources, BcmNexus_SoundOutput_Resources);
    BcmNexus_Lock();
    BcmNexus_Resource_Release(&BcmNexus_Resource_SoundOutput, resources->id);
    BcmNexus_Unlock();
    BDBG_OBJECT_DESTROY(resources, BcmNexus_SoundOutput_Resources);
    BKNI_Free(resources);
    return;
}

/* The streamplayer driver updates the BcmNexus_State decoder state */
void
BcmNexus_Audio_Status_Update(const BcmNexus_StreamPlayer_Resources *resources, bool started)
{
	BDBG_ASSERT(resources);
	BcmNexus_State.audioStarted[0] = started;
}

NEXUS_Error
BcmNexus_SetVideoRegion(const BcmNexus_StreamPlayer_Resources *resources, NEXUS_Rect *planeRect, NEXUS_Rect *rect, NEXUS_Rect *clipRect)
{
    NEXUS_VideoWindowSettings windowSettings;
	NEXUS_Error rc;

	BDBG_MSG(("BcmNexus_SetVideoRegion:rect %d,%dx%u,%u clip:%d,%dx%u,%u", (int)rect->x, (int)rect->y, (unsigned)rect->width, (unsigned)rect->height, (int)clipRect->x, (int)clipRect->y, (unsigned)clipRect->width, (unsigned)clipRect->height));
	BDBG_MSG(("BcmNexus_SetVideoRegion: planeRect %d,%dx%u,%u scale x=%d y=%d", (int)planeRect->x, (int)planeRect->y, (int)planeRect->width, (int)planeRect->height, resources->scaleX, resources->scaleY));
    NEXUS_VideoWindow_GetSettings(resources->videoWindow, &windowSettings);
	windowSettings.visible = true;
	windowSettings.position.x = (planeRect->x + rect->x) * resources->scaleX;
	windowSettings.position.y = (planeRect->y + rect->y) * resources->scaleY;
	windowSettings.position.width  = rect->width * resources->scaleX;
	windowSettings.position.height = rect->height * resources->scaleY;
	if (planeRect->width && planeRect->height) {
		if (windowSettings.position.width > (planeRect->width * resources->scaleX)) {
		   windowSettings.position.width = planeRect->width * resources->scaleX;
		} 
		if (windowSettings.position.height > (planeRect->height * resources->scaleY)) {
		   windowSettings.position.height = (planeRect->height * resources->scaleY);
		} 
	}

	BDBG_MSG(("BcmNexus_SetVideoRegion: Final windowSettings %d,%d x %d,%d", windowSettings.position.x, windowSettings.position.y, windowSettings.position.width, windowSettings.position.height));
    rc = NEXUS_VideoWindow_SetSettings(resources->videoWindow, &windowSettings);
    if (rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); }

    return rc;
}

const BcmNexus_StreamPlayer_Resources *
BcmNexus_StreamPlayer_Resources_Acquire( const BcmNexus_StreamPlayer_Resources_Config *config)
{
	NEXUS_Error rc;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
	NEXUS_PlaypumpSettings playpumpSettings;
	NEXUS_VideoDecoderSettings videoDecoderSettings;
	BcmNexus_StreamPlayer_Resources *resources;
    NEXUS_VideoWindowSettings windowSettings;
	int i;

	if (BcmNexus_State.streamResource) {
		BDBG_MSG_TRACE(("BcmNexus_StreamPlayer_Resources_Acquire: Already acquired!!"));
		return NULL;
	}

	BDBG_ASSERT(config);
	BDBG_ASSERT(config->numPlaypumps <= sizeof(resources->playpump)/sizeof(*resources->playpump));
	BDBG_MSG_TRACE(("BcmNexus_StreamPlayer_Resources_Acquire"));
	
if(config->enableHDCP==true){
	gAppEnabledHDCP=true;
}
	BcmNexus_Lock();
	resources= BKNI_Malloc(sizeof(*resources));
	if(!resources) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }

    resources->scaleX = 1;
    resources->scaleY = 1;
	resources->config = *config;
	
	resources->stageWindow = config->stageWindow;
	

#if NEXUS_DTV_PLATFORM
    if (AVIO_CONFIG_IS_BVN_USAGE_CONFIG9) {
        resources->scaleX = 2;
    }
#endif

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto; /* playback */
    resources->stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
	if(!resources->stcChannel) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_stcChannel; }

    resources->videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
	if(!resources->videoDecoder) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_videoDecoder; }

	resources->audioDecoder[0] = NULL;
	resources->audioDecoder[1] = NULL;
	resources->audioDecoder[0] = BcmNexus_State.audioDecoder[0];
	if(!resources->audioDecoder[0]) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_audioDecoder; }
#if BRCM_COPY_PROTECTION_ENABLED
 	resources->display = BcmNexus_State.display[0];
	#endif
    resources->videoWindow = NEXUS_VideoWindow_Open(BcmNexus_State.display[0], 0);
	if(!resources->videoWindow) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_videoWindow; }
	
	/* Video window is invisible till SetVideoRegion is called */
    NEXUS_VideoWindow_GetSettings(resources->videoWindow, &windowSettings);
	windowSettings.visible = false;
	windowSettings.autoMaster = get_automaster();
    rc = NEXUS_VideoWindow_SetSettings(resources->videoWindow, &windowSettings);
	if(rc!=NEXUS_SUCCESS) { rc=BERR_TRACE(rc); goto err_videoWindow; }

	resources->playpump[0] = NULL;
	resources->playpump[1] = NULL;
	resources->audioStatusUpdate = BcmNexus_Audio_Status_Update;
	resources->setVideoRegion = BcmNexus_SetVideoRegion;
#if NEXUS_HAS_DMA && NEXUS_HAS_SECURITY
	resources->dmaHandle = NEXUS_Dma_Open(0, NULL);
#endif

	for(i=0; i<resources->config.numPlaypumps; i++) {
		resources->playpump[i] = NEXUS_Playpump_Open(i, NULL);
		if (!resources->playpump[i]) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_playpump; }

		NEXUS_Playpump_GetSettings(resources->playpump[0], &playpumpSettings);
		if (!i)
			playpumpSettings.dataCallback = config->videoDataCallback;
		else
			playpumpSettings.dataCallback = config->audioDataCallback;

		if (!config->transportType) {
			playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
#if BCMEXUS_STREAM_VIDEO_ES || BCMEXUS_STREAM_AUDIO_ES
			playpumpSettings.transportType = NEXUS_TransportType_eEs;
#endif
		} else {
			playpumpSettings.transportType = config->transportType;
		}

#if NEXUS_HAS_DMA && NEXUS_HAS_SECURITY
		if (resources->config.decryptContext) 
			playpumpSettings.securityContext = resources->config.decryptContext;		
#endif

		rc = NEXUS_Playpump_SetSettings(resources->playpump[i], &playpumpSettings);
		if (rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_playpumpSettings;}	

		if (!i) {
			resources->videoPidChannel = NEXUS_Playpump_OpenPidChannel(resources->playpump[i], config->videoPid, NULL);
			if (!resources->videoPidChannel) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_videoPidChannel; }
			if (resources->config.numPlaypumps != 2) {
				/* In case of Netflix, don't open the audio channel */
				resources->audioPidChannel = NEXUS_Playpump_OpenPidChannel(resources->playpump[i], config->audioPid, NULL);
				if (!resources->audioPidChannel) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_audioPidChannel; }
			}
		} else {
			resources->audioPidChannel = NEXUS_Playpump_OpenPidChannel(resources->playpump[i], config->audioPid, NULL);
			if (!resources->audioPidChannel) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_audioPidChannel; }
		}
	}

    NEXUS_Platform_GetConfiguration(&platformConfig);
	NEXUS_VideoWindow_AddInput(resources->videoWindow, NEXUS_VideoDecoder_GetConnector(resources->videoDecoder));

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = config->videoCodec;
    videoProgram.pidChannel = resources->videoPidChannel;
    videoProgram.stcChannel = resources->stcChannel;

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = config->audioCodec;
    audioProgram.pidChannel = resources->audioPidChannel;
    audioProgram.stcChannel = resources->stcChannel;
	BcmNexus_State.audioSetting = audioProgram;
    
    NEXUS_VideoDecoder_GetSettings(resources->videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eMute; 
	if (config->videoSourceCallback.callback)
		videoDecoderSettings.sourceChanged = config->videoSourceCallback;
	if (config->fifoEmptyCallback.callback)
		videoDecoderSettings.fifoEmpty = config->fifoEmptyCallback;

	rc = NEXUS_VideoDecoder_SetSettings(resources->videoDecoder, &videoDecoderSettings);
    if(rc!=NEXUS_SUCCESS) { rc=BERR_TRACE(rc); goto err_videoSettings;}
	BcmNexus_State.streamResource = resources;
    BcmNexus_Unlock();
	BDBG_MSG_TRACE(("BcmNexus_StreamPlayer_Resources_Acquire returning resources=%p", (void*)resources));

	return resources;

err_videoSettings:
err_audioPidChannel:
	if (resources->videoPidChannel) 
		NEXUS_Playpump_ClosePidChannel(resources->playpump[0], resources->videoPidChannel);
	if (resources->audioPidChannel) {
		if(resources->config.numPlaypumps == 2)
			NEXUS_Playpump_ClosePidChannel(resources->playpump[1], resources->audioPidChannel);
		else
			NEXUS_Playpump_ClosePidChannel(resources->playpump[0], resources->audioPidChannel);
	}
err_videoPidChannel:
	NEXUS_Playpump_Close(resources->playpump[0]);
	if (resources->config.numPlaypumps == 2)
		NEXUS_Playpump_Close(resources->playpump[1]);			
err_playpumpSettings:
err_playpump:
    NEXUS_VideoWindow_Close(resources->videoWindow);
err_videoWindow:
err_audioDecoder:
	NEXUS_VideoDecoder_Close(resources->videoDecoder);
err_videoDecoder:
	NEXUS_StcChannel_Close(resources->stcChannel);
err_stcChannel:
	BKNI_Free(resources);
err_alloc:
    BcmNexus_Unlock();
	return NULL;
}

void 
BcmNexus_StreamPlayer_Resources_Release(const BcmNexus_StreamPlayer_Resources *resources_)
{
    if (resources_) {   
		BcmNexus_StreamPlayer_Resources *resources = (BcmNexus_StreamPlayer_Resources *)resources_;
		BcmNexus_Lock();
		if(resources->config.numPlaypumps == 2) {
			NEXUS_Playpump_ClosePidChannel(resources->playpump[0], resources->videoPidChannel);
			NEXUS_Playpump_ClosePidChannel(resources->playpump[1], resources->audioPidChannel);
			NEXUS_Playpump_Close(resources->playpump[0]);
			NEXUS_Playpump_Close(resources->playpump[1]);
		} else {
			NEXUS_Playpump_ClosePidChannel(resources->playpump[0], resources->videoPidChannel);
			NEXUS_Playpump_ClosePidChannel(resources->playpump[0], resources->audioPidChannel);
			NEXUS_Playpump_Close(resources->playpump[0]);
		}		
		NEXUS_VideoWindow_RemoveAllInputs(resources->videoWindow);
		NEXUS_VideoWindow_Close(resources->videoWindow);
		NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(resources->videoDecoder));
		NEXUS_VideoDecoder_Close(resources->videoDecoder);
		NEXUS_StcChannel_Close(resources->stcChannel);
#if NEXUS_HAS_DMA && NEXUS_HAS_SECURITY
		NEXUS_Dma_Close(resources->dmaHandle);
#endif
		BKNI_Free((void *)resources);
		BcmNexus_State.streamResource = NULL;
		BcmNexus_Unlock();
    }
	return;
}

#ifdef NEXUS_HAS_PICTURE_DECODER
const BcmNexus_ImageDecoder_Resources *
BcmNexus_ImageDecoder_Resources_Acquire( const BcmNexus_ImageDecoder_Resources_Config *config )
{
    NEXUS_Error rc;
    BcmNexus_ImageDecoder_Resources *resources;
    NEXUS_PictureDecoderOpenSettings pictureDecoderOpenSettings;
    BDBG_MSG_TRACE(("BcmNexus_ImageDecoder_Resources_Acquire<"));

	BcmNexus_Lock();
	if (BcmNexus_State.pictureDecoder) {
		BDBG_MSG_TRACE(("BcmNexus_ImageDecoder_Resources_Acquire returns NULL"));
		BcmNexus_Unlock();
		return NULL;
	}
    resources = BKNI_Malloc(sizeof(*resources));
    if(!resources) { rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc; }
    resources->graphics = BcmNexus_State.graphics2d;
    resources->graphics_sync = BcmNexus_State.graphics2d_sync;
    if(!resources->graphics || !resources->graphics_sync) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_graphics;}
    resources->config = *config;
    NEXUS_PictureDecoder_GetDefaultOpenSettings(&pictureDecoderOpenSettings);
    pictureDecoderOpenSettings.bufferSize = config->imageMaxSize;
	pictureDecoderOpenSettings.multiScanBufferSize = 0; /* don't use HW decoder for multiscan JPEG's */

	BDBG_MSG_TRACE(("BcmNexus_ImageDecoder_Resources_Acquire: Opening image decoder"));
    resources->pictureDecoder = NEXUS_PictureDecoder_Open(0, &pictureDecoderOpenSettings); 
    if(!resources->pictureDecoder) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_picture_decoder; }
	BcmNexus_State.pictureDecoder = resources->pictureDecoder;

    BDBG_MSG_TRACE(("BcmNexus_ImageDecoder_Resources_Acquire>"));
	BcmNexus_Unlock();
    return resources;

err_picture_decoder:
err_graphics:
    BKNI_Free(resources);
err_alloc:
	BcmNexus_Unlock();
    return NULL;
}

void BcmNexus_ImageDecoder_Resources_Release(const BcmNexus_ImageDecoder_Resources *resources)
{
    BDBG_MSG_TRACE(("BcmNexus_ImageDecoder_Resources_Release>"));
	BDBG_ASSERT(resources);
	BcmNexus_Lock();
    if(BcmNexus_State.pictureDecoder) {
    	BDBG_MSG_TRACE(("BcmNexus_ImageDecoder_Resources_Release: Closing image decoder"));
        NEXUS_PictureDecoder_Close(resources->pictureDecoder);
		BcmNexus_State.pictureDecoder = NULL;
	    BKNI_Free((void *)resources);
    }
    BDBG_MSG_TRACE(("BcmNexus_ImageDecoder_Resources_Release<"));
	BcmNexus_Unlock();
    return;
}
#endif /*NEXUS_HAS_PICTURE_DECODER*/

/* Keymap for the "One-For-All" remote */
static const KeyMapElement IrBasicCodeMap [] = 
{
    { '0',                          0x00, true },
    { '1',                          0x01, true },
    { '2',                          0x02, true },
    { '3',                          0x03, true },
    { '4',                          0x04, true },
    { '5',                          0x05, true },
    { '6',                          0x06, true },
    { '7',                          0x07, true },
    { '8',                          0x08, true },
    { '9',                          0x09, true },
    { BCM_AEKEY_POWER,              0x0A, true },
    { BCM_AEKEY_CHANNEL_UP,         0x0B, true },
    { BCM_AEKEY_CHANNEL_DOWN,       0x0C, true },
    { BCM_AEKEY_VOLUME_UP,          0x0D, true },
    { BCM_AEKEY_VOLUME_DOWN,        0x0E, true },
    { BCM_AEKEY_VOLUME_MUTE,        0x0F, true },  /* mute */
    { BCM_AEKEY_ENTER,              0x10, true },  /* enter */
    { BCM_AEKEY_ENTER,              0x11, true },  /* OK */
    { BCM_AEKEY_TERMINATEANIMATION, 0x12, true },
    { BCM_AEKEY_BACK,               0x13, true },  /* prev */
    { BCM_AEKEY_INPUT,              0x14, true },
    { BCM_AEKEY_MENU,               0x19, true },
    { BCM_AEKEY_FAST_FORWARD,       0x1D, true },
    { BCM_AEKEY_REWIND,             0x1E, true },
    { BCM_AEKEY_PAUSE,              0x1F, true },
    { BCM_AEKEY_GUIDE,              0x30, true },
    { BCM_AEKEY_RECORD,             0x31, true },
    { BCM_AEKEY_INFO,               0x33, true },
    { BCM_AEKEY_UP,                 0x34, true },
    { BCM_AEKEY_DOWN,               0x35, true },
    { BCM_AEKEY_LEFT,               0x36, true },
    { BCM_AEKEY_RIGHT,              0x37, true },
    { BCM_AEKEY_PLAY,               0x38, true },
    { BCM_AEKEY_STOP,               0x39, true },
    { BCM_AEKEY_SKIP_BACKWARD,      0x3C, true },  /* replay */
    { BCM_AEKEY_SUBTITLE,           0x3D, true },  /* cc */
    { BCM_AEKEY_ASPECT,             0x3E, true },  /* format */
    { BCM_AEKEY_SKIP_FORWARD,       0x3F, true },  /* advance */
	{ CODEMAP_LAST_ENTRY,           -1, false }
};

/* Translate Remote specific IR Event to BCM_AEKEY_xxx which map to Adobe FL AEKEY_xxx */
static int
TranslateIrEvent(const KeyMapElement *keymap, int hwCode, bool bKeyRepeated)
{
 	int i;

	for (i=0; keymap[i].ae_code!=CODEMAP_LAST_ENTRY; i++)
	{
		if (keymap[i].hwEventCode==hwCode)
		{
			if (bKeyRepeated && !keymap[i].allowRepeats) {
				return 0;
			} else {
				return keymap[i].ae_code;
			}
		}
	}
    return 0;
}

static bool
ConvertIrToEventCode(NEXUS_IrInputEvent irEvent, uint32_t *eventCode, NEXUS_IrInputMode mode)
{
	if (mode == NEXUS_IrInputMode_eRemoteA) {
		*eventCode = irEvent.code & 0xff;
    } else {
		BDBG_ERR(("No support for selected IR mode"));
		return false;
	}
	return true;
}

/* TTY Resource Handling */
static uint32_t
BcmNexus_TTY_TranslateKey(uint32_t key)
{
#define CNTL_P  0x10
#define ESC     0x1B
#define DEL     0x7F

    uint32_t flKey;
    static int escape_sequence_state = 0;

    switch (escape_sequence_state) {
    case 0:
        break;
    case 1:
        if (key == '[') {
            escape_sequence_state = 2;
            key = 0;
        } else {
            escape_sequence_state = 0;
        }
        break;
    case 2:
        switch (key) {
		    case 'A':
                key = BCM_AEKEY_UP;
                break;
		    case 'B':
                key = BCM_AEKEY_DOWN;
                break;
		    case 'C':
                key = BCM_AEKEY_RIGHT;
                break;
		    case 'D':
                key = BCM_AEKEY_LEFT;
                break;
        }
        escape_sequence_state = 0;
        break;
    }

	switch (key)
	{
        case ESC :
            escape_sequence_state = 1;
            flKey = 0;
            break;
		case DEL :  flKey = BCM_AEKEY_TERMINATEANIMATION; break;
		case '\r':	
		case '\n':	flKey = BCM_AEKEY_ENTER; break;
        case '\t':  flKey = BCM_AEKEY_GUIDE; break;
        case '\b':  flKey = BCM_AEKEY_DELETEKEY; break;
#ifdef _SC_DIRECT_NEXUS_
        case CNTL_P : screen_capture(); break;
#endif /* _SC_DIRECT_NEXUS_ */
		default:	flKey = key; break;
	}
	return flKey;
}

static BcmNexus_TTY *signal_tty = NULL;
static void 
signal_tty_handler(int signum)
{
    static const char message[]="Catched signal\n";

    write(2,message,sizeof(message)-1);
    if(signal_tty) {
        signal_tty->post_exit = true;
        signal_tty->signal = signum;
    }
    return;
}

static void 
signal_alarm_handler(int signum)
{
    sigset_t mask_set;
    sigset_t old_set;

    signum = signum;

    sigfillset(&mask_set);
    sigprocmask(SIG_SETMASK, &mask_set, &old_set);

    BKNI_AcquireMutex(BcmNexus_State.key_up_lock);
    if (BcmNexus_State.FLevent) {
        gettimeofday(&BcmNexus_State.last_input_timestamp, NULL);
        TestHarnessAddKey(BcmNexus_State.FLevent, KEY_DIRECTION_UP);
        BcmNexus_State.inputResource->public.config.key_up_callback.callback(BcmNexus_State.inputResource->public.config.key_up_callback.context, BcmNexus_State.FLevent );
        BcmNexus_State.FLevent = 0;
    }
    BKNI_ReleaseMutex(BcmNexus_State.key_up_lock);

    sigprocmask(SIG_SETMASK, &old_set, NULL);
}

static void *
BcmNexus_TTY_Thread(void *tty_)
{
    BcmNexus_TTY *tty = tty_;
    while(!tty->exit) {
        int rc;
        fd_set rd;
        struct timeval tv;
        unsigned char ch;
        struct itimerval it;

        FD_ZERO(&rd);

        FD_SET(tty->fd, &rd);

        tv.tv_sec = 0;
        tv.tv_usec = 1000 * 100; /* wait 100 ms */

        if(tty->post_exit) {
            tty->post_exit = false;
            signal(tty->signal, signal_tty_handler);
            if(tty->callback.callback) {
                BDBG_WRN(("posting termination key"));
                tty->callback.callback(tty->callback.context, BCM_AEKEY_TERMINATEANIMATION);
            }
            continue;
        }
        rc = select(tty->fd+1, &rd, NULL, NULL, &tv);
        if(rc>0) {
            rc = read(tty->fd, &ch, 1);
            if(rc<0) {
                if(rc==EINTR) { continue; }
                break;
            }
            if(tty->callback.callback) {
                uint32_t flKey = BcmNexus_TTY_TranslateKey(ch);
                if(flKey) {
                    memset(&it, 0, sizeof(it));
                    setitimer(ITIMER_REAL, &it, NULL);

                    BKNI_AcquireMutex(BcmNexus_State.key_up_lock);

                    if (BcmNexus_State.FLevent != flKey) {
                
                        if (BcmNexus_State.FLevent) {
                            /* The timer was canceled before it expired.  */
                            if (TestHarnessGetMode() == TestHarnessModeInputCapture) {
                                TestHarnessAddKey(flKey, KEY_DIRECTION_UP);
                            }
                            BcmNexus_State.inputResource->public.config.key_up_callback.callback(BcmNexus_State.inputResource->public.config.key_up_callback.context, BcmNexus_State.FLevent );
                        }

                        gettimeofday(&BcmNexus_State.last_input_timestamp, NULL);
                        if (TestHarnessGetMode() == TestHarnessModeInputCapture) {
                            TestHarnessAddKey(flKey, KEY_DIRECTION_DOWN);
                        }
                        tty->callback.callback(tty->callback.context, flKey);
                        BcmNexus_State.FLevent = flKey;
                    }

                    BKNI_ReleaseMutex(BcmNexus_State.key_up_lock);
                    memset(&it, 0, sizeof(it));
                    it.it_value.tv_usec = 50000;
                    setitimer(ITIMER_REAL, &it, NULL);
                }
            }
        } 
    }
    return NULL;
}



static NEXUS_Error 
BcmNexus_TTY_Init(BcmNexus_TTY *tty, const char *ttydev)
{
    struct termios termIO;
    NEXUS_Error rc;

    BDBG_ASSERT(tty);
    tty->fd = -1;
    tty->thread = -1;
    if (ttydev == NULL) {
        rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_open;
    }

    tty->exit = false;
    tty->post_exit = false;
    tty->signal = -1;
    tty->fd = open(ttydev, O_RDWR | O_NDELAY);
    if(tty->fd == -1) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_open; }

    if(tcgetattr(tty->fd, &termIO)==-1) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_getattr;}
    tty->savedTermIos = termIO;
    cfmakeraw(&termIO);
    termIO.c_iflag |= BRKINT;
    termIO.c_iflag &= ~IGNBRK;
    termIO.c_lflag |= ISIG;
    termIO.c_oflag |= OPOST | ONLCR;
    termIO.c_cc[VMIN] = 1;
    termIO.c_cc[VTIME] = 0;
    if(tcsetattr(tty->fd, TCSANOW, &termIO)==-1) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_setattr;}

    if(pthread_create(&tty->thread, NULL, BcmNexus_TTY_Thread, tty)==-1) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_thread;}

    signal(SIGHUP, signal_tty_handler);
    signal(SIGINT, signal_tty_handler);
    signal(SIGTERM, signal_tty_handler);
    signal_tty = tty;
    return NEXUS_SUCCESS;

err_thread:
    tcsetattr(tty->fd, TCSANOW, &tty->savedTermIos);
err_setattr:
err_getattr:
    close(tty->fd);
err_open:
    return rc;
}

static void
BcmNexus_TTY_Uninit(BcmNexus_TTY *tty)
{
    NEXUS_Error rc;
    BDBG_ASSERT(tty);
    if (tty->thread != (unsigned)-1) {
		signal_tty = NULL;
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		tty->exit = true;
		pthread_join(tty->thread, NULL);
    }
    if (tty->fd != -1) {
		if(tcsetattr(tty->fd, TCSANOW, &tty->savedTermIos)==-1) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);}
		close(tty->fd);
    }
    return;
}

static void
BcmNexus_IrEventHandler(void *pParam, int iParam)
{
    BcmNexus_Input_Resources_Impl *resources = pParam;

    BDBG_OBJECT_ASSERT(resources, BcmNexus_Input_Resources);
    BSTD_UNUSED(iParam);

    for(;;) {
	    bool    overflow;
    	uint32_t    numEvents = 1;
	    uint32_t    eventCode = 0;
   	    NEXUS_IrInputEvent	irEvent;
	    NEXUS_Error rc ;
        uint32_t FLevent;
        bool bKeyRepeated;
        struct itimerval it;

    	rc = NEXUS_IrInput_GetEvents(resources->irInput, &irEvent, 1, &numEvents, &overflow);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);break;}
        if(numEvents==0) {
            break;
        }

        bKeyRepeated = irEvent.repeat;

        if (!ConvertIrToEventCode(irEvent, &eventCode, resources->mode))	return;
        BDBG_MSG_TRACE(("IR Event: raw code=%x repeat=%d", eventCode, bKeyRepeated));

        /* Translate Nexus Key Event to FL Key Event */
        FLevent = TranslateIrEvent(resources->keymap, eventCode, bKeyRepeated);
        if (FLevent) {
            memset(&it, 0, sizeof(it));
            setitimer(ITIMER_REAL, &it, NULL);

            BKNI_AcquireMutex(BcmNexus_State.key_up_lock);

            if (!bKeyRepeated) {
                if (BcmNexus_State.FLevent) {
                    /* The timer was canceled before it expired.  */
                    if (TestHarnessGetMode() == TestHarnessModeInputCapture) {
                        TestHarnessAddKey(FLevent, KEY_DIRECTION_UP);
                    }
                    BcmNexus_State.inputResource->public.config.key_up_callback.callback(BcmNexus_State.inputResource->public.config.key_up_callback.context, BcmNexus_State.FLevent );
                }

                gettimeofday(&BcmNexus_State.last_input_timestamp, NULL);
                if (TestHarnessGetMode() == TestHarnessModeInputCapture) {
                    TestHarnessAddKey(FLevent, KEY_DIRECTION_DOWN);
                }
                resources->public.config.key_down_callback.callback(resources->public.config.key_down_callback.context, FLevent);
                BcmNexus_State.FLevent = FLevent;
            }
            BKNI_ReleaseMutex(BcmNexus_State.key_up_lock);

            memset(&it, 0, sizeof(it));
            it.it_value.tv_usec = 105000;
            setitimer(ITIMER_REAL, &it, NULL);
        }
	}
    return;
}

/* IR Resource */
const BcmNexus_Input_Resources *
BcmNexus_Input_Resources_Acquire(const BcmNexus_Input_Resources_Config *config)
{
	NEXUS_Error				rc;
	NEXUS_IrInputSettings	irInputSettings;
    BcmNexus_Input_Resources_Impl *resources;

    BDBG_ASSERT(config);
    BDBG_MSG_TRACE(("BcmNexus_Input_Resources_Acquire"));
	BcmNexus_Lock();
    resources = BKNI_Malloc(sizeof(*resources));
    if (!resources) { rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }
    BDBG_OBJECT_INIT(resources, BcmNexus_Input_Resources);

	/* FL Callback */
    resources->public.config = *config;
    resources->tty.callback = config->key_down_callback;

	if (config->enable_tty) {
		rc = BcmNexus_TTY_Init(&resources->tty, BCM_TTY_DEV);
		if (rc!=BERR_SUCCESS) {
			/* Failing the open the TTY driver should not be a fatal error. */
			BDBG_MSG_TRACE(("Unable to open device %s", BCM_TTY_DEV));
			rc = BcmNexus_TTY_Init(&resources->tty, BCM_CONSOLE_DEV);
			if (rc!=BERR_SUCCESS) {
				BDBG_MSG_TRACE(("Unable to open device %s", BCM_CONSOLE_DEV));
			}
		}
	}

    memset(&BcmNexus_State.last_input_timestamp,0,sizeof(BcmNexus_State.last_input_timestamp));

	NEXUS_IrInput_GetDefaultSettings(&irInputSettings);
	irInputSettings.dataReady.callback = BcmNexus_IrEventHandler;
	irInputSettings.dataReady.context = resources;
	irInputSettings.repeatFilterTime = REPEAT_MILLI_SECS;
	irInputSettings.mode = NEXUS_IrInputMode_eRemoteA;

	resources->keymap = IrBasicCodeMap;
	resources->irInput = NEXUS_IrInput_Open(0, &irInputSettings);
    if (!resources->irInput) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_fail_ir; }
	resources->mode = irInputSettings.mode;

    BcmNexus_State.inputResource = resources;

    signal(SIGALRM, signal_alarm_handler);

	BcmNexus_Unlock();
	return &resources->public;

err_fail_ir:
    BcmNexus_TTY_Uninit(&resources->tty);
    signal(SIGALRM, SIG_IGN);
	BKNI_Free(resources);
err_alloc:
	BcmNexus_Unlock();
	return NULL;
}

void 
BcmNexus_Input_Resources_Release(const BcmNexus_Input_Resources *resources_)
{
    BcmNexus_Input_Resources_Impl *resources = (BcmNexus_Input_Resources_Impl *)resources_;

    BDBG_MSG_TRACE(("BcmNexus_Input_Resources_Release"));
	/* Note: Stop the display Flip callback before stopping the TTY driver, else the system hangs */
	if (BcmNexus_State.doublebuffer) {
		NEXUS_StopCallbacks(BcmNexus_State.display[0]);
	}
    BDBG_OBJECT_ASSERT(resources, BcmNexus_Input_Resources);
	BcmNexus_Lock();
    if (resources->public.config.enable_tty) BcmNexus_TTY_Uninit(&resources->tty);
	if (resources->irInput) NEXUS_IrInput_Close(resources->irInput);
    BDBG_OBJECT_DESTROY(resources, BcmNexus_Input_Resources);
    signal(SIGALRM, SIG_IGN);
    signal_tty = NULL;
	BcmNexus_Unlock();
    BKNI_Free(resources);
    return;
}

struct timeval BcmNexus_Key_Last_Key_Timestamp(void)
{
    return BcmNexus_State.last_input_timestamp;
}

#ifdef _DFB_TEST_HARNESS_
static 
unsigned int 
dfb_checksum(void *pFrameBuffer, 
            int x_start,int y_start, 
            int height,int width,
            int pitch)
{
    unsigned int * pFrameBufferLine;
    int ind_x,ind_y;
    unsigned int chk = 0;

    for(ind_y=0; ind_y < height;ind_y++)
    {
        pFrameBufferLine = (unsigned int *)pFrameBuffer + (x_start) + 
            ((y_start + ind_y) * (pitch >> 2));

        for(ind_x = 0; ind_x < width; ind_x++) 
        {
            chk += pFrameBufferLine[ind_x];
        }
    }

    return chk;
}

void	
BcmNexus_DFBWindowUpdate(void * pFrameBuffer, 
                           int x,int y, 
                           int h,int w,
                           int pitch,
                           STG_KEY_UPDATE pfnKeyUpdateFn,
                           void * keyUpdateCnxt)
{
    TestHarnessMode tMode=TestHarnessModeNone;
    static unsigned int previous_checksum = 0xDEADBEEF;
    static unsigned int mismatch=0,counter=1;
    static NEXUS_Rect previous_rect = {0,0,0,0};
    unsigned int checksum;
    tMode = TestHarnessGetMode();

    if(tMode == TestHarnessModeNone)
        return;

    checksum = dfb_checksum(pFrameBuffer,x,y,h,w,pitch);
    if((checksum == previous_checksum) && (previous_rect.width == w) && 
        (previous_rect.height == h) && (previous_rect.x == x) && 
        (previous_rect.y == y))
    {
        return;
    }else{
        previous_checksum    = checksum;
        previous_rect.width  = w;
        previous_rect.height = h;
        previous_rect.x      = x;
        previous_rect.y      = y;
    }


    if((tMode == TestHarnessModeInputCapture) ||
        (tMode == TestHarnessModeOutputCapture))
    {
        TestHarnessAddFrameHash(checksum);
    }

    if(tMode == TestHarnessModePlayback)
    {
        if (checksum != TestHarnessGetFrameHash(counter)) 
        {
            TestHarnessFail(counter);
            mismatch++;
        }
    }

    if ((tMode == TestHarnessModeOutputCapture) ||
        (tMode == TestHarnessModePlayback)) 
    {
        unsigned int key = 0xDEADBEEF;
        unsigned int direction = 0xDEADBEEF;
        while (TestHarnessGetKey(counter, &key, &direction)) 
        {
            pfnKeyUpdateFn(keyUpdateCnxt,key,direction);
        }

        if ((key == 0x00400100) && (!mismatch)) 
            TestHarnessPass();
    }

    counter++;
    return;
}


void 
BcmNexus_DFBKeyEvent(U32 keyCode, U32 direction)
{
    TestHarnessAddKey(keyCode,direction);
}
#endif
