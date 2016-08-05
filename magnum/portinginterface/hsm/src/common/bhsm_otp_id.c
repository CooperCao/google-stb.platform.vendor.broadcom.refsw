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
#include "bhsm_bsp_msg.h"
#include "bsp_s_otp.h"
#include "bhsm_otp_id.h"
#include "bhsm_private.h"


BDBG_MODULE(BHSM);

BDBG_OBJECT_ID_DECLARE( BHSM_P_Handle );

BERR_Code    BHSM_ReadOTPId(
        BHSM_Handle            hHsm,
        BHSM_ReadOTPIdIO_t    *pReadOtpId
)
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status;

    BDBG_ENTER( pReadOtpId );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pReadOtpId == NULL ) {
        return BERR_TRACE(  BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOFFLINE_OTP_READ, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOfflineOtpRead_eEnum, 15 /* keyId */ );
    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOfflineOtpRead_eKeyType, (uint8_t)pReadOtpId->OtpId );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    pReadOtpId->unStatus = status;
    if( status != 0 ) {
        BDBG_ERR(("BHSM_ReadOTPId FAILED [%d]", status  ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    BDBG_CASSERT( sizeof(pReadOtpId->aucOTPId) >= 8 );
    BHSM_BspMsg_GetArray( hMsg, BCMD_Otp_OutCmdOfflineOtpRead_eRegValueLo, &pReadOtpId->aucOTPId[4], 4 );
    BHSM_BspMsg_GetArray( hMsg, BCMD_Otp_OutCmdOfflineOtpRead_eRegValueHi, &pReadOtpId->aucOTPId[0], 4 );
    pReadOtpId->unIdSize = 8;

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_ReadOTPId );
    return rc;
}
