
#############################################################################
#    (c)2010-2011 Broadcom Corporation
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
#
#
#############################################################################

PWD := ${shell pwd}
OPENSOURCE ?= ${PWD}/../
THIS_DIR := BSEAV/lib/protobuf
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

ifneq ($(filter $(B_REFSW_DEBUG),n no_error_messages),)
    DEBUG_SUFFIX=release
else
    DEBUG_SUFFIX=debug
endif

B_PROTOBUF_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)

PROTOBUF_VER?=2.5.0
PROTOBUF_NAME=protobuf-$(PROTOBUF_VER)

PROTOBUF_SOURCE_TARBALL:=$(PROTOBUF_NAME).tar.bz2
PROTOBUF_SOURCE_DIRECTORY:=$(abspath $(B_REFSW_OBJ_ROOT)/BSEAV/lib/protobuf/$(B_REFSW_ARCH).$(DEBUG_SUFFIX))
PROTOBUF_SOURCE_PATH:=${PROTOBUF_SOURCE_DIRECTORY}/${PROTOBUF_NAME}
PROTOBUF_TAR_PATH:=${PROTOBUF_SOURCE_DIRECTORY}
PROTOBUF_BUILD_DIR:=$(abspath $(NEXUS_TOP)/../BSEAV/lib/protobuf)

PROTOBUF_CFLAGS?= -O2 -fPIC
PROTOBUF_LDFLAGS?=
PROTOBUF_DIR:=$(PROTOBUF_NAME)
PROTOBUF_LIB_FOLDER:=${PROTOBUF_TAR_PATH}/${PROTOBUF_DIR}/$(NEXUS_PLATFORM)/lib

PROTOBUF_INC_CFLAGS:= -D_REENTRANT -I${PROTOBUF_TAR_PATH}/${PROTOBUF_DIR}/$(NEXUS_PLATFORM)/include
PROTOBUF_INC_LDFLAGS:= -L$(PROTOBUF_LIB_FOLDER) -lprotobuf
GTEST_SOURCE_PATH:=$(PROTOBUF_SOURCE_PATH)/gtest
GTEST_LIB_FOLDER:=${PROTOBUF_TAR_PATH}/${PROTOBUF_DIR}/gtest/lib/.libs/
GTEST_INC_CFLAGS:= -D_REENTRANT -I${PROTOBUF_TAR_PATH}/${PROTOBUF_DIR}/gtest/include
GTEST_INC_LDFLAGS:= -L$(GTEST_LIB_FOLDER) -lgtest


.PHONY: source all clean

B_PROTOBUF_EXTRACT_REDIR :=

ifeq ($(B_REFSW_PROTOBUF_IS_EXTERNAL),y)
    $(warning B_REFSW_PROTOBUF_IS_EXTERNAL=${B_REFSW_PROTOBUF_IS_EXTERNAL} Skipping build of Protobuf...)
endif   #  ifeq ($(B_REFSW_PROTOBUF_IS_EXTERNAL),y)

source:
ifneq ($(B_REFSW_PROTOBUF_IS_EXTERNAL),y)
	@if [ ! -d "${PROTOBUF_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "Protobuf source directory: $(abspath ${PROTOBUF_SOURCE_PATH}) not found, extracting from tarball..."  ${B_PROTOBUF_EXTRACT_REDIR}; \
		echo ""; \
		echo "PROTOBUF_SOURCE_DIRECTORY is ${PROTOBUF_SOURCE_DIRECTORY}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${PROTOBUF_TAR_PATH}/${PROTOBUF_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading OPENSSL tarball  to ${PROTOBUF_TAR_PATH}"; \
			mkdir -p ${PROTOBUF_TAR_PATH}; \
			echo "${PROTOBUF_BUILD_DIR}/${PROTOBUF_NAME}"; \
			echo "${PROTOBUF_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${PROTOBUF_BUILD_DIR}/${PROTOBUF_NAME}.info ${PROTOBUF_TAR_PATH}; \
		fi ;\
		if [ -e "${PROTOBUF_TAR_PATH}/${PROTOBUF_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting PROTOBUF tarball"; \
			echo "${PROTOBUF_SOURCE_PATH}"; \
			mkdir -p ${PROTOBUF_SOURCE_PATH}; \
			echo "PROTOBUF_TAR_PATH is ${PROTOBUF_TAR_PATH}"; \
			echo "PROTOBUF_SOURCE_TARBALL is ${PROTOBUF_SOURCE_TARBALL}"; \
			tar -jxf ${PROTOBUF_TAR_PATH}/${PROTOBUF_SOURCE_TARBALL} -C ${PROTOBUF_SOURCE_PATH}/..; \
		else \
			echo "Tarball ${PROTOBUF_TAR_PATH}/${PROTOBUF_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_PROTOBUF_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_PROTOBUF_IS_EXTERNAL),y)

#install: all
#	@echo "================ Installing PROTOBUF"
#	@echo "APPLIBS_TARGET_LIB_DIR is ${APPLIBS_TARGET_LIB_DIR}"
#	@echo "APPLIBS_TARGET_INC_DIR is ${APPLIBS_TARGET_INC_DIR}"
#	@mkdir -p ${APPLIBS_TARGET_LIB_DIR}
#	@mkdir -p ${APPLIBS_TARGET_INC_DIR}
#	@cp -af $(PROTOBUF_TAR_PATH)/$(PROTOBUF_NAME)/src/.libs/*.so* ${APPLIBS_TARGET_LIB_DIR}

all : source
	@if [ ! -d "${PROTOBUF_SOURCE_PATH}" ]; then \
		echo "Protobuf source directory: $(abspath ${PROTOBUF_SOURCE_PATH}) not found, extracting from tarball..."  ${B_PROTOBUF_EXTRACT_REDIR}; \
		if [ ! -e "${PROTOBUF_TAR_PATH}/${PROTOBUF_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading OPENSSL tarball  to ${PROTOBUF_TAR_PATH}"; \
			mkdir -p ${PROTOBUF_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${PROTOBUF_BUILD_DIR}/${PROTOBUF_NAME}.info ${PROTOBUF_TAR_PATH}; \
		fi ;\
	fi
	@echo "================ Configuring PROTOBUF"
	@if [ ! -e "${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}/Makefile" ]; then \
		echo "================ Configure PROTOBUF"; \
		cd ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}; \
		CC=$(strip $(subst $(B_PROTOBUF_TOOLCHAIN_ARCH)-,,$(CC))) \
		CFLAGS="${PROTOBUF_CFLAGS}" \
		LDFLAGS="${PROTOBUF_LDFLAGS}" \
		./configure \
		--prefix=${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}/${NEXUS_PLATFORM}${LOWER_BCHP_VER} \
		--host=${B_PROTOBUF_TOOLCHAIN_ARCH} \
		--disable-static\
		--with-protoc= \
		--enable-shared; \
		echo "================ PROTOBUF Configured"; \
	fi

	@if [ ! -e "${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}/Makefile" ]; then \
		echo "${PROTOBUF_NAME} is not configured!"; exit 1;\
	fi

	@echo "================ Compiling PROTOBUF==============================="
	${MAKE} ${MAKE_OPTIONS} -C ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME} -j10 ${MAKE_SUFFIX}
	${MAKE} ${MAKE_OPTIONS} -C ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}/gtest -j10 ${MAKE_SUFFIX}
	${MAKE} ${MAKE_OPTIONS} -C ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME} install ${MAKE_SUFFIX}
	@if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		${STRIP} ${PROTOBUF_LIB_FOLDER}/*.so; \
	fi
	@rm -rf protobuf_ver.inc
	@echo "PROTOBUF_VER:="$(PROTOBUF_VER) >> protobuf_ver.inc
	@echo "PROTOBUF_NAME:="$(PROTOBUF_NAME) >> protobuf_ver.inc
	@echo "PROTOBUF_CFLAGS:="$(PROTOBUF_INC_CFLAGS) >> protobuf_ver.inc
	@echo "PROTOBUF_LDFLAGS:="$(PROTOBUF_INC_LDFLAGS) >> protobuf_ver.inc
	@echo "PROTOBUF_SOURCE_PATH:="$(PROTOBUF_SOURCE_PATH) >> protobuf_ver.inc
	@echo "GTEST_CFLAGS:="$(GTEST_INC_CFLAGS) >> protobuf_ver.inc
	@echo "GTEST_LDFLAGS:="$(GTEST_INC_LDFLAGS) >> protobuf_ver.inc
	@echo "GTEST_SOURCE_PATH:="$(GTEST_SOURCE_PATH) >> protobuf_ver.inc
	@echo "================ PROTOBUF Compiled!"
	@touch $@

clean :
	@echo "================ MAKE CLEAN"
	@rm -rf protobuf_ver.inc
	@rm -rf ${PROTOBUF_TAR_PATH}
	@rm -rf all
	@echo "================ Done"
