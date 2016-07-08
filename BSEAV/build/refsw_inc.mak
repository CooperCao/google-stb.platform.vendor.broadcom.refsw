###########################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
###########################################################

###############################################
#
# Inputs:
#    BSEAV - this is required only for the optional MAGNUM defines
#    NEXUS_PLATFORM - this is required
#    SYSTEM - linux (default), linuxkernel, vxworks
#    B_REFSW_DEBUG - y (default) or n
# Outputs:
#    B_REFSW_CFLAGS - compiler flags
#    B_REFSW_LDFLAGS - linker flags
#    B_REFSW_MAGNUM_CFLAGS
#    DEBUG_SUFFIX - helpful for directory names
#
# All internal variables should be prefixed with B_REFSW_ to avoid namespace pollution.

# We base a lot of things on the particular platform we compiling for.
# This variable MUST be defined in order to continue.  Make sure it is!
ifeq ($(NEXUS_PLATFORM),)
NEXUS_PLATFORM = $(PLATFORM)
ifeq ($(NEXUS_PLATFORM),)
$(error NEXUS_PLATFORM environment variable is required)
endif
endif

###############################################
#
# Set defaults
#
ifeq ($(SYSTEM),)
SYSTEM = linux
endif

# Set B_REFSW_DEBUG flag for debug interface, default y.
# If you set B_REFSW_DEBUG=n, please realize that you won't even get error messages.
ifeq ($(B_REFSW_DEBUG),)
B_REFSW_DEBUG = y
endif

# Set a default optimization level.
ifeq ($(OPTIMIZER_LEVEL),)
OPTIMIZER_LEVEL = 2
endif

###############################################
#
# Magnum DBG defines
#

ifeq (${B_REFSW_DEBUG},y)
#
# debug
#
B_REFSW_CFLAGS += -DBDBG_DEBUG_BUILD=1 -DBRCM_DBG
DEBUG_SUFFIX = debug
# Add future platforms here (leave 97038 alone -- its in maintainence mode).
ifeq ($(findstring $(NEXUS_PLATFORM),97398 97401 97403 97400 97405 97335 97342 97340 97118 97118RNG 97455 97458 97456 97459 97420 97410 97409 93380vms 97125 97025 97119 97019 97116 97468 97208 93549 93556 97550 97408 97422 97425 97435 97358 97552 97231 97230 97429 97428 97241 97584), $(NEXUS_PLATFORM))
 B_REFSW_CFLAGS += -O$(OPTIMIZER_LEVEL)
 ifneq ($(findstring $(B_REFSW_ARCH), i386-mingw i386-linux mips-vxworks), $(B_REFSW_ARCH))
 B_REFSW_CFLAGS += -mips32
 endif
endif
ifeq ($(findstring $(NEXUS_PLATFORM),97325), $(NEXUS_PLATFORM))
 B_REFSW_CFLAGS += -O$(OPTIMIZER_LEVEL)
 ifneq ($(findstring $(B_REFSW_ARCH), i386-mingw i386-linux mips-vxworks), $(B_REFSW_ARCH))
 B_REFSW_CFLAGS += -mips32r2
 endif
endif
ifneq ($(SYSTEM),linuxkernel)
B_REFSW_CFLAGS += -g
B_REFSW_LDFLAGS += -g
endif

else
#
# release
#
DEBUG_SUFFIX = release
B_REFSW_CFLAGS += -DNDEBUG
# Add future platforms here (leave 97038 alone -- its in maintainence mode).
ifeq ($(findstring $(NEXUS_PLATFORM),97398 97401 97403 97400 97405 97335 97342 97340 97118 97118RNG 97455 97458 97456 97459 97420 97410 97409 93380vms 97468 97208 93549 93556 97550 97408 97422 97425 97435 97358 97552 97231 97230 97584), $(NEXUS_PLATFORM))
 B_REFSW_CFLAGS += -O$(OPTIMIZER_LEVEL)
 ifneq ($(findstring $(B_REFSW_ARCH),i386-linux mips-vxworks), $(B_REFSW_ARCH))
 B_REFSW_CFLAGS += -mips32
 endif
 ifeq ($(findstring $(NEXUS_PLATFORM),97325), $(NEXUS_PLATFORM))
 B_REFSW_CFLAGS += -O$(OPTIMIZER_LEVEL)
 ifneq ($(findstring $(B_REFSW_ARCH), i386-mingw i386-linux mips-vxworks), $(B_REFSW_ARCH))
 B_REFSW_CFLAGS += -mips32r2
 endif
 endif
else
ifneq ($(SYSTEM),linuxkernel)
# -O (not -O2) is required for correct float math
B_REFSW_CFLAGS += -O -Wuninitialized
endif
endif

endif

ifeq ($(SYSTEM),linuxkernel)
B_REFSW_LDFLAGS += -r -d
LINUX ?= /opt/brcm/linux
LINUX_VER_GE_2_6 ?= $(shell (grep 'PATCHLEVEL = 6' ${LINUX}/Makefile >/dev/null && echo 'y'))
ifeq ($(LINUX_VER_GE_2_6),y)
 B_REFSW_CFLAGS += -fno-common
 LINUX_PATCHLEVEL = $(shell grep 'PATCHLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')
 LINUX_SUBLEVEL = $(shell grep 'SUBLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')
 LINUX_VER_GE_2_6_31 = $(shell test $(LINUX_PATCHLEVEL) -eq 6 -a $(LINUX_SUBLEVEL) -ge 31 && echo y)
 LINUX_VER_EQ_2_6_12 = $(shell (grep 'SUBLEVEL = 12' ${LINUX}/Makefile >/dev/null && echo 'y'))
 LINUX_VER_EQ_2_6_18 = $(shell (grep 'SUBLEVEL = 18' ${LINUX}/Makefile >/dev/null && echo 'y'))
 LINUX_VER_EQ_2_6_31 = $(shell (grep 'SUBLEVEL = 31' ${LINUX}/Makefile >/dev/null && echo 'y'))
 LINUX_VER_EQ_2_6_37 = $(shell (grep 'SUBLEVEL = 37' ${LINUX}/Makefile >/dev/null && echo 'y'))
 ifeq ($(LINUX_VER_EQ_2_6_12),y)
  ifeq ($(shell grep -e 'EXTRAVERSION =-[1-3]' $(LINUX)/Makefile > /dev/null && echo 'y'),)
  # Version 2.6.12-4.0 or later
  B_REFSW_CFLAGS += -DHAVE_RSRVD=1
  endif
 else
  # Non 2.6.12 version, assume 2.6.18 or later
  B_REFSW_CFLAGS += -DHAVE_RSRVD=1
 endif
endif
endif

###############################################
#
# SYSTEM-specific defines.
# The following values are supported: linux, linuxkernel, vxworks
# Keep all rules explicit and positive. Don't add "ifneq linux".

B_REFSW_CFLAGS += -DB_SYSTEM_${SYSTEM}=1

#
# linux
#
ifeq ($(SYSTEM),linux)
ifeq ($(B_REFSW_ARCH),mips-linux)
B_REFSW_CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_BIG
else
B_REFSW_CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE
endif
B_REFSW_CFLAGS += -D_GNU_SOURCE=1
B_REFSW_CFLAGS_SHARED = -fpic -DPIC
B_REFSW_CFLAGS += -DLINUX
B_REFSW_CFLAGS += -pipe
B_REFSW_CFLAGS += -W
# Get the uclibc version for a runtime check in bsettop_init()
ifneq ($(B_REFSW_ARCH),i386-linux)
UCLIBC_VERSION = $(shell basename $(TOOLCHAIN_DIR)/mipsel-linux/lib/libuClibc*)
B_REFSW_CFLAGS += -DUCLIBC_VERSION=\"$(UCLIBC_VERSION)\"
endif
B_REFSW_CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
ifneq ($(BUILD_SYSTEM),legacy)
# NOTE: This will be upgraded to -pedantic-errors soon.
B_REFSW_CFLAGS += -pedantic
endif
B_REFSW_C_ONLY_FLAGS = -std=c99
endif

ifeq ($(SYSTEM),win32)
B_REFSW_CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE
ifeq ($(B_REFSW_ARCH),i386-mingw)
B_REFSW_CFLAGS += -D_GNU_SOURCE=1
B_REFSW_CFLAGS_SHARED = -fpic -DPIC
B_REFSW_CFLAGS += -pipe
B_REFSW_CFLAGS += -W
B_REFSW_CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
B_REFSW_CFLAGS += -pedantic
endif
endif

#
# linux kernel
#
ifeq ($(SYSTEM),linuxkernel)
ifeq ($(B_REFSW_ARCH),mips-linux)
B_REFSW_CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_BIG
else
B_REFSW_CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE
endif
ifeq ($(LINUX_VER_GE_2_6),y)
ifeq ($(NEXUS_PLATFORM),97325)
B_REFSW_CFLAGS += -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -Wa,--trap -DMODULE -mlong-calls \
	          -fno-common -ffreestanding -finline-limit=100000  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 \
	          -Wa,--trap -DMODULE -mlong-calls
else
B_REFSW_CFLAGS += -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -Wa,--trap -DMODULE -mlong-calls \
	          -fno-common -ffreestanding -finline-limit=100000  -mabi=32 -march=mips32 -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 \
	          -Wa,--trap -DMODULE -mlong-calls
endif
else
B_REFSW_CFLAGS += -mips2 -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -Wa,--trap -DMODULE -mlong-calls
endif
# -O2 is required for kernel inline functions
B_REFSW_CFLAGS += -O2
B_REFSW_CFLAGS += -msoft-float
B_REFSW_CFLAGS += -pipe

ifeq ($(LINUX_VER_GE_2_6_31),y)
B_REFSW_LINUXKERNEL_CFLAGS += -I${LINUX_INC} -DLINUX -D__KERNEL__ 
B_REFSW_LINUXKERNEL_CFLAGS += -I${LINUX_INC}/asm-mips \
	-I${LINUX}/arch/mips/include \
	-I${LINUX}/arch/mips/include/asm/mach-brcmstb \
	-I${LINUX}/arch/mips/include/asm/mach-generic
else
B_REFSW_LINUXKERNEL_CFLAGS += -nostdinc -I${LINUX_INC} -I${STD_INC} -I${GCC_INC} -DLINUX -D__KERNEL__
B_REFSW_LINUXKERNEL_CFLAGS += -I${LINUX_INC}/asm-mips/mach-brcmstb -I${LINUX_INC}/asm-mips/mach-mips
B_REFSW_LINUXKERNEL_CFLAGS += -I${LINUX_INC}/asm/mach-generic/
endif

endif

#
# vxworks
#
ifeq ($(SYSTEM),vxworks)
ifeq ($(WIND_BASE),)
$(error WIND_BASE is required. It is normally defined by torVars.bat or wrenv.exe)
endif
B_REFSW_CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_BIG
B_REFSW_CFLAGS += -DVXWORKS
B_REFSW_CFLAGS += -W
B_REFSW_LDFLAGS += -r -nostdlib

# Platform specific definitions (legacy)
ifeq ($(findstring $(NEXUS_PLATFORM),97111 97110 97312 97318 97318AVC), $(NEXUS_PLATFORM))
B_REFSW_CFLAGS += -DTOOL_FAMILY=gnu -DCPU=MIPS32 -DTOOL=sfgnu -mips2 -msoft-float -DSOFT_FLOAT
else
# Platform specific definitions (current)
ifeq ($(findstring $(NEXUS_PLATFORM),97401 97403 97400 97405 97335 97340 97342 97325 97435 97118 97118RNG 97468 97408), $(NEXUS_PLATFORM))
B_REFSW_CFLAGS += -DTOOL_FAMILY=gnu -DCPU=MIPS32 -DTOOL=sfgnu -mips2 -msoft-float -DSOFT_FLOAT
B_REFSW_LDFLAGS += -L$(WIND_BASE)/target/lib/mips/MIPS32/sfcommon
else
B_REFSW_CFLAGS += -DTOOL_FAMILY=gnu -DCPU=MIPS64 -DTOOL=gnu -mips3
B_REFSW_LDFLAGS += -L$(WIND_BASE)/target/lib/mips/MIPS64/common
endif
B_REFSW_CFLAGS += -G 0 -mno-branch-likely -fno-builtin -EB -DMIPSEB
endif

# Tool specific definitions
ifeq ($(vxWorksVersion),6)
ifneq ($(findstring $(NEXUS_PLATFORM),97401 97403 97400 97405 97325 97335 97340 97342 97435 97118 97118RNG), $(NEXUS_PLATFORM))
B_REFSW_CFLAGS += -mabi=o64 -mgp64
endif
B_REFSW_CFLAGS += -DVXWORKS6
B_REFSW_CFLAGS += -fdollars-in-identifiers
B_REFSW_CFLAGS += -I$(WIND_BASE)/target/h/wrn/coreip/ -I$(WIND_BASE)/target/h
endif
endif

###############################################
#
# Generic magnum defines. Useful if code is not chip-specific.
#

ifeq ($(B_REFSW_OS),)
ifeq (${SYSTEM},linux)
B_REFSW_OS = linuxuser
else
B_REFSW_OS = ${SYSTEM}
endif
endif

ifneq ($(BSEAV),)
# bchp.inc requires BCHP_CHIP, and that is derived from NEXUS_PLATFORM in nexus_platforms.inc
include $(BSEAV)/../nexus/platforms/common/build/nexus_platforms.inc
MAGNUM := $(BSEAV)/../magnum
include ${MAGNUM}/basemodules/dbg/bdbg.inc
include ${MAGNUM}/basemodules/err/berr.inc
include ${MAGNUM}/basemodules/std/bstd.inc
include ${MAGNUM}/basemodules/chp/bchp.inc
include ${MAGNUM}/basemodules/reg/breg.inc
include ${MAGNUM}/basemodules/kni/bkni.inc
include ${MAGNUM}/basemodules/tmr/btmr.inc
include ${MAGNUM}/commonutils/lst/blst.inc

B_REFSW_MAGNUM_INCLUDE_DIRS = -I${BSEAV}/..
B_REFSW_MAGNUM_INCLUDE_DIRS += $(addprefix -I,$(BDBG_INCLUDES) $(BERR_INCLUDES) $(BSTD_INCLUDES) $(BCHP_INCLUDES) $(BREG_INCLUDES) $(BKNI_INCLUDES) $(BTMR_INCLUDES) $(BLST_INCLUDES))
# clear MAGNUM_MODULES to avoid a Nexus build error
MAGNUM_MODULES=
endif

B_REFSW_GENERIC_MAGNUM_CFLAGS += -DBCHP_CHIP=GENERIC -DBCHP_VER=BCHP_VER_A0
B_REFSW_CFLAGS += $(addprefix -D,$(MAGNUM_SYSTEM_DEFINES))
B_REFSW_CFLAGS += $(addprefix -D,$(BDBG_DEFINES))
