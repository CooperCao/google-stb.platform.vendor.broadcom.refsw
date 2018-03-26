/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bstd.h"
#include "nexus_base.h"
#include "nexus_security_module.h"
#include "priv/nexus_security_priv.h"
#include "nexus_types.h"
#include "nexus_rsa.h"
#include "bhsm.h"
#include "bhsm_rsa.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_rsa);

/* Module data for NEXUS RSA */

#define NEXUS_RSA_P_SERVICE_INTERVAL_MS  (100) /* how often the module wakes up to poll HSM */
#define NEXUS_RSA_P_DATA_SIZE            (256*3) /* TODO set correctly.  */

typedef enum
{
    NEXUS_P_RsaState_eIdle,
    NEXUS_P_RsaState_eQueued,
    NEXUS_P_RsaState_eInProgress,
    NEXUS_P_RsaState_eFinished,

    NEXUS_P_RsaState_eMax
}NEXUS_P_RsaState;

/* RSA Instance data, one instance per handle. */
typedef struct NEXUS_Rsa  /* Instance handle structure */
{
    NEXUS_OBJECT(NEXUS_Rsa);

    BLST_S_ENTRY(NEXUS_Rsa) link;
    NEXUS_RsaExponentiateSettings settings; /* the instance settings/data. */
    NEXUS_RsaExponentiateResult result;     /* the instance result. */

    NEXUS_P_RsaState state;                 /* current state of instance. */
    void *pRsaData;                         /* NEXUS memory used to hold RSA data.*/

    unsigned debugInstanceCount;
}NEXUS_Rsa;


static struct
{
    BHSM_RsaHandle hHsmRsa;  /* only one HSM Rsa instance. */
    BLST_S_HEAD(nexus_rsa_job_list, NEXUS_Rsa) rsaJobs; /* list of pending RSA Jobs */

    NEXUS_TimerHandle engineTimer;
    NEXUS_Rsa *pCurrentInstance;

    unsigned debugInstanceCount;
    unsigned debugCounter;
} gRsaModuleData;


static void _RsaEngine( void *data );
static BHSM_RsaKeySize  _HsmKeySize( NEXUS_RsaKeySize  keySize );



NEXUS_Error NEXUS_Rsa_Init( void )
{
    BHSM_Handle hHsm;

    BDBG_ENTER( NEXUS_Rsa_Init );

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { return BERR_TRACE( NEXUS_NOT_INITIALIZED ); }

    BKNI_Memset( &gRsaModuleData, 0, sizeof(gRsaModuleData) );

    gRsaModuleData.hHsmRsa = BHSM_Rsa_Open( hHsm );
    if( !gRsaModuleData.hHsmRsa ) { return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); }

    BLST_S_INIT( &gRsaModuleData.rsaJobs );

    BDBG_LEAVE( NEXUS_Rsa_Init );
    return NEXUS_SUCCESS;
}


void NEXUS_Rsa_Uninit( void )
{
    BDBG_ENTER( NEXUS_Rsa_Uninit );

    if( gRsaModuleData.engineTimer ) NEXUS_CancelTimer( gRsaModuleData.engineTimer );
    if( gRsaModuleData.hHsmRsa ) BHSM_Rsa_Close( gRsaModuleData.hHsmRsa );

    BKNI_Memset( &gRsaModuleData, 0, sizeof(gRsaModuleData) );
    BDBG_LEAVE( NEXUS_Rsa_Uninit );
    return;
}


NEXUS_RsaHandle NEXUS_Rsa_Open( unsigned index )
{
    NEXUS_RsaHandle handle;
    NEXUS_Error rc;
    NEXUS_MemoryAllocationSettings memSettings;

    BDBG_ENTER( NEXUS_Rsa_Open );
    BSTD_UNUSED( index );

    handle = BKNI_Malloc( sizeof(struct NEXUS_Rsa) );
    if( !handle ) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return NULL; }
    NEXUS_OBJECT_INIT(NEXUS_Rsa, handle);

    NEXUS_Memory_GetDefaultAllocationSettings( &memSettings );
    memSettings.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    rc = NEXUS_Memory_Allocate( NEXUS_RSA_P_DATA_SIZE, &memSettings, &handle->pRsaData );
    if( rc ) { BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto error; }

    handle->state = NEXUS_P_RsaState_eIdle;
    handle->debugInstanceCount = gRsaModuleData.debugInstanceCount++;

    BDBG_LEAVE( NEXUS_Rsa_Open );
    return handle;

error:
    if( handle ) {
        if( handle->pRsaData )NEXUS_Memory_Free( handle->pRsaData );
        NEXUS_OBJECT_DESTROY(NEXUS_Rsa, handle);
        BKNI_Free( handle );
    }
    return NULL;
}

NEXUS_OBJECT_CLASS_MAKE( NEXUS_Rsa, NEXUS_Rsa_Close );

static void NEXUS_Rsa_P_Finalizer( NEXUS_RsaHandle handle )
{
    BDBG_ENTER( NEXUS_Rsa_P_Finalizer );
    NEXUS_OBJECT_ASSERT( NEXUS_Rsa, handle );

    BDBG_MSG(("[%s] RSA instance [%d]", BSTD_FUNCTION, handle->debugInstanceCount ));

    if( handle->pRsaData )NEXUS_Memory_Free( handle->pRsaData );

    NEXUS_OBJECT_DESTROY(NEXUS_Rsa, handle);
    BKNI_Free( handle );

    BDBG_LEAVE( NEXUS_Rsa_P_Finalizer );
	return;
}

void  NEXUS_Rsa_GetDefaultExponentiateSettings( NEXUS_RsaExponentiateSettings *pSettings )
{
    BDBG_ENTER( NEXUS_Rsa_GetDefaultExponentiateSettings );

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );
    NEXUS_CallbackDesc_Init( &pSettings->rsaComplete );

    BDBG_LEAVE( NEXUS_Rsa_GetDefaultExponentiateSettings );
    return;
}

NEXUS_Error NEXUS_Rsa_Exponentiate( NEXUS_RsaHandle handle, const NEXUS_RsaExponentiateSettings *pSettings )
{
    NEXUS_Rsa *pInstance = (NEXUS_Rsa*)handle;
    unsigned keySizeBytes = 0;

    BDBG_ENTER( NEXUS_Rsa_Exponentiate );
    NEXUS_OBJECT_ASSERT( NEXUS_Rsa, handle );

    BDBG_MSG(("[%s] RSA instance [%d]", BSTD_FUNCTION, pInstance->debugInstanceCount ));

    if( pInstance->state != NEXUS_P_RsaState_eIdle ) { return BERR_TRACE(NEXUS_SECURITY_STATE_ERROR); }

    pInstance->settings = *pSettings;

    switch( pSettings->keySize ) {
        case NEXUS_RsaKeySize_e1024: { keySizeBytes = 128; break; }
        case NEXUS_RsaKeySize_e2048: { keySizeBytes = 256; break; }
        default: return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    /* copy modulus, exponent and base into contigious device menory. */
    BKNI_Memcpy( pInstance->pRsaData,                                  pSettings->modulus,   keySizeBytes );
    BKNI_Memcpy( (void*)((char*)pInstance->pRsaData + keySizeBytes),   pSettings->exponent,  keySizeBytes );
    BKNI_Memcpy( (void*)((char*)pInstance->pRsaData + keySizeBytes*2), pSettings->inputBase, keySizeBytes );

    BLST_S_INSERT_HEAD( &gRsaModuleData.rsaJobs, pInstance, link );
    pInstance->state = NEXUS_P_RsaState_eQueued;

    if( gRsaModuleData.engineTimer ) NEXUS_CancelTimer( gRsaModuleData.engineTimer );
    gRsaModuleData.engineTimer = NEXUS_ScheduleTimer( 1, _RsaEngine, NULL );

    BDBG_LEAVE( NEXUS_Rsa_Exponentiate );
    return NEXUS_SUCCESS;
}

void NEXUS_Rsa_GetDefaultResult( NEXUS_RsaExponentiateResult *pResult )
{
    BDBG_ENTER( NEXUS_Rsa_GetDefaultResult );

    BKNI_Memset( pResult, 0, sizeof(*pResult) );

    BDBG_LEAVE( NEXUS_Rsa_GetDefaultResult );
    return;
}

NEXUS_Error NEXUS_Rsa_GetResult( NEXUS_RsaHandle handle, NEXUS_RsaExponentiateResult *pResult )
{
    NEXUS_Rsa *pInstance = (NEXUS_Rsa*)handle;

    BDBG_ENTER( NEXUS_Rsa_GetResult );
    NEXUS_OBJECT_ASSERT( NEXUS_Rsa, handle );

    BDBG_MSG(("[%s] RSA instance [%d]", BSTD_FUNCTION, pInstance->debugInstanceCount ));

    if( pInstance->state == NEXUS_P_RsaState_eQueued || pInstance->state == NEXUS_P_RsaState_eInProgress ) {
        return NEXUS_SECURITY_PENDING;
    }

    if( pInstance->state != NEXUS_P_RsaState_eFinished ) return BERR_TRACE(NEXUS_SECURITY_STATE_ERROR);

    *pResult = pInstance->result;

    pInstance->state = NEXUS_P_RsaState_eIdle;

    BDBG_LEAVE( NEXUS_Rsa_GetResult );
    return NEXUS_SUCCESS;
}


void _RsaEngine( void *data )
{
    NEXUS_Rsa *pInstance = NULL;
    BERR_Code rc;

    BSTD_UNUSED( data );
    BDBG_LOG(( "PRINT FROM [%s] [%d]", BSTD_FUNCTION, gRsaModuleData.debugCounter++ ));

    if( gRsaModuleData.pCurrentInstance == NULL ) {

        NEXUS_Rsa *pIterator = NULL;

        /* get item at tail of list. */
        pIterator = BLST_S_FIRST( &gRsaModuleData.rsaJobs );
        while( pIterator ) {
            gRsaModuleData.pCurrentInstance = pIterator;
            pIterator = BLST_S_NEXT( pIterator, link );
        }

        if( gRsaModuleData.pCurrentInstance == NULL ) return;
    }

    pInstance = gRsaModuleData.pCurrentInstance;

    if( pInstance->state == NEXUS_P_RsaState_eQueued ){ /* pass the item to BHSM */

        BHSM_RsaExponentiateSettings hsmParam;

        BKNI_Memset( &hsmParam, 0, sizeof(hsmParam) );
        hsmParam.keySize = _HsmKeySize( pInstance->settings.keySize );
        hsmParam.counterMeasure = pInstance->settings.counterMeasures;
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(5,0)
        hsmParam.rsaData = NEXUS_AddrToOffset( pInstance->pRsaData );
#else
        hsmParam.rsaData = pInstance->pRsaData;
#endif

        NEXUS_Memory_FlushCache( pInstance->pRsaData, NEXUS_RSA_P_DATA_SIZE );
        rc = BHSM_Rsa_Exponentiate( gRsaModuleData.hHsmRsa, &hsmParam );
        if( rc != BERR_SUCCESS ) { BERR_TRACE( NEXUS_SECURITY_HSM_ERROR ); return; }

        BDBG_LOG(( "PRINT FROM [%s] Exponentiated, inProgress ", BSTD_FUNCTION ));
        pInstance->state = NEXUS_P_RsaState_eInProgress;
    }
    else if( pInstance->state == NEXUS_P_RsaState_eInProgress ) { /* Check HSM progress. */
        BHSM_RsaExponentiateResult hsmParam;

        BKNI_Memset( &hsmParam, 0, sizeof(hsmParam) );

        BDBG_LOG(( "PRINT FROM [%s] GettingResult, finished", BSTD_FUNCTION ));
        rc = BHSM_Rsa_GetResult( gRsaModuleData.hHsmRsa, &hsmParam );
        if( rc != BERR_SUCCESS ) { BERR_TRACE( NEXUS_SECURITY_HSM_ERROR ); return; }

        BDBG_CASSERT( sizeof(pInstance->result.data) == sizeof(hsmParam.data) );

        /* copy back result */
        BKNI_Memcpy( pInstance->result.data, hsmParam.data, hsmParam.dataLength /* sizeof(pInstance->result.data)*/ );
        pInstance->result.dataLength = hsmParam.dataLength /* pInstance->result.dataLength*/;

        pInstance->state = NEXUS_P_RsaState_eFinished;
    }

    gRsaModuleData.engineTimer = NEXUS_ScheduleTimer( 500, _RsaEngine, NULL );

    return;
}



static BHSM_RsaKeySize _HsmKeySize( NEXUS_RsaKeySize  keySize )
{
    switch(keySize)
    {
        case NEXUS_RsaKeySize_e1024: return BHSM_RsaKeySize_e1024;
        case NEXUS_RsaKeySize_e2048: return BHSM_RsaKeySize_e2048;
        default: BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }
    return (BHSM_RsaKeySize)keySize;
}
