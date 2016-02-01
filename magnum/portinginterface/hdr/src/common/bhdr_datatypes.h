/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
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
 ***************************************************************************/
 
#ifndef BHDR_DATATYPES_H__
#define BHDR_DATATYPES_H__



/******************************************************************************
Summary:
HDMI Rx Context Handle
*******************************************************************************/
typedef struct BHDR_P_Handle *BHDR_Handle ;


/******************************************************************************
Summary:
HDMI Front End Context Handle
*******************************************************************************/
typedef struct BHDR_FE_P_Handle *BHDR_FE_Handle ;


/******************************************************************************
Summary:
HDMI Front End Channel Context Handle
*******************************************************************************/
typedef struct BHDR_FE_P_ChannelHandle *BHDR_FE_ChannelHandle ;


/******************************************************************************
Summary:
HDMI Rx States for Syncing Video
*******************************************************************************/
typedef enum 
{
	BHDR_P_VideoInput_eSyncStart,
	BHDR_P_VideoInput_eWaitForDeviceConnect,
	BHDR_P_VideoInput_eWaitForPllLock,	
	BHDR_P_VideoInput_eWaitForKickStartComplete,
	BHDR_P_VideoInput_eWaitForStableFormat,
	BHDR_P_VideoInput_eReady,
	BHDR_P_VideoInput_eDone	
}  BHDR_P_VideoInputStates ;





#endif
