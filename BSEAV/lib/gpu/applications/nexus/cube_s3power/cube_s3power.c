/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include "esutil.h"
#include "default_nexus.h"

#include "../common/init.h"

#include "nexus_ir_input.h"

typedef struct
{
   bool     useMultisample;
   bool     stretchToFit;
   int      x;
   int      y;
   int      vpW;
   int      vpH;
   int      bpp;
   int      frames;
   unsigned clientId;
   bool     secure;
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

NXPL_PlatformHandle nxpl_handle = 0;

ESMatrix projection_matrix;
ESMatrix modelview_matrix;
ESMatrix mvp_matrix;

GLint mvp_matrix_loc;
GLint position_loc;
GLint color_loc;

GLint program_object;

GLuint vbo[2];

BKNI_EventHandle event_power_down;
NEXUS_IrInputHandle ir_handle;

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

void irCallback(void *pParam, int iParam)
{
   size_t numEvents = 1;
   NEXUS_Error rc = 0;
   bool overflow;
   NEXUS_IrInputHandle irHandle = *(NEXUS_IrInputHandle *)pParam;
   BSTD_UNUSED(iParam);
   while (numEvents && !rc)
   {
      NEXUS_IrInputEvent irEvent;
      NEXUS_PlatformStandbySettings nexusStandbySettings;
      rc = NEXUS_IrInput_GetEvents(irHandle, &irEvent, 1, &numEvents, &overflow);
      NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
      if (numEvents && !irEvent.repeat)
      {
         if (irEvent.code == 0xf50aff00)
         {
            printf("Received Power Command from IR\n");
            BKNI_SetEvent(event_power_down);
         }
      }
   }
}

void InitGLState(void)
{
   /* The shaders */
   const char vShaderStr[] =
      "uniform mat4   u_mvpMatrix;               \n"
      "attribute vec4 a_position;                \n"
      "attribute vec4 a_color;                   \n"
      "varying vec4   v_color;                   \n"
      "                                          \n"
      "void main()                               \n"
      "{                                         \n"
      "  gl_Position = u_mvpMatrix * a_position; \n"
      "  v_color = a_color;                      \n"
      "}                                         \n";

   const char fShaderStr[] =
      "precision mediump float;                  \n"
      "varying vec4 v_color;                     \n"
      "                                          \n"
      "void main()                               \n"
      "{                                         \n"
      "  gl_FragColor = v_color;                 \n"
      "}                                         \n";

   GLuint     v, f;
   GLint      ret;
   const char *ff;
   const char *vv;
   char       *p, *q;

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
   color_loc = glGetAttribLocation(program_object, "a_color");

   /* Get the uniform locations */
   mvp_matrix_loc = glGetUniformLocation(program_object, "u_mvpMatrix");

   esMatrixLoadIdentity(&projection_matrix);
   esMatrixLoadIdentity(&modelview_matrix);
   esMatrixLoadIdentity(&mvp_matrix);

   esMatrixLoadIdentity(&modelview_matrix);
   esTranslate(&modelview_matrix, 0, 0, -500);
   esScale(&modelview_matrix, 100, 100, 100);
}

void TerminateGLState(void)
{
   glDeleteProgram(program_object);
   glDeleteBuffers(2, vbo);
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
   eglQuerySurface(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW), EGL_WIDTH, &w);
   eglQuerySurface(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW), EGL_HEIGHT, &h);

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
   if (BKNI_WaitForEvent(event_power_down, 0) == BERR_SUCCESS)
   {
      NEXUS_Error rc;
      NEXUS_PlatformStandbySettings nexusStandbySettings;
      NEXUS_PlatformStandbyStatus nexusStandbyStatus;

      printf("Enter S3 mode\n");

      NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
      nexusStandbySettings.mode = NEXUS_PlatformStandbyMode_eDeepSleep;
      nexusStandbySettings.wakeupSettings.ir = true;
#if NEXUS_HAS_CEC
      nexusStandbySettings.wakeupSettings.cec = true;
#endif
      nexusStandbySettings.wakeupSettings.gpio = true;
      nexusStandbySettings.wakeupSettings.timeout = 0;
      rc = NEXUS_Platform_SetStandbySettings(&nexusStandbySettings);
      BDBG_ASSERT(!rc);

      printf("S3 Warm Boot - press any remote control key to restore\n");
      system("echo mem > /sys/power/state");

      NEXUS_Platform_GetStandbyStatus(&nexusStandbyStatus);

      printf("S3 Wake up Status\n"
             "Standby Status : \n"
             "IR      : %d\n"
             "UHF     : %d\n"
             "XPT     : %d\n"
             "CEC     : %d\n"
             "GPIO    : %d\n"
             "Timeout : %d\n\n",
         nexusStandbyStatus.wakeupStatus.ir,
         nexusStandbyStatus.wakeupStatus.uhf,
         nexusStandbyStatus.wakeupStatus.transport,
         nexusStandbyStatus.wakeupStatus.cec,
         nexusStandbyStatus.wakeupStatus.gpio,
         nexusStandbyStatus.wakeupStatus.timeout);

      printf("Resuming Normal Mode\n");

      NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
      nexusStandbySettings.mode = NEXUS_PlatformStandbyMode_eOn;
      nexusStandbySettings.openFrontend = false;
      rc = NEXUS_Platform_SetStandbySettings(&nexusStandbySettings);
      BDBG_ASSERT(!rc);

      NEXUS_IrInput_DisableDataFilter(ir_handle);
   }

   Resize();

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
   glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), BUFFER_OFFSET(0));
   glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
   glEnableVertexAttribArray(position_loc);
   glEnableVertexAttribArray(color_loc);

   /* Load the MVP matrix */
   glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)&mvp_matrix.m[0][0]);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

   /* Finally draw the elements */
   glDrawElements(GL_TRIANGLES, sizeof(cube_idx) / sizeof(GLushort), GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

   /* Display the framebuffer */
   eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_READ));
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
   EGLSurface egl_surface      = 0;
   EGLContext egl_context      = 0;
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
      attr[i++] = EGL_RENDERABLE_TYPE; attr[i++] = EGL_OPENGL_ES2_BIT;

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
   {
      const int   NUM_ATTRIBS = 3;
      EGLint      *attr = (EGLint *)malloc(NUM_ATTRIBS * sizeof(EGLint));
      int         i = 0;

#ifdef EGL_PROTECTED_CONTENT_EXT
      if (config->secure)
      {
         attr[i++] = EGL_PROTECTED_CONTENT_EXT; attr[i++] = EGL_TRUE;
      }
#endif
      attr[i++] = EGL_NONE;

      egl_surface = eglCreateWindowSurface(egl_display, egl_config[config_select], egl_win, attr);
   }

   if (egl_surface == EGL_NO_SURFACE)
   {
      eglGetError(); /* Clear error */
      egl_surface = eglCreateWindowSurface(egl_display, egl_config[config_select], NULL, NULL);
   }

   if (egl_surface == EGL_NO_SURFACE)
   {
      printf("eglCreateWindowSurface() failed\n");
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
      if (config->secure)
      {
         attr[i++] = EGL_PROTECTED_CONTENT_EXT; attr[i++] = EGL_TRUE;
      }
#endif

      attr[i++] = EGL_CONTEXT_CLIENT_VERSION; attr[i++] = 2;
      attr[i++] = EGL_NONE;

      egl_context = eglCreateContext(egl_display, egl_config[config_select], EGL_NO_CONTEXT, attr);
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

   eglSwapInterval(egl_display, 1);

   return true;
}

bool InitDisplay(float *aspect, const AppConfig *config)
{
   NXPL_NativeWindowInfoEXT   win_info;

   eInitResult res = InitPlatformAndDefaultDisplay(&nexus_display, aspect, config->vpW, config->vpH, config->secure);
   if (res != eInitSuccess)
      return false;

   /* Register with the platform layer */
   NXPL_RegisterNexusDisplayPlatform(&nxpl_handle, nexus_display);

   NXPL_GetDefaultNativeWindowInfoEXT(&win_info);

   win_info.x = config->x;
   win_info.y = config->y;
   win_info.width = config->vpW;
   win_info.height = config->vpH;
   win_info.stretch = config->stretchToFit;
   win_info.clientID = config->clientId;

   native_window = NXPL_CreateNativeWindowEXT(&win_info);

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
   config->stretchToFit      = false;
   config->x                 = 0;
   config->y                 = 0;
   config->vpW               = WIDTH;
   config->vpH               = HEIGHT;
   config->bpp               = BPP;
   config->frames            = FRAMES;
   config->clientId          = 0;
   config->secure            = false;

   for (a = 1; a < argc; ++a)
   {
      const char  *arg = argv[a];

      if (strcmp(arg, "+m") == 0)
         config->useMultisample = true;
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
      else if (strcmp(arg, "+secure") == 0)
         config->secure = true;
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

   printf("********************************************************************************\n"
          "* WARNING, ENTERING STANDBY WILL TERMINATE YOUR NETWORK CONNECTIONS            *\n"
          "* PRESS <CTRL><C> TO ABORT                                                     *\n"
          "* POWER BUTTON ON REMOTE WILL TOGGLE S3                                        *\n"
          "********************************************************************************\n");

   if (!processArgs(argc, argv, &config))
   {
      const char  *progname = argc > 0 ? argv[0] : "";
      fprintf(stderr, "Usage: %s [+m] [+s] [d=WxH] [o=XxY] [bpp=16/24/32] [f=frames] [+secure]\n", progname);
      return 0;
   }

#ifndef EGL_PROTECTED_CONTENT_EXT
   if (config.secure)
      printf("+secure selected, but headers not available in this driver version. defaulting off\n");
   config.secure = false;
#endif

   /* Setup the display and EGL */
   if (InitDisplay(&panelAspect, &config))
   {
      NEXUS_IrInputSettings irSettings;

      /* triggered via the IR handler */
      BKNI_CreateEvent(&event_power_down);

      NEXUS_IrInput_GetDefaultSettings(&irSettings);
      irSettings.mode = NEXUS_IrInputMode_eCirNec;
      irSettings.dataReady.callback = irCallback;
      irSettings.dataReady.context = &ir_handle;
      ir_handle = NEXUS_IrInput_Open(0, &irSettings);

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

      NEXUS_IrInput_Close(ir_handle);
      BKNI_DestroyEvent(event_power_down);
   }

   /* Terminate EGL */
   eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglTerminate(eglDisplay);

   NXPL_DestroyNativeWindow(native_window);
   NXPL_UnregisterNexusDisplayPlatform(nxpl_handle);

   /* Close the platform */
   TermPlatform(nexus_display);

   return 0;
}
