/***************************************************************************
*     Copyright (c) 2005-2008, Broadcom Corporation
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

#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE(rap_pvr);		/* Register software module with debug interface */


/*******************************************************
 * Local defines
 *******************************************************/
 /*******************************************************
 * Local Type definitions
 *******************************************************/
/*******************************************************
 * Static Variables & Functions
 *******************************************************/


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
	)
{
	BERR_Code err;

	BDBG_ENTER(BRAP_PVR_EnablePause);
	/* Assert the function arguments*/
	BDBG_ASSERT(hRapCh);

    BDBG_MSG(("BRAP_PVR_EnablePause: bOnOff=%d", bOnOff));

	/* If audio is already in required state, return success */
	if ((hRapCh->sTrickModeState.bAudioPaused==bOnOff)
		&& (!(BRAP_P_GetInternalCallFlag(hRapCh))))
		return BERR_SUCCESS;

	/*Check is Hardware is reset */
	if(hRapCh->hRap->bResetHwDone==false)
	{
		/* Pause the DSP */
		err = BRAP_DSPCHN_P_EnablePause(hRapCh->sModuleHandles.hDspCh, bOnOff);
		if (err!=BERR_SUCCESS)
			return BERR_TRACE(err);
	}

	if (bOnOff==true) 
	{
		/* Change "audio paused" state to "true" */
		hRapCh->sTrickModeState.bAudioPaused = true;

	}
	else if (bOnOff==false) 
	{
		/* Clear frame residual time on resume */
		hRapCh->sTrickModeState.uiFrameAdvResidualTime = 0;
		/* Change "audio paused" state to "false" */
		hRapCh->sTrickModeState.bAudioPaused = false;
	}			
	

	BDBG_LEAVE(BRAP_PVR_EnablePause);
	return BERR_SUCCESS;
}

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
										   1 = Audio paused
										   0 = Audio playing
										   */
	)
{
	BDBG_ENTER(BRAP_PVR_GetPauseStatus);
	/* Assert the function arguments*/
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(pbPauseStatus);

	/* Return the current pause state for this channel */
	*pbPauseStatus = hRapCh->sTrickModeState.bAudioPaused;


    BDBG_MSG(("BRAP_PVR_GetPauseStatus: bPauseStatus=%d", *pbPauseStatus));

	BDBG_LEAVE(BRAP_PVR_GetPauseStatus);
	return BERR_SUCCESS;
}

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
	)
{
	BERR_Code err;
	unsigned int uiNumFrames, uiOneFrameTime;
	bool bDecoderPaused;

	BDBG_ENTER(BRAP_PVR_FrameAdvance);
	/* Assert the function arguments*/
	BDBG_ASSERT(hRapCh);

	/* Return error if audio is not paused */
	if (!hRapCh->sTrickModeState.bAudioPaused) {
		BDBG_ERR(("For Frame Advance audio should be in paused state"));
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}

	/* Check if decoder got into "paused" state, i.e. check whether decoder got atleast
	 * one frame of data for calculation of TSM upper threshold. TSM upper threshold
	 * is required for calculating number of frames to be frame advanced. If decoder
	 * is not in "paused" state, then just return. */

	bDecoderPaused = BRAP_DSPCHN_P_GetDecoderPauseState(hRapCh->sModuleHandles.hDspCh);
	if (!bDecoderPaused)
		return BERR_SUCCESS;

	/* Get one audio frame time required for number of frames calculations */
	err = BRAP_DSPCHN_P_GetOneAudioFrameTime(hRapCh->sModuleHandles.hDspCh, &uiOneFrameTime);
	if (err!=BERR_SUCCESS)
		return BERR_TRACE(err);

	/* Translate frame advance time into number of frames to be advanced */
	uiNumFrames = (hRapCh->sTrickModeState.uiFrameAdvResidualTime + uiFrameAdvTime)/uiOneFrameTime;
	hRapCh->sTrickModeState.uiFrameAdvResidualTime = 
		(hRapCh->sTrickModeState.uiFrameAdvResidualTime + uiFrameAdvTime) % uiOneFrameTime;
    BDBG_MSG(("uiFrameAdvTime (from app)=%d, uiOneFrameTime (from register)=%d, uiNumFrames=%d", 
            uiFrameAdvTime, uiOneFrameTime, uiNumFrames));

    /* If number of frames > 256, force it to 256. Ignore the remaining */
	if (uiNumFrames > 255)
	{
        uiNumFrames = 255;
        BDBG_MSG(("uiNumFrames=%d", uiNumFrames));
    }
	/*Check is Hardware is reset */
	if(hRapCh->hRap->bResetHwDone==false)
	{
		err = BRAP_DSPCHN_P_FrameAdvance(hRapCh->sModuleHandles.hDspCh, uiNumFrames);
		if (err!=BERR_SUCCESS)
			return BERR_TRACE(err);
	}
	
	BDBG_LEAVE(BRAP_PVR_FrameAdvance);
	return BERR_SUCCESS;
}

/* End of File */

