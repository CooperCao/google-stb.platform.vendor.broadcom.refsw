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
NEXUS_TOP := ../../../../nexus
BSEAV_TOP := $(NEXUS_TOP)/../BSEAV/
OPENSSL_TOP := $(BSEAV_TOP)/lib/openssl/

# Build the name of this makefile, relative to B_REFSW_TOP (used for recipe tracing)
# Keep this line before any includes!
B_THIS_MAKEFILE_NAME := $(subst $(abspath ${NEXUS_TOP}/..),,$(abspath $(lastword $(MAKEFILE_LIST))))
# Uncomment the following two lines to enable recipe tracing.
# B_TRACE_RECIPE_BEGIN = @echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>  ${B_THIS_MAKEFILE_NAME}: Begin recipe: $@"
# B_TRACE_RECIPE_END   = @echo "<<<<<<<<<<<<<<<<<<<<<<<<<<<<  ${B_THIS_MAKEFILE_NAME}:   End recipe: $@"


# variables for out of source builds
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= $(abspath ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR})
BCRYPT_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/bcrypt

# Include nexus definitions
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc
include $(BSEAV_TOP)/lib/openssl/openssl.inc


# Sanity check that we received a valid platform
ifndef BCHP_CHIP
$(error Unsupported platform $(NEXUS_PLATFORM))
endif

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

include bcrypt_libdir.inc

# Custom include paths
CFLAGS += -I./include
CFLAGS += -DPIC -fpic
CFLAGS += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS})

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

ifeq ($(ANDROID_BUILD),y)
ifndef ANDROID
$(error ANDROID env variable required)
endif
ifeq ($(ANDROID_USES_BORINGSSL),y)
CFLAGS += -DUSES_BORINGSSL
else
CFLAGS += -I$(ANDROID)/external/openssl/include
endif
else # end of ANDROID_BUILD=y
CFLAGS += $(B_OPENSSL_CFLAGS)
endif # end of ANDROID_BUILD=n

#allow c++ style comments
CFLAGS += -std=c99

CFLAGS +=  $(B_OPENSSL_CFLAGS)

############################################################################
#
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##

# DRM Makefile exclude the 'include' subfolder which is built with the makefile
D_ALL_SUBDIRS		:= $(filter-out %include, $(shell find . -type d))
D_ALL_SUBDIRS	:= $(filter-out ./examples/%, ${D_ALL_SUBDIRS})
D_ALL_MODULE_DIR	:= $(filter-out %include, $(shell find . -maxdepth 1 -type d))
#$(error XXX D_ALL_SUBDIRS = ${D_ALL_SUBDIRS} XXX)

vpath %.cpp    		${D_ALL_SUBDIRS}
vpath  %.c    		${D_ALL_SUBDIRS}
vpath %.h      		${D_ALL_SUBDIRS}
LIB_BASENAME        := bcrypt




############
# Set F_PUBLIC_INCS to the basenames of include files that are to be
# used by other libraries or applications.  These files will be
# installed into .../${BCHP_VER}/include/
#####
F_PUBLIC_INCS    	+= $(sort $(notdir $(shell find ${D_ALL_MODULE_DIR} -name '*.h')))


#$(error ${F_PUBLIC_INCS})

############
# If your library depends on another shared lib, you should add that
# lib here (and any additional lib directories to search).
#####
#LDFLAGS				+= -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR} -ldrmrootfs

# TODO: uncheck when shared OpenSSL library is available
ifeq ($(ANDROID_BUILD),y)
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc
LDFLAGS             += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR} -ldrmrootfs
LDFLAGS             += --sysroot=$(ANDROID)/out/target/product/bcm_platform/system
LDFLAGS             += -lcrypto -llog
LDFLAGS             += $(NEXUS_LDFLAGS)
LDFLAGS             += $(NEXUS_LD_LIBRARIES)
else
LDFLAGS		    += $(B_OPENSSL_LDFLAGS)
endif





############
# Directory where to install the headers
#####
D_FOR_INC_INSTALL	:= include



############
# You probably shouldn't have to modify the text below this point.
#####
F_LIB_NAMES         := lib${LIB_BASENAME}.a  lib${LIB_BASENAME}.so
F_PUBLIC_LIBS    	+= $(addprefix ${BCRYPT_OBJ_ROOT}/,${F_LIB_NAMES})
D_FOR_LIB_INSTALL	:= ${BCRYPT_OBJ_ROOT}/${BCRYPT_LIBDIR}

F_SRCS	       		:= $(filter-out ${F_SRC_EXCLUDES}, $(notdir $(wildcard $(addsuffix /*.c, ${D_ALL_SUBDIRS}))))
F_OBJS 		   		:= $(patsubst %.c,%.o, ${F_SRCS})
F_DEPS 		   		:= $(patsubst %.c,%.d, ${F_SRCS})
END_OBJS            := $(addprefix ${BCRYPT_OBJ_ROOT}/,${F_OBJS})
END_DEPS            := $(addprefix ${BCRYPT_OBJ_ROOT}/,${F_DEPS})
F_INSTALLED_LIBS	:= $(addprefix ${D_FOR_LIB_INSTALL}/, ${F_LIB_NAMES})
CDEP_FLAGS          := -MMD
#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##

#$(error END_OBJS = $(END_OBJS))
#$(error CFLAGS = $(CFLAGS))
#$(warning F_LIB_NAMES: ${F_LIB_NAMES} )
#$(warning F_PUBLIC_LIBS: ${F_PUBLIC_LIBS} )
#$(warning F_INSTALLED_LIBS: ${F_INSTALLED_LIBS} )
#$(warning D_FOR_LIB_INSTALL: ${D_FOR_LIB_INSTALL} )





############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##

.PHONY : all
all: ${F_PUBLIC_LIBS}

${F_PUBLIC_LIBS}  ${END_OBJS}     : | prep_folder
#  For non-Android builds, the OpenSSL lib and headers will be a prerequisite
#  before we can start compiling the bcrypt source files.
ifneq ($(ANDROID_BUILD),y)
    ${F_PUBLIC_LIBS}  ${END_OBJS} : | ssl_lib
endif

include $(BSEAV)/lib/openssl/b_openssl_targets.inc
.PHONY : ssl_lib
ifeq ($(B_BUILD_DEPS),n)
ssl_lib:
else
ssl_lib: b_openssl_target_build
endif

.PHONY : prep_folder
prep_folder:
	$(B_TRACE_RECIPE_BEGIN)
	$(Q_)mkdir -p ${BCRYPT_OBJ_ROOT}/
	$(B_TRACE_RECIPE_END)

.PHONY : clean
clean:
	$(B_TRACE_RECIPE_BEGIN)
	@ echo "[Clean..... bcrypt]"
	$(Q_)rm -f ${END_DEPS}
	$(Q_)rm -f ${END_OBJS}
	$(Q_)rm -f ${F_INSTALLED_LIBS} ${BCRYPT_OBJ_ROOT}/lib${LIB_BASENAME}.so ${BCRYPT_OBJ_ROOT}/lib${LIB_BASENAME}.a
	$(B_TRACE_RECIPE_END)


#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##



############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
${BCRYPT_OBJ_ROOT}/lib${LIB_BASENAME}.so: ${END_OBJS}
	$(B_TRACE_RECIPE_BEGIN)
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	${CC} -shared -o $@  ${LDFLAGS}  $^ -Wl,-Bdynamic
else
	@ echo "[Link...... $(notdir $@) (bcrypt)]"
	$(Q_)${CC} -shared -o $@ ${LDFLAGS} ${END_OBJS} -Wl,-dy

endif
	$(B_TRACE_RECIPE_END)

${BCRYPT_OBJ_ROOT}/lib${LIB_BASENAME}.a: ${END_OBJS}
	$(B_TRACE_RECIPE_BEGIN)
	@ echo "[Archive... $(notdir $@) (bcrypt)]"
	$(Q_)${AR} rc $@ ${END_OBJS}
	$(B_TRACE_RECIPE_END)

${BCRYPT_OBJ_ROOT}/%.o: %.c
	$(B_TRACE_RECIPE_BEGIN)
	@ echo "[Compile... $(notdir $<) (bcrypt)]"
	$(Q_)${CC} ${CFLAGS} ${CDEP_FLAGS} -c -o $@ $<
	$(B_TRACE_RECIPE_END)

idirs:
	$(B_TRACE_RECIPE_BEGIN)
	$(Q_)[ -d ${D_FOR_LIB_INSTALL} ] || mkdir -p ${D_FOR_LIB_INSTALL}
	$(Q_)[ -d ${D_FOR_INC_INSTALL} ] || mkdir -p ${D_FOR_INC_INSTALL}
	$(B_TRACE_RECIPE_END)

install: all idirs ${F_INSTALLED_LIBS}

${D_FOR_LIB_INSTALL}/%.a : ${BCRYPT_OBJ_ROOT}/%.a
	$(B_TRACE_RECIPE_BEGIN)
	@install -m 0755 $< $@
	$(B_TRACE_RECIPE_END)

${D_FOR_LIB_INSTALL}/%.so : ${BCRYPT_OBJ_ROOT}/%.so
	$(B_TRACE_RECIPE_BEGIN)
	@install -m 0755 $< $@
	$(B_TRACE_RECIPE_END)

.PHONY : copy
copy:
	$(B_TRACE_RECIPE_BEGIN)
	@echo [Install... bcrypt]
	@cp -f ${D_FOR_LIB_INSTALL}/libbcrypt.so $(NEXUS_BIN_DIR)/
	$(B_TRACE_RECIPE_END)

#  Include dependency file:
-include ${BCRYPT_OBJ_ROOT}/*.d
