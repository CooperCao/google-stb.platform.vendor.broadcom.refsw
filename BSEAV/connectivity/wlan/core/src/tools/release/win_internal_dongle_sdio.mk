#
# Internal makefile to build SDIO DHD drivers, app and package them
#
# $Id$
#

PARENT_MAKEFILE := win_dhd_sdio.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

export BRAND       := win_internal_dongle_sdio

include $(PARENT_MAKEFILE)

# Embeddable image for win dhd driver comes from module makefile
EMBED_DONGLE_IMAGE     :=

DBUS_SDIO_DONGLE_IMAGE :=

## Include any other dongle images needed for this brand only
SDIO_DONGLE_IMAGES :=
SDIO_DONGLE_IMAGES += $(DBUS_SDIO_DONGLE_IMAGE)

# brand specific mogrifier defines and undefines if any
# UNDEFS +=
# DEFS   +=
