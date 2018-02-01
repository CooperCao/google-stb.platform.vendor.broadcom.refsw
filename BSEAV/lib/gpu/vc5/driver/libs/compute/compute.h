/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include "libs/core/v3d/v3d_gen.h"
#if !V3D_USE_CSD
#include "libs/core/v3d/v3d_barrier.h"
#include "libs/platform/gmem.h"
#endif

EXTERN_C_BEGIN

typedef struct compute_params
{
   uint32_t num_qpus;
   uint32_t shared_mem_per_core;
   uint32_t wg_size;
   uint32_t shared_block_size;
   uint32_t num_threads;
   bool has_barrier;
} compute_params;

typedef struct compute_sg_config
{
   uint16_t max_wgs;
   uint8_t wgs_per_sg;
} compute_sg_config;

//! Chooses super-group configuration for the given compute params.
compute_sg_config compute_choose_sg_config(const compute_params* p);

//! Returns false if the compute shader cannot be run alongside other shaders.
bool compute_allow_concurrent_jobs(const compute_params* p, uint32_t wgs_per_sg);

#if !V3D_USE_CSD

// forward declare
typedef struct compute_job_mem compute_job_mem;

typedef struct compute_program
{
   v3d_addr_t code_addr;
   uint16_t wg_size[3];
   uint16_t items_per_wg;
   uint16_t max_wgs;
   uint8_t wgs_per_sg;
   uint8_t num_varys;
   uint8_t vary_map[3];
   bool has_barrier;
   bool has_shared;
   bool scb_wait_on_first_thrsw;
   v3d_threading_t threading;
} compute_program;

typedef struct compute_cl_mem_if
{
   //! Obtain a pointer to append size bytes to the control-list. Returns NULL for failure.
   uint8_t* (*write_cl)(void* ctx, unsigned size);

   //! Obtain a pointer to append size bytes to the control-list for the last time.
   //! Size will be no more than V3D_CL_BRANCH_SIZE and must not fail.
   uint8_t* (*write_cl_final)(void* ctx, unsigned size);

} compute_cl_mem_if;

//! Initialise the compute runtime.
void compute_init(void) noexcept;

//! Terminate the compute runtime.
void compute_term(void) noexcept;

//! Returns the GLSL backend flags to link the program.
uint32_t compute_backend_flags(unsigned items_per_wg) noexcept;

//! Returns flags for accesses made within compute runtime managed memory.
v3d_barrier_flags compute_mem_access_flags(void) noexcept;

//! Create a new compute job memory object for use with compute_dispatch. Returns NULL on failure.
compute_job_mem* compute_job_mem_new(void) noexcept;

//! Delete a compute job memory object, must be called after the V3D compute job has finished.
void compute_job_mem_delete(compute_job_mem* job) noexcept;

//! Flush compute job mem.
void compute_job_mem_flush(compute_job_mem* job) noexcept;

//! Enumerate V3D buffer accesses made by this compute job memory.
void compute_job_mem_enumerate_accesses(compute_job_mem const* job_mem, void (*callback)(void* ctx, gmem_handle_t, v3d_barrier_flags rw_flags), void* ctx);

//! Patch the GMP table to use this job memory.
void compute_job_mem_patch_gmp_table(compute_job_mem const* job_mem, void* gmp_table);

//! Build the secondary control list for this dispatch and patch the primary control list. Returns false on failure.
bool compute_build_dispatch(
   compute_job_mem* job_mem,
   uint8_t* dispatch_in_primary_cl,
   const compute_program* program,
   v3d_addr_t unifs_addr,
   const uint32_t num_work_groups[3]) noexcept;

//! Patch the primary control list with a nop.
void compute_clear_dispatch(uint8_t* dispatch_in_primary_cl) noexcept;

//! Begin writing a primary compute job control list to the memory interface. Returns false on failure.
bool compute_cl_begin(compute_cl_mem_if const* mem, void* mem_ctx) noexcept;

//! Add a compute dispatch to the primary compute job control list.
//! Returns the pointer to use with compute_build_dispatch or NULL on failure.
uint8_t* compute_cl_add_dispatch(compute_cl_mem_if const* mem, void* mem_ctx) noexcept;

//! Returns the number of bytes written by compute_cl_add_dispatch.
v3d_size_t compute_cl_dispatch_size(void) noexcept;

//! End writing a primary compute job control list to the memory interface.
void compute_cl_end(compute_cl_mem_if const* mem, void* mem_ctx) noexcept;

#endif

EXTERN_C_END
