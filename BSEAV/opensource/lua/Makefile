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

B_REFSW_OS ?= linuxuser

include ../../../nexus/build/os/$(B_REFSW_OS)/os_tools.inc

PWD := ${shell pwd}
OPENSOURCE ?= ${PWD}/../
THIS_DIR := BSEAV/opensource/lua
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include $(BSEAV_TOP)/opensource/lua/lua.inc
include lua.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

.PHONY: source all clean

B_LUA_EXTRACT_REDIR :=

all : source
	@if [ ! -d "${LUA_SOURCE_PATH}" ]; then \
		echo "lua source directory: $(abspath ${LUA_SOURCE_PATH}) not found, extracting from tarball..."  ${B_LUA_EXTRACT_REDIR}; \
		if [ ! -e "${LUA_TAR_PATH}/${LUA_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading LUA tarball  to ${LUA_TAR_PATH}"; \
			mkdir -p ${LUA_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${LUA_BUILD_DIR}/${LUA_NAME}.info ${LUA_TAR_PATH}; \
		fi ;\
	fi

	@echo "================ Compiling LUA==============================="
	echo "NEXUS_TOP=$(abspath ${NEXUS_TOP})"
	@$(MAKE) -C $(LUA_TAR_PATH) -f Makefile posix > /dev/null
	@$(MAKE) -C $(LUA_TAR_PATH) -f Makefile local > /dev/null

	@if [ "$(B_REFSW_DEBUG)" != 'y' ]; then \
		if ls ${LUA_LIB_FOLDER}/*.so > /dev/null 2>&1 ; then ${STRIP} ${LUA_LIB_FOLDER}/*.so ; fi \
	fi
	@echo "================ LUA Compiled!"

source:
ifneq ($(B_REFSW_LUA_IS_EXTERNAL),y)
	@if [ ! -d "${LUA_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "lua source directory: $(abspath ${LUA_SOURCE_PATH}) not found, extracting from tarball..."  ${B_LUA_EXTRACT_REDIR}; \
		echo ""; \
		echo "LUA_SOURCE_DIRECTORY is ${LUA_SOURCE_DIRECTORY}"; \
		echo "$(B_REFSW_ARCH).$(DEBUG_SUFFIX)"; \
		if [ ! -e "${LUA_TAR_PATH}/${LUA_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading LUA tarball  to ${LUA_TAR_PATH}"; \
			mkdir -p ${LUA_TAR_PATH}; \
			echo "${LUA_BUILD_DIR}/${LUA_NAME}"; \
			echo "${LUA_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${LUA_BUILD_DIR}/${LUA_NAME}.info ${LUA_TAR_PATH}; \
		fi ;\
		if [ -e "${LUA_TAR_PATH}/${LUA_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting LUA tarball"; \
			echo "${LUA_SOURCE_PATH}"; \
			mkdir -p ${LUA_SOURCE_PATH}; \
			echo "LUA_TAR_PATH is ${LUA_TAR_PATH}"; \
			echo "LUA_SOURCE_TARBALL is ${LUA_SOURCE_TARBALL}"; \
			tar -xpf ${LUA_TAR_PATH}/${LUA_SOURCE_TARBALL} -C ${LUA_SOURCE_PATH}/..; \
			echo "Copy ${LUA_BUILD_DIR}/${LUA_NAME} to ${LUA_SOURCE_PATH}/.." ; \
			cp -r ${LUA_BUILD_DIR}/${LUA_NAME} ${LUA_SOURCE_PATH}/.. ; \
			cp ${BSEAV}/api/build/tools.mak ${LUA_SOURCE_PATH}/src > /dev/null; \
		else \
			echo "Tarball ${LUA_TAR_PATH}/${LUA_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_LUA_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_LUA_IS_EXTERNAL),y)

install_to_nexus_bin: all
	@echo "================ Installing LUA to ${NEXUS_BIN_DIR}"
	@mkdir -p ${NEXUS_BIN_DIR}
	@install -c -m 0755 $(LUA_LIB_FOLDER)/*.so* ${NEXUS_BIN_DIR}

clean :
	@echo "================ MAKE CLEAN"
	@rm -rf lua_ver.inc
	@rm -rf ${LUA_TAR_PATH}
	@echo "================ Done"
