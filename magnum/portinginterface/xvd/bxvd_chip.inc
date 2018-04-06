############################################################
# Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

ifeq ($(findstring $(BCHP_CHIP), 7260), $(BCHP_CHIP))
ifeq ($(findstring $(BCHP_VER), A0), $(BCHP_VER))
BXVD_CHIP = revn0
BXVD_DEFINES += BXVD_P_AVD_ARC600
else
BXVD_CHIP = revt0
BXVD_DEFINES += BXVD_P_AVD_ARC600
endif
endif


ifeq ($(findstring $(BCHP_CHIP), 7278 7255 7211), $(BCHP_CHIP))
BXVD_CHIP = revt0
BXVD_DEFINES += BXVD_P_AVD_ARC600
endif
endif
