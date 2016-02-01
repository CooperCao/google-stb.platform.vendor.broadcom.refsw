/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Interlock system

FILE DESCRIPTION
Interlock system to ensure correct ordering of reads/writes.
=============================================================================*/

#ifndef KHRN_INTERLOCK_PRIV_4_H
#define KHRN_INTERLOCK_PRIV_4_H

#include "middleware/khronos/common/khrn_interlock.h"

typedef enum {
   KHRN_INTERLOCK_FIFO_NONE, /* just for khrn_interlock_get_write_fifo */

   /* todo: would KHRN_INTERLOCK_FIFO_HW_BIN/KHRN_INTERLOCK_FIFO_HW_BIN_RENDER be useful? */
   KHRN_INTERLOCK_FIFO_HW_RENDER,
   KHRN_INTERLOCK_FIFO_WORKER
} KHRN_INTERLOCK_FIFO_T;

extern void khrn_interlock_transfer(KHRN_INTERLOCK_T *interlock, KHRN_INTERLOCK_USER_T user, KHRN_INTERLOCK_FIFO_T fifo);

#endif
