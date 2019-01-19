#!/bin/bash
#
# This script serves following purpose:
#
# 1. It generates native version information by querying
#    automerger maintained database to see where src/include
#    came from
# 2. For select components, as listed in compvers.sh
#    it generates component version files
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
#
# <<Broadcom-WL-IPTag/Proprietary:>>
#
# $Id$
#

7271_EPIVERS_DIR := $(shell pwd)/../../../../7271/src/include
TARGETS := epivers.h

ifdef VERBOSE
export VERBOSE
endif

all release: epivers compvers

# Generate epivers.h for native branch url
epivers:
	@echo "$(7271_EPIVERS_DIR)/epivers.sh"
	@if [ -e "$(7271_EPIVERS_DIR)/epivers.sh" ]; then\
		bash $(7271_EPIVERS_DIR)/epivers.sh; \
	else \
		echo "$(7271_EPIVERS_DIR)/epivers.sh not found"; \
		exit 1; \
	fi

# Generate component versions based on component url
compvers:
	@if [ -s "compvers.sh" ]; then \
		echo "Generating component versions, if any"; \
		bash compvers.sh; \
	else \
		echo "Skipping component version generation"; \
	fi

# Generate epivers.h for native branch version
clean_compvers:
	@if [ -s "compvers.sh" ]; then \
		echo "bash compvers.sh clean"; \
		bash compvers.sh clean; \
	else \
		echo "Skipping component version clean"; \
	fi

clean:
	rm -f $(TARGETS) *.prev

clean_all: clean clean_compvers

.PHONY: all release clean epivers compvers clean_compvers
