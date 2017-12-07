############################################################
#  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
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
############################################################
THIS_DIR := BSEAV/opensource/libbacktrace
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

all: build
.PHONY: all clean build

THIS_PATH=$(B_REFSW_TOP)/${THIS_DIR}
LIBBACKTRACE_VERSION=libbacktrace-14d377e9be7c89511c472d728e1b88b4e96f1946
LIBBACKTRACE_ARCHIVE=${LIBBACKTRACE_VERSION}.zip
LIBBACKTRACE_ROOT=$(abspath $(B_REFSW_OBJ_ROOT)/${THIS_DIR})
LIBBACKTRACE_SOURCE=${LIBBACKTRACE_ROOT}/${LIBBACKTRACE_VERSION}


build: ${LIBBACKTRACE_ROOT}/libbacktrace.a

${LIBBACKTRACE_SOURCE}/.source:
	${RM} -rf ${LIBBACKTRACE_ROOT}
	${MKDIR} ${LIBBACKTRACE_ROOT}
	${BSEAV_TOP}/tools/build/fetch.sh libbacktrace.info ${LIBBACKTRACE_ROOT}
	(cd ${LIBBACKTRACE_ROOT} && unzip ${LIBBACKTRACE_ARCHIVE})
	touch $@

${LIBBACKTRACE_ROOT}/libbacktrace.a: ${LIBBACKTRACE_SOURCE}/.source
	(cd ${LIBBACKTRACE_SOURCE} && env CFLAGS='-DPIC -fpic' ./configure --build=i686-linux-gnu --host=${B_REFSW_ARCH} && ${MAKE})
	${RM} ${LIBBACKTRACE_ROOT}/backtrace-supported.h ${LIBBACKTRACE_ROOT}/backtrace.h ${LIBBACKTRACE_ROOT}/libbacktrace.a
	ln -s ${LIBBACKTRACE_SOURCE}/backtrace-supported.h ${LIBBACKTRACE_ROOT}
	ln -s ${LIBBACKTRACE_SOURCE}/backtrace.h ${LIBBACKTRACE_ROOT}
	ln -s ${LIBBACKTRACE_SOURCE}/.libs/libbacktrace.a ${LIBBACKTRACE_ROOT}/libbacktrace.a

clean :
	${RM} -rf ${LIBBACKTRACE_ROOT}
