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
#
############################################################

############################################################
#
# Nexus multiprocess examples
#
# The multiprocess directory contains both client and server apps.
# For kernel mode (i.e. NEXUS_MODE=proxy), all apps link to libnexus.so.
# for user mode (i.e. NEXUS_MODE=client), server apps link to libnexus.so and client apps link to libnexus_client.so.
# In both cases, nexus will perform an internal two-pass build to generate both driver/proxy or server/client.
# The following are typical build instructions:
# 
# # make nexus server, client and all apps
# make 
# 
# # make nexus client and all client apps
# make client
# 
# # make nexus server and all server apps
# make server
# 
# # clean nexus server, client and all apps
# make clean
# 
# # clean just the apps
# make clean_apps
#
# # make a specific client or server app
# make <appname>
#
############################################################

SERVER_APPS := \
	boot_server \
	decode_server \
	blit_server  \
	encode_server \
	input_router \
	linux_twoapp \
	fill \
	surface_compositor

CLIENT_APPS := \
	animation_client \
	audio_client \
	blit_client \
	cpufill_client \
	decode_client \
	encode_client \
	fcc_client \
	input_client \
	live_client \
	performance_test \
	picture_decoder_client \
	status_client \
	client_stress_test

APPS = $(CLIENT_APPS) $(SERVER_APPS)
.PHONY: client server

include ../Makefile.inc

# media probe
BSEAV = $(NEXUS_TOP)/../BSEAV
include $(BSEAV)/lib/utils/batom.inc
include $(BSEAV)/lib/media/bmedia.inc
CFLAGS += $(addprefix -I,$(BMEDIA_INCLUDES))
CFLAGS += $(addprefix -D,$(BMEDIA_PROBE_DEFINES))

server: $(addprefix $(BINDIR)/,$(SERVER_APPS))

client: $(addprefix $(BINDIR)/,$(CLIENT_APPS))

# use make's static pattern rules to give clients special LDFLAGS
vpath %.c $(NEXUS_TOP)/nxclient/apps/utils $(NEXUS_TOP)/utils

$(addprefix ${BINDIR}/,$(CLIENT_APPS)): LDFLAGS := $(CLIENT_LDFLAGS)

${BINDIR}/decode_client: CFLAGS += -I$(NEXUS_TOP)/nxclient/apps/utils -I$(NEXUS_TOP)/utils
${BINDIR}/decode_client: media_probe.c bfile_crypto.c namevalue.c

${BINDIR}/fcc_client: CFLAGS += -I$(NEXUS_TOP)/nxclient/apps/utils -I$(NEXUS_TOP)/utils
${BINDIR}/fcc_client: tspsimgr3.c namevalue.c

-include $(wildcard $(BINDIR)/*.d)
