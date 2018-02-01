#############################################################################
# Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

# ASP Examples.
B_THIS_DIR:=nexus/lib/asp/examples

ifdef B_REFSW_REAL_NEXUS_TOP
    NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

# no extra source needed
BASIC_BUILD_APPS = \
		 asp_playpump_streamer \
         asp_streamer \
         asp_nw_sw_rules_test \
         asp_player

# extra source needed
CUSTOM_BUILD_APPS =
B_LIB_TOP := $(NEXUS_TOP)/lib

############################################################
# Input Flags:
#   ASP_APP define if app uses ASP lib and includes
############################################################

APPS = $(BASIC_BUILD_APPS) $(CUSTOM_BUILD_APPS)
APPS ?= $(APP)

# Define some VERBOSE-related variables
ifeq ($(VERBOSE),)
    Q_ := @
else
   Q_ :=
endif

ifeq ($(B_REFSW_REAL_MAKE),)
######### Start of first pass (in source directory tree). ##########

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/lib/asp/examples/$(notdir ${CURDIR})

#  Define second-phase targets as phony first-phase targets so they
#  will trigger the respawn to the out-of-source directory.
PHONY += all apps clean clean_apps install
.PHONY: ${PHONY}

${PHONY} ${APPS}: | ${BINDIR}
	$(Q_)${MAKE}             -C ${BINDIR} B_REFSW_CURDIR=${CURDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile ${MAKECMDGOALS}

.PHONY: $(BINDIR)
${BINDIR}:
	$(Q_)mkdir -p ${BINDIR}


#  For convenience, allow some of the asp/build/Makefile's targets to be
#  passed through from here.  This will allow users to clean specific ASP
#  components or sub-libraries from here.  For example: "make clean_nexus"
#  will clean nexus or "make clean_client" will clean only the client
#  component of ASP.
ASP_TOP := ${NEXUS_TOP}/lib/asp
include ${ASP_TOP}/b_asp_lib.inc

######### End of first pass (in source directory tree). ##########
else # B_REFSW_REAL_MAKE
######### Start of second pass (out of source directory tree). ##########

NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}

# Include Nexus definitions
ifeq ($(NEXUS_PREBUILT_BINARY),y)
    # do "make api" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
    NEXUS_BIN_DIR = ${B_REFSW_OBJ_ROOT}/bin${NEXUS_BIN_DIR_SUFFIX}
    include $(NEXUS_BIN_DIR)/include/platform_app.inc
else
    NEXUS_PLATFORM ?= $(PLATFORM)
    ifndef NEXUS_PLATFORM
        $(error NEXUS_PLATFORM is not defined)
    endif

    include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif

# Include ASP definitions
ASP_TOP := ${NEXUS_TOP}/lib/asp
include ${ASP_TOP}/b_asp_lib.inc

####  Target Rules  ####

#  Default rule / "make all" / "make apps"
.PHONY:  all  apps
all: apps
apps: $(APPS)

$(APPS): | asp b_os nexus asp_netfilter_drv

# This cleans nexus and local apps
.PHONY:  clean  install
clean:   clean_asp clean_nexus clean_b_os clean_apps clean_asp_netfilter_drv
install:                      install_apps
	cp -f ${APPS} ${NEXUS_BIN_DIR}

# Build ASP and its sublibraries.

.PHONY:  asp clean_asp
asp:
	@echo "[Making... asp]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/asp install

clean_asp:
	@echo "[Cleaning... asp]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/asp clean

###############################################################################
#  Define build/clean targets for each individual sub-library.
###############################################################################
#  Nexus Targets  ("nexus", "clean_nexus").
###############################################################################
.PHONY: nexus
nexus:
    # Don't build nexus if we have pre-built binaries
    ifneq ($(NEXUS_PREBUILT_BINARY),y)
	    @echo "[Build... nexus]"
	    $(Q_)$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build install
    endif

.PHONY: clean_nexus
clean_nexus:
    # Don't build nexus if we have pre-built binaries
    ifneq ($(NEXUS_PREBUILT_BINARY),y)
	    @echo "[Clean... nexus]"
	    $(Q_)$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build clean
    endif #NEXUS_PREBUILT_BINARY

.PHONY: asp_netfilter_drv
asp_netfilter_drv:
    # Build ASP Netfilter Driver
	@echo "[Build... asp_netfilter_drv]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/linux/driver/asp

.PHONY: clean_asp_netfilter_drv
clean_asp_netfilter_drv:
	@echo "[Clean... asp_netfilter_drv]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/linux/driver/asp clean

###############################################################################
#  b_os Targets  ("b_os", "clean_b_os").
###############################################################################
.PHONY: b_os
b_os:
	@echo "[Build... b_os]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/os install

.PHONY: clean_b_os
clean_b_os:
	@echo "[Clean... b_os]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/os clean

# CLEAN_APPS allows cleaning apps that may not be in APPS because of env variables
.PHONY:  clean_apps  install_apps
clean_apps:  $(OTHER_APP_CLEANS)
	@echo "[Cleaning... $(APPS)"
	-$(Q_)$(RM) $(APPS) $(CLEAN_APPS) *.d *.o *.out

install_apps: $(APPS) asp b_os nexus tshdrbuilder
	cp -f ${APPS} ${NEXUS_BIN_DIR}

# This is the minimum needed to compile and link with Nexus

CFLAGS += -I$(NEXUS_TOP)/lib/os/include -I$(NEXUS_TOP)/lib/os/include/linuxuser -I$(NEXUS_TOP)/lib/asp/include
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))  $(B_ASP_LIB_CFLAGS)
CFLAGS +=   $(B_ASP_LIB_CFLAGS) $(addprefix -I,$(B_ASP_LIB_PUBLIC_INCLUDES))

# Always build with debug
CFLAGS += -g


LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES) -lm $(B_ASP_LIB_LDFLAGS) -lb_os
LDFLAGS += -lb_asp  #hack as B_ASP_LIB_LDFLAGS is not defined.

vpath %.c ${B_REFSW_CURDIR}

# append to EXTRA_OBJS for any extra source file needs, including DRM, etc.
# namevalue.c
CFLAGS += -I$(NEXUS_TOP)/utils -I$(NEXUS_TOP)/../BSEAV/lib/media
vpath %.c $(NEXUS_TOP)/utils $(NEXUS_TOP)/../BSEAV/lib/media

# media_probe.c
CFLAGS += -I$(NEXUS_TOP)/nxclient/apps/utils
vpath %.c $(NEXUS_TOP)/nxclient/apps/utils

#
# kludge: UDP parser is needed to parse the raw user data from nexus API for cc test purpose.
# This part is to compile UDP of magnum and link with transcode unittests.
MAGNUM = $(NEXUS_TOP)/../magnum
#include $(MAGNUM)/basemodules/mma/bmma.inc
#CFLAGS += $(addprefix -I,$(BMMA_INCLUDES))
#MODULES += BMMA BAVC BFMT BPXL BSUR BUDP BVLC
#MAGNUM_MODULES:=


#CFLAGS +=$(addprefix -I,$(sort $(foreach module, $(MODULES), $($(module)_INCLUDES))))
#CFLAGS += $(addprefix -D,$(sort $(foreach module, $(MODULES), $($(module)_DEFINES))))
#EXTRA_SRCS += $(notdir $(sort $(foreach module, $(MODULES), $($(module)_SOURCES))))
#vpath %c $(dir $(sort $(foreach module, $(MODULES), $($(module)_SOURCES))))

# rule for building local apps
$(BASIC_BUILD_APPS): %: %.c namevalue.c
	@echo [Compile ssood... $@]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)
	cp -f $@ ${NEXUS_BIN_DIR}

-include *.d

######### End of second pass (out of source directory tree). ##########
endif # B_REFSW_REAL_MAKE
