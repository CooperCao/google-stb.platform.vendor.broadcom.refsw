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

#ifndef BHDCPLIB_HDCP22_PRIV_H__
#define BHDCPLIB_HDCP22_PRIV_H__


#ifdef __cplusplus
extern "C" {
#endif

#define HDCP22_ERRORS_BASE								0x200

#define HDCP22_ERR_SUCCESS								BERR_SUCCESS
#define HDCP22_ERR_CANNOT_BOUND_MODULE_ID				HDCP22_ERRORS_BASE+1
#define HDCP22_ERR_INSUFFICIENT_BUFFER_RX_ID_LIST		HDCP22_ERRORS_BASE+2
#define HDCP22_ERR_INVALID_BINFILEMANAGER_HANDLE		HDCP22_ERRORS_BASE+3
#define HDCP22_ERR_CANNOT_SEND_INDICATION_TO_HOST		HDCP22_ERRORS_BASE+4
#define HDCP22_ERR_SENDING_AKE_INIT						HDCP22_ERRORS_BASE+5
#define HDCP22_ERR_CHECKING_AUTOI2C_HW_TIMERS			HDCP22_ERRORS_BASE+6
#define HDCP22_ERR_INVALID_CORE_ID						HDCP22_ERRORS_BASE+7
#define HDCP22_ERR_CANNOT_SEND_RECEIVER_ID_LIST			HDCP22_ERRORS_BASE+8
#define HDCP22_ERR_LOADING_CRYPTO_SECRET				HDCP22_ERRORS_BASE+9
#define HDCP22_ERR_OPEN_BINFILEMANGER_HANDLE			HDCP22_ERRORS_BASE+10
#define HDCP22_ERR_UPSTREAM_UNAUTHENTICATED				HDCP22_ERRORS_BASE+11
#define HDCP22_ERR_INVALID_HOST_REQUEST					HDCP22_ERRORS_BASE+12


/*********************************
This header file define structures/definitions
shares between HDCPLIB and SAGE HDCP22 Platform
**********************************/
/* define SAGE Hdcp22 module Id */
typedef enum Hdcp22_ModuleId
{
	Hdcp22_ModuleId_eTx = 0x1,
	Hdcp22_ModuleId_eRx

} Hdcp22_ModuleId;


/* define SAGE Hdcp22 module CommandIds (Tx and Rx) */
typedef enum Hdcp22_CommandId
{
	Hdcp22_CommandId_Tx_eStartAuthentication = 0x1,
	Hdcp22_CommandId_Tx_eStopAuthentication,
	Hdcp22_CommandId_Tx_eGetReceiverIdlist,
	Hdcp22_CommandId_Rx_eSetBinFile,
	Hdcp22_CommandId_Rx_eSendReceiverIdList
} Hdcp22_CommandId;


typedef enum Hdcp22_IndicationType
{
	Hdcp22_IndicationType_eAuthenticationStatus = 0x1,
	Hdcp22_IndicationType_eAuthenticationError,
	Hdcp22_IndicationType_eContentStreamTypeUpdate,
	Hdcp22_IndicationType_eSecurityViolation
} Hdcp22_IndicationType;


typedef enum Hdcp22_AuthenticationStatus
{
	Hdcp22_AuthenticationStatus_eUnAuthenticated,
	Hdcp22_AuthenticationStatus_eSessionKeyLoaded,
	Hdcp22_AuthenticationStatus_eAuthenticated,
	Hdcp22_AuthenticationStatus_eRxAuthenticated,
	Hdcp22_AuthenticationStatus_eRepeaterAuthenticated,
	Hdcp22_AuthenticationStatus_eReadyToDisableKeyRamSerial,
	Hdcp22_AuthenticationStatus_eAutoI2CTimerUnavailable
} Hdcp22_AuthenticationStatus;


typedef enum Hdcp22_AuthenticationError
{
	Hdcp22_AuthenticationError_eSuccess,
	Hdcp22_AuthenticationError_eInvalidContainerProvided,
	Hdcp22_AuthenticationError_eInvalidPointerProvided,
	Hdcp22_AuthenticationError_eInvalidBinFileManagerHandle,

	Hdcp22_AuthenticationError_eGenerateRtx,
	Hdcp22_AuthenticationError_eGenerateRrx,
	Hdcp22_AuthenticationError_eGettingTxCaps,
	Hdcp22_AuthenticationError_eGettingRxCaps,
	Hdcp22_AuthenticationError_eSendingReAuthReq,

	Hdcp22_AuthenticationError_eAllocateMsgStorage,
	Hdcp22_AuthenticationError_eSendMsgViaProxy,
	Hdcp22_AuthenticationError_eOutOfLocalSramMemory,

	Hdcp22_AuthenticationError_eProtocalMsgExchangeTimeout,
	Hdcp22_AuthenticationError_eTimeoutAKESendCert,
	Hdcp22_AuthenticationError_eTimeoutAKESendHPrime,
	Hdcp22_AuthenticationError_eTimeoutAKESendPairingInfo,
	Hdcp22_AuthenticationError_eTimeoutLCSendLPrime,
	Hdcp22_AuthenticationError_eTimeoutSKESendEks,
	Hdcp22_AuthenticationError_eTimeoutRepeaterAuthSendReceiverIdList,
	Hdcp22_AuthenticationError_eTimeoutRepeaterAuthSendAck,
	Hdcp22_AuthenticationError_eTimeoutRepeaterAuthStreamReady,

	Hdcp22_AuthenticationError_eGettingRxCertificate,
	Hdcp22_AuthenticationError_eInvalidRxCertificateLength,
	Hdcp22_AuthenticationError_eCalculateDkey,
	Hdcp22_AuthenticationError_eGenerateKm,
	Hdcp22_AuthenticationError_eEncrypteKm,
	Hdcp22_AuthenticationError_eDecrypteNoStoredKm,
	Hdcp22_AuthenticationError_eDecrypteStoredKm,
	Hdcp22_AuthenticationError_eInvalidReceiverID,
	Hdcp22_AuthenticationError_eRevokedReceiverIDFound,
	Hdcp22_AuthenticationError_eInvalidRxCertSignature,
	Hdcp22_AuthenticationError_eComputeHValue,
	Hdcp22_AuthenticationError_eComputeHPrimeValue,
	Hdcp22_AuthenticationError_eHValueMismatch,
	Hdcp22_AuthenticationError_eGenerateRn,
	Hdcp22_AuthenticationError_eComputeLValue,
	Hdcp22_AuthenticationError_eComputeLPrimeValue,
	Hdcp22_AuthenticationError_eLocalityCheckFailed,
	Hdcp22_AuthenticationError_eGenerateKs,
	Hdcp22_AuthenticationError_eGenerateRiv,
	Hdcp22_AuthenticationError_eEncryptKs,
	Hdcp22_AuthenticationError_eDecryptKs,
	Hdcp22_AuthenticationError_eLoadSessionKey,
	Hdcp22_AuthenticationError_eMaxCascadeExceeded,
	Hdcp22_AuthenticationError_eMaxDeviceExceeded,
	Hdcp22_AuthenticationError_eInvalidSeqNumV,
	Hdcp22_AuthenticationError_eComputeVValue,
	Hdcp22_AuthenticationError_eComputeVPrimeValue,
	Hdcp22_AuthenticationError_eVValueMismatch,
	Hdcp22_AuthenticationError_eSeqNumMRollOver,
	Hdcp22_AuthenticationError_eComputeMValue,
	Hdcp22_AuthenticationError_eComputeMPrimeValue,
	Hdcp22_AuthenticationError_eMValueMismatch,

	Hdcp22_AuthenticationError_eIncorrectSizeAKEInit,
	Hdcp22_AuthenticationError_eIncorrectSizeAKESendCert,
	Hdcp22_AuthenticationError_eIncorrectSizeAKENoStoredKm,
	Hdcp22_AuthenticationError_eIncorrectSizeAKEStoredKm,
	Hdcp22_AuthenticationError_eIncorrectSizeAKESendHPrime,
	Hdcp22_AuthenticationError_eIncorrectSizeSendPairingInfo,
	Hdcp22_AuthenticationError_eIncorrectSizeLCInit,
	Hdcp22_AuthenticationError_eIncorrectSizeLCSendLPrime,
	Hdcp22_AuthenticationError_eIncorrectSizeSKESendEks,
	Hdcp22_AuthenticationError_eIncorrectSizeRepeaterAuthSendAck,
	Hdcp22_AuthenticationError_eIncorrectSizeRepeaterAuthStreamReady,
	Hdcp22_AuthenticationError_eIncorrectSizeRepeaterAuthStreamManage
} Hdcp22_AuthenticationError;

typedef enum Hdcp22_SecurityViolationError
{
	Hdcp22_SecurityViolationError_eNone,
	Hdcp22_SecurityViolationError_eHdcpLevel,
	Hdcp22_SecurityViolationError_eSvpTA
} Hdcp22_SecurityViolationError;

typedef enum Hdcp22_ContentStreamType
{
	Hdcp22_ContentStreamType_eType0 = 0,
	Hdcp22_ContentStreamType_eType1,
	Hdcp22_ContentStreamType_eReserved,
	Hdcp22_ContentStreamType_eMax
} Hdcp22_ContentStreamType;


#ifdef __cplusplus
}
#endif

#endif /* BHDCPLIB_HDCP22_PRIV_H__ */
