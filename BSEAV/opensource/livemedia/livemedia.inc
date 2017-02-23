#############################################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#  Check for required definitions.
$(if ${NEXUS_PLATFORM},,$(error NEXUS_PLATFORM is not defined))
$(if ${NEXUS_TOP},,$(error NEXUS_TOP is not defined))
$(if ${B_REFSW_ARCH},,$(error B_REFSW_ARCH is not defined))

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}

#  Define Livemedia's Makefile directory... probably the same dir that this file is in.
B_LIVEMEDIA_BUILD_DIR := $(abspath $(NEXUS_TOP)/../BSEAV/opensource/livemedia)

#  Define the Livemedia out-of-source directory tree.
B_LIVEMEDIA_DIR = ${B_REFSW_OBJ_ROOT}/BSEAV/opensource/livemedia/live

#  List any compiler defines here.
B_LIVEMEDIA_DEFINES :=

B_LIVEMEDIA_STAGED_INSTALL_DIR := ${B_LIVEMEDIA_DIR}/staged_install

#  List directories that contain include files.
B_LIVEMEDIA_INCLUDES := $(B_LIVEMEDIA_STAGED_INSTALL_DIR)/include/liveMedia             \
                        $(B_LIVEMEDIA_STAGED_INSTALL_DIR)/include/groupsock             \
                        $(B_LIVEMEDIA_STAGED_INSTALL_DIR)/include/UsageEnvironment      \
                        $(B_LIVEMEDIA_STAGED_INSTALL_DIR)/include/BasicUsageEnvironment

#  Assemble compiler defines and include directories into CFLAGS.
B_LIVEMEDIA_CFLAGS := $(addprefix -I,$(B_LIVEMEDIA_INCLUDES))

#  List names of libraries.
B_LIVEMEDIA_LIBS := liveMedia               \
                    groupsock               \
                    UsageEnvironment        \
                    BasicUsageEnvironment

#  List directories that contain the libraries.
B_LIVEMEDIA_LIB_DIRS := $(B_LIVEMEDIA_STAGED_INSTALL_DIR)/lib

#  Assemble libraries and their directories into LDFLAGS.
B_LIVEMEDIA_LDFLAGS := $(addprefix -L,$(B_LIVEMEDIA_LIB_DIRS))   $(addprefix -l,$(B_LIVEMEDIA_LIBS))
