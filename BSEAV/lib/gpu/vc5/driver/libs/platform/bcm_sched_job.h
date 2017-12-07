/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BCM_SCHED_JOB_H
#define BCM_SCHED_JOB_H
#include "gmem.h"
#include "libs/core/v3d/v3d_barrier.h"
#include "libs/core/v3d/v3d_csd.h"
#include "libs/util/demand.h"

#define BCM_SCHED_MAX_DEPENDENCIES 8
#define BCM_SCHED_MAX_COMPLETIONS 8
#define BCM_SCHED_MAX_JOBS 8

/* These are defined by the platform */
struct bcm_gmem_synclist;

#define V3D_MAX_BIN_SUBJOBS 8
#define V3D_MAX_RENDER_SUBJOBS 16

#define V3D_MAX_QPU_SUBJOBS 12
#define V3D_IDENT_REGISTERS 4

typedef enum v3d_empty_tile_mode
{
   V3D_EMPTY_TILE_MODE_NONE,
   V3D_EMPTY_TILE_MODE_SKIP,
   V3D_EMPTY_TILE_MODE_FILL
} v3d_empty_tile_mode;

typedef struct v3d_subjob
{
   uint32_t start;
   uint32_t end;
#ifdef __cplusplus
   v3d_subjob() : start(0), end(0) {}
   v3d_subjob(uint32_t start, uint32_t end) : start(start), end(end) {}
#endif
} v3d_subjob;

typedef struct
{
   unsigned num_subjobs;
   v3d_subjob *subjobs;
}v3d_subjobs_list;

typedef struct v3d_core_job
{
   const void* gmp_table;
   bool no_overlap;              // If true, do not run this sub concurrently on the same core with other job types.
#if !V3D_VER_AT_LEAST(3,3,0,0)
   bool workaround_gfxh_1181;   /* GFXH-1181 */
#endif
} v3d_core_job;

typedef struct v3d_bin_job
{
   v3d_core_job base;
   uint32_t minInitialBinBlockSize;
   uint32_t tile_state_size;
   v3d_subjobs_list subjobs_list;
} v3d_bin_job;

typedef struct v3d_render_job
{
   v3d_core_job base;
   v3d_empty_tile_mode empty_tile_mode;
   uint32_t tile_alloc_layer_stride;
   uint32_t num_layers;
   v3d_subjobs_list subjobs_list;  /* subjobs_list->subjobs[layer * num_subjobs_per_layer + subjob_in_layer]
                                    *, where num_subjobs_per_layer = subjobs_list->num_subjobs/num_layers */
} v3d_render_job;

#if V3D_USE_CSD

typedef struct v3d_compute_subjobs* v3d_compute_subjobs_id; // 0 is not a valid ID.

typedef struct v3d_compute_subjob
{
   union
   {
      struct
      {
         uint32_t num_wgs_x;
         uint32_t num_wgs_y;
         uint32_t num_wgs_z;
      };
      uint32_t num_wgs[3];
   };
   uint16_t wg_size;
   uint8_t wgs_per_sg;
   uint8_t max_sg_id;
   v3d_addr_t shader_addr;
   v3d_addr_t unifs_addr;
   uint16_t shared_block_size;
   v3d_threading_t threading;
   bool single_seg;
   bool propagate_nans;
   bool no_overlap;                 // If true, do not run this csd subjob concurrently on the same core with other job types.
} v3d_compute_subjob;

// Note this function may assert if subjob is very large. For that reason, it
// should not be used in production code.
static inline V3D_CSD_CFG_T v3d_compute_subjob_to_single_csd_cfg(const v3d_compute_subjob *subjob)
{
   V3D_CSD_CFG_T cfg = {0};
   cfg.num_wgs_x = subjob->num_wgs_x;
   cfg.num_wgs_y = subjob->num_wgs_y;
   cfg.num_wgs_z = subjob->num_wgs_z;
   cfg.wg_size = subjob->wg_size;
   cfg.wgs_per_sg = subjob->wgs_per_sg;
   cfg.max_sg_id = subjob->max_sg_id;
   cfg.threading = subjob->threading;
   cfg.single_seg = subjob->single_seg;
   cfg.propagate_nans = subjob->propagate_nans;
   cfg.shader_addr = subjob->shader_addr;
   cfg.unifs_addr = subjob->unifs_addr;
   v3d_csd_cfg_compute_num_batches(&cfg);
   demand(cfg.num_batches <= (1ull << 32));
   return cfg;
}

typedef struct v3d_compute_job
{
   v3d_core_job base;
   v3d_compute_subjobs_id subjobs_id;
   const v3d_compute_subjob* subjobs;
   unsigned num_subjobs;
} v3d_compute_job;

#endif

struct v3d_user_job
{
   unsigned int n;
   uint32_t pc[V3D_MAX_QPU_SUBJOBS];
   uint32_t unif[V3D_MAX_QPU_SUBJOBS];
};

#if !V3D_PLATFORM_SIM
struct bcm_fence_wait_job
{
   int fence;
};
#endif

typedef uint64_t bcm_sched_event_id;

struct bcm_event_job
{
   bcm_sched_event_id id;
};

enum tfu_axithrottle {
   TFU_AXITHROTTLE_NONE = 0,
   TFU_AXITHROTTLE_1 = 1,
   TFU_AXITHROTTLE_2 = 2,
   TFU_AXITHROTTLE_3 = 3,
};

enum tfu_rgbord {
   TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV = 0,
   TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU = 1,
   TFU_RGBORD_ARGB_OR_YYUV          = 2,
   TFU_RGBORD_BGRA_OR_VUYY          = 3,
   TFU_RGBORD_INVALID               = 4
};

enum tfu_input_byte_format {
   TFU_IFORMAT_RASTER      = 0,
   TFU_IFORMAT_SAND128     = 1,
   TFU_IFORMAT_SAND256     = 2,
   TFU_IFORMAT_LINEARTILE  = 11,
   TFU_IFORMAT_UBLINEAR_1  = 12,
   TFU_IFORMAT_UBLINEAR_2  = 13,
   TFU_IFORMAT_UIF         = 14,
   TFU_IFORMAT_UIF_XOR     = 15,
   TFU_IFORMAT_INVALID     = 16
};

enum tfu_input_type {
   TFU_ITYPE_YUV_420_2PLANE_REC709           = 39,
   TFU_ITYPE_YUV_420_2PLANE_REC601           = 40,
   TFU_ITYPE_YUV_420_2PLANE_JPEG             = 41,
   TFU_ITYPE_YUV_422_2PLANE_REC709           = 42,
   TFU_ITYPE_YUV_422_2PLANE_REC601           = 43,
   TFU_ITYPE_YUV_422_2PLANE_JPEG             = 44,
   TFU_ITYPE_YUYV_422_1PLANE_REC709          = 45,
   TFU_ITYPE_YUYV_422_1PLANE_REC601          = 46,
   TFU_ITYPE_YUYV_422_1PLANE_JPEG            = 47,
   TFU_ITYPE_R8                              = 0,
   TFU_ITYPE_R8_SNORM                        = 1,
   TFU_ITYPE_RG8                             = 2,
   TFU_ITYPE_RG8_SNORM                       = 3,
   TFU_ITYPE_RGBA8                           = 4,
   TFU_ITYPE_RGBA8_SNORM                     = 5,
   TFU_ITYPE_RGB565                          = 6,
   TFU_ITYPE_RGBA4                           = 7,
   TFU_ITYPE_RGB5_A1                         = 8,
   TFU_ITYPE_RGB10_A2                        = 9,
   TFU_ITYPE_R16                             = 10,
   TFU_ITYPE_R16_SNORM                       = 11,
   TFU_ITYPE_RG16                            = 12,
   TFU_ITYPE_RG16_SNORM                      = 13,
   TFU_ITYPE_RGBA16                          = 14,
   TFU_ITYPE_RGBA16_SNORM                    = 15,
   TFU_ITYPE_R16F                            = 16,
   TFU_ITYPE_RG16F                           = 17,
   TFU_ITYPE_RGBA16F                         = 18,
   TFU_ITYPE_R11F_G11F_B10F                  = 19,
   TFU_ITYPE_RGB9_E5                         = 20,
   TFU_ITYPE_DEPTH_COMP16                    = 21,
   TFU_ITYPE_DEPTH_COMP24                    = 22,
   TFU_ITYPE_DEPTH_COMP32F                   = 23,
   TFU_ITYPE_DEPTH24_X8                      = 24,
   TFU_ITYPE_R4                              = 25,
   TFU_ITYPE_R1                              = 26,
   TFU_ITYPE_S8                              = 27,
   TFU_ITYPE_S16                             = 28,
   TFU_ITYPE_S32                             = 29,
   TFU_ITYPE_S64                             = 30,
   TFU_ITYPE_S128                            = 31,
   TFU_ITYPE_C_RGB8_ETC2                     = 32,
   TFU_ITYPE_C_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 33,
   TFU_ITYPE_C_R11_EAC                       = 34,
   TFU_ITYPE_C_SIGNED_R11_EAC                = 35,
   TFU_ITYPE_C_RG11_EAC                      = 36,
   TFU_ITYPE_C_SIGNED_RG11_EAC               = 37,
   TFU_ITYPE_C_RGBA8_ETC2_EAC                = 38,
   TFU_ITYPE_C_BC1                           = 48,
   TFU_ITYPE_C_BC2                           = 49,
   TFU_ITYPE_C_BC3                           = 50,
   TFU_ITYPE_C_BC4                           = 51,
   TFU_ITYPE_C_BC4_SNORM                     = 52,
   TFU_ITYPE_C_BC5                           = 53,
   TFU_ITYPE_C_BC5_SNORM                     = 54,
   TFU_ITYPE_C_BC6                           = 55,
   TFU_ITYPE_C_BC6_SNORM                     = 56,
   TFU_ITYPE_C_BC7                           = 57,
   TFU_ITYPE_C_ASTC_4X4                      = 64,
   TFU_ITYPE_C_ASTC_5X4                      = 65,
   TFU_ITYPE_C_ASTC_5X5                      = 66,
   TFU_ITYPE_C_ASTC_6X5                      = 67,
   TFU_ITYPE_C_ASTC_6X6                      = 68,
   TFU_ITYPE_C_ASTC_8X5                      = 69,
   TFU_ITYPE_C_ASTC_8X6                      = 70,
   TFU_ITYPE_C_ASTC_8X8                      = 71,
   TFU_ITYPE_C_ASTC_10X5                     = 72,
   TFU_ITYPE_C_ASTC_10X6                     = 73,
   TFU_ITYPE_C_ASTC_10X8                     = 74,
   TFU_ITYPE_C_ASTC_10X10                    = 75,
   TFU_ITYPE_C_ASTC_12X10                    = 76,
   TFU_ITYPE_C_ASTC_12X12                    = 77,
   TFU_ITYPE_C_ASTC_3X3X3                    = 80,
   TFU_ITYPE_C_ASTC_4X3X3                    = 81,
   TFU_ITYPE_C_ASTC_4X4X3                    = 82,
   TFU_ITYPE_C_ASTC_4X4X4                    = 83,
   TFU_ITYPE_C_ASTC_5X4X4                    = 84,
   TFU_ITYPE_C_ASTC_5X5X4                    = 85,
   TFU_ITYPE_C_ASTC_5X5X5                    = 86,
   TFU_ITYPE_C_ASTC_6X5X5                    = 87,
   TFU_ITYPE_C_ASTC_6X6X5                    = 88,
   TFU_ITYPE_C_ASTC_6X6X6                    = 89,
   TFU_ITYPE_INVALID                         = 128
};

enum tfu_endianness {
   TFU_BIGEND_NOREORDER = 0,
   TFU_BIGEND_WORD_32 = 1,
   TFU_BIGEND_WORD_16 = 2,
};

enum tfu_output_byte_format {
   TFU_OFORMAT_LINEARTILE  = 3,
   TFU_OFORMAT_UBLINEAR_1  = 4,
   TFU_OFORMAT_UBLINEAR_2  = 5,
   TFU_OFORMAT_UIF         = 6,
   TFU_OFORMAT_UIF_XOR     = 7,
   TFU_OFORMAT_INVALID     = 8,
};

/*
 * TFU Job Flags
 *
 * OUTPUT_DISABLE_MAIN_TEXTURE - Will cause only mipmaps
 *      to be generated.
 *
 * OUTPUT_USECOEF - Will cause a YUV -> RGB conversion
 *      to use the custom coefficients supplied with the job.
 *
 * INPUT_FLIPY - Will treat the input as a Y-Flipped image,
 *      and read backwards.
 *
 * INPUT_SRGB - Treat input RGB[A] as being in the sRGB
 *      colourspace
 */

#define TFU_OUTPUT_DISABLE_MAIN_TEXTURE (1<<0)
#define TFU_OUTPUT_USECOEF (1<<1)

#define TFU_INPUT_FLIPY (1<<0)
#define TFU_INPUT_SRGB (1<<1)

/*
 * TFU Job Structure
 *
 * Upon job submission, the contents of this structure are
 * submitted to the TFU's APB registers, where they will
 * enter a 32-deep command fifo.
 *
 * TFUDRV_FIXME: Replace bus addresses with bcm_mem
 * - address fields are bus addresses for the TFU peripheral
 * - Width and Height must be < 16384
 * - Output must be 128-byte aligned
 * - Input has no alignment constraints
 * - Custom coefficients are in 6.10 fixed-point format
 */
struct bcm_tfu_job {
   struct {
      enum tfu_input_type texture_type;
      enum tfu_input_byte_format byte_format;
      enum tfu_endianness endianness;
      enum tfu_rgbord component_order;
      unsigned int raster_stride;
      unsigned int chroma_stride;
      unsigned int address;
      unsigned int chroma_address;
      unsigned int uplane_address;
      uint64_t flags;
   } input;

   struct {
      unsigned int mipmap_count;
      unsigned int vertical_padding;
      unsigned int width;
      unsigned int height;
      enum tfu_endianness endianness;
      enum tfu_output_byte_format byte_format;
      unsigned int address;
      uint32_t flags;
   } output;

   struct {
      uint16_t a_y;
      uint16_t a_rc;
      uint16_t a_bc;
      uint16_t a_gc;
      uint16_t a_rr;
      uint16_t a_gr;
      uint16_t a_gb;
      uint16_t a_bb;
   } custom_coefs;
};

enum bcm_sched_job_type
{
   BCM_SCHED_JOB_TYPE_NULL,
   BCM_SCHED_JOB_TYPE_V3D_BIN,
   BCM_SCHED_JOB_TYPE_V3D_RENDER,
   BCM_SCHED_JOB_TYPE_V3D_USER,
   BCM_SCHED_JOB_TYPE_V3D_TFU,
#if !V3D_PLATFORM_SIM
   BCM_SCHED_JOB_TYPE_FENCE_WAIT,
#endif
   BCM_SCHED_JOB_TYPE_TEST,
   BCM_SCHED_JOB_TYPE_USERMODE,
   BCM_SCHED_JOB_TYPE_V3D_BARRIER,
   BCM_SCHED_JOB_TYPE_WAIT_ON_EVENT,
   BCM_SCHED_JOB_TYPE_SET_EVENT,
   BCM_SCHED_JOB_TYPE_RESET_EVENT,
#if V3D_USE_CSD
   BCM_SCHED_JOB_TYPE_V3D_COMPUTE,
#endif
   BCM_SCHED_JOB_TYPE_NUM_JOB_TYPES
};

enum bcm_sched_job_error
{
   BCM_SCHED_JOB_SUCCESS,        /* Job completed without error */
   BCM_SCHED_JOB_OUT_OF_MEMORY,  /* Job could not complete due to lack of memory */
   BCM_SCHED_JOB_ERROR           /* Job did not complete due to unspecified error */
};

struct bcm_sched_dependencies
{
   uint64_t dependency[BCM_SCHED_MAX_DEPENDENCIES];
   unsigned int n;
};

struct bcm_sched_query_response
{
   int state_achieved;  /* Non-zero indicates that the queried dependencies
                           are finalised/completed */
};

struct bcm_sched_test
{
   unsigned int delay;
};

typedef void (*bcm_user_fn)(void *data);

struct bcm_usermode_job
{
   bcm_user_fn    user_fn;
   void          *data;
};

typedef void (*bcm_completion_fn)(void *data, uint64_t jobId, enum bcm_sched_job_error error);

struct bcm_sched_job
{
   uint64_t                      job_id;
   enum bcm_sched_job_type       job_type;

   struct bcm_sched_dependencies completed_dependencies;
   struct bcm_sched_dependencies finalised_dependencies;

   bcm_completion_fn             completion_fn;
   void                         *completion_data;

   // These apply to any V3D job, or a barrier job.
   // For a V3D job, the driver performs the flushes/clears before the job and the cleans after.
   // For a barrier, the driver performs the cleans then the flushes/clears.
   // For a TFU job, it is not valid to specify per-core caches.
   // Not valid for any other job type.
   v3d_cache_ops cache_ops;

   bool secure;

   union
   {
      struct v3d_core_job core;        // base class for bin, render, compute jobs.
      struct v3d_bin_job bin;
      struct v3d_render_job render;
#if V3D_USE_CSD
      struct v3d_compute_job compute;
#endif
      struct v3d_user_job user;
#if !V3D_PLATFORM_SIM
      struct bcm_fence_wait_job fence_wait;
#endif
      struct bcm_sched_test test;
      struct bcm_tfu_job tfu;
      struct bcm_usermode_job usermode;
      struct bcm_event_job event;
   }
   driver;
};

#endif /* BCM_SCHED_JOB_H */
