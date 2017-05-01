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

B_THIS_DIR:=BSEAV/tools/baspmon

ifdef B_REFSW_REAL_NEXUS_TOP
NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

BSEAV = $(shell cd ../../;pwd)
magnum = $(shell cd ../../../magnum;pwd)

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
include ${NEXUS_TOP}/../BSEAV/tools/bmemconfig/bmemconfig.inc

ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mipsel-linux
endif

ifeq ($(filter ${B_REFSW_ARCH}, mipsel-linux mipsel-uclibc mipsel-linux-uclibc mipsel-linux-android), ${B_REFSW_ARCH})
B_REFSW_CROSS_COMPILE ?= mipsel-linux-
else
ifeq ($(filter ${B_REFSW_ARCH}, mips-linux mips-uclibc mips-linux-uclibc mips-linux-android), ${B_REFSW_ARCH})
B_REFSW_CROSS_COMPILE ?= mips-linux-
else
ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), ${B_REFSW_ARCH})
B_REFSW_CROSS_COMPILE ?= arm-linux-
endif
endif
endif

ifeq ($(BINDIR),)
ifeq ($(B_REFSW_OBJ_DIR),)
BINDIR = $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/nexus/bin
else
BINDIR = $(NEXUS_TOP)/../$(B_REFSW_OBJ_DIR)/nexus/bin
endif
endif

AWKDIR = $(BSEAV)/tools/bmemconfig  ### this is where the awk scripts are for bmemconfig
AWKDIR2 = $(BSEAV)/tools/bmemperf/include ### this is where the awk scripts are for bmemperf
OBJDIR = $(BINDIR)/../baspmon
APP_CFLAGS = \
	-DBCHP_CHIP=$(BCHP_CHIP) \
	-DBCHP_VER=BCHP_VER_$(BCHP_VER)\
	-I$(magnum)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)\
	-I../include

# This is the minimum needed to compile and link with Nexus
APP_CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
APP_CFLAGS += -I$(NEXUS_TOP)/../BSEAV/api/include
APP_CFLAGS += -I${NEXUS_TOP}/../${B_THIS_DIR}/${NEXUS_PLATFORM}
APP_CFLAGS += -I${NEXUS_TOP}/lib/os/include -I${NEXUS_TOP}/lib/os/include/linuxuser
APP_CFLAGS += -L${BINDIR}
APP_CFLAGS += -Wall -lpthread -lm -Wno-long-long
APP_CFLAGS += -I${NEXUS_TOP}/../$(B_THIS_DIR)
APP_CFLAGS += -DUSE_BOXMODES

# Added for aarch64-linux-gcc
APP_CFLAGS += -D_LINUX_TYPES_H

APP_CFLAGS += -DBMEMCONFIG_READ32_SUPPORTED
APP_CFLAGS += -I${NEXUS_TOP}/../BSEAV/tools/bmemperf/include -I${NEXUS_TOP}/../BSEAV/tools/bmemconfig/
NEXUS_BIN_DIR = $(BINDIR)
EXTRA_OBJS += $(OBJDIR)/bmemconfig_box_info.c $(OBJDIR)/bmemperf_info.c
APP_CFLAGS += -I$(NEXUS_TOP)/../magnum/portinginterface/xpt/src/core28nm
ASP_INC = $(NEXUS_TOP)/../magnum/basemodules/asp/include/debug
APP_CFLAGS += -I$(ASP_INC)

ifeq ($(findstring $(BCHP_CHIP),7278 ),$(BCHP_CHIP))
PATH_TO_FILE = $(NEXUS_TOP)/../magnum/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)/bchp_asp_mcpb_ch0.h
all: boa $(BINDIR)/baspmon.cgi
	@echo [Installing .. $< for $(B_THIS_DIR)]
	@test -d "$(BINDIR)" || ( echo mkdir $(BINDIR) && mkdir -p $(BINDIR))
	@cp -p baspmon.html ${BINDIR}/
	@cp -p baspmon.js ${BINDIR}/
	@cp -p baspmon.css ${BINDIR}/
	@cp -p baspmon.cfg ${BINDIR}/
	@cp -p baspmon*.png ${BINDIR}/
	@cp -p BCM$(NEXUS_PLATFORM).json ${BINDIR}/

ifneq ("$(wildcard $(PATH_TO_FILE))","")
FILE_EXISTS = 1
APP_CFLAGS += -DASP_SUPPORT
endif
else
all:
	@echo [This tool is not supported for the $(NEXUS_PLATFORM)]

endif

clean:
	@rm -f $(BINDIR)/baspmon.html $(BINDIR)/baspmon.js $(BINDIR)/baspmon.css $(BINDIR)/baspmon.cfg $(BINDIR)/baspmon*.png $(BINDIR)/baspmon.cgi $(BINDIR)/BCM$(NEXUS_PLATFORM).json
	@rm -rf $(OBJDIR)


boa:
	@$(MAKE) -C $(NEXUS_TOP)/../BSEAV/opensource/boa

### the awk command returns some type of error code to the Makefile; added echo at the end to signal to Makefile that awk command was successful
$(OBJDIR)/bmemperf_info.c:
	@echo [Create... $(notdir $@) for CHIP $(BCHP_CHIP)]
	@test -d "$(BINDIR)" || ( echo mkdir $(BINDIR) && mkdir -p $(BINDIR))
ifeq ($(findstring $(BCHP_CHIP),7429 7435 7425 7231 7346 7584),$(BCHP_CHIP))
    ### memc_0_default_config.h files was extracted from 74xxb0_single_encode_RefSW_RTS.docx document which was found on 74xx Twiki page
	@echo [Parsing for 40nm client info for chip ../$(BCHP_CHIP)/$(BCHP_VER_LOWER) and platform $(NEXUS_PLATFORM)]
	@awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemperf/include/bmemperf_info_pre.awk Makefile > $(OBJDIR)/bmemperf_info.c
	@awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemperf/include/bmemperf_info_40nm.awk ../$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h  >> $(OBJDIR)/bmemperf_info.c || echo [Awk done]
	@awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemperf/include/bmemperf_info_post.awk $(OBJDIR)/bmemperf_info.c > $(OBJDIR)/bmemperf_info.tmp || echo [Awk done];
	@cat $(OBJDIR)/bmemperf_info.tmp >> $(OBJDIR)/bmemperf_info.c
else
	@echo [Parsing for 28nm client info for chip $(BCHP_CHIP)/$(BCHP_VER_LOWER) and platform $(NEXUS_PLATFORM)]
	@echo awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemperf/include/bmemperf_info_pre.awk Makefile > $(OBJDIR)/bmemperf_info.c
	@awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemperf/include/bmemperf_info_pre.awk Makefile > $(OBJDIR)/bmemperf_info.c
	@$(foreach myfile,$(BBOX_MEMC_FILES), awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemperf/include/bmemperf_info.awk $(myfile) >> $(OBJDIR)/bmemperf_info.c || echo [Awk $(myfile) done] ; ) >/dev/null
	@awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemperf/include/bmemperf_info_post.awk $(OBJDIR)/bmemperf_info.c > $(OBJDIR)/bmemperf_info.tmp || echo [Awk done];
	@cat $(OBJDIR)/bmemperf_info.tmp >> $(OBJDIR)/bmemperf_info.c
	@rm $(OBJDIR)/bmemperf_info.tmp
endif


### the awk command returns some type of error code to the Makefile; added echo at the end to signal to Makefile that awk command was successful
$(OBJDIR)/bmemconfig_box_info.c:
	@test -d "$(BINDIR)" || ( echo mkdir $(BINDIR) && mkdir -p $(BINDIR))
	@test -d "$(OBJDIR)" || ( echo mkdir $(OBJDIR) && mkdir -p $(OBJDIR))
	@echo [Create... $(notdir $@) for CHIP $(BCHP_CHIP)]
ifeq ($(findstring $(BCHP_CHIP),7429 7435 7425 7231 7346 7584),$(BCHP_CHIP))
        ### memc_0_default_config.h files was extracted from 74xxb0_single_encode_RefSW_RTS.docx document which was found on 74xx Twiki page
	@echo [Parse for 40nm client info for chip ../$(BCHP_CHIP)/$(BCHP_VER_LOWER) and platform $(NEXUS_PLATFORM)];
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemconfig/bmemconfig_box_info_pre.awk ${NEXUS_TOP}/../${B_THIS_DIR}/Makefile > $(OBJDIR)/bmemconfig_box_info.c || echo [Awk done];
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemconfig/bmemconfig_box_info_40nm.awk ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h >> $(OBJDIR)/bmemconfig_box_info.c || echo [Awk done];
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemconfig/bmemconfig_box_info_post.awk $(OBJDIR)/bmemconfig_box_info.c > $(OBJDIR)/bmemconfig_box_info.tmp || echo [Awk done];
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || cat $(OBJDIR)/bmemconfig_box_info.tmp >> $(OBJDIR)/bmemconfig_box_info.c;
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || rm $(OBJDIR)/bmemconfig_box_info.tmp;
	@test   -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || printf "\nERROR ... file ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h does not exist\n\n";
	@test   -f ${NEXUS_TOP}/../${B_THIS_DIR}/../bmemperf/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h || exit 1;
else
	@echo [Parse for 28nm client info for chip ../$(BCHP_CHIP)/$(BCHP_VER_LOWER) and platform $(NEXUS_PLATFORM)]
	@awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemconfig/bmemconfig_box_info_pre.awk ${NEXUS_TOP}/../${B_THIS_DIR}/Makefile > $(OBJDIR)/bmemconfig_box_info.c
###  When platforms have box mode 0, the pre.awk script above will output the DDR and SCB speeds. For platforms
###  that do NOT have box mode 0, we have to complete the initialization string that was started in the pre.awk
###  script above.
	@echo ",\"unknown DDR SCB\",\"unknown\",0,0}" >> $(OBJDIR)/bmemconfig_box_info.c
	@$(foreach myfile,$(BBOX_MEMC_FILES), awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemconfig/bmemconfig_box_info.awk $(myfile) >> $(OBJDIR)/bmemconfig_box_info.c || echo [Awk $(myfile) done] ; ) >/dev/null
	@awk -f ${NEXUS_TOP}/../BSEAV/tools/bmemconfig/bmemconfig_box_info_post.awk $(OBJDIR)/bmemconfig_box_info.c > $(OBJDIR)/bmemconfig_box_info.tmp || echo [Awk done];
	@cat $(OBJDIR)/bmemconfig_box_info.tmp >> $(OBJDIR)/bmemconfig_box_info.c
	@rm $(OBJDIR)/bmemconfig_box_info.tmp
endif

$(BINDIR)/bmemperf_server: $(NEXUS_TOP)/../BSEAV/tools/bmemperf/server/bmemperf_server.c
	@echo [Compiling ... $(notdir $@)]
	@echo $(MAKE) -C $(NEXUS_TOP)/../BSEAV/tools/bmemperf/server $(CFLAGS)
	@$(MAKE) -C $(NEXUS_TOP)/../BSEAV/tools/bmemperf/server $(CFLAGS) > /dev/null

$(BINDIR)/baspmon.cgi: $(BINDIR)/bmemperf_server $(OBJDIR)/bmemconfig_box_info.c $(OBJDIR)/bmemperf_info.c baspmon.c ../bmemperf/common/bmemperf_utils.c ../bmemperf/common/bmemperf_lib.c
	@echo [Compiling ... $@]
	@$(CC) $(APP_CFLAGS) -o $(BINDIR)/baspmon.cgi baspmon.c ../bmemperf/common/bmemperf_utils.c ../bmemperf/common/bmemperf_lib.c ../bmemperf/common/bmemperf_api.c ../bmemperf/common/bmemperf.c $(EXTRA_OBJS)
	@$(RM) $(EXTRA_OBJS)
