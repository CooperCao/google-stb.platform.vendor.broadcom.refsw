/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/common/2708/khrn_interlock_priv_4.h"
#include "middleware/khronos/common/2708/khrn_render_state_4.h"
#include "middleware/khronos/common/khrn_hw.h"

#define POS_WHO_SHIFT     62
#define POS_WHO_MASK      (0x3ull << POS_WHO_SHIFT)
#define POS_WHO_HW_BIN    (0ull << POS_WHO_SHIFT)
#define POS_WHO_HW_RENDER (1ull << POS_WHO_SHIFT)
#define POS_WHO_WORKER    (2ull << POS_WHO_SHIFT)

void khrn_interlock_read_immediate(KHRN_INTERLOCK_T *interlock)
{
   khrn_interlock_read(interlock, KHRN_INTERLOCK_USER_NONE);
   khrn_wait_for_job_done(interlock->pos);
}

void khrn_interlock_write_immediate(KHRN_INTERLOCK_T *interlock)
{
   khrn_interlock_write(interlock, KHRN_INTERLOCK_USER_NONE);
   khrn_wait_for_job_done(interlock->pos);
}

void khrn_interlock_flush(KHRN_INTERLOCK_USER_T user)
{
   assert(_count(user) == 1);
   khrn_render_state_flush(khrn_interlock_render_state_i(user));
}

bool khrn_interlock_would_flush(KHRN_INTERLOCK_USER_T user)
{
   assert(_count(user) == 1);
   return khrn_render_state_would_flush(khrn_interlock_render_state_i(user));
}

void khrn_interlock_transfer(KHRN_INTERLOCK_T *interlock, KHRN_INTERLOCK_USER_T user, KHRN_INTERLOCK_FIFO_T fifo)
{
   if (fifo != KHRN_INTERLOCK_FIFO_WORKER)
      interlock->pos = khrn_get_last_issued_seq() + 1;
   khrn_interlock_release(interlock, user);
}
