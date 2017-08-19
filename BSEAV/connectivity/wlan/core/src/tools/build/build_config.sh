#!/bin/bash
#
# Config file to define branch and release properties. This is sourced or
# executed by build_<platform>.sh and build_brands to get a list of DEFAULT
# build brands
#
# This script is also used by other automations like build_summary.sh, fw
# image listing and coverity reporting to get valid brands and active branches
#
# NOTE: Define BRANCH specs ahead of TWIG specs for the given BRANCH
# NOTE: e.g: KIRIN_TWIG defs come after KIRIN_BRANCH defs
#
# Default list of brands can be defined for either a specific _REL_ build
# or _TWIG_ or _BRANCH_. The list of brands are derived like a waterfall
# as follows:
# If default list of brands exist for RELNAME, use them
# If not use TWIGNAME default list of brands, if defined
# If not use BRANCHNAME default list of brands, if defined
# If not use TAGPFX default list of brands, if defined
# If not use DEFAULT list of brands
#
# $Copyright (C) 2008 Broadcom Corporation9
#
# Author: Prakash Dhavali
#
# $Id$
#
# SVN: $HeadURL$
#
# Usage   : sh $0 [-r <tag>] [-p <platform>]
#
# Examples and test cases:
# Showing both using long cmd line option and short option
# ------------------------------------------------------------------
# Example: To see script help and available cmd line options
#           sh build_config.sh --help
#           sh build_config.sh -h
# Example: Show default windows build brands for TOT
#           sh build_config.sh --platform windows
#           sh build_config.sh -p windows
# Example: Show default linux build brands for TOT
#           sh build_config.sh --platform linux
#           sh build_config.sh -p linux
# Example: Show default windows brands for PHOENIX2_BRANCH_6_10
#           sh build_config.sh --revision PHOENIX2_BRANCH_6_10 --platform window
#           sh build_config.sh -r PHOENIX2_BRANCH_6_10 -p window
# Example: Show default brands across all platforms
#           sh build_config.sh --show_brands
#           sh build_config.sh -b
# Example: Show all active branches and twigs
#           sh build_config.sh --show_active
#           sh build_config.sh -a
# Example: Show all default brands for AKASHI_BRANCH_5_110 program
#           sh build_config.sh --revision AKASHI_BRANCH_5_110
#           sh build_config.sh -r AKASHI_BRANCH_5_110
# Example: Show linux build brands that need 32bit resources
#           sh build_config.sh --show_32bit_brands
#           sh build_config.sh --32
# Example: Show all active coverity branches and twigs
#           sh build_config.sh --show_coverity_active
#           sh build_config.sh -c
#
# WARN: Changes to this config file need to be validated
# WARN: by running build_config_validate.sh

BUILD_CONFIG_VERSION='$Id$'

SHOW_DEBUG_INFO=false

usage ()
{
cat <<EOF
Usage: $0 [options]

-3|--32|--show_32bit_brands:
        Show 32bit linux build brands (optional)

-6|--62|--show_64bit_brands:
        Show 64bit linux build brands (optional)

-a|--show_active:
        Show active branches (with nightly builds or releases) (optional)

-b|--show_brands:
	Show default build brands (optional)

-c|--show_coverity_active:
        Show active coverity branches (with nightly coverity builds) (optional)

-d|--debug:
	Set debug mode (optional) [NOTE: Not implemented yet]

-h|--help:
	Show this usage text (optional)

-l|--list:
	List brands one per line given a platform (optional)

-p|--platform:
	Show brands for this platform (required)
	One of linux, windows, macos, netbsd

-q|--quiet:
	Show this usage text (optional) [NOTE: Not implemented yet]

-r|--revision:
	Show brands for this tag. If platform is set, restrict the list
	that particular platform (required)

-u|--show_ubuntu_brands:
        Show ubuntu linux build brands (optional)

-v|--verbose:
	Set verbose mode

EOF
    exit 1
}

# Print debug messages, when debug flag is turned on
dbg_msg ()
{
	msg=$1

	# If debug is enabled prefix message as DBG:
	if [ "$SHOW_DEBUG_INFO" == "true" ]; then
		echo "DBG: $msg"
	fi
}

## -------------------------------------------------------------------------
## Active Branches - used for build results reporting
## PC-OEM branches (hnd-stadev-list)   : BASS_* PBR_*
## ROUTER branches (hnd-routerdev-list): MILLAU_* COMANCHE2_*
## ESTA   branches (hnd-lpsta-list)    : RAPTOR* ROMTERM* F15* F16* FALCON*
## this comment needs updating
## -------------------------------------------------------------------------
active_branches_all=( \
   NIGHTLY \
)

## -------------------------------------------------------------------------
## Active Coverity Branches - used for coverity results reporting
## -------------------------------------------------------------------------
active_coverity_branches_all=( \
   BIS715GALA_BRANCH_7_21 \
   BIS747T144RC2_BRANCH_7_64 \
   BIS747T144RC2_TWIG_7_64_3 \
   BISON04T_BRANCH_7_14 \
   BISON04T_TWIG_7_14_131 \
   BISON05T_BRANCH_7_35 \
   BISON05T_TWIG_7_35_260_64 \
   BISON06T_BRANCH_7_45 \
   BISON06T_TWIG_7_45_45 \
   DHD_BRANCH_1_363 \
   DHD_BRANCH_1_579 \
   DHD_TWIG_1_363_45_58 \
   DIN2915T250RC1_BRANCH_9_30 \
   DIN2930R18_BRANCH_9_44 \
   EAGLE_BRANCH_10_10 \
   EAGLE_TWIG_10_10_122 \
   EAGLE_TWIG_10_10_69 \
   IGUANA_BRANCH_13_10 \
   IGUANA08T_BRANCH_13_35 \
   JAGUAR_BRANCH_14_10 \
   NIGHTLY \
)

active_branches_linux=(disabled)
active_branches_macos=(disabled)
active_branches_netbsd=(disabled)
active_branches_windows=(disabled)

## -------------------------------------------------------------------------
## Default list of build brands to be built for all tot/tob and release builds
## -------------------------------------------------------------------------

## -------------------------------------------------------------------------
# Default linux build brands for TAG builds (if not overriden below)
# DEFAULT is not a name of tag, it is a pseudo tag
# This should be obsolete from BISON_BRANCH_7_10 forward.
linux_brands_DEFAULT_=( \
   linux-internal-wl \
   linux26-internal-router \
   linux-external-dongle-sdio \
   linux-mfgtest-dongle-sdio \
)

# Default macos build brands for TAG builds (if not overriden below)
macos_brands_DEFAULT_=(disabled)

# Default netbsd build brands for TAG builds (if not overriden below)
netbsd_brands_DEFAULT_=(disabled)

# Default windows build brands for TAG builds (if not overriden below)
windows_brands_DEFAULT_=( \
   win_external_wl \
   win_internal_wl \
)

## -------------------------------------------------------------------------
## DINGO_BRANCH_* and DINGO_REL_* dongle brands so firmware images get built.

linux_brands_DINGO=( \
  linux-mfgtest-dongle-pcie \
  linux-internal-dongle-pcie \
  linux-external-dongle-pcie \
)

macos_brands_DINGO=(disabled)

netbsd_brands_DINGO=(disabled)

windows_brands_DINGO=(disabled)

## -------------------------------------------------------------------------
## BISON_BRANCH_* and BISON_REL_* dongle brands so firmware images get built.

linux_brands_BISON=( \
  android-external-dongle \
  linux-external-dongle-sdio \
  linux-external-dongle-pcie \
  linux-external-media \
  linux-internal-dongle \
  linux-internal-dongle-pcie \
  linux-internal-media \
  linux-mfgtest-dongle-sdio \
  linux-mfgtest-dongle-pcie \
  linux-mfgtest-media \
)

macos_brands_BISON=(disabled)

netbsd_brands_BISON=(disabled)

windows_brands_BISON=(disabled)

## -------------------------------------------------------------------------
## BISON_TWIG_7_10_120 dongle brands so firmware images get built.

linux_brands_BISON_TWIG_7_10_120=( \
  hndrte-dongle-wl \
  linux-external-dongle-pcie \
  linux-mfgtest-dongle-pcie \
)

macos_brands_BISON_TWIG_7_10_120=()
netbsd_brands_BISON_TWIG_7_10_120=()
windows_brands_BISON_TWIG_7_10_120=( \
  win_mfgtest_dongle_sdio \
  win_mfgtest_wl \
)

## -------------------------------------------------------------------------
## BISON_TWIG_7_10_226 dongle brands so firmware images get built.

linux_brands_BISON_TWIG_7_10_226=( \
  hndrte-dongle-wl \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
)

macos_brands_BISON_TWIG_7_10_226=(disabled)
netbsd_brands_BISON_TWIG_7_10_226=(disabled)
windows_brands_BISON_TWIG_7_10_226=(disabled)

## -------------------------------------------------------------------------

## BISON03T_BRANCH_* and BISON03T_REL_* dongle brands so firmware images get built.
linux_brands_BISON03T=( \
  hndrte-dongle-wl \
  linux-external-wl-media \
  linux-external-wl \
  linux-internal-wl \
  linux-internal-wl-media \
  linux-mfgtest-wl \
  linux26-internal-router \
  linux26-internal-usbap \
  linux26-external-usbap-full-src \
  linux-external-wl-portsrc-hybrid \
  linux-2.6.36-arm-internal-router-dhdap \
  linux-2.6.36-arm-external-vista-router-dhdap-full-src \
)

macos_brands_BISON03T=(disabled)

netbsd_brands_BISON03T=(disabled)

windows_brands_BISON03T=(disabled)

## -------------------------------------------------------------------------
## BISON04T_BRANCH_* and BISON04T_REL_* dongle brands so firmware images get built.

linux_brands_BISON04T=( \
  hndrte-dongle-wl \
  linux-2.6.36-arm-external-vista-router-full-src \
  linux-2.6.36-arm-external-vista-router-partial-src \
  linux-2.6.36-arm-internal-router \
  linux-2.6.36-arm-internal-router-dhdap \
  linux-2.6.36-arm-mfgtest-router-noramdisk \
  linux-2.6.36-arm-up-external-vista-router-full-src \
  linux-2.6.36-arm-up-internal-router \
  linux26-external-usbap-full-src \
  linux26-external-vista-router-full-src \
  linux26-internal-router \
  linux26-internal-usbap \
)

macos_brands_BISON04T=(disabled)

netbsd_brands_BISON04T=(disabled)

windows_brands_BISON04T=(disabled)

## -------------------------------------------------------------------------
## BISON05T_BRANCH_* and BISON05T_REL_* dongle brands so firmware images get built.

linux_brands_BISON05T=( \
  hndrte-dongle-wl \
  android-external-dongle \
  linux-2.6.36-arm-external-vista-router-dhdap-full-src \
  linux-2.6.36-arm-external-vista-router-full-src \
  linux-2.6.36-arm-internal-router \
  linux-2.6.36-arm-internal-router-dhdap \
  linux-2.6.36-arm-mfgtest-router-dhdap-noramdisk \
  linux-2.6.36-arm-mfgtest-router-noramdisk \
  linux-2.6.36-arm-up-external-vista-router-dhdap-full-src \
  linux26-external-usbap-full-src \
  linux26-external-vista-router-full-src \
  linux26-internal-router \
  linux26-internal-usbap \
  linux-external-dongle-pcie \
  linux-external-dongle-sdio \
  linux-external-media \
  linux-internal-media \
  linux-mfgtest-dongle-pcie \
  linux-mfgtest-dongle-sdio \
  linux-mfgtest-media \
)

macos_brands_BISON05T=(disabled)

netbsd_brands_BISON05T=(disabled)

windows_brands_BISON05T=(disabled)

## -------------------------------------------------------------------------
## BIS120RC4_BRANCH_* and BIS120RC4_REL_* dongle brands so firmware images get built.

linux_brands_BIS120RC4=( \
  hndrte-dongle-wl \
  linux-external-dongle-pcie \
  linux-mfgtest-dongle-pcie \
  linux-internal-wl \
)

macos_brands_BIS120RC4=( \
)

netbsd_brands_BIS120RC4=(disabled)

windows_brands_BIS120RC4=( \
  win8_external_wl \
  win8_internal_wl \
  win_internal_wl \
  win_external_wl \
  win_mfgtest_dongle_sdio \
  win_mfgtest_wl \
)

## -------------------------------------------------------------------------
## BIS120RC4PHY_BRANCH_* and BIS120RC4PHY_REL_* dongle brands so firmware images get built.

linux_brands_BIS120RC4PHY=( \
  linux-mfgtest-dongle-pcie \
)

macos_brands_BIS120RC4PHY=( \
)

netbsd_brands_BIS120RC4PHY=(disabled)

windows_brands_BIS120RC4PHY=( \
)

## -------------------------------------------------------------------------
## CARIBOU from Trunk
## Primary Use: many groups; Owners: sriramn

#- Default list of nightly TOT linux brands
linux_brands_CARIBOU=( \
  hndrte \
  hndrte-dongle-wl \
  linux-internal-dongle \
  linux-external-dongle-pcie\
  linux-internal-dongle-pcie\
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-pcie \
  linux-mfgtest-dongle-sdio \
)

macos_brands_CARIBOU=( \
)

netbsd_brands_CARIBOU=( \
)

windows_brands_CARIBOU=(disabled)


## -------------------------------------------------------------------------
## DHD_BRANCH_1_28 specific brands
## Primary Use: eSTA group; Owners: hnd-lpsta-list

linux_brands_DHD_BRANCH_1_28=( \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
)

macos_brands_DHD_BRANCH_1_28=(disabled)

netbsd_brands_DHD_BRANCH_1_28=(disabled)

windows_brands_DHD_BRANCH_1_28=(disabled)

## -------------------------------------------------------------------------
## DHD_BRANCH_1_141 specific brands
##

linux_brands_DHD_BRANCH_1_141=( \
  android-external-dongle \
  hndrte-dongle-wl \
  linux-external-dongle-pcie \
  linux-internal-dongle-pcie \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
)

macos_brands_DHD_BRANCH_1_141=(disabled)

netbsd_brands_DHD_BRANCH_1_141=(disabled)

windows_brands_DHD_BRANCH_1_141=(disabled)

## -------------------------------------------------------------------------
## DHD_BRANCH_1_201 specific brands
##

linux_brands_DHD_BRANCH_1_201=( \
  android-external-dongle \
  hndrte-dongle-wl \
  linux-external-dongle-pcie \
  linux-internal-dongle-pcie \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
)

macos_brands_DHD_BRANCH_1_201=(disabled)

netbsd_brands_DHD_BRANCH_1_201=(disabled)

windows_brands_DHD_BRANCH_1_201=(
  win_mfgtest_dongle_sdio \
)

## -------------------------------------------------------------------------
## DHD_REL_1_125 specific brands
##

linux_brands_DHD_REL_1_125=( \
  hndrte-dongle-wl \
  linux-external-dongle-pcie \
  linux-internal-dongle-pcie \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
)
macos_brands_DHD_REL_1_125=(disabled)
netbsd_brands_DHD_REL_1_125=(disabled)
windows_brands_DHD_REL_1_125=(disabled)

## -------------------------------------------------------------------------
## DHD_REL_1_126 specific brands
##

linux_brands_DHD_REL_1_126=(${linux_brands_DHD_REL_1_125[@]})
macos_brands_DHD_REL_1_126=(disabled)
netbsd_brands_DHD_REL_1_126=(disabled)
windows_brands_DHD_REL_1_126=(disabled)

## -------------------------------------------------------------------------
## DHD_REL_1_127 specific brands
##

linux_brands_DHD_REL_1_127=(${linux_brands_DHD_REL_1_125[@]})
macos_brands_DHD_REL_1_127=(disabled)
netbsd_brands_DHD_REL_1_127=(disabled)
windows_brands_DHD_REL_1_127=(disabled)

## -------------------------------------------------------------------------
## DHD_REL_1_128 specific brands
##

linux_brands_DHD_REL_1_128=(${linux_brands_DHD_REL_1_125[@]})
macos_brands_DHD_REL_1_128=(disabled)
netbsd_brands_DHD_REL_1_128=(disabled)
windows_brands_DHD_REL_1_128=(disabled)

## -------------------------------------------------------------------------
## DHD_REL_1_129 specific brands
##

linux_brands_DHD_REL_1_129=(${linux_brands_DHD_REL_1_125[@]})
macos_brands_DHD_REL_1_129=(disabled)
netbsd_brands_DHD_REL_1_129=(disabled)
windows_brands_DHD_REL_1_129=(disabled)

## -------------------------------------------------------------------------
## DHD_REL_1_130 specific brands
##

linux_brands_DHD_REL_1_130=(${linux_brands_DHD_REL_1_125[@]})
macos_brands_DHD_REL_1_130=(disabled)
netbsd_brands_DHD_REL_1_130=(disabled)
windows_brands_DHD_REL_1_130=(disabled)


## -------------------------------------------------------------------------
## AARDVARK_BRANCH_* and AARDVARK_REL_* specific brands
## Primary Use: STA group; Owners: hnd-stadev-list (stafford/davidm)

linux_brands_AARDVARK=( \
  hndrte-dongle-wl \
  hndrte \
  linux-external-wl \
  linux-internal-wl \
  linux-internal-wl-media \
  linux-external-wl-media-full-src \
  linux-internal-dongle \
  linux-internal-dongle-usb \
  linux-mfgtest-dongle-usb \
  linux-mfgtest-dongle-sdio \
  linux-mfgtest-wl \
  linux26-internal-router \
  linux26-external-vista-router-full-src \
  linux26-external-vista-router-partial-src \
  linux26-external-vista-router-combo \
  linux26-mfgtest-router-noramdisk \
  linux26-mfgtest-router \
  linux26-internal-usbap \
  linux26-external-usbap-full-src \
  linux-external-wl-portsrc-hybrid \
  linux-external-dongle-usb \
  linux-external-dongle-sdio \
  linux26-external-router-mini-full-src \
  linux26-external-router-mini-partial-src \
)

macos_brands_AARDVARK=( \
  macos-external-wl-lion \
  macos-internal-wl-lion \
  macos-internal-wl-ml \
  macos-external-wl-ml \
  macos-external-wl-cab \
  macos-internal-wl-cab \
)

netbsd_brands_AARDVARK=( \
  netbsd-external-wl \
)

windows_brands_AARDVARK=( \
  win_internal_wl \
  win_external_wl \
  win_mfgtest_wl \
  win8_external_wl \
  win8_internal_wl \
  winblue_external_wl \
  winblue_internal_wl \
  win_mfgtest_dongle_sdio \
  efi-external-wl \
  efi-internal-wl \
)


## -------------------------------------------------------------------------
## AARDVARK01T_BRANCH_* and AARDVARK01T_REL_* train branches
## Primary Use: ??? group; Owners: hnd-build-list ()

# First AARDVARK Train, tail end of old B/TW/REL model
linux_brands_AARDVARK01T=( \
  ${linux_brands_AARDVARK[@]} \
  linux26-external-usbap-partial-src \
)
macos_brands_AARDVARK01T=(${macos_brands_AARDVARK[@]})
netbsd_brands_AARDVARK01T=(${netbsd_brands_AARDVARK[@]})
windows_brands_AARDVARK01T=( \
  ${windows_brands_AARDVARK[@]} \
  win_mfgtest_wl \
  winblue_external_wl \
  winblue_internal_wl \
)

## -------------------------------------------------------------------------
## AARDVARK01T_TWIG_6_37_3 specific brands
## -------------------------------------------------------------------------

linux_brands_AARDVARK01T_TWIG_6_37_3=( \
  hndrte \
  hndrte-dongle-wl \
  linux-external-wl \
  linux-internal-wl \
  linux-internal-wl-media \
  linux-external-wl-media-full-src \
  linux-external-wl-portsrc-hybrid \
  linux-internal-dongle \
  linux-external-dongle-usb \
  linux-internal-dongle-usb \
  linux-mfgtest-dongle-usb \
  linux-mfgtest-wl \
)
macos_brands_AARDVARK01T_TWIG_6_37_3=(disabled)
netbsd_brands_AARDVARK01T_TWIG_6_37_3=(disabled)
windows_brands_AARDVARK01T_TWIG_6_37_3=(win_mfgtest_wl)

## -------------------------------------------------------------------------
## AARDVARK_TWIG_6_30_301 specific brands
## needs dongle builds for wl development on windows
## -------------------------------------------------------------------------

linux_brands_AARDVARK_TWIG_6_30_301=( \
  hndrte-dongle-wl \
)
macos_brands_AARDVARK_TWIG_6_30_301=(disabled)
netbsd_brands_AARDVARK_TWIG_6_30_301=(disabled)
windows_brands_AARDVARK_TWIG_6_30_301=(disabled)

## -------------------------------------------------------------------------
## AARDVARK_TWIG_6_30_302 specific brands
## needs dongle builds for wl development on windows
## -------------------------------------------------------------------------

linux_brands_AARDVARK_TWIG_6_30_302=( \
  hndrte-dongle-wl \
)
macos_brands_AARDVARK_TWIG_6_30_302=(disabled)
netbsd_brands_AARDVARK_TWIG_6_30_302=(disabled)
windows_brands_AARDVARK_TWIG_6_30_302=(disabled)

## -------------------------------------------------------------------------
## AARDVARK_TWIG_6_30_9 specific brands
## Primary Use: ESTA group; Owners: hnd-media-list

linux_brands_AARDVARK_TWIG_6_30_9=( \
  hndrte-dongle-wl \
  linux-external-wl \
  linux-internal-wl \
  linux-internal-wl-media \
  linux-external-wl-media-full-src \
  linux26-internal-router \
)

macos_brands_AARDVARK_TWIG_6_30_9=(disabled)

netbsd_brands_AARDVARK_TWIG_6_30_9=(disabled)

windows_brands_AARDVARK_TWIG_6_30_9=( \
  win_external_wl \
  win_internal_wl \
  win_mfgtest_wl \
  win_wps_enrollee \
)


## -------------------------------------------------------------------------
## AARDVARK_TWIG_6_30_207 specific brands
## -------------------------------------------------------------------------

linux_brands_AARDVARK_TWIG_6_30_207=( \
  hndrte-dongle-wl \
  linux-external-dongle-sdio \
  linux-external-dongle-usb \
  linux-mfgtest-dongle-sdio \
  linux-mfgtest-dongle-usb \
)

macos_brands_AARDVARK_TWIG_6_30_207=(disabled)

netbsd_brands_AARDVARK_TWIG_6_30_207=(disabled)

windows_brands_AARDVARK_TWIG_6_30_207=(disabled)

## -------------------------------------------------------------------------
## AARDVARK_TWIG_6_30_223 specific brands
## -------------------------------------------------------------------------

linux_brands_AARDVARK_TWIG_6_30_223=( \
  hndrte-dongle-wl \
  linux-external-wl \
  linux-internal-wl \
  linux-internal-wl-media \
  linux-external-wl-media-full-src \
  linux-internal-dongle \
  linux-internal-dongle-usb \
  linux-mfgtest-dongle-usb \
  linux-mfgtest-dongle-sdio \
  linux-mfgtest-wl \
  linux26-internal-usbap \
  linux26-external-usbap-full-src \
  linux-external-wl-portsrc-hybrid \
  linux-external-dongle-usb \
  linux-external-dongle-sdio \
)

macos_brands_AARDVARK_TWIG_6_30_223=(\
  macos-external-wl-lion \
  macos-internal-wl-lion \
  macos-external-wl-cab \
  macos-internal-wl-cab \
  macos-external-wl-ml \
  macos-internal-wl-ml \
)

netbsd_brands_AARDVARK_TWIG_6_30_223=(\
  netbsd-external-wl \
)

windows_brands_AARDVARK_TWIG_6_30_223=(\
  win_internal_wl \
  win_external_wl \
  win_mfgtest_wl \
  win8_external_wl \
  win8_internal_wl \
  win_mfgtest_dongle_sdio \
  efi-external-wl \
  efi-internal-wl \
  winblue_external_wl \
  winblue_internal_wl \
)



## -------------------------------------------------------------------------
## AARDVARK_TWIG_6_30_235 specific brands
## -------------------------------------------------------------------------

linux_brands_AARDVARK_TWIG_6_30_235=( \
  linux-mfgtest-dongle-pcie \
  linux-internal-dongle-pcie \
)

macos_brands_AARDVARK_TWIG_6_30_235=(disabled)
netbsd_brands_AARDVARK_TWIG_6_30_235=(disabled)
windows_brands_AARDVARK_TWIG_6_30_235=(disabled)

## -------------------------------------------------------------------------
## AARDVARK_TWIG_6_30_250 specific brands
## -------------------------------------------------------------------------

linux_brands_AARDVARK_TWIG_6_30_250=( \
  hndrte-dongle-wl \
  linux-external-dongle-sdio \
  linux-external-dongle-usb \
  linux-mfgtest-dongle-sdio \
  linux-mfgtest-dongle-usb \
)
macos_brands_AARDVARK_TWIG_6_30_250=(disabled)
netbsd_brands_AARDVARK_TWIG_6_30_250=(disabled)
windows_brands_AARDVARK_TWIG_6_30_250=(disabled)


## -------------------------------------------------------------------------
## AARDVARK_TWIG_6_30_362 specific brands
## -------------------------------------------------------------------------

linux_brands_AARDVARK_TWIG_6_30_362=( \
  hndrte-dongle-wl \
  linux-external-dongle-pcie \
  linux-mfgtest-dongle-pcie \
)
macos_brands_AARDVARK_TWIG_6_30_362=(disabled)
netbsd_brands_AARDVARK_TWIG_6_30_362=(disabled)
windows_brands_AARDVARK_TWIG_6_30_362=( \
  win_mfgtest_dongle_sdio \
  win_mfgtest_wl \
)



## -------------------------------------------------------------------------
## Example to set smaller subset of brands for a given REL
## This can be used currently, for very large scope branches
## like aardvark, where default list tend to grow.
##
## AARDVARK_REL_6_30_1_x specific brands
## Primary Use: ESTA group; Owners: hnd-routerdev-list

linux_brands_AARDVARK_REL_6_30_1=(${linux_brands_AARDVARK_TWIG_6_30_[@]})
macos_brands_AARDVARK_REL_6_30_1=(disabled)
netbsd_brands_AARDVARK_REL_6_30_1=(disabled)
windows_brands_AARDVARK_REL_6_30_1=(disabled)


## -------------------------------------------------------------------------
## PHOENIX2_TWIG_6_10_9 specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_PHOENIX2_TWIG_6_10_9=( \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
  linux-internal-dongle \
  linux-external-dongle-usb \
  linux-internal-dongle-usb \
  linux-mfgtest-dongle-usb \
  linux-internal-cxapi \
  linux-external-cxapi \
  linux-internal-wl \
  hndrte-dongle-wl \
  hndrte \
)

macos_brands_PHOENIX2_TWIG_6_10_9=(disabled)

netbsd_brands_PHOENIX2_TWIG_6_10_9=(disabled)

windows_brands_PHOENIX2_TWIG_6_10_9=( \
  win_external_dongle_sdio \
  win_mfgtest_dongle_sdio \
  nucleus-external-dongle-sdio \
  win_mfgtest_dongle_usb \
  win_internal_dongle_sdio \
)

## -------------------------------------------------------------------------
## PHOENIX2_TWIG_6_10_* (PHOENIX2 6.10.153) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_PHOENIX2_TWIG_6_10_153=( \
  linux-mfgtest-dongle-sdio \
  linux-external-dongle-usb \
  linux-mfgtest-dongle-usb \
  hndrte-dongle-wl \
)

macos_brands_PHOENIX2_TWIG_6_10_153=(disabled)

netbsd_brands_PHOENIX2_TWIG_6_10_153=(disabled)

windows_brands_PHOENIX2_TWIG_6_10_153=( \
  win_mfgtest_dongle_sdio \
)
## -------------------------------------------------------------------------
## PHOENIX2_TWIG_6_10_* (PHOENIX2 6.10.173) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list (Burak)

linux_brands_PHOENIX2_TWIG_6_10_173=( \
  linux-olympic-dongle-src \
  linux-external-dongle-usb \
  linux-mfgtest-dongle-usb \
  hndrte-dongle-wl \
  hndrte \
)

macos_brands_PHOENIX2_TWIG_6_10_173=(disabled)

netbsd_brands_PHOENIX2_TWIG_6_10_173=(disabled)

windows_brands_PHOENIX2_TWIG_6_10_173=( \
  win_mfgtest_dongle_sdio \
)

## -------------------------------------------------------------------------
## PHOENIX2_TWIG_6_10_* (PHOENIX2 6.10.167) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list (Burak)

linux_brands_PHOENIX2_TWIG_6_10_167=( \
  linux-olympic-dongle-src \
  linux-external-dongle-usb \
  linux-mfgtest-dongle-usb \
  hndrte-dongle-wl \
  hndrte \
)

macos_brands_PHOENIX2_TWIG_6_10_167=(disabled)

netbsd_brands_PHOENIX2_TWIG_6_10_167=(disabled)

windows_brands_PHOENIX2_TWIG_6_10_167=( \
  win_mfgtest_dongle_sdio \
)

## -------------------------------------------------------------------------
## PHOENIX2PHY_BRANCH* (PHOENIX2PHY 6.12) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_PHOENIX2PHY=( \
  linux-mfgtest-dongle-sdio \
  linux-mfgtest-dongle-usb \
  hndrte-dongle-wl \
)

macos_brands_PHOENIX2PHY=(disabled)

netbsd_brands_PHOENIX2PHY=(disabled)

windows_brands_PHOENIX2PHY=( \
  win_mfgtest_dongle_sdio \
  win_mfgtest_dongle_usb \
)

## -------------------------------------------------------------------------
## PHO2203RC1_BRANCH specific brands
## Primary Use: olympic-list

linux_brands_PHO2203RC1=( \
  linux-olympic-dongle-src \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
  linux-external-dongle-usb \
  linux-mfgtest-dongle-usb \
  hndrte-dongle-wl \
)
macos_brands_PHO2203RC1=(disabled)

netbsd_brands_PHO2203RC1=(disabled)

windows_brands_PHO2203RC1=( \
  win_mfgtest_dongle_sdio \
)

## -------------------------------------------------------------------------
## CLMROMDEV Default Brands
## Primary Use: all groups; Owners: hnd-software-clm-list

#- Default list of CLMROMDEV linux brands
linux_brands_CLMROMDEV=( \
  hndrte-dongle-wl \
  linux-internal-wl \
  linux-internal-router \
  linux-internal-dongle \
)

macos_brands_CLMROMDEV=(disabled)

netbsd_brands_CLMROMDEV=(disabled)

windows_brands_CLMROMDEV=( \
 win_internal_wl \
 win_external_dongle_sdio \
)

## -------------------------------------------------------------------------
## AKASHI BRANCH 5.110
## Primary Use: Router group; Owners: hnd-routerdev-list

linux_brands_AKASHI=( \
  linux-external-router \
  linux-internal-dslcpe \
  linux-internal-router \
  linux-mfgtest-router \
  linux26-external-vista-router-full-src \
  linux26-external-vista-router-partial-src \
  linux26-internal-router \
)

macos_brands_AKASHI=(disabled)

netbsd_brands_AKASHI=(disabled)

windows_brands_AKASHI=(disabled)

## -------------------------------------------------------------------------
## HARLEY REL 5.112
## HARLEY REL builds are svn*COPY* of AKASHIREL_BRANCH_5_110 for media
## Primary Use: Media group; Owners: WlanMedia-list

linux_brands_HARLEY=( \
  linux-external-router-media-partial-src \
  linux-internal-router-media \
  linux-mfgtest-router-media \
  linux26-external-router-media-partial-src \
  linux26-external-router-media-full-src \
  linux-external-router-media-full-src \
)

macos_brands_HARLEY=(disabled)

netbsd_brands_HARLEY=(disabled)

windows_brands_HARLEY=(disabled)

## -------------------------------------------------------------------------
## AKASHI TWIG 5.110.1
## Primary Use: Router group; Owners: hnd-routerdev-list

linux_brands_AKASHI_TWIG_5_110_1=( \
  linux-internal-router \
  linux-external-router \
  linux-external-vista-router-partial-src \
  linux-external-vista-wapi-media-router-full-src \
  linux26-external-vista-router-full-src \
)

macos_brands_AKASHI_TWIG_5_110_1=(disabled)

netbsd_brands_AKASHI_TWIG_5_110_1=(disabled)

windows_brands_AKASHI_TWIG_5_110_1=( \
  ${windows_brands_AKASHI[@]} \
)

## -------------------------------------------------------------------------
## AKASHI TWIG 5_110_27
## Primary Use: Router group; Owners: hnd-routerdev-list

linux_brands_AKASHI_TWIG_5_110_27=( \
  linux-external-vista-router-partial-src \
  linux-external-vista-wapi-router-full-src \
  linux-internal-router \
  linux-mfgtest-router \
  linux26-external-router-mini-full-src \
  linux26-external-vista-router-full-src \
  linux26-internal-router \
)

macos_brands_AKASHI_TWIG_5_110_27=(disabled)

netbsd_brands_AKASHI_TWIG_5_110_27=(disabled)

windows_brands_AKASHI_TWIG_5_110_27=(disabled)

## -------------------------------------------------------------------------
## AKASHI TWIG 5_110_58
## Primary Use: Router group; Owners: hnd-routerdev-list

linux_brands_AKASHI_TWIG_5_110_58=( \
  linux-mfgtest-router \
  linux26-external-plc-router-full-src \
  linux26-external-plc-router-partial-src \
  linux26-internal-router \
)

macos_brands_AKASHI_TWIG_5_110_58=(disabled)

netbsd_brands_AKASHI_TWIG_5_110_58=(disabled)

windows_brands_AKASHI_TWIG_5_110_58=(disabled)

## -------------------------------------------------------------------------
## AKASHI TWIG 5_110_64
## Primary Use: Router group; Owners: hnd-routerdev-list

linux_brands_AKASHI_TWIG_5_110_64=( \
  linux-internal-router-media \
  linux-external-router-media-full-src \
  linux-external-router-media-partial-src \
  linux-internal-router \
)

macos_brands_AKASHI_TWIG_5_110_64=(disabled)

netbsd_brands_AKASHI_TWIG_5_110_64=(disabled)

windows_brands_AKASHI_TWIG_5_110_64=(disabled)

## -------------------------------------------------------------------------
## DUCATI_BRANCH_5_24 and DUCATI_REL_5_24* specific brands
## Primary Use: Router group; Owners: WlanMedia-list

linux_brands_DUCATI=( \
   linux-internal-router \
)

macos_brands_DUCATI=(disabled)

netbsd_brands_DUCATI=(disabled)

windows_brands_DUCATI=(disabled)

## -------------------------------------------------------------------------
## MILLAU_TWIG_5_70_3 and MILLAU_REL_5_70_3_* specific brands
## Primary Use: Router group; Owners: hnd-routerdev-list

linux_brands_MILLAU_TWIG_5_70_3=( \
   linux-internal-router \
   linux-external-vista-router-partial-src \
   linux26-internal-router \
   linux26-external-vista-router-partial-src \
)

macos_brands_MILLAU_TWIG_5_70_3=(disabled)

netbsd_brands_MILLAU_TWIG_5_70_3=(disabled)

windows_brands_MILLAU_TWIG_5_70_3=(disabled)

## -------------------------------------------------------------------------
## MILLAU_BRANCH_5_70* and MILLAU_REL_5_70* specific brands
## Primary Use: Router group; Owners: hnd-routerdev-list

linux_brands_MILLAU=( \
   linux-internal-router \
   linux-external-router \
   linux-external-router-full-src \
   linux-external-vista-router-partial-src \
   linux-external-usbap-full-src \
   linux-internal-router-phydev \
   linux-internal-router-sdio-std \
   linux-mfgtest-router \
   linux26-internal-router \
   linux26-external-vista-router-partial-src \
   linux26-external-usbap-full-src \
)

macos_brands_MILLAU=(disabled)

netbsd_brands_MILLAU=(disabled)

windows_brands_MILLAU=(disabled)

## -------------------------------------------------------------------------
## KIRIN_BRANCH_* and KIRIN_REL_** specific brands
## Primary Use: STA group; Owners: hnd-stadev-list

linux_brands_KIRIN=( \
  linux-external-wl \
  linux-internal-wl \
  hndrte-dongle-wl \
  linux-external-wl-media-full-src \
  linux-internal-wl-media \
  linux-mfgtest-wl-media-full-src \
  linux-external-wl-portsrc-hybrid \
)

macos_brands_KIRIN=( \
  macos-external-wl-lion \
  macos-internal-wl-lion \
)

netbsd_brands_KIRIN=(disabled)

windows_brands_KIRIN=( \
  win_external_wl \
  win_internal_wl \
  win_mfgtest_wl \
)

## -------------------------------------------------------------------------
## KIRIN_TWIG_5_100_82* and KIRIN_REL_5_100_82* specific brands
## Primary Use: STA group; Owners: hnd-stadev-list

linux_brands_KIRIN_TWIG_5_100_82=( \
  linux-internal-wl \
  linux-external-wl-portsrc-hybrid \
)

macos_brands_KIRIN_TWIG_5_100_82=(disabled)

netbsd_brands_KIRIN_TWIG_5_100_82=(disabled)

windows_brands_KIRIN_TWIG_5_100_82=( \
  win_internal_wl \
  win_external_wl \
  win_mfgtest_wl \
)

## -------------------------------------------------------------------------
## KIR106RC58_TWIG_5_106_198* and KIR106RC58_REL_5_106_198* specific brands
## Primary Use: STA group; Owners: hnd-stadev-list

linux_brands_KIR106RC58_TWIG_5_106_198=(\
  linux-internal-wl \
  linux-external-wl \
)

macos_brands_KIR106RC58_TWIG_5_106_198=( \
  macos-internal-wl-lion \
  macos-external-wl-lion \
)

netbsd_brands_KIR106RC58_TWIG_5_106_198=(disabled)

windows_brands_KIR106RC58_TWIG_5_106_198=( \
  win_internal_wl \
  win_external_wl \
  win_mfgtest_wl \
  efi-external-wl \
  efi-internal-wl \
)

## -------------------------------------------------------------------------
## KIRIN_TWIG_5_102_98* and KIRIN_REL_5_102_98* specific brands
## Primary Use: STA group; Owners: WlanMedia-list

linux_brands_KIRIN_TWIG_5_102_98=( \
  hndrte-dongle-wl \
  linux-internal-wl \
  linux-internal-wl-media \
  linux-external-wl-media-full-src \
  linux-mfgtest-wl-media-full-src \
)

macos_brands_KIRIN_TWIG_5_102_98=(disabled)

netbsd_brands_KIRIN_TWIG_5_102_98=(disabled)

windows_brands_KIRIN_TWIG_5_102_98=( \
  win_external_wl \
  win_mfgtest_wl \
)

## -------------------------------------------------------------------------
## KIRIN_TWIG_5_100_51* and KIRIN_TWIG_5_100_51_* specific brands
## Primary Use: Router group; Owners: hnd-routerdev-list

linux_brands_KIRIN_TWIG_5_100_51=( \
  netbsd-internal-router \
)

macos_brands_KIRIN_TWIG_5_100_51=(disabled)

netbsd_brands_KIRIN_TWIG_5_100_51=(disabled)

windows_brands_KIRIN_TWIG_5_100_51=(disabled)

## -------------------------------------------------------------------
## PBR specific brands
## Primary Use : Connectivity group; Owners hnd-stadev-list

linux_brands_PBR=(disabled)

macos_brands_PBR=( \
  macos-internal-wl-lion \
  macos-external-wl-lion \
)

netbsd_brands_PBR=(disabled)

windows_brands_PBR=(disabled)


## -------------------------------------------------------------------------
## PBR_TWIG_5_10_131* and PBR_TWIG_5_10_131* specific brands
## Primary Use: STA group; Owners: hnd-stadev-list

linux_brands_PBR_TWIG_5_10_131=(disabled)

macos_brands_PBR_TWIG_5_10_131=( \
  macos-internal-wl-lion \
  macos-external-wl-lion \
)

netbsd_brands_PBR_TWIG_5_10_131=(disabled)

windows_brands_PBR_TWIG_5_10_131=(disabled)

## -------------------------------------------------------------------------
## FALCON_TWIG* (FALCON 5.90.100) specific brands (10Q4 release)
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_FALCON_TWIG_5_90_100=( \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
  linux-internal-dongle \
  hndrte-dongle-wl \
)
macos_brands_FALCON_TWIG_5_90_100=(disabled)

netbsd_brands_FALCON_TWIG_5_90_100=(disabled)

windows_brands_FALCON_TWIG_5_90_100=( \
  win_external_dongle_sdio \
  win_mfgtest_dongle_sdio \
  nucleus-external-dongle-sdio \
  win_internal_dongle_sdio \
)

## -------------------------------------------------------------------------
## FAL100RC12_BRANCH_5_94* (FALCON 5.94.x) specific brands (10Q4 release)
## 4336 HP support twig, Primary Owner: Eric Stucki
## Primary Use: ESTA group; Owners: Eric Stucki, Graeme Cox

linux_brands_FAL100RC12=(${linux_brands_FALCON_TWIG_5_90_100[@]})
macos_brands_FAL100RC12=(${macos_brands_FALCON_TWIG_5_90_100[@]})
netbsd_brands_FAL100RC12=(${netbsd_brands_FALCON_TWIG_5_90_100[@]})
windows_brands_FAL100RC12=(${windows_brands_FALCON_TWIG_5_90_100[@]})


## -------------------------------------------------------------------------
## FALCON_TWIG* (FALCON 5.92.125) specific brands (Blade release)
## Primary Use: ESTA group; Owners: hnd-media-list

linux_brands_FALCON_TWIG_5_92_125=( \
  linux-external-dongle-usb \
  linux-internal-dongle-usb \
  hndrte-dongle-wl \
)

macos_brands_FALCON_TWIG_5_92_125=(disabled)

netbsd_brands_FALCON_TWIG_5_92_125=(disabled)

windows_brands_FALCON_TWIG_5_92_125=(disabled)


## -------------------------------------------------------------------------
## FALCON_TWIG* (FALCON 5.90.126) specific brands (Nokia twig)
## Primary Use: ESTA group; Owners: yuex

linux_brands_FALCON_TWIG_5_90_126=( \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
  linux-internal-dongle \
  hndrte-dongle-wl \
)

macos_brands_FALCON_TWIG_5_90_126=(disabled)

netbsd_brands_FALCON_TWIG_5_90_126=(disabled)

windows_brands_FALCON_TWIG_5_90_126=( \
  win_mfgtest_dongle_sdio \
)

## -------------------------------------------------------------------------
## FALCON_TWIG* (FALCON 5.90.153) specific brands (Cyclops release)
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_FALCON_TWIG_5_90_153=( \
  linux-external-dongle-sdio \
  hndrte-dongle-wl \
)

macos_brands_FALCON_TWIG_5_90_153=(disabled)

netbsd_brands_FALCON_TWIG_5_90_153=(disabled)

windows_brands_FALCON_TWIG_5_90_153=( \
  win_external_dongle_sdio \
  win_mfgtest_dongle_sdio \
  win_internal_dongle_sdio \
)

## -------------------------------------------------------------------------
## FALCON_TWIG* (FALCON 5.90.156) specific brands (Olympic Telluride release)
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_FALCON_TWIG_5_90_156=( \
  linux-external-dongle-usb \
  linux-mfgtest-dongle-usb \
  linux-internal-dongle-usb \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
  linux-internal-dongle \
  linux-external-cxapi \
  linux-internal-secfrw \
  hndrte-dongle-wl \
)

macos_brands_FALCON_TWIG_5_90_156=(disabled)

netbsd_brands_FALCON_TWIG_5_90_156=(disabled)

windows_brands_FALCON_TWIG_5_90_156=( \
  win_mfgtest_dongle_sdio \
  win_mfgtest_dongle_usb \
)

## -------------------------------------------------------------------------
## FALCON_TWIG* (FALCON 5.90.188)
## Primary Use: UTF

## -------------------------------------------------------------------------
## FALCON_TWIG* (FALCON 5.90.228)
linux_brands_FALCON_TWIG_5_90_228=( \
  linux-external-dongle-usb \
  linux-internal-dongle-usb \
  linux-external-dongle-usb-media \
  hndrte-dongle-wl \
  linux-mfgtest-dongle-usb \
  linux-internal-wl \
)
macos_brands_FALCON_TWIG_5_90_228=(disabled)
netbsd_brands_FALCON_TWIG_5_90_228=(disabled)
windows_brands_FALCON_TWIG_5_90_228=(disabled)

## -------------------------------------------------------------------------
## FAL195RC100_TWIG_5_90_208 (FALCON 5.90.208.x) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_FAL195RC100_TWIG_5_90_208=( \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
  hndrte-dongle-wl \
)

macos_brands_FAL195RC100_TWIG_5_90_208=(disabled)

netbsd_brands_FAL195RC100_TWIG_5_90_208=(disabled)

windows_brands_FAL195RC100_TWIG_5_90_208=(disabled)

## -------------------------------------------------------------------------
## FAL156RC30_TWIG_5_95_35* (FALCON 5.95.35.x) specific brands (Olympic Hoodoo point patch)
## Primary Use: ESTA group; Owners: fire-sw-list

linux_brands_FAL156RC30_TWIG_5_95_35=( \
  linux-external-dongle-usb \
  linux-internal-dongle-usb \
  linux-mfgtest-dongle-usb \
  hndrte-dongle-wl \
)
macos_brands_FAL156RC30_TWIG_5_95_35=(disabled)

netbsd_brands_FAL156RC30_TWIG_5_95_35=(disabled)

windows_brands_FAL156RC30_TWIG_5_95_35=( \
  win_mfgtest_dongle_sdio \
)

## FAL156RC30_BRANCH_5_95* (FALCON 5.95.x) specific brands (Olympic Hoodoo)


## -------------------------------------------------------------------------
## FALCONPHY_BRANCH* (FALCONPHY 5.92) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_FALCONPHY=( \
  linux-mfgtest-dongle-sdio \
  linux-mfgtest-dongle-usb \
)

macos_brands_FALCONPHY=(disabled)

netbsd_brands_FALCONPHY=(disabled)

windows_brands_FALCONPHY=(disabled)

## -------------------------------------------------------------------------
## RT2TWIG46_BRANCH_4_221_* (RT2TWIG46 4.221.x) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_RT2TWIG46=( \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
  linux-internal-dongle \
  hndrte-dongle-wl \
  linux-external-cxapi \
)

macos_brands_RT2TWIG46=(disabled)

netbsd_brands_RT2TWIG46=(disabled)

windows_brands_RT2TWIG46=( \
  win_mfgtest_dongle_sdio \
  win_internal_dongle_sdio \
)

## -------------------------------------------------------------------------
## RT2TWIG46_TWIG_4_221_90_* (RT2TWIG46 TWIG 4.221.90.x) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_RT2TWIG46_TWIG_4_221_90=( \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
  linux-internal-dongle \
  hndrte-dongle-wl \
  linux-external-cxapi \
)

macos_brands_RT2TWIG46_TWIG_4_221_90=(disabled)

netbsd_brands_RT2TWIG46_TWIG_4_221_90=(disabled)

windows_brands_RT2TWIG46_TWIG_4_221_90=( \
  win_mfgtest_dongle_sdio \
  win_internal_dongle_sdio \
)

## -------------------------------------------------------------------------
## ROMTERM2_TWIG* (Raptor 4.219.57) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list/G2 team

linux_brands_ROMTERM2_TWIG_4_219_57=( \
  linux-internal-dongle \
  linux-external-dongle-sdio \
  linux-external-dongle-usb \
  hndrte-dongle-wl \
  hndrte \
)

macos_brands_ROMTERM2_TWIG_4_219_57=(disabled)

netbsd_brands_ROMTERM2_TWIG_4_219_57=(disabled)

windows_brands_ROMTERM2_TWIG_4_219_57=( \
  win_external_dongle_sdio \
  win_internal_dongle_sdio \
)
## -------------------------------------------------------------------------
## ROMTERM2_BRANCH* (Romterm2 4.219) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_ROMTERM2=( \
  linux-internal-dongle \
  linux-mfgtest-dongle-sdio \
  hndrte-dongle-wl \
)

macos_brands_ROMTERM2=(disabled)

netbsd_brands_ROMTERM2=(disabled)

windows_brands_ROMTERM2=( \
  uitron-external-dongle-sdio-mcc \
  uitron-internal-dongle-sdio-mcc \
)

## -------------------------------------------------------------------------
## ROMTERM3_BRANCH* (ROMTERM3 4.220) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_ROMTERM3=( \
  linux-internal-dongle \
)

macos_brands_ROMTERM3=(disabled)

netbsd_brands_ROMTERM3=(disabled)

windows_brands_ROMTERM3=( \
  win_internal_dongle_sdio \
)

## -------------------------------------------------------------------------
## ROMTERM_TWIG_4_218_234* (Romterm 4.218.234) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_ROMTERM_TWIG_4_218_234=( \
  hndrte-dongle-wl \
)

macos_brands_ROMTERM=(disabled)

netbsd_brands_ROMTERM=(disabled)

windows_brands_ROMTERM_TWIG_4_218_234=( \
  wp7_external_dongle_sdio \
)

## -------------------------------------------------------------------------
## ROMTERM_BRANCH* (Romterm 4.218) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_ROMTERM=( \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
  linux-internal-dongle \
  linux-external-dongle-usb \
  linux-internal-dongle-usb \
  linux-mfgtest-dongle-usb \
  hndrte-dongle-wl \
  hndrte \
)

macos_brands_ROMTERM=(disabled)

netbsd_brands_ROMTERM=(disabled)

windows_brands_ROMTERM=( \
  win_mfgtest_dongle_sdio \
  win_mfgtest_dongle_usb \
  nucleus-external-dongle-sdio \
)

## -------------------------------------------------------------------------
## RT1TWIG227_BRANCH* (Romterm 4.222) specific brands
## Primary Use: ESTA group Nokia; Owners: nokia-sw-internal-list

linux_brands_RT1TWIG227=( \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
  linux-internal-dongle \
  hndrte-dongle-wl \
)

macos_brands_RT1TWIG227=(disabled)

netbsd_brands_RT1TWIG227=(disabled)

windows_brands_RT1TWIG227=( \
  win_mfgtest_dongle_sdio \
)

## -------------------------------------------------------------------------
## RAPTOR3_BRANCH* (Raptor3 4.230) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_RAPTOR3=(disabled)

macos_brands_RAPTOR3=(disabled)

netbsd_brands_RAPTOR3=(disabled)

windows_brands_RAPTOR3=( \
  win_mfgtest_dongle_sdio \
)

## -------------------------------------------------------------------------
## RAPTOR2_BRANCH_* (Raptor2 4.217) specific brands
## Primary Use: ESTA group; Owners: hnd-lpsta-list

linux_brands_RAPTOR2=( \
  linux-external-dongle-sdio \
  linux-mfgtest-dongle-sdio \
  linux-internal-dongle \
  hndrte-dongle-wl \
  linux-internal-cxapi \
  linux-external-cxapi \
)

macos_brands_RAPTOR2=(disabled)

netbsd_brands_RAPTOR2=(disabled)

windows_brands_RAPTOR2=( \
  win_mfgtest_dongle_sdio \
  win_external_dongle_usb \
  win_mfgtest_dongle_usb \
  nucleus-external-dongle-sdio \
  win_internal_dongle_sdio \
)

## -------------------------------------------------------------------------
## WL60G_BRANCH_* and WL60G_REL_* dongle brands so firmware images get built.

linux_brands_WL60G=( \
  hndrte-dongle-wl \
  linux-external-dongle-pcie \
  linux-mfgtest-dongle-pcie \
  linux-internal-wl \
)

macos_brands_WL60G=( \
  macos-external-wl-cab \
  macos-internal-wl-cab \
)

netbsd_brands_WL60G=(disabled)

windows_brands_WL60G=(disabled)

## -------------------------------------------------------------------------
## FALTWIG97_BRANCH_5_93 brands
## Primary Use: Win8 slate

linux_brands_FALTWIG97_BRANCH_5_93=(disabled)
macos_brands_FALTWIG97_BRANCH_5_93=(disabled)
netbsd_brands_FALTWIG97_BRANCH_5_93=(disabled)
windows_brands_FALTWIG97_BRANCH_5_93=(win8_slate_internal_wl win8_slate_external_wl)

## -------------------------------------------------------------------------
## TOT DAILY Default Brands (WARNING! DO NOT EDIT this list)
## Pseudo tag: DAILY
## Primary Use: all groups; Owners: hnd-build-list

#- Default list of daily TOT linux brands
linux_brands_DAILY_TOT=( \
  hndrte-dongle-wl \
  linux-internal-wl \
  linux26-internal-router \
  linux-internal-dongle \
)

#- Default list of daily TOT macos brands
macos_brands_DAILY_TOT=( \
  macos-internal-wl-lion \
)

#- Default list of daily TOT netbsd brands
netbsd_brands_DAILY_TOT=(disabled)

#- Default list of daily TOT windows brands
windows_brands_DAILY_TOT=(disabled)

## -------------------------------------------------------------------------
## TOT UTF Default Brands
## Pseudo tag: TOTUTF
## Primary Use: TOT UTF groups; Owners: hnd-sig-list, Tim Auckland

#- Default list of UTF TOT linux brands
linux_brands_TOTUTF=( \
  linux-internal-wl \
  linux26-internal-router \
  linux26-external-router \
  linux-internal-dongle \
)

#- Default list of UTF TOT macos brands
macos_brands_TOTUTF=(disabled)

#- Default list of UTF TOT netbsd brands
netbsd_brands_TOTUTF=(disabled)

#- Default list of UTF TOT windows brands
windows_brands_TOTUTF=( \
  win_internal_wl \
  win_external_wl \
  win_external_dongle_sdio \
)

## -------------------------------------------------------------------------
## TOT NIGHTLY Default Brands
## Pseudo tag: NIGHTLY
## Primary Use: all groups; Owners: hnd-build-list

#- Default list of nightly TOT linux brands
linux_brands_NIGHTLY=( \
  hndrte \
  linux-external-wl-portsrc-hybrid \
  linux-external-wl \
  linux-internal-wl \
  linux-external-dhd \
  linux-internal-dongle \
  linux-internal-media \
  linux-external-media \
  linux26-internal-router \
  linux26-external-vista-router-full-src \
  linux26-external-usbap-full-src \
  linux26-internal-usbap \
  hndrte-dongle-wl \
  linux26-external-router-mini-partial-src \
  linux-external-dongle-pcie\
  linux-internal-dongle-pcie\
  linux-external-dongle-sdio \
  linux-external-dongle-usb \
  linux-internal-dongle-usb \
)

linux_brands_TOT=(${linux_brands_NIGHTLY[@]})

#- Default list of nightly TOT macos brands
macos_brands_NIGHTLY=( \
  macos-internal-wl-ml \
  macos-internal-wl-cab \
)
macos_brands_TOT=(${macos_brands_NIGHTLY[@]})

#- Default list of nightly TOT netbsd brands
netbsd_brands_NIGHTLY=( \
)
netbsd_brands_TOT=(${netbsd_brands_NIGHTLY[@]})

#- Default list of nightly TOT windows brands
windows_brands_NIGHTLY=(disabled)
windows_brands_TOT=(${windows_brands_NIGHTLY[@]})

## -------------------------------------------------------------------------
## XMOGTEST Default Brands
## This branch is used for mogrification unification
## Set XMOGTEST branch to have same default brands as TOT
linux_brands_XMOGTEST=(${linux_brands_NIGHTLY[@]})
macos_brands_XMOGTEST=(${macos_brands_NIGHTLY[@]})
netbsd_brands_XMOGTEST=(${netbsd_brands_NIGHTLY[@]})
windows_brands_XMOGTEST=(${windows_brands_NIGHTLY[@]})

## -------------------------------------------------------------------------
## TOTMERGE_BRANCH_6_1 Default Brands
## Used for DHD merge from Mobility branches to trunk
linux_brands_TOTMERGE=(${linux_brands_NIGHTLY[@]})
macos_brands_TOTMERGE=(${macos_brands_NIGHTLY[@]})
netbsd_brands_TOTMERGE=(${netbsd_brands_NIGHTLY[@]})
windows_brands_TOTMERGE=(${windows_brands_NIGHTLY[@]})

macos_brands_RAPTOR2=(disabled)

## -------------------------------------------------------------------------
## -- WARN -- * -- WARN -- * -- WARN -- * -- WARN -- * -- WARN -- * -- WARN
## --
## -- Do NOT EDIT anything below this line
## --
## -- WARN -- * -- WARN -- * -- WARN -- * -- WARN -- * -- WARN -- * -- WARN
## -------------------------------------------------------------------------
## NOTE: hndrte has a forced check for 32bit resource
## NOTE: On trunk this brand has been ported to work on 64bit
## TODO: On active branches check for 64bit node needs to be removed to
## TODO: remove it from following list

## Ubuntu brands for LSF ubuntu resource queue
LINUX_UBUNTUBRANDS=( \
  ubuntu-external-wpa-supp \
  ubuntu-external-wpa-supp-dev \
  ubuntu-internal-wpa-supp \
)

## Linux brands for 32bit LSF resource queue
LINUX_R32BITBRANDS=( \
  hndrte \
  linux-internal-cxapi \
  linux-external-cxapi \
)

## TODO: This needs to be made default resource for all non $LINUX_32BITBRANDS
## TODO: in build brands
## Linux brands for 64bit LSF resource queue
LINUX_R64BITBRANDS=( \
  linux-internal-wl \
  linux-internal-wl-media \
  linux-external-wl \
  linux-external-wl-portsrc-hybrid \
  linux-external-wl-media-full-src \
  linux-mfgtest-wl \
  linux-mfgtest-wl-full-src \
  linux-external-dongle-sdio \
  linux-external-dhd \
  linux-internal-dongle \
  linux-mfgtest-dongle-sdio \
  linux-external-dongle-usb \
  linux-external-dongle-usb-media \
  linux-external-dongle-pcie-media \
  linux-internal-dongle-usb \
  linux-mfgtest-dongle-usb \
  linux-external-router \
  linux-internal-router \
  linux-mfgtest-router \
  linux-external-router-combo \
  linux-external-router-full-src \
  linux-external-router-partial-src \
  linux-external-wet \
  linux-external-router-sdio-std \
  linux-internal-router-sdio-std \
  linux-external-vista-router-combo \
  linux-external-vista-router \
  linux-external-vista-router-partial-src \
  linux-external-vista-router-full-src \
  linux26-internal-router \
  linux26-external-router \
  linux26-external-router-full-src \
  linux26-external-router-partial-src \
  linux26-external-router-combo \
  linux26-external-router-sdio-std \
  linux26-internal-74k-router \
  linux26-external-74k-router \
  linux26-external-74k-router-full-src \
  linux26-external-74k-router-partial-src \
  linux26-external-74k-router-combo \
  linux26-external-74k-router-sdio-std \
  linux26-mfgtest-router-noramdisk \
  linux-external-router-mini \
  linux-external-router-mini-full-src \
  linux-external-router-mini-partial-src \
  linux-external-usbap \
  linux-internal-usbap \
  linux-external-usbap-full-src \
  hndrte-dongle-wl \
  linux-external-dslcpe-full-src \
  linux-internal-dslcpe \
  linux-olympic-dongle-src \
)

# ===================================================================
# Start of program
# ===================================================================

NULL="/dev/null"

SHORTOPTS="36abcdhlp:qr:suv"
LONGOPTS="32,64,show_32bit_brands,show_64bit_brands,show_ubuntu_brands,show_active,show_brands,show_coverity_active,debug,help,list,platform:,quiet,revision:,verbose"

# Test which getopts is available on current system
# getopt -T returns with exit code 4
if $(getopt -T > $NULL 2>&1); [ $? = 4 ]; then

	# New getopts is supported
	OPTS=$(getopt -o $SHORTOPTS --long $LONGOPTS -n "$0" -- "$@")

else
	# Old legacy getopt
	case $1 in
	--help)
		usage;
		exit 0
		;;
	esac
	OPTS=$(getopt $SHORTOPTS "$@")
fi

while [ $# -gt 0 ]; do
    case $1 in
        -3|--32|--show_32bit_brands)
            SHOW_32BIT_BRANDS=true
            shift
            ;;
        -6|--64|--show_64bit_brands)
            SHOW_64BIT_BRANDS=true
            shift
            ;;
        -a|--show_active)
            SHOW_ACTIVE_BRANCHES=true
            shift
            ;;
        -b|--show_brands)
            SHOW_DEFAULT_BRANDS=true
            shift
            ;;
        -c|--show_coverity_active)
            SHOW_ACTIVE_COVERITY_BRANCHES=true
            shift
            ;;
        -d|--debug)
            export SHOW_DEBUG_INFO=true
            echo "INFO: Enabling debug mode"
            echo "WARN: This can only be used in cmd line mode"
            echo "WARN: Build scripts can't invoke build_config in debug mode"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        -l|--list)
            SHOW_BRANDS_LIST=true
            shift
            ;;
        -p|--platform)
            shift
            MYPLATFORM=$1
            MYPLATFORM=$(echo $MYPLATFORM | tr 'A-Z' 'a-z')
            shift
            ;;
        -q|--quiet)
            QUIET=true
            shift
            ;;
        -r|--revision)
            shift
            MYTAG=$1
            shift
            ;;
        -s|--show_reserved)
            # This flag is reserved
            shift
            ;;
        -u|--show_ubuntu_brands)
            SHOW_UBUNTU_BRANDS=true
            shift
            ;;
        -v|--verbose)
            SHOW_VERBOSE_INFO=true
            shift
            ;;
        *)
            echo "Getopt Error: option processing error: $1" 1>&2
            exit 1
            ;;
    esac
done

# If tag is set and platform isn't set, show default brands
[ -n "$MYTAG" -a ! -n "$MYPLATFORM" ] && SHOW_DEFAULT_BRANDS=true

# Default tag is TOT or NIGHTLY
MYTAG=${MYTAG:-NIGHTLY}

# Synonyms for trunk, translate to NIGHTLY
case "${MYTAG}" in
	tot|TOT|trunk|TRUNK)
		MYTAG="NIGHTLY"
		;;
esac

dbg_msg "MYTAG=$MYTAG"

# Bare DAILY tag means DAILY_TOT
if [ "$MYTAG" == "DAILY" ]; then
   MYTAG=DAILY_TOT
fi

dbg_msg "MYTAG=$MYTAG"
dbg_msg "MYPLATFORM=$MYPLATFORM"

## ---------------------------------------------------------------------------
## Now given a tag/branch/twig name, compute list of valid brands or branches
## ---------------------------------------------------------------------------

if [ -n "${SHOW_ACTIVE_BRANCHES}" ]; then

   # Show active branches and exit
   echo ${active_branches_all[@]}

elif [ -n "${SHOW_ACTIVE_COVERITY_BRANCHES}" ]; then

   # Show active coverity branches and exit
   echo ${active_coverity_branches_all[@]}

elif [ -n "${SHOW_32BIT_BRANDS}" ]; then

   # Show 32bit brands
   echo ${LINUX_R32BITBRANDS[@]}

elif [ -n "${SHOW_64BIT_BRANDS}" ]; then

   # Show 64bit brands
   echo ${LINUX_R64BITBRANDS[@]}

elif [ -n "${SHOW_UBUNTU_BRANDS}" ]; then

   # Show ubuntu brands
   echo ${LINUX_UBUNTUBRANDS[@]}

elif [ "${MYTAG}" != "" ]; then

   # Drive default set of build brands from specified TAG
   TAGPFX=$(echo ${MYTAG} | awk -F_ '{print $1}')

   # Initialize branch and twig names
   BRANCHNAME=${MYTAG}
   TWIGNAME=${MYTAG}

   # Strip out branchname/twigname and _REL_ prefixes
   RELNUMC=$(echo ${MYTAG} |  perl -ne 's/[a-zA-Z0-9]+_REL_//g; s/_/ /g; print $_' | wc -w | xargs printf "%d")

   # If TAGNAME to be queried is a _REL_ tag, then set RELNAME
   if echo $MYTAG | egrep -q "_REL_"; then
      RELNAME=$(echo ${MYTAG} |  perl -ne 's/_[0-9]+$//g; print $_')
      dbg_msg "Derived Release Prefix = ${RELNAME}"
   fi

   # From XXX_REL_x_y_n builds derive XXX_BRANCH_x_y BRANCH name
   # E.g: for PBR_REL_5_10_38 tag, branch is set to PBR_BRANCH_5_10
   if [ ${RELNUMC} -eq 3 ]; then
      IFS="_	 " tag=(${MYTAG})
      unset IFS
      branchpfx=${tag[0]};
      major=${tag[2]}; minor=${tag[3]}
      BRANCHNAME=${branchpfx}_BRANCH_${major}_${minor}
      unset major minor
      dbg_msg "Derived Branch Name = ${BRANCHNAME} from RELNUMC=$RELNUMC"
   fi

   # From XXX_REL_x_y_z_n builds derive XXX_TWIG_x_y_z TWIG name
   # E.g: for PBR_REL_5_10_38_17 tag, twig is set to PBR_TWIG_5_10_38
   if [ ${RELNUMC} -gt 3 ]; then
      IFS="_	 " tag=(${MYTAG})
      unset IFS
      twigpfx=${tag[0]};
      major=${tag[2]}; minor=${tag[3]}; bldnum=${tag[4]}
      TWIGNAME=${twigpfx}_TWIG_${major}_${minor}_${bldnum}
      unset major minor bldnum
      dbg_msg "Derived Twig Name = ${TWIGNAME} from RELNUMC=$RELNUMC"
   fi

   # If default list of brands exist for RELNAME, use them
   # If not use TWIGNAME default list of brands, if any
   # If not use BRANCHNAME default list of brands, if any
   # If not use TAGPFX default list of brands, if any
   # If not use DEFAULT list of brands

   case "${MYPLATFORM}" in
        Linux|linux) eval BRANDS=('${linux_brands_'$RELNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${linux_brands_'$TWIGNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${linux_brands_'$BRANCHNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${linux_brands_'$TAGPFX[@]})
                     [[ -z "${BRANDS[*]}" && "$MYTAG" != *_BRANCH_7_* && "$MYTAG" != *_TWIG_7_* ]] && \
                          BRANDS=(${linux_brands_DEFAULT_[@]})
                     ;;
        Darwin|macos) eval BRANDS=('${macos_brands_'$RELNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${macos_brands_'$TWIGNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${macos_brands_'$BRANCHNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${macos_brands_'$TAGPFX[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          BRANDS=(${macos_brands_DEFAULT_[@]})
                     ;;
        NetBSD|netbsd) eval BRANDS=('${netbsd_brands_'$RELNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${netbsd_brands_'$TWIGNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${netbsd_brands_'$BRANCHNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${netbsd_brands_'$TAGPFX[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          BRANDS=(${netbsd_brands_DEFAULT_[@]})
                     ;;
        CYGWIN*|window*) eval BRANDS=('${windows_brands_'$RELNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${windows_brands_'$TWIGNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${windows_brands_'$BRANCHNAME[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          eval BRANDS=('${windows_brands_'$TAGPFX[@]})
                     [ -z "${BRANDS[*]}" ] && \
                          BRANDS=(${windows_brands_DEFAULT_[@]})
                     ;;
                  *)
                     BRANDS=();;
   esac #

   if [ "$BRANDS" == "disabled" ]; then BRANDS=(); exit 1; fi

   # Show build brand list that is derived above
   if [ -n "${SHOW_BRANDS_LIST}" ]; then
      echo ${BRANDS[@]} | fmt -1 | sort
   else
      echo ${BRANDS[@]}
   fi

   # Show default brands for each platform
   if [ -n "${SHOW_DEFAULT_BRANDS}" ]; then
      # Derive default build brands for given TAG and platform
      LINUX_BRANDS=($(bash $0 -r ${MYTAG} -p linux))
      MACOS_BRANDS=($(bash $0 -r ${MYTAG} -p macos))
      NETBSD_BRANDS=($(bash $0 -r ${MYTAG} -p netbsd))
      WINDOWS_BRANDS=($(bash $0 -r ${MYTAG} -p windows))

      # In quite mode, show non-verbose output
      if [ -n "${QUIET}" ]; then
         echo ${LINUX_BRANDS[@]} \
	      ${MACOS_BRANDS[@]} \
	      ${NETBSD_BRANDS[@]} \
	      ${WINDOWS_BRANDS[@]}
      else
         # Otherwise show platform-wise listing
         [ -n "${LINUX_BRANDS[*]}" ] && \
           echo -e "\n${MYTAG} Default Linux Brands[${#LINUX_BRANDS[@]}]:" &&
           echo "${LINUX_BRANDS[@]}" | fmt -1 |sort -u | sed -e "s/^/\t/"
         [ -n "${MACOS_BRANDS[*]}" ] && \
           echo -e "\n${MYTAG} Default Macos Brands[${#MACOS_BRANDS[@]}]:" &&
           echo "${MACOS_BRANDS[@]}" | fmt -1 |sort -u | sed -e "s/^/\t/"
         [ -n "${NETBSD_BRANDS[*]}" ] && \
           echo -e "\n${MYTAG} Default NetBSD Brands[${#NETBSD_BRANDS[@]}]:" &&
           echo "${NETBSD_BRANDS[@]}" | fmt -1 |sort -u | sed -e "s/^/\t/"
         [ -n "${WINDOWS_BRANDS[*]}" ] && \
           echo -e "\n${MYTAG} Default Windows Brands[${#WINDOWS_BRANDS[@]}]:" &&
           echo "${WINDOWS_BRANDS[@]}" | fmt -1 |sort -u | sed -e "s/^/\t/"
      fi
   fi
fi
