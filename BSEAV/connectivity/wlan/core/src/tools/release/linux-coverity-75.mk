#
# Linux Coverity Run Targets
# This is a temporary brand used to debug Coverity version 6.5
#
# Author: Mark Beyer
# Contact: hnd-software-scm-list
#
# $Id $
#
# NOTE: Do not update this makefile or add any new targets. Contact
# NOTE: prakashd for any questions
#

BRAND            ?= linux-coverity-75
COVERITY_RULES   ?= coverity-rules-75.mk
COVERITY_TARGETS ?= coverity-target-info.mk
SVNROOT          ?= http://svn.sj.broadcom.com/svn/wlansvn/proj
SVNCMD           ?= svn --non-interactive

# Linux Utils
LINUX_CXAPI_OPTS  = $(fc4-opts)

include $(COVERITY_TARGETS)
include $(COVERITY_RULES)

# Checkout central trunk version of coverity target descriptions and rules
%.mk:
	$(SVNCMD) export $(if $(SVNCUTOFF),-r {"$(SVNCUTOFF)"}) \
		$(SVNROOT)/trunk/src/tools/release/$@ $@
