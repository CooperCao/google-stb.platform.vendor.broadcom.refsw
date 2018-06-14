/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef NEXUS_HDMI_OUTPUT_HDCP_H__
#define NEXUS_HDMI_OUTPUT_HDCP_H__

#include "nexus_hdmi_output.h"
#ifdef NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input.h"
#else
#include "nexus_core_compat.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Enumeration for HDCP An Values

Description:
The DVI/HDMI transmitter is capable of generating a pseudo-random number (An)
which is used as a initial seed for the HDCP calculations.  This Enumerated
Type specifies the type of An value which can be generated.
**/
typedef enum NEXUS_HdmiOutputHdcpAnValue
{
    NEXUS_HdmiOutputHdcpAnValue_eTestA1B1, /* generate fixed A1/B1 HDCP Spec An value */
    NEXUS_HdmiOutputHdcpAnValue_eTestA1B2, /* generate fixed A1/B2 HDCP Spec An value */
    NEXUS_HdmiOutputHdcpAnValue_eTestA2B1, /* generate fixed A2/B1 HDCP Spec An value */
    NEXUS_HdmiOutputHdcpAnValue_eTestA2B2, /* generate fixed A2/B2 HDCP Spec An value */
    NEXUS_HdmiOutputHdcpAnValue_eRandom    /* generate random An value */
} NEXUS_HdmiOutputHdcpAnValue;

/**
Summary:
Structure for an HDCP KSV key
**/
#define NEXUS_HDMI_OUTPUT_HDCP_KSV_LENGTH 5
typedef struct NEXUS_HdmiOutputHdcpKsv
{
    uint8_t data[NEXUS_HDMI_OUTPUT_HDCP_KSV_LENGTH];
} NEXUS_HdmiOutputHdcpKsv;

/**
Summary:
Structure for an encrypted HDCP key
**/
#define NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS 40
typedef struct NEXUS_HdmiOutputHdcpKey
{
    uint8_t  alg;
    uint8_t  custKeyVarL;
    uint8_t  custKeyVarH;
    uint8_t  custKeySel;
    uint32_t caDataLo;
    uint32_t caDataHi;
    uint32_t tCaDataLo;
    uint32_t tCaDataHi;
    uint32_t hdcpKeyLo;
    uint32_t hdcpKeyHi;
} NEXUS_HdmiOutputHdcpKey;

/**
Summary:
Enumeration for HDCP authentication selection

Description:
Users requiring a specific authentication level can request a specific version (or best available)
**/
typedef enum NEXUS_HdmiOutputHdcpVersion
{
    NEXUS_HdmiOutputHdcpVersion_eAuto,  /* Highest available on the attached HDMI Receiver, default */
    NEXUS_HdmiOutputHdcpVersion_e2_2,   /* Always use HDCP 2.2 */
    NEXUS_HdmiOutputHdcpVersion_e1_x,   /* Always use HDCP 1.x */
    NEXUS_HdmiOutputHdcpVersion_eMax
} NEXUS_HdmiOutputHdcpVersion;

/**
Summary:
HDCP Settings

Description:
By default, this structure will contain a test key set.  The test key set
will not authenticate with production devices.  For production devices, you
must obtain a production key set that can be loaded via this routine.
Please contact your FAE for more details on this process.
**/
typedef struct NEXUS_HdmiOutputHdcpSettings
{
    NEXUS_HdmiOutputHdcpAnValue anValue;
    NEXUS_HdmiOutputHdcpKsv aksv;
    NEXUS_HdmiOutputHdcpKey encryptedKeySet[NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS];

    bool transmitEncrypted;                     /* If true, transmission will be encrypted once authenticated */
    bool pjCheckEnabled;                        /* If true, the HDCP Pj key will be checked */

    NEXUS_CallbackDesc successCallback;         /* Called when authentication succeeds. It is recommended that NEXUS_HdmiOutputHdcpStatus.hdcpState be tested in the callback
                                                   to prevent race conditions; however the HDCP protocol should ensure that state changes not occur faster than 2 seconds. */
    NEXUS_CallbackDesc failureCallback;         /* Called if authentication fails. It is recommended that NEXUS_HdmiOutputHdcpStatus.hdcpState be tested in the callback
                                                   to prevent race conditions; however the HDCP protocol should ensure that state changes not occur faster than 2 seconds. */
    NEXUS_CallbackDesc stateChangedCallback;    /* Called on all state transitions */

    unsigned waitForValidVideo;                 /* Wait time for valid video before starting authentication. units in milliseconds. */
    unsigned waitForRxR0Margin;                 /* Additional wait time to read receiver R0' (in addition to the required 100ms). units in milliseconds. */
    unsigned waitForKsvFifoMargin;              /* Additional wait time for KSV List ready (in addition to the required 5secs). units in milliseconds. */

    unsigned maxDeviceCountSupported;           /* Max downstream device count supported */
    unsigned maxDepthSupported;                 /* Max depth supported */

    NEXUS_Hdcp2xContentStream hdcp2xContentStreamControl;

    NEXUS_HdmiOutputHdcpVersion hdcp_version;   /* Select hdcp_version mode to use for authentication */
} NEXUS_HdmiOutputHdcpSettings;

/**
Summary:
Get HDCP Settings
**/
void NEXUS_HdmiOutput_GetHdcpSettings(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputHdcpSettings *pSettings /* [out] */
    );

/**
Summary:
Set HDCP Settings
**/
NEXUS_Error NEXUS_HdmiOutput_SetHdcpSettings(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiOutputHdcpSettings *pSettings
    );

/**
Summary:
Establish list of Revoked KSV's

As an exception, this function is callable with a NEXUS_ALIAS_ID NEXUS_HdmiOutputHandle.
**/
NEXUS_Error NEXUS_HdmiOutput_SetHdcpRevokedKsvs(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiOutputHdcpKsv *pRevokedKsvs,    /* attr{nelem=numKsvs} array of revoked ksv's */
    uint16_t numKsvs                                /* Number of ksvs in the array provided */
    );


/**
Summary:
Set Hdcp 2.x encrypted keys read from bin file
**/
NEXUS_Error NEXUS_HdmiOutput_SetHdcp2xBinKeys(
    NEXUS_HdmiOutputHandle handle,
    const uint8_t *pBinFileBuffer,  /* attr{nelem=length} pointer to encrypted key buffer */
    uint32_t length                 /* size of data in pBinFileBuffer in bytes */
);

/**
Summary:
Initiate HDCP authentication

Description:
Calls to NEXUS_HdmiOutput_SetHdcpSettings() and NEXUS_HdmiOutput_SetHdcpRevokedKsvs()
should be made prior to starting authentication.

See Also:
NEXUS_HdmiOutput_SetHdcpSettings
NEXUS_HdmiOutput_SetHdcpRevokedKsvs
**/
NEXUS_Error NEXUS_HdmiOutput_StartHdcpAuthentication(
    NEXUS_HdmiOutputHandle handle
    );

/**
Summary:
Terminate HDCP authentication
**/
NEXUS_Error NEXUS_HdmiOutput_DisableHdcpAuthentication(
    NEXUS_HdmiOutputHandle handle
    );

/**
Summary:
Enable HDCP encryption. For Hdcp 1.x, this API is equivalent to
    NEXUS_HdmiOutput_StartHdcpAuthentication
**/
NEXUS_Error NEXUS_HdmiOutput_EnableHdcpEncryption(
    NEXUS_HdmiOutputHandle handle
    );

/**
Summary:
Disable HDCP encryption. For Hdcp 1.x, this API is equivalent to
    NEXUS_HdmiOutput_DisableHdcpAuthentication
**/
NEXUS_Error NEXUS_HdmiOutput_DisableHdcpEncryption(
    NEXUS_HdmiOutputHandle handle
    );

/**
Summary:
Enumeration for HDCP states
**/
typedef enum NEXUS_HdmiOutputHdcpState
{
    NEXUS_HdmiOutputHdcpState_eUnpowered,
    NEXUS_HdmiOutputHdcpState_eUnauthenticated,
    NEXUS_HdmiOutputHdcpState_eWaitForValidVideo,
    NEXUS_HdmiOutputHdcpState_eInitializedAuthentication,
    NEXUS_HdmiOutputHdcpState_eWaitForReceiverAuthentication,
    NEXUS_HdmiOutputHdcpState_eReceiverR0Ready,
    NEXUS_HdmiOutputHdcpState_eR0LinkFailure,
    NEXUS_HdmiOutputHdcpState_eReceiverAuthenticated,
    NEXUS_HdmiOutputHdcpState_eWaitForRepeaterReady,
    NEXUS_HdmiOutputHdcpState_eCheckForRepeaterReady,
    NEXUS_HdmiOutputHdcpState_eRepeaterReady,
    NEXUS_HdmiOutputHdcpState_eLinkAuthenticated,      /* Includes down stream devices */
    NEXUS_HdmiOutputHdcpState_eEncryptionEnabled,
    NEXUS_HdmiOutputHdcpState_eRepeaterAuthenticationFailure,
    NEXUS_HdmiOutputHdcpState_eRiLinkIntegrityFailure,
    NEXUS_HdmiOutputHdcpState_ePjLinkIntegrityFailure,
    NEXUS_HdmiOutputHdcpState_eMax
} NEXUS_HdmiOutputHdcpState;


/**
Summary:
Enumeration for HDCP errors
**/
typedef enum NEXUS_HdmiOutputHdcpError
{
    NEXUS_HdmiOutputHdcpError_eSuccess,
    NEXUS_HdmiOutputHdcpError_eRxBksvError,
    NEXUS_HdmiOutputHdcpError_eRxBksvRevoked,
    NEXUS_HdmiOutputHdcpError_eRxBksvI2cReadError,
    NEXUS_HdmiOutputHdcpError_eTxAksvError,
    NEXUS_HdmiOutputHdcpError_eTxAksvI2cWriteError,
    NEXUS_HdmiOutputHdcpError_eReceiverAuthenticationError,
    NEXUS_HdmiOutputHdcpError_eRepeaterAuthenticationError,
    NEXUS_HdmiOutputHdcpError_eRxDevicesExceeded,
    NEXUS_HdmiOutputHdcpError_eRepeaterDepthExceeded,
    NEXUS_HdmiOutputHdcpError_eRepeaterFifoNotReady,
    NEXUS_HdmiOutputHdcpError_eRepeaterDeviceCount0,
    NEXUS_HdmiOutputHdcpError_eRepeaterLinkFailure,     /* Repeater Error */
    NEXUS_HdmiOutputHdcpError_eLinkRiFailure,
    NEXUS_HdmiOutputHdcpError_eLinkPjFailure,
    NEXUS_HdmiOutputHdcpError_eFifoUnderflow,
    NEXUS_HdmiOutputHdcpError_eFifoOverflow,
    NEXUS_HdmiOutputHdcpError_eMultipleAnRequest,
    NEXUS_HdmiOutputHdcpError_eMax
} NEXUS_HdmiOutputHdcpError;


/**
Summary:
HDCP Status
**/
typedef struct NEXUS_HdmiOutputHdcpStatus
{
    NEXUS_HdmiOutputHdcpState hdcpState;    /* Current HDCP State */
    NEXUS_HdmiOutputHdcpError hdcpError;    /* Last HDCP error */

    bool linkReadyForEncryption;    /* True when link is ready for encryption or when link is encrypted */
    bool transmittingEncrypted;     /* True indicates Encryption is enabled */

    bool isHdcpRepeater;            /* Receiver is a repeater */
    bool ksvFifoReady;              /* Receiver ksv FIFO is ready */
    bool i2c400Support;             /* Receiver supports 400kHz I2C accesses */
    bool hdcp1_1Features;           /* Receiver supports HDCP 1.1 features {EESS, Advance Cipher, Enhanced Link Verification} */
    bool fastReauthentication;      /* Receiver can receive un-encrypted video during re-authentication */

    bool hdcp2_2Features;           /* Deprecated: true if HDMI is currently authenticated with HDCP 2.2 protocol */
    struct {
        bool hdcp1_xDeviceDownstream;   /* From HDCP2.2 spec: when set to 1, indicates present of an HDCP1.x-compliance Device in the topology */
    } hdcp2_2RxInfo;

    NEXUS_HdmiOutputHdcpKsv bksv;   /* Receiver's Bksv value */

    NEXUS_HdcpVersion rxMaxHdcpVersion; /* Highest HDCP version supported by Rx. NOTE: the highest version enum
                                                           does not have highest integer value. */
    NEXUS_HdcpVersion selectedHdcpVersion;     /* HDCP Version selected by user or eAuto algorithm for authentication.
                                                          Use linkReadyForEncryption to determine if HDCP is authenticated */

} NEXUS_HdmiOutputHdcpStatus;

/**
Summary:
Get HDCP Status
**/
NEXUS_Error NEXUS_HdmiOutput_GetHdcpStatus(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputHdcpStatus *pStatus /* [out] */
    );

/**
Summary:
Get HDCP info for Downstream Devices connected to the hdmi transmitter
This API is only applicable for HDCP 1.x
**/
NEXUS_Error NEXUS_HdmiOutput_HdcpGetDownstreamInfo(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiHdcpDownStreamInfo *pDownstream  /* [out] */
    );

/**
Summary:
Get the HDCP KSVs of the down stream devices connected to the hdmi transmitter
This API only applicable for HDCP 1.x
**/
NEXUS_Error NEXUS_HdmiOutput_HdcpGetDownstreamKsvs(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiHdcpKsv *pKsvs, /* attr{nelem=numDevices;nelem_out=pNumRead} */
    unsigned numDevices,
    unsigned *pNumRead /* [out] */
    );

#define NEXUS_HdmiOuput_HdcpGetDownstreamInfo NEXUS_HdmiOutput_HdcpGetDownstreamInfo
#define NEXUS_HdmiOuput_HdcpGetDownstreamKsvs NEXUS_HdmiOutput_HdcpGetDownstreamKsvs

/**
Summary:
Only applicable for HDMI HDCP 2.x repeaters.
Set/create a link between hdmi input & output in order to load downstream HDCP info
from output->input. This information will then be uploaded to the upstream transmitter
via HDCP 2.2 protocol message
**/
NEXUS_Error NEXUS_HdmiOutput_SetRepeaterInput(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiInputHandle input /* attr{null_allowed=y} */
);


/**
Summary:
Get the HDCP2.x ReceiverIdList  of the down stream devices connected to the hdmi transmitter
This API only applicable for HDCP 2.x
**/
NEXUS_Error NEXUS_HdmiOutput_GetHdcp2xReceiverIdListData(
	NEXUS_HdmiOutputHandle handle,
	NEXUS_Hdcp2xReceiverIdListData *pData
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_HDMI_OUTPUT_HDCP_H__ */

