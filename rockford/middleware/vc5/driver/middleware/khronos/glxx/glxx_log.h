/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glxx
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "vcos.h"
#include "vcos_logging.h"

extern VCOS_LOG_CAT_T glxx_log;
extern VCOS_LOG_CAT_T glxx_query_log;
extern VCOS_LOG_CAT_T glxx_fbo_log;
extern VCOS_LOG_CAT_T glxx_tf_log;
extern VCOS_LOG_CAT_T glxx_buffer_log;
extern VCOS_LOG_CAT_T glxx_vao_log;
extern VCOS_LOG_CAT_T glxx_attrib_log;
extern VCOS_LOG_CAT_T glxx_error_log;
extern VCOS_LOG_CAT_T glxx_draw_log;
extern VCOS_LOG_CAT_T glxx_clear_log;
extern VCOS_LOG_CAT_T glxx_blit_log;
extern VCOS_LOG_CAT_T glxx_invalidate_log;
extern VCOS_LOG_CAT_T glxx_cle_log;
extern VCOS_LOG_CAT_T glxx_ubo_log;
extern VCOS_LOG_CAT_T glxx_program_log;
extern VCOS_LOG_CAT_T glxx_ldst_log;
extern VCOS_LOG_CAT_T glxx_depth_log;
extern VCOS_LOG_CAT_T glxx_stencil_log;
extern VCOS_LOG_CAT_T glxx_scissor_log;
extern VCOS_LOG_CAT_T glxx_blend_log;
extern VCOS_LOG_CAT_T glxx_texture_log;
extern VCOS_LOG_CAT_T glxx_teximage_log;
extern VCOS_LOG_CAT_T glxx_flush_log;
extern VCOS_LOG_CAT_T glxx_zonly_log;
extern VCOS_LOG_CAT_T glxx_bin_log;
extern VCOS_LOG_CAT_T glxx_render_log;
extern VCOS_LOG_CAT_T glxx_read_log;
extern VCOS_LOG_CAT_T glxx_uniform_log;
extern VCOS_LOG_CAT_T glxx_record_log;
extern VCOS_LOG_CAT_T glxx_rs_log;
extern VCOS_LOG_CAT_T glxx_prog_iface_log;

extern void glxx_trace_rs(const VCOS_LOG_CAT_T *cat, const void *rs, const char *msg);
