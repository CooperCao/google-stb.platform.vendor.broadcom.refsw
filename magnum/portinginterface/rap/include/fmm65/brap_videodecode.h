/***************************************************************************
*     Copyright (c) 2006-2010, Broadcom Corporation
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
*	This file contains the implementations of the APIs for a decode channel, 
*   which are exposed to the upper layer by the Raptor Audio PI. This file 
*   is part of Audio Manager Module. Audio Manager module is the top level 
*   module of the RAP PI, which interfaces with the caller and manages the 
*   internal channel objects to get RAP PI produce the desired result.
*
* Revision History:
*
* $brcm_Log: $
* 
**************************************************************************/

#ifndef _BRAP_VIDEODECODE_H_
#define _BRAP_VIDEODECODE_H_

#include "bxdm_decoder.h"
#include "bxdm_picture.h"
#include "bxdm_dih.h"
#include "bxdm_pp.h"
#include "brap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 Summary:
  Enum used to set XDM's picture drop mode

 Description:
  DM may need to drop pictures in order to perform rate matching.  DM
  can be set into a field-drop or frame-drop mode with the
  BRAP_SetPictureDropMode() command.
 See also:
  BRAP_SetPictureDropMode

****************************************************************************/
typedef enum BRAP_VID_PictureDropMode
{
   BRAP_VID_PictureDropMode_eField,
   BRAP_VID_PictureDropMode_eFrame,

   /* Add more enums ABOVE this line */
   BRAP_VID_PictureDropMode_eMax
} BRAP_VID_PictureDropMode;

typedef enum BRAP_VID_PulldownMode
{
   BRAP_VID_PulldownMode_eTopBottom,
   BRAP_VID_PulldownMode_eBottomTop,
   BRAP_VID_PulldownMode_eUseEncodedFormat,

   BRAP_VID_PulldownMode_eMax
} BRAP_VID_PulldownMode;

typedef enum BRAP_VID_FrameRateDetectionMode
{
   /* Disable frame rate detection.  Will always return unknown */
   BRAP_VID_FrameRateDetectionMode_eOff,

   /* Fast frame rate detection.  Will return best guess as soon as
    * possible.  Result may oscillate initially for very close frame
    * rates (e.g. 29.97 vs 30 fps) */
   BRAP_VID_FrameRateDetectionMode_eFast,

   /* Stable frame rate detection.  Will return "unknown" until frame
    * rate stabilizes.  Good for differentiating between close frame
    * rates (e.g. 29.97 vs 30 fps) */
   BRAP_VID_FrameRateDetectionMode_eStable
} BRAP_VID_FrameRateDetectionMode;

typedef enum BRAP_VID_MonitorRefreshRate
{
   BRAP_VID_MonitorRefreshRate_eUnknown = 0,
   BRAP_VID_MonitorRefreshRate_e23_976Hz,
   BRAP_VID_MonitorRefreshRate_e24Hz,
   BRAP_VID_MonitorRefreshRate_e25Hz,
   BRAP_VID_MonitorRefreshRate_e29_97Hz,
   BRAP_VID_MonitorRefreshRate_e30Hz,
   BRAP_VID_MonitorRefreshRate_e50Hz,
   BRAP_VID_MonitorRefreshRate_e59_94Hz,
   BRAP_VID_MonitorRefreshRate_e60Hz,
   BRAP_VID_MonitorRefreshRate_e48Hz,

   /* Add new enums above this line */
   BRAP_VID_MonitorRefreshRate_eMax
} BRAP_VID_MonitorRefreshRate;

/***************************************************************************
 Summary:
  Enum used to specify Display Manager picture error handling.
****************************************************************************/
typedef enum BRAP_VID_ErrorHandling
{
   BRAP_VID_ErrorHandling_eOff = 0,         /* display all pictures */
   BRAP_VID_ErrorHandling_ePicture,         
   BRAP_VID_ErrorHandling_ePrognostic,      
   BRAP_VID_ErrorHandling_eMaxErrorHandling
} BRAP_VID_ErrorHandling;

/***************************************************************************
Summary:
    Structure for returning the GOP time code value

Description:
    SMPTE time codes coming in the MPEG stream as per ISO/IEC 13818-2
        standard.
See Also:
    BRAP_VID_GetGopTimeCode

****************************************************************************/
typedef struct BRAP_VID_GopTimeCode
{
  unsigned long ulTimeCodeHours;    /* The hours field */
  unsigned long ulTimeCodeMinutes;  /* The minutes field */
  unsigned long ulTimeCodeSeconds;  /* The seconds field */
  unsigned long ulTimeCodePictures; /* The pictures (frames) field */
  bool bTimeCodeValid;              /* Valid timecode in input stream */
} BRAP_VID_GopTimeCode;

#if 0

/***************************************************************************
Summary:
    Enum used to select the type of field display.

Description:
    We can select top field only, bottom fld or both field display.

See Also:
    BRAP_VID_SetDisplayFieldMode, BRAP_VID_GetDisplayFieldMode, BRAP_VID_GetDisplayFieldMode_isr

****************************************************************************/
typedef enum BRAP_VID_DisplayFieldType
{
  BRAP_VID_DisplayFieldType_eBothField,       /* regular both field display */
  BRAP_VID_DisplayFieldType_eTopFieldOnly,    /* Display Top fld only */
  BRAP_VID_DisplayFieldType_eBottomFieldOnly,  /* Display Bottom fld only */
  BRAP_VID_DisplayFieldType_eSingleField, /* Display single field only.
                                           * The field that is displayed
                                           * is the same as the one that
                                           * is currently being
                                           * displayed. */
  BRAP_VID_DisplayFieldType_eMaxFieldType
} BRAP_VID_DisplayFieldType;
#endif

/**************************************************************************
Summary:
  Used to specify the channel change mode. This instructs the display
  manager to either mute video during a channel change or to display
  the last frame of the previous channel.

  FirstPicturePreview allows for the first picture of the new channel
  to be displayed as soon as it is available, while TSM lock is in
  the process of being acquired.  The first picture will remain on
  screen until display can continue with TSM, if TSM is enabled.
****************************************************************************/
typedef enum BRAP_VID_ChannelChangeMode
{
  BRAP_VID_ChannelChangeMode_eMute,                                /* Mute on Channel change */
  BRAP_VID_ChannelChangeMode_eLastFramePreviousChannel,            /* Display last frame of previous channel */
  BRAP_VID_ChannelChangeMode_eMuteWithFirstPicturePreview,         /* Mute until first picture of new channel */
  BRAP_VID_ChannelChangeMode_eLastFramePreviousWithFirstPicturePreview, /* Show last picture until first picture of new channel */
  BRAP_VID_ChannelChangeMode_eMaxModes
} BRAP_VID_ChannelChangeMode;

/**************************************************************************
Summary:
  Used to specify still content interpolation mode
****************************************************************************/
typedef enum BRAP_VID_StillContentInterpolationMode
{
  BRAP_VID_StillContentInterpolationMode_eDefault, /* This is the default state wherein XVD is instructed to use the correct display reccomendations */
  BRAP_VID_StillContentInterpolationMode_eSingleField,/* Last Valid Field Display on Pause */
  BRAP_VID_StillContentInterpolationMode_eBothField,/* Both Field Display on Pause */
  BRAP_VID_StillContentInterpolationMode_eFrame, /* Frame Display (conditional) on Pause */
  BRAP_VID_StillContentInterpolationMode_eMaxModes
} BRAP_VID_StillContentInterpolationMode;

/**************************************************************************
Summary:
  Used to specify moving content interpolation mode
****************************************************************************/
typedef enum BRAP_VID_MovingContentInterpolationMode
{
  BRAP_VID_MovingContentInterpolationMode_eDefault,
  BRAP_VID_MovingContentInterpolationMode_eInterlacedScanout,
  BRAP_VID_MovingContentInterpolationMode_eProgressiveScanout,
  BRAP_VID_MovingContentInterpolationMode_eMaxModes
} BRAP_VID_MovingContentInterpolationMode;

/***************************************************************************
Summary:
  Structure for returning all display threshold values (in 45KHz clock ticks)
****************************************************************************/
typedef struct BRAP_VID_DisplayThresholds
{
  uint32_t  ui32ThresholdLo;
  uint32_t  ui32ThresholdHi;
  uint32_t  ui32DiscardThreshold;
  uint32_t  ui32VeryLateThreshold;
} BRAP_VID_DisplayThresholds;

/***************************************************************************
Summary:
	Real video channel params structure.

Description:
	Structure to hold the channel start parameters. This structure is used 
	while starting any real video channel.
****************************************************************************/
typedef struct BRAP_VideoParams
{
    BRAP_DSPCHN_VideoType	        eType;                      /* Video Type */
    unsigned int                    ui32DispStripeWidth;        /*What will be Default Values?? */                    
    unsigned int                    ui32DispStripeHeight;       /*What will be Default Values?? */ 
    unsigned int                    ui32NumBuffAvl;         /*What will be Default Values?? */ 
    BXDM_DisplayInterruptHandler_Handle hXdmDih;                /* XDM Display Interrupt Handle */
    BRAP_VID_PictureDropMode        ePictureDropMode;           /* PIcture Drop mode for XDM */
	uint32_t                        uiVsyncDiffThreshDefault;   /* Threshold for STC and PTS diff */
	BRAP_VID_MonitorRefreshRate     eMonitorRefreshRate;     /* The monitor Refresh Rate in Hertz*/
	BXDM_PictureProvider_1080pScanMode          e1080pScanMode;             /* 1080p scan mode for this channel */
	BAVC_VideoCompressionStd        eVideoCmprStd;              /* Video compression standard */
	bool                            bCrcMode;                   /* CRC Enable/Disable for XDM */
	BRAP_VID_PulldownMode           ePulldownMode;              /* Picture pulldown mode */
	unsigned long                   ulRemovalDelay;             /* Initial removal delay in vsync units */
	uint32_t                        uiPreRollRate;  		    /* essentially a slow motion rate to be applied until a PPB passes TSM test */
	bool                            bPlayback;                  /* decoder is in playback mode */
	BAVC_FrameRateCode              eDefaultFrameRate;          /* User-settable default frame rate if unknown from stream */
	BRAP_VID_FrameRateDetectionMode eFrameRateDetectionMode;    /* Frame Rate Detection Mode that should be used if both the coded frame rate and eDefaultFrameRate are unknown */
	bool 	                        bAstmMode;                  /* Set DM to Adaptive System Time Management (ASTM) mode */
	bool                            bVsyncModeOnPcrDiscontinuity;	/* if the STC is valid, continue displaying pictures even if the PCR offset becomes invalid. */
	uint32_t                        uiVDCRectangleNum;	/* VDC rectangle number, multi-decode mode */
    BRAP_VID_ErrorHandling          eErrorHandling;
} BRAP_VideoParams;

BERR_Code BRAP_VID_GetDMInterface
(
         BRAP_ChannelHandle hRapCh,
         BXDM_Decoder_Interface *pstDecoderInterface,
         void **pPrivateContext
);

BERR_Code BRAP_VID_SetPictureDropMode
(
   BRAP_ChannelHandle           hRapCh,             /* [in] The Rap Channel handle */
   BRAP_VID_PictureDropMode     ePictureDropMode    /* [in] the picture drop mode */
);

BERR_Code BRAP_VID_SetPictureDropMode_isr
(
   BRAP_ChannelHandle           hRapCh,             /* [in] The Rap Channel handle */
   BRAP_VID_PictureDropMode     ePictureDropMode    /* [in] the picture drop mode */
);

BERR_Code BRAP_VID_SetPtsStcDiffThreshold
(
   BRAP_ChannelHandle hRapCh,
   long uiPtsStcDiffThreshold
);

BERR_Code BRAP_VID_SetPtsStcDiffThreshold_isr
(
   BRAP_ChannelHandle hRapCh,
   long uiPtsStcDiffThreshold
);

BERR_Code BRAP_VID_GetPtsStcDiffThreshold
(
   BRAP_ChannelHandle hRapCh,
   long *puiPtsStcDiffThreshold
);

BERR_Code BRAP_VID_GetPtsStcDiffThreshold_isr
(
   BRAP_ChannelHandle hRapCh,
   long *puiPtsStcDiffThreshold
);

BERR_Code BRAP_VID_SetMonitorRefreshRate
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MonitorRefreshRate eMonitorRefreshRate
);

BERR_Code BRAP_VID_SetMonitorRefreshRate_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MonitorRefreshRate eMonitorRefreshRate
);

BERR_Code BRAP_VID_GetMonitorRefreshRate
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MonitorRefreshRate *peMonitorRefreshRate
);

BERR_Code BRAP_VID_GetMonitorRefreshRate_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MonitorRefreshRate *peMonitorRefreshRate
);

BERR_Code BRAP_VID_Set1080pScanMode
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_1080pScanMode e1080pScanMode
);

BERR_Code BRAP_VID_Set1080pScanMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_1080pScanMode e1080pScanMode
);

BERR_Code BRAP_VID_Get1080pScanMode
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_1080pScanMode *pe1080pScanMode
);

BERR_Code BRAP_VID_Get1080pScanMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_1080pScanMode *pe1080pScanMode
);

BERR_Code BRAP_VID_SetErrorHandlingMode
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ErrorHandling eErrorMode
);

BERR_Code BRAP_VID_SetErrorHandlingMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ErrorHandling eErrorMode
);

BERR_Code BRAP_VID_GetErrorHandlingMode
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ErrorHandling * peErrorMode
);

BERR_Code BRAP_VID_GetErrorHandlingMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ErrorHandling * peErrorMode
);

BERR_Code BRAP_VID_GetPTS
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_PTSInfo   *pPTSInfo
);

BERR_Code BRAP_VID_GetPTS_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_PTSInfo   *pPTSInfo
);

BERR_Code BRAP_VID_GetLastCodedPTS
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_PTSInfo       *pPTSInfo
   );

BERR_Code BRAP_VID_GetLastCodedPTS_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_PTSInfo       *pPTSInfo
   );

BERR_Code BRAP_VID_GetNextPTS
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_PTSInfo *pPTSInfo /* [out] PTS Info is returned*/
   );

BERR_Code BRAP_VID_GetNextPTS_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_PTSInfo *pPTSInfo /* [out] PTS Info is returned*/
   );

BERR_Code BRAP_VID_GetIPictureFoundStatus
(
   BRAP_ChannelHandle hRapCh,
   bool *pbIPictureFound
   );

BERR_Code BRAP_VID_GetIPictureFoundStatus_isr
(
   BRAP_ChannelHandle hRapCh,
   bool *pbIPictureFound
   );

BERR_Code BRAP_VID_GetPictureTag
(
   BRAP_ChannelHandle    hRapCh,
   unsigned long        *pulPictureTag
   );

BERR_Code BRAP_VID_GetPictureTag_isr
(
   BRAP_ChannelHandle    hRapCh,
   unsigned long        *pulPictureTag
   );

BERR_Code BRAP_VID_GetGopTimeCode
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_GopTimeCode *pGopTimeCode
   );

BERR_Code BRAP_VID_GetGopTimeCode_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_GopTimeCode *pGopTimeCode
   );

BERR_Code BRAP_VID_EnableMute
(
   BRAP_ChannelHandle hRapCh,
   bool               bEnable
   );

BERR_Code BRAP_VID_EnableMute_isr
(
   BRAP_ChannelHandle hRapCh,
   bool               bEnable
   );

BERR_Code BRAP_VID_SetDisplayFieldMode
(
   BRAP_ChannelHandle    hRapCh,
   BXDM_PictureProvider_DisplayFieldMode eDisplayFieldType
   );

BERR_Code BRAP_VID_SetDisplayFieldMode_isr
(
   BRAP_ChannelHandle    hRapCh,
   BXDM_PictureProvider_DisplayFieldMode eDisplayFieldType
   );

BERR_Code BRAP_VID_GetDisplayFieldMode
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayFieldMode *peDisplayFieldType
   );

BERR_Code BRAP_VID_GetDisplayFieldMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayFieldMode *peDisplayFieldType
   );

BERR_Code BRAP_VID_SetChannelChangeMode
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ChannelChangeMode eChChangeMode
   );

BERR_Code BRAP_VID_SetChannelChangeMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ChannelChangeMode eChChangeMode
   );


BERR_Code BRAP_VID_GetChannelChangeMode
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ChannelChangeMode *peChChangeMode
   );

BERR_Code BRAP_VID_GetChannelChangeMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_ChannelChangeMode *peChChangeMode
   );

BERR_Code BRAP_VID_SetInterpolationModeForStillContent
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_StillContentInterpolationMode eNewInterpolation
   );

BERR_Code BRAP_VID_SetInterpolationModeForStillContent_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_StillContentInterpolationMode eNewInterpolation
   );

BERR_Code BRAP_VID_GetInterpolationModeForStillContent
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_StillContentInterpolationMode *peStillContentIntrpMode
   );

BERR_Code BRAP_VID_GetInterpolationModeForStillContent_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_StillContentInterpolationMode *peStillContentIntrpMode
   );

BERR_Code BRAP_VID_SetInterpolationModeForMovingContent
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MovingContentInterpolationMode eNewInterpolation
   );

BERR_Code BRAP_VID_SetInterpolationModeForMovingContent_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MovingContentInterpolationMode eNewInterpolation
   );

BERR_Code BRAP_VID_GetInterpolationModeForMovingContent
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MovingContentInterpolationMode *peNewInterpolation
   );

BERR_Code BRAP_VID_GetInterpolationModeForMovingContent_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_MovingContentInterpolationMode *peNewInterpolation
   );

BERR_Code BRAP_VID_SetVideoFreezeState
(
   BRAP_ChannelHandle hRapCh,
   bool               bVFState
   );

BERR_Code BRAP_VID_SetVideoFreezeState_isr
(
   BRAP_ChannelHandle hRapCh,
   bool               bVFState
   );

BERR_Code BRAP_VID_GetVideoFreezeState
(
   BRAP_ChannelHandle hRapCh,
   bool               *bVFState
   );

BERR_Code BRAP_VID_GetVideoFreezeState_isr
(
   BRAP_ChannelHandle hRapCh,
   bool               *bVFState
   );

BERR_Code BRAP_VID_Set240iScanMode
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_240iScanMode e240iScanMode /* [in] The new 240i scan mode */
   );

BERR_Code BRAP_VID_Set240iScanMode_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_240iScanMode e240iScanMode /* [in] The new 240i scan mode */
   );

BERR_Code BRAP_VID_Get240iScanMode
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_240iScanMode *pe240iScanMode /* [out] The current 240i scan mode */
   );

BERR_Code BRAP_VID_Get240iScanMode_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BXDM_PictureProvider_240iScanMode *pe240iScanMode /* [out] The current 240i scan mode */
   );

BERR_Code BRAP_VID_SetDisplayOffset
(
   BRAP_ChannelHandle hRapCh,
   long               lDisplayOffsetValue
   );

BERR_Code BRAP_VID_SetDisplayOffset_isr
(
   BRAP_ChannelHandle hRapCh,
   long               lDisplayOffsetValue
   );

BERR_Code BRAP_VID_GetDisplayOffset
(
   BRAP_ChannelHandle hRapCh,
   long               *plDisplayOffsetValue
   );

BERR_Code BRAP_VID_GetDisplayOffset_isr
(
   BRAP_ChannelHandle hRapCh,
   long               *plDisplayOffsetValue
   );

BERR_Code BRAP_VID_SetVideoDisplayMode
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayMode   eDisplayMode
   );

BERR_Code BRAP_VID_SetVideoDisplayMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayMode   eDisplayMode
   );

BERR_Code BRAP_VID_GetVideoDisplayMode
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayMode   *peDisplayMode
   );

BERR_Code BRAP_VID_GetVideoDisplayMode_isr
(
   BRAP_ChannelHandle hRapCh,
   BXDM_PictureProvider_DisplayMode   *peDisplayMode
   );

BERR_Code BRAP_VID_GetDisplayThresholds
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_DisplayThresholds *pDispThresholds
   );

BERR_Code BRAP_VID_GetDisplayThresholds_isr
(
   BRAP_ChannelHandle hRapCh,
   BRAP_VID_DisplayThresholds *pDispThresholds
   );

BERR_Code BRAP_VID_SetDiscardThreshold
(
   BRAP_ChannelHandle hRapCh,
   uint32_t           ui32DiscardThreshold
   );

BERR_Code BRAP_VID_SetDiscardThreshold_isr
(
   BRAP_ChannelHandle hRapCh,
   uint32_t           ui32DiscardThreshold
   );

BERR_Code BRAP_VID_SetVeryLateThreshold
(
   BRAP_ChannelHandle hRapCh,
   uint32_t           ui32VeryLateThreshold
   );

BERR_Code BRAP_VID_SetVeryLateThreshold_isr
(
   BRAP_ChannelHandle hRapCh,
   uint32_t           ui32VeryLateThreshold
   );

BERR_Code BRAP_VID_SetSTCInvalidFlag
(
   BRAP_ChannelHandle hRapCh,
   bool bStcInvalidFlag
   );

BERR_Code BRAP_VID_SetSTCInvalidFlag_isr
(
   BRAP_ChannelHandle hRapCh,
   bool bStcInvalidFlag
   );

BERR_Code BRAP_VID_GetSTCInvalidFlag
(
   BRAP_ChannelHandle hRapCh,
   bool *pbStcInvalidFlag
   );

BERR_Code BRAP_VID_GetSTCInvalidFlag_isr
(
   BRAP_ChannelHandle hRapCh,
   bool *pbStcInvalidFlag
   );

BERR_Code BRAP_VID_SetClipTime
(
   BRAP_ChannelHandle hRapCh,  /* [in] The XVD Channel handle */
   BXDM_PictureProvider_ClipTimeType eClipTimeType, /* [in] Clip Time Type */
   uint32_t    ui32StartTime, /* [in] Start display from the specified time/PTS */
   uint32_t    ui32StopTime /* [in] Stop display at the specified time/PTS */
   );

BERR_Code BRAP_VID_SetClipTime_isr
(
   BRAP_ChannelHandle hRapCh,  /* [in] The XVD Channel handle */
   BXDM_PictureProvider_ClipTimeType eClipTimeType, /* [in] Clip Time Type */
   uint32_t    ui32StartTime, /* [in] Start display from the specified time/PTS */
   uint32_t    ui32StopTime /* [in] Stop display at the specified time/PTS */
   );

BERR_Code BRAP_VID_SetSwPcrOffset
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   uint32_t uiSwPcrOffset     /* [in] software PCR offset */
   );

BERR_Code BRAP_VID_SetSwPcrOffset_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   uint32_t uiSwPcrOffset     /* [in] software PCR offset */
   );

BERR_Code BRAP_VID_GetSwPcrOffset
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   uint32_t * puiSwPcrOffset  /* [out] software PCR offset */
   );

BERR_Code BRAP_VID_GetSwPcrOffset_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   uint32_t * puiSwPcrOffset  /* [out] software PCR offset */
   );

BERR_Code BRAP_VID_SetHwPcrOffsetEnable
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   bool bHwPcrOffsetEnable    /* [in] hardware PCR offset enable flag */
   );

BERR_Code BRAP_VID_SetHwPcrOffsetEnable_isr
(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   bool bHwPcrOffsetEnable    /* [in] hardware PCR offset enable flag */
   );

BERR_Code BRAP_VID_GetHwPcrOffsetEnable
(
   BRAP_ChannelHandle hRapCh,    /* [in] The XVD Channel handle */
   bool * pbHwPcrOffsetEnable    /* [out] hardware PCR offset enable flag */
   );

BERR_Code BRAP_VID_GetHwPcrOffsetEnable_isr
(
   BRAP_ChannelHandle hRapCh,    /* [in] The XVD Channel handle */
   bool * pbHwPcrOffsetEnable    /* [out] hardware PCR offset enable flag */
   );

BERR_Code BRAP_VID_SetVsyncPlaybackRate(
   BRAP_ChannelHandle hRapCh,
   uint32_t uiVsyncPlaybackRate
   );

BERR_Code BRAP_VID_SetVsyncPlaybackRate_isr(
   BRAP_ChannelHandle hRapCh,
   uint32_t uiVsyncPlaybackRate
   );

BERR_Code BRAP_VID_GetVsyncPlaybackRate(
   BRAP_ChannelHandle hRapCh,
   uint32_t * puiVsyncPlaybackRate
   );

BERR_Code BRAP_VID_GetVsyncPlaybackRate_isr(
   BRAP_ChannelHandle hRapCh,
   uint32_t * puiVsyncPlaybackRate
   );

BERR_Code BRAP_VID_SetJitterToleranceImprovementEnable(
   BRAP_ChannelHandle hRapCh,
   bool bEnable
   );

BERR_Code BRAP_VID_SetJitterToleranceImprovementEnable_isr(
   BRAP_ChannelHandle hRapCh,
   bool bEnable
   );

BERR_Code BRAP_VID_GetJitterToleranceImprovementEnable(
   BRAP_ChannelHandle hRapCh,
   bool *pbEnable
   );

BERR_Code BRAP_VID_GetJitterToleranceImprovementEnable_isr(
   BRAP_ChannelHandle hRapCh,
   bool *pbEnable
   );

BERR_Code BRAP_VID_GetPictureDropMode(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BRAP_VID_PictureDropMode *pePictureDropMode /* [out] the current picture drop mode */
   );

BERR_Code BRAP_VID_GetPictureDropMode_isr(
   BRAP_ChannelHandle hRapCh, /* [in] The XVD Channel handle */
   BRAP_VID_PictureDropMode *pePictureDropMode /* [out] the current picture drop mode */
   );

#ifdef __cplusplus
}
#endif


#endif /* _BRAP_VIDEODECODE_H_ */
