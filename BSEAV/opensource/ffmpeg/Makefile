#***************************************************************************
#*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#*
#*  This program is the proprietary software of Broadcom and/or its licensors,
#*  and may only be used, duplicated, modified or distributed pursuant to the terms and
#*  conditions of a separate, written license agreement executed between you and Broadcom
#*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#*  no license (express or implied), right to use, or waiver of any kind with respect to the
#*  Software, and Broadcom expressly reserves all rights in and to the Software and all
#*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#*
#*  Except as expressly set forth in the Authorized License,
#*
#*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#*  and to use this information only in connection with your use of Broadcom integrated circuit products.
#*
#*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#*  USE OR PERFORMANCE OF THE SOFTWARE.
#*
#*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#*  ANY LIMITED REMEDY.
#**************************************************************************
include ../../api/build/tools.mak
BSEAV = $(shell cd "../.." && ${PWD})
ROCKFORD = $(BSEAV)/../rockford
NEXUS_TOP = ${BSEAV}/../nexus

ifndef LIB
APP = ffmpeg_nexus
endif

all: ffmpeg-2.4 install
#all: ${FFMPEG_BIN}/ffmpeg-2.4/.done

NEXUS_C_WARNINGS := -Wall -W -Wstrict-prototypes
NEXUS_C_STD := gnu9x

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

STD_INC ?= $(TOOLCHAIN_DIR)/mipsel-linux/include
GCC_BASE ?= $(shell (test -d $(TOOLCHAIN_DIR)/lib/gcc-lib && echo $(TOOLCHAIN_DIR)/lib/gcc-lib/mipsel-linux) || echo $(TOOLCHAIN_DIR)/lib/gcc/mipsel-linux)
GCC_VER = $(shell (ls $(GCC_BASE)))
GCC_INC = $(GCC_BASE)/$(GCC_VER)/include

CFLAGS += -I. -Os -g
CFLAGS += -fpic -fPIC
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))



vpath %.c  ${FFMPEG_VERSION}/libavcodec ${FFMPEG_VERSION}/libavutil ${FFMPEG_VERSION}/libavformat

CFLAGS += ${FFMPEG_CFLAGS} -Os

BPROFILE_SUPPORT ?=n
BTRC_SUPPORT ?=n

ifndef LIB
SRCS := ffmpeg_nexus.c ffmpeg_buffers.c
else
SRCS := bsoft_ffmpeg_decode.c
endif


FFMPEG_BIN := ${B_REFSW_OBJ_ROOT}/BSEAV/opensource/ffmpeg
CFLAGS += -I${FFMPEG_BIN}/ffmpeg-2.4 -I${BSEAV}/lib/media -I${BSEAV}/lib/utils
vpath %c $(sort $(dir ${BFLV_FILTER_SOURCES} ${BMEDIA_PROBE_SOURCES} ${BFILE_STDIO_SOURCES} ${TSPSI_SOURCES}))

ifeq ($(BTRC_SUPPORT),y)
include ${BSEAV}/lib/bprofile/bprofile.inc
vpath %.c  ${BSEAV}/lib/bprofile
CFLAGS += -I${BSEAV}/lib/bprofile -I${BSEAV}/lib/utils
SRCS += bsymtable.c
SRCS += btrc.c bperf_counter.c
ifeq ($(BPROFILE_SUPPORT),y)
SRCS += bprofile.c bprofile_data.c
SYM_SRC = bsymbols.c
CFLAGS += -DB_HAS_SYMTABLE
CFLAGS_BPROFILE=-finstrument-functions -DB_HAS_BPROFILE=1 -DBPROFILE_CFG_SINGLE_THREAD=1
endif
endif


CROSS_COMPILE ?= mipsel-linux-

LDFLAGS += -lnexus -L$(NEXUS_BIN_DIR) -lpthread -lm


ODIR = ${FFMPEG_BIN}
OS = ${B_REFSW_OS}

#include ${ROCKFORD}/modules/basemodules.inc

ifeq ($(SYSTEM),linuxkernel)
BKNI_SOURCES += $(MAGNUM)/basemodules/kni/$(OS)/bkni.c
endif

#include $(ROCKFORD)/modules/transition.inc
#include ${ROCKFORD}/inc/compose.mak

#OBJS += ${MAGNUM_OBJS}
LIBS = ${FFMPEG_LIBS}

OTHER_CLEANS = ffmpeg_clean

include ${BSEAV}/api/build/rules.mak

test:
	echo ${APP_IMAGE} ${MAGNUM_OBJS}

install: application
	cp ${APP_IMAGE} ${NEXUS_BIN_DIR}

ffmpeg_clean:
	-mv ${FFMPEG_VERSION} ${FFMPEG_VERSION}.old
	${RM} -rf ${FFMPEG_VERSION}.old

#${OBJS}: ${FFMPEG_VERSION}

lib: ${FFMPEG_VERSION} static

extract:  ${FFMPEG_VERSION}
${FFMPEG_VERSION}:
	tar -jxf ffmpeg-export-snapshot.tar.bz2
	(cd ${FFMPEG_VERSION};patch -p 1 <../ffmpeg.patch; patch -p 1 <../ffmpeg.patch.ogg)
	${CP} ffmpeg_config.h ${FFMPEG_VERSION}/config.h
	mv ${FFMPEG_VERSION}/libavformat/utils.c ${FFMPEG_VERSION}/libavformat/avf_utils.c

LIBRARY_OBJS := $(LIBRARY_SRCS:%.c=${ODIR}/%.o)
LIBRARY := ${ODIR}/libsoftdecode.a
library: ${LIBRARY}

${LIBRARY}: ${LIBRARY_OBJS}
	@echo [Linking... $(notdir $@)]
	${Q_}$(RM) $@
	${Q_}$(AR) cr $@  $^
	${Q_}$(RANLIB) $@

.NOTPARALLEL: ffmpeg-2.4.library ffmpeg-2.4.library.full static application

LIBS=${FFMPEG_BIN}/ffmpeg-2.4/libavfilter/libavfilter.a ${FFMPEG_BIN}/ffmpeg-2.4/libswscale/libswscale.a ${FFMPEG_BIN}/ffmpeg-2.4/libavformat/libavformat.a ${FFMPEG_BIN}/ffmpeg-2.4/libavcodec/libavcodec.a ${FFMPEG_BIN}/ffmpeg-2.4/libswresample/libswresample.a  ${FFMPEG_BIN}/ffmpeg-2.4/libavutil/libavutil.a

ifndef LIB
ffmpeg-2.4: ${FFMPEG_BIN}/ffmpeg-2.4/.done application
else
ffmpeg-2.4: ${FFMPEG_BIN}/ffmpeg-2.4/.done static
endif

ffmpeg-2.4.clean:
	${RM} -rf ${FFMPEG_BIN}

ffmpeg-2.4.library:
	${MAKE} -C ${FFMPEG_BIN}/ffmpeg-2.4

${FFMPEG_BIN}/ffmpeg-2.4/.done:
	rm -rf ${FFMPEG_BIN}/ffmpeg-2.4;mkdir -p ${FFMPEG_BIN}/ffmpeg-2.4
	B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} ${BSEAV}/tools/build/fetch.sh ffmpeg.info ${FFMPEG_BIN};
	tar -jxf ${FFMPEG_BIN}/ffmpeg-2.4.tar.bz2 -C ${FFMPEG_BIN}
	(cd ${FFMPEG_BIN}/ffmpeg-2.4; ./configure --arch=armv6 --target-os=linux --cross-prefix=arm-linux- --extra-cflags="-fPIC -fpic -marm")
	(cd ${FFMPEG_BIN}/ffmpeg-2.4; patch) < ffmpeg-2.4.patch
	${MAKE} -C ${FFMPEG_BIN}/ffmpeg-2.4
	touch ${FFMPEG_BIN}/ffmpeg-2.4/.done
