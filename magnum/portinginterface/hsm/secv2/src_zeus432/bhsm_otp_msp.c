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
#include "bhsm_bsp_msg.h"
#include "bhsm_otp_msp.h"
#include "bsp_s_otp_common.h"
#include "bsp_s_otp.h"
#include "bhsm_otp_priv.h"

BDBG_MODULE( BHSM );

BERR_Code BHSM_OtpMsp_Write( BHSM_Handle hHsm, BHSM_OtpMspWrite *pParam )
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    uint8_t         dataBitLen = 0;
    uint32_t        bit = 1;
    unsigned char   status = ~0;
    BHSM_BspMsgHeader_t header;

    BDBG_ENTER( BHSM_OtpMsp_Program );

    if( !pParam ) { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( pParam->index >= BCMD_Otp_CmdMsp_eSize ) { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }

    for (bit = 0; bit < 32; bit++)
    {
        if ((pParam->mask & (1 << bit)))
        {
            dataBitLen = bit + 1;
        }
    }

    if( ( rc = BHSM_OTPPatternSequence_Program_priv( hHsm, false ) ) != BERR_SUCCESS ) {
        /* In case historic BUG in ROM code ... may need to send OTP programming pattern twice */
        if( ( rc = BHSM_OTPPatternSequence_Program_priv( hHsm, false ) ) != BERR_SUCCESS ) {
            return BERR_TRACE( rc );
        }
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOFFLINE_PROG_MSP, &header );

    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOfflineProgMsp_eMode32, BCMD_OTP_DATASECTIONPROG_MODE );
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack16( hMsg, BCMD_Otp_InCmdOfflineProgMsp_eEnum, pParam->index );
#else
    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOfflineProgMsp_eEnum, pParam->index );
#endif
    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOfflineProgMsp_eNumOfBits, dataBitLen );
    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOfflineProgMsp_eMask, pParam->mask );
    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOfflineProgMsp_eData, pParam->data );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        BDBG_ERR( ( "%s status[0x%02X]", BSTD_FUNCTION, status ) );
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

  BHSM_P_DONE_LABEL:

    ( void ) BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_OtpMsp_Program );

    return rc;
}

BERR_Code BHSM_OtpMsp_Read( BHSM_Handle hHsm, BHSM_OtpMspRead *pParam )
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    unsigned char   status = ~0;
    BHSM_BspMsgHeader_t header;

    BDBG_ENTER( BHSM_OtpMsp_Read );

    if( pParam == NULL )                         { return BERR_TRACE( BHSM_STATUS_FAILED ); }
    if( pParam->index >= BCMD_Otp_CmdMsp_eSize ) { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOFFLINE_MSP_READ, &header );

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack16( hMsg, BCMD_Otp_InCmdOfflineMspRead_eEnum, pParam->index );
#else
    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOfflineMspRead_eEnum, pParam->index );
#endif
    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status ); /* check status */
    if( status != 0 ) {
        BDBG_ERR( ( "%s enum[%d] status[0x%X]", BSTD_FUNCTION, pParam->index, status ) );
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get32( hMsg, BCMD_Otp_OutCmdOfflineMspRead_eMspValue, &pParam->data );
    BHSM_BspMsg_Get32( hMsg, BCMD_Otp_OutCmdOfflineMspRead_eMspLockValue, &pParam->valid );

  BHSM_P_DONE_LABEL:

    ( void ) BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_OtpMsp_Read );
    return rc;
}



BERR_Code BHSM_OtpMsp_ReadRange( BHSM_Handle hHsm, BHSM_OtpMspReadRange *pParam )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    uint8_t         status = 0;
    uint16_t        length = 0;
    unsigned        i;
    BHSM_BspMsgHeader_t header;

    BDBG_ENTER( BHSM_OtpMsp_ReadRange );

    if( pParam == NULL ) { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( pParam->startIndex >= BCMD_Otp_CmdMsp_eSize ) { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( pParam->startIndex + pParam->numMsp >= BCMD_Otp_CmdMsp_eSize ) { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOTP_ReadMSP32, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOtpReadMSP32_eMspGroupType,  pParam->readLock?1:0 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOtpReadMSP32_eStartGroup, pParam->startIndex );
    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOtpReadMSP32_eRange, pParam->numMsp );

    if ((rc = BHSM_BspMsg_SubmitCommand (hMsg)) != BERR_SUCCESS)
    {
        BERR_TRACE (rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 (hMsg, BCMD_CommonBufferFields_eStatus, &status);  /* check status */
    if (status != 0)
    {
        BDBG_ERR(("%s status[0x%X]", BSTD_FUNCTION, status ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get16( hMsg, BCMD_CommonBufferFields_eParamLen, &length );   /* read howmany words to read */

    length /= 4; /* Convert to num words */
    length--;    /* Account for status word in output data. */

    if( (unsigned)length != pParam->numMsp ) {
        rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR );
        goto BHSM_P_DONE_LABEL;
    }

    for( i = 0; i < (unsigned)length; i++ )
    {
        uint32_t bspMsp = 0;

        BHSM_BspMsg_Get32( hMsg, BCMD_Otp_OutCmdOtpReadMSP32_eMspValueStartAddr+(i*4), &bspMsp );

        /* Byte reverse (endian-swap) the returned 32-bit value */
        pParam->pMem[i]  = ( bspMsp & 0x000000ff ) << 24;
        pParam->pMem[i] |= ( bspMsp & 0x0000ff00 ) << 8;
        pParam->pMem[i] |= ( bspMsp & 0x00ff0000 ) >> 8;
        pParam->pMem[i] |= ( bspMsp & 0xff000000 ) >> 24;
    }

BHSM_P_DONE_LABEL:

    (void) BHSM_BspMsg_Destroy (hMsg);

    BDBG_LEAVE( BHSM_OtpMsp_ReadRange );
    return rc;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pParam );
    return BERR_TRACE( BERR_NOT_SUPPORTED );
#endif
}
