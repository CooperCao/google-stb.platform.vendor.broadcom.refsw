############################################################################
##  This file includes targets for generating a full dependency list.
############################################################################

# "make deps" generates all the individual dependency files and concatenates
# them into one file.
deps : $(sort $(filter-out snmp,$(subst .o,.dep,$(MACH_DEP))))
	copy *.dep makefile.deps >NUL:
#	cat *.dep | sed -e "s/([a-z])$$/\1 \\/" >makefile.deps
	rm *.dep

# This sed command string is used by both the C and C++ rules below.  It turns
# any filename in the Tornado (WIND_BASE) or Epilogue directories into "DELETE"
# for later removal by deps.sed.
SED_CMD = -e "s, $(WIND_BASE)[^ ]*, DELETE,g" -e "s, $(subst \,\\,$(EPILOGUE_ROOT))[^ ]*, DELETE,g" -f ../deps.sed

# A rule to make a dependency listing for a C++ source file.
%.dep : %.cpp
	@ echo ==== Making $@ from $(subst \,/,$<)
	@ $(CXX) $(C++FLAGS) -M $< >$@.tmp
	@ cat $@.tmp | sed $(SED_CMD) | sort /r >$@
	@ echo #>>$@
	@ rm -f $@.tmp

# A rule to make a dependency listing for a C source file.
%.dep : %.c
	@ echo ==== Making $@ from $(subst \,/,$<)
	@ $(CC) $(CFLAGS) -M $< >$@.tmp
	@ cat $@.tmp | sed $(SED_CMD) | sort /r >$@
	@ echo #>>$@
	@ rm -f $@.tmp

# A rule to make a dependency listing for an assembly source file.
%.dep : %.s
	@ echo ==== Making $@ from $(subst \,/,$<)
	@ echo #>>$@

# A rule to make a dependency listing for a supplied object file.
%.dep : %.o
	@ echo ==== Making $@ from $(subst \,/,$<)
	@ echo #>>$@

# Conditionally include an existing makefile.deps.  This won't generate an
# error if the file doesn't exist.
ifeq ("$(wildcard makefile.deps)", "makefile.deps")
include makefile.deps
endif
