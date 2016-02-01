#############################################################################
#    (c)2011-2015 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
#
#############################################################################
ifeq ($(URSR_TOP),)
$(warning URSR_TOP is not defined! setting to ../../../../)
URSR_TOP := ../../../../
endif

NEXUS_TOP=$(URSR_TOP)/nexus
COMMON_DRM_TOP := $(URSR_TOP)/BSEAV/lib/security/common_drm

NEXUS_PLATFORM ?= $(PLATFORM)
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
COMMON_DRM_OBJ_ROOT = ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm



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


# Set directory based on architecture
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux arm-linux), ${B_REFSW_ARCH})
ARCH_DIR=arm
else
ARCH_DIR=mips
endif

include common_drm.inc


# create 'target' dir in out of source dir
$(shell mkdir -p ${COMMON_DRM_OBJ_ROOT}/drm/playready)

####################################
# Use URSR CFLAGS
###################################



# SRAI is always from URSR_TOP
ifeq ($(SAGE_SUPPORT),y)
	include $(URSR_TOP)/magnum/syslib/sagelib/bsagelib_public.inc
	SAGE_CFLAGS += -I${BSAGELIB_INCLUDES}
	SAGE_CFLAGS += -I$(URSR_TOP)/BSEAV/lib/security/sage/include
	SAGE_CFLAGS += -I$(URSR_TOP)/BSEAV/lib/security/sage/platforms/include
	SAGE_CFLAGS += -I$(URSR_TOP)/BSEAV/lib/security/sage/srai/include
endif



CFLAGS += -DPIC -fpic
CFLAGS += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS})
CFLAGS += -std=c99

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += $(COMMON_DRM_CFLAGS)



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
PRDY_LDFLAGS += -L${URSR_TOP}/BSEAV/lib/playready/2.5/bin/${ARCH_DIR}/lib
ifeq ($(SAGE_SUPPORT),y)
PRDY_LDFLAGS += -lplayreadypk_host
else
PRDY_LDFLAGS += -lplayreadypk
endif # end of SAGE_SUPPORT
PRDY_LDFLAGS += -ldrmrootfs
else # end of ANDROID_BUILD=y
CFLAGS += $(B_OPENSSL_CFLAGS)
PRDY_LDFLAGS += -L$(URSR_TOP)/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR} -ldrmrootfs
endif # end of ANDROID_BUILD=n


#allow c++ style comments
CFLAGS += -std=c99

ifeq ($(CMNDRM_SKIP_BINFILE_OVERWRITE), y)
$(warning Common DRM TL will not overwrite Type 1 or Type 2 DRM bin files on the rootfs)
CFLAGS += -DCMNDRM_SKIP_BINFILE_OVERWRITE=1
endif

############################################################################
# Go through all <DRM>_SAGE variables from config.inc and include those
# <drm>.inc files
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
ifeq ($(SAGE_SUPPORT),y)
ifeq ($(findstring $(BCHP_CHIP), 7445 7366 7439 7364 7250 74371), $(BCHP_CHIP))
include config/config_zeus4x.inc
else ifeq ($(findstring $(BCHP_CHIP), 7435 7584 75845), $(BCHP_CHIP))
include config/config_zeus30.inc
endif
else
include config/config_no_sage.inc
endif

# always include the core TL files ???
ifeq ($(SAGE_SUPPORT),y)
include drm_tl/common_tl/common_tl.inc
COMMON_DRM_TL_SOURCES += ${COMMON_TL_SOURCES}
COMMON_DRM_TL_INCLUDES += ${COMMON_TL_INCLUDES}
COMMON_DRM_TL_DEFINES += ${COMMON_TL_DEFINES}
endif

#################### ADOBE ################################
ifeq (${ADOBE_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
include drm_tl/adobe/adobe.inc
COMMON_DRM_TL_INCLUDES += ${ADOBE_INCLUDES}
COMMON_DRM_TL_SOURCES += ${ADOBE_SOURCES}
COMMON_DRM_TL_DEFINES += ${ADOBE_DEFINES}
endif

######################widevine modular DRM################################
ifeq (${WVOEMCRYPTO_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
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


#################### CUSTOM PRIVATE ################################
ifeq (${CUSTOM_PRIVATE_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
$(error Support for CustomPrivate on SAGE does not exist)
endif

COMMON_DRM_INCLUDES += ${CUSTOM_PRIVATE_INCLUDES}
COMMON_DRM_SOURCES += ${CUSTOM_PRIVATE_SOURCES}
COMMON_DRM_DEFINES += ${CUSTOM_PRIVATE_DEFINES}

#################### DATAPROC ################################
ifeq (${DATAPROC_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
	$(error Support for DataProc on SAGE does not exist)
endif

COMMON_DRM_INCLUDES += ${DATAPROC_INCLUDES}
COMMON_DRM_SOURCES += ${DATAPROC_SOURCES}
COMMON_DRM_DEFINES += ${DATAPROC_DEFINES}

#################### DTCP-IP ################################
ifeq (${DTCP_IP_SAGE},OFF)
COMMON_DRM_INCLUDES += ${DTCP_IP_INCLUDES}
COMMON_DRM_SOURCES += ${DTCP_IP_SOURCES}
COMMON_DRM_DEFINES += ${DTCP_IP_DEFINES}
else
include drm_tl/dtcp_ip/dtcp_ip.inc
COMMON_DRM_TL_INCLUDES += ${DTCP_IP_INCLUDES}
COMMON_DRM_TL_SOURCES += ${DTCP_IP_SOURCES}
COMMON_DRM_TL_DEFINES += ${DTCP_IP_DEFINES}
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
#Cannot build non-TL version of Common DRM library)
else
$(error Support for HDCP on SAGE does not exist)
endif

COMMON_DRM_INCLUDES += ${HDCP_INCLUDES}
COMMON_DRM_SOURCES += ${HDCP_SOURCES}
COMMON_DRM_DEFINES += ${HDCP_DEFINES}

#################### MARLIN ################################
ifeq (${MARLIN_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
$(error Support for Marlin on SAGE does not exist)
endif

COMMON_DRM_INCLUDES += ${MARLIN_INCLUDES}
COMMON_DRM_SOURCES += ${MARLIN_SOURCES}
COMMON_DRM_DEFINES += ${MARLIN_DEFINES}

#################### OCAP ################################
ifeq (${OCAP_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
$(error Support for OCAP on SAGE does not exist)
endif

COMMON_DRM_INCLUDES += ${OCAP_INCLUDES}
COMMON_DRM_SOURCES += ${OCAP_SOURCES}
COMMON_DRM_DEFINES += ${OCAP_DEFINES}

#################### NETFLIX ################################
ifeq (${NETFLIX_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
include drm_tl/netflix/netflix.inc
COMMON_DRM_TL_INCLUDES += ${NETFLIX_INCLUDES}
COMMON_DRM_TL_SOURCES += ${NETFLIX_SOURCES}
COMMON_DRM_TL_DEFINES += ${NETFLIX_DEFINES}
endif

#################### PMC ################################
ifeq (${PMC_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
$(error Support for PMC on SAGE does not exist)
endif

COMMON_DRM_INCLUDES += ${PMC_INCLUDES}
COMMON_DRM_SOURCES += ${PMC_SOURCES}
COMMON_DRM_DEFINES += ${PMC_DEFINES}

#################### SECURE SW RSA ################################
ifeq (${SECURE_SW_RSA_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
$(error Support for SECURE_SW_RSA on SAGE does not exist)
endif

COMMON_DRM_INCLUDES += ${SECURE_SW_RSA_INCLUDES}
COMMON_DRM_SOURCES += ${SECURE_SW_RSA_SOURCES}
COMMON_DRM_DEFINES += ${SECURE_SW_RSA_DEFINES}


#################### VUDU ################################
ifeq (${VUDU_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
$(error Support for VUDU on SAGE does not exist)
endif

COMMON_DRM_INCLUDES += ${VUDU_INCLUDES}
COMMON_DRM_SOURCES += ${VUDU_SOURCES}
COMMON_DRM_DEFINES += ${VUDU_DEFINES}

#################### WIDEVINE ################################
ifeq (${WIDEVINE_SAGE},OFF)
#Cannot build non-TL version of Common DRM library)
else
include drm_tl/widevine/widevine.inc
endif

COMMON_DRM_INCLUDES += ${WIDEVINE_INCLUDES}
COMMON_DRM_SOURCES += ${WIDEVINE_SOURCES}
COMMON_DRM_DEFINES += ${WIDEVINE_DEFINES}


#############################################################


CFLAGS += ${COMMON_DRM_DEFINES}


########################### Playready 2.5 ##################################
ifeq ($(findstring 2.5, ${MSDRM_PRDY_SDK_VERSION}), 2.5)
include drm/playready/playready.inc
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
DRM_BUILD_PROFILE   := 900
CFLAGS              += -DDRM_BUILD_PROFILE=$(DRM_BUILD_PROFILE)
CFLAGS              += -I${NXCLIENT_INCLUDES}
CFLAGS              += -I$(URSR_TOP)/BSEAV/lib/playready/2.5/inc -DTARGET_LITTLE_ENDIAN=1
CFLAGS              += -DTARGET_SUPPORTS_UNALIGNED_DWORD_POINTERS=1

PRDY_BASENAME       := cmndrmprdy
PRDY_PUBLIC_LIBS    += lib${PRDY_BASENAME}.a lib${PRDY_BASENAME}.so
PRDY_OBJS           := $(patsubst %.c,%.o, ${PLAYREADY_SOURCES})

include lib/prdy_libdir.inc
PRDY_FOR_LIB_INSTALL:= ${CMNDRM_PRDY_LIBDIR}
PRDY_INSTALLED_LIBS	:= $(addprefix ${COMMON_DRM_OBJ_ROOT}/${PRDY_FOR_LIB_INSTALL}/, ${PRDY_PUBLIC_LIBS})
endif

ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), ${B_REFSW_ARCH})
CFLAGS +=-marm -march=armv7-a
endif

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##



#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##

LIB_TL_BASENAME     := cmndrm_tl


###########################################################################
## LDFLAGS section
###########################################################################
LDFLAGS = -L$(NEXUS_BIN_DIR)/ -lnexus${NEXUS_LIB_SUFFIX}


#TL lib should always link against regular libcmndrm.so
LDFLAGS += $(COMMON_DRM_LDFLAGS)




############
# You probably shouldn't have to modify the text below this point.
#####
COMMON_DRM_OBJECTS  :=  $(patsubst %.c,%.o, ${COMMON_DRM_SOURCES})
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



#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##

#$(warning TL_LDFLAGS = ${TL_LDFLAGS})
#$(warning LDFLAGS = ${LDFLAGS})
#$(warning F_INSTALLED_LIBS = ${F_INSTALLED_LIBS})
#$(warning COMMON_DRM_OBJECTS = ${COMMON_DRM_OBJECTS})
#$(warning COMMON_DRM_TL_OBJECTS = ${COMMON_DRM_TL_OBJECTS})
#$(warning COMMON_DRM_SOURCES = ${COMMON_DRM_SOURCES})
#$(warning COMMON_DRM_TL_SOURCES = ${COMMON_DRM_TL_SOURCES})
#$(warning CFLAGS = ${CFLAGS})


############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
ifeq ($(ANDROID_BUILD),y)
all: ${F_PUBLIC_LIBS} ${F_PUBLIC_TL_LIBS}
else
all: prep_openssl prep_srai ${F_PUBLIC_LIBS} ${F_PUBLIC_TL_LIBS}
endif

prep_folder:
	@ mkdir -p $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $(dir ${PLAYREADY_SOURCES} ${COMMON_DRM_TL_SOURCES}))
ifeq ($(SAGE_SUPPORT),y)
	@ mkdir -p $(dir ${F_INSTALLED_TL_LIBS})
endif

prep_srai:
ifeq ($(SAGE_SUPPORT),y)
	@echo [..... prep_SRAI];
	$(Q)$(MAKE) -C $(URSR_TOP)/BSEAV/lib/security/sage/srai re
endif

prep_openssl:
	@echo [..... prep_openssl with j option];
	$(Q_)$(MAKE) -j8 -C $(B_OPENSSL_BUILD_DIR) > /dev/null

playready: ${PRDY_PUBLIC_LIBS} prdyidirs ${PRDY_INSTALLED_LIBS}
ifneq ($(findstring 2.5, ${MSDRM_PRDY_SDK_VERSION}), 2.5)
	$(error invalid Playready SDK version ${MSDRM_PRDY_SDK_VERSION})
endif

clean:
	rm -rf ${COMMON_DRM_OBJ_ROOT}

report:
	@echo "NEXUS CFLAGS is $(NEXUS_CFLAGS)"
	@echo "TL_FLAGS is $(TL_LDFLAGS)"
	@echo "LDFLAGS is $(LDFLAGS)"

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##


############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
lib${LIB_TL_BASENAME}.so: ${COMMON_DRM_TL_OBJECTS}
	@ echo [Linking shared TL library .... $@ ]
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	@ ${CC} -shared -o ${COMMON_DRM_OBJ_ROOT}/$@ ${TL_LDFLAGS} ${LDFLAGS} $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $^) -Wl,-Bdynamic
else
	@ ${CC} -shared -o ${COMMON_DRM_OBJ_ROOT}/$@ ${TL_LDFLAGS} $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $^) -Wl,-dy
endif

lib${LIB_TL_BASENAME}.a: ${COMMON_DRM_TL_OBJECTS}
	@ echo [Linking static TL library .... $@ ]
	@ ${AR} rc ${COMMON_DRM_OBJ_ROOT}/$@ $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $^)

lib${PRDY_BASENAME}.so: ${PRDY_OBJS}
	@ echo [Linking shared library .... $@ ]
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	 ${CC} -v -shared -o ${COMMON_DRM_OBJ_ROOT}/$@ ${LD_SYSROOT} ${PRDY_LDFLAGS} $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $^) -Wl,--no-whole-archive -Wl,-Bdynamic
else
	@ ${CC} -v -shared -o ${COMMON_DRM_OBJ_ROOT}/$@ ${LD_SYSROOT} ${PRDY_LDFLAGS} $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $^) -Wl,--no-whole-archive -Wl,-dy
endif

lib${PRDY_BASENAME}.a: ${PRDY_OBJS}
	@ echo [Linking static library .... $@ ]
	@ ${AR} rc ${COMMON_DRM_OBJ_ROOT}/$@ $(addprefix ${COMMON_DRM_OBJ_ROOT}/, $^)

prdyidirs:
	@ [ -d ${COMMON_DRM_OBJ_ROOT}/${PRDY_FOR_LIB_INSTALL} ] || mkdir -p ${COMMON_DRM_OBJ_ROOT}/${PRDY_FOR_LIB_INSTALL}


install: all ${F_INSTALLED_LIBS} ${F_INSTALLED_TL_LIBS}

${COMMON_DRM_OBJ_ROOT}/lib/${CMNDRM_DIR}/%.a : %.a
	install -m 0755 ${COMMON_DRM_OBJ_ROOT}/$< $@

${COMMON_DRM_OBJ_ROOT}/lib/${CMNDRM_DIR}/%.so : %.so
	install -m 0755 ${COMMON_DRM_OBJ_ROOT}/$< $@

${COMMON_DRM_OBJ_ROOT}/${PRDY_FOR_LIB_INSTALL}/%.a : %.a
	install -m 0755 ${COMMON_DRM_OBJ_ROOT}/$< $@

${COMMON_DRM_OBJ_ROOT}/${PRDY_FOR_LIB_INSTALL}/%.so : %.so
	install -m 0755 ${COMMON_DRM_OBJ_ROOT}/$< $@

%.o: %.c prep_folder
	@ echo [Compiling with $(CC).... $< ]
ifeq ($(SAGE_SUPPORT),y)
	@ ${CC} ${CFLAGS} ${SAGE_CFLAGS} ${COMMON_DRM_TL_DEFINES} -c -o ${COMMON_DRM_OBJ_ROOT}/$@ $<
else
	@ ${CC} ${CFLAGS} -c -o ${COMMON_DRM_OBJ_ROOT}/$@ $<
endif


copy: all
	install -m 0755 ${COMMON_DRM_TOP}/lib/${CMNDRM_DIR}/libcmndrm.so $(NEXUS_BIN_DIR); \
	if [ -e ${COMMON_DRM_OBJ_ROOT}/libcmndrmprdy.so ]; then \
		install -m 0755 ${COMMON_DRM_OBJ_ROOT}/libcmndrmprdy.so $(NEXUS_BIN_DIR); \
	fi;
	if [ -e ${COMMON_DRM_OBJ_ROOT}/libcmndrm_tl.so ]; then \
		install -m 0755 ${COMMON_DRM_OBJ_ROOT}/libcmndrm_tl.so $(NEXUS_BIN_DIR); \
	fi;
