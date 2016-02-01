############################################################
#     (c)2010-2013 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#   This file defines the Android flags for building in linuxuser mode.
#
# Revision History:
#
# $brcm_Log: $
#
############################################################

############################################################
# Overview:
#
# This file should only be included by os_flags.inc
#
############################################################
ifeq ($(ANDROID_OUT_DIR),)
ANDROID_OUT_DIR = $(ANDROID)/out
endif
ANDROID_LIB = $(ANDROID_OUT_DIR)/target/product/${ANDROID_PRODUCT_OUT}/obj/lib

# cflags - generic.
NEXUS_CFLAGS := -DBDBG_DEBUG_BUILD=1 \
	-D_GNU_SOURCE=1 \
	-DLINUX \
	-pipe \
	-D_FILE_OFFSET_BITS=64 \
	-D_LARGEFILE_SOURCE \
	-D_LARGEFILE64_SOURCE \
	-DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE \
	-Wstrict-prototypes \
	-Wno-unused-value \
	-DANDROID \
	-DSK_RELEASE \
	-DNDEBUG \
	-DHAVE_SYS_UIO_H \
	-UDEBUG \
	-DB_REFSW_ANDROID

# cflags - optimization.
NEXUS_CFLAGS += -Os \
	-finline-functions \
	-finline-limit=300 \
	-fno-inline-functions-called-once \
	-fgcse-after-reload \
	-frename-registers \
	-fomit-frame-pointer \
	-fstrict-aliasing \
	-funswitch-loops

# cflags arch specific - only arm supported.
NEXUS_CFLAGS += -msoft-float \
        -fPIE \
	-pie \
        -mfloat-abi=softfp \
	-mcpu=cortex-a15 \
	-mfpu=neon

ifeq ($(B_REFSW_USES_CLANG),y)
NEXUS_CFLAGS += -nostdlibinc \
	-target arm-linux-androideabi \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin
else
NEXUS_CFLAGS += -frerun-cse-after-loop
endif

# backward compatibility for K.
ifeq ($(B_REFSW_ANDROID_DEFINE_MMAP64),y)
NEXUS_CFLAGS += -Dmmap64=mmap
endif

# includes - generic.
NEXUS_CFLAGS += -I$(ANDROID)/system/core/include \
	-I$(ANDROID)/hardware/libhardware/include \
	-I$(ANDROID)/hardware/ril/include \
	-I$(ANDROID)/dalvik/libnativehelper/include \
	-I$(ANDROID)/frameworks/base/include \
	-I$(ANDROID)/external/skia/include \
	-I$(ANDROID)/out/target/product/${ANDROID_PRODUCT_OUT}/obj/include \
	-I$(ANDROID)/bionic/libc/include \
	-I$(ANDROID)/bionic/libstdc++/include \
	-I$(ANDROID)/bionic/libm/include \
	-I$(ANDROID)/bionic/libthread_db/include \
	-I$(ANDROID)/bionic/libm \
	-I$(ANDROID)/out/target/product/${ANDROID_PRODUCT_OUT}/obj/SHARED_LIBRARIES/libm_intermediates \
	-I$(ANDROID)/frameworks/native/include \
	-I$(ANDROID)/bionic/libc/kernel/uapi

# includes - arch specific - only arm supported.
NEXUS_CFLAGS += -I$(ANDROID)/bionic/libc/arch-arm/include \
	-I$(ANDROID)/bionic/libm/arm \
	-I$(ANDROID)/bionic/libc/kernel/uapi/asm-arm

# ldflags - generic.
NEXUS_LDFLAGS := -nostdlib  \
	-Wl,--gc-sections \
	-Wl,-shared,-Bsymbolic \
	-L$(ANDROID_LIB)

ifeq ($(B_REFSW_USES_CLANG),y)
NEXUS_LDFLAGS += -Wl,--no-whole-archive \
	-lc -lm -lcutils -lc++ \
	-Wl,--no-undefined \
	-Wl,--whole-archive \
	${ANDROID_TOP}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/lib/gcc/arm-linux-androideabi/4.9.x-google/libgcc.a \
	-target arm-linux-androideabi \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin
else
NEXUS_LDFLAGS += -Wl,--no-whole-archive \
	-lcutils -lc -lm -lgcc \
	-Wl,--no-undefined \
	-Wl,--whole-archive
endif

# ldflags - arch specific - only arm supported.
NEXUS_LDFLAGS += -Wl,--fix-cortex-a8
