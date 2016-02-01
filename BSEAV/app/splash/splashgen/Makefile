############################################################
#	  (c)2003-2014 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#  1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# Created: 08/28/2007 by Jon Garrett
#
# $brcm_Log: $
#
############################################################

# Tuning
suffix ?=

APPS = splashgen

export NEXUS_POWER_MANAGEMENT=n
export BRDC_DISABLE_TRACK_EXECUTION=y
export BMMA_USE_STUB=y

# These are the only supported targets
PHONY := \
	all clean api clean_api clean_apps artifacts clean_artifacts \
	nexus_headers nexus_print_options

.PHONY: ${PHONY}
.NOTPARALLEL: api apps artifacts

# App Stuff
APP_SRCS = \
	splash_main.c \
	splash_vdc_rulgen.c  \
	splash_setup_vdc.c \
	platformconfig.c \
	splash_bmp.c \
	splash_file.c
APP_OBJS = $(foreach file, $(patsubst %.c,splashgen/%.o,$(APP_SRCS)), $(file))

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

# This trick has to work in all three directories
B_THIS_DIR := BSEAV/app/splash/splashgen
REPO_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))

# Other useful directories
NEXUS_TOP := ${REPO_TOP}/nexus
SPLASH_TOP := ${REPO_TOP}/BSEAV/app/splash
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${REPO_TOP}/${B_REFSW_OBJ_DIR}
BIN_DIR := ${B_REFSW_OBJ_ROOT}/nexus/bin
ARTIFACT_DIR := ${B_REFSW_OBJ_ROOT}/BSEAV/app/splash

# Always build with debug
CFLAGS += -g
LDFLAGS += -g

# Apply the user's suffix everywhere
CFLAGS += -DSPLASH_DATA_SUFFIX=${suffix}


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
include $(REPO_TOP)/magnum/basemodules/mem/bmem.inc
include $(REPO_TOP)/magnum/basemodules/mma/bmma.inc
include $(REPO_TOP)/magnum/basemodules/int/bint.inc
include $(REPO_TOP)/magnum/basemodules/tmr/btmr.inc
include $(REPO_TOP)/magnum/portinginterface/i2c/bi2c.inc
include $(REPO_TOP)/magnum/commonutils/mrc/bmrc.inc
include $(REPO_TOP)/magnum/commonutils/box/bbox.inc
ifneq ($(SPLASH_NOHDM),y)
MAGNUM_DEFINES_T += SPLASH_SUPPORT_HDM
include $(REPO_TOP)/magnum/portinginterface/hdm/bhdm.inc
endif
ifneq ($(NEXUS_RFM_SUPPORT),n)
MAGNUM_DEFINES_T += SPLASH_SUPPORT_RFM
include $(REPO_TOP)/magnum/portinginterface/rfm/brfm.inc
endif
MAGNUM_INCLUDE_PATHS_T += \
	$(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
MAGNUM_INCLUDE_PATHS += $(addprefix -I,$(MAGNUM_INCLUDE_PATHS_T))
MAGNUM_DEFINES_T += \
	$(sort $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))
MAGNUM_DEFINES := $(addprefix -D,$(MAGNUM_DEFINES_T))
MAGNUM_CFLAGS := $(MAGNUM_INCLUDE_PATHS) $(MAGNUM_DEFINES)

# These won't get built by the api target
ONLY_MODULES=BVDC BRDC BFMT BSUR BAVC BPXL BMTH BBOX
ifneq ($(SPLASH_NOHDM),y)
ONLY_MODULES += BI2C BHDM
endif
ONLY_DEFINES_T := \
	$(sort $(foreach module, $(ONLY_MODULES), $($(module)_DEFINES)))
ONLY_DEFINES := $(addprefix -D,$(ONLY_DEFINES_T))
ONLY_DIR =  $(foreach module,$(ONLY_MODULES),$(dir $($(module)_SOURCES)))

# Final targets
all: api artifacts
	@echo "Final build in ${BIN_DIR}"
	@${MAKE} -C ${BIN_DIR} \
		suffix=${suffix} B_REFSW_MAKE_PART=nexus REPO_TOP=${REPO_TOP} \
		-f ${CURDIR}/Makefile ONLY_OBJ="${ONLY_OBJ}" $@
clean: clean_api clean_artifacts clean_apps

# Top level (applications) software: artifacts
artifacts: ${ARTIFACT_DIR}/splashgen/exists api
	@echo "Build intermediate files (artifacts) in ${ARTIFACT_DIR}"
	@${MAKE} -C ${ARTIFACT_DIR} \
		suffix=${suffix} REPO_TOP=${REPO_TOP} B_REFSW_MAKE_PART=BSEAV \
		ONLY_OBJ="${ONLY_OBJ}" ONLY_DEFINES="${ONLY_DEFINES}" \
		ONLY_SRC="${ONLY_SRC}" ONLY_DIR="${ONLY_DIR}" \
		MAGNUM_CFLAGS="${MAGNUM_CFLAGS}" \
		-f ${CURDIR}/Makefile $@
clean_artifacts: ${ARTIFACT_DIR}/splashgen/exists
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
api:
	@echo "Build nexus API"
	@$(MAKE) -C $(NEXUS_TOP)/build BREG_CAPTURE=y
clean_api:
	@echo "Clean nexus API"
	@$(MAKE) -C $(NEXUS_TOP)/build clean
nexus_print_options nexus_headers:
	$(MAKE) -C $(NEXUS_TOP)/build $@

${BIN_DIR}/exists ${ARTIFACT_DIR}/splashgen/exists:
	mkdir -p $@

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
# obj.nnnnn/BSEAV/app/splash.                                 #
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
CFLAGS += -I$(SPLASH_TOP)/splashgen/$(NEXUS_PLATFORM)
CFLAGS += -I$(SPLASH_TOP)/os/usermode
CFLAGS += -I$(SPLASH_TOP)/splashgen

# build with shrinked vdc for BOOTUPDATER
ifeq ($(B_PI_FOR_BOOTUPDATER),y)
CFLAGS += -DBVDC_FOR_BOOTUPDATER=1
CFLAGS += -DBFMT_FOR_BOOTUPDATER=1
CFLAGS += -DBFMT_PICK_eNTSC=1
CFLAGS += -DBFMT_PICK_e480p=1
#CFLAGS += -DBFMT_PICK_e720p=1
#CFLAGS += -DBFMT_PICK_e1080i=1
#CFLAGS += -DBFMT_PICK_e1080p=1
#CFLAGS += -DBFMT_PICK_e1080p_24Hz=1
#CFLAGS += -DBFMT_PICK_e1080p_30Hz=1
CFLAGS += -Os -fdata-sections -ffunction-sections -Wl,--gc-sections
#CFLAGS += -Os -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--print-gc-sections
endif

ifeq ($(NEXUS_PLATFORM),97435)
ifneq ($(NEXUS_PLATFORM_7435_1U4T),n)
CFLAGS += -DNEXUS_PLATFORM_7435_BOX_TYPE_1u4t=1
endif
endif

vpath %.c ${SPLASH_TOP}/splashgen
vpath %.c ${SPLASH_TOP}/common
vpath %.c ${SPLASH_TOP}/splashgen/$(NEXUS_PLATFORM)

CFLAGS += ${ONLY_DEFINES}

artifacts: ${APP_OBJS} ${ONLY_OBJ}

clean_artifacts:
	-$(RM) *.d *.o *.out splashgen/*.d splashgen/*.o splashgen/*.out

%.o: %.c
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)
splashgen/%.o: %.c
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)

-include *.d
-include splashgen/*.d

endif # B_REFSW_MAKE_PART
#################################
# End of location-specific code #
#################################
