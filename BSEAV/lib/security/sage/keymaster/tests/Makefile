#############################################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
include ${BSEAV_TOP}/lib/security/sage/keymaster/keymaster.inc
include ${BSEAV_TOP}/opensource/openssl/openssl.inc

ifeq ($(NXCLIENT_SUPPORT),y)
include $(URSR_TOP)/nexus/nxclient/include/nxclient.inc
endif

APP = keymaster_test

# Add example sources
SRCS := $(wildcard *.c)


OBJS = ${SRCS:.c=.o}

# Host/Sage shared headers
APP_INCLUDES  = ${BSAGELIB_INCLUDES} \
                ${SRAI_TOP}/include \
                ${BSEAV_TOP}/lib/security/common_drm/include/tl \
                ${SAGE_PUBLIC_TOP}/include \
                ${KEYMASTER_INCLUDES} \
                ${SAGE_PUBLIC_TOP}/platforms/include \
                ${B_OPENSSL_INC_INSTALL_DIR}

ifeq ($(NXCLIENT_SUPPORT),y)
APP_INCLUDE += ${NXCLIENT_INCLUDES}
endif

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/${NEXUS_PLATFORM}/build/platform_app.inc

#srai lib path first to get latest SRAI lib
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai -lsrai
LDFLAGS += -L${NEXUS_BIN_DIR} -lnexus -lsage_keymaster
LDFLAGS += -L${B_OPENSSL_LIB_INSTALL_DIR} -lssl -lcrypto
ifeq ($(NXCLIENT_SUPPORT),y)
LDFLAGS += ${NXCLIENT_LDFLAGS}
endif
LDFLAGS +=  -lpthread -lm

#debug symbols
CFLAGS += -g

CFLAGS += $(addprefix -I,${APP_INCLUDES})
CFLAGS += $(addprefix -I,${BSAGELIB_INCLUDES})
CFLAGS += $(addprefix -D,${BSAGELIB_DEFINES})
ifeq ($(NXCLIENT_SUPPORT),y)
CFLAGS += $(NXCLIENT_CFLAGS)
endif
CFLAGS += -DPIC -fpic -Wall

# This is the minimum needed to compile and link with Nexus
CFLAGS += ${NEXUS_CFLAGS} $(addprefix -I,${NEXUS_APP_INCLUDE_PATHS})  $(addprefix -D,${NEXUS_APP_DEFINES})

#allow c++ style comments
CFLAGS += -std=c99
ifeq ($(ANDROID_BUILD),y)
CFLAGS += -DANDROID
endif

ifeq ($(B_REFSW_OBJ_ROOT),)
KEYMASTER_OBJDIR:= obj
else
KEYMASTER_OBJDIR:=${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/keymaster
endif
export KEYMASTER_OBJDIR

.PHONY: all clean uninstall install re


all: srai prep_keymaster prep_nexus ${APP}

# Implicit rule for building local apps
${APP}: openssl ${OBJS}
	@echo [Link... $@]
	${Q_} ${CC} $(addprefix ${KEYMASTER_OBJDIR}/,${OBJS}) -o ${APP} ${LDFLAGS}

.PHONY:  openssl
openssl:
	@echo [Build..... OpenSSL]
	$(Q_)$(MAKE) -C $(B_OPENSSL_BUILD_DIR)

%.o: %.c
	@echo [Compile... $<]
	@if ! [ -e "${KEYMASTER_OBJDIR}" ]; then \
		mkdir -p ${KEYMASTER_OBJDIR}; \
	fi;
	${Q_} ${CC} ${CFLAGS} -c $< -o ${KEYMASTER_OBJDIR}/$@

clean:
	@echo [Clean... ${APP}]
	${Q_} rm -f ${OBJS}
	${Q_} rm -f ${APP}

srai:
	make -C ${SRAI_TOP} install

prep_keymaster:
	@echo [Building Keymaster lib ...]
	make -C ${SAGE_PUBLIC_TOP}/keymaster re

prep_nexus:
ifeq ($(NXCLIENT_SUPPORT),y)
	@if [ ! -e $(NEXUS_TOP)/../$(B_REFSW_OBJ_DIR)/nexus/bin/nxserver ]; then \
		echo [ building nxclient .... ] && \
		$(MAKE) -C $(NEXUS_TOP)/nxclient >/dev/null; \
	fi
else
	@if [ ! -e $(NEXUS_TOP)/../$(B_REFSW_OBJ_DIR)/nexus/bin/libnexus.so ]; then \
		echo [ building nexus .... ] && \
		$(MAKE) -C $(NEXUS_TOP)/build >/dev/null; \
	fi
endif

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

re: srai prep_keymaster uninstall clean all install
