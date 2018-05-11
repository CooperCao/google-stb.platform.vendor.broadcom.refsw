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
ifeq ($(URSR_TOP),)
#$(warning URSR_TOP is not defined! setting to ../../../../)
URSR_TOP := $(abspath ../../../../)
endif

NEXUS_TOP=$(URSR_TOP)/nexus
BSEAV_TOP      ?= $(URSR_TOP)/BSEAV
COMMON_DRM_TOP := $(BSEAV_TOP)/lib/security/common_drm

NEXUS_PLATFORM ?= $(PLATFORM)
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
COMMON_DRM_OBJ_ROOT = ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm
D_FOR_LIB_INSTALL = ${COMMON_DRM_OBJ_ROOT}/lib/${CMNDRM_DIR}

# Include nexus definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

# common_drm expects NEXUS_SECURITY_API_VERSION to be set to either '1' or '2'
ifndef NEXUS_SECURITY_API_VERSION
$(error NEXUS_SECURITY_API_VERSION is not defined.)
endif
ifneq ($(filter $(NEXUS_SECURITY_API_VERSION),1 2),$(NEXUS_SECURITY_API_VERSION))
$(error Unsupported NEXUS_SECURITY_API_VERSION = $(NEXUS_SECURITY_API_VERSION))
endif

include lib/prdy_libdir.inc

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

# Define job slots for parallel build dependencies
NJOBS ?= 1

include common_drm.inc

####################################
# Use URSR CFLAGS
###################################

# SRAI is always from URSR_TOP
ifeq ($(SAGE_SUPPORT),y)
    include $(URSR_TOP)/magnum/syslib/sagelib/bsagelib_public.inc
    SAGE_CFLAGS += $(addprefix -D,${BSAGELIB_DEFINES})
    SAGE_CFLAGS += -I${BSAGELIB_INCLUDES}
    SAGE_CFLAGS += -I$(URSR_TOP)/BSEAV/lib/security/sage/include
    SAGE_CFLAGS += -I$(URSR_TOP)/BSEAV/lib/security/sage/platforms/include
    SAGE_CFLAGS += -I$(URSR_TOP)/BSEAV/lib/security/sage/srai/include
    SAGE_CFLAGS += -DPLAYREADY_HOST_IMPL

    # set default Unified value
    #    Sage 2.x : always NO
    #    Sage 3.x : if common_drm ta binary is found, default to YES
    #               else default to NO
    ifeq ($(SAGE_VERSION), 2x)
      USE_UNIFIED_COMMON_DRM = y
    else
      CMNDRM_TA_BIN_RESULTS=$(wildcard $(BSEAV)/lib/security/sage/bin/$(BCHP_CHIP)$(BCHP_VER)/dev/sage_ta_common_drm*)
      ifeq (${CMNDRM_TA_BIN_RESULTS}, )
        USE_UNIFIED_COMMON_DRM ?= n
      else
        USE_UNIFIED_COMMON_DRM ?= y
      endif
    endif
endif

CFLAGS += -DPIC -fpic
CFLAGS += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS})
CFLAGS += -std=c99

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += $(COMMON_DRM_CFLAGS)

ifeq ($(USE_UNIFIED_COMMON_DRM), y)
    $(warning Using a Unified Common DRM implementation)
    CFLAGS += -DUSE_UNIFIED_COMMON_DRM
else
    $(warning Using a non-Unified Common DRM implementation)
endif

ifeq ($(ANDROID_BUILD),y)
ifeq ($(filter ${B_REFSW_ARCH}, mipsel-linux-android mips-linux-android mipsel-linux-uclibc), ${B_REFSW_ARCH})
CFLAGS += -EL -DANDROID
else
CFLAGS += -DANDROID -ffreestanding
endif # end of B_REFSW_ARCH
CFLAGS += -I$(ANDROID)/external/openssl/include
CFLAGS += -I$(ANDROID)/vendor/broadcom/refsw/nexus/extensions/security/usercmd/include/40nm
CFLAGS += -I$(ANDROID)/vendor/broadcom/refsw/nexus/extensions/security/otpmsp/include/40nm
CFLAGS += -I$(ANDROID)/vendor/broadcom/refsw/nexus/extensions/security/keyladder/include/40nm
CFLAGS += -I$(ANDROID)/vendor/broadcom/refsw/nexus/extensions/security/securersa/include
ifneq ($(ANDROID_LINKER_SYSROOT),)
PRDY_LDFLAGS += --sysroot=$(ANDROID_LINKER_SYSROOT)
endif # end of ANDROID_LINKER_SYSROOT
PRDY_LDFLAGS += -lnexus -llog

PRDY_LIB_PATH := ${URSR_TOP}/BSEAV/${CMNDRM_PRDY_LIBDIR}

PRDY_LDFLAGS += -L${PRDY_LIB_PATH}

ifeq ($(SAGE_SUPPORT),y)
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
  PRDY_LDFLAGS += -lplayreadypk64_host
else ifeq $(filter ${B_REFSW_ARCH}, arm-linux), ${B_REFSW_ARCH})
  PRDY_LDFLAGS += -lplayreadypk_host
endif
else
  PRDY_LDFLAGS += -lplayreadypk
endif # end of SAGE_SUPPORT
PRDY_LDFLAGS += -ldrmrootfs

else # end of ANDROID_BUILD=y
CFLAGS += $(B_OPENSSL_CFLAGS)
PRDY_LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR} -ldrmrootfs
endif # end of ANDROID_BUILD=n

#allow c++ style comments
CFLAGS += -std=c99

ifeq ($(CMNDRM_SKIP_BINFILE_OVERWRITE), y)
$(warning Common DRM TL will not overwrite Type 1 or Type 2 DRM bin files on the rootfs)
CFLAGS += -DCMNDRM_SKIP_BINFILE_OVERWRITE=1
endif

########################################
# Determine core size and ZEUS version
########################################
include ${NEXUS_TOP}/modules/security/security_defs.inc

############################################################################
# Go through all <DRM>_SAGE variables from config.inc and include those
# <drm>.inc files
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
ifeq ($(SAGE_SUPPORT),y)
	ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR), 4)
		include config/config_zeus4x.inc
	endif

	ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR), 3)
		include config/config_zeus30.inc
	endif

	ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR), 5)
		include config/config_zeus5x.inc
	endif
else
	include config/config_no_sage.inc
endif

#################### COMMON ################################
include ${URSR_TOP}/BSEAV/lib/security/common_drm/drm/common/common.inc
COMMON_DRM_SOURCES += ${COMMON_SOURCES}
COMMON_DRM_DEFINES += ${COMMON_DEFINES}

# always include the core TL files ???
ifeq ($(SAGE_SUPPORT),y)
include drm_tl/common_tl/common_tl.inc
COMMON_DRM_TL_SOURCES += ${COMMON_TL_SOURCES}
COMMON_DRM_TL_INCLUDES += ${COMMON_TL_INCLUDES}
COMMON_DRM_TL_DEFINES += ${COMMON_TL_DEFINES}
endif

#################### ADOBE ################################
ifeq (${ADOBE_NONSAGE},ON)
ifeq (${ENABLE_ADOBE},y)
ifeq (${COMMON_DRM_ADOBE},y)
include drm/adobe/adobe.inc
COMMON_DRM_SOURCES += ${ADOBE_SOURCES}
COMMON_DRM_DEFINES += ${ADOBE_DEFINES}
endif #COMMON_DRM_ADOBE
endif #ENABLE_ADOBE
endif #COMMON_DRM_ADOBE

ifeq (${ADOBE_SAGE},ON)
include drm_tl/adobe/adobe.inc
COMMON_DRM_TL_INCLUDES += ${ADOBE_INCLUDES}
COMMON_DRM_TL_SOURCES += ${ADOBE_SOURCES}
COMMON_DRM_TL_DEFINES += ${ADOBE_DEFINES}
endif

################### WIDEVINE ##############################
# WIDEVINE CENC (for non-SAGE)
ifeq (${WVOEMCRYPTO_SAGE},OFF)
ifeq (${COMMON_DRM_WIDEVINE},y)
include drm/widevine_cenc/widevine_cenc.inc
COMMON_DRM_INCLUDES += ${WIDEVINE_CENC_INCLUDES}
COMMON_DRM_SOURCES += ${WIDEVINE_CENC_SOURCES}
COMMON_DRM_DEFINES += ${WIDEVINE_CENC_DEFINES}
endif #COMMON_DRM_WIDEVINE
else
# WIDEVINE MODULAR DRM (for SAGE)
include drm_tl/wvoemcrypto/wvoemcrypto.inc
COMMON_DRM_TL_INCLUDES += ${WVOEMCRYPTO_INCLUDES}
COMMON_DRM_TL_SOURCES += ${WVOEMCRYPTO_SOURCES}
COMMON_DRM_TL_DEFINES += ${WVOEMCRYPTO_DEFINES}
endif

###################### Playback ################################
ifeq (${PLAYBACK_SAGE},OFF)
else
include drm_tl/playback/playback.inc
COMMON_DRM_TL_INCLUDES += ${PLAYBACK_INCLUDES}
COMMON_DRM_TL_SOURCES += ${PLAYBACK_SOURCES}
COMMON_DRM_TL_DEFINES += ${PLAYBACK_DEFINES}
endif

#################### DTCP-IP ################################
# non-SAGE DTCPIP can be supported on non-SAGE platforms and Zeus4x
ifeq (${DTCP_IP_NONSAGE},ON)
ifeq (${COMMON_DRM_DTCPIP},y)
include drm/dtcp_ip/dtcp_ip.inc
COMMON_DRM_SOURCES += ${DTCP_IP_SOURCES}
COMMON_DRM_DEFINES += ${DTCP_IP_DEFINES}
endif #COMMON_DRM_DTCPIP
endif

# SAGE DTCPIP support for SAGE platforms
ifeq (${DTCP_IP_SAGE},ON)
include drm_tl/dtcp_ip/dtcp_ip.inc
COMMON_DRM_TL_INCLUDES += ${DTCP_IP_TL_INCLUDES}
COMMON_DRM_TL_SOURCES += ${DTCP_IP_TL_SOURCES}
COMMON_DRM_TL_DEFINES += ${DTCP_IP_TL_DEFINES}
endif

#################### EDRM ################################
ifeq (${EDRM_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
include drm_tl/edrm/edrm.inc
COMMON_DRM_TL_INCLUDES += ${EDRM_INCLUDES}
COMMON_DRM_TL_SOURCES += ${EDRM_SOURCES}
COMMON_DRM_TL_DEFINES += ${EDRM_DEFINES}
endif

#################### HDCP ################################
ifeq (${HDCP_SAGE},OFF)
include drm/hdcp/hdcp.inc
COMMON_DRM_SOURCES += ${HDCP_SOURCES}
COMMON_DRM_DEFINES += ${HDCP_DEFINES}
else
#$(warning Support for HDCP on SAGE does not exist)
endif

#################### OCAP ################################
ifeq (${OCAP_SAGE},OFF)
ifeq (${COMMON_DRM_OCAP},y)
include drm/ocap/ocap.inc
COMMON_DRM_SOURCES += ${OCAP_SOURCES}
COMMON_DRM_DEFINES += ${OCAP_DEFINES}
endif #COMMON_DRM_OCAP
else
#$(warning Support for OCAP on SAGE does not exist)
endif

#################### NETFLIX ################################
ifeq (${NETFLIX_SAGE},OFF)
ifeq (${COMMON_DRM_NETFLIX},y)
include drm/netflix/netflix.inc
COMMON_DRM_SOURCES += ${NETFLIX_SOURCES}
COMMON_DRM_DEFINES += ${NETFLIX_DEFINES}
endif #COMMON_DRM_NETFLIX
else
include drm_tl/netflix/netflix.inc
COMMON_DRM_TL_INCLUDES += ${NETFLIX_INCLUDES}
COMMON_DRM_TL_SOURCES += ${NETFLIX_SOURCES}
COMMON_DRM_TL_DEFINES += ${NETFLIX_DEFINES}
endif

#################### PMC ################################
ifeq (${PMC_SAGE},OFF)
ifeq (${ENABLE_ADOBE},y) # PMC only supported with Adobe
ifeq (${COMMON_DRM_ADOBE},y) # Confirms that all Adobe and PMC components are present
include drm/pmc/pmc.inc
COMMON_DRM_SOURCES += ${PMC_SOURCES}
COMMON_DRM_DEFINES += ${PMC_DEFINES}
endif #COMMON_DRM_ADOBE
else
#$(warning PMC support requires Adobe since all other PMC modules have been deprecated !!!)
endif #ENABLE_ADOBE
else
#$(warning Support for PMC on SAGE does not exist)
endif

#############################################################

CFLAGS += ${COMMON_DRM_DEFINES}

########################### Playready 2.5 ##################################
include drm/playready/playready.inc
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
DRM_BUILD_PROFILE   := 900
CFLAGS              += -DDRM_BUILD_PROFILE=$(DRM_BUILD_PROFILE)
CFLAGS              += ${NXCLIENT_CFLAGS}
ifeq ($(SAGE_VERSION), 2x)
CFLAGS              += -I$(URSR_TOP)/BSEAV/thirdparty/playready/2.5/inc/2x -DTARGET_LITTLE_ENDIAN=1
else
CFLAGS              += -I$(URSR_TOP)/BSEAV/thirdparty/playready/2.5/inc -DTARGET_LITTLE_ENDIAN=1
endif
CFLAGS              += -DTARGET_SUPPORTS_UNALIGNED_DWORD_POINTERS=1

PRDY_BASENAME       := cmndrmprdy
PRDY_PUBLIC_LIBS    += lib${PRDY_BASENAME}.a lib${PRDY_BASENAME}.so
PRDY_OBJS           := $(patsubst %.c,%.o, ${PLAYREADY_SOURCES})

PRDY_FOR_LIB_INSTALL:= ${CMNDRM_PRDY_LIBDIR}
PRDY_INSTALLED_LIBS := $(addprefix ${COMMON_DRM_OBJ_ROOT}/${PRDY_FOR_LIB_INSTALL}/, ${PRDY_PUBLIC_LIBS})

ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), ${B_REFSW_ARCH})
CFLAGS +=-marm -march=armv7-a
endif

ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
CFLAGS += -march=armv8-a -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast
endif

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##


#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##

ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
LIB_BASENAME        := cmndrm64
else
LIB_BASENAME        := cmndrm
endif

LIB_TL_BASENAME     := cmndrm_tl

###########################################################################
## LDFLAGS section
###########################################################################
LDFLAGS = -L$(NEXUS_BIN_DIR)/ -lnexus${NEXUS_LIB_SUFFIX}

# TODO: uncheck when shared OpenSSL library is available
ifeq ($(ANDROID_BUILD),y)
ifneq ($(ANDROID_LINKER_SYSROOT),)
LDFLAGS += --sysroot=$(ANDROID_LINKER_SYSROOT)
endif # end of ANDROID_LINKER_SYSROOT
LDFLAGS += -lcrypto -ldrmrootfs -lbcrypt
LDFLAGS += $(NEXUS_LDFLAGS)
else # end of ANDROID_BUILD=y
LDFLAGS += -L$(B_OPENSSL_LIB_INSTALL_DIR) -lcrypto
endif # end of ANDROID_BUILD=n

# Link against drmrootfs
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR} -ldrmrootfs

#TL lib should always link against regular libcmndrm.so
LDFLAGS += $(COMMON_DRM_LDFLAGS)

# make sure SONAME is setup for ELF in android.
LDSHARED_FLAGS := -shared
ifeq ($(ANDROID_BUILD),y)
LDSHARED_FLAGS += -Wl,-soname,lib${LIB_BASENAME}.so
endif

############
# You probably shouldn't have to modify the text below this point.
#####
F_PUBLIC_LIBS    	+= lib${LIB_BASENAME}.a lib${LIB_BASENAME}.so
COMMON_DRM_OBJECTS  :=  $(patsubst %.c,%.o, ${COMMON_DRM_SOURCES})
TARGET_COMMON_DRM_OBJECTS = $(addprefix ${COMMON_DRM_OBJ_ROOT}/, ${COMMON_DRM_OBJECTS})
F_INSTALLED_LIBS	:= $(addprefix ${COMMON_DRM_OBJ_ROOT}/lib/${CMNDRM_DIR}/, ${F_PUBLIC_LIBS})

ifeq ($(SAGE_SUPPORT),y)
F_PUBLIC_TL_LIBS   	= lib${LIB_TL_BASENAME}.a lib${LIB_TL_BASENAME}.so
COMMON_DRM_TL_OBJECTS :=  $(patsubst %.c,%.o, ${COMMON_DRM_TL_SOURCES})
F_INSTALLED_TL_LIBS	= $(addprefix ${COMMON_DRM_OBJ_ROOT}/lib/${CMNDRM_DIR}/, ${F_PUBLIC_TL_LIBS})
else #clear the TL flags so that non-sage platforms don't try to build certain targets
F_PUBLIC_TL_LIBS   	=
COMMON_DRM_TL_OBJECTS :=
F_INSTALLED_TL_LIBS	=
endif
BUILD_LIBS = $(addprefix ${COMMON_DRM_OBJ_ROOT}/, ${F_PUBLIC_TL_LIBS})

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##

#$(warning TL_LDFLAGS = ${TL_LDFLAGS})
#$(warning LDFLAGS = ${LDFLAGS})
#$(warning F_INSTALLED_LIBS = ${F_INSTALLED_LIBS})
#$(warning D_FOR_LIB_INSTALL = ${D_FOR_LIB_INSTALL})
#$(warning COMMON_DRM_OBJECTS = ${COMMON_DRM_OBJECTS})
#$(warning TARGET_COMMON_DRM_OBJECTS = ${TARGET_COMMON_DRM_OBJECTS})
#$(warning COMMON_DRM_TL_OBJECTS = ${COMMON_DRM_TL_OBJECTS})
#$(warning COMMON_DRM_SOURCES = ${COMMON_DRM_SOURCES})
#$(warning COMMON_DRM_TL_SOURCES = ${COMMON_DRM_TL_SOURCES})
#$(warning CFLAGS = ${CFLAGS})

############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
ifeq ($(ANDROID_BUILD),y)
all: $(BUILD_LIBS)
else
all: prep_folder prep_nexus prep_openssl prep_bcrypt prep_drmrootfs prep_srai $(BUILD_LIBS)
endif

.PHONY: prep_folder prep_openssl prep_srai prep_drmrootfs playready

prep_folder:
	@ mkdir -p ${COMMON_DRM_OBJ_ROOT}/
	@ mkdir -p ${D_FOR_LIB_INSTALL}/
	@ mkdir -p $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $(dir ${COMMON_SOURCES}))
	@ mkdir -p $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $(dir ${PLAYREADY_SOURCES} ${COMMON_DRM_TL_SOURCES}))
	@ mkdir -p ${COMMON_DRM_OBJ_ROOT}/${PRDY_FOR_LIB_INSTALL}
ifeq ($(SAGE_SUPPORT),y)
	@ mkdir -p ${COMMON_DRM_OBJ_ROOT}/lib/${CMNDRM_DIR}
endif

ifeq (${ADOBE_NONSAGE},ON)
ifeq (${PMC_SAGE},OFF)
ifeq (${ENABLE_ADOBE},y)
ifeq (${COMMON_DRM_ADOBE},y)
	@ mkdir -p $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $(dir ${ADOBE_SOURCES} ${PMC_SOURCES}))
endif
endif
endif
endif

ifeq (${WVOEMCRYPTO_SAGE},OFF)
ifeq (${COMMON_DRM_WIDEVINE},y)
	@ mkdir -p $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $(dir ${WIDEVINE_CENC_SOURCES}))
endif
endif

ifeq (${DTCP_IP_NONSAGE},ON)
ifeq (${COMMON_DRM_DTCPIP},y)
	@ mkdir -p $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $(dir ${DTCP_IP_SOURCES}))
endif
endif

ifeq (${HDCP_SAGE},OFF)
	@ mkdir -p $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $(dir ${HDCP_SOURCES}))
endif

ifeq (${OCAP_SAGE},OFF)
ifeq (${COMMON_DRM_OCAP},y)
	@ mkdir -p $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $(dir ${OCAP_SOURCES}))
endif
endif

ifeq (${NETFLIX_SAGE},OFF)
ifeq (${COMMON_DRM_NETFLIX},y)
	@ mkdir -p $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $(dir ${NETFLIX_SOURCES}))
endif
endif

prep_nexus:
	@if [ ! -e ${NEXUS_BIN_DIR}/libnexus.so ]; then \
		echo [ building nexus .... ] && \
		$(MAKE) -j$(NJOBS) -C $(NEXUS_TOP)/build >/dev/null; \
	fi

include $(BSEAV)/opensource/openssl/b_openssl_targets.inc
ifeq ($(B_BUILD_DEPS),n)
 prep_openssl:
else
 prep_openssl: b_openssl_target_build
endif

prep_bcrypt:
	@echo [Compile and install Bcrypt dependency ....] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/bcrypt install copy > /dev/null

include $(BSEAV)/lib/drmrootfs/b_drmrootfs_targets.inc
ifeq ($(B_BUILD_DEPS),n)
 prep_drmrootfs:
else
 prep_drmrootfs: b_drmrootfs_target_build
endif

include $(BSEAV)/lib/security/sage/srai/b_srai_targets.inc
ifeq ($(B_BUILD_DEPS),n)
 prep_srai:
else
 prep_srai: b_srai_target_build
endif

playready: prep_folder prep_drmrootfs prep_playready ${PRDY_INSTALLED_LIBS}

prep_playready: $(addprefix ${COMMON_DRM_OBJ_ROOT}/, ${PRDY_PUBLIC_LIBS})

# Order-only prerequisites
${PRDY_INSTALLED_LIBS}: |prep_playready
$(addprefix ${COMMON_DRM_OBJ_ROOT}/, ${PRDY_PUBLIC_LIBS}): |prep_folder prep_drmrootfs

clean:
	@rm -rf ${COMMON_DRM_OBJ_ROOT}
	$(MAKE) -C ${URSR_TOP}/BSEAV/lib/security/sage/srai clean

report:
	@echo "NEXUS CFLAGS is $(NEXUS_CFLAGS)"
	@echo "TL_FLAGS is $(TL_LDFLAGS)"
	@echo "LDFLAGS is $(LDFLAGS)"
	@echo "CC: $(CC)"
	@echo "toolchain version: $(GCCVERSION)"
	@echo "$(shell gcc --version | grep ^gcc | sed 's/^.* //g')"

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##


############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
ifeq ($(SAGE_SUPPORT),y)
${COMMON_DRM_OBJ_ROOT}/lib${LIB_TL_BASENAME}.so: $(addprefix ${COMMON_DRM_OBJ_ROOT}/, ${COMMON_DRM_TL_OBJECTS})
	@ echo "[Link...... $(notdir $@) (common_drm)]"
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	@ ${CC} -shared -o $@ ${TL_LDFLAGS} ${LDFLAGS} $^ -Wl,-Bdynamic
else
	@ ${CC} -shared -o $@ ${TL_LDFLAGS} $^ -Wl,-dy
endif
endif

ifeq ($(SAGE_SUPPORT),y)
${COMMON_DRM_OBJ_ROOT}/lib${LIB_TL_BASENAME}.a: $(addprefix ${COMMON_DRM_OBJ_ROOT}/, ${COMMON_DRM_TL_OBJECTS})
	@ echo "[Archive... $(notdir $@) (common_drm)]"
	@ ${AR} rc $@ $^
endif

${COMMON_DRM_OBJ_ROOT}/%.o: %.c |prep_folder
	@ echo "[Compile... $(notdir $<) (common_drm)]"
ifeq ($(SAGE_SUPPORT),y)
	@ ${CC} ${CFLAGS} ${SAGE_CFLAGS} ${COMMON_DRM_TL_DEFINES} -c -o $@ $<
else
	@ ${CC} ${CFLAGS} -c -o $@ $<
endif

${D_FOR_LIB_INSTALL}/lib${LIB_BASENAME}.so: ${TARGET_COMMON_DRM_OBJECTS}
	@ echo [Linking shared library .... $@ ]
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	@ ${CC} ${LDSHARED_FLAGS} -o $@  ${LDFLAGS}  $^ -Wl,-Bdynamic
else
	@ ${CC} ${LDSHARED_FLAGS} -o $@  ${LDFLAGS}  $^ -Wl,-dy
endif

${D_FOR_LIB_INSTALL}/lib${LIB_BASENAME}.a: ${TARGET_COMMON_DRM_OBJECTS}
	@ echo [Linking static library .... $@ ]
	@ ${AR} rc  $@  $^

${COMMON_DRM_OBJ_ROOT}/lib${PRDY_BASENAME}.so: $(addprefix ${COMMON_DRM_OBJ_ROOT}/, ${PRDY_OBJS})
	@ echo [Linking shared library .... $@ ]
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	@ ${CC} -v -shared -o $@ ${LD_SYSROOT} ${PRDY_LDFLAGS} $^ -Wl,--no-whole-archive -Wl,-Bdynamic
else
	@ ${CC} -v -shared -o $@ ${LD_SYSROOT} ${PRDY_LDFLAGS} $^ -Wl,--no-whole-archive -Wl,-dy
endif

${COMMON_DRM_OBJ_ROOT}/lib${PRDY_BASENAME}.a: $(addprefix ${COMMON_DRM_OBJ_ROOT}/, ${PRDY_OBJS})
	@ echo "[Archive... $(notdir $@) (common_drm)]"
	@ ${AR} rc $@ $^

# Install a copy of cmndrmprdy libs in the out of source build folder
install: all ${F_INSTALLED_LIBS} ${F_INSTALLED_TL_LIBS}

# all must be done before installing (order-only dependency)
${F_INSTALLED_LIBS} ${F_INSTALLED_TL_LIBS}: |all

${COMMON_DRM_OBJ_ROOT}/lib/${CMNDRM_DIR}/%.a : ${COMMON_DRM_OBJ_ROOT}/%.a
	@install -m 0755 $< $@

${COMMON_DRM_OBJ_ROOT}/lib/${CMNDRM_DIR}/%.so : ${COMMON_DRM_OBJ_ROOT}/%.so
	@install -m 0755 $< $@

${COMMON_DRM_OBJ_ROOT}/${PRDY_FOR_LIB_INSTALL}/%.a : ${COMMON_DRM_OBJ_ROOT}/%.a
	@install -m 0755 $< $@

${COMMON_DRM_OBJ_ROOT}/${PRDY_FOR_LIB_INSTALL}/%.so : ${COMMON_DRM_OBJ_ROOT}/%.so
	@install -m 0755 $< $@

copy: all
	@if [ -f ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/lib/${CMNDRM_DIR}/libcmndrm.so ]; then \
		install -m 0755 ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/lib/${CMNDRM_DIR}/libcmndrm.so $(NEXUS_BIN_DIR); \
	fi;
	@if [ -f ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/lib/${CMNDRM_DIR}/libcmndrm64.so ]; then \
		install -m 0755 ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/lib/${CMNDRM_DIR}/libcmndrm64.so $(NEXUS_BIN_DIR); \
	fi;
	@if [ -e ${COMMON_DRM_OBJ_ROOT}/libcmndrmprdy.so ]; then \
		install -m 0755 ${COMMON_DRM_OBJ_ROOT}/libcmndrmprdy.so $(NEXUS_BIN_DIR); \
	fi;
	@if [ -e ${COMMON_DRM_OBJ_ROOT}/libcmndrm_tl.so ]; then \
		install -m 0755 ${COMMON_DRM_OBJ_ROOT}/libcmndrm_tl.so $(NEXUS_BIN_DIR); \
	fi
