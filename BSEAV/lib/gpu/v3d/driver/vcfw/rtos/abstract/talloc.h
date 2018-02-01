/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <inttypes.h>

extern void *talloc_initialize(void);
extern void talloc_term(void *h);
extern bool talloc_alloc(void *h, size_t size, size_t align, void **cur_map_ptr, uintptr_t *cur_lock_phys);
extern void talloc_free(void *h, void *p);
