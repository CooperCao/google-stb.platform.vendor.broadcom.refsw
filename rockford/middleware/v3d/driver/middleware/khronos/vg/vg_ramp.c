/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG ramp

FILE DESCRIPTION
VG ramp allocation and generation implementation.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/vg/vg_ramp.h"
#include "interface/khronos/common/khrn_int_color.h"
#include "interface/khronos/common/khrn_int_math.h"
#include "interface/khronos/common/khrn_int_util.h"

/*
   list of the VG_RAMP_Ts that have between 1 and VG_RAMP_HEIGHT - 1 (inclusive)
   unused slots. vg_ramp_alloc will fill these VG_RAMP_Ts before allocating more
*/

static MEM_HANDLE_T free_head_handle = MEM_INVALID_HANDLE;

static VCOS_MUTEX_T mutex;
static bool mutex_created = false;

bool vg_ramp_alloc(
   MEM_HANDLE_T *handle_out,
   uint32_t *i_out)
{
   MEM_HANDLE_T handle;
   uint32_t i;
   if (free_head_handle == MEM_INVALID_HANDLE) {
      MEM_HANDLE_T data_handle;
      VG_RAMP_T *ramp;
      uint32_t j;

      if (mutex_created == false) {
         /* Create a mutex to protect the ramp mem list*/
         if (!vcos_verify(vcos_mutex_create(&mutex, "vg_ramp") == VCOS_SUCCESS)) {
            return false;
         }
         mutex_created = true;
      }

      /*
         alloc struct
      */

      handle = MEM_ALLOC_STRUCT_EX(VG_RAMP_T, MEM_COMPACT_DISCARD);
      if (handle == MEM_INVALID_HANDLE) {
         return false;
      }

      /*
         alloc data
      */

      data_handle = mem_alloc_ex(
         0, /* don't actually allocate the data now, just get a handle */
         VG_RAMP_ALIGN,
         (MEM_FLAG_T)(MEM_FLAG_DISCARDABLE | MEM_FLAG_DIRECT | MEM_FLAG_RESIZEABLE | MEM_FLAG_NO_INIT),
         "VG_RAMP_T.data",
         MEM_COMPACT_DISCARD);
      if (data_handle == MEM_INVALID_HANDLE) {
         mem_release(handle);
         return false;
      }

      /*
         fill in struct
      */

      vcos_mutex_lock(&mutex);

      ramp = (VG_RAMP_T *)mem_lock(handle, NULL);

      i = 0; /* arbitrarily put this ramp in the first slot */
      for (j = 0; j != VG_RAMP_HEIGHT; ++j) {
         ramp->ref_counts[j] = (j == i) ? 1 : 0;
         ramp->valids[j] = false; /* not strictly necessary */
      }

      ramp->data = data_handle;

      /* link into free list (which is empty) */
      ramp->free_prev = MEM_INVALID_HANDLE;
      ramp->free_next = MEM_INVALID_HANDLE;
      free_head_handle = handle;
      vcos_mutex_unlock(&mutex);

      mem_unlock(handle);
   } else {
      VG_RAMP_T *ramp;
      uint32_t j;

      vcos_mutex_lock(&mutex);

      handle = free_head_handle;
      ramp = (VG_RAMP_T *)mem_lock(handle, NULL);

      /*
         find an unused slot
      */

      for (i = 0; ramp->ref_counts[i] != 0; ++i) ;
      vcos_assert(i < VG_RAMP_HEIGHT); /* must be one or this VG_RAMP_T wouldn't be in the free list */

      /*
         remove from the free list if there are no more unused slots
      */

      for (j = i + 1; (j != VG_RAMP_HEIGHT) && (ramp->ref_counts[j] != 0); ++j) ; /* find next unused slot */
      if (j == VG_RAMP_HEIGHT) {
         vcos_assert(free_head_handle == handle);
         free_head_handle = ramp->free_next;
         if (free_head_handle != MEM_INVALID_HANDLE) {
            VG_RAMP_T *head = (VG_RAMP_T *)mem_lock(free_head_handle, NULL);
            vcos_assert(head->free_prev == handle);
            head->free_prev = MEM_INVALID_HANDLE;
            mem_unlock(free_head_handle);
         }
      }

      /*
         update rest of struct
      */

      ramp->ref_counts[i] = 1;
      ramp->valids[i] = false;

      mem_unlock(handle);

      vcos_mutex_unlock(&mutex);
   }

   *handle_out = handle;
   *i_out = i;
   return true;
}

void vg_ramp_acquire(
   MEM_HANDLE_T handle,
   uint32_t i)
{
   VG_RAMP_T *ramp;
   vcos_mutex_lock(&mutex);

   ramp = (VG_RAMP_T *)mem_lock(handle, NULL);
   vcos_assert(ramp->ref_counts[i] != 0);
   ++ramp->ref_counts[i];
   mem_unlock(handle);

   vcos_mutex_unlock(&mutex);
}

void vg_ramp_release(
   MEM_HANDLE_T handle,
   uint32_t ref_count_index)
{
   VG_RAMP_T *ramp;
   vcos_mutex_lock(&mutex);

   ramp = (VG_RAMP_T *)mem_lock(handle, NULL);
   vcos_assert(ramp->ref_counts[ref_count_index] != 0);
   if (--ramp->ref_counts[ref_count_index] == 0) {
      uint32_t c = 0;
      uint32_t i;
      for (i = 0; i != VG_RAMP_HEIGHT; ++i) {
         if (ramp->ref_counts[i] != 0) {
            ++c;
         }
      }
      if (c == 0) {
         /*
            no used slots, remove from free list
         */

         if (ramp->free_prev == MEM_INVALID_HANDLE) {
            /* Remove this handle from the free list
             * from the beginning of the list
             */
            vcos_assert(free_head_handle == handle);
            free_head_handle = ramp->free_next;
         } else {
            /* Remove this handle from the free list
             * from the middle of the list
             */
            VG_RAMP_T *prev = (VG_RAMP_T *)mem_lock(ramp->free_prev, NULL);
            vcos_assert(prev->free_next == handle);
            prev->free_next = ramp->free_next;
            mem_unlock(ramp->free_prev);
         }
         if (ramp->free_next != MEM_INVALID_HANDLE) {
            /* Update the handle to the previous of the next handle
             * to not point to the handle which has been removed
             */
            VG_RAMP_T *next = (VG_RAMP_T *)mem_lock(ramp->free_next, NULL);
            vcos_assert(next->free_prev == handle);
            next->free_prev = ramp->free_prev;
            mem_unlock(ramp->free_next);
         }

         /*
            release
         */

         mem_release(ramp->data);
         mem_unlock(handle);
         mem_release(handle);
      } else if (c == (VG_RAMP_HEIGHT - 1)) {
         /*
            was full, now has unused slot, add to free list
         */

         /* Add the handle at the beginning of the free list */
         ramp->free_prev = MEM_INVALID_HANDLE;
         ramp->free_next = free_head_handle;

         /* If the free list was not empty */
         if (free_head_handle != MEM_INVALID_HANDLE) {
            /* Point the handle previous of the original head
             * to the new free list head which is this handle
             */
            VG_RAMP_T *head = (VG_RAMP_T *)mem_lock(free_head_handle, NULL);
            head->free_prev = handle;
            mem_unlock(free_head_handle);
         }
         /* Make this handle the head of the free list */
         free_head_handle = handle;

         mem_unlock(handle);
      } else {
         /*
            some used slots, some unused slots, already in free list
         */

         mem_unlock(handle);
      }
   } else {
      mem_unlock(handle);
   }

   vcos_mutex_unlock(&mutex);
}

static const float VG_RAMP_DEFAULT_STOPS[] = {
   0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
   1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

static void fixup_stops(
   const float **stops_io, uint32_t *stops_count_io)
{
   const float *stops = *stops_io;
   uint32_t stops_count = *stops_count_io;

   /*
      the spec says:
      - stops < 0 or > 1 are ignored.
      - if stops are not monotonic increasing, entire sequence should be ignored.
      - if there are no valid stops, the default stops should be used.
   */

   float prev_t = 0.0f;
   int32_t first_i = -1; /* index of first valid stop */
   uint32_t last_i = 0;  /* index of last valid stop */
   uint32_t j;
   for (j = 0; j != stops_count; j += 5) {
      float t = stops[j];
      if ((j != 0) && (t < prev_t)) {
         /* not monotonic increasing -- entire sequence should be ignored */
         first_i = -1; /* no valid stops */
         break;
      }
      if ((t >= 0.0f) && (t <= 1.0f)) {
         /* valid stop */
         if (first_i == -1) { first_i = j; }
         last_i = j;
      }
      prev_t = t;
   }
   if (first_i == -1) {
      /* no valid stops */
      *stops_io = VG_RAMP_DEFAULT_STOPS;
      *stops_count_io = ARR_COUNT(VG_RAMP_DEFAULT_STOPS);
   } else {
      /* chop off invalid stops */
      *stops_io = stops + first_i;
      *stops_count_io = (last_i - first_i) + 5;
   }
}

static INLINE void get_stop(float *t, float *r, float *g, float *b, float *a, bool pre, const float *stop)
{
   *t = stop[0];
   *r = stop[1];
   *g = stop[2];
   *b = stop[3];
   *a = stop[4];
   if (pre) {
      *r *= *a;  *g *= *a;  *b *= *a;
   }
}

static INLINE void put_rgba(uint32_t *data, uint32_t i, uint32_t j, uint32_t rgba)
{
   /* 32-bit lineartile */
   vcos_static_assert(VG_RAMP_HEIGHT == 4);
   data[((j >> 2) * 16) + (i * 4) + (j & 0x3)] = rgba;
}

/*
   1st pixel is colour of first ramp stop
   2nd pixel is single sample at 1.0f / VG_RAMP_WIDTH
   3rd pixel is single sample at 2.0f / VG_RAMP_WIDTH
   ...
   last pixel is colour of last ramp stop

   texture s coordinate should be g(x, y) + (0.5f / VG_RAMP_WIDTH)
*/

static void vg_ramp_gen(
   uint32_t *data,
   uint32_t i,
   bool pre, /* premultiply for interpolation? */
   const float *stops, uint32_t stops_count) /* stops_count must be a multiple of 5 */
{
   float t1, r1, g1, b1, a1;
   uint32_t k;
   float t0, r0, g0, b0, a0;
   float oo_t1_m_t0;
   uint32_t j;

   vcos_assert((stops_count % 5) == 0);
   fixup_stops(&stops, &stops_count);

   /*
      set the first pixel of the ramp to the colour of the first stop and the
      last pixel of the ramp to the colour of the last stop so pad tiling mode
      will work correctly
   */

   {
      float t, r, g, b, a;
      get_stop(&t, &r, &g, &b, &a, false, stops + 0); /* first */
      put_rgba(data, i, 0, color_floats_to_rgba(r, g, b, a));
      get_stop(&t, &r, &g, &b, &a, false, stops + (stops_count - 5)); /* last */
      put_rgba(data, i, VG_RAMP_WIDTH - 1, color_floats_to_rgba(r, g, b, a));
   }

   /*
      now generate the rest of the pixels

      the spec says:
      - if there is no stop at 0, an implicit stop is added with the same color
        as the first valid stop.
      - if there is no stop at 1, an implicit stop is added with the same color
        as the last valid stop.
   */

   get_stop(&t1, &r1, &g1, &b1, &a1, pre, stops + 0); /* 1 = first */
   k = 5; /* index of next stop */
   t0 = 0.0f; r0 = r1; g0 = g1; b0 = b1; a0 = a1; /* 0 = same color as 1, but at 0 */
   oo_t1_m_t0 = (t1 == 0.0f) ? 0.0f : recip_(t1); /* won't be used if t1 is 0, but want to avoid ever calling recip_ with 0 */

   for (j = 1; j != (VG_RAMP_WIDTH - 1); ++j) { /* already set the first and last pixels */
      float t, u, omu, r, g, b, a;

      t = (1.0f / (float)VG_RAMP_WIDTH) * (float)j; /* in [0, 1) */

      while (t1 <= t) { /* exclude endpoint */
         float t1_m_t0;
         t0 = t1;  r0 = r1;  g0 = g1;  b0 = b1;  a0 = a1; /* 0 = 1 */
         if (k == stops_count) {
            t1 = 1.0f; /* 1 = same color but at 1 */
         } else {
            get_stop(&t1, &r1, &g1, &b1, &a1, pre, stops + k); /* 1 = next */
            k += 5;
         }
         t1_m_t0 = t1 - t0;
         oo_t1_m_t0 = (t1_m_t0 == 0.0f) ? 0.0f : recip_(t1_m_t0); /* won't be used if (t1 - t0) is 0, but want to avoid ever calling recip_ with 0 */
      }

      u = _minf((t - t0) * oo_t1_m_t0, 1.0f);
      omu = 1.0f - u;
      #define I(X, Y) (((X) * omu) + ((Y) * u))
      r = I(r0, r1); g = I(g0, g1); b = I(b0, b1); a = I(a0, a1);
      #undef I
      if (pre) {
         float oo_a = (a < EPS) ? 0.0f : recip_(a);
         r *= oo_a;  g *= oo_a;  b *= oo_a;
      }

      put_rgba(data, i, j, color_floats_to_rgba(r, g, b, a));
   }
}

uint32_t vg_ramp_get_rgba(
   MEM_HANDLE_T stops_handle,
   bool last) /* else first */
{
   const float *stops;
   uint32_t stops_count;
   float t, r, g, b, a;

   vcos_mutex_lock(&mutex);

   if (stops_handle == MEM_INVALID_HANDLE) {
      stops = NULL;
      stops_count = 0;
   } else {
      stops = (const float *)mem_lock(stops_handle, NULL);
      stops_count = mem_get_size(stops_handle) / sizeof(float);
   }
   vcos_assert((stops_count % 5) == 0);
   fixup_stops(&stops, &stops_count);
   get_stop(&t, &r, &g, &b, &a, false, stops + (last ? (stops_count - 5) : 0));
   if (stops_handle != MEM_INVALID_HANDLE) {
      mem_unlock(stops_handle);
   }

   vcos_mutex_unlock(&mutex);

   return color_floats_to_rgba(r, g, b, a);
}

bool vg_ramp_retain(
   MEM_HANDLE_T handle,
   uint32_t i,
   bool pre,
   MEM_HANDLE_T stops_handle)
{
   VG_RAMP_T *ramp;
   vcos_mutex_lock(&mutex);

   ramp = (VG_RAMP_T *)mem_lock(handle, NULL);
   vcos_assert(ramp->ref_counts[i] != 0);

   /*
      reinstate storage if it was discarded
   */

   if (!mem_retain(ramp->data)) {
      uint32_t j;

      if (!mem_resize_ex(ramp->data, VG_RAMP_SIZE, MEM_COMPACT_DISCARD)) {
         mem_unretain(ramp->data);
         mem_unlock(handle);
         vcos_mutex_unlock(&mutex);
         return false;
      }

      for (j = 0; j != VG_RAMP_HEIGHT; ++j) {
         ramp->valids[j] = false;
      }
   }

   /*
      ensure slot is valid
   */

   if (!ramp->valids[i]) {
      const float *stops;
      uint32_t stops_count;
      if (stops_handle == MEM_INVALID_HANDLE) {
         stops = NULL;
         stops_count = 0;
      } else {
         stops = (const float *)mem_lock(stops_handle, NULL);
         stops_count = mem_get_size(stops_handle) / sizeof(float);
      }
      vg_ramp_gen((uint32_t *)mem_lock(ramp->data, NULL), i, pre, stops, stops_count);
      mem_unlock(ramp->data);
      if (stops_handle != MEM_INVALID_HANDLE) {
         mem_unlock(stops_handle);
      }
      ramp->valids[i] = true;
   }

   mem_unlock(handle);
   vcos_mutex_unlock(&mutex);
   return true;
}

void vg_ramp_unretain(
   MEM_HANDLE_T handle)
{
   VG_RAMP_T *ramp;
   vcos_mutex_lock(&mutex);

   ramp = (VG_RAMP_T *)mem_lock(handle, NULL);
   mem_unretain(ramp->data);
   mem_unlock(handle);

   vcos_mutex_unlock(&mutex);
}
