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

################################################################
# THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED
# SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM.
# YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT
# SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# File:     makefile
# Description: makefile.
#
################################################################/
ASTRA_TOP_DIR ?= $(shell /bin/pwd)
OBJ_ROOT ?= $(ASTRA_TOP_DIR)/../../../../obj$(TZ_ARCH)/astra
ASTRA_TOP_OBJ_DIR ?= $(OBJ_ROOT)/linux/utils/astra_top

ifeq ($(TZ_ARCH),Arm64)
	G++ = aarch64-linux-g++
else ifeq ($(TZ_ARCH),Arm32)
	G++ = arm-linux-g++
endif

#
# Targets
#

ASTRA_TOP = astra_top
#
#
# Program src
#

ASTRA_TOP_SRCS = CmdLine.cpp astra_top.cpp
CFLAGS += -I$(ASTRA_TOP_DIR)

#
# Targets
#
TARGET = $(patsubst %,$(ASTRA_TOP_OBJ_DIR)/%,$(ASTRA_TOP))

all: $(TARGET)

$(TARGET):$(ASTRA_TOP_SRCS) | $(ASTRA_TOP_OBJ_DIR)
	$(G++) -std=c++11 $(CFLAGS) $(ASTRA_TOP_SRCS) -o $(TARGET)

$(ASTRA_TOP_OBJ_DIR) :
	mkdir -p $(ASTRA_TOP_OBJ_DIR)

clean:
	$(RM) -rf $(ASTRA_TOP_OBJ_DIR)
