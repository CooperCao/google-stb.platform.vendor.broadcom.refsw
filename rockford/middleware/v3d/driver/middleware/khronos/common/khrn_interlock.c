/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Interlock system

FILE DESCRIPTION
Interlock system to ensure correct ordering of reads/writes.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_interlock.h"

void khrn_interlock_init(KHRN_INTERLOCK_T *interlock)
{
   interlock->users = KHRN_INTERLOCK_USER_NONE;
   interlock->pos = 0;
}

void khrn_interlock_term(KHRN_INTERLOCK_T *interlock)
{
   interlock->users &= ~KHRN_INTERLOCK_USER_INVALID;
   vcos_assert(!interlock->users);
}

bool khrn_interlock_read(KHRN_INTERLOCK_T *interlock, KHRN_INTERLOCK_USER_T user) /* user allowed to be KHRN_INTERLOCK_USER_NONE */
{
   vcos_assert(_count(user) <= 1);
   if (!(interlock->users & user)) {
      if (interlock->users & KHRN_INTERLOCK_USER_WRITING) {
         vcos_assert(_count(interlock->users) == 2); /* exactly 1 writer (plus writing bit) */
         khrn_interlock_flush((KHRN_INTERLOCK_USER_T)(interlock->users & ~KHRN_INTERLOCK_USER_WRITING));
         vcos_assert(!interlock->users);
      }
      interlock->users = (KHRN_INTERLOCK_USER_T)(interlock->users | user);
      return true;
   }
   return false;
}

bool khrn_interlock_write(KHRN_INTERLOCK_T *interlock, KHRN_INTERLOCK_USER_T user) /* user allowed to be KHRN_INTERLOCK_USER_NONE */
{
   interlock->users &= ~KHRN_INTERLOCK_USER_INVALID;
   vcos_assert(_count(user) <= 1);
   if (!user || (~interlock->users & (user | KHRN_INTERLOCK_USER_WRITING))) {
      for (;;) {
         KHRN_INTERLOCK_USER_T other_users, other_user;
         other_users = (KHRN_INTERLOCK_USER_T)(interlock->users & ~user & ~KHRN_INTERLOCK_USER_WRITING);
         if (!other_users) { break; }
         other_user = (KHRN_INTERLOCK_USER_T)(1 << _msb(other_users));
         khrn_interlock_flush(other_user);
         vcos_assert(!(interlock->users & other_user));
      }
      if (user) {
         interlock->users = (KHRN_INTERLOCK_USER_T)(interlock->users | user | KHRN_INTERLOCK_USER_WRITING);
      }
      return true;
   }
   return false;
}

KHRN_INTERLOCK_USER_T khrn_interlock_get_writer(KHRN_INTERLOCK_T *interlock)
{
   vcos_assert(!(interlock->users & KHRN_INTERLOCK_USER_WRITING) || (_count(interlock->users) == 2));
   return (interlock->users & KHRN_INTERLOCK_USER_WRITING) ?
      (KHRN_INTERLOCK_USER_T)(interlock->users & ~KHRN_INTERLOCK_USER_WRITING) :
      KHRN_INTERLOCK_USER_NONE;
}

bool khrn_interlock_release(KHRN_INTERLOCK_T *interlock, KHRN_INTERLOCK_USER_T user)
{
   vcos_assert(_count(user) == 1);
   vcos_assert(!(interlock->users & KHRN_INTERLOCK_USER_WRITING) || (interlock->users == (user | KHRN_INTERLOCK_USER_WRITING)));
   if (interlock->users & user) {
      interlock->users = (KHRN_INTERLOCK_USER_T)(interlock->users & ~user & ~KHRN_INTERLOCK_USER_WRITING);
      return true;
   }
   return false;
}

bool khrn_interlock_write_would_block(KHRN_INTERLOCK_T *interlock)
{
   KHRN_INTERLOCK_USER_T other_users;
   other_users = (KHRN_INTERLOCK_USER_T)(interlock->users & ~KHRN_INTERLOCK_USER_WRITING & ~KHRN_INTERLOCK_USER_INVALID);
   return !!(other_users);
}

void khrn_interlock_invalidate(KHRN_INTERLOCK_T *interlock)
{
   khrn_interlock_write (interlock, KHRN_INTERLOCK_USER_NONE);
   interlock->users |= KHRN_INTERLOCK_USER_INVALID;
}

bool khrn_interlock_is_invalid(KHRN_INTERLOCK_T *interlock)
{
   return ((interlock->users & KHRN_INTERLOCK_USER_INVALID) != 0);
}
