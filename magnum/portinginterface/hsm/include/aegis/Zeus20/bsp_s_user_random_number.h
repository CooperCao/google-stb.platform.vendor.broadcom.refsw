/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/


#ifndef BSP_S_USER_RANDOM_NUMBER_H__
#define BSP_S_USER_RANDOM_NUMBER_H__

#include "bsp_s_commands.h"


#define RANDOM_NUMBER_OUTPUT_TO_HOST                 0

#define BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_RNGRAW        0
#define BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_RNGSHA        1
#define BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_MAX           2



enum BCMD_UserRandomNumber_CmdInputField_e
{
    BCMD_UserRandomNumber_CmdInputField_eRandomNumberType   = (BCMD_CommonBufferFields_eParamLen + 5),
    BCMD_UserRandomNumber_CmdInputField_eDestination        = (BCMD_UserRandomNumber_CmdInputField_eRandomNumberType + 4),
    BCMD_UserRandomNumber_CmdInputField_eRandomNumberLength = (BCMD_UserRandomNumber_CmdInputField_eDestination + 3)
};

enum BCMD_UserRandomNumber_CmdOutputField_e
{
    BCMD_UserRandomNumber_CmdOutputField_eStatus            = (BCMD_CommonBufferFields_eParamLen + 5),
    BCMD_UserRandomNumber_CmdOutputField_eRandomNumber      = (BCMD_UserRandomNumber_CmdOutputField_eStatus + 1)
};

#endif
