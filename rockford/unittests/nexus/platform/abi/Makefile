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

ifdef B_REFSW_REAL_NEXUS_TOP
NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
ifdef COMSPEC
# Any DOS environment
NEXUS_TOP ?= $(shell cd ../../../../../nexus && cd)
else
NEXUS_TOP ?= $(shell cd ../../../../../nexus; pwd)
endif
QEMU_DIR?="/projects/stbdevrefsw/vsilyaev/work/qemu/build"

APPS = \
    verify_abi

include ${NEXUS_TOP}/../rockford/unittests/nexus/common/Makefile.inc

CC_64:=aarch64-linux-gcc
CC_32:=arm-linux-gcc
VERIFY_MODULE ?= core
VERIFY_INCLUDES ?= ${NEXUS_TOP}/modules/${VERIFY_MODULE}/include/*.h



ifeq ($(B_REFSW_REAL_MAKE),1)

ABI_APPS := $(addprefix ${APPS}, .64 .32)
all: report
TOOLCHAIN_DIR := $(shell dirname $(dir $(shell which ${CC})) )

CFLAGS += -I.

.PHONY: verify_types.inc

verify_types.inc: nexus
	(cd ${NEXUS_TOP}/..;${CC} -MM ${CFLAGS} ${INCLUDES_DIR} ${VERIFY_INCLUDES}|perl -I nexus/build/tools/common -I nexus/build/tools/abiverify nexus/build/tools/abiverify/bapi_build.pl -mode translate -stdin -class_list=${B_REFSW_OBJ_ROOT}/nexus/core/syncthunk/class_list.inc -output $(abspath $@) UNKNOWN ${B_REFSW_OBJ_ROOT}/nexus/core/syncthunk ${VERIFY_INCLUDES} )

INCLUDES_DIR := $(addprefix -I,$(sort $(dir ${VERIFY_INCLUDES})))

%.64: %.c verify_types.inc
	@echo ${CC_64} $(notdir $<)
	${Q_}${CC_64} -DVERIFY -o $@ $< ${CFLAGS} ${INCLUDES_DIR}

%.32: %.c verify_types.inc
	@echo ${CC_32} $(notdir $<)
	${Q_}${CC_32} -DVERIFY -o $@ $< ${CFLAGS} ${INCLUDES_DIR}

report: ${ABI_APPS}
	${QEMU_DIR}/arm-linux-user/qemu-arm -L ${TOOLCHAIN_DIR}/arm-linux-gnueabihf/sys-root verify_abi.32 >verify_abi.32.csv
	${QEMU_DIR}/aarch64-linux-user/qemu-aarch64 -L ${TOOLCHAIN_DIR}/aarch64-linux-gnu/sys-root verify_abi.64 >verify_abi.64.csv
	diff -u verify_abi.32.csv verify_abi.64.csv

endif
