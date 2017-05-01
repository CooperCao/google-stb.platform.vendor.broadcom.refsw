/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "libs/util/common.h"
#include "glsl_backend_uniforms.h"

struct inline_qasm {
   const uint64_t *code;
   size_t          size;
};

extern const struct inline_qasm cs_barrier_preamble;
extern const struct inline_qasm tcs_barrier_preamble;
extern const struct inline_qasm cs_pad_setmsf_with_barriers;
extern const struct inline_qasm cs_scoreboard_wait;
extern const struct inline_qasm cs_one_thread_wait;
extern const struct inline_qasm cs_pad_setmsf;
extern const struct inline_qasm cs_barrier;
extern const struct inline_qasm tcs_barrier;
extern const struct inline_qasm cs_barrier_lthrsw;
extern const struct inline_qasm tcs_barrier_lthrsw;

static inline size_t copy_inline_qasm_if(uint64_t *dst, size_t dst_offset, const struct inline_qasm *src) {
   if (dst) memcpy(dst + dst_offset, src->code, src->size * sizeof(uint64_t));
   return dst_offset + src->size;
}

struct inline_umap {
   const umap_entry *unifs;
   size_t            size;
};

// Unifs for both tcs and cs barrier preambles.
extern const struct inline_umap cs_barrier_preamble_unif;
extern const struct inline_umap tcs_barrier_preamble_unif;
extern const struct inline_umap cs_pad_setmsf_with_barriers_unif;
extern const struct inline_umap cs_pad_setmsf_unif;
extern const struct inline_umap barrier_unif;

static inline size_t copy_inline_umap_if(umap_entry *dst, size_t dst_offset, const struct inline_umap *src) {
   if (dst) memcpy(dst + dst_offset, src->unifs, src->size * sizeof(umap_entry));
   return dst_offset + src->size;
}

// If using cs_pad_setmsf_with_barriers  truncate the last instruction and unif of the barrier code.
static const size_t cs_pad_setmsf_with_barriers_code_truncate = 1;
static const size_t cs_pad_setmsf_with_barriers_unif_truncate = 1;
