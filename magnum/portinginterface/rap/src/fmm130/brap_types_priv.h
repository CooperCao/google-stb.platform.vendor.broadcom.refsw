/***************************************************************************
*     Copyright (c) 2004-2006, Broadcom Corporation
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
* Module Description: Private data structures common to all the modules of
*					  Raptor PI.
* Revision History:
* $brcm_Log: $
* 
***************************************************************************/
#ifndef _BRAP_TYPES_PRIV_H__ /*{{{*/
#define _BRAP_TYPES_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Supported Audio Channels.

Description:
	This enum defines the different types of supported Audio Channels.
	
See Also:
	
****************************************************************************/
typedef enum BRAP_P_ChannelType
{
	BRAP_P_ChannelType_eDecode = 0,/* Live Decode Channel:
								   The input comes from the transport module 
								   as compressed stream. This channel can do 
								   decode and/or pass-through or SRC on the 
								   input stream */
	BRAP_P_ChannelType_ePcmPlayback,/* Playback Channel:
								   The input is PCM audio coming directly 
								   from secondary storage device. */
	BRAP_P_ChannelType_eCapture,/* Capture Channel:
								   The input is PCM data coming from any one 
								   of the capture input. This data is 
								   used for capturing. */
#ifndef BCHP_7411_VER /* For chips other than 7411 */
/* Encoder support in RM */
	BRAP_P_ChannelType_eEncode, /* Encode channel, data can come either RT or NRT */
#endif
	BRAP_P_ChannelType_eMax     /* This can be used both as invalid or max 
	                               number of channel types */
}BRAP_P_ChannelType;

#ifdef __cplusplus
}
#endif

#endif /*}}} #ifndef _BRAP_TYPES_PRIV_H__ */

/* End of File */
