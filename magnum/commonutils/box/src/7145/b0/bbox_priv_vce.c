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
   /* Box Mode: 1 (720p30x3) */
   { 1,
      { /* Instance Array */
         { 0, 0x3, BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */
            { /* Bounds Array */
               /* Bounds[eInterlaced] */  {  720, 576, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60 } },
               /* Bounds[eProgressive] */ { 1280, 720, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60 } },
            },
         },
         { 1, 0x1, BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[1] */
            { /* Bounds Array */
               /* Bounds[eInterlaced] */  {  720, 576, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60 } },
               /* Bounds[eProgressive] */ { 1280, 720, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60 } },
            },
         },
      },
   },
};

const size_t BBOX_P_Vce_CapabilitiesLUT_size = sizeof( BBOX_P_Vce_CapabilitiesLUT );
/* end of file */
