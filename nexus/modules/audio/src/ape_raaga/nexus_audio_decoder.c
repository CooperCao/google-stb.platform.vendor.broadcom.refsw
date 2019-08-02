/***************************************************************************
*  Copyright (C) 2019 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* API Description:
*   API name: AudioDecoder
*    API for audio decoder management.
*
***************************************************************************/

#include "nexus_audio_module.h"
#include "priv/nexus_audio_decoder_priv.h"
#include "priv/nexus_pid_channel_priv.h"
#include "priv/nexus_stc_channel_priv.h"
#if NEXUS_HAS_SAGE
#include "priv/nexus_sage_priv.h"
#endif

/* Comment this line to disable debug logging */
#define RAAGA_DEBUG_LOG_CHANGES 1

#if NEXUS_AUDIO_RAVE_MONITOR_EXT
#define NEXUS_AUDIO_RAVE_MONITOR_CONSUME    0
#endif

#if BAPE_DSP_SUPPORT
#include "bdsp.h"
#endif

BDBG_MODULE(nexus_audio_decoder);
BDBG_FILE_MODULE(nexus_flow_audio_decoder);

#define BDBG_MSG_TRACE(X)

NEXUS_AudioDecoderHandle g_decoders[NEXUS_MAX_AUDIO_DECODERS];

static void NEXUS_AudioDecoder_P_Watchdog(void *pParam);
static void NEXUS_AudioDecoder_P_SampleRate(void *pParam);
static void NEXUS_AudioDecoder_P_ChannelChangeReport(void * context);
static void NEXUS_AudioDecoder_P_FirstPts_isr(void *pParam1, int param2, const BAPE_DecoderTsmStatus *pTsmStatus);
static void NEXUS_AudioDecoder_P_AudioTsmFail_isr(void *pParam1, int param2, const BAPE_DecoderTsmStatus *pTsmStatus);
static void NEXUS_AudioDecoder_P_AudioTsmPass_isr(void *pParam1, int param2, const BAPE_DecoderTsmStatus *pTsmStatus);
static void NEXUS_AudioDecoder_P_SampleRateChange_isr(void *pParam1, int param2, unsigned sampleRate);
static void NEXUS_AudioDecoder_P_Lock_isr(void *pParam1, int param2);
static void NEXUS_AudioDecoder_P_StreamStatusAvailable_isr(void *pParam1, int param2);
static void NEXUS_AudioDecoder_P_AncillaryData_isr(void *pParam1, int param2);
static void NEXUS_AudioDecoder_P_StreamParameterChanged_isr(void *pParam1, int param2);
static void NEXUS_AudioDecoder_P_CdbItbOverflow_isr(void *pParam1, int param2);
static void NEXUS_AudioDecoder_P_CdbItbUnderflow_isr(void *pParam1, int param2);
static BERR_Code NEXUS_AudioDecoder_P_GetPtsCallback_isr(void *pContext, BAVC_PTSInfo *pPTSInfo);
static BERR_Code NEXUS_AudioDecoder_P_GetCdbLevelCallback_isr(void *pContext, unsigned *pCdbLevel);
static BERR_Code NEXUS_AudioDecoder_P_StcValidCallback_isr(void *pContext);
static void NEXUS_AudioDecoder_P_FifoWatchdog(void *context);
static void NEXUS_AudioDecoder_P_StcChannelStatusChangeEventHandler(void *context);
static void NEXUS_AudioDecoder_P_Watchdog_isr(void *pParam1, int param2);
static void NEXUS_AudioDecoder_P_InputFormatChange_isr(void *pParam1, int param2);
static void NEXUS_AudioDecoder_P_InputFormatChange(void *pContext);
static BERR_Code NEXUS_AudioDecoder_P_SetPcrOffset_isr(void *pContext, uint32_t pcrOffset);
static BERR_Code NEXUS_AudioDecoder_P_GetPcrOffset_isr(void *pContext, uint32_t *pPcrOffset);
static void NEXUS_AudioDecoder_P_DialnormChanged_isr(void *pParam1, int param2);
static void NEXUS_AudioDecoder_P_UnlicensedAlgo_isr(void *pParam1, int param2);
static void NEXUS_AudioDecoder_P_UnlicensedAlgo(void *pParam);
static NEXUS_Error NEXUS_AudioDecoder_P_SetCompressedMute(NEXUS_AudioDecoderHandle decoder, bool muted);
static NEXUS_Error NEXUS_AudioDecoder_P_SetCompressedEncoderMute(NEXUS_AudioDecoderHandle decoder, bool muted);
static NEXUS_Error NEXUS_AudioDecoder_P_EnableRaveContexts(NEXUS_AudioDecoderHandle handle, NEXUS_AudioDecoderStartSettings * program);
static void NEXUS_AudioDecoder_P_DisableRaveContexts(NEXUS_AudioDecoderHandle handle, bool flush);

#define LOCK_TRANSPORT()    NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.transport)
#define UNLOCK_TRANSPORT()  NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.transport)

#define NEXUS_AUDIO_DECODER_P_PULL_DISCARD_THRESHOLD_MPEG (0x7fffffff / 45) /* 47722 seconds (largest positive int we can represent in 45 KHz domain) */
#define NEXUS_AUDIO_DECODER_P_PULL_DISCARD_THRESHOLD_DSS (0x7fffffff / 27000) /* 79.5 seconds (largest positive int we can represent in 27 MHz domain) */
#define NEXUS_AUDIO_DECODER_P_PUSH_DISCARD_THRESHOLD (3000) /* 3 seconds */
#define NEXUS_AUDIO_DECODER_P_GET_DISCARD_THRESHOLD(IS_PULL, IS_DSS) \
    ((IS_PULL) ? \
        ((IS_DSS) ? \
            NEXUS_AUDIO_DECODER_P_PULL_DISCARD_THRESHOLD_DSS : \
            NEXUS_AUDIO_DECODER_P_PULL_DISCARD_THRESHOLD_MPEG) : \
        NEXUS_AUDIO_DECODER_P_PUSH_DISCARD_THRESHOLD)

static BKNI_EventHandle g_watchdogEvent;
static NEXUS_EventCallbackHandle g_watchdogCallback;
static unsigned g_numWatchdogs;

static void NEXUS_AudioDecoder_P_Watchdog_isr(void *pParam1, int param2)
{
    BSTD_UNUSED(pParam1);
    BSTD_UNUSED(param2);
    BDBG_ERR(("Audio Watchdog Interrupt Received"));
    BKNI_SetEvent_isr(g_watchdogEvent);
}

/* PAK is only required on non-bp3 chips */
#if ((BCHP_CHIP == 7278 && BCHP_VER < BCHP_VER_B0) || (BCHP_CHIP == 7439 && BCHP_VER > BCHP_VER_A0) || \
     (BCHP_CHIP == 7250 && BCHP_VER > BCHP_VER_A0) || (BCHP_CHIP == 7364) || \
     (BCHP_CHIP == 7445 && BCHP_VER > BCHP_VER_C0) || (BCHP_CHIP == 7252 && BCHP_VER > BCHP_VER_C0) || \
     (BCHP_CHIP == 7366 && BCHP_VER > BCHP_VER_A0) || (BCHP_CHIP == 7260 && BCHP_VER <= BCHP_VER_B0) ||\
     (BCHP_CHIP == 7271 && BCHP_VER < BCHP_VER_C0) || (BCHP_CHIP == 7268 && BCHP_VER < BCHP_VER_C0))
#define NEXUS_AUDIO_PAK_SUPPORT 1
#else
#define NEXUS_AUDIO_PAK_SUPPORT 0
#endif

#if NEXUS_AUDIO_PAK_SUPPORT
extern void *NEXUS_AUDIO_IMG_Context;
extern BIMG_Interface NEXUS_AUDIO_IMG_Interface;

static NEXUS_Error NEXUS_AudioDecoder_P_Img_Create(void)
{
#if defined(NEXUS_MODE_driver)
    NEXUS_Error rc = BERR_SUCCESS;

    rc = Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_AUDIO_PAK, &g_NEXUS_audioModuleData.pPakContext, &g_NEXUS_audioModuleData.pakImg);
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        g_NEXUS_audioModuleData.pPakContext = NULL;
    }
#else
    g_NEXUS_audioModuleData.pPakContext = NEXUS_AUDIO_IMG_Context;
    g_NEXUS_audioModuleData.pakImg = NEXUS_AUDIO_IMG_Interface;
#endif

    return NEXUS_SUCCESS;
}

static void NEXUS_AudioDecoder_P_Img_Destroy(void)
{
#if defined(NEXUS_MODE_driver)
    Nexus_Core_P_Img_Destroy(g_NEXUS_audioModuleData.pPakContext);
#endif
}

static NEXUS_Error NEXUS_AudioDecoder_P_LoadFile(
    BIMG_Interface *pImg,
    void *pContext,
    NEXUS_AudioImage image,
    NEXUS_MemoryBlockHandle *pBlock,
    void **pMemory,
    NEXUS_Addr *pOffset,
    unsigned *pLength
    )
{
    NEXUS_Error errCode;
    void *pImage = NULL;
    uint32_t *pSize;
    const void *pBuffer;

    BDBG_ASSERT(NULL != pImg);
    BDBG_ASSERT(NULL != pContext);
    BDBG_ASSERT(NULL != pBlock);
    BDBG_ASSERT(NULL != pMemory);
    BDBG_ASSERT(NULL != pOffset);

    *pBlock = NULL;
    *pMemory = NULL;
    *pOffset = 0;
    *pLength = 0;

    /* Open file */
    errCode = pImg->open(pContext, &pImage, image);
    if( errCode )
    {
        if ( errCode != BERR_INVALID_PARAMETER )
        {
            errCode = BERR_TRACE(errCode);
        }
        goto err;
    }

    /* Get size */
    errCode = pImg->next(pImage, 0, (const void **)&pSize, sizeof(uint32_t));
    if( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err;
    }

    BDBG_ASSERT(NULL != pSize);

    *pLength = *pSize;
    *pBlock = NEXUS_MemoryBlock_Allocate(NULL, *pLength, 4096, NULL);
    if ( NULL == pBlock )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto err;
    }

    errCode = NEXUS_MemoryBlock_Lock(*pBlock, pMemory);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err;
    }

    errCode = pImg->next(pImage, 0, &pBuffer, *pLength);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err;
    }

    BKNI_Memcpy(*pMemory, pBuffer, *pLength);
    NEXUS_FlushCache(*pMemory, *pLength);

    pImg->close(pImage);
    pImage = NULL;

    errCode = NEXUS_MemoryBlock_LockOffset(*pBlock, pOffset);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err;
    }

    return NEXUS_SUCCESS;

err:
    if ( pImage )
    {
        pImg->close(pImage);
    }
    if ( *pBlock )
    {
        if ( *pOffset )
        {
            NEXUS_MemoryBlock_UnlockOffset(*pBlock);
        }
        if ( *pMemory )
        {
            NEXUS_MemoryBlock_Unlock(*pBlock);
        }
        NEXUS_MemoryBlock_Free(*pBlock);
    }

    return errCode;
}

void NEXUS_AudioDecoder_P_LoadPak(void)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;

    void *pContext = g_NEXUS_audioModuleData.pPakContext;
    BIMG_Interface *pImg = &g_NEXUS_audioModuleData.pakImg;
    NEXUS_MemoryBlockHandle hPak= NULL, hDrm = NULL;
    void *pPakMem = NULL, *pDrmMem = NULL;
    NEXUS_Addr pakAddr=0, drmAddr=0;
    unsigned pakLength, drmLength;
    BDSP_ProcessPAKSettings pakSettings;
    BDSP_ProcessPAKStatus pakStatus;
    NEXUS_AudioImage imageType = NEXUS_AudioImage_ePak;


    BDBG_MSG(("Loading PAK"));
#if NEXUS_HAS_SAGE
    {
        BSAGElib_ChipInfo chipInfo;
        NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.sage);
        NEXUS_Sage_GetChipInfo_priv(&chipInfo);
        NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.sage);
        imageType = (chipInfo.chipType == BSAGElib_ChipType_eZS) ? NEXUS_AudioImage_ePakDev : NEXUS_AudioImage_ePak;

        if ( imageType == NEXUS_AudioImage_ePakDev )
        {
            BCHP_Info chpInfo;

            BCHP_GetInfo(g_NEXUS_pCoreHandles->chp, &chpInfo);
            switch ( chpInfo.productId )
            {
            case 0x72501: imageType = NEXUS_AudioImage_ePakDev_72501; break;
            case 0x72502: imageType = NEXUS_AudioImage_ePakDev_72502; break;
            case 0x7250:  imageType = NEXUS_AudioImage_ePakDev_7250;  break;
            case 0x72511: imageType = NEXUS_AudioImage_ePakDev_72511; break;
            case 0x72521: imageType = NEXUS_AudioImage_ePakDev_72521; break;
            case 0x72525: imageType = NEXUS_AudioImage_ePakDev_72525; break;
            case 0x72603: imageType = NEXUS_AudioImage_ePakDev_72603; break;
            case 0x72604: imageType = NEXUS_AudioImage_ePakDev_72604; break;
            case 0x7260:  imageType = NEXUS_AudioImage_ePakDev_7260;  break;
            case 0x7268:  imageType = NEXUS_AudioImage_ePakDev_7268;  break;
            case 0x7271:  imageType = NEXUS_AudioImage_ePakDev_7271;  break;
            case 0x73649: imageType = NEXUS_AudioImage_ePakDev_73649; break;
            case 0x7364:  imageType = NEXUS_AudioImage_ePakDev_7364;  break;
            case 0x7366:  imageType = NEXUS_AudioImage_ePakDev_7366;  break;
            case 0x7367:  imageType = NEXUS_AudioImage_ePakDev_7367;  break;
            case 0x74381: imageType = NEXUS_AudioImage_ePakDev_74381; break;
            case 0x7444:  imageType = NEXUS_AudioImage_ePakDev_7444;  break;
            case 0x7445:  imageType = NEXUS_AudioImage_ePakDev_7445;  break;
            case 0x74481: imageType = NEXUS_AudioImage_ePakDev_74481; break;
            case 0x74491: imageType = NEXUS_AudioImage_ePakDev_74491; break;
            case 0x74495: imageType = NEXUS_AudioImage_ePakDev_74495; break;
            default: break;
            }
        }
    }
#endif
    errCode = NEXUS_AudioDecoder_P_LoadFile(pImg, pContext, imageType, &hPak, &pPakMem, &pakAddr, &pakLength);
    if ( errCode )
    {
        if ( errCode == BERR_INVALID_PARAMETER && imageType > NEXUS_AudioImage_ePakDev )
        {
            BDBG_MSG(("product pak_dev.bin not found, trying pak_dev.bin"));
            imageType = NEXUS_AudioImage_ePakDev;
            errCode = NEXUS_AudioDecoder_P_LoadFile(pImg, pContext, imageType, &hPak, &pPakMem, &pakAddr, &pakLength);
        }
        if ( errCode == BERR_INVALID_PARAMETER && imageType == NEXUS_AudioImage_ePakDev )
        {
            BDBG_MSG(("pak_dev.bin not found, trying pak.bin"));
            errCode = NEXUS_AudioDecoder_P_LoadFile(pImg, pContext, NEXUS_AudioImage_ePak, &hPak, &pPakMem, &pakAddr, &pakLength);
        }
        if ( errCode )
        {
            if ( errCode == BERR_INVALID_PARAMETER )
            {
                BDBG_WRN(("Unable to load pak.bin file"));
            }
            else
            {
                errCode = BERR_TRACE(errCode);
            }
            goto err;
        }
    }

    BDBG_MSG(("Done loading PAK - Loading DRM"));
    errCode = NEXUS_AudioDecoder_P_LoadFile(pImg, pContext, NEXUS_AudioImage_eDrm, &hDrm, &pDrmMem, &drmAddr, &drmLength);
    if ( errCode )
    {
        if ( errCode == BERR_INVALID_PARAMETER )
        {
            BDBG_MSG(("drm.bin not found, trying drm_dev.bin"));
            errCode = NEXUS_AudioDecoder_P_LoadFile(pImg, pContext, NEXUS_AudioImage_eDrmDev, &hDrm, &pDrmMem, &drmAddr, &drmLength);
        }
        if ( errCode )
        {
            if ( errCode == BERR_INVALID_PARAMETER )
            {
                BDBG_WRN(("Unable to load drm.bin"));
            }
            else
            {
                errCode = BERR_TRACE(errCode);
            }
            goto err;
        }
    }
    BDBG_MSG(("Done Loading DRM"));

    BDSP_GetDefaultProcessPAKSettings(&pakSettings);
    pakSettings.pakMemory.hBlock = NEXUS_MemoryBlock_GetBlock_priv(hPak);
    pakSettings.pakMemory.pAddr = pPakMem;
    pakSettings.pakMemory.offset = pakAddr;
    pakSettings.pakSize = pakLength;
    pakSettings.drmMemory.hBlock = NEXUS_MemoryBlock_GetBlock_priv(hDrm);
    pakSettings.drmMemory.pAddr = pDrmMem;
    pakSettings.drmMemory.offset = drmAddr;
    pakSettings.drmSize = drmLength;

    errCode = BDSP_ProcessPAK(g_NEXUS_audioModuleData.dspHandle, &pakSettings, &pakStatus);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err;
    }

    BDBG_WRN(("Successfully loaded PAK"));

    errCode = NEXUS_SUCCESS;
    /* Fall through to clean up */
err:
    if ( hPak )
    {
        if ( pPakMem )
        {
            NEXUS_MemoryBlock_Unlock(hPak);
        }
        if ( pakAddr )
        {
            NEXUS_MemoryBlock_UnlockOffset(hPak);
        }
        NEXUS_MemoryBlock_Free(hPak);
    }
    if ( hDrm )
    {
        if ( pDrmMem )
        {
            NEXUS_MemoryBlock_Unlock(hDrm);
        }
        if ( drmAddr )
        {
            NEXUS_MemoryBlock_UnlockOffset(hDrm);
        }
        NEXUS_MemoryBlock_Free(hDrm);
    }
    if ( errCode )
    {
        BDBG_WRN(("Unable to load PAK data"));
        if ( errCode != BERR_INVALID_PARAMETER )
        {
            (void)BERR_TRACE(errCode);
        }
    }
    return;
}
#else
void NEXUS_AudioDecoder_P_LoadPak(void)
{
    /* Stub */
}
#endif /* NEXUS_AUDIO_PAK_SUPPORT */

NEXUS_Error NEXUS_AudioDecoder_P_Init(void)
{
    BERR_Code errCode;
    BAPE_InterruptHandlers interrupts;

    g_numWatchdogs = 0;
    errCode = BKNI_CreateEvent(&g_watchdogEvent);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    g_watchdogCallback = NEXUS_RegisterEvent(g_watchdogEvent, NEXUS_AudioDecoder_P_Watchdog, NULL);
    if ( NULL == g_watchdogCallback )
    {
        BKNI_DestroyEvent(g_watchdogEvent);
        g_watchdogEvent = NULL;
        return BERR_TRACE(BERR_OS_ERROR);
    }

    /* Allow watchdog to be disabled for debugging */
    if ( !NEXUS_GetEnv("no_watchdog") && g_NEXUS_audioModuleData.settings.watchdogEnabled )
    {
        BAPE_GetInterruptHandlers(NEXUS_AUDIO_DEVICE_HANDLE, &interrupts);
        interrupts.watchdog.pCallback_isr = NEXUS_AudioDecoder_P_Watchdog_isr;
        errCode = BAPE_SetInterruptHandlers(NEXUS_AUDIO_DEVICE_HANDLE, &interrupts);
        if ( errCode )
        {
            NEXUS_UnregisterEvent(g_watchdogCallback);
            g_watchdogCallback = NULL;
            BKNI_DestroyEvent(g_watchdogEvent);
            g_watchdogEvent = NULL;
            return BERR_TRACE(errCode);
        }
    }

    /* initialize global decoder data */
    BKNI_Memset(&g_decoders, 0, sizeof(g_decoders));

#if NEXUS_AUDIO_PAK_SUPPORT
    /* Initialize IMG interface; used to pull out an image on the file system from the kernel. */
    errCode = NEXUS_AudioDecoder_P_Img_Create();
    if ( errCode )
    {
        BAPE_GetInterruptHandlers(NEXUS_AUDIO_DEVICE_HANDLE, &interrupts);
        interrupts.watchdog.pCallback_isr = NULL;
        (void)BAPE_SetInterruptHandlers(NEXUS_AUDIO_DEVICE_HANDLE, &interrupts);
        NEXUS_UnregisterEvent(g_watchdogCallback);
        g_watchdogCallback = NULL;
        BKNI_DestroyEvent(g_watchdogEvent);
        g_watchdogEvent = NULL;
        return BERR_TRACE(errCode);
    }

    NEXUS_AudioDecoder_P_LoadPak();
#endif

    return BERR_SUCCESS;
}

void NEXUS_AudioDecoder_P_Uninit(void)
{
    BAPE_InterruptHandlers interrupts;

    BAPE_GetInterruptHandlers(NEXUS_AUDIO_DEVICE_HANDLE, &interrupts);
    interrupts.watchdog.pCallback_isr = NULL;
    BAPE_SetInterruptHandlers(NEXUS_AUDIO_DEVICE_HANDLE, &interrupts);

#if NEXUS_AUDIO_PAK_SUPPORT
    NEXUS_AudioDecoder_P_Img_Destroy();
#endif

    NEXUS_UnregisterEvent(g_watchdogCallback);
    g_watchdogCallback = NULL;
    BKNI_DestroyEvent(g_watchdogEvent);
    g_watchdogEvent = NULL;
}

/***************************************************************************
Summary:
Get default open settings for an audio decoder
***************************************************************************/
void NEXUS_AudioDecoder_GetDefaultOpenSettings(
    NEXUS_AudioDecoderOpenSettings *pSettings   /* [out] default settings */
    )
{
    BAPE_DecoderOpenSettings apeSettings;

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->independentDelay = g_NEXUS_audioModuleData.settings.independentDelay;
    BAPE_Decoder_GetDefaultOpenSettings(&apeSettings);
    pSettings->multichannelFormat = apeSettings.multichannelFormat==BAPE_MultichannelFormat_e7_1 ? NEXUS_AudioMultichannelFormat_e7_1:NEXUS_AudioMultichannelFormat_e5_1;
    pSettings->ancillaryDataFifoSize = apeSettings.ancillaryDataFifoSize;
}

static void NEXUS_AudioDecoder_P_GetDefaultKaraokeSettings(
    NEXUS_AudioDecoderKaraokeSettings *pSettings /* [out] default settings */
    )
{
    BAPE_DecoderKaraokeSettings karaokeSettings;

    BAPE_Decoder_GetDefaultKaraokeSettings(&karaokeSettings);
    pSettings->vocalSuppressionLevel = karaokeSettings.vocalSuppressionLevel;
    pSettings->vocalSuppressionFrequency = karaokeSettings.vocalSuppressionFrequency;
    pSettings->outputMakeupBoost = karaokeSettings.outputMakeupBoost;
}

static void NEXUS_AudioDecoder_P_GetDefaultSettings(
    NEXUS_AudioDecoderSettings *pSettings   /* [out] default settings */
    )
{
    int i;
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioDecoderSettings));
    pSettings->loudnessEquivalenceEnabled = true;
    for ( i = 0; i < NEXUS_AudioChannel_eMax; i++ )
    {
        pSettings->volumeMatrix[i][i] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
    }
    NEXUS_AudioDecoder_P_GetDefaultKaraokeSettings(&pSettings->karaokeSettings);
    NEXUS_CallbackDesc_Init(&pSettings->sourceChanged);
    NEXUS_CallbackDesc_Init(&pSettings->lockChanged);
    NEXUS_CallbackDesc_Init(&pSettings->ptsError);
    NEXUS_CallbackDesc_Init(&pSettings->firstPts);
    NEXUS_CallbackDesc_Init(&pSettings->fifoUnderflow);
    NEXUS_CallbackDesc_Init(&pSettings->fifoOverflow);
    NEXUS_CallbackDesc_Init(&pSettings->streamStatusAvailable);
    NEXUS_CallbackDesc_Init(&pSettings->ancillaryData);
    NEXUS_CallbackDesc_Init(&pSettings->dialnormChanged);

}

NEXUS_Error NEXUS_AudioDecoder_P_ConfigureRave(NEXUS_RaveHandle rave, const NEXUS_AudioDecoderStartSettings *pProgram, const NEXUS_PidChannelStatus * pPidStatus, bool isPlayback)
{
    NEXUS_RaveStatus raveStatus;
    NEXUS_RaveSettings raveSettings;
    NEXUS_Error errCode;

    LOCK_TRANSPORT();
    errCode = NEXUS_Rave_GetStatus_priv(rave, &raveStatus);
    if ( errCode == NEXUS_SUCCESS && !raveStatus.enabled )
    {
        NEXUS_Rave_GetDefaultSettings_priv(&raveSettings);
        raveSettings.pidChannel = pProgram->pidChannel;
        /* band hold must have both a pid channel playback source and be playback TSM or no TSM to be enabled */
        raveSettings.bandHold = pPidStatus->playback && !(pProgram->stcChannel && !isPlayback);
        /* CC should only be enabled for live pid channels */
        raveSettings.continuityCountEnabled = !pPidStatus->playback;
        raveSettings.numOutputBytesEnabled = true;
        raveSettings.nonRealTime = pProgram->nonRealTime;
        raveSettings.audioDescriptor = pProgram->secondaryDecoder; /* We need descriptor values for any secondary decoder */
        BDBG_MSG(("Configure Rave cxt %p", (void*)rave));
        errCode = NEXUS_Rave_ConfigureAudio_priv(rave, pProgram->codec, &raveSettings);
        if (errCode)
        {
            BERR_TRACE(errCode);
        }
    }
    UNLOCK_TRANSPORT();
    return errCode;
}

static void NEXUS_AudioDecoder_P_VerifyTimebase(NEXUS_AudioDecoderHandle handle)
{
    int i, j;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_WRN(("Audio Decoder %u using Timebase %u", handle->index, handle->apeStartSettings.stcIndex));
    for ( i = 0; i < NEXUS_AudioConnectorType_eMax; i++ ) {
        for ( j = 0; j < NEXUS_AUDIO_MAX_OUTPUTS; j++ ) {
            NEXUS_AudioOutputHandle output = handle->outputLists[i].outputs[j];
            if(output) {
                NEXUS_AudioOutput_P_VerifyTimebase(output);
            }
        }
    }
}

/***************************************************************************
Summary:
Open an audio decoder of the specified type
***************************************************************************/
NEXUS_AudioDecoderHandle NEXUS_AudioDecoder_Open( /* attr{destructor=NEXUS_AudioDecoder_Close}  */
    unsigned index,
    const NEXUS_AudioDecoderOpenSettings *pSettings   /* settings */
    )
{
    NEXUS_AudioDecoderHandle handle;
    NEXUS_AudioDecoderOpenSettings openSettings;
    NEXUS_RaveOpenSettings raveSettings;
    BAPE_DecoderOpenSettings decoderOpenSettings;
    BAPE_DecoderInterruptHandlers interrupts;
    BAPE_Connector mixerInput;
    BERR_Code errCode;
    unsigned j;
    NEXUS_AudioCapabilities audioCapabilities;

    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if ( index == NEXUS_ANY_ID )
    {
        for ( index = 0; index < audioCapabilities.numDecoders; index++ )
        {
            if (!g_decoders[index]) break;
        }
    }

    if ( index >= audioCapabilities.numDecoders )
    {
        errCode=BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    handle = g_decoders[index];

    /* Check if handle is already open */
    if ( handle )
    {
        BDBG_ERR(("Decoder %d already opened", index));
        errCode=BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) {
        errCode=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    if ( NULL == pSettings )
    {
        NEXUS_AudioDecoder_GetDefaultOpenSettings(&openSettings);
        pSettings = &openSettings;
    }

    NEXUS_OBJECT_INIT(NEXUS_AudioDecoder, handle);
    handle->index = index;
    handle->trickMute = false;
    handle->trickForceStopped = false;
    handle->suspended = false;
    handle->openSettings = *pSettings;

#if NEXUS_HAS_ASTM
    BKNI_Memset(&handle->astm.settings, 0, sizeof(NEXUS_AudioDecoderAstmSettings));
    BKNI_Memset(&handle->astm.status, 0, sizeof(NEXUS_AudioDecoderAstmStatus));
#endif

    if(pSettings->spliceEnabled) {
        handle->spliceCallback = NEXUS_IsrCallback_Create(handle, NULL);
        if(!handle->spliceCallback) {errCode=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_sample_rate_event;}
        BKNI_Memset(&handle->spliceSettings,0,sizeof(NEXUS_AudioDecoderSpliceSettings));
        NEXUS_CALLBACKDESC_INIT(&handle->spliceSettings.splicePoint);
        handle->spliceFlowStopped = false;
    }
    for ( j = 0; j < NEXUS_AudioConnectorType_eMax; j++ )
    {
        /* Setup handle linkage */
        BKNI_Snprintf(handle->name, sizeof(handle->name), "DECODER %u",index);
        NEXUS_AUDIO_INPUT_INIT(&handle->connectors[j], NEXUS_AudioInputType_eDecoder, handle);
        NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &handle->connectors[j], Open);
        handle->connectors[j].pName = handle->name;

        /* Set format per-connector */
        if ( j == NEXUS_AudioConnectorType_eStereo || j == NEXUS_AudioConnectorType_eAlternateStereo )
        {
            handle->connectors[j].format = NEXUS_AudioInputFormat_ePcmStereo;
        }
        else if ( j == NEXUS_AudioConnectorType_eMultichannel )
        {
            /* The correct value for this is set later if multichannel is enabled */
            handle->connectors[j].format = NEXUS_AudioInputFormat_eNone;
        }
        else if ( j == NEXUS_AudioConnectorType_eMono )
        {
            handle->connectors[j].format = NEXUS_AudioInputFormat_ePcmMono;
        }
        else
        {
            handle->connectors[j].format = NEXUS_AudioInputFormat_eCompressed;
        }

        /* Invalidate outputs */
        BKNI_Memset(&(handle->outputLists[j]), 0, sizeof(NEXUS_AudioOutputList));
    }

    NEXUS_AudioDecoder_P_TrickReset(handle);

    /* Create Events */
    errCode = BKNI_CreateEvent(&handle->sampleRateEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_sample_rate_event;
    }

    handle->sampleRateCallback = NEXUS_RegisterEvent(handle->sampleRateEvent, NEXUS_AudioDecoder_P_SampleRate, handle);
    if ( NULL == handle->sampleRateCallback )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        goto err_sample_rate_callback;
    }

    errCode = BKNI_CreateEvent(&handle->channelChangeReportEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_channel_change_report_event;
    }

    handle->channelChangeReportEventHandler = NEXUS_RegisterEvent(handle->channelChangeReportEvent, NEXUS_AudioDecoder_P_ChannelChangeReport, handle);
    if ( NULL == handle->channelChangeReportEventHandler )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        goto err_channel_change_report_event_handler;
    }

    errCode = BKNI_CreateEvent(&handle->inputFormatChangeEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_input_format_change_event;
    }

    handle->inputFormatChangeEventHandler = NEXUS_RegisterEvent(handle->inputFormatChangeEvent, NEXUS_AudioDecoder_P_InputFormatChange, handle);
    if ( NULL == handle->inputFormatChangeEventHandler )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        goto err_input_format_change_event_handler;
    }

    errCode = BKNI_CreateEvent(&handle->stc.statusChangeEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_stc_status_change_event;
    }

    handle->stc.statusChangeEventHandler = NEXUS_RegisterEvent(handle->stc.statusChangeEvent, NEXUS_AudioDecoder_P_StcChannelStatusChangeEventHandler, handle);
    if ( NULL == handle->stc.statusChangeEventHandler )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        goto err_stc_status_change_event_handler;
    }

    errCode = BKNI_CreateEvent(&handle->unlicensedAlgoEvent);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_unlicensed_algo_event;
    }

    handle->unlicensedAlgoEventHandler = NEXUS_RegisterEvent(handle->unlicensedAlgoEvent, NEXUS_AudioDecoder_P_UnlicensedAlgo, handle);
    if ( NULL == handle->unlicensedAlgoEventHandler )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        goto err_unlicensed_algo_event_handler;
    }

    if ( NEXUS_GetEnv("multichannel_disabled") || NEXUS_GetEnv("audio_processing_disabled") )
    {
        handle->connectors[NEXUS_AudioConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcmStereo;
    }
    else
    {
        switch ( pSettings->multichannelFormat )
        {
        case NEXUS_AudioMultichannelFormat_eNone:
            /* There is no benefit to disabling multichannel audio on APE it does not save memory.  Always use 5.1 as default. */
        case NEXUS_AudioMultichannelFormat_e5_1:
            handle->connectors[NEXUS_AudioConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcm5_1;
            #if NEXUS_NUM_HDMI_OUTPUTS && 0
            /* TODO: Notify PI of 5.1 vs. 7.1 */
            pChannelSettings->sChnRBufPool.uiMaxNumOutChPairs[0] += 3;
            #endif
            break;
        case NEXUS_AudioMultichannelFormat_e7_1:
            handle->connectors[NEXUS_AudioConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcm7_1;
            #if NEXUS_NUM_HDMI_OUTPUTS && 0
            /* TODO: Notify PI of 5.1 vs. 7.1 */
            pChannelSettings->sChnRBufPool.uiMaxNumOutChPairs[0] += 4;
            #endif
            break;
        default:
            BDBG_ERR(("Unsupported multichannel audio format"));
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto err_channel;
        }
    }
    if ( pSettings->independentDelay && !g_NEXUS_audioModuleData.settings.independentDelay )
    {
        BDBG_ERR(("Independent delay must be enabled at the audio module level.  Please check NEXUS_AudioModuleSettings.independentDelay."));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_channel;
    }

    /* Open APE decoder */
    BAPE_Decoder_GetDefaultOpenSettings(&decoderOpenSettings);
    decoderOpenSettings.dspIndex = pSettings->dspIndex;
    if ( g_NEXUS_audioModuleData.armHandle && !g_NEXUS_audioModuleData.dspHandle )
    {
        if ( decoderOpenSettings.dspIndex < BAPE_DEVICE_ARM_FIRST )
        {
            decoderOpenSettings.dspIndex += BAPE_DEVICE_ARM_FIRST;
        }

        if ( decoderOpenSettings.dspIndex >= (BAPE_DEVICE_ARM_FIRST+g_NEXUS_audioModuleData.capabilities.numSoftAudioCores) )
        {
            decoderOpenSettings.dspIndex = BAPE_DEVICE_ARM_FIRST;
        }
    }

    /* detect and report any mismatches of secure cdb settings */
    if ( pSettings->cdbHeap != NULL )
    {
        NEXUS_HeapRuntimeSettings heapRTSettings;
        bool deviceIsSecureCapable = false;

        NEXUS_Heap_GetRuntimeSettings_priv(pSettings->cdbHeap, &heapRTSettings);

        if ( BAPE_DEVICE_ARM_VALID(decoderOpenSettings.dspIndex) && g_NEXUS_audioModuleData.capabilities.dsp.softAudioSecureDecode )
        {
            deviceIsSecureCapable = true;
        }
        else if ( BAPE_DEVICE_DSP_VALID(decoderOpenSettings.dspIndex) && g_NEXUS_audioModuleData.capabilities.dsp.dspSecureDecode )
        {
            deviceIsSecureCapable = true;
        }

        if ( heapRTSettings.secure && !deviceIsSecureCapable )
        {
            BDBG_ERR(("Secure heap requested, but decoder device is not a secure client"));
            BERR_TRACE(BERR_INVALID_PARAMETER);
            goto err_channel;
        }
    }

    decoderOpenSettings.ancillaryDataFifoSize = pSettings->ancillaryDataFifoSize;
    decoderOpenSettings.karaokeSupported = pSettings->karaokeSupported;
    decoderOpenSettings.multichannelFormat = pSettings->multichannelFormat==NEXUS_AudioMultichannelFormat_e7_1 ? BAPE_MultichannelFormat_e7_1 : BAPE_MultichannelFormat_e5_1;
    switch ( pSettings->type )
    {
    default:
    case NEXUS_AudioDecoderType_eDecode:
        decoderOpenSettings.type = BAPE_DecoderType_eUniversal;
        break;
    case NEXUS_AudioDecoderType_eAudioDescriptor:
        decoderOpenSettings.type = BAPE_DecoderType_eDecode;
        break;
    case NEXUS_AudioDecoderType_ePassthrough:
        decoderOpenSettings.type = BAPE_DecoderType_ePassthrough;
        break;
    case NEXUS_AudioDecoderType_eDecodeToMemory:
        decoderOpenSettings.type = BAPE_DecoderType_eDecodeToMemory;
        break;
    }
    errCode = BAPE_Decoder_Open(NEXUS_AUDIO_DEVICE_HANDLE, index, &decoderOpenSettings, &handle->channel);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_channel;
    }

    /* Init connectors */
    BAPE_Decoder_GetConnector(handle->channel, BAPE_ConnectorFormat_eStereo, &mixerInput);
    handle->connectors[NEXUS_AudioConnectorType_eStereo].port = (size_t)mixerInput;
    BAPE_Decoder_GetConnector(handle->channel, BAPE_ConnectorFormat_eAlternateStereo, &mixerInput);
    handle->connectors[NEXUS_AudioConnectorType_eAlternateStereo].port = (size_t)mixerInput;
    BAPE_Decoder_GetConnector(handle->channel, BAPE_ConnectorFormat_eMultichannel, &mixerInput);
    handle->connectors[NEXUS_AudioConnectorType_eMultichannel].port = (size_t)mixerInput;
    BAPE_Decoder_GetConnector(handle->channel, BAPE_ConnectorFormat_eCompressed, &mixerInput);
    handle->connectors[NEXUS_AudioConnectorType_eCompressed].port = (size_t)mixerInput;
    BAPE_Decoder_GetConnector(handle->channel, BAPE_ConnectorFormat_eCompressed4x, &mixerInput);
    handle->connectors[NEXUS_AudioConnectorType_eCompressed4x].port = (size_t)mixerInput;
    BAPE_Decoder_GetConnector(handle->channel, BAPE_ConnectorFormat_eCompressed16x, &mixerInput);
    handle->connectors[NEXUS_AudioConnectorType_eCompressed16x].port = (size_t)mixerInput;
    BAPE_Decoder_GetConnector(handle->channel, BAPE_ConnectorFormat_eMono, &mixerInput);
    handle->connectors[NEXUS_AudioConnectorType_eMono].port = (size_t)mixerInput;

    /* Hook up decoder interrupts */
    BAPE_Decoder_GetInterruptHandlers(handle->channel, &interrupts);
    interrupts.firstPts.pCallback_isr = NEXUS_AudioDecoder_P_FirstPts_isr;
    interrupts.firstPts.pParam1 = handle;
    interrupts.tsmFail.pCallback_isr = NEXUS_AudioDecoder_P_AudioTsmFail_isr;
    interrupts.tsmFail.pParam1 = handle;
    interrupts.tsmPass.pCallback_isr = NEXUS_AudioDecoder_P_AudioTsmPass_isr;
    interrupts.tsmPass.pParam1 = handle;
    interrupts.sampleRateChange.pCallback_isr = NEXUS_AudioDecoder_P_SampleRateChange_isr;
    interrupts.sampleRateChange.pParam1 = handle;
    interrupts.lock.pCallback_isr = NEXUS_AudioDecoder_P_Lock_isr;
    interrupts.lock.pParam1 = handle;
    interrupts.lock.param2 = true;
    interrupts.unlock.pCallback_isr = NEXUS_AudioDecoder_P_Lock_isr;
    interrupts.unlock.pParam1 = handle;
    interrupts.unlock.param2 = false;
    interrupts.statusReady.pCallback_isr = NEXUS_AudioDecoder_P_StreamStatusAvailable_isr;
    interrupts.statusReady.pParam1 = handle;
    interrupts.modeChange.pCallback_isr = NEXUS_AudioDecoder_P_StreamParameterChanged_isr;
    interrupts.modeChange.pParam1 = handle;
    interrupts.cdbItbOverflow.pCallback_isr = NEXUS_AudioDecoder_P_CdbItbOverflow_isr;
    interrupts.cdbItbOverflow.pParam1 = handle;
    interrupts.cdbItbUnderflow.pCallback_isr = NEXUS_AudioDecoder_P_CdbItbUnderflow_isr;
    interrupts.cdbItbUnderflow.pParam1 = handle;
    interrupts.ancillaryData.pCallback_isr = NEXUS_AudioDecoder_P_AncillaryData_isr;
    interrupts.ancillaryData.pParam1 = handle;
#if 0   /* JDG: This causes far too many source changed callbacks to fire.  Some codecs will trigger
                this interrupt on every frame.  It was not functional on older RAP platforms likely
                for the same reason. */
    interrupts.bitrateChange.pCallback_isr = NEXUS_AudioDecoder_P_StreamParameterChanged_isr;
    interrupts.bitrateChange.pParam1 = handle;
#endif
    interrupts.dialnormChange.pCallback_isr = NEXUS_AudioDecoder_P_DialnormChanged_isr;
    interrupts.dialnormChange.pParam1 = handle;
    interrupts.unlicensedAlgo.pCallback_isr = NEXUS_AudioDecoder_P_UnlicensedAlgo_isr;
    interrupts.unlicensedAlgo.pParam1 = handle;
    BAPE_Decoder_SetInterruptHandlers(handle->channel, &interrupts);

    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(&raveSettings);
    UNLOCK_TRANSPORT();

    BAPE_Decoder_GetDefaultCdbItbConfig(handle->channel, &raveSettings.config);

    if ( pSettings->fifoSize == 0 )
    {
        /* NOTE: Don't automatically increase CDB/ITB for IP Settop internally. */
    }
    else
    {
        /* Make ITB proportional to CDB */
        raveSettings.config.Itb.Length = 1024*((raveSettings.config.Itb.Length/1024) * (pSettings->fifoSize/1024))/(raveSettings.config.Cdb.Length/1024);
        BDBG_ASSERT(0 != raveSettings.config.Itb.Length);
        raveSettings.config.Cdb.Length = pSettings->fifoSize;
    }

    #if BAPE_DSP_SUPPORT
    raveSettings.config.Cdb.Alignment = BAPE_ADDRESS_ALIGN_CDB;
    raveSettings.config.Itb.Alignment = BAPE_ADDRESS_ALIGN_ITB;
    #endif

    raveSettings.heap = pSettings->cdbHeap;
    raveSettings.spliceEnabled = pSettings->spliceEnabled;

    LOCK_TRANSPORT();
    handle->raveContext = NEXUS_Rave_Open_priv(&raveSettings);
    UNLOCK_TRANSPORT();
    if ( NULL == handle->raveContext )
    {
        BDBG_ERR(("Unable to allocate RAVE context"));
        goto err_rave;
    }

    #if NEXUS_AUDIO_RAVE_MONITOR_EXT
    {
        NEXUS_AudioRaveMonitorCreateSettings createSettings;

        NEXUS_AudioRaveMonitor_GetDefaultCreateSettings(&createSettings);
        BKNI_Snprintf(createSettings.name, sizeof(createSettings.name), "Rave%u", handle->index);
        BKNI_Snprintf(createSettings.fileName, sizeof(createSettings.fileName), "audioCdb%u.dat", handle->index);
        #if NEXUS_AUDIO_RAVE_MONITOR_CONSUME
        createSettings.consume = true;
        #endif
        handle->raveMonitor.monitor = NEXUS_AudioRaveMonitor_Create(&createSettings);
    }
    #endif

    handle->sourceChangeAppCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->sourceChangeAppCallback )
    {
        goto err_app_callback;
    }

    handle->lockCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->lockCallback )
    {
        goto err_lock_callback;
    }

    handle->ptsErrorCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->ptsErrorCallback )
    {
        goto err_pts_error_callback;
    }

    handle->firstPtsCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->firstPtsCallback )
    {
        goto err_first_pts_callback;
    }

    handle->fifoOverflowCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->fifoOverflowCallback )
    {
        goto err_overflow_callback;
    }

    handle->fifoUnderflowCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->fifoUnderflowCallback )
    {
        goto err_underflow_callback;
    }

    handle->streamStatusCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->streamStatusCallback )
    {
        goto err_status_callback;
    }

    handle->ancillaryDataCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->ancillaryDataCallback )
    {
        goto err_ancillary_callback;
    }

    handle->dialnormChangedCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->dialnormChangedCallback )
    {
        goto err_dial_norm_callback;
    }

    if ( pSettings->type == NEXUS_AudioDecoderType_eDecodeToMemory )
    {
        errCode = NEXUS_AudioDecoder_P_InitDecodeToMemory(handle);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_decode_to_memory;
        }
    }

    /* Success */
    (void)NEXUS_AudioDecoder_SetSettings(handle, NULL);
    g_decoders[handle->index] = handle;

    return handle;

err_decode_to_memory:
    NEXUS_IsrCallback_Destroy(handle->dialnormChangedCallback);
err_dial_norm_callback:
    NEXUS_IsrCallback_Destroy(handle->ancillaryDataCallback);
err_ancillary_callback:
    NEXUS_IsrCallback_Destroy(handle->streamStatusCallback);
err_status_callback:
    NEXUS_IsrCallback_Destroy(handle->fifoUnderflowCallback);
err_underflow_callback:
    NEXUS_IsrCallback_Destroy(handle->fifoOverflowCallback);
err_overflow_callback:
    NEXUS_IsrCallback_Destroy(handle->firstPtsCallback);
err_first_pts_callback:
    NEXUS_IsrCallback_Destroy(handle->ptsErrorCallback);
err_pts_error_callback:
    NEXUS_IsrCallback_Destroy(handle->lockCallback);
err_lock_callback:
    NEXUS_IsrCallback_Destroy(handle->sourceChangeAppCallback);
err_app_callback:
err_rave:
    BAPE_Decoder_Close(handle->channel);
    if ( handle->raveContext )
    {
        LOCK_TRANSPORT();
        NEXUS_Rave_Close_priv(handle->raveContext);
        UNLOCK_TRANSPORT();
    }
err_channel:
    NEXUS_UnregisterEvent(handle->unlicensedAlgoEventHandler);
err_unlicensed_algo_event_handler:
    BKNI_DestroyEvent(handle->unlicensedAlgoEvent);
err_unlicensed_algo_event:
    NEXUS_UnregisterEvent(handle->stc.statusChangeEventHandler);
err_stc_status_change_event_handler:
    BKNI_DestroyEvent(handle->stc.statusChangeEvent);
err_stc_status_change_event:
    NEXUS_UnregisterEvent(handle->inputFormatChangeEventHandler);
err_input_format_change_event_handler:
    BKNI_DestroyEvent(handle->inputFormatChangeEvent);
err_input_format_change_event:
    NEXUS_UnregisterEvent(handle->channelChangeReportEventHandler);
err_channel_change_report_event_handler:
    BKNI_DestroyEvent(handle->channelChangeReportEvent);
err_channel_change_report_event:
    NEXUS_UnregisterEvent(handle->sampleRateCallback);
err_sample_rate_callback:
    BKNI_DestroyEvent(handle->sampleRateEvent);
err_sample_rate_event:
    NEXUS_OBJECT_DESTROY(NEXUS_AudioDecoder, handle);
    BKNI_Free(handle);
    return NULL;
}

/***************************************************************************
Summary:
Close an audio decoder of the specified type
***************************************************************************/
static void NEXUS_AudioDecoder_P_Finalizer(
    NEXUS_AudioDecoderHandle handle
    )
{
    int i;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, handle);

    if ( handle->started )
    {
        NEXUS_AudioDecoder_Stop(handle);
    }

    for ( i = 0; i < NEXUS_AudioConnectorType_eMax; i++ )
    {
        if ( handle->connectors[i].pMixerData )
        {
            const char *pType;
            switch ( i )
            {
            case NEXUS_AudioConnectorType_eStereo:
                pType = "stereo";
                break;
            case NEXUS_AudioConnectorType_eCompressed:
                pType = "compressed";
                break;
            case NEXUS_AudioConnectorType_eCompressed4x:
                pType = "compressed 4x";
                break;
            case NEXUS_AudioConnectorType_eCompressed16x:
                pType = "compressed 16x";
                break;
            case NEXUS_AudioConnectorType_eMultichannel:
                pType = "multichannel";
                break;
            case NEXUS_AudioConnectorType_eMono:
                pType = "mono";
                break;
            case NEXUS_AudioConnectorType_eAlternateStereo:
                pType = "alt stereo";
                break;
            }
            BDBG_MSG(("Decoder Connector %p (type=%s) is still active. Calling NEXUS_AudioInput_Shutdown.", (void *)&handle->connectors[i], pType));
            NEXUS_AudioInput_Shutdown(&handle->connectors[i]);
        }
    }

    if ( handle->openSettings.type == NEXUS_AudioDecoderType_eDecodeToMemory )
    {
        NEXUS_AudioDecoder_P_UninitDecodeToMemory(handle);
    }

    BAPE_Decoder_Close(handle->channel);

    NEXUS_IsrCallback_Destroy(handle->ptsErrorCallback);
    NEXUS_IsrCallback_Destroy(handle->lockCallback);
    NEXUS_IsrCallback_Destroy(handle->sourceChangeAppCallback);
    NEXUS_IsrCallback_Destroy(handle->firstPtsCallback);
    NEXUS_IsrCallback_Destroy(handle->fifoUnderflowCallback);
    NEXUS_IsrCallback_Destroy(handle->fifoOverflowCallback);
    NEXUS_IsrCallback_Destroy(handle->streamStatusCallback);
    NEXUS_IsrCallback_Destroy(handle->ancillaryDataCallback);
    NEXUS_IsrCallback_Destroy(handle->dialnormChangedCallback);
    #if NEXUS_AUDIO_RAVE_MONITOR_EXT
    if ( handle->raveMonitor.monitor )
    {
        NEXUS_AudioRaveMonitor_Destroy(handle->raveMonitor.monitor);
        handle->raveMonitor.monitor = NULL;
    }
    #endif
    LOCK_TRANSPORT();
    if ( handle->raveContext )
    {
        NEXUS_Rave_Close_priv(handle->raveContext);
        handle->raveContext = NULL;
    }
    UNLOCK_TRANSPORT();
    NEXUS_UnregisterEvent(handle->stc.statusChangeEventHandler);
    BKNI_DestroyEvent(handle->stc.statusChangeEvent);
    NEXUS_UnregisterEvent(handle->unlicensedAlgoEventHandler);
    BKNI_DestroyEvent(handle->unlicensedAlgoEvent);
    NEXUS_UnregisterEvent(handle->inputFormatChangeEventHandler);
    BKNI_DestroyEvent(handle->inputFormatChangeEvent);
    NEXUS_UnregisterEvent(handle->channelChangeReportEventHandler);
    BKNI_DestroyEvent(handle->channelChangeReportEvent);
    NEXUS_UnregisterEvent(handle->sampleRateCallback);
    BKNI_DestroyEvent(handle->sampleRateEvent);
    if(handle->spliceCallback) {
        NEXUS_IsrCallback_Destroy(handle->spliceCallback);
        handle->spliceCallback = NULL;
    }
    g_decoders[handle->index] = NULL;
    NEXUS_OBJECT_DESTROY(NEXUS_AudioDecoder, handle);
    BKNI_Free(handle);
}

static void NEXUS_AudioDecoder_P_Release(NEXUS_AudioDecoderHandle handle)
{
    unsigned i;
    for ( i = 0; i < NEXUS_AudioConnectorType_eMax; i++ )
    {
        NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connectors[i], Close);
    }
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioDecoder, NEXUS_AudioDecoder_Close);

/***************************************************************************
Summary:
Get Settings for an audio decoder
***************************************************************************/
void NEXUS_AudioDecoder_GetSettings(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderSettings *pSettings   /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
Set Settings for an audio decoder
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_SetSettings(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderSettings *pSettings /* Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    if ( NULL == pSettings )
    {
        NEXUS_AudioDecoder_P_GetDefaultSettings(&handle->settings);
    }
    else
    {
        if ( pSettings->fifoThreshold != handle->settings.fifoThreshold )
        {
            BERR_Code rc;

            LOCK_TRANSPORT();
            rc = NEXUS_Rave_SetCdbThreshold_priv(handle->raveContext, pSettings->fifoThreshold);
            UNLOCK_TRANSPORT();
            if (rc) return BERR_TRACE(rc);
        }

        handle->settings = *pSettings;
    }
    NEXUS_IsrCallback_Set(handle->sourceChangeAppCallback, &(handle->settings.sourceChanged));
    NEXUS_IsrCallback_Set(handle->lockCallback, &(handle->settings.lockChanged));
    NEXUS_IsrCallback_Set(handle->ptsErrorCallback, &(handle->settings.ptsError));
    NEXUS_IsrCallback_Set(handle->firstPtsCallback, &(handle->settings.firstPts));
    NEXUS_IsrCallback_Set(handle->fifoUnderflowCallback, &(handle->settings.fifoUnderflow));
    NEXUS_IsrCallback_Set(handle->fifoOverflowCallback, &(handle->settings.fifoOverflow));
    NEXUS_IsrCallback_Set(handle->streamStatusCallback, &(handle->settings.streamStatusAvailable));
    NEXUS_IsrCallback_Set(handle->ancillaryDataCallback, &(handle->settings.ancillaryData));
    NEXUS_IsrCallback_Set(handle->dialnormChangedCallback, &(handle->settings.dialnormChanged));
    if (handle->running)
    {
        return NEXUS_AudioDecoder_ApplySettings_priv(handle);
    }
    else
    {
        return NEXUS_SUCCESS;
    }
}

void NEXUS_AudioDecoder_Clear_priv( NEXUS_AudioDecoderHandle handle )
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    NEXUS_IsrCallback_Clear(handle->sourceChangeAppCallback);
    NEXUS_IsrCallback_Clear(handle->lockCallback);
    NEXUS_IsrCallback_Clear(handle->ptsErrorCallback);
    NEXUS_IsrCallback_Clear(handle->firstPtsCallback);
    NEXUS_IsrCallback_Clear(handle->fifoUnderflowCallback);
    NEXUS_IsrCallback_Clear(handle->fifoOverflowCallback);
    NEXUS_IsrCallback_Clear(handle->streamStatusCallback);
    NEXUS_IsrCallback_Clear(handle->ancillaryDataCallback);
    NEXUS_IsrCallback_Clear(handle->dialnormChangedCallback);
}

/***************************************************************************
Summary:
Initialize an audio decoder program structure
***************************************************************************/
void NEXUS_AudioDecoder_GetDefaultStartSettings(
    NEXUS_AudioDecoderStartSettings *pSettings /* [out] Program Defaults */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioDecoderStartSettings));
    pSettings->codec = NEXUS_AudioCodec_eAc3;
    pSettings->targetSyncEnabled = NEXUS_AudioDecoderTargetSyncMode_eEnabled;
    pSettings->maxOutputRate = 48000;

    BDBG_CASSERT((int)NEXUS_AudioDecoderTargetSyncMode_eDisabledAfterLock == (int)BAPE_DecoderTargetSyncMode_eDisabledAfterLock);
    BDBG_CASSERT((int)NEXUS_AudioDecoderTargetSyncMode_eEnabled == (int)BAPE_DecoderTargetSyncMode_eEnabled);
    BDBG_CASSERT((int)NEXUS_AudioDecoderTargetSyncMode_eDisabled == (int)BAPE_DecoderTargetSyncMode_eDisabled);
    BDBG_CASSERT((int)NEXUS_AudioDecoderTargetSyncMode_eMax == (int)BAPE_DecoderTargetSyncMode_eMax);
}

#if BDBG_DEBUG_BUILD
static void NEXUS_AudioDecoder_P_PrintRaveStatus(NEXUS_RaveHandle cxt, char * header)
{
    BERR_Code errCode;
    NEXUS_RaveStatus raveStatus;

    LOCK_TRANSPORT();
    errCode = NEXUS_Rave_GetStatus_priv(cxt, &raveStatus);
    UNLOCK_TRANSPORT();
    if (errCode)
    {
        errCode = BERR_TRACE(errCode);
        return;
    }

    BDBG_MSG(("Rave Context \"%s\":", header?header:"null"));
    BDBG_MSG(("  CDB base %x", raveStatus.xptContextMap.CDB_Base));
    BDBG_MSG(("  CDB end %x", raveStatus.xptContextMap.CDB_End));
    BDBG_MSG(("  CDB read %x", raveStatus.xptContextMap.CDB_Read));
    BDBG_MSG(("  CDB valid %x", raveStatus.xptContextMap.CDB_Valid));
    BDBG_MSG(("  CDB wrap %x", raveStatus.xptContextMap.CDB_Wrap));
    BDBG_MSG(("  ITB base %x", raveStatus.xptContextMap.ITB_Base));
    BDBG_MSG(("  ITB end %x", raveStatus.xptContextMap.ITB_End));
    BDBG_MSG(("  ITB read %x", raveStatus.xptContextMap.ITB_Read));
    BDBG_MSG(("  ITB valid %x", raveStatus.xptContextMap.ITB_Valid));
    BDBG_MSG(("  ITB wrap %x", raveStatus.xptContextMap.ITB_Wrap));
}
#endif

/***************************************************************************
Summary:
Start deocding the specified program
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Start(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderStartSettings *pProgram    /* What to start decoding */
    )
{
    NEXUS_Error errCode;
    bool useTsm;
    NEXUS_PidChannelStatus pidChannelStatus;
    BAVC_StreamType streamType=BAVC_StreamType_eTsMpeg;
    BAPE_DecoderStartSettings *pStartSettings;
    BAPE_DecoderTsmSettings tsmSettings;


    BDBG_ENTER(NEXUS_AudioDecoder_Start);

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pProgram);

    BDBG_MODULE_MSG(nexus_flow_audio_decoder, ("start %p, pidChannel %p, stcChannel %p, codec %d",
        (void *)handle, (void *)pProgram->pidChannel, (void *)pProgram->stcChannel, pProgram->codec));

    if ( handle->started )
    {
        BDBG_ERR(("This decoder is already started.  Please call stop first."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->suspended )
    {
        BDBG_ERR(("This decoder is suspended.  Please call stop first."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pStartSettings = &handle->apeStartSettings;
    BAPE_Decoder_GetDefaultStartSettings(pStartSettings);
    pStartSettings->targetSyncEnabled = (BAPE_DecoderTargetSyncMode)pProgram->targetSyncEnabled;
    pStartSettings->forceCompleteFirstFrame = pProgram->forceCompleteFirstFrame;
    pStartSettings->nonRealTime = pProgram->nonRealTime;
    pStartSettings->karaokeModeEnabled = pProgram->karaokeModeEnabled;
    switch ( pProgram->mixingMode )
    {
    default:
        BDBG_ERR(("Invalid mixing mode"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    case NEXUS_AudioDecoderMixingMode_eDescription:
        pStartSettings->mixingMode = BAPE_DecoderMixingMode_eDescription;
        break;
    case NEXUS_AudioDecoderMixingMode_eSoundEffects:
        pStartSettings->mixingMode = BAPE_DecoderMixingMode_eSoundEffects;
        break;
    case NEXUS_AudioDecoderMixingMode_eApplicationAudio:
        pStartSettings->mixingMode = BAPE_DecoderMixingMode_eApplicationAudio;
        break;
    case NEXUS_AudioDecoderMixingMode_eStandalone:
        pStartSettings->mixingMode = BAPE_DecoderMixingMode_eStandalone;
        break;
    }
    switch ( pProgram->maxOutputRate )
    {
    default:
        BDBG_ERR(("Invalid max output rate"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    case 48000:
    case 96000:
        pStartSettings->maxOutputRate = pProgram->maxOutputRate;
        break;
    }

    NEXUS_AudioDecoder_P_TrickReset(handle); /* reset trick state on start */

    /* Save Program */
    handle->programSettings = *pProgram;

    /* Sanity check program */
    if ( pProgram->pidChannel )
    {
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_PidChannel, pProgram->pidChannel);

        /* Transport source */
        errCode = NEXUS_PidChannel_GetStatus(pProgram->pidChannel, &pidChannelStatus);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
        }

        /* There are 4 types of DSS A/V streams:
        DSS SD video - this is DSS ES
        DSS HD video - this is DSS PES
        DSS MPEG audio - this actually uses MPEG1 system headers, but it's very similar to PES, therefore DSS PES
            Therefore we convert DSS ES to DSS PES here.
        DSS AC3 audio - uses MPEG2 System PES, therefore DSS PES
        */
        if (pidChannelStatus.transportType == NEXUS_TransportType_eDssEs)
        {
            pidChannelStatus.transportType = NEXUS_TransportType_eDssPes;
        }
        /* All DSS is converted to PES above */
        handle->isDss = (pidChannelStatus.transportType == NEXUS_TransportType_eDssPes)?true:false;

        /* Convert to AVC transport type */
        errCode = NEXUS_P_TransportType_ToMagnum_isrsafe(pidChannelStatus.transportType, &streamType);
        if (errCode)
        {
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto err_stream_type;
        }

        switch ( pProgram->latencyMode )
        {
        case NEXUS_AudioDecoderLatencyMode_eLowest:
            pStartSettings->delayMode = BAPE_DspDelayMode_eLowVariable;
            break;
        case NEXUS_AudioDecoderLatencyMode_eLow:
            pStartSettings->delayMode = BAPE_DspDelayMode_eLowFixed;
            break;
        default:
        case NEXUS_AudioDecoderLatencyMode_eAuto:
        case NEXUS_AudioDecoderLatencyMode_eNormal:
            pStartSettings->delayMode = BAPE_DspDelayMode_eDefault;
            break;
        }
    }
    else if ( pProgram->input )
    {
        streamType = BAVC_StreamType_ePes;
        handle->isDss = false;
        pStartSettings->inputPort = NEXUS_AudioInput_P_GetInputPort(pProgram->input);
        switch ( pProgram->latencyMode )
        {
        default:
        case NEXUS_AudioDecoderLatencyMode_eAuto:
        case NEXUS_AudioDecoderLatencyMode_eLow:
            pStartSettings->delayMode = BAPE_DspDelayMode_eLowFixed;
            break;
        case NEXUS_AudioDecoderLatencyMode_eNormal:
            pStartSettings->delayMode = BAPE_DspDelayMode_eDefault;
            break;
        case NEXUS_AudioDecoderLatencyMode_eLowest:
            pStartSettings->delayMode = BAPE_DspDelayMode_eLowVariable;
            break;
        }
        if ( NULL == pStartSettings->inputPort )
        {
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto err_input_port;
        }
        BKNI_Memset(&pidChannelStatus, 0, sizeof(NEXUS_PidChannelStatus));
    }
    else
    {
        BDBG_ERR(("No PID Channel or Input provided"));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_start_settings;
    }

    pStartSettings->streamType = streamType;
    pStartSettings->codec = NEXUS_Audio_P_CodecToMagnum(pProgram->codec);

    handle->isPlayback = false;

    if (pProgram->stcChannel)
    {
        NEXUS_StcChannelSettings stcSettings;
        LOCK_TRANSPORT();
        NEXUS_StcChannel_GetIndex_priv(pProgram->stcChannel, &pStartSettings->stcIndex);
        UNLOCK_TRANSPORT();
        NEXUS_StcChannel_GetSettings(pProgram->stcChannel, &stcSettings);
        handle->isPlayback = (stcSettings.mode != NEXUS_StcChannelMode_ePcr && NULL == pProgram->input);
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_StcChannel, pProgram->stcChannel);

        /*
         * if we are not coming from XPT input, and we have valid stc channel
         * set the stc index to pull PTS for compressed audio
         */
        if (pProgram->input)
        {
            errCode = NEXUS_AudioInput_P_SetStcIndex(pProgram->input, pStartSettings->stcIndex);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_start;
            }
        }
    }

    /* Compute TSM details */
    BAPE_Decoder_GetTsmSettings(handle->channel, &tsmSettings);
    /* this should depend on stc channel if present or be false */
    tsmSettings.playback = handle->isPlayback;
    tsmSettings.ptsOffset = handle->settings.ptsOffset;
#if NEXUS_HAS_SYNC_CHANNEL
    tsmSettings.ptsOffset += handle->sync.settings.delay;
#endif
    /* this requires only stc channel is playback mode */
    /* 20140605 bandrews - update to "infinity", which is max signed int in 45 KHz ticks converted to ms */
    tsmSettings.thresholds.discard = NEXUS_AUDIO_DECODER_P_GET_DISCARD_THRESHOLD(tsmSettings.playback, handle->isDss);
    tsmSettings.thresholds.grossAdjustment = (pidChannelStatus.originalTransportType == NEXUS_TransportType_eAvi) ? 0x30 : 0x8;
    BAPE_Decoder_SetTsmSettings(handle->channel, &tsmSettings);

    /* Determine TSM mode */
    if ( NULL == pProgram->stcChannel ||
         NULL != NEXUS_GetEnv("force_vsync") ||
         handle->openSettings.type == NEXUS_AudioDecoderType_eDecodeToMemory ||
         (pProgram->pidChannel && (pidChannelStatus.transportType == NEXUS_TransportType_eEs ||
                                   pidChannelStatus.originalTransportType == NEXUS_TransportType_eWav ||
                                   pidChannelStatus.originalTransportType == NEXUS_TransportType_eAiff ||
                                   #if 1 /* SW7425-2000: OGG PTS values are invalid from bmedia currently */
                                   pidChannelStatus.originalTransportType == NEXUS_TransportType_eOgg ||
                                   #endif
                                   pidChannelStatus.originalTransportType == NEXUS_TransportType_eEs)) )
    {
        useTsm = false;
    }
    else
    {
        useTsm = true;
    }
    handle->tsmPermitted = useTsm;
    NEXUS_AudioDecoder_P_SetTsm(handle);

    if ( pProgram->pidChannel )
    {
        BDBG_MSG(("Configure RAVE"));
        errCode = NEXUS_AudioDecoder_P_ConfigureRave(handle->raveContext, pProgram, &pidChannelStatus, handle->isPlayback);
        if (errCode)
        {
            errCode = BERR_TRACE(errCode);
            goto err_configure_audio;
        }
        handle->raveCxtConfigured = true;
        LOCK_TRANSPORT();
        errCode = NEXUS_Rave_GetStatus_priv(handle->raveContext, &handle->raveStatus);
        UNLOCK_TRANSPORT();
        if (errCode)
        {
            errCode = BERR_TRACE(errCode);
            goto err_rave_status;
        }
        pStartSettings->pContextMap = &handle->raveStatus.xptContextMap;

        #if BDBG_DEBUG_BUILD
        NEXUS_AudioDecoder_P_PrintRaveStatus(handle->raveContext, "Audio");
        #endif
    }

    BDBG_MSG(("%s input", (pProgram->input != NULL) ? "HAS" : "DOES NOT HAVE"));
    if ( pProgram->input )
    {
        BDBG_MSG(("Supports Format Change %d", NEXUS_AudioInput_P_SupportsFormatChanges(pProgram->input)));
        if ( NEXUS_AudioInput_P_SupportsFormatChanges(pProgram->input) )
        {
            /* If this input supports dynamic format changes, enable the dynamic format change interrupt and kick off the format state machine. */
            errCode = NEXUS_AudioInput_P_SetFormatChangeInterrupt(pProgram->input, NEXUS_AudioInputType_eDecoder, NEXUS_AudioDecoder_P_InputFormatChange_isr, handle, 0);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_start;
            }
            handle->started = true;
            if ( handle->programSettings.inputFormatChangeMode == NEXUS_AudioInputFormatChangeMode_eManual && handle->programSettings.codec != NEXUS_AudioCodec_eMax )
            {
                BDBG_MSG(("  Start Decoder"));
                BERR_TRACE(NEXUS_AudioDecoder_P_Start(handle));
            }
            else
            {
                BDBG_MSG(("  Force first Format Change"));
                NEXUS_AudioDecoder_P_InputFormatChange(handle);
            }
            return BERR_SUCCESS;
        }
    }

    /* We're ready to start.  Build up lists of outputs to check for configuration changes. */
    errCode = NEXUS_AudioDecoder_P_Start(handle);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_start;
    }

    if(NEXUS_GetEnv("verify_timebase"))
        NEXUS_AudioDecoder_P_VerifyTimebase(handle);

    handle->started = true;

    BDBG_LEAVE(NEXUS_AudioDecoder_Start);

    /* Success */
    return BERR_SUCCESS;

err_start:
err_rave_status:
    if ( pProgram->pidChannel )
    {
        LOCK_TRANSPORT();
        if ( handle->raveCxtConfigured )
        {
            NEXUS_Rave_RemovePidChannel_priv(handle->raveContext);
        }
        handle->raveCxtConfigured = false;
        UNLOCK_TRANSPORT();
    }

err_configure_audio:
    if ( handle->programSettings.stcChannel )
    {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_StcChannel, handle->programSettings.stcChannel);
    }
err_start_settings:
err_input_port:
err_stream_type:
    if ( handle->programSettings.pidChannel )
    {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, handle->programSettings.pidChannel);
    }
    BKNI_Memset(&handle->programSettings, 0, sizeof(NEXUS_AudioDecoderStartSettings));

    return errCode;
}

/***************************************************************************
Summary:
Stop deocding the current program
***************************************************************************/
void NEXUS_AudioDecoder_Stop(
    NEXUS_AudioDecoderHandle handle
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    BDBG_ENTER(NEXUS_AudioDecoder_Stop);

    BDBG_MODULE_MSG(nexus_flow_audio_decoder, ("stop %p", (void *)handle));
    if ( !handle->started && !handle->suspended )
    {
        BDBG_ERR(("Decoder not started or suspended"));
        return;
    }

    if ( !handle->suspended )
    {
        if ( handle->programSettings.input )
        {
            if ( NEXUS_AudioInput_P_SupportsFormatChanges(handle->programSettings.input) )
            {
                /* If this input supports dynamic format changes, disable the dynamic format change interrupt. */
                (void)NEXUS_AudioInput_P_SetFormatChangeInterrupt(handle->programSettings.input, NEXUS_AudioInputType_eDecoder, NULL, handle, 0);
            }
        }
    }

    errCode = NEXUS_AudioDecoder_P_Stop(handle, true);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }

    if (handle->primer) {
        /* decode will stop and primer will not restart. just disconnect. */
        NEXUS_AudioDecoderPrimer_P_DecodeStopped(handle->primer);
    }

    handle->running = false;
    handle->started = false;
    handle->suspended = false;
    handle->trickForceStopped = false; /* do we need to forcedly unmute on Stop, in a way it helps if in a PIP change mode decoder is moved from trickmode on one channel to normal mode on another channel, however it hurts if one stops decoder just in order to change a PID/ audio program */

    if ( handle->programSettings.stcChannel )
    {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_StcChannel, handle->programSettings.stcChannel);
    }
    if ( handle->programSettings.pidChannel )
    {
        LOCK_TRANSPORT();
        if ( handle->raveCxtConfigured )
        {
            BDBG_MSG(("Remove Pid Channel from RAVE"));
            NEXUS_Rave_RemovePidChannel_priv(handle->raveContext);
        }
        handle->raveCxtConfigured = false;
        UNLOCK_TRANSPORT();
        NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, handle->programSettings.pidChannel);
    }

    BKNI_Memset(&handle->programSettings, 0, sizeof(handle->programSettings));

    BDBG_LEAVE(NEXUS_AudioDecoder_Stop);
}

/***************************************************************************
Summary:
Stop decoding the current program without flushing
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Suspend(
    NEXUS_AudioDecoderHandle handle
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    BDBG_ENTER(NEXUS_AudioDecoder_Suspend);

    BDBG_MODULE_MSG(nexus_flow_audio_decoder, ("suspend %p", (void *)handle));

    if ( !handle->started )
    {
        BDBG_ERR(("Decoder not started"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->programSettings.input )
    {
        if ( NEXUS_AudioInput_P_SupportsFormatChanges(handle->programSettings.input) )
        {
            /* If this input supports dynamic format changes, disable the dynamic format change interrupt. */
            (void)NEXUS_AudioInput_P_SetFormatChangeInterrupt(handle->programSettings.input, NEXUS_AudioInputType_eDecoder, NULL, handle, 0);
        }
    }

    errCode = NEXUS_AudioDecoder_P_Stop(handle, false);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }

    handle->running = false;
    handle->started = false;
    handle->suspended = true;
    handle->trickForceStopped = false; /* do we need to forcedly unmute on Stop, in a way it helps if in a PIP change mode decoder is moved from trickmode on one channel to normal mode on another channel, however it hurts if one stops decoder just in order to change a PID/ audio program */

    BDBG_LEAVE(NEXUS_AudioDecoder_Suspend);
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Resumes from the suspended state
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Resume(
    NEXUS_AudioDecoderHandle handle
    )
{
    NEXUS_Error errCode;

    BDBG_ENTER(NEXUS_AudioDecoder_Resume);

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    BDBG_MODULE_MSG(nexus_flow_audio_decoder, ("start %p, pidChannel %p, stcChannel %p, codec %d",
        (void *)handle, (void *)handle->programSettings.pidChannel, (void *)handle->programSettings.stcChannel, handle->programSettings.codec));

    if ( handle->started )
    {
        BDBG_ERR(("This decoder is already started.  Nothing to resume."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( !handle->suspended )
    {
        BDBG_ERR(("This decoder not suspended.  Nothing to resume."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (handle->programSettings.stcChannel)
    {
        unsigned stcIndex;
        LOCK_TRANSPORT();
        NEXUS_StcChannel_GetIndex_priv(handle->programSettings.stcChannel, &stcIndex);
        UNLOCK_TRANSPORT();

        if (handle->programSettings.input)
        {
            /*
             * if we are not coming from XPT input, and we have valid stc channel
             * set the stc index to pull PTS for compressed audio
             */
            errCode = NEXUS_AudioInput_P_SetStcIndex(handle->programSettings.input, stcIndex);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_start;
            }
        }
    }

    if ( handle->programSettings.input )
    {
        if ( NEXUS_AudioInput_P_SupportsFormatChanges(handle->programSettings.input) )
        {
            /* If this input supports dynamic format changes, enable the dynamic format change interrupt and kick off the format state machine. */
            errCode = NEXUS_AudioInput_P_SetFormatChangeInterrupt(handle->programSettings.input, NEXUS_AudioInputType_eDecoder, NEXUS_AudioDecoder_P_InputFormatChange_isr, handle, 0);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_start;
            }
            handle->started = true;
            if ( handle->programSettings.inputFormatChangeMode == NEXUS_AudioInputFormatChangeMode_eManual && handle->programSettings.codec != NEXUS_AudioCodec_eMax )
            {
                BDBG_MSG(("  Start Decoder"));
                BERR_TRACE(NEXUS_AudioDecoder_P_Start(handle));
            }
            else
            {
                BDBG_MSG(("  Force first Format Change"));
                NEXUS_AudioDecoder_P_InputFormatChange(handle);
            }
            return BERR_SUCCESS;
        }
    }

    /* We're ready to start.  Build up lists of outputs to check for configuration changes. */
    errCode = NEXUS_AudioDecoder_P_Start(handle);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_start;
    }

    handle->started = true;
    handle->suspended = false;

    BDBG_LEAVE(NEXUS_AudioDecoder_Resume);

    /* Success */
    return BERR_SUCCESS;

err_start:
    if ( handle->programSettings.input )
    {
        if ( NEXUS_AudioInput_P_SupportsFormatChanges(handle->programSettings.input) )
        {
            /* If this input supports dynamic format changes, disable the dynamic format change interrupt. */
            (void)NEXUS_AudioInput_P_SetFormatChangeInterrupt(handle->programSettings.input, NEXUS_AudioInputType_eDecoder, NULL, handle, 0);
        }
    }
    return errCode;
}

/***************************************************************************
Summary:
Discards all data accumulated in the decoder buffer
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Flush(
    NEXUS_AudioDecoderHandle handle
    )
{
    BERR_Code rc = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    if ( !handle->started || !handle->running )
    {
        return BERR_SUCCESS;
    }

    BAPE_Decoder_DisableForFlush(handle->channel);

    BDBG_ASSERT(handle->raveContext);

    if ( handle->programSettings.pidChannel )
    {
        NEXUS_AudioDecoder_P_DisableRaveContexts(handle, true);
    }

    rc = BAPE_Decoder_Flush(handle->channel);
    if ( rc )
    {
        BDBG_ERR(("WARNING: Unable to Audio Decoder."));
        (void)BERR_TRACE(rc);
    }

    if ( handle->openSettings.type == NEXUS_AudioDecoderType_eDecodeToMemory )
    {
        NEXUS_AudioDecoder_P_FlushDecodeToMemory(handle);
    }

    if ( handle->programSettings.pidChannel )
    {
        rc = NEXUS_AudioDecoder_P_EnableRaveContexts(handle, NULL);
        if ( rc )
        {
            BDBG_ERR(("ERROR: Unable to re-start Audio context."));
            return BERR_TRACE(rc);
        }
    }

    return rc;
}

/***************************************************************************
Summary:
    Get codec-specific decoder settings
***************************************************************************/
void NEXUS_AudioDecoder_GetCodecSettings(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioCodec codec,
    NEXUS_AudioDecoderCodecSettings *pSettings  /* [out] settings for specified codec */
    )
{
    BAPE_DecoderCodecSettings codecSettings;
    BAVC_AudioCompressionStd avcCodec;
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSettings);

    avcCodec = NEXUS_Audio_P_CodecToMagnum(codec);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->codec = codec;

    BAPE_Decoder_GetCodecSettings(handle->channel, avcCodec, &codecSettings);
    switch ( codec )
    {
    case NEXUS_AudioCodec_eAc3:
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eLine == (NEXUS_AudioDecoderDolbyDrcMode)BAPE_Ac3DrcMode_eLine);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eRf == (NEXUS_AudioDecoderDolbyDrcMode)BAPE_Ac3DrcMode_eRf);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eCustomA == (NEXUS_AudioDecoderDolbyDrcMode)BAPE_Ac3DrcMode_eCustomA);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eCustomD == (NEXUS_AudioDecoderDolbyDrcMode)BAPE_Ac3DrcMode_eCustomD);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyDrcMode_eCustomTarget == (NEXUS_AudioDecoderDolbyDrcMode)BAPE_Ac3DrcMode_eCustomTarget);
        pSettings->codecSettings.ac3.drcMode = codecSettings.codecSettings.ac3.drcMode;
        pSettings->codecSettings.ac3.drcModeDownmix = codecSettings.codecSettings.ac3.drcModeDownmix;
        pSettings->codecSettings.ac3.customTargetLevel = codecSettings.codecSettings.ac3.customTargetLevel;
        pSettings->codecSettings.ac3.customTargetLevelDownmix = codecSettings.codecSettings.ac3.customTargetLevelDownmix;
        pSettings->codecSettings.ac3.cut = codecSettings.codecSettings.ac3.drcScaleHi;
        pSettings->codecSettings.ac3.boost = codecSettings.codecSettings.ac3.drcScaleLow;
        pSettings->codecSettings.ac3.cutDownmix = codecSettings.codecSettings.ac3.drcScaleHiDownmix;
        pSettings->codecSettings.ac3.boostDownmix = codecSettings.codecSettings.ac3.drcScaleLowDownmix;
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic == (NEXUS_AudioDecoderDolbyStereoDownmixMode)BAPE_Ac3StereoMode_eAuto);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible == (NEXUS_AudioDecoderDolbyStereoDownmixMode)BAPE_Ac3StereoMode_eLtRt);
        BDBG_CASSERT(NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard == (NEXUS_AudioDecoderDolbyStereoDownmixMode)BAPE_Ac3StereoMode_eLoRo);
        pSettings->codecSettings.ac3.stereoDownmixMode = codecSettings.codecSettings.ac3.stereoMode;
        pSettings->codecSettings.ac3.scale = codecSettings.codecSettings.ac3.scale;
        pSettings->codecSettings.ac3.scaleDownmix = codecSettings.codecSettings.ac3.scaleDownmix;
        pSettings->codecSettings.ac3.dialogNormalization = codecSettings.codecSettings.ac3.dialogNormalization;
        pSettings->codecSettings.ac3.dialogNormalizationValue = codecSettings.codecSettings.ac3.dialogNormalizationValue;
        pSettings->codecSettings.ac3.substreamId = codecSettings.codecSettings.ac3.substreamId;
        pSettings->codecSettings.ac3.enableAtmosProcessing = codecSettings.codecSettings.ac3.enableAtmosProcessing;
        pSettings->codecSettings.ac3.certificationMode = codecSettings.codecSettings.ac3.certificationMode;
        break;
    case NEXUS_AudioCodec_eAc3Plus:
        pSettings->codecSettings.ac3Plus.drcMode = codecSettings.codecSettings.ac3Plus.drcMode;
        pSettings->codecSettings.ac3Plus.drcModeDownmix = codecSettings.codecSettings.ac3Plus.drcModeDownmix;
        pSettings->codecSettings.ac3Plus.customTargetLevel = codecSettings.codecSettings.ac3Plus.customTargetLevel;
        pSettings->codecSettings.ac3Plus.customTargetLevelDownmix = codecSettings.codecSettings.ac3Plus.customTargetLevelDownmix;
        pSettings->codecSettings.ac3Plus.cut = codecSettings.codecSettings.ac3Plus.drcScaleHi;
        pSettings->codecSettings.ac3Plus.boost = codecSettings.codecSettings.ac3Plus.drcScaleLow;
        pSettings->codecSettings.ac3Plus.cutDownmix = codecSettings.codecSettings.ac3Plus.drcScaleHiDownmix;
        pSettings->codecSettings.ac3Plus.boostDownmix = codecSettings.codecSettings.ac3Plus.drcScaleLowDownmix;
        pSettings->codecSettings.ac3Plus.stereoDownmixMode = codecSettings.codecSettings.ac3Plus.stereoMode;
        pSettings->codecSettings.ac3Plus.scale = codecSettings.codecSettings.ac3Plus.scale;
        pSettings->codecSettings.ac3Plus.scaleDownmix = codecSettings.codecSettings.ac3Plus.scaleDownmix;
        pSettings->codecSettings.ac3Plus.dialogNormalization = codecSettings.codecSettings.ac3Plus.dialogNormalization;
        pSettings->codecSettings.ac3Plus.dialogNormalizationValue = codecSettings.codecSettings.ac3Plus.dialogNormalizationValue;
        pSettings->codecSettings.ac3Plus.substreamId = codecSettings.codecSettings.ac3Plus.substreamId;
        pSettings->codecSettings.ac3Plus.enableAtmosProcessing = codecSettings.codecSettings.ac3Plus.enableAtmosProcessing;
        pSettings->codecSettings.ac3Plus.certificationMode = codecSettings.codecSettings.ac3Plus.certificationMode;
        break;
    case NEXUS_AudioCodec_eAc4:
        switch ( codecSettings.codecSettings.ac4.drcMode )
        {
        default:
        case BAPE_DolbyDrcMode_eLine:
            pSettings->codecSettings.ac4.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            break;
        case BAPE_DolbyDrcMode_eRf:
            pSettings->codecSettings.ac4.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            break;
        case BAPE_DolbyDrcMode_eDisabled:
            pSettings->codecSettings.ac4.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eOff;
            break;
        }
        switch ( codecSettings.codecSettings.ac4.drcModeDownmix )
        {
        default:
        case BAPE_DolbyDrcMode_eRf:
            pSettings->codecSettings.ac4.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            break;
        case BAPE_DolbyDrcMode_eLine:
            pSettings->codecSettings.ac4.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            break;
        case BAPE_DolbyDrcMode_eDisabled:
            pSettings->codecSettings.ac4.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eOff;
            break;
        }
        pSettings->codecSettings.ac4.drcScaleHi = codecSettings.codecSettings.ac4.drcScaleHi;
        pSettings->codecSettings.ac4.drcScaleLow = codecSettings.codecSettings.ac4.drcScaleLow;
        pSettings->codecSettings.ac4.drcScaleHiDownmix = codecSettings.codecSettings.ac4.drcScaleHiDownmix;
        pSettings->codecSettings.ac4.drcScaleLowDownmix = codecSettings.codecSettings.ac4.drcScaleLowDownmix;
        switch ( codecSettings.codecSettings.ac4.stereoMode )
        {
        default:
        case BAPE_DolbyStereoMode_eLoRo:
            pSettings->codecSettings.ac4.stereoMode = NEXUS_AudioDecoderStereoDownmixMode_eLoRo;
            break;
        case BAPE_DolbyStereoMode_eLtRt:
            pSettings->codecSettings.ac4.stereoMode = NEXUS_AudioDecoderStereoDownmixMode_eLtRt;
            break;
        }
        pSettings->codecSettings.ac4.selectionMode = (NEXUS_AudioDecoderAc4PresentationSelectionMode)codecSettings.codecSettings.ac4.selectionMode;
        pSettings->codecSettings.ac4.programSelection = codecSettings.codecSettings.ac4.programSelection;
        pSettings->codecSettings.ac4.programBalance = codecSettings.codecSettings.ac4.programBalance;
        pSettings->codecSettings.ac4.dialogEnhancerAmount = codecSettings.codecSettings.ac4.dialogEnhancerAmount;
        pSettings->codecSettings.ac4.certificationMode = codecSettings.codecSettings.ac4.certificationMode;
        pSettings->codecSettings.ac4.enableAssociateMixing = codecSettings.codecSettings.ac4.enableAssociateMixing;
        /* personalization settings */
        BDBG_CASSERT(NEXUS_AUDIO_AC4_PRESENTATION_ID_LENGTH == BAPE_AC4_PRESENTATION_ID_LENGTH);
        BKNI_Memcpy(pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationId, codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eMain].presentationId, sizeof(char) * NEXUS_AUDIO_AC4_PRESENTATION_ID_LENGTH);
        BKNI_Memcpy(pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].presentationId, codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eAlternate].presentationId, sizeof(char) * NEXUS_AUDIO_AC4_PRESENTATION_ID_LENGTH);
        pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationIndex = codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eMain].presentationIndex;
        pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].presentationIndex = codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eAlternate].presentationIndex;
        pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferLanguageOverAssociateType = codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eMain].preferLanguageOverAssociateType;
        pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferLanguageOverAssociateType = codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eAlternate].preferLanguageOverAssociateType;
        pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferredAssociateType = (NEXUS_AudioAc4AssociateType)codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eMain].preferredAssociateType;
        pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferredAssociateType = (NEXUS_AudioAc4AssociateType)codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eAlternate].preferredAssociateType;
        BDBG_CASSERT(NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH == BAPE_AC4_LANGUAGE_NAME_LENGTH);
        BDBG_CASSERT(NEXUS_AUDIO_AC4_NUM_LANGUAGES == BAPE_AC4_NUM_LANGUAGES);
        for ( i = 0; i < NEXUS_AUDIO_AC4_NUM_LANGUAGES; i++ )
        {
            BKNI_Memcpy(pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[i].selection, codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eMain].languagePreference[i].selection, sizeof(char) * NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
            BKNI_Memcpy(pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[i].selection, codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eAlternate].languagePreference[i].selection, sizeof(char) * NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
        }
        break;
    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacLoas:
        pSettings->codecSettings.aac.cut = codecSettings.codecSettings.aac.drcScaleHi;
        pSettings->codecSettings.aac.boost = codecSettings.codecSettings.aac.drcScaleLow;
        pSettings->codecSettings.aac.drcTargetLevel = codecSettings.codecSettings.aac.drcReferenceLevel;
        BDBG_CASSERT((int)BAPE_AacStereoMode_eMatrix == (int)NEXUS_AudioDecoderAacDownmixMode_eMatrix);
        BDBG_CASSERT((int)BAPE_AacStereoMode_eArib == (int)NEXUS_AudioDecoderAacDownmixMode_eArib);
        BDBG_CASSERT((int)BAPE_AacStereoMode_eLtRt == (int)NEXUS_AudioDecoderAacDownmixMode_eLtRt);
        BDBG_CASSERT((int)BAPE_AacStereoMode_eLoRo == (int)NEXUS_AudioDecoderAacDownmixMode_eLoRo);
        pSettings->codecSettings.aac.downmixMode = codecSettings.codecSettings.aac.downmixMode;
        BDBG_CASSERT((int)NEXUS_AudioDecoderDolbyPulseDrcMode_eLine == (int)BAPE_DolbyPulseDrcMode_eLine);
        BDBG_CASSERT((int)NEXUS_AudioDecoderDolbyPulseDrcMode_eRf == (int)BAPE_DolbyPulseDrcMode_eRf);
        BDBG_CASSERT((int)NEXUS_AudioDecoderDolbyPulseDrcMode_eOff == (int)BAPE_DolbyPulseDrcMode_eOff);
        pSettings->codecSettings.aac.drcMode = codecSettings.codecSettings.aac.drcMode;
        pSettings->codecSettings.aac.drcDefaultLevel = codecSettings.codecSettings.aac.drcDefaultLevel;
        pSettings->codecSettings.aac.mpegConformanceMode = codecSettings.codecSettings.aac.mpegConformanceMode;
        pSettings->codecSettings.aac.enableSbrDecoding = codecSettings.codecSettings.aac.enableSbrDecoding;
        pSettings->codecSettings.aac.ignoreEmbeddedPrl = codecSettings.codecSettings.aac.ignoreEmbeddedPrl;
        break;
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:
        pSettings->codecSettings.aacPlus.cut = codecSettings.codecSettings.aacPlus.drcScaleHi;
        pSettings->codecSettings.aacPlus.boost = codecSettings.codecSettings.aacPlus.drcScaleLow;
        pSettings->codecSettings.aacPlus.drcTargetLevel = codecSettings.codecSettings.aacPlus.drcReferenceLevel;
        BDBG_CASSERT((int)BAPE_AacStereoMode_eMatrix == (int)NEXUS_AudioDecoderAacDownmixMode_eMatrix);
        BDBG_CASSERT((int)BAPE_AacStereoMode_eArib == (int)NEXUS_AudioDecoderAacDownmixMode_eArib);
        BDBG_CASSERT((int)BAPE_AacStereoMode_eLtRt == (int)NEXUS_AudioDecoderAacDownmixMode_eLtRt);
        BDBG_CASSERT((int)BAPE_AacStereoMode_eLoRo == (int)NEXUS_AudioDecoderAacDownmixMode_eLoRo);
        pSettings->codecSettings.aacPlus.downmixMode = codecSettings.codecSettings.aacPlus.downmixMode;
        BDBG_CASSERT((int)NEXUS_AudioDecoderDolbyPulseDrcMode_eLine == (int)BAPE_DolbyPulseDrcMode_eLine);
        BDBG_CASSERT((int)NEXUS_AudioDecoderDolbyPulseDrcMode_eRf == (int)BAPE_DolbyPulseDrcMode_eRf);
        BDBG_CASSERT((int)NEXUS_AudioDecoderDolbyPulseDrcMode_eOff == (int)BAPE_DolbyPulseDrcMode_eOff);
        pSettings->codecSettings.aacPlus.drcMode = codecSettings.codecSettings.aacPlus.drcMode;
        pSettings->codecSettings.aacPlus.drcDefaultLevel = codecSettings.codecSettings.aacPlus.drcDefaultLevel;
        pSettings->codecSettings.aacPlus.mpegConformanceMode = codecSettings.codecSettings.aacPlus.mpegConformanceMode;
        pSettings->codecSettings.aacPlus.enableSbrDecoding = codecSettings.codecSettings.aacPlus.enableSbrDecoding;
        pSettings->codecSettings.aacPlus.ignoreEmbeddedPrl = codecSettings.codecSettings.aacPlus.ignoreEmbeddedPrl;
        break;
    case NEXUS_AudioCodec_eMp3:
        pSettings->codecSettings.mp3.inputReferenceLevel = codecSettings.codecSettings.mp3.inputReferenceLevel;
        pSettings->codecSettings.mp3.attenuateMonoToStereo = codecSettings.codecSettings.mp3.attenuateMonoToStereo;
        break;
    case NEXUS_AudioCodec_eMpeg:
        pSettings->codecSettings.mpeg.inputReferenceLevel = codecSettings.codecSettings.mpeg.inputReferenceLevel;
        pSettings->codecSettings.mpeg.attenuateMonoToStereo = codecSettings.codecSettings.mpeg.attenuateMonoToStereo;
        break;
    case NEXUS_AudioCodec_eWmaPro:
        /* TODO: Nexus is exposing the older WMA Pro DRC mode.  This should switch from bool .. enum to match APE */
        switch ( codecSettings.codecSettings.wmaPro.drcMode )
        {
        case BAPE_WmaProDrcMode_eDisabled:
            pSettings->codecSettings.wmaPro.dynamicRangeControlValid = false;
            break;
        default:
            pSettings->codecSettings.wmaPro.dynamicRangeControlValid = true;
            break;
        }
        /* TODO: Expose stereo mode */
        pSettings->codecSettings.wmaPro.dynamicRangeControl.peakReference = codecSettings.codecSettings.wmaPro.peakAmplitudeReference;
        pSettings->codecSettings.wmaPro.dynamicRangeControl.peakTarget = codecSettings.codecSettings.wmaPro.desiredPeak;
        pSettings->codecSettings.wmaPro.dynamicRangeControl.averageReference = codecSettings.codecSettings.wmaPro.rmsAmplitudeReference;
        pSettings->codecSettings.wmaPro.dynamicRangeControl.averageTarget = codecSettings.codecSettings.wmaPro.desiredRms;
        break;
    case NEXUS_AudioCodec_eDts:
    case NEXUS_AudioCodec_eDtsHd:
    case NEXUS_AudioCodec_eDtsLegacy: /* For DTS streams with legacy frame-sync.  These streams are something called as 14bits stream */
    case NEXUS_AudioCodec_eDtsExpress:
        pSettings->codecSettings.dts.mixLfeToPrimary = codecSettings.codecSettings.dts.mixLfeToPrimary;
        BDBG_CASSERT((int)NEXUS_AudioDecoderDtsDownmixMode_eAuto == (int)BAPE_DtsStereoMode_eAuto);
        BDBG_CASSERT((int)NEXUS_AudioDecoderDtsDownmixMode_eLtRt == (int)BAPE_DtsStereoMode_eLtRt);
        BDBG_CASSERT((int)NEXUS_AudioDecoderDtsDownmixMode_eLoRo == (int)BAPE_DtsStereoMode_eLoRo);
        pSettings->codecSettings.dts.stereoDownmixMode = (NEXUS_AudioDecoderDtsStereoDownmixMode)codecSettings.codecSettings.dts.stereoMode;
        pSettings->codecSettings.dts.enableDrc = codecSettings.codecSettings.dts.drcMode == BAPE_DtsDrcMode_eEnabled ? true : false;
        pSettings->codecSettings.dts.boost = codecSettings.codecSettings.dts.drcScaleLow;
        pSettings->codecSettings.dts.cut = codecSettings.codecSettings.dts.drcScaleHi;
        pSettings->codecSettings.dts.littleEndian = codecSettings.codecSettings.dts.littleEndian;
        break;
    case NEXUS_AudioCodec_eAdpcm:
        pSettings->codecSettings.adpcm.enableGain = codecSettings.codecSettings.adpcm.gain.enabled;
        pSettings->codecSettings.adpcm.gainFactor = codecSettings.codecSettings.adpcm.gain.factor;
        break;

    case NEXUS_AudioCodec_eIlbc:
        pSettings->codecSettings.ilbc.packetLoss = codecSettings.codecSettings.ilbc.packetLoss;
        pSettings->codecSettings.ilbc.frameLength = codecSettings.codecSettings.ilbc.frameLength;
        pSettings->codecSettings.ilbc.enableGain = codecSettings.codecSettings.ilbc.gain.enabled;
        pSettings->codecSettings.ilbc.gainFactor = codecSettings.codecSettings.ilbc.gain.factor;
        break;
    case NEXUS_AudioCodec_eIsac:
        pSettings->codecSettings.isac.packetLoss = codecSettings.codecSettings.isac.packetLoss;
        pSettings->codecSettings.isac.bandMode = codecSettings.codecSettings.isac.bandMode;
        pSettings->codecSettings.isac.enableGain = codecSettings.codecSettings.isac.gain.enabled;
        pSettings->codecSettings.isac.gainFactor = codecSettings.codecSettings.isac.gain.factor;
        break;
    case NEXUS_AudioCodec_eAls:
    case NEXUS_AudioCodec_eAlsLoas:
        switch (codecSettings.codecSettings.als.stereoMode) {
        default:
        case BAPE_AlsStereoMode_eArib:
            pSettings->codecSettings.als.stereoMode = NEXUS_AudioDecoderStereoDownmixMode_eArib;
            break;
        case BAPE_AlsStereoMode_eLtRt:
            pSettings->codecSettings.als.stereoMode = NEXUS_AudioDecoderStereoDownmixMode_eLtRt;
            break;
        }
        pSettings->codecSettings.als.aribMatrixMixdownIndex = codecSettings.codecSettings.als.aribMatrixMixdownIndex;
        break;
    default:
        return;
    }
}

/***************************************************************************
Summary:
    Set codec-specific decoder settings
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_SetCodecSettings(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderCodecSettings *pSettings
    )
{
    BAPE_DecoderCodecSettings codecSettings;
    BAVC_AudioCompressionStd avcCodec;
    BERR_Code errCode;
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSettings);

    avcCodec = NEXUS_Audio_P_CodecToMagnum(pSettings->codec);

    BAPE_Decoder_GetCodecSettings(handle->channel, avcCodec, &codecSettings);
    switch ( pSettings->codec )
    {
    case NEXUS_AudioCodec_eAc3:
        codecSettings.codecSettings.ac3.drcMode = pSettings->codecSettings.ac3.drcMode;
        codecSettings.codecSettings.ac3.drcModeDownmix = pSettings->codecSettings.ac3.drcModeDownmix;
        codecSettings.codecSettings.ac3.customTargetLevel = pSettings->codecSettings.ac3.customTargetLevel;
        codecSettings.codecSettings.ac3.customTargetLevelDownmix = pSettings->codecSettings.ac3.customTargetLevelDownmix;
        codecSettings.codecSettings.ac3.drcScaleHi = pSettings->codecSettings.ac3.cut;
        codecSettings.codecSettings.ac3.drcScaleLow = pSettings->codecSettings.ac3.boost;
        codecSettings.codecSettings.ac3.drcScaleHiDownmix = pSettings->codecSettings.ac3.cutDownmix;
        codecSettings.codecSettings.ac3.drcScaleLowDownmix = pSettings->codecSettings.ac3.boostDownmix;
        codecSettings.codecSettings.ac3.stereoMode = pSettings->codecSettings.ac3.stereoDownmixMode;
        codecSettings.codecSettings.ac3.scale = pSettings->codecSettings.ac3.scale;
        codecSettings.codecSettings.ac3.scaleDownmix = pSettings->codecSettings.ac3.scaleDownmix;
        codecSettings.codecSettings.ac3.dialogNormalization = pSettings->codecSettings.ac3.dialogNormalization;
        codecSettings.codecSettings.ac3.dialogNormalizationValue = pSettings->codecSettings.ac3.dialogNormalizationValue;
        codecSettings.codecSettings.ac3.substreamId = pSettings->codecSettings.ac3.substreamId;
        codecSettings.codecSettings.ac3.enableAtmosProcessing = pSettings->codecSettings.ac3.enableAtmosProcessing;
        codecSettings.codecSettings.ac3.certificationMode = pSettings->codecSettings.ac3.certificationMode;
        break;
    case NEXUS_AudioCodec_eAc3Plus:
        codecSettings.codecSettings.ac3Plus.drcMode = pSettings->codecSettings.ac3Plus.drcMode;
        codecSettings.codecSettings.ac3Plus.drcModeDownmix = pSettings->codecSettings.ac3Plus.drcModeDownmix;
        codecSettings.codecSettings.ac3Plus.customTargetLevel = pSettings->codecSettings.ac3Plus.customTargetLevel;
        codecSettings.codecSettings.ac3Plus.customTargetLevelDownmix = pSettings->codecSettings.ac3Plus.customTargetLevelDownmix;
        codecSettings.codecSettings.ac3Plus.drcScaleHi = pSettings->codecSettings.ac3Plus.cut;
        codecSettings.codecSettings.ac3Plus.drcScaleLow = pSettings->codecSettings.ac3Plus.boost;
        codecSettings.codecSettings.ac3Plus.drcScaleHiDownmix = pSettings->codecSettings.ac3Plus.cutDownmix;
        codecSettings.codecSettings.ac3Plus.drcScaleLowDownmix = pSettings->codecSettings.ac3Plus.boostDownmix;
        codecSettings.codecSettings.ac3Plus.stereoMode = pSettings->codecSettings.ac3Plus.stereoDownmixMode;
        codecSettings.codecSettings.ac3Plus.scale = pSettings->codecSettings.ac3Plus.scale;
        codecSettings.codecSettings.ac3Plus.scaleDownmix = pSettings->codecSettings.ac3Plus.scaleDownmix;
        codecSettings.codecSettings.ac3Plus.dialogNormalization = pSettings->codecSettings.ac3Plus.dialogNormalization;
        codecSettings.codecSettings.ac3Plus.dialogNormalizationValue = pSettings->codecSettings.ac3Plus.dialogNormalizationValue;
        codecSettings.codecSettings.ac3Plus.substreamId = pSettings->codecSettings.ac3Plus.substreamId;
        codecSettings.codecSettings.ac3Plus.enableAtmosProcessing = pSettings->codecSettings.ac3Plus.enableAtmosProcessing;
        codecSettings.codecSettings.ac3Plus.certificationMode = pSettings->codecSettings.ac3Plus.certificationMode;
        break;
    case NEXUS_AudioCodec_eAc4:
        switch ( pSettings->codecSettings.ac4.drcMode )
        {
        case NEXUS_AudioDecoderDolbyDrcMode_eLine:
            codecSettings.codecSettings.ac4.drcMode = BAPE_DolbyDrcMode_eLine;
            break;
        case NEXUS_AudioDecoderDolbyDrcMode_eRf:
            codecSettings.codecSettings.ac4.drcMode = BAPE_DolbyDrcMode_eRf;
            break;
        case NEXUS_AudioDecoderDolbyDrcMode_eOff:
            codecSettings.codecSettings.ac4.drcMode = BAPE_DolbyDrcMode_eDisabled;
            break;
        default:
            BDBG_ERR(("Invalid drcMode %lu specified, defaulting to eLine", (unsigned long)pSettings->codecSettings.ac4.drcMode));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
        }
        switch ( pSettings->codecSettings.ac4.drcModeDownmix )
        {
        case NEXUS_AudioDecoderDolbyDrcMode_eRf:
            codecSettings.codecSettings.ac4.drcModeDownmix = BAPE_DolbyDrcMode_eRf;
            break;
        case NEXUS_AudioDecoderDolbyDrcMode_eLine:
            codecSettings.codecSettings.ac4.drcModeDownmix = BAPE_DolbyDrcMode_eLine;
            break;
        case NEXUS_AudioDecoderDolbyDrcMode_eOff:
            codecSettings.codecSettings.ac4.drcModeDownmix = BAPE_DolbyDrcMode_eDisabled;
            break;
        default:
            BDBG_ERR(("Invalid drcModeDownmix %lu specified, defaulting to eRf", (unsigned long)pSettings->codecSettings.ac4.drcModeDownmix));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
        }
        codecSettings.codecSettings.ac4.drcScaleHi = pSettings->codecSettings.ac4.drcScaleHi;
        codecSettings.codecSettings.ac4.drcScaleLow = pSettings->codecSettings.ac4.drcScaleLow;
        codecSettings.codecSettings.ac4.drcScaleHiDownmix = pSettings->codecSettings.ac4.drcScaleHiDownmix;
        codecSettings.codecSettings.ac4.drcScaleLowDownmix = pSettings->codecSettings.ac4.drcScaleLowDownmix;
        switch ( pSettings->codecSettings.ac4.stereoMode )
        {
        case NEXUS_AudioDecoderStereoDownmixMode_eLoRo:
            codecSettings.codecSettings.ac4.stereoMode = BAPE_DolbyStereoMode_eLoRo;
            break;
        case NEXUS_AudioDecoderStereoDownmixMode_eLtRt:
            codecSettings.codecSettings.ac4.stereoMode = BAPE_DolbyStereoMode_eLtRt;
            break;
        default:
            BDBG_ERR(("Invalid stereo mode %lu specified, defaulting to eLoRo", (unsigned long)pSettings->codecSettings.ac4.stereoMode));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
        }
        codecSettings.codecSettings.ac4.selectionMode = (BAPE_Ac4PresentationSelectionMode)pSettings->codecSettings.ac4.selectionMode;
        codecSettings.codecSettings.ac4.programSelection = pSettings->codecSettings.ac4.programSelection;
        codecSettings.codecSettings.ac4.programBalance = pSettings->codecSettings.ac4.programBalance;
        codecSettings.codecSettings.ac4.dialogEnhancerAmount = pSettings->codecSettings.ac4.dialogEnhancerAmount;
        codecSettings.codecSettings.ac4.certificationMode = pSettings->codecSettings.ac4.certificationMode;
        codecSettings.codecSettings.ac4.enableAssociateMixing = pSettings->codecSettings.ac4.enableAssociateMixing;
        /* personalization settings */
        BKNI_Memcpy(codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eMain].presentationId, pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationId, sizeof(char) * BAPE_AC4_PRESENTATION_ID_LENGTH);
        BKNI_Memcpy(codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eAlternate].presentationId, pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].presentationId, sizeof(char) * BAPE_AC4_PRESENTATION_ID_LENGTH);
        codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eMain].presentationIndex = pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationIndex;
        codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eAlternate].presentationIndex = pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].presentationIndex;
        codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eMain].preferLanguageOverAssociateType = pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferLanguageOverAssociateType;
        codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eAlternate].preferLanguageOverAssociateType = pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferLanguageOverAssociateType;
        codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eMain].preferredAssociateType = (BAPE_Ac4AssociateType)pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferredAssociateType;
        codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eAlternate].preferredAssociateType = (BAPE_Ac4AssociateType)pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferredAssociateType;
        for ( i = 0; i < BAPE_AC4_NUM_LANGUAGES; i++ )
        {
            BKNI_Memcpy(codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eMain].languagePreference[i].selection, pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[i].selection, sizeof(char) * BAPE_AC4_LANGUAGE_NAME_LENGTH);
            BKNI_Memcpy(codecSettings.codecSettings.ac4.programs[BAPE_Ac4Program_eAlternate].languagePreference[i].selection, pSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[i].selection, sizeof(char) * BAPE_AC4_LANGUAGE_NAME_LENGTH);
        }
        break;
    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacLoas:
        codecSettings.codecSettings.aac.drcScaleHi = pSettings->codecSettings.aac.cut;
        codecSettings.codecSettings.aac.drcScaleLow = pSettings->codecSettings.aac.boost;
        codecSettings.codecSettings.aac.drcReferenceLevel = pSettings->codecSettings.aac.drcTargetLevel;
        codecSettings.codecSettings.aac.downmixMode = pSettings->codecSettings.aac.downmixMode;
        codecSettings.codecSettings.aac.drcMode = pSettings->codecSettings.aac.drcMode;
        codecSettings.codecSettings.aac.drcDefaultLevel = pSettings->codecSettings.aac.drcDefaultLevel;
        codecSettings.codecSettings.aac.mpegConformanceMode = pSettings->codecSettings.aac.mpegConformanceMode;
        codecSettings.codecSettings.aac.enableSbrDecoding = pSettings->codecSettings.aac.enableSbrDecoding;
        codecSettings.codecSettings.aac.ignoreEmbeddedPrl = pSettings->codecSettings.aac.ignoreEmbeddedPrl;
        break;
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:
        codecSettings.codecSettings.aacPlus.drcScaleHi = pSettings->codecSettings.aacPlus.cut;
        codecSettings.codecSettings.aacPlus.drcScaleLow = pSettings->codecSettings.aacPlus.boost;
        codecSettings.codecSettings.aacPlus.drcReferenceLevel = pSettings->codecSettings.aacPlus.drcTargetLevel;
        codecSettings.codecSettings.aacPlus.downmixMode = pSettings->codecSettings.aacPlus.downmixMode;
        codecSettings.codecSettings.aacPlus.drcMode = pSettings->codecSettings.aacPlus.drcMode;
        codecSettings.codecSettings.aacPlus.drcDefaultLevel = pSettings->codecSettings.aacPlus.drcDefaultLevel;
        codecSettings.codecSettings.aacPlus.mpegConformanceMode = pSettings->codecSettings.aacPlus.mpegConformanceMode;
        codecSettings.codecSettings.aacPlus.enableSbrDecoding = pSettings->codecSettings.aacPlus.enableSbrDecoding;
        codecSettings.codecSettings.aacPlus.ignoreEmbeddedPrl = pSettings->codecSettings.aacPlus.ignoreEmbeddedPrl;
        break;
    case NEXUS_AudioCodec_eMp3:
        codecSettings.codecSettings.mp3.inputReferenceLevel = pSettings->codecSettings.mp3.inputReferenceLevel;
        codecSettings.codecSettings.mp3.attenuateMonoToStereo = pSettings->codecSettings.mp3.attenuateMonoToStereo;
        break;
    case NEXUS_AudioCodec_eMpeg:
        codecSettings.codecSettings.mpeg.inputReferenceLevel = pSettings->codecSettings.mpeg.inputReferenceLevel;
        codecSettings.codecSettings.mpeg.attenuateMonoToStereo = pSettings->codecSettings.mpeg.attenuateMonoToStereo;
        break;
    case NEXUS_AudioCodec_eWmaPro:
        /* TODO: Nexus is exposing the older WMA Pro DRC mode.  This should switch from bool .. enum to match APE */
        if ( pSettings->codecSettings.wmaPro.dynamicRangeControlValid )
        {
            codecSettings.codecSettings.wmaPro.drcMode = BAPE_WmaProDrcMode_eHigh;
        }
        else
        {
            codecSettings.codecSettings.wmaPro.drcMode = BAPE_WmaProDrcMode_eDisabled;
        }
        /* TODO: Expose stereo mode */
        codecSettings.codecSettings.wmaPro.peakAmplitudeReference = pSettings->codecSettings.wmaPro.dynamicRangeControl.peakReference;
        codecSettings.codecSettings.wmaPro.desiredPeak = pSettings->codecSettings.wmaPro.dynamicRangeControl.peakTarget;
        codecSettings.codecSettings.wmaPro.rmsAmplitudeReference = pSettings->codecSettings.wmaPro.dynamicRangeControl.averageReference;
        codecSettings.codecSettings.wmaPro.desiredRms = pSettings->codecSettings.wmaPro.dynamicRangeControl.averageTarget;
        break;
    case NEXUS_AudioCodec_eDts:
    case NEXUS_AudioCodec_eDtsHd:
    case NEXUS_AudioCodec_eDtsLegacy: /* For DTS streams with legacy frame-sync.  These streams are something called as 14bits stream */
    case NEXUS_AudioCodec_eDtsExpress:
        codecSettings.codecSettings.dts.littleEndian = pSettings->codecSettings.dts.littleEndian;
        codecSettings.codecSettings.dts.mixLfeToPrimary = pSettings->codecSettings.dts.mixLfeToPrimary;
        codecSettings.codecSettings.dts.stereoMode = (BAPE_DtsStereoMode)pSettings->codecSettings.dts.stereoDownmixMode;
        codecSettings.codecSettings.dts.drcMode = pSettings->codecSettings.dts.enableDrc ? BAPE_DtsDrcMode_eEnabled : BAPE_DtsDrcMode_eDisabled;
        codecSettings.codecSettings.dts.drcScaleLow = pSettings->codecSettings.dts.boost;
        codecSettings.codecSettings.dts.drcScaleHi = pSettings->codecSettings.dts.cut;
        break;
    case NEXUS_AudioCodec_eAdpcm:
        codecSettings.codecSettings.adpcm.gain.enabled = pSettings->codecSettings.adpcm.enableGain;
        codecSettings.codecSettings.adpcm.gain.factor = pSettings->codecSettings.adpcm.gainFactor;
        break;
    case NEXUS_AudioCodec_eIlbc:
        codecSettings.codecSettings.ilbc.packetLoss = pSettings->codecSettings.ilbc.packetLoss;
        codecSettings.codecSettings.ilbc.frameLength = pSettings->codecSettings.ilbc.frameLength;
        codecSettings.codecSettings.ilbc.gain.enabled = pSettings->codecSettings.ilbc.enableGain;
        codecSettings.codecSettings.ilbc.gain.factor = pSettings->codecSettings.ilbc.gainFactor;
        break;
    case NEXUS_AudioCodec_eIsac:
        codecSettings.codecSettings.isac.packetLoss = pSettings->codecSettings.isac.packetLoss;
        codecSettings.codecSettings.isac.bandMode = pSettings->codecSettings.isac.bandMode;
        codecSettings.codecSettings.isac.gain.enabled = pSettings->codecSettings.isac.enableGain;
        codecSettings.codecSettings.isac.gain.factor = pSettings->codecSettings.isac.gainFactor;
        break;
    case NEXUS_AudioCodec_eAls:
        switch (pSettings->codecSettings.als.stereoMode)
        {
        case NEXUS_AudioDecoderStereoDownmixMode_eArib:
            codecSettings.codecSettings.als.stereoMode = BAPE_AlsStereoMode_eArib;
            break;
        case NEXUS_AudioDecoderStereoDownmixMode_eLtRt:
            codecSettings.codecSettings.als.stereoMode = BAPE_AlsStereoMode_eLtRt;
            break;
        default:
            BDBG_ERR(("%s Invalid Stereo Mode(%d) for ALS.  Only ARIB and LtRT supported", BSTD_FUNCTION, pSettings->codecSettings.als.stereoMode));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
        }
        codecSettings.codecSettings.als.aribMatrixMixdownIndex = pSettings->codecSettings.als.aribMatrixMixdownIndex;
        break;
    default:
        return BERR_SUCCESS;
    }

    errCode = BAPE_Decoder_SetCodecSettings(handle->channel, &codecSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

#if 0 /* Currently Unused */
static unsigned NEXUS_AudioDecoder_P_GetAc3Bitrate(unsigned frameSizeCode)
{
    const unsigned bitrateTable[] = {

        32000,  /* '000000' 32 kbps  */
        32000,  /* '000001' 32 kbps  */
        40000,  /* '000010' 40 kbps  */
        40000,  /* '000011' 40 kbps  */
        48000,  /* '000100' 48 kbps  */
        48000,  /* '000101' 48 kbps  */
        56000,  /* '000110' 56 kbps  */
        56000,  /* '000111' 56 kbps  */
        64000,  /* '001000' 64 kbps  */
        64000,  /* '001001' 64 kbps  */
        80000,  /* '001010' 80 kbps  */
        80000,  /* '001011' 80 kbps  */
        96000,  /* '001100' 96 kbps  */
        96000,  /* '001101' 96 kbps  */
        112000, /* '001110' 112 kbps */
        112000, /* '001111' 112 kbps */

        128000, /* '010000' 128 kbps */
        128000, /* '010001' 128 kbps */
        160000, /* '010010' 160 kbps */
        160000, /* '010011' 160 kbps */
        192000, /* '010100' 192 kbps */
        192000, /* '010101' 192 kbps */
        224000, /* '010110' 224 kbps */
        224000, /* '010111' 224 kbps */
        256000, /* '011000' 256 kbps */
        256000, /* '011001' 256 kbps */
        320000, /* '011010' 320 kbps */
        320000, /* '011011' 320 kbps */
        384000, /* '011100' 384 kbps */
        384000, /* '011101' 384 kbps */
        448000, /* '011110' 448 kbps */
        448000, /* '011111' 448 kbps */

        512000, /* '100000' 512 kbps */
        512000, /* '100001' 512 kbps */
        576000, /* '100010' 576 kbps */
        576000, /* '100011' 576 kbps */
        640000, /* '100100' 640 kbps */
        640000  /* '100101' 640 kbps */
    };

    if ( frameSizeCode < 38 )
    {
        return bitrateTable[frameSizeCode];
    }
    else
    {
        return 0;
    }
}

#endif

/***************************************************************************
Summary:
    Get the current audio decoder status
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetStatus(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderStatus *pStatus   /* [out] current status */
    )
{
    unsigned depth=0, size=0;
    BAPE_DecoderStatus decoderStatus;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_AudioDecoderStatus));
    pStatus->started = handle->started ? NEXUS_AudioRunningState_eStarted :
        (handle->suspended ? NEXUS_AudioRunningState_eSuspended : NEXUS_AudioRunningState_eStopped);
    pStatus->locked = handle->locked;
    pStatus->codec = handle->programSettings.codec;
    pStatus->ptsType = NEXUS_PtsType_eInterpolatedFromInvalidPTS;
    pStatus->numFifoOverflows = handle->numFifoOverflows;
    pStatus->numFifoUnderflows = handle->numFifoUnderflows;

    if(!handle->running)
    {
        return BERR_SUCCESS;
    }

    BKNI_EnterCriticalSection();
    if ( handle->programSettings.pidChannel )
    {
        NEXUS_Rave_GetCdbBufferInfo_isr(handle->raveContext, &depth, &size);
    }
    BKNI_LeaveCriticalSection();

    pStatus->fifoDepth = depth;
    pStatus->fifoSize = size;

    pStatus->tsm = handle->tsmEnabled;
    pStatus->timebase = handle->apeStartSettings.stcIndex;
    pStatus->ptsErrorCount = handle->ptsErrorCount;
    pStatus->codec = handle->programSettings.codec;
    pStatus->numWatchdogs = g_numWatchdogs;

    if ( handle->programSettings.pidChannel )
    {
        NEXUS_RaveStatus raveStatus;
        NEXUS_Error errCode;
        LOCK_TRANSPORT();
        NEXUS_Rave_GetAudioFrameCount_priv(handle->raveContext, &pStatus->queuedFrames);
        errCode = NEXUS_Rave_GetStatus_priv(handle->raveContext, &raveStatus);
        UNLOCK_TRANSPORT();
        if ( NEXUS_SUCCESS == errCode )
        {
            if(raveStatus.numOutputBytes >= pStatus->fifoDepth) {
                pStatus->numBytesDecoded = raveStatus.numOutputBytes - pStatus->fifoDepth;
            } else {
                pStatus->numBytesDecoded = 0;
            }
        }
    }

    /* Get decoder info */
    BAPE_Decoder_GetStatus(handle->channel, &decoderStatus);
    {
        unsigned frameLength, bitrate;

        pStatus->valid = decoderStatus.valid;
        pStatus->sampleRate = decoderStatus.sampleRate;
        pStatus->pts = decoderStatus.tsmStatus.ptsInfo.ui32CurrentPTS;
        pStatus->ptsType = (NEXUS_PtsType)decoderStatus.tsmStatus.ptsInfo.ePTSType;
        pStatus->ptsStcDifference = decoderStatus.tsmStatus.ptsStcDifference;

        pStatus->framesDecoded = decoderStatus.framesDecoded;
        pStatus->frameErrors = decoderStatus.frameErrors;
        pStatus->dummyFrames = decoderStatus.dummyFrames;
        pStatus->queuedOutput = decoderStatus.queuedOutput;

        /* Convert codec to nexus type */
        pStatus->codec = NEXUS_Audio_P_MagnumToCodec(decoderStatus.codec);

        /* Handle specifics per-codec */
        switch ( decoderStatus.codec )
        {
        case BAVC_AudioCompressionStd_eMpegL1:
        case BAVC_AudioCompressionStd_eMpegL2:
        case BAVC_AudioCompressionStd_eMpegL3:
            pStatus->codec = (decoderStatus.codecStatus.mpeg.layer == 3)?NEXUS_AudioCodec_eMp3:NEXUS_AudioCodec_eMpeg;
            pStatus->codecStatus.mpeg.channelMode = decoderStatus.codecStatus.mpeg.mpegChannelMode;
            switch (  decoderStatus.codecStatus.mpeg.layer )
            {
            default:
                BDBG_MSG(("Invalid MPEG layer %d",decoderStatus.codecStatus.mpeg.layer));
                BKNI_Memset(&pStatus->codecStatus, 0, sizeof(pStatus->codecStatus));
                pStatus->codec = NEXUS_AudioCodec_eUnknown;
                goto status_complete;
                break; /* unreachable */
            case 1:
                pStatus->codecStatus.mpeg.layer = NEXUS_AudioMpegLayer_e1;
                pStatus->codec = NEXUS_AudioCodec_eMpeg;
                break;
            case 2:
                pStatus->codecStatus.mpeg.layer = NEXUS_AudioMpegLayer_e2;
                pStatus->codec = NEXUS_AudioCodec_eMpeg;
                break;
            case 3:
                pStatus->codecStatus.mpeg.layer = NEXUS_AudioMpegLayer_e3;
                pStatus->codec = NEXUS_AudioCodec_eMp3;
                break;
            }
            pStatus->codecStatus.mpeg.emphasis = decoderStatus.codecStatus.mpeg.emphasisMode;
            pStatus->codecStatus.mpeg.original = decoderStatus.codecStatus.mpeg.original;
            pStatus->codecStatus.mpeg.copyright = decoderStatus.codecStatus.mpeg.copyright;
            pStatus->codecStatus.mpeg.bitrate = decoderStatus.codecStatus.mpeg.bitRate;
            if (  pStatus->sampleRate > 0 )
            {
                /* MPEG audio uses a CDB sync, so the frame count is bogus.  Calculate based on frame size and CDB depth */
                bitrate = (decoderStatus.codecStatus.mpeg.bitRate>0)?1000*decoderStatus.codecStatus.mpeg.bitRate:128000;
                if ( decoderStatus.codecStatus.mpeg.layer == 1 )
                {
                    frameLength = (48*bitrate)/pStatus->sampleRate;
                }
                else
                {
                    frameLength = (144*bitrate)/pStatus->sampleRate;
                }
                if ( frameLength == 0 )
                {
                    pStatus->queuedFrames = 0;
                }
                else
                {
                    pStatus->queuedFrames = pStatus->fifoDepth/frameLength;
                }
                BDBG_MSG_TRACE(("Queued Frames %d bitrate %d framelength %d samplerate %d", pStatus->queuedFrames, bitrate, frameLength, pStatus->sampleRate));
            }
            break;
        case BAVC_AudioCompressionStd_eAacLoas:
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacPlusLoas:
        case BAVC_AudioCompressionStd_eAacPlusAdts:
            /* Convert channel mode to aac acmod */
            switch ( decoderStatus.codecStatus.aac.channelMode )
            {
            case BAPE_ChannelMode_e1_0: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eOneCenter_1_0_C; break;
            case BAPE_ChannelMode_e1_1: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eTwoMono_1_ch1_ch2; break;
            case BAPE_ChannelMode_e2_0: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eTwoChannel_2_0_L_R; break;
            case BAPE_ChannelMode_e3_0: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eThreeChannel_3_0_L_C_R; break;
            case BAPE_ChannelMode_e2_1: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eThreeChannel_2_1_L_R_S; break;
            case BAPE_ChannelMode_e3_1: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eFourChannel_3_1_L_C_R_S; break;
            case BAPE_ChannelMode_e2_2: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eFourChannel_2_2_L_R_SL_SR; break;
            case BAPE_ChannelMode_e3_2: pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eFiveChannel_3_2_L_C_R_SL_SR; break;
            default:                    pStatus->codecStatus.aac.acmod = NEXUS_AudioAacAcmod_eMax; break;
            }
            pStatus->codecStatus.aac.profile = decoderStatus.codecStatus.aac.profile;
            pStatus->codecStatus.aac.bitrate = decoderStatus.codecStatus.aac.bitRate;
            pStatus->codecStatus.aac.lfe = decoderStatus.codecStatus.aac.lfe;
            pStatus->codecStatus.aac.pseudoSurround = decoderStatus.codecStatus.aac.pseudoSurround;
            pStatus->codecStatus.aac.stereoMatrix = decoderStatus.codecStatus.aac.stereoMatrix;
            pStatus->codecStatus.aac.matrixIndex = decoderStatus.codecStatus.aac.matrixIndex;
            pStatus->codecStatus.aac.numLfeChannels = decoderStatus.codecStatus.aac.numLfeChannels;
            pStatus->codecStatus.aac.numBackChannels = decoderStatus.codecStatus.aac.numBackChannels;
            pStatus->codecStatus.aac.numSideChannels = decoderStatus.codecStatus.aac.numSideChannels;
            pStatus->codecStatus.aac.numFrontChannels = decoderStatus.codecStatus.aac.numFrontChannels;
            pStatus->codecStatus.aac.dialnorm = decoderStatus.codecStatus.aac.dialnorm;
            pStatus->codecStatus.aac.previousDialnorm = decoderStatus.codecStatus.aac.previousDialnorm;
            break;
        case BAVC_AudioCompressionStd_eAc3:
        case BAVC_AudioCompressionStd_eAc3Plus:
            pStatus->codec = (decoderStatus.codec == BAVC_AudioCompressionStd_eAc3)?NEXUS_AudioCodec_eAc3:NEXUS_AudioCodec_eAc3Plus;
            pStatus->codecStatus.ac3.bitStreamId = decoderStatus.codecStatus.ac3.bitstreamId;
            BDBG_CASSERT((int)NEXUS_AudioAc3Acmod_eMax==(int)BAPE_Ac3Acmod_eMax);
            pStatus->codecStatus.ac3.acmod = decoderStatus.codecStatus.ac3.acmod;
            pStatus->codecStatus.ac3.frameSizeCode = decoderStatus.codecStatus.ac3.frameSizeCode;
            pStatus->codecStatus.ac3.bitrate = decoderStatus.codecStatus.ac3.bitrate;
            pStatus->codecStatus.ac3.lfe = decoderStatus.codecStatus.ac3.lfe;
            pStatus->codecStatus.ac3.copyright = decoderStatus.codecStatus.ac3.copyright;
            BDBG_CASSERT((int)NEXUS_AudioAc3DependentFrameChannelMap_eMax==(int)BAPE_Ac3DependentFrameChannelMap_eMax);
            pStatus->codecStatus.ac3.dependentFrameChannelMap = (NEXUS_AudioAc3DependentFrameChannelMap)decoderStatus.codecStatus.ac3.dependentFrameChannelMap;
            pStatus->codecStatus.ac3.dialnorm = decoderStatus.codecStatus.ac3.dialnorm;
            pStatus->codecStatus.ac3.previousDialnorm = decoderStatus.codecStatus.ac3.previousDialnorm;
            pStatus->codecStatus.ac3.atmosDetected = decoderStatus.codecStatus.ac3.atmosDetected;
            break;
        case BAVC_AudioCompressionStd_eAc4:
            BDBG_CASSERT((int)NEXUS_AudioAc4Acmod_eMax==(int)BAPE_Ac4Acmod_eMax);
            pStatus->codecStatus.ac4.acmod = decoderStatus.codecStatus.ac4.acmod;
            pStatus->codecStatus.ac4.lfe = decoderStatus.codecStatus.ac4.lfe;
            pStatus->codecStatus.ac4.bitrate = decoderStatus.codecStatus.ac4.bitrate;
            pStatus->codecStatus.ac4.dialnorm = decoderStatus.codecStatus.ac4.dialnorm;
            pStatus->codecStatus.ac4.previousDialnorm = decoderStatus.codecStatus.ac4.previousDialnorm;

            pStatus->codecStatus.ac4.streamInfoVersion = decoderStatus.codecStatus.ac4.streamInfoVersion;
            pStatus->codecStatus.ac4.numPresentations = decoderStatus.codecStatus.ac4.numPresentations;
            pStatus->codecStatus.ac4.currentPresentationIndex = decoderStatus.codecStatus.ac4.currentPresentationIndex;
            pStatus->codecStatus.ac4.currentAlternateStereoPresentationIndex = decoderStatus.codecStatus.ac4.currentAlternateStereoPresentationIndex;
            pStatus->codecStatus.ac4.dialogEnhanceMax = decoderStatus.codecStatus.ac4.dialogEnhanceMax;
            break;
        case BAVC_AudioCompressionStd_eDts:
        case BAVC_AudioCompressionStd_eDtshd:
        case BAVC_AudioCompressionStd_eDtsLegacy:
            pStatus->codecStatus.dts.amode = decoderStatus.codecStatus.dts.amode;
            pStatus->codecStatus.dts.pcmResolution = decoderStatus.codecStatus.dts.pcmResolution;
            pStatus->codecStatus.dts.copyHistory = decoderStatus.codecStatus.dts.copyHistory;
            pStatus->codecStatus.dts.extensionDescriptor = decoderStatus.codecStatus.dts.extensionDescriptor;
            pStatus->codecStatus.dts.bitRate = decoderStatus.codecStatus.dts.bitRate;
            pStatus->codecStatus.dts.version = decoderStatus.codecStatus.dts.version;
            pStatus->codecStatus.dts.esFormat = decoderStatus.codecStatus.dts.esFormat;
            pStatus->codecStatus.dts.lfe = decoderStatus.codecStatus.dts.lfe;
            pStatus->codecStatus.dts.extensionPresent = decoderStatus.codecStatus.dts.extensionPresent;
            pStatus->codecStatus.dts.crc = decoderStatus.codecStatus.dts.crc;
            pStatus->codecStatus.dts.hdcdFormat = decoderStatus.codecStatus.dts.hdcdFormat;
            pStatus->codecStatus.dts.drc = decoderStatus.codecStatus.dts.drc;
            pStatus->codecStatus.dts.downmixCoefficients = decoderStatus.codecStatus.dts.downmixCoefficients;
            pStatus->codecStatus.dts.neo = decoderStatus.codecStatus.dts.neo;
            pStatus->codecStatus.dts.frameSize = decoderStatus.codecStatus.dts.frameSize;
            pStatus->codecStatus.dts.numChannels = decoderStatus.codecStatus.dts.numChannels;
            pStatus->codecStatus.dts.pcmFrameSize = decoderStatus.codecStatus.dts.pcmFrameSize;
            pStatus->codecStatus.dts.numPcmBlocks = decoderStatus.codecStatus.dts.numPcmBlocks;
            break;
        case BAVC_AudioCompressionStd_eWmaStd:
        case BAVC_AudioCompressionStd_eWmaStdTs:
            pStatus->codecStatus.wma.bitRate = decoderStatus.codecStatus.wma.bitRate;
            pStatus->codecStatus.wma.original = decoderStatus.codecStatus.wma.original;
            pStatus->codecStatus.wma.copyright = decoderStatus.codecStatus.wma.copyright;
            pStatus->codecStatus.wma.crc = decoderStatus.codecStatus.wma.crc;
            pStatus->codecStatus.wma.stereo = decoderStatus.codecStatus.wma.channelMode == (BAPE_ChannelMode_e2_0)?true:false;
            pStatus->codecStatus.wma.version = decoderStatus.codecStatus.wma.version;
            break;
        case BAVC_AudioCompressionStd_eWmaPro:
            pStatus->codecStatus.wmaPro.bitRate = decoderStatus.codecStatus.wmaPro.bitRate;
            pStatus->codecStatus.wmaPro.original = decoderStatus.codecStatus.wmaPro.original;
            pStatus->codecStatus.wmaPro.copyright = decoderStatus.codecStatus.wmaPro.copyright;
            pStatus->codecStatus.wmaPro.crc = decoderStatus.codecStatus.wmaPro.crc;
            pStatus->codecStatus.wmaPro.lfe = decoderStatus.codecStatus.wmaPro.lfe;
            pStatus->codecStatus.wmaPro.version = decoderStatus.codecStatus.wmaPro.version;
            pStatus->codecStatus.wmaPro.stereoMode = decoderStatus.codecStatus.wmaPro.stereoMode;
            switch ( decoderStatus.codecStatus.wmaPro.channelMode )
            {
            case BAPE_ChannelMode_e1_0:
                pStatus->codecStatus.wmaPro.acmod = NEXUS_AudioWmaProAcmod_e1_0_C;
                break;
            case BAPE_ChannelMode_e2_0:
                pStatus->codecStatus.wmaPro.acmod = NEXUS_AudioWmaProAcmod_e2_0_LR;
                break;
            case BAPE_ChannelMode_e3_0:
                pStatus->codecStatus.wmaPro.acmod = NEXUS_AudioWmaProAcmod_e3_0_LCR;
                break;
            case BAPE_ChannelMode_e2_1:
                pStatus->codecStatus.wmaPro.acmod = NEXUS_AudioWmaProAcmod_e2_1_LRS;
                break;
            case BAPE_ChannelMode_e3_1:
                pStatus->codecStatus.wmaPro.acmod = NEXUS_AudioWmaProAcmod_e3_1_LCRS;
                break;
            case BAPE_ChannelMode_e2_2:
                pStatus->codecStatus.wmaPro.acmod = NEXUS_AudioWmaProAcmod_e2_2_LRLsRs;
                break;
            case BAPE_ChannelMode_e3_2:
                pStatus->codecStatus.wmaPro.acmod = NEXUS_AudioWmaProAcmod_e3_2_LCRLsRs;
                break;
            default:
                pStatus->codecStatus.wmaPro.acmod = NEXUS_AudioWmaProAcmod_eUndefined;
                break;
            }
            break;
        case BAVC_AudioCompressionStd_ePcmWav:
            pStatus->codecStatus.pcmWav.numChannels = decoderStatus.codecStatus.pcmWav.numChannels;
            break;
        case BAVC_AudioCompressionStd_eAmr:
            pStatus->codecStatus.amr.bitRate = decoderStatus.codecStatus.amr.bitRate;
            break;
        case BAVC_AudioCompressionStd_eDra:
            pStatus->codecStatus.dra.frameSize = decoderStatus.codecStatus.dra.frameSize;
            pStatus->codecStatus.dra.numBlocks = decoderStatus.codecStatus.dra.numBlocks;
            pStatus->codecStatus.dra.acmod = decoderStatus.codecStatus.dra.acmod;
            pStatus->codecStatus.dra.lfe = decoderStatus.codecStatus.dra.lfe;
            pStatus->codecStatus.dra.stereoMode = decoderStatus.codecStatus.dra.stereoMode;
            break;
        case BAVC_AudioCompressionStd_eCook:
            pStatus->codecStatus.cook.stereo = decoderStatus.codecStatus.cook.stereo;
            pStatus->codecStatus.cook.frameSize = decoderStatus.codecStatus.cook.frameSize;
            break;
        case BAVC_AudioCompressionStd_eAls:
        case BAVC_AudioCompressionStd_eAlsLoas:
            pStatus->codecStatus.als.bitsPerSample = decoderStatus.codecStatus.als.bitsPerSample;
            break;
        default:
            /* No specifics for this codec */
            break;
        }
    }
status_complete:
    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Get the Presentation info for specificed presentation.
Description:
    Query the Presentation Info for the indexed presentation.
    Use NEXUS_AudioDecoder_GetStatus() to retrieve the number of presentations, etc
See Also:
    NEXUS_AudioDecoder_GetStatus
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetPresentationStatus(
    NEXUS_AudioDecoderHandle handle,
    unsigned presentationIndex,
    NEXUS_AudioDecoderPresentationStatus *pStatus
    )
{
    BAPE_DecoderPresentationInfo piPresentationInfo;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pStatus);

    BDBG_CASSERT(BAPE_AC4_LANGUAGE_NAME_LENGTH == NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
    BDBG_CASSERT(BAPE_AC4_PRESENTATION_NAME_LENGTH == NEXUS_AUDIO_AC4_PRESENTATION_NAME_LENGTH);
    BDBG_CASSERT(NEXUS_AudioAc4AssociateType_eMax == (NEXUS_AudioAc4AssociateType)BAPE_Ac4AssociateType_eMax);
    BDBG_CASSERT(NEXUS_AudioAc4PresentationType_eMax == (NEXUS_AudioAc4PresentationType)BAPE_Ac4PresentationType_eMax);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_AudioDecoderPresentationStatus));

    BAPE_Decoder_GetPresentationInfo(handle->channel, presentationIndex, &piPresentationInfo);
    if ( piPresentationInfo.codec == BAVC_AudioCompressionStd_eMax )
    {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    pStatus->codec = NEXUS_Audio_P_MagnumToCodec(piPresentationInfo.codec);
    switch ( pStatus->codec )
    {
    default:
        pStatus->codec = NEXUS_AudioCodec_eMax;
        BDBG_WRN(("Presentation Status not supported for codec %d", pStatus->codec));
        break;
    case NEXUS_AudioCodec_eAc4:
        BKNI_Memcpy(pStatus->status.ac4.name, piPresentationInfo.info.ac4.name, sizeof(char)*NEXUS_AUDIO_AC4_PRESENTATION_NAME_LENGTH);
        BKNI_Memcpy(pStatus->status.ac4.language, piPresentationInfo.info.ac4.language, sizeof(char)*NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
        BKNI_Memcpy(pStatus->status.ac4.id, piPresentationInfo.info.ac4.id, sizeof(char)*NEXUS_AUDIO_AC4_PRESENTATION_ID_LENGTH);
        pStatus->status.ac4.index = piPresentationInfo.info.ac4.index;
        pStatus->status.ac4.associateType = (NEXUS_AudioAc4AssociateType)piPresentationInfo.info.ac4.associateType;
        pStatus->status.ac4.presentationType = (NEXUS_AudioAc4PresentationType)piPresentationInfo.info.ac4.presentationType;
        break;
    }

    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Get raw channel status information from the decoder

Description:
    When the decoder is connected to a digital input, this routine can
    return the raw channel status bit information from the input device.
    Currently, this applies to HDMI or SPDIF inputs only.  This routine
    will return an error if not connected to a digital input.

See Also:
    NEXUS_SpdifOutput_SetRawChannelStatus
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetRawChannelStatus(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioRawChannelStatus *pStatus   /* [out] current status */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Get an audio connector for use in the audio mixer
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_AudioDecoder_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioConnectorType type
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(type < NEXUS_AudioConnectorType_eMax);
    return &handle->connectors[type];
}

NEXUS_Error NEXUS_AudioDecoder_ApplySettings_priv(
    NEXUS_AudioDecoderHandle handle
    )
{
    BAPE_DecoderTsmSettings tsmSettings;
    BAPE_DecoderSettings decoderSettings, currentDecoderSettings;
    unsigned i,j;
    BERR_Code errCode;
    bool forceMute=false;
    bool mute = false;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    BAPE_Decoder_GetTsmSettings(handle->channel, &tsmSettings);
    tsmSettings.ptsOffset = handle->settings.ptsOffset;

#if NEXUS_HAS_SYNC_CHANNEL
    tsmSettings.ptsOffset += handle->sync.settings.delay;
 #endif

    /* default audio master mode off here but enable if astm or stc chan wants to after */
    tsmSettings.thresholds.syncLimit = 0;
    handle->stc.master = false;

#if NEXUS_HAS_ASTM
    if (handle->astm.settings.enableAstm)
    {
        BDBG_MSG(("ASTM is setting the sync limit for audio channel %p to %d", (void *)handle, handle->astm.settings.syncLimit));
        tsmSettings.thresholds.syncLimit = handle->astm.settings.syncLimit;
        BDBG_MSG(("ASTM is %s playback mode for audio channel %p", handle->astm.settings.enablePlayback ? "enabling" : "disabling", (void *)handle));
        tsmSettings.playback = handle->astm.settings.enablePlayback;
        tsmSettings.ptsOffset += handle->astm.settings.ptsOffset;
    }
    else
#endif
    {
        if (handle->programSettings.stcChannel)
        {
            NEXUS_StcChannelSettings stcSettings;
            NEXUS_StcChannel_GetSettings(handle->programSettings.stcChannel, &stcSettings);
            if (stcSettings.mode == NEXUS_StcChannelMode_eAuto && stcSettings.modeSettings.Auto.behavior == NEXUS_StcChannelAutoModeBehavior_eAudioMaster)
            {
                BDBG_MSG(("StcChannel is setting the sync limit for audio channel %p to %d", (void *)handle, 5000));
                tsmSettings.thresholds.syncLimit = 5000;
                handle->stc.master = true;
            }
        }
    }

    BDBG_MSG(("audio channel %p is stc %s, sync limit = %u", (void *)handle, handle->stc.master ? "master" : "slave", tsmSettings.thresholds.syncLimit));

    if ( 0 == handle->settings.discardThreshold )
    {
        tsmSettings.thresholds.discard = NEXUS_AUDIO_DECODER_P_GET_DISCARD_THRESHOLD(handle->isPlayback, handle->isDss);
    }
    else
    {
        tsmSettings.thresholds.discard = handle->settings.discardThreshold;
    }

    if ( 0 == handle->settings.gaThreshold )
    {
        if (!handle->settings.wideGaThreshold)
        {
#if NEXUS_HAS_SIMPLE_DECODER
            if (handle->simple.settings.gaThreshold)
            {
                /* this is for sync adjustment concealment
                 * it only takes effect if the user has not specified an explicit
                 * gaThreshold or wideGaThreshold */
                tsmSettings.thresholds.grossAdjustment = handle->simple.settings.gaThreshold;
            }
            else
#endif
            {
                NEXUS_PidChannelStatus pidChannelStatus;

                BKNI_Memset(&pidChannelStatus, 0, sizeof(pidChannelStatus));
                if ( NULL != handle->programSettings.pidChannel )
                {
                    (void)NEXUS_PidChannel_GetStatus(handle->programSettings.pidChannel, &pidChannelStatus);
                }
                tsmSettings.thresholds.grossAdjustment = (pidChannelStatus.originalTransportType == NEXUS_TransportType_eAvi) ? 0x30 : 0x8;
            }
        }
    }
    else
    {
        tsmSettings.thresholds.grossAdjustment = handle->settings.gaThreshold;
    }
    BAPE_Decoder_SetTsmSettings(handle->channel, &tsmSettings);

    NEXUS_AudioDecoder_P_SetTsm(handle);

    BAPE_Decoder_GetSettings(handle->channel, &decoderSettings);
    BKNI_Memcpy(&currentDecoderSettings, &decoderSettings, sizeof(currentDecoderSettings));
    switch ( handle->settings.outputLfeMode )
    {
    default:
        decoderSettings.outputLfe = true;
        break;
    case NEXUS_AudioDecoderOutputLfeMode_eOff:
        decoderSettings.outputLfe = false;
        break;
    }
    switch ( handle->settings.outputMode )
    {
    default:
    case NEXUS_AudioDecoderOutputMode_eAuto:
        if ( handle->openSettings.multichannelFormat == NEXUS_AudioMultichannelFormat_e7_1 )
        {
            decoderSettings.outputMode = BAPE_ChannelMode_e3_4;
        }
        else
        {
            decoderSettings.outputMode = BAPE_ChannelMode_e3_2;
        }
        break;
    case NEXUS_AudioDecoderOutputMode_e1_0:
        decoderSettings.outputMode = BAPE_ChannelMode_e1_0;
        break;
    case NEXUS_AudioDecoderOutputMode_e1_1:
        decoderSettings.outputMode = BAPE_ChannelMode_e1_1;
        break;
    case NEXUS_AudioDecoderOutputMode_e2_0:
        decoderSettings.outputMode = BAPE_ChannelMode_e2_0;
        break;
    case NEXUS_AudioDecoderOutputMode_e3_0:
        decoderSettings.outputMode = BAPE_ChannelMode_e3_0;
        break;
    case NEXUS_AudioDecoderOutputMode_e2_1:
        decoderSettings.outputMode = BAPE_ChannelMode_e2_1;
        break;
    case NEXUS_AudioDecoderOutputMode_e3_1:
        decoderSettings.outputMode = BAPE_ChannelMode_e3_1;
        break;
    case NEXUS_AudioDecoderOutputMode_e2_2:
        decoderSettings.outputMode = BAPE_ChannelMode_e2_2;
        break;
    case NEXUS_AudioDecoderOutputMode_e3_2:
        decoderSettings.outputMode = BAPE_ChannelMode_e3_2;
        break;
    case NEXUS_AudioDecoderOutputMode_e3_4:
        decoderSettings.outputMode = BAPE_ChannelMode_e3_4;
        break;
    }
    switch ( handle->settings.dualMonoMode )
    {
    default:
    case NEXUS_AudioDecoderDualMonoMode_eStereo:
        decoderSettings.dualMonoMode = BAPE_DualMonoMode_eStereo;
        break;
    case NEXUS_AudioDecoderDualMonoMode_eLeft:
        decoderSettings.dualMonoMode = BAPE_DualMonoMode_eLeft;
        break;
    case NEXUS_AudioDecoderDualMonoMode_eRight:
        decoderSettings.dualMonoMode = BAPE_DualMonoMode_eRight;
        break;
    case NEXUS_AudioDecoderDualMonoMode_eMix:
        decoderSettings.dualMonoMode = BAPE_DualMonoMode_eMix;
        break;
    }
    switch ( handle->openSettings.multichannelFormat )
    {
    default:
    case NEXUS_AudioMultichannelFormat_e5_1:
        decoderSettings.multichannelFormat = BAPE_MultichannelFormat_e5_1;
        break;
    case NEXUS_AudioMultichannelFormat_e7_1:
        decoderSettings.multichannelFormat = BAPE_MultichannelFormat_e7_1;
        break;
    }
    if ( handle->trickState.rate == 0 )
    {
        decoderSettings.decodeRate = BAPE_NORMAL_DECODE_RATE;
    }
    else if ( handle->trickState.rate > (2*NEXUS_NORMAL_DECODE_RATE) || handle->trickState.rate < (NEXUS_NORMAL_DECODE_RATE/2) )
    {
        BDBG_MSG(("Audio trick modes are only supported between 0.5x and 2x normal rate.  Muting audio decoder."));
        forceMute = true;
        decoderSettings.decodeRate = BAPE_NORMAL_DECODE_RATE;
    }
    else
    {
        decoderSettings.decodeRate = (handle->trickState.rate * BAPE_NORMAL_DECODE_RATE)/ NEXUS_NORMAL_DECODE_RATE;
    }
    decoderSettings.loudnessEquivalenceEnabled = handle->settings.loudnessEquivalenceEnabled;
    decoderSettings.ancillaryDataEnabled = handle->settings.ancillaryDataEnabled;
    decoderSettings.karaokeSettings.vocalSuppressionLevel = handle->settings.karaokeSettings.vocalSuppressionLevel;
    decoderSettings.karaokeSettings.vocalSuppressionFrequency = handle->settings.karaokeSettings.vocalSuppressionFrequency;
    decoderSettings.karaokeSettings.outputMakeupBoost = handle->settings.karaokeSettings.outputMakeupBoost;

    if (BKNI_Memcmp(&decoderSettings, &currentDecoderSettings, sizeof(decoderSettings)) != 0) {
        errCode = BAPE_Decoder_SetSettings(handle->channel, &decoderSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    mute = handle->settings.muted || handle->trickState.muted
#if NEXUS_HAS_SYNC_CHANNEL
        /* TODO: if we are not running, or if we are running already muted, and sync wants to keep it muted, fine;
        but if we are already running, have unmuted, and sync wants to re-mute, don't! */
        || handle->sync.mute
#endif
        || forceMute;

    if(handle->outputLists[NEXUS_AudioConnectorType_eCompressed].outputs[0] ||
       handle->outputLists[NEXUS_AudioConnectorType_eCompressed4x].outputs[0] ||
       handle->outputLists[NEXUS_AudioConnectorType_eCompressed16x].outputs[0])
    {
        bool compressedMute = false;
        if (handle->trickState.rate != NEXUS_NORMAL_DECODE_RATE) {
            compressedMute = true;
        }
        errCode = NEXUS_AudioDecoder_P_SetCompressedMute(handle, mute || compressedMute);
        if (errCode) {
            BERR_TRACE(errCode);
        }
    }
    /* Check if there is an AC3/DTS encoder attached to decoder and if so mute the outputs */
    NEXUS_AudioDecoder_P_SetCompressedEncoderMute(handle, mute);

    /* Apply volume settings - but only if an output has ever been connected by the user.
       Otherwise this forces a Shutdown() call in the application. */
    if ( handle->connectors[NEXUS_AudioConnectorType_eStereo].pMixerData )
    {
        NEXUS_AudioInput_P_GetVolume(&handle->connectors[NEXUS_AudioConnectorType_eStereo], &handle->volume.current);
    }
    else if ( handle->connectors[NEXUS_AudioConnectorType_eAlternateStereo].pMixerData )
    {
        NEXUS_AudioInput_P_GetVolume(&handle->connectors[NEXUS_AudioConnectorType_eAlternateStereo], &handle->volume.current);
    }
    else if ( handle->connectors[NEXUS_AudioConnectorType_eMultichannel].pMixerData )
    {
        NEXUS_AudioInput_P_GetVolume(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel], &handle->volume.current);
    }
    else if ( handle->connectors[NEXUS_AudioConnectorType_eMono].pMixerData )
    {
        NEXUS_AudioInput_P_GetVolume(&handle->connectors[NEXUS_AudioConnectorType_eMono], &handle->volume.current);
    }
    else
    {
        /* Compressed outputs do not have volume control */
        goto skip_volume;
    }

    BKNI_Memcpy(&handle->volume.next, &handle->volume.current, sizeof(handle->volume.current));

    /* NEXUS and APE use opposite volume matrix layouts. Swap the matrix here */
    for ( i = 0; i < NEXUS_AudioChannel_eMax; i++ )
    {
        for ( j = 0; j < NEXUS_AudioChannel_eMax; j++ )
        {
            handle->volume.next.coefficients[i][j] = handle->settings.volumeMatrix[j][i];
        }
    }

    handle->volume.next.muted  = mute;

    if (BKNI_Memcmp(&handle->volume.next, &handle->volume.current, sizeof(handle->volume.next)) == 0)
    {
        /* volume hasn't changed */
        goto skip_volume;
    }

    if ( handle->connectors[NEXUS_AudioConnectorType_eStereo].pMixerData )
    {
        errCode = NEXUS_AudioInput_P_SetVolume(&handle->connectors[NEXUS_AudioConnectorType_eStereo], &handle->volume.next);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    if ( handle->connectors[NEXUS_AudioConnectorType_eAlternateStereo].pMixerData )
    {
        errCode = NEXUS_AudioInput_P_SetVolume(&handle->connectors[NEXUS_AudioConnectorType_eAlternateStereo], &handle->volume.next);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    if ( handle->connectors[NEXUS_AudioConnectorType_eMultichannel].pMixerData )
    {
        errCode = NEXUS_AudioInput_P_SetVolume(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel], &handle->volume.next);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    if ( handle->connectors[NEXUS_AudioConnectorType_eMono].pMixerData )
    {
        errCode = NEXUS_AudioInput_P_SetVolume(&handle->connectors[NEXUS_AudioConnectorType_eMono], &handle->volume.next);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
skip_volume:
    return BERR_SUCCESS;
}

#if NEXUS_HAS_ASTM
void NEXUS_AudioDecoder_GetAstmSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    NEXUS_AudioDecoderAstmSettings * pAstmSettings  /* [out] */
    )
{
    NEXUS_ASSERT_MODULE();

    *pAstmSettings = audioDecoder->astm.settings;
}

NEXUS_Error NEXUS_AudioDecoder_SetAstmSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    const NEXUS_AudioDecoderAstmSettings * pAstmSettings
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_ASSERT_MODULE();

    /* copy settings as-is, this way ASTM will always get what it set */
    audioDecoder->astm.settings = *pAstmSettings;

    /* if ASTM is internally permitted, apply settings */
    if (audioDecoder->running)
    {
        rc = NEXUS_AudioDecoder_ApplySettings_priv(audioDecoder);
    }

    return rc;
}

NEXUS_Error NEXUS_AudioDecoder_GetAstmStatus_isr(
    NEXUS_AudioDecoderHandle audioDecoder,
    NEXUS_AudioDecoderAstmStatus * pAstmStatus  /* [out] */
    )
{
    BAPE_DecoderTsmStatus tsmStatus;
    BERR_Code rc;

    BKNI_ASSERT_ISR_CONTEXT();

    rc = BAPE_Decoder_GetTsmStatus_isr(audioDecoder->channel, &tsmStatus);
    if (rc != NEXUS_SUCCESS) {
        audioDecoder->astm.status.ptsStcDiff = 0;
        rc = NEXUS_SUCCESS;
    }
    else {
        audioDecoder->astm.status.ptsStcDiff = tsmStatus.ptsStcDifference;
    }

    *pAstmStatus = audioDecoder->astm.status;
    return rc;
}
#endif

static NEXUS_Error NEXUS_AudioDecoder_P_SetCompressedMute(NEXUS_AudioDecoderHandle decoder, bool muted)
{
    NEXUS_AudioOutputHandle output;
    NEXUS_Error rc;
    int i;

    for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
    {
        output = decoder->outputLists[NEXUS_AudioConnectorType_eCompressed].outputs[i];
        if ( output )
        {
            rc = NEXUS_AudioOutput_P_SetCompressedMute(output, muted);
            if ( rc )
            {
                return BERR_TRACE(rc);
            }
        }

        output = decoder->outputLists[NEXUS_AudioConnectorType_eCompressed4x].outputs[i];
        if ( output )
        {
            rc = NEXUS_AudioOutput_P_SetCompressedMute(output, muted);
            if ( rc )
            {
                return BERR_TRACE(rc);
            }
        }

        output = decoder->outputLists[NEXUS_AudioConnectorType_eCompressed16x].outputs[i];
        if ( output )
        {
            rc = NEXUS_AudioOutput_P_SetCompressedMute(output, muted);
            if ( rc )
            {
                return BERR_TRACE(rc);
            }
        }
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_AudioDecoder_P_SetCompressedEncoderMute(NEXUS_AudioDecoderHandle handle, bool mute)
{
    NEXUS_AudioOutputHandle output;
    NEXUS_Error rc;
    int i;

    /* If encoding to output, encoder will be directly attached */
    for (i=0; i < NEXUS_AudioDecoderConnectorType_eMax; i++)
    {
        NEXUS_AudioInputHandle connector = NEXUS_AudioInput_P_LocateDownstream(&handle->connectors[i], NULL);
        while (connector)
        {
            if (connector->objectType == NEXUS_AudioInputType_eEncoder)
            {
                NEXUS_AudioEncoderSettings encoderSettings;

                NEXUS_AudioEncoder_GetSettings( connector->pObjectHandle, &encoderSettings);
                if (encoderSettings.codec == NEXUS_AudioCodec_eAc3 || encoderSettings.codec == NEXUS_AudioCodec_eDts)
                {
                    output = NEXUS_AudioInput_P_LocateOutput(connector, NULL);
                    while ( output )
                    {
                        rc = NEXUS_AudioOutput_P_SetCompressedMute(output, mute);
                        if ( rc )
                        {
                            return BERR_TRACE(rc);
                        }
                        output = NEXUS_AudioInput_P_LocateOutput(connector, output);
                    }
                }
            }
            connector = NEXUS_AudioInput_P_LocateDownstream(&handle->connectors[i], connector);
        }
    }
    return BERR_SUCCESS;
}

#if NEXUS_HAS_SYNC_CHANNEL
static NEXUS_Error NEXUS_AudioDecoder_P_SetSyncMute(NEXUS_AudioDecoderHandle decoder, bool muted)
{
    bool wasMuted = false;

    /* if sync called this while decoder was not running, it was to set persistent startup behavior */
    if (!decoder->running)
    {
        BDBG_MSG(("SetSyncMute(%u): start muted = %d", decoder->index, muted));
        decoder->sync.startMuted = muted;
    }

    wasMuted = decoder->sync.mute;
    decoder->sync.mute = muted;

    /* if we are running, and unmuting */
    if (decoder->running && wasMuted && !muted)
    {
        /* the ApplySettings call is in SetSyncSettings_priv now */

        BDBG_MSG(("SetSyncMute(%u): unmute", decoder->index));

/* TODO */
#if 0
#if B_HAS_TRC
        if (decoder->index == 0) {
            BTRC_TRACE(ChnChange_SyncUnmuteAudio, STOP);
            if (BTRC_MODULE_HANDLE(ChnChange_DecodeStopAudio)->stats[0].count) {
                BTRC_TRACE(ChnChange_Total_Audio, STOP);
                BKNI_SetEvent(decoder->channelChangeReportEvent);
            }
        }
#endif
#endif
    }

    return NEXUS_SUCCESS;
}

void NEXUS_AudioDecoder_GetSyncSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    NEXUS_AudioInputSyncSettings *pSyncSettings  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(audioDecoder, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSyncSettings);
    NEXUS_ASSERT_MODULE();

    *pSyncSettings = audioDecoder->sync.settings;
}

NEXUS_Error NEXUS_AudioDecoder_SetSyncSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    const NEXUS_AudioInputSyncSettings *pSyncSettings
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDBG_OBJECT_ASSERT(audioDecoder, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSyncSettings);

    NEXUS_ASSERT_MODULE();

    audioDecoder->sync.settings = *pSyncSettings;

    rc = NEXUS_AudioDecoder_P_SetSyncMute(audioDecoder, audioDecoder->sync.settings.mute);
    if (rc) BERR_TRACE(rc);

    /* here we must apply settings, because pts offset and mute are both handled there */
    if (audioDecoder->running)
    {
        rc = NEXUS_AudioDecoder_ApplySettings_priv(audioDecoder);
        if (rc) BERR_TRACE(rc);
    }

    return rc;
}

NEXUS_Error NEXUS_AudioDecoder_GetSyncStatus_isr(
    NEXUS_AudioDecoderHandle audioDecoder,
    NEXUS_AudioInputSyncStatus *pSyncStatus  /* [out] */
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(audioDecoder, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pSyncStatus);
    BKNI_ASSERT_ISR_CONTEXT();
    BKNI_Memset(pSyncStatus, 0, sizeof(*pSyncStatus));
    pSyncStatus->started = audioDecoder->sync.started;
    pSyncStatus->digital = true;
    pSyncStatus->dsolaEnabled = audioDecoder->apeStartSettings.decodeRateControl;
    errCode = BAPE_Decoder_GetPathDelay_isr(audioDecoder->channel, &pSyncStatus->delay);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* added for disabling Sarnoff offset by default */
    if (pSyncStatus->started)
    {
        pSyncStatus->nonRealTime = audioDecoder->programSettings.nonRealTime;
    }
    else
    {
        pSyncStatus->nonRealTime = false;
    }

    /*
     * for 64 sample FMM delay -> this gets us within 0 - 1 ms, since the FMM
     * delay really depends on Fs. 32K -> 2 ms, 44.1K -> 1.45 ms, 48K -> 1.33 ms
     *
     * NRT doesn't have an FMM involved, so it shouldn't have this delay.
     */
    if (!pSyncStatus->nonRealTime)
    {
        pSyncStatus->delay += 1;
    }
    else
    {
        /* NRT always enforces a zero delay, no matter what the decoder reports */
        pSyncStatus->delay = 0;
    }

    return BERR_SUCCESS;
}
#endif

bool NEXUS_AudioDecoder_P_IsRunning(NEXUS_AudioDecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    /* This is slightly dangerous with trick modes, because a trick mute may cause this to return false.
       But, it's required for HDMI/SPDIF inputs where the decoder actually "starts stopped".  */
    return handle->running;
}

void NEXUS_AudioDecoder_P_Reset(void)
{
    unsigned i;
    NEXUS_AudioCapabilities audioCapabilities;
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    /* Process watchdog STOP */
    BAPE_ProcessWatchdogInterruptStop(NEXUS_AUDIO_DEVICE_HANDLE);

    for ( i = 0; i < audioCapabilities.numDecoders; i++ )
    {
        if ( g_decoders[i] && g_decoders[i]->running && NULL == g_decoders[i]->programSettings.input )
        {
            NEXUS_AudioDecoder_P_DisableRaveContexts(g_decoders[i], true);
        }
    }

    /* Reset AudioMuxOutput objects before restarting the decoders */
    NEXUS_AudioMuxOutput_P_WatchdogReset();

    /* Process watchdog RESUME */
    BAPE_ProcessWatchdogInterruptResume(NEXUS_AUDIO_DEVICE_HANDLE);

#if NEXUS_AUDIO_PAK_SUPPORT
    /* Reload PAK data */
    NEXUS_AudioDecoder_P_LoadPak();
#endif

    /* Restart RAVE contexts */
    for ( i = 0; i < audioCapabilities.numDecoders; i++ )
    {
        if ( g_decoders[i] && g_decoders[i]->running && NULL == g_decoders[i]->programSettings.input )
        {
            NEXUS_Error rc;
            rc = NEXUS_AudioDecoder_P_EnableRaveContexts(g_decoders[i], NULL);
            if ( rc )
            {
                BDBG_ERR(("Unable to re-start Audio Rave contexts."));
                BERR_TRACE(rc);
            }
        }
    }

#if NEXUS_HAS_ASTM
    for ( i = 0; i < audioCapabilities.numDecoders; i++ )
    {
        if ( g_decoders[i] && g_decoders[i]->running && g_decoders[i]->astm.settings.enableAstm )
        {
            NEXUS_Callback astm_watchdog_isr = g_decoders[i]->astm.settings.watchdog_isr;
            BDBG_MSG(("Audio channel %p is notifying ASTM of its watchdog recovery", (void *)g_decoders[i]));
            if (astm_watchdog_isr)
            {
                BKNI_EnterCriticalSection();
                astm_watchdog_isr(g_decoders[i]->astm.settings.callbackContext, 0);
                BKNI_LeaveCriticalSection();
            }
        }
    }
#endif
}

static void NEXUS_AudioDecoder_P_Watchdog(void *pParam)
{
    bool corePending=false;

    BSTD_UNUSED(pParam);

    g_numWatchdogs++;

    #if BAPE_DSP_SUPPORT

    #if BDBG_DEBUG_BUILD
    if ( NEXUS_GetEnv("assert_audio_watchdog") )
    {
        BDBG_ERR(("Intentionally generating an assert due to a watchdog interrupt for debug testing."));
        BDBG_ASSERT(0);
    }
    #endif

    /* Check if core dump support is enabled.  If so, spin and wait for it to complete. */
    if ( g_NEXUS_audioModuleData.settings.dspDebugSettings.typeSettings[NEXUS_AudioDspDebugType_eCoreDump].enabled )
    {
        unsigned i;
        for ( i = 0; i < BAPE_DEVICE_TYPE_MAX; i++ )
        {
            BDSP_Handle hDsp = NULL;
            if ( i == BAPE_DEVICE_TYPE_DSP )
            {
                hDsp = g_NEXUS_audioModuleData.dspHandle;
            }
            else if ( i == BAPE_DEVICE_TYPE_ARM )
            {
                hDsp = g_NEXUS_audioModuleData.armHandle;
            }

            if ( hDsp )
            {
                /* Poll for core dump to finish and inform application */
                unsigned retries = 100;

                while ( BDSP_GetCoreDumpStatus(hDsp, 0) == BDSP_FwStatus_eCoreDumpInProgress )
                {
                    BKNI_Sleep(1);
                    if ( 0 == --retries )
                    {
                        break;
                    }
                }

                if ( BDSP_GetCoreDumpStatus(hDsp, 0) == BDSP_FwStatus_eCoreDumpComplete )
                {
                    corePending = true;
                }
            }
        }
    }
    #endif

    if ( corePending )
    {
        BDBG_WRN(("Audio watchdog reset postponed for core dump retrieval"));
        g_NEXUS_audioModuleData.watchdogDeferred = true;
    }
    else if ( !NEXUS_GetEnv("no_watchdog") )
    {
        /* Just restart now */
        NEXUS_AudioDecoder_P_Reset();
    }
    else
    {
        BDBG_WRN(("Watchdog Processing Disabled"));
    }
}

static void NEXUS_AudioDecoder_P_SampleRate(void *pParam)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam;

    if(pDecoder->settings.wideGaThreshold) {
        BAPE_DecoderTsmStatus tsmStatus;
        BAPE_DecoderTsmSettings tsmSettings;
        uint32_t new_threshold;

        BAPE_Decoder_GetTsmStatus(pDecoder->channel, &tsmStatus);

        new_threshold = (tsmStatus.lastFrameLength*3)/2;

        BAPE_Decoder_GetTsmSettings(pDecoder->channel, &tsmSettings);

        BDBG_WRN(("Adjusting GA threshold to: %d (was %d)", new_threshold, tsmSettings.thresholds.grossAdjustment));

        tsmSettings.thresholds.grossAdjustment = new_threshold;

        (void)BAPE_Decoder_SetTsmSettings(pDecoder->channel, &tsmSettings);
    }

    BDBG_MSG(("Sample Rate decoder[%d]", pDecoder->index));
}

static void NEXUS_AudioDecoder_P_ChannelChangeReport(void * context)
{
    BSTD_UNUSED(context);
}

static void NEXUS_AudioDecoder_P_FirstPts_isr(void *pParam1, int param2, const BAPE_DecoderTsmStatus *pTsmStatus)
{
    NEXUS_Error errCode;
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;

    BSTD_UNUSED(param2);

    BDBG_MSG(("audio[%d] First PTS %08x", pDecoder->index, pTsmStatus->ptsInfo.ui32CurrentPTS));
    if ( pDecoder->programSettings.stcChannel && pDecoder->stc.connector)
    {
        NEXUS_IsrCallback_Fire_isr(pDecoder->firstPtsCallback);
        errCode = NEXUS_StcChannel_RequestStc_isr(pDecoder->stc.connector, &pTsmStatus->ptsInfo);
        if ( errCode )
        {
            errCode=BERR_TRACE(errCode);
        }
    }
#if NEXUS_HAS_ASTM
    if (pDecoder->astm.settings.enableAstm)
    {
        pDecoder->astm.status.pts = pTsmStatus->ptsInfo.ui32CurrentPTS;

        if (pDecoder->astm.settings.firstPts_isr)
        {
            pDecoder->astm.settings.firstPts_isr(pDecoder->astm.settings.callbackContext, 0);
        }
    }
#endif /* NEXUS_HAS_ASTM */
}

static void NEXUS_AudioDecoder_P_AudioTsmFail_isr(void *pParam1, int param2, const BAPE_DecoderTsmStatus *pTsmStatus)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BAPE_DecoderTsmStatus tsmStatus;
    NEXUS_Error errCode;
    uint32_t stc;

    BSTD_UNUSED(param2);
    BDBG_ASSERT(NULL != pTsmStatus);

    BDBG_MSG_TRACE(("audio[%d] Tsm fail: PTS 0x%08x, type %d",
        pDecoder->index, pTsmStatus->ptsInfo.ui32CurrentPTS, pTsmStatus->ptsInfo.ePTSType));

    NEXUS_StcChannel_GetStc_isr(pDecoder->programSettings.stcChannel, &stc);

    /* If we're in a non-standard STC trick mode */
    if ( pDecoder->trickState.stcTrickEnabled && (pDecoder->trickState.rate < 500 || pDecoder->trickState.rate > 2000) )
    {
        /* in STC trick mode, PTS might lag the STC because of decoder drop algorithm. don't reset STC in this case. */
        if ( stc > pTsmStatus->ptsInfo.ui32CurrentPTS && stc - pTsmStatus->ptsInfo.ui32CurrentPTS < 45000 * 8 )
        {
            return;
        }
    }

    BDBG_MSG_TRACE(("pts2stcphase: %d", pTsmStatus->ptsStcDifference));

    if (pDecoder->stc.master
#if NEXUS_HAS_ASTM
        || (pDecoder->astm.settings.enableAstm && pDecoder->astm.settings.syncLimit > 0)
#endif /* NEXUS_HAS_ASTM */
    )
    {
        tsmStatus = *pTsmStatus;
        if
        (
            ((tsmStatus.ptsStcDifference >= 0) && (tsmStatus.ptsStcDifference < 128 * 45))
            ||
            ((tsmStatus.ptsStcDifference < 0) && (tsmStatus.ptsStcDifference > -128 * 45))
        )
        {
            tsmStatus.ptsInfo.ui32CurrentPTS = (uint32_t)((int32_t)stc - tsmStatus.ptsStcDifference);
        }
        pTsmStatus = &tsmStatus;
    }

    /* PR:52308 ignore PTS errors for non-XPT inputs - we can't do anything about them from stcchannel/pcrlib anyway */
    if (!pDecoder->programSettings.input)
    {
        NEXUS_IsrCallback_Fire_isr(pDecoder->ptsErrorCallback);
        if (pDecoder->stc.connector)
        {
            errCode = NEXUS_StcChannel_PtsError_isr(pDecoder->stc.connector, &pTsmStatus->ptsInfo);
            if (errCode)
            {
                errCode=BERR_TRACE(errCode);
                /* keep going */
            }
        }
    }

#if NEXUS_HAS_ASTM
    if (pDecoder->astm.settings.enableAstm)
    {
        pDecoder->astm.status.pts = pTsmStatus->ptsInfo.ui32CurrentPTS;

        if (pDecoder->astm.settings.tsmFail_isr)
        {
            pDecoder->astm.settings.tsmFail_isr(pDecoder->astm.settings.callbackContext, 0);
        }
    }
#endif /* NEXUS_HAS_ASTM */

    pDecoder->ptsErrorCount++;
}

static void NEXUS_AudioDecoder_P_AudioTsmPass_isr(void *pParam1, int param2, const BAPE_DecoderTsmStatus *pTsmStatus)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;

    BSTD_UNUSED(param2);
    BSTD_UNUSED(pTsmStatus);
    BDBG_ASSERT(NULL != pTsmStatus);

    BDBG_MSG(("audio[%d] TSM pass: PTS 0x%08x, type %d",
        pDecoder->index, pTsmStatus->ptsInfo.ui32CurrentPTS, pTsmStatus->ptsInfo.ePTSType));

#if NEXUS_HAS_ASTM
    if (pDecoder->astm.settings.enableAstm)
    {
        pDecoder->astm.status.pts = pTsmStatus->ptsInfo.ui32CurrentPTS;

        if (pDecoder->astm.settings.tsmPass_isr)
        {
            pDecoder->astm.settings.tsmPass_isr(pDecoder->astm.settings.callbackContext, 0);
        }
    }
#endif
}

static void NEXUS_AudioDecoder_P_SampleRateChange_isr(void *pParam1, int param2, unsigned sampleRate)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;

    BSTD_UNUSED(param2);

    BDBG_MSG(("audio[%d] Sampling Rate Info, samplingRate=%u", pDecoder->index, sampleRate));

    BKNI_SetEvent_isr(pDecoder->sampleRateEvent);
    NEXUS_IsrCallback_Fire_isr(pDecoder->sourceChangeAppCallback);

#if NEXUS_HAS_SYNC_CHANNEL
    if (pDecoder->sync.settings.sampleRateCallback_isr)
    {
        (*pDecoder->sync.settings.sampleRateCallback_isr)(pDecoder->sync.settings.callbackContext, 0);
    }
#endif
}

static void NEXUS_AudioDecoder_P_Lock_isr(void *pParam1, int param2)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;

    pDecoder->locked = param2;
    BDBG_MSG(("Decoder %u %s", pDecoder->index, param2?"lock":"unlock"));
    NEXUS_IsrCallback_Fire_isr(pDecoder->lockCallback);
}

static void NEXUS_AudioDecoder_P_StreamStatusAvailable_isr(void *pParam1, int param2)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BSTD_UNUSED(param2);

    BDBG_MSG(("Decoder %u Stream Status Ready", pDecoder->index));
    NEXUS_IsrCallback_Fire_isr(pDecoder->streamStatusCallback);
    /*NEXUS_IsrCallback_Fire_isr(pDecoder->sourceChangeAppCallback);*/
}

static void NEXUS_AudioDecoder_P_AncillaryData_isr(void *pParam1, int param2)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BSTD_UNUSED(param2);

    BDBG_MSG(("Decoder %u Ancillary Data Ready", pDecoder->index));
    NEXUS_IsrCallback_Fire_isr(pDecoder->ancillaryDataCallback);
}

static void NEXUS_AudioDecoder_P_StreamParameterChanged_isr(void *pParam1, int param2)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BSTD_UNUSED(param2);

    BDBG_MSG(("Decoder %u Stream Parameter Changed", pDecoder->index));
    NEXUS_IsrCallback_Fire_isr(pDecoder->sourceChangeAppCallback);
}

static void NEXUS_AudioDecoder_P_CdbItbOverflow_isr(void *pParam1, int param2)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BSTD_UNUSED(param2);

    BDBG_MSG(("Decoder %u CDB/ITB overflow", pDecoder->index));
    pDecoder->numFifoOverflows++;
    NEXUS_IsrCallback_Fire_isr(pDecoder->fifoOverflowCallback);
}

static void NEXUS_AudioDecoder_P_CdbItbUnderflow_isr(void *pParam1, int param2)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BSTD_UNUSED(param2);

    BDBG_MSG(("Decoder %u CDB/ITB underflow", pDecoder->index));
    pDecoder->numFifoUnderflows++;
    NEXUS_IsrCallback_Fire_isr(pDecoder->fifoUnderflowCallback);
}

static void NEXUS_AudioDecoder_P_DialnormChanged_isr(void *pParam1, int param2)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BSTD_UNUSED(param2);

    BDBG_MSG(("Decoder %u Dialog Norm Change", pDecoder->index));
    NEXUS_IsrCallback_Fire_isr(pDecoder->dialnormChangedCallback);
}

static void NEXUS_AudioDecoder_P_UnlicensedAlgo_isr(void *pParam1, int param2)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam1;
    BSTD_UNUSED(param2);
    BKNI_SetEvent_isr(pDecoder->unlicensedAlgoEvent);

}

static void NEXUS_AudioDecoder_P_UnlicensedAlgo(void *pParam)
{
    NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)pParam;
    BAPE_DecoderStatus decoderStatus;

    BAPE_Decoder_GetStatus(pDecoder->channel, &decoderStatus);
    BDBG_ERR(("*********************************************************************"));
    BDBG_ERR(("Unlicensed Algo (%s) detected on decoder %d .  Audio will be muted.",
              decoderStatus.codecName, pDecoder->index));
    BDBG_ERR(("*********************************************************************"));
}

void NEXUS_AudioDecoder_P_SetTsm(NEXUS_AudioDecoderHandle handle)
{
    BAPE_DecoderTsmSettings tsmSettings;

    BAPE_Decoder_GetTsmSettings(handle->channel, &tsmSettings);
    tsmSettings.tsmEnabled = false;

    if ( handle->tsmPermitted )
    {
        if ( handle->trickState.tsmEnabled )
        {
#if NEXUS_HAS_ASTM
            if ( handle->astm.settings.enableAstm )
            {
                BDBG_MSG_TRACE(("ASTM is %s TSM for audio channel %p", handle->astm.settings.enableTsm ? "enabling" : "disabling", (void *)handle));
                if ( handle->astm.settings.enableTsm )
                {
                    tsmSettings.tsmEnabled = true;
                }
            }
            else
#endif
            {
                tsmSettings.tsmEnabled = true;
            }
        }
    }

    BDBG_MSG_TRACE(("%s TSM",tsmSettings.tsmEnabled?"Enabling":"Disabling"));

#if NEXUS_HAS_ASTM
    /* Only allow ASTM if we have TSM enabled for all non-ASTM controls and we have a TS source */
    if ( handle->tsmPermitted && handle->trickState.tsmEnabled &&
         handle->apeStartSettings.streamType == BAVC_StreamType_eTsMpeg )
    {
        BDBG_MSG_TRACE(("%s ASTM mode for audio channel %p",handle->astm.settings.enableAstm?"Enabling":"Disabling", (void *)handle));
        tsmSettings.astmEnabled = handle->astm.settings.enableAstm;
    }
    else
    {
        BDBG_MSG_TRACE(("Disabling ASTM mode for audio channel %p", (void *)handle));
        tsmSettings.astmEnabled = false;
    }
#endif

    BAPE_Decoder_SetTsmSettings(handle->channel, &tsmSettings);

    handle->tsmEnabled = tsmSettings.tsmEnabled;
}

static void NEXUS_AudioDecoder_P_StopStcChannel(NEXUS_AudioDecoderHandle handle)
{
    LOCK_TRANSPORT();
    if (handle->programSettings.pidChannel)
    {
        NEXUS_StcChannel_DisablePidChannel_priv(handle->programSettings.stcChannel, handle->programSettings.pidChannel);
    }
    if (handle->stc.connector)
    {
        NEXUS_StcChannel_DisconnectDecoder_priv(handle->stc.connector);
        handle->stc.connector = NULL;
    }
    UNLOCK_TRANSPORT();
}

static NEXUS_Error NEXUS_AudioDecoder_P_StartStcChannel(NEXUS_AudioDecoderHandle handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelDecoderConnectionSettings stcChannelConnectionSettings;
    NEXUS_AudioDecoderStartSettings * pProgram;

    pProgram = &handle->programSettings;

    LOCK_TRANSPORT();

    NEXUS_StcChannel_GetDefaultDecoderConnectionSettings_priv(&stcChannelConnectionSettings);
    stcChannelConnectionSettings.type = NEXUS_StcChannelDecoderType_eAudio;
    stcChannelConnectionSettings.priority = handle->stc.priority;
    stcChannelConnectionSettings.getPts_isr = NEXUS_AudioDecoder_P_GetPtsCallback_isr;
    stcChannelConnectionSettings.getCdbLevel_isr = NEXUS_AudioDecoder_P_GetCdbLevelCallback_isr;
    stcChannelConnectionSettings.stcValid_isr = NEXUS_AudioDecoder_P_StcValidCallback_isr;
    stcChannelConnectionSettings.pCallbackContext = handle;
    if (pProgram->nonRealTime)
    {
        stcChannelConnectionSettings.getPcrOffset_isr = NEXUS_AudioDecoder_P_GetPcrOffset_isr;
        stcChannelConnectionSettings.setPcrOffset_isr = NEXUS_AudioDecoder_P_SetPcrOffset_isr;
    }
    stcChannelConnectionSettings.statusUpdateEvent = handle->stc.statusChangeEvent;
    handle->stc.connector = NEXUS_StcChannel_ConnectDecoder_priv(pProgram->stcChannel, &stcChannelConnectionSettings);
    if (!handle->stc.connector) { UNLOCK_TRANSPORT(); rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err; }
    if (pProgram->pidChannel)
    {
        NEXUS_StcChannel_EnablePidChannel_priv(pProgram->stcChannel, pProgram->pidChannel);
    }

    UNLOCK_TRANSPORT();

err:
    return rc;
}

static void NEXUS_AudioDecoder_P_ApplyDefaultInputVolume(NEXUS_AudioInputHandle input)
{
    BAPE_MixerInputVolume volume;
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_AudioInput_P_GetVolume(input, &volume);
    rc = NEXUS_AudioInput_P_SetVolume(input, &volume);
    if (rc)
    {
        BDBG_ERR(("Error applying default input volume"));
    }
}

static void NEXUS_AudioDecoder_P_SetDefaultVolume(NEXUS_AudioDecoderHandle handle)
{

    if ( handle->connectors[NEXUS_AudioConnectorType_eStereo].pMixerData )
    {
        NEXUS_AudioInput_P_GetDefaultInputVolume(&handle->connectors[NEXUS_AudioConnectorType_eStereo]);
        NEXUS_AudioDecoder_P_ApplyDefaultInputVolume(&handle->connectors[NEXUS_AudioConnectorType_eStereo]);
    }
    if ( handle->connectors[NEXUS_AudioConnectorType_eMultichannel].pMixerData )
    {
        NEXUS_AudioInput_P_GetDefaultInputVolume(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel]);
        NEXUS_AudioDecoder_P_ApplyDefaultInputVolume(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel]);
    }
    if ( handle->connectors[NEXUS_AudioConnectorType_eMono].pMixerData )
    {
        NEXUS_AudioInput_P_GetDefaultInputVolume(&handle->connectors[NEXUS_AudioConnectorType_eMono]);
        NEXUS_AudioDecoder_P_ApplyDefaultInputVolume(&handle->connectors[NEXUS_AudioConnectorType_eMono]);
    }
}

static NEXUS_Error NEXUS_AudioDecoder_P_EnableRaveContexts(NEXUS_AudioDecoderHandle handle, NEXUS_AudioDecoderStartSettings * program)
{
    BSTD_UNUSED(program);
    LOCK_TRANSPORT();
    BDBG_MSG(("Enable RAVE"));
    NEXUS_Rave_Enable_priv(handle->raveContext);
    UNLOCK_TRANSPORT();

    #if NEXUS_AUDIO_RAVE_MONITOR_EXT
    handle->raveMonitor.startSettings.rave = handle->raveContext;
    if ( handle->raveMonitor.monitor )
    {
        NEXUS_AudioRaveMonitor_Start(handle->raveMonitor.monitor, &handle->raveMonitor.startSettings);
    }
    #endif

    return NEXUS_SUCCESS;
}

static void NEXUS_AudioDecoder_P_DisableRaveContexts(NEXUS_AudioDecoderHandle handle, bool flush)
{

    #if NEXUS_AUDIO_RAVE_MONITOR_EXT
    if ( handle->raveMonitor.monitor )
    {
        NEXUS_AudioRaveMonitor_Stop(handle->raveMonitor.monitor);
    }
    #endif

    LOCK_TRANSPORT();
    BDBG_MSG(("Disable / Flush RAVE"));
    NEXUS_Rave_Disable_priv(handle->raveContext);
    if ( flush )
    {
        NEXUS_Rave_Flush_priv(handle->raveContext);
    }
    UNLOCK_TRANSPORT();
}

NEXUS_Error NEXUS_AudioDecoder_P_Start(NEXUS_AudioDecoderHandle handle)
{
    NEXUS_Error errCode;
    const NEXUS_AudioDecoderStartSettings *pProgram;

    BDBG_ASSERT(NULL != handle);

    BDBG_ENTER(NEXUS_AudioDecoder_P_Start);

    pProgram = &handle->programSettings;

    {
        int i;
        NEXUS_AudioOutputList *outputLists = &handle->outputLists[0];
        bool hasPcmOutputs;
        bool hasCompressedOutputs;

        /* Get output lists */
        for ( i = 0; i < NEXUS_AudioConnectorType_eMax; i++ )
        {
            NEXUS_AudioInput_P_GetOutputs(&handle->connectors[i], &outputLists[i], false);
        }

        /* 20111212 bandrews - the following code is concerned with stc programming priority.
        As such, the variable in use has been renamed to stcPriority.  DecoderType is now simply
        a mapping combining type and index */

        hasPcmOutputs = (outputLists[NEXUS_AudioConnectorType_eStereo].outputs[0] ||
                         outputLists[NEXUS_AudioConnectorType_eAlternateStereo].outputs[0] ||
                         outputLists[NEXUS_AudioConnectorType_eMultichannel].outputs[0] ||
                         outputLists[NEXUS_AudioConnectorType_eMono].outputs[0]) ? true : false;
        hasCompressedOutputs = (outputLists[NEXUS_AudioConnectorType_eCompressed].outputs[0] ||
                                outputLists[NEXUS_AudioConnectorType_eCompressed4x].outputs[0] ||
                                outputLists[NEXUS_AudioConnectorType_eCompressed16x].outputs[0]) ? true : false;

        /* Determine mode to add new outputs */
        if ( hasCompressedOutputs && !hasPcmOutputs )
        {
            /* Compressed output, but no stereo or multichannel.  This is a passthrough channel. */
            if ( handle->connectors[NEXUS_AudioConnectorType_eCompressed].format == NEXUS_AudioInputFormat_eCompressed )
            {
                BDBG_MSG(("Decoder %d is a passthrough channel", handle->index));
            }
            else
            {
                BDBG_MSG(("Decoder %d is a decode channel (PCM data for passthrough)", handle->index));
            }

            handle->stc.priority = 1;
        }
        else
        {
            if ( handle->descriptorParent || handle->programSettings.secondaryDecoder )
            {
                /* AD child or secondary decoder (MS10/MS11).  This is decoder type audio2 */
                handle->stc.priority = 2;
            }
            else
            {
                /* Standard decode channel.  Compressed will be simul */
                handle->stc.priority = 0;
            }
            /* Determine decoder mode as decode or simul based on presence of compressed outputs */
            if ( hasCompressedOutputs )
            {
                /* Simul Mode */
                BDBG_MSG(("Decoder %d is a simul channel", handle->index));
            }
            else
            {
                /* Decode Mode */
                BDBG_MSG(("Decoder %d is a decode channel", handle->index));
            }
        }

        if ( handle->openSettings.type == NEXUS_AudioDecoderType_eDecodeToMemory )
        {
            if ( hasCompressedOutputs || hasPcmOutputs )
            {
                BDBG_ERR(("You cannot connect outputs to a DecodeToMemory decoder"));
                BKNI_Memset(handle->outputLists, 0, sizeof(handle->outputLists));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
        else
        {
            if ( !hasCompressedOutputs && !hasPcmOutputs )
            {
                BDBG_ERR(("No outputs have been connected to this decoder."));
                BKNI_Memset(handle->outputLists, 0, sizeof(handle->outputLists));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }

        for ( i = 0; i < NEXUS_AudioConnectorType_eMax; i++ )
        {

            /* Only call this for inputs about to actually start */
            BDBG_MSG(("Preparing to start path %d", i));

            errCode = NEXUS_AudioInput_P_PrepareToStart(&handle->connectors[i]);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }

        /* Get Volume.  Apply correct volume in apply settings*/
        if (hasPcmOutputs)
        {
            NEXUS_AudioDecoder_P_SetDefaultVolume(handle);
        }
    }

    /* Setup StcChannel */
    if ( pProgram->stcChannel )
    {
        errCode = NEXUS_AudioDecoder_P_StartStcChannel(handle);
        if (errCode) { errCode = BERR_TRACE(errCode); goto err_set_tsm; }

        if ( pProgram->pidChannel && !handle->fifoWatchdog.timer)
        {
            handle->fifoWatchdog.timer = NEXUS_ScheduleTimer(250, NEXUS_AudioDecoder_P_FifoWatchdog, handle);
        }
    }

    /* After all that, we're ready to go.  Start. */
    handle->ptsErrorCount = 0;
    handle->numFifoOverflows = 0;
    BDBG_MSG(("Starting Decoder %d", handle->index));

#if NEXUS_HAS_SYNC_CHANNEL
    /* must be before apply settings to get the mute state correct */
    if (handle->sync.startMuted)
    {
        handle->sync.mute = true;
        BDBG_MSG(("Sync requested to start %u muted", handle->index));
    }
#endif

    /* Re-apply settings */
    NEXUS_AudioDecoder_ApplySettings_priv(handle);
    BDBG_MSG(("Starting with codec %u", handle->apeStartSettings.codec));

#if NEXUS_HAS_SYNC_CHANNEL
    /* must be before audio actually starts, in order not to miss any callbacks */
    handle->sync.started = true;
    if (handle->sync.settings.startCallback_isr)
    {
        BKNI_EnterCriticalSection();
        (*handle->sync.settings.startCallback_isr)(handle->sync.settings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }
#endif

    handle->locked = false;
    handle->numFifoOverflows = handle->numFifoUnderflows = 0;

    if (handle->settings.alwaysEnableDsola) {
        bool dspMixerAttached = false;
        errCode = NEXUS_AudioDecoder_P_IsDspMixerAttached(handle, &dspMixerAttached);
        if ( !errCode ) {
            if (g_NEXUS_audioModuleData.capabilities.dsp.decodeRateControl &&
                handle->settings.alwaysEnableDsola &&
                handle->trickState.rate <= NEXUS_AUDIO_DECODER_P_MAX_DSOLA_RATE &&
                !dspMixerAttached) {
                handle->apeStartSettings.decodeRateControl = true;
            }
        }
    }

#if !NEXUS_AUDIO_RAVE_MONITOR_EXT || !NEXUS_AUDIO_RAVE_MONITOR_CONSUME
    errCode = BAPE_Decoder_Start(handle->channel, &handle->apeStartSettings);
    if ( errCode && !handle->started ) { errCode = BERR_TRACE(errCode); goto err_dec_start; }
    #endif

    if ( handle->programSettings.pidChannel )
    {
        NEXUS_Error rc;
        rc = NEXUS_AudioDecoder_P_EnableRaveContexts(handle, &handle->programSettings);
        if ( rc )
        {
            BDBG_ERR(("Unable to start Audio Rave context(s)."));
            BERR_TRACE(rc); goto err_rave;
        }
    }
    handle->running = true;

    if ( handle->programSettings.pidChannel )
    {
        (void)NEXUS_PidChannel_ConsumerStarted(handle->programSettings.pidChannel);
    }

#if NEXUS_HAS_ASTM
    handle->astm.status.started = true;

    if (handle->astm.settings.lifecycle_isr)
    {
        BDBG_MSG(("Audio channel %p is notifying ASTM of its start action", (void *)handle));
        BKNI_EnterCriticalSection();
        (*handle->astm.settings.lifecycle_isr)(handle->astm.settings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }
#endif

    BDBG_LEAVE(NEXUS_AudioDecoder_P_Start);

    return BERR_SUCCESS;

err_rave:
    BAPE_Decoder_Stop(handle->channel);
err_dec_start:
    if (handle->primer) {
        /* decode will stop and primer will not restart. just disconnect. */
        NEXUS_AudioDecoderPrimer_P_DecodeStopped(handle->primer);
    }

    if (handle->fifoWatchdog.timer)
    {
        NEXUS_CancelTimer(handle->fifoWatchdog.timer);
        handle->fifoWatchdog.timer = NULL;
    }
err_set_tsm:
    if (handle->programSettings.stcChannel)
    {
        NEXUS_AudioDecoder_P_StopStcChannel(handle);
    }
    return errCode;
}

NEXUS_Error NEXUS_AudioDecoder_P_Stop(NEXUS_AudioDecoderHandle handle, bool flush)
{
    BDBG_ENTER(NEXUS_AudioDecoder_P_Stop);

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    if ( handle->running )
    {
        if ( handle->fifoWatchdog.timer )
        {
            NEXUS_CancelTimer(handle->fifoWatchdog.timer);
            handle->fifoWatchdog.timer = NULL;
        }

        handle->locked = false;
        #if !NEXUS_AUDIO_RAVE_MONITOR_EXT || !NEXUS_AUDIO_RAVE_MONITOR_CONSUME
        BAPE_Decoder_Stop(handle->channel);
        #endif
    }

    if ( handle->programSettings.pidChannel && flush )
    {
        NEXUS_AudioDecoder_P_DisableRaveContexts(handle, true);
    }

    if ( handle->running )
    {
        handle->running = false;
        if (handle->programSettings.stcChannel)
        {
            NEXUS_AudioDecoder_P_StopStcChannel(handle);
        }

#if NEXUS_HAS_SYNC_CHANNEL
        handle->sync.started = false;
        if (handle->sync.settings.startCallback_isr)
        {
            BKNI_EnterCriticalSection();
            (*handle->sync.settings.startCallback_isr)(handle->sync.settings.callbackContext, 0);
            BKNI_LeaveCriticalSection();
        }
#endif

#if NEXUS_HAS_ASTM
        handle->astm.status.started = false;

        if (handle->astm.settings.lifecycle_isr)
        {
            BDBG_MSG(("Audio channel %p is notifying ASTM of its stop action", (void *)handle));
            BKNI_EnterCriticalSection();
            (*handle->astm.settings.lifecycle_isr)(handle->astm.settings.callbackContext, 0);
            BKNI_LeaveCriticalSection();
        }
#endif

        if ( handle->openSettings.type == NEXUS_AudioDecoderType_eDecodeToMemory )
        {
            NEXUS_AudioDecoder_P_FlushDecodeToMemory(handle);
        }
    }

    /* Unset compressed output mutes incase outputs do not get attached again for compressed output */
    NEXUS_AudioDecoder_P_SetCompressedMute(handle, false);
    NEXUS_AudioDecoder_P_SetCompressedEncoderMute(handle, false);

    /* Invalidate the output list */
    BKNI_Memset(handle->outputLists, 0, sizeof(handle->outputLists));
    BDBG_LEAVE(NEXUS_AudioDecoder_P_Stop);

    return BERR_SUCCESS;
}

static BERR_Code NEXUS_AudioDecoder_P_GetPtsCallback_isr(void *pContext, BAVC_PTSInfo *pPTSInfo)
{
    BERR_Code errCode;
    NEXUS_AudioDecoderHandle handle = pContext;
    BAPE_DecoderTsmStatus tsmStatus;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pPTSInfo);
    errCode = BAPE_Decoder_GetTsmStatus_isr(handle->channel, &tsmStatus);
    if ( errCode )
    {
        return errCode; /* BERR_TRACE intentionally omitted */
    }
    *pPTSInfo = tsmStatus.ptsInfo;
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_AudioDecoder_P_GetCdbLevelCallback_isr(void *pContext, unsigned *pCdbLevel)
{
    unsigned depth=0, size=0;
    NEXUS_AudioDecoderHandle audioDecoder = (NEXUS_AudioDecoderHandle)pContext;
    BDBG_OBJECT_ASSERT(audioDecoder, NEXUS_AudioDecoder);
    if ( audioDecoder->programSettings.pidChannel )
    {
        NEXUS_Rave_GetCdbBufferInfo_isr(audioDecoder->raveContext, &depth, &size);
    }
    BDBG_MSG_TRACE(("GetCdbLevel - returned %d", depth));
    *pCdbLevel = depth;
    return 0;
}

static BERR_Code NEXUS_AudioDecoder_P_StcValidCallback_isr(void *pContext)
{
    NEXUS_AudioDecoderHandle audioDecoder = (NEXUS_AudioDecoderHandle)pContext;
    BDBG_OBJECT_ASSERT(audioDecoder, NEXUS_AudioDecoder);
    BDBG_MSG_TRACE(("STC Valid Decoder %u", audioDecoder->index));
    return BAPE_Decoder_SetStcValid_isr(audioDecoder->channel);
}

/* TODO: hard-coded from HE-AACv2 32KHz 2048/32000*45000 */
#define NEXUS_AUDIO_DECODER_P_MAX_SOURCE_FRAME_DURATION_45K 2880

static void NEXUS_AudioDecoder_P_FifoWatchdog(void *context)
{
    NEXUS_AudioDecoderHandle audioDecoder = context;
    unsigned timeout=150;

    audioDecoder->fifoWatchdog.timer = NULL;

    if (audioDecoder->programSettings.stcChannel)
    {
        uint64_t cdbValidPointer, cdbReadPointer;
        unsigned depth, size;
        BAPE_DecoderTsmStatus tsmStatus;

        BKNI_EnterCriticalSection();
        NEXUS_Rave_GetCdbPointers_isr(audioDecoder->raveContext, &cdbValidPointer, &cdbReadPointer);
        NEXUS_Rave_GetCdbBufferInfo_isr(audioDecoder->raveContext, &depth, &size);
        BAPE_Decoder_GetTsmStatus_isr(audioDecoder->channel, &tsmStatus);
        BKNI_LeaveCriticalSection();

        /* fifo threshold can override max fill depth, so we must use it to compare fullness */
        if (audioDecoder->settings.fifoThreshold && audioDecoder->settings.fifoThreshold < size)
        {
            size = audioDecoder->settings.fifoThreshold;
        }

        if (audioDecoder->fifoWatchdog.lastCdbValidPointer == cdbValidPointer && audioDecoder->fifoWatchdog.lastCdbReadPointer == cdbReadPointer) {
            if (audioDecoder->fifoWatchdog.staticCount < 20) {
                audioDecoder->fifoWatchdog.staticCount++;
            }
        }
        else {
            audioDecoder->fifoWatchdog.staticCount = 0;
            audioDecoder->fifoWatchdog.lastCdbValidPointer = cdbValidPointer;
            audioDecoder->fifoWatchdog.lastCdbReadPointer = cdbReadPointer;
        }

        if (audioDecoder->stc.connector)
        {
            LOCK_TRANSPORT();
            NEXUS_StcChannel_GetDefaultDecoderFifoWatchdogStatus_priv(&audioDecoder->fifoWatchdog.status);
            audioDecoder->fifoWatchdog.status.isHung = (audioDecoder->fifoWatchdog.staticCount > 4) && (audioDecoder->trickState.rate >= NEXUS_NORMAL_DECODE_RATE);
            audioDecoder->fifoWatchdog.status.percentFull = size ? depth * 100 / size : 0;
            audioDecoder->fifoWatchdog.status.frameSyncUnlocked = audioDecoder->locked;
            /* audio reports STC - PTS */
            audioDecoder->fifoWatchdog.status.tsmWait = -tsmStatus.ptsStcDifference > NEXUS_AUDIO_DECODER_P_MAX_SOURCE_FRAME_DURATION_45K;
            NEXUS_StcChannel_ReportDecoderHang_priv(audioDecoder->stc.connector, &audioDecoder->fifoWatchdog.status);
            UNLOCK_TRANSPORT();
        }
    }

    audioDecoder->fifoWatchdog.timer = NEXUS_ScheduleTimer(timeout, NEXUS_AudioDecoder_P_FifoWatchdog, audioDecoder);
}

static void NEXUS_AudioDecoder_P_StcChannelStatusChangeEventHandler(void *context)
{
    NEXUS_AudioDecoderHandle audioDecoder = context;
    NEXUS_StcChannelDecoderConnectionStatus status;

    if (!audioDecoder->stc.connector) return;

    LOCK_TRANSPORT();
    NEXUS_StcChannel_GetDecoderConnectionStatus_priv(audioDecoder->stc.connector, &status);
    UNLOCK_TRANSPORT();

    if (status.flush || status.zeroFill)
    {
        /* reset count if we are doing something about it */
        audioDecoder->fifoWatchdog.staticCount = 0;
    }

    /* if both flush and zero-fill are set, flush first */

    if (status.flush) {
        BDBG_WRN(("Audio FIFO watchdog flush"));
        NEXUS_AudioDecoder_Flush(audioDecoder);
        LOCK_TRANSPORT();
        NEXUS_StcChannel_ReportDecoderFlush_priv(audioDecoder->stc.connector);
        UNLOCK_TRANSPORT();
    }

    /* this is a one-shot, like a flush */
    if (status.zeroFill)
    {
        BDBG_WRN(("audio %p: Enabling audio gap zero fill", (void *)audioDecoder));
        BAPE_Decoder_EnterUnderflowMode(audioDecoder->channel);
        LOCK_TRANSPORT();
        NEXUS_StcChannel_ReportDecoderZeroFill_priv(audioDecoder->stc.connector);
        UNLOCK_TRANSPORT();
    }
}

static void NEXUS_AudioDecoder_P_InputFormatChange_isr(void *pParam1, int param2)
{
    NEXUS_AudioDecoderHandle handle = (NEXUS_AudioDecoderHandle)pParam1;
    BSTD_UNUSED(param2);
    /* convert to task time */
    BKNI_SetEvent_isr(handle->inputFormatChangeEvent);
}

static void NEXUS_AudioDecoder_P_InputFormatChange(void *pParam)
{
    NEXUS_AudioDecoderHandle handle = (NEXUS_AudioDecoderHandle)pParam;
    NEXUS_AudioInputPortStatus inputPortStatus;
    NEXUS_Error errCode;
    BAVC_AudioCompressionStd avcCodec;
    BAPE_DecoderStatus *pDecoderStatus = &handle->apeStatus;
    bool stop=false, start=false;
    bool wasRunning;

    BDBG_MSG(("%s", BSTD_FUNCTION));
    if (!handle->started) {
        return;
    }

    wasRunning = handle->running;

    errCode = NEXUS_AudioInput_P_GetInputPortStatus(handle->programSettings.input, &inputPortStatus);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        return;
    }

    BAPE_Decoder_GetStatus(handle->channel, pDecoderStatus);

    avcCodec = NEXUS_Audio_P_CodecToMagnum(inputPortStatus.codec);

    if ( handle->running )  /* If Nexus thinks that APE decoder is running. */
    {
        BDBG_MSG(("Input Format Change - Decoder is running, running %d, halted %d, lastNumChs %d, newNumChs %d", pDecoderStatus->running, pDecoderStatus->halted, handle->inputPortStatus.numPcmChannels, inputPortStatus.numPcmChannels));
        if ( pDecoderStatus->halted       ||      /* due to on-the-fly input format change. */
             !pDecoderStatus->running     ||      /* due to format change during APE decoder start. */
             avcCodec != handle->apeStartSettings.codec )  /* due to on-the-fly codec change.    */
        {
            /* The APE decoder has stopped because of a format change that can't be handled on-the-fly. */
            BDBG_MSG(("APE Decoder has halted due to format change.  Need to stop decoder."));
            stop = true;
            if ( inputPortStatus.signalPresent )
            {
                /* Restart with new codec if signal is present */
                start = handle->programSettings.inputFormatChangeMode == NEXUS_AudioInputFormatChangeMode_eAuto;
                BDBG_MSG(("Valid input signal. Need to restart or halt."));
            }
            else
            {
                BDBG_MSG(("No valid input signal. Don't need to restart."));
            }
        }
        else
        {
            /* Nothing to do, return. */
            BDBG_MSG(("APE Decoder running and not halted. No action needed."));
            return;
        }
    }
    else  /* Nexus thinks that APE decoder is stopped. */
    {
        BDBG_MSG(("Input Format Change - Decoder is not running"));
        if ( inputPortStatus.signalPresent )
        {
            /* Start with new codec if signal is present */
            BDBG_MSG(("Valid input signal, starting decoder."));
            start = handle->programSettings.inputFormatChangeMode == NEXUS_AudioInputFormatChangeMode_eAuto;
        }
        else
        {
            BDBG_MSG(("No valid input signal, not starting decoder."));
        }
    }

    if ( stop )
    {
        NEXUS_Error errCode;
        BDBG_MSG(("Stop decoder on input format change"));
        if ( handle->programSettings.inputFormatChangeMode == NEXUS_AudioInputFormatChangeMode_eAuto )
        {
            errCode = NEXUS_AudioDecoder_P_Stop(handle, true);
            if ( errCode )
            {
                (void)BERR_TRACE(errCode);
            }
        }
        else
        {
            (void)NEXUS_AudioDecoder_Stop(handle);
        }
    }

    handle->apeStartSettings.codec = avcCodec;
    if ( start )
    {
        NEXUS_Error errCode;
        BDBG_MSG(("Start decoder on input format change codec %d", handle->apeStartSettings.codec));
        errCode = NEXUS_AudioDecoder_P_Start(handle);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
        }
    }

    handle->inputPortStatus = inputPortStatus;

    /* application may need to take action due to this change */
    if ( (handle->running != wasRunning || !handle->running) &&
         handle->sourceChangeAppCallback )
    {
        BKNI_EnterCriticalSection();
        NEXUS_IsrCallback_Fire_isr(handle->sourceChangeAppCallback);
        BKNI_LeaveCriticalSection();
    }
}

static BERR_Code NEXUS_AudioDecoder_P_SetPcrOffset_isr(void *pContext, uint32_t pcrOffset)
{
    NEXUS_AudioDecoderHandle handle = (NEXUS_AudioDecoderHandle)pContext;
    BAPE_DecoderTsmSettings tsmSettings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    BAPE_Decoder_GetTsmSettings_isr(handle->channel, &tsmSettings);
    tsmSettings.stcOffset = pcrOffset;
    errCode = BAPE_Decoder_SetTsmSettings_isr(handle->channel, &tsmSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_AudioDecoder_P_GetPcrOffset_isr(void *pContext, uint32_t *pPcrOffset)
{
    NEXUS_AudioDecoderHandle handle = (NEXUS_AudioDecoderHandle)pContext;
    BAPE_DecoderTsmSettings tsmSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pPcrOffset);

    BAPE_Decoder_GetTsmSettings_isr(handle->channel, &tsmSettings);
    *pPcrOffset = tsmSettings.stcOffset;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_GetExtendedStatus(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderExtendedStatus *pStatus /* [out] */
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(NULL != pStatus);
    if ( handle->raveContext )
    {
        NEXUS_RaveStatus raveStatus;

        LOCK_TRANSPORT();
        errCode = NEXUS_Rave_GetStatus_priv(handle->raveContext, &raveStatus);
        UNLOCK_TRANSPORT();
        if ( errCode == BERR_SUCCESS )
        {
            pStatus->raveIndex = raveStatus.index;
        }
    }

    return errCode;
}

void NEXUS_AudioDecoder_IsCodecSupported(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioCodec codec,
    bool *pSupported
    )
{
    NEXUS_AudioCapabilities caps;

    BSTD_UNUSED(handle);
    BDBG_ASSERT(NULL != pSupported);

    if(codec >= NEXUS_AudioCodec_eMax)
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        *pSupported = false;
        return;
    }

    NEXUS_GetAudioCapabilities(&caps);
    *pSupported = caps.dsp.codecs[codec].decode;
}

void NEXUS_AudioDecoder_IsPassthroughCodecSupported(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioCodec codec,
    bool *pSupported
    )
{
    NEXUS_AudioCapabilities caps;

    BSTD_UNUSED(handle);
    BDBG_ASSERT(NULL != pSupported);

    if(codec >= NEXUS_AudioCodec_eMax)
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        *pSupported = false;
        return;
    }

    NEXUS_GetAudioCapabilities(&caps);
    *pSupported = caps.dsp.codecs[codec].passthrough;
    return;
}

NEXUS_Error NEXUS_AudioDecoder_GetAncillaryDataBuffer(
    NEXUS_AudioDecoderHandle handle,
    void **pBuffer,   /* [out] attr{memory=cached} pointer to ancillary data */
    size_t *pSize     /* [out] number of bytes of userdata */
    )
{
    void *pBuf;
    BERR_Code errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, handle);

    errCode = BAPE_Decoder_GetAncillaryDataBuffer(handle->channel, &pBuf, pSize);
    if ( errCode )
    {
        handle->lastAncillarySize = 0;
        return BERR_TRACE(errCode);
    }
    if ( !pBuf )
    {
        handle->lastAncillarySize = 0;
        return BERR_TRACE(BERR_NOT_AVAILABLE);
    }
    *pBuffer = pBuf;
    handle->lastAncillarySize = *pSize;
    return BERR_SUCCESS;
}

void NEXUS_AudioDecoder_AncillaryDataReadComplete(
    NEXUS_AudioDecoderHandle handle,
    size_t size   /* number of bytes of userdata consumed by the application */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, handle);

    if ( size > handle->lastAncillarySize )
    {
        BDBG_ERR(("Invalid number of bytes to NEXUS_AudioDecoder_AncillaryDataReadComplete"));
        BDBG_ERR(("Last Size from GetBuffer: %lu Request to Read %lu", (unsigned long)handle->lastAncillarySize, (unsigned long)size));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    (void)BAPE_Decoder_ConsumeAncillaryData(handle->channel, size);
    handle->lastAncillarySize = 0;
}

void NEXUS_AudioDecoder_FlushAncillaryData(
    NEXUS_AudioDecoderHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, handle);

    BAPE_Decoder_FlushAncillaryData(handle->channel);
}

NEXUS_Error NEXUS_AudioDecoder_P_IsDspMixerAttached(
    NEXUS_AudioDecoderHandle handle,
    bool *pDSPMixerAttached
    )
{
    int i;

    /* DSP Mixer has to be the first mixer if present */
    for (i=0; i < NEXUS_AudioDecoderConnectorType_eMax; i++)
    {
        NEXUS_AudioMixerHandle mixerHandle = NEXUS_AudioInput_P_LocateMixer( &handle->connectors[i], NULL);
        if (mixerHandle)
        {
            NEXUS_AudioInputHandle connector = NEXUS_AudioMixer_GetConnector(mixerHandle);
            if (connector->objectType == NEXUS_AudioInputType_eDspMixer)
            {
                *pDSPMixerAttached = true;
                return BERR_SUCCESS;
            }
        }
    }
    *pDSPMixerAttached = false;
    return BERR_SUCCESS;
}

#if NEXUS_HAS_SIMPLE_DECODER
void NEXUS_AudioDecoder_GetSimpleSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    NEXUS_AudioDecoderSimpleSettings *pSimpleSettings  /* [out] */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, audioDecoder);
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(pSimpleSettings);

    *pSimpleSettings = audioDecoder->simple.settings;
}

NEXUS_Error NEXUS_AudioDecoder_SetSimpleSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    const NEXUS_AudioDecoderSimpleSettings *pSimpleSettings
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, audioDecoder);
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(pSimpleSettings);

    audioDecoder->simple.settings = *pSimpleSettings;

    rc = NEXUS_AudioDecoder_ApplySettings_priv(audioDecoder);
    if (rc) BERR_TRACE(rc);

    return rc;
}
#endif

static void NEXUS_AudioDecoder_P_SplicePoint_isr(void *Ctx, uint32_t pts)
{
    NEXUS_AudioDecoderHandle handle = (NEXUS_AudioDecoderHandle)(Ctx);
    uint32_t ptsRight = 0;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    /* ptsLeft is handle->spliceSettings.pts */
    ptsRight = handle->spliceSettings.pts + handle->spliceSettings.ptsThreshold;
    if ((handle->spliceSettings.pts <= pts) && (pts <= ptsRight)) {
        if (handle->spliceSettings.mode == NEXUS_DecoderSpliceMode_eStartAtPts) {
            handle->spliceStatus.state = NEXUS_DecoderSpliceState_eFoundStartPts;
        }else if (handle->spliceSettings.mode == NEXUS_DecoderSpliceMode_eStopAtPts) {
            handle->spliceStatus.state = NEXUS_DecoderSpliceState_eFoundStopPts;
        }
        handle->spliceStatus.pts = pts;
        NEXUS_IsrCallback_Fire_isr(handle->spliceCallback);
    }
    return;
}


NEXUS_Error NEXUS_AudioDecoder_SetSpliceSettings(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderSpliceSettings *pSettings
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_Rave_SpliceSettings raveSpliceSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);

    if (false == handle->openSettings.spliceEnabled) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return rc;
    }

    BKNI_Memset(&raveSpliceSettings,0,sizeof(NEXUS_Rave_SpliceSettings));
    switch (pSettings->mode) {
    case NEXUS_DecoderSpliceMode_eDisabled: {
        if ((handle->spliceStatus.state == NEXUS_DecoderSpliceState_eWaitForStopPts) ||
            (handle->spliceStatus.state == NEXUS_DecoderSpliceState_eWaitForStartPts)) {
            rc = NEXUS_Rave_SetSplicePoint_priv(handle->raveContext,&raveSpliceSettings);
            if (rc) return BERR_TRACE(rc);
        }
        NEXUS_IsrCallback_Set(handle->spliceCallback, NULL);
        BKNI_Memset(&handle->spliceSettings,0,sizeof(NEXUS_AudioDecoderSpliceSettings));
        NEXUS_CALLBACKDESC_INIT(&handle->spliceSettings.splicePoint);
        handle->spliceStatus.state = NEXUS_DecoderSpliceState_eNone;
        handle->spliceStatus.pts = 0;
        rc = NEXUS_SUCCESS;
        break;
    }
    case NEXUS_DecoderSpliceMode_eStopAtPts: {
        raveSpliceSettings.context = (void *)handle;
        raveSpliceSettings.ptsThreshold = pSettings->ptsThreshold;
        raveSpliceSettings.pts = pSettings->pts;
        raveSpliceSettings.splicePoint = NEXUS_AudioDecoder_P_SplicePoint_isr;
        raveSpliceSettings.type = NEXUS_Rave_SpliceType_eStopPts;
        rc = NEXUS_Rave_SetSplicePoint_priv(handle->raveContext,&raveSpliceSettings);
        if (rc) return BERR_TRACE(rc);
        handle->spliceSettings = *pSettings;
        NEXUS_IsrCallback_Set(handle->spliceCallback, &pSettings->splicePoint);
        handle->spliceStatus.state = NEXUS_DecoderSpliceState_eWaitForStopPts;
        break;
    }
    case NEXUS_DecoderSpliceMode_eStartAtPts: {
        raveSpliceSettings.context = (void *)handle;
        raveSpliceSettings.ptsThreshold = pSettings->ptsThreshold;
        raveSpliceSettings.pts = pSettings->pts;
        raveSpliceSettings.splicePoint = NEXUS_AudioDecoder_P_SplicePoint_isr;
        raveSpliceSettings.type = NEXUS_Rave_SpliceType_eStartPts;
        rc = NEXUS_Rave_SetSplicePoint_priv(handle->raveContext,&raveSpliceSettings);
        if (rc) return BERR_TRACE(rc);
        handle->spliceSettings = *pSettings;
        NEXUS_IsrCallback_Set(handle->spliceCallback, &pSettings->splicePoint);
        handle->spliceStatus.state = NEXUS_DecoderSpliceState_eWaitForStartPts;
        break;
    }
    default:
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    }
    return rc;
}

void NEXUS_AudioDecoder_GetSpliceSettings(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderSpliceSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    *pSettings = handle->spliceSettings;
}

NEXUS_Error NEXUS_AudioDecoder_GetSpliceStatus(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderSpliceStatus *pStatus
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    *pStatus = handle->spliceStatus;
    rc = NEXUS_SUCCESS;
    return rc;
}

void NEXUS_AudioDecoder_SpliceStopFlow(
    NEXUS_AudioDecoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    if (NEXUS_DecoderSpliceMode_eDisabled == handle->spliceSettings.mode) {
        BDBG_WRN(("%s:%d: call is not supported in this context", __FILE__, __LINE__));
        goto ExitFunc;
    }
    if (!handle->started) {
        BDBG_WRN(("%s:%d: call is not supported in this context", __FILE__, __LINE__));
        goto ExitFunc;
    }
    if (handle->spliceFlowStopped) {
        BDBG_WRN(("%s:%d: call is not supported in this context", __FILE__, __LINE__));
        goto ExitFunc;
    }

    NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, handle->programSettings.pidChannel);
    LOCK_TRANSPORT();
    NEXUS_Rave_Disable_priv(handle->raveContext);
    NEXUS_Rave_RemovePidChannel_priv(handle->raveContext);
    UNLOCK_TRANSPORT();
    handle->spliceFlowStopped = true;
ExitFunc:
    return;
}

void NEXUS_AudioDecoder_GetDefaultSpliceStartFlowSettings(
    NEXUS_AudioDecoderSpliceStartFlowSettings *pSettings
    )
{
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioDecoderSpliceStartFlowSettings));
}

NEXUS_Error NEXUS_AudioDecoder_SpliceStartFlow(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderSpliceStartFlowSettings *pSettings
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_PidChannelStatus pidChannelStatus;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDecoder);
    BDBG_ASSERT(pSettings);

    if (NEXUS_DecoderSpliceMode_eDisabled == handle->spliceSettings.mode) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto ExitFunc;
    }

    if (!handle->started) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto ExitFunc;
    }
    if (!handle->spliceFlowStopped) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto ExitFunc;
    }
    if (!pSettings->pidChannel) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto ExitFunc;
    }
    rc = NEXUS_PidChannel_GetStatus(pSettings->pidChannel, &pidChannelStatus);
    if (rc) {
        BERR_TRACE(rc);
        goto ExitFunc;
    }
    LOCK_TRANSPORT();
    NEXUS_Rave_AddPidChannel_priv(handle->raveContext, pSettings->pidChannel);
    NEXUS_Rave_SetBandHold(handle->raveContext,pidChannelStatus.playback);
    NEXUS_Rave_Enable_priv(handle->raveContext);
    UNLOCK_TRANSPORT();
    NEXUS_OBJECT_ACQUIRE(handle, NEXUS_PidChannel, pSettings->pidChannel);
    /* needed to unpause playback */
    NEXUS_PidChannel_ConsumerStarted(pSettings->pidChannel);
    handle->programSettings.pidChannel = pSettings->pidChannel;
    handle->spliceFlowStopped = false;
    rc = NEXUS_SUCCESS;
ExitFunc:
    return rc;
}
