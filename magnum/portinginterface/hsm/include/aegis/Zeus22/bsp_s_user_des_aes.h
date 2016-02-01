/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/


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
