/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once
#include "libs/platform/gmem.h"

typedef struct khrn_shared_tile_state
{
   uint32_t refs;          // bit31 is weak reference
   gmem_handle_t handle;
} khrn_shared_tile_state;

void khrn_tile_state_init(void);
void khrn_tile_state_deinit(void);

//! Allocate tile-state from gmem.
gmem_handle_t khrn_tile_state_alloc_gmem(size_t size, bool secure);

//! Allocate from shared tile-state memory. Assumed protected by GL lock.
khrn_shared_tile_state* khrn_tile_state_alloc_shared(size_t size, bool secure);

//! Release shared tile-state memory.
void khrn_tile_state_release_shared(khrn_shared_tile_state* mem);
