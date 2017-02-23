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

THIS_DIR := BSEAV/lib/hostapd
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
include $(NEXUS_TOP)/../BSEAV/opensource/openssl/openssl.inc
include $(NEXUS_TOP)/../BSEAV/lib/hostapd/hostapd.inc

export B_HOSTAPD_CFLAGS	   := ${NEXUS_CFLAGS} -fPIC -D_REENTRANT -I${B_LIBNL_INC_INSTALL_DIR} -I${B_OPENSSL_INC_INSTALL_DIR}
export B_HOSTAPD_CXXFLAGS   := ${CXXFLAGS} -fPIC
export B_HOSTAPD_LDFLAGS    := -L${B_LIBNL_LIB_INSTALL_DIR} -L$(B_OPENSSL_LIB_INSTALL_DIR)
export B_HOSTAPD_CROSS_HOST :=${B_REFSW_ARCH}

B_HOSTAPD_TOOLCHAIN_ARCH ?= $(B_REFSW_CROSS_COMPILE)

B_HOSTAPD_TAR_PATH:=${B_HOSTAPD_SOURCE_DEST}
B_HOSTAPD_TAR_FILENAME:=$(B_HOSTAPD_NAME).tar.gz
B_HOSTAPD_LIB_BUILD_CFLAGS?= -02 -fPIC
B_HOSTAPD_LIB_BUILD_LDFLAGS?=

.PHONY: clean all bin-target target-clean

ifeq ($(B_REFSW_HOSTAPD_IS_EXTERNAL),y)
    $(warning B_REFSW_HOSTAPD_IS_EXTERNAL=${B_REFSW_HOSTAPD_IS_EXTERNAL} Skipping build of hostapd...)
endif   #  ifeq ($(B_REFSW_HOSTAPD_IS_EXTERNAL),y)

all: install

#  Set up some make output filtering to get a medium amount of neatly formatted output (unless VERBOSE is enabled).
B_HOSTAPD_EXTRACT_REDIR :=
B_HOSTAPD_CONFIG_REDIR  :=
B_HOSTAPD_COMPILE_REDIR :=
B_HOSTAPD_INSTALL_REDIR :=

ifeq ($(B_REFSW_VERBOSE),)
	B_HOSTAPD_EXTRACT_REDIR := 2>/dev/null  |  egrep --line-buffered  "===" ; test $${PIPESTATUS[0]}
	B_HOSTAPD_CONFIG_REDIR := 2>/dev/null  |  													\
			egrep --line-buffered  "===|making depend in |making links in " |   		\
			sed -u 																		\
					-e  "s/^making depend in \(.*\)$$/[Depends... \1 (hostapd)]/ "		\
					-e  "s/^making links in \(.*\)$$/[Symlinking... \1 (hostapd)]/ "	\
			; test $${PIPESTATUS[0]}
	B_HOSTAPD_COMPILE_REDIR := >/dev/null  |  											\
			egrep --line-buffered  "===|making all in " |   					\
			sed -u 																\
					-e  "s/^making all in \(.*\)$$/[Build... \1 (hostapd)]/ "	\
			; test $${PIPESTATUS[0]}
	B_HOSTAPD_INSTALL_REDIR := >/dev/null
endif


.source:
ifneq ($(B_REFSW_HOSTAPD_IS_EXTERNAL),y)
	@if [ ! -d "${B_HOSTAPD_SOURCE_PATH}" ]; then \
		echo "hostapd source directory: $(abspath ${B_HOSTAPD_SOURCE_PATH}) not found, extracting from tarball..."  ${B_HOSTAPD_EXTRACT_REDIR}; \
		if [ ! -e "${B_HOSTAPD_TAR_PATH}/${B_HOSTAPD_TAR_FILENAME}" ]; then \
			echo "================ Downloading HOSTAPD tarball"; \
			mkdir -p ${B_HOSTAPD_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${B_HOSTAPD_BUILD_DIR}/${B_HOSTAPD_NAME}.info ${B_HOSTAPD_TAR_PATH}; \
		fi ;\
		if [ -e "${B_HOSTAPD_TAR_PATH}/${B_HOSTAPD_TAR_FILENAME}" ]; then \
			echo "================ Extracting HOSTAPD tarball"; \
			mkdir -p ${B_HOSTAPD_SOURCE_PATH}; \
			tar -xzf ${B_HOSTAPD_TAR_PATH}/${B_HOSTAPD_TAR_FILENAME} -C ${B_HOSTAPD_SOURCE_PATH}/..; \
			if [ -e "hostapd-${B_HOSTAPD_VER}-fix_libhostapd_client_build-1.patch" ]; then \
				echo "applying patch hostapd-${B_HOSTAPD_VER}-fix_libhostapd_client_build-1.patch"; \
				patch -t -d ${B_HOSTAPD_SOURCE_PATH} -p1 < hostapd-${B_HOSTAPD_VER}-fix_libhostapd_client_build-1.patch ${B_HOSTAPD_EXTRACT_REDIR}; \
			fi; \
			if [ -e "hostapd-${B_HOSTAPD_VER}-driver_cmd_support.patch" ]; then \
				echo "applying patch hostapd-${B_HOSTAPD_VER}-driver_cmd_support.patch"; \
				patch -t -d ${B_HOSTAPD_SOURCE_PATH} -p1 < hostapd-${B_HOSTAPD_VER}-driver_cmd_support.patch ${B_HOSTAPD_EXTRACT_REDIR}; \
			fi; \
		else \
			echo "Tarball ${B_HOSTAPD_TAR_PATH}/${B_HOSTAPD_TAR_FILENAME} is missing"; exit 1;\
		fi;\
	fi
endif   #  ifeq ($(B_REFSW_HOSTAPD_IS_EXTERNAL),y)

.configure: .source
ifneq ($(B_REFSW_HOSTAPD_IS_EXTERNAL),y)
	@if [ ! -e "${B_HOSTAPD_SOURCE_PATH}/.configured" ]; then \
		echo "================ Configuring HOSTAPD" && \
		cd ${B_HOSTAPD_SOURCE_PATH}/hostapd && \
		cp -f defconfig .config && \
		echo "CFLAGS+=-fPIC -I$(B_HOSTAPD_INC_INSTALL_DIR) -I$(B_HOSTAPD_INC_INSTALL_DIR)/libnl3 -I${B_OPENSSL_INC_INSTALL_DIR}" >> .config && \
		echo "LDFLAGS+=$(B_HOSTAPD_LDFLAGS)" >> .config && \
		echo "CONFIG_LIBNL32=y" >> .config && \
		echo "CONFIG_EAPOL_TEST=y" >> .config && \
		echo "LIBDIR=$(B_HOSTAPD_LIB_INSTALL_DIR)" >> .config && \
		echo "INCDIR=$(B_HOSTAPD_INC_INSTALL_DIR)" >> .config && \
		echo "BINDIR=$(B_HOSTAPD_BIN_INSTALL_DIR)" >> .config && \
		touch ${B_HOSTAPD_SOURCE_PATH}/.configured && \
		echo "================ HOSTAPD Configured"; \
	fi  ${B_HOSTAPD_CONFIG_REDIR}
endif   #  ifeq ($(B_REFSW_HOSTAPD_IS_EXTERNAL),y)

MAKE_SUFFIX:=APPS=

WPA_CC := $(strip $(CC))
export PKG_CONFIG_PATH=$(B_LIBNL_PKG_INSTALL_DIR)

.$(B_HOSTAPD_NAME): .source .configure
ifneq ($(B_REFSW_HOSTAPD_IS_EXTERNAL),y)
	@echo "[Build.... (hostapd)]"
	@${MAKE} ${MAKE_OPTIONS} -C ${B_HOSTAPD_SOURCE_PATH}/hostapd CC=$(WPA_CC) ${MAKE_SUFFIX}  ${B_HOSTAPD_COMPILE_REDIR}
endif   #  ifeq ($(B_REFSW_HOSTAPD_IS_EXTERNAL),y)


ifdef INSTALL_DIR
 B_INSTALL_DIR = $(INSTALL_DIR)
else
 ifdef DESTDIR
  B_INSTALL_DIR = $(DESTDIR)
 else
  ifdef NEXUS_BIN_DIR
   B_INSTALL_DIR = $(NEXUS_BIN_DIR)
  endif
 endif
endif

install: .$(B_HOSTAPD_NAME) $(B_HOSTAPD_BIN_INSTALL_DIR)/hostapd.conf
ifneq ($(B_REFSW_HOSTAPD_IS_EXTERNAL),y)
	@echo "[Install.. (hostapd)]"
	@${MAKE} -C ${B_HOSTAPD_SOURCE_PATH}/hostapd install CC=$(WPA_CC) ${MAKE_SUFFIX} ${B_HOSTAPD_COMPILE_REDIR}
ifdef B_INSTALL_DIR
	@mkdir -p $(B_INSTALL_DIR)
	@cp -fp $(B_HOSTAPD_BIN_INSTALL_DIR)/hostapd $(B_INSTALL_DIR)
	@cp -fp $(B_HOSTAPD_BIN_INSTALL_DIR)/hostapd_cli $(B_INSTALL_DIR)
	@cp -fp $(B_HOSTAPD_BIN_INSTALL_DIR)/hostapd.conf $(B_INSTALL_DIR)
	@cp -fp $(B_HOSTAPD_LIB_INSTALL_DIR)/libhostapd_client.so* $(B_INSTALL_DIR)
endif
endif

.PHONY: $(B_HOSTAPD_BIN_INSTALL_DIR)/hostapd.conf
$(B_HOSTAPD_BIN_INSTALL_DIR)/hostapd.conf: ${BSEAV_TOP}/lib/hostapd/hostapd.conf
	@cp -f ${BSEAV_TOP}/lib/hostapd/hostapd.conf ${B_HOSTAPD_BIN_INSTALL_DIR}

target-clean: clean

clean:
ifneq ($(B_REFSW_HOSTAPD_IS_EXTERNAL),y)
	@echo "[Clean.... (hostapd)]"
	@rm -rf ${B_HOSTAPD_SOURCE_PATH}
	@rm -rf ${NEXUS_PLATFORM}
endif   #  ifeq ($(B_REFSW_HOSTAPD_IS_EXTERNAL),y)
