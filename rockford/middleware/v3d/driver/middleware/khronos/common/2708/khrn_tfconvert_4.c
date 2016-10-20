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
#include "middleware/khronos/common/2708/khrn_interlock_priv_4.h"
#include "middleware/khronos/glxx/2708/glxx_inner_4.h"
#include "middleware/khronos/common/2708/khrn_render_state_4.h"

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

void khrn_tfconvert_logevents(BEGL_HWNotification *notification)
{
#ifdef TIMELINE_EVENT_LOGGING
   EventData               ev;

   /* not a real copy, just for output in spyhook so no reference counting required */
   ev.eventData = 0;
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
void khrn_tfconvert_done(BEGL_HWCallbackRecord *cbRecord)
{
   KHRN_FMEM_T    *fmem = (KHRN_FMEM_T*)cbRecord->payload[0];

   khrn_job_done_fmem(fmem);

   free(cbRecord);
}

bool khrn_rso_to_tf_convert(GLXX_SERVER_STATE_T *state, MEM_HANDLE_T hsrc, MEM_HANDLE_T hdst)
{
   vcos_demand(hsrc != MEM_HANDLE_INVALID);
   vcos_demand(hdst != MEM_HANDLE_INVALID);

   KHRN_IMAGE_T *src = (KHRN_IMAGE_T *)mem_lock(hsrc, NULL);

   /* if a YUV image has come through this path, ignore */
   if (get_src_format(src->format) == TLB_INVALID_FORMAT)
   {
      mem_unlock(hsrc);
      return false;
   }

   KHRN_IMAGE_T *dst = (KHRN_IMAGE_T *)mem_lock(hdst, NULL);

   bool secure = src->secure;
   /* if the src is secure, the destination must also be */
   if (secure)
      vcos_demand(dst->secure == true);
   else
      /* its allowable to raise security */
      secure = dst->secure;

   uint32_t x_tiles = (src->width + 63) / 64;
   uint32_t y_tiles = (src->height + 63) / 64;

   vcos_assert(src->width == dst->width && src->height == dst->height);

   KHRN_FMEM_T *fmem = khrn_fmem_init(KHRN_INTERLOCK_USER_TEMP);
   if (!fmem)
   {
      mem_unlock(hsrc);
      mem_unlock(hdst);
      return false;
   }

   if (!khrn_fmem_start_render(fmem))
   {
      khrn_fmem_discard(fmem);
      mem_unlock(hsrc);
      mem_unlock(hdst);
      return false;
   }

   uint32_t dirty_tiles = x_tiles * y_tiles;
   uint32_t bytes = 11 + (dirty_tiles * 11);
   if (bytes < NULL_LIST_SIZE)
      bytes = NULL_LIST_SIZE;    /* Must have at least this many bytes in case we need a NULL list */

   bool res = false;
   uint8_t *p;
   uint8_t *list = p = khrn_fmem_cle(fmem, bytes);
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

      bool store = false;
      uint32_t y;
      for (y = 0; y != y_tiles; y++)
      {
         uint32_t x;
         for (x = 0; x != x_tiles; x++)
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
            ADD_BYTE(p, (uint8_t)x);
            ADD_BYTE(p, (uint8_t)y);

            store = true;
         }
      }

      /* Add the final store with EOF */
      ADD_BYTE(p, KHRN_HW_INSTR_STORE_SUBSAMPLE_EOF);

      /* Pad end with NOP's if necessary */
      uint32_t b;
      for (b = 11 + (dirty_tiles * 11); b < dirty_tiles; b++)
         ADD_BYTE(p, KHRN_HW_INSTR_NOP);

#ifdef KHRN_AUTOCLIF
      GLXX_HW_RENDER_STATE_T *rs = (GLXX_HW_RENDER_STATE_T *)khrn_render_state_get_data(state->current_render_state);
      khrn_interlock_transfer(&dst->interlock, khrn_interlock_user(rs->name), KHRN_INTERLOCK_FIFO_HW_RENDER);
#else
      UNUSED(state);
#endif

      khrn_issue_tfconvert_job(fmem, secure);

#ifdef KHRN_AUTOCLIF
      khrn_interlock_read_immediate(&dst->interlock);
#endif

      res = true;
   }

   mem_unlock(hsrc);
   mem_unlock(hdst);

   return res;
}