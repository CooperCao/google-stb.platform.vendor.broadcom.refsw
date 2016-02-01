/***************************************************************************
*     Copyright (c) 2004-2009, Broadcom Corporation
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
    false,                  /* bProcessIdHigh */
    BRAP_RM_P_INVALID_INDEX /* uiGroupId */
#if BRAP_UNSIGNED_PCM_SUPPORTED
        ,true        /* bIsSigned */
#endif
    ,BRAP_DataEndian_eSame
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

    if(BRAP_DataEndian_eSwap == sParams.eDataSwap)
    {
        /* Asking FMM to take swapped data */
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, REVERSE_ENDIAN, 1));
    }
    else
    {
        /* Asking FMM to take same data */
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, REVERSE_ENDIAN, 0));
    }

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


    BRAP_Write32 (hSrcCh->hRegister,
                BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + hSrcCh->ui32Offset,
                ui32RegVal);
	

    BDBG_LEAVE (BRAP_SOURCECH_P_HWConfig);
    return ret;
}

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
    uint32_t ui32RegVal = 0;


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
        hSrcCh->ui32Offset = uiSrcChIndex *4;
		
    	hSrcCh->uiGroupId = uiSrcChIndex;

        /* Program the Group Id same as SrcCh Id */
        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID ,hSrcCh->uiGroupId));
        BRAP_Write32 (hSrcCh->hRegister,
                                BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + hSrcCh->ui32Offset,
                                ui32RegVal);
    
    }   /* End: If not in WatchDog recovery */
    else
    {
        hSrcCh = *phSrcCh;
    }

    /* Fill up the Source Channel Settings Structure */
    /* No settings at present */
    
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag(hFmm) == false)
    {   /* If not in WatchDog recovery */ 
        hSrcCh->eState = BRAP_SRCCH_P_State_eStopped; 

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
    uint32_t  ui32RegVal = 0;

    BDBG_ENTER (BRAP_SRCCH_P_Close);
    BDBG_ASSERT (hSrcCh);
    
    BDBG_MSG (("BRAP_SRCCH_P_Close(): hSrcCh=0x%x, Index=%d ", 
                hSrcCh, hSrcCh->uiIndex));


    /* Program the Group Id same as SrcCh Id */
    ui32RegVal = 0;
    ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID ,hSrcCh->uiIndex));
    BRAP_Write32 (hSrcCh->hRegister,
                            BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + hSrcCh->ui32Offset,
                            ui32RegVal);


    /* Remove referrence to this Source Channel from the parent FMM */ 
    hSrcCh->hFmm->hSrcCh[hSrcCh->uiIndex] = NULL;

    /* Free the Source Channel Handle memory*/
    BKNI_Free (hSrcCh); 
                 
    BDBG_LEAVE (BRAP_SRCCH_P_Close);
    return ret;
}


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
#if BRAP_UNSIGNED_PCM_SUPPORTED
        BDBG_MSG(("hSrcCh->sParams.bIsSigned=%d ",hSrcCh->sParams.bIsSigned));
#endif
    }   /* End: If not in WatchDog recovery */
   
    
    hRegister = hSrcCh->hRegister;
    ui32Offset = hSrcCh->ui32Offset;

    if ((true == bWdgRecovery)
        || (hSrcCh->eState == BRAP_SRCCH_P_State_eStopped))
    {
        /* Configure Source Channel Hardware */
        BRAP_SRCCH_P_HWConfig (hSrcCh); 


        if(false == bWdgRecovery)
        {
        /* Change Software state Before enabling the SRCCH */
        hSrcCh->eState = BRAP_SRCCH_P_State_eRunning;
        }
        
        /* Enable the Source channel */

        ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset);    
        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE, 1));
        BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset, ui32RegVal);

        /* Start the Source Channel */
        ui32RegVal = 0;

        /*Program Start Selection*/
        ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset);
        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, START_SELECTION));
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


    }
    else
    {
        /* Start can happen only after the Stopped state */
        BDBG_MSG (("BRAP_SRCCH_P_Start(): Start can happen only after the" 
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
    uint32_t  ui32RegVal;
    unsigned int ui32Offset;
    BREG_Handle hRegister;
    unsigned int uiTimeOutCtr=0;

    BDBG_ENTER(BRAP_SRCCH_P_Stop);
    BDBG_ASSERT (hSrcCh);
    BDBG_MSG (("BRAP_SRCCH_P_Stop(): hSrcCh=0x%x, Index=%d ", hSrcCh, hSrcCh->uiIndex));
    
    hRegister = hSrcCh->hRegister;
    ui32Offset = hSrcCh->ui32Offset;

    if ( hSrcCh->eState ==  BRAP_SRCCH_P_State_eRunning)
    {
        /* Stop the Source Channel */
    /*Set PLAY_RUN=0, and optionally wait for GROUP_FLOWON=0 to confirm.*/
        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CTRLi, PLAY_RUN, 0));
        BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + ui32Offset, ui32RegVal);

	uiTimeOutCtr = 100 /*BRAP_P_STOP_TIMEOUT_COUNTER*/;
	do
	{
		if(100 != uiTimeOutCtr)
		{
			BKNI_Sleep(1);
		}
		ui32RegVal = BRAP_Read32(hRegister,BCHP_AUD_FMM_BF_CTRL_SOURCECH_GROUP_FLOWON);
                BDBG_MSG(("FLOW ON=%d, hSrcCh->uiIndex =%d",ui32RegVal,hSrcCh->uiIndex));
		ui32RegVal = BCHP_GET_FIELD_DATA(ui32RegVal, AUD_FMM_BF_CTRL_SOURCECH_GROUP_FLOWON, STATUS);
		ui32RegVal &= (1<<hSrcCh->uiIndex);
		uiTimeOutCtr--;		
	}while((0!=ui32RegVal) && (0!=uiTimeOutCtr));

	if(0!=ui32RegVal)
	{
		BDBG_ERR(("AUD_FMM_BF_CTRL_SOURCECH_GROUP_FLOWON=%#x is not disabled. [Time out]",ui32RegVal));
	}



        ui32RegVal = 0;
        ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset);
        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE, 0));

        /* Clear 'shared rbuf/src ch' info */
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, 
                                                              SHARED_SBUF_ID, 0));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARE_SBUF, 0));
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_SOURCECH_CFGi, START_SELECTION, 0));

        BRAP_Write32(hRegister, BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + ui32Offset, ui32RegVal);

        /* Program the Group Id same as SrcCh Id */
        hSrcCh->uiGroupId = hSrcCh->uiIndex;
        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID, hSrcCh->uiGroupId));
        BRAP_Write32 (hSrcCh->hRegister,
                            BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + hSrcCh->ui32Offset,
                            ui32RegVal);
        /* Change Software state */
        hSrcCh->eState = BRAP_SRCCH_P_State_eStopped;
    }
    else
    {
        /* SRCCH was not running */
        BDBG_MSG (("BRAP_SRCCH_P_Stop(): This source channel was not running. Ignoring this fnc call"));
    }

    BDBG_LEAVE (BRAP_SRCCH_P_Stop);
    return ret;
}




/* End of File */
