## Create dummy rtecdc.h or rtecdc_<chiprev>.h for bmac or dhd driver builds
## Application like coverity or precommit infra will can make use of these
## when they build DHD or BMAC drivers
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##

SRCBASE     ?= ..
NULL        := /dev/null
SVNCMD      ?= svn --non-interactive

# Firmware image properties. These can be overriden via cmd line
RELNUM      ?= $(shell date '+%Y.%m.%d.0')
DNGLTIME    ?= $(shell date '+%Y/%m/%d %H:%M:%S')

# Default firmware header file
DNGL_HEADER ?= $(SRCBASE)/include/rtecdc.h

# Multi embeddable firmware images (for certain target)
DNGL_HEADERS = $(sort $(patsubst %,$(SRCBASE)/include/%,$(subst ",,$(shell grep "\#include.*rtecdc_.*.h" $(SRCBASE)/shared/dbus*.c 2> $(NULL) | awk '{print $$NF}'))))

all: generate_dummy_dongle generate_dummy_dongles

# Default rule, create common dummy rtecdc.h that is typically embedded by
# all high/dhd driver ports

generate_dummy_dongle dummy_dongle: $(DNGL_HEADER)

$(DNGL_HEADER):
	@echo '/* Dummy rtecdc.h for dhd build'         > $@
	@echo ' * $Broadcom Proprietary and Confidential. Copyright (C) 2017,@
	@echo ' * $All Rights Reserved.@
	@echo ' * $@
	@echo ' * $This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;@
	@echo ' * $the contents of this file may not be disclosed to third parties, copied@
	@echo ' * $or duplicated in any form, in whole or in part, without the prior@
	@echo ' * $written permission of Broadcom.@
	@echo ' * $$Id$$'                              >> $@
	@echo ' */'                                    >> $@
	@echo "#ifdef MEMBLOCK"                        >> $@
	@echo "#define MYMEMBLOCK MEMBLOCK"            >> $@
	@echo "#else"                                  >> $@
	@echo "#define MYMEMBLOCK 2048"                >> $@
	@echo "#endif"                                 >> $@
	@echo "char dlimagename[] = \"dummy_dongle\";" >> $@
	@echo "char dlimagever[]  = \"$(RELNUM)\";"    >> $@
	@echo "char dlimagedate[] = \"$(DNGLTIME)\";"  >> $@
	@echo ""                                       >> $@
	@echo "unsigned char dlarray[MYMEMBLOCK + 1];" >> $@
	@echo "#undef MYMEMBLOCK"                      >> $@
	@echo "INFO: Generated dummy $(DNGL_HEADER) file"

# Optionally create a dummy rtecdc.h, followed chip specific headers that
# are embedded some flavors of high/dhd driver ports

generate_dummy_dongles dummy_dongles: $(DNGL_HEADER) $(DNGL_HEADERS)

$(DNGL_HEADERS): chiprev=$(subst rtecdc_,,$(basename $(shell basename $@)))
$(DNGL_HEADERS): $(DNGL_HEADER)
$(DNGL_HEADERS):
		@cat $(DNGL_HEADER) | \
		sed     -e "s/dlarray/dlarray_$(chiprev)/g" \
			-e "s/dlimagename/dlimagename_$(chiprev)/g" \
			-e "s/dlimagever/dlimagever_$(chiprev)/g" \
			-e "s/dlimagedate/dlimagedate_$(chiprev)/g" \
			-e "s/dummy/dummy_$(chiprev)/g" \
			-e "s/rtecdc.h/rtecdc_$(chiprev).h/g" \
		> $@
		@echo "INFO: Generated dummy $@ file"

clean:
	@rm -fv $(DNGL_HEADER) $(DNGL_HEADERS)
