/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Interlock system

FILE DESCRIPTION
Platform-specific stuff. This should only be included by ../khrn_interlock.h.
=============================================================================*/

#ifndef KHRN_INTERLOCK_FILLER_4_H
#define KHRN_INTERLOCK_FILLER_4_H

#include "interface/khronos/common/khrn_int_util.h"

/* users are render states. user = 1 << render state index */
typedef enum {
   KHRN_INTERLOCK_USER_NONE    = 0,
   KHRN_INTERLOCK_USER_INVALID = 1 << 29, /* <= 29 render states, so can use top 3 bits for temp/writing/inv */
   KHRN_INTERLOCK_USER_TEMP    = 1 << 30,
   KHRN_INTERLOCK_USER_WRITING = 1 << 31
} KHRN_INTERLOCK_USER_T;
static INLINE KHRN_INTERLOCK_USER_T khrn_interlock_user(uint32_t i) { return (KHRN_INTERLOCK_USER_T)(1 << i); }
static INLINE uint32_t khrn_interlock_render_state_i(KHRN_INTERLOCK_USER_T user) { return _msb(user); }

#endif
