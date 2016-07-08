/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
GL extensions. Specifically the ones that are optionally supported
=============================================================================*/
#include "../common/khrn_int_common.h"
#include "../common/khrn_process.h"
#include "gl_public_api.h"

typedef struct {
   const char *name;
   bool (*supported)(void);
} OPT_EXTENSION_T;

static const OPT_EXTENSION_T all_extensions[] =
{
   {"GL_OES_compressed_ETC1_RGB8_texture", },
   {"GL_OES_compressed_paletted_texture", },
   {"GL_OES_texture_npot", },
   {"GL_OES_vertex_half_float", },
   {"GL_OES_EGL_image", },
   {"GL_OES_EGL_image_external", },
   {"GL_EXT_discard_framebuffer", },
   {"GL_OES_rgb8_rgba8", },
   {"GL_OES_depth24", },
   {"GL_OES_mapbuffer", },
   {"GL_OES_vertex_array_object", },
   {"GL_OES_packed_depth_stencil", },
   {"GL_OES_EGL_sync", },
   {"GL_OES_standard_derivatives", },
   {"GL_OES_surfaceless_context", },
   {"GL_EXT_shader_texture_lod", },
   {"GL_EXT_draw_elements_base_vertex", },
   {"GL_EXT_multisampled_render_to_texture", },
   {"GL_KHR_debug", },
   {"GL_OES_texture_stencil8", },
   {"GL_EXT_shader_integer_mix", },
#if KHRN_GLES31_DRIVER
   {"GL_OES_texture_storage_multisample_2d_array", },
#endif
#if GL_EXT_texture_format_BGRA8888
   {"GL_EXT_texture_format_BGRA8888", },
#endif
#if GL_EXT_debug_marker
   {"GL_EXT_debug_marker", },
#endif
#if GL_KHR_texture_compression_astc_ldr
   {"GL_KHR_texture_compression_astc_ldr", khrn_get_has_astc},
#endif
#if GL_EXT_color_buffer_float
   {"GL_EXT_color_buffer_float", },
#endif
#if GL_BRCM_multi_draw_indirect
   {"GL_BRCM_multi_draw_indirect", },
#endif
#if GL_BRCM_base_instance
   {"GL_BRCM_base_instance", },
#endif
#if GL_EXT_robustness
   {"GL_EXT_robustness", },
#endif

#if V3D_VER_AT_LEAST(3,3,0,0)
   {"GL_EXT_texture_sRGB_R8", },
   {"GL_EXT_texture_sRGB_RG8", },
#if GL_BRCM_texture_unnormalised_coords
   {"GL_BRCM_texture_unnormalised_coords", },
#endif
#if GL_BRCM_provoking_vertex
   {"GL_BRCM_provoking_vertex", },
#endif
#endif
};

unsigned int glxx_get_num_extensions(void)
{
   unsigned int i;
   const unsigned int n = sizeof (all_extensions) / sizeof (all_extensions[0]);
   unsigned int result = 0;

   for (i = 0; i < n; i++)
   {
      if (all_extensions[i].supported == NULL || all_extensions[i].supported())
         result++;
   }
   return result;
}

const char *glxx_get_extension(unsigned int index)
{
   unsigned int i;
   const unsigned int n = sizeof (all_extensions) / sizeof (all_extensions[0]);
   const char *result = NULL;

   for (i = 0; i < n; i++)
   {
      if (all_extensions[i].supported == NULL || all_extensions[i].supported())
      {
         if (index == 0)
         {
            result = all_extensions[i].name;
            break;
         }
         else
            index--;
      }
   }
   return result;
}
