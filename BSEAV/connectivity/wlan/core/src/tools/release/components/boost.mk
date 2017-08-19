override PROD_TAG = $Name: not supported by cvs2svn $

SOURCES := src/vendorlibs/BCGSoft \
	src/vendorlibs/boost/README src/vendorlibs/boost/boost/config \
	src/vendorlibs/boost/boost/detail

LOCAL_SOURCES :=  src/vendorlibs/boost/boost

include src/tools/release/components/makerules.mk
