/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/gl11/gl11_server.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_server_internal.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"

/*
   Khronos Documentation:
   http://www.khronos.org/registry/gles/extensions/OES/OES_draw_texture.txt
   Overview

   This extension defines a mechanism for writing pixel
    rectangles from one or more textures to a rectangular
    region of the screen.  This capability is useful for
    fast rendering of background paintings, bitmapped font
    glyphs, and 2D framing elements in games.  This
    extension is primarily intended for use with OpenGL ES.

    The extension relies on a new piece of texture state
    called the texture crop rectangle, which defines a
    rectangular subregion of a texture object.  These
    subregions are used as sources of pixels for the texture
    drawing function.

    Applications use this extension by configuring the
    texture crop rectangle for one or more textures via
    ActiveTexture() and TexParameteriv() with pname equal to
    TEXTURE_CROP_RECT_OES.  They then request a drawing
    operation using DrawTex{sifx}[v]OES().  The effect of
    the latter function is to generate a screen-aligned
    target rectangle, with texture coordinates chosen to map
    the texture crop rectangle(s) linearly to fragments in
    the target rectangle.  The fragments are then processed
    in accordance with the fragment pipeline state.

   Texture Rectangle Drawing

    OpenGL supports drawing sub-regions of a texture to
    rectangular regions of the screen using the texturing
    pipeline.  Source region size and content are determined
    by the texture crop rectangle(s) of the enabled
    texture(s) (see section 3.8.14).

    The functions

    void DrawTex{sifx}OES(T Xs, T Ys, T Zs, T Ws, T Hs);
    void DrawTex{sifx}vOES(T *coords);

    draw a texture rectangle to the screen.  Xs, Ys, and Zs
    specify the position of the affected screen rectangle.
    Xs and Ys are given directly in window (viewport)
    coordinates.  Zs is mapped to window depth Zw as follows:

                 { n,                 if z <= 0
            Zw = { f,                 if z >= 1
                 { n + z * (f - n),   otherwise

    where <n> and <f> are the near and far values of
    DEPTH_RANGE.  Ws and Hs specify the width and height of
    the affected screen rectangle in pixels.  These values
    may be positive or negative; however, if either (Ws <=
    0) or (Hs <= 0), the INVALID_VALUE error is generated.

    Calling one of the DrawTex functions generates a
    fragment for each pixel that overlaps the screen
    rectangle bounded by (Xs, Ys) and (Xs + Ws), (Ys + Hs).
    For each generated fragment, the depth is given by Zw
    as defined above, and the color by the current color.

    If EXT_fog_coord is supported, and FOG_COORDINATE_SOURCE_EXT
    is set to FOG_COORINATE_EXT, then the fragment distance for
    fog purposes is set to CURRENT_FOG_COORDINATE. Otherwise,
    the fragment distance for fog purposes is set to 0.

    Texture coordinates for each texture unit are computed
    as follows:

    Let X and Y be the screen x and y coordinates of each
    sample point associated with the fragment.  Let Wt and
    Ht be the width and height in texels of the texture
    currently bound to the texture unit.  (If the texture is
    a mipmap, let Wt and Ht be the dimensions of the level
    specified by TEXTURE_BASE_LEVEL.)  Let Ucr, Vcr, Wcr and
    Hcr be (respectively) the four integers that make up the
    texture crop rectangle parameter for the currently bound
    texture.  The fragment texture coordinates (s, t, r, q)
    are given by

    s = (Ucr + (X - Xs)*(Wcr/Ws)) / Wt
    t = (Vcr + (Y - Ys)*(Hcr/Hs)) / Ht
    r = 0
    q = 1

    In the specific case where X, Y, Xs and Ys are all
    integers, Wcr/Ws and Hcr/Hs are both equal to one, the
    base level is used for the texture read, and fragments
    are sampled at pixel centers, implementations are
    required to ensure that the resulting u, v texture
    indices are also integers.  This results in a one-to-one
    mapping of texels to fragments.

    Note that Wcr and/or Hcr can be negative.  The formulas
    given above for s and t still apply in this case.  The
    result is that if Wcr is negative, the source rectangle
    for DrawTex operations lies to the left of the reference
    point (Ucr, Vcr) rather than to the right of it, and
    appears right-to-left reversed on the screen after a
    call to DrawTex.  Similarly, if Hcr is negative, the
    source rectangle lies below the reference point (Ucr,
    Vcr) rather than above it, and appears upside-down on
    the screen.

    Note also that s, t, r, and q are computed for each
    fragment as part of DrawTex rendering.  This implies
    that the texture matrix is ignored and has no effect on
    the rendered result.


   Implementation Notes:

*/

static void try_slow_path(GLXX_SERVER_STATE_T *state,
   GLfloat Xs, GLfloat Ys, GLfloat Zw, GLfloat Ws, GLfloat Hs)
{
   bool tex_ok = false;

   /* check textures are acceptable by V3D HW */
   /* all enabled textures are tformat or linear tile or 32bit, pot, 4k aligned raster */

   for (uint32_t i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      GL11_TEXUNIT_T *texunit = &state->texunits[i];
      GLXX_TEXTURE_T *texture = state->bound_texture[i].twod;

      if (texunit->target_enabled && texture != NULL) {
         KHRN_IMAGE_FORMAT_T format = texture->format;

         if (ABGR_8888_RSO == texture->format &&
             texture->width == texture->height &&
             texture->width == (uint32_t)(1 << _msb(texture->width)) &&
             COMPLETE == glxx_texture_check_complete(texture))/* pot */
         {
            unsigned buf = texture->target == GL_TEXTURE_2D ?
                 TEXTURE_BUFFER_TWOD : TEXTURE_BUFFER_EXTERNAL;
            KHRN_IMAGE_T *image = texture->mipmaps[buf][0];
            if (image != NULL)
            {
               MEM_HANDLE_T hstorage = image->mh_storage;
               if (hstorage != MEM_HANDLE_INVALID)
               {
                  void *pixels = mem_lock(hstorage, NULL);
                  if(!((uintptr_t)pixels & 0xfff))/* 4k aligned */
                     tex_ok = true;

                  mem_unlock(hstorage);
               }
            }
         }
         else if (khrn_image_is_tformat(texture->format) || khrn_image_is_lineartile(texture->format))
            tex_ok = true;

         if (!tex_ok) /*found an enabled but not supported texture */
         {
            if (texture->format == ABGR_8888_RSO || texture->format == BGR_888_RSO)
            {
               /* call texture_image with null pixel pointer - should convert mipmaps back into tformat blob */
               tex_ok = glxx_texture_image(texture, GL_TEXTURE_2D, 0, texture->width, texture->height,
                  texture->format == ABGR_8888_RSO ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, 4, 0, false);
            }
         }
      }
   }

   if (tex_ok)
      glxx_hw_draw_tex(state, Xs, Ys, Zw, Ws, Hs);
}

static void draw_texfOES(GLfloat Xs, GLfloat Ys, GLfloat Zs, GLfloat Ws, GLfloat Hs)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if(Ws <=0.0f || Hs <= 0.0f)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else
   {
      GLfloat Zw;
      GLclampf n, f;
      n = state->viewport.near;
      f = state->viewport.far;
      Zw = Zs <=0 ? n : Zs>=1 ? f : n + Zs * (f - n);

      try_slow_path(state, Xs, Ys, Zw, Ws, Hs);
   }

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glDrawTexsOES(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
   draw_texfOES((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)width, (GLfloat)height);
}

GL_API void GL_APIENTRY glDrawTexiOES(GLint x, GLint y, GLint z, GLint width, GLint height)
{
   draw_texfOES((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)width, (GLfloat)height);
}

GL_API void GL_APIENTRY glDrawTexxOES(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
   draw_texfOES(fixed_to_float(x), fixed_to_float(y), fixed_to_float(z), fixed_to_float(width), fixed_to_float(height));
}

GL_API void GL_APIENTRY glDrawTexsvOES(const GLshort *coords)
{
   draw_texfOES((GLfloat)coords[0], (GLfloat)coords[1], (GLfloat)coords[2], (GLfloat)coords[3], (GLfloat)coords[4]);
}

GL_API void GL_APIENTRY glDrawTexivOES(const GLint *coords)
{
   draw_texfOES((GLfloat)coords[0], (GLfloat)coords[1], (GLfloat)coords[2], (GLfloat)coords[3], (GLfloat)coords[4]);
}

GL_API void GL_APIENTRY glDrawTexxvOES(const GLfixed *coords)
{
   draw_texfOES(fixed_to_float(coords[0]), fixed_to_float(coords[1]), fixed_to_float(coords[2]), fixed_to_float(coords[3]), fixed_to_float(coords[4]));
}

GL_API void GL_APIENTRY glDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height)
{
   draw_texfOES(x, y, z, width, height);
}

GL_API void GL_APIENTRY glDrawTexfvOES(const GLfloat *coords)
{
   draw_texfOES((GLfloat)coords[0], (GLfloat)coords[1], (GLfloat)coords[2], (GLfloat)coords[3], (GLfloat)coords[4]);
}
