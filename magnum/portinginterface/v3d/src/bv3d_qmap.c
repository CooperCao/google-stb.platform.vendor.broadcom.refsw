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
#include "bv3d_iq_priv.h"
#include "bv3d_qmap_priv.h"
#include "blst_slist.h"

typedef struct BV3D_P_IQMap
{
   uint32_t uiClientId;
   BV3D_IQHandle hIQ;
   BLST_S_ENTRY(BV3D_P_IQMap)      sChain;
} BV3D_P_IQMap;

typedef struct BV3D_P_IQMapHandle
{
   uint32_t uiSize;
   BLST_S_HEAD(sList, BV3D_P_IQMap) sList;
} BV3D_P_IQMapHandle;

/***************************************************************************/
BERR_Code BV3D_P_IQMapCreate(
   BV3D_IQMapHandle *phIQMap
)
{
   BV3D_IQMapHandle hIQMap;

   if (phIQMap == NULL)
      return BERR_INVALID_PARAMETER;

   hIQMap = (BV3D_IQMapHandle)BKNI_Malloc(sizeof(BV3D_P_IQMapHandle));
   if (hIQMap == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   BLST_S_INIT(&hIQMap->sList);

   hIQMap->uiSize = 0;

   *phIQMap = hIQMap;

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_IQMapDestroy(
   BV3D_IQMapHandle hIQMap
)
{
   if (hIQMap == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(BLST_S_EMPTY(&hIQMap->sList));

   BKNI_Free(hIQMap);

   return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BV3D_P_IQMapInsert(
   BV3D_IQMapHandle hIQMap,
   BV3D_IQHandle hIQ,
   uint32_t uiClientId
)
{
   BV3D_P_IQMap * psIQMap;

   if (hIQMap == NULL)
      return BERR_INVALID_PARAMETER;

   psIQMap = (BV3D_P_IQMap *)BKNI_Malloc(sizeof(BV3D_P_IQMap));
   if (psIQMap == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   psIQMap->hIQ = hIQ;
   psIQMap->uiClientId = uiClientId;

   hIQMap->uiSize += 1;

   BLST_S_DICT_ADD(&hIQMap->sList, psIQMap, BV3D_P_IQMap, uiClientId, sChain, duplicate);

   return BERR_SUCCESS;
duplicate:
   BKNI_Printf("%s : FATAL : Duplicate client id inserted to map\n", __FUNCTION__);
   BKNI_Free(psIQMap);
   return BERR_INVALID_PARAMETER;
}

/***************************************************************************/
BV3D_IQHandle BV3D_P_IQMapGet(
   BV3D_IQMapHandle hIQMap,
   uint32_t uiClientId
)
{
   BV3D_P_IQMap *psIQMap;

   if (hIQMap == NULL)
      return NULL;

   BDBG_ASSERT(!BLST_S_EMPTY(&hIQMap->sList));

   BLST_S_DICT_FIND(&hIQMap->sList, psIQMap, uiClientId, uiClientId, sChain);

   BDBG_ASSERT(psIQMap != NULL);

   return (psIQMap) ? psIQMap->hIQ : NULL;
}

/***************************************************************************/
BERR_Code BV3D_P_IQMapRemove(
   BV3D_IQMapHandle hIQMap,
   uint32_t uiClientId
)
{
   BV3D_P_IQMap *psIQMap;

   if (hIQMap == NULL)
      return BERR_INVALID_PARAMETER;

   BDBG_ASSERT(!BLST_S_EMPTY(&hIQMap->sList));

   BLST_S_DICT_REMOVE(&hIQMap->sList, psIQMap, uiClientId, BV3D_P_IQMap, uiClientId, sChain);

   if (psIQMap == NULL)
      return BERR_INVALID_PARAMETER;

   BKNI_Free(psIQMap);

   hIQMap->uiSize -= 1;

   return BERR_SUCCESS;
}

/***************************************************************************/
BV3D_IQHandle BV3D_P_IQMapFirst(
   BV3D_IQMapHandle hIQMap,
   void ** ppNext
)
{
   BV3D_P_IQMap *psIQMap;

   if ((hIQMap == NULL) || (ppNext == NULL))
      return NULL;

   psIQMap = BLST_S_FIRST(&hIQMap->sList);

   *ppNext = (void *)psIQMap;

   return (psIQMap) ? psIQMap->hIQ : NULL;
}

/***************************************************************************/
BV3D_IQHandle BV3D_P_IQMapNext(
   BV3D_IQMapHandle hIQMap,
   void ** ppNext
)
{
   BV3D_P_IQMap * psIQMap;

   if ((hIQMap == NULL) || (ppNext == NULL))
      return NULL;

   psIQMap = *(BV3D_P_IQMap **)ppNext;
   if (psIQMap == NULL)
      return NULL;

   psIQMap = BLST_S_NEXT(psIQMap, sChain);

   *ppNext = (void *)psIQMap;

   return (psIQMap) ? psIQMap->hIQ : NULL;
}

/***************************************************************************/
uint32_t BV3D_P_IQMapFirstKey(
   BV3D_IQMapHandle hIQMap,
   void ** ppNext
)
{
   BV3D_P_IQMap * psIQMap;

   if ((hIQMap == NULL) || (ppNext == NULL))
      return (uint32_t)~0;

   psIQMap = BLST_S_FIRST(&hIQMap->sList);

   *ppNext = (void *)psIQMap;

   return (psIQMap) ? psIQMap->uiClientId : (uint32_t)~0;
}

/***************************************************************************/
uint32_t BV3D_P_IQMapNextKey(
   BV3D_IQMapHandle hIQMap,
   void ** ppNext
)
{
   BV3D_P_IQMap * psIQMap;

   if ((hIQMap == NULL) || (ppNext == NULL))
      return (uint32_t)~0;

   psIQMap = *(BV3D_P_IQMap **)ppNext;
   if (psIQMap == NULL)
      return (uint32_t)~0;

   psIQMap = BLST_S_NEXT(psIQMap, sChain);

   *ppNext = (void *)psIQMap;

   return (psIQMap) ? psIQMap->uiClientId : (uint32_t)~0;
}

/***************************************************************************/
uint32_t BV3D_P_IQMapSize(
   BV3D_IQMapHandle hIQMap
)
{
   return (hIQMap) ? hIQMap->uiSize : 0;
}
