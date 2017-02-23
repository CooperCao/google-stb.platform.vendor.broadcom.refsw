############################################################
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
#
# Module Description:
#
############################################################

NEXUS_TOP ?= $(shell cd ../../../../../nexus; pwd)

all: dual_abi
.PHONY: arm32 arm64
dual_abi: arm32 arm64
NEXUS_BUILD_EXAMPLES?=y
NEXUS_BUILD_UTILS?=y
NEXUS_BUILD_NXCLIENT?=y
NEXUS_BUILD_CUBE?=y
NEXUS_BUILD_TESTS?=y


arm32: arm32_nexus arm32_apps
arm64: arm64_nexus arm64_apps

arm32_nexus:
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux  NEXUS_CLIENT_ONLY=y -C ${NEXUS_TOP}/build

arm32_examples: arm32_nexus
ifeq ($(NEXUS_MODE),proxy)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux B_NEXUS_API_BUILD_COMPLETED=y -C ${NEXUS_TOP}/examples/multiprocess
else
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux B_NEXUS_API_BUILD_COMPLETED=y -C ${NEXUS_TOP}/examples/multiprocess client
endif

arm32_tests: arm32_nexus
ifeq ($(NEXUS_MODE),proxy)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux B_NEXUS_API_BUILD_COMPLETED=y -C ${NEXUS_TOP}/../rockford/unittests/nexus/multiprocess
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux B_NEXUS_API_BUILD_COMPLETED=y -C ${NEXUS_TOP}/../rockford/unittests/nexus/dma
else
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux B_NEXUS_API_BUILD_COMPLETED=y -C ${NEXUS_TOP}/../rockford/unittests/nexus/multiprocess client
endif

ifeq ($(NEXUS_BUILD_UTILS),y)
arm32_apps: arm32_utils
arm64_apps: arm64_utils
endif

ifeq ($(NEXUS_BUILD_NXCLIENT),y)
arm32_apps: arm32_nxclient
arm64_apps: arm64_nxclient
endif

ifeq ($(NEXUS_BUILD_EXAMPLES),y)
arm32_apps: arm32_examples
arm64_apps: arm64_examples
endif

ifeq ($(NEXUS_BUILD_CUBE),y)
arm32_apps: arm32_cube
arm64_apps: arm32_cube
endif

ifeq ($(NEXUS_BUILD_TESTS),y)
arm32_apps: arm32_tests
arm64_apps: arm64_tests
endif


arm32_utils: arm32_nexus
ifeq ($(NEXUS_MODE),proxy)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux B_NEXUS_API_BUILD_COMPLETED=y -C ${NEXUS_TOP}/utils
endif


arm32_nxclient: arm32_nexus
ifeq ($(NEXUS_MODE),proxy)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux B_NEXUS_API_BUILD_COMPLETED=y PLAYBACK_IP_SUPPORT=n -C ${NEXUS_TOP}/lib/os
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux B_NEXUS_API_BUILD_COMPLETED=y PLAYBACK_IP_SUPPORT=n -C ${NEXUS_TOP}/nxclient/build
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux B_NEXUS_API_BUILD_COMPLETED=y PLAYBACK_IP_SUPPORT=n -C ${NEXUS_TOP}/nxclient
endif

arm32_cube: arm32_nexus
ifeq ($(NEXUS_MODE),proxy)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux B_NEXUS_API_BUILD_COMPLETED=y -C ${NEXUS_TOP}/../rockford/applications/khronos/v3d/nexus/cube
endif



arm64_nexus:
	${MAKE} -C ${NEXUS_TOP}/build B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 B_REFSW_ARCH=aarch64-linux
ifeq ($(NEXUS_MODE),)
	@# if not proxy, then force usermode multiprocess
	${MAKE} -C ${NEXUS_TOP}/build B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 NEXUS_MODE=client
endif


arm64_utils: arm64_nexus
	${MAKE} -C ${NEXUS_TOP}/utils B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 B_NEXUS_API_BUILD_COMPLETED=y

arm64_examples: arm64_nexus
	${MAKE} -C ${NEXUS_TOP}/examples/multiprocess B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 B_NEXUS_API_BUILD_COMPLETED=y

arm64_nxclient: arm64_nexus
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 B_NEXUS_API_BUILD_COMPLETED=y PLAYBACK_IP_SUPPORT=n -C ${NEXUS_TOP}/lib/os
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 B_NEXUS_API_BUILD_COMPLETED=y PLAYBACK_IP_SUPPORT=n -C ${NEXUS_TOP}/nxclient/build
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 B_NEXUS_API_BUILD_COMPLETED=y PLAYBACK_IP_SUPPORT=n -C ${NEXUS_TOP}/nxclient

arm64_cube: arm64_nexus
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 B_NEXUS_API_BUILD_COMPLETED=y -C ${NEXUS_TOP}/../rockford/applications/khronos/v3d/nexus/cube

arm64_tests: arm64_nexus
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 B_NEXUS_API_BUILD_COMPLETED=y -C ${NEXUS_TOP}/../rockford/unittests/nexus/multiprocess
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 B_NEXUS_API_BUILD_COMPLETED=y -C ${NEXUS_TOP}/../rockford/unittests/nexus/dma

clean: clean_nexus clean_arm32_apps clean_arm64_apps

clean_nexus:
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux -C ${NEXUS_TOP}/build clean
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 -C ${NEXUS_TOP}/build clean

clean_arm32_apps: clean_nexus
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux -C ${NEXUS_TOP}/build clean
ifeq ($(NEXUS_BUILD_UTILS),y)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux -C ${NEXUS_TOP}/utils clean
endif
ifeq ($(NEXUS_BUILD_NXCLIENT),y)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux -C ${NEXUS_TOP}/lib/os clean
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux -C ${NEXUS_TOP}/nxclient clean
endif
ifeq ($(NEXUS_BUILD_EXAMPLES),y)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux -C ${NEXUS_TOP}/examples clean
endif
ifeq ($(NEXUS_BUILD_CUBE),y)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux -C ${NEXUS_TOP}/../rockford/applications/khronos/v3d/nexus/cube clean
endif
ifeq ($(NEXUS_BUILD_TESTS),y)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux -C ${NEXUS_TOP}/../rockford/unittests/nexus/multiprocess clean
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux -C ${NEXUS_TOP}/../rockford/unittests/nexus/dma clean
endif

clean_arm64_apps: clean_nexus
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 -C ${NEXUS_TOP}/build clean
ifeq ($(NEXUS_BUILD_UTILS),y)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 -C ${NEXUS_TOP}/utils clean
endif
ifeq ($(NEXUS_BUILD_NXCLIENT),y)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 -C ${NEXUS_TOP}/lib/os clean
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 -C ${NEXUS_TOP}/nxclient clean
endif
ifeq ($(NEXUS_BUILD_EXAMPLES),y)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 -C ${NEXUS_TOP}/examples/multiprocess clean
endif
ifeq ($(NEXUS_BUILD_CUBE),y)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.64 -C ${NEXUS_TOP}/../rockford/applications/khronos/v3d/nexus/cube clean
endif
ifeq ($(NEXUS_BUILD_TESTS),y)
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux -C ${NEXUS_TOP}/../rockford/unittests/nexus/multiprocess clean
	${MAKE} B_REFSW_OBJ_DIR=obj.${NEXUS_PLATFORM}.32 B_REFSW_ARCH=arm-linux -C ${NEXUS_TOP}/../rockford/unittests/nexus/dma clean
endif
