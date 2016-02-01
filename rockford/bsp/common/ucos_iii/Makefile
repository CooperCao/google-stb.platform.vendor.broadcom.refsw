################################################################################
#
#       Copyright (c) 2005-2013, Broadcom Corporation.
#       All rights reserved.
#       Confidential Property of Broadcom Corporation.
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
    $(warning Building for arm)
    UCOS_CPU=arm
else
    $(warning Building for mips)
    UCOS_CPU=mips
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
ifeq ($(UCOS_CPU),arm)
    CC = arm-linux-gcc
    AS = arm-linux-gcc
    LD = arm-linux-ld
else
    CC = sde-gcc
    AS = sde-gcc
    LD = sde-ld
endif

CFLAGS = -std=c89 -Wall -W -Wstrict-prototypes -Wno-unused-value -c -O2 -g -ffunction-sections -fdata-sections -MMD

ifeq ($(UCOS_CPU),arm)
    CFLAGS += -nostdlib -ffreestanding -fno-short-enums -march=armv7-a -mabi=aapcs -mthumb -MD
#    CFLAGS += -msoft-float -mfloat-abi=softfp
    ifeq ($(BE), 1)
        CFLAGS += -mbig-endian
    endif
else
    CFLAGS += $(ENDIAN_FLAG) -G 0 -DMIPS_SDE -pedantic -mcpu=4kc -mips32 -msoft-float '-Afloat(soft)'
endif

ifeq ($(UCOS_CPU),mips)
    ifneq ($(OS), Windows_NT)
        ifeq ($(BE), 1)
            LIBPATHS = /opt/toolchains/sde_5.03/sde/lib/mips32/sof
        else
            LIBPATHS = /opt/toolchains/sde_5.03/sde/lib/el/mips32/sof
        endif
    else
        ifeq ($(BE), 1)
            LIBPATHS = $(LIBRARY_PATH)/mips32/sof
        else
            LIBPATHS = $(LIBRARY_PATH)/el/mips32/sof
        endif
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

ifeq ($(UCOS_CPU),arm)
    DIAGS_VPATH += \
        $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Ports/ARM-Cortex-A15/Generic/GNU \
        $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/ARM-Cortex-A15/GNU
else
    DIAGS_VPATH += \
        $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/Broadcom-MIPS \
        $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Ports/Broadcom-MIPS/SDE
endif

VPATH = $(DIAGS_VPATH)
ifeq ($(UCOS_CPU),arm)
    OBJDIR = arm.$(ENDIAN_SUFFIX).obj
else
    OBJDIR = mips.$(ENDIAN_SUFFIX).obj
endif

#
################################################################################
# Make Target Library
################################################################################
#

ifeq ($(UCOS_CPU),arm)
    UCOS_LIB_DIR = $(ROCKFORD_DIR)/bsp/common/lib/gnu/arm
else
    UCOS_LIB_DIR = $(ROCKFORD_DIR)/bsp/common/lib/sde
endif

UCOS_LIB_NAME = libucos_iii_$(ENDIAN_SUFFIX).a
UCOS_LIB = $(UCOS_LIB_DIR)/$(UCOS_LIB_NAME)

all : makefile $(OBJDIR) $(UCOS_LIB)

$(OBJDIR):
	mkdir $(OBJDIR)

ifeq ($(UCOS_CPU),arm)
    AR = arm-linux-ar
    RANLIB = arm-linux-ranlib
else
    AR = sde-ar
    RANLIB = sde-ranlib
endif

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
ifeq ($(UCOS_CPU),arm)
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
