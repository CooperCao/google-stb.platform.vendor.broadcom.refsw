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

LUA_SOURCE_DIR := $(shell ${PWD})
LUA_ROOT := $(B_REFSW_OBJ_ROOT)/BSEAV/lib/lua
LUA_DIR  := $(B_REFSW_OBJ_ROOT)/BSEAV/lib/lua/lua-5.2.4

all: lua

.PHONY: lua_dir
lua_dir:
	@if [ ! -d $(LUA_DIR) ]; then \
		mkdir -p $(LUA_ROOT); \
		cd $(LUA_ROOT); \
		echo "[Unpack.... lua]"; \
		tar xzvf $(LUA_SOURCE_DIR)/lua-5.2.4.tar.gz > /dev/null; \
		patch -p0 < $(LUA_SOURCE_DIR)/patch.txt > /dev/null; \
		cp $(BSEAV)/api/build/tools.mak $(LUA_DIR)/src > /dev/null; \
	fi

lua: lua_dir
	@echo "[Build..... lua]";
	@$(MAKE) -C $(LUA_DIR) -f Makefile posix > /dev/null
	@$(MAKE) -C $(LUA_DIR) -f Makefile local > /dev/null

clean:
	@rm -fr $(LUA_ROOT)
