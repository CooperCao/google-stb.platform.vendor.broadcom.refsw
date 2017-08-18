#
# Generate external package for broadcom P2P library/apps
#
# $Id: linux-external-p2p.mk 279131 2011-08-23 03:12:30Z $
#

# Common defs and undefs are in included common makefile
DEFS        := BCMINTERNAL
UNDEFS      := $(filter-out BCMINTERNAL,$(UNDEFS))
BRAND       ?= linux-internal-p2p
OEM_LIST    ?= bcm

include linux-p2p.mk

%.mk: OVFILE=$(if $(OVERRIDE),$(OVERRIDE)/tools/release/$@,)
%.mk:
	cvs co $(if $(TAG),-r $(TAG)) -p src/tools/release/$@ > $@
ifneq ($(OVERRIDE),)
	-[ -f "$(OVFILE)" ] && cp $(OVFILE) $@
endif
