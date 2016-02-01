/******************************************************************************
 * (c) 2015 Broadcom Corporation
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
#include "bip_priv.h"

BDBG_MODULE( bip_ssl_client );

BDBG_OBJECT_ID( BIP_SslClient );

BIP_SETTINGS_ID(BIP_SslClientFactoryInitSettings);

typedef struct BIP_SslClientFactory *BIP_SslClientFactoryHandle;

typedef enum BIP_SslClientFactoryState
{
    BIP_SslClientFactoryState_eNotInitialized,
    BIP_SslClientFactoryState_eInitialized,
    BIP_SslClientFactoryState_eMax
} BIP_SslClientFactoryState;

/*
 * The BIP_SslClientFactory class is a singleton (only one instance) that maintains the list of BIP_SslClientFactoryAkeEntry structures.
 */
typedef struct BIP_SslClientFactory
{
    B_MutexHandle                           hStateMutex;            /* Mutex for SslClientFactory state. */
    BIP_SslClientFactoryState               state;
    void                                    *pPbipSslCtx;
} BIP_SslClientFactory;

/* Allocate a single instance of the SslClientFactory. */
#ifdef B_HAS_SSL
static BIP_SslClientFactory g_BIP_SslClientFactory;
#endif

void BIP_SslClientFactory_Uninit()
{
#ifdef B_HAS_SSL
    BIP_Status completionStatus;
    BIP_SslClientFactoryHandle hSslClientFactory = &g_BIP_SslClientFactory;

    BIP_CHECK_GOTO(( hSslClientFactory->state == BIP_SslClientFactoryState_eInitialized), ( "SslClientFactory is not yet initialized, did you forget to call BIP_SslClientFactory_Init() before calling BIP_SslClientFactory_Uninit()"),
            error, BIP_ERR_INVALID_API_SEQUENCE, completionStatus );

    if (hSslClientFactory->pPbipSslCtx)
    {
        B_PlaybackIp_SslUnInit(hSslClientFactory->pPbipSslCtx);
        hSslClientFactory->pPbipSslCtx = NULL;
    }

    if (hSslClientFactory->hStateMutex)
    {
        B_Mutex_Destroy(hSslClientFactory->hStateMutex);
    }

    hSslClientFactory->state = BIP_SslClientFactoryState_eNotInitialized;
    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_SslClientFactory: Uninitialized" BIP_MSG_PRE_ARG ));
error:
    return;
#else
    BDBG_WRN(( BIP_MSG_PRE_FMT "Did you forget to compile with SSL_SUPPORT? Please do a clean build after exporting it!!" BIP_MSG_PRE_ARG ));
#endif /* B_HAS_SSL */
}

BIP_Status BIP_SslClientFactory_Init(
    BIP_SslClientFactoryInitSettings *pSettings
    )
{
#ifdef B_HAS_SSL
    BIP_Status completionStatus;
    BIP_SslClientFactoryHandle hSslClientFactory = &g_BIP_SslClientFactory;
    BIP_SslClientFactoryInitSettings defaultSettings;

    if (hSslClientFactory->state == BIP_SslClientFactoryState_eInitialized) return BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_SslClientFactory: Initializing" BIP_MSG_PRE_ARG ));
    BKNI_Memset(hSslClientFactory, 0, sizeof(*hSslClientFactory));

    if (pSettings == NULL)
    {
        BIP_SslClientFactory_GetDefaultInitSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }

    BIP_SETTINGS_ASSERT(pSettings, BIP_SslClientFactoryInitSettings);

    /* Create a mutext to protect the state. */
    hSslClientFactory->hStateMutex = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( hSslClientFactory->hStateMutex ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, completionStatus );

    /* Initialize SSL Library via PBIP SSL i/f. */
    {
        B_PlaybackIpSslInitSettings pbipSslInitSettings;
        BKNI_Memset(&pbipSslInitSettings, 0, sizeof(pbipSslInitSettings));
        pbipSslInitSettings.rootCaCertPath = (char *)pSettings->pRootCaCertPath;
        pbipSslInitSettings.sslLibInitDone = pSettings->sslLibInitDone;
        hSslClientFactory->pPbipSslCtx = B_PlaybackIp_SslInit(&pbipSslInitSettings);
        BIP_CHECK_GOTO( ( hSslClientFactory->pPbipSslCtx ), ( "B_PlaybackIp_SslInit() Failed"), error, BIP_ERR_SSL_INIT, completionStatus );
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_SslClientFactory is Initialized." BIP_MSG_PRE_ARG ));
    hSslClientFactory->state = BIP_SslClientFactoryState_eInitialized;
    return (BIP_SUCCESS);

error:
    BIP_SslClientFactory_Uninit();
    return (completionStatus);
#else
    BSTD_UNUSED(pSettings);
    BDBG_WRN(( BIP_MSG_PRE_FMT "Did you forget to compile with SSL_SUPPORT? Please do a clean build after exporting it!!" BIP_MSG_PRE_ARG ));
    return (BIP_ERR_NOT_SUPPORTED);
#endif /* B_HAS_SSL */
} /* BIP_SslClientFactory_Init */

BIP_Status BIP_SslClientFactory_GetStatus(
    BIP_SslClientFactoryStatus *pStatus
    )
{
#ifdef B_HAS_SSL
    BIP_Status completionStatus;
    BIP_SslClientFactoryHandle hSslClientFactory = &g_BIP_SslClientFactory;

    BIP_CHECK_GOTO(( hSslClientFactory->state == BIP_SslClientFactoryState_eInitialized), ( "SslClientFactory is not yet initialized, did you forget to call BIP_SslClientFactory_Init() before calling BIP_SslClientFactory_GetStatus()"),
            error, BIP_ERR_INVALID_API_SEQUENCE, completionStatus );

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->pPbipSslCtx = hSslClientFactory->pPbipSslCtx;
    completionStatus = BIP_SUCCESS;
error:
    return (completionStatus);
#else
    BSTD_UNUSED(pStatus);
    BDBG_WRN(( BIP_MSG_PRE_FMT "Error: Did you forget to compile with SSL_SUPPORT? Please do a clean build after exporting it!!" BIP_MSG_PRE_ARG ));
    return (BIP_ERR_INVALID_API_SEQUENCE);
#endif
}
