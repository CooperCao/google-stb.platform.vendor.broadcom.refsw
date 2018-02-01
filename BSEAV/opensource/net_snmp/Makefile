###############################################################################
#   Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to the terms and
#   conditions of a separate, written license agreement executed between you and Broadcom
#   (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#   no license (express or implied), right to use, or waiver of any kind with respect to the
#   Software, and Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#   HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#   NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#   secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#   and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#   AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#   WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#   THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#   OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#   LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#   OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#   USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#   LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#   EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#   USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#   THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#   ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#   LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#   ANY LIMITED REMEDY.
#
###############################################################################
THIS_DIR := BSEAV/opensource/net_snmp
SHELL:=/bin/bash
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OPENSOURCE ?= ${ROOT_DIR}/../
B_REFSW_TOP ?= $(subst /${THIS_DIR},,$(ROOT_DIR))

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc
include ${NEXUS_TOP}/../${THIS_DIR}/net_snmp.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

B_NET_SNMP_TOOLCHAIN_ARCH ?= $(B_REFSW_ARCH)
NET_SNMP_CONFIG_CFLAGS  := -O2 -fPIC -DNET_SNMP_MAX_WRITE_SIZE=65536

all : source

.PHONY: all clean

B_NET_SNMP_EXTRACT_REDIR :=

ifeq ($(B_REFSW_NET_SNMP_IS_EXTERNAL),y)
    $(warning B_REFSW_NET_SNMP_IS_EXTERNAL=${B_REFSW_NET_SNMP_IS_EXTERNAL} Skipping build of net_snmp...)
endif   #  ifeq ($(B_REFSW_NET_SNMP_IS_EXTERNAL),y)

source:
ifneq ($(B_REFSW_NET_SNMP_IS_EXTERNAL),y)
	@if [ ! -d "${NET_SNMP_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "net_snmp source directory: " $(abspath ${NET_SNMP_SOURCE_PATH}) ; \
		echo "not found; extracting from tarball..."  ${B_NET_SNMP_EXTRACT_REDIR}; \
		echo ""; \
		echo "NET_SNMP_SOURCE_DIRECTORY is ${NET_SNMP_SOURCE_DIRECTORY}"; \
		if [ ! -e "${NET_SNMP_TAR_PATH}/${NET_SNMP_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading NET_SNMP $(NET_SNMP_VER) tarball  to ${NET_SNMP_TAR_PATH}"; \
			mkdir -p ${NET_SNMP_TAR_PATH}; \
			echo "${NET_SNMP_BUILD_DIR}/${NET_SNMP_NAME}"; \
			echo "${NET_SNMP_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${NET_SNMP_BUILD_DIR}/${NET_SNMP_NAME}.info ${NET_SNMP_TAR_PATH}; \
		fi ;\
		if [ -e "${NET_SNMP_TAR_PATH}/${NET_SNMP_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting NET_SNMP $(NET_SNMP_VER) tarball"; \
			echo "${NET_SNMP_SOURCE_PATH}"; \
			mkdir -p ${NET_SNMP_SOURCE_PATH}; \
			echo "NET_SNMP_TAR_PATH is ${NET_SNMP_TAR_PATH}"; \
			echo "NET_SNMP_SOURCE_TARBALL is ${NET_SNMP_SOURCE_TARBALL}"; \
			tar -xzf ${NET_SNMP_TAR_PATH}/${NET_SNMP_SOURCE_TARBALL} -C ${NET_SNMP_SOURCE_PATH}/..; \
		else \
			echo "Tarball ${NET_SNMP_TAR_PATH}/${NET_SNMP_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_NET_SNMP_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_NET_SNMP_IS_EXTERNAL),y)

# Determine if we are compiling for 64-bit machine
COMPILER := ${B_REFSW_ARCH}-gcc
BIT64 = $(findstring aarch64,${COMPILER})

all : source
	@echo "================ TARGET all: NET_SNMP $(NET_SNMP_VER) ============================="
	@echo "================ Configuring NET_SNMP $(NET_SNMP_VER) ============================="

	@if [ ! -e "${NET_SNMP_TAR_PATH}/${NET_SNMP_NAME2}/Makefile" ]; then \
		echo "================ ./configure NET_SNMP $(NET_SNMP_VER) ============================="; \
		cd ${NET_SNMP_TAR_PATH}/${NET_SNMP_NAME2}; \
		CFLAGS="${NET_SNMP_CONFIG_CFLAGS}" \
		LDFLAGS="${NET_SNMP_CONFIG_LDFLAGS}" \
		./configure \
			--prefix=/?tmp \
			--host=i686-pc-linux-gnu \
			--target=arm-linux \
			--with-cc=${B_REFSW_ARCH}-gcc \
			--with-ar=${B_REFSW_ARCH}-ar \
			--with-endianness=little \
			--with-default-snmp-version="3" \
			--with-sys-contact="who@where" \
			--with-sys-location="Unknown" \
			--with-logfile="/var/log/snmpd-master.log" \
			--disable-as-needed \
			--disable-embedded-perl \
			--with-persistent-directory="/var/net_snmp" > /dev/null 2>&1 ; \
		echo "================ Configuring NET_SNMP $(NET_SNMP_VER) complete ===================="; \
		echo "================ Patching    NET_SNMP $(NET_SNMP_VER) ============================="; \
		(cd $(NET_SNMP_TAR_PATH)/$(NET_SNMP_NAME2) && patch -p1 < $(WORKING_DIR)/net_snmp-$(NET_SNMP_VER).patch); \
	else \
		echo "================ NET_SNMP $(NET_SNMP_VER) already configured ======================"; \
	fi

	@if [ ! -e "${NET_SNMP_TAR_PATH}/${NET_SNMP_NAME2}/Makefile" ]; then \
		echo "${NET_SNMP_NAME2} is not configured!"; exit 1;\
	fi

	@echo "================ Compiling NET_SNMP $(NET_SNMP_VER) ==============================="
	@${MAKE} ${MAKE_OPTIONS} -C ${NET_SNMP_TAR_PATH}/${NET_SNMP_NAME2}/snmplib -j10 ${MAKE_SUFFIX} >/dev/null 2>&1

	@echo "================ Installing NET_SNMP $(NET_SNMP_VER) to nexus/bin ================="
	@mkdir -p ${NEXUS_BIN_DIR}
	@(cd $(NET_SNMP_LIB_FOLDER) && tar cBf - libnetsnmp*.so*) | (cd ${NEXUS_BIN_DIR} && tar xBf -)

clean :
	@echo "================ MAKE CLEAN =========================================="
	@rm -rf ${NET_SNMP_TAR_PATH}
	@rm -rf ${NEXUS_BIN_DIR}/net_snmp* ${NEXUS_BIN_DIR}/libnet_snmp*
