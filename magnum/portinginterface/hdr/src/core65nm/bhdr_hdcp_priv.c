
/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bhdr_hdcp.h"
#include "bhdr_priv.h"

BDBG_MODULE(BHDR_HDCP_PRIV) ;

#if (HDMI_RX_GEN == 3548)

#define REGADDR_STATUS_0 BCHP_DVP_HR_DVP_HR_OTP_STATUS_0
#define REGADDR_STATUS_1 BCHP_DVP_HR_DVP_HR_OTP_STATUS_1
#define REGADDR_STATUS_2 BCHP_DVP_HR_DVP_HR_OTP_STATUS_2
#define REGADDR_STATUS_3 BCHP_DVP_HR_DVP_HR_OTP_STATUS_3
#define REGADDR_STATUS_4 BCHP_DVP_HR_DVP_HR_OTP_STATUS_4
#define REGADDR_STATUS_5 BCHP_DVP_HR_DVP_HR_OTP_STATUS_5
#define REGNAME_STATUS_0 DVP_HR_DVP_HR_OTP_STATUS_0
#define REGNAME_STATUS_1 DVP_HR_DVP_HR_OTP_STATUS_1
#define REGNAME_STATUS_2 DVP_HR_DVP_HR_OTP_STATUS_2
#define REGNAME_STATUS_3 DVP_HR_DVP_HR_OTP_STATUS_3
#define REGNAME_STATUS_4 DVP_HR_DVP_HR_OTP_STATUS_4
#define REGNAME_STATUS_5 DVP_HR_DVP_HR_OTP_STATUS_5

#elif (HDMI_RX_GEN == 35230)

#if (BCHP_CHIP == 35233)
#include "bchp_dvp_hr_dvp_otp.h"
#endif 

#define REGADDR_STATUS_0 BCHP_DVP_HR_DVP_OTP_STATUS_0
#define REGADDR_STATUS_1 BCHP_DVP_HR_DVP_OTP_STATUS_1
#define REGADDR_STATUS_2 BCHP_DVP_HR_DVP_OTP_STATUS_2
#define REGADDR_STATUS_3 BCHP_DVP_HR_DVP_OTP_STATUS_3
#define REGADDR_STATUS_4 BCHP_DVP_HR_DVP_OTP_STATUS_4
#define REGADDR_STATUS_5 BCHP_DVP_HR_DVP_OTP_STATUS_5
#define REGNAME_STATUS_0 DVP_HR_DVP_OTP_STATUS_0
#define REGNAME_STATUS_1 DVP_HR_DVP_OTP_STATUS_1
#define REGNAME_STATUS_2 DVP_HR_DVP_OTP_STATUS_2
#define REGNAME_STATUS_3 DVP_HR_DVP_OTP_STATUS_3
#define REGNAME_STATUS_4 DVP_HR_DVP_OTP_STATUS_4
#define REGNAME_STATUS_5 DVP_HR_DVP_OTP_STATUS_5

#else
#error "Unknown/Incorrect HDMI/DVP HR Block for this chip"
#endif

/******************************************************************************
Summary: Enable HDCP Key Loading 
*******************************************************************************/
BERR_Code BHDR_HDCP_P_EnableKeyLoading(BHDR_Handle hHDR) 
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	uint32_t programmedCRC ;
	bool 
		otpCrcCalcDone,
		validRxKsv,
		userWriteLock,
		userReadLock,
		jtagLock ;
	unsigned char   OtpRxBksv[] = { 0x00, 0x01, 0x02, 0x03, 0x04 }; /* LSB...MSB */
	

	BDBG_ENTER(BHDR_HDCP_P_EnableKeyLoading) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	
	BKNI_Memset(&hHDR->stHdcpStatus, 0, sizeof(BHDR_HDCP_Status)) ;
	
	/* HDCP Key Set is always stored in ROM for this platform */
	hHDR->stHdcpStatus.eKeyStorage = BHDR_HDCP_KeyStorage_eOtpROM ;
	
	hHDR->stHdcpStatus.eOtpState = BHDR_HDCP_OtpState_eCalcInitialized ;
	
	/* make sure OTP has been programmed and locked */
	Register = BREG_Read32(hRegister, REGADDR_STATUS_3) ;
	userWriteLock = BCHP_GET_FIELD_DATA(Register, REGNAME_STATUS_3, OTP_USER_LOCK) ;
	userReadLock = BCHP_GET_FIELD_DATA(Register, REGNAME_STATUS_3, OTP_READ_LOCK) ;
	jtagLock = BCHP_GET_FIELD_DATA(Register, REGNAME_STATUS_3, OTP_JTAG_DISABLE) ;
	otpCrcCalcDone = BCHP_GET_FIELD_DATA(Register, REGNAME_STATUS_3, OTP_READY) ;
	
	if ((!otpCrcCalcDone) || (!userWriteLock) || (!userReadLock) || (!jtagLock))
	{
		BDBG_ERR(("??????????????????????????????????????????????????")) ;
		BDBG_WRN(("DVP_HR OTP HDCP has NOT BEEN PROGRAMMED!!!")) ;
		BDBG_WRN((" Write Lock: %d", userWriteLock)) ;
		BDBG_WRN(("  Read Lock: %d", userReadLock)) ;
		BDBG_WRN(("  JTAG Lock: %d", jtagLock)) ;
		BDBG_ERR(("??????????????????????????????????????????????????")) ;
		hHDR->stHdcpStatus.eOtpState =  BHDR_HDCP_OtpState_eHwError ;
		goto done ;
	}
	
	/* check for a CRC value */
	Register = BREG_Read32(hRegister, REGADDR_STATUS_4) ;
	programmedCRC = 
		BCHP_GET_FIELD_DATA(Register, REGNAME_STATUS_4, OTP_CRC) ;

	if (!programmedCRC)
	{
		BDBG_WRN(("DVP_HR OTP CRC value has not been programmed!!!")) ; 	
		hHDR->stHdcpStatus.eOtpState =  BHDR_HDCP_OtpState_eHwError ;
		goto done ;
	}
	
	/* get the BKSV */
	Register = BREG_Read32(hRegister, REGADDR_STATUS_0) ;
	OtpRxBksv[0] =  Register & 0x000000FF ;
	OtpRxBksv[1] = (Register & 0x0000FF00) >>  8 ;
	OtpRxBksv[2] = (Register & 0x00FF0000) >> 16 ;
	OtpRxBksv[3] = (Register & 0xFF000000) >> 24 ;

	Register = BREG_Read32(hRegister, REGADDR_STATUS_1) ;
	OtpRxBksv[4] = BCHP_GET_FIELD_DATA(Register, REGNAME_STATUS_1, OTP_BKSV_1) ;
	
	BKNI_Memcpy(&hHDR->stHdcpStatus.bksvValue, &OtpRxBksv, BAVC_HDMI_HDCP_KSV_LENGTH) ;


	/* check for valid KSV i.e. 20 0s and 20 1s */	
	Register = BREG_Read32(hRegister, REGADDR_STATUS_2) ;
	validRxKsv = BCHP_GET_FIELD_DATA(Register, REGNAME_STATUS_2, OTP_KSV_VALID) ;
	if (!validRxKsv)
	{
		BDBG_ERR(("????????????????????????")) ;
		BDBG_ERR(("?  BKSV check FAILED!! ?")) ;
		BDBG_ERR(("????????????????????????")) ;

		BDBG_ERR(("Invalid BKSV from DVP_HR OTP : %02x %02x %02x %02x %02x ",
			OtpRxBksv[4], OtpRxBksv[3], OtpRxBksv[2], OtpRxBksv[1], OtpRxBksv[0])) ;

		/* Verify RxBksv has 20 zeros & 20 ones */
		if (BAVC_HDMI_HDCP_NumberOfSetBits(OtpRxBksv, BAVC_HDMI_HDCP_KSV_LENGTH) != 20) 
		{
			BDBG_ERR(("DVP_HR OTP BSKV from OTP does not have 20 ones and 20 zeros")) ;
		}
		
		hHDR->stHdcpStatus.eOtpState =  BHDR_HDCP_OtpState_eHwError ;
		goto done ;
	}

	/* Calculate OTP CRC to verify OTP is still good; kick off calculation */
	hHDR->stHdcpStatus.eOtpState = BHDR_HDCP_OtpState_eCalcInitialized ;

#if HDMI_RX_GEN == 3548
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_DVP_HR_OTP_CTRL_0) ;
	Register &= ~ BCHP_MASK(DVP_HR_DVP_HR_OTP_CTRL_0, OTP_COMPUTE_CRC) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_DVP_HR_OTP_CTRL_0, Register) ;

	Register |= BCHP_FIELD_DATA(DVP_HR_DVP_HR_OTP_CTRL_0, OTP_COMPUTE_CRC, 1) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_DVP_HR_OTP_CTRL_0, Register) ;

#else 
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_DVP_OTP_CTRL_0) ;
	Register &= ~ BCHP_MASK(DVP_HR_DVP_OTP_CTRL_0, OTP_COMPUTE_CRC) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_DVP_OTP_CTRL_0, Register) ;

	Register |= BCHP_FIELD_DATA(DVP_HR_DVP_OTP_CTRL_0, OTP_COMPUTE_CRC, 1) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_DVP_OTP_CTRL_0, Register) ;
#endif	


	BTMR_StartTimer_isr(hHDR->timerOtpCrcCalc, 1000) ;
#if BHDR_CONFIG_DEBUG_HDCP_KEY_LOADING
	BDBG_WRN(("RX BKSV	   : %02x %02x %02x %02x %02x ",
		OtpRxBksv[4], OtpRxBksv[3], OtpRxBksv[2], OtpRxBksv[1], OtpRxBksv[0])) ;

	BDBG_WRN(("HDCP Key Set verification check started...")) ;
#endif
done :

	if (ulOffset)
	{
		BDBG_ERR(("!!!Adjust HDCP Key Load base address for multiple HDMI Rx cores!!!")) ;
		BDBG_ASSERT(false) ;
	}

	BDBG_LEAVE(BHDR_HDCP_P_EnableKeyLoading) ;
	return rc ;
}


/******************************************************************************
Summary: Disable HDCP Key Loading 
*******************************************************************************/
BERR_Code BHDR_HDCP_P_DisableKeyLoading(BHDR_Handle hHDR) 
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register ;
	
	BDBG_ENTER(BHDR_HDCP_P_DisableKeyLoading) ;

	hRegister = hHDR->hRegister ;

	/* serial key loading is always used on 7422; no change needed */
	BSTD_UNUSED(hRegister) ;
	BSTD_UNUSED(Register) ;


	BDBG_LEAVE(BHDR_HDCP_P_DisableKeyLoading) ;
	return rc ;
}


/******************************************************************************
Summary: Install Callback used to Notify for HDCP Key Set Status/Errors
*******************************************************************************/
BERR_Code BHDR_HDCP_P_InstallHdcpStatusChangeCallback( 
	BHDR_Handle hHDR,			/* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for packet error changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */ 
	int iParm2)    /* [in] the second argument(int) passed to the callback function */ 
{
	BERR_Code			rc = BERR_SUCCESS;
	
	BDBG_ENTER(BHDR_HDCP_P_InstallHdcpStatusChangeCallback) ;

	/* Check if this is a valid function */
	if( pfCallback_isr == NULL )
	{
		rc = BERR_TRACE(BERR_INVALID_PARAMETER);
		return rc;
	}
		
	BKNI_EnterCriticalSection() ;
		hHDR->pfHdcpStatusChangeCallback = pfCallback_isr;
		hHDR->pvHdcpStatusChangeParm1 = pvParm1 ;
		hHDR->iHdcpStatusChangeParm2 = iParm2 ;
	BKNI_LeaveCriticalSection() ;
	
	BDBG_LEAVE(BHDR_HDCP_P_InstallHdcpStatusChangeCallback);
	
	return rc ;
}


/******************************************************************************
Summary: Uninstall Callback used to Notify for HDCP Key Set Status/Errors
*******************************************************************************/
BERR_Code BHDR_HDCP_P_UnInstallHdcpStatusChangeCallback( 
	BHDR_Handle hHDR,                       /* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr) /* [in] cb for Packet Error change Notification */
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(pfCallback_isr) ;

	BDBG_ENTER(BHDR_HDCP_UnInstallHdcpStatusChangeCallback);
	BKNI_EnterCriticalSection() ;
	
		hHDR->pfHdcpStatusChangeCallback = (BHDR_CallbackFunc) NULL ;
		hHDR->pvHdcpStatusChangeParm1 = NULL  ;
		hHDR->iHdcpStatusChangeParm2 = 0 ;

	BKNI_LeaveCriticalSection() ;
	
	BDBG_LEAVE(BHDR_HDCP_UnInstallHdcpStatusChangeCallback);
	
	return rc;
}



/******************************************************************************
Summary: Check Status of HDCP Key Load
*******************************************************************************/
void BHDR_HDCP_P_VerifyOtpCalculation_isr(
	BHDR_Handle hHDR) 
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	uint32_t programmedCRC ;
	uint32_t computedCRC ;
	uint8_t otpCrcCalcDone ;
	uint8_t otpCrcMatch ;
	
	BDBG_ENTER(BHDR_HDCP_P_VerifyOtpCalculation_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;
	BDBG_ASSERT(hRegister) ;

	/* check if HDCP OTP CRC is ready */	
	Register = BREG_Read32_isr(hRegister, REGADDR_STATUS_2) ;
	otpCrcCalcDone = BCHP_GET_FIELD_DATA(Register, REGNAME_STATUS_2, OTP_CRC_READY) ;


	/*	if HDCP OTP CRC is not ready warn/return */ 
	if (!otpCrcCalcDone)
	{
		hHDR->stHdcpStatus.eOtpState = BHDR_HDCP_OtpState_eCalcRunning ;
		BDBG_WRN(("OTP CRC calculation has not completed!")) ;
		goto done ;
	}
	

	Register = BREG_Read32(hRegister, REGADDR_STATUS_2) ;
	otpCrcMatch = BCHP_GET_FIELD_DATA(Register, REGNAME_STATUS_2, OTP_CRC_CORRECT) ;
	
	Register = BREG_Read32(hRegister, REGADDR_STATUS_4) ;
	programmedCRC = 
		BCHP_GET_FIELD_DATA(Register, REGNAME_STATUS_4, OTP_CRC) ;
	hHDR->stHdcpStatus.programmedCrc = programmedCRC ;

	Register = BREG_Read32(hRegister, REGADDR_STATUS_5) ;
	computedCRC = BCHP_GET_FIELD_DATA(
		Register, REGNAME_STATUS_5, COMPUTED_CRC) ;
	hHDR->stHdcpStatus.calculatedCrc = computedCRC ;

	if (!otpCrcMatch)
	{		
		hHDR->stHdcpStatus.eOtpState = BHDR_HDCP_OtpState_eCrcMismatch ;
		BDBG_ERR(("OTP CRC <%X> and Computed CRC <%X> mismatch!!!",
			programmedCRC, computedCRC)) ;
		
		goto done ;
	}

	hHDR->stHdcpStatus.eOtpState = BHDR_HDCP_OtpState_eCrcMatch ;
	BDBG_WRN(("OTP HDCP Keyset OK: programmed CRC == Calculated CRC")) ;
	
done :
	BDBG_LEAVE(BHDR_HDCP_P_VerifyOtpCalculation_isr) ;
	
}	
	

BERR_Code BHDR_HDCP_P_GetStatus(BHDR_Handle hHDR, BHDR_HDCP_Status *pStatus) 
{ 
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	
	unsigned char   OtpRxBksv[] = { 0x00, 0x01, 0x02, 0x03, 0x04 }; /* LSB...MSB */
	unsigned char RecdTxAksv[]  = {0x00, 0x01, 0x02, 0x03, 0x04}; /* LSB...MSB */
	unsigned char RecdAnValue[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x6, 0x7}; /* LSB...MSB */
	
	BDBG_ENTER(BHDR_HDCP_GetStatus) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;
	
	BKNI_Memset(pStatus, 0, sizeof(BHDR_HDCP_Status)) ;

	/* get the BKSV */
	Register = BREG_Read32(hRegister, REGADDR_STATUS_0) ;
	OtpRxBksv[0] =  Register & 0x000000FF ;
	OtpRxBksv[1] = (Register & 0x0000FF00) >>  8 ;
	OtpRxBksv[2] = (Register & 0x00FF0000) >> 16 ;
	OtpRxBksv[3] = (Register & 0xFF000000) >> 24 ;

	Register = BREG_Read32(hRegister, REGADDR_STATUS_1) ;
	OtpRxBksv[4] = BCHP_GET_FIELD_DATA(Register, REGNAME_STATUS_1, OTP_BKSV_1) ;
	
	BKNI_Memcpy(&hHDR->stHdcpStatus.bksvValue, &OtpRxBksv, BAVC_HDMI_HDCP_KSV_LENGTH) ;

	/* get the AKSV */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_AKSV_0 + ulOffset) ;
	RecdTxAksv[0] =  Register & 0x000000FF ;
	RecdTxAksv[1] = (Register & 0x0000FF00) >>  8 ;
	RecdTxAksv[2] = (Register & 0x00FF0000) >> 16 ;
	RecdTxAksv[3] = (Register & 0xFF000000) >> 24 ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_AKSV_1 + ulOffset) ;
	RecdTxAksv[4] = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_MON_AKSV_1, HDCP_RX_MON_AKSV) ;
	
	BKNI_Memcpy(&hHDR->stHdcpStatus.aksvValue, &RecdTxAksv, BAVC_HDMI_HDCP_KSV_LENGTH) ;

	
	/* get the Random An Value  */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_AN_0 + ulOffset) ;
	RecdAnValue[0] =  Register & 0x000000FF ;
	RecdAnValue[1] = (Register & 0x0000FF00) >>  8 ;
	RecdAnValue[2] = (Register & 0x00FF0000) >> 16 ;
	RecdAnValue[3] = (Register & 0xFF000000) >> 24 ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_AN_1 + ulOffset) ;
	RecdAnValue[4] =  Register & 0x000000FF ;
	RecdAnValue[5] = (Register & 0x0000FF00) >>  8 ;
	RecdAnValue[6] = (Register & 0x00FF0000) >> 16 ;
	RecdAnValue[7] = (Register & 0xFF000000) >> 24 ;
	
	BKNI_Memcpy(&hHDR->stHdcpStatus.anValue, &RecdAnValue, 
		BAVC_HDMI_HDCP_KSV_LENGTH) ;

	/* copy updated values */
	BKNI_EnterCriticalSection() ;
		BKNI_Memcpy_isr(pStatus, &hHDR->stHdcpStatus, sizeof(BHDR_HDCP_Status)) ;
	BKNI_LeaveCriticalSection() ;

	
#if BHDR_CONFIG_DEBUG_HDCP_KEY_LOADING
	BDBG_WRN(("HDCP Keys Storage: OTP ROM;  OTP State: %d ", 
		pStatus->eOtpState)) ;

	if (hHDR->stHdcpStatus.eOtpState <= BHDR_HDCP_OtpState_eCalcRunning)
	{
		BDBG_WRN(("HDCP Key Set verification not finished...")) ;
	}
	else		
	{
		BDBG_WRN(("Computed   CRC %08x ", pStatus->calculatedCrc )) ;
		BDBG_WRN(("Programmed CRC %08x ", pStatus->programmedCrc)) ;
	}
#endif


#if BHDR_CONFIG_DEBUG_HDCP_VALUES	
	BDBG_WRN(("RX BKSV     : %02x %02x %02x %02x %02x ",
		OtpRxBksv[4], OtpRxBksv[3], OtpRxBksv[2], OtpRxBksv[1], OtpRxBksv[0])) ;

	if (hHDR->stHdcpStatus.eAuthState >= BHDR_HDCP_AuthState_eWaitForKeyloading)
	{
		BDBG_WRN(("TX AKSV     : %02x %02x %02x %02x %02x ",
			RecdTxAksv[4], RecdTxAksv[3], RecdTxAksv[2], RecdTxAksv[1], RecdTxAksv[0])) ;

		BDBG_WRN(("TX An Value : %02x %02x %02x %02x %02x %02x %02x %02x",
			RecdAnValue[7], RecdAnValue[6], RecdAnValue[5], RecdAnValue[4],
			RecdAnValue[3], RecdAnValue[2], RecdAnValue[1], RecdAnValue[0])) ;
	}
#endif

	BDBG_LEAVE(BHDR_HDCP_GetStatus) ;

	return rc ;

}




