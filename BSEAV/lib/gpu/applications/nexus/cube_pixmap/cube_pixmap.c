/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __STDC_CONSTANT_MACROS
#include <stdint.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>  /* For access to the GL_EXT_discard_framebuffer extension */

#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "bkni.h"

#include "esutil.h"
#include "default_nexus.h"

#include "../common/init.h"

#ifndef SINGLE_PROCESS
#error This example will not work in multi-process mode
#endif

#define WIDTH_2D  720
#define HEIGHT_2D 480

#define GL_WIDTH  256
#define GL_HEIGHT 256

#define MAX_OPERATIONS 100

#define RED   8
#define GREEN 8
#define BLUE  8
#define ALPHA 8

/* SIXTY_HZ needs to be defined for B552 LVDS->DVI converter support */
/* #define SIXTY_HZ */

NEXUS_DisplayHandle nexus_display;
EGLNativeDisplayType native_display = 0;
NEXUS_SurfaceHandle nexus_gl_surface;
NEXUS_SurfaceHandle nexus_framebuffer[2]; /* Double buffering */
NEXUS_Graphics2DHandle nexus_gfx;
BKNI_EventHandle blit_fill_event;
BKNI_EventHandle vsync_event;
NEXUS_HeapHandle heap;

NXPL_PlatformHandle nxpl_handle = 0;

PFNGLDISCARDFRAMEBUFFEREXTPROC myDiscardFramebufferExt = NULL;

ESMatrix projection_matrix;
ESMatrix modelview_matrix;
ESMatrix mvp_matrix;

GLint mvp_matrix_loc;
GLint position_loc;
GLint color_loc;
GLint alpha_loc;

GLint program_object;

GLuint vbo[2];

bool secure;

static const GLfloat cube[] = {
   /*          POSITION                            COLOR                */
   1.000000f, 1.000000f, -1.000000f,     1.000000f, 0.000000f, 0.000000f,
   1.000000f, -1.000000f, -1.000000f,    1.000000f, 0.000000f, 0.000000f,
   -1.000000f, -1.000000f, -1.000000f,   1.000000f, 0.000000f, 0.000000f,
   -1.000000f, 1.000000f, -1.000000f,    1.000000f, 0.000000f, 0.000000f,

   -1.000000f, -1.000000f, 1.000000f,    1.000000f, 1.000000f, 0.000000f,
   -1.000000f, 1.000000f, 1.000000f,     1.000000f, 1.000000f, 0.000000f,
   -1.000000f, 1.000000f, -1.000000f,    1.000000f, 1.000000f, 0.000000f,
   -1.000000f, -1.000000f, -1.000000f,   1.000000f, 1.000000f, 0.000000f,

   1.000000f, -1.000000f, 1.000000f,     0.000000f, 0.000000f, 1.000000f,
   1.000000f, 1.000000f, 1.000001f,      0.000000f, 0.000000f, 1.000000f,
   -1.000000f, -1.000000f, 1.000000f,    0.000000f, 0.000000f, 1.000000f,
   -1.000000f, 1.000000f, 1.000000f,     0.000000f, 0.000000f, 1.000000f,

   1.000000f, -1.000000f, -1.000000f,    1.000000f, 0.000000f, 1.000000f,
   1.000000f, 1.000000f, -1.000000f,     1.000000f, 0.000000f, 1.000000f,
   1.000000f, -1.000000f, 1.000000f,     1.000000f, 0.000000f, 1.000000f,
   1.000000f, 1.000000f, 1.000001f,      1.000000f, 0.000000f, 1.000000f,

   1.000000f, 1.000000f, -1.000000f,     0.000000f, 1.000000f, 0.000000f,
   -1.000000f, 1.000000f, -1.000000f,    0.000000f, 1.000000f, 0.000000f,
   1.000000f, 1.000000f, 1.000001f,      0.000000f, 1.000000f, 0.000000f,
   -1.000000f, 1.000000f, 1.000000f,     0.000000f, 1.000000f, 0.000000f,

   1.000000f, -1.000000f, -1.000000f,    0.000000f, 1.000000f, 1.000000f,
   1.000000f, -1.000000f, 1.000000f,     0.000000f, 1.000000f, 1.000000f,
   -1.000000f, -1.000000f, 1.000000f,    0.000000f, 1.000000f, 1.000000f,
   -1.000000f, -1.000000f, -1.000000f,   0.000000f, 1.000000f, 1.000000f,
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

void InitGLState(void)
{
   /* The shaders */
   const char vShaderStr[] =
      "uniform mat4   u_mvpMatrix;    \n"
      "uniform float  u_alpha;        \n"
      "attribute vec4 a_position;     \n"
      "attribute vec4 a_color;        \n"
      "varying vec4   v_color;        \n"
      "                               \n"
      "void main()                    \n"
      "{                              \n"
      "  gl_Position = u_mvpMatrix *  \n"
      "                a_position;    \n"
      "  v_color = a_color;           \n"
      "  v_color.a = u_alpha;         \n"
      "}                              \n";

   const char fShaderStr[] =
      "precision mediump float;    \n"
      "varying vec4 v_color;       \n"
      "                            \n"
      "void main()                 \n"
      "{                           \n"
      "  gl_FragColor = v_color;   \n"
      "}                           \n";

   GLuint     v, f;
   GLint      ret;
   const char *ff;
   const char *vv;
   char       *p, *q;

   glClearDepthf(1.0f);
   glClearColor(1.0f, 1.0f, 1.0f, 1);  /* White background */

   glEnable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);

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
   color_loc = glGetAttribLocation(program_object, "a_color");

   /* Get the uniform locations */
   mvp_matrix_loc = glGetUniformLocation(program_object, "u_mvpMatrix");
   alpha_loc = glGetUniformLocation(program_object, "u_alpha");

   esMatrixLoadIdentity(&projection_matrix);
   esMatrixLoadIdentity(&modelview_matrix);
   esMatrixLoadIdentity(&mvp_matrix);
}

void TerminateGLState(void)
{
   glDeleteProgram(program_object);
   glDeleteBuffers(2, vbo);
}

void InitGLViewPort(unsigned int width, unsigned int height)
{
   glViewport(0, 0, width, height);

   esPerspective(&projection_matrix, 45.0f, (float)width / (float)height, 100, 1000);

   esMatrixLoadIdentity(&modelview_matrix);
   esTranslate(&modelview_matrix, 0, 0, -500);
   esScale(&modelview_matrix, 100, 100, 100);
}

bool InitEGL(unsigned int w, unsigned int h)
{
   EGLDisplay egl_display		= 0;
   EGLSurface egl_surface		= 0;
   EGLContext egl_context		= 0;
   EGLConfig *egl_config;
   EGLint     config_attribs[128];
   EGLint     major_version;
   EGLint     minor_version;
   int        i = 0;
   int        configs;

   /*
      Step 1 - Get the default display.
   */
   egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

   /*
      Step 2 - Initialize EGL.
      EGL has to be initialized with the display obtained in the
      previous step. We cannot use other EGL functions except
      eglGetDisplay and eglGetError before eglInitialize has been
      called.
   */
   if (!eglInitialize(egl_display, &major_version, &minor_version))
   {
      printf("eglInitialize() failed");
      return false;
   }

   /*
      Step 3 - Specify the required configuration attributes.
      An EGL "configuration" describes the pixel format and type of
      surfaces that can be used for drawing.
      For now we just want to use a 16 bit RGB surface that is a
      Window surface, i.e. it will be visible on screen. The list
      has to contain key/value pairs, terminated with EGL_NONE.
   */
   config_attribs[i++] = EGL_RED_SIZE;
   config_attribs[i++] = RED;

   config_attribs[i++] = EGL_GREEN_SIZE;
   config_attribs[i++] = GREEN;

   config_attribs[i++] = EGL_BLUE_SIZE;
   config_attribs[i++] = BLUE;

   config_attribs[i++] = EGL_ALPHA_SIZE;
   config_attribs[i++] = ALPHA;

   config_attribs[i++] = EGL_DEPTH_SIZE;
   config_attribs[i++] = 24;

   config_attribs[i++] = EGL_STENCIL_SIZE;
   config_attribs[i++] = 8;

   config_attribs[i++] = EGL_SURFACE_TYPE;
   config_attribs[i++] = EGL_PIXMAP_BIT;

   config_attribs[i++] = EGL_RENDERABLE_TYPE;
   config_attribs[i++] = EGL_OPENGL_ES2_BIT;

   /* For multisample
   config_attribs[i++] = EGL_SAMPLE_BUFFERS;
   config_attribs[i++] = 1;
   config_attribs[i++] = EGL_SAMPLES;
   config_attribs[i++] = 4;
   */

   /* Terminate the list by adding EGL_NONE */
   config_attribs[i++] = EGL_NONE;

   /*
      Step 3.5 - Get the number of configurations to correctly size the array
      used in step 4
   */
   if (!eglGetConfigs(egl_display, NULL, 0, &configs))
   {
      printf("eglGetConfigs() failed");
      return false;
   }

   egl_config = (EGLConfig *)alloca(configs * sizeof(EGLConfig));

   /*
      Step 4 - Find a config that matches all requirements.
      eglChooseConfig provides a list of all available configurations
      that meet or exceed the requirements given as the second
      argument.
   */
   if (!eglChooseConfig(egl_display, config_attribs, egl_config, configs, &configs) || (configs == 0))
   {
      printf("eglChooseConfig() failed");
      return false;
   }

   for (i = 0; i < configs; i++)
   {
      /*
         Configs with deeper color buffers get returned first by eglChooseConfig.
         Applications may find this counterintuitive, and need to perform additional processing on the list of
         configs to find one best matching their requirements. For example, specifying RGBA depths of 565
         could return a list whose first config has a depth of 888.
      */

      /* If we asked for a 565, but all the 8888 match first, we need to pick an actual 565 buffer */
      EGLint red_size, green_size, blue_size;
      eglGetConfigAttrib(egl_display, egl_config[i], EGL_RED_SIZE, &red_size);
      eglGetConfigAttrib(egl_display, egl_config[i], EGL_GREEN_SIZE, &green_size);
      eglGetConfigAttrib(egl_display, egl_config[i], EGL_BLUE_SIZE, &blue_size);

      if (RED == 5 && GREEN == 6 && BLUE == 5)
      {
         if ((red_size == 5) && (green_size == 6) && (blue_size == 5))
            break;
      }
      else
      {
         if ((red_size == 8) && (green_size == 8) && (blue_size == 8))
            break;
      }
   }
   configs = i;

   /*
      Step 5 - Create a surface to draw to.
      Use the config picked in the previous step and the native window
      handle to create a window surface.
   */


   /*
      Make a Nexus surface to render into.
   */
   NEXUS_SurfaceCreateSettings surfSettings;
   NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);
   surfSettings.compatibility.graphicsv3d = true;
   surfSettings.width = w;
   surfSettings.height = h;
   surfSettings.pixelFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;
   surfSettings.heap = NEXUS_Platform_GetFramebufferHeap(secure ?
         NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE : NEXUS_OFFSCREEN_SURFACE);
   if (!surfSettings.heap)
   {
      printf("No heap\n");
      return false;
   }
   nexus_gl_surface = NEXUS_Surface_Create(&surfSettings);
   if (!nexus_gl_surface)
   {
      printf("NEXUS_Surface_Create() failed\n");
      return false;
   }

   egl_surface = eglCreatePixmapSurface(egl_display, egl_config[configs], nexus_gl_surface, NULL);
   if (egl_surface == EGL_NO_SURFACE)
   {
      printf("eglCreatePixmapSurface() failed\n");
      return false;
   }

   /*
      Step 6 - Create a context.
      EGL has to create a context for OpenGL ES. Our OpenGL ES resources
      like textures will only be valid inside this context (or shared contexts)
   */
   {
      const int   NUM_ATTRIBS = 5;
      EGLint      *attr = (EGLint *)malloc(NUM_ATTRIBS * sizeof(EGLint));
      int         i = 0;

#ifdef EGL_PROTECTED_CONTENT_EXT
      if (secure)
      {
         attr[i++] = EGL_PROTECTED_CONTENT_EXT; attr[i++] = EGL_TRUE;
      }
#endif

      attr[i++] = EGL_CONTEXT_CLIENT_VERSION; attr[i++] = 2;
      attr[i++] = EGL_NONE;

      egl_context = eglCreateContext(egl_display, egl_config[configs], EGL_NO_CONTEXT, attr);
      if (egl_context == EGL_NO_CONTEXT)
      {
         printf("eglCreateContext() failed");
         return false;
      }

      free(attr);
   }

   /*
      Step 7 - Bind the context to the current thread and use our
      window surface for drawing and reading.
      We need to specify a surface that will be the target of all
      subsequent drawing operations, and one that will be the source
      of read operations. They can be the same surface.
   */
   eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

   return true;
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void blit_or_fill_finished(void *data, int unused)
{
   BSTD_UNUSED(unused);
   BKNI_SetEvent((BKNI_EventHandle)data);
}

void vsync(void *data, int unused)
{
   BSTD_UNUSED(unused);
   BKNI_SetEvent((BKNI_EventHandle)data);
}

void Display(unsigned int xpos, unsigned int ypos, float bg_alpha, float cube_alpha, unsigned int blit_on_top)
{
   NEXUS_Error rc;
   NEXUS_Graphics2DBlitSettings blit_settings;
   NEXUS_Graphics2DFillSettings fillSettings;
   GLenum buffers[2] = { GL_DEPTH_EXT, GL_STENCIL_EXT };

   static int cur_fb = 0;

   /* Wait for previous vsync to trigger */
   BKNI_WaitForEvent(vsync_event, 0xffffffff);

   /* Fill dest surface with something slightly more interesting than a flat color */
   /* Should run in parallel with 3D rendering */
   NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
   fillSettings.surface = nexus_framebuffer[cur_fb];
   fillSettings.rect.width = WIDTH_2D;
   fillSettings.rect.height = HEIGHT_2D;
   fillSettings.color = 0xFF101010;
   NEXUS_Graphics2D_Fill(nexus_gfx, &fillSettings);

   fillSettings.rect.x = WIDTH_2D / 6;
   fillSettings.rect.y = HEIGHT_2D / 6;
   fillSettings.rect.width = WIDTH_2D - (WIDTH_2D / 3);
   fillSettings.rect.height = HEIGHT_2D - (HEIGHT_2D / 3);
   fillSettings.color = 0xFF303030;
   NEXUS_Graphics2D_Fill(nexus_gfx, &fillSettings);

   fillSettings.rect.x = WIDTH_2D / 4;
   fillSettings.rect.y = HEIGHT_2D / 4;
   fillSettings.rect.width = WIDTH_2D - (WIDTH_2D / 2);
   fillSettings.rect.height = HEIGHT_2D - (HEIGHT_2D / 2);
   fillSettings.color = 0xFF404040;
   NEXUS_Graphics2D_Fill(nexus_gfx, &fillSettings);

   /* Rotate the cube */
   esRotate(&modelview_matrix, 1.0f, 1, 0, 0);
   esRotate(&modelview_matrix, 0.5f, 0, 1, 0);

   /* Compute the final MVP by multiplying the model-view and perspective matrices together */
   esMatrixMultiply(&mvp_matrix, &modelview_matrix, &projection_matrix);

   /* Clear all the buffers we asked for during config to ensure fast-path */
   glClearColor(1.0f, 1.0f, 1.0f, bg_alpha);
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(program_object);

   /* Enable cube array */
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
   glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), BUFFER_OFFSET(0));
   glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
   glEnableVertexAttribArray(position_loc);
   glEnableVertexAttribArray(color_loc);

   /* Load the MVP matrix */
   glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)&mvp_matrix.m[0][0]);

   /* Set the overall alpha for the cube */
   glUniform1f(alpha_loc, cube_alpha);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

   /* Finally draw the elements */
   glDrawElements(GL_TRIANGLES, sizeof(cube_idx) / sizeof(GLushort), GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

   /* Mark the depth & stencil as discarded, so they don't need to be preserved during glFinish() */
   if (myDiscardFramebufferExt != NULL)
      myDiscardFramebufferExt(GL_FRAMEBUFFER, 2, buffers);

   /* Finish 3D rendering - blocks until done. After this our surface will have the result. */
   glFinish();

   /* Blit 3D results into current framebuffer */
   NEXUS_Graphics2D_GetDefaultBlitSettings(&blit_settings);
   blit_settings.colorOp = NEXUS_BlitColorOp_eUseSourceAlpha;
   blit_settings.alphaOp = NEXUS_BlitAlphaOp_eCopyDest;
   blit_settings.source.surface = nexus_gl_surface;
   blit_settings.output.surface = nexus_framebuffer[cur_fb];
   blit_settings.output.rect.x = xpos;
   blit_settings.output.rect.y = ypos;
   blit_settings.output.rect.width = GL_WIDTH;
   blit_settings.output.rect.height = GL_HEIGHT;
   blit_settings.dest.surface = nexus_framebuffer[cur_fb];
   blit_settings.dest.rect.x = xpos;
   blit_settings.dest.rect.y = ypos;
   blit_settings.dest.rect.width = GL_WIDTH;
   blit_settings.dest.rect.height = GL_HEIGHT;

   NEXUS_Graphics2D_Blit(nexus_gfx, &blit_settings);

   if (blit_on_top)
   {
      /* Blit something on top of the 3D, just to show that it works */
      NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
      fillSettings.surface = nexus_framebuffer[cur_fb];
      fillSettings.colorOp = NEXUS_FillOp_eBlend;
      fillSettings.rect.x = WIDTH_2D / 8;
      fillSettings.rect.y = HEIGHT_2D / 8;
      fillSettings.rect.width = WIDTH_2D - (WIDTH_2D / 4);
      fillSettings.rect.height = HEIGHT_2D - (HEIGHT_2D / 4);
      fillSettings.color = 0x80A00000;
      NEXUS_Graphics2D_Fill(nexus_gfx, &fillSettings);
   }

   rc = NEXUS_Graphics2D_Checkpoint(nexus_gfx, NULL);
   if (rc == NEXUS_GRAPHICS2D_QUEUED)
   {
      /* Wait for the blit to complete before continuing */
      BKNI_WaitForEvent(blit_fill_event, 0xffffffff);
   }

   /* Switch the display to use this framebuffer */
   NEXUS_Display_SetGraphicsFramebuffer(nexus_display, nexus_framebuffer[cur_fb]);
   cur_fb++;
   if (cur_fb > 1)
      cur_fb = 0;
}

void InitDisplay(unsigned int width, unsigned int height, bool secure)
{
   NEXUS_SurfaceCreateSettings   create_settings;
   NEXUS_Graphics2DOpenSettings  openSettings;
   NEXUS_GraphicsSettings        graphicsSettings;
   NEXUS_Graphics2DSettings      gfxSettings;

   if (!InitPlatform(secure))
   {
      fprintf(stderr, "Error: This example was not designed to run in multi-process mode\n");
      exit(0);
   }

   nexus_display = OpenDisplay((NEXUS_VideoFormat)0, width, height, secure);
   InitCompositeOutput(nexus_display, width, height);
   InitComponentOutput(nexus_display);
   InitHDMIOutput(nexus_display);

   /* Init the NXPL platform layer, BUT since we're only rendering into a pixmap, we should not
    * send the nexus_display in, as we want to keep exclusive access to it here. This will be fine
    * providing we don't call eglSwapBuffers(), which we wouldn't work for pixmaps anyway. */
   NXPL_RegisterNexusDisplayPlatform(&nxpl_handle, NULL);

   heap = NEXUS_Platform_GetFramebufferHeap(secure ? NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE : 0);

   /* Initialise EGL with a smaller 3D window */
   InitEGL(GL_WIDTH, GL_HEIGHT);

   /* Create a base surface to use as the framebuffer */
   NEXUS_Surface_GetDefaultCreateSettings(&create_settings);
   create_settings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
   create_settings.width = width;
   create_settings.height = height;
   create_settings.heap = heap;
   nexus_framebuffer[0] = NEXUS_Surface_Create(&create_settings);

   NEXUS_Surface_GetDefaultCreateSettings(&create_settings);
   create_settings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
   create_settings.width = width;
   create_settings.height = height;
   create_settings.heap = heap;
   nexus_framebuffer[1] = NEXUS_Surface_Create(&create_settings);

   /* Open Nexus 2D */
   NEXUS_Graphics2D_GetDefaultOpenSettings(&openSettings);
   openSettings.preAllocPacketMemory = true;
   openSettings.maxOperations = MAX_OPERATIONS;
   openSettings.secure = secure;
   nexus_gfx = NEXUS_Graphics2D_Open(0, &openSettings);

   /* Set the vsync initially triggered to prevent block */
   BKNI_CreateEvent(&vsync_event);
   BKNI_SetEvent(vsync_event);

   NEXUS_Display_GetGraphicsSettings(nexus_display, &graphicsSettings);
   graphicsSettings.enabled = true;
   graphicsSettings.position.width = width;
   graphicsSettings.position.height = height;
   graphicsSettings.clip.width = width;
   graphicsSettings.clip.height = height;
   graphicsSettings.frameBufferCallback.callback = vsync;
   graphicsSettings.frameBufferCallback.context = vsync_event;
   graphicsSettings.secure = secure;
   NEXUS_Display_SetGraphicsSettings(nexus_display, &graphicsSettings);

   /* Setup a blit finished callback */
   BKNI_CreateEvent(&blit_fill_event);

   NEXUS_Graphics2D_GetSettings(nexus_gfx, &gfxSettings);
   gfxSettings.checkpointCallback.callback = blit_or_fill_finished;
   gfxSettings.checkpointCallback.context = blit_fill_event;
   NEXUS_Graphics2D_SetSettings(nexus_gfx, &gfxSettings);
}

void MoveWindow(float *px, float *py)
{
   static float vx = 2.0f;
   static float vy = 1.0f;

   *px += vx;
   *py += vy;

   if (*px + GL_WIDTH >= WIDTH_2D)
   {
      vx = -vx;
      *px = WIDTH_2D - GL_WIDTH - 1;
   }
   else if (*px <= 0.0f)
   {
      vx = -vx;
      *px = 0.0f;
   }

   if (*py + GL_HEIGHT >= HEIGHT_2D)
   {
      vy = -vy;
      *py = HEIGHT_2D - GL_HEIGHT - 1;
   }
   else if (*py <= 0.0f)
   {
      vy = -vy;
      *py = 0.0f;
   }
}

int main(int argc, char** argv)
{
   NEXUS_GraphicsSettings graphicsSettings;
   EGLDisplay   eglDisplay;
   unsigned int frame = 1;
   float        px, py;
   const char  *extStr = NULL;
   int          a;

   secure = false;
   for (a = 1; a < argc; ++a)
   {
      const char  *arg = argv[a];

      if (strcmp(arg, "+secure") == 0)
         secure = true;
      else
      {
         const char  *progname = argc > 0 ? argv[0] : "";
         fprintf(stderr, "Usage: %s [+secure]\n", progname);
         exit(0);
      }
   }

#ifndef EGL_PROTECTED_CONTENT_EXT
   if (secure)
      printf("+secure selected, but headers not available in this driver version. defaulting off\n");
   secure = false;
#endif

   /* Setup the display and EGL */
   InitDisplay(WIDTH_2D, HEIGHT_2D, secure);

   /* Setup the local state for this demo */
   InitGLState();
   InitGLViewPort(GL_WIDTH, GL_HEIGHT);

   /* Check for the discard extension to help performance */
   extStr = (const char *)glGetString(GL_EXTENSIONS);
   if (extStr != NULL && strstr(extStr, "GL_EXT_discard_framebuffer") != NULL)
      myDiscardFramebufferExt = (PFNGLDISCARDFRAMEBUFFEREXTPROC)eglGetProcAddress("glDiscardFramebufferEXT");

   printf("Press CTRL+C to terminate early\n");

   px = py = 0.0f;

   printf("OpenGL-ES pixmap composited on Nexus fills (background alpha = 1.0, cube alpha = 1.0) ...\n");
   for (frame = 0; frame < 255; frame++)
   {
      MoveWindow(&px, &py);
      Display((unsigned int)px, (unsigned int)py, 1.0f, 1.0f, 0);
   }

   printf("OpenGL-ES pixmap composited on Nexus fills (background alpha = 0.0, cube alpha = 1.0) ...\n");
   for (frame = 0; frame < 255; frame++)
   {
      MoveWindow(&px, &py);
      Display((unsigned int)px, (unsigned int)py, 0.0f, 1.0f, 0);
   }

   printf("OpenGL-ES pixmap composited on Nexus fills (background alpha = 0.0, cube alpha = varying) ...\n");
   for (frame = 0; frame < 255; frame++)
   {
      MoveWindow(&px, &py);
      Display((unsigned int)px, (unsigned int)py, 0.0f, (float)frame / 255.0f, 0);
   }

   printf("OpenGL-ES pixmap composited on Nexus fills (background alpha = 1.0, cube alpha = 0.0) ...\n");
   for (frame = 0; frame < 255; frame++)
   {
      MoveWindow(&px, &py);
      Display((unsigned int)px, (unsigned int)py, 1.0f, 0.0f, 0);
   }

   printf("OpenGL-ES pixmap composited on Nexus fills, filled on top with Nexus ...\n");
   for (frame = 0; frame < 500; frame++)
   {
      MoveWindow(&px, &py);
      Display((unsigned int)px, (unsigned int)py, 0.0f, 1.0f, 1);
   }

   /* Close the local state for this demo */
   TerminateGLState();

   eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

   /* Remove the framebuffer surface so we can delete it */
   NEXUS_Display_GetGraphicsSettings(nexus_display, &graphicsSettings);
   graphicsSettings.enabled = false;
   NEXUS_Display_SetGraphicsSettings(nexus_display, &graphicsSettings);

   /* Close the graphics 2D */
   NEXUS_Graphics2D_Close(nexus_gfx);

   /* Destroy the nexus surfaces */
   NEXUS_Surface_Destroy(nexus_framebuffer[0]);
   NEXUS_Surface_Destroy(nexus_framebuffer[1]);

   /* Terminate EGL */
   eglTerminate(eglDisplay);

   NEXUS_Surface_Destroy(nexus_gl_surface);

   NXPL_UnregisterNexusDisplayPlatform(nxpl_handle);

   /* Close the Nexus display */
   NEXUS_Display_Close(nexus_display);

   /* Free resources */
   BKNI_DestroyEvent(blit_fill_event);
   BKNI_DestroyEvent(vsync_event);

   /* Close the platform */
   NEXUS_Platform_Uninit();

   return 0;
}
