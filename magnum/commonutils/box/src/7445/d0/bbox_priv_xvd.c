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
#include "bbox.h"
#include "bbox_xvd.h"

BDBG_MODULE(BBOX_PRIV_XVD);
BDBG_OBJECT_ID(BBOX_BOX_PRIV_XVD);

const BBOX_Xvd_Config BBOX_P_Xvd_ConfigLUT[] =
{
   {
      1,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 2, 1, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 1, 1 ),
         BBOX_XVD_DECODER_INFO_Dual1080p60_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 5, 2, 4, 3 ),
      }
   },

   {
      2,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 1, 0, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e10Bit, 1, 1),
      }
   },

   {
      3,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 2, 1, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_1080p30_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 3, 1),
         BBOX_XVD_DECODER_INFO_Dual1080p30_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 4, 2, 5, 3 ),
      }
   },

   {
      4,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 1, 0, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_Dual1080p50_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 1, 1, 2, 2),
      }
   },

   {
      5,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 1, 0, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_1080p30_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 2, 1 ),
      }
   },

   {
      6,
      {
         BBOX_XVD_DECODER_INFO_1080p60_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_Dual1080p60_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e10Bit, 2, 1, 3, 2 ),
      }
   },

   {
      7,
      {
         BBOX_XVD_DECODER_INFO_Dual1080p60_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e10Bit, 0, 0, 1, 1 ),
         BBOX_XVD_DECODER_INFO_Dual1080p60_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e10Bit, 2, 2, 3, 3 ),
         BBOX_XVD_DECODER_INFO_Dual1080p60_V1( 2, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e10Bit, 4, 4, 5, 5 ),
      }
   },

   {
      8,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 2, 0, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e10Bit, 1, 1 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e10Bit, 2, 2 ),
      }
   },

   {
      9,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 2, 1, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_1080p30_V1( 2, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 1, 1 ),
         BBOX_XVD_DECODER_INFO_Dual1080p30_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 2, 2, 3, 2 )
      }
   },

   {
      10,
      {
         BBOX_XVD_DECODER_INFO_Dual4Kp60_V1( 1, 0, BAVC_VideoBitDepth_e10Bit, 0, 0, 1, 0 )
      }
   },

   {
      12,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 2, 1, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 1, 1 ),
         BBOX_XVD_DECODER_INFO_Dual1080p60_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 5, 2, 4, 3 )
      }
   },

   {
      13,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 2, 1, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 1, 1 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 2, 2 )
      }
   },

   {
      14,
      {
         BBOX_XVD_DECODER_INFO_1080p60_V1( 2, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 2, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 1, 1 ),
         BBOX_XVD_DECODER_INFO_Dual1080p30_V1( 2, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 2, 2, 3, 3 )
      }
   },

   {
      15,
      {
         BBOX_XVD_DECODER_INFO_1080p60_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 0, 1 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 1, 2 )
      }
   },

   {
      16,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 2, 1, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_Dual1080p60_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 1, 1, 2, 2 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 1, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 3, 3 )
      }
   },

   {
      17,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 1, 0, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_Dual1080p30_V1( 2, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 1, 1, 2, 2 ),
         BBOX_XVD_DECODER_INFO_1080p30_V1( 2, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 3, 3 )
      }
   },

   {
      18,
      {
         BBOX_XVD_DECODER_INFO_1080p60_V1( 2, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 2, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 1, 1 ),
         BBOX_XVD_DECODER_INFO_Dual1080p30_V1( 2, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 2, 2, 3, 3 )
      }
   },

   {
      1000,
      {
         BBOX_XVD_DECODER_INFO_4Kp60_V1( 2, 1, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
         BBOX_XVD_DECODER_INFO_Dual1080p60_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 3, 1, 2, 2),
         BBOX_XVD_DECODER_INFO_1080p60_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e8Bit, 1, 2 )
      }
   },

   {
      1001,
      {
         BBOX_XVD_DECODER_INFO_1080p60_V1( 0, BBOX_XVD_UNUSED, BAVC_VideoBitDepth_e10Bit, 0, 0 ),
      }
   },

};

const size_t BBOX_P_Xvd_ConfigLUT_size = sizeof( BBOX_P_Xvd_ConfigLUT );
/* end of file */
