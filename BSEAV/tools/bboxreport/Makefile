###############################################################################
# Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
###############################################################################
# Nexus examples
B_THIS_DIR:=BSEAV/tools/bboxreport
PHONY := all boa clean clean_apps nexus_headers clean_boa
.PHONY: ${PHONY}
.NOTPARALLEL: apps boa

ifeq ($(LINUXDIR),)
LINUXDIR = $(LINUX)
endif

THEAPPS = \
	bboxreport

WEB_FILES = bboxreport.html bboxreport.js bboxreport.png bboxreport.css

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(B_REFSW_REAL_MAKE),)
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/bboxreport

${PHONY} ${THEAPPS}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile ${MAKECMDGOALS}

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/bboxreport

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
include ${NEXUS_TOP}/../BSEAV/tools/bmemconfig/bmemconfig.inc

APPS = $(THEAPPS)
APPS ?= $(APP)

vpath %.c ${NEXUS_TOP}/../BSEAV/tools/bboxreport

all: $(NEXUS_BIN_DIR)/bboxreport_box_info.c $(NEXUS_BIN_DIR)/libnexus.so apps

apps: $(APPS)

$(NEXUS_BIN_DIR)/libnexus.so:
	$(MAKE) -C $(NEXUS_TOP)/build

LDFLAGS += -lpthread -L${NEXUS_BIN_DIR}
EXTRA_SRCS += ${NEXUS_TOP}/../BSEAV/tools/bmemperf/common/bmemperf_utils.c ${NEXUS_TOP}/../BSEAV/tools/bmemperf/common/bmemperf_lib.c $(NEXUS_BIN_DIR)/bboxreport_box_info.c
EXTRA_SRCS += ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemconfig/${NEXUS_PLATFORM}/boxmodes.c ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemconfig/memusage.c

# This cleans nexus and local apps
clean: clean_apps clean_boa

boa:
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/opensource/boa
clean_boa:
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/opensource/boa clean

clean_apps:
	@-$(RM) $(APPS)
	@-$(RM) ${NEXUS_BIN_DIR}/bboxreport.html
	@-$(RM) ${NEXUS_BIN_DIR}/bboxreport.js
	@-$(RM) ${NEXUS_BIN_DIR}/bboxreport.png
	@-$(RM) ${NEXUS_BIN_DIR}/bboxreport.css
	@-$(RM) ${NEXUS_BIN_DIR}/bboxreport.cgi
	@-$(RM) $(NEXUS_BIN_DIR)/bboxreport_box_info.c
	@-$(RM) -rf $(BINDIR)

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -I${NEXUS_TOP}/../BSEAV/tools/bmemperf/include
CFLAGS += -I${NEXUS_TOP}/../BSEAV/tools/bmemconfig
CFLAGS += -I${LINUXDIR}/include
CFLAGS += -I${NEXUS_BIN_DIR}
CFLAGS += -I$(NEXUS_TOP)/../BSEAV/lib/media
CFLAGS += -I${NEXUS_TOP}/../${B_THIS_DIR}/${NEXUS_PLATFORM} -Wimplicit-function-declaration -lnexus
CFLAGS += -I$(NEXUS_TOP)/utils
CFLAGS += -Wimplicit-function-declaration -Wall -lm
ifeq ($(ANDROID_BUILD),y)
CFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
endif
CFLAGS += -DBMEMCONFIG_READ32_SUPPORTED
CFLAGS += -I$(NEXUS_TOP)/../magnum/portinginterface/xpt/src/core28nm

include $(NEXUS_TOP)/../$(B_THIS_DIR)/../bsysperf/bsysperf.inc

# Always build with debug
CFLAGS += -g
CFLAGS += -D_LINUX_TYPES_H

AWKDIR := ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemconfig

# Implicit rule for building local apps
bboxreport: boa ${NEXUS_TOP}/../${B_THIS_DIR}/bboxreport.c ${NEXUS_TOP}/../${B_THIS_DIR}/bboxreport_svgplot.c ${NEXUS_TOP}/../${B_THIS_DIR}/bboxreport_svgplot.h
	@echo [Compiling ... $<]
	$(Q_)$(CC) -o $@ ${NEXUS_TOP}/../${B_THIS_DIR}/bboxreport.c ${NEXUS_TOP}/../${B_THIS_DIR}/bboxreport_svgplot.c $(EXTRA_SRCS) $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)
	$(Q_)echo [Copy $@ to ${NEXUS_BIN_DIR}]
	$(if $(filter $@,bboxreport),$(Q_)$(CP) -f $@ $(NEXUS_BIN_DIR)/$@.cgi,$(Q_)$(CP) -f $@ $(NEXUS_BIN_DIR))
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bboxreport.html  ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bboxreport.js    ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bboxreport.png   ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bboxreport.css   ${NEXUS_BIN_DIR}

### the awk command returns some type of error code to the Makefile; added echo at the end to signal to Makefile that awk command was successful
$(NEXUS_BIN_DIR)/bboxreport_box_info.c:
	@test -d "$(NEXUS_BIN_DIR)" || mkdir -p $(NEXUS_BIN_DIR)
	@echo [Create... $@ for CHIP $(BCHP_CHIP)]
ifeq ($(findstring $(BCHP_CHIP),7429 7435 7425 7231 7346 7584),$(BCHP_CHIP))
        ### memc_0_default_config.h files was extracted from 74xxb0_single_encode_RefSW_RTS.docx document which was found on 74xx Twiki page
	@echo [Parsing for 40nm client info for chip ../$(BCHP_CHIP)/$(BCHP_VER_LOWER) and platform $(NEXUS_PLATFORM)];
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || awk -f ${AWKDIR}/bmemconfig_box_info_pre.awk ${AWKDIR}/Makefile > $(NEXUS_BIN_DIR)/bboxreport_box_info.c;
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || awk -f ${AWKDIR}/bmemconfig_box_info_40nm.awk ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h >> $(NEXUS_BIN_DIR)/bboxreport_box_info.c || echo [Awk done];
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || awk -f ${AWKDIR}/bmemconfig_box_info_post.awk $(NEXUS_BIN_DIR)/bboxreport_box_info.c > $(NEXUS_BIN_DIR)/bboxreport_box_info.tmp || echo [Awk done];
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || cat $(NEXUS_BIN_DIR)/bboxreport_box_info.tmp >> $(NEXUS_BIN_DIR)/bboxreport_box_info.c;
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || rm $(NEXUS_BIN_DIR)/bboxreport_box_info.tmp;
	@test   -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || printf "\nERROR ... file ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h does not exist\n\n";
	@test   -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || exit 1;
else
	@echo [Parsing for 28nm client info for chip ../$(BCHP_CHIP)/$(BCHP_VER_LOWER) and platform $(NEXUS_PLATFORM)]
	@awk -f ${AWKDIR}/bmemconfig_box_info_pre.awk ${AWKDIR}/Makefile > $(NEXUS_BIN_DIR)/bboxreport_box_info.c
###  When platforms have box mode 0, the pre.awk script above will output the DDR and SCB speeds. For platforms
###  that do NOT have box mode 0, we have to complete the initialization string that was started in the pre.awk
###  script above.
	@echo ",\"unknown DDR SCB\",\"unknown\",0,0}" >> $(NEXUS_BIN_DIR)/bboxreport_box_info.c
	@$(foreach myfile,$(BBOX_MEMC_FILES), awk -f ${AWKDIR}/bmemconfig_box_info.awk $(myfile) >> $(NEXUS_BIN_DIR)/bboxreport_box_info.c || echo [Awk $(myfile) done] ; ) >/dev/null
	@awk -f ${AWKDIR}/bmemconfig_box_info_post.awk $(NEXUS_BIN_DIR)/bboxreport_box_info.c > $(NEXUS_BIN_DIR)/bboxreport_box_info.tmp || echo [Awk done];
	@cat $(NEXUS_BIN_DIR)/bboxreport_box_info.tmp >> $(NEXUS_BIN_DIR)/bboxreport_box_info.c
	@rm $(NEXUS_BIN_DIR)/bboxreport_box_info.tmp
endif

-include *.d

endif # B_REFSW_REAL_MAKE
