/***************************************************************************
*     Copyright (c) 2005-2011, Broadcom Corporation
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
#ifdef RAP_VIDEOONDSP_SUPPORT

BERR_Code BRAP_PvrEnablePause_isr
(
    BRAP_ChannelHandle hRapCh,
    bool bEnablePause
);

BERR_Code BRAP_PvrGetPauseStatus_isr
(
    BRAP_ChannelHandle hRapCh,
    bool *pbPauseStatus
);

BERR_Code BRAP_PvrSetSlowMotionRate_isr
(
    BRAP_ChannelHandle hRapCh,
    unsigned long ulRate
);

BERR_Code BRAP_PvrGetSlowMotionRate_isr
(
    BRAP_ChannelHandle hRapCh,
    unsigned long *pSMRate
);

BERR_Code BRAP_PvrEnableReverseFields_isr(
    BRAP_ChannelHandle hRapCh,
    bool bEnable
);

BERR_Code BRAP_PvrGetReverseFieldStatus_isr
(
    BRAP_ChannelHandle hRapCh,
    bool *pbEnable
);

BERR_Code BRAP_PvrSetGopTrickMode_isr
(
    BRAP_ChannelHandle hRapCh,
    bool bEnableTrickMode
);

BERR_Code BRAP_PvrGetGopTrickMode_isr
(
    BRAP_ChannelHandle hRapCh,
    bool *pbEnableTrickMode
);

BERR_Code BRAP_PvrSetAutoValidateStcOnPause_isr
(
    BRAP_ChannelHandle hRapCh,
    bool bAutoValidateStcOnPause
);

BERR_Code BRAP_PvrGetAutoValidateStcOnPause_isr
(
    BRAP_ChannelHandle hRapCh,
    bool *pbAutoValidateStcOnPause
);
#endif
/***************************************************************************
Summary: 
	Pause/resume audio

Description: 
	This function pauses/resumes audio. When unpaused (resumed), audio
	starts from the point it paused. This function works only for decode 
	channel in playback mode.

Returns:
	BERR_SUCCESS - If pause/resume operation is successful.

See Also: 
	BRAP_GetPvrPauseStatus
****************************************************************************/
BERR_Code BRAP_EnablePvrPause(
	BRAP_ChannelHandle		hRapCh,     /* [In] The RAP Channel handle */
	bool					bOnOff	    /* [In] true = Pause audio
											    false = Resume audio */
	)
{
	BERR_Code               err = BERR_SUCCESS;
    BRAP_DSPCHN_Handle      hDspCh = NULL;

	BDBG_ENTER(BRAP_EnablePvrPause);
	/* Assert the function arguments*/
	BDBG_ASSERT(hRapCh);

#ifdef RAP_VIDEOONDSP_SUPPORT
    if(BRAP_ChannelType_eVideoDecode == hRapCh->eChannelType)
    {
        BKNI_EnterCriticalSection();
        err = BRAP_PvrEnablePause_isr (hRapCh,bOnOff);
        if(BERR_SUCCESS != err)
        {
    		return BERR_TRACE(err);
        }	
        BKNI_LeaveCriticalSection();
        goto end;
    }
#endif

    /* Validate channel handle */
    if(BRAP_ChannelType_eDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("BRAP_EnablePvrPause: API supported only for a decode"
            " channel. Current channel type is %d", hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("BRAP_EnablePvrPause: bOnOff=%d", bOnOff));

	/* If audio is already in required state, return success */
	if((hRapCh->sTrickModeState.bAudioPaused==bOnOff) &&
	   (false == hRapCh->bInternalCallFromRap))
	{
        BDBG_MSG(("Decoder is already in %s state",((bOnOff)?"Pause":"Unpaused")));
	    goto end;
    }

    /* Get hDspCh handle for decode */
    hDspCh = BRAP_DSPCHN_P_GetDspChHandle(&hRapCh, true);
    if(NULL == hDspCh)
    {
        BDBG_ERR(("BRAP_EnablePvrPause: Can't get a valid decoder DspCh handle"));
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    }

    /* Pause the DSP */
	err = BRAP_DSPCHN_P_EnablePause(hDspCh, bOnOff);
	if(BERR_SUCCESS != err)
    {
		return BERR_TRACE(err);
    }	
    
	if(true == bOnOff) 
    {
		/* Change "audio paused" state to "true" */
		hRapCh->sTrickModeState.bAudioPaused = true;	
	}
    else
    {
		/* Clear frame residual time on resume */
		hRapCh->sTrickModeState.uiFrameAdvResidualTime = 0;
		/* Change "audio paused" state to "false" */
		hRapCh->sTrickModeState.bAudioPaused = false;
	}
end:
	BDBG_LEAVE(BRAP_EnablePvrPause);
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
	BRAP_EnablePvrPause
****************************************************************************/
BERR_Code BRAP_GetPvrPauseStatus(
	BRAP_ChannelHandle		hRapCh,		    /* [In] The RAP Channel handle */
	bool					*pbPauseStatus  /* [Out] true = Audio paused
										             false = Audio not paused */
	)
{
	BDBG_ENTER(BRAP_GetPvrPauseStatus);
	/* Assert the function arguments*/
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(pbPauseStatus);

#ifdef RAP_VIDEOONDSP_SUPPORT
    if(BRAP_ChannelType_eVideoDecode == hRapCh->eChannelType)
    {
        BERR_Code err = BERR_SUCCESS;
        BKNI_EnterCriticalSection();
        err = BRAP_PvrGetPauseStatus_isr (hRapCh,pbPauseStatus);
        if(BERR_SUCCESS != err)
        {
    		return BERR_TRACE(err);
        }	
        BKNI_LeaveCriticalSection();
        goto end;
    }
#endif

    /* Validate channel handle */
    if(BRAP_ChannelType_eDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("BRAP_GetPvrPauseStatus: API supported only for a decode " 
            "channel. Current channel type is %d", hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

	/* Return the current pause state for this channel */
	*pbPauseStatus = hRapCh->sTrickModeState.bAudioPaused;

    BDBG_MSG(("BRAP_GetPvrPauseStatus: bPauseStatus=%d", *pbPauseStatus));
#ifdef RAP_VIDEOONDSP_SUPPORT
end:
#endif
	BDBG_LEAVE(BRAP_GetPvrPauseStatus);
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
BERR_Code BRAP_PvrFrameAdvance (
	BRAP_ChannelHandle		hRapCh,			/* [In] The RAP Channel handle */
	unsigned int			uiFrameAdvTime	/* [In] Frame advance time in msec */
	)
{
    BERR_Code           err = BERR_SUCCESS;
    BRAP_DSPCHN_Handle  hDspCh = NULL;
    unsigned int uiFrameAdvTime45Khz =0;
    
    BDBG_ENTER(BRAP_PvrFrameAdvance);
    /* Assert the function arguments*/
    BDBG_ASSERT(hRapCh);

/* #warning [RAP] Yet to implement Frame advance for Video. */
    /* Validate channel handle */
    if(BRAP_ChannelType_eDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("BRAP_PvrFrameAdvance: API supported only for a decode"
            " channel. Current channel type is %d", hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

	/* Return error if audio is not paused */
    if(false == hRapCh->sTrickModeState.bAudioPaused) 
    {
        BDBG_ERR(("BRAP_PvrFrameAdvance: For Frame Advance audio should be in"
        " paused state"));
        return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
    }

    /* Get hDspCh handle for decode */
    hDspCh = BRAP_DSPCHN_P_GetDspChHandle(&hRapCh, true);
    if(NULL == hDspCh)
    {
        BDBG_ERR(("BRAP_PvrFrameAdvance: Can't get a valid decoder DspCh handle"));
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    }

    uiFrameAdvTime45Khz = uiFrameAdvTime *45;  /* Time in TIME_45KHZ_TICKS*/
    BDBG_MSG(("uiFrameAdvTime (from app)=%d uiFrameAdvTime45Khz (for fw) %d", 
            uiFrameAdvTime,uiFrameAdvTime45Khz));

    err = BRAP_DSPCHN_P_FrameAdvance(hDspCh, uiFrameAdvTime45Khz);
    if (err!=BERR_SUCCESS)
    {
        return BERR_TRACE(err);
    }	

	BDBG_LEAVE(BRAP_PvrFrameAdvance);
	return BERR_SUCCESS;
}

#ifdef RAP_VIDEOONDSP_SUPPORT
/***************************************************************************
***************************************************************************/

BERR_Code BRAP_PvrEnablePause_isr
(
    BRAP_ChannelHandle hRapCh,
    bool bEnablePause
)
{
    BERR_Code ret;
    BXDM_Picture_Rate stCurrentPlaybackRate;
    BDBG_ENTER(BRAP_PvrEnablePause_isr);

    BDBG_ASSERT(hRapCh);

    ret = BXDM_PictureProvider_GetPlaybackRate_isr(
            hRapCh->hPictureProvider,
            &stCurrentPlaybackRate);

    if ( BERR_SUCCESS == ret )
    {
      switch ( bEnablePause )
      {
         case true:
            if ( 0 != stCurrentPlaybackRate.uiNumerator )
            {
#if 0
               hRapCh->stSavedPlaybackRate = stCurrentPlaybackRate;
               hRapCh->bSavedPlaybackRateValid = true;
#endif
               stCurrentPlaybackRate.uiNumerator = 0;
               stCurrentPlaybackRate.uiDenominator = 0;

               ret = BXDM_PictureProvider_SetPlaybackRate_isr(
                        hRapCh->hPictureProvider,
                        &stCurrentPlaybackRate
                        );
            }
            break;

         case false:
            if ( 0 == stCurrentPlaybackRate.uiNumerator )
            {
#if 0
               if ( true == hRapCh->bSavedPlaybackRateValid )
               {
                  ret = BXDM_PictureProvider_SetPlaybackRate_isr(
                           hRapCh->hPictureProvider,
                           &hRapCh->stSavedPlaybackRate
                           );
               }
               else
#endif
               {
                  stCurrentPlaybackRate.uiNumerator = 1;
                  stCurrentPlaybackRate.uiDenominator = 1;

                  ret = BXDM_PictureProvider_SetPlaybackRate_isr(
                           hRapCh->hPictureProvider,
                           &stCurrentPlaybackRate
                           );
#if 0
                  hRapCh->stSavedPlaybackRate = stCurrentPlaybackRate;
                  hRapCh->bSavedPlaybackRateValid = true;
#endif
               }
            }
            break;
      }
    }

    BDBG_MSG( ("BRAP_PvrEnablePause(%d)", bEnablePause));

    BDBG_LEAVE(BRAP_EnablePause_isr);
    return BERR_TRACE(ret);
}

/***************************************************************************
***************************************************************************/

BERR_Code BRAP_PvrGetPauseStatus_isr
(
    BRAP_ChannelHandle hRapCh,
    bool *pbPauseStatus
)
{
    BERR_Code ret;
    BXDM_Picture_Rate stCurrentPlaybackRate;
    BDBG_ENTER(BRAP_PvrGetPauseStatus_isr);

    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pbPauseStatus);

    ret = BXDM_PictureProvider_GetPlaybackRate_isr(
            hRapCh->hPictureProvider,
            &stCurrentPlaybackRate);

    *pbPauseStatus = (0 == stCurrentPlaybackRate.uiNumerator );

    BDBG_LEAVE(BRAP_PvrGetPauseStatus_isr);
    return BERR_TRACE( ret );
}

/***************************************************************************
***************************************************************************/
BERR_Code BRAP_PvrSetSlowMotionRate
(
    BRAP_ChannelHandle hRapCh,
    unsigned long ulRate
)
{
    BERR_Code ret;

    BDBG_ENTER(BRAP_PvrSetSlowMotionRate);

    /* Validate channel handle */
    if(BRAP_ChannelType_eVideoDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("%s : API supported only for a Video decode"
            " channel. Current channel type is %d", __FUNCTION__,hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BKNI_EnterCriticalSection();
    ret = BRAP_PvrSetSlowMotionRate_isr(
      hRapCh,
      ulRate
      );
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_PvrSetSlowMotionRate);
    return BERR_TRACE(ret);
}

BERR_Code BRAP_PvrSetSlowMotionRate_isr
(
    BRAP_ChannelHandle hRapCh,
    unsigned long ulRate
)
{
    BERR_Code ret;
    BXDM_Picture_Rate stPlaybackRate;
    BDBG_ENTER(BRAP_PvrSetSlowMotionRate_isr);

    BDBG_ASSERT(hRapCh);

    BDBG_MSG( ("BRAP_PvrSetSlowMotionRate(%d)", ulRate));

    /* The addition of "uiPvrPlaybackRate" allows the playback rate to be treated roughly
    * as a peretentage of normal playback.
    */
    BKNI_Memset( &stPlaybackRate, 0, sizeof( BXDM_Picture_Rate ) );

    stPlaybackRate.uiNumerator = 1;
    if ( ulRate > 1 )
    {
      stPlaybackRate.uiDenominator = ulRate;
    }
    else
    {
      stPlaybackRate.uiDenominator = 1;
    }

    ret = BXDM_PictureProvider_SetPlaybackRate_isr(
         hRapCh->hPictureProvider,
         &stPlaybackRate
         );
#if 0
    hRapCh->stSavedPlaybackRate = stPlaybackRate;
    hRapCh->bSavedPlaybackRateValid = true;
#endif
    BDBG_LEAVE(BRAP_PvrSetSlowMotionRate);
    return BERR_TRACE( ret );
}

/***************************************************************************
***************************************************************************/
BERR_Code BRAP_PvrGetSlowMotionRate
(
    BRAP_ChannelHandle hRapCh,
    unsigned long *pSMRate
)
{
    BERR_Code ret;
    BDBG_ENTER(BRAP_PvrGetSlowMotionRate);

    /* Validate channel handle */
    if(BRAP_ChannelType_eVideoDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("%s : API supported only for a Video decode"
            " channel. Current channel type is %d", __FUNCTION__,hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BKNI_EnterCriticalSection();
    ret = BRAP_PvrGetSlowMotionRate_isr(
      hRapCh,
      pSMRate
      );
    BKNI_LeaveCriticalSection();
    
    BDBG_LEAVE(BRAP_PvrGetSlowMotionRate);
    return BERR_TRACE(ret);
}

BERR_Code BRAP_PvrGetSlowMotionRate_isr
(
    BRAP_ChannelHandle hRapCh,
    unsigned long *pSMRate
)
{
    BERR_Code ret;
    BXDM_Picture_Rate stPlaybackRate;
    BDBG_ENTER(BRAP_PvrGetSlowMotionRate_isr);

    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pSMRate);

    ret = BXDM_PictureProvider_GetPlaybackRate_isr(
         hRapCh->hPictureProvider,
         &stPlaybackRate
         );

    if ( 0 == stPlaybackRate.uiNumerator )
    {
      *pSMRate = 0;
    }
    else
    {
      *pSMRate = stPlaybackRate.uiDenominator / stPlaybackRate.uiNumerator;
    }

    BDBG_LEAVE(BRAP_PvrGetSlowMotionRate_isr);
    return BERR_TRACE( ret );
}

/***************************************************************************
***************************************************************************/
#if 0
    BERR_Code BRAP_PvrFrameAdvance
    (
    BRAP_ChannelHandle hRapCh,
    BRAP_PvrFrameAdvanceMode eFrameAdvMode
    )
    {
    BERR_Code ret;

    BDBG_ENTER(BRAP_PvrFrameAdvance);

    BKNI_EnterCriticalSection();
    ret = BRAP_PvrFrameAdvance_isr(
      hRapCh,
      eFrameAdvMode
      );
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_PvrFrameAdvance);
    return BERR_TRACE(ret);
    }

    BERR_Code BRAP_PvrFrameAdvance_isr
    (
    BRAP_ChannelHandle hRapCh,
    BRAP_PvrFrameAdvanceMode eFrameAdvMode
    )
    {
    BERR_Code ret;
    BDBG_ENTER(BRAP_PvrFrameAdvance_isr);

    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(eFrameAdvMode <= BRAP_PvrFrameAdvanceMode_eField_by_Field);

    switch ( eFrameAdvMode )
    {
      case BRAP_PvrFrameAdvanceMode_eFrame_by_Frame:
         ret = BXDM_PictureProvider_SetFrameAdvanceMode_isr(
               hRapCh->hPictureProvider,
               BXDM_PictureProvider_FrameAdvanceMode_eFrameByField
               );
         break;

      case BRAP_PvrFrameAdvanceMode_eField_by_Field:
         ret = BXDM_PictureProvider_SetFrameAdvanceMode_isr(
               hRapCh->hPictureProvider,
               BXDM_PictureProvider_FrameAdvanceMode_eField
               );
         break;

      default:
         BRAP_DBG_ERR(hRapCh, ("Unknown frame advance mode (%d)", eFrameAdvMode));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_MSG( ("BRAP_PvrFrameAdvance(%d)", eFrameAdvMode));

    BDBG_LEAVE(BRAP_PvrFrameAdvance_isr);
    return BERR_TRACE( ret );
    }
#endif
/***************************************************************************
***************************************************************************/
BERR_Code BRAP_PvrEnableReverseFields(
    BRAP_ChannelHandle hRapCh,
    bool bEnable
)
{
    BERR_Code ret;

    BDBG_ENTER(BRAP_PvrEnableReverseFields);

    /* Validate channel handle */
    if(BRAP_ChannelType_eVideoDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("%s : API supported only for a Video decode"
            " channel. Current channel type is %d", __FUNCTION__,hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BKNI_EnterCriticalSection();
    ret = BRAP_PvrEnableReverseFields_isr(
      hRapCh,
      bEnable
      );
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_PvrEnableReverseFields);
    return BERR_TRACE(ret);
}

BERR_Code BRAP_PvrEnableReverseFields_isr(
    BRAP_ChannelHandle hRapCh,
    bool bEnable
)
{
    BERR_Code ret;
    BDBG_ENTER(BRAP_PvrEnableReverseFields_isr);

    BDBG_ASSERT(hRapCh);

    ret = BXDM_PictureProvider_SetReverseFieldsMode_isr(
         hRapCh->hPictureProvider,
         bEnable
         );

    BDBG_MSG( ("BRAP_PvrEnableReverseFields(%d)", bEnable));

    BDBG_LEAVE(BRAP_PvrEnableReverseFields_isr);
    return BERR_TRACE( ret );
}

/***************************************************************************
***************************************************************************/
BERR_Code BRAP_PvrGetReverseFieldStatus
(
    BRAP_ChannelHandle hRapCh,
    bool *pbEnable
)
{
    BERR_Code ret;

    BDBG_ENTER(BRAP_PvrGetReverseFieldStatus);

    /* Validate channel handle */
    if(BRAP_ChannelType_eVideoDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("%s : API supported only for a Video decode"
            " channel. Current channel type is %d", __FUNCTION__,hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BKNI_EnterCriticalSection();
    ret = BRAP_PvrGetReverseFieldStatus_isr(
      hRapCh,
      pbEnable
      );
    BKNI_LeaveCriticalSection();
    BDBG_LEAVE(BRAP_PvrGetReverseFieldStatus);
    return BERR_TRACE(ret);
}

BERR_Code BRAP_PvrGetReverseFieldStatus_isr
(
    BRAP_ChannelHandle hRapCh,
    bool *pbEnable
)
{
    BERR_Code ret;
    BDBG_ENTER(BRAP_PvrGetReverseFieldStatus_isr);

    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pbEnable);

    ret = BXDM_PictureProvider_GetReverseFieldsMode_isr(
         hRapCh->hPictureProvider,
         pbEnable
         );

    BDBG_LEAVE(BRAP_PvrGetReverseFieldStatus_isr);
    return BERR_TRACE( ret );
}

/***************************************************************************
***************************************************************************/
BERR_Code BRAP_PvrSetHostSparseMode(
    BRAP_ChannelHandle hRapCh,
    bool bSparseMode
)
{
/*    BAVC_XptContextMap  XptContextMap;
    bool bCurSparseMode = hRapCh->stDecoderConfig.bHostSparseMode;*/

/*    BXDM_PictureProvider_PreserveStateSettings stPreserveStateSettings;*/
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER(BRAP_PvrSetHostSparseMode);
    BDBG_ASSERT(hRapCh);
    BSTD_UNUSED(bSparseMode);

    /* Validate channel handle */
    if(BRAP_ChannelType_eVideoDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("%s : API supported only for a Video decode"
            " channel. Current channel type is %d", __FUNCTION__,hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

/* #warning [RAP] Implement host sparse mode */
#if 0
    hRapCh->stDecoderConfig.bHostSparseMode = bSparseMode;

    if ((bCurSparseMode != bSparseMode) && (hRapCh->eDecoderState == BRAP_P_DecoderState_eActive))
    {
      if ((hRapCh->sDecodeSettings.eVideoCmprStd != BAVC_VideoCompressionStd_eH264) &&
      (bSparseMode == true))
      {
     BRAP_DBG_WRN(hRapCh, ("Sparse mode enabled on Non H264 stream, Sparse mode now disabled"));
         hRapCh->stDecoderConfig.bHostSparseMode = false;
      }
      else
      {
     BDBG_MSG( ("BRAP_PvrSetHostSparseMode: Stopping and starting decoding"));

     BERR_TRACE(BXDM_PictureProvider_GetDefaultPreserveStateSettings(
              hRapCh->hPictureProvider,
              &stPreserveStateSettings
              ));

     stPreserveStateSettings.bDisplay = true;
     stPreserveStateSettings.bCounters = true;

     BERR_TRACE(BXDM_PictureProvider_SetPreserveStateSettings_isr(
              hRapCh->hPictureProvider,
              &stPreserveStateSettings
              ));

     BERR_TRACE(BRAP_StopDecode(hRapCh));

     /* Reset XPT Rave CDB read register address */
     XptContextMap.CDB_Read = hRapCh->ulXptCDB_Read;
     hRapCh->sDecodeSettings.pContextMap = &XptContextMap;

     ret = BERR_TRACE(BRAP_StartDecode(hRapCh, &hRapCh->sDecodeSettings));
      }
    }
    else
    {
      BDBG_MSG( ("BRAP_PvrSetHostSparseMode: NOT stopping/starting decoding.. Letting flush do it!!!"));
    }
#endif

    BDBG_LEAVE(BRAP_PvrSetHostSparseMode);
    return BERR_TRACE(ret);
}

/***************************************************************************
***************************************************************************/
BERR_Code BRAP_PvrGetHostSparseMode
(
    BRAP_ChannelHandle hRapCh,
    bool *pbSparseMode
)
{
    BDBG_ENTER(BRAP_PvrGetHostSparseMode);
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pbSparseMode);
#if 0
    *pbSparseMode = hRapCh->stDecoderConfig.bHostSparseMode;
#endif
    BDBG_LEAVE(BRAP_PvrGetHostSparseMode);
    return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
***************************************************************************/
BERR_Code BRAP_PvrSetGopTrickMode
(
    BRAP_ChannelHandle hRapCh,
    bool bEnableTrickMode
)
{
    BERR_Code ret;

    BDBG_ENTER(BRAP_PvrSetGopTrickMode);

    /* Validate channel handle */
    if(BRAP_ChannelType_eVideoDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("%s : API supported only for a Video decode"
            " channel. Current channel type is %d", __FUNCTION__,hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BKNI_EnterCriticalSection();
    ret = BRAP_PvrSetGopTrickMode_isr(
      hRapCh,
      bEnableTrickMode
      );
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_PvrSetGopTrickMode);
    return BERR_TRACE(ret);
}

BERR_Code BRAP_PvrSetGopTrickMode_isr
(
    BRAP_ChannelHandle hRapCh,
    bool bEnableTrickMode
)
{
    BDBG_ENTER(BRAP_PvrSetGopTrickMode_isr);

    BDBG_ASSERT(hRapCh);

/* #warning [RAP] Implement DQT support*/
#if 0
    hRapCh->stDecoderConfig.bReversePlayback = bEnableTrickMode;
#endif
    BDBG_MSG( ("BRAP_PvrSetGopTrickMode(%d)", bEnableTrickMode));

    BDBG_LEAVE(BRAP_PvrSetGopTrickMode_isr);
    return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
***************************************************************************/
BERR_Code BRAP_PvrGetGopTrickMode
(
    BRAP_ChannelHandle hRapCh,
    bool *pbEnableTrickMode
)
{
    BERR_Code ret;

    BDBG_ENTER(BRAP_PvrGetGopTrickMode);

    /* Validate channel handle */
    if(BRAP_ChannelType_eVideoDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("%s : API supported only for a Video decode"
            " channel. Current channel type is %d", __FUNCTION__,hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BKNI_EnterCriticalSection();
    ret = BRAP_PvrGetGopTrickMode_isr(
      hRapCh,
      pbEnableTrickMode
      );
    BKNI_LeaveCriticalSection();
    BDBG_LEAVE(BRAP_PvrGetGopTrickMode);
    return BERR_TRACE(ret);
}

BERR_Code BRAP_PvrGetGopTrickMode_isr
(
    BRAP_ChannelHandle hRapCh,
    bool *pbEnableTrickMode
)
{
    BDBG_ENTER(BRAP_PvrGetGopTrickMode_isr);

    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pbEnableTrickMode);
#if 0
    *pbEnableTrickMode = hRapCh->stDecoderConfig.bReversePlayback;
#endif
    BDBG_LEAVE(BRAP_PvrGetGopTrickMode_isr);
    return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
***************************************************************************/
BERR_Code BRAP_PvrSetAutoValidateStcOnPause
(
    BRAP_ChannelHandle hRapCh,
    bool bAutoValidateStcOnPause
)
{
    BERR_Code ret;

    BDBG_ENTER(BRAP_PvrSetAutoValidateStcOnPause);

    /* Validate channel handle */
    if(BRAP_ChannelType_eVideoDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("%s : API supported only for a Video decode"
            " channel. Current channel type is %d", __FUNCTION__,hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BKNI_EnterCriticalSection();
    ret = BRAP_PvrSetAutoValidateStcOnPause_isr(
      hRapCh,
      bAutoValidateStcOnPause
      );
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_PvrSetAutoValidateStcOnPause);
    return BERR_TRACE(ret);
}

BERR_Code BRAP_PvrSetAutoValidateStcOnPause_isr
(
    BRAP_ChannelHandle hRapCh,
    bool bAutoValidateStcOnPause
)
{
    BERR_Code ret;
    BDBG_ENTER(BRAP_PvrSetAutoValidateStcOnPause_isr);

    BDBG_ASSERT(hRapCh);

    ret = BXDM_PictureProvider_SetAutoValidateStcOnPauseMode_isr(
         hRapCh->hPictureProvider,
         bAutoValidateStcOnPause
         );

    BDBG_MSG( ("BRAP_PvrSetAutoValidateStcOnPause(%d)", bAutoValidateStcOnPause));

    BDBG_LEAVE(BRAP_PvrSetAutoValidateStcOnPause_isr);
    return BERR_TRACE(ret);
}

/***************************************************************************
***************************************************************************/
BERR_Code BRAP_PvrGetAutoValidateStcOnPause
(
    BRAP_ChannelHandle hRapCh,
    bool *pbAutoValidateStcOnPause
)
{
    BERR_Code ret;

    BDBG_ENTER(BRAP_PvrGetAutoValidateStcOnPause);

    /* Validate channel handle */
    if(BRAP_ChannelType_eVideoDecode != hRapCh->eChannelType)
    {
        BDBG_ERR(("%s : API supported only for a Video decode"
            " channel. Current channel type is %d", __FUNCTION__,hRapCh->eChannelType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BKNI_EnterCriticalSection();
    ret = BRAP_PvrGetAutoValidateStcOnPause_isr(
      hRapCh,
      pbAutoValidateStcOnPause
      );
    BKNI_LeaveCriticalSection();
    BDBG_LEAVE(BRAP_PvrGetAutoValidateStcOnPause);
    return BERR_TRACE(ret);
}

BERR_Code BRAP_PvrGetAutoValidateStcOnPause_isr
(
    BRAP_ChannelHandle hRapCh,
    bool *pbAutoValidateStcOnPause
)
{
    BERR_Code ret;
    BDBG_ENTER(BRAP_PvrGetAutoValidateStcOnPause_isr);

    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pbAutoValidateStcOnPause);

    ret = BXDM_PictureProvider_GetAutoValidateStcOnPauseMode_isr(
         hRapCh->hPictureProvider,
         pbAutoValidateStcOnPause
         );

    BDBG_LEAVE(BRAP_PvrGetAutoValidateStcOnPause_isr);
    return BERR_TRACE( ret );
}
#endif


/* End of File */

