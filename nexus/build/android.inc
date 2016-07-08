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
	-Wl,--no-undefined \
	-Wl,-rpath-link=$(B_REFSW_ANDROID_LIB) \
	-L$(B_REFSW_ANDROID_LIB) \
	-nostdlib \
	$(B_REFSW_ANDROID_LIB)/crtbegin_dynamic.o \
	$(B_REFSW_ANDROID_LIB)/crtend_android.o \
	-lc -lm -lcutils -lc++ -ldl

ifeq ($(B_REFSW_USES_CLANG),y)
ifeq ($(B_REFSW_ARCH), aarch64-linux)
ANDROID_BASE_LDFLAGS += \
	${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
	-target aarch64-linux-android \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin
else
ANDROID_BASE_LDFLAGS += \
	${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
	-target arm-linux-androideabi \
	-B${ANDROID_TOP}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin
endif
endif

# Define separate server/client LDFLAGS for Android but maintain backward
# compatibility for apps that link with ANDROID_LDFLAGS
ANDROID_SERVER_LDFLAGS := -lnexus -L${NEXUS_BIN_DIR} ${ANDROID_BASE_LDFLAGS}
ANDROID_CLIENT_LDFLAGS := -lnexus${NEXUS_LIB_SUFFIX} -L${NEXUS_BIN_DIR} ${ANDROID_BASE_LDFLAGS}
ANDROID_LDFLAGS        := ${ANDROID_CLIENT_LDFLAGS}
