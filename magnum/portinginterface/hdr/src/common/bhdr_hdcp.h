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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
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
	Register a callback function to be called after a HDMI ksv FIFO request

Description:
	This function is used to enable a callback function that will
	be called any time an attached receiver requests the downstream KSV FIFO

Input:
	hHDR - HDMI Rx Handle
	pfCallback_isr - pointer to callback function to be called at the time of the request
	pvParam1 - User defined data structure casted to void.
	iParam2 - Additional user defined value...

Returns:
	BERR_SUCCESS - Callback Installation Successful

See Also:
		BHDR_HDCP_UnInstallRepeaterRequestKsvFifoCallback

**************************************************************************/
BERR_Code BHDR_HDCP_InstallRepeaterRequestKsvFifoCallback(
	BHDR_Handle hHDR,			/* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for packet changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2) ;   /* [in] the second argument(int) passed to the callback function */


/**************************************************************************
Summary:
	Remove a previously registered callback function for Ksv Request

Description:
	This function is used to remove the callback function that is called when there is KSV FIFO request

Input:
	hHDR - HDMI Rx Handle
	pfCallback - pointer to callback function to be removed

Returns:
	BERR_SUCCESS - Callback Removal Successful

See Also:
	BHDR_HDCP_InstallRepeaterRequestKsvFifoCallback
**************************************************************************/
BERR_Code BHDR_HDCP_UnInstallRepeaterRequestKsvFifoCallback(
	BHDR_Handle hHDR,                       /* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr) ; /* [in] cb for format changes */


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
BERR_Code BHDR_HDCP_SetHdcp2xRxCaps(
       const BHDR_Handle hHDR,
       const uint8_t version,
       const uint16_t rxCapsMask,
       const uint8_t repeater
);

BERR_Code BHDR_HDCP_EnableSerialKeyRam(
	const BHDR_Handle hHDR,
	const bool enable
);

#endif


#ifdef __cplusplus
}
#endif

#endif /* BHDR_HDCP_H__ */

