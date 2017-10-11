# GNUmakefile : build all brands and target types (debug or release)
# of the wireless 802.11 driver
#
# $Id$
#

WLAN_ComponentsInUse := bcmwifi clm-api ppr keymgmt iocv dump phy ndis dongle xp chctx
include ../../makefiles/WLAN_Common.mk
SRCBASE := $(WLAN_SrcBaseR)
COMPONENTS_SRCBASE := $(SRCBASE)/../components

#DNGL_IMAGE_NAME ?= 4322/roml-ndis-xp-noreclaim-noccx-mfgtest-g
#DNGL_IMAGE_NAME ?= 4325b0/sdio-g-cdc-reclaim-ndis
#DNGL_IMAGE_NAME ?= 43239a0-roml/sdio-ag-mfgtest-seqcmds
DNGL_IMAGE_NAME ?= 4335c0-ram/sdio-ag-mfgtest-seqcmds-ndis
DNGL_IMAGE_PATH := $(SRCBASE)/../build/dongle/$(DNGL_IMAGE_NAME)
DRIVER_NAME     := bcmsddhd.sys

#the only valid value is 7600
WDK_VER = 7600
REQUIRE_WDM7600=1

# Use DDK instead of WDK to build this module
USEDDK=1

include $(SRCBASE)/makefiles/env.mk

	DDK_OS=wxp
WDMDDK=$(WDM7600WDK)
WDMDDK.UNC=$(WDM7600WDK.UNC)
NEWPATH += $(WDMDDK.UNC)/bin/amd64 $(WDMDDK.UNC)/bin/x86/x86 $(WDMDDK.UNC)/bin/

BASEDIR=$(WDMDDK)
# ObjPfx can be overriden on command line
ObjPfx ?= obj

# If ObjPfx is set to anything other than obj, redirect pdb files
# to unique identified by unique ObjPfx for build concurrency
# If ObjPfx is not custom set, the default pdb is created as vcxx.pdb
ifneq ($(ObjPfx),obj)
  PDBNAME ?= $(subst /,_,$(ObjPfx))
  PDBARG  ?= /Fd$(PDBNAME)
endif

# env defined NEWPATH...
empty:=
space:= $(empty) $(empty)
export PATH:=$(subst $(space),:,$(strip $(NEWPATH))):$(PATH)

CC  = cl
RC  = rc
LD  = link
LIBCMD = lib

WLCFGDIR	:= $(SRCBASE)/wl/config
CONFIG_WL_CONF	:= wlconfig_win_wl_sdstd
WLNDIS=1
OsLNDIS=1
include    $(WLCFGDIR)/$(CONFIG_WL_CONF)
include    $(WLCFGDIR)/wl.mk

SRCDIRS := $(addprefix $(SRCBASE)/,dhd/sys bcmsdio/sys)
SRCDIRS += $(addprefix $(COMPONENTS_SRCBASE)/,drivers/wl/shim/src)
SRCDIRS += $(WLAN_StdSrcDirsR) $(WLAN_ComponentSrcDirsR)

vpath %.c $(SRCDIRS)
vpath %.rc $(SRCDIRS)

WLFILES = dhd_cdc.c dhd_wlfc.c dhd_sdio.c dhd_common.c dhd_ip.c dhd_ndis5.c

# NDIS layer files
WLFILES += wl_ndis5.c ndshared5.c wl_ndconfig5.c ndis_osl.c ndiserrmap.c wl_ndvif.c wl_oideml5.c wl_oidcmn5.c

# WL Shim component files
WLFILES += wl_shim.c wl_shim_nodes_arr.c wl_shim_node_default.c wl_shim_node_2.c wl_shim_node_4.c

# Other files
WLFILES += bcmevent.c bcmutils.c bcmxtlv.c bcm_app_utils.c hnd_pktq.c hnd_pktpool.c siutils.c sbutils.c aiutils.c \
	hndpmu.c bcmwifi_channels.c bcmstdlib.c hnd_pktq.c hnd_pktpool.c wlc_alloc.c

# SDIOH layer files for standard host controller
WLFILES += bcmsdh.c bcmsdstd.c bcmsdstd_ndis.c bcmsdh_ndis.c

WLFLAGS += -DBCMDBG_MEM -DSDTEST -DBCMPERFSTATS
WLFLAGS += -DBDC -DSHOW_EVENTS -DBCMSDIO -DBCMSDIOH_STD -DBCMDONGLEHOST -DSTA -DBCMSUP_PSK -DND_ALL_PASSIVE
WLFLAGS += -DWL_WLC_SHIM -DWLVIF
WLFLAGS += -DWL_WLC_SHIM_EVENTS
# WLFLAGS += -DBISON_SHIM_PATCH -DCARIBOU_SHIM_PATCH

#-DBCMSLTGT -DBCMQT -DTESTDONGLE

DHDFLAGS = -DDHD_DEBUG -DWL_IW_USE_ISCAN -DWLCNT -DWL11N

ifdef RWL_DONGLE
WLFILES += wlc_rwl.c
endif

ifneq ($(DNGL_IMAGE_NAME),FakeDongle)
ifneq ($(DNGL_IMAGE_NAME),)
DHDFLAGS += -DBCMEMBEDIMAGE="\"$(DNGL_IMAGE_PATH)/rtecdc.h\""
endif
endif

ifdef BRAND
C_DEFINES += -D$(BRAND)
endif


C_DEFINES += -DNTDEBUG=ntstd -DNTDEBUG_TYPE=windbg -DTARGETENV_win32 -DNDIS -DNDIS_MINIPORT_DRIVER -DNDIS_WDM -DWDM -DWLNDIS -DBCMDRIVER -DNDIS50_MINIPORT -Dwin_external_wl -D_X86_=1 -Di386=1  -DSTD_CALL -DCONDITION_HANDLING=1   -DNT_INST=0 -DWIN32=100 -D_NT1X_=100 -DWINNT=1 -D_WIN32_WINNT=0x0500 /DWINVER=0x0500 -D_WIN32_IE=0x0600 -DWIN32_LEAN_AND_MEAN=1 -DDEVL=1 -D__BUILDMACHINE__=WinDDK -D_DLL=1  -DNDIS_DMAWAR 
C_DEFINES += $(WLFLAGS) $(DHDFLAGS)

ifdef WLTEST
  C_DEFINES += -DWLTEST -DDHD_SPROM -DIOCTL_RESP_TIMEOUT=20000
  WLFILES += bcmsrom.c bcmotp.c
endif

C_DEFINES64 += -D_WIN64 -D_AMD64_ -DAMD64 -DCONDITION_HANDLING=1 -DNT_INST=0 -DWIN32=100 -D_NT1X_=100 -DWINNT=1 -D_WIN32_WINNT=0x0502 /DWINVER=0x0502 -D_WIN32_IE=0x0600 -DWIN32_LEAN_AND_MEAN=1 -D_AMD64_SIMULATOR_PERF_ -D_SKIP_IF_SIMULATOR_ -D_AMD64_SIMULATOR_ -D_AMD64_WORKAROUND_ -DDEVL=1 -D__BUILDMACHINE__=WinDDK -D_DLL=1 -DBUILD_W2K=0
C_DEFINES64 += -DTARGETENV_win32 -DNDIS -DNDIS_MINIPORT_DRIVER
C_DEFINES64 += -DWDM -DNDIS_WDM -DNDIS_DMAWAR
C_DEFINES64 += -DWLNDIS -DBCMDRIVER
C_DEFINES64 += $(WLFLAGS) $(DHDFLAGS)

#ifdef BCMCCX
#C_DEFINES += -DBCMCCX -DBCMSUP_PSK
#endif /* BCMCCX */

TARGETLIBS=$(DDK_LIB_PATH)/ndis.lib

INCLUDES=i386;$(SRCBASE);$(subst $(space),;,$(WLAN_ComponentIncDirsR) $(WLAN_IncDirsR));$(SRCBASE)/dongle;$(SRCBASE)/dhd/sys;$(COMPONENTS_SRCBASE)/drivers/wl/shim/include;
INCLUDES+=$(BASEDIR)/inc;$(BASEDIR)/inc/ddk/wdm;$(BASEDIR)/inc/ddk;$(TARGETPATH);$(WDMDDK)/inc/$(DDK_OS);$(WDMDDK)/inc/ddk/$(DDK_OS);$(WDMDDK)/inc/ddk/wdm/$(DDK_OS);$(WDMDDK)/inc/crt;$(WDMDDK)/inc/api;

INCLUDES64=$(subst $(space),;,$(WLAN_ComponentIncDirsR) $(WLAN_IncDirsR));$(SRCBASE)/dongle
INCLUDES64+=amd64;.;$(WDMDDK)/inc/mfc42;..;$(WDMDDK)/inc/$(DDK_OS);$(WDMDDK)/inc/ddk/$(DDK_OS);$(WDMDDK)/inc/ddk/wdm/$(DDK_OS);$(WDMDDK)/inc/crt;

CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(INCLUDES)))
CPPFLAGS64 = $(patsubst %,-I%,$(subst ;, ,$(INCLUDES64)))

# windows only resource file
WLSRCS = wl.rc

# file list from wlconfig's wl.mk
WLSRCS += $(WLFILES)
# remove duplicate, otherwise windows build will barf
SOURCES   = $(sort $(WLSRCS))

ifneq ($(BCMNVRAMR),1)
SOURCES	+= nvramstubs.c
endif

all:: dngl_image checked free
#all:: dngl_image obj/i386/checked/$(DRIVER_NAME) 
#all:: obj/i386/ndis51/checked/bcmwl51.sys obj/i386/ndis51/free/bcmwl51.sys 
#all:: obj/i386/se/checked/$(DRIVER_NAME) obj/i386/se/free/$(DRIVER_NAME) 
#all:: obj/amd64/checked/bcmwl564.sys obj/amd64/free/bcmwl564.sys 
#all:: obj/i386/free/$(DRIVER_NAME)

# Discrete targets to launch only checked or free targets
checked:: dngl_image $(ObjPfx)/i386/checked/$(DRIVER_NAME)
free:: dngl_image $(ObjPfx)/i386/free/$(DRIVER_NAME)

dngl_image:
ifneq ($(DNGL_IMAGE_NAME),FakeDongle)
ifneq ($(DNGL_IMAGE_NAME),)
	@[ -f $(DNGL_IMAGE_PATH)/rtecdc.h ] || \
		{ echo "ERROR: $(DNGL_IMAGE_NAME) is not built"; exit 1; }
endif
endif

# Make debug target, just print the path
path:
	echo "PATH = $(PATH)"

SOURCES.RES	= $(filter %.res,$(patsubst %.rc,%.res,$(SOURCES)))
SOURCES.OBJ	= $(patsubst %.c,%.obj,$(filter %.c,$(SOURCES)))
AIROSOURCES.OBJ	= $(patsubst %.c,%.obj,$(filter %.c,$(AIROSOURCES)))

LINKFLAGS =  -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -SECTION:INIT,d \
		  	-OPT:REF -OPT:ICF \
		  	-IGNORE:4010,4037,4039,4065,4070,4078,4087,4089,4198,4221 \
		  	-INCREMENTAL:NO -FULLBUILD /release -NODEFAULTLIB /WX \
		  	-debug:FULL -debugtype:cv -version:5.1 -osversion:5.1 \
		  	-STACK:0x40000,0x1000 -driver -base:0x10000 \
		  	-align:0x80 -subsystem:native,5.1 -entry:DriverEntry@8 \
		  	-map /MACHINE:IX86

LINKFLAGS64 = -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -SECTION:INIT,d \
		  	-OPT:REF -OPT:ICF \
		  	-IGNORE:4010,4037,4039,4065,4070,4078,4087,4089,4198,4221 \
		  	-INCREMENTAL:NO -FULLBUILD /release -NODEFAULTLIB /WX \
		  	-debug:FULL -debugtype:cv -version:5.2 -osversion:5.2 \
		  	-STACK:0x40000,0x1000 -driver -base:0x10000 \
		  	-subsystem:native,5.02 -entry:DriverEntry \
		  	-map -machine:amd64

%/$(DRIVER_NAME) : DDK_OS=wxp
%/bcmwl51.sys : DDK_OS=wxp

LIBS  = $(WDMDDK)/lib/$(DDK_OS)/i386/ntoskrnl.lib 
LIBS += $(WDMDDK)/lib/$(DDK_OS)/i386/hal.lib 
LIBS += $(WDMDDK)/lib/$(DDK_OS)/i386/wmilib.lib 
LIBS += $(WDMDDK)/lib/$(DDK_OS)/i386/ndis.lib 
LIBS += $(WDMDDK)/lib/$(DDK_OS)/i386/bufferoverflowk.lib

LIBS64  = $(WDMDDK)/lib/$(DDK_OS)/amd64/ntoskrnl.lib 
LIBS64 += $(WDMDDK)/lib/$(DDK_OS)/amd64/hal.lib 
LIBS64 += $(WDMDDK)/lib/$(DDK_OS)/amd64/wmilib.lib 
LIBS64 += $(WDMDDK)/lib/$(DDK_OS)/amd64/ndis.lib 

$(ObjPfx)/i386/checked/$(DRIVER_NAME) :	$(ObjPfx)/i386/checked/NUL \
				$(ObjPfx)/i386/checked/wlconf.h \
				$(addprefix $(ObjPfx)/i386/checked/,$(SOURCES.OBJ)) 
	@echo "Linking Driver - $@"
	$(LD) $(LINKFLAGS) $(filter %.res %.obj,$^) $(LIBS) -OUT:$@

$(ObjPfx)/i386/free/$(DRIVER_NAME) :      $(ObjPfx)/i386/free/NUL \
				$(ObjPfx)/i386/free/wlconf.h \
				$(addprefix $(ObjPfx)/i386/free/,$(SOURCES.OBJ)) 
	@echo "Linking Driver - $@"
	$(LD) $(LINKFLAGS) $(filter %.res %.obj,$^) $(LIBS) -OUT:$@ 

$(ObjPfx)/i386/se/checked/$(DRIVER_NAME) :$(ObjPfx)/i386/se/checked/NUL \
				$(ObjPfx)/i386/checked/wlconf.h \
				$(addprefix $(ObjPfx)/i386/checked/,$(SOURCES.OBJ)) 
	@echo "Linking Driver - $@"
	$(LD) $(LINKFLAGS) $(filter %.res %.obj,$^) $(LIBS) -OUT:$@


$(ObjPfx)/i386/se/free/$(DRIVER_NAME) :	$(ObjPfx)/i386/se/free/NUL \
				$(ObjPfx)/i386/free/wlconf.h \
				$(addprefix $(ObjPfx)/i386/free/,$(SOURCES.OBJ)) 
	@echo "Linking Driver - $@"
	$(LD) $(LINKFLAGS) $(filter %.res %.obj,$^) $(LIBS) -OUT:$@ 

$(ObjPfx)/i386/ndis51/checked/bcmwl51.sys : $(ObjPfx)/i386/ndis51/checked/NUL \
				$(ObjPfx)/i386/ndis51/checked/wlconf.h \
				$(addprefix $(ObjPfx)/i386/ndis51/checked/,$(SOURCES.OBJ)) 
	@echo "Linking Driver - $@"
	$(LD) $(LINKFLAGS) $(filter %.res %.obj,$^) $(LIBS) -OUT:$@

$(ObjPfx)/i386/ndis51/free/bcmwl51.sys: $(ObjPfx)/i386/ndis51/free/NUL \
				$(ObjPfx)/i386/ndis51/free/wlconf.h \
				$(addprefix $(ObjPfx)/i386/ndis51/free/,$(SOURCES.OBJ)) 
	@echo "Linking Driver - $@"
	$(LD) $(LINKFLAGS) $(filter %.res %.obj,$^) $(LIBS) -OUT:$@

$(ObjPfx)/amd64/checked/bcmwl564.sys	: $(ObjPfx)/amd64/checked/NUL \
				$(ObjPfx)/amd64/checked/wlconf.h \
				$(addprefix $(ObjPfx)/amd64/checked/,$(SOURCES.OBJ)) 
	@echo "Linking Driver - $@"
	$(LD) $(LINKFLAGS64) $(filter %.res %.obj,$^) $(LIBS64) -OUT:$@

$(ObjPfx)/amd64/free/bcmwl564.sys	: $(ObjPfx)/amd64/free/NUL \
				$(ObjPfx)/amd64/free/wlconf.h \
				$(addprefix $(ObjPfx)/amd64/free/,$(SOURCES.OBJ)) 
	@echo "Linking Driver - $@"
	$(LD) $(LINKFLAGS64) $(filter %.res %.obj,$^) $(LIBS64) -OUT:$@


ifneq ($(DNGL_IMAGE_NAME),FakeDongle)
ifneq ($(DNGL_IMAGE_NAME),)
$(ObjPfx)/i386/checked/dhd_sdio.obj: $(DNGL_IMAGE_PATH)/rtecdc.h
$(ObjPfx)/i386/free/dhd_sdio.obj: $(DNGL_IMAGE_PATH)/rtecdc.h
endif
endif



CFLAGS_DBG = -nologo /c /Zp8 /Gy -cbstring /W3 /WX /Gz \
			 /EHsc- /Gm- /GR- /GF -Zi \
			 -FI$(WDMDDK)/inc/api/warning.h  \
			 /Od /Oi /Oy- 

CFLAGS_OPT = -nologo /c /Zp8 /Gy -cbstring /W3 /WX /Gz \
			 /EHsc- /Gm- /GR- /GF -Z7 \
			 -FI$(WDMDDK)/inc/api/warning.h  \
			 /Oxs /Oy  

CFLAGS_DBG64 = -nologo -c /Zp8 /Gy /Gi- -cbstring /W3 /Wp64 /WX \
			/GX- /GR- /GF -Z7 /Od /Oi -d2home \
			-FI$(WDMDDK)/inc/api/warning.h \
			/d1noWchar_t

CFLAGS_OPT64 = -nologo -c /Zp8 /Gy /Gi- -cbstring /W3 /Wp64 /WX \
			/GX- /GR- /GF -Z7 /Oxt \
			-FI$(WDMDDK)/inc/api/warning.h \
			/d1noWchar_t

obj/i386/checked/%.obj : obj/i386/checked/NUL %.c
	$(CC) -c $(CFLAGS_DBG) -DNDIS50_MINIPORT -DNDIS50 $(C_DEFINES) -I"$(@D)" $(CPPFLAGS) -DDEBUG  -DDBG=1 -DFPO=0 -DBCMDBG -Fo"$(@D)/"  $(filter %.c,$^)

obj/i386/free/%.obj : obj/i386/free/NUL %.c
	$(CC) -c $(CFLAGS_OPT) -DNDIS50_MINIPORT -DNDIS50 $(C_DEFINES) -I"$(@D)" $(CPPFLAGS) -DNDEBUG -DFPO=1 -Fo"$(@D)/" $(filter %.c,$^)

obj/i386/ndis51/checked/%.obj : obj/i386/ndis51/checked/NUL %.c
	$(CC) -c $(CFLAGS_DBG) -DNDIS51_MINIPORT -DNDIS51 $(C_DEFINES) -I"$(@D)" $(CPPFLAGS) -DDEBUG  -DDBG=1 -DFPO=0 -DBCMDBG -Fo"$(@D)/" $(filter %.c,$^)

obj/i386/ndis51/free/%.obj : obj/i386/ndis51/free/NUL %.c
	$(CC) -c $(CFLAGS_OPT) -DNDIS51_MINIPORT -DNDIS51 $(C_DEFINES) -I"$(@D)" $(CPPFLAGS) -DNDEBUG -DFPO=1 -Fo"$(@D)/" $(filter %.c,$^)

obj/amd64/checked/%.obj : obj/amd64/checked/NUL %.c
	$(CC) -c $(CPPFLAGS64) -DNDIS51_MINIPORT -DNDIS51 $(C_DEFINES64) -I"$(@D)" -DDBG=1 -DBCMDBG $(CFLAGS_DBG64) -Fo"$(@D)/" $(filter %.c,$^)

obj/amd64/free/%.obj : obj/amd64/free/NUL %.c
	$(CC) -c $(CPPFLAGS64) -DNDIS51_MINIPORT -DNDIS51 $(C_DEFINES64) -I"$(@D)" -DNDEBUG $(CFLAGS_OPT64) -Fo"$(@D)/" $(filter %.c,$^)

OEM_LIST=bcm se

$(foreach oem,$(OEM_LIST), oem/$(oem)/wl.res) : \
	oem/%/wl.res : wl.rc oem/%/OEMDefs.h
	@echo "Compiling - $(notdir $(filter %.rc,$<))"
	$(RC) -r $(RCFLAGS) $(C_DEFINES) \
		/I "oem/$*" \
		/I "$(SRCBASE)/include" \
		/I "$(WDMDDK)/inc/$(DDK_OS)" \
		/I "$(MSSDK)/include" \
		/I "$(MSDEV)/VC98/INCLUDE" \
		-fo"$@" $(filter %.rc,$<)


$(foreach oem,$(OEM_LIST), oem/$(oem)/OEMDefs.h) : \
	oem/%/OEMDefs.h : oem/%/NUL $(COMPONENTS_SRCBASE)/shared/resources/locale/english/%.txt
	cat $^ | egrep 'STR_OEM_' | sed 's/STR_\(.*\)L\"/#define \1\"/' > $@

#
# Utility target to create target directories on demand.
#
%/NUL :
	mkdir -p $(@D)

%/wlconf.h : $(SRCBASE)/wl/config/wltunable_sample.h
	cp $^ $@

clean : 
	rm -rf $(ObjPfx)/i386/checked/*.obj
	rm -rf $(ObjPfx)/i386/free/*.obj
	rm -rf $(ObjPfx)/i386/ndis51/checked/*.obj
	rm -rf $(ObjPfx)/i386/ndis51/free/*.obj
	rm -rf $(ObjPfx)/amd64/checked/*.obj
	rm -rf $(ObjPfx)/amd64/free/*.obj
	rm -rf oem/*/*

clobber : 
	rm -rf $(ObjPfx)/
	rm -rf oem
