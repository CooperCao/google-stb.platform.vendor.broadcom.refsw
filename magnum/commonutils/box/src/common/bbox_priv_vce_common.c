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
#include "bbox_priv.h"
#include "bbox_vce.h"

BDBG_MODULE(BBOX_PRIV_VCE_COMMON);
BDBG_OBJECT_ID(BBOX_BOX_PRIV_VCE_COMMON);

extern const BBOX_Vce_Capabilities BBOX_P_Vce_CapabilitiesLUT[];
extern const size_t BBOX_P_Vce_CapabilitiesLUT_size;

BERR_Code BBOX_P_Vce_SetBoxMode
   ( uint32_t               ulBoxId,
     BBOX_Vce_Capabilities *pBoxVce )
{
   unsigned i;
   BERR_Code eStatus = BERR_SUCCESS;

   BDBG_ASSERT(pBoxVce);

   BKNI_Memset( pBoxVce, 0, sizeof( BBOX_Vce_Capabilities) );

   /* Search for Box Mode in VCE Capabilities LUT */
   for ( i = 0; i < ( BBOX_P_Vce_CapabilitiesLUT_size / sizeof( BBOX_Vce_Capabilities ) ); i++ )
   {
      if ( BBOX_P_Vce_CapabilitiesLUT[i].uiBoxId == ulBoxId )
      {
         *pBoxVce = BBOX_P_Vce_CapabilitiesLUT[i];
         goto done;
      }

      /* If we hit a box mode of 0 in the LUT and it didn't match above,
       * then VCE is not supported, and just return */
      if ( 0 == BBOX_P_Vce_CapabilitiesLUT[i].uiBoxId )
      {
         goto done;
      }
   }
   BDBG_ERR(("VCE Box mode %d is not supported on this chip.", ulBoxId));
   eStatus = BERR_TRACE(BERR_INVALID_PARAMETER);
done:

   return BERR_TRACE(eStatus);
}


/* end of file */
