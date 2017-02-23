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
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/


/* NOTE:
 * SWVC4-472 : Add a NxClient video texturing sample to rockford/applications
 *             At the moment only one decoder per client is supported and
 *             only one client can use the decoder
 */

#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>

#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#define GL_GLEXT_PROTOTYPES
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

#ifndef SINGLE_PROCESS
#include "nxclient.h"
#endif

#define NUM_CUBE_FACES 6

typedef struct
{
   bool         showFPS;
   bool         useMultisample;
   bool         stretchToFit;
   bool         benchmark;
   int          vpX;
   int          vpY;
   int          vpW;
   int          vpH;
   int          bpp;
   int          frames;
   int          swapInterval;
   unsigned     clientId;
   unsigned     numVideos;
   char         videoFile[PATH_MAX];
   bool         showbackgroundvideo;
   int          texW;
   int          texH;
   bool         secure;
} AppConfig;

typedef struct
{
   uint32_t                         width;
   uint32_t                         height;
   uint32_t                         duration;
   uint32_t                         videoPid;
   uint32_t                         pcrPid;
   uint32_t                         audioPid;
   uint32_t                         extVideoPid;
   NEXUS_TransportType              transportType;
   NEXUS_TransportTimestampType     tsTimestampType;
   NEXUS_VideoCodec                 videoCodec;
   NEXUS_VideoCodec                 extVideoCodec;
   NEXUS_AudioCodec                 audioCodec;
   NEXUS_VideoDecoderTimestampMode  decoderTimestampMode;
   char                             filename[PATH_MAX];
} MediaData;


typedef struct
{
   NEXUS_SimpleVideoDecoderHandle   videoDecoder;
   NEXUS_FilePlayHandle             filePlay;
   NEXUS_PlaypumpHandle             playPump;
   NEXUS_PlaybackHandle             playBack;
   NEXUS_SimpleStcChannelHandle     simpleStcChannel;
   NEXUS_PidChannelHandle           videoPidChannel;
   NEXUS_DisplayHandle              nexusDisplay;
   uint32_t                         sourceWidth;
   uint32_t                         sourceHeight;
   uint32_t                         outputWidth;
   uint32_t                         outputHeight;
   float                            outputAspect;
   NEXUS_SurfaceClientHandle        surfaceClient;
   unsigned                         connectId;
   NEXUS_SurfaceHandle              videoSurfaces[NEXUS_SIMPLE_DECODER_MAX_SURFACES];
} VideoStream;

static NxClient_AllocResults allocResults;

/*Default configuration values*/
const unsigned int WIDTH      = 1280;
const unsigned int HEIGHT     = 720;
const unsigned int FRAMES     = 0;
const unsigned int BPP        = 32;

static AppConfig              config;

static NEXUS_DISPLAYHANDLE    nexus_display = 0;

static void                   *native_window = 0;
static float                  panelAspect = 1.0f;

static NXPL_PlatformHandle    nxpl_handle = 0;

static ESMatrix               projection_matrix;
static ESMatrix               modelview_matrix;
static ESMatrix               model_matrix;
static ESMatrix               view_matrix;
static ESMatrix               mvp_matrix;
static const ESMatrix         color_matrices[NUM_CUBE_FACES] =
{
   {{
      { 1.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 1.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 1.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 0.8f }
   }},
   {{
      { 1.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 1.0f, 0.0f },
      { 0.0f, 1.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 0.8f }
   }},
   {{
      { 0.0f, 1.0f, 0.0f, 0.0f },
      { 1.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 1.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 0.8f }
   }},
   {{
      { 2.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 2.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 2.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 0.8f }
   }},
   {{
      { 0.33f, 0.33f, 0.33f, 0.0f },
      { 0.33f, 0.33f, 0.33f, 0.0f },
      { 0.33f, 0.33f, 0.33f, 0.0f },
      { 0.0f,  0.0f,  0.0f,  0.8f }
   }},
   {{
      { -1.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, -1.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, -1.0f, 0.0f },
      { 1.0f, 1.0f, 1.0f, 0.8f  }
   }}
};

static GLint                  mvp_matrix_loc;
static GLint                  position_loc;
static GLint                  tc_loc;
static GLint                  tex_unit_loc;
static GLint                  col_matrix_loc;

static GLint                  program_object;
static GLuint                 vbo[2];

static VideoStream            videoStream;
static BKNI_EventHandle       blitTextureDone;
static NEXUS_Graphics2DHandle gfx2d;

static EGLDisplay             egl_display = EGL_NO_DISPLAY;

static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC    s_glEGLImageTargetTexture2DOES = NULL;
static PFNEGLCREATEIMAGEKHRPROC               s_eglCreateImageKHR = NULL;
static PFNEGLDESTROYIMAGEKHRPROC              s_eglDestroyImageKHR = NULL;
#if EGL_BRCM_image_update_control
static PFNEGLIMAGEUPDATEPARAMETERIVBRCMPROC   s_eglImageUpdateParameterivBRCM = NULL;
static PFNEGLIMAGEUPDATEPARAMETERIBRCMPROC    s_eglImageUpdateParameteriBRCM = NULL;
#define NUM_TEX  1
#else
#define NUM_TEX  2
static EGLSyncKHR     m_fences[] = {EGL_NO_SYNC_KHR, EGL_NO_SYNC_KHR};
#endif

static EGLNativePixmapType    eglPixmaps[NUM_TEX];
static NEXUS_SurfaceHandle    nativePixmaps[NUM_TEX];
static EGLImageKHR            eglImages[NUM_TEX];
static GLuint                 esTextures[NUM_TEX];
static unsigned int           current_tex = 0;

static const GLfloat cube[] =
{
   /*          POSITION                        TEXCOORD     */
   1.000000f, 1.000000f, -1.000000f,     1.000000f, 1.000000f,
   1.000000f, -1.000000f, -1.000000f,    1.000000f, 0.000000f,
   -1.000000f, -1.000000f, -1.000000f,   0.000000f, 0.000000f,
   -1.000000f, 1.000000f, -1.000000f,    0.000000f, 1.000000f,

   -1.000000f, -1.000000f, 1.000000f,    0.000000f, 1.000000f,
   -1.000000f, 1.000000f, 1.000000f,     1.000000f, 1.000000f,
   -1.000000f, 1.000000f, -1.000000f,    1.000000f, 0.000000f,
   -1.000000f, -1.000000f, -1.000000f,   0.000000f, 0.000000f,

   1.000000f, -1.000000f, 1.000000f,     1.000000f, 1.000000f,
   1.000000f, 1.000000f, 1.000001f,      1.000000f, 0.000000f,
   -1.000000f, -1.000000f, 1.000000f,    0.000000f, 1.000000f,
   -1.000000f, 1.000000f, 1.000000f,     0.000000f, 0.000000f,

   1.000000f, -1.000000f, -1.000000f,    0.000000f, 1.000000f,
   1.000000f, 1.000000f, -1.000000f,     1.000000f, 1.000000f,
   1.000000f, -1.000000f, 1.000000f,     0.000000f, 0.000000f,
   1.000000f, 1.000000f, 1.000001f,      1.000000f, 0.000000f,

   1.000000f, 1.000000f, -1.000000f,     1.000000f, 0.000000f,
   -1.000000f, 1.000000f, -1.000000f,    0.000000f, 0.000000f,
   1.000000f, 1.000000f, 1.000001f,      1.000000f, 1.000000f,
   -1.000000f, 1.000000f, 1.000000f,     0.000000f, 1.000000f,

   1.000000f, -1.000000f, -1.000000f,    1.000000f, 1.000000f,
   1.000000f, -1.000000f, 1.000000f,     1.000000f, 0.000000f,
   -1.000000f, -1.000000f, 1.000000f,    0.000000f, 0.000000f,
   -1.000000f, -1.000000f, -1.000000f,   0.000000f, 1.000000f
};

static const GLushort cube_idx[] =
{
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
   23, 20, 22
};

static void CleanupVideoStream(VideoStream *stream);
static void UpdateVideoTexture(void);

static void InitMediaData(MediaData *data)
{
   data->width = 0;
   data->height = 0;
   data->duration = 0;
   data->videoPid = 0;
   data->pcrPid = 0;
   data->audioPid = 0;
   data->extVideoPid = 0;
   data->transportType = NEXUS_TransportType_eUnknown;
   data->tsTimestampType = NEXUS_TransportTimestampType_eNone;
   data->videoCodec = NEXUS_VideoCodec_eUnknown;
   data->extVideoCodec = NEXUS_VideoCodec_eUnknown;
   data->audioCodec = NEXUS_AudioCodec_eUnknown;
   data->decoderTimestampMode = NEXUS_VideoDecoderTimestampMode_eDecode;
   data->filename[0] = '\0';
}

static NEXUS_TransportType MapTransportType(bstream_mpeg_type type)
{
   NEXUS_TransportType ntype;

   switch ((int)type)
   {
   case bstream_mpeg_type_unknown: ntype = NEXUS_TransportType_eTs; break;
   case bstream_mpeg_type_es: ntype = NEXUS_TransportType_eEs; break;
   case bstream_mpeg_type_bes: ntype = NEXUS_TransportType_eTs; break;
   case bstream_mpeg_type_pes: ntype = NEXUS_TransportType_eMpeg2Pes; break;
   case bstream_mpeg_type_ts: ntype = NEXUS_TransportType_eTs; break;
   case bstream_mpeg_type_dss_es: ntype = NEXUS_TransportType_eDssEs; break;
   case bstream_mpeg_type_dss_pes: ntype = NEXUS_TransportType_eDssPes; break;
   case bstream_mpeg_type_vob: ntype = NEXUS_TransportType_eVob; break;
   case bstream_mpeg_type_asf: ntype = NEXUS_TransportType_eAsf; break;
   case bstream_mpeg_type_avi: ntype = NEXUS_TransportType_eAvi; break;
   case bstream_mpeg_type_mpeg1: ntype = NEXUS_TransportType_eMpeg1Ps; break;
   case bstream_mpeg_type_mp4: ntype = NEXUS_TransportType_eMp4; break;
   case bstream_mpeg_type_mkv: ntype = NEXUS_TransportType_eMkv; break;
   case bstream_mpeg_type_wav: ntype = NEXUS_TransportType_eWav; break;
   case bstream_mpeg_type_mp4_fragment: ntype = NEXUS_TransportType_eMp4Fragment; break;
   case bstream_mpeg_type_rmff: ntype = NEXUS_TransportType_eRmff; break;
   case bstream_mpeg_type_flv: ntype = NEXUS_TransportType_eFlv; break;
   case bstream_mpeg_type_ogg: ntype = NEXUS_TransportType_eOgg; break;
   default: ntype = NEXUS_TransportType_eTs; break;
   }

   return ntype;
}

static NEXUS_AudioCodec MapAudioCodec(baudio_format format)
{
   NEXUS_AudioCodec codec;

   switch ((int)format)
   {
   case baudio_format_unknown: codec = NEXUS_AudioCodec_eUnknown; break;
   case baudio_format_mpeg: codec = NEXUS_AudioCodec_eMpeg; break;
   case baudio_format_mp3: codec = NEXUS_AudioCodec_eMp3; break;
   case baudio_format_aac: codec = NEXUS_AudioCodec_eAac; break;
   case baudio_format_aac_plus: codec = NEXUS_AudioCodec_eAacPlus; break;
   case baudio_format_aac_plus_adts: codec = NEXUS_AudioCodec_eAacPlusAdts; break;
   /*case baudio_format_aac_plus_loas: codec = NEXUS_AudioCodec_eAacPlusLoas; break;*/
   case baudio_format_ac3: codec = NEXUS_AudioCodec_eAc3; break;
   case baudio_format_ac3_plus: codec = NEXUS_AudioCodec_eAc3Plus; break;
   case baudio_format_dts: codec = NEXUS_AudioCodec_eDts; break;
   case baudio_format_lpcm_hddvd: codec = NEXUS_AudioCodec_eLpcmHdDvd; break;
   case baudio_format_lpcm_bluray: codec = NEXUS_AudioCodec_eLpcmBluRay; break;
   case baudio_format_dts_hd: codec = NEXUS_AudioCodec_eDtsHd; break;
   case baudio_format_wma_std: codec = NEXUS_AudioCodec_eWmaStd; break;
   case baudio_format_wma_pro: codec = NEXUS_AudioCodec_eWmaPro; break;
   case baudio_format_lpcm_dvd: codec = NEXUS_AudioCodec_eLpcmDvd; break;
   case baudio_format_avs: codec = NEXUS_AudioCodec_eAvs; break;
   case baudio_format_amr: codec = NEXUS_AudioCodec_eAmr; break;
   case baudio_format_dra: codec = NEXUS_AudioCodec_eDra; break;
   case baudio_format_cook: codec = NEXUS_AudioCodec_eCook; break;
   case baudio_format_pcm: codec = NEXUS_AudioCodec_ePcmWav; break;
   case baudio_format_adpcm: codec = NEXUS_AudioCodec_eAdpcm; break;
   case baudio_format_dvi_adpcm: codec = NEXUS_AudioCodec_eAdpcm; break;
   case baudio_format_vorbis: codec = NEXUS_AudioCodec_eVorbis; break;
   default: codec = NEXUS_AudioCodec_eUnknown; break;
   }

   return codec;
}

static NEXUS_VideoCodec MapVideoCodec(bvideo_codec format)
{
   NEXUS_AudioCodec codec;

   switch ((int)format)
   {
   case bvideo_codec_mpeg1: codec = NEXUS_VideoCodec_eMpeg1; break;
   case bvideo_codec_mpeg2: codec = NEXUS_VideoCodec_eMpeg2; break;
   case bvideo_codec_mpeg4_part2: codec = NEXUS_VideoCodec_eMpeg4Part2; break;
   case bvideo_codec_h263: codec = NEXUS_VideoCodec_eH263; break;
   case bvideo_codec_h264: codec = NEXUS_VideoCodec_eH264; break;
   case bvideo_codec_h264_svc: codec = NEXUS_VideoCodec_eH264_Svc; break;
   case bvideo_codec_h264_mvc: codec = NEXUS_VideoCodec_eH264_Mvc; break;
   case bvideo_codec_vc1: codec = NEXUS_VideoCodec_eVc1; break;
   case bvideo_codec_vc1_sm: codec = NEXUS_VideoCodec_eVc1SimpleMain; break;
   case bvideo_codec_divx_311: codec = NEXUS_VideoCodec_eDivx311; break;
   case bvideo_codec_rv40: codec = NEXUS_VideoCodec_eRv40; break;
   case bvideo_codec_vp6: codec = NEXUS_VideoCodec_eVp6; break;
   case bvideo_codec_vp8: codec = NEXUS_VideoCodec_eVp8; break;
   case bvideo_codec_spark: codec = NEXUS_VideoCodec_eSpark; break;
   case bvideo_codec_avs: codec = NEXUS_VideoCodec_eAvs; break;
   default: codec = NEXUS_VideoCodec_eUnknown; break;
   }

   return codec;
}

/* Examine the media file and gather information */
static void GetStreamData(char *filename, MediaData *data)
{
   bmedia_probe_t probe = NULL;
   bmedia_probe_config probe_config;
   const bmedia_probe_stream *stream = NULL;
   const bmedia_probe_track *track = NULL;
   bfile_io_read_t fd = NULL;
   bpcm_file_t pcm_file = NULL;
   bool foundAudio = false, foundVideo = false;
   bool detectAvcExtension = true;
   FILE *fin;

   strcpy(data->filename, filename);

   probe = bmedia_probe_create();

   fin = fopen64(filename, "rb");

   if (!fin)
   {
      printf("Cannot open video file");
      goto error1;
   }

   fd = bfile_stdio_read_attach(fin);

   bmedia_probe_default_cfg(&probe_config);

   probe_config.file_name = filename;
   probe_config.type = bstream_mpeg_type_unknown;
   stream = bmedia_probe_parse(probe, pcm_file ? bpcm_file_get_file_interface( pcm_file ) : fd, &probe_config);

   if (stream && stream->type == bstream_mpeg_type_cdxa)
   {
      bcdxa_file_t cdxa_file;
      cdxa_file = bcdxa_file_create(fd);

      if (cdxa_file)
      {
         const bmedia_probe_stream *cdxa_stream;
         cdxa_stream = bmedia_probe_parse(probe, bcdxa_file_get_file_interface(cdxa_file), &probe_config);
         bcdxa_file_destroy(cdxa_file);

         if (cdxa_stream)
         {
            bmedia_probe_stream_free(probe, stream);
            stream = cdxa_stream;
         }
      }
   }

   if (pcm_file)
      bpcm_file_destroy(pcm_file);

   /* Now stream is either NULL, or stream descriptor with linked list of audio/video tracks */
   bfile_stdio_read_detach(fd);

   fclose(fin);

   if (!stream)
   {
      printf("Video stream cannot be parsed");
      goto error1;
   }

   data->duration = stream->duration;
   data->transportType = MapTransportType(stream->type);

   if (stream->type == bstream_mpeg_type_ts && ((bmpeg2ts_probe_stream*)stream)->pkt_len == 192)
   {
      if (data->tsTimestampType == NEXUS_TransportTimestampType_eNone)
         data->tsTimestampType = NEXUS_TransportTimestampType_eMod300;
   }

   for (track = BLST_SQ_FIRST(&stream->tracks); track; track = BLST_SQ_NEXT(track, link))
   {
      foundAudio = false;
      foundVideo = false;

      switch (track->type)
      {
      case bmedia_track_type_audio:
         if (track->info.audio.codec != baudio_format_unknown && !foundAudio)
         {
            data->audioPid = track->number;
            data->audioCodec = MapAudioCodec(track->info.audio.codec);
            foundAudio = true;
         }
         break;

      case bmedia_track_type_video:
         data->width = track->info.video.width;
         data->height = track->info.video.height;

         if (track->info.video.codec == bvideo_codec_h264_svc || track->info.video.codec == bvideo_codec_h264_mvc )
         {
            if (detectAvcExtension)
            {
               data->extVideoPid = track->number;
               data->extVideoCodec = MapVideoCodec(track->info.video.codec);
            }
         }
         else if (track->info.video.codec != bvideo_codec_unknown && !foundVideo)
         {
            data->videoPid = track->number;
            data->videoCodec = MapVideoCodec(track->info.video.codec);
            foundVideo = true;

            /* timestamp reordering can be done at the host or decoder.
               to do it at the decoder, disable it at the host and use media_probe to
               determine the correct decoder timestamp mode */
            if (false) /*playpumpTimestampReordering == false)*/
               data->decoderTimestampMode = (NEXUS_VideoDecoderTimestampMode)track->info.video.timestamp_order;
         }

         break;

      case bmedia_track_type_pcr:
         data->pcrPid = track->number;
         break;

      default:
         break;
      }

      if (foundVideo)
         break;
   }

   if (probe)
   {
      if (stream)
         bmedia_probe_stream_free(probe, stream);

      bmedia_probe_destroy(probe);
   }

   return;

error1:
   exit(0);
}

/* Configure a video stream for playback */
static void ConfigureVideoStream(VideoStream *stream, const MediaData *mediaData,
                                 NEXUS_DisplayHandle nexusDisplay)
{
   NEXUS_PlaypumpOpenSettings                   playpumpOpenSettings;
   NEXUS_PlaybackSettings                       playbackSettings;
   NEXUS_PlaybackPidChannelSettings             pidChannelSettings;
   NEXUS_SimpleVideoDecoderStartSettings        videoProgram;
   NEXUS_SimpleVideoDecoderStartCaptureSettings videoCaptureSettings;
   NEXUS_SurfaceCreateSettings                  createSettings;
   NxClient_AllocSettings                       allocSettings;
   NxClient_ConnectSettings                     connectSettings;
   int                                          i;
   NEXUS_Error                                  rc;

   stream->videoDecoder     = NULL;
   stream->filePlay         = NULL;
   stream->playPump         = NULL;
   stream->playBack         = NULL;
   stream->simpleStcChannel = NULL;
   stream->videoPidChannel  = NULL;
   stream->sourceWidth      = 0;
   stream->sourceHeight     = 0;
   stream->outputWidth      = 0;
   stream->outputHeight     = 0;
   stream->nexusDisplay     = nexusDisplay;
   stream->connectId        = 0;

   CleanupVideoStream(stream);

   stream->sourceWidth  = mediaData->width;
   stream->sourceHeight = mediaData->height;

   printf("Media source size: %dx%d\n", stream->sourceWidth, stream->sourceHeight);

   stream->outputAspect = (float)stream->sourceWidth / (float)stream->sourceHeight;

   /* Calculate a good size for the video texture. Don't make it bigger than the original though. */
   if (config.texW == 0)
   {
      stream->outputWidth = config.vpW / 2;
      if (stream->outputWidth > stream->sourceWidth)
         stream->outputWidth = stream->sourceWidth;
   }
   else
   {
      stream->outputWidth = config.texW;
   }

   stream->outputHeight = stream->outputWidth / stream->outputAspect;

   printf("Texture size: %dx%d\n", stream->outputWidth, stream->outputHeight);

   NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
   stream->playPump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
   if (stream->playPump == NULL)
   {
      printf("NEXUS_Playpump_Open failed\n");
      goto error1;
   }

   stream->playBack = NEXUS_Playback_Create();
   if (stream->playBack == NULL)
   {
      printf("NEXUS_Playback_Create failed\n");
      goto error1;
   }

   stream->simpleStcChannel = NEXUS_SimpleStcChannel_Create(NULL);
   if (stream->simpleStcChannel == NULL)
   {
      printf("NEXUS_SimpleStcChannel_Create failed\n");
      goto error1;
   }

   /* open stream file */
   /* 2nd argument is the index */
   stream->filePlay = NEXUS_FilePlay_OpenPosix(mediaData->filename, mediaData->filename);
   if (stream->filePlay == NULL)
   {
      printf("Failed to open video file\n");
      goto error1;
   }

   /* setup playback */
   NEXUS_Playback_GetSettings(stream->playBack, &playbackSettings);
   playbackSettings.playpump                       = stream->playPump;
   playbackSettings.playpumpSettings.transportType = mediaData->transportType;
   playbackSettings.endOfStreamAction              = NEXUS_PlaybackLoopMode_eLoop;
   playbackSettings.simpleStcChannel               = stream->simpleStcChannel;
   playbackSettings.stcTrick                       = true;
   NEXUS_Playback_SetSettings(stream->playBack, &playbackSettings);

   /*Allocate the number of video streams / decoders*/
   NxClient_GetDefaultAllocSettings(&allocSettings);
   allocSettings.simpleVideoDecoder = 1;
   rc = NxClient_Alloc(&allocSettings, &allocResults);
   if (rc != NEXUS_SUCCESS)
   {
      printf("NxClient allocation failed with error: %d", rc);
      goto error1;
   }

   if (allocResults.simpleVideoDecoder[0].id)
   {
      stream->videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
   }

   if (!stream->videoDecoder)
   {
      printf("video decoder not available\n");
      goto error1;
   }

   /* video */
   NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
   pidChannelSettings.pidSettings.pidType                 = NEXUS_PidType_eVideo;
   pidChannelSettings.pidTypeSettings.video.codec         = mediaData->videoCodec;
   pidChannelSettings.pidTypeSettings.video.simpleDecoder = stream->videoDecoder;
   pidChannelSettings.pidTypeSettings.video.index         = true;

   stream->videoPidChannel = NEXUS_Playback_OpenPidChannel(stream->playBack, mediaData->videoPid, &pidChannelSettings);
   if (stream->videoPidChannel == NULL)
   {
      printf("NEXUS_Playback_OpenPidChannel failed\n");
      goto error1;
   }

   NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
   videoProgram.settings.codec      = mediaData->videoCodec;
   videoProgram.settings.pidChannel = stream->videoPidChannel;
   videoProgram.displayEnabled      = config.showbackgroundvideo;

   /* connect client resources to server's resources */
   NxClient_GetDefaultConnectSettings(&connectSettings);
   if (videoProgram.settings.pidChannel)
   {
       connectSettings.simpleVideoDecoder[0].id       = allocResults.simpleVideoDecoder[0].id;
       connectSettings.simpleVideoDecoder[0].windowId = 0;

       connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth  = stream->sourceWidth;
       connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = stream->sourceHeight;
   }
   else
   {
      printf("No pid channel\n");
   }

   connectSettings.simpleVideoDecoder[0].decoderCapabilities.secureVideo = config.secure;
   rc = NxClient_Connect(&connectSettings, &stream->connectId);
   if (rc != NEXUS_SUCCESS)
   {
       printf("NxClient_Connect failed - is the server running?\n");
       goto error1;
   }

   /* Need to create surface for the capture*/
   NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
#ifdef BIG_ENDIAN_CPU
   createSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;
#else
   createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
#endif
   createSettings.width  = stream->outputWidth;
   createSettings.height = stream->outputHeight;
   if (config.secure)
      createSettings.heap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE);

   NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&videoCaptureSettings);
   for (i = 0; i < NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++)
   {
      stream->videoSurfaces[i]        = NEXUS_Surface_Create(&createSettings);
      videoCaptureSettings.surface[i] = stream->videoSurfaces[i];
   }
   videoCaptureSettings.displayEnabled = config.showbackgroundvideo;
   videoCaptureSettings.secure = config.secure;

   /* Start decode */
   NEXUS_SimpleVideoDecoder_Start(stream->videoDecoder, &videoProgram);
   NEXUS_SimpleVideoDecoder_StartCapture(stream->videoDecoder, &videoCaptureSettings);

   /* start playback */
   NEXUS_Playback_Start(stream->playBack, stream->filePlay, NULL);

   return;

error1:
   CleanupVideoStream(stream);
   exit(0);
}

/* Cleanup a video stream */
static void CleanupVideoStream(VideoStream *stream)
{
   unsigned int i;

   /* Destroy the surfaces used by the decoder for the captures */
   for (i = 0; i < NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++)
   {
      if (stream->videoSurfaces[i])
      {
         NEXUS_Surface_Destroy(stream->videoSurfaces[i]);
         stream->videoSurfaces[i] = NULL;
      }
   }

   if (stream->playBack)
   {
      NEXUS_Playback_Stop(stream->playBack);
   }

   if (stream->videoDecoder)
   {
      NEXUS_SimpleVideoDecoder_StopCapture(stream->videoDecoder);
      NEXUS_SimpleVideoDecoder_Stop(stream->videoDecoder);
   }

   if (stream->videoPidChannel)
   {
      NEXUS_Playback_ClosePidChannel(stream->playBack, stream->videoPidChannel);
      stream->videoPidChannel = NULL;
   }

   if (stream->simpleStcChannel)
   {
      stream->simpleStcChannel = NULL;
   }

   if (stream->connectId)
   {
      NxClient_Disconnect(stream->connectId);
   }

   NxClient_Free(&allocResults);

   if (stream->filePlay)
   {
      NEXUS_FilePlay_Close(stream->filePlay);
      stream->filePlay = NULL;
   }

   if (stream->videoDecoder)
   {
      NEXUS_SimpleVideoDecoder_Release(stream->videoDecoder);
      stream->videoDecoder = NULL;
   }

   if (stream->playBack)
   {
      NEXUS_Playback_Destroy(stream->playBack);
      stream->playBack = NULL;
   }

   if (stream->playPump)
   {
      NEXUS_Playpump_Close(stream->playPump);
      stream->playPump = NULL;
   }
}

/* Create a native pixmap (Nexus surface) with appropriate constraints for use as a texture */
static bool MakeNativePixmap(EGLNativePixmapType *retEglPixmap, NEXUS_SurfaceHandle *retNexusSurface,
                             uint32_t w, uint32_t h, bool secure)
{
   BEGL_PixmapInfoEXT   pixInfo;

   NXPL_GetDefaultPixmapInfoEXT(&pixInfo);

   pixInfo.width = w;
   pixInfo.height = h;

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
   pixInfo.format = BEGL_BufferFormat_eR8G8B8A8;
#else
   pixInfo.format = BEGL_BufferFormat_eA8B8G8R8;
#endif

   pixInfo.secure = secure;

   return NXPL_CreateCompatiblePixmapEXT(nxpl_handle, retEglPixmap, retNexusSurface, &pixInfo);
}

static void InitGLState(void)
{
   /* The shaders */
   const char vShaderStr[] =
      "uniform mat4   u_mvpMatrix;               \n"
      "attribute vec4 a_position;                \n"
      "attribute vec4 a_texcoord;                \n"
      "varying vec4   v_texcoord;                \n"
      "                                          \n"
      "void main()                               \n"
      "{                                         \n"
      "  gl_Position = u_mvpMatrix * a_position; \n"
      "  v_texcoord = a_texcoord;                \n"
      "}                                         \n";

   const char fShaderStr[] =
      "precision mediump float;                     \n"
      "uniform sampler2D u_textureUnit;             \n"
      "uniform mat4      u_colorMat;                \n"
      "varying vec4      v_texcoord;                \n"
      "                                             \n"
      "void main()                                  \n"
      "{                                            \n"
      "  vec4 color = vec4(texture2D(u_textureUnit, \n"
      "                     v_texcoord.st).rgb,1.0);\n"
      "  gl_FragColor = u_colorMat * color;         \n"
      "}                                            \n";

   GLuint     v, f;
   GLint      ret;
   const char *ff;
   const char *vv;
   char       *p, *q, *r;
   EGLint     attr_list[] = { EGL_NONE };

   NEXUS_SurfaceHandle dstSurface = NULL;
   NEXUS_Graphics2DFillSettings fillSettings;
   NEXUS_Error rc;

   /* Reserve some texture objects */
   glGenTextures(NUM_TEX, esTextures);

   /* Make a new native pixmap which we will convert to an EGLImage, and use as a texture.
   * Changes to the native pixmap will automatically appear in the texture without needing
   * to resubmit the texture */
   for (unsigned int pixmap_index = 0; pixmap_index < NUM_TEX; pixmap_index++)
   {
      if (!MakeNativePixmap(&eglPixmaps[pixmap_index], &nativePixmaps[pixmap_index],
                            videoStream.outputWidth, videoStream.outputHeight,
                            config.secure))
      {
         fprintf(stderr, "Failed to create native pixmap\n");
         exit(0);
      }

      /* Clear the pixmap with a Fill */
      dstSurface = nativePixmaps[pixmap_index];
      NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
      fillSettings.surface = dstSurface;
      fillSettings.rect.width = videoStream.outputWidth;
      fillSettings.rect.height = videoStream.outputHeight;
      fillSettings.color = 0;
      rc = NEXUS_Graphics2D_Fill(gfx2d, &fillSettings);
      BDBG_ASSERT(!rc);
      /* We must wait for the fill to complete now */
      do
      {
         rc = NEXUS_Graphics2D_Checkpoint(gfx2d, NULL);
         if (rc == NEXUS_GRAPHICS2D_QUEUED)
         {
            rc = BKNI_WaitForEvent(blitTextureDone, 1000);
         }
      }
      while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);


      /* Wrap the native pixmap (actually a NEXUS_Surface) as an EGLImage */
      eglImages[pixmap_index] = s_eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR,
                                     (EGLClientBuffer)eglPixmaps[pixmap_index], attr_list);
      if (eglImages[pixmap_index] == EGL_NO_IMAGE_KHR)
      {
         fprintf(stderr, "Failed to create EGLImage\n");
         exit(0);
      }

      /* Bind the EGL image as a texture, and set filtering (don't set to use mipmaps) */
      glBindTexture(GL_TEXTURE_2D, esTextures[pixmap_index]);
      s_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImages[pixmap_index]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   }

#if EGL_BRCM_image_update_control
   if (s_eglImageUpdateParameteriBRCM)
   {
      /* Tell GL we will be using explicit EGL image updates */
      s_eglImageUpdateParameteriBRCM(eglGetCurrentDisplay(), eglImages[current_tex],
                                    EGL_IMAGE_UPDATE_CONTROL_SET_MODE_BRCM,
                                    EGL_IMAGE_UPDATE_CONTROL_EXPLICIT_BRCM);
   }
#endif

   glClearDepthf(1.0f);
   glClearColor(0.2f, 0.2f, 0.2f, 0.0);  /* Gray background */

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
      printf("Shader compile error:\n%s\n", p);
      exit(0);
   }

   glCompileShader(f);
   glGetShaderiv(f, GL_COMPILE_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetShaderiv(f, GL_INFO_LOG_LENGTH, &ret);
      q = (char *)alloca(ret);
      glGetShaderInfoLog(f, ret, NULL, q);
      printf("Shader compile error:\n%s\n", q);
      exit(0);
   }

   program_object = glCreateProgram();
   glAttachShader(program_object, v);
   glAttachShader(program_object, f);

   /* Link the program */
   glLinkProgram(program_object);

   glGetProgramiv(program_object, GL_LINK_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetProgramiv(program_object, GL_INFO_LOG_LENGTH, &ret);
      if (ret > 0)
      {
         r = (char *)alloca(ret);
         glGetProgramInfoLog(program_object, ret, NULL, r);
         printf("Shader link error:\n%s\n", r);
         exit(0);
      }
      printf("Shader link error:\n");
      exit(0);
   }

   /* Get the attribute locations */
   position_loc = glGetAttribLocation(program_object, "a_position");
   tc_loc       = glGetAttribLocation(program_object, "a_texcoord");

   /* Get the uniform locations */
   col_matrix_loc = glGetUniformLocation(program_object, "u_colorMat");
   mvp_matrix_loc = glGetUniformLocation(program_object, "u_mvpMatrix");
   tex_unit_loc   = glGetAttribLocation(program_object,  "u_textureUnit");

   /* Adjust for video aspect ratio */
   esMatrixLoadIdentity(&projection_matrix);
   esMatrixLoadIdentity(&modelview_matrix);
   esMatrixLoadIdentity(&model_matrix);
   esMatrixLoadIdentity(&view_matrix);
   esMatrixLoadIdentity(&mvp_matrix);

   /* We will anisotropically scale the cube to best fit the video. */
   esScale(&model_matrix, 100, (uint32_t)(100.0f / videoStream.outputAspect),
                               (uint32_t)(100.0f / videoStream.outputAspect));

   esMatrixLoadIdentity(&view_matrix);
   esTranslate(&view_matrix, 0, 0, -320);
}

static void TerminateGLState(void)
{
   glDeleteProgram(program_object);
   glDeleteBuffers(2, vbo);
   glDeleteTextures(NUM_TEX, esTextures);

   for (unsigned int tex_index = 0; tex_index < NUM_TEX; tex_index++)
   {
      s_eglDestroyImageKHR(eglGetCurrentDisplay(), eglImages[tex_index]);
      NXPL_DestroyCompatiblePixmap(nxpl_handle, eglPixmaps[tex_index]);
   }
}

/* Map the function pointers for the GL and EGL extensions we will be using (if they exist) */
static void InitGLExtensions(void)
{
   s_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
   s_eglCreateImageKHR            = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
   s_eglDestroyImageKHR           = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
#if EGL_BRCM_image_update_control
   s_eglImageUpdateParameterivBRCM = (PFNEGLIMAGEUPDATEPARAMETERIVBRCMPROC)eglGetProcAddress("eglImageUpdateParameterivBRCM");
   s_eglImageUpdateParameteriBRCM  = (PFNEGLIMAGEUPDATEPARAMETERIBRCMPROC)eglGetProcAddress("eglImageUpdateParameteriBRCM");
#endif

   if (!s_glEGLImageTargetTexture2DOES || !s_eglCreateImageKHR || !s_eglDestroyImageKHR)
   {
      printf("Error: EGLImage texturing is not supported. Cannot continue.\n");
      exit(0);
   }
}

static void InitGLViewPort(unsigned int width, unsigned int height, float panelAspect, bool stretch)
{
   glViewport(0, 0, width, height);

   esMatrixLoadIdentity(&projection_matrix);

   if (stretch)
   {
      esPerspective(&projection_matrix, 45.0f, panelAspect, 100, 1000);
   }
   else
   {
      esPerspective(&projection_matrix, 45.0f, (float)width / (float)height, 100, 1000);
   }
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static void Resize(void)
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

/* Show the current frame-rate */
static void ShowFPS(void)
{
   struct timeval curTime;
   int            nowMs;
   static int     lastPrintTime  = 0;
   static int     lastPrintFrame = 0;
   static int     frame = 0;

   gettimeofday(&curTime, NULL);
   nowMs = curTime.tv_usec / 1000;
   nowMs += curTime.tv_sec * 1000;

   frame++;

   if (nowMs - lastPrintTime > 1000 || lastPrintFrame == 0)
   {
      if (nowMs - lastPrintTime != 0 && lastPrintTime != 0)
      {
         float fps = (float)(frame - lastPrintFrame) / ((float)(nowMs - lastPrintTime) / 1000.0f);
         printf("Frames per second: %f\n", fps);
      }

      lastPrintFrame = frame;
      lastPrintTime  = nowMs;
   }
}

/* Called once per frame */
static void Display(void)
{
   uint32_t v;

   if (config.showFPS)
   {
      ShowFPS();
   }

   /* Handle any resizing that may have occurred */
   Resize();

   /* Update the video frame textures (will leave the textures untouched if no new frame is ready) */
   UpdateVideoTexture();

   /* Rotate the cube */
   esRotate(&view_matrix, 0.5f,  1, 0, 0);
   esRotate(&view_matrix, 0.25f, 0, 1, 0);

   esMatrixMultiply(&modelview_matrix, &model_matrix, &view_matrix);

   /* Compute the final MVP by multiplying the model-view and perspective matrices together */
   esMatrixMultiply(&mvp_matrix, &modelview_matrix, &projection_matrix);

   /* Clear all the buffers we asked for during config to ensure fast-path */
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(program_object);

   /* Enable cube array */
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
   glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), BUFFER_OFFSET(0));
   glVertexAttribPointer(tc_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
   glEnableVertexAttribArray(position_loc);
   glEnableVertexAttribArray(tc_loc);

   /* Load the MVP matrix */
   glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)&mvp_matrix.m[0][0]);
   glUniform1i(tex_unit_loc, 0);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

   /* Finally draw the elements */
   /* We draw the cube one face at a time, to add different effects in the shader */
   for (v=0; v < NUM_CUBE_FACES; v++)
   {
      glUniformMatrix4fv(col_matrix_loc, 1, GL_FALSE, (GLfloat*)&color_matrices[v].m[0][0]);
      /* Draw the first video on all remaining faces with a single draw call */
      glBindTexture(GL_TEXTURE_2D, esTextures[current_tex]);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(v * 6 * sizeof(short)));
   }

#if !EGL_BRCM_image_update_control
   if (m_fences[current_tex] != EGL_NO_SYNC_KHR)
      eglDestroySyncKHR(egl_display, m_fences[current_tex]);

   m_fences[current_tex]  = eglCreateSyncKHR(egl_display, EGL_SYNC_FENCE_KHR, NULL);
#endif

   /* Post the framebuffer for display */
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

static bool InitEGL(NativeWindowType egl_win, const AppConfig *config)
{
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

      free(attr);
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

   /* Set the swap interval. 1 is almost always the right value. 0 is for benchmarking only, as tearing
    * will be introduced. Values >1 result in slower performance. */
   eglSwapInterval(egl_display, config->swapInterval);

   return true;
}

static bool InitDisplay(float *aspect, const AppConfig *config)
{
   NXPL_NativeWindowInfoEXT   win_info;

   eInitResult res = InitPlatformAndDefaultDisplay(&nexus_display, aspect, config->vpW, config->vpH, config->secure);
   if (res != eInitSuccess)
      return false;

   /* Register the Nexus display with the platform layer. The platform layer then controls the display. */
   NXPL_RegisterNexusDisplayPlatform(&nxpl_handle, nexus_display);

   NXPL_GetDefaultNativeWindowInfoEXT(&win_info);

   win_info.x        = config->vpX;
   win_info.y        = config->vpY;
   win_info.width    = config->vpW;
   win_info.height   = config->vpH;
   win_info.stretch  = config->stretchToFit;
   win_info.clientID = config->clientId;

   native_window = NXPL_CreateNativeWindowEXT(&win_info);

   /* Initialise EGL now we have a 'window' */
   if (!InitEGL(native_window, config))
      return false;

   return true;
}

/* Event callback that will be called when a 2D operation is complete */
static void complete(void *data, int unused)
{
   BSTD_UNUSED(unused);
   BKNI_SetEvent((BKNI_EventHandle)data);
}

static void InitVideo(bool secure)
{
   MediaData                  mediaData;
   NEXUS_Graphics2DSettings   gfxSettings;
   NEXUS_Graphics2DOpenSettings graphics2dOpenSettings;

   /* We'll need an event that will be triggered when the blit/fill is complete */
   BKNI_CreateEvent(&blitTextureDone);

   NEXUS_Graphics2D_GetDefaultOpenSettings(&graphics2dOpenSettings);
   graphics2dOpenSettings.secure = secure;
   /* Prepare the graphics2D module */
   gfx2d = NEXUS_Graphics2D_Open(0, &graphics2dOpenSettings);
   if (gfx2d == NULL)
   {
      printf("NEXUS_Graphics2D_Open failed\n");
      exit(0);
   }

   /* Tell it about our 'done' callback */
   NEXUS_Graphics2D_GetSettings(gfx2d, &gfxSettings);
   gfxSettings.checkpointCallback.callback = complete;
   gfxSettings.checkpointCallback.context = blitTextureDone;
   NEXUS_Graphics2D_SetSettings(gfx2d, &gfxSettings);

   InitMediaData(&mediaData);
   GetStreamData(config.videoFile, &mediaData);

   if (config.benchmark)
   {
      /* Check the video looks right for benchmarking */
      if (mediaData.width != 1920 || mediaData.height != 1080 || mediaData.duration != 50589 ||
          mediaData.videoPid != 5922 || mediaData.videoCodec != 2)
      {
         printf("You are not using the correct video file for benchmarking. Please use shark.mpg\n");
         exit(0);
      }
   }

   ConfigureVideoStream(&videoStream, &mediaData, nexus_display);
}

static void TermVideo(void)
{
   CleanupVideoStream(&videoStream);

   if (gfx2d)
   {
      NEXUS_Graphics2D_Close(gfx2d);
      gfx2d = NULL;
   }

   if (blitTextureDone)
   {
      BKNI_DestroyEvent(blitTextureDone);
      blitTextureDone = NULL;
   }
}

/* This tells the 3D driver that the texture is currently being written, so to avoid
   updating it until it's unlocked */
static void LockTexture(void)
{
#if EGL_BRCM_image_update_control
   if (s_eglImageUpdateParameteriBRCM)
   {
      s_eglImageUpdateParameteriBRCM(eglGetCurrentDisplay(), eglImages[current_tex],
      EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM,
      EGL_IMAGE_UPDATE_CONTROL_LOCK_BRCM);
   }
#endif
}

/* This tells the 3D driver that the texture is currently not being written, so it can
   update it */
static void UnlockTexture(void)
{
#if EGL_BRCM_image_update_control
   if (s_eglImageUpdateParameteriBRCM)
   {
      s_eglImageUpdateParameteriBRCM(eglGetCurrentDisplay(), eglImages[current_tex],
      EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM,
      EGL_IMAGE_UPDATE_CONTROL_UNLOCK_BRCM);
   }
#endif
}

/* This tells the 3D driver that a portion (or all) of the texture has been modified */
static void SetUpdatedTextureRegion(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
#if EGL_BRCM_image_update_control
   if (s_eglImageUpdateParameterivBRCM)
   {
      EGLint rect[4];
      rect[0] = x;
      rect[1] = y;
      rect[2] = w;
      rect[3] = h;
      s_eglImageUpdateParameterivBRCM(eglGetCurrentDisplay(), eglImages[current_tex],
                                      EGL_IMAGE_UPDATE_CONTROL_CHANGED_REGION_BRCM, rect);
   }
#else
   BSTD_UNUSED(x);
   BSTD_UNUSED(y);
   BSTD_UNUSED(w);
   BSTD_UNUSED(h);
#endif
}

/* Put any new video frames into the video textures */
static void UpdateVideoTexture(void)
{
   NEXUS_SurfaceHandle                    captureSurface;
   NEXUS_SimpleVideoDecoderCaptureStatus  captureStatus;
   NEXUS_Graphics2DBlitSettings           blitSettings;
   unsigned                               numReturned;
   NEXUS_Error                            rc;

   /* Try to capture a frame from the simple decoder*/
   NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(videoStream.videoDecoder, &captureSurface,&captureStatus, 1, &numReturned);

   /* If not frame available*/
   if (numReturned == 0)
   {
      BKNI_Sleep(10);
   }
   else
   {
      /*Lock the destination buffer for writing. This might take some time if the 3D core is using it right now.*/
      LockTexture();

#if !EGL_BRCM_image_update_control
      current_tex++;

      if (current_tex >= NUM_TEX)
         current_tex = 0;

      if (m_fences[current_tex] != EGL_NO_SYNC_KHR)
            eglClientWaitSyncKHR(egl_display, m_fences[current_tex], EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, EGL_FOREVER_KHR);
#endif

      NEXUS_SurfaceHandle dstSurface = nativePixmaps[current_tex];

      /*Blit the captured frame onto a texture*/
      NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
      blitSettings.source.surface = captureSurface;
      blitSettings.output.surface = dstSurface;
      rc = NEXUS_Graphics2D_Blit(gfx2d, &blitSettings);

      /* Wait for the blitter to finish*/
      do
      {
        rc = NEXUS_Graphics2D_Checkpoint(gfx2d, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED)
          rc = BKNI_WaitForEvent(blitTextureDone, 1000);
      }
      while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

      /* Release the capture surface for further capture */
      NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(videoStream.videoDecoder, &captureSurface, numReturned);

     /* Tell V3D we've changed it - all of it */
     SetUpdatedTextureRegion(0, 0, videoStream.outputWidth, videoStream.outputHeight);

     /*Unlock the texture so V3D can use it.*/
     UnlockTexture();
   }
}

static bool ProcessArgs(int argc, const char *argv[], AppConfig *config)
{
   int   a;

   if (config == NULL)
      return false;

   config->showFPS              = false;
   config->useMultisample       = false;
   config->stretchToFit         = false;
   config->benchmark            = false;
   config->vpX                  = 0;
   config->vpY                  = 0;
   config->vpW                  = WIDTH;
   config->vpH                  = HEIGHT;
   config->bpp                  = BPP;
   config->frames               = FRAMES;
   config->clientId             = 0;
   config->swapInterval         = 1;
   config->showbackgroundvideo  = false;
   config->secure               = false;

   for (a = 1; a < argc; ++a)
   {
      const char  *arg = argv[a];

      if (strcmp(arg, "+m") == 0)
         config->useMultisample = true;
      else if (strcmp(arg, "+fps") == 0)
         config->showFPS = true;
      else if (strcmp(arg, "+bench") == 0)
         config->benchmark = true;
      else if (strcmp(arg, "+s") == 0)
         config->stretchToFit = true;
      else if (strcmp(arg, "+backgroundvideo") == 0)
         config->showbackgroundvideo = true;
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
      else if (strncmp(arg, "video=", 6) == 0)
      {
         if (sscanf(arg, "video=%s", config->videoFile) != 1)
            return false;

         config->numVideos++;
      }
      else if (strncmp(arg, "swap=", 5) == 0)
      {
         if (sscanf(arg, "swap=%d", &config->swapInterval) != 1)
            return false;
      }
      else if (strncmp(arg, "t=", 2) == 0)
      {
         if (sscanf(arg, "t=%dx%d", &config->texW, &config->texH) != 2)
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

void Usage(const char *progname)
{
   fprintf(stderr, "Usage: %s video=<filename> [+backgroundvideo] [+m] [+p] [+s] [+fps] [+bench] [d=WxH] [o=XxY] [swap=N] [bpp=16/24/32] [f=frames] [t=WxH] [+secure]\n", progname);
}

int CLIENT_MAIN(int argc, const char** argv)
{
   int         frame = 1;
   const char  *progname = argc > 0 ? argv[0] : "";

   config.numVideos = 0;

   if (!ProcessArgs(argc, argv, &config) || config.numVideos == 0)
   {
      Usage(progname);
      return 0;
   }

#ifndef EGL_PROTECTED_CONTENT_EXT
   if (config.secure)
      printf("+secure selected, but headers not available in this driver version. defaulting off\n");
   config.secure = false;
#endif

   if (config.benchmark)
   {
      config.numVideos      = 1;
      config.vpW            = 1920;
      config.vpH            = 1080;
      config.useMultisample = true;
      config.bpp            = 32;
      config.stretchToFit   = true;
      config.swapInterval   = 0;
   }

   /* Setup the display and EGL */
   if (InitDisplay(&panelAspect, &config))
   {
      /* Setup the local state for this demo */
      InitGLExtensions();
      InitVideo(config.secure);

      InitGLState();
      InitGLViewPort(config.vpW, config.vpH, panelAspect, config.stretchToFit);

      printf("Rendering ");

      if (config.frames != 0)
      {
         printf("%d frames", config.frames);
      }

      printf(": press CTRL+C to terminate early\n");

      if (config.benchmark)
      {
         struct timeval curTime;
         int            start, end;

         printf("Priming...\n");

         /* Run for 500 frames  to get going */
         while (frame <= 500)
         {
            Display();
            frame++;
         }

         printf("Benchmarking...\n");
         gettimeofday(&curTime, NULL);
         start = curTime.tv_usec / 1000;
         start += curTime.tv_sec * 1000;

         /* Time the next 2000 frames */
         while (frame <= 2500)
         {
            Display();
            frame++;
         }

         gettimeofday(&curTime, NULL);
         end = curTime.tv_usec / 1000;
         end += curTime.tv_sec * 1000;

         printf("Benchmark average = %f fps\n", 2000.0f / (0.001f * (end - start)));
      }
      else
      {
         while (config.frames == 0 || frame <= config.frames)
         {
            Display();
            frame++;
         }
      }

      /* Close the local state for this demo */
      TerminateGLState();
   }

   /* Terminate EGL */
   egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglTerminate(egl_display);

   TermVideo();

   NXPL_DestroyNativeWindow(native_window);
   NXPL_UnregisterNexusDisplayPlatform(nxpl_handle);

   /* Close the platform */
   TermPlatform(nexus_display);

   return 0;
}
