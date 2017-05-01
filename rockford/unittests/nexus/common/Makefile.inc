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

.PHONY: all nexus apps clean clean_apps install install_only print_files      bwin

# Build the name of this makefile, relative to B_REFSW_TOP (used for recipe tracing)
# Keep this line before any includes!
B_THIS_MAKEFILE_NAME := $(subst $(abspath ${NEXUS_TOP}/..),,$(abspath $(lastword $(MAKEFILE_LIST))))
# Uncomment the following two lines to enable recipe tracing.
# B_TRACE_RECIPE_BEGIN = @echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>  ${B_THIS_MAKEFILE_NAME}: Begin recipe: $@"
# B_TRACE_RECIPE_END   = @echo "<<<<<<<<<<<<<<<<<<<<<<<<<<<<  ${B_THIS_MAKEFILE_NAME}:   End recipe: $@"

ifndef B_REFSW_TOP
# we can derive B_REFSW_TOP from this include file's path
B_REFSW_TOP := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../../../..)
endif
NEXUS_TOP := $(B_REFSW_TOP)/nexus

NEXUS_PLATFORM ?= $(PLATFORM)
ifeq ($(NEXUS_PLATFORM),)
    $(error NEXUS_PLATFORM is not defined)
endif

BSEAV ?= ${NEXUS_TOP}/../BSEAV
MAGNUM ?= ${NEXUS_TOP}/../magnum

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
ifeq ($(NEXUS_MODE),)
    # if not proxy, then force usermode multiprocess
    export NEXUS_BUILD_OPTIONS = NEXUS_MODE=client
else
    export NEXUS_MODE
endif

THISDIR := $(subst $(dir $(NEXUS_TOP)),,$(shell pwd))
BINDIR := $(B_REFSW_OBJ_ROOT)/${THISDIR}

# include IPSTB-related variables if PLAYBACK_IP_SUPPORT is explicitly on
ifeq ($(PLAYBACK_IP_SUPPORT),y)

    #needed for blive_ext.mak
    ifeq ($(B_REFSW_ARCH),)
        B_REFSW_ARCH=mipsel-linux
        export B_REFSW_ARCH
    endif
    B_REFSW_DEBUG ?= y
    ifeq ($(B_REFSW_DEBUG),y)
        DEBUG_SUFFIX = debug
    else
        DEBUG_SUFFIX = release
    endif

    # ip_playback applib definitions
    include $(NEXUS_TOP)/lib/playback_ip/b_playback_ip_lib.inc
    NEXUS_APP_INCLUDE_PATHS += $(B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES)

    # The b_playback_ip_lib.inc seems to add "B_HAS_AVI" and that causes problems.
    # So filter it out of playback_ip's defines.
    NEXUS_APP_DEFINES +=   $(filter-out B_HAS_AVI,$(B_PLAYBACK_IP_LIB_DEFINES))
    LDFLAGS += $(PLAYBACK_IP_LDFLAGS)

    ifneq ($(LIVEMEDIA_SUPPORT),n)
        LIVEMEDIA_SUPPORT=y
        export LIVEMEDIA_SUPPORT
    endif

    # Add playback_ip to the list of sub-libraries to build.
    SUBLIBS += ippblibs

endif #PLAYBACK_IP_SUPPORT

ifeq ($(UNITTEST_BWIN_SUPPORT),y)
    # Add bwin to the list of sub-libraries to build.
    SUBLIBS += bwin
endif

# If we're building the playback_ip sub-library, then it will build nexus and os_lib
# but if not, then we'll need to build them... so add them to the sub-lib list.
ifneq ($(PLAYBACK_IP_SUPPORT),y)
    ifneq ($(B_NEXUS_API_BUILD_COMPLETED),y)
        SUBLIBS += nexus
    endif
    ifeq ($(APPS),BUILD_SUBLIBS_ONLY)
        OS_LIB_UNITTESTS = y
    endif
    ifeq ($(OS_LIB_UNITTESTS),y)
        SUBLIBS += oslib
    endif
endif

ifeq ($(NEXUS_POWER_MANAGEMENT),y)
    SUBLIBS += pmlib
endif

ifeq ($(SSL_SUPPORT),y)
    export SSL_SUPPORT
endif

# If the APPS is specified as BUILD_SUBLIBS_ONLY, then we're running
# the unittests/nexus/common/Makefile that just builds the sub-libraries.
ifeq ($(APPS),)
    $(error Must define APPS)
endif
ifeq ($(APPS),BUILD_SUBLIBS_ONLY)
    APPS =
endif

ifeq ($(BUILD_IP_UNITTESTS),y)
    ifneq ($(PLAYBACK_IP_SUPPORT),y)
        $(error You must set PLAYBACK_IP_SUPPORT=y to use option BUILD_IP_UNITTESTS)
    endif

    ifneq ($(NETACCEL_SUPPORT),n)
        export NETACCEL_SUPPORT=y
    endif

    IP_STREAMER_DIR := ../ip_streamer
    include $(IP_STREAMER_DIR)/Makefile.inc
    vpath %c $(IP_STREAMER_DIR)
    vpath %h $(IP_STREAMER_DIR)
    IP_STREAMER_CFLAGS := -I$(IP_STREAMER_DIR) -DIP_STREAMER_SUBST
endif #BUILD_IP_UNITTESTS

ifeq ($(BUILD_CHANNELS_FILE_UNITTESTS),y)

    CHANNELS_FILE_DIR := ../channels_file
    include $(CHANNELS_FILE_DIR)/Makefile.inc
    vpath %c $(CHANNELS_FILE_DIR)
    vpath %h $(CHANNELS_FILE_DIR)
    CHANNELS_FILE_CFLAGS := -I$(CHANNELS_FILE_DIR) -DCHANNELS_FILE_SUPPORT
endif #BUILD_CHANNELS_FILE_UNITTESTS

APP_BINS = $(addprefix $(BINDIR)/,$(APPS))

.PHONY: nexus apps clean clean_apps

# If the sub-libraries have already been built, we don't have
# to build them now.
ifeq ($(B_SUBLIB_BUILD_COMPLETED),1)
    SUBLIBS =
endif

ifeq ($(APPS),)
    all: $(SUBLIBS)
else
    all: apps
endif

apps: $(APPS)
$(APP_BINS): $(SUBLIBS)

ifeq ($(DTCP_IP_SUPPORT),y)
    include $(NEXUS_TOP)/lib/dtcp_ip/dtcp_ip_lib.inc
    NEXUS_APP_INCLUDE_PATHS += $(B_DTCP_IP_LIB_PUBLIC_INCLUDES) $(BSEAV)/lib/security/bcrypt/include
    NEXUS_APP_DEFINES += $(B_DTCP_IP_LIB_DEFINES)
    NEXUS_CFLAGS += $(B_DTCP_IP_LIB_CFLAGS)
    LDFLAGS += $(B_DTCP_IP_LIB_LDFLAGS)
endif #DTCP_IP_SUPPORT

ifeq ($(UNITTEST_BWIN_SUPPORT),y)
    ifeq ($(B_REFSW_ARCH),)
        export B_REFSW_ARCH=mipsel-linux
    endif
    ifneq ($(DEBUG),n)
        export DEBUG=y
    endif

    include $(BSEAV)/lib/bwin/include/bwin.mak
    CFLAGS += $(BWIN_CFLAGS)
    LDFLAGS += ${BWIN_LDFLAGS}

    .PHONY: bwin clean_bwin

bwin:
		$(B_TRACE_RECIPE_BEGIN)
		$(Q_)$(MAKE) -C $(BSEAV)/lib/bwin/build -j 1
		$(B_TRACE_RECIPE_END)

clean_bwin:
		$(B_TRACE_RECIPE_BEGIN)
		$(Q_)$(MAKE) -C $(BSEAV)/lib/bwin/build clean
		$(B_TRACE_RECIPE_END)
endif


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

# This builds playback_ip lib
ifeq ($(PLAYBACK_IP_SUPPORT),y)
    .PHONY: ippblibs
    ippblibs:
		$(B_TRACE_RECIPE_BEGIN)
		$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/playback_ip/apps  install
		$(B_TRACE_RECIPE_END)

    .PHONY: clean_ippblibs
    clean_ippblibs:
		$(B_TRACE_RECIPE_BEGIN)
		$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/playback_ip/apps clean
		$(B_TRACE_RECIPE_END)
endif # PLAYBACK_IP_SUPPORT

ifeq ($(NEXUS_POWER_MANAGEMENT),y)
    .PHONY: pmlib
    pmlib:
		$(B_TRACE_RECIPE_BEGIN)
		$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/pmlib
		$(B_TRACE_RECIPE_END)

    .PHONY: clean_pmlib
    clean_pmlib:
		$(B_TRACE_RECIPE_BEGIN)
		$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/pmlib clean
		$(B_TRACE_RECIPE_END)
endif

clean_apps: $(OTHER_APP_CLEANS)
	$(B_TRACE_RECIPE_BEGIN)
	-$(RM) $(APP_BINS) $(BINDIR)/*.d $(BINDIR)/*.o
	$(B_TRACE_RECIPE_END)

#
# OS lib option for unittests
#
ifneq ($(PLAYBACK_IP_SUPPORT),y)
    ifeq ($(OS_LIB_UNITTESTS),y)
        B_LIB_TOP := $(NEXUS_TOP)/lib
        include $(NEXUS_TOP)/lib/os/b_os_lib.inc
        CFLAGS += $(addprefix -I,$(B_OS_LIB_PUBLIC_INCLUDES))
        LDFLAGS += -lb_os
        oslib:
			$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/os
        clean_oslib:
			$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/os clean
    endif
endif



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

ifeq ($(BUILD_IP_UNITTESTS),y)
CFLAGS += $(IP_STREAMER_CFLAGS)
EXTRA_SRC += $(IP_STREAMER_SRC)
endif

ifeq ($(BUILD_CHANNELS_FILE_UNITTESTS),y)
CFLAGS += $(CHANNELS_FILE_CFLAGS)
EXTRA_SRC += $(CHANNELS_FILE_SRC)
endif

# Disable the built-in rules
.SUFFIXES:

define app_rule
$$(BINDIR)/$1: $$(BINDIR)/$1.o $$(addprefix $$(BINDIR)/,$$(patsubst %.c,%.o,$$(EXTRA_SRC)))
	$$(B_TRACE_RECIPE_BEGIN)
	@echo [Link... $$(notdir $$@)]
	@mkdir -p $$(dir $$@)
	$$(Q_)$$(CC) -o $$@ $$(filter %.o, $$^) $$(LDFLAGS)
	$$(Q_)mkdir -p $$(DESTDIR)
	$$(CP) $$@ $$(DESTDIR)
	$$(B_TRACE_RECIPE_END)

$1: $$(BINDIR)/$1
	@test 1
endef

$(foreach app, $(APPS), $(eval $(call app_rule,$(strip $(app)))))

$(BINDIR)/%.o: %.c
	@echo [Compile... $<]
	@mkdir -p $(dir $@)
	$(Q_)$(CC) -c -o $@ $^ $(CFLAGS)

.PHONY: install install_only
install: all install_only

APP_STREAMS = $(addsuffix .str,$(APPS))

install_only:
	$(B_TRACE_RECIPE_BEGIN)
	$(Q_)mkdir -p $(INSTALL_DIR)
	$(CP) $(APP_BINS) $(wildcard $(APP_STREAMS) $(APP_CHANNELS)) $(INSTALL_DIR)
ifneq  ($(CHANNEL_MAP_FILE),)
	$(CP) $(CHANNEL_MAP_FILE) $(DESTDIR)
endif
	$(B_TRACE_RECIPE_END)

print_files:
	$(B_TRACE_RECIPE_BEGIN)
	@echo $(APP_BINS)
	$(B_TRACE_RECIPE_END)
