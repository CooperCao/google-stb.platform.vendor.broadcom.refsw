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

#include "bstd.h"
#include "bhsm.h"
#include "bhsm_random_number.h"
#include "bhsm_bsp_msg.h"
#include "bsp_s_user_random_number.h"

BDBG_MODULE( BHSM );

BERR_Code _GetRandomNumber( BHSM_Handle hHsm, unsigned length/*Bytes*/, uint8_t *pDest );

#define BHSM_MAX_RANDOM_NUM_LEN   352


BERR_Code BHSM_GetRandomNumber( BHSM_Handle hHsm, BHSM_GetRandomNumberData *pData )
{
    BERR_Code rc = BERR_SUCCESS;
    unsigned itterations; /* # of time we'll call BSP for full load. */
    unsigned residual;    /* # of bytes required from last call */
    unsigned count = 0;
    uint8_t  *pDest;

    BDBG_ENTER( BHSM_GetRandomNumber );

    pDest = pData->pRandomNumber;

    itterations = pData->randomNumberSize / BHSM_MAX_RANDOM_NUM_LEN;
    while( count++ < itterations )
    {

        rc = _GetRandomNumber( hHsm, BHSM_MAX_RANDOM_NUM_LEN, pDest );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

        pDest += BHSM_MAX_RANDOM_NUM_LEN;
    }

    residual = pData->randomNumberSize % BHSM_MAX_RANDOM_NUM_LEN;
    if( residual )
    {
        if( residual/4 )
        {
            unsigned byteCount = (residual/4)*4;

            rc = _GetRandomNumber( hHsm, byteCount, pDest );
            if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
            pDest += byteCount;
        }

        if( residual%4 )
        {
            uint8_t array[4] = {0};

            rc = _GetRandomNumber( hHsm, 4, array );
            if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
            BKNI_Memcpy( pDest, array, residual%4 );
        }
    }

    BDBG_LEAVE( BHSM_GetRandomNumber );
    return rc;
}


BERR_Code _GetRandomNumber( BHSM_Handle hHsm, unsigned length/*Bytes*/, uint8_t *pDest )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = ~0;

    BDBG_ENTER( _GetRandomNumber );

    if( length % 4 ){ return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); };
    if( length > BHSM_MAX_RANDOM_NUM_LEN ){ return BERR_TRACE(  BHSM_STATUS_INPUT_PARM_ERR ); };

    rc = BHSM_BspMsg_Create( hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eUSER_RANDOM_NUMBER, &header );

    BHSM_BspMsg_Pack16( hMsg, BCMD_UserRandomNumber_CmdInputField_eRandomNumberLength, (uint16_t)length );

    rc = BHSM_BspMsg_SubmitCommand( hMsg);
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto _exit; }

    BHSM_BspMsg_Get8( hMsg, BCMD_UserRandomNumber_CmdOutputField_eStatus, &status );
    if( status != 0 ) {
        BDBG_ERR(("Random Number FAILED [0x%02x] len[0x%02x]", status, length ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto _exit;
    }

    BHSM_BspMsg_GetArray( hMsg, BCMD_UserRandomNumber_CmdOutputField_eRandomNumber, pDest, length );

_exit:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( _GetRandomNumber );
    return rc;
}
