/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>
#include "../egl/egl_thread.h"
#include "../egl/egl_display.h"
#include "../common/khrn_process.h"
#include "libs/platform/bcm_perf_api.h"

#if EGL_BRCM_event_monitor

static EGLint egl_get_num_event_tracks()
{
   return bcm_sched_get_num_event_tracks();
}

static EGLint egl_get_num_events()
{
   return bcm_sched_get_num_events();
}

EGLAPI EGLint EGLAPIENTRY eglGetEventConstantBRCM(
   EGLenum pname
   )
{
   EGLint ret = -1;
   EGLint error = EGL_SUCCESS;

   if (!egl_initialized(0, false))
      return ret;

   switch (pname)
   {
   case EGL_NUM_EVENT_TRACKS_BRCM:
      ret = egl_get_num_event_tracks();
      break;
   case EGL_NUM_EVENTS_BRCM:
      ret = egl_get_num_events();
      break;
   case EGL_MAX_EVENT_STRING_LEN_BRCM:
      ret = 64;
      break;
   default:
      ret = -1;
      error = EGL_BAD_PARAMETER;
      break;
   }

   egl_thread_set_error(error);
   return ret;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetEventTrackInfoBRCM(
   EGLint track,
   EGLint nameStrSize,
   char *trackNameStr
   )
{
   bool                                 ok;
   struct bcm_sched_event_track_desc    desc;

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   ok = bcm_sched_describe_event_track(track, &desc);

   if (ok)
   {
      egl_thread_set_error(EGL_SUCCESS);

      if (nameStrSize > 0 && trackNameStr != NULL)
         strncpy(trackNameStr, desc.name, nameStrSize);
   }
   else
   {
      egl_thread_set_error(EGL_BAD_PARAMETER);
      return EGL_FALSE;
   }

   return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetEventInfoBRCM(
   EGLint event,
   EGLint nameStrSize,
   char *nameStr,
   EGLint *numDataFields
   )
{
   bool                           ok;
   struct bcm_sched_event_desc    desc;

   if (numDataFields)
      *numDataFields = 0;

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   ok = bcm_sched_describe_event(event, &desc);

   if (ok)
   {
      egl_thread_set_error(EGL_SUCCESS);

      if (nameStrSize > 0 && nameStr != NULL)
         strncpy(nameStr, desc.name, nameStrSize);

      if (numDataFields != NULL)
         *numDataFields = desc.num_data_fields;
   }
   else
   {
      egl_thread_set_error(EGL_BAD_PARAMETER);
      return EGL_FALSE;
   }

   return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetEventDataFieldInfoBRCM(
   EGLint event,
   EGLint field,
   EGLint nameStrSize,
   char *nameStr,
   EGLBoolean *isSigned,
   EGLint *numBytes
   )
{
   bool                                ok;
   struct bcm_sched_event_field_desc   desc;

   if (numBytes)
      *numBytes = 0;

   if (isSigned)
      *isSigned = EGL_FALSE;

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   ok = bcm_sched_describe_event_data(event, field, &desc);

   if (ok)
   {
      egl_thread_set_error(EGL_SUCCESS);

      if (nameStrSize > 0 && nameStr != NULL)
         strncpy(nameStr, desc.name, nameStrSize);

      if (isSigned != NULL && numBytes != NULL)
      {
         switch (desc.data_type)
         {
         case BCM_EVENT_INT32:   *isSigned = EGL_TRUE;   *numBytes = 4; break;
         case BCM_EVENT_UINT32:  *isSigned = EGL_FALSE;  *numBytes = 4; break;
         case BCM_EVENT_INT64:   *isSigned = EGL_TRUE;   *numBytes = 8; break;
         case BCM_EVENT_UINT64:  *isSigned = EGL_FALSE;  *numBytes = 8; break;
         }
      }
   }
   else
   {
      egl_thread_set_error(EGL_BAD_PARAMETER);
      return EGL_FALSE;
   }

   return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglSetEventCollectionBRCM(
   EGLenum pname
   )
{
   enum bcm_sched_event_state  s;
   bool                        ok;
   EGL_THREAD_T                *thread = egl_thread_get();

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   if (thread == NULL)
      return EGL_FALSE;

   switch (pname)
   {
   case EGL_ACQUIRE_EVENTS_BRCM:  s = BCM_EVENT_ACQUIRE; break;
   case EGL_RELEASE_EVENTS_BRCM:  s = BCM_EVENT_RELEASE; break;
   case EGL_START_EVENTS_BRCM:    s = BCM_EVENT_START; break;
   case EGL_STOP_EVENTS_BRCM:     s = BCM_EVENT_STOP;  break;
   default :                      egl_thread_set_error(EGL_BAD_PARAMETER); return EGL_FALSE;
   }

   ok = bcm_sched_set_event_collection(s);
   if (!ok)
   {
      egl_thread_set_error(EGL_BAD_ACCESS);
      return EGL_FALSE;
   }

   if (pname == EGL_ACQUIRE_EVENTS_BRCM)
      thread->events_acquired = true;
   else if (pname == EGL_RELEASE_EVENTS_BRCM)
      thread->events_acquired = false;

   egl_thread_set_error(EGL_SUCCESS);
   return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetEventDataBRCM(
   EGLint dataBufferBytes,
   void *data,
   EGLint *bytesWritten,
   EGLBoolean *overflowed,
   EGLuint64BRCM *timebase
   )
{
   uint32_t      bytes = 0;
   bool          lost_sched_data;
   EGL_THREAD_T  *thread = egl_thread_get();

   if (bytesWritten)
      *bytesWritten = 0;

   if (overflowed)
      *overflowed = EGL_FALSE;

   if (timebase)
      *timebase = 0;

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   if (thread == NULL || !thread->events_acquired)
   {
      egl_thread_set_error(EGL_BAD_ACCESS);
      return EGL_FALSE;
   }

   if (dataBufferBytes < 0)
   {
      egl_thread_set_error(EGL_BAD_PARAMETER);
      return EGL_FALSE;
   }

   bytes = bcm_sched_poll_event_timeline(dataBufferBytes, data, &lost_sched_data, timebase);
   assert(!data || !dataBufferBytes || bytes <= (unsigned)dataBufferBytes);

   if (bytesWritten)
      *bytesWritten = bytes;

   if (overflowed)
      *overflowed = lost_sched_data;

   egl_thread_set_error(EGL_SUCCESS);
   return EGL_TRUE;
}

#endif
