override PROD_TAG = $Name: not supported by cvs2svn $

WLAN_ComponentsInUse += bcmwifi clm-api ppr olpc
include src/makefiles/WLAN_Common.mk

SOURCES := src/wl/config \
	 src/wl/sys \
	 src/wl/exe \
	 src/wl/linux \
	 $(WLAN_ComponentIncDirs) \
	 $(WLAN_ComponentSrcDirs)

include src/tools/release/components/makerules.mk
