############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
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
# $brcm_Log: $
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

ifeq ($(BCHP_CHIP),7420)
ifeq ($(BPWR_TESTPORT), y)
    BCHP_DEFINES += BCHP_PWR_TPORT
endif
endif
