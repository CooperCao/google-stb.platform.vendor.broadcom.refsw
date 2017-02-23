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

# Tuning
suffix ?=

APPS = splashrun

export NEXUS_POWER_MANAGEMENT=n
export NEXUS_FILE_MUX_SUPPORT=n

# These are the only supported targets
PHONY := \
	all api save clean clean_api clean_apps artifacts clean_artifacts \
	nexus_headers nexus_print_options

.PHONY: ${PHONY} FORCE
.NOTPARALLEL: api apps artifacts

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif
# Splash programs cannot compile in kernel mode
ifeq ($(NEXUS_MODE),proxy)
$(error You have defined NEXUS_MODE=proxy. This is not supported)
endif

# This trick has to work in all three directories
B_THIS_DIR := BSEAV/app/splash/splashrun
REPO_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))

# Other useful directories
NEXUS_TOP := ${REPO_TOP}/nexus
SPLASH_TOP := ${REPO_TOP}/BSEAV/app/splash
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${REPO_TOP}/${B_REFSW_OBJ_DIR}
BIN_DIR := ${B_REFSW_OBJ_ROOT}/nexus/bin
ARTIFACT_DIR := ${B_REFSW_OBJ_ROOT}/BSEAV/app/splash

# Apply the user's suffix everywhere
SPLASH_RUL_CFILE = splash_vdc_rul${suffix}.c
SPLASH_RUL_HFILE = splash_vdc_rul${suffix}.h
SPLASH_RUL_FUNCTION = GetSplashData${suffix}
CFLAGS += -DSPLASH_RUL_CFILE=${SPLASH_RUL_CFILE}
CFLAGS += -DSPLASH_RUL_HFILE=${SPLASH_RUL_HFILE}
CFLAGS += -DSPLASH_RUL_FUNCTION=${SPLASH_RUL_FUNCTION}

# These are the names of files to be saved back into the git repository
ifeq ($(splash_pal),y)
CFLAGS += -DCFG_SPLASH_PAL
SPLASH_RUL_H = splash_vdc_rul${suffix}_pal.h
SPLASH_RUL_C = splash_vdc_rul${suffix}_pal.c
else
SPLASH_RUL_H = splash_vdc_rul${suffix}_def.h
SPLASH_RUL_C = splash_vdc_rul${suffix}_def.c
endif

# App Stuff
APP_SRCS = \
	splash_vdc_run.c  \
	splash_script_load.c  \
	splash_bmp.c \
	splash_file.c \
	${SPLASH_RUL_CFILE} \
	splash_main.c
APP_OBJS = $(foreach file, $(patsubst %.c,splashrun/%.o,$(APP_SRCS)), $(file))

# Always build with debug
CFLAGS += -g
LDFLAGS += -g


ifeq ($(B_REFSW_MAKE_PART),)
###########################################################
# This section of Makefile executes from source directory #
###########################################################

# Needed to get some basic definitions, such as BCHP_CHIP
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

# The following are necessary because the applicaton's source files
# include magnum .h files. However some of these defines interfere with
# nexus makefiles.
MAGNUM := $(REPO_TOP)/magnum
include $(REPO_TOP)/magnum/portinginterface/vdc/bvdc.inc
include $(REPO_TOP)/magnum/commonutils/rdc/brdc.inc
# include $(REPO_TOP)/magnum/portinginterface/pwm/bpwm.inc
include $(REPO_TOP)/magnum/commonutils/fmt/bfmt.inc
include $(REPO_TOP)/magnum/commonutils/sur/bsur.inc
include $(REPO_TOP)/magnum/commonutils/avc/bavc.inc
include $(REPO_TOP)/magnum/commonutils/pxl/bpxl.inc
include $(REPO_TOP)/magnum/commonutils/mth/bmth.inc
include $(REPO_TOP)/magnum/basemodules/mma/bmma.inc
include $(REPO_TOP)/magnum/basemodules/int/bint.inc
include $(REPO_TOP)/magnum/basemodules/tmr/btmr.inc
include $(REPO_TOP)/magnum/basemodules/dtu/bdtu.inc
include $(REPO_TOP)/magnum/portinginterface/i2c/bi2c.inc
include $(REPO_TOP)/magnum/commonutils/mrc/bmrc.inc
include $(REPO_TOP)/magnum/commonutils/box/bbox.inc
MAGNUM_INCLUDE_PATHS_T += \
	$(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
MAGNUM_INCLUDE_PATHS += $(addprefix -I,$(MAGNUM_INCLUDE_PATHS_T))
MAGNUM_DEFINES_T := \
	$(sort $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))
MAGNUM_DEFINES := $(addprefix -D,$(MAGNUM_DEFINES_T))
MAGNUM_CFLAGS := $(MAGNUM_INCLUDE_PATHS) $(MAGNUM_DEFINES)

# These won't get built by the api target
ONLY_MODULES=BFMT BSUR BAVC BPXL BMTH BBOX
ONLY_DEFINES_T := \
	$(sort $(foreach module, $(ONLY_MODULES), $($(module)_DEFINES)))
ONLY_DEFINES := $(addprefix -D,$(ONLY_DEFINES_T))
ONLY_DIR =  $(foreach module,$(ONLY_MODULES),$(dir $($(module)_SOURCES)))

# Final targets
all: api artifacts save
	@echo "Final build in ${BIN_DIR}"
	@${MAKE} -C ${BIN_DIR} \
		suffix=${suffix} B_REFSW_MAKE_PART=nexus REPO_TOP=${REPO_TOP} \
		-f ${CURDIR}/Makefile ONLY_OBJ="${ONLY_OBJ}" $@
clean: clean_api clean_artifacts clean_apps

# Top level (applications) software: artifacts
artifacts: ${ARTIFACT_DIR}/splashrun/exists api
	@echo "Build intermediate files (artifacts) in ${ARTIFACT_DIR}"
	@${MAKE} -C ${ARTIFACT_DIR} \
		suffix=${suffix} REPO_TOP=${REPO_TOP} B_REFSW_MAKE_PART=BSEAV \
		ONLY_OBJ="${ONLY_OBJ}" ONLY_DEFINES="${ONLY_DEFINES}" \
		ONLY_SRC="${ONLY_SRC}" ONLY_DIR="${ONLY_DIR}" \
		MAGNUM_CFLAGS="${MAGNUM_CFLAGS}" \
		-f ${CURDIR}/Makefile $@
clean_artifacts: ${ARTIFACT_DIR}/splashrun/exists
	@echo "Clean intermediate files (artifacts) in ${ARTIFACT_DIR}"
	@${MAKE} -C ${ARTIFACT_DIR} \
		suffix=${suffix} REPO_TOP=${REPO_TOP} B_REFSW_MAKE_PART=BSEAV \
		-f ${CURDIR}/Makefile $@
clean_apps:  ${BIN_DIR}/exists
	@echo "Clean finished targets in ${BIN_DIR}"
	@${MAKE} -C ${BIN_DIR} \
		suffix=${suffix} B_REFSW_MAKE_PART=nexus REPO_TOP=${REPO_TOP} \
		-f ${CURDIR}/Makefile $@

# Nexus features
api: ${BIN_DIR}/exists
	@echo "Build nexus API"
	@$(MAKE) -C $(NEXUS_TOP)/build
clean_api:
	@echo "Clean nexus API"
	@$(MAKE) -C $(NEXUS_TOP)/build clean
nexus_print_options nexus_headers:
	$(MAKE) -C $(NEXUS_TOP)/build $@

${BIN_DIR}/exists ${ARTIFACT_DIR}/splashrun/exists:
	mkdir -p $@

# copy from ${BIN_DIR}
save: ${NEXUS_PLATFORM}${BCHP_VER}/$(SPLASH_RUL_H) ${NEXUS_PLATFORM}${BCHP_VER}/$(SPLASH_RUL_C)
${NEXUS_PLATFORM}${BCHP_VER}/$(SPLASH_RUL_H) : FORCE
	@echo [Copying to ... $@]
	@mkdir -p ${NEXUS_PLATFORM}${BCHP_VER}
	@cp -f ${BIN_DIR}/${SPLASH_RUL_HFILE} $@
${NEXUS_PLATFORM}${BCHP_VER}/$(SPLASH_RUL_C) : FORCE
	@echo [Copying to ... $@]
	@mkdir -p ${NEXUS_PLATFORM}${BCHP_VER}
	@cp -f ${BIN_DIR}/${SPLASH_RUL_CFILE} $@

else ifeq ($(B_REFSW_MAKE_PART), nexus)
###############################################################
# This section of Makefile executes from obj.nnnnn/nexus/bin. #
# Final outputs go here.                                      #
###############################################################

# This is the minimum needed to link with Nexus
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)

all: apps splash.bmp

apps: $(APPS)

LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)

ifeq ($(B_REFSW_ANDROID),y)
include $(NEXUS_TOP)/build/android.inc
LDFLAGS := $(ANDROID_SERVER_LDFLAGS)
endif

ALL_OBJ = \
	$(foreach file, $(APP_OBJS) $(ONLY_OBJ), \
		$(addprefix $(ARTIFACT_DIR)/, $(file)))

clean_apps:
	-$(RM) $(APPS) splash.bmp

splash.bmp: $(SPLASH_TOP)/splashgen/splash.bmp
	cp $< $@

$(APPS): $(ALL_OBJ)
	@echo [Link... $(notdir $@)]
	$(Q_)$(CC) -o $@ $(ALL_OBJ) $(LDFLAGS)

else ifeq ($(B_REFSW_MAKE_PART), BSEAV)
###############################################################
# This section of Makefile executes from                      #
# obj.nnnnn/BSEAV/app/splashrun.                              #
# Intermediate files go here.                                 #
###############################################################

# This is the minimum needed to compile with Nexus
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
CFLAGS += \
	$(NEXUS_CFLAGS) \
	$(addprefix -I, $(NEXUS_APP_INCLUDE_PATHS)) \
	$(addprefix -D, $(NEXUS_APP_DEFINES))
CFLAGS += -I$(NEXUS_TOP)/base/include

CFLAGS += $(MAGNUM_CFLAGS)
CFLAGS += -I$(SPLASH_TOP)/common
CFLAGS += -I$(SPLASH_TOP)/os/usermode
CFLAGS += -I$(SPLASH_TOP)/splashrun
CFLAGS += -I$(BIN_DIR)

ifeq ($(NEXUS_PLATFORM),97435)
ifneq ($(NEXUS_PLATFORM_7435_1U4T),n)
CFLAGS += -DNEXUS_PLATFORM_7435_BOX_TYPE_1u4t=1
endif
endif

vpath %.c ${SPLASH_TOP}/splashrun
vpath %.c ${SPLASH_TOP}/common
vpath %.c ${BIN_DIR} # For splash_vdc_rul....c

CFLAGS += ${ONLY_DEFINES}

artifacts: ${APP_OBJS} ${ONLY_OBJ}

clean_artifacts:
	-$(RM) *.d *.o *.out splashrun/*.d splashrun/*.o splashrun/*.out

%.o: %.c
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)
splashrun/%.o: %.c
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)

-include *.d
-include splashrun/*.d

endif # B_REFSW_MAKE_PART
#################################
# End of location-specific code #
#################################

FORCE:
