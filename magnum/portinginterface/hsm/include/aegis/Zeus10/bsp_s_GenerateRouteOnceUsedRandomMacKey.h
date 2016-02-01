/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/

#ifndef BSP_S_GenerateRouteOnceUsedRandomMacKey_H__
#define BSP_S_GenerateRouteOnceUsedRandomMacKey_H__

typedef enum BCMD_GenerateRouteOnceUsedRandomMacKey_InCmdField_e
{
    BCMD_GenerateRouteOnceUsedRandomMacKey_InCmd_eOtpID              	= (5<<2) + 3,
    BCMD_GenerateRouteOnceUsedRandomMacKey_InCmd_eRngGenFlag			= (6<<2) + 3,
    BCMD_GenerateRouteOnceUsedRandomMacKey_InCmd_eM2MSlotNum          = (7<<2) + 3,
    BCMD_GenerateRouteOnceUsedRandomMacKey_InCmd_eMax
}BCMD_GenerateRouteOnceUsedRandomMacKey_InCmdField_e;

typedef enum BCMD_RngGenFlag_e
{
    BCMD_RngGenFlag_eNoGen			= 0,
    BCMD_RngGenFlag_eGen			= 1,
    BCMD_RngGenFlag_eMax
} BCMD_RngGenFlag_e;


#endif
