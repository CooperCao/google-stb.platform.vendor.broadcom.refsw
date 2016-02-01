############################################################
#     (c)2003-2013 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
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
include ../../api/build/tools.mak
BSEAV := $(shell cd "../.." && ${PWD})
ROCKFORD = $(BSEAV)/../rockford
NEXUS_TOP = ${BSEAV}/../nexus
LIBVPX_VERSION = libvpx-1.5.0
LIB := softdecode

all: libvpx static

NEXUS_C_WARNINGS := -Wall -W -Wstrict-prototypes
NEXUS_C_STD := gnu9x

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

include $(BSEAV)/build/refsw_inc.mak

SRCS := bsoft_vpx_decode.c

LIBVPX_BIN := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/libvpx
ODIR = ${LIBVPX_BIN}

LIBVPX?=${LIBVPX_BIN}/${LIBVPX_VERSION}/vpx/libvpx.a
LIBVPX_INCLUDE?=${LIBVPX_BIN}/${LIBVPX_VERSION}

CFLAGS += -I. -Os -g
CFLAGS += -fpic -fPIC
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -I${BSEAV}/lib/ffmpeg
CFLAGS += -I${LIBVPX_INCLUDE} -Os

CLIBS = ${LIBVPX_LIBS}

OTHER_CLEANS = libvpx_clean

include ${BSEAV}/api/build/rules.mak

install: application
	cp ${APP_IMAGE} ${NEXUS_BIN_DIR}

libvpx_clean:
	-mv ${LIBVPX_VERSION} ${LIBVPX_VERSION}.old
	${RM} -rf ${LIBVPX_VERSION}.old

${OBJS}: ${LIBVPX_VERSION}



.NOTPARALLEL: libvpx static

LIBS=${LIBVPX}

libvpx: ${LIBVPX}

${LIBVPX_VERSION}_clean:
	${RM} -rf ${LIBVPX_BIN}

${LIBVPX}:
	rm -rf ${LIBVPX_BIN};mkdir -p ${LIBVPX_BIN}
	B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} ${BSEAV}/tools/build/fetch.sh libvpx.info ${LIBVPX_BIN};
	tar -jxf ${LIBVPX_BIN}/${LIBVPX_VERSION}.tar.bz2 -C ${LIBVPX_BIN}
	(cd ${LIBVPX_BIN}/${LIBVPX_VERSION}/vpx; env CROSS=arm-linux-gnueabihf- ../configure --target=armv7-linux-gcc --enable-pic --disable-install-docs --disable-vp8 --disable-vp10 --disable-vp9-encoder)
	${MAKE} -C ${LIBVPX_BIN}/${LIBVPX_VERSION}/vpx
