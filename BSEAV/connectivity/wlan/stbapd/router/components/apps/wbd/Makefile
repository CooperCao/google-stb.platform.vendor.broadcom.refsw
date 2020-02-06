#
# Linux WBD Makefile
#
# $Copyright Open Broadcom Corporation$
#
# <<Broadcom-WL-IPTag/Open:>>
# $Id: Makefile 766304 2018-07-30 08:14:14Z pp888947 $
#

dirs = shared master slave cli
WBDINSTALLDIR = $(INSTALLDIR)/wbd
PREBUILTDIR = prebuilt
LIBWBDSHARED = $(PREBUILTDIR)/libwbdshared.so
WBDMASTER = $(PREBUILTDIR)/wbd_master
WBDSLAVE = $(PREBUILTDIR)/wbd_slave
WBDCLI = $(PREBUILTDIR)/wb_cli

ifeq ($(PLC_WBD),1)
export CFLAGS += -DPLC_WBD
endif

CFLAGS  += -DWL11AC_160

DOXY_PATH = dox
DOXY_FILE = wbd.conf

.PHONY: all
all:
	@for i in $(dirs); do \
		[ ! -d $$i ] || $(MAKE) -C $$i || exit $$? ; \
	done

.PHONY: install
install:
	install -d $(PREBUILTDIR)
# Call individual Makefiles if the directory exists which will copy to prebuilt
	@for i in $(dirs); do \
		[ ! -d $$i ] || $(MAKE) -C $$i install || exit $$? ; \
	done
# Now install to router install directory from prebuilt
	install -d $(WBDINSTALLDIR)/usr/lib
	install -d $(WBDINSTALLDIR)/usr/sbin/
# Install WBD Shared Library
	install -m 755 $(LIBWBDSHARED) $(WBDINSTALLDIR)/usr/lib
	$(STRIP) $(WBDINSTALLDIR)/usr/lib/libwbdshared.so
# Install WBD Master App
	install $(WBDMASTER) $(WBDINSTALLDIR)/usr/sbin/
	$(STRIP) $(WBDINSTALLDIR)/usr/sbin/wbd_master
# Install WBD Slave App
	install $(WBDSLAVE) $(WBDINSTALLDIR)/usr/sbin/
	$(STRIP) $(WBDINSTALLDIR)/usr/sbin/wbd_slave
# Install WBD CLI App
	install $(WBDCLI) $(WBDINSTALLDIR)/usr/sbin/
	$(STRIP) $(WBDINSTALLDIR)/usr/sbin/wb_cli

.PHONY: doc
doc:
	doxygen $(DOXY_FILE)

.PHONY: clean
clean:
	@for i in $(dirs); do \
		[ ! -d $$i ] || $(MAKE) -C $$i clean ; \
	done
	[ -d blanket ] && cd blanket && rm -f *.o && cd -
	rm -rf $(DOXY_PATH)
