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
B_THIS_DIR:=BSEAV/tools/bpower
PHONY := all boa clean clean_apps install nexus_headers clean_boa bpower_regs.h
.PHONY: ${PHONY}
.NOTPARALLEL: apps boa

ifeq ($(LINUXDIR),)
LINUXDIR = $(LINUX)
endif

THEAPPS = \
	bpower.cgi

WEB_FILES = bpower.html bpower.js bpower.css bmemconfig.js bmemconfig.png

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(B_REFSW_REAL_MAKE),)
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/bpower

${PHONY} ${THEAPPS}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile ${MAKECMDGOALS}

${BINDIR}/exists:
	@echo [mkdir $(BINDIR)/exists]
	@mkdir -p $@

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

APPS = $(THEAPPS)
APPS ?= $(APP)

vpath %.c ${NEXUS_TOP}/../BSEAV/tools/bpower

all: apps

apps: $(APPS)

LDFLAGS += -lpthread
MAIN_OBJS = ${NEXUS_TOP}/../${B_THIS_DIR}/bpower.c ${NEXUS_TOP}/../${B_THIS_DIR}/bpower_utils.c ${NEXUS_TOP}/../${B_THIS_DIR}/bthermal.c
EXTRA_OBJS += ${NEXUS_TOP}/../BSEAV/tools/bmemperf/common/bmemperf_utils.c ${NEXUS_TOP}/../BSEAV/tools/bmemperf/common/bmemperf_lib.c
CLKGEN_PATH = ${NEXUS_TOP}/../magnum/basemodules/chp/src/common

# This cleans nexus and local apps
clean: clean_apps clean_boa clean_bpower_regs

boa:
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/opensource/boa
clean_boa:
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/opensource/boa clean

#
bpower_regs.h:
ifeq ($(findstring $(BCHP_CHIP),7429 7435 7425 7231 7346 7584),$(BCHP_CHIP))
	@echo "CLK_TREE_REG_INFO registerInfo[] = {{\"none\",\"none\",0,0,0,0 }};" > ${NEXUS_BIN_DIR}/bpower_regs.h;
else
	@test ! -f ${NEXUS_BIN_DIR}/bpower_regs.h || echo "[Exists ... bpower_regs.h]" > /dev/null;
	@test   -f ${NEXUS_BIN_DIR}/bpower_regs.h || echo "[Creating ... bpower_regs.h]";
	@test   -f ${NEXUS_BIN_DIR}/bpower_regs.h || awk -f ${NEXUS_TOP}/../${B_THIS_DIR}/bpower1.awk ${NEXUS_TOP}/../${B_THIS_DIR}/bpower_$(BCHP_CHIP)_$(BCHP_VER)_tree.txt | sort -u > ${NEXUS_BIN_DIR}/temp.c;
	@test   -f ${NEXUS_BIN_DIR}/bpower_regs.h || awk -f ${NEXUS_TOP}/../${B_THIS_DIR}/bpower2.awk ${NEXUS_BIN_DIR}/temp.c  > ${NEXUS_BIN_DIR}/bpower_regs.h;
##	@test   -f ${NEXUS_BIN_DIR}/bpower_regs.h || rm -f ${NEXUS_BIN_DIR}/temp.c;
	@test   -f ${NEXUS_BIN_DIR}/bpower_tree.h || cp ${NEXUS_TOP}/../${B_THIS_DIR}/bpower_$(BCHP_CHIP)_$(BCHP_VER)_tree.txt ${NEXUS_BIN_DIR}/bpower_tree.txt
endif

clean_bpower_regs:
	@-$(RM) ${NEXUS_BIN_DIR}/bpower_regs.h ${NEXUS_BIN_DIR}/bpower_tree.txt

clean_apps:
	@-$(RM) $(APPS) *.d *.o *.out
	@-$(RM) ${NEXUS_BIN_DIR}/bpower.html
	@-$(RM) ${NEXUS_BIN_DIR}/bpower.js
	@-$(RM) ${NEXUS_BIN_DIR}/bpower.css
	@-$(RM) ${NEXUS_BIN_DIR}/bpower.png
	@-$(RM) ${NEXUS_BIN_DIR}/bpower.cgi

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -I${NEXUS_TOP}/../BSEAV/tools/bmemperf/include
CFLAGS += -I${NEXUS_TOP}/../BSEAV/tools/bsysperf
CFLAGS += -I${LINUXDIR}/include
CFLAGS += -I${NEXUS_BIN_DIR}
CFLAGS += -Wimplicit-function-declaration -Wall -lm
ifeq ($(ANDROID_BUILD),y)
CFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DB_ANDROID_BUILD
endif

# Always build with debug
CFLAGS += -g
CFLAGS += -D_LINUX_TYPES_H
CFLAGS += -DBMEMCONFIG_READ32_SUPPORTED
CFLAGS += -I$(NEXUS_TOP)/../magnum/portinginterface/xpt/src/core28nm

include $(NEXUS_TOP)/../$(B_THIS_DIR)/../bsysperf/bsysperf.inc

# Implicit rule for building local apps
bpower.cgi: boa bpower_regs.h $(MAIN_OBJS) $(EXTRA_OBJS)
	@echo [Compiling ... $@]
	$(Q_)$(CC) -o ${NEXUS_BIN_DIR}/$@ $(MAIN_OBJS) $(EXTRA_OBJS) $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bpower.html  ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bpower.js    ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bpower.css   ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bpower.png   ${NEXUS_BIN_DIR}
#	$(Q_)rm -f ${NEXUS_BIN_DIR}/bpower_regs.h
	$(Q_)rm -f ${NEXUS_BIN_DIR}/bpower.d

-include *.d

install: all
	@echo [Installing to ... ${NEXUS_BIN_DIR}]
	$(Q_)$(CP) -f ${APPS} ${NEXUS_BIN_DIR}

endif # B_REFSW_REAL_MAKE
