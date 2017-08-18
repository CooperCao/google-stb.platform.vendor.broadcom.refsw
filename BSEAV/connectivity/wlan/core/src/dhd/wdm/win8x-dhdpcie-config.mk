# This makefile is included by win8_slate_..._wl.mk brand makefiles
# It defines files that shall be distributed with drivers (firmware images, NVRAM-files, etc.)

# FIRMWARE_IMAGES defines which firmware images shall be built for driver
# FIRMWARE_IMAGES is a space-separated list of image specifiers
# Each image specifier has TAG.image form
# Note that chipset part of image name (part before first dash) must exactly match those, mentioned in .inf files, whereas tag and firmware options (head and tail of image name) may vary
FIRMWARE_IMAGES ?= BISON05T_REL_7_35_180_157.src/dongle/rte/wl/builds.4356a2-roml/pcie-ag-msgbuf-chkd2hdma-ndis-vista-dhdoid-ap-p2p-txbf-pktctx-amsdutx-pktfilter-pno-aoe-ndoe-gtkoe-mfp-proptxstatus-keepalive-sr-ampduretry-ltecx-fbt-assocmgr \
                   DIN2915T250RC1_REL_9_30_79.build/dongle.4364a0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-chkd2hdma-ndis-extsta-txbf-pktctx-amsdutx-pktfilter-pno-aoe-ndoe-gtkoe-mfp-proptxstatus-keepalive-sr-ampduretry-wapi-logtrace-clm_min-noclminc-norsdb-assert-nohchk \
                   DIN2915T250RC1_REL_9_30_79.build/dongle.4364b0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-chkd2hdma-ndis-extsta-txbf-pktctx-amsdutx-pktfilter-pno-aoe-ndoe-gtkoe-mfp-proptxstatus-keepalive-sr-ampduretry-wapi-logtrace-clm_min-noclminc-norsdb-assert-nohchk \
                   DIN2915T250RC1_REL_9_30_79.build/dongle.4364b1-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-chkd2hdma-ndis-extsta-txbf-pktctx-amsdutx-pktfilter-pno-aoe-ndoe-gtkoe-mfp-proptxstatus-keepalive-sr-ampduretry-wapi-logtrace-clm_min-noclminc-norsdb-assert-nohchk \
                   trunk.build/dongle.43602a1-ram/pcie-ag-msgbuf-chkd2hdma-ndis-extsta-pktctx-amsdutx-mfp-proptxstatus
MFG_FIRMWARE_IMAGES ?= BISON05T_REL_7_35_180_157.src/dongle/rte/wl/builds.4356a2-roml/pcie-ag-msgbuf-chkd2hdma-ndis-vista-dhdoid-apsta-proptxstatus-sr-mfgtest-seqcmds-txbf \
                   DIN2915T250RC1_REL_9_30_79.build/dongle.4364a0-roml/threadx-pcie-ag-msgbuf-mfgtest-seqcmds-splitrx-ltecx-wlota-txcal-txbf-swdiv-txpwrcap-tcms-consuartseci-pwrstats-dfsradar-p2p \
# DRIVER_FILES_NVRAM is a list of files used by Full Dongle for HW calibration data
# File names are relative to src/dhd/wdm directory
DRIVER_FILES_NVRAM ?= bcm94364fcpagb_2.txt 4364.clmb
