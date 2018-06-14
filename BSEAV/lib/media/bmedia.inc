############################################################
# Copyright (C) 2017-2018 Broadcom.  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#
# Module Description:
#   Media framework module descriptor
#
############################################################

# Input variables:
#   BASF_XXX - defined in basf.inc
#   BAVI_XXX - defined in bavi.inc
#   BFLV_XXX - defined in bflv.inc
#   BRMFF_XXX - defined in brmff.inc
# Output variables:
#   BMEDIA_SOURCES, BMEDIA_INCLUDES, BMEDIA_CFLAGS - core interface
#   BMEDIA_PLAYER_SOURCES - player sources
#   BMEDIA_FILTER_SOURCES - filter (media -> PES) probe sources
#   BMEDIA_PROBE_SOURCES - probe sources
#   BMEDIA_DEPENDS - modules used by bmedia library

BMEDIA_DEPENDS := BATOM BFILE BFILE_CACHE BFILE_MEDIA

-include ${BSEAV}/lib/media/dbv/bmedia_dbv.inc

BMEDIA_INCLUDES = ${BSEAV}/lib/media ${BATOM_INCLUDES} ${BAVI_INCLUDES} ${BASF_INCLUDES} ${BFLV_INCLUDES} ${BRMFF_INCLUDES} ${BMEDIA_DBV_INCLUDES} ${BSEAV}/lib/mpeg2_ts_parse
BMEDIA_DEFINES = ${BAVI_DEFINES} ${BASF_DEFINES} ${BFLV_DEFINES} ${BRMFF_DEFINES} ${BMEDIA_DBV_DEFINES}

# Core module
BMEDIA_SOURCES = \
	${BSEAV}/lib/media/bmedia_util.c \
	${BAVI_SOURCES} ${BASF_SOURCES} ${BMEDIA_DBV_SOURCES}

# MP4 player
B_MEDIA_FILTER_MP4_SOURCES :=  \
	${BSEAV}/lib/media/bmp4_ce_util.c \
	${BSEAV}/lib/media/bmp4_fragment_demux.c \
	${BSEAV}/lib/media/bmp4_parser.c \
	${BSEAV}/lib/media/bmp4_util.c \
	${BSEAV}/lib/media/bmpeg4_util.c

# FILTER module
BMEDIA_FILTER_SOURCES = \
    ${BSEAV}/lib/media/baiff_filter.c \
    ${BSEAV}/lib/media/baiff_util.c \
    ${BSEAV}/lib/media/bamr_util.c \
    ${BSEAV}/lib/media/bamr_parser.c \
    ${BSEAV}/lib/media/bape_util.c \
    ${BSEAV}/lib/media/bflac_parser.c \
    ${BSEAV}/lib/media/bflac_util.c \
    ${BSEAV}/lib/media/bmedia_filter.c \
    ${BSEAV}/lib/media/bmedia_pes.c \
    ${BSEAV}/lib/media/bmedia_es.c \
    ${BSEAV}/lib/media/bmedia_mjpeg.c \
    ${BSEAV}/lib/media/bmpeg1_parser.c  \
    ${BSEAV}/lib/media/bogg_parser.c \
    ${BSEAV}/lib/media/bogg_util.c \
    ${BSEAV}/lib/media/briff_parser.c \
    ${BSEAV}/lib/media/bwav_filter.c \
    ${BAVI_FILTER_SOURCES} ${BASF_FILTER_SOURCES} ${BFLV_FILTER_SOURCES} ${BRMFF_FILTER_SOURCES} ${B_MEDIA_FILTER_MP4_SOURCES} \

BMEDIA_FILTER_INCLUDES = ${BSEAV}/lib/bfile

# Player modules
B_MEDIA_PLAYER_CORE_SOURCES := \
    ${BSEAV}/lib/media/bape_player.c \
    ${BSEAV}/lib/media/bid3_parser.c \
    ${BSEAV}/lib/media/bmedia_index.c \
    ${BSEAV}/lib/media/bmedia_player.c \
    ${BSEAV}/lib/media/bmedia_player_es.c \
    ${BSEAV}/lib/media/bmedia_player_generic.c \
    ${BSEAV}/lib/media/bmpeg_audio_util.c \
    ${BSEAV}/lib/media/bogg_player.c \
    ${BSEAV}/lib/media/bwav_player.c

# MP4 player
B_MEDIA_PLAYER_MP4_SOURCES :=  \
    ${BSEAV}/lib/media/bmp4_track.c \
    ${BSEAV}/lib/media/bmp4_player.c

# MKV player
B_MEDIA_PLAYER_MKV_SOURCES :=  \
    ${BSEAV}/lib/media/bmkv_parser.c  \
    ${BSEAV}/lib/media/bmkv_file_parser.c  \
    ${BSEAV}/lib/media/bmkv_player.c  \
    ${BSEAV}/lib/media/bmkv_util.c

#  MPEG-2 NAV Player (requires also ${BSEAV}/lib/bcmplayer/bcmplayer.inc )
B_MEDIA_NAV_PLAYER_SOURCES := ${BSEAV}/lib/media/bmedia_player_nav.c
B_MEDIA_NAV_PLAYER_DEFINES := B_HAS_NAV_PLAYER=1

# No index/timestamp based player
B_MEDIA_PLAYER_TIME_SOURCES := \
    ${BSEAV}/lib/media/bmpeg2ts_parser.c \
    ${BSEAV}/lib/media/bmpeg2ts_player.c \
    ${BSEAV}/lib/media/bmpeg2pes_parser.c \
    ${BSEAV}/lib/media/bmpeg2pes_player.c \
    ${BSEAV}/lib/media/btime_indexer.c \
    ${BSEAV}/lib/otfpvr/botf_marker.c
B_MEDIA_PLAYER_TIME_INCLUDES := ${BSEAV}/lib/otfpvr


BMEDIA_PLAYER_SOURCES = ${B_MEDIA_PLAYER_CORE_SOURCES} ${B_MEDIA_PLAYER_MP4_SOURCES} ${B_MEDIA_PLAYER_MKV_SOURCES} ${B_MEDIA_PLAYER_TIME_SOURCES} \
    ${BASF_PLAYER_SOURCES} ${BAVI_PLAYER_SOURCES} ${BRMFF_PLAYER_SOURCES}

BMEDIA_PLAYER_INCLUDES = ${BMEDIA_INCLUDES} ${B_MEDIA_PLAYER_TIME_INCLUDES}
BMEDIA_PLAYER_DEFINES = ${BMEDIA_DEFINES}


# Probe MPEG2 TS module
B_MEDIA_PROBE_MPEG2TS_SOURCES := \
    ${BSEAV}/lib/media/bmpeg2pes_parser.c \
    ${BSEAV}/lib/media/bmpeg2psi_parser.c \
    ${BSEAV}/lib/media/bmpeg2ts_probe.c \
    ${BSEAV}/lib/media/bmpeg2pes_probe.c \
    ${BSEAV}/lib/media/bmpeg2ts_psi_probe.c \
    ${BSEAV}/lib/mpeg2_ts_parse/ts_pmt.c \
    ${BSEAV}/lib/mpeg2_ts_parse/ts_pat.c \
    ${BSEAV}/lib/mpeg2_ts_parse/ts_priv.c \
    ${BSEAV}/lib/mpeg2_ts_parse/ts_psi.c \
    ${BSEAV}/lib/mpeg2_ts_parse/tspsi_validate.c

B_MEDIA_PROBE_ES_SOURCES := \
    ${BSEAV}/lib/media/baac_adts_probe.c \
    ${BSEAV}/lib/media/baac_loas_probe.c \
    ${BSEAV}/lib/media/bac3_probe.c \
    ${BSEAV}/lib/media/bavc_video_probe.c \
    ${BSEAV}/lib/media/bavs_video_probe.c \
    ${BSEAV}/lib/media/bdts_probe.c \
    ${BSEAV}/lib/media/bhevc_video_probe.c \
    ${BSEAV}/lib/media/bmedia_probe_es.c \
    ${BSEAV}/lib/media/bmp3_probe.c \
    ${BSEAV}/lib/media/bmpeg_audio_probe.c \
    ${BSEAV}/lib/media/bmpeg_video_probe.c

# Probe module
BMEDIA_PROBE_SOURCES = \
    ${BSEAV}/lib/media/baiff_probe.c \
    ${BSEAV}/lib/media/bamr_probe.c \
    ${BSEAV}/lib/media/bape_probe.c \
    ${BSEAV}/lib/media/bflac_probe.c \
    ${BSEAV}/lib/media/bmedia_probe.c \
    ${BSEAV}/lib/media/bmedia_probe_demux.c \
    ${BSEAV}/lib/media/bmedia_probe_util.c \
    ${BSEAV}/lib/media/bmedia_cdxa.c \
    ${BSEAV}/lib/media/bmedia_pcm.c \
    ${BSEAV}/lib/media/bmkv_probe.c \
    ${BSEAV}/lib/media/bmp4_probe.c \
    ${BSEAV}/lib/media/bmpeg1_probe.c \
    ${BSEAV}/lib/media/bogg_probe.c \
    ${BSEAV}/lib/media/bwav_probe.c \
    ${B_MEDIA_PROBE_ES_SOURCES} \
    ${B_MEDIA_PROBE_MPEG2TS_SOURCES} \
    ${BFLV_PROBE_SOURCES} \
    ${BAVI_PROBE_SOURCES} \
    ${BASF_PROBE_SOURCES} \
    ${BRMFF_PROBE_SOURCES}

BMEDIA_PROBE_INCLUDES = ${BMEDIA_INCLUDES}
BMEDIA_PROBE_DEFINES = ${BMEDIA_DEFINES}
