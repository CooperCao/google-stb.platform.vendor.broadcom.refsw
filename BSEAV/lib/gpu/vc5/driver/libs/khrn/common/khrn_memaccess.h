/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/util/common.h"
#include "libs/core/v3d/v3d_addr.h"
#include "libs/core/v3d/v3d_barrier.h"
#include "gmem.h"

EXTERN_C_BEGIN

typedef struct khrn_render_state khrn_render_state;
typedef struct khrn_memaccess khrn_memaccess;
typedef struct khrn_resource khrn_resource;

khrn_memaccess* khrn_memaccess_create(khrn_render_state* rs);
void khrn_memaccess_destroy(khrn_memaccess* ma);

//! Record usage of a buffer.
void khrn_memaccess_add_buffer(khrn_memaccess* ma, gmem_handle_t handle, v3d_barrier_flags bin_flags, v3d_barrier_flags rdr_flags);

//! Register a resource so that buffer accesses can be synchronised.
void khrn_memaccess_register_resource(khrn_memaccess* ma, khrn_resource* res);

//! This must be called before using khrn_memaccess_read().
void khrn_memaccess_pre_read(khrn_memaccess* ma);

//! Get readable pointer from v3d address. Must call khrn_memaccess_pre_read() before use.
const void *khrn_memaccess_read_ptr(khrn_memaccess* ma, v3d_addr_t read_addr, v3d_size_t read_size);

//! Implements simcom_memaccess.read.
void khrn_memaccess_read_fn(void *dst, v3d_addr_t src_addr, size_t size,
   const char *file, uint32_t line, const char *func, void *p);

//! Build GMP tables for bin/render jobs.
void khrn_memaccess_build_gmp_tables(khrn_memaccess* ma, uint8_t* gmp_tables);

EXTERN_C_END
