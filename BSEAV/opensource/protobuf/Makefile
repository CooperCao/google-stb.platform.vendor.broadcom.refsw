############################################################
#  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
PWD := ${shell pwd}
OPENSOURCE ?= ${PWD}/../
THIS_DIR := BSEAV/opensource/protobuf
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include protobuf.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_PROTOBUF_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)

PROTOC_VER := $(shell protoc --version | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')
ifeq ("$(PROTOC_VER)", "$(PROTOBUF_VER)")
    PROTOC_COMMAND := protoc
else
    PROTOC_COMMAND := $(B_REFSW_OBJ_ROOT)/BSEAV/opensource/protobuf/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)/protobuf-$(PROTOBUF_VER)/host/bin/protoc
endif

PROTOBUF_CONFIG_CXXFLAGS?= -O2 -fPIC -Wno-unused-local-typedefs -Wno-maybe-uninitialized -Wno-return-type

.PHONY: source all clean build_protoc

B_PROTOBUF_EXTRACT_REDIR :=

ifeq ($(B_REFSW_PROTOBUF_IS_EXTERNAL),y)
    $(warning B_REFSW_PROTOBUF_IS_EXTERNAL=${B_REFSW_PROTOBUF_IS_EXTERNAL} Skipping build of Protobuf...)
endif   #  ifeq ($(B_REFSW_PROTOBUF_IS_EXTERNAL),y)

all : ${PROTOBUF_SOURCE_PATH}/.compiled

build_protoc: source
ifneq ($(PROTOBUF_VER), $(PROTOC_VER))
	@echo "================ Configuring PROTOBUF for build server"
	@if [ ! -e "${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}/Makefile" ]; then \
		echo "================ Configure PROTOBUF"; \
		cd ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}; \
                CXXFLAGS="${PROTOBUF_CONFIG_CXXFLAGS}" \
		./configure \
		--prefix=${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}/host \
		--disable-shared; \
		echo "================ PROTOBUF Configured"; \
	fi
	@echo "================ Building protoc for build server"
	${MAKE} ${MAKE_OPTIONS} -C ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME} ${MAKE_SUFFIX}
	${MAKE} ${MAKE_OPTIONS} -C ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME} install ${MAKE_SUFFIX}
	@if [ ! -e "${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}/host/bin/protoc" ]; then \
		echo "Filed to build protoc!"; exit 1; \
	fi
	${MAKE} ${MAKE_OPTIONS} -C ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME} distclean ${MAKE_SUFFIX}
	@echo "================ protoc for build server was built"
else
	@echo "================ Use protoc installed in build server"
endif # ifneq ($(PROTOBUF_VER), $(PROTOC_VER))

${PROTOBUF_SOURCE_PATH}/.compiled : | build_protoc
	@echo "================ Configuring PROTOBUF"
	@if [ ! -e "${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}/Makefile" ]; then \
		echo "================ Configure PROTOBUF"; \
		cd ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}; \
		CC=$(strip $(subst $(B_PROTOBUF_TOOLCHAIN_ARCH)-,,$(CC))) \
		CXXFLAGS="${PROTOBUF_CONFIG_CXXFLAGS}" \
		./configure \
		--prefix=${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}/${NEXUS_PLATFORM}${LOWER_BCHP_VER} \
		--host=${B_PROTOBUF_TOOLCHAIN_ARCH} \
		--enable-static \
		--with-protoc=${PROTOC_COMMAND} \
		--enable-shared; \
		echo "================ PROTOBUF Configured"; \
	fi

	@if [ ! -e "${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}/Makefile" ]; then \
		echo "${PROTOBUF_NAME} is not configured!"; exit 1;\
	fi

	@echo "================ Compiling PROTOBUF==============================="
	${MAKE} ${MAKE_OPTIONS} -C ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME} ${MAKE_SUFFIX}
	@echo "================ Compiling PROTOBUF gtest==============================="
	${MAKE} ${MAKE_OPTIONS} -C ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME}/gtest ${MAKE_SUFFIX}
	@echo "================ Installing PROTOBUF headers==============================="
	${MAKE} ${MAKE_OPTIONS} -C ${PROTOBUF_TAR_PATH}/${PROTOBUF_NAME} install-data ${MAKE_SUFFIX}
	@if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		if ls ${PROTOBUF_LIB_FOLDER}/*.so > /dev/null 2>&1 ; then ${STRIP} ${PROTOBUF_LIB_FOLDER}/*.so ; fi ; \
		if ls ${GTEST_LIB_FOLDER}/*.so > /dev/null 2>&1 ; then ${STRIP} ${GTEST_LIB_FOLDER}/*.so ; fi \
	fi
	@echo "================ PROTOBUF Compiled!"
	@touch $@

source:
ifneq ($(B_REFSW_PROTOBUF_IS_EXTERNAL),y)
	@if [ ! -d "${PROTOBUF_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "Protobuf source directory: $(abspath ${PROTOBUF_SOURCE_PATH}) not found, extracting from tarball..."  ${B_PROTOBUF_EXTRACT_REDIR}; \
		echo ""; \
		echo "PROTOBUF_SOURCE_DIRECTORY is ${PROTOBUF_SOURCE_DIRECTORY}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${PROTOBUF_TAR_PATH}/${PROTOBUF_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading PROTOBUF tarball  to ${PROTOBUF_TAR_PATH}"; \
			mkdir -p ${PROTOBUF_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${PROTOBUF_BUILD_DIR}/${PROTOBUF_NAME}.info ${PROTOBUF_TAR_PATH}; \
		fi ;\
		if [ -e "${PROTOBUF_TAR_PATH}/${PROTOBUF_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting PROTOBUF tarball"; \
			mkdir -p ${PROTOBUF_SOURCE_PATH}; \
			tar -xf ${PROTOBUF_TAR_PATH}/${PROTOBUF_SOURCE_TARBALL} -C ${PROTOBUF_SOURCE_PATH}/..; \
		else \
			echo "Tarball ${PROTOBUF_TAR_PATH}/${PROTOBUF_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_PROTOBUF_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_PROTOBUF_IS_EXTERNAL),y)

install_to_nexus_bin: all
	@echo "================ Installing PROTOBUF to ${NEXUS_BIN_DIR}"
	@mkdir -p ${NEXUS_BIN_DIR}
	@install -c -m 0755 $(PROTOBUF_LIB_FOLDER)/*.so* ${NEXUS_BIN_DIR}
	@install -c -m 0755 $(GTEST_LIB_FOLDER)/*.so* ${NEXUS_BIN_DIR}

clean :
	@echo "================ MAKE CLEAN"
	@rm -rf .libs
	@rm -rf ${PROTOBUF_TAR_PATH}
	@rm -rf ${PROTOBUF_SOURCE_PATH}/.compiled
	@echo "================ Done"
