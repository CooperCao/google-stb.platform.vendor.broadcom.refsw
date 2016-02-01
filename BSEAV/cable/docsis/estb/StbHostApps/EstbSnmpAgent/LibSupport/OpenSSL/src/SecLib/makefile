#**************************************************************************
#
#    Copyright (c) 2008 Broadcom Corporation
#    All Rights Reserved
#    No portions of this material may be reproduced in any form without the
#    written permission of:
#             Broadcom Corporation
#             16251 Laguna Canyon Road
#             Irvine, California  92618
#    All information contained in this document is Broadcom Corporation
#    company private, proprietary, and trade secret.
#
#

# Include general makefile rules 
include $(RULES_FILE)

# global definitions
PROJECTLIB  = $(LIBRARY_PATH)\libseccrypto.a

#  Source files 
sourcec       := $(wildcard *.c)

subdirs =

# Internal definitions
dirs          := $(patsubst %,%.dir,$(subdirs))
c_subdirs     := $(patsubst %,%.cln,$(subdirs))
all_subdirs   := $(patsubst %,%.all,$(subdirs))
allcfiles      = $(sourcec)
c_objects      = $(allcfiles:.c=.o)

all_objects   := $(c_objects)
depends        = $(all_objects:.o=.d) 
old_depends   := $(wildcard *.d)

#
# Build targets
#
target : $(dirs) project

project : $(depends) $(all_objects) $(PROJECTLIB) 

$(PROJECTLIB): $(all_objects)
	$(ARCHIVE) $?

all : 
	@$(MAKE) clean
	@$(MAKE) target	

clean : $(c_subdirs)
	rm -f *.o
	rm -f *.d
	rm -f $(PROJECTLIB)
   
# Include dependencies

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),all)
ifneq ($(wildcard *.d),)

include $(wildcard *.d)

endif
endif
endif


