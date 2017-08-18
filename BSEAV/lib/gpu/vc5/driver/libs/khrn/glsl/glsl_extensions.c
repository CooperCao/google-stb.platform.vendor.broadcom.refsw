/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <string.h>
#include "glsl_extensions.h"
#include "glsl_stdlib.auto.h"

enum glsl_ext tng_implies[] = { GLSL_EXT_IO_BLOCKS, GLSL_EXT_COUNT };
enum glsl_ext aep_implies[] = { GLSL_EXT_BLEND_EQUATION_ADVANCED, GLSL_EXT_SAMPLE_VARIABLES,  GLSL_EXT_IMAGE_ATOMIC,
                                GLSL_EXT_MS_INTERPOLATION,        GLSL_EXT_TEXTURE_2DMSARRAY, GLSL_EXT_GEOMETRY,
                                GLSL_EXT_GPU_SHADER5,             GLSL_EXT_BOUNDING_BOX_EXT,  GLSL_EXT_IO_BLOCKS,
                                GLSL_EXT_TESSELLATION,            GLSL_EXT_TEXTURE_BUFFER,    GLSL_EXT_CUBE_MAP_ARRAY, GLSL_EXT_COUNT };

static struct {
   const char          *identifiers[GLSL_EXT_MAX_ID_COUNT];
   bool                 supported;
   enum glsl_ext_status status;
   int                  stdlib_prop;
   enum glsl_ext       *implies;
} extensions[] = {
   {{"GL_OES_EGL_image_external",},                    true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_EGL_IMAGE_EXTERNAL,   NULL },
   {{"GL_BRCM_texture_1D",},                           true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_BRCM_TEXTURE_1D,          NULL },
   {{"GL_BRCM_sampler_fetch",},                        true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_BRCM_SAMPLER_FETCH,       NULL },
   {{"GL_BRCM_texture_gather_lod",},                   true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_BRCM_TEXTURE_GATHER_LOD,  NULL },
   {{"GL_EXT_shader_texture_lod",},                    true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_EXT_SHADER_TEXTURE_LOD,   NULL },
   {{"GL_OES_standard_derivatives",},                  true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_STANDARD_DERIVATIVES, NULL },
   {{"GL_EXT_shader_integer_mix",},                    true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_EXT_SHADER_INTEGER_MIX,   NULL },
   {{"GL_OES_texture_storage_multisample_2d_array",},  true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY, NULL },
   {{"GL_OES_shader_image_atomic",},                   true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_SHADER_IMAGE_ATOMIC,  NULL },
   /* Primitive bounding box has different symbol names for EXT and OES, so needs separate entries here */
   {{"GL_OES_primitive_bounding_box",},                true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_PRIMITIVE_BOUNDING_BOX, NULL },
   {{"GL_EXT_primitive_bounding_box",},                true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_EXT_PRIMITIVE_BOUNDING_BOX, NULL },
   {{"GL_KHR_blend_equation_advanced",},               true, GLSL_DISABLED, 0, NULL },

   {{"GL_BRCM_shader_framebuffer_fetch_depth_stencil",}, V3D_VER_AT_LEAST(4,0,2,0), GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_BRCM_SHADER_FRAMEBUFFER_FETCH_DEPTH_STENCIL, NULL },

   {{"GL_OES_sample_variables",},                      V3D_VER_AT_LEAST(4,0,2,0), GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_SAMPLE_VARIABLES, NULL},
   {{"GL_OES_shader_multisample_interpolation",},      V3D_VER_AT_LEAST(4,0,2,0), GLSL_DISABLED, 0, NULL },

   {{"GL_OES_texture_buffer",          "GL_EXT_texture_buffer",},         V3D_VER_AT_LEAST(4,0,2,0), GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_TEXTURE_BUFFER, NULL },
   {{"GL_OES_gpu_shader5",             "GL_EXT_gpu_shader5"},             V3D_VER_AT_LEAST(4,0,2,0), GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_GPU_SHADER5, NULL },
   {{"GL_OES_texture_cube_map_array",  "GL_EXT_texture_cube_map_array"},  V3D_VER_AT_LEAST(4,0,2,0), GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_TEXTURE_CUBE_MAP_ARRAY, NULL },
   {{"GL_OES_shader_io_blocks",        "GL_EXT_shader_io_blocks"},        false, GLSL_DISABLED, 0, NULL },

   {{"GL_OES_tessellation_shader",     "GL_EXT_tessellation_shader"},     V3D_VER_AT_LEAST(4,0,2,0), GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_TESSELLATION_SHADER, tng_implies },
   {{"GL_OES_tessellation_point_size", "GL_EXT_tessellation_point_size"}, V3D_VER_AT_LEAST(4,0,2,0), GLSL_DISABLED, 0, NULL },
   {{"GL_OES_geometry_shader",         "GL_EXT_geometry_shader"},         false, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_GEOMETRY_SHADER, tng_implies },
   {{"GL_OES_geometry_point_size",     "GL_EXT_geometry_point_size"},     false, GLSL_DISABLED, 0, NULL },

   {{"GL_BRCM_no_perspective",},     V3D_HAS_VARY_NO_PERSP, GLSL_DISABLED, 0, NULL },
   {{"GL_BRCM_image_formats",},      V3D_HAS_GFXH1638_FIX,  GLSL_DISABLED, 0, NULL },

   {{"GL_ANDROID_extension_pack_es31a", },     V3D_VER_AT_LEAST(4,0,2,0) && false, GLSL_DISABLED, 0, aep_implies },
};

void glsl_ext_init() {
   for (int i=0; i < GLSL_EXT_COUNT; i++) extensions[i].status = GLSL_DISABLED;
}

static void set_status(enum glsl_ext extension, enum glsl_ext_status status) {
   assert(extension < GLSL_EXT_COUNT || extension == GLSL_EXT_ALL);

   if (extension < GLSL_EXT_COUNT) {
      extensions[extension].status = status;
   } else {
      for (int i = 0; i < GLSL_EXT_COUNT; i++)
         if (extensions[i].supported)
            extensions[i].status = status;
   }
}

void glsl_ext_enable(enum glsl_ext extension, bool warn) {
   enum glsl_ext_status status = warn ? GLSL_ENABLED_WARN : GLSL_ENABLED;
   set_status(extension, status);

   /* Also enable any extensions that are implied by this one. This is not transitive */
   if (extension < GLSL_EXT_COUNT && extensions[extension].implies != NULL) {
      for (int i=0; extensions[extension].implies[i] != GLSL_EXT_COUNT; i++) {
         if (!warn || extensions[extensions[extension].implies[i]].status != GLSL_ENABLED)
            set_status(extensions[extension].implies[i], status);
      }
   }
}

void glsl_ext_disable(enum glsl_ext extension) {
   set_status(extension, GLSL_DISABLED);
}

enum glsl_ext_status glsl_ext_status(enum glsl_ext extension) {
   assert(extension < GLSL_EXT_COUNT);
   return extensions[extension].status;
}

enum glsl_ext glsl_ext_lookup(const char *identifier) {
   for (int i = 0; i < GLSL_EXT_COUNT; i++) {
      if (!extensions[i].supported) continue;

      for (int j = 0; j < GLSL_EXT_MAX_ID_COUNT; j++)
         if (extensions[i].identifiers[j] && !strcmp(extensions[i].identifiers[j], identifier))
            return i;
   }

   return GLSL_EXT_NOT_SUPPORTED;
}

const char *glsl_ext_get_identifier(unsigned ext, unsigned id) {
   assert(ext < (unsigned)GLSL_EXT_COUNT);
   assert(id < (unsigned)GLSL_EXT_MAX_ID_COUNT);
   if (!extensions[ext].supported) return NULL;
   else return extensions[ext].identifiers[id];
}

int glsl_ext_get_symbol_mask() {
   int mask = 0;
   for (int i=0; i<GLSL_EXT_COUNT; i++) {
      if (extensions[i].status != GLSL_DISABLED) mask |= extensions[i].stdlib_prop;
   }
   return mask;
}
