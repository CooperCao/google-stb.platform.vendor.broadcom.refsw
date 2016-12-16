###############################################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
# This program is the proprietary software of Broadcom and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################
# Nexus examples
B_THIS_DIR:=BSEAV/tools/brbviewer
PHONY := all boa clean clean_apps nexus_headers clean_boa
.PHONY: ${PHONY}
.NOTPARALLEL: apps boa

ifeq ($(LINUXDIR),)
LINUXDIR = $(LINUX)
endif

THEAPPS = \
	brbviewer

WEB_FILES = brbviewer.html brbviewer.js brbviewer2.js brbviewer.png brbviewer.css

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(B_REFSW_REAL_MAKE),)
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/brbviewer

${PHONY} ${THEAPPS}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile ${MAKECMDGOALS}

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/brbviewer

include $(NEXUS_TOP)/examples/Makefile_prologue.inc
include ${NEXUS_TOP}/../BSEAV/tools/bmemconfig/bmemconfig.inc

APPS = $(THEAPPS)
APPS ?= $(APP)

vpath %.c ${NEXUS_TOP}/../BSEAV/tools/brbviewer

ifeq ($(findstring $(BCHP_CHIP),7429 7425 7231 7346 7435 7584),$(BCHP_CHIP))
all:
	@echo [The brbviewer tool is not designed to work on the $(BCHP_CHIP) chip ]
else
all: $(NEXUS_BIN_DIR)/libnexus.so apps
endif

apps: $(APPS)

$(NEXUS_BIN_DIR)/libnexus.so:
	$(MAKE) -C $(NEXUS_TOP)/build

LDFLAGS += -lpthread -L${NEXUS_BIN_DIR}
EXTRA_SRCS += ${NEXUS_TOP}/../BSEAV/tools/bmemperf/common/bmemperf_utils.c ${NEXUS_TOP}/../BSEAV/tools/bmemperf/common/bmemperf_lib.c
EXTRA_SRCS += ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemconfig/${NEXUS_PLATFORM}/boxmodes.c ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemconfig/memusage.c
EXTRA_SRCS += ${NEXUS_TOP}/../BSEAV/lib/utils/bbase64.c

PLIB_OUT_DIR = $(NEXUS_BIN_DIR)/.plib

# This cleans nexus and local apps
clean: clean_apps clean_boa

boa:
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/boa
clean_boa:
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/boa clean

clean_apps:
	@-$(RM) $(APPS)
	@-$(RM) ${NEXUS_BIN_DIR}/brbviewer.html
	@-$(RM) ${NEXUS_BIN_DIR}/brbviewer.js
	@-$(RM) ${NEXUS_BIN_DIR}/brbviewer2.js
	@-$(RM) ${NEXUS_BIN_DIR}/brbviewer.png
	@-$(RM) ${NEXUS_BIN_DIR}/brbviewer.css
	@-$(RM) ${NEXUS_BIN_DIR}/brbviewer.cgi
	@-$(RM) -rf $(BINDIR)
	@-$(RM) -rf $(PLIB_OUT_DIR)

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -I${NEXUS_TOP}/../BSEAV/tools/bmemperf/include
CFLAGS += -I${NEXUS_TOP}/../BSEAV/tools/bmemconfig
CFLAGS += -I${LINUXDIR}/include
CFLAGS += -I${NEXUS_BIN_DIR}
CFLAGS += -I$(NEXUS_TOP)/../BSEAV/lib/media
CFLAGS += -I$(NEXUS_TOP)/../BSEAV/lib/utils
CFLAGS += -I${NEXUS_TOP}/../${B_THIS_DIR}/${NEXUS_PLATFORM} -Wimplicit-function-declaration -lnexus
CFLAGS += -I$(NEXUS_TOP)/utils
CFLAGS += -Wimplicit-function-declaration -Wall -lm
ifeq ($(ANDROID_BUILD),y)
CFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
endif
CFLAGS += -DBMEMCONFIG_READ32_SUPPORTED
CFLAGS += -I${BINDIR}

# Always build with debug
CFLAGS += -g
CFLAGS += -D_LINUX_TYPES_ -D_LINUX_TYPES_H

AWKDIR := ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemconfig

BRBVIEWER_SRCS = ${NEXUS_TOP}/../${B_THIS_DIR}/brbviewer.c ${BINDIR}/brbviewer_regs_autogen.h

javascript_files: ${NEXUS_TOP}/../${B_THIS_DIR}/bvnview2.js
	@if [ ! -d $(PLIB_OUT_DIR) ]; then \
		echo [Create... $(PLIB_OUT_DIR) ] ; \
		test -d "$(PLIB_OUT_DIR)" || mkdir -p $(PLIB_OUT_DIR) ; \
		test -d "$(PLIB_OUT_DIR)/js" || mkdir -p $(PLIB_OUT_DIR)/js ; \
		test -d "$(PLIB_OUT_DIR)/js/3party" || mkdir -p $(PLIB_OUT_DIR)/js/3party ; \
		test -d "$(PLIB_OUT_DIR)/js/bcm" || mkdir -p $(PLIB_OUT_DIR)/js/bcm ; \
		$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/saXMLUtils.js   ${PLIB_OUT_DIR}/js/3party ; \
		$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/base64.js       ${PLIB_OUT_DIR}/js/3party ; \
		$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/base64-brcm.js  ${PLIB_OUT_DIR}/js/3party ; \
		$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bvnview.js      ${PLIB_OUT_DIR}/js/bcm ; \
		$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bvnview2.js     ${PLIB_OUT_DIR}/js/bcm ; \
		$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/soap.js         ${PLIB_OUT_DIR}/js/bcm ; \
		$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/example-data.js ${PLIB_OUT_DIR}/js/bcm ; \
	fi ;

${BINDIR}/brbviewer_regs_autogen.h:
	@echo [Create ... brbviewer_regs_autogen.tmp]
	@awk -f ${NEXUS_TOP}/../${B_THIS_DIR}/brbviewer_pass1.awk $(NEXUS_TOP)/../magnum/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)/bchp_vnet_f.h > ${BINDIR}/brbviewer_regs_autogen.tmp
	@awk -f ${NEXUS_TOP}/../${B_THIS_DIR}/brbviewer_pass1.awk $(NEXUS_TOP)/../magnum/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)/bchp_vnet_b.h >> ${BINDIR}/brbviewer_regs_autogen.tmp
	@echo [Create ... brbviewer_regs_autogen.h]
	@awk -f ${NEXUS_TOP}/../${B_THIS_DIR}/brbviewer_pass2.awk ${BINDIR}/brbviewer_regs_autogen.tmp > ${BINDIR}/brbviewer_regs_autogen.h

brbviewer: boa ${BINDIR}/brbviewer_regs_autogen.h javascript_files ${BRBVIEWER_SRCS}
	@echo [Compile ... $@]
	$(Q_)$(CC) -o $@ ${BRBVIEWER_SRCS} $(EXTRA_SRCS) $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)
	$(Q_)echo [Copy $@ to ${NEXUS_BIN_DIR}]
	$(if $(filter $@,brbviewer),$(Q_)$(CP) -f $@ $(NEXUS_BIN_DIR)/$@.cgi,$(Q_)$(CP) -f $@ $(NEXUS_BIN_DIR))
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/brbviewer.html  ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/brbviewer.png   ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/brbviewer.js    ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/brbviewer.css   ${NEXUS_BIN_DIR}

-include *.d

endif # B_REFSW_REAL_MAKE
