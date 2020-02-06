###############################################################################
#   Copyright (C) 2019 Broadcom.
#   The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to
#   the terms and conditions of a separate, written license agreement executed
#   between you and Broadcom (an "Authorized License").  Except as set forth in
#   an Authorized License, Broadcom grants no license (express or implied),
#   right to use, or waiver of any kind with respect to the Software, and
#   Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#   THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#   IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization,
#   constitutes the valuable trade secrets of Broadcom, and you shall use all
#   reasonable efforts to protect the confidentiality thereof, and to use this
#   information only in connection with your use of Broadcom integrated circuit
#   products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#   OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#   RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#   IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#   A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#   ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#   THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#   OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#   INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#   RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#   HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#   EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#   WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#   FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################

B_REFSW_OS ?= linuxuser

PWD := ${shell pwd}
OPENSOURCE ?= ${PWD}/../
THIS_DIR := BSEAV/opensource/iperf
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
B_LIB_TOP := $(NEXUS_TOP)/lib
include ${B_LIB_TOP}/build/b_lib_defs.inc
include $(BSEAV_TOP)/opensource/iperf/iperf.inc

$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

.PHONY: source all clean

B_IPERF_EXTRACT_REDIR :=

# Determine if we are compiling for 64-bit machine
COMPILER := ${B_REFSW_ARCH}-gcc
BIT64 = $(findstring aarch64,${COMPILER})

LINUX_VERSION := $(shell grep -m 1 '^VERSION = ' ${LINUX}/Makefile | awk '{print $$3}')
LINUX_PATCHLEVEL := $(shell grep -m 1 '^PATCHLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')
LINUX_SUBLEVEL := $(shell grep -m 1 '^SUBLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')
LINUX_VER_EQ_3 :=  $(shell test $(LINUX_VERSION) -eq 3 && echo y)
ifeq ($(LINUX_VER_EQ_3),y)
    LINUX_VER_EQ_3_3 := $(shell test $(LINUX_PATCHLEVEL) -eq 3 && echo 'y')
endif
PATCH_LOG := $(shell ls -1 $(IPERF_EXE_FOLDER)/patch.log 2>/dev/null )
PATCH_LOG_LINUX_3.3 := $(shell ls -1 $(IPERF_EXE_FOLDER)/patch_linux_33.log 2>/dev/null )

all: source
	@echo "================ Configuring IPERF $(IPERF_VER) ==============================="

	@if [ ! -e "${IPERF_TAR_PATH}/${IPERF_NAME}/Makefile" ]; then \
		echo "================ Configure IPERF $(IPERF_VER) ================================="; \
		cd ${IPERF_TAR_PATH}/${IPERF_NAME}; \
		CFLAGS="${IPERF_CONFIG_CFLAGS}" \
		LDFLAGS="${IPERF_CONFIG_LDFLAGS}" \
		./configure --host=${B_REFSW_ARCH} --without-openssl --disable-shared --enable-static >/dev/null 2>&1 ; \
		echo "================ Configuring IPERF $(IPERF_VER) complete ======================"; \
	else \
		echo "================ IPERF $(IPERF_VER) already configured ========================"; \
	fi

	@if [ ! -e "${IPERF_TAR_PATH}/${IPERF_NAME}/Makefile" ]; then \
		echo "${IPERF_NAME} is not configured!"; exit 1;\
	fi

	@if ! test -z $(LINUX_VER_EQ_3_3); then echo "[Detected 3.3 kernel]" >/dev/null ; \
		if test -z $(PATCH_LOG_LINUX_3.3); then echo "[Linux-3.3 Patch has not been done yet]" >/dev/null ; \
			echo "================ Patching using iperf-${IPERF_VER}.linux-3.3.patch ======================"; \
			(cd $(IPERF_EXE_FOLDER) && patch -p4 < $(WORKING_DIR)/iperf-$(IPERF_VER).linux-3.3.patch && date > $(IPERF_EXE_FOLDER)/patch_linux_33.log ); \
		else echo "[Linux 3.3 Patch has already been done]" >/dev/null ; \
		fi ; \
    else echo "[Detected NON 3.3 kernel ]" >/dev/null ; \
	fi
	@if test -z $(PATCH_LOG); then echo "[Common patch has not been done yet]" >/dev/null ; \
		echo "================ Patching using iperf-${IPERF_VER}.patch ======================"; \
		(cd $(IPERF_EXE_FOLDER) && patch -p2 < $(WORKING_DIR)/iperf-$(IPERF_VER).patch && date > $(IPERF_EXE_FOLDER)/patch.log ); \
		else echo "[Common patch has already been done]" >/dev/null ; \
	fi ; \

	@echo "================ Compiling IPERF $(IPERF_VER) ================================="
	@${MAKE} ${MAKE_OPTIONS} -C ${IPERF_TAR_PATH}/${IPERF_NAME} -j10 ${MAKE_SUFFIX} >/dev/null 2>&1

	@echo "================ Installing IPERF $(IPERF_VER) to nexus/bin ==================="
	@mkdir -p ${NEXUS_BIN_DIR}
	@install -c -m 0755 $(IPERF_EXE_FOLDER)/iperf3 ${NEXUS_BIN_DIR}
	@rm -f $(NEXUS_BIN_DIR)/iperf
	@(cd $(NEXUS_BIN_DIR) && ln -s iperf3 iperf) >/dev/null 2>&1

install_to_nexus_bin: all

source:
ifneq ($(B_REFSW_IPERF_IS_EXTERNAL),y)
	@if [ ! -d "${IPERF_SOURCE_PATH}" ]; then \
		echo ""; \
		echo "iperf source directory: " $(abspath ${IPERF_SOURCE_PATH}) ; \
		echo "not found; extracting from tarball..."  ${B_IPERF_EXTRACT_REDIR}; \
		echo ""; \
		echo "IPERF_SOURCE_DIRECTORY is ${IPERF_SOURCE_DIRECTORY}"; \
		if [ ! -e "${IPERF_TAR_PATH}/${IPERF_SOURCE_TARBALL}" ]; then \
			echo "================ Downloading IPERF $(IPERF_VER) tarball  to ${IPERF_TAR_PATH}"; \
			mkdir -p ${IPERF_TAR_PATH}; \
			echo "${IPERF_BUILD_DIR}/${IPERF_NAME}"; \
			echo "${IPERF_TAR_PATH}"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${IPERF_BUILD_DIR}/${IPERF_NAME}.info ${IPERF_TAR_PATH}; \
		fi ;\
		if [ -e "${IPERF_TAR_PATH}/${IPERF_SOURCE_TARBALL}" ]; then \
			echo "================ Extracting IPERF $(IPERF_VER) tarball"; \
			echo "${IPERF_SOURCE_PATH}"; \
			mkdir -p ${IPERF_SOURCE_PATH}; \
			echo "IPERF_TAR_PATH is ${IPERF_TAR_PATH}"; \
			echo "IPERF_SOURCE_TARBALL is ${IPERF_SOURCE_TARBALL}"; \
			tar -xzf ${IPERF_TAR_PATH}/${IPERF_SOURCE_TARBALL} -C ${IPERF_SOURCE_PATH}/..; \
		else \
			echo "Tarball ${IPERF_TAR_PATH}/${IPERF_SOURCE_TARBALL} is missing"; exit 1;\
		fi ${B_IPERF_EXTRACT_REDIR};\
	fi
endif   #  ifeq ($(B_REFSW_IPERF_IS_EXTERNAL),y)

clean :
	@echo "================ MAKE CLEAN"
	@rm -rf iperf_ver.inc
	@rm -rf ${IPERF_TAR_PATH}
	@echo "================ Done"
