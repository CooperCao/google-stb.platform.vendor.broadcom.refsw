/***************************************************************************
 *     Copyright (c) 2004-2012, Broadcom Corporation
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
 * Module Description:
 *   See Module Overview below
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ****************************************************************************/
#ifndef BXVD_PVR_H__
#define BXVD_PVR_H__

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif


/***************************************************************************
Summary:
	Frame advance mode enum

Description:
	Enum to specify whether frame advance is frame by frame or field by field

See Also:
	BXVD_PVR_FrameAdvance
****************************************************************************/
typedef enum BXVD_PVR_FrameAdvanceMode
{
   BXVD_PVR_FrameAdvanceMode_eFrame_by_Frame, /* Frame by frame advance. Only 1st
                                                 field of each frame is displayed */
   BXVD_PVR_FrameAdvanceMode_eField_by_Field  /* Field by field advance. Only
                                                 one field is advanced */
} BXVD_PVR_FrameAdvanceMode;

/***************************************************************************
Summary:
	BTP Mode Type

Description:
	BTP modes can be either implemented as BTP type (Transport) or
	as BUD type (User Data) trick modes.

Note:
	Currently only BXVD_PVR_BTPMode_eOff is used. This is the value that
	BXVD_OpenChannel and BXVD_StopDecode sets the channel's state to.

 This enum is deprecated and may be removed in a future revision of PVR. Do
 not use this in new code.

See Also:
	BXVD_PVR_EnableBTPMode
****************************************************************************/
typedef enum BXVD_PVR_BTPMode
{
   BXVD_PVR_BTPMode_eOff, /* Trick modes are off */
   BXVD_PVR_BTPMode_eBTP, /* Trick mode info travels in the
                             Adaptation field of Transport Packets */
   BXVD_PVR_BTPMode_eBUD, /* Trick mode info travels in the
                             User Data with a special marker */
   BXVD_PVR_BTPMode_MaxMode
} BXVD_PVR_BTPMode;

/***************************************************************************
Summary:
	Pause/resume video

Description:
	This function pauses/resumes video. When unpaused (resumed), video
	starts from the point it paused. This function works only for playback.

Returns:
	BERR_SUCCESS - If pause/resume operation is successful.

See Also:
	BXVD_PVR_GetPauseStatus
****************************************************************************/
BERR_Code BXVD_PVR_EnablePause
(
   BXVD_ChannelHandle      hXvdCh,        /* [In] XVD channel handle */
   bool                    bEnablePause   /* [In] true=pause, false=resume */
   );

BERR_Code BXVD_PVR_EnablePause_isr
(
   BXVD_ChannelHandle      hXvdCh,        /* [In] XVD channel handle */
   bool                    bEnablePause   /* [In] true=pause, false=resume */
   );

/***************************************************************************
Summary:
	Gets pause status of video

Description:
	Returns true if paused, false otherwise
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetPauseStatus
(
   BXVD_ChannelHandle      hXvdCh,        /* [In] XVD channel handle */
   bool                   *pbPauseStatus /* [Out] video pause status */
   );

BERR_Code BXVD_PVR_GetPauseStatus_isr
(
   BXVD_ChannelHandle      hXvdCh,        /* [In] XVD channel handle */
   bool                   *pbPauseStatus /* [Out] video pause status */
   );
#endif

/***************************************************************************
SW7400-2870:
DEPRECATED: use BXVD_SetPlaybackRate(isr) instead

Summary:
	Set Slow motion rate

Description:
	This function programs video decoder to perform slow motion trick mode
	with slow motion rate that is passed to this function. If rate > 1,
	then slow motion with that rate. If rate = 1, then normal decode.
	This is a decoder based trick mode. This function works only for
	playback.

Returns:
	BERR_SUCCESS - If slow motion rate is programmed successfully.

See Also:
	BXVD_PVR_GetSlowMotionRate
****************************************************************************/
BERR_Code BXVD_PVR_SetSlowMotionRate
(
   BXVD_ChannelHandle      hXvdCh,      /* [In] XVD channel handle */
   unsigned long           ulRate       /* [In] slow motion rate */
   );

BERR_Code BXVD_PVR_SetSlowMotionRate_isr
(
   BXVD_ChannelHandle      hXvdCh,      /* [In] XVD channel handle */
   unsigned long           ulRate       /* [In] slow motion rate */
   );

/***************************************************************************
SW7400-2870
DEPRECATED: use BXVD_GetPlaybackRate(isr) instead
Summary:
	Get slow motion rate programmed

Description: This function returns the slow motion rate that is currently
             programmed in the video decoder.

Returns:
	BERR_SUCCESS - If opened XVD is successful.

See Also:
	BXVD_PVR_SetSlowMotionRate
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetSlowMotionRate
(
   BXVD_ChannelHandle       hXvdCh,   /* [In] XVD channel handle */
   unsigned long           *pSMRate   /* [Out] slow motion rate */
   );

BERR_Code BXVD_PVR_GetSlowMotionRate_isr
(
   BXVD_ChannelHandle       hXvdCh,   /* [In] XVD channel handle */
   unsigned long           *pSMRate   /* [Out] slow motion rate */
   );
#endif

/***************************************************************************
Summary:
	Set frame advance trick mode

Description:
	This function programs video decoder to perform frame advance trick
	mode. Parameter eFrameAdvMode that is passed to this function tells
	whether frame advance is achieved by advancing frames or fields.
	This is a decoder based trick mode. This function works only for
	playback.

Returns:
	BERR_SUCCESS - If frame advance mode is programmed successfully.

See Also:
****************************************************************************/
BERR_Code BXVD_PVR_FrameAdvance
(
   BXVD_ChannelHandle          hXvdCh,         /* [In] XVD channel handle */
   BXVD_PVR_FrameAdvanceMode   eFrameAdvMode  /* [In] Frame advance mode */
   );

BERR_Code BXVD_PVR_FrameAdvance_isr
(
   BXVD_ChannelHandle          hXvdCh,         /* [In] XVD channel handle */
   BXVD_PVR_FrameAdvanceMode   eFrameAdvMode  /* [In] Frame advance mode */
   );

/***************************************************************************
Summary:
	Enable/disable field reversal

Description:
	This function enables/disables field reversal. When enabled fields are
	displayed in the reverse order (bottom/top), this function has only effect
	if source has interlaced format. Usually field reversal is enabled in
	rewind type of trickmodes.

Returns:
	BERR_SUCCESS - If field reversal is enabled/disabled successfully.

See Also:
	BXVD_PVR_GetReverseFieldStatus
****************************************************************************/
BERR_Code BXVD_PVR_EnableReverseFields
(
   BXVD_ChannelHandle   hXvdCh,   /* [In] XVD channel handle */
   bool                 bEnable	  /* [In] flag to activate field reversal */
   );

BERR_Code BXVD_PVR_EnableReverseFields_isr
(
   BXVD_ChannelHandle   hXvdCh,  /* [In] XVD channel handle */
   bool                 bEnable  /* [In] flag to activate field reversal */
   );

/***************************************************************************
Summary:
	Get current status of field reversal

Description:
	This function returns current status of field reversal

Returns:
	BERR_SUCCESS - If status was returned successfully.

See Also:
	BXVD_PVR_GetReverseFieldStatus
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetReverseFieldStatus
(
   BXVD_ChannelHandle   hXvdCh,   /* [In] XVD channel handle */
   bool                *pbEnable  /* [Out] Status of field reversal */
   );

BERR_Code BXVD_PVR_GetReverseFieldStatus_isr
(
   BXVD_ChannelHandle   hXvdCh,  /* [In] XVD channel handle */
   bool                *pbEnable /* [Out] Status of field reversal */
   );
#endif

/***************************************************************************
Summary:
  Enable or disable host sparse mode.

Description:
  This function enables host sparse mode when bSparseMode is true or disables
  it when bSparseMode is false. For AVC this function is used to tell the
	 decoder it is in a host trick mode... it will have no effect on a non-H264
  stream. Calling this function on valid AVC stream will result in the decode
  stopping then restarting.

Returns:
	BERR_SUCCESS - If status was returned successfully.

See Also:
  BXVD_PVR_GetHostSparseMode
****************************************************************************/
BERR_Code BXVD_PVR_SetHostSparseMode
(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   bool bSparseMode           /* [In] Sparse mode enable flag */
   );

/***************************************************************************
Summary:
  Return the current host sparse mode setting.

Description:
  This function returns a boolean value indicating the current setting of
  the host sparse mode flag (true/false).

Returns:
	BERR_SUCCESS - If status was returned successfully.

See Also:
  BXVD_PVR_SetHostSparseMode
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetHostSparseMode
(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   bool *pbSparseMode         /* [Out] Current sparse mode status */
   );
#endif

/***************************************************************************
Summary:
  Enable or disable GOP reverse trick mode.

Description:
  This function sets the flag which enables/disables the DQT mode.
  In this mode, the DM displays the pictures within a GOP in reverse order.
  It takes effect when "BXVD_StartDecode()" is called.

  SW7445-2490: when DQT is enabled, the video display mode
  (BXVD_SetVideoDisplayMode) needs to either be set to eVirtualTSM or
  eTSM with the SW STC programmed to run in reverse (BXVD_SetClockOverride)

Returns:
	BERR_SUCCESS - If status was returned successfully.

See Also:
    BXVD_PVR_GetGopTrickMode
****************************************************************************/
BERR_Code BXVD_PVR_SetGopTrickMode
(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   bool bEnableTrickMode         /* [In] enable/disable DQT trickmode */
   );

BERR_Code BXVD_PVR_SetGopTrickMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   bool bEnableTrickMode         /* [In] enable/disable DQT trickmode */
   );

/***************************************************************************
Summary:
  Return the current setting of the DQT flag.

Description:
  This function returns a boolean value indicating the current setting of
  the DQT (Display Queue Trick) mode flag.


Returns:
	BERR_SUCCESS - If status was returned successfully.

See Also:
    BXVD_PVR_SetGopTrickMode
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetGopTrickMode
(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   bool * pbEnableTrickMode         /* [Out] enable/disable DQT trickmode */
   );

BERR_Code BXVD_PVR_GetGopTrickMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   bool * pbEnableTrickMode         /* [Out] enable/disable DQT trickmode */
   );
#endif

/***************************************************************************
Summary:
  Enable or disable automatic validation of the STC when transitioning from
  pause to play (AutoValidateStcOnPause).

Description:
  By default, the display manager will try to automatically determine if the
  STC is valid when transitioning from pause to play by waiting for "TSM pass" in
  processing stamp management.  AutoValidateStcOnPause will cause the display manager
  to pause indefinately waiting for TSM to pass, and may cause video to stop.

  Disabling this feature will cause the display manager to process TSM normally during
  the transition, but may also result in some pictures being discarded if the STC is
  not properly set or set in time.

  Making this call will have no effect when done during trick mode transition from pause to
  play.  The call must be made before playback is "unpaused".

Returns:
	BERR_SUCCESS - If status was returned successfully.

See Also:
  BXVD_PVR_GetAutoValidateStcOnPause
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_SetAutoValidateStcOnPause
(
   BXVD_ChannelHandle hXvdCh,     /* [In] XVD channel handle */
   bool bAutoValidateStcOnPause   /* [In] Enable/disable flag */
   );

BERR_Code BXVD_PVR_SetAutoValidateStcOnPause_isr
(
   BXVD_ChannelHandle hXvdCh,     /* [In] XVD channel handle */
   bool bAutoValidateStcOnPause   /* [In] Enable/disable flag */
   );
#endif

/***************************************************************************
Summary:
  Return the current host AutoValidateStcOnPause setting.

Description:
  This function returns a boolean value indicating the current setting of
  the AutoValidateStcOnPause (true/false).

Returns:
	BERR_SUCCESS - If status was returned successfully.

See Also:
  BXVD_PVR_SetAutoValidateStcOnPause
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetAutoValidateStcOnPause
(
   BXVD_ChannelHandle hXvdCh,     /* [In] XVD channel handle */
   bool *pbAutoValidateStcOnPause /* [Out] Current sparse mode status */
   );

BERR_Code BXVD_PVR_GetAutoValidateStcOnPause_isr
(
   BXVD_ChannelHandle hXvdCh,     /* [In] XVD channel handle */
   bool *pbAutoValidateStcOnPause /* [Out] Current sparse mode status */
   );
#endif

/***************************************************************************

  SW7425-2270:

  The application will call SetIgnoreNRTUnderflow when it determines that an
  NRT underflow is actually a gap in the content (e.g. slideshow or end of
  stream) and the repeated picture should actually be encoded.

  When SetIgnoreNRTUnderflow=true, the "decoder underflow" scenario should
  be ignored until either:
  - the underflow condition ends
  - the app explicitly sets SetIgnoreNRTUnderflow=false

  Note: only the "decoder underflow" condition is ignored. All other NRT cenarios
  (e.g. "Other Transcode Stalled", "FIC Stall", etc) are still in effect.

****************************************************************************/

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_SetIgnoreNRTUnderflow(
   BXVD_ChannelHandle hXvdCh,
   bool bIgnoreNRTUnderflow
   );
#endif

BERR_Code BXVD_PVR_SetIgnoreNRTUnderflow_isr(
   BXVD_ChannelHandle hXvdCh,
   bool bIgnoreNRTUnderflow
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetIgnoreNRTUnderflow(
   BXVD_ChannelHandle hXvdCh,
   bool * pbIgnoreNRTUnderflow
   );

BERR_Code BXVD_PVR_GetIgnoreNRTUnderflow_isr(
   BXVD_ChannelHandle hXvdCh,
   bool * pbIgnoreNRTUnderflow
   );
#endif

/***************************************************************************
   SW7425-3358: support for FNRT.
****************************************************************************/

#define BXVD_PVR_FNRTSettings  BXDM_PictureProvider_FNRTSettings

BERR_Code BXVD_PVR_SetFNRTSettings(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_PVR_FNRTSettings * pstFNRTSettings
   );

BERR_Code BXVD_PVR_SetFNRTSettings_isr(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_PVR_FNRTSettings * pstFNRTSettings
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetFNRTSettings(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PVR_FNRTSettings * pstFNRTSettings
   );

BERR_Code BXVD_PVR_GetFNRTSettings_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PVR_FNRTSettings * pstFNRTSettings
   );
#endif

BERR_Code BXVD_PVR_GetDefaultFNRTSettings(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PVR_FNRTSettings * pstFNRTSettings
   );

BERR_Code BXVD_PVR_GetDefaultFNRTSettings_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PVR_FNRTSettings * pstFNRTSettings
   );


/*******************/
/* Deprecated APIs */
/*******************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
  [DEPRECATED] Enable or disable BUD trick mode.
Description:
  This function enables BUD trick mode when bBudMode is true or disables
  it when bBudMode is false.

Returns:
	BERR_SUCCESS - If status was returned successfully.

See Also:
      BXVD_PVR_GetBUDMode
****************************************************************************/
BERR_Code BXVD_PVR_SetBUDMode
(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   bool bBudMode              /* [In] BUD mode enable flag */
   );

/***************************************************************************
Summary:
  [DEPRECATED] Get the current setting of the BUD trick mode flag.

Description:
  This function returns a boolean value indicating the current setting of
  BUD trick mode enable flag (true/false).

Returns:
	BERR_SUCCESS - If status was returned successfully.

See Also:
  BXVD_PVR_SetBUDMode
****************************************************************************/
BERR_Code BXVD_PVR_GetBUDMode
(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   bool *pbBudMode            /* [Out] Current BUD mode status */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

#ifdef __cplusplus
}
#endif

#endif /* BXVD_PVR_H__ */
/* End of file. */
