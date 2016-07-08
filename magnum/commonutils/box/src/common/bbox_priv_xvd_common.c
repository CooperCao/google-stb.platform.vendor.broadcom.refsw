/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_priv.h"
#include "bbox_xvd.h"

BDBG_MODULE(BBOX_PRIV_XVD_COMMON);
BDBG_OBJECT_ID(BBOX_BOX_PRIV_XVD_COMMON);

extern const BBOX_Xvd_Config BBOX_P_Xvd_ConfigLUT[];
extern const size_t BBOX_P_Xvd_ConfigLUT_size;

BERR_Code BBOX_P_Xvd_SetBoxMode
   ( uint32_t        ulBoxId,
     BBOX_Xvd_Config *pBoxXvd )
{
   unsigned i;
   BERR_Code eStatus = BERR_SUCCESS;

   BDBG_ASSERT(pBoxXvd);

   BKNI_Memset( pBoxXvd, 0, sizeof( BBOX_Xvd_Config) );

   /* Search for Box Mode in Xvd Config LUT */
   for ( i = 0; i < ( BBOX_P_Xvd_ConfigLUT_size / sizeof( BBOX_Xvd_Config ) ); i++ )
   {
      if ( BBOX_P_Xvd_ConfigLUT[i].uiBoxId == ulBoxId )
      {
         *pBoxXvd = BBOX_P_Xvd_ConfigLUT[i];
         goto done;
      }

      /* If we hit a box mode of 0 in the LUT and it didn't match above,
       * then Xvd is not supported, and just return */
      if ( 0 == BBOX_P_Xvd_ConfigLUT[i].uiBoxId )
      {
         goto done;
      }
   }
   BDBG_ERR(("Xvd Box mode %d is not supported on this chip.", ulBoxId));
   eStatus = BERR_TRACE(BERR_INVALID_PARAMETER);
done:

   return BERR_TRACE(eStatus);
}

/* end of file */
