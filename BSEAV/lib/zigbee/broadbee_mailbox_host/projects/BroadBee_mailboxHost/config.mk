##############################################################################
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
##############################################################################
#
# DESCRIPTION:
#       Project make custom configuration.
#
##########################################################################################

### Override the default Stack root directory. Set it to the actual Stack root directory
### path relative to the directory where this particular Makefile is saved.
override STACKDIR = ../../stack


### Enable echoing of shell commands in recipes (just remove the comment to enable).
override SILENT ?= @


### Override the default Project platform (remove the comment from a single row).
# override PLATFORM = __i386__
# override PLATFORM = __ML507__
override PLATFORM = __ARM__

### Override default tools for Project build: C compiler, assembler, linker, python.
# override CC = another_mcc
# override AS = another_mcc
# override LD = another_mcc
# override PY = another_python

CROSS_COMPILE ?= /opt/toolchains/stbgcc-4.8-1.0/bin/arm-linux-
override CC = $(CROSS_COMPILE)gcc
override LD = $(CROSS_COMPILE)gcc
#override PY=/tools/oss/packages/x86_64-rhel5/python/3.3.1/bin/python
override PY=/usr/bin/python3

### Override tools' default flags for Project build: C compiler, assembler, linker.
override CFLAGS  = -c -O0 -g -std=gnu99 -Wattributes -fpack-struct=4 -fshort-enums
#-fshort-enums
override ASFLAGS =
# Linker flags.

override LDFLAGS += -g -Xlinker --no-enum-size-warning

override LIBS += $(HOST_PATH)/rf4ce_registration/libsqlite3.a -lpthread -ldl

### Override the default auxiliary directories. It is recommended to use private
### directories for each Project to save object and log files of Application and Stack
### instead of the Stack own auxiliary directories. If need to use directories from the
### outside of the Application root directory then complete assignments.
# override OBJDIR = ./somewhere/obj
# override LOGDIR = ./somewhere/log


### Set up the full set of Stack and Project components.
# The basic set of Stack and Project components is given in $(STACKDIR)/components.mk.
# The Stack Makefile script automatically parces that file and discovers all Stack and
# Project components. By default all components are to be build. But here are two lists
# may be provided in order to change the set of components to be built. Define two lists
# of components to be (1) excluded from and (2) reincluded into the automatically
# discovered list of components. Use '%' as the wildcard.
# Example 1: ignore the automatically discovered list and reinclude all ZBPRO components.
#   COMPEXCL = %
#   COMPINCL = z%
# Example 2: exclude all ZBPRO and RF4CE components except RF4CE_NWK and RF4CE_ZRC.
#   COMPEXCL = z% r%
#   COMPINCL = rnwk rzrc

# Exclude Security component and all ZigBee PRO and RF4CE components.
COMPEXCL += %
COMPINCL += sys hal mbx rrn prj common ict
ifneq ($(BYPASS_RPC),y)
COMPINCL += rpc
endif


### Set up the full set of source files.
# The basic set of source files of a particular component is discovered automatically by
# the Stack Makefile script according to the platform selection (see comments above) and
# the component root directory defined in the $(STACKDIR)/components.mk file. By default
# all source files found are included into the build set. A component may provide its own
# exclude.mk files (to be automatically found they must reside in component's source
# directories $(STACKDIR)/$(COMPDIR)/src and $(STACKDIR)/$(COMPDIR)/$(PLATFDIR)/src) that
# define a list of source files to be excluded conditionally according to some specific
# settings provided for the component by this particular Project (i.e., somewhere inside
# this Makefile or its includes). Also here are two lists may be provided in order to
# change the set of source files to be compiled. Define two lists of source files to be
# (1) excluded from and (2) included into the automatically discovered lists of source
# files for each component. These two lists are common for all components, but if a unit
# listed in them does not belong to a component it will not be included into such
# component build. Use '%' as the wildcard. Files may be specified without extension.

# Exclude units not necessary for MAC Sertification Tests.
SRCEXCL += bbSys% zigbee_core_sim zigbee_rpc_client shell sqlite3 bbPcUsart bbHal% bbExtPowerFilterKey bbExtTestEngine
SRCINCL += bbSysEvent bbSysDbg bbSysFsm bbSysMemMan bbSysDbgMm bbSysPayload bbSysStackData bbSysTaskScheduler bbSysTimeoutTask bbSysQueue bbHalSystemTimer bbHalTask

# Make RF4CE_TEST enabled by default.
RF4CE_TEST=n
ifeq ($(RF4CE_TEST), y)
override CDEFS = \
    RF4CE_TEST
endif

ifeq ($(BYPASS_RPC),y)
override CDEFS += \
    BYPASS_RPC
endif

#SRCEXCL += zigbee_file_server_cc
SRCINCL += zigbee_file

### Override the default set of additional compiler flags to be passed with '-D' key.
override CDEFS += \
    _HOST_  \
    __i386__ \
    SERVER \
    MAILBOX_UNIT_TEST  \
    _MAILBOX_WRAPPERS_TEST_ENGINE_=1 \
    _DEBUG_ \
    _DEBUG_COMPLEX_ \
    _DEBUG_LOG_ \
    _DEBUG_HARNESSLOG_=2 \
    _DEBUG_FILELINE_ \
    _MAILBOX_WRAPPERS_MAC_=1 \
    _MAILBOX_WRAPPERS_NWK_=1 \
    _MAILBOX_WRAPPERS_APS_=1 \
    _MAILBOX_WRAPPERS_ZDO_=2 \
    _MAILBOX_WRAPPERS_TC_=2 \
    _MAILBOX_WRAPPERS_ZCL_=2 \
    _MAILBOX_WRAPPERS_PROFILE_=2 \
    _MAILBOX_WRAPPERS_ZHA_=2 \
    _MAILBOX_INTERFACE_=1 \
    _RF4CE_ \
    RF4CE_ZRC_WAKEUP_ACTION_CODE_SUPPORT \
    RF4CE_TARGET \
    USE_RF4CE_PROFILE_ZRC1 \
    USE_RF4CE_PROFILE_GDP=1 \
    USE_RF4CE_PROFILE_ZRC2  \
    _ZBPRO_ \
    USE_ZBPRO_PROFILE_ZHA \
    _ZHA_PROFILE_CIE_DEVICE_IMPLEMENTATION_ \
    _MAC_BAN_TABLE_SIZE_=1 \
    _RELEASE_TO_HOST_ \
    _PHY_SAP_IEEE_ \
    _PHY_TEST_HOST_INTERFACE_ \
    _USE_ASYNC_UART_

#
# FIXME
ifeq ($(USE_RF4CE_PROFILE_ZRC2), y)
override CDEFS += USE_RF4CE_PROFILE_ZRC2
endif

#    USE_RF4CE_PROFILE_ZRC2
### Override the default configuration of the Stack.
# override _DEBUG_ = 0
# override _PROFILE_ = 0
override _HAL_USE_PRNG_ = 0
override _ZBPRO_ = 1
override _RF4CE_ = 1
override RF4CE_TARGET = 1
override USE_RF4CE_PROFILE_ZRC1 = 1
override USE_RF4CE_PROFILE_ZRC2 = 1


### Define the binary output file name.
BINTARGET := BroadBee_mailboxHost


### eof config.mk ########################################################################
