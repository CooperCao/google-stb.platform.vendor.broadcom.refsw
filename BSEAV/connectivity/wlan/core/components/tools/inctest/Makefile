#/**
# * @file
# * @brief
# * Makefile for building inctest sources.
# *
# * $ Copyright Broadcom Corporation $
# *
# *
# * <<Broadcom-WL-IPTag/Proprietary:>>
# *
# * $Id$
# */

CC := gcc
INCTEST_TOP := ../../../
INCTEST_BUILD := inctest_build/

INCLUDE_DIRS := \
  components/wlioctl/include/ \
  components/proto/include/ \
  src/include/ \
  components/shared/ \
  src/shared/bcmwifi/include/
OBJS := inctest.o \
  inctest_arr.o \
  inctest_packed.o \
  inctest_unpacked.o

CFLAGS := -I./ $(addprefix -I$(INCTEST_TOP),$(INCLUDE_DIRS))
BUILT_OBJS := $(addprefix $(INCTEST_BUILD),$(OBJS))
MAIN := $(INCTEST_BUILD)inctest

.PHONY: all test
all: $(MAIN)
test: $(MAIN)
	$<

$(MAIN): $(BUILT_OBJS) | $(INCTEST_BUILD)
	$(CC) -o $@ $^

$(INCTEST_BUILD)%.o: %.c | $(INCTEST_BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(INCTEST_BUILD):
	mkdir -p $@

.PHONY: clean
clean:
	$(RM) -r $(INCTEST_BUILD)
