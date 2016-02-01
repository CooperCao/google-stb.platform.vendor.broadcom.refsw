#############################################################################
#
#       Copyright (c) 2005-2014, Broadcom Corporation.
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
# File Description:
#
# Revision History:
#
# $brcm_Log: $
#
#############################################################################
#
# Conditional Options for this Makefile
#
# BVDC_MACROVISION - If set to 'y', macrovision support will be added.
# BVDC_DCS - If set to 'y', DCS support will be added.
#

##
# Chip specific selections!!!
#
ifeq ($(filter $(BCHP_CHIP), 7325 7335 7336 7400 7405), $(BCHP_CHIP))
	BVDC_VEC_DIR = modularvec
else
	BVDC_VEC_DIR = orthogonalvec
endif

##
# Define this module
#
MAGNUM_MODULES += BVDC

##
# Module Includes
#
BVDC_INCLUDES = \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR) \
	$(MAGNUM)/portinginterface/vdc/src/common \
	$(MAGNUM)/portinginterface/vdc/include
##
# Module Sources, core file selection
#
ifeq ($(B_PI_FOR_BOOTUPDATER),y)
# To build shrinked bfmt and bvdc for BOOTUPDATER, refer to
# /rockford/applications/vdc_for_bootupdater/Makefile
BVDC_DCS = n
BVDC_MACROVISION = n
CFLAGS += -DBVDC_FOR_BOOTUPDATER
CFLAGS += -Os -fdata-sections -ffunction-sections -Wl,--gc-sections
BVDC_SOURCES = \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_compositor.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_compositor_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_displayfmt_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_csc_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_displaymatrices_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_gfxfeeder.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_gfxfeedertable_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_gfxfeeder_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_gfxsurface_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_resource_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_source.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_source_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_test.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_window.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_window_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_display.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_display_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_bootupdater.c
else  ### ifeq ($(B_PI_FOR_BOOTUPDATER),y)
BVDC_SOURCES = \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_656in_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_anrtable_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_anralg_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_anr_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_boxdetect_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_bufferheap_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_buffer_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_capture_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_ccbtable_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_ccb_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_chromacoeff_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_vnetcrc_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_coeff_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_compositor.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_compositor_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_csc_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_dbg.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_bvnerrtable_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_displayfmt_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_displaymatrices_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_dnrtable_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_dnr_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_feeder_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_gfxfeeder.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_gfxfeedertable_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_gfxfeeder_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_gfxsurface_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_hddvi_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_heap.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_hist_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_hscaler_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_mad_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_mcdi_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_mcvp_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_oscl_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_pepalgorithm_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_pepcmstable_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_pepcms_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_pepdyncontrast_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_pep_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_resource_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_scaler_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_source.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_source_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_subrul_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_test.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_tntalgorithm_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_tnt_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_videotable_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_vnet_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_window.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_window_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_xsrc_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_tntd_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_memconfig.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_memconfig_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_display.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_display_priv.c
endif ### ifeq ($(B_PI_FOR_BOOTUPDATER),y)

##
# Copy protection files selection
#
ifeq ($(BVDC_MACROVISION),y)
BVDC_SOURCES += \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_macrovision.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_macrovision_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_macrovisiontable_priv.c
else
ifeq ($(BVDC_DCS),y)
BVDC_SOURCES += \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_dcs.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_dcs_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_dcstable_priv.c
else
BVDC_SOURCES += \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_nomacrovision_priv.c
endif
endif

##
# Vec files selection
#
ifeq ($(BVDC_VEC_DIR),orthogonalvec)
BVDC_SOURCES += \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_displaystg_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_displayvip_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_displayhandler_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_displaycabledetect_priv.c
endif
ifeq ($(BVDC_VEC_DIR),modularvec)
BVDC_SOURCES += \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_displaytable_priv.c
endif

##
# For boot loder or mini-driver to keep gfx display on
#
ifeq ($(B_REFSW_GFX_PERSIST),y)
BVDC_DEFINES += BVDC_GFX_PERSIST
endif

##
# For VIP capture dump
#
ifeq ($(BVDC_DUMP_VIP_CAPTURE), y)
BVDC_DEFINES += BVDC_P_DUMP_VIP_PICTURE=1
endif

##
# CRC capture need picture-to-picture match, disable dithering
#
ifeq ($(BVDC_CRC_CAPTURE),y)
BVDC_DEFINES += BVDC_CRC_CAPTURE=1
endif

##
# End of file!
#
