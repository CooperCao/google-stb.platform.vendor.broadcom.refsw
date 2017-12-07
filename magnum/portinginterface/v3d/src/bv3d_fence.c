/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#include "bv3d_fence_priv.h"

/******************************************************************************
 * NOTE!
 * Nexus platform layers use explicit sync, so buffers are only dequeued when they
 * can be written into.  For queue a swapbuffer job is inserted into the pipeline
 * which signals a user space equivalency of a fence when render is done.
 *
 * These functions are only used for Android
 *****************************************************************************/

BDBG_MODULE(BV3D_Fence);

/***************************************************************************/
BERR_Code BV3D_P_FenceArrayCreate(
   BV3D_FenceArrayHandle *phFenceArray
)
{
   BSTD_UNUSED(phFenceArray);
   return BERR_SUCCESS;
}

/***************************************************************************/
void BV3D_P_FenceClientDestroy(
   BV3D_FenceArrayHandle hFenceArr,
   uint32_t              uiClientId
)
{
   BSTD_UNUSED(hFenceArr);
   BSTD_UNUSED(uiClientId);
}

/***************************************************************************/
void BV3D_P_FenceArrayDestroy(
   BV3D_FenceArrayHandle hFenceArr
)
{
   BSTD_UNUSED(hFenceArr);
}

/***************************************************************************/
int BV3D_P_FenceOpen(
   BV3D_FenceArrayHandle   hFenceArr,
   uint32_t                uiClientId,
   void                  **dataToSignal,
   char                    cType,
   int                     iPid
)
{
   BSTD_UNUSED(hFenceArr);
   BSTD_UNUSED(uiClientId);
   BSTD_UNUSED(cType);
   BSTD_UNUSED(iPid);
   *dataToSignal = NULL;
   return -1;
}

/***************************************************************************/
void BV3D_P_FenceClose(
   BV3D_FenceArrayHandle hFenceArr,
   int                   iFenceId
)
{
   BSTD_UNUSED(hFenceArr);
   BSTD_UNUSED(iFenceId);
}

/***************************************************************************/
void BV3D_P_FenceSignalAndCleanup(
   BV3D_FenceArrayHandle   hFenceArr,
   void                   *dataToSignal,
   const char             *function
)
{
   BSTD_UNUSED(hFenceArr);
   BSTD_UNUSED(dataToSignal);
   BSTD_UNUSED(function);
}

/***************************************************************************/
bool BV3D_P_FenceIsSignalled(
   BV3D_FenceArrayHandle   hFenceArr,
   void                   *pWaitData
)
{
   BSTD_UNUSED(hFenceArr);
   BSTD_UNUSED(pWaitData);
   return BV3D_FENCE_SIGNALLED;
}

/***************************************************************************/
void BV3D_P_FenceFree(
   BV3D_FenceArrayHandle   hFenceArr,
   void                   *pWaitData
)
{
   BSTD_UNUSED(hFenceArr);
   BSTD_UNUSED(pWaitData);
}

/***************************************************************************/
BERR_Code BV3D_P_FenceWaitAsync(
   BV3D_Handle           hV3d,
   uint32_t              uiClientId,
   int                   iFenceId,
   void                **ppWaitData
)
{
   /* nexus version shouldn't be called */
   BSTD_UNUSED(hV3d);
   BSTD_UNUSED(uiClientId);
   BSTD_UNUSED(iFenceId);
   BSTD_UNUSED(ppWaitData);

   return BERR_SUCCESS;
}
