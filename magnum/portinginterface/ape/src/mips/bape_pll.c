/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_pll0.h"
#if BAPE_CHIP_MAX_PLLS > 1
#include "bchp_aud_fmm_pll1.h"
#define BAPE_PLL_STRIDE (BCHP_AUD_FMM_PLL1_MACRO-BCHP_AUD_FMM_PLL0_MACRO)
#if BAPE_CHIP_MAX_PLLS > 2
#include "bchp_aud_fmm_pll2.h"
#endif
#else
#define BAPE_PLL_STRIDE 0
#endif

BDBG_MODULE(bape_pll);

void BAPE_Pll_GetSettings(
    BAPE_Handle handle,
    BAPE_Pll pll,
    BAPE_PllSettings *pSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);
    *pSettings = handle->pllStatus[pll].settings;
}

BERR_Code BAPE_Pll_SetSettings(
    BAPE_Handle handle,
    BAPE_Pll pll,
    const BAPE_PllSettings *pSettings
    )
{
    uint32_t regVal, regAddr;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);
    BDBG_ASSERT(NULL != pSettings);
    handle->pllStatus[pll].settings = *pSettings;

    regAddr = BCHP_AUD_FMM_PLL0_CONTROL + (BAPE_PLL_STRIDE * pll);

    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT);
    if ( pSettings->freeRun )
    {
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, CLKGEN);
    }
    else
    {
        /* VCXO source */
        switch ( pSettings->vcxo )
        {
        case 0:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, VCXO_0);
            break;
#ifdef BCHP_AUD_FMM_PLL0_CONTROL_REFERENCE_SELECT_VCXO_1
        case 1:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, VCXO_1);            
            break;
#endif
        default:
            BDBG_ERR(("Invalid VCXO number (%d) for PLL %d", pSettings->vcxo, pll));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    BREG_Write32(handle->regHandle, regAddr, regVal);

    return BERR_SUCCESS;
}

void BAPE_P_AttachMixerToPll(BAPE_MixerHandle mixer, BAPE_Pll pll)
{
    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);
    BLST_S_INSERT_HEAD(&mixer->deviceHandle->pllStatus[pll].mixerList, mixer, pllNode);
}

void BAPE_P_DetachMixerFromPll(BAPE_MixerHandle mixer, BAPE_Pll pll)
{
    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);
    BLST_S_REMOVE(&mixer->deviceHandle->pllStatus[pll].mixerList, mixer, BAPE_Mixer, pllNode);
}

static BERR_Code BAPE_P_GetPllBaseSampleRate_isr(BAVC_AudioSamplingRate sampleRate, BAVC_AudioSamplingRate *pBaseRate)
{
    switch ( sampleRate )
    {
    case BAVC_AudioSamplingRate_e32k:    /* 32K Sample rate */
    case BAVC_AudioSamplingRate_e64k:      /* 64K Sample rate */
    case BAVC_AudioSamplingRate_e128k:     /* 128K Sample rate */
        *pBaseRate = BAVC_AudioSamplingRate_e32k;
        return BERR_SUCCESS;
    case BAVC_AudioSamplingRate_e44_1k:    /* 44.1K Sample rate */
    case BAVC_AudioSamplingRate_e88_2k:    /* 88.2K Sample rate */
    case BAVC_AudioSamplingRate_e176_4k:   /* 176.4K Sample rate */
        *pBaseRate = BAVC_AudioSamplingRate_e44_1k;
        return BERR_SUCCESS;
    case BAVC_AudioSamplingRate_e48k:      /* 48K Sample rate */
    case BAVC_AudioSamplingRate_e96k:      /* 96K Sample rate */
    case BAVC_AudioSamplingRate_e192k:     /* 19296K Sample rate */
        *pBaseRate = BAVC_AudioSamplingRate_e48k;
        return BERR_SUCCESS;
    default:
        BDBG_ERR(("Invalid sampling rate %u", sampleRate));
        *pBaseRate = BAVC_AudioSamplingRate_eUnknown;
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
}

BERR_Code BAPE_P_UpdatePll_isr(BAPE_Handle handle, BAPE_Pll pll)
{
    BAVC_AudioSamplingRate baseRate = BAVC_AudioSamplingRate_eUnknown;
    BAVC_AudioSamplingRate idleRate = BAVC_AudioSamplingRate_eUnknown;
    BAPE_Mixer *pMixer;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);

    /* Walk through each mixer and make sure we have no conflicts */
    for ( pMixer = BLST_S_FIRST(&handle->pllStatus[pll].mixerList);
          pMixer != NULL;
          pMixer = BLST_S_NEXT(pMixer, pllNode) )
    {
        BAVC_AudioSamplingRate mixerRate;

        if ( pMixer->sampleRate == BAVC_AudioSamplingRate_eUnknown )
        {
            continue;
        }

        errCode = BAPE_P_GetPllBaseSampleRate_isr(pMixer->sampleRate, &mixerRate);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        if ( pMixer->running )
        {
            if ( baseRate == BAVC_AudioSamplingRate_eUnknown )
            {
                baseRate = mixerRate;
            }
            else if ( baseRate != mixerRate )
            {
                BDBG_WRN(("Sample rate conflict on PLL %d.  One mixer requests %s another requests %s. No change until conflict is resolved.",
                          pll, BAPE_P_GetSampleRateString_isrsafe(baseRate), BAPE_P_GetSampleRateString_isrsafe(mixerRate)));
                return BERR_SUCCESS;
            }
        }
        else if ( idleRate == BAVC_AudioSamplingRate_eUnknown )
        {
            idleRate = mixerRate;
        }
    }

    if ( baseRate == BAVC_AudioSamplingRate_eUnknown )
    {
        baseRate = idleRate;
    }

    if ( baseRate != BAVC_AudioSamplingRate_eUnknown )
    {
        uint32_t regVal, regAddr;
        unsigned baseRateNum;
        regAddr = BCHP_AUD_FMM_PLL0_MACRO + (BAPE_PLL_STRIDE * pll);
        regVal = BREG_Read32(handle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_PLL0_MACRO, MACRO_SELECT);
        switch ( baseRate )
        {
        case BAVC_AudioSamplingRate_e32k:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_MACRO, MACRO_SELECT, Mult_of_32000);
            break;
        case BAVC_AudioSamplingRate_e44_1k:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_MACRO, MACRO_SELECT, Mult_of_44100);
            break;
        case BAVC_AudioSamplingRate_e48k:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_MACRO, MACRO_SELECT, Mult_of_48000);
            break;
        default:
            BDBG_ASSERT(0); /* Should never get here */
            break;
        }
        BREG_Write32(handle->regHandle, regAddr, regVal);

        baseRateNum = BAPE_P_GetSampleRateValue_isrsafe(baseRate);
        BDBG_ASSERT(baseRateNum != 0);

        BDBG_MSG(("Update PLL %u base sample rate to %u Hz", pll, baseRateNum));

        /* For each output, set it's mclk appropriately */
        for ( pMixer = BLST_S_FIRST(&handle->pllStatus[pll].mixerList);
              pMixer != NULL;
              pMixer = BLST_S_NEXT(pMixer, pllNode) )
        {
            BAPE_MixerOutput output;
            unsigned multiplier;
            unsigned rateNum;
            BAPE_MclkSource mclkSource;
            switch ( pll )
            {
            default:
            case BAPE_Pll_e0:
                mclkSource = BAPE_MclkSource_ePll0;
                break;
            case BAPE_Pll_e1:
                mclkSource = BAPE_MclkSource_ePll1;
                break;
            case BAPE_Pll_e2:
                mclkSource = BAPE_MclkSource_ePll2;
                break;
            }
            rateNum = BAPE_P_GetSampleRateValue_isrsafe(pMixer->sampleRate);
            /* SW7408-199 20110111 bandrews - ensure rate is nonzero before
            programming */
            if (rateNum)
            {
                multiplier = rateNum/baseRateNum;
                for ( output = BLST_S_FIRST(&pMixer->outputList);
                      output != NULL;
                      output = BLST_S_NEXT(output, node) )
                {
                    if ( output->setMclk_isr )
                    {
                        output->setMclk_isr(output->pHandle, mclkSource, multiplier);
                    }
                }
                if ( pMixer->fs != BAPE_FS_INVALID )
                {
                    BAPE_P_SetFsTiming_isr(handle, pMixer->fs, mclkSource, multiplier);
                }
            }
        }
    }
    else
    {
        BDBG_MSG(("Not updating PLL %u rate (unknown)", pll));
    }

    return BERR_SUCCESS;
}

