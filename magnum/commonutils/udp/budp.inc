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
# $brcm_Log: $
# 
############################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BUDP

# Module Includes
BUDP_INCLUDES += \
	$(MAGNUM)/commonutils/udp/src \
	$(MAGNUM)/commonutils/udp/include

BUDP_SOURCES += \
	$(MAGNUM)/commonutils/udp/src/budp_dccparse.c \
	$(MAGNUM)/commonutils/udp/src/budp_scteparse.c \
	$(MAGNUM)/commonutils/udp/src/budp_jp3dparse.c \
	$(MAGNUM)/commonutils/udp/src/budp_bitread.c

# By default, customer-specific software will not be compiled.
ifeq ($(B_REFSW_DSS_SUPPORT), y)
	BUDP_SOURCES += $(MAGNUM)/commonutils/udp/src/budp_dccparse_dss.c
endif
ifeq ($(B_REFSW_DIVICOM_SUPPORT), y)
	BUDP_SOURCES += $(MAGNUM)/commonutils/udp/src/budp_dccparse_divicom.c
endif
