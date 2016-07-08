############################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
############################################################

# use bchp_rdb.inc to catch invalid versions with a helpful message, also builds BCHP_VER_LOWER
include $(MAGNUM)/basemodules/chp/build/bchp_rdb.inc

#
# Determine the ZEUS version and CORETECH
#
SECURITY_CORE_LIST_ZEUS_42      := 7445e0 7445d0 7366b0 7145b0 7252d0 7399a0 7439b0 7459c0 7364a0 7250b0 7366c0 7586a0 7364b0 7271a0 7268a0 7364c0
SECURITY_CORE_LIST_ZEUS_41      := 7366a0 7439a0 74371a0
SECURITY_CORE_LIST_ZEUS_30      := 7584a0 7584a1 7435b0 75845a0
SECURITY_CORE_LIST_ZEUS_22      := 7360a0 7360a1 7362a0 7422b1 7422b2 7563a0 7228a0 75635a0 73625a0 75525a0
SECURITY_CORE_LIST_ZEUS_20      := 7231b0 7344b0 7346b0 7552b0 7231b1 7344b1 7346b1 7231b2 7346b2 7425b1 7425b2 7241b0 7429b0 7428b0 74295a0 73465a0
SECURITY_CORE_LIST_ZEUS_10      := 7346a0 7358a0 7358a1
SECURITY_CORE_LIST_65nmASKM     := 7125c0 7125d0 7125e0 7340a0 7340b0 7420c0 7420c1 7468a0 7468b0
SECURITY_CORE_LIST_65nmNonAskm  := 7405b0 7405b1 7405c0


BCHP_CHIP_NAME_VERSION:=$(BCHP_CHIP)$(BCHP_VER_LOWER)

ifeq ($(filter $(BCHP_CHIP_NAME_VERSION), $(SECURITY_CORE_LIST_ZEUS_42) ), $(BCHP_CHIP_NAME_VERSION))
    ZEUSVER  := Zeus42
    CORETECH := core28nm
    BHSM_ZEUS_VER_MAJOR:=4
    BHSM_ZEUS_VER_MINOR:=2
else
    ifeq ($(filter $(BCHP_CHIP_NAME_VERSION), $(SECURITY_CORE_LIST_ZEUS_41) ), $(BCHP_CHIP_NAME_VERSION))
        ZEUSVER  := Zeus41
        CORETECH := core28nm
        BHSM_ZEUS_VER_MAJOR:=4
        BHSM_ZEUS_VER_MINOR:=1
    else
        ifeq ($(filter $(BCHP_CHIP_NAME_VERSION), $(SECURITY_CORE_LIST_ZEUS_30) ), $(BCHP_CHIP_NAME_VERSION))
            ZEUSVER  := Zeus30
            CORETECH := core40nm
            BHSM_ZEUS_VER_MAJOR:=3
            BHSM_ZEUS_VER_MINOR:=0
        else
            ifeq ($(filter $(BCHP_CHIP_NAME_VERSION), $(SECURITY_CORE_LIST_ZEUS_22) ), $(BCHP_CHIP_NAME_VERSION))
                ZEUSVER  := Zeus22
                CORETECH := core40nm
                BHSM_ZEUS_VER_MAJOR:=2
                BHSM_ZEUS_VER_MINOR:=2
            else
                ifeq ($(filter $(BCHP_CHIP_NAME_VERSION), $(SECURITY_CORE_LIST_ZEUS_20) ), $(BCHP_CHIP_NAME_VERSION))
                    ZEUSVER  := Zeus20
                    CORETECH := core40nm
                    BHSM_ZEUS_VER_MAJOR:=2
                    BHSM_ZEUS_VER_MINOR:=0
                else
                    ifeq ($(filter $(BCHP_CHIP_NAME_VERSION), $(SECURITY_CORE_LIST_ZEUS_10) ), $(BCHP_CHIP_NAME_VERSION))
                        ZEUSVER := Zeus10
                        CORETECH := core40nm
                        BHSM_ZEUS_VER_MAJOR:=1
                        BHSM_ZEUS_VER_MINOR:=0
                    else
                        ifeq ($(filter $(BCHP_CHIP_NAME_VERSION), $(SECURITY_CORE_LIST_65nmASKM) ), $(BCHP_CHIP_NAME_VERSION))
                            ZEUSVER := core65nmASKM
                            CORETECH := core65nmASKM
                            BHSM_ZEUS_VER_MAJOR:=0
                            BHSM_ZEUS_VER_MINOR:=2
                        else
                            ifeq ($(filter $(BCHP_CHIP_NAME_VERSION), $(SECURITY_CORE_LIST_65nmNonAskm) ), $(BCHP_CHIP_NAME_VERSION))
                                ZEUSVER := core65nmNonASKM
                                CORETECH := core65nm
                                BHSM_ZEUS_VER_MAJOR:=0
                                BHSM_ZEUS_VER_MINOR:=1
                            else
                                $(error  Can not determine Zeus version.  Add NEXUS_SECURITY_SUPPORT:=n to platform_options.inc for $(BCHP_CHIP) $(BCHP_VER) )
                            endif
                        endif
                    endif
                endif
            endif
        endif
    endif
endif


ifdef BSHM_VERBOSE_BUILD
    $(warning  $(BCHP_CHIP)$(BCHP_VER_LOWER) $(ZEUSVER) $(CORETECH)  )
endif
