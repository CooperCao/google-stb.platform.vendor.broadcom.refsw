/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#ifndef KHRN_PROCESS_H
#define KHRN_PROCESS_H
#include <stdbool.h>
#include "libs/core/v3d/v3d_gen.h"
#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "khrn_int_common.h"
#include "libs/platform/gmem.h"

VCOS_EXTERN_C_BEGIN

extern bool khrn_process_init(void);
extern void khrn_process_shutdown(void);
extern gmem_handle_t get_dummy_texture(void);

struct glxx_texture_sampler_state;
extern const struct glxx_tex_sampler_state *khrn_get_image_unit_default_sampler(void);

extern gmem_handle_t khrn_get_gfxh_1320_buffer(void);
extern char *khrn_get_extensions(void);
extern gfx_lfmt_translate_gl_ext_t khrn_get_lfmt_translate_exts(void);
#if !V3D_HAS_NEW_TMU_CFG
extern const V3D_MISCCFG_T *khrn_get_hw_misccfg(void);
#endif

static inline int khrn_get_v3d_version(void);
static inline uint32_t khrn_get_num_qpus_per_core(void);
static inline uint32_t khrn_get_num_render_subjobs(void);
static inline uint32_t khrn_get_num_bin_subjobs(void);
static inline uint32_t khrn_get_num_cores(void);
static inline bool khrn_get_has_astc(void);
static inline bool khrn_get_has_tfu(void);
static inline uint32_t khrn_get_vpm_size(void);
static inline uint32_t khrn_get_num_slices_per_core(void);
static inline uint32_t khrn_get_num_tmus_per_core(void);

VCOS_EXTERN_C_END

#include "khrn_process.inl"

#endif /* KHRN_PROCESS_H */
