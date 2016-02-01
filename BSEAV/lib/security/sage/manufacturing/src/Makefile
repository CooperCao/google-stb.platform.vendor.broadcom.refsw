#############################################################################
#    (c)2013-2015 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
# $brcm_Workfile: Makefile $
# $brcm_Revision: ursr_integration/1 $
# $brcm_Date: 3/25/15 3:05p $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: /EchostarGlobalNexus/ursr/BSEAV/lib/sage/manufacturing/src/Makefile $
#
#############################################################################
######## localisation
ifeq (${URSR_TOP},)
ifneq (${NEXUS_TOP},)
URSR_TOP = ${NEXUS_TOP}/..
else
URSR_TOP = ../../../../../..
endif
endif

# Unified Reference Software path
NEXUS_TOP = $(URSR_TOP)/nexus
B_LIB_TOP ?= $(NEXUS_TOP)/lib
MAGNUM_TOP ?= ${NEXUS_TOP}/../magnum
BSEAV_TOP ?= ${NEXUS_TOP}/../BSEAV
SAGE_PUBLIC_TOP ?= ${BSEAV_TOP}/lib/security/sage
SRAI_TOP  ?= ${SAGE_PUBLIC_TOP}/srai
IMAGE_DIR ?= ${SAGE_PUBLIC_TOP}/manufacturing/image
EXPECTED_PATH ?= $(IMAGE_DIR)/$(IMAGE_NAME)


# Include library definitions
include $(B_LIB_TOP)/build/b_lib_defs.inc

include $(MAGNUM_TOP)/syslib/sagelib/bsagelib_public.inc

#
# Identify this library
#
B_LIBS += B_SAGE_MANUFACTURING_LIB

#
# Define source files in this library
#
B_SAGE_MANUFACTURING_LIB_SOURCES = $(BSEAV_TOP)/lib/security/sage/manufacturoing/src/sage_manufacturing_api.c

#
# Define public include path(s) for this library
#
B_SAGE_MANUFACTURING_LIB_PUBLIC_INCLUDES =  $(BSEAV_TOP)/lib/security/sage/manufacturing/include

#
# Define private include path(s) for this library
#
B_SAGE_MANUFACTURING_LIB_PRIVATE_INCLUDES = $(BSEAV_TOP)/lib/security/sage/manufacturing/src \
					   ${SRAI_TOP}/include \
					   ${BSEAV_TOP}/lib/security/common_drm/include/tl \
					   ${SAGE_PUBLIC_TOP}/manufacturing \
					   ${BSAGELIB_INCLUDES}



B_SAGE_MANUFACTURING_LIB_LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai -lsrai \
				   -L${NEXUS_BIN_DIR} -lnexus -lm


# Instruct build system to build only this library and name it libb_dcc
B_BUILD_LIBS := B_SAGE_MANUFACTURING_LIB

B_LIB_OUTPUT_NAME := b_sage_manufacturing

# Include library rules
include $(B_LIB_TOP)/build/b_lib_rules.inc

B_LIB_CFLAGS += -std=c99 -g
