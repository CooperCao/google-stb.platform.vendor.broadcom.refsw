#***************************************************************************
#*  Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#*
#*  This program is the proprietary software of Broadcom and/or its licensors,
#*  and may only be used, duplicated, modified or distributed pursuant to the terms and
#*  conditions of a separate, written license agreement executed between you and Broadcom
#*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#*  no license (express or implied), right to use, or waiver of any kind with respect to the
#*  Software, and Broadcom expressly reserves all rights in and to the Software and all
#*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#*
#*  Except as expressly set forth in the Authorized License,
#*
#*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#*  and to use this information only in connection with your use of Broadcom integrated circuit products.
#*
#*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#*  USE OR PERFORMANCE OF THE SOFTWARE.
#*
#*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#*  ANY LIMITED REMEDY.
#**************************************************************************

NEXUS_TOP ?= $(shell cd ../../../nexus; pwd)
MAGNUM ?= ${NEXUS_TOP}/../magnum
BSEAV ?= ${NEXUS_TOP}/../BSEAV

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
BINDIR := $(B_REFSW_OBJ_ROOT)/BSEAV/app/transcode

APPS = \
    get_encode_latency \
    sid_transcode \
    transcode_hls \
    video_conf \
    encode_audio_es \
    encode_video_es \
    transcode_mp4 \
    transcode_vpx \
    transcode_x \
    transcode_ts

# Enable optimization (this also will force compile to look for more obscurities, e.g. warnings)
CFLAGS += -Os


# add tshdrbuilder
include $(BSEAV)/lib/tshdrbuilder/tshdrbuilder.inc
MODULES += TSHDRBUILDER
# add bcmplayer
include $(BSEAV)/lib/bcmplayer/bcmplayer.inc
API_MODULES += B_NAV_PLAYER

include $(BSEAV)/lib/bfile/bfile.inc
MODULES += BFILE_STDIO
API_MODULES += BFILE
ifeq ($(MEDIA_AVI_SUPPORT),y)
include $(BSEAV)/lib/avi/bavi.inc
endif
ifeq ($(MEDIA_ASF_SUPPORT),y)
include $(BSEAV)/lib/asf/basf.inc
endif
ifeq ($(MEDIA_RMFF_SUPPORT),y)
include $(BSEAV)/lib/media/rmff/brmff.inc
endif
include $(BSEAV)/lib/utils/batom.inc
include $(BSEAV)/lib/media/bmedia.inc
MODULES += BMEDIA_PROBE

# add namevalue
CFLAGS += -I$(NEXUS_TOP)/utils

include $(MAGNUM)/commonutils/avc/bavc.inc
include $(MAGNUM)/commonutils/udp/budp.inc
include $(MAGNUM)/commonutils/vlc/bvlc.inc
# do not add BAVC to MODULES list. we only need the include path.
MODULES += BUDP BVLC
MAGNUM_MODULES:=

CFLAGS +=$(addprefix -I,$(sort $(foreach module, $(MODULES) ${API_MODULES}, $($(module)_INCLUDES))))
CFLAGS +=$(addprefix -I,$(BAVC_INCLUDES))
CFLAGS += $(addprefix -D,$(sort $(foreach module, $(MODULES) ${API_MODULES}, $($(module)_DEFINES))))
EXTRA_SRC += $(notdir $(sort $(foreach module, $(MODULES), $($(module)_SOURCES))))
vpath %c $(dir $(sort $(foreach module, $(MODULES), $($(module)_SOURCES))))
APP_BINS = $(addprefix $(BINDIR)/,$(APPS))

ifneq ($(B_NEXUS_API_BUILD_COMPLETED),y)
SUBLIBS += nexus
endif
SUBLIBS += oslib

.PHONY: nexus apps clean clean_apps

ifeq ($(APPS),)
    all: $(SUBLIBS)
else
    all: apps
endif

apps: $(APPS)
$(APP_BINS): $(SUBLIBS)

# This builds nexus
nexus:
	$(B_TRACE_RECIPE_BEGIN)
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/build $(NEXUS_BUILD_OPTIONS)
	$(B_TRACE_RECIPE_END)

clean_nexus:
	$(B_TRACE_RECIPE_BEGIN)
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/build $(NEXUS_BUILD_OPTIONS) clean
	$(B_TRACE_RECIPE_END)


# This cleans nexus and local apps
# Add the clean targets for each sub-library that we're using.
clean: clean_apps  $(foreach sublib,${SUBLIBS},clean_${sublib})

#
# OS lib option
#
B_LIB_TOP := $(NEXUS_TOP)/lib
include $(NEXUS_TOP)/lib/os/b_os_lib.inc
CFLAGS += $(addprefix -I,$(B_OS_LIB_PUBLIC_INCLUDES))
LDFLAGS += -lb_os
oslib:
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/os
clean_oslib:
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/os clean

# Assemble the Nexus flags into our CFLAGS and LDFLAGS
CFLAGS += $(NEXUS_CFLAGS) ${NEXUS_INTERNAL_CFLAGS} $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)
LDFLAGS += -lm

# Always build with debug
CFLAGS += -g

#
# Installation: copying app to DESTDIR
#
ifeq ($(DESTDIR),)
    DESTDIR = $(NEXUS_BIN_DIR)
    INSTALL_DIR := $(NEXUS_BIN_DIR)/unittests/$(shell basename `pwd`)
endif

ifeq ($(INSTALL_DIR),)
    INSTALL_DIR = $(DESTDIR)
endif

# cancel this implicit rule so we can separate compile & link stages
%: %.c

# Disable the built-in rules
.SUFFIXES:

define app_rule
$$(BINDIR)/$1: $$(BINDIR)/$1.o $$(addprefix $$(BINDIR)/,$$(patsubst %.c,%.o,$$(EXTRA_SRC)))
	@echo [Link... $$(notdir $$@)]
	@mkdir -p $$(dir $$@)
	$$(Q_)$$(CC) -o $$@ $$(filter %.o, $$^) $$(LDFLAGS)
	$$(Q_)mkdir -p $$(DESTDIR)
	$$(CP) $$@ $$(DESTDIR)

$1: $$(BINDIR)/$1
	@test 1
endef

$(foreach app, $(APPS), $(eval $(call app_rule,$(strip $(app)))))

$(BINDIR)/%.o: %.c
	@echo [Compile... $<]
	@mkdir -p $(dir $@)
	$(Q_)$(CC) -c -o $@ $^ $(CFLAGS)

.PHONY: install
install: all
