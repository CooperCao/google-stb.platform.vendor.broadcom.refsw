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
# Created: 01/09/2007 by Jon Garrett
#
# $brcm_Log: $
#
############################################################

#
# Select XVD CHIP family
#
BXVD_CHIP = $(BCHP_CHIP)

ifeq ($(findstring $(BCHP_CHIP), 7405 7325 7335 3548 3556 7336 7340 7342 7125 7468 7408), $(BCHP_CHIP))
BXVD_CHIP = 7405
endif

ifeq ($(findstring $(BCHP_CHIP), 7601 7635), $(BCHP_CHIP))
BXVD_CHIP = 7601
endif

ifeq ($(findstring $(BCHP_CHIP), 35230 7550 7630 35125), $(BCHP_CHIP))
BXVD_CHIP = revj0
endif

ifeq ($(findstring $(BCHP_CHIP), 35233 7135 7228 7231 7358 7344 7346 7360 7362 7422 7425 7435 7429 7543 7552 7563 7584 7640), $(BCHP_CHIP))
BXVD_CHIP = revk0
BXVD_DEFINES += BXVD_P_AVD_ARC600

#
# Need to use else clause since chip names are too similar
#
else

ifeq ($(findstring $(BCHP_CHIP), 7145 7250 7268 7271 7445 7364 73625 7366 74295 7439 74371 75525 75635 75845 7586 73465), $(BCHP_CHIP))
BXVD_CHIP = revn0
BXVD_DEFINES += BXVD_P_AVD_ARC600
endif
endif
