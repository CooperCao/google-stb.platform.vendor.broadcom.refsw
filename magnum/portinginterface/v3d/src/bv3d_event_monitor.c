/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bstd.h"
#include "bv3d.h"
#include "bv3d_priv.h"
#include "bv3d_worker_priv.h"
#include "bchp_pwr.h"
#include "bv3d_event_monitor_priv.h"

BDBG_MODULE(BV3D);

static void _strncpy(char *d, const char *s, uint32_t maxLen)
{
   uint32_t i;
   for (i = 0; i < maxLen - 1 && *s != 0; i++)
      *d++ = *s++;

   *d = '\0';
}

static void BV3D_P_SetEventTrack(
   BV3D_P_EventMonitor    *ev,
   uint32_t               track,
   const char             *name
   )
{
   _strncpy(ev->sTrackDescs[track].caName, name, BV3D_MAX_EVENT_STRING_LEN);

   if (track >= ev->uiNumTracks)
      ev->uiNumTracks = track + 1;
}

static void BV3D_P_SetEvent(
   BV3D_P_EventMonitor    *ev,
   uint32_t               event,
   const char             *name
   )
{
   _strncpy(ev->sEventDescs[event].sDesc.caName, name, BV3D_MAX_EVENT_STRING_LEN);
   ev->sEventDescs[event].sDesc.uiNumDataFields = 0;
   ev->sEventDescs[event].uiSize = BV3D_P_EVENT_FIXED_BYTES;
}

static void BV3D_P_SetEventDesc(
   BV3D_P_EventDesc    *eventDesc,
   BV3D_FieldType       type,
   const char          *name
   )
{
   uint32_t             id = eventDesc->sDesc.uiNumDataFields;
   BV3D_EventFieldDesc  *field = &eventDesc->sFieldDescs[id];

   BDBG_ASSERT(id < BV3D_P_EVENT_MONITOR_MAX_FIELDS);

   field->eDataType = type;
   _strncpy(field->caName, name, BV3D_MAX_EVENT_STRING_LEN);

   eventDesc->sDesc.uiNumDataFields++;

   switch(field->eDataType)
   {
   case BV3D_EventInt32:
   case BV3D_EventUInt32:
      eventDesc->uiSize += 4;
      break;
   case BV3D_EventInt64:
   case BV3D_EventUInt64:
      eventDesc->uiSize += 8;
      break;
   }
}

static void BV3D_P_SetEvent3(
   BV3D_P_EventMonitor   *ev,
   uint32_t               event,
   const char            *name,
   BV3D_FieldType         ftype1,
   const char            *fname1,
   BV3D_FieldType         ftype2,
   const char            *fname2,
   BV3D_FieldType         ftype3,
   const char            *fname3
   )
{
   BV3D_P_EventDesc  *eventDesc = &ev->sEventDescs[event];

   BV3D_P_SetEvent(ev, event, name);

   BV3D_P_SetEventDesc(eventDesc, ftype1, fname1);
   BV3D_P_SetEventDesc(eventDesc, ftype2, fname2);
   BV3D_P_SetEventDesc(eventDesc, ftype3, fname3);
}

static void BV3D_P_SetEvents(
   BV3D_P_EventMonitor    *ev,
   uint32_t               event,
   const char             *name
   )
{
   BV3D_P_EventDesc  *eventDesc = &ev->sEventDescs[event];

   BV3D_P_SetEvent(ev, event, name);

   BV3D_P_SetEventDesc(eventDesc, BV3D_EventUInt32, "ClientID");
   BV3D_P_SetEventDesc(eventDesc, BV3D_EventUInt64, "JobID");
}

/***************************************************************************/

void BV3D_P_InitEventMonitor(
   BV3D_Handle  hV3d
   )
{
   BKNI_AcquireMutex(hV3d->hEventMutex);

   BKNI_Memset(&hV3d->sEventMonitor, 0, sizeof(hV3d->sEventMonitor));

   BV3D_P_SetEventTrack(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_SCHED_TRACK,  "Scheduler");
   BV3D_P_SetEventTrack(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_USER_TRACK,   "User");
   BV3D_P_SetEventTrack(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_BIN_TRACK,    "Binner");
   BV3D_P_SetEventTrack(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_RENDER_TRACK, "Renderer");

   BV3D_P_SetEvents(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_BINNING,   "Binning");
   BV3D_P_SetEvents(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_RENDERING, "Rendering");
   BV3D_P_SetEvents(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_USER,      "User");

   BV3D_P_SetEvent(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_BOOM, "Bin OOM");
   BV3D_P_SetEvent(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_BIN_LOCKUP, "Bin Lockup");
   BV3D_P_SetEvent(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_RENDER_LOCKUP, "Render Lockup");
   BV3D_P_SetEvent(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_USER_LOCKUP, "User Lockup");

   BV3D_P_SetEvent3(&hV3d->sEventMonitor, BV3D_P_EVENT_MONITOR_FLUSH_V3D, "Flush V3D Caches",
                    BV3D_EventUInt32, "L3",
                    BV3D_EventUInt32, "L2C/Slice",
                    BV3D_EventUInt32, "L2T");

   BKNI_ReleaseMutex(hV3d->hEventMutex);
}

/***************************************************************************/
void BV3D_GetEventCounts(
   BV3D_Handle  hV3d,
   uint32_t     *uiNumTracks,
   uint32_t     *uiNumEvents
   )
{
   BDBG_ENTER(BV3D_GetEventCounts);
   BKNI_AcquireMutex(hV3d->hModuleMutex);
   BKNI_AcquireMutex(hV3d->hEventMutex);

   if (uiNumTracks)
      *uiNumTracks = hV3d->sEventMonitor.uiNumTracks;

   if (uiNumEvents)
      *uiNumEvents = BV3D_P_EVENT_MONITOR_NUM_EVENTS;

   BKNI_ReleaseMutex(hV3d->hEventMutex);
   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_GetEventCounts);
}

BERR_Code BV3D_GetEventTrackInfo(
   BV3D_Handle           hV3d,
   uint32_t              uiTrack,
   BV3D_EventTrackDesc   *psTrackDesc
   )
{
   BERR_Code err = BERR_SUCCESS;

   BDBG_ENTER(BV3D_GetEventTrackInfo);
   BKNI_AcquireMutex(hV3d->hModuleMutex);
   BKNI_AcquireMutex(hV3d->hEventMutex);

   if (uiTrack < hV3d->sEventMonitor.uiNumTracks)
      _strncpy(psTrackDesc->caName, hV3d->sEventMonitor.sTrackDescs[uiTrack].caName, BV3D_MAX_EVENT_STRING_LEN);
   else
      err = BERR_INVALID_PARAMETER;

   BKNI_ReleaseMutex(hV3d->hEventMutex);
   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_GetEventTrackInfo);

   return err;
}

BERR_Code BV3D_GetEventInfo(
   BV3D_Handle       hV3d,
   uint32_t          uiEvent,
   BV3D_EventDesc   *psEventDesc
   )
{
   BERR_Code err = BERR_SUCCESS;

   BDBG_ENTER(BV3D_GetEventInfo);
   BKNI_AcquireMutex(hV3d->hModuleMutex);
   BKNI_AcquireMutex(hV3d->hEventMutex);

   if (uiEvent < BV3D_P_EVENT_MONITOR_NUM_EVENTS)
   {
      psEventDesc->uiNumDataFields = hV3d->sEventMonitor.sEventDescs[uiEvent].sDesc.uiNumDataFields;
      _strncpy(psEventDesc->caName, hV3d->sEventMonitor.sEventDescs[uiEvent].sDesc.caName, BV3D_MAX_EVENT_STRING_LEN);
   }
   else
      err = BERR_INVALID_PARAMETER;

   BKNI_ReleaseMutex(hV3d->hEventMutex);
   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_GetEventInfo);

   return err;
}

BERR_Code BV3D_GetEventDataFieldInfo(
   BV3D_Handle          hV3d,
   uint32_t             uiEvent,
   uint32_t             uiField,
   BV3D_EventFieldDesc  *psFieldDesc
   )
{
   BERR_Code err = BERR_SUCCESS;

   BDBG_ENTER(BV3D_GetEventDataFieldInfo);
   BKNI_AcquireMutex(hV3d->hModuleMutex);
   BKNI_AcquireMutex(hV3d->hEventMutex);

   if (uiEvent < BV3D_P_EVENT_MONITOR_NUM_EVENTS)
   {
      if (uiField < hV3d->sEventMonitor.sEventDescs[uiEvent].sDesc.uiNumDataFields)
      {
         psFieldDesc->eDataType = hV3d->sEventMonitor.sEventDescs[uiEvent].sFieldDescs[uiField].eDataType;
         _strncpy(psFieldDesc->caName, hV3d->sEventMonitor.sEventDescs[uiEvent].sFieldDescs[uiField].caName,
                  BV3D_MAX_EVENT_STRING_LEN);
      }
      else
         err = BERR_INVALID_PARAMETER;
   }
   else
      err = BERR_INVALID_PARAMETER;

   BKNI_ReleaseMutex(hV3d->hEventMutex);
   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_GetEventDataFieldInfo);

   return err;
}

static void BV3D_P_CreateEventBuffer(
   BV3D_Handle hV3d
   )
{
   if (hV3d->sEventMonitor.sBuffer.uiCapacityBytes == 0)
   {
      hV3d->sEventMonitor.sBuffer.pBuffer = BKNI_Malloc(BV3D_P_EVENT_BUFFER_BYTES);
      if (hV3d->sEventMonitor.sBuffer.pBuffer != NULL)
      {
         hV3d->sEventMonitor.sBuffer.uiCapacityBytes = BV3D_P_EVENT_BUFFER_BYTES;
         hV3d->sEventMonitor.sBuffer.uiBytesUsed = 0;
         hV3d->sEventMonitor.sBuffer.pRead = hV3d->sEventMonitor.sBuffer.pBuffer;
         hV3d->sEventMonitor.sBuffer.pWrite = hV3d->sEventMonitor.sBuffer.pBuffer;
         hV3d->sEventMonitor.sBuffer.bOverflow = false;
      }
   }
}

static void BV3D_P_DeleteEventBuffer(
   BV3D_Handle hV3d)
{
   if (hV3d->sEventMonitor.sBuffer.pBuffer != NULL)
   {
      BKNI_Free(hV3d->sEventMonitor.sBuffer.pBuffer);
      hV3d->sEventMonitor.sBuffer.pBuffer = NULL;
      hV3d->sEventMonitor.sBuffer.uiCapacityBytes = 0;
      hV3d->sEventMonitor.sBuffer.uiBytesUsed = 0;
      hV3d->sEventMonitor.sBuffer.pRead = NULL;
      hV3d->sEventMonitor.sBuffer.pWrite = NULL;
      hV3d->sEventMonitor.sBuffer.bOverflow = false;
   }
}

static BERR_Code BV3D_P_SetEventCollection(
   BV3D_Handle       hV3d,
   uint32_t          uiClientId,
   BV3D_EventState   eState
   )
{
   BERR_Code err = BERR_SUCCESS;

   BKNI_AcquireMutex(hV3d->hEventMutex);

   switch (eState)
   {
   case BV3D_EventAcquire:
      if (hV3d->sEventMonitor.bAcquired)
      {
         if (hV3d->sEventMonitor.uiClientId != uiClientId)
         {
            err = BERR_NOT_AVAILABLE;
            goto error;
         }
      }
      else
      {
         BV3D_P_CreateEventBuffer(hV3d);
         if (hV3d->sEventMonitor.sBuffer.pBuffer == NULL)
         {
            err = BERR_OUT_OF_SYSTEM_MEMORY;
            goto error;
         }

         hV3d->sEventMonitor.bAcquired = true;
         hV3d->sEventMonitor.uiClientId = uiClientId;
      }

      break;
   case BV3D_EventRelease:
      if (hV3d->sEventMonitor.bAcquired)
      {
         BV3D_P_DeleteEventBuffer(hV3d);
         hV3d->sEventMonitor.bAcquired = false;
         hV3d->sEventMonitor.uiClientId = 0;
      }
      break;
   case BV3D_EventStart:
      if (hV3d->sEventMonitor.bAcquired && hV3d->sEventMonitor.uiClientId == uiClientId)
      {
         hV3d->sEventMonitor.bActive = true;
      }
      else
         err = BERR_NOT_AVAILABLE;
      break;
   case BV3D_EventStop:
      if (hV3d->sEventMonitor.bAcquired && hV3d->sEventMonitor.uiClientId == uiClientId)
      {
         hV3d->sEventMonitor.bActive = false;
      }
      else
         err = BERR_NOT_AVAILABLE;
      break;
   default:
      err = BERR_INVALID_PARAMETER;
      break;
   }

error:
   BKNI_ReleaseMutex(hV3d->hEventMutex);

   return err;
}

BERR_Code BV3D_SetEventCollection(
   BV3D_Handle       hV3d,
   uint32_t          uiClientId,
   BV3D_EventState   eState
   )
{
   BERR_Code err;

   BDBG_ENTER(BV3D_SetEventCollection);
   BKNI_AcquireMutex(hV3d->hModuleMutex);

   err = BV3D_P_SetEventCollection(hV3d, uiClientId, eState);

   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_SetEventCollection);

   return err;
}

static void BV3D_P_SetOverflow_isrsafe(BV3D_Handle hV3d, bool overflow)
{
   BV3D_P_EventBuffer *psBuf = &hV3d->sEventMonitor.sBuffer;
   if (psBuf->bOverflow != overflow)
   {
      psBuf->bOverflow = overflow;
      if (overflow)
         BDBG_WRN(("Event buffer overflow - event recording stopped"));
      else
         BDBG_WRN(("Event buffer read - event recording resumed"));
   }
}

#define LOCK_EVENTS() BKNI_AcquireMutex(hV3d->hEventMutex)
#define UNLOCK_EVENTS() BKNI_ReleaseMutex(hV3d->hEventMutex)

uint32_t BV3D_GetEventData(
   BV3D_Handle    hV3d,
   uint32_t       uiClientId,
   uint32_t       uiEventBufferBytes,
   void           *pvEventBuffer,
   uint32_t       *puiLostData,
   uint64_t       *puiTimeStamp
   )
{
   BV3D_P_EventBuffer *psBuf = &hV3d->sEventMonitor.sBuffer;
   uint32_t           bytes  = 0;

   BDBG_ENTER(BV3D_GetEventData);

   BKNI_AcquireMutex(hV3d->hModuleMutex);
   LOCK_EVENTS();

   if (!hV3d->sEventMonitor.bAcquired || hV3d->sEventMonitor.uiClientId != uiClientId)
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

      bytes = bytesToCopy = psBuf->uiBytesUsed < uiEventBufferBytes ?
         psBuf->uiBytesUsed : uiEventBufferBytes;

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

      BV3D_P_SetOverflow_isrsafe(hV3d, false);
   }

error:
   if (puiTimeStamp)
      *puiTimeStamp = BV3D_P_GetEventTimestamp();

   UNLOCK_EVENTS();
   BKNI_ReleaseMutex(hV3d->hModuleMutex);
   BDBG_LEAVE(BV3D_GetEventData);

   return bytes;
}

/* You must have a lock on hEventMutex before calling this */
static bool BV3D_P_Add32_isrsafe(
   BV3D_Handle    hV3d,
   uint32_t       uiData
   )
{
   BV3D_P_EventBuffer *psBuf = &hV3d->sEventMonitor.sBuffer;
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
static bool BV3D_P_Add64_isrsafe(
   BV3D_Handle    hV3d,
   uint64_t       uiData
   )
{
   BV3D_P_EventBuffer *psBuf = &hV3d->sEventMonitor.sBuffer;
   bool               ok = true;

   if (psBuf->uiCapacityBytes - psBuf->uiBytesUsed < 8)
      return false;

   ok = ok && BV3D_P_Add32_isrsafe(hV3d, (uint32_t)uiData);
   ok = ok && BV3D_P_Add32_isrsafe(hV3d, (uint32_t)(uiData >> 32));

   BDBG_ASSERT(ok); /* Should be ok as we check for space above */

   return ok;
}

static bool BV3D_P_AddEvent_isrsafe(
   BV3D_Handle    hV3d,
   uint32_t       uiTrackIndex,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BV3D_EventType eEventType,
   uint64_t       uiTimestamp
   )
{
   BV3D_P_EventBuffer *psBuf = &hV3d->sEventMonitor.sBuffer;
   BV3D_P_EventDesc   *eventDesc;
   bool               ok = true;
   uint32_t           before;

   BDBG_ASSERT(uiEventIndex < BV3D_P_EVENT_MONITOR_NUM_EVENTS);
   eventDesc = &hV3d->sEventMonitor.sEventDescs[uiEventIndex];
   BDBG_ASSERT(eventDesc->uiSize >= BV3D_P_EVENT_FIXED_BYTES);

   if (psBuf->uiCapacityBytes - psBuf->uiBytesUsed < eventDesc->uiSize)
   {
      /* No room for this event */
      BV3D_P_SetOverflow_isrsafe(hV3d, true);
      ok = false;
      goto error;
   }

   before = psBuf->uiBytesUsed;

   ok = ok && BV3D_P_Add64_isrsafe(hV3d, uiTimestamp);
   ok = ok && BV3D_P_Add32_isrsafe(hV3d, uiTrackIndex);
   ok = ok && BV3D_P_Add32_isrsafe(hV3d, uiId);
   ok = ok && BV3D_P_Add32_isrsafe(hV3d, uiEventIndex);
   ok = ok && BV3D_P_Add32_isrsafe(hV3d, eEventType);

   BDBG_ASSERT(ok); /* Should be ok as we check for space above */
   BDBG_ASSERT(psBuf->uiBytesUsed - before == BV3D_P_EVENT_FIXED_BYTES);

   if (!ok)
      BV3D_P_SetOverflow_isrsafe(hV3d, true);

error:
   return ok;
}

bool BV3D_P_AddEvent(
   BV3D_Handle    hV3d,
   uint32_t       uiTrack,
   uint32_t       uiId,
   uint32_t       uiEventIndex,
   BV3D_EventType eEventType,
   uint64_t       uiTimestamp
   )
{
   bool ret = true;

   if (hV3d->sEventMonitor.bActive)
   {
      LOCK_EVENTS();
      ret = BV3D_P_AddEvent_isrsafe(hV3d, uiTrack, uiId, uiEventIndex,
            eEventType, uiTimestamp);
      UNLOCK_EVENTS();
   }

   return ret;
}

bool BV3D_P_AddFlushEvent(
   BV3D_Handle    hV3d,
   uint32_t       uiId,
   BV3D_EventType eEventType,
   bool           clearL3,
   bool           clearL2C,
   bool           clearL2T,
   uint64_t       uiTimestamp
   )
{
   bool  ok = true;
   uint32_t before;

   BV3D_P_EventBuffer  *psBuf   = &hV3d->sEventMonitor.sBuffer;

   if (!hV3d->sEventMonitor.bActive)
      return ok;

   LOCK_EVENTS();

   before = psBuf->uiBytesUsed;
   ok = BV3D_P_AddEvent_isrsafe(hV3d, BV3D_P_EVENT_MONITOR_SCHED_TRACK, uiId,
         BV3D_P_EVENT_MONITOR_FLUSH_V3D, eEventType, uiTimestamp);
   if (ok)
   {
      ok = ok && BV3D_P_Add32_isrsafe(hV3d, (uint32_t)clearL3);
      ok = ok && BV3D_P_Add32_isrsafe(hV3d, (uint32_t)clearL2C);
      ok = ok && BV3D_P_Add32_isrsafe(hV3d, (uint32_t)clearL2T);

      BDBG_ASSERT(ok); /* Should be ok as we check for space above */
      BDBG_ASSERT(psBuf->uiBytesUsed - before ==
            hV3d->sEventMonitor.sEventDescs[BV3D_P_EVENT_MONITOR_FLUSH_V3D].uiSize);
   }

   if (!ok)
      BV3D_P_SetOverflow_isrsafe(hV3d, true);

   UNLOCK_EVENTS();

   return ok;
}

static bool BV3D_P_AddEventCJ_isrsafe(
   BV3D_Handle             hV3d,
   uint32_t                uiTrack,
   uint32_t                uiEventIndex,
   BV3D_EventType          eEventType,
   uint32_t                uiClientId,
   uint64_t                uiJobId,
   uint64_t                uiTimestamp
   )
{
   bool  ok = true;

   BV3D_P_EventBuffer  *psBuf   = &hV3d->sEventMonitor.sBuffer;
   uint32_t             before;

   before = psBuf->uiBytesUsed;
   ok = BV3D_P_AddEvent_isrsafe(hV3d, uiTrack, (uint32_t) uiJobId,
         uiEventIndex, eEventType, uiTimestamp);
   if (ok)
   {
      ok = ok && BV3D_P_Add32_isrsafe(hV3d, uiClientId);
      ok = ok && BV3D_P_Add64_isrsafe(hV3d, uiJobId);

      BDBG_ASSERT(ok); /* Should be ok as we check for space above */
      BDBG_ASSERT(psBuf->uiBytesUsed - before ==
            hV3d->sEventMonitor.sEventDescs[uiEventIndex].uiSize);
   }

   if (!ok)
      BV3D_P_SetOverflow_isrsafe(hV3d, true);

   return ok;
}

bool BV3D_P_AddEventCJ(
   BV3D_Handle             hV3d,
   uint32_t                uiTrack,
   uint32_t                uiEventIndex,
   BV3D_EventType          eEventType,
   uint32_t                uiClientId,
   uint64_t                uiJobId,
   uint64_t                uiTimestamp
   )
{
   bool  ok = true;

   if (hV3d->sEventMonitor.bActive)
   {
      LOCK_EVENTS();
      ok = BV3D_P_AddEventCJ_isrsafe(hV3d, uiTrack, uiEventIndex,
            eEventType, uiClientId, uiJobId, uiTimestamp);
      UNLOCK_EVENTS();
   }

   return ok;
}

bool BV3D_P_AddJobEvent(
   BV3D_Handle          hV3d,
   uint32_t             uiTrack,
   uint32_t             uiEventIndex,
   BV3D_EventType       eEventType,
   BV3D_P_EventInfo    *psEventInfo,
   uint64_t             uiTimestamp
   )
{
   return BV3D_P_AddEventCJ(hV3d, uiTrack, uiEventIndex,
      eEventType, psEventInfo->uiClientId, psEventInfo->uiJobId, uiTimestamp);
}

void BV3D_P_EventMemStats(
   BV3D_Handle hV3d,
   size_t      *puiCapacityBytes,
   size_t      *puiUsedBytes
   )
{
   LOCK_EVENTS();

   *puiCapacityBytes = hV3d->sEventMonitor.sBuffer.uiCapacityBytes;
   *puiUsedBytes     = hV3d->sEventMonitor.sBuffer.uiBytesUsed;

   UNLOCK_EVENTS();
}

void BV3D_P_EventsRemoveClient(
   BV3D_Handle    hV3d,
   uint32_t       uiClientId
   )
{
   if (hV3d->sEventMonitor.bAcquired && hV3d->sEventMonitor.uiClientId == uiClientId)
   {
      BV3D_P_SetEventCollection(hV3d, uiClientId, BV3D_EventStop);
      BV3D_P_SetEventCollection(hV3d, uiClientId, BV3D_EventRelease);
   }
}

void BV3D_P_PopulateEventInfo(
   BV3D_Handle hV3d,
   const BV3D_Job *psJob,
   BV3D_P_EventInfo *psEventInfo
)
{
   BSTD_UNUSED(hV3d);

   if (psEventInfo)
   {
      psEventInfo->uiTimeStamp = BV3D_P_GetEventTimestamp();

      psEventInfo->uiClientId = psJob->uiClientId;
      psEventInfo->uiJobId = psJob->uiSequence;
   }
}
