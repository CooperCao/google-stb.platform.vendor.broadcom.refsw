/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_module);

/* global module handle & data */
NEXUS_ModuleHandle g_NEXUS_audioModule;
NEXUS_AudioModuleData g_NEXUS_audioModuleData;

/******************************************************************************
The array to represent the value of volume in hex corresponding to the value 
in DB. The application inputs the volume in terms of DB and the Corresponding
HEX value is mentioned here. The formula used for the same is:

    HEX = (2^23) * 10^(DB/20)

Note: 23 is the number of bits in the volume control field. 

The volume can range from 0-1. 0 in hex corresponds to the 139 DB from the above
Formula. If application enters more than this value, it is forced to 0.
******************************************************************************/
static const int32_t g_db2linear[] = 
{
    0x800000,	0x721482,	0x65AC8C,	0x5A9DF7,	0x50C335,
    0x47FACC,	0x4026E7,	0x392CED,	0x32F52C,	0x2D6A86,
    0x287A26,	0x241346,	0x2026F3,	0x1CA7D7,	0x198A13,
    0x16C310,	0x144960,	0x12149A,	0x101D3F,	0xE5CA1,
    0xCCCCC,	0xB6873,	0xA2ADA,	0x90FCB,	0x81385,
    0x732AE,	0x66A4A,	0x5B7B1,	0x51884,	0x48AA7,
    0x40C37,	0x39B87,	0x33718,	0x2DD95,	0x28DCE,
    0x246B4,	0x20756,	0x1CEDC,	0x19C86,	0x16FA9,
    0x147AE,	0x1240B,	0x10449,	0xE7FA,		0xCEC0,
    0xB844,		0xA43A,		0x925E,		0x8273,		0x7443,
    0x679F,		0x5C5A,		0x524F,		0x495B,		0x4161,
    0x3A45,		0x33EF,		0x2E49,		0x2940,		0x24C4,
    0x20C4,		0x1D34,		0x1A07,		0x1732,		0x14AC,
    0x126D,		0x106C,		0xEA3,		0xD0B,		0xBA0,
    0xA5C,		0x93C,		0x83B,		0x755,		0x689,
    0x5D3,		0x531,		0x4A0,		0x420,		0x3AD,
    0x346,		0x2EB,		0x29A,		0x251,		0x211,
    0x1D7,		0x1A4,		0x176,		0x14D,		0x129,
    0xEC,		0xD2,		0xA7,		0x95,		0x84,
    0x76,		0x69,		0x5E,		0x53,		0x4A,
    0x42,		0x3B,		0x34,		0x2F,		0x2A,
    0x25,		0x21,		0x1D,		0x1A,		0x17,
    0x15,		0x12,		0x10,		0xE,		0xD,
    0xB,		0xA,		0x9,		0x8,		0x7,
    0x6,		0x5,		0x5,		0x4,		0x4,
    0x3,		0x3,		0x2,		0x2,		0x2,
    0x2,		0x1,		0x1,		0x1,		0x1,
    0x1,		0x1,		0x1,		0x0
};

/* TODO: These should be linear, but are linearly scaling to dB - which will be exponential */
int32_t NEXUS_AudioModule_P_Vol2Magnum(NEXUS_AudioVolumeType type, int32_t volume)
{
    if ( type == NEXUS_AudioVolumeType_eDecibel )
    {
        if ( volume > 0 )
        {
            BDBG_ERR(("Currently, amplification is not supported for dB values.  Clipping to 0 dB"));
            return 0;
        }
        else if ( volume < NEXUS_AUDIO_VOLUME_DB_MIN )
        {
            BDBG_ERR(("Clipping out of range volume to minimum"));
            return NEXUS_AUDIO_VOLUME_DB_MIN;
        }
        return -volume;
    }
    else
    {
        int i;
        /* 7401/7400 do not have direct linear APIs and it do not support amplification.  */
        if ( volume > NEXUS_AUDIO_VOLUME_LINEAR_NORMAL )
        {
            BDBG_ERR(("This platform does not support amplification.  Volume will be set to normal"));
            return NEXUS_AUDIO_VOLUME_DB_NORMAL;
        }
        for ( i = 0; i < (int)(sizeof(g_db2linear)/sizeof(int32_t)); i++ )
        {
            if ( g_db2linear[i] <= volume )
            {
                break;
            }
        }
        if ( i >= (int)(sizeof(g_db2linear)/sizeof(int32_t)) )
        {
            return NEXUS_AUDIO_VOLUME_DB_MIN;
        }
        else if ( i == 0 )
        {
            return 0;
        }
        else
        {
            /* We have found the first value <= the current value. */
            /* Use raptor's formula in reverse to average between this value and the greater value (previous entry) */
            /* This will result in some values not being possible, but should be generally correct */
            volume -= g_db2linear[i];
            return (i*100)+((100*volume)/(g_db2linear[i-1]-g_db2linear[i]));
        }
    }
}

NEXUS_ModuleHandle NEXUS_AudioModule_Init(const NEXUS_AudioModuleSettings *pSettings)
{
    BERR_Code errCode;
    NEXUS_ModuleSettings moduleSettings;
    BRAP_Settings *pRapSettings;

    BDBG_ASSERT(NULL != pSettings); /* no default */
    BDBG_ASSERT(NULL != pSettings->modules.transport);
    BDBG_ASSERT(NULL == g_NEXUS_audioModule);

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow; /* decoder interface is slow */
    g_NEXUS_audioModule = NEXUS_Module_Create("audio", &moduleSettings);
    if ( NULL == g_NEXUS_audioModule ) 
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }
    NEXUS_LockModule();
    /* Save transport handle */
    g_NEXUS_audioModuleData.transport = pSettings->modules.transport;

    /* Get defaults if required */
    if ( NULL == pSettings )
    {
        NEXUS_AudioModule_GetDefaultSettings(NULL, &g_NEXUS_audioModuleData.moduleSettings);
    }
    else
    {
        g_NEXUS_audioModuleData.moduleSettings = *pSettings;
    }

    /* Initialize Raptor */
    pRapSettings = BKNI_Malloc(sizeof(BRAP_Settings));
    if ( NULL == pRapSettings )
    {
        errCode=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    BRAP_GetDefaultSettings(pRapSettings);

    pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eMpeg] = true;
#ifdef B_RAP_NO_AAC_SUPPORT
	pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eAac] = false;
	BDBG_WRN(("NO_AAC_SUPPORT"));
#else
    pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eAac] = true;
#endif
#ifdef B_RAP_NO_AACPLUS_SUPPORT
	pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eAacSbr] = false;
#else
    pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eAacSbr] = true;
#endif
#ifdef B_RAP_NO_AC3_SUPPORT
	pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eAc3] = false;
#else
    pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eAc3] = true;
#endif
#ifdef B_RAP_NO_AC3PLUS_SUPPORT
	pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eAc3Plus] = false;
#else
	pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eAc3Plus] = true;
#endif
#ifdef B_RAP_NO_WMA_SUPPORT
	pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eWmaStd] = false;
	pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eWmaPro] = false;
#else
	pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eWmaStd] = true;
    pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eWmaPro] = true;
#endif
    pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eLpcmDvd] = true;  

#if (BCHP_CHIP==7400)
	pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_ePcmWav] = true;  
	/* 7400 can pass thru, not decode DTS */
	pRapSettings->bSupportAlgos[BRAP_DSPCHN_AudioType_eDts] = true;  
#endif

    pRapSettings->bSupportPpAlgos[BRAP_DSPCHN_PP_Algo_eSrc] = true; /* enables MPEG/AAC LSF */

#if NEXUS_DOLBYVOL
    pRapSettings->bSupportPpAlgos[BRAP_DSPCHN_PP_Algo_eDolbyVolume] = true; /* enables Dolby Volume */
#endif
#if NEXUS_SRSTRUVOL
    pRapSettings->bSupportPpAlgos[BRAP_DSPCHN_PP_Algo_eSRS_TruVol] = true; /* SRS TruVolume */
#endif
    /* TODO: How to handle bimage */
    pRapSettings->pImgInterface = &BRAP_IMG_Interface;
    pRapSettings->pImgContext = BRAP_IMG_Context;

    BDBG_MSG(("Opening RAP"));
    /* Open the RAP device (first instance only) */
    errCode = BRAP_Open(&g_NEXUS_audioModuleData.hRap,
                        g_pCoreHandles->chp,
                        g_pCoreHandles->reg,
                        g_pCoreHandles->heap[0].mem,
                        g_pCoreHandles->bint,
                        g_pCoreHandles->tmr,
                        pRapSettings);

    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_rap;
    }    

    if(pRapSettings->bFwAuthEnable == true)   
    {
        /* Open the RAP device (first instance only) */
        errCode = BRAP_StartDsp(g_NEXUS_audioModuleData.hRap);
        if ( errCode )
        {
            errCode=BERR_TRACE(errCode);
            goto err_rap;
        }  
    }
    
    BDBG_MSG(("Initializing DACS"));
    errCode = NEXUS_AudioDac_P_Init();
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_dac;
    }

    #if NEXUS_NUM_SPDIF_OUTPUTS
    BDBG_MSG(("Initializing SPDIF Outputs"));
    errCode = NEXUS_SpdifOutput_P_Init();
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_spdif;
    }
    #endif

    #if NEXUS_NUM_AUDIO_DECODERS
    BDBG_MSG(("Initializing Decoders"));
    errCode = NEXUS_AudioDecoder_P_Init(pRapSettings->bSupportAlgos);
    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
        goto err_decoder;
    }
    #endif

    #if NEXUS_SRSTRUVOL
    (void)NEXUS_TruVolume_P_Init();
    #endif
    #if NEXUS_DOLBYVOL
    (void)NEXUS_DolbyVolume_P_Init();
    #endif

    /* Success */
    BKNI_Free(pRapSettings);
    NEXUS_UnlockModule();
    return g_NEXUS_audioModule;
err_spdif:
#if NEXUS_NUM_AUDIO_DECODERS
err_decoder:
#endif
err_dac:
    BRAP_Close(g_NEXUS_audioModuleData.hRap);
err_rap:
    BKNI_Free(pRapSettings);
err_malloc:
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_audioModule);
    return NULL;
}

void NEXUS_AudioModule_Uninit(void)
{
    NEXUS_LockModule();  
    #if NEXUS_SRSTRUVOL
    NEXUS_TruVolume_P_Uninit();
    #endif
    #if NEXUS_DOLBYVOL
    NEXUS_DolbyVolume_P_Uninit();
    #endif
    #if NEXUS_NUM_AUDIO_DECODERS
    NEXUS_AudioDecoder_P_Uninit();
    #endif
    BRAP_Close(g_NEXUS_audioModuleData.hRap);
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_audioModule);    
	g_NEXUS_audioModule = NULL;
	BKNI_Memset(&g_NEXUS_audioModuleData, 0, sizeof(g_NEXUS_audioModuleData));
}

void NEXUS_AudioModule_GetDefaultSettings(
    const struct NEXUS_Core_PreInitState *preInitState,
    NEXUS_AudioModuleSettings *pSettings    /* [out] */
    )
{
    BSTD_UNUSED(preInitState);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->watchdogEnabled = true;
    if ( NEXUS_GetEnv("no_independent_delay") )
    {
        pSettings->independentDelay = false; /* Disable independent delay if requested */
    }
    else
    {
        pSettings->independentDelay = true; /* Enable independent delay by default */
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

NEXUS_Error NEXUS_AudioModule_SetRampStepSettings(
    const NEXUS_AudioRampStepSettings *pRampStepSettings  /* ramping step size for scale change for all output ports */
    )
{
    BDBG_ASSERT(NULL != g_NEXUS_audioModule);
    BSTD_UNUSED(pRampStepSettings);
    return NEXUS_NOT_SUPPORTED;      /* Not supported on 7400 */
}
void NEXUS_AudioModule_GetRampStepSettings(
    NEXUS_AudioRampStepSettings *pRampStepSettings      /* [out] ramping step size for scale change for all output ports */
    )
{
    BDBG_ASSERT(NULL != g_NEXUS_audioModule);
    pRampStepSettings = NULL;      /* Not supported on 7400 */
    return;
}
NEXUS_Error NEXUS_AudioModule_EnableExternalMclk(
    unsigned mclkIndex,
    NEXUS_AudioOutputPll pllIndex,
    NEXUS_ExternalMclkRate mclkRate
    )
{
    BERR_Code errCode;
    BRAP_OP_ExtMClkSettings mclkSettings;
    mclkSettings.ePll = pllIndex;
    mclkSettings.eMClockRate = mclkRate;
    errCode = BRAP_OP_ExtMClkConfig(g_NEXUS_audioModuleData.hRap, mclkIndex, &mclkSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

#include "bchp_common.h"
#include "bchp_aud_fmm_misc.h"

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

#if defined BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_END
#define NUM_STCS (BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_END + 1)
#else
#define NUM_STCS 0
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
    pCaps->numStcs = NUM_STCS;

    pCaps->numDsps = 1; /* All RAP STB chips are single-dsp */

    #ifndef B_RAP_NO_AC3PLUS_SUPPORT
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAc3].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAc3Plus].decode = true;
    #endif
    #ifndef B_RAP_NO_AC3_SUPPORT
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAc3].decode = true;
    #endif
    #ifdef RAP_PCMWAV_SUPPORT
    pCaps->dsp.codecs[NEXUS_AudioCodec_ePcmWav].decode = true;
    #endif
    pCaps->dsp.codecs[NEXUS_AudioCodec_eMpeg].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eMp3].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAacAdts].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAacLoas].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAacPlusAdts].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eAacPlusLoas].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eLpcmDvd].decode = true;
    #ifndef B_RAP_NO_WMA_SUPPORT
    pCaps->dsp.codecs[NEXUS_AudioCodec_eWmaStd].decode = true;
    pCaps->dsp.codecs[NEXUS_AudioCodec_eWmaPro].decode = true;
    #endif

    #if NEXUS_DOLBYVOL
    pCaps->dsp.dolbyVolume = true;
    #endif
    #if NEXUS_SRSTRUVOL
    pCaps->dsp.truVolume = true;
    #endif
}

static NEXUS_AudioOutputPllSettings g_pllSettings[NUM_PLLS];

void NEXUS_AudioOutputPll_GetSettings(
    NEXUS_AudioOutputPll pll,
    NEXUS_AudioOutputPllSettings *pSettings       /* [out] Current Settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    if ( pll < NUM_PLLS )
    {
        *pSettings = g_pllSettings[pll];
    }
    else
    {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    }
}

NEXUS_Error NEXUS_AudioOutputPll_SetSettings(
    NEXUS_AudioOutputPll pll,
    const NEXUS_AudioOutputPllSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    if ( pll < NUM_PLLS )
    {
        g_pllSettings[pll] = *pSettings;
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    return BERR_SUCCESS;
}

void NEXUS_GetAudioFirmwareVersion(
    NEXUS_AudioFirmwareVersion *pVersion /* [out] */
    )
{
    BRAP_RevisionInfo versionInfo;

    BDBG_ASSERT(NULL != pVersion);
    BKNI_Memset(pVersion, 0, sizeof(*pVersion));

    if ( BERR_SUCCESS == BRAP_GetRevision(g_NEXUS_audioModuleData.hRap, &versionInfo) )
    {
        pVersion->major = versionInfo.sPIVersion.ui32MajorVersion;
        pVersion->minor = versionInfo.sPIVersion.ui32MinorVersion;
    }
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

