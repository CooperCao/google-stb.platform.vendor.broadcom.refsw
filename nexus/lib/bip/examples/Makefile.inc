#############################################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

# Makefile include fragment for BIP Examples.

############################################################
# Input Flags:
#   BIP_SERVER_APP	 define if app uses bip_server lib and includes
#   BIP_CLIENT_APP   define if app uses bip_client lib and includes
#   BIP_NAL_APP      define if app uses bip/utils/nal/ code
############################################################

APPS = $(THEAPPS)
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
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/lib/bip/examples/$(notdir ${CURDIR})

# In the event that the APPS variable is empty, then there's nothing to do,
# so define a target that does nothing.
ifeq ($(APPS),)
    donothing:
		@test 0;    # Just a no-op for the shell
endif

#  Define second-phase targets as phony first-phase targets so they
#  will trigger the respawn to the out-of-source directory.
PHONY += all apps clean minclean veryclean clean_apps install
.PHONY: ${PHONY}

${PHONY} ${APPS}: | ${BINDIR}
	$(Q_) B_REFSW_REAL_MAKE=1 ${MAKE} -C ${BINDIR} B_REFSW_CURDIR=${CURDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} -f ${CURDIR}/Makefile ${MAKECMDGOALS}

.PHONY: $(BINDIR)
${BINDIR}:
	$(Q_)mkdir -p ${BINDIR}


#  For convenience, allow some of the bip/build/Makefile's targets to be
#  passed through from here.  This will allow users to clean specific BIP
#  components or sub-libraries from here.  For example: "make clean_nexus"
#  will clean nexus or "make clean_client" will clean only the client
#  component of BIP.
BIP_TOP := ${NEXUS_TOP}/lib/bip
include ${BIP_TOP}/bip_lib.inc

BIP_SUBMAKE_TARGETS := $(foreach lib,$(BIP_LIB_LIST),clean_$(lib))
BIP_SUBMAKE_TARGETS += $(foreach sublib,$(BIP_ALL_LIB_SUBLIBS),clean_$(sublib))

.PHONY: $(BIP_SUBMAKE_TARGETS)
$(BIP_SUBMAKE_TARGETS):
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/bip/build   $@

######### End of first pass (in source directory tree). ##########
else # B_REFSW_REAL_MAKE
######### Start of second pass (out of source directory tree). ##########

NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
BSEAV_TOP ?= $(abspath ${NEXUS_TOP}/../BSEAV)


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

# Always use NEXUS_CFLAGS (with or without NxClient).
NEXUS_APP_INCLUDE_PATHS+=$(NEXUS_TOP)/utils
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# But don't use NEXUS_LDFLAGS or NEXUS_LD_LIBRARIES for NxClient.
# Set up Nexus' and NxClient's LDFLAGS and CFLAGS depending on the build flags.
ifeq ($(NXCLIENT_SUPPORT),y)
        # Nexus NxClient Mode: NXCLIENT_SUPPORT=y, NEXUS_CLIENT_SUPPORT=<don't care>
        include $(NEXUS_TOP)/nxclient/include/nxclient.inc
        CFLAGS += ${NXCLIENT_CFLAGS}
        CXXFLAGS += ${NXCLIENT_CFLAGS}
        LDFLAGS += ${NXCLIENT_LDFLAGS}
else
    ifeq ($(NEXUS_CLIENT_SUPPORT),y)
        # Nexus Multiprocess non-NxClient Mode: NXCLIENT_SUPPORT=, NEXUS_CLIENT_SUPPORT=y
        LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_CLIENT_LD_LIBRARIES) -lm
    else
        # Nexus Single-process Mode: NXCLIENT_SUPPORT=, NEXUS_CLIENT_SUPPORT=
        LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES) -lm
    endif
endif

# Include BIP definitions
BIP_TOP := ${NEXUS_TOP}/lib/bip
include ${BIP_TOP}/bip_lib.inc

# Decide which of the four sets of BIP's CFLAGS and LDFLAGS we should use.
ifneq ($(BIP_CLIENT_APP),)
    ifneq ($(BIP_SERVER_APP),)
        CFLAGS  += $(BIP_ALL_LIB_CFLAGS)
        LDFLAGS += $(BIP_ALL_LIB_LDFLAGS)
    else
        CFLAGS  += $(BIP_CLIENT_LIB_CFLAGS)
        LDFLAGS += $(BIP_CLIENT_LIB_LDFLAGS)
    endif
else
    ifneq ($(BIP_SERVER_APP),)
        CFLAGS  += $(BIP_SERVER_LIB_CFLAGS)
        LDFLAGS += $(BIP_SERVER_LIB_LDFLAGS)
    else
        CFLAGS  += $(BIP_BASE_LIB_CFLAGS)
        LDFLAGS += $(BIP_BASE_LIB_LDFLAGS)
    endif
endif

include ${NEXUS_TOP}/lib/os/b_os_lib.inc
CFLAGS += $(addprefix -I,$(B_OS_LIB_PUBLIC_INCLUDES))
CFLAGS += -std=c99
LDFLAGS += -L$(NEXUS_BIN_DIR) -lb_os

BIPEX_EXTRA_INCLUDES += $(NEXUS_TOP)/utils

include ${BSEAV_TOP}/lib/media/bmedia.inc
BIPEX_EXTRA_INCLUDES += $(BMEDIA_INCLUDES)
BIPEX_EXTRA_INCLUDES += $(BSEAV_TOP)/../magnum/syslib/sagelib/include
BIPEX_EXTRA_INCLUDES += $(BSEAV_TOP)/lib/security/sage/srai/include

####  Target Rules  ####

#  Default rule / "make all" / "make apps"
.PHONY:  all  apps
all: apps
apps: $(APPS)

$(APPS): | bip_libs

# This cleans nexus and local apps
.PHONY:  clean  minclean  veryclean  install
clean:          clean_bip_libs  clean_apps
minclean:    minclean_bip_libs  clean_apps
veryclean:  veryclean_bip_libs  clean_apps
install:                      install_apps
	cp -f ${APPS} ${NEXUS_BIN_DIR}

# Build BIP and its sublibraries.
BIP_EXAMPLE_BIP_LIBS := base

ifneq ($(BIP_CLIENT_APP),)
    BIP_EXAMPLE_BIP_LIBS += client
endif
ifneq ($(BIP_SERVER_APP),)
    BIP_EXAMPLE_BIP_LIBS += server
endif

.PHONY: nexus_libs
nexus_libs:
    ifeq ($(NXCLIENT_SUPPORT),y)
		@echo "[Build... nxclient (server)]"
		$(Q_)B_REFSW_REAL_MAKE= $(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/nxclient/server
    else
		@echo "[Build... nexus]"
		$(Q_)$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build
    endif

.PHONY:  bip_libs  clean_bip_libs  minclean_bip_libs  veryclean_bip_libs
bip_libs:  nexus_libs
	@echo "[Making... bip]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/bip/build   $(foreach lib,$(BIP_EXAMPLE_BIP_LIBS),$(lib))

clean_bip_libs:
	@echo "[Cleaning... bip]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/bip/build   $(foreach lib,$(BIP_EXAMPLE_BIP_LIBS),clean_$(lib))

minclean_bip_libs:
	@echo "[Cleaning... bip]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/bip/build   $(foreach lib,$(BIP_EXAMPLE_BIP_LIBS),minclean_$(lib))

veryclean_bip_libs:
	@echo "[Cleaning... bip]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/bip/build   $(foreach lib,$(BIP_EXAMPLE_BIP_LIBS),veryclean_$(lib))
    ifeq ($(NXCLIENT_SUPPORT),y)
		@echo "[Cleaning... nxclient (server)]"
		$(Q_)B_REFSW_REAL_MAKE= $(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/nxclient/server clean
    else
		@echo "[Cleaning... nexus]"
		$(Q_)$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build clean
    endif

install_bip_libs: bip_libs
	@echo "[Install... bip]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/bip/build   $(foreach lib,$(BIP_EXAMPLE_BIP_LIBS),install_$(lib))


# CLEAN_APPS allows cleaning apps that may not be in APPS because of env variables
.PHONY:  clean_apps  install_apps
clean_apps:  $(OTHER_APP_CLEANS)
	@echo "[Cleaning... $(APPS)"
	-$(Q_)$(RM) $(APPS) $(CLEAN_APPS) *.d *.o *.out

install_apps: $(APPS) install_bip_libs
	cp -f ${APPS} ${NEXUS_BIN_DIR}


# Always build with debug
CFLAGS += -g

CFLAGS += $(addprefix -I,$(BIPEX_EXTRA_INCLUDES))

ifeq ($(B_REFSW_ANDROID),y)
    include $(NEXUS_TOP)/build/android.inc
    LDFLAGS := $(ANDROID_LDFLAGS)
endif

vpath %.c ${B_REFSW_CURDIR}

# append to EXTRA_OBJS for any extra source file needs, including DRM, etc.

# Uncomment the next line to print out these variables for debugging...
#  BIP_LIB_DEBUG=1
ifneq ($(BIP_LIB_DEBUG),)
    $(foreach i,                        \
    NEXUS_LDFLAGS                       \
    NXCLIENT_LDFLAGS                    \
    NEXUS_LD_LIBRARIES                  \
    NEXUS_CLIENT_LD_LIBRARIES           \
    LDFLAGS                             \
    ,                                   \
        $(info )                        \
        $(info $i: **************)      \
        $(foreach j,$($(i)),            \
            $(info $(j))                \
         )                              \
    )
endif

# Implicit rule for building local apps
%: $(patsubst %.o,%.c,$(EXTRA_OBJS)) %.c
	@echo [Compile... $@]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)
	cp -f $@ ${NEXUS_BIN_DIR}

-include *.d


######### End of second pass (out of source directory tree). ##########
endif # B_REFSW_REAL_MAKE
