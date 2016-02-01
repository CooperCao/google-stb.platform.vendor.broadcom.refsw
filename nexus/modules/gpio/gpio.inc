############################################################
#     (c)2007-2012 Broadcom Corporation
#  
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
# 
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

#
# Conditional Options for this Makefile
#

######################
# Define this module #
######################
NEXUS_MODULES += GPIO

##################
# Module Options #
##################

###################
# Module Includes #
###################
NEXUS_GPIO_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/gpio/include

##################
# Module Sources #
##################
NEXUS_GPIO_SOURCES := \
	$(NEXUS_TOP)/modules/gpio/src/nexus_gpio.c

ifeq ($(shell test -e $(NEXUS_TOP)/modules/gpio/src/${BCHP_CHIP}/nexus_gpio_table.c && echo y),y)
NEXUS_GPIO_TABLE := $(NEXUS_TOP)/modules/gpio/src/${BCHP_CHIP}/nexus_gpio_table.c
else
NEXUS_GPIO_TABLE := $(B_REFSW_OBJ_ROOT)/nexus/core/syncthunk/nexus_gpio_table.c
$(NEXUS_GPIO_TABLE):
	${Q_}cat $(NEXUS_TOP)/modules/gpio/src/nexus_gpio_table_prologue.c >$@
	${Q_}$(PERL) $(NEXUS_TOP)/modules/gpio/generate_nexus_gpio_mapping.pl $(NEXUS_TOP)/../magnum/basemodules/chp/include/${BCHP_CHIP}/rdb/${BCHP_VER_LOWER}/bchp_aon_pin_ctrl.h >>$@
	${Q_}$(PERL) $(NEXUS_TOP)/modules/gpio/generate_nexus_gpio_mapping.pl $(NEXUS_TOP)/../magnum/basemodules/chp/include/${BCHP_CHIP}/rdb/${BCHP_VER_LOWER}/bchp_sun_top_ctrl.h >>$@
	${Q_}cat $(NEXUS_TOP)/modules/gpio/src/nexus_gpio_table_epilogue.c >>$@
	${Q_}cat $(NEXUS_TOP)/modules/gpio/src/${BCHP_CHIP}/nexus_gpio_table_epilogue_${BCHP_CHIP}.c >>$@
endif

NEXUS_GPIO_SOURCES += $(NEXUS_GPIO_TABLE)

# thunk layer
NEXUS_GPIO_PRIVATE_INCLUDES += $(NEXUS_TOP)/modules/gpio/src $(NEXUS_SYNCTHUNK_DIR)
NEXUS_GPIO_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_gpio_thunks.c

#######################
# Module Dependencies #
#######################
NEXUS_GPIO_DEPENDENCIES := CORE

##################
# Magnum Modules #
##################
# These modules will be excluded from app-only builds
NEXUS_GPIO_MAGNUM_MODULES =  
