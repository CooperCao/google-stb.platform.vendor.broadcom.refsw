#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#
# Module Description:
#
#############################################################################

ifdef COMSPEC
# Any DOS environment
B_LIB_TOP := $(shell cd ../../../nexus/lib && cd)
B_BSEAV_TOP := $(shell cd ../../../BSEAV && cd)
else
B_LIB_TOP := $(shell cd ../../../nexus/lib; pwd)
B_BSEAV_TOP := $(shell cd ../../../BSEAV && pwd)
endif
NEXUS_TOP := $(B_LIB_TOP)/..

ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(B_LIB_TOP)/build/b_lib_defs.inc
ifeq ($(NXCLIENT_SUPPORT),y)
THEAPPS = \
        mmt_playback \
	tsmf_mmt_live \
	mmt_message_from_playback \
	mmt_subtitle_from_playback \
	mmt_record_from_playback
else
THEAPPS = \
	mmt_playback \
	mmt_live \
	mmt_message_from_playback \
	mmt_subtitle_from_playback \
	mmt_all_pass_record \
	mmt_record \
	mmt_record_from_playback \
	tlv_isdb_s3_live \
	tlv_isdb_s3_all_pass_record \
	tsmf_mmt_live
endif

APPS = $(THEAPPS)
all: $(APPS)
$(APPS): api
.PHONY: api clean $(APPS)
BINDIR := $(B_REFSW_OBJ_ROOT)/BSEAV/app/mmt
TARGET_APPS := $(addprefix $(BINDIR)/,$(APPS))


# add flags for tshdrbuilder lib
CFLAGS += -Wno-long-long
# include applib specifics
include $(B_LIB_TOP)/os/b_os_lib.inc
include $(B_BSEAV_TOP)/lib/mmt/b_mmt_lib.inc

# For linux builds, link to the correct libraries
#ifneq ($(findstring linux,$(B_REFSW_OS)),)
ifeq ($(NXCLIENT_SUPPORT),y)
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)
LDFLAGS += -lm -lpthread -lb_os  -lb_mmt
LDFLAGS += $(NXCLIENT_LDFLAGS)
CFLAGS += -DNXCLIENT_SUPPORT
else
LDFLAGS := -lm -L$(NEXUS_BIN_DIR) -lpthread -lnexus${LIB_NEXUS_SUFFIX} -lb_os  -lb_mmt
endif
#endif

# This builds nexus and needed applibs
ifeq ($(NXCLIENT_SUPPORT),y)
api:
	$(MAKE) -C $(NEXUS_TOP)/nxclient server
	#$(MAKE) -C $(NEXUS_TOP)/nxclient/build
	$(MAKE) -C $(B_LIB_TOP)/os
	$(MAKE) -C $(B_BSEAV_TOP)/lib/mmt
# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/nxclient clean_server
	#$(MAKE) -C $(NEXUS_TOP)/nxclient/build clean
	$(MAKE) -C $(B_LIB_TOP)/os clean
	$(MAKE) -C $(B_BSEAV_TOP)/lib/mmt clean
	-$(RM) $(TARGET_APPS)
else
api:
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(B_LIB_TOP)/os
	$(MAKE) -C $(B_BSEAV_TOP)/lib/mmt
# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	$(MAKE) -C $(B_LIB_TOP)/os clean
	$(MAKE) -C $(B_BSEAV_TOP)/lib/mmt clean
	-$(RM) $(TARGET_APPS)
endif
# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Convert applib include paths into single variable.
APPLIB_INCLUDES := $(foreach lib,$(B_LIBS),$($(lib)_PUBLIC_INCLUDES))
CFLAGS += $(addprefix -I,$(APPLIB_INCLUDES))
APPLIB_DEFINES := $(foreach lib,$(B_LIBS),$($(lib)_DEFINES))
CFLAGS += $(addprefix -D,$(APPLIB_DEFINES))


# Always build with debug
CFLAGS += -g

# use template to generate out-of-source build rules
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

install:
	cp -f ${TARGET_APPS} ${NEXUS_BIN_DIR}
