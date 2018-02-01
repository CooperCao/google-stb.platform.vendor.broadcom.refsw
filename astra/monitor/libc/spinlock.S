/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_helpers.h>

	.globl	spin_lock
	.globl	spin_unlock

/*
 * Acquire lock using load-/store-exclusive instruction pair.
 *
 * void spin_lock(spinlock_t *lock);
 */
func spin_lock
	mov	w2, #1
	sevl
l1:	wfe
l2:	ldaxr	w1, [x0]
	cbnz	w1, l1
	stxr	w1, w2, [x0]
	cbnz	w1, l2
	ret
endfunc spin_lock

/*
 * Release lock previously acquired by spin_lock.
 *
 * Unconditionally write 0, and conditionally generate an event.
 *
 * void spin_unlock(spinlock_t *lock);
 */
func spin_unlock
	stlr	wzr, [x0]
	ret
endfunc spin_unlock
