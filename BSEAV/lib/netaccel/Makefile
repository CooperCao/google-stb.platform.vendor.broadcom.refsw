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
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

LINUX ?= /opt/brcm/linux

include ../../api/build/tools.mak
include chip.mak

TOP_DIR 	= $(shell ${PWD})
BSEAV 	?= $(shell cd ../..;pwd)
NETACCEL_BIN=$(TOP_DIR)/bin/${BCHP_CHIP}
QUIET=@
BUILD_INSTALL_DIR=${TOP_DIR}/bin/test
INSTALL_DIR ?= ${BSEAV}/bin

#Default build is driver and lib dirs. You can 
# make the app dir by "make app"
BUILD_DIRS = module lib 
#ifeq ($(NETACCEL_APP),y)
#BUILD_DIRS += app
#endif

.PHONY: clean all app lib module bin 
# Turn off BCMEMAC support for 7118, 
ifeq ($(findstring $(PLATFORM),97018 97018RNG 97118 97118RNG), $(PLATFORM))
BCMEMAC_SUPPORT=n
else
BCMEMAC_SUPPORT=y
endif

ifneq ($(NETACCEL_SUPPORT),n)
NETACCEL_SUPPORT=y
export NETACCEL_SUPPORT
endif


ifeq (${BCMEMAC_SUPPORT},y)
	export BCMEMAC_SUPPORT=y
	CFLAGS += -DB_HAS_BCMEMAC_SUPPORT
endif 

ifeq (${NETACCEL_SUPPORT},y)
	CFLAGS += -DB_HAS_NETACCEL
	export NETACCEL_MEMDMA_DIRECT=y

all: module lib bin

else
# only make the bcmnetaccel usermode driver when 
# NETACCEL SUPPORT is not on
all: bin module

endif
	
app: bin lib 
	$(QUIET)$(MAKE) TOP_DIR=$(TOP_DIR) -C app all
	$(QUIET)$(MAKE) TOP_DIR=$(TOP_DIR) -C app INSTALL_DIR=$(BUILD_INSTALL_DIR) install

		   
lib: bin module
	$(QUIET)$(MAKE) TOP_DIR=$(TOP_DIR) -C lib all
	$(QUIET)$(MAKE) TOP_DIR=$(TOP_DIR) -C lib INSTALL_DIR=$(BUILD_INSTALL_DIR) install


module: bin 
	$(QUIET)$(MAKE) TOP_DIR=$(TOP_DIR) -C module all
	$(QUIET)$(MAKE) TOP_DIR=$(TOP_DIR) -C module INSTALL_DIR=$(BUILD_INSTALL_DIR) install

bin:
	$(QUIET)$(MKDIR) ${TOP_DIR}/bin
	$(QUIET)$(MKDIR) ${TOP_DIR}/bin/test

copy:
	$(QUIET)echo [Copy... NetAccel Binaries] 
		
clean:
	$(QUIET)for dir in $(BUILD_DIRS); do \
		$(MAKE) TOP_DIR=$(TOP_DIR) -C $$dir clean; \
	done
	$(QUIET)$(RM) -f $(TOP_DIR)/bin/test/*

ifneq ($(wildcard $(TOP_DIR)/module/Makefile),)
install: all
# This will copy binaries from build destination netaccel/bin/test if built
# from available source, and copy pre-built binaries corresponding to the
# ${LINUX} kernel otherwise.
	@if [ -d $(INSTALL_DIR) ]; then \
		cp -f $(TOP_DIR)/bin/test/* $(INSTALL_DIR); \
	fi
	@if [ -d $(BSEAV)/../nexus/bin ]; then \
		cp -f $(TOP_DIR)/bin/test/bcmnetaccel.ko $(BSEAV)/../nexus/bin; \
	fi
else
install: copy
    ifeq ($(shell (grep 'VERSION = 3' ${LINUX}/Makefile >/dev/null && echo 'y')),y)
        ifeq ($(B_REFSW_ARCH),mips-linux)
		$(CP) $(NETACCEL_BIN)/33/be/* $(INSTALL_DIR)
        else
		${QUIET}$(CP) $(filter-out $(NETACCEL_BIN)/33/be, \
		$(wildcard $(NETACCEL_BIN)/33/*)) $(INSTALL_DIR)
        endif
    else	
    ifeq ($(shell (grep 'SUBLEVEL = 31' ${LINUX}/Makefile >/dev/null && echo 'y')),y)
        ifeq ($(SMP),y)
          ifeq ($(B_REFSW_ARCH),mips-linux)
		$(CP) $(NETACCEL_BIN)/2631/be/smp/* $(INSTALL_DIR)
          else
		${QUIET}$(CP) $(filter-out $(NETACCEL_BIN)/2631/smp/be , \
		$(wildcard $(NETACCEL_BIN)/2631/smp/*)) $(INSTALL_DIR)
          endif
        else
          ifeq ($(B_REFSW_ARCH),mips-linux) 	
		${QUIET}$(CP) $(filter-out $(NETACCEL_BIN)/2631/be/smp, \
		$(wildcard $(NETACCEL_BIN)/2631/be/*)) $(INSTALL_DIR)
          else
		${QUIET}$(CP) $(filter-out $(NETACCEL_BIN)/2631/smp $(NETACCEL_BIN)/2631/be , \
		$(wildcard $(NETACCEL_BIN)/2631/*)) $(INSTALL_DIR)
          endif
        endif
    else
    ifeq ($(shell (grep 'SUBLEVEL = 37' ${LINUX}/Makefile >/dev/null && echo 'y')),y)
        ifeq ($(B_REFSW_ARCH),mips-linux)
		$(CP) $(NETACCEL_BIN)/2637/be/* $(INSTALL_DIR)
        else
		${QUIET}$(CP) $(filter-out $(NETACCEL_BIN)/2637/be, \
		$(wildcard $(NETACCEL_BIN)/2637/*)) $(INSTALL_DIR)
        endif	
    else
	@echo [Unsupported Linux Version... set NETACCEL_SUPPORT=n]
    endif
    endif
    endif
    
endif
