##############################################################################
# (c) 2014 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
##############################################################################
#
# FILENAME: $Workfile: trunk/stack/default.mk $
#
# DESCRIPTION:
#   Stack make default configuration.
#
# $Revision: 3536 $
# $Date: 2014-09-11 07:21:52Z $
#
##########################################################################################


### Suppress echoing of shell commands in recipes by default.
SILENT ?= @


### Set up SoC as the default platform.
PLATFORM ?= __ML507__
# ... finally substitute with the following:
# PLATFORM ?= __SoC__


### Set up default tools for Project build: C compiler, assembler, linker, python.
CC = mcc
AS = mcc
LD = mcc
PY = python


### Set up the default tools flags: C compiler, assembler, linker.
CFLAGS  += -O0 -g -Hnocopyr -arc601 -Xbs -Xmpy16 -Xswap -Hlpc_width=8 -Hpc_width=24
ASFLAGS += -Hasopt=-g -Hnocopyr -arc601 -Xbs -Xmpy16 -Xswap
LDFLAGS += -Hldopt=-q -zallow_memory_overlap -e_start -arc601 -Xbs -Xmpy16 -Xswap -Bpc_width=24 -Bcopydata -Bzerobss -zpurgetext -Hldopt=-Xcompress_stats
# ... finally substitute with the following:
# CFLAGS  += -Os -Hnocopyr -arc601 -Xbs -Xmpy16 -Xswap -Hlpc_width=8 -Hpc_width=24
# ASFLAGS += -Hnocopyr -arc601 -Xbs -Xmpy16 -Xswap
# LDFLAGS += -Hhostlib= -Hldopt=-q -zallow_memory_overlap -e_start -arc601 -Xbs -Xmpy16 -Xswap -Bpc_width=24 -Bcopydata -Bzerobss -zpurgetext -Hldopt=-Xcompress_stats


### Set up default auxiliary directories.
OBJDIR ?= ./obj.$(NEXUS_PLATFORM)
LOGDIR ?= ./log


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

COMPEXCL +=
COMPINCL +=


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

SRCEXCL +=
SRCINCL +=


### Define additional compiler flags to be passed with '-D' key (i.e., definitions).
# To override the default settings use the following instruction in the project config.mk.
#   override CDEFS = ...
CDEFS ?= \
    __ML507__ \
    _DEBUG_ \
    _DEBUG_COMPLEX_ \
    _DEBUG_LOG_ \
    _DEBUG_HARNESSLOG_=2 \
    _DEBUG_CONSOLELOG_=1 \
    _DEBUG_STDOUTLOG_ \
    _DEBUG_FILELINE_ \
    _PROFILE_ \
    _HAL_USE_PRNG_ \
    SECURITY_EMU \
    _MEMORY_MANAGER_ \
    MM_DEBUG="DBG_FAIL|DBG_HALT|DBG_WARN|DBG_INFO" \
    MM_POOL_SIZE=2048 \
    _MAILBOX_INTERFACE_=0 \
    _MAILBOX_WRAPPERS_TEST_ENGINE_ \
    _MAILBOX_WRAPPERS_MAC_=1 \
    _MAILBOX_WRAPPERS_NWK_=1 \
    _MAILBOX_WRAPPERS_APS_=1 \
    _MAILBOX_WRAPPERS_ZDO_=2 \
    _MAILBOX_WRAPPERS_TC_=2 \
    _MAILBOX_WRAPPERS_ZCL_=2 \
    _MAILBOX_WRAPPERS_PROFILE_=2 \
    _MAILBOX_WRAPPERS_ZHA_=2 \
    _MAC_BAN_TABLE_SIZE_=4 \
    _ZBPRO_ \
    USE_ZBPRO_PROFILE_ZHA \
    _WORKAROUND_FOR_AT_BEHAVIOR_ \
    _RF4CE_ \
    RF4CE_TARGET \
    USE_RF4CE_NWK \
    USE_RF4CE_PROFILE_ZRC \
    USE_RF4CE_PROFILE_MSO \
    RF4CE_CUSTOM_COMPILE_RULES \
    RF4CE_NWKC_NODE_CAPABILITIES=0x07 \
    RF4CE_NWKC_VENDOR_IDENTIFIER=0x1234 \
    RF4CE_NWKC_VENDOR_STRING="\"BRDBEE\"" \
    ENABLE_RF4CE_FREQUENCY_AGILITY \
    RF4CE_NWK_NVM_ENABLED \
    RF4CE_NWK_GU_DISCOVERY \
    RF4CE_NWK_INCLUDE_FA_WARNINGS


### Define the default configuration of the Stack.
# To override the default settings use the following instruction in the project config.mk.
#   override _DEBUG_ = 0
# N.B. Do not put a comment into the same line with the overridden variable after its assigned value!

# --- Override with =0 if no _DEBUG_xxx_ switches are listed in CDEFS
_DEBUG_ ?= 1

# --- Override with =0 if _PROFILE_ is switched off in CDEFS
_PROFILE_ ?= 1

# --- Override with =0 if _HAL_USE_TRNG_ is used instead of _HAL_USE_PRNG_ in CDEFS
_HAL_USE_PRNG_ ?= 1

# --- Override with =0 if SECURITY_EMU is switched off in CDEFS
SECURITY_EMU ?= 1

# --- Override with =1 if _MAILBOX_INTERFACE_=1 (not =0) is in CDEFS
_MAILBOX_INTERFACE_ ?= 0

# --- Override with =0 if _ZBPRO_ is switched off in CDEFS
_ZBPRO_ ?= 1

# --- Override with =0 if _RF4CE_ is switched off in CDEFS
_RF4CE_ ?= 1

# --- Override with =0 if RF4CE_CONTROLLER is used instead of RF4CE_TARGET in CDEFS
RF4CE_TARGET ?= 1

# --- Override with =1 if USE_RF4CE_PROFILE_ZRC1 (1.1) is included in CDEFS
USE_RF4CE_PROFILE_ZRC1 ?= 1

# --- Override with =0 if USE_RF4CE_PROFILE_ZRC (2.0) is switched off in CDEFS
USE_RF4CE_PROFILE_ZRC2 ?= 1

# --- Override with =0 if USE_RF4CE_PROFILE_MSO is switched off in CDEFS
USE_RF4CE_PROFILE_MSO ?= 1

# --- Override with =0 if a component shall be compiled from separate source files
_SYS_INTEGRATED_ ?= 1
_MAIL_INTEGRATED_ ?= 1
_NVM_INTEGRATED_ ?= 1
_RPC_INTEGRATED_ ?= 1
_SEC_INTEGRATED_ ?= 1
_HAL_INTEGRATED_ ?= 1
_SOC_INTEGRATED_ ?= 1
_ML507_INTEGRATED_ ?= 1
_IEEE_INTEGRATED_ ?= 1
_ZBPRO_NWK_INTEGRATED_ ?= 1
_ZBPRO_APS_INTEGRATED_ ?= 1
_ZBPRO_ZDO_INTEGRATED_ ?= 1
_ZBPRO_TC_INTEGRATED_ ?= 1
_ZBPRO_SSP_INTEGRATED_ ?= 1
_ZBPRO_ZCL_INTEGRATED_ ?= 1
_ZBPRO_ZHA_INTEGRATED_ ?= 1
_RF4CE_NWK_INTEGRATED_ ?= 1
_RF4CE_PM_INTEGRATED_ ?= 1
_RF4CE_ZRC_INTEGRATED_ ?= 1
_RF4CE_MSO_INTEGRATED_ ?= 1

### eof default.mk #######################################################################
