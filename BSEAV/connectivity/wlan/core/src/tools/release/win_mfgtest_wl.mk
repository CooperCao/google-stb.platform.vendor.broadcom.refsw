#
# Build the windows wireless drivers and tools
#
# Contact: hnd-software-scm-list
#
# $Id$
#

PARENT_MAKEFILE := win_external_wl.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

## BCM_MFGTEST flag is read by included makefile to set appropriate
## gnu makefile options
## TODO: BCM_MFGTEST can be obsoleted as WLTEST is common everywhere
BCM_MFGTEST    := 1
WLTEST         := 1
BRAND          := win_mfgtest_wl

# Suppress building trayapp/installshield etc.,
export BUILD_TRAYAPP  := false

# Suppressing notification of dialog/ui violations in trayapps
export DLGTEST_NOTIFY := false

include $(PARENT_MAKEFILE)

## OEM_LIST is read by trayapp vs03 build. This does not affect anything else.
export OEM_LIST = bcm
