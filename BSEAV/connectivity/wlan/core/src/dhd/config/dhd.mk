# Helper makefile for building Broadcom dongle host driver (DHD)
# This file maps dhd feature flags DHDFLAGS(import) and DHDFILES_SRC(export).
#
# Copyright (C) 2017, Broadcom. All Rights Reserved.
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id$



#ifdef WLTEST
ifeq ($(WLTEST),1)
	DHDFLAGS += -DWLTEST -DIOCTL_RESP_TIMEOUT=20000
endif
#endif

# Used only lfor debug/internal builds
DHD_DEBUG_FLAGS += -DDHD_DEBUG
DHD_DEBUG_FLAGS += -DBCMPERFSTATS
DHD_DEBUG_FLAGS += -DBCMSLTGT
DHD_DEBUG_FLAGS += -DBCMQT
DHD_DEBUG_FLAGS += -DBCMSDIOH_STD
DHD_DEBUG_FLAGS += -DTESTDONGLE
DHD_DEBUG_FLAGS += -DBCMDBG_ASSERT
DHD_DEBUG_FLAGS += -DBCMDBG_MEM

# NOTE: These need to be conditionalized via appropriate
#       dhdconfig_xxx file

ifeq ($(COMMON_FLAGS),1)
DHDFLAGS += -DBCMDBG
DHDFLAGS += -DSHOW_EVENTS
DHDFLAGS += -DBCMDRIVER
DHDFLAGS += -DBDC
DHDFLAGS += -DBCMSDIO
DHDFLAGS += -DSTA
DHDFLAGS += -DBCMSUP_PSK
DHDFLAGS += -DBCMWPA2
DHDFLAGS += -DBCMDONGLEHOST
DHDFLAGS += -DEMBEDDED_PLATFORM
DHDFLAGS += -DARP_OFFLOAD_SUPPORT
DHDFLAGS += -DPKT_FILTER_SUPPORT

# Inside dhd_sdio.c, this appears as "#include <rtecdc.h>"
DHDFLAGS += -DBCMEMBEDIMAGE="\"\<rtecdc.h\>\""
endif # COMMON_FLAGS

# NOTE: These need to be conditionalized via appropriate
#       dhdconfig_xxx file
ifeq ($(COMMON_FLAGS),1)
DHDFILES_SRC += src/dhd/sys/dhd_cdc.c
DHDFILES_SRC += src/dhd/sys/dhd_wlfc.c
DHDFILES_SRC += src/dhd/sys/dhd_common.c
DHDFILES_SRC += src/dhd/sys/dhd_debug.c
DHDFILES_SRC += src/dhd/sys/dhd_mschdbg.c
DHDFILES_SRC += src/dhd/sys/dhd_ip.c
DHDFILES_SRC += src/dhd/sys/dhd_sdio.c
DHDFILES_SRC += src/shared/aiutils.c
DHDFILES_SRC += src/shared/bcmstdlib.c
DHDFILES_SRC += src/shared/bcmutils.c
DHDFILES_SRC += src/shared/bcmxtlv.c
DHDFILES_SRC += src/shared/hnd_pktq.c
DHDFILES_SRC += src/shared/hnd_pktpool.c
DHDFILES_SRC += src/shared/bcmwifi_channels.c
DHDFILES_SRC += src/shared/hndpmu.c
DHDFILES_SRC += src/shared/ndshared.c
DHDFILES_SRC += src/shared/sbutils.c
DHDFILES_SRC += src/shared/siutils.c
endif # COMMON_FLAGS

# Legacy DHDFILES pathless definition, please use new src relative path
# in target dhd makefiles.
DHDFILES := $(sort $(notdir $(DHDFILES_SRC)))

ifdef DHDCONFIG_TEST
dhdconfig_test:
	@echo "DHDFILES = $(DHDFILES)"
	@echo "DHDFLAGS = $(DHDFLAGS)"
endif # DHDCONFIG_TEST
