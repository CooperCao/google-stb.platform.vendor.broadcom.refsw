##############################################################################
#  Copyright (C) 2018 Broadcom.
#  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to
#  the terms and conditions of a separate, written license agreement executed
#  between you and Broadcom (an "Authorized License").  Except as set forth in
#  an Authorized License, Broadcom grants no license (express or implied),
#  right to use, or waiver of any kind with respect to the Software, and
#  Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
##############################################################################


SHELL := /bin/bash

THIS_DIR := BSEAV/opensource/ebtables
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
B_REFSW_OBJ_DIR  ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV_TOP}/../${B_REFSW_OBJ_DIR}

NEXUS_TOP := $(B_REFSW_TOP)/nexus
include $(shell pwd)/ebtables.inc
B_EBTABLES_TAR_PATH:=$(B_EBTABLES_SOURCE_DEST)
B_EBTABLES_TAR_FILENAME:=$(B_EBTABLES_NAME).tar.gz

.PHONY: clean all target-clean

ifeq ($(B_REFSW_EBTABLES_IS_EXTERNAL),y)
    $(warning B_REFSW_EBTABLES_IS_EXTERNAL=${B_REFSW_EBTABLES_IS_EXTERNAL} Skipping build of ebtables...)
endif   #  ifeq ($(B_REFSW_EBTABLES_IS_EXTERNAL),y)

all: .source

#  Set up some make output filtering to get a medium amount of neatly formatted output (unless VERBOSE is enabled).
B_EBTABLES_EXTRACT_REDIR :=
B_EBTABLES_CONFIG_REDIR  :=
B_EBTABLES_COMPILE_REDIR :=
B_EBTABLES_INSTALL_REDIR :=

ifeq ($(B_REFSW_VERBOSE),)
	B_EBTABLES_EXTRACT_REDIR := 2>/dev/null  |  egrep --line-buffered  "===" ; test $${PIPESTATUS[0]}
	B_EBTABLES_CONFIG_REDIR := 2>/dev/null  |  													\
			egrep --line-buffered  "===|making depend in |making links in " |   		\
			sed -u 																		\
					-e  "s/^making depend in \(.*\)$$/[Depends... \1 (ebtables)]/ "		\
					-e  "s/^making links in \(.*\)$$/[Symlinking... \1 (ebtables)]/ "	\
			; test $${PIPESTATUS[0]}
	B_EBTABLES_COMPILE_REDIR := >/dev/null  |  											\
			egrep --line-buffered  "===|making all in " |   					\
			sed -u 																\
					-e  "s/^making all in \(.*\)$$/[Build... \1 (ebtables)]/ "	\
			; test $${PIPESTATUS[0]}
	B_EBTABLES_INSTALL_REDIR := >/dev/null
endif

.source:
	echo "${B_EBTABLES_SOURCE_PATH}"
ifneq ($(B_REFSW_EBTABLES_IS_EXTERNAL),y)
	@if [ ! -d "${B_EBTABLES_SOURCE_PATH}" ]; then \
		echo "ebtables source directory: $(abspath ${B_EBTABLES_SOURCE_PATH}) not found, extracting from tarball..."  ${B_EBTABLES_EXTRACT_REDIR}; \
		if [ ! -e "${B_EBTABLES_TAR_PATH}/${B_EBTABLES_TAR_FILENAME}" ]; then \
			echo "================ Downloading EBTABLES tarball"; \
			mkdir -p ${B_EBTABLES_TAR_PATH}; \
			chmod -R 755 ${B_EBTABLES_TAR_PATH}; \
			echo "$(NEXUS_TOP) $(B_REFSW_TOP)"; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${B_EBTABLES_OPENSRC_DIR}/${B_EBTABLES_NAME}.info ${B_EBTABLES_TAR_PATH}; \
		fi ;\
		if [ -e "${B_EBTABLES_TAR_PATH}/${B_EBTABLES_TAR_FILENAME}" ]; then \
			echo "================ Extracting EBTABLES tarball"; \
			mkdir -p ${B_EBTABLES_SOURCE_PATH}; \
			tar -xzf ${B_EBTABLES_TAR_PATH}/${B_EBTABLES_TAR_FILENAME} -C ${B_EBTABLES_TAR_PATH}; \
			if [ -e "ebtables-${B_EBTABLES_VER}.patch" ]; then \
				echo "applying patch ebtables-${B_EBTABLES_VER}.patch"; \
				patch -t -d ${B_EBTABLES_SOURCE_PATH} -p1 < ebtables-${B_EBTABLES_VER}.patch ${B_EBTABLES_EXTRACT_REDIR}; \
			fi; \
			if [ -e "ebtables-${B_EBTABLES_VER}-android_p.patch" ] && [ "${STBANDROID}" == "1" ]; then \
				echo "applying patch ebtables-${B_EBTABLES_VER}-android_p.patch"; \
				patch -t -d ${B_EBTABLES_SOURCE_PATH} -p1 < ebtables-${B_EBTABLES_VER}-android_p.patch ${B_EBTABLES_EXTRACT_REDIR}; \
			fi; \
		else \
			echo "Tarball ${B_EBTABLES_TAR_PATH}/${B_EBTABLES_TAR_FILENAME} is missing"; exit 1;\
		fi;\
	fi
endif   #  ifeq ($(B_REFSW_EBTABLES_IS_EXTERNAL),y)

target-clean: clean

clean:
ifneq ($(B_REFSW_EBTABLES_IS_EXTERNAL),y)
	@echo "[Clean.... (ebtables)]"
	@rm -rf ${B_EBTABLES_SOURCE_PATH}
endif   #  ifeq ($(B_REFSW_EBTABLES_IS_EXTERNAL),y)
