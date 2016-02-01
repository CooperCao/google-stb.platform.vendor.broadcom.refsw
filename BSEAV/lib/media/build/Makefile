############################################################################
#     Copyright (c) 2009-2013, Broadcom Corporation
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
#
# Revision History:
#
# $brcm_Log: $
# 
###########################################################################

include ../../../api/build/tools.mak
BSEAV = $(shell cd "../../.." && ${PWD})
MAGNUM = $(BSEAV)/../magnum
B_REFSW_OBJ_DIR ?= obj.${B_REFSW_ARCH}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}

-include ${BSEAV}/lib/asf/basf.inc
-include ${BSEAV}/lib/avi/bavi.inc
-include ${BSEAV}/lib/media/flv/bflv.inc
-include ${BSEAV}/lib/media/rmff/brmff.inc
include ${BSEAV}/lib/utils/batom.inc
include ${BSEAV}/lib/bfile/bfile.inc
include ${BSEAV}/lib/media/bmedia.inc
include ${BSEAV}/lib/bcmplayer/bcmplayer.inc

TEST_MODULES := BMEDIA BMEDIA_PLAYER BMEDIA_FILTER BMEDIA_PROBE BFILE_ASYNC B_NAV_PLAYER B_MEDIA_NAV_PLAYER
TEST_MODULES += BFILE_STDIO BASF_PARS
B_MODULES := $(sort ${TEST_MODULES} $(foreach module, $(TEST_MODULES), $($(module)_DEPENDS)))
B_TEST_INC := $(addprefix -I,$(sort $(foreach module, $(B_MODULES), $($(module)_INCLUDES))))
B_TEST_CFLAGS := $(addprefix -D,$(sort $(foreach module, $(B_MODULES), $($(module)_DEFINES))))
B_TEST_SOURCES = $(sort $(foreach module, $(B_MODULES), $($(module)_SOURCES)))
B_TEST_SRCS := $(notdir ${B_TEST_SOURCES})
vpath %c $(dir ${B_TEST_SOURCES})

# build media app
APP = media
# option to build libmedia.so library
#LIB = media
#CFLAGS_SHARED += ${B_REFSW_CFLAGS_SHARED}

#PROFILE_FLAGS = -pg
#CFLAGS += -DPROFILE=1
CFLAGS += -W -Wall ${PROFILE_FLAGS}
CFLAGS += -I ../../../api/include
CFLAGS += $(B_REFSW_CFLAGS) $(B_REFSW_GENERIC_MAGNUM_CFLAGS) $(B_REFSW_MAGNUM_INCLUDE_DIRS)
CFLAGS += ${B_REFSW_C_ONLY_FLAGS} ${B_REFSW_LINUXKERNEL_CFLAGS}
CFLAGS += ${B_TEST_CFLAGS} ${B_TEST_INC}
CFLAGS += -fstrict-aliasing -frename-registers
CFLAGS += -D_POSIX_C_SOURCE=199309L
CFLAGS += -Os
CFLAGS += -DBKNI_TRACK_MALLOCS=1
ifeq ($(B_REFSW_ARCH),i386-linux)
#CFLAGS += -m32
endif
#CFLAGS += -S
SRCS += ${B_TEST_SRCS}

ifneq ($(APP),)
SRCS += bmedia_test.c
endif
SRCS += bmedia_benchmark.c
SRCS += blst_aa_tree_test.c

CFLAGS += -I ../../utils
vpath %.c ../../utils ${MAGNUM}/commonutils/lst/test

include $(BSEAV)/build/refsw_inc.mak

ODIR = ${B_REFSW_OBJ_ROOT}/BSEAV/lib/media/build/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)

OS = ${B_REFSW_OS}

# Need some Magnum basemodules
include $(MAGNUM)/basemodules/kni/bkni.inc
include $(MAGNUM)/basemodules/dbg/bdbg.inc
include $(MAGNUM)/commonutils/lst/blst.inc
include $(MAGNUM)/commonutils/vlc/bvlc.inc
CFLAGS += $(addprefix -I, $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES)))
CFLAGS += $(addprefix -D, $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))
SRCS += $(notdir $(foreach module, $(MAGNUM_MODULES), $($(module)_SOURCES)))
vpath %.c $(dir $(foreach module, $(MAGNUM_MODULES), $($(module)_SOURCES)))

LDFLAGS += ${PROFILE_FLAGS}
ifeq ($(B_REFSW_ARCH),i386-linux)
#LDFLAGS += -m32
endif

ifeq (${PROBE_ONLY},y)
CFLAGS += -DB_PROBE_ONLY=1
endif
ifeq (${OS},linuxuser)
LDFLAGS += -lpthread -lrt
#LDFLAGS += -static
endif

OBJS += ${MAGNUM_OBJS}
OTHER_CLEANS = backup_clean

ifeq ($(BPROFILE_SUPPORT),y)
CFLAGS_BPROFILE= -falign-functions=4 -finstrument-functions -DB_HAS_BPROFILE=1
#for now compile symbol table only for BPROFILE
SYM_SRC = bsymbols.c
CFLAGS += -DB_HAS_SYMTABLE
vpath %.c  ${BSEAV}/lib/bprofile
CFLAGS += -I${BSEAV}/lib/bprofile -I${BSEAV}/lib/utils
SRCS += bsymtable.c
SRCS += bprofile.c bprofile_data.c btrc.c bperf_counter.c
endif


include ${BSEAV}/api/build/rules.mak

backup_clean:
	${RM} -rv core* *~ ../*~ ./*~ ../../bfile/*~ ../../utils/*~ gmon.out *.debug *.release

#STREAM_AVI = /public/streams/playback/divx/c14_longfight_divx51_4mbps_mp3_cbr320.avi
#STREAM_AVI = /public/streams/playback/divx/b03_divx1_smpte_720x480_divx51sp_4mbps.avi
#STREAM_AVI = /public/streams/playback/divx/f06_48khz_cbr320_Pulpdiner.avi
#STREAM_AVI = 352x288_tennis_vid.avi
#STREAM_AVI = /public/streams/playback/divx/a04_divx412_640x304_870kbps_mp3_44khz_cbr128.avi
#STREAM_AVI = /public/streams/playback/divx/s06_AC3_spy_clip4_merrygoround_divx505b_1500kbps.avi
#STREAM_AVI = /public/vsilyaev/streams/avi/googlevideo/legend_of_zelda.avi
#STREAM_AVI = /public/vsilyaev/streams/avi/googlevideo/humancomputation.avi
#STREAM_AVI = /public/streams/vsilyaev/divx/a04_divx412_640x304_870kbps_mp3_44khz_cbr128.avi
#STREAM_AVI = /public/streams/vsilyaev/avi/customers/Humax/ebs-1.avi
#STREAM_AVI = /public/streams/vsilyaev/avi/divx/c16_divx51_stretch720x576_4mbps_mp3_48khz_vbr192.avi
#STREAM_AVI = /public/streams/vsilyaev/avi/divx/a06_divx311fast_640x304_870kbps_mp3_44khz_vbr128.avi
#STREAM_AVI = /public/gmohile/streams/Divx5_320x240x16xYUV2.avi
#STREAM_AVI = /projects/stbdevrelease/streams/vsilyaev/avi/customers/Humax_HIPI/ProblemNo5_1.avi
STREAM_AVI = /projects/stbdevrelease/streams/vsilyaev/avi/divx_hd/C25_HD_1080p10_mp3_surround_224cbr_48_01.divx


STREAM_MPEG1 = /public/vsilyaev/streams/mpeg1/anycall.mpg
#STREAM_AVI = /public/streams/playback/divx/s01_smpte_bframes_720x480_divx505_2mbps.avi
#STREAM_AVI = /public/gmohile/streams/f06_48khz_cbr320_Pulpdiner.avi
#STREAM_MP4 = /public/vsilyaev/streams/mp4/Tank.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/bbc-africa_m480p.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/long_1080p.mp4
#STREAM_MP4 = a7-tone-oddities.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/yrajakar/abc-Nokia.mp4
STREAM_MP4 = /public/vsilyaev/streams/mp4/yrajakar/d01012007.mp4

#STREAM_MP4 = /public/vsilyaev/streams/mp4/bbc-japan_480p.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/bbc-africa_m480p.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/bbc-japan_720p.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/hercules.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/rataouille-clip_h480p.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/al00_08.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/legend_of_zelda.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/sample_h264_1mbit.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/sample_300kbit.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/sample_100kbit.mp4
#STREAM_MP4 = /public/gmohile/streams/10min720vcd80k.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/JessicaAlba.mp4
#STREAM_MP4 = /public/vsilyaev/streams/mp4/wwan/FighterPilot_180x320_750kbps.mp4

#STREAM_FLV = /public/vsilyaev/streams/flv/fluid.flv
#STREAM_FLV = /public/vsilyaev/streams/flv/outfoxed.flv
STREAM_FLV = /public/vsilyaev/streams/flv/tanks.flv

#STREAM_ASF = /public/vsilyaev/streams/vc1/LewisClark_HiDef_ap.wmv
#STREAM_ASF = /public/vsilyaev/streams/vc1/wm9_1080p24.wmv
#STREAM_ASF = /public/vsilyaev/streams/vc1/sm/Dust_to_Glory_720_aud.wmv
#STREAM_ASF = /public/vsilyaev/streams/vc1/sm/PT-109_HiDef.wmv
#STREAM_ASF = /public/vsilyaev/streams/vc1/ma20081.wmv
#STREAM_ASF = /public/vsilyaev/streams/vc1/Ranga.wma
#STREAM_ASF = The_Other_Side_Of_The_Bed_decrypted_chunk_1.wmv
#STREAM_ASF = /public/vsilyaev/streams/vc1/MP10_Stiker_adv_aud.wmv
#STREAM_ASF = /public/vsilyaev/streams/asf/test2_WMA_40_v2_64kbps_32kHz_2_zeros.asf
STREAM_ASF = /public/vsilyaev/streams/asf/customers/Humax_HIPI/ProblemNo6_2.wmv

#STREAM_MKV = /public/vsilyaev/streams/mkv/azumi.mkv
#STREAM_MKV = /public/vsilyaev/streams/mkv/soraya.mkv
STREAM_MKV = /public/vsilyaev/streams/mkv/test/x264_no-b-frames.mkv
#STREAM_MKV = /public/vsilyaev/streams/mkv/legend_of_zelda_avi.mkv
#STREAM_MKV = /public/vsilyaev/streams/mkv/legend_of_zelda_mp4.mkv
#STREAM_MKV = /public/vsilyaev/streams/mkv/Matrix.Reloaded.Trailer-640x346-XviD-1.0beta2-HE_AAC_subtitled.mkv
#STREAM_MKV = /public/vsilyaev/streams/mkv/mewmew-vorbis-ssa.mkv
#STREAM_MKV = /public/vsilyaev/streams/mkv/customer/LG/Lee.San.E35.720p.HDTV.x264-iamKG.mkv
#STREAM_MKV = /public/vsilyaev/streams/mkv/test/virgle.mkv

#STREAM = /public/vsilyaev/streams/vc1/MP10_Stiker_adv_aud.wmv

#STREAM = pbs_bugs_q64_0x00.mpg
#STREAM = /public/streams/streamer/pbs_bugs_q64.mpg
#STREAM = /public/streams/vsilyaev/pbs_bugs_q64.ts
#STREAM = /public/streams/streamer/bskyb/SDPatternTest.trp
#STREAM = /public/vsilyaev/streams/cnnticker.mpg
#STREAM = /public/vsilyaev/streams/timestamp.mpg
#STREAM = /public/streams/playback/audio/aac/WATERS02_44k_11_14.pvr.ts
#STREAM = /public/streams/playback/brutus21.mpg
#STREAM = /public/streams/playback/es_still/Photo1_5.mpg
#STREAM = /public/streams/playback/es_still/lquant.mpg
#STREAM = /public/vsilyaev/streams/mp4/bbc-africa_m480p.mp4

#STREAM = /public/vsilyaev/streams/divx/720x480.divx
#STREAM = /public/streams/playback/audio/ac3/dtrtall.ac3
#STREAM = /public/streams/vsilyaev/avi/DivXTestCDv2.0/G_AUDIO_CODECS/AUD-5A.ac3
#STREAM = /public/streams/vsilyaev/avi/DivXTestCD/Musik/Test.ac3
#STREAM = /public/streams/playback/audio/mpeg/mpeg1_L1.mpg
#STREAM = /public/streams/playback/audio/mpeg/mpeg1_L2.mp2
#STREAM = /public/streams/vsilyaev/avi/DivXTestCD/Musik/Test-CBR-128.mp3
#STREAM = /public/streams/vsilyaev/avi/DivXTestCD/Musik/Test-CBR-320.mp3
#STREAM = /public/streams/vsilyaev/avi/DivXTestCD/Musik/Test-VBR.mp3
#STREAM = /public/streams/orgrimmar.mp3
#STREAM = /public/vsilyaev/streams/old/encoder/2_4_9/fox_tape.ves
#STREAM = /public/vsilyaev/streams/mpeg1/SugarTooth.mpg
#STREAM = /public/streams/vsilyaev/mpeg1/safe_area_test_ntsc.mpg
#STREAM = /public/vsilyaev/streams/misc/probe/stream.mpg
#STREAM = /public/streams/playback/mp4_demux/BTShort_HD_apl_192k_audio.aac
#STREAM = /public/vsilyaev/streams/audio/aac.aac
#STREAM = /public/vsilyaev/streams/audio/aac1.aac
#STREAM = /public/streams/playback/audio/aac/al00_16_11_14.pvr.ts
#STREAM = /public/streams/playback/audio/aac/1_1_44k_11_14.pvr.ts
#STREAM = /public/streams/playback/audio/aac/bond_aac.mpg
#STREAM = /public/streams/playback/es/cnnticker.audio.pes
#STREAM = /public/streams/playback/es/cnnticker.video.pes
#STREAM = /public/streams/playback/divx/a01_divx51b_640x304_870kbps_mp3_44khz_cbr128.pes
#STREAM = /public/streams/playback/vob/gb_chap17_3.vob
#STREAM = /public/streams/playback/vob/gilmour0_pcm24.vob
#STREAM = /public/streams/playback/vob/depeche_pcm16.vob
#STREAM = /public/streams/vsilyaev/divx/06_48khz_cbr320_Pulpdiner.pes
#STREAM = /public/vsilyaev/streams/old/656/VBI/7114-1/VBI71131.TS
#STREAM = /public/streams/playback/vc1/MS_VC1_Compliance/WMVVC1ArtTestVectors/Artificial_test_Vectors_VC-1_RTM/vc1_007_MP_Prog_F1D0R0B0M1X0U0V0Q0S0P0L0T0C1G0f1b0d0r-1l-1x0v0_0_320x240.wmv
#STREAM = /public/vsilyaev/streams/vc1/test/lewis_clark_1xrew.pes
#STREAM = /public/streams/playback/pixar_lamp.vob
#STREAM = /public/streams/playback/customer/samsung/MOVIE/MPEG1_PS/sara.mpeg
#STREAM_TS = /public/streams/vsilyaev/pbs_bugs_q64.ts
STREAM_TS = /public/vsilyaev/streams/temp/pbs_bugs_q64_3.ts
#STREAM_TS = pbs_cut.mpg
#STREAM_TS = pbs_cut3.mpg
#STREAM_TS = /public/streams/vsilyaev/pbs_bugs_q64.mpg



#ARG = -msg bavi_stream -type AVI -video 1 -audio 0 -video_id 0 ${STREAM_AVI} avi.pes
#ARG = -msg bavi_stream -type AVI -video 1 -audio 0 ${STREAM_AVI} avi.pes
#ARG = -msg _bavi_parser -msg bmedia_test -msg bavi_player -player -type AVI -video 1 -video_id 0 ${STREAM_AVI} avi.es
#ARG = -msg bavi_parser -msg bmedia_test -msg _bavi_player -player -type AVI -video 1 -direction 1 ${STREAM_AVI} avi.pes
#ARG = -msg bavi_parser -msg bmedia_test -msg _bavi_player -player -type AVI -video 1 -direction 10 ${STREAM_AVI} avi.pes
#ARG = -msg bavi_parser -msg bfile_cache -msg bmedia_test -msg _bavi_player -player -type AVI -video 1 -direction 10 ${STREAM_AVI} avi.pes
#ARG = -msg bavi_parser -msg bfile_cache -msg bmedia_test -msg bmedia_index -msg bavi_util -msg bavi_player -player -type AVI -step -video 1 -audio 2 -seek 17000000 -direction -10 ${STREAM_AVI} avi.pes
#ARG = -msg _bavi_parser -msg bfile_cache -msg bmedia_test -msg bmedia_index -msg bavi_util -msg bavi_player -player -type AVI -step -video 1 -audio 2 -seek 124872 -direction -1000 ${STREAM_AVI} avi.pes
#ARG = -msg bavi_parser -msg bfile_cache -msg bmedia_test -msg bmedia_index -msg bavi_util -msg bavi_player -player -type AVI -step -video 1 -audio 2 -seek 12872 -direction -1000 ${STREAM_AVI} avi.pes
#ARG = -type MPEG1 -video 0xE0 -audio 0xC0 ${STREAM_MPEG1} mpeg1.pes
#ARG = -msg bmp4_parser -msg bmp4_util -step -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -step -video 1 -audio 20 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -step -video 1 -audio 2 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -step -video 1 -audio 2 -type MP4 -seek 2000 ${STREAM_MP4} mp4.pes
#ARG = -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -step -video 1 -audio 2 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -video 1 -audio 2 -step -seek 2000 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -video 1 -audio 2 -step -seek 8000 -seek2 2000 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -step -video 2 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -step -direction 100 -video 1 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bfile_buffer -msg _bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -step -direction -100 -seek 90000 -video 1 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bfile_buffer -msg _bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -step -seek 90000 -video 1 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bfile_buffer -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -step -seek 90000 -video 2  -audio 3 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bmedia_test -msg bflv_parser -msg bmedia_filter -video 9  -audio 8 -step -type FLV ${STREAM_FLV} flv.pes
#ARG = -msg bmedia_test -msg bflv_parser -msg bmedia_filter -audio 8 -audio_id 0 -step -type FLV ${STREAM_FLV} flv.mp3
#ARG = -msg _bmedia_;pes -msg _bmedia_test -msg _bflv_parser -msg _bmedia_filter -audio 8 -type FLV ${STREAM_FLV} flv.pes
#ARG = -msg bfile_buffer -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -step -video 2  -audio 3 -seek 90000 -direction -100 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bfile_buffer -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -step -video 2  -audio 3 -seek 90000 -direction -100 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg bfile_buffer -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -seek 100 -step -audio 1 -video 2 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg _bfile_buffer -msg _bmp4_track -msg _bmedia_index -msg _barena -msg _bmedia_test -msg _bfile_cache -msg _bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -async -audio 1 -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg _bfile_buffer -msg _bmp4_track -msg _bmedia_index -msg _barena -msg _bmedia_test -msg _bfile_cache -msg _bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -audio 1 -video 2 -type MP4 -async ${STREAM_MP4} mp4.pes
#ARG = -msg _bfile_buffer -msg _bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg _bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -audio 1 -video 2 -async -step -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg _bfile_buffer -msg bmp4_track -msg _bmedia_index -msg _barena -msg _bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -video 2 -async -step -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg _basf_parser -msg basf_stream -msg _bfile_buffer -msg _bmp4_track -msg _bmedia_index -msg _barena -msg _bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -audio 1 -type ASF ${STREAM_ASF} asf.pes
#ARG = -msg basf_util -msg basf_player -msg _basf_parser -msg basf_stream -msg _bfile_buffer -msg _bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -video 2 -audio 0 -step -player -direction -1 -seek 90000 -type ASF ${STREAM_ASF} asf.pes
#ARG = -msg _bfile_buffer -msg bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -player -audio 0 -video 1 -async -step  -type MP4 ${STREAM_MP4} mp4.pes
#ARG = -msg _bavi_parser -msg _bavi_strem -msg _bfile_cache -msg _bmedia_test -msg _bmedia_index -msg bavi_util -msg _bavi_player -player -type AVI -video 1 -audio 2 -direction 0 ${STREAM_AVI} avi.pes
#ARG = -msg basf_probe -msg bmedia_probe -msg basf_util -msg basf_player -msg _basf_parser -msg basf_stream -msg _bfile_buffer -msg _bmp4_track -msg _bmedia_index -msg _barena -msg bmedia_test -msg _bfile_cache -msg bmp4_player -msg _bmp4_track -msg _bmedia_filter -msg _bmp4_parser -msg bmp4_util -video 0 -audio 0 -step -player -probe -direction -1 -seek 90000 ${STREAM_ASF} asf.pes
#ARG = -msg _bmedia_test -msg basf_stream -msg _bfile_buffer -msg _bmp4_parser -msg bmp4_util -msg bmedia_probe -msg bavi_util -msg basf_util -audio 4 -player -direction 0 -probe ${STREAM_MP4} mp4.pes
#ARG = -msg _bfile_buffer -msg bmedia_test -msg bmedia_probe -msg bmpeg2pes_parser -msg bmpeg2ts_parser -msg bmpeg2psi_parser -msg bmpeg2ts_psi_probe -probe-only -video 0 ${STREAM_TS} mp4.pes
#ARG = -probe-only -msg bfile_buffer -msg baac_adts_probe -msg bmpeg2ts_probe -msg bmpeg2pes_probe -msg bmpeg2pes_parser -msg bmpeg1_probe -msg bmedia_probe_demux -msg bmedia_probe -msg bac3_probe -msg bmpeg_audio_probe -msg bmpeg_video_probe -msg bmedia_probe_es ${STREAM}
#ARG = -probe-only  ${STREAM}
#ARG = -msg _basf_parser -msg basf_stream -msg _bfile_buffer -msg _bmp4_track -msg bmedia_index -msg barena -msg _bmedia_test -msg bfile_cache -msg bmp4_player -msg bmp4_track -msg bmedia_filter -msg bmp4_parser -msg bmp4_util -audio 1 -type ASF ${STREAM_ASF} asf.pes
ARG = -msg _bfile_buffer -msg bmpeg2ts_parser -msg bmpeg2ts_player -msg btime_indexer -msg _bmedia_test -msg bfile_cache -msg bmedia_filter -player -seek 1000000 -seek2 1800000 -video 0x21 -type TS -step ${STREAM_TS} /dev/null
#ARG = -msg bmedia_util -msg bmedia_test -msg bmkv_\* -player -type MKV -video 1 -audio 2 -step ${STREAM_MKV}
#ARG = -msg bmedia_test -msg _bmkv_\* -msg bmkv_player -player -step -type MKV -video 1 -audio 2 -seek 900000 -seek2 1900000 -direction -100000 -async -step ${STREAM_MKV} ${STREAM_MKV}.pes
#ARG = -msg _bmedia_test -msg _bmkv_\* -msg bavi_\*util -player -step -type AVI -video 1 -audio 2 -step -player -seek 900000 ${STREAM_AVI} /dev/null
#ARG = -msg _bmedia_test -msg bmkv_\* -msg bavi_\*util -player -type MKV -async -video 2 -audio 1 -seek 90000 ${STREAM_MKV} /dev/null
#ARG = -msg _bmedia_test -msg bmkv_\* -msg bavi_\*util -player -type MKV -async -direction -30000 -video 2 -audio 1 -seek 0 -step ${STREAM_MKV} /dev/null
#ARG = -msg bmkv_\* -async -player -step -type MKV -video 1 ${STREAM_MKV} /dev/null
#ARG =  -type ASF -video 1 ${STREAM_ASF} /dev/null
#ARG = -type AVI -msg bavi\* -step -msg bmedia\* -video 1 -audio 2 ${STREAM_AVI} /dev/null

valgrind: application
	valgrind -v  --show-reachable=yes --leak-check=yes ${APP_IMAGE} ${ARG}

probe: application
	ls /public/vsilyaev/streams/*/* | valgrind -v  --show-reachable=yes --leak-check=yes ${APP_IMAGE} -msg _bmkv_\* -probe-only -

test: application
	${APP_IMAGE}  ${ARG}

debug: application
	echo "set args ${ARG}" >.gdbinit
	echo "r" >>.gdbinit
	gdb ${APP_IMAGE}
	rm .gdbinit

install: application
	${CP} ${APP_IMAGE} ${DESTDIR}






