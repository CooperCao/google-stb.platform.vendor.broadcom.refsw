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
#include "bv3d.h"
#include "bv3d_notifyq_priv.h"
#include "bkni.h"
#include "blst_queue.h"

typedef struct BV3D_P_Notification
{
   uint64_t             uiParam;
   uint32_t             uiSync;
   uint32_t             uiClientId;
   uint32_t             uiOutOfMemory;
   uint64_t             uiJobSequence;
   BV3D_TimelineData    sTimelineData;
   BLST_Q_ENTRY(BV3D_P_Notification)      sChain;
} BV3D_P_Notification;

typedef struct BV3D_P_NotifyQHandle
{
   BLST_Q_HEAD(sList, BV3D_P_Notification) sQueue;
} BV3D_P_NotifyQHandle;

/***************************************************************************/
BERR_Code BV3D_P_NotifyQCreate(
   BV3D_NotifyQHandle *phNotifyQ
)
{
   BV3D_NotifyQHandle hNotifyQ;

   if (phNotifyQ == NULL)
      return BERR_INVALID_PARAMETER;

   hNotifyQ = (BV3D_NotifyQHandle)BKNI_Malloc(sizeof(BV3D_P_NotifyQHandle));
   if (hNotifyQ == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   BLST_Q_INIT(&hNotifyQ->sQueue);

   *phNotifyQ = hNotifyQ;

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_NotifyQDestroy(
   BV3D_NotifyQHandle hNotifyQ
)
{
   if (hNotifyQ == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(BLST_Q_EMPTY(&hNotifyQ->sQueue));

   BKNI_Free(hNotifyQ);

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_NotifyQPush(
   BV3D_NotifyQHandle hNotifyQ,
   uint32_t           uiClientId,
   uint64_t           uiParam,
   uint32_t           uiSync,
   uint32_t           uiOutOfMemory,
   uint64_t           uiJobSequence,
   BV3D_TimelineData  *sTimelineData
)
{
   BV3D_P_Notification *pNotification;

   if (hNotifyQ == NULL)
      return BERR_INVALID_PARAMETER;

   pNotification = (BV3D_P_Notification *)BKNI_Malloc(sizeof(BV3D_P_Notification));
   if (pNotification == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   pNotification->uiParam       = uiParam;
   pNotification->uiSync        = uiSync;
   pNotification->uiClientId    = uiClientId;
   pNotification->uiOutOfMemory = uiOutOfMemory;
   pNotification->uiJobSequence = uiJobSequence;

   if (sTimelineData != NULL)
      pNotification->sTimelineData = *sTimelineData;

   BLST_Q_INSERT_TAIL(&hNotifyQ->sQueue, pNotification, sChain);

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_NotifyQPop(
   BV3D_NotifyQHandle hNotifyQ,
   uint32_t           uiClientId,
   uint64_t           *puiParam,
   uint32_t           *puiSync,
   uint32_t           *puiOutOfMemory,
   uint64_t           *puiJobSequence,
   BV3D_TimelineData  *sTimelineData
)
{
   BV3D_P_Notification * pNotification;

   if (hNotifyQ == NULL)
      return BERR_INVALID_PARAMETER;

   pNotification = BLST_Q_FIRST(&hNotifyQ->sQueue);

   while ((pNotification != NULL) && (uiClientId != pNotification->uiClientId))
   {
      pNotification = BLST_Q_NEXT(pNotification, sChain);
   }

   if (pNotification != NULL)
   {
      if (puiParam)
         *puiParam = pNotification->uiParam;
      if (puiSync)
         *puiSync = pNotification->uiSync;
      if (puiOutOfMemory)
         *puiOutOfMemory = pNotification->uiOutOfMemory;
      if (puiJobSequence)
         *puiJobSequence = pNotification->uiJobSequence;
      if (sTimelineData)
         *sTimelineData = pNotification->sTimelineData;

      BLST_Q_REMOVE(&hNotifyQ->sQueue, pNotification, sChain);
      BKNI_Free(pNotification);
   }
   else
   {
      if (puiParam)
         *puiParam = 0;
      if (puiSync)
         *puiSync = 0;
      if (puiJobSequence)
         *puiJobSequence = 0;
   }

   return (pNotification != NULL) ? BERR_SUCCESS : BERR_INVALID_PARAMETER;
}
