/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Porting interface code for the secondary PID channel. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BXPT_SPID_H__
#define BXPT_SPID_H__

#include "bxpt.h"

#ifdef __cplusplus
extern "C"{
#endif

/*= Module Overview *********************************************************

Overview

Usage / Sample Code

%CODE{"cpp"}%
%ENDCODE%
***************************************************************************/

/***************************************************************************
Summary:
Settings done on a per-SPID channel basis. Each secondary PID channel can
perform one of these actions on its PIDs. The reserved values should not 
be used. 
****************************************************************************/
typedef enum BXPT_Spid_eChannelMode
{
	BXPT_Spid_eChannelMode_Disable = 0,			/* Disable this secondary PID table entry */
	BXPT_Spid_eChannelMode_Reserved1 = 1,		/* Reserved. Do not use. */
	BXPT_Spid_eChannelMode_Reserved2 = 2,		/* Reserved. Do not use. */
	BXPT_Spid_eChannelMode_Reserved3 = 3,		/* Reserved. Do not use. */
	BXPT_Spid_eChannelMode_Reserved4 = 4,		/* Reserved. Do not use. */
	BXPT_Spid_eChannelMode_Reserved5 = 5,		/* Reserved. Do not use. */
	BXPT_Spid_eChannelMode_Merge = 6,			/* Merge PIDs from primary and secondary on this channel */
	BXPT_Spid_eChannelMode_Remap = 7			/* Remap PIDs from secondary to primary on this channel. */
}
BXPT_Spid_eChannelMode;

/***************************************************************************
Summary:
Set the configuration for the given secondary PID channel.

Description:
Set the secondary PID and the SPID mode for the given channel. 

Returns:
    BERR_SUCCESS                - Configuration set.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
****************************************************************************/
BERR_Code BXPT_Spid_ConfigureChannel(
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	unsigned int ChannelNum, 	/* [in] Which secondary channel to configure. */
	unsigned int Spid, 				/* [in] Secondary PID to use. */
	BXPT_Spid_eChannelMode Mode 	/* [in] Mode for secondary PIDs on this channel. */
	);

#ifdef __cplusplus
}
#endif

#endif

