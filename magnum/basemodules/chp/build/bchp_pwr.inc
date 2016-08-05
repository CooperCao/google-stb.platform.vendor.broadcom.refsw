############################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

# BCHP_PWR basemodule.

# the source files need to get included in the library unconditionally
BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/common/bchp_pwr.c

ifeq ($(shell test -e $(MAGNUM)/basemodules/chp/src/$(BCHP_CHIP)/pwr && echo y),y)

BPWR_CHIP_VER_LOWER := $(BCHP_VER_LOWER)

ifeq ($(BCHP_CHIP), 7231)
     ifeq ($(filter $(BCHP_VER_LOWER), b0 b1 b2), $(BCHP_VER_LOWER))
         BPWR_CHIP_VER_LOWER := b0
     else
         BPWR_CHIP_VER_LOWER := a0
     endif
endif
ifeq ($(BCHP_CHIP), 7425)
    ifeq ($(filter $(BCHP_VER_LOWER), b0 b1 b2), $(BCHP_VER_LOWER))
	BPWR_CHIP_VER_LOWER := b0
    else
	BPWR_CHIP_VER_LOWER := a1
    endif
endif
ifeq ($(BCHP_CHIP), 7429)
    ifeq ($(filter $(BCHP_VER_LOWER), a0 b0), $(BCHP_VER_LOWER))
	BPWR_CHIP_VER_LOWER := a0
    endif
endif
ifeq ($(BCHP_CHIP), 7344)
    ifeq ($(filter $(BCHP_VER_LOWER), b0 b1), $(BCHP_VER_LOWER))
	BPWR_CHIP_VER_LOWER := b0
    else
	BPWR_CHIP_VER_LOWER := a0
    endif
endif
ifeq ($(BCHP_CHIP), 7346)
    ifeq ($(filter $(BCHP_VER_LOWER), b0 b1 b2), $(BCHP_VER_LOWER))
	BPWR_CHIP_VER_LOWER := b0
    else
	BPWR_CHIP_VER_LOWER := a0
    endif
endif
ifeq ($(BCHP_CHIP), 7364)
    ifeq ($(filter $(BCHP_VER_LOWER), a0 b0 c0), $(BCHP_VER_LOWER))
	BPWR_CHIP_VER_LOWER := a0
    endif
endif
ifeq ($(BCHP_CHIP), 7366)
    ifeq ($(filter $(BCHP_VER_LOWER), b0 c0), $(BCHP_VER_LOWER))
	BPWR_CHIP_VER_LOWER := b0
else
    BPWR_CHIP_VER_LOWER := a0
endif
endif
ifeq ($(BCHP_CHIP), 7445)
    ifeq ($(filter $(BCHP_VER_LOWER), a0 b0), $(BCHP_VER_LOWER))
	BPWR_CHIP_VER_LOWER := a0
    endif
    ifeq ($(filter $(BCHP_VER_LOWER), c0), $(BCHP_VER_LOWER))
	BPWR_CHIP_VER_LOWER := c0
    endif
    ifeq ($(filter $(BCHP_VER_LOWER), e0), $(BCHP_VER_LOWER))
	BPWR_CHIP_VER_LOWER := d0
    endif
endif
ifeq ($(BCHP_CHIP), 7584)
    BPWR_CHIP_VER_LOWER := a0
endif

ifeq ($(shell test -e $(MAGNUM)/basemodules/chp/src/$(BCHP_CHIP)/pwr/$(BPWR_CHIP_VER_LOWER)/bchp_pwr_resources.c && echo y),y)
     ifeq ($(BCHP_PWR_SUPPORT),y)
         BCHP_DEFINES += BCHP_PWR_SUPPORT
     endif

     BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/common/pwr/$(BPWR_CHIP_VER_LOWER)
     BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/src/$(BCHP_CHIP)/pwr/$(BPWR_CHIP_VER_LOWER)
     BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/$(BCHP_CHIP)/pwr/$(BPWR_CHIP_VER_LOWER)/bchp_pwr_resources.c
     BCHP_DEFINES += BCHP_PWR_HAS_RESOURCES
endif

else

ifeq ($(shell test -e $(MAGNUM)/basemodules/chp/src/$(BCHP_CHIP)/bchp_pwr_resources.c && echo y),y)
     ifeq ($(BCHP_PWR_SUPPORT),y)
        BCHP_DEFINES += BCHP_PWR_SUPPORT
     endif

     BCHP_INCLUDES += $(MAGNUM)/basemodules/chp/src/$(BCHP_CHIP)
     BCHP_SOURCES += $(MAGNUM)/basemodules/chp/src/$(BCHP_CHIP)/bchp_pwr_resources.c
     BCHP_DEFINES += BCHP_PWR_HAS_RESOURCES
endif

endif
