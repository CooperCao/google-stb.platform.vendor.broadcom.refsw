#***************************************************************************
#*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#*
#*  This program is the proprietary software of Broadcom and/or its licensors,
#*  and may only be used, duplicated, modified or distributed pursuant to the terms and
#*  conditions of a separate, written license agreement executed between you and Broadcom
#*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#*  no license (express or implied), right to use, or waiver of any kind with respect to the
#*  Software, and Broadcom expressly reserves all rights in and to the Software and all
#*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#*
#*  Except as expressly set forth in the Authorized License,
#*
#*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#*  and to use this information only in connection with your use of Broadcom integrated circuit products.
#*
#*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#*  USE OR PERFORMANCE OF THE SOFTWARE.
#*
#*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#*  ANY LIMITED REMEDY.
#**************************************************************************

NEXUS_TOP ?= $(shell cd ../../../../nexus; pwd)
MAGNUM ?= ${NEXUS_TOP}/../magnum
BSEAV ?= ${NEXUS_TOP}/../BSEAV

APPS = \
    encode \
    get_encode_latency \
    sid_transcode \
    transcode_hls \
    video_conf \
    encode_audio_es \
    encode_video_es \
    transcode_mp4 \
    transcode_vpx \
    transcode_ts


# transcode_ts required oslib
OS_LIB_UNITTESTS=y

ifeq ($(SSL_SUPPORT), y)
    #  Include OpenSSL make definitions
    include $(BSEAV)/opensource/openssl/openssl.inc

    LDFLAGS += $(B_OPENSSL_LDFLAGS)
endif

# add tshdrbuilder
include $(BSEAV)/lib/tshdrbuilder/tshdrbuilder.inc
MODULES += TSHDRBUILDER
# add bcmplayer
include $(BSEAV)/lib/bcmplayer/bcmplayer.inc
API_MODULES += B_NAV_PLAYER

B_PSIP_TOP := $(NEXUS_TOP)/../rockford/lib/psip

# add psiplib
include $(B_PSIP_TOP)/b_psip_lib.inc
B_PSIP_LIB_INCLUDES = ${B_PSIP_LIB_PUBLIC_INCLUDES} ${B_PSIP_LIB_PRIVATE_INCLUDES} $(BSEAV)/api/include
MODULES += B_PSIP_LIB

include $(BSEAV)/lib/bfile/bfile.inc
MODULES += BFILE_STDIO
API_MODULES += BFILE
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
MODULES += BMEDIA_PROBE

# add namevalue
CFLAGS += -I$(NEXUS_TOP)/utils

include $(MAGNUM)/basemodules/mma/bmma.inc
include $(MAGNUM)/commonutils/avc/bavc.inc
include $(MAGNUM)/commonutils/fmt/bfmt.inc
include $(MAGNUM)/commonutils/pxl/bpxl.inc
include $(MAGNUM)/commonutils/sur/bsur.inc
include $(MAGNUM)/commonutils/udp/budp.inc
include $(MAGNUM)/commonutils/vlc/bvlc.inc
MODULES += BMMA BAVC BFMT BPXL BSUR BUDP BVLC
MAGNUM_MODULES:=

CFLAGS +=$(addprefix -I,$(sort $(foreach module, $(MODULES) ${API_MODULES}, $($(module)_INCLUDES))))
CFLAGS += $(addprefix -D,$(sort $(foreach module, $(MODULES) ${API_MODULES}, $($(module)_DEFINES))))
EXTRA_SRC += $(notdir $(sort $(foreach module, $(MODULES), $($(module)_SOURCES))))
vpath %c $(dir $(sort $(foreach module, $(MODULES), $($(module)_SOURCES))))

include ../common/Makefile.inc
