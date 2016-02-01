############################################################################
#     Copyright (c) 2011 Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################################

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
