############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
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
# Created: 01/09/2007 by Jon Garrett
#
# $brcm_Log: $
#
############################################################
#
# Conditional Options for this Makefile
#
# BXVD_IMG_ONLY - If defined, only the firmware image related code will be compiled.
# BXVD_IMG_EXTERNAL - If defined, firmware related code (interface AND context) will NOT be compiled. Also defines -DBXVD_USE_CUSTOM_IMAGE.  The app is expected to specify their own firmware interface AND image context when calling BXVD_Open().
# BXVD_USE_CUSTOM_CONTEXT - If defined, the firmware image context will NOT be compiled. The app is expected to provide their own firmware image context when calling BXVD_Open().
# BXVD_USE_DM_V1 - If defined, Display Manager (DM) version 1.x will be used.
# BXVD_POWER_MANAGEMENT - Defaults to 'y' (yes) on DTV platforms, to 'n' (no) used to enable dynamic power management feature
# BXVD_FW_DEBUG_DRAM_LOGGING - If 'y' (yes), starts AVD FW DRAM logging and reads and prints out the debug log during picture data ready_isr
# BXVD_FLATTEN_USERDATA - If 'y' (yes), the userdata code will coalesce userdata packets otherwise, they will be delivered singly.
# BXVD_BREAK_ON_TYPE_CHANGE - If 'y' (yes), the userdata packet whose UD type is different from other packets in a group will be delivered separately from coalesced packets of the same type. Must be used in conjunction with BXVD_FLATTEN_USERDATA=1
#

# Define this module
MAGNUM_MODULES += BXVD

#
# When true, select the BDBG_INSTANCE_* macros, otherwise use BDBG_*
#
ifeq ($(BXVD_USE_INSTANCE_MACROS),y)
BXVD_DEFINES += BXVD_P_USE_INSTANCE_MACROS
endif

#
# When true, coalesce userdata packets, otherwise they are delivered singly
#
ifeq ($(BXVD_FLATTEN_USERDATA),y)
BXVD_DEFINES += BXVD_FLATTEN_USERDATA
endif

#
# When true, along with BXVD_FLATTEN_USERDATA, a change in userdata type will cause packet coalescing to stop
#
ifeq ($(BXVD_BREAK_ON_TYPE_CHANGE),y)
BXVD_DEFINES += BXVD_BREAK_ON_TYPE_CHANGE
endif

#
# When true, this will cause a live dump of userdata packets. Use with caution as this will have
# a negative effect on packet timing.
#
ifeq ($(BXVD_USERDATA_EXTRA_DEBUG),y)
BXVD_DEFINES += BXVD_USERDATA_EXTRA_DEBUG
endif

ifeq ($(BXVD_P_USE_BINARY_IMAGE),y)
BXVD_DEFINES += BXVD_P_USE_BINARY_IMAGE
endif

include $(MAGNUM)/portinginterface/xvd/bxvd_chip.inc

#
# Power management is enabled by default on DTV chips
#
ifeq ($(findstring $(BCHP_CHIP), 3548 3556 35233), $(BCHP_CHIP))
ifneq ($(BXVD_POWER_MANAGEMENT),n)
BXVD_DEFINES += BXVD_P_POWER_MANAGEMENT
endif
endif

#
# Power management is disabled by default on certain platforms
#

ifneq ($(findstring $(BCHP_CHIP), 35125 35230 7135 7325 7336 7340 7342 7401 7403 7440 7550 7601 7630 7635 7640), $(BCHP_CHIP))
ifeq ($(BXVD_POWER_MANAGEMENT),y)
BXVD_DEFINES += BXVD_P_POWER_MANAGEMENT
endif
endif

#
# Force the SVD picture buffers to be on MEMC0
#
ifeq ($(BXVD_SVD_USE_MEMC0),y)
BXVD_DEFINES += BXVD_P_SVD_USE_MEMC0
endif

#
# For MVC, set the pNext on the older DVD platforms.
#
ifeq ($(findstring $(BCHP_CHIP), 7630 7631 7632 7633 7634), $(BCHP_CHIP))
ifneq ($(BXVD_LEGACY_MVC_SUPPORT),n)
BXVD_DEFINES += BXVD_LEGACY_MVC_SUPPORT
endif
endif

#
# Enable FW DRAM Log output to be read and printed in PDR_isr()
#
ifeq ($(BXVD_FW_DEBUG_DRAM_LOGGING),y)
BXVD_DEFINES += BXVD_P_FW_DEBUG_DRAM_LOGGING
endif

#### Use DMv2 by default for all platforms
BXVD_DM_VERSION = 2

ifeq ($(BXVD_USE_DM_V1),)
else
BXVD_DM_VERSION = 1
endif

#### Module Includes ####
BXVD_INCLUDES += $(MAGNUM)/portinginterface/xvd/include

#### Source file(s) ####
ifeq ($(BXVD_IMG_ONLY),)
#### Common file(s) ####
BXVD_SOURCES += \
	$(MAGNUM)/portinginterface/xvd/src/bxvd.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_devcmds_priv.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_relf.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_priv.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_reg.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_userdata.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_pvr.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_intr.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_memory_priv.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_status.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_dbg.c

#### Core file(s) ####
BXVD_SOURCES += \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_core_avd_reve0.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_core_avd_revk0.c \
	$(MAGNUM)/portinginterface/xvd/src/bxvd_core_avd_revn0.c

#### Platform specific file(s) ####
ifeq ($(BXVD_DM_VERSION),2)
BXVD_SOURCES += \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_dip.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_decoder.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_decoder_timer.c \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_decoder_dbg.c
endif

BXVD_SOURCES += \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_platform_$(BXVD_CHIP).c

endif # BXVD_IMG_ONLY

#### Firmware image file(s) ####
ifeq ($(BXVD_IMG_EXTERNAL),)
# Common image file(s)
BXVD_SOURCES +=	\
        $(MAGNUM)/portinginterface/xvd/src/bxvd_image.c

ifeq ($(BXVD_USE_CUSTOM_CONTEXT),)
# Platform image file(s)
BXVD_SOURCES += \
        $(MAGNUM)/portinginterface/xvd/src/bxvd_platform_$(BXVD_CHIP)_image.c
endif # !BXVD_USE_CUSTOM_CONTEXT

else # !BXVD_USE_CUSTOM_IMAGE
BXVD_DEFINES += BXVD_USE_CUSTOM_IMAGE
endif

