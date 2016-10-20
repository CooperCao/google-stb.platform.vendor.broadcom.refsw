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
# Module Description:
#   This file defines the rules to build the nexus platforms
#
############################################################

# backward compatibility
NEXUS_PLATFORM ?= $(PLATFORM)

# Sanity check that platform is defined
ifeq ($(NEXUS_PLATFORM),)
$(error NEXUS_PLATFORM is not defined!)
endif

# derive BCHP_CHIP from NEXUS_PLATFORM if not explicitly defined
ifndef BCHP_CHIP
    # Define platform specifics
    ifeq ($(NEXUS_PLATFORM),97346)
        BCHP_CHIP := 7346
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=B0)
        BCHP_VER := B0
        endif
    endif
    ifeq ($(NEXUS_PLATFORM),973465)
        BCHP_CHIP := 73465
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif
    ifeq ($(filter $(NEXUS_PLATFORM),97344 97418), $(NEXUS_PLATFORM))
        BCHP_CHIP := 7344
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=B1)
        BCHP_VER := B1
        endif
    endif
    ifeq ($(NEXUS_PLATFORM),97630)
        BCHP_CHIP := 7630
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=B0)
        BCHP_VER := B0
        endif
    endif
    ifeq ($(NEXUS_PLATFORM),97435)
        BCHP_CHIP := 7435
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif
    ifeq ($(filter $(NEXUS_PLATFORM),97425), $(NEXUS_PLATFORM))
        BCHP_CHIP := 7425
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif
    ifeq ($(filter $(NEXUS_PLATFORM),97422 93322), $(NEXUS_PLATFORM))
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
        ifeq ($(BCHP_VER),A0)
            BCHP_CHIP := 7422
        else
            BCHP_CHIP := 7425
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97231)
        BCHP_CHIP := 7231
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := B0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97230)
        BCHP_CHIP := 7231
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := B0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97358)
        BCHP_CHIP := 7358
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97552)
        BCHP_CHIP := 7552
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),975525)
        BCHP_CHIP := 75525
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97429)
        BCHP_CHIP := 7429
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97428)
        BCHP_CHIP := 7429
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97241)
        BCHP_CHIP := 7429
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif
    ifeq ($(NEXUS_PLATFORM),974295)
        BCHP_CHIP := 74295
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),974285)
        BCHP_CHIP := 74295
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),972415)
        BCHP_CHIP := 74295
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97360)
        BCHP_CHIP := 7360
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97362)
        BCHP_CHIP := 7362
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),973625)
        BCHP_CHIP := 73625
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97228)
        BCHP_CHIP := 7228
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97584)
        BCHP_CHIP := 7584
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),975845)
        BCHP_CHIP := 75845
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97445)
        BCHP_CHIP := 7445
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=D0)
        BCHP_VER := D0
        endif
    ifeq ($(BCHP_VER),D1)
    BCHP_VER := D0
        $(warning Building with BCHP_VER=D1 is same as that with BCHP_VER=D0 )
    endif
    endif

    ifeq ($(NEXUS_PLATFORM),97252)
        BCHP_CHIP := 7445
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=D0)
        BCHP_VER := D0
        endif
    ifeq ($(BCHP_VER),D1)
        BCHP_VER := D0
        $(warning Building with BCHP_VER=D1 is same as that with BCHP_VER=D0 )
    endif
    endif

    ifeq ($(NEXUS_PLATFORM),97449)
        BCHP_CHIP := 7445
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=D0)
        BCHP_VER := D0
        endif
    ifeq ($(BCHP_VER),D1)
        BCHP_VER := D0
        $(warning Building with BCHP_VER=D1 is same as that with BCHP_VER=D0 )
    endif
    endif

    ifeq ($(NEXUS_PLATFORM),97563)
        BCHP_CHIP := 7563
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),975635)
        BCHP_CHIP := 75635
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97364)
        BCHP_CHIP := 7364
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97586)
        BCHP_CHIP := 7586
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97250)
        BCHP_CHIP := 7250
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=B0)
        BCHP_VER := B0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97260)
        BCHP_CHIP := 7260
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97268)
        BCHP_CHIP := 7268
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97271)
        BCHP_CHIP := 7271
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97278)
        BCHP_CHIP := 7278
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97586)
        BCHP_CHIP := 7586
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97366)
        BCHP_CHIP := 7366
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),97439)
        BCHP_CHIP := 7439
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),974371)
        BCHP_CHIP := 74371
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=A0)
        BCHP_VER := A0
        endif
    endif

    ifeq ($(NEXUS_PLATFORM),911360)
        BCHP_CHIP := 11360
        ifndef BCHP_VER
        $(warning Defaulting to BCHP_VER=B0)
        BCHP_VER := B0
        endif
    ifeq ($(BCHP_VER),B2)
    BCHP_VER := B0
        $(warning Building with BCHP_VER=B2 is same as that with BCHP_VER=B0 )
    endif
    endif

    # Sanity check that we received a valid platform
    ifndef BCHP_CHIP
    $(error Unsupported platform $(NEXUS_PLATFORM))
    endif
endif
