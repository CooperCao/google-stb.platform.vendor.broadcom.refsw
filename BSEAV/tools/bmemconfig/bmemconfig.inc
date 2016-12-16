############################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
# This program is the proprietary software of Broadcom and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
############################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# If boxmode_xxxx.inc exists, use the list of source files specified in that file
ifneq ("$(wildcard $(NEXUS_TOP)/../magnum/commonutils/box/src/$(BCHP_CHIP)/bbox_$(BCHP_CHIP).inc)","")  ### if file exists
-include $(NEXUS_TOP)/../magnum/commonutils/box/src/$(BCHP_CHIP)/bbox_$(BCHP_CHIP).inc
else  ### file does not exist
endif

BCHP_VER_LOWER=$(shell echo $(BCHP_VER) | tr A-Z a-z)

# Create a list of bbox files that end in config.c (list will contain both memc and vdc files.)
BBOX_SOURCES2=$(filter %config.c, $(BBOX_SOURCES))

# One of Make's greatest shortcomings is its poor ability to handle regular expressions.
# The functions filter and filter-out can't find "something" in the middle of a word.
# Use shell command with sed to create a list of files that DO NOT contain bbox_memc.
UNWANTED_FILES = $(shell echo $(BBOX_SOURCES2) | sed 's/[^ ]*bbox_memc[^ ]* *//g')

# Now filter out the unwanted files ... leaving just the wanted files ... bbox_memc
BBOX_MEMC_FILES1 = $(filter-out $(UNWANTED_FILES), $(BBOX_SOURCES2))

ifeq ($(BCHP_VER_LOWER_OVERRIDE),$(BCHP_VER_LOWER))
BBOX_MEMC_FILES2 = $(shell echo ${BBOX_MEMC_FILES1} | grep -E -o '[^[:space:]]+/$(BCHP_VER_LOWER)/[^[:space:]]+')
BBOX_MEMC_FILES = $(BBOX_MEMC_FILES2)
else  ### versions are different; use all of the files specified in the bbox_xxxx.inc file
BBOX_MEMC_FILES = $(BBOX_MEMC_FILES1)
endif

ifeq ($(filter $(BCHP_CHIP),7445), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),E0), $(BCHP_VER))
BCHP_VER=D0
BCHP_VER_LOWER=$(shell echo $(BCHP_VER) | tr A-Z a-z)
endif
endif

ifeq ($(filter $(BCHP_CHIP),7364), $(BCHP_CHIP))
ifeq ($(filter $(BCHP_VER),C0), $(BCHP_VER))
BCHP_VER=B0
BCHP_VER_LOWER=$(shell echo $(BCHP_VER) | tr A-Z a-z)
endif
endif


#
# End of file
#
