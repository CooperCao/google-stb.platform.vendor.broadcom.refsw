/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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
 *****************************************************************************/
/* Nexus example app: playback and decode */

#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif

#include "nexus_wmdrmpd.h"
#include "nexus_wmdrmpd_types.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_event_group.h"

#include "prdy_http.h"
#include "basf_parser.h"
#include "asfpars.h"

BDBG_MODULE(playback_encrypted_asf);

/* signifies drm encrypted or not
*/
typedef enum bdrm_hdr_state {
    bdrm_hdr_read_ok    = (0),             /* drm encrypted asf */
    bdrm_hdr_incomplete = (1),             /* not ready yet */
    bdrm_hdr_none       = (2)              /* no drm headers */
} bdrm_hdr_state;


/* content encryption object
*/
typedef struct basf_cencr_object {
    bdrm_hdr_state state;               /* headers state */
    basf_dword sec_dlen;                /* security data length */
    basf_byte  *sec_data;               /* security data */
    basf_dword prot_tlen;               /* protocol length */
    basf_byte  *prot_type;              /* protocol type */
    basf_dword keyid_len;               /* key id length */
    basf_byte  *keyid;                  /* key id data */
    basf_dword licurl_len;              /* license url length */
    basf_byte  *licurl;                 /* license url data */
} basf_cencr_object;

/* extended content encryption objectd
*/
typedef struct basf_xcencr_object {
    bdrm_hdr_state state;               /* headers state */
    basf_dword size;                    /* extended encr size */
    basf_byte  *data;                   /* extended encr data */
} basf_xcencr_object;

/* digital signature object
*/
typedef struct basf_digsign_object {
    bdrm_hdr_state state;               /* headers state */
    basf_dword type;                    /* digital signature type */
    basf_dword len;                     /* digital signature blob len */
    basf_byte* data;                    /* digital signature blob */
} basf_digsign_object;


/* PLAYREADY: Protection system identifier object
*/
typedef struct basf_psi_object {
    bdrm_hdr_state state;               /* headers state */
    basf_guid systemId;                 /* Defines the GUID of the Content Protection system. Microsoft PlayReady systems must use ASF_Content_Protection_System_Microsoft_PlayReady */
    basf_dword sysversion;              /* Version specific to each Content Protection system. Microsoft PlayReady must use System Version = 1*/
    basf_dword size;                    /* Specifies the size in bytes of the data field */
    basf_byte* data;                    /* The content protection data for the PlayReady Object */
} basf_psi_object;

/* holds asf header handlers. We use this to pass the context through
** the callback.
*/                                    
typedef struct basf_drm_object_handler {
    basf_parser_object_handler handler;        /* standard asf handler */
} basf_drm_object_handler;

/* holds initialized/uninitialized state for object handlers
*/
typedef enum basf_drm_handlers_state {
    basf_drm_handlers_init      = 0,    /* handlers were initialized */
    basf_drm_handlers_uninit    = 1     /* handlers were not initialized */
} basf_drm_handlers_state;


/* holds drm sub-header handlers
*/
typedef struct basf_drm_handlers {
    basf_drm_handlers_state state;
    basf_drm_object_handler digsign_handler; /* digital signature handler */         
    basf_drm_object_handler xencr_handler;   /* extended content encryption handler */      
    basf_drm_object_handler encr_handler;    /* content encryption handler */ 
    basf_drm_object_handler psi_handler;     /*PLAYREADY : Protection system Identifier */
} basf_drm_handlers;

/* DRM header GUIDs
*/
static const basf_guid
basf_content_encr_guid = 
{{0x22, 0x11, 0xB3, 0xFB, 0xBD, 0x23, 0x11, 0xD2, 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E}},
basf_content_encr_ext_guid = 
{{0x29, 0x8A, 0xE6, 0x14, 0x26, 0x22, 0x4C, 0x17, 0xB9, 0x35, 0xDA, 0xE0, 0x7E, 0xE9, 0x28, 0x9C}},
basf_content_digi_sign_guid = 
{{0x22, 0x11, 0xB3, 0xFC, 0xBD, 0x23, 0x11, 0xD2, 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E}},
/*PLAYREADY*/
basf_protection_system_identifier_guid =
{{0x9A, 0x04, 0xF0, 0x79, 0x98, 0x40, 0x42, 0x86, 0xAB, 0x92, 0xE6, 0x5B, 0xE0, 0x88, 0x5F, 0x95}},
basf_content_protection_system_microsoft_playready_guid =
{{0xF4, 0x63, 0x70, 0x10, 0x03, 0xC3, 0x42, 0xCD, 0xB9, 0x32, 0xB4, 0x8A, 0xDF, 0x3A, 0x6A, 0x54}},
basf_streamtype_playready_encrypted_command_media_guid =
{{0x86, 0x83, 0x97, 0x3A, 0x66, 0x39, 0x46, 0x3A, 0xAB, 0xD7, 0x64, 0xF1, 0xCE, 0x3E, 0xEA, 0xE0}};

basf_psi_object global_psi;
basf_digsign_object global_digsign;
basf_xcencr_object global_xcencr;
basf_cencr_object global_cencr;


#define CH_DEFAULT_NON_QUIET    (1)     /* default non-quiet on */
#define CH_DEFAULT_APP_SEC      (150)   /* default app security test-level */

/*
 * When building Nexus, you will need the following env variables:
 *
 * export MSDRM_PD_SUPPORT=y
 * export MEDIA_ASF_SUPPOR=y
 */

/* For pre-delivered license */
/*#define PRE_LICENSE_DELIVERY 1*/

/* For user specific handling of external license acquisition */
#define LICENSE_EXT_USER 1

/* Force license response failure */
/*#define WMDRMPD_FORCE_NO_RESPONSE 1*/

/* Test WMDRMPD get/set settings */
#define WMDRMPD_CONFIG_SET 1

/* Preload license before playback. 
Make sure devcert.dat and sample.hd were delected before the test */
/*#define WMDRMPD_PRELOAD_LICENSE 1*/

#define PRINT_CLIENT_INFO(data,num) \
{ \
    int i; \
    for(i=0;i<num;i++) \
        printf("%c", data[i]); \
    printf("\n"); \
}

#define SAVE_DATA2FILE(fn,dat,sz) \
{ \
    FILE* fp; \
    fp = fopen(fn,"w+"); \
    if(fp) \
    { \
        fwrite(dat,1,sz,fp); \
        fclose(fp); \
    } \
}

#define PLAYREADY_DRM 1

/*
 * When building Nexus, you will need the following env variables:
 *
 * export MSDRM_PD_SUPPORT=y
 * export MEDIA_ASF_SUPPOR=y
 */

#define LICENSE_MAX_EVENTS 3

typedef struct license_event
{
    BKNI_EventGroupHandle license_status_group;
    BKNI_EventHandle license_acquired_evt;
    BKNI_EventHandle license_failtranser_evt;
    BKNI_EventHandle license_faillocal_evt;
}license_event;

typedef struct license_callback_params
{
    NEXUS_WmDrmPdHandle handle;
    BKNI_EventHandle event;
    bool threadExit;
    license_event evt;
}license_callback_params;

typedef struct policy_callback_params
{
    NEXUS_WmDrmPdHandle handle;
    BKNI_EventHandle event;
    bool threadExit;
}policy_callback_params;


void license_callback(void *context, int param)
{
    BSTD_UNUSED(param);
     printf("license_callback - event %p\n", context);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void license_ext_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void policy_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    printf("policy_callback - event %p\n", context);
}


void policy_callback_thread(void* params)
{
    BSTD_UNUSED(params);
}


void license_callback_thread(void* params)
{
    BSTD_UNUSED(params);
}


#if NEXUS_HAS_PLAYBACK
#define NUM_STREAM 1

typedef struct main_app_params_
{
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_FilePlayHandle file[NUM_STREAM];
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    BKNI_EventHandle licCbEvent;
    BKNI_EventHandle licExtCbEvent;
    BKNI_EventHandle licPreCbEvent;
    BKNI_EventHandle licPreExtCbEvent;
    BKNI_EventHandle policyCbEvent;
    NEXUS_ThreadHandle licCbThreadHandle;
    NEXUS_ThreadHandle licExtCbThreadHandle;
    NEXUS_ThreadHandle policyCbThreadHandle;
    license_callback_params* licCbParams;
    license_callback_params* licExtCbParams;
    NEXUS_ThreadHandle licPreCbThreadHandle;
    NEXUS_ThreadHandle licPreExtCbThreadHandle;
    license_callback_params* licPreCbParams;
    license_callback_params* licPreExtCbParams;
    policy_callback_params*  policyCbParams;
    NEXUS_WmDrmPdSettings drmSettings, drmSettings2;
    NEXUS_WmDrmPdHandle drm;
    NEXUS_WmDrmPdHandle drm_lic;
    NEXUS_WmDrmPdHandle drm_wrap;
    NEXUS_DmaHandle hDma;
    NEXUS_VideoCodec vidCodec;
    uint16_t vid_pid_num;
    basf_parser_t asf;
    basf_props_t asfp;
}main_app_params;
#endif

void get_drm_header(const char* fname, main_app_params* p_params);
void b_asf_init_handlers (basf_parser_t asf, basf_props_t asfp, basf_drm_handlers asf_handler);

int main(void)  {
#if NEXUS_HAS_PLAYBACK
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if PLAYREADY_DRM
    const char *fname[NUM_STREAM] = {"videos/Bear_Video_OPLs0.pyv"};
#else
    const char *fname[NUM_STREAM] = {"videos/MP10_Stiker_adv-drm.wmv"};
#endif
    main_app_params* p_params = NULL;
    
    uint8_t non_quiet=CH_DEFAULT_NON_QUIET;    
    uint32_t app_security=CH_DEFAULT_APP_SEC;
    uint32_t post_ret;
    uint32_t startOffset, length;
    NEXUS_SecurityKeySlotSettings genericSettings;
    NEXUS_KeySlotHandle genericKeyHandle;
    NEXUS_WmDrmPdLicenseChallenge challenge;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_Error rc = 0;
    int i;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);


    p_params = BKNI_Malloc(sizeof(main_app_params));
    if(p_params == NULL)
    {
        printf("Error: p_params alloc failed\n");
        return 0;
    }

    memset(p_params, 0, sizeof(main_app_params));

    #if PLAYREADY_DRM
    p_params->vid_pid_num = 2;
    p_params->vidCodec = NEXUS_VideoCodec_eVc1SimpleMain;
    #else
    p_params->vid_pid_num = 1;
    p_params->vidCodec = NEXUS_VideoCodec_eVc1;
    #endif



    p_params->playpump = NEXUS_Playpump_Open(0, NULL);
    assert(p_params->playpump);
    p_params->playback = NEXUS_Playback_Create();
    assert(p_params->playback);
    p_params->hDma = NEXUS_Dma_Open(0, NULL);
    assert(p_params->hDma);

    for(i = 0; i < NUM_STREAM; i++)
    {
        p_params->file[i] = NEXUS_FilePlay_OpenPosix(fname[i], fname[i]);
        if (!p_params->file[i]) {
            fprintf(stderr, "can't open file:%s\n", fname[i]);
        return -1;
    }
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &p_params->stcSettings);
    p_params->stcSettings.timebase = NEXUS_Timebase_e0;
    p_params->stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    p_params->stcChannel = NEXUS_StcChannel_Open(0, &p_params->stcSettings);

    /******************** Initialse main WMDRMPD module. *******************************/
    NEXUS_WmDrmPd_GetDefaultSettings(&p_params->drmSettings);

#ifdef WMDRMPD_CONFIG_SET
#if 0
    /* Assign the playpump device to recieve the ASF stream */
    p_params->drmSettings.playpump_handle = p_params->playpump;
#endif
    /* Assign the dma handle */
    p_params->drmSettings.dma = p_params->hDma;
    p_params->drmSettings.playpump = p_params->playpump;
    p_params->drmSettings.transportType = NEXUS_TransportType_eAsf;

    /* License status callback. If callback is NULL, internal callback will be used. */

    /* If user wants to use the default external license acquisition (http), null this callback.*/

    p_params->drmSettings.policyCallback.callback = policy_callback;
    p_params->drmSettings.policyCallback.context = NULL;
    p_params->drmSettings.policyCallback.param = 0;


#else
    BKNI_Memset(&p_params->drmSettings, 0, sizeof(NEXUS_WmDrmPdSettings));
#endif /* WMDRMPD_CONFIG_SET */

    /* Open main WMDRMPD module. */
    p_params->drm = NEXUS_WmDrmPd_Create(&p_params->drmSettings);
    if( p_params->drm == NULL )
    {
        printf(("error: drm handle open failed"));;
        return -1;
    }

    /* Parameter holder for license status callback */  
/* Parameter holder for policy callback */  
    /* Open DRM after the license thread is up. */
    /*----------------------------*/
    /*----------------------------*/
    NEXUS_Security_GetDefaultKeySlotSettings(&genericSettings);
    genericSettings.keySlotEngine = NEXUS_SecurityEngine_eGeneric;

    genericKeyHandle = NEXUS_Security_AllocateKeySlot(&genericSettings);
    if(genericKeyHandle == NULL)
    {
        printf(("error: NEXUS_Security_AllocateKeySlot failed"));
        return -1;
    }

    if ( NEXUS_WmDrmPd_ConfigureKeySlot(p_params->drm, genericKeyHandle) )
    {
        printf(("error: NEXUS_WmDrmPd_ConfigureKeySlot failed"));
        return -1;
    }

    NEXUS_Playback_GetSettings(p_params->playback, &p_params->playbackSettings);
    p_params->playbackSettings.playpump = p_params->playpump;
    p_params->playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eAsf;
    p_params->playbackSettings.stcChannel = p_params->stcChannel;
    p_params->playbackSettings.playpumpSettings.securityContext = genericKeyHandle; 

    NEXUS_Playback_SetSettings(p_params->playback, &p_params->playbackSettings);

    /* Bring up video display and outputs */
    p_params->display = NEXUS_Display_Open(0, NULL);
    p_params->window = NEXUS_VideoWindow_Open(p_params->display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(p_params->display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif 
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(p_params->display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(p_params->display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(p_params->display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(p_params->display, &displaySettings);
        }
    }
#endif

    /* bring up decoder and connect to display */
    p_params->videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(p_params->window, NEXUS_VideoDecoder_GetConnector(p_params->videoDecoder));

    /* Open the video pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&p_params->playbackPidSettings);
    p_params->playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;

    /* For AVI/ASF streams, timestamps are usually in decode order. To let the video decoder 
       (instead of the host) handle the timestamp reordering,
       
           set NEXUS_PlaybackPidChannelSettings.pidSettings.allowTimestampReordering = false 
           and NEXUS_VideoDecoderStartSettings.timestampMode = NEXUS_VideoDecoderTimestampMode_eDecode (see below).

       For other streams, the timestamp mode may not be known in advance. For such cases, use bmedia_probe 
       to get bmedia_probe_track.info.video.bmedia_timestamp_order, and propagate it to 
       NEXUS_VideoDecoderStartSettings.timestampMode.
    */
    p_params->playbackPidSettings.pidSettings.allowTimestampReordering = false;
    p_params->playbackPidSettings.pidTypeSettings.video.codec = p_params->vidCodec; /* must be told codec for correct handling */
    p_params->playbackPidSettings.pidTypeSettings.video.index = true;
    p_params->playbackPidSettings.pidTypeSettings.video.decoder = p_params->videoDecoder;
    p_params->videoPidChannel = NEXUS_Playback_OpenPidChannel(p_params->playback, p_params->vid_pid_num, &p_params->playbackPidSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&p_params->videoProgram);
    p_params->videoProgram.codec = p_params->vidCodec;
    p_params->videoProgram.timestampMode = NEXUS_VideoDecoderTimestampMode_eDecode;
    p_params->videoProgram.pidChannel = p_params->videoPidChannel;
    p_params->videoProgram.stcChannel = p_params->stcChannel;

    get_drm_header(fname[0], p_params);

    if ( NEXUS_WmDrmPd_LoadLicense(p_params->drm) != NEXUS_SUCCESS) 
    {
#if 1           
        NEXUS_WmDrmPd_GetLicenseChallenge(p_params->drm, &challenge);
        /*PRINT_CLIENT_INFO(pData.data,pData.size);*/
                
#if PLAYREADY_DRM
        post_ret = bhttpclient_license_post_soap((char *)challenge.pUrl, (char *)challenge.pData, non_quiet, app_security, (unsigned char**)&(challenge.pResponseBuffer), &startOffset, &length);
#else
        post_ret = bhttpclient_license_post_default((char *)challenge.pUrl, (char *)challenge.pData, non_quiet, app_security, (unsigned char**)&(challenge.pResponseBuffer), &startOffset, &length);
#endif
        if (post_ret == 0) 
        {
            challenge.pResponseBuffer[length] = '\0';
            #if 0
            printf("Response <<%s>>\n", challenge.pResponseBuffer);
            #endif
            NEXUS_WmDrmPd_LicenseChallengeComplete( p_params->drm, length, startOffset);
        }
        else
        {
            printf("Error doing post \n");
        }

#endif                
    }   
  
    /* Start playback */
    NEXUS_VideoDecoder_Start(p_params->videoDecoder, &p_params->videoProgram);
    NEXUS_Playback_Start(p_params->playback, p_params->file[0], NULL);


#if 1
    for (i = 0; i < 20; i++) {
        NEXUS_VideoDecoderStatus status;
        NEXUS_VideoDecoder_GetStatus(p_params->videoDecoder, &status);
        BDBG_MSG(("Main - numDecoded = '%u'   numDecodeErrors = '%u'   ptsErrorCount = '%u'", status.numDecoded, status.numDecodeErrors, status.ptsErrorCount));
        if (status.numDecodeErrors) {
            BDBG_ERR(("failing with numDecodeErrors = '%u'", status.numDecodeErrors));
            rc = -1;
            break;
        }
        BKNI_Sleep(1000);
    }
#else
    printf("Press ENTER to play the next stream\n");
    getchar();
#endif

    NEXUS_VideoDecoder_Stop(p_params->videoDecoder);
    NEXUS_Playback_Stop(p_params->playback);

        
    NEXUS_Playback_ClosePidChannel(p_params->playback, p_params->videoPidChannel);

    /* Bring down system */
    NEXUS_FilePlay_Close(p_params->file[0]);
    NEXUS_Playback_Destroy(p_params->playback);


    /* Close down drm stuff */
    #if PLAYREADY_DRM
    NEXUS_Memory_Free((void *)(global_psi.data));
    #else
    NEXUS_Memory_Free((void *)(global_digsign.data));
    NEXUS_Memory_Free((void *)(global_cencr.keyid));
    NEXUS_Memory_Free((void *)(global_cencr.licurl));
    NEXUS_Memory_Free((void *)(global_cencr.prot_type));
    NEXUS_Memory_Free((void *)(global_cencr.sec_data));
    NEXUS_Memory_Free((void *)(global_xcencr.data));
    #endif
    NEXUS_Security_FreeKeySlot(genericKeyHandle);
    NEXUS_WmDrmPd_Destroy(p_params->drm);

    NEXUS_Playpump_Close(p_params->playpump);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(p_params->videoDecoder));
    NEXUS_VideoDecoder_Close(p_params->videoDecoder);
    NEXUS_VideoWindow_Close(p_params->window);
    NEXUS_Display_Close(p_params->display);
    NEXUS_StcChannel_Close(p_params->stcChannel);
    BKNI_Free(p_params);
    NEXUS_Platform_Uninit();
#endif
    return rc;
}

void get_drm_header(const char* fname, main_app_params* p_params)
{
    basf_drm_handlers asf_handler;
    NEXUS_WmDrmPdPsiObjectInfo psiHeader;
    NEXUS_WmDrmPdDigsignObjectInfo digsignHeader;

    BSTD_UNUSED(digsignHeader);
    
     global_psi.state = bdrm_hdr_none;
     global_cencr.state = bdrm_hdr_none;
     global_xcencr.state = bdrm_hdr_none;
     global_digsign.state = bdrm_hdr_none;
 
     p_params->asfp = basf_open_asf (fname,  basf_hdr_ctx_custom, &(p_params->asf));
     b_asf_init_handlers (p_params->asf, p_params->asfp, asf_handler);
 
     /*--------------------------------------*/
     if (global_psi.state == bdrm_hdr_read_ok) 
     {
         psiHeader.sysVersion = global_psi.sysversion;
         BKNI_Memcpy(&(psiHeader.systemId), &(global_psi.systemId), sizeof(NEXUS_WmDrmPdGuid));

         NEXUS_WmDrmPd_SetPsiObject( p_params->drm, &psiHeader, global_psi.data, global_psi.size );

     }
     /*----------------------------*/
     if (global_cencr.state == bdrm_hdr_read_ok)
     {
         printf("Calling NEXUS_WmDrmPd_SetCencrObject\n");
         NEXUS_WmDrmPd_SetCencrObject( p_params->drm, 
                                       global_cencr.sec_data, 
                                       global_cencr.sec_dlen,
                                       global_cencr.prot_type,
                                       global_cencr.prot_tlen,
                                       global_cencr.keyid,
                                       global_cencr.keyid_len,
                                       global_cencr.licurl,
                                       global_cencr.licurl_len
                                      );
     }
     /*----------------------------*/
     if (global_xcencr.state == bdrm_hdr_read_ok) 
     {
         printf("Calling NEXUS_WmDrmPd_SetXcencrObject\n");
         NEXUS_WmDrmPd_SetXcencrObject( p_params->drm, global_xcencr.data, global_xcencr.size);
     }
 
     /*----------------------------*/
     if (global_digsign.state == bdrm_hdr_read_ok) 
     {
         printf("Calling NEXUS_WmDrmPd_SetDigsignObject\n");
         digsignHeader.type = global_digsign.type;
         NEXUS_WmDrmPd_SetDigsignObject(p_params->drm, &digsignHeader, global_digsign.data, global_digsign.len);
     }

     #if PLAYREADY_DRM
     basf_close_asf(p_params->asfp);
     #endif
}

unsigned
basf_read_arr(batom_cursor *c, basf_byte *str, unsigned len)
{
    unsigned i;    

#ifndef __HAS_NO_BIOVEC_INTERNALS__
    for(i=0; i < len; i++) {
        BATOM_NEXT(str[i],c);
    }

#else /* __HAS_NO_BIOVEC_INTERNALS__ */

    i = batom_cursor_copy(c, str, len);

#endif /* __HAS_NO_BIOVEC_INTERNALS__ */

    return (i);
}

/* read a zero-terminated string from the current position
*/
void 
basf_read_str(batom_cursor *c, basf_byte *str, unsigned len)
{
    unsigned i = basf_read_arr(c, str, len-1);
    str[i] = '\0';

    return;
}



/* Extract encryption object from asf header
*/
static unsigned int
b_asf_parse_content_encr_object (struct basf_parser_object_handler *handler, batom_cursor* cursor)
{
    unsigned int ret = 0;
    basf_cencr_object *encr;    
    basf_drm_object_handler* drmh = ((struct basf_drm_object_handler *)handler);
    BSTD_UNUSED(drmh);
printf("\nCalling b_asf_parse_content_encr_object \n");

    encr = &global_cencr;
    encr->sec_dlen = basf_read_dword(cursor);           /* secret */
    NEXUS_Memory_Allocate(encr->sec_dlen, NULL, (void **)(&(encr->sec_data)));
    basf_read_arr(cursor, encr->sec_data, encr->sec_dlen);

    encr->prot_tlen = basf_read_dword(cursor)+1;        /* protection type */
    NEXUS_Memory_Allocate(encr->prot_tlen, NULL, (void **)(&(encr->prot_type)));
    basf_read_str(cursor, encr->prot_type, encr->prot_tlen);

    encr->keyid_len = basf_read_dword(cursor)+1;        /* key id */
    NEXUS_Memory_Allocate(encr->keyid_len, NULL, (void **)(&(encr->keyid)));
    basf_read_str(cursor, encr->keyid, encr->keyid_len);

    encr->licurl_len = basf_read_dword(cursor)+1;       /* license url */
    NEXUS_Memory_Allocate(encr->licurl_len, NULL, (void **)(&(encr->licurl)));
    basf_read_str(cursor, encr->licurl, encr->licurl_len);

    if (encr->keyid_len != 0 && encr->licurl_len != 0) {
        encr->state = bdrm_hdr_read_ok;
    }

    return(ret);
}

/* Extract extended encryption object from asf header
*/
static unsigned int
b_asf_parse_content_encr_ext_object (struct basf_parser_object_handler *handler, batom_cursor* cursor)
{   
    unsigned int ret = 0;
    struct basf_xcencr_object *xencr;
    basf_drm_object_handler* drmh = ((struct basf_drm_object_handler *)handler);
    BSTD_UNUSED(drmh);
printf("\nCalling b_asf_parse_content_encr_ext_object \n");

    xencr = &global_xcencr;
    /* extended encryption data */
    xencr->size = basf_read_dword(cursor);
    NEXUS_Memory_Allocate(xencr->size, NULL, (void **)(&(xencr->data)));
    basf_read_arr(cursor, xencr->data, xencr->size);

    if (xencr->size != 0) {
        xencr->state = bdrm_hdr_read_ok;
    }

    return(ret);
}


/* Extract digital signature object from asf header
*/
static unsigned int
b_asf_parse_digi_sign_object (struct basf_parser_object_handler *handler, batom_cursor* cursor)
{
    unsigned int ret = 0;
    struct basf_digsign_object *digsign;
    basf_drm_object_handler* drmh = ((struct basf_drm_object_handler *)handler);
    BSTD_UNUSED(drmh);
printf("\nCalling b_asf_parse_digi_sign_object \n");
    digsign = &global_digsign;
    digsign->type = basf_read_dword(cursor);
    digsign->len = basf_read_dword(cursor);

    NEXUS_Memory_Allocate(digsign->len, NULL, (void **)(&(digsign->data)));
    basf_read_arr(cursor, digsign->data, digsign->len);

    if (digsign->len != 0) {
        digsign->state = bdrm_hdr_read_ok;
    }

    return(ret);
}


/* PLAYREADY: Extract protection system identifier object from asf header
*/
static unsigned int
b_asf_parse_psi_object (struct basf_parser_object_handler *handler, batom_cursor* cursor)
{
    unsigned int ret = 0;
    struct basf_psi_object *psi;
    basf_drm_object_handler* drmh = ((struct basf_drm_object_handler *)handler);
    BSTD_UNUSED(drmh);

    psi = &global_psi;
    basf_read_guid(cursor, &psi->systemId);
    psi->sysversion = basf_read_dword(cursor);
    psi->size = basf_read_dword(cursor);

    NEXUS_Memory_Allocate(psi->size, NULL, (void **)(&(psi->data)));
    basf_read_arr(cursor, psi->data, psi->size);

    if (psi->size != 0) {
        psi->state = bdrm_hdr_read_ok;
    }

    return(ret);
}




/* Content encryption object handler callback
*/
basf_parser_handler_action
b_asf_handler_cencr_object_cb (struct basf_parser_object_handler *handler, batom_t atom)
{
    batom_cursor cursor;

    batom_cursor_from_atom(&cursor, atom);
    b_asf_parse_content_encr_object(handler, &cursor);

    return(basf_parser_handler_action_none);
}

/* Extended content encryption object handler callback
*/
basf_parser_handler_action
b_asf_handler_xencr_object_cb (struct basf_parser_object_handler *handler, batom_t atom)
{
    batom_cursor cursor;

    batom_cursor_from_atom(&cursor, atom);
    b_asf_parse_content_encr_ext_object(handler, &cursor);

    return(basf_parser_handler_action_none);
}

/* Digital signature object handler callback
*/
basf_parser_handler_action
b_asf_handler_digsign_object_cb (struct basf_parser_object_handler *handler, batom_t atom)
{
    batom_cursor cursor;

    batom_cursor_from_atom(&cursor, atom);
    b_asf_parse_digi_sign_object(handler, &cursor);

    return(basf_parser_handler_action_none);
}


/* PLAYREADY : Protection system identifier object handler callback
*/
basf_parser_handler_action
b_asf_handler_psi_object_cb (struct basf_parser_object_handler *handler, batom_t atom)
{
    batom_cursor cursor;
    
    batom_cursor_from_atom(&cursor, atom);
    b_asf_parse_psi_object(handler, &cursor);

    return(basf_parser_handler_action_none);
}

static basf_feed_control 
b_asf_feed_state_handler(void *asf)
{
    basf_feed_control ret = basf_feed_continue;
    BSTD_UNUSED(asf);

    if ( global_psi.state == bdrm_hdr_read_ok || global_digsign.state == bdrm_hdr_read_ok || global_cencr.state == bdrm_hdr_read_ok || global_xcencr.state == bdrm_hdr_read_ok) {
        ret = basf_feed_stop;
    }
    return(ret);
}


void
b_asf_init_handlers (basf_parser_t asf, basf_props_t asfp, basf_drm_handlers asf_handler)
{
    /* don't launch internal scan if it's done outside of drmcore 
     * */
       
    printf("Calling b_asf_init_handlers 0\n");
        if(asf)
        {
        printf("Calling b_asf_init_handlers 1\n");
        /*PLAYREADY*/
        basf_parser_install_object_handler(asf, &asf_handler.psi_handler.handler, &basf_protection_system_identifier_guid, b_asf_handler_psi_object_cb);
       
        basf_parser_install_object_handler(asf, &asf_handler.encr_handler.handler, &basf_content_encr_guid, b_asf_handler_cencr_object_cb);
        printf("Calling b_asf_init_handlers 2\n");
        basf_parser_install_object_handler(asf, &asf_handler.xencr_handler.handler, &basf_content_encr_ext_guid, b_asf_handler_xencr_object_cb);
        printf("Calling b_asf_init_handlers 3\n");
        basf_parser_install_object_handler(asf, &asf_handler.digsign_handler.handler, &basf_content_digi_sign_guid, b_asf_handler_digsign_object_cb);
        printf("Calling b_asf_init_handlers 4\n");
        printf("Calling b_asf_init_handlers 5\n");

        asf_handler.state = basf_drm_handlers_init;       /* mark handlers as initialized */
        }
        else
        {
            printf("Calling b_asf_init_handlers 6\n");
            asf_handler.state = basf_drm_handlers_uninit;
        }

        basf_install_feed_handler(asfp, b_asf_feed_state_handler, (void*)asf);
        basf_scan_asf(asfp);                       /* read drm headers */

    return;
}


