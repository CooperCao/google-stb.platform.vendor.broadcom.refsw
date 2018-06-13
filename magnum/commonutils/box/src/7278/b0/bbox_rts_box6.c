/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_rts_priv.h"
#include "bbox_rts_box6.h"

BDBG_MODULE(BBOX_RTS_BOX6);
BDBG_OBJECT_ID(BBOX_RTS_BOX6);

void BBOX_P_SetBox6MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    BBOX_P_SET_DVI_CFC_MEMC(pBoxMemConfig, 0);

    BBOX_P_SET_HDR_VIDEO_AND_GFX_MEMC(pBoxMemConfig, Display0,   0,       1);

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video0,  1,       0);
    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display0,  Video1,  1,       0);
    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display0,  Gfx0,    1         );

    BBOX_P_SET_GFX_WIN_MEMC(  pBoxMemConfig, Display1,  Gfx0,    1         );

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display2,  Video0,  0,       0);

    BBOX_P_SET_VIDEO_WIN_MEMC(pBoxMemConfig, Display3,  Video0,  0,       0);

    BBOX_P_SET_NUM_MEMC(pBoxMemConfig, stBoxRts_7278B0_box6_box_mid_performance_sm_box6.ulNumMemc);
}

void BBOX_P_GetBox6Rts
    ( BBOX_Rts *pBoxRts )
{
    *pBoxRts = stBoxRts_7278B0_box6_box_mid_performance_sm_box6;
}
