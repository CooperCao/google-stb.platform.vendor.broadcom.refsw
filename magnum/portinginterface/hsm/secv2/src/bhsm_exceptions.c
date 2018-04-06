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

#include "bhsm_exceptions.h"
#include "bhsm_exceptions_priv.h"
#include "bhsm_priv.h"
#include "bhsm_p_control.h"
#include "bsp_types.h"

BDBG_MODULE( BHSM );

BERR_Code  BHSM_Exception_GetMemcArch( BHSM_Handle hHsm, BHSM_ExceptionMemcArch_t *pException )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_ReadCaptureRegistersRequest command;

    BDBG_ENTER( BHSM_Exception_GetMemcArch );

    if( !hHsm ) { return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( !pException ) { return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }

    BKNI_Memset( &command, 0, sizeof(command) );

    command.device = BCMD_ExceptionCaptureRegRead_Device_eMemcARCH;
    command.keepStatus = pException->keepStatus;
    command.unit = pException->memcIndex;
    command.subUnit = pException->archIndex;

    rc = BHSM_ReadCaptureRegisters_priv( hHsm, &command );
    if( rc != BERR_SUCCESS ) {  return BERR_TRACE( rc ); }

    pException->startAddress = ((uint64_t)(command.registers[1] << 3) + ((uint64_t)command.registers[3] << 35));
    pException->endAddress  =  ((uint64_t)(command.registers[5] << 3) + ((uint64_t)command.registers[7] << 35));
    pException->scbClientId =  (( command.registers[9] >> 24)   & 0xFF ); /*  8 bits [31:24]  */
    pException->numBlocks   =  (( command.registers[9] >> 12)   & 0x3FF); /* 10 bits [21:12]  */
    pException->requestType =   ( command.registers[9]          & 0x1FF); /*  9 bits [8:0]    */

    BDBG_LEAVE( BHSM_Exception_GetMemcArch );
    return BERR_SUCCESS;
}


BERR_Code BHSM_ReadCaptureRegisters_priv( BHSM_Handle hHsm, BHSM_ReadCaptureRegistersRequest *pRequest )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_P_ControlExceptionCaptureRegRead capRegsCmd;
    uint8_t  numEntries = 0;
    unsigned i = 0;

    BDBG_ENTER(BHSM_ReadCaptureRegisters_priv );

    if( !hHsm ) { return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( !pRequest ) { return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }

    BKNI_Memset( &capRegsCmd, 0, sizeof(capRegsCmd) );

    capRegsCmd.in.keepStatus = pRequest->keepStatus ? BCMD_ExceptionCaptureRegRead_KeepStatus_eDoNotClear :
                                                      BCMD_ExceptionCaptureRegRead_KeepStatus_eClearRegs;
    capRegsCmd.in.device = pRequest->device;
    capRegsCmd.in.unit = pRequest->unit;
    capRegsCmd.in.subUnit = pRequest->subUnit;

    rc = BHSM_P_Control_ExceptionCaptureRegRead( hHsm, &capRegsCmd );
    if( rc != BERR_SUCCESS ) {  return BERR_TRACE( rc ); }

    numEntries = capRegsCmd.out.numRegs;

    if( numEntries > BHSM_P_NUM_DATA_REGISTERS ) { return BERR_TRACE( BHSM_STATUS_BSP_ERROR ); }

    pRequest->numEntries = numEntries;

    for( i = 0; i < numEntries; i++ ) {
        pRequest->registers[i] = capRegsCmd.out.registerX[i];
    }

    BDBG_LEAVE(BHSM_ReadCaptureRegisters_priv );
    return BERR_SUCCESS;
}
