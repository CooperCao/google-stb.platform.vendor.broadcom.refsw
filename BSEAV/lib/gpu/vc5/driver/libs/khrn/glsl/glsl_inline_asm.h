/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "libs/util/common.h"
#include "glsl_backend_uniforms.h"

typedef struct inline_qasm {
   const uint64_t *code;
   size_t          size;
} inline_qasm;

typedef struct inline_umap {
   const umap_entry *unifs;
   size_t            size;
} inline_umap;

static inline size_t copy_inline_qasm_if(uint64_t *dst, size_t dst_offset, const struct inline_qasm *src) {
   if (src) {
      if (dst) memcpy(dst + dst_offset, src->code, src->size * sizeof(uint64_t));
      return dst_offset + src->size;
   }
   return 0;
}

static inline size_t copy_inline_umap_if(umap_entry *dst, size_t dst_offset, const struct inline_umap *src) {
   if (src) {
      if (dst) memcpy(dst + dst_offset, src->unifs, src->size * sizeof(umap_entry));
      return dst_offset + src->size;
   }
   return 0;
}

#if !V3D_USE_CSD
// If using cs_pad_setmsf_with_barriers  truncate the last instruction and unif of the barrier code.
static const size_t cs_pad_setmsf_with_barriers_code_truncate = 1;
static const size_t cs_pad_setmsf_with_barriers_unif_truncate = 1;
extern const inline_qasm cs_pad_setmsf_after_barrier_preamble;
extern const inline_qasm cs_pad_setmsf_threaded;
extern const inline_qasm cs_pad_setmsf_unthreaded;
#endif
extern const inline_qasm cs_barrier_preamble;
extern const inline_qasm cs_barrier;
extern const inline_qasm cs_barrier_lthrsw;

extern const inline_qasm tcs_barrier_preamble_bin;
extern const inline_qasm tcs_barrier_preamble_rdr;
extern const inline_qasm tcs_barrier_bin;
extern const inline_qasm tcs_barrier_rdr;
extern const inline_qasm tcs_barrier_lthrsw_bin;
extern const inline_qasm tcs_barrier_lthrsw_rdr;
extern const inline_qasm tcs_barrier_mem_only;

#if V3D_VER_AT_LEAST(4,2,14,0)
extern const inline_qasm fs_null;
#endif

#if !V3D_USE_CSD
extern const inline_umap cs_pad_setmsf_after_barrier_preamble_unif;
extern const inline_umap cs_pad_setmsf_unif;
#endif
extern const inline_umap cs_barrier_preamble_unif;
extern const inline_umap cs_barrier_unif;
extern const inline_umap tcs_barrier_preamble_bin_unif;
extern const inline_umap tcs_barrier_preamble_rdr_unif;
extern const inline_umap tcs_barrier_bin_unif;
extern const inline_umap tcs_barrier_rdr_unif;
extern const inline_umap tcs_barrier_lthrsw_bin_unif;
extern const inline_umap tcs_barrier_lthrsw_rdr_unif;
