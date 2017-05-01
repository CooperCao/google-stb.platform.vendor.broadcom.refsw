/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef V3D_ALIGN_H
#define V3D_ALIGN_H

#include "v3d_limits.h"
#include "v3d_ver.h"
#include "libs/util/assert_helpers.h"

VCOS_EXTERN_C_BEGIN

/* Maximum alignment relevant to V3D. Strictly speaking the GMP page size is
 * larger than this and alignment relative to GMP pages is relevant, but that
 * is ignored here. */
#define V3D_MAX_ALIGN 4096

/* The _REC_ALIGN values are recommended alignments to make best use of
 * caches/buses/etc. They are larger than strictly necessary. */

/* The _CONSERVATIVE_ALIGN values are conservative in the sense that if you use
 * that alignment when allocating, you should be safe. The actual alignment
 * requirements may not be that strict. */

#define V3D_CL_ALIGN             4 /* Because in big endian mode it's a bit weird */
#define V3D_CL_REC_ALIGN        16 /* Bus alignment */
#define V3D_CL_COMPR_ALIGN      32 /* Because of the weird branches */

#define V3D_CLIP_RECORD_ALIGN 32

/* 8/16/32-bit indices should be 1/2/4-byte aligned */
#define V3D_INDICES_CONSERVATIVE_ALIGN  4
#define V3D_INDICES_REC_ALIGN          16

#define V3D_SHADREC_ALIGN       32
/* Default attribute values, as pointed to by shader record */
#define V3D_ATTR_DEFAULTS_ALIGN 16

/* "Transform Feedback Enable" instruction: buffer addresses must be 32-bit aligned */
#define V3D_TF_BUFFER_ALIGN      4

#define V3D_DRAW_INDIRECT_ALIGN  4

#define V3D_ATTR_ALIGN           1
#define V3D_ATTR_REC_ALIGN      16

#define V3D_TILE_ALLOC_ALIGN         4096 /* See GFXH-1179 */
#define V3D_TILE_ALLOC_GRANULARITY   4096 /* PTB consumes memory in chunks of this size */
#define V3D_TILE_ALLOC_BLOCK_SIZE_MIN  64
#define V3D_TILE_ALLOC_BLOCK_SIZE_MAX 256

#if V3D_VER_AT_LEAST(4,0,2,0)
#define V3D_TILE_STATE_ALIGN   256
#define V3D_TILE_STATE_SIZE    256 /* Per tile */
#else
#define V3D_TILE_STATE_ALIGN   128
#define V3D_TILE_STATE_SIZE     64 /* Per tile */
#endif

#define V3D_QPU_INSTR_ALIGN   8
#define V3D_QPU_UNIFS_ALIGN   16 /* To workaround GFXH-1181 */

#if V3D_VER_AT_LEAST(4,0,2,0)
#define V3D_TMU_TEX_STATE_ALIGN           16
#define V3D_TMU_EXTENDED_TEX_STATE_ALIGN  32
#define V3D_TMU_SAMPLER_ALIGN              8
#define V3D_TMU_EXTENDED_SAMPLER_ALIGN    32
#else
#define V3D_TMU_INDIRECT_ALIGN         32
#endif
#define V3D_TMU_GENERAL_CONSERVATIVE_ALIGN 16
#define V3D_TMU_GENERAL_REC_ALIGN          64

#define V3D_GMP_TABLE_ALIGN 256 /* Table must be 256 byte aligned */

#define V3D_MMU_TABLE_ALIGN 4096

/** Images (textures/framebuffers) */

/* This alignment should be fine for all image data */
#define V3D_CONSERVATIVE_IMAGE_ALIGN 4096

#if !V3D_VER_AT_LEAST(4,0,2,0)
#define V3D_TMU_CFG0_BASE_PTR_ALIGN 512
#endif

/* Each mip level should be at least this aligned. Provided level 0 is at least
 * this aligned, the texture layout rules should guarantee that the other
 * levels are at least this aligned. */
#define V3D_TMU_ML_ALIGN            64

/** Query counters */

/* Query counters are a bit weird...
 *
 * The address in the OCCLUSION_QUERY_COUNTER_ENABLE instruction must satisfy:
 * - addr & 0x3 == 0 (ie be 4-byte aligned)
 * - addr & V3D_OCCLUSION_QUERY_COUNTER_CORE_MASK == 0
 *
 * The actual address used by the hardware is:
 * addr + core index * V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE
 *
 * Each core loads query counters into its cache in blocks of size
 * V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE. The occlusion query cache
 * does *not* have per-word dirty bits,
 *
 * To make it all work, query counters should be allocated in blocks of size
 * (num cores * V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE), with alignment
 * V3D_OCCLUSION_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN. You get
 * V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_COUNTERS separate counters out of
 * each block. */

#define V3D_OCCLUSION_QUERY_COUNTER_LOG2_SINGLE_CORE_CACHE_LINE_SIZE 6
#define V3D_OCCLUSION_QUERY_COUNTER_CORE_BITS 4
static_assrt(V3D_MAX_CORES <= (1 << V3D_OCCLUSION_QUERY_COUNTER_CORE_BITS));

#define V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE ( \
   1 << V3D_OCCLUSION_QUERY_COUNTER_LOG2_SINGLE_CORE_CACHE_LINE_SIZE)
#define V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_COUNTERS ( \
   V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE / 4)

#define V3D_OCCLUSION_QUERY_COUNTER_CORE_MASK ( \
   ((1 << V3D_OCCLUSION_QUERY_COUNTER_CORE_BITS) - 1) << \
   V3D_OCCLUSION_QUERY_COUNTER_LOG2_SINGLE_CORE_CACHE_LINE_SIZE)

#define V3D_OCCLUSION_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN ( \
   (1 << V3D_OCCLUSION_QUERY_COUNTER_CORE_BITS) * V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE)

#define V3D_PRIM_COUNTS_ALIGN 32

VCOS_EXTERN_C_END

#endif
