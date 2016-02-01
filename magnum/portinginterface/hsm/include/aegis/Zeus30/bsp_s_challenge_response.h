/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/


#ifndef BSP_S_CHALLENGE_RESPONSE_H__
#define BSP_S_CHALLENGE_RESPONSE_H__


typedef enum BCMD_RebootWithEJTAGDebug_InCmdField_e
{
    BCMD_RebootWithEJTAGDebug_InCmdField_eReserved  = (5<<2) + 3,
    BCMD_RebootWithEJTAGDebug_InCmdField_eMax
} BCMD_RebootWithEJTAGDebug_InCmdField_e;

typedef enum BCMD_RebootWithEJTAGDebug_OutCmdField_e
{
    BCMD_RebootWithEJTAGDebug_OutCmdField_eStatus   = (5<<2) + 3,
    BCMD_RebootWithEJTAGDebug_OutCmdField_eMax
} BCMD_RebootWithEJTAGDebug_OutCmdField_e;

#endif
