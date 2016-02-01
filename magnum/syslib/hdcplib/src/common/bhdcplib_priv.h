/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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

#define BHDCPLIB_HDCP22_RXCAPS_VERSION 0x02
#define BHDCPLIB_HDCP22_RXCAPS_RECEIVER_CAPABILITY_MASK 0x0000

#define BHDCPLIB_HDCP2X_SAGERESPONSE_TIMEOUT 5000 /* in ms */


typedef enum BHDCPlib_Hdcp2xState
{
	BHDCPlib_Hdcp2xState_eSagePlatformOpen = 100,
	BHDCPlib_Hdcp2xState_eSagePlatformInit,
	BHDCPlib_Hdcp2xState_eSageModuleInit,
	BHDCPlib_Hdcp2xState_eUnauthenticated,
	BHDCPlib_Hdcp2xState_eAuthenticated,
	BHDCPlib_Hdcp2xState_eSystemCannotInitialize
} BHDCPlib_Hdcp2xState;


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

	BSAGElib_RpcRemoteHandle hSagelibRpcPlatformHandle;
	BSAGElib_RpcRemoteHandle hSagelibRpcModuleHandle;
	BSAGElib_InOutContainer *sageContainer;
	uint32_t uiLastAsyncId;
	uint32_t uiStartAuthenticationId;
	uint32_t uiGetReceiverIdListId;
	BHDCPlib_ReceiverIdListData stReceiverIdListData;
	BHDCPlib_Hdcp2xState currentHdcp2xState;
	BHDCPlib_SageIndicationData stIndicationData;
	bool bHdcp2xAuthenticationRequested;
	uint8_t uiInitRetryCounter;
	BHDCPlib_Hdcp2xContentStreamType eContentStreamTypeReceived;
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


