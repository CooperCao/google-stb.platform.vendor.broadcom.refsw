/***************************************************************************
*     Copyright (c) 2004-2013, Broadcom Corporation
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
*   Module name: INT
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "brap.h"
#include "brap_priv.h"


BDBG_MODULE (rap_int);


#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
#define BRAP_P_INCLUDE_MPEG_LAYER_CHANGE 1  
#else
#define BRAP_P_INCLUDE_MPEG_LAYER_CHANGE 0  
#endif


/* PR 22374: 
There's a HW bug in the RAVE for ALL Raptor chips (7401,7118,7403,7400,7440, 3563). 
As a workaround, Rave needs to program a certain XPT register differently for 
MPEG L1 streams, vs L2 and L3. But RAVE has no way of knowing before hand 
whether its L1, L2 or L3. FW is the first one to know.
So to handle this the following will be done
- FW will generate an interrupt. 
- On getting that interrupt, PI will program RAVE register from ISR context. 
- Rave PI will access those registers only through critical section.

Note: Since this is a hack, we are not exposing this interrupt in the enum BRAP_Interrupt 

Note: as of 04/04/2007, this code is being added only for 7401.
Once the 7400,7440, 3563 RDB and FW changes are ready, this should be 
enabled for those chips also. 
*/

#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
#include "bchp_xpt_rave.h"
#define BRAP_P_INCLUDE_MPEG_RAVE_WORKAROUND 1 
#define BRAP_P_XPT_COMP2_EXCLUSION 0x0000FFFF 
#else
#define BRAP_P_INCLUDE_MPEG_RAVE_WORKAROUND 0  
#endif




#ifndef BCHP_7411_VER /* For chips other than 7411 */
static const  BINT_Id ui32DSPInterruptId[] =
{
	  BCHP_INT_ID_ESR_SO0
	, BCHP_INT_ID_ESR_SO1
	, BCHP_INT_ID_ESR_SO2
#if ( BCHP_CHIP == 7400 )
	, BCHP_INT_ID_ESR_SO3
	, BCHP_INT_ID_ESR_SO4
	, BCHP_INT_ID_ESR_SO5	
#endif /* ( BCHP_CHIP == 7400 ) */
};
#endif
static void BRAP_P_StreaminfoChange_isr(void *pParm1, int dummy);
static void BRAP_P_FirstPtsReady_isr(void *pParm1, int dummy);
static void BRAP_P_PtsError_isr(void *pParm1, int dummy);
static void BRAP_P_SampleRateChange_isr(void *pParm1, int dummy);
static void BRAP_P_BitRateChange_isr(void *pParm1, int dummy);
static void BRAP_P_ModeChange_isr(void *pParm1, int dummy);
static void BRAP_P_CrcError_isr(void *pParm1, int dummy);
static void BRAP_P_TsmLog_isr(void *pParm1, int dummy);
static void BRAP_P_DecoderLock_isr(void *pParm1, int dummy);
static void BRAP_P_DecoderUnlock_isr(void *pParm1, int dummy);
#if BCHP_7411_VER > BCHP_VER_C0
static void BRAP_P_StartPtsReached_isr(void *pParm1, int dummy);
static void BRAP_P_StopPtsReached_isr(void *pParm1, int dummy);
#endif
#ifdef BCHP_7411_VER /* Only for the 7411 */
static void BRAP_P_Watchdog_isr(void *pParm1);
#else
static void BRAP_P_Watchdog_isr(void *pParm1, int iParm2 );
#endif
#if (BRAP_P_INCLUDE_MPEG_LAYER_CHANGE == 1)
static void BRAP_P_MpegLayerChange_isr(void *pParm1, int dummy);
#endif
#if (BRAP_P_INCLUDE_MPEG_RAVE_WORKAROUND == 1)
static void BRAP_P_MpegNotLayer1_isr(void *pParm1, int dummy);
#endif

#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)|| (BCHP_CHIP == 7403))
static void 
BRAP_P_CRC_isr(
void *pParm1, 
int iParm2
);
#endif

static void BRAP_P_AstmTsmPass_isr(void *pParm1, int iParam2);
#if (BCHP_CHIP == 7400) 
static void BRAP_P_RampEnable_isr(void *pParm1, int iParam2);
static void BRAP_P_CDBITBUnderflow_isr(void *pParm1, int iParam2);
static void BRAP_P_CDBReady_isr(void *pParm1, int iParam2);

#endif

/* During intialisation ie in BRAP_Open(), Clear all raptor interrupts & mask (disable) them */
BERR_Code BRAP_P_ClearInterrupts (
        BREG_Handle hRegister   /* [in] The register handle */
)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_P_ClearInterrupts);

    BDBG_ASSERT (hRegister);            
   
    /* Mask all interrupts. Write 1 to bits to be masked and 0 to reserved bits */

    
   	/* Mask all DSP interrupts */
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_INTH0_R5F_MASK_SET, 
                 ~(BCHP_MASK (AUD_DSP_INTH0_R5F_MASK_SET, reserved0)));
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_INTH0_PCI_MASK_SET, 
                 ~(BCHP_MASK (AUD_DSP_INTH0_PCI_MASK_SET, reserved0)));    
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_BE0_MASK_SET, 
                 ~(BCHP_MASK (AUD_DSP_ESR_BE0_MASK_SET, reserved0)));    
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_PI0_MASK_SET, 
                 ~(BCHP_MASK (AUD_DSP_ESR_PI0_MASK_SET, reserved0)));   

   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_SET, 0xffffffff );
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO10_MASK_SET, 0xffffffff );
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO20_MASK_SET, 0xffffffff );
#if ( BCHP_CHIP == 7400 )
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO30_MASK_SET, 0xffffffff );
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO40_MASK_SET, 0xffffffff );
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO50_MASK_SET, 0xffffffff );
#endif


    /* Mask all FMM interrupts. */
    BDBG_MSG(("Masking all FMM interrupts"));
    BRAP_Write32 (hRegister, BCHP_AIO_INTH_R5F_MASK_SET, 
                 ~(BCHP_MASK (AIO_INTH_R5F_MASK_SET, reserved0)));   

    BRAP_Write32 (hRegister, BCHP_AIO_INTH_PCI_MASK_SET, 
                 ~(BCHP_MASK (AIO_INTH_PCI_MASK_SET, reserved0)));   
    
#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_ESR0_MASK_SET, 
                 ~(BCHP_MASK (AUD_FMM_BF_ESR0_MASK_SET, reserved0)));   

    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_ESR1_MASK_SET, 
                 ~(BCHP_MASK (AUD_FMM_BF_ESR0_MASK_SET, reserved0)));   
#else
    /* 7400 has no reserved bits */
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_ESR0_MASK_SET, 0xffffffff );  
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_ESR1_MASK_SET, 0xffffffff );
#endif
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_DP_ESR0_MASK_SET, 
                 ~(BCHP_MASK (AUD_FMM_DP_ESR0_MASK_SET, reserved0)));   

#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_DP_ESR1_MASK_SET, 
                 ~(BCHP_MASK (AUD_FMM_DP_ESR0_MASK_SET, reserved0)));   
#endif /* defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 ) */

    BRAP_Write32 (hRegister, BCHP_AUD_FMM_MS_ESR_MASK_SET, 
                 ~(BCHP_MASK (AUD_FMM_MS_ESR_MASK_SET, reserved0)));   
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_OP_ESR_MASK_SET, 
                 ~(BCHP_MASK (AUD_FMM_OP_ESR_MASK_SET, reserved0)));   

    /* Mask all Misc interrupts */
    BRAP_Write32 (hRegister, BCHP_AIO_INTD0_MASK_SET, 
                 ~(BCHP_MASK (AIO_INTD0_MASK_SET, reserved0)));   


	/* Clear all interrupts: 1 to clear intrpt, 0 for reserved bits */

	/* Clear all pending DSP interrupts */
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_INTH0_R5F_CLEAR, 
                     ~(BCHP_MASK (AUD_DSP_INTH0_R5F_CLEAR, reserved0)));   
    
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_BE0_INT_CLEAR, 
                     ~(BCHP_MASK (AUD_DSP_ESR_BE0_INT_CLEAR, reserved0)));   
    
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_PI0_INT_CLEAR, 
                                   ~(BCHP_MASK (AUD_DSP_ESR_PI0_INT_CLEAR, reserved0)));   
    
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO00_INT_CLEAR, 0xffffffff);
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO10_INT_CLEAR, 0xffffffff);
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO20_INT_CLEAR, 0xffffffff);
#if ( BCHP_CHIP == 7400 )
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO30_INT_CLEAR, 0xffffffff);
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO40_INT_CLEAR, 0xffffffff);
   	BRAP_Write32(hRegister, BCHP_AUD_DSP_ESR_SO50_INT_CLEAR, 0xffffffff);
#endif
   
    /* Clear any pending FMM interrupts */
    BDBG_MSG(("Clearing all FMM interrupts"));
    BRAP_Write32 (hRegister, BCHP_AIO_INTH_R5F_CLEAR, 
                                ~(BCHP_MASK (AIO_INTH_R5F_CLEAR, reserved0)));   
    BRAP_Write32 (hRegister, BCHP_AIO_INTH_PCI_CLEAR, 
                                ~(BCHP_MASK (AIO_INTH_PCI_CLEAR, reserved0)));   

#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)		
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_ESR0_STATUS_CLEAR, 
                                ~(BCHP_MASK (AUD_FMM_BF_ESR0_STATUS_CLEAR, reserved0)));   
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_ESR1_STATUS_CLEAR, 
                                ~(BCHP_MASK (AUD_FMM_BF_ESR0_STATUS_CLEAR, reserved0)));   
#else 
    /* 7400 has no reserved bits */
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_ESR0_STATUS_CLEAR, 0xffffffff);
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_ESR1_STATUS_CLEAR, 0xffffffff);
#endif

    BRAP_Write32 (hRegister, BCHP_AUD_FMM_DP_ESR0_STATUS_CLEAR, 
                                ~(BCHP_MASK (AUD_FMM_DP_ESR0_STATUS_CLEAR, reserved0)));   
    
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_DP_ESR1_STATUS_CLEAR,
                                ~(BCHP_MASK (AUD_FMM_DP_ESR0_STATUS_CLEAR, reserved0)));   
#endif /* defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 ) */

    BRAP_Write32 (hRegister, BCHP_AUD_FMM_MS_ESR_STATUS_CLEAR, 
                                ~(BCHP_MASK (AUD_FMM_MS_ESR_STATUS_CLEAR, reserved0)));   
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_OP_ESR_STATUS_CLEAR, 
                                ~(BCHP_MASK (AUD_FMM_OP_ESR_STATUS_CLEAR, reserved0)));   
        
        
    /* Clear all miscellaneous interrupts */   
    BRAP_Write32 (hRegister, BCHP_AIO_INTD0_INT_CLEAR, 
                                ~(BCHP_MASK (AIO_INTD0_INT_CLEAR, reserved0)));  

    BDBG_LEAVE (BRAP_P_ClearInterrupts);
    return ret;
}

#ifdef BCHP_7411_VER /* Only for the 7411 */
BERR_Code BRAP_P_DeviceLevelInterruptInstall (
	BRAP_Handle		hRap
)
{
	BERR_Code ret = BERR_SUCCESS;

	BDBG_ASSERT(hRap);
   	ret = BINT_CreateCallback(
               	&hRap->hCallback,
                hRap->hInt,
                BCHP_INT_ID_7411_AUDIO_IRQ,
                BRAP_P_TopLevelDevice_isr,
                (void*)hRap,
                0 /* Not used */);
	if (ret!=BERR_SUCCESS)
		return BERR_TRACE(ret);

   	ret = BINT_EnableCallback(hRap->hCallback);
	if (ret!=BERR_SUCCESS)
		return BERR_TRACE(ret);
	
	return ret;
}

BERR_Code BRAP_P_DeviceLevelInterruptUnInstall (
	BRAP_Handle		hRap
)
{
	BERR_Code ret = BERR_SUCCESS;
	
	BDBG_ASSERT(hRap);
    if(hRap->hCallback)
    {
        ret = BINT_DisableCallback(hRap->hCallback);
	    if (ret!=BERR_SUCCESS)
		    return BERR_TRACE(ret);
        ret = BINT_DestroyCallback(hRap->hCallback);
	    if (ret!=BERR_SUCCESS)
		    return BERR_TRACE(ret);
        BDBG_MSG(("Callback destroyed."));
    }
	return ret;
}

void BRAP_P_TopLevelDevice_isr (
		void *pParm1,	/* [in] Raptor handle */
		int iParm2		/* [in] Not used */
)
{
	uint32_t ui32IntStatus;
	BRAP_Handle hRap = (BRAP_Handle) pParm1;

	BSTD_UNUSED(iParm2);
	/* Read the interrupt status register */
	ui32IntStatus = BRAP_Read32_isr(hRap->hRegister, BCHP_AUD_DSP_INTH0_R5F_STATUS);
	
	/* Clear the interrupts to be serviced in this function 
	 * Currently we service only watchdog interrupt here */
	if (ui32IntStatus & BCHP_MASK(AUD_DSP_INTH0_R5F_STATUS, WDOG_TIMER_ATTN)) {
		BRAP_Write32_isr(hRap->hRegister, BCHP_AUD_DSP_INTH0_R5F_STATUS,
			 BCHP_MASK(AUD_DSP_INTH0_R5F_STATUS, WDOG_TIMER_ATTN));
	/* Call the handler */
	BRAP_P_Watchdog_isr(pParm1);
	}
}
#else
BERR_Code BRAP_P_DeviceLevelInterruptInstall (
	BRAP_Handle		hRap
)
{
	BERR_Code ret = BERR_SUCCESS;

	BDBG_ASSERT(hRap);
	ret = BINT_CreateCallback(
				&hRap->hCallback,
				hRap->hInt,
				BCHP_INT_ID_WDOG_TIMER_ATTN,
				BRAP_P_Watchdog_isr,
				(void*)hRap,
				0 /* Not used */);
	
	if (ret!=BERR_SUCCESS)
		return BERR_TRACE(ret);

	return ret;
}

BERR_Code BRAP_P_DeviceLevelInterruptUnInstall (
	BRAP_Handle		hRap
)
{
	BERR_Code ret = BERR_SUCCESS;
	
	BDBG_ASSERT(hRap);

    if(hRap->hCallback)
    {
        ret = BINT_DisableCallback(hRap->hCallback);
    	if (ret!=BERR_SUCCESS)
    		return BERR_TRACE(ret);
        ret = BINT_DestroyCallback(hRap->hCallback);
    	if (ret!=BERR_SUCCESS)
    		return BERR_TRACE(ret);
        BDBG_MSG(("Callback destroyed."));
    }
	return ret;
}
#endif




#ifdef BCHP_7411_VER /* Only for the 7411 */
BERR_Code BRAP_P_InterruptInstall (
	BRAP_ChannelHandle 		hRapCh		/* [in] Raptor Channel handle */
)
{

	uint32_t ui32RegVal=0;
	BERR_Code ret = BERR_SUCCESS;
	BREG_Handle hRegister;

	BDBG_ASSERT (hRapCh);

	BDBG_ENTER (BRAP_P_InterruptInstall);    

#ifndef EMULATION /* dont handled interrupts in emulation */
	hRegister = hRapCh->hRegister;

	/* Unmask interrupts to be handled for respective modules */
	BDBG_MSG(("Unmasking FMM interrupts (for Host)"));
   
	if (BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap) == false) 
	{
		hRapCh->ui32FmmIntMask = (BCHP_FIELD_DATA (AIO_INTH_R5F_MASK_CLEAR, FMM_BF, 1));
		ui32RegVal = 0;

		/* Save masked int info */
		hRapCh->ui32FmmBFIntMask |= ui32RegVal;           
	}

	BRAP_Write32 (hRegister, BCHP_AIO_INTH_R5F_MASK_CLEAR, hRapCh->ui32FmmIntMask);
	BRAP_Write32 (hRegister, BCHP_AUD_FMM_BF_ESR1_MASK_CLEAR, hRapCh->ui32FmmBFIntMask);
	BDBG_MSG (("hRapCh->ui32FmmIntMask = 0x%x", hRapCh->ui32FmmIntMask));
	BDBG_MSG (("hRapCh->ui32FmmIntMask = 0x%x", hRapCh->ui32FmmBFIntMask));                


	if (hRapCh->eChannelType ==   BRAP_P_ChannelType_eDecode)
	{
		BDBG_MSG(("Unmasking  DSP interrupts (for Host)"));
		if (!BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap)) 
		{
			hRapCh->ui32DspIntMask = (BCHP_FIELD_DATA (AUD_DSP_INTH0_R5F_MASK_CLEAR, ESR_SO0, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_INTH0_R5F_MASK_CLEAR, ESR_SO1, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_INTH0_R5F_MASK_CLEAR, ESR_SO2, 1));
		}			   
		BRAP_Write32 (hRegister, BCHP_AUD_DSP_INTH0_R5F_MASK_CLEAR, hRapCh->ui32DspIntMask);


		/* Enable following interrupt bits for all 3 DSP contexts 
		* PTS_RCV - FirstPtsReady
		* PTS_DISCARD - PtsError
		* SR_CHG - StreamInfo
		* TSM_LOG - Tsm Log
		* LOCK - Decoder lock
		* Unlock - Decoder unlock
		*/
		ui32RegVal = 0;
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, SR_CHG, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, PTS_RCV, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, PTS_DISCARD, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, TSM_LOG, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, LOCK, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, UNLOCK, 1))
#if BCHP_7411_VER > BCHP_VER_C0
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, START_PTS_RCH, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, END_PTS_RCH, 1))
#endif
				;
		BRAP_Write32 (hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR, ui32RegVal );
		BRAP_Write32 (hRegister, BCHP_AUD_DSP_ESR_SO10_MASK_CLEAR, ui32RegVal );
		BRAP_Write32 (hRegister, BCHP_AUD_DSP_ESR_SO20_MASK_CLEAR, ui32RegVal );
		BDBG_MSG (("hRapCh->ui32DspIntMask = 0x%x", hRapCh->ui32DspIntMask));    
	}
  
	/* Install the interrupt callback for the top level 7411 L2 Audio interrupt */
	if (!BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap))
	{
	    	ret = BINT_CreateCallback(
	                	&hRapCh->hCallback,
	                	hRapCh->hInt,
	                	BCHP_INT_ID_7411_AUDIO_IRQ,
	                	BRAP_P_TopLevel_isr,
	                	(void*)hRapCh,
	                	0 /* Not used */);
		if (ret!=BERR_SUCCESS)
			return BERR_TRACE(ret);
	    	ret = BINT_EnableCallback(hRapCh->hCallback);
		if (ret!=BERR_SUCCESS)
			return BERR_TRACE(ret);
	}
#endif /* ifndef EMULATION */

	BDBG_LEAVE(BRAP_P_InterruptInstall);
	return ret;    
 
}

BERR_Code BRAP_P_InterruptUnInstall (
	BRAP_ChannelHandle 		hRapCh		/* [in] Raptor Channel handle */
)
{

	uint32_t ui32RegVal=0;
	BERR_Code ret = BERR_SUCCESS;
	BREG_Handle hRegister;

	BDBG_ASSERT (hRapCh);            

	BDBG_ENTER (BRAP_P_InterruptUnInstall);    

#ifndef EMULATION /* dont handle interrupts in emulation */
	hRegister = hRapCh->hRegister;

	/* mask interrupts handled for respective modules */
	BDBG_MSG(("Masking FMM interrupts (for Host)"));
	ui32RegVal = BRAP_Read32 (hRegister, BCHP_AIO_INTH_R5F_MASK_STATUS);
	ui32RegVal |= hRapCh->ui32FmmIntMask;
	BRAP_Write32 (hRegister, BCHP_AIO_INTH_R5F_MASK_SET, ui32RegVal);
	BDBG_MSG (("BCHP_AIO_INTH_R5F_MASK_SET = 0x%x", ui32RegVal));

	BDBG_MSG(("Masking DSP interrupts (for Host)"));
	ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_DSP_INTH0_R5F_MASK_STATUS);
	BDBG_MSG (("BCHP_AUD_DSP_INTH0_R5F_MASK_STATUS = 0x%x", ui32RegVal));
	ui32RegVal |= hRapCh->ui32DspIntMask;
	BRAP_Write32 (hRegister, BCHP_AUD_DSP_INTH0_R5F_MASK_SET, ui32RegVal);
	BDBG_MSG (("BCHP_AUD_DSP_INTH0_R5F_MASK_SET = 0x%x", ui32RegVal));

	/* TODO: Do we need to mask all lower level interrupt?? */

	/* UnInstall the interrupt callback for the top level 7411 L2 Audio interrupt */    
	if(hRapCh->hCallback)
	{
		ret = BINT_DisableCallback(hRapCh->hCallback);
		if (ret!=BERR_SUCCESS)
			return BERR_TRACE(ret);

		ret = BINT_DestroyCallback(hRapCh->hCallback);
		if (ret!=BERR_SUCCESS)
			return BERR_TRACE(ret);

		BDBG_MSG(("Callback destroyed."));
	}
#endif /* ifndef EMULATION */

	BDBG_LEAVE(BRAP_P_InterruptUnInstall);
	return ret;    
 
}
#else
/* for chips other than 7411 */
BERR_Code BRAP_P_InterruptInstall (
	BRAP_ChannelHandle 		hRapCh		/* [in] Raptor Channel handle */
)
{

	uint32_t ui32RegVal=0;
	uint32_t ui32Offset=0;
	BERR_Code ret = BERR_SUCCESS;
	BREG_Handle hRegister;

	BDBG_ASSERT (hRapCh);            

	BDBG_ENTER (BRAP_P_InterruptInstall);    

#ifndef EMULATION /* dont handled interrupts in emulation */
	hRegister = hRapCh->hRegister;


	/* Get correct register for this DSP context */
	switch (hRapCh->sRsrcGrnt.uiDspContextId)
	{
		case 0:
			ui32Offset = 0;
			break;
		case 1:
			ui32Offset = BCHP_AUD_DSP_ESR_SO10_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
			break;
		case 2:
			ui32Offset = BCHP_AUD_DSP_ESR_SO20_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
		break;
#if ( BCHP_CHIP == 7400 )
		case 3:
			ui32Offset = BCHP_AUD_DSP_ESR_SO30_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
			break;
		case 4:
			ui32Offset = BCHP_AUD_DSP_ESR_SO40_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
			break;
		case 5:
			ui32Offset = BCHP_AUD_DSP_ESR_SO50_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
			break;
#endif
	}

	/* Enable following interrupt bits for all 3 DSP contexts 
	* PTS_RCV - FirstPtsReady
	* PTS_DISCARD - PtsError
	* SR_CHG - StreamInfo
	* TSM_LOG - Tsm Log
	* MPEG_LAYER_CHANGE - MPEG Layer change
	* LOCK - Decoder lock
	* Unlock - Decoder unlock
	*/
	ui32RegVal = 0;
	ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, SR_CHG, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, PTS_RCV, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, PTS_DISCARD, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, TSM_LOG, 1))
#if (BRAP_P_INCLUDE_MPEG_LAYER_CHANGE == 1)
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, MPEG_LAYER_CHANGE, 1))
#endif				
#if (BRAP_P_INCLUDE_MPEG_RAVE_WORKAROUND == 1)
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, MPEG_NOT_LAYER1, 1))
#endif	
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, LOCK, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, UNLOCK, 1))
#if(BCHP_CHIP == 7400)				
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, RAMP_ENABLE, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, CDB_ITB_UNF, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, CDB_RDY, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, STREAM_INFO_REG_CHG, 1))				
#endif				
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, ASTM_TSM_PASS, 1));

	BRAP_Write32 (hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR + ui32Offset, ui32RegVal );


	if (!BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap))
	{
		/* Install the interrupt callback for the DSP to HOST 7401 L2 Audio interrupt */
		if(hRapCh->eChannelType == BRAP_P_ChannelType_eDecode)
		{
			ret = BINT_CreateCallback(
					&hRapCh->hDSPCallback,
					hRapCh->hInt,
					ui32DSPInterruptId[hRapCh->sRsrcGrnt.uiDspContextId],
					BRAP_P_DSP2Host_isr,
					(void*)hRapCh,
					0 /* Not used */);
			if ( ret != BERR_SUCCESS )
			{
				BDBG_MSG(("Create Callback failed for DSP INT_ID[%d]",ui32DSPInterruptId[hRapCh->sRsrcGrnt.uiDspContextId]));
				return BERR_TRACE(ret);
			}
			ret = BINT_EnableCallback(hRapCh->hDSPCallback);
			if ( ret != BERR_SUCCESS )
			{
				BDBG_MSG(("Enable Callback failed for DSP INT_ID[%d]",ui32DSPInterruptId[hRapCh->sRsrcGrnt.uiDspContextId]));
				return BERR_TRACE(ret);
			}
		}

		/* Install the interrupt callback for the FMM to HOST 7401 L2 Audio interrupt */
		ret = BINT_CreateCallback(
					&hRapCh->hFMMCallback,
					hRapCh->hInt,
					BCHP_INT_ID_FMM_BF,
					BRAP_P_FMM_BF_isr,
					(void*)hRapCh,
					0 /* Not used */);
		if ( ret != BERR_SUCCESS )
		{
			BDBG_MSG(("Create Callback failed for FMM INT_ID[%d]", BCHP_INT_ID_FMM_BF));
			return BERR_TRACE(ret);
		}
		ret = BINT_EnableCallback(hRapCh->hFMMCallback);		
		if ( ret != BERR_SUCCESS )
		{
			BDBG_MSG(("Enable Callback failed for FMM INT_ID[%d]", BCHP_INT_ID_FMM_BF));
			return BERR_TRACE(ret);
		}
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0) || (BCHP_CHIP == 7403))
		ret = BINT_CreateCallback(
			&hRapCh->hFMMOpCallback,
			hRapCh->hInt,
			BCHP_INT_ID_FMM_OP,
			BRAP_P_CRC_isr,
			(void*)hRapCh,
			0 /* Not used */
			);
		if ( ret != BERR_SUCCESS )
		{
			BDBG_MSG(("Create Callback failed for FMM INT_ID[%d]", BCHP_INT_ID_FMM_OP));
			ret = BERR_TRACE(ret);
			return BERR_TRACE(ret);
		}

		ret = BINT_EnableCallback(hRapCh->hFMMOpCallback);		
		if ( ret != BERR_SUCCESS )
		{
			BDBG_MSG(("Enable Callback failed for FMM INT_ID[%d]", BCHP_INT_ID_FMM_OP));
			ret = BERR_TRACE(ret);
			return BERR_TRACE(ret);
		}
#endif
	}
	else
	{/* Watchdog recovery mode */
		/* First disable and then enable the callback */
		if(hRapCh->eChannelType == BRAP_P_ChannelType_eDecode
            		&& hRapCh->hDSPCallback)
		{	
   			ret = BINT_DisableCallback(hRapCh->hDSPCallback);		
			if ( ret != BERR_SUCCESS )
			{
				BDBG_MSG(("Disable Callback failed for DSP"));
				return BERR_TRACE(ret);
			}
		}
        	if(hRapCh->hFMMCallback)
        	{
    			ret = BINT_DisableCallback(hRapCh->hFMMCallback);		
			if ( ret != BERR_SUCCESS )
			{
				BDBG_MSG(("Disable Callback failed for FMM"));
				return BERR_TRACE(ret);
			}
        	}
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)||(BCHP_CHIP == 7403))
        	if(hRapCh->hFMMOpCallback)
        	{
    			ret = BINT_DisableCallback(hRapCh->hFMMOpCallback);		
			if ( ret != BERR_SUCCESS )
			{
				BDBG_MSG(("Disable Callback failed for FMM"));
				return BERR_TRACE(ret);
			}
        	}
#endif		
		if(hRapCh->eChannelType == BRAP_P_ChannelType_eDecode)
		{
			ret = BINT_EnableCallback(hRapCh->hDSPCallback);
			if ( ret != BERR_SUCCESS )
			{
				BDBG_MSG(("Enable Callback failed for DSP"));
				return BERR_TRACE(ret);
			}
		}
		ret = BINT_EnableCallback(hRapCh->hFMMCallback);
		if ( ret != BERR_SUCCESS )
		{
			BDBG_MSG(("Enable Callback failed for FMM"));
			return BERR_TRACE(ret);
		}
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)||(BCHP_CHIP == 7403))
		ret = BINT_EnableCallback(hRapCh->hFMMOpCallback);
		if ( ret != BERR_SUCCESS )
		{
			BDBG_MSG(("Enable Callback failed for FMM"));
			return BERR_TRACE(ret);
		}
#endif
	}
#endif /* ifndef EMULATION */

	BDBG_LEAVE(BRAP_P_InterruptInstall);
	return ret;    
}

BERR_Code BRAP_P_InterruptUnInstall (
	BRAP_ChannelHandle 		hRapCh		/* [in] Raptor Channel handle */
)
{
	BERR_Code ret = BERR_SUCCESS;

#if 0	
	BREG_Handle hRegister;
#endif

	BDBG_ASSERT (hRapCh);            

	BDBG_ENTER (BRAP_P_InterruptUnInstall);    

#ifndef EMULATION /* dont handle interrupts in emulation */

#if 0
	hRegister = hRapCh->hRegister;
#endif

	/* TODO: Do we need to mask all lower level interrupt?? */

	/* UnInstall the interrupt callback for DSP and FMM 7401 L2 Audio interrupt */
    if(hRapCh->hDSPCallback)
    {
    	ret = BINT_DisableCallback(hRapCh->hDSPCallback);
	if (ret!=BERR_SUCCESS)
		return BERR_TRACE(ret);
		
    	ret = BINT_DestroyCallback(hRapCh->hDSPCallback);
	if (ret!=BERR_SUCCESS)
		return BERR_TRACE(ret);
    }
    if(hRapCh->hFMMCallback)
    {
    	ret = BINT_DisableCallback(hRapCh->hFMMCallback);
	if (ret!=BERR_SUCCESS)
		return BERR_TRACE(ret);
		
    	ret = BINT_DestroyCallback(hRapCh->hFMMCallback);
	if (ret!=BERR_SUCCESS)
		return BERR_TRACE(ret);
    }
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)||(BCHP_CHIP == 7403))
	if(hRapCh->hFMMOpCallback)
	{
		ret = BINT_DisableCallback(hRapCh->hFMMOpCallback);
		if (ret!=BERR_SUCCESS)
	    		return BERR_TRACE(ret);

		ret = BINT_DestroyCallback(hRapCh->hFMMOpCallback);
		if (ret!=BERR_SUCCESS)
			return BERR_TRACE(ret);
	}
#endif
	BDBG_MSG(("Callbacks destroyed."));
#endif /* ifndef EMULATION */

	BDBG_LEAVE(BRAP_P_InterruptUnInstall);
	return ret;    
}
#endif


#ifdef BCHP_7411_VER /* Only for the 7411 */
void BRAP_P_TopLevel_isr (
        void * pParm1, /* [in] Raptor channel handle */
        int    iParm2  /* [in] Not used */        
)
{

    BRAP_ChannelHandle hRapCh;
    uint32_t ui32Offset=0;
    uint32_t ui32IntStatus=0;
    uint32_t ui32MaskStatus=0;
    uint32_t ui32RegVal=0;
    uint32_t temp=0;
    BRAP_RM_P_OpPathResrc  sOpPathResrc;
    bool    bWMFlag = false;
    bool    bFullWMflag = false;
	
    BDBG_ENTER (BRAP_P_TopLevel_isr);
    BDBG_ASSERT (pParm1);
   
	BSTD_UNUSED(iParm2);
    hRapCh = (BRAP_ChannelHandle) pParm1;

	BDBG_MSG(("BRAP_P_TopLevel_isr\n"));
 
    /* Check all ESR registers and call corresponding ISR for all supported
       interrupts */

    /* For buffer block related interrupts, check AUD_FMM_BF_ESR1_STATUS */
    ui32IntStatus = BRAP_Read32_isr (hRapCh->hRegister, BCHP_AUD_FMM_BF_ESR1_STATUS);
    BDBG_MSG (("BCHP_AUD_FMM_BF_ESR1_STATUS = 0x%x", ui32IntStatus));

    /* Clear the interrupts in BCHP_AUD_FMM_BF_ESR1_CLEAR.
     * This is edge triggered. so we need to clear first.*/
    ui32MaskStatus = BRAP_Read32_isr (hRapCh->hRegister, BCHP_AUD_FMM_BF_ESR1_MASK);
    BDBG_MSG (("BCHP_AUD_FMM_BF_ESR1_MASK = 0x%x", ui32MaskStatus));

    ui32IntStatus &= (hRapCh->ui32FmmBFIntMask & (~ui32MaskStatus));    
    BDBG_MSG(("ui32IntStatus = 0x%x",ui32IntStatus));    

    BRAP_Write32_isr ( hRapCh->hRegister, BCHP_AUD_FMM_BF_ESR1_STATUS_CLEAR, ui32IntStatus);
    /* Clear the Buffer Block bit in AIO_INTH_R5F_CLEAR */
    BRAP_Write32_isr ( hRapCh->hRegister, BCHP_AIO_INTH_R5F_CLEAR, BCHP_AIO_INTH_R5F_CLEAR_FMM_BF_MASK);

    /* TODO: this should ideally check over all outputChannelPairs, but since
      this top level ISR is specific to 7411, which can have only 1 pair,
      we check only for that pair */
   	sOpPathResrc = hRapCh->sRsrcGrnt.sOpResrcId[0];
	if(sOpPathResrc.uiSrcChId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
        switch (sOpPathResrc.uiSrcChId)
        {
            case 0:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_0_EXCEED_FREEMARK) != 0;
                break;   
            case 1:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_1_EXCEED_FREEMARK) != 0;
                break;
            case 2:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_2_EXCEED_FREEMARK) != 0;
                break;
            case 3:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_3_EXCEED_FREEMARK) != 0;
                break;
            case 4:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_4_EXCEED_FREEMARK) != 0;
                break;
            case 5:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_5_EXCEED_FREEMARK) != 0;
                break;
            default:
                break;
        }
		if ((bWMFlag==true) && (hRapCh->ui32FmmIntMask & BCHP_AIO_INTH_R5F_SET_FMM_BF_MASK))
		{
           BRAP_P_FmmRbufFreeWaterMark_isr (pParm1, sOpPathResrc.uiSrcChId); 
		}
	}

	sOpPathResrc = hRapCh->sSimulPtRsrcGrnt.sOpResrcId[0];
	bWMFlag = false;
	if(sOpPathResrc.uiSrcChId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
        switch (sOpPathResrc.uiSrcChId)
        {
            case 0:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_0_EXCEED_FREEMARK) != 0;
                break;   
            case 1:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_1_EXCEED_FREEMARK) != 0;
                break;
            case 2:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_2_EXCEED_FREEMARK) != 0;
                break;
            case 3:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_3_EXCEED_FREEMARK) != 0;
                break;
            case 4:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_4_EXCEED_FREEMARK) != 0;
                break;
            case 5:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_5_EXCEED_FREEMARK) != 0;
                break;
            default:
                break;
        }

		if ((bWMFlag==true) && (hRapCh->ui32FmmIntMask & BCHP_AIO_INTH_R5F_SET_FMM_BF_MASK))
		{
           BRAP_P_FmmRbufFreeWaterMark_isr (pParm1, sOpPathResrc.uiSrcChId); 
		}
	}

    /* Check if dest rbuf exceeds full byte mark for a capture channel in capture only mode */
    if(hRapCh->eChannelType == BRAP_P_ChannelType_eCapture &&
       hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly &&
       hRapCh->sModuleHandles.hDstCh->uiIndex != (unsigned int)BRAP_RM_P_INVALID_INDEX)
    {
        switch (hRapCh->sModuleHandles.hDstCh->uiIndex)
        {
            case 0:
                bFullWMflag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, DEST_RINGBUF_0_EXCEED_FULLMARK) != 0;
                break;   
            case 1:
                bFullWMflag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, DEST_RINGBUF_1_EXCEED_FULLMARK) != 0;
                break;   
            default:
                break;
        }/* switch */

		if ((bFullWMflag==true) && (hRapCh->ui32FmmIntMask & BCHP_AIO_INTH_R5F_SET_FMM_BF_MASK))
		{
           BRAP_P_FmmDstRbufFullWaterMark_isr(pParm1, hRapCh->sModuleHandles.hDstCh->uiIndex); 
		}       
    }
    /* Handle all other SrcCh, Mixer etc interrupts and clear corresponding bits in AIO_INTH_R5F_CLEAR */


    /* TODO: list all other interrupts to be serviced for all other FMM modules as reqd. */

    if(hRapCh->eChannelType == BRAP_P_ChannelType_eDecode)
    {
        /* Get correct register for this DSP context */
        switch (hRapCh->sRsrcGrnt.uiDspContextId)
        {
            case 0:
                ui32Offset = 0;
                ui32RegVal = BCHP_AUD_DSP_INTH0_R5F_CLEAR_ESR_SO0_MASK;
                break;
            case 1:
                ui32Offset = BCHP_AUD_DSP_ESR_SO10_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
                ui32RegVal = BCHP_AUD_DSP_INTH0_R5F_CLEAR_ESR_SO1_MASK;
                break;
            case 2:
                ui32Offset = BCHP_AUD_DSP_ESR_SO20_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
                ui32RegVal = BCHP_AUD_DSP_INTH0_R5F_CLEAR_ESR_SO2_MASK;
                break;
        }

        /* For DSP context X, check AUD_DSP_ESR_SOX0 */ 
        ui32MaskStatus = BRAP_Read32_isr (hRapCh->hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_STATUS + ui32Offset);
        BDBG_MSG (("BCHP_AUD_DSP_ESR_SOX0_MASK_STATUS=0x%x", ui32MaskStatus));
        ui32IntStatus = BRAP_Read32_isr (hRapCh->hRegister, BCHP_AUD_DSP_ESR_SO00_INT_STATUS + ui32Offset);
        BDBG_MSG (("BCHP_AUD_DSP_ESR_SOX0_INT_STATUS=0x%x", ui32IntStatus));

        /* Clear the interrupts in BCHP_AUD_DSP_ESR_SOX0.
         * This is edge triggered. so we need to clear first.*/
        ui32IntStatus &=  ~ui32MaskStatus;
        BRAP_Write32_isr (hRapCh->hRegister, BCHP_AUD_DSP_ESR_SO00_INT_CLEAR + ui32Offset, ui32IntStatus);
        /* Clear the DSP context bit in BCHP_AUD_DSP_INTH0_R5F_CLEAR */
        BRAP_Write32_isr (hRapCh->hRegister, BCHP_AUD_DSP_INTH0_R5F_CLEAR, ui32RegVal);
        temp = BRAP_Read32_isr (hRapCh->hRegister, BCHP_AUD_DSP_ESR_SO00_INT_STATUS + ui32Offset);

         /* PR 23103: If the FW sees a lock and unlock even happen within a frame, 
    	it sends out both together. The PI will then see them in the order 
    	which they are read, and may end up with the incorrect sequence. 
    	Since Fw would never trigger L->U (it wud only trigger U->L), we can 
    	work around it by having the PI always read the unlock bit first and 
    	then the lock bit. */
    	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, UNLOCK) == 1)
    	{
    		BRAP_P_DecoderUnlock_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
    	}        
    	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, LOCK) == 1)
    	{
    		BRAP_P_DecoderLock_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
    	}
		if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, SR_CHG) == 1)
        {
    		BRAP_P_SampleRateChange_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
        }
        if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, BR_CHG) == 1)
        {
    		BRAP_P_BitRateChange_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
        }
        if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, MOD_CHG) == 1)
        {
    		BRAP_P_ModeChange_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
        }
        if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, CRC_ERR) == 1)
        {
    		BRAP_P_CrcError_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
        }
        if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, PTS_RCV) == 1)
    	{
            BRAP_P_FirstPtsReady_isr (pParm1, hRapCh->sRsrcGrnt.uiDspContextId);  
    	}
        if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, PTS_DISCARD) == 1)
    	{
    		BRAP_P_PtsError_isr (pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
    	}
    	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, TSM_LOG) == 1)
    	{
    		BRAP_P_TsmLog_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
    	}

#if BCHP_7411_VER > BCHP_VER_C0
    	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, START_PTS_RCH) == 1)
    	{
    		BRAP_P_StartPtsReached_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
    	}    	
	    if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, END_PTS_RCH) == 1)
    	{
    		BRAP_P_StopPtsReached_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
    	}
#endif
        /* TODO: Add all other interrupts to be serviced for DSP context X as reqd */


        /* Unmask the interrupts */
       	BRAP_Write32_isr (hRapCh->hRegister, BCHP_AUD_DSP_INTH0_R5F_MASK_CLEAR, hRapCh->ui32DspIntMask);
    }/* if decode channel */
    
    BRAP_Write32_isr (hRapCh->hRegister, BCHP_AIO_INTH_R5F_MASK_CLEAR, hRapCh->ui32FmmIntMask);    
    
    BDBG_LEAVE (BRAP_P_TopLevel_isr);
    return ;
}
#else
/* for other than 7411 */
void BRAP_P_DSP2Host_isr (
        void * pParm1, /* [in] Raptor channel handle */
        int    iParm2  /* [in] Not used */        
)
{
	BRAP_ChannelHandle hRapCh;
	uint32_t ui32Offset=0;
	uint32_t ui32IntStatus=0;
	uint32_t ui32MaskStatus=0;

#if 0	
	uint32_t ui32RegVal=0;
#endif

	BDBG_ENTER (BRAP_P_DSP2Host_isr);
	BDBG_ASSERT (pParm1);

	BSTD_UNUSED(iParm2);
	hRapCh = (BRAP_ChannelHandle) pParm1;

	BDBG_MSG(("BRAP_P_DSP2Host_isr\n"));

	/* Check all ESR registers and call corresponding ISR for all supported
	interrupts */
	
	/* Get correct register for this DSP context */
	switch (hRapCh->sRsrcGrnt.uiDspContextId)
	{
		case 0:
			ui32Offset = 0;
#if 0			
			ui32RegVal = BCHP_AUD_DSP_INTH0_R5F_CLEAR_ESR_SO0_MASK;
#endif
			break;
		case 1:
			ui32Offset = BCHP_AUD_DSP_ESR_SO10_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
#if 0			
			ui32RegVal = BCHP_AUD_DSP_INTH0_R5F_CLEAR_ESR_SO1_MASK;
#endif
			break;
		case 2:
			ui32Offset = BCHP_AUD_DSP_ESR_SO20_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
#if 0			
			ui32RegVal = BCHP_AUD_DSP_INTH0_R5F_CLEAR_ESR_SO2_MASK;
#endif
			break;
#if ( BCHP_CHIP == 7400 )
		case 3:
			ui32Offset = BCHP_AUD_DSP_ESR_SO30_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
#if 0			
			ui32RegVal = BCHP_AUD_DSP_INTH0_R5F_CLEAR_ESR_SO3_MASK;
#endif
			break;
		case 4:
			ui32Offset = BCHP_AUD_DSP_ESR_SO40_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
#if 0			
			ui32RegVal = BCHP_AUD_DSP_INTH0_R5F_CLEAR_ESR_SO4_MASK;
#endif
			break;
		case 5:
			ui32Offset = BCHP_AUD_DSP_ESR_SO50_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
#if 0			
			ui32RegVal = BCHP_AUD_DSP_INTH0_R5F_CLEAR_ESR_SO5_MASK;
#endif
			break;
#endif
	}

	/* For DSP context X, check AUD_DSP_ESR_SOX0 */ 
	ui32MaskStatus = BRAP_Read32_isr (hRapCh->hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_STATUS + ui32Offset);
	BDBG_MSG (("BCHP_AUD_DSP_ESR_SOX0_MASK_STATUS=0x%x", ui32MaskStatus));
	ui32IntStatus = BRAP_Read32_isr (hRapCh->hRegister, BCHP_AUD_DSP_ESR_SO00_INT_STATUS + ui32Offset);
	BDBG_MSG (("BCHP_AUD_DSP_ESR_SOX0_INT_STATUS=0x%x", ui32IntStatus));

	/* Clear the interrupts in BCHP_AUD_DSP_ESR_SOX0.
	* This is edge triggered. so we need to clear first.*/
	ui32IntStatus &=  ~ui32MaskStatus;
	BRAP_Write32_isr (hRapCh->hRegister, BCHP_AUD_DSP_ESR_SO00_INT_CLEAR + ui32Offset, ui32IntStatus);

	/* PR 23103: If the FW sees a lock and unlock even happen within a frame, 
    it sends out both together. The PI will then see them in the order 
    which they are read, and may end up with the incorrect sequence. 
    Since Fw would never trigger L->U (it wud only trigger U->L), we can 
    work around it by having the PI always read the unlock bit first and 
    then the lock bit. */    
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, UNLOCK) == 1)
	{
		BRAP_P_DecoderUnlock_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, LOCK) == 1)
	{
		BRAP_P_DecoderLock_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, SR_CHG) == 1)
	{
		BRAP_P_SampleRateChange_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, BR_CHG) == 1)
	{
		BRAP_P_BitRateChange_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, MOD_CHG) == 1)
	{
		BRAP_P_ModeChange_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, CRC_ERR) == 1)
	{
		BRAP_P_CrcError_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, PTS_RCV) == 1)
	{
		BRAP_P_FirstPtsReady_isr (pParm1, hRapCh->sRsrcGrnt.uiDspContextId);  
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, PTS_DISCARD) == 1)
	{
		BRAP_P_PtsError_isr (pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, TSM_LOG) == 1)
	{
		BRAP_P_TsmLog_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, ASTM_TSM_PASS) == 1)
	{
		BRAP_P_AstmTsmPass_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}
#if ( BCHP_CHIP == 7400 )
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, STREAM_INFO_REG_CHG) == 1)
	{
		BRAP_P_StreaminfoChange_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, RAMP_ENABLE) == 1)
	{
		BRAP_P_RampEnable_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, CDB_ITB_UNF) == 1)
	{
		BRAP_P_CDBITBUnderflow_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}	
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, CDB_RDY) == 1)
	{
		BRAP_P_CDBReady_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}	
#endif

    
#if (BRAP_P_INCLUDE_MPEG_LAYER_CHANGE == 1)    
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, MPEG_LAYER_CHANGE) == 1)
	{
		BRAP_P_MpegLayerChange_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}    
#endif   
#if (BRAP_P_INCLUDE_MPEG_RAVE_WORKAROUND == 1)    
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, MPEG_NOT_LAYER1) == 1)
	{
		BRAP_P_MpegNotLayer1_isr(pParm1, hRapCh->sRsrcGrnt.uiDspContextId);
	}    
#endif  
    /* TODO: Add all other interrupts to be serviced for DSP context X as reqd */

	BDBG_LEAVE (BRAP_P_DSP2Host_isr);
	return ;
}

void BRAP_P_FMM_BF_isr (
        void * pParm1, /* [in] Raptor channel handle */
        int    iParm2  /* [in] Not used */        
)
{
	BRAP_ChannelHandle hRapCh;
	uint32_t ui32IntStatus=0;
	uint32_t ui32MaskStatus=0;
	BRAP_RM_P_OpPathResrc  sOpPathResrc;
	bool    bWMFlag = false;
    bool    bFullWMflag = false;
	
	BDBG_ENTER (BRAP_P_FMM_BF_isr);
	BDBG_ASSERT (pParm1);

	BSTD_UNUSED(iParm2);

	hRapCh = (BRAP_ChannelHandle) pParm1;

	BDBG_MSG(("BRAP_P_FMM_BF_isr\n"));
 
	/* Check all ESR registers and call corresponding ISR for all supported
	interrupts */
    
	/* For buffer block related interrupts, check AUD_FMM_BF_ESR1_STATUS */
	ui32IntStatus = BRAP_Read32_isr (hRapCh->hRegister, BCHP_AUD_FMM_BF_ESR1_STATUS);
	BDBG_MSG (("BCHP_AUD_FMM_BF_ESR1_STATUS = 0x%x ChanType(%d)", ui32IntStatus, hRapCh->eChannelType));

	/* Clear the interrupts in BCHP_AUD_FMM_BF_ESR1_CLEAR.
	* This is edge triggered. so we need to clear first.*/
    ui32MaskStatus = BRAP_Read32_isr (hRapCh->hRegister, BCHP_AUD_FMM_BF_ESR1_MASK);
    BDBG_MSG (("BCHP_AUD_FMM_BF_ESR1_MASK = 0x%x", ui32MaskStatus));

    ui32IntStatus &= (hRapCh->ui32FmmBFIntMask & (~ui32MaskStatus));    
    BDBG_MSG(("ui32IntStatus = 0x%x",ui32IntStatus));    

	BRAP_Write32_isr ( hRapCh->hRegister, BCHP_AUD_FMM_BF_ESR1_STATUS_CLEAR, ui32IntStatus);
	
	/* TODO: this should ideally check over all outputChannelPairs, but since
	this top level ISR is specific to 7411, which can have only 1 pair,
	we check only for that pair */
	sOpPathResrc = hRapCh->sRsrcGrnt.sOpResrcId[0];
	if(sOpPathResrc.uiSrcChId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		switch (sOpPathResrc.uiSrcChId)
		{
			case 0:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                    AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_0_EXCEED_FREEMARK) != 0;
				break;   
			case 1:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                    AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_1_EXCEED_FREEMARK) != 0;
				break;
			case 2:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                    AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_2_EXCEED_FREEMARK) != 0;
				break;
			case 3:
                bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                    AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_3_EXCEED_FREEMARK) != 0;
				break;
#if ( BCHP_CHIP == 7400 )
			case 4:
				bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
					AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_4_EXCEED_FREEMARK) != 0;
				break;   
			case 5:
				bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
					AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_5_EXCEED_FREEMARK) != 0;
				break;   
			case 6:
				bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
					AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_6_EXCEED_FREEMARK) != 0;
				break;   
			case 7:
				bWMFlag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
					AUD_FMM_BF_ESR0_STATUS, SOURCE_RINGBUF_7_EXCEED_FREEMARK) != 0;
				break;   
#endif
			default:
				break;
		}
        if ((bWMFlag==true))
		{
			BRAP_P_FmmRbufFreeWaterMark_isr (pParm1, sOpPathResrc.uiSrcChId); 
        }
	}

    /* Check if dest rbuf exceeds full byte mark for a capture channel in capture only mode */
    if(hRapCh->eChannelType == BRAP_P_ChannelType_eCapture &&
       hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly &&
       hRapCh->sModuleHandles.hDstCh->uiIndex != (unsigned int)BRAP_RM_P_INVALID_INDEX)
    {
        switch (hRapCh->sModuleHandles.hDstCh->uiIndex)
        {
            case 0:
                bFullWMflag = BCHP_GET_FIELD_DATA (ui32IntStatus, 
                        AUD_FMM_BF_ESR0_STATUS, DEST_RINGBUF_0_EXCEED_FULLMARK) != 0;
                break;   
            default:
                break;
        }/* switch */

		if ((bFullWMflag==true) && (hRapCh->ui32FmmIntMask & BCHP_AIO_INTH_R5F_SET_FMM_BF_MASK))
		{
           BRAP_P_FmmDstRbufFullWaterMark_isr(pParm1, hRapCh->sModuleHandles.hDstCh->uiIndex); 
		}
    }

	/* Handle all other SrcCh, Mixer etc interrupts and clear corresponding bits in AIO_INTH_R5F_CLEAR */

	/* TODO: list all other interrupts to be serviced for all other FMM modules as reqd. */
	BDBG_LEAVE (BRAP_P_FMM_BF_isr);
	return ;
}

#endif



void BRAP_P_FmmDstRbufFullWaterMark_isr (
        void * pParm1, /* [in] Raptor channel handle */
        int    iParm2  /* [in] RBUF index */
)
{
    BRAP_ChannelHandle hRapCh;
    BDBG_ASSERT (pParm1);
    BSTD_UNUSED ( iParm2 );

    hRapCh = (BRAP_ChannelHandle) pParm1;

#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT==0)
    if(hRapCh->eChannelType == BRAP_P_ChannelType_eCapture)
        BRAP_P_MaskInterrupt_isr(hRapCh, BRAP_Interrupt_eFmmDstRbufFullMark);
#endif
#endif

#ifndef RAP_SRSTRUVOL_CERTIFICATION
    hRapCh->hInterruptCount->uiFmmDstRbufFullWaterMark++;
#endif

    /* If PCM capture channel, then call the callback handler only if it is in 
       running state. */
    /* Note: Here we should be checking "channel state". But since "channel state" is 
             not maintained separately, we look at the SrcCh state. Also, PCM capture
             always uses 0th channel pair. */
    if(hRapCh->eChannelType == BRAP_P_ChannelType_eCapture &&
       hRapCh->sModuleHandles.hDstCh->eState == BRAP_SRCCH_P_State_eRunning)
    {
        if( hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmDstRbufFullMark].pfAppCb)
    	{
    		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmDstRbufFullMark].pfAppCb (
    			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmDstRbufFullMark].pParm1,
    			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmDstRbufFullMark].iParm2,
    			NULL /* Not used */
    		);
    	}
    }
    return;
}

void BRAP_P_FmmRbufFreeWaterMark_isr (
        void * pParm1, /* [in] Raptor channel handle */
        int    iParm2  /* [in] RBUF index */
)
{
    BRAP_ChannelHandle hRapCh;
    BDBG_ASSERT (pParm1);
    BSTD_UNUSED ( iParm2 );
    hRapCh = (BRAP_ChannelHandle) pParm1;
#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT==0)
    if(hRapCh->eChannelType == BRAP_P_ChannelType_ePcmPlayback)
        BRAP_P_MaskInterrupt_isr(hRapCh, BRAP_Interrupt_eFmmRbufFreeByte);
#endif
#endif
    /* If PCM Playback channel, then call the callback handler only if it is in 
       running state. */
    /* Note: Here we should be checking "channel state". But since "channel state" is not maintained separately, 
         we look at the SrcCh state. Also, PCM playback always uses 0th channel pair. */
    if(hRapCh->eChannelType == BRAP_P_ChannelType_ePcmPlayback &&
       hRapCh->sModuleHandles.hSrcCh[0]->eState == BRAP_SRCCH_P_State_eRunning)
    {
        if( hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmRbufFreeByte].pfAppCb)
    	{
    		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmRbufFreeByte].pfAppCb (
    			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmRbufFreeByte].pParm1,
    			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmRbufFreeByte].iParm2,
    			NULL /* Not used */
    		);
    	}
    }
   
    return;
}

#ifdef BCHP_7411_VER /* Only for the 7411 */
static void BRAP_P_Watchdog_isr(void *pParm1)
{
	BRAP_Handle hRap = (BRAP_Handle) pParm1;

	BDBG_WRN(("Raptor Watchdog Interrupt occured for RAP handle 0x%08x", hRap));
	if(hRap->sAppIntCbInfo[BRAP_DeviceLevelInterrupt_eWatchdog].pfAppCb) {
		hRap->sAppIntCbInfo[BRAP_DeviceLevelInterrupt_eWatchdog].pfAppCb (
			hRap->sAppIntCbInfo[BRAP_DeviceLevelInterrupt_eWatchdog].pParm1,
			hRap->sAppIntCbInfo[BRAP_DeviceLevelInterrupt_eWatchdog].iParm2,
			NULL);
	}
}
#else
static void BRAP_P_Watchdog_isr(void *pParm1,  int iParm2 )
{
	BRAP_Handle hRap = (BRAP_Handle) pParm1;
	BSTD_UNUSED ( iParm2 );

	/* Reset the DSP */
	BRAP_DSP_P_WatchDogResetHardware ( hRap->hDsp[0] );

	/* Disable the FMM block */
#if 0
/*Refer PR 26669: 
    - If we just disable the FMM here, and then renable it only in the 
    ProcessWatchdog task, then there's a long interval in between these two 
    during which any access to FMM registers will cause a bus error.
    So we should do any of the following:
    - Make sure to reset the FMM ie reable it here
        Drawback: if the app happens to read a register, it will get the reset 
        value for all registers instead of the actaul values
    - Dont reset the FMM at all here. let it get reset in ProcessWatchdog()
        Drawback: if any register is corrupted, app will end up reading that 
        corrupted value.

    The second option seems like the lesser of the two evils. It is valid for 
    us to tell the application that things will be in an shaky state in the 
    interval between this ISR and the ProcessWatchdog() callback being called. 
    */

	BRAP_FMM_P_ResetHardware ( hRap->hFmm[0] );
#endif
	BDBG_WRN(("Raptor Watchdog Interrupt occured for RAP handle 0x%08x", hRap));

	/* Call the application callback */
	if(hRap->sAppIntCbInfo[BRAP_DeviceLevelInterrupt_eWatchdog].pfAppCb) {
		hRap->sAppIntCbInfo[BRAP_DeviceLevelInterrupt_eWatchdog].pfAppCb (
		hRap->sAppIntCbInfo[BRAP_DeviceLevelInterrupt_eWatchdog].pParm1,
		hRap->sAppIntCbInfo[BRAP_DeviceLevelInterrupt_eWatchdog].iParm2,
		NULL);
	}
}
#endif

static void BRAP_P_FirstPtsReady_isr(void *pParm1, int dummy)
{
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;
	BRAP_DSPCHN_PtsInfo	sPtsInfo;

	BSTD_UNUSED(dummy);
	BDBG_MSG(("First PTS Ready interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));


	BRAP_DSPCHN_GetCurrentPTS_isr(hRapCh, &sPtsInfo);

	/* Call the application callback function */

	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFirstPtsReady].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFirstPtsReady].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFirstPtsReady].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFirstPtsReady].iParm2,
			&sPtsInfo);
	}

    return;
}

static void BRAP_P_PtsError_isr(void *pParm1, int dummy)
{
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;
	BRAP_DSPCHN_PtsInfo	sPtsInfo;
	
	BSTD_UNUSED(dummy);
	BDBG_MSG(("TSM error interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

	    hRapCh->hInterruptCount->uiPtsError++;

	BRAP_DSPCHN_GetCurrentPTS_isr(hRapCh, &sPtsInfo);

	/* Call the application callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_ePtsError].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_ePtsError].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_ePtsError].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_ePtsError].iParm2,
			&sPtsInfo);
	}
    return;
}

static void BRAP_P_StreaminfoChange_isr(void *pParm1, int dummy)
{
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;


	BSTD_UNUSED(dummy);
	BDBG_MSG(("StreamInfo Change interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));
	
	/* Call the application callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStreamInfoAvailable].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStreamInfoAvailable].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStreamInfoAvailable].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStreamInfoAvailable].iParm2,
			NULL);
    }
    return;
}


static void BRAP_P_SampleRateChange_isr(void *pParm1, int dummy)
{
    BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;
    BRAP_DSPCHN_P_StreamInfo	sPrivateStreamInfo;
    unsigned int i=0;
#ifndef 	BCHP_7411_VER
	unsigned int j = 0;
#endif
    BAVC_AudioSamplingRate eSamplingRate;
	BRAP_P_OpPortPrevMuteStatus sOpPortPrevMuteStatus;

    BSTD_UNUSED(dummy);
    BDBG_MSG(("Sample Rate Change interrupt occured for RAP channel %d, DSP context %d Old hDspCh->eSR = %d",
    		hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex, 
    		hRapCh->sModuleHandles.hDspCh->eSamplingRate));

	BRAP_P_MuteChannelOutputOnSr_isr(hRapCh, &sOpPortPrevMuteStatus);

    BRAP_DSPCHN_P_GetStreamInformation_isr(hRapCh->sModuleHandles.hDspCh, &sPrivateStreamInfo);
    BDBG_MSG(("New hDspCh->eSR = %d",hRapCh->sModuleHandles.hDspCh->eSamplingRate));
      eSamplingRate = hRapCh->sModuleHandles.hDspCh->eSamplingRate;


    if (hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode
        == BRAP_DSPCHN_DecodeMode_ePassThru)
    {
        /* if the channel is in PassThru mode, it carryies only compressed data and 
        it wont be mixed with any PCM channels. We merely update the SR in the output port handle 
        and do nothing else*/    
        hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eSamplingRate = eSamplingRate; 
    }
    else
    {
    
    /* Check all output ports being used by this channel */  
    for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
        if (hRapCh->sModuleHandles.hOp[i] != NULL)
        {
            /* This PI will do SRC and update the SR for any PB/CAP channels associated with this output port */

            BRAP_OP_P_SetSamplingRate(
                    hRapCh->hRap, 
                    hRapCh->sModuleHandles.hOp[i]->eOutputPort,
		      eSamplingRate, 
                    true, /* bOverride */
                    true /* bCalledFromISR */);
             /* remember to save the new SR for this output port */
            hRapCh->sModuleHandles.hOp[i]->eSamplingRate = eSamplingRate;        
        }

#ifndef BCHP_7411_VER /* For chips other than 7411 */
    if( hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.bProgramPLL==true)
    {
        /* If this channel pair is cloned to any other output ports, handle them also */
        for(j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
        {
            if ((hRapCh->sCloneOpPathHandles[i][j].hOp != NULL)
                && (hRapCh->sCloneOpPathHandles[i][j].bSimul == false))
            {
                /* This PI will do SRC and update the SR for any PB/CAP channels associated with this port */
                BRAP_OP_P_SetSamplingRate(
                        hRapCh->hRap, 
                        hRapCh->sCloneOpPathHandles[i][j].hOp->eOutputPort,
    		          eSamplingRate, 
                        true, /* bOverride */
                        true /* bCalledFromISR */);
                /* remember to save the new SR for this output port */
                hRapCh->sCloneOpPathHandles[i][j].hOp->eSamplingRate = eSamplingRate;         
            }
        }/* for  BRAP_RM_P_MAX_OUTPUTS*/
	}
#endif
    }

    /* For the SimulPt port, we dont do  SRC since SR is controlled by DSP. But we update the SR in the output port handle */
    if (hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR] != NULL)
    {
        hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eSamplingRate = eSamplingRate;         
#ifndef BCHP_7411_VER /* For chips other than 7411 */
        /* If this simultpt port has been cloned, update the cloned port handle as well */
        for(j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
        {
            if ((hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp != NULL)
                && (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].bSimul == true))
            {
                hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp->eSamplingRate = eSamplingRate;         
            }
        }/* for  BRAP_RM_P_MAX_OUTPUTS*/
#endif
        
    }
    }
    if(hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eStreamType 
            == BAVC_StreamType_eTsMpeg)/* MPEG-2 */
    {
        for(i=0; i< BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        {
            if(hRapCh->sModuleHandles.hSpdifFm[i] != NULL)
            {
                BRAP_SPDIFFM_P_ChangeBurstRepPeriodForMPEG2_isr(
                        hRapCh->sModuleHandles.hSpdifFm[i]);
            }
        }
    }
	/* If sample rate change, call the app callback function for sample rate change */
		if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eSampleRateChange].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eSampleRateChange].pfAppCb(
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eSampleRateChange].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eSampleRateChange].iParm2,
			&sPrivateStreamInfo.sPrivateFsChangeInfo.sSampleRateChangeInfo);
	}
        
    /* Decode on MAI */
    if(BRAP_P_ChannelType_eDecode == hRapCh->eChannelType)
    {
    	 uint32_t reg = 0;

#ifndef RAP_DRA_SUPPORT
        if (hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode
            != BRAP_DSPCHN_DecodeMode_ePassThru)
#else
        if(((hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort == BRAP_OutputPort_eSpdif) &&
            ((hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eType == BRAP_DSPCHN_AudioType_eDra)
            || (hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eType == BRAP_DSPCHN_AudioType_eDts))) 
            || (hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode
            != BRAP_DSPCHN_DecodeMode_ePassThru))                     
#endif            
        {
             for(i=0; i< BRAP_RM_P_MAX_SPDIFFM_STREAMS; i++)
            {
                if(hRapCh->hRap->hFmm[0]->hSpdifFm[i] != NULL)
                {
    			reg = BRAP_Read32(hRapCh->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 + 
                            ((BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1 - BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0)*hRapCh->hRap->hFmm[0]->hSpdifFm[i]->uiStreamIndex));
    			reg |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA, Enable);
    			BRAP_Write32 (hRapCh->hRegister, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 +  
                            ((BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1 - BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0)*hRapCh->hRap->hFmm[0]->hSpdifFm[i]->uiStreamIndex), reg);
                }
            }
        }
    }
     
	BRAP_P_UnMuteChannelOutputOnSr_isr(hRapCh, &sOpPortPrevMuteStatus);
    return;
}

static void BRAP_P_BitRateChange_isr(void *pParm1, int dummy)
{

	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;
	BRAP_DSPCHN_P_StreamInfo	sPrivateStreamInfo;
    BRAP_DSPCHN_BitRateChangeInfo sBitRateInfo;
	
	BSTD_UNUSED(dummy);
	BDBG_MSG(("Bit Rate Change interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

	BRAP_DSPCHN_P_GetStreamInformation_isr(hRapCh->sModuleHandles.hDspCh, &sPrivateStreamInfo);

    sBitRateInfo.eType = sPrivateStreamInfo.sStreamInfo.eType;
   
    /* QUERY: do any other audio types need to be handled here ? */
	switch (sBitRateInfo.eType) {
		case BRAP_DSPCHN_AudioType_eMpeg:
			sBitRateInfo.ui32BitRate = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sMpegInfo.ui32BitRate; 
			sBitRateInfo.ui32BitRateIndex = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sMpegInfo.ui32BitRateIndex;
			break;
		case BRAP_DSPCHN_AudioType_eAacSbr:
			sBitRateInfo.ui32BitRate = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAacSbrInfo.sAacInfo.ui32BitRate; 
			sBitRateInfo.ui32BitRateIndex = 0;
			break;
		case BRAP_DSPCHN_AudioType_eAac:
			sBitRateInfo.ui32BitRate = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAacInfo.ui32BitRate; 
			sBitRateInfo.ui32BitRateIndex = 0;
			break;
		case BRAP_DSPCHN_AudioType_eDts:
			sBitRateInfo.ui32BitRate = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sDtsInfo.ui32BitRate; 
			sBitRateInfo.ui32BitRateIndex = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sDtsInfo.ui32BitRateIndex;
			break;
		case BRAP_DSPCHN_AudioType_eDtshd:
			sBitRateInfo.ui32BitRate = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sDtshdInfo.ui32BitRate; 
			sBitRateInfo.ui32BitRateIndex = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sDtshdInfo.ui32BitRateIndex;
			break;
		case BRAP_DSPCHN_AudioType_eWmaStd:
			sBitRateInfo.ui32BitRate = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sWmaStdInfo.ui32BitRate;
			sBitRateInfo.ui32BitRateIndex = 0;
			break;
		case BRAP_DSPCHN_AudioType_eWmaPro:
			sBitRateInfo.ui32BitRate = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sWmaProInfo.ui32BitRate;
			sBitRateInfo.ui32BitRateIndex = 0;
			break;			
		default: 
			BDBG_ERR(("Bit Rate information not available for Audio type %d", sBitRateInfo.eType));
            break;
	}
    
	/* Call the application streaminfo callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eBitRateChange].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eBitRateChange].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eBitRateChange].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eBitRateChange].iParm2,
			&sBitRateInfo);
	}
    return;
}


static void BRAP_P_ModeChange_isr(void *pParm1, int dummy)
{

	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;
	BRAP_DSPCHN_P_StreamInfo	sPrivateStreamInfo;
    BRAP_DSPCHN_ModeChangeInfo sModeInfo;
	
	BSTD_UNUSED(dummy);
	BDBG_MSG(("Mode Change interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

	BRAP_DSPCHN_P_GetStreamInformation_isr(hRapCh->sModuleHandles.hDspCh, &sPrivateStreamInfo);

    sModeInfo.eType = sPrivateStreamInfo.sStreamInfo.eType;
    
    /* QUERY: do any other audio types need to be handled here ? */
	switch (sModeInfo.eType) {
		case BRAP_DSPCHN_AudioType_eMpeg:
			sModeInfo.uModInfo.eChmod = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sMpegInfo.eChmod; 
			sModeInfo.bLfeOn = 0; /* Not supported */
			break;
		case BRAP_DSPCHN_AudioType_eAc3:
			sModeInfo.uModInfo.eAcmod = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAc3Info.eAcmod; 
			sModeInfo.bLfeOn = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAc3Info.bLfeOn; 			
			break;
		case BRAP_DSPCHN_AudioType_eAc3Lossless:
			sModeInfo.uModInfo.eAcmod = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAc3LosslessInfo.sAc3Info.eAcmod; 
			sModeInfo.bLfeOn = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAc3LosslessInfo.sAc3Info.bLfeOn;
			break;
		case BRAP_DSPCHN_AudioType_eAc3Plus:
			sModeInfo.uModInfo.eAcmod = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAc3PlusInfo.sAc3Info.eAcmod; 
			sModeInfo.bLfeOn = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAc3PlusInfo.sAc3Info.bLfeOn;
			break;
#if (BRAP_DTS_SUPPORTED == 1)		            
		case BRAP_DSPCHN_AudioType_eDts:
			sModeInfo.uModInfo.eAcmod = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sDtsInfo.eAMode;
			sModeInfo.bLfeOn = 0; /* TODO -sPrivateStreamInfo.sStreamInfo.uStreamInfo.sDtsInfo.bLfeOn;*/
			break;       
#endif            
		case BRAP_DSPCHN_AudioType_eAac:
			sModeInfo.uModInfo.eAacAcmod = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAacInfo.eAcmod;
			sModeInfo.bLfeOn = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAacInfo.bLfeOn;
			break;
		case BRAP_DSPCHN_AudioType_eAacSbr:
			sModeInfo.uModInfo.eAacAcmod = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAacSbrInfo.sAacInfo.eAcmod;
			sModeInfo.bLfeOn = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sAacSbrInfo.sAacInfo.bLfeOn;
			break;
		case BRAP_DSPCHN_AudioType_eWmaStd:
			sModeInfo.uModInfo.eWmaStdAcmod = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sWmaStdInfo.eAcmod;
			sModeInfo.bLfeOn = 0; /* Not supported */
			break;
		case BRAP_DSPCHN_AudioType_eWmaPro:
			sModeInfo.uModInfo.eWmaProAcmod = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sWmaProInfo.eAcmod;
			sModeInfo.bLfeOn = sPrivateStreamInfo.sStreamInfo.uStreamInfo.sWmaProInfo.bLfeOn;
			break;
		case BRAP_DSPCHN_AudioType_eDra:
			break;            
		default: 
			BDBG_ERR(("Mode information not available for Audio type %d", sModeInfo.eType));
            break;
	}
    
	/* Call the application streaminfo callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eModeChange].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eModeChange].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eModeChange].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eModeChange].iParm2,
			&sModeInfo);
	}
    return;
}

static void BRAP_P_CrcError_isr(void *pParm1, int dummy)
{
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;

	BSTD_UNUSED(dummy);
	BDBG_MSG(("CRC error interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

    hRapCh->hInterruptCount->uiCrcError++;

	/* Call the application callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCrcError].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCrcError].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCrcError].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCrcError].iParm2,
			NULL);
	}
	return;
}

#if (BRAP_P_INCLUDE_MPEG_LAYER_CHANGE == 1)
static void BRAP_P_MpegLayerChange_isr(void *pParm1, int dummy)
{
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;


	BSTD_UNUSED(dummy);
	BDBG_MSG(("Mpeg layer change interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

    /* Do nothing here. The application should install BRAP_DEC_Flush() as the 
    callback for this interrupt. BRAP_DEC_Flush()will restart the channel 
    cleanly. */
	
	/* Call the application callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eMpegLayerChange].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eMpegLayerChange].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eMpegLayerChange].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eMpegLayerChange].iParm2,
			NULL);
	}
	return;
}
#endif
#if (BRAP_P_INCLUDE_MPEG_RAVE_WORKAROUND == 1)
static void BRAP_P_MpegNotLayer1_isr(void *pParm1, int dummy)
{
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;
    uint32_t ui32Offset =0;


	BSTD_UNUSED(dummy);
	BDBG_MSG(("Mpeg_not_layer1 interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

    ui32Offset = (BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR)  
                        * hRapCh->uiXptChannelNo ;

    BREG_Write32 (hRapCh->hRegister, 
                    BCHP_XPT_RAVE_CX0_AV_EXCLUSION + ui32Offset, 
                    BRAP_P_XPT_COMP2_EXCLUSION);
	
    /* Do not add an application callback for this */
	return;
}
#endif

static void BRAP_P_TsmLog_isr(void *pParm1, int dummy)
{
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;
	BRAP_DSPCHN_TsmLogInfo sTsmLogInfo;

	BSTD_UNUSED(dummy);
	BDBG_MSG(("TSM log interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

	BRAP_DSPCHN_P_TsmLog_isr(hRapCh->sModuleHandles.hDspCh, &sTsmLogInfo);
	
	/* Call the application callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eTsmLog].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eTsmLog].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eTsmLog].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eTsmLog].iParm2,
			&sTsmLogInfo);
	}
	return;
}
static void BRAP_P_DecoderLock_isr(void *pParm1, int dummy)
{
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;



	BSTD_UNUSED(dummy);
	BDBG_MSG(("Decoder lock interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

	hRapCh->sModuleHandles.hDspCh->bDecLocked = true;

	
	/* Call the application callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eDecLock].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eDecLock].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eDecLock].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eDecLock].iParm2,
			NULL);
	}
	return;
}

static void BRAP_P_DecoderUnlock_isr(void *pParm1, int dummy)
{
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;

	BSTD_UNUSED(dummy);
	BDBG_MSG(("Decoder unlock interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

	hRapCh->sModuleHandles.hDspCh->bDecLocked = false;

    hRapCh->hInterruptCount->uiDecoderUnlock++;
	
	/* Call the application callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eDecUnlock].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eDecUnlock].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eDecUnlock].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eDecUnlock].iParm2,
			NULL);
	}
	return;
}

#if BCHP_7411_VER > BCHP_VER_C0
static void BRAP_P_StartPtsReached_isr(void *pParm1, int dummy)
{
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;

	BSTD_UNUSED(dummy);
	BDBG_MSG(("Start PTS reached interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

	/* Call the application callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStartPtsReached].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStartPtsReached].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStartPtsReached].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStartPtsReached].iParm2,
			NULL);
	}
	return;

}

static void BRAP_P_StopPtsReached_isr(void *pParm1, int dummy)
{
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;

	BSTD_UNUSED(dummy);
	BDBG_MSG(("Stop PTS reached interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

	/* Call the application callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStopPtsReached].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStopPtsReached].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStopPtsReached].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eStopPtsReached].iParm2,
			NULL);
	}
	return;
}
#endif

#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)|| (BCHP_CHIP == 7403))
static void BRAP_P_CRC_isr(void *pParm1, int iParam2)
{
	BRAP_ChannelHandle	hRapCh = (BRAP_ChannelHandle)pParm1;
	uint32_t				Crc_cfg = hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmCrc].iParm2;
	uint32_t				Crc_value;
	BDBG_ENTER(BRAP_P_CRC_isr);
	BDBG_ASSERT(hRapCh);
	BSTD_UNUSED(iParam2);	
	BDBG_MSG(("In crc_isr %x",Crc_cfg));
	Crc_value=BRAP_Read32 (hRapCh->hRegister, 
                   ( BCHP_AUD_FMM_OP_CTRL_CRC_STATUSi_ARRAY_BASE 
                   + Crc_cfg*4 )); 

    hRapCh->hInterruptCount->uiFmmCRC++;
	
	if (hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmCrc].pfAppCb)
	{
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmCrc].pfAppCb (
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmCrc].pParm1,
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmCrc].iParm2,
		&Crc_value);
	}
	BDBG_LEAVE(BRAP_P_CRC_isr);
}
#endif


static void BRAP_P_AstmTsmPass_isr(void *pParm1, int iParam2)
{
	BRAP_ChannelHandle	hRapCh = (BRAP_ChannelHandle)pParm1;
	BRAP_DSPCHN_PtsInfo	sPtsInfo;

	BDBG_ENTER(BRAP_P_AstmTsmPass_isr);
	BDBG_ASSERT(hRapCh);
	BSTD_UNUSED(iParam2);	

	BRAP_DSPCHN_GetCurrentPTS_isr(hRapCh, &sPtsInfo);

	BDBG_MSG(("AstmTsmPass  interrupt occured for RAP channel %d, DSP context %d",
		hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

	if (hRapCh->sAppIntCbInfo[BRAP_Interrupt_eAstmTsmPass].pfAppCb)
	{
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eAstmTsmPass].pfAppCb (
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eAstmTsmPass].pParm1,
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eAstmTsmPass].iParm2,
		&sPtsInfo);
	}
	
	BDBG_LEAVE(BRAP_P_AstmTsmPass_isr);

}
#if ( BCHP_CHIP == 7400 )	
static void BRAP_P_RampEnable_isr(void *pParm1, int iParam2)
{
	BRAP_ChannelHandle	hRapCh = (BRAP_ChannelHandle)pParm1;
    uint32_t    ui32RegVal=0;

	BDBG_ENTER(BRAP_P_AstmTsmPass_isr);
	BDBG_ASSERT(hRapCh);
	BSTD_UNUSED(iParam2);	


	BDBG_MSG(("Ramp Enable  interrupt occured for RAP channel %d, DSP context %d",
		hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

    ui32RegVal = BRAP_Read32 (hRapCh->hRegister,BCHP_AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP);  

    ui32RegVal &= ~(BCHP_MASK (    
                    AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, 
                    SCALE_RAMP_STEP_SIZE));    

    ui32RegVal |= (BCHP_FIELD_DATA (    
                        AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, 
                        SCALE_RAMP_STEP_SIZE, 
                        0x20));

    BRAP_Write32 (hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, ui32RegVal);
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
    BRAP_Write32 (hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_FMM_SCALE_VOL_STEP, ui32RegVal);
#endif
	
	BDBG_LEAVE(BRAP_P_AstmTsmPass_isr);

}
static void BRAP_P_CDBITBUnderflow_isr(void *pParm1, int iParam2)
{

	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;
	BDBG_ENTER(BRAP_P_CDBITBUnderflow_isr);
	BSTD_UNUSED(iParam2);	

	BDBG_MSG(("Decoder CDBITBUnder flow interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

	
	/* Call the application callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCdbItbUnderflow].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCdbItbUnderflow].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCdbItbUnderflow].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCdbItbUnderflow].iParm2,
			NULL);
	}
	
	BDBG_LEAVE(BRAP_P_CDBITBUnderflow_isr);
	return;

}
static void BRAP_P_CDBReady_isr(void *pParm1, int iParam2)
{
	
	BRAP_ChannelHandle hRapCh = (BRAP_ChannelHandle)pParm1;
	BDBG_ENTER(BRAP_P_CDBReady_isr);
	BSTD_UNUSED(iParam2);	
	BDBG_MSG(("Decoder CDB Ready interrupt occured for RAP channel %d, DSP context %d",
			hRapCh->uiChannelNo, hRapCh->sModuleHandles.hDspCh->channelIndex));

	
	/* Call the application callback function */
	if(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCdbReady].pfAppCb) {
		hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCdbReady].pfAppCb (
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCdbReady].pParm1,
			hRapCh->sAppIntCbInfo[BRAP_Interrupt_eCdbReady].iParm2,
			NULL);
	}
	
	BDBG_LEAVE(BRAP_P_CDBReady_isr);
	return;

}
#endif


