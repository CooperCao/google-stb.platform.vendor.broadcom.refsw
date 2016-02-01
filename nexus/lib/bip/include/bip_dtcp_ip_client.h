/******************************************************************************
 * (c) 2007-2015 Broadcom Corporation
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
 *
 *****************************************************************************/

#ifndef BIP_DTCP_IP_CLIENT_H
#define BIP_DTCP_IP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip_dtcp_ip.h"

/** @addtogroup bip_dtcp_ip_client

BIP_DtcpIpClientFactory & BIP_DtcpIpClient Interface Definition.

These interfaces provides higher level abstraction to Broadcom's DTCP/IP library.

DtcpIpClientFactory holds authentication information (AKE) for each
Server that any BIP client (e.g., BIP_Player) interacts with.

This allows multiple DtcpIpClient instances (main/pip/background recordings), which
interact with the same Server, to use the same Server entry.

Factory carries out task of authenticating the client with server
and returns a context back to the caller. This context can be used to obtain
the decryption context.

**/

/**
Summary:
Settings for BIP_DtcpIpClientFactory_Init().

See Also:
BIP_DtcpIpClientFactory_Init
BIP_DtcpIpClientFactory_GetDefaultInitSettings
**/
typedef struct BIP_DtcpIpClientFactoryInitSettings
{
    bool                pcpUsageRuleEnabled;        /*!< Optional: Enable PCP-UR feature in the DTCP library. */
                                                    /*!< true => PCP-UR of 16 bits (containing Media Usage Rule) + Nonce of 48 bits. */
                                                    /*!< false => PCP-UR is not used & PCP will contain Nonce of 48 bits. */
                                                    /*!< Defaults to false. */

    B_DTCP_KeyFormat_T  keyFormat;                  /*!< Format of DTCP Certificate & Key. */
                                                    /*!< Defaults to B_DTCP_KeyFormat_eCommonDRM: DTLA Certificate,
                                                         Production key, & Constants are wrapped in the Common DRM bin format. */
                                                    /*!< For testing, if CommonDRM keys are not available, it can be set to
                                                         B_DTCP_KeyFormat_eTest: DTCP/IP library will internally generate test keys. */
    BIP_SETTINGS(BIP_DtcpIpClientFactoryInitSettings) /* Internal use... for init verification. */

} BIP_DtcpIpClientFactoryInitSettings;
BIP_SETTINGS_ID_DECLARE(BIP_DtcpIpClientFactoryInitSettings);

/**
Summary:
Get default settings for BIP_DtcpIpClientFactory_Init().

See Also:
BIP_DtcpIpClientFactoryInitSettings
BIP_DtcpIpClientFactory_Init
**/
#define BIP_DtcpIpClientFactory_GetDefaultInitSettings(pSettings)                       \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_DtcpIpClientFactoryInitSettings)  \
        /* Set non-zero defaults explicitly. */                                   \
        (pSettings)->keyFormat = B_DTCP_KeyFormat_eCommonDRM;                     \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
API to initialize DtcpIpClient Factory.

Description:
Initializes the BIP_DtcpIpClientFactory singleton.  This API must be called
before any other BIP_DtcpIPClientFactory_ APIs.

See Also:
BIP_DtcpIpClientFactoryInitSettings
BIP_DtcpIpClientFactory_GetDefaultInitSettings
BIP_DtcpIpClientFactory_Uninit
**/
BIP_Status BIP_DtcpIpClientFactory_Init(
    BIP_DtcpIpClientFactoryInitSettings *pSettings
    );

/**
Summary:
Uninitialize the DtcpIpClientFactory.

See Also:
BIP_DtcpIpClientFactory_Init
**/
void BIP_DtcpIpClientFactory_Uninit(void);

/**
Summary:
Settings for BIP_DtcpIpClientFactory_DoAke().

Description:
This API allows caller to do authentication with the Server.
API provides both blocking & async API.

See Also:
BIP_DtcpIpClientFactory_GetDefaultDoAkeSettings
BIP_DtcpIpClientFactory_DoAke
**/
typedef struct BIP_DtcpIpClientFactoryDoAkeSettings
{
    int     timeoutInMs;                                /*!< API timeout: This API fails if it is not completed in this timeout interval. -1: waits until completion or error. */
    BIP_SETTINGS(BIP_DtcpIpClientFactoryDoAkeSettings)  /* Internal use... for init verification. */

} BIP_DtcpIpClientFactoryDoAkeSettings;
BIP_SETTINGS_ID_DECLARE(BIP_DtcpIpClientFactoryDoAkeSettings);

/**
Summary:
Get default settings for BIP_DtcpIpClientFactory_GetDefaultDoAkeSettings().

See Also:
BIP_DtcpIpClientFactoryDoAkeSettings
BIP_DtcpIpClientFactory_DoAke
**/
#define BIP_DtcpIpClientFactory_GetDefaultDoAkeSettings(pSettings)                          \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_DtcpIpClientFactoryDoAkeSettings)     \
        /* Set non-zero defaults explicitly. */                                             \
        (pSettings)->timeoutInMs = -1;   /* blocking */                                     \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
States of each AkeEntry.

**/
typedef enum BIP_DtcpIpClientFactoryAkeEntryState
{
    BIP_DtcpIpClientFactoryAkeEntryState_eIdle,
    BIP_DtcpIpClientFactoryAkeEntryState_eNewAke,
    BIP_DtcpIpClientFactoryAkeEntryState_eWaitingForAke,
    BIP_DtcpIpClientFactoryAkeEntryState_eAkeDone,
    BIP_DtcpIpClientFactoryAkeEntryState_eMax
} BIP_DtcpIpClientFactoryAkeEntryState;

/**
Summary:
API to Perform AKE (DTCP/IP Authentication) Handshake with a given Server.

Description:
This API performs Partial AKE handshake with Server if it has previously authenticated
with that Server. If that fails or its is the first handshake with the Server,
API will do full AKE Authentication with the Server.

Upon success, it return BIP_SUCCESS & fills-in a AKE handle (specific to that server)
that can be later used by the DtcpIpClient for obtaining Decryption Context.

If full authentication also fails, then BIP_Status is set to reflect the error status and
AKE handle is not filled-in.

If 2nd instance of DtcpIpClient (on behalf of 2nd BIP Player instance) calls this API
while 1st one is in progress, the 2nd instance will be blocked until 1st one completes.

Given this blocking nature of this API, an Async version of the API is also provided.
This allows callers to be notified via Async Callback & Async Status about the API completion
and its BIP Status.

See Also:
BIP_DtcpIpClientFactory_GetDefaultDoAkeSettings
BIP_DtcpIpClientFactoryDoAkeSettings
**/

typedef struct BIP_DtcpIpClientFactoryAkeEntry *BIP_DtcpIpClientFactoryAkeEntryHandle;

BIP_Status BIP_DtcpIpClientFactory_DoAke(
    const char                              *pServerIp,         /*!< [in]  Required: IP Address of Server to do AKE with. */
    const char                              *pServerPort,       /*!< [in]  Required: Port # of Server to do AKE with. */
    BIP_DtcpIpClientFactoryDoAkeSettings    *pSettings,         /*!< [in]  Optional Settings. */
    BIP_DtcpIpClientFactoryAkeEntryHandle   *phAke              /*!< [out] Required: returned AKE handle, valid upon success. */
    );

BIP_Status BIP_DtcpIpClientFactory_DoAkeAsync(
    const char                              *pServerIp,         /*!< [in]  Required: IP Address of Server to do AKE with. */
    const char                              *pServerPort,       /*!< [in]  Required: Port # of Server to do AKE with. */
    BIP_DtcpIpClientFactoryDoAkeSettings    *pSettings,         /*!< [in]  Optional Settings. */
    BIP_DtcpIpClientFactoryAkeEntryHandle   *phAke,             /*!< [out] Required: returned AKE handle, valid upon success. */
    BIP_CallbackDesc                        *pAsyncCallback,    /*!< [in]  Async completion callback: called at API completion. */
    BIP_Status                              *pAsyncStatus       /*!< [out] Completion status of async API. */
                                                                /*!< Note: App can specify either both pAsyncCallback & pAsyncStatus or either one of them depending upon how it wants to use Async API. */
    );

/**
Summary:
Get Status associated with a given AkeEntryHandle.

See Also:
**/
typedef struct BIP_DtcpIpClientFactoryAkeEntryStatus
{
    void        *pDtcpIpLibAkeCtx;
    const char  *pServerIp;
    const char  *pServerPort;
} BIP_DtcpIpClientFactoryAkeEntryStatus;

BIP_Status BIP_DtcpIpClientFactory_GetAkeEntryStatus(
    BIP_DtcpIpClientFactoryAkeEntryHandle   hDtcpIpClientFactoryAkeEntry,
    BIP_DtcpIpClientFactoryAkeEntryStatus   *pAkeEntryStatus
    );


#if 0

/* The following APIs & definitions are not yet implemented as they are currently not required when using the PBIP library interface. */
BIP_Status BIP_DtcpIpClientFactory_OpenDecryptionCtx(
    BIP_DtcpIpClientFactoryAkeEntryHandle   hAke,                       /*!< [in]  Required: AKE Entry handle for Server from which Client is receiving Encrypted stream. */
    void                                    **pDtcpIpLibDecryptionCtx   /*!< [out] Decryption Context returned by the DTCP/IP library which can be used in the Decryption API. */
    );

/**
Summary:
Datatype to hold a reference to a BIP_DtcpIpClient object.
**/
typedef struct BIP_DtcpIpClient *BIP_DtcpIpClientHandle;        /* Main DtcpIpClient object */

/**
Summary:
Settings for BIP_DtcpIpClientCreate().

See Also:
BIP_DtcpIpClient_Create
BIP_DtcpIpClient_GetDefaultCreateSettings
**/
typedef struct BIP_DtcpIpClientCreateSettings
{
    BIP_SETTINGS(BIP_DtcpIpClientCreateSettings) /* Internal use... for init verification. */

} BIP_DtcpIpClientCreateSettings;
BIP_SETTINGS_ID_DECLARE(BIP_DtcpIpClientCreateSettings);

/**
Summary:
Get default settings for BIP_DtcpIpClient_Create().

See Also:
BIP_DtcpIpClient_Create
BIP_DtcpIpClientCreateSettings
**/
#define BIP_DtcpIpClient_GetDefaultCreateSettings(pSettings)                       \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_DtcpIpClientCreateSettings)  \
        /* Set non-zero defaults explicitly. */                                   \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Create an DtcpIpClient

Description:

Return:
    non-NULL :        A BIP_DtcpIpClientHandle used for calling subsequent DtcpIpClient related APIs
Return:
    NULL     :        Failure, an DtcpIpClient instance could not be created.

See Also:
BIP_DtcpIpClientCreateSettings
BIP_DtcpIpClient_GetDefaultCreateSettings
BIP_DtcpIpClient_Destroy
**/
BIP_DtcpIpClientHandle BIP_DtcpIpClient_Create(
    const BIP_DtcpIpClientCreateSettings *pSettings
    );

typedef struct BIP_DtcpIpClientFoctoryDoAkeSettings
{
    int unused;          /*!< Unused placeholder. */
} BIP_DtcpIpClientFoctoryDoAkeSettings;

/**
Summary:
Start a BIP_DtcpIpClient

Description:
BIP will start listening on client socket and accept incoming HTTP requests after this call.

See Also:
BIP_DtcpIpClientDoAkeFactorySettings
BIP_DtcpIpClient_GetDefaultDoAkeSettings
BIP_DtcpIpClient_Stop
**/
BIP_Status BIP_DtcpIpClient_DoAke(
    BIP_DtcpIpClientHandle                  hDtcpIpClient,
    const char                              *pServerIp,
    const char                              *pServerPort,
    BIP_DtcpIpClientFactoryDoAkeSettings    *pSettings
    );

/**
Summary:
Stop a BIP_DtcpIpClient

Description:
BIP will not accept any new connection requests after this call.

See Also:
BIP_DtcpIpClient_Start
**/
BIP_Status BIP_DtcpIpClient_OpenDecryptionCtx(
    BIP_DtcpIpClientHandle hDtcpIpClient
    );

BIP_Status BIP_DtcpIpClient_CloseDecryptionCtx(
    BIP_DtcpIpClientHandle hDtcpIpClient
    );

BIP_Status BIP_DtcpIpClient_Decrypt(
    BIP_DtcpIpClientHandle  hDtcpIpClient,
    uint8_t                 *pEncryptedBuffer,
    size_t                  encryptedBufferSize,
    uint8_t                 *pDecryptedBuffer,
    size_t                  *pDecryptedBufferSize
    );

typedef struct BIP_DtcpIpClientStatus
{
    void *          pDtcpIpLibAkeCtx;
    const char *    pAkePort;
} BIP_DtcpIpClientStatus;

/**
Summary:
Get Currnet Status.

Description:

See Also:
BIP_DtcpIpClient_Start
**/
BIP_Status BIP_DtcpIpClient_GetStatus(
    BIP_DtcpIpClientHandle hDtcpIpClient,
    BIP_DtcpIpClientStatus *pStatus
    );

/**
Summary:
Destroys a BIP_DtcpIpClient

See Also:
BIP_DtcpIpClient_Create
**/
void BIP_DtcpIpClient_Destroy(
    BIP_DtcpIpClientHandle hDtcpIpClient
    );
#endif /* if 0 */

#ifdef __cplusplus
}
#endif

#endif /* BIP_DTCP_IP_CLIENT_H */
