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

#include "bhsm_exceptions.h"
#include "bhsm_priv.h"
#include "bhsm_bsp_msg.h"
#include "bsp_s_commands.h"
#include "bsp_s_misc.h"
#include "bhsm_exceptions_priv.h"

BDBG_MODULE( BHSM );

BERR_Code  BHSM_Exception_GetMemcArch( BHSM_Handle hHsm, BHSM_ExceptionMemcArch_t *pException )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_ReadCaptureRegistersRequest command;

    BDBG_ENTER( BHSM_Exception_GetMemcArch );

    if( !hHsm ) { return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( !pException ) { return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }

    BKNI_Memset( &command, 0, sizeof(command) );

    command.device = BCMD_ExceptionStatus_Device_eMemcARCH;
    command.keepStatus = pException->keepStatus;
    command.unit = pException->memcIndex;
    command.subUnit = pException->archIndex;

    rc = BHSM_ReadCaptureRegisters_priv( hHsm, &command );
    if( rc != BERR_SUCCESS ) {  return BERR_TRACE( rc ); }

    pException->startAddress = ((uint64_t)(command.registers[1] & 0x3F) << 35) + ((uint64_t)command.registers[0] << 3);
    pException->endAddress  =  ((uint64_t)(command.registers[3] & 0x3F) << 35) + ((uint64_t)command.registers[2] << 3);
    pException->scbClientId =  (( command.registers[4] >> 24)   & 0xFF ); /*  8 bits [31:24]  */
    pException->numBlocks   =  (( command.registers[4] >> 12)   & 0x3FF); /* 10 bits [21:12]  */
    pException->requestType =   ( command.registers[4]          & 0x1FF); /*  9 bits [8:0]    */

    BDBG_LEAVE( BHSM_Exception_GetMemcArch );
    return BERR_SUCCESS;
}

/* reads the Capture registers from the BFW. */
BERR_Code BHSM_ReadCaptureRegisters_priv( BHSM_Handle hHsm, BHSM_ReadCaptureRegistersRequest *pRequest )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status = BHSM_P_BSP_INVALID_STATUS;
    uint8_t  numEntries = 0;
    unsigned i = 0;

    BDBG_ENTER( BHSM_ReadCaptureRegisters_priv );

    if( !hHsm ) { return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( !pRequest ) { return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }

    if( (BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(4,2)) &&
        !isBfwVersion_GreaterOrEqual( hHsm, 4,0,0 ) ) {
        return  BERR_TRACE( BERR_NOT_SUPPORTED ); /* request only supported on Zeus 4.2 BFW 4.0.0 and up */
    }

    rc = BHSM_BspMsg_Create( hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eREAD_EXCEPTION_STATUS, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_ReadExceptionStatus_InCmdField_eKeepStatus, pRequest->keepStatus ? BCMD_ExceptionStatus_KeepStatus_eDoNotClear :
                                                                              BCMD_ExceptionStatus_KeepStatus_eClearRegs );
    BHSM_BspMsg_Pack8( hMsg, BCMD_ReadExceptionStatus_InCmdField_eSubCommand, BCMD_ExceptionStatus_Operation_eCaptureReg );
    BHSM_BspMsg_Pack8( hMsg, BCMD_ReadExceptionStatus_InCmdField_eDevice,     pRequest->device );
    BHSM_BspMsg_Pack8( hMsg, BCMD_ReadExceptionStatus_InCmdField_eUnit,       (uint8_t)pRequest->unit );
    BHSM_BspMsg_Pack8( hMsg, BCMD_ReadExceptionStatus_InCmdField_eSubUnit,    (uint8_t)pRequest->subUnit );

    rc = BHSM_BspMsg_SubmitCommand ( hMsg );
    if( rc != BERR_SUCCESS ) {  BERR_TRACE( rc ); goto exit; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        BDBG_ERR(("%s BSP status[0x%02X] error", BSTD_FUNCTION, status ));
        rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR );  /* BSP command failed. */
        goto exit;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_ReadExceptionStatus_OutCmdField_eNumRegs, &numEntries );

    if( numEntries > BHSM_P_NUM_DATA_REGISTERS ) { rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR ); goto exit; }

    pRequest->numEntries = numEntries;

    for( i = 0; i < numEntries; i++ ) {
        BHSM_BspMsg_Get32( hMsg, BCMD_ReadExceptionStatus_OutCmdField_eRegister0+(4*i), &pRequest->registers[i] );
    }

exit:
    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_ReadCaptureRegisters_priv );
    return rc;
}
