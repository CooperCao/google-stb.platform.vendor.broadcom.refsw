##
## WpsWizard project makefile
##
## $Id: GNUmakefile,v 1.1 2008/09/23 18:16:04 Exp $

SRCBASE            := ../../../..
SOLUTION           := $(SRCBASE)/wps/win32/wps_enr/wps_enr.sln
PROJECT            := WpsWizard
PROJECTSTRINGS     := WpsWizardStrs.txt
PROJECTRC2         := $(PROJECT)Strings.rc2
BRANDINGRC2        := brandingStrings.rc2
BRANDINGRC         := brandingStrings.rc
BUILD_BRANDS       := true
#	OEM                ?= bcm
#	OEM_LIST           ?= $(OEM)

#	REG_FILES          := WlMgmt.reg
#	REG_FILES          += WlMgmtInteractiveSec.reg 
#	REG_FILES          += WlMgmtServiceSec.reg 
#	REG_FILES          += WlMgmtNTSystemSec.reg

include $(SRCBASE)/wl/locale/Locale.mk
include $(SRCBASE)/makefiles/msvs_rules.mk

#	prebuild_xp: prebuild_rc prebuild_rc_extra
prebuild_xp: prebuild_rc

#	prebuild_rc_extra: $(patsubst %.reg,$(PROJCFGDIR)/$(OEM)/%.branded.reg,$(REG_FILES))

#$(patsubst %.reg,$(PROJCFGDIR)/$(OEM)/%.branded.reg,$(REG_FILES)): $(PROJCFGDIR)/$(OEM)/oem_change.sh
#	mkdir -p $(@D)
#	cat $(subst .branded,,$(@F)) | sh $(@D)/oem_change.sh > $@
