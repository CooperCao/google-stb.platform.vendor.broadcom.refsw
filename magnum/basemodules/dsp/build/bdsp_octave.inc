#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#############################################################################


#
# Conditional Options for this Makefile
#
# BDSP_AUDIO_SUPPORT (default=y) - Include audio support
# BDSP_VIDEO_SUPPORT (default=n) - Include video support
# BDSP_IMG_ONLY (default=n) - Include firmware only
# BDSP_IMG_EXTERNAL (default=n) - Include non-firmware files only
#

# Include standard files
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/include/octave
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/include/common_v2

# Include standard files
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)

# Include standard files
ifneq ($(BDSP_IMG_ONLY),y)
BDSP_SOURCES += \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_cit_priv.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_command_priv.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_fwdownload_priv.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_fwinterface_priv.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_hw_priv.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_int_priv.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_mm_priv.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_priv.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_fw_settings.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_fw_algo.c  \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_algo_priv.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_io_priv.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_cit_log.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_cit_log_ext.c \
        $(MAGNUM)/basemodules/dsp/src/octave/bdsp_raaga_cit_log_buf.c
endif


########################################################################
# Added lines for libdspcontrol integration
########################################################################
BDSP_RAAGA_FPSDK_MACH := $(shell echo $(BDSP_RAAGA_FP_VER) | tr '[:lower:]' '[:upper:]' | egrep -o 'FP[0-9]+')
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/

ifneq ($(BDSP_IMG_ONLY),y)
BDSP_SOURCES += \
    $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/DSP/DSP.c              \
    $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/DSP/DSP_octave_maestro.c   \
    $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/DSP/DSP_raaga_octave.c   \
    $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/DSP/DSP_raaga_octave_atu.c \
    $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/ENDIANESS.c        \
    $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/TB_common.c        \
    $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/TB_circular.c      \
    $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/UTIL_addr.c

    ifeq (${SYSTEM},linuxemu)
        ifeq (${FIREPATH_BM},y)
            BDSP_DEFINES += FIREPATH_BM
            APP_CFLAGS += --std=gnu99 -Wno-pointer-arith
	    BDSP_SOURCES += $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/DSP/DSP_raaga_octave_bm.c
        endif

        BDSP_DEFINES += BFPSDK_LIBDSPCONTROL_TARGET_SOC=Raaga$(BDSP_RAAGA_FPSDK_MACH)_bm _DSP_CONTROL_API_
        BDSP_SOURCES += \
            $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/DSPLOG.c    \
            $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/OS_file.c   \
            $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/OS_linux.c

        APP_INCLUDES += $(shell /projects/firepath/tools/bin/fp-pkg-config --pkg fp_binutils --print-include)
        APP_LDFLAGS += $(shell /projects/firepath/tools/bin/fp-pkg-config --pkg fp_binutils --print-lib --32-bit)/system-control-proxy-c.o -lrt
    else
        BDSP_DEFINES += BFPSDK_LIBDSPCONTROL_TARGET_SOC=Raaga$(BDSP_RAAGA_FPSDK_MACH)_si_magnum
        BDSP_SOURCES += \
            $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/DSP/DSP_raaga_octave_si.c \
            $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/DSPLOG_stubs.c \
            $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/DSP/DSP_raaga_si.c     \
            $(MAGNUM)/basemodules/dsp/src/octave/fp_sdk/libdspcontrol/src/DSP/DSP_raaga_mem_si.c
    endif
endif
########################################################################
# Added lines for libdspcontrol integration - end
########################################################################

# Set CFLAG for Raaga Support
BDSP_DEFINES += BDSP_RAAGA_SUPPORT=1

# Include firmware files
include $(MAGNUM)/basemodules/dsp/build/bdsp_octave_img.inc
