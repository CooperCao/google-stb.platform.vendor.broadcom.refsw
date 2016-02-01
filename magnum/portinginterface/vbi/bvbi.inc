############################################################
#     Copyright (c) 2003-2012, Broadcom Corporation
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
# N/A
#

##
# Chip specific selections!!!
#
ifeq ($(filter $(BCHP_CHIP), 7325 7335 7336 7400 7405), $(BCHP_CHIP))
    BVBI_VEC_DIR = modularvec
else
    BVBI_VEC_DIR = orthogonalvec
endif

# Define this module
MAGNUM_MODULES += BVBI

# Module Includes
BVBI_INCLUDES += \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR) \
	$(MAGNUM)/portinginterface/vbi/src/common \
	$(MAGNUM)/portinginterface/vbi/include

# Module Sources, common.
BVBI_SOURCES += \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_amol.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_cc.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_cgms.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_dec.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_enc.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_field.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_gs.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_mcc.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_p656.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_prot.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_scte.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_tt.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_util_priv.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_vps.c \
	$(MAGNUM)/portinginterface/vbi/src/common/bvbi_wss.c

# Module sources, chip-specific.
BVBI_SOURCES += \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_a656.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_amole.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_cce.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_cgmse.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_gse.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_in656.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_itu656.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_mcce.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_sctee.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_tte.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_ve.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_veccore.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_vie.c \
 	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_vpse.c \
	$(MAGNUM)/portinginterface/vbi/src/$(BVBI_VEC_DIR)/bvbi_wsse.c
