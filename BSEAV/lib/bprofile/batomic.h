/***************************************************************************
 *     Copyright (c) 2006-2007, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Atomic operations
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef __BATOMIC_H__
#define __BATOMIC_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct batomic_t {
	volatile long atomic;
} batomic_t;

#define BATOMIC_INIT(v) {(v)}

BSTD_INLINE void __attribute__((no_instrument_function))
batomic_set(batomic_t *a, long val) {
	a->atomic = val;
}

BSTD_INLINE long __attribute__((no_instrument_function))
batomic_get(const batomic_t *a) {
	return a->atomic;
}

BSTD_INLINE long __attribute__((no_instrument_function))
batomic_add_return(batomic_t *a, long val)
{
	unsigned long result;
#if defined(__mips__)
	unsigned long temp;
	__asm__ __volatile__(
		"	.set	mips32					\n"
		"1:	ll	%1, %2		# b_atomic_add_return\n"
		"	addu	%0, %1, %3				\n"
		"	sc	%0, %2					\n"
		"	beqz	%0, 1b					\n"
		"	addu	%0, %1, %3				\n"
		"	.set	mips0					\n"
		: "=&r" (result), "=&r" (temp), "=m" (a->atomic)
		: "Ir" (val), "m" (a->atomic)
		: "memory");
#else
	a->atomic += val;
	result = a->atomic;
#endif
	return result;
}


#ifdef __cplusplus
}
#endif

#endif /* __BATOMIC_H__ */
