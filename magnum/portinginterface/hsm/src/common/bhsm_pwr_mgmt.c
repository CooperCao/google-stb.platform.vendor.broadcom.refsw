/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
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
******************************************************************************/
#include "bhsm.h"
#include "bhsm_private.h"
#include "bsp_s_misc.h"
#include "bhsm_pwr_mgmt.h"
#include "bhsm_bsp_msg.h"

BDBG_MODULE(BHSM);

BDBG_OBJECT_ID_DECLARE( BHSM_P_Handle );

BERR_Code BHSM_PwrMgmt ( BHSM_Handle  hHsm,
                         BHSM_PwrMgmtIO_t    *pPwrMgmt )
{
#if BHSM_ZEUS_VERSION <= BHSM_ZEUS_VERSION_CALC(4,1)
    BERR_Code              rc = BERR_SUCCESS;
    BHSM_BspMsg_h          hMsg = NULL;
    BHSM_BspMsgHeader_t    header;
    uint8_t                status  = 0;

    BDBG_ENTER( BHSM_PwrMgmt );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pPwrMgmt == NULL )
    {
        return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_ePowerMgmtOp, &header );


    BHSM_BspMsg_Pack8( hMsg, BCMD_MISC_PowerMgmtField_InCmdField_ePwrMgmtOp, pPwrMgmt->pwrMgmtOp );
    /* BHSM_BspMsg_Pack8( hMsg, BCMD_MISC_PowerMgmtField_InCmdField_eBkGndPeriod, pPwrMgmt-> );*/

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE(rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    pPwrMgmt->unStatus = (uint32_t)status;
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_PwrMgmt );
    return rc;

#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pPwrMgmt );
    return BERR_TRACE( BERR_NOT_SUPPORTED );
#endif
}

#if ( BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0) ) && ( BHSM_ZEUS_VERSION <= BHSM_ZEUS_VERSION_CALC(2,5) )

BERR_Code BHSM_GROURMacKey( BHSM_Handle hHsm,
                            BHSM_GROURMacKeyIO_t *pGropuMacKey )
{
    BERR_Code              rc = BERR_SUCCESS;
    BHSM_BspMsg_h          hMsg = NULL;
    BHSM_BspMsgHeader_t    header;
    uint8_t                status  = 0;

    BDBG_ENTER( BHSM_GROURMacKey );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pGropuMacKey == NULL )
    {
        return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eGenerateRouteOnceUsedRandomMacKey, &header ); /*BCMD_cmdType_eRESERVED_95*/

    BHSM_BspMsg_Pack8( hMsg,BCMD_GenerateRouteOnceUsedRandomMacKey_InCmd_eOtpID , pGropuMacKey->otpID );
    BHSM_BspMsg_Pack8( hMsg,BCMD_GenerateRouteOnceUsedRandomMacKey_InCmd_eRngGenFlag , pGropuMacKey->RNGGenFlag );
    BHSM_BspMsg_Pack8( hMsg,BCMD_GenerateRouteOnceUsedRandomMacKey_InCmd_eM2MSlotNum , pGropuMacKey->M2MKeySlotNum );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE(rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    pGropuMacKey->unStatus = (uint32_t)status;
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_GROURMacKey );
    return rc;
}

#endif
