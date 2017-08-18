/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_PROCESS_H
#define KHRN_PROCESS_H
#include <stdbool.h>
#include "libs/core/v3d/v3d_gen.h"
#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "khrn_int_common.h"
#include "libs/platform/gmem.h"

EXTERN_C_BEGIN

extern bool khrn_process_init(void);
extern void khrn_process_shutdown(void);
extern gmem_handle_t get_dummy_texture(void);

struct glxx_texture_sampler_state;
extern const struct glxx_tex_sampler_state *khrn_get_image_unit_default_sampler(void);

#if !V3D_HAS_GFXH1636_FIX
extern gmem_handle_t khrn_get_dummy_ocq_buffer(void);
#endif
extern const char *khrn_get_device_name(void);
extern const char *khrn_get_gl11_exts_str(void);
extern const char *khrn_get_gl3x_exts_str(void);
extern unsigned khrn_get_num_gl3x_exts(void);
extern const char *khrn_get_gl3x_ext(unsigned i);
extern gfx_lfmt_translate_gl_ext_t khrn_get_lfmt_translate_exts(void);
#if !V3D_VER_AT_LEAST(4,0,2,0)
extern const V3D_MISCCFG_T *khrn_get_hw_misccfg(void);
#endif

static inline uint32_t khrn_get_num_qpus_per_core(void);
static inline uint32_t khrn_get_num_render_subjobs(void);
static inline uint32_t khrn_get_num_bin_subjobs(void);
static inline uint32_t khrn_get_num_cores(void);
static inline bool khrn_get_has_astc(void);
static inline bool khrn_get_has_tfu(void);
static inline uint32_t khrn_get_vpm_size(void);
static inline uint32_t khrn_get_num_slices_per_core(void);
static inline uint32_t khrn_get_num_tmus_per_core(void);

EXTERN_C_END

#include "khrn_process.inl"

#endif /* KHRN_PROCESS_H */
