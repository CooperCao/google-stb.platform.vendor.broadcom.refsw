# Nexus examples

ifdef COMSPEC
# Any DOS environment
BSEAV := $(shell cd ../../../BSEAV && cd)
NEXUS_TOP := $(shell cd ../../../nexus && cd)
else
BSEAV := $(shell cd ../../../BSEAV; pwd)
NEXUS_TOP := $(shell cd ../../../nexus; pwd)
endif

B_LIB_TOP=$(NEXUS_TOP)/lib
LIB=$(NEXUS_TOP)/../BSEAV/lib

ifndef PLATFORM
$(error PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(PLATFORM)/build/platform_app.inc

OS ?= $(B_REFSW_OS)
ARCH ?= $(B_REFSW_ARCH)
DEBUG ?= $(B_REFSW_DEBUG)

FONTS = AriBlk.ttf

THEAPPS = subtitle \
	  regcode


ifeq ($(B_REFSW_OS),vxworks)
# VxWorks needs a wrapper function to call main.
VXOBJS = vxworks_cmd.o
# We like to use .out for loadable objects.
APPSUFFIX = .out
APPS = $(addsuffix .out, $(THEAPPS))
else
APPS = $(THEAPPS)	
endif

include ${BSEAV}/lib/avi/bavi.inc
include ${BSEAV}/lib/bfile/bfile.inc
include ${BSEAV}/lib/media/bmedia.inc
include $(B_LIB_TOP)/os/b_os_lib.inc
include $(BSEAV)/lib/bwin/include/bwin.mak

.PHONY: api clean

OBJDIR=./obj

all: $(APPS)


$(OBJDIR)/exists:
	mkdir -p $(OBJDIR)
	@touch $@

$(APPS): api

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -lnexus -L$(NEXUS_TOP)/bin -lpthread -lb_os -lm
endif


# This builds the settop api
api:
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(B_LIB_TOP)/os
	$(MAKE) -C $(BSEAV)/lib/bwin/build

# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	$(MAKE) -C $(B_LIB_TOP)/os clean
	$(MAKE) -C $(BSEAV)/lib/bwin/build clean
	-$(RM) -r $(APP) $(OBJDIR)

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

CFLAGS += $(addprefix -I,$(MAGNUM)/commonutils/fmt)
CFLAGS += $(addprefix -I,$(MAGNUM)/commonutils/vlc)


# Convert applib include paths into single variable.
APPLIB_INCLUDES := $(foreach lib,$(B_LIBS),$($(lib)_PUBLIC_INCLUDES))
CFLAGS += $(addprefix -I,$(APPLIB_INCLUDES))
APPLIB_DEFINES := $(foreach lib,$(B_LIBS),$($(lib)_DEFINES))
CFLAGS += $(addprefix -D,$(APPLIB_DEFINES))

# Always build with debug
CFLAGS += -g


BSEAV = $(NEXUS_TOP)/../BSEAV
include $(BSEAV)/lib/bfile/bfile.inc
ifeq ($(MEDIA_AVI_SUPPORT),y)
include $(BSEAV)/lib/avi/bavi.inc
endif
include $(BSEAV)/lib/media/bmedia.inc
include $(BSEAV)/lib/utils/batom.inc
CFLAGS += $(addprefix -I,$(BMEDIA_INCLUDES) $(BFILE_MEDIA_INCLUDES)) -I$(BSEAV)/lib/mpeg2_ts_parse -I$(BSEAV)/lib/tspsi
CFLAGS += $(addprefix -D,$(BMEDIA_PROBE_DEFINES))

SRCS += $(BMEDIA_PROBE_SOURCES) $(BFILE_SOURCES) $(BFILE_MEDIA_SOURCES) $(BFILE_STDIO_SOURCES)
SRCS += \
	$(BSEAV)/lib/mpeg2_ts_parse/ts_pmt.c \
	$(BSEAV)/lib/mpeg2_ts_parse/ts_pat.c \
	$(BSEAV)/lib/mpeg2_ts_parse/ts_priv.c \
	$(BSEAV)/lib/mpeg2_ts_parse/ts_psi.c \
	$(BSEAV)/lib/tspsi/tspsi_validate.c


CFLAGS += -I ${BSEAV}/lib/subtitle_sync
SRCS += ${BSEAV}/lib/subtitle_sync/sync_manager.c
SRCS += subtitles.c subtitle_control.c subtitle_priv.c subtitle_parse.c key.c

OBJS = $(foreach file, $(SRCS), $(OBJDIR)/$(notdir $(patsubst %.c, %.o, $(file))))

REGCODE_SRC = regcode.c key.c
REGCODE_OBJ = $(foreach file, $(REGCODE_SRC), $(OBJDIR)/$(notdir $(patsubst %.c, %.o, $(file))))

vpath %.c $(foreach file, $(SRCS), $(dir $(patsubst %.c, %.o, $(file))))

CFLAGS += $(BWIN_CFLAGS)
LDFLAGS += ${BWIN_LDFLAGS}


$(OBJDIR)/%.o: %.c ${OBJDIR}/exists
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS)


subtitle$(APPSUFFIX): $(OBJS) $(VXOBJS)
	@echo [Link... $@]
ifeq ($(B_REFSW_OS),linuxkernel)
	${Q_}$(LD) ${LDFLAGS} --strip-debug -r $(filter %.o, $^) ${NEXUS_BIN_DIR}/libnexus.a -o $@
else
	$(Q_)$(CC) -o $@ $(filter %.o, $^) $(LDFLAGS)
endif
	cp -f $@ ${FONTS} ${NEXUS_BIN_DIR}

regcode$(APPSUFFIX): $(REGCODE_OBJ) $(VXOBJS)
	@echo [Link... $@]
ifeq ($(B_REFSW_OS),linuxkernel)
	${Q_}$(LD) ${LDFLAGS} --strip-debug -r $(filter %.o, $^) ${NEXUS_BIN_DIR}/libnexus.a -o $@
else
	$(Q_)$(CC) -o $@ $(filter %.o, $^) $(LDFLAGS)
endif
	cp -f $@ ${NEXUS_BIN_DIR}


ifeq ($(SYSTEM),vxworks)
# Explicit rule for building vxworks wrapper app
vxworks_cmd.o: vxworks_cmd.c
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
endif

install: all
