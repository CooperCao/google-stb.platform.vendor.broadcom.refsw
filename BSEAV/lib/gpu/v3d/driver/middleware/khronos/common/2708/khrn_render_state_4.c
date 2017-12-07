/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/2708/khrn_render_state_4.h"
#include "middleware/khronos/glxx/glxx_hw.h"
#include "middleware/khronos/glxx/2708/glxx_inner_4.h"
#include <stddef.h>

#define N 16 /* keep this <= 29 (the interlock stuff relies on this) and <= TESSS_N_MAX in vg/2708/tess.c */

typedef struct {
   KHRN_RENDER_STATE_TYPE_T type;
   union {
      GLXX_HW_RENDER_STATE_T glxx;
   } data;
} KHRN_RENDER_STATE_T;

static KHRN_RENDER_STATE_T render_states[N];
static uint32_t prefer = 0;

void khrn_render_state_init(void)
{
   uint32_t i;
   for (i = 0; i != N; ++i) {
      render_states[i].type = KHRN_RENDER_STATE_TYPE_NONE;
   }
}

void khrn_render_state_term(void)
{
#ifndef NDEBUG
   uint32_t i;
   for (i = 0; i != N; ++i) {
      assert(render_states[i].type == KHRN_RENDER_STATE_TYPE_NONE);
   }
#endif
}

uint32_t khrn_render_state_start(KHRN_RENDER_STATE_TYPE_T type)
{
   uint32_t i, j;

   assert(type != KHRN_RENDER_STATE_TYPE_NONE);

   prefer = (prefer == (N - 1)) ? 0 : (prefer + 1);

   for (i = 0, j = prefer; i != N; ++i, j = ((j == 0) ? (N - 1) : (j - 1))) {
      if (render_states[j].type == KHRN_RENDER_STATE_TYPE_NONE) {
         render_states[j].type = type;
         return j;
      }
   }

   khrn_render_state_flush(prefer);
   assert(render_states[prefer].type == KHRN_RENDER_STATE_TYPE_NONE);
   render_states[prefer].type = type;
   return prefer;
}

KHRN_RENDER_STATE_TYPE_T khrn_render_state_get_type(uint32_t i)
{
   assert(render_states[i].type != KHRN_RENDER_STATE_TYPE_NONE);
   return render_states[i].type;
}

void *khrn_render_state_get_data(uint32_t i)
{
   assert(render_states[i].type != KHRN_RENDER_STATE_TYPE_NONE);
   return &render_states[i].data;
}

uint32_t khrn_render_state_get_index_from_data(void *data)
{
   uint32_t i = (KHRN_RENDER_STATE_T *)((uint8_t *)data - offsetof(KHRN_RENDER_STATE_T, data)) - render_states;
   assert(i < N);
   return i;
}

void khrn_render_state_usurp(uint32_t i, KHRN_RENDER_STATE_TYPE_T type)
{
   assert(render_states[i].type != KHRN_RENDER_STATE_TYPE_NONE);
   render_states[i].type = type;
}

void khrn_render_state_flush(uint32_t i)
{
   switch (render_states[i].type) {
   /* TODO: error checking for GLXX! */
   case KHRN_RENDER_STATE_TYPE_GLXX:        glxx_hw_render_state_flush(&render_states[i].data.glxx); break;
   default:                                 UNREACHABLE();
   }
   assert(render_states[i].type == KHRN_RENDER_STATE_TYPE_NONE);
}

bool khrn_render_state_would_flush(uint32_t i)
{
   bool res = false;
   switch (render_states[i].type) {
   /* TODO: error checking for GLXX! */
   case KHRN_RENDER_STATE_TYPE_GLXX:        res = glxx_hw_render_state_would_flush(&render_states[i].data.glxx); break;
   default:                                 UNREACHABLE();
   }
   return res;
}


void khrn_render_state_flush_all(KHRN_RENDER_STATE_TYPE_T type)
{
   uint32_t i;
   for (i = 0; i != N; ++i) {
      if ((type == KHRN_RENDER_STATE_TYPE_NONE) ^ (render_states[i].type == type)) {
         khrn_render_state_flush(i);
      }
   }
}

void khrn_render_state_flush_except(uint32_t i)
{
   uint32_t j;
   for (j = 0; j != N; ++j) {
      if ((j != i) && (render_states[j].type != KHRN_RENDER_STATE_TYPE_NONE)) {
         khrn_render_state_flush(j);
      }
   }
}

void khrn_render_state_finish(uint32_t i)
{
   assert(render_states[i].type != KHRN_RENDER_STATE_TYPE_NONE);
   render_states[i].type = KHRN_RENDER_STATE_TYPE_NONE;
}
