/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <string.h>
#include "glsl_extensions.h"
#include "glsl_stdlib.auto.h"

static struct {
   const char          *identifiers[GLSL_EXT_MAX_ID_COUNT];
   bool                 supported;
   enum glsl_ext_status status;
   int                  stdlib_prop;
} extensions[] = {
   {{"GL_OES_EGL_image_external",},                    true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_EGL_IMAGE_EXTERNAL },
   {{"GL_BRCM_texture_1D",},                           true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_BRCM_TEXTURE_1D },
   {{"GL_BRCM_sampler_fetch",},                        true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_BRCM_SAMPLER_FETCH },
   {{"GL_BRCM_texture_gather_lod",},                   true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_BRCM_TEXTURE_GATHER_LOD },
   {{"GL_EXT_shader_texture_lod",},                    true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_EXT_SHADER_TEXTURE_LOD },
   {{"GL_OES_standard_derivatives",},                  true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_STANDARD_DERIVATIVES },
   {{"GL_EXT_shader_integer_mix",},                    true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_EXT_SHADER_INTEGER_MIX },
   {{"GL_OES_texture_storage_multisample_2d_array",},  true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY },
   {{"GL_OES_shader_image_atomic",},                   true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_SHADER_IMAGE_ATOMIC },
   {{"GL_EXT_primitive_bounding_box",},                true, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_EXT_PRIMITIVE_BOUNDING_BOX },

   {{"GL_OES_sample_variables",},                      V3D_HAS_SRS, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_SAMPLE_VARIABLES},
   {{"GL_OES_shader_multisample_interpolation",},      V3D_HAS_SRS, GLSL_DISABLED, 0 },

   {{"GL_OES_gpu_shader5",            "GL_EXT_gpu_shader5"},            V3D_HAS_NEW_TMU_CFG, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_GPU_SHADER5 },
   {{"GL_OES_texture_cube_map_array", "GL_EXT_texture_cube_map_array"}, V3D_HAS_NEW_TMU_CFG, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_TEXTURE_CUBE_MAP_ARRAY },
   {{"GL_OES_shader_io_blocks",       "GL_EXT_shader_io_blocks"},       false, GLSL_DISABLED, 0 },
   {{"GL_OES_tessellation_shader",    "GL_EXT_tessellation_shader"},    V3D_HAS_TNG, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_TESSELLATION_SHADER },
   {{"GL_OES_geometry_shader",        "GL_EXT_geometry_shader"},        false, GLSL_DISABLED, GLSL_STDLIB_PROPERTY_GL_OES_GEOMETRY_SHADER },
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
