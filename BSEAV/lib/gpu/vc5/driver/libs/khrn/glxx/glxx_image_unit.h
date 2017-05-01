/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glxx_enum_types.h"
#include "glxx_texture.h"
#include "../glsl/glsl_program.h"

typedef enum
{
   enumify(GL_READ_ONLY),
   enumify(GL_WRITE_ONLY),
   enumify(GL_READ_WRITE)
}glxx_binding_access;

typedef enum
{
   enumify(GL_RGBA32F),
   enumify(GL_RGBA16F),
   enumify(GL_R32F),
   enumify(GL_RGBA32UI),
   enumify(GL_RGBA16UI),
   enumify(GL_RGBA8UI),
   enumify(GL_R32UI),
   enumify(GL_RGBA32I),
   enumify(GL_RGBA16I),
   enumify(GL_RGBA8I),
   enumify(GL_R32I),
   enumify(GL_RGBA8),
   enumify(GL_RGBA8_SNORM)
}glxx_image_unit_fmt;

typedef struct
{
   GLXX_TEXTURE_T *texture; /* this can be NULL */
   unsigned level;
   bool layered;  /* layered = true --> use all the layers of that level (if
                                        texture type has layers);
                     layered = false, use the specified layer; for cube_maps,
                               layer(s) translates to face */
   unsigned layer;
   glxx_binding_access access;
   glxx_image_unit_fmt internalformat;
}glxx_image_unit;

/* this gets calculated at draw time and it contains valid data for that
 * texture */
typedef struct glxx_calc_image_unit
{
   unsigned level;
   bool use_face_layer;
   unsigned layer;
   unsigned face;
   GFX_LFMT_T fmt;
   bool write; /* set to true if we are writing to this image */
} glxx_calc_image_unit;

void glxx_image_unit_init_default(glxx_image_unit *image_unit);
void glxx_image_unit_deinit(glxx_image_unit *image_unit);

GL_APICALL void GL_APIENTRY glBindImageTexture (GLuint unit, GLuint texture,
      GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);

typedef enum glxx_unit_acess
{
   GLXX_ACC_INVALID,
   GLXX_ACC_UNDEFINED,
   GLXX_ACC_OK
} glxx_unit_access;
extern glxx_unit_access glxx_get_calc_image_unit(const glxx_image_unit *image_unit,
      const GLSL_IMAGE_T *glsl_image_info, glxx_calc_image_unit *calc_image_unit);
