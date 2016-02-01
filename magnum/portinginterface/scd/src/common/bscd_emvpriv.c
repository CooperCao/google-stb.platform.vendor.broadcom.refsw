/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * Module Description: This file contains Broadcom smart card Porting 
 *                     Interface EMV private functions.  
 *                    			                    
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#if (BCHP_CHIP!=7125)||((BCHP_CHIP==7125)&& (BCHP_VER>=BCHP_VER_C0))
#include "bchp_sca.h"
#else
#include "bchp_scb.h"
#endif
#include "bscd.h"
#include "bscd_priv.h"
#include "bscd_emvpriv.h"

BDBG_MODULE(BSCD);

BERR_Code BSCD_Channel_P_EMVATRReadNextByte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char *outp_ucData, 
		unsigned long *outp_ulTotalAtrByteTimeInETU, 
		unsigned char *outp_ucParityErrorDetected 
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	uint32_t		      ulValue;

	BDBG_ENTER(BSCD_Channel_P_EMVATRReadNextByte);
	BSCD_P_CHECK_ERR_CODE_CONDITION( errCode, BSCD_STATUS_FAILED, 
		(in_channelHandle->ulRxLen  ==  BSCD_MAX_ATR_SIZE ) );

#if 1


/*EMV2000*/
	if (in_channelHandle->currentChannelSettings.scStandard == BSCD_Standard_eEMV2000) {
		BSCD_P_CHECK_ERR_CODE_FUNC2(errCode, BSCD_STATUS_DEACTIVATE,
			BSCD_Channel_P_EMVATRByteRead(in_channelHandle, outp_ucData, 
			BSCD_MAX_ETU_PER_ATR_BYTE_EMV2000,
			BSCD_MAX_EMV_ETU_FOR_ALL_ATR_BYTES_EMV2000, 
			outp_ulTotalAtrByteTimeInETU));		
	}

	else { /* EMV 96 */
		BSCD_P_CHECK_ERR_CODE_FUNC2(errCode, BSCD_STATUS_DEACTIVATE,
			BSCD_Channel_P_EMVATRByteRead(in_channelHandle, outp_ucData, 
			BSCD_MAX_ETU_PER_ATR_BYTE,
			BSCD_MAX_EMV_ETU_FOR_ALL_ATR_BYTES, 
			outp_ulTotalAtrByteTimeInETU));	
	}

#endif

#if 0
	errCode = BERR_TRACE(BSCD_Channel_P_EMVATRByteRead(in_channelHandle, outp_ucData, 
			BSCD_MAX_ETU_PER_ATR_BYTE,
			BSCD_MAX_EMV_ETU_FOR_ALL_ATR_BYTES, 
			outp_ulTotalAtrByteTimeInETU));
	BDBG_MSG(("In  atqwe errCode = %d\n", errCode));  
	if (errCode != BERR_SUCCESS) 
	{										
	 	BDBG_MSG(("In  BSCD_P_CHECK_ERR_CODE_CONDITION errCode = %d\n", errCode));  	
		errCode = BSCD_STATUS_DEACTIVATE;	
	 	BDBG_MSG(("After  BSCD_P_CHECK_ERR_CODE_CONDITION errCode = %d\n", errCode));  			
		goto BSCD_P_DONE_LABEL;							
	}
#endif	
	
#if 0
	if ( (errCode = BSCD_Channel_P_EMVATRByteRead(in_channelHandle, outp_ucData, 
			BSCD_MAX_ETU_PER_ATR_BYTE,
			BSCD_MAX_EMV_ETU_FOR_ALL_ATR_BYTES, 
			outp_ulTotalAtrByteTimeInETU)) != BERR_SUCCESS) {

		BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_DEACTIVATE, true);	
	}
#endif	

        BDBG_MSG(("After EMVATRByteRead errCode = %d\n", errCode));
		
	ulValue = BREG_Read32( 
		in_channelHandle->moduleHandle->regHandle, 
		(in_channelHandle->ulRegStartAddr + BSCD_P_STATUS_2));	

	/* 
		If a parity error is detected, then continue to read in entire 
		ATR, but immediately after doing so, return BSCD_STATUS_FAILED to 
		deactivate as per EMV spec 
	*/
	if ((ulValue & BCHP_SCA_SC_STATUS_2_rpar_err_MASK) == 
						BCHP_SCA_SC_STATUS_2_rpar_err_MASK) {
						
		BDBG_MSG(("Parity Error found in ATR byte"));
		/* 
			Clear this retry interrupt so that we could continue reading.
			Test 1726 x=2 need this 
		*/
		BKNI_EnterCriticalSection();
		in_channelHandle->ulIntrStatus1 &= ~BCHP_SCA_SC_INTR_STAT_1_retry_intr_MASK;
		BKNI_LeaveCriticalSection();		
		*outp_ucParityErrorDetected = 1;
	}
BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVATRReadNextByte);
	BDBG_MSG(("Leave  EMVATRReadNextByte errCode = 0x%x\n", errCode));	
	return( errCode );
}


BERR_Code BSCD_Channel_P_EMVATRCheckForAdditionalATRBytes(
		BSCD_ChannelHandle	in_channelHandle
)
{
	BERR_Code		errCode = BERR_SUCCESS;
#ifdef BSCD_EMV2000_CWT_PLUS_4_EVENT_INTR	
	uint32_t		ulVal;
#endif	
	unsigned char       bval;
	unsigned long       ulTotalAtrByteTimeInETU = 0;
	unsigned int        unAdditionalByteCount = 0;

	
	BDBG_MSG(("In  BSCD_Channel_P_EMVATRCheckForAdditionalATRBytes\n"));	
	while (errCode == BERR_SUCCESS) {

		/* 
			Since GP is used for total ATR time and WWT is used for WWT, we use
			event2_intr for this 200 ETU checking
		*/

#ifdef BSCD_EMV2000_CWT_PLUS_4_EVENT_INTR		
		/* 200 ETU */
		BREG_Write32( 
			in_channelHandle->moduleHandle->regHandle, 
			(in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMP), 
			 200); 

		/* start event src */
		BREG_Write32( 
			in_channelHandle->moduleHandle->regHandle, 
			(in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMD_3), 
			 BSCD_P_RX_ETU_TICK_EVENT_SRC); 	

		/* increment event src */
		BREG_Write32( 
			in_channelHandle->moduleHandle->regHandle, 
			(in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMD_2), 
			 BSCD_P_RX_ETU_TICK_EVENT_SRC); 			

		/* reset event src */
		BREG_Write32( 
			in_channelHandle->moduleHandle->regHandle, 
			(in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMD_1), 
			 BSCD_P_NO_EVENT_EVENT_SRC); 

		/* event_en, intr_mode, run_after_reset and run_after_compare*/
		ulVal = BCHP_SCA_SC_EVENT2_CMD_4_event_en_MASK | 
				BCHP_SCA_SC_EVENT2_CMD_4_intr_after_compare_MASK |
				BCHP_SCA_SC_EVENT2_CMD_4_run_after_reset_MASK |
				BCHP_SCA_SC_EVENT2_CMD_4_run_after_compare_MASK;
		
		ulVal &= ~(BCHP_SCA_SC_EVENT2_CMD_4_intr_after_reset_MASK);
		BREG_Write32( 
			in_channelHandle->moduleHandle->regHandle, 
			(in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMD_4), 
			 ulVal); 	


		BSCD_Channel_EnableIntrCallback_isr (
							in_channelHandle, BSCD_IntType_eEvent2Int,
				               		BSCD_Channel_P_Event2CB_isr);		

#endif

		errCode = BSCD_Channel_P_EMVATRByteRead(in_channelHandle, &bval, 200, 
					BSCD_MAX_EMV_ETU_FOR_ALL_ATR_BYTES, 
					&ulTotalAtrByteTimeInETU);

#ifdef BSCD_EMV2000_CWT_PLUS_4_EVENT_INTR
			/* Disable event2 */
			ulVal = BREG_Read32( 
				in_channelHandle->moduleHandle->regHandle, 
				(in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMD_4));
			ulVal &= ~(BCHP_SCA_SC_EVENT2_CMD_4_event_en_MASK);
			BREG_Write32( 
				in_channelHandle->moduleHandle->regHandle, 
				(in_channelHandle->ulRegStartAddr + BSCD_P_EVENT2_CMD_4), 
				 ulVal); 
#endif				 
			
		if (errCode == BERR_SUCCESS) {
			BDBG_MSG(("In  SmartCardATRCheckForAdditionalATRBytes: Extra Byte Detected\n"));    
			unAdditionalByteCount++;
		}
	}
			

	if (unAdditionalByteCount)
		return BSCD_STATUS_FAILED;
	else    	
		return BERR_SUCCESS;
}



BERR_Code BSCD_Channel_P_EMVATRReceiveAndDecode(
		BSCD_ChannelHandle	in_channelHandle
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	/* unsigned char         ucTDCount = 0; */
	unsigned char         ucFfactor = 1, ucDFactor = 1;

	/* Used to store value of T0, and TDx's as applicable */

	/* unsigned char         ucGenChecksum = 0;
	unsigned int          unGuardTime = 0; */
	unsigned int          unNumHistoricalBytes = 0;

	unsigned char         ucT0Byte = 0, ucTD1Byte = 0, ucTD2Byte = 0, ucTD3Byte = 0, ucTC1Byte = 0;
	unsigned long         ulTotalAtrByteTimeInETU = 0;
	unsigned char         ucParityErrorDetected = 0;
	unsigned int          i;
	bool       bFailedATR = false;
	/* bool		bInvalidTS = false; */
	bool        bTCKRequired = false;   
	uint32_t	ulValue;

	BSCD_Timer 		timer = {BSCD_TimerType_eWaitTimer, {BSCD_GPTimerMode_eIMMEDIATE}, false, true};	
	BSCD_Timer 		wwtTimer = {BSCD_TimerType_eWaitTimer, {BSCD_GPTimerMode_eIMMEDIATE}, false, false};	
	

	BDBG_ENTER(BSCD_Channel_P_EMVATRReceiveAndDecode);
	
	in_channelHandle->ulRxLen = 0;
	errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
	if (errCode == BSCD_STATUS_DEACTIVATE) {
		errCode = BERR_TRACE(errCode);	
		goto BSCD_P_DONE_LABEL;			
	}

	BSCD_P_CHECK_ERR_CODE_FUNC2(errCode, BSCD_STATUS_DEACTIVATE,
				BSCD_Channel_P_EMVValidateTSByte(
					in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]));	

	errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
					&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
				        &ulTotalAtrByteTimeInETU, 
				        &ucParityErrorDetected);

	BDBG_MSG(("After T0 EMVATRReadNextByte errCode = 0x%x\n", errCode));
	if (errCode == BSCD_STATUS_DEACTIVATE) {
		errCode = BERR_TRACE(errCode);	
		goto BSCD_P_DONE_LABEL;			
	}

	errCode = BSCD_Channel_P_EMVValidateT0Byte(
					in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen],
					&unNumHistoricalBytes);
	ucT0Byte = in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++];
	if (errCode == BSCD_STATUS_FAILED)
		bFailedATR = true;

	if (ucT0Byte  & 0x10 ) { /* TA1 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		BDBG_MSG(("After T0 EMVATRReadNextByte errCode = 0x%x\n", errCode));	
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;			
		}

		errCode= BSCD_Channel_P_EMVValidateTA1Byte(in_channelHandle,
		           in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;

		/* 
			Since BSCD_Channel_P_EMVValidateTA1Byte passed successfully at this point, 
			we may obtain local copies of the F & D values for future use 
		*/
		ucFfactor = in_channelHandle->currentChannelSettings.ucFFactor;
		ucDFactor = in_channelHandle->currentChannelSettings.ucDFactor;


	} 

	else {
		/* 
		If TA1 is absent, use ucFfactor = 1, ucDFactor = 1, 
		which corresponds to D = 1, F = 372 
		*/ 
		/* 
			If TA1 is absent, use ucFfactor = 1, ucDFactor = 1, 
			which corresponds to D = 1, F = 372 
		*/ 		
		in_channelHandle->currentChannelSettings.ucFFactor = ucFfactor;
		in_channelHandle->currentChannelSettings.ucDFactor = ucDFactor;

	}

	if (ucT0Byte & 0x20) {  /* TB1 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;			
		}

		errCode= BSCD_Channel_P_EMVValidateTB1Byte(in_channelHandle,
		           in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;
	}

	else if (in_channelHandle->resetType == BSCD_ResetType_eCold) {

		/* Cold Reset must have a TB1 value specified for EMV */
		BDBG_ERR (("Cold Reset & Missing TB1 - not allowed for EMV\n"));
		errCode = BERR_TRACE(BSCD_STATUS_FAILED);			
		bFailedATR = true;
	}

	if (ucT0Byte & 0x40) {  /* TC1 is present */
		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;			
		}

		ucTC1Byte =  in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++];

	}

	else { 
		/* TC1 is absent from the ATR, so simply program guardtime of zero ETUs */
		in_channelHandle->currentChannelSettings.extraGuardTime.ulValue  = 0;
		in_channelHandle->currentChannelSettings.extraGuardTime.unit = BSCD_TimerUnit_eETU;		
		BREG_Write32( 
			in_channelHandle->moduleHandle->regHandle, 
			(in_channelHandle->ulRegStartAddr + BSCD_P_TGUARD), 
			in_channelHandle->currentChannelSettings.extraGuardTime.ulValue);

		BDBG_MSG(("\nSmartCardValidateTC1Byte: ulGuardTime = 0x%x \n", 
				in_channelHandle->currentChannelSettings.extraGuardTime.ulValue));
	}

	if (ucT0Byte & 0x80) {  /* TD1 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;			
		}

		ucTD1Byte = in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++];  /* Store ucTD1Byte */

		errCode= BSCD_Channel_P_EMVValidateTD1Byte(in_channelHandle,
		           ucTD1Byte, &bTCKRequired);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;

	}


	/* Check TC1 here, so that we can take care of TC1=255 and T=1 case */	
	if (ucTC1Byte != 0) {
		errCode= BSCD_Channel_P_EMVValidateTC1Byte(in_channelHandle,
		           ucTC1Byte);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;
	}
	

	if (ucTD1Byte & 0x10) {  /* TA2 is present */
		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;			
		}	   	

		errCode= BSCD_Channel_P_EMVValidateTA2Byte(
		           in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;
	}
	else {
		/* If TA1 is present and TA2 is absent, we should use F=1 and D=372 */
		in_channelHandle->currentChannelSettings.ucFFactor = ucFfactor = 1;
		in_channelHandle->currentChannelSettings.ucDFactor = ucDFactor = 1;		
	}

        /* EMV 2000 */
        in_channelHandle->currentChannelSettings.ucEtuClkDiv = 
				BSCD_P_GetETUClkDiv(in_channelHandle, in_channelHandle->currentChannelSettings.ucDFactor,
									  in_channelHandle->currentChannelSettings.ucFFactor);

	in_channelHandle->currentChannelSettings.ucScClkDiv = 
						BSCD_P_GetClkDiv(in_channelHandle, in_channelHandle->currentChannelSettings.ucDFactor,
										  in_channelHandle->currentChannelSettings.ucFFactor) ;		

	in_channelHandle->currentChannelSettings.unPrescale = 
		BSCD_P_GetPrescale(in_channelHandle, in_channelHandle->currentChannelSettings.ucDFactor,
						  in_channelHandle->currentChannelSettings.ucFFactor)  *
			in_channelHandle->currentChannelSettings.ucExternalClockDivisor + 
			(in_channelHandle->currentChannelSettings.ucExternalClockDivisor - 1);

	in_channelHandle->currentChannelSettings.ucBaudDiv = 
			BSCD_P_GetBaudDiv(in_channelHandle, in_channelHandle->currentChannelSettings.ucDFactor,
						  in_channelHandle->currentChannelSettings.ucFFactor)  ;		
	
	in_channelHandle->currentChannelSettings.currentBaudRate =
			in_channelHandle->moduleHandle->currentSettings.moduleClkFreq.ulClkFreq /
					in_channelHandle->currentChannelSettings.ucEtuClkDiv/
					(in_channelHandle->currentChannelSettings.unPrescale+1)/
					in_channelHandle->currentChannelSettings.ucBaudDiv;

	/* Call SmartCardFDAdjust to adjust clock and bit rate. */
	BSCD_P_CHECK_ERR_CODE_FUNC(errCode, 
		BSCD_P_AdjustWWT(in_channelHandle, ucFfactor, ucDFactor, 
			BSCD_ISO_DEFAULT_WORK_WAIT_TIME_INTEGER));
	

	if (ucTD1Byte & 0x20) {  /* TB2 is present */
		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;			
		}	 	

		errCode= BSCD_Channel_P_EMVValidateTB2Byte(
		           in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;
		
	}

	if (ucTD1Byte & 0x40) { /* TC2 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;			
		}

		errCode= BSCD_Channel_P_EMVValidateTC2Byte(in_channelHandle,
		           in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;
	}

	else { /* TC2 is absent, so use default WI = 10 */

		BSCD_P_CHECK_ERR_CODE_FUNC(errCode, 
			BSCD_P_AdjustWWT(in_channelHandle, 
				in_channelHandle->currentChannelSettings.ucFFactor, 
				in_channelHandle->currentChannelSettings.ucDFactor,
				BSCD_ISO_DEFAULT_WORK_WAIT_TIME_INTEGER));
		

	}

/*EMV2000*/
/*
	if (in_channelHandle->currentChannelSettings.scStandard == BSCD_Standard_eEMV2000)
		timeValue.ulValue = in_channelHandle->currentChannelSettings.workWaitTime.ulValue
						+ in_channelHandle->currentChannelSettings.ucDFactor * 
						BSCD_DEFAULT_EXTRA_WORK_WAITING_TIME_EMV2000;
	else		
*/
/* BSYT???:  May not need this */

/*
	timeValue.ulValue = in_channelHandle->currentChannelSettings.workWaitTime.ulValue;		
	timer.eTimerType = BSCD_TimerType_eWaitTimer;				
	timer.timerMode.eWaitTimerMode = BSCD_WaitTimerMode_eWorkWaitTime;
	BSCD_P_CHECK_ERR_CODE_FUNC(errCode, 
			BSCD_Channel_ConfigTimer(in_channelHandle, &timer, &timeValue));	
*/

	if (ucTD1Byte & 0x80) {  /* TD2 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;			
		}

		ucTD2Byte = in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++];  /* Store ucTD1Byte */

		errCode= BSCD_Channel_P_EMVValidateTD2Byte(in_channelHandle, ucTD2Byte,
		           ucTD1Byte, &bTCKRequired);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;
	}

	if (ucTD2Byte & 0x10) {  /* TA3 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;	
		}

		errCode= BSCD_Channel_P_EMVValidateTA3Byte(in_channelHandle,
		           in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;
	}

	else {

		/*
			Continue card session using value of '0x20'
			for TA3 as per EMV'96 spec
		*/
		in_channelHandle->currentChannelSettings.unCurrentIFSD = BSCD_DEFAULT_EMV_INFORMATION_FIELD_SIZE;		

	}
		
	if (ucTD2Byte & 0x20) { /* TB3 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;	
		}   	
		
		errCode= BSCD_Channel_P_EMVValidateTB3Byte(in_channelHandle,
		           in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++],
				in_channelHandle->currentChannelSettings.ucFFactor, 
				in_channelHandle->currentChannelSettings.ucDFactor	);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;

	}

	else { 
		/* 
		if TB3 is absent, then we must be in T = 0 mode, 
		otherwise reject ATR 
		*/
		if (in_channelHandle->currentChannelSettings.eProtocolType == BSCD_AsyncProtocolType_e1) {
			BDBG_MSG(("Failing in SmartCardEMVATRDecode - TB3 \n"));

			/* Cannot just return BSCD_STATUS_FAILED. Test 1800 needs to read the TCK */  
			bFailedATR = true;
		}
	}

	if (ucTD2Byte & 0x40) { /* TC3 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;	
		}   

		errCode= BSCD_Channel_P_EMVValidateTC3Byte(
		           in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;
	}


/* EMV 2000 */
	if (ucTD2Byte & 0x80) {  /* TD3 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;			
		}

		ucTD3Byte = in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++];  /* Store ucTD1Byte */

		/* 
		errCode= BSCD_Channel_P_EMVValidateTD3Byte(in_channelHandle, ucTD2Byte,
		           ucTD1Byte, &bTCKRequired);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true; 
		*/
	}

	if (ucTD3Byte & 0x10) {  /* TA4 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;	
		}

		/*
		errCode= BSCD_Channel_P_EMVValidateTA4Byte(in_channelHandle,
		           in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;
		*/
	}

	if (ucTD3Byte & 0x20) { /* TB4 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;	
		}   	

		/*
		errCode= BSCD_Channel_P_EMVValidateTB4Byte(in_channelHandle,
		           in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++],
				in_channelHandle->currentChannelSettings.ucFFactor, 
				in_channelHandle->currentChannelSettings.ucDFactor	);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;
		*/

	}

	if (ucTD3Byte & 0x40) { /* TC4 is present */

		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;	
		}   

		/*
		errCode= BSCD_Channel_P_EMVValidateTC3Byte(
		           in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]);
		if (errCode == BSCD_STATUS_FAILED)
			bFailedATR = true;
		*/
	}


/* EMV 2000 */

	if (unNumHistoricalBytes) {

		for (i = 0; i < unNumHistoricalBytes; i++) {

			errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
						&(in_channelHandle->aucRxBuf[(in_channelHandle->ulRxLen)++]), 
					        &ulTotalAtrByteTimeInETU, 
					        &ucParityErrorDetected);
			if (errCode == BSCD_STATUS_DEACTIVATE) {
					errCode = BERR_TRACE(errCode);	
					goto BSCD_P_DONE_LABEL;	
			} 			
		   
		}
	}

	if (bTCKRequired) {
		BDBG_MSG (("Checking for TCK Byte"));  	
		errCode = BSCD_Channel_P_EMVATRReadNextByte(in_channelHandle, 
					&(in_channelHandle->aucRxBuf[in_channelHandle->ulRxLen]), 
				        &ulTotalAtrByteTimeInETU, 
				        &ucParityErrorDetected);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;	
		} 

		/* if in T = 1 mode, then TCK byte must be present and validated */
		errCode = BSCD_Channel_P_EMVValidateTCKByte(in_channelHandle, 
				in_channelHandle->aucRxBuf,  (in_channelHandle->ulRxLen)++);
		if (errCode == BSCD_STATUS_DEACTIVATE) {
			errCode = BERR_TRACE(errCode);	
			goto BSCD_P_DONE_LABEL;	
		} 
	
	}

	/* 
	Only after reading entire ATR, now if a parity error has been 
	detected, return BSCD_STATUS_FAILED 
	*/
	if (ucParityErrorDetected)
		return BSCD_STATUS_DEACTIVATE;


	errCode = BSCD_Channel_P_EMVATRCheckForAdditionalATRBytes(in_channelHandle);
	if (errCode == BSCD_STATUS_FAILED) {
		errCode = BERR_TRACE(errCode);	
		goto BSCD_P_DONE_LABEL;			
	}

	/* Disable timer, which was enable upon receiving atr_intr */
	BSCD_Channel_EnableDisableTimer_isr(in_channelHandle, &timer);	

	/* disable WWT.  This was enabled in activating time */
	wwtTimer.timerMode.eWaitTimerMode = BSCD_WaitTimerMode_eWorkWaitTime;			
	BSCD_Channel_EnableDisableTimer_isr(in_channelHandle, &wwtTimer);					
			
	/* Print ATR message. */
	BSCD_P_HexDump("ATR Data",
		in_channelHandle->aucRxBuf, 
		in_channelHandle->ulRxLen); 

	if (bFailedATR)
		BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);
	
	BSCD_P_CHECK_ERR_CODE_FUNC(errCode,
		BSCD_P_FDAdjust(in_channelHandle, ucFfactor, ucDFactor));
	
	if ( (errCode = BSCD_Channel_EnableInterrupts(in_channelHandle)) != BERR_SUCCESS) {
		errCode = BERR_TRACE(errCode);	
		goto BSCD_P_DONE_LABEL;			
	}
	
	/* Update the BSCD_P_UART_CMD_2 here.  If we need to update more, make this a function */
	ulValue = BREG_Read32( 
		in_channelHandle->moduleHandle->regHandle, 
		(in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_2));
	BDBG_MSG(("orig EMVATRReceiveAndDecode: BSCD_P_UART_CMD_2 = 0x%x\n", 	ulValue));
	
	ulValue  &=  (BCHP_SCA_SC_UART_CMD_2_convention_MASK);
	
	if (in_channelHandle->currentChannelSettings.eProtocolType == BSCD_AsyncProtocolType_e0 ) {
		
		ulValue |= (in_channelHandle->currentChannelSettings.ucRxRetries << BCHP_SCA_SC_UART_CMD_2_rpar_retry_SHIFT) | 
				(in_channelHandle->currentChannelSettings.ucTxRetries);
	}
	else if ( (in_channelHandle->currentChannelSettings.eProtocolType == BSCD_AsyncProtocolType_e1 )  ||
			(in_channelHandle->currentChannelSettings.eProtocolType == BSCD_AsyncProtocolType_e14_IRDETO ) ) {
		/* No OP */ ;
	}
   	BREG_Write32( 
		in_channelHandle->moduleHandle->regHandle, 
		(in_channelHandle->ulRegStartAddr + BSCD_P_UART_CMD_2), 
		ulValue);

	BDBG_MSG(("EMVATRReceiveAndDecode: BSCD_P_UART_CMD_2 = 0x%x\n", 	ulValue));


	
BSCD_P_DONE_LABEL:


	/* Disable timer, which was enable upon receiving atr_intr */
	BSCD_Channel_EnableDisableTimer_isr(in_channelHandle, &timer);	

	/* disable WWT.  This was enabled in activating time */
	wwtTimer.timerMode.eWaitTimerMode = BSCD_WaitTimerMode_eWorkWaitTime;			
	BSCD_Channel_EnableDisableTimer_isr(in_channelHandle, &wwtTimer);	
	
	BDBG_MSG(("Leave EMVATRReceiveAndDecode  errCode = 0x%x\n", errCode));	
	BDBG_LEAVE(BSCD_Channel_P_EMVATRReceiveAndDecode);
	return( errCode );
	
}

BERR_Code BSCD_Channel_P_EMVValidateTSByte(
		unsigned char in_ucTSByte
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	
	if (in_ucTSByte == 0x3f) {
		BDBG_MSG(("TS = %02x, Inverse Convention\n", in_ucTSByte));
		return BERR_SUCCESS;
	} 

	else if (in_ucTSByte == 0x3b) {
		BDBG_MSG(("TS = %02x, Direct Convention\n", in_ucTSByte));
		return BERR_SUCCESS;
	} 

	else {
		BDBG_ERR(("TS = %02x, Unknown Convention\n", in_ucTSByte));
		errCode = BERR_TRACE(BSCD_STATUS_FAILED);
		return BSCD_STATUS_FAILED;
	}
}

BERR_Code BSCD_Channel_P_EMVValidateT0Byte(
      unsigned char ucT0Byte, 
      unsigned int  *unNumHistoricalBytes
)
{
	*unNumHistoricalBytes = (unsigned int)(ucT0Byte & 0x0F);
	return BERR_SUCCESS;
}

BERR_Code BSCD_Channel_P_EMVValidateTA1Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTA1Byte
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	unsigned char       ucFfactor, ucDFactor;

	BDBG_ENTER(BSCD_Channel_P_EMVValidateTA1Byte);

	BDBG_MSG(("in_ucTA1Byte = %d\n", in_ucTA1Byte));
		
	/* Decode TA1 (F and D adjustment). */
	ucFfactor = (in_ucTA1Byte >> 4) & 0x0f;
	ucDFactor = in_ucTA1Byte & 0x0f;

	if ( ((ucFfactor == 1) && (ucDFactor == 1)) ||
	      ((ucFfactor == 1) && (ucDFactor == 2)) ||
	      ((ucFfactor == 1) && (ucDFactor == 3)) ) {
	      
		/* 
			 FOR EMV ONLY 
			 Acceptable FI value is 1, acceptable DI value is 1, 2, or 3 - 
		*/
		in_channelHandle->currentChannelSettings.ucFFactor = ucFfactor;
		in_channelHandle->currentChannelSettings.ucDFactor = ucDFactor;
		BDBG_MSG(("Clock rate conversion factor, ucFfactor = %1d\n",ucFfactor));
		BDBG_MSG(("Bit rate adjustment factor, ucDFactor = %1d\n",ucDFactor));
	}

	else {
		BDBG_ERR(("Invalid adjustment factor = %02x\n",in_ucTA1Byte));
		BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);
	}
	
BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVValidateTA1Byte);
	return( errCode );
	
}

BERR_Code BSCD_Channel_P_EMVValidateTB1Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTB1Byte
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	
	BDBG_ENTER((SmartCardValidateTB1Byte));    
	
	/* Decode TB1 (programming voltage) */
	if (in_ucTB1Byte == 0x00) {
		/* 
		BDBG_MSG(("VPP is not connected in the ICC\n"));
		BDBG_MSG(("No programming current\n")); 
		*/
	}

	else {
		/* 
		According to EMV ATR spec, in response to a warm reset, 
		the terminal shall accept an ATR containing TB1 of any value 
		*/
		if (in_channelHandle->resetType == BSCD_ResetType_eWarm) {
			/*
			BDBG_ERR(("VPP is not connected in the ICC\n"));
			BDBG_ERR(("No programming current\n"));
			*/
		}

		else {
			BDBG_ERR(("Non-Zero TB1 = %02x during cold reset.  Not acceptable for EMV.\n",in_ucTB1Byte));
			BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);
		}
	}

	
BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVValidateTB1Byte);
	return( errCode );
	
}


BERR_Code BSCD_Channel_P_EMVValidateTC1Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTC1Byte
)
{	
	BDBG_MSG(("In  SmartCardValidateTC1Byte\n"));      

	/* Decode TC1 (guard time) */
	if ( (in_channelHandle->currentChannelSettings.eProtocolType == BSCD_AsyncProtocolType_e0) &&
		(in_ucTC1Byte == 0xff))  {

		/* 
			When in T = 0 mode and ucTC1Byte == 0xff, simply set 
			additional guardtime to zero ETUs.  This is to pass the test and it is different from EMV 96. 
		*/
		in_channelHandle->currentChannelSettings.extraGuardTime.ulValue = 0;

	} 
	else {
		/* 
			use value of ucTC1Byte for additional guardtime, 
			regardless of T = 0 or T = 1 mode 
		*/
		in_channelHandle->currentChannelSettings.extraGuardTime.ulValue = in_ucTC1Byte;
	}
   
	in_channelHandle->currentChannelSettings.extraGuardTime.unit = BSCD_TimerUnit_eETU;
			
	BREG_Write32( 
		in_channelHandle->moduleHandle->regHandle, 
		(in_channelHandle->ulRegStartAddr + BSCD_P_TGUARD), 
		in_channelHandle->currentChannelSettings.extraGuardTime.ulValue);

	BDBG_MSG(("\nSmartCardValidateTC1Byte: ulGuardTime = 0x%x \n", 
			in_channelHandle->currentChannelSettings.extraGuardTime.ulValue));

	return BERR_SUCCESS;
}



BERR_Code BSCD_Channel_P_EMVValidateTD1Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTD1Byte, 
		bool *outp_bTCKRequired
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	
	BDBG_ENTER((BSCD_Channel_P_EMVValidateTD1Byte));      

	if  ((in_ucTD1Byte & 0x0f) > 0x01) {

		/* 
		If the lower nibble of ucTD1Byte is not equal to either 0 
		or 1, then return fail, otherwise return success 
		*/
		BDBG_ERR(("Erroneous TD1 l.s. nibble = %02x, should be either 0x00 or 0x01\n",in_ucTD1Byte));

		*outp_bTCKRequired = true;
		BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);
	} 

	else if ((in_ucTD1Byte & 0x0f) == 0x01) {

		/* set protocol type to T = 1 */
		in_channelHandle->currentChannelSettings.eProtocolType = BSCD_AsyncProtocolType_e1;

		*outp_bTCKRequired = true;
	}

BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVValidateTD1Byte);
	return( errCode );
}

BERR_Code BSCD_Channel_P_EMVValidateTA2Byte(
		unsigned char in_ucTA2Byte
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	
	BDBG_ENTER((SmartCardValidateTA2Byte));      	

	if ((in_ucTA2Byte & 0x10) != 0) {
		BDBG_ERR(("Invalid TA2 = %02x \n",in_ucTA2Byte));
		BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);
	}

BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVValidateTA2Byte);
	return( errCode );
}

BERR_Code BSCD_Channel_P_EMVValidateTB2Byte(
		unsigned char in_ucTB2Byte
)
{
	BSTD_UNUSED(in_ucTB2Byte);
	BDBG_MSG(("In  SmartCardValidateTB2Byte\n"));

	/* TB2 is not supported by EMV, therefore return failed */
	BDBG_ERR(("TB2 is present, but not required for Europay standard.  Invalid ATR\n"));

	return BSCD_STATUS_FAILED;
}

BERR_Code BSCD_Channel_P_EMVValidateTC2Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTC2Byte		
)
{
	BERR_Code		errCode = BERR_SUCCESS;

	BDBG_ENTER((SmartCardValidateTC2Byte));
	BDBG_MSG(("In  BSCD_Channel_P_EMVValidateTC2Byte\n"));	

	/* Decode TC2.  NOTE: TC2 is specific to protocol type T = 0 */
	if ((in_ucTC2Byte == 0x00) || (in_ucTC2Byte > 0x0A)) {

		/* Reject ATR if TC2 is equal to '0x00' or greater than '0x0A' */
		BDBG_ERR(("Invalid TC2 = %02x \n",in_ucTC2Byte));
		BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);
	}

	else {
		/* Reset work waiting time, using valid TC2 Value */
		/* Specify work wait time used for T = 0 protocol */
		BSCD_P_CHECK_ERR_CODE_FUNC(errCode, 
			BSCD_P_AdjustWWT(in_channelHandle, 
				in_channelHandle->currentChannelSettings.ucFFactor, 
				in_channelHandle->currentChannelSettings.ucDFactor,
				in_ucTC2Byte));		

	}


BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVValidateTC2Byte);
	return( errCode );
}

BERR_Code BSCD_Channel_P_EMVValidateTD2Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTD2Byte, 
		unsigned char in_ucTD1Byte, 
		bool * outp_bTCKRequired
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	
	BDBG_ENTER((BSCD_Channel_P_EMVValidateTD2Byte));

	BSTD_UNUSED(in_channelHandle);
	if ((in_ucTD1Byte & 0x0F) == 0x00) {

		if (((in_ucTD2Byte & 0x0F) != 0x0E) && ((in_ucTD2Byte & 0x0F) != 0x01)) {
			/* 
			If l.s. nibble of TD1 is '0', then l.s. nibble of TD2 
			must equal '0xE' or 0x01 
			*/ 
			BDBG_ERR(("Failing in SmartCardValidateTD2Byte - TD2 != 0x0E \n"));			
			BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);

		} 

		else {
			*outp_bTCKRequired = true;
			goto BSCD_P_DONE_LABEL;
		}  
	} 

	else if ((in_ucTD1Byte & 0x0F) == 0x01) {

		if ((in_ucTD2Byte & 0x0F) != 0x01) {

			/* 
			If l.s. nibble of TD1 is '1', then l.s. nibble of TD2 must 
			equal '0x01'
			*/
			BDBG_MSG(("Failing in SmartCardValidateTD2Byte - TD2 != 0x01 \n"));
			BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);
		}
	}

BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVValidateTD2Byte);
	return( errCode );
}


BERR_Code BSCD_Channel_P_EMVValidateTA3Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTA3Byte
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	
	BDBG_ENTER((BSCD_Channel_P_EMVValidateTA3Byte));
	/* Decode TA3. */
	if ((in_ucTA3Byte <= 0x0f) || (in_ucTA3Byte == 0xff))  {

		BDBG_ERR(("Invalid EMV TA3 = %02x \n",in_ucTA3Byte));
		BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);
	}

	else {    		
		in_channelHandle->currentChannelSettings.unCurrentIFSD = in_ucTA3Byte;
	}
	
BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVValidateTA3Byte);
	return( errCode );
	
}

BERR_Code BSCD_Channel_P_EMVValidateTB3Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTB3Byte, 
		unsigned char in_ucFFactor, 
		unsigned char in_ucDFactor
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	uint32_t ulGuardTime, ulGuardTimePlusOne, ulCWTValue, ulBlockWaitTime; 
	unsigned char ucCWIVal, ucBWIVal;
	unsigned char ucBaudDiv, ucClkDiv;
	unsigned int unCWICheck;	

	BDBG_ENTER((BSCD_Channel_P_EMVValidateTB3Byte));

	if (in_channelHandle->currentChannelSettings.eProtocolType == BSCD_AsyncProtocolType_e1) {

		/* Decode TB3. */
		ucCWIVal = in_ucTB3Byte & 0x0f;
		ucBWIVal = (in_ucTB3Byte >> 4) & 0x0f;

		/*
			Obtain the power(2,CWI) factor from the value of
			CWI - see TB3 in EMV'96 spec for more
		*/
		if (ucCWIVal == 0x00) {
			unCWICheck = 1;
		}
		else {
			unCWICheck = 2<<(ucCWIVal-1);
		}

		/* Obtain guard time for comparison check below */
		ulGuardTime = in_channelHandle->currentChannelSettings.extraGuardTime.ulValue ;	
		
		/* 
		Note: if ulGuardTime = 0xFF, then unGuardTimePlusOne = 0, 
		according to EMV ATR rules P. I-28 
		*/
		if (ulGuardTime == 0xff)
			ulGuardTimePlusOne = 0;
		else
			ulGuardTimePlusOne = ulGuardTime + 1;


		/* 
		Terminal shall reject ATR not containing TB3, or 
		containing TB3 indicating BWI greater than 4 and/or 
		CWI greater than 5, or power(2,CWI) < (N+1), where 
		N is value of TC1 
		*/
		/*EMV2000*/
		if ((ucCWIVal > 5) || (ucBWIVal > 4) || 
				(unCWICheck <= ulGuardTimePlusOne)) {

			BDBG_MSG (("ucCWIVal = 0x%02x, ucBWIVal = 0x%02x, unCWICheck = %02d, guard_time = 0x%02x, ulGuardTimePlusOne=%ld\n", 
					ucCWIVal, ucBWIVal, unCWICheck, ulGuardTime, ulGuardTimePlusOne));         		
			BDBG_ERR(("Invalid TB3 = %02x \n",in_ucTB3Byte));
			BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);			

		} 

		else {


			/* Set CWT */
			ulCWTValue = BREG_Read32( 
				in_channelHandle->moduleHandle->regHandle, 
				(in_channelHandle->ulRegStartAddr + BSCD_P_PROTO_CMD));

			/*
			and with 0xf0 to remove the original 0x0f inserted
			into this register
			*/
			ulCWTValue &= 0xf0;
			ulCWTValue |= ucCWIVal;
			in_channelHandle->currentChannelSettings.ulCharacterWaitTimeInteger = ucCWIVal;


#ifdef BSCD_EMV2000_CWT_PLUS_4
		  if ( (in_channelHandle->currentChannelSettings.scStandard != BSCD_Standard_eEMV2000) ||
		  	(in_channelHandle->currentChannelSettings.eProtocolType != BSCD_AsyncProtocolType_e1)) {
#endif
	 		BREG_Write32( 
				in_channelHandle->moduleHandle->regHandle, 
				(in_channelHandle->ulRegStartAddr + BSCD_P_PROTO_CMD), 
				ulCWTValue);

#ifdef BSCD_EMV2000_CWT_PLUS_4
		  }
#endif

			BDBG_MSG(("ulCWTValue = 0x%x\n", ulCWTValue));
			

			/* set BWT */
			/* 
				The block waiting time is encoded as described in ISO 7816-3, 
				repeated here in the following equation:

				BWT = [11 + 2 bwi x 960 x 372 x D/F] etu

				e.g If bwi = 4 and F/D = 372 then BWT = 15,371 etu. 
				The minimum and maximum BWT are ~186 and 15,728,651 etu.
			*/
			in_channelHandle->currentChannelSettings.unPrescale = 
				BSCD_P_GetPrescale(in_channelHandle, in_ucDFactor, in_ucFFactor) *
					in_channelHandle->currentChannelSettings.ucExternalClockDivisor + 
					(in_channelHandle->currentChannelSettings.ucExternalClockDivisor - 1);


			ucBaudDiv = BSCD_P_GetBaudDiv(in_channelHandle, in_ucDFactor, in_ucFFactor);
			ucClkDiv = BSCD_P_GetClkDiv(in_channelHandle, in_ucDFactor, in_ucFFactor);

			if (ucBWIVal == 0x00) {

				ulBlockWaitTime = 960 * 372 * ucClkDiv / 
					(in_channelHandle->currentChannelSettings.unPrescale+1) / ucBaudDiv + 11;
			}

			else {

				ulBlockWaitTime = (2<<(ucBWIVal-1)) * 960 *  372 * 
					ucClkDiv / (in_channelHandle->currentChannelSettings.unPrescale+1) / ucBaudDiv  + 11;
			}

			BDBG_MSG (("In SmartCardValidateTB3Byte: unBlockWaitTime = %d, ulClkDiv = 0x%02x\n", 
				ulBlockWaitTime, ucClkDiv));

			BDBG_MSG (("In SmartCardValidateTB3Byte: ulPrescale = 0x%02x, ucBaudDiv = 0x%02x\n", 
				in_channelHandle->currentChannelSettings.unPrescale, ucBaudDiv));
				

			/* Change timer to equal calculated BWT */
			if (in_channelHandle->currentChannelSettings.scStandard == BSCD_Standard_eEMV2000) 
				in_channelHandle->currentChannelSettings.blockWaitTime.ulValue =  ulBlockWaitTime +
					BSCD_P_GetISOBaudRateAdjustor(in_channelHandle->currentChannelSettings.ucDFactor) * 
					BSCD_DEFAULT_EXTRA_BLOCK_WAITING_TIME_EMV2000;		
			else
				in_channelHandle->currentChannelSettings.blockWaitTime.ulValue =  ulBlockWaitTime ;		
			in_channelHandle->currentChannelSettings.blockWaitTime.unit = BSCD_TimerUnit_eETU;			

			BDBG_MSG (("TB3, blockWaitTime = %ld\n", 
				in_channelHandle->currentChannelSettings.blockWaitTime.ulValue));

			/* Need this for MetroWerks */
/*			
			timer.eTimerType = BSCD_TimerType_eWaitTimer;							
			timer.timerMode.eWaitTimerMode = BSCD_WaitTimerMode_eBlockWaitTime;
			
			timeValue.ulValue = in_channelHandle->currentChannelSettings.blockWaitTime.ulValue ;
		
			BSCD_P_CHECK_ERR_CODE_FUNC(errCode, 
					BSCD_Channel_ConfigTimer(in_channelHandle, &timer, &timeValue));
*/
		}
	} /* eProtocolType == BSCD_AsyncProtocolType_e1 */
	
BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVValidateTB3Byte);
	return( errCode );
}

BERR_Code BSCD_Channel_P_EMVValidateTC3Byte(
		unsigned char in_ucTC3Byte
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	
	BDBG_ENTER((BSCD_Channel_P_EMVValidateTC3Byte));
	BDBG_MSG(("In BSCD_Channel_P_EMVValidateTC3Byte = 0x%02x\n", in_ucTC3Byte));	

	/* Terminal shall reject ATR containing non-zero TC3 Byte */
	if (in_ucTC3Byte != 0) {
		BDBG_ERR(("Failing in SmartCardValidateTC3Byte \n"));
		BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);
	}

BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVValidateTC3Byte);
	return( errCode );

}

BERR_Code BSCD_Channel_P_EMVValidateTCKByte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char *inp_ucATR, 
		unsigned int in_unATRLength
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	
	unsigned char ucTCKCompare = 0;
	unsigned int i;

	BDBG_ENTER((BSCD_Channel_P_EMVValidateTCKByte));
	BSTD_UNUSED(in_channelHandle);
	
	/* Start from T0 to TCK.  Including historical bytes if they exist */
	for (i=1; i<=in_unATRLength; i++) {
		ucTCKCompare = ucTCKCompare ^ inp_ucATR[i];
		BDBG_MSG(("In  SmartCardValidateTCKByte inp_ucATR[%d] = %02x\n", i, inp_ucATR[i]));
	}

	if (ucTCKCompare != 0) {
		BDBG_ERR(("Invalid TCK. \n"));
		BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_DEACTIVATE, true);
	}

	/* TCK validated successfully */
BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVValidateTCKByte);
	return( errCode );
}

BERR_Code BSCD_Channel_P_EMVATRByteRead(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char *inoutp_ucData,
		unsigned long in_ulMaxAtrByteTimeInETU,
		long in_lMaxTotalAtrByteTimeInETU,
		unsigned long *inoutp_ultotalAtrByteTimeInETU 
)
{  
	BERR_Code		errCode = BERR_SUCCESS;
	uint32_t        ulTimerCntVal1, ulTimerCntVal2, ulTimerCntVal;
	uint32_t 			ulStatus;
	BSCD_Timer 		timer = {BSCD_TimerType_eGPTimer, {BSCD_GPTimerMode_eIMMEDIATE}, false, false};	
	/* BSCD_TimerValue    timeValue= {BSCD_MAX_ETU_PER_ATR_BYTE, BSCD_TimerUnit_eETU}; */

	BSCD_Timer 		wwtTimer = {BSCD_TimerType_eWaitTimer, {BSCD_GPTimerMode_eIMMEDIATE}, false, false};	

	BDBG_ENTER(BSCD_Channel_P_EMVATRByteRead);

	BSTD_UNUSED(in_ulMaxAtrByteTimeInETU);
	#ifdef SMARTCARD_32_BIT_REGISTER
	BSTD_UNUSED(ulTimerCntVal2);
	#endif
	BKNI_EnterCriticalSection();	
	ulStatus = in_channelHandle->ulStatus2 = BREG_Read32( 
		in_channelHandle->moduleHandle->regHandle, 
		(in_channelHandle->ulRegStartAddr + BSCD_P_STATUS_2));
	BDBG_MSG(("Status2 = 0x%x\n", ulStatus));
	BKNI_LeaveCriticalSection();

	if ((ulStatus & BCHP_SCA_SC_STATUS_2_rempty_MASK) == BCHP_SCA_SC_STATUS_2_rempty_MASK) {

		/* Do not have any byte in SC_RECEIVE */			
 
		if ( (errCode = BSCD_Channel_P_WaitForRcv(in_channelHandle)) != BERR_SUCCESS) {

			/* disable the timer, always return the  previous error */

			/* Disable timer, which was enable upon receiving atr_intr */
			BSCD_Channel_EnableDisableTimer_isr(in_channelHandle, &timer);	

			/* disable WWT.  This was enabled in activating time */
			wwtTimer.timerMode.eWaitTimerMode = BSCD_WaitTimerMode_eWorkWaitTime;			
			BSCD_Channel_EnableDisableTimer_isr(in_channelHandle, &wwtTimer);	

			/* Read timer counter and accumulate it to *inoutp_ultotalAtrByteTimeInETU */
			#ifndef SMARTCARD_32_BIT_REGISTER
			ulTimerCntVal2 =  BREG_Read32( 
				in_channelHandle->moduleHandle->regHandle, 
				(in_channelHandle->ulRegStartAddr + BSCD_P_TIMER_CNT_2));

			ulTimerCntVal1=  BREG_Read32( 
				in_channelHandle->moduleHandle->regHandle, 
				(in_channelHandle->ulRegStartAddr + BSCD_P_TIMER_CNT_1));
		
			ulTimerCntVal = (((unsigned int) ulTimerCntVal2) << 8) | ulTimerCntVal1;
			#else
			ulTimerCntVal = ulTimerCntVal1=  BREG_Read32(in_channelHandle->moduleHandle->regHandle,
				(in_channelHandle->ulRegStartAddr + BSCD_P_TIMER_CNT_1));
			#endif
			*inoutp_ultotalAtrByteTimeInETU += ulTimerCntVal; 

			if (*inoutp_ultotalAtrByteTimeInETU > (unsigned long) in_lMaxTotalAtrByteTimeInETU) {
				BDBG_MSG(("SmartCardATRByteRead: inoutp_ultotalAtrByteTimeInETU = %lu , in_lMaxTotalAtrByteTimeInETU = %d\n", 
						*inoutp_ultotalAtrByteTimeInETU, in_lMaxTotalAtrByteTimeInETU));	       											
				BSCD_P_CHECK_ERR_CODE_CONDITION(errCode, BSCD_STATUS_FAILED, true);
			}
			
			BDBG_MSG(("After  WaitForRcv in EMVATRByteRead errCode = 0x%x\n", errCode));	
			return errCode;
		}

	}

	else {
		BKNI_EnterCriticalSection();	
		in_channelHandle->ulIntrStatus2 &= ~BCHP_SCA_SC_INTR_STAT_2_rcv_intr_MASK;
		BKNI_LeaveCriticalSection();		
		BDBG_MSG(("Cancel out RCV_INTR, pSc_intrStatus2 = 0x%x\n", 
			in_channelHandle->ulIntrStatus2));
	}

	*inoutp_ucData = (unsigned char) BREG_Read32( 
				in_channelHandle->moduleHandle->regHandle, 
				(in_channelHandle->ulRegStartAddr + BSCD_P_RECEIVE));
			
	BDBG_WRN(("atr = 0x%x\n", *inoutp_ucData));

BSCD_P_DONE_LABEL:
	
	BDBG_LEAVE(BSCD_Channel_P_EMVATRByteRead);
	BDBG_MSG(("Leave EMVATRByteRead errCode = 0x%x\n", errCode));	
	return( errCode );	
}

