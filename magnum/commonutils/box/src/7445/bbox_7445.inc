#################################################
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
#################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# D0 and E0 have identical box mode files
BCHP_VER_LOWER_OVERRIDE:=d0


BBOX_SOURCES += \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_priv.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_priv_vce.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_priv_xvd.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box1_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box2_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box3_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box4_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box5_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box6_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box7_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box8_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box10_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box12_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box13_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box14_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box15_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box16_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box1000_config.c \
    $(MAGNUM)/commonutils/box/src/$(BCHP_CHIP)/$(BCHP_VER_LOWER_OVERRIDE)/bbox_memc_box1001_config.c

#
# End of file
#
