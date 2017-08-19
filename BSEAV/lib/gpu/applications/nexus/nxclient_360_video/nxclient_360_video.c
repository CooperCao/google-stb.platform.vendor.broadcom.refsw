/******************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

/***************************************************************************
*     Broadcom Proprietary and Confidential. (c)2011-2016 Broadcom.  All rights reserved.
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
*  LICENSORS BE LIABLE for(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
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
#include <math.h>
#include <limits.h>
#include <sys/time.h>

#include <EGL/egl.h>
#define  EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#define  GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>

#include "esutil.h"
#include "default_nexus.h"

#include "../common/init.h"

#include "bfile_stdio.h"
#include "bmedia_pcm.h"
#include "bmedia_probe.h"
#include "bmpeg2ts_probe.h"
#include "bmedia_cdxa.h"

#if B_HAS_ASF
#include "basf_probe.h"
#endif

#if B_HAS_AVI
#include "bavi_probe.h"
#endif

#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_mosaic_video_decoder.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_stc_channel.h"
#include "nexus_file.h"
#include "nexus_base_types.h"
#include "nexus_core_utils.h"
#include "binput.h"

#include "nexus_platform_client.h"
#include "media_player.h"
#include "nexus_surface_client.h"
#include "nxclient.h"
#include "nexus_graphics2d.h"

#define ENABLE_AMBISONIC_AUDIO   1

#if ENABLE_AMBISONIC_AUDIO
BDBG_MODULE(audio_decode_ambisonic);
#endif

#ifndef SINGLE_PROCESS
#include "nxclient.h"
#endif

#if EGL_BRCM_image_update_control
#define NUM_TEX  1
#else
#define NUM_TEX  2
#endif


#define ENABLE_ROLL     0
#define ENABLE_ZOOM     0

//*************************************************************************************
//  Data Structures
//*************************************************************************************

//! 360 video formats
//------------------------------------------------
typedef enum eFormat
{
    FORMAT_EQUIRECT=0,                 //!<   equirectangular projection
    FORMAT_CUBE_32_0=1,                //!<   cube projection 3:2 rotation 0
    FORMAT_CUBE_32_90=2,               //!<   cube projection 3:2 rotation 90
    FORMAT_CUBE_32_270=3,              //!<   cube projection 3:2 rotation 270
    FORMAT_CUBE_32_P270=4,             //!<   cube projection 3:2 rotation p270
    FORMAT_CUBE_43_0=5,                //!<   cube projection 4:3 rotation 0
    FORMAT_FISHEYE=6,                  //!<   fisheye projection
    FORMAT_ICOSAHEDRON=7,              //!<   icosahedron projection
    FORMAT_OCTAHEDRON=8,               //!<   octahedron projection
    FORMAT_EAP=9,                      //!<   equal area projection
    FORMAT_TOTAL_NUM,                  //!<   number of supported formats

} eFormat_t;

//! App Configuration
//------------------------------------------------
typedef struct
{
    bool         showFPS;              //!<   display frames/sec performance counter or not
    int          vpX;                  //!<   viewport horizontal position on screen
    int          vpY;                  //!<   viewport vertical position on screen
    int          vpW;                  //!<   viewport width
    int          vpH;                  //!<   viewport height
    int          swapInterval;         //!<   buffer swap interval (1-normal, 0-tearing possible)
    unsigned     clientId;             //!<   client ID
    unsigned     numVideos;            //!<   number of videos
    char         videoFile0[PATH_MAX]; //!<   video file name
    char         videoFile1[PATH_MAX]; //!<   video file name
    char         videoFile2[PATH_MAX]; //!<   video file name
    char         videoFile3[PATH_MAX]; //!<   video file name
    int          texW;                 //!<   texture width
    int          texH;                 //!<   texture height
    eFormat_t    vFormat;              //!<   360 video format

} AppConfig;


//! Media Data Parameters
//------------------------------------------------
typedef struct
{
    uint32_t                         width;                  //!<   video width
    uint32_t                         height;                 //!<   video height
    char                             filename[PATH_MAX];     //!<   filename

} MediaData;


//! Video Stream Parameters
//------------------------------------------------
typedef struct
{
    NEXUS_SimpleVideoDecoderHandle   videoDecoder;                                      //!<   video decoder handle
    NEXUS_SimpleAudioDecoderHandle   audioDecoder;                                      //!<   audio decoder handle
    NEXUS_DisplayHandle              nexusDisplay;                                      //!<   display handle
    uint32_t                         sourceWidth;                                       //!<   source width
    uint32_t                         sourceHeight;                                      //!<   source height
    uint32_t                         outputWidth;                                       //!<   output width
    uint32_t                         outputHeight;                                      //!<   output height
    NEXUS_SurfaceClientHandle        surfaceClient;                                     //!<   surface client handle
    NEXUS_SurfaceHandle              videoSurfaces[NEXUS_SIMPLE_DECODER_MAX_SURFACES];  //!<   video surface handles
    media_player_t                   mediaPlayer;                                       //!<   media player
    bool                             secure;                                            //!<   secure decode/graphics

} VideoStream;


//! 3D vector
//------------------------------------------------
typedef struct
{
    float   v[3];           //!<   vector values

} ESVec3;


//! camera structure
//------------------------------------------------
typedef struct camera_s
{
    // camera attributes
    ESVec3  position;       //!<   camera position in 3D space
    ESVec3  front;          //!<   camera front direction vector
    ESVec3  up;             //!<   camera up direction vector
    ESVec3  right;          //!<   camera right direction vector
    ESVec3  worldUp;        //!<   3D world up vector

    // euler angles
    float   yaw;            //!<   camera horizontal rotation in radians
    float   pitch;          //!<   camera vertical rotation in radians
    float   roll;           //!<   camera roll in radians

    unsigned   yawDeg;      //!<   camera horizontal rotation in degrees (0-359)
    unsigned   pitchDeg;    //!<   camera vertical rotation in degrees (0-359)
    unsigned   rollDeg;     //!<   camera roll in degrees (0-359)

    // camera options
    float   movementSpeed;  //!<   movement speed
    float   sensitivity;    //!<   rotation sensitivity
    float   zoom;           //!<   field of fiew zoom angle

} camera_t;

//! cube face location and orientation within cube video
//------------------------------------------------
typedef struct cube_face_s
{
    int     posX;           //!<   horizontal position within grid
    int     posY;           //!<   vertical position within grid
    int     rot;            //!<   face rotation (0=0, 1=90, 2=180, 3=270)

} cube_face_t;

//! cube video format definition
//------------------------------------------------
typedef struct cube_fmt_s
{
    int          gridW;     //!<   number of horizontal cube faces in 360 video grid
    int          gridH;     //!<   number of vertical cube faces in 360 video grid
    cube_face_t  face[6];   //!<   face locations and orientation (0=left, 1=back, 2=right, 3=front, 4=top, 5=bot)

} cube_fmt_t;



//*************************************************************************************
//  Global Variables
//*************************************************************************************

// app configuration
static AppConfig               config;                       //!<   app config structure

// model / view / projection matrix transformations
static ESMatrix                model;                        //!<   cube model transformation matrix
static ESMatrix                view;                         //!<   camera view matrix
static ESMatrix                projection;                   //!<   view projection matrix

// shader attribute locations
static GLint                   modelLoc;                     //!<   model transform matrix location
static GLint                   viewLoc;                      //!<   view transform matrix location
static GLint                   projectionLoc;                //!<   projection transform matrix location
static GLint                   positionLoc;                  //!<   pixel position attribute location
static GLint                   texCoordLoc;                  //!<   texture coordinate attribute location for cube formats
static GLint                   texUnitLoc;                   //!<   texture unit location

// OpenGL context buffers
static GLint                   programObject;                //!<   openGL program object
static GLuint                  vbo[2];                       //!<   vertex buffer object

// EGL texture mapping
static EGLDisplay              eglDisplay = EGL_NO_DISPLAY;  //!<   EGL display
static EGLNativePixmapType     eglPixmaps[NUM_TEX];          //!<   EGL pixmap type
static EGLImageKHR             eglImages[NUM_TEX];           //!<   EGL images
static GLuint                  esTextures[NUM_TEX];          //!<   EGL textures
static unsigned int            currentTex = 0;               //!<   current texture index

// nexus handles
static NEXUS_DISPLAYHANDLE     nexusDisplay = 0;             //!<   nexus display handle
static NXPL_PlatformHandle     nxplHandle = 0;               //!<   nexus platform handle
static NEXUS_SurfaceHandle     nativePixmaps[NUM_TEX];       //!<   nexus surface handles
static BKNI_EventHandle        blitTextureDone;              //!<   blit done event handle
static NEXUS_Graphics2DHandle  gfx2d;                        //!<   2D graphics handle
static NxClient_AllocResults   allocResults;                 //!<   allocation results
static VideoStream             videoStream;                  //!<   video stream structure
static void                   *nativeWindow = 0;             //!<   window handle

// display status
static int       waitCnt = 0;                                //!<   number of times video frame buffer was empty during texture update
static int       totalCnt = 0;                               //!<   total number of times texture was updated
static int       activeVideo = 0;                            //!<   active video sequence

// 3D camera
static camera_t  glCamera;                                   //!<   camera structure

// rotation state
static float     xRotate;                                    //!<   current camera horizontal rotation amount
static float     yRotate;                                    //!<   current camera vertical rotation amount
static float     rRotate;                                    //!<   current camera roll rotation amount


// EGL function prototype pointers
static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC    s_glEGLImageTargetTexture2DOES = NULL;
static PFNEGLCREATEIMAGEKHRPROC               s_eglCreateImageKHR = NULL;
static PFNEGLDESTROYIMAGEKHRPROC              s_eglDestroyImageKHR = NULL;
#if EGL_BRCM_image_update_control
static PFNEGLIMAGEUPDATEPARAMETERIVBRCMPROC   s_eglImageUpdateParameterivBRCM = NULL;
static PFNEGLIMAGEUPDATEPARAMETERIBRCMPROC    s_eglImageUpdateParameteriBRCM = NULL;
#else
static EGLSyncKHR                             SyncKHR[] = {EGL_NO_SYNC_KHR, EGL_NO_SYNC_KHR};
#endif


// default viewport size
#define   WIDTH       1920                                                             //!<   default viewport width
#define   HEIGHT      1080                                                             //!<   default viewport height

// default camera settings
#define YAW           0.0f                                                             //!<   default camera horizontal rotation
#define PITCH         0.0f                                                             //!<   default camera vertical rotation
#define ROLL          0.0f                                                             //!<   default camera roll
#define SPEED         3.0f                                                             //!<   camera movement speed
#define SENSITIVTY   0.25f                                                             //!<   mouse sensitivity
#define ZOOM         70.0f                                                             //!<   default field of view angle
#define ZOOM_MIN      5.0f                                                             //!<   minimum field of view angle
#define ZOOM_MAX    140.0f                                                             //!<   maximum field of view angle

// macro definitions
#define CLIP3(min,max,val)       (((val)<(min))?(min):(((val)>(max))?(max):(val)))     //!<  clip value between min and max
#define BUFFER_OFFSET(i)         ((char *)NULL + (i))                                  //!<  offset to buffer position


//! 3D Cube Vertex List
//------------------------------------------------
static GLfloat cube[] =
{
    -1.000000f, -1.000000f, -1.000000f,  0,  0,
    -1.000000f,  1.000000f, -1.000000f,  0,  0,
     1.000000f,  1.000000f, -1.000000f,  0,  0,
     1.000000f, -1.000000f, -1.000000f,  0,  0,

    -1.000000f, -1.000000f,  1.000000f,  0,  0,
    -1.000000f,  1.000000f,  1.000000f,  0,  0,
    -1.000000f,  1.000000f, -1.000000f,  0,  0,
    -1.000000f, -1.000000f, -1.000000f,  0,  0,

     1.000000f, -1.000000f,  1.000000f,  0,  0,
     1.000000f,  1.000000f,  1.000000f,  0,  0,
    -1.000000f,  1.000000f,  1.000000f,  0,  0,
    -1.000000f, -1.000000f,  1.000000f,  0,  0,

     1.000000f, -1.000000f, -1.000000f,  0,  0,
     1.000000f,  1.000000f, -1.000000f,  0,  0,
     1.000000f,  1.000000f,  1.000000f,  0,  0,
     1.000000f, -1.000000f,  1.000000f,  0,  0,

     1.000000f,  1.000000f, -1.000000f,  0,  0,
    -1.000000f,  1.000000f, -1.000000f,  0,  0,
    -1.000000f,  1.000000f,  1.000000f,  0,  0,
     1.000000f,  1.000000f,  1.000000f,  0,  0,

    -1.000000f, -1.000000f, -1.000000f,  0,  0,
     1.000000f, -1.000000f, -1.000000f,  0,  0,
     1.000000f, -1.000000f,  1.000000f,  0,  0,
    -1.000000f, -1.000000f,  1.000000f,  0,  0,
};

//! 3D Cube Face Index List
//------------------------------------------------
static const GLushort cube_idx[] =
{
     0,  1,  2,
     3,  0,  2,
     4,  5,  6,
     7,  4,  6,
     8,  9, 10,
    11,  8, 10,
    12, 13, 14,
    15, 12, 14,
    16, 17, 18,
    19, 16, 18,
    20, 21, 22,
    23, 20, 22
};


//! 3D Icosahedron Vertex List
//------------------------------------------------
static GLfloat icos[] =
{
    -0.7236f,   0.4472f,   0.5258f,   0.09091f,   0.00000f,
    -0.7236f,   0.4472f,   0.5258f,   0.27273f,   0.00000f,
    -0.7236f,   0.4472f,   0.5258f,   0.45455f,   0.00000f,
    -0.7236f,   0.4472f,   0.5258f,   0.63636f,   0.00000f,
    -0.7236f,   0.4472f,   0.5258f,   0.81818f,   0.00000f,

     0.0000f,   1.0000f,   0.0000f,   0.00000f,   0.33333f,
    -0.7236f,   0.4472f,  -0.5258f,   0.18182f,   0.33333f,
    -0.8944f,  -0.4472f,   0.0000f,   0.36364f,   0.33333f,
    -0.2764f,  -0.4472f,   0.8507f,   0.54545f,   0.33333f,
     0.2764f,   0.4472f,   0.8507f,   0.72727f,   0.33333f,
     0.0000f,   1.0000f,   0.0000f,   0.90909f,   0.33333f,

     0.2764f,   0.4472f,  -0.8507f,   0.09091f,   0.66667f,
    -0.2764f,  -0.4472f,  -0.8507f,   0.27273f,   0.66667f,
     0.0000f,  -1.0000f,   0.0000f,   0.45455f,   0.66667f,
     0.7236f,  -0.4472f,   0.5258f,   0.63636f,   0.66667f,
     0.8944f,   0.4472f,   0.0000f,   0.81818f,   0.66667f,
     0.2764f,   0.4472f,  -0.8507f,   1.00000f,   0.66667f,

     0.7236f,  -0.4472f,  -0.5258f,   0.18182f,   1.00000f,
     0.7236f,  -0.4472f,  -0.5258f,   0.36364f,   1.00000f,
     0.7236f,  -0.4472f,  -0.5258f,   0.54545f,   1.00000f,
     0.7236f,  -0.4472f,  -0.5258f,   0.72727f,   1.00000f,
     0.7236f,  -0.4472f,  -0.5258f,   0.90909f,   1.00000f,
};


//! 3D Icosahedron Face Index List
//------------------------------------------------
static const GLushort icos_idx[] =
{
      0,  5,  6,
      1,  6,  7,
      2,  7,  8,
      3,  8,  9,
      4,  9, 10,
      5, 11,  6,
      6, 12,  7,
      7, 13,  8,
      8, 14,  9,
      9, 15, 10,
      6, 11, 12,
      7, 12, 13,
      8, 13, 14,
      9, 14, 15,
     10, 15, 16,
     11, 17, 12,
     12, 18, 13,
     13, 19, 14,
     14, 20, 15,
     15, 21, 16,
 };


//! 3D Octahedron Vertex List
//------------------------------------------------
static GLfloat octa[] =
{
     0.0000f,   0.0000f,   1.0000f,   0.8750f,   0.0000f,
     0.0000f,   0.0000f,   1.0000f,   0.6250f,   0.0000f,
     0.0000f,   0.0000f,   1.0000f,   0.3750f,   0.0000f,
     0.0000f,   0.0000f,   1.0000f,   0.1250f,   0.0000f,

     0.0000f,   1.0000f,   0.0000f,   1.0000f,   0.5000f,
    -1.0000f,   0.0000f,   0.0000f,   0.7500f,   0.5000f,
     0.0000f,  -1.0000f,   0.0000f,   0.5000f,   0.5000f,
     1.0000f,   0.0000f,   0.0000f,   0.2500f,   0.5000f,
     0.0000f,   1.0000f,   0.0000f,   0.0000f,   0.5000f,

     0.0000f,   0.0000f,  -1.0000f,   0.8750f,   1.0000f,
     0.0000f,   0.0000f,  -1.0000f,   0.6250f,   1.0000f,
     0.0000f,   0.0000f,  -1.0000f,   0.3750f,   1.0000f,
     0.0000f,   0.0000f,  -1.0000f,   0.1250f,   1.0000f,
};


//! 3D Octahedron Face Index List
//------------------------------------------------
static const GLushort octa_idx[] =
{
      0,  4,  5,
      1,  5,  6,
      2,  6,  7,
      3,  7,  8,
      4,  9,  5,
      5, 10,  6,
      6, 11,  7,
      7, 12,  8,
 };


//*************************************************************************************
//  Configuration
//*************************************************************************************
static void  Usage(const char *progname);
static bool  ProcessArgs(int argc, const char *argv[], AppConfig *config);
static bool  InitDisplay(const AppConfig *config);


//*************************************************************************************
//  OpenGL 3D Graphics
//*************************************************************************************
static void  InitGLState(void);
static void  InitGLViewPort(unsigned int width, unsigned int height);
static void  TerminateGLState(void);
static void  SetTextureCoords();


//*************************************************************************************
//  EGL Graphics Interface
//*************************************************************************************
static void  InitEGLExtensions(void);
static bool  InitEGL(NativeWindowType eglWin, const AppConfig *config);
static void  GfxComplete(void *data, int unused);


//*************************************************************************************
//  Video Playback
//*************************************************************************************
static void  InitVideo(void);
static void  InitMediaData(MediaData *data);
static void  ConfigureVideoStream(VideoStream *stream, const MediaData *mediaData, NEXUS_DisplayHandle nexusDisplay);
static void  GetStreamData(char *filename, MediaData *data);
static void  CleanupVideoStream(VideoStream *stream);
static void  RestartVideoStream(VideoStream *stream, char *filename);
static void  TermVideo(void);


//*************************************************************************************
//  Video Texture
//*************************************************************************************
static void  UpdateVideoTexture(void);
static void  SetUpdatedTextureRegion(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
static void  LockTexture(void);
static void  UnlockTexture(void);
static bool  MakeNativePixmap(EGLNativePixmapType *retEglPixmap, NEXUS_SurfaceHandle *retNexusSurface, uint32_t w, uint32_t h);


//*************************************************************************************
//  3D Graphics Loop
//*************************************************************************************
static void  Display(void);
static void  ShowFPS(void);


//*************************************************************************************
//  Remote UI
//*************************************************************************************
static bool  RemoteUI(binput_t input);


//*************************************************************************************
//  OpenGL Camera
//*************************************************************************************
static void      CameraInit(camera_t *camera, ESVec3 pos, ESVec3 up, float yaw, float pitch, float roll);
static void      ProcessRotation(camera_t *camera, float xoffset, float yoffset, float roffset, bool constrainPitch, bool restrictView);
static ESMatrix  GetViewMatrix(camera_t *camera);
static void      UpdateCameraVectors(camera_t *camera);
static void      UpdateCameraZoom(camera_t *camera, float offset);
static ESVec3    normalize(ESVec3 vec);
static ESVec3    cross(ESVec3 vec1, ESVec3 vec2);
static float     dot(ESVec3 vec1, ESVec3 vec2);
static ESVec3    RotateVec(ESVec3 vec, ESVec3 axis, float angle);



//*************************************************************************************
//  360 Video Application Functions
//*************************************************************************************

/*!
************************************************************************
* \brief
*    main() - app entry point
*
* \param argc
*    number of input arguments
* \param argv
*    argument list
* \return
*    encoder return code
*
************************************************************************
*/
int main(int argc, const char** argv)
{
    const char  *progname = argc > 0 ? argv[0] : "";
    int         done = 0;
    binput_t    input;
    bool        restrictView = false;
    config.numVideos = 0;

    // process command line
    if(!ProcessArgs(argc, argv, &config) || config.numVideos == 0) {
        Usage(progname);
        return 0;
    }

    // restrict view for demo sequence
    //restrictView = true;

    // setup the display and EGL
    if(InitDisplay(&config)) {

        // setup EGL extension function prototypes
        InitEGLExtensions();

        // setup video playback
        InitVideo();

        // initialize OpenGL context
        InitGLState();

        // initialize OpenGL viewport
        InitGLViewPort(config.vpW, config.vpH);

        // open the remote control interface
        input = binput_open(NULL);

        // capture all remote keys
        binput_set_mask(input, 0xFFFFFFFF);


        // game loop - loop until exit is requested
        printf("Rendering: Press STOP or CLEAR to exit.\n");
        printf("  ARROW keys to rotate. \n");
        printf("  SELECT to stop rotation.\n");
        #if ENABLE_ZOOM
        printf("  FFWD and RWND to zoom.\n");
        #endif
        #if ENABLE_ROLL
        printf("  PLAY and PAUSE to roll.\n");
        #endif
        printf("  LAST or POWER to recenter.\n");
        while(true) {

            // display next frame
            Display();

            // handle remote control inputs
            if(RemoteUI(input)) {
                break;
            }

            // rotate camera
            ProcessRotation(&glCamera, xRotate, yRotate, rRotate, true, restrictView);
        }

        // close the local state for this demo
        TerminateGLState();
    }

    // close the remote control interface
    binput_close(input);

    // terminate EGL
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(eglDisplay);

    // terminate video
    TermVideo();
    NXPL_DestroyNativeWindow(nativeWindow);
    NXPL_UnregisterNexusDisplayPlatform(nxplHandle);

    // close the platform
    TermPlatform(nexusDisplay);

    return 0;
}



//*************************************************************************************
//  Configuration
//*************************************************************************************

/*!
************************************************************************
* \brief
*    Usage() - display the program usage
*
* \param progname
*    name of program
*
************************************************************************
*/
static void Usage(const char *progname)
{
    fprintf(stderr, "Usage: %s video=<filename> [+fps] [d=WxH] [o=XxY] [t=WxH] [swap=N] [client=N] [format=N]\n", progname);
    fprintf(stderr, "Formats:    0=equirect     1=cube_32_0     2=cube_32_90 \n");
    fprintf(stderr, "            3=cube_32_270  4=cube_32_p270  5=cube_43_0  \n");
    fprintf(stderr, "            6=fisheye      7=icosahedron   8=octahedron \n");
    fprintf(stderr, "            9=eap \n");
}

/*!
************************************************************************
* \brief
*    ProcessArgs() - process command line arguments
*
* \param argc
*    number of input arguments
* \param argv
*    argument list
* \return
*    TRUE if configuration is valid
*
************************************************************************
*/
static bool ProcessArgs(int argc, const char *argv[], AppConfig *config)
{
    int   argIdx;

    // quit if config structure is not valid
    if(config == NULL) {
        return false;
    }

    // set default values
    config->showFPS      = false;
    config->vpX          = 0;
    config->vpY          = 0;
    config->vpW          = WIDTH;
    config->vpH          = HEIGHT;
    config->clientId     = 0;
    config->swapInterval = 1;
    config->vFormat      = FORMAT_EQUIRECT;

    // loop through all
    for(argIdx = 1; argIdx < argc; ++argIdx)
    {
        const char *arg = argv[argIdx];

        // get video stream file names
        if(strncmp(arg, "video=", 6) == 0) {
            if(config->numVideos == 0) {
                if(sscanf(arg, "video=%s", config->videoFile0) != 1) {
                    return false;
                }
            }
            else if(config->numVideos == 1) {
                if(sscanf(arg, "video=%s", config->videoFile1) != 1) {
                    return false;
                }
            }
            else if(config->numVideos == 2) {
                if(sscanf(arg, "video=%s", config->videoFile2) != 1) {
                    return false;
                }
            }
            else if(config->numVideos == 3) {
                if(sscanf(arg, "video=%s", config->videoFile3) != 1) {
                    return false;
                }
            }
            config->numVideos++;
        }
        // set to periodically print out frames/sec
        else if(strcmp(arg, "+fps") == 0) {
            config->showFPS = true;
        }
        // set viewport size
        else if(strncmp(arg, "d=", 2) == 0) {
            if(sscanf(arg, "d=%dx%d", &config->vpW, &config->vpH) != 2) {
                return false;
            }
        }
        // set viewport position
        else if(strncmp(arg, "o=", 2) == 0) {
            if(sscanf(arg, "o=%dx%d", &config->vpX, &config->vpY) != 2) {
                return false;
            }
        }
        // set texture size
        else if(strncmp(arg, "t=", 2) == 0) {
            if(sscanf(arg, "t=%dx%d", &config->texW, &config->texH) != 2) {
                return false;
            }
        }
        // set buffer swap interval
        else if(strncmp(arg, "swap=", 5) == 0) {
            if(sscanf(arg, "swap=%d", &config->swapInterval) != 1) {
                return false;
            }
        }
        // set client ID
        else if(strncmp(arg, "client=", 7) == 0) {
            if(sscanf(arg, "client=%u", &config->clientId) != 1) {
                return false;
            }
        }
        // set 360 video format
        else if(strncmp(arg, "format=", 7) == 0) {
            if(sscanf(arg, "format=%u", &config->vFormat) != 1) {
                return false;
            }
            config->vFormat = CLIP3(0, FORMAT_TOTAL_NUM-1, config->vFormat);
        }
        // unknown parameter - display usage info and quit
        else {
            return false;
        }
    }

    // config valid
    return true;
}

/*!
************************************************************************
* \brief
*    InitDisplay() - initialize display window
*
* \param aspect
*    panel aspect ratio
* \param config
*    application config structure
* \return
*    TRUE if window initialized successfully
*
************************************************************************
*/
static bool InitDisplay(const AppConfig *config)
{
    float aspect;
    NXPL_NativeWindowInfoEXT   winInfo;

    // initalize Nexus platform and display
    printf("InitPlatformAndDefaultDisplay = %dx%d\n", config->vpW, config->vpH);
    eInitResult res = InitPlatformAndDefaultDisplay(&nexusDisplay, &aspect, config->vpW, config->vpH, false);
    if(res != eInitSuccess) {
        return false;
    }

    // register the Nexus display with the platform layer
    // the platform layer then controls the display
    NXPL_RegisterNexusDisplayPlatform(&nxplHandle, nexusDisplay);

    // get default window parameters
    NXPL_GetDefaultNativeWindowInfoEXT(&winInfo);

    // setup custom window parameters
    winInfo.x        = config->vpX;
    winInfo.y        = config->vpY;
    winInfo.width    = config->vpW;
    winInfo.height   = config->vpH;
    winInfo.stretch  = false;
    winInfo.clientID = config->clientId;

    // create window for viewport
    nativeWindow = NXPL_CreateNativeWindowEXT(&winInfo);

    // initialise EGL now that we have a 'window'
    if(!InitEGL(nativeWindow, config)) {
        return false;
    }
    return true;
}



//*************************************************************************************
//  OpenGL 3D Graphics
//*************************************************************************************

/*!
************************************************************************
* \brief
*    InitGLState() - initialize OpenGL context state
*
************************************************************************
*/
static void InitGLState(void)
{
    GLuint      v, f;
    GLint       ret;
    const char  *ff;
    const char  *vv;
    char        *p, *q, *r;
    EGLint      attrList[] = { EGL_NONE };

    NEXUS_SurfaceHandle dstSurface = NULL;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Error rc;

    // set initial camera position/direction
    ESVec3       pos   = {{0, 0, 0}};
    ESVec3       up    = {{0, 1, 0}};
    float        yaw   = YAW;
    float        pitch = PITCH;
    float        roll  = ROLL;

    // vertex shader program for equirectangular projection
    const char vShaderStrEq[] =
        "attribute vec3 position;                                             \n"
        "varying   vec3 texCoords;                                            \n"
        "uniform   mat4 model;                                                \n"
        "uniform   mat4 view;                                                 \n"
        "uniform   mat4 projection;                                           \n"
        "                                                                     \n"
        "void main()                                                          \n"
        "{                                                                    \n"
        "    gl_Position = projection * view * model * vec4(position, 1.0);   \n"
        "    texCoords   = position;                                          \n"
        "}                                                                    \n";

    // fragment shader program for equirectangular projection
    const char fShaderStrEq[] =
        "precision mediump   float;                                                                                \n"
        "uniform   sampler2D texture;                                                                              \n"
        "varying   vec3      texCoords;                                                                            \n"
        "const     float     PI = 3.141592653589793238462643383;                                                   \n"
        "                                                                                                          \n"
        "void main()                                                                                               \n"
        "{                                                                                                         \n"
        "    float longitude = ((atan(texCoords.z, texCoords.x) / PI + 1.0) * 0.5);                                \n"
        "    float length    = sqrt(texCoords.x*texCoords.x + texCoords.y*texCoords.y + texCoords.z*texCoords.z);  \n"
        "    float latitude  = 0.5 - asin(texCoords.y / length) / PI;                                              \n"
        "    vec2 latlong    = vec2(longitude, latitude);                                                          \n"
        "    gl_FragColor    = texture2D(texture, latlong);                                                        \n"
        "}                                                                                                         \n";

    // fragment shader program for equal area projection
    const char fShaderStrEap[] =
        "precision mediump   float;                                                                                \n"
        "uniform   sampler2D texture;                                                                              \n"
        "varying   vec3      texCoords;                                                                            \n"
        "const     float     PI = 3.141592653589793238462643383;                                                   \n"
        "                                                                                                          \n"
        "void main()                                                                                               \n"
        "{                                                                                                         \n"
        "    float longitude = ((atan(texCoords.z, texCoords.x) / PI + 1.0) * 0.5);                                \n"
        "    float length    = sqrt(texCoords.x*texCoords.x + texCoords.y*texCoords.y + texCoords.z*texCoords.z);  \n"
        "    float latitude  = 0.5 - (texCoords.y / length) / 2.0;                                                   \n"
        "    vec2 latlong    = vec2(longitude, latitude);                                                          \n"
        "    gl_FragColor    = texture2D(texture, latlong);                                                        \n"
        "}                                                                                                         \n";

    // fragment shader program for fisheye projection
    const char fShaderStrFish[] =
        "precision mediump   float;                                                                                \n"
        "uniform   sampler2D texture;                                                                              \n"
        "varying   vec3      texCoords;                                                                            \n"
        "const     float     PI = 3.141592653589793238462643383;                                                   \n"
        "                                                                                                          \n"
        "void main()                                                                                               \n"
        "{                                                                                                         \n"
        "    vec3 sphere;                                                                                          \n"
        "    vec2 texPos;                                                                                          \n"
        "    float R = sqrt(texCoords.x*texCoords.x + texCoords.y*texCoords.y + texCoords.z*texCoords.z);          \n"
        "    sphere.x = texCoords.x/R;                                                                             \n"
        "    sphere.y = texCoords.y/R;                                                                             \n"
        "    sphere.z = texCoords.z/R;                                                                             \n"
        "    float d = sqrt(sphere.x*sphere.x + sphere.y*sphere.y);                                                \n"
        "    float angle = atan(d,abs(sphere.z)) / 3.1415926;                                                      \n"
        "    if(d != 0.0) {                                                                                        \n"
        "        texPos.x = (angle / d) * sphere.x;                                                                \n"
        "        texPos.y = (angle / d) * sphere.y;                                                                \n"
        "    }                                                                                                     \n"
        "    else {                                                                                                \n"
        "        texPos.x = 0.0;                                                                                   \n"
        "        texPos.y = 0.0;                                                                                   \n"
        "    }                                                                                                     \n"
        "    if(texCoords.z > 0.0) {                                                                               \n"
        "        texPos.x = 0.75 - texPos.x / 2.0;                                                                 \n"
        "    }                                                                                                     \n"
        "    else {                                                                                                \n"
        "        texPos.x = 0.25 + texPos.x / 2.0;                                                                 \n"
        "    }                                                                                                     \n"
        "    texPos.y = -texPos.y + 0.5;                                                                           \n"
        "    gl_FragColor    = texture2D(texture, texPos);                                                         \n"
        "}                                                                                                         \n";

    // vertex shader program for cube projection
    const char vShaderStrCube[] =
        "attribute vec3 position;                                             \n"
        "attribute vec2 coords;                                               \n"
        "uniform   mat4 model;                                                \n"
        "uniform   mat4 view;                                                 \n"
        "uniform   mat4 projection;                                           \n"
        "varying   vec2 texCoords;                                            \n"
        "                                                                     \n"
        "void main()                                                          \n"
        "{                                                                    \n"
        "    gl_Position = projection * view * model * vec4(position, 1.0);   \n"
        "    texCoords   = coords;                                            \n"
        "}                                                                    \n";

    // fragment shader program for cube projection
    const char fShaderStrCube[] =
        "precision mediump   float;                                           \n"
        "uniform   sampler2D texture;                                         \n"
        "varying   vec2      texCoords;                                       \n"
        "                                                                     \n"
        "void main()                                                          \n"
        "{                                                                    \n"
        "    gl_FragColor = texture2D(texture, texCoords);                    \n"
        "}                                                                    \n";



    // reserve texture objects
    glGenTextures(NUM_TEX, esTextures);

    // make a new native pixmap which we will convert to an EGLImage and use as a texture
    for(unsigned int pixmapIdx = 0; pixmapIdx < NUM_TEX; pixmapIdx++) {
        if(!MakeNativePixmap(&eglPixmaps[pixmapIdx], &nativePixmaps[pixmapIdx], videoStream.outputWidth, videoStream.outputHeight)) {
            fprintf(stderr, "Failed to create native pixmap\n");
            exit(0);
        }

        // clear the pixmap with a fill
        dstSurface = nativePixmaps[pixmapIdx];
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = dstSurface;
        fillSettings.rect.width = videoStream.outputWidth;
        fillSettings.rect.height = videoStream.outputHeight;
        fillSettings.color = 0;
        rc = NEXUS_Graphics2D_Fill(gfx2d, &fillSettings);
        BDBG_ASSERT(!rc);

        // wait for the fill to complete
        do {
            rc = NEXUS_Graphics2D_Checkpoint(gfx2d, NULL);
            if(rc == NEXUS_GRAPHICS2D_QUEUED) {
                rc = BKNI_WaitForEvent(blitTextureDone, 1000);
            }
        }
        while(rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

        // wrap the native pixmap (actually a NEXUS_Surface) as an EGLImage
        eglImages[pixmapIdx] = s_eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)eglPixmaps[pixmapIdx], attrList);
        if(eglImages[pixmapIdx] == EGL_NO_IMAGE_KHR) {
            fprintf(stderr, "Failed to create EGLImage\n");
            exit(0);
        }

        // bind the EGLImage as a texture, and set filtering (don't use mipmaps)
        glBindTexture(GL_TEXTURE_2D, esTextures[pixmapIdx]);
        s_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImages[pixmapIdx]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // tell GL we will be using explicit EGL image updates if necessary
    #if EGL_BRCM_image_update_control
        if(s_eglImageUpdateParameteriBRCM) {
            s_eglImageUpdateParameteriBRCM(eglGetCurrentDisplay(), eglImages[currentTex],
                EGL_IMAGE_UPDATE_CONTROL_SET_MODE_BRCM,
                EGL_IMAGE_UPDATE_CONTROL_EXPLICIT_BRCM);
        }
    #endif

    // clear depth buffer
    glClearDepthf(1.0f);

    // clear background with gray color
    glClearColor(0.2f, 0.2f, 0.2f, 0.0);

    // enable depth testing
    glEnable(GL_DEPTH_TEST);

    // cull front faces of triangles (only need texure on the inside of cube)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    // set texture maping points for cube formats
    SetTextureCoords();

    // create vertex buffer objects for icosahedron vertices and faces
    if(config.vFormat == FORMAT_ICOSAHEDRON) {
        glGenBuffers(2, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(icos), icos, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(icos_idx), icos_idx, GL_STATIC_DRAW);
    }
    // create vertex buffer objects for octahedron vertices and faces
    else if(config.vFormat == FORMAT_OCTAHEDRON) {
        glGenBuffers(2, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(octa), octa, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(octa_idx), octa_idx, GL_STATIC_DRAW);
    }
    // create vertex buffer objects for cube vertices and faces
    else {
        glGenBuffers(2, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_idx), cube_idx, GL_STATIC_DRAW);
    }

    // create vertex and fragment shader
    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);

    // load shader programs
    if(config.vFormat == FORMAT_EQUIRECT) {
        ff = fShaderStrEq;
        vv = vShaderStrEq;
    }
    else if(config.vFormat == FORMAT_FISHEYE) {
        ff = fShaderStrFish;
        vv = vShaderStrEq;
    }
    else if(config.vFormat == FORMAT_EAP) {
        ff = fShaderStrEap;
        vv = vShaderStrEq;
    }
    else {
        ff = fShaderStrCube;
        vv = vShaderStrCube;
    }
    glShaderSource(v, 1, &vv, NULL);
    glShaderSource(f, 1, &ff, NULL);

    // compile the vertex shader
    glCompileShader(v);
    glGetShaderiv(v, GL_COMPILE_STATUS, &ret);
    if(ret == GL_FALSE) {
        glGetShaderiv(v, GL_INFO_LOG_LENGTH, &ret);
        p = (char *)alloca(ret);
        glGetShaderInfoLog(v, ret, NULL, p);
        printf("Vertex shader compile error:\n%s\n", p);
        exit(0);
    }

    // compile the fragment shader
    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &ret);
    if(ret == GL_FALSE) {
        glGetShaderiv(f, GL_INFO_LOG_LENGTH, &ret);
        q = (char *)alloca(ret);
        glGetShaderInfoLog(f, ret, NULL, q);
        printf("Fragment shader compile error:\n%s\n", q);
        exit(0);
    }

    // create GL program
    programObject = glCreateProgram();

    // attach shaders to program
    glAttachShader(programObject, v);
    glAttachShader(programObject, f);

    // link the program
    glLinkProgram(programObject);
    glGetProgramiv(programObject, GL_LINK_STATUS, &ret);
    if(ret == GL_FALSE) {
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &ret);
        if(ret > 0) {
            r = (char *)alloca(ret);
            glGetProgramInfoLog(programObject, ret, NULL, r);
            printf("Shader link error:\n%s\n", r);
            exit(0);
        }
        printf("Shader link error:\n");
        exit(0);
    }

    // get the position attribute location
    positionLoc = glGetAttribLocation(programObject, "position");
    if(config.vFormat != FORMAT_EQUIRECT &&
       config.vFormat != FORMAT_FISHEYE &&
       config.vFormat != FORMAT_EAP) {
        texCoordLoc = glGetAttribLocation(programObject, "coords");
    }

    // get the projection/view/model matrix locations
    projectionLoc = glGetUniformLocation(programObject, "projection");
    viewLoc       = glGetUniformLocation(programObject, "view");
    modelLoc      = glGetUniformLocation(programObject, "model");

    // get the texture location
    texUnitLoc   = glGetAttribLocation(programObject,  "texture");

    // load and scale model matrix
    esMatrixLoadIdentity(&model);
    esScale(&model, 500, 500, 500);

    // initialize camera at center of cube
    CameraInit(&glCamera, pos, up, yaw, pitch, roll);
    xRotate = yRotate = rRotate = 0;
}

/*!
************************************************************************
* \brief
*    InitGLViewPort() - initialize OpenGL viewport
*
* \param width
*    viewport width in pixels
* \param height
*    viewport height in pixels
*
************************************************************************
*/
static void InitGLViewPort(unsigned int width, unsigned int height)
{
    // create viewport
    glViewport(0, 0, width, height);

    // load identity matrix for projection
    esMatrixLoadIdentity(&projection);

    // apply persepective based on field of view
    esPerspective(&projection, glCamera.zoom, (float)width / (float)height, 100, 5000);
}

/*!
************************************************************************
* \brief
*    TerminateGLState() - close OpenGL context
*
************************************************************************
*/
static void TerminateGLState(void)
{
    // delete programs, buffers, and textures
    glDeleteProgram(programObject);
    glDeleteBuffers(2, vbo);
    glDeleteTextures(NUM_TEX, esTextures);
    for(unsigned int texIdx = 0; texIdx < NUM_TEX; texIdx++) {
        s_eglDestroyImageKHR(eglGetCurrentDisplay(), eglImages[texIdx]);
        NXPL_DestroyCompatiblePixmap(nxplHandle, eglPixmaps[texIdx]);
    }
}

/*!
************************************************************************
* \brief
*    SetTextureCoords() - set texture maping points for cube formats
*
************************************************************************
*/
static void SetTextureCoords()
{
    cube_fmt_t   fmt;
    cube_face_t *face;
    GLfloat      tex[8];
    float        min[2], max[2], val;
    int          xIdx, yIdx, faceIdx;
    int          texSize[2] = {config.texW, config.texH};

    // texture coordinates not used for some formats
    if((config.vFormat == FORMAT_EQUIRECT) ||
       (config.vFormat == FORMAT_FISHEYE)  ||
       (config.vFormat == FORMAT_EAP)) {
        return;
    }
    // texture coordinates manually set for some formats
    else if((config.vFormat == FORMAT_ICOSAHEDRON) ||
            (config.vFormat == FORMAT_OCTAHEDRON)) {
        return;
    }
    // cube_32_0 format
    else if(config.vFormat == FORMAT_CUBE_32_0) {
        fmt.gridW = 3;         fmt.gridH = 2;
        fmt.face[0].posX = 1;  fmt.face[0].posY = 1;  fmt.face[0].rot = 0;
        fmt.face[1].posX = 2;  fmt.face[1].posY = 0;  fmt.face[1].rot = 0;
        fmt.face[2].posX = 1;  fmt.face[2].posY = 0;  fmt.face[2].rot = 0;
        fmt.face[3].posX = 0;  fmt.face[3].posY = 0;  fmt.face[3].rot = 0;
        fmt.face[4].posX = 2;  fmt.face[4].posY = 1;  fmt.face[4].rot = 0;
        fmt.face[5].posX = 0;  fmt.face[5].posY = 1;  fmt.face[5].rot = 0;
    }
    // cube_32_90 format
    else if(config.vFormat == FORMAT_CUBE_32_90) {
        fmt.gridW = 3;         fmt.gridH = 2;
        fmt.face[0].posX = 2;  fmt.face[0].posY = 0;  fmt.face[0].rot = 0;
        fmt.face[1].posX = 1;  fmt.face[1].posY = 0;  fmt.face[1].rot = 0;
        fmt.face[2].posX = 0;  fmt.face[2].posY = 0;  fmt.face[2].rot = 0;
        fmt.face[3].posX = 1;  fmt.face[3].posY = 1;  fmt.face[3].rot = 1;
        fmt.face[4].posX = 0;  fmt.face[4].posY = 1;  fmt.face[4].rot = 1;
        fmt.face[5].posX = 2;  fmt.face[5].posY = 1;  fmt.face[5].rot = 1;
    }
    // cube_32_270 format
    else if(config.vFormat == FORMAT_CUBE_32_270) {
        fmt.gridW = 3;         fmt.gridH = 2;
        fmt.face[0].posX = 2;  fmt.face[0].posY = 0;  fmt.face[0].rot = 0;
        fmt.face[1].posX = 1;  fmt.face[1].posY = 0;  fmt.face[1].rot = 0;
        fmt.face[2].posX = 0;  fmt.face[2].posY = 0;  fmt.face[2].rot = 0;
        fmt.face[3].posX = 1;  fmt.face[3].posY = 1;  fmt.face[3].rot = 3;
        fmt.face[4].posX = 2;  fmt.face[4].posY = 1;  fmt.face[4].rot = 3;
        fmt.face[5].posX = 0;  fmt.face[5].posY = 1;  fmt.face[5].rot = 3;
    }
    // cube_32_p270 format
    else if(config.vFormat == FORMAT_CUBE_32_P270) {
        fmt.gridW = 3;         fmt.gridH = 2;
        fmt.face[0].posX = 1;  fmt.face[0].posY = 1;  fmt.face[0].rot = 3;
        fmt.face[1].posX = 2;  fmt.face[1].posY = 0;  fmt.face[1].rot = 0;
        fmt.face[2].posX = 1;  fmt.face[2].posY = 0;  fmt.face[2].rot = 0;
        fmt.face[3].posX = 0;  fmt.face[3].posY = 0;  fmt.face[3].rot = 0;
        fmt.face[4].posX = 2;  fmt.face[4].posY = 1;  fmt.face[4].rot = 0;
        fmt.face[5].posX = 0;  fmt.face[5].posY = 1;  fmt.face[5].rot = 0;
    }
    // cube_43_0 format
    else if(config.vFormat == FORMAT_CUBE_43_0) {
        fmt.gridW = 4;         fmt.gridH = 3;
        fmt.face[0].posX = 3;  fmt.face[0].posY = 1;  fmt.face[0].rot = 0;
        fmt.face[1].posX = 2;  fmt.face[1].posY = 1;  fmt.face[1].rot = 0;
        fmt.face[2].posX = 1;  fmt.face[2].posY = 1;  fmt.face[2].rot = 0;
        fmt.face[3].posX = 0;  fmt.face[3].posY = 1;  fmt.face[3].rot = 0;
        fmt.face[4].posX = 0;  fmt.face[4].posY = 0;  fmt.face[4].rot = 0;
        fmt.face[5].posX = 0;  fmt.face[5].posY = 2;  fmt.face[5].rot = 0;
    }

    // loop through each face
    for(faceIdx=0; faceIdx<6; faceIdx++) {
        face = &fmt.face[faceIdx];

        // get texture boundaries
        min[0] = (float)(face->posX + 0) / (float)fmt.gridW;
        max[0] = (float)(face->posX + 1) / (float)fmt.gridW;
        min[1] = (float)(face->posY + 0) / (float)fmt.gridH;
        max[1] = (float)(face->posY + 1) / (float)fmt.gridH;

        // match up face orientation to cube face
        if(face->rot == 0) {
            tex[0] = min[0];  tex[1] = max[1];
            tex[2] = min[0];  tex[3] = min[1];
            tex[4] = max[0];  tex[5] = min[1];
            tex[6] = max[0];  tex[7] = max[1];
        }
        else if(face->rot == 1) {
            tex[0] = max[0];  tex[1] = max[1];
            tex[2] = min[0];  tex[3] = max[1];
            tex[4] = min[0];  tex[5] = min[1];
            tex[6] = max[0];  tex[7] = min[1];
        }
        else if(face->rot == 2) {
            tex[0] = max[0];  tex[1] = min[1];
            tex[2] = max[0];  tex[3] = max[1];
            tex[4] = min[0];  tex[5] = max[1];
            tex[6] = min[0];  tex[7] = min[1];
        }
        else {
            tex[0] = min[0];  tex[1] = min[1];
            tex[2] = max[0];  tex[3] = min[1];
            tex[4] = max[0];  tex[5] = max[1];
            tex[6] = min[0];  tex[7] = max[1];
        }

        // round to nearest textel within face
        for(yIdx=0; yIdx<4; yIdx++) {
            for(xIdx=0; xIdx<2; xIdx++) {

                // round highest coordinate down, round lowest coordinate up
                val = tex[2*yIdx + xIdx];
                if(val == max[xIdx]) {
                    val = (val * texSize[xIdx] - 0.5) / texSize[xIdx];
                }
                else {
                    val = (val * texSize[xIdx] + 0.5) / texSize[xIdx];
                }

                // set texture coordinate within cube array
                cube[20*faceIdx + 5*yIdx + 3 + xIdx] = val;
            }
        }
    }
}



//*************************************************************************************
//  EGL Graphics Interface
//*************************************************************************************

/*!
************************************************************************
* \brief
*    InitEGLExtensions() - map the function pointers for the EGL extensions we will be using (if they exist)
*
************************************************************************
*/
static void InitEGLExtensions(void)
{
    s_glEGLImageTargetTexture2DOES     =  (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    s_eglCreateImageKHR                =  (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    s_eglDestroyImageKHR               =  (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
    #if EGL_BRCM_image_update_control
    s_eglImageUpdateParameterivBRCM    =  (PFNEGLIMAGEUPDATEPARAMETERIVBRCMPROC)eglGetProcAddress("eglImageUpdateParameterivBRCM");
    s_eglImageUpdateParameteriBRCM     =  (PFNEGLIMAGEUPDATEPARAMETERIBRCMPROC)eglGetProcAddress("eglImageUpdateParameteriBRCM");
    #endif
    if(!s_glEGLImageTargetTexture2DOES || !s_eglCreateImageKHR || !s_eglDestroyImageKHR) {
        printf("Error: EGLImage texturing is not supported. Cannot continue.\n");
        exit(0);
    }
}

/*!
************************************************************************
* \brief
*    InitEGL() - initialize Open GL context
*
* \param eglWin
*    display window
* \param config
*    application config structure
* \return
*    TRUE if context initialized successfully
*
************************************************************************
*/
static bool InitEGL(NativeWindowType eglWin, const AppConfig *config)
{
    int        configs;
    int        configSelect = 0;
    EGLSurface eglSurface = 0;
    EGLContext eglContext = 0;
    EGLConfig *eglConfig;
    EGLint     majorVersion;
    EGLint     minorVersion;

    // get the EGL display
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(eglDisplay == EGL_NO_DISPLAY) {
        printf("eglGetDisplay() failed\n");
        return false;
    }

    // initialize EGL
    if(!eglInitialize(eglDisplay, &majorVersion, &minorVersion)) {
        printf("eglInitialize() failed\n");
        return false;
    }

    // get the number of configurations to correctly size the array used in next step
    if(!eglGetConfigs(eglDisplay, NULL, 0, &configs)) {
        printf("eglGetConfigs() failed\n");
        return false;
    }
    eglConfig = (EGLConfig *)alloca(configs * sizeof(EGLConfig));

    // get an array of configurations to find one matching requiements (RGBA pixel format)
    {
        const int   NUM_ATTRIBS = 21;
        EGLint      *attr = (EGLint *)malloc(NUM_ATTRIBS * sizeof(EGLint));
        int         i = 0;

        attr[i++] = EGL_RED_SIZE;        attr[i++] = 8;
        attr[i++] = EGL_GREEN_SIZE;      attr[i++] = 8;
        attr[i++] = EGL_BLUE_SIZE;       attr[i++] = 8;
        attr[i++] = EGL_ALPHA_SIZE;      attr[i++] = 8;
        attr[i++] = EGL_DEPTH_SIZE;      attr[i++] = 24;
        attr[i++] = EGL_STENCIL_SIZE;    attr[i++] = 0;
        attr[i++] = EGL_SURFACE_TYPE;    attr[i++] = EGL_WINDOW_BIT;
        attr[i++] = EGL_RENDERABLE_TYPE; attr[i++] = EGL_OPENGL_ES2_BIT;
        attr[i++] = EGL_NONE;
        assert(i <= NUM_ATTRIBS);

        if(!eglChooseConfig(eglDisplay, attr, eglConfig, configs, &configs) || (configs == 0)) {
            printf("eglChooseConfig() failed");
            return false;
        }
        free(attr);
    }

    // find a config in the returned array that is an exact match
    for(configSelect = 0; configSelect < configs; configSelect++) {
        EGLint redSize, greenSize, blueSize, alphaSize, depthSize;
        eglGetConfigAttrib(eglDisplay, eglConfig[configSelect], EGL_RED_SIZE,   &redSize);
        eglGetConfigAttrib(eglDisplay, eglConfig[configSelect], EGL_GREEN_SIZE, &greenSize);
        eglGetConfigAttrib(eglDisplay, eglConfig[configSelect], EGL_BLUE_SIZE,  &blueSize);
        eglGetConfigAttrib(eglDisplay, eglConfig[configSelect], EGL_ALPHA_SIZE, &alphaSize);
        eglGetConfigAttrib(eglDisplay, eglConfig[configSelect], EGL_DEPTH_SIZE, &depthSize);
        if((redSize == 8) && (greenSize == 8) && (blueSize == 8) && (alphaSize == 8)) {
            printf("Selected config: R=%d G=%d B=%d A=%d Depth=%d\n", redSize, greenSize, blueSize, alphaSize, depthSize);
            break;
        }
    }
    if(configSelect == configs) {
        printf("No suitable configs found\n");
        return false;
    }

    // create a surface to draw on
    {
        const int   NUM_ATTRIBS = 3;
        EGLint      *attr = (EGLint *)malloc(NUM_ATTRIBS * sizeof(EGLint));
        int         i = 0;
        attr[i++]  = EGL_NONE;
        eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig[configSelect], eglWin, attr);
    }
    if(eglSurface == EGL_NO_SURFACE) {
        eglGetError();
        eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig[configSelect], NULL, NULL);
    }
    if(eglSurface == EGL_NO_SURFACE) {
        printf("eglCreateWindowSurface() failed\n");
        return false;
    }

    // create an OpenGL context
    {
        const int   NUM_ATTRIBS = 5;
        EGLint      *attr = (EGLint *)malloc(NUM_ATTRIBS * sizeof(EGLint));
        int         i = 0;
        attr[i++] = EGL_CONTEXT_CLIENT_VERSION; attr[i++] = 2;
        attr[i++] = EGL_NONE;
        eglContext = eglCreateContext(eglDisplay, eglConfig[configSelect], EGL_NO_CONTEXT, attr);
        if(eglContext == EGL_NO_CONTEXT) {
            printf("eglCreateContext() failed");
            return false;
        }
        free(attr);
    }

    // bind the context to the current thread and use our window surface for drawing and reading
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    eglSwapInterval(eglDisplay, config->swapInterval);
    return true;
}

/*!
************************************************************************
* \brief
*    GfxComplete() - event callback that will be called when a 2D operation is complete
*
* \param data
*    pointer to callback data
* \param unused
*    unused parameter in function prototype
*
************************************************************************
*/
static void GfxComplete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}



//*************************************************************************************
//  Video Playback
//*************************************************************************************

/*!
************************************************************************
* \brief
*    InitVideo() - initialize video playback
*
************************************************************************
*/
static void InitVideo(void)
{
    MediaData                  mediaData;
    NEXUS_Graphics2DSettings   gfxSettings;
    NEXUS_Graphics2DOpenSettings graphics2dOpenSettings;

    // initialize video stream
    InitMediaData(&mediaData);
    GetStreamData(config.videoFile0, &mediaData);
    ConfigureVideoStream(&videoStream, &mediaData, nexusDisplay);

    // create an event that will be triggered when the blit/fill is complete
    BKNI_CreateEvent(&blitTextureDone);

    // prepare the 2D graphics module
    NEXUS_Graphics2D_GetDefaultOpenSettings(&graphics2dOpenSettings);
    graphics2dOpenSettings.secure = videoStream.secure;
    gfx2d = NEXUS_Graphics2D_Open(0, &graphics2dOpenSettings);
    if(gfx2d == NULL) {
        printf("NEXUS_Graphics2D_Open failed\n");
        exit(0);
    }

    // tell 2D graphics about our 'done' callback
    NEXUS_Graphics2D_GetSettings(gfx2d, &gfxSettings);
    gfxSettings.checkpointCallback.callback = GfxComplete;
    gfxSettings.checkpointCallback.context = blitTextureDone;
    NEXUS_Graphics2D_SetSettings(gfx2d, &gfxSettings);
}

/*!
************************************************************************
* \brief
*    InitMediaData() - initialize media data structure
*
* \param data
*    media data
*
************************************************************************
*/
static void InitMediaData(MediaData *data)
{
    data->width        =   0;
    data->height       =   0;
    data->filename[0]  =   '\0';
}

/*!
************************************************************************
* \brief
*    ConfigureVideoStream() - configure video stream for playback
*
* \param stream
*    stream parameters
* \param mediaData
*    media data parameters
* \param nexusDisplay
*    nexus display handle
*
************************************************************************
*/
static void ConfigureVideoStream(VideoStream *stream, const MediaData *mediaData, NEXUS_DisplayHandle nexusDisplay)
{
    NEXUS_SimpleVideoDecoderStartCaptureSettings videoCaptureSettings;
    NEXUS_SimpleAudioDecoderSettings             audioSettings;
    NEXUS_SurfaceCreateSettings                  createSettings;
    NxClient_AllocSettings                       allocSettings;
    NxClient_ConnectSettings                     connectSettings;
    media_player_create_settings                 mediaCreateSettings;
    media_player_start_settings                  mediaStartSettings;
    NEXUS_ClientConfiguration                    clientConfig;
    NxClient_JoinSettings                        joinSettings;
    int                                          i;
    NEXUS_Error                                  rc;

    // intialize stream parameters
    memset(stream, 0, sizeof(VideoStream));
    stream->nexusDisplay = nexusDisplay;

    // get video source size
    stream->sourceWidth  = mediaData->width;
    stream->sourceHeight = mediaData->height;
    printf("Media source size: %dx%d\n", stream->sourceWidth, stream->sourceHeight);

    // set texture size equal to video size unless otherwise requested
    if(config.texW == 0) {
        stream->outputWidth  = stream->sourceWidth;
        stream->outputHeight = stream->sourceHeight;
    }
    else {
        stream->outputWidth = config.texW;
        stream->outputHeight = config.texH;
    }
    printf("Texture size: %dx%d\n", stream->outputWidth, stream->outputHeight);

    // get default media player settings
    media_player_get_default_create_settings(&mediaCreateSettings);
    media_player_get_default_start_settings(&mediaStartSettings);

    // join nxclient to server
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "360 video - %s", mediaData->filename);
    rc = NxClient_Join(&joinSettings);
    if(rc) {
        CleanupVideoStream(stream);
        exit(0);
    }

    // get platform client configuration
    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    // allocate client memory
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if(rc != NEXUS_SUCCESS) {
        printf("NxClient allocation failed with error: %d", rc);
        CleanupVideoStream(stream);
        exit(0);
    }

    // get handle of surface client
    stream->surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);

    // create media player
    mediaCreateSettings.window.surfaceClientId = allocResults.surfaceClient[0].id;
    mediaCreateSettings.window.id = 0;
    stream->mediaPlayer = media_player_create(&mediaCreateSettings);
    if(!stream->mediaPlayer) {
        printf("Media player create failed");
        CleanupVideoStream(stream);
        exit(0);
    }

#if ENABLE_AMBISONIC_AUDIO
    // get audio decoder resource
    stream->audioDecoder = media_player_get_audio_decoder(stream->mediaPlayer);
    if(!stream->audioDecoder) {
        printf("Get audio decoder failed");
        CleanupVideoStream(stream);
        exit(0);
    }

    // setup ambisonic audio processor
    NEXUS_SimpleAudioDecoder_GetSettings(stream->audioDecoder, &audioSettings);
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.connected = true;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.contentType = NEXUS_AmbisonicContentType_eAmbisonic;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.connectorType = NEXUS_AudioConnectorType_eMultichannel;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.yaw = 0;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.pitch = 0;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.roll = 0;
    NEXUS_SimpleAudioDecoder_SetSettings(stream->audioDecoder, &audioSettings);
#endif

    // start media player
    mediaStartSettings.videoWindowType = NxClient_VideoWindowType_eNone;
    mediaStartSettings.stream_url = mediaData->filename;
    rc = media_player_start(stream->mediaPlayer, &mediaStartSettings);
    if(rc) {
        printf("Media player start failed");
        CleanupVideoStream(stream);
        exit(0);
    }

    // create video surfaces
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    #ifdef BIG_ENDIAN_CPU
    createSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;
    #else
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    #endif
    createSettings.width  = stream->outputWidth;
    createSettings.height = stream->outputHeight;
    printf("Surface Size: %dx%d\n", createSettings.width, createSettings.height);
    for(i=0; i<NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++) {
        stream->videoSurfaces[i] = NEXUS_Surface_Create(&createSettings);
    }

    // get video decoder resource
    stream->videoDecoder = media_player_get_video_decoder(stream->mediaPlayer);
    if(!stream->videoDecoder) {
        printf("Get video decoder failed");
        CleanupVideoStream(stream);
        exit(0);
    }

    // create video capture to fill surfaces
    NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&videoCaptureSettings);
    BKNI_Memcpy(&videoCaptureSettings.surface, &stream->videoSurfaces, sizeof(stream->videoSurfaces));
    videoCaptureSettings.forceFrameDestripe = true;
    videoCaptureSettings.secure = stream->secure = false;
    rc = NEXUS_SimpleVideoDecoder_StartCapture(stream->videoDecoder, &videoCaptureSettings);
    if(rc == NEXUS_NOT_SUPPORTED) {
        printf("Video as graphics not supported");
        CleanupVideoStream(stream);
        exit(0);
    }
}

/*!
************************************************************************
* \brief
*    GetStreamData() - examine the media file and gather information
*
* \param filename
*    video stream filename
* \param data
*    media data parameters
*
************************************************************************
*/
static void GetStreamData(char *filename, MediaData *data)
{
    bmedia_probe_t probe = NULL;
    bmedia_probe_config probeCfg;
    const bmedia_probe_stream *stream = NULL;
    const bmedia_probe_track *track = NULL;
    bfile_io_read_t fd = NULL;
    bpcm_file_t pcmFile = NULL;
    FILE *fin;

    // save file name
    strcpy(data->filename, filename);

    // open input file
    fin = fopen64(filename, "rb");
    if(!fin) {
        printf("Cannot open video file");
        exit(0);
    }

    // attach input file
    fd = bfile_stdio_read_attach(fin);

    // create media probe
    probe = bmedia_probe_create();

    // set media probe configuration
    bmedia_probe_default_cfg(&probeCfg);
    probeCfg.file_name = filename;
    probeCfg.type = bstream_mpeg_type_unknown;

    // probe stream to get info
    stream = bmedia_probe_parse(probe, pcmFile ? bpcm_file_get_file_interface( pcmFile ) : fd, &probeCfg);

    // handle cdxa streams
    if(stream && stream->type == bstream_mpeg_type_cdxa) {
        bcdxa_file_t cdxa_file;
        cdxa_file = bcdxa_file_create(fd);

        if(cdxa_file) {
            const bmedia_probe_stream *cdxa_stream;
            cdxa_stream = bmedia_probe_parse(probe, bcdxa_file_get_file_interface(cdxa_file), &probeCfg);
            bcdxa_file_destroy(cdxa_file);

            if(cdxa_stream) {
                bmedia_probe_stream_free(probe, stream);
                stream = cdxa_stream;
            }
        }
    }

    // handle pcm streams
    if(pcmFile) {
        bpcm_file_destroy(pcmFile);
    }

    // detach file and close
    bfile_stdio_read_detach(fd);
    fclose(fin);

    // check for valid stream
    if(!stream) {
        printf("Video stream cannot be parsed");
        exit(0);
    }

    // find video track
    for(track = BLST_SQ_FIRST(&stream->tracks); track; track = BLST_SQ_NEXT(track, link)) {

        // record video info
        if(track->type == bmedia_track_type_video) {
            data->width = track->info.video.width;
            data->height = track->info.video.height;
            break;
        }
    }

    // free probe
    if(probe) {
        if(stream) {
            bmedia_probe_stream_free(probe, stream);
        }
        bmedia_probe_destroy(probe);
    }
    return;
}

/*!
************************************************************************
* \brief
*    CleanupVideoStream() - cleanup the video stream
*
* \param stream
*    stream parameters
*
************************************************************************
*/
static void CleanupVideoStream(VideoStream *stream)
{
    unsigned int i;

    // destroy the surfaces used by the decoder for the captures
    for(i = 0; i < NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++) {
        if(stream->videoSurfaces[i]) {
            NEXUS_Surface_Destroy(stream->videoSurfaces[i]);
            stream->videoSurfaces[i] = NULL;
        }
    }

    // stop video decoder
    if(stream->videoDecoder) {
        NEXUS_SimpleVideoDecoder_StopCapture(stream->videoDecoder);
    }

    // stop media player
    if(stream->mediaPlayer) {
        media_player_stop(stream->mediaPlayer);
        media_player_destroy(stream->mediaPlayer);
    }

    // release surface client
    if(stream->surfaceClient) {
        NEXUS_SurfaceClient_Release(stream->surfaceClient);
    }

    // disconnect NxClient
    NxClient_Free(&allocResults);

    // release video decoder
    if(stream->videoDecoder) {
        NEXUS_SimpleVideoDecoder_Release(stream->videoDecoder);
        stream->videoDecoder = NULL;
    }
}

/*!
************************************************************************
* \brief
*    TermVideo() - terminate video playback
*
************************************************************************
*/
static void TermVideo(void)
{
    CleanupVideoStream(&videoStream);
    if(gfx2d) {
        NEXUS_Graphics2D_Close(gfx2d);
        gfx2d = NULL;
    }
    if(blitTextureDone) {
        BKNI_DestroyEvent(blitTextureDone);
        blitTextureDone = NULL;
    }
}

/*!
************************************************************************
* \brief
*    RestartVideoStream() - restart the video stream with a different file
*
* \param stream
*    stream parameters
* \param filename
*    filename
*
************************************************************************
*/
static void RestartVideoStream(VideoStream *stream, char *filename)
{
    NEXUS_SimpleVideoDecoderStartCaptureSettings videoCaptureSettings;
    NEXUS_SurfaceCreateSettings                  createSettings;
    media_player_start_settings                  mediaStartSettings;
    NEXUS_Error                                  rc;
    int                                          i;

    // destroy the surfaces used by the decoder for the captures
    for(i = 0; i < NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++) {
        if(stream->videoSurfaces[i]) {
            NEXUS_Surface_Destroy(stream->videoSurfaces[i]);
            stream->videoSurfaces[i] = NULL;
        }
    }

    // stop video capture
    if(stream->videoDecoder) {
        NEXUS_SimpleVideoDecoder_StopCapture(stream->videoDecoder);
    }

    // stop media player
    if(stream->mediaPlayer) {
        media_player_stop(stream->mediaPlayer);
    }

    // start media player
    media_player_get_default_start_settings(&mediaStartSettings);
    mediaStartSettings.videoWindowType = NxClient_VideoWindowType_eNone;
    mediaStartSettings.stream_url = filename;
    rc = media_player_start(stream->mediaPlayer, &mediaStartSettings);
    if(rc) {
        printf("Media player start failed");
        CleanupVideoStream(stream);
        exit(0);
    }

    // create video surfaces
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    #ifdef BIG_ENDIAN_CPU
    createSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;
    #else
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    #endif
    createSettings.width  = stream->outputWidth;
    createSettings.height = stream->outputHeight;
    printf("Surface Size: %dx%d\n", createSettings.width, createSettings.height);
    for(i=0; i<NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++) {
        stream->videoSurfaces[i] = NEXUS_Surface_Create(&createSettings);
    }

    // create video capture to fill surfaces
    NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&videoCaptureSettings);
    BKNI_Memcpy(&videoCaptureSettings.surface, &stream->videoSurfaces, sizeof(stream->videoSurfaces));
    videoCaptureSettings.forceFrameDestripe = true;
    videoCaptureSettings.secure = stream->secure = false;
    rc = NEXUS_SimpleVideoDecoder_StartCapture(stream->videoDecoder, &videoCaptureSettings);
    if(rc == NEXUS_NOT_SUPPORTED) {
        printf("Video as graphics not supported");
        CleanupVideoStream(stream);
        exit(0);
    }
}



//*************************************************************************************
//  Video Texture
//*************************************************************************************

/*!
************************************************************************
* \brief
*    UpdateVideoTexture() - put any new video frames into the video texture
*
************************************************************************
*/
static void UpdateVideoTexture(void)
{
    NEXUS_SurfaceHandle                    captureSurface;
    NEXUS_SimpleVideoDecoderCaptureStatus  captureStatus;
    NEXUS_Graphics2DBlitSettings           blitSettings;
    BKNI_EventHandle                       checkpointEvent;
    unsigned                               numReturned;
    NEXUS_Error                            rc;

    BKNI_CreateEvent(&checkpointEvent);

    // try to capture a frame from the simple decoder
    NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(videoStream.videoDecoder, &captureSurface,&captureStatus, 1, &numReturned);

    // update display status count
    totalCnt++;

    // sleep if not frame available
    if(numReturned == 0) {
        waitCnt++;
        BKNI_Sleep(5);
    }
    else {

        // lock the destination buffer for writing.
        // this might take some time if the 3D core is using it right now
        LockTexture();

        #if !EGL_BRCM_image_update_control
        currentTex++;
        if(currentTex >= NUM_TEX) {
            currentTex = 0;
        }

        if(SyncKHR[currentTex] != EGL_NO_SYNC_KHR) {
            eglClientWaitSyncKHR(eglDisplay, SyncKHR[currentTex], EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, EGL_FOREVER_KHR);
        }
        #endif

        NEXUS_SurfaceHandle dstSurface = nativePixmaps[currentTex];

        // blit the captured frame onto a texture
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = captureSurface;
        blitSettings.output.surface = dstSurface;
        rc = NEXUS_Graphics2D_Blit(gfx2d, &blitSettings);

        // wait for the blitter to finish
        do {
            rc = NEXUS_Graphics2D_Checkpoint(gfx2d, NULL);
            if(rc == NEXUS_GRAPHICS2D_QUEUED) {
                rc = BKNI_WaitForEvent(blitTextureDone, 1000);
            }
        }
        while(rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

        // release the capture surface for further capture
        NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(videoStream.videoDecoder, &captureSurface, numReturned);

        // tell V3D we've changed it - all of it
        SetUpdatedTextureRegion(0, 0, videoStream.outputWidth, videoStream.outputHeight);

        // unlock the texture so V3D can use it
        UnlockTexture();
    }
}

/*!
************************************************************************
* \brief
*    SetUpdatedTextureRegion() - tells the 3D driver that a portion (or all) of the texture has been modified
*
* \param x
*    left edge of updated region
* \param y
*    top edge of updated region
* \param w
*    updated region width
* \param h
*    updated region height
*
************************************************************************
*/
static void SetUpdatedTextureRegion(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    #if EGL_BRCM_image_update_control
    if(s_eglImageUpdateParameterivBRCM) {
        EGLint rect[4];
        rect[0] = x;
        rect[1] = y;
        rect[2] = w;
        rect[3] = h;
        s_eglImageUpdateParameterivBRCM(eglGetCurrentDisplay(), eglImages[currentTex],
                                        EGL_IMAGE_UPDATE_CONTROL_CHANGED_REGION_BRCM, rect);
    }
    #else
    BSTD_UNUSED(x);
    BSTD_UNUSED(y);
    BSTD_UNUSED(w);
    BSTD_UNUSED(h);
    #endif
}

/*!
************************************************************************
* \brief
*    LockTexture() - lock texture to avoid prevent 3D driver from using it while it is being modified
*
************************************************************************
*/
static void LockTexture(void)
{
    #if EGL_BRCM_image_update_control
    if(s_eglImageUpdateParameteriBRCM) {
        s_eglImageUpdateParameteriBRCM(eglGetCurrentDisplay(), eglImages[currentTex],
            EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM,
            EGL_IMAGE_UPDATE_CONTROL_LOCK_BRCM);
    }
    #endif
}

/*!
************************************************************************
* \brief
*    UnlockTexture() - unlock texture for 3D driver
*
************************************************************************
*/
static void UnlockTexture(void)
{
    #if EGL_BRCM_image_update_control
    if(s_eglImageUpdateParameteriBRCM) {
        s_eglImageUpdateParameteriBRCM(eglGetCurrentDisplay(), eglImages[currentTex],
            EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM,
            EGL_IMAGE_UPDATE_CONTROL_UNLOCK_BRCM);
    }
    #endif
}

/*!
************************************************************************
* \brief
*    MakeNativePixmap() - create a native pixmap (Nexus surface) with appropriate constraints for use as a texture
*
* \param retEglPixmap
*    viewport width in pixels
* \param retNexusSurface
*    viewport height in pixels
* \param w
*    viewport width in pixels
* \param h
*    viewport height in pixels
* \return
*    TRUE if successful
*
************************************************************************
*/
static bool MakeNativePixmap(EGLNativePixmapType *retEglPixmap, NEXUS_SurfaceHandle *retNexusSurface, uint32_t w, uint32_t h)
{
    BEGL_PixmapInfoEXT   pixInfo;

    // get default pixmap settings
    NXPL_GetDefaultPixmapInfoEXT(&pixInfo);

    // customize the pixmap settings
    pixInfo.width = w;
    pixInfo.height = h;
    pixInfo.secure = false;
    #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    pixInfo.format = BEGL_BufferFormat_eR8G8B8A8;
    #else
    pixInfo.format = BEGL_BufferFormat_eA8B8G8R8;
    #endif

    // create pixmap
    return NXPL_CreateCompatiblePixmapEXT(nxplHandle, retEglPixmap, retNexusSurface, &pixInfo);
}



//*************************************************************************************
//  3D Graphics Loop
//*************************************************************************************

/*!
************************************************************************
* \brief
*    Display() - called once in each frame loop to refresh the display
*
************************************************************************
*/
static void Display(void)
{
    uint32_t v;
    uint32_t numVert;

    // set number of vertices in vertex mesh
    if(config.vFormat == FORMAT_ICOSAHEDRON) {
        numVert = 60;
    }
    else if(config.vFormat == FORMAT_OCTAHEDRON) {
        numVert = 24;
    }
    else {
        numVert = 36;
    }

    // display frame/sec performance if requested
    if(config.showFPS) {
        ShowFPS();
    }

    // update the video frame textures (will leave the textures untouched if no new frame is ready)
    UpdateVideoTexture();

    // clear all the buffers we asked for during config to ensure fast-path
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // activate program
    glUseProgram(programObject);

    // enable cube array
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(positionLoc);
    if(config.vFormat != FORMAT_EQUIRECT &&
       config.vFormat != FORMAT_FISHEYE  &&
       config.vFormat != FORMAT_EAP) {
        glVertexAttribPointer(texCoordLoc,  2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(texCoordLoc);
    }

    // get current view matrix based on camera position
    view = GetViewMatrix(&glCamera);

    // get projection matrix based on camera zoom
    esMatrixLoadIdentity(&projection);
    esPerspective(&projection, glCamera.zoom, (float)config.vpW / (float)config.vpH, 100, 5000);

    // load the model/view/projection matrices
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (GLfloat*)&projection.m[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (GLfloat*)&view.m[0][0]);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (GLfloat*)&model.m[0][0]);

    // load the texture
    glUniform1i(texUnitLoc, 0);

    // bind cube vertices to buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

    // draw the elements
    glBindTexture(GL_TEXTURE_2D, esTextures[currentTex]);
    glDrawElements(GL_TRIANGLES, numVert, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
    #if !EGL_BRCM_image_update_control
        if(SyncKHR[currentTex] != EGL_NO_SYNC_KHR) {
            eglDestroySyncKHR(eglDisplay, SyncKHR[currentTex]);
        }
        SyncKHR[currentTex]  = eglCreateSyncKHR(eglDisplay, EGL_SYNC_FENCE_KHR, NULL);
    #endif

    // post the frame buffer for display
    eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_READ));
}

/*!
************************************************************************
* \brief
*    ShowFPS() - show the current frame-rate performance
*
************************************************************************
*/
static void ShowFPS(void)
{
    struct timeval curTime;
    int            nowMs;
    static int     lastPrintTime  = 0;
    static int     lastPrintFrame = 0;
    static int     frame = 0;

    // get current time
    gettimeofday(&curTime, NULL);
    nowMs = curTime.tv_usec / 1000;
    nowMs += curTime.tv_sec * 1000;

    // incrment frame counter
    frame++;

    // aprrox once per second, compute time difference between current and previous call and update last time
    if(nowMs - lastPrintTime > 1000 || lastPrintFrame == 0) {
        if(nowMs - lastPrintTime != 0 && lastPrintTime != 0) {
            float fps = (float)(frame - lastPrintFrame) / ((float)(nowMs - lastPrintTime) / 1000.0f);
            if(totalCnt != 0) {
                printf("Frames per second: %f     wait %% = %f\n", fps, (float)waitCnt/(float)totalCnt);
                printf("yaw = %3d   pitch = %3d   roll = %3d\n", glCamera.yawDeg, glCamera.pitchDeg, glCamera.rollDeg);
                waitCnt = totalCnt = 0;
            }
        }
        lastPrintFrame = frame;
        lastPrintTime  = nowMs;
    }
}



//*************************************************************************************
//  Remote UI
//*************************************************************************************

/*!
************************************************************************
* \brief
*    RemoteUI() - handle remote keypresses
*
* \param input
*    remote input interface structure
* \return
*    TRUE if program exit was requested
*
************************************************************************
*/
static bool RemoteUI(binput_t input)
{
    b_remote_key key;

    // get next remote key if any
    if(binput_read_no_repeat(input, &key) == 0) {

        // chandle key values
        switch(key) {

        // up button - start upward rotation or increase speed
        case b_remote_key_up:
            if(yRotate < 0) {
                yRotate = 0;
            }
            else if(yRotate < 0.03) {
                yRotate += 0.01;
            }
            break;

        // down button - start downward rotation or increase speed
        case b_remote_key_down:
            if(yRotate > 0) {
                yRotate = 0;
            }
            else if(yRotate > -0.03) {
                yRotate += -0.01;
            }
            break;

        // left button - start left rotation or increase speed
        case b_remote_key_left:
            if(xRotate > 0) {
                xRotate = 0;
            }
            else if(xRotate > -0.06){
                xRotate -= 0.02;
            }
            break;

        // right button - start right rotation or increase speed
        case b_remote_key_right:
            if(xRotate < 0) {
                xRotate = 0;
            }
            else if(xRotate < 0.06){
                xRotate += 0.02;
            }
            break;

        // stop button - start left roll or increase speed
        case b_remote_key_play:
            #if ENABLE_ROLL
            if(rRotate < 0) {
                rRotate = 0;
            }
            else if(rRotate < 0.06) {
                rRotate += 0.02;
            }
            #endif
            break;

        // pause button - start right roll or increase speed
        case b_remote_key_pause:
            #if ENABLE_ROLL
            if(rRotate > 0) {
                rRotate = 0;
            }
            else if(rRotate > -0.06) {
                rRotate -= 0.02;
            }
            #endif
            break;

        // select button - cancel all rotation
        case b_remote_key_select:
            xRotate = 0;
            yRotate = 0;
            rRotate = 0;
            break;

        // fast-forward button - use for zoom in
        case b_remote_key_fast_forward:
            #if ENABLE_ZOOM
            UpdateCameraZoom(&glCamera, -5);
            #endif
            break;

        // rewind button - use for zoom out
        case b_remote_key_rewind:
            #if ENABLE_ZOOM
            UpdateCameraZoom(&glCamera, +5);
            #endif
            break;

        // last or power button - reset camera to center
        case b_remote_key_back:
        case b_remote_key_power:
        case b_remote_key_info:
        case b_remote_key_guide:
        case b_remote_key_menu:
        case b_remote_key_clear:
            xRotate = 0;
            yRotate = 0;
            rRotate = 0;
            glCamera.yaw   = YAW;
            glCamera.pitch = PITCH;
            glCamera.roll  = ROLL;
            break;

        // stop/clear buttons
        //case b_remote_key_stop:
        //case b_remote_key_clear:
            //return true;
            //break;

        // number 0 button
        case b_remote_key_zero:
            if(config.numVideos > 0 && activeVideo != 0) {
                RestartVideoStream(&videoStream, config.videoFile0);
                activeVideo = 0;
            }
            break;

        // number 1 button
        case b_remote_key_one:
            if(config.numVideos > 1 && activeVideo != 1) {
                RestartVideoStream(&videoStream, config.videoFile1);
                activeVideo = 1;
            }
            break;

        // number 2 button
        case b_remote_key_two:
            if(config.numVideos > 2 && activeVideo != 2) {
                RestartVideoStream(&videoStream, config.videoFile2);
                activeVideo = 2;
            }
            break;

        // number 3 button
        case b_remote_key_three:
            if(config.numVideos > 3 && activeVideo != 3) {
                RestartVideoStream(&videoStream, config.videoFile3);
                activeVideo = 3;
            }
            break;

        // any other values
        default:
            break;
        }
    }
    return false;
}



//*************************************************************************************
//  OpenGL Camera
//*************************************************************************************

/*!
************************************************************************
* \brief
*    CameraInit() - camera constructor
*
* \param camera
*    camera structure
* \param pos
*    camera position
* \param up
*    camera up vector
* \param yaw
*    horizontal camera rotation
* \param pitch
*    vertical camera rotation
* \param roll
*    camera roll
*
************************************************************************
*/
static void CameraInit(camera_t *camera, ESVec3 pos, ESVec3 up, float yaw, float pitch, float roll)
{
    // configure camera settings
    camera->position = pos;
    camera->worldUp  = up;
    camera->yaw      = yaw;
    camera->pitch    = pitch;
    camera->roll     = roll;

    // set default values
    camera->front.v[0]       =  0;
    camera->front.v[1]       =  0;
    camera->front.v[2]       = -1;
    camera->movementSpeed    = SPEED;
    camera->sensitivity      = SENSITIVTY;
    camera->zoom             = ZOOM;

    // initialize camera vectors
    UpdateCameraVectors(camera);
}

/*!
************************************************************************
* \brief
*    ProcessRotation() - process input for camera rotation
*
* \param camera
*    camera structure
* \param xoffset
*    horizontal rotation
* \param yoffset
*    vertical rotation
* \param roffset
*    roll rotation
* \param constrainPitch
*    constrain camera rotation so pitch cannot exceed +/- 180 degrees
* \param restrictView
*    restriced view for specific demo test sequence
*
************************************************************************
*/
static void ProcessRotation(camera_t *camera, float xoffset, float yoffset, float roffset, bool constrainPitch, bool restrictView)
{
    NEXUS_SimpleAudioDecoderSettings audioSettings;
    int yawDegrees=0, pitchDegrees=0, rollDegrees=0;

    // scale offset by sensitivity
    xoffset *= camera->sensitivity;
    yoffset *= camera->sensitivity;
    roffset *= camera->sensitivity;

    // adjust camera rotation to follow mouse
    camera->yaw   += xoffset;
    camera->pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if(constrainPitch) {
        if(camera->pitch > 1.57f) {
            yRotate = 0;
            camera->pitch = 1.57f;
        }
        if(camera->pitch < -1.57f) {
            yRotate = 0;
            camera->pitch = -1.57f;
        }
    }
    if(restrictView) {
        if(camera->pitch < -0.279f) {
            yRotate = 0;
            camera->pitch = -0.279f;
        }
        if(camera->pitch > 0.436f) {
            yRotate = 0;
            camera->pitch = 0.436f;
        }
    }

    // get angles in degrees for ambisonic processor
    yawDegrees   = (int)(180*camera->yaw / 3.14159);
    pitchDegrees = (int)(180*camera->pitch / 3.14159);

    // keep angles within 0-359
    while(yawDegrees < 0) {
        yawDegrees += 360;
    }
    while(yawDegrees >= 360) {
        yawDegrees -= 360;
    }
    while(pitchDegrees < 0) {
        pitchDegrees += 360;
    }
    while(pitchDegrees >= 360) {
        pitchDegrees -= 360;
    }

    // update roll if enabled
    #if ENABLE_ROLL
    camera->roll += roffset;
    rollDegrees = (int)(180*camera->roll / 3.14159);
    while(rollDegrees < 0) {
        rollDegrees += 360;
    }
    while(rollDegrees >= 360) {
        rollDegrees -= 360;
    }
    #endif

    // store camera angles in degrees
    camera->yawDeg = yawDegrees;
    camera->pitchDeg = pitchDegrees;
    camera->rollDeg = rollDegrees;

    // update camera angles
    UpdateCameraVectors(camera);

#if ENABLE_AMBISONIC_AUDIO
    // setup ambisonic audio processor
    NEXUS_SimpleAudioDecoder_GetSettings(videoStream.audioDecoder, &audioSettings);
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.yaw = yawDegrees;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.pitch = pitchDegrees;
    audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.roll = rollDegrees;
    NEXUS_SimpleAudioDecoder_SetSettings(videoStream.audioDecoder, &audioSettings);
#endif
}

/*!
************************************************************************
* \brief
*    GetViewMatrix() - returns the view matrix calculated using Euler angles and the LookAt matrix
*
* \param camera
*    camera structure
* \return
*    view transformation matrix
*
************************************************************************
*/
static ESMatrix GetViewMatrix(camera_t *camera)
{
    ESMatrix output;
    ESVec3  f = camera->front;
    ESVec3  u = camera->up;
    ESVec3  s = camera->right;

    output.m[0][0] = s.v[0];
    output.m[1][0] = s.v[1];
    output.m[2][0] = s.v[2];
    output.m[3][0] =   0.0f;
    output.m[0][1] = u.v[0];
    output.m[1][1] = u.v[1];
    output.m[2][1] = u.v[2];
    output.m[3][1] =   0.0f;
    output.m[0][2] =-f.v[0];
    output.m[1][2] =-f.v[1];
    output.m[2][2] =-f.v[2];
    output.m[3][2] =   0.0f;
    output.m[3][0] =-dot(s, camera->position);
    output.m[3][1] =-dot(u, camera->position);
    output.m[3][2] = dot(f, camera->position);
    output.m[3][3] =   1.0f;
    return output;
}

/*!
************************************************************************
* \brief
*    UpdateCameraVectors() - calculate front/right/up vectors for camera given current position and LookAt vectors
*
* \param camera
*    camera structure
*
************************************************************************
*/
static void UpdateCameraVectors(camera_t *camera)
{
    ESVec3 front, right;

    // calculate the new front vector
    front.v[0] = cos(camera->yaw) * cos(camera->pitch);
    front.v[1] = sin(camera->pitch);
    front.v[2] = sin(camera->yaw) * cos(camera->pitch);
    camera->front = normalize(front);

    // calculate right vector perpendicular to front and worldUp
    right = normalize(cross(camera->front, camera->worldUp));

    // rotate right vector according to camera roll
    #if ENABLE_ROLL
    right = RotateVec(right, camera->front, camera->roll);
    camera->right = normalize(right);
    #else
    camera->right = right;
    #endif

    // calculate the camera up vector perpendicular to right and front
    camera->up = normalize(cross(camera->right, camera->front));
}

/*!
************************************************************************
* \brief
*    UpdateCameraZoom() - update camera zoom parameter
*
* \param camera
*    camera structure
* \param offset
*    zoom offset in degrees
*
************************************************************************
*/
static void UpdateCameraZoom(camera_t *camera, float offset)
{
    glCamera.zoom += offset;
    glCamera.zoom = CLIP3(ZOOM_MIN, ZOOM_MAX, glCamera.zoom);
}

/*!
************************************************************************
* \brief
*    normalize() - normalize vector length
*
* \param vec
*    input vector
* \return
*    normalized output vector
*
************************************************************************
*/
static ESVec3 normalize(ESVec3 vec)
{
    ESVec3 output;
    float  length;
    length = sqrt(vec.v[0]*vec.v[0] + vec.v[1]*vec.v[1] + vec.v[2]*vec.v[2]);
    output.v[0] = vec.v[0] / length;
    output.v[1] = vec.v[1] / length;
    output.v[2] = vec.v[2] / length;
    return output;
}

/*!
************************************************************************
* \brief
*    cross() - compute cross product of two vectors
*
* \param vec1
*    first input vector
* \param vec2
*    second input vector
* \return
*    cross product of input vectors
*
************************************************************************
*/
static ESVec3 cross(ESVec3 vec1, ESVec3 vec2)
{
    ESVec3 output;
    output.v[0] = vec1.v[1] * vec2.v[2] - vec1.v[2] * vec2.v[1];
    output.v[1] = vec1.v[2] * vec2.v[0] - vec1.v[0] * vec2.v[2];
    output.v[2] = vec1.v[0] * vec2.v[1] - vec1.v[1] * vec2.v[0];
    return output;
}

/*!
************************************************************************
* \brief
*    dot() - compute dot product of two vectors
*
* \param vec1
*    first input vector
* \param vec2
*    second input vector
* \return
*    dot product of input vectors
*
************************************************************************
*/
static float dot(ESVec3 vec1, ESVec3 vec2)
{
    return vec1.v[0] * vec2.v[0] + vec1.v[1] * vec2.v[1] + vec1.v[2] * vec2.v[2];
}

/*!
************************************************************************
* \brief
*    RotateVec() - rotate a vector around an axis
*
* \param vec
*    vector to rotate
* \param axis
*    axis around which to rotate vec1
* \param angle
*    angle of rotation
* \return
*    rotated vector
*
************************************************************************
*/
static ESVec3 RotateVec(ESVec3 vec, ESVec3 axis, float angle)
{
    ESVec3 output;
    float d = dot(vec, axis);
    float x = vec.v[0],  y = vec.v[1],  z = vec.v[2];
    float u = axis.v[0], v = axis.v[1], w = axis.v[2];
    float c = cos(angle);
    float s = sin(angle);

    // rotate vector around the axis
    output.v[0] = u*d*(1-c) + x*c + (-w*y + v*z)*s;
    output.v[1] = v*d*(1-c) + y*c + ( w*x - u*z)*s;
    output.v[2] = w*d*(1-c) + z*c + (-v*x + u*y)*s;
    return output;
}
