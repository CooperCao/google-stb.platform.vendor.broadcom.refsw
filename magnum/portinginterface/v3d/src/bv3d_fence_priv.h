/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 **************************************************************************/
#ifndef BV3D_FENCE_H__
#define BV3D_FENCE_H__

/* Fences (external view):

   Fences are used by the driver to coordinate accesses to resources.
   Fences can be created, signalled and waited upon:
      Create creates a fence in the active state
      Signal moves the fence to the signalled state, if a wait is active on this signal
      it will allowed to proceed.
      Wait will block on the fence until it has been signalled or times out
   They are global entities that are managed in the BV3D module
   A fence will persist until it has been closed
   Signal and wait can happen in either order.
   Fences are identified by an integer id.
 */

/* BV3D_P_FenceState

   Fences can be in one of three states:
   available -- the fence is free and available for use
   active    -- the fence has been allocated and is in use, but not signalled
   signalled -- the fence has been allocated and is in use and has ben signalled

 */
typedef enum BV3D_P_FenceState
{
   BV3D_FENCE_AVAILABLE = 0,
   BV3D_FENCE_ACTIVE,
   BV3D_FENCE_SIGNALLED
} BV3D_P_FenceState;

/* BV3D_P_FenceState

   Each fence holds its state and a callback closure.  The callback is invoked when
   the fence is waited upon and is signalled (thi could be immediately

 */
typedef struct BV3D_P_Fence
{
   BV3D_P_FenceState   eState;

   /* TODO: this should be a list/array of callbacks */
   void              (*pfnCallback)(void *, void *);  /* pfnCallback(hV3d, pArg) */
   void               *pArg;

   /* Client id is used when client dies to remove fences belonging to it                   */
   uint32_t            uiClientId;

   /* Identifies this fence.  Each fence is allocated a new id to prevent accidental re-use */
   int32_t             iFenceId;
} BV3D_P_Fence;

/* Fence slots are allocated BV3D_P_FENCE_ARRAY_CHUNK at a time */
#define BV3D_P_FENCE_ARRAY_CHUNK   16

typedef struct BV3D_P_FenceArray *BV3D_FenceArrayHandle;

/* BV3D_P_FenceArrayCreate

 Create a fence array

 */
BERR_Code BV3D_P_FenceArrayCreate(
   BV3D_FenceArrayHandle *phFenceArr
);

/* BV3D_P_FenceArrayDestroy

 Destroy a fence array

 */
void BV3D_P_FenceArrayDestroy(
   BV3D_FenceArrayHandle hFenceArr
);

/* BV3D_P_FenceAlloc

 Allocates a new fence.  The array must have been acquired.

 */
int BV3D_P_FenceAlloc(
   BV3D_FenceArrayHandle hFenceArr,
   uint32_t              uiClientId
);

/* BV3D_P_FenceFree

 Frees a fence.  The array must have been acquired.

 */
void BV3D_P_FenceFree(
   BV3D_FenceArrayHandle hFenceArr,
   int iFence
);

/* BV3D_P_FenceArrayAcquire

 Acquire the array.  The array must not have already been acquired.

 */
void BV3D_P_FenceArrayMutexAcquire(
   BV3D_FenceArrayHandle hFenceArr
);

/* BV3D_P_FenceArrayRelease

 Release the array.  The array must have been acquired.

 */
void BV3D_P_FenceArrayMutexRelease(
   BV3D_FenceArrayHandle hFenceArr
);

/* BV3D_P_FenceGet

 Get fence from array.  The array must have been acquired.

 */
BV3D_P_Fence *BV3D_P_FenceGet(
   BV3D_FenceArrayHandle hFenceArr,
   int iFence
);

/* BV3D_P_FenceCallback

 */
void BV3D_P_FenceCallback(
   void           *hV3d,
   BV3D_P_Fence   *pFence
);

/* BV3D_P_FenceIsSignalled

 Check if fence has been signalled yet

 */
bool BV3D_P_FenceIsSignalled(
   BV3D_FenceArrayHandle hFenceArr,
   int iFence
);

/* BV3D_P_FenceAddCallback

 Add a callback to the fence.

 */
void BV3D_P_FenceAddCallback(
   BV3D_P_Fence   *pFence,
   void          (*pfnCallback)(void *, void *),
   void           *pArg
);

/* BV3D_P_FenceRemoveCallback

 Remove a callback from the fence.

 */
void BV3D_P_FenceRemoveCallback(
   BV3D_P_Fence   *pFence,
   void           *pArg
);

/* BV3D_P_FenceClientDestroy

 Remove all fences associated with a client.

 */
void BV3D_P_FenceClientDestroy(
   BV3D_FenceArrayHandle hFenceArr,
   uint32_t              uiClientId
);

#endif /* BV3D_FENCE_H__ */
