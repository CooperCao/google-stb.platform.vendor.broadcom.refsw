/***************************************************************************
 *     Broadcom Proprietary and Confidential. (c)2008-2016 Broadcom.  All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/


#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <VG/openvg.h>
#include <freetype/ftoutln.h>

#include "default_nexus.h"

#include "../common/init.h"

typedef struct
{
   bool  stretchToFit;
   int   vpX;
   int   vpY;
   int   vpW;
   int   vpH;
   int   bpp;
   int   frames;
   unsigned clientId;
} AppConfig;

const unsigned int WIDTH             = 1280;
const unsigned int HEIGHT            = 720;
const unsigned int FRAMES            = 0;
const unsigned int BPP               = 32;
const unsigned int FONTSIZE          = 64;

NEXUS_DISPLAYHANDLE  nexus_display = 0;
EGLNativeDisplayType native_display = 0;
void                 *native_window = 0;

NXPL_PlatformHandle nxpl_handle = 0;

static FT_Library g_library;
static FT_Face g_face;
static VGFont g_vgfont;

static void appendFTVectorsToPath(VGPath path,
                                  VGubyte segment,
                                  FT_Vector** vectors)
{
   VGfloat coords[6];

   switch (segment) {
      case VG_MOVE_TO:
      case VG_LINE_TO:
         coords[0] = (VGfloat)vectors[0]->x / FONTSIZE;
         coords[1] = (VGfloat)vectors[0]->y / FONTSIZE;
         break;
      case VG_QUAD_TO:
         coords[0] = (VGfloat)vectors[0]->x / FONTSIZE;
         coords[1] = (VGfloat)vectors[0]->y / FONTSIZE;
         coords[2] = (VGfloat)vectors[1]->x / FONTSIZE;
         coords[3] = (VGfloat)vectors[1]->y / FONTSIZE;
         break;
      case VG_CUBIC_TO:
         coords[0] = (VGfloat)vectors[0]->x / FONTSIZE;
         coords[1] = (VGfloat)vectors[0]->y / FONTSIZE;
         coords[2] = (VGfloat)vectors[1]->x / FONTSIZE;
         coords[3] = (VGfloat)vectors[1]->y / FONTSIZE;
         coords[4] = (VGfloat)vectors[2]->x / FONTSIZE;
         coords[5] = (VGfloat)vectors[2]->y / FONTSIZE;
         break;
      default:
         printf("Unknown segment type\n");
         return;
   }

   vgAppendPathData(path, 1, &segment, (void *)coords);
}

int FTOutlineMoveTo(FT_Vector* to, void* user)
{
   VGPath path = (VGPath)user;
   appendFTVectorsToPath(path, VG_MOVE_TO, &to);
   return 0;
}

int FTOutlineLineTo(FT_Vector* to, void* user)
{
   VGPath path = (VGPath)user;
   appendFTVectorsToPath(path, VG_LINE_TO, &to);
   return 0;
}

int FTOutlineConicTo(FT_Vector* control,
                     FT_Vector* to,
                     void* user)
{
   FT_Vector* vectors[2];
   VGPath path = (VGPath)user;
   vectors[0] = control;
   vectors[1] = to;
   appendFTVectorsToPath(path, VG_QUAD_TO, vectors);
   return 0;
}

int FTOutlineCubicTo(FT_Vector* control1,
                     FT_Vector* control2,
                     FT_Vector* to,
                     void* user)
{
   FT_Vector* vectors[3];
   VGPath path = (VGPath)user;
   vectors[0] = control1;
   vectors[1] = control2;
   vectors[2] = to;
   appendFTVectorsToPath(path, VG_CUBIC_TO, vectors);
   return 0;
}

static VGPath convertGlyphToPath(void)
{
   static const FT_Outline_Funcs funcs = {
      &FTOutlineMoveTo,
      &FTOutlineLineTo,
      &FTOutlineConicTo,
      &FTOutlineCubicTo,
      0, 0
   };

   VGPath path = vgCreatePath(
      VG_PATH_FORMAT_STANDARD,
      VG_PATH_DATATYPE_F,
      1.0f, 0.0f,
      0, g_face->glyph->outline.n_points,
      VG_PATH_CAPABILITY_ALL);

   if (FT_Outline_Decompose(&g_face->glyph->outline,
                              &funcs,
                              (void *)path))
   {
      printf("FT_Outline_Decompose() failed\n");
      vgDestroyPath(path);
      return VG_INVALID_HANDLE;
   }
   return path;
}

static void drawString(VGfloat x, VGfloat y, const char *str)
{
   VGfloat origin[2];
   int i;
   VGuint * indices = (VGuint *)alloca(strlen(str) * sizeof(VGuint));
   origin[0] = x;
   origin[1] = y;

   for (i = 0; i < (int)strlen(str); i++)
      indices[i] = str[i];

   vgSetfv(VG_GLYPH_ORIGIN, 2, origin);
   vgDrawGlyphs(g_vgfont, strlen(str), indices,
               NULL, NULL, VG_FILL_PATH, VG_TRUE);
}

void InitVGState(const AppConfig *config)
{
   VGfloat origin[2] = { 0.0f, 0.0f };
   VGfloat color[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; /* Opaque yellow */
   VGfloat escapement[2];
   VGErrorCode err;
   VGPath path;
   FT_ULong c;

   if (FT_Init_FreeType(&g_library))
   {
      printf("FT_Init_FreeType() failed\n");
      return;
   }

   if (FT_New_Face(g_library,
                   "LinLibertine_Re-4.4.1.ttf",
                   0,
                   &g_face))
   {
      printf("FT_New_Face() failed\n");
      return;
   }

   if (FT_Select_Charmap(g_face, FT_ENCODING_UNICODE))
   {
      printf("FT_Select_Charmap() failed\n");
      return;
   }

   if (FT_Set_Pixel_Sizes(g_face, FONTSIZE, FONTSIZE))
   {
      printf("FT_Set_Pixel_Sizes() failed\n");
      return;
   }

   vgSetfv(VG_CLEAR_COLOR, 4, color);

   g_vgfont = vgCreateFont(128);
   if (g_vgfont == VG_INVALID_HANDLE)
   {
      printf("vgCreateFont() failed\n");
      return;
   }

   for (c = 1; c < 128; c++)
   {
      if (FT_Load_Char(g_face, c, FT_LOAD_DEFAULT))
      {
         printf("FT_Load_Char() failed: %lu\n", c);
         continue;
      }

      if (g_face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
      {
         printf("aface->glyph->format not supported: %d\n", g_face->glyph->format);
         continue;
      }

      escapement[0] = (VGfloat)g_face->glyph->advance.x / FONTSIZE;
      escapement[1] = (VGfloat)g_face->glyph->advance.y / FONTSIZE;

      path = convertGlyphToPath();
      if (path == VG_INVALID_HANDLE)
      {
         continue;
      }
      vgSetGlyphToPath(g_vgfont, c, path, VG_TRUE, origin, escapement);
      err = vgGetError();
      if (err != VG_NO_ERROR)
      {
         printf("vgSetGlyphToPath() failed: %d\n", err);
      }
   }
}

void TerminateVGState(void)
{
   if (g_vgfont != VG_INVALID_HANDLE)
   {
      vgDestroyFont(g_vgfont);
   }
   if (g_face)
   {
      FT_Done_Face(g_face);
   }
   if (g_library)
   {
      FT_Done_FreeType(g_library);
   }
}

void Display(AppConfig *config)
{
   VGfloat start = 650.0f;
   VGfloat color[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; /* white */

   vgSeti(VG_SCISSORING, VG_FALSE);
   vgSetfv(VG_CLEAR_COLOR, 4, color);
   vgClear(0, 0, (float)config->vpW, (float)config->vpH);

   drawString(0.0f, start, "Lorem ipsum dolor sit amet, consectetur adipiscing");            start -= 64.0f;
   drawString(0.0f, start, "elit. Sed rutrum arcu eu augue consectetur");                    start -= 64.0f;
   drawString(0.0f, start, "rhoncus sed sit amet ligula.  Donec ut nisi ut");                start -= 64.0f;
   drawString(0.0f, start, "dolor gravida. Donec gravida, tellus ut ");                      start -= 64.0f;
   drawString(0.0f, start, "ullamcorper vestibulum, elit enim aliquam nisi,");               start -= 64.0f;
   drawString(0.0f, start, "non gravida est enim euismod leo. Aliquam");                     start -= 64.0f;
   drawString(0.0f, start, "sollicitudin rhoncus elit at placerat. Duis luctus est ");       start -= 64.0f;
   drawString(0.0f, start, "sapien, laoreet varius metus porta a. In tempor,");              start -= 64.0f;
   drawString(0.0f, start, "ipsum a viverra");                                               start -= 64.0f;

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

   if (!eglBindAPI(EGL_OPENVG_API))
   {
      printf("eglBindAPI(EGL_OPENVG_API) failed\n");
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
      attr[i++] = EGL_RENDERABLE_TYPE; attr[i++] = EGL_OPENVG_BIT;

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

   /*
      Step 6 - Create a context.
      EGL has to create a context for OpenGL ES. Our OpenGL ES resources
      like textures will only be valid inside this context (or shared contexts)
   */
   egl_context = eglCreateContext(egl_display, egl_config[config_select], EGL_NO_CONTEXT, NULL);
   if (egl_context == EGL_NO_CONTEXT)
   {
      printf("eglCreateContext() failed");
      return false;
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

   eInitResult res = InitPlatformAndDefaultDisplay(&nexus_display, aspect, config->vpW, config->vpH, false);
   if (res != eInitSuccess)
      return false;

   /* Register with the platform layer */
   NXPL_RegisterNexusDisplayPlatform(&nxpl_handle, nexus_display);

   NXPL_GetDefaultNativeWindowInfoEXT(&win_info);

   win_info.x = config->vpX; 
   win_info.y = config->vpY;
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

   config->stretchToFit      = false;
   config->vpX               = 0;
   config->vpY               = 0;
   config->vpW               = WIDTH;
   config->vpH               = HEIGHT;
   config->bpp               = BPP;
   config->frames            = FRAMES;
   config->clientId          = 0;

   for (a = 1; a < argc; ++a)
   {
      const char  *arg = argv[a];

      if (strcmp(arg, "+s") == 0)
         config->stretchToFit = true;
      else if (strncmp(arg, "d=", 2) == 0)
      {
         if (sscanf(arg, "d=%dx%d", &config->vpW, &config->vpH) != 2)
            return false;
      }
      else if (strncmp(arg, "o=", 2) == 0)
      {
         if (sscanf(arg, "o=%dx%d", &config->vpX, &config->vpY) != 2)
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
   AppConfig    config;
   float        panelAspect = 1.0f;
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
      /* Setup the local state for this demo */
      InitVGState(&config);

      printf("Rendering ");

      if (config.frames != 0)
         printf("%d frames", config.frames);

      printf(": press CTRL+C to terminate early\n");
   
      while (config.frames == 0 || frame <= config.frames)
      {
         Display(&config);
         frame++;
      }

      /* Close the local state for this demo */
      TerminateVGState();
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
