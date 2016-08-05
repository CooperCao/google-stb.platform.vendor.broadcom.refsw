#############################################################################
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
#############################################################################

#############################################################################
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
#############################################################################


#
# Conditional Options for this Makefile
#

# Define this module
MAGNUM_MODULES += BXPT
BXPT_DEFINES += UNIFIED_ITB_SUPPORT
BCHP_VER_LC = $(shell echo $(BCHP_VER) | tr A-Z a-z)

ifeq ($(findstring $(BCHP_CHIP), 7366 7145 74371 7439 7445 7364 7250 7271 7268 7260), $(BCHP_CHIP))
        BXPT_DEFINES += BXPT_IS_CORE28NM

        ifeq ($(findstring $(BCHP_CHIP), 7364 7250 7271 7268 7260), $(BCHP_CHIP))
            RAVE_IMAGE_DIR = cat5b
        else # all others
            RAVE_IMAGE_DIR = cat5a
        endif

        ifeq ($(BCHP_CHIP), 7445)
            RAVE_IMAGE_DIR = cat5b
        endif

        ifeq ($(findstring $(BCHP_CHIP), 7366 7145 7439), $(BCHP_CHIP))
            ifeq ($(BCHP_VER_LC), a0)
                RAVE_IMAGE_DIR = cat5a
            else
                RAVE_IMAGE_DIR = cat5b
            endif
        endif

        BXPT_INCLUDES += \
                $(MAGNUM)/portinginterface/xpt/src/common/ \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/

        BXPT_SOURCES += \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/bxpt.c \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/bxpt_directv_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/bxpt_mpod.c \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/bxpt_packetsub.c \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/bxpt_pcr_offset_emv.c \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/bxpt_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/bxpt_tsmux.c \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/bxpt_rave.c \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/bxpt_remux.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_directv.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_directv_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_directv_pcroffset.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_sam.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_spid.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_interrupt.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_directv_remux.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_remux_priv.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_mesg.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_rsbuf_priv.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_xcbuf_priv.c \
                $(MAGNUM)/portinginterface/xpt/src/rave/$(RAVE_IMAGE_DIR)/bxpt_rave_ihex.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_wakeup.c \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/bxpt_pwr_mgmt.c \
                $(MAGNUM)/portinginterface/xpt/src/core28nm/bxpt_dma.c

                RDB_DIR = $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LC)
                ifneq ($(wildcard $(RDB_DIR)/bchp_xpt_wdma_ch0.h),)
                    BXPT_DEFINES += BXPT_DMA_HAS_WDMA_CHX
                endif

                ifneq ($(wildcard $(RDB_DIR)/bchp_xpt_memdma_mcpb.h),)
                    BXPT_DEFINES += BXPT_DMA_HAS_MEMDMA_MCPB
                endif
endif

ifeq ($(findstring $(BCHP_CHIP), 7425 7422 7358 7552 7543 7344 7640 7360 7362 73625 7228 7435 7231 7346 7429 7584 7563 75635 75845 74295 73465 75525), $(BCHP_CHIP))
        ifeq ($(findstring $(BCHP_CHIP), 7422 7358 7552 7543 7344 7360 7362 73625 7228 7563 75635 75525), $(BCHP_CHIP))
                RAVE_IMAGE_DIR = cat3a
        endif
        ifeq ($(findstring $(BCHP_CHIP), 7640), $(BCHP_CHIP))
                RAVE_IMAGE_DIR = cat3c
        endif
        ifeq ($(findstring $(BCHP_CHIP), 7425 7435 7231 7346 7429 7584 75845 74295 73465), $(BCHP_CHIP))
                RAVE_IMAGE_DIR = cat4a
        endif

        BXPT_DEFINES += BXPT_IS_CORE40NM
        BXPT_INCLUDES += \
                $(MAGNUM)/portinginterface/xpt/src/common/ \
                $(MAGNUM)/portinginterface/xpt/src/core40nm/

        BXPT_SOURCES += \
                $(MAGNUM)/portinginterface/xpt/src/core40nm/bxpt.c \
                $(MAGNUM)/portinginterface/xpt/src/core40nm/bxpt_directv_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core40nm/bxpt_mpod.c \
                $(MAGNUM)/portinginterface/xpt/src/core40nm/bxpt_packetsub.c \
                $(MAGNUM)/portinginterface/xpt/src/core40nm/bxpt_pcr_offset_emv.c \
                $(MAGNUM)/portinginterface/xpt/src/core40nm/bxpt_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core40nm/bxpt_rave.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_directv.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_directv_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_directv_pcroffset.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_sam.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_spid.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_interrupt.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_directv_remux.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_remux.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_remux_priv.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_mesg.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_rsbuf_priv.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_xcbuf_priv.c \
                $(MAGNUM)/portinginterface/xpt/src/rave/$(RAVE_IMAGE_DIR)/bxpt_rave_ihex.c \
                $(MAGNUM)/portinginterface/xpt/src/common/bxpt_wakeup.c \
                $(MAGNUM)/portinginterface/xpt/src/core40nm/bxpt_pwr_mgmt.c

        ifeq ($(findstring $(BCHP_CHIP), 7543), $(BCHP_CHIP))
                BXPT_7543_EXCLUDE += \
                        $(MAGNUM)/portinginterface/xpt/src/core40nm/bxpt_mpod.c \
                        $(MAGNUM)/portinginterface/xpt/src/common/bxpt_sam.c \
                        $(MAGNUM)/portinginterface/xpt/src/common/bxpt_interrupt.c \
                        $(MAGNUM)/portinginterface/xpt/src/common/bxpt_directv_remux.c \
                        $(MAGNUM)/portinginterface/xpt/src/common/bxpt_remux.c \
                        $(MAGNUM)/portinginterface/xpt/src/common/bxpt_remux_priv.c \
                        $(MAGNUM)/portinginterface/xpt/src/common/bxpt_mesg.c

                BXPT_SOURCES := $(filter-out $(BXPT_7543_EXCLUDE), $(BXPT_SOURCES))
        endif

                ifeq ($(findstring $(BCHP_CHIP), 7231 7346 7425 7429 7435 7584 75845 74295 73465), $(BCHP_CHIP))
                BXPT_SOURCES += \
                        $(MAGNUM)/portinginterface/xpt/src/core40nm/bxpt_tsmux.c
                endif

                EMMDIR = $(MAGNUM)/portinginterface/xpt/src/common/gpg/
                EMMSRC = $(EMMDIR)/bxpt_rave_emm.c
                ifneq (,$(findstring $(EMMSRC),$(wildcard $(EMMSRC) )))
                        BXPT_SOURCES += $(MAGNUM)/portinginterface/xpt/src/common/gpg/bxpt_rave_emm.c
                        BXPT_INCLUDES += $(EMMDIR)
                endif
endif

ifeq ($(findstring $(BCHP_CHIP), 7468), $(BCHP_CHIP))
        ifeq ($(findstring $(BCHP_CHIP), 7468), $(BCHP_CHIP))
                RAVE_IMAGE_DIR = cat2a
        endif

        BXPT_DEFINES += BXPT_IS_CORE65NM
        BXPT_INCLUDES += $(MAGNUM)/portinginterface/xpt/src/core65nm/
        BXPT_SOURCES += \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_sam.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_spid.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_packetsub.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_rave.c \
                $(MAGNUM)/portinginterface/xpt/src/rave/$(RAVE_IMAGE_DIR)/bxpt_rave_ihex.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_interrupt.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_mesg.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_pcroffset.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_pcr_offset_emv.c
endif

ifeq ($(findstring $(BCHP_CHIP), 7405 7336 7325 7335 7420 7340 7342 7125), $(BCHP_CHIP))
        ifeq ($(findstring $(BCHP_CHIP), 7125 7420 7340 7342), $(BCHP_CHIP))
                RAVE_IMAGE_DIR = cat2a
        endif
        ifeq ($(findstring $(BCHP_CHIP), 7405 7336), $(BCHP_CHIP))
                RAVE_IMAGE_DIR = cat1b
        endif
        ifeq ($(findstring $(BCHP_CHIP), 7325 7335), $(BCHP_CHIP))
                RAVE_IMAGE_DIR = cat1a
        endif

        BXPT_DEFINES += BXPT_IS_CORE65NM
        BXPT_INCLUDES += $(MAGNUM)/portinginterface/xpt/src/core65nm/
        BXPT_SOURCES += \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_pcroffset.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_sam.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_spid.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_packetsub.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_interrupt.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_remux.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_remux.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_remux_private.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_mpod.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_mesg.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_rsbuf.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_xcbuf.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_rave.c \
                $(MAGNUM)/portinginterface/xpt/src/rave/$(RAVE_IMAGE_DIR)/bxpt_rave_ihex.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_pcr_offset_emv.c

                PSGDIR = $(MAGNUM)/portinginterface/xpt/src/core65nm/
                PSGSRC = $(PSGDIR)/bxpt_psg.c
                ifneq (,$(findstring $(PSGSRC),$(wildcard $(PSGSRC) )))
                        BXPT_SOURCES += $(PSGDIR)/bxpt_psg.c
                endif
endif

ifeq ($(findstring $(BCHP_CHIP), 7400), $(BCHP_CHIP))
        RAVE_IMAGE_DIR = cat7400e

        BXPT_DEFINES += BXPT_IS_CORE65NM
        BXPT_INCLUDES += $(MAGNUM)/portinginterface/xpt/src/core65nm/
        BXPT_SOURCES += \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_sam.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_spid.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_packetsub.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_interrupt.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_remux.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_remux.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_remux_private.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_mpod.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_mesg.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_rsbuf.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_xcbuf.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_rave.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_pcroffset.c \
                $(MAGNUM)/portinginterface/xpt/src/rave/$(RAVE_IMAGE_DIR)/bxpt_rave_ihex.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_pcr_offset_emv.c
endif

ifeq ($(findstring $(BCHP_CHIP), 7408), $(BCHP_CHIP))
        RAVE_IMAGE_DIR = cat7408b

        BXPT_DEFINES += BXPT_IS_CORE65NM
        BXPT_INCLUDES += $(MAGNUM)/portinginterface/xpt/src/core65nm/
        BXPT_SOURCES += \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_directv_pcroffset.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_spid.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_rave.c \
                $(MAGNUM)/portinginterface/xpt/src/rave/$(RAVE_IMAGE_DIR)/bxpt_rave_ihex.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_pcr_offset_emv.c
endif

ifeq ($(findstring $(BCHP_CHIP), 7550), $(BCHP_CHIP))
        RAVE_IMAGE_DIR = cat2a

        BXPT_DEFINES += BXPT_IS_CORE65NM
        BXPT_INCLUDES += $(MAGNUM)/portinginterface/xpt/src/core65nm/
        BXPT_SOURCES += \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_spid.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_packetsub.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_playback.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_pcr.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_remux.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_remux_private.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_mpod.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_rave.c \
                $(MAGNUM)/portinginterface/xpt/src/rave/$(RAVE_IMAGE_DIR)/bxpt_rave_ihex.c \
                $(MAGNUM)/portinginterface/xpt/src/core65nm/bxpt_pcr_offset_emv.c
endif

# Module Includes
BXPT_INCLUDES += $(MAGNUM)/portinginterface/xpt/include
BXPT_INCLUDES += $(MAGNUM)/portinginterface/xpt/src/rave/$(RAVE_IMAGE_DIR)

# Other options
ifneq ($(BXPT_POWER_MANAGEMENT),n)
BXPT_DEFINES += BXPT_POWER_MANAGEMENT
endif
