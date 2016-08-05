/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
BCM2708 implementation of hardware abstraction layer.
Functions common to OpenGL ES 1.1 and OpenGL ES 2.0
=============================================================================*/

#include "../glxx/gl_public_api.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_translate.h"
#include "gl11_draw.h"

#include <limits.h>

#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_cl.h"

/*************************************************************
 Global Functions
 *************************************************************/

bool gl11_cache_uniforms(GLXX_SERVER_STATE_T *state, KHRN_FMEM_T *fmem)
{
   GL11_STATE_T *s = &state->gl11;
   bool have_lights    = !!(s->shaderkey.vertex & GL11_LIGHTING);
   bool matrix_palette = !!(s->shaderkey.vertex & GL11_MPAL_M);
   bool user_clip      = !!(s->shaderkey.fragment & GL11_UCLIP_M);

   if (!matrix_palette)
   {
      if (state->gl11.changed.projection_modelview)
      {
         gl11_matrix_mult(s->projection_modelview, s->current_projection, s->current_modelview);
         s->changed.projection_modelview = false;
      }

      /* Normals must be transformed by inverse modelview */
      if (have_lights && s->changed.modelview_inv)
      {
         gl11_matrix_invert_3x3(s->modelview_inv, s->current_modelview);
         s->changed.modelview_inv = false;
      }
   }
   else
   {
      /* Snapshot the matrix palette so install_uniforms doesn't have to do it more than once */
      float *matrices = (float *)khrn_fmem_data(fmem, sizeof(s->palette_matrices), 4);
      if (!matrices) return false;

      khrn_memcpy(matrices, s->palette_matrices, sizeof(s->palette_matrices));
      s->palette_matrices_base_ptr = khrn_fmem_hw_address(fmem, matrices);
      /* Don't transform normals, it will be done on the QPU once the final matrix is known */
   }

   if (user_clip)
   {
      float inv[16];
      int i;
      float u_dot_p;
      float u[3] = { (float)state->viewport.x + (float)state->viewport.width / 2.0f,
                     (float)state->viewport.y + (float)state->viewport.height / 2.0f,
                     state->viewport.internal_zoffset };

      gl11_matrix_invert_4x4(inv, s->current_projection);
      gl11_matrix_mult_row(s->projected_clip_plane, s->planes[0], inv);
      s->projected_clip_plane[0] /= ((float)state->viewport.width / 2.0f);   /* xscale */
      s->projected_clip_plane[1] /= ((float)state->viewport.height / 2.0f);  /* yscale */
      s->projected_clip_plane[2] /= state->viewport.internal_zscale;         /* zscale */

      u_dot_p = 0;
      for (i=0; i<3; i++) u_dot_p += u[i] * s->projected_clip_plane[i];

      s->projected_clip_plane[3] -= u_dot_p;
   }

   s->line_width = state->line_width;

   return true;
}

bool gl11_is_points(GLXX_PRIMITIVE_T draw_mode) {
   return (draw_mode == GL_POINTS);
}

bool gl11_is_lines(GLXX_PRIMITIVE_T draw_mode) {
   switch (draw_mode) {
   case GL_LINES:
   case GL_LINE_STRIP:
   case GL_LINE_LOOP:   return true;

   default:             return false;
   }
}

uint32_t gl11_compute_vertex(GL11_STATE_T *state,
                             GLXX_PRIMITIVE_T draw_mode)
{
   uint32_t vertex;

   vertex = state->statebits.vertex & state->statebits.v_enable & state->statebits.v_enable2;

   /* Two-sided lighting is only for triangles */
   if (gl11_is_points(draw_mode) || gl11_is_lines(draw_mode)) vertex &= ~GL11_TWOSIDE;

   return vertex;
}


uint32_t gl11_compute_fragment(GL11_STATE_T *state,
                               GLXX_PRIMITIVE_T draw_mode,
                               const GLXX_HW_FRAMEBUFFER_T *fb)
{
   uint32_t fragment;
   uint32_t gl11_sample_mask = fb->ms ? ~0u : ~GL11_SAMPLE_ONE;

   fragment = state->statebits.fragment & state->statebits.f_enable & gl11_sample_mask;

   if (state->shade_model == GL_FLAT) fragment |= GL11_FLATSHADE;

   /* Ignore antialiasing if point sprites are enabled */
   if (gl11_is_points(draw_mode) && !state->point_sprite)
      fragment |= (state->statebits.fragment & GL11_POINTSMOOTH);

   if (gl11_is_lines(draw_mode))
      fragment |= (state->statebits.fragment & GL11_LINESMOOTH);

   return fragment;
}

void gl11_compute_texture_key(GLXX_SERVER_STATE_T *state, bool points) {
   int i;
   bool tex_enabled[GL11_CONFIG_MAX_TEXTURE_UNITS];
   bool tex_color  [GL11_CONFIG_MAX_TEXTURE_UNITS];
   bool tex_alpha  [GL11_CONFIG_MAX_TEXTURE_UNITS];

   for (i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      GLXX_TEXTURE_T *texture = NULL;
      tex_enabled[i] = false;
      tex_color  [i] = false;
      tex_alpha  [i] = false;

      /* In order actually to be enabled the texunit must be enabled */
      {
         bool enabled = state->gl11.texunits[i].target_enabled_EXTERNAL_OES ||
                        state->gl11.texunits[i].target_enabled_2D;

         if (!enabled) continue;
      }

      /* There must be a texture bound to it */
      {
         enum glxx_tex_target tex_target;
         tex_target = state->gl11.texunits[i].target_enabled_2D ? GL_TEXTURE_2D :
                                                                  GL_TEXTURE_EXTERNAL_OES;
         texture = glxx_textures_get_texture(&state->bound_texture[i], tex_target);
         assert(state->bound_sampler[i] == NULL);      /* Not in ES1 */

         if (texture == NULL) continue;
      }

      /* and the texture bound there must be complete */
      {
         unsigned junk0, junk1;
         bool test_base_complete = false;

         if (texture->sampler.filter.min == GL_NEAREST || texture->sampler.filter.min == GL_LINEAR)
            test_base_complete = true;

         if (!glxx_texture_check_completeness(texture, test_base_complete, &junk0, &junk1))
            continue;
      }

      tex_enabled[i] = true;
      {
         bool ok;
         ok = glxx_texture_es1_has_color_alpha(texture, &tex_color[i], &tex_alpha[i]);
         assert(ok);
         vcos_unused_in_release(ok);
      }
   }

   for (i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (tex_enabled[i])
      {
         GLXX_VAO_T *vao = state->vao.bound;
         GL11_TEXUNIT_T *texunit = &state->gl11.texunits[i];
         GLenum mode = texunit->mode;
         bool coord_replace;
         uint32_t texbits;

         coord_replace = points && state->gl11.point_sprite;

         /* Coord replace is meaningless except for points */
         texbits = state->gl11.statebits.texture[i];
         if (!coord_replace) texbits &= ~GL11_TEX_COORDREPLACE;

         if (mode != GL_COMBINE)
            SET_MASKED(texbits, lookup_tex_mode(mode, tex_color[i], tex_alpha[i]), GL11_TEX_COMBINE_M);

         if (vao->attrib_config[GL11_IX_TEXTURE_COORD+i].size == 4 ||
             gl11_matrix_is_projective(texunit->current_matrix)  )
         {
            texbits |= GL11_TEX_COMPLEX;
         }

         state->gl11.shaderkey.texture[i] = texbits;
      }
      else state->gl11.shaderkey.texture[i] = 0;
   }
}
