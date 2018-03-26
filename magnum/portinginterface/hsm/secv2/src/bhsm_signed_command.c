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

#include "bhsm.h"
#include "bhsm_priv.h"
#include "bsp_types.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_signed_command.h"
#include "bhsm_otp_priv.h"
#include "bsp_components.h"
#include "bsp_otpmsp.h"

BDBG_MODULE( BHSM );

BERR_Code BHSM_SignedCommand_Set( BHSM_Handle hHsm, const BHSM_SignedCommand *pCmd )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_BspMsg_h hMsg = NULL;
    uint16_t bspError = 0;
    BHSM_BspMsgCreate_t msgCreate;
    BHSM_BspMsgConfigure_t msgConfig;
    uint32_t word = 0;
    char *pSend = NULL;
    unsigned component = 0;
    unsigned command = 0;

    BDBG_ENTER( BHSM_SignedCommand_Set );

    if( !pCmd ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if( !pCmd->pCommand ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    word = (uint32_t)(*(uint32_t*)pCmd->pCommand);
    component = (unsigned)((word>>8) & 0xFF);
    command   = (unsigned)( word     & 0xFF);

    if( component == Bsp_CmdComponent_eOtpMsp && command == Bsp_CmdOtpMsp_eProg )
    {
        rc = BHSM_Otp_EnableProgram_priv( hHsm, !BHSM_OTP_CACHE_PROGRAM_REQUEST );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
    }

    BKNI_Memset( &msgCreate, 0, sizeof(msgCreate) );
    BKNI_Memset( &msgConfig, 0, sizeof(msgConfig) );

    hMsg = BHSM_BspMsg_Create( hHsm, &msgCreate );
    if( !hMsg ) { return BERR_TRACE( BERR_NOT_AVAILABLE ); }

    pSend = msgCreate.pSend;

    msgConfig.component = component;
    msgConfig.command = command;

    msgConfig.signedCommand.enable = true;
    msgConfig.signedCommand.rsaKeyId = pCmd->rsaKeyId;
    msgConfig.signedCommand.signatureOffset = pCmd->signatureOffset;

    rc = BHSM_BspMsg_Configure( hMsg, &msgConfig );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }

    BKNI_Memcpy( pSend,
                 (pCmd->pCommand + 4),             /* first 4 bytes (re)compiled internal to msg. */
                 (BHSM_SIGNED_COMMAND_SIZE - 4) ); /* account for above 4 bytes. */

    rc = BHSM_BspMsg_SubmitCommand ( hMsg, &bspError );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }

    if( bspError != 0 ) {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP Status error [0x%X]", BSTD_FUNCTION, bspError ));
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_SignedCommand_Set );

    return BERR_SUCCESS;
}
