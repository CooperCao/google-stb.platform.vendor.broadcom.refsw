#############################################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

SHELL := /bin/bash

THIS_DIR := BSEAV/lib/libnl
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
B_REFSW_OBJ_DIR  ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV_TOP}/../${B_REFSW_OBJ_DIR}

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

include $(NEXUS_TOP)/../BSEAV/lib/libnl/libnl.inc

B_LIBNL_TOOLCHAIN_ARCH ?= $(B_REFSW_CROSS_COMPILE)

B_LIBNL_TAR_PATH:=${B_LIBNL_SOURCE_DEST}
B_LIBNL_TAR_FILENAME:=$(B_LIBNL_NAME).tar.gz
B_LIBNL_LIB_BUILD_CFLAGS?= -02 -fPIC
B_LIBNL_LIB_BUILD_LDFLAGS?=

.PHONY: clean all bin-target

ifeq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)
    $(warning B_REFSW_LIBNL_IS_EXTERNAL=${B_REFSW_LIBNL_IS_EXTERNAL} Skipping build of libnl...)
endif   #  ifeq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)

all: install

#  Set up some make output filtering to get a medium amount of neatly formatted output (unless VERBOSE is enabled).
B_LIBNL_EXTRACT_REDIR :=
B_LIBNL_CONFIG_REDIR  :=
B_LIBNL_COMPILE_REDIR :=
B_LIBNL_INSTALL_REDIR :=

ifeq ($(B_REFSW_VERBOSE),)
	B_LIBNL_EXTRACT_REDIR := 2>/dev/null  |  egrep --line-buffered  "===" ; test $${PIPESTATUS[0]}
	B_LIBNL_CONFIG_REDIR := 2>/dev/null  |  													\
			egrep --line-buffered  "===|making depend in |making links in " |   		\
			sed -u 																		\
					-e  "s/^making depend in \(.*\)$$/[Depends... \1 (libnl)]/ "		\
					-e  "s/^making links in \(.*\)$$/[Symlinking... \1 (libnl)]/ "	\
			; test $${PIPESTATUS[0]}
	B_LIBNL_COMPILE_REDIR := 2>/dev/null  |  											\
			egrep --line-buffered  "===|making all in " |   					\
			sed -u 																\
					-e  "s/^making all in \(.*\)$$/[Build... \1 (libnl)]/ "	\
			; test $${PIPESTATUS[0]}
	B_LIBNL_INSTALL_REDIR := &>/dev/null
endif


.source:
ifneq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)
	@if [ ! -d "${B_LIBNL_SOURCE_PATH}" ]; then \
		echo "libnl source directory: $(abspath ${B_LIBNL_SOURCE_PATH}) not found, extracting from tarball..."  ${B_LIBNL_EXTRACT_REDIR}; \
		if [ ! -e "${B_LIBNL_TAR_PATH}/${B_LIBNL_TAR_FILENAME}" ]; then \
			echo "================ Downloading LIBNL tarball"; \
			mkdir -p ${B_LIBNL_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${B_LIBNL_BUILD_DIR}/${B_LIBNL_NAME}.info ${B_LIBNL_TAR_PATH}; \
		fi ;\
		if [ -e "${B_LIBNL_TAR_PATH}/${B_LIBNL_TAR_FILENAME}" ]; then \
			echo "================ Extracting LIBNL tarball"; \
			mkdir -p ${B_LIBNL_SOURCE_PATH}; \
			tar -xzf ${B_LIBNL_TAR_PATH}/${B_LIBNL_TAR_FILENAME} -C ${B_LIBNL_SOURCE_PATH}/..; \
# Example of how to add a patch file \
#			if [ -e "libnl-${B_LIBNL_VER}-fix_parallel_build-1.patch" ]; then \
#				echo "applying patch libnl-${B_LIBNL_VER}-fix_parallel_build-1.patch";\
#				patch -t -d ${B_LIBNL_SOURCE_PATH} -p1 < libnl-${B_LIBNL_VER}-fix_parallel_build-1.patch ${B_LIBNL_EXTRACT_REDIR}; \
#			fi;\
		else \
			echo "Tarball ${B_LIBNL_TAR_PATH}/${B_LIBNL_TAR_FILENAME} is missing"; exit 1;\
		fi;\
	fi
endif   #  ifeq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)

.configure: .source
ifneq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)
	@if [ ! -e "${B_LIBNL_SOURCE_PATH}/.configured" ]; then \
		echo "================ Configuring LIBNL" && \
		cd ${B_LIBNL_SOURCE_PATH} && \
		./configure \
		--enable-shared \
		--enable-static \
		--host=$(B_REFSW_ARCH) \
		--prefix=$(B_LIBNL_INSTALL_ROOT) \
		--exec-prefix=$(B_LIBNL_INSTALL_ROOT) &&\
		touch ${B_LIBNL_SOURCE_PATH}/.configured && \
		echo "================ LIBNL Configured"; \
	fi  ${B_LIBNL_CONFIG_REDIR}
endif   #  ifeq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)

MAKE_SUFFIX:=APPS=

.$(B_LIBNL_NAME): .source .configure
ifneq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)
	@echo "[Build.... (libnl)]"
	@${MAKE} ${MAKE_OPTIONS} -C ${B_LIBNL_SOURCE_PATH} ${MAKE_SUFFIX}  ${B_LIBNL_COMPILE_REDIR}
endif   #  ifeq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)

ifdef INSTALL_DIR
 B_LIBNL_INSTALL_DIR = $(INSTALL_DIR)
else
 ifdef DESTDIR
  B_LIBNL_INSTALL_DIR = $(DESTDIR)
 else
  ifdef NEXUS_BIN_DIR
   B_LIBNL_INSTALL_DIR = $(NEXUS_BIN_DIR)
  endif
 endif
endif

install: .$(B_LIBNL_NAME)
ifneq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)
	@echo "[Install.. (libnl)]"
	@${MAKE} -C ${B_LIBNL_SOURCE_PATH} install ${MAKE_SUFFIX} ${B_LIBNL_COMPILE_REDIR}
ifdef B_LIBNL_INSTALL_DIR
	@cp -fp $(B_LIBNL_LIB_INSTALL_DIR)/libnl-3.so* $(B_LIBNL_LIB_INSTALL_DIR)/libnl-genl-3.so* $(B_LIBNL_INSTALL_DIR)
endif

endif   #  ifeq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)

target-clean: clean

clean:
ifneq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)
	@echo "[Clean.... (libnl)]"
	@${MAKE} -C ${B_LIBNL_SOURCE_PATH} uninstall ${MAKE_SUFFIX} ${B_LIBNL_COMPILE_REDIR}
	@rm -rf ${B_LIBNL_SOURCE_PATH}
endif   #  ifeq ($(B_REFSW_LIBNL_IS_EXTERNAL),y)
