/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/


#ifndef BSP_S_S2R_COMMANDS_H__
#define BSP_S_S2R_COMMANDS_H__

#define BCMD_S2R_COMMAND_PM_INITIATE_MASK       0x000000FEUL
#define BCMD_S2R_COMMAND_PM_INITIATE_SHIFT      1

#define BCMD_S2R_COMMAND_PM_INITIATE_SUCCESS    0UL
#define BCMD_S2R_COMMAND_PM_INITIATE_FAILED     0x000000FEUL

typedef enum BCMD_S2R_cmdType_e
{
    BCMD_S2R_cmdType_eClockStop_Initiated = 0x00,
    BCMD_S2R_cmdType_eGenerateLoadRandomS2RMACkey = 0x4A,
    BCMD_S2R_cmdType_eReloadRandomS2RMACkey = 0x55,
    BCMD_S2R_cmdType_eLoadFixedS2RMACkey = 0x63,

    BCMD_S2R_cmdType_eMax
} BCMD_S2R_cmdType_e;

#endif
