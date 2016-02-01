/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
GL extensions. Specifically the ones that are optionally supported
=============================================================================*/
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_process.h"
#include "interface/khronos/glxx/gl_public_api.h"
#include "middleware/khronos/ext/gl_brcm_provoking_vertex.h"

typedef bool (IS_SUPPORTED_T)(void);

typedef struct {
   const char *name;
   IS_SUPPORTED_T *supported;
} OPT_EXTENSION_T;

static bool always(void)
{
   return true;
}

static bool ver_33(void) {
   return (khrn_get_v3d_version() >= V3D_MAKE_VER(3, 3));
}

const OPT_EXTENSION_T gl30_all_extensions[] =
{
   {"GL_OES_compressed_ETC1_RGB8_texture", always},
   {"GL_OES_compressed_paletted_texture", always},
   {"GL_OES_texture_npot", always},
   {"GL_OES_vertex_half_float", always},
   {"GL_OES_EGL_image", always},
   {"GL_OES_EGL_image_external", always},
   {"GL_EXT_discard_framebuffer", always},
   {"GL_OES_rgb8_rgba8", always},
   {"GL_OES_depth24", always},
   {"GL_OES_mapbuffer", always},
   {"GL_OES_vertex_array_object", always},
   {"GL_OES_packed_depth_stencil", always},
   {"GL_OES_EGL_sync", always},
   {"GL_OES_standard_derivatives", always},
   {"GL_OES_surfaceless_context", always},
   {"GL_EXT_shader_texture_lod", always},
   {"GL_EXT_draw_elements_base_vertex", always},
   {"GL_EXT_multisampled_render_to_texture", always},
   {"GL_KHR_debug", always},
   {"GL_OES_texture_stencil8", always},
#if GL_EXT_texture_format_BGRA8888
   {"GL_EXT_texture_format_BGRA8888", always},
#endif
#if GL_EXT_debug_marker
   {"GL_EXT_debug_marker", always},
#endif
#if GL_KHR_texture_compression_astc_ldr
   {"GL_KHR_texture_compression_astc_ldr", khrn_get_has_astc},
#endif
#if GL_EXT_color_buffer_float
   {"GL_EXT_color_buffer_float", always},
#endif
#if GL_BRCM_multi_draw_indirect
   {"GL_BRCM_multi_draw_indirect", always},
#endif
#if GL_BRCM_base_instance
   {"GL_BRCM_base_instance", always},
#endif
   {"GL_EXT_texture_sRGB_R8", ver_33},
   {"GL_EXT_texture_sRGB_RG8", ver_33},
#if GL_BRCM_provoking_vertex
   {"GL_BRCM_provoking_vertex", gl_brcm_provoking_vertex_supported},
#endif
#if GL_EXT_robustness
   {"GL_EXT_robustness", always},
#endif
#if GL_BRCM_texture_unnormalised_coords
   {"GL_BRCM_texture_unnormalised_coords", ver_33},
#endif
};

unsigned int glxx_get_num_gl30_extensions(void)
{
   unsigned int i;
   const unsigned int n = sizeof (gl30_all_extensions) / sizeof (gl30_all_extensions[0]);
   unsigned int result = 0;

   for (i = 0; i < n; i++)
   {
      if (gl30_all_extensions[i].supported())
         result++;
   }
   return result;
}

const char *glxx_get_gl30_extension(unsigned int index)
{
   unsigned int i;
   const unsigned int n = sizeof (gl30_all_extensions) / sizeof (gl30_all_extensions[0]);
   const char *result = NULL;

   for (i = 0; i < n; i++)
   {
      if (gl30_all_extensions[i].supported())
      {
         if (index == 0)
         {
            result = gl30_all_extensions[i].name;
            break;
         }
         else
            index--;
      }
   }
   return result;
}
