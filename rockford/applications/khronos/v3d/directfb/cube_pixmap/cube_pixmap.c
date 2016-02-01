/******************************************************************************
 *    (c)2008-2010 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 *
 *****************************************************************************/

#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <directfb.h>
#include <directfb_util.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "esutil.h"
#include "default_directfb.h"

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x) \
     {                                                                \
          int err = x;                                                \
          if (err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, err );                         \
          }                                                           \
     }

volatile int terminate = 0;

void sigint_handler(int __unused__)
{
   terminate = 1;
}

/* unlike nexus, the width/height and bpp are taken from the underlying surface format */
typedef struct
{
   bool  useMultisample;
   bool  usePreservingSwap;
   int   frames;
} AppConfig;

const unsigned int FRAMES            = 0;

IDirectFB             *dfb = NULL;
IDirectFBSurface      *surface = NULL;
IDirectFBSurface      *offscreen_surface = NULL;
IDirectFBFont         *font = NULL;

DBPL_PlatformHandle dbpl_handle = 0;
void* dbpl_surface = 0;

ESMatrix projection_matrix;
ESMatrix modelview_matrix;
ESMatrix mvp_matrix;

GLint mvp_matrix_loc;
GLint position_loc;
GLint color_loc;

GLint program_object;

GLuint vbo[2];

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
   glClearColor(0.2f, 0.2f, 0.2f, 1);  /* Gray background */

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
}

void TerminateGLState(void)
{
   glDeleteProgram(program_object);
   glDeleteBuffers(2, vbo);
}

void InitGLViewPort(unsigned int width, unsigned int height, float panelAspect, bool stretch)
{
   glViewport(0, 0, width, height);

   if (stretch)
   {
      esPerspective(&projection_matrix, 45.0f, panelAspect, 100, 1000);
   }
   else
   {
      esPerspective(&projection_matrix, 45.0f, (float)width / (float)height, 100, 1000);
   }

   esMatrixLoadIdentity(&modelview_matrix);
   esTranslate(&modelview_matrix, 0, 0, -500);
   esScale(&modelview_matrix, 100, 100, 100);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void Display(void)
{
   static const char *text = "Blitting DFB and GL together using pixmaps";
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
   glFinish();

   DFBCHECK(surface->Blit(surface, offscreen_surface, NULL, 0, 0));
   DFBCHECK(surface->SetColor (surface, 0x80, 0x0, 0x20, 0xFF));
   DFBCHECK(surface->DrawString (surface, text, -1, 50, 50, DSTF_LEFT));
   DFBCHECK(surface->Flip(surface, 0, DSFLIP_WAIT));
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

bool InitEGL(IDirectFBSurface* egl_win, const AppConfig *config)
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

   DFBSurfacePixelFormat pixel_format;
   DFBCHECK(egl_win->GetPixelFormat(egl_win, &pixel_format));

   BppToChannels(DFB_BYTES_PER_PIXEL(pixel_format) * 8, &want_red, &want_green, &want_blue, &want_alpha);

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
      attr[i++] = EGL_SURFACE_TYPE;    attr[i++] = EGL_PIXMAP_BIT;
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

   BEGL_PixmapInfo pixmapInfo;
   memset(&pixmapInfo, 0, sizeof(BEGL_PixmapInfo));
   DFBCHECK(egl_win->GetSize(egl_win, &pixmapInfo.width, &pixmapInfo.height));
   pixmapInfo.format = BEGL_BufferFormat_eA8B8G8R8;

   if (!DBPL_CreateCompatiblePixmap(
       dbpl_handle, &dbpl_surface, &offscreen_surface, &pixmapInfo))
   {
      printf("DBPL_CreateCompatiblePixmap() failed\n");
      return false;
   }
   
   egl_surface = eglCreatePixmapSurface(egl_display, egl_config[config_select], dbpl_surface, NULL);
   if (egl_surface == EGL_NO_SURFACE)
   {
      printf("eglCreateContext() failed");
      return false;
   }

   /*
      Step 6 - Create a context.
      EGL has to create a context for OpenGL ES. Our OpenGL ES resources
      like textures will only be valid inside this context (or shared contexts)
   */
   {
      EGLint     ctx_attrib_list[3] =
      {
           EGL_CONTEXT_CLIENT_VERSION, 2, /* For ES2 */
           EGL_NONE
      };

      egl_context = eglCreateContext(egl_display, egl_config[config_select], EGL_NO_CONTEXT, ctx_attrib_list);
      if (egl_context == EGL_NO_CONTEXT)
      {
         printf("eglCreateContext() failed");
         return false;
      }
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

bool InitDisplay(const AppConfig *config)
{
   DFBSurfaceDescription  desc;
   DFBFontDescription font_dsc;

   DFBCHECK(DirectFBCreate(&dfb));

   dfb->SetCooperativeLevel(dfb, DFSCL_FULLSCREEN);

   desc.flags = DSDESC_CAPS;
   desc.caps  = DSCAPS_PRIMARY | DSCAPS_DOUBLE;

   DFBCHECK(dfb->CreateSurface(dfb, &desc, &surface));

   font_dsc.flags = DFDESC_HEIGHT;
   font_dsc.height = 48;
   DFBCHECK(dfb->CreateFont (dfb, "../../../share/fonts/decker.ttf", &font_dsc, &font));
   DFBCHECK(surface->SetFont (surface, font));

   /* register directfb instance with opengl */
   DBPL_RegisterDirectFBDisplayPlatform(&dbpl_handle, dfb);

   /* Initialise EGL now we have a 'window' */
   if (!InitEGL(surface, config))
      return false;

   return true;
}

bool processArgs(int argc, char *argv[], AppConfig *config)
{
   int   a;

   config->useMultisample    = false;
   config->usePreservingSwap = false;
   config->frames            = FRAMES;

   if (config == NULL)
      return false;

   for (a = 1; a < argc; ++a)
   {
      char  *arg = argv[a];

      if (strcmp(arg, "+m") == 0)
         config->useMultisample = true;
      else if (strcmp(arg, "+p") == 0)
         config->usePreservingSwap = true;
      else if (strncmp(arg, "f=", 2) == 0)
      {
         if (sscanf(arg, "f=%d", &config->frames) != 1)
            return false;
      }
      else
      {
         return false;
      }
   }

   return true;
}

int main(int argc, char** argv)
{
   AppConfig    config;
   int          panelWidth, panelHeight;
   EGLDisplay   eglDisplay;
   int         frame = 1;

   /* rewrites argc/argv, removing any dfb specific options, leaving ours */
   DFBCHECK(DirectFBInit( &argc, &argv ));

   if (!processArgs(argc, argv, &config))
   {
      const char  *progname = argc > 0 ? argv[0] : "";
      fprintf(stderr, "Usage: %s [+m] [+p] [f=frames]\n", progname);
      return 0;
   }

   /* Setup the display and EGL */
   if (InitDisplay(&config))
   {
      /* Setup the local state for this demo */
      InitGLState();

      /* get width/height and generate the aspect */
      surface->GetSize(surface, &panelWidth, &panelHeight);

      InitGLViewPort(panelWidth, panelHeight, (float)panelWidth / panelHeight, true);

      printf("Rendering ");

      if (config.frames != 0)
         printf("%d frames", config.frames);

      printf(": press CTRL+C to terminate early\n");

      signal(SIGINT, sigint_handler);
   
      while ((!terminate) && (config.frames == 0 || frame <= config.frames))
      {
         Display();
         frame++;
      }

      /* Close the local state for this demo */
      TerminateGLState();
   }

   if (dfb != 0)
   {
      /* Terminate EGL */
      eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      eglTerminate(eglDisplay);

      DBPL_UnregisterDirectFBDisplayPlatform(dbpl_handle);

      surface->Release(surface);
      dfb->Release(dfb);
   }

   return 0;
}
