/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include "bhdcplib.h"
#include "bhdcplib_priv.h"
#include "bhdcplib_hdcp22_priv.h"

#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
#include "bsagelib.h"
#include "bsagelib_rai.h"
#include "bsagelib_client.h"
#include "bhdm_auto_i2c.h"
#if BHDCPLIB_HDR_SUPPORT
#include "bhdr_hdcp.h"
#endif
#endif


BDBG_MODULE(bhdcplib_hdcp2x);


#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)

static BERR_Code BHDCPlib_P_Hdcp2x_ModuleInit(const BHDCPlib_Handle hHDCPlib);

static BERR_Code BHDCPlib_P_Hdcp2x_SendSageCommand(
	const BHDCPlib_Handle hHDCPlib,
	const Hdcp22_CommandId eCommandId
);

static BERR_Code BHDCPlib_P_Hdcp2x_ProcessRequest(
	const BHDCPlib_Handle hHDCPlib,
	BHDCPlib_P_Hdcp2xRequest eHdcp2xRequest
);


/***********************************
*** PRIVATE APIs
************************************/
/* DEBUG */
typedef struct
{
	BHDCPlib_Hdcp2xState eState ;
	const char *hdcp2xStateText ;
}  BHDCPlib_Hdcp2x_StateText ;

static BHDCPlib_Hdcp2x_StateText  hdcp2xStateTextTable[] =
{
	{BHDCPlib_Hdcp2xState_eSagePlatformOpen,		BDBG_STRING("OpenSAGEPlatform")},
	{BHDCPlib_Hdcp2xState_eSagePlatformInit,		BDBG_STRING("InitializeSAGEPlatform")},
	{BHDCPlib_Hdcp2xState_eSageModuleInit,	BDBG_STRING("InitializeSAGEModule")},
	{BHDCPlib_Hdcp2xState_eUnauthenticated,		BDBG_STRING("Unauthenticated")},
	{BHDCPlib_Hdcp2xState_eAuthenticated, 	BDBG_STRING("Authenticated")},
	{BHDCPlib_Hdcp2xState_eSystemCannotInitialize,	BDBG_STRING("SAGESystemCannotInitialize")}
};

const char * BHDCPlib_Hdcp2x_StateToStr(
	BHDCPlib_Hdcp2xState eState)
{
	uint8_t i=0;

	for (i=0; i<sizeof(hdcp2xStateTextTable)/sizeof(BHDCPlib_Hdcp2x_StateText); i++) {
		if (hdcp2xStateTextTable[i].eState != eState)
			continue;

		return hdcp2xStateTextTable[i].hdcp2xStateText;
	}

	return BDBG_STRING("Invalid Hdcp2x State");

}

typedef struct
{
	uint32_t hdcp2xResponseError;
	const char *hdcp2xResponseErrorText ;
}  BHDCPlib_Hdcp2x_ResponseErrorText ;

static BHDCPlib_Hdcp2x_ResponseErrorText  hdcp2xResponseErrorTextTable[] =
{
	{HDCP22_ERR_SUCCESS,			BDBG_STRING("SUCCESS")},
	{HDCP22_ERR_CANNOT_BOUND_MODULE_ID,			BDBG_STRING("Cannot Bound Module ID in SAGE framework")},
	{HDCP22_ERR_INSUFFICIENT_BUFFER_RX_ID_LIST,			BDBG_STRING("Provided buffer is too short for receiver ID List")},
	{HDCP22_ERR_INVALID_BINFILEMANAGER_HANDLE,			BDBG_STRING("Invalid Binfile Manager Handle")},
	{HDCP22_ERR_CANNOT_SEND_INDICATION_TO_HOST,			BDBG_STRING("Could not send indication from SAGE to HOST")},
	{HDCP22_ERR_SENDING_AKE_INIT, 		BDBG_STRING("Error sending AKE_INIT message to kick start authentication process")},
	{HDCP22_ERR_CHECKING_AUTOI2C_HW_TIMERS, 		BDBG_STRING("Error checking availability of AUTO I2C HW Timers")},
	{HDCP22_ERR_INVALID_CORE_ID,		BDBG_STRING("Invalid HDMI Core ID")},
	{HDCP22_ERR_CANNOT_SEND_RECEIVER_ID_LIST,		BDBG_STRING("Cannot send Receiver ID List")},
	{HDCP22_ERR_LOADING_CRYPTO_SECRET,		BDBG_STRING("Error loading crypto secret values")},
	{HDCP22_ERR_OPEN_BINFILEMANGER_HANDLE,		BDBG_STRING("Error opening SAGE BinFileManager Handle")},
	{HDCP22_ERR_UPSTREAM_UNAUTHENTICATED,		BDBG_STRING("Invalid Request to SendReceiverIdList. Not yet authenticated with upstream transmitter")},
	{HDCP22_ERR_INVALID_HOST_REQUEST,		BDBG_STRING("Invalid command/request from host")}
};

const char * BHDCPlib_Hdcp2x_P_ResponseErrorToStr(
	uint32_t responseError)
{
	uint8_t i=0;

	for (i=0; i<sizeof(hdcp2xResponseErrorTextTable)/sizeof(BHDCPlib_Hdcp2x_ResponseErrorText); i++) {
		if (hdcp2xResponseErrorTextTable[i].hdcp2xResponseError != responseError)
			continue;

		return hdcp2xResponseErrorTextTable[i].hdcp2xResponseErrorText;
	}

	return BDBG_STRING("Invalid Hdcp2x Response Error returned from SAGE");
}


typedef struct
{
	Hdcp22_AuthenticationError eAuthenticationError;
	const char *hdcp2xAuthErrorText ;
}  BHDCPlib_Hdcp2x_AuthenticationErrorText ;

static BHDCPlib_Hdcp2x_AuthenticationErrorText hdcp2xAuthenticationErrorTextTable[] =
{
	{Hdcp22_AuthenticationError_eSuccess, 		BDBG_STRING("SUCCESS - No Authentication Error")},
	{Hdcp22_AuthenticationError_eInvalidContainerProvided,			BDBG_STRING("Invalid SAGE container provided")},
	{Hdcp22_AuthenticationError_eInvalidPointerProvided,			BDBG_STRING("Invalid pointer provided in SAGE container")},
	{Hdcp22_AuthenticationError_eInvalidBinFileManagerHandle,		BDBG_STRING("Invalid SAGE BinFileManager Handle - Most likely HDCP2.2 Rx Keys are invalid or not provided")},

	{Hdcp22_AuthenticationError_eGenerateRtx,			BDBG_STRING("Error generating RTX")},
	{Hdcp22_AuthenticationError_eGenerateRrx,			BDBG_STRING("Error generating RRX")},
	{Hdcp22_AuthenticationError_eGettingTxCaps,			BDBG_STRING("Error getting TxCaps")},
	{Hdcp22_AuthenticationError_eGettingRxCaps,			BDBG_STRING("Error getting RxCaps")},
	{Hdcp22_AuthenticationError_eSendingReAuthReq, 		BDBG_STRING("Error sending REAUTH_REQ")},

	{Hdcp22_AuthenticationError_eAllocateMsgStorage, 		BDBG_STRING("Error allocating buffer to hold protocol message")},
	{Hdcp22_AuthenticationError_eSendMsgViaProxy,		BDBG_STRING("Cannot send message due to error from sage framework")},
	{Hdcp22_AuthenticationError_eOutOfLocalSramMemory,		BDBG_STRING("Out of SRAM memory")},

	{Hdcp22_AuthenticationError_eProtocalMsgExchangeTimeout,	BDBG_STRING("A time out occurred while exchange HDCP2.x protocal messages")},
	{Hdcp22_AuthenticationError_eTimeoutAKESendCert,	BDBG_STRING("A time out occurred while waiting for AKE_Send_Cert message")},
	{Hdcp22_AuthenticationError_eTimeoutAKESendHPrime,	BDBG_STRING("A time out occurred while waiting for AKE_Send_HPrime message")},
	{Hdcp22_AuthenticationError_eTimeoutAKESendPairingInfo,		BDBG_STRING("A time out occurred while waiting for AKE_Send_PairingInfo message")},
	{Hdcp22_AuthenticationError_eTimeoutLCSendLPrime, 	BDBG_STRING("A time out occurred while waiting for LC_Send_LPrime message")},
	{Hdcp22_AuthenticationError_eTimeoutSKESendEks,		BDBG_STRING("A time out occurred while waiting for completion of SKE_Send_Eks")},
	{Hdcp22_AuthenticationError_eTimeoutRepeaterAuthSendReceiverIdList, 	BDBG_STRING("A time out occurred while waiting for RepeaterAuth_Send_ReceiverId_List")},
	{Hdcp22_AuthenticationError_eTimeoutRepeaterAuthSendAck,		BDBG_STRING("A time out occurred while waiting for completion of RepeaterAuth_Send_Ack")},
	{Hdcp22_AuthenticationError_eTimeoutRepeaterAuthStreamReady,		BDBG_STRING("A time out occurred while waiting for RepeaterAuth_Stream_Ready")},

	{Hdcp22_AuthenticationError_eGettingRxCertificate,			BDBG_STRING("Error getting Rx Certificate")},
	{Hdcp22_AuthenticationError_eInvalidRxCertificateLength,	BDBG_STRING("Invalid Rx Certificate Length")},
	{Hdcp22_AuthenticationError_eCalculateDkey,			BDBG_STRING("Error calculating Dkey")},
	{Hdcp22_AuthenticationError_eGenerateKm,			BDBG_STRING("Error generating Km")},
	{Hdcp22_AuthenticationError_eEncrypteKm,			BDBG_STRING("Error encrypting Km")},
	{Hdcp22_AuthenticationError_eDecrypteNoStoredKm,			BDBG_STRING("Error decrypt No-Stored Km")},
	{Hdcp22_AuthenticationError_eDecrypteStoredKm,			BDBG_STRING("Error decrypt Stored Km")},
	{Hdcp22_AuthenticationError_eInvalidReceiverID,			BDBG_STRING("Invalid Receiver ID")},
	{Hdcp22_AuthenticationError_eRevokedReceiverIDFound, 		BDBG_STRING("Receiver ID is revoked")},
	{Hdcp22_AuthenticationError_eInvalidRxCertSignature,		BDBG_STRING("Invalid Rx Certificate signature")},
	{Hdcp22_AuthenticationError_eComputeHValue,			BDBG_STRING("Error computing H value")},
	{Hdcp22_AuthenticationError_eComputeHPrimeValue, 		BDBG_STRING("Error computing H prime value")},
	{Hdcp22_AuthenticationError_eHValueMismatch, 		BDBG_STRING("Calculated H value does not match with Rx's value")},
	{Hdcp22_AuthenticationError_eGenerateRn,			BDBG_STRING("Error generating Rn")},
	{Hdcp22_AuthenticationError_eComputeLValue,			BDBG_STRING("Error computing L value")},
	{Hdcp22_AuthenticationError_eComputeLPrimeValue, 		BDBG_STRING("Error computing L Prime value")},
	{Hdcp22_AuthenticationError_eLocalityCheckFailed,			BDBG_STRING("Failed Locality Check")},
	{Hdcp22_AuthenticationError_eGenerateKs,			BDBG_STRING("Error generating Ks")},
	{Hdcp22_AuthenticationError_eGenerateRiv,			BDBG_STRING("Error generating Riv")},
	{Hdcp22_AuthenticationError_eEncryptKs,			BDBG_STRING("Error encrypting Ks")},
	{Hdcp22_AuthenticationError_eDecryptKs,			BDBG_STRING("Error decrypting Ks")},
	{Hdcp22_AuthenticationError_eLoadSessionKey,			BDBG_STRING("Error loading session key")},
	{Hdcp22_AuthenticationError_eMaxCascadeExceeded,			BDBG_STRING("Max Cascade Exceeded")},
	{Hdcp22_AuthenticationError_eMaxDeviceExceeded,			BDBG_STRING("Max Devices Exceeded")},
	{Hdcp22_AuthenticationError_eInvalidSeqNumV,			BDBG_STRING("Invalid SeqNumV")},
	{Hdcp22_AuthenticationError_eComputeVValue,			BDBG_STRING("Error computing V Value")},
	{Hdcp22_AuthenticationError_eComputeVPrimeValue, 		BDBG_STRING("Error computing V Prime Value")},
	{Hdcp22_AuthenticationError_eVValueMismatch,			BDBG_STRING("Calculated V value does not match with Rx's value")},
	{Hdcp22_AuthenticationError_eSeqNumMRollOver,			BDBG_STRING("SequenceNumM value rolled over")},
	{Hdcp22_AuthenticationError_eComputeMValue,			BDBG_STRING("Error computing M value")},
	{Hdcp22_AuthenticationError_eComputeMPrimeValue,			BDBG_STRING("Error computing M value")},
	{Hdcp22_AuthenticationError_eMValueMismatch,			BDBG_STRING("Computed M value does not match with Rx's value")},

	{Hdcp22_AuthenticationError_eIncorrectSizeAKEInit,			BDBG_STRING("Received AKE_Init message with incorrect size")},
	{Hdcp22_AuthenticationError_eIncorrectSizeAKESendCert,			BDBG_STRING("Received AKE_Send_Cert message with incorrect size")},
	{Hdcp22_AuthenticationError_eIncorrectSizeAKENoStoredKm,			BDBG_STRING("Received AKE_No_Stored_Km message with incorrect size")},
	{Hdcp22_AuthenticationError_eIncorrectSizeAKEStoredKm,			BDBG_STRING("Received AKE_Stored_Km message with incorrect size")},
	{Hdcp22_AuthenticationError_eIncorrectSizeAKESendHPrime,			BDBG_STRING("Received AKE_Send_H_Prime message with incorrect size")},
	{Hdcp22_AuthenticationError_eIncorrectSizeSendPairingInfo,			BDBG_STRING("Received Send_Pairing_Info message with incorrect size")},
	{Hdcp22_AuthenticationError_eIncorrectSizeLCInit,			BDBG_STRING("Received LC_Init message with incorrect size")},
	{Hdcp22_AuthenticationError_eIncorrectSizeLCSendLPrime,			BDBG_STRING("Received LC_Send_L_Prime message with incorrect size")},
	{Hdcp22_AuthenticationError_eIncorrectSizeSKESendEks,			BDBG_STRING("Received SKE_Send_Eks message with incorrect size")},
	{Hdcp22_AuthenticationError_eIncorrectSizeRepeaterAuthSendAck,			BDBG_STRING("Received RepeaterAuthSendAck message with incorrect size")},
	{Hdcp22_AuthenticationError_eIncorrectSizeRepeaterAuthStreamReady,			BDBG_STRING("Received RepeaterAuthStreamReady message with incorrect size")},
	{Hdcp22_AuthenticationError_eIncorrectSizeRepeaterAuthStreamManage,			BDBG_STRING("Received RepeaterAuthStreamManage message with incorrect size")}
};

const char * BHDCPlib_Hdcp2x_P_AuthenticationErrorToStr(
	Hdcp22_AuthenticationError eAuthenticationError
)
{
	uint8_t i=0;

	for (i=0; i<sizeof(hdcp2xAuthenticationErrorTextTable)/sizeof(BHDCPlib_Hdcp2x_AuthenticationErrorText); i++) {
		if (hdcp2xAuthenticationErrorTextTable[i].eAuthenticationError != eAuthenticationError)
			continue;

		return hdcp2xAuthenticationErrorTextTable[i].hdcp2xAuthErrorText;
	}

	return BDBG_STRING("Invalid Hdcp2x Authentication Error");
}


static void BHDCPlib_P_Hdcp2x_CleanSageContainer(BHDCPlib_Handle hHDCPlib)
{
	uint8_t i;

	if (hHDCPlib->sageContainer == NULL) return;

	BKNI_Memset(hHDCPlib->sageContainer->basicIn, 0, BSAGE_CONTAINER_MAX_BASICIN);
	BKNI_Memset(hHDCPlib->sageContainer->basicOut, 0, BSAGE_CONTAINER_MAX_BASICOUT);
	for (i=0; i < BSAGE_CONTAINER_MAX_SHARED_BLOCKS; i++)
	{
		if (hHDCPlib->sageContainer->blocks[i].data.ptr != NULL)
		{
			/* Don't clear the memory location holding the drm bin file, just reset container pointer */
			if (hHDCPlib->sageContainer->blocks[i].data.ptr != hHDCPlib->stBinKey.pBuffer) {
				BSAGElib_Rai_Memory_Free(hHDCPlib->stDependencies.hSagelibClientHandle,
											hHDCPlib->sageContainer->blocks[i].data.ptr);
			}

			hHDCPlib->sageContainer->blocks[i].data.ptr = NULL;
			hHDCPlib->sageContainer->blocks[i].len = 0;
		}
	}

	return;
}


static BERR_Code BHDCPlib_P_Hdcp2x_WaitForSage(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;
	BSAGElib_ResponseData data;
	BHDCPlib_SageResponseData stSageResponseData;

	if (hHDCPlib->uiLastAsyncId == 0)
	{
		rc = BERR_SUCCESS;
		goto done;
	}

	for (;;)
	{
		/* Wait for response from sage - Event will be set by NEXUS */
		rc = BKNI_WaitForEvent(hHDCPlib->stDependencies.sageResponseReceivedEvent, BHDCPLIB_HDCP2X_SAGERESPONSE_TIMEOUT);
		if (rc == BERR_TIMEOUT)
		{
			BDBG_ERR(("%s: Timeout (%dms) waiting for sage response from previous request",
				__FUNCTION__, BHDCPLIB_HDCP2X_SAGERESPONSE_TIMEOUT));
			rc = BERR_TRACE(rc);
			goto done;
		}
		else if (rc) {
			rc = BERR_TRACE(rc);
			goto done;
		}

		/* Get Sage response */
		rc = BSAGElib_Client_GetResponse(hHDCPlib->stDependencies.hSagelibClientHandle, &data);
		if (rc == BERR_SUCCESS) {
			break;
		}
	}

	stSageResponseData.rpcRemoteHandle = data.remote;
	stSageResponseData.async_id = data.async_id;
	stSageResponseData.error = data.rc;
	rc = BHDCPlib_Hdcp2x_ReceiveSageResponse(hHDCPlib, &stSageResponseData);
	if (rc != BERR_SUCCESS)
	{
		rc = BERR_TRACE(rc);
		goto done;
	}

done:
	/* clean up SAGE container if error */
	if (rc != BERR_SUCCESS) {
		BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);
	}

	return rc;
}


/*****************************************************/

static BERR_Code BHDCPlib_P_Hdcp2x_ModuleInit(const BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;
	Hdcp22_ModuleId eHdcp22ModuleId;

	BDBG_ENTER(BHDCPlib_P_Hdcp2x_ModuleInit);

	/* reconfigure SageContainer for next SAGE communications */
	BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);


	if (hHDCPlib->stDependencies.eCoreType == BHDCPlib_CoreType_eRx)
	{
#if BHDCPLIB_HDR_SUPPORT
		BHDR_Settings hdrSettings;

		rc = BHDR_GetSettings(hHDCPlib->stDependencies.hHdr, &hdrSettings);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Error getting BHDR_Settings"));
			BERR_TRACE(rc);
			goto done;
		}

		hHDCPlib->sageContainer->basicIn[0] = hdrSettings.eCoreId;

		eHdcp22ModuleId = Hdcp22_ModuleId_eRx;
		BDBG_MSG(("Ready to initialize Rx Module"));
#else
        return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
	}
	else
	{
		BHDM_Settings hdmSettings;

		/* Get Hdmi coreId */
		rc = BHDM_GetHdmiSettings(hHDCPlib->stDependencies.hHdm, &hdmSettings);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Error getting BHDM_Settings"));
			BERR_TRACE(rc);
			goto done;
		}

		hHDCPlib->sageContainer->basicIn[0] = hdmSettings.eCoreId;
		eHdcp22ModuleId = Hdcp22_ModuleId_eTx;
	}

	/* Initialize Sage Hdcp2x module instance */
	rc = BSAGElib_Rai_Module_Init(hHDCPlib->hSagelibRpcPlatformHandle, eHdcp22ModuleId,
						hHDCPlib->sageContainer, &hHDCPlib->hSagelibRpcModuleHandle,	/*out */
						(void *) hHDCPlib, &hHDCPlib->uiLastAsyncId /*out */);

	BDBG_MSG(("Initialize SAGE Hdcp2x Module: receivedSageModuleHandle [0x%x], assignedAsyncId [0x%x]",
		hHDCPlib->hSagelibRpcModuleHandle, hHDCPlib->uiLastAsyncId));

	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error initializing SAGE hdcp2.2 module, hdcplib(0x%x), error [0x%x] '%s'",
			hHDCPlib,rc, BSAGElib_Tools_ReturnCodeToString(rc)));
		BERR_TRACE(rc);
		goto done;
	}

done:
	if (rc != BERR_SUCCESS)
	{
		/* Uninitialized Sage HDCP22 Tx module */
		if (hHDCPlib->hSagelibRpcModuleHandle != NULL)
		{
			BSAGElib_Rai_Module_Uninit(hHDCPlib->hSagelibRpcModuleHandle, &hHDCPlib->uiLastAsyncId);
			BHDCPlib_P_Hdcp2x_WaitForSage(hHDCPlib);
			BSAGElib_Rpc_RemoveRemote(hHDCPlib->hSagelibRpcModuleHandle);
			BDBG_MSG(("Uninit & remove Hdcp2x SAGE Module: assignedAsyncId [0x%x]", hHDCPlib->uiLastAsyncId));
			hHDCPlib->hSagelibRpcModuleHandle = NULL;
		}
	}

	BDBG_LEAVE(BHDCPlib_P_Hdcp2x_ModuleInit);
	return rc;
}


static BERR_Code BHDCPlib_P_Hdcp2x_SendSageCommand(
	const BHDCPlib_Handle hHDCPlib,
	const Hdcp22_CommandId eCommandId
)
{
	BERR_Code rc = BERR_SUCCESS;
	uint8_t *revokedReceiverIdList = NULL;

	BDBG_ENTER(BHDCPlib_P_Hdcp2x_SendSageCommand);

	switch (eCommandId)
	{
	case Hdcp22_CommandId_Tx_eStartAuthentication:
		/* Kick-start HDCP2.2 cipher (set version, signified AKE_INIT message etc. in HW) */
		BHDM_HDCP_KickStartHdcp2xCipher(hHDCPlib->stDependencies.hHdm);

		/* reconfigure SageContainer for next SAGE communications */
		BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);

		/***
			TODO:
			+ Add settings to allow application to control these settings
			+ Only immediateRetryCount is supported
		***/
		/* specify number of retries and the intervals */
		hHDCPlib->sageContainer->basicIn[0] = 1; /* immediate retry count */
		hHDCPlib->sageContainer->basicIn[1] = 500; /* medium interval, in ms */
		hHDCPlib->sageContainer->basicIn[2] = 0; /* medium interval retry count */
		hHDCPlib->sageContainer->basicIn[3] = 2000; /* long interval, in ms */
		hHDCPlib->sageContainer->basicIn[4] = 0; /* long interval retry count */

		/* specify number of *locality check* retry - up to 1023 per HDCP 2.2 spec
		Each locality check process/phase takes about 20ms to complete, i.e. 25x20=500ms */
		hHDCPlib->sageContainer->basicIn[5] = 25;

		/* pass the ContentStreamControl to hdcp22 Module in SAGE to use in RASM message */
		switch (hHDCPlib->stHdcpConfiguration.eHdcp2xContentStreamControl)
		{
		case BHDCPlib_Hdcp2xContentStreamType_eType0:
			hHDCPlib->sageContainer->basicIn[6] = Hdcp22_ContentStreamType_eType0;
			break;

		case BHDCPlib_Hdcp2xContentStreamType_eType1:
			hHDCPlib->sageContainer->basicIn[6] = Hdcp22_ContentStreamType_eType1;
			break;

		default:
			hHDCPlib->sageContainer->basicIn[6] = Hdcp22_ContentStreamType_eReserved;
			break;
		}

		/* Pass binFile */
		if ((hHDCPlib->stBinKey.pBuffer == NULL) || (hHDCPlib->stBinKey.uiSize == 0))
		{
			BDBG_ERR(("Invalid HDCP2.x keys buffer or size"));
			rc = BERR_INVALID_PARAMETER;
			BERR_TRACE(rc);
			goto done;
		}

		hHDCPlib->sageContainer->blocks[0].data.ptr = hHDCPlib->stBinKey.pBuffer;
		hHDCPlib->sageContainer->blocks[0].len = hHDCPlib->stBinKey.uiSize;

		if ((hHDCPlib->RevokedKsvList.uiNumRevokedKsvs > 0) && (hHDCPlib->RevokedKsvList.Ksvs != NULL))
		{
			/* Allocate memory to hold revoked ReceiverId List */
			revokedReceiverIdList = BSAGElib_Rai_Memory_Allocate(hHDCPlib->stDependencies.hSagelibClientHandle,
					hHDCPlib->RevokedKsvList.uiNumRevokedKsvs*BHDCPLIB_HDCP2X_RECEIVERID_LENGTH, BSAGElib_MemoryType_Global);
			if (revokedReceiverIdList == NULL)
			{
				BDBG_ERR(("Error allocating BSAGElib_InOutContainer"));
				rc = BERR_OUT_OF_SYSTEM_MEMORY;
				BERR_TRACE(rc);
				goto done;
			}

			/* copy revokedReceiverId data */
			BKNI_Memcpy(revokedReceiverIdList, hHDCPlib->RevokedKsvList.Ksvs,
									hHDCPlib->RevokedKsvList.uiNumRevokedKsvs * BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);

			/* load container block */
			hHDCPlib->sageContainer->blocks[1].data.ptr = revokedReceiverIdList;
			hHDCPlib->sageContainer->blocks[1].len =
						(hHDCPlib->RevokedKsvList.uiNumRevokedKsvs*BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);
		}
		else {
			/* populate 1 entry list to maintain consistency with SAGE interface */
			revokedReceiverIdList = BSAGElib_Rai_Memory_Allocate(hHDCPlib->stDependencies.hSagelibClientHandle,
					1 * BHDCPLIB_HDCP2X_RECEIVERID_LENGTH, BSAGElib_MemoryType_Global);
			if (revokedReceiverIdList == NULL)
			{
				BDBG_ERR(("Error allocating BSAGElib_InOutContainer"));
				rc = BERR_OUT_OF_SYSTEM_MEMORY;
				BERR_TRACE(rc);
				goto done;
			}

			revokedReceiverIdList[0] = 0xFF;
			revokedReceiverIdList[1] = 0xFF;
			revokedReceiverIdList[2] = 0xFF;
			revokedReceiverIdList[3] = 0xFF;
			revokedReceiverIdList[4] = 0xFF;

			/* load container block */
			hHDCPlib->sageContainer->blocks[1].data.ptr = revokedReceiverIdList;
			hHDCPlib->sageContainer->blocks[1].len = 1 * BHDCPLIB_HDCP2X_RECEIVERID_LENGTH;
		}


		/* Send command to SAGE request to start HDCP authentication */
		rc = BSAGElib_Rai_Module_ProcessCommand(hHDCPlib->hSagelibRpcModuleHandle,
				Hdcp22_CommandId_Tx_eStartAuthentication, hHDCPlib->sageContainer, &hHDCPlib->uiLastAsyncId);
		BDBG_MSG(("Sending command to SAGE: sageModuleHandle [0x%x], commandId [%d], assignedAsyncId [0x%x]",
			hHDCPlib->hSagelibRpcModuleHandle, Hdcp22_CommandId_Tx_eStartAuthentication, hHDCPlib->uiLastAsyncId));

		/* Save StartAuthentication async Id */
		hHDCPlib->uiStartAuthenticationId = hHDCPlib->uiLastAsyncId;
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Error sending command to SAGE Hdcp2x Tx Module - hdcplib(0x%x), error [0x%x] '%s'",
				hHDCPlib, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
			BERR_TRACE(BERR_OS_ERROR);
			goto done;
		}

		break;


	case Hdcp22_CommandId_Tx_eStopAuthentication:
		/* reconfigure SageContainer for next SAGE communications */
		BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);

		rc = BSAGElib_Rai_Module_ProcessCommand(hHDCPlib->hSagelibRpcModuleHandle,
				Hdcp22_CommandId_Tx_eStopAuthentication, hHDCPlib->sageContainer, &hHDCPlib->uiLastAsyncId);

		BDBG_MSG(("Sending command to SAGE: sageModuleHandle [0x%x], commandId [%d], assignedAsyncId [0x%x]",
			hHDCPlib->hSagelibRpcModuleHandle, Hdcp22_CommandId_Tx_eStopAuthentication, hHDCPlib->uiLastAsyncId));

		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Error sending command to SAGE Hdcp2x Tx Module - hdcplib(0x%x), error [0x%x] '%s'",
				hHDCPlib, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
			BERR_TRACE(BERR_OS_ERROR);
			goto done;
		}

		break;


	case Hdcp22_CommandId_Tx_eGetReceiverIdlist:
		/* reconfigure SageContainer for next SAGE communications */
		BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);

		/* Allocate memory for receiverIdList */
		hHDCPlib->sageContainer->blocks[0].data.ptr =
				BSAGElib_Rai_Memory_Allocate(hHDCPlib->stDependencies.hSagelibClientHandle,
					BHDCPLIB_HDCP2X_RECEIVERID_LENGTH*BHDCPLIB_HDCP2X_MAX_DEVICE_COUNT, BSAGElib_MemoryType_Global);
		if (hHDCPlib->sageContainer->blocks[0].data.ptr == NULL)
		{
			BDBG_ERR(("Error allocating memory for receiverIdList"));
			rc = BERR_OUT_OF_SYSTEM_MEMORY;
			BERR_TRACE(rc);
			goto done;
		}

		hHDCPlib->sageContainer->blocks[0].len = BHDCPLIB_HDCP2X_RECEIVERID_LENGTH*BHDCPLIB_HDCP2X_MAX_DEVICE_COUNT;


		rc = BSAGElib_Rai_Module_ProcessCommand(hHDCPlib->hSagelibRpcModuleHandle,
				Hdcp22_CommandId_Tx_eGetReceiverIdlist, hHDCPlib->sageContainer, &hHDCPlib->uiLastAsyncId);

		BDBG_MSG(("Sending command to SAGE: sageModuleHandle [0x%x], commandId [%d], assignedAsyncId [0x%x]",
			hHDCPlib->hSagelibRpcModuleHandle, Hdcp22_CommandId_Tx_eGetReceiverIdlist, hHDCPlib->uiLastAsyncId));

		/* Save GetReceiverIdList async Id */
		hHDCPlib->uiGetReceiverIdListId = hHDCPlib->uiLastAsyncId;
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Error sending command to SAGE Hdcp2x Tx Module - hdcplib(0x%x), error [0x%x] '%s'",
				hHDCPlib, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
			rc = BERR_OS_ERROR;
			BERR_TRACE(rc);
			goto done;
		}

		break;

	case Hdcp22_CommandId_Rx_eSetBinFile:
	case Hdcp22_CommandId_Rx_eSendReceiverIdList:
		break;

	}

done:

	if (rc != BERR_SUCCESS)
	{
		/* clean up sage container and free any allocated memory blocks */
		BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);
	}

	BDBG_LEAVE(BHDCPlib_P_Hdcp2x_SendSageCommand);
	return rc;
}


BERR_Code BHDCPlib_P_Hdcp2x_Open(BHDCPlib_Handle *hHDCPlib, const BHDCPlib_Dependencies *pstDependencies)
{
	BERR_Code rc = BERR_SUCCESS;
	BHDCPlib_Handle hHandle = NULL;

	BDBG_ENTER(BHDCPlib_P_Hdcp2x_Open);
	BDBG_ASSERT(pstDependencies->hSagelibClientHandle);


	if (pstDependencies->eCoreType == BHDCPlib_CoreType_eRx) {
#if BHDCPLIB_HDR_SUPPORT
		BDBG_ASSERT(pstDependencies->hHdr);

		/* Set RxCaps HW register to use later in the authentication process */
		rc = BHDR_HDCP_SetHdcp2xRxCaps(pstDependencies->hHdr, BHDCPLIB_HDCP22_RXCAPS_VERSION,
							BHDCPLIB_HDCP22_RXCAPS_RECEIVER_CAPABILITY_MASK, 1 /* Repeater */);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Error setting RxCaps"));
			rc = BERR_TRACE(rc);
			goto done;
		}
#else
        return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
	}
	else {
		BDBG_ASSERT(pstDependencies->hHdm);

		/* Set TxCaps HW register to use later in the authentication process */
		rc = BHDM_HDCP_SetHdcp2xTxCaps(pstDependencies->hHdm, BHDCPLIB_HDCP22_TXCAPS_VERSION,
							BHDCPLIB_HDCP22_TXCAPS_TRANSMITTER_CAPABILITY_MASK);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Error setting TxCaps"));
			rc = BERR_TRACE(rc);
			goto done;
		}
	}

	/* Alloc memory from the system */
	hHandle = BKNI_Malloc(sizeof(BHDCPlib_P_Handle));
	if (hHandle == NULL)
	{
		BDBG_ERR(("%s: BKNI_malloc() failed\n", __FUNCTION__));
		rc = BERR_OUT_OF_SYSTEM_MEMORY;
		BERR_TRACE(rc);
		goto done;
	}

	BKNI_Memset(hHandle, 0, sizeof(BHDCPlib_P_Handle));
	BDBG_OBJECT_SET(hHandle, HDCPLIB);
	hHandle->stDependencies = *pstDependencies;

	/* Allocate a single container and reuse throught out the life of HDCPlib handle */
	hHandle->sageContainer = BSAGElib_Rai_Container_Allocate(pstDependencies->hSagelibClientHandle);
	if (hHandle->sageContainer == NULL)
	{
		BDBG_ERR(("Error allocating BSAGElib_InOutContainer - hdcplib(0x%x)", hHDCPlib));
		rc = BERR_OUT_OF_SYSTEM_MEMORY;
		BERR_TRACE(rc);
		goto done;
	}

	/* Set initial state */
	hHandle->currentHdcp2xState = BHDCPlib_Hdcp2xState_eSagePlatformOpen;

	/* open platform */
	rc = BSAGElib_Rai_Platform_Open(pstDependencies->hSagelibClientHandle, BSAGE_PLATFORM_ID_HDCP22,
					hHandle->sageContainer, &hHandle->hSagelibRpcPlatformHandle, /*out */
					(void *)hHandle, &hHandle->uiLastAsyncId /*out */);
	BDBG_MSG(("Opening HDCP2x SAGE platform: sagePlatformHandle [0x%x], assignedAsyncId [0x%x]",
		hHandle->hSagelibRpcPlatformHandle, hHandle->uiLastAsyncId));

	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error opening SAGE HDCP22 Platform - hdcplib(0x%x), error [%x] '%s'",
			hHDCPlib, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
		BERR_TRACE(rc);
		goto done;
	}

	/* Initialize revoked receiverID list */
	hHandle->RevokedKsvList.Ksvs = NULL;
	hHandle->RevokedKsvList.uiNumRevokedKsvs = 0;

	/* Initialize hdcp22 BinKey struct */
	hHandle->stBinKey.pBuffer = NULL;
	hHandle->stBinKey.uiSize = 0;

	/* create indicationEvent */
	rc = BKNI_CreateEvent(&(hHandle->hdcp2xIndicationEvent));
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error creating hdcp2xIndicationEvent"));
		rc = BERR_TRACE(rc);
		goto done;
	}

	/* wait for sage response before proceeding */
	rc = BHDCPlib_P_Hdcp2x_WaitForSage(hHandle);
	if (rc != BERR_SUCCESS)
	{
		rc = BERR_TRACE(rc);
		goto done;
	}

	/* save handle */
	*hHDCPlib = hHandle;

done:
	if (rc != BERR_SUCCESS)	{
		if (hHandle != NULL)
		{
			/* free allocated container space */
			if (hHandle->sageContainer != NULL) {
				BSAGElib_Rai_Container_Free(pstDependencies->hSagelibClientHandle, hHandle->sageContainer);
				BKNI_Memset(hHandle->sageContainer, 0, sizeof(BSAGElib_InOutContainer));
			}

			/* Close HDCP22 platform handle */
			if (hHandle->hSagelibRpcPlatformHandle != NULL)
			{
				BSAGElib_Rai_Platform_Close(hHandle->hSagelibRpcPlatformHandle, &hHandle->uiLastAsyncId);
				BHDCPlib_P_Hdcp2x_WaitForSage(hHandle);
				BSAGElib_Rpc_RemoveRemote(hHandle->hSagelibRpcPlatformHandle);
				BDBG_MSG(("Close & remove Hdcp2x SAGE platform: assignedAsyncId [0x%x]", hHandle->uiLastAsyncId));
				hHandle->hSagelibRpcPlatformHandle = NULL;
			}

			BKNI_Free(hHandle);
			hHandle=NULL;
			*hHDCPlib=NULL;
		}
	}

	BDBG_LEAVE(BHDCPlib_P_Hdcp2x_Open);
	return rc ;

}


BERR_Code BHDCPlib_P_Hdcp2x_Close(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_P_Hdcp2x_Close);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);


	/* free memory associated with the Revoked KSV list, if any */
	if (hHDCPlib->RevokedKsvList.Ksvs)
	{
		BKNI_Free(hHDCPlib->RevokedKsvList.Ksvs);
		hHDCPlib->RevokedKsvList.Ksvs = NULL;
		hHDCPlib->RevokedKsvList.uiNumRevokedKsvs = 0;
	}

	if (hHDCPlib->sageContainer != NULL) {
		BSAGElib_Rai_Container_Free(hHDCPlib->stDependencies.hSagelibClientHandle, hHDCPlib->sageContainer);
		BKNI_Memset(hHDCPlib->sageContainer, 0, sizeof(BSAGElib_InOutContainer));
	}

	if (hHDCPlib->stBinKey.pBuffer != NULL) {
		BSAGElib_Rai_Memory_Free(hHDCPlib->stDependencies.hSagelibClientHandle, hHDCPlib->stBinKey.pBuffer);
		hHDCPlib->stBinKey.pBuffer = NULL;
		hHDCPlib->stBinKey.uiSize = 0;
	}

	/* Uninitialized Sage HDCP22 Tx module */
	if (hHDCPlib->hSagelibRpcModuleHandle != NULL)
	{
		BSAGElib_Rai_Module_Uninit(hHDCPlib->hSagelibRpcModuleHandle, &hHDCPlib->uiLastAsyncId);
		BHDCPlib_P_Hdcp2x_WaitForSage(hHDCPlib);
		BSAGElib_Rpc_RemoveRemote(hHDCPlib->hSagelibRpcModuleHandle);
		BDBG_MSG(("Uninit & remove Hdcp2x SAGE Module: assignedAsyncId [0x%x]", hHDCPlib->uiLastAsyncId));
		hHDCPlib->hSagelibRpcModuleHandle = NULL;
	}

	/* Close HDCP22 platform handle */
	if (hHDCPlib->hSagelibRpcPlatformHandle != NULL)
	{
		BSAGElib_Rai_Platform_Close(hHDCPlib->hSagelibRpcPlatformHandle, &hHDCPlib->uiLastAsyncId);
		BHDCPlib_P_Hdcp2x_WaitForSage(hHDCPlib);
		BSAGElib_Rpc_RemoveRemote(hHDCPlib->hSagelibRpcPlatformHandle);
		BDBG_MSG(("Close & remove Hdcp2x SAGE platform: assignedAsyncId [0x%x]", hHDCPlib->uiLastAsyncId));
		hHDCPlib->hSagelibRpcPlatformHandle = NULL;
	}

	if (hHDCPlib->hdcp2xIndicationEvent != NULL) {
		BKNI_DestroyEvent((hHDCPlib->hdcp2xIndicationEvent));
		hHDCPlib->hdcp2xIndicationEvent = NULL;
	}

	/* free memory associated with the HDMIlib Handle */
	BKNI_Memset(hHDCPlib, 0, sizeof(BHDCPlib_P_Handle));
	BDBG_OBJECT_DESTROY(hHDCPlib, HDCPLIB);
	BKNI_Free( (void *) hHDCPlib) ;
	hHDCPlib = NULL;


	BDBG_LEAVE(BHDCPlib_P_Hdcp2x_Close);
	return rc;
}


BERR_Code BHDCPlib_Hdcp2x_ProcessWatchDog(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_Hdcp2x_ProcessWatchDog);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	BDBG_WRN(("%s: Sage watchdog occurred. Process watchdog", __FUNCTION__));

	/* Free all SAGE RPC handles */
	/* Uninitialized Sage HDCP22 Tx module */
	if (hHDCPlib->hSagelibRpcModuleHandle != NULL)
	{
		BSAGElib_Rai_Module_Uninit(hHDCPlib->hSagelibRpcModuleHandle, &hHDCPlib->uiLastAsyncId);
		hHDCPlib->uiLastAsyncId = 0; /* no response coming */
		BSAGElib_Rpc_RemoveRemote(hHDCPlib->hSagelibRpcModuleHandle);
		BDBG_MSG(("Uninit and remove Hdcp2x SAGE Module: assignedAsyncId [0x%x]", hHDCPlib->uiLastAsyncId));
		hHDCPlib->hSagelibRpcModuleHandle = NULL;
	}

	/* Close HDCP22 platform handle */
	if (hHDCPlib->hSagelibRpcPlatformHandle != NULL)
	{
		BSAGElib_Rai_Platform_Close(hHDCPlib->hSagelibRpcPlatformHandle, &hHDCPlib->uiLastAsyncId);
		hHDCPlib->uiLastAsyncId = 0; /* no response coming */
		BSAGElib_Rpc_RemoveRemote(hHDCPlib->hSagelibRpcPlatformHandle);
		BDBG_MSG(("Close and remove Hdcp2x SAGE platform: assignedAsyncId [0x%x]", hHDCPlib->uiLastAsyncId));
		hHDCPlib->hSagelibRpcPlatformHandle = NULL;
	}

	/* Reinitialize SAGE rpc handles */
	/* Reset initial state */
	hHDCPlib->currentHdcp2xState = BHDCPlib_Hdcp2xState_eSagePlatformOpen;

	/* open platform */
	rc = BSAGElib_Rai_Platform_Open(hHDCPlib->stDependencies.hSagelibClientHandle, BSAGE_PLATFORM_ID_HDCP22,
					hHDCPlib->sageContainer, &hHDCPlib->hSagelibRpcPlatformHandle, /*out */
					(void *)hHDCPlib, &hHDCPlib->uiLastAsyncId /*out */);
	BDBG_MSG(("Open Hdcp2x SAGE platform: platformHandle [0x%x] assignedAsyncId [0x%x]",
		hHDCPlib->hSagelibRpcPlatformHandle, hHDCPlib->uiLastAsyncId));
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error opening SAGE HDCP22 Platform - hdcplib(0x%x) error[%x] '%s'",
			hHDCPlib, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
		rc = BERR_TRACE(rc);
		goto done;
	}

	/* wait for sage response before proceeding */
	rc = BHDCPlib_P_Hdcp2x_WaitForSage(hHDCPlib);
	if (rc != BERR_SUCCESS)
	{
		rc = BERR_TRACE(rc);
		goto done;
	}

done:

	if (rc != BERR_SUCCESS)
	{
		/* Close HDCP22 platform handle */
		if (hHDCPlib->hSagelibRpcPlatformHandle != NULL)
		{
			BSAGElib_Rai_Platform_Close(hHDCPlib->hSagelibRpcPlatformHandle, &hHDCPlib->uiLastAsyncId);
			BHDCPlib_P_Hdcp2x_WaitForSage(hHDCPlib);
			BSAGElib_Rpc_RemoveRemote(hHDCPlib->hSagelibRpcPlatformHandle);
			BDBG_MSG(("Close & remove Hdcp2x SAGE platform: assignedAsyncId [0x%x]", hHDCPlib->uiLastAsyncId));
			hHDCPlib->hSagelibRpcPlatformHandle = NULL;
		}
	}

	BDBG_LEAVE(BHDCPlib_Hdcp2x_ProcessWatchDog);
	return rc;
}


BERR_Code BHDCPlib_P_Hdcp2x_StartAuthentication(const BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_P_Hdcp2x_StartAuthentication);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);


	/* Make sure system is ready for the request */
	switch (hHDCPlib->currentHdcp2xState)
	{
	case BHDCPlib_Hdcp2xState_eAuthenticated:
	case BHDCPlib_Hdcp2xState_eUnauthenticated:
	case BHDCPlib_Hdcp2xState_eSessionKeyLoaded:
		break;

	case BHDCPlib_Hdcp2xState_eAuthenticating:
#if 0
		BDBG_ERR(("Busy authenticating from previous request"));
		goto done;
#endif
		break;


	case BHDCPlib_Hdcp2xState_eSagePlatformOpen:
	case BHDCPlib_Hdcp2xState_eSagePlatformInit:
	case BHDCPlib_Hdcp2xState_eSageModuleInit:
		BDBG_MSG(("System not ready. HDCP2.x authentication will start once system initialized -- current state '%s'",
			BHDCPlib_Hdcp2x_StateToStr(hHDCPlib->currentHdcp2xState)));

		/* Record this request to start authentication later */
		hHDCPlib->bHdcp2xAuthenticationRequested = true;

		goto done;
		break;

	case BHDCPlib_Hdcp2xState_eSystemCannotInitialize:
		BDBG_ERR(("HDCP2.x system cannot be used due to error during initialization"));
		rc = BERR_OS_ERROR;
		BERR_TRACE(rc);
		goto done;
		break;

	default:
		BDBG_ERR(("Invalid Hdcp2.x state - %d", hHDCPlib->currentHdcp2xState));
		rc = BERR_INVALID_PARAMETER;
		BERR_TRACE(rc);
		goto done;
		break;
	}

	/* Reset HDCP link status */
	hHDCPlib->hdcp2xLinkAuthenticated = false;
	BKNI_Memset(&hHDCPlib->stReceiverIdListData,0, sizeof(BHDCPlib_ReceiverIdListData));

	/* Configure/Enable AutoI2C settings for Hdcp2.2*/
	BHDM_AUTO_I2C_EnableReadChannel(hHDCPlib->stDependencies.hHdm,
							BHDM_AUTO_I2C_P_CHANNEL_ePollHdcp22RxStatus, 1) ;

	rc = BHDCPlib_P_Hdcp2x_ProcessRequest(hHDCPlib,
					BHDCPlib_P_Hdcp2xRequest_eHost_StartAuthentication);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error [0x%x] starting Hdcp2.2 authentication", rc ));
		rc = BERR_TRACE(rc);

		/* Disable AutoI2C settings for Hdcp2.x*/
		BHDM_AUTO_I2C_EnableReadChannel(hHDCPlib->stDependencies.hHdm,
								BHDM_AUTO_I2C_P_CHANNEL_ePollHdcp22RxStatus, 0) ;
		goto done;
	}

	/* update state */
	hHDCPlib->currentHdcp2xState = BHDCPlib_Hdcp2xState_eAuthenticating;

done:

	BDBG_LEAVE(BHDCPlib_P_Hdcp2x_StartAuthentication);
	return rc;
}



BERR_Code BHDCPlib_P_Hdcp2x_StopAuthentication(const BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_P_Hdcp2x_StopAuthentication);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	/* Make sure system is ready for the request */
	switch (hHDCPlib->currentHdcp2xState)
	{
	case BHDCPlib_Hdcp2xState_eAuthenticated:
	case BHDCPlib_Hdcp2xState_eUnauthenticated:
	case BHDCPlib_Hdcp2xState_eAuthenticating:
	case BHDCPlib_Hdcp2xState_eSessionKeyLoaded:
		break;

	case BHDCPlib_Hdcp2xState_eSagePlatformOpen:
	case BHDCPlib_Hdcp2xState_eSagePlatformInit:
	case BHDCPlib_Hdcp2xState_eSageModuleInit:
		BDBG_MSG(("System not ready to handle StopAuthentication request -- current state '%s'",
			BHDCPlib_Hdcp2x_StateToStr(hHDCPlib->currentHdcp2xState)));

		/* Record this request for later use */
		hHDCPlib->bHdcp2xAuthenticationRequested = false;

		goto done;
		break;

	case BHDCPlib_Hdcp2xState_eSystemCannotInitialize:
		BDBG_ERR(("HDCP2.x system cannot be used due to error during initialization"));
		rc = BERR_OS_ERROR;
		BERR_TRACE(rc);
		goto done;
		break;

	default:
		BDBG_ERR(("Invalid Hdcp2.x state - %d", hHDCPlib->currentHdcp2xState));
		rc = BERR_INVALID_PARAMETER;
		BERR_TRACE(rc);
		goto done;
		break;
	}


	/* Disable AutoI2C settings for Hdcp2.x*/
	BHDM_AUTO_I2C_EnableReadChannel(hHDCPlib->stDependencies.hHdm,
							BHDM_AUTO_I2C_P_CHANNEL_ePollHdcp22RxStatus, 0) ;

	/* Disable HDCP2.x encryption bit */
	rc = BHDM_HDCP_EnableHdcp2xEncryption(hHDCPlib->stDependencies.hHdm, false);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error [0x%x] disabling Hdcp2.2 encryption", rc));
		rc = BERR_TRACE(rc);
		goto done;
	}

	/* Send request to stop authentication */
	rc = BHDCPlib_P_Hdcp2x_ProcessRequest(hHDCPlib,
					BHDCPlib_P_Hdcp2xRequest_eHost_StopAuthentication);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error stopping Hdcp2.2 authentication"));
		rc = BERR_TRACE(rc);
		goto done;
	}

done:

	BDBG_LEAVE(BHDCPlib_P_Hdcp2x_StopAuthentication);
	return rc;
}


static BERR_Code BHDCPlib_P_Hdcp2x_ProcessRequest(
	const BHDCPlib_Handle hHDCPlib,
	BHDCPlib_P_Hdcp2xRequest eHdcp2xRequest
)
{
	BERR_Code rc = BERR_SUCCESS;
	BHDCPlib_Hdcp2xState nextState = hHDCPlib->currentHdcp2xState;
	bool waitForSageResponse = false;

	BDBG_ENTER(BHDCPlib_P_Hdcp2x_ProcessRequest);


	switch (eHdcp2xRequest)
	{
	case BHDCPlib_P_Hdcp2xRequest_eHost_StartAuthentication:
		rc = BHDCPlib_P_Hdcp2x_SendSageCommand(hHDCPlib,
					Hdcp22_CommandId_Tx_eStartAuthentication);
		if (rc != BERR_SUCCESS) {
			rc = BERR_TRACE(rc);
			goto done;
		}

		/* update state */
		nextState = BHDCPlib_Hdcp2xState_eUnauthenticated;
		waitForSageResponse = true;
		break;


	case BHDCPlib_P_Hdcp2xRequest_eHost_StopAuthentication:
		rc = BHDCPlib_P_Hdcp2x_SendSageCommand(hHDCPlib,
						Hdcp22_CommandId_Tx_eStopAuthentication);
		if (rc != BERR_SUCCESS) {
			rc = BERR_TRACE(rc);
			goto done;
		}

		/* No state change until confirmed from Sage via Indication callback */
		waitForSageResponse = true;
		break;


	case BHDCPlib_P_Hdcp2xRequest_eHost_GetReceiverIdList:
		/* retrieve receiverIdList contain information of downstream device in connection topology */
		rc = BHDCPlib_P_Hdcp2x_SendSageCommand(hHDCPlib,
					Hdcp22_CommandId_Tx_eGetReceiverIdlist);
		if (rc != BERR_SUCCESS) {
			rc = BERR_TRACE(rc);
			goto done;
		}

		/* update state */
		nextState = hHDCPlib->currentHdcp2xState;
		waitForSageResponse = true;
		break;


	/* received indication from Sage */
	case BHDCPlib_P_Hdcp2xRequest_eSage_ProcessIndication:
		switch(hHDCPlib->stIndicationData.indication_id)
		{
		case Hdcp22_IndicationType_eAuthenticationError:
			if (hHDCPlib->stIndicationData.value == Hdcp22_AuthenticationError_eSuccess){
				break;
			}

			BDBG_ERR(("**********************************************"));
			BDBG_ERR(("Hdcp22 SAGE module authentication error"));
			BDBG_ERR(("- Code (0x%04x):  %s", hHDCPlib->stIndicationData.value,
				BHDCPlib_Hdcp2x_P_AuthenticationErrorToStr(hHDCPlib->stIndicationData.value)));
			BDBG_ERR(("**********************************************"));
			break;

		case Hdcp22_IndicationType_eContentStreamTypeUpdate:
			/* saved the received content stream type and provide to application later */
			switch (hHDCPlib->stIndicationData.value)
			{
			case Hdcp22_ContentStreamType_eType0:
				hHDCPlib->eContentStreamTypeReceived = BHDCPlib_Hdcp2xContentStreamType_eType0;
				break;

			case Hdcp22_ContentStreamType_eType1:
				hHDCPlib->eContentStreamTypeReceived = BHDCPlib_Hdcp2xContentStreamType_eType1;
				break;

			default:
				hHDCPlib->eContentStreamTypeReceived = BHDCPlib_Hdcp2xContentStreamType_eMax;
				break;
			}
			break;

		case Hdcp22_IndicationType_eAuthenticationStatus:
			switch (hHDCPlib->stIndicationData.value)
			{
			/* Authentication process failed or link no longer authenticated */
			case Hdcp22_AuthenticationStatus_eUnAuthenticated:
				BDBG_WRN(("Indication Status Received - UNAUTHENTICATED"));
				nextState = BHDCPlib_Hdcp2xState_eUnauthenticated;
				hHDCPlib->hdcp2xLinkAuthenticated = false;

				rc = BHDM_HDCP_UpdateHdcp2xAuthenticationStatus(hHDCPlib->stDependencies.hHdm, false);
				if (rc != BERR_SUCCESS) {
					BDBG_ERR(("Error updating Hdcp2x Authentication status in HDMI HW"));
					BERR_TRACE(rc) ;
				}

				/* Disable AutoI2C settings for Hdcp2.x */
				BHDM_AUTO_I2C_EnableReadChannel(hHDCPlib->stDependencies.hHdm,
										BHDM_AUTO_I2C_P_CHANNEL_ePollHdcp22RxStatus, 0) ;

				/* Set event informing HDCP authentication result */
				BKNI_SetEvent(hHDCPlib->hdcp2xIndicationEvent);
				break;

			/* Session key loaded, now wait for OK_TO_ENC_EN intterupt before enable HDCP 2.2 encryption */
			case Hdcp22_AuthenticationStatus_eSessionKeyLoaded:
				BDBG_WRN(("Indication Status Received - SessionKeyLoaded"));
				/* Kick-start count down timer to enable Hdcp2.2 encryption */
				rc = BHDM_HDCP_UpdateHdcp2xAuthenticationStatus(hHDCPlib->stDependencies.hHdm, true);
				if (rc != BERR_SUCCESS) {
					BDBG_ERR(("Error updating Hdcp2x Authentication status in HDMI HW"));
					BERR_TRACE(rc) ;
				}
				nextState = BHDCPlib_Hdcp2xState_eSessionKeyLoaded;
				break;


			case Hdcp22_AuthenticationStatus_eAutoI2CTimerUnavailable:
				BDBG_WRN(("******* Auto I2C HW Timers are unavailable *****"));
				break;


			/* Start HDCP 2.x authentication process. Make sure to disable SERIAL_KEY_RAM */
			case Hdcp22_AuthenticationStatus_eReadyToDisableKeyRamSerial:
#if BHDCPLIB_HDR_SUPPORT
				rc = BHDR_HDCP_EnableSerialKeyRam(hHDCPlib->stDependencies.hHdr, false);
				if (rc != BERR_SUCCESS) {
					BDBG_ERR(("Error disabling Serial Key Ram in HDMI RX HW"));
					BERR_TRACE(rc) ;
				}
#endif
				break;


			/* Authentication process succeed */
			case Hdcp22_AuthenticationStatus_eAuthenticated:
				BDBG_WRN(("Indication Status Received - AUTHENTICATED"));
				/* Request downstream info in connection topology */
				rc = BHDCPlib_P_Hdcp2x_SendSageCommand(hHDCPlib,
							Hdcp22_CommandId_Tx_eGetReceiverIdlist);
				if (rc != BERR_SUCCESS) {
					rc = BERR_TRACE(rc);
					goto done;
				}

				/* Wait for SAGE command to be completed */
				rc = BHDCPlib_P_Hdcp2x_WaitForSage(hHDCPlib);
				if (rc != BERR_SUCCESS) {
					rc = BERR_TRACE(rc);
					goto done;
				}
				/* intentionally fall through to the next piece of code */

			case Hdcp22_AuthenticationStatus_eRxAuthenticated:
			case Hdcp22_AuthenticationStatus_eRepeaterAuthenticated:
				nextState = BHDCPlib_Hdcp2xState_eAuthenticated;
				hHDCPlib->hdcp2xLinkAuthenticated = true;

				/* fire event informing HDCP authentication result */
				BKNI_SetEvent(hHDCPlib->hdcp2xIndicationEvent);
				break;

			default:
				BDBG_ERR(("Receive Indication callback from SAGE with invalid status"));
				rc = BERR_OS_ERROR;
				BERR_TRACE(rc);
				goto done;
				break;
			}

			break;	/* done process indication type *AuthenticationStatus* */

		default:
			BDBG_ERR(("Invalid Indication callback (Id 0x%x) received from SAGE",
							hHDCPlib->stIndicationData.indication_id));
			rc = BERR_INVALID_PARAMETER;
			BERR_TRACE(rc);
			goto done;
			break;
		}

		break;	/* done process indication */


	/* received response for previous Sage request */
	case BHDCPlib_P_Hdcp2xRequest_eSage_ProcessResponse:
			BDBG_MSG(("Process SAGE Response, current hdcp22 state '%s'",
						BHDCPlib_Hdcp2x_StateToStr(hHDCPlib->currentHdcp2xState)));
			switch (hHDCPlib->currentHdcp2xState)
			{
			/* Sage confirms platform is opened, now initialize platform */
			case BHDCPlib_Hdcp2xState_eSagePlatformOpen:

				/* If already opened & initialized, initialize the SAGE Hdcp 2.2 Tx/Rx module */
				if (hHDCPlib->sageContainer->basicOut[0] == BSAGElib_State_eInit)
				{
					nextState = BHDCPlib_Hdcp2xState_eSageModuleInit;
					BDBG_MSG(("Platform already initialized. Next step: Module Init >>"));

					rc = BHDCPlib_P_Hdcp2x_ModuleInit(hHDCPlib);
					if (rc != BERR_SUCCESS)
					{
						rc = BERR_TRACE(rc);
						goto done;
					}
				}
				else {
					nextState = BHDCPlib_Hdcp2xState_eSagePlatformInit;
					rc = BSAGElib_Rai_Platform_Init(hHDCPlib->hSagelibRpcPlatformHandle,
															NULL, &hHDCPlib->uiLastAsyncId);
					BDBG_MSG(("Initialize Hdcp2x SAGE platform: assignedAsyncId [0x%x]", hHDCPlib->uiLastAsyncId));
					if (rc != BERR_SUCCESS)
					{
						BDBG_ERR(("Error initializing SAGE HDCP22 platform - hdcplib(0x%x) error [0x%x] '%s'",
							hHDCPlib, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
						BERR_TRACE(BERR_OS_ERROR);
						goto done;
					}
				}

				waitForSageResponse = true;
				break;


			/* Sage confirms platform is opened/initialized, now initialize module */
			case BHDCPlib_Hdcp2xState_eSagePlatformInit:
				nextState = BHDCPlib_Hdcp2xState_eSageModuleInit;

				rc = BHDCPlib_P_Hdcp2x_ModuleInit(hHDCPlib);
				if (rc != BERR_SUCCESS)
				{
					rc = BERR_TRACE(rc);
					goto done;
				}

				waitForSageResponse = true;
				break;


			/* Sage confirms module is initialized, system now ready */
			case BHDCPlib_Hdcp2xState_eSageModuleInit:
				nextState = BHDCPlib_Hdcp2xState_eUnauthenticated;

				if (hHDCPlib->bHdcp2xAuthenticationRequested) {
					/* update current state before starting authentication */
					hHDCPlib->currentHdcp2xState = nextState;

					/* Start authentication as requested earlier */
					rc = BHDCPlib_P_Hdcp2x_StartAuthentication(hHDCPlib);
					if (rc != BERR_SUCCESS) {
						rc = BERR_TRACE(rc);
					}
					hHDCPlib->bHdcp2xAuthenticationRequested = false;

					/* skip updating current state - already updated */
					goto done;
				}
				break;

			/* Sage confirms last command (StartAuthentication or StopAuthentication) was successful
				Now just wait for Indication Callback from sage with update of the authentication status */
			case BHDCPlib_Hdcp2xState_eUnauthenticated:
			case BHDCPlib_Hdcp2xState_eAuthenticated:
			case BHDCPlib_Hdcp2xState_eAuthenticating:
			case BHDCPlib_Hdcp2xState_eSessionKeyLoaded:
			case BHDCPlib_Hdcp2xState_eSystemCannotInitialize:
				/* No state change */
				break;

			default:
				BDBG_ERR(("Invalid Hdcp2.x state - %d", hHDCPlib->currentHdcp2xState));
				rc = BERR_INVALID_PARAMETER;
				BERR_TRACE(rc);
				goto done;
				break;
			}

		break;

	case BHDCPlib_P_Hdcp2xRequest_eSage_FailInitialization:
		/* Something went very wrong on the SAGE domain. Cannot initialize HDCP2.x SAGE system */
		BDBG_ERR(("Cannot initialize HDCP2.x SAGE system. No HDCP2.x operation can be accomplished"));
		nextState = BHDCPlib_Hdcp2xState_eSystemCannotInitialize;

		/* HDCP2.x authentication was requested. Fire indication event to
			inform application the authentication failed */
		if (hHDCPlib->bHdcp2xAuthenticationRequested)
		{
			/* Disable AutoI2C settings for Hdcp2.x */
			BHDM_AUTO_I2C_EnableReadChannel(hHDCPlib->stDependencies.hHdm,
									BHDM_AUTO_I2C_P_CHANNEL_ePollHdcp22RxStatus, 0) ;

			/* Fire event informing HDCP authentication failed */
			hHDCPlib->hdcp2xLinkAuthenticated = false;
			BKNI_SetEvent(hHDCPlib->hdcp2xIndicationEvent);

			hHDCPlib->bHdcp2xAuthenticationRequested = false;
		}
		break;

	default:
		BDBG_ERR(("Invalid request"));
		rc = BERR_INVALID_PARAMETER;
		BERR_TRACE(rc);
		goto done;
	}


	/* update state */
	hHDCPlib->currentHdcp2xState = nextState;

	/* wait for sage response before proceeding */
	if (waitForSageResponse)
	{
		rc = BHDCPlib_P_Hdcp2x_WaitForSage(hHDCPlib);
		if (rc != BERR_SUCCESS) {
			rc = BERR_TRACE(rc);
			goto done;
		}
	}

done:

	BDBG_LEAVE(BHDCPlib_P_Hdcp2x_ProcessRequest);
	return rc;
}



/***********************************
*** PUBLIC APIs
***********************************/
BERR_Code BHDCPlib_Hdcp2x_GetEventHandle(
   const BHDCPlib_Handle hHDCPlib, 		  /* [in] HDCPlib handle */
   BHDCPLIB_Hdcp2x_EventType eEventType,
   BKNI_EventHandle *pBHDCPlibEvent /* [out] event handle */
)
{
	BERR_Code	   rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_Hdcp2x_GetEventHandle) ;
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB) ;

	switch (eEventType)
	{
	case BHDCPLIB_Hdcp2x_EventIndication:
		*pBHDCPlibEvent = hHDCPlib->hdcp2xIndicationEvent ;
		break ;

	default :
		BDBG_ERR(("%s: Unknown Event Handle: %d", __FUNCTION__, eEventType)) ;
		rc = BERR_INVALID_PARAMETER ;
		goto done ;
	}


done:
	BDBG_LEAVE(BHDCPlib_Hdcp2x_GetEventHandle) ;
	return rc ;
}


void BHDCPlib_Hdcp2x_EnableEncryption(const BHDCPlib_Handle hHDCPlib, const bool enable)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_Hdcp2x_EnableEncryption);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	/* Make sure system is ready for the request */
	switch (hHDCPlib->currentHdcp2xState)
	{
	case BHDCPlib_Hdcp2xState_eAuthenticated:
	case BHDCPlib_Hdcp2xState_eUnauthenticated:
	case BHDCPlib_Hdcp2xState_eSessionKeyLoaded:
	case BHDCPlib_Hdcp2xState_eSagePlatformOpen:
	case BHDCPlib_Hdcp2xState_eSagePlatformInit:
	case BHDCPlib_Hdcp2xState_eSageModuleInit:
	case BHDCPlib_Hdcp2xState_eSystemCannotInitialize:
		break;

	case BHDCPlib_Hdcp2xState_eAuthenticating:
#if 0
		BDBG_ERR(("Still authenticating from previous request - Skip request to %s encryption",
			enable?"enable":"disable"));
		goto done;
#endif
		break;

	default:
		BDBG_ERR(("Invalid Hdcp2.x state - %d", hHDCPlib->currentHdcp2xState));
		rc = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto done;
		break;
	}


	rc = BHDM_HDCP_EnableHdcp2xEncryption(hHDCPlib->stDependencies.hHdm, enable);

done:

	BDBG_LEAVE(BHDCPlib_Hdcp2x_EnableEncryption);
	return;
}


BERR_Code BHDCPlib_Hdcp2x_ReceiveSageResponse(
	const BHDCPlib_Handle hHDCPlib,
	const BHDCPlib_SageResponseData *sageResponseData
)
{
	BERR_Code rc = BERR_SUCCESS;
	BHDCPlib_P_Hdcp2xRequest eHdcp2xRequest;
	bool waitForSageResponse = false;

	BDBG_ENTER(BHDCPlib_Hdcp2x_ReceiveSageResponse);

	/* Sanity check on async_id */
	if (hHDCPlib->uiLastAsyncId != sageResponseData->async_id)
	{
		BDBG_WRN(("Incorrect async_id received. Ignore response from SAGE - expecting [0x%x] - received [0x%x]",
			hHDCPlib->uiLastAsyncId, sageResponseData->async_id));
		rc = BERR_SUCCESS;
		goto done;
	}
	/* no response pending */
	hHDCPlib->uiLastAsyncId = 0;

	/* display response error if any */
	switch (hHDCPlib->currentHdcp2xState)
	{
		case BHDCPlib_Hdcp2xState_eSagePlatformOpen:
		case BHDCPlib_Hdcp2xState_eSagePlatformInit:
		case BHDCPlib_Hdcp2xState_eSystemCannotInitialize:
			break;

		case BHDCPlib_Hdcp2xState_eSageModuleInit:
		case BHDCPlib_Hdcp2xState_eUnauthenticated:
		case BHDCPlib_Hdcp2xState_eAuthenticated:
		case BHDCPlib_Hdcp2xState_eAuthenticating:
		case BHDCPlib_Hdcp2xState_eSessionKeyLoaded:

			if (hHDCPlib->sageContainer->basicOut[0] != HDCP22_ERR_SUCCESS)
			{
				BDBG_ERR(("**********************************************"));
				BDBG_ERR(("SAGE error occurred in hdcp2x module"));
				BDBG_ERR(("Code (0x%08x): \t%s", hHDCPlib->sageContainer->basicOut[0],
								BHDCPlib_Hdcp2x_P_ResponseErrorToStr(hHDCPlib->sageContainer->basicOut[0])));
				BDBG_ERR(("**********************************************"));
			}
			break;
	}

	/* Response with error */
	if ((sageResponseData->error != BERR_SUCCESS)
	&& (sageResponseData->error != BSAGE_ERR_ALREADY_INITIALIZED))
	{
		if (hHDCPlib->uiStartAuthenticationId == sageResponseData->async_id)
		{
			/* Check return code for BinFile error	*/
			BDBG_ERR(("**********************************************"));
			BDBG_ERR(("SAGE error occurred when starting HDCP 2.x authentication process"));
			if (hHDCPlib->sageContainer->basicOut[2] != 0) {
				BDBG_ERR(("Code (0x%08x): \t%s", hHDCPlib->sageContainer->basicOut[2],
							BSAGElib_Tools_ReturnCodeToString(hHDCPlib->sageContainer->basicOut[2])));
			}
			BDBG_ERR(("**********************************************"));

			/* Disable AutoI2C settings for Hdcp2.x */
			BHDM_AUTO_I2C_EnableReadChannel(hHDCPlib->stDependencies.hHdm,
									BHDM_AUTO_I2C_P_CHANNEL_ePollHdcp22RxStatus, 0) ;

			hHDCPlib->hdcp2xLinkAuthenticated = false;
			/* Set event informing HDCP authentication result */
			BKNI_SetEvent(hHDCPlib->hdcp2xIndicationEvent);

			/* Clean up sage container before exiting */
			BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);
			goto done;
		}
		else
		{
			/* Previous initialization request failed. Retry one more time */
			switch (hHDCPlib->currentHdcp2xState)
			{
			case BHDCPlib_Hdcp2xState_eSagePlatformOpen:
				/* already try platform_open again */
				if (hHDCPlib->uiInitRetryCounter) {
					BDBG_ERR(("Error opening HDCP2.x SAGE platform - error [0x%x] '%s'",
							sageResponseData->error, BSAGElib_Tools_ReturnCodeToString(sageResponseData->error)));
					eHdcp2xRequest = BHDCPlib_P_Hdcp2xRequest_eSage_FailInitialization;
					break;
				}

				BDBG_WRN(("Received error opening HDCP2.x SAGE platform - error[0x%x] '%s'. Try again now >> ",
								sageResponseData->error, BSAGElib_Tools_ReturnCodeToString(sageResponseData->error)));
				hHDCPlib->uiInitRetryCounter++;

				/* reconfigure SageContainer for next SAGE communications */
				BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);

				/* attempt to open hdcp2.x sage platform again */
				rc = BSAGElib_Rai_Platform_Open(hHDCPlib->stDependencies.hSagelibClientHandle, BSAGE_PLATFORM_ID_HDCP22,
								hHDCPlib->sageContainer, &hHDCPlib->hSagelibRpcPlatformHandle, /*out */
								(void *)hHDCPlib, &hHDCPlib->uiLastAsyncId /*out */);
				BDBG_MSG(("Another attempt to open HDCP2x SAGE platform: sagePlatformHandle [0x%x], assignedAsyncId [0x%x]",
					hHDCPlib->hSagelibRpcPlatformHandle, hHDCPlib->uiLastAsyncId));

				if (rc != BERR_SUCCESS)
				{
					BDBG_ERR(("Error opening SAGE HDCP22 Platform - hdcplib(0x%x), error [%x] '%s'",
						hHDCPlib, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
					rc = BERR_TRACE(rc);
					goto done;
				}

				waitForSageResponse = true;
				break;


			case BHDCPlib_Hdcp2xState_eSagePlatformInit:
				/* already try platform_init again */
				if (hHDCPlib->uiInitRetryCounter) {
					BDBG_ERR(("Error initializing HDCP2.x SAGE platform - error [0x%x] '%s'",
							sageResponseData->error, BSAGElib_Tools_ReturnCodeToString(sageResponseData->error)));
					eHdcp2xRequest = BHDCPlib_P_Hdcp2xRequest_eSage_FailInitialization;
					break;
				}

				BDBG_WRN(("Received error initializing HDCP2.x SAGE platform - error [0x%x] '%s'. Try again now >> ",
									sageResponseData->error, BSAGElib_Tools_ReturnCodeToString(sageResponseData->error)));
				hHDCPlib->uiInitRetryCounter++;

				/* reconfigure SageContainer for next SAGE communications */
				BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);

				/* attempt to initialize hdcp2.x sage platform again */
				rc = BSAGElib_Rai_Platform_Init(hHDCPlib->hSagelibRpcPlatformHandle, NULL, &hHDCPlib->uiLastAsyncId);
				BDBG_MSG(("Another attempt to nitialize Hdcp2x SAGE platform: assignedAsyncId [0x%x]",
							hHDCPlib->uiLastAsyncId));
				if (rc != BERR_SUCCESS)
				{
					BDBG_ERR(("Error initializing SAGE HDCP22 platform - hdcplib(0x%x) error [0x%x] '%s'",
						hHDCPlib, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
					rc = BERR_TRACE(rc);
					goto done;
				}

				waitForSageResponse = true;
				break;

			/* Sage confirms module is initialized, system now ready */
			case BHDCPlib_Hdcp2xState_eSageModuleInit:
				/* already try module_init again */
				if (hHDCPlib->uiInitRetryCounter) {
					BDBG_ERR(("Error initializing HDCP2.x SAGE module - error [0x%x] '%s'",
							sageResponseData->error, BSAGElib_Tools_ReturnCodeToString(sageResponseData->error)));
					eHdcp2xRequest = BHDCPlib_P_Hdcp2xRequest_eSage_FailInitialization;
					break;
				}

				BDBG_WRN(("Received error initializing HDCP2.x SAGE module - error [0x%x] '%s'. Try again now >> ",
										sageResponseData->error, BSAGElib_Tools_ReturnCodeToString(sageResponseData->error)));
				hHDCPlib->uiInitRetryCounter++;

				rc = BHDCPlib_P_Hdcp2x_ModuleInit(hHDCPlib);
				if (rc != BERR_SUCCESS) {
					rc = BERR_TRACE(rc);
					goto done;
				}

				waitForSageResponse = true;
				break;

			case BHDCPlib_Hdcp2xState_eUnauthenticated:
			case BHDCPlib_Hdcp2xState_eAuthenticated:
			case BHDCPlib_Hdcp2xState_eAuthenticating:
			case BHDCPlib_Hdcp2xState_eSessionKeyLoaded:
			case BHDCPlib_Hdcp2xState_eSystemCannotInitialize:
				BDBG_ERR(("Previous request to SAGE did not finished successfully - error [%x] '%s'",
					sageResponseData->error, BSAGElib_Tools_ReturnCodeToString(sageResponseData->error)));
				rc = BERR_TRACE(BERR_OS_ERROR);
				goto done;	/* no additional actions needed */
				break;

			default:
				BDBG_ERR(("Invalid Hdcp2.x state - %d", hHDCPlib->currentHdcp2xState));
				rc = BERR_TRACE(BERR_INVALID_PARAMETER);
				goto done;	/* no additional actions needed */
				break;
			}

			/* wait for sage response before proceeding */
			if (waitForSageResponse)
			{
				rc = BHDCPlib_P_Hdcp2x_WaitForSage(hHDCPlib);
				if (rc != BERR_SUCCESS) {
					rc = BERR_TRACE(rc);
				}

				goto done;	/* No further actions */
			}
		}
	}
	else {		/* No error executing previous SAGE command */
		if (hHDCPlib->uiGetReceiverIdListId == sageResponseData->async_id)
		{
			/* Got the receiverId List, now save it for later */
			hHDCPlib->stReceiverIdListData.depth = (hHDCPlib->sageContainer->basicOut[1] & 0xE0) >> 5;
			hHDCPlib->stReceiverIdListData.deviceCount = (hHDCPlib->sageContainer->basicOut[1] & 0x1F);
			hHDCPlib->stReceiverIdListData.maxDevsExceeded = (hHDCPlib->sageContainer->basicOut[2] & 0xF0) >> 4;
			hHDCPlib->stReceiverIdListData.maxCascadeExceeded = (hHDCPlib->sageContainer->basicOut[2] & 0x0F);
			hHDCPlib->stReceiverIdListData.hdcp2LegacyDeviceDownstream = (hHDCPlib->sageContainer->basicOut[3] & 0xF0) >> 4;
			hHDCPlib->stReceiverIdListData.hdcp1DeviceDownstream = (hHDCPlib->sageContainer->basicOut[3] & 0x0F);

			/* Copy the list */
			BKNI_Memcpy(hHDCPlib->stReceiverIdListData.rxIdList, hHDCPlib->sageContainer->blocks[0].data.ptr,
							hHDCPlib->stReceiverIdListData.deviceCount*BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);

			/* Clean up sage container before exiting */
			BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);
			goto done;
		}
		else {
			eHdcp2xRequest = BHDCPlib_P_Hdcp2xRequest_eSage_ProcessResponse;

			/* reset initRetryCounter*/
			hHDCPlib->uiInitRetryCounter = 0;
		}
	}

	/* There's a request to be processed  - Clean SAGE container before proceeding*/
	BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);

	rc = BHDCPlib_P_Hdcp2x_ProcessRequest(hHDCPlib, eHdcp2xRequest);

done:

	BDBG_LEAVE(BHDCPlib_Hdcp2x_ReceiveSageResponse);
	return rc;
}


BERR_Code BHDCPlib_Hdcp2x_ReceiveSageIndication(
	const BHDCPlib_Handle hHDCPlib,
	const BHDCPlib_SageIndicationData *sageIndicationData
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_Hdcp2x_ReceiveSageIndication);

	BKNI_Memcpy(&hHDCPlib->stIndicationData, sageIndicationData, sizeof(BHDCPlib_SageIndicationData));

	rc = BHDCPlib_P_Hdcp2x_ProcessRequest(hHDCPlib,
							BHDCPlib_P_Hdcp2xRequest_eSage_ProcessIndication);

	BDBG_LEAVE(BHDCPlib_Hdcp2x_ReceiveSageIndication);
	return rc;

}


BERR_Code BHDCPlib_Hdcp2x_GetAuthenticationStatus(
	BHDCPlib_Handle hHDCPlib,
	BHDCPlib_Hdcp2x_AuthenticationStatus *pAuthenticationStatus
)
{
	BDBG_ENTER(BHDCPlib_Hdcp2x_GetAuthenticationStatus);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	pAuthenticationStatus->linkAuthenticated = hHDCPlib->hdcp2xLinkAuthenticated;
	pAuthenticationStatus->hdcp1DeviceDownstream = (bool) hHDCPlib->stReceiverIdListData.hdcp1DeviceDownstream;
	pAuthenticationStatus->eContentStreamTypeFromUpstream = hHDCPlib->eContentStreamTypeReceived;

	BDBG_LEAVE(BHDCPlib_Hdcp2x_GetAuthenticationStatus);
	return BERR_SUCCESS;
}


BERR_Code BHDCPlib_Hdcp2x_SetBinKeys(
	const BHDCPlib_Handle hHDCPlib,
	const uint8_t *pBuffer,
	const uint32_t uiSize
)
{
	BERR_Code rc = BERR_SUCCESS;
	uint8_t *binFileBuffer = NULL;

	BDBG_ENTER(BHDCPlib_Hdcp2x_SetBinKeys);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	if (hHDCPlib->stBinKey.pBuffer != NULL)
	{
		BSAGElib_Rai_Memory_Free(hHDCPlib->stDependencies.hSagelibClientHandle, hHDCPlib->stBinKey.pBuffer);
		hHDCPlib->stBinKey.pBuffer = NULL;
		hHDCPlib->stBinKey.uiSize = 0;
	}

	if (uiSize == 0)
	{
		BDBG_WRN(("Invalid size"));
		BERR_TRACE(BERR_INVALID_PARAMETER);
		goto done;
	}

	if (pBuffer == NULL)
	{
		BDBG_WRN(("Invalid buffer"));
		BERR_TRACE(BERR_INVALID_PARAMETER);
		goto done;
	}

	/* Allocate memory to copy the bin file */
	binFileBuffer = BSAGElib_Rai_Memory_Allocate(hHDCPlib->stDependencies.hSagelibClientHandle,
						uiSize, BSAGElib_MemoryType_Global);
	if (binFileBuffer == NULL)
	{
		BDBG_ERR(("Error allocating memory for Hdcp2.x encrypted keys"));
		rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto done;
	}

	BKNI_Memcpy(binFileBuffer, pBuffer, uiSize);

	/* Save memory location */
	hHDCPlib->stBinKey.pBuffer = binFileBuffer;
	hHDCPlib->stBinKey.uiSize = uiSize;

	/* If this is for Rx, send SAGE command now */
	if (hHDCPlib->stDependencies.eCoreType == BHDCPlib_CoreType_eRx)
	{
		/* Make sure system is ready for the request */
		switch (hHDCPlib->currentHdcp2xState)
		{
		case BHDCPlib_Hdcp2xState_eAuthenticated:
		case BHDCPlib_Hdcp2xState_eUnauthenticated:
		case BHDCPlib_Hdcp2xState_eAuthenticating:
		case BHDCPlib_Hdcp2xState_eSessionKeyLoaded:
			break;

		case BHDCPlib_Hdcp2xState_eSagePlatformOpen:
		case BHDCPlib_Hdcp2xState_eSagePlatformInit:
		case BHDCPlib_Hdcp2xState_eSageModuleInit:
			BDBG_ERR(("System not ready for this SAGE request"));
			rc = BERR_OS_ERROR;
			BERR_TRACE(rc);
			goto done;

		case BHDCPlib_Hdcp2xState_eSystemCannotInitialize:
			BDBG_ERR(("HDCP2.x system cannot be used due to error during initialization"));
			rc = BERR_OS_ERROR;
			BERR_TRACE(rc);
			goto done;
			break;

		default:
			BDBG_ERR(("Invalid Hdcp2.x state - %d", hHDCPlib->currentHdcp2xState));
			rc = BERR_INVALID_PARAMETER;
			BERR_TRACE(rc);
			goto done;
			break;
		}

		hHDCPlib->sageContainer->blocks[0].data.ptr = hHDCPlib->stBinKey.pBuffer;
		hHDCPlib->sageContainer->blocks[0].len = hHDCPlib->stBinKey.uiSize;

		/* Send command */
		rc = BSAGElib_Rai_Module_ProcessCommand(hHDCPlib->hSagelibRpcModuleHandle,
				Hdcp22_CommandId_Rx_eSetBinFile, hHDCPlib->sageContainer, &hHDCPlib->uiLastAsyncId);

		BDBG_MSG(("Sending command to SAGE: sageModuleHandle [0x%x], commandId [%d], assignedAsyncId [0x%x]",
			hHDCPlib->hSagelibRpcModuleHandle, Hdcp22_CommandId_Rx_eSetBinFile, hHDCPlib->uiLastAsyncId));

		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Error sending command to SAGE Hdcp22 Rx Module - hdcplib(0x%x) error [0x%x] '%s'",
				hHDCPlib, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
			BERR_TRACE(BERR_OS_ERROR);
			goto done;
		}

		/* Wait for SAGE response */
		rc = BHDCPlib_P_Hdcp2x_WaitForSage(hHDCPlib);
		if (rc != BERR_SUCCESS)
		{
			rc = BERR_TRACE(rc);
			goto done;
		}
	}

done:
	BDBG_LEAVE(BHDCPlib_Hdcp2x_SetBinKeys);
	return rc;
}


BERR_Code BHDCPlib_Hdcp2x_Tx_GetReceiverIdList(
	const BHDCPlib_Handle hHDCPlib,
	BHDCPlib_ReceiverIdListData *stReceiverIdListData
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_Hdcp2x_Tx_GetReceiverIdList);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	if (hHDCPlib->stDependencies.eCoreType == BHDCPlib_CoreType_eRx) {
		BDBG_ERR(("%s not applicable for HDCP 2.2 Receiver/Repeater", __FUNCTION__));
		rc = BERR_INVALID_PARAMETER;
		BERR_TRACE(rc);
		BKNI_Memset(stReceiverIdListData,0, sizeof(BHDCPlib_ReceiverIdListData));
		goto done;
	}

	/* Make sure system is ready for the request */
	switch (hHDCPlib->currentHdcp2xState)
	{
	case BHDCPlib_Hdcp2xState_eAuthenticated:
	case BHDCPlib_Hdcp2xState_eUnauthenticated:
	case BHDCPlib_Hdcp2xState_eAuthenticating:
	case BHDCPlib_Hdcp2xState_eSessionKeyLoaded:
		break;

	case BHDCPlib_Hdcp2xState_eSagePlatformOpen:
	case BHDCPlib_Hdcp2xState_eSagePlatformInit:
	case BHDCPlib_Hdcp2xState_eSageModuleInit:
		BDBG_ERR(("System not ready for this SAGE request"));
		rc = BERR_OS_ERROR;
		BERR_TRACE(rc);
		goto done;

	case BHDCPlib_Hdcp2xState_eSystemCannotInitialize:
		BDBG_ERR(("HDCP2.x system cannot be used due to error during initialization"));
		rc = BERR_OS_ERROR;
		BERR_TRACE(rc);
		goto done;
		break;

	default:
		BDBG_ERR(("Invalid Hdcp2.x state - %d", hHDCPlib->currentHdcp2xState));
		rc = BERR_INVALID_PARAMETER;
		BERR_TRACE(rc);
		goto done;
		break;
	}

	rc = BHDCPlib_P_Hdcp2x_ProcessRequest(hHDCPlib,
					BHDCPlib_P_Hdcp2xRequest_eHost_GetReceiverIdList);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error [0x%x] requesting ReceiverId List", rc));
		rc = BERR_TRACE(rc);
		goto done;
	}

	/* Fill relevent data */
	stReceiverIdListData->deviceCount = hHDCPlib->stReceiverIdListData.deviceCount;
	stReceiverIdListData->depth = hHDCPlib->stReceiverIdListData.depth;
	stReceiverIdListData->maxDevsExceeded = hHDCPlib->stReceiverIdListData.maxDevsExceeded;
	stReceiverIdListData->maxCascadeExceeded = hHDCPlib->stReceiverIdListData.maxCascadeExceeded;
	stReceiverIdListData->hdcp2LegacyDeviceDownstream = hHDCPlib->stReceiverIdListData.hdcp2LegacyDeviceDownstream;
	stReceiverIdListData->hdcp1DeviceDownstream = hHDCPlib->stReceiverIdListData.hdcp1DeviceDownstream;

	/* Copy the list */
	BKNI_Memcpy(stReceiverIdListData->rxIdList, hHDCPlib->stReceiverIdListData.rxIdList,
					stReceiverIdListData->deviceCount*BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);

done:

	BDBG_LEAVE(BHDCPlib_Hdcp2x_Tx_GetReceiverIdList);
	return rc;
}


BERR_Code BHDCPlib_Hdcp2x_Rx_UploadReceiverIdList(
	const BHDCPlib_Handle hHDCPlib,
	const BHDCPlib_ReceiverIdListData *stReceiverIdListData
)
{
	BERR_Code rc = BERR_SUCCESS;
	uint8_t *receiverIdList = NULL;

	BDBG_ENTER(BHDCPlib_Hdcp2x_Rx_UploadReceiverIdList);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);
	BDBG_ASSERT(stReceiverIdListData);

	if (hHDCPlib->stDependencies.eCoreType == BHDCPlib_CoreType_eTx) {
		BDBG_ERR(("%s not applicable for HDCP 2.2 Transmitter", __FUNCTION__));
		rc = BERR_INVALID_PARAMETER;
		BERR_TRACE(rc);
		goto done;
	}

	if (hHDCPlib->currentHdcp2xState != BHDCPlib_Hdcp2xState_eAuthenticated) {
		BDBG_WRN(("HDMI Rx Authenticated Link has not been establish. Drop request"));
		goto done;
	}

	/* reconfigure SageContainer for next SAGE communications */
	BHDCPlib_P_Hdcp2x_CleanSageContainer(hHDCPlib);

	/* load container input params */
	hHDCPlib->sageContainer->basicIn[0] = stReceiverIdListData->deviceCount;
	hHDCPlib->sageContainer->basicIn[1] = stReceiverIdListData->depth;
	hHDCPlib->sageContainer->basicIn[2] = stReceiverIdListData->maxDevsExceeded;
	hHDCPlib->sageContainer->basicIn[3] = stReceiverIdListData->maxCascadeExceeded;
	hHDCPlib->sageContainer->basicIn[4] = stReceiverIdListData->hdcp2LegacyDeviceDownstream;
	hHDCPlib->sageContainer->basicIn[5] = stReceiverIdListData->hdcp1DeviceDownstream;

	/* Allocate memory for receiverIdList */
	receiverIdList = BSAGElib_Rai_Memory_Allocate(hHDCPlib->stDependencies.hSagelibClientHandle,
								BHDCPLIB_HDCP2X_RECEIVERID_LENGTH*stReceiverIdListData->deviceCount, BSAGElib_MemoryType_Global);
	if (receiverIdList == NULL)
	{
		BDBG_ERR(("Error allocating memory for receiverIdList"));
		rc = BERR_OUT_OF_SYSTEM_MEMORY;
		BERR_TRACE(rc);
		goto done;
	}

	/* copy receiverIdList */
	BKNI_Memcpy(receiverIdList, stReceiverIdListData->rxIdList,
						BHDCPLIB_HDCP2X_RECEIVERID_LENGTH*stReceiverIdListData->deviceCount);

	/* load container block */
	hHDCPlib->sageContainer->blocks[0].data.ptr = receiverIdList;
	hHDCPlib->sageContainer->blocks[0].len = BHDCPLIB_HDCP2X_RECEIVERID_LENGTH*stReceiverIdListData->deviceCount;

	/* Send command */
	rc = BSAGElib_Rai_Module_ProcessCommand(hHDCPlib->hSagelibRpcModuleHandle,
			Hdcp22_CommandId_Rx_eSendReceiverIdList, hHDCPlib->sageContainer, &hHDCPlib->uiLastAsyncId);

	BDBG_MSG(("Sending command to SAGE: sageModuleHandle [0x%x], commandId [%d], assignedAsyncId [0x%x]",
		hHDCPlib->hSagelibRpcModuleHandle, Hdcp22_CommandId_Rx_eSendReceiverIdList, hHDCPlib->uiLastAsyncId));

	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error sending command to SAGE Hdcp22 Rx Module - hdcplib(0x%x) error[0x%x] '%s'",
			hHDCPlib, rc, BSAGElib_Tools_ReturnCodeToString(rc)));
		BERR_TRACE(BERR_OS_ERROR);
		goto done;
	}

	/* Wait for SAGE response */
	rc = BHDCPlib_P_Hdcp2x_WaitForSage(hHDCPlib);
	if (rc != BERR_SUCCESS)
	{
		rc = BERR_TRACE(rc);
		goto done;
	}

done:
	BDBG_LEAVE(BHDCPlib_Hdcp2x_Rx_UploadReceiverIdList);
	return rc;

}

#endif
