############################################################
#  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#
############################################################
# Nexus examples
B_THIS_DIR:=nexus/examples/dvr
ifdef B_REFSW_REAL_NEXUS_TOP
NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

include $(NEXUS_TOP)/examples/Makefile_prologue.inc

THEAPPS = \
	codec_probe \
	dqt_trick \
	index_audio \
	linked_mosaic_playback \
	mosaic_playback \
	multiplexed_playpump \
	playback \
	playback_audio_es \
	playback_chunk \
	playback_customfile \
	playback_divx \
	playback_dtu \
	playback_message_paced_pes \
	playback_mkv \
	playback_pcr_indexing \
	playback_pip \
	playback_trick \
	playback_4k2k \
	playpump \
	playpump_audio_pes_from_memory \
	playpump_change_asf \
	playpump_ip \
	playpump_lpcm \
	playpump_scatter_gather \
	playpump_two_pes_files \
	record \
	record_allpass \
	record_allpass_from_playback \
	record_allpass_from_qam \
	record_allpass_from_remux \
	record_chunk \
	record_from_playback \
	recpump \
	recpump_with_wrap \
	recpump_with_wrap_and_dma \
	remap \
	stream_editing \
	timeshift

# The following app can't be build in prebuilt binary mode
ifneq ($(NEXUS_PREBUILT_BINARY),y)
THEAPPS += record_append
endif
CLEAN_APPS += record_append

CFLAGS += -I$(NEXUS_TOP)/../BSEAV/lib/netaccel/include
BCMPLAYER_DIR=$(NEXUS_TOP)/../BSEAV/lib/bcmplayer
CFLAGS += -I$(BCMPLAYER_DIR)/include

codec_probe: CFLAGS += -I$(NEXUS_TOP)/../BSEAV/lib/media -I$(NEXUS_TOP)/../BSEAV/lib/utils


include ${NEXUS_TOP}/examples/Makefile.inc


ifeq ($(B_REFSW_REAL_MAKE),1)
CFLAGS += -I${NEXUS_TOP}/../BSEAV/lib/subtitle_sync
vpath %.c ${NEXUS_TOP}/../BSEAV/lib/subtitle_sync
playback_message_paced_pes: sync_manager.c
CFLAGS += -I$(BCMPLAYER_DIR)/utils
vpath %.c $(BCMPLAYER_DIR)/utils
playback_customfile: playback_customfile.c b_customfile.c b_playback.c
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(LDFLAGS) $(CFLAGS)
	cp -f $@ $(NEXUS_BIN_DIR)
endif # B_REFSW_REAL
