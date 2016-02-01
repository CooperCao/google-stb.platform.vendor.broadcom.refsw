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
   /* Box Mode: 1 (720p30x1) */
   { 1,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p30( 0, 0x1, 0 ),
      },
   },
   /* Box Mode: 2 (720p30x2) */
   { 2,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p30( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 3 (No Transcode) */
   { 3,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 4 (1080p30x2) */
   { 4,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 5 (No Transcode) */
   { 5,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 6 (1080p30x2) */
   { 6,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x3, 0 ),
      },
   },
};

const size_t BBOX_P_Vce_CapabilitiesLUT_size = sizeof( BBOX_P_Vce_CapabilitiesLUT );
/* end of file */
