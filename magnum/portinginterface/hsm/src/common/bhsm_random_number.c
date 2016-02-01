/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 *****************************************************************************/

#include "bhsm.h"
#include "bhsm_private.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_random_number.h"
#include "bsp_s_user_random_number.h"

BDBG_MODULE(BHSM);

BERR_Code BHSM_UserRandomNumber (
        BHSM_Handle                hHsm,
        BHSM_UserRandomNumberIO_t *pRandomNumber
)
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status;

    BDBG_ENTER( BHSM_UserRandomNumber );

    if( ( hHsm == NULL ) || ( hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) ) {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    /* only fill the command specific input parameters */
    if ( pRandomNumber->RandomNumType >= BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_MAX ) {
        return BERR_TRACE(  BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( pRandomNumber->keyDest != RANDOM_NUMBER_OUTPUT_TO_HOST ) {
        return BERR_TRACE(  BHSM_STATUS_INPUT_PARM_ERR );  /* only HOST supported. */
    }

    if( pRandomNumber->unDataLen > BHSM_MAX_RANDOM_NUM_LEN || ( pRandomNumber->unDataLen % 4 != 0) ) {
        return BERR_TRACE(  BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    header.hChannel = hHsm->channelHandles[BSP_CmdInterface];;
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eUSER_RANDOM_NUMBER, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_UserRandomNumber_CmdInputField_eRandomNumberType, pRandomNumber->RandomNumType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_UserRandomNumber_CmdInputField_eDestination, pRandomNumber->keyDest );
    BHSM_BspMsg_Pack16( hMsg, BCMD_UserRandomNumber_CmdInputField_eRandomNumberLength, pRandomNumber->unDataLen );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_UserRandomNumber_CmdOutputField_eStatus, &status );
    pRandomNumber->unStatus =  (uint32_t)status;
    if( status != 0 ) {
        BDBG_ERR(("Random Number FAILED [0x%02x] type[0x%02x] dest[0x%02x] len[0x%02x]", status,
                        pRandomNumber->RandomNumType, pRandomNumber->keyDest, pRandomNumber->unDataLen ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetArray( hMsg, BCMD_UserRandomNumber_CmdOutputField_eRandomNumber, pRandomNumber->aucData, pRandomNumber->unDataLen );

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE(BHSM_UserRandomNumber);
    return rc;
}
