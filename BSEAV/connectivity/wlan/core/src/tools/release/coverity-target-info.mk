##
## This file is meant to provide a descriptive names to cryptic
## coverity targetnames, that don't make sense when generating
## coverity reports to all.
##
## This file acts as a common file between coverity builder
## and coverity status reporter (or for any other need in future)
##
## WARN: THIS IS A CENTRALIZED CONFIG FILE AND EVEN IF IT EXISTS ON
## WARN: A BRANCH, COVERITY BUILD PROCESS STILL REFERS IT FROM TRUNK
##
## WARN: SO BRANCH COPY OF coverity-targets-info.mk IS NOT USED
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##
## # $Id$
##

UNAME            := $(shell uname -s)

LINUX_DONGLE_DIR := src/dongle/make/wl
WIN_IHV_DIR := components/apps/windows/ihvfrm/ihv
TREES := GCLIENT
ROUTER_TREE=src/router

# Weekly schedule is deprecated. These targets will run daily instead
# after older branches are swapped out for newer active branches
# ifeq ($(strip $(shell date '+%a')),Sun)
#   WEEKLYBLD=1
# endif # DAY

ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
  COV-WINBUILD   := 1
endif
ifeq ($(findstring Darwin,$(UNAME)),Darwin)
  COV-MACBUILD   := 1
endif
ifeq ($(findstring Linux,$(UNAME)),Linux)
  COV-LINUXBUILD := 1
endif
ifndef TAG
  TAG:=TOT
endif

########################################################################
## NOTE: Following targets need corresponding Coverity
## NOTE: stream and projects
########################################################################

##################### Windows Target Description #######################

# Windows NIC and BMAC Drivers
build_xp_driver_wl_x86          = "WinXP x86 WL NIC Driver"
build_xp_driver_wl_x64          = "WinXP x64 WL NIC Driver"
build_win7_driver_wl_x86        = "Win7 x86 WL NIC Driver"
build_win7_driver_wl_x64        = "Win7 x64 WL NIC Driver"
build_win8x_driver_wl_x86       = "Win8X x86 WL NIC Driver"
build_xp_high_driver_wl_x86     = "WinXP x86 WL BMac Driver"
build_xp_high_driver_wl_x64     = "WinXP x64 WL BMac Driver"
build_win7_high_driver_wl_x86   = "Win7 x86 WL BMac Driver"
build_win7_high_driver_wl_x64   = "Win7 x64 WL BMac Driver"
build_win8x_high_driver_wl_x86  = "Win8x x86 WL BMac Driver"

# Windows DHD Drivers
build_usb_xp_driver_x86         = "WinXP x86 DHD USB Driver"
build_usb_xp_driver_x64         = "WinXP x64 DHD USB Driver"
build_usb_xp_driver_amd64       = "WinXP amd64 DHD USB Driver"
build_usb_vista_driver_x86      = "WinVista x86 DHD USB Driver"
build_usb_vista_driver_x64      = "WinVista x64 DHD USB Driver"
build_usb_vista_driver_amd64    = "WinVista amd64 DHD USB Driver"

# Windows Utils
winxp-wl-exe                    = "WinXP x86 WL Utility"
winvista-wl-exe                 = "WinVista x86 WL Utility"
winxp-dhd-exe                   = "WinXP x86 DHD Utility"

##################### Linux Target Description #######################
# Linux NIC and BMAC Drivers
debug-apdef-stadef              = "Linux x86 WL NIC Driver"
debug-apdef-stadef-high         = "Linux x86 WL BMac Driver"

# Linux DHD Drivers
dhd-cdc-sdstd                   = "Linux x86 DHD SDIO Driver"
dhd-cdc-sdmmc-android-panda-icsmr1-cfg80211-oob = "Android ARM DHD SDIO Cfg80211 Driver"
dhd-cdc-usb-gpl                 = "Linux x86 DHD USB GPL Driver"

# Linux Utils
linux-wl-exe                    = "Linux x86 WL Utility"
linux-dhd-exe                   = "Linux x86 DHD Utility"
linux-cxapi                     = "Linux x86 Connection API"

# Linux Firmware Images
# Firmware names are used verbatim in Coverity target desc

##################### MacOS Target Description #######################
# MacOS NIC and BMAC Drivers
macos-tiger-wl-driver             = "MacOS Tiger WL NIC Driver"
macos-leopard-wl-driver           = "MacOS Leopard WL NIC Driver"
macos-snowleopard-wl-driver       = "MacOS SnowLeopard WL NIC Driver"
macos-borelo-wl-driver            = "MacOS Borelo WL NIC Driver"

# MacOS Utils
macos-wl-exe                      = "MacOS WL Utility"

Release_10_9                      = "MacOS Cab WL NIC Driver"
Release_Mfg_10_9                  = "MacOS Cab WL NIC Driver"

########################################################################

# == WARN == WARN == WARN == WARN == WARN == WARN == WARN == WARN ====
#
# Coverity runs are very resource intensive. So add only those targets
# are absolutely required. TOT covers most of the runs
#
# == WARN == WARN == WARN == WARN == WARN == WARN == WARN == WARN ====

# =====================================================================
# Branches now can pick any targets they want from above list of targets
# =====================================================================

##################### TOT Targets #######################
ifneq ($(filter NIGHTLY TOT HEAD TRUNK,$(TAG)),)

    ROUTER_TREE=components/router

    # Windows WL NIC and BMAC driver Coverity Targets
    WIN_WL_TARGETS       :=
    WIN_WL_TARGETS       += build_win8x_driver_wl_x86

    WIN10_WL_TARGETS     :=
    WIN10_WL_TARGETS     += build_win10_driver_wl

    # Windows USB DHD Coverity Targets
    WIN_DHD_TARGETS      :=

    WIN10_DHD_TARGETS    :=
    WIN10_DHD_TARGETS    += build_pcie_win10_driver
    WIN10_DHD_TARGETS    += build_sdio_win10_driver

    # Windows Mobile SDIO DHD Targets
    WIN_WM_TARGETS       :=

    # Windows Utils
    WIN_UTILS_TARGETS    :=

    # Windows IHV
    WIN_IHV_TARGETS          :=

    # Linux WL NIC and BMAC driver Coverity Targets
    LINUX_WL_TARGETS     :=
    LINUX_WL_TARGETS     += debug-apdef-stadef

    # Linux SDIO DHD Coverity Targets
    LINUX_DHD_TARGETS    :=
    LINUX_DHD_TARGETS    += dhd-cdc-sdstd-hc3
    LINUX_DHD_TARGETS    += dhd-cdc-sdmmc-android-panda-cfg80211-oob
    LINUX_DHD_TARGETS    += dhd-msgbuf-pciefd-debug
    LINUX_DHD_TARGETS    += dhd-cdc-usb-gpl

    # Linux PCIE DHD Coverity Targets
    LINUX_DHD_BRIX_TARGETS :=
    LINUX_DHD_BRIX_TARGETS += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt

    # Linux Utils
    LINUX_UTILS_TARGETS  :=
    LINUX_UTILS_TARGETS  += linux-wl-exe

    # All Dongle images
    LINUX_DONGLE_TARGETS :=

    # Linux Routers
    LINUX_ROUTER_TARGETS :=
    LINUX_ROUTER_TARGETS += linux-2.6.36-router-main

    # Mac WL NIC driver Coverity Targets
    MAC_WL_TARGETS_10_10 :=
    MAC_WL_TARGETS_10_10 += Debug_10_10
    MAC_WL_TARGETS_10_10 += Debug_Mfg_10_10
    MAC_WL_TARGETS_10_10 += Release_10_10

endif # TOT/NIGHTLY/TRUNK

##################### DHD_BRANCH_1_363 Targets #######################

ifneq ($(filter DHD_BRANCH_1_363,$(TAG)),)

    # Linux SDIO DHD Coverity Targets
    LINUX_DHD_TARGETS    :=
    LINUX_DHD_TARGETS    += dhd-msgbuf-pciefd-debug
    LINUX_DHD_TARGETS    += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-debug

endif # DHD_BRANCH_1_363

##################### DHD_BRANCH_1_579 Targets #######################

ifneq ($(filter DHD_BRANCH_1_579,$(TAG)),)

    HNDSVN_BOMS_GCLIENT := dhd

    # Linux SDIO DHD Coverity Targets
    LINUX_DHD_TARGETS    :=
    LINUX_DHD_TARGETS    += dhd-cdc-sdmmc-btsharedsdio-android-panda-cfg80211-oob-debug-3.2.0-panda 
    LINUX_DHD_TARGETS    += dhd-msgbuf-pciefd-debug

    LINUX_DHD_BRIX_TARGETS :=
    LINUX_DHD_BRIX_TARGETS += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-debug

endif # DHD_BRANCH_1_579

##################### DIN2915T250RC1_BRANCH_9_30 Targets #######################
ifneq ($(filter DIN2915T250RC1_BRANCH_9_30,$(TAG)),)

    # All Dongle images
    LINUX_DONGLE_TARGETS :=

    LINUX_DONGLE_TARGETS += 43430b0-roml/config_sdio_release
    LINUX_DONGLE_TARGETS += 43430b1-roml/config_sdio_mfgtest
    LINUX_DONGLE_TARGETS += 43430b1-roml/config_sdio_release

    LINUX_DONGLE_TARGETS += 4355b3-roml/config_pcie_release
    LINUX_DONGLE_TARGETS += 4355c0-roml/config_pcie_release
    LINUX_DONGLE_TARGETS += 4355c0-roml/config_pcie_release_sdb

    LINUX_DONGLE_TARGETS += 4364b0-roml/config_pcie_release

endif # DIN2915T250RC1_BRANCH_9_30

##################### EAGLE_BRANCH_10_10 Targets #######################
ifneq ($(filter EAGLE_BRANCH_%,$(TAG)),)

    ROUTER_TREE=components/router

    # Windows WL NIC and BMAC driver Coverity Targets
    WIN_WL_TARGETS       :=
    WIN_WL_TARGETS       += build_win8x_driver_wl_x86

    # Windows USB DHD Coverity Targets
    WIN_DHD_TARGETS :=

    # Windows Mobile SDIO DHD Targets
    WIN_WM_TARGETS :=

    # Windows Utils
    WIN_UTILS_TARGETS :=

    # Windows IHV
    WIN_IHV_TARGETS :=

    # Linux WL NIC and BMAC driver Coverity Targets
    LINUX_WL_TARGETS :=
    LINUX_WL_TARGETS += debug-apdef-stadef
    LINUX_WL_TARGETS += debug-apdef-stadef-high
    LINUX_WL_TARGETS += debug-apdef-stadef-high-p2p-mchan-tdls-media

    # Linux SDIO DHD Coverity Targets
    LINUX_DHD_TARGETS :=

    # Linux PCIE DHD Coverity Targets
    LINUX_DHD_BRIX_TARGETS :=

    # Linux Utils
    LINUX_UTILS_TARGETS :=

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 43602a1-ram/pcie-ag-err-assert-splitrx

    # Linux Routers
    LINUX_ROUTER_TARGETS :=
    LINUX_ROUTER_TARGETS += linux-2.6.36-router-main

    # Mac WL NIC driver Coverity Targets
    MAC_WL_TARGETS_10_10       :=

endif # EAGLE_BRANCH

##################### BISON06T_BRANCH_7_45 Targets #######################

ifneq ($(filter BISON06T_BRANCH_7_45,$(TAG)),)

    LINUX_DONGLE_DIR := src/dongle/rte/wl
    TREES := SPARSE

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 43430a1-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-ccx-okc-anqpo-vista-ndis-mfp-ndoe-11nprop-ve-wl11u-hs20sta-tdls-monitor-aibss-d0c-txbf-sr
    LINUX_DONGLE_TARGETS += 43430a0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-sr-mchan-proptxstatus-lpc-wl11u-autoabn-txbf-rcc-fmc-wepso-ccx-okc-anqpo-p2po-ltecx-noe-aibss-proxd-relmcast-sr
    LINUX_DONGLE_TARGETS += 43430a0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-sr
    LINUX_DONGLE_TARGETS += 43430a1-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-sr-11nprop-tdls-hs20sta-aibss-relmcast
    LINUX_DONGLE_TARGETS += 4345c0-roml/sdio-ag-pool-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl
    LINUX_DONGLE_TARGETS += 4345c0-roml/pcie-ag-msgbuf-splitrx-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-amsdutx-ltecx-wfds-okc-ccx-ve-clm_ss_mimo-txpwr-abt-rcc-fmc-wepso-noccxaka-sarctrl-nocis

    # Linux WL NIC and BMAC driver Coverity Targets
    LINUX_WL_TARGETS :=
    LINUX_WL_TARGETS += debug-apdef-stadef-high-p2p-mchan-tdls

endif # BISON06T_BRANCH_7_45

##################### BISON05T_BRANCH_7_35 Targets #######################

ifneq ($(filter BISON05T_BRANCH_7_35,$(TAG)),)

    LINUX_DONGLE_DIR := src/dongle/rte/wl
    TREES := SPARSE

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-proptxstatus

endif # BISON05T_BRANCH_7_35

##################### BISON04T_BRANCH_7_14 Targets #######################

ifneq ($(filter BISON04T_BRANCH_7_14,$(TAG)),)

    TREES := SPARSE

    # Linux Routers
    LINUX_ROUTER_TARGETS :=
    LINUX_ROUTER_TARGETS += linux-2.6.36-router

endif # BISON04T_BRANCH_7_14

##################### BIS747T144RC2_BRANCH_7_64 Targets #######################

ifneq ($(filter BIS747T144RC2_BRANCH_7_64,$(TAG)),)

    LINUX_DONGLE_DIR := src/dongle/rte/wl
    TREES := SPARSE

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 43452a2-roml/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace_pcie-srscan-clm_min-txpwrcap-bssinfo
    LINUX_DONGLE_TARGETS += 43452a3-roml/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace_pcie-srscan-clm_min-txpwrcap-bssinfo
    LINUX_DONGLE_TARGETS += 4350c5-roml/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace_pcie-srscan-clm_min-txpwrcap-err-bdo-swdiv-ecounters

endif # BIS747T144RC2_BRANCH_7_64

##################### BIS715GALA_BRANCH_7_21 Targets #######################

ifneq ($(filter BIS715GALA_BRANCH_7_21,$(TAG)),)

    TREES := SPARSE

    # Mac WL NIC driver Coverity Targets
    MAC_WL_TARGETS_10_10 :=
    MAC_WL_TARGETS_10_10 += Debug_10_10
    MAC_WL_TARGETS_10_10 += Release_10_10

    MAC_WL_TARGETS_10_11 :=
    MAC_WL_TARGETS_10_11 += Debug_10_11
    MAC_WL_TARGETS_10_11 += Release_10_11

    MAC_WL_TARGETS_10_12 :=
    MAC_WL_TARGETS_10_12 += Debug_10_12
    MAC_WL_TARGETS_10_12 += Release_10_12

    # Mac Utils
    MAC_UTILS_TARGETS_10_10  :=
    MAC_UTILS_TARGETS_10_10  += mac-wl-exe

endif # BIS715GALA_BRANCH_7_21

##################### BISON04T_TWIG_7_14_131 Targets #######################

ifneq ($(filter BISON04T_TWIG_7_14_131,$(TAG)),)

    HNDSVN_BOMS_GCLIENT := linux-2.6.36-router-dhdap

    # Linux Routers
    LINUX_ROUTER_TARGETS :=
    LINUX_ROUTER_TARGETS += linux-2.6.36-router-dhdap

endif # BISON04T_TWIG_7_14_131

##################### EAGLE_TWIG_10_10_69 Targets #######################

ifneq ($(filter EAGLE_TWIG_10_10_69,$(TAG)),)

    HNDSVN_BOMS_GCLIENT := linux-2.6.36-router-dhdap
    GCLIENT_REPO_URL :=  http://svn.sj.broadcom.com/svn/wlansvn/components/deps/branches/BISON04T_TWIG_7_14_131

    # Linux Routers
    LINUX_ROUTER_TARGETS :=
    LINUX_ROUTER_TARGETS += linux-2.6.36-router-dhdap

endif # EAGLE_TWIG_10_10_69

##################### BISON05T_TWIG_7_35_260_64 Targets #######################

ifneq ($(filter BISON05T_TWIG_7_35_260_64,$(TAG)),)

    HNDSVN_BOMS_GCLIENT := linux-2.6.36-router-dhdap
    GCLIENT_REPO_URL :=  http://svn.sj.broadcom.com/svn/wlansvn/components/deps/branches/BISON04T_TWIG_7_14_131

    # Linux Routers
    LINUX_ROUTER_TARGETS :=
    LINUX_ROUTER_TARGETS += linux-2.6.36-router-dhdap

endif # BISON05T_TWIG_7_35_260_64

##################### DHD_TWIG_1_363_45_58 Targets #######################

ifneq ($(filter DHD_TWIG_1_363_45_58,$(TAG)),)

    HNDSVN_BOMS_GCLIENT := linux-2.6.36-router-dhdap
    GCLIENT_REPO_URL :=  http://svn.sj.broadcom.com/svn/wlansvn/components/deps/branches/BISON04T_TWIG_7_14_131

    # Linux Routers
    LINUX_ROUTER_TARGETS :=
    LINUX_ROUTER_TARGETS += linux-2.6.36-router-dhdap

endif # DHD_TWIG_1_363_45_58

##################### BIS747T144RC2_TWIG_7_64_3 Targets #######################

ifneq ($(filter BIS747T144RC2_TWIG_7_64_3,$(TAG)),)

    LINUX_DONGLE_DIR := src/dongle/rte/wl
    TREES := SPARSE

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 43452a3-roml/pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace_pcie-srscan-clm_min-txpwrcap-bssinfo

endif # BIS747T144RC2_TWIG_7_64_3

##################### IGUANA_BRANCH_13_10 Targets #######################

ifneq ($(filter IGUANA_BRANCH_13_10,$(TAG)),)

    HNDSVN_BOMS_GCLIENT := hndrte-dongle-wl

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 43012b0-roml/config_sdio
    LINUX_DONGLE_TARGETS += 4357b0-roml/config_pcie_debug
    LINUX_DONGLE_TARGETS += 4361a0-ram/config_pcie_natoe
    # LINUX_DONGLE_TARGETS += 4361a0-roml/config_pcie_release2
    LINUX_DONGLE_TARGETS += 4361b0-roml/config_pcie_fullfeature

endif # IGUANA_BRANCH_13_10

##################### IGUANA08T_BRANCH_13_35 Target #######################

ifneq ($(filter IGUANA08T_BRANCH_13_35,$(TAG)),)

    HNDSVN_BOMS_GCLIENT := hndrte-dongle-wl

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 4361b0-roml/config_pcie_fullfeature

endif # IGUANA08T_BRANCH_13_35

##################### BISON06T_TWIG_7_45_45 Targets #######################

ifneq ($(filter BISON06T_TWIG_7_45_45,$(TAG)),)

    LINUX_DONGLE_DIR := src/dongle/rte/wl
    TREES := SPARSE

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 43430a1-roml/sdio-g-pool-p2p-idsup-idauth-pno-pktfilter-keepalive-aoe-lpc-swdiv-srfast-fuart-btcxhybridhw

endif # BISON06T_TWIG_7_45_45

##################### IOTTEST_BRANCH_1_10 Targets #######################

ifneq ($(filter IOTTEST_BRANCH_1_10,$(TAG)),)

    HNDSVN_BOMS_GCLIENT := hndrte-dongle-wl

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 43602a1-ram/config_pcie_debug

    # Linux SDIO DHD Coverity Targets
    LINUX_DHD_TARGETS    :=
    #LINUX_DHD_TARGETS    += dhd-cdc-sdstd-hc3

endif # IOTTEST_BRANCH_1_10

##################### DIN2930R18_BRANCH_9_44 Targets #######################

ifneq ($(filter DIN2930R18_BRANCH_9_44,$(TAG)),)

    HNDSVN_BOMS_GCLIENT := hndrte-dongle-wl

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 43430b1-roml/config_sdio_release
    LINUX_DONGLE_TARGETS += 4355b3-roml/config_pcie_release
    LINUX_DONGLE_TARGETS += 4355c0-roml/config_pcie_release

endif # DIN2930R18_BRANCH_9_44

##################### EAGLE_TWIG_10_10_122 Targets #######################

ifneq ($(filter EAGLE_TWIG_10_10_122,$(TAG)),)

    ROUTER_TREE=components/router

    # Windows WL NIC and BMAC driver Coverity Targets
    WIN_WL_TARGETS       :=
    WIN_WL_TARGETS       += build_win8x_driver_wl_x86

    # Windows USB DHD Coverity Targets
    WIN_DHD_TARGETS :=

    # Linux WL NIC and BMAC driver Coverity Targets
    LINUX_WL_TARGETS :=
    LINUX_WL_TARGETS += debug-apdef-stadef
    LINUX_WL_TARGETS += debug-apdef-stadef-high
    LINUX_WL_TARGETS += debug-apdef-stadef-high-p2p-mchan-tdls-media

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 43602a1-ram/pcie-ag-err-assert-splitrx

    # Linux Routers
    LINUX_ROUTER_TARGETS :=
    LINUX_ROUTER_TARGETS += linux-2.6.36-router-main

endif # EAGLE_TWIG_10_10_122

##################### JAGUAR_BRANCH_14_10 Targets #######################

ifneq ($(filter JAGUAR_BRANCH_14_10,$(TAG)),)

    HNDSVN_BOMS_GCLIENT := hndrte-dongle-wl

    # All Dongle images
    LINUX_DONGLE_TARGETS :=
    LINUX_DONGLE_TARGETS += 4369a0-ram/config_pcie_coverity

endif # JAGUAR_BRANCH_14_10

##############################################################

ALL_COVERITY_TARGETS :=
#ifdef COV-WINBUILD
      ALL_COVERITY_TARGETS += $(WIN10_DHD_TARGETS)
      ALL_COVERITY_TARGETS += $(WIN10_WL_TARGETS)
      ALL_COVERITY_TARGETS += $(WIN_DHD_TARGETS)
      ALL_COVERITY_TARGETS += $(WIN_IHV_TARGETS)
      ALL_COVERITY_TARGETS += $(WIN_PREFAST_TARGETS)
      ALL_COVERITY_TARGETS += $(WIN_UTILS_TARGETS)
      ALL_COVERITY_TARGETS += $(WIN_WL_TARGETS)
#endif # COV-WINBUILD

#ifdef COV-LINUXBUILD
      ALL_COVERITY_TARGETS += $(LINUX_CXAPI_TARGETS)
      ALL_COVERITY_TARGETS += $(LINUX_DHD_BRIX_TARGETS)
      ALL_COVERITY_TARGETS += $(LINUX_DHD_TARGETS)
      ALL_COVERITY_TARGETS += $(LINUX_DONGLE_TARGETS)
      ALL_COVERITY_TARGETS += $(LINUX_ROUTER_TARGETS)
      ALL_COVERITY_TARGETS += $(LINUX_UTILS_TARGETS)
      ALL_COVERITY_TARGETS += $(LINUX_WL_TARGETS)
     #TO-DO# ALL_COVERITY_TARGETS += $(LINUX_DONGLE_TARGETS:%="% Firmware")
#endif # COV-LINUXBUILD

#ifdef COV-MACBUILD
      ALL_COVERITY_TARGETS += $(MAC_DHD_TARGETS_10_11)
      ALL_COVERITY_TARGETS += $(MAC_UTILS_TARGETS_10_10)
      ALL_COVERITY_TARGETS += $(MAC_WL_TARGETS_10_10)
      ALL_COVERITY_TARGETS += $(MAC_WL_TARGETS_10_11)
      ALL_COVERITY_TARGETS += $(MAC_WL_TARGETS_10_12)
#endif # COV-MACBUILD

default: all

#
# Expand given target to its description
#
%:
	@echo "$(if $($@),$($@),$@)"

#
# Show a list of targets given a TAG
#
show_coverity_targets:
	@echo "$(ALL_COVERITY_TARGETS)"
