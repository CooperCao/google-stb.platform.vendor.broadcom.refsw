############################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
############################################################

NEXUS_TOP ?= $(shell cd ../../../../nexus; pwd)

APPS = \
    dolby_vol \
    dolby_pulse_singledecode \
    dolby_pulse_multistream_audiomixing \
    dolby_digitalplus_singledecode \
    dolby_digitalplus_dualplayback \
    dolby_digitalplus_multistream_audiomixing \
    dolby_ms11_dualplayback \
    dolby_ms12_dualplayback
        

BSEAV = $(NEXUS_TOP)/../BSEAV
include $(BSEAV)/lib/bfile/bfile.inc
ifeq ($(MEDIA_AVI_SUPPORT),y)
include $(BSEAV)/lib/avi/bavi.inc
endif
ifeq ($(MEDIA_ASF_SUPPORT),y)
include $(BSEAV)/lib/asf/basf.inc
endif
ifeq ($(MEDIA_RMFF_SUPPORT),y)
include $(BSEAV)/lib/media/rmff/brmff.inc
endif
include $(BSEAV)/lib/utils/batom.inc
include $(BSEAV)/lib/media/bmedia.inc

NEXUS_UTILS_INCLUDES := ${NEXUS_TOP}/utils/
API_MODULES += BMEDIA BFILE_MEDIA BMEDIA_PROBE NEXUS_UTILS

include ${NEXUS_TOP}/../rockford/unittests/nexus/avenger/lib/ave_lib.inc
MODULES += B_AVE_LIB
CFLAGS +=$(addprefix -I,$(sort $(foreach module, $(MODULES) ${API_MODULES}, $($(module)_INCLUDES))))
CFLAGS += $(addprefix -D,$(sort $(foreach module, $(MODULES) ${API_MODULES}, $($(module)_DEFINES))))
EXTRA_SRC += $(notdir $(sort $(foreach module, $(MODULES), $($(module)_SOURCES))))
vpath %c $(dir $(sort $(foreach module, $(MODULES), $($(module)_SOURCES))))

# For Enabling ring buffer capture #
MAGNUM = $(NEXUS_TOP)/../magnum
CFLAGS += -I$(MAGNUM)/basemodules/chp/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER) -I$(MAGNUM)/basemodules/mem
ifeq ($(RAP_RBUF_CAP_SUPPORT),y)
CFLAGS += -DBDSP_FW_RBUF_CAPTURE
endif
include ${NEXUS_TOP}/../rockford/unittests/nexus/common/Makefile.inc
