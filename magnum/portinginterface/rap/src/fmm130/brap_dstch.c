/***************************************************************************
*     Copyright (c) 2004-2007, Broadcom Corporation
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

                       


/* Start the DSTCH in FullDuplex mode, with input as itnernal I2S0 */
static const BRAP_DSTCH_P_Params defDstChParams =
{
    {
        BRAP_CapInputPort_eInput0,    /* sExtParams.eCapPort */        
        {
            false,                        /* sExtParams.sInputI2sParams.bLsbAtLRClk */
            true,                         /* sExtParams.sInputI2sParams.bAlignedToLRClk */
            false                         /* sExtParams.sInputI2sParams.bLRClkPolarity */
        }
    },
    BRAP_CaptureMode_eFullDuplex, /*eCapMode */
    BRAP_BufDataMode_eStereoNoninterleaved, /* eBufDataMode */    
    BRAP_CapInputType_eExtI2s,   /* eCapInputType */
    false, /* bNotPauseWhenFull */
    BRAP_RM_P_INVALID_INDEX, /* uiLtOrSnglRBufId */
    BRAP_RM_P_INVALID_INDEX, /* uiRtRBufId */
    BRAP_RM_P_INVALID_INDEX, /* uiSrcChId */
    BRAP_InputBitsPerSample_e16 /* eInputBitsPerSample */         
};

BERR_Code 
BRAP_DSTCH_P_GetDefaultParams ( 
    BRAP_DSTCH_P_Params    *pDefParams   /* Pointer to memory where default
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




BERR_Code BRAP_DSTCH_P_Open (
    BRAP_FMM_P_Handle       hFmm,            /* [in] Parent FMM handle */
    BRAP_DSTCH_P_Handle *   phDstCh,         /* [out] Pointer to Destination 
                                                Channel handle */
    unsigned int            uiDstChIndex,    /* [in] Destination channel index */
    const BRAP_DSTCH_P_Settings * pSettings  /* [in] Destination channel settings */
)
{
	BERR_Code ret = BERR_SUCCESS;
    BRAP_DSTCH_P_Handle hDstCh;

    BDBG_ENTER (BRAP_DSTCH_P_Open);
    BDBG_MSG (("BRAP_DSTCH_P_Open:"
               "hFmm=0x%x, uiDstChIndex=%d,"
               "No settings",
               hFmm, uiDstChIndex));

    /* 1. Check all input parameters to the function. Return error if
     * - FMM handle is NULL
     * - Given index exceeds maximum no. of Destination Channels
     * - Pointer to Settings structure is NULL
     * - Pointer to Destination Channel handle is NULL     */
    BDBG_ASSERT (hFmm);
    BDBG_ASSERT (phDstCh);
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   /* If not in WatchDog recovery */    
        BDBG_ASSERT (pSettings);
        BSTD_UNUSED(pSettings);
    }

    if (  uiDstChIndex > (BRAP_RM_P_MAX_DST_CHANNELS -1))
    {
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }       

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   /* If not in WatchDog recovery */  
        
        if (hFmm->hDstCh[uiDstChIndex] != NULL )
        {
            BDBG_ERR (("BRAP_DSTCH_P_Open: Destination Channel %d is already open", uiDstChIndex));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
        }
    
        /* 2. Allocate memory for the Destination Channel handle. Fill in parameters in the Destination Channel
        * handle. */
    
        /* Allocate Dst Channel handle */
        hDstCh = (BRAP_DSTCH_P_Handle) BKNI_Malloc (sizeof(BRAP_DSTCH_P_Object));
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
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
        hDstCh->ui32Offset = (BCHP_AUD_FMM_BF_CTRL_DESTCH_1_CFG  
                       - BCHP_AUD_FMM_BF_CTRL_DESTCH_0_CFG) 
                       * uiDstChIndex;
#else
		/* Only one Destination Channel is present in 7401, 7118 */
		hDstCh->ui32Offset = 0;
#endif
    }   /* End: If not in WatchDog recovery */
    else
    {
        hDstCh = *phDstCh;
    }
    hDstCh->eState = BRAP_DSTCH_P_State_eStopped;  

    /* Fill up the Destination Channel Settings Structure */
    /* No settings at present */
    
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag(hFmm) == false)
    {   /* If not in WatchDog recovery */ 

        /* Store Destination Channel handle inside the FMM handle */
        hFmm->hDstCh[uiDstChIndex] = hDstCh;
    
        *phDstCh = hDstCh;
    }   /* End: If not in WatchDog recovery */        
    
    BDBG_MSG(("BRAP_DSTCH_P_Open: handle=0x%x", hDstCh)); 
    BDBG_LEAVE (BRAP_DSTCH_P_Open);
    return ret;
}

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

#ifndef BCHP_7411_VER
static void BRAP_DSTCH_P_ProgramBufMap(BREG_Handle hRegister, 
						   unsigned int uiChIndx, 
						   unsigned int uiLtRBufId,
						   unsigned int uiRtRBufId)
{
    uint32_t  ui32RegVal;
  
	ui32RegVal = BRAP_Read32(hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_1);

	switch (uiChIndx)
	{		
		case 0:
			if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_1, CAPTURE_BUF_00));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_1, 
									CAPTURE_BUF_00, uiLtRBufId));
			}

			if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_1, CAPTURE_BUF_01));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_1, 
									CAPTURE_BUF_01, uiRtRBufId));
			}	
			break;

		case 1:
			if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_1, CAPTURE_BUF_10));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_1, 
									CAPTURE_BUF_10, uiLtRBufId));
			}

			if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
		        ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_BUFFER_MAP_1, CAPTURE_BUF_11));
		        ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_BUFFER_MAP_1, 
									CAPTURE_BUF_11, uiRtRBufId));
			}	
			break;

		default:
			BDBG_ASSERT(0);
	}
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_BUFFER_MAP_1, ui32RegVal);			
}
#endif

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
    BRAP_DSTCH_P_Handle hDstCh 		/* [in] Destination Channel handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal;
    unsigned int ui32Offset;
    BREG_Handle hRegister;
    BRAP_DSTCH_P_Params sParams;

    BDBG_ENTER (BRAP_DSTCH_P_HWConfig);
    BDBG_ASSERT (hDstCh);

    BDBG_MSG (("BRAP_DSTCH_P_HWConfig(): hDstCh=0x%x, Index=%d, offset=%d \n", 
        hDstCh, hDstCh->uiIndex, hDstCh->ui32Offset));
  
    hRegister = hDstCh->hRegister;
    ui32Offset = hDstCh->ui32Offset;
    sParams = hDstCh->sParams;    
    
    ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_DESTCH_0_CFG + ui32Offset);
    
    ui32RegVal &= ~( (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, I2S_BITS_PER_SAMPLE)) 
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, I2S_DATA_JUSTIFICATION)) 
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, I2S_DATA_ALIGNMENT)) 
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, I2S_SCLK_POLARITY)) 
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, I2S_LRCK_POLARITY)) 
                     | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, I2S_MAI_SEL)) 
					 | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, INPUT_PORT_SEL))
					 | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, NOT_PAUSE_WHEN_FULL))
					 | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, SOURCE_FIFO_ID))
					 | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, INPUT_FRM_SOURCEFIFO))
					 | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, CAPTURE_TO_SOURCEFIFO))
					 | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, PLAY_FROM_CAPTURE))
					 | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, BUFFER_PAIR_ENABLE))
			 		 | (BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, CAPTURE_ENABLE))
		);

	switch(sParams.eCapInputType)
	{
		case BRAP_CapInputType_eExtI2s:
		case BRAP_CapInputType_eIntI2s0:
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
		case BRAP_CapInputType_eIntI2s1:
		case BRAP_CapInputType_eIntI2s2:
#endif
		case BRAP_CapInputType_eIntFlex:
			
			/* I2S input configuration required */

			/* Bits per sample */
			switch(sParams.eInputBitsPerSample)
			{
				case BRAP_InputBitsPerSample_e16:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
											I2S_BITS_PER_SAMPLE, Bitwidth16));
					break;
				case BRAP_InputBitsPerSample_e18:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
											I2S_BITS_PER_SAMPLE, Bitwidth18));
					break;
				case BRAP_InputBitsPerSample_e20:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
											I2S_BITS_PER_SAMPLE, Bitwidth20));
					break;
				case BRAP_InputBitsPerSample_e24:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
											I2S_BITS_PER_SAMPLE, Bitwidth24));
					break;
				case BRAP_InputBitsPerSample_e32:
					ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
											I2S_BITS_PER_SAMPLE, Bitwidth32));
					break;
				default:
					BDBG_ERR (("BRAP_DSTCH_P_HWConfig: "
							   "I2S Input Bits/Sample %d is not supported", 
							   sParams.eInputBitsPerSample));
					return BERR_TRACE (BERR_NOT_SUPPORTED);
			}

			/* Data Justification */
			if (sParams.sExtParams.sInputI2sParams.bLsbAtLRClk == true)
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
												I2S_DATA_JUSTIFICATION, LSB));
			}
			else 
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
												I2S_DATA_JUSTIFICATION, MSB));
			}

			/* Data Alignment */
			if (sParams.sExtParams.sInputI2sParams.bAlignedToLRClk == false)
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
												I2S_DATA_ALIGNMENT, Delayed));
			}
			else 
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
												I2S_DATA_ALIGNMENT, Aligned));
			}

			/* S Clock Polarity to set to Falling_aligned_with_sdata */
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
							I2S_SCLK_POLARITY, Falling_aligned_with_sdata));
			
			/* LR Clock Polarity */
			if (sParams.sExtParams.sInputI2sParams.bLRClkPolarity == true)
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
												I2S_LRCK_POLARITY, High_for_left));
			}
			else 
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
												I2S_LRCK_POLARITY, Low_for_left));
			}

			/* Select I2S as input type */
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, 
											I2S_MAI_SEL, 0));

			break;
		
		/* TODO: Put the case for BTSC, when supported */

		default:
			/* Input configuration not required */
			break;
	}

	/* Configure Buffer Pair Enable */
  	switch(sParams.eBufDataMode)
	{
		case BRAP_BufDataMode_eMono:
		case BRAP_BufDataMode_eStereoInterleaved:
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, BUFFER_PAIR_ENABLE, 0));
			break;
		case BRAP_BufDataMode_eStereoNoninterleaved:
            ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, BUFFER_PAIR_ENABLE, 1));
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
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, NOT_PAUSE_WHEN_FULL, 1));
	}
	else
	{
    	/* Set 'Not Pause On Full' = 0 */
	    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, NOT_PAUSE_WHEN_FULL, 0));
	}
	
	/* Configure depending upon the capture mode */
	switch(sParams.eCapMode)
	{
		case BRAP_CaptureMode_eCaptureOnly:
			/* Capture Data to Destination FIFO and Don't play from Capture Ring Buffers */
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, CAPTURE_TO_SOURCEFIFO, 0));
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, PLAY_FROM_CAPTURE, 0));
			break;
		case BRAP_CaptureMode_eFullDuplex:
			if  (sParams.uiSrcChId > BRAP_RM_P_MAX_SRC_CHANNELS) 
			{
				return BERR_TRACE (BERR_INVALID_PARAMETER);
			}
			/* Capture Data to Destination FIFO and Play from Capture Ring Buffers */
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, CAPTURE_TO_SOURCEFIFO, 0));
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, SOURCE_FIFO_ID, sParams.uiSrcChId));
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, PLAY_FROM_CAPTURE, 1));
			break;
		case BRAP_CaptureMode_eByPass:
			if  (sParams.uiSrcChId > BRAP_RM_P_MAX_SRC_CHANNELS) 
			{
				return BERR_TRACE (BERR_INVALID_PARAMETER);
			}
			/* Capture Data to Source FIFO */
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, CAPTURE_TO_SOURCEFIFO, 1));
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, SOURCE_FIFO_ID, sParams.uiSrcChId));
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, PLAY_FROM_CAPTURE, 0));
			break;
		default:
			BDBG_ERR(("BRAP_DSTCH_P_HWConfig(): Invalid Capture Mode(%d)",
						sParams.eCapMode));
			BDBG_ASSERT(0);
			break;
	}

	/* Program the capture input port */
	switch(hDstCh->sParams.sExtParams.eCapPort)
	{
		case BRAP_CapInputPort_eInput0:
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, INPUT_PORT_SEL, 0));
			break;
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
		case BRAP_CapInputPort_eInput1:
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, INPUT_PORT_SEL, 1));
			break;
#endif
		default:
			BDBG_ERR(("BRAP_DSTCH_P_HWConfig(): Input port(%d) not supported",
					   hDstCh->sParams.sExtParams.eCapPort));
			return BERR_TRACE (BERR_NOT_SUPPORTED);
	}

	/* Input Data is coming from Capture Input, not from DRAM */
	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, INPUT_FRM_SOURCEFIFO, 0));

	/* Enable Capture */
	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CFG, CAPTURE_ENABLE, 1));

    /* Leave all other field settings as Reset values */
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_DESTCH_0_CFG + ui32Offset, ui32RegVal);

    BDBG_LEAVE (BRAP_DESTCH_P_HWConfig);
    return ret;
}

BERR_Code BRAP_DSTCH_P_Start ( 
	BRAP_DSTCH_P_Handle     hDstCh,        /* [in] Destination Channel Handle */
    const BRAP_DSTCH_P_Params *   pParams  /* [in] Pointer to start
                                              parameters for this Destination 
                                              Channel */ 
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal;
    uint32_t ui32Offset;
    BREG_Handle hRegister;

    BDBG_ENTER (BRAP_DSTCH_P_Start);
    BDBG_ASSERT (hDstCh);

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hDstCh->hFmm) == false)
    {   /* If not in WatchDog recovery */  
    
        BDBG_ASSERT (pParams);
    
        /* Store the start parameters inside the handle */
        hDstCh->sParams = *pParams;

        BDBG_MSG (("BRAP_DSTCH_P_Start(): hDstCh=0x%x, Index=%d, \n"
				"hDstCh->sParams.sExtParams.bLsbAtLRClk=%d\n"
				"hDstCh->sParams.sExtParams.bAlignedToLRClk=%d\n"
                "hDstCh->sParams.sExtParams.bLRClkPolarity=%d\n"
                "hDstCh->sParams.sExtParams.eCapPort =%d\n"
                "hDstCh->sParams.eInputBitsPerSample=%d\n"
                "hDstCh->sParams.eCapMode=%d\n"
                "hDstCh->sParams.eBufDataMode=%d\n"
                "hDstCh->sParams.eCapInputType=%d\n"
                "hDstCh->sParams.bNotPauseWhenFull=%d\n"
                "hDstCh->sParams.uiLtOrSnglRBufId=%d\n"
                "hDstCh->sParams.uiRtRBufId=%d\n"
				"hDstCh->sParams.uiSrcChId=%d\n",
                hDstCh, hDstCh->uiIndex,
				hDstCh->sParams.sExtParams.sInputI2sParams.bLsbAtLRClk, 
				hDstCh->sParams.sExtParams.sInputI2sParams.bAlignedToLRClk, 
				hDstCh->sParams.sExtParams.sInputI2sParams.bLRClkPolarity, 
				hDstCh->sParams.sExtParams.eCapPort, 				
				hDstCh->sParams.eInputBitsPerSample, 
                hDstCh->sParams.eCapMode, 
                hDstCh->sParams.eBufDataMode,
                hDstCh->sParams.eCapInputType, 
                hDstCh->sParams.bNotPauseWhenFull, 
                hDstCh->sParams.uiLtOrSnglRBufId,
                hDstCh->sParams.uiRtRBufId,
                hDstCh->sParams.uiSrcChId));    
    
    }   /* End: If not in WatchDog recovery */
   
    hRegister = hDstCh->hRegister;
    ui32Offset = hDstCh->ui32Offset;

    if (hDstCh->eState ==  BRAP_DSTCH_P_State_eStopped)
    {
        /* Configure Destination Channel Hardware */
        /* TODO: if loopback, then make sure input I2S parameters are equal to ouput I2S params. */
        BRAP_DSTCH_P_HWConfig (hDstCh); 
#ifndef BCHP_7411_VER
 		BRAP_DSTCH_P_ProgramBufMap(hRegister, hDstCh->uiIndex, 
                 hDstCh->sParams.uiLtOrSnglRBufId, hDstCh->sParams.uiRtRBufId);
#endif
		/* Start Capture */
		ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_DESTCH_0_CTRL + ui32Offset);
		ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CTRL, CAPTURE_RUN));
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_DESTCH_0_CTRL, CAPTURE_RUN, 1));
		BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_DESTCH_0_CTRL + ui32Offset, ui32RegVal);

        /* Change Software state */
        hDstCh->eState = BRAP_DSTCH_P_State_eRunning;
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



BERR_Code BRAP_DSTCH_P_Stop ( 
    BRAP_DSTCH_P_Handle     hDstCh        /* [in] Destination Channel Handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal;
    unsigned int ui32Offset;
    BREG_Handle hRegister;

    BDBG_ENTER(BRAP_DSTCH_P_Stop);
    BDBG_ASSERT (hDstCh);
    BDBG_MSG (("BRAP_DSTCH_P_Stop(): hDstCh=0x%x, Index=%d ", hDstCh, hDstCh->uiIndex));
    
    hRegister = hDstCh->hRegister;
    ui32Offset = hDstCh->ui32Offset;

    if ( hDstCh->eState ==  BRAP_DSTCH_P_State_eRunning)
    {
        /* Disable the Destination Channel */
        ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_DESTCH_0_CFG + ui32Offset);
		ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CFG, CAPTURE_ENABLE));        
		BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_DESTCH_0_CFG + ui32Offset, ui32RegVal);

        /* Stop the Destination Channel */
        ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_BF_CTRL_DESTCH_0_CTRL + ui32Offset);
		ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_DESTCH_0_CTRL, CAPTURE_RUN));
		BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_CTRL_DESTCH_0_CTRL + ui32Offset, ui32RegVal);

        /* Change Software state */
        hDstCh->eState = BRAP_DSTCH_P_State_eStopped;
    }
    else
    {
        /* DSTCH was not running */
        BDBG_WRN (("BRAP_DSTCH_P_Stop(): This source channel was not running. Ignoring this fnc call"));
    }

    BDBG_LEAVE (BRAP_DSTCH_P_Stop);
    return ret;
}

/* End of File */
