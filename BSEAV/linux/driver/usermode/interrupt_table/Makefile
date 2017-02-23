#*******************************************************************************
#* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#*
#* This program is the proprietary software of Broadcom and/or its
#* licensors, and may only be used, duplicated, modified or distributed pursuant
#* to the terms and conditions of a separate, written license agreement executed
#* between you and Broadcom (an "Authorized License").  Except as set forth in
#* an Authorized License, Broadcom grants no license (express or implied), right
#* to use, or waiver of any kind with respect to the Software, and Broadcom
#* expressly reserves all rights in and to the Software and all intellectual
#* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#*
#* Except as expressly set forth in the Authorized License,
#*
#* 1. This program, including its structure, sequence and organization,
#*    constitutes the valuable trade secrets of Broadcom, and you shall use all
#*    reasonable efforts to protect the confidentiality thereof, and to use
#*    this information only in connection with your use of Broadcom integrated
#*    circuit products.
#*
#* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#*
#* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#*******************************************************************************
B_REFSW_ROOT := $(realpath ${CURDIR}/../../../../..)
MAGNUM=${B_REFSW_ROOT}/magnum

ifeq ($(NEXUS_PLATFORM),)
NEXUS_PLATFORM=$(PLATFORM)
endif
B_REFSW_OBJ_DIR ?= obj.$(NEXUS_PLATFORM)
B_REFSW_OBJ_ROOT ?= ${B_REFSW_ROOT}/${B_REFSW_OBJ_DIR}
OBJDIR = ${B_REFSW_OBJ_ROOT}/$(subst ${B_REFSW_ROOT}/,,${CURDIR})

override B_REFSW_OS:=linuxuser
override B_REFSW_DEBUG=y

ifneq (${MAKECMDGOALS}, clean)
ifndef BCHP_CHIP
# nexus can translate NEXUS_PLATFORM into BCHP_CHIP, but nexus should not be a hard dependency for bcmdriver
-include ${B_REFSW_ROOT}/nexus/platforms/common/build/nexus_platforms.inc
ifndef BCHP_CHIP
$(error BCHP_CHIP must be defined)
endif
endif
endif

BCHP_DEFINES := BCHP_CHIP=$(BCHP_CHIP) BCHP_VER=BCHP_VER_$(BCHP_VER)

include $(MAGNUM)/basemodules/chp/bchp.inc
include $(MAGNUM)/basemodules/dbg/bdbg.inc
include $(MAGNUM)/basemodules/err/berr.inc
include $(MAGNUM)/basemodules/int/bint.inc
include $(MAGNUM)/basemodules/kni/bkni.inc
include $(MAGNUM)/basemodules/reg/breg.inc
include $(MAGNUM)/basemodules/std/bstd.inc
include $(MAGNUM)/basemodules/tmr/btmr.inc
include $(MAGNUM)/commonutils/lst/blst.inc
include $(MAGNUM)/commonutils/vlc/bvlc.inc

CFLAGS += $(addprefix -I, $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES)))
CFLAGS += $(addprefix -D, $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))
SRCS += $(foreach module, $(MAGNUM_MODULES), $($(module)_SOURCES))
SRCS += print_table.c
CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE
CFLAGS += -DBDBG_P_LOG_SUPPORTED=0 -DHAS_NPTL=0

ifeq (${NEXUS_GISB_ARB},y)
CFLAGS += -DNEXUS_GISB_ARB
endif

HOST_CC?=cc
all: ${OBJDIR}/interrupt_table.c

${OBJDIR}/interrupt_table.c: ${OBJDIR}/print_table template.c
	@echo "[Making.... $(notdir $@) ]"
	@$^ $@

${OBJDIR}/exists:
	@mkdir -p $@

${OBJDIR}/print_table: ${SRCS} ${OBJDIR}/exists
	@echo "[Compile... $(notdir $@)]"
	@${HOST_CC} -o $@ ${CFLAGS} ${SRCS} -lpthread -lrt

clean:
	@rm -rf ${OBJDIR}
