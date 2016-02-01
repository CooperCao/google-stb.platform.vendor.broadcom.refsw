/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: SpdifOutput
*    Specific APIs related to SPDIF audio outputs.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_module);

NEXUS_ModuleHandle g_NEXUS_audioModule;
NEXUS_AudioModuleData g_NEXUS_audioModuleData;

static void NEXUS_AudioModule_Print(void)
{
    BDBG_MSG(("Audio:"));
    #if 0   /* TODO */
    BDBG_MSG((" handles: xpt:%p hdmi in:%p hdmi out:%p fe:%p rap:%p",g_NEXUS_audioModuleData.transport,g_NEXUS_audioModuleData.hdmiInput,g_NEXUS_audioModuleData.hdmiOutput,g_NEXUS_audioModuleData.frontend,g_NEXUS_audioModuleData.hRap));
    BDBG_MSG((" img: ctxt:%p i:%p",g_NEXUS_audioModuleData.img_context,g_NEXUS_audioModuleData.img_interface));
    BDBG_MSG((" settings: wd:%d id:%d hbre:%d maxpb:%d",g_NEXUS_audioModuleData.moduleSettings.watchdogEnabled,g_NEXUS_audioModuleData.moduleSettings.independentDelay,g_NEXUS_audioModuleData.moduleSettings.hbrEnabled,g_NEXUS_audioModuleData.moduleSettings.maximumProcessingBranches));
    #endif
}

void NEXUS_AudioModule_GetDefaultSettings(
    const struct NEXUS_Core_PreInitState *preInitState,
    NEXUS_AudioModuleSettings *pSettings    /* [out] */
    )
{
    BSTD_UNUSED(preInitState);
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioModuleSettings));
}

NEXUS_ModuleHandle NEXUS_AudioModule_Init(
    const NEXUS_AudioModuleSettings *pSettings  /* NULL will use default settings */
    )
{
    BERR_Code errCode;
    NEXUS_ModuleSettings moduleSettings;
    BAPE_Settings apeSettings;
    NEXUS_AudioModuleSettings audioModuleSettings;
    static BIMG_Interface imgInterface;

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow; /* decoder interface is slow */
    moduleSettings.dbgPrint = NEXUS_AudioModule_Print;
    moduleSettings.dbgModules = "nexus_audio_module";
    g_NEXUS_audioModule = NEXUS_Module_Create("audio", &moduleSettings);

    if ( NULL == g_NEXUS_audioModule )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }

    NEXUS_LockModule();

    if ( NULL == pSettings )
    {
        NEXUS_AudioModule_GetDefaultSettings(NULL, &audioModuleSettings);
        pSettings = &audioModuleSettings;
    }

    /* TODO: IMG */

    g_NEXUS_audioModuleData.settings = *pSettings;

    BAPE_GetDefaultSettings(&apeSettings);

    apeSettings.readReserved = pSettings->readReserved;
    apeSettings.writeReserved = pSettings->writeReserved;
    apeSettings.readCoreRegister = pSettings->readCoreRegister;
    apeSettings.writeCoreRegister = pSettings->writeCoreRegister;
    apeSettings.readCmtControl = pSettings->readCmtControl;
    apeSettings.writeCmtControl = pSettings->writeCmtControl;
    apeSettings.pParam1 = pSettings->pReservedParam1;
    apeSettings.param2 = pSettings->reservedParam2;
    if ( NEXUS_GetEnv("audio_ramp_disabled") )
    {
        apeSettings.rampPcmSamples = false;
    }

    if ( Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_RAP, &g_NEXUS_audioModuleData.pImageContext, &imgInterface) == NEXUS_SUCCESS )
    {
        BDBG_WRN(("FW download used"));
        apeSettings.pImageContext = g_NEXUS_audioModuleData.pImageContext;
        apeSettings.pImageInterface = &imgInterface;
    }

    errCode = BAPE_Open(&NEXUS_AUDIO_DEVICE_HANDLE,
                        g_pCoreHandles->chp,
                        g_pCoreHandles->reg,
                        g_pCoreHandles->heap[0].mem,
                        g_pCoreHandles->bint,
                        g_pCoreHandles->tmr,
                        &apeSettings);

    if ( errCode )
    {
        NEXUS_Module_Destroy(g_NEXUS_audioModule);
        (void)BERR_TRACE(errCode);
        goto err_open;
    }

    errCode = NEXUS_AudioDecoder_P_Init();
    if ( errCode )
    {
        BAPE_Close(NEXUS_AUDIO_DEVICE_HANDLE);
        NEXUS_Module_Destroy(g_NEXUS_audioModule);
        (void)BERR_TRACE(errCode);
        goto err_init;
    }

    NEXUS_UnlockModule();

    return g_NEXUS_audioModule;

err_init:
err_open:
    NEXUS_UnlockModule();
    return NULL;
}

void NEXUS_AudioModule_Uninit(void)
{
    NEXUS_LockModule();
    NEXUS_AudioDecoder_P_Uninit();

    if ( NEXUS_AUDIO_DEVICE_HANDLE )
    {
        BAPE_Close(NEXUS_AUDIO_DEVICE_HANDLE);
    }
    if (g_NEXUS_audioModuleData.pImageContext) {
        Nexus_Core_P_Img_Destroy(g_NEXUS_audioModuleData.pImageContext);
    }
    NEXUS_UnlockModule();
    if ( g_NEXUS_audioModule )
    {
        NEXUS_Module_Destroy(g_NEXUS_audioModule);
    }    
}

BAVC_AudioSamplingRate NEXUS_AudioModule_P_SampleRate2Avc(unsigned sampleRate)
{
    switch ( sampleRate )
    {
    case 48000:
        return BAVC_AudioSamplingRate_e48k;
    case 44100:
        return BAVC_AudioSamplingRate_e44_1k;
    case 32000:
        return BAVC_AudioSamplingRate_e32k;
    case 96000:
        return BAVC_AudioSamplingRate_e96k;
    case 16000:
        return BAVC_AudioSamplingRate_e16k;
    case 22050:
        return BAVC_AudioSamplingRate_e22_05k;
    case 24000:
        return BAVC_AudioSamplingRate_e24k;
    case 64000:
        return BAVC_AudioSamplingRate_e64k;
    case 88200:
        return BAVC_AudioSamplingRate_e88_2k;
    case 128000:
        return BAVC_AudioSamplingRate_e128k;
    case 176400:
        return BAVC_AudioSamplingRate_e176_4k;
    case 192000:
        return BAVC_AudioSamplingRate_e192k;
    case 8000:
        return BAVC_AudioSamplingRate_e8k;
    case 12000:
        return BAVC_AudioSamplingRate_e12k;
    case 11025:
        return BAVC_AudioSamplingRate_e11_025k;
    default:
        BDBG_WRN(("Unrecognized sample rate (%u)- defaulting to BAVC_AudioSamplingRate_e48k", sampleRate));
        return BAVC_AudioSamplingRate_e48k;
    }
}

unsigned NEXUS_AudioModule_P_Avc2SampleRate(BAVC_AudioSamplingRate sampleRate)
{
    switch ( sampleRate )
    {
    case BAVC_AudioSamplingRate_e48k:
        return 48000;
    case BAVC_AudioSamplingRate_e44_1k:
        return 44100;
    case BAVC_AudioSamplingRate_e32k:
        return 32000;
    case BAVC_AudioSamplingRate_e96k:
        return 96000;
    case BAVC_AudioSamplingRate_e16k:
        return 16000;
    case BAVC_AudioSamplingRate_e22_05k:
        return 22050;
    case BAVC_AudioSamplingRate_e24k:
        return 24000;
    case BAVC_AudioSamplingRate_e64k:
        return 64000;
    case BAVC_AudioSamplingRate_e88_2k:
        return 88200;
    case BAVC_AudioSamplingRate_e128k:
        return 128000;
    case BAVC_AudioSamplingRate_e176_4k:
        return 176400;
    case BAVC_AudioSamplingRate_e192k:
        return 192000;
    case BAVC_AudioSamplingRate_e8k:
        return 8000;
    case BAVC_AudioSamplingRate_e12k:
        return 12000;
    case BAVC_AudioSamplingRate_e11_025k:
        return 11025;
    default:
        BDBG_WRN(("Unrecognized sample rate (%u) - defaulting to 48000", sampleRate));
        return 48000;
        /* Fall through */
    }
}

void NEXUS_AudioModule_GetRampStepSettings(
    NEXUS_AudioRampStepSettings *pRampStepSettings      /* [out] ramping step size for scale change for all output ports */
    )
{
    uint32_t val;
    if ( NULL == pRampStepSettings )
    {
        return;
    }
    BAPE_GetOutputVolumeRampStep(NEXUS_AUDIO_DEVICE_HANDLE, &val);
    pRampStepSettings->mixerRampStep = val;
}

NEXUS_Error NEXUS_AudioModule_SetRampStepSettings(
    const NEXUS_AudioRampStepSettings *pRampStepSettings  /* ramping step size for scale change for all output ports */
    )
{
    if ( NULL == pRampStepSettings )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BAPE_SetOutputVolumeRampStep(NEXUS_AUDIO_DEVICE_HANDLE, pRampStepSettings->mixerRampStep);

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioModule_EnableExternalMclk(
    unsigned mclkIndex,
    NEXUS_AudioOutputPll pllIndex,
    NEXUS_ExternalMclkRate mclkRate
    )
{
    BSTD_UNUSED(mclkIndex);
    BSTD_UNUSED(pllIndex);
    BSTD_UNUSED(mclkRate);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

#include "bchp_common.h"

#if defined BCHP_VCXO_2_RM_REG_START
#define NUM_VCXOS 3
#elif defined BCHP_VCXO_1_RM_REG_START
#define NUM_VCXOS 2
#elif defined BCHP_VCXO_0_RM_REG_START
#define NUM_VCXOS 1
#else
#define NUM_VCXOS 0
#endif

#if defined BCHP_AUD_FMM_PLL2_REG_START
#define NUM_PLLS 3
#elif defined BCHP_AUD_FMM_PLL1_REG_START
#define NUM_PLLS 2
#elif defined BCHP_AUD_FMM_PLL0_REG_START
#define NUM_PLLS 1
#else
#define NUM_PLLS 0
#endif

void NEXUS_GetAudioCapabilities(NEXUS_AudioCapabilities *pCaps)
{
    BDBG_ASSERT(NULL != pCaps);
    BKNI_Memset(pCaps, 0, sizeof(NEXUS_AudioCapabilities));
    #if NEXUS_NUM_I2S_INPUTS
    pCaps->numInputs.i2s = NEXUS_NUM_I2S_INPUTS;
    #endif
    #if NEXUS_NUM_AUDIO_CAPTURES
    pCaps->numOutputs.capture = NEXUS_NUM_AUDIO_CAPTURES;
    #endif
    #if NEXUS_NUM_HDMI_OUTPUTS
    pCaps->numOutputs.hdmi = NEXUS_NUM_HDMI_OUTPUTS;
    #endif
    #if NEXUS_NUM_I2S_OUTPUTS
    pCaps->numOutputs.i2s = NEXUS_NUM_I2S_OUTPUTS;
    #endif
    #if NEXUS_NUM_RFM_OUTPUTS
    pCaps->numOutputs.rfmod = NEXUS_NUM_RFM_OUTPUTS;
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS
    pCaps->numOutputs.spdif = NEXUS_NUM_SPDIF_OUTPUTS;
    #endif
    #if NEXUS_NUM_AUDIO_DECODERS
    pCaps->numDecoders = NEXUS_NUM_AUDIO_DECODERS;
    #endif
    #if NEXUS_NUM_AUDIO_PLAYBACKS
    pCaps->numPlaybacks = NEXUS_NUM_AUDIO_PLAYBACKS;
    #endif
    pCaps->numVcxos = NUM_VCXOS;
    pCaps->numPlls = NUM_PLLS;

    #ifdef BAPE_DDP_SUPPORT
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAc3].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAc3Plus].decode = true;
    #endif
    #ifdef BAPE_AC3_SUPPORT
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAc3].decode = true;
    #endif
    #ifdef BAPE_PCMWAV_SUPPORT
    pCaps->dsp.codecs[NEXUS_AudioCodec_ePcmWav].decode = true;
    #endif
    #ifdef BAPE_MPEG_SUPPORT
    pCaps->dsp.codecs[NEXUS_AudioCodec_eMpeg].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eMp3].decode = true;
    #endif
    #ifdef BAPE_AAC_SUPPORT
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAacAdts].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAacLoas].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAacPlusAdts].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAacPlusLoas].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eLpcmDvd].decode = true;
    #endif
}

void NEXUS_AudioOutputPll_GetSettings(
    NEXUS_AudioOutputPll pll,
    NEXUS_AudioOutputPllSettings *pSettings       /* [out] Current Settings */
    )
{
    BAPE_PllSettings apePllSettings;
    BAPE_Pll apePll;

    apePll = (BAPE_Pll)pll;
    BDBG_ASSERT(NULL != pSettings);

    BAPE_Pll_GetSettings(g_NEXUS_audioModuleData.apeHandle, apePll, &apePllSettings);
    pSettings->mode = apePllSettings.freeRun ? NEXUS_AudioOutputPllMode_eCustom : NEXUS_AudioOutputPllMode_eVcxo;
    pSettings->modeSettings.vcxo.vcxo = (NEXUS_Vcxo)apePllSettings.vcxo;
}

NEXUS_Error NEXUS_AudioOutputPll_SetSettings(
    NEXUS_AudioOutputPll pll,
    const NEXUS_AudioOutputPllSettings *pSettings
    )
{
    BAPE_PllSettings apePllSettings;
    BERR_Code errCode;
    BAPE_Pll apePll;

    apePll = (BAPE_Pll)pll;
    BDBG_ASSERT(NULL != pSettings);

    BAPE_Pll_GetSettings(g_NEXUS_audioModuleData.apeHandle, apePll, &apePllSettings);
    apePllSettings.freeRun = pSettings->mode == NEXUS_AudioOutputPllMode_eCustom ? true : false;
    apePllSettings.vcxo = (unsigned)pSettings->modeSettings.vcxo.vcxo;       
    errCode = BAPE_Pll_SetSettings(g_NEXUS_audioModuleData.apeHandle, apePll, &apePllSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void NEXUS_GetAudioFirmwareVersion(
    NEXUS_AudioFirmwareVersion *pVersion /* [out] */
    )
{
    BDBG_ASSERT(NULL != pVersion);
    BKNI_Memset(pVersion, 0, sizeof(*pVersion));
}

void NEXUS_AudioModule_GetDefaultUsageSettings(
    NEXUS_AudioModuleUsageSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error NEXUS_AudioModule_GetMemoryEstimate(
    const NEXUS_AudioModuleUsageSettings *pSettings,
    NEXUS_AudioModuleMemoryEstimate *pEstimate  /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
    BDBG_ASSERT(NULL != pEstimate);
    BKNI_Memset(pEstimate, 0, sizeof(*pEstimate));

    return BERR_SUCCESS;
}

