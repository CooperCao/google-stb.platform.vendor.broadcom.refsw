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
 * Module Description: Debug Status Interface
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"

#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
#include "bchp_aud_fmm_iop_out_mai_0.h"

#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_1_REG_START
#include "bchp_aud_fmm_iop_out_mai_1.h"
#endif
#endif

#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
#include "bchp_aud_fmm_iop_out_spdif_0.h"
#endif

#include "bape_debug.h"

BDBG_MODULE(bape_debug);

BDBG_OBJECT_ID(BAPE_Debug);


typedef struct BAPE_Debug
{
    BDBG_OBJECT(BAPE_Debug)       
    BAPE_Handle deviceHandle;
#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0 
    BAPE_OutputPort spdif[BAPE_CHIP_MAX_SPDIF_OUTPUTS];
#endif
#if BAPE_CHIP_MAX_MAI_OUTPUTS > 0 
    BAPE_OutputPort hdmi[BAPE_CHIP_MAX_MAI_OUTPUTS];
#endif
#if BAPE_CHIP_MAX_DACS > 0
    BAPE_OutputPort dac[BAPE_CHIP_MAX_DACS];
#endif
#if BAPE_CHIP_MAX_I2S_OUTPUTS > 0
    BAPE_OutputPort i2s[BAPE_CHIP_MAX_I2S_OUTPUTS];
#endif
} BAPE_Debug;


/**************************************************************************/

BERR_Code BAPE_Debug_Open(
    BAPE_Handle deviceHandle,
    const BAPE_DebugOpenSettings * pSettings, 
    BAPE_DebugHandle * pHandle /* [out] */
    )
{

    BERR_Code errCode = BERR_SUCCESS;
    BAPE_DebugHandle handle;   
    BSTD_UNUSED(pSettings);
    
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);
    
    BDBG_MSG(("%s opening Debug",__FUNCTION__));

    *pHandle = NULL;

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_Debug));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_Debug));
    BDBG_OBJECT_SET(handle, BAPE_Debug);
    handle->deviceHandle = deviceHandle;    
    *pHandle = handle;
    return errCode;

}

void BAPE_Debug_Close(BAPE_DebugHandle handle)
{
    
    /* destroy the handle*/
    BDBG_OBJECT_DESTROY(handle, BAPE_Debug);
    BKNI_Free(handle); 

}


BERR_Code BAPE_Debug_GetStatus(
    BAPE_DebugHandle handle,
    BAPE_DebugSourceType type,
    BAPE_DebugStatus * pStatus /* out */
    )

{
    BERR_Code errCode = BERR_SUCCESS;   
    
    BDBG_OBJECT_ASSERT(handle, BAPE_Debug);
    BKNI_Memset(pStatus, 0, sizeof(BAPE_DebugStatus));

    pStatus->type = type;

    switch (pStatus->type)
    {
        case BAPE_DebugSourceType_eOutput:
            errCode = BAPE_Debug_GetOutputStatus(handle, pStatus);
            break;
        case BAPE_DebugSourceType_eVolume:
            errCode = BAPE_Debug_GetOutputVolume(handle, pStatus);
            break;
        default:
            BDBG_ERR(("%s pStatus->type %u not supported",__FUNCTION__,pStatus->type));
            errCode = BERR_INVALID_PARAMETER;
            break;
    }

    return errCode;
}

BERR_Code BAPE_Debug_GetOutputStatus(
    BAPE_DebugHandle handle,    
    BAPE_DebugStatus * pStatus /* out */
    )

{
    BERR_Code errCode = BERR_SUCCESS;  
    int i;
    
#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0 
    for (i = 0; i < BAPE_CHIP_MAX_SPDIF_OUTPUTS; i++)
    {
        if (handle->spdif[i])
        {
           errCode |= BAPE_Debug_GetChannelStatus(handle,handle->spdif[i],&pStatus->status.outputStatus.spdif[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_MAI_OUTPUTS > 0 
    for (i = 0; i < BAPE_CHIP_MAX_MAI_OUTPUTS; i++)    {
        if (handle->hdmi[i])
        {
           errCode |= BAPE_Debug_GetChannelStatus(handle,handle->hdmi[i],&pStatus->status.outputStatus.hdmi[i]);
        }
    }    
#endif
    return errCode;
}


BERR_Code BAPE_Debug_GetChannelStatus(
    BAPE_DebugHandle handle,
    BAPE_OutputPort output,
    BAPE_DebugDigitalOutputStatus *status)
{

    BERR_Code errCode = BERR_SUCCESS;   
    const BAPE_FMT_Descriptor     *pFormat;
    uint32_t regAddr;


    BDBG_OBJECT_ASSERT(handle, BAPE_Debug);
    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);
    BDBG_MSG(("%s called for output->type %s[%d]",__FUNCTION__,output->pName,output->index));   

    status->enabled = true;
    status->pName = (char *)output->pName;
    status->index = output->index;
    if (output->mixer)
    {
        pFormat = BAPE_Mixer_P_GetOutputFormat(output->mixer);
        
        status->type = pFormat->type;        
        status->compressedAsPcm = BAPE_FMT_P_IsDtsCdCompressed_isrsafe(pFormat);
        status->sampleRate = pFormat->sampleRate;

    }

#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START     
    switch (output->type)
    {
        case BAPE_OutputPortType_eMaiOutput:
            if (status->index == 0)
            {
                regAddr = BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CHANSTAT_0;
            }
            else
            {
#if BCHP_AUD_FMM_IOP_OUT_MAI_1_REG_START
                regAddr = BCHP_AUD_FMM_IOP_OUT_MAI_1_SPDIF_CHANSTAT_0;
#else
                return BERR_INVALID_PARAMETER;
#endif
            }
            status->cbits[0] = BAPE_Reg_P_Read(handle->deviceHandle, regAddr);
            status->formatId = BAPE_Debug_BitRangeValue(status->cbits[0],0,0);
            status->audio = BAPE_Debug_BitRangeValue(status->cbits[0],1,1);
            status->copyright = BAPE_Debug_BitRangeValue(status->cbits[0],2,2);
            status->emphasis = BAPE_Debug_BitRangeValue(status->cbits[0],3,5);
            status->mode = BAPE_Debug_BitRangeValue(status->cbits[0],6,7);
            status->categoryCode = BAPE_Debug_BitRangeValue(status->cbits[0],8,15);
            status->samplingFrequency = BAPE_Debug_BitRangeValue(status->cbits[0],24,27);
        
            if (status->index == 0)
            {
                regAddr = BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CHANSTAT_1;
            }
            else
            {
#if BCHP_AUD_FMM_IOP_OUT_MAI_1_REG_START
                regAddr = BCHP_AUD_FMM_IOP_OUT_MAI_1_SPDIF_CHANSTAT_1;
#else
                return BERR_INVALID_PARAMETER;
#endif
            }                
            status->cbits[1] = BAPE_Reg_P_Read(handle->deviceHandle, regAddr);
            status->pcmWordLength = BAPE_Debug_BitRangeValue(status->cbits[1],0,0);
            status->pcmSampleWordLength = BAPE_Debug_BitRangeValue(status->cbits[1],1,3);
            status->pcmOrigSamplingFrequency = BAPE_Debug_BitRangeValue(status->cbits[1],4,7);
            status->pcmCgmsA = BAPE_Debug_BitRangeValue(status->cbits[1],8,9);                      
            break;
        case BAPE_OutputPortType_eSpdifOutput:
            if (status->index == 0)
            {
                regAddr = BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CHANSTAT_0;
            }
            else
            {
                return BERR_INVALID_PARAMETER;
            }
            status->cbits[0] = BAPE_Reg_P_Read(handle->deviceHandle, regAddr);
            status->formatId = BAPE_Debug_BitRangeValue(status->cbits[0],0,0);
            status->audio = BAPE_Debug_BitRangeValue(status->cbits[0],1,1);
            status->copyright = BAPE_Debug_BitRangeValue(status->cbits[0],2,2);
            status->emphasis = BAPE_Debug_BitRangeValue(status->cbits[0],3,5);
            status->mode = BAPE_Debug_BitRangeValue(status->cbits[0],6,7);
            status->categoryCode = BAPE_Debug_BitRangeValue(status->cbits[0],8,15);
            status->samplingFrequency = BAPE_Debug_BitRangeValue(status->cbits[0],24,27);
                
            if (status->index == 0)
            {
                regAddr = BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CHANSTAT_1;
            }
            else
            {
                return BERR_INVALID_PARAMETER;
            }
            status->cbits[1] = BAPE_Reg_P_Read(handle->deviceHandle, regAddr);                
            status->pcmWordLength = BAPE_Debug_BitRangeValue(status->cbits[1],0,0);
            status->pcmSampleWordLength = BAPE_Debug_BitRangeValue(status->cbits[1],1,3);
            status->pcmOrigSamplingFrequency = BAPE_Debug_BitRangeValue(status->cbits[1],4,7);
            status->pcmCgmsA = BAPE_Debug_BitRangeValue(status->cbits[1],8,9);           
            break;
        default:
            BDBG_ERR(("%s INVALID TYPE %d",__FUNCTION__,status->index));
            errCode = BERR_INVALID_PARAMETER;
            break;
        
    }
    

#else /* Legacy */
    switch (output->type)
    {
        case BAPE_OutputPortType_eMaiOutput:
            regAddr = BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1;
            status->cbits[0] = BAPE_Reg_P_Read(handle->deviceHandle, regAddr);            
            status->formatId = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_PRO_CONS);
            status->audio = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_COMP_LIN);
            status->copyright = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_CP);
            status->emphasis = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_EMPH);
            status->mode = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_CMODE);
            status->categoryCode = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_CATEGORY);       
            status->samplingFrequency = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_FREQ);
                   
            regAddr = BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1;
            status->cbits[1] = BAPE_Reg_P_Read(handle->deviceHandle, regAddr);
            status->pcmWordLength = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, PCM_MAX_LEN);
            status->pcmSampleWordLength = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, PCM_LENGTH);
            status->pcmOrigSamplingFrequency = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, PCM_ORIG_FREQ);
            status->pcmCgmsA = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, PCM_CGMS_A);        
            break;
        case BAPE_OutputPortType_eSpdifOutput:
            regAddr = BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0;
            status->cbits[0] = BAPE_Reg_P_Read(handle->deviceHandle, regAddr);
            status->formatId = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_PRO_CONS);
            status->audio = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_COMP_LIN);
            status->copyright = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_CP);
            status->emphasis = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_EMPH);
            status->mode = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_CMODE);
            status->categoryCode = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_CATEGORY);        
            status->samplingFrequency = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_FREQ);
            
            regAddr = BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0;
            status->cbits[1] = BAPE_Reg_P_Read(handle->deviceHandle, regAddr);
            status->pcmWordLength = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, PCM_MAX_LEN);
            status->pcmSampleWordLength = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, PCM_LENGTH);
            status->pcmOrigSamplingFrequency = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, PCM_ORIG_FREQ);
            status->pcmCgmsA = BAPE_Reg_P_ReadField(handle->deviceHandle, regAddr, AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, PCM_CGMS_A);        
            break;
        default:
            BDBG_ERR(("%s INVALID TYPE %d",__FUNCTION__,output->type));
            errCode = BERR_INVALID_PARAMETER;
            break;
    }
#endif

    return errCode;
  
}

static BERR_Code BAPE_Debug_P_GetVolume(
    BAPE_DebugHandle handle,
    BAPE_OutputPort output,
    BAPE_DebugOutputVolume *volume)
{

    BERR_Code errCode = BERR_SUCCESS;
    const BAPE_FMT_Descriptor     *pFormat;

    BDBG_OBJECT_ASSERT(handle, BAPE_Debug);
    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);
    BDBG_MSG(("%s called for output->type %s[%d]",__FUNCTION__, output->pName, output->index));

    volume->enabled = true;
    volume->pName = (char *)output->pName;
    volume->index = output->index;
    if (output->mixer)
    {
        pFormat = BAPE_Mixer_P_GetOutputFormat(output->mixer);
        volume->type = pFormat->type;
        BAPE_GetOutputVolume(output, &volume->outputVolume);
    }
    else
    {
        /* If for whatever reason it can't get the mixer then consider that output as not enabled. */
        volume->enabled = false;
    }

    return errCode;

}

BERR_Code BAPE_Debug_GetOutputVolume(
    BAPE_DebugHandle handle,
    BAPE_DebugStatus * pStatus /* out */
    )

{
    BERR_Code errCode = BERR_SUCCESS;
    int i;

#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0
    for (i = 0; i < BAPE_CHIP_MAX_SPDIF_OUTPUTS; i++) {
        if (handle->spdif[i]) {
           errCode |= BAPE_Debug_P_GetVolume(handle, handle->spdif[i], &pStatus->status.volumeStatus.spdif[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_MAI_OUTPUTS > 0
    for (i = 0; i < BAPE_CHIP_MAX_MAI_OUTPUTS; i++) {
        if (handle->hdmi[i]) {
           errCode |= BAPE_Debug_P_GetVolume(handle, handle->hdmi[i], &pStatus->status.volumeStatus.hdmi[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_DACS > 0
    for (i = 0; i < BAPE_CHIP_MAX_DACS; i++) {
        if (handle->dac[i]) {
           errCode |= BAPE_Debug_P_GetVolume(handle, handle->dac[i], &pStatus->status.volumeStatus.dac[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_I2S_OUTPUTS > 0
    for (i = 0; i < BAPE_CHIP_MAX_I2S_OUTPUTS; i++) {
        if (handle->i2s[i]) {
           errCode |= BAPE_Debug_P_GetVolume(handle, handle->i2s[i], &pStatus->status.volumeStatus.i2s[i]);
        }
    }
#endif

    return errCode;
}

#if !B_REFSW_MINIMAL
void BAPE_Debug_GetInterruptHandlers(
    BAPE_DebugHandle handle,
    BAPE_DebugInterruptHandlers *pInterrupts /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pInterrupts);
    BDBG_ERR(("%s is unimplemented and should not be used",__FUNCTION__));
}


BERR_Code BAPE_Debug_SetInterruptHandlers(
    BAPE_DebugHandle handle,
    const BAPE_DebugInterruptHandlers *pInterrupts
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pInterrupts);
    BDBG_ERR(("%s is unimplemented and should not be used",__FUNCTION__));
    return BERR_INVALID_PARAMETER;
}
#endif

BERR_Code BAPE_Debug_AddOutput(
    BAPE_DebugHandle handle,
    BAPE_OutputPort output)
{
    BERR_Code errCode = BERR_SUCCESS;   

    BDBG_OBJECT_ASSERT(handle, BAPE_Debug);
    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    switch ( output->type )
    {
        case BAPE_OutputPortType_eMaiOutput:
#if BAPE_CHIP_MAX_MAI_OUTPUTS
            handle->hdmi[output->index] = output;
#endif
        break;
        case BAPE_OutputPortType_eSpdifOutput:
#if BAPE_CHIP_MAX_SPDIF_OUTPUTS
            handle->spdif[output->index] = output;
#endif
        break;
        case BAPE_OutputPortType_eDac:
#if BAPE_CHIP_MAX_DACS
            handle->dac[output->index] = output;
#endif
            break;
        case BAPE_OutputPortType_eI2sOutput:
#if BAPE_CHIP_MAX_I2S_OUTPUTS
            handle->i2s[output->index] = output;
#endif
            break;
        default:
            break;
    }

    return errCode;
}

BERR_Code BAPE_Debug_RemoveOutput(
    BAPE_DebugHandle handle,
    BAPE_OutputPort output)
{
    BERR_Code errCode = BERR_SUCCESS;   

    BDBG_OBJECT_ASSERT(handle, BAPE_Debug);
    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    switch ( output->type )
    {
        case BAPE_OutputPortType_eMaiOutput:
#if BAPE_CHIP_MAX_MAI_OUTPUTS
            handle->hdmi[output->index] = NULL;
#endif
            break;
        case BAPE_OutputPortType_eSpdifOutput:
#if BAPE_CHIP_MAX_SPDIF_OUTPUTS
            handle->spdif[output->index] = NULL;
#endif
            break;
        case BAPE_OutputPortType_eDac:
#if BAPE_CHIP_MAX_DACS
            handle->dac[output->index] = NULL;
#endif
            break;
        case BAPE_OutputPortType_eI2sOutput:
#if BAPE_CHIP_MAX_I2S_OUTPUTS
            handle->i2s[output->index] = NULL;
#endif
            break;
        default:
            break;
    }
    
    return errCode;
}


uint32_t BAPE_Debug_BitRangeValue(
    uint32_t value,
    unsigned start,
    unsigned stop)
{
    int temp;    
    if (start > 31 || stop > 31)
    {
        return 0;
    }
    if (start > stop)
    {
        temp = start;
        start = stop;
        stop = temp;
    }

    
    value = value >> start;
    temp = 0;
    while (start <= stop)
    {
        temp = temp <<1;
        temp |= 0x1;        
        start++;
    }    
    return (temp & value);
}

char* BAPE_Debug_IntToBinaryString(
    unsigned value,
    unsigned signifDigits,
    char* ptr)
{  
    unsigned i;

    for (i=0;i<signifDigits;i++)
    {
        ptr[i]= (((value>>i)&1)?'1':'0');
    }         
    ptr[signifDigits]=0;    
    return ptr;
}
