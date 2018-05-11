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

#include "bhsm_private.h"
#include "bhsm_exception_status.h"
#include "bhsm_bsp_msg.h"
#include "bsp_s_misc.h"

BDBG_MODULE(BHSM);

#define UINT64_WORD_HI(a) ((uint32_t)((a)>>32))
#define UINT64_WORD_LO(a) ((uint32_t)((a)&0xFFFFFFFF))




BERR_Code  BHSM_GetExceptionStatus(
        BHSM_Handle                    hHsm,
        BHSM_ExceptionStatusRequest_t *pRequest,
        BHSM_ExceptionStatus_t        *pStatus )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t             status = 0;

    BDBG_ENTER( BHSM_GetExceptionStatus );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( ( pRequest == false ) || ( pStatus == false ) )
    {
        return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( (BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(4,2)) && !isBfwVersion_GreaterOrEqual( hHsm, 4,0,0 ) )
    {
        return BERR_NOT_SUPPORTED; /* request only supported on Zeus 4.2 BFW 4.0.0 and up */
    }

    BKNI_Memset( pStatus, 0, sizeof(*pStatus) );

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );  /* Failed to create a BSP message instance. */
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eREAD_EXCEPTION_STATUS, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_ReadExceptionStatus_InCmdField_eKeepStatus, pRequest->keepStatus ? BCMD_ExceptionStatus_KeepStatus_eDoNotClear :
                                                                                                     BCMD_ExceptionStatus_KeepStatus_eClearRegs );
    BHSM_BspMsg_Pack8( hMsg, BCMD_ReadExceptionStatus_InCmdField_eSubCommand, BCMD_ExceptionStatus_Operation_eCaptureReg );
    BHSM_BspMsg_Pack8( hMsg, BCMD_ReadExceptionStatus_InCmdField_eDevice, pRequest->deviceType );

    switch( pRequest->deviceType )
    {
        case BHSM_ExceptionStatusDevice_eMemcArch:
        {
            BHSM_BspMsg_Pack8( hMsg, BCMD_ReadExceptionStatus_InCmdField_eUnit,    pRequest->u.memArch.memcIndex );
            BHSM_BspMsg_Pack8( hMsg, BCMD_ReadExceptionStatus_InCmdField_eSubUnit, pRequest->u.memArch.archIndex );
            break;
        }
        default:
        {
            return BERR_TRACE( BERR_NOT_SUPPORTED );
        }
    }

    rc = BHSM_BspMsg_SubmitCommand ( hMsg );
    if( rc != BERR_SUCCESS ) {
        rc = BERR_TRACE( rc );  /* BSP communication failed.  */
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        BDBG_ERR(("%s BSP Status  [0x%X]", BSTD_FUNCTION, status ));
        rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR );  /* BSP command failed. */
        goto BHSM_P_DONE_LABEL;
    }

    /* parse the output depending on the device type  */
    switch( pRequest->deviceType )
    {
        case BHSM_ExceptionStatusDevice_eMemcArch:
        {
            uint8_t  numRegs = 0;
            uint32_t reg     = 0;
            uint32_t regLsb  = 0;   /* address bits[3..34]  are read into this variable */
            uint32_t regMsb  = 0;   /* address bits[35..39] are read into this variable ... 6 bits valid*/

            BHSM_BspMsg_Get8( hMsg, BCMD_ReadExceptionStatus_OutCmdField_eNumRegs, &numRegs );

            /* start address */
            BHSM_BspMsg_Get32( hMsg, BCMD_ReadExceptionStatus_OutCmdField_eRegister0+(4*0), &regLsb );
            BHSM_BspMsg_Get32( hMsg, BCMD_ReadExceptionStatus_OutCmdField_eRegister0+(4*1), &regMsb );
            pStatus->u.memArch.startAddress = ((uint64_t)(regMsb & 0x3F) << 35) + ((uint64_t)regLsb << 3);

            /* end address */
            BHSM_BspMsg_Get32( hMsg, BCMD_ReadExceptionStatus_OutCmdField_eRegister0+(4*2), &regLsb );
            BHSM_BspMsg_Get32( hMsg, BCMD_ReadExceptionStatus_OutCmdField_eRegister0+(4*3), &regMsb );
            pStatus->u.memArch.endAddress   = ((uint64_t)(regMsb & 0x3F) << 35) + ((uint64_t)regLsb << 3);

            /* control */
            BHSM_BspMsg_Get32( hMsg, BCMD_ReadExceptionStatus_OutCmdField_eRegister0+(4*4), &reg );
            pStatus->u.memArch.scbClientId = ((reg >> 24) & 0xFF ); /* bits[31:24]  */
            pStatus->u.memArch.numBlocks   = ((reg >> 12) & 0x3FF); /* bits[21:12]  */
            pStatus->u.memArch.requestType = ( reg        & 0x1FF); /* bits[8:0]    */

            break;
        }
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_GetExceptionStatus );
    return rc;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pRequest );
    BSTD_UNUSED( pStatus );
    return BERR_NOT_SUPPORTED;
#endif
}
