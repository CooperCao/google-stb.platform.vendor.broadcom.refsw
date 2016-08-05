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
#include "bhsm_private.h"
#include "bhsm_datatypes.h"
#include "bhsm_keyladder_enc.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_otp_key.h"

BDBG_MODULE ( BHSM );

BDBG_OBJECT_ID_DECLARE( BHSM_P_Handle );

BERR_Code BHSM_ProgOTPKey ( BHSM_Handle hHsm, BHSM_ProgramOtpKey_t * pConfig )
{
#if HSM_IS_ASKM_28NM_ZEUS_4_2
    BERR_Code       errCode = BERR_SUCCESS;
    uint8_t         status = 0;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;

    BDBG_ENTER ( BHSM_ProgOTPKey );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if ( !pConfig )
    {
        return BERR_TRACE ( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if ( ( pConfig->keyLayer < BCMD_KeyRamBuf_eKey3 ) || ( pConfig->keyLayer >= BCMD_KeyRamBuf_eMax ) )
    {
        BDBG_ERR ( ( "Invalid key layer value %d", pConfig->keyLayer ) );
        goto BHSM_P_DONE_LABEL;
    }

    if ( pConfig->keyDataSize > BHSM_OTP_ENC_KEY_DATA_LEN )
    {
        BDBG_ERR ( ( "Invalid key data size value %d", pConfig->keyDataSize ) );
        goto BHSM_P_DONE_LABEL;
    }

    errCode = BHSM_BspMsg_Create ( hHsm, &hMsg );
    if ( errCode != BERR_SUCCESS )
    {
        BERR_TRACE ( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader ( &header );
    BHSM_BspMsg_Header ( hMsg, BCMD_cmdType_eOTP_KEY_FIELD_PROG, &header );

    /* KeyDest */
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_Otp_InCmdKeyFieldProg_eKeyDest, pConfig->keyType );

    /* VKLId */
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_Otp_InCmdKeyFieldProg_eVKLId, BHSM_RemapVklId ( pConfig->virtualKeyLadderID ) );

    /* KeyLayer */
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_Otp_InCmdKeyFieldProg_eKeyLayer, pConfig->keyLayer );

    /* Fill in the Encrypted OTP key data for decryption */
    BHSM_BspMsg_PackArray ( hMsg, BCMD_Otp_InCmdKeyFieldProg_eOTPKeyData,       /* offset */
                            pConfig->keyData,   /* data   */
                            pConfig->keyDataSize        /* length */
         );

    /* Submit the command */
    errCode = BHSM_BspMsg_SubmitCommand ( hMsg );
    if ( errCode != BERR_SUCCESS )
    {
        errCode = BERR_TRACE ( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 ( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if ( status != 0 )
    {
        errCode = BERR_TRACE ( BHSM_STATUS_FAILED );
        BDBG_ERR ( ( "%s BSP status[0x%02X] error", __FUNCTION__, status ) );
        goto BHSM_P_DONE_LABEL;
    }

  BHSM_P_DONE_LABEL:

    ( void ) BHSM_BspMsg_Destroy ( hMsg );

    BDBG_LEAVE ( BHSM_ProgOTPKey );

    return errCode;

#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pConfig );
    return  BERR_TRACE( BHSM_NOT_SUPPORTED_ERR );
#endif

}
