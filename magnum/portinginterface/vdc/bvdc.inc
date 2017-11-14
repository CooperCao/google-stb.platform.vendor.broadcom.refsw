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
#
# File Description:
#
#############################################################################
#
# Conditional Options for this Makefile
#
# BVDC_MACROVISION - If set to 'y', macrovision support will be added.
# BVDC_DCS - If set to 'y', DCS support will be added.
# BVDC_DBV_SUPPORT - If set to 'y', DBV support will be added.
#

##
# Chip specific selection
#
BVDC_VEC_DIR = orthogonalvec

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
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_cfc_priv.c \
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
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_chromacoeff_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_vnetcrc_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_coeff_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_compositor.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_compositor_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_csc_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_cfc_priv.c \
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
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_hscaler_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_mcdi_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_mcvp_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_pepalgorithm_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_pep_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_resource_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_scaler_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_source.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_source_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_subrul_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_test.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_tnt_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_vnet_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_window.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_window_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_xsrc_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/common/bvdc_vfc_priv.c \
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

ifneq ($(B_PI_FOR_BOOTUPDATER),y)

##
# DBV files selection
#
BCHP_DBV_HW_DEF=$(NEXUS_TOP)/../magnum/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)/bchp_hdr_cmp_0_dbv.h
BVDC_DBV_IMPL=$(MAGNUM)/portinginterface/vdc/src/dbv
ifneq ("$(wildcard $(BCHP_DBV_HW_DEF))","")
ifneq ("$(wildcard $(BVDC_DBV_IMPL))","")
BVDC_DBV_SUPPORT?=y
endif
endif
ifeq ($(BVDC_DBV_SUPPORT),y)
include $(MAGNUM)/portinginterface/vdc/src/dbv/bdbv_api.inc
BVDC_DEFINES += BVDC_DBV_SUPPORT=1

##
# DBV conformance mode:
# 0 - BVN processing follows regular API control without DBV conformance
# 1 - BVN filters are all disabled to conform with DBV
# 2 - BVN filters are mostly disabled to conform with DBV
#
BVDC_DBV_MODE_BVN_CONFORM?=y
ifeq ($(BVDC_DBV_MODE_BVN_CONFORM),y)
BVDC_DEFINES += BVDC_DBV_MODE_BVN_CONFORM=1
else
BVDC_DEFINES += BVDC_DBV_MODE_BVN_CONFORM=0
endif
ifeq ($(BVDC_DBV_GEN_LUTS), y)
BVDC_DEFINES += BVDC_DBV_GEN_LUTS=1
ifeq ($(BVDC_DBV_ORIG_GEN_LUTS), y)
BVDC_DEFINES += BVDC_DBV_ORIG_GEN_LUTS=1
endif
endif
BVDC_INCLUDES += \
    $(BDBV_ROOT)/$(BDBV_REF_VER)/src
BVDC_SOURCES += \
	$(BDBV_ROOT)/$(BDBV_REF_VER)/src/bvdc_cfc_dbv_priv.c
endif

##
# TCH files selection
#
BVDC_TCH_IMPL=$(MAGNUM)/portinginterface/vdc/src/tch
ifneq ("$(wildcard $(BVDC_TCH_IMPL))","")
BVDC_TCH_SUPPORT?=y
endif
ifeq ($(BVDC_TCH_SUPPORT),y)
include $(MAGNUM)/portinginterface/vdc/src/tch/btch.inc
BVDC_DEFINES += BVDC_TCH_SUPPORT=1
endif

endif ### ifneq ($(B_PI_FOR_BOOTUPDATER),y)

##
# Vec files selection
#
BVDC_SOURCES += \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_displaystg_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_displayvip_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_displayhandler_priv.c \
	$(MAGNUM)/portinginterface/vdc/src/$(BVDC_VEC_DIR)/bvdc_displaycabledetect_priv.c

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
ifeq ($(BVDC_DITHER_OFF),y)
BVDC_DEFINES += BVDC_DITHER_OFF=1
endif

##
# SD path frame capture for 50i-to-60i and 60i-to-50i
ifeq ($(BVDC_ENABLE_50HZ_60HZ_FRAME_CAPTURE),y)
BVDC_DEFINES += BVDC_ENABLE_50HZ_60HZ_FRAME_CAPTURE=1
endif

ifeq ($(BVDC_ENABLE_60HZ_50HZ_FRAME_CAPTURE),y)
BVDC_DEFINES += BVDC_ENABLE_60HZ_50HZ_FRAME_CAPTURE=1
endif

##
# End of file!
#
