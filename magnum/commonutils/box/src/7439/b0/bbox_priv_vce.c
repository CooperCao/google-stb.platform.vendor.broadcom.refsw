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

#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_vce.h"

BDBG_MODULE(BBOX_PRIV_VCE);
BDBG_OBJECT_ID(BBOX_BOX_PRIV_VCE);

const BBOX_Vce_Capabilities BBOX_P_Vce_CapabilitiesLUT[] =
{
   /* Box Mode: 1 (No Transcode) */
   { 1,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 2 (720p60x2) */
   { 2,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p60( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 3 (No Transcode) */
   { 3,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 4 (1080p25x2) */
   { 4,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p25( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 5 (1080p30x1) */
   { 5,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x1, 0 ),
      },
   },
   /* Box Mode: 6 (No Transcode) */
   { 6,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 7 (720p25x2) */
   { 7,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p25( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 9 (1080p25x2) */
   { 9,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p25( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 10 (No Transcode) */
   { 10,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 12 (No Transcode) */
   { 12,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 13 (No Transcode) */
   { 13,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 14 (No Transcode) */
   { 14,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 16 (1080p30 x1) */
   { 16,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x1, 0 ),
      },
   },
   /* Box Mode: 17 (720p60x1) */
   { 17,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p60( 0, 0x1, 0 ),
      },
   },
   /* Box Mode: 18 (1080p30x2) */
   { 18,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 19 (720p25x1) */
   { 19,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p25( 0, 0x1, 0 ),
      },
   },
   /* Box Mode: 20 (No Transcode) */
   { 20,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
};

const size_t BBOX_P_Vce_CapabilitiesLUT_size = sizeof( BBOX_P_Vce_CapabilitiesLUT );
/* end of file */
