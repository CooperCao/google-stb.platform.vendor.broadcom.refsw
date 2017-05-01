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

NEXUS_TOP := $(shell cd ../../../nexus; pwd)
LIB=$(NEXUS_TOP)/../BSEAV/lib

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
BINDIR := $(B_REFSW_OBJ_ROOT)/BSEAV/app/thumbnail

# single app Makefile
APP = thumbnail

.PHONY: thumbnail nexus

all: $(APP)
$(APP): nexus

nexus:
	$(MAKE) -C $(NEXUS_TOP)/build

# This cleans nexus and app
clean: clean_app
	$(MAKE) -C $(NEXUS_TOP)/build clean

clean_app:
	$(RM) $(APP) $(OBJS)

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -g
LDFLAGS := $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)

OBJS += $(BINDIR)/main.o $(BINDIR)/thumbnail_demo.o

include $(NEXUS_TOP)/nxclient/apps/utils/nxutils.inc
OBJS += $(addprefix $(BINDIR)/,$(foreach src,$(NXUTILS_SINGLE_SOURCES), $(patsubst %.c,%.o,$(notdir $(src)))))
CFLAGS += $(addprefix -I,$(NXUTILS_INCLUDES))
vpath %.c $(foreach src,$(NXUTILS_SINGLE_SOURCES), $(dir $(src)))

thumbnail: $(BINDIR)/thumbnail
	$(CP) $(BINDIR)/thumbnail $(NEXUS_BIN_DIR)

$(BINDIR)/thumbnail: $(OBJS)
	$(Q_)echo [link $@]
	@mkdir -p $(dir $@)
	$(Q_)$(CC) -o $@ $(LDFLAGS) $(filter %.o,$^)

$(BINDIR)/%.o: %.c
	$(Q_)echo [compile $@]
	@mkdir -p $(dir $@)
	$(Q_)$(Q_)$(CC) -c -o $@ $(CFLAGS) $^

install: all
