#
# Top Level Makefile to Build all WPS Components
#
#
#
# Author : Rajaram Gurumurthy, Broadcom Corporation
#
# $Id$
#


WINCEVER  ?= 500 600
PROCESSOR ?= X86 ARM
TTYPE     ?= DBG OPT

SRC_DIR    = ../..

#SHARED_FILES := \
#	src/include \
#	src/tools/epictrl \
#	src/tools/bin/WINCE \
#	src/wl/ce/xsupplicant \
#	src/makefiles \
#	src/bcmcrypto \
#	src/8021x/winpcap/PacketCE \
#	src/GNUmakefile.inc \
#	src/Makerules.env \
#	src/tools/build \
#	src/branding.inc

#MODULE_FILES := \
#        src/wps/ce \
#        src/wps/common

BUILD_DIR_LIST := $(SRC_DIR)/wl/ce/xsupplicant/winpcap/driver \
		$(SRC_DIR)/wl/ce/xsupplicant/winpcap/dll \
		$(SRC_DIR)/wps/common \
		wps_enr/wps_api \
		wps_enr/wps_api_tester

all: build 

build_wps_test_gui:
	# Build wps_test_gui application separately
	$(MAKE) -C wps_enr/wps_test_gui

build_ce: $(foreach devos,$(WINCEVER),$(foreach devproc,$(PROCESSOR),$(foreach ttype,$(TTYPE),$(foreach blddir,$(BUILD_DIR_LIST),$(subst /,__,$(blddir))/$(devos)/$(devproc)/$(ttype)))))
	
$(foreach devos,$(WINCEVER),$(foreach devproc,$(PROCESSOR),$(foreach ttype,$(TTYPE),$(foreach blddir,$(BUILD_DIR_LIST),$(subst /,__,$(blddir))/$(devos)/$(devproc)/$(ttype))))):
	@echo "Current Target: $@"
	@blddir=$(subst __,/,$(word 1,$(subst /, ,$@))); \
	devos=$(word 2,$(subst /, ,$@)); \
	devproc=$(word 3,$(subst /, ,$@)); \
	ttype=$(word 4,$(subst /, ,$@)); \
	echo "$(MAKE) -C $$blddir WINCEVER=$$devos PROCESSOR=$$devproc TTYPE=$$ttype"; \
	$(MAKE) -C $$blddir WINCEVER=$$devos PROCESSOR=$$devproc TTYPE=$$ttype

build_ce_installer:$(foreach devos,$(WINCEVER),$(foreach ttype,$(TTYPE),$(devos)/$(ttype)))
	
$(foreach devos,$(WINCEVER),$(foreach ttype,$(TTYPE),$(devos)/$(ttype))):
	@echo "Current Target: $@"
	@blddir=$(SRC_DIR)/wps/ce/wps_enr/wps_installer \
	devos=$(word 1,$(subst /, ,$@)); \
	devproc=ARM \
	ttype=$(word 2,$(subst /, ,$@)); \
	echo "$(MAKE) -C $$blddir WINCEVER=$$devos PROCESSOR=$$devproc TTYPE=$$ttype"; \
	gmake -C $$blddir WINCEVER=$$devos PROCESSOR=$$devproc TTYPE=$$ttype

build: build_ce build_wps_test_gui build_ce_installer phony

phony:

clean: 
	$(foreach build_dir, $(BUILD_DIR_LIST), $(MAKE) -C $(build_dir) clean;)
	$(MAKE) -C wps_enr/wps_test_gui clean

