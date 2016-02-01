/******************************************************************************
 *    (c)2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/

#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <VG/openvg.h>

#include "esutil.h"
#include "default_linuxfb.h"

#include "../common/init.h"

typedef struct
{
   bool     useMultisample;
   bool     usePreservingSwap;
   bool     stretchToFit;
   int      x;
   int      y;
   int      vpW;
   int      vpH;
   int      bpp;
   int      frames;
   unsigned clientId;
} AppConfig;

const unsigned int WIDTH             = 720;
const unsigned int HEIGHT            = 480;
const unsigned int FRAMES            = 0;
const unsigned int BPP               = 32;

AppConfig            config;

NEXUS_DISPLAYHANDLE  nexus_display = 0;

EGLNativeDisplayType native_display = 0;
void                 *native_window = 0;
float                panelAspect = 1.0f;

LFPL_PlatformHandle lfpl_handle = 0;

ESMatrix projection_matrix;
ESMatrix modelview_matrix;
ESMatrix mvp_matrix;

GLint mvp_matrix_loc;
GLint position_loc;
GLint tex_coord_loc;
GLint texture_loc;

GLint program_object;

GLuint vbo[2];

GLuint texture;

EGLSurface egl_es_surface = 0;
EGLSurface egl_vg_surface = 0;
EGLContext egl_es_context = 0;
EGLContext egl_vg_context = 0;
EGLDisplay egl_display = 0;

static const GLfloat cube[] = {
   /*          POSITION                     TEXTURE_COORD     */
   1.000000f, 1.000000f, -1.000000f,     0.0f, 1.0f,
   1.000000f, -1.000000f, -1.000000f,    0.0f, 0.0f,
   -1.000000f, -1.000000f, -1.000000f,   1.0f, 0.0f,
   -1.000000f, 1.000000f, -1.000000f,    1.0f, 1.0f,

   -1.000000f, -1.000000f, 1.000000f,    0.0f, 0.0f,
   -1.000000f, 1.000000f, 1.000000f,     0.0f, 1.0f,
   -1.000000f, 1.000000f, -1.000000f,    1.0f, 1.0f,
   -1.000000f, -1.000000f, -1.000000f,   1.0f, 0.0f,

   1.000000f, -1.000000f, 1.000000f,     1.0f, 0.0f,
   1.000000f, 1.000000f, 1.000001f,      1.0f, 1.0f,
   -1.000000f, -1.000000f, 1.000000f,    0.0f, 0.0f,
   -1.000000f, 1.000000f, 1.000000f,     0.0f, 1.0f,

   1.000000f, -1.000000f, -1.000000f,    1.0f, 0.0f,
   1.000000f, 1.000000f, -1.000000f,     1.0f, 1.0f,
   1.000000f, -1.000000f, 1.000000f,     0.0f, 0.0f,
   1.000000f, 1.000000f, 1.000001f,      0.0f, 1.0f,

   1.000000f, 1.000000f, -1.000000f,     1.0f, 0.0f,
   -1.000000f, 1.000000f, -1.000000f,    0.0f, 0.0f,
   1.000000f, 1.000000f, 1.000001f,      1.0f, 1.0f,
   -1.000000f, 1.000000f, 1.000000f,     0.0f, 1.0f,

   1.000000f, -1.000000f, -1.000000f,    0.0f, 0.0f,
   1.000000f, -1.000000f, 1.000000f,     0.0f, 1.0f,
   -1.000000f, -1.000000f, 1.000000f,    1.0f, 1.0f,
   -1.000000f, -1.000000f, -1.000000f,   1.0f, 0.0f
};

static const GLushort cube_idx[] = {
   0, 1, 2,
   3, 0, 2,
   4, 5, 6,
   7, 4, 6,
   8, 9, 10,
   9, 11, 10,
   12, 13, 14,
   13, 15, 14,
   16, 17, 18,
   17, 19, 18,
   20, 21, 22,
   23, 20, 22,
};

static VGPath g_path = VG_INVALID_HANDLE;
static VGPaint g_paint = VG_INVALID_HANDLE;

static VGubyte const starSegments[] = {
   VG_MOVE_TO_ABS,
   VG_LINE_TO_ABS,
   VG_LINE_TO_ABS,
   VG_LINE_TO_ABS,
   VG_LINE_TO_ABS,
   VG_LINE_TO_ABS,
   VG_LINE_TO_ABS,
   VG_LINE_TO_ABS,
   VG_LINE_TO_ABS,
   VG_LINE_TO_ABS,
   VG_CLOSE_PATH
};

static VGfloat const starCoords[] = {
   437.39607f + 24.906574f, 377.64971f + 15.23675f,
   287.23644f + 24.906574f, 350.77055f + 15.23675f,
   179.96624f + 24.906574f, 459.23028f + 15.23675f,
   159.12796f + 24.906574f, 308.11386f + 15.23675f,
   22.828316f + 24.906574f, 239.60974f + 15.23675f,
   160.10919f + 24.906574f, 173.09382f + 15.23675f,
   183.14157f + 24.906574f, 22.296218f + 15.23675f,
   288.82409f + 24.906574f, 132.30354f + 15.23675f,
   439.35854f + 24.906574f, 107.60961f + 15.23675f,
   367.39305f + 24.906574f, 242.11379f + 15.23675f
};

void InitGLState(void)
{
   /* The shaders */
   const char vShaderStr[] =
      "uniform mat4 u_mvpMatrix;                            \n"
      "attribute vec4 a_position;                           \n"
      "attribute vec2 a_texCoord;                           \n"
      "varying vec2 v_texCoord;                             \n"
      "varying vec4 v_color;                                \n"
      "void main()                                          \n"
      "{                                                    \n"
      "  gl_Position = u_mvpMatrix * a_position;            \n"
      "  v_texCoord = a_texCoord;                           \n"
      "}                                                    \n";

   const char fShaderStr[] =
      "precision mediump float;                             \n"
      "varying vec2 v_texCoord;                             \n"
      "varying vec4 v_color;                                \n"
      "uniform sampler2D s_texture;                         \n"
      "void main()                                          \n"
      "{                                                    \n"
      "  gl_FragColor = texture2D(s_texture, v_texCoord);   \n"
      "}                                                    \n";

   GLuint     v, f;
   GLint      ret;
   const char *ff;
   const char *vv;
   char       *p, *q;

   glGenTextures(1, &texture);

   glBindTexture(GL_TEXTURE_2D, texture);
   /* setup a default texture */
   eglBindTexImage(egl_display, egl_vg_surface, EGL_BACK_BUFFER);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   glClearDepthf(1.0f);
#ifdef CLIENT_MAIN
   glClearColor(0.2f, 0.2f, 0.2f, 0);  /* Gray background */
#else
   glClearColor(0.2f, 0.2f, 0.2f, config.clientId ? 0 : 1);  /* Gray background */
#endif

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   /* Create vertex buffer objects */
   glGenBuffers(2, vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_idx), cube_idx, GL_STATIC_DRAW);

   v = glCreateShader(GL_VERTEX_SHADER);
   f = glCreateShader(GL_FRAGMENT_SHADER);

   ff = fShaderStr;
   vv = vShaderStr;
   glShaderSource(v, 1, &vv, NULL);
   glShaderSource(f, 1, &ff, NULL);

   /* Compile the shaders */
   glCompileShader(v);

   glGetShaderiv(v, GL_COMPILE_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetShaderiv(v, GL_INFO_LOG_LENGTH, &ret);
      p = (char *)alloca(ret);
      glGetShaderInfoLog(v, ret, NULL, p);
      assert(0);
   }
   glCompileShader(f);
   glGetShaderiv(f, GL_COMPILE_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetShaderiv(f, GL_INFO_LOG_LENGTH, &ret);
      q = (char *)alloca(ret);
      glGetShaderInfoLog(f, ret, NULL, q);
      assert(0);
   }

   program_object = glCreateProgram();
   glAttachShader(program_object, v);
   glAttachShader(program_object, f);

   /* Link the program */
   glLinkProgram(program_object);

   /* Get the attribute locations */
   position_loc = glGetAttribLocation(program_object, "a_position");
   tex_coord_loc = glGetAttribLocation(program_object, "a_texCoord");

   /* Get the uniform locations */
   mvp_matrix_loc = glGetUniformLocation(program_object, "u_mvpMatrix");
   texture_loc = glGetUniformLocation(program_object, "s_texture");

   esMatrixLoadIdentity(&modelview_matrix);
   esTranslate(&modelview_matrix, 0, 0, -500);
   esScale(&modelview_matrix, 100, 100, 100);

   eglBindAPI(EGL_OPENVG_API);
   eglMakeCurrent(egl_display, egl_vg_surface, egl_vg_surface, egl_vg_context);

   /* Cache the path */
   if (g_path == VG_INVALID_HANDLE || !vgGetPathCapabilities(g_path))
   {
      g_path = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                        VG_PATH_DATATYPE_F,
                        1.0f, /* scale */
                        0.0f, /* bias */
                        6,    /* segmentCapacityHint */
                        10,   /* coordCapacityHint */
                        VG_PATH_CAPABILITY_ALL);
      vgAppendPathData(g_path, sizeof(starSegments), starSegments, starCoords);
   }

   if (g_paint == VG_INVALID_HANDLE)
   {
      g_paint = vgCreatePaint();
      vgSetColor(g_paint, 0xFFFF00FF);    /* yellow */
   }

   vgSetf(VG_STROKE_LINE_WIDTH, 4.0f);
}

void TerminateGLState(void)
{
   glDeleteProgram(program_object);
   glDeleteBuffers(2, vbo);

   glDeleteTextures(1, &texture);
}

void InitGLViewPort(unsigned int width, unsigned int height, float panelAspect, bool stretch)
{
   glViewport(0, 0, width, height);

   esMatrixLoadIdentity(&projection_matrix);

   if (stretch)
      esPerspective(&projection_matrix, 45.0f, panelAspect, 100, 1000);
   else
      esPerspective(&projection_matrix, 45.0f, (float)width / (float)height, 100, 1000);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void Resize(void)
{
   EGLint w = 0, h = 0;

   /* As this is just an example, and we don't have any kind of resize event, we will
      check whether the underlying window has changed size and adjust our viewport at the start of
      each frame. Obviously, this would be more efficient if event driven. */
   eglQuerySurface(egl_display, eglGetCurrentSurface(EGL_DRAW), EGL_WIDTH, &w);
   eglQuerySurface(egl_display, eglGetCurrentSurface(EGL_DRAW), EGL_HEIGHT, &h);

   if (w != config.vpW || h != config.vpH)
   {
      config.vpW = w;
      config.vpH = h;

      /* Ignore the panelAspect and stretch - if we resized we are window based anyway */
      InitGLViewPort(w, h, (float)w / (float)h, false);
   }
}

void Display(void)
{
   VGfloat color[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; /* blue */

   Resize();

   eglBindAPI(EGL_OPENVG_API);
   eglMakeCurrent(egl_display, egl_vg_surface, egl_vg_surface, egl_vg_context);

   vgSeti(VG_SCISSORING, VG_FALSE);
   vgSetfv(VG_CLEAR_COLOR, 4, color);
   vgClear(0, 0, 512, 512);

   vgTranslate(256.0f, 256.0f);
   vgRotate(1.0f);
   vgTranslate(-256.0f, -256.0f);

   /* Draw the star directly using the OpenVG API. */
   vgSetPaint(g_paint, VG_FILL_PATH);
   vgDrawPath(g_path, VG_FILL_PATH | VG_STROKE_PATH);

   eglBindAPI(EGL_OPENGL_ES_API);
   eglMakeCurrent(egl_display, egl_es_surface, egl_es_surface, egl_es_context);

   /* Rotate the cube */
   esRotate(&modelview_matrix, 1.0f, 1, 0, 0);
   esRotate(&modelview_matrix, 0.5f, 0, 1, 0);

   /* Compute the final MVP by multiplying the model-view and perspective matrices together */
   esMatrixMultiply(&mvp_matrix, &modelview_matrix, &projection_matrix);

   /* Clear all the buffers we asked for during config to ensure fast-path */
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(program_object);

   /* Enable cube array */
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
   glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), BUFFER_OFFSET(0));
   glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
   glEnableVertexAttribArray(position_loc);
   glEnableVertexAttribArray(tex_coord_loc);

   /* Load the MVP matrix */
   glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)&mvp_matrix.m[0][0]);

   glBindTexture(GL_TEXTURE_2D, texture);
   glUniform1i(texture_loc, 0);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

   /* Finally draw the elements */
   glDrawElements(GL_TRIANGLES, sizeof(cube_idx) / sizeof(GLushort), GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

   /* Display the framebuffer */
   eglSwapBuffers(egl_display, eglGetCurrentSurface(EGL_READ));
}

static void BppToChannels(int bpp, int *r, int *g, int *b, int *a)
{
   switch (bpp)
   {
   default:
   case 16:             /* 16-bit RGB (565)  */
      *r = 5;
      *g = 6;
      *b = 5;
      *a = 0;
      break;

   case 32:             /* 32-bit RGBA       */
      *r = 8;
      *g = 8;
      *b = 8;
      *a = 8;
      break;

   case 24:             /* 24-bit RGB        */
      *r = 8;
      *g = 8;
      *b = 8;
      *a = 0;
      break;
   }
}

bool InitEGL(NativeWindowType egl_win, const AppConfig *config)
{
   EGLDisplay egl_display      = 0;
   EGLConfig *egl_config;
   EGLint     major_version;
   EGLint     minor_version;
   int        config_select    = 0;
   int        configs;

   /*
      Specifies the required configuration attributes.
      An EGL "configuration" describes the pixel format and type of
      surfaces that can be used for drawing.
      For now we just want to use a 16 bit RGB surface that is a
      Window surface, i.e. it will be visible on screen. The list
      has to contain key/value pairs, terminated with EGL_NONE.
   */
   int   want_red   = 0;
   int   want_green = 0;
   int   want_blue  = 0;
   int   want_alpha = 0;

   BppToChannels(config->bpp, &want_red, &want_green, &want_blue, &want_alpha);

   /*
      Step 1 - Get the EGL display.
   */
   egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (egl_display == EGL_NO_DISPLAY)
   {
      printf("eglGetDisplay() failed, did you register any exclusive displays\n");
      return false;
   }

   /*
      Step 2 - Initialize EGL.
      EGL has to be initialized with the display obtained in the
      previous step. We cannot use other EGL functions except
      eglGetDisplay and eglGetError before eglInitialize has been
      called.
   */
   if (!eglInitialize(egl_display, &major_version, &minor_version))
   {
      printf("eglInitialize() failed\n");
      return false;
   }

   /*
      Step 3 - Get the number of configurations to correctly size the array
      used in step 4
   */
   if (!eglGetConfigs(egl_display, NULL, 0, &configs))
   {
      printf("eglGetConfigs() failed\n");
      return false;
   }

   egl_config = (EGLConfig *)alloca(configs * sizeof(EGLConfig));

   /*
      Step 4 - Find a config that matches all requirements.
      eglChooseConfig provides a list of all available configurations
      that meet or exceed the requirements given as the second
      argument.
   */

   {
      const int   NUM_ATTRIBS = 21;
      EGLint      *attr = (EGLint *)malloc(NUM_ATTRIBS * sizeof(EGLint));
      int         i = 0;

      attr[i++] = EGL_RED_SIZE;        attr[i++] = want_red;
      attr[i++] = EGL_GREEN_SIZE;      attr[i++] = want_green;
      attr[i++] = EGL_BLUE_SIZE;       attr[i++] = want_blue;
      attr[i++] = EGL_ALPHA_SIZE;      attr[i++] = want_alpha;
      attr[i++] = EGL_DEPTH_SIZE;      attr[i++] = 24;
      attr[i++] = EGL_STENCIL_SIZE;    attr[i++] = 0;
      attr[i++] = EGL_SURFACE_TYPE;    attr[i++] = EGL_WINDOW_BIT;
      attr[i++] = EGL_RENDERABLE_TYPE; attr[i++] = EGL_OPENGL_ES2_BIT | EGL_OPENVG_BIT;

      if (config->useMultisample)
      {
         attr[i++] = EGL_SAMPLE_BUFFERS; attr[i++] = 1;
         attr[i++] = EGL_SAMPLES;        attr[i++] = 4;
      }

      attr[i++] = EGL_NONE;

      assert(i <= NUM_ATTRIBS);

      if (!eglChooseConfig(egl_display, attr, egl_config, configs, &configs) || (configs == 0))
      {
         printf("eglChooseConfig() failed");
         return false;
      }

      free(attr);
   }

   for (config_select = 0; config_select < configs; config_select++)
   {
      /*
         Configs with deeper color buffers get returned first by eglChooseConfig.
         Applications may find this counterintuitive, and need to perform additional processing on the list of
         configs to find one best matching their requirements. For example, specifying RGBA depths of 565
         could return a list whose first config has a depth of 888.
      */

      /* Check that config is an exact match */
      EGLint red_size, green_size, blue_size, alpha_size, depth_size;

      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_RED_SIZE,   &red_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_GREEN_SIZE, &green_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_BLUE_SIZE,  &blue_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_ALPHA_SIZE, &alpha_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_DEPTH_SIZE, &depth_size);

      if ((red_size == want_red) && (green_size == want_green) && (blue_size == want_blue) && (alpha_size == want_alpha))
      {
         printf("Selected config: R=%d G=%d B=%d A=%d Depth=%d\n", red_size, green_size, blue_size, alpha_size, depth_size);
         break;
      }
   }

   if (config_select == configs)
   {
      printf("No suitable configs found\n");
      return false;
   }

   /*
      Step 5 - Create a surface to draw to.
      Use the config picked in the previous step and the native window
      handle to create a window surface.
   */
   egl_es_surface = eglCreateWindowSurface(egl_display, egl_config[config_select], egl_win, NULL);
   if (egl_es_surface == EGL_NO_SURFACE)
   {
      eglGetError(); /* Clear error */
      egl_es_surface = eglCreateWindowSurface(egl_display, egl_config[config_select], NULL, NULL);
   }

   if (egl_es_surface == EGL_NO_SURFACE)
   {
      printf("eglCreateWindowSurface() failed\n");
      return false;
   }

   /* Only use preserved swap if you need the contents of the frame buffer to be preserved from
    * one frame to the next
    */
   if (config->usePreservingSwap)
   {
      printf("Using preserved swap.  Application will run slowly.\n");
      eglSurfaceAttrib(egl_display, egl_es_surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);
   }

   /*
      Step 6 - Create a ES context.
      EGL has to create a context for OpenGL ES. Our OpenGL ES resources
      like textures will only be valid inside this context (or shared contexts)
   */
   {
      EGLint     ctx_attrib_list[3] =
      {
           EGL_CONTEXT_CLIENT_VERSION, 2, /* For ES2 */
           EGL_NONE
      };

      egl_es_context = eglCreateContext(egl_display, egl_config[config_select], EGL_NO_CONTEXT, ctx_attrib_list);
      if (egl_es_context == EGL_NO_CONTEXT)
      {
         printf("eglCreateContext() failed");
         return false;
      }
   }

   eglMakeCurrent(egl_display, egl_es_surface, egl_es_surface, egl_es_context);

   eglSwapInterval(egl_display, 1);

   return true;
}

void CreateOffscreenVGContext(void)
{
   EGLConfig *egl_config;
   int        config_select    = 0;
   int        configs;

   EGLint vg_attrs[] = {
      EGL_WIDTH, 512,
      EGL_HEIGHT, 512,
      EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
      EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
      EGL_MIPMAP_TEXTURE, EGL_FALSE,
      EGL_VG_ALPHA_FORMAT, EGL_VG_ALPHA_FORMAT_PRE,
      EGL_NONE
   };

   EGLint attr[] =
   {
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_SURFACE_TYPE, EGL_PIXMAP_BIT,
      EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
      EGL_NONE
   };

   egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

   eglBindAPI(EGL_OPENVG_API);

   if (!eglGetConfigs(egl_display, NULL, 0, &configs))
   {
      printf("eglGetConfigs() failed\n");
      return;
   }

   egl_config = (EGLConfig *)alloca(configs * sizeof(EGLConfig));

   if (!eglChooseConfig(egl_display, attr, egl_config, configs, &configs) || (configs == 0))
   {
      printf("eglChooseConfig() failed");
      return;
   }

   for (config_select = 0; config_select < configs; config_select++)
   {
      /* Check that config is an exact match */
      EGLint red_size, green_size, blue_size, alpha_size, depth_size;

      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_RED_SIZE,   &red_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_GREEN_SIZE, &green_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_BLUE_SIZE,  &blue_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_ALPHA_SIZE, &alpha_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_DEPTH_SIZE, &depth_size);

      if ((red_size == 8) && (green_size == 8) && (blue_size == 8) && (alpha_size == 8))
      {
         printf("Selected config: R=%d G=%d B=%d A=%d Depth=%d\n", red_size, green_size, blue_size, alpha_size, depth_size);
         break;
      }
   }

   if (config_select == configs)
   {
      printf("No suitable configs found\n");
      return;
   }

   egl_vg_context = eglCreateContext(egl_display, egl_config[config_select], EGL_NO_CONTEXT, NULL);
   if (egl_vg_context == EGL_NO_CONTEXT)
   {
      printf("eglCreateContext() failed");
      return;
   }

   egl_vg_surface = eglCreatePbufferSurface(egl_display, egl_config[config_select], vg_attrs);
}

bool InitDisplay(float *aspect, const AppConfig *config)
{
   LFPL_NativeWindowInfo   win_info;

   eInitResult res = InitPlatformAndDefaultDisplay(&nexus_display, aspect, config->vpW, config->vpH);
   if (res != eInitSuccess)
      return false;

   /* Register with the platform layer */
   LFPL_RegisterNexusDisplayPlatform(&lfpl_handle, nexus_display);

   win_info.x = config->x;
   win_info.y = config->y;
   win_info.width = config->vpW;
   win_info.height = config->vpH;
   win_info.stretch = config->stretchToFit;
   win_info.clientID = config->clientId;
   native_window = LFPL_CreateNativeWindow(&win_info);

   /* Initialise EGL now we have a 'window' */
   if (!InitEGL(native_window, config))
      return false;

   return true;
}

bool processArgs(int argc, const char *argv[], AppConfig *config)
{
   int   a;

   if (config == NULL)
      return false;

   config->useMultisample    = false;
   config->usePreservingSwap = false;
   config->stretchToFit      = false;
   config->x                 = 0;
   config->y                 = 0;
   config->vpW               = WIDTH;
   config->vpH               = HEIGHT;
   config->bpp               = BPP;
   config->frames            = FRAMES;
   config->clientId          = 0;

   for (a = 1; a < argc; ++a)
   {
      const char  *arg = argv[a];

      if (strcmp(arg, "+m") == 0)
         config->useMultisample = true;
      else if (strcmp(arg, "+p") == 0)
         config->usePreservingSwap = true;
      else if (strcmp(arg, "+s") == 0)
         config->stretchToFit = true;
      else if (strncmp(arg, "d=", 2) == 0)
      {
         if (sscanf(arg, "d=%dx%d", &config->vpW, &config->vpH) != 2)
            return false;
      }
      else if (strncmp(arg, "o=", 2) == 0)
      {
         if (sscanf(arg, "o=%dx%d", &config->x, &config->y) != 2)
            return false;
      }
      else if (strncmp(arg, "bpp=", 4) == 0)
      {
         if (sscanf(arg, "bpp=%d", &config->bpp) != 1)
            return false;
      }
      else if (strncmp(arg, "f=", 2) == 0)
      {
         if (sscanf(arg, "f=%d", &config->frames) != 1)
            return false;
      }
      else if (strncmp(arg, "client=", 7) == 0)
      {
         if (sscanf(arg, "client=%u", &config->clientId) != 1)
            return false;
      }
      else
      {
         return false;
      }
   }

   return true;
}

#ifndef CLIENT_MAIN
#define CLIENT_MAIN main
#endif

int CLIENT_MAIN(int argc, const char** argv)
{
   EGLDisplay   eglDisplay;
   int         frame = 1;

   if (!processArgs(argc, argv, &config))
   {
      const char  *progname = argc > 0 ? argv[0] : "";
      fprintf(stderr, "Usage: %s [+m] [+p] [+s] [d=WxH] [o=XxY] [bpp=16/24/32] [f=frames]\n", progname);
      return 0;
   }

   /* Setup the display and EGL */
   if (InitDisplay(&panelAspect, &config))
   {
      CreateOffscreenVGContext();
      /* Setup the local state for this demo */
      InitGLState();
      InitGLViewPort(config.vpW, config.vpH, panelAspect, config.stretchToFit);

      printf("Rendering ");

      if (config.frames != 0)
         printf("%d frames", config.frames);

      printf(": press CTRL+C to terminate early\n");

      while (config.frames == 0 || frame <= config.frames)
      {
         Display();
         frame++;
      }

      /* Close the local state for this demo */
      TerminateGLState();
   }

   /* Terminate EGL */
   eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglTerminate(eglDisplay);

   LFPL_DestroyNativeWindow(native_window);
   LFPL_UnregisterNexusDisplayPlatform(lfpl_handle);

   /* Close the platform */
   TermPlatform(nexus_display);

   return 0;
}
