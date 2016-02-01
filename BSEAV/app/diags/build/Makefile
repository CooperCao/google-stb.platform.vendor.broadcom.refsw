#############################################################################
# (c) 2003-2014 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#############################################################################

BUILD_FRONTEND_TEST=y
PLAYBACK_IP_SUPPORT=n

ifeq ($(filter $(NEXUS_PLATFORM),97366 97445), $(NEXUS_PLATFORM))
	ifeq ($(NEXUS_USE_7445_DBS),y)
		BUILD_SAT_TEST=1
	else
		ifeq ($(NEXUS_USE_7366_SV),y)
			BUILD_SAT_TEST=1
		else
			ifeq ($(NEXUS_USE_7399_SFF),y)
				BUILD_SAT_TEST=1
			else
				ifeq ($(NEXUS_USE_7445_SV),y)
					BUILD_QAM_TEST=1
				endif
			endif
		endif
	endif
endif

#
# Reference Software Makefile
#
# This Makefile will build 3 things
# 1) driver(s)
# 2) nexus api (shared or static)
# 3) libs
# 4) diags
#

#####
###First thing we must include the tools
#####

#########
#### Figure out OS
#########
ifeq ($(B_REFSW_OS),)
B_REFSW_OS=linuxuser
endif

include ../../../../nexus/build/os/$(B_REFSW_OS)/os_tools.inc

BSEAV = $(shell cd "../../.." && ${PWD})
NEXUS_TOP = $(shell cd "../../../../nexus" && ${PWD})
ifeq ($(NEXUS_PLATFORM),)
# PLATFORM is deprecated
$(error Please define NEXUS_PLATFORM first)
else
PLATFORM := $(NEXUS_PLATFORM)
export PLATFORM
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}

ifeq ($(DIAGS_BIN_DIR),)
# location of final binaries
DIAGS_BIN_ROOT = $(B_REFSW_OBJ_ROOT)
DIAGS_BIN_DIR = $(DIAGS_BIN_ROOT)/BSEAV/bin${DIAGS_BIN_DIR_SUFFIX}
else
DIAGS_BIN_ROOT = $(DIAGS_BIN_DIR)
endif

ifeq ($(NEXUS_MODE),)
# MODE is deprecated
NEXUS_MODE = $(MODE)
endif


#########
#### Figure out ARCH
#########
ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mips-linux
export B_REFSW_ARCH
endif
ifneq ($(filter $(B_REFSW_DEBUG),n no_error_messages),)
    DEBUG_SUFFIX=release
else
    DEBUG_SUFFIX=debug
endif

#########
#### Include NEXUS .inc
#########
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
B_LIB_TOP := $(NEXUS_TOP)/lib
include $(B_LIB_TOP)/os/b_os_lib.inc

# Get system-specific CFLAGS and LDFLAGS based on SYSTEM, PLATFORM, DEBUG
include $(BSEAV)/build/refsw_inc.mak

#################
############ Staging Area setup directory. It will be destroyed later
#################
STAGING_DIR = ${DIAGS_BIN_DIR}/setup

##############
##### DIAGS_ODIR keeps binary files separate
##### BCHP_REV is not included, so there should be no #ifdef BCHP_REV in Diags
##############
DIAGS_ODIR = $(DIAGS_BIN_ROOT)/BSEAV/app/diags/$(NEXUS_PLATFORM).$(B_REFSW_ARCH).$(DEBUG_SUFFIX)
APP = $(DIAGS_ODIR)/diags
DRIVERDIR=$(BSEAV)/${SYSTEM}/driver/build

#######
#### Font Directories
#######
ifeq ($(B_REFSW_ARCH),mips-linux)
PRERENDERED_FONTS = ../fonts/mswebfonts/be/*.bwin_font
else # goes for ARM as well which is LE only
PRERENDERED_FONTS = ../fonts/mswebfonts/le/*.bwin_font
endif

##########
#### Diags now defaults to IP Settop mode for all platforms
#### You can turn this default off with PLAYBACK_IP_SUPPORT=n
#### Netaccel support is deprecated and not included in any further Releases
##########
# Playback is not used yet in Diags only standalone apps are copied over ip_streamer ip_client
ifneq ($(PLAYBACK_IP_SUPPORT),n)
PLAYBACK_IP_SUPPORT=y
include $(NEXUS_TOP)/lib/playback_ip/b_playback_ip_lib.inc
LDFLAGS += $(PLAYBACK_IP_LDFLAGS)
CFLAGS += -DPLAYBACK_IP_SUPPORT $(PLAYBACK_IP_CFLAGS)
B_PSIP_TOP := $(NEXUS_TOP)/../rockford/lib/psip
include $(B_PSIP_TOP)/b_psip_lib.inc
include $(BSEAV)/lib/tshdrbuilder/tshdrbuilder.inc
CFLAGS += $(addprefix -I,$(TSHDRBUILDER_INCLUDES))

IP_APPLIB_TEST_INCLUDE_PATH += \
	$(B_PSIP_LIB_PUBLIC_INCLUDES) \
	$(NEXUS_TOP)/lib/playback_ip/src/ \
	$(NEXUS_TOP)/lib/playback_ip/include/ \
	$(BSEAV)/lib/rtp \
	$(BSEAV)/lib/utils \
	$(BSEAV)/lib/mpeg2_ts_parse \
	$(BSEAV)/lib/tspsi \
	$(BSEAV)/api/include \
	$(NEXUS_TOP)/lib/dtcp_ip/include

CFLAGS += $(addprefix -I,$(IP_APPLIB_TEST_INCLUDE_PATH)) 
AUTO_PSI_SUPPORT=y
endif

.PHONY: bin_install api all $(DIAGS_ODIR)

# The name of the installation tar ball , Change to Diags
#
BUILDDATE = `date +%Y%m%d`
DIAGS_TAR_BALL = refsw-$(BUILDDATE).$(NEXUS_PLATFORM)-$(SYSTEM)
DIAGS_TAR_BALL := $(DIAGS_TAR_BALL)-diags
DIAGS_TAR_BALL := $(DIAGS_TAR_BALL).bin.tgz

#
# Set B_REFSW_SHAREABLE based on a variety of flags
#
ifeq ($(filter y,$(STATIC_DIAGS)),y)
B_REFSW_SHAREABLE=n
endif

# bin_install is the default rule.
# 'all' builds everything but doesn't perform bin_install or install.
# bin_install copies only binaries into BSEAV/bin. It allows you to
#  run from that directory without having your configuration files overwritten.
# install builds an installation tarball in BSEAV/bin.
bin_install:  $(DIAGS_BIN_DIR) api all 
	@echo [Install... binaries]
# Copy over the nexus script into the correct Directory
	$(Q_)$(CP) $(APP) $(DIAGS_BIN_DIR)
	$(Q_)$(CP) $(NEXUS_BIN_DIR)/logger $(DIAGS_BIN_DIR)
	$(Q_)$(CP) $(NEXUS_BIN_DIR)/nexus $(DIAGS_BIN_DIR)
	$(Q_)$(CP) start $(DIAGS_BIN_DIR)
	@chmod +x $(DIAGS_BIN_DIR)/nexus
	@chmod +x $(DIAGS_BIN_DIR)/start
	@echo [Install... Diags Complete]




$(DIAGS_ODIR):
	$(Q_)$(MKDIR) $(DIAGS_ODIR)
	
##########
##### NEXUS API Libraries
##########

##########
#####always build main.o because it contains __DATE__
##########
build_date:
	$(Q_)$(RM) -f $(DIAGS_ODIR)/main.o

clean-diags:
	-@$(RM) -rf $(DIAGS_ODIR)

lib:
	${Q_}$(MAKE) -C $(BSEAV)/lib/bwin/build
	${Q_}$(MAKE) -C $(BSEAV)/lib/mlibs
	${Q_}$(MAKE) -C $(BWIDGETS_DIR)/build

lib-clean:
	${Q_}$(MAKE) -C $(BSEAV)/lib/bwin/build clean
	${Q_}$(MAKE) -C $(BSEAV)/lib/mlibs clean
	${Q_}$(MAKE) -C $(BWIDGETS_DIR)/build clean

print-diags-tarball:
	@echo $(DIAGS_TAR_BALL)

$(DIAGS_BIN_DIR):
	$(Q_)${MKDIR} ${DIAGS_BIN_DIR}

########## 
##### Rule for all and clean
##########
all:  $(DIAGS_BIN_DIR) $(APP)
	${Q_}$(CP) -f $(APP) $(DIAGS_BIN_DIR)

clean: api-clean playback_ip_lib_clean lib-clean clean-diags

api:  $(DIAGS_BIN_DIR) driver $(DIAGS_ODIR)
	@echo [Build ... Nexus]
	${Q_}$(MAKE) -C $(NEXUS_TOP)/lib/os INSTALL_DIR=$(DIAGS_BIN_DIR)
	${Q_}$(MAKE) -C $(NEXUS_TOP)/build  INSTALL_DIR=$(DIAGS_BIN_DIR)
	${Q_}$(CP) -f $(NEXUS_BIN_DIR)/lib*.so $(DIAGS_BIN_DIR)
ifeq  ($(NEXUS_MODE),proxy)
	@echo [Install ... KernelMode Driver]
	${Q_}$(CP)   $(NEXUS_BIN_DIR)/nexus.ko $(DIAGS_BIN_DIR)
endif

api-install: driver-install 
	@echo [Build ... Nexus]
	${Q_}$(MAKE) -C $(NEXUS_TOP)/lib/os NEXUS_BIN_DIR=$(STAGING_DIR) 
	${Q_}$(MAKE) -C $(NEXUS_TOP)/build  NEXUS_BIN_DIR=$(STAGING_DIR)
ifeq  ($(NEXUS_MODE),proxy)
	@echo [Install ... KernelMode Driver]
	${Q_}$(CP)   $(NEXUS_BIN_DIR)/nexus.ko $(STAGING_DIR)
endif


api-clean: driver-clean
	${Q_}$(MAKE) -C $(NEXUS_TOP)/lib/os clean
	${Q_}$(MAKE) -C $(NEXUS_TOP)/build clean
ifeq  ($(NEXUS_MODE),proxy)
	${Q_}$(RM) -f  $(DIAGS_BIN_DIR)/nexus.ko
endif

driver:
	@echo [Build ... Driver]
ifneq  ($(NEXUS_MODE),proxy)
	${Q_}$(MAKE) INSTALL_DIR=$(DIAGS_BIN_DIR) -C $(DRIVERDIR) install
endif

driver-install:
	@echo [Build ... Driver]
ifneq  ($(NEXUS_MODE),proxy)
	${Q_}$(MAKE) INSTALL_DIR=$(STAGING_DIR) -C $(DRIVERDIR) install
endif

driver-clean:
	${Q_}$(MAKE) -C $(DRIVERDIR) clean
	${Q_}$(RM) -f $(DIAGS_BIN_DIR)/bcmdriver.ko

################
######## Source Files for compiling
###############



##################
######## Diags objs
#################

APP_OBJECTS += \
	audiotest.o \
	bicaptest.o \
	colorbar.o \
	diags.o \
	i2ctest.o \
	irbtest.o \
	kirtest.o \
	kpdtest.o \
	ledtest.o \
	prompt.o \
	pwmtest2.o \
	rand.o \
	spitest2.o \
	timertest.o \
	uarttest.o \
	uhfrtest.o \
	upgtest.o

DIAGS_CFLAGS = \
	-DBCM_BOARD=$(NEXUS_PLATFORM) \
	-DDIAGS \
	-DLINUX_DIAGS

ifeq ($(B_REFSW_ARCH),arm-linux)
DIAGS_CFLAGS += -DBUILD_FOR_ARM
endif

ifeq ($(BUILD_FRONTEND_TEST), y)
	DIAGS_CFLAGS += \
		-DDIAGS_FRONTEND_TEST
	APP_OBJECTS += \
		frontend.o
endif

ifeq ($(BUILD_MISC_TEST), 1)
	DIAGS_CFLAGS += \
		-DDIAGS_MISC_TEST
	APP_OBJECTS += \
		misctest.o
endif

ifeq ($(BUILD_OFDM_TEST), 1)
	DIAGS_CFLAGS += \
		-DDIAGS_OFDM
	APP_OBJECTS += \
		ofdmtest.o
endif

ifeq ($(BUILD_PICQUAL_TEST), 1)
	DIAGS_CFLAGS += \
		-DDIAGS_PICQUAL
	APP_OBJECTS += \
		picqual.o
endif

ifeq ($(BUILD_QAM_TEST), 1)
	DIAGS_CFLAGS += \
		-DDIAGS_QAM
	APP_OBJECTS += \
		qamtest.o
endif

ifeq ($(BUILD_SAT_TEST), 1)
	DIAGS_CFLAGS += \
		-DDIAGS_SAT
	APP_OBJECTS += \
		sattest.o
endif

ifeq ($(BUILD_SATA_TEST), 1)
	DIAGS_CFLAGS += \
		-DDIAGS_SATA_SUPPORT
	ifeq ($(BUILD_SATA_TEST), 1)
		DIAGS_CFLAGS += \
			-DDIAGS_SATA_TEST \
			-DCFG_FAT32FS \
			-DCFG_FATFS \
			-DUSE_CACHE_FOR_SATA
		C_SRC += \
			sata.c
		ifeq ($(findstring $(BOARD_NUM), 97230 97231 97241 97346 97360 97422 97425 97429 97435 97445), $(BOARD_NUM))
			DIAGS_CFLAGS += -DDIAGS_SATA_AHCI
			C_SRC += ahci_test.c
		else
			DIAGS_CFLAGS += -DDIAGS_SATA
			C_SRC += atatest.c
		endif
	endif
endif

ifeq ($(BUILD_SDRAM_TEST), 1)
	APP_OBJECTS += sdramtest.o
endif

ifeq ($(BUILD_SOFTMODEM_TEST), 1)
	DIAGS_CFLAGS += \
		-DDIAGS_SOFTMODEM_TEST
	APP_OBJECTS += \
		smtest.o
endif

ifeq ($(BUILD_UART), A)
	DIAGS_CFLAGS += -DDIAGS_UART_A
endif
ifeq ($(BUILD_UART), B)
	DIAGS_CFLAGS += -DDIAGS_UART_B
endif
ifeq ($(BUILD_UART), C)
	DIAGS_CFLAGS += -DDIAGS_UART_C
endif

ifeq ($(BUILD_UART_TEST), 1)
	APP_OBJECTS += \
		uarttest.o
endif

ifeq ($(BUILD_WHETSTONE_TEST), 1)
	DIAGS_CFLAGS += \
		-DDIAGS_WHETSTONE_TEST
	APP_OBJECTS += \
		whets.o
endif

ifeq ($(BUILD_HDMI_TEST), 1)
	DIAGS_CFLAGS += \
		-DDIAGS_HDMI_TEST
	APP_OBJECTS += hdmi.o \
		hdmitest.o
endif

#ifeq ($(BUILD_HDMI_CEC_TEST), 1)
#	DIAGS_CFLAGS += -DDIAGS_HDMI_CEC_TEST
#	APP_OBJECTS += hdmitest_output_cec.o
#endif

# Expand Magnum Variables
include ../../../../magnum/commonutils/box/bbox.inc
include ../../../../magnum/basemodules/int/bint.inc
include ../../../../magnum/basemodules/mem/bmem.inc
include ../../../../magnum/basemodules/mma/bmma.inc
include ../../../../magnum/basemodules/tmr/btmr.inc
include ../../../../magnum/commonutils/avc/bavc.inc
include ../../../../magnum/commonutils/fmt/bfmt.inc
include ../../../../magnum/commonutils/mrc/bmrc.inc
include ../../../../magnum/commonutils/pxl/bpxl.inc
include ../../../../magnum/commonutils/sur/bsur.inc

include ../../../../magnum/portinginterface/bcp/bbcp.inc
include ../../../../magnum/portinginterface/icp/bicp.inc
include ../../../../magnum/portinginterface/irb/birb.inc
include ../../../../magnum/portinginterface/led/bled.inc
include ../../../../magnum/portinginterface/kir/bkir.inc
include ../../../../magnum/portinginterface/pwm/bpwm.inc
include ../../../../magnum/portinginterface/spi/bspi.inc
include ../../../../magnum/portinginterface/uhf/buhf.inc

MAGNUM_INCLUDES := $(sort $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES)))
# Don't compile magnum modules that are accesses using proxy interface
MAGNUM_SOURCES := $(patsubst %.o,%.c,$(sort $(foreach module,$(filter-out ${MAGNUM_PROXY_MODULES},${MAGNUM_MODULES}), $($(module)_SOURCES))))
MAGNUM_DEFINES := $(sort $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))
MAGNUM_CFLAGS := $(addprefix -D,$(MAGNUM_DEFINES)) $(addprefix -I,$(MAGNUM_INCLUDES))

#$(warning .... MAGNUM_MODULES=$(MAGNUM_MODULES))
#$(warning .... MAGNUM_DEFINES=$(MAGNUM_DEFINES))
#$(warning .... MAGNUM_INCLUDES=$(MAGNUM_INCLUDES))

#NEXUS_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))
#NEXUS_CFLAGS := $(addprefix -D,$(NEXUS_DEFINES)) $(addprefix -I,$(NEXUS_INCLUDES))
#$(warning .... NEXUS_DEFINES=$(NEXUS_DEFINES))

CFLAGS += \
	$(DIAGS_CFLAGS) \
	$(MAGNUM_CFLAGS) \
	-I../../../../nexus/base/include \
	-I../../../../nexus/base/include/linuxuser

#	$(NEXUS_CFLAGS) \

#####
### Diags Dirs
#####
DIAGS_DIRS += ../src 
DIAGS_DIRS += ../src/mvc
DIAGS_DIRS += ../src/widgets
DIAGS_DIRS += ../src/platform
DIAGS_DIRS += ../src/views
DIAGS_DIRS += ../src/ocap

#################
##### PSI SCAN
#################
ifneq ($(PSISCAN_SUPPORT),n)
	CFLAGS += -DPSISCAN_SUPPORT
	TRANSPORTDECODE_DIR=$(BSEAV)/lib/mpeg2_ts_parse
	TSPSI_DIR=$(BSEAV)/lib/tspsi2
	DIAGS_DIRS += \
		$(TRANSPORTDECODE_DIR) \
		$(TSPSI_DIR)
	# all other tspsi and mpeg2_ts_parse are compiled in with nexus
	APP_OBJECTS += tspsimgr2.o
endif

DIAGS_OBJS := $(addprefix $(DIAGS_ODIR)/,$(APP_OBJECTS))

# For linux builds, link to the correct libraries
ifeq ($(B_REFSW_OS),linuxkernel)
LDFLAGS += ${NEXUS_LDFLAGS}
else
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS += -lnexus -L${NEXUS_BIN_DIR} -lpthread -lm  $(EXTRA_DRM_OBJS)
endif
endif

###############
####### Lua
###############
LUA_SUPPORT=n
ifneq ($(LUA_SUPPORT),n)
	APP_OBJECTS += atlas_lua.o
	LUA_DIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/lua/lua-5.1.5
	CFLAGS += -DLUA_SUPPORT -I$(LUA_DIR)/include
	LIBS += $(LUA_DIR)/lib/liblua.a

    LINENOISE_DIR = $(BSEAV)/app/atlas/src/linenoise/antirez-linenoise-02d7935
    CFLAGS += -DLINENOISE_SUPPORT -I$(LINENOISE_DIR)
    VPATH += $(LINENOISE_DIR)
    APP_OBJECTS += linenoise.o
endif

###############
###### BWIN Widgets
###############
MLIBS=$(BSEAV)/lib/mlibs
CXXFLAGS += -I$(MLIBS)
CFLAGS += -I$(MLIBS)

# DEBUG SUFFIX Defined above
MLIBS_ODIR=$(B_REFSW_OBJ_ROOT)/BSEAV/lib/mlibs/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)

CFLAGS += -I$(MLIBS) -L$(MLIBS_ODIR)
LDFLAGS += -L$(MLIBS_ODIR) -lmcommon -lmnet

BWIN=$(BSEAV)/lib/bwin
include $(BWIN)/include/bwin.mak
CXXFLAGS += $(BWIN_CFLAGS) -DBWIN_SUPPORT
LDFLAGS += $(BWIN_LDFLAGS)
CFLAGS += -DBWIN_SUPPORT

VPATH += $(BWIDGETS_DIR)/src
include $(BSEAV)/lib/bwidgets/include/bwidgets.mak
CFLAGS += $(BWIDGETS_CFLAGS)
LDFLAGS += $(BWIDGETS_LDFLAGS)


###############
###### ASF AVI ETC MEDIA PROBE common Library
###############

# check to see if libraries exist, they should by default
ifeq ($(MEDIA_ASF_SUPPORT),)
ASF_SUPPORT= $(shell test -e ${BSEAV}/lib/asf && echo y)
else
ASF_SUPPORT=$(MEDIA_ASF_SUPPORT)
endif

ifeq ($(MEDIA_AVI_SUPPORT),)
AVI_SUPPORT= $(shell test -e ${BSEAV}/lib/avi && echo y)
else
AVI_SUPPORT=$(MEDIA_AVI_SUPPORT)
endif

ifeq ($(ASF_SUPPORT),y)
	include ${BSEAV}/lib/asf/basf.inc
endif
ifeq ($(AVI_SUPPORT),y)
	include ${BSEAV}/lib/avi/bavi.inc
endif
	include ${BSEAV}/../magnum/commonutils/vlc/bvlc.inc
	include ${BSEAV}/lib/media/bmedia.inc
        include ${BSEAV}/lib/utils/batom.inc
        include ${BSEAV}/lib/bfile/bfile.inc
ifneq ($(MEDIA_RMFF_SUPPORT),n)
        -include ${BSEAV}/lib/media/rmff/brmff.inc
endif
ifneq ($(MEDIA_FLV_SUPPORT),n)
        -include ${BSEAV}/lib/media/flv/bflv.inc
endif
        include ${BSEAV}/lib/bfile/bfile.inc
        DIAGS_MODULES := BMEDIA_PROBE BATOM BFILE_STDIO BVLC
	CFLAGS += -DPVR_SUPPORT

	# Direct bcmplayer access is needed for Video::indexToTime.
	CFLAGS += -I$(BSEAV)/lib/bcmplayer/include

	B_DIAGS_INC := $(addprefix -I,$(sort $(foreach module, $(DIAGS_MODULES), $($(module)_INCLUDES))))
	B_DIAGS_CFLAGS := $(addprefix -D,$(sort $(foreach module, $(DIAGS_MODULES), $($(module)_DEFINES))))
	B_DIAGS_SOURCES = $(sort $(foreach module, $(DIAGS_MODULES), $($(module)_SOURCES)))
	B_DIAGS_OBJS := $(patsubst %.c,%.o,$(notdir ${B_DIAGS_SOURCES}))
	vpath %c $(dir ${B_DIAGS_SOURCES})

	APP_OBJECTS += ${B_DIAGS_OBJS}
	CFLAGS += ${B_DIAGS_CFLAGS} ${B_DIAGS_INC}



# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Always build with debug
CFLAGS += -MD -g
CFLAGS += -DBDBG_DEBUG_BUILD
CFLAGS += -W -MMD -pedantic
LDFLAGS += -ldl -lpthread

DIAGS_INCLUDE_DIRS = $(addprefix -I,$(DIAGS_DIRS))
VPATH += $(DIAGS_DIRS)

####
## Include the B_OS abstraction layer
####
DIAGS_INCLUDE_DIRS += -I$(BSEAV)/../nexus/lib/os/include
DIAGS_INCLUDE_DIRS += -I$(BSEAV)/../nexus/lib/os/include/linuxuser

##########
#### Link nexus libraries
##########
LDFLAGS += -lnexus -lb_os
CFLAGS += $(DIAGS_INCLUDE_DIRS) 

C_ONLY_FLAGS := -std=c99 -std=c89 -Wstrict-prototypes

CXXFLAGS += -fno-rtti -fno-exceptions 

vpath %.cpp $(VPATH)
vpath %.c $(VPATH)

# Implicit rule for building local app
$(APP): api lib $(DIAGS_ODIR) build_date $(addprefix $(DIAGS_ODIR)/,$(APP_OBJECTS)) $(LIBS)
	@echo [Linking... $(notdir $@)]
	$(Q_)$(CXX) -o $@ $(addprefix $(DIAGS_ODIR)/,$(APP_OBJECTS)) $(filter-out -lsettop,$(LDFLAGS)) $(LIBS)

$(DIAGS_LIB): $(addprefix $(DIAGS_ODIR)/,$(APP_OBJECTS))
	@echo [Linking... $(notdir $@)]
	${Q_}$(RM) $@
	${Q_}$(AR) cr $@ $^
	${Q_}$(RANLIB) $@


%.o: %.cpp
	@echo [Compile... $(notdir $<)]
	${Q_}$(CXX) $(CXXFLAGS) $(filter-out $(C_ONLY_FLAGS),$(CFLAGS)) -c $< -o $(DIAGS_ODIR)/$@

%.o: %.c
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) $(CFLAGS) -c $< -o $(DIAGS_ODIR)/$@

$(DIAGS_ODIR)/%.o: %.c
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS) $(C_ONLY_FLAGS)

$(DIAGS_ODIR)/%.o: %.cpp
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CXX) -c -o $@ $< $(filter-out $(C_ONLY_FLAGS),$(CFLAGS)) $(CXXFLAGS)


###############
######### Playback IP and Releated libraries
###############
ifeq ($(PLAYBACK_IP_SUPPORT),y)
playback_ip_lib: api
	${Q_}$(MAKE) REFSW_BUILD=y -C $(NEXUS_TOP)/lib/playback_ip/apps INSTALL_DIR=$(DIAGS_BIN_DIR) install

playback_ip_lib_install: api
	${Q_}$(MAKE) REFSW_BUILD=y -C $(NEXUS_TOP)/lib/playback_ip/apps INSTALL_DIR=$(STAGING_DIR) install

playback_ip_lib_clean:
	${Q_}$(MAKE) REFSW_BUILD=y -C $(NEXUS_TOP)/lib/playback_ip/apps clean

playback_ip_lib_min_clean:
	@echo [..... Minimal Clean ....]
	@echo [..... Not Cleaning SSL ....]
	@echo [..... When Switching Endianness please do a \"make clean\" ....]
	${Q_}$(MAKE) REFSW_BUILD=y -C $(NEXUS_TOP)/lib/playback_ip/ clean

else #PLAYBACK_IP_SUPPORT is not defined, so define empty targets
playback_ip_lib:
#intentionally left empty

playback_ip_lib_install:
#intentionally left empty

playback_ip_lib_clean:
#intentionally left empty

playback_ip_lib_min_clean:
	@echo [..... Minimal Clean ....]

endif # PLAYBACK_IP_SUPPORT

#Only Cleans Playback-IP library not SSL, only
min-clean: playback_ip_lib_min_clean clean-diags api-clean
	@echo [..... Minimal Cleaning Done....]

#
# This is the main installation rule.
# It will build the binaries and utilities, and then
# perform the actual installation procedure.
#
install:  bin_install $(STAGING_DIR) install-binaries make-tarball

#
# Install drivers, libraries and apps
#
.PHONY: install-binaries
install-binaries: api-install playback_ip_lib_install install-config-files

#
# STAGING_DIR is phony because we always want to remove it and
# recreate it.
#
.PHONY: $(STAGING_DIR)
$(STAGING_DIR):
	@$(RM) -rf $(STAGING_DIR)
	@${MKDIR} $(STAGING_DIR)

#
# Copy everything into temporary STAGING_DIR and tar it up.
# The final tarball is left in DIAGS_BIN_DIR.
#

make-tarball: install-support-files
	@echo [Build..... tarball]
	@(cd $(STAGING_DIR);tar -pzckf $(DIAGS_BIN_DIR)/${DIAGS_TAR_BALL} .)
	@$(RM) -rf $(STAGING_DIR)
	@echo ""
	@echo "*********************************************"
	@echo "* Installation tarball created in $(DIAGS_BIN_DIR)."
	@echo "*"
	@echo "* To complete installation perform the"
	@echo "* following operations on the settop box:"
	@echo "*"
	@echo "* gunzip -c ${DIAGS_TAR_BALL} | tar xvpf -"
	@echo "* start diags"
	@echo "*********************************************"
	@echo ""

#
# Install drivers, libraries and apps
#
.PHONY: install-support-files
install-support-files:
	@echo [Install... scripts]
	$(Q_)$(CP) start $(STAGING_DIR)
	$(Q_)chmod +x $(STAGING_DIR)/start
ifeq ($(NEXUS_MODE),proxy)
	$(Q_)$(CP) $(NEXUS_BIN_DIR)/nexus.ko $(STAGING_DIR)
endif
	$(Q_)$(CP) $(NEXUS_BIN_DIR)/logger $(STAGING_DIR)
	$(Q_)$(CP) $(NEXUS_BIN_DIR)/nexus $(STAGING_DIR)
	$(Q_)chmod +x $(STAGING_DIR)/nexus

	$(Q_)$(CP) $(APP) $(STAGING_DIR)

#	@echo [Install... fonts]
#	@${MKDIR} $(STAGING_DIR)/fonts
#	@$(CP) $(PRERENDERED_FONTS) $(STAGING_DIR)/fonts

####
## install Config Files
####
install-config-files:

bin-install-config-files:
# Copy over config files so we can run without doing a make install
# you can run this manually with "make bin-install-config-files" 

-include $(DIAGS_ODIR)/*.d
