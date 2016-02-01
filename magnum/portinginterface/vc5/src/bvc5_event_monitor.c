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

#include "bstd.h"
#include "bvc5.h"
#include "bvc5_priv.h"
#include "bvc5_event_monitor_priv.h"

BDBG_MODULE(BVC5);

static void _strncpy(char *d, const char *s, uint32_t maxLen)
{
   uint32_t i;
   for (i = 0; i < maxLen - 1 && *s != 0; i++)
      *d++ = *s++;

   *d = '\0';
}

static void BVC5_P_SetEventTrack(
   BVC5_P_EventMonitor    *ev,
   uint32_t               track,
   const char             *name
   )
{
   _strncpy(ev->sTrackDescs[track].caName, name, BVC5_MAX_EVENT_STRING_LEN);

   if (track >= ev->uiNumTracks)
      ev->uiNumTracks = track + 1;
}

static void BVC5_P_SetCoreEventTrack(
   BVC5_P_EventMonitor    *ev,
   uint32_t               core,
   uint32_t               track,
   const char             *name
   )
{
   BDBG_ASSERT(core < 10);

   _strncpy(ev->sTrackDescs[track].caName, "Core   ", 8);
   ev->sTrackDescs[track].caName[5] = core + '0';
   _strncpy(&ev->sTrackDescs[track].caName[7], name, BVC5_MAX_EVENT_STRING_LEN - 7);

   if (track >= ev->uiNumTracks)
      ev->uiNumTracks = track + 1;
}

static void BVC5_P_SetEvent(
   BVC5_P_EventMonitor    *ev,
   uint32_t               event,
   const char             *name,
   uint32_t                numFields
   )
{
   _strncpy(ev->sEventDescs[event].sDesc.caName, name, BVC5_MAX_EVENT_STRING_LEN);
   ev->sEventDescs[event].sDesc.uiNumDataFields = numFields;
}

static void BVC5_P_SetEventDesc(
   BVC5_P_EventDesc    *eventDesc,
   uint32_t             id,
   BVC5_FieldType       type,
   const char          *name
   )
{
   BVC5_EventFieldDesc  *field = &eventDesc->sFieldDescs[id];

   field->eDataType = type;
   _strncpy(field->caName, name, BVC5_MAX_EVENT_STRING_LEN);
}

static void BVC5_P_SetEvent3(
   BVC5_P_EventMonitor   *ev,
   uint32_t               event,
   const char            *name,
   BVC5_FieldType         ftype1,
   const char            *fname1,
   BVC5_FieldType         ftype2,
   const char            *fname2,
   BVC5_FieldType         ftype3,
   const char            *fname3
   )
{
   BVC5_P_EventDesc  *eventDesc = &ev->sEventDescs[event];

   uint32_t numFields = 3;
   uint32_t i         = 0;

   BVC5_P_SetEvent(ev, event, name, numFields);

   BVC5_P_SetEventDesc(eventDesc, i++, ftype1, fname1);
   BVC5_P_SetEventDesc(eventDesc, i++, ftype2, fname2);
   BVC5_P_SetEventDesc(eventDesc, i++, ftype3, fname3);

   BDBG_ASSERT(i == numFields && i <= BVC5_P_EVENT_MONITOR_MAX_FIELDS);
}

static void BVC5_P_SetEvents(
   BVC5_P_EventMonitor    *ev,
   uint32_t               event,
   const char             *name,
   uint32_t               extraCount
   )
{
   const uint32_t     numFields = 2 + extraCount;
   BVC5_P_EventDesc  *eventDesc = &ev->sEventDescs[event];

   uint32_t           i = 0;
   uint32_t           n;

   BVC5_P_SetEvent(ev, event, name, numFields);

   BVC5_P_SetEventDesc(eventDesc, i++, BVC5_EventUInt32, "ClientID");
   BVC5_P_SetEventDesc(eventDesc, i++, BVC5_EventUInt64, "JobID");

   for (n = 0; n < extraCount; ++n)
      BVC5_P_SetEventDesc(eventDesc, i++, BVC5_EventUInt64, "Dep");

   BDBG_ASSERT(i == numFields && i <= BVC5_P_EVENT_MONITOR_MAX_FIELDS);
}

static void BVC5_P_SetEventTFU(
   BVC5_P_EventMonitor    *ev,
   uint32_t               extraCount
   )
{
   const uint32_t     numFields = 7 + extraCount;
   BVC5_P_EventDesc  *eventDesc = &ev->sEventDescs[BVC5_P_EVENT_MONITOR_TFU];

   uint32_t           i = 0;
   uint32_t           n;

   BVC5_P_SetEvent(ev, BVC5_P_EVENT_MONITOR_TFU, "TFU", numFields);

   BVC5_P_SetEventDesc(eventDesc, i++, BVC5_EventUInt32, "ClientID");
   BVC5_P_SetEventDesc(eventDesc, i++, BVC5_EventUInt64, "JobID");
   BVC5_P_SetEventDesc(eventDesc, i++, BVC5_EventUInt32, "Width");
   BVC5_P_SetEventDesc(eventDesc, i++, BVC5_EventUInt32, "Height");
   BVC5_P_SetEventDesc(eventDesc, i++, BVC5_EventUInt32, "Stride");
   BVC5_P_SetEventDesc(eventDesc, i++, BVC5_EventUInt32, "Mipmaps");
   BVC5_P_SetEventDesc(eventDesc, i++, BVC5_EventUInt32, "TType");

   for (n = 0; n < extraCount; ++n)
      BVC5_P_SetEventDesc(eventDesc, i++, BVC5_EventUInt64, "Dep");

   BDBG_ASSERT(i == numFields && i <= BVC5_P_EVENT_MONITOR_MAX_FIELDS);
}

/***************************************************************************/
void BVC5_P_InitEventMonitor(
   BVC5_Handle  hVC5
   )
{
   uint32_t core;
   uint32_t numDeps = hVC5->sOpenParams.bGPUMonDeps ? BVC5_MAX_DEPENDENCIES : 0;

   BKNI_AcquireMutex(hVC5->hEventMutex);

   BKNI_Memset(&hVC5->sEventMonitor, 0, sizeof(hVC5->sEventMonitor));

   BVC5_P_SetEventTrack(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_SCHED_TRACK, "Scheduler");
   BVC5_P_SetEventTrack(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_TFU_TRACK,   "TFU");

   BVC5_P_SetEvents(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_PROXY, "Proxy", numDeps);

   for (core = 0; core < hVC5->uiNumCores; core++)
   {
      uint32_t start;

      start = BVC5_P_EVENT_MONITOR_NON_CORE_TRACKS + core * BVC5_P_EVENT_MONITOR_NUM_CORE_TRACKS;

      BVC5_P_SetCoreEventTrack(&hVC5->sEventMonitor, core, start + BVC5_P_EVENT_MONITOR_CORE_BIN_TRACK,    "Binner");
      BVC5_P_SetCoreEventTrack(&hVC5->sEventMonitor, core, start + BVC5_P_EVENT_MONITOR_CORE_RENDER_TRACK, "Renderer");
   }

   BVC5_P_SetEvents(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_BINNING,   "Binning",   numDeps);
   BVC5_P_SetEvents(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_RENDERING, "Rendering", numDeps);

   BVC5_P_SetEventTFU(&hVC5->sEventMonitor, numDeps);

   BVC5_P_SetEvent(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_BOOM, "Bin OOM", 0);
   BVC5_P_SetEvent(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_BIN_LOCKUP, "Bin Lockup", 0);
   BVC5_P_SetEvent(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_RENDER_LOCKUP, "Render Lockup", 0);
   BVC5_P_SetEvent(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_TFU_LOCKUP, "TFU Lockup", 0);
   BVC5_P_SetEvent(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_POWER_UP, "Power Up", 0);
   BVC5_P_SetEvent(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_POWER_DOWN, "Power Down", 0);

   BVC5_P_SetEvent3(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_FLUSH_VC5, "Flush VC5 Caches",
                    BVC5_EventUInt32, "L3",
                    BVC5_EventUInt32, "L2C/Slice",
                    BVC5_EventUInt32, "L2T");

   BVC5_P_SetEvent(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_FLUSH_CPU, "Flush CPU Cache", 0);
   BVC5_P_SetEvent(&hVC5->sEventMonitor, BVC5_P_EVENT_MONITOR_PART_FLUSH_CPU, "Partial Flush CPU Cache", 0);

   BKNI_ReleaseMutex(hVC5->hEventMutex);
}

/***************************************************************************/
void BVC5_GetEventCounts(
   BVC5_Handle  hVC5,
   uint32_t     *uiNumTracks,
   uint32_t     *uiNumEvents
   )
{
   BDBG_ENTER(BVC5_GetEventCounts);
   BKNI_AcquireMutex(hVC5->hModuleMutex);
   BKNI_AcquireMutex(hVC5->hEventMutex);

   if (uiNumTracks)
      *uiNumTracks = hVC5->sEventMonitor.uiNumTracks;

   if (uiNumEvents)
      *uiNumEvents = BVC5_P_EVENT_MONITOR_NUM_EVENTS;

   BKNI_ReleaseMutex(hVC5->hEventMutex);
   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_GetEventCounts);
}

BERR_Code BVC5_GetEventTrackInfo(
   BVC5_Handle           hVC5,
   uint32_t              uiTrack,
   BVC5_EventTrackDesc   *psTrackDesc
   )
{
   BERR_Code err = BERR_SUCCESS;

   BDBG_ENTER(BVC5_GetEventTrackInfo);
   BKNI_AcquireMutex(hVC5->hModuleMutex);
   BKNI_AcquireMutex(hVC5->hEventMutex);

   if (uiTrack < hVC5->sEventMonitor.uiNumTracks)
      _strncpy(psTrackDesc->caName, hVC5->sEventMonitor.sTrackDescs[uiTrack].caName, BVC5_MAX_EVENT_STRING_LEN);
   else
      err = BERR_INVALID_PARAMETER;

   BKNI_ReleaseMutex(hVC5->hEventMutex);
   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_GetEventTrackInfo);

   return err;
}

BERR_Code BVC5_GetEventInfo(
   BVC5_Handle       hVC5,
   uint32_t          uiEvent,
   BVC5_EventDesc   *psEventDesc
   )
{
   BERR_Code err = BERR_SUCCESS;

   BDBG_ENTER(BVC5_GetEventInfo);
   BKNI_AcquireMutex(hVC5->hModuleMutex);
   BKNI_AcquireMutex(hVC5->hEventMutex);

   if (uiEvent < BVC5_P_EVENT_MONITOR_NUM_EVENTS)
   {
      psEventDesc->uiNumDataFields = hVC5->sEventMonitor.sEventDescs[uiEvent].sDesc.uiNumDataFields;
      _strncpy(psEventDesc->caName, hVC5->sEventMonitor.sEventDescs[uiEvent].sDesc.caName, BVC5_MAX_EVENT_STRING_LEN);
   }
   else
      err = BERR_INVALID_PARAMETER;

   BKNI_ReleaseMutex(hVC5->hEventMutex);
   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_GetEventInfo);

   return err;
}

BERR_Code BVC5_GetEventDataFieldInfo(
   BVC5_Handle          hVC5,
   uint32_t             uiEvent,
   uint32_t             uiField,
   BVC5_EventFieldDesc  *psFieldDesc
   )
{
   BERR_Code err = BERR_SUCCESS;

   BDBG_ENTER(BVC5_GetEventDataFieldInfo);
   BKNI_AcquireMutex(hVC5->hModuleMutex);
   BKNI_AcquireMutex(hVC5->hEventMutex);

   if (uiEvent < BVC5_P_EVENT_MONITOR_NUM_EVENTS)
   {
      if (uiField < hVC5->sEventMonitor.sEventDescs[uiEvent].sDesc.uiNumDataFields)
      {
         psFieldDesc->eDataType = hVC5->sEventMonitor.sEventDescs[uiEvent].sFieldDescs[uiField].eDataType;
         _strncpy(psFieldDesc->caName, hVC5->sEventMonitor.sEventDescs[uiEvent].sFieldDescs[uiField].caName,
                  BVC5_MAX_EVENT_STRING_LEN);
      }
      else
         err = BERR_INVALID_PARAMETER;
   }
   else
      err = BERR_INVALID_PARAMETER;

   BKNI_ReleaseMutex(hVC5->hEventMutex);
   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_GetEventDataFieldInfo);

   return err;
}

static void BVC5_P_CreateEventBuffer(
   BVC5_Handle hVC5
   )
{
   if (hVC5->sEventMonitor.sBuffer.uiCapacityBytes == 0)
   {
      hVC5->sEventMonitor.sBuffer.pBuffer = BKNI_Malloc(BVC5_P_EVENT_BUFFER_BYTES);
      if (hVC5->sEventMonitor.sBuffer.pBuffer != NULL)
      {
         hVC5->sEventMonitor.sBuffer.uiCapacityBytes = BVC5_P_EVENT_BUFFER_BYTES;
         hVC5->sEventMonitor.sBuffer.uiBytesUsed = 0;
         hVC5->sEventMonitor.sBuffer.pRead = hVC5->sEventMonitor.sBuffer.pBuffer;
         hVC5->sEventMonitor.sBuffer.pWrite = hVC5->sEventMonitor.sBuffer.pBuffer;
         hVC5->sEventMonitor.sBuffer.bOverflow = false;
      }
   }
}

static void BVC5_P_DeleteEventBuffer(
   BVC5_Handle hVC5)
{
   if (hVC5->sEventMonitor.sBuffer.pBuffer != NULL)
   {
      BKNI_Free(hVC5->sEventMonitor.sBuffer.pBuffer);
      hVC5->sEventMonitor.sBuffer.pBuffer = NULL;
      hVC5->sEventMonitor.sBuffer.uiCapacityBytes = 0;
      hVC5->sEventMonitor.sBuffer.uiBytesUsed = 0;
      hVC5->sEventMonitor.sBuffer.pRead = NULL;
      hVC5->sEventMonitor.sBuffer.pWrite = NULL;
      hVC5->sEventMonitor.sBuffer.bOverflow = false;
   }
}

static BERR_Code BVC5_P_SetEventCollection(
   BVC5_Handle       hVC5,
   uint32_t          uiClientId,
   BVC5_EventState   eState
   )
{
   BERR_Code err = BERR_SUCCESS;

   BKNI_AcquireMutex(hVC5->hEventMutex);

   switch (eState)
   {
   case BVC5_EventAcquire:
      if (hVC5->sEventMonitor.bAcquired)
      {
         if (hVC5->sEventMonitor.uiClientId != uiClientId)
         {
            err = BERR_NOT_AVAILABLE;
            goto error;
         }
      }
      else
      {
         BVC5_P_CreateEventBuffer(hVC5);
         if (hVC5->sEventMonitor.sBuffer.pBuffer == NULL)
         {
            err = BERR_OUT_OF_SYSTEM_MEMORY;
            goto error;
         }

         hVC5->sEventMonitor.bAcquired = true;
         hVC5->sEventMonitor.uiClientId = uiClientId;
      }

      break;
   case BVC5_EventRelease:
      if (hVC5->sEventMonitor.bAcquired)
      {
         BVC5_P_DeleteEventBuffer(hVC5);
         hVC5->sEventMonitor.bAcquired = false;
         hVC5->sEventMonitor.uiClientId = 0;
      }
      break;
   case BVC5_EventStart:
      if (hVC5->sEventMonitor.bAcquired && hVC5->sEventMonitor.uiClientId == uiClientId)
         hVC5->sEventMonitor.bActive = true;
      else
         err = BERR_NOT_AVAILABLE;
      break;
   case BVC5_EventStop:
      if (hVC5->sEventMonitor.bAcquired && hVC5->sEventMonitor.uiClientId == uiClientId)
         hVC5->sEventMonitor.bActive = false;
      else
         err = BERR_NOT_AVAILABLE;
      break;
   default:
      err = BERR_INVALID_PARAMETER;
      break;
   }

error:
   BKNI_ReleaseMutex(hVC5->hEventMutex);

   return err;
}

BERR_Code BVC5_SetEventCollection(
   BVC5_Handle       hVC5,
   uint32_t          uiClientId,
   BVC5_EventState   eState
   )
{
   BERR_Code err;

   BDBG_ENTER(BVC5_SetEventCollection);
   BKNI_AcquireMutex(hVC5->hModuleMutex);

   err = BVC5_P_SetEventCollection(hVC5, uiClientId, eState);

   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_SetEventCollection);

   return err;
}

uint32_t BVC5_GetEventData(
   BVC5_Handle    hVC5,
   uint32_t       uiClientId,
   uint32_t       uiEventBufferBytes,
   void           *pvEventBuffer,
   uint32_t       *puiLostData,
   uint64_t       *puiTimeStamp
   )
{
   BVC5_P_EventBuffer *psBuf = &hVC5->sEventMonitor.sBuffer;
   uint32_t           bytes  = 0;

   BDBG_ENTER(BVC5_GetEventData);
   BKNI_AcquireMutex(hVC5->hModuleMutex);
   BKNI_AcquireMutex(hVC5->hEventMutex);

   if (!hVC5->sEventMonitor.bAcquired || hVC5->sEventMonitor.uiClientId != uiClientId)
      goto error;

   if (puiLostData)
      *puiLostData = psBuf->bOverflow;

   if (uiEventBufferBytes == 0 || pvEventBuffer == NULL)
   {
      bytes = psBuf->uiBytesUsed;
   }
   else
   {
      /* Copy the data out */
      uint32_t    bytesToCopy, bytesToEnd;
      uintptr_t   end = (uintptr_t)psBuf->pBuffer + psBuf->uiCapacityBytes;

      bytes = bytesToCopy = psBuf->uiBytesUsed;
      if (uiEventBufferBytes < bytesToCopy)
         bytesToCopy = uiEventBufferBytes;

      bytesToEnd = end - (uintptr_t)psBuf->pRead;
      if (bytesToEnd > bytesToCopy)
      {
         /* No wrapping */
         BKNI_Memcpy(pvEventBuffer, psBuf->pRead, bytesToCopy);
         psBuf->pRead = (void*)((uintptr_t)psBuf->pRead + bytesToCopy);
         psBuf->uiBytesUsed -= bytesToCopy;
      }
      else
      {
         /* Buffer wraps */
         BKNI_Memcpy(pvEventBuffer, psBuf->pRead, bytesToEnd);
         psBuf->pRead = psBuf->pBuffer;
         psBuf->uiBytesUsed -= bytesToEnd;
         pvEventBuffer = (void*)((uintptr_t)pvEventBuffer + bytesToEnd);
         bytesToCopy -= bytesToEnd;

         BKNI_Memcpy(pvEventBuffer, psBuf->pRead, bytesToCopy);
         psBuf->pRead = (void*)((uintptr_t)psBuf->pRead + bytesToCopy);
         psBuf->uiBytesUsed -= bytesToCopy;
      }
   }

error:
   if (puiTimeStamp)
      BVC5_P_GetTime_isrsafe(puiTimeStamp);


   BKNI_ReleaseMutex(hVC5->hEventMutex);
   BKNI_ReleaseMutex(hVC5->hModuleMutex);
   BDBG_LEAVE(BVC5_GetEventData);

   return bytes;
}

/* You must have a lock on hEventMutex before calling this */
static bool BVC5_P_Add32(
   BVC5_Handle    hVC5,
   uint32_t       uiData
   )
{
   BVC5_P_EventBuffer *psBuf = &hVC5->sEventMonitor.sBuffer;
   uintptr_t          end = (uintptr_t)psBuf->pBuffer + psBuf->uiCapacityBytes;
   uint32_t           bytesToEnd = end - (uintptr_t)psBuf->pWrite;

   if (psBuf->uiCapacityBytes - psBuf->uiBytesUsed < 4)
      return false;

   if (bytesToEnd < 4)
   {
      BDBG_ASSERT(bytesToEnd == 0);
      psBuf->pWrite = psBuf->pBuffer;
   }

   *((uint32_t*)psBuf->pWrite) = uiData;
   psBuf->pWrite = (void*)((uintptr_t)psBuf->pWrite + 4);
   psBuf->uiBytesUsed += 4;

   return true;
}

/* You must have a lock on hEventMutex before calling this */
static bool BVC5_P_Add64(
   BVC5_Handle    hVC5,
   uint64_t       uiData
   )
{
   bool               ok = true;

   ok = ok && BVC5_P_Add32(hVC5, (uint32_t)uiData);
   ok = ok && BVC5_P_Add32(hVC5, (uint32_t)(uiData >> 32));

   return ok;
}

static bool AddEvent(
   BVC5_Handle    hVC5,
   uint32_t       uiTrackIndex,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BVC5_EventType eEventType,
   bool           bAlreadyHaveEventMutex
   )
{
   BVC5_P_EventBuffer *psBuf = &hVC5->sEventMonitor.sBuffer;
   bool               ok = true;
   uint64_t           now;

   if (!bAlreadyHaveEventMutex)
      BKNI_AcquireMutex(hVC5->hEventMutex);

   if (psBuf->uiCapacityBytes - psBuf->uiBytesUsed < 24)
   {
      /* No room for this event */
      psBuf->bOverflow = true;
      ok = false;
      goto error;
   }

   BVC5_P_GetTime_isrsafe(&now);
   ok = ok && BVC5_P_Add64(hVC5, now);
   ok = ok && BVC5_P_Add32(hVC5, uiTrackIndex);
   ok = ok && BVC5_P_Add32(hVC5, uiId);
   ok = ok && BVC5_P_Add32(hVC5, uiEventIndex);
   ok = ok && BVC5_P_Add32(hVC5, eEventType);

   BDBG_ASSERT(ok); /* Should be ok as we check for space above */

   if (!ok)
      psBuf->bOverflow = true;

error:
   if (!bAlreadyHaveEventMutex)
      BKNI_ReleaseMutex(hVC5->hEventMutex);

   return ok;
}

static bool BVC5_P_AddEvent_priv(
   BVC5_Handle    hVC5,
   uint32_t       uiTrack,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BVC5_EventType eEventType,
   bool           bAlreadyHaveEventMutex
   )
{
   bool ret = true;

   if (!hVC5->sEventMonitor.bActive)
      return ret;

   if (!bAlreadyHaveEventMutex)
      BKNI_AcquireMutex(hVC5->hEventMutex);

   ret = AddEvent(hVC5, uiTrack, uiId, uiEventIndex, eEventType, /*bAlreadyHaveEventMutex=*/true);

   if (!bAlreadyHaveEventMutex)
      BKNI_ReleaseMutex(hVC5->hEventMutex);

   return ret;
}

bool BVC5_P_AddEvent(
   BVC5_Handle    hVC5,
   uint32_t       uiTrack,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BVC5_EventType eEventType
   )
{
   return BVC5_P_AddEvent_priv(hVC5, uiTrack, uiId, uiEventIndex, eEventType, /*bAlreadyHaveEventMutex=*/false);
}

static bool BVC5_P_AddDeps(
   BVC5_Handle             hVC5,
   BVC5_SchedDependencies *psDeps
   )
{
   bool  ok = true;

   if (psDeps != NULL)
   {
      uint32_t  n;

      for (n = 0; n < BVC5_MAX_DEPENDENCIES && ok; ++n)
      {
         uint64_t dep = n < psDeps->uiNumDeps ? psDeps->uiDep[n] : 0;

         ok = ok && BVC5_P_Add64(hVC5, dep);
      }
   }

   return ok;
}

bool BVC5_P_AddFlushEvent(
   BVC5_Handle    hVC5,
   uint32_t       uiId,
   BVC5_EventType eEventType,
   bool           clearL3,
   bool           clearL2C,
   bool           clearL2T
   )
{
   bool  ok = true;

   BVC5_P_EventBuffer  *psBuf   = &hVC5->sEventMonitor.sBuffer;

   if (!hVC5->sEventMonitor.bActive)
      return ok;

   BKNI_AcquireMutex(hVC5->hEventMutex);

   if (psBuf->uiCapacityBytes - psBuf->uiBytesUsed < 36)
   {
       /* No room for this event */
      psBuf->bOverflow = true;
      ok = false;
      goto error;
   }

   ok = BVC5_P_AddEvent_priv(hVC5, BVC5_P_EVENT_MONITOR_SCHED_TRACK, uiId, BVC5_P_EVENT_MONITOR_FLUSH_VC5, eEventType, /*bAlreadyHaveEventMutex=*/true);

   ok = ok && BVC5_P_Add32(hVC5, (uint32_t)clearL3);
   ok = ok && BVC5_P_Add32(hVC5, (uint32_t)clearL2C);
   ok = ok && BVC5_P_Add32(hVC5, (uint32_t)clearL2T);

error:
   BKNI_ReleaseMutex(hVC5->hEventMutex);

   return ok;
}


bool BVC5_P_AddTFUJobEvent(
   BVC5_Handle          hVC5,
   BVC5_EventType       eEventType,
   BVC5_P_InternalJob  *psJob
   )
{
   bool  ok = true;

   if (!hVC5->sEventMonitor.bActive)
      return true;

   if (psJob == NULL)
      return false;

   {
      BVC5_P_EventBuffer  *psBuf   = &hVC5->sEventMonitor.sBuffer;
      BVC5_JobTFU         *pTFUJob = (BVC5_JobTFU *)psJob->pBase;
      uint32_t             depsPad = hVC5->sOpenParams.bGPUMonDeps ? BVC5_MAX_DEPENDENCIES * sizeof(uint64_t) : 0;
      uint64_t             uiJobId = psJob->uiJobId;

      BKNI_AcquireMutex(hVC5->hEventMutex);

      if (psBuf->uiCapacityBytes - psBuf->uiBytesUsed < (60 + depsPad))
      {
          /* No room for this event */
         psBuf->bOverflow = true;
         ok = false;
         goto error;
      }

      ok = BVC5_P_AddEvent_priv(hVC5, BVC5_P_EVENT_MONITOR_TFU_TRACK, (uint32_t)uiJobId, BVC5_P_EVENT_MONITOR_TFU, eEventType, /*bAlreadyHaveEventMutex=*/true);

      ok = ok && BVC5_P_Add32(hVC5, psJob->uiClientId);
      ok = ok && BVC5_P_Add64(hVC5, uiJobId);
      ok = ok && BVC5_P_Add32(hVC5, pTFUJob->sOutput.uiWidth);
      ok = ok && BVC5_P_Add32(hVC5, pTFUJob->sOutput.uiHeight);
      ok = ok && BVC5_P_Add32(hVC5, pTFUJob->sInput.uiRasterStride);
      ok = ok && BVC5_P_Add32(hVC5, pTFUJob->sOutput.uiMipmapCount);
      ok = ok && BVC5_P_Add32(hVC5, pTFUJob->sInput.uiTextureType);
      ok = ok && BVC5_P_AddDeps(hVC5, hVC5->sOpenParams.bGPUMonDeps ? &psJob->pBase->sCompletedDependencies : NULL);

      BDBG_ASSERT(ok); /* Should be ok as we check for space above */

      if (!ok)
         psBuf->bOverflow = true;
   }

error:
   BKNI_ReleaseMutex(hVC5->hEventMutex);

   return ok;
}

bool BVC5_P_AddCoreEvent_priv(
   BVC5_Handle    hVC5,
   uint32_t       uiCore,
   uint32_t       uiTrack,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BVC5_EventType eEventType,
   bool           bAlreadyHaveEventMutex
   )
{
   uint32_t  uiTrackIndex = uiTrack + BVC5_P_EVENT_MONITOR_NON_CORE_TRACKS + BVC5_P_EVENT_MONITOR_NUM_CORE_TRACKS * uiCore;

   if (!hVC5->sEventMonitor.bActive)
      return true;

   return AddEvent(hVC5, uiTrackIndex, uiId, uiEventIndex, eEventType, bAlreadyHaveEventMutex);
}

bool BVC5_P_AddCoreEvent(
   BVC5_Handle    hVC5,
   uint32_t       uiCore,
   uint32_t       uiTrack,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BVC5_EventType eEventType
   )
{
   return BVC5_P_AddCoreEvent_priv(hVC5, uiCore, uiTrack, uiId, uiEventIndex, eEventType, /*bAlreadyHaveEventMutex=*/false);
}

static bool BVC5_P_AddCoreEventCJD(
   BVC5_Handle             hVC5,
   uint32_t                uiCore,
   uint32_t                uiTrack,
   uint32_t                uiEventIndex,
   BVC5_EventType          eEventType,
   BVC5_P_InternalJob     *psJob,
   BVC5_SchedDependencies *psDeps
   )
{
   bool  ok = true;

   if (!hVC5->sEventMonitor.bActive)
      return true;

   if (psJob == NULL)
      return false;

   {
      BVC5_P_EventBuffer  *psBuf   = &hVC5->sEventMonitor.sBuffer;
      uint32_t             depsPad = psDeps != NULL ? BVC5_MAX_DEPENDENCIES * sizeof(uint64_t) : 0;
      uint64_t             uiJobId = psJob->uiJobId;

      BKNI_AcquireMutex(hVC5->hEventMutex);

      if (psBuf->uiCapacityBytes - psBuf->uiBytesUsed < (36 + depsPad))
      {
         /* No room for this event */
         psBuf->bOverflow = true;
         ok = false;
         goto error;
      }

      ok = BVC5_P_AddCoreEvent_priv(hVC5, uiCore, uiTrack, (uint32_t)uiJobId, uiEventIndex, eEventType, /*bAlreadyHaveEventMutex=*/true);

      ok = ok && BVC5_P_Add32(hVC5, psJob->uiClientId);
      ok = ok && BVC5_P_Add64(hVC5, uiJobId);
      ok = ok && BVC5_P_AddDeps(hVC5, psDeps);

      BDBG_ASSERT(ok); /* Should be ok as we check for space above */

      if (!ok)
         psBuf->bOverflow = true;
   }

error:
   BKNI_ReleaseMutex(hVC5->hEventMutex);

   return ok;
}

bool BVC5_P_AddCoreEventCJ(
   BVC5_Handle          hVC5,
   uint32_t             uiCore,
   uint32_t             uiTrack,
   uint32_t             uiEventIndex,
   BVC5_EventType       eEventType,
   BVC5_P_InternalJob  *psJob
   )
{
   return BVC5_P_AddCoreEventCJD(hVC5, uiCore, uiTrack, uiEventIndex, eEventType, psJob, /*psDeps=*/NULL);
}

bool BVC5_P_AddCoreJobEvent(
   BVC5_Handle          hVC5,
   uint32_t             uiCore,
   uint32_t             uiTrack,
   uint32_t             uiEventIndex,
   BVC5_EventType       eEventType,
   BVC5_P_InternalJob  *psJob
   )
{
   BVC5_SchedDependencies  *deps = hVC5->sOpenParams.bGPUMonDeps ? &psJob->pBase->sCompletedDependencies : NULL;

   return BVC5_P_AddCoreEventCJD(hVC5, uiCore, uiTrack, uiEventIndex, eEventType, psJob, deps);
}

void BVC5_P_EventMemStats(
   BVC5_Handle hVC5,
   size_t      *puiCapacityBytes,
   size_t      *puiUsedBytes
   )
{
   BKNI_AcquireMutex(hVC5->hEventMutex);

   *puiCapacityBytes = hVC5->sEventMonitor.sBuffer.uiCapacityBytes;
   *puiUsedBytes     = hVC5->sEventMonitor.sBuffer.uiBytesUsed;

   BKNI_ReleaseMutex(hVC5->hEventMutex);
}

void BVC5_P_EventsRemoveClient(
   BVC5_Handle    hVC5,
   uint32_t       uiClientId
   )
{
   if (hVC5->sEventMonitor.bAcquired && hVC5->sEventMonitor.uiClientId == uiClientId)
   {
      BVC5_P_SetEventCollection(hVC5, uiClientId, BVC5_EventStop);
      BVC5_P_SetEventCollection(hVC5, uiClientId, BVC5_EventRelease);
   }
}

/* End of File */
