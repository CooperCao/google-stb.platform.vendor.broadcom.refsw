#
# Router rc control scripts Makefile
#
# Copyright (C) 2018, Broadcom Corporation. All Rights Reserved.
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id: Makefile 732738 2017-11-21 00:25:52Z $
#

include $(TOP)/.config

ifneq (2_4,$(LINUX_VERSION))
ifneq (2_6,$(LINUX_VERSION))
CFLAGS += -DLINUX26 -DLINUX_2_6_36 -DAUTOFW_PORT_DEPRECATED
else
CFLAGS += -DLINUX26
endif
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall
ifneq ($(strip $(BCA_HNDROUTER)),)
BB_BASE_PATH = $(BUILD_DIR)/userspace/gpl/apps/busybox
BB_HEADER_PATH = $(firstword $(dir $(shell find $(BB_BASE_PATH) -name  Config.h)))
ifeq ($(strip $(BB_HEADER_PATH)),)
$(error "Could not found busybox header path in $(BB_BASE_PATH)")
else
CFLAGS += -I$(BB_HEADER_PATH)
endif
CFLAGS += -I$(BUILD_DIR)/userspace/public/include
CFLAGS += -I$(BUILD_DIR)/userspace/public/include/linux
CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
ifeq ($(strip $(BUILD_ETHWAN)),)
# GMAC builds do not support WAN
CFLAGS += -DNO_ETHWAN
endif
else
CFLAGS += -I$(LINUXDIR)/include -I$(TOP)/busybox-1.x
endif
else
CFLAGS += -I. -I$(TOP)/shared -I$(TOP)/busybox -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall
endif

CFLAGS += $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
CFLAGS += -I$(TOP)/dpsta

#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s
ifeq ($(CONFIG_RTR_OPTIMIZE_SIZE),y)
CFLAGS  += -Os
else
CFLAGS  += -O2
endif
LDFLAGS	+= -L$(TOP)/nvram -L$(PLATFORMDIR)/install/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(PLATFORMDIR)/install/shared/usr/lib -lshared \
	-L$(TOP)/libbcmcrypto -L$(INSTALLDIR)/libbcmcrypto/usr/lib -lbcmcrypto
LDFLAGS += -L$(TOP)/libbcm -L$(INSTALLDIR)/libbcm/usr/lib -lbcm
LDFLAGS += $(EXTRA_LDFLAGS)


ifeq ($(BUILD_WFA_APUT),1)
CFLAGS	+= -DCAPI_AP
endif

ifeq ($(CONFIG_WAPI_IAS),y)
CFLAGS += -DRC_BUILDTIME=$(shell date -d "now" +"%s")
endif

OBJS := rc.o init.o interface.o network.o services.o udhcpc.o http.o stats.o bcmutils.o mtd.o usb_hotplug.o pmon.o 
ifeq ($(CONFIG_BCMQOS),y)
OBJS += qos.o
endif

ifeq ($(CONFIG_NAT),y)
OBJS += firewall.o
LDFLAGS += -L$(TOP)/netconf -L$(PLATFORMDIR)/install/netconf/usr/lib -lnetconf
endif

ifeq ($(CONFIG_NAT),y)
OBJS += ppp.o
endif

ifeq ($(RWL),1)
CFLAGS += -DRWL_SOCKET
endif

ifneq ($(CONFIG_AIRIQ),)
CFLAGS += -DWL_AIR_IQ
endif # CONFIG_AIRIQ

vpath %.c $(TOP)/shared $(SRCBASE)/shared

all: rc

clean:
	rm -f *.o rc .*.depend

install: all
	install -d $(INSTALLDIR)/sbin
	install rc $(INSTALLDIR)/sbin
	$(STRIP) $(INSTALLDIR)/sbin/rc
ifeq ($(strip $(BCA_HNDROUTER)),)
	cd $(INSTALLDIR)/sbin && ln -sf rc write_cfe
endif
	cd $(INSTALLDIR)/sbin && ln -sf rc write
	cd $(INSTALLDIR)/sbin && ln -sf rc init
	cd $(INSTALLDIR)/sbin && ln -sf rc stats
	cd $(INSTALLDIR)/sbin && ln -sf rc hotplug
	cd $(INSTALLDIR)/sbin && ln -sf rc preinit
	cd $(INSTALLDIR)/sbin && ln -sf rc erase

rc: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: .%.depend
.%.depend: %.c
	$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
