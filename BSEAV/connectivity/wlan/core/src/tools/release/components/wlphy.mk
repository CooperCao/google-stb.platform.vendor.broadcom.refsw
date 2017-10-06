override PROD_TAG = $Name: not supported by cvs2svn $

SOURCES := components/phy/old src/tools/release/components/wlphy-filelist.txt

ifeq ("$(MK_TYPE)", "SRC")
MOD_VERSION = phy_version
endif

include src/hndcvs/makerules.mk

ifeq ("$(MOD_VERSION)", "phy_version")
phy_version:
	cvs -q co -p src/tools/release/mkversion.sh > mkversion.sh
	bash ./mkversion.sh components/phy/old/phy_version.h.in components/phy/old/phy_version.h $(TAG_NAME)
endif
