WLAN_ComponentsInUse := bcmwifi
include ../../../makefiles/WLAN_Common.mk
BRCMBASE := $(WLAN_SrcBaseR)

BLDTYPE := release
#BLDTYPE = debug
CC = gcc
export $(CC)


ifeq ($(BLDTYPE),debug)
CFLAGS = -Wall -Wnested-externs -g -D_TUDEBUGTRACE -DWPS_WIRELESS_ENROLLEE -DDEBUG
CXXFLAGS = -Wall -Wnested-externs -g -D_TUDEBUGTRACE -DWPS_WIRELESS_ENROLLEE
else
CFLAGS = -Wall -Os -Wnested-externs -DWPS_WIRELESS_ENROLLEE
endif

CFLAGS += -DBCMWPA2



ifeq ($(CC), arm-linux-gcc)
CFLAGS += -mstructure-size-boundary=8 
STRIP = arm-linux-strip
endif

ifeq ($(CC), mipsel-uclibc-gcc)
STRIP = mipsel-uclibc-strip
endif

ifeq ($(CC), gcc)
STRIP = strip
endif

export INCLUDE =  -I$(BRCMBASE)/include -I$(BRCMBASE)/common/include -I../../common/include -I../inc $(WLAN_ComponentIncPathR) $(WLAN_SrcIncPathR)

OBJS =  $(CC)/wps_enr.o $(CC)/wps_linux_hooks.o $(CC)/wl_wps.o
REGOBJS =  $(CC)/wps_reg.o $(CC)/wps_linux_hooks.o $(CC)/wl_wps.o
APIOBJS =  $(CC)/wps_api.o $(CC)/wps_linux_hooks.o $(CC)/wl_wps.o
SAMPLEAPPOBJS = $(CC)/wps_api_tester.o

LIBS =  $(CC)/libwpsenr.a $(CC)/libwpscom.a $(CC)/libbcmcrypto.a
SAMPLEAPPLIBS =  $(CC)/libwpsapi.a $(CC)/libwpsenr.a $(CC)/libwpscom.a $(CC)/libbcmcrypto.a -lpthread

default: libs wpsenr wpsapi wpsreg wpsapitester


libs :
	mkdir -p $(CC)/bcmcrypto
	mkdir -p $(CC)/sta
	mkdir -p $(CC)/enrollee
	mkdir -p $(CC)/registrar
	mkdir -p $(CC)/shared
	make BLDTYPE=$(BLDTYPE) CFLAGS="$(CFLAGS)" CC=$(CC) LIBDIR=$(PWD)/$(CC) -C ../../common -f wps_enr_lib.mk
	make BLDTYPE=$(BLDTYPE) CFLAGS="$(CFLAGS)" CC=$(CC) LIBDIR=$(PWD)/$(CC) -C ../../common -f wps_common_lib.mk	

ifeq ($(BLDTYPE),debug)
wpsenr : $(OBJS) $(LIBS)
	$(CC) $(OBJS) $(LIBS) -o $(CC)/wpsenr

wpsapi : $(APIOBJS)
	$(AR) cr $(CC)/libwpsapi.a $^

wpsreg : $(REGOBJS) $(LIBS)
	$(CC) $(REGOBJS) $(LIBS) -o $(CC)/wpsreg

wpsapitester : $(SAMPLEAPPOBJS) $(SAMPLEAPPLIBS)
	$(CC) $(SAMPLEAPPOBJS) $(SAMPLEAPPLIBS) -o $(CC)/wpsapitester

else
wpsenr : $(OBJS) $(LIBS)
	$(CC) $(OBJS) $(LIBS) -o $(CC)/wpsenr
	$(STRIP) $(CC)/wpsenr

wpsapi : $(APIOBJS)
	$(AR) cr $(CC)/libwpsapi.a $^

wpsreg : $(REGOBJS) $(LIBS)
	$(CC) $(REGOBJS) $(LIBS) -o $(CC)/wpsreg
	$(STRIP) $(CC)/wpsreg

wpsapitester : $(SAMPLEAPPOBJS) $(SAMPLEAPPLIBS)
	$(CC) $(SAMPLEAPPOBJS) $(SAMPLEAPPLIBS) -o $(CC)/wpsapitester
	$(STRIP) $(CC)/wpsapitester
endif

$(CC)/%.o : %.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	find -name "*.o" | xargs rm -f 
	find -name "*.so" | xargs rm -f 
	find -name "*.a" | xargs rm -f 
	find -name  wpsenr | xargs rm -f
	find -name  wpsapitester | xargs rm -f
	find -name  wpsreg | xargs rm -f

phony:
