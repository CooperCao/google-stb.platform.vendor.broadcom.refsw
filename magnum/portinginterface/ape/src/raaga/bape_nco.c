/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 *****************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"

#if BCHP_CLKGEN_REG_START
#include "bchp_clkgen.h"
#endif

BDBG_MODULE(bape_nco);

#if BAPE_CHIP_MAX_NCOS > 0     /* If no NCOs on this chip, then skip all of this.  None of these functions are ever called. */

#ifdef BCHP_AUD_FMM_IOP_NCO_0_REG_START
#include "bchp_aud_fmm_iop_nco_0.h"
#ifdef BCHP_AUD_FMM_IOP_NCO_1_REG_START
#include "bchp_aud_fmm_iop_nco_1.h"
#define BAPE_NCO_STRIDE (BCHP_AUD_FMM_IOP_NCO_1_REG_START - BCHP_AUD_FMM_IOP_NCO_0_REG_START)
#else
#define BAPE_NCO_STRIDE 0
#endif
#endif

#ifdef BCHP_AUD_FMM_OP_MCLKGEN_REG_START
#include "bchp_aud_fmm_op_mclkgen.h"
#if BAPE_CHIP_MAX_NCOS > 1
#define BAPE_NCO_STRIDE (BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_1_SAMPLE_INC - BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_SAMPLE_INC)
#else
#define BAPE_NCO_STRIDE 0
#endif
#endif

#if defined BCHP_AUD_FMM_OP_MCLKGEN_REG_START
static void BAPE_Nco_UpdateDividers_isr(BAPE_Handle handle, BAPE_Nco nco, uint32_t sampleInc, uint32_t  numerator, uint32_t denominator, uint32_t phaseInc, BAVC_Timebase outputTimebase)
{
    uint32_t regAddr, regVal;

    regAddr = BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL + (BAPE_NCO_STRIDE * nco);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE);
    switch ( outputTimebase )
    {
    case BAVC_Timebase_e0:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_0);
        break;
    case BAVC_Timebase_e1:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_1);
        break;
    case BAVC_Timebase_e2:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_2);
        break;
    case BAVC_Timebase_e3:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_3);
        break;
    case BAVC_Timebase_e4:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_4);
        break;
    case BAVC_Timebase_e5:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_5);
        break;
    case BAVC_Timebase_e6:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_6);
        break;
    case BAVC_Timebase_e7:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_7);
        break;
    case BAVC_Timebase_e8:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_8);
        break;
    case BAVC_Timebase_e9:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_9);
        break;
    case BAVC_Timebase_e10:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_10);
        break;
    case BAVC_Timebase_e11:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_11);
        break;
    case BAVC_Timebase_e12:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_12);
        break;
    case BAVC_Timebase_e13:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_13);
        break;
    default:
    case BAVC_Timebase_eMax:
        BDBG_ERR(("%s: invalid timebase (%d)", __FUNCTION__, outputTimebase));
        BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    regAddr = BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_RATE_RATIO + (BAPE_NCO_STRIDE * nco);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_RATE_RATIO, DENOMINATOR);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_RATE_RATIO, DENOMINATOR, denominator);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    regAddr = BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_SAMPLE_INC + (BAPE_NCO_STRIDE * nco);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_SAMPLE_INC, SAMPLE_INC) | BCHP_MASK(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_SAMPLE_INC, NUMERATOR));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_SAMPLE_INC, NUMERATOR, numerator);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_SAMPLE_INC, SAMPLE_INC, sampleInc);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    regAddr = BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_PHASE_INC + (BAPE_NCO_STRIDE * nco);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_PHASE_INC, PHASE_INC);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_PHASE_INC, PHASE_INC, phaseInc);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
}
#elif defined BCHP_AUD_FMM_IOP_NCO_0_REG_START
static void BAPE_Nco_UpdateDividers_isr(BAPE_Handle handle, BAPE_Nco nco, uint32_t sampleInc, uint32_t  numerator, uint32_t denominator, uint32_t phaseInc, BAVC_Timebase outputTimebase)
{
    uint32_t regAddr, regVal;

    regAddr = BCHP_AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL + (BAPE_NCO_STRIDE * nco);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE);
    switch ( outputTimebase )
    {
    case BAVC_Timebase_e0:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_0);
        break;
    case BAVC_Timebase_e1:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_1);
        break;
    case BAVC_Timebase_e2:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_2);
        break;
    case BAVC_Timebase_e3:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_3);
        break;
    case BAVC_Timebase_e4:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_4);
        break;
    case BAVC_Timebase_e5:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_5);
        break;
    case BAVC_Timebase_e6:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_6);
        break;
    case BAVC_Timebase_e7:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_7);
        break;
    case BAVC_Timebase_e8:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_8);
        break;
    case BAVC_Timebase_e9:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_9);
        break;
    case BAVC_Timebase_e10:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_10);
        break;
    case BAVC_Timebase_e11:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_11);
        break;
    case BAVC_Timebase_e12:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_12);
        break;
    case BAVC_Timebase_e13:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_CONTROL, TIMEBASE, TIMEBASE_13);
        break;
    default:
    case BAVC_Timebase_eMax:
        BDBG_ERR(("%s: invalid timebase (%d)", __FUNCTION__, outputTimebase));
        BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    regAddr = BCHP_AUD_FMM_IOP_NCO_0_MCLK_GEN_0_RATE_RATIO + (BAPE_NCO_STRIDE * nco);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_RATE_RATIO, DENOMINATOR);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_RATE_RATIO, DENOMINATOR, denominator);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    regAddr = BCHP_AUD_FMM_IOP_NCO_0_MCLK_GEN_0_SAMPLE_INC + (BAPE_NCO_STRIDE * nco);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_SAMPLE_INC, SAMPLE_INC) | BCHP_MASK(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_SAMPLE_INC, NUMERATOR));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_SAMPLE_INC, NUMERATOR, numerator);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_SAMPLE_INC, SAMPLE_INC, sampleInc);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    regAddr = BCHP_AUD_FMM_IOP_NCO_0_MCLK_GEN_0_PHASE_INC + (BAPE_NCO_STRIDE * nco);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_PHASE_INC, PHASE_INC);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_NCO_0_MCLK_GEN_0_PHASE_INC, PHASE_INC, phaseInc);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
}
#else
#error Unknown NCO register layout
#endif



void BAPE_P_AttachMixerToNco(BAPE_MixerHandle mixer, BAPE_Nco nco)
{
    unsigned ncoIndex = nco - BAPE_Nco_e0;

    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    if ( ncoIndex >= BAPE_CHIP_MAX_NCOS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(ncoIndex < BAPE_CHIP_MAX_NCOS);
        return;
    }

    BDBG_MSG(("Attaching mixer %p to NCO:%u", (void *)mixer, ncoIndex ));
    BLST_S_INSERT_HEAD(&mixer->deviceHandle->audioNcos[ncoIndex].mixerList, mixer, ncoNode);
    /* Update MCLK source for attached outputs */
    BKNI_EnterCriticalSection();
    BAPE_P_UpdateNco_isr(mixer->deviceHandle, nco);
    BKNI_LeaveCriticalSection();
}

void BAPE_P_DetachMixerFromNco(BAPE_MixerHandle mixer, BAPE_Nco nco)
{
    unsigned ncoIndex = nco - BAPE_Nco_e0;

    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    if ( ncoIndex >= BAPE_CHIP_MAX_NCOS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(ncoIndex < BAPE_CHIP_MAX_NCOS);
        return;
    }
    BDBG_MSG(("Detaching mixer %p from NCO:%u", (void *)mixer, ncoIndex ));
    BLST_S_REMOVE(&mixer->deviceHandle->audioNcos[ncoIndex].mixerList, mixer, BAPE_Mixer, ncoNode);
}

static BERR_Code BAPE_P_SetNcoFreq_isr( BAPE_Handle handle, BAPE_Nco nco, unsigned baseRate, unsigned *pOversample, BAVC_Timebase outputTimebase )
{
    unsigned ncoIndex = nco - BAPE_Nco_e0;
    int i;

    /* The following table prioritizes faster mclk rates over slower, allowing for base rates to also support 4x multiples. */
    struct ncoInfo {
            unsigned baseFs; int oversample; long ncoFreq; int sampleInc; long numerator; int denominator; int phaseInc;
    } ncoInfo[] =
    {       /* Multiples of 32 KHz */
            {  32000,              512,        16384000,          1,           1327,           2048,          0x136B06  },
            {  32000,              256,         8192000,          3,            303,           1024,          0x09b583  },
            {  32000,              128,         4096000,          6,            606,           1024,          0x04DAC1  },
            {  64000,              256,        16384000,          1,           1327,           2048,          0x136B06  },
            {  64000,              128,         8192000,          3,            303,           1024,          0x09b583  },
            { 128000,              128,        16384000,          1,           1327,           2048,          0x136B06  },
            { 128000,              256,        32768000,          0,           3375,           4096,          0x26D60D  },

            /* Multiples of 44.1 KHz */
            {  44100,              512,        22579200,          1,            307,           1568,          0x1AC2B2  },
            {  44100,              256,        11289600,          2,            307,            784,          0x0d6159  },
            {  44100,              128,        5644800,           4,            307,            392,          0x06B0AC  },
            {  88200,              256,        22579200,          1,            307,           1568,          0x1AC2B2  },
            {  88200,              128,        11289600,          2,            307,            784,          0x0d6159  },
            { 176400,              128,        22579200,          1,            307,           1568,          0x1AC2B2  },
            { 176400,              256,        45158400,          0,           1875,           3136,          0x358564  },

            /* Multiples of 48 KHz */
            {  48000,              512,        24576000,          1,            101,           1024,          0x1D208A  },
            {  48000,              256,        12288000,          2,            101,            512,          0x0e9045  },
            {  48000,              128,        6144000,           4,            101,            256,          0x074822  },
            {  96000,              256,        24576000,          1,            101,           1024,          0x1D208A  },
            {  96000,              128,        12288000,          2,            101,            512,          0x0e9045  },
            { 192000,              128,        24576000,          1,            101,           1024,          0x1D208A  },
            { 192000,              256,        49152000,          0,           1125,           2048,          0x3A4114  },
    };
    int numElems = sizeof ncoInfo/sizeof ncoInfo[0];

    if (pOversample ) *pOversample = 0;     /* set to zero in case of early return */

    /* Search the table above to find the first entry for our baseRate.  There may be several entries for
     * the baseRate, but for now, just use the first one we find.  */
    for ( i=0 ; i<numElems ; i++  )
    {
        if ( ncoInfo[i].baseFs ==  baseRate )
        {
            break;
        }
    }

    if ( i >= numElems )
    {
        BDBG_ERR(("Can't find ncoInfo for sampling rate %u", baseRate));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(ncoInfo[i].baseFs ==  baseRate);

    BDBG_MSG(("Setting NCO %u frequency to %lu Hz (%u * %u)", nco,  ncoInfo[i].ncoFreq, ncoInfo[i].oversample, ncoInfo[i].baseFs));

    BAPE_Nco_UpdateDividers_isr(handle, nco, ncoInfo[i].sampleInc, ncoInfo[i].numerator, ncoInfo[i].denominator, ncoInfo[i].phaseInc, outputTimebase);
    handle->audioNcos[ncoIndex].baseSampleRate   = baseRate;
    handle->audioNcos[ncoIndex].ncoFreq          = ncoInfo[i].ncoFreq;
    handle->audioNcos[ncoIndex].timebase         = outputTimebase;

    if (pOversample ) *pOversample = ncoInfo[i].oversample;   /* pass oversample factor back to caller. */

    return BERR_SUCCESS;
}

BERR_Code BAPE_P_GetNcoConfiguration(BAPE_Handle handle, BAPE_Nco nco, BAPE_NcoConfiguration * pConfig)
{
    unsigned ncoIndex = nco - BAPE_Nco_e0;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT( pConfig != NULL );

    BKNI_Memset(pConfig, 0, sizeof(*pConfig));

    if ( ncoIndex >= BAPE_CHIP_MAX_NCOS )
    {
        BDBG_ASSERT(ncoIndex < BAPE_CHIP_MAX_NCOS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pConfig->baseFs = handle->audioNcos[ncoIndex].baseSampleRate;
    pConfig->frequency = handle->audioNcos[ncoIndex].ncoFreq;
    pConfig->timebase = handle->audioNcos[ncoIndex].timebase;

    return BERR_SUCCESS;
}

BERR_Code BAPE_P_UpdateNco_isr(BAPE_Handle handle, BAPE_Nco nco)
{
    unsigned ncoIndex = nco - BAPE_Nco_e0;
    unsigned baseRate = 0;
    unsigned idleRate = 0;
    BAVC_Timebase outputTimebase = (unsigned) -1;
    bool          gotTimebase = false;
    BAPE_Mixer *pMixer;
    BAPE_Mixer *pLastMixer = NULL;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    if ( ncoIndex >= BAPE_CHIP_MAX_NCOS )
    {
        BDBG_ASSERT(ncoIndex < BAPE_CHIP_MAX_NCOS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Walk through each mixer and make sure we have no conflicts */
    for ( pMixer = BLST_S_FIRST(&handle->audioNcos[ncoIndex].mixerList);
          pMixer != NULL;
          pMixer = BLST_S_NEXT(pMixer, ncoNode) )
    {
        unsigned mixerRate;

        if ( BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer) == 0 )
        {
            continue;
        }

        if ( ! gotTimebase )
        {
            outputTimebase = pMixer->settings.outputTimebase;
            gotTimebase = true;
        }
        else if (pMixer->settings.outputTimebase != outputTimebase )
        {
            BDBG_WRN(("Timebase conflict on NCO %d.  One mixer requests timebase %u another requests timebase %u", nco, outputTimebase, pMixer->settings.outputTimebase));
        }

        mixerRate = BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer);

        if ( pMixer->running )
        {
            if ( baseRate == 0 )
            {
                baseRate = mixerRate;
            }
            else if ( baseRate != mixerRate )
            {
                BAPE_OutputPort outputPort;
                BDBG_WRN(("Sample rate conflict on NCO %d between mixer %p and mixer %p", nco, (void *)pLastMixer, (void *)pMixer));
                BDBG_WRN(("  Mixer %p: requests %u Hz", (void *)pLastMixer, baseRate));
                for ( outputPort = BLST_S_FIRST(&pLastMixer->outputList);
                    outputPort != NULL;
                    outputPort = BLST_S_NEXT(outputPort, node) )
                {
                    BDBG_WRN(("    --> %s Output", outputPort->pName));
                }
                BDBG_WRN(("  Mixer %p: requests %u Hz", (void *)pMixer, mixerRate));
                for ( outputPort = BLST_S_FIRST(&pMixer->outputList);
                    outputPort != NULL;
                    outputPort = BLST_S_NEXT(outputPort, node) )
                {
                    BDBG_WRN(("    --> %s Output", outputPort->pName));
                }
            }
        }
        else if ( idleRate == 0 )
        {
            idleRate = mixerRate;
        }
        else if ( idleRate != mixerRate )
        {
            BAPE_OutputPort outputPort;
            BDBG_WRN(("Sample rate conflict on NCO %d between mixer %p and mixer %p", nco, (void *)pLastMixer, (void *)pMixer));
            BDBG_WRN(("  Mixer %p: requests %u Hz", (void *)pLastMixer, idleRate));
            for ( outputPort = BLST_S_FIRST(&pLastMixer->outputList);
                outputPort != NULL;
                outputPort = BLST_S_NEXT(outputPort, node) )
            {
                BDBG_WRN(("    --> %s Output", outputPort->pName));
            }
            BDBG_WRN(("  Mixer %p: requests %u Hz", (void *)pMixer, mixerRate));
            for ( outputPort = BLST_S_FIRST(&pMixer->outputList);
                outputPort != NULL;
                outputPort = BLST_S_NEXT(outputPort, node) )
            {
                BDBG_WRN(("    --> %s Output", outputPort->pName));
            }
        }

        pLastMixer = pMixer;
    }

    if ( baseRate == 0 )
    {
        baseRate = idleRate;
    }

    if ( baseRate != 0 )
    {
        uint32_t oversample;

        BDBG_MSG(("Updating NCO %u for base sample rate of %u Hz, timebase %u", nco, baseRate, outputTimebase));
        errCode = BAPE_P_SetNcoFreq_isr( handle, nco, baseRate, &oversample, outputTimebase );
        if ( errCode )
        {
            BDBG_ERR(("Failed to set NCO %u for sample rate of %u Hz", nco, baseRate));
            return BERR_TRACE(errCode);
        }

        /* For each output, set it's mclk appropriately */
        for ( pMixer = BLST_S_FIRST(&handle->audioNcos[ncoIndex].mixerList);
              pMixer != NULL;
              pMixer = BLST_S_NEXT(pMixer, ncoNode) )
        {
            BAPE_OutputPort output;
            BAPE_MclkSource mclkSource;

            BDBG_ASSERT( BAPE_MCLKSOURCE_IS_NCO(pMixer->mclkSource));

            if ( BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer) == 0 )
            {
                /* Skip this mixer if it doesn't have a sample rate yet */
                continue;
            }

            switch ( nco )
            {
            default:
            case BAPE_Nco_e0:
                mclkSource = BAPE_MclkSource_eNco0;
                break;
#if BAPE_CHIP_MAX_NCOS > 1
            case BAPE_Nco_e1:
                mclkSource = BAPE_MclkSource_eNco1;
                break;
#endif
#if BAPE_CHIP_MAX_NCOS > 2
            case BAPE_Nco_e2:
                mclkSource = BAPE_MclkSource_eNco2;
                break;
#endif
#if BAPE_CHIP_MAX_NCOS > 3
            case BAPE_Nco_e3:
                mclkSource = BAPE_MclkSource_eNco3;
                break;
#endif
#if BAPE_CHIP_MAX_NCOS > 4
            case BAPE_Nco_e4:
                mclkSource = BAPE_MclkSource_eNco4;
                break;
#endif
#if BAPE_CHIP_MAX_NCOS > 5
            case BAPE_Nco_e5:
                mclkSource = BAPE_MclkSource_eNco5;
                break;
#endif
#if BAPE_CHIP_MAX_NCOS > 6
            case BAPE_Nco_e6:
                mclkSource = BAPE_MclkSource_eNco6;
                break;
#endif
            }
            BDBG_ASSERT( mclkSource == pMixer->mclkSource);

            for ( output = BLST_S_FIRST(&pMixer->outputList);
                  output != NULL;
                  output = BLST_S_NEXT(output, node) )
            {
                if ( output->setMclk_isr )
                {
                    BDBG_MSG(("Setting output mclk for '%s' to source:%s ratio:%u",
                               output->pName, BAPE_Mixer_P_MclkSourceToText_isrsafe(mclkSource), oversample));
                    output->setMclk_isr(output, mclkSource, 0, oversample);
                }
            }
            if ( pMixer->fs != BAPE_FS_INVALID )
            {
                BDBG_MSG(("Setting FS mclk for FS %u to source:%s ratio:%u",
                           pMixer->fs, BAPE_Mixer_P_MclkSourceToText_isrsafe(mclkSource), oversample));
                BAPE_P_SetFsTiming_isr(handle, pMixer->fs, mclkSource, 0, oversample);
            }
        }
    }
    else
    {
        BDBG_MSG(("Not updating NCO %u rate (unknown)", nco));
    }

    return BERR_SUCCESS;
}


BERR_Code BAPE_Nco_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    ncoIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each nco, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( ncoIndex=0 ; ncoIndex<BAPE_CHIP_MAX_NCOS ; ncoIndex++ )
    {
        BAPE_AudioNco *pNco = &bapeHandle->audioNcos[ncoIndex];

        /* Now apply changes for the settings struct. */
            /* Nothing to do here. NCOs don't have a SetSettings function to call. */

        /* Now restore the dynamic stuff from the values saved in the device struct. */
        if (pNco->baseSampleRate != 0)
        {
            BKNI_EnterCriticalSection();
                errCode = BAPE_P_SetNcoFreq_isr( bapeHandle, ncoIndex, pNco->baseSampleRate, NULL, pNco->timebase );
            BKNI_LeaveCriticalSection();
            if ( errCode ) return BERR_TRACE(errCode);
        }
    }
    return errCode;
}


/***************************************************************************
    Define stub functions for when there are no NCOs.
***************************************************************************/
#else /* BAPE_CHIP_MAX_NCOS > 0 */
    /* No NCOs, just use stubbed out functions. */

/**************************************************************************/

BERR_Code BAPE_Nco_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BSTD_UNUSED(bapeHandle);
    return BERR_SUCCESS;
}

#endif /* BAPE_CHIP_MAX_NCOS > 0 */
