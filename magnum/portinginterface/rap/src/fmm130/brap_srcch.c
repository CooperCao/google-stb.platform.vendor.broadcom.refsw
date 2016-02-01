/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*   Module name: SRCCH
*   This file contains the implementation of all PIs for the Source
*   Channel (Fifo) abstraction. 
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE (rap_srcch);


#define BRAP_SRCCH_P_RAMP_DOWN_TIMEOUT  20
#define BRAP_SRCCH_P_INVALID_SAMPLING_RATE				(-1)

/* Default Settings and Parameters */
static const BRAP_SRCCH_P_Params defSrcChParams =
{
    true,       /* bInputSrValid */
    BRAP_BufDataMode_eStereoNoninterleaved,/* eBufDataMode */
    true,       /* bCompress */
    false,       /* bSharedRbuf */ 
    BRAP_INVALID_VALUE, /* uiMasterSrcCh */
    BRAP_SRCCH_P_INVALID_SAMPLING_RATE,  /* eInputSR */
    BRAP_SRCCH_P_INVALID_SAMPLING_RATE,  /* eOutputSR */
    BRAP_InputBitsPerSample_e8, /* eInputBitsPerSample */ 
    BRAP_SRCCH_P_LRDataControl_LR_2_LR, /* L to L and R to R data control */
    false, 		/* bNotPauseWhenEmpty: false = PAUSE when empty */
    false, 		/* bStartSelection: false = start from the read point */
    false,      /* bCapDirect2SrcFIFO */
#if (BRAP_7440_ARCH == 0)
    BRAP_RM_P_INVALID_INDEX,/* uiLtOrSnglRBufId */
    BRAP_RM_P_INVALID_INDEX/* uiRtRBufId */
#else
    false,                  /* bProcessIdHigh */
    BRAP_RM_P_INVALID_INDEX /* uiGroupId */
#endif    		
#if BRAP_UNSIGNED_PCM_SUPPORTED
        ,true        /* bIsSigned */
#endif
};


BERR_Code 
BRAP_SRCCH_P_GetDefaultParams ( 
    BRAP_SRCCH_P_Params    *pDefParams   /* Pointer to memory where default
                                           settings should be written */    
    )
{
    BERR_Code  ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_SRCCH_P_GetDefaultParams);
    BDBG_ASSERT (pDefParams);

    *pDefParams = defSrcChParams;

    BDBG_LEAVE(BRAP_SRCCH_P_GetDefaultParams);
    return ret;    
}

BERR_Code 
BRAP_SRCCH_P_GetCurrentParams (
    BRAP_SRCCH_P_Handle     hSrcCh,         /* [in] SrcCh handle */
    BRAP_SRCCH_P_Params     *pCurParams     /* [out] Pointer to memory where 
                                             current params should be written */    
    )
{
    BERR_Code  ret = BERR_SUCCESS;

    BDBG_ENTER(BRAP_SRCCH_P_GetCurrentParams);
    BDBG_ASSERT(hSrcCh);
    BDBG_ASSERT(pCurParams);
    
    *pCurParams = hSrcCh->sParams;

    BDBG_LEAVE(BRAP_SRCCH_P_GetCurrentParams);
    return ret;    
}

/***************************************************************************
Summary:
    Configures the HW registers for the Source Channel

Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    None.
**************************************************************************/
#if (BRAP_7440_ARCH==0)
static BERR_Code BRAP_SRCCH_P_HWConfig ( 
    BRAP_SRCCH_P_Handle hSrcCh 		/* [in] Source Channel handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal = 0;
    unsigned int ui32Offset = 0;
    BREG_Handle hRegister = NULL;
    BRAP_SRCCH_P_Params sParams;

    BDBG_ENTER (BRAP_SRCCH_P_HWConfig);
    BDBG_ASSERT (hSrcCh);

    BDBG_MSG (("BRAP_SRCCH_P_HWConfig(): hSrcCh=0x%x" 
			   "Index=%d", hSrcCh, hSrcCh->uiIndex));
  
    hRegister = hSrcCh->hRegister;
    ui32Offset = hSrcCh->ui32Offset;
    sParams = hSrcCh->sParams;    
    
    ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG + ui32Offset);
    
    ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, REVERSE_ENDIAN));
    ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, BUFFER_PAIR_ENABLE));
    ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SAMPLE_CH_MODE));    
    
  	if (sParams.bCompress)
    {
        /* for compressed stereo data */
        /* do nothing */
       
    }
    else if (sParams.eBufDataMode == BRAP_BufDataMode_eMono)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SAMPLE_CH_MODE, 1));
    }
	else if (sParams.eBufDataMode == BRAP_BufDataMode_eStereoInterleaved)
    {
        /* do nothing */
    }
	else if (sParams.eBufDataMode == BRAP_BufDataMode_eStereoNoninterleaved)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, BUFFER_PAIR_ENABLE, 1));
    }
	else
	{
		BDBG_ERR(("BRAP_SRCCH_P_HWConfig(): Unhandled case bCompress(%d) eBufDataMode(%d)",
			sParams.bCompress, sParams.eBufDataMode));
		BDBG_ASSERT(0);
	}

	/* Set the LR Data control */
    ui32RegVal &= ~ (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, LR_DATA_CTRL));   
    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, LR_DATA_CTRL, sParams.eLRDataCntl));
	BDBG_MSG(("BRAP_SRCCH_P_HWConfig(): LR_DATA_CTRL = %d",sParams.eLRDataCntl));

    ui32RegVal &= ~ (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, NOT_PAUSE_WHEN_EMPTY)); 
	if(sParams.bNotPauseWhenEmpty == true)
	{
		/* Set 'Not Pause On Empty' = 1 */
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, NOT_PAUSE_WHEN_EMPTY, 1));
	}
	BDBG_MSG(("BRAP_SRCCH_P_HWConfig(): NOT_PAUSE_WHEN_EMPTY = %d",sParams.bNotPauseWhenEmpty));
				
    /* As per RDB recommendation,
      'Sample Repeat Enable'    = 0, for compressed stream
                                = 1, for uncompressed stream */
    ui32RegVal &= ~ (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SAMPLE_REPEAT_ENABLE));                                 
    if (sParams.bCompress == false)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SAMPLE_REPEAT_ENABLE, 1));
    }

    ui32RegVal &= ~ (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SHARED_SBUF_ID));  
    ui32RegVal &= ~ (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SHARE_SBUF));      
    if (sParams.bSharedRbuf == true)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, 
                                                              SHARED_SBUF_ID, 
                                                              sParams.uiMasterSrcCh));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SHARE_SBUF, 1));        
    }


    ui32RegVal &= ~ (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, BIT_RESOLUTION));  
	switch(sParams.eInputBitsPerSample)
	{
		case BRAP_InputBitsPerSample_e8:
	        ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, BIT_RESOLUTION, Bitwidth8));
			break;
			
		case BRAP_InputBitsPerSample_e16:
	        ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, BIT_RESOLUTION, Bitwidth16));			
			break;
			
		case BRAP_InputBitsPerSample_e32:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, BIT_RESOLUTION, Bitwidth32));			
			break;
			
		default: 	
			BDBG_ASSERT(0);
	}

    /* DMA_READ_DISABLE and FLOWON_SFIFO_HALFFULL should be set to 1 if data is written directly to the
       source FIFO. Ex: Bypass mode of the capture channel. For all other channels types and modes, they
       should retain the reset value 0 */
    ui32RegVal &= ~ (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, FLOWON_SFIFO_HALFFULL));  
    ui32RegVal &= ~ (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, DMA_READ_DISABLE));      
    if(true == sParams.bCapDirect2SrcFIFO)
    {
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, FLOWON_SFIFO_HALFFULL, 1));			
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, DMA_READ_DISABLE, 1));
    }
    
#if BRAP_UNSIGNED_PCM_SUPPORTED
    /* For Signed and unsigned data */
    ui32RegVal &= ~ (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, INVERT_MSB));

    if((sParams.bCompress == true) && (sParams.bIsSigned == false))
    {
        BDBG_ERR(("BRAP_SRCCH_P_HWConfig : Compressed data can not be unsigned "));
        BDBG_ASSERT(0);
    }

    if(sParams.bIsSigned == true)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, INVERT_MSB,0));
    }
    else
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, INVERT_MSB,1));
    }
#endif

    /* Leave all other field settings as Reset values */
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG + ui32Offset, ui32RegVal);

    BDBG_LEAVE (BRAP_SOURCECH_P_HWConfig);
    return ret;
}

BERR_Code BRAP_SRCCH_P_ProgramLRControl ( 
    BRAP_SRCCH_P_Handle     hSrcCh        /* [in] Source Channel Handle */
    )
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t  ui32RegVal = 0;
	unsigned int ui32Offset = 0;
	BREG_Handle hRegister = NULL;
	BRAP_SRCCH_P_Params sParams;

	BDBG_ENTER (BRAP_SRCCH_P_ProgramLRControl);
	BDBG_ASSERT (hSrcCh);

	BDBG_MSG(("BRAP_SRCCH_P_ProgramLRControl(): hSrcCh=0x%x" 
	   "Index=%d", hSrcCh, hSrcCh->uiIndex));

	hRegister = hSrcCh->hRegister;
	ui32Offset = hSrcCh->ui32Offset;
	sParams = hSrcCh->sParams;    

	ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG + ui32Offset);

	ui32RegVal &= ~ (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, LR_DATA_CTRL));   
	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, LR_DATA_CTRL, sParams.eLRDataCntl));
	BDBG_MSG(("BRAP_SRCCH_P_ProgramLRControl(): LR_DATA_CTRL = %d",sParams.eLRDataCntl));

	BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG + ui32Offset, ui32RegVal);

	return ret;

}

#else
static BERR_Code BRAP_SRCCH_P_HWConfig ( 
    BRAP_SRCCH_P_Handle hSrcCh 		/* [in] Source Channel handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal;
    unsigned int ui32Offset;
    BREG_Handle hRegister;
    BRAP_SRCCH_P_Params sParams;

    BDBG_ENTER (BRAP_SRCCH_P_HWConfig);
    BDBG_ASSERT (hSrcCh);

    BDBG_MSG (("BRAP_SRCCH_P_HWConfig(): hSrcCh=0x%x" 
			   "Index=%d", hSrcCh, hSrcCh->uiIndex));
  
    hRegister = hSrcCh->hRegister;
    ui32Offset = hSrcCh->ui32Offset;
    sParams = hSrcCh->sParams; 

    ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset);

    ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, BUFFER_PAIR_ENABLE)) 
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_CH_MODE)) 
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, NOT_PAUSE_WHEN_EMPTY)) 
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_REPEAT_ENABLE))
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, REVERSE_ENDIAN)) 
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION))
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, LR_DATA_CTRL)) 
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SFIFO_START_HALFFULL)) 
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, DMA_READ_DISABLE))
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_ID_HIGH))
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_SEQ_ID_VALID))
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARED_SBUF_ID))
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARE_SBUF))
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_SIZE_DOUBLE)));

    if (sParams.bCompress)
    {
        /* for compressed stereo data */
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, BUFFER_PAIR_ENABLE, 0));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_CH_MODE, 0)); /* don't care */
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_SIZE_DOUBLE, 1));
    }
    else if (sParams.eBufDataMode == BRAP_BufDataMode_eMono)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, BUFFER_PAIR_ENABLE, 0));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_CH_MODE, 1));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_SIZE_DOUBLE, 0));
    }
	else if (sParams.eBufDataMode == BRAP_BufDataMode_eStereoInterleaved)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, BUFFER_PAIR_ENABLE, 0));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_CH_MODE, 0));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_SIZE_DOUBLE, 1));
    }
	else if (sParams.eBufDataMode == BRAP_BufDataMode_eStereoNoninterleaved)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, BUFFER_PAIR_ENABLE, 1));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_CH_MODE, 0));/* don't care */
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_SIZE_DOUBLE, 0));
    }
	else
	{
		BDBG_ERR(("BRAP_SRCCH_P_HWConfig(): Unhandled case bCompress(%d) eBufDataMode(%d)",
			sParams.bCompress, sParams.eBufDataMode));
		BDBG_ASSERT(0);
	}
    /* Set the LR Data control */
    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, LR_DATA_CTRL, sParams.eLRDataCntl));
	BDBG_MSG(("BRAP_SRCCH_P_HWConfig(): LR_DATA_CTRL = %d",sParams.eLRDataCntl));

	if(sParams.bNotPauseWhenEmpty)
	{
		/* Set 'Not Pause On Empty' = 1 */
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, NOT_PAUSE_WHEN_EMPTY, 1));
	}
	else
	{
    	/* Set 'Not Pause On Empty' = 0 */
	    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, NOT_PAUSE_WHEN_EMPTY, 0)); 
	}
	BDBG_MSG(("BRAP_SRCCH_P_HWConfig(): NOT_PAUSE_WHEN_EMPTY = %d",sParams.bNotPauseWhenEmpty));
				
    /* As per RDB recommendation,
      'Sample Repeat Enable'    = 0, for compressed stream
                                = 1, for uncompressed stream */
    if (sParams.bCompress == true)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_REPEAT_ENABLE, 0));
    }
    else
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_REPEAT_ENABLE, 1));
    }
    if (sParams.bSharedRbuf == true)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, 
                                                              SHARED_SBUF_ID, 
                                                              sParams.uiMasterSrcCh));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARE_SBUF, 1));        
    }

	switch(sParams.eInputBitsPerSample)
	{
		case BRAP_InputBitsPerSample_e8:
	        ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION, Bitwidth8));
			break;
			
		case BRAP_InputBitsPerSample_e16:
	        ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION, Bitwidth16));			
			break;
			
		case BRAP_InputBitsPerSample_e32:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION, Bitwidth32));			
			break;
			
		default: 	
			BDBG_ASSERT(0);
	}

    /* DMA_READ_DISABLE and FLOWON_SFIFO_HALFFULL should be set to 1 if data is written directly to the
       source FIFO. Ex: Bypass mode of the capture channel. For all other channels types and modes, they
       should retain the reset value 0 */
    if(true == sParams.bCapDirect2SrcFIFO)
    {
		/* ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SFIFO_START_HALFFULL, 1));	*/		
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, DMA_READ_DISABLE, 1));
    }
    else
    {
		/* ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SFIFO_START_HALFFULL, 0)); */
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, DMA_READ_DISABLE, 0));
    }
    /* Hack for 7440: found during audio bring up */
    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SFIFO_START_HALFFULL, 1)); 
    
	/* Program process_id_high field given in sParams*/
    if(sParams.bProcessIdHigh)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_ID_HIGH, 1));
    }
    else
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_ID_HIGH, 0));
    }

    /* Make PROCESS_SEQ_ID_VALID to 1 */
    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_SEQ_ID_VALID, 1));
    
    /* Leave all other field settings as Reset values */
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset, ui32RegVal);

    /*These two RDB's are programmed here because the grouping and the process_id_high 
        are known only at the start time*/
	/* Program group ID in the RDB and channel handle given in sParams*/

    ui32RegVal = BRAP_Read32 (hSrcCh->hRegister, 
                                BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + ui32Offset);

    ui32RegVal &= ~((BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID)));

    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID,sParams.uiGroupId));
#if ((!((BCHP_CHIP == 7440) &&(BCHP_VER == A0)))&&(BCHP_CHIP != 3563))
    ui32RegVal &= ~((BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_GRPi, INVERT_MSB)));

#if (BRAP_UNSIGNED_PCM_SUPPORTED == 1)
    if(true == hSrcCh->sParams.bIsSigned)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_GRPi, INVERT_MSB,0));
    }
    else
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_GRPi, INVERT_MSB,1));
    }
#else
    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_GRPi, INVERT_MSB,0));
#endif
#endif

    BRAP_Write32 (hSrcCh->hRegister,
                BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + hSrcCh->ui32Offset,
                ui32RegVal);
	

    BDBG_LEAVE (BRAP_SOURCECH_P_HWConfig);
    return ret;
}
#endif


BERR_Code BRAP_SRCCH_P_Open (
    BRAP_FMM_P_Handle       hFmm,            /* [in] Parent FMM handle */
    BRAP_SRCCH_P_Handle *   phSrcCh,         /* [out] Pointer to Source 
                                                Channel handle */
    unsigned int            uiSrcChIndex,    /* [in] Source channel index */
    const BRAP_SRCCH_P_Settings * pSettings  /* [in] Source channel settings */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_SRCCH_P_Handle hSrcCh;
#if (BRAP_7440_ARCH==1)
	uint32_t ui32RegVal = 0;
#endif

    BDBG_ENTER (BRAP_SRCCH_P_Open);
    BDBG_MSG (("BRAP_SRCCH_P_Open:"
               "hFmm=0x%x, uiSrcChIndex=%d,"
               "No settings",
               hFmm, uiSrcChIndex));

    /* 1. Check all input parameters to the function. Return error if
     * - FMM handle is NULL
     * - Given index exceeds maximum no. of Source Channels
     * - Pointer to Settings structure is NULL
     * - Pointer to Source Channel handle is NULL     */
    BDBG_ASSERT (hFmm);
    BDBG_ASSERT (phSrcCh);
    BSTD_UNUSED  (pSettings);
	
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   /* If not in WatchDog recovery */    
        BDBG_ASSERT (pSettings);
    }

    if (  uiSrcChIndex > (BRAP_RM_P_MAX_SRC_CHANNELS -1))
    {
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }       

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   /* If not in WatchDog recovery */  
        
        if (hFmm->hSrcCh[uiSrcChIndex] != NULL )
        {
            BDBG_ERR (("BRAP_SRCCH_P_Open: Source Channel %d is already open", uiSrcChIndex));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
        }
    
        /* 2. Allocate memory for the Source Channel handle. Fill in parameters in the Source Channel
        * handle. */
    
        /* Allocate Src Channel handle */
        hSrcCh = (BRAP_SRCCH_P_Handle) BKNI_Malloc (sizeof(BRAP_SRCCH_P_Object));
        if (hSrcCh == NULL)
        {
            return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
        }
        
        /* Clear the handle memory */
        BKNI_Memset (hSrcCh, 0, sizeof(BRAP_SRCCH_P_Object));

        /* Initialise known elements in Source Channel handle */
        hSrcCh->hChip = hFmm->hChip;
        hSrcCh->hRegister = hFmm->hRegister;
        hSrcCh->hHeap = hFmm->hHeap;
        hSrcCh->hInt = hFmm->hInt;
        hSrcCh->hFmm = hFmm;
        hSrcCh->uiIndex = uiSrcChIndex;
#if	(BRAP_7440_ARCH == 0)
        hSrcCh->ui32Offset = (BCHP_AUD_FMM_BF_CTRL_SOURCECH_1_CFG  
                       - BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG) 
                       * uiSrcChIndex;
#else
        hSrcCh->ui32Offset = uiSrcChIndex *4;
		
    	hSrcCh->uiGroupId = uiSrcChIndex;

        /* Program the Group Id same as SrcCh Id */
        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID ,hSrcCh->uiGroupId));
        BRAP_Write32 (hSrcCh->hRegister,
                                BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + hSrcCh->ui32Offset,
                                ui32RegVal);
    
#endif
    }   /* End: If not in WatchDog recovery */
    else
    {
        hSrcCh = *phSrcCh;
    }
    hSrcCh->eState = BRAP_SRCCH_P_State_eStopped;  

    /* Fill up the Source Channel Settings Structure */
    /* No settings at present */
    
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag(hFmm) == false)
    {   /* If not in WatchDog recovery */ 

        /* Store Source Channel handle inside the FMM handle */
        hFmm->hSrcCh[uiSrcChIndex] = hSrcCh;
    
        *phSrcCh = hSrcCh;
    }   /* End: If not in WatchDog recovery */        
    
    BDBG_MSG(("BRAP_SRCCH_P_Open: handle=0x%x", hSrcCh)); 
    BDBG_LEAVE (BRAP_SRCCH_P_Open);
    return ret;
}




BERR_Code BRAP_SRCCH_P_Close ( 
    BRAP_SRCCH_P_Handle hSrcCh      /* [in] Source Channel Handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
#if (BRAP_7440_ARCH==1)
    uint32_t  ui32RegVal = 0;
#endif

    BDBG_ENTER (BRAP_SRCCH_P_Close);
    BDBG_ASSERT (hSrcCh);
    
    BDBG_MSG (("BRAP_SRCCH_P_Close(): hSrcCh=0x%x, Index=%d ", 
                hSrcCh, hSrcCh->uiIndex));

#if (BRAP_7440_ARCH==1)
    /* Program the Group Id same as SrcCh Id */
    ui32RegVal = 0;
    ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID ,hSrcCh->uiIndex));
    BRAP_Write32 (hSrcCh->hRegister,
                            BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + hSrcCh->ui32Offset,
                            ui32RegVal);
#endif    

    /* Remove referrence to this Source Channel from the parent FMM */ 
    hSrcCh->hFmm->hSrcCh[hSrcCh->uiIndex] = NULL;

    /* Free the Source Channel Handle memory*/
    BKNI_Free (hSrcCh); 
                 
    BDBG_LEAVE (BRAP_SRCCH_P_Close);
    return ret;
}

#ifndef BCHP_7411_VER 
#if (BRAP_7440_ARCH==0)
static void BRAP_SRCCH_P_ProgramBufMap(BREG_Handle hRegister, 
						   unsigned int uiChIndx, 
						   unsigned int uiLtRBufId,
						   unsigned int uiRtRBufId)
{
    uint32_t  ui32RegVal;
  

	switch (uiChIndx)
	{	
		case 0:
			ui32RegVal = BRAP_Read32(hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_0);
			if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_0, PLAYBACK_BUF_00));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_0, 
									PLAYBACK_BUF_00, uiLtRBufId));
			}

			if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_0, PLAYBACK_BUF_01));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_0, 
									PLAYBACK_BUF_01, uiRtRBufId));
			}	
                    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_0, ui32RegVal);			
			break;
			
		case 1:
			ui32RegVal = BRAP_Read32(hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_0);
			if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_0, PLAYBACK_BUF_10));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_0, 
									PLAYBACK_BUF_10, uiLtRBufId));
			}

			if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_0, PLAYBACK_BUF_11));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_0, 
									PLAYBACK_BUF_11, uiRtRBufId));
			}	
				BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_0, ui32RegVal);			
			break;
			
		case 2:
			ui32RegVal = BRAP_Read32(hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_0);
			if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_0, PLAYBACK_BUF_20));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_0, 
									PLAYBACK_BUF_20, uiLtRBufId));
			}

			if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_0, PLAYBACK_BUF_21));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_0, 
									PLAYBACK_BUF_21, uiRtRBufId));
			}	
                BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_0, ui32RegVal);						
			break;
			
		case 3:	
			ui32RegVal = BRAP_Read32(hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_0);
			if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_0, PLAYBACK_BUF_30));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_0, 
									PLAYBACK_BUF_30, uiLtRBufId));
			}

			if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_0, PLAYBACK_BUF_31));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_0, 
									PLAYBACK_BUF_31, uiRtRBufId));
			}	
               	BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_0, ui32RegVal);			
			break;

#if ( BCHP_CHIP == 7400 )		
		case 4:
			ui32RegVal = BRAP_Read32(hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_1);
			if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_1, PLAYBACK_BUF_40));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_1, 
									PLAYBACK_BUF_40, uiLtRBufId));
			}

			if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_1, PLAYBACK_BUF_41));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_1, 
									PLAYBACK_BUF_41, uiRtRBufId));
			}	
             	       BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_1, ui32RegVal);			
			break;
			
		case 5:
			ui32RegVal = BRAP_Read32(hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_1);
			if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_1, PLAYBACK_BUF_50));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_1, 
									PLAYBACK_BUF_50, uiLtRBufId));
			}

			if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_1, PLAYBACK_BUF_51));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_1, 
									PLAYBACK_BUF_51, uiRtRBufId));
			}	
             	       BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_1, ui32RegVal);			
			break;
			
		case 6:
			ui32RegVal = BRAP_Read32(hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_2);
			if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_2, PLAYBACK_BUF_60));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_2, 
									PLAYBACK_BUF_60, uiLtRBufId));
			}

			if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_2, PLAYBACK_BUF_61));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_2, 
									PLAYBACK_BUF_61, uiRtRBufId));
			}	
             	       BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_2, ui32RegVal);			
			break;
			
		case 7:
			ui32RegVal = BRAP_Read32(hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_2);
			if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_2, PLAYBACK_BUF_70));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_2, 
									PLAYBACK_BUF_70, uiLtRBufId));
			}

			if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_2, PLAYBACK_BUF_71));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_2, 
									PLAYBACK_BUF_71, uiRtRBufId));
			}	
            BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_2, ui32RegVal);			
			break;
#endif			
			
		default:
			BDBG_ASSERT(0);
	}

}
#endif /* 7440 */
#endif

BERR_Code BRAP_SRCCH_P_Start ( 
    BRAP_SRCCH_P_Handle     hSrcCh,        /* [in] Source Channel Handle */
    const BRAP_SRCCH_P_Params *   pParams  /* [in] Pointer to start
                                              parameters for this Source 
                                              Channel */ 
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal;
    uint32_t ui32Offset;
    BREG_Handle hRegister;
    bool bWdgRecovery = false;        

    BDBG_ENTER (BRAP_SRCCH_P_Start);
    BDBG_ASSERT (hSrcCh);
    bWdgRecovery = BRAP_FMM_P_GetWatchdogRecoveryFlag (hSrcCh->hFmm);
    
    if(bWdgRecovery == false)
    {   /* If not in WatchDog recovery */  
    
        BDBG_ASSERT (pParams);
    
        /* Store the start parameters inside the handle */
        hSrcCh->sParams = *pParams;

        BDBG_MSG (("BRAP_SRCCH_P_Start(): hSrcCh=0x%x, Index=%d, "
                "hSrcCh->sParams.bInputSrValid=%d, "
                "hSrcCh->sParams.eBufDataMode=%d, "
                "hSrcCh->sParams.bCompress=%d, "
                "hSrcCh->sParams.eInputSR=%d, "
                "hSrcCh->sParams.eOutputSR=%d, ",
                hSrcCh, hSrcCh->uiIndex, 
                hSrcCh->sParams.bInputSrValid, 
                hSrcCh->sParams.eBufDataMode,
                hSrcCh->sParams.bCompress, 
                hSrcCh->sParams.eInputSR,
                hSrcCh->sParams.eOutputSR));
#if (BRAP_7440_ARCH == 0)
        BDBG_MSG(("hSrcCh->sParams.uiLtOrSnglRBufId=%d, "
                "hSrcCh->sParams.uiRtRBufId=%d, ",
                hSrcCh->sParams.uiLtOrSnglRBufId,
                hSrcCh->sParams.uiRtRBufId));
#endif                
#if BRAP_UNSIGNED_PCM_SUPPORTED
        BDBG_MSG(("hSrcCh->sParams.bIsSigned=%d ",hSrcCh->sParams.bIsSigned));
#endif
    }   /* End: If not in WatchDog recovery */
   
    
    hRegister = hSrcCh->hRegister;
    ui32Offset = hSrcCh->ui32Offset;

    if ((hSrcCh->eState == BRAP_SRCCH_P_State_eStopped) || 
        (true == bWdgRecovery))
    {
        /* Configure Source Channel Hardware */
        BRAP_SRCCH_P_HWConfig (hSrcCh); 

#ifndef BCHP_7411_VER 
#if (BRAP_7440_ARCH==0)
		/* Program buffer mapping */
		BRAP_SRCCH_P_ProgramBufMap(hRegister, hSrcCh->uiIndex, 
                hSrcCh->sParams.uiLtOrSnglRBufId, hSrcCh->sParams.uiRtRBufId);
#endif
#endif
        /* Change Software state Before enabling the SRCCH */
        hSrcCh->eState = BRAP_SRCCH_P_State_eRunning;
        
        /* Enable the Source channel */
#if (BRAP_7440_ARCH ==1)
        ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset);    
        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE, 1));
        BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset, ui32RegVal);
#else
        ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG + ui32Offset);    
        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SOURCEFIFO_ENABLE));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SOURCEFIFO_ENABLE, 1));
        BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG + ui32Offset, ui32RegVal);
#endif
        /* Start the Source Channel */
        ui32RegVal = 0;
#if (BRAP_7440_ARCH == 0)
#if (BCHP_CHIP == 7400) || (BRAP_7401_FAMILY == 1)
		/* For 7400 etc when shared ring buffer is enabled, the slave RBUF need not have START_SELECTION enabled.
		This is because, if master is a decode channel then DSP controls the SRART_WR_POINTER for master only.
		This causes a distorted audio in slave.*/	

		if(hSrcCh->sParams.bStartSelection == true &&
			hSrcCh->sParams.bSharedRbuf == false )
        {
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, START_SELECTION, 1));
		}
#else
		if(hSrcCh->sParams.bStartSelection == true)
		{        
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, START_SELECTION, 1));
		}
#endif		
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, PLAY_RUN, 1));
        BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CTRL + ui32Offset, ui32RegVal);
#else
        /*Program Start Selection*/
        ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset);
        if(hSrcCh->sParams.bStartSelection == true)
		{        
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, START_SELECTION, 1));
        }
		else
		{
	        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, START_SELECTION, 0));
		}
        BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset, ui32RegVal);

        /*Program Play Run*/
        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CTRLi, PLAY_RUN, 1));
        BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + ui32Offset, ui32RegVal);
#endif

    }
    else
    {
        /* Start can happen only after the Stopped state */
        BDBG_WRN (("BRAP_SRCCH_P_Start(): Start can happen only after the" 
                   " Stopped state. Ignoring this call."));
    }

    BDBG_LEAVE (BRAP_SRCCH_P_Start);
    return ret;
}



BERR_Code BRAP_SRCCH_P_Stop ( 
    BRAP_SRCCH_P_Handle     hSrcCh        /* [in] Source Channel Handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal=0x0,ui32RegVal1=0x0,ui32RegVal2=0x0,i=0;
    unsigned int ui32Offset,ui32Offset_Ada=0x0;
    BREG_Handle hRegister;

    BDBG_ENTER(BRAP_SRCCH_P_Stop);
    BDBG_ASSERT (hSrcCh);
    BDBG_MSG (("BRAP_SRCCH_P_Stop(): hSrcCh=0x%x, Index=%d ", hSrcCh, hSrcCh->uiIndex));
    
    hRegister = hSrcCh->hRegister;
    ui32Offset = hSrcCh->ui32Offset;
     ui32Offset_Ada = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_1_CFG -   BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG;

    if ( hSrcCh->eState ==  BRAP_SRCCH_P_State_eRunning)
    {
    #if (BRAP_7440_ARCH == 0)
        /* Disable the Source Channel */
        ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG + ui32Offset);    
        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SOURCEFIFO_ENABLE));

        /* Clear 'shared rbuf/src ch' info */
        ui32RegVal &=~(BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SHARED_SBUF_ID));
        ui32RegVal &=~(BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SHARE_SBUF));
        BRAP_Write32(hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG + ui32Offset, ui32RegVal);

        /* Clear Adaptive rate control */
        if(hSrcCh->sParams.bCompress == false)
        { 
			for(i=0;i<4;i++)
			{
				ui32RegVal1 = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + (i * ui32Offset_Ada )); 
				ui32RegVal2 = BCHP_GET_FIELD_DATA(ui32RegVal1, AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_SFIFO_SEL);

				if(ui32RegVal2 == hSrcCh->uiIndex)
				{
					BRAP_Write32(hSrcCh->hRegister, 
					(BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + (i*ui32Offset_Ada)), 
					0x0);   
					BRAP_Write32(hSrcCh->hRegister, 
					            (BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0 + (i*ui32Offset_Ada)), 
					            0x0);
					break;
				}			
			}
    	}

    #else
        ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset);
        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE, 0));

        /* Clear 'shared rbuf/src ch' info */
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, 
                                                              SHARED_SBUF_ID, 0));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARE_SBUF, 0));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, START_SELECTION, 0));

        BRAP_Write32(hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset, ui32RegVal);

        /* Stop the Source Channel */
        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CTRLi, PLAY_RUN, 0));
        BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + ui32Offset, ui32RegVal);

        /* Program the Group Id same as SrcCh Id */
        hSrcCh->uiGroupId = hSrcCh->uiIndex;
        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID, hSrcCh->uiGroupId));
        BRAP_Write32 (hSrcCh->hRegister,
                            BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + hSrcCh->ui32Offset,
                            ui32RegVal);
    #endif

        /* Change Software state */
        hSrcCh->eState = BRAP_SRCCH_P_State_eStopped;
    }
    else
    {
        /* SRCCH was not running */
        BDBG_WRN (("BRAP_SRCCH_P_Stop(): This source channel was not running. Ignoring this fnc call"));
    }

    BDBG_LEAVE (BRAP_SRCCH_P_Stop);
    return ret;
}

#if (BRAP_7440_ARCH==0) /* TODO: Enable if required later for 7440 */
BERR_Code BRAP_SRCCH_P_SetMute ( 
    BRAP_SRCCH_P_Handle  hSrcCh,   /* [in] Source Channel Handle */
    BRAP_MIXER_P_Handle  hMixer,   /* [in] Mixer handle of connected mixer*/    
    bool                 bMute     /* [in] Mute enable/disable:
                                          TRUE: Mute source channel
                                          FALSE: Un-Mute */                     
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal;
    uint32_t ui32Temp=0;
    unsigned int uiStreamId=0;    

    BDBG_ENTER (BRAP_SRCCH_P_SetMute);
    BDBG_ASSERT (hSrcCh);
    BDBG_ASSERT (hMixer);
    BDBG_MSG (("BRAP_SRCCH_P_SetMute(): hSrcCh=0x%x, Index=%d, bMute=%d",
                hSrcCh, hSrcCh->uiIndex, bMute));
    
    if (hSrcCh->sParams.bCompress == true)
    {
        BDBG_ERR(("BRAP_SRCCH_P_SetMute(): valid only for uncompressed data."));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }

    ret = BRAP_RM_P_GetDpStreamId (hSrcCh->uiIndex, 
                                   hMixer->uiDpIndex, &uiStreamId); 
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_MIXER_P_HWConfig(): Cant get stream index in this DP"));
        return BERR_TRACE (ret);
    }
    
    ui32Temp = BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK1_SCALE_SRC_CONFIG
                - BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG;


    ui32RegVal = BRAP_Read32 (hMixer->hRegister, 
                    (   hMixer->ui32DpOffset 
                      + BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG + 
                      + (uiStreamId * ui32Temp)));

    ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, SCALE_MUTE_ENA));
    
    if (bMute == true )
    {
        ui32RegVal |= BCHP_FIELD_ENUM (
                            AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, 
                            SCALE_MUTE_ENA, 
                            Enable);
    }
    else
    {
        ui32RegVal |= BCHP_FIELD_ENUM (
                            AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, 
                            SCALE_MUTE_ENA, 
                            Disable);
    }

        
    /* Write to the SRC Config register */
    BRAP_Write32 (hMixer->hRegister, 
                    (hMixer->ui32DpOffset 
                      + BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG + 
                      + (uiStreamId * ui32Temp)), 
                     ui32RegVal);
      
    hSrcCh->bMute = bMute; 
    BDBG_LEAVE (BRAP_SRCCH_P_SetMute);
    return ret;    
}

BERR_Code BRAP_SRCCH_P_WaitForRampDown ( 
    BRAP_SRCCH_P_Handle  hSrcCh,   /* [in] Source Channel Handle */
    BRAP_MIXER_P_Handle  hMixer   /* [in] Mixer handle of connected mixer*/    
                     
)
{
    BERR_Code ret = BERR_SUCCESS;
    volatile uint32_t  ui32RegVal;
    volatile unsigned int uiTemp; 
    unsigned int uiStreamId, i;

    BDBG_ENTER (BRAP_SRCCH_P_WaitForRampDown);
    BDBG_ASSERT (hSrcCh);
    BDBG_ASSERT (hMixer);
    BDBG_MSG (("BRAP_SRCCH_P_WaitForRampDown(): hSrcCh=0x%x, Index=%d",
                hSrcCh, hSrcCh->uiIndex));

    if (hSrcCh->bMute == false)
    {
        BDBG_ERR(("BRAP_SRCCH_P_WaitForRampDown(): Channel was not muted. Exiting without wait."));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }   
    
    if (hSrcCh->sParams.bCompress == true)
    {
        BDBG_ERR(("BRAP_SRCCH_P_WaitForRampDown(): valid only for uncompressed data."));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }

    ret = BRAP_RM_P_GetDpStreamId (hSrcCh->uiIndex, hMixer->uiDpIndex, &uiStreamId); 
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_SRCCH_P_WaitForRampDown(): Cant get stream index in this DP"));
        return BERR_TRACE (ret);
    }
    
    /* Poll on the AUD_FMM_DP_ESR*_STATUS register,  */
    for (i=0; i <BRAP_SRCCH_P_RAMP_DOWN_TIMEOUT; i ++)
    {
        ui32RegVal = BRAP_Read32 (hMixer->hRegister,
                             (hMixer->ui32DpOffset  + BCHP_AUD_FMM_DP_ESR0_STATUS));
        switch (uiStreamId)
        {
            case 0:
                uiTemp = BCHP_GET_FIELD_DATA (
                        ui32RegVal,
                        AUD_FMM_DP_ESR0_STATUS, 
                        SCALE_RAMP_DONE0) ;
                BDBG_MSG (("SCALE_RAMP_DONE0=%d", uiTemp));
                break;
            case 1:
                uiTemp = BCHP_GET_FIELD_DATA (
                        ui32RegVal,
                        AUD_FMM_DP_ESR0_STATUS, 
                        SCALE_RAMP_DONE1) ;
                BDBG_MSG (("SCALE_RAMP_DONE1=%d", uiTemp));
                break;
            case 2:
                uiTemp = BCHP_GET_FIELD_DATA (
                        ui32RegVal,
                        AUD_FMM_DP_ESR0_STATUS, 
                        SCALE_RAMP_DONE2) ;
                BDBG_MSG (("SCALE_RAMP_DONE2=%d", uiTemp));
                break;
            case 3:
                uiTemp = BCHP_GET_FIELD_DATA (
                        ui32RegVal,
                        AUD_FMM_DP_ESR0_STATUS, 
                        SCALE_RAMP_DONE3) ;
                BDBG_MSG (("SCALE_RAMP_DONE3=%d", uiTemp));
                break;                
            default:
                BDBG_ERR(("BRAP_SRCCH_P_WaitForRampDown(): Incorrect stream index %d",
                            uiStreamId));
                return BERR_TRACE (BERR_NOT_SUPPORTED);                
        }
        if (uiTemp!=0) 
        {
            break;
        }
           BKNI_Sleep(1);
    }
    if (i >= BRAP_SRCCH_P_RAMP_DOWN_TIMEOUT)
    {
        BDBG_ERR(("Timed out waiting for ramp down to complete!!!\n")) ;
#if 0 
        /* dont return error on timeout. else Stop_Decode() will fail). shud
        find some other way to convey this timeout */ 
        return BERR_TRACE(BERR_TIMEOUT) ; 
#endif        
    }

    BDBG_LEAVE (BRAP_SRCCH_P_WaitForRampDown);
    return ret;    
}

BERR_Code BRAP_SRCCH_P_GetMute ( 
    BRAP_SRCCH_P_Handle  hSrcCh,   /* [in] Source Channel Handle */
    bool *               pMute     /* [out] Pointer to user memory 
                                            to write Mute Status */        
)
{
    BERR_Code  ret = BERR_SUCCESS;

    BDBG_ASSERT (hSrcCh);
    
    BDBG_ENTER (BRAP_SRCCH_P_GetMute);

    if (hSrcCh->sParams.bCompress == true )
    {
        BDBG_ERR (("BRAP_SRCCH_P_GetMute(): valid only for uncompressed data."));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    
    *pMute = hSrcCh->bMute;

    BDBG_MSG (("BRAP_SRCCH_P_GetMute(): hSrcCh=0x%x, Index=%d, bMute=%d",
                hSrcCh, hSrcCh->uiIndex, *pMute));
    BDBG_LEAVE (BRAP_SRCCH_P_GetMute);
    return ret;
}
#endif /* Not for 7440 */

#if 0 /*TBD later */

BERR_Code 
BRAP_SRCCH_P_Pause( 
               BRAP_SRCCH_P_Handle hSrcCh 
)
{
    BERR_Code                ret = BERR_SUCCESS;
    uint32_t            reg;
    uint32_t            ui32Offset;
    BREG_Handle            hRegister = hSrcCh->hRegister;

    BDBG_ENTER(BRAP_SRCCH_P_Pause);
    BDBG_MSG(("BRAP_SRCCH_Pause(): ID=0x%08x ", hSrcCh->uiIndex));

    if ( hSrcCh->eState ==  BRAP_SRCCH_State_eRuning)
    {
        ui32Offset = hSrcCh->ui32Offset;

        /* Pause the playback */
        reg = BRAP_Read32(hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CTRL + ui32Offset);    
        reg &=~(BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, PLAY_RUN));
        reg |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, PLAY_RUN, 0));
        BRAP_Write32(hRegister,   BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CTRL + ui32Offset , reg);

        hSrcCh->eState = BRAP_SRCCH_State_ePaused;
        
    }
    else
    {
        ret = BRAP_ERR_BAD_STATE;
    }

    BDBG_LEAVE(BRAP_SRCCH_P_Pause);

    return ret;
}


BERR_Code 
BRAP_SRCCH_P_Resume( 
                BRAP_SRCCH_P_Handle hSrcCh 
                )
{
    BERR_Code                ret = BERR_SUCCESS;
    uint32_t            reg;
    uint32_t            ui32Offset;
    BREG_Handle            hRegister = hSrcCh->hRegister;

    BDBG_ENTER(BRAP_SRCCH_P_Resume);
    BDBG_MSG(("BRAP_SRCCH_Resume(): ID=0x%08x ", hSrcCh->uiIndex));

    if ( hSrcCh->eState == BRAP_SRCCH_State_ePaused )
    {
        ui32Offset = hSrcCh->ui32Offset;

        /* Start the playback */
        reg = BRAP_Read32(hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CTRL + ui32Offset);    
        reg &=~(BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, PLAY_RUN));
        reg |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, PLAY_RUN, 1));
        BRAP_Write32(hRegister,   BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CTRL + ui32Offset , reg);
        
        hSrcCh->eState = BRAP_SRCCH_State_eRuning;
        
    }
    else
    {
        ret = BRAP_ERR_BAD_STATE;
    }

    BDBG_LEAVE(BRAP_SRCCH_P_Resume);

    return ret;
}





BERR_Code 
BRAP_SRCCH_P_ShareRingBuffer(
        BRAP_SRCCH_P_Handle hSrcCh,    /* [in] Present source channel handle 
                                        (this is going to act as slave) */
        uint32_t uiChannelIndex)        /* [in] The Source FIFO index, whose input 
                                        is to be shared (this is going to 
                                        act as Master) */
{
    BERR_Code        ret = BERR_SUCCESS;
    uint32_t        ui32Offset;
    uint32_t        reg;

    BREG_Handle            hRegister = hSrcCh->hRegister;
    
    ui32Offset = hSrcCh->ui32Offset;    

    /* Program uiChannelIndex to SHARED_SBUF_ID (This source FIFO will act as a master 
       i.e. the ring buffers connected to this source FIFO is going to be shared) & 1 
       to SHARE_SBUF of AUD_FMM_BF_CTRL_SOURCECH_0_CFG Register */
    
    reg = BRAP_Read32(hRegister,   BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG  + ui32Offset);
    reg &=~((BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SHARED_SBUF_ID)) |
            (BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_0_CFG, SHARE_SBUF)) );
    reg |=    (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, SHARED_SBUF_ID, uiChannelIndex)) |
            (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_0_CTRL, SHARE_SBUF, 1));
    BRAP_Write32(hRegister,   BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG + ui32Offset , reg);
    
    return ret;
}



BERR_Code
BRAP_SRCCH_P_SetBlockCnt(
                    BRAP_SRCCH_P_Handle hSrcCh,
                    BAVC_AudioSamplingRate  eSR
                    )
{
    /* for 32 khz , it should be 1, for 96 khz it shud be 2 and so on */
 
    switch(eSR)
        {
          case BAVC_AudioSamplingRate_e32:
              /* program DMA_BLOCK_CNT_7* */
          case BAVC_AudioSamplingRate_e44:
          case BAVC_AudioSamplingRate_e48:
          case BAVC_AudioSamplingRate_e92:
          case BAVC_AudioSamplingRate_e192:
        }
}




#endif /*TBD later*/


/* End of File */
