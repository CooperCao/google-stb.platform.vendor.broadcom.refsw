/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/
#include <EGL/egl.h>

/* TODO: This is a hack moved from old version of eglext.h to keep using
 * the EGL extensions directly here (i.e. not via eglGetProcAddress()).
 * This hack should be removed and code should be reworked to access the
 * relevant functionality properly (i.e. via eglGetProcAdderss()).
 */
#define EGL_EGLEXT_PROTOTYPES 1
#include <EGL/eglext.h>
#undef EGL_EGLEXT_PROTOTYPES

#include <EGL/eglext_brcm.h>
#include <GLES/glplatform.h>
#include <GLES3/gl3.h>
#include "vcos.h"
#include "../egl/egl_types.h"
#include "../egl/egl_thread.h"
#include "../egl/egl_display.h"
#include "../egl/egl_image.h"
#include "../egl/egl_image_framebuffer.h"
#include "../egl/egl_context_gl.h"
#include "glxx_server.h"
#include "glxx_tmu_blit.h"
#include "glxx_ds_to_color.h"

#ifdef TMU_DEBUG_SAVE_IMAGE
#include "libs/tools/txtfmt/txtfmt_gfx_buffer.h"

static void *map_image(size_t *psize, const KHRN_IMAGE_T *img)
{
   khrn_interlock_read_now(&img->blob->res_i->interlock);

   size_t offset = img->start_elem * img->blob->array_pitch;
   size_t size = img->num_array_elems * img->blob->array_pitch;
   void* ptr = gmem_map(img->blob->res_i->handle);

   gmem_sync_pre_cpu_access_range(
      img->blob->res_i->handle,
      offset,
      size,
      GMEM_SYNC_CPU_READ | GMEM_SYNC_RELAXED);

   if (psize) *psize = size;
   return ptr ? (char*)ptr + offset : NULL;
}

static void unmap_image(const KHRN_IMAGE_T *img)
{
   gmem_unmap(img->blob->res_i->handle);
}

void glxx_debug_save_image(const char *fname, const KHRN_IMAGE_T *img)
{
   char fullname[1024];
   size_t size;
   uint32_t plane_num;

   void *data = map_image(&size, img);

   for(plane_num = 0; plane_num < img->blob->desc[0].num_planes; plane_num++)
   {
      sprintf(fullname, "%s_%u.txtfmt", fname, plane_num);
      // Info: display the output from txtfmt using gtxt
      txtfmt_store_gfx_buffer(&img->blob->desc[0], data, plane_num, true, fullname,
         NULL, NULL);
   }

   unmap_image(img);
}
#endif

typedef GLfloat fpoint_t[2];
struct fbox
{
   fpoint_t    inf;
   fpoint_t    sup;
};

enum cmp_type
{
   CMP_FLOAT,
   CMP_INT,
   CMP_UINT,
   N_CMP_TYPES,
};

struct gl_state
{
   GLuint fb;
   GLuint program[N_CMP_TYPES];
   GLuint vertex_buf;
};

struct scissor_state
{
   GLboolean enabled;
   GLint     box[4];// Only set if scissor_enabled
};

struct context
{
   EGLContext  context;

   /*
    * This is just the config_id of the EGLContext, but it saves us looking it
    * up over and over again.
    */
   EGLint      config_id;
   EGLSurface  draw;
   EGLSurface  read;
};

/*
 * Anything in here can potentially be re-used on the next call to
 * glBlitFramebuffer
 */
struct persist
{
   struct context    context;
   struct gl_state   gl_state;
};
static struct persist g_persist;

enum buf_type
{
   COLOR,
   DEPTH,
   STENCIL,
   N_BUF_TYPES,
};

struct target_buf
{
   GLint          type;       /* either GL_RENDERBUFFER or GL_TEXTURE */
   GLuint         name;

   /*
    * Aux-buf targets need their own image to make a texture attachment from
    */
   EGLImageKHR    image;
   unsigned color_mask;

   /* Only meaningful for GL_TEXTURE */
   struct
   {
      GLint       target;
      GLint       level;
      GLint       layer;
   }
   tex;
};

struct target
{
   enum buf_type     buf_type;
   struct target_buf buffers[GLXX_MAX_RENDER_TARGETS];
   unsigned          count;
};

struct src_buf
{
   enum buf_type  buf_type;
   enum cmp_type  cmp_type;
   EGLImageKHR    image;
};

struct src
{
   struct src_buf    src_bufs[N_BUF_TYPES];
   unsigned          count;
};

static bool init_target_buf(struct target_buf *tb,
                            enum buf_type buf_type,
                            unsigned render_target, /* valid only for COLOR buf_type */
                            EGLContext ctx,
                            GLenum filter);

static void apply_context(const struct context *context)
{
   EGLDisplay dpy = eglGetCurrentDisplay();
   eglMakeCurrent(dpy, context->draw, context->read, context->context);
}

/*
 * Turn box which is a subrectangle of (0,0)->size into a floating-point box
 * fitting [0, size) into the range [0.0f, 1.0f).
 */
static void make_fbox(struct fbox *fbox,
      const struct box *box, const int size[2])
{
   int i;

   for (i = 0; i < 2; i++)
   {
      GLfloat fsize = (GLfloat) size[i];
      fbox->inf[i] = box->inf[i] / fsize;
      fbox->sup[i] = box->sup[i] / fsize;
   }
}

static GLfloat *append_vertex(GLfloat *buf, GLfloat v0, GLfloat v1)
{
   buf[0] = v0;
   buf[1] = v1;

   return buf + 2;
}

static GLfloat *append_tex_coord(GLfloat *buf, GLfloat t0, GLfloat t1)
{
   buf[0] = t0;
   buf[1] = t1;

   return buf + 2;
}

static void fill_vertex_buf_from_blit_geom(const struct blit_geom *blit_geom)
{
   /* 2D vertex followed by a 2D tex-coord for 6 vertices */
   /* We set the geometry up for individual triangles because the hardware
    * needs the final vertex to be the same for the two triangles in order
    * not to generate a diagonal seam where the texture coordinates are
    * interpolated differently. Neither STRIP nor FAN allows that. */
   GLfloat data[(2 + 2) * 6 * sizeof (GLfloat)], *p = data;
   struct fbox src, dst = {
      {0.f, 0.f},
      {1.f, 1.f}
   };
   int i;

   make_fbox(&src, &blit_geom->src, blit_geom->src_size);

   for(i = 0; i < 2; i++)
   {
      float srcwidth = src.sup[i] - src.inf[i];
      if(src.inf[i] < 0.f)
      {
         float shiftdst = -src.inf[i] / srcwidth;

         // Make destination geometry smaller
         if(blit_geom->flip[i])
            dst.sup[i] -= shiftdst;
         else
            dst.inf[i] += shiftdst;

         // Clamp source coordinate to [0 1] range
         src.inf[i] = 0.f;
      }
      // Same but for opposite bound
      if(src.sup[i] > 1.f)
      {
         float shiftdst = (src.sup[i] - 1.f) / srcwidth;

         if(blit_geom->flip[i])
            dst.inf[i] += shiftdst;
         else
            dst.sup[i] -= shiftdst;

         src.sup[i] = 1.f;
      }

      // Convert to normalized device coordinates
      dst.inf[i] = dst.inf[i]*2.f - 1.f;
      dst.sup[i] = dst.sup[i]*2.f - 1.f;
   }

   p = append_vertex(p, dst.inf[0], dst.sup[1]);
   p = append_tex_coord(p, blit_geom->flip[0] ? src.sup[0] : src.inf[0],
                           blit_geom->flip[1] ? src.inf[1] : src.sup[1]);

   p = append_vertex(p, dst.inf[0], dst.inf[1]);
   p = append_tex_coord(p, blit_geom->flip[0] ? src.sup[0] : src.inf[0],
                           blit_geom->flip[1] ? src.sup[1] : src.inf[1]);

   p = append_vertex(p, dst.sup[0], dst.sup[1]);
   p = append_tex_coord(p, blit_geom->flip[0] ? src.inf[0] : src.sup[0],
                           blit_geom->flip[1] ? src.inf[1] : src.sup[1]);

   p = append_vertex(p, dst.sup[0], dst.inf[1]);
   p = append_tex_coord(p, blit_geom->flip[0] ? src.inf[0] : src.sup[0],
                           blit_geom->flip[1] ? src.sup[1] : src.inf[1]);

   p = append_vertex(p, dst.inf[0], dst.inf[1]);
   p = append_tex_coord(p, blit_geom->flip[0] ? src.sup[0] : src.inf[0],
                           blit_geom->flip[1] ? src.sup[1] : src.inf[1]);

   p = append_vertex(p, dst.sup[0], dst.sup[1]);
   p = append_tex_coord(p, blit_geom->flip[0] ? src.inf[0] : src.sup[0],
                           blit_geom->flip[1] ? src.inf[1] : src.sup[1]);

   glBufferData(GL_ARRAY_BUFFER, sizeof data, data, GL_STATIC_DRAW);
}

/* Attribute locations that will be bound for the program */
static const unsigned int vertex_attrib = 0;
static const unsigned int tex_attrib = 1;

static void enable_attribs()
{
   glEnableVertexAttribArray(vertex_attrib);
   glVertexAttribPointer(vertex_attrib, 2, GL_FLOAT, GL_FALSE,
         4*sizeof(float), 0);

   glEnableVertexAttribArray(tex_attrib);
   glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE,
         4*sizeof(float), (void *)(2 * sizeof (GLfloat)));
}

static GLuint create_program(enum cmp_type cmp_type)
{
   GLuint vshader, fshader;
   GLuint program;

   static const char *vshader_source =
      "#version 300 es\n"
      "in vec4 vertex;\n"
      "in vec2 tex_coord;\n"
      "out vec2 vtex_coord;\n"
      "void main(void)\n"
      "{\n"
      "   gl_Position = vertex;\n"
      "   vtex_coord = tex_coord;\n"
      "}\n";

   static const char *fshader_version = "#version 300 es\n";
   static const char *sampler_decls[N_CMP_TYPES] = {
      "uniform highp sampler2D tex;\n",
      "uniform highp isampler2D tex;\n",
      "uniform highp usampler2D tex;\n"
   };
   static const char *output_decls[N_CMP_TYPES] = {
      "out vec4 FragColor;\n",
      "out ivec4 FragColor;\n",
      "out uvec4 FragColor;\n"
   };

   static const char *fshader_src =
      "in vec2 vtex_coord;\n"
      "void main(void)\n"
      "{\n"
      "   FragColor = texture(tex, vtex_coord);\n"
      "}\n";

   const char *(fshader_source[4]) = { fshader_version, sampler_decls[cmp_type], output_decls[cmp_type], fshader_src };

   vshader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vshader, 1, &vshader_source, NULL);
   glCompileShader(vshader);

   fshader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fshader, 4, fshader_source, NULL);
   glCompileShader(fshader);

   program = glCreateProgram();

   glAttachShader(program, vshader);
   glAttachShader(program, fshader);
   glBindAttribLocation(program, vertex_attrib, "vertex");
   glBindAttribLocation(program, tex_attrib,    "tex_coord");
   glLinkProgram(program);

   glDeleteShader(vshader);
   glDeleteShader(fshader);

   return program;
}

static void get_scissor_state(struct scissor_state *scissor)
{
   // If the outside context has scissor testing enabled, we need to save the scissor box.
   scissor->enabled = glIsEnabled(GL_SCISSOR_TEST);
   if(scissor->enabled)
      glGetIntegerv(GL_SCISSOR_BOX, scissor->box);
}

static void create_persistent_gl_resources(struct gl_state *state)
{
   if(!state->vertex_buf)
      glGenBuffers(1, &state->vertex_buf);

   /*
    * The program doesn't depend on the geometry or anything specific to a
    * particular blit, so can safely be re-used
    */
   for (int i=0; i<N_CMP_TYPES; i++) {
      if (!state->program[i])
         state->program[i] = create_program(i);
   }

   if (!state->fb)
      glGenFramebuffers(1, &state->fb);
}

static void apply_gl_state(struct gl_state *state,
      const struct scissor_state *scissor,
      const struct blit_geom *blit_geom)
{
   const struct box* dst = &blit_geom->dst;

   glBindBuffer(GL_ARRAY_BUFFER, state->vertex_buf);
   fill_vertex_buf_from_blit_geom(blit_geom);

   glViewport(dst->inf[0], dst->inf[1],
         dst->sup[0] - dst->inf[0],
         dst->sup[1] - dst->inf[1]);

   glDisable(GL_DEPTH_TEST);
   glDepthMask(GL_FALSE);
   glStencilMask(GL_FALSE);
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state->fb);

   if(scissor->enabled)
   {
      // Enable scissor testing on the inside context, and apply the box.
      glEnable(GL_SCISSOR_TEST);
      glScissor(scissor->box[0],
                scissor->box[1],
                scissor->box[2],
                scissor->box[3]);
   }
}

static void destroy_gl_persistent_resources(struct gl_state *state)
{
   for (int i=0; i<N_CMP_TYPES; i++) {
      if (state->program[i])
      {
         glDeleteProgram(state->program[i]);
         state->program[i] = 0;
      }
   }

   if (state->vertex_buf)
   {
      glDeleteBuffers(1, &state->vertex_buf);
      state->vertex_buf = 0;
   }

   if (state->fb)
   {
      glDeleteFramebuffers(1, &state->fb);
      state->fb = 0;
   }
}

static void destroy_src(struct src *src)
{
   unsigned i;
   struct src_buf *sb;
   EGLDisplay dpy = eglGetCurrentDisplay();

   for (i = 0; i < src->count; i++)
   {
      sb = src->src_bufs + i;

      if (sb->image)
      {
         eglDestroyImageKHR(dpy, sb->image);
         sb->image = EGL_NO_IMAGE_KHR;
      }
   }

   src->count = 0;
}

static GLuint texture_from_image(EGLImageKHR image, GLenum filter)
{
   GLuint ret = 0;

   glGenTextures(1, &ret);
   glBindTexture(GL_TEXTURE_2D, ret);
   glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

   return ret;
}

/*
 * Set up the source for the blit. This is some combination of source buffers
 * from ctx (so ctx should be the "outside" context). The source buffers are
 * packed into src_buf from the beginning, and there are between 0 and 3 of
 * them.
 */
static bool init_src(struct src *src, EGLContext ctx, GLbitfield mask)
{
   EGLDisplay dpy = eglGetCurrentDisplay();
   EGLint attribs[] =
   {
      EGL_GL_FRAMEBUFFER_TARGET_BRCM, GL_READ_FRAMEBUFFER,
      EGL_GL_FRAMEBUFFER_ATTACHMENT_BRCM, 0 /* to be poked */,
      EGL_NONE,
   };

   /* We're going to be poking in different values for this attribute */
   EGLint *att_attrib = attribs + 3;

   /*
    * The attachment attribute values corresponding to all the differents bits
    * in mask.
    */
   static const struct
   {
      enum buf_type  buf_type;
      GLbitfield     bits;
   }
   attachments[] =
   {
      { COLOR, GL_COLOR_BUFFER_BIT },
      { DEPTH, GL_DEPTH_BUFFER_BIT },
      { STENCIL, GL_STENCIL_BUFFER_BIT },
   };

   bool ok = false;

   memset(src, 0, sizeof *src);

   for (unsigned i = 0; i < sizeof(attachments)/sizeof(attachments[0]); i++)
   {
      struct src_buf *sb = src->src_bufs + src->count;
      if (mask & attachments[i].bits)
      {
         GLint fb_name;
         glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &fb_name);

         switch( attachments[i].buf_type)
         {
            case COLOR:
               {
                  glGetIntegerv(GL_READ_BUFFER, att_attrib);
                  /* This is a hack, since this should be GL_BACK */
                  if (*att_attrib == GL_BACK)
                  {
                     assert (fb_name == 0);
                    *att_attrib = GL_COLOR_ATTACHMENT0;
                  }
               }
               break;
            case DEPTH:
               *att_attrib = GL_DEPTH_ATTACHMENT;
               break;
            case STENCIL:
               *att_attrib = GL_STENCIL_ATTACHMENT;
               break;
            default:
               unreachable();
               break;
         }

         sb->image = eglCreateImageKHR(dpy, ctx, EGL_GL_FRAMEBUFFER_BRCM, NULL, attribs);

         if (sb->image == EGL_NO_IMAGE_KHR)
         {
            /* TODO: Is clearing the error the correct thing here? The existing code is not consistent. */
            EGLint error = eglGetError();
            assert(error == EGL_BAD_ALLOC);
            goto end;
         }

         sb->buf_type = attachments[i].buf_type;
         sb->cmp_type = CMP_FLOAT;
         ++src->count;

         if (sb->buf_type == COLOR) {
            if (fb_name != 0) {
               GLint cmp_type;
               glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER, *att_attrib,
                                                     GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &cmp_type);
               switch(cmp_type) {
                  case GL_INT:          sb->cmp_type = CMP_INT; break;
                  case GL_UNSIGNED_INT: sb->cmp_type = CMP_UINT; break;
                  default:              sb->cmp_type = CMP_FLOAT; break;
               }
            }
         }
      }
   }

   ok = true;
end:
   if (!ok) destroy_src(src);
   return ok;
}

static void attach_texture(const struct target_buf *tb, GLint attachment)
{
   assert(tb->type == GL_TEXTURE);

   if (tb->tex.target == GL_TEXTURE_3D)
   {
      glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER,
            attachment, tb->name, tb->tex.level, tb->tex.layer);
   }
   else
   {
      glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
            attachment, tb->tex.target, tb->name, tb->tex.level);
   }
}

static void attach_renderbuffer(const struct target_buf *tb,
      GLint attachment)
{
   assert(tb->type == GL_RENDERBUFFER);

   glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
         attachment, GL_RENDERBUFFER, tb->name);
}

/*
 * Set up the currently bound framebuffer to draw to the buffer(s) specified
 * by target. Note that we're always attaching to colour targets even if we're
 * blitting a depth/stencil buffer, because the shader program that we're
 * using to do the blit outputs to the colour attachment.
 */
static void apply_target(const struct target *target)
{
   unsigned i;

   for (i = 0; i < target->count; i++)
   {
      const struct target_buf *tb = target->buffers + i;

      assert(tb->color_mask != 0);

      glColorMask((tb->color_mask & (0xff<<24)) > 0,
            (tb->color_mask & (0xff<<16)) > 0,
            (tb->color_mask & (0xff<<8)) > 0,
            (tb->color_mask & 0xff) > 0);

      switch (tb->type)
      {
      case GL_TEXTURE:
         attach_texture(tb, GL_COLOR_ATTACHMENT0 + i);
         break;

      case GL_RENDERBUFFER:
         attach_renderbuffer(tb, GL_COLOR_ATTACHMENT0 + i);
         break;

      case GL_FRAMEBUFFER_DEFAULT:
         /* we've created a an eglimage with texture for this case, so we
          * shouldn't get here */
         assert(0);
         break;
      default:
         assert(0);
      }
   }
}

static void query_texture(struct target_buf *out, GLint attachment)
{
   glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachment,
         GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE,
         &out->tex.target);

   glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachment,
         GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL,
         &out->tex.level);

   if (out->tex.target == GL_NONE)
   {
      glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachment,
            GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER,
            &out->tex.layer);

      if (out->tex.layer == 0)
         out->tex.target = GL_TEXTURE_2D;
      else
         out->tex.target = GL_TEXTURE_3D;
   }
}

static bool init_color_target(struct target *target, EGLContext ctx,
      GLenum filter)
{
   memset(target, 0, sizeof *target);
   target->buf_type = COLOR;

   for (unsigned i = 0; i < GLXX_MAX_RENDER_TARGETS; i++)
   {
      struct target_buf *tb = target->buffers + target->count;

      GLint buf;
      glGetIntegerv(GL_DRAW_BUFFER0 + i, &buf);
      if (buf == GL_NONE) continue;

      glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, buf,
            GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &tb->type);

      if(tb->type == GL_NONE)
         continue;


      tb->color_mask = 0xffffffff;

      switch (tb->type)
      {
      case GL_TEXTURE:
         glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, buf,
            GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint *) &tb->name);
         query_texture(tb, buf);
         break;
      case GL_RENDERBUFFER:
         glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, buf,
            GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint *) &tb->name);
         break;
      case GL_FRAMEBUFFER_DEFAULT:
         if (!init_target_buf(tb, COLOR, i, ctx, filter))
            return false;
         assert(tb->type == GL_TEXTURE);
         break;
      default:
         assert(0);
      }
      ++target->count;
   }
   return true;
}

static bool init_aux_target(struct target *target, enum buf_type buf_type, EGLContext ctx)
{
   memset(target, 0, sizeof *target);

   if (!init_target_buf(&target->buffers[0], buf_type, 0, ctx, GL_NEAREST))
      return false;

   target->buf_type = buf_type;
   target->count++;
   return true;
}

static void destroy_target_buf(struct target_buf *target)
{
   EGLDisplay dpy = eglGetCurrentDisplay();

   if (target->image == EGL_NO_IMAGE_KHR)
      return;

   assert(target->type == GL_TEXTURE);

   eglDestroyImageKHR(dpy, target->image);
   glDeleteTextures(1, &target->name);

   /*
    * target_bufs that didn't have an EGLImage got their textures or
    * renderbuffers from the context that glBlitFramebuffer was originally
    * called in. We don't own them and have no business destroying them.
    */
}

static void destroy_target(struct target *target)
{
   unsigned i;

   for (i = 0; i < target->count; i++)
      destroy_target_buf(target->buffers + i);
}

/*
 * creates a texture (through an egl_image) from the draw framebuffer buffer
 * buf_type, or if buf_type = COLOR, from the color attachment for render_target;
 */
static bool init_target_buf(struct target_buf *tb,
                            enum buf_type buf_type,
                            unsigned render_target, /* valid only for COLOR buf_type */
                            EGLContext ctx,
                            GLenum filter)
{
   EGL_IMAGE_T *egl_image = NULL;
   EGLint attribs[] =
   {
      EGL_GL_FRAMEBUFFER_TARGET_BRCM, GL_DRAW_FRAMEBUFFER,
      EGL_GL_FRAMEBUFFER_ATTACHMENT_BRCM, 0,
      EGL_NONE,
   };
   int *att_attribs = attribs + 3;

   switch(buf_type)
   {
      case COLOR:
         *att_attribs = GL_COLOR_ATTACHMENT0 + render_target;
         break;
      case DEPTH:
         *att_attribs = GL_DEPTH_ATTACHMENT;
         break;
      case STENCIL:
         *att_attribs = GL_STENCIL_ATTACHMENT;
         break;
      default:
         assert(0);
   }

   bool ok = false;

   memset(tb, 0, sizeof *tb);

   EGLDisplay dpy = eglGetCurrentDisplay();
   tb->image = eglCreateImageKHR(dpy, ctx, EGL_GL_FRAMEBUFFER_BRCM, NULL,
         attribs);

   if (tb->image == EGL_NO_IMAGE_KHR)
      return false;

   egl_image = egl_get_image_refinc(tb->image);
   KHRN_IMAGE_T * image = egl_image_get_image(egl_image);

   assert(khrn_image_get_num_planes(image) == 1);

   switch(buf_type)
   {
      case COLOR:
         tb->color_mask = 0xffffffff;
         break;
      case DEPTH:
         tb->color_mask = glxx_ds_color_lfmt_get_depth_mask(khrn_image_get_lfmt(image, 0));
         break;
      case STENCIL:
         tb->color_mask = glxx_ds_color_lfmt_get_stencil_mask(khrn_image_get_lfmt(image, 0));
         break;
      default:
         assert(0);
   }

   egl_image_refdec(egl_image);

   tb->name = texture_from_image(tb->image, filter);
   if (tb->name == 0) goto end;
   tb->type = GL_TEXTURE;
   tb->tex.target = GL_TEXTURE_2D;

   ok = true;
end:
   if (!ok)
      destroy_target_buf(tb);
   return ok;
}

/* If src has a buffer matching target's buf_type, blit it */
static void blit_src(struct gl_state *gl,
      const struct target *target,
      const struct src *src,
      GLenum filter)
{
   unsigned i;
   const struct src_buf *sb = src->src_bufs;
   GLuint src_tex;

   assert(target->count);

   for (i = 0; i < src->count; i++)
   {
      sb = src->src_bufs + i;
      if (sb->buf_type == target->buf_type)
         break;
   }

   if (i == src->count)
      return;

   apply_target(target);

   glUseProgram(gl->program[sb->cmp_type]);

   // Texture is returned already bound.
   src_tex = texture_from_image(sb->image, filter);

   /* Blit the source buffer */
   /* See comment in geometry setup for why we use triangles, not a strip/fan */
   glDrawArrays(GL_TRIANGLES, 0, 6);

   glDeleteTextures(1, &src_tex);
}

/*
 * Create a new context in out with the same config as in, also which can share
 * all in's shareables.
 */
static bool clone_context(struct context *out, const struct context *in)
{
   EGLConfig config;
   EGLint num_config = 0;
   EGLDisplay dpy = eglGetCurrentDisplay();
   EGLint config_attrs[] = { EGL_CONFIG_ID, in->config_id, EGL_NONE };
   EGLint attrs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };

   memset(out, 0, sizeof *out);

   eglChooseConfig(dpy, config_attrs, &config, 1, &num_config);
   out->context = eglCreateContext(dpy, config, in->context, attrs);
   if (out->context == EGL_NO_CONTEXT) return false;

   out->config_id = in->config_id;
   return true;
}

static void destroy_context(struct context *ctx)
{
   EGLDisplay dpy = eglGetCurrentDisplay();

   if (ctx->context == EGL_NO_CONTEXT)
      return;

   eglDestroyContext(dpy, ctx->context);
}

/*
 * Obtain a context for doing the blit with, based on current (which is the
 * context in which glBlitFramebuffer is being called).
 */
static struct context *get_blit_context(const struct context *current)
{
   if (g_persist.context.config_id != current->config_id)
   {
      destroy_context(&g_persist.context);
      if (!clone_context(&g_persist.context, current))
         return NULL;
   }

   g_persist.context.read = current->read;
   g_persist.context.draw = current->draw;

   return &g_persist.context;
}

static void egl_image_get_size(GLint size[2], EGLImageKHR im)
{
   EGL_IMAGE_T *egl_image = NULL;
   KHRN_IMAGE_T *kim = NULL;
   unsigned w, h;

   egl_image= egl_get_image_refinc(im);
   assert(egl_image);

   kim = egl_image_get_image(egl_image);
   khrn_image_get_dimensions(kim, &w, &h, NULL, NULL);
   size[0] = w;
   size[1] = h;

   egl_image_refdec(egl_image);
}

/*
 * Make a valid box (i.e. inf <= sup) from two points p0 and p1. If p0 > p1 in
 * either dimension, record that by toggling the flip value for that
 * dimension.
 */
static void make_box(struct box *box, bool flip[2], point_t p0, point_t p1)
{
   int i;

   for (i = 0; i < 2; i++)
   {
      if (p0[i] <= p1[i])
      {
         box->inf[i] = p0[i];
         box->sup[i] = p1[i];
      }
      else
      {
         box->inf[i] = p1[i];
         box->sup[i] = p0[i];
         flip[i] = !flip[i];
      }
   }
}

static void init_blit_geom(struct blit_geom *geom,
      const struct src_buf *src,
      GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
      GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1)
{
   point_t src0 = {srcX0, srcY0};
   point_t src1 = {srcX1, srcY1};

   point_t dst0 = {dstX0, dstY0};
   point_t dst1 = {dstX1, dstY1};

   memset(geom, 0, sizeof *geom);
   make_box(&geom->src, geom->flip, src0, src1);
   make_box(&geom->dst, geom->flip, dst0, dst1);
   egl_image_get_size(geom->src_size, src->image);
}

static void query_context(struct context *context)
{
   EGLDisplay dpy = eglGetCurrentDisplay();

   memset(context, 0, sizeof *context);
   context->context = eglGetCurrentContext();
   eglQueryContext(dpy, context->context, EGL_CONFIG_ID, &context->config_id);
   context->draw = eglGetCurrentSurface(EGL_DRAW);
   context->read = eglGetCurrentSurface(EGL_READ);
}

void glxx_tmu_blit_framebuffer(
      GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
      GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
      GLbitfield mask, GLenum filter)
{
   GLenum error = GL_OUT_OF_MEMORY;
   struct blit_geom geom;
   struct src src;
   struct target dst[N_BUF_TYPES];
   struct gl_state *gl = &g_persist.gl_state;
   bool multisampled_read_framebuffer;

   /*
    * The "outside" context is the one in which glBlitFramebuffer was called.
    * The "inside" context is one we create internally (and keep in g_persist)
    * for doing the blit. Having an inside context was considered easier than
    * guaranteeing that all side-effects on the outside context were undone
    * when we return from glBlitFramebuffer
    */
   struct context outside, *inside;

   src.count = 0;/* In case we break early to the fail label. */
   memset(dst, 0 , N_BUF_TYPES * sizeof(struct target));

   {
      GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
      // Get the value of GL_SAMPLE_BUFFERS for the read framebuffer.
      multisampled_read_framebuffer = glxx_fb_get_ms_mode(state->bound_read_framebuffer) != GLXX_NO_MS;
      glxx_unlock_server_state();
   }

   query_context(&outside);
   inside = get_blit_context(&outside);
   if (!inside) goto end;

   if (!init_src(&src, outside.context, mask))
      goto end;

   /* we've decided to do the blit, we must have something in the src */
   assert(src.count != 0);

   if(multisampled_read_framebuffer)
   {
      srcX0 *= 2;
      srcY0 *= 2;
      srcX1 *= 2;
      srcY1 *= 2;
   }

   if (mask & GL_COLOR_BUFFER_BIT)
   {
      if (!init_color_target(&dst[COLOR], outside.context, filter))
         goto end;
   }

   if (mask & GL_DEPTH_BUFFER_BIT)
   {
      if (!init_aux_target(&dst[DEPTH], DEPTH, outside.context))
         goto end;
   }

   if (mask & GL_STENCIL_BUFFER_BIT)
   {
      if (!init_aux_target(&dst[STENCIL], STENCIL, outside.context))
         goto end;
   }

   init_blit_geom(&geom, &src.src_bufs[0],
         srcX0, srcY0, srcX1, srcY1,
         dstX0, dstY0, dstX1, dstY1);

   /* Get the scissor setting from the outside context so we can apply it inside */
   struct scissor_state scissor;
   get_scissor_state(&scissor);

   apply_context(inside);
   create_persistent_gl_resources(gl);
   apply_gl_state(gl, &scissor, &geom);

   enable_attribs();

   /* Do the blitting. */
   if (mask & GL_COLOR_BUFFER_BIT)
      blit_src(gl, &dst[COLOR], &src, filter);

   if (mask & GL_DEPTH_BUFFER_BIT)
      blit_src(gl, &dst[DEPTH], &src, filter);

   if (mask & GL_STENCIL_BUFFER_BIT)
      blit_src(gl, &dst[STENCIL], &src, filter);

   destroy_gl_persistent_resources(&g_persist.gl_state);

   apply_context(&outside);
   if (glGetError() != GL_NO_ERROR)
      goto end;

   error = GL_NO_ERROR;
end:
   destroy_src(&src);
   for (unsigned i =0; i < N_BUF_TYPES; i++)
      destroy_target(&dst[i]);

   if (error != GL_NO_ERROR)
   {
      GLXX_SERVER_STATE_T *state;

      if (egl_context_gl_lock())
      {
         state = egl_context_gl_server_state(NULL);
         glxx_server_state_set_error(state, error);
         egl_context_gl_unlock();
      }
   }

   /*
    * Destroy all the persistent data every time. Because we share everything
    * with the outside context, this potentially may use a lot of memory. The
    * optimization is worth exploring later, but, for now, it's simpler like
    * this.
    */
   glxx_blitframebuffer_shutdown();
}

void glxx_blitframebuffer_shutdown(void)
{
   destroy_context(&g_persist.context);
   memset(&g_persist, 0, sizeof g_persist);
}
