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
############################################################

BSEAV := $(NEXUS_TOP)/../BSEAV
include $(BSEAV)/lib/tshdrbuilder/tshdrbuilder.inc

NXUTILS_INCLUDES := \
	$(NEXUS_TOP)/nxclient/apps/utils \
	$(NEXUS_TOP)/utils \
	$(TSHDRBUILDER_INCLUDES) \
	$(NEXUS_TOP)/../BSEAV/lib/thumbnail \
	$(NEXUS_TOP)/../BSEAV/lib/media \
	$(NEXUS_TOP)/../BSEAV/lib/utils \
	$(NEXUS_TOP)/../BSEAV/lib/bcmplayer/include

# These can be used with and without NxClient
NXUTILS_SINGLE_SOURCES := \
	$(NEXUS_TOP)/nxclient/apps/utils/bfile_crypto.c \
	$(NEXUS_TOP)/nxclient/apps/utils/bfont.c \
	$(NEXUS_TOP)/nxclient/apps/utils/bgui.c \
	$(NEXUS_TOP)/nxclient/apps/utils/binput.c \
	$(NEXUS_TOP)/nxclient/apps/utils/dvr_crypto.c \
	$(NEXUS_TOP)/nxclient/apps/utils/media_probe.c \
	$(NEXUS_TOP)/utils/namevalue.c \
	$(NEXUS_TOP)/nxclient/apps/utils/picdecoder.c \
	$(TSHDRBUILDER_SOURCES) \
	$(NEXUS_TOP)/nxclient/apps/utils/tspsimgr3.c \
	$(NEXUS_TOP)/nxclient/apps/utils/wav_file.c \
	$(NEXUS_TOP)/nxclient/apps/utils/thumbdecoder.c \
	$(NEXUS_TOP)/nxclient/apps/utils/botf_bcmindexer.c \
	$(NEXUS_TOP)/../BSEAV/lib/thumbnail/bthumbnail_extractor.c \
	$(NEXUS_TOP)/../BSEAV/lib/bcmplayer/src/tsindexer.c

NXUTILS_SOURCES := \
	$(NXUTILS_SINGLE_SOURCES) \
	$(NEXUS_TOP)/nxclient/apps/utils/live_decode.c \
	$(NEXUS_TOP)/nxclient/apps/utils/live_source.c \
	$(NEXUS_TOP)/nxclient/apps/utils/media_player.c \
	$(NEXUS_TOP)/nxclient/apps/utils/brecord_gui.c \
	$(NEXUS_TOP)/nxclient/apps/utils/nxapps_cmdline.c
