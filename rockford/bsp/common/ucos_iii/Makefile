################################################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
################################################################################
#
################################################################################
#       Makefile
#
#   please use the following style to build the UCOS library.
#
#    	make
#
################################################################################
#
ifeq ($(B_REFSW_ARCH),arm-linux)
    $(warning Building for arm linux toolchain)
else
    ifeq ($(B_REFSW_ARCH),mipsel-linux)
        $(warning Building for mips linux toolchain)
    else
        $(warning Building for mips sde toolchain)
    endif
endif

#
################################################################################
# Endian
################################################################################
#
ifeq ($(BE), 1)
    $(warning Building for big endian)
    ENDIAN_SUFFIX = be
    ENDIAN_FLAG = -EB
else
    $(warning Building for little endian)
    ENDIAN_SUFFIX = le
    ENDIAN_FLAG = -EL
endif

#
################################################################################
# TARGET
################################################################################
#
TARGET = bcm$(BOARD_NUM)_diags_${ENDIAN_SUFFIX}

#
#-----------------------------------------------------------
#       Directives
#-----------------------------------------------------------
# CFLAGS        = Compiler Directives
# AFLAGS        = Assembler Directives
# LFLAGS        = Linker Directives
#
#-----------------------------------------------------------
#       Tools Defined
#-----------------------------------------------------------
# RM            = remove file program
# CP            = copy file program
# CC            = cross compiler
# AS            = assembler
# LD            = Linker
#

#
################################################################################
# Determine Configuration flags
################################################################################
#

STACK_ADRS      = 0x84000000
ROM_TEXT_ADRS   = 0x80010000
TEXT_FLAG = -Ttext $(ROM_TEXT_ADRS)
ADDR_FLAG = -DROM_TEXT_ADRS=$(ROM_TEXT_ADRS) -DSTACK_ADRS=$(STACK_ADRS)

#
################################################################################
# Tools
################################################################################
#
RM=rm
CP=cp

#
# Algorithmics Tools
#

CC = $(B_REFSW_ARCH)-gcc
AS = $(B_REFSW_ARCH)-gcc
LD = $(B_REFSW_ARCH)-ld
AR = $(B_REFSW_ARCH)-ar
RANLIB = $(B_REFSW_ARCH)-ranlib

CFLAGS = -std=c89 -Wall -W -Wstrict-prototypes -Wno-unused-value -c -O2 -g -ffunction-sections -fdata-sections -MMD

ifeq ($(B_REFSW_ARCH),arm-linux)
    CFLAGS += -nostdlib -ffreestanding -fno-short-enums -march=armv7-a -mabi=aapcs -mthumb -MD
#    CFLAGS += -msoft-float -mfloat-abi=softfp
    ifeq ($(BE), 1)
        CFLAGS += -mbig-endian
    endif
else
    ifeq ($(B_REFSW_ARCH),mipsel-linux)
        CFLAGS += $(ENDIAN_FLAG) -G 0 -DMIPS_SDE -pedantic -mips32 -msoft-float '-Afloat(soft)'
    else
        CFLAGS += $(ENDIAN_FLAG) -G 0 -DMIPS_SDE -pedantic -mcpu=4kc -mips32 -msoft-float '-Afloat(soft)'
    endif
endif

ROCKFORD_DIR  = ../../..

DIAGS_VPATH += \
    $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU \
    $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/BSP/Template \
    $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/Cfg/Template \
    $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-LIB \
    $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Source \
    $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Cfg/Template

ifeq ($(B_REFSW_ARCH),arm-linux)
    DIAGS_VPATH += \
        $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Ports/ARM-Cortex-A15/Generic/GNU \
        $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/ARM-Cortex-A15/GNU
else
    DIAGS_VPATH += \
        $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/Broadcom-MIPS \
        $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Ports/Broadcom-MIPS/SDE
endif

VPATH = $(DIAGS_VPATH)
ifeq ($(B_REFSW_ARCH),arm-linux)
    OBJDIR = arm.$(ENDIAN_SUFFIX).obj
else
    OBJDIR = mips.$(ENDIAN_SUFFIX).obj
endif

#
################################################################################
# Make Target Library
################################################################################
#

ifeq ($(B_REFSW_ARCH),arm-linux)
    UCOS_LIB_DIR = $(ROCKFORD_DIR)/bsp/common/lib/gnu/arm
else
    ifeq ($(B_REFSW_ARCH),mipsel-linux)
        UCOS_LIB_DIR = $(ROCKFORD_DIR)/bsp/common/lib/gnu/mips
    else
        UCOS_LIB_DIR = $(ROCKFORD_DIR)/bsp/common/lib/sde
    endif
endif

UCOS_LIB_NAME = libucos_iii_$(ENDIAN_SUFFIX).a
UCOS_LIB = $(UCOS_LIB_DIR)/$(UCOS_LIB_NAME)

all : Makefile $(OBJDIR) $(UCOS_LIB)

$(OBJDIR):
	mkdir $(OBJDIR)

#
################################################################################
# Make UCOS Library
################################################################################
#

UCOS_SRC = \
    cpu_bsp.c \
    cpu_core.c \
    os_cfg_app.c \
    os_core.c \
    os_dbg.c \
    os_flag.c \
    os_int.c \
    os_mem.c \
    os_msg.c \
    os_mutex.c \
    os_prio.c \
    os_q.c \
    os_sem.c \
    os_stat.c \
    os_task.c \
    os_tick.c \
    os_time.c \
    os_tmr.c \
    os_var.c

UCOS_OBJS = $(UCOS_SRC:.c=.o)

$(UCOS_LIB):  $(OBJDIR) $(addprefix $(OBJDIR)/,$(UCOS_OBJS))
	$(AR) cr $(UCOS_LIB) $(addprefix $(OBJDIR)/, $(UCOS_OBJS))
	$(RANLIB) $(UCOS_LIB)

#
################################################################################
#  Rules for making
################################################################################
#

vpath %.h $(DIAGS_VPATH)
vpath %.c $(DIAGS_VPATH)
vpath %.o $(OBJDIR)

DIAGS_INCLUDE=$(patsubst %,-I%,$(subst :, ,$(DIAGS_VPATH)))

CFLAGS += $(DIAGS_INCLUDE)

$(OBJDIR)/%.o: %.c
	@echo '$(CC) $(notdir $<)'
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: %.cpp
	@echo '$(CC) $(notdir $<)'
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: %.s
	@echo '$(AS) $(notdir $<)'
ifeq ($(B_REFSW_ARCH),arm-linux)
	@$(CC) $(CFLAGS) $< -o $@
else
	@$(CC) $(CFLAGS) $(ADDR_FLAG) -D_ASMLANGUAGE -x assembler-with-cpp $< -o $@
#	@mv $(@:.o=.d) $(OBJDIR)
endif

# include individual depend files
-include $(OBJDIR)/*.d

#
################################################################################
#  Rules for clean
################################################################################
#

clean :
	$(RM) -rf $(OBJDIR)
