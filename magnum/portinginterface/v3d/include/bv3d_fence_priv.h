/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 **************************************************************************/
#ifndef BV3D_FENCE_H__
#define BV3D_FENCE_H__

#include "bv3d.h"

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

   /* Client id is used when client dies to remove fences belonging to it                   */
   uint32_t            uiClientId;

   /* Identifies this fence.  Each fence is allocated a new id to prevent accidental re-use */
   int32_t             iFenceId;
} BV3D_P_Fence;

/* Fence slots are allocated BV3D_P_FENCE_ARRAY_CHUNK at a time */
#define BV3D_P_FENCE_ARRAY_CHUNK   16

typedef struct BV3D_P_FenceArray *BV3D_FenceArrayHandle;

/***************************************************************************/
BERR_Code BV3D_P_FenceArrayCreate(
   BV3D_FenceArrayHandle  *phFenceArr
);

/***************************************************************************/
void BV3D_P_FenceArrayDestroy(
   BV3D_FenceArrayHandle   hFenceArr
);

/***************************************************************************/
int BV3D_P_FenceOpen(
   BV3D_FenceArrayHandle   hFenceArr,
   uint32_t                uiClientId,
   void                  **dataToSignal,
   char                    cType,
   int                     iPid
);

/***************************************************************************/
void BV3D_P_FenceClose(
   BV3D_FenceArrayHandle   hFenceArr,
   int                     iFenceId
);

/***************************************************************************/
void BV3D_P_FenceSignalAndCleanup(
   BV3D_FenceArrayHandle   hFenceArr,
   void                   *dataToSignal,
   const char             *function
);

/***************************************************************************/
bool BV3D_P_FenceIsSignalled(
   BV3D_FenceArrayHandle   hFenceArr,
   void                   *pWaitData
);

/***************************************************************************/
void BV3D_P_FenceFree(
   BV3D_FenceArrayHandle   hFenceArr,
   void                   *pWaitData
);

/***************************************************************************/
BERR_Code BV3D_P_FenceWaitAsync(
   BV3D_Handle             hV3d,
   uint32_t                uiClientId,
   int                     iFenceId,
   void                  **ppWaitData
);

/***************************************************************************/
void BV3D_P_FenceClientDestroy(
   BV3D_FenceArrayHandle   hFenceArr,
   uint32_t                uiClientId
);

#endif /* BV3D_FENCE_H__ */
