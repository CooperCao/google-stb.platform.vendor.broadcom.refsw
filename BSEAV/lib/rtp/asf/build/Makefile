
include tools.mak

BSEAV = $(shell cd "../../../.." && ${PWD})
ROCKFORD = $(BSEAV)/../rockford

APPS = bin/asfip
ASFIP_BUILD_DIR = .

LM_BASE = $(BSEAV)/lib/livemedia/live
LM_BASE_INCLUDE = $(LM_BASE)/liveMedia/include
LM_BASIC_USAGE_ENV = $(LM_BASE)/BasicUsageEnvironment
LM_BASIC_USAGE_ENV_INCLUDE = $(LM_BASIC_USAGE_ENV)/include

LM_USAGE_ENV = $(LM_BASE)/UsageEnvironment
LM_USAGE_ENV_INCLUDE = $(LM_USAGE_ENV)/include
LM_GROUPSOCK_ENV = $(LM_BASE)/groupsock
LM_GROUPSOCK_ENV_INCLUDE = $(LM_GROUPSOCK_ENV)/include
DRM_INCLUDE = $(BSEAV)/lib/drm 

CFLAGS = -W ${PROFILE_FLAGS}
CFLAGS += -I. -I.. -I$(BSEAV)/lib/utils -I$(BSEAV)/lib/bfile -I$(BSEAV)/lib/asf
CFLAGS += -I$(BSEAV)/lib/media
CFLAGS += $(B_REFSW_MAGNUM_INCLUDE_DIRS) -I$(BSEAV)/api/src 

CFLAGS += -I$(LM_BASE_INCLUDE) -I$(LM_BASIC_USAGE_ENV_INCLUDE) -I$(LM_USAGE_ENV_INCLUDE) -I$(LM_GROUPSOCK_ENV_INCLUDE)

ifeq ($(DRM_SUPPORT), y)
CFLAGS += -DB_HAS_DRM=1 -I$(DRM_INCLUDE)
endif

LDFLAGS += -L$(LM_BASE)/liveMedia -lliveMedia -lpthread
LDFLAGS += -L$(LM_BASE)/groupsock -lgroupsock
LDFLAGS += -L$(LM_BASE)/UsageEnvironment -lUsageEnvironment
LDFLAGS += -L$(LM_BASE)/BasicUsageEnvironment -lBasicUsageEnvironment
CPPFLAGS = ${CFLAGS}

include $(BSEAV)/build/refsw_inc.mak
ODIR = $(B_REFSW_ARCH).$(DEBUG_SUFFIX)
OS = ${B_REFSW_OS}

# Suppress LiveMedia Output for non VERBOSE builds
ifeq ($(VERBOSE),)
VERB_LIVEMEDIA := &> /dev/null
else
VERB_LIVEMEDIA :=
endif

all: $(APPS)

.PHONY: api api_clean	

vpath %.c ../
vpath %.cpp ../ 
vpath %.c $(BSEAV)/api/utils/common
vpath %.c $(BSEAV)/lib/utils
vpath %.c $(BSEAV)/lib/asf/
vpath %.c $(BSEAV)/lib/bfile/

APP_OBJECT =  $(ODIR)/basf_hdr_common.o	\
		$(ODIR)/basf_rtsp_session.o 	\
		$(ODIR)/basf_rtp_proc.o		\
		$(ODIR)/basf_rtp_feed.o		\
		$(ODIR)/basf_rtp_app.o

APP_OBJECT += $(ODIR)/bfile_io.o	\
		$(ODIR)/bfile_stdio.o	\
		$(ODIR)/biovec.o 	\
		$(ODIR)/basf_util.o 	\
		$(ODIR)/basf_stream.o 	\
		$(ODIR)/basf_parser.o 	\
		$(ODIR)/basf_player.o 	\
		$(ODIR)/biobits.o	\
		$(ODIR)/b_namevalue_pairs.o	\
		$(ODIR)/bsettop_namevalue_pairs.o	\
		$(ODIR)/bargs.o		\
		$(ODIR)/btune.o		\
		$(ODIR)/asfpars.o

# handle corner case when static brutus and drm nd
ifeq ($(DRMND_SUPPORT), y) 
ifeq ($(STATIC_BRUTUS), y)
	DRMND_LIB_DIR = $(BSEAV)/lib/drmnd/wmdrmnd10
	LDFLAGS += $(DRMND_LIB_DIR)/bin/drmndport.a 
	LDFLAGS += $(DRMND_LIB_DIR)/oem/ansi/oem_ansi.a
endif
endif

${APPS} : prereq $(APP_OBJECT) livemedia
	@echo [Compile... $(notdir $<)]	
	${Q_}$(CC) -o $@ $(APP_OBJECT) ${MAGNUM_OBJS} $(CFLAGS) $(LDFLAGS) ${BSETTOP_LDFLAGS}

prereq: 
	@$(MKDIR) $(ODIR)
	@$(MKDIR) bin

livemedia:
	@echo [Compile... $(notdir $@)]
	@$(MAKE) -C $(LM_BASE) $(VERB_LIVEMEDIA);	

api_clean:
	@$(MAKE) -C $(BSEAV)/lib/livemedia/live clean

include ${BSEAV}/api/include/api.mak

ifeq ($(findstring $(CHIP),7038 3560), $(CHIP))
CFLAGS += -I$(BSEAV)/api/src/magnum/board
else
CFLAGS += -I$(BSEAV)/api/src/$(CHIP)/board
endif

CFLAGS += $(BSETTOP_CFLAGS) -I$(BSEAV)/api/utils/common
LDFLAGS += -lstdc++ ${PROFILE_FLAGS}

install:
	@$(CP) $(APPS) $(INSTALL_DIR)

clean:
	@${RM} $(APPS) $(APP_OBJECT) *.d *.o *.out

$(ODIR)/%.o: %.cpp
	@echo [Compile... $(notdir $<)]
	${Q_}$(CXX) -s $(CPPFLAGS) -c $^ -o $@

$(ODIR)/%.o: %.c
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) -s $(CFLAGS) -c $^ -o $@


