############################################################
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
#
# Module Description:
#   This file defines the OS-specific flags for building in linuxkernel mode.
#
############################################################

LINUX ?= /opt/brcm/linux
LINUX_OUT ?= $(LINUX)

noop: all

# parse the Linux Makefile
LINUX_VERSION := $(shell grep -m 1 '^VERSION = ' ${LINUX}/Makefile | awk '{print $$3}')
LINUX_PATCHLEVEL := $(shell grep -m 1 '^PATCHLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')
LINUX_SUBLEVEL := $(shell grep -m 1 '^SUBLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')

LINUX_VER_GE_3 :=  $(shell test $(LINUX_VERSION) -ge 3 && echo y)
ifeq ($(LINUX_VER_GE_3),y)
	LINUX_VER_GE_2_6 = y
	LINUX_VER_GE_2_6_31 = y
	LINUX_VER_GE_2_6_37 = y
else
LINUX_VER_GE_2_6 ?= $(shell test $(LINUX_PATCHLEVEL) -eq 6 && echo 'y')
ifeq ($(LINUX_VER_GE_2_6),y)
LINUX_VER_GE_2_6_31 :=  $(shell test $(LINUX_SUBLEVEL) -ge 31 && echo y)
LINUX_VER_GE_2_6_37 :=  $(shell test $(LINUX_SUBLEVEL) -ge 37 && echo y)
LINUX_VER_EQ_2_6_12 := $(shell test $(LINUX_SUBLEVEL) -eq 12 && echo 'y')
endif
endif

LINUX_INC ?= $(LINUX_OUT)/include

ifeq ($(LINUX_VER_GE_2_6_31),y)
ifeq ($(LINUX_VER_GE_2_6_37),y)
LINUX_CONFIGURED := $(shell test -f $(LINUX_INC)/generated/autoconf.h && echo "y")
else
LINUX_CONFIGURED := $(shell test -f $(LINUX_INC)/linux/autoconf.h && echo "y")
endif
ifneq ($(LINUX_CONFIGURED),y)
ifneq ($(MAKECMDGOALS),clean) # allow clean even if linux is not configured
$(error $(LINUX) must be configured before using it to build a driver.)
endif
endif
endif

ifeq ($(LINUX_VER_GE_2_6),y)
 NEXUS_CFLAGS += -fno-common
 ifeq ($(LINUX_VER_EQ_2_6_12),y)
  ifeq ($(shell grep -e '^EXTRAVERSION =-[1-3]' $(LINUX)/Makefile > /dev/null && echo 'y'),)
  # Version 2.6.12-4.0 or later
  NEXUS_CFLAGS += -DHAVE_RSRVD=1
  endif
 else
  # Non 2.6.12 version, assume 2.6.18 or later
  NEXUS_CFLAGS += -DHAVE_RSRVD=1
 endif
endif

ifeq ($(B_REFSW_OPTIMIZER_LEVEL),)
B_REFSW_OPTIMIZER_LEVEL = 2
endif

# Seperate MIPS and ARM here
ifneq ($(filter ${B_REFSW_ARCH}, aarch64-linux arm-linux), ${B_REFSW_ARCH})
NEXUS_CFLAGS += -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -Wa,--trap -DMODULE -mlong-calls
NEXUS_CFLAGS += -msoft-float -pipe
NEXUS_CFLAGS += -DLINUX -D__KERNEL__
ifeq ($(LINUX_VER_GE_2_6_31),y)
LINUX_ASM_INC ?= $(LINUX)/arch/mips/include/asm/mach-brcmstb
NEXUS_CFLAGS += \
    -I$(LINUX_INC) \
    -I$(LINUX)/arch/mips/include \
    -I$(LINUX_ASM_INC) \
    -I$(LINUX)/arch/mips/include/asm/mach-generic
else
ifeq ($(filter ${B_REFSW_ARCH}, mips-linux mips-uclibc mips-linux-uclibc), ${B_REFSW_ARCH})
B_REFSW_TOOLCHAIN_ARCH=mips-linux-uclibc
else
B_REFSW_TOOLCHAIN_ARCH=mipsel-linux-uclibc
endif
STD_INC ?= $(B_REFSW_TOOLCHAIN_DIR)/$(B_REFSW_TOOLCHAIN_ARCH)/include
GCC_BASE ?= $(shell (test -d $(B_REFSW_TOOLCHAIN_DIR)/lib/gcc-lib && echo $(B_REFSW_TOOLCHAIN_DIR)/lib/gcc-lib/$(B_REFSW_TOOLCHAIN_ARCH)) || echo $(B_REFSW_TOOLCHAIN_DIR)/lib/gcc/$(B_REFSW_TOOLCHAIN_ARCH))
GCC_VER ?= $(shell (ls $(GCC_BASE)))
GCC_INC ?= $(GCC_BASE)/$(GCC_VER)/include
NEXUS_CFLAGS += -nostdinc -I$(LINUX_INC) -I$(STD_INC) -I$(GCC_INC)
LINUX_ASM_INC ?= $(LINUX)/asm-mips/mach-brcmstb
NEXUS_CFLAGS += \
    -I$(LINUX_ASM_INC) \
    -I$(LINUX_INC)/asm-mips/mach-mips \
    -I$(LINUX_INC)/asm/mach-generic/
endif


NEXUS_CFLAGS += -DBSTD_CPU_ENDIAN=$(NEXUS_ENDIAN)

ifeq ($(LINUX_VER_GE_2_6),y)
ifeq ($(NEXUS_PLATFORM),97325)
NEXUS_CFLAGS += -ffreestanding -finline-limit=100000  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -fno-strict-aliasing
else
NEXUS_CFLAGS += -ffreestanding -finline-limit=100000  -mabi=32 -march=mips32 -Wa,-32 -Wa,-march=mips32 -Wa,-mips32
endif
else
NEXUS_CFLAGS += -mips2
endif

# Kbuild uses this flag. We suppress this warning, which may occur inside Linux kernel code/headers.
NEXUS_CFLAGS += -Wno-pointer-sign

NEXUS_LDFLAGS += -G 0 --warn-common

else
# ARM

ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
ARM_ARCH=arm64
else
ARM_ARCH=arm
endif

# CFLAGS only for OS abstraction files
NEXUS_KERNEL_CFLAGS += \
        -DLINUX \
        -D__KERNEL__ \
        -I$(LINUX_OUT)/include\
        -I$(LINUX)/include\
        -I$(LINUX_OUT)/arch/$(ARM_ARCH)/include\
        -I$(LINUX)/arch/$(ARM_ARCH)/include\
        -include linux/kconfig.h\
        -I$(LINUX_OUT)/arch/$(ARM_ARCH)/include/generated\
        -I$(LINUX_OUT)/arch/$(ARM_ARCH)/include/uapi\
        -I$(LINUX)/arch/$(ARM_ARCH)/include/uapi\
        -I$(LINUX_OUT)/arch/$(ARM_ARCH)/include/generated/uapi\
        -I$(LINUX)/arch/$(ARM_ARCH)/include/generated/uapi\
        -I$(LINUX_OUT)/include/uapi\
        -I$(LINUX)/include/uapi\
        -I$(LINUX_OUT)/include/generated/uapi

# CFLAGS for all files compiled into linuxkernel
ifeq ($(B_REFSW_DEBUG),y)
NEXUS_CFLAGS  += $(filter-out -Wundef -mgeneral-regs-only,$(BCM_KBUILD_CFLAGS))
else
NEXUS_CFLAGS  += $(filter-out -Wundef -Wall -mgeneral-regs-only,$(BCM_KBUILD_CFLAGS))
endif
NEXUS_LDFLAGS += $(BCM_KBUILD_LDFLAGS)


ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
#TODO: -mfpu not avialble with aarch64 use -mcpu option disable NEON instructions
#NEXUS_CFLAGS += -mfpu=fp-armv8
else
# We are using -mfpu=vfpv3-d16 to force off generation of NEON instructions for fixed point code, when compiler comes with default -mfpu=neon-vfpv4
NEXUS_CFLAGS += -mfpu=vfpv3-d16
endif
endif

NEXUS_CFLAGS += -DBSTD_CPU_ENDIAN=$(NEXUS_ENDIAN)

ifeq ($(B_REFSW_ANDROID),y)
NEXUS_CFLAGS += -DB_REFSW_ANDROID
NEXUS_KERNEL_CFLAGS += \
        -I$(LINUX)/drivers/staging/android
endif

# Flag to generate dependencies
CDEP_FLAG = -MMD

# NEXUS_EXTRA_CFLAGS can be defined by the user to bring in extra CFLAGS
NEXUS_CFLAGS += ${NEXUS_EXTRA_CFLAGS}
