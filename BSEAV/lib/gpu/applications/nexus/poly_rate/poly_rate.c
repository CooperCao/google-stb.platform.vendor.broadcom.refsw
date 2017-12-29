/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <alloca.h>
#include <sys/time.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include "nexus_platform.h"
#include "nexus_display.h"
#include "bkni.h"
#include "default_nexus.h"

#include "../common/init.h"

unsigned int vpX = 0;
unsigned int vpY = 0;
unsigned int vpW = 64;
unsigned int vpH = 64;
float        triSidePixels = 1.0f;
unsigned int clientId = 0;

bool useMultisample = false;
bool stretchToFit = false;
bool secure = false;

NEXUS_DISPLAYHANDLE  nexus_display;
EGLNativeDisplayType native_display = 0;
void                 *native_window = 0;
NXPL_PlatformHandle  nxpl_handle = 0;
EGLDisplay           egl_display = 0;
EGLSurface           egl_surface = 0;
EGLContext           egl_context = 0;

GLuint vbo[2];
GLint program_object;
GLint position_loc;

typedef GLubyte           INDX_TYPE;
#define INDX_ELEM_TYPE    GL_UNSIGNED_BYTE

INDX_TYPE *many_quad_idxs;
GLfloat *quad;

static const GLfloat squad[] = {
   /*          POSITION               */
   1.00000f, 1.00000f,  0.00000f,
   1.00000f, 0.00000f,  0.00000f,
   0.00000f, 0.00000f,  0.00000f,
   0.00000f, 1.00000f,  0.00000f,
};

static const INDX_TYPE quad_idx[] = {
   0, 1, 2, 3,
};

#define NUM_VERTS 400000

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void InitGLState(void)
{
   /* The shaders */
   const char vShaderStr[] =
      "attribute vec4 a_position;                \n"
      "void main()                               \n"
      "{                                         \n"
      "  gl_Position = a_position;               \n"
      "}                                         \n";

   const char fShaderStr[] =
      "precision mediump float;                  \n"
      "void main()                               \n"
      "{                                         \n"
      "  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
      "}                                         \n";

   GLuint     v, f;
   GLint      ret;
   const char *ff;
   const char *vv;
   char       *p, *q;
   unsigned int i, j;

   glClearDepthf(1.0f);
   glClearColor(0.2f, 0.2f, 0.2f, 1);  /* Gray background */

   glEnable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);

   many_quad_idxs = (INDX_TYPE*)malloc(NUM_VERTS * sizeof(INDX_TYPE));
   quad = (GLfloat*)malloc(sizeof(GLfloat) * 4 * 3);

   for (i = 0; i < NUM_VERTS / 4; i++)
      memcpy(&many_quad_idxs[i * 4], quad_idx, 4 * sizeof(INDX_TYPE));

   for (j = 0; j < 4; j++)
   {
      quad[j*3] = squad[j*3] * triSidePixels / ((float)vpW * 0.5f);
      quad[j*3+1] = squad[j*3+1] * triSidePixels / ((float)vpH * 0.5f);
      quad[j*3+2] = squad[j*3+2];
   }

   /* Create vertex buffer objects */
   glGenBuffers(2, vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, quad, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDX_TYPE) * NUM_VERTS, many_quad_idxs, GL_STATIC_DRAW);

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
}

void CleanUp(void)
{
   glDeleteProgram(program_object);
   glDeleteBuffers(2, vbo);

   free(many_quad_idxs);
   free(quad);
}

void InitGLViewPort(unsigned int width, unsigned int height)
{
   vpW = width;
   vpH = height;

   glViewport(0, 0, width, height);
}

unsigned int GetTimeNowMs(void)
{
   struct timeval curTime;
   unsigned int nowMs;

   gettimeofday(&curTime, NULL);
   nowMs = curTime.tv_usec / 1000;
   nowMs += curTime.tv_sec * 1000;

   return nowMs;
}

void TestTrisPerSecond(void)
{
   unsigned int start;
   unsigned int end;
   unsigned int loops = 100;
   unsigned int i;
   unsigned int tri_count = 0;
   float        seconds = 0.0f;

   printf("Testing tris per second ...\n");

   start = GetTimeNowMs();

   for (i = 0; i < loops; i++)
   {
      /* Clear all the buffers we asked for during config to ensure fast-path */
      glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(program_object);

      /* Enable cube array */
      glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
      glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
      glEnableVertexAttribArray(position_loc);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

      /* Finally draw the elements */
      glDrawElements(GL_TRIANGLE_STRIP, NUM_VERTS, INDX_ELEM_TYPE, BUFFER_OFFSET(0));

      tri_count += (NUM_VERTS - 2);

      eglSwapBuffers(egl_display, egl_surface);
   }
   glFinish();

   end = GetTimeNowMs();

   seconds = (float)(end - start) / 1000.0f;

   printf("%d triangles drawn in %f seconds\n", tri_count, seconds);
   printf("%f triangles per second\n", (float)tri_count / seconds);
}

int InitEGL(NativeWindowType egl_win)
{
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
      return 0;
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
   config_attribs[i++] = 5;

   config_attribs[i++] = EGL_GREEN_SIZE;
   config_attribs[i++] = 6;

   config_attribs[i++] = EGL_BLUE_SIZE;
   config_attribs[i++] = 5;

   config_attribs[i++] = EGL_ALPHA_SIZE;
   config_attribs[i++] = 0;

   config_attribs[i++] = EGL_DEPTH_SIZE;
   config_attribs[i++] = 24;

   config_attribs[i++] = EGL_STENCIL_SIZE;
   config_attribs[i++] = 0;

   config_attribs[i++] = EGL_SURFACE_TYPE;
   config_attribs[i++] = EGL_WINDOW_BIT;

   config_attribs[i++] = EGL_RENDERABLE_TYPE;
   config_attribs[i++] = EGL_OPENGL_ES2_BIT;

   /* Terminate the list by adding EGL_NONE */
   config_attribs[i++] = EGL_NONE;

   /*
      Step 3.5 - Get the number of configurations to correctly size the array
      used in step 4
   */
   if (!eglGetConfigs(egl_display, NULL, 0, &configs))
   {
      printf("eglGetConfigs() failed");
      return 0;
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
      return 0;
   }

   for (i = 0; i < configs; i++)
   {
      /*
         Configs with deeper color buffers get returned first by eglChooseConfig.
         Applications may find this counterintuitive, and need to perform additional processing on the list of
         configs to find one best matching their requirements. For example, specifying RGBA depths of 565
         could return a list whose first config has a depth of 888.
      */

      /* We asked for a 565, but all the 8888 match first, we need to pick an actual 565 buffer */
      EGLint red_size, green_size, blue_size;
      eglGetConfigAttrib(egl_display, egl_config[i], EGL_RED_SIZE, &red_size);
      eglGetConfigAttrib(egl_display, egl_config[i], EGL_GREEN_SIZE, &green_size);
      eglGetConfigAttrib(egl_display, egl_config[i], EGL_BLUE_SIZE, &blue_size);

      if ((red_size == 5) && (green_size == 6) && (blue_size == 5))
         break;
   }
   configs = i;

   /*
      Step 5 - Create a surface to draw to.
      Use the config picked in the previous step and the native window
      handle to create a window surface.
   */
   {
      const int   NUM_ATTRIBS = 3;
      EGLint      *attr = (EGLint *)malloc(NUM_ATTRIBS * sizeof(EGLint));
      int         i = 0;

#ifdef EGL_PROTECTED_CONTENT_EXT
      if (secure)
      {
         attr[i++] = EGL_PROTECTED_CONTENT_EXT; attr[i++] = EGL_TRUE;
      }
#endif
      attr[i++] = EGL_NONE;

      egl_surface = eglCreateWindowSurface(egl_display, egl_config[configs], egl_win, attr);
   }

   if (egl_surface == EGL_NO_SURFACE)
   {
      eglGetError(); /* Clear error */
      egl_surface = eglCreateWindowSurface(egl_display, egl_config[configs], NULL, NULL);
   }

   if (egl_surface == EGL_NO_SURFACE)
   {
      printf("eglCreateWindowSurface() failed\n");
      return 0;
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

   /*
      Step 8 - This is a benchmark so swap must be 0, otherwise we are only
      measuring the vblank
   */
   eglSwapInterval(egl_display, 0);

   return 1;
}

void InitDisplay(unsigned int width, unsigned int height, bool secure)
{
   NXPL_NativeWindowInfoEXT   win_info;
   float                   aspect;

   eInitResult res = InitPlatformAndDefaultDisplay(&nexus_display, &aspect, width, height, secure);
   if (res != eInitSuccess)
      return;

   /* Register with the platform layer */
   NXPL_RegisterNexusDisplayPlatform(&nxpl_handle, nexus_display);

   NXPL_GetDefaultNativeWindowInfoEXT(&win_info);

   win_info.x = vpX;
   win_info.y = vpY;
   win_info.width = width;
   win_info.height = height;
   win_info.stretch = stretchToFit;
   win_info.clientID = clientId;

   native_window = NXPL_CreateNativeWindowEXT(&win_info);

   /* Initialise EGL now we have a 'window' */
   InitEGL(native_window);
}

void usageExit(const char *appName)
{
   fprintf(stderr, "Usage: %s options\n"
      "Options :\n"
      "+m   use multi-sampling\n"
      "+s   stretch to fit display panel\n"
      "d=WWWxHHH       set window size\n"
      "o=XXXxYYY  set window origin\n"
      "pix=N           size of the side of the triangle\n"
      "+secure   run in a secure GL context\n",
      appName);

   exit(1);
}

bool processArg(const char *arg)
{
   if (!strcmp(arg, "+m"))
      useMultisample = true;
   else if (!strcmp(arg, "+s"))
      stretchToFit = true;
   else if (!strncmp(arg, "d=", 2))
   {
      if (sscanf(arg, "d=%dx%d", (int*)&vpW, (int*)&vpH) != 2)
         return false;
   }
   else if (!strncmp(arg, "o=", 2))
   {
      if (sscanf(arg, "o=%dx%d", (int*)&vpX, (int*)&vpY) != 2)
         return false;
   }
   else if (!strncmp(arg, "pix=", 3))
   {
      if (sscanf(arg, "pix=%f", (float*)&triSidePixels) != 1)
         return false;
   }
   else if (strncmp(arg, "client=", 7) == 0)
   {
      if (sscanf(arg, "client=%u", &clientId) != 1)
         return false;
   }
   else if (!strcmp(arg, "+secure"))
      secure = true;
   else
      return false;

   return true;
}

#ifndef CLIENT_MAIN
#define CLIENT_MAIN main
#endif

int CLIENT_MAIN(int argc, const char** argv)
{
   int a;

   for (a = 1; a < argc; a++)
      if (!processArg(argv[a]))
         usageExit(argv[0]);

#ifndef EGL_PROTECTED_CONTENT_EXT
   if (secure)
      printf("+secure selected, but headers not available in this driver version. defaulting off\n");
   secure = false;
#endif

   /* Setup the display and EGL */
   InitDisplay(vpW, vpH, secure);

   /* Setup the local state for this demo */
   InitGLState();
   InitGLViewPort(vpW, vpH);

   TestTrisPerSecond();

   /* Close the local state for this demo */
   CleanUp();

   /* Terminate EGL */
   eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroySurface(egl_display, egl_surface);
   eglDestroyContext(egl_display, egl_context);
   eglTerminate(egl_display);

   NXPL_DestroyNativeWindow(native_window);
   NXPL_UnregisterNexusDisplayPlatform(nxpl_handle);

   /* Close the platform */
   TermPlatform(nexus_display);

   return 0;
}
