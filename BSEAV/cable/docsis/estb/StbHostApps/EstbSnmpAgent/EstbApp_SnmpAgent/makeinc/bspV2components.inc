##############################################################################
#
# Copyright (c) 2007-2009 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
# AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
# SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
##############################################################################
############################################################################
##                 DEFINES
############################################################################ 

DEBUG           = 1
TARGET_DIR      = $(TARGETDIR)

############################################################################
##                 PATHS
############################################################################                                       
BSP_COMMON              = ../../../../../cm_bsp_v2/bsp_common
BSP_COMMON_INCLUDE      = $(BSP_COMMON)/common_inc
BSP_COMMON_SRC          = $(BSP_COMMON)/common_src
BSP_OS_SRC              = $(BSP_COMMON)/os/$(LC_TARGETOS)
CMHAL_SRC               = $(BSP_COMMON)/CmHal
NIHAL_SRC               = $(BSP_COMMON)/nihal

ifeq ($(TARGETOS),Linux)
  ADDED_CFLAGS += -DPROCESSOR_TYPE=BCMMIPSEL
else
  ADDED_CFLAGS += -DPROCESSOR_TYPE=BCMMIP        \
                  -DTARGETOS_$(TARGETOS)
endif

############################################################################
##                 OBJECT FILES
############################################################################
MACH_EXTRA += crc.o


############################################################################
##                 EXTRA_INCLUDE - include path
############################################################################                                       

EXTRA_INCLUDE +=-I$(BSP_OS_SRC) \
                -I$(CMHAL_SRC) \
                -I$(NIHAL_SRC) \
                -I$(BSP_COMMON_INCLUDE)

############################################################################
##                 VPATH - source path
############################################################################                                       
VPATH        += $(BSP_COMMON_SRC) $(BSP_COMMON_INCLUDE):
