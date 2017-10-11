override PROD_TAG = $Name: not supported by cvs2svn $

# NOTE: 'customer' component is obsoleted by moto_linux
# Do not use 'customer' for new BOM branches.

SOURCES :=  src/customer/motorola/linux

include src/tools/release/components/makerules.mk
