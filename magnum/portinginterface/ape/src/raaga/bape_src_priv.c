/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description: Sample Rate Converter (SRC) Interface
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_src_ctrl0.h"

BDBG_MODULE(bape_src_priv);

typedef struct BAPE_SrcGroup
{
    bool allocated;
    bool started;
    uint8_t blockId;
    unsigned numChannelPairs;
    unsigned inputRate, outputRate;
    BAPE_SrcMode mode;
    BAPE_SrcGroupCoefficients *pCoefMemory[2];   /* IIR Coefficient Memory.  Two are required if doubleBuffered is true, otherwise one. */    
    BAPE_Handle deviceHandle;
    uint32_t srcIds[BAPE_ChannelPair_eMax];
    BAPE_SrcGroupSettings settings;
} BAPE_SrcGroup;

static void BAPE_Src_P_SetGroup(BAPE_Handle deviceHandle, uint32_t src, uint32_t group);
static void BAPE_SrcGroup_P_SetEnable_isr(BAPE_SrcGroupHandle handle, uint32_t enable);

BERR_Code BAPE_P_InitSrcSw(
                        BAPE_Handle handle
                        )
{
    unsigned i;
    uint32_t regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    /* TODO: Register for ESR's to catch HW Failures?? */

    /* Setup Group Handles */
    BDBG_MSG(("Allocating %u SRC Groups", BAPE_CHIP_MAX_SRC_GROUPS));
    handle->srcGroups[0] = BKNI_Malloc(BAPE_CHIP_MAX_SRC_GROUPS*sizeof(BAPE_SrcGroup));
    if ( NULL == handle->srcGroups[0] )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle->srcGroups[0], 0, BAPE_CHIP_MAX_SRC_GROUPS*sizeof(BAPE_SrcGroup));
    for ( i = 1; i < BAPE_CHIP_MAX_SRC_GROUPS; i++ )
    {
        handle->srcGroups[i] = handle->srcGroups[0] + i;
    }

    /* Initialize the sampleRateConverterRampStep by reading the default value from the hardware. */
    regVal = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_RAMP_STEP);
    handle->sampleRateConverterRampStep = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_SRC_CTRL0_RAMP_STEP, STEP_SIZE);

    return BERR_SUCCESS;
}

BERR_Code BAPE_P_InitSrcHw(
                        BAPE_Handle handle
                        )
{
    unsigned i;

    #if BAPE_CHIP_SRC_TYPE_IS_LEGACY
        uint32_t regAddr, regVal, stride, mem=0;

        BDBG_OBJECT_ASSERT(handle, BAPE_Device);

        /* We only use LINT mode on STB, so we just need to give each SRC 4 bytes of memory to work with. */
        regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0;
        stride = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG1 - BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0;
        for ( i = 0; i < BAPE_CHIP_MAX_SRCS; i++ )
        {
            regVal = BREG_Read32(handle->regHandle, regAddr);
            regVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFG0, BASE_ADDR)|
                        BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFG0, TYPE));
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFG0, BASE_ADDR, mem);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_SRC_CFG0, TYPE, Bypass);
            BREG_Write32(handle->regHandle, regAddr, regVal);
            mem += 4;
            regAddr += stride;
        }
    /* End of BAPE_CHIP_SRC_TYPE_IS_LEGACY */

    #elif BAPE_CHIP_SRC_TYPE_IS_IIR       
        uint32_t regAddr, regVal=0x0, stride, mem=0x30;
        uint32_t coefSelRegAddr,coefRegAddr;

        BDBG_OBJECT_ASSERT(handle, BAPE_Device);

        /* Total SRAM Memory: 1280-48(HW Internal use) */
        /* Each SRC is allocated 102 bytes of Scratch memory which is required for IIR. */
        /* TODO: Add dynamic allocation support when Upx or Downx SRC types are required */
        regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_BASE;
        coefSelRegAddr = BCHP_AUD_FMM_SRC_CTRL0_CF_SELi_ARRAY_BASE;
        coefRegAddr = BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE;    
        stride = 4;
        for ( i = 0; i < BAPE_CHIP_MAX_SRCS; i++ )
        {
            /* Setup for IIR w/102 bytes scratch ram */
            regVal = 0x0;
            regVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi, TM_BASE_ADDR)|
                        BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi, SRC_TYPE)|
                        BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi, WR_BANK_SEL));
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFGi, TM_BASE_ADDR, mem);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_SRC_CFGi, SRC_TYPE, Bypass);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFGi, WR_BANK_SEL, 0);
            BREG_Write32(handle->regHandle, regAddr, regVal);
            regVal = 0x0;
            BREG_Write32(handle->regHandle, coefSelRegAddr, regVal);        
            regVal = 0x0;
            BREG_Write32(handle->regHandle, coefSelRegAddr, regVal);
            mem += 102;      
            regAddr += stride;
            coefSelRegAddr += stride;
            coefRegAddr += stride;
        }

        /* Zero the coef memory */
        regAddr = BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE;
        for ( i = 0; i <= BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_END; i++ )
        {
            BREG_Write32(handle->regHandle, regAddr + (4*i), 0);
        }

    /* End of BAPE_CHIP_SRC_TYPE_IS_LEGACY */  
    #else
        #error "Unknown BAPE_CHIP_SRC_TYPE"
    #endif /* BAPE_CHIP_SRC_TYPE_... */  

    /* Reset group IDs to default */    
    for ( i = 0; i < BAPE_CHIP_MAX_SRCS; i++ )
    {
        BAPE_Src_P_SetGroup(handle, i, i);
    }

    /* Refresh the HW SRC ramp step from the value saved in the device structure. */
    (void)BAPE_SetSampleRateConverterRampStep(handle, handle->sampleRateConverterRampStep);

    /* TODO: Register for ESR's to catch HW Failures?? */

    return BERR_SUCCESS;
}


void BAPE_P_UninitSrcSw(
                     BAPE_Handle handle
                     )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    if ( handle->srcGroups[0] )
    {
        BKNI_Free(handle->srcGroups[0]);
        BKNI_Memset(handle->srcGroups, 0, sizeof(handle->srcGroups));
    }
}

#include "bape_src_lint_coefs.h"

static void BAPE_Src_P_SetSampleRate_isr(BAPE_Handle handle, uint32_t srcId, unsigned inputRate, unsigned outputRate)
{
    uint32_t regVal, regAddr;
    BAVC_AudioSamplingRate inputRateEnum, outputRateEnum;
    uint32_t type, num, den, denScale;

    /* If the input rate is unknown, use the output rate so we go into bypass mode */
    if ( inputRate == 0 )
    {
        inputRate = outputRate;
    }
    BDBG_ASSERT(outputRate != 0);

    /* Convert from unsigned to array indexes */
    inputRateEnum = BAPE_P_GetSampleRateEnum_isr(inputRate);
    outputRateEnum = BAPE_P_GetSampleRateEnum_isr(outputRate);

    if ( inputRateEnum == BAVC_AudioSamplingRate_eUnknown || outputRateEnum == BAVC_AudioSamplingRate_eUnknown )
    {
        /* Handle non-standard rates that aren't in a pre-computed table */
        type = TYPE_LINT;
        if ( inputRate == outputRate )
        {
            num = den = 1;
        }
        else
        {
            num = inputRate;
            den = outputRate;
            /* Reduce coefficients by removing common prime factors to increase quality of scaling */
            /* Divide by two until we can't anymore */
            while ( num % 2 == 0 && den % 2 == 0 )
            {
                num /= 2;
                den /= 2;
            }
            /* Divide by three until we can't anymore */
            while ( num % 3 == 0 && den % 3 == 0 )
            {
                num /= 3;
                den /= 3;
            }
            /* Divide by five until we can't anymore */
            while ( num % 5 == 0 && den % 5 == 0 )
            {
                num /= 5;
                den /= 5;
            }
        }
        denScale = DEN_SCALE(den);
    }
    else
    {
        /* Use the tables to select the correct mode */
        type = g_pSrcTable[inputRateEnum][outputRateEnum].type;
        num = g_pSrcTable[inputRateEnum][outputRateEnum].num;
        den = g_pSrcTable[inputRateEnum][outputRateEnum].den;
        denScale = g_pSrcTable[inputRateEnum][outputRateEnum].denScale;
    }

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(srcId < BAPE_CHIP_MAX_SRCS);

    BDBG_MSG(("SRC Programming - Input Rate %u Output Rate %u", inputRate, outputRate));
    BDBG_MSG(("Programming SRC %u to mode %u num %u den %u scale %u", srcId, type, num, den, denScale));

    #if BAPE_CHIP_SRC_TYPE_IS_LEGACY   
        BDBG_ASSERT(num > 0);
        BDBG_ASSERT(den > 0);
        BDBG_ASSERT(denScale > 0);
        BDBG_ASSERT(num <= 0x400000);
        BDBG_ASSERT(den <= 0x400000);
        BDBG_ASSERT(denScale <= 0x400000);
        if ( num > den )
        {
            BDBG_ASSERT(num <= (2*den));
        }
        BDBG_ASSERT(denScale == DEN_SCALE(den));
        if (num == 1 && den == 1)
        {
            type = TYPE_NONE;
        }
        else 
        {
            type = TYPE_LINT;
        }

        regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0 + (((uint32_t) srcId)*(BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG1-BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0));

        regAddr += BCHP_AUD_FMM_SRC_CTRL0_LI_DEN0-BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0;
        regVal = BREG_Read32_isr(handle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_LI_DEN0, DENOMINATOR);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_DEN0, DENOMINATOR, den);
        BREG_Write32_isr(handle->regHandle, regAddr, regVal);

        regAddr += BCHP_AUD_FMM_SRC_CTRL0_LI_DEN_SCALE0-BCHP_AUD_FMM_SRC_CTRL0_LI_DEN0;
        regVal = BREG_Read32_isr(handle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_LI_DEN_SCALE0, DEN_SCALE);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_DEN_SCALE0, DEN_SCALE, denScale);
        BREG_Write32_isr(handle->regHandle, regAddr, regVal);

        regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0 + (((uint32_t) srcId)*(BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG1-BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0));
        regAddr += BCHP_AUD_FMM_SRC_CTRL0_LI_NUM0-BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0;

        regVal = BREG_Read32_isr(handle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_LI_NUM0, NUMERATOR);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_NUM0, NUMERATOR, num);
        BREG_Write32_isr(handle->regHandle, regAddr, regVal);

        regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0 + (((uint32_t) srcId)*(BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG1-BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0));
        regVal = BREG_Read32_isr(handle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFG0, TYPE);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFG0, TYPE, type);
        BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    #endif /* BAPE_CHIP_SRC_TYPE_IS_LEGACY */  

    #if BAPE_CHIP_SRC_TYPE_IS_IIR       
        {
            uint32_t coefAddr;

            /* Reverse engineer location of coefs for this SRC */
            regVal = BREG_Read32_isr(handle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_CF_SELi_ARRAY_BASE + (4*srcId));
            coefAddr = (BCHP_GET_FIELD_DATA(regVal, AUD_FMM_SRC_CTRL0_CF_SELi, CF_BASE_ADDR0)-0x18);
            BDBG_ASSERT(coefAddr < (BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_END-2));
            coefAddr *= 4;
            coefAddr += BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE;

            /* Now, write coefficients in order */
            BREG_Write32_isr(handle->regHandle, coefAddr+0, num);
            BREG_Write32_isr(handle->regHandle, coefAddr+4, den);
            BREG_Write32_isr(handle->regHandle, coefAddr+8, denScale);

            /* Set SRC mode */
            regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_BASE + ((BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_ELEMENT_SIZE * srcId) / 8);
            regVal = BREG_Read32_isr(handle->regHandle, regAddr);
            regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi, SRC_TYPE);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFGi, SRC_TYPE, type);
            BREG_Write32_isr(handle->regHandle, regAddr, regVal);
        }
    #endif /* BAPE_CHIP_SRC_TYPE_IS_IIR */  
}

void BAPE_SrcGroup_P_GetDefaultCreateSettings(
                                             BAPE_SrcGroupCreateSettings *pSettings   /* [out] */
                                             )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->blockId = 0;
    pSettings->numChannelPairs = 1;
    pSettings->mode = BAPE_SrcMode_eLinear;

    #if BAPE_CHIP_SRC_TYPE_IS_IIR       
        pSettings->pCoefMemory[0]=NULL;
        pSettings->pCoefMemory[1]=NULL;    
        pSettings->equalizerSettings.numIirBands = 8;
        pSettings->equalizerSettings.rampEnable = false;
    #endif /* BAPE_CHIP_SRC_TYPE_IS_IIR */  
}

BERR_Code BAPE_SrcGroup_P_Create(
                                BAPE_Handle deviceHandle,
                                const BAPE_SrcGroupCreateSettings *pSettings,
                                BAPE_SrcGroupHandle *pHandle        /* [out] */
                                )
{
    unsigned i, src;
    BERR_Code errCode;
    BAPE_SrcGroupHandle handle=NULL;
    BAPE_SrcGroupCoefficients   *pCoefMemory[2]={NULL, NULL};

    #if BAPE_CHIP_SRC_TYPE_IS_IIR       
        uint32_t    regAddr, regVal;
        BAPE_SrcCoefficientsSettings coeffSettings;
    #endif /* BAPE_CHIP_SRC_TYPE_IS_IIR */  

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(pSettings->numChannelPairs <= BAPE_ChannelPair_eMax);
    BDBG_ASSERT(pSettings->blockId == 0);   /* TODO: Handle more than one */

    switch ( pSettings->mode )
    {
    case BAPE_SrcMode_eBypass:
    case BAPE_SrcMode_eLinear:
        break;

    case BAPE_SrcMode_eIir:
        #if BAPE_CHIP_SRC_TYPE_IS_LEGACY   
            BDBG_ERR(("SRC Equalization is not supported on this chipset."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        #endif /* BAPE_CHIP_SRC_TYPE_IS_LEGACY */  
        break;

    default:
        /* TODO: Could add HQ modes if a usage case arises */
        BDBG_ERR(("SRC Mode %u is not supported.", pSettings->mode));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Find an available group handle */
    for ( i = 0; i < BAPE_CHIP_MAX_SRC_GROUPS; i++ )
    {
        BDBG_ASSERT(NULL != deviceHandle->srcGroups[i]);
        if ( !deviceHandle->srcGroups[i]->allocated )
        {
            handle = deviceHandle->srcGroups[i];
            break;
        }
    }

    /* If none found, return error */
    if ( NULL == handle )
    {
        BDBG_ERR(("Not enough Audio SRC hardware resources to support this usage case."));
        return BERR_TRACE(BERR_NOT_AVAILABLE);
    }

    /* Now search for the correct number of resources */
    errCode = BAPE_P_AllocateFmmResource(deviceHandle, BAPE_FmmResourceType_eSrc, pSettings->numChannelPairs, &src);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_alloc_src;
    }

    #if BAPE_CHIP_SRC_TYPE_IS_IIR       
        /* Allocate SRC coefficient memory */
        if(BAPE_SrcMode_eBypass != pSettings->mode)
        {  
            unsigned j;
            
            if(NULL != pSettings->pCoefMemory[0])
            {                
                pCoefMemory[0] = pSettings->pCoefMemory[0];
                if(pSettings->equalizerSettings.rampEnable)
                {
                    if(NULL != pSettings->pCoefMemory[1])
                    {
                        pCoefMemory[1] = pSettings->pCoefMemory[1];
                    }
                    else
                    {
                        BDBG_ERR((" In case of Equalizer ramp enable the sharing of Coefficients can happen either for both banks or not at all"));
                        goto err_alloc_src;
                    }
                }
            }
            else
            {
                pCoefMemory[0] = BKNI_Malloc(sizeof(BAPE_SrcGroupCoefficients));
                if ( NULL == pCoefMemory[0] )
                {
                    errCode = BERR_OUT_OF_SYSTEM_MEMORY;
                    return BERR_TRACE(errCode);
                }
                BKNI_Memset(pCoefMemory[0], 0, sizeof(BAPE_SrcGroupCoefficients));
                if(pSettings->equalizerSettings.rampEnable)
                {
                    pCoefMemory[1] = BKNI_Malloc(sizeof(BAPE_SrcGroupCoefficients));
                    if ( NULL == pCoefMemory[1] )
                    {
                        errCode = BERR_OUT_OF_SYSTEM_MEMORY;
                        return BERR_TRACE(errCode);
                    }
                    BKNI_Memset(pCoefMemory[1], 0, sizeof(BAPE_SrcGroupCoefficients));        
                }
            }    

            for ( i = 0; i < 2; i++ )
            {     
                if(NULL != pCoefMemory[i])
                {        
                    for ( j = 0; j < pSettings->numChannelPairs; j++ )
                    {    
                        coeffSettings.mode = pSettings->mode;
                        errCode = BAPE_SrcCoefficients_P_Allocate(deviceHandle, &coeffSettings, &pCoefMemory[i]->srcCoefficients[j]);            
                        if ( errCode )
                        {
                            errCode = BERR_TRACE(errCode);
                            goto err_alloc_src;
                        }            
                        regAddr = BCHP_AUD_FMM_SRC_CTRL0_CF_SELi_ARRAY_BASE + (4*(src+j));
                        regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
                        if ( i == 0 )
                        {
                            regVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_CF_SELi, CF_BASE_ADDR0));
                            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_CF_SELi, CF_BASE_ADDR0, (pCoefMemory[i]->srcCoefficients[j].baseAddress + 0x18));
                        }
                        else
                        {
                            regVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_CF_SELi, CF_BASE_ADDR1));
                            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_CF_SELi, CF_BASE_ADDR1, (pCoefMemory[i]->srcCoefficients[j].baseAddress + 0x18));
                        }
                        BREG_Write32(deviceHandle->regHandle, regAddr, regVal);  
                    }
                }
            }

            for ( i = 0; i < pSettings->numChannelPairs; i++ )
            {        
                BDBG_MSG(("Programming SRC %u to SRC Type %u", (src+i), pSettings->mode));
                
                regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_BASE + ((BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_ELEMENT_SIZE * (src+i)) / 8);
                regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
                regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi, SRC_TYPE);
                if ( pSettings->mode == BAPE_SrcMode_eIir )
                {
                    regVal |= BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_SRC_CFGi, SRC_TYPE, IIR);
                }
                else 
                {
                    regVal |= BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_SRC_CFGi, SRC_TYPE, L_Int);
                }
                BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
            }
        }

    #else
        for ( i = 0; i < pSettings->numChannelPairs; i++ )
        {        
            uint32_t regVal, regAddr;
            BDBG_MSG(("Programming SRC %u to SRC Type %u", (src+i), pSettings->mode));

            regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0 + (((uint32_t) src+i)*(BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG1-BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0));
            regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
            regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFG0, TYPE);

            if ( pSettings->mode == BAPE_SrcMode_eBypass )
            {
                regVal |= BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_SRC_CFG0, TYPE, Bypass);
            }
            else 
            {
                regVal |= BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_SRC_CFG0, TYPE, L_Int);
            }
            BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
        }
    #endif /* BAPE_CHIP_SRC_TYPE_IS_IIR */  

    /* Successfully allocated resources.  Initialize Group */
    handle->allocated = true;
    handle->started = false;
    handle->blockId = pSettings->blockId;
    handle->numChannelPairs = pSettings->numChannelPairs;
    handle->deviceHandle = deviceHandle;
    handle->pCoefMemory[0] = pCoefMemory[0];
    handle->pCoefMemory[1] = pCoefMemory[1];
    handle->mode = pSettings->mode;
    BKNI_Memset(&handle->settings, 0, sizeof(handle->settings));
    BAPE_FciIdGroup_Init(&handle->settings.input);
    handle->settings.rampEnabled = deviceHandle->settings.rampPcmSamples;
    handle->settings.startupRampEnabled = deviceHandle->settings.rampPcmSamples;
    BKNI_Memset(handle->srcIds, 0xff, sizeof(handle->srcIds));
    for ( i = 0; i < pSettings->numChannelPairs; i++ )
    {
        handle->srcIds[i] = src + i;
        /* Setup Grouping */
        BAPE_Src_P_SetGroup(deviceHandle, src + i, src);
    }
    /* Unmute */
    BKNI_EnterCriticalSection();
    BAPE_SrcGroup_P_SetMute_isr(handle, false);
    BAPE_SrcGroup_P_SetSampleRate_isr(handle, 48000, 48000);
    BKNI_LeaveCriticalSection();
    *pHandle = handle;
    return BERR_SUCCESS;

    err_alloc_src:
    return errCode;
}

void BAPE_SrcGroup_P_Destroy(
                            BAPE_SrcGroupHandle handle
                            )
{
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(!handle->started);
    BDBG_OBJECT_ASSERT(handle->deviceHandle, BAPE_Device);

    /* Clear SRC Grouping */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_Src_P_SetGroup(handle->deviceHandle, handle->srcIds[i], handle->srcIds[i]);
    }
    /* Unmute and revert to bypass */
    BKNI_EnterCriticalSection();
    BAPE_SrcGroup_P_SetMute_isr(handle, false);
    BAPE_SrcGroup_P_SetSampleRate_isr(handle, 48000, 48000);
    BKNI_LeaveCriticalSection();

    /* Free SRC coefficient memory */
    #if BAPE_CHIP_SRC_TYPE_IS_IIR       
        for(i=0; i<2; i++)
        {
            if(NULL != handle->pCoefMemory[i])
            {
                unsigned j;
                for(j=0; j<handle->numChannelPairs; j++)
                {
                    BAPE_SrcCoefficients_P_Free(
                        handle->deviceHandle,
                        &handle->pCoefMemory[i]->srcCoefficients[j]
                        );
                }
                BKNI_Free(handle->pCoefMemory[i]);
            }    
        }
    #endif /* BAPE_CHIP_SRC_TYPE_IS_IIR */  

    /* Release Resources */
    BAPE_P_FreeFmmResource(handle->deviceHandle, BAPE_FmmResourceType_eSrc, handle->numChannelPairs, handle->srcIds[0]);
    BKNI_Memset(handle->srcIds, 0xff, sizeof(handle->srcIds));
    handle->allocated = false;
}

void BAPE_SrcGroup_P_GetSettings(
                                BAPE_SrcGroupHandle handle,
                                BAPE_SrcGroupSettings *pSettings  /* [out] */
                                )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    *pSettings = handle->settings;
}

BERR_Code BAPE_SrcGroup_P_SetSettings(
                                     BAPE_SrcGroupHandle handle,
                                     const BAPE_SrcGroupSettings *pSettings
                                     )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    if ( handle->started )
    {
        /* TODO: Implement bypass on the fly?  Maybe not required since only use L_INT and in==out will do it anyway. */
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    handle->settings = *pSettings;
    if ( !handle->deviceHandle->settings.rampPcmSamples )
    {
        handle->settings.rampEnabled = false;
        handle->settings.startupRampEnabled = false;
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Set SRC Sample Rate (used for Linear mode only)
***************************************************************************/
void BAPE_SrcGroup_P_SetSampleRate_isr(
                                      BAPE_SrcGroupHandle handle,
                                      unsigned inputRate,
                                      unsigned outputRate
                                      )
{
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BKNI_ASSERT_ISR_CONTEXT();

    if ( handle->mode != BAPE_SrcMode_eLinear )
    {
        return;
    }

    handle->inputRate = inputRate;
    handle->outputRate = outputRate;

    if ( handle->mode == BAPE_SrcMode_eBypass || handle->settings.bypass )
    {
        inputRate = outputRate; /* Force bypass mode */
    }

    if ( handle->started )
    {
        BAPE_SrcGroup_P_SetEnable_isr(handle, 0);
    }

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_Src_P_SetSampleRate_isr(handle->deviceHandle, handle->srcIds[i],inputRate, outputRate);
    }
    if ( handle->started )
    {
        BAPE_SrcGroup_P_SetEnable_isr(handle, 1);
    }
}

/***************************************************************************
Summary:
Set SRC Mute
***************************************************************************/
void BAPE_SrcGroup_P_SetMute_isr(
                                BAPE_SrcGroupHandle handle,
                                bool muted
                                )
{
    unsigned i;
    uint32_t regAddr, regVal;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BKNI_ASSERT_ISR_CONTEXT();

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        regAddr = BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE + 
                  (handle->srcIds[i] * (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_ELEMENT_SIZE/8));
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, MUTE_ENA);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, MUTE_ENA, muted?1:0);
        BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);
    }
}

/***************************************************************************
Summary:
Start SRC
***************************************************************************/
BERR_Code BAPE_SrcGroup_P_Start(
                               BAPE_SrcGroupHandle handle
                               )
{
    uint32_t regVal, regAddr;
#if BAPE_CHIP_SRC_TYPE_IS_LEGACY   
    uint32_t mask, value;
#endif /* BAPE_CHIP_SRC_TYPE_IS_LEGACY */  
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    /* Apply Settings */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        #if BAPE_CHIP_SRC_TYPE_IS_IIR       
            /* Ramping Options, Priority, input FCI */
            regAddr = BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE + 
                      (handle->srcIds[i] * (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_ELEMENT_SIZE/8));
            regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
            regVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, RAMP_ENA)|
                        BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, STARTUP_RAMP_ENA)|
                        BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, FCI_ID)|
                        BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, PRIORITY));
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, RAMP_ENA, handle->settings.rampEnabled?1:0);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, STARTUP_RAMP_ENA, handle->settings.startupRampEnabled?1:0);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, FCI_ID, handle->settings.input.ids[i]);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, PRIORITY, handle->settings.highPriority?1:0);
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
        #endif /* BAPE_CHIP_SRC_TYPE_IS_IIR */  


        #if BAPE_CHIP_SRC_TYPE_IS_LEGACY   
            /* Ramping Options, Priority, input FCI */
            regAddr = BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE + 
                      (handle->srcIds[i] * (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_ELEMENT_SIZE/8));
            regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
            regVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, RAMP_ENA)|
            #ifdef BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_RAMP_ZERO_DET_ENA_MASK
                        BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, RAMP_ZERO_DET_ENA)|  /* SW7425-574: This feature does not work unless both L+R have non-zero data.  The field is renamed to ZERO_DETECT when it works correctly. */
            #endif                    
                        BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, STARTUP_RAMP_ENA));
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, RAMP_ENA, handle->settings.rampEnabled?1:0);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, STARTUP_RAMP_ENA, handle->settings.startupRampEnabled?1:0);
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

            /* Input FCI */
            regAddr = BCHP_AUD_FMM_SRC_CTRL0_IDMAP10 + ((handle->srcIds[i]/2)*(BCHP_AUD_FMM_SRC_CTRL0_IDMAP32-BCHP_AUD_FMM_SRC_CTRL0_IDMAP10));
            regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
            if ( handle->srcIds[i] & 1 )
            {
                /* Odd */
                regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP10, STREAM1);
                regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP10, STREAM1, handle->settings.input.ids[i]);
            }
            else
            {
                /* Even */
                regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP10, STREAM0);
                regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP10, STREAM0, handle->settings.input.ids[i]);
            }
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
        #endif /* BAPE_CHIP_SRC_TYPE_IS_LEGACY */  
    }

    /* Re-apply Sample Rate */
    BKNI_EnterCriticalSection();
    BAPE_SrcGroup_P_SetSampleRate_isr(handle, handle->inputRate, handle->outputRate);
    BKNI_LeaveCriticalSection();

    #if BAPE_CHIP_SRC_TYPE_IS_IIR       
        /* Set Enables Last */
        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            regAddr = BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE + 
                      (handle->srcIds[i] * (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_ELEMENT_SIZE/8));
            regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
            regVal |= BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, ENABLE);
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
        }
    #endif /* BAPE_CHIP_SRC_TYPE_IS_IIR */  

    #if BAPE_CHIP_SRC_TYPE_IS_LEGACY   
        /* Set Priority */
        mask=value=0;
        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            mask |= (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA, PRIORITY0) << handle->srcIds[i]);
        }
        /*value = (handle->settings.highPriority) ? mask : 0;*/
        /* always set high priority for 7425/7358/etc */
        value = mask;

        regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA);
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA, (regVal & ~mask) | value);

        /* Set Enable */
        mask=0;
        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            mask |= (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA, ENA0) << handle->srcIds[i]);
        }
        regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA);
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA, regVal | mask);
    #endif /* BAPE_CHIP_SRC_TYPE_IS_LEGACY */  

    handle->started = true;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Stop SRC
***************************************************************************/
void BAPE_SrcGroup_P_Stop(
                         BAPE_SrcGroupHandle handle
                         )
{
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    if ( !handle->started )
    {
        return;
    }

    /* Stop */
    #if BAPE_CHIP_SRC_TYPE_IS_LEGACY   
        {
            uint32_t regVal, mask;

            mask=0;
            for ( i = 0; i < handle->numChannelPairs; i++ )
            {
                mask |= (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA, ENA0) << handle->srcIds[i]);
            }
            regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA);
            BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA, regVal & (~mask));
        }
    #endif /* BAPE_CHIP_SRC_TYPE_IS_LEGACY */  

    #if BAPE_CHIP_SRC_TYPE_IS_IIR       
        {
            uint32_t regVal, regAddr;

            for ( i = 0; i < handle->numChannelPairs; i++ )
            {
                regAddr = BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE + 
                          (handle->srcIds[i] * (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_ELEMENT_SIZE/8));
                regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
                regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, ENABLE);
                BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
            }
        }
    #endif /* BAPE_CHIP_SRC_TYPE_IS_IIR */  

    handle->started = false;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
Summary:
Set Coefficient Index (used for Equalizer mode only)
***************************************************************************/
void BAPE_SrcGroup_P_SetCoefficientIndex_isr(
                                            BAPE_SrcGroupHandle handle,
                                            unsigned index
                                            )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(index);
    /* These chips don't support IIR filtering */
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif

void BAPE_SrcGroup_P_GetOutputFciIds(
                                    BAPE_SrcGroupHandle handle,
                                    BAPE_FciIdGroup *pFciGroup      /* [out] */
                                    )
{
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->blockId == 0);

    BAPE_FciIdGroup_Init(pFciGroup);
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        pFciGroup->ids[i] = BAPE_FCI_BASE_SRC | handle->srcIds[i];
    }
}

static void BAPE_Src_P_SetGroup(BAPE_Handle deviceHandle, uint32_t src, uint32_t group)
{
    BREG_Handle regHandle;
    uint32_t regAddr, regVal;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    regHandle = deviceHandle->regHandle;

    regAddr = BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE + (src * (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_ELEMENT_SIZE/8));
    regVal = BREG_Read32(regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, GROUP);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, GROUP, group);
    BREG_Write32(regHandle, regAddr, regVal);
}

void BAPE_GetSampleRateConverterRampStep(
                                        BAPE_Handle deviceHandle,
                                        uint32_t *pRampStep                 /* All sample rate converters volume is changed by this amount
                                                                               every Fs while ramping.  Specified in 4.23 format. 
                                                                               Ignored for compressed data. */
                                        )
{
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pRampStep);

    *pRampStep = deviceHandle->sampleRateConverterRampStep;
}

BERR_Code BAPE_SetSampleRateConverterRampStep(
                                             BAPE_Handle deviceHandle,
                                             uint32_t rampStep             /* All sample rate converters volume is changed by this amount
                                                                                    every Fs while ramping.  Specified in 4.23 format. 
                                                                                    Ignored for compressed data. */
                                             )
{
    uint32_t regVal;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    if ( rampStep > 0x8000 )
    {
        BDBG_ERR(("Maximum SRC ramp step is 0x8000"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_RAMP_STEP);
    regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_RAMP_STEP, STEP_SIZE);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_RAMP_STEP, STEP_SIZE, rampStep);
    BREG_Write32(deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_RAMP_STEP, regVal);

    /* Save the setting in the BAPE_Device struct.  Use the value from the register field in case the user's value was truncated. */
    deviceHandle->sampleRateConverterRampStep = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_SRC_CTRL0_RAMP_STEP, STEP_SIZE);

    return BERR_SUCCESS;
}

static void BAPE_SrcGroup_P_SetEnable_isr(BAPE_SrcGroupHandle handle, uint32_t enable)
{
#if BAPE_CHIP_SRC_TYPE_IS_IIR       
    uint32_t regVal, regAddr;
    unsigned i;

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        regAddr = BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE + 
                  (handle->srcIds[i] * (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_ELEMENT_SIZE/8));
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
        if ( enable )
        {
            regVal |= BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, ENABLE);
        }
        else
        {
            regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi, ENABLE);
        }
        BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);
    }
#else
    uint32_t regVal, mask;
    unsigned i;

    mask=0;
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        mask |= (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA, ENA0) << handle->srcIds[i]);
    }
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA);
    if ( enable )
    {
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA, regVal | mask);
    }
    else
    {
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA, regVal & (~mask));
    }
#endif
}

#if BAPE_CHIP_SRC_TYPE_IS_IIR
BERR_Code BAPE_SrcCoefficients_P_Allocate(
                                        BAPE_Handle deviceHandle,
                                        const BAPE_SrcCoefficientsSettings *pSettings,
                                        BAPE_SrcCoefficients *pCoefficients           /* [out] Coefficient status */
                                        )
{
    unsigned    chunk;
    BERR_Code errCode=BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* 
        SRC Coefficient Memory Partition:
        0
        . --> IIR
        .
        11

        12
        . --> Linear
        .
        27
    */
    switch(pSettings->mode)
    {
        case BAPE_SrcMode_eIir:
            for(chunk=BAPE_CHIP_P_SRC_IIR_CHUNK_BASE; chunk<=BAPE_CHIP_P_SRC_IIR_CHUNK_END; chunk++)
            {
                if(false == deviceHandle->srcCoeffAllocated[chunk])
                {
                    pCoefficients->baseAddress = chunk*BAPE_CHIP_P_MAX_SRC_IIR_COEFF_PER_CHUNK;
                    pCoefficients->numCoefs = BAPE_CHIP_P_MAX_SRC_IIR_COEFF_PER_CHUNK;
                    break;
                }
            }
            if(chunk > BAPE_CHIP_P_SRC_IIR_CHUNK_END)
            {
                BDBG_ERR(("No free IIR coefficients memory available"));
                errCode = BERR_NOT_SUPPORTED;
            }
            else
            {
                deviceHandle->srcCoeffAllocated[chunk] = true;
            }
            break;
            
        case BAPE_SrcMode_eLinear:
            for(chunk=BAPE_CHIP_P_SRC_LIN_CHUNK_BASE; chunk<=BAPE_CHIP_P_SRC_LIN_CHUNK_END; chunk++)
            {
                if(false == deviceHandle->srcCoeffAllocated[chunk])
                {
                    pCoefficients->baseAddress = BAPE_CHIP_P_TOTAL_SRC_IIR_COEFF + 
                        (chunk - BAPE_CHIP_P_SRC_LIN_CHUNK_BASE)*BAPE_CHIP_P_NUM_LIN_COEFF_PER_CHUNK;
                    pCoefficients->numCoefs = BAPE_CHIP_P_NUM_LIN_COEFF_PER_CHUNK;
                    break;
                }
            }
            if(chunk > BAPE_CHIP_P_SRC_LIN_CHUNK_END)
            {
                BDBG_ERR(("No free Linear coefficients memory available"));
                errCode = BERR_NOT_SUPPORTED;
            }
            else
            {
                deviceHandle->srcCoeffAllocated[chunk] = true;
            }
            break;
            
        default:
            BDBG_ERR(("Unsupported SRC Type %d", pSettings->mode));
            return BERR_NOT_SUPPORTED;
    }

    BDBG_MSG(("SRC Coeff Memory Allocated: Mode=%d, Coeff Offset=%u", pSettings->mode, pCoefficients->baseAddress));
    return errCode;
}

void BAPE_SrcCoefficients_P_Free(
    BAPE_Handle deviceHandle,
    BAPE_SrcCoefficients *pCoefficients             /* [modified] Coefficients will be released */
    )
{
    unsigned chunk;
    BDBG_ASSERT(NULL != pCoefficients);

    if(pCoefficients->baseAddress <= (BAPE_CHIP_P_MAX_SRC_IIR_CHUNKS*BAPE_CHIP_P_MAX_SRC_IIR_COEFF_PER_CHUNK))
    {
        chunk = pCoefficients->baseAddress/BAPE_CHIP_P_MAX_SRC_IIR_COEFF_PER_CHUNK;
    }
    else
    {
        chunk = (pCoefficients->baseAddress - BAPE_CHIP_P_TOTAL_SRC_IIR_COEFF)/BAPE_CHIP_P_NUM_LIN_COEFF_PER_CHUNK + \
                    BAPE_CHIP_P_SRC_LIN_CHUNK_BASE;
    }        
        
    deviceHandle->srcCoeffAllocated[chunk] = false;    
}

void BAPE_SrcGroup_P_UpdateCoefficients_isr(
        BAPE_SrcGroupHandle src, 
        BAPE_SRC_P_IIRCoeff *pCoeff,
        unsigned *pStepSize
        )
{
    unsigned i, j, regAddr, coefRegAddr, regVal, newBank=0;
    BDBG_ASSERT(NULL!=src);

    for(i=0; i<src->numChannelPairs; i++)
    {
        if(pStepSize)
        {
            regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_BASE + ((BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_ELEMENT_SIZE * (src->srcIds[i])) / 8);
            regVal = BREG_Read32_isr(src->deviceHandle->regHandle, regAddr);
            newBank = (0 == (BCHP_GET_FIELD_DATA(regVal, AUD_FMM_SRC_CTRL0_SRC_CFGi, WR_BANK_SEL)))?1:0;
        }

        coefRegAddr = BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE +
        (src->pCoefMemory[newBank]->srcCoefficients[i].baseAddress*(BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_ELEMENT_SIZE/8));
        
        for(j=0; j<BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC; j++)
        {
            BREG_Write32_isr(src->deviceHandle->regHandle, 
                coefRegAddr+(4*5*j), 
                pCoeff->b0[j]);
            BREG_Write32_isr(src->deviceHandle->regHandle, 
                coefRegAddr+(4*5*j)+4, 
                pCoeff->b1[j]);
            BREG_Write32_isr(src->deviceHandle->regHandle, 
                coefRegAddr+(4*5*j)+8,
                pCoeff->b2[j]);
            BREG_Write32_isr(src->deviceHandle->regHandle, 
                coefRegAddr+(4*5*j)+12,
                pCoeff->a1[j]);    
            BREG_Write32_isr(src->deviceHandle->regHandle, 
                coefRegAddr+(4*5*j)+16,
                pCoeff->a2[j]);   
        }

        if(pStepSize)
        {
            regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_BASE + ((BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_ELEMENT_SIZE * (src->srcIds[i])) / 8);
            regVal = BREG_Read32_isr(src->deviceHandle->regHandle, regAddr);
            regVal = ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi, IIR_STEP_SIZE));
            regVal = ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi, WR_BANK_SEL));
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFGi, IIR_STEP_SIZE, *pStepSize);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFGi, WR_BANK_SEL, (newBank));
            BREG_Write32_isr(src->deviceHandle->regHandle, regAddr, regVal);
        }        
    }
}
#endif /* BAPE_CHIP_SRC_TYPE_IS_IIR */  
