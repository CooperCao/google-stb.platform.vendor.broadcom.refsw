#############################################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
# File Description:
#
#############################################################################
# Nexus examples

PHONY := all nexus api clean clean_apps install
.PHONY: ${PHONY}

THEAPPS = \
	standby

APPS = $(THEAPPS)

NEXUS_PLATFORM ?= $(PLATFORM)
ifeq ($(NEXUS_PLATFORM),)
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(B_REFSW_REAL_MAKE),)
ifdef COMSPEC
# Any DOS environment
NEXUS_TOP := $(shell cd ../../../nexus && cd)
else
NEXUS_TOP := $(shell cd ../../../nexus; pwd)
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/BSEAV/app/standby

${PHONY} ${APPS}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile ${MAKECMDGOALS}

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE

NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}

ifeq ($(NEXUS_PREBUILT_BINARY),y)
# do "make api" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
include $(NEXUS_BIN_DIR)/include/platform_app.inc
Q_ ?= @
else
# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif


OBJDIR=./obj
vpath %.c ${NEXUS_TOP}/../BSEAV/app/standby
all: $(APPS)

$(OBJDIR)/exists:
	mkdir -p $(OBJDIR)
	@touch $@

ifneq ($(NEXUS_PREBUILT_BINARY),y)
$(APPS): $(if ${B_NEXUS_API_BUILD_COMPLETED}, , api)
endif

LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)

ifeq ($(B_REFSW_ANDROID),y)
include $(NEXUS_TOP)/build/android.inc
LDFLAGS := $(ANDROID_LDFLAGS)
endif


# This builds the Nexus api
nexus api:
	$(MAKE) -C $(NEXUS_TOP)/build

# needed to link with nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
LDFLAGS += ${NEXUS_EXTRA_LDFLAGS}

# Always build with debug
CFLAGS += -g

K_VERSION       := $(shell grep -m1 "^VERSION" $(LINUX)/Makefile | cut -d\  -f3)
K_PATCHLEVEL    := $(shell grep -m1 "^PATCHLEVEL" $(LINUX)/Makefile | cut -d\  -f3)
CFLAGS += -DK_VERSION=$(K_VERSION) -DK_PATCHLEVEL=$(K_PATCHLEVEL)

STANDBY_SRCS += standby.c standby_priv.c cmdline.c util.c

# Pmlib
PMLIB_TOP=$(NEXUS_TOP)/../BSEAV/lib/pmlib
include $(PMLIB_TOP)/pmlib.inc
clean: clean_pmlib
libs: pmlib

pmlib:
	$(Q_)$(MAKE) -C $(PMLIB_TOP)
clean_pmlib:
	$(Q_)$(MAKE) -C $(PMLIB_TOP) clean

CFLAGS += $(PMLIB_CFLAGS)
LDFLAGS += $(PMLIB_LDFLAGS)

ifeq ($(WOWLAN_SUPPORT),y)
CFLAGS += -DWOWLAN
endif

# needed for Android
ifeq ($(B_REFSW_ANDROID),y)
GLOB_PATH=$(NEXUS_TOP)/../BSEAV/lib/glob
vpath %.c $(GLOB_PATH)

CFLAGS += -I$(GLOB_PATH)
STANDBY_SRCS += glob.c
endif

# PSIP library is being built aas part of playback Ip
B_PSIP_TOP := $(NEXUS_TOP)/../rockford/lib/psip
include $(B_PSIP_TOP)/b_psip_lib.inc
CFLAGS += $(addprefix -I,$(B_PSIP_LIB_PUBLIC_INCLUDES)) -I$(BSEAV)/api/include

#
# playback_ip
#
.PHONY: playback_ip clean_playback_ip
ifeq ($(PLAYBACK_IP_SUPPORT),y)
include $(NEXUS_TOP)/lib/playback_ip/b_playback_ip_lib.inc
clean: clean_playback_ip
libs: playback_ip

playback_ip:
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/playback_ip/apps install
clean_playback_ip:
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/playback_ip/apps clean

CFLAGS += $(PLAYBACK_IP_CFLAGS)
LDFLAGS += $(PLAYBACK_IP_LDFLAGS)
else
.PHONY: os clean_os psip clean_psip
B_LIB_TOP=$(NEXUS_TOP)/lib
include $(B_LIB_TOP)/os/b_os_lib.inc
clean: clean_os clean_psip
libs: os psip

os:
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/os
clean_os:
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/os clean

psip:
	$(Q_)$(MAKE) -C $(B_PSIP_TOP)
clean_psip:
	$(Q_)$(MAKE) -C $(B_PSIP_TOP) clean

CFLAGS += $(addprefix -I,$(B_OS_LIB_PUBLIC_INCLUDES))
LDFLAGS += -lb_psip -lb_os
endif


include $(BSEAV)/lib/bfile/bfile.inc
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
CFLAGS += $(addprefix -I,$(BMEDIA_INCLUDES) $(BFILE_MEDIA_INCLUDES)) -I$(BSEAV)/lib/mpeg2_ts_parse -I$(BSEAV)/lib/tspsi
CFLAGS += $(addprefix -D,$(BMEDIA_PROBE_DEFINES))


STANDBY_OBJS = $(foreach file, $(STANDBY_SRCS), $(OBJDIR)/$(notdir $(patsubst %.c, %.o, $(file))))

$(OBJDIR)/%.o: %.c ${OBJDIR}/exists
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS) ${CDEP_FLAG}

$(APPS): $(STANDBY_OBJS) libs
	@echo [Link... $@]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	cp -f $@ $(NEXUS_BIN_DIR)

# This cleans nexus and local apps
clean: clean_apps
	$(MAKE) -C $(NEXUS_TOP)/build clean

clean_apps:
	-$(RM) -r $(APPS) $(OBJDIR)

install: all

-include ${OBJDIR}/*.d

endif # B_REFSW_REAL_MAKE
