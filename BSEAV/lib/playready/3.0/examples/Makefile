############################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#
############################################################

ifeq ($(URSR_TOP),)
	$(warning URSR_TOP is not defined. Establising relative paths to nexus and BSEAV)
	NEXUS_TOP := $(shell cd ../../../../../nexus; pwd)
	BSEAV_TOP := $(shell cd ../../../../../BSEAV; pwd)
else
	NEXUS_TOP := $(URSR_TOP)/nexus
	BSEAV_TOP := $(URSR_TOP)/BSEAV
endif

$(warning target = $(target))

ifeq ($(target), )
	$(error Makefile usage:  make target=<directory> (i.e. make target=prdy30_svp))
else
	include ./$(target)/$(target)_app.inc
	TARGET_DIR := BSEAV/lib/playready/3.0/examples
endif

# Set directory for installing lib based on architecture
ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), ${B_REFSW_ARCH})
	ARCH_DIR_PRDY=arm
else
        # Distinguish between 65nm-askm and 40nm MIPS products
	ifeq ($(filter $(BCHP_CHIP), 7400 7401 7405 7413 7325 7335 7125 7340 7342 7410 7420 7468), $(BCHP_CHIP))
	ARCH_DIR_PRDY=65nm-askm
else
	ARCH_DIR_PRDY=mips
endif
endif

ifneq ($(SAGE_SUPPORT),y)
$(error Playready 3.0 does not currently work without Sage)
endif

APPS += $(DRM_APPS)
SRCS = $(DRM_APP_SRCS)
OBJS = ${SRCS:.c=.o}

NEXUS_PLATFORM ?= $(PLATFORM)

# Include nexus definitions
include $(URSR_TOP)/nexus/platforms/common/build/nexus_platforms.inc
include $(URSR_TOP)/nexus/platforms/common/build/platform_app.inc
include $(URSR_TOP)/nexus/build/nexus_defs.inc

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

# create 'target' dir in out of source dir
$(shell mkdir -p ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target})

include $(BSEAV_TOP)/lib/security/common_crypto/common_crypto.inc

ifeq ($(SAGE_SUPPORT),y)
	include $(BSEAV_TOP)/lib/security/sage/srai/srai.inc
endif


TARGET_OBJS = $(addprefix  ${target}/, ${OBJS})
APP_BINS = $(APPS)
#$(warning APP_BINS = $(APP_BINS))
INSTALLED_APPS = $(addprefix  ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}/, ${APPS})
CLEAN_APPS = $(addprefix  $(NEXUS_BIN_DIR)/, ${APPS})
#$(warning INSTALLED_APPS = ${INSTALLED_APPS})
#$(warning CLEAN_APPS = ${CLEAN_APPS})

all:  $(APPS) copy


# For linux builds, link to the correct libraries
LDFLAGS :=
LDFLAGS += -L$(NEXUS_BIN_DIR)/ -lpthread -lm -lnexus -ldrmrootfs -lprdyhttp -lprdypiff

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))


include $(NEXUS_TOP)/../BSEAV/lib/drmrootfs/drmrootfs_libdir.inc

CFLAGS += $(addprefix -I,$(COMMON_CRYPTO_INCLUDES))
CFLAGS += -I$(BSEAV_TOP)/lib/drmrootfs

CFLAGS += $(DRM_APP_CFLAGS)
LDFLAGS += $(DRM_APP_LDFLAGS)


PLAYREADY_LIB_PATH :=
ifeq ($(SAGE_SUPPORT),y)
	LDFLAGS += $(SRAI_LDFLAGS)
	CFLAGS += -I$(BSEAV_TOP)/../magnum/syslib/sagelib/include
	CFLAGS += -I$(BSEAV_TOP)/lib/security/sage/srai/include
	LDFLAGS +=  -L${BSEAV_TOP}/lib/playready/3.0/bin/${ARCH_DIR_PRDY}/lib -lplayready30pk
	PLAYREADY_LIB_PATH = ${CP} -pv ${BSEAV}/lib/playready/3.0/bin/${ARCH_DIR_PRDY}/lib/libplayready30pk.so  ${NEXUS_BIN_DIR}
endif



# This cleans nexus and local apps
clean:
	rm -rf ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}
	rm -f ${CLEAN_APPS}


APP_INCLUDES += ${DRM_APP_INCLUDES}

# Always build with debug
CFLAGS += -g
CFLAGS += -std=c99 -finline-functions

include $(BSEAV_TOP)/lib/utils/batom.inc
include $(BSEAV_TOP)/lib/bfile/bfile.inc
include $(BSEAV_TOP)/lib/media/bmedia.inc

CFLAGS += $(addprefix -I,$(BATOM_INCLUDES))
CFLAGS += $(addprefix -I,$(BMEDIA_INCLUDES))
CFLAGS += $(addprefix -I,$(BFILE_INCLUDES))

include $(BSEAV_TOP)/lib/playready/http/prdyhttp.inc
include $(BSEAV_TOP)/lib/playready/piff/prdypiff.inc


CFLAGS += $(addprefix -I,$(BMEDIA_INCLUDES))
CFLAGS += $(addprefix -I,$(PRDY_HTTP_INCLUDES))
CFLAGS += $(addprefix -I,$(PRDY_PIFF_INCLUDES))

$(info PRDY_PIFF_INCLUDES ,$(PRDY_PIFF_INCLUDES))

LDFLAGS += $(PRDY_HTTP_LDFLAGS)


CFLAGS += -I$(BSEAV_TOP)/lib/playready/3.0/inc

# PLayready SDK flags
DRM_BUILD_PROFILE   := 900
CFLAGS              += -DDRM_BUILD_PROFILE=$(DRM_BUILD_PROFILE)
CFLAGS              += -DTARGET_LITTLE_ENDIAN=1
CFLAGS              += -DTARGET_SUPPORTS_UNALIGNED_DWORD_POINTERS=1

# Implicit rule for building local apps
$(APPS): prep_nexus prep_drmrootfs prep_srai prep_http prep_piff $(TARGET_OBJS)
#$(APPS): prep_prdy prep_nexus prep_drmrootfs prep_srai prep_http prep_piff $(TARGET_OBJS)
	@echo [Link... $@]
	${Q_} $(CC) -o ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}/$@ ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}/$@.o $(LDFLAGS)

prep_drmrootfs:
	echo [..... prep_drmrootfs];
	$(Q)$(MAKE) -C $(BSEAV_TOP)/lib/drmrootfs install > /dev/null
	${CP} -pv ${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR}/libdrmrootfs.so ${NEXUS_BIN_DIR}

prep_srai:
ifeq ($(SAGE_SUPPORT),y)
	echo [..... prep_srai];
	$(Q)$(MAKE) -C ../../../security/sage/srai re
endif

prep_http:
	echo [..... prep_http];
	$(Q)$(MAKE) -C $(BSEAV_TOP)/lib/playready/http re

prep_piff:
	echo [..... prep_piff];
	$(Q)$(MAKE) -C $(BSEAV_TOP)/lib/playready/piff re

prep_nexus:
	@if [ ! -e $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/nexus/bin/nxserver ]; then \
		echo [ building nxclient .... ]; \
		$(MAKE) -C $(NEXUS_TOP)/nxclient >/dev/null; \
	fi
	@if [ ! -e $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/nexus/bin/libnexus.so ]; then \
		echo [ building nexus .... ]; \
		$(MAKE) -C $(NEXUS_TOP)/build >/dev/null; \
	fi

#prep_prdy:
#	$(MAKE) -C ${PRDY_TOP}/3.0/source clean all >/dev/null; \
#	$(MAKE) -C ${PRDY_TOP}/3.0/source/linux/libraries clean all >/dev/null; \


${target}/%.o: ${target}/%.c
	@ echo [Compiling  with $(CC).... $< ]
	@${CC} ${CFLAGS} -c -o  ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/$@ $<

copy:
	@echo [Copying executables...]
	cp -f $(INSTALLED_APPS) $(NEXUS_BIN_DIR)
ifeq ($(SAGE_SUPPORT),y)
	cp -f ${BSEAV}/lib/playready/3.0/bin/${ARCH_DIR_PRDY}/lib/libplayready30pk.so  ${NEXUS_BIN_DIR}
endif
