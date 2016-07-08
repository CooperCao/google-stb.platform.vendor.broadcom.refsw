/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  3D Tools
Module   :  Control list creation and submission for RSO to t-format using tile buffer

FILE DESCRIPTION
Builds a control list for RSO to t-format conversion using tile buffer
=============================================================================*/

#include "middleware/khronos/common/2708/khrn_tfconvert_4.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "interface/khronos/common/khrn_api_interposer.h"

#define TLB_INVALID_FORMAT 3
#define NULL_LIST_SIZE     27

static uint32_t get_dest_format(KHRN_IMAGE_FORMAT_T format)
{
   switch (format & IMAGE_FORMAT_PIXEL_LAYOUT_MASK)
   {
   case IMAGE_FORMAT_8888 : return 1;
   case IMAGE_FORMAT_565  : return 2;
   default                : return TLB_INVALID_FORMAT;
   }
}

static uint32_t get_src_format(KHRN_IMAGE_FORMAT_T format)
{
   switch (format & IMAGE_FORMAT_PIXEL_LAYOUT_MASK)
   {
   case IMAGE_FORMAT_8888 : return 0;
   case IMAGE_FORMAT_565  : return 2;
   default                : return TLB_INVALID_FORMAT;
   }
}

static uint32_t get_type_flag(KHRN_IMAGE_FORMAT_T format)
{
   if (khrn_image_is_tformat(format))
      return 1;
   else if (khrn_image_is_lineartile(format))
      return 2;
   else // if (khrn_image_is_rso(format))
      return 0;
}

// This function nulls out all tile conversions that are not needed because they aren't currently
// marked as dirty.
static bool skip_clean_tiles(CONVERT_CALLBACK_DATA_T *data, KHRN_IMAGE_T *dst)
{
   uint8_t *p = data->controlList;
   uint8_t *end = data->controlList + data->numCLBytes - 1;
   uint32_t cnt = 0;
   uint32_t xTiles, yTiles, i, j;
   uint32_t numDirty;
   EGL_IMAGE_T *eglimage;

   if (data->controlList == NULL || data->numCLBytes < NULL_LIST_SIZE)
      return false;

   eglimage = (EGL_IMAGE_T *)mem_lock(data->heglimage, NULL);

   numDirty = egl_image_num_dirty_tiles(eglimage);

   if (numDirty == 0)
   {
      mem_unlock(data->heglimage);
      return false;
   }

   xTiles = (dst->width  + 63) / 64;
   yTiles = (dst->height + 63) / 64;

   p = p + 11;
   for (j = 0; j != yTiles; ++j)
   {
      for (i = 0; i != xTiles; ++i)
      {
         /* We write the store for the previous tile here as we're not storing all tiles.
          * We can then add the final store with EOF after the loop */
         if (!egl_image_is_tile_dirty(eglimage, i, j))
         {
            ADD_BYTE(p, KHRN_HW_INSTR_NOP);
            ADD_BYTE(p, KHRN_HW_INSTR_NOP);
            ADD_BYTE(p, KHRN_HW_INSTR_NOP);
            ADD_BYTE(p, KHRN_HW_INSTR_NOP);
            ADD_BYTE(p, KHRN_HW_INSTR_NOP);
            ADD_BYTE(p, KHRN_HW_INSTR_NOP);
            ADD_BYTE(p, KHRN_HW_INSTR_NOP);
            ADD_BYTE(p, KHRN_HW_INSTR_NOP);
            ADD_BYTE(p, KHRN_HW_INSTR_NOP);
            ADD_BYTE(p, KHRN_HW_INSTR_NOP);
            ADD_BYTE(p, KHRN_HW_INSTR_NOP);
         }
         else
         {
            cnt++;
            if (cnt == numDirty)
            {
               // Ensure the last valid tile uses an EOF store
               p = p + 10;
               ADD_BYTE(p, KHRN_HW_INSTR_STORE_SUBSAMPLE_EOF);
            }
            else
               p = p + 11;
         }
      }
   }

   mem_unlock(data->heglimage);

   return true;
}

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
#define LogLock()   EventLog(eEVENT_WAITING, eEVENT_TFCONVERT, eEVENT_RENDERER, (uint32_t)convData->heglimage, 0);
#else
#define LogLock()
#endif

void khrn_tfconvert_logevents(BEGL_HWNotification *notification, BEGL_HWCallbackRecord *cbRec)
{
#ifdef TIMELINE_EVENT_LOGGING
   EventData               ev;
   CONVERT_CALLBACK_DATA_T *convData = (CONVERT_CALLBACK_DATA_T *)&cbRec->payload[0];

   /* not a real copy, just for output in spyhook so no reference counting required */
   ev.eventData = (uint32_t)convData->heglimage;
   ev.desc = 0;

   ev.eventType = eEVENT_START;
   ev.eventCode = eEVENT_TFCONVERT;
   ev.eventRow  = eEVENT_RENDERER;
   ev.eventSecs = notification->timelineData->renderStart.secs;
   ev.eventNanosecs = 1000 * notification->timelineData->renderStart.microsecs;
   khrn_remote_event_log(&ev);

   ev.eventType = eEVENT_END;
   ev.eventCode = eEVENT_TFCONVERT;
   ev.eventRow  = eEVENT_RENDERER;
   ev.eventSecs = notification->timelineData->renderEnd.secs;
   ev.eventNanosecs = 1000 * notification->timelineData->renderEnd.microsecs;
   khrn_remote_event_log(&ev);
#endif
}

/* BEWARE: This is a callback function and is on another thread to the main API code
 * Anything you do in these functions must not interfere with state that might be being used on the main API thread.
 */
bool khrn_tfconvert_lock(BEGL_HWCallbackRecord *cbRecord)
{
   CONVERT_CALLBACK_DATA_T *convData = (CONVERT_CALLBACK_DATA_T *)&cbRecord->payload[0];
   bool                    abandon    = false;

   if (convData && convData->heglimage != MEM_HANDLE_INVALID)
   {
      EGL_IMAGE_T    *eglimage = (EGL_IMAGE_T *)mem_lock(convData->heglimage, NULL);

      if (eglimage->mh_shadow_image != MEM_HANDLE_INVALID)
      {
         KHRN_IMAGE_T   *dst = (KHRN_IMAGE_T *)mem_lock(eglimage->mh_shadow_image, NULL);

         convData->conversionKilled = false;
         convData->semaphoreTaken = false;

         vcos_mutex_lock(&eglimage->dirtyBitsMutex);

         if (!eglimage->forceConversion && !egl_image_any_tile_dirty(eglimage))
         {
            // No dirty tiles any more, so kill the conversion
            convData->conversionKilled = true;

            vcos_mutex_unlock(&eglimage->dirtyBitsMutex);
            abandon = true;
         }
         else
         {
            // Time to lock the EGL image
            LogLock();

            // Try to take the lock
            if (vcos_semaphore_wait_timeout(&eglimage->lockSemaphore, 0) == VCOS_SUCCESS)
            {
               // We got the lock
               convData->semaphoreTaken = true;

               // Adjust the control list for the current state of the dirty tiles
               abandon = !skip_clean_tiles(convData, dst);
               egl_image_clr_all_tile_dirty_bits(eglimage);
               eglimage->forceConversion = false;
            }
            else
            {
               // The application has the image locked right now.
               // Record the fact that we missed a conversion that we actually wanted.
               // We use tryLock since we may already have the flag locked from a previous conversion attempt.
               // We don't care if it fails due to already being locked, we just want to ensure it is locked.
               vcos_semaphore_wait_timeout(&eglimage->missedConvFlag, 0);

               // Kill the conversion
               convData->conversionKilled = true;
               abandon = true;
            }
         }

         vcos_mutex_unlock(&eglimage->dirtyBitsMutex);

         mem_unlock(eglimage->mh_shadow_image);
      }

      mem_unlock(convData->heglimage);
   }

   /* no change, but the flush was deferred anyway */
   khrn_hw_flush_dcache_range(convData->controlList, convData->numCLBytes);

   cbRecord->reason = KHRN_TFCONVERT_DONE;

   return !abandon;
}

/* BEWARE: This is a callback function and is on another thread to the main API code
 * Anything you do in these functions must not interfere with state that might be being used on the main API thread.
 */
void khrn_tfconvert_done(BEGL_HWCallbackRecord *cbRecord)
{
   CONVERT_CALLBACK_DATA_T *convData = (CONVERT_CALLBACK_DATA_T *)&cbRecord->payload[0];

   // We're done with the image, so unlock it
   if (convData)
   {
      if (convData->heglimage != MEM_HANDLE_INVALID)
      {
         EGL_IMAGE_T *eglimage = mem_lock(convData->heglimage, NULL);

         if (!convData->conversionKilled)
         {
            // Release our lock
            if (convData->semaphoreTaken)
            {
               vcos_semaphore_wait_timeout(&eglimage->lockSemaphore, 0);
               vcos_semaphore_post(&eglimage->lockSemaphore);

               // Clear the 'missed' flag mutex, since we've now done the conversion
               vcos_semaphore_wait_timeout(&eglimage->missedConvFlag, 0);
               vcos_semaphore_post(&eglimage->missedConvFlag);
            }

            INCR_DRIVER_COUNTER(hw_tf_conversions);
         }

         vcos_mutex_lock(&eglimage->dirtyBitsMutex);
         eglimage->completed++;
         vcos_mutex_unlock(&eglimage->dirtyBitsMutex);

         mem_unlock(convData->heglimage);
         MEM_ASSIGN(convData->heglimage, MEM_HANDLE_INVALID);
      }

      if (convData->fmem != NULL)
         khrn_job_done_fmem(convData->fmem);
   }

   free(cbRecord);
}

bool khrn_rso_to_tf_convert(MEM_HANDLE_T heglimage, MEM_HANDLE_T srcImg, MEM_HANDLE_T dstImg)
{
   KHRN_FMEM_T             *fmem;
   uint8_t                 *p, *listPtr;
   uint32_t                i, j, b;
   bool                    store;
   KHRN_IMAGE_T            *src;
   KHRN_IMAGE_T            *dst;
   uint32_t                xTiles;
   uint32_t                yTiles;
   uint32_t                numDirtyTiles;
   uint32_t                numBytes;
   bool                    res;
   EGL_IMAGE_T             *eglimage;
   bool                    secure;

   if ((heglimage == MEM_INVALID_HANDLE) || (dstImg == MEM_HANDLE_INVALID))
      return false;

   eglimage = (EGL_IMAGE_T *)mem_lock(heglimage, NULL);

   if (srcImg != MEM_HANDLE_INVALID)
      src = (KHRN_IMAGE_T *)mem_lock(srcImg, NULL);
   else
   {
      if (eglimage->mh_image != MEM_HANDLE_INVALID)
         src = (KHRN_IMAGE_T *)mem_lock(eglimage->mh_image, NULL);
      else
      {
         mem_unlock(heglimage);
         return false;
      }
   }

   /* if a YUV image has come through this path, ignore */
   if (get_src_format(src->format) == TLB_INVALID_FORMAT)
   {
      mem_unlock(srcImg);
      mem_unlock(heglimage);
      return false;
   }

   dst = (KHRN_IMAGE_T *)mem_lock(dstImg, NULL);

   secure = src->secure;
   /* if the src is secure, the destination must also be */
   if (secure)
      vcos_demand(dst->secure == true);
   else
      /* its allowable to raise security */
      secure = dst->secure;

   xTiles = (src->width  + 63) / 64;
   yTiles = (src->height + 63) / 64;

   vcos_assert(src->width == dst->width && src->height == dst->height);

   vcos_mutex_lock(&eglimage->dirtyBitsMutex);

   if ((eglimage->queued > eglimage->completed) ||
       (!eglimage->forceConversion && !egl_image_any_tile_dirty(eglimage)))
   {
      vcos_mutex_unlock(&eglimage->dirtyBitsMutex);
      mem_unlock(heglimage);
      return true;
   }

   fmem = khrn_fmem_init(KHRN_INTERLOCK_USER_TEMP);
   if (!fmem)
   {
      vcos_mutex_unlock(&eglimage->dirtyBitsMutex);
      mem_unlock(heglimage);
      return false;
   }

   if (!khrn_fmem_start_render(fmem))
   {
      khrn_fmem_discard(fmem);
      vcos_mutex_unlock(&eglimage->dirtyBitsMutex);
      mem_unlock(heglimage);
      return false;
   }

   if (srcImg == MEM_HANDLE_INVALID)
      eglimage->queued++;

   vcos_mutex_unlock(&eglimage->dirtyBitsMutex);

   numDirtyTiles = xTiles * yTiles;

   numBytes = 11 + (numDirtyTiles * 11);
   if (numBytes < NULL_LIST_SIZE)
      numBytes = NULL_LIST_SIZE;    /* Must have at least this many bytes in case we need a NULL list */

   listPtr = p = khrn_fmem_cle(fmem, numBytes);
   if (p)
   {
      /* set rendering mode config */
      ADD_BYTE(p, KHRN_HW_INSTR_STATE_TILE_RENDERING_MODE);
      khrn_fmem_add_fix(fmem, &p, dst->mh_storage, 0);

      ADD_SHORT(p, src->width);
      ADD_SHORT(p, src->height);
      ADD_BYTE(p,
         (0 << 0) | /* disable ms mode */
         (0 << 1) | /* disable 64-bit color mode */
         (get_dest_format(dst->format) << 2) |
         (0 << 4) | /* No decimation */
         (get_type_flag(dst->format) << 6));

      ADD_BYTE(p,
         (0 << 0) | /* no vg mask */
         (0 << 1) | /* no coverage mode */
         (0 << 2) | /* don't care z-update dir */
         (0 << 3) | /* disable early Z */
         (0 << 4)); /* no double buffer */

      store = false;
      for (j = 0; j != yTiles; ++j)
      {
         for (i = 0; i != xTiles; ++i)
         {
            /* We write the store for the previous tile here as we're not storing all tiles.
            * We can then add the final store with EOF after the loop */
            if (store)
               ADD_BYTE(p, KHRN_HW_INSTR_STORE_SUBSAMPLE);

            ADD_BYTE(p, KHRN_HW_INSTR_LOAD_GENERAL);
            ADD_BYTE(p, (uint8_t)(
               (1 << 0) | /* load color */
               (get_type_flag(src->format) << 4)));

            ADD_BYTE(p, get_src_format(src->format));
            khrn_fmem_add_fix(fmem, &p, src->mh_storage, 0);

            ADD_BYTE(p, KHRN_HW_INSTR_STATE_TILE_COORDS);
            ADD_BYTE(p, (uint8_t)i);
            ADD_BYTE(p, (uint8_t)j);

            store = true;
         }
      }

      /* Add the final store with EOF */
      ADD_BYTE(p, KHRN_HW_INSTR_STORE_SUBSAMPLE_EOF);

      /* Pad end with NOP's if necessary */
      for (b = 11 + (numDirtyTiles * 11); b < numBytes; b++)
         ADD_BYTE(p, KHRN_HW_INSTR_NOP);

      if (srcImg != MEM_HANDLE_INVALID)
         mem_unlock(srcImg);
      else
         mem_unlock(eglimage->mh_image);
      mem_unlock(dstImg);

      khrn_issue_tfconvert_job(fmem, heglimage, listPtr, numBytes, secure);

      mem_unlock(heglimage);

      res = true;
   }
   else
   {
      mem_unlock(eglimage->mh_image);
      mem_unlock(heglimage);
      mem_unlock(dstImg);

      res = false;
   }

   return res;
}
