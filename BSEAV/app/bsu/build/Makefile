###############################################################################
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
###############################################################################
include Makefile.cfl
export B_REFSW_OS=ucos
export B_REFSW_UCOS_VERSION=3

ifeq ($(B_REFSW_UCOS_VERSION),1)
    export B_REFSW_UCOS_SUBDIR_NAME=ucos
else
    ifeq ($(B_REFSW_UCOS_VERSION),2)
        export B_REFSW_UCOS_SUBDIR_NAME=ucos_ii
    else
        ifeq ($(B_REFSW_UCOS_VERSION),3)
            export B_REFSW_UCOS_SUBDIR_NAME=ucos_iii
        endif
    endif
endif

#
# Reference Software Makefile
#
# This Makefile will build 3 things
# 1) driver(s)
# 2) nexus api (shared or static)
# 3) libs
# 4) BSU
#

#####
###First thing we must include the tools
#####

include ../../../../nexus/build/os/ucos/os_tools.inc

# REFERENCE VERSION
# BOLT_DIR_NAME = ../../../reference/modified/bolt-v1.00
# CFE_DIR_NAME = ../../../reference/modified/CFE_15_2

# GIT VERSION
# BOLT_DIR_NAME = ../../bolt

# OFFICIAL RELEASE VERSION
BOLT_DIR_NAME = bolt_v1.08
CFE_DIR_NAME = CFE_15_4

BSEAV = $(shell cd "../../.." && ${PWD})
NEXUS_TOP = $(shell cd "../../../../nexus" && ${PWD})
BSEAV_TOP = $(shell cd "../../../../BSEAV" && ${PWD})
ROCKFORD_TOP = $(shell cd "../../../../rockford" && ${PWD})
MAGNUM_TOP = $(shell cd "../../../../magnum" && ${PWD})
BOLT_TOP = $(shell cd "../../../../$(BOLT_DIR_NAME)" && ${PWD})
CFE_TOP = $(shell cd "../../../../$(CFE_DIR_NAME)" && ${PWD})
ifeq ($(NEXUS_PLATFORM),)
# PLATFORM is deprecated
NEXUS_PLATFORM = $(PLATFORM)
endif

# If NEWLIB points to a NEWLIB libc library, the build will link to that library.
#NEWLIB = /fe_lab/agin/src/newlib-arm/arm-elf/newlib/libc.a
#NEWLIB = /fe_lab/agin/src/git/eie-io7/lib/newlib/arm-elf/newlib/libc.a
#NEWLIB = /fe_lab/agin/src/newlib-arm2/arm-elf/newlib/libc.a

# Use MIPS SDE Standard library for malloc and printf.  Not the version
# exported by the CFE BSU API.  Only relevant for MIPS SDE builds.
MIPS_SDE_STDLIB = 1

# Use local printf.  For ARM only, for now.
USE_LOCAL_PRINTF = 1

# Additional heap provided by BSU at the end of physical memory.  This is
# optional.  To take advantage of this extra heap, you must review the
# appropriate nexus_platform_9xxxx.c file to review the function
# NEXUS_Platform_P_GetDefaultSettings().  In that function, determine the amount
# of memory (if any) which can be reduced for the BSU heap.  Once that amount
# is determined, modify the function, NEXUS_Platform_P_GetHostMemory() in the
# nexus_platform_os.c file to fake NEXUS into thinking it has that amount of
# physical memory less than it really has.
#
# 97584 example
# MIPS_BSU_HEAP_ADDR = 0x53000000
# MIPS_BSU_HEAP_SIZE = 208*1024*1024

ifeq ($(NEXUS_PLATFORM),97439)
# Extra heap need for MoCA
MIPS_BSU_HEAP_ADDR = 0x09200000
MIPS_BSU_HEAP_SIZE = 0x06e00000
else
MIPS_BSU_HEAP_ADDR = 0
MIPS_BSU_HEAP_SIZE = 0
endif

# Enable memory test, if MIPS_BSU_HEAP_SIZE is non-zero.
MIPS_BSU_HEAP_TEST = 0

#########
#### Figure out processor
#########

# Newest plat scripts set B_REFSW_ARCH to mipsel-linux.  Need the following, to fix MIPS builds
ifeq ($(B_REFSW_ARCH),mipsel-linux)
    B_REFSW_ARCH=sde
    export B_REFSW_ARCH
endif

ifeq ($(B_REFSW_ARCH), arm-linux)
    ifeq ($(TOOLCHAIN_PATH),)
        TOOLCHAIN_PATH=/opt/toolchains/stbgcc-4.8-1.0
    endif
else
    ifeq ($(TOOLCHAIN_PATH),)
        TOOLCHAIN_PATH=/opt/toolchains/sde_5.03
    endif
endif

ifeq ($(BSU_BIN_DIR),)
# location of final binaries
BSU_BIN_ROOT = $(NEXUS_TOP)/../obj.${NEXUS_PLATFORM}
BSU_BIN_DIR = $(BSU_BIN_ROOT)/BSEAV/bin${BSU_BIN_DIR_SUFFIX}
endif

ifeq ($(NEXUS_MODE),)
# MODE is deprecated
NEXUS_MODE = $(MODE)
endif


#########
#### Figure out ARCH
#########
ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mips-linux
export B_REFSW_ARCH
endif
ifneq ($(B_REFSW_DEBUG),n)
DEBUG_SUFFIX=debug
else
DEBUG_SUFFIX=release
endif

#########
#### Include OS LIB and NEXUS .inc
#########
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

# Get system-specific CFLAGS and LDFLAGS based on SYSTEM, PLATFORM, DEBUG
include $(BSEAV)/build/refsw_inc.mak

#################
############ Staging Area setup directory. It will be destroyed later
#################
STAGING_DIR = ${BSU_BIN_DIR}/setup

##############
##### BSU_ODIR keeps binary files separate
##### BCHP_REV is not included, so there should be no #ifdef BCHP_REV in BSU
##############
BSU_ODIR = $(NEXUS_PLATFORM).$(B_REFSW_ARCH).$(DEBUG_SUFFIX)
APP = $(BSU_ODIR)/bsu

#######
#### Font Directories
#######
ifeq ($(B_REFSW_ARCH),mips-linux)
PRERENDERED_FONTS = ../fonts/mswebfonts/be/*.bwin_font
else # goes for ARM as well which is LE only
PRERENDERED_FONTS = ../fonts/mswebfonts/le/*.bwin_font
endif

.PHONY: bin_install api all $(BSU_ODIR)

# The name of the installation tar ball , Change to BSU
#
BUILDDATE = `date +%Y%m%d`
BSU_TAR_BALL = refsw-$(BUILDDATE).$(NEXUS_PLATFORM)-$(SYSTEM)
BSU_TAR_BALL := $(BSU_TAR_BALL)-BSU
BSU_TAR_BALL := $(BSU_TAR_BALL).bin.tgz

#
# Set B_REFSW_SHAREABLE based on a variety of flags
#
ifeq ($(filter y,$(STATIC_BSU)),y)
B_REFSW_SHAREABLE=n
endif

# bin_install is the default rule.
# 'all' builds everything but doesn't perform bin_install or install.
# bin_install copies only binaries into BSEAV/bin. It allows you to
#  run from that directory without having your configuration files overwritten.
# install builds an installation tarball in BSEAV/bin.
bin_install:  $(BSU_BIN_DIR) api all 
	@echo [Install... binaries]
# Copy over the nexus script into the correct Directory
	$(Q_)$(CP) $(APP).bin $(BSU_BIN_DIR)
	$(Q_)$(CP) $(APP).bin.gz $(BSU_BIN_DIR)
	$(Q_)$(CP) $(APP) $(BSU_BIN_DIR)
	$(Q_)$(CP) $(APP).gz $(BSU_BIN_DIR)
	$(Q_)$(CP) $(NEXUS_BIN_DIR)/nexus $(BSU_BIN_DIR)
	$(Q_)$(CP) start $(BSU_BIN_DIR)
	@chmod +x $(BSU_BIN_DIR)/nexus
	@chmod +x $(BSU_BIN_DIR)/start
	@echo [Install... BSU Complete]




$(BSU_ODIR):
	$(Q_)$(MKDIR) $(BSU_ODIR)
	
##########
##### NEXUS API Libraries
##########

##########
#####always build main.o because it contains __DATE__
##########
build_date:
	$(Q_)$(RM) -f $(BSU_ODIR)/main.o

clean-BSU:
	@$(RM) -rf $(BSU_ODIR)
	@$(RM) -rf $(BSU_LIB_DIR)
	@$(RM) -rf $(BSEAV)/../obj.$(NEXUS_PLATFORM)

clean-moca:
ifeq ($(MOCA), src)
	make -C $(ROCKFORD_TOP)/bsp/common/moca2 clean
endif

clean-ucos:
ifeq ($(UCOS), src)
	make -C $(ROCKFORD_TOP)/bsp/common/$(B_REFSW_UCOS_SUBDIR_NAME) clean
endif

print-BSU-tarball:
	@echo $(BSU_TAR_BALL)

$(BSU_BIN_DIR):
	$(Q_)${MKDIR} ${BSU_BIN_DIR}

ifeq ($(BUILD),)
    BUILD=normal
endif

BUILD_UART = A

UCOS ?= lib
BSEAV_DIR = $(BSEAV_TOP)
ROCKFORD_DIR = $(ROCKFORD_TOP)
MAGNUM_DIR = $(MAGNUM_TOP)
NEXUS_DIR = $(NEXUS_TOP)
BOLT_DIR = $(BOLT_TOP)
CFE_DIR = $(CFE_TOP)

ifeq ($(B_REFSW_ARCH), arm-linux)
    CPU = arm
else
    ifeq ($(filter $(NEXUS_PLATFORM),97208 97358 97360 97401 97468 97550 97552), $(NEXUS_PLATFORM))
        CPU = bmips3300
    else
        ifeq ($(filter $(NEXUS_PLATFORM),97019 97125 97205 97228 97230 97231 97335 97340 97342 97362 973625 97405 97408 97466 97563 975635 97584 975845), $(NEXUS_PLATFORM))
            CPU = bmips4380
        else
            ifeq ($(filter $(NEXUS_PLATFORM),97241 93380vms 97344 97346 97410 97420 97422 97425 97428 97429 97435), $(NEXUS_PLATFORM))
                CPU = bmips5000
            else
                ifeq ($(NEXUS_PLATFORM),97325)
                    CPU = mti34kf
                else
                    Unknown platform
                endif
            endif
        endif
    endif
endif

ifeq ($(filter $(NEXUS_PLATFORM),97205 97405 97466), $(NEXUS_PLATFORM))
    CHIP_4501_VERSION_LOWERCASE = c0
    TUNER_CHIP = 3420
    VSB_CHIP = 3510
endif

CPU_UPPERCASE = $(shell awk 'BEGIN{print toupper("$(CPU)")}')

ifeq ($(BE), 1)
    CFLAGS += -DBRCM_ENDIAN_LITTLE=0
    CFLAGS += -D__MIPSEB
    export NEXUS_EXTRA_CFLAGS += -D__MIPSEB -DBSU
    CFLAGS += -D__BIG_ENDIAN
    CFLAGS += -mbig-endian
    ENDIAN_FLAG = -EB
    LDFLAGS += -EB
    ENDIAN_SUFFIX = be
    ifeq ($(B_REFSW_ARCH), arm-linux)
        LIBPATHS = $(TOOLCHAIN_PATH)/lib/gcc/arm-linux-gnueabihf/4.8.4
    else
        STUB_LIB = $(NEXUS_TOP)/../rockford/bsp/common/lib/sde/stubs_be.o
        LIBPATHS = $(TOOLCHAIN_PATH)/sde/lib/mips32/sof
    endif
else
    CFLAGS += -DBRCM_ENDIAN_LITTLE=1
    CFLAGS += -D__MIPSEL
    export NEXUS_EXTRA_CFLAGS += -D__MIPSEL -DBSU
    CFLAGS += -D__LITTLE_ENDIAN
    ENDIAN_FLAG = -EL
    LDFLAGS += -EL
    ENDIAN_SUFFIX = le
    ifeq ($(B_REFSW_ARCH), arm-linux)
        LIBPATHS = $(TOOLCHAIN_PATH)/lib/gcc/arm-linux-gnueabihf/4.8.4
    else
        STUB_LIB = $(NEXUS_TOP)/../rockford/bsp/common/lib/sde/stubs_le.o
        LIBPATHS = $(TOOLCHAIN_PATH)/sde/lib/el/mips32/sof
    endif
endif

CFLAGS += \
    -DBCM_BOARD=$(NEXUS_PLATFORM) \
    -DBSU \
    -D$(CPU_UPPERCASE)

export NEXUS_EXTRA_CFLAGS += -D_LIB_NO_MACROS_

BSU_OBJECTS += \
    bsu.o \
    bsu_error.o \
    bsu_filesys.o \
    bsu_net.o \
    bsu_nvram.o \
    bsu_security.o \
    bsu_task.o \
    int1.o \
    os_cpu_c.o

ifeq ($(B_REFSW_UCOS_VERSION),1)
    BSU_OBJECTS += \
        b_ucos.o \
        cpu.o
endif

ifeq ($(NEWLIB),)
    ifeq ($(B_REFSW_ARCH), arm-linux)
        BSU_OBJECTS += \
            bsu_stdlib.o \
            bsu_string.o \
            bsu_string2.o
        CFLAGS += -I$(ROCKFORD_DIR)/bsp/common/lib/bootloader

        ifeq ($(USE_LOCAL_PRINTF),1)
            BSU_OBJECTS += \
                console.o \
                dev_16550_uart.o \
                bsu_stdio.o
            BCHP_VER_LOWERCASE = $(shell awk 'BEGIN{print tolower("$(BCHP_VER)")}')
            CFLAGS += -DUSE_LOCAL_PRINTF=1 -DCFG_ARCH_ARM -DARM_V7 -I$(BOLT_DIR)/gen/$(BCHP_CHIP)$(BCHP_VER_LOWERCASE)
        else
            CFLAGS += -DUSE_LOCAL_PRINTF=0
        endif
    else
        ifeq ($(MIPS_SDE_STDLIB),1)
            CFLAGS += -DMIPS_SDE_STDLIB
        else
            BSU_OBJECTS += \
                stdlib.o \
                string.o \
                string2.o
            CFLAGS += -I$(ROCKFORD_DIR)/bsp/common/lib/bootloader
        endif
    endif
else
    CFLAGS += \
        export NEXUS_EXTRA_CFLAGS += -DNEWLIB
endif

ifneq ($(B_REFSW_ARCH), arm-linux)
    BSU_OBJECTS += \
        cache_util.o \
        setup_tlb.o
endif

ifeq ($(B_REFSW_ARCH), arm-linux)
    ASM_SRC += init.s vectors.s
else
    ASM_SRC += reset_noos.s cpuctrl.s
endif

ifeq ($(B_REFSW_UCOS_VERSION),1)
    VPATH += \
        $(ROCKFORD_DIR)/bsp/common/ucos/Hdi_4/board/common/ucos \
        $(ROCKFORD_DIR)/bsp/common/ucos/Hdi_4/board/97358/bsp \
        $(ROCKFORD_DIR)/bsp/common/ucos/misc
else
    ifeq ($(B_REFSW_UCOS_VERSION),3)
        VPATH += \
            $(NEXUS_DIR)/platforms/common/src/ucos_iii/ \
            $(NEXUS_DIR)/platforms/common/src/ucos_iii/common \
            $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Cfg \
            $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Cfg/Template \
            $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Source \
            $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU \
            $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/BSP/Template \
            $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/Cfg/Template \
            $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-LIB

        ifeq ($(B_REFSW_ARCH), arm-linux)
            VPATH += \
                $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Ports/ARM-Cortex-A15/Generic/GNU \
                $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/ARM-Cortex-A15/GNU
        else
            VPATH += \
                $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Ports/Broadcom-MIPS/SDE \
                $(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/Broadcom-MIPS
        endif
    endif
endif

BSU_LIB_DIR = $(ROCKFORD_TOP)/bsp/$(BSU_ODIR)
BSU_LIB_NAME = libbsu_$(ENDIAN_SUFFIX).a
BSU_LIB = $(BSU_LIB_DIR)/$(BSU_LIB_NAME)

ifeq ($(B_REFSW_UCOS_VERSION),1)
    ASM_SRC += \
        cpu_a.s \
        reset.s
else
    ifeq ($(B_REFSW_UCOS_VERSION),3)
        ASM_SRC += \
            os_cpu_a.s \
            cpu_a.s
    endif
endif

ifeq ($(BUILD_UART), A)
    CFLAGS += -DDIAGS_UART_A
else
    ifeq ($(BUILD_UART), B)
        CFLAGS += -DDIAGS_UART_B
    else
        ifeq ($(BUILD_UART), C)
            CFLAGS += -DDIAGS_UART_C
        endif
    endif
endif

VPATH += \
    $(ROCKFORD_DIR)/bsp/common/lib/bootloader

ifeq ($(B_REFSW_ARCH), arm-linux)
    VPATH += \
        $(ROCKFORD_DIR)/bsp/common/arm
else
    VPATH += \
        $(CFE_DIR)/bsp \
        $(ROCKFORD_DIR)/bsp/common/mips \
        $(ROCKFORD_DIR)/bsp/common_bsp/mips
endif

ifeq ($(NEXUS_PLATFORM),97439)
    BUILD_MOCA_SUPPORT=2
else
    BUILD_MOCA_SUPPORT=0
endif

ifeq ($(NEXUS_PLATFORM),97241)
    ifeq ($(NEXUS_PLATFORM_7241_DCSFBTSFF),y)
        BUILD_QAM_SUPPORT=1
    endif
endif

# ifeq ($(filter $(BOARD),97231cbsv 97231csff 97231fbtsff 97241dcsfbtsff 97425vms_sff 97435sv 97445vms_sff 97552cbsv 97584sv 97584wifi), $(BOARD))
ifeq ($(filter $(NEXUS_PLATFORM),97435 97445 97584), $(NEXUS_PLATFORM))
    BUILD_QAM_SUPPORT=1
endif

ifeq ($(BUILD_MOCA_SUPPORT),1)
    CFLAGS += \
        -DMOCA_SUPPORT \
        -DMOCA1_SUPPORT
endif

ifeq ($(BUILD_MOCA_SUPPORT),2)
    CFLAGS += \
        -DMOCA_SUPPORT \
        -DMOCA2_SUPPORT
endif

ifeq ($(BUILD_QAM_SUPPORT),1)
    CFLAGS += \
        -DBSU_QAM_SUPPORT
endif

ifeq ($(filter $(BUILD_MOCA_SUPPORT),1 2), $(BUILD_MOCA_SUPPORT))
    VPATH += \
        $(ROCKFORD_DIR)/bsp/common/moca2 \
        $(ROCKFORD_DIR)/bsp/common/moca2/bmoca/ucos \
        $(ROCKFORD_DIR)/bsp/common/moca2/include/ucos \
        $(ROCKFORD_DIR)/bsp/common/moca2/ucos
    CFLAGS += \
        -I$(ROCKFORD_DIR)/bsp/common/moca2/include/ucos \
        -I$(ROCKFORD_DIR)/bsp/common/moca2/ucos \
        -I$(MAGNUM_DIR)/basemodules/int \
        -I$(MAGNUM_DIR)/basemodules/int/7439 \
        -DARM
    APP_OBJECTS += \
        bmoca.o \
        bmocadriver.o \
        moca20core-gen23.o
endif

ifeq ($(BUILD), test)
    VPATH += \
        $(BSEAV_DIR)/app/bsu/src/test
    CFLAGS += \
        -DBSU_TEST \
        -I$(BSEAV_DIR)/app/bsu/src/test

    APP_OBJECTS += \
        bsu_colorbar_test.o \
        bsu_ethernet_test.o \
        bsu_ir_input_test.o \
        bsu_message_test.o \
        bsu_nvram_test.o \
        bsu_prompt.o \
        bsu_test.o \
        bsu_uart_test.o \
        bsu_usb_test.o

    ifeq ($(filter $(BUILD_MOCA_SUPPORT),1 2), $(BUILD_MOCA_SUPPORT))
        BUILD_MOCA_TEST=1
    endif

    # ifeq ($(filter $(BOARD),97218tc2rsff 97231eusff 97231eusff_v20 97550 97552cbsv 97563rsff2l 97563sv 975639sff), $(BOARD))
    ifeq ($(NEXUS_PLATFORM),97563)
        BUILD_OFDM_TEST=1
    endif

    ifeq ($(BUILD_QAM_SUPPORT),1)
        BUILD_QAM_TEST=1
    endif

    # ifeq ($(filter $(BOARD),97346dbsmb 97358dbsv 97358sff 97360sff 97360sv 97362sff 97362sv 97425dbs), $(BOARD))
    ifeq ($(filter $(NEXUS_PLATFORM),97346 97360 97362 97425), $(NEXUS_PLATFORM))
        BUILD_SAT_TEST=1
    endif

    ifeq ($(BUILD_MOCA_TEST),1)
        APP_OBJECTS += \
            bsu_moca_test.o
        CFLAGS += \
            -DBSU_MOCA_TEST
    endif

    ifeq ($(BUILD_OFDM_TEST),1)
        APP_OBJECTS += \
            bsu_ofdm_test.o
        CFLAGS += \
            -DBSU_OFDM_TEST
    endif

    ifeq ($(BUILD_QAM_TEST),1)
        APP_OBJECTS += \
            bsu_qam_test.o
        CFLAGS += \
            -DBSU_QAM_TEST
    endif

    ifeq ($(BUILD_SAT_TEST),1)
        APP_OBJECTS += \
            bsu_satellite_test.o
        CFLAGS += \
            -DBSU_SAT_TEST
    endif
else
    VPATH += \
        $(BSEAV_DIR)/app/bsu/src/gui
    CFLAGS += \
        -DBSU_GUI \
        -I$(BSEAV_DIR)/app/bsu/src/gui

    APP_OBJECTS += \
        bsu_bfont.o \
        bsu_gui.o \
        bsu_picdecoder.o
endif

ifeq ($(B_REFSW_ARCH), arm-linux)
    VPATH += \
        $(BSEAV_DIR)/app/bsu/build/arm

    BSU_OBJECTS += \
        interrupts.o \
        lib.o \
        uart.o
else
    ifeq ($(CPU),bmips5000)
        ASM_SRC += tlb_r5k.s cache_lib_5xxx.s
    else
        ifeq ($(CPU),bmips4380)
            ASM_SRC += tlb_4380.s cache_lib_4380.s
        else
            ifeq ($(CPU),bmips3300)
                ASM_SRC += tlb_3300.s cache_lib_3300.s
            else
                $(warning unknown CPU)
            endif
        endif
    endif

    BSU_OBJECTS += \
        boot.o

    # Extra for linking with SDE libc library
    ifeq ($(MIPS_SDE_STDLIB),1)
        CFLAGS += -DMIPS_SDE_STDLIB
        BSU_OBJECTS += \
            queue.o \
            sbd.o \
            serial.o \
            termio.o
    endif
endif

ifneq ($(MIPS_BSU_HEAP_SIZE),0)
    ifeq ($(B_REFSW_ARCH), arm-linux)
        VPATH += \
            $(ROCKFORD_DIR)/bsp/common/lib/bootloader/arm
    else
        VPATH += \
            $(ROCKFORD_DIR)/bsp/common/lib/bootloader/mips
    endif

    CFLAGS += \
        -DMIPS_BSU_HEAP_ADDR=$(MIPS_BSU_HEAP_ADDR) \
        -DMIPS_BSU_HEAP_SIZE=$(MIPS_BSU_HEAP_SIZE) \
        -DCFG_ARCH_ARM \
        -DARM_V7
    ifeq ($(MIPS_BSU_HEAP_TEST),1)
        CFLAGS += -DMIPS_BSU_HEAP_TEST
    endif
    ifneq ($(B_REFSW_ARCH), arm-linux)
        CFLAGS += -DUSE_BSU_MALLOC_NAME
    endif
    BSU_OBJECTS += \
        bsu_memory.o \
        bsu_malloc.o
endif

# Default includes
export NEXUS_EXTRA_CFLAGS += \
    -I$(BSEAV_DIR)/app/bsu/src \
    -I$(ROCKFORD_DIR)/bsp/common/lib/bootloader

ifeq ($(B_REFSW_ARCH), arm-linux)
    export NEXUS_EXTRA_CFLAGS += \
        -I$(BOLT_DIR)/include \
        -I$(BOLT_DIR)/include/arm
else
    export NEXUS_EXTRA_CFLAGS += \
        -I$(CFE_DIR)/bsp \
        -I$(CFE_DIR)/cfe/arch/include \
        -I$(CFE_DIR)/cfe/include \
        -I$(CFE_DIR)/cfe/net \
        -I$(ROCKFORD_DIR)/bsp/common/mips \
        -I$(ROCKFORD_DIR)/bsp/common_bsp/mips
endif

ifeq ($(B_REFSW_UCOS_VERSION),1)
    export NEXUS_EXTRA_CFLAGS += \
        -DUCOS \
        -DUCOS_VERSION=1 \
        -I$(ROCKFORD_DIR)/bsp/common/ucos/kernel2/CPU \
        -I$(ROCKFORD_DIR)/bsp/common/ucos/kernel2/Code \
        -I$(ROCKFORD_DIR)/bsp/common/ucos/nds/Issue22/inc \
        -I$(ROCKFORD_DIR)/bsp/common/ucos/nds/Issue22/kernel2/inc \
        -I$(ROCKFORD_DIR)/bsp/common/ucos/Hdi_4/board/common/ucos \
        -I$(ROCKFORD_DIR)/bsp/common/ucos/Hdi_4/board/97358/bsp \
        -I$(ROCKFORD_DIR)/bsp/common/ucos/Hdi_4/base \
        -I$(ROCKFORD_DIR)/bsp/common/ucos/misc
else
    ifeq ($(B_REFSW_UCOS_VERSION),3)
        export NEXUS_EXTRA_CFLAGS += \
            -DUCOS \
            -DUCOS_VERSION=3 \
            -I$(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Source \
            -I$(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Cfg/Template \
            -I$(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU \
            -I$(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/Cfg/Template \
            -I$(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-LIB

        ifeq ($(B_REFSW_ARCH), arm-linux)
            export NEXUS_EXTRA_CFLAGS += \
                -I$(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Ports/ARM-Cortex-A15/Generic/GNU \
                -I$(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/ARM-Cortex-A15/GNU
        else
            export NEXUS_EXTRA_CFLAGS += \
                -I$(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uCOS-III/Ports/Broadcom-MIPS/SDE \
                -I$(ROCKFORD_DIR)/bsp/common/ucos_iii/Micrium/Software/uC-CPU/Broadcom-MIPS
        endif
    endif
endif

# No WEBHIF timer exists for 7250
ifeq ($(filter $(NEXUS_PLATFORM),97145 97250 97364 97366), $(NEXUS_PLATFORM))
    export NEXUS_EXTRA_CFLAGS += -DNEXUS_TMR_EXCLUSION_MASK=0x8 \
                                 -DBSU_USE_UPG_TIMER
endif

##########
##### uC/OS
##########

ifeq ($(B_REFSW_ARCH), arm-linux)
    UCOS_LIB_DIR = $(ROCKFORD_TOP)/bsp/common/lib/gnu/arm
else
    UCOS_LIB_DIR = $(ROCKFORD_TOP)/bsp/common/lib/sde
endif

ifeq ($(B_REFSW_UCOS_VERSION),1)
    UCOS_LIB_NAME = libucos_$(ENDIAN_SUFFIX).a
else
    ifeq ($(B_REFSW_UCOS_VERSION),2)
        UCOS_LIB_NAME = libucos_ii_$(ENDIAN_SUFFIX).a
    else
        UCOS_LIB_NAME = libucos_iii_$(ENDIAN_SUFFIX).a
    endif
endif
UCOS_LIB = $(UCOS_LIB_DIR)/$(UCOS_LIB_NAME)

make_ucos:
ifeq ($(UCOS), src)
	make -C $(ROCKFORD_TOP)/bsp/common/$(B_REFSW_UCOS_SUBDIR_NAME)
endif

########## 
##### MoCA
##########

ifeq ($(B_REFSW_ARCH), arm-linux)
    MOCA_LIB_DIR = $(ROCKFORD_TOP)/bsp/common/lib/gnu/arm
    MOCA_CPU = arm
else
    MOCA_LIB_DIR = $(ROCKFORD_TOP)/bsp/common/lib/sde
    MOCA_CPU = mips
endif

# Just store the library here, for now.
MOCA_LIB_DIR = $(ROCKFORD_TOP)/bsp/common/moca2

ifeq ($(BUILD_MOCA_SUPPORT),0)
    MOCA_LIB=
else
    ifeq ($(B_REFSW_UCOS_VERSION),1)
        MOCA_LIB_NAME = libmoca_$(ENDIAN_SUFFIX).a
    else
        ifeq ($(B_REFSW_UCOS_VERSION),2)
            MOCA_LIB_NAME = libmoca_ii_$(ENDIAN_SUFFIX).a
        else
            MOCA_LIB_NAME = libmoca_iii_$(ENDIAN_SUFFIX).a
        endif
    endif
    MOCA_LIB = $(MOCA_LIB_DIR)/$(MOCA_LIB_NAME)
endif

make_moca:
ifeq ($(MOCA), src)
	make -C $(ROCKFORD_TOP)/bsp/common/moca2
endif

##########
##### Rule for all and clean
##########
all: $(BSU_BIN_DIR) make_moca make_ucos $(APP)
	${Q_}$(CP) -f $(APP).bin $(BSU_BIN_DIR)
	${Q_}$(CP) -f $(APP).bin.gz $(BSU_BIN_DIR)
	${Q_}$(CP) -f $(APP) $(BSU_BIN_DIR)
	${Q_}$(CP) -f $(APP).gz $(BSU_BIN_DIR)

clean: api-clean clean-BSU clean-ucos clean-moca

api:  $(BSU_BIN_DIR) $(BSU_ODIR)
	@echo [Build ... Nexus]
#	${Q_}$(MAKE) -C $(NEXUS_TOP)/lib/os INSTALL_DIR=$(BSU_BIN_DIR) 
	${Q_}$(MAKE) -C $(NEXUS_TOP)/build  INSTALL_DIR=$(BSU_BIN_DIR)

api-install: driver-install 
	@echo [Build ... Nexus]
	${Q_}$(MAKE) -C $(NEXUS_TOP)/build  NEXUS_BIN_DIR=$(STAGING_DIR)


api-clean:
	${Q_}$(MAKE) -C $(NEXUS_TOP)/build clean


################
######## Source Files for compiling
###############

# Default to little endian.
ifeq ($(BE),)
    BE=0
endif

##################
######## BSU objs
#################

LDFLAGS += $(addprefix $(BSU_ODIR)/,$(APP_OBJECTS))

# At this point, we have the assembly files from the CFE also.
BSU_OBJECTS += $(ASM_SRC:.s=.o)

#####
### BSU Dirs
#####
BSU_DIRS += ../src 
BSU_DIRS += ../../../../rockford/bsp/common

ifeq ($(B_REFSW_ARCH), arm-linux)
    BSU_DIRS += ../../../../rockford/bsp/common/arm
endif

    include ${BSEAV}/../magnum/commonutils/vlc/bvlc.inc
#    include ${BSEAV}/lib/media/bmedia.inc
#    include ${BSEAV}/lib/utils/batom.inc
#    include ${BSEAV}/lib/bfile/bfile.inc
ifneq ($(MEDIA_RMFF_SUPPORT),n)
        -include ${BSEAV}/lib/media/rmff/brmff.inc
endif
ifneq ($(MEDIA_FLV_SUPPORT),n)
        -include ${BSEAV}/lib/media/flv/bflv.inc
endif
#        include ${BSEAV}/lib/bfile/bfile.inc
        BSU_MODULES := BMEDIA_PROBE BATOM BFILE_STDIO BVLC 
	CFLAGS += -DPVR_SUPPORT

	# Direct bcmplayer access is needed for Video::indexToTime.
	CFLAGS += -I$(BSEAV)/lib/bcmplayer/include

	B_BSU_INC := $(addprefix -I,$(sort $(foreach module, $(BSU_MODULES), $($(module)_INCLUDES))))
	B_BSU_CFLAGS := $(addprefix -D,$(sort $(foreach module, $(BSU_MODULES), $($(module)_DEFINES))))
	B_BSU_SOURCES = $(sort $(foreach module, $(BSU_MODULES), $($(module)_SOURCES)))
	B_BSU_OBJS := $(patsubst %.c,%.o,$(notdir ${B_BSU_SOURCES}))
	vpath %c $(dir ${B_BSU_SOURCES})

	BSU_OBJECTS += ${B_BSU_OBJS}
	CFLAGS += ${B_BSU_CFLAGS} ${B_BSU_INC}



# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Always build with debug
CFLAGS += -MD -g
CFLAGS += -DBDBG_DEBUG_BUILD
CFLAGS += -MMD

BSU_INCLUDE_DIRS = $(addprefix -I,$(BSU_DIRS))
VPATH += $(BSU_DIRS)

####
## Include the B_OS abstraction layer
####
#BSU_INCLUDE_DIRS += -I$(BSEAV)/../nexus/lib/os/include
#BSU_INCLUDE_DIRS += -I$(BSEAV)/../nexus/lib/os/include/linuxuser

##########
#### Link nexus libraries
##########
LDFLAGS += -lnexus -L${NEXUS_BIN_DIR}
CFLAGS += $(BSU_INCLUDE_DIRS) 
#C_ONLY_FLAGS := -std=c99 -std=c89 -Wstrict-prototypes
CXXFLAGS += -fno-rtti -fno-exceptions 

vpath %.cpp $(VPATH)
vpath %.c $(VPATH)
vpath %.h $(VPATH)

CFLAGS += -ffunction-sections -fdata-sections

LDFLAGS += $(addprefix -L, $(LIBPATHS))
LDFLAGS += -Map $(BSU_ODIR)/bsu.map
AFLAGS += $(CFLAGS)

ifeq ($(B_REFSW_ARCH), arm-linux)
    LDFLAGS += -T arm/init.lds
    LDFLAGS += -nostdlib -static --gc-sections
else
    STACK_ADRS      = 0x89000000
    STACK_SIZE      = 64*1024
    ROM_TEXT_ADRS   = 0x88010000

    #
    # Original location:
    #   STACK_ADRS      = 0x84000000
    #   ROM_TEXT_ADRS   = 0x80010000
    #
    # Need to modify nexus_platform_os.c also:
    #   pMemory->osRegion[0].base = 0x0c000000;
    #   pMemory->osRegion[0].length = 0x04000000; /* 64MB */
    #
    # And need to modify nexus_platform_os.c:
    #   pMemory->memc[0].region[0].base = pMemory->memc[0].region[0].original_base = 0x08000000;
    #

    TEXT_FLAG = -Ttext $(ROM_TEXT_ADRS)
    ADDR_FLAG = -DROM_TEXT_ADRS=$(ROM_TEXT_ADRS) -DSTACK_ADRS=$(STACK_ADRS) -DSTACK_SIZE=$(STACK_SIZE)

    AFLAGS += -mcpu=4kc -mips32 -msoft-float '-Afloat(soft)'
    AFLAGS += \
        $(ENDIAN_FLAG) \
        -G 0 -O \
        -x assembler-with-cpp \
        -D_ASMLANGUAGE \
        $(ADDR_FLAG)

    LDFLAGS += --architecture mips --stats $(TEXT_FLAG) --gc-sections
endif

ifeq ($(B_REFSW_ARCH), arm-linux)
	AR = arm-linux-ar
	RANLIB = arm-linux-ranlib
else
	AR = sde-ar
	RANLIB = sde-ranlib
endif

$(BSU_LIB_DIR):
	$(Q_)$(MKDIR) $(BSU_LIB_DIR)

$(BSU_LIB): $(BSU_LIB_DIR) $(addprefix $(BSU_ODIR)/,$(BSU_OBJECTS))
	$(AR) cr $(BSU_LIB) $(addprefix $(BSU_ODIR)/,$(BSU_OBJECTS))
	$(RANLIB) $(BSU_LIB)

moca20core-gen23.c:
	$(Q_)tar xvf $(ROCKFORD_TOP)/bsp/common/moca2/moca20core-gen23.c.tgz

# Implicit rule for building local app
$(APP): api $(BSU_ODIR) build_date $(addprefix $(BSU_ODIR)/,$(APP_OBJECTS)) $(STUB_LIB) $(BSU_LIB) $(UCOS_LIB) $(MOCA_LIB)
	@echo [Linking... $(notdir $@)]
ifeq ($(B_REFSW_ARCH), arm-linux)
	$(Q_)$(LD) $(LDFLAGS) --start-group $(BSU_LIB) $(UCOS_LIB) $(MOCA_LIB) $(STUB_LIB) $(NEWLIB) -lgcc --end-group -o $@
	$(Q_)$(OBJCOPY) -O binary $@ $@.bin
else
	$(Q_)$(LD) $(LDFLAGS) --start-group $(BSU_LIB) $(UCOS_LIB) $(MOCA_LIB) $(STUB_LIB) -lc -le -lm --end-group -o $@
	$(Q_)$(CONV) -f bin -o $@.bin $@
endif
	$(GZIP) -cf9 $@.bin > $@.bin.gz
	$(GZIP) -cf9 $@ > $@.gz

%.o: %.cpp
	@echo [Compile... $(notdir $<)]
	${Q_}$(CXX) $(CXXFLAGS) $(filter-out $(C_ONLY_FLAGS),$(CFLAGS)) -c $< -o $(BSU_ODIR)/$@

%.o: %.c
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) $(CFLAGS) -c $< -o $(BSU_ODIR)/$@

$(BSU_ODIR)/%.o: %.c
	@echo [Compile ... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS) $(C_ONLY_FLAGS)
ifneq ($(B_REFSW_ARCH), arm-linux)
	$(Q_)$(PERL) $(NEXUS_TOP)/build/os/ucos/fix_dep.pl $(dir $@) $(notdir $(@:.o=.d)) $(patsubst %.o,%.d,$@)
endif

$(BSU_ODIR)/%.o: %.cpp
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CXX) -c -o $@ $< $(filter-out $(C_ONLY_FLAGS),$(CFLAGS)) $(CXXFLAGS)
ifneq ($(B_REFSW_ARCH), arm-linux)
	$(Q_)$(PERL) $(NEXUS_TOP)/build/os/ucos/fix_dep.pl $(dir $@) $(notdir $(@:.o=.d)) $(patsubst %.o,%.d,$@)
endif

$(BSU_ODIR)/%.o: %.s
	@echo [Assemble... $(notdir $<)]
	$(Q_)$(AS) $(AFLAGS) -MMD -c -o $@ $< 
ifneq ($(B_REFSW_ARCH), arm-linux)
	$(Q_)$(PERL) $(NEXUS_TOP)/build/os/ucos/fix_dep.pl $(dir $@) $(notdir $(@:.o=.d)) $(patsubst %.o,%.d,$@)
endif

$(BSU_ODIR)/%.o: %.S
	@echo [Assemble... $(notdir $<)]
	$(Q_)$(AS) $(AFLAGS) -MMD -c -o $@ $<
ifneq ($(B_REFSW_ARCH), arm-linux)
	$(Q_)$(PERL) $(NEXUS_TOP)/build/os/ucos/fix_dep.pl $(dir $@) $(notdir $(@:.o=.d)) $(patsubst %.o,%.d,$@)
endif

#Only Cleans Playback-IP library not SSL, only 
min-clean: playback_ip_lib_min_clean clean-BSU api-clean
	@echo [..... Minimal Cleaning Done....]

#
# This is the main installation rule.
# It will build the binaries and utilities, and then
# perform the actual installation procedure.
#
install:  bin_install $(STAGING_DIR) install-binaries make-tarball	

#
# Install drivers, libraries and apps
#
.PHONY: install-binaries
install-binaries: api-install playback_ip_lib_install install-config-files install-cable-files

#
# STAGING_DIR is phony because we always want to remove it and
# recreate it.
#
.PHONY: $(STAGING_DIR)
$(STAGING_DIR):
	@$(RM) -rf $(STAGING_DIR)
	@${MKDIR} $(STAGING_DIR)

#
# Copy everything into temporary STAGING_DIR and tar it up.
# The final tarball is left in BSU_BIN_DIR.
#

make-tarball: install-support-files
	@echo [Build..... tarball]
	@(cd $(STAGING_DIR);tar -pzckf $(BSU_BIN_DIR)/${BSU_TAR_BALL} .)
	@$(RM) -rf $(STAGING_DIR)
	@echo ""
	@echo "*********************************************"
	@echo "* Installation tarball created in $(BSU_BIN_DIR)."
	@echo "*"
	@echo "* To complete installation perform the"
	@echo "* following operations on the settop box:"
	@echo "*"
	@echo "* gunzip -c ${BSU_TAR_BALL} | tar xvpf -"
	@echo "* start BSU"
	@echo "*********************************************"
	@echo ""

#
# Install drivers, libraries and apps
#
.PHONY: install-support-files
install-support-files: 
	@echo [Install... scripts]
	$(Q_)$(CP) start $(STAGING_DIR)
	$(Q_)chmod +x $(STAGING_DIR)/start
	$(Q_)$(CP) $(NEXUS_BIN_DIR)/nexus $(STAGING_DIR)
	$(Q_)chmod +x $(STAGING_DIR)/nexus
	$(Q_)$(CP) $(APP) $(STAGING_DIR)

	@echo [Install... fonts]
	@${MKDIR} $(STAGING_DIR)/fonts
	@$(CP) $(PRERENDERED_FONTS) $(STAGING_DIR)/fonts

####
## install Config Files
####
install-config-files:
	$(Q_)cd ../; \
		$(CP) -r images/bcmlogo.png images scripts $(STAGING_DIR)
	$(Q_)$(CP) -r ../scripts/BSU.cfg $(STAGING_DIR)/BSU.cfg
	$(Q_)if [ -f ../channelmap/$(PLATFORM).channels.xml ] ; then  \
		$(CP) ../channelmap/$(PLATFORM).channels.xml $(STAGING_DIR)/channels.xml; \
	else  \
		$(CP) ../channelmap/channels.xml $(STAGING_DIR)/channels.xml; \
	fi;
	@cd $(STAGING_DIR); chmod 664 BSU.cfg

bin-install-config-files:
# Copy over config files so we can run without doing a make install
# you can run this manually with "make bin-install-config-files" 
	@echo  [...... Installing config files to $(BSU_BIN_DIR) ......]
	$(Q_)cd ../; \
		$(CP) -r images/bcmlogo.png images scripts $(BSU_BIN_DIR)
	$(Q_)$(CP) -r ../scripts/BSU.cfg $(BSU_BIN_DIR)/BSU.cfg
	$(Q_)if [ -f ../channelmap/$(PLATFORM).channels.xml ] ; then  \
		$(CP) ../channelmap/$(PLATFORM).channels.xml $(BSU_BIN_DIR)/channels.xml; \
	else  \
		$(CP) ../channelmap/channels.xml $(BSU_BIN_DIR)/channels.xml; \
	fi;
	@cd $(BSU_BIN_DIR); chmod 664 BSU.cfg
	@echo [Install... fonts]
	@${MKDIR} $(BSU_BIN_DIR)/fonts
	@$(CP) $(PRERENDERED_FONTS) $(BSU_BIN_DIR)/fonts



-include $(BSU_ODIR)/*.d
