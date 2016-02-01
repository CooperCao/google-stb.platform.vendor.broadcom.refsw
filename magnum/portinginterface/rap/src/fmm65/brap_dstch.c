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
*   Module name: DSTCH
*   This file contains the implementation of all APIs for the Destination
*   Channel (Fifo) abstraction. 
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE (rap_dstch);

static const BRAP_DSTCH_P_Params defDstChParams =
{
    BRAP_CaptureMode_eFullDuplex,   /*eCapMode */
    BRAP_BufDataMode_eStereoNoninterleaved, 
                                    /* eBufDataMode */    
    false,                          /* bNotPauseWhenFull */
    BRAP_RM_P_INVALID_INDEX,        /* uiSrcChId */
    false,                          /* bProcessIdHigh*/
    BRAP_INVALID_VALUE,             /* uiInputFciId */
    false                           /* bFwToProgramCaptureRun */
};

/***************************************************************************
Summary:
    Returns default values for Destination Channel Start time parameters.

Description:
    For parameters that the system cannot assign default values to, 
    an invalid value is returned. Note that the default parameters are common
    for all Destination Channels.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_DSTCH_P_GetDefaultSettings.
**************************************************************************/
BERR_Code 
BRAP_DSTCH_P_GetDefaultParams ( 
    BRAP_DSTCH_P_Params     *pDefParams     /* Pointer to memory where default
                                               settings should be written */    
)
{
    BERR_Code  ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_DSTCH_P_GetDefaultParams);
    BDBG_ASSERT (pDefParams);

    *pDefParams = defDstChParams;

    BDBG_LEAVE(BRAP_DSTCH_P_GetDefaultParams);
    return ret;    
}

/***************************************************************************
Summary:
    Opens a Destination Channel.

Description:
    Initializes the Destination Channel and returns a Destination channel handle.    
    The handle can then be used for all other Destination channel function calls.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_DSTCH_P_Close, BRAP_DSTCH_P_GetDefaultSettings.
**************************************************************************/
BERR_Code BRAP_DSTCH_P_Open (
    BRAP_FMM_P_Handle           hFmm,           /* [in] Parent FMM handle */
    BRAP_DSTCH_P_Handle         *phDstCh,       /* [out] Pointer to Destination 
                                                   Channel handle */
    unsigned int                uiDstChIndex,   /* [in] Destination channel index */
    const BRAP_DSTCH_P_Settings *pSettings      /* [in] Destination channel settings */
)
{
	BERR_Code               ret = BERR_SUCCESS;
    BRAP_DSTCH_P_Handle     hDstCh = NULL;

    BDBG_ENTER (BRAP_DSTCH_P_Open);
    BDBG_MSG (("BRAP_DSTCH_P_Open:hFmm=0x%x, uiDstChIndex=%d,No settings",
                                                           hFmm, uiDstChIndex));

    /* Check all input parameters to the function. Return error if
     * - FMM handle is NULL
     * - Given index exceeds maximum no. of Destination Channels
     * - Pointer to Settings structure is NULL
     * - Pointer to Destination Channel handle is NULL     */
     
    BDBG_ASSERT (hFmm);
    BDBG_ASSERT (phDstCh);
    
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   
        /* If not in WatchDog recovery */    
        BDBG_ASSERT (pSettings);
    }
    BSTD_UNUSED(pSettings);
    
    if ( uiDstChIndex > (BRAP_RM_P_MAX_DST_CHANNELS -1))
    {
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }       

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   
        /* If not in WatchDog recovery */  
        if (hFmm->hDstCh[uiDstChIndex] != NULL )
        {
            BDBG_ERR (("BRAP_DSTCH_P_Open: Destination Channel %d is already open",
                                                                uiDstChIndex));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
        }
    
        /* Allocate memory for the Destination Channel handle. Fill in 
           parameters in the Destination Channel handle. */
    
        /* Allocate Dst Channel handle */
        hDstCh = (BRAP_DSTCH_P_Handle) BKNI_Malloc(sizeof(BRAP_DSTCH_P_Object));
        if (hDstCh == NULL)
        {
            return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
        }
        
        /* Clear the handle memory */
        BKNI_Memset (hDstCh, 0, sizeof(BRAP_DSTCH_P_Object));

        /* Initialise known elements in Destination Channel handle */
        hDstCh->hChip = hFmm->hChip;
        hDstCh->hRegister = hFmm->hRegister;
        hDstCh->hHeap = hFmm->hHeap;
        hDstCh->hInt = hFmm->hInt;
        hDstCh->hFmm = hFmm;
        hDstCh->uiIndex = uiDstChIndex;
        hDstCh->ui32Offset = 
            BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_ELEMENT_SIZE * uiDstChIndex;
        hDstCh->eState = BRAP_DSTCH_P_State_eStopped;  
    }   /* End: If not in WatchDog recovery */
    else
    {
        hDstCh = *phDstCh;
    }

    /* Fill up the Destination Channel Settings Structure */
    /* No settings at present */
    
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag(hFmm) == false)
    {   
        /* If not in WatchDog recovery */ 
        /* Store Destination Channel handle inside the FMM handle */
        hFmm->hDstCh[uiDstChIndex] = hDstCh;
    
        *phDstCh = hDstCh;
    }   /* End: If not in WatchDog recovery */        
    
    BDBG_MSG(("BRAP_DSTCH_P_Open: handle=0x%x", hDstCh)); 
    BDBG_LEAVE (BRAP_DSTCH_P_Open);
    return ret;
}

/***************************************************************************
Summary:
    Releases all the resources associated with this Destination Channel 
    and frees the handles.

Description:
    

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_DSTCH_P_Open
**************************************************************************/
BERR_Code BRAP_DSTCH_P_Close ( 
    BRAP_DSTCH_P_Handle hDstCh      /* [in] Destination Channel Handle */
)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_DSTCH_P_Close);
    BDBG_ASSERT (hDstCh);
    
    BDBG_MSG (("BRAP_DSTCH_P_Close(): hDstCh=0x%x, Index=%d ", 
                                                      hDstCh, hDstCh->uiIndex));
    
    /* Remove referrence to this Destination Channel from the parent FMM */ 
    hDstCh->hFmm->hDstCh[hDstCh->uiIndex] = NULL;

    /* Free the Destination Channel Handle memory*/
    BKNI_Free (hDstCh); 
                 
    BDBG_LEAVE (BRAP_DSTCH_P_Close);
    return ret;
}

/***************************************************************************
Summary:
    Configures the HW registers for the Destination Channel

Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    None.
**************************************************************************/
static BERR_Code BRAP_DSTCH_P_HWConfig ( 
    BRAP_DSTCH_P_Handle     hDstCh 		/* [in] Destination Channel handle */
)
{
    BERR_Code           ret = BERR_SUCCESS;
    uint32_t            ui32RegVal = 0;
    unsigned int        ui32Offset = 0;
    BREG_Handle         hRegister = NULL;
    BRAP_DSTCH_P_Params sParams;

    BDBG_ENTER (BRAP_DSTCH_P_HWConfig);
    BDBG_ASSERT (hDstCh);

    BDBG_MSG (("BRAP_DSTCH_P_HWConfig(): hDstCh=0x%x, Index=%d, offset=%d \n", 
                                hDstCh, hDstCh->uiIndex, hDstCh->ui32Offset));
  
    hRegister = hDstCh->hRegister;
    ui32Offset = hDstCh->ui32Offset;
    sParams = hDstCh->sParams;    
    
    ui32RegVal = BRAP_Read32 (hRegister, 
                      BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_BASE + ui32Offset);
    
    ui32RegVal &= ~ ((BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_SEQ_ID_VALID)) 
                    |(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, BLOCKED_ACCESS_DISABLE)) 
                    |(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_ID_HIGH)) 
                    |(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, DMA_BLOCK_CNT)) 
                    |(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, FCI_CAP_ID)) 
					|(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, NOT_PAUSE_WHEN_FULL))
					|(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, SOURCE_FIFO_ID))
					|(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, INPUT_FRM_SOURCEFIFO))
					|(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_TO_SOURCEFIFO))
					|(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, PLAY_FROM_CAPTURE))
					|(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, DESTFIFO_SIZE_DOUBLE))
					|(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, BUFFER_PAIR_ENABLE))
#if (BRAP_3548_FAMILY == 1)
					|(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_MODE))
#endif
			 		|(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_ENABLE))
		);

    /* Program PROCESS_SEQ_ID_VALID always valid */
	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_SEQ_ID_VALID, 1));

    /* Program BLOCKED_ACCESS_DISABLE to enable the access */
    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, BLOCKED_ACCESS_DISABLE, 0));
    
    /* Program PROCESS_ID_HIGH as given by user */
    if(true == sParams.bProcessIdHigh)
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_ID_HIGH, 1));
    }
    else
    {
        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_ID_HIGH, 0));
    }

    /* Program DMA_BLOCK_CNT to 0 */
    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, DMA_BLOCK_CNT, 0));

    /* Program FCI ID */

    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, FCI_CAP_ID, sParams.uiInputFciId));


    /* Configure Buffer Pair Enable */
  	switch(sParams.eBufDataMode)
	{
		case BRAP_BufDataMode_eMono:
		case BRAP_BufDataMode_eStereoInterleaved:
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, BUFFER_PAIR_ENABLE, 0));
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, DESTFIFO_SIZE_DOUBLE, 1));
#if (BRAP_3548_FAMILY == 1)             
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_MODE, 1));
#endif
			break;
		case BRAP_BufDataMode_eStereoNoninterleaved:
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, BUFFER_PAIR_ENABLE, 1));
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, DESTFIFO_SIZE_DOUBLE, 0));
#if (BRAP_3548_FAMILY == 1)             
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_MODE, 0));
#endif            
	        break;
		default:
			BDBG_ERR(("BRAP_DSTCH_P_HWConfig(): Invalid eBufDataMode(%d)",
						sParams.eBufDataMode));
			BDBG_ASSERT(0);
			break;
	}

	if(sParams.bNotPauseWhenFull)
	{
		/* Set 'Not Pause On Full' = 1 */
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, NOT_PAUSE_WHEN_FULL, 1));
	}
	else
	{
    	/* Set 'Not Pause On Full' = 0 */
	    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, NOT_PAUSE_WHEN_FULL, 0));
	}
	
	/* Configure depending upon the capture mode */
	switch(sParams.eCapMode)
	{
        case BRAP_CaptureMode_eByPass:
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_TO_SOURCEFIFO, 1));
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, SOURCE_FIFO_ID, sParams.uiSrcChId));
            BDBG_MSG (("DSTCH in ByPass Mode. Connected to SRCCH ID :%d",sParams.uiSrcChId));
            break;
            
		case BRAP_CaptureMode_eCaptureOnly:
			/* Capture Data to Destination FIFO and Don't play from Capture Ring Buffers */
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_TO_SOURCEFIFO, 0));
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, PLAY_FROM_CAPTURE, 0));
			break;
		case BRAP_CaptureMode_eFullDuplex:
			if  (sParams.uiSrcChId > BRAP_RM_P_MAX_SRC_CHANNELS) 
			{
				return BERR_TRACE (BERR_INVALID_PARAMETER);
			}

#if ((BRAP_3548_FAMILY != 1))
			/* 
            Note: For 3548, Ealier this was Capture Data to Destination FIFO and Play from Capture Ring Buffers.
			But due to the requirement of having Post processing algorithms on capture channel, we need
			to pass the PCM data to the DSP. Hence disabling the code below that plays from capture.

			In future if there is a need to revert back, we need to enable the two lines below for 3548 
			*/
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, SOURCE_FIFO_ID, sParams.uiSrcChId));
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, PLAY_FROM_CAPTURE, 1));
#endif
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_TO_SOURCEFIFO, 0));
			break;
		default:
			BDBG_ERR(("BRAP_DSTCH_P_HWConfig(): Invalid Capture Mode(%d)",
						sParams.eCapMode));
			BDBG_ASSERT(0);
			break;
	}

	
	/* Input Data is coming from Capture Input, not from DRAM */
	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, INPUT_FRM_SOURCEFIFO, 0));

	/* Enable Capture */
	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_ENABLE, 1));

    /* Leave all other field settings as Reset values */
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_BASE + ui32Offset, ui32RegVal);

    BDBG_LEAVE (BRAP_DESTCH_P_HWConfig);
    return ret;
}

/***************************************************************************
Summary:
    Starts a Destination Channel.

Description:
    To start the Destination FIFO taking data from ring buffer, this function 
    must be called. Destination channel will start based on the existing settings. 

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_DSTCH_P_Stop.
**************************************************************************/
BERR_Code BRAP_DSTCH_P_Start ( 
	BRAP_DSTCH_P_Handle         hDstCh,   /* [in] Destination Channel Handle */
    const BRAP_DSTCH_P_Params   *pParams  /* [in] Pointer to start
                                              parameters for this Destination 
                                              Channel */ 
)
{
    BERR_Code       ret = BERR_SUCCESS;
    uint32_t        ui32RegVal = 0;
    uint32_t        ui32Offset = 0;
    BREG_Handle     hRegister = NULL;
    bool            bEnableCapture = false;
    bool bWdogRecovery;

    BDBG_ENTER (BRAP_DSTCH_P_Start);
    BDBG_ASSERT (hDstCh);

    bWdogRecovery = BRAP_FMM_P_GetWatchdogRecoveryFlag (hDstCh->hFmm);
    if (bWdogRecovery == false)
    {   /* If not in WatchDog recovery */  
    
        BDBG_ASSERT (pParams);
    
        /* Store the start parameters inside the handle */
        hDstCh->sParams = *pParams;

        BDBG_MSG (("BRAP_DSTCH_P_Start(): hDstCh=0x%x, Index=%d, \n"
                    "hDstCh->sParams.eCapMode=%d\n"
                    "hDstCh->sParams.eBufDataMode=%d\n"
                    "hDstCh->sParams.bNotPauseWhenFull=%d\n"
    				"hDstCh->sParams.uiSrcChId=%d\n"
    				"hDstCh->sParams.bProcessIdHigh=%d\n"
    				"hDstCh->sParams.uiInputFciId=%d",
                    hDstCh, hDstCh->uiIndex,
                    hDstCh->sParams.eCapMode, 
                    hDstCh->sParams.eBufDataMode,
                    hDstCh->sParams.bNotPauseWhenFull, 
                    hDstCh->sParams.uiSrcChId,
                    hDstCh->sParams.bProcessIdHigh,
                    hDstCh->sParams.uiInputFciId));    
    
    }   /* End: If not in WatchDog recovery */
   
    hRegister = hDstCh->hRegister;
    ui32Offset = 
        hDstCh->uiIndex * 4;
/*        hDstCh->uiIndex * BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_ELEMENT_SIZE;*/
	BDBG_MSG(("ui32OffSet > %d", ui32Offset));

    if (bWdogRecovery == false)
    {   /* If not in WatchDog recovery */  
	hDstCh->ui32Offset = ui32Offset;
    }

    if ((bWdogRecovery == true) ||
        (hDstCh->eState ==  BRAP_DSTCH_P_State_eStopped))
    {
        /* Configure Destination Channel Hardware */
        /* TODO: if loopback, then make sure input I2S parameters are equal to ouput I2S params. */
        ret = BRAP_DSTCH_P_HWConfig (hDstCh); 
        if(ret != BERR_SUCCESS)
        {
            return BERR_TRACE(ret);
        }

        if(false == hDstCh->sParams.bFwToProgramCaptureRun)
        {
    		bEnableCapture = true;
        }
        else
        {
    		bEnableCapture = false;
        }

        /* Start Capture */
        if(bEnableCapture == true)
        {
        	ui32RegVal = BRAP_Read32 (hRegister, 
    	                BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_BASE + ui32Offset);

            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CTRLi, CAPTURE_RUN));
        	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CTRLi, CAPTURE_RUN, bEnableCapture));

            BRAP_Write32 (hRegister, 
             BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_BASE + ui32Offset, ui32RegVal);
        }
        
        if (bWdogRecovery == false)
        {   /* If not in WatchDog recovery */          
        /* Change Software state */
        hDstCh->eState = BRAP_DSTCH_P_State_eRunning;
        }
    }
    else
    {
        /* Start can happen only after the Stopped state */
        BDBG_WRN (("BRAP_DSTCH_P_Start(): Start can happen only after the" 
                   " Stopped state. Ignoring this call."));
    }

    BDBG_LEAVE (BRAP_DSTCH_P_Start);
    return ret;
}

/***************************************************************************
Summary:
    Stops a Destination Channel.

Description:
  

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_DSTCH_Start.
**************************************************************************/
BERR_Code BRAP_DSTCH_P_Stop ( 
    BRAP_DSTCH_P_Handle     hDstCh        /* [in] Destination Channel Handle */
)
{
    BERR_Code       ret = BERR_SUCCESS;
    uint32_t        ui32RegVal = 0;
    unsigned int    ui32Offset = 0;
    BREG_Handle     hRegister = NULL;

    BDBG_ENTER(BRAP_DSTCH_P_Stop);
    BDBG_ASSERT (hDstCh);
    BDBG_MSG (("BRAP_DSTCH_P_Stop(): hDstCh=0x%x, Index=%d ", 
                                                      hDstCh, hDstCh->uiIndex));
    
    hRegister = hDstCh->hRegister;
    ui32Offset = 
        hDstCh->uiIndex * BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_ELEMENT_SIZE;

    if ( hDstCh->eState ==  BRAP_DSTCH_P_State_eRunning)
    {
        /* Disable the Destination Channel */
        ui32RegVal = BRAP_Read32 (hRegister, 
                      BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_BASE + hDstCh->ui32Offset);

        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_ENABLE));
	    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_ENABLE, 0));

        BRAP_Write32 (hRegister, 
          BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_BASE + hDstCh->ui32Offset, ui32RegVal);

        /* Stop the Destination Channel */
        ui32RegVal = BRAP_Read32 (hRegister, 
                    BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_BASE + ui32Offset);
        
		ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_CTRLi, CAPTURE_RUN));
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_CTRLi, CAPTURE_RUN, 0));

        BRAP_Write32 (hRegister, 
          BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_BASE + ui32Offset, ui32RegVal);

        /* Change Software state */
        hDstCh->eState = BRAP_DSTCH_P_State_eStopped;
    }
    else
    {
        /* DSTCH was not running */
        BDBG_WRN (("BRAP_DSTCH_P_Stop(): This source channel was not running. "
                        "Ignoring this fnc call"));
    }

    BDBG_LEAVE (BRAP_DSTCH_P_Stop);
    return ret;
}

/* End of File */
