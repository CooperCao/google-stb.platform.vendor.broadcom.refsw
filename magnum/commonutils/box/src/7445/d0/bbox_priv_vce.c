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
   /* Box Mode: 1 (720p30x2) */
   { 1,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p30( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 2 (No Transcode) */
   { 2,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 3 (1080p30x3) */
   { 3,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x3, 0 ),
         BBOX_VCE_CHANNEL_INFO_1080p30( 1, 0x1, 2 ),
      },
   },
   /* Box Mode: 4 (720p25x1) */
   { 4,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p25( 0, 0x1, 1 ),
      },
   },
   /* Box Mode: 5 (720p30x1) */
   { 5,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p30( 0, 0x1, 1 ),
      },
   },
   /* Box Mode: 6 (720p60x2) */
   { 6,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p60( 0, 0x3, 1 ),
      },
   },

   /* Box Mode 7: (720p30x6) */
   { 7,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p30( 0, 0x7, 2 ),
         BBOX_VCE_CHANNEL_INFO_720p30( 1, 0x7, 2 ),
      },
   },
   /* Box Mode: 8 (1080p30x3) */
   { 8,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x3, 0 ),
         BBOX_VCE_CHANNEL_INFO_1080p30( 1, 0x1, 1 ),
      },
   },
   /* Box Mode: 9 (1080p30x2) */
   { 9,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 10 (No Transcode) */
   { 10,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 12 (720p30x2) */
   { 12,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p30( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 13 (1080p60x1) */
   { 13,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p60( 0, 0x1, 0 ),
      },
   },
   /* Box Mode: 14 (1080p30x3) */
   { 14,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x3, 1 ),
         BBOX_VCE_CHANNEL_INFO_1080p30( 1, 0x1, 1 ),
      },
   },
   /* Box Mode: 15 (1080p60x1) */
   { 15,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p60( 0, 0x1, 0 ),
      },
   },
   /* Box Mode: 1000 (1080p30x4) */
   { 1000,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x3, 0 ),
         BBOX_VCE_CHANNEL_INFO_1080p30( 1, 0x3, 1 ),
      },
   },
   /* Box Mode: 1001 (No Transcode) */
   { 1001,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
};

const size_t BBOX_P_Vce_CapabilitiesLUT_size = sizeof( BBOX_P_Vce_CapabilitiesLUT );
/* end of file */
