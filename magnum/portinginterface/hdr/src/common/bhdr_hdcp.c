/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#include "bchp_dvp_hr_key_ram.h"
#include "bhdr_hdcp.h"
#include "bhdr_priv.h"
#include "bhdr_hdcp_priv.h"


BDBG_MODULE(BHDR_HDCP) ;


/******************************************************************************
Summary: Get Default HDCP Settings
*******************************************************************************/
void BHDR_HDCP_GetDefaultSettings(BHDR_HDCP_Settings *pHdcpSettings)
{
	BDBG_ENTER(BHDR_HDCP_GetSettings) ;

	BKNI_Memset(pHdcpSettings, 0, sizeof(BHDR_HDCP_Settings)) ;

	pHdcpSettings->bRepeater = true ;
	pHdcpSettings->uiMaxLevels = 7 ;
	pHdcpSettings->uiMaxDevices = 127 ;

	BDBG_LEAVE(BHDR_HDCP_GetSettings) ;
}



/******************************************************************************
Summary: Get Current HDCP Settings
*******************************************************************************/
void BHDR_HDCP_GetSettings(BHDR_Handle hHDR, BHDR_HDCP_Settings *pHdcpSettings)
{
	BDBG_ENTER(BHDR_HDCP_GetSettings) ;
	BKNI_Memset(pHdcpSettings, 0, sizeof(BHDR_HDCP_Settings)) ;

	BKNI_Memcpy(pHdcpSettings, &hHDR->stHdcpSettings,
		sizeof(BHDR_HDCP_Settings)) ;

	BDBG_LEAVE(BHDR_HDCP_GetSettings) ;
}


/******************************************************************************
Summary: Set/Update HDCP Settings
*******************************************************************************/
void BHDR_HDCP_SetSettings(BHDR_Handle hHDR, BHDR_HDCP_Settings *pHdcpSettings)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;

	BDBG_ENTER(BHDR_HDCP_SetSettings) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BDBG_ASSERT(pHdcpSettings) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_CONFIG + ulOffset) ;
	Register &= ~ (
		  BCHP_MASK(HDMI_RX_0_HDCP_CONFIG, RDB_REPEATER)
		| BCHP_MASK(HDMI_RX_0_HDCP_CONFIG, DISABLE_OTP_REPEATER)) ;
	Register |=
		  BCHP_FIELD_DATA(HDMI_RX_0_HDCP_CONFIG, RDB_REPEATER, pHdcpSettings->bRepeater)
		| BCHP_FIELD_DATA(HDMI_RX_0_HDCP_CONFIG, DISABLE_OTP_REPEATER, pHdcpSettings->bRepeater) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_CONFIG + ulOffset, Register) ;

	BKNI_Memcpy(&hHDR->stHdcpSettings, pHdcpSettings, sizeof(BHDR_HDCP_Settings)) ;

	if (!pHdcpSettings->bRepeater)
	{
		BDBG_WRN((" ")) ;
		BDBG_WRN(("!!!!!  HDCP License requires repeater functionality be implemented")) ;
		BDBG_WRN(("!!!!!  if devices re-transmit encrypted content")) ;
		BDBG_WRN((" ")) ;
	}

#if 0
TODO CODE TO RETRIEVE BKSV ONCE STORED
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_BKSV_0 + ulOffset) ;
		hHDR->stHdcpSettings.rxBksv[0] = (Register & 0x000000FF) ;
		hHDR->stHdcpSettings.rxBksv[1] = (Register & 0x0000FF00) >>  8 ;
		hHDR->stHdcpSettings.rxBksv[2] = (Register & 0x00FF0000) >> 16 ;
		hHDR->stHdcpSettings.rxBksv[3] = (Register & 0xFF000000) >> 24 ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_BKSV_1 + ulOffset) ;
		hHDR->stHdcpSettings.rxBksv[4] = (Register & 0x000000FF) ;
#endif
	hHDR->stHdcpStatus.eAuthState = BHDR_HDCP_AuthState_eIdle ;

	BDBG_LEAVE(BHDR_HDCP_SetSettings) ;

}


/******************************************************************************
Summary: Enable HDCP Key Loading
*******************************************************************************/
BERR_Code BHDR_HDCP_EnableKeyLoading(BHDR_Handle hHDR)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDR_HDCP_EnableKeyLoading) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	rc = BHDR_HDCP_P_EnableKeyLoading(hHDR) ;

	BDBG_LEAVE(BHDR_HDCP_EnableKeyLoading) ;
	return rc ;
}


/******************************************************************************
Summary: Disable HDCP Key Loading
*******************************************************************************/
BERR_Code BHDR_HDCP_DisableKeyLoading(BHDR_Handle hHDR)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDR_HDCP_DisableKeyLoading) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	rc = BHDR_HDCP_P_DisableKeyLoading(hHDR);

	BDBG_LEAVE(BHDR_HDCP_DisableKeyLoading) ;
    return rc ;
}


/******************************************************************************
Summary: Install Callback used to Notify for changes in HDCP Status,  Key Set loading etc.
*******************************************************************************/
BERR_Code BHDR_HDCP_InstallHdcpStatusChangeCallback(
	BHDR_Handle hHDR,			/* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for packet error changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2)    /* [in] the second argument(int) passed to the callback function */
{
	BERR_Code			rc = BERR_SUCCESS;

	BDBG_ENTER(BHDR_HDCP_InstallHdcpStatusChangeCallback) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* Check if this is a valid function */
	if( pfCallback_isr == NULL )
	{
		rc = BERR_TRACE(BERR_INVALID_PARAMETER);
		return rc;
	}

	BKNI_EnterCriticalSection() ;
		hHDR->pfHdcpStatusChangeCallback = pfCallback_isr ;
		hHDR->pvHdcpStatusChangeParm1 = pvParm1  ;
		hHDR->iHdcpStatusChangeParm2 = iParm2 ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_HDCP_InstallHdcpStatusChangeCallback);

	return rc ;
}


/******************************************************************************
Summary: Uninstall Callback used to Notify for for changes in HDCP Status,  Key Set loading etc.
*******************************************************************************/
BERR_Code BHDR_HDCP_UnInstallHdcpStatusChangeCallback(
	BHDR_Handle hHDR,                       /* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr) /* [in] cb for Packet Error change Notification */
{
	BERR_Code rc = BERR_SUCCESS ;

	BSTD_UNUSED(pfCallback_isr) ;

	BDBG_ENTER(BHDR_HDCP_UnInstallHdcpStatusChangeCallback) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_EnterCriticalSection() ;

		hHDR->pfHdcpStatusChangeCallback = NULL ;
		hHDR->pvHdcpStatusChangeParm1 = NULL  ;
		hHDR->iHdcpStatusChangeParm2 = 0  ;

	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_UnInstallPacketErrorChangeCallback);

	return rc ;
}


/******************************************************************************
Summary: Get current status of HDCP Key Set, authentication etc.
*******************************************************************************/
BERR_Code BHDR_HDCP_GetStatus(BHDR_Handle hHDR, BHDR_HDCP_Status *pStatus)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDR_HDCP_GetStatus) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	rc = BHDR_HDCP_P_GetStatus(hHDR, pStatus) ;

	BDBG_LEAVE(BHDR_HDCP_GetStatus) ;

	return rc ;
}

#if BHDR_CONFIG_HDCP2X_SUPPORT
BERR_Code BHDR_HDCP_SetHdcp2xRxCaps(
	const BHDR_Handle hHDR,
	const uint8_t version,
	const uint16_t rxCapsMask,
	const uint8_t repeater
)
{
	uint32_t Register, ulOffset;
	BREG_Handle hRegister;

	BDBG_ENTER(BHDR_HDCP_SetHdcp2xRxCaps);
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister;
	ulOffset = hHDR->ulOffset;

	Register = BREG_Read32(hRegister, BCHP_HDCP2_RX_0_RXCAPS  + ulOffset);
		Register &= ~(BCHP_MASK(HDCP2_RX_0_RXCAPS, VERSION)
					| BCHP_MASK(HDCP2_RX_0_RXCAPS, RECEIVER_CAPABILITY_MASK)
					| BCHP_MASK(HDCP2_RX_0_RXCAPS, REPEATER));


		Register |= BCHP_FIELD_DATA(HDCP2_RX_0_RXCAPS , VERSION, version)
				| BCHP_FIELD_DATA(HDCP2_RX_0_RXCAPS, RECEIVER_CAPABILITY_MASK, rxCapsMask)
				| BCHP_FIELD_DATA(HDCP2_RX_0_RXCAPS, REPEATER, repeater);
	BREG_Write32(hRegister, BCHP_HDCP2_RX_0_RXCAPS + ulOffset, Register) ;

	/* Set HDCP2Version register accordingly to indicate HDCP2.x support */
	Register = BREG_Read32(hRegister, BCHP_HDCP2_RX_0_HDCP2VERSION_CFG  + ulOffset);
		Register &= ~ BCHP_MASK(HDCP2_RX_0_HDCP2VERSION_CFG, VERSION);
		Register |= BCHP_FIELD_DATA(HDCP2_RX_0_HDCP2VERSION_CFG, VERSION, 0x04);
	BREG_Write32(hRegister, BCHP_HDCP2_RX_0_HDCP2VERSION_CFG + ulOffset, Register) ;

	BDBG_LEAVE(BHDR_HDCP_SetHdcp2xRxCaps);
	return BERR_SUCCESS;
}


BERR_Code BHDR_HDCP_EnableSerialKeyRam(
	const BHDR_Handle hHDR,
	const bool enable
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDR_HDCP_EnableSerialKeyRam);
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	BKNI_EnterCriticalSection();
	rc = BHDR_HDCP_P_EnableSerialKeyRam_isr(hHDR, enable);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BHDR_HDCP_EnableSerialKeyRam);
	return rc;

}


BERR_Code BHDR_HDCP_GetHdcp2xAuthenticationStatus(
	const BHDR_Handle hHDR,
	BHDR_HDCP_Hdcp2xAuthenticationStatus *pAuthenticationStatus
)
{
	uint32_t Register, ulOffset;
	BREG_Handle hRegister;

	BDBG_ENTER(BHDR_HDCP_GetHdcp2xAuthenticationStatus);
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister;
	ulOffset = hHDR->ulOffset;

	Register = BREG_Read32(hRegister, BCHP_HDCP2_RX_0_STATUS_0 + ulOffset);
	pAuthenticationStatus->bEncrypted = BCHP_GET_FIELD_DATA(Register, HDCP2_RX_0_STATUS_0, ENCRYPTION_ENABLED) > 0 ;
	pAuthenticationStatus->bAuthenticated = BCHP_GET_FIELD_DATA(Register, HDCP2_RX_0_STATUS_0, AUTH_STATUS) > 0 ;

	BDBG_LEAVE(BHDR_HDCP_GetHdcp2xAuthenticationStatus);
	return BERR_SUCCESS;
}


BERR_Code BHDR_HDCP_SendHdcp2xReAuthREQ(
	const BHDR_Handle hHDR
)
{
	uint32_t Register, ulOffset;
	BREG_Handle hRegister;

	BDBG_ENTER(BHDR_HDCP_SendHdcp2xReAuthREQ);
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister;
	ulOffset = hHDR->ulOffset;

	Register = BREG_Read32(hRegister, BCHP_HDCP2_RX_0_RXSTATUS_AUTH_CFG + ulOffset);
		Register &= ~ BCHP_MASK(HDCP2_RX_0_RXSTATUS_AUTH_CFG, REAUTH_REQ);
		Register |= BCHP_FIELD_DATA(HDCP2_RX_0_RXSTATUS_AUTH_CFG, REAUTH_REQ, 0x01);
	BREG_Write32(hRegister, BCHP_HDCP2_RX_0_RXSTATUS_AUTH_CFG + ulOffset, Register) ;


	BDBG_LEAVE(BHDR_HDCP_SendHdcp2xReAuthREQ);
	return BERR_SUCCESS;
}


BERR_Code BHDR_HDCP_InstallDisconnectNotifyCallback(
	const BHDR_Handle hHDR,
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for notification */
	void *pvParm1,  /* [in] the first argument (void *) passed to the callback function */
	int iParm2      /* [in] the second argument(int) passed to the callback function */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BDBG_ENTER(BHDR_HDCP_InstallDisconnectNotifyCallback);
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	if (hHDR->pfHdcpDisconnectNotifyCallback_isr)
	{
		BDBG_ERR(("Callback handler already installed for DisconnectNotify...overriding")) ;
	}

	BKNI_EnterCriticalSection() ;
		hHDR->pfHdcpDisconnectNotifyCallback_isr = pfCallback_isr ;
		hHDR->pvHdcpDisconnectNotifyParm1 = pvParm1 ;
		hHDR->iHdcpDisconnectNotifyParm2 = iParm2 ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_HDCP_InstallDisconnectNotifyCallback) ;
	return rc ;
}


BERR_Code BHDR_HDCP_UnInstallDisconnectNotifyCallback(
	const BHDR_Handle hHDR,
	const BHDR_CallbackFunc pfCallback_isr /* [in] cb for notification  */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BDBG_ENTER(BHDR_HDCP_UnInstallDisconnectNotifyCallback);
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	BSTD_UNUSED(pfCallback_isr) ;

	if (hHDR->pfHdcpDisconnectNotifyCallback_isr == (BHDR_CallbackFunc) NULL)
	{
		BDBG_WRN(("No callback handler to uninstall for DisconnectNotify callback")) ;
		goto done ;
	}

	BKNI_EnterCriticalSection() ;
		hHDR->pfHdcpDisconnectNotifyCallback_isr = (BHDR_CallbackFunc) NULL ;
		hHDR->pvHdcpDisconnectNotifyParm1 = (void *) NULL  ;
		hHDR->iHdcpDisconnectNotifyParm2 = 0  ;
	BKNI_LeaveCriticalSection() ;

done :
	BDBG_LEAVE(BHDR_HDCP_UnInstallDisconnectNotifyCallback) ;
	return rc ;
}
#endif
