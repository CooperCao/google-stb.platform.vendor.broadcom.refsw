/***************************************************************************
 *     (c)2003-2013 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 ***************************************************************************/
#ifndef BMEDIA_TYPES_H__
#define BMEDIA_TYPES_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*
Summary:
    MPEG2 Transport types
*/
typedef enum bstream_mpeg_type {
    bstream_mpeg_type_unknown,  /* unknown or not supported stream format */
    bstream_mpeg_type_es,       /* Elementary stream */
    bstream_mpeg_type_bes,      /* Broadcom elementary stream */
    bstream_mpeg_type_pes,      /* Packetized elementary stream */
    bstream_mpeg_type_ts,       /* Transport stream */
    bstream_mpeg_type_dss_es,
    bstream_mpeg_type_dss_pes,
    bstream_mpeg_type_vob,      /* video object, used with DVD */
    bstream_mpeg_type_asf,      /* Advanced Systems Format */
    bstream_mpeg_type_avi,      /* Audio Video Interleave */
    bstream_mpeg_type_mpeg1,    /* MPEG1 System Stream */
    bstream_mpeg_type_mp4,      /* MP4 (MPEG-4 Part12) container */
    bstream_mpeg_type_flv,      /* Flash video container */
    bstream_mpeg_type_mkv,      /* Matroska container */
    bstream_mpeg_type_wav,      /* WAVE audio container */
    bstream_mpeg_type_rmff,     /* RealMedia File Format container */
    bstream_mpeg_type_mp4_fragment, /* portions of MPEG4 Part12 fragmented stream */
    bstream_mpeg_type_ogg,      /* The Ogg Encapsulation Format */
    bstream_mpeg_type_flac,     /* The FLAC Encapsulation Format */
    bstream_mpeg_type_ape,      /* Monkey's Audio */
    bstream_mpeg_type_aiff,     /* Audio Interchange File Format */
    bstream_mpeg_type_amr,      /* AMR audio ES format - RFC4867 */
    bstream_mpeg_type_cdxa      /* CDXA RIFF file */
} bstream_mpeg_type;

/*
Summary:
    Digital audio formats
Description:
    Values 0, 3 and 4 are reservered for baudio_format_mpeg, even though baudio_format_mpeg == 0x3.
        3 is the value for MPEG1 audio (ISO/IEC 11172-3).
        4 is the value for MPEG2 audio (ISO/IEC 13818-3).
*/
typedef enum baudio_format {
   baudio_format_unknown = 0,           /* unknown/not supported audio format */
   baudio_format_mpeg = 0x3,        /* MPEG1/2, layer 1/2. This does not support layer 3 (mp3). */
   baudio_format_mp3 = 0x1,         /* MPEG1/2, layer 3. */
   baudio_format_aac = 0xF,         /* Advanced audio coding. Part of MPEG-4 */
   baudio_format_aac_plus = 0x11,   /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE) */
   baudio_format_aac_plus_adts = 0x12,  /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with ADTS (Audio Data Transport Format) */
   baudio_format_aac_plus_loas = 0x11,  /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with LOAS (Low Overhead Audio Stream) */
   baudio_format_ac3 = 0x81,        /* Dolby Digital AC3 audio */
   baudio_format_ac3_plus = 0x6,    /* Dolby Digital Plus (AC3+ or DDP) audio */
   baudio_format_avs = 0x43,        /* AVS Audio */
   baudio_format_dra = 0xda,
   baudio_format_cook = 0xf0,
   baudio_format_dts = 0x82,        /* Digital Digital Surround sound. */
   baudio_format_dts_hd,            /* Digital Digital Surround sound, HD */
   baudio_format_dts_cd,
   baudio_format_dts_lbr,           /*  DTS-Express (DTS-LBR).  Low bit rate DTS format used in BluRay and streaming applications. */
   baudio_format_lpcm_hddvd,        /* LPCM, HD-DVD mode */
   baudio_format_lpcm_bluray,       /* LPCM, Blu-Ray mode */
   baudio_format_wma_std,           /* WMA Standard */  
   baudio_format_wma_pro,           /* WMA Professional */  
   baudio_format_pcm,
   baudio_format_lpcm_dvd,          /* LPCM, DVD mode */
   baudio_format_lpcm_1394,         /* IEEE-1394 LPCM */
   baudio_format_adpcm,             /* MS ADPCM Format */
   baudio_format_g726,              /* G.726 ITU-T ADPCM  */
   baudio_format_dvi_adpcm,         /* DVI ADPCM Format */
   baudio_format_amr_nb,
   baudio_format_amr = baudio_format_amr_nb,
   baudio_format_amr_wb,
   baudio_format_vorbis,
   baudio_format_flac,
   baudio_format_ape,
   baudio_format_mlp,          /* Dolby True-HD */
   baudio_format_g711,
   baudio_format_ilbc,
   baudio_format_isac,
   baudio_format_opus,
   baudio_format_als          /* MPEG-4 Audio Lossless Coding */
} baudio_format;

/*
Summary:
    Codec type for digital video 
*/
typedef enum bvideo_codec {
    bvideo_codec_none = 0,              /* not coded video */ 
    bvideo_codec_unknown = 0,           /* unknown/not supported video codec */
    bvideo_codec_mpeg1 = 1,             /* MPEG-1 Video (ISO/IEC 11172-2) */
    bvideo_codec_mpeg2 = 2,             /* MPEG-2 Video (ISO/IEC 13818-2) */
    bvideo_codec_mpeg4_part2 = 0x10,    /* MPEG-4 Part 2 Video */
    bvideo_codec_h263 = 0x1A,           /* H.263 Video. The value of the enum is not based on PSI standards. */
    bvideo_codec_h264 = 0x1B,           /* H.264 (ITU-T) or ISO/IEC 14496-10/MPEG-4 AVC */
    bvideo_codec_h264_svc = 0x1F,       /* H.264 (ITU-T) or ISO/IEC 14496-10/MPEG-4 AVC Annex G*/
    bvideo_codec_h264_mvc = 0x20,       /* H.264 (ITU-T) or ISO/IEC 14496-10/MPEG-4 AVC Annex H*/
    bvideo_codec_h265,                  /* H.265 (ITU-T) or ISO/IEC 23008-2 MPEG-H Part 2 - HEVC video */
    bvideo_codec_vc1 = 0xEA,            /* VC-1 Advanced Profile */
    bvideo_codec_vc1_sm = 0xEB,         /* VC-1 Simple&Main Profile */
    bvideo_codec_divx_311 = 0x311,      /* DivX 3.11 coded video */
    bvideo_codec_avs = 0x42,            /* AVS video */
    bvideo_codec_spark = 0xF0,          /* H.263 Sorenson Spark */
    bvideo_codec_vp6,
    bvideo_codec_rv40,
    bvideo_codec_vp8,
    bvideo_codec_vp9,
    bvideo_codec_mjpeg
} bvideo_codec;

#ifdef __cplusplus
}
#endif

#endif /* BMEDIA_TYPES_H__*/

