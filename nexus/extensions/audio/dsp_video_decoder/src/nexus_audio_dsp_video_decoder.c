/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* API Description:
*
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_dsp_video_decoder);

BDBG_OBJECT_ID(NEXUS_DspVideoDecoder);

struct NEXUS_DspVideoDecoder {
    BDBG_OBJECT(NEXUS_DspVideoDecoder)
    bool opened;
    bool started;
    NEXUS_RaveStatus raveStatus;
    BVDE_ChannelHandle vdeChannel;
    BVDE_ChannelStartSettings vdeStartSettings;
    NEXUS_DspVideoDecoderStartSettings startSettings;
    BXDM_Decoder_Interface decoderInterface; /* keep a persisitent copy */
};

static struct NEXUS_DspVideoDecoder g_decoders[NEXUS_NUM_DSP_VIDEO_DECODERS];
static BKNI_EventHandle g_watchdogEvent;
static NEXUS_EventCallbackHandle g_watchdogCallback;

static void NEXUS_DspVideoDecoder_P_Watchdog_isr(void *pParam1, int param2);
static void NEXUS_DspVideoDecoder_P_Watchdog(void *pParam);

#define LOCK_TRANSPORT()    NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.transport)
#define UNLOCK_TRANSPORT()  NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.transport)

void NEXUS_DspVideoDecoder_GetDefaultStartSettings_priv( NEXUS_DspVideoDecoderStartSettings *pSettings )
{
    NEXUS_ASSERT_MODULE();
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

void NEXUS_DspVideoDecoder_GetDefaultOpenSettings_priv( NEXUS_DspVideoDecoderOpenSettings *pSettings )
{
    NEXUS_ASSERT_MODULE();
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->pictureHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mma;
}

NEXUS_DspVideoDecoderHandle NEXUS_DspVideoDecoder_Open_priv(unsigned index, const NEXUS_DspVideoDecoderOpenSettings *openSettings)
{
    NEXUS_DspVideoDecoderHandle  decoder;
    BVDE_ChannelOpenSettings channelOpenSettings;
    NEXUS_Error rc;

    NEXUS_ASSERT_MODULE();
    BSTD_UNUSED(openSettings);
    if(index>=NEXUS_NUM_DSP_VIDEO_DECODERS) { rc=BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_args;}

    decoder = &g_decoders[index];
    if ( decoder->opened )
    {
        BDBG_ERR(("DSP Video Decoder %u already opened", index));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_args;
    }

    BKNI_Memset(decoder, 0, sizeof(*decoder));
    BDBG_OBJECT_SET(decoder, NEXUS_DspVideoDecoder);
    decoder->opened = true;

    BVDE_Channel_GetDefaultOpenSettings(&channelOpenSettings);
    channelOpenSettings.memPicHandle = openSettings->pictureHeap;
    rc = BVDE_Channel_Open(g_NEXUS_vdeHandle, index, &channelOpenSettings, &decoder->vdeChannel);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_channel_open;}

    return decoder;

err_channel_open:
    BKNI_Memset(decoder, 0, sizeof(*decoder));
err_args:
    return NULL;
}

void NEXUS_DspVideoDecoder_Close_priv(NEXUS_DspVideoDecoderHandle decoder)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(decoder, NEXUS_DspVideoDecoder);
    BVDE_Channel_Close(decoder->vdeChannel);
    BKNI_Memset(decoder, 0, sizeof(*decoder));  /* Also invalidates object ID */
    return;
}

void NEXUS_DspVideoDecoder_GetDecoderInterface_priv(NEXUS_DspVideoDecoderHandle decoder, const BXDM_Decoder_Interface **decoderInterface, void **decoderContext)
{
    BERR_Code rc;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(decoder, NEXUS_DspVideoDecoder);

    rc = BVDE_Channel_GetDMInterface(decoder->vdeChannel, &decoder->decoderInterface, decoderContext);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);/* keep going */}
    *decoderInterface = &decoder->decoderInterface;
    return;
}

static const struct {
    NEXUS_VideoCodec nexusCodec;
    BAVC_VideoCompressionStd avcCodec;
} NEXUS_DspVideoDecoder_P_CodecTable[] = {
    {NEXUS_VideoCodec_eVp6,  BAVC_VideoCompressionStd_eVP6}
};

static BAVC_VideoCompressionStd NEXUS_DspVideoDecoder_P_GetAvcCodec( NEXUS_VideoCodec codec)
{
    unsigned i;
    for(i=0;i<sizeof(NEXUS_DspVideoDecoder_P_CodecTable)/sizeof(*NEXUS_DspVideoDecoder_P_CodecTable);i++) {
        if(NEXUS_DspVideoDecoder_P_CodecTable[i].nexusCodec == codec) {
            return NEXUS_DspVideoDecoder_P_CodecTable[i].avcCodec;
        }
    }
    BDBG_ERR(("Unsupported codec %u", (unsigned)codec));
    return BAVC_VideoCompressionStd_eMax;
}

NEXUS_Error NEXUS_DspVideoDecoder_Start_priv(NEXUS_DspVideoDecoderHandle decoder, const NEXUS_DspVideoDecoderStartSettings *startSettings)
{
    BERR_Code rc;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(decoder, NEXUS_DspVideoDecoder);

    if ( decoder->started )
    {
        BDBG_ERR(("Already running."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BVDE_Channel_GetDefaultStartSettings(&decoder->vdeStartSettings);
    decoder->vdeStartSettings.codec = NEXUS_DspVideoDecoder_P_GetAvcCodec(startSettings->codec);
    if(startSettings->raveContext) {
        LOCK_TRANSPORT();
        rc = NEXUS_Rave_GetStatus_priv(startSettings->raveContext, &decoder->raveStatus);
        UNLOCK_TRANSPORT();
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto  err_rave_status;}
        decoder->vdeStartSettings.pContextMap = &decoder->raveStatus.xptContextMap;
    }


    rc = BVDE_Channel_Start(decoder->vdeChannel, &decoder->vdeStartSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_start;}
    decoder->startSettings = *startSettings;
    decoder->started = true;

    return NEXUS_SUCCESS;

err_start:
err_rave_status:
    return BERR_TRACE(rc);
}

void NEXUS_DspVideoDecoder_Stop_priv(NEXUS_DspVideoDecoderHandle decoder)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(decoder, NEXUS_DspVideoDecoder);
    BVDE_Channel_Stop(decoder->vdeChannel);
    decoder->started = false;
    return;
}

NEXUS_Error NEXUS_DspVideoDecoder_Flush_priv(NEXUS_DspVideoDecoderHandle decoder)
{
    BERR_Code rc;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(decoder, NEXUS_DspVideoDecoder);

    if ( !decoder->started )
    {
        BDBG_ERR(("Decoder is not running.  Cannot flush."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }


    /* TODO: Do we need disable for flush/flush?  */
    BVDE_Channel_Stop(decoder->vdeChannel);

    if(decoder->startSettings.raveContext) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Disable_priv(decoder->startSettings.raveContext);
        NEXUS_Rave_Flush_priv(decoder->startSettings.raveContext);
        UNLOCK_TRANSPORT();
    }

    rc = BVDE_Channel_Start(decoder->vdeChannel, &decoder->vdeStartSettings);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    if(decoder->startSettings.raveContext) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Enable_priv(decoder->startSettings.raveContext);
        UNLOCK_TRANSPORT();
    }

    return NEXUS_SUCCESS;
}


void NEXUS_DspVideoDecoder_GetRaveSettings_priv(NEXUS_RaveOpenSettings *raveSettings)
{
    NEXUS_ASSERT_MODULE();
    /* TODO: Need API for this in VDE */

    raveSettings->config.Cdb.Length = 256*1024;
    raveSettings->config.Cdb.Alignment = 6; /* cache line on 4380 */
    raveSettings->config.Itb.Length = 128*1024;
    raveSettings->config.Itb.Alignment = 6; /* cache line on 4380 */
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
    raveSettings->config.Cdb.LittleEndian = true;
#else
    raveSettings->config.Cdb.LittleEndian = false;
#endif

    return;
}

NEXUS_Error NEXUS_DspVideoDecoder_P_InitWatchdog(void)
{
    BERR_Code errCode;
    BVDE_InterruptHandlers interrupts;

    errCode = BKNI_CreateEvent(&g_watchdogEvent);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    g_watchdogCallback = NEXUS_RegisterEvent(g_watchdogEvent, NEXUS_DspVideoDecoder_P_Watchdog, NULL);
    if ( NULL == g_watchdogCallback )
    {
        BKNI_DestroyEvent(g_watchdogEvent);
        g_watchdogEvent = NULL;
        return BERR_TRACE(BERR_OS_ERROR);
    }

    /* Allow watchdog to be disabled for debugging */
    if ( !NEXUS_GetEnv("no_watchdog") )
    {
        BVDE_GetInterruptHandlers(g_NEXUS_vdeHandle, &interrupts);
        interrupts.watchdog.pCallback_isr = NEXUS_DspVideoDecoder_P_Watchdog_isr;
        errCode = BVDE_SetInterruptHandlers(g_NEXUS_vdeHandle, &interrupts);
        if ( errCode )
        {
            NEXUS_UnregisterEvent(g_watchdogCallback);
            g_watchdogCallback = NULL;
            BKNI_DestroyEvent(g_watchdogEvent);
            g_watchdogEvent = NULL;
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

void NEXUS_DspVideoDecoder_P_UninitWatchdog(void)
{
    BVDE_InterruptHandlers interrupts;

    BVDE_GetInterruptHandlers(g_NEXUS_vdeHandle, &interrupts);
    interrupts.watchdog.pCallback_isr = NULL;
    BVDE_SetInterruptHandlers(g_NEXUS_vdeHandle, &interrupts);

    NEXUS_UnregisterEvent(g_watchdogCallback);
    g_watchdogCallback = NULL;
    BKNI_DestroyEvent(g_watchdogEvent);
    g_watchdogEvent = NULL;
}

static void NEXUS_DspVideoDecoder_P_Watchdog_isr(void *pParam1, int param2)
{
    BSTD_UNUSED(pParam1);
    BSTD_UNUSED(param2);
    BDBG_ERR(("DSP Video Watchdog Interrupt Received"));
    BKNI_SetEvent_isr(g_watchdogEvent);
}

static void NEXUS_DspVideoDecoder_P_Watchdog(void *pParam)
{
    unsigned i;

    BSTD_UNUSED(pParam);

    LOCK_TRANSPORT();
    for ( i = 0; i < NEXUS_NUM_DSP_VIDEO_DECODERS; i++ )
    {
        if ( g_decoders[i].opened && g_decoders[i].started )
        {
            NEXUS_Rave_Disable_priv(g_decoders[i].startSettings.raveContext);
            NEXUS_Rave_Flush_priv(g_decoders[i].startSettings.raveContext);
        }
    }
    UNLOCK_TRANSPORT();

    /* Process watchdog event */
    BVDE_ProcessWatchdogInterrupt(g_NEXUS_vdeHandle);

    /* Restart RAVE contexts */
    LOCK_TRANSPORT();
    for ( i = 0; i < NEXUS_NUM_DSP_VIDEO_DECODERS; i++ )
    {
        if ( g_decoders[i].opened && g_decoders[i].started )
        {
            NEXUS_Rave_Enable_priv(g_decoders[i].startSettings.raveContext);
        }
    }
    UNLOCK_TRANSPORT();
}
