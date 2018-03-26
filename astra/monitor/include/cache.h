/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CACHE_H__
#define __CACHE_H__

void flush_dcache_range(uintptr_t addr, size_t size);
void clean_dcache_range(uintptr_t addr, size_t size);
void inv_dcache_range(uintptr_t addr, size_t size);

void dcsw_op_louis(uint64_t op);
void dcsw_op_all(uint64_t op);
void dcsw_op_level1(uint64_t op);
void dcsw_op_level2(uint64_t op);
void dcsw_op_level3(uint64_t op);

#endif /* __CACHE_H__ */
