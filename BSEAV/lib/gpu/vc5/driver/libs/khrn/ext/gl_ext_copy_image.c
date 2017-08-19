/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../glxx/gl_public_api.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_server_texture.h"

enum operand_type
{
   operand_type_renderbuffer,
   operand_type_texture
};

struct operand
{
   enum operand_type type;
   union
   {
      GLXX_RENDERBUFFER_T *renderbuffer;
      GLXX_TEXTURE_T *texture;
   };
};

struct selector
{
   unsigned elem; /* array index of an array type texture */
   unsigned face; /* face of cubemap or cubamep array */
   unsigned depth; /* z coordinate of 3D texture */
};

static GLenum get_operand(GLXX_SERVER_STATE_T *state, GLuint name,
      GLenum target, struct operand *operand)
{
   if (target ==  GL_TEXTURE_BUFFER)
      return GL_INVALID_ENUM;

   if (glxx_texture_is_tex_target(state, target))
   {
      operand->type = operand_type_texture;
      operand->texture = khrn_map_lookup(&state->shared->textures, name);
      if (!operand->texture)
         return GL_INVALID_VALUE;

      if (target != operand->texture->target)
         return GL_INVALID_ENUM;

      return GL_NO_ERROR;
   }

   if (target == GL_RENDERBUFFER)
   {
      operand->type = operand_type_renderbuffer;
      operand->renderbuffer = khrn_map_lookup(&state->shared->renderbuffers, name);
      return operand->renderbuffer ? GL_NO_ERROR : GL_INVALID_VALUE;
   }

   return GL_INVALID_ENUM;
}

static khrn_image* get_image(const struct operand *operand, unsigned face,
      unsigned level)
{
   khrn_image* image;

   switch (operand->type)
   {
   case operand_type_renderbuffer:
      assert(operand->renderbuffer != NULL);
      assert(face == 0);
      assert(level == 0);
      image = operand->renderbuffer->image;
      break;
   case operand_type_texture:
      assert(operand->texture != NULL);
      image = operand->texture->img[face][level];
      break;
   default:
      unreachable();
      image = NULL;
   }
   assert(!image || image->level == level);
   return image;
}

static GFX_LFMT_T get_format(const struct operand *operand)
{
   /* assumption: all levels/faces have the same format */
   const khrn_image *img = get_image(operand, 0, 0);
   return img ? khrn_image_get_lfmt(img, 0) : GFX_LFMT_NONE;
}

/* Instead of implementing the whole concept of view classes this is a simple
 * list of checks that follows the same principles and returns the same values
 * for matching or not matching formats as tables in the GLES 3.2 spec:
 *
 * - no depth / stencil / yuv as they're not included in the tables
 * - match pixel/block size for all formats
 * - additionally match block size if both src and dst are compressed
 */
static bool format_valid(GFX_LFMT_T lfmt)
{
   return !( /* any of the below is NOT valid */
         lfmt == GFX_LFMT_NONE ||
         gfx_lfmt_has_depth(lfmt) ||
         gfx_lfmt_has_stencil(lfmt) ||
         gfx_lfmt_has_depth_stencil(lfmt) ||
         gfx_lfmt_has_y(lfmt) ||
         gfx_lfmt_has_u(lfmt) ||
         gfx_lfmt_has_v(lfmt));
}

static bool formats_match(const struct operand *src, const struct operand *dst)
{
   bool match;
   GFX_LFMT_T src_format = get_format(src);
   GFX_LFMT_T dst_format = get_format(dst);

   /* check if formats meet the minimum criteria and have the same bit depth */
   if (  format_valid(src_format) &&
         format_valid(dst_format) &&
         gfx_lfmt_bytes_per_block(src_format) ==
               gfx_lfmt_bytes_per_block(dst_format))
   {
      /* if both formats are compressed also check block sizes */
      if (gfx_lfmt_is_compressed(src_format) &&
            gfx_lfmt_is_compressed(dst_format))
      {
         GFX_LFMT_BASE_DETAIL_T src_bd, dst_bd;
         gfx_lfmt_base_detail(&src_bd, src_format);
         gfx_lfmt_base_detail(&dst_bd, dst_format);
         match =
               src_bd.block_w == dst_bd.block_w &&
               src_bd.block_h == dst_bd.block_h &&
               src_bd.block_d == dst_bd.block_d;
      }
      else
      {
         /* matching bpp is sufficient for uncompressed or mixed formats */
         match = true;
      }
   }
   else
   {
      match = false;
   }
   return match;
}

static bool is_multisample(const struct operand *operand)
{
   switch (operand->type)
   {
   case operand_type_texture:
      return glxx_tex_target_is_multisample(operand->texture->target);
   case operand_type_renderbuffer:
      return operand->renderbuffer->ms_mode != GLXX_NO_MS;
   default:
      unreachable();
      return false;
   }
}

static bool samples_match(const struct operand *src, const struct operand *dst)
{
   return is_multisample(src) == is_multisample(dst);
}

static void tex_image_selector(struct selector *selector,
      enum glxx_tex_target target, unsigned z)
{
   switch (target)
   {
   case GL_TEXTURE_2D_ARRAY:
      selector->elem = z;
      selector->face = 0;
      selector->depth = 0;
      break;
   case GL_TEXTURE_CUBE_MAP:
      selector->elem = 0;
      selector->face = z;
      selector->depth = 0;
      break;
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      selector->elem = z / MAX_FACES;
      selector->face = z % MAX_FACES;
      selector->depth = 0;
      break;
   default:
      selector->elem = 0;
      selector->face = 0;
      selector->depth = z;
      break;
   }
}

static void image_selector(struct selector *selector,
      const struct operand *operand, unsigned z)
{
   switch (operand->type)
   {
   case operand_type_renderbuffer:
      assert(z == 0);
      selector->elem = 0;
      selector->face = 0;
      selector->depth = 0;
      break;
   case operand_type_texture:
      assert(operand->texture != NULL);
      tex_image_selector(selector, operand->texture->target, z);
      break;
   default:
      unreachable();
      break;
   }
}

static inline bool valid_start(unsigned start, unsigned size, unsigned align)
{
   return start < size && start % align == 0;
}

static inline bool valid_end(unsigned end, unsigned size, unsigned align)
{
   return end == size || (end < size && end % align == 0);
}

static inline bool valid_region(const khrn_image *img,
      unsigned x, unsigned y, unsigned z, unsigned e,
      unsigned w, unsigned h, unsigned d, unsigned de)
{
   unsigned img_w, img_h, img_d, img_e;
   khrn_image_get_dimensions(img, &img_w, &img_h, &img_d, &img_e);

   if (e > img_e || e + de > img_e)
      return false;

   GFX_LFMT_T lfmts[GFX_BUFFER_MAX_PLANES];
   unsigned num_planes;
   khrn_image_get_lfmts(img, lfmts, &num_planes);

   for (unsigned plane = 0; plane < num_planes; plane++)
   {
      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, lfmts[plane]);
      if (  !valid_start(x, img_w, bd.block_w) ||
            !valid_start(y, img_h, bd.block_h) ||
            !valid_start(z, img_d, bd.block_d) ||
            !valid_end(x + w, img_w, bd.block_w) ||
            !valid_end(y + h, img_h, bd.block_h) ||
            !valid_end(z + d, img_d, bd.block_d))
         return false;
   }
   return true;
}

static GLenum check_texture(GLXX_TEXTURE_T *texture, GLint level,
      GLint x, GLint y, GLint z, GLsizei width, GLsizei height, GLsizei depth)
{
   if (!glxx_texture_is_legal_level(texture->target, level))
      return GL_INVALID_VALUE;

   if (!glxx_texture_are_legal_dimensions(texture->target, x, y, z))
      return GL_INVALID_VALUE;

   if (!glxx_texture_are_legal_dimensions(texture->target, width, height, depth))
      return GL_INVALID_VALUE;

   /* Check the parameters using the image from the 1st slice. The texture
    * must be complete for the operation to actually go ahead, which will check
    * that all the other images match the ones that are checked here.  */
   struct selector first, size;
   tex_image_selector(&first, texture->target, z);
   tex_image_selector(&size,  texture->target, depth);
   if (first.face >= MAX_FACES || first.face + size.face > MAX_FACES)
      return GL_INVALID_VALUE;

   khrn_image *img = texture->img[first.face][level];
   if (!img)
      return GL_INVALID_VALUE;

   if (!valid_region(img, x, y, first.depth, first.elem,
         width, height, size.depth, size.elem))
      return GL_INVALID_VALUE;

   return GL_NO_ERROR;
}

static GLenum check_tex_completeness(GLXX_TEXTURE_T *texture, GLint level)
{
   unsigned base_level, num_levels;
   if (!glxx_texture_check_completeness(texture, false,
         &base_level, &num_levels))
      return GL_INVALID_OPERATION;

   if ((unsigned)level < base_level ||  (unsigned)level >= base_level + num_levels)
      return GL_INVALID_VALUE;

   return GL_NO_ERROR;
}

static GLenum check_renderbuffer(GLXX_RENDERBUFFER_T *rb, GLint level,
      GLint x, GLint y, GLint z, GLsizei width, GLsizei height, GLsizei depth)
{
   if (level != 0 || x < 0 || y < 0 || z != 0  ||
         width < 0 || height < 0 || depth < 0 || depth > 1)
      return GL_INVALID_VALUE;

   if ((unsigned)(x + width) > rb->width_pixels ||
         (unsigned)(y + height) > rb->height_pixels)
      return GL_INVALID_VALUE;

   if (!rb->image)
      return GL_INVALID_VALUE;

   return GL_NO_ERROR;
}

static GLenum check(const struct operand *operand,
      GLint level, GLint x, GLint y, GLint z,
      GLsizei width, GLsizei height, GLsizei depth)
{
   switch (operand->type)
   {
   case operand_type_texture:
      return check_texture(operand->texture, level, x, y, z,
                           width, height, depth);
   case operand_type_renderbuffer:
      return check_renderbuffer(operand->renderbuffer, level, x, y, z,
                                width, height, depth);
   default:
      unreachable();
      return false;
   }
}

static void copy_one_slice(GLXX_SERVER_STATE_T *state,
      const struct operand *src, unsigned src_level,
      unsigned src_x, unsigned src_y, unsigned src_z,
      const struct operand *dst, unsigned dst_level,
      unsigned dst_x, unsigned dst_y, unsigned dst_z,
      unsigned  src_width, unsigned src_height)
{
   /* convert z coordinate into image selector (face,element,depth) */
   struct selector src_sel, dst_sel;
   image_selector(&src_sel, src, src_z);
   image_selector(&dst_sel, dst, dst_z);

   khrn_image *src_img = get_image(src, src_sel.face, src_level);
   khrn_image *dst_img = get_image(dst, dst_sel.face, dst_level);

   bool ok = khrn_image_memcpy_one_elem_slice(dst_img, dst_x, dst_y, dst_sel.depth,
         dst_sel.elem, src_img, src_x, src_y, src_sel.depth,
         src_sel.elem, src_width, src_height, &state->fences,
         egl_context_gl_secure(state->context));

   assert(ok);
}

static void set_dest_size(GFX_LFMT_T dst_lfmt, GLsizei *dst_width,
      GLsizei *dst_height, GLsizei *dst_depth, GFX_LFMT_T src_lfmt,
      GLsizei src_width, GLsizei src_height, GLsizei src_depth)
{
   GFX_LFMT_BASE_DETAIL_T src_bd, dst_bd;
   gfx_lfmt_base_detail(&src_bd, src_lfmt);
   gfx_lfmt_base_detail(&dst_bd, dst_lfmt);
   *dst_width  = src_width  * dst_bd.block_w / src_bd.block_w;
   *dst_height = src_height * dst_bd.block_h / src_bd.block_h;
   *dst_depth  = src_depth  * dst_bd.block_d / src_bd.block_d;
}

static void copy_image_sub_data_impl(GLuint src_name, GLenum src_target,
      GLint src_level, GLint src_x, GLint src_y, GLint src_z, GLuint dst_name,
      GLenum dst_target, GLint dst_level, GLint dst_x, GLint dst_y, GLint dst_z,
      GLsizei src_width, GLsizei src_height, GLsizei src_depth)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   struct operand src, dst;
   GLenum error = GL_NO_ERROR;
   if (!state)
      return;

   error = get_operand(state, src_name, src_target, &src);
   if (error)
      goto end;

   error = get_operand(state, dst_name, dst_target, &dst);
   if (error)
      goto end;

   if (!formats_match(&src, &dst) || !samples_match(&src, &dst))
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   error = check(&src, src_level, src_x, src_y, src_z,
         src_width, src_height, src_depth);
   if (error)
      goto end;

   GLsizei dst_width, dst_height, dst_depth;
   set_dest_size(get_format(&dst), &dst_width, &dst_height, &dst_depth,
         get_format(&src), src_width, src_height, src_depth);

   error = check(&dst, dst_level, dst_x, dst_y, dst_z,
         dst_width, dst_height, dst_depth);
   if (error)
      goto end;

   if (src.type == operand_type_texture)
      error = check_tex_completeness(src.texture, src_level);
   if (dst.type == operand_type_texture)
      error = check_tex_completeness(dst.texture, dst_level);
   if (error)
      goto end;

   if (!src_width || !src_height || !src_depth)
      goto end; /* nothing to do */

   while (src_depth--)
   {
      copy_one_slice(state,
            &src, src_level, src_x, src_y, src_z++,
            &dst, dst_level, dst_x, dst_y, dst_z++,
            src_width, src_height);
   }

end:
   if (error)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glCopyImageSubDataEXT(GLuint srcName,
      GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
      GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY,
      GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
   copy_image_sub_data_impl(srcName, srcTarget, srcLevel, srcX, srcY, srcZ,
         dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight,
         srcDepth);
}

GL_APICALL void GL_APIENTRY glCopyImageSubDataOES(GLuint srcName,
      GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
      GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY,
      GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
   copy_image_sub_data_impl(srcName, srcTarget, srcLevel, srcX, srcY, srcZ,
         dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight,
         srcDepth);
}

#if KHRN_GLES32_DRIVER
GL_APICALL void GL_APIENTRY glCopyImageSubData(GLuint srcName,
      GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
      GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY,
      GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
   copy_image_sub_data_impl(srcName, srcTarget, srcLevel, srcX, srcY, srcZ,
         dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight,
         srcDepth);
}

#endif
