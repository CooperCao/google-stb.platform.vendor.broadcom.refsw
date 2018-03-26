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
#include "bhsm.h"
#include "bhsm_priv.h"
#include "bhsm_rsa.h"
#include "bhsm_bsp_msg.h"
#include "bsp_s_user_rsa.h"
#include "bsp_s_pke.h"

#define BSP_PKE_IN_PROGRESS        (0xA4)
#define BHSM_POLLING_TIMES         (2000)
#define BHSM_POLLING_INTERVAL      (100)

typedef struct BHSM_Rsa {
    BHSM_Handle   hHsm;
    bool          inUse;                                   /* only one instance allowed.  */
} BHSM_Rsa;

BDBG_MODULE( BHSM );

BERR_Code BHSM_Rsa_Init(
    BHSM_Handle hHsm,
    BHSM_RsaModuleSettings * pSettings )
{
    BHSM_Rsa     *pRsa;

    BDBG_ENTER( BHSM_Rsa_Init );
    BSTD_UNUSED( pSettings );

    if( hHsm->modules.pRsa != NULL ) {
        return BERR_TRACE( BERR_UNKNOWN );
    }

    pRsa = ( BHSM_Rsa * ) BKNI_Malloc( sizeof( BHSM_Rsa ) );
    if( !pRsa ) {
        return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
    }

    BKNI_Memset( pRsa, 0, sizeof( *pRsa ) );
    pRsa->hHsm = hHsm;
    hHsm->modules.pRsa = ( void * ) pRsa;

    BDBG_ENTER( BHSM_Rsa_Init );
    return BERR_SUCCESS;
}

void BHSM_Rsa_Uninit(
    BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_Rsa_Uninit );

    if( !hHsm->modules.pRsa ) {
        BERR_TRACE( BERR_UNKNOWN );
        return;
    }

    BKNI_Free( hHsm->modules.pRsa );
    hHsm->modules.pRsa = NULL;

    BDBG_ENTER( BHSM_Rsa_Uninit );
    return;
}

BHSM_RsaHandle BHSM_Rsa_Open(
    BHSM_Handle hHsm )
{
    BHSM_Rsa     *pRsa;

    BDBG_ENTER( BHSM_Rsa_Open );

    pRsa = hHsm->modules.pRsa;  /* there is only one RSA instance in the system/chip. */
    if( !pRsa ) {
        BERR_TRACE( BERR_UNKNOWN );
        return NULL;
    }

    if( pRsa->inUse == true ) {
        BERR_TRACE( BERR_NOT_AVAILABLE );
        return NULL;
    }

    pRsa->inUse = true;

    BDBG_LEAVE( BHSM_Rsa_Open );
    return ( BHSM_RsaHandle ) pRsa;
}

void BHSM_Rsa_Close(
    BHSM_RsaHandle handle )
{
    BHSM_Rsa     *pRsa = handle;

    BDBG_ENTER( BHSM_Rsa_Close );

    if( !pRsa->inUse ) { BERR_TRACE( BERR_NOT_INITIALIZED ); return; }

    pRsa->inUse = false;

    BDBG_LEAVE( BHSM_Rsa_Close );
    return;
}

BERR_Code BHSM_Rsa_Exponentiate(
    BHSM_RsaHandle handle,
    const BHSM_RsaExponentiateSettings * pSettings )
{
    BERR_Code     rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t       status = 0;
    uint32_t      counterMeasure = 0;
    BHSM_Handle   hHsm;
    uint32_t      keySize = 0;
    uint8_t      *pData;

    BDBG_ENTER( BHSM_Rsa_Exponentiate );
    hHsm = handle->hHsm;

    if( !pSettings ) { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }

    rc = BHSM_BspMsg_Create( hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    keySize = ( pSettings->keySize == BHSM_RsaKeySize_e1024 ) ? 128 : 256;

    /*Load Modulus */
    pData = pSettings->rsaData;
    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eUSER_RSA, &header );
    BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_eRSASize, pSettings->keySize );
    BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_esubCmdId, BCMD_User_RSA_SubCommand_eLoadModulus );
    BHSM_BspMsg_PackArray( hMsg, BCMD_User_RSA_InCmdField_eRSAKeyData, pData, keySize );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR ); goto exit; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );

    /*Load Exponent */
    pData += keySize;
    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eUSER_RSA, &header );
    BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_eRSASize, pSettings->keySize );
    BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_esubCmdId, BCMD_User_RSA_SubCommand_eLoadExponent );
    BHSM_BspMsg_PackArray( hMsg, BCMD_User_RSA_InCmdField_eRSAKeyData, pData, keySize );

    rc = BHSM_BspMsg_SubmitCommand( hMsg ); /*  send the command to the BSP. */
    if( rc != BERR_SUCCESS ) { rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR ); goto exit; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );

    /*Load Base and Start Up */
    pData += keySize;
    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eUSER_RSA, &header );
    counterMeasure = pSettings->counterMeasure ? 1 : 0;
    BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_eCountermeasure, counterMeasure );
    BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_eRSASize, pSettings->keySize );
    BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_esubCmdId, BCMD_User_RSA_SubCommand_eLoadBaseStartOp );
    BHSM_BspMsg_PackArray( hMsg, BCMD_User_RSA_InCmdField_eRSAKeyData, pData, keySize );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR ); goto exit; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );

    /* status here has to be 0xA4 - PKE in Progress */
    if( status != BSP_PKE_IN_PROGRESS ) {
        rc = BERR_TRACE( BHSM_STATUS_FAILED );
        BDBG_ERR( ( "eUSER_RSA BSP ERROR status[%X] ", status ) );
        goto exit;
    }

  exit:

    if ( hMsg) { ( void ) BHSM_BspMsg_Destroy( hMsg ); }

    BDBG_LEAVE( BHSM_Rsa_Exponentiate );
    return rc;
}

BERR_Code BHSM_Rsa_GetResult(
    BHSM_RsaHandle handle,
    BHSM_RsaExponentiateResult * pResult )
{
    unsigned      timeout = BHSM_POLLING_TIMES;
    BHSM_Handle   hHsm;
    BERR_Code     rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t       status = 0;
    uint16_t      outputLength = 0;

    BDBG_ENTER( BHSM_Rsa_GetResult );
    hHsm = handle->hHsm;
    rc = BHSM_BspMsg_Create( hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );

    /* Poll for command completion.
     * Poll max for 10 seconds the RSA command completion. */
    do {
        BKNI_Sleep( BHSM_POLLING_INTERVAL );

        BHSM_BspMsg_Header( hMsg, BCMD_cmdType_ePKE_Cmd_Poll_Status, &header ); /* reuse message instance */
        BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_esubCmdId, BCMD_PollingCommand_PollingTarget_eUserRSA );

        rc = BHSM_BspMsg_SubmitCommand( hMsg );
        if( rc != BERR_SUCCESS ) {
            rc = BERR_TRACE( rc );
            goto exit;
        }

        BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    } while( --timeout && ( status == BSP_PKE_IN_PROGRESS ) );

    if( !timeout ) {
        rc = BERR_TRACE( BHSM_STATUS_TIME_OUT );
        goto exit;
    }

    if( status ) {
        BDBG_ERR( ( "PKE_Cmd_Poll_Status BSP ERROR status[%X] ", status ) );
        rc = BERR_TRACE( BHSM_STATUS_FAILED );
        goto exit;
    }

    /* Read the data from the output buffer. */
    BHSM_BspMsg_Get16( hMsg, BCMD_User_RSA_OutCmdField_eOutputDataSize, &outputLength );

    if( outputLength > 0 && outputLength <= BHSM_RSA_OUTPUT_SIZE_MAX ) {
        BHSM_BspMsg_GetArray( hMsg, BCMD_User_RSA_OutCmdField_eOutputData, pResult->data, ( unsigned ) outputLength );
        pResult->dataLength = outputLength;
    }

  exit:

    if( hMsg ) { ( void ) BHSM_BspMsg_Destroy( hMsg ); }

    BDBG_LEAVE( BHSM_Rsa_GetResult );
    return rc;
}
