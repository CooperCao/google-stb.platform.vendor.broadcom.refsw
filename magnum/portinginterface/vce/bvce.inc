#############################################################################
#
#       Copyright (c) 2003-2013, Broadcom Corporation.
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
# [File Description:]
#
# Revision History:
#
# $brcm_Log: $
# 
#############################################################################

############################################################
#
# Conditional Options for this Makefile
#
# BVCE_IMG_ONLY - If defined, only the firmware image related code will be compiled.
# BVCE_IMG_EXTERNAL - If defined, firmware related code (interface AND context) will NOT be compiled. Also defines -DBVCE_USE_CUSTOM_IMAGE.  The app is expected to specify their own firmware interface AND image context when calling BVCE_Open().
# BVCE_USE_CUSTOM_CONTEXT - If defined, the firmware image context will NOT be compiled. The app is expected to provide their own firmware image context when calling BVCE_Open().
# BVCE_IMG_FILE - If defined, firmware will be loaded from files: bvce_fw_image_[picarc/mbarc].bin OR the file(s) is specified in the environment variables: BVCE_IMG_PICARC and/or BVCE_IMG_MBARC

# Define this module
MAGNUM_MODULES += BVCE

#
# Select VCE CHIP family
#
BVCE_CHIP = $(BCHP_CHIP)

#### Module Includes ####
BVCE_INCLUDES += \
	$(MAGNUM)/portinginterface/vce/include \
	$(MAGNUM)/portinginterface/vce/src

#### Source File(s) ####
ifeq ($(BVCE_IMG_ONLY),)
#### Common File(s) ####
BVCE_SOURCES_LOCAL += \
             $(MAGNUM)/portinginterface/vce/src/bvce.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_platform.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_platform_$(BVCE_CHIP).c \
             $(MAGNUM)/portinginterface/vce/src/bvce_fw_api.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_telem.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_output.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_power.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_debug_priv.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_core.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_buffer.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_debug_common.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_image_common.c

#### Core File(s) ####

#### Platform File(s) ####

endif # BVCE_IMG_ONLY

#### Firmware image file(s) ####
ifeq ($(BVCE_IMG_FILE),)
ifeq ($(BVCE_IMG_EXTERNAL),)
# Common image file(s)
BVCE_SOURCES_LOCAL +=	\
             $(MAGNUM)/portinginterface/vce/src/bvce_image.c \
             $(MAGNUM)/portinginterface/vce/src/bvce_image_common.c

ifeq ($(BVCE_USE_CUSTOM_CONTEXT),)
# Platform image file(s)
BVCE_SOURCES_LOCAL += \
             $(MAGNUM)/portinginterface/vce/src/bvce_core_image.c
endif # !BVCE_USE_CUSTOM_CONTEXT

else # !BVCE_USE_CUSTOM_IMAGE
BVCE_DEFINES += BVCE_USE_CUSTOM_IMAGE
endif #!BVCE_IMG_EXTERNAL
else
# Common image file(s)
BVCE_SOURCES_LOCAL +=	\
             $(MAGNUM)/portinginterface/vce/src/bvce_image_file.c
BVCE_DEFINES += BVCE_USE_FILE_IMAGE=1
endif

BVCE_DEBUG_INCLUDES += \
    $(MAGNUM)/portinginterface/vce/include \
    $(MAGNUM)/portinginterface/vce/src

BVCE_DEBUG_SOURCES += \
	$(MAGNUM)/portinginterface/vce/src/bvce_debug.c

# DEBUG Build Flags
ifeq ($(BVCE_DUMP_OUTPUT_CDB),y)
BVCE_DEFINES += BVCE_P_DUMP_OUTPUT_CDB=1
endif
ifeq ($(BVCE_DUMP_OUTPUT_ITB),y)
BVCE_DEFINES += BVCE_P_DUMP_OUTPUT_ITB=1
endif
ifeq ($(BVCE_DUMP_OUTPUT_ITB_DESC),y)
BVCE_DEFINES += BVCE_P_DUMP_OUTPUT_ITB_DESC=1
endif
ifeq ($(BVCE_DUMP_MAU_PERFORMANCE_DATA),y)
BVCE_DEFINES += BVCE_P_DUMP_MAU_PERFORMANCE_DATA=1
endif
ifeq ($(BVCE_ENABLE_UART),y)
BVCE_DEFINES += BVCE_P_ENABLE_UART=1
endif
ifeq ($(BVCE_ENABLE_UART),1)
BVCE_DEFINES += BVCE_P_ENABLE_UART=1
endif
ifeq ($(BVCE_ENABLE_UART),2)
BVCE_DEFINES += BVCE_P_ENABLE_UART=2
endif
ifeq ($(BVCE_DUMP_ARC_DEBUG_LOG),y)
BVCE_DEFINES += BVCE_P_DUMP_ARC_DEBUG_LOG=1
endif
ifeq ($(BVCE_DUMP_USERDATA_LOG),y)
BVCE_DEFINES += BVCE_P_DUMP_USERDATA_LOG=1
endif
ifeq ($(BVCE_TEST_MODE),y)
BVCE_DEFINES += BVCE_P_TEST_MODE=1
endif
ifeq ($(BVCE_PRERELEASE_TEST_MODE),y)
BVCE_DEFINES += BVCE_P_PRERELEASE_TEST_MODE=1
endif

# Remove duplicate sources to fix duplicate linker errors
BVCE_SOURCES += $(sort $(BVCE_SOURCES_LOCAL))