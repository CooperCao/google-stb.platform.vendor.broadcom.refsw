/******************************************************************************
 *  Copyright (C) 2019 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/

#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "default_nexus.h"
#include "nexus_base_mmap.h"
#include "nexus_core_utils.h"
#include "nexus_graphics2d.h"
#include "nexus_platform.h"
#include "nexus_playback.h"
#include "nexus_surface.h"
#include "nexus_surface_client.h"
#include "nxclient.h"
#include "bkni.h"
#include "media_probe.h"
#include "esutil.h"

#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>

#include <GLES2/gl2.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>

#include "png.h"

static const unsigned int WIDTH             = 1920;
static const unsigned int HEIGHT            = 1080;

/* video sizes on screen between 0 and 1, 1 meaning full screen */
static const double SCALE_MIN = 0.1f; /* for scale animation  */
static const double SCALE_MED = 0.5f; /* size for move animation */
static const double SCALE_MAX = 0.9f; /* max scale and bounding box for move */

static const int SCALE_STEP = 2; /* scale animation speed */
static const int MOVE_STEP  = 5; /* move animation speed */

BDBG_MODULE(video_sync);

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

enum AnimationType {
    MOVE,
    SCALE,
};

enum DrawType {
    STANDARD,
    ALTERNATING,
    MASK
};

typedef struct AppConfig
{
   char                 video[PATH_MAX];
   enum AnimationType   mode;
   enum DrawType        draw;
   int                  swapInterval;
} AppConfig;

struct Position
{
   int16_t x;
   int16_t y;
};

struct Size
{
   uint16_t width;
   uint16_t height;
};

struct Rect
{
   struct Position position;
   struct Size size;
};

enum { NUM_TEX_COORD = 2 * 4 }; /* u,v for 4 corners of quad */
struct Glyph
{
   GLfloat tex_coord[NUM_TEX_COORD];
};

enum { MAX_GLYPH = 256 };
struct Font
{
   GLuint texture;
   struct Glyph glyph[MAX_GLYPH];
   struct Size glyph_size;

   GLint program_object;

   GLint mvp_matrix_loc;
   GLint position_loc;
   GLint color_loc;
   GLint tex_coord_loc;
   GLint texture_loc;
};

struct State {
   struct AppConfig config;

   NEXUS_SurfaceClientHandle videoSurfaceClient;

   struct Size screen;
   struct Rect videoWindow;
   struct Rect videoMin;
   struct Rect videoMed;
   struct Rect videoMax;

   union animation
   {
      struct scale
      {
         int step;
         int scale; /* 0 = min_size, 100 = max_size */
      } scale;
      struct move
      {
         struct Position step;
      } move;
   } anim;

   NXPL_PlatformHandle nxplHandle;
   void* nxplWindow;
   NXPL_NativeWindowInfoEXT windowInfo;

   EGLDisplay eglDisplay;
   EGLSurface eglSurface;
   EGLContext eglContext;

   struct Font font;
};

static void FatalError(const char *cause)
{
   fprintf(stderr, cause);
   assert(0);
   exit(1);
}

static bool LoadFont(struct Font *font, const char *fname)
{
   /* DONT CHANGE THE SCOPE HERE... GCC4.5 has a bug with goto and you'll get compile
      errors */
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   png_byte pixel_depth, color_type;
   int w, h;
   png_bytep *row_pointers = NULL;
   unsigned char *texture_img = NULL;

   /* open file and test for it being a png */
   FILE *fp = fopen(fname, "rb");
   if (!fp)
      return false;

   char header[8];               /* 8 is the maximum size that can be checked */
   fread(header, 1, 8, fp);
   if (png_sig_cmp((png_bytep)header, 0, 8))
      goto error0;

   /* initialize stuff */
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
      goto error0;

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
      goto error1;

   if (setjmp(png_jmpbuf(png_ptr)))
      goto error2;

   png_init_io(png_ptr, fp);
   png_set_sig_bytes(png_ptr, 8);

   png_read_info(png_ptr, info_ptr);
   png_set_interlace_handling(png_ptr);

   pixel_depth = png_get_bit_depth(png_ptr, info_ptr) * png_get_channels(png_ptr, info_ptr);
   color_type  = png_get_color_type(png_ptr, info_ptr);

   if ((color_type == PNG_COLOR_TYPE_GRAY_ALPHA) && (pixel_depth == 0x10))
   {
      png_set_expand(png_ptr);
      png_set_gray_to_rgb(png_ptr);
   }
   else if ((color_type == PNG_COLOR_TYPE_RGB) && (pixel_depth == 0x18))
   {
      png_set_expand(png_ptr);
      png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
   }

   png_read_update_info(png_ptr, info_ptr);

   w = png_get_image_width(png_ptr, info_ptr);
   h = png_get_image_height(png_ptr, info_ptr);

   row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * h);
   if (!row_pointers)
      goto error2;

   texture_img = malloc(png_get_rowbytes(png_ptr, info_ptr) * h);
   if (!texture_img)
      goto error3;

   /* load the image upside down for OpenGL */
   row_pointers[h - 1] = (png_bytep)texture_img;
   for (int y=h-1; y>0; y--)
      row_pointers[y-1] = (png_bytep)(row_pointers[y] + png_get_rowbytes(png_ptr, info_ptr));

   /* read file */
   if (setjmp(png_jmpbuf(png_ptr)))
      goto error4;

   png_read_image(png_ptr, row_pointers);

   png_read_end(png_ptr, info_ptr);

   /* assume a single glyph for now */
   font->glyph_size.width = w;
   font->glyph_size.height = h;

   glGenTextures(1, &font->texture);
   glBindTexture(GL_TEXTURE_2D, font->texture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_img);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   free(texture_img);
   free(row_pointers);
   free(info_ptr);
   free(png_ptr);

   fclose(fp);

   return true;

error4:
   free(texture_img);
error3:
   free(row_pointers);
error2:
   free(info_ptr);
error1:
   free(png_ptr);

error0:
   fclose(fp);
   return false;
}

static void InitFontGeometry(struct Font *font)
{
   static const char s_glyphs[] = "0123456789:xposize";
   static const int num_glyphs = sizeof(s_glyphs) - 1; /* -1 for NULL termination */
   GLfloat glyphWidth = 1.0f / (GLfloat)num_glyphs;
   for (int i = 0; i < num_glyphs; i++)
   {
      struct Glyph *glyph = &font->glyph[(int)s_glyphs[i]];

      GLfloat left   = glyphWidth * i;
      GLfloat right  = left + glyphWidth;
      GLfloat top    = 1.0f;
      GLfloat bottom = 0.0f;

      /* triangle fan */
      /* 3-------2
         |     / |
         | /     |
         0-------1 */
      glyph->tex_coord[0] = left;   glyph->tex_coord[1] = bottom;
      glyph->tex_coord[2] = right;  glyph->tex_coord[3] = bottom;
      glyph->tex_coord[4] = right;  glyph->tex_coord[5] = top;
      glyph->tex_coord[6] = left;   glyph->tex_coord[7] = top;
   }

   /* update glyph size */
   font->glyph_size.width /= num_glyphs;
   /* font->glyph_size.height is valid as there's just one row of glyphs */
}

static void InitFontShaders(struct Font *font)
{
   /* The shaders */
   const char vShaderStr[] =
         "uniform mat4   u_mvpMatrix;                          \n"
         "attribute vec4 a_position;                           \n"
         "attribute vec2 a_texCoord;                           \n"
         "varying vec2 v_texCoord;                             \n"
         "                                                     \n"
         "void main()                                          \n"
         "{                                                    \n"
         "  gl_Position = u_mvpMatrix * a_position;            \n"
         "  v_texCoord = a_texCoord;                           \n"
         "}                                                    \n";

   const char fShaderStr[] =
         "precision mediump float;                             \n"
         "uniform vec4 u_color;                                \n"
         "varying vec2 v_texCoord;                             \n"
         "uniform sampler2D s_texture;                         \n"
         "                                                     \n"
         "void main()                                          \n"
         "{                                                    \n"
         "  gl_FragColor = vec4(u_color.rgb, u_color.a *       \n"
         "     texture2D(s_texture, v_texCoord).a);            \n"
         "}                                                    \n";

   GLuint v = glCreateShader(GL_VERTEX_SHADER);
   GLuint f = glCreateShader(GL_FRAGMENT_SHADER);

   const char *ff = fShaderStr;
   const char *vv = vShaderStr;

   glShaderSource(v, 1, &vv, NULL);
   glShaderSource(f, 1, &ff, NULL);

   /* Compile the shaders */
   glCompileShader(v);

   GLint ret;
   glGetShaderiv(v, GL_COMPILE_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetShaderiv(v, GL_INFO_LOG_LENGTH, &ret);
      char *p = (char *)alloca(ret);
      glGetShaderInfoLog(v, ret, NULL, p);
      FatalError(p);
   }
   glCompileShader(f);
   glGetShaderiv(f, GL_COMPILE_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetShaderiv(f, GL_INFO_LOG_LENGTH, &ret);
      char *q = (char *)alloca(ret);
      glGetShaderInfoLog(f, ret, NULL, q);
      FatalError(q);
   }

   font->program_object = glCreateProgram();
   glAttachShader(font->program_object, v);
   glAttachShader(font->program_object, f);

   /* Link the program */
   glLinkProgram(font->program_object);

   glGetError();

   /* Get the attribute locations */
   font->position_loc  = glGetAttribLocation(font->program_object, "a_position");
   font->tex_coord_loc = glGetAttribLocation(font->program_object, "a_texCoord");

   /* Get the uniform locations */
   font->mvp_matrix_loc = glGetUniformLocation(font->program_object, "u_mvpMatrix");
   font->color_loc      = glGetUniformLocation(font->program_object, "u_color");
   font->texture_loc    = glGetUniformLocation(font->program_object, "s_texture");
}

static bool InitFont(struct Font *font, unsigned width, unsigned height)
{
   ESMatrix mvp_matrix;

   if (!LoadFont(font, "glyph.png"))
      FatalError("Failed to load font file: glyph.png");

   esMatrixLoadIdentity(&mvp_matrix);

   /* This program uses a virtual co-ordinate system based at 1080p.
      This doesn't imply that it is rendered at this size */
   esOrtho(&mvp_matrix, 0.0f, width, 0.0f, height, -1.0f, 1.0f);

   InitFontShaders(font);

   /* Load the MVP matrix */
   glUseProgram(font->program_object);
   glUniformMatrix4fv(font->mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)&mvp_matrix.m[0][0]);

   InitFontGeometry(font);
   return true;
}

static void TerminateFont(struct Font *font)
{
   glDeleteTextures(1, &font->texture);

   glDeleteProgram(font->program_object);
}

static void DrawText(struct State* state, const char *text, unsigned x, unsigned y)
{
   struct Font *font = &state->font;

   glUseProgram(font->program_object);

   /* TODO: pass text colour as parameter */
   glUniform4f(font->color_loc, 0, 1, 1, 1);

   /* Enable texture */
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, font->texture);
   glUniform1i(font->texture_loc, 0);

   /* Finally draw the elements */
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   size_t len = strlen(text);
   for (size_t i = 0; i < len; i++)
   {
      /* Enable quad array */
      struct Glyph *g = &font->glyph[(int)text[i]];

      GLfloat fx = x * font->glyph_size.width;
      GLfloat fy = y * font->glyph_size.height;
      GLfloat fwidth = font->glyph_size.width;
      GLfloat fheight = font->glyph_size.height;

      GLfloat quad[] = {
            /*    POSITION    */
            fx         , fy          ,  1.0f, g->tex_coord[0], g->tex_coord[1],
            fx + fwidth, fy          ,  1.0f, g->tex_coord[2], g->tex_coord[3],
            fx + fwidth, fy + fheight,  1.0f, g->tex_coord[4], g->tex_coord[5],
            fx         , fy + fheight,  1.0f, g->tex_coord[6], g->tex_coord[7],
      };

      glVertexAttribPointer(font->position_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), quad);
      glEnableVertexAttribArray(font->position_loc);
      glVertexAttribPointer(font->tex_coord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &quad[3]);
      glEnableVertexAttribArray(font->tex_coord_loc);

      /* Draw quad */
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

      glDisableVertexAttribArray(font->position_loc);
      glDisableVertexAttribArray(font->tex_coord_loc);

      x++;
   }

   glDisable(GL_BLEND);
}

static bool InitEGL(struct State* state)
{
   state->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (state->eglDisplay == EGL_NO_DISPLAY)
   {
      BDBG_ERR(("Failed to get EGL display"));
      return false;
   }

   EGLint major, minor;
   if (!eglInitialize(state->eglDisplay, &major, &minor))
   {
      BDBG_ERR(("eglInitialize() failed"));
   return false;
   }

   static EGLint attr[] =
   {
      EGL_SURFACE_TYPE,      EGL_WINDOW_BIT,
      EGL_RENDERABLE_TYPE,   EGL_OPENGL_ES2_BIT,
      EGL_DEPTH_SIZE,        24,
      EGL_RED_SIZE,          8,
      EGL_GREEN_SIZE,        8,
      EGL_BLUE_SIZE,         8,
      EGL_ALPHA_SIZE,        8,
      EGL_BUFFER_SIZE,       EGL_DONT_CARE,
      EGL_NONE
   };

   EGLConfig ecfg;
   EGLint numConfig;
   if (!eglChooseConfig(state->eglDisplay, attr, &ecfg, 1, &numConfig) || (numConfig == 0))
   {
      BDBG_ERR(("eglChooseConfig() failed"));
      return false;
   }

   state->eglSurface = eglCreateWindowSurface(state->eglDisplay, ecfg, state->nxplWindow, NULL);
   if (state->eglSurface == EGL_NO_SURFACE)
   {
      BDBG_ERR(("Failed to create EGL window surface!"));
      return false;
   }

   eglBindAPI(EGL_OPENGL_ES_API);

   static EGLint ctxattr[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   state->eglContext = eglCreateContext(state->eglDisplay, ecfg, EGL_NO_CONTEXT, ctxattr);
   if (state->eglContext == EGL_NO_CONTEXT)
   {
      BDBG_ERR(("Failed to create EGL context!"));
      return false;
   }

   BDBG_LOG(("EGL %d.%d Initialization finished.", major, minor));

   eglMakeCurrent(state->eglDisplay, state->eglSurface, state->eglSurface, state->eglContext);

   eglSwapInterval(state->eglDisplay, state->config.swapInterval);

   return true;
}

void TerminateEGL(struct State* state)
{
   eglMakeCurrent(state->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglTerminate(state->eglDisplay);
}

static bool InitGL(struct State* state)
{
   glViewport(0, 0, state->screen.width, state->screen.height);

   glDisable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);

   if (!InitFont(&state->font, state->screen.width, state->screen.height))
      assert(0);

   return true;
}

void TerminateGL(struct State* state)
{
   TerminateFont(&state->font);
}

static bool InitDisplay(struct State* state, unsigned surfaceClientId)
{
   NXPL_RegisterNexusDisplayPlatform(&state->nxplHandle, NULL);

   NXPL_GetDefaultNativeWindowInfoEXT(&state->windowInfo);
   state->windowInfo.x = 0;
   state->windowInfo.y = 0;
   state->windowInfo.width = state->screen.width;
   state->windowInfo.height = state->screen.height;
   state->windowInfo.stretch = true;
   state->windowInfo.clientID = surfaceClientId;

   state->nxplWindow = NXPL_CreateNativeWindowEXT(&state->windowInfo);

   if (!state->nxplWindow)
   {
      BDBG_ERR(("Error: NXPL_CreateNativeWindowEXT/NXPL_CreateNativeWindow() failed"));
      return false;
   }

#if NEXUS_COMMON_PLATFORM_VERSION >= NEXUS_PLATFORM_VERSION(17,3)
   NxClient_DisplaySettings display_settings;
   NxClient_GetDisplaySettings(&display_settings);
   display_settings.graphicsSettings.sourceBlendFactor =
      NEXUS_CompositorBlendFactor_eConstantAlpha;
   display_settings.graphicsSettings.destBlendFactor =
      NEXUS_CompositorBlendFactor_eInverseSourceAlpha;
   display_settings.graphicsSettings.constantAlpha = 0xff;
   NxClient_SetDisplaySettings(&display_settings);
#endif

   return InitEGL(state) && InitGL(state);
}

void TerminateDisplay(struct State* state)
{
   TerminateGL(state);
   TerminateEGL(state);

   NXPL_ReleaseVideoWindowClientEXT(state->nxplWindow);
   NXPL_DestroyNativeWindow(state->nxplWindow);
   NXPL_UnregisterNexusDisplayPlatform(state->nxplHandle);
}

static void DrawContent(struct State* state, int frame)
{
   char str[256];

   switch (state->config.draw)
   {
   case ALTERNATING:
   {
      struct Colour {
         GLfloat r;
         GLfloat g;
         GLfloat b;
         GLfloat a;
      };
      static const struct Colour clear[7] = {
            {1.0f, 0.0f, 0.0f, 0.5f}, /* R */
            {0.0f, 1.0f, 0.0f, 0.5f}, /* G */
            {0.0f, 0.0f, 1.0f, 0.5f}, /* B */
            {0.0f, 1.0f, 1.0f, 0.5f}, /* C */
            {1.0f, 0.0f, 1.0f, 0.5f}, /* M */
            {1.0f, 1.0f, 0.0f, 0.5f}, /* Y */
            {1.0f, 1.0f, 1.0f, 0.5f}, /* W */
      };

      const struct Colour *colour = &clear[frame % 7];
      glClearColor(colour->r, colour->g, colour->b, colour->a);
      break;
   }
   case STANDARD:
      glClearColor(1.0f, 0.0f, 0.0f, 0.5f);
      break;
   case MASK:
      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      break;
   }
   glClearDepthf(1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   glEnable(GL_SCISSOR_TEST);

   /* (0,0) is the LOWER left corner in GL */
   GLint yFlip = state->screen.height -
         (state->videoWindow.position.y + state->videoWindow.size.height);
   glScissor(
         state->videoWindow.position.x, yFlip,
         state->videoWindow.size.width, state->videoWindow.size.height);

   BDBG_MSG(("CLEAR %d,%d %ux%u",
         state->videoWindow.position.x, yFlip,
         state->videoWindow.size.width, state->videoWindow.size.height));

   if (state->config.draw == MASK)
      glClearColor(0.0f, 0.0f, 0.0f, 0.9f); //black mask over video
   else
      glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //transparent cut-out

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   glDisable(GL_SCISSOR_TEST);

   snprintf(str, 255, "pos: %4dx%d", state->videoWindow.position.x, state->videoWindow.position.y);
   DrawText(state, str, 5, 16);

   snprintf(str, 255, "size:%4ux%u", state->videoWindow.size.width, state->videoWindow.size.height);
   DrawText(state, str, 5, 15);
}

static void MoveVideoWindow(struct State* state)
{
   state->windowInfo.videoX = state->videoWindow.position.x;
   state->windowInfo.videoY = state->videoWindow.position.y;
   state->windowInfo.videoWidth = state->videoWindow.size.width;
   state->windowInfo.videoHeight = state->videoWindow.size.height;
   NXPL_UpdateNativeWindowEXT(state->nxplWindow, &state->windowInfo);
}

static void FitInside(struct Size *inner, const struct Size *outer)
{
   double scalew = (double)outer->width  / inner->width;
   double scaleh = (double)outer->height / inner->height;

   double scale  = scalew < scaleh ? scalew : scaleh;

   inner->width  = (double)inner->width  * scale;
   inner->height = (double)inner->height * scale;
}

static void Move(struct Position *pos, const struct Position *distance)
{
   pos->x += distance->x;
   pos->y += distance->y;
}

static void Centre(struct Rect *dest, const struct Rect *src)
{
   dest->position.x = src->position.x + (src->size.width  - dest->size.width)  / 2;
   dest->position.y = src->position.y + (src->size.height - dest->size.height) / 2;
}

static void Stretch(struct Size *result, const struct Size *min, const struct Size *max, double value)
{
   result->width  = min->width  * (1.0f - value) + max->width  * value;
   result->height = min->height * (1.0f - value) + max->height * value;
}

static void AnimateScale(struct State* state)
{
   int new_scale = state->anim.scale.scale + state->anim.scale.step;
   if (new_scale < 0 || new_scale > 100)
      state->anim.scale.step *= -1; /* change direction */
   state->anim.scale.scale += state->anim.scale.step;

   Stretch(&state->videoWindow.size, &state->videoMin.size, &state->videoMax.size,
         state->anim.scale.scale / 100.0f);
   Centre(&state->videoWindow, &state->videoMax);

   BDBG_MSG(("scale %0.2f: %d,%d %ux%u",
         state->anim.scale.scale / 100.0f,
         state->videoWindow.position.x, state->videoWindow.position.y,
         state->videoWindow.size.width, state->videoWindow.size.height));
}

static void AnimatePosition(struct State* state)
{
   struct Position min, max;
   min = max = state->videoMax.position;
   max.x += state->videoMax.size.width  - state->videoWindow.size.width;
   max.y += state->videoMax.size.height - state->videoWindow.size.height;

   struct Position check = state->videoWindow.position;
   Move(&check, &state->anim.move.step);

   if (check.x < min.x || check.x > max.x)
      state->anim.move.step.x *= -1; /* change x direction */

   if (check.y < min.y || check.y > max.y)
      state->anim.move.step.y *= -1; /* change y direction */

   Move(&state->videoWindow.position, &state->anim.move.step);

   BDBG_MSG(("move %d,%d: %d,%d %ux%u",
         state->anim.move.step.x, state->anim.move.step.y,
         state->videoWindow.position.x, state->videoWindow.position.y,
         state->videoWindow.size.width, state->videoWindow.size.height));
}

static void Animate(struct State* state, int frame)
{
   if (state->config.mode == SCALE)
      AnimateScale(state);
   else
      AnimatePosition(state);
   DrawContent(state, frame);
   MoveVideoWindow(state);

   eglSwapBuffers(state->eglDisplay, state->eglSurface);
}

bool processArgs(int argc, const char *argv[], AppConfig *config)
{
   bool result = false;
   int   a;

   if (config == NULL)
      return false;

   config->video[0]          = '\0';
   config->mode              = MOVE;
   config->swapInterval      = 1;
   config->draw              = STANDARD;

   for (a = 1; a < argc; ++a)
   {
      const char  *arg = argv[a];

      if (strncmp(arg, "video=", 6) == 0)
      {
         if (sscanf(arg, "video=%s", config->video) != 1)
            return false;
         result = true; /* mandatory argument */
      }
      else if (strncmp(arg, "mode=", 5) == 0)
      {
         char modeStr[32];
         if (sscanf(arg, "mode=%s", modeStr) != 1)
            return false;
         if (strcmp(modeStr, "move") == 0)
            config->mode = MOVE;
         else if (strcmp(modeStr, "scale") == 0)
            config->mode = SCALE;
         else
            return false;
      }
      else if (strncmp(arg, "swap=", 5) == 0)
      {
         if (sscanf(arg, "swap=%d", &config->swapInterval) != 1)
            return false;
      }
      else if (strncmp(arg, "draw=", 5) == 0)
      {
         char bgStr[32];
         if (sscanf(arg, "draw=%s", bgStr) != 1)
            return false;
         if (strcmp(bgStr, "std") == 0)
            config->draw = STANDARD;
         else if (strcmp(bgStr, "alt") == 0)
            config->draw = ALTERNATING;
         else if (strcmp(bgStr, "mask") == 0)
            config->draw = MASK;
         else
            return false;
      }
      else
         return false;
   }
   return result;
}

int main(int argc, const char* argv[])
{
   NEXUS_Error rc;

   struct State state;
   memset(&state, 0, sizeof(state));

   if (!processArgs(argc, argv, &state.config))
   {
      const char *progname = argc > 0 ? argv[0] : "";
      fprintf(stderr,
         "Usage: %s video=<filename> [OPTION]...\n"
         "  mode=[move|scale]   set window animation mode, (default is move)\n"
         "  swap=N              set the OpenGLES swap interval (default is 1)\n"
         "  draw=[std|alt|mask] graphics drawing mode: (default is std)\n"
         "                      std:  semi-transparent red bg and transparent cutout\n"
         "                      alt:  alternating bg colours and transparent cutout\n"
         "                      mask: transparent bg and black mask over video\n"
         , progname);
      return -1;
   }

   if (access(state.config.video, R_OK) != 0)
   {
      BDBG_ERR(("Can't access file %s", state.config.video));
      return -1;
   }

   NxClient_JoinSettings joinSettings;
   NxClient_GetDefaultJoinSettings(&joinSettings);
   strncpy(joinSettings.name, "video sync", NXCLIENT_MAX_NAME);
   rc = NxClient_Join(&joinSettings);
   if (rc)
   {
      BDBG_ERR(("Failed to initialize Join NxClient!"));
      return -1;
   }

   NxClient_AllocSettings allocSettings;
   NxClient_AllocResults allocResults;
   NxClient_GetDefaultAllocSettings(&allocSettings);
   allocSettings.simpleVideoDecoder = 1;
   allocSettings.surfaceClient = 0;
   rc = NxClient_Alloc(&allocSettings, &allocResults);
   if (rc)
   {
      BDBG_ERR(("NxClient_Alloc failed!"));
      return -1;
   }
   BDBG_ASSERT(allocResults.simpleVideoDecoder[0].id);
   BDBG_ASSERT(!allocResults.surfaceClient[0].id);

   state.screen.width = WIDTH;
   state.screen.height = HEIGHT;
   if (!InitDisplay(&state, allocResults.surfaceClient[0].id))
   {
      BDBG_ERR(("Failed to initialize display!"));
      return -1;
   }

   state.videoSurfaceClient = NXPL_CreateVideoWindowClient(state.nxplWindow, 0);
   BDBG_ASSERT(state.videoSurfaceClient);

   struct probe_results probeResults;
   rc = probe_media(state.config.video, &probeResults);
   if (rc) return BERR_TRACE(rc);

   unsigned connectId;
   NxClient_ConnectSettings connectSettings;
   NxClient_GetDefaultConnectSettings(&connectSettings);
   connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
   connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
   connectSettings.simpleVideoDecoder[0].surfaceClientId = NXPL_GetClientID(state.nxplWindow);
   connectSettings.simpleVideoDecoder[0].windowId = 0;
   connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = probeResults.video[0].width;
   connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = probeResults.video[0].height;
   rc = NxClient_Connect(&connectSettings, &connectId);
   if (rc)
   {
      BDBG_ERR(("Failed to connect to NxClient"));
      return -1;
   }

   NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
   NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
   NEXUS_PlaypumpHandle playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
   BDBG_ASSERT(playpump);
   NEXUS_PlaybackHandle playback = NEXUS_Playback_Create();
   BDBG_ASSERT(playback);

   struct Size videoSize = { probeResults.video[0].width, probeResults.video[0].height};
   state.videoMin.size = state.videoMed.size = state.videoMax.size = videoSize;

   struct Size min = { SCALE_MIN * state.screen.width, SCALE_MIN * state.screen.height };
   struct Size med = { SCALE_MED * state.screen.width, SCALE_MED * state.screen.height };
   struct Size max = { SCALE_MAX * state.screen.width, SCALE_MAX * state.screen.height };

   FitInside(&state.videoMin.size, &min);
   FitInside(&state.videoMed.size, &med);
   FitInside(&state.videoMax.size, &max);

   struct Rect screen = {{0, 0}, {state.screen.width, state.screen.height}};
   Centre(&state.videoMin, &screen);
   Centre(&state.videoMed, &screen);
   Centre(&state.videoMax, &screen);

   BDBG_LOG(("Animation type: %s", state.config.mode == SCALE ? "scale" : "move"));
   BDBG_LOG(("Screen size:   %4u x %u", state.screen.width, state.screen.height));
   BDBG_LOG(("Video size:    %4u x %u", videoSize.width, videoSize.height));

   if (state.config.mode == SCALE)
   {
      state.videoWindow = state.videoMin;
      state.anim.scale.scale = 0;
      state.anim.scale.step = SCALE_STEP;
      BDBG_LOG(("Minimum scale: %4u x %u", state.videoMin.size.width, state.videoMin.size.height));
      BDBG_LOG(("Maximum scale: %4u x %u", state.videoMax.size.width, state.videoMax.size.height));
   }
   else
   {
      state.videoWindow = state.videoMed;
      state.anim.move.step.x = MOVE_STEP;
      state.anim.move.step.y = MOVE_STEP;
      BDBG_LOG(("Window size:   %4u x %u",
            state.videoMed.size.width, state.videoMed.size.height));
   }

   NEXUS_FilePlayHandle file = NEXUS_FilePlay_OpenPosix(state.config.video, state.config.video);
   if (!file)
   {
      BDBG_ERR(("Can't open file: %s", state.config.video));
      return -1;
   }

   NEXUS_SimpleStcChannelHandle stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

   NEXUS_PlaybackSettings playbackSettings;
   NEXUS_Playback_GetSettings(playback, &playbackSettings);
   playbackSettings.playpump = playpump;
   playbackSettings.simpleStcChannel = stcChannel;
   playbackSettings.playpumpSettings.transportType = probeResults.transportType;
   NEXUS_Playback_SetSettings(playback, &playbackSettings);

   NEXUS_SimpleVideoDecoderHandle videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(
      allocResults.simpleVideoDecoder[0].id);

   NEXUS_SimpleVideoDecoderStartSettings videoProgram;
   NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

   NEXUS_PlaybackPidChannelSettings playbackPidSettings;
   NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
   playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
   playbackPidSettings.pidTypeSettings.video.codec = probeResults.video[0].codec;
   playbackPidSettings.pidTypeSettings.video.index = true;
   playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
   videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, probeResults.video[0].pid, &playbackPidSettings);
   videoProgram.settings.codec = probeResults.video[0].codec;
   videoProgram.maxWidth = probeResults.video[0].width;
   videoProgram.maxHeight = probeResults.video[0].height;

   if (videoProgram.settings.pidChannel)
   {
      NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
      NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
   }

   /* Start playback */
   rc = NEXUS_Playback_Start(playback, file, NULL);
   if (rc)
      BDBG_ERR(("Failed to start playback! %d", rc));

   int frame = 0;
   while (1)
   {
      BDBG_MSG(("animate frame %d", frame));
      Animate(&state, frame);
      frame++;
   }

   BDBG_LOG(("Terminating"));

   NEXUS_Playback_Stop(playback);
   NEXUS_Playback_Destroy(playback);
   NEXUS_Playpump_Close(playpump);
   NEXUS_FilePlay_Close(file);

   TerminateDisplay(&state);

   NxClient_Disconnect(connectId);
   NxClient_Free(&allocResults);

   NxClient_Uninit();

   return 0;
}
