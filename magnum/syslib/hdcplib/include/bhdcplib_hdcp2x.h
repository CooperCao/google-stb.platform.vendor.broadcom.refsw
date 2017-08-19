/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
******************************************************************************/


#ifndef BHDCPLIB_HDCP2X_H__
#define BHDCPLIB_HDCP2X_H__


#ifdef __cplusplus
extern "C" {
#endif


#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)

#define BHDCPLIB_HDCP2X_RECEIVERID_LENGTH 	5
#define BHDCPLIB_HDCP2X_MAX_DEVICE_COUNT	31
#define BHDCPLIB_HDCP2X_MAX_DEPTH			4

#define BHDCPLIB_HDCP22_RXCAPS_VERSION 0x02
#define BHDCPLIB_HDCP22_RXCAPS_RECEIVER_CAPABILITY_MASK 0x0000

/******************************************************************************
Summary:
Enumerated Type of different events relating to HDCP2.x core

Description:
Applications can request notification of events associated with HDCP2.x core (coming from sage)
This enumeration can be passed to the BHDCPlib_Hdcp2x_GetEvent function

See Also:
	o BHDCPlib_Hdcp2x_GetEvent

*******************************************************************************/
typedef enum
{
	BHDCPLIB_Hdcp2x_EventIndication,
	BHDCPLIB_Hdcp2x_EventInvalid
} BHDCPLIB_Hdcp2x_EventType ;


/******************************************************************************
Summary:
	Hdcp2.x authentication status receive from sage

Description:
	Structure holds authentication status information receive from SAGE

See Also:
	o BHDCPlib_Hdcp2x_GetAuthenticationStatus

*******************************************************************************/
typedef struct BHDCPlib_Hdcp2x_AuthenticationStatus
{
	bool linkAuthenticated;
	bool hdcp1DeviceDownstream;
	bool downstreamIsRepeater;
	BHDCPlib_HdcpError eAuthenticationError;
	BHDCPlib_State eHdcpState;

	/* ContentStreamType received through RASM from upstream Transmitter. Applicable for Rx only */
	BHDCPlib_Hdcp2xContentStreamType eContentStreamTypeFromUpstream;
} BHDCPlib_Hdcp2x_AuthenticationStatus;


/***************************************************************************
Summary:
	Get Hdcp2.x authentication status information receive from SAGE

Input:
	hHDCPlib - HDCPlib control handle that was previously opened by BHDCPlib_Open.

Output:
	pAuthenticationStatus - structure holds status data

See Also:
	BHDCPlib_Hdcp2x_AuthenticationStatus

****************************************************************************/
BERR_Code BHDCPlib_Hdcp2x_GetAuthenticationStatus(
	BHDCPlib_Handle hHDCPlib,		/* [in] HDCPlib handle */
	BHDCPlib_Hdcp2x_AuthenticationStatus *pAuthenticationStatus /* [out] status */
);


/***************************************************************************
Summary:
	Get the event handle for checking HDCP2x events.

Input:
	hHDCPlib - HDCPlib control handle that was previously opened by BHDCPlib_Open.
	EventType - Type of event the requested handle is for

Output:
	pBHDCPlibEvent - HDCPlib Event Handle

See Also:
	BHDCPlib_Hdcp2x_EventType

****************************************************************************/
BERR_Code BHDCPlib_Hdcp2x_GetEventHandle(
   BHDCPlib_Handle hHDCPlib,           /* [in] HDCPlib handle */
   BHDCPLIB_Hdcp2x_EventType eEventType,    /* [in] HDCPlib Event Type */
   BKNI_EventHandle *pBHDCPlibEvent	/* [out] event handle */
);


/******************************************************************************
Summary:
	Response Data from SAGE

Description:
	Structure holds information receive from SAGE regarding the last request sent to SAGE

See Also:
	o BHDCPlib_SageIndicationData

*******************************************************************************/
typedef struct BHDCPlib_SageResponseData
{
	BSAGElib_RpcRemoteHandle rpcRemoteHandle;
	uint32_t async_id;
	BERR_Code error;

} BHDCPlib_SageResponseData;

/******************************************************************************
Summary:
	Indication Data from SAGE

Description:
	Structure holds information receive from SAGE through an indication from SAGE -> Host

See Also:
	o BHDCPlib_SageResponseData

*******************************************************************************/
typedef struct BHDCPlib_SageIndicationData
{
	BSAGElib_RpcRemoteHandle rpcRemoteHandle;
	uint32_t indication_id;
	uint32_t value;
} BHDCPlib_SageIndicationData;


/******************************************************************************
Summary:
	Receiver Id List Data

Description:
	Defines all data of an HDCP 2.x Receiver Id List

*******************************************************************************/
typedef struct BHDCPlib_ReceiverIdListData
{
	uint8_t deviceCount;
	uint8_t depth;
	uint8_t maxDevsExceeded;
	uint8_t maxCascadeExceeded;
	uint8_t hdcp2LegacyDeviceDownstream;
	uint8_t hdcp1DeviceDownstream;
	uint8_t downstreamIsRepeater;
	uint8_t rxIdList[155];
} BHDCPlib_ReceiverIdListData;


/******************
Summary: Reinitialize HDCP2x related rpc handles after sage watchdog occurred.
******************/
BERR_Code BHDCPlib_Hdcp2x_ProcessWatchDog(BHDCPlib_Handle hHDCPlib);


/******************
Summary: Provide responses from SAGE to HDCPlib
******************/
BERR_Code BHDCPlib_Hdcp2x_ReceiveSageResponse(
	const BHDCPlib_Handle hHDCPlib,
	const BHDCPlib_SageResponseData *sageResponseData
);


/******************
Summary: Provide indications from SAGE to HDCPlib
******************/
BERR_Code BHDCPlib_Hdcp2x_ReceiveSageIndication(
	const BHDCPlib_Handle hHDCPlib,
	const BHDCPlib_SageIndicationData *sageIndicationData
);


/******************
Summary:
	Enable/Disable Hdcp2.x encryption.

Input:
	hHDCPlib - HDCPlib handle that was previously open by BHDCPlib_Open
	enable - "true" to enable, "false" to disable

******************/
void BHDCPlib_Hdcp2x_EnableEncryption(
	const BHDCPlib_Handle hHDCPlib,
	const bool enable
);


/******************
Summary:
	Set Hdcp2.x encrypted key read from bin file

Input:
	hHDCPlib - HDCPlib handle that was previously open by BHDCPlib_Open
	pBuffer - pointer to buffer containing encrypted keys
	uiSize - size of buffer

******************/
BERR_Code BHDCPlib_Hdcp2x_SetBinKeys(
	const BHDCPlib_Handle hHDCPlib,
	const uint8_t *pBuffer,
	const uint32_t uiSize
);

/******************
Summary:
	Set Hdcp2.x Rx feature certificate file

Input:
	hHDCPlib - HDCPlib handle that was previously open by BHDCPlib_Open
	pBuffer - pointer to buffer containing the feature certificate
	uiSize - size of buffer

******************/
BERR_Code BHDCPlib_Hdcp2x_SetBinFeatCert(
	const BHDCPlib_Handle hHDCPlib,
	const uint8_t *pBuffer,
	const uint32_t uiSize
);

/******************
Summary: Retrieve Hdcp2.x ReceiverIdList
******************/
BERR_Code BHDCPlib_Hdcp2x_Tx_GetReceiverIdList(
	const BHDCPlib_Handle hHDCPlib,
	BHDCPlib_ReceiverIdListData *stReceiverIdListData
);


/******************
Summary: Upload ReceiverIdList to upstream Tx.
******************/
BERR_Code BHDCPlib_Hdcp2x_Rx_UploadReceiverIdList(
	const BHDCPlib_Handle hHDCPlib,
	const BHDCPlib_ReceiverIdListData *stReceiverIdListData
);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BHDCPLIB_HDCP2X_H__ */
