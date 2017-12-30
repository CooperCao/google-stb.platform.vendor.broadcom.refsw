/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

typedef enum {
   KHRN_RENDER_STATE_TYPE_NONE,
   KHRN_RENDER_STATE_TYPE_GLXX,
   KHRN_RENDER_STATE_TYPE_VG
} KHRN_RENDER_STATE_TYPE_T;

/* global init/term */
extern void khrn_render_state_init(void);
extern void khrn_render_state_term(void); /* will flush any unflushed render states */

/*
   allocates a render state of the specified type and returns the slot index
   this may have to flush a slot to make room
*/

extern uint32_t khrn_render_state_start(KHRN_RENDER_STATE_TYPE_T type);

/*
   queries
*/

extern KHRN_RENDER_STATE_TYPE_T khrn_render_state_get_type(uint32_t i);
extern void *khrn_render_state_get_data(uint32_t i);
extern uint32_t khrn_render_state_get_index_from_data(void *data);

/*
   take over a render state. the caller is responsible for ensuring the old data
   structure is cleaned up and the new data structure is setup correctly
*/

extern void khrn_render_state_usurp(uint32_t i, KHRN_RENDER_STATE_TYPE_T type);

/*
   flush will call a type-specific flush function which will flush the render
   state and then call khrn_render_state_finish to release the slot

   flush_all will call flush on all slots with the specified type (or just all
   slots if type is KHRN_RENDER_STATE_TYPE_NONE)

   flush_except will call flush on all slots except the specified one
*/

extern bool khrn_render_state_would_flush(uint32_t i);
extern void khrn_render_state_flush(uint32_t i);
extern void khrn_render_state_flush_all(KHRN_RENDER_STATE_TYPE_T type);
extern void khrn_render_state_flush_except(uint32_t i);

/*
   finish is the complement to start -- it will release the specified slot. this
   should only be called by the code that conceptually owns the slot (which
   should have released or transferred all resources owned by the render state)
*/

extern void khrn_render_state_finish(uint32_t i);
