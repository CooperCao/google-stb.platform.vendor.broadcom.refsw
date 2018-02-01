/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CONTEXT_HELPERS_H__
#define __CONTEXT_HELPERS_H__

#ifndef __ASSEMBLY__

#include <context.h>

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
void el1_sysregs_context_save(el1_sys_regs_t *regs);
void el1_sysregs_context_restore(el1_sys_regs_t *regs);
#if CTX_INCLUDE_FPREGS
void fpregs_context_save(fp_regs_t *regs);
void fpregs_context_restore(fp_regs_t *regs);
#endif
void gpregs_context_save(void);
void gpregs_context_restore_eret(void);
void gpregs_callee_restore_eret(void);

#endif /* __ASSEMBLY__ */

#endif /* __CONTEXT_HELPERS_H__ */
