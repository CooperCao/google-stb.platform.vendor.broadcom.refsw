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
# Nexus examples
B_THIS_DIR:=BSEAV/tools/bsysperf
PHONY := all $(BINDIR)/../bin/boa_server clean clean_apps install nexus_headers clean_boa
.PHONY: ${PHONY}
.NOTPARALLEL: apps $(BINDIR)/../bin/boa_server

ifeq ($(LINUXDIR),)
LINUXDIR = $(LINUX)
endif

THEAPPS = \
	${NEXUS_BIN_DIR}/bsysperf.cgi \
	${NEXUS_BIN_DIR}/bsysperf_server \
	${NEXUS_BIN_DIR}/bsysperf.html \
	${NEXUS_BIN_DIR}/bsysperf.js \
	${NEXUS_BIN_DIR}/bsysperf.css \
	${NEXUS_BIN_DIR}/bsysperf.png \
	${NEXUS_BIN_DIR}/bmemperf_play.jpg \
	${NEXUS_BIN_DIR}/bmemperf_pause.jpg \
	${NEXUS_BIN_DIR}/bsysperf_flame.js \
	${NEXUS_BIN_DIR}/chart1.js \
	${NEXUS_BIN_DIR}/chart2.js \
	${NEXUS_BIN_DIR}/boundingbox.js \
	${NEXUS_BIN_DIR}/svgvml3d.js \
	${NEXUS_BIN_DIR}/iperf \

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(B_REFSW_REAL_MAKE),)
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/bsysperf

${PHONY} ${THEAPPS}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile ${MAKECMDGOALS}

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/bsysperf

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

APPS = $(THEAPPS)
APPS ?= $(APP)

vpath %.c ${NEXUS_TOP}/../BSEAV/tools/bsysperf

all: apps

apps: $(APPS)

LDFLAGS += -lpthread
MAIN_OBJS = ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.c ${NEXUS_TOP}/../${B_THIS_DIR}/bheaps.c
SERVER_OBJS = ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf_server.c
EXTRA_OBJS += ${NEXUS_TOP}/../BSEAV/tools/bmemperf/common/bmemperf_lib.c
EXTRA_OBJS += ${NEXUS_TOP}/../BSEAV/tools/bmemperf/common/bsysperf_wifi.c
CLKGEN_PATH = ${NEXUS_TOP}/../magnum/basemodules/chp/src/common

ifeq ($(findstring $(BCHP_CHIP),7271 ),$(BCHP_CHIP))
WLAN_DRIVER = "y"
export WLAN_DRIVER_PATH="BSEAV/connectivity/wlan/core"
BWL_OBJ_DIR = $(NEXUS_TOP)/../$(B_REFSW_OBJ_DIR)/BSEAV/lib/bwl
BWL_SRC_DIR = $(NEXUS_TOP)/../BSEAV/lib/bwl
BWL_SRC_FILES = $(shell find $(BWL_SRC_DIR) -name "*.c" -o -name "*.h" -o -name "Makefile" )
else
endif

BOA_SRC_DIR = $(NEXUS_TOP)/../BSEAV/opensource/boa
BOA_SRC_FILES = $(shell find $(BOA_SRC_DIR) -name "Makefile" -o -name "boa*.gz" -o -name "boa.conf" -o -name "index*" )

IPERF_SRC_DIR = $(NEXUS_TOP)/../BSEAV/opensource/iperf
IPERF_SRC_FILES = $(shell find $(IPERF_SRC_DIR) -name "Makefile" -o -name "*.info" -o -name "*.inc" )

# This cleans nexus and local apps
clean: clean_apps clean_boa clean_bwl clean_iperf

$(BINDIR)/../bin/boa_server: $(BOA_SRC_FILES)
	@echo [Building ... $(notdir $@)]
	$(MAKE) -C $(BOA_SRC_DIR)

${NEXUS_BIN_DIR}/iperf: $(IPERF_SRC_FILES)
	@echo [Building ... $(notdir $@)]
	$(MAKE) -C $(IPERF_SRC_DIR) install_to_nexus_bin

clean_iperf:
	$(MAKE) -C $(IPERF_SRC_DIR) clean

clean_boa:
	$(MAKE) -C $(BOA_SRC_DIR) clean

clean_apps:
	@-$(RM) $(APPS) *.d *.o *.out
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf.html
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf.js
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf_flame.js
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf*.png
	@-$(RM) ${NEXUS_BIN_DIR}/bmemperf_play.jpg
	@-$(RM) ${NEXUS_BIN_DIR}/bmemperf_pause.jpg
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf.cgi
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf.sh
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf_server
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf.css
	@-$(RM) ${NEXUS_BIN_DIR}/chart*.js
	@-$(RM) ${NEXUS_BIN_DIR}/boundingbox.js
	@-$(RM) ${NEXUS_BIN_DIR}/svgvml3d.js

${NEXUS_BIN_DIR}/bsysperf.html: ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.html
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.html     ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/bsysperf.js: ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.js
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.js     ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/bsysperf.css: ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.css
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.css     ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/bsysperf.png: ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.png ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf_record1.png ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf_record2.png
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.png     ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf_record1.png     ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf_record2.png     ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/player/bmemperf_play.jpg     ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/player/bmemperf_pause.jpg    ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/bsysperf_flame.js: ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf_flame.js
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf_flame.js     ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/chart1.js: ${NEXUS_TOP}/../${B_THIS_DIR}/chart1.js
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/chart1.js ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/chart2.js: ${NEXUS_TOP}/../${B_THIS_DIR}/chart2.js
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/chart2.js ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/boundingbox.js: ${NEXUS_TOP}/../${B_THIS_DIR}/boundingbox.js
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/boundingbox.js ${NEXUS_BIN_DIR}

${NEXUS_BIN_DIR}/svgvml3d.js: ${NEXUS_TOP}/../${B_THIS_DIR}/svgvml3d.js
	@echo [Copying ... $(notdir $@)]
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/svgvml3d.js ${NEXUS_BIN_DIR}

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -I${NEXUS_TOP}/../BSEAV/tools/bmemperf/include
CFLAGS += -I${LINUXDIR}/include
CFLAGS += -I${NEXUS_BIN_DIR}
CFLAGS += -Wimplicit-function-declaration -Wall -lm
ifeq ($(ANDROID_BUILD),y)
CFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DB_ANDROID_BUILD
endif

include $(NEXUS_TOP)/../$(B_THIS_DIR)/bsysperf.inc

ifdef WLAN_DRIVER
LDFLAGS += -L$(BWL_OBJ_DIR) -lbwl
CFLAGS += -DBWL_SUPPORT
CFLAGS += -I$(BWL_SRC_DIR)/include
BWL_LIB_A = $(BWL_OBJ_DIR)/libbwl.a

clean_bwl:
	$(MAKE) -C $(BWL_SRC_DIR) clean

$(BWL_OBJ_DIR)/libbwl.a: $(BWL_SRC_FILES)
	$(MAKE) -C $(BWL_SRC_DIR)

else
clean_bwl:
endif

CFLAGS += -DBMEMCONFIG_READ32_SUPPORTED
CFLAGS += -I$(NEXUS_TOP)/../magnum/portinginterface/xpt/src/core28nm

# Always build with debug
CFLAGS += -g
CFLAGS += -D_LINUX_TYPES_H

# Implicit rule for building local apps
${NEXUS_BIN_DIR}/bsysperf.cgi: $(BINDIR)/../bin/boa_server $(BWL_LIB_A) $(MAIN_OBJS) $(EXTRA_OBJS)
	@echo [Building ... $@]
	$(Q_)$(CC) -o $@ $(MAIN_OBJS) $(EXTRA_OBJS) $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)
	$(Q_)rm -f ${NEXUS_BIN_DIR}/bsysperf.d

${NEXUS_BIN_DIR}/bsysperf_server: $(SERVER_OBJS) $(EXTRA_OBJS) $(BWL_LIB_A)
	@echo [Building ... $@]
	$(Q_)$(CC) -c $(SERVER_OBJS) $(CFLAGS)
	$(Q_)$(CC) -o $@ $(SERVER_OBJS) $(EXTRA_OBJS) $(CFLAGS) $(LDFLAGS)
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.sh   ${NEXUS_BIN_DIR}
	$(Q_)rm -f ${NEXUS_BIN_DIR}/bsysperf_server.d

-include *.d

install: all
	@echo [Installing to ... ${NEXUS_BIN_DIR}]
	$(Q_)$(CP) -f ${APPS} ${NEXUS_BIN_DIR}

endif # B_REFSW_REAL_MAKE
