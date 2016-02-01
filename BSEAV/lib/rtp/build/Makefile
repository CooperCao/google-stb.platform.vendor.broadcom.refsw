############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
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
############################################################

include ../../../api/build/tools.mak
BSEAV = $(shell cd "../../.." && ${PWD})
ROCKFORD = $(BSEAV)/../rockford
MAGNUM = $(BSEAV)/../magnum

APP = rtp
include Makefile.rtp
CFLAGS += $(B_REFSW_CFLAGS) $(B_REFSW_GENERIC_MAGNUM_CFLAGS) $(B_REFSW_MAGNUM_INCLUDE_DIRS)

SRCS += brtp_test.c 
CFLAGS += -I.



include $(BSEAV)/build/refsw_inc.mak

ODIR = $(B_REFSW_ARCH).$(DEBUG_SUFFIX)

OS = ${B_REFSW_OS}

include ${MAGNUM}/basemodules/std/bstd.inc
include ${MAGNUM}/basemodules/kni/bkni.inc
include ${MAGNUM}/basemodules/dbg/bdbg.inc
include ${MAGNUM}/commonutils/lst/blst.inc
include ${MAGNUM}/commonutils/vlc/bvlc.inc
include ${ROCKFORD}/inc/compose.mak

MAGNUM_SRC := $(sort $(foreach module, $(MAGNUM_MODULES), $($(module)_SOURCES)))
MAGNUM_OBJ := $(patsubst %.c,%.o,$(MAGNUM_SRC))

LDFLAGS += -lpthread ${PROFILE_FLAGS}
OBJS += ${MAGNUM_OBJ}

OTHER_CLEANS = backup_clean 

all: application
include Makefile.livemedia


include ../../../api/build/rules.mak

backup_clean:
	${RM} -rv core* *~ ../*~ ./*~ ../../bfile/*~ ../../utils/*~ gmon.out *.debug *.release .*.swp

debug: application
	echo "set args /public/vsilyaev/work/rtp/seq1/rtp_dump_1.rtp" >.gdbinit
	gdb ${APP_IMAGE} 

stream:
	/public/vsilyaev/work/rtptools/rtptools-1.18/rtpplay  -T -f /public/vsilyaev/work/rtp/seq2/rtp_dump_0.rtp 10.6.1.61/1026 |\
	/public/vsilyaev/work/rtptools/rtptools-1.18/rtpplay  -T -f /public/vsilyaev/work/rtp/seq2/rtp_dump_1.rtp 10.6.1.61/1028 

install:  application
	cp ${APP_IMAGE} ${DESTDIR}

test: application
	#${APP_IMAGE} -time  /public/vsilyaev/97398_rtp/cartoon.pes.rtp
	#${APP_IMAGE} -timed_rtp -timed_pkt -msg _brtp_parser -play rtsp://video1.multicasttech.com/AFTVCartoonsH264500.sdp cartoon.pes.rtp
	#${APP_IMAGE} -timed_pkt -msg _brtp_parser -play rtsp://video1.multicasttech.com/AFTVCartoonsH264500.sdp sw.pes
	#${APP_IMAGE} -timed_pkt -timed_rtp -msg _btimestamp -msg _brtp -play rtsp://10.6.0.251/sw.mp4  sw.pes
	#${APP_IMAGE} -play2 rtsp://10.6.0.251/sw.mp4  sw.pes
	${APP_IMAGE} -play2 rtsp://10.6.0.251/sample_h264_1mbit.mp4 /dev/null
	#${APP_IMAGE} -play rtsp://10.6.0.251/sw.mp4  sw.pes
	#${APP_IMAGE} -timed_rtp -msg _btimestamp -msg _brtp -play rtsp://10.6.0.251/sw.mp4  sw.pes
	#${APP_IMAGE} -play rtsp://10.6.0.251/sample_h264_1mbit.mp4  sw.pes
	#${APP_IMAGE} -msg brtp_test -time  /public/vsilyaev/97398_rtp/cartoon.timed.pes 2>&1|less
	#${APP_IMAGE} -time /public/vsilyaev/97398_rtp/sw-time.pes
	#${APP_IMAGE} -time cartoon.pes.rtp 2>&1 | less
	#${APP_IMAGE}  -msg _brtp_parser -msg brtp_parser_mpeg4 -timed_pkt -play rtsp://10.6.0.251/sw.mp4
	#${APP_IMAGE}  -msg _bbase64 -msg _brtp_parser_mpeg4 -msg _brtp_parser_h264 -timed_pkt -play rtsp://10.6.0.251/elvis.mp4
	#${APP_IMAGE}  -msg _brtp_parser -msg brtp_parser_mpeg4 -timed_pkt -play rtsp://10.6.0.251/sample_h264_1mbit.mp4
	#${APP_IMAGE}  -msg _brtp_parser -msg brtp_parser_mpeg4 -timed_pkt -play rtsp://10.6.0.251/sample_h264_100kbit.mp4
	#${APP_IMAGE} -timed_pkt -play rtsp://video1.multicasttech.com/AFTVCartoonsH264500.sdp
	#${APP_IMAGE} -type aac -config 1190 -clock_rate 48000 -pes_id 0xc0 /public/vsilyaev/work/rtp/h264_1mbit/rtp_dump_0.rtp -- -type h264 -pes_id 0xe0 -sprop J01AHqkYMB73oA==,KM4C+IA= /public/vsilyaev/work/rtp/h264_1mbit/rtp_dump_1.rtp av.pes && cmp av.pes h264_1mbit.pes
	#${APP_IMAGE} -type aac -config 1190 -clock_rate 48000 -pes_id 0xc0 /public/vsilyaev/work/rtp/h264_1mbit/rtp_dump_0.rtp aac.pes
	#${APP_IMAGE} -type h264 -pes_id 0xe0 -sprop J01AHqkYMB73oA==,KM4C+IA= /public/vsilyaev/work/rtp/h264_1mbit/rtp_dump_1.rtp h264.pes

	#${APP_IMAGE} -type aac -pes_id 0xc0 -clock_offset 1258753135 :1026 -- -type h264 -pes_id 0xe0 -clock_offset 1342679248 :1028 av.pes |\
	/public/vsilyaev/work/rtptools/rtptools-1.18/rtpplay  -T -f /public/vsilyaev/work/rtp/seq2/rtp_dump_0.rtp localhost/1026 |\
	/public/vsilyaev/work/rtptools/rtptools-1.18/rtpplay  -T -f /public/vsilyaev/work/rtp/seq2/rtp_dump_1.rtp localhost/1028 

	#${APP_IMAGE} -msg brtp_test -type h264 -pes_id 0xe0 :1026 h264.pes 
	#${APP_IMAGE} -msg brtp_test -pes_id 0 /public/vsilyaev/work/rtp/seq3/rtp_dump_0.rtp aac.es
	#${APP_IMAGE} -type aac -pes_id 0xc0 -clock_offset 1258753135 /public/vsilyaev/work/rtp/seq2/rtp_dump_0.rtp -- -type h264 -pes_id 0xe0 -clock_offset 1342679248 /public/vsilyaev/work/rtp/seq2/rtp_dump_1.rtp av.pes
	#${APP_IMAGE} -type aac -pes_id 0 /public/vsilyaev/work/rtp/seq2/rtp_dump_0.rtp aac.es && cmp aac.es aac.es.test
	#${APP_IMAGE} -type aac -clock_rate 44100 -pes_id 0xc0 /public/vsilyaev/work/rtp/seq2/rtp_dump_0.rtp aac.pes && cmp aac.pes aac.pes.test
	#${APP_IMAGE} -type h264 -pes_id 0x0 /public/vsilyaev/work/rtp/seq2/rtp_dump_1.rtp h264.es && cmp h264.es h264.es.test
	#${APP_IMAGE} -type h264 -pes_id 0xe0 /public/vsilyaev/work/rtp/seq2/rtp_dump_1.rtp h264.pes && cmp h264.pes h264.pes.test
	#${APP_IMAGE} -pes_id 0 /public/vsilyaev/work/livemedia/live/testProgs/rtp_dump_1.rtp aac.es
	#${APP_IMAGE} /public/vsilyaev/work/rtp/seq3/rtp_dump_1.rtp
	#${APP_IMAGE} /public/vsilyaev/work/rtp/seq1/rtp_dump_1.rtp
	#${APP_IMAGE} /public/vsilyaev/work/livemedia/live/testProgs/rtp_dump_0.rtp





