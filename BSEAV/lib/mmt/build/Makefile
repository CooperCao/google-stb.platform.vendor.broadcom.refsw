############################################################################
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
###########################################################################

include ../../../api/build/tools.mak
BSEAV := $(shell cd "../../.." && ${PWD})
MAGNUM = $(BSEAV)/../magnum
WITH_NEXUS?=y
ifeq (${WITH_NEXUS},y)
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
else
B_REFSW_OBJ_DIR ?= obj.${B_REFSW_ARCH}
endif
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}
B_REFSW_STRICT_PRINTF_FORMAT=y


include ${BSEAV}/lib/utils/batom.inc
include ${BSEAV}/lib/bfile/bfile.inc
include ${BSEAV}/lib/bfile/bfile.inc
include ${BSEAV}/lib/mmt/bmmt.inc

BMEDIA_SOURCES := ${BSEAV}/lib/media/bmedia_util.c
TEST_MODULES := BMMT BMEDIA
TEST_MODULES += BFILE_STDIO
B_MODULES := $(sort ${TEST_MODULES} $(foreach module, $(TEST_MODULES), $($(module)_DEPENDS)))
B_TEST_INC := $(addprefix -I,$(sort $(foreach module, $(B_MODULES), $($(module)_INCLUDES))))
B_TEST_CFLAGS := $(addprefix -D,$(sort $(foreach module, $(B_MODULES), $($(module)_DEFINES))))
B_TEST_SOURCES = $(sort $(foreach module, $(B_MODULES), $($(module)_SOURCES)))
B_TEST_SRCS := $(notdir ${B_TEST_SOURCES})
vpath %c $(dir ${B_TEST_SOURCES})

# build media app
APP = mmt

CFLAGS += -W -Wall
CFLAGS += -I ../../../api/include
ifeq (${WITH_NEXUS},y)
NEXUS_TOP = $(BSEAV)/../nexus
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
CFLAGS += -DWITH_NEXUS=1 $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
LDFLAGS += ${NEXUS_LDFLAGS} $(NEXUS_LD_LIBRARIES)
else
CFLAGS += $(B_REFSW_CFLAGS) $(B_REFSW_GENERIC_MAGNUM_CFLAGS) $(B_REFSW_MAGNUM_INCLUDE_DIRS)
CFLAGS += ${B_REFSW_C_ONLY_FLAGS} ${B_REFSW_LINUXKERNEL_CFLAGS}
endif
CFLAGS += ${B_TEST_CFLAGS} ${B_TEST_INC}
CFLAGS += -fstrict-aliasing -frename-registers
CFLAGS += -D_POSIX_C_SOURCE=199309L
CFLAGS += -Os
CFLAGS += -DBKNI_TRACK_MALLOCS=1
ifeq ($(B_REFSW_ARCH),i386-linux)
#CFLAGS += -m32
endif
#CFLAGS += -S
SRCS += ${B_TEST_SRCS}

ifneq ($(APP),)
SRCS += bmmt_test.c
endif

CFLAGS += -I ../../utils
vpath %.c ../../utils ${MAGNUM}/commonutils/lst/test

include $(BSEAV)/build/refsw_inc.mak

ODIR = ${B_REFSW_OBJ_ROOT}/BSEAV/lib/mmt/build/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)

OS = ${B_REFSW_OS}
ifeq (${WITH_NEXUS},y)
else
# Need some Magnum basemodules
include $(MAGNUM)/basemodules/kni/bkni.inc
include $(MAGNUM)/basemodules/dbg/bdbg.inc
include $(MAGNUM)/commonutils/lst/blst.inc
CFLAGS += $(addprefix -I, $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES)))
CFLAGS += $(addprefix -D, $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))
SRCS += $(notdir $(foreach module, $(MAGNUM_MODULES), $($(module)_SOURCES)))
vpath %.c $(dir $(foreach module, $(MAGNUM_MODULES), $($(module)_SOURCES)))
endif

ifeq ($(B_REFSW_ARCH),i386-linux)
#LDFLAGS += -m32
endif

ifeq (${PROBE_ONLY},y)
CFLAGS += -DB_PROBE_ONLY=1
endif
ifeq (${OS},linuxuser)
LDFLAGS += -lpthread -lrt
#LDFLAGS += -static
endif

OBJS += ${MAGNUM_OBJS}

include ${BSEAV}/api/build/rules.mak

ifeq (${WITH_NEXUS},y)
install: application
	${CP} ${APP_IMAGE} $(NEXUS_BIN_DIR)
endif
