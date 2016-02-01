############################################################
#     Copyright (c) 2007 Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#   Media framework module descriptor
#
# Revision History:
#
# $brcm_Log: $
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


BMEDIA_INCLUDES = ${BSEAV}/lib/media ${BATOM_INCLUDES} ${BAVI_INCLUDES} ${BASF_INCLUDES} ${BFLV_INCLUDES} ${BRMFF_INCLUDES} ${BSEAV}/lib/mpeg2_ts_parse ${BSEAV}/lib/tspsi
BMEDIA_DEFINES = ${BAVI_DEFINES} ${BASF_DEFINES} ${BFLV_DEFINES} ${BRMFF_DEFINES}

# Core module
BMEDIA_SOURCES = \
	${BSEAV}/lib/media/bmedia_util.c \
	${BAVI_SOURCES} ${BASF_SOURCES}

# MP4 player
B_MEDIA_FILTER_MP4_SOURCES :=  \
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
