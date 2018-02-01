###############################################################################
#   Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to the terms and
#   conditions of a separate, written license agreement executed between you and Broadcom
#   (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#   no license (express or implied), right to use, or waiver of any kind with respect to the
#   Software, and Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#   HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#   NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#   secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#   and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#   AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#   WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#   THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#   OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#   LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#   OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#   USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#   LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#   EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#   USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#   THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#   ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#   LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#   ANY LIMITED REMEDY.
#
###############################################################################
B_THIS_DIR ?= BSEAV/tools/astratop

PHONY := all $(BINDIR)/../bin/boa_server clean_boa clean_apps install clean
.PHONY: ${PHONY}
.NOTPARALLEL: apps $(BINDIR)/../bin/boa_server
ifeq ($(LINUXDIR),)
LINUXDIR = $(LINUX)
endif

APPS = \
	${NEXUS_BIN_DIR}/astratop.cgi \
	${NEXUS_BIN_DIR}/astratop_server \
	${NEXUS_BIN_DIR}/astratop.html \
	${NEXUS_BIN_DIR}/astratop.js \
	${NEXUS_BIN_DIR}/astratop.css \
	${NEXUS_BIN_DIR}/astratop.png \
	${NEXUS_BIN_DIR}/bmemperf_play.jpg \
	${NEXUS_BIN_DIR}/bmemperf_pause.jpg \
	${NEXUS_BIN_DIR}/stop.png

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(B_REFSW_REAL_MAKE),)
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/astratop

${PHONY} ${APPS}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile ${MAKECMDGOALS}

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

vpath %.c ${NEXUS_TOP}/../BSEAV/tools/astratop
all: apps

apps: $(APPS)

BOA_SRC_DIR = $(NEXUS_TOP)/../BSEAV/opensource/boa
BOA_SRC_FILES = $(shell find $(BOA_SRC_DIR) -name "Makefile" -o -name "boa*.gz" -o -name "boa.conf" -o -name "index*" )

${NEXUS_BIN_DIR}/boa_server: $(BOA_SRC_FILES)
	@echo [Building ... $(notdir $@)]
	$(MAKE) -C $(BOA_SRC_DIR)

ifeq ($(TZ_ARCH),Arm64)
    G++ = aarch64-linux-g++
else ifeq ($(TZ_ARCH),Arm32)
    G++ = arm-linux-g++
endif
MAIN_OBJS = ${NEXUS_TOP}/../${B_THIS_DIR}/astratop.c
SERVER_OBJS = ${NEXUS_TOP}/../${B_THIS_DIR}/astratop_server.cpp
EXTRA_OBJS += ${NEXUS_TOP}/../BSEAV/tools/bmemperf/common/bmemperf_lib.c ${NEXUS_TOP}/../${B_THIS_DIR}/LUT.c

LDFLAGS += -lpthread
ASTRA_SERVER_CFLAGS += -I$(CURDIR)
${NEXUS_BIN_DIR}/astratop_server: $(SERVER_OBJS)
	@echo [Building ... $@]
	$(G++) -std=c++11 $(ASTRA_SERVER_CFLAGS) $(SERVER_OBJS) -o $@ $(LDFLAGS)
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/astratop.sh   ${NEXUS_BIN_DIR}
	$(Q_)rm -f ${NEXUS_BIN_DIR}/astratop_server.d

#-include *.d

CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -I${NEXUS_TOP}/../BSEAV/tools/bmemperf/include
#CFLAGS += -I${LINUXDIR}/include
#CFLAGS += -I${NEXUS_BIN_DIR}
CFLAGS += -Wall -lm
CFLAGS += -DBMEMCONFIG_READ32_SUPPORTED
#CFLAGS += -I$(NEXUS_TOP)/../magnum/portinginterface/xpt/src/core28nm
CFLAGS += -g
#CFLAGS += -D_LINUX_TYPES_H

# Implicit rule for building local apps
${NEXUS_BIN_DIR}/astratop.cgi: ${NEXUS_BIN_DIR}/boa_server $(MAIN_OBJS) $(EXTRA_OBJS)
	$(Q_)$(CC) -o $@ $(MAIN_OBJS) $(EXTRA_OBJS) $(CFLAGS)
	$(Q_)rm -f ${NEXUS_BIN_DIR}/astratop.d

${NEXUS_BIN_DIR}/astratop.html: ${NEXUS_TOP}/../${B_THIS_DIR}/astratop.html
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/astratop.html     ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/astratop.js: ${NEXUS_TOP}/../${B_THIS_DIR}/astratop.js
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/astratop.js     ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/astratop.css: ${NEXUS_TOP}/../${B_THIS_DIR}/astratop.css
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/astratop.css     ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/astratop.png: ${NEXUS_TOP}/../${B_THIS_DIR}/astratop.png
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/astratop.png     ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/player/bmemperf_play.jpg     ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/player/bmemperf_pause.jpg    ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/stop.png: ${NEXUS_TOP}/../${B_THIS_DIR}/stop.png
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/stop.png ${NEXUS_BIN_DIR}

install: all
	@echo [Installing to ... ${NEXUS_BIN_DIR}]
	$(Q_)$(CP) -f ${APPS} ${NEXUS_BIN_DIR}

clean_boa:
	$(MAKE) -C $(BOA_SRC_DIR) clean

clean_apps:
	@-$(RM) $(APPS) *.d *.o *.out
	@-$(RM) ${NEXUS_BIN_DIR}/astratop.html
	@-$(RM) ${NEXUS_BIN_DIR}/astratop.js
	@-$(RM) ${NEXUS_BIN_DIR}/astratop.png
	@-$(RM) ${NEXUS_BIN_DIR}/stop.png
	@-$(RM) ${NEXUS_BIN_DIR}/astratop.cgi
	@-$(RM) ${NEXUS_BIN_DIR}/astratop.sh
	@-$(RM) ${NEXUS_BIN_DIR}/astratop_server
	@-$(RM) ${NEXUS_BIN_DIR}/astratop.css
	@-$(RM) ${NEXUS_BIN_DIR}/bmemperf_play.jpg
	@-$(RM) ${NEXUS_BIN_DIR}/bmemperf_pause.jpg

clean: clean_apps clean_boa
endif