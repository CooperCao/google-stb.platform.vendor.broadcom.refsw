/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#ifndef BHDCPLIB_H__
#define BHDCPLIB_H__

#include "bstd.h"
#include "berr.h"
#include "bfmt.h"
#include "bavc_hdmi.h"

#include "bhdm.h"
#include "bhdm_edid.h"
#include "bhdm_hdcp.h"
#include "bhsm.h"
#include "btmr.h"

#include "bchp_common.h"
#ifdef BCHP_HDCP2_TX_HAE_INTR2_0_REG_START
#define BHDCPLIB_HAS_HDCP_2X_SUPPORT 1
#endif


#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
#if BHDCPLIB_HDR_SUPPORT
#include "bhdr.h"
#endif
#include "bsagelib.h"
#include "bsagelib_client.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
Summary:
HDCPlib Context Handle
*******************************************************************************/
typedef struct BHDCPlib_P_Handle *BHDCPlib_Handle ;


/******************************************************************************
Summary:
    Encrypted HDCP key settings.

*******************************************************************************/
typedef struct BHDCPlib_EncryptedHdcpKeyStruct
{
    uint8_t  Alg;
    uint8_t  cusKeyVarL;
    uint8_t  cusKeyVarH;
    uint8_t  cusKeySel;
    uint32_t CaDataLo;
    uint32_t CaDataHi;
    uint32_t TCaDataLo;
    uint32_t TCaDataHi;
    uint32_t HdcpKeyLo;
    uint32_t HdcpKeyHi;
} BHDCPlib_EncryptedHdcpKeyStruct;


/******************************************************************************
Summary:
    HDCP information from attached receiver

Description:
    Receiver's HDCP related information is retrieved and save for additional usages
    o RxBksv  -  Receiver BKSV
    o uiIsAuthenticated  -  authentication result
    o uiRxBCaps  -  Rx Capabilities
    o bIsHdcpRepeater  -  Receiver Repeater bit

See Also:
    o BHDCPlib_GetReceiverInfo

*******************************************************************************/
typedef struct BHDCPlib_RxInfo
{
    BAVC_HDMI_HDCP_KSV RxBksv;          /* Receiver Bksv Value */
    uint8_t uiIsAuthenticated;          /* flag indicate the authentication result */
    uint8_t uiRxBCaps;                  /* Rx Capabilities  */
    bool bIsHdcpRepeater;               /* flag indicate whether Rx is an HDCP Repeater */
} BHDCPlib_RxInfo;



/******************************************************************************
Summary:
    Transmitter's HDCP key set

Description:
    Transmitter's HDCP keys for use in HDCP authentication process
    o TxAksv  -  Transmitter AKSV
    o TxKeyStructure  - HDCP keys

See Also:
    o BHDCPlib_GetConfiguration
    o BHDCPlib_SetConfiguration

*******************************************************************************/
typedef struct BHDCPlib_EncryptedTxKeySet
{
    /* Transmitter Information */
    BAVC_HDMI_HDCP_KSV TxAksv;                              /* Transmitter Aksv value */
    BHDCPlib_EncryptedHdcpKeyStruct
            TxKeyStructure[BAVC_HDMI_HDCP_N_PRIVATE_KEYS];  /* HDCP private keys */

} BHDCPlib_EncryptedTxKeySet;


/******************************************************************************
Summary:
    List of HDCP Revoked KSVs

Description:
    The latest list of HDCP Revoked KSV to check the attach receiver(s) KSV
    o Ksvs  -  List of KSV
    o uiNumRevokedKsvs  -  number of KSV in the revoked KSV list

See Also:
    o BHDCPlib_SetRevokedKSVs

*******************************************************************************/
typedef struct BHDCPlib_RevokedKsvList
{
    BAVC_HDMI_HDCP_KSV *Ksvs;                   /* pointer to Revoked KSV List */
    uint16_t uiNumRevokedKsvs;                  /* number of KSVs in Revoked Ksv List */
} BHDCPlib_RevokedKsvList ;


#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
/******************************************************************************
Summary:
    Enumerated Type indicating the Hdcp2.x Content Stream Type

Description:
    Type 0 Content Stream - Content might be transmitted by HDCP Repeaters to all HDCP Devices
    Type 1 Content Stream - Content must not be transmitted by HDCP Repeater to HDCP 1.x Devices.

See Also:
    o BHDCPlib_GetDefaultConfiguration
    o BHDCPlib_GetConfiguration
    o BHDCPlib_SetConfiguration

*******************************************************************************/
typedef enum BHDCPlib_Hdcp2xContentStreamType
{
    BHDCPlib_Hdcp2xContentStreamType_eType0 = 0,    /* Content may be transmitted to all HDCP downstream devices */
    BHDCPlib_Hdcp2xContentStreamType_eType1,        /* Content must not be transmitted to HDCP 1.x downstream devices */
    BHDCPlib_Hdcp2xContentStreamType_eMax

} BHDCPlib_Hdcp2xContentStreamType;
#endif


/******************************************************************************
Summary:
    HDCP configuration for HDCPlib

Description:
    Contains all HDCP configurations/information of both transmitter and receivers
    o eAnSelection  -  An Selection
    o TxKeySet  - Transmitter HDCP key set
    o RxInfo  -  Receiver HDCP information
    o msWaitForR0  -  Wait time before reading R0' (>= 100ms)
    o msIntervalKsvFifoReadyCheck  -  Interval to check for KSV FIFO ready bit
    o uiKsvFifoReadyChecks  -  Number of intervals, start value: 1 (count to 50)

See Also:
    o BHDCPlib_GetDefaultConfiguration
    o BHDCPlib_GetConfiguration
    o BHDCPlib_SetConfiguration

*******************************************************************************/
typedef struct BHDCPlib_Configuration
{
    BHDM_HDCP_AnSelect eAnSelection;
    BHDCPlib_EncryptedTxKeySet TxKeySet;
    BHDCPlib_RxInfo RxInfo;
    uint16_t msWaitForValidVideo;               /* Wait time for valid video before starting authentication */
    uint8_t msWaitForRxR0Margin;                /* Wait time to read R0' in addition to 100ms required by the spec */
    uint8_t msIntervalKsvFifoReadyCheck;        /* Interval to check for KSV FIFO ready bit. Default at 100ms */
    uint16_t msWaitForKsvFifoMargin;        /* Additional wait time for KSV FIFO ready;
                                                        needed for some repeater test equipment */
    uint8_t uiMaxDeviceCount;                   /* Max downstream devices support */
    uint8_t uiMaxDepth;                         /* Max depth support */

#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
    BHDCPlib_Hdcp2xContentStreamType eHdcp2xContentStreamControl;   /* Hdcp2x Content Stream Type */
#endif
} BHDCPlib_Configuration;


/******************************************************************************
Summary:
    Enumerated Type indicating the current HDCP authentication state


See Also:
    o BHDCPlib_GetAuthenticationState

*******************************************************************************/
typedef enum BHDCPlib_State
{
    BHDCPlib_State_eUnPowered,
    BHDCPlib_State_eUnauthenticated,
    BHDCPlib_State_eWaitForValidVideo,
    BHDCPlib_State_eInitializeAuthentication,
    BHDCPlib_State_eWaitForReceiverAuthentication,
    BHDCPlib_State_eReceiverR0Ready,
    BHDCPlib_State_eR0LinkFailure,
    BHDCPlib_State_eReceiverAuthenticated,  /* Part 1 Completed; R0 Match */
    BHDCPlib_State_eWaitForRepeaterReady,
    BHDCPlib_State_eCheckForRepeaterReady,
    BHDCPlib_State_eRepeaterReady,
    BHDCPlib_State_eLinkAuthenticated,      /* Part 2 Completed; Include down stream devices */
    BHDCPlib_State_eEncryptionEnabled,      /* Part 3 Ri Link Integrity Checks Match */
    BHDCPlib_State_eRepeaterAuthenticationFailure,
    BHDCPlib_State_eRiLinkIntegrityFailure,
    BHDCPlib_State_ePjLinkIntegrityFailure,
    BHDCPlib_State_eMax
} BHDCPlib_State;


/******************************************************************************
Summary:
    Enumerated Type indicating the latest HDCP errors

See Also:
    o BHDCPlib_State
    o BHDCPlib_GetHdcpStatus

*******************************************************************************/
typedef enum BHDCPlib_HdcpError
{
    BHDCPlib_HdcpError_eSuccess,
    BHDCPlib_HdcpError_eRxBksvError,
    BHDCPlib_HdcpError_eRxBksvRevoked,
    BHDCPlib_HdcpError_eRxBksvI2cReadError,
    BHDCPlib_HdcpError_eTxAksvError,
    BHDCPlib_HdcpError_eTxAksvI2cWriteError,

    BHDCPlib_HdcpError_eReceiverAuthenticationError,
    BHDCPlib_HdcpError_eRepeaterAuthenticationError,
    BHDCPlib_HdcpError_eRxDevicesExceeded,
    BHDCPlib_HdcpError_eRepeaterDepthExceeded,
    BHDCPlib_HdcpError_eRepeaterFifoNotReady,
    BHDCPlib_HdcpError_eRepeaterDeviceCount0,

    BHDCPlib_HdcpError_eRepeaterLinkFailure,        /* Repeater Error */
    BHDCPlib_HdcpError_eLinkRiFailure,
    BHDCPlib_HdcpError_eLinkPjFailure,

    BHDCPlib_HdcpError_eFifoUnderflow,
    BHDCPlib_HdcpError_eFifoOverflow,
    BHDCPlib_HdcpError_eMultipleAnRequest,

    BHDCPlib_HdcpError_eCount

} BHDCPlib_HdcpError;


/******************************************************************************
Summary:
    Current HDCP status

Description:
    The latest HDCP status to report back to the higher application.
    o eAuthenticationState  -  current authentication state
    o msRecommendedWaitTime  -  the recommended wait time for higher application
                                before continue with the authentication processs

See Also:
    o BHDCPlib_ProcessAuthentication

*******************************************************************************/
typedef struct BHDCPlib_Status
{
    BHDCPlib_State eAuthenticationState;        /* current authentication state */
    BHDCPlib_HdcpError eHdcpError;      /* last Hdcp error */
    uint16_t msRecommendedWaitTime;
} BHDCPlib_Status;


typedef void (*BHDCPlib_CallbackFunc)
    (void                        *pvParam1,
     int                         iParam2,
     void                        *pvData);


/******************************************************************************
Summary:
    HDCPlib core type

Description:
    Each HDCPlib Handle is designated to a specific core (Tx or Rx)
    Rx core type is only applicable for HDCP2.x

See Also:
    o BHDCPlib_Dependencies

*******************************************************************************/
typedef enum BHDCPlib_CoreType
{
    BHDCPlib_CoreType_eTx,
    BHDCPlib_CoreType_eRx
} BHDCPlib_CoreType;



/******************************************************************************
Summary:
    HDCPlib dependencies data

Description:
    Dependencies data require for BHDCPlib_Open()
    o hHdm  -  HDMI Core handle
    o hHsm  -  HSM handle
    o hTme  -  TMR handle
    o Revoke KSV Callback.

See Also:
    o BHDCPlib_Open
    o BHDCPlib_GetDefaultDependencies

*******************************************************************************/
typedef struct BHDCPlib_Dependencies
{
    BHDM_Handle hHdm;               /* HDMI handle */
    BHSM_Handle hHsm;
    BTMR_Handle hTmr ;

    BHDM_HDCP_Version eVersion;

#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
    BHDCPlib_CoreType eCoreType;
#if BHDCPLIB_HDR_SUPPORT
    BHDR_Handle hHdr;               /* HDMI Rx Handle */
#endif
    uint8_t *pHdcpTA; /* HDCP2.2_TA image loaded into memory.                    */
    uint32_t hdcpTASize;
    BSAGElib_ClientHandle hSagelibClientHandle;
    BKNI_EventHandle sageResponseReceivedEvent;
#endif

    void (*lockHsm)(void);
    void (*unlockHsm)(void);
} BHDCPlib_Dependencies;


/****************************************** HDMI ************************************/
/*************
Summary: Open the HDMIlib handle
**************/
BERR_Code BHDCPlib_Open(BHDCPlib_Handle *hHDCPlib, const BHDCPlib_Dependencies *pstDependencies);


/****************
Summary: Close the HDMIlib handle
****************/
BERR_Code BHDCPlib_Close(BHDCPlib_Handle hHDCPlib);


/****************
Summary: Get the default dependencies
****************/
BERR_Code BHDCPlib_GetDefaultDependencies(BHDCPlib_Dependencies *pDefaultDependencies);


/******************
Summary: HDMI Syslib Event Handler
******************/
void BHDCPlib_ProcessEvent(BHDCPlib_Handle hHDCPlib, BHDM_EventType event);


/******************************************* HDCP ***********************************/
/******************
Summary: Initialize the  HDCP authentication state machine.
******************/
BERR_Code BHDCPlib_StartAuthentication(BHDCPlib_Handle hHDCPlib);


/******************************************************************************
Summary:
Enable/Start HDCP authentication state machine.

Description:

Input:
    hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
    stHdcpStatus - HDCP status

Returns:
    BERR_SUCCESS - Successfully closed the HDMI connection.
    BHDM_NO_RX_DEVICE
    BHDM_HDCP_RX_BKSV_ERROR
    BHDM_HDCP_RX_BKSV_REVOKED
    BHDM_HDCP_TX_AKSV_ERROR
    BHDM_HDCP_RX_NO_HDCP_SUPPORT
    BHDM_HDCP_AUTH_ABORTED
    BHDM_HDCP_AUTHENTICATE_ERROR
    BHDM_HDCP_LINK_RI_FAILURE
    BHDM_HDCP_LINK_PJ_FAILURE
    BHDM_HDCP_REPEATER_FIFO_NOT_READY
    BHDM_HDCP_REPEATER_DEVCOUNT_0
    BHDM_HDCP_REPEATER_DEPTH_EXCEEDED
    BHDM_HDCP_RX_DEVICES_EXCEEDED

See Also:

*******************************************************************************/
BERR_Code BHDCPlib_ProcessAuthentication(BHDCPlib_Handle hHDCPlib, BHDCPlib_Status *stHdcpStatus);


/******************
Summary: Disable/Stop HDCP authentication state machine.
******************/
BERR_Code BHDCPlib_DisableAuthentication(BHDCPlib_Handle hHDCPlib);


/******************
Summary: Retrieve HDCP information of the attached receiver (Bcaps, etc.)
******************/
BERR_Code BHDCPlib_GetReceiverInfo(BHDCPlib_Handle hHDCPlib,
    BHDCPlib_RxInfo *stRxHdcpInfo);


/******************
Summary: Get default hdcp configurations (keys, receiver info, etc.)
******************/
BERR_Code BHDCPlib_GetDefaultConfiguration(BHDCPlib_Configuration * stHdcpConfiguration);


/******************
Summary: Get hdcp configurations (keys, receiver info, etc.)
******************/
BERR_Code BHDCPlib_GetConfiguration(BHDCPlib_Handle hHDCPlib, BHDCPlib_Configuration *stHdcpConfiguration);


/******************
Summary: Set hdcp configurations (keys, receiver info, etc.)
******************/
BERR_Code BHDCPlib_SetConfiguration(BHDCPlib_Handle hHDCPlib, BHDCPlib_Configuration *stHdcpConfiguration);


/******************
Summary: Set hdcp revoked ksvs
******************/
BERR_Code BHDCPlib_SetRevokedKSVs(BHDCPlib_Handle hHDCPlib, BHDCPlib_RevokedKsvList *stKsvList);


/******************
Summary: Transmit encrypted video
******************/
BERR_Code BHDCPlib_TransmitEncrypted(BHDCPlib_Handle hHDCPlib);


/******************
Summary: Transmit clear video
******************/
BERR_Code BHDCPlib_TransmitClear(BHDCPlib_Handle hHDCPlib);


/******************
Summary: Retrieve current authentication state
******************/
BERR_Code BHDCPlib_GetAuthenticationState(BHDCPlib_Handle hHDCPlib, BHDCPlib_State *eAuthenticationState);


/******************
Summary: Retrieve current authentication state
******************/
BERR_Code BHDCPlib_GetHdcpStatus(BHDCPlib_Handle hHDCPlib, BHDCPlib_Status *stHdcpStatus);

/******************
Summary:
    Check the current status of the HDCP link

Description:
    Returns true if the link is authenticated and ready for encryption.
******************/
bool BHDCPlib_LinkReadyForEncryption(BHDCPlib_Handle hHDCPlib);


#ifdef __cplusplus
}
#endif

#endif /* BHDCPLIB_H__ */
