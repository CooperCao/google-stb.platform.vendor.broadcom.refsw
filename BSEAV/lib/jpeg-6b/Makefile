############################################################
#     Copyright (c) 2003-2012, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#     Makefile for LUA library
#
############################################################
B_REFSW_OS ?= linuxuser

include ../../../nexus/build/os/$(B_REFSW_OS)/os_tools.inc

BSEAV = $(shell cd "../.." && ${PWD})

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}

JPEG_SOURCE_DIR := $(shell ${PWD})
JPEG_ROOT := $(B_REFSW_OBJ_ROOT)/BSEAV/lib
JPEG_DIR  := $(B_REFSW_OBJ_ROOT)/BSEAV/lib/jpeg-6b

all: jpeg

.PHONY: jpeg_dir
jpeg_dir:
	@if [ ! -d $(JPEG_DIR) ]; then \
		mkdir -p $(JPEG_ROOT); \
		cd $(JPEG_ROOT); \
		echo "[Unpack.... libjpeg]"; \
		tar xzvf $(JPEG_SOURCE_DIR)/jpegsrc.v6b.tar.gz > /dev/null; \
		patch -p0 < $(JPEG_SOURCE_DIR)/patch.txt > /dev/null; \
	fi

.PHONY: config-libjpeg
config-libjpeg: jpeg_dir
#speed of machine vs. granularity of creation/modification creates the need for a little sleep here
	@if [ ! -e $(JPEG_DIR)/Makefile ]; then \
		echo [Config.... libjpeg]; \
		cd $(JPEG_DIR); RANLIB=$(RANLIB) AR="$(AR) rc" CC='$(CC)' sh ./configure 1> /dev/null; \
		sleep 1; \
	fi

jpeg: config-libjpeg
	@if ! $(MAKE) -q -C $(JPEG_DIR) libjpeg.a; then \
		echo [Build..... libjpeg]; \
		$(MAKE) -C $(JPEG_DIR) libjpeg.a 1>/dev/null; \
	fi

clean:
	@rm -fr $(JPEG_DIR)
