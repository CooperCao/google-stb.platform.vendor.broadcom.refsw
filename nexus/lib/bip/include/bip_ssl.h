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

#ifndef BIP_SSL_H
#define BIP_SSL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip.h"

/** @addtogroup bip_ssl

BIP_SslClientFactory Interface Definition.

This interfaces provides higher level abstraction to the OpenSSL Library.

SslClientFactory holds SSL Context that is used to setup a SSL Connection
with the Server.

**/

/**
Summary:
Settings for BIP_SslClientFactory_Init().

See Also:
BIP_SslClientFactory_Init
BIP_SslClientFactory_GetDefaultInitSettings
**/
typedef struct BIP_SslClientFactoryInitSettings
{
    const char              *pRootCaCertPath;           /*!< Path for root CA cert. */
    bool                    sslLibInitDone;             /*!< Set to true if App has already initialized OpenSSL library (i.e. called SSL_library_init()). */
    BIP_SETTINGS(BIP_SslClientFactoryInitSettings)      /* Internal use... for init verification. */

} BIP_SslClientFactoryInitSettings;
BIP_SETTINGS_ID_DECLARE(BIP_SslClientFactoryInitSettings);

/**
Summary:
Get default settings for BIP_SslClientFactory_Init().

See Also:
BIP_SslClientFactoryInitSettings
BIP_SslClientFactory_Init
**/
#define BIP_SslClientFactory_GetDefaultInitSettings(pSettings)                       \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_SslClientFactoryInitSettings)  \
        /* Set non-zero defaults explicitly. */                                   \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
API to initialize SslClient Factory.

Description:
Initializes the BIP_SslClientFactory singleton.  This API must be called
before SSL functionality can be enabled by BIP.

See Also:
BIP_SslClientFactoryInitSettings
BIP_SslClientFactory_GetDefaultInitSettings
BIP_SslClientFactory_Uninit
**/
BIP_Status BIP_SslClientFactory_Init(
    BIP_SslClientFactoryInitSettings *pSettings
    );

/**
Summary:
Uninitialize the SslClientFactory.

See Also:
BIP_SslClientFactory_Init
**/
void BIP_SslClientFactory_Uninit(void);

/**
Summary:
Get Currnet Status.

**/
typedef struct BIP_SslClientFactoryStatus
{
    void            *pPbipSslCtx;
} BIP_SslClientFactoryStatus;

BIP_Status BIP_SslClientFactory_GetStatus(
    BIP_SslClientFactoryStatus *pStatus
    );

#ifdef __cplusplus
}
#endif

#endif /* BIP_SSL_H */
