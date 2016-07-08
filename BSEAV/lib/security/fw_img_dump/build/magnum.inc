##############################################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
##############################################################################


#############################################################################
# The purpose of this makefile is to add object files, include paths,
# and C-flags required to build magnum base modules, common utilities, and
# "lower" porting interfaces.
#
# The following defines are the result:
#  MAGNUM_SOURCES
#  MAGNUM_INCLUDES
#  MAGNUM_CFLAGS
#
# This file also defines the location for all the magnum modules
#  MAGNUM_MODULE_DIR

#############################################################################

#############################################################################
# Get appropriate value for BCHP_CHIP
include ${FRAMEWORK_DIR}/build/plat.inc

#############################################################################
# basemodules	(add to MAGNUM_SOURCES and MAGNUM_INCLUDES)

BASEMODULES = err std dbg kni chp int reg tmr mma

include $(foreach module, ${BASEMODULES}, ${MAGNUM_DIR}/basemodules/${module}/b${module}.inc)

#############################################################################
# commonutils (add to MAGNUM_SOURCES and MAGNUM_INCLUDES)

COMMONUTILS = afl lst img avc fmt pxl udp box

include $(foreach module, ${COMMONUTILS}, ${MAGNUM_DIR}/commonutils/${module}/b${module}.inc)

#############################################################################
# defines required to build magnum
# NEXUS_PLATFORM_DEFINES is a value local to this makefile.

NEXUS_PLATFORM_DEFINES += \
	BCHP_CHIP=$(BCHP_CHIP) \
	BCHP_REV=$(BCHP_VER) \
	BCHP_VER=BCHP_VER_$(BCHP_VER) \
	FRAMEWORK_NEXUS_PLATFORM=$(NEXUS_PLATFORM) \
	FRAMEWORK_DO_FPGA=$(FRAMEWORK_DO_FPGA) \
	FRAMEWORK_DO_I2C=$(FRAMEWORK_DO_I2C)

# Additional chipset versions
NEXUS_PLATFORM_DEFINES += BSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE

# default is debug build
B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
NEXUS_PLATFORM_DEFINES += BDBG_DEBUG_BUILD=1
endif

MAGNUM_CFLAGS += $(addprefix -D, ${NEXUS_PLATFORM_DEFINES})

#
# End of file
#
