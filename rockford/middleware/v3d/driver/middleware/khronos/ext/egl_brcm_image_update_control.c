/*=============================================================================
Copyright (c) 2012 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  EGL Broadcom image update control extension

FILE DESCRIPTION
Server-side implementation of EGL_BRCM_image_update_control.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/ext/egl_khr_image.h"
#include "interface/khronos/common/khrn_api_interposer.h"
#include "middleware/khronos/common/2708/khrn_tfconvert_4.h"

#ifdef EGL_BRCM_image_update_control

#define PIXELS_PER_TILE 64

#ifdef TIMELINE_EVENT_LOGGING
static void EventLog(uint32_t t, uint32_t c, uint32_t r, uint32_t d, char *desc)
{
   EventData ev;
   ev.eventType = t;
   ev.eventCode = c;
   ev.eventRow  = r;
   ev.eventData = d;
   ev.desc = desc;
   ev.eventSecs = 0;
   ev.eventNanosecs = 0;
   khrn_remote_event_log(&ev);
}
#define LogLock(IMAGE)   EventLog(eEVENT_WAITING, eEVENT_EGLIMAGE_APP_LOCK, khrn_event_cpu_row(), (uint32_t)IMAGE, 0);
#define LogLocked(IMAGE) EventLog(eEVENT_START,   eEVENT_EGLIMAGE_APP_LOCK, khrn_event_cpu_row(), (uint32_t)IMAGE, 0);
#define LogUnlock(IMAGE) EventLog(eEVENT_END,     eEVENT_EGLIMAGE_APP_LOCK, khrn_event_cpu_row(), (uint32_t)IMAGE, 0);
#else
#define LogLock(IMAGE)
#define LogLocked(IMAGE)
#define LogUnlock(IMAGE)
#endif

// This function is called with an API lock in place
MEM_HANDLE_T eglImageUpdateParameterBRCM_lockPhase1(EGLImageKHR image)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T       heglimage;

   heglimage = khrn_map_lookup(&state->eglimages, (uint32_t)image);
   if (heglimage != MEM_HANDLE_INVALID)
   {
      mem_acquire(heglimage);

      LogLock(heglimage);

      // Note: dont release the eglimage here - we want it to remain retained until eglImageUpdateParameterBRCM_lockPhase4 is called.
   }

   return heglimage;
}

// NOTE : This function is called WITHOUT an API lock in place.
// Take extreme care!!
bool eglImageUpdateParameterBRCM_lockPhase2(MEM_HANDLE_T heglimage)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   if (heglimage != MEM_HANDLE_INVALID)
   {
      EGL_IMAGE_T *eglimage = mem_lock(heglimage, NULL);

      if (eglimage == NULL)
         return true;

      // First wait for the 'missed' flag to be cleared following a successful conversion
      // It might already be clear of course.
      // (Timeout after 500ms to handle end of render conditions).
      if (vcos_semaphore_wait_timeout(&eglimage->missedConvFlag, 500) == VCOS_SUCCESS)
         vcos_semaphore_post(&eglimage->missedConvFlag);
      else
      {
         mem_unlock(heglimage);
         return false;
      }

      mem_unlock(heglimage);
      return true;
   }

   return true;
}

// This function is called with an API lock in place
void eglImageUpdateParameterBRCM_lockPhase3(MEM_HANDLE_T heglimage, bool phase2Ret)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   if (heglimage != MEM_HANDLE_INVALID)
   {
      EGL_IMAGE_T *eglimage = (EGL_IMAGE_T *)mem_lock(heglimage, NULL);

      if (!phase2Ret)
      {
         // Timed out waiting for the 'missed' flag to lock - it's highly likely that the app has stopped rendering 3d stuff.
         // So, we'll post the conversion again, and wait once more for completion. If we don't we could stall the 2d thread
         // indefinitely.
         khrn_rso_to_tf_convert(heglimage, MEM_HANDLE_INVALID, eglimage->mh_shadow_image);
      }

      mem_unlock(heglimage);
   }
}

// NOTE : This function is called WITHOUT an API lock in place.
// Take extreme care!!
void eglImageUpdateParameterBRCM_lockPhase4(MEM_HANDLE_T heglimage, bool phase2Ret)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   if (heglimage != MEM_HANDLE_INVALID)
   {
      EGL_IMAGE_T *eglimage = (EGL_IMAGE_T *)mem_lock(heglimage, NULL);

      if (!phase2Ret)
      {
         if (vcos_semaphore_wait_timeout(&eglimage->missedConvFlag, 500) == VCOS_SUCCESS)
         {
            // We got the lock, so the conversion completed successfully. Clear the lock again.
            vcos_semaphore_post(&eglimage->missedConvFlag);
         }
         else
         {
            // We timed out again - so something weird must have happened. Let's pretend the 2d app got the image lock so
            // it doesn't stall. Nothing to do here.
         }
      }

      // Now lock the EGL image
      while (vcos_semaphore_wait(&eglimage->lockSemaphore) != VCOS_SUCCESS)
         continue;

      LogLocked(heglimage);

      // Force the image into a conversion now to avoid 'every-other-frame-syndrome'
      vcos_mutex_lock(&eglimage->dirtyBitsMutex);
      eglimage->forceConversion = true;
      vcos_mutex_unlock(&eglimage->dirtyBitsMutex);

      mem_unlock(heglimage);

      MEM_ASSIGN(heglimage, MEM_HANDLE_INVALID);
   }
}

bool eglImageUpdateParameterBRCM_impl(EGLImageKHR image, EGLenum pname, const EGLint *params)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   uint32_t id = (uint32_t)image;
   MEM_HANDLE_T heglimage;

   uint32_t maxTiles = sizeof(tile_bits_t) * 8; /* 32 for 2048 limit, 64 for 4096 limit */

#ifndef V3D_LEAN
   vcos_log("KHRN eglImageUpdateParameterBRCM_impl %d", image);
#endif

   heglimage = khrn_map_lookup(&state->eglimages, (uint32_t)id);
   if (heglimage != MEM_HANDLE_INVALID)
   {
      EGL_IMAGE_T  *eglimage = (EGL_IMAGE_T *)mem_lock(heglimage, NULL);

      if (pname == EGL_IMAGE_UPDATE_CONTROL_SET_MODE_BRCM)
      {
         if (params[0] == EGL_IMAGE_UPDATE_CONTROL_ALWAYS_BRCM)
            eglimage->explicit_updates = false;
         else if (params[0] == EGL_IMAGE_UPDATE_CONTROL_EXPLICIT_BRCM)
            eglimage->explicit_updates = true;
      }
      else if (pname == EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM)
      {
         if (params[0] == EGL_IMAGE_UPDATE_CONTROL_LOCK_BRCM)
         {
            // Should go through the separate lock interface
            UNREACHABLE();
         }
         else
         {
            // Unlock EGL image

            // See if we have a missed conversion to deal with
            bool missed = (vcos_semaphore_wait_timeout(&eglimage->missedConvFlag, 0) != VCOS_SUCCESS);
            if (!missed)
               vcos_semaphore_post(&eglimage->missedConvFlag); // Undo our tryLock that succeeded

            // If the conversion is flagged as 'missed' we need to repost the conversion to ensure that it happens at some point.
            if (missed)
               khrn_rso_to_tf_convert(heglimage, MEM_HANDLE_INVALID, eglimage->mh_shadow_image);

            LogUnlock(heglimage);
            vcos_semaphore_wait_timeout(&eglimage->lockSemaphore, 0);
            vcos_semaphore_post(&eglimage->lockSemaphore);
         }
      }
      else if (pname == EGL_IMAGE_UPDATE_CONTROL_CHANGED_REGION_BRCM)
      {
         uint32_t    tx[2], ty[2];
         uint32_t    y;
         tile_bits_t rowMask = 0, startMask = 0, endMask = 0;

         if (params[2] > 0 && params[3] > 0) // Don't process empty regions
         {
            /* Determine start tiles */
            tx[0] = params[0] / PIXELS_PER_TILE;
            ty[0] = params[1] / PIXELS_PER_TILE;

            /* Determine end tiles (using x + w - 1, y + h - 1) */
            tx[1] = (params[0] + params[2] - 1) / PIXELS_PER_TILE;
            ty[1] = (params[1] + params[3] - 1) / PIXELS_PER_TILE;

            startMask = ~((1 << tx[0]) - 1);
            if (tx[0] >= maxTiles)  // Can't shift 32 (or 64 for uint64), so check
               startMask = 0;

            endMask = (1 << (tx[1] + 1)) - 1;
            if (tx[1] + 1 >= maxTiles)  // Can't shift 32 (or 64 for uint64), so check
               endMask = ~0;

            rowMask = startMask & endMask;

            if (rowMask != 0 && ty[0] < maxTiles)
            {
               if (ty[1] >= maxTiles)
                  ty[1] = maxTiles - 1;

               vcos_mutex_lock(&eglimage->dirtyBitsMutex);
               for (y = ty[0]; y <= ty[1]; y++)
                  eglimage->dirtyBits.m_rowBits[y] |= rowMask;

               vcos_mutex_unlock(&eglimage->dirtyBitsMutex);
            }
         }
      }
      mem_unlock(heglimage);
   }
   else
      return false;

   return true;
}



#endif
