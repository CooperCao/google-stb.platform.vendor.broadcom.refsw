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
#ifndef BHDR_HDCP_H__
#define BHDR_HDCP_H__

#include "bhdr.h"


typedef enum BHDR_HDCP_KeyStorage
{
	BHDR_HDCP_KeyStorage_eNone,
	BHDR_HDCP_KeyStorage_eOtpROM,
	BHDR_HDCP_KeyStorage_eKeyRAM,
	BHDR_HDCP_KeyStorage_eMax
} BHDR_HDCP_KeyStorage ;


typedef enum BHDR_HDCP_OtpState
{
	BHDR_HDCP_OtpState_eNone,
	BHDR_HDCP_OtpState_eHwError,
	BHDR_HDCP_OtpState_eCalcInitialized,
	BHDR_HDCP_OtpState_eCalcRunning,
	BHDR_HDCP_OtpState_eCrcMatch,
	BHDR_HDCP_OtpState_eCrcMismatch,
	BHDR_HDCP_OtpState_eMax
} BHDR_HDCP_OtpState ;


typedef enum BHDR_HDCP_AuthState
{
	BHDR_HDCP_AuthState_eKeysetInitialization,
	BHDR_HDCP_AuthState_eKeysetError,
	BHDR_HDCP_AuthState_eIdle,
	BHDR_HDCP_AuthState_eWaitForKeyloading,
	BHDR_HDCP_AuthState_eWaitForDownstreamKsvs,
	BHDR_HDCP_AuthState_eKsvFifoReady,
	BHDR_HDCP_AuthState_eMax
} BHDR_HDCP_AuthState ;


/******************************************************************************
Summary:
	Encrypted HDCP Key structure
*******************************************************************************/
typedef struct BHDR_HDCP_EncryptedKeyStruct
{
	uint32_t CaDataLo;
	uint32_t CaDataHi;
	uint32_t TCaDataLo;
	uint32_t TCaDataHi;
	uint32_t HdcpKeyLo;
	uint32_t HdcpKeyHi;
} BHDR_HDCP_EncryptedKeyStruct ;


typedef enum BHDR_HDCP_Version
{
	BHDR_HDCP_Version_e1x = 0,
	BHDR_HDCP_Version_e2x,
	BHDR_HDCP_Version_eMax
} BHDR_HDCP_Version;


typedef struct
{
	BHDR_HDCP_Version eVersion;
	BHDR_HDCP_KeyStorage eKeyStorage ;
	BHDR_HDCP_OtpState eOtpState ;
	BHDR_HDCP_OtpState eAuthState ;
	uint32_t programmedCrc ;
	uint32_t calculatedCrc ;
	uint8_t anValue[BAVC_HDMI_HDCP_AN_LENGTH] ;
	uint8_t aksvValue[BAVC_HDMI_HDCP_KSV_LENGTH] ;
	uint8_t bksvValue[BAVC_HDMI_HDCP_KSV_LENGTH] ;

} BHDR_HDCP_Status ;


typedef struct BHDR_HDCP_Settings
{
	uint8_t rxBksv[BAVC_HDMI_HDCP_KSV_LENGTH] ;
	uint8_t uiMaxLevels ;
	uint8_t uiMaxDevices ;
	uint8_t bRepeater ;

	BHDR_HDCP_Version eVersion;

} BHDR_HDCP_Settings ;

/* HDCP Key Loading Base Address */
#define BHDR_HDCP_RX_0_KEY_BASE_ADDRESS (0x00)
#define BHDR_HDCP_RX_1_KEY_BASE_ADDRESS (0x80)



/***************************************************************************
Summary:
HDMI HDCP Key Set Information  (keys, Tx and Rx KSVs, etc.).
****************************************************************************/

void BHDR_HDCP_GetDefaultSettings(
	BHDR_HDCP_Settings *pHdcpSettings) ;

void BHDR_HDCP_GetSettings(
	BHDR_Handle hHDR,
	BHDR_HDCP_Settings *pHdcpSettings) ;

void BHDR_HDCP_SetSettings(
	BHDR_Handle hHDR,
	BHDR_HDCP_Settings *pHdcpSettings) ;

BERR_Code BHDR_HDCP_GetStatus(
	BHDR_Handle hHDR,
	BHDR_HDCP_Status *pStatus) ;

BERR_Code BHDR_HDCP_UnInstallHdcpStatusChangeCallback(
	BHDR_Handle hHDR,                       /* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr)  ; /* [in] cb for Packet Error change Notification */

BERR_Code BHDR_HDCP_InstallHdcpStatusChangeCallback(
	BHDR_Handle hHDR,			/* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for packet error changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2) ;    /* [in] the second argument(int) passed to the callback function */

BERR_Code BHDR_HDCP_EnableKeyLoading(BHDR_Handle hHDR)  ;

BERR_Code BHDR_HDCP_DisableKeyLoading(BHDR_Handle hHDR)  ;


typedef struct BHDR_HDCP_RepeaterDownStreamInfo
{
    uint8_t depth;                  /* number of levels of devices */
    bool maxDepthExceeded;

    uint8_t devices;            /* number of devices  */
    bool maxDevicesExceeded;

    bool isRepeater;
    uint8_t repeaterKsv[BAVC_HDMI_HDCP_KSV_LENGTH] ;
} BHDR_HDCP_RepeaterDownStreamInfo ;


#if BHDR_CONFIG_HDCP_REPEATER
/**************************************************************************
Summary:
	Load the KSV FIFO with downstream KSVs

Description:
	This function will load each KSV into the KSV FIFO for access by an attached transmtter.

Input:
	hHDR - HDMI Rx Handle
	pDownstreamInfo  - pointer to structure containing the downstream Info (devices, levels, number of KSVs)

Returns:
	none

See Also:
	BHDR_HDCP_InstallRepeaterRequestKsvFifoCallback
	BHDR_HDCP_UnInstallRepeaterRequestKsvFifoCallback
**************************************************************************/
void BHDR_HDCP_LoadRepeaterKsvFifo(
	BHDR_Handle hHDR,
	BHDR_HDCP_RepeaterDownStreamInfo *pDownstreamInfo,
	uint8_t *pKsvs) ;
#endif


#if BHDR_CONFIG_HDCP2X_SUPPORT

typedef struct
{
	bool bAuthenticated;
	bool bEncrypted;

} BHDR_HDCP_Hdcp2xAuthenticationStatus;


/**************************************************************************
Summary:
	Update Hdcp2.x RxCaps

Description:
	This function will update the HDCP2.x RxCaps in the HW

Input:
	hHDR - HDMI Rx Handle
	version - hdcp version per the HDCP spec
	rxCapsMask - RxCaps_MASK define in HDCP spec
	repeater - identify as Repeater or Receiver

Returns:
	none

**************************************************************************/
BERR_Code BHDR_HDCP_SetHdcp2xRxCaps(
       const BHDR_Handle hHDR,
       const uint8_t version,
       const uint16_t rxCapsMask,
       const uint8_t repeater
);


/**************************************************************************
Summary:
	Enable/Disable Serial Key RAM

Description:
	This function will enable/disable Serial Key RAM which is needed based on the current
	HDCP version being authenticated

Input:
	hHDR - HDMI Rx Handle
	enable - true to enable Serial Key RAM. Otherwise, disable

Returns:
	none

**************************************************************************/
BERR_Code BHDR_HDCP_EnableSerialKeyRam(
	const BHDR_Handle hHDR,
	const bool enable
);


/**************************************************************************
Summary:
	Get HDCP2.x authentication status

Description:
	This function will retrieve the current HDCP2.x authentication status

Input:
	hHDR - HDMI Rx Handle
	bEncrypted - true if HDCP2.x is currently authenticated and encryption is anbled

Returns:
	none

**************************************************************************/
BERR_Code BHDR_HDCP_GetHdcp2xAuthenticationStatus(
	const BHDR_Handle hHDR,
	BHDR_HDCP_Hdcp2xAuthenticationStatus *pAuthenticationStatus
);



/**************************************************************************
Summary:
	Issue a REAUTH_REQ to the upstream HDCP2.x transmitter

Description:
	This function basically send a REAUTH_REQ to the upstream HDCP 2.x transmitter
	requesting a restart of HDCP authentication

Input:
	hHDR - HDMI Rx Handle

Returns:
	none
**************************************************************************/
BERR_Code BHDR_HDCP_SendHdcp2xReAuthREQ(
	const BHDR_Handle hHDR
);

/**************************************************************************
Summary:
	Install HPD disconnect event callback

Description:
	This function allows the application to install a callback to be notify when the HPD
	signal is pull/held LOW

Input:
	hHDR - HDMI Rx Handle
	pfCallback_isr - callback function for notification
	pvParm1 - the first argument (void *) passed to the callback function
	iParm2 - the second argument(int) passed to the callback function

Returns:
	none

**************************************************************************/
BERR_Code BHDR_HDCP_InstallDisconnectNotifyCallback(
	const BHDR_Handle hHDR,
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for notification */
	void *pvParm1,  /* [in] the first argument (void *) passed to the callback function */
	int iParm2      /* [in] the second argument(int) passed to the callback function */
);


/**************************************************************************
Summary:
	UnInstall HPD disconnect event callback

Description:
	This function uninstall the HDCP disconnect event callback registered/installed by
	the application using BHDR_HDCP_InstallDisconnectNotifyCallback

Input:
	hHDR - HDMI Rx Handle
	pfCallback_isr - callback function need to uninstall

Returns:
	none

**************************************************************************/
BERR_Code BHDR_HDCP_UnInstallDisconnectNotifyCallback(
	const BHDR_Handle hHDR,
	const BHDR_CallbackFunc pfCallback_isr /* [in] cb for notification  */
);


#endif


#ifdef __cplusplus
}
#endif

#endif /* BHDR_HDCP_H__ */

