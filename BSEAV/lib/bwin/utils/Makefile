############################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

export FREETYPE_SUPPORT=y
export PNG_SUPPORT=n

.PHONY: all clean nexus clean_nexus clean_apps install libs clean_libs

APPS = \
	buildfont

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif
NEXUS_TOP := $(shell cd ../../../../nexus;pwd)
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
BINDIR := $(B_REFSW_OBJ_ROOT)/BSEAV/lib/bwin/utils

BSEAV := $(NEXUS_TOP)/../BSEAV
BWIN=$(BSEAV)/lib/bwin
include $(BWIN)/include/bwin.mak
CFLAGS += $(BWIN_CFLAGS) -DBWIN_SUPPORT
LDFLAGS += $(BWIN_LDFLAGS)

all: $(APPS)

include $(BSEAV)/lib/bwin/build/b_bwin_targets.inc
B_BWIN_OTHER_BUILDS += b_bwin_target_build
B_BWIN_OTHER_CLEANS += b_bwin_target_clean

clean: clean_apps clean_libs

nexus:
	$(MAKE) -C $(NEXUS_TOP)/build
clean_nexus:
	$(MAKE) -C $(NEXUS_TOP)/build clean

libs: nexus $(B_BWIN_OTHER_BUILDS)

clean_libs: clean_nexus $(B_BWIN_OTHER_CLEANS)

clean_apps:
	-$(RM) -rf $(BINDIR)

CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -g
LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES) -lm

$(BINDIR)/%.o: %.c
	@echo [Compile... $(notdir $<)]
	@mkdir -p $(dir $@)
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS) ${CDEP_FLAG}

$(BINDIR)/buildfont: $(BINDIR)/buildfont.o libs
	@echo [Install... $@ to $(NEXUS_BIN_DIR)]
	@mkdir -p $(dir $@)
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o %.a, $^) $(LDFLAGS)
	cp -f $@ $(NEXUS_BIN_DIR)

.PHONY: buildfont
buildfont: $(BINDIR)/buildfont

-include %(BINDIR)/*.d

install: all
