/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include "libs/util/assert_helpers.h"

#define V3D_MAX_CORES 16

#define V3D_MAX_QPUS_PER_CORE 16
#define V3D_MAX_QPUS_PER_SLICE 4
#define V3D_NUM_THREADS_PER_QPU 4

#define V3D_VPAR 16        //!< Virtual paralleism: 16 way (number of lanes)

#if V3D_HAS_LARGE_REGFILE
#define V3D_QPU_RF_SIZE 128    // Number of register file rows per QPU
#else
#define V3D_QPU_RF_SIZE 64     // Number of register file rows per QPU
#endif

#define V3D_MAX_SUPERTILES 256

#define V3D_MAX_MIP_COUNT       15

#define V3D_MAX_CLIP_WIDTH    4096
#define V3D_MAX_CLIP_HEIGHT   4096

#define V3D_MAX_RENDER_TARGETS   4

#define V3D_MAX_TLB_WIDTH_PX    64 /* In pixels */
#define V3D_MAX_TLB_HEIGHT_PX   64

/* Maximum words per sample in TLB itself */
#define V3D_MAX_RT_WORDS_PER_SAMPLE 4
/* V3D_MAX_RT_BPP is really max bits per sample, not per pixel */
#define V3D_MAX_RT_BPP              (V3D_MAX_RT_WORDS_PER_SAMPLE * 32)

/* Maximum words per sample on QPU read/write interface */
#define V3D_MAX_READ_WRITE_WORDS_PER_SAMPLE 4

#define V3D_MAX_SAMPLES          4

#define V3D_MAX_POINT_SIZE 512.0f /* TODO is this right? */

#define V3D_MAX_VERTS_PER_PRIM 32

/* TODO get these values right */
#define V3D_MAX_PRIM_COUNT         (1u << 24)
#define V3D_MAX_INDEX              ((1u << 24) - 1)
#define V3D_MAX_ATTR_ARRAYS        16
#define V3D_MAX_ATTR_ARRAY_STRIDE  (1u << 16)
#define V3D_MAX_VARYING_COMPONENTS 64
#define V3D_MAX_EXTRA_VARYINGS     3 /* Extra varyings for lines/points */

#define V3D_MAX_TF_BUFFERS 4
#define V3D_MAX_TF_SPECS 16

#define V3D_MAX_TILE_LIST_SETS 8

/* These are in bytes */
#define V3D_MAX_CACHE_LINE_SIZE 256u
#define V3D_MAX_CLE_READAHEAD 256u // 128 on 3.2
#define V3D_CLE_MIN_DIST_FROM_END_TO_AVOID_FALSE_HITS 31 /* To workaround GFXH-1285 */
#define V3D_MAX_QPU_UNIFS_READAHEAD (8/*3.2 only*/ + 4) /* Unifs FIFO in QPU (3.2 only) plus L1 prefetch (only 1 unif) */
#define V3D_MAX_QPU_INSTRS_READAHEAD 8 /* L1 prefetch (only 1 instr) */

/* Size of FIFOs per QPU. Need to divide by threadedness for usable space by a
 * single thread */
#define V3D_TMU_INPUT_FIFO_SIZE  16 /* One entry used per TMU magic register write */
#define V3D_TMU_CONFIG_FIFO_SIZE  8 /* Input FIFO for uniform/config data. One entry used per access. */
#define V3D_TMU_OUTPUT_FIFO_SIZE 16 /* One entry popped by each ldtmu */

#define V3D_GMP_PAGE_SIZE (128 * 1024)
/* 32-bit address space / V3D_GMP_PAGE_SIZE = 32k pages
 * 32k pages * 2 bits per page = 8k bytes */
#define V3D_GMP_TABLE_SIZE 8192
/* 32k pages / 32 lines in table = 1k pages per line */
#define V3D_GMP_PAGES_PER_LINE 1024

#define V3D_TMU_CONFIG_CACHE_LINE_SIZE 64

#define V3D_CLE_MAX_NESTING_DEPTH 2

#define V3D_TFU_MAX_WIDTH 16384
#define V3D_TFU_MAX_HEIGHT 16384
#define V3D_TFU_MAX_JOBS 32

#define V3D_MMU_PAGE_SIZE (4096)
#define V3D_MMU_MEGAPAGE_SIZE (1u << 28u)

#define V3D_MAX_PATCH_VERTICES 32u
#define V3D_MAX_TESS_GEN_LEVEL 64u

#define V3D_MAX_GEOMETRY_INVOCATIONS 32u

/* Number of bits of sub-pixel precision in the rasterizer */
#if V3D_HAS_RAST_PREC
#define V3D_COORD_SHIFT 6
#else
#define V3D_COORD_SHIFT 4
#endif
static_assrt(V3D_COORD_SHIFT >= 4);
static_assrt(V3D_COORD_SHIFT <= 8);

#define V3D_COORD_SCALE (1 << V3D_COORD_SHIFT)
