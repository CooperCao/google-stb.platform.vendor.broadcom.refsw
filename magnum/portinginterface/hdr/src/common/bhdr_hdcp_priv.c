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
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bhdr_config.h"
#include "bhdr.h"
#include "bchp_dvp_hr.h"
#include "bchp_dvp_hr_key_ram.h"

#include "bhdr_priv.h"
#include "bhdr_hdcp_priv.h"

BDBG_MODULE(BHDR_HDCP_PRIV) ;


BERR_Code BHDR_HDCP_P_EnableKeyLoading(BHDR_Handle hHDR)
{
	static const uint8_t HdcpSpecRx1Bksv[BAVC_HDMI_HDCP_KSV_LENGTH] =
		{0xCD, 0x1A, 0xF2, 0x1E, 0x51} ;

	static const uint8_t HdcpSpecRx2Bksv[BAVC_HDMI_HDCP_KSV_LENGTH] =
		{0x01, 0xF4, 0x97, 0x26, 0xE7} ;

	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	uint32_t Bksv4, Bksv3_0 ;
	uint8_t keyLoadBaseAddress ;

	BDBG_ENTER(BHDR_HDCP_P_EnableKeyLoading) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	/* enable Key RAM loading */
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_KEY_RAM_CTRL_0);
	Register |= BCHP_MASK(DVP_HR_KEY_RAM_CTRL_0, KEY_RAM_INIT) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_KEY_RAM_CTRL_0, Register) ;

	/* set loading address for the key */
	if (ulOffset == 0)
	{
		keyLoadBaseAddress = BHDR_HDCP_RX_0_KEY_BASE_ADDRESS ;
	}
	else
	{
		keyLoadBaseAddress = BHDR_HDCP_RX_1_KEY_BASE_ADDRESS ;
		BDBG_ERR(("!!!Adjust HDCP Key Load base address for multiple HDMI Rx cores!!!")) ;
		BDBG_ASSERT(false) ;
	}

	Register = BREG_Read32(hRegister,BCHP_HDMI_RX_0_HDCP_CONFIG + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_0_HDCP_CONFIG, KEY_LOAD_BASE_ADDRESS ) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_0_HDCP_CONFIG, KEY_LOAD_BASE_ADDRESS, keyLoadBaseAddress) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_CONFIG + ulOffset, Register) ;


	/* make sure KSV is not from the Test Key Set*/
	if ((!BKNI_Memcmp(HdcpSpecRx1Bksv, hHDR->stHdcpSettings.rxBksv, BAVC_HDMI_HDCP_KSV_LENGTH))
	|| (!BKNI_Memcmp(HdcpSpecRx2Bksv, hHDR->stHdcpSettings.rxBksv, BAVC_HDMI_HDCP_KSV_LENGTH)))
	{
		BDBG_WRN(("\n\n\n")) ;
		BDBG_WRN(("******************************")) ;
		BDBG_WRN(("RxBksv = %02x %02x %02x %02x %02x",
			HdcpSpecRx1Bksv[4], HdcpSpecRx1Bksv[3],
			HdcpSpecRx1Bksv[2], HdcpSpecRx1Bksv[1],
			HdcpSpecRx1Bksv[0]));
		BDBG_WRN(("RxBksv is part of the HDCP Spec Test Key Set")) ;
		BDBG_WRN(("Production Key Set required for use with a Production Transmitter")) ;
		BDBG_WRN(("******************************")) ;
		rc = BHDR_HDCP_KSV_ERROR ;
		goto done ;
	}


	Bksv3_0  =
		  (uint32_t) hHDR->stHdcpSettings.rxBksv[3] << 24
		| (uint32_t) hHDR->stHdcpSettings.rxBksv[2] << 16
		| (uint32_t) hHDR->stHdcpSettings.rxBksv[1] <<  8
		| (uint32_t) hHDR->stHdcpSettings.rxBksv[0] ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_BKSV_0 + ulOffset, Bksv3_0) ;


	Bksv4 = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_BKSV_1 + ulOffset) ;
	Bksv4  &= ~ BCHP_MASK(HDMI_RX_0_HDCP_BKSV_1, HDCP_RX_BKSV) ;
	Bksv4  |= BCHP_FIELD_DATA(HDMI_RX_0_HDCP_BKSV_1, HDCP_RX_BKSV,
		(uint32_t) hHDR->stHdcpSettings.rxBksv[4]) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_BKSV_1 + ulOffset, Bksv4 ) ;

#if BHDR_CONFIG_DEBUG_HDCP_KEY_LOADING
	BDBG_WRN(("HDMI HDCP Rx Bksv: %x %x",
		Bksv4, Bksv3_0)) ;
#endif

done:
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

	uint8_t msTimeout, bKeysLoaded ;

	BDBG_ENTER(BHDR_HDCP_P_DisableKeyLoading) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;


	/* verify HDCP Key Load */
	msTimeout = 100 ;
	do
	{
		Register = BREG_Read32(hRegister, BCHP_DVP_HR_KEY_RAM_STATUS_2) ;

		bKeysLoaded =
			BCHP_GET_FIELD_DATA(Register, DVP_HR_KEY_RAM_STATUS_2, KEY_RAM_INIT_DONE) ;

		if (bKeysLoaded)
			break ;

		/* use for debugging */
		BDBG_WRN(("%03dms Wait for HDCP Key RAM initialization completion: %d",
			msTimeout, bKeysLoaded)) ;

		BKNI_Delay(100) ;
	} while (msTimeout--) ;

	/* key loading did not complete; return */
	if (!bKeysLoaded)
	{
		BDBG_WRN(("Encrypted HDCP Key Loading FAILED; KEY_RAM_STATUS_2: %#08x",
			Register )) ;
		rc = BHDR_HARDWARE_FAILURE ;
		goto done ;
	}

#if BHDR_CONFIG_DEBUG_HDCP_KEY_LOADING
	BDBG_WRN(("Encrypted HDCP Key Loading SUCCESS; KEY_RAM_STATUS_2: %#08x",
		Register)) ;
#endif


	/* KSV Initialization is dependent upon successful KEY RAM Initializaton */

	Register = BREG_Read32(hRegister, BCHP_DVP_HR_KEY_RAM_CTRL_0);
	Register &= ~ BCHP_MASK(DVP_HR_KEY_RAM_CTRL_0, KEY_RAM_INIT) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_KEY_RAM_CTRL_0, Register);


	/* verify KSV */
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_KEY_RAM_CTRL_0);
	Register |= BCHP_MASK(DVP_HR_KEY_RAM_CTRL_0, KSV_INIT) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_KEY_RAM_CTRL_0, Register);


	msTimeout = 100 ;
	do
	{
		Register = BREG_Read32(hRegister, BCHP_DVP_HR_KEY_RAM_STATUS_2) ;
		bKeysLoaded =
			BCHP_GET_FIELD_DATA(Register, DVP_HR_KEY_RAM_STATUS_2, RAM_KSV_VALID) ;

		if (bKeysLoaded)
			break ;

		BKNI_Delay(1000) ;
	} while (msTimeout--) ;

	if (!bKeysLoaded)
	{
		BDBG_ERR(("HDCP BKsv Load FAILED!!! KEY_RAM_STATUS_2: %#08x",
			Register)) ;
		hHDR->stHdcpStatus.eOtpState  = BHDR_HDCP_OtpState_eCrcMismatch ;
		rc = BHDR_HARDWARE_FAILURE ;
		goto done ;
	}

	hHDR->stHdcpStatus.eAuthState = BHDR_HDCP_AuthState_eIdle ;

#if BHDR_CONFIG_DEBUG_HDCP_KEY_LOADING
	BDBG_WRN(("HDCP BKsv Load SUCCESS; KEY_RAM_STATUS_2: %#08x", Register)) ;
#endif

done:
	/* disable Key RAM / KSV  loading */
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_KEY_RAM_CTRL_0);
	Register &= ~ BCHP_MASK(DVP_HR_KEY_RAM_CTRL_0, KEY_RAM_INIT) ;
	Register &= ~ BCHP_MASK(DVP_HR_KEY_RAM_CTRL_0, KSV_INIT) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_KEY_RAM_CTRL_0, Register);

#if BHDR_CONFIG_DEBUG_HDCP_KEY_LOADING
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_KEY_RAM_CTRL_0);
	BDBG_WRN(("KEY_RAM_CTRL_0: %#08x", Register)) ;
#endif

	BDBG_LEAVE(BHDR_HDCP_P_DisableKeyLoading) ;
    return rc ;
}


/******************************************************************************
Summary: Install Callback used to Notify for for changes in HDCP Status,  Key Set loading etc.
*******************************************************************************/
BERR_Code BHDR_HDCP_P_InstallHdcpStatusChangeCallback(
	BHDR_Handle hHDR,			/* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for packet error changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2)    /* [in] the second argument(int) passed to the callback function */
{
	BERR_Code			rc = BERR_SUCCESS;

	BSTD_UNUSED(hHDR) ;
	BSTD_UNUSED(pfCallback_isr) ;
	BSTD_UNUSED(pvParm1) ;
	BSTD_UNUSED(iParm2) ;

	BDBG_WRN(("BHDR_HDCP_P_InstallHdcpStatusChangeCallback not implemented")) ;
	return rc ;
}


/******************************************************************************
Summary: Uninstall Callback used to Notify for for changes in HDCP Status,  Key Set loading etc.
*******************************************************************************/
BERR_Code BHDR_HDCP_P_UnInstallHdcpStatusChangeCallback(
	BHDR_Handle hHDR,                       /* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr) /* [in] cb for Packet Error change Notification */
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(hHDR) ;
	BSTD_UNUSED(pfCallback_isr) ;
	BDBG_WRN(("BHDR_HDCP_P_UnInstallHdcpStatusChangeCallback not implemented")) ;

	return rc;
}


/******************************************************************************
Summary: Get current HDCP status ,  Key Set loading etc.
*******************************************************************************/
BERR_Code BHDR_HDCP_P_GetStatus(BHDR_Handle hHDR, BHDR_HDCP_Status *pStatus)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;

	bool bValidKsvLoaded ;
	bool bKeySetLoaded ;

	unsigned char   RamRxBksv[] = { 0x00, 0x01, 0x02, 0x03, 0x04 }; /* LSB...MSB */
	unsigned char RecdTxAksv[]  = {0x00, 0x01, 0x02, 0x03, 0x04}; /* LSB...MSB */
	unsigned char RecdAnValue[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x6, 0x7}; /* LSB...MSB */

	BDBG_ENTER(BHDR_HDCP_P_GetStatus) ;

	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;
	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	BKNI_Memset(pStatus, 0, sizeof(BHDR_HDCP_Status)) ;

	/* Check current HDCP version */
#if BHDR_CONFIG_HDCP2X_SUPPORT
	Register = BREG_Read32(hRegister, BCHP_HDCP2_RX_0_STATUS_0);
	hHDR->stHdcpStatus.eVersion = (BHDR_HDCP_Version)
		BCHP_GET_FIELD_DATA(Register, HDCP2_RX_0_STATUS_0, HDCP_VERSION);
#else
	hHDR->stHdcpStatus.eVersion = BHDR_HDCP_Version_e1x;
#endif

	/* HDCP Key Set is always stored in RAM for this platform */
	hHDR->stHdcpStatus.eKeyStorage = BHDR_HDCP_KeyStorage_eKeyRAM ;

	/* update DVP HR RAM state; check if  keys are already loaded */
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_KEY_RAM_STATUS_2);

	bValidKsvLoaded =
		BCHP_GET_FIELD_DATA(Register, DVP_HR_KEY_RAM_STATUS_2, RAM_KSV_VALID) ;

	bKeySetLoaded =
		BCHP_GET_FIELD_DATA(Register, DVP_HR_KEY_RAM_STATUS_2, KEY_RAM_INIT_DONE) ;

	if (bValidKsvLoaded && bKeySetLoaded)
	{
		BDBG_LOG(("HDCP keyset/BKSV stored in SECURE RAM")) ;
	}
	else
	{
		BDBG_WRN(("HDCP Key Set invalid/not loaded to RAM ")) ;
		BDBG_WRN(("HDCP BKsv Valid: %x", bValidKsvLoaded)) ;
		BDBG_WRN(("HDCP Keys loaded to RAM: %x", bKeySetLoaded)) ;
	}


	hHDR->stHdcpStatus.eOtpState = bKeySetLoaded
		? BHDR_HDCP_OtpState_eCrcMatch : BHDR_HDCP_OtpState_eHwError ;


	/* get the BKSV */
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_KEY_RAM_STATUS_0) ;
	RamRxBksv[0] =	Register & 0x000000FF ;
	RamRxBksv[1] = (Register & 0x0000FF00) >>  8 ;
	RamRxBksv[2] = (Register & 0x00FF0000) >> 16 ;
	RamRxBksv[3] = (Register & 0xFF000000) >> 24 ;

	Register = BREG_Read32(hRegister, BCHP_DVP_HR_KEY_RAM_STATUS_1) ;
	RamRxBksv[4] = BCHP_GET_FIELD_DATA(Register, DVP_HR_KEY_RAM_STATUS_1, RAM_BKSV_1) ;

	BKNI_Memcpy(&hHDR->stHdcpStatus.bksvValue, &RamRxBksv, BAVC_HDMI_HDCP_KSV_LENGTH) ;


	/* get the AKSV */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_AKSV_0 + ulOffset) ;
	RecdTxAksv[0] =  Register & 0x000000FF ;
	RecdTxAksv[1] = (Register & 0x0000FF00) >>	8 ;
	RecdTxAksv[2] = (Register & 0x00FF0000) >> 16 ;
	RecdTxAksv[3] = (Register & 0xFF000000) >> 24 ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_AKSV_1 + ulOffset) ;
	RecdTxAksv[4] = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_MON_AKSV_1, HDCP_RX_MON_AKSV) ;

	BKNI_Memcpy(&hHDR->stHdcpStatus.aksvValue, &RecdTxAksv, BAVC_HDMI_HDCP_KSV_LENGTH) ;


	/* get the Random An Value	*/
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
		BAVC_HDMI_HDCP_AN_LENGTH) ;

	/* copy updated values */
	BKNI_EnterCriticalSection() ;
		BKNI_Memcpy_isr(pStatus, &hHDR->stHdcpStatus, sizeof(BHDR_HDCP_Status)) ;
	BKNI_LeaveCriticalSection() ;

#if BHDR_CONFIG_DEBUG_HDCP_VALUES
	BDBG_WRN(("*******************************")) ;
	BDBG_WRN(("Stored RX BKSV	   : %02x %02x %02x %02x %02x ",
		RamRxBksv[4], RamRxBksv[3], RamRxBksv[2], RamRxBksv[1], RamRxBksv[0])) ;

	BDBG_WRN(("Stored TX AKSV	   : %02x %02x %02x %02x %02x ",
		RecdTxAksv[4], RecdTxAksv[3], RecdTxAksv[2], RecdTxAksv[1], RecdTxAksv[0])) ;

	BDBG_WRN(("Stored TX An Value : %02x %02x %02x %02x %02x %02x %02x %02x",
		RecdAnValue[7], RecdAnValue[6], RecdAnValue[5], RecdAnValue[4],
		RecdAnValue[3], RecdAnValue[2], RecdAnValue[1], RecdAnValue[0])) ;
	BDBG_WRN(("*******************************")) ;
#endif

	BDBG_LEAVE(BHDR_HDCP_P_GetStatus) ;

	return rc ;
}



