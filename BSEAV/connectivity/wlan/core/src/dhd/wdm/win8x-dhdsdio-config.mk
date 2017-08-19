# This makefile is included by win8_slate_..._wl.mk brand makefiles
# It defines files that shall be distributed with drivers (firmware images, NVRAM-files, etc.)

# FIRMWARE_IMAGES defines which firmware images shall be built for driver
# FIRMWARE_IMAGES is a space-separated list of image specifiers
# Each image specifier has TAG.image form
# Note that chipset part of image name (part before first dash) must exactly match those, mentioned in .inf files, whereas tag and firmware options (head and tail of image name) may vary
FIRMWARE_IMAGES ?= BISON05T_REL_7_35_180_12.src/dongle/rte/wl/builds.4356a2-roml/sdio-ag-ndis-vista-pktfilter-d0c-pno-aoe-p2p-dhdoid-ndoe-gtkoe-mfp-proptxstatus-dmatxrc-keepalive-ap-sr-ampduretry-pclose-txbf \
                   BISON06T_REL_7_45_87.src/dongle/rte/wl/builds.43455c0-roml/sdio-ag-ndis-vista-pktfilter-d0c-pno-aoe-p2p-dhdoid-ndoe-gtkoe-mfp-proptxstatus-dmatxrc-keepalive-ap-ampduretry-pclose-txbf \
                   BISON_REL_7_10_323_70.src/dongle/rte/wl/builds.43438a0-roml/sdio-g-ndis-vista-pktfilter-d0c-pno-aoe-p2p-dhdoid-ndoe-mfp-proptxstatus-dmatxrc-keepalive-ap-ampduretry-pclose \
                   BISON_REL_7_10_323_70.src/dongle/rte/wl/builds.43438a1-roml/sdio-g-ndis-vista-pktfilter-d0c-pno-aoe-p2p-dhdoid-ndoe-mfp-proptxstatus-dmatxrc-keepalive-ap-ampduretry-pclose

# DRIVER_FILES_INTEL is a list of files used by drivers for Intel platform
# File names are relative to src/dhd/wdm directory
DRIVER_FILES_INTEL ?= bcm94356z_p122.txt bcm943430wlselgs.txt bcm943430a1wlselgs.txt
# DRIVER_FILES_ARM is a list of files used by drivers for ARM platform
# File names are relative to src/dhd/wdm directory
DRIVER_FILES_ARM ?= bcm943430wlselgs.txt
# DRIVER_FILES_OTHER is a list of files used by all drivers drivers built for internal releases
# File names are relative to src/dhd/wdm directory
DRIVER_FILES_OTHER ?= win8-dhdsdio-release-notes.txt
