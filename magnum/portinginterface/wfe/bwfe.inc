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

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BWFE

# Add dependency on BFEC
BWFE_INCLUDES += \
   $(MAGNUM)/commonutils/fec/include \
	$(MAGNUM)/portinginterface/wfe/include/common \
	$(MAGNUM)/portinginterface/wfe/src/common

BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/common/bwfe.c


##############################
# Handle 4538
##############################
ifeq ($(findstring 4538,$(BWFE_CHIP)),4538)

# If chip version not specified, select a default
ifeq ($(BWFE_4538_VER),)
BWFE_4538_VER := A0
endif

BWFE_4538_VER_UPPER := $(strip $(BWFE_4538_VER))
BWFE_4538_VER_UPPER := $(subst a,A,$(BWFE_4538_VER_UPPER))
BWFE_4538_VER_UPPER := $(subst b,B,$(BWFE_4538_VER_UPPER))
BWFE_4538_VER_UPPER := $(subst c,C,$(BWFE_4538_VER_UPPER))

BWFE_4538_VER_LOWER := $(strip $(BWFE_4538_VER))
BWFE_4538_VER_LOWER := $(subst A,a,$(BWFE_4538_VER_LOWER))
BWFE_4538_VER_LOWER := $(subst B,b,$(BWFE_4538_VER_LOWER))
BWFE_4538_VER_LOWER := $(subst C,c,$(BWFE_4538_VER_LOWER))

# Source needs to know which revision
BWFE_DEFINES += BWFE_4538_VER=BCHP_VER_$(BWFE_4538_VER_UPPER)

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/4538 \
	$(MAGNUM)/portinginterface/wfe/src/4538

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/4538/bwfe_4538.c \
	$(MAGNUM)/portinginterface/wfe/src/4538/bwfe_4538_priv.c

endif


##############################
# Handle 7366
##############################
ifeq ($(findstring 7366,$(BWFE_CHIP)),7366)

# If chip version not specified, select a default
ifeq ($(BWFE_7366_VER),)
BWFE_7366_VER := A0
endif

BWFE_7366_VER_UPPER := $(strip $(BWFE_7366_VER))
BWFE_7366_VER_UPPER := $(subst a,A,$(BWFE_7366_VER_UPPER))
BWFE_7366_VER_UPPER := $(subst b,B,$(BWFE_7366_VER_UPPER))
BWFE_7366_VER_UPPER := $(subst c,C,$(BWFE_7366_VER_UPPER))

BWFE_7366_VER_LOWER := $(strip $(BWFE_7366_VER))
BWFE_7366_VER_LOWER := $(subst A,a,$(BWFE_7366_VER_LOWER))
BWFE_7366_VER_LOWER := $(subst B,b,$(BWFE_7366_VER_LOWER))
BWFE_7366_VER_LOWER := $(subst C,c,$(BWFE_7366_VER_LOWER))

# Source needs to know which revision
BWFE_DEFINES += BWFE_7366_VER=BCHP_VER_$(BWFE_7366_VER_UPPER)

ifeq ($(BYPASS_LEAP),1)

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/g3 \
	$(MAGNUM)/portinginterface/wfe/src/g3/common \
	$(MAGNUM)/portinginterface/wfe/src/g3/7366

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/common/bwfe_priv.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_adc.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_corr.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_lic.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_rffe.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/7366/bwfe_7366_priv.c

else

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/7366 \
	$(MAGNUM)/portinginterface/wfe/src/7366

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/7366/bwfe_7366.c \
	$(MAGNUM)/portinginterface/wfe/src/7366/bwfe_7366_priv.c

endif

endif


##############################
# Handle 45216
##############################
ifeq ($(findstring 45216,$(BWFE_CHIP)),45216)

# If chip version not specified, select a default
ifeq ($(BWFE_45216_VER),)
BWFE_45216_VER := A0
endif

BWFE_45216_VER_UPPER := $(strip $(BWFE_45216_VER))
BWFE_45216_VER_UPPER := $(subst a,A,$(BWFE_45216_VER_UPPER))
BWFE_45216_VER_UPPER := $(subst b,B,$(BWFE_45216_VER_UPPER))
BWFE_45216_VER_UPPER := $(subst c,C,$(BWFE_45216_VER_UPPER))

BWFE_45216_VER_LOWER := $(strip $(BWFE_45216_VER))
BWFE_45216_VER_LOWER := $(subst A,a,$(BWFE_45216_VER_LOWER))
BWFE_45216_VER_LOWER := $(subst B,b,$(BWFE_45216_VER_LOWER))
BWFE_45216_VER_LOWER := $(subst C,c,$(BWFE_45216_VER_LOWER))

# Source needs to know which revision
BWFE_DEFINES += BWFE_45216_VER=BCHP_VER_$(BWFE_45216_VER_UPPER)

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/45216 \
	$(MAGNUM)/portinginterface/wfe/src/45216

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/45216/bwfe_45216.c \
	$(MAGNUM)/portinginterface/wfe/src/45216/bwfe_45216_priv.c

endif


##############################
# Handle 45308
##############################
ifeq ($(findstring 45308,$(BWFE_CHIP)),45308)

# If chip version not specified, select a default
ifeq ($(BWFE_45308_VER),)
BWFE_45308_VER := A0
endif

BWFE_45308_VER_UPPER := $(strip $(BWFE_45308_VER))
BWFE_45308_VER_UPPER := $(subst a,A,$(BWFE_45308_VER_UPPER))
BWFE_45308_VER_UPPER := $(subst b,B,$(BWFE_45308_VER_UPPER))
BWFE_45308_VER_UPPER := $(subst c,C,$(BWFE_45308_VER_UPPER))

BWFE_45308_VER_LOWER := $(strip $(BWFE_45308_VER))
BWFE_45308_VER_LOWER := $(subst A,a,$(BWFE_45308_VER_LOWER))
BWFE_45308_VER_LOWER := $(subst B,b,$(BWFE_45308_VER_LOWER))
BWFE_45308_VER_LOWER := $(subst C,c,$(BWFE_45308_VER_LOWER))

# Source needs to know which revision
BWFE_DEFINES += BWFE_45308_VER=BCHP_VER_$(BWFE_45308_VER_UPPER)

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/45308 \
	$(MAGNUM)/portinginterface/wfe/src/common \
	$(MAGNUM)/portinginterface/wfe/src/45308

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/45308/bwfe_45308.c \
	$(MAGNUM)/portinginterface/wfe/src/45308/bwfe_45308_priv.c

endif



##############################
# Handle 7364
##############################
ifeq ($(findstring 7364,$(BWFE_CHIP)),7364)

# If chip version not specified, select a default
ifeq ($(BWFE_7364_VER),)
BWFE_7364_VER := A0
endif

BWFE_7364_VER_UPPER := $(strip $(BWFE_7364_VER))
BWFE_7364_VER_UPPER := $(subst a,A,$(BWFE_7364_VER_UPPER))
BWFE_7364_VER_UPPER := $(subst b,B,$(BWFE_7364_VER_UPPER))
BWFE_7364_VER_UPPER := $(subst c,C,$(BWFE_7364_VER_UPPER))

BWFE_7364_VER_LOWER := $(strip $(BWFE_7364_VER))
BWFE_7364_VER_LOWER := $(subst A,a,$(BWFE_7364_VER_LOWER))
BWFE_7364_VER_LOWER := $(subst B,b,$(BWFE_7364_VER_LOWER))
BWFE_7364_VER_LOWER := $(subst C,c,$(BWFE_7364_VER_LOWER))

# Source needs to know which revision
BWFE_DEFINES += BWFE_7364_VER=BCHP_VER_$(BWFE_7364_VER_UPPER)

# Module Includes
BWFE_INCLUDES += \
	$(MAGNUM)/portinginterface/wfe/include/g3 \
	$(MAGNUM)/portinginterface/wfe/src/g3/common \
	$(MAGNUM)/portinginterface/wfe/src/g3/7364 \

# Module Sources
BWFE_SOURCES += \
	$(MAGNUM)/portinginterface/wfe/src/common/bwfe_priv.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_adc.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_corr.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_lic.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/common/bwfe_g3_priv_rffe.c \
	$(MAGNUM)/portinginterface/wfe/src/g3/7364/bwfe_7364_priv.c

endif
