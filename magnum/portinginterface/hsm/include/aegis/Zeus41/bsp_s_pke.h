/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/


#ifndef BSP_S_PKE_H__
#define BSP_S_PKE_H__


typedef enum BCMD_PKE_CmdPollInCmdField_e
{
    BCMD_PollingCommand_CmdInputField_ePollingTarget = (5<<2) + 3,
    BCMD_PollingCommand_CmdInputField_eMax
} BCMD_PKE_CmdPollInCmdField_e;


typedef enum BCMD_PollingCommand_PollingTarget_e
{
        BCMD_PollingCommand_PollingTarget_eSecureRSA = 0,
        BCMD_PollingCommand_PollingTarget_eUserRSA = 1,
        BCMD_PollingCommand_PollingTarget_eMax
} BCMD_PollingCommand_PollingTarget_e;

#endif
