###############################################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
###############################################################################

#
# NOTE: Do not add platform-specific logic here. There should be no variation of ifeq ($(NEXUS_PLATFORM),) in this file.
# Platform-specific logic can be added to nexus/platforms/$(NEXUS_PLATFORM)/build/platform_options.inc.
#

######################
# Define this module #
######################
NEXUS_MODULES += PLATFORM

##################
# Module Options #
##################

##########################
# Default Module Options #
##########################
ifeq ($(NEXUS_POWER_MANAGEMENT),)
NEXUS_POWER_MANAGEMENT := y
endif

ifndef NEXUS_CONFIG_IMAGE
ifeq ($(filter $(B_REFSW_OS),linuxkernel bare),$(B_REFSW_OS))
ifeq ($(NEXUS_MODE),driver)
NEXUS_CONFIG_IMAGE :=y
endif
endif
endif

ifeq ($(NEXUS_MODE),proxy)
ifndef NEXUS_CONFIG_IMAGE
NEXUS_CONFIG_IMAGE :=y
endif
endif

ifeq ($(NEXUS_PLATFORM_COMMON_DIR),)
NEXUS_PLATFORM_COMMON_DIR=$(NEXUS_TOP)/platforms/common
endif
ifeq ($(NEXUS_PLATFORM_SPECIFIC_DIR),)
NEXUS_PLATFORM_SPECIFIC_DIR=$(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)
endif

###################
# Module Includes #
###################
NEXUS_PLATFORM_PUBLIC_INCLUDES = \
    $(NEXUS_PLATFORM_COMMON_DIR)/include \
    $(NEXUS_PLATFORM_SPECIFIC_DIR)/include \

# must add nexus/platforms to resolve #include-style symlinks
NEXUS_PLATFORM_PUBLIC_INCLUDES += $(NEXUS_TOP)/platforms

NEXUS_PLATFORM_PRIVATE_INCLUDES += $(NEXUS_PLATFORM_COMMON_DIR)/src \
                                   $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS) \
                                   $(NEXUS_SYNCTHUNK_DIR) \
                                   $(MAGNUM)/../BSEAV/lib/utils

ifeq ($(filter $(NEXUS_MODE),proxy driver),$(NEXUS_MODE))
NEXUS_PLATFORM_PRIVATE_INCLUDES += $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxkernel/common
endif

##################
# Module Sources #
##################
NEXUS_PLATFORM_SOURCES = \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_map.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_$(NEXUS_PLATFORM).c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_config.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_core.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_interrupt.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_ipc.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_memconfig.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_settings.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_standby.c \

ifeq ($(filter $(B_REFSW_OS),ucos ucos_iii),$(B_REFSW_OS))
NEXUS_PLATFORM_SOURCES += \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/ucos/nexus_platform_server.c
else
NEXUS_PLATFORM_SOURCES += \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_platform_server.c
endif

NEXUS_PLATFORM_LOCAL_SOURCES += \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_local.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_version.c

ifeq ($(NEXUS_SYSTEM_CABLE),y)
NEXUS_PLATFORM_SOURCES += $(NEXUS_PLATFORM_SPECIFIC_DIR)/src/nexus_platform_pinmux_cable.c
else
NEXUS_PLATFORM_SOURCES += $(NEXUS_PLATFORM_SPECIFIC_DIR)/src/nexus_platform_pinmux.c
endif

#
# Allow platform-specific vcxo file to take precedence over common one
#
ifneq ($(wildcard $(NEXUS_PLATFORM_SPECIFIC_DIR)/src/nexus_platform_vcxo.c),)
NEXUS_PLATFORM_SOURCES += \
    $(NEXUS_PLATFORM_SPECIFIC_DIR)/src/nexus_platform_vcxo.c
else
NEXUS_PLATFORM_SOURCES += \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_vcxo.c
endif

NEXUS_PLATFORM_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_platform_thunks.c

ifeq ($(NEXUS_SYSTEM_CABLE),y)
NEXUS_PLATFORM_SOURCES += $(NEXUS_PLATFORM_SPECIFIC_DIR)/src/nexus_platform_frontend_cable.c
else
NEXUS_PLATFORM_SOURCES += $(NEXUS_PLATFORM_SPECIFIC_DIR)/src/nexus_platform_frontend.c
endif

ifeq ($(shell test -e $(NEXUS_PLATFORM_SPECIFIC_DIR)/src/nexus_platform_frontend_power_management.c && echo y),y)
NEXUS_PLATFORM_SOURCES += $(NEXUS_PLATFORM_SPECIFIC_DIR)/src/nexus_platform_frontend_power_management.c
endif

FLOATLIB=$(NEXUS_TOP)/../BSEAV/linux/driver/floatlib

LINUX_OUT ?= $(LINUX)
NEXUS_GISB_ARB := $(shell if grep 'CONFIG_BRCMSTB_GISB_ARB' -q ${LINUX_OUT}/include/generated/autoconf.h ; then echo "y" ; fi)
NEXUS_WKTMR    := $(shell if grep 'CONFIG_BRCMSTB_WKTMR' -q ${LINUX_OUT}/include/generated/autoconf.h ; then echo "y" ; fi)

ifeq ($(NEXUS_GISB_ARB),y)
NEXUS_PLATFORM_DEFINES += NEXUS_GISB_ARB
endif
ifeq ($(NEXUS_WKTMR),y)
NEXUS_PLATFORM_DEFINES += NEXUS_WKTMR
endif

ifeq ($(NEXUS_POWER_MANAGEMENT),y)
ifeq ($(NEXUS_WKTMR),y)
NEXUS_PLATFORM_PRIVATE_INCLUDES += $(MAGNUM)/../BSEAV/linux/driver/wakeup
endif
endif

ifeq ($(NEXUS_ARM_AUDIO_SUPPORT),y)
BDSP_ARM_AUDIO_SUPPORT ?= y
NEXUS_CFLAGS += -DNEXUS_ARM_AUDIO_SUPPORT=1

ifeq ($(B_REFSW_OS),linuxkernel)
NEXUS_PLATFORM_PRIVATE_INCLUDES += $(BSEAV)/../astra/linux/bcm_astra
else
NEXUS_PLATFORM_PRIVATE_INCLUDES += $(BSEAV)/../astra/linux/libbcm_astra
endif
NEXUS_PLATFORM_DEFINES += NEXUS_TEE_SUPPORT=1
NEXUS_PLATFORM_SOURCES += $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_tee.c
endif

ifeq ($(B_REFSW_OS),linuxkernel)
    NEXUS_PLATFORM_PRIVATE_INCLUDES +=$(MAGNUM)/../BSEAV/linux/driver/usermode
    NEXUS_PLATFORM_SOURCES += \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_driver_procfs.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxuser/nexus_platform_cma.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_platform_os.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_interrupt_map.c

    ifeq ($(findstring mips, ${B_REFSW_ARCH}),mips)
    NEXUS_PLATFORM_SOURCES += \
        $(FLOATLIB)/src/fp-bit-single.c \
        $(FLOATLIB)/src/fp-bit-double.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/support/libgcc/qdivrem.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/support/libgcc/divdi3.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/support/libgcc/moddi3.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/support/libgcc/udivdi3.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/support/libgcc/umoddi3.c
    endif
    ifeq ($(findstring arm, ${B_REFSW_ARCH}),arm)
    NEXUS_PLATFORM_SOURCES += \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/support/libgcc/qdivrem.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/support/libgcc/divdi3.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/support/libgcc/moddi3.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_driver_arm.c
    endif

    ifeq ($(NEXUS_MODE),driver)
    ifneq ($(LINUX_VER_GE_2_6_31),y)
    NEXUS_PLATFORM_SOURCES += \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_driver.mod.c
    endif
    NEXUS_PLATFORM_SOURCES += \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_driver.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_generic_driver.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_driver_callbacks.c

        # nexus_generic_driver.c depends on the thunk in order to trap NEXUS_Platform_Init/Uninit
        ifeq ($(NEXUS_ABICOMPAT_MODE),y)
        $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_generic_driver.c: $(NEXUS_SYNCTHUNK_DIR)/nexus_platform_abiverify_ioctl.h
        else
        $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_generic_driver.c: $(NEXUS_SYNCTHUNK_DIR)/nexus_platform_ioctl.h
        endif

    endif

    ifeq ($(NEXUS_CONFIG_IMAGE),y)
    NEXUS_PLATFORM_SOURCES += \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_img_kernel.c
    endif
endif

ifeq ($(B_REFSW_OS),bare)
    NEXUS_PLATFORM_PRIVATE_INCLUDES += $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxkernel
# __divdi3.o & friends are libgcc files obtained from uclinux-rootfs-2.6.31-2.4.tar.bz2.tar.bz2, uclinux-rootfs/user/root_sign/libgcc
    NEXUS_PLATFORM_SOURCES += \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxkernel/nexus_generic_driver.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxkernel/nexus_driver_callbacks.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxkernel/nexus_img_kernel.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxkernel/nexus_interrupt_map.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxkernel/nexus_driver_procfs.c \
        $(FLOATLIB)/src/fp-bit-single.c \
        $(FLOATLIB)/src/fp-bit-double.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/bare/nexus_platform_os.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/bare/jumptable_fwd.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/bare/libgcc/ucmpdi2.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/bare/libgcc/__divdi3.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/bare/libgcc/__udivdi3.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/bare/libgcc/__moddi3.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/bare/libgcc/__umoddi3.c \
        $(NEXUS_PLATFORM_COMMON_DIR)/src/bare/libgcc/__udivmoddi4.c

        # NEXUS_PLATFORM_SOURCES can only take .c files, so the build rule for these files is in build/os/bare/os_rules.inc
        # $(NEXUS_PLATFORM_COMMON_DIR)/src/bare/stubs_rev.S
        # $(NEXUS_PLATFORM_COMMON_DIR)/src/bare/libgcc/memset.S
        # $(NEXUS_PLATFORM_COMMON_DIR)/src/bare/libgcc/memcpy_brcm.S

    # nexus_generic_driver.c depends on the thunk in order to trap NEXUS_Platform_Init/Uninit
    ifeq ($(NEXUS_ABICOMPAT_MODE),y)
    $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxkernel/nexus_generic_driver.c: $(NEXUS_SYNCTHUNK_DIR)/nexus_platform_abiverify_ioctl.h
    else
    $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxkernel/nexus_generic_driver.c: $(NEXUS_SYNCTHUNK_DIR)/nexus_platform_ioctl.h
    endif
endif

# allow override of socket interface
NEXUS_SOCKET_INTF ?= $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxuser/nexus_socket.c

ifeq ($(B_REFSW_OS),linuxuser)
ifeq ($(NEXUS_MODE),)
NEXUS_PLATFORM_PRIVATE_INCLUDES += $(MAGNUM)/../BSEAV/linux/driver/usermode
NEXUS_PLATFORM_SOURCES += \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_platform_os.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_platform_privilege.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/linuxuser/nexus_platform_cma.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_debug_log.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_audio_log.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_sage_log.c

ifeq ($(NEXUS_SERVER_SUPPORT),y)
NEXUS_EXTRA_INCLUDES += $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)
NEXUS_PLATFORM_SOURCES += \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_driver_callbacks.c \
    $(NEXUS_SOCKET_INTF)
endif
endif

NEXUS_PLATFORM_LOCAL_SOURCES += \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_standby_local.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_thermal_local.c
endif

ifeq ($(filter $(B_REFSW_OS),ucos ucos_iii), $(B_REFSW_OS))
NEXUS_PLATFORM_PRIVATE_INCLUDES += $(NEXUS_TOP)/modules/core/$(BCHP_CHIP)/src \
                                   $(NEXUS_TOP)/../rockford/bsp/common/ \
                                   $(MAGNUM)/../BSEAV/linux/driver/usermode

NEXUS_PLATFORM_SOURCES += \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_driver_callbacks.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_interrupt_map.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_platform_os.c
endif

ifeq ($(NEXUS_MODE),proxy)
NEXUS_PLATFORM_PRIVATE_INCLUDES += $(NEXUS_SYNCTHUNK_DIR)
# allow override of proxy thread setup
NEXUS_PLATFORM_PROXY_INTF ?= $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS).$(NEXUS_MODE)/nexus_platform_proxy_intf.c
NEXUS_PLATFORM_PROXY_SOURCES = \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS).$(NEXUS_MODE)/nexus_platform_os.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS)/nexus_platform_privilege.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_map.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_debug_log.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_audio_log.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/nexus_platform_sage_log.c \
    $(NEXUS_PLATFORM_PROXY_INTF)

ifeq ($(NEXUS_CONFIG_IMAGE),y)
NEXUS_PLATFORM_PROXY_SOURCES += \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS).$(NEXUS_MODE)/nexus_platform_image.c
ifeq ($(SAGE_SUPPORT),y)
NEXUS_PLATFORM_PROXY_SOURCES += \
    $(NEXUS_TOP)/modules/sage/src/linuxuser/nexus_sage_image.c \
    $(NEXUS_TOP)/modules/hdmi_output/src/linuxuser/nexus_hdmi_output_image.c
endif
ifeq ($(SCM_SUPPORT),y)
NEXUS_PLATFORM_PROXY_SOURCES += \
     $(NEXUS_TOP)/extensions/scm/src/linuxuser/nexus_scm_image.c
endif
endif
NEXUS_PLATFORM_SOURCES := $(NEXUS_PLATFORM_PROXY_SOURCES)
endif

ifeq ($(NEXUS_MODE),client)
NEXUS_PLATFORM_CLIENT_SOURCES := \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS).${NEXUS_MODE}/nexus_platform_os.c \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS).${NEXUS_MODE}/nexus_platform_client.c \
    $(NEXUS_SOCKET_INTF)
NEXUS_PLATFORM_SOURCES := ${NEXUS_PLATFORM_CLIENT_SOURCES}
NEXUS_PLATFORM_PRIVATE_INCLUDES += $(NEXUS_SYNCTHUNK_DIR)
NEXUS_LIB_SUFFIX :=_client

ifeq ($(NEXUS_WEBCPU),core1_server)
NEXUS_PLATFORM_CLIENT_SOURCES += \
    $(NEXUS_PLATFORM_COMMON_DIR)/src/$(B_REFSW_OS).${NEXUS_MODE}/nexus_webcpu_platform_os.c \
    $(NEXUS_TOP)/modules/core/$(BCHP_CHIP)/src/nexus_core_convert.c \
    $(NEXUS_TOP)/base/src/b_objdb.c \
    $(NEXUS_TOP)/../magnum/basemodules/mma/src/bmma_stub.c
NEXUS_PLATFORM_SOURCES := $(NEXUS_PLATFORM_CLIENT_SOURCES)
NEXUS_PLATFORM_PRIVATE_INCLUDES += $(MAGNUM)/../BSEAV/linux/driver/usermode
NEXUS_LIB_SUFFIX :=_webcpu
# see below for NEXUS_PLATFORM_DEFINES
endif

endif


##################
# Module Defines #
##################

ifeq ($(NEXUS_WEBCPU),core1_server)
# core1_server runs M2MC1 locally
NEXUS_PLATFORM_DEFINES += NEXUS_WEBCPU_core1_server
endif

ifeq ($(NEXUS_CONFIG_IMAGE),y)
NEXUS_PLATFORM_DEFINES += NEXUS_CONFIG_IMAGE
endif

ifneq ($(NEXUS_MODE),)
NEXUS_PLATFORM_DEFINES += NEXUS_MODE_$(NEXUS_MODE)
endif

ifeq ($(BCM3128_OOB_CABLECARD_SUPPORT),y)
NEXUS_PLATFORM_DEFINES += BCM3128_OOB_CABLECARD_SUPPORT=1
endif

ifeq ($(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT=1
endif

ifeq ($(NEXUS_PLATFORM_7429_CABLE),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT=1
endif

ifeq ($(NEXUS_PLATFORM_7445_CABLE),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT=1
endif

ifeq ($(NEXUS_PLATFORM_7449_CABLE),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT=1
endif

ifeq ($(NEXUS_PLATFORM_4517_DBS),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_4517_DBS=1
endif

ifeq ($(NEXUS_PLATFORM_4538_DBS),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_4538_DBS=1
endif

ifeq ($(filter $(RAP_REALVIDEO_SUPPORT) $(RAP_VP6_SUPPORT), y), y)
NEXUS_PLATFORM_DEFINES += NEXUS_NUM_ZSP_VIDEO_DECODERS=1
endif

ifneq ($(BAPE_DSP_SUPPORT),n)
ifeq ($(filter $(BDSP_VP6_SUPPORT), y), y)
NEXUS_PLATFORM_DEFINES += NEXUS_NUM_DSP_VIDEO_DECODERS=1
NEXUS_PLATFORM_DEFINES += NEXUS_BDSP_VP6_SUPPORT=1
endif

ifeq ($(filter $(BDSP_ENCODER_SUPPORT), y), y)
NEXUS_PLATFORM_DEFINES += NEXUS_NUM_DSP_VIDEO_ENCODERS=1
endif

ifeq ($(filter $(BDSP_ENCODER_ACCELERATOR_SUPPORT), y), y)
NEXUS_PLATFORM_DEFINES += NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT=1
endif
endif

ifeq ($(filter $(BSID_MJPEG_SUPPORT), y), y)
NEXUS_PLATFORM_DEFINES += NEXUS_NUM_SID_VIDEO_DECODERS=1
endif

ifeq ($(filter $(BSOFT_VIDEO_DECODER_SUPPORT), y), y)
NEXUS_PLATFORM_DEFINES += NEXUS_NUM_SOFT_VIDEO_DECODERS=1
endif

ifeq ($(NEXUS_TOUCHPAD_SUPPORT),y)
# deprecated. use #if NEXUS_HAS_TOUCHPAD, which is auto-defined
NEXUS_PLATFORM_DEFINES += NEXUS_TOUCHPAD_SUPPORT
endif

ifeq ($(NEXUS_TEMP_MONITOR_SUPPORT),y)
# deprecated. use #if NEXUS_HAS_TEMP_MONITOR, which is auto-defined
NEXUS_PLATFORM_DEFINES += NEXUS_TEMP_MONITOR_SUPPORT
endif

NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM=$(NEXUS_PLATFORM)

# Add DIAGS module
ifeq ($(DIAGS),y)
NEXUS_PLATFORM_DEFINES += DIAGS
endif

# Add DIAGS_MEM_DMA_TEST
ifeq ($(DIAGS_MEM_DMA_TEST),y)
NEXUS_PLATFORM_DEFINES += DIAGS_MEM_DMA_TEST
endif

#######################
# Module Dependencies #
#######################
NEXUS_PLATFORM_DEPENDENCIES = $(filter-out PLATFORM, $(NEXUS_MODULES))

##################
# Magnum Modules #
##################
NEXUS_PLATFORM_MAGNUM_MODULES =

ifneq ($(NEXUS_SYSTEM_CABLE)$(NEXUS_PLATFORM_7425_CABLE)$(NEXUS_PLATFORM_7422_CABLE)$(NEXUS_PLATFORM_7445_CABLE)$(NEXUS_PLATFORM_7449_CABLE),)
include $(NEXUS_TOP)/extensions/platform/cable_frontend/platform_cable_frontend.inc
endif
