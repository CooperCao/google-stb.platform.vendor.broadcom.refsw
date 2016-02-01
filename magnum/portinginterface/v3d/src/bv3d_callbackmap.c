/***************************************************************************
 *     (c)2012 Broadcom Corporation
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
#include "bv3d.h"
#include "bv3d_callbackmap_priv.h"
#include "bv3d_worker_priv.h"
#include "blst_slist.h"

BDBG_MODULE(BV3D);

typedef struct BV3D_P_LoadStats
{
   BV3D_EventTime sLastCollectedTime;
   int64_t        iRenderTimeUs;
   uint32_t       uiRenderCount;
} BV3D_P_LoadStats;


typedef struct BV3D_P_CallbackMap
{
   uint32_t    uiClientId;
   uint32_t    uiClientPID;
   void        *pContext;
   void        (*pCallback) (uint32_t, void *);
   BV3D_P_LoadStats     sLoadStats;
   BLST_S_ENTRY(BV3D_P_CallbackMap)      sChain;
} BV3D_P_CallbackMap;

typedef struct BV3D_P_CallbackMapHandle
{
   BLST_S_HEAD(sList, BV3D_P_CallbackMap) sList;
} BV3D_P_CallbackMapHandle;

/***************************************************************************/
BERR_Code BV3D_P_CallbackMapCreate(
   BV3D_CallbackMapHandle *phCallbackMap
)
{
   BV3D_CallbackMapHandle hCallbackMap;

   if (phCallbackMap == NULL)
      return BERR_INVALID_PARAMETER;

   hCallbackMap = (BV3D_CallbackMapHandle)BKNI_Malloc(sizeof(BV3D_P_CallbackMapHandle));
   if (hCallbackMap == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   BLST_S_INIT(&hCallbackMap->sList);

   *phCallbackMap = hCallbackMap;

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_CallbackMapDestroy(
   BV3D_CallbackMapHandle hCallbackMap
)
{
   if (hCallbackMap == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(BLST_S_EMPTY(&hCallbackMap->sList));

   BKNI_Free(hCallbackMap);

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_CallbackMapInsert(
   BV3D_CallbackMapHandle hCallbackMap,
   uint32_t               uiClientId,
   uint32_t               uiClientPID,
   void                   *pContext,  
   void                   (*pCallback)(uint32_t, void *)
)
{
   BV3D_P_CallbackMap * psCallbackMap;
   BV3D_EventTime       sTimeNow;

   if (hCallbackMap == NULL)
      return BERR_INVALID_PARAMETER;

   psCallbackMap = (BV3D_P_CallbackMap *)BKNI_Malloc(sizeof(BV3D_P_CallbackMap));
   if (psCallbackMap == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   BV3D_P_GetTimeNow(&sTimeNow);

   psCallbackMap->uiClientId = uiClientId;
   psCallbackMap->uiClientPID = uiClientPID;
   psCallbackMap->pContext   = pContext;
   psCallbackMap->pCallback  = pCallback;

   psCallbackMap->sLoadStats.sLastCollectedTime = sTimeNow;
   psCallbackMap->sLoadStats.iRenderTimeUs = 0;
   psCallbackMap->sLoadStats.uiRenderCount = 0;

   BLST_S_DICT_ADD(&hCallbackMap->sList, psCallbackMap, BV3D_P_CallbackMap, uiClientId, sChain, duplicate);

   return BERR_SUCCESS;
duplicate:
   BKNI_Printf("%s : FATAL : Duplicate client id inserted to map\n", __FUNCTION__);
   BKNI_Free(psCallbackMap);
   return BERR_INVALID_PARAMETER;
}

/***************************************************************************/
BERR_Code BV3D_P_CallbackMapRemove(
   BV3D_CallbackMapHandle hCallbackMap,
   uint32_t uiClientId
)
{
   BV3D_P_CallbackMap * psCallbackMap;

   if (hCallbackMap == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(!BLST_S_EMPTY(&hCallbackMap->sList));

   BLST_S_DICT_REMOVE(&hCallbackMap->sList, psCallbackMap, uiClientId, BV3D_P_CallbackMap, uiClientId, sChain);

   if (psCallbackMap == NULL)
      return BERR_INVALID_PARAMETER;

   BKNI_Free(psCallbackMap);

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_CallbackMapGet(
   BV3D_CallbackMapHandle hCallbackMap,
   uint32_t               uiClientId,
   uint32_t               *uiClientPID,
   void                   **ppContext,
   void                   (**ppCallback)(uint32_t, void *)
)
{
   BV3D_P_CallbackMap * psCallbackMap;

   if (hCallbackMap == NULL)
      return BERR_INVALID_PARAMETER;

   if (uiClientPID == NULL)
      return BERR_INVALID_PARAMETER;

   if (ppContext == NULL)
      return BERR_INVALID_PARAMETER;

   if (ppCallback == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(!BLST_S_EMPTY(&hCallbackMap->sList));

   BLST_S_DICT_FIND(&hCallbackMap->sList, psCallbackMap, uiClientId, uiClientId, sChain);

   BDBG_ASSERT(psCallbackMap != NULL);

   *uiClientPID = psCallbackMap->uiClientPID;
   *ppContext   = psCallbackMap->pContext;
   *ppCallback  = psCallbackMap->pCallback;

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_CallbackMapGetSize(
   BV3D_CallbackMapHandle hCallbackMap,
   uint32_t               *pNumClients
)
{
   BV3D_P_CallbackMap * psCallbackMap;

   if (hCallbackMap == NULL)
      return BERR_INVALID_PARAMETER;

   if (pNumClients == NULL)
      return BERR_INVALID_PARAMETER;

   *pNumClients = 0;

   for (psCallbackMap = BLST_S_FIRST(&hCallbackMap->sList); psCallbackMap != NULL; psCallbackMap = BLST_S_NEXT(psCallbackMap, sChain))
      (*pNumClients)++;

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_CallbackMapGetStats(
   BV3D_CallbackMapHandle hCallbackMap,
   BV3D_Instruction       *psRenderInstr,
   BV3D_ClientLoadData    *pLoadData,
   uint32_t               uiNumClients,
   uint32_t               *pValidClients,
   bool                   bReset
)
{
   BV3D_P_CallbackMap *psCallbackMap;
   BV3D_EventTime      sTimeNow;
   uint32_t            i = 0;

   if (pLoadData == NULL || pValidClients == NULL)
      return BERR_INVALID_PARAMETER;

   BV3D_P_GetTimeNow(&sTimeNow);

   for (psCallbackMap = BLST_S_FIRST(&hCallbackMap->sList); psCallbackMap != NULL; psCallbackMap = BLST_S_NEXT(psCallbackMap, sChain))
   {
      uint64_t uiElapsedUs;
      uint64_t uiCurRenderTime = 0;

      if (i >= uiNumClients)
         break;

      if (psRenderInstr->psJob && psRenderInstr->psJob->uiClientId == psCallbackMap->uiClientId)
      {
         /* There is a render job in progress for this client. Account for it's elapsed time too. */
         uiCurRenderTime = ((uint64_t)sTimeNow.uiSecs * 1000000 + sTimeNow.uiMicrosecs) -
                           ((uint64_t)psRenderInstr->psJob->sTimelineData.sRenderStart.uiSecs * 1000000 +
                            psRenderInstr->psJob->sTimelineData.sRenderStart.uiMicrosecs);
      }

      uiElapsedUs = (sTimeNow.uiSecs * 1000000 + sTimeNow.uiMicrosecs) -
                    (psCallbackMap->sLoadStats.sLastCollectedTime.uiSecs * 1000000 + psCallbackMap->sLoadStats.sLastCollectedTime.uiMicrosecs);

      pLoadData[i].uiClientId = psCallbackMap->uiClientId;
      pLoadData[i].uiClientPID = psCallbackMap->uiClientPID;
      pLoadData[i].uiNumRenders = psCallbackMap->sLoadStats.uiRenderCount;
      pLoadData[i].sRenderPercent = (uint8_t)((psCallbackMap->sLoadStats.iRenderTimeUs + uiCurRenderTime) * 100 / uiElapsedUs);

      if (bReset)
      {
         psCallbackMap->sLoadStats.sLastCollectedTime = sTimeNow;
         psCallbackMap->sLoadStats.iRenderTimeUs = 0;
         psCallbackMap->sLoadStats.uiRenderCount = 0;
      }

      /* As we've already reported this partial render time, we need to make sure we don't count it again next time */
      psCallbackMap->sLoadStats.iRenderTimeUs -= uiCurRenderTime;

      i++;
   }

   *pValidClients = i;

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_CallbackMapUpdateStats(
   BV3D_CallbackMapHandle hCallbackMap,
   uint32_t               uiClientId,
   uint64_t               uiTimeInRendererUs
   )
{
   BV3D_P_CallbackMap *psCallbackMap;

   if (hCallbackMap == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(!BLST_S_EMPTY(&hCallbackMap->sList));

   BLST_S_DICT_FIND(&hCallbackMap->sList, psCallbackMap, uiClientId, uiClientId, sChain);
   if (psCallbackMap == NULL)
      return BERR_INVALID_PARAMETER;

   psCallbackMap->sLoadStats.uiRenderCount += 1;
   psCallbackMap->sLoadStats.iRenderTimeUs += uiTimeInRendererUs;

   return BERR_SUCCESS;
}

/***************************************************************************/
