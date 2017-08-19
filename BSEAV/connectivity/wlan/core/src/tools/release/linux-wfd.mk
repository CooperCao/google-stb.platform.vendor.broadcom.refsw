#
# Common makefile to build Linux WFD on linux from trunk.
#
# NOTE: This makefile should not be used by itself, it is included into
# other brands like linux-external-wfd.mk
#
#

HNDGC_BOM ?= linux-wfd
HNDGC_BOM_DIR = src

all: profile.log checkout build_wfd

include $(MOGRIFY_RULES)
include $(BRAND_RULES)

profile.log :
	@date +"BUILD_START: $(BRAND) TAG=${TAG} %D %T" | tee profile.log

# check out files
checkout: $(CHECKOUT_TGT)

## ------------------------------------------------------------------
## Build WFD
## ------------------------------------------------------------------
build_wfd:
	cd src; make
