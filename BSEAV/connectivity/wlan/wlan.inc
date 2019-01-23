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

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
DEBUG_SUFFIX=debug
else
DEBUG_SUFFIX=release
endif

#WLAN_DRIVER_PATH used for BSEAV tools ie.  bmon,bysperf
ifeq ($(NEXUS_PLATFORM),97271)
B_WLAN_VER?=7271
WLAN_DRIVER_PATH ?= $(B_REFSW_OBJ_ROOT)/BSEAV/connectivity/wlan/7271
else ifeq ($(NEXUS_PLATFORM),97255)
B_WLAN_VER?=43570
WLAN_DRIVER_PATH ?= $(B_REFSW_OBJ_ROOT)/BSEAV/connectivity/wlan/43570/source/DHD_REL_1_363_110_86
else ifeq ($(NEXUS_PLATFORM),97216)
B_WLAN_VER?=4375
WLAN_DRIVER_PATH ?= $(B_REFSW_OBJ_ROOT)/BSEAV/connectivity/wlan/4375/dhd
else
B_WLAN_VER =
WLAN_DRIVER_PATH =
endif

ifneq ($(B_WLAN_VER),STB7271_BRANCH_15_10)
endif

#  Define the Makefile directory... probably the same dir that this file is in.
B_WLAN_INSRC_DIR := $(abspath $(NEXUS_TOP)/../BSEAV/connectivity/wlan)
B_WLAN_INSRC_VER_DIR:=${B_WLAN_INSRC_DIR}/${B_WLAN_VER}

B_WLAN_OOSRC_DIR:=$(abspath $(B_REFSW_OBJ_ROOT)/BSEAV/connectivity/wlan)
B_WLAN_OOSRC_VER_DIR:=${B_WLAN_OOSRC_DIR}/${B_WLAN_VER}
B_WLAN_TOP_INSTALL_DIR ?= $(abspath ${B_WLAN_OOSRC_VER_DIR})

#  Define the dirs where the Broadcom wlan wrapper makefile installs things.
B_WLAN_INC_INSTALL_DIR ?= ${B_WLAN_TOP_INSTALL_DIR}/include

#  CFLAGS and LDFLAGS
B_WLAN_CFLAGS:= -I${B_WLAN_INC_INSTALL_DIR}
B_WLAN_LDFLAGS:=
