override PROD_TAG = $Name: not supported by cvs2svn $

SOURCES := src/8021x/openssl src/8021x/wpdpack src/8021x/xsupplicant \
	src/8021x/win32 src/8021x/win64

include src/tools/release/components/makerules.mk
