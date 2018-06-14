#############################################################################
# Copyright (C) 2018 Broadcom.
# The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to
# the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied),
# right to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
# THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
# IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#############################################################################


######## localisation
ifeq (${URSR_TOP},)
ifneq (${NEXUS_TOP},)
URSR_TOP = ${NEXUS_TOP}/..
else
URSR_TOP = ../../../../../../..
endif
endif

ifeq (${NEXUS_PLATFORM}, )
$(error NEXUS_PLATFORM not set, run the plat command)
endif

# Unified Reference Software path
NEXUS_TOP ?= ${URSR_TOP}/nexus
MAGNUM_TOP ?= ${URSR_TOP}/magnum
BSEAV_TOP ?= ${URSR_TOP}/BSEAV
SAGE_PUBLIC_TOP ?= ${BSEAV_TOP}/lib/security/sage
SRAI_TOP  ?= ${SAGE_PUBLIC_TOP}/srai


include ${MAGNUM_TOP}/syslib/sagelib/bsagelib_public.inc
include ${BSEAV_TOP}/lib/security/sage/utility/utility.inc

# Add example sources
SRCS := utility_securersa_host.c
SRCS += secure_rsa_test_vectors_host.c


OBJS = ${SRCS:.c=.o}

# Host/Sage shared headers
APP_INCLUDES  = ${BSAGELIB_INCLUDES} \
                ${SRAI_TOP}/include \
                ${BSEAV_TOP}/lib/security/common_drm/include/tl \
                ${SAGE_PUBLIC_TOP}/include \
                ${UTILITY_INCLUDES} \
                ${SAGE_PUBLIC_TOP}/platforms/include

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/${NEXUS_PLATFORM}/build/platform_app.inc

#srai lib path first to get latest SRAI lib
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai -lsrai
LDFLAGS += -L${NEXUS_BIN_DIR} -lnexus -lutility
LDFLAGS +=  -lpthread -lm

#debug symbols
CFLAGS += -g

CFLAGS += $(addprefix -I,${APP_INCLUDES})
CFLAGS += $(addprefix -I,${BSAGELIB_INCLUDES})
CFLAGS += $(addprefix -D,${BSAGELIB_DEFINES})
CFLAGS += -DPIC -fpic

# This is the minimum needed to compile and link with Nexus
CFLAGS += ${NEXUS_CFLAGS} $(addprefix -I,${NEXUS_APP_INCLUDE_PATHS})  $(addprefix -D,${NEXUS_APP_DEFINES})

#allow c++ style comments
CFLAGS += -std=c99

ifeq (${SECURE_RSA_TEST},1024)
CFLAGS += -DSECURE_RSA_TEST_1024
else ifeq (${SECURE_RSA_TEST},2048)
CFLAGS += -DSECURE_RSA_TEST_2048
else ifeq (${SECURE_RSA_TEST},3072)
CFLAGS += -DSECURE_RSA_TEST_3072
else
$(error SECURE_RSA_TEST is not set or invalid, export SECURE_RSA_TEST to 1024, 2048, or 3072)
endif

APP = brcm_utility_securersa_tool_$(SECURE_RSA_TEST)

ifeq ($(B_REFSW_OBJ_ROOT),)
SECURE_RSA_OBJDIR:= obj
else
SECURE_RSA_OBJDIR:=${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/utility
endif
export SECURE_RSA_OBJDIR

.PHONY: all clean uninstall install re


all: srai prep_utility prep_nexus ${APP}

# Implicit rule for building local apps
${APP}: ${OBJS}
	@echo [Link... $@]
	${Q_} ${CC} $(addprefix ${SECURE_RSA_OBJDIR}/,${OBJS}) -o ${APP} ${LDFLAGS}

%.o: %.c
	@echo [Compile... $<]
	@if ! [ -e "${SECURE_RSA_OBJDIR}" ]; then \
		mkdir -p ${SECURE_RSA_OBJDIR}; \
	fi;
	${Q_} ${CC} ${CFLAGS} -c $< -o ${SECURE_RSA_OBJDIR}/$@

clean:
	@echo [Clean... ${APP}]
	${Q_} rm -f ${OBJS}
	${Q_} rm -f ${APP}

srai:
	make -C ${SRAI_TOP} install

prep_utility:
	@echo [Building Utility lib ...]
	make -C ${SAGE_PUBLIC_TOP}/utility re

prep_nexus:
	@if [ ! -e $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/nexus/bin/libnexus.so ]; then \
		echo [ building nexus .... ] && \
		$(MAKE) -C $(NEXUS_TOP)/build >/dev/null; \
	fi

uninstall:
	@echo [Uninstall... ${APP}]
	${Q_} rm -f ${NEXUS_BIN_DIR}/${APP}
	@echo [Uninstall... SRAI]
	${Q_} rm -f ${NEXUS_BIN_DIR}/libsrai.*

install:
	@echo [Installing ${APP}... in ${NEXUS_BIN_DIR}]
	${Q_} install -m 0755 ${APP} ${NEXUS_BIN_DIR}
	@echo [Installing SRAI... in ${NEXUS_BIN_DIR}]
	${Q_} install -m 0755 ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai/* ${NEXUS_BIN_DIR}

re: srai prep_utility uninstall clean all install
