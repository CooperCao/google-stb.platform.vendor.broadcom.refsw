/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ***************************************************************************/
#ifndef BHDCPLIB_PRIV_H__
#define BHDCPLIB_PRIV_H__


#include "bhdcplib.h"
#include "bhdcplib_hdcp2x.h"
#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
#include "bhdcplib_hdcp22_priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(HDCPLIB);

#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
#define BHDCPLIB_HDCP22_TXCAPS_VERSION 0x02
#define BHDCPLIB_HDCP22_TXCAPS_TRANSMITTER_CAPABILITY_MASK 0x0000

#define BHDCPLIB_HDCP2X_SAGERESPONSE_TIMEOUT 5000 /* in ms */
#define BHDCPLIB_HDCP2X_HWAUTOI2CTIMER_VERIFICATION_TIMER 100 /* in ms */
#define BHDCPLIB_HDCP2X_AUTHENTICATION_PROCESS_TIMEOUT 5000 /* in ms */
#define BHDCPLIB_HDCP2X_ENABLEENCRYPTION_TIMER 200


typedef enum BHDCPlib_Hdcp2xState
{
	BHDCPlib_Hdcp2xState_eSagePlatformOpen = 100,
	BHDCPlib_Hdcp2xState_eSagePlatformInit,
	BHDCPlib_Hdcp2xState_eSageModuleInit,
	BHDCPlib_Hdcp2xState_eUnauthenticating,
	BHDCPlib_Hdcp2xState_eUnauthenticated,
	BHDCPlib_Hdcp2xState_eAuthenticating,
	BHDCPlib_Hdcp2xState_eSessionKeyLoaded,
	BHDCPlib_Hdcp2xState_eAuthenticated,
	BHDCPlib_Hdcp2xState_eRepeaterAuthenticated,
	BHDCPlib_Hdcp2xState_eSystemCannotInitialize
} BHDCPlib_Hdcp2xState;

typedef enum BHDCPlib_Hdcp2xEncryptionState
{
	BHDCPlib_Hdcp2xEncryptionState_eUnencrypted = 200,
	BHDCPlib_Hdcp2xEncryptionState_eEncrypting,
	BHDCPlib_Hdcp2xEncryptionState_eEncrypted
} BHDCPlib_Hdcp2xEncryptionState;

typedef enum BHDCPlib_P_Hdcp2xRequest
{
	BHDCPlib_P_Hdcp2xRequest_eSage_ProcessResponse,
	BHDCPlib_P_Hdcp2xRequest_eSage_ProcessIndication,
	BHDCPlib_P_Hdcp2xRequest_eSage_FailInitialization,
	BHDCPlib_P_Hdcp2xRequest_eHost_StartAuthentication,
	BHDCPlib_P_Hdcp2xRequest_eHost_StopAuthentication,
	BHDCPlib_P_Hdcp2xRequest_eHost_GetReceiverIdList
} BHDCPlib_P_Hdcp2xRequest;


typedef struct BHDCPlib_Hdcp2xKeyBin
{
	uint8_t *pBuffer; 					/* pointer to bin file */
	uint32_t uiSize; 					/* size of buffer */
} BHDCPlib_Hdcp2xKeyBin;


typedef enum
{
	BHDCPlib_P_Timer_eAutoI2c,  /* AutoI2C HW Stuck  */
	BHDCPlib_P_Timer_eAuthentication,   /* timer for authentication result */
	BHDCPlib_P_Timer_eEncryptionEnable
} BHDCPlib_P_Timer ;
#endif


typedef struct BHDCPlib_P_Handle
{
	BDBG_OBJECT(HDCPLIB)

	BHDCPlib_Dependencies stDependencies;

	/* HDCP */
	BHDCPlib_Event stHdcpLIC_Ri;
	BHDCPlib_Event stHdcpLIC_Pj;
	BHDCPlib_Configuration stHdcpConfiguration ;
	BHDCPlib_RevokedKsvList RevokedKsvList;
	BHDCPlib_Status stHdcpStatus;
	uint8_t uiKsvFifoReadyCount;	/* Number of times KSV FIFO has been checked */

	BTMR_Handle hTmr ;
	BTMR_TimerHandle hTimer;
	BTMR_Settings stTmrSettings;
	bool bR0ReadyTimerExpired;
	bool bRepeaterReadyTimerExpired;

#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
	BHDCPlib_Hdcp2xKeyBin stBinKey;
	bool hdcp2xLinkAuthenticated;
	BKNI_EventHandle hdcp2xIndicationEvent;
	BTMR_TimerHandle hAuthenticationTimer;
	BTMR_TimerHandle hReadyToEnableEncryptionTimer;

	BSAGElib_RpcRemoteHandle hSagelibRpcPlatformHandle;
	BSAGElib_RpcRemoteHandle hSagelibRpcModuleHandle;
	BSAGElib_InOutContainer *sageContainer;
	uint32_t uiLastAsyncId;
	uint32_t uiModuleInitAsyncId;
	uint32_t uiGetReceiverIdListId;
	BHDCPlib_ReceiverIdListData stReceiverIdListData;
	BHDCPlib_Hdcp2xState currentHdcp2xState;
	BHDCPlib_Hdcp2xEncryptionState currentHdcp2xEncryptionState;
	BHDCPlib_HdcpError lastAuthenticationError;
	BHDCPlib_SageIndicationData stIndicationData;
	uint8_t uiInitRetryCounter;
	BHDCPlib_Hdcp2xContentStreamType eContentStreamTypeReceived;
	uint8_t uiSageSessionId;
	uint32_t uiSageVersionId;
#endif
} BHDCPlib_P_Handle;


/******************
Summary: Initiate the authentication process with the attached receiver
******************/
BERR_Code BHDCPlib_InitializeReceiverAuthentication(BHDCPlib_Handle hHDCPlib);


/******************
Summary: Authenticate the receiver
Descripttion: Exchange KSVs, An and verify R0=R0'
******************/
BERR_Code BHDCPlib_AuthenticateReceiver(BHDCPlib_Handle hHDCPlib);


/******************
Summary: Initiate the authentication process with the attached repeater
******************/
BERR_Code BHDCPlib_InitializeRepeaterAuthentication(BHDCPlib_Handle hHDCPlib);


/******************
Summary: Authenticate attached repeater
Descripttion: Assemble a list of all downstream KSVs attached to the HDCP Repeater
******************/
BERR_Code BHDCPlib_AuthenticateRepeater(BHDCPlib_Handle hHDCPlib);

#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
BERR_Code BHDCPlib_P_Hdcp2x_Open(
	BHDCPlib_Handle *hHDCPlib,
	const BHDCPlib_Dependencies *pstDependencies
);

BERR_Code BHDCPlib_P_Hdcp2x_Close(
	BHDCPlib_Handle hHDCPlib
);

BERR_Code BHDCPlib_P_Hdcp2x_StartAuthentication(
	BHDCPlib_Handle hHDCPlib
);

BERR_Code BHDCPlib_P_Hdcp2x_StopAuthentication(
	BHDCPlib_Handle hHDCPlib
);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BHDCPLIB_PRIV_H__ */


