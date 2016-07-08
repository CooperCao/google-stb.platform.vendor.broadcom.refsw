############################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
# Module Description:
#
############################################################

#
# Conditional Options for this Makefile
#
# N/A
#

##
# Chip specific selections
#
BVBI_VEC_DIR = orthogonalvec

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
