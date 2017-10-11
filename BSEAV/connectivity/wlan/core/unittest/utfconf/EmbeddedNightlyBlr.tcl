# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$Id
#

#######
#
# UTF Config file for STADev UTF-based smoke testing coffin.
#
#######

# Power button controller
UTF::Power::Synaccess 10.132.22.90 -user hwlab -passwd hnd123

# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/BCM4329/work/abhay/UTF/src/tools/unittest/ResultUTF"

# LAN Endpoint and Relay
UTF::Linux 10.132.22.250 -sta {lan eth1}

#Path
UTF::DHD 10.132.22.237 -sta {43239 eth1} \
    -power {10.132.22.250 1} \
    -console 10.132.22.237:40002\
    -image "/projects/BCM4329/work/abhay/UTF/src/tools/unittest/drivers/rtecdc.bin"\
    -type 43239ao-rom/sdio-ag-pool-mfgtest.bin \
    -nvram nvram.txt \
    -hostconsole 10.132.22.250:40000 


set [10.132.22.237 info vars archive]  /root/projects_hnd_archives_unix_UTF
set [10.132.22.250 info vars archive]  /root/projects_hnd_archives_unix_UTF


# Router
UTF::Router 4705 \
    -sta {4705/4322 eth1} \
    -brand linux-external-router \
    -relay 10.132.22.250 \
    -lanpeer lan \
    -console 10.132.22.250:40001\
    -power {10.132.22.90 2}\
    -nvram {
      wl_msglevel=0x101
      router_disable=0
      fw_disable=0
      wl0_ssid=STADevSmokeTest
      wl0_channel=5u
      wl0_frameburst=on
      wl1_radio=0
      wl0_obss_coex=0
     }

