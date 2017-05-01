#############################################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#############################################################################

# pmlib.inc
# This file should be included by applications linking to pmlib.
#
# Inputs: PMLIB_TOP
# Outputs: PMLIB_SRCS, PMLIB_CFLAGS
#
# Work out Kernel Version from Kernel Source's Top Level Makefile
#
K_VERSION       := $(shell grep -m1 "^VERSION" $(LINUX)/Makefile | cut -d\  -f3)
K_PATCHLEVEL    := $(shell grep -m1 "^PATCHLEVEL" $(LINUX)/Makefile | cut -d\  -f3)
K_SUBLEVEL      := $(shell grep -m1 "^SUBLEVEL" $(LINUX)/Makefile | cut -d\  -f3)
K_EXTRAVERSION  := $(shell grep -m1 "^EXTRAVERSION" $(LINUX)/Makefile | cut -d-  -f2-)
KERNEL_VERSION  := $(K_VERSION).$(K_PATCHLEVEL).$(K_SUBLEVEL)-$(K_EXTRAVERSION)

pmlib-check-linux-version = $(shell expr \( $(K_VERSION) \* 65536 + $(K_PATCHLEVEL) \* 256 + $(K_SUBLEVEL) \) \>= \( $(1) \* 65536 + $(2) \* 256 + $(3) \))


ifeq ($(call pmlib-check-linux-version,3,14,1),1)   # if kernel >= 3.14.1
    PMLIB_DIR=314
else
ifeq ($(call pmlib-check-linux-version,2,6,30),1)   # if kernel >= 2.6.30
    PMLIB_DIR=26
else                                    # if kernel < 2.6.30
    $(error PMlib not supported!!)
endif
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}

PMLIB_LIBDIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/$(B_REFSW_ARCH)/pmlib
PMLIB_CFLAGS += -I$(PMLIB_TOP)/$(PMLIB_DIR)
PMLIB_CFLAGS += -DPMLIB_VER=$(PMLIB_DIR)
PMLIB_LDFLAGS += -L$(PMLIB_LIBDIR) -lpmlib
