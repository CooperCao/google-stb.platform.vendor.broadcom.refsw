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
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#include "bv3d_fence_priv.h"

BDBG_MODULE(BV3D_Fence);

typedef struct BV3D_P_FenceArray
{
   BKNI_MutexHandle  hMutex;
   uint32_t          uiCapacity;
   int32_t           iFirstAvailable;
   int32_t           iNextFenceId;
   BV3D_P_Fence     *pFences;
} BV3D_P_FenceArray;

BERR_Code BV3D_P_FenceArrayCreate(
   BV3D_FenceArrayHandle *phFenceArray
)
{
   BERR_Code               err = BERR_SUCCESS;
   BV3D_FenceArrayHandle   hFenceArr = NULL;

   if (phFenceArray == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   hFenceArr = (BV3D_P_FenceArray *)BKNI_Malloc(sizeof(BV3D_P_FenceArray));

   if (hFenceArr == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   BKNI_Memset(hFenceArr, 0, sizeof(BV3D_P_FenceArray));

   err = BKNI_CreateMutex(&hFenceArr->hMutex);

   if (err != BERR_SUCCESS)
      goto exit;

   hFenceArr->pFences = (BV3D_P_Fence *)BKNI_Malloc(sizeof(BV3D_P_Fence) * BV3D_P_FENCE_ARRAY_CHUNK);

   if (hFenceArr->pFences == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   hFenceArr->uiCapacity      = BV3D_P_FENCE_ARRAY_CHUNK;
   hFenceArr->iFirstAvailable = 0;
   hFenceArr->iNextFenceId    = 1;

   BKNI_Memset(hFenceArr->pFences, 0, sizeof(BV3D_P_Fence) * hFenceArr->uiCapacity);

   *phFenceArray = hFenceArr;

exit:
   if (err != BERR_SUCCESS)
      BV3D_P_FenceArrayDestroy(hFenceArr);

   return err;
}

void BV3D_P_FenceArrayDestroy(
   BV3D_FenceArrayHandle hFenceArr
)
{
   if (hFenceArr == NULL)
      return;

   if (hFenceArr->hMutex != 0)
      BKNI_DestroyMutex(hFenceArr->hMutex);

   if (hFenceArr->pFences != NULL)
      BKNI_Free(hFenceArr->pFences);

   BKNI_Free(hFenceArr);
}

int BV3D_P_FenceAlloc(
   BV3D_FenceArrayHandle hFenceArr,
   uint32_t              uiClientId
)
{
   bool     found   = false;
   int32_t  fenceId = -1;
   int32_t  fenceIx = 0;
   uint32_t i;

   /* Did we run out of fences? */
   if (hFenceArr->iFirstAvailable == -1)
   {
      BV3D_P_Fence *newArray =
         (BV3D_P_Fence *)BKNI_Malloc(sizeof(BV3D_P_Fence) * (hFenceArr->uiCapacity + BV3D_P_FENCE_ARRAY_CHUNK));

      if (newArray == NULL)
         goto exit;

      BKNI_Memcpy(newArray, hFenceArr->pFences, sizeof(BV3D_P_Fence) * hFenceArr->uiCapacity);
      BKNI_Memset(&newArray[hFenceArr->uiCapacity], 0, sizeof(BV3D_P_Fence) * BV3D_P_FENCE_ARRAY_CHUNK);

      hFenceArr->iFirstAvailable = hFenceArr->uiCapacity;
      hFenceArr->uiCapacity      = hFenceArr->uiCapacity + BV3D_P_FENCE_ARRAY_CHUNK;

      BKNI_Free(hFenceArr->pFences);
      hFenceArr->pFences = newArray;
   }

   /* Allocate a fence */
   fenceIx = hFenceArr->iFirstAvailable;
   fenceId = hFenceArr->iNextFenceId;

   hFenceArr->pFences[fenceIx].eState     = BV3D_FENCE_ACTIVE;
   hFenceArr->pFences[fenceIx].uiClientId = uiClientId;
   hFenceArr->pFences[fenceIx].iFenceId   = fenceId;

   if (fenceId == 0x7fffffff)
      hFenceArr->iNextFenceId = 1;
   else
      hFenceArr->iNextFenceId += 1;

   BDBG_MSG(("Fence %d = %p", fenceId, (void *)&hFenceArr->pFences[fenceIx]));

   /* Find the next available fence */
   for (i = hFenceArr->iFirstAvailable + 1; i < hFenceArr->uiCapacity && !found; ++i)
   {
      if (hFenceArr->pFences[i].eState == BV3D_FENCE_AVAILABLE)
      {
         hFenceArr->iFirstAvailable = i;
         found = true;
      }
   }

   /* Run out? */
   if (!found)
      hFenceArr->iFirstAvailable = -1;

exit:
   return fenceId;
}

/* Find the index for a fence
   -1 if not found
 */
static int32_t BV3D_P_FenceIndex(
   BV3D_FenceArrayHandle hFenceArr,
   int iFenceId
)
{
   uint32_t i;
   int32_t  iFenceIx = -1;

   if (iFenceId < 0)
      return -1;

   for (i = 0; i < hFenceArr->uiCapacity && iFenceIx == -1; ++i)
   {
      if (hFenceArr->pFences[i].iFenceId == iFenceId)
         iFenceIx = i;
   }

   return iFenceIx;
}

void BV3D_P_FenceFree(
   BV3D_FenceArrayHandle hFenceArr,
   int                   iFenceId
)
{
   int32_t         iFenceIx = BV3D_P_FenceIndex(hFenceArr, iFenceId);
   BV3D_P_Fence   *pFence;

   if (iFenceIx < 0 || (uint32_t)iFenceIx >= hFenceArr->uiCapacity)
      return;

   BDBG_MSG(("Fence Free %d", iFenceId));

   pFence = &hFenceArr->pFences[iFenceIx];

   pFence->pfnCallback = NULL;
   pFence->pArg        = NULL;
   pFence->eState      = BV3D_FENCE_AVAILABLE;
   pFence->iFenceId    = 0;
   pFence->uiClientId  = 0;

   if (iFenceIx < hFenceArr->iFirstAvailable)
      hFenceArr->iFirstAvailable = iFenceIx;
}

void BV3D_P_FenceArrayMutexAcquire(
   BV3D_FenceArrayHandle hFenceArr
)
{
   BKNI_AcquireMutex(hFenceArr->hMutex);
}

void BV3D_P_FenceArrayMutexRelease(
   BV3D_FenceArrayHandle hFenceArr
)
{
   BKNI_ReleaseMutex(hFenceArr->hMutex);
}

BV3D_P_Fence *BV3D_P_FenceGet(
   BV3D_FenceArrayHandle hFenceArr,
   int                   iFenceId
)
{
   int32_t  iFenceIx = BV3D_P_FenceIndex(hFenceArr, iFenceId);

   if (hFenceArr == NULL || iFenceIx < 0 || (uint32_t)iFenceIx >= hFenceArr->uiCapacity)
      return NULL;

   return &hFenceArr->pFences[iFenceIx];
}

void BV3D_P_FenceAddCallback(
   BV3D_P_Fence   *pFence,
   void          (*pfnCallback)(void *, void *),
   void           *pArg
)
{
   /* TODO: handle multiple callbacks */
   BDBG_ASSERT(pFence->pArg == NULL);

   pFence->pfnCallback = pfnCallback;
   pFence->pArg        = pArg;
}

void BV3D_P_FenceRemoveCallback(
   BV3D_P_Fence   *pFence,
   void           *pArg
)
{
   /* TODO: handle multiple callbacks */
   BDBG_ASSERT(pFence->pArg == NULL || pFence->pArg == pArg);

   pFence->pfnCallback = NULL;
   pFence->pArg        = NULL;
}

void BV3D_P_FenceCallback(
   void           *hV3d,
   BV3D_P_Fence   *pFence
)
{
   /* TODO: handle multiple callbacks */
   if (pFence->pfnCallback != NULL)
   {
      pFence->pfnCallback(hV3d, pFence->pArg);
   }

   /* This callback is done so don't do it again */
   pFence->pfnCallback = NULL;
   pFence->pArg        = NULL;
}

bool BV3D_P_FenceIsSignalled(
   BV3D_FenceArrayHandle hFenceArr,
   int                   iFenceId
)
{
   int32_t  iFenceIx = BV3D_P_FenceIndex(hFenceArr, iFenceId);

   if (hFenceArr == NULL || iFenceIx < 0 || (uint32_t)iFenceIx >= hFenceArr->uiCapacity)
      return false;

   return hFenceArr->pFences[iFenceIx].eState == BV3D_FENCE_SIGNALLED;
}

void BV3D_P_FenceClientDestroy(
   BV3D_FenceArrayHandle hFenceArr,
   uint32_t              uiClientId
)
{
   uint32_t i;

   BV3D_P_FenceArrayMutexAcquire(hFenceArr);

   for (i = 0; i < hFenceArr->uiCapacity; ++i)
   {
      BV3D_P_Fence   *pFence = &hFenceArr->pFences[i];

      if (pFence->eState != BV3D_FENCE_AVAILABLE && pFence->uiClientId == uiClientId)
         BV3D_P_FenceFree(hFenceArr, pFence->iFenceId);
   }

   BV3D_P_FenceArrayMutexRelease(hFenceArr);
}
