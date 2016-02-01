############################################################
#     Copyright (c) 2004-2014, Broadcom Corporation
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
# Module Description: List of support platforms
#
# Revision History:
#
# Created: 02/09/2001 by Marcus Kellerman
#
# $brcm_Log: $
# 
############################################################

#
# ATTENTION: If you add anything to the following PLATFORMS list, you must support it
# throughout the entire refsw tree. Do not modify it and check in without doing the following:
#
#   Edit BSEAV/api/include/api.mak and select the appropriate BCHP_CHIP for your PLATFORM
#
#	Look at where BCHP_CHIP is used in all .h and .c files. Modify accordingly.
#	Look at where BCM_BOARD is used in all .h and .c files. Modify accordingly.
#
#   In general, you're done only when brutus builds and runs for your new platform.
#

# Magnum-based platforms
MAGNUM_PLATFORMS = \
	93560 \
	97038 97398 \
	97401 97403 97325 97335 97342 97340 97405 97466 97205 97400 97455 97458 97456 97459 97435 97445 97252 97366 \
	97018 97018RNG 97118 97118RNG 97420 93380vms 97410 97409 \
	97125 97025 97119 97019 97116 97550 97468 97208 97408 97422 97425 97358 97552 97231 97230 97418 97344 97346 \
        97429 97428 97241 97360 97584 97563


# Legacy (non-magnum) platforms
LEGACY_PLATFORMS = \
	97110 97111 97112 97115 \
	97312 97313 97314 97315 97317 97318 97318AVC \
	97319 97320 97328

NEXUS_PLATFORMS = \
	93563 97325 97335 97342 97340 93549 93556 97466 97205 97405 97468 97208 97550 97408 97418 97366

# Linux 2.4 platforms
LINUX_2_4_PLATFORMS = \
	97038 97398 93560 \
	$(LEGACY_PLATFORMS)

# As support is added in BSEAV/linux/driver/build for your board, add it here.
# A platform can exist in both the 2.4 and 2.6 platform list (not true for magnum vs. legacy)

# IMPORTANT - Only add platforms here that have actual defconfig files.  If you do not, the linux include build scripts will fail.
LINUX_2_6_PLATFORMS = \
	97400 97405 97466 97205\
	97401 97403 \
	97118 \
	97455 97456 97458 97459 \
	97325 97335 97342 97340 \
	97420 97410 97409 \
	97125 97025 97119 97019 97116 97468 97208 97550 97408 97422 97425 97358 97360 97552 97231 97230 97346 97344 97418 \
        97429 97428 97241 97435 97584 97563

# If your platform supports SMP, add it here.
LINUX_2_6_SMP_PLATFORMS = \
	97400 97405 97466 97205 97325 97335 97342 97340 93549 93556 97420 97410 97409 \
	97125 97025 97119 97019 97116 97422 97425 97358 97360 97552 97231 97230 97418 97344 97346\
        97429 97428 97241 97435 97584 97563

# Magnum and legacy are mutually exclusive and define the set of all platforms
PLATFORMS = \
	$(MAGNUM_PLATFORMS) \
	$(NEXUS_PLATFORMS) \
	$(LEGACY_PLATFORMS)

