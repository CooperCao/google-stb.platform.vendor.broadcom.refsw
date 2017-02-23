# makefile for libpng
# Copyright (C) Glenn Randers-Pehrson
# Copyright (C) 1995 Guy Eric Schalnat, Group 42, Inc.
# For conditions of distribution and use, see copyright notice in png.h

B_THIS_DIR:=BSEAV/opensource/libpng-1.2.8
PHONY := all clean
.PHONY: ${PHONY}

ifeq ($(B_REFSW_REAL_MAKE),)
BSEAV ?= $(subst /${B_THIS_DIR},,$(CURDIR))/BSEAV

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/$(B_THIS_DIR)

${PHONY}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} B_REFSW_REAL_BESAV=${BSEAV} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/makefile ${MAKECMDGOALS}

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE

BSEAV?=${B_REFSW_REAL_BESAV}
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}
ZLIBDIR=$(BSEAV)/opensource/zlib
ZLIB_ODIR=$(B_REFSW_OBJ_ROOT)/BSEAV/opensource/zlib-1.1.3

CFLAGS=-I$(ZLIB_ODIR)/include
ifneq ($(findstring $(PLATFORM),97401 97403 97118 97118RNG 97455 97458),)
CFLAGS += -DPNG_NO_FLOATING_POINT_SUPPORTED
endif
LDFLAGS=-L. -L$(ZLIB_ODIR) -lpng -lz -lm

vpath %.c $(BSEAV)/opensource/libpng
vpath %.h $(BSEAV)/opensource/libpng

#
# Broadcom build support
#
include $(BSEAV)/api/build/tools.mak
include $(BSEAV)/build/refsw_inc.mak
CFLAGS += -g $(B_REFSW_CFLAGS) -w

OBJS = png.o pngset.o pngget.o pngrutil.o pngtrans.o pngwutil.o \
	pngread.o pngrio.o pngwio.o pngwrite.o pngrtran.o \
	pngwtran.o pngmem.o pngerror.o pngpread.o

all: libpng.a

libpng.a: $(OBJS)
	$(AR) rc $@  $(OBJS)
	$(RANLIB) $@

clean:
	-rm -f *.o *.d libpng.a pngtest pngout.png

# DO NOT DELETE THIS LINE -- make depend depends on it.

png.o: png.h pngconf.h
pngerror.o: png.h pngconf.h
pngrio.o: png.h pngconf.h
pngwio.o: png.h pngconf.h
pngmem.o: png.h pngconf.h
pngset.o: png.h pngconf.h
pngget.o: png.h pngconf.h
pngread.o: png.h pngconf.h
pngpread.o: png.h pngconf.h
pngrtran.o: png.h pngconf.h
pngrutil.o: png.h pngconf.h
pngtest.o: png.h pngconf.h
pngtrans.o: png.h pngconf.h
pngwrite.o: png.h pngconf.h
pngwtran.o: png.h pngconf.h
pngwutil.o: png.h pngconf.h
endif
