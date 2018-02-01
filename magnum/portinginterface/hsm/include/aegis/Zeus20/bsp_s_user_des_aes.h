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


#ifndef BSP_S_USER_DES_AES_H__
#define BSP_S_USER_DES_AES_H__

#include "bsp_s_commands.h"


#define BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_ONE_DES_ECB           0x00
#define BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_THREE_DES_ABA_ECB     0x01
#define BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_AES_128_ECB           0x02
#define BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_THREE_DES_ABC_ECB     0x03

#define KEY_INPUT_PARAMETER_SIZE                    (192/8)

enum BCMD_UserDESAES_CmdInputField_e
{
    BCMD_UserDESAES_CmdInputField_eKey                 = (BCMD_CommonBufferFields_eParamLen + 2),
    BCMD_UserDESAES_CmdInputField_eAlgorithmSel        = (BCMD_UserDESAES_CmdInputField_eKey + KEY_INPUT_PARAMETER_SIZE + 3),
    BCMD_UserDESAES_CmdInputField_eOperationMode       = (BCMD_UserDESAES_CmdInputField_eAlgorithmSel + 4),
    BCMD_UserDESAES_CmdInputField_eDataLength          = (BCMD_UserDESAES_CmdInputField_eOperationMode + 3),
    BCMD_UserDESAES_CmdInputField_eDataLengthPlus1     = (BCMD_UserDESAES_CmdInputField_eDataLength + 1),
    BCMD_UserDESAES_CmdInputField_eData                = (BCMD_UserDESAES_CmdInputField_eDataLength + 2)
};

#define BCMD_USER_DES_AES_INPUT_DATA_MAX_LENGTH     (BCMD_BUFFER_BYTE_SIZE - BCMD_UserDESAES_CmdInputField_eData)

enum BCMD_UserDESAES_CmdOutputField_e
{
    BCMD_UserDESAES_CmdOutputField_eStatus             = (BCMD_CommonBufferFields_eParamLen + 5),
    BCMD_UserDESAES_CmdOutputField_eDataLength         = (BCMD_UserDESAES_CmdOutputField_eStatus + 3),
    BCMD_UserDESAES_CmdOutputField_eDataLengthPlus1    = (BCMD_UserDESAES_CmdOutputField_eDataLength + 1),
    BCMD_UserDESAES_CmdOutputField_eData               = (BCMD_UserDESAES_CmdOutputField_eDataLength + 2)
};

#endif
