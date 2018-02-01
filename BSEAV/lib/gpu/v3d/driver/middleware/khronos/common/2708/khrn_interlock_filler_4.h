/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_util.h"

/* users are render states. user = 1 << render state index */
typedef enum {
   KHRN_INTERLOCK_USER_NONE    = 0,
   KHRN_INTERLOCK_USER_INVALID = 1 << 29, /* <= 29 render states, so can use top 3 bits for temp/writing/inv */
   KHRN_INTERLOCK_USER_TEMP    = 1 << 30,
   KHRN_INTERLOCK_USER_WRITING = 1 << 31
} KHRN_INTERLOCK_USER_T;
static inline KHRN_INTERLOCK_USER_T khrn_interlock_user(uint32_t i) { return (KHRN_INTERLOCK_USER_T)(1 << i); }
static inline uint32_t khrn_interlock_render_state_i(KHRN_INTERLOCK_USER_T user) { return _msb(user); }
