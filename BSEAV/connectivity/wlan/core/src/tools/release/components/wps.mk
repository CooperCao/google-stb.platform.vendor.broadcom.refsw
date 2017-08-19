override PROD_TAG = $Name: not supported by cvs2svn $

SOURCES := src/wps components/router/wps src/tools/release/components/wps-filelist.txt

ifeq ("$(MK_TYPE)", "SRC")
MOD_VERSION = wps_version
endif

include src/tools/release/components/makerules.mk

ifeq ("$(MOD_VERSION)", "wps_version")
wps_version :
	cvs co src/tools/release/mkversion.sh
	bash src/tools/release/mkversion.sh src/wps/common/include/version.h.in src/wps/common/include/wps_version.h "$(PROD_TAG)"
endif
