/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * Module Description: Debug Status Interface
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
            pStatus->status.outputStatus.type = pStatus->type;
            errCode = BAPE_Debug_GetOutputStatus(handle,pStatus);
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

