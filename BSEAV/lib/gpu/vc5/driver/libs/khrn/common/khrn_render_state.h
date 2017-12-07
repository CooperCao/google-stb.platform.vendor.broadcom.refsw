/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_RENDER_STATE_4_H
#define KHRN_RENDER_STATE_4_H
#include "khrn_types.h"
#include <stdbool.h>
#include "../glxx/glxx_hw.h"
#include "../glxx/glxx_compute_render_state.h"

// max render states limited by num bits in khrn_render_state_set_t
#define MAX_RENDER_STATES (8*sizeof(khrn_render_state_set_t))

typedef enum
{
   KHRN_RENDER_STATE_TYPE_NONE,
   KHRN_RENDER_STATE_TYPE_GLXX,
   KHRN_RENDER_STATE_TYPE_GLXX_COMPUTE
}
khrn_render_state_type_t;

typedef enum khrn_render_state_flush_state
{
    KHRN_RENDER_STATE_FLUSH_ALLOWED,
    KHRN_RENDER_STATE_FLUSH_DISALLOWED,
    KHRN_RENDER_STATE_FLUSHING
} khrn_render_state_flush_state;

struct khrn_render_state
{
   /*
    * The data goes at the top, so it is possible to cast back from a
    * GLXX_HW_RENDER_STATE_T * to a khrn_render_state * for example if you
    * need to so don't break that.
    */
   union
   {
      GLXX_HW_RENDER_STATE_T glxx;
#if V3D_VER_AT_LEAST(3,3,0,0)
      glxx_compute_render_state glxx_compute;
#endif
   }
   data;

   /*
    * The higher this number the more recently this render state was
    * allocated. We use then when deciding which one to evict.
    */
   unsigned allocated_order;
   khrn_render_state_type_t type;
   khrn_render_state_flush_state flush_state;
};

extern khrn_render_state render_states[MAX_RENDER_STATES];

/* Allocate a render state of the specified type. May flush one to make room */
extern khrn_render_state *khrn_render_state_new(
      khrn_render_state_type_t type);

static inline GLXX_HW_RENDER_STATE_T *khrn_render_state_get_glxx(khrn_render_state *rs);

#if V3D_VER_AT_LEAST(3,3,0,0)
static inline glxx_compute_render_state* khrn_render_state_get_glxx_compute(khrn_render_state* rs);
#endif

//! Called by child object prior to flush.
static inline void khrn_render_state_begin_flush(khrn_render_state* rs);

//! Called to disable flushing on this render-state.
static inline void khrn_render_state_disallow_flush(khrn_render_state* rs);

//! Called to re-enable flushing on this render-state.
static inline void khrn_render_state_allow_flush(khrn_render_state* rs);

extern void khrn_render_state_flush(khrn_render_state *rs);

/* Flush all render-states of type GLXX */
extern void khrn_render_state_flush_all(void);

/* Flush the oldest existing render state that is possible to flush.
 * Returns true if a render state was flushed. */
extern bool khrn_render_state_flush_oldest_possible(void);

extern void khrn_render_state_delete(khrn_render_state *rs);

//! Return true if rs is in the set.
static inline bool khrn_render_state_set_contains(khrn_render_state_set_t set, const void *rs);

//! Add rs to set, return false if rs was already in the set.
static inline bool khrn_render_state_set_add(khrn_render_state_set_t *set, const void *rs);

//! Remove rs from set.
static inline void khrn_render_state_set_remove(khrn_render_state_set_t *set, const void *rs);

//! Return a render state from set and remove it, or NULL if there aren't any.
extern khrn_render_state *khrn_render_state_set_pop(khrn_render_state_set_t *set);

/* returns true is a is a subset of b */
static inline bool khrn_render_state_set_is_subset(const khrn_render_state_set_t *b,
      const khrn_render_state_set_t *a);

/*
 * Iterate through the render states from the set and on each of them call cb
 * with khrn_render_state and the passed in param
 * Returns false if any of the callbacks rerturns false;
 */
typedef bool (*RENDERSTATESET_CB_T)(khrn_render_state *rs, void *param);
bool khrn_render_state_set_iterate_cb(const khrn_render_state_set_t *set,
      RENDERSTATESET_CB_T cb, void *param);


/* For debugging */
extern unsigned khrn_hw_render_state_allocated_order(const GLXX_HW_RENDER_STATE_T *hw_rs);

/* For each hw/glxx renderstate, execute the callback function. If a callback
 * returns true, it is considered and early exit and we stop iterating through
 * the rest of the render states; Note: If iterating through all the glxx
 * render states is needed, the callback function must return false in all
 * cases*/
typedef bool (*glxx_hw_rs_foreach_callback)(GLXX_HW_RENDER_STATE_T *hw_rs, void *data);
extern void glxx_hw_render_state_foreach(glxx_hw_rs_foreach_callback callback, void *data);

extern bool khrn_render_state_is_valid(const khrn_render_state *rs);

/* record a fence for render state. The fence will be signalled when all the
 * users of that fene will complete */
extern bool khrn_render_state_record_fence_to_signal(khrn_render_state *rs,
      khrn_fence *fence);

#include "khrn_render_state.inl"

#endif
