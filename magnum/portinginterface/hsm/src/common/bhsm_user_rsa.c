/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "bhsm.h"
#include "bhsm_private.h"
#include "bsp_s_commands.h"
#include "bhsm_usercmd_common.h"
#include "bhsm_user_rsa.h"
#include "bhsm_bsp_msg.h"
#include "bsp_s_pke.h"

BDBG_MODULE(BHSM);

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)

BDBG_OBJECT_ID_DECLARE( BHSM_P_Handle );

BERR_Code BHSM_UserRSA(
        BHSM_Handle         hHsm,
        BHSM_UserRSAIO_t    *pRsa
)
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t             status;
    uint32_t            counterMeasure;
    uint16_t            outputLength;
    unsigned            timeout;

    BDBG_ENTER( BHSM_UserRSA );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pRsa == NULL )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    pRsa->unStatus = 0;
    pRsa->unOutputDataLen = 0;

    rc = BHSM_BspMsg_Create( hHsm, &hMsg );
    if( rc != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eUSER_RSA, &header );

    if(pRsa->rsaSubCmd == BCMD_User_RSA_SubCommand_eLoadBaseStartOp )
    {
        counterMeasure =  ( pRsa->bFWRSAMode1?1:0 ) |
                         (( pRsa->bHWModExpProtect?1:0 ) << 0x03);

        #if BHSM_ZEUS_VER_MAJOR != 4     /* Feature disabled for Zeus 4 */
        counterMeasure |= ((pRsa->HWRandomStall & 0x03 ) << 0x1 );
        #endif

        BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_eCountermeasure, counterMeasure );
    }
    BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_eRSASize, pRsa->rsaKeySize );
    BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_esubCmdId, pRsa->rsaSubCmd );
    BHSM_BspMsg_PackArray(  hMsg,
                            BCMD_User_RSA_InCmdField_eRSAKeyData,
                            pRsa->rsaData,
                            (pRsa->rsaKeySize == BCMD_User_RSA_Size_e1024 ?128:256) );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );   /*  send the command to the BSP. */
    if( rc != BERR_SUCCESS )
    {
        rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );

    /* return with status if RSA subcommand is not eLoadBaseStartOp */
    if (   (pRsa->rsaSubCmd == BCMD_User_RSA_SubCommand_eLoadExponent)
        || (pRsa->rsaSubCmd == BCMD_User_RSA_SubCommand_eLoadModulus)
        )
    {
        if( status != BERR_SUCCESS )
        {
            rc = BERR_TRACE( BHSM_STATUS_FAILED );
            BDBG_ERR(( "eUSER_RSA BSP ERROR status[%X] ", status ));
        }
        goto BHSM_P_DONE_LABEL;
    }

    /* status here has to be 0xA4 - PKE in Progress */
    if( status != BSP_PKE_IN_PROGRESS )
    {
        rc = BERR_TRACE( BHSM_STATUS_FAILED );
        BDBG_ERR(( "eUSER_RSA BSP ERROR status[%X] ", status ));
        goto BHSM_P_DONE_LABEL;
    }

    timeout = 4000; /* 20 seconds*/
    do
    {
        BKNI_Sleep( 5 /*ms*/);

        /* Poll for command completion. */
        BHSM_BspMsg_Header( hMsg, BCMD_cmdType_ePKE_Cmd_Poll_Status, &header ); /* reuse message instance */
        BHSM_BspMsg_Pack8( hMsg, BCMD_User_RSA_InCmdField_esubCmdId, BCMD_PollingCommand_PollingTarget_eUserRSA );

        rc = BHSM_BspMsg_SubmitCommand( hMsg );
        if( rc != BERR_SUCCESS )
        {
            rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR );
            goto BHSM_P_DONE_LABEL;
        }
        BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );

    }while( --timeout && (status == BSP_PKE_IN_PROGRESS) );

    if( timeout == 0 )
    {
        rc = BERR_TRACE( BHSM_STATUS_TIME_OUT );
        goto BHSM_P_DONE_LABEL;
    }

    if( status != 0 )
    {
        BDBG_ERR(( "PKE_Cmd_Poll_Status BSP ERROR status[%X] ", status ));
        rc = BERR_TRACE( BHSM_STATUS_FAILED );
        goto BHSM_P_DONE_LABEL;
    }

    /* Read the data from the output buffer. */
    BHSM_BspMsg_Get16( hMsg, BCMD_User_RSA_OutCmdField_eOutputDataSize, &outputLength );

    if( outputLength > 0 && outputLength <= sizeof(pRsa->aucOutputData) )
    {
        BHSM_BspMsg_GetArray( hMsg, BCMD_User_RSA_OutCmdField_eOutputData, pRsa->aucOutputData, (unsigned)outputLength  );
        pRsa->unOutputDataLen = outputLength;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_UserRSA );

    return rc;
}
#endif /* #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0) */
