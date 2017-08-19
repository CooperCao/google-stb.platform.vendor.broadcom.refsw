override PROD_TAG = $Name: not supported by cvs2svn $

#
#  Get the VoIP modules

SOURCES := src/voip components/router/voipd src/tools/release/components/voip-filelist.txt


include src/tools/release/components/makerules.mk
