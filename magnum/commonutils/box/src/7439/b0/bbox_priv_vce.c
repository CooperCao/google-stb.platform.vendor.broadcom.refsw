/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

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
   /* Box Mode: 21 (720p60x1) */
   { 21,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p60( 0, 0x1, 0 ),
      },
   },
   /* Box Mode: 22 (1080p30x2) */
   { 22,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 23 (720p60x1) */
   { 23,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p60( 0, 0x1, 0 ),
      },
   },
   /* Box Mode: 24 (1080p30x2) */
   { 24,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 25 (720p60x2) */
   { 25,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_720p60( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 26 (No Transcode) */
   { 26,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
   /* Box Mode: 27 (1080p30x2) */
   { 27,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_1080p30( 0, 0x3, 0 ),
      },
   },
   /* Box Mode: 28 (No Transcode) */
   { 28,
      { /* Instance Array */
         BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( 0, 0x0, 0 ),
      },
   },
};

const size_t BBOX_P_Vce_CapabilitiesLUT_size = sizeof( BBOX_P_Vce_CapabilitiesLUT );
/* end of file */
