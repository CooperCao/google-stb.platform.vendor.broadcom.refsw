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
# Makefile for building the Vulkan Loader

# Ensure cmake is available
CMAKE_GTEQ_208011 := $(shell expr `cmake --version 2>&1 | head -1 | \
                     awk 'BEGIN { FS = "\\\.|\\\ " } { printf("%d%2.2d%2.2d\n", $$3, $$4, $$5) }'` \>= 20811)
ifneq ("$(CMAKE_GTEQ_208011)", "1")
$(error cmake >= 2.8.11 must be available on build machine)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

.PHONY : loader cmake_run

loader : cmake_run
	@cmake --build loader_cmake_build

# Note : the -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast are in the CFLAGS below
# to stop the loader build from whining. The casts only appear to be done on NULL, so I
# don't believe that they will cause real runtime issues.

cmake_run :
	@mkdir -p ./loader_cmake_build
	@cd ./loader_cmake_build &&                                \
	CFLAGS="-Wno-int-to-pointer-cast -Wno-pointer-to-int-cast" \
	cmake -G "Unix Makefiles"                                  \
		-D CMAKE_SYSTEM_NAME=Linux                             \
		-D CMAKE_C_COMPILER=$(CC)                              \
		-D CMAKE_CXX_COMPILER=$(CXX)                           \
		-D DEMOS_WSI_SELECTION=DISPLAY                         \
		-D BUILD_WSI_XCB_SUPPORT=OFF                           \
		-D BUILD_WSI_XLIB_SUPPORT=OFF                          \
		-D BUILD_WSI_WAYLAND_SUPPORT=OFF                       \
		-D BUILD_WSI_MIR_SUPPORT=OFF                           \
		-D BUILD_LOADER=ON                                     \
		-D BUILD_TESTS=OFF                                     \
		-D BUILD_LAYERS=OFF                                    \
		-D BUILD_DEMOS=OFF                                     \
		-D BUILD_VKJSON=OFF                                    \
		$(LIBVULKAN_SOURCE_EXTRACTED)
