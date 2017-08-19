#
# External makefile to build SDIO DHD drivers, app and package them
#
# $Id$
#
PARENT_MAKEFILE := win_dhd_sdio.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

export BRAND       := win_external_dongle_sdio

include $(PARENT_MAKEFILE)

# Embeddable image for win dhd driver
EMBED_DONGLE_IMAGE :=

## Include any other dongle images needed for this brand only
DBUS_SDIO_DONGLE_IMAGE :=
SDIO_DONGLE_IMAGES :=
SDIO_DONGLE_IMAGES += $(DBUS_SDIO_DONGLE_IMAGE)

# brand specific mogrifier defines and undefines if any
# DEFS +=
UNDEFS += BCMINTERNAL
