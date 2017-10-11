TAG =
PROD_TAG =

ifneq ($(TAG),)
PRODUCT_TAG= -r $(TAG)
endif

all:
	cvs co $(PRODUCT_TAG) src/tools/release/linux-wps-enrollee.mk
	make -f src/tools/release/linux-wps-enrollee.mk extract_tag
	make -f src/tools/release/linux-wps-enrollee.mk build
