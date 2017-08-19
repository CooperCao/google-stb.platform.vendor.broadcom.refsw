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
#############################################################################

ifdef COMSPEC
# Any DOS environment
B_LIB_TOP := $(shell cd ../../../nexus/lib && cd)
B_BSEAV_TOP := $(shell cd ../../../BSEAV && cd)
else
B_LIB_TOP := $(shell cd ../../../nexus/lib; pwd)
B_BSEAV_TOP := $(shell cd ../../../BSEAV; pwd)
endif
NEXUS_TOP := $(B_LIB_TOP)/..

ifndef NEXUS_PLATFORM
NEXUS_PLATFORM = $(PLATFORM)
endif
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif


ifeq ($(SOFT_GFX_SUPPORT),y)
CFLAGS += -DB_HAS_SOFT_GFX=1
CFLAGS += -I$(NEXUS_TOP)/lib/softgfx/include
EXTRA_SOFT_GFX_OBJS := b_softgfx
B_DCC_LIB_CFLAGS_OPTIONS += -DB_HAS_SOFT_GFX=1
endif #SOFT_GFX_SUPPORT

# include cross-compiler definitions
include $(B_LIB_TOP)/build/b_lib_defs.inc

THEAPPS = \
	eia708_app

ifeq ($(B_REFSW_OS),vxworks)
# VxWorks needs a wrapper function to call main.
VXOBJS = vxworks_cmd.o
# We like to use .out for loadable objects.
APPSUFFIX = .out
APPS = $(addsuffix .out, $(THEAPPS))
else
APPS = $(THEAPPS)
endif




.PHONY: api clean freetype clean_freetype

ifeq ($(APP),)
all: $(APPS)
$(APPS): api
else
all: $(APP)
$(APP): api
endif

# include applib specifics
include $(B_LIB_TOP)/os/b_os_lib.inc
include $(B_BSEAV_TOP)/lib/dcc/b_digital_closed_caption.inc

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -lnexus -lb_os -lm -lb_dcc -L$(NEXUS_BIN_DIR) -lpthread
endif

ifeq ($(SOFT_GFX_SUPPORT),y)
LDFLAGS += -l$(EXTRA_SOFT_GFX_OBJS)

CFLAGS += -I../../../../nexus/lib/softgfx/include
CFLAGS += -DB_HAS_SOFT_GFX=1
endif

ifeq ($(FREETYPE_SUPPORT),y)
FREETYPE_DIR = $(NEXUS_TOP)/../BSEAV/opensource/freetype
include $(FREETYPE_DIR)/b_freetype_targets.inc
CFLAGS += -DFREETYPE_SUPPORT -I$(FREETYPE_SOURCE_PATH)/include
LDFLAGS += -L$(FREETYPE_LIB_FOLDER) -lfreetype
freetype: b_freetype_target_build
clean_freetype: b_freetype_target_clean_all
endif

# This builds nexus and needed applibs
api: freetype
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(B_LIB_TOP)/os
	$(MAKE) -C $(B_BSEAV_TOP)/lib/dcc

# This cleans nexus and local apps
clean: clean_freetype
	$(MAKE) -C $(NEXUS_TOP)/build clean
	$(MAKE) -C $(B_LIB_TOP)/os clean
	$(MAKE) -C $(B_BSEAV_TOP)/lib/dcc clean

	-$(RM) $(APPS) *.d *.o *.out;

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Convert applib include paths into single variable.
APPLIB_INCLUDES := $(foreach lib,$(B_LIBS),$($(lib)_PUBLIC_INCLUDES))
CFLAGS += $(addprefix -I,$(APPLIB_INCLUDES))
APPLIB_DEFINES := $(foreach lib,$(B_LIBS),$($(lib)_DEFINES))
CFLAGS += $(addprefix -D,$(APPLIB_DEFINES))
CFLAGS += $(B_DCC_LIB_CFLAGS_OPTIONS)


# Always build with debug
CFLAGS += -g

# Implicit rule for building local apps
%$(APPSUFFIX): %.c $(VXOBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	@echo [Install... $@]
	$(Q_)$(CP) $@ $(NEXUS_BIN_DIR)
	@if [ ! -d $(NEXUS_BIN_DIR)/cc_fonts ]; then \
		echo [Install Fonts]; \
		$(MKDIR) $(NEXUS_BIN_DIR)/cc_fonts; \
		$(CP) CaptionFonts/* $(NEXUS_BIN_DIR)/cc_fonts; \
	fi

ifeq ($(SYSTEM),vxworks)
# Explicit rule for building vxworks wrapper app
vxworks_cmd.o: vxworks_cmd.c
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
endif

#
# Installation: copying app to DESTDIR
#
ifeq ($(DESTDIR),)
DESTDIR = $(NEXUS_BIN_DIR)
endif

.PHONY: install install_only
install: all install_only

ifeq ($(APP),)
install_only:
	$(CP) $(APPS) $(DESTDIR)
else
install_only:
	$(CP) $(APP) $(DESTDIR)
endif
