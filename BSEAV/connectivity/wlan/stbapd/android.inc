##############################################################################
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
##############################################################################

# Includes for toolchains
include $(BROADCOM_STBAPD_SOURCE_PATH)/os_tools.inc

# Debugging info
$(warning "android.inc: B_REFSW_ARCH=$(B_REFSW_ARCH)")
$(warning "android.inc: B_REFSW_USES_CLANG=$(B_REFSW_USES_CLANG)")

# cflags - generic.
ANDROID_CFLAGS := -DBDBG_DEBUG_BUILD=1 \
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
ANDROID_CFLAGS += -Os \
	-finline-functions \
	-fomit-frame-pointer \
	-fstrict-aliasing

# cflags arch specific - only arm/aarch64 supported.
ANDROID_CFLAGS += -fPIE

ifeq ($(B_REFSW_ARCH), aarch64-linux)
ANDROID_CFLAGS += -mcpu=cortex-a53
else
ANDROID_CFLAGS += -msoft-float \
	-mfloat-abi=softfp \
	-mcpu=cortex-a15 \
	-mfpu=neon
endif

ifeq ($(B_REFSW_USES_CLANG),y)
ANDROID_CFLAGS += -nostdlibinc \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin
ifeq ($(B_REFSW_ARCH), aarch64-linux)
	ANDROID_CFLAGS += -target aarch64-linux-android
else
	ANDROID_CFLAGS += -target arm-linux-androideabi
endif
endif

# backward compatibility for K.
ifeq ($(B_REFSW_ANDROID_DEFINE_MMAP64),y)
ANDROID_CFLAGS += -Dmmap64=mmap
endif

# includes - generic.
ANDROID_CFLAGS += -I$(ANDROID)/system/core/include \
	-I$(ANDROID)/hardware/libhardware/include \
	-I$(ANDROID)/hardware/ril/include \
	-I$(ANDROID)/dalvik/libnativehelper/include \
	-I$(ANDROID)/frameworks/base/include \
	-I$(ANDROID)/external/skia/include \
	-I$(ANDROID)/out/target/product/${LOCAL_PRODUCT_OUT}/obj/include \
	-I$(ANDROID)/bionic/libc/include \
	-I$(ANDROID)/bionic/libstdc++/include \
	-I$(ANDROID)/bionic/libm/include \
	-I$(ANDROID)/bionic/libthread_db/include \
	-I$(ANDROID)/bionic/libm \
	-I$(ANDROID)/out/target/product/${LOCAL_PRODUCT_OUT}/obj/SHARED_LIBRARIES/libm_intermediates \
	-I$(ANDROID)/frameworks/native/include \
	-I$(ANDROID)/bionic/libc/kernel/uapi \
	-I$(ANDROID)/bionic/libc/kernel/android/uapi

# includes - arch specific - only arm/aarch64 supported.
ifeq ($(B_REFSW_ARCH), aarch64-linux)
ANDROID_CFLAGS += -I$(ANDROID)/bionic/libc/arch-arm64/include \
	-I$(ANDROID)/bionic/libm/arm64 \
	-I$(ANDROID)/bionic/libc/kernel/uapi/asm-arm64
else
ANDROID_CFLAGS += -I$(ANDROID)/bionic/libc/arch-arm/include \
	-I$(ANDROID)/bionic/libm/arm \
	-I$(ANDROID)/bionic/libc/kernel/uapi/asm-arm
endif

ifeq ($(B_REFSW_ARCH), aarch64-linux)
ANDROID_LINKER := /system/bin/linker64
else
ANDROID_LINKER := /system/bin/linker
endif

ANDROID_BASE_LDFLAGS := \
	-Bdynamic \
	-Wl,-dynamic-linker,$(ANDROID_LINKER) \
	-Wl,--gc-sections \
	-Wl,-z,nocopyreloc \
	-Wl,-rpath-link=$(B_REFSW_ANDROID_LIB) \
	-Wl,-s \
	-fPIE -pie \
	-L$(B_REFSW_ANDROID_LIB) \
	-nostdlib \
	$(B_REFSW_ANDROID_LIB)/crtbegin_dynamic.o \
	$(B_REFSW_ANDROID_LIB)/crtend_android.o \
	-lc -lm -lcutils -llog -lc++ -ldl

ANDROID_SHARE_LDFLAGS := \
	-Bdynamic \
	-Wl,-dynamic-linker,$(ANDROID_LINKER) \
	-Wl,--gc-sections \
	-Wl,-z,nocopyreloc \
	-Wl,-rpath-link=$(B_REFSW_ANDROID_LIB) \
	-Wl,-s \
	-fPIE -pie \
	-L$(B_REFSW_ANDROID_LIB) \
	-nostdlib \
	$(B_REFSW_ANDROID_LIB)/crtbegin_so.o \
	$(B_REFSW_ANDROID_LIB)/crtend_android.o \
	-lc -lm -lcutils -llog -lc++ -ldl

ifeq ($(B_REFSW_USES_CLANG),y)
ifeq ($(B_REFSW_ARCH), aarch64-linux)
ANDROID_BASE_LDFLAGS += \
	${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
	-target aarch64-linux-android \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin
ANDROID_SHARE_LDFLAGS += \
	${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
	-target aarch64-linux-android \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin
else
ANDROID_BASE_LDFLAGS += \
	${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
	-target arm-linux-androideabi \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin
ANDROID_SHARE_LDFLAGS += \
	${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
	-target arm-linux-androideabi \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin
endif
endif

##############################################################################
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
##############################################################################

# Includes for toolchains
include $(BROADCOM_STBAPD_SOURCE_PATH)/os_tools.inc

# Debugging info
$(warning "android.inc: B_REFSW_ARCH=$(B_REFSW_ARCH)")
$(warning "android.inc: B_REFSW_USES_CLANG=$(B_REFSW_USES_CLANG)")

# cflags - generic.
ANDROID_CFLAGS := -DBDBG_DEBUG_BUILD=1 \
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
ANDROID_CFLAGS += -Os \
	-finline-functions \
	-fomit-frame-pointer \
	-fstrict-aliasing

# cflags arch specific - only arm/aarch64 supported.
ANDROID_CFLAGS += -fPIE

ifeq ($(B_REFSW_ARCH), aarch64-linux)
ANDROID_CFLAGS += -mcpu=cortex-a53
else
ANDROID_CFLAGS += -msoft-float \
	-mfloat-abi=softfp \
	-mcpu=cortex-a15 \
	-mfpu=neon
endif

ifeq ($(B_REFSW_USES_CLANG),y)
ANDROID_CFLAGS += -nostdlibinc \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin
ifeq ($(B_REFSW_ARCH), aarch64-linux)
ANDROID_CFLAGS += -target aarch64-linux-android
else
ANDROID_CFLAGS += -target arm-linux-androideabi
endif
endif

# backward compatibility for K.
ifeq ($(B_REFSW_ANDROID_DEFINE_MMAP64),y)
ANDROID_CFLAGS += -Dmmap64=mmap
endif

# includes - generic.
ANDROID_CFLAGS += -I$(ANDROID)/system/core/include \
	-I$(ANDROID)/hardware/libhardware/include \
	-I$(ANDROID)/hardware/ril/include \
	-I$(ANDROID)/dalvik/libnativehelper/include \
	-I$(ANDROID)/frameworks/base/include \
	-I$(ANDROID)/external/skia/include \
	-I$(ANDROID)/out/target/product/${LOCAL_PRODUCT_OUT}/obj/include \
	-I$(ANDROID)/bionic/libc/include \
	-I$(ANDROID)/bionic/libstdc++/include \
	-I$(ANDROID)/bionic/libm/include \
	-I$(ANDROID)/bionic/libthread_db/include \
	-I$(ANDROID)/bionic/libm \
	-I$(ANDROID)/out/target/product/${LOCAL_PRODUCT_OUT}/obj/SHARED_LIBRARIES/libm_intermediates \
	-I$(ANDROID)/frameworks/native/include \
	-I$(ANDROID)/bionic/libc/kernel/uapi \
	-I$(ANDROID)/bionic/libc/kernel/android/uapi

# includes - arch specific - only arm/aarch64 supported.
ifeq ($(B_REFSW_ARCH), aarch64-linux)
ANDROID_CFLAGS += -I$(ANDROID)/bionic/libc/arch-arm64/include \
	-I$(ANDROID)/bionic/libm/arm64 \
	-I$(ANDROID)/bionic/libc/kernel/uapi/asm-arm64
else
ANDROID_CFLAGS += -I$(ANDROID)/bionic/libc/arch-arm/include \
	-I$(ANDROID)/bionic/libm/arm \
	-I$(ANDROID)/bionic/libc/kernel/uapi/asm-arm
endif

ifeq ($(B_REFSW_ARCH), aarch64-linux)
ANDROID_LINKER := /system/bin/linker64
else
ANDROID_LINKER := /system/bin/linker
endif

ANDROID_BASE_LDFLAGS := \
	-Bdynamic \
	-Wl,-dynamic-linker,$(ANDROID_LINKER) \
	-Wl,--gc-sections \
	-Wl,-z,nocopyreloc \
	-Wl,-rpath-link=$(B_REFSW_ANDROID_LIB) \
	-Wl,-s \
	-fPIE -pie \
	-L$(B_REFSW_ANDROID_LIB) \
	-nostdlib \
	$(B_REFSW_ANDROID_LIB)/crtbegin_dynamic.o \
	$(B_REFSW_ANDROID_LIB)/crtend_android.o \
	-lc -lm -lcutils -llog -lc++ -ldl

ANDROID_SHARE_LDFLAGS := \
	-Bdynamic \
	-Wl,-dynamic-linker,$(ANDROID_LINKER) \
	-Wl,--gc-sections \
	-Wl,-z,nocopyreloc \
	-Wl,-rpath-link=$(B_REFSW_ANDROID_LIB) \
	-Wl,-s \
	-fPIE -pie \
	-L$(B_REFSW_ANDROID_LIB) \
	-nostdlib \
	$(B_REFSW_ANDROID_LIB)/crtbegin_so.o \
	$(B_REFSW_ANDROID_LIB)/crtend_android.o \
	-lc -lm -lcutils -llog -lc++ -ldl

ifeq ($(B_REFSW_USES_CLANG),y)
ifeq ($(B_REFSW_ARCH), aarch64-linux)
ANDROID_BASE_LDFLAGS += \
	${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
	-target aarch64-linux-android \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin
ANDROID_SHARE_LDFLAGS += \
	${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
	-target aarch64-linux-android \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin
else
ANDROID_BASE_LDFLAGS += \
	${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
	-target arm-linux-androideabi \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin
ANDROID_SHARE_LDFLAGS += \
	${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
	-target arm-linux-androideabi \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin
endif
endif

