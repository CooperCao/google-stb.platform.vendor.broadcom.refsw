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

.PHONY: all nexus api clean clean_apps install

APPS = \
	message

DECODE_APPS = \
	playback \
	record \
	decode \
	audio_decode \
	audio_playback \
	video

DECODE_SRCS += cmdline_args.c namevalue.c fileio_custom.c frontend.c decoder_bitrate.c

APPS += $(DECODE_APPS)

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

NEXUS_TOP := $(shell cd ..;pwd)

ifeq ($(NEXUS_PREBUILT_BINARY),y)
# do "make api" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
include $(NEXUS_BIN_DIR)/include/platform_app.inc
Q_ ?= @
else
# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif
BINDIR := $(B_REFSW_OBJ_ROOT)/nexus/utils/$(B_REFSW_ARCH)

all: $(APPS)

LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)

ifeq ($(B_REFSW_ANDROID),y)
include $(NEXUS_TOP)/build/android.inc
LDFLAGS := $(ANDROID_LDFLAGS)
endif

# This builds the Nexus api
nexus api:
	$(MAKE) -C $(NEXUS_TOP)/build

# This cleans nexus and local apps
clean: clean_apps
	$(MAKE) -C $(NEXUS_TOP)/build clean

clean_apps:
	-$(RM) -r $(addprefix $(BINDIR)/,$(APPS)) $(BINDIR)/obj

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS)  $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
# This enables extra goodies (like compiler warnings)
CFLAGS += ${NEXUS_INTERNAL_CFLAGS}

LDFLAGS += ${NEXUS_EXTRA_LDFLAGS}

# Always build with debug
CFLAGS += -g

# media probe (only needed for playback and record)
BSEAV = $(NEXUS_TOP)/../BSEAV
include $(BSEAV)/lib/utils/batom.inc
include $(BSEAV)/lib/media/bmedia.inc
CFLAGS += $(addprefix -I,$(BMEDIA_INCLUDES))
CFLAGS += $(addprefix -D,$(BMEDIA_PROBE_DEFINES))

# DECODE_APPS need DECODE_SRCS
DECODE_OBJS = $(foreach file, $(DECODE_SRCS), $(BINDIR)/obj/$(patsubst %.c,%.o,$(file)))
$(addprefix $(BINDIR)/,$(DECODE_APPS)): $(DECODE_OBJS)

$(BINDIR)/obj/%.o: %.c
	@echo [Compile... $<]
	@mkdir -p $(dir $@)
	$(Q_)$(CC) -c -o $@ $^ $(CFLAGS)

define example_app_rule
$$(BINDIR)/$1: $1.c $$(if $${B_NEXUS_API_BUILD_COMPLETED}, , api)
	@echo [Compile... $$<]
	@mkdir -p $$(dir $$@)
	$$(Q_)$$(CC) -o $$@ $$(filter %.c %.s %.o, $$^) $$(CFLAGS) $$(LDFLAGS)
	cp -f $$@ $${NEXUS_BIN_DIR}

$1: $$(BINDIR)/$1
	@test 1
endef

$(foreach app, $(APPS), $(eval $(call example_app_rule,$(strip $(app)))))

install: all
