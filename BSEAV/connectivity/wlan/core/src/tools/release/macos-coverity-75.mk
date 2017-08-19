#
# Macos Coverity targets
#
# Contact: hnd-software-scm-list
#
# NOTE: Do not update this makefile or add any new targets. Contact
# NOTE: hnd-software-scm-list for any questions
#

BRAND            ?= macos-coverity-75
COVERITY_RULES   ?= coverity-rules-75.mk
COVERITY_TARGETS ?= coverity-target-info.mk
SVNROOT          ?= http://svn.sj.broadcom.com/svn/wlansvn/proj
SVNCMD           ?= svn --non-interactive

include $(COVERITY_TARGETS)
include $(COVERITY_RULES)

# Checkout central trunk version of coverity target descriptions and rules
%.mk:
	$(SVNCMD) export $(if $(SVNCUTOFF),-r {"$(SVNCUTOFF)"}) \
		$(SVNROOT)/trunk/src/tools/release/$@ $@
