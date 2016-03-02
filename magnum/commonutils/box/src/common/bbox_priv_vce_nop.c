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
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_vce.h"

BDBG_MODULE(BBOX_PRIV_VCE_NOP);
BDBG_OBJECT_ID(BBOX_BOX_PRIV_NOP);

#if (BCHP_CHIP == 7425)
/* For chips that don't support box modes, but has an encoder */
const BBOX_Vce_Capabilities BBOX_P_Vce_CapabilitiesLUT[] =
{
   /* Box Mode: 0 (720p30x2 - Default worst case for custom RTS boxes) */
   { 0,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p30( 0, 0x3, 1 ),
      },
   },
};
#else
const BBOX_Vce_Capabilities BBOX_P_Vce_CapabilitiesLUT[1];
#endif
const size_t BBOX_P_Vce_CapabilitiesLUT_size = sizeof( BBOX_P_Vce_CapabilitiesLUT );
/* end of file */
