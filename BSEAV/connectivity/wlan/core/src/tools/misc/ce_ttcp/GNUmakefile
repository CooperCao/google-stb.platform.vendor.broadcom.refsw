#
# src/tools/misc/ce_ttcp/GNUmakefile
#
# Copyright (C) 2000 Broadcom Corporation, Inc.
#

SRCBASE	= ../../..

BUILD_SH=build_ce.sh

ifdef free
BUILD_SH+=free
DEBUG=free
else
BUILD_SH+=checked
DEBUG=checked
endif

include $(SRCBASE)/Makerules

ifeq ($(TARGETENV), win32)

all: 
	$(MAKE) -k -w ce_ttcp
	$(MAKE) -k -w ce_ttcp free=1

ce_ttcp:
	@echo "Building $(DEBUG) target"; \
	source $(SRCBASE)/tools/build/$(BUILD_SH); \
	unset MAKEFLAGS; \
	$(BUILD) $(MAKEOVERRIDES)

clean:
	rm -rf `find . -name "obj" -or -name "obj-*"`
	rm -rf `find . -name "build.*" -or -name "*.exp" -or -name "*.gen"`

else # !win32

all:

endif # win32

.PHONY: all clean
.PHONY: ce_ttcp
