/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/glxx/glxx_shared.h"

#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_server_internal.h"
#include "interface/khronos/include/GLES/glext.h"

#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/glxx/glxx_buffer.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/common/khrn_interlock.h"
#include "middleware/khronos/common/khrn_hw.h"

#include "interface/khronos/glxx/gl11_int_config.h"
#include "middleware/khronos/gl20/gl20_config.h"

//#include "../gl11/hw.h"
//#include "../gl20/hw.h"
#include "middleware/khronos/glxx/glxx_hw.h"

#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/gl20/gl20_shader.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"

#include "interface/khronos/common/khrn_client_platform.h"
#include "middleware/khronos/glxx/glxx_tweaker.h"

#include "vcfw/drivers/chip/abstract_v3d.h"
#include "interface/khronos/common/khrn_client.h"
#include "middleware/khronos/common/khrn_mem.h"

#include <string.h>
#include <math.h>
#include <limits.h>

static VCOS_MUTEX_T gl_lock;

static MEM_HANDLE_T buffer_handle(GLXX_BUFFER_T *buffer)
{
   if (buffer != NULL) {
      MEM_HANDLE_T hresult = glxx_buffer_get_storage_handle(buffer);
      return hresult;
   } else
      return MEM_HANDLE_INVALID;
}

static MEM_HANDLE_T buffer_interlock_offset(GLXX_BUFFER_T *buffer)
{
   if (buffer != NULL) {
      uint32_t offset = glxx_buffer_get_interlock_offset(buffer);
      assert(offset<=sizeof(GLXX_BUFFER_T));
      return offset;
   } else
      return ~0;
}

static bool valid_frame_buffer(GLXX_SERVER_STATE_T *state)
{
   return state->bound_framebuffer != NULL;
}

// We don't need access to the draw, depth and stencil images. The only things
// which care about them are the HAL itself, which does its own thing, and the
// bit depth queries, which only require the image format.
KHRN_IMAGE_FORMAT_T glxx_get_draw_image_format(GLXX_SERVER_STATE_T *state)
{
   KHRN_IMAGE_FORMAT_T result = IMAGE_FORMAT_INVALID;
   if (valid_frame_buffer(state)) {
      GLXX_FRAMEBUFFER_T *framebuffer = state->bound_framebuffer;
      KHRN_IMAGE_T *image = glxx_attachment_info_get_images(&framebuffer->attachments.color, NULL);
      if (image != NULL)
         result = image->format;
   } else {
      KHRN_IMAGE_T *image = state->draw;
      result = image->format;
   }
   return result;
}

KHRN_IMAGE_FORMAT_T glxx_get_depth_image_format(GLXX_SERVER_STATE_T *state)
{
   KHRN_IMAGE_FORMAT_T result = IMAGE_FORMAT_INVALID;
   if (valid_frame_buffer(state)) {
      GLXX_FRAMEBUFFER_T *framebuffer = state->bound_framebuffer;
      GLXX_RENDERBUFFER_T *renderbuffer = framebuffer->attachments.depth.object;

      if (renderbuffer != NULL) {
         switch (renderbuffer->type) {
         case RB_DEPTH16_T:
            result = DEPTH_16_TF;
            break;
         case RB_DEPTH24_T:
         case RB_DEPTH24_STENCIL8_T:
            result = DEPTH_32_TF;
            break;
         default:
         /* default: leave as IMAGE_FORMAT_INVALID */
            break;
         }
      }
   } else if (state->depth != NULL) {
      KHRN_IMAGE_T *image = state->depth;
      if (state->config_depth_bits > 0)
         result = image->format;
   } // else leave as IMAGE_FORMAT_INVALID
   return result;
}

static KHRN_IMAGE_FORMAT_T get_stencil_image_format(GLXX_SERVER_STATE_T *state)
{
   KHRN_IMAGE_FORMAT_T result = IMAGE_FORMAT_INVALID;
   if (valid_frame_buffer(state)) {
      GLXX_FRAMEBUFFER_T *framebuffer = state->bound_framebuffer;
      GLXX_RENDERBUFFER_T *renderbuffer = framebuffer->attachments.stencil.object;

      if (renderbuffer != NULL) {
         switch (renderbuffer->type) {
         case RB_STENCIL_T:
         case RB_DEPTH24_STENCIL8_T:
            result = DEPTH_32_TF;
            break;
         default:
         /* default: leave as IMAGE_FORMAT_INVALID */
            break;
         }
      }

      return result;
   } else if (state->depth != NULL) {
      KHRN_IMAGE_T *image = state->depth;
      if (state->config_stencil_bits > 0)
         result = image->format;
   } // else leave as IMAGE_FORMAT_INVALID
   return result;
}

/*
   uint32_t get_stencil_size(GLXX_SERVER_STATE_T *state)

   Returns the number of stencil bits per pixel in the stencil buffer, or 0 if there is
   no stencil buffer.

   Preconditions:

   state is a valid GLXX_SERVER_STATE_T object;

   Postconditions:

   result < 32.
*/

uint32_t glxx_get_stencil_size(GLXX_SERVER_STATE_T *state)
{
   uint32_t result = 0;
   KHRN_IMAGE_FORMAT_T format = get_stencil_image_format(state);
   if(format!=IMAGE_FORMAT_INVALID)
      result = khrn_image_get_stencil_size(format);
   return result;
}

static GLXX_TEXTURE_T *create_texture(GLenum target)
{
   GLXX_TEXTURE_T *texture = KHRN_MEM_ALLOC_STRUCT(GLXX_TEXTURE_T);
   if (texture == NULL) return NULL;

   khrn_mem_set_term(texture, glxx_texture_term);
   glxx_texture_init(texture, 0, target);
   return texture;
}

/*
   initialises common portions of the GLXX_SERVER_STATE_T state structure
   this function is called by the OpenGL ES version specific functions
   gl11_server_state_init and gl20_server_state_init
*/

bool glxx_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared)
{
   GLXX_TEXTURE_T *texture_cube = NULL, *texture, *texture_external;

   KHRN_MEM_ASSIGN(state->shared, shared);

   /*
      do texture stuff first so that we have
      the default texture to assign to the texture units
   */
   texture = create_texture(GL_TEXTURE_2D);
   if (texture == NULL)
       return false;

   KHRN_MEM_ASSIGN(state->default_texture_twod, texture);

   texture_external = create_texture(GL_TEXTURE_EXTERNAL_OES);
   if (texture_external == NULL)
       return false;

   KHRN_MEM_ASSIGN(state->default_texture_external, texture_external);

   if(!IS_GL_11(state)) {
      /*
         And the cube map texture too
      */
      texture_cube = create_texture(GL_TEXTURE_CUBE_MAP);
      if (texture_cube == NULL)
         return false;

      KHRN_MEM_ASSIGN(state->default_texture_cube, texture_cube);
   }
   /*
      now the rest of the structure
   */

   state->active_texture = GL_TEXTURE0;

   state->error = GL_NO_ERROR;

   assert(state->bound_buffer.array_buffer == NULL);
   assert(state->bound_buffer.element_array_buffer == NULL);

   if(IS_GL_11(state)) {
      for (int i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
         KHRN_MEM_ASSIGN(state->bound_texture[i].twod, texture);
         KHRN_MEM_ASSIGN(state->bound_texture[i].external, texture_external);
      }
   } else {
      for (int i = 0; i < GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS; i++) {
         KHRN_MEM_ASSIGN(state->bound_texture[i].twod, texture);
         KHRN_MEM_ASSIGN(state->bound_texture[i].external, texture_external);
         KHRN_MEM_ASSIGN(state->bound_texture[i].cube, texture_cube);
      }
   }
   KHRN_MEM_ASSIGN(texture_cube, NULL);
   KHRN_MEM_ASSIGN(texture, NULL);
   KHRN_MEM_ASSIGN(texture_external, NULL);

   state->clear_color[0] = 0.0f;
   state->clear_color[1] = 0.0f;
   state->clear_color[2] = 0.0f;
   state->clear_color[3] = 0.0f;

   state->clear_depth = 1.0f;

   state->clear_stencil = 0;

   state->shader.common.blend.color_mask = 0xffffffff;

   state->cull_mode = GL_BACK;

   state->depth_func = GL_LESS;

   state->depth_mask = GL_TRUE;

   state->caps.cull_face = false;
   state->caps.polygon_offset_fill = false;
   state->caps.sample_coverage = false;
   state->caps.sample_alpha_to_coverage = false;
   state->caps.scissor_test = false;
   state->caps.stencil_test = false;
   state->caps.depth_test = false;
   state->caps.blend = false;
   state->caps.dither = true;
   state->caps.multisample = true;

   state->front_face = GL_CCW;

   state->hints.generate_mipmap = GL_DONT_CARE;

   state->line_width = 1.0f;

   state->polygon_offset.factor = 0.0f;
   state->polygon_offset.units = 0.0f;

   state->sample_coverage.value = 1.0f;
   state->sample_coverage.invert = GL_FALSE;

   state->scissor.x = 0;
   state->scissor.y = 0;
   state->scissor.width = 0;
   state->scissor.height = 0;

   state->stencil_func.front.func = GL_ALWAYS;
   state->stencil_func.front.ref = 0;
   state->stencil_func.front.mask = 0xffffffff;
   state->stencil_mask.front = 0xffffffff;

   state->stencil_op.front.fail = GL_KEEP;
   state->stencil_op.front.zfail = GL_KEEP;
   state->stencil_op.front.zpass = GL_KEEP;

   state->stencil_func.back.func = GL_ALWAYS;
   state->stencil_func.back.ref = 0;
   state->stencil_func.back.mask = 0xffffffff;
   state->stencil_mask.back = 0xffffffff;

   state->stencil_op.back.fail = GL_KEEP;
   state->stencil_op.back.zfail = GL_KEEP;
   state->stencil_op.back.zpass = GL_KEEP;

   state->blend_func.src_rgb = GL_ONE;
   state->blend_func.dst_rgb = GL_ZERO;
   state->blend_func.src_alpha = GL_ONE;
   state->blend_func.dst_alpha = GL_ZERO;

   state->blend_color[0] = 0.0f;
   state->blend_color[1] = 0.0f;
   state->blend_color[2] = 0.0f;
   state->blend_color[3] = 0.0f;

   state->blend_equation.rgb = GL_FUNC_ADD;
   state->blend_equation.alpha = GL_FUNC_ADD;

   state->viewport.x = 0;
   state->viewport.y = 0;
   state->viewport.width = 0;
   state->viewport.height = 0;
   state->viewport.near = 0.0f;
   state->viewport.far = 1.0f;

   glxx_update_viewport_internal(state);

   assert(state->draw == NULL);
   assert(state->read == NULL);
   assert(state->depth == NULL);
   assert(state->color_multi == NULL);
   assert(state->ds_multi == NULL);

   state->changed_misc = IS_GL_11(state);
   state->changed_light = IS_GL_11(state);
   state->changed_texunit = true;
   state->changed_backend = true;
   state->changed_vertex = true;
   state->changed_directly = true;

   state->current_render_state = 0;    /* = No render state */
   state->changed_cfg = true;
   state->changed_linewidth = true;
   state->changed_polygon_offset = true;
   state->changed_viewport = true;
   state->old_flat_shading_flags = ~0;

   state->made_current = GL_FALSE;

   assert(state->bound_renderbuffer == NULL);
   assert(state->bound_framebuffer == NULL);

   state->alignment.pack = 4;
   state->alignment.unpack = 4;

   state->bound_buffer.array_name = 0;
   state->bound_buffer.element_array_name = 0;

   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++) {
      state->attrib[i].enabled = GL_FALSE;
      state->attrib[i].size = 4;
      state->attrib[i].type = GL_FLOAT;
      state->attrib[i].normalized = GL_FALSE;
      state->attrib[i].stride = 0;
      state->attrib[i].buffer = 0;
      state->attrib[i].pointer = NULL;
      state->attrib[i].offset = 0;
      state->attrib[i].value[0] = 0.0f;
      state->attrib[i].value[1] = 0.0f;
      state->attrib[i].value[2] = 0.0f;
      state->attrib[i].value[3] = 1.0f;
   }

   return true;
}

void glxx_server_state_flush(bool wait)
{
   glxx_hw_finish_context(wait);
}

GLXX_TEXTURE_T *glxx_server_state_get_texture(GLXX_SERVER_STATE_T *state, GLenum target, GLboolean use_face)
{
   GLXX_TEXTURE_T *texture = NULL;

   switch (target) {
   case GL_TEXTURE_2D:
      texture = state->bound_texture[state->active_texture - GL_TEXTURE0].twod;
      break;
   case GL_TEXTURE_EXTERNAL_OES:
      texture = state->bound_texture[state->active_texture - GL_TEXTURE0].external;
      break;
   case GL_TEXTURE_CUBE_MAP:
      if (IS_GL_11(state)) {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);//gl 2.0 only
         return NULL;
      }
      else {
         if (!use_face)
            texture = state->bound_texture[state->active_texture - GL_TEXTURE0].cube;
      }
      break;
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      if (IS_GL_11(state)) {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);//gl 2.0 only
         return NULL;
      }
      else {
         if (use_face)
            texture = state->bound_texture[state->active_texture - GL_TEXTURE0].cube;
      }
      break;
   }

   if (texture == NULL) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return NULL;
   }

   return texture;
}

void glxx_server_state_set_buffers(GLXX_SERVER_STATE_T *state,
   KHRN_IMAGE_T *draw,
   KHRN_IMAGE_T *read,
   KHRN_IMAGE_T *depth,
   KHRN_IMAGE_T *color_multi,
   KHRN_IMAGE_T *ds_multi,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits)
{
   KHRN_MEM_ASSIGN(state->draw, draw);
   KHRN_MEM_ASSIGN(state->read, read);
   KHRN_MEM_ASSIGN(state->depth, depth);
   KHRN_MEM_ASSIGN(state->color_multi, color_multi);
   KHRN_MEM_ASSIGN(state->ds_multi, ds_multi);

   state->config_depth_bits = config_depth_bits;
   state->config_stencil_bits = config_stencil_bits;

   if (!state->made_current) {
      state->scissor.x = 0;
      state->scissor.y = 0;
      state->scissor.width = draw->width;
      state->scissor.height = draw->height;

      state->viewport.x = 0;
      state->viewport.y = 0;
      state->viewport.width = draw->width;
      state->viewport.height = draw->height;

      glxx_update_viewport_internal(state);

#ifdef KHRN_SIMPLE_MULTISAMPLE
      state->scissor.width /= 2;
      state->scissor.height /= 2;
      state->viewport.width /= 2;
      state->viewport.height /= 2;
#endif
      state->made_current = GL_TRUE;
   }
}

void glxx_server_state_term(void *p)
{
   GLXX_SERVER_STATE_T *state = p;

   glxx_context_gl_lock();
   // We want to make sure that any external buffers modified by this context
   // get updated.
   glxx_server_state_flush(true); /* todo: do we need to wait here? */
   glxx_context_gl_unlock();

   if (!IS_GL_11(state))
      KHRN_MEM_ASSIGN(state->program, NULL);

   KHRN_MEM_ASSIGN(state->bound_renderbuffer, NULL);
   KHRN_MEM_ASSIGN(state->bound_framebuffer, NULL);

   KHRN_MEM_ASSIGN(state->bound_buffer.array_buffer, NULL);
   KHRN_MEM_ASSIGN(state->bound_buffer.element_array_buffer, NULL);

   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      KHRN_MEM_ASSIGN(state->attrib[i].attrib, NULL);

   for (int i = 0; i < GLXX_CONFIG_MAX_TEXTURE_UNITS; i++) {
      KHRN_MEM_ASSIGN(state->bound_texture[i].twod, NULL);
      KHRN_MEM_ASSIGN(state->bound_texture[i].external, NULL);
      KHRN_MEM_ASSIGN(state->bound_texture[i].cube, NULL);
   }

   KHRN_MEM_ASSIGN(state->shared, NULL);
   KHRN_MEM_ASSIGN(state->default_texture_twod, NULL);
   KHRN_MEM_ASSIGN(state->default_texture_external, NULL);
   KHRN_MEM_ASSIGN(state->default_texture_cube, NULL);

   KHRN_MEM_ASSIGN(state->draw, NULL);
   KHRN_MEM_ASSIGN(state->read, NULL);
   KHRN_MEM_ASSIGN(state->depth, NULL);
   KHRN_MEM_ASSIGN(state->color_multi, NULL);
   KHRN_MEM_ASSIGN(state->ds_multi, NULL);
}

//TODO are there any other operations which need a valid framebuffer and
//therefore need to call this function, that I've forgotten about?
GLenum glxx_check_framebuffer_status(GLXX_SERVER_STATE_T *state, GLenum target)
{
   GLenum result = GL_FRAMEBUFFER_COMPLETE;

   assert(state);

   if (target == GL_FRAMEBUFFER) {
      if (valid_frame_buffer(state)) {
         GLXX_FRAMEBUFFER_T *framebuffer = state->bound_framebuffer;
         result = glxx_framebuffer_check_status(framebuffer);
      } else {
         // If there is no bound buffer then we are using the
         // window-system-provided framebuffer. This is always complete.
      }
   } else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      result = 0;
   }

   return result;
}

/*
   glBlendFuncSeparate()

   Sets the RGB and alpha source and destination blend functions to one of

      GL_ZERO
      GL_ONE
      GL_SRC_COLOR
      GL_ONE_MINUS_SRC_COLOR
      GL_DST_COLOR
      GL_ONE_MINUS_DST_COLOR
      GL_SRC_ALPHA
      GL_ONE_MINUS_SRC_ALPHA
      GL_DST_ALPHA
      GL_ONE_MINUS_DST_ALPHA
      GL_CONSTANT_COLOR
      GL_ONE_MINUS_CONSTANT_COLOR
      GL_CONSTANT_ALPHA
      GL_ONE_MINUS_CONSTANT_ALPHA
      GL_SRC_ALPHA_SATURATE*

   Gives GL_INVALID_ENUM error if any function is not one of these.

   *source functions only

   Implementation: Done
   Error Checks: Done
*/

static GLboolean is_blend_func(GLXX_SERVER_STATE_T *state, GLenum func, GLboolean is_src)
{
   //gl 1.1
   return func == GL_ZERO ||
          func == GL_ONE ||
          func == GL_SRC_ALPHA ||
          func == GL_ONE_MINUS_SRC_ALPHA ||
          func == GL_DST_ALPHA ||
          func == GL_ONE_MINUS_DST_ALPHA ||
          (func == GL_SRC_ALPHA_SATURATE && is_src) ||
          (IS_GL_11(state) && (
                (func == GL_SRC_COLOR && !is_src) ||
                (func == GL_ONE_MINUS_SRC_COLOR && !is_src) ||
                (func == GL_DST_COLOR && is_src) ||
                (func == GL_ONE_MINUS_DST_COLOR && is_src)
                )) ||
          (!IS_GL_11(state) && (
             func == GL_SRC_COLOR ||
             func == GL_ONE_MINUS_SRC_COLOR ||
             func == GL_DST_COLOR ||
             func == GL_ONE_MINUS_DST_COLOR ||
             func == GL_CONSTANT_COLOR ||
             func == GL_ONE_MINUS_CONSTANT_COLOR ||
             func == GL_CONSTANT_ALPHA ||
             func == GL_ONE_MINUS_CONSTANT_ALPHA
          ));
}

void glxx_server_set_blend_func(GLXX_SERVER_STATE_T *state, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) // S
{
   if (is_blend_func(state, srcRGB, GL_TRUE) && is_blend_func(state, dstRGB, GL_FALSE) &&
       is_blend_func(state, srcAlpha, GL_TRUE) && is_blend_func(state, dstAlpha, GL_FALSE))
   {
      state->changed_backend = true;
      state->blend_func.src_rgb = srcRGB;
      state->blend_func.dst_rgb = dstRGB;
      state->blend_func.src_alpha = srcAlpha;
      state->blend_func.dst_alpha = dstAlpha;
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
}

GL_API void GL_APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   glxx_server_set_blend_func(state, sfactor, dfactor, sfactor, dfactor);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_API void GL_APIENTRY glClear(GLbitfield mask)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (glxx_check_framebuffer_status(state, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      /*  If a buffer is not present, then a Clear directed at that buffer has no effect */
   }
   else if (mask != 0 && !glxx_hw_clear(mask & GL_COLOR_BUFFER_BIT ? GL_TRUE : GL_FALSE,
                   mask & GL_DEPTH_BUFFER_BIT ? GL_TRUE : GL_FALSE,
                   mask & GL_STENCIL_BUFFER_BIT ? GL_TRUE : GL_FALSE,
                   state))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}


static KHRN_IMAGE_FORMAT_T set_palette(GLenum internalformat, const void *data, void **ret, uint32_t *ps)
{
   KHRN_IMAGE_FORMAT_T format;
   uint32_t *palette;
   uint32_t i, palsize;
   const uint8_t *ptr = (const uint8_t *)data;

   switch (internalformat)
   {
      case GL_PALETTE4_RGB8_OES:
         palsize = 16;
         format = XBGR_8888_TF;
         break;
      case GL_PALETTE4_RGBA8_OES:
         palsize = 16;
         format = ABGR_8888_TF;
         break;
      case GL_PALETTE4_R5_G6_B5_OES:
         palsize = 16;
         format = RGB_565_TF;
         break;
      case GL_PALETTE4_RGBA4_OES:
         palsize = 16;
         format = RGBA_4444_TF;
         break;
      case GL_PALETTE4_RGB5_A1_OES:
         palsize = 16;
         format = RGBA_5551_TF;
         break;
      case GL_PALETTE8_RGB8_OES:
         palsize = 256;
         format = XBGR_8888_TF;
         break;
      case GL_PALETTE8_RGBA8_OES:
         palsize = 256;
         format = ABGR_8888_TF;
         break;
      case GL_PALETTE8_R5_G6_B5_OES:
         palsize = 256;
         format = RGB_565_TF;
         break;
      case GL_PALETTE8_RGBA4_OES:
         palsize = 256;
         format = RGBA_4444_TF;
         break;
      case GL_PALETTE8_RGB5_A1_OES:
         palsize = 256;
         format = RGBA_5551_TF;
         break;
      default:
         palsize = 0;
         format = IMAGE_FORMAT_INVALID;
         UNREACHABLE();
   }

   palette = (uint32_t *)malloc(4 * palsize);
   if (palette == NULL)
      return IMAGE_FORMAT_INVALID;

   for (i = 0; i < palsize; i++)
   {
      switch (internalformat)
      {
      case GL_PALETTE4_RGB8_OES:
      case GL_PALETTE8_RGB8_OES:
#ifdef BIG_ENDIAN_CPU
         palette[i] = *(uint32_t*)ptr | 0x000000ff;
#else
         palette[i] = (uint32_t)ptr[0] | (uint32_t)ptr[1] << 8 | (uint32_t)ptr[2] << 16 | 0xff000000;
#endif
         ptr += 3;
         break;
      case GL_PALETTE4_RGBA8_OES:
      case GL_PALETTE8_RGBA8_OES:
         palette[i] = *(uint32_t*)ptr;
         ptr += 4;
         break;
      case GL_PALETTE4_R5_G6_B5_OES:
      case GL_PALETTE4_RGBA4_OES:
      case GL_PALETTE4_RGB5_A1_OES:
      case GL_PALETTE8_R5_G6_B5_OES:
      case GL_PALETTE8_RGBA4_OES:
      case GL_PALETTE8_RGB5_A1_OES:
         palette[i] = *(uint16_t*)ptr;
         ptr += 2;
         break;
      default:
         UNREACHABLE();
      }
   }

   *ret = (void *)palette;
   *ps = palsize;

   return format;
}

static uint32_t get_palette_size(GLenum internalformat)
{
   switch (internalformat)
   {
   case GL_PALETTE4_RGB8_OES: return 16 * 3;
   case GL_PALETTE4_RGBA8_OES: return 16 * 4;
   case GL_PALETTE4_R5_G6_B5_OES: return 16 * 2;
   case GL_PALETTE4_RGBA4_OES: return 16 * 2;
   case GL_PALETTE4_RGB5_A1_OES: return 16 * 2;
   case GL_PALETTE8_RGB8_OES: return 256 * 3;
   case GL_PALETTE8_RGBA8_OES: return 256 * 4;
   case GL_PALETTE8_R5_G6_B5_OES: return 256 * 2;
   case GL_PALETTE8_RGBA4_OES: return 256 * 2;
   case GL_PALETTE8_RGB5_A1_OES: return 256 * 2;
   default:
      UNREACHABLE();
      return 0;
   }
}

GL_API void GL_APIENTRY glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (target == GL_TEXTURE_EXTERNAL_OES) {
       glxx_server_state_set_error(state, GL_INVALID_ENUM);
       goto end;
   }

   UNUSED(imageSize);

   switch (internalformat)
   {
   case GL_ETC1_RGB8_OES:
   {
      //TODO check imageSize field is correct
      GLXX_TEXTURE_T *texture = glxx_server_state_get_texture(state, target, GL_TRUE);

      state->changed_texunit = true; /* GLES1.1 - may change has_color/has_alpha */
      if (texture) {
         if (data == NULL)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
         else if (level < 0 || level > LOG2_MAX_TEXTURE_SIZE)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
         else if (width < 0 || width > MAX_TEXTURE_SIZE || height < 0 || height > MAX_TEXTURE_SIZE)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
         else if (border != 0)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
         else if (IS_GL_11(state) && target != GL_TEXTURE_2D)
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         else if (!IS_GL_11(state) && target != GL_TEXTURE_2D && width != height)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
         else if (!glxx_texture_etc1_blank_image(texture, target, level, width, height))
            glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         else
         {
            glxx_texture_etc1_sub_image(texture, target, level, 0, 0, width, height, data);
         }
      }
      break;
   }
   case GL_PALETTE4_RGB8_OES:
   case GL_PALETTE4_RGBA8_OES:
   case GL_PALETTE4_R5_G6_B5_OES:
   case GL_PALETTE4_RGBA4_OES:
   case GL_PALETTE4_RGB5_A1_OES:
   case GL_PALETTE8_RGB8_OES:
   case GL_PALETTE8_RGBA8_OES:
   case GL_PALETTE8_R5_G6_B5_OES:
   case GL_PALETTE8_RGBA4_OES:
   case GL_PALETTE8_RGB5_A1_OES:
   {
      //TODO check imageSize field is correct
      GLXX_TEXTURE_T *texture = glxx_server_state_get_texture(state, target, GL_TRUE);
      if (texture) {
         if (data == NULL)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
         else if (level < 0 || level > LOG2_MAX_TEXTURE_SIZE)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
         else if (width < 0 || width > MAX_TEXTURE_SIZE || height < 0 || height > MAX_TEXTURE_SIZE)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
         else if (1<<level > width && 1<<level > height)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);   //TODO is this a valid check?
         else if (border != 0)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
         else if (IS_GL_11(state) && target != GL_TEXTURE_2D)
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         else if (!IS_GL_11(state) && target != GL_TEXTURE_2D && width != height)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
         else
         {
            KHRN_IMAGE_FORMAT_T format;
            void *palette = NULL;
            uint32_t ps;
            state->changed_texunit = true;   /* May change has_color/_alpha */
            format = set_palette(internalformat, data, &palette, &ps);
            if (format == IMAGE_FORMAT_INVALID)
               glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
            else if (!glxx_texture_paletted_blank_image(texture, target, level, width, height, format))
               glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
            else
            {
               glxx_texture_paletted_sub_image(texture, target, level, width, height, (const uint32_t *)palette,
                  ps, (char *)data + get_palette_size(internalformat), imageSize);
            }

            /* destroy the palette now upload complete */
            free(palette);
         }
      }
      break;
   }
   default:
      // Some format we don't recognise
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

end:
   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_API void GL_APIENTRY glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
   UNUSED(target);
   UNUSED(level);
   UNUSED(xoffset);
   UNUSED(yoffset);
   UNUSED(width);
   UNUSED(height);
   UNUSED(imageSize);
   UNUSED(data);

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   switch (format)
   {
   case GL_ETC1_RGB8_OES:
   case GL_PALETTE4_RGB8_OES:
   case GL_PALETTE4_RGBA8_OES:
   case GL_PALETTE4_R5_G6_B5_OES:
   case GL_PALETTE4_RGBA4_OES:
   case GL_PALETTE4_RGB5_A1_OES:
   case GL_PALETTE8_RGB8_OES:
   case GL_PALETTE8_RGBA8_OES:
   case GL_PALETTE8_R5_G6_B5_OES:
   case GL_PALETTE8_RGBA4_OES:
   case GL_PALETTE8_RGB5_A1_OES:
      // Cannot specify subimages of paletted textures
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      break;
   default:
      // Some format we don't recognise
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      break;
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

static int format_is_copy_compatible(GLenum internalformat, KHRN_IMAGE_FORMAT_T srcformat)
{
   switch (srcformat & ~IMAGE_FORMAT_PRE)
   {
   case ABGR_8888_TF:
   case RGBA_4444_TF:
   case RGBA_5551_TF:
   case ABGR_8888_RSO:
   case RGBA_8888_RSO:
      return internalformat == GL_RGBA
          || internalformat == GL_RGB
          || internalformat == GL_LUMINANCE_ALPHA
          || internalformat == GL_LUMINANCE
          || internalformat == GL_ALPHA;
   case XBGR_8888_TF:
   case RGB_565_TF:
   case RGBX_8888_RSO:
   case XBGR_8888_RSO:
   case RGB_565_RSO:
      return internalformat == GL_RGB
          || internalformat == GL_LUMINANCE;
   default:
      // Source image should be a framebuffer. Other image formats aren't
      // used for framebuffers.
      // Unreachable
      UNREACHABLE();
      return 0;
   }
}

static GLboolean is_texture_internal_format(GLenum internalformat)
{
   return internalformat == GL_ALPHA ||
          internalformat == GL_LUMINANCE ||
          internalformat == GL_LUMINANCE_ALPHA ||
          internalformat == GL_RGB ||
          internalformat == GL_RGBA ||
#if GL_EXT_texture_format_BGRA8888
          internalformat == GL_BGRA_EXT ||
#endif
#if GL_APPLE_rgb_422
          internalformat == GL_RGB_422_APPLE ||
#endif
          0;
}

static KHRN_IMAGE_T *get_read_image(GLXX_SERVER_STATE_T *state)
{
   KHRN_IMAGE_T *image = NULL;

   if (valid_frame_buffer(state)) {
      GLXX_FRAMEBUFFER_T *framebuffer = state->bound_framebuffer;
      image = glxx_attachment_info_get_images(&framebuffer->attachments.color, NULL);
      assert(image != NULL);      //TODO: is this valid?
   } else
      image = state->read;

   return image;
}

GL_API void GL_APIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (target == GL_TEXTURE_EXTERNAL_OES) {
       glxx_server_state_set_error(state, GL_INVALID_ENUM);
       goto end;
   }

   if (glxx_check_framebuffer_status(state, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
   {
      glxx_server_state_set_error(state, GL_INVALID_FRAMEBUFFER_OPERATION);
      goto end;
   }

   GLXX_TEXTURE_T *texture = glxx_server_state_get_texture(state, target, GL_TRUE);
   if (!texture) goto end;

   if (level < 0 || level > LOG2_MAX_TEXTURE_SIZE)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (width < 0 || width > MAX_TEXTURE_SIZE || height < 0 || height > MAX_TEXTURE_SIZE)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (IS_GL_11(state) && target != GL_TEXTURE_2D)
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else if (!IS_GL_11(state) && target != GL_TEXTURE_2D && width != height)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (border != 0 || !is_texture_internal_format(internalformat))
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
#if GL_APPLE_rgb_422
   else if ((internalformat == GL_RGB_422_APPLE) && ((width & 1) != 0 || (x & 1) != 0))
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
#endif
   else
   {
      KHRN_IMAGE_T *src = get_read_image(state);
      if (!format_is_copy_compatible(internalformat, src->format))
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      else
      {
         khrn_interlock_read_immediate(&src->interlock);

         /* src image was generated by HW, flush to get good copy */
         KHRN_IMAGE_WRAP_T src_wrap;
         khrn_image_lock_wrap(src, &src_wrap);
         khrn_hw_flush_dcache_range(src_wrap.storage, khrn_image_get_size(src_wrap.format, src_wrap.width, src_wrap.height));
         khrn_image_unlock_wrap(src);

         state->changed_texunit = true; /* GLES1.1 - may change has_color/has_alpha */

         if (!glxx_texture_copy_image(texture, target, level, width, height, internalformat, src, x, y))
            glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      }
   }
end:
   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GLboolean glxx_is_texture_target(GLXX_SERVER_STATE_T *state, GLenum target)
{
   switch (target)
   {
      case GL_TEXTURE_2D:
      case GL_TEXTURE_EXTERNAL_OES:
         return true;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
         return !IS_GL_11(state);
      default:
         return false;
   }
}

GL_API void GL_APIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (target == GL_TEXTURE_EXTERNAL_OES) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (glxx_check_framebuffer_status(state, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
   {
      glxx_server_state_set_error(state, GL_INVALID_FRAMEBUFFER_OPERATION);
      goto end;
   }

   GLXX_TEXTURE_T *texture = glxx_server_state_get_texture(state, target, GL_TRUE);
   if (!texture) goto end;

   if (!glxx_is_texture_target(state,target))
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else if (width < 0 || width > MAX_TEXTURE_SIZE || height < 0 || height > MAX_TEXTURE_SIZE)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (!glxx_texture_includes(texture, target, level, xoffset, yoffset))
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (!glxx_texture_includes(texture, target, level, xoffset + width - 1, yoffset + height - 1))
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (!khrn_image_is_uncomp(glxx_texture_incomplete_get_mipmap_format(texture, target, level)))
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
   else if (khrn_image_is_yuv422(glxx_texture_incomplete_get_mipmap_format(texture, target, level)) && ((width & 1) != 0 || (xoffset & 1) != 0))
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
   else if (width > 0 && height > 0)
   {
      KHRN_IMAGE_T *src = get_read_image(state);
      //TODO: check formats are compatible?

      khrn_interlock_read_immediate(&src->interlock);

      /* src image was generated by HW, flush to get good copy */
      KHRN_IMAGE_WRAP_T src_wrap;
      khrn_image_lock_wrap(src, &src_wrap);
      khrn_hw_flush_dcache_range(src_wrap.storage, khrn_image_get_size(src_wrap.format, src_wrap.width, src_wrap.height));
      khrn_image_unlock_wrap(src);

      if (x < 0) { xoffset -= x; width += x;  x = 0; }
      if (y < 0) { yoffset -= y; height += y; y = 0; }
      if (width > src->width   || x + width > src->width)   { width = src->width - x; }
      if (height > src->height || y + height > src->height) { height = src->height - y; }

      if (width > 0 && height > 0)
      {
         if (!glxx_texture_copy_sub_image(texture, target, level, xoffset, yoffset, width, height, src, x, y))
            glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      }
   }
end:
   glxx_unlock_server_state(OPENGL_ES_ANY);
}

/*
   Check if 'func' is a valid depth or stencil test function.
*/

static GLboolean is_func(GLenum func)
{
   return func == GL_NEVER ||
          func == GL_ALWAYS ||
          func == GL_LESS ||
          func == GL_LEQUAL ||
          func == GL_EQUAL ||
          func == GL_GREATER ||
          func == GL_GEQUAL ||
          func == GL_NOTEQUAL;
}

/*
   glDepthFunc()

   Sets the function which determines whether a pixel passes or fails the
   depth test, specifying one of

      GL_NEVER
      GL_ALWAYS
      GL_LESS
      GL_LEQUAL
      GL_EQUAL
      GL_GREATER
      GL_GEQUAL
      GL_NOTEQUAL

   Gives GL_INVALID_ENUM error if the function is not one of these.

   Implementation: Done
   Error Checks: Done
*/

GL_API void GL_APIENTRY glDepthFunc(GLenum func)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (is_func(func))
   {
      state->changed_cfg = true;
      state->depth_func = func;

      glxx_tweaker_setdepthfunc(&state->tweak_state, func);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

/*
   glDepthMask()

   Sets the write enable for the depth buffer. No errors are generated.

   Implementation: Done
   Error Checks: Done
*/

GL_API void GL_APIENTRY glDepthMask(GLboolean flag)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   state->changed_cfg = true;
   state->depth_mask = clean_boolean(flag);

   glxx_tweaker_setdepthmask(&state->tweak_state, flag ? true : false);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

void glxx_update_color_material(GLXX_SERVER_STATE_T *state)
{
   if (state->shader.color_material) {
      int i;

      for (i = 0; i < 4; i++) {
         state->material.ambient[i] = state->attrib[GL11_IX_COLOR].value[i];
         state->material.diffuse[i] = state->attrib[GL11_IX_COLOR].value[i];
      }
   }
}

static void set_enabled(GLenum cap, bool enabled)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   assert(GL11_CONFIG_MAX_PLANES == 1);
   assert(GL11_CONFIG_MAX_LIGHTS == 8);

   switch (cap) {
   case GL_NORMALIZE:
      if(IS_GL_11(state)) {
         state->changed_light = true;
         state->shader.normalize = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_RESCALE_NORMAL:
      if(IS_GL_11(state)) {
         state->changed_light = true;
         state->shader.rescale_normal = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_FOG:
      if(IS_GL_11(state)) {
         state->changed_misc = true;
         state->caps_fragment.fog = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_LIGHTING:
      if(IS_GL_11(state)) {
         state->changed_light = true;
         state->shader.lighting = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_COLOR_MATERIAL:
      if(IS_GL_11(state)) {
         state->changed_vertex = true;
         state->shader.color_material = enabled;
         glxx_update_color_material(state);
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_LIGHT0:
   case GL_LIGHT1:
   case GL_LIGHT2:
   case GL_LIGHT3:
   case GL_LIGHT4:
   case GL_LIGHT5:
   case GL_LIGHT6:
   case GL_LIGHT7:
      if(IS_GL_11(state)) {
         state->changed_light = true;
         state->lights[cap - GL_LIGHT0].enabled = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_TEXTURE_2D:
   case GL_TEXTURE_EXTERNAL_OES:
      if(IS_GL_11(state)) {
         state->changed_texunit = true;
         state->texunits[state->active_texture - GL_TEXTURE0].target_enabled = enabled ? cap : GL_NONE;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_ALPHA_TEST:
      if(IS_GL_11(state)) {
         state->changed_misc = true;
         state->caps_fragment.alpha_test = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_CLIP_PLANE0:
      if(IS_GL_11(state)) {
         state->changed_misc = true;
         state->caps.clip_plane[cap - GL_CLIP_PLANE0] = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_POINT_SMOOTH:
      if(IS_GL_11(state)) {
         state->changed_misc = true;
         state->caps_fragment.point_smooth = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_POINT_SPRITE_OES:
      if(IS_GL_11(state)) {
         state->changed_misc = true;
         state->caps_fragment.point_sprite = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_LINE_SMOOTH:
      if(IS_GL_11(state)) {
         state->changed_misc = true;
         state->caps_fragment.line_smooth = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_CULL_FACE:
      state->changed_cfg = true;
      state->caps.cull_face = enabled;
      break;
   case GL_POLYGON_OFFSET_FILL:
      state->changed_cfg = true;
      state->caps.polygon_offset_fill = enabled;
      break;
   case GL_MULTISAMPLE:
      if(IS_GL_11(state)) {
         state->changed_cfg = true;
         state->caps.multisample = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_SAMPLE_ALPHA_TO_COVERAGE:
      state->changed_backend = true;
      state->caps.sample_alpha_to_coverage = enabled;
      break;
   case GL_SAMPLE_ALPHA_TO_ONE:
      if(IS_GL_11(state)) {
         state->changed_backend = true;
         state->caps.sample_alpha_to_one = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_SAMPLE_COVERAGE:
      state->changed_backend = true;
      state->caps.sample_coverage = enabled;
      break;
   case GL_SCISSOR_TEST:
      state->changed_viewport = true;
      state->caps.scissor_test = enabled;
      break;
   case GL_STENCIL_TEST:
      state->changed_backend = true;
      state->caps.stencil_test = enabled;
      break;
   case GL_DEPTH_TEST:
      state->changed_cfg = true;
      state->caps.depth_test = enabled;
      break;
   case GL_BLEND:
      state->changed_backend = true;
      state->caps.blend = enabled;
      glxx_tweaker_setblendenabled(&state->tweak_state, enabled);
      break;
   case GL_DITHER:
      state->caps.dither = enabled;
      break;
   case GL_COLOR_LOGIC_OP:
      if(IS_GL_11(state)) {
         state->changed_backend = true;
         state->caps.color_logic_op = enabled;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_API void GL_APIENTRY glDisable(GLenum cap)
{
   set_enabled(cap, false);
}

static int translate_max_index(GLXX_BUFFER_T *buffer, uintptr_t v, GLint size, GLenum type, GLsizei stride)
{
   if (buffer != NULL) {
      int attrib_size, actual_stride, buffer_size, result;
      intptr_t offset;

      assert(buffer);   // should never be possible to reach here with a deleted or non-existent buffer
      assert(size >= 1 && size <= 4);

      attrib_size = khrn_get_type_size(type) * size;
      actual_stride = stride ? stride : attrib_size;
      offset = (intptr_t)v;
      buffer_size = glxx_buffer_get_size(buffer);

      assert(actual_stride > 0);
      assert(attrib_size > 0);

      if (offset < 0 || offset + attrib_size > buffer_size)
         result = 0;    //Not even the first vertex will fit in the buffer
      else {
         result = (buffer_size - offset - attrib_size) / actual_stride + 1;
         assert(result >= 1);
         assert(offset + (result-1) * actual_stride + attrib_size <= buffer_size);
         assert(offset +  result    * actual_stride + attrib_size >  buffer_size);
      }

      return result;
   } else
      return INT_MAX;
}

/*
   Fills in attrib_handles and returns the maximum index which
   is permitted across all attributes. (INT_MAX if we don't care).
*/

static int set_attrib_handles(GLXX_SERVER_STATE_T *state, GLXX_ATTRIB_T *attribs, MEM_HANDLE_T *attrib_handles)
{
   int i;
   int result = INT_MAX;
   uint32_t live = state->batch.cattribs_live | state->batch.vattribs_live;

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      if (attribs[i].enabled && (live & 0xf << (4*i))) {
         result = _min(result,translate_max_index(state->attrib[i].attrib, attribs[i].offset, attribs[i].size, attribs[i].type, attribs[i].stride));
         if (state->attrib[i].attrib)
            attrib_handles[i] = buffer_handle(state->attrib[i].attrib);
         else
            attrib_handles[i] = MEM_HANDLE_INVALID;
      } else
         attrib_handles[i] = MEM_HANDLE_INVALID;

   return result;
}

static GLboolean is_index_type(GLenum type)
{
   return type == GL_UNSIGNED_BYTE ||
          type == GL_UNSIGNED_SHORT;
}

static void draw_elements (GLXX_SERVER_STATE_T *state, GLenum mode,
      GLsizei count, GLenum type,
      GLXX_BUFFER_T *indices, uint32_t indices_offset)
{
   GLXX_ATTRIB_T *attribs = state->attrib;
   POINTER_OFFSET_T interlocks[GLXX_CONFIG_MAX_VERTEX_ATTRIBS+1];
   int i, interlock_count;

   /* SW-5891 glDrawArrays should handle >65536 vertices, fix is implemented for all but triangle fans and line loops  */
   if(type==0 && ((int)indices_offset + count) > 65536 && (mode == GL_TRIANGLE_FAN || mode == GL_LINE_LOOP))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto fail_or_done;
   }

   INCR_DRIVER_COUNTER(draw_calls);

   { /* block kept to avoid changing indentation of every line below */
      bool indices_ok;
      int max_index;
      MEM_HANDLE_T indices_handle;
      MEM_HANDLE_T attrib_handles[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];

      state->batch.primitive_mode = mode;    /* Needed for getting attr_live. Should mode be checked for validity? */

      if (!glxx_hw_get_attr_live(state, attribs))
      {
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         goto fail_or_done;
      }
      max_index = set_attrib_handles(state, attribs, attrib_handles);

      if(type==0) {
         int first = (int)indices_offset;
         indices_ok = first >= 0 && first < max_index && first + count <= max_index;
         assert(indices == NULL);
         indices_handle = MEM_HANDLE_INVALID;
      }
      else {
         int type_size;
         uint32_t buffer_size;

         assert(indices != NULL);
         indices_handle = buffer_handle(indices);

         type_size = khrn_get_type_size(type);
         buffer_size = glxx_buffer_get_size(indices);

         // Need to check two things:
         // - we don't look beyond the index buffer object (the easy bit)
         // - none of the indices go beyond any vertex buffer object, i.e. max_index
         // Note that it is assumed that anything not coming from a buffer object is safe.
         // (The client will have worked out how much data it needs and sent that
         // much across).

         indices_ok = indices_offset <= buffer_size && indices_offset + count * type_size <= buffer_size
            && max_index > 0;

      }

// adjust count to match mode so no degenerate primitives are left on the end
// see Open GL ES 1.1, 2.0 spec and HW-2858
      if (type==0 || is_index_type(type))
      {
         switch (mode) {
            case GL_POINTS:
               count = count < 1 ? 0 : count;
               break;
            case GL_LINES:
               count = count < 2 ? 0 : ( count % 2 != 0 ? (count/2)*2 : count );
               break;
            case GL_LINE_LOOP:
            case GL_LINE_STRIP:
               count = count < 2 ? 0 : count;
               break;
            case GL_TRIANGLES:
               count = count < 3 ? 0 : ( count % 3 != 0 ? (count/3)*3 : count );
               break;
            case GL_TRIANGLE_STRIP:
            case GL_TRIANGLE_FAN:
               count = count < 3 ? 0 : count;
               break;
            default:
               glxx_server_state_set_error(state, GL_INVALID_ENUM);
               break;
         }
      }
//

      if (glxx_check_framebuffer_status(state, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
         glxx_server_state_set_error(state, GL_INVALID_FRAMEBUFFER_OPERATION);
      else if (!IS_GL_11(state) && !gl20_validate_current_program(state))
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      else if (count >= 0 && (!IS_GL_11(state) || attribs[GL11_IX_VERTEX].enabled) && indices_ok) {
         if (type==0 || is_index_type(type))
            switch (mode) {
            case GL_POINTS:
            case GL_LINES:
            case GL_LINE_LOOP:
            case GL_LINE_STRIP:
            case GL_TRIANGLES:
            case GL_TRIANGLE_STRIP:
            case GL_TRIANGLE_FAN:
               interlock_count = 0;
               for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
               {
                  if (attribs[i].enabled && attribs[i].attrib != NULL)
                  {
                     interlocks[interlock_count].p = attribs[i].attrib;
                     interlocks[interlock_count].offset = buffer_interlock_offset(attribs[i].attrib);
                     interlock_count++;
                  }
               }
               if (type != 0 && indices != NULL)
               {
                  interlocks[interlock_count].p = indices;
                  interlocks[interlock_count].offset = buffer_interlock_offset(indices);
                  interlock_count++;
               }
               assert(interlock_count <= GLXX_CONFIG_MAX_VERTEX_ATTRIBS+1);
               if (!glxx_hw_draw_triangles(count, type, indices_offset, state, attribs, indices_handle, attrib_handles,
                  max_index, interlocks, interlock_count))
                  glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
               break;
            default:
               glxx_server_state_set_error(state, GL_INVALID_ENUM);
               break;
            }
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }

fail_or_done:
   return;
}

GL_API void GL_APIENTRY glEnable(GLenum cap)
{
   set_enabled(cap, true);
}

void flush_internal(bool wait)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (state)
   {
      glxx_server_state_flush(wait);
      glxx_unlock_server_state(OPENGL_ES_ANY);
   }
}

GL_API void GL_APIENTRY glFinish(void)
{
   flush_internal(true);
}

GL_API void GL_APIENTRY glFlush(void)
{
   flush_internal(false);
}

static GLboolean is_front_face(GLenum mode)
{
   return mode == GL_CW ||
          mode == GL_CCW;
}

/*
   glFrontFace()

   Sets which winding order is considered to be front facing, specifying
   one of CW (clockwise) or CCW (counterclockwise). Gives GL_INVALID_ENUM
   error if the mode is not one of these.

   Implementation: Done
   Error Checks: Done
*/

GL_API void GL_APIENTRY glFrontFace(GLenum mode)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (is_front_face(mode))
   {
      state->changed_cfg = true;
      state->front_face = mode;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_API void GL_APIENTRY glGenBuffers(GLsizei n, GLuint *buffers)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_SHARED_T *shared = state->shared;

   int32_t i = 0;

   if (n < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);   // The conformance tests insist...
   else if (buffers)
      while (i < n) {
         if (glxx_shared_get_buffer(shared, shared->next_buffer, false) == NULL)
            buffers[i++] = shared->next_buffer;

         shared->next_buffer++;
      }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_API void GL_APIENTRY glGenTextures(GLsizei n, GLuint *textures)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (n < 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);   // The conformance tests insist...
      goto end;
   }

   GLXX_SHARED_T *shared = state->shared;

   if (textures)
   {
      int32_t i = 0;
      while (i < n) {
         if (glxx_shared_get_texture(shared, shared->next_texture) == NULL)
            textures[i++] = shared->next_texture;

         shared->next_texture++;
      }
   }

end:
   glxx_unlock_server_state(OPENGL_ES_ANY);
}

/*
   GetError

   Current error code(s) NO ERROR GetError
*/

GL_API GLenum GL_APIENTRY glGetError(void)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return GL_NO_ERROR;

   GLenum result = state->error;

   state->error = GL_NO_ERROR;

   glxx_unlock_server_state(OPENGL_ES_ANY);

   return result;
}

GLboolean glxx_is_boolean (GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch (pname) {
   //common gl 1.1 and 2.0 booleans
   case GL_SAMPLE_COVERAGE_INVERT:
   case GL_COLOR_WRITEMASK:
   case GL_DEPTH_WRITEMASK:
   case GL_CULL_FACE:
   case GL_POLYGON_OFFSET_FILL:
   case GL_SAMPLE_ALPHA_TO_COVERAGE:
   case GL_SAMPLE_COVERAGE:
   case GL_SCISSOR_TEST:
   case GL_STENCIL_TEST:
   case GL_DEPTH_TEST:
   case GL_BLEND:
   case GL_DITHER:
      return GL_TRUE;
   //gl 1.1 specific booleans
   case GL_LIGHT_MODEL_TWO_SIDE:
   case GL_NORMALIZE:
   case GL_RESCALE_NORMAL:
   case GL_CLIP_PLANE0:
   case GL_FOG:
   case GL_LIGHTING:
   case GL_COLOR_MATERIAL:
   case GL_LIGHT0:
   case GL_LIGHT1:
   case GL_LIGHT2:
   case GL_LIGHT3:
   case GL_LIGHT4:
   case GL_LIGHT5:
   case GL_LIGHT6:
   case GL_LIGHT7:
   case GL_POINT_SMOOTH:
   case GL_POINT_SPRITE_OES:
   case GL_LINE_SMOOTH:
   case GL_MULTISAMPLE:
   case GL_SAMPLE_ALPHA_TO_ONE:
   case GL_TEXTURE_2D:
   case GL_ALPHA_TEST:
   case GL_COLOR_LOGIC_OP:
   case GL_VERTEX_ARRAY:
   case GL_NORMAL_ARRAY:
   case GL_COLOR_ARRAY:
   case GL_TEXTURE_COORD_ARRAY:
   case GL_POINT_SIZE_ARRAY_OES:
      return IS_GL_11(state);
   //gl 2.0 specific booleans
   case GL_SHADER_COMPILER:
      return !IS_GL_11(state);
   default:
      return GL_FALSE;
   }
}

GLboolean glxx_is_integer (GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch (pname) {
   //common gl 1.1 and 2.0 integers
   case GL_VIEWPORT:
   case GL_CULL_FACE_MODE:
   case GL_FRONT_FACE:
   case GL_TEXTURE_BINDING_2D:
   case GL_TEXTURE_BINDING_EXTERNAL_OES:
   case GL_ACTIVE_TEXTURE:
   case GL_SCISSOR_BOX:
   case GL_STENCIL_FUNC:
   case GL_STENCIL_WRITEMASK:
   case GL_STENCIL_CLEAR_VALUE:
   case GL_STENCIL_VALUE_MASK:
   case GL_STENCIL_REF:
   case GL_STENCIL_FAIL:
   case GL_STENCIL_PASS_DEPTH_FAIL:
   case GL_STENCIL_PASS_DEPTH_PASS:
   case GL_DEPTH_FUNC:
   case GL_GENERATE_MIPMAP_HINT:
   case GL_SUBPIXEL_BITS:
   case GL_MAX_TEXTURE_SIZE:
   case GL_MAX_VIEWPORT_DIMS:
   case GL_SAMPLE_BUFFERS:
   case GL_SAMPLES:
   case GL_COMPRESSED_TEXTURE_FORMATS:
   case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
   case GL_RED_BITS:
   case GL_GREEN_BITS:
   case GL_BLUE_BITS:
   case GL_ALPHA_BITS:
   case GL_DEPTH_BITS:
   case GL_STENCIL_BITS:
   case GL_FRAMEBUFFER_BINDING:
   case GL_RENDERBUFFER_BINDING:
   case GL_MAX_RENDERBUFFER_SIZE:
   case GL_MAX_SAMPLES_EXT:
   case GL_ARRAY_BUFFER_BINDING:
   case GL_ELEMENT_ARRAY_BUFFER_BINDING:
   case GL_IMPLEMENTATION_COLOR_READ_TYPE:
   case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
   case GL_UNPACK_ALIGNMENT:
   case GL_PACK_ALIGNMENT:
      return GL_TRUE;
   //gl 1.1 specific integers
   case GL_MODELVIEW_STACK_DEPTH:
   case GL_PROJECTION_STACK_DEPTH:
   case GL_TEXTURE_STACK_DEPTH:
   case GL_MATRIX_MODE:
   case GL_FOG_MODE:
   case GL_SHADE_MODEL:
   case GL_ALPHA_TEST_FUNC:
   case GL_BLEND_SRC:
   case GL_BLEND_DST:
   case GL_LOGIC_OP_MODE:
   case GL_PERSPECTIVE_CORRECTION_HINT:
   case GL_POINT_SMOOTH_HINT:
   case GL_LINE_SMOOTH_HINT:
   case GL_FOG_HINT:
   case GL_MAX_LIGHTS:
   case GL_MAX_CLIP_PLANES:
   case GL_MAX_MODELVIEW_STACK_DEPTH:
   case GL_MAX_PROJECTION_STACK_DEPTH:
   case GL_MAX_TEXTURE_STACK_DEPTH:
   case GL_MAX_TEXTURE_UNITS:
   case GL_MODELVIEW_MATRIX_FLOAT_AS_INT_BITS_OES:
   case GL_PROJECTION_MATRIX_FLOAT_AS_INT_BITS_OES:
   case GL_TEXTURE_MATRIX_FLOAT_AS_INT_BITS_OES:
   case GL_CLIENT_ACTIVE_TEXTURE:
   case GL_VERTEX_ARRAY_SIZE:
   case GL_VERTEX_ARRAY_TYPE:
   case GL_VERTEX_ARRAY_STRIDE:
   case GL_NORMAL_ARRAY_TYPE:
   case GL_NORMAL_ARRAY_STRIDE:
   case GL_COLOR_ARRAY_SIZE:
   case GL_COLOR_ARRAY_TYPE:
   case GL_COLOR_ARRAY_STRIDE:
   case GL_TEXTURE_COORD_ARRAY_SIZE:
   case GL_TEXTURE_COORD_ARRAY_TYPE:
   case GL_TEXTURE_COORD_ARRAY_STRIDE:
   case GL_POINT_SIZE_ARRAY_TYPE_OES:
   case GL_POINT_SIZE_ARRAY_STRIDE_OES:
   case GL_VERTEX_ARRAY_BUFFER_BINDING:
   case GL_NORMAL_ARRAY_BUFFER_BINDING:
   case GL_COLOR_ARRAY_BUFFER_BINDING:
   case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
   case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
      return IS_GL_11(state);
   //gl 2.0 specific integers
   case GL_TEXTURE_BINDING_CUBE_MAP:
   case GL_STENCIL_BACK_WRITEMASK:
   case GL_STENCIL_BACK_FUNC:
   case GL_STENCIL_BACK_VALUE_MASK:
   case GL_STENCIL_BACK_REF:
   case GL_STENCIL_BACK_FAIL:
   case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
   case GL_STENCIL_BACK_PASS_DEPTH_PASS:
   case GL_BLEND_SRC_RGB:
   case GL_BLEND_SRC_ALPHA:
   case GL_BLEND_DST_RGB:
   case GL_BLEND_DST_ALPHA:
   case GL_BLEND_EQUATION_RGB:
   case GL_BLEND_EQUATION_ALPHA:
   case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
   case GL_SHADER_BINARY_FORMATS:
   case GL_NUM_SHADER_BINARY_FORMATS:
   case GL_MAX_VERTEX_ATTRIBS:
   case GL_MAX_VERTEX_UNIFORM_VECTORS:
   case GL_MAX_VARYING_VECTORS:
   case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
   case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
   case GL_MAX_TEXTURE_IMAGE_UNITS:
   case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
   case GL_CURRENT_PROGRAM:
      return !IS_GL_11(state);
   default:
      return GL_FALSE;
   }
}

static GLboolean is_small_float_glxx (GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch (pname) {
   //common gl 1.1 and 2.0 floats in range 0-1
   case GL_DEPTH_RANGE:
   case GL_COLOR_CLEAR_VALUE:
   case GL_DEPTH_CLEAR_VALUE:
      return GL_TRUE;
   //gl 1.1 specific floats in range 0-1
   case GL_FOG_COLOR:
   case GL_LIGHT_MODEL_AMBIENT:
   case GL_ALPHA_TEST_REF:
      return IS_GL_11(state);
   //gl 2.0 specific floats in range 0-1
   case GL_BLEND_COLOR:
      return !IS_GL_11(state);
   default:
      return GL_FALSE;
   }
}

GLboolean glxx_is_float (GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch (pname) {
   //common gl 1.1 and 2.0 floats
   case GL_DEPTH_RANGE:
   case GL_LINE_WIDTH:
   case GL_POLYGON_OFFSET_FACTOR:
   case GL_POLYGON_OFFSET_UNITS:
   case GL_SAMPLE_COVERAGE_VALUE:
   case GL_COLOR_CLEAR_VALUE:
   case GL_DEPTH_CLEAR_VALUE:
   case GL_ALIASED_POINT_SIZE_RANGE:
   case GL_ALIASED_LINE_WIDTH_RANGE:
      return GL_TRUE;
   //gl 1.1 specific floats
   case GL_FOG_COLOR:
   case GL_LIGHT_MODEL_AMBIENT:
   case GL_MODELVIEW_MATRIX:
   case GL_PROJECTION_MATRIX:
   case GL_TEXTURE_MATRIX:
   case GL_FOG_DENSITY:
   case GL_FOG_START:
   case GL_FOG_END:
   case GL_POINT_SIZE_MIN:
   case GL_POINT_SIZE_MAX:
   case GL_POINT_FADE_THRESHOLD_SIZE:
   case GL_POINT_DISTANCE_ATTENUATION:
   case GL_ALPHA_TEST_REF:
   case GL_SMOOTH_POINT_SIZE_RANGE:
   case GL_SMOOTH_LINE_WIDTH_RANGE:
   case GL_CURRENT_TEXTURE_COORDS:
   case GL_CURRENT_COLOR:
   case GL_CURRENT_NORMAL:
   case GL_POINT_SIZE:
      return IS_GL_11(state);
   //gl 2.0 specific floats
   case GL_BLEND_COLOR:
      return !IS_GL_11(state);
   default:
      return GL_FALSE;
   }

}

/*
   glGetBooleanv()

   Gets the value(s) of a specified state variable into an array of
   booleans. Native integer and float variables return GL_FALSE if zero
   and GL_TRUE if non-zero. Gives GL_INVALID_ENUM if the state variable
   does not exist.

   Implementation: Done
   Error Checks: Done
*/

GL_API void GL_APIENTRY glGetBooleanv(GLenum pname, GLboolean *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (glxx_is_boolean(state, pname))
      glxx_get_boolean_internal(state, pname, params);
   else if (glxx_is_integer(state, pname))
   {
      GLint temp[16];
      GLuint count = glxx_get_integer_internal(state, pname, temp);
      GLuint i;

      assert(count <= 16);

      for (i = 0; i < count; i++)
         params[i] = temp[i] != 0;
   }
   else if (glxx_is_float(state, pname))
   {
      GLfloat temp[16];
      GLuint count = glxx_get_float_internal(state, pname, temp);
      GLuint i;

      assert(count <= 16);

      for (i = 0; i < count; i++)
         params[i] = temp[i] != 0.0f;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

/*
   GetBufferParameteriv

   BUFFER SIZE 0 GetBufferParameteriv
   BUFFER USAGE STATIC DRAW GetBufferParameteriv
   BUFFER ACCESS WRITE ONLY GetBufferParameteriv
   BUFFER MAPPED False GetBufferParameteriv
*/

GL_API void GL_APIENTRY glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_BUFFER_T *buffer;

   buffer = glxx_get_bound_buffer(state, target);

   if (buffer) {
      switch (pname) {
      case GL_BUFFER_SIZE:
         params[0] = mem_get_size(glxx_buffer_get_storage_handle(buffer));
         break;
      case GL_BUFFER_USAGE:
         params[0] = buffer->usage;
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_API void GL_APIENTRY glGetIntegerv(GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (glxx_is_boolean(state, pname))
   {
      GLboolean temp[4];
      GLuint count = glxx_get_boolean_internal(state, pname, temp);
      GLuint i;

      assert(count <= 4);

      for (i = 0; i < count; i++)
         params[i] = temp[i] ? 1 : 0;
   }
   else if (glxx_is_integer(state, pname))
      glxx_get_integer_internal(state, pname, params);
   else if (is_small_float_glxx(state, pname))
   {
      GLfloat temp[4];
      GLuint count = glxx_get_float_internal(state, pname, temp);
      GLuint i;

      assert(count <= 4);

      for (i = 0; i < count; i++) {
         params[i] = (GLint)floor((4294967295.0f * temp[i] - 1.0f) / 2.0f + 0.5f);

         if (params[i] < 0)
            params[i] = 0x7fffffff;
      }
   }
   else if (glxx_is_float(state, pname))
   {
      GLfloat temp[16];
      GLuint count = glxx_get_float_internal(state, pname, temp);
      GLuint i;

      assert(count <= 16);

      for (i = 0; i < count; i++)
         params[i] = float_to_int(temp[i]);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

#define MAX_RENDERER_STRLEN 64
#define MAX_RENDERERVER_STRLEN 8

GL_API const GLubyte * GL_APIENTRY glGetString(GLenum name)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return NULL;

   const GLubyte *result = NULL;

   static GLubyte renderer[MAX_RENDERER_STRLEN];
   GLubyte version[MAX_RENDERERVER_STRLEN];
   switch (name) {
   case GL_VENDOR:
#ifndef NDEBUG
      result = "Broadcom DEBUG";
#else
      result = "Broadcom";
#endif
      break;
   case GL_RENDERER:
      get_core_revision_internal(MAX_RENDERERVER_STRLEN, version);
      snprintf((char *)renderer, sizeof(renderer), "VideoCore IV HW (V3D-%s)", version);
      result = renderer;
      break;

   default:
      if (IS_GL_11(state)) {
         switch (name) {
         case GL_VERSION:
            result = (const GLubyte *)"OpenGL ES-CM 1.1";
            break;
         case GL_EXTENSIONS:
            result = (const GLubyte *)"GL_OES_compressed_ETC1_RGB8_texture "
               "GL_OES_compressed_paletted_texture "
               "GL_OES_texture_npot "
               "GL_OES_EGL_image "
               "GL_EXT_discard_framebuffer "
               "GL_OES_query_matrix "
               "GL_OES_framebuffer_object "
               "GL_OES_rgb8_rgba8 "
               "GL_OES_depth24 "
               "GL_OES_depth32 "
               "GL_OES_stencil8 "
               "GL_OES_draw_texture "
               "GL_OES_packed_depth_stencil "
               "GL_EXT_debug_marker "
               "GL_EXT_multisampled_render_to_texture "
               "GL_EXT_texture_format_BGRA8888 "
               "GL_EXT_read_format_bgra "
#ifdef ANDROID
               "GL_OES_EGL_image_external "
#endif
#ifdef EGL_KHR_fence_sync
               "GL_OES_EGL_sync "
#endif
               ;
            break;
         default:
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
            result = NULL;
            break;
         }
      }
      else if (IS_GL_20(state)) {
         switch (name) {
         case GL_VERSION:
            result = (const GLubyte *)"OpenGL ES 2.0";
            break;
         case GL_SHADING_LANGUAGE_VERSION:
            result = (const GLubyte *)"OpenGL ES GLSL ES 1.00";
            break;
         case GL_EXTENSIONS:
            result = (const GLubyte *)"GL_OES_compressed_ETC1_RGB8_texture "
               "GL_OES_compressed_paletted_texture "
               "GL_OES_texture_npot "
               "GL_OES_depth24 "
               "GL_OES_vertex_half_float "
               "GL_OES_EGL_image "
               "GL_EXT_discard_framebuffer "
               "GL_OES_rgb8_rgba8 "
               "GL_OES_depth32 "
               "GL_APPLE_rgb_422 "
               "GL_OES_packed_depth_stencil "
               "GL_EXT_debug_marker "
               "GL_EXT_multisampled_render_to_texture "
               "GL_EXT_texture_format_BGRA8888 "
               "GL_EXT_read_format_bgra "
#ifdef ANDROID
               "GL_OES_EGL_image_external "
#endif
#ifdef EGL_KHR_fence_sync
               "GL_OES_EGL_sync "
#endif
               ;
            break;
         default:
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
            result = NULL;
            break;
         }
      }
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);

   return result;
}

/*
   GetTexParameteriv

   TEXTURE MIN FILTER NEAREST MIPMAP LINEAR GetTexParameteriv
   TEXTURE MAG FILTER LINEAR GetTexParameteriv
   TEXTURE WRAP S REPEAT GetTexParameteriv
   TEXTURE WRAP T REPEAT GetTexParameteriv
   GENERATE MIPMAP GetTexParameter //gl 1.1 specific
*/

int glxx_get_texparameter_internal(GLenum target, GLenum pname, GLint *params)
{
   GLXX_TEXTURE_T *texture;

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return 0;

   texture = glxx_server_state_get_texture(state, target, GL_FALSE);

   int result = 1;
   if (texture) {
      switch (pname) {
      case GL_TEXTURE_MIN_FILTER:
         params[0] = texture->min;
         break;
      case GL_TEXTURE_MAG_FILTER:
         params[0] = texture->mag;
         break;
      case GL_TEXTURE_WRAP_S:
         params[0] = texture->wrap.s;
         break;
      case GL_TEXTURE_WRAP_T:
         params[0] = texture->wrap.t;
         break;
      case GL_GENERATE_MIPMAP:
         if (IS_GL_11(state) && target != GL_TEXTURE_EXTERNAL_OES)
            params[0] = texture->generate_mipmap;
         else {
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
            result = 0;
         }
         break;
      case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
         params[0] = 1;
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
         break;
      }
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);

   return result;
}

GL_API void GL_APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
   GLint temp[4];
   int count = glxx_get_texparameter_internal(target, pname, temp);

   if (count) {
      assert(count == 1 || count == 4);
      for(int i=0;i<count;i++)
         params[i] = (GLfloat)temp[i];
   }
}

GL_API void GL_APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
   glxx_get_texparameter_internal(target, pname, params);
}

static GLboolean is_hint(GLenum mode)
{
   return mode == GL_FASTEST ||
          mode == GL_NICEST ||
          mode == GL_DONT_CARE;
}

GL_API void GL_APIENTRY glHint(GLenum target, GLenum mode)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (is_hint(mode))
      switch (target) {
      case GL_PERSPECTIVE_CORRECTION_HINT:
         if(IS_GL_11(state)) {
            state->hints.perspective_correction = mode;
         } else {
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         }
         break;
      case GL_POINT_SMOOTH_HINT:
         if(IS_GL_11(state)) {
            state->hints.point_smooth = mode;
         } else {
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         }
         break;
      case GL_LINE_SMOOTH_HINT:
         if(IS_GL_11(state)) {
            state->hints.line_smooth = mode;
         } else {
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         }
         break;
      case GL_FOG_HINT:
         if(IS_GL_11(state)) {
            state->hints_program.fog = mode;
         } else {
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         }
         break;
      case GL_GENERATE_MIPMAP_HINT:
         state->hints.generate_mipmap = mode;
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_API GLboolean GL_APIENTRY glIsBuffer(GLuint buffer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return GL_FALSE;

   GLboolean result = glxx_shared_get_buffer(state->shared, buffer, false) != NULL;

   glxx_unlock_server_state(OPENGL_ES_ANY);

   return result;
}

GL_API GLboolean GL_APIENTRY glIsEnabled(GLenum cap)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return GL_FALSE;

   GLboolean result;

   switch (cap) {
   case GL_CULL_FACE:
   case GL_POLYGON_OFFSET_FILL:
   case GL_SAMPLE_ALPHA_TO_COVERAGE:
   case GL_SAMPLE_COVERAGE:
   case GL_SCISSOR_TEST:
   case GL_STENCIL_TEST:
   case GL_DEPTH_TEST:
   case GL_BLEND:
   case GL_DITHER:
   {
      GLuint count = glxx_get_boolean_internal(state, cap, &result);

      UNUSED_NDEBUG(count);
      assert(count == 1);
      break;
   }
   case GL_VERTEX_ARRAY:
   case GL_NORMAL_ARRAY:
   case GL_COLOR_ARRAY:
   case GL_POINT_SIZE_ARRAY_OES:
   case GL_TEXTURE_COORD_ARRAY:
   case GL_NORMALIZE:
   case GL_RESCALE_NORMAL:
   case GL_CLIP_PLANE0:
   case GL_FOG:
   case GL_LIGHTING:
   case GL_COLOR_MATERIAL:
   case GL_LIGHT0:
   case GL_LIGHT1:
   case GL_LIGHT2:
   case GL_LIGHT3:
   case GL_LIGHT4:
   case GL_LIGHT5:
   case GL_LIGHT6:
   case GL_LIGHT7:
   case GL_POINT_SMOOTH:
   case GL_POINT_SPRITE_OES:
   case GL_LINE_SMOOTH:
   case GL_MULTISAMPLE:
   case GL_SAMPLE_ALPHA_TO_ONE:
   case GL_TEXTURE_2D:
   case GL_ALPHA_TEST:
   case GL_COLOR_LOGIC_OP:
      if(IS_GL_11(state)) {
         GLuint count = glxx_get_boolean_internal(state, cap, &result);
         UNUSED_NDEBUG(count);
         assert(count == 1);
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = GL_FALSE;
      }
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      result = GL_FALSE;
      break;
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);

   return result;
}

/*
   glIsTexture()

   Returns TRUE if texture is the name of a texture object. Returns False if texture is zero, or
   is a nonzero value that is not the name of a texture object, or if an error condition occurs.
   A name returned by GenTextures, but not yet bound, is not the name of a texture object.

   Implementation: Done
   Error Checks: Done
*/

GL_API GLboolean GL_APIENTRY glIsTexture(GLuint texture)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return GL_FALSE;

   GLboolean result = glxx_shared_get_texture(state->shared, texture) != NULL;

   glxx_unlock_server_state(OPENGL_ES_ANY);

   return result;
}

static GLboolean is_readpixels_format(GLenum format)
{
   return format == GL_ALPHA ||
          format == GL_RGB ||
          format == GL_RGBA ||
          format == GL_BGRA_EXT;
}

static GLboolean is_readpixels_type(GLenum type)
{
   return type == GL_UNSIGNED_BYTE ||
          type == GL_UNSIGNED_SHORT_5_6_5 ||
          type == GL_UNSIGNED_SHORT_4_4_4_4 ||
          type == GL_UNSIGNED_SHORT_5_5_5_1;
}

static GLboolean texture_readpixels_format_type_match(GLenum format, GLenum type)
{
   return (format == GL_RGB && type == GL_UNSIGNED_BYTE) ||
          (format == GL_RGBA && type == GL_UNSIGNED_BYTE) ||
          (format == GL_BGRA_EXT && type == GL_UNSIGNED_BYTE);
}

GL_API void GL_APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if ((width < 0) || (height < 0))
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (!texture_readpixels_format_type_match(format, type))
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
   else if (!is_readpixels_format(format) || !is_readpixels_type(type))
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else if (glxx_check_framebuffer_status(state, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      glxx_server_state_set_error(state, GL_INVALID_FRAMEBUFFER_OPERATION);
   else
   {
      uint32_t a, n, s, k;
      char *cpixels;
      KHRN_IMAGE_WRAP_T src_wrap, dst_wrap;
      uint32_t dstx = 0, dsty = 0;

      KHRN_IMAGE_T *src = get_read_image(state);

      int fbwidth = src->width;
      int fbheight = src->height;

      // Copied from texture.c, get_pixel
      a = state->alignment.pack;
      n = (format == GL_RGBA) || (format == GL_BGRA_EXT) ? 4 : 3;
      s = 1;

      if (s < a)
         k = (a / s) * ((s * n * width + a - 1) / a);
      else
         k = n * width;

      khrn_interlock_read_immediate(&src->interlock);

      cpixels = (char*)pixels;

      if (x < 0) { dstx -= x; width += x;  x = 0; }
      if (y < 0) { dsty -= y; height += y; y = 0; }
      if (width > fbwidth   || x + width > fbwidth)   { width = fbwidth - x; }
      if (height > fbheight || y + height > fbheight) { height = fbheight - y; }

      khrn_image_lock_wrap(src, &src_wrap);

      if (!src->secure)
      {
         /* src image was generated by HW, flush to get good copy */
         khrn_hw_flush_dcache_range(src_wrap.storage, khrn_image_get_size(src_wrap.format, src_wrap.width, src_wrap.height));

         KHRN_IMAGE_FORMAT_T fmt;
#ifdef BIG_ENDIAN_CPU
         switch (format) {
         case GL_RGBA: fmt = RGBA_8888_RSO; break;
         case GL_RGB: fmt = RGB_888_RSO; break;
         case GL_BGRA_EXT: fmt = BGRA_8888_RSO; break;
         default:
            fmt = IMAGE_FORMAT_INVALID; break;
         }
#else
         switch (format) {
         case GL_RGBA: fmt = ABGR_8888_RSO; break;
         case GL_RGB: fmt = RGB_888_RSO; break;
         case GL_BGRA_EXT: fmt = ARGB_8888_RSO; break;
         default:
            fmt = IMAGE_FORMAT_INVALID; break;
         }
#endif
         khrn_image_wrap(&dst_wrap, fmt, width, height, k, 0, false, pixels);

         khrn_image_wrap_copy_region(
            &dst_wrap, dstx, dsty, width, height,
            &src_wrap, x, y, IMAGE_CONV_GL);
      }

      khrn_image_unlock_wrap(src);
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

void glxx_sample_coverage_internal (GLclampf value, GLboolean invert) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   state->changed_backend = true;
   state->sample_coverage.value = clampf(value, 0.0f, 1.0f);
   state->sample_coverage.invert = clean_boolean(invert);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_API void GL_APIENTRY glSampleCoverage(GLclampf value, GLboolean invert)
{
   glxx_sample_coverage_internal(value, invert);
}

GL_API void GL_APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (width >= 0 && height >= 0)
   {
      state->changed_viewport = true;
      state->scissor.x = x;
      state->scissor.y = y;
      state->scissor.width = width;
      state->scissor.height = height;
   } else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

static GLboolean is_face(GLenum face)
{
   return face == GL_FRONT ||
          face == GL_BACK ||
          face == GL_FRONT_AND_BACK;
}

void glxx_server_set_stencil_func(GLXX_SERVER_STATE_T *state, GLenum face, GLenum func, GLint ref, GLuint mask)
{
   if (is_face(face) && is_func(func)) {
      if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
         state->stencil_func.front.func = func;
         state->stencil_func.front.ref = ref;
         state->stencil_func.front.mask = mask;
      }

      if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
         state->stencil_func.back.func = func;
         state->stencil_func.back.ref = ref;
         state->stencil_func.back.mask = mask;
      }
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
}

GL_API void GL_APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   glxx_server_set_stencil_func(state, GL_FRONT_AND_BACK, func, ref, mask);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

static GLboolean is_op(GLXX_SERVER_STATE_T *state, GLenum op)
{
   return op == GL_KEEP ||
          op == GL_ZERO ||
          op == GL_REPLACE ||
          op == GL_INCR ||
          op == GL_DECR ||
          op == GL_INVERT ||
          (!IS_GL_11(state) && (
          op == GL_INCR_WRAP ||
          op == GL_DECR_WRAP
          ));
}

void glxx_server_set_stencil_mask(GLXX_SERVER_STATE_T *state, GLenum face, GLuint mask)
{
   if (is_face(face)) {
      if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
         state->stencil_mask.front = mask;

      if (face == GL_BACK || face == GL_FRONT_AND_BACK)
         state->stencil_mask.back = mask;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
}

GL_API void GL_APIENTRY glStencilMask(GLuint mask)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   glxx_server_set_stencil_mask(state, GL_FRONT_AND_BACK, mask);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

void glxx_server_set_stencil_op(GLXX_SERVER_STATE_T *state, GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
   if (is_face(face) && is_op(state, fail) && is_op(state, zfail) && is_op(state, zpass)) {
      if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
         state->stencil_op.front.fail = fail;
         state->stencil_op.front.zfail = zfail;
         state->stencil_op.front.zpass = zpass;
      }

      if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
         state->stencil_op.back.fail = fail;
         state->stencil_op.back.zfail = zfail;
         state->stencil_op.back.zpass = zpass;
      }
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
}

GL_API void GL_APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   glxx_server_set_stencil_op(state, GL_FRONT_AND_BACK, fail, zfail, zpass);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

static GLboolean is_texture_type(GLenum type)
{
   return (type == GL_UNSIGNED_BYTE) ||
      (type == GL_UNSIGNED_SHORT_4_4_4_4) ||
      (type == GL_UNSIGNED_SHORT_5_5_5_1) ||
      (type == GL_UNSIGNED_SHORT_5_6_5) ||
#if GL_APPLE_rgb_422
      (type == GL_UNSIGNED_SHORT_8_8_APPLE) ||
      (type == GL_UNSIGNED_SHORT_8_8_REV_APPLE) ||
#endif
      0;
}

static GLboolean texture_format_type_match(GLenum format, GLenum type)
{
   return (format == GL_RGBA && type == GL_UNSIGNED_BYTE) ||
          (format == GL_RGB && type == GL_UNSIGNED_BYTE) ||
          (format == GL_RGBA && type == GL_UNSIGNED_SHORT_4_4_4_4) ||
          (format == GL_RGBA && type == GL_UNSIGNED_SHORT_5_5_5_1) ||
          (format == GL_RGB && type == GL_UNSIGNED_SHORT_5_6_5) ||
          (format == GL_LUMINANCE_ALPHA && type == GL_UNSIGNED_BYTE) ||
          (format == GL_LUMINANCE && type == GL_UNSIGNED_BYTE) ||
          (format == GL_ALPHA && type == GL_UNSIGNED_BYTE) ||
#if GL_EXT_texture_format_BGRA8888
          (format == GL_BGRA_EXT && type == GL_UNSIGNED_BYTE) ||
#endif
#if GL_APPLE_rgb_422
          (format == GL_RGB_422_APPLE && type == GL_UNSIGNED_SHORT_8_8_APPLE) ||
          (format == GL_RGB_422_APPLE && type == GL_UNSIGNED_SHORT_8_8_REV_APPLE) ||
#endif
          0;
}

GL_API void GL_APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLenum error = GL_NONE;
   GLXX_TEXTURE_T *texture = NULL;

   switch (target) {
   case GL_TEXTURE_EXTERNAL_OES:
       error = GL_INVALID_ENUM;
       goto end;
   case GL_TEXTURE_2D:
       break;
   default:
       if (IS_GL_11(state)) {
           error = GL_INVALID_VALUE;
           goto end;
       }
       break;
   }

   texture = glxx_server_state_get_texture(state, target, GL_TRUE);
   state->changed_texunit = true; /* GLES1.1 - may change has_color/has_alpha */

   if (!texture) goto end;

   if (level < 0 || level > LOG2_MAX_TEXTURE_SIZE)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (width < 0 || width > MAX_TEXTURE_SIZE || height < 0 || height > MAX_TEXTURE_SIZE)
      error = GL_INVALID_VALUE;
   else if (IS_GL_11(state) && target != GL_TEXTURE_2D)
      error = GL_INVALID_ENUM;
   else if (!IS_GL_11(state) && target != GL_TEXTURE_2D && width != height)
      error = GL_INVALID_VALUE;
   else if (border != 0)
      error = GL_INVALID_VALUE;
   else if (!is_texture_internal_format(internalformat))
      error = GL_INVALID_VALUE;
   else if (!is_texture_internal_format(format))
      error = GL_INVALID_ENUM;
   else if (!is_texture_type(type))
      error = GL_INVALID_ENUM;
   else if (internalformat != (GLint)format)
      error = GL_INVALID_OPERATION;
   else if (!texture_format_type_match(format, type))
      error = GL_INVALID_OPERATION;
#if GL_APPLE_rgb_422
   else if ((format == GL_RGB_422_APPLE) && ((width & 1) != 0))
      error = GL_INVALID_OPERATION;
#endif
   else if (!glxx_texture_image(texture, target, level, width, height, format, type, state->alignment.unpack, pixels, state->secure))
      error = GL_OUT_OF_MEMORY;

end:
   if (error != GL_NONE)
       glxx_server_state_set_error(state, error);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

static GLboolean is_mag_filter(int i)
{
   return i == GL_NEAREST ||
          i == GL_LINEAR;
}

static GLboolean is_min_filter(GLenum target, int i)
{
   switch (i) {
   case GL_NEAREST_MIPMAP_NEAREST:
   case GL_NEAREST_MIPMAP_LINEAR:
   case GL_LINEAR_MIPMAP_NEAREST:
   case GL_LINEAR_MIPMAP_LINEAR:
      if (target == GL_TEXTURE_EXTERNAL_OES)
         return false;
      /* drop through */
   case GL_NEAREST:
   case GL_LINEAR:
      return true;
   }
   return false;
}

static GLboolean is_wrap(GLXX_SERVER_STATE_T *state, GLenum target, int i)
{
    switch (i) {
    case GL_MIRRORED_REPEAT:
        if (!IS_GL_11(state) && target != GL_TEXTURE_EXTERNAL_OES)
            return true;
        break;
    case GL_REPEAT:
        if (target != GL_TEXTURE_EXTERNAL_OES)
            return true;
        break;
    case GL_CLAMP_TO_EDGE:
        return true;
    }
    return false;
}

void glxx_texparameter_internal(GLenum target, GLenum pname, const GLint *i)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_TEXTURE_T *texture = glxx_server_state_get_texture(state, target, GL_FALSE);      // performs valid target check

   if (texture) {
      switch (pname) {
      case GL_TEXTURE_MIN_FILTER:
         if (is_min_filter(target, *i))
            texture->min = *i;
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      case GL_TEXTURE_MAG_FILTER:
         if (is_mag_filter(*i))
            texture->mag = *i;
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      case GL_TEXTURE_WRAP_S:
         if (is_wrap(state, target, *i))
            texture->wrap.s = *i;
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      case GL_TEXTURE_WRAP_T:
         if (is_wrap(state, target, *i))
            texture->wrap.t = *i;
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      case GL_GENERATE_MIPMAP:
         if(IS_GL_11(state)) {
            if (*i)
               texture->generate_mipmap = GL_TRUE;
            else
               texture->generate_mipmap = GL_FALSE;
         } else {
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         }
         break;
      case GL_TEXTURE_CROP_RECT_OES:
         if (IS_GL_11(state)) {
            glxx_texture_set_crop_rect(texture, i);
         } else {
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         }
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_API void GL_APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
   GLint iparams[4];
   iparams[0] = (GLint)param;

   if (pname == GL_TEXTURE_CROP_RECT_OES) {        /* If we need 4 params, fill in the rest. */
      iparams[1] = iparams[2] = iparams[3] = 0;    /* TODO: Is this the right answer         */
   }

   glxx_texparameter_internal(target, pname, iparams);
}

GL_API void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
   GLint iparams[4];
   iparams[0] = param;

   if (pname == GL_TEXTURE_CROP_RECT_OES) {
      iparams[1] = iparams[2] = iparams[3] = 0;
   }

   glxx_texparameter_internal(target, pname, iparams);
}

GL_API void GL_APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
   if (params)
   {
      GLint iparams[4];
      iparams[0] = (GLint)params[0];
      if (pname == GL_TEXTURE_CROP_RECT_OES) {
         for (int i = 1; i < 4; i++)
            iparams[i] = (GLint)params[i];
      }
      glxx_texparameter_internal(target, pname, iparams);
   }
}

GL_API void GL_APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
   if (params)
      glxx_texparameter_internal(target, pname, params);
}

GL_API void GL_APIENTRY glTexParameterx(GLenum target, GLenum pname, GLfixed param)
{
   GLint iparams[4];
   iparams[0] = (GLint)param;                /* no scaling for enum to fixed */

   if (pname == GL_TEXTURE_CROP_RECT_OES)
      iparams[1] = iparams[2] = iparams[3] = 0;

   glxx_texparameter_internal(target, pname, iparams);
}

GL_API void GL_APIENTRY glTexParameterxv(GLenum target, GLenum pname, const GLfixed *params)
{
   if (params)
   {
      GLint iparams[4];
      iparams[0] = (GLint)params[0];         /* no scaling for enum to fixed */

      if (pname == GL_TEXTURE_CROP_RECT_OES) {
         for (int i = 1; i < 4; i++)
            iparams[i] = (GLint)params[i];   /* no scaling for enum to fixed */
      }

      glxx_texparameter_internal(target, pname, iparams);
   }
}

GL_API void GL_APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_TEXTURE_T *texture = NULL;

   GLenum error = GL_NONE;

   if (target == GL_TEXTURE_EXTERNAL_OES) {
      error = GL_INVALID_ENUM;
      goto end;
   }

   texture = glxx_server_state_get_texture(state, target, GL_TRUE);

   if (!texture) goto end;

   if (!glxx_is_texture_target(state, target))
      error = GL_INVALID_ENUM;
   else if (width < 0 || width > MAX_TEXTURE_SIZE || height < 0 || height > MAX_TEXTURE_SIZE)
      error = GL_INVALID_VALUE;
   else if (!glxx_texture_includes(texture, target, level, xoffset, yoffset))
      error = GL_INVALID_VALUE;
   else if (!is_texture_internal_format(format))
      error = GL_INVALID_ENUM;
   else if (!is_texture_type(type))
      error = GL_INVALID_ENUM;
   else if (!texture_format_type_match(format, type))
      error = GL_INVALID_OPERATION;
   else if (!glxx_texture_includes(texture, target, level, xoffset + width - 1, yoffset + height - 1))
      error = GL_INVALID_VALUE;
   else if (!khrn_image_is_uncomp(glxx_texture_incomplete_get_mipmap_format(texture, target, level)))
      error = GL_INVALID_OPERATION;
#if GL_APPLE_rgb_422
   else if ((format == GL_RGB_422_APPLE) && ((width & 1) != 0 || (xoffset & 1) != 0))
      error = GL_INVALID_OPERATION;
#endif
   else if (width > 0 && height > 0)
   {
      if (!glxx_texture_sub_image(texture, target, level, xoffset, yoffset, width, height, format, type, state->alignment.unpack, pixels))
         error = GL_OUT_OF_MEMORY;
   }

end:
   if (error != GL_NONE)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

static int glxx_find_max(GLXX_BUFFER_T *indices, GLsizei count, GLenum type, uint32_t indices_offset)
{
   int result = -1;

   if (indices != NULL) {
      MEM_HANDLE_T indices_handle = buffer_handle(indices);
      uint32_t indices_size;

      const uint8_t *indices_pointer = mem_lock(indices_handle, NULL);
      indices_pointer += indices_offset;

      if (indices_offset < mem_get_size(indices_handle))
         indices_size = mem_get_size(indices_handle) - indices_offset;
      else
         indices_size = 0;

      switch (type) {
      case GL_UNSIGNED_BYTE:
         count = _min(count, indices_size);
         result = find_max(count, 1, indices_pointer);
         break;
      case GL_UNSIGNED_SHORT:
         count = _min(count, indices_size / 2);
         result = find_max(count, 2, indices_pointer);
         break;
      default:
         UNREACHABLE();
         break;
      }

      mem_unlock(indices_handle);
   }

   return result;
}

void get_core_revision_internal(GLsizei bufSize, GLubyte *revisionStr)
{
   BEGL_HWInfo info;

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   driverInterfaces->hwInterface->GetInfo(driverInterfaces->hwInterface->context, &info);

   strncpy((char *)revisionStr, info.name, bufSize - 1);
   revisionStr[bufSize - 1] = '\0';
}

GL_APICALL void GL_APIENTRY glDiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (target != GL_FRAMEBUFFER)
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else if (numAttachments < 0 || attachments == NULL)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else
   {
      GLsizei i;
      bool    color = false;
      bool    ds = false;
      bool    err = false;

      if (valid_frame_buffer(state))
      {
         for (i = 0; i < numAttachments; i++)
         {
            switch (attachments[i])
            {
            case GL_COLOR_ATTACHMENT0:
               color = true;
               break;
            case GL_DEPTH_ATTACHMENT:
               ds = true;
               break;
            case GL_STENCIL_ATTACHMENT:
               ds = true;
               break;
            default:
               glxx_server_state_set_error(state, GL_INVALID_ENUM);
               err = true;
               break;
            }
         }
      }
      else
      {
         for (i = 0; i < numAttachments; i++)
         {
            switch (attachments[i])
            {
            case GL_COLOR_EXT:
               color = true;
               break;
            case GL_DEPTH_EXT:
               ds = true;
               break;
            case GL_STENCIL_EXT:
               ds = true;
               break;
            default:
               glxx_server_state_set_error(state, GL_INVALID_ENUM);
               err = true;
               break;
            }
         }
      }

      if (!err)
         glxx_hw_invalidate_frame(state, color, ds, ds, color, false);
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

/* OES_framebuffer_object for ES 1.1 and core in ES 2.0 */

/* check 1.1 constants match their 2.0 equivalents */
vcos_static_assert(GL_FRAMEBUFFER == GL_FRAMEBUFFER_OES);
vcos_static_assert(GL_INVALID_FRAMEBUFFER_OPERATION == GL_INVALID_FRAMEBUFFER_OPERATION_OES);
vcos_static_assert(GL_FRAMEBUFFER_BINDING == GL_FRAMEBUFFER_BINDING_OES);
vcos_static_assert(GL_RENDERBUFFER_BINDING == GL_RENDERBUFFER_BINDING_OES);
vcos_static_assert(GL_MAX_RENDERBUFFER_SIZE == GL_MAX_RENDERBUFFER_SIZE_OES);
vcos_static_assert(GL_COLOR_ATTACHMENT0 == GL_COLOR_ATTACHMENT0_OES);
vcos_static_assert(GL_DEPTH_ATTACHMENT == GL_DEPTH_ATTACHMENT_OES);
vcos_static_assert(GL_STENCIL_ATTACHMENT == GL_STENCIL_ATTACHMENT_OES);
vcos_static_assert(GL_RENDERBUFFER == GL_RENDERBUFFER_OES);
vcos_static_assert(GL_DEPTH_COMPONENT16 == GL_DEPTH_COMPONENT16_OES);
vcos_static_assert(GL_RGBA4 == GL_RGBA4_OES);
vcos_static_assert(GL_RGB5_A1 == GL_RGB5_A1_OES);
vcos_static_assert(GL_RGB565 == GL_RGB565_OES);
vcos_static_assert(GL_STENCIL_INDEX8 == GL_STENCIL_INDEX8_OES);
vcos_static_assert(GL_RENDERBUFFER_WIDTH == GL_RENDERBUFFER_WIDTH_OES);
vcos_static_assert(GL_RENDERBUFFER_HEIGHT == GL_RENDERBUFFER_HEIGHT_OES);
vcos_static_assert(GL_RENDERBUFFER_INTERNAL_FORMAT == GL_RENDERBUFFER_INTERNAL_FORMAT_OES);
vcos_static_assert(GL_RENDERBUFFER_RED_SIZE == GL_RENDERBUFFER_RED_SIZE_OES);
vcos_static_assert(GL_RENDERBUFFER_GREEN_SIZE == GL_RENDERBUFFER_GREEN_SIZE_OES);
vcos_static_assert(GL_RENDERBUFFER_BLUE_SIZE == GL_RENDERBUFFER_BLUE_SIZE_OES);
vcos_static_assert(GL_RENDERBUFFER_ALPHA_SIZE == GL_RENDERBUFFER_ALPHA_SIZE_OES);
vcos_static_assert(GL_RENDERBUFFER_DEPTH_SIZE == GL_RENDERBUFFER_DEPTH_SIZE_OES);
vcos_static_assert(GL_RENDERBUFFER_STENCIL_SIZE == GL_RENDERBUFFER_STENCIL_SIZE_OES);
vcos_static_assert(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE == GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_OES);
vcos_static_assert(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME == GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES);
vcos_static_assert(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL == GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_OES);
vcos_static_assert(GL_NONE == GL_NONE_OES);
vcos_static_assert(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES);
vcos_static_assert(GL_FRAMEBUFFER_UNSUPPORTED == GL_FRAMEBUFFER_UNSUPPORTED_OES);
vcos_static_assert(GL_FRAMEBUFFER_UNSUPPORTED == GL_FRAMEBUFFER_UNSUPPORTED_OES);
vcos_static_assert(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES);
vcos_static_assert(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES);


static bool is_valid_attachment(GLenum attachment)
{
   return attachment == GL_COLOR_ATTACHMENT0 ||
      attachment == GL_DEPTH_ATTACHMENT ||
      attachment == GL_STENCIL_ATTACHMENT;
}

/*
   glIsRenderbuffer()

   [Not in spec, behaviour defined to be analogous to texture]

   Returns TRUE if renderbuffer is the name of a renderbuffer object. Returns FALSE if renderbuffer
   is zero, or is a nonzero value that is not the name of a renderbuffer object, or if an error
   condition occurs. A name returned by GenRenderbuffers, but not yet bound, is not the name of
   a renderbuffer object.

   Implementation: Done
   Error Checks: Done
*/

GLboolean is_renderbuffer_internal(GLuint renderbuffer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return GL_FALSE;

   GLboolean result = glxx_shared_get_renderbuffer(state->shared, renderbuffer, false) != NULL;

   glxx_unlock_server_state(OPENGL_ES_ANY);

   return result;
}

GL_APICALL GLboolean GL_APIENTRY glIsRenderbuffer(GLuint renderbuffer)
{
   return is_renderbuffer_internal(renderbuffer);
}

/*
   glBindRenderbuffer()

   Called with target set to RENDERBUFFER and renderbuffer set to the unused name. If renderbuffer is not zero,
   then the resulting renderbuffer object is a new state vector, initialized with a zero-sized memory buffer, and
   comprising the state values listed in table 6.32. Any previous binding to target is broken.

   BindRenderbuffer may also be used to bind an existing renderbuffer object. If the bind is successful, no
   change is made to the state of the newly bound renderbuffer object, and any previous binding to target is
   broken.

   Implementation: Done
   Error Checks: Done
*/

void bind_renderbuffer_internal(GLenum target, GLuint renderbuffer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (target == GL_RENDERBUFFER) {
      GLXX_RENDERBUFFER_T *glxx_renderbuffer = NULL;

      if (renderbuffer) {
         glxx_renderbuffer = glxx_shared_get_renderbuffer(state->shared, renderbuffer, true);

         if (glxx_renderbuffer == NULL) {
            glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

            glxx_unlock_server_state(OPENGL_ES_ANY);
            return;
         }
      }

      KHRN_MEM_ASSIGN(state->bound_renderbuffer, glxx_renderbuffer);
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
   bind_renderbuffer_internal(target, renderbuffer);
}

static void detach_renderbuffer(GLXX_FRAMEBUFFER_T *framebuffer, void *p)
{
   if (framebuffer->attachments.color.object == p)
   {
      framebuffer->attachments.color.type = GL_NONE;
      framebuffer->attachments.color.target = 0;
      framebuffer->attachments.color.level = 0;
      KHRN_MEM_ASSIGN(framebuffer->attachments.color.object, NULL);
   }

   if (framebuffer->attachments.depth.object == p)
   {
      framebuffer->attachments.depth.type = GL_NONE;
      framebuffer->attachments.depth.target = 0;
      framebuffer->attachments.depth.level = 0;
      KHRN_MEM_ASSIGN(framebuffer->attachments.depth.object, NULL);
   }

   if (framebuffer->attachments.stencil.object == p)
   {
      framebuffer->attachments.stencil.type = GL_NONE;
      framebuffer->attachments.stencil.target = 0;
      framebuffer->attachments.stencil.level = 0;
      KHRN_MEM_ASSIGN(framebuffer->attachments.stencil.object, NULL);
   }
}

void delete_renderbuffers_internal(GLsizei n, const GLuint *renderbuffers)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_SHARED_T *shared = state->shared;

   if (n < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);   // The conformance tests insist...
   else if (renderbuffers)
   {
      int32_t i;
      for (i = 0; i < n; i++)
      {
         if (renderbuffers[i])
         {
            GLXX_RENDERBUFFER_T *glxx_renderbuffer = glxx_shared_get_renderbuffer(shared, renderbuffers[i], false);

            if (glxx_renderbuffer != NULL)
            {
               if (state->bound_renderbuffer == glxx_renderbuffer)
                  KHRN_MEM_ASSIGN(state->bound_renderbuffer, NULL);

               if (state->bound_framebuffer != NULL)
               {
                  GLXX_FRAMEBUFFER_T *framebuffer = state->bound_framebuffer;
                  if (framebuffer)
                     detach_renderbuffer(framebuffer, glxx_renderbuffer);
               }

               glxx_shared_delete_renderbuffer(shared, renderbuffers[i]);
            }
         }
      }
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
   delete_renderbuffers_internal(n, renderbuffers);
}

void gen_renderbuffers_internal(GLsizei n, GLuint *renderbuffers)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_SHARED_T *shared = state->shared;

   int32_t i = 0;

   if (n < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);   // The conformance tests insist...
   else if (renderbuffers)
      while (i < n) {
         if (glxx_shared_get_renderbuffer(shared, shared->next_renderbuffer, false) == NULL)
            renderbuffers[i++] = shared->next_renderbuffer;

         shared->next_renderbuffer++;
      }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
   gen_renderbuffers_internal(n, renderbuffers);
}

static GLboolean is_internal_format(GLenum format)
{
   return format == GL_RGBA4 ||
          format == GL_RGB5_A1 ||
          format == GL_RGB565 ||
          format == GL_RGB8_OES ||
          format == GL_RGBA8_OES ||
          format == GL_DEPTH_COMPONENT16 ||
          format == GL_DEPTH_COMPONENT24_OES ||
          format == GL_STENCIL_INDEX8 ||
          format == GL_DEPTH_COMPONENT32_OES ||
          format == GL_DEPTH24_STENCIL8_OES
/* TODO confirm whether these values are permissable (defined in 2.0 headers but not in core spec as allowed formats)
             format == GL_DEPTH_COMPONENT ||
             format == GL_RGB ||
             format == GL_RGBA ||
             format == GL_STENCIL_INDEX
*/
          ;
}

void renderbuffer_storage_multisample_internal(GLenum target, GLsizei samples,
   GLenum internalformat, GLsizei width, GLsizei height)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   /* Promote 16 bit depth attachments to 24 bit so that they play nice with stencil 8 */
   if (internalformat == GL_DEPTH_COMPONENT16)
      internalformat = GL_DEPTH_COMPONENT24_OES;

   if (target != GL_RENDERBUFFER)
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else if (state->bound_renderbuffer == NULL)
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
   else if (!is_internal_format(internalformat))
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else if (width < 0 || width > GLXX_CONFIG_MAX_RENDERBUFFER_SIZE ||
            height < 0 || height > GLXX_CONFIG_MAX_RENDERBUFFER_SIZE ||
            samples < 0 || samples > GLXX_CONFIG_SAMPLES)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else {
      GLXX_RENDERBUFFER_T *bound_renderbuffer = state->bound_renderbuffer;
      if (!glxx_renderbuffer_storage_multisample(bound_renderbuffer, samples, internalformat, width, height, state->secure))
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
   renderbuffer_storage_multisample_internal(target, 0, internalformat, width, height);
}

/*
   GetRenderBufferParameteriv

   RENDERBUFFER WIDTH 0 GetRenderbufferParameteriv
   RENDERBUFFER HEIGHT 0 GetRenderbufferParameteriv
   RENDERBUFFER INTERNAL FORMAT RGBA GetRenderbufferParameteriv
   RENDERBUFFER RED SIZE 0 GetRenderbufferParameteriv
   RENDERBUFFER GREEN SIZE 0 GetRenderbufferParameteriv
   RENDERBUFFER BLUE SIZE 0 GetRenderbufferParameteriv
   RENDERBUFFER ALPHA SIZE 0 GetRenderbufferParameteriv
   RENDERBUFFER DEPTH SIZE 0 GetRenderbufferParameteriv
   RENDERBUFFER STENCIL SIZE 0 GetRenderbufferParameteriv
   RENDERBUFFER SAMPLES EXT 0 GetRenderbufferParameteriv

   here we assume that the allowable internal renderbuffer
   formats are

      GL_DEPTH_COMPONENT
      GL_RGB
      GL_RGBA
      GL_RGBA4
      GL_RGB5_A1
      GL_RGB565
      GL_DEPTH_COMPONENT16
      GL_STENCIL_INDEX
      GL_STENCIL_INDEX8
*/

void get_renderbuffer_parameteriv_internal(GLenum target, GLenum pname, GLint* params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (target == GL_RENDERBUFFER) {
      if (state->bound_renderbuffer != NULL) {
         GLXX_RENDERBUFFER_T *renderbuffer = state->bound_renderbuffer;

         KHRN_IMAGE_T *storage = renderbuffer->storage;

         switch (pname) {
         case GL_RENDERBUFFER_WIDTH:
            params[0] = storage ? storage->width : 0;
            break;
         case GL_RENDERBUFFER_HEIGHT:
            params[0] = storage ? storage->height : 0;
            break;
         case GL_RENDERBUFFER_INTERNAL_FORMAT:
             if (!storage)
                params[0] = GL_RGBA4;  //The initial value is GL_RGBA4.
             else
             {
                KHRN_IMAGE_FORMAT_T format = khrn_image_no_colorspace_format(storage->format);
                format = khrn_image_no_layout_format(format);
                switch (format) {
                case ABGR_8888:
                   params[0] = GL_RGBA8_OES;/* TODO confirm this is the correct constant to return  */
                   break;
                case XBGR_8888:
                   params[0] = GL_RGB8_OES;/* TODO confirm this is the correct constant to return  */
                   break;
                case RGBA_4444:
                   params[0] = GL_RGBA4;
                   break;
                case RGBA_5551:
                   params[0] = GL_RGB5_A1;
                   break;
                case RGB_565:
                   params[0] = GL_RGB565;
                   break;
                case DEPTH_32:
                   params[0] = GL_DEPTH_COMPONENT24_OES;/* TODO confirm this is the correct constant to return  */
                   break;
                case DEPTH_16:
                   params[0] = GL_DEPTH_COMPONENT16;
                   break;
                default:
                   UNREACHABLE();
                   break;
                }
             }
             break;
         case GL_RENDERBUFFER_RED_SIZE:
            params[0] = storage ? khrn_image_get_red_size(storage->format) : 0;
            break;
         case GL_RENDERBUFFER_GREEN_SIZE:
            params[0] = storage ? khrn_image_get_green_size(storage->format) : 0;
            break;
         case GL_RENDERBUFFER_BLUE_SIZE:
            params[0] = storage ? khrn_image_get_blue_size(storage->format) : 0;
            break;
         case GL_RENDERBUFFER_ALPHA_SIZE:
            params[0] = storage ? khrn_image_get_alpha_size(storage->format) : 0;
            break;
         case GL_RENDERBUFFER_DEPTH_SIZE:
            params[0] = storage ? khrn_image_get_z_size(storage->format) : 0;
            break;
         case GL_RENDERBUFFER_STENCIL_SIZE:
            params[0] = storage ? khrn_image_get_stencil_size(storage->format) : 0;
            break;
         case GL_RENDERBUFFER_SAMPLES_EXT:
            params[0] = renderbuffer->samples;
            break;
         default:
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
            break;
         }
      } else {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      }
   } else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
   get_renderbuffer_parameteriv_internal(target, pname, params);
}

GLboolean is_framebuffer_internal(GLuint framebuffer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return GL_FALSE;

   GLboolean result = glxx_shared_get_framebuffer(state->shared, framebuffer, false) != NULL;

   glxx_unlock_server_state(OPENGL_ES_ANY);

   return result;
}

GL_APICALL GLboolean GL_APIENTRY glIsFramebuffer(GLuint framebuffer)
{
   return is_framebuffer_internal(framebuffer);
}

void bind_framebuffer_internal(GLenum target, GLuint framebuffer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (target == GL_FRAMEBUFFER) {
      GLXX_FRAMEBUFFER_T *glxx_framebuffer = NULL;

      if (framebuffer) {
         glxx_framebuffer = glxx_shared_get_framebuffer(state->shared, framebuffer, true);

         if (glxx_framebuffer == NULL) {
            glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

            glxx_unlock_server_state(OPENGL_ES_ANY);
            return;
         }
      }

      KHRN_MEM_ASSIGN(state->bound_framebuffer, glxx_framebuffer);
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glBindFramebuffer(GLenum target, GLuint framebuffer)
{
   bind_framebuffer_internal(target, framebuffer);
}

void delete_framebuffers_internal(GLsizei n, const GLuint *framebuffers)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_SHARED_T *shared = state->shared;

   if (n < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);   // The conformance tests insist...
   else if (framebuffers) {
      int i;
      for (i = 0; i < n; i++)
         if (framebuffers[i]) {
            GLXX_FRAMEBUFFER_T *glxx_framebuffer = glxx_shared_get_framebuffer(shared, framebuffers[i], false);

            if (glxx_framebuffer != NULL) {
               if (state->bound_framebuffer == glxx_framebuffer)
                  KHRN_MEM_ASSIGN(state->bound_framebuffer, NULL);

               glxx_shared_delete_framebuffer(shared, framebuffers[i]);
            }
         }
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
   delete_framebuffers_internal(n, framebuffers);
}

void gen_framebuffers_internal(GLsizei n, GLuint *framebuffers)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_SHARED_T *shared = state->shared;

   int32_t i = 0;

   if (n < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);   // The conformance tests insist...
   else if (framebuffers)
      while (i < n) {
         if (glxx_shared_get_framebuffer(shared, shared->next_framebuffer, false) == NULL)
            framebuffers[i++] = shared->next_framebuffer;

         shared->next_framebuffer++;
      }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glGenFramebuffers(GLsizei n, GLuint *framebuffers)
{
   gen_framebuffers_internal(n, framebuffers);
}

GLenum check_framebuffer_status_internal(GLenum target)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return GL_NONE;

   GLenum result = glxx_check_framebuffer_status(state, target);

   glxx_unlock_server_state(OPENGL_ES_ANY);
   return result;
}

GL_APICALL GLenum GL_APIENTRY glCheckFramebufferStatus(GLenum target)
{
   return check_framebuffer_status_internal(target);
}

void framebuffer_texture2D_multisample_internal(GLenum target, GLenum a,
   GLenum textarget, GLuint texture, GLint level, GLsizei samples, bool only_attachment0)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (only_attachment0 && a != GL_COLOR_ATTACHMENT0) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (target != GL_FRAMEBUFFER || !is_valid_attachment(a)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (state->bound_framebuffer == NULL) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   GLXX_ATTACHMENT_INFO_T *attachment;
   GLXX_FRAMEBUFFER_T *framebuffer = state->bound_framebuffer;

   assert(framebuffer);

   switch (a) {
   case GL_COLOR_ATTACHMENT0:
      attachment = &framebuffer->attachments.color;
      break;
   case GL_DEPTH_ATTACHMENT:
      attachment = &framebuffer->attachments.depth;
      break;
   case GL_STENCIL_ATTACHMENT:
      attachment = &framebuffer->attachments.stencil;
      break;
   default:
      attachment = NULL;
      UNREACHABLE();
      break;
   }

   if (texture) {
      if (level != 0)
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
      else {
         if (!glxx_is_texture_target(state, textarget))
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         else if (IS_GL_11(state) && GL_TEXTURE_2D != textarget)
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         else if (samples < 0 || samples > GLXX_CONFIG_SAMPLES)
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
         else {
            GLXX_TEXTURE_T *glxx_texture = glxx_shared_get_texture(state->shared, texture);

            if (glxx_texture == NULL)
               glxx_server_state_set_error(state, GL_INVALID_OPERATION);
            else {
               if ((glxx_texture->target == GL_TEXTURE_CUBE_MAP) != (textarget != GL_TEXTURE_2D))
                  glxx_server_state_set_error(state, GL_INVALID_OPERATION);
               else {
                  attachment->type = GL_TEXTURE;
                  attachment->target = textarget;
                  attachment->level = level;

                  /* Clamp the number of samples to 0 (non-multisample) or 4 (multisample)
                  1 is considerated as multisample in the spec EXT_framebuffer_multisample, revision #7, Issue (2)
                  (Written based on the wording of the OpenGL 1.5 specification.) */
                  attachment->samples = samples ? GLXX_CONFIG_SAMPLES : 0;

                  if (samples) {
                     KHRN_IMAGE_T *ms_image = glxx_image_create_ms(COL_32_TLBD,
                        glxx_texture->width, glxx_texture->height,
                        IMAGE_CREATE_FLAG_RENDER_TARGET | IMAGE_CREATE_FLAG_ONE, state->secure);

                     if (ms_image != NULL) {
                        KHRN_MEM_ASSIGN(glxx_texture->ms_image, ms_image);
                        KHRN_MEM_ASSIGN(ms_image, NULL);
                        KHRN_MEM_ASSIGN(attachment->object, glxx_texture);
                     }
                     else
                        glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
                  }
                  else
                     KHRN_MEM_ASSIGN(attachment->object, glxx_texture);
               }
            }
         }
      }
   } else {
      attachment->type = GL_NONE;
      attachment->target = 0;
      attachment->level = 0;
      attachment->samples = 0;

      KHRN_MEM_ASSIGN(attachment->object, NULL);
   }

end:

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
   framebuffer_texture2D_multisample_internal(target, attachment, textarget, texture, level, 0, false);
}

void framebuffer_renderbuffer_internal(GLenum target, GLenum a, GLenum renderbuffertarget, GLuint renderbuffer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (target == GL_FRAMEBUFFER && is_valid_attachment(a)) {
      if (state->bound_framebuffer != NULL) {
         GLXX_ATTACHMENT_INFO_T *attachment;
         GLXX_FRAMEBUFFER_T *framebuffer = state->bound_framebuffer;

         assert(framebuffer);

         switch (a) {
         case GL_COLOR_ATTACHMENT0:
            attachment = &framebuffer->attachments.color;
            break;
         case GL_DEPTH_ATTACHMENT:
            attachment = &framebuffer->attachments.depth;
            break;
         case GL_STENCIL_ATTACHMENT:
            attachment = &framebuffer->attachments.stencil;
            break;
         default:
            attachment = NULL;
           UNREACHABLE();
            break;
         }

         assert(attachment);

         if (renderbuffer) {
            if (renderbuffertarget == GL_RENDERBUFFER) {
               GLXX_RENDERBUFFER_T *glxx_renderbuffer = glxx_shared_get_renderbuffer(state->shared, renderbuffer, false);

               if (glxx_renderbuffer == NULL)
                  glxx_server_state_set_error(state, GL_INVALID_OPERATION);
               else {
                  attachment->type = GL_RENDERBUFFER;
                  attachment->target = 0;
                  attachment->level = 0;
                  attachment->samples = glxx_renderbuffer->samples;

                  KHRN_MEM_ASSIGN(attachment->object, glxx_renderbuffer);
               }
            } else
               glxx_server_state_set_error(state, GL_INVALID_ENUM);
         } else {
            attachment->type = GL_NONE;
            attachment->target = 0;
            attachment->level = 0;

            KHRN_MEM_ASSIGN(attachment->object, NULL);
         }
      } else
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
   framebuffer_renderbuffer_internal(target, attachment, renderbuffertarget, renderbuffer);
}

/*
   GetFramebufferParameteriv

   FRAMEBUFFER OBJECT TYPE NONE GetFramebufferParameteriv
   FRAMEBUFFER OBJECT NAME 0 GetFramebufferParameteriv
   FRAMEBUFFER TEXTURE LEVEL 0 GetFramebufferParameteriv
   FRAMEBUFFER TEXTURE CUBE MAP FACE +ve X face GetFramebufferParameteriv
*/

void get_framebuffer_attachment_parameteriv_internal(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (target == GL_FRAMEBUFFER && is_valid_attachment(attachment)) {
      if (state->bound_framebuffer != NULL) {
         GLXX_FRAMEBUFFER_T *framebuffer = state->bound_framebuffer;

         GLXX_ATTACHMENT_INFO_T *attachmentinfo;

         switch (attachment) {
         case GL_COLOR_ATTACHMENT0:
            attachmentinfo = &framebuffer->attachments.color;
            break;
         case GL_DEPTH_ATTACHMENT:
            attachmentinfo = &framebuffer->attachments.depth;
            break;
         case GL_STENCIL_ATTACHMENT:
            attachmentinfo = &framebuffer->attachments.stencil;
            break;
         default:
            attachmentinfo = 0;
         }

         if (attachmentinfo) {
            switch (attachmentinfo->type) {
            case GL_RENDERBUFFER:
               switch (pname) {
               case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
                  params[0] = GL_RENDERBUFFER;
                  break;
               case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
               {
                  GLXX_RENDERBUFFER_T *renderbuffer = attachmentinfo->object;
                  params[0] = renderbuffer->name;
                  break;
               }
               case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT:
               {
                  GLXX_RENDERBUFFER_T *renderbuffer = attachmentinfo->object;
                  params[0] = renderbuffer->samples;
                  break;
               }
               case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
               case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
               default:
                  glxx_server_state_set_error(state, GL_INVALID_ENUM);
               }
               break;
            case GL_TEXTURE:
               switch (pname) {
               case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
                  params[0] = GL_TEXTURE;
                  break;
               case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
               {
                  GLXX_TEXTURE_T *texture = attachmentinfo->object;
                  params[0] = texture->name;
                  break;
               }
               case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
                  params[0] = attachmentinfo->level;
                  break;
               case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
                  if(IS_GL_11(state)) {
                     glxx_server_state_set_error(state, GL_INVALID_ENUM);
                  }
                  else {
                  //TODO: is this right? We return integers rather than enums
                     switch (attachmentinfo->target) {
                     case GL_TEXTURE_2D:
                        params[0] = 0;
                        break;
                     case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
                     case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
                     case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
                     case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
                     case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
                     case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                        params[0] = attachmentinfo->target;
                        break;
                     default:
                        //Unreachable
                       UNREACHABLE();
                     }
                  }
                  break;
               case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT:
                  {
                     params[0] = attachmentinfo->samples;
                     break;
                  }
               default:
                  glxx_server_state_set_error(state, GL_INVALID_ENUM);
               }
               break;
            case GL_NONE:
               switch (pname) {
               case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
                  params[0] = GL_NONE;
                  break;
               case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
               case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
               case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
               default:
                  glxx_server_state_set_error(state, GL_INVALID_ENUM);
               }
               break;
            default:
               // Unreachable
              UNREACHABLE();
            }
         } else {
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         }
      } else {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      }
   } else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
   get_framebuffer_attachment_parameteriv_internal(target, attachment, pname, params);
}

void generate_mipmap_internal(GLenum target)
{
   GLXX_TEXTURE_T *texture;
   bool invalid_operation;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   texture = glxx_server_state_get_texture(state, target, GL_FALSE);

   if (texture) {
      switch (target) {
      case GL_TEXTURE_2D:
         if (!glxx_texture_generate_mipmap(texture, TEXTURE_BUFFER_TWOD, &invalid_operation))
            glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         else if (invalid_operation)
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         break;
      case GL_TEXTURE_CUBE_MAP:
         if(IS_GL_11(state)) {
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         }
         else if (glxx_texture_is_cube_complete(texture)) {
            int i;
            for (i = TEXTURE_BUFFER_POSITIVE_X; i <= TEXTURE_BUFFER_NEGATIVE_Z; i++) {
               if (!glxx_texture_generate_mipmap(texture, i, &invalid_operation))
                  glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
               else if (invalid_operation)
                  glxx_server_state_set_error(state, GL_INVALID_OPERATION);
            }
         } else
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         break;
      case GL_TEXTURE_EXTERNAL_OES:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      default:
         UNREACHABLE();
         break;
      }
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_APICALL void GL_APIENTRY glGenerateMipmap(GLenum target)
{
   generate_mipmap_internal(target);
}

void glxx_RenderbufferStorageMultisampleEXT_impl(GLenum target, GLsizei samples,
   GLenum internalformat, GLsizei width, GLsizei height, bool secure)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   /* Promote 16 bit depth attachments to 24 bit so that they play nice with stencil 8 */
   if (internalformat == GL_DEPTH_COMPONENT16)
      internalformat = GL_DEPTH_COMPONENT24_OES;

   if (target != GL_RENDERBUFFER)
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
   else if (state->bound_renderbuffer == NULL)
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
   else if (!is_internal_format(internalformat))
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else if (width < 0 || width > GLXX_CONFIG_MAX_RENDERBUFFER_SIZE || height < 0 || height > GLXX_CONFIG_MAX_RENDERBUFFER_SIZE || samples > GLXX_CONFIG_SAMPLES)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else {
      GLXX_RENDERBUFFER_T *bound_renderbuffer = state->bound_renderbuffer;
      if (!glxx_renderbuffer_storage_multisample(bound_renderbuffer, samples, internalformat, width, height, secure))
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

static GLboolean is_alignment(GLint param)
{
   return param == 1 ||
      param == 2 ||
      param == 4 ||
      param == 8;
}

GL_API void GL_APIENTRY glPixelStorei(GLenum pname, GLint param)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if ((pname == GL_PACK_ALIGNMENT || pname == GL_UNPACK_ALIGNMENT) && (!is_alignment(param)))
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (param < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else {
      switch (pname) {
      case GL_PACK_ALIGNMENT:
         state->alignment.pack = param;
         break;
      case GL_UNPACK_ALIGNMENT:
         state->alignment.unpack = param;
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   }

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

static int calc_length(int max, int size, GLenum type, int stride)
{
   if (max >= 0) {
      int type_size = khrn_get_type_size((int)type);

      return size * type_size + max * (stride ? stride : size * type_size);
   }
   else
      return 0;
}

typedef struct MERGE_INFO {
   bool send;

   const char *start;
   const char *end;
   GLXX_BUFFER_T *buffer;
   int offset;

   struct MERGE_INFO *next;
} MERGE_INFO_T;

static GLXX_BUFFER_T *create_scratch_buffer(const void *data, size_t size);

static int merge_attribs(GLXX_SERVER_STATE_T *state, int max)
{
   bool merged = false;

   MERGE_INFO_T merge[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++) {
      merge[i].send = state->attrib[i].enabled && state->attrib[i].buffer == 0;
      merge[i].start = state->attrib[i].pointer;
      merge[i].end = (const char *)state->attrib[i].pointer + calc_length(max, state->attrib[i].size, state->attrib[i].type, state->attrib[i].stride);
      merge[i].buffer = NULL;
      merge[i].offset = 0;
      merge[i].next = NULL;
   }

   for (int i = 1; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      if (merge[i].send)
         for (int j = 0; j < i; j++)
            if (merge[j].send && !merge[j].next) {
               const char *start = merge[i].start < merge[j].start ? merge[i].start : merge[j].start;
               const char *end = merge[i].end > merge[j].end ? merge[i].end : merge[j].end;

               if ((uint32_t)(end - start) < (uint32_t)((merge[i].end - merge[i].start) + (merge[j].end - merge[j].start))) {
                  MERGE_INFO_T *curr;

                  if (merge[i].start < merge[j].start) {
                     curr = &merge[i];
                     while (curr->next)
                        curr = curr->next;
                     curr->end = end;

                     merge[j].offset = merge[j].start - merge[i].start;
                     merge[j].next = &merge[i];
                  }
                  else {
                     curr = &merge[j];
                     while (curr->next)
                        curr = curr->next;
                     curr->end = end;

                     merge[i].offset = merge[i].start - merge[j].start;
                     merge[i].next = &merge[j];
                  }
               }
            }

   /* create merged scratch buffers */
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      if (merge[i].send && !merge[i].next) {
         assert(merge[i].offset == 0);
         merge[i].buffer = create_scratch_buffer(merge[i].start,
               merge[i].end - merge[i].start);
         if (merge[i].buffer == NULL)
            goto end; /* out of memory */
      }

   /* calculate offsets and assign buffers to merged siblings */
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++) {
      MERGE_INFO_T *curr;

      for (curr = merge[i].next; curr; curr = curr->next)
      {
         KHRN_MEM_ASSIGN(merge[i].buffer, curr->buffer);
         merge[i].offset += curr->offset;
      }

      merge[i].next = NULL;
   }

   /* assign merged buffers and calculated offsets to attributes */
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      if (merge[i].send) {
         assert(merge[i].buffer != NULL);
         KHRN_MEM_ASSIGN(state->attrib[i].attrib, merge[i].buffer);
         state->attrib[i].offset = merge[i].offset;
      }
      else
         assert(merge[i].buffer == NULL);

   merged = true;

end:
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      KHRN_MEM_ASSIGN(merge[i].buffer, NULL);
   return merged;
}

static GLXX_BUFFER_T *create_scratch_buffer(const void *data, size_t size)
{
   GLXX_BUFFER_T *buffer;

   buffer = glxx_shared_create_buffer(0);

   if (buffer != NULL)
   {
      bool copied = glxx_buffer_data(buffer, size, data, GL_STATIC_DRAW, /*transient=*/true);
      if (!copied)
         KHRN_MEM_ASSIGN(buffer, NULL);
   }
   return buffer;
}

bool glxx_is_aligned(GLenum type, size_t value)
{
   switch (type) {
   case GL_BYTE:
   case GL_UNSIGNED_BYTE:
      return true;
   case GL_SHORT:
   case GL_UNSIGNED_SHORT:
      return (value & 1) == 0;
   case GL_FIXED:
   case GL_FLOAT:
      return (value & 3) == 0;
   default:
      UNREACHABLE();
      return false;
   }
}

static void drawarrays_or_elements(GLenum mode, GLint first, GLsizei count, bool check_type, GLenum type, const GLvoid *indices)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_BUFFER_T *indices_buffer = NULL;

   if (count <= 0)
   {
      if (count<0)
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (check_type && !is_index_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

#if (__mips__) || (__arm__ && __ARM_FEATURE_UNALIGNED != 1)
   if (check_type && !glxx_is_aligned(type, (size_t)indices)) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }
#endif

   bool send_any = false;
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++) {
      send_any |= state->attrib[i].enabled && state->attrib[i].pointer != NULL && state->attrib[i].buffer == 0;

      /* TODO: what should we do if people give us null pointers? */
      if (state->attrib[i].enabled && state->attrib[i].pointer == NULL && state->attrib[i].buffer == 0)
         goto end;
   }

   uint32_t indices_offset;
   bool send_indices;
   int max;

   if (type == 0) {
      indices_offset = first;
      send_indices = false;
      max = first + count - 1;
      indices_buffer = NULL;
   }
   else {
      if (state->bound_buffer.element_array_name != 0 &&
            state->bound_buffer.element_array_buffer == NULL)
      {
         /* inconsistent state */
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         goto end;
      }

      send_indices = state->bound_buffer.element_array_name == 0;

      if (send_indices)
      {
         /* copy client-side indices into a scratch buffer */
         size_t indices_length = count * khrn_get_type_size((int)type);
         indices_buffer = create_scratch_buffer(indices, indices_length);
         if (indices_buffer == NULL)
         {
            glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
            goto end;
         }
         indices_offset = 0;
      }
      else
      {
         /* use bound indices buffer */
         KHRN_MEM_ASSIGN(indices_buffer, state->bound_buffer.element_array_buffer);
         indices_offset = (uint32_t)(uintptr_t)indices;
      }
      max = glxx_find_max(indices_buffer, count, type, indices_offset);
   }

   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      if (state->attrib[i].enabled && state->attrib[i].buffer != 0 &&
            state->attrib[i].attrib == NULL)
      {
         /*
            If a glBindBuffer call has failed due to an out of memory condition, the client's estimate
            of which attributes have buffers bound can differ from reality. If this happens, we cause
            subsequent glDrawArrays() and glDrawElements() calls to fail with GL_OUT_OF_MEMORY.
         */
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         goto end;
      }
   }

   if (merge_attribs(state, max))
      draw_elements(state, mode, count, type, indices_buffer, indices_offset);
   else
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

end:
   /* release a temp refence to scratch buffer containing indices */
   KHRN_MEM_ASSIGN(indices_buffer, NULL);

   /* release temp refences to scratch buffers containing attributes */
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      if (state->attrib[i].enabled && state->attrib[i].buffer == 0)
         KHRN_MEM_ASSIGN(state->attrib[i].attrib, NULL);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_API void GL_APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   drawarrays_or_elements(mode, first, count, false, 0, NULL);
}

GL_API void GL_APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
   drawarrays_or_elements(mode, 0, count, true, type, indices);
}

GL_API void GL_APIENTRY glGetPointerv(GLenum pname, GLvoid **params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (IS_GL_20(state)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   switch (pname) {
   case GL_VERTEX_ARRAY_POINTER:
      params[0] = (void *)state->attrib[GL11_IX_VERTEX].pointer;
      break;
   case GL_NORMAL_ARRAY_POINTER:
      params[0] = (void *)state->attrib[GL11_IX_NORMAL].pointer;
      break;
   case GL_COLOR_ARRAY_POINTER:
      params[0] = (void *)state->attrib[GL11_IX_COLOR].pointer;
      break;
   case GL_TEXTURE_COORD_ARRAY_POINTER:
      params[0] = (void *)state->attrib[state->client_active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].pointer;
      break;
   case GL_POINT_SIZE_ARRAY_POINTER_OES:
      params[0] = (void *)state->attrib[GL11_IX_POINT_SIZE].pointer;
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

end:
   glxx_unlock_server_state(OPENGL_ES_ANY);
}

static void point_size(GLfloat size) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   size = clean_float(size);

   if (size > 0.0f) {
      if (IS_GL_11(state))
         state->attrib[GL11_IX_POINT_SIZE].value[0] = size;
      else
         state->point_size = size;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state(OPENGL_ES_ANY);
}

GL_API void GL_APIENTRY glPointSize(GLfloat size)
{
   point_size(size);
}

GL_API void GL_APIENTRY glPointSizex(GLfixed size)
{
   point_size(fixed_to_float(size));
}

void glxx_context_gl_create_lock(void)
{
   if (vcos_mutex_create(&gl_lock, "GLXX") != VCOS_SUCCESS)
      exit(EXIT_FAILURE);
}

void glxx_context_gl_lock(void)
{
   vcos_mutex_lock(&gl_lock);
}

void glxx_context_gl_unlock(void)
{
   vcos_mutex_unlock(&gl_lock);
}

GLXX_SERVER_STATE_T *glxx_lock_server_state(EGL_CONTEXT_TYPE_T type)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   if (thread->opengl.context == NULL)
      return NULL;

   glxx_context_gl_lock();

   EGL_CONTEXT_T *context = thread->opengl.context;

   vcos_demand(context->state);  // 1->1 mapping for EGL_CONTEXT_T to GLXX_SERVER_STATE_T

   GLXX_SERVER_STATE_T *state = context->state;

   if (!glxx_check_gl_api(state, type)) {
      glxx_context_gl_unlock();
      state = NULL;
   }

   return state;
}

void glxx_unlock_server_state(EGL_CONTEXT_TYPE_T type)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   if (thread->opengl.context == NULL)
      return;

   assert(vcos_mutex_is_locked(&gl_lock));

   EGL_CONTEXT_T *context = thread->opengl.context;

   vcos_demand(context->state);  // 1->1 mapping for EGL_CONTEXT_T to GLXX_SERVER_STATE_T

   GLXX_SERVER_STATE_T *state = context->state;

   if (!glxx_check_gl_api(state, type))
      assert(0);

   glxx_context_gl_unlock();
}
