/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

 ******************************************************************************/

#include "nexusMgr.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "blst_slist.h"
#include "bchp_gio.h"
#include "nexus_platform.h"

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


static struct {
    bool                                initialized;
    bool                                autoInitialized;

    BKNI_MutexHandle                    resource_lock;
    NEXUS_DisplayHandle                 display[2];

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

#if NEXUS_NUM_AUDIO_DACS
    /* Connect DAC to mixer */
    if(platformConfig.outputs.audioDacs[0])  {
        rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), NEXUS_AudioMixer_GetConnector(BcmNexus_State.mixer));
        if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_mixer_output;}
    }
#endif

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
#if NEXUS_NUM_AUDIO_DACS
err_mixer_output:
#endif
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

#if NEXUS_NUM_AUDIO_DACS
    if(platformConfig.outputs.audioDacs[0])  {
        NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), NEXUS_AudioMixer_GetConnector(BcmNexus_State.mixer));
    }
#endif
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
    NEXUS_PlatformConfiguration platformConfig;

    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdcpStatus);


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

	NEXUS_DisplaySettings		displayCfg;
	NEXUS_PlatformConfiguration	platformConfig;
	NEXUS_DisplayHandle		    display0;

    BDBG_MSG_TRACE(("BcmNexus_Platform_Init (Nexus)"));

    if (BcmNexus_State.initialized) {
        return false;
    }
	
    BcmNexus_State.initialized = true;

	/* Bring up all modules for a platform in a default configuration for this platform */
	NEXUS_Platform_GetDefaultSettings(&platformSettings); 
	platformSettings.openFrontend = false;



	rc = NEXUS_Platform_Init(&platformSettings); 
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_nexus;}
    rc = BKNI_CreateMutex(&BcmNexus_State.resource_lock);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_resource_lock;}

    rc = BKNI_CreateMutex(&BcmNexus_State.key_up_lock);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_key_up_lock;}

	NEXUS_Platform_GetConfiguration(&platformConfig);       	
	NEXUS_Display_GetDefaultSettings(&displayCfg);



		displayCfg.format = config_get_res();
		displayCfg.displayType = NEXUS_DisplayType_eAuto;
		display0 = NEXUS_Display_Open(0, &displayCfg);
		BDBG_ASSERT(display0);

#if NEXUS_NUM_COMPONENT_OUTPUTS

		rc = NEXUS_Display_AddOutput(display0, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
		BDBG_ASSERT(!rc);
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

    BcmNexus_State.display[0] = display0;
    BcmNexus_State.display[1] = NULL;

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



err_key_up_lock:
err_resource_lock:
err_nexus:
    return false;
}

/*_SC_DIRECT_NEXUS_*/
void 
BcmNexus_Platform_Uninit(void)
{

	#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Platform_GetConfiguration(&platformConfig);
#endif

    BDBG_ASSERT(BcmNexus_State.initialized);
    BDBG_MSG_TRACE(("BcmNexus_Platform_Uninit"));
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = BcmNexus_State.hdmiHotplugCallbackOriginal.callback;
    NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);

#if BRCM_COPY_PROTECTION_ENABLED 	
	NEXUS_HdmiOutput_DisableHdcpAuthentication(platformConfig.outputs.hdmi[0]);
      gAppEnabledHDCP=false;      
#endif
#endif

	NEXUS_Display_RemoveAllOutputs(BcmNexus_State.display[0]);	
	NEXUS_Display_Close(BcmNexus_State.display[0]); 


    BKNI_DestroyMutex(BcmNexus_State.key_up_lock);
    BKNI_DestroyMutex(BcmNexus_State.resource_lock);
    BcmNexus_Platform_Uninit_Audio();
	NEXUS_Platform_Uninit();
    BcmNexus_State.initialized = false;


    return;
}








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
    BSTD_UNUSED(resources);
	BcmNexus_State.audioStarted[0] = started;
}

NEXUS_Error
BcmNexus_SetVideoRegion(const BcmNexus_StreamPlayer_Resources *resources, NEXUS_Rect *planeRect, NEXUS_Rect *rect, NEXUS_Rect *clipRect)
{
    NEXUS_VideoWindowSettings windowSettings;
	NEXUS_Error rc;
    BSTD_UNUSED(clipRect);
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

	BDBG_LOG(("VGD:BcmNexus_SetVideoRegion: Final windowSettings %d,%d x %d,%d", windowSettings.position.x, windowSettings.position.y, windowSettings.position.width, windowSettings.position.height));
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

struct timeval BcmNexus_Key_Last_Key_Timestamp(void)
{
    return BcmNexus_State.last_input_timestamp;
}
