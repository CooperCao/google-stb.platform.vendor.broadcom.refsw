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

B_THIS_DIR:=BSEAV/tools/bmemperf/server

ifdef B_REFSW_REAL_NEXUS_TOP
NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

BSEAV = $(shell cd ../../../;pwd)
magnum = $(shell cd ../../../../magnum;pwd)

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

APP_CFLAGS = \
	-DBCHP_CHIP=$(BCHP_CHIP) \
	-DBCHP_VER=BCHP_VER_$(BCHP_VER)\
	-I$(magnum)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)\
	-I../include -g

# This is the minimum needed to compile and link with Nexus
APP_CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
APP_CFLAGS += -I$(NEXUS_TOP)/../BSEAV/api/include
APP_CFLAGS += -I$(NEXUS_TOP)/../BSEAV/tools/bmemconfig
APP_CFLAGS += -I${NEXUS_TOP}/../${B_THIS_DIR}/${NEXUS_PLATFORM}
APP_CFLAGS += -I${NEXUS_TOP}/lib/os/include -I${NEXUS_TOP}/lib/os/include/linuxuser
APP_CFLAGS += -L${BINDIR}
APP_CFLAGS += -Wall -lpthread -lm -Wno-long-long
APP_CFLAGS += -I${NEXUS_TOP}/../$(B_THIS_DIR)
APP_CFLAGS += -DUSE_BOXMODES

ifeq ($(ANDROID_BUILD),y)
APP_CFLAGS += -DB_ANDROID_BUILD
endif

include $(NEXUS_TOP)/../$(B_THIS_DIR)/../../bsysperf/bsysperf.inc
APP_CFLAGS += $(CFLAGS)

# Added for aarch64-linux-gcc
APP_CFLAGS += -D_LINUX_TYPES_H
APP_CFLAGS += -I$(NEXUS_TOP)/../magnum/portinginterface/xpt/src/core28nm

install: all
	@echo [Installing .. $< for $(B_THIS_DIR)]
	@test -d "$(BINDIR)" || mkdir -p $(BINDIR)
	@cp bmemperf.sh ${BINDIR}/
	@chmod a+x ${BINDIR}/bmemperf.sh

clean:
	@rm -f $(BINDIR)/bmemperf*
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/opensource/boa clean
ifeq ($(ANDROID_BUILD),y)
	@rm -f $(BINDIR)/device_node.cfg
endif

all:	boa bmemperf_server

boa:
	@$(MAKE) -C $(NEXUS_TOP)/../BSEAV/opensource/boa

### the awk command returns some type of error code to the Makefile; added echo at the end to signal to Makefile that awk command was successful
$(BINDIR)/bmemperf_info.c :
	@test -d "$(BINDIR)" || mkdir -p $(BINDIR)
ifeq ($(findstring $(BCHP_CHIP),7429 7435 7425 7231 7346 7584),$(BCHP_CHIP))
    ### memc_0_default_config.h files was extracted from 74xxb0_single_encode_RefSW_RTS.docx document which was found on 74xx Twiki page
	@echo [Parsing for 40nm client info for chip ../$(BCHP_CHIP)/$(BCHP_VER_LOWER) and platform $(NEXUS_PLATFORM)];
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h  || awk -f ../include/bmemperf_info_pre.awk Makefile > $(BINDIR)/bmemperf_info.c;
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h  || awk -f ../include/bmemperf_info_40nm.awk ../$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h  >> $(BINDIR)/bmemperf_info.c || echo [Awk done];
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h  || awk -f ../include/bmemperf_info_post.awk $(BINDIR)/bmemperf_info.c > $(BINDIR)/bmemperf_info.tmp || echo [Awk done];
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h  || cat $(BINDIR)/bmemperf_info.tmp >> $(BINDIR)/bmemperf_info.c;
	@test ! -f ${NEXUS_TOP}/../${B_THIS_DIR}/../$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h  || rm $(BINDIR)/bmemperf_info.tmp;
	@test   -f ${NEXUS_TOP}/../${B_THIS_DIR}/../$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h  || printf "\nERROR ... file ${NEXUS_TOP}/../${B_THIS_DIR}/../$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h does not exist\n\n";
	@test   -f ${NEXUS_TOP}/../${B_THIS_DIR}/../$(BCHP_CHIP)/$(BCHP_VER_LOWER)/memc_0_default_config.h  || exit 1;
else
	@echo [Parsing for 28nm client info for chip ../$(BCHP_CHIP)/$(BCHP_VER_LOWER) and platform $(NEXUS_PLATFORM)]
	@awk -f ../include/bmemperf_info_pre.awk Makefile > $(BINDIR)/bmemperf_info.c
	@$(foreach myfile,$(BBOX_MEMC_FILES), awk -f ../include/bmemperf_info.awk $(myfile) >> $(BINDIR)/bmemperf_info.c || echo [Awk $(myfile) done] ; ) >/dev/null
	@awk -f ../include/bmemperf_info_post.awk $(BINDIR)/bmemperf_info.c > $(BINDIR)/bmemperf_info.tmp || echo [Awk done];
	@cat $(BINDIR)/bmemperf_info.tmp >> $(BINDIR)/bmemperf_info.c
	@rm $(BINDIR)/bmemperf_info.tmp
endif

bmemperf_server: bmemperf_server.c bmemperf_powerprobe.c bmemperf_powerprobe.h ../common/bmemperf_utils.c ../common/bmemperf_api.c ../common/bmemperf.c ../common/bmemperf_lib.c $(BINDIR)/bmemperf_info.c
	@echo [Compiling ... $@]
	@$(CC) $(APP_CFLAGS) -o $(BINDIR)/bmemperf_server bmemperf_server.c bmemperf_powerprobe.c ../common/bmemperf_api.c ../common/bmemperf.c ../common/bmemperf_utils.c ../common/bmemperf_lib.c $(BINDIR)/bmemperf_info.c
### if the server built successfully, delete temporary info file; otherwise leave it to help with debugging
	@test ! -f $(BINDIR)/bmemperf_server || rm -f $(BINDIR)/bmemperf_info.c
ifeq ($(ANDROID_BUILD),y)
	@cp device_node.cfg ${BINDIR}
endif
