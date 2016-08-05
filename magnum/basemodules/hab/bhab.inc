############################################################
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
#
############################################################

#
# Conditional Options for this Makefile
#
# BHAB_IMG_ONLY - If defined, only the firmware image related code will be compiled.
# BHAB_IMG_EXTERNAL - If defined, firmware related code (interface AND context) will NOT be compiled.
#

# Define this module
MAGNUM_MODULES += BHAB

# If the chip version is not specified, select a default
ifeq ($(BCHP_312X_VER),)
BCHP_312X_VER := C0
endif

ifeq ($(BCHP_3158_VER),)
BCHP_3158_VER := A0
endif

ifeq ($(BCHP_3461_VER),)
BCHP_3461_VER := B0
endif

ifeq ($(BCHP_3462_VER),)
BCHP_3462_VER := A0
endif

ifeq ($(BCHP_3472_VER),)
BCHP_3472_VER := B0
endif

ifeq ($(BCHP_7364_VER),)
BCHP_7364_VER := A0
endif

ifeq ($(BCHP_4528_VER),)
BCHP_4528_VER := A0
endif

ifeq ($(BCHP_4517_VER),)
BCHP_4517_VER := B0
endif

ifeq ($(BCHP_4538_VER),)
BCHP_4538_VER := A0
endif

ifeq ($(BCHP_7366_VER),)
BCHP_7366_VER := C0
endif

ifeq ($(BCHP_45216_VER),)
BCHP_45216_VER := A0
endif

ifeq ($(BCHP_45308_VER),)
BCHP_45308_VER := B0
endif

BHAB_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_312X_VER)")}')

BHAB_3461_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_3461_VER)")}')
BHAB_3462_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_3462_VER)")}')
BHAB_3472_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_3472_VER)")}')
BHAB_7364_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_7364_VER)")}')
BHAB_4528_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_4528_VER)")}')
BHAB_4538_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_4538_VER)")}')
BHAB_4517_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_4517_VER)")}')
BHAB_7366_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_7366_VER)")}')
BHAB_45216_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_45216_VER)")}')
BHAB_45308_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_45308_VER)")}')
BHAB_3158_VER_LOWER ?= $(shell awk 'BEGIN{print tolower("$(BCHP_3158_VER)")}')

# Source needs to know which revision
BHAB_DEFINES += BHAB_312X_VER=BCHP_VER_$(BCHP_312X_VER)
BHAB_DEFINES += BHAB_3461_VER=BCHP_VER_$(BCHP_3461_VER)
BHAB_DEFINES += BHAB_45216_VER=BCHP_VER_$(BCHP_45216_VER)
BHAB_DEFINES += BHAB_45308_VER=BCHP_VER_$(BCHP_45308_VER)

# Module Includes
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/common
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/common
BHAB_INCLUDES += $(MAGNUM)/commonutils/fec/include
BHAB_INCLUDES += $(MAGNUM)/commonutils/img

ifneq ($(BHAB_3461_SUPPORT),)
ifeq ($(BHAB_LEAP_UART_ENABLE),y)
BHAB_DEFINES += BHAB_LEAP_UART_ENABLE
endif
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/3461/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/3461/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/common/
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/3461/rdb/hab/$(BHAB_3461_VER_LOWER)
ifeq ($(BHAB_IMG_ONLY),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/3461/bhab_3461.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/3461/bhab_3461_priv.c
endif
ifeq ($(BHAB_IMG_EXTERNAL),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/3461/bhab_3461_fw_$(BHAB_3461_VER_LOWER).c
endif
endif

ifneq ($(BHAB_3462_SUPPORT),)
ifeq ($(BHAB_IMG_ONLY),)
BHAB_DEFINES += BHAB_CHIP=3462
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/3462/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/3462/rdb/hab/$(BHAB_3462_VER_LOWER)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/3462/bhab_3462.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/bhab_34xx_priv.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/3462/bhab_3462_fw_$(BHAB_3462_VER_LOWER).c
endif
endif

ifneq ($(BHAB_3472_SUPPORT),)
ifeq ($(BHAB_IMG_ONLY),)
ifeq ($(BHAB_LEAP_UART_ENABLE),y)
BHAB_DEFINES += BHAB_LEAP_UART_ENABLE
endif
BHAB_DEFINES += BHAB_CHIP=3472
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/3472/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/3472/rdb/hab/$(BHAB_3472_VER_LOWER)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/3472/bhab_3472.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/bhab_34xx_priv.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/3472/bhab_3472_fw_$(BHAB_3472_VER_LOWER).c
endif
endif

ifneq ($(BHAB_3128_SUPPORT),)
ifeq ($(BHAB_LEAP_UART_ENABLE),y)
BHAB_DEFINES += BHAB_LEAP_UART_ENABLE
endif
BHAB_312X_CHIP := 3128
BHAB_RDB_CHIP := 3128
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/3128/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/3128/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/common/
ifeq ($(BHAB_IMG_ONLY),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/3128/bhab_3128.c
endif
ifeq ($(BHAB_IMG_EXTERNAL),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/3128/bhab_3128_fw_c0.c
endif
endif

ifneq ($(BHAB_3158_SUPPORT),)
ifeq ($(BHAB_LEAP_UART_ENABLE),y)
BHAB_DEFINES += BHAB_LEAP_UART_ENABLE
endif
BHAB_DEFINES += BHAB_CHIP=3158
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/3158/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/common/
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/3158/rdb/hab/$(BHAB_3158_VER_LOWER)
ifeq ($(BHAB_IMG_ONLY),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/3158/bhab_3158.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/bhab_leap_priv.c
endif
ifeq ($(BHAB_IMG_EXTERNAL),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/3158/bhab_3158_fw_$(BHAB_3158_VER_LOWER).c
endif
endif

ifneq ($(BHAB_7584_SUPPORT),)
ifeq ($(BHAB_LEAP_UART_ENABLE),y)
BHAB_DEFINES += BHAB_LEAP_UART_ENABLE
endif
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/7584/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/7584/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/common/
ifeq ($(BHAB_IMG_ONLY),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/7584/bhab_7584.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/7584/bhab_7584_priv.c
endif
ifeq ($(BHAB_IMG_EXTERNAL),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/7584/bhab_7584_fw_a0.c
endif
endif

ifneq ($(BHAB_7563_SUPPORT),)
ifeq ($(BHAB_LEAP_UART_ENABLE),y)
BHAB_DEFINES += BHAB_LEAP_UART_ENABLE
endif
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/7563/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/7563/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/common/
ifeq ($(BHAB_IMG_ONLY),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/7563/bhab_7563.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/7563/bhab_7563_priv.c
endif
ifeq ($(BHAB_IMG_EXTERNAL),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/7563/bhab_7563_fw_$(BCHP_VER_LOWER).c
endif
endif

ifneq ($(BHAB_7364_SUPPORT),)
ifeq ($(BHAB_LEAP_UART_ENABLE),y)
BHAB_DEFINES += BHAB_LEAP_UART_ENABLE
endif
BHAB_DEFINES += BHAB_CHIP=7364
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/$(BCHP_CHIP)/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/common/
ifeq ($(BHAB_IMG_ONLY),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/$(BCHP_CHIP)/bhab_7364.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/bhab_leap_priv.c
endif
ifeq ($(BHAB_IMG_EXTERNAL),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/$(BCHP_CHIP)/bhab_7364_fw_$(BHAB_7364_VER_LOWER).c
endif
endif

ifneq ($(BHAB_75525_SUPPORT),)
ifeq ($(BHAB_LEAP_UART_ENABLE),y)
BHAB_DEFINES += BHAB_LEAP_UART_ENABLE
endif
BHAB_DEFINES += BHAB_CHIP=75525
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/$(BCHP_CHIP)/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapctfe/
ifeq ($(BHAB_IMG_ONLY),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/$(BCHP_CHIP)/bhab_75525.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/bhab_leap_priv.c
endif
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/$(BCHP_CHIP)/bhab_75525_fw_a0.c
endif

ifneq ($(BHAB_3117_SUPPORT),)
ifeq ($(BHAB_IMG_ONLY),)
BHAB_31xx_CHIP := 3117
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/8051/3117/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/8051/
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/8051/3117/bhab_3117.c
endif
endif

ifneq ($(BHAB_3114_SUPPORT),)
ifeq ($(BHAB_IMG_ONLY),)
BHAB_31xx_CHIP := 3114
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/8051/3114/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/8051/
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/8051/3114/bhab_3114.c
endif
endif

ifneq ($(BHAB_3112_SUPPORT),)
ifeq ($(BHAB_IMG_ONLY),)
BHAB_31xx_CHIP := 3112
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/8051/3112/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/8051/
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/8051/3112/bhab_3112.c
endif
endif

ifneq ($(BHAB_3109_SUPPORT),)
ifeq ($(BHAB_IMG_ONLY),)
BHAB_31xx_CHIP := 3109
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/8051/3109/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/8051/
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/8051/3109/bhab_3109.c
endif
endif

ifneq ($(filter y,$(BHAB_7364_SUPPORT) $(BHAB_3128_SUPPORT) $(BHAB_3158_SUPPORT) $(BHAB_7584_SUPPORT) $(BHAB_3461_SUPPORT) $(BHAB_7563_SUPPORT)),)
ifeq ($(BHAB_IMG_EXTERNAL),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/common/bhab_ctfe_img.c
endif
endif

ifneq ($(BHAB_4528_SUPPORT),)
ifeq ($(BHAB_IMG_ONLY),)
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/4528/
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/4528/rdb/hab/$(BHAB_4528_VER_LOWER)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/4528/bhab_4528.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/4528/bhab_4528_priv.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/4528/bhab_4528_fw_$(BHAB_4528_VER_LOWER).c
endif
endif

ifneq ($(BHAB_4517_SUPPORT),)
ifeq ($(BHAB_IMG_ONLY),)
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/4517/
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/4517/rdb/hab/$(BHAB_4517_VER_LOWER)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/4517/bhab_4517.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/4517/bhab_4517_priv.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/4517/bhab_4517_fw_$(BHAB_4517_VER_LOWER).c
endif
endif

ifneq ($(filter y,$(BHAB_4538_SUPPORT) $(BHAB_7366_SUPPORT) $(BHAB_45216_SUPPORT) $(BHAB_45308_SUPPORT)),)
ifeq ($(BHAB_IMG_EXTERNAL),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/common/bhab_satfe_img.c
endif
endif

ifneq ($(BHAB_4538_SUPPORT),)
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/4538/
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/4538/rdb/hab/$(BHAB_4538_VER_LOWER)
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapsatfe/common/
ifeq ($(BHAB_IMG_ONLY),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/4538/bhab_4538.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/4538/bhab_4538_priv.c
endif # BHAB_IMG_ONLY
ifeq ($(BHAB_IMG_EXTERNAL),)
ifeq ($(BHAB_4538_FIRMWARE),SWM)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/4538/bhab_4538_swm_fw_$(BHAB_4538_VER_LOWER).c
else
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/4538/bhab_4538_fw_$(BHAB_4538_VER_LOWER).c
endif
endif # BHAB_IMG_EXTERNAL
endif

ifneq ($(BHAB_7366_SUPPORT),)
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/7366/
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/7366/rdb/hab/$(BHAB_7366_VER_LOWER)
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapsatfe/common/
ifeq ($(BHAB_IMG_ONLY),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/7366/bhab_7366.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/7366/bhab_7366_priv.c
endif # BHAB_IMG_ONLY
ifeq ($(BHAB_IMG_EXTERNAL),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/7366/bhab_7366_fw_$(BHAB_7366_VER_LOWER).c
endif # BHAB_IMG_EXTERNAL
endif # BHAB_7366_SUPPORT

ifneq ($(BHAB_4548_SUPPORT),)
BHAB_DEFINES += BHAB_CHIP=4548
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/4548/rdb/a0
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/7366/
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/7366/bhab_7366.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/7366/bhab_7366_priv.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/7366/bhab_7366_fw_$(BHAB_7366_VER_LOWER).c
endif

ifneq ($(BHAB_45216_SUPPORT),)
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/45216/rdb/hab/$(BHAB_45216_VER_LOWER)
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/45216/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapsatfe/common/
ifeq ($(BHAB_IMG_ONLY),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/45216/bhab_45216.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/45216/bhab_45216_priv.c
endif # BHAB_IMG_ONLY
ifeq ($(BHAB_IMG_EXTERNAL),)
ifeq ($(BHAB_45216_FIRMWARE),echo)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/45216/bhab_45216_fw_$(BHAB_45216_VER_LOWER)_echo.c
else
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/45216/bhab_45216_fw_$(BHAB_45216_VER_LOWER).c
endif
endif
endif

ifneq ($(BHAB_45308_SUPPORT),)
BHAB_DEFINES += BHAB_45308_VER=BCHP_VER_$(BCHP_45308_VER)
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/45308/rdb/hab/$(BHAB_45308_VER_LOWER)
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/include/45308/
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/leapsatfe/common/
ifeq ($(BHAB_IMG_ONLY),)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/45308/bhab_45308.c
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/45308/bhab_45308_priv.c
endif # BHAB_IMG_ONLY
ifeq ($(BHAB_IMG_EXTERNAL),)
ifeq ($(BHAB_45308_FIRMWARE),echo)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/45308/bhab_45308_fw_$(BHAB_45308_VER_LOWER)_echo.c
else
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapsatfe/45308/bhab_45308_fw_$(BHAB_45308_VER_LOWER).c
endif
endif
endif

# Module Sources
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/common/bhab.c
ifneq ($(BHAB_31xx_CHIP),)
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/$(BHAB_31xx_CHIP)/common
BHAB_INCLUDES += $(MAGNUM)/basemodules/hab/src/8051/
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/8051/bhab_31xx_priv.c
ifeq ($(BCHP_31XX_VER), B0)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/8051/bhab_31xx_fw_b0.c
else
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/8051/bhab_31xx_fw_a0.c
endif
endif

ifneq ($(BHAB_312X_CHIP),)
ifeq ($(BHAB_IMG_ONLY),)
BHAB_INCLUDES += $(MAGNUM)/basemodules/chp/include/$(BHAB_RDB_CHIP)/rdb/hab/$(BHAB_VER_LOWER)
BHAB_SOURCES += $(MAGNUM)/basemodules/hab/src/leapctfe/$(BHAB_312X_CHIP)/bhab_312x_priv.c
endif
endif
