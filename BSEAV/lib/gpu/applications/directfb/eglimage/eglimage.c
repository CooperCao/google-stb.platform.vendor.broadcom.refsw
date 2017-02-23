/******************************************************************************
 *    Broadcom Proprietary and Confidential. (c)2013 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

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

/* virtual coordinate system for the orthographic projection */
const unsigned int WIDTH             = 1920;
const unsigned int HEIGHT            = 1080;

IDirectFB             *dfb = NULL;
IDirectFBSurface      *surface = NULL;

DBPL_PlatformHandle dbpl_handle = 0;

ESMatrix projection_matrix;
ESMatrix modelview_matrix;
ESMatrix mvp_matrix;

GLint mvp_matrix_loc;
GLint position_loc;
GLint texcoord_loc;
GLint textureunit_loc;

GLint program_object;

typedef struct
{
   unsigned int  egl_image_height;
   unsigned int  egl_image_width;

   EGLNativePixmapType  eglPixmap;
   IDirectFBSurface    *dfbSurface;

   EGLImageKHR          eglimage;
   GLuint               texture;

   int pos_x;
   int pos_y;
} image_ctx;

#define IMAGES 6

#ifdef vc5
#define BEGL_BufferFormat_eA8B8G8R8_Texture BEGL_BufferFormat_eA8B8G8R8
#endif

image_ctx* images[IMAGES] = { NULL };

static image_ctx* load_image(char * filename)
{
   IDirectFBImageProvider *provider = NULL;

   image_ctx * p = malloc(sizeof(image_ctx));
   if (p)
   {
      IDirectFBSurface *tmpSurface;
      DFBSurfaceDescription dsc;
      EGLint attr_list[] = { EGL_NONE };
      BEGL_PixmapInfo      pixInfo;

      DFBCHECK (dfb->CreateImageProvider(dfb, filename, &provider));
      DFBCHECK (provider->GetSurfaceDescription(provider, &dsc));

      /* create a pixmap */
      p->egl_image_width =
         pixInfo.width = dsc.width;
      p->egl_image_height =
         pixInfo.height = dsc.height;
      pixInfo.format = BEGL_BufferFormat_eA8B8G8R8_Texture;
      /* this image will have the correct stride for 3d core */
      if (!DBPL_CreateCompatiblePixmap(dbpl_handle, &p->eglPixmap, &p->dfbSurface, &pixInfo))
         goto error0;

      /* scratch surface to decode into, as it needs flipping */
      DFBCHECK (dfb->CreateSurface(dfb, &dsc, &tmpSurface));
      DFBCHECK (provider->RenderTo(provider, tmpSurface, NULL));

      /* OpenGL has 0,0 at the bottom of the image, so flip the texture */
      DFBCHECK (p->dfbSurface->SetBlittingFlags(p->dfbSurface, DSBLIT_FLIP_VERTICAL));

      /* blit from the scratch to the eglimage backing */
      DFBCHECK (p->dfbSurface->Blit(p->dfbSurface, tmpSurface, NULL, 0, 0));

      tmpSurface->Release(tmpSurface);

      /* create the egl image */
      p->eglimage = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)p->eglPixmap, attr_list);
      if (p->eglimage == EGL_NO_IMAGE_KHR)
      {
         printf("eglCreateImageKHR() failed\n");
         goto error1;
      }

      /* Bind the EGL image as a texture, and set filtering (no mipmaps) */
      glGenTextures(1, &p->texture);

      glBindTexture(GL_TEXTURE_2D, p->texture);

      glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, p->eglimage);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      provider->Release(provider);
   }
   return p;

error1:
   DBPL_DestroyCompatiblePixmap(dbpl_handle, p->eglPixmap);

error0:
   provider->Release(provider);
   free(p);

   return NULL;
}

void InitGLState(void)
{
   /* The shaders */
   const char vShaderStr[] =
      "uniform mat4   u_mvpMatrix;               \n"
      "attribute vec4 a_position;                \n"
      "attribute vec2 a_texcoord;                \n"
      "varying vec2   v_texCoord;                \n"
      "                                          \n"
      "void main()                               \n"
      "{                                         \n"
      "  v_texCoord = a_texcoord;                \n"
      "  gl_Position = u_mvpMatrix * a_position; \n"
      "}                                         \n";

   const char fShaderStr[] =
      "precision mediump float;                                      \n"
      "uniform sampler2D u_textureUnit;                              \n"
      "varying vec2 v_texCoord;                                      \n"
      "                                                              \n"
      "void main()                                                   \n"
      "{                                                             \n"
      "  gl_FragColor = texture2D(u_textureUnit, v_texCoord);        \n"
      "}                                                             \n";

   GLuint     v, f;
   GLint      ret;
   const char *ff;
   const char *vv;
   char       *p, *q;

   glClearDepthf(1.0f);
   glClearColor(0.2f, 0.2f, 0.2f, 1);  /* Gray background */

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

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
   glGetProgramiv(program_object, GL_LINK_STATUS, &ret);
   if (ret != GL_TRUE)
   {
      printf("Program did not correctly link\n");
      exit(0);
   }

   /* Get the attribute locations */
   position_loc = glGetAttribLocation(program_object, "a_position");
   texcoord_loc = glGetAttribLocation(program_object, "a_texcoord");

   /* Get the uniform locations */
   mvp_matrix_loc = glGetUniformLocation(program_object, "u_mvpMatrix");
   textureunit_loc = glGetUniformLocation(program_object, "u_textureUnit");

   /* LOAD the images */
   images[0] = load_image("textures//image_1.png");     images[0]->pos_x =    0;   images[0]->pos_y =   0;
   images[1] = load_image("textures//image_2.png");     images[1]->pos_x = 1470;   images[1]->pos_y = 350;
   images[2] = load_image("textures//image_3.png");     images[2]->pos_x =  500;   images[2]->pos_y = 350;
   images[3] = load_image("textures//image_4.png");     images[3]->pos_x =  350;   images[3]->pos_y = 800;
   images[4] = load_image("textures//image_5.png");     images[4]->pos_x =   90;   images[4]->pos_y =  80;
   images[5] = load_image("textures//image_6.png");     images[5]->pos_x =  750;   images[5]->pos_y = 250;
}

void TerminateGLState(void)
{
   int i;
   for (i = 0; i < IMAGES; i++)
   {
      eglDestroyImageKHR(eglGetCurrentDisplay(), images[i]->eglimage);
      DBPL_DestroyCompatiblePixmap(dbpl_handle, images[i]->eglPixmap);
      glDeleteTextures(1, &images[i]->texture);
   }
}

void InitGLViewPort(unsigned int width, unsigned int height, float panelAspect, bool stretch)
{
   glViewport(0, 0, width, height);

   esMatrixLoadIdentity(&projection_matrix);

   /* This program uses a virtual co-ordinate system based at 1080p.
      This doesn't imply that it is rendered at this size */
   esOrtho(&projection_matrix, 0.0f, WIDTH, 0.0f, HEIGHT, -1.0f, 1.0f);
}

void Display(void)
{
   int i;

   glClearColor(0, 0, 1, 1);
   glClearDepthf(1.0f);
   glClearStencil(0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   glUseProgram(program_object);

   for (i = 0; i < IMAGES; i++)
   {
      int j = 0;
      GLfloat v[4 * 5];

      /* TODO: created specific for image based on width/height */
      v[j++] = 0.0f;                              v[j++] =  0.0f;                               v[j++] =  1; v[j++] = 0; v[j++] = 0;
      v[j++] = (float)images[i]->egl_image_width; v[j++] =  0.0f;                               v[j++] =  1; v[j++] = 1; v[j++] = 0;
      v[j++] = (float)images[i]->egl_image_width; v[j++] =  (float)images[i]->egl_image_height; v[j++] =  1; v[j++] = 1; v[j++] = 1;
      v[j++] = 0.0f;                              v[j++] =  (float)images[i]->egl_image_height; v[j++] =  1; v[j++] = 0; v[j++] = 1;

      /* set up a default modelview matrix */
      esMatrixLoadIdentity(&modelview_matrix);

      esTranslate(&modelview_matrix, images[i]->pos_x, images[i]->pos_y, 0);

      /* Compute the final MVP by multiplying the model-view and perspective matrices together */
      esMatrixMultiply(&mvp_matrix, &modelview_matrix, &projection_matrix);

      glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)&mvp_matrix.m[0][0]);

      glBindTexture(GL_TEXTURE_2D, images[i]->texture);
      glUniform1i(textureunit_loc, 0);

      glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), v);
      glEnableVertexAttribArray(position_loc);
      glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &v[3]);
      glEnableVertexAttribArray(texcoord_loc);

      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

      glDisableVertexAttribArray(position_loc);
      glDisableVertexAttribArray(texcoord_loc);
   }

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
   egl_win->GetPixelFormat(egl_win, &pixel_format);

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
   egl_surface = eglCreateWindowSurface(egl_display, egl_config[config_select], egl_win, NULL);
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

   /* Only use preserved swap if you need the contents of the frame buffer to be preserved from
    * one frame to the next
    */
   if (config->usePreservingSwap)
   {
      printf("Using preserved swap.  Application will run slowly.\n");
      eglSurfaceAttrib(egl_display, egl_surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);
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

   eglSwapInterval(egl_display, 1);

   return true;
}

bool InitDisplay(const AppConfig *config)
{
   DFBSurfaceDescription  desc;

   DFBCHECK(DirectFBCreate(&dfb));

   dfb->SetCooperativeLevel(dfb, DFSCL_FULLSCREEN);

   desc.flags = DSDESC_CAPS | DSDESC_PIXELFORMAT;
#if (DIRECTFB_MAJOR_VERSION <= 1) && (DIRECTFB_MINOR_VERSION <= 4) && (DIRECTFB_MICRO_VERSION <=5)
   desc.caps  = DSCAPS_PRIMARY | DSCAPS_DOUBLE;
#else
   desc.caps  = DSCAPS_PRIMARY | DSCAPS_TRIPLE | DSCAPS_GL;
#endif
   desc.pixelformat = DSPF_ABGR;

   DFBCHECK(dfb->CreateSurface(dfb, &desc, &surface));

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
