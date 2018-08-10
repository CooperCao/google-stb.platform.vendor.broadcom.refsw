###############################################################################
#   Copyright (C) 2018 Broadcom.
#   The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to
#   the terms and conditions of a separate, written license agreement executed
#   between you and Broadcom (an "Authorized License").  Except as set forth in
#   an Authorized License, Broadcom grants no license (express or implied),
#   right to use, or waiver of any kind with respect to the Software, and
#   Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#   THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#   IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization,
#   constitutes the valuable trade secrets of Broadcom, and you shall use all
#   reasonable efforts to protect the confidentiality thereof, and to use this
#   information only in connection with your use of Broadcom integrated circuit
#   products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#   OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#   RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#   IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#   A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#   ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#   THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#   OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#   INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#   RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#   HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#   EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#   WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#   FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################

B_THIS_DIR:=BSEAV/tools/bmon/plugin_launcher

ifdef B_REFSW_REAL_NEXUS_TOP
    NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
include $(NEXUS_TOP)/../BSEAV/opensource/cjson/cjson.inc
NEWCC = $(subst gcc,g++,$(CC))

LDFLAGS=-L$(NEXUS_BIN_DIR)
SFLAGS=-O2 -fPIC -D_LARGEFILE64_SOURCE=1
SHARED_NAME=libplugin_launcher.so
LDSHARED=-shared -Wl,-soname,$(SHARED_NAME)
CFLAGS += -I$(NEXUS_TOP)/../$(B_THIS_DIR)/../common
CFLAGS += -I$(CJSON_SOURCE_PATH) -lcjson -L$(NEXUS_BIN_DIR)
BMON_JSON_OBJ_FILE = $(B_REFSW_OBJ_ROOT)/$(B_THIS_DIR)/bmon_json.o
BMON_URI_OBJ_FILE = $(B_REFSW_OBJ_ROOT)/$(B_THIS_DIR)/bmon_uri.o

$(NEXUS_BIN_DIR)/$(SHARED_NAME): LIB_CJSON $(NEXUS_TOP)/../$(B_THIS_DIR)/plugin_launcher.cpp BMON_JSON_OBJS
	@echo [Build... $(notdir $(NEXUS_BIN_DIR)/$(SHARED_NAME)) .. CC $(NEWCC)]
	@$(NEWCC) $(LDSHARED) $(SFLAGS) -o $(NEXUS_BIN_DIR)/$(SHARED_NAME)  $(NEXUS_TOP)/../$(B_THIS_DIR)/plugin_launcher.cpp $(BMON_JSON_OBJ_FILE) $(BMON_URI_OBJ_FILE) $(LDFLAGS) $(CFLAGS)

BMON_JSON_OBJS:
	@echo [Build... $(notdir $(BMON_JSON_OBJ_FILE)) .. CC $(CC)]
	@mkdir -p $(B_REFSW_OBJ_ROOT)/$(B_THIS_DIR)
	@$(CC) -c -o $(BMON_JSON_OBJ_FILE) ${NEXUS_TOP}/../$(B_THIS_DIR)/../common/bmon_json.c $(SFLAGS) $(CFLAGS)
	@$(CC) -c -o $(BMON_URI_OBJ_FILE)  ${NEXUS_TOP}/../$(B_THIS_DIR)/../common/bmon_uri.c  $(SFLAGS) $(CFLAGS)

LIB_CJSON:
	@echo [Build... $@ .. CC $(CC)]
	make -C $(CJSON_BUILD_DIR)

clean:
	@echo [Clean... $(SHARED_NAME)]
	@rm -f $(NEXUS_BIN_DIR)/$(SHARED_NAME)
	@rm -fr $(B_REFSW_OBJ_ROOT)/$(B_THIS_DIR)
