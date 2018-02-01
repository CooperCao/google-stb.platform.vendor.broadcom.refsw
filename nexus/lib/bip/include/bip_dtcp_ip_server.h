/******************************************************************************
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
 *****************************************************************************/

#ifndef BIP_DTCP_IP_SERVER_H
#define BIP_DTCP_IP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip_dtcp_ip.h"

/** @addtogroup bip_dtcp_ip_server
 *
 * BIP_DtcpIpServer Interface Definition.
 *
 * For Server side, BIP_DtcpIpServer is the main interface providing the
 * DTCP Server implementation.  This class uses Broadcom's DTCP/IP library
 * for the actual DTCP/IP Server side implementation.
 *
 */

/**
Summary:
Datatype to hold a reference to a BIP_DtcpIpServer object.
**/
typedef struct BIP_DtcpIpServer *BIP_DtcpIpServerHandle;        /* Main DtcpIpServer object */

/**
Summary:
Settings for BIP_DtcpIpServerCreate().

See Also:
BIP_DtcpIpServer_Create
BIP_DtcpIpServer_GetDefaultCreateSettings
**/
typedef struct BIP_DtcpIpServerCreateSettings
{
    int unused;          /*!< Unused placeholder. */
} BIP_DtcpIpServerCreateSettings;

/**
Summary:
Get default settings for BIP_DtcpIpServer_Create().

See Also:
BIP_DtcpIpServer_Create
BIP_DtcpIpServerCreateSettings
**/
void BIP_DtcpIpServer_GetDefaultCreateSettings(
    BIP_DtcpIpServerCreateSettings *pSettings
    );

/**
Summary:
Create an DtcpIpServer

Description:
A BIP_DtcpIpServer is responsible for:
-# Listening for incoming AKE connections.
-# Receiving incoming AKE Requests and carrying out the AKE.
-# Sending outgoing HTTP Response Messages.
-# Starting BIP_Streamers to stream content.

Calling Context:
\code
    BIP_DtcpIpServerHandle                 hDtcpIpServer;
    BIP_DtcpIpServerCreateSettings         createSettings;

    BIP_DtcpIpServer_GetDefaultCreateSettings( &createSettings );
    createSettings.<settingName> =  <settingValue>;     //  Set any non-default settings as desired.
    hDtcpIpServer = BIP_DtcpIpServer_Create( &createSettings );
\endcode

Input: &createSettings  Optional address of a BIP_DtcpIpServerCreateSettings structure. Passing NULL will use default settings.

Return:
    non-NULL :        A BIP_DtcpIpServerHandle used for calling subsequent DtcpIpServer related APIs
Return:
    NULL     :        Failure, an DtcpIpServer instance could not be created.

See Also:
BIP_DtcpIpServerCreateSettings
BIP_DtcpIpServer_GetDefaultCreateSettings
BIP_DtcpIpServer_Destroy
**/
BIP_DtcpIpServerHandle BIP_DtcpIpServer_Create(
    const BIP_DtcpIpServerCreateSettings *pSettings
    );

/**
Summary:
Settings for BIP_DtcpIpServer_Start().

See Also:
BIP_DtcpIpServer_Start
BIP_DtcpIpServer_GetDefaultStartSettings
**/
typedef struct BIP_DtcpIpServerStartSettings
{
    BIP_SETTINGS(BIP_DtcpIpServerStartSettings) /* Internal use... for init verification. */

    const char *pAkePort;                       /*!< Required: Port# to listen for AKE Requests. */
                                                /*!< Note: DTCP AKE Server listens to BIP_DtcpIpServerStartSettings.pInterfaceName if it is specified else it listens on all interfaces. */

    bool        pcpUsageRuleEnabled;            /*!< Optional: Enable PCP-UR feature in the DTCP library. */
                                                /*!< true => PCP-UR of 16 bits (containing Media Usage Rule) + Nonce of 48 bits. App will set the specific Usage Rules via the BIP_HttpStreamerSettings.dtcpSettings */
                                                /*!< false => PCP-UR is not used & PCP will contain Nonce of 48 bits. */
                                                /*!< Defaults to false. */

    B_DTCP_KeyFormat_T keyFormat;               /*!< Format of DTCP Certificate & Key. */
                                                /*!< Defaults to B_DTCP_KeyFormat_eCommonDRM: DTLA Certificate, Production key, & Constants are wrapped in the Common DRM bin format. */
                                                /*!< For testing, if CommonDRM keys are not available, it can be set to B_DTCP_KeyFormat_eTest: DTCP/IP library will internally generate test keys. */

    const char *pKeyFileAbsolutePathname;       /*!< Absolute pathname to the key file. */
                                                /*!< Defaults to ?, TODO: currently not used. */
    bool enableHwOffload;                       /*!< Optional: enables offload to h/w like ASP if available on a platform: encryption process will be taken care by ASP. */
} BIP_DtcpIpServerStartSettings;
BIP_SETTINGS_ID_DECLARE(BIP_DtcpIpServerStartSettings);

#define DTCP_IP_SERVER_DEFAULT_AKE_LISTENING_PORT "8000"

/**
Summary:
Get default settings for BIP_DtcpIpServer_Start().

See Also:
BIP_DtcpIpServerStartSettings
BIP_DtcpIpServer_Start
**/
#define BIP_DtcpIpServer_GetDefaultStartSettings(pSettings)                       \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_DtcpIpServerStartSettings)  \
        /* Set non-zero defaults explicitly. */                                   \
        (pSettings)->pAkePort = DTCP_IP_SERVER_DEFAULT_AKE_LISTENING_PORT;        \
        (pSettings)->keyFormat = B_DTCP_KeyFormat_eCommonDRM;                     \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Start a BIP_DtcpIpServer

Description:
BIP will start listening on server socket and accept incoming HTTP requests after this call.

See Also:
BIP_DtcpIpServerStartSettings
BIP_DtcpIpServer_GetDefaultStartSettings
BIP_DtcpIpServer_Stop
**/
BIP_Status BIP_DtcpIpServer_Start(
    BIP_DtcpIpServerHandle hDtcpIpServer,
    BIP_DtcpIpServerStartSettings *pSettings
    );

/**
Summary:
Stop a BIP_DtcpIpServer

Description:
BIP will not accept any new connection requests after this call.

See Also:
BIP_DtcpIpServer_Start
**/
BIP_Status BIP_DtcpIpServer_Stop(
    BIP_DtcpIpServerHandle hDtcpIpServer
    );

typedef struct BIP_DtcpIpServerStatus
{
    void *          pDtcpIpLibCtx;
    const char *    pAkePort;
} BIP_DtcpIpServerStatus;

/**
Summary:
Get Currnet Status.

Description:

See Also:
BIP_DtcpIpServer_Start
**/
BIP_Status BIP_DtcpIpServer_GetStatus(
    BIP_DtcpIpServerHandle hDtcpIpServer,
    BIP_DtcpIpServerStatus *pStatus
    );

/**
Summary:
Datatype to hold a reference to a BIP_DtcpIpServer object.
**/
typedef struct BIP_DtcpIpServerStream *BIP_DtcpIpServerStreamHandle;        /* Main DtcpIpServer object */

/**
Summary:
API to check if Client/Sink has completed AKE with this server.

Before a stream can be opened, server has to check if Client has been authenticated via DTCP/IP AKE.
This process takes multiple message exchanges between client & server. So this API simply returns
if client has completed AKE with this server or not. Caller can re-check using this API to see if
AKE has finished or not.

Once AKE is completed, then caller can use BIP_DtcpIpServer_OpenStream() to open
the stream between the client & server.

Returns:
BIP_SUCCESS: If AKE is successfully completed with the client.
BIP_INF_DTCPIP_SERVER_AKE_NOT_DONE: AKE is not yet completed.

See Also:
BIP_DtcpIpServer_OpenStream
BIP_DtcpIpServer_CloseStream
**/

BIP_Status BIP_DtcpIpServer_CheckForClientAke(
    BIP_DtcpIpServerHandle      hDtcpIpServer,      /*!< [in] DtcpIp Server Handle. */
    const char *                pClientIp           /*!< [in] Client's IP address, required to check the AKE completion status. */
    );

/**
Summary:
API to Open a stream with the Client.

Before a stream can be opened, caller must call BIP_DtcpIpServer_CheckForClientAke() to ensure
that AKE is completed.

Once AKE is completed, then a stream is opened between the client & server. This stream allows
caller to then retrieve Exchange Key & Nonce information needed to offload the encryption
process to a h/w block (such as ASP).

See Also:
BIP_DtcpIpServer_CheckForClientAke
BIP_DtcpIpServer_CloseStream
**/
BIP_Status BIP_DtcpIpServer_OpenStream(
    BIP_DtcpIpServerHandle      hDtcpIpServer,      /*!< [in] DtcpIp Server Handle. */
    const char *                pClientIp,          /*!< [in] Client's IP address for whom this stream will be opened. */
    B_CCI_T                     copyControlInfo,    /*!< [in] CCI Info. */
    BIP_DtcpIpServerStreamHandle *phStream          /*!< [out] Stream Handle is filled-in when async API completes. */
    );

BIP_Status BIP_DtcpIpServer_CloseStream(
    BIP_DtcpIpServerHandle      hDtcpIpServer,      /*!< [in] DtcpIp Server Handle. */
    BIP_DtcpIpServerStreamHandle hStream            /*!< [in] Stream handle. */
    );

/**
Summary:
Get DTCP/IP Stream related Status for a specific client.

Description:

See Also:
BIP_DtcpIpServer_Start
**/
typedef struct BIP_DtcpIpServerStreamStatus
{
#define BIP_DTCP_IP_EXCH_KEY_IN_BYTES 12
    uint8_t                     exchKey[BIP_DTCP_IP_EXCH_KEY_IN_BYTES];
    uint8_t                     exchKeyLabel;
    unsigned                    emiModes;
#define BIP_DTCP_IP_NONCE_IN_BYTES 8
    uint8_t                     initialNonce[BIP_DTCP_IP_NONCE_IN_BYTES];
} BIP_DtcpIpServerStreamStatus;

BIP_Status BIP_DtcpIpServer_GetStreamStatus(
    BIP_DtcpIpServerHandle          hDtcpIpServer,      /*!< [in] DtcpIp Server handle. */
    BIP_DtcpIpServerStreamHandle    hStream,            /*!< [in] Stream handle. */
    BIP_DtcpIpServerStreamStatus    *pStatus            /*!< [out] Stream status. */
    );

/**
Summary:
Destroys a BIP_DtcpIpServer

See Also:
BIP_DtcpIpServer_Create
**/
void BIP_DtcpIpServer_Destroy(
    BIP_DtcpIpServerHandle hDtcpIpServer
    );

#ifdef __cplusplus
}
#endif

#endif /* BIP_DTCP_IP_SERVER_H */
