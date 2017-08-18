#
# Given a component show its filtered list of recursive and non-recursive dirs
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#

COMPONENTS_DIR ?= components
COMPONENT_DEF  := $(COMPONENTS_DIR)/$(COMPONENT).mk

all: show_sources show_local_sources

show_sources:
	@(cat $(COMPONENT_DEF); echo 'show_comp:;@echo \$$(SOURCES)') | \
		$(MAKE) -k -f - show_comp 2> /dev/null

show_local_sources:
	@(cat $(COMPONENT_DEF); echo 'show_comp:;@echo \$$(LOCAL_SOURCES)') | \
		$(MAKE) -k -f - show_comp 2> /dev/null
