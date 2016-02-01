/***************************************************************************
*     Copyright (c) 2005, Broadcom Corporation
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
* Module Description: This module contains APIs related to PVR trick modes
*                     of Raptor audio decoder.
*
* Revision History:
* $brcm_Log: $
* 
***************************************************************************/

#ifndef BRAP_PVR_H__
#define BRAP_PVR_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary: 
	Pause/resume audio

Description: 
	This function pauses/resumes audio. When unpaused (resumed), audio
	starts from the point it paused. This function works only for playback.

Returns:
	BERR_SUCCESS - If pause/resume operation is successful.

See Also: 
	BRAP_PVR_GetPauseStatus
****************************************************************************/
BERR_Code BRAP_PVR_EnablePause(
	BRAP_ChannelHandle		hRapCh, /* [In] The RAP Channel handle */
	bool					bOnOff	/* [In] TRUE = Pause video
											FALSE = Resume video */
	);


/***************************************************************************
Summary: 
	Get Pause/resume audio status

Description: 
	This function gets the current pause/resume status of the audio. When 
	audio is paused it returns TRUE and when audio is playing it returns 
	FALSE.

Returns:
	BERR_SUCCESS - If pause/resume information is retrieved successfully.

See Also: 
	BRAP_PVR_EnablePause
****************************************************************************/
BERR_Code BRAP_PVR_GetPauseStatus(
	BRAP_ChannelHandle		hRapCh,		/* [In] The RAP Channel handle */
	bool					*pbPauseStatus /* [Out]
										   1 = Video paused
										   0 = Video playing
										   */
	);

/***************************************************************************
Summary: 
	Frame advance trick mode

Description: 
	This function programs audio decoder to perform frame advance trick 
	mode. During frame advance trick mode,audio is muted at the output port.
	Parameter uiFrameAdvTime tells audio decoder how many frames to advance
	in terms of time (in millisecond). 
	This function works only for playback.

Returns:
	BERR_SUCCESS - If frame advance mode is programmed successfully.

See Also:
****************************************************************************/
BERR_Code BRAP_PVR_FrameAdvance (
	BRAP_ChannelHandle		hRapCh,			/* [In] The RAP Channel handle */
	unsigned int			uiFrameAdvTime	/* [In] Frame advance time in msec */
	);

#ifdef __cplusplus
}
#endif

#endif /* BRAP_PVR_H__ */
/* End of File */

