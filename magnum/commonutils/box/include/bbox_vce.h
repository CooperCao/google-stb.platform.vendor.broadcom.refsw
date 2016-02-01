/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BBOX_VCE_H__
#define BBOX_VCE_H__

#include "bstd.h"
#include "berr_ids.h"    /* Error codes */

#ifdef __cplusplus
extern "C" {
#endif

#define BBOX_VCE_CHANNEL_INFO_V1( _instance, _channels, _memcIndex, _input_type_default, _input_type_default_memory, _bounds_int_width, _bounds_int_height, _bounds_int_fr_default, _bounds_int_fr_min, _bounds_int_fr_max, _bounds_prog_width, _bounds_prog_height, _bounds_prog_fr_default, _bounds_prog_fr_min, _bounds_prog_fr_max ) \
{\
   _instance, /* uiInstance */\
   _channels, /* uiChannels */\
   _memcIndex, /* uiMemcIndex */\
   _input_type_default, /* eDefaultInputType */\
   _input_type_default_memory, /* eDefaultInputTypeMemory */\
   {\
   BBOX_VCE_CHANNEL_BOUNDS_INFO_V1( _bounds_int_width, _bounds_int_height, _bounds_int_fr_default, _bounds_int_fr_min, _bounds_int_fr_max ), \
   BBOX_VCE_CHANNEL_BOUNDS_INFO_V1( _bounds_prog_width, _bounds_prog_height, _bounds_prog_fr_default, _bounds_prog_fr_min, _bounds_prog_fr_max ), \
   } /* stBounds[2] */\
}

#define BBOX_VCE_CHANNEL_BOUNDS_INFO_V1( _width, _height, _fr_default, _fr_min, _fr_max ) \
   {\
      _width, /* uiWidth */\
      _height, /* uiHeight */\
      BBOX_VCE_CHANNEL_FRAME_RATE_INFO_V1( _fr_default, _fr_min, _fr_max ),\
   } /* stBounds */

#define BBOX_VCE_CHANNEL_FRAME_RATE_INFO_V1( _default, _min, _max ) \
   {\
      _default, /* eDefault */\
      _min, /* eMin */\
      _max /* eMax */\
   } /* stFrameRate */

#define BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      0, 0x00, 0,\
      BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */\
      0, 0, BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e30, /* Bounds[eInterlaced] */\
      0, 0, BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60 /* Bounds[eProgressive] */\
   )

#define BBOX_VCE_CHANNEL_INFO_720p25( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      _instance, _channels, _memcIndex,\
      BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */\
      720, 576, BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e25, /* Bounds[eInterlaced] */\
      1280, 720, BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e50 /* Bounds[eProgressive] */\
   )

#define BBOX_VCE_CHANNEL_INFO_720p30( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      _instance, _channels, _memcIndex,\
      BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */\
      720, 576, BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e30, /* Bounds[eInterlaced] */\
      1280, 720, BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60 /* Bounds[eProgressive] */\
   )

#define BBOX_VCE_CHANNEL_INFO_720p60( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      _instance, _channels, _memcIndex,\
      BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */\
      720, 576, BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e30, /* Bounds[eInterlaced] */\
      1280, 720, BAVC_FrameRateCode_e60, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60 /* Bounds[eProgressive] */\
   )

#define BBOX_VCE_CHANNEL_INFO_1080p25( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      _instance, _channels, _memcIndex,\
      BAVC_ScanType_eProgressive, BAVC_ScanType_eInterlaced, /* Instance[0] */\
      1920, 1088, BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e50, /* Bounds[eInterlaced] */\
      1920, 1088, BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e50 /* Bounds[eProgressive] */\
   )

#define BBOX_VCE_CHANNEL_INFO_1080p30( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      _instance, _channels, _memcIndex,\
      BAVC_ScanType_eProgressive, BAVC_ScanType_eInterlaced, /* Instance[0] */\
      1920, 1088, BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e30, /* Bounds[eInterlaced] */\
      1920, 1088, BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60 /* Bounds[eProgressive] */\
   )

#define BBOX_VCE_CHANNEL_INFO_1080p60( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      _instance, _channels, _memcIndex,\
      BAVC_ScanType_eProgressive, BAVC_ScanType_eInterlaced, /* Instance[0] */\
      1920, 1088, BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e30, /* Bounds[eInterlaced] */\
      1920, 1088, BAVC_FrameRateCode_e60, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60 /* Bounds[eProgressive] */\
   )

typedef struct BBOX_Vce_Channel_Capabilities
{
   uint8_t uiInstance;
   uint8_t uiChannels; /* A bit mask indicating which channels are valid */
   uint8_t uiMemcIndex;

   BAVC_ScanType eDefaultInputType;
   BAVC_ScanType eDefaultInputTypeMemory;
   struct
   {
      unsigned uiWidth;
      unsigned uiHeight;
      struct
      {
         BAVC_FrameRateCode eDefault;
         BAVC_FrameRateCode eMin;
         BAVC_FrameRateCode eMax;
      } stFrameRate;
   } stBounds[2]; /* Bounds for each BAVC_ScanType. The 1st entry is the default settings */
} BBOX_Vce_Channel_Capabilities;

#define BBOX_VCE_MAX_INSTANCE_COUNT 2
#define BBOX_VCE_MAX_CHANNEL_COUNT 3

/***************************************************************************
Summary:
	List of VCE's exposed capabilities
****************************************************************************/
typedef struct BBOX_Vce_Capabilities
{
   uint32_t uiBoxId;
   BBOX_Vce_Channel_Capabilities stInstance[BBOX_VCE_MAX_INSTANCE_COUNT];
} BBOX_Vce_Capabilities;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BBOX_VCE_H__ */

/* end of file */
