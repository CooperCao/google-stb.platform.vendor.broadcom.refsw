
# Bunnik UTF Test Configuration
#
# UTF configuration for BUNOS testbed @ Broadcom Bunnik
#

# load packages
package require UTF::Aeroflex
package require UTF::Power
package require UTF::PandaOS
package require UTF::LinuxOS

UTF::Logfile "bunos.log"
#set ::UTF::SummaryDir "/projects/hnd_software_nl/sig/$::env(LOGNAME)/bunos"
set ::UTF::SummaryDir "/home/$::env(LOGNAME)/UTF/bunos"
#set ::UTF::SummaryDir "/projects/hnd_software_ext3/$::env(LOGNAME)/UTF/bunos"

# Environment variables affect driver selection
if {[info exists ::env(PARENT_JOB)]} {
  set ::driver_tag "$::env(PARENT_JOB)"
} else {
  set ::driver_tag "nightly"
}
if {[info exists ::env(GIT_ORIGIN)]} {
  set ::driver_brand "brcm80211-$::env(GIT_ORIGIN)"
  if { $::env(GIT_ORIGIN) eq "external" } {
    set ::driver_type ""
  } else {
    set ::driver_type "-debug"
  }
} else {
  set ::driver_brand "brcm80211-internal"
  set ::driver_type "-debug"
}

# this is needed for multiple STA to multiple AP tests
#set ::sta_list "43143sdio 43242fmac 4334b1 43143fmac"
#set ::ap_list "4717_1 4717_3"

UTF::Power::Synaccess ap_pwr \
    -lan_ip 192.168.0.50 \
    -relay privat_lan \
    -rev 1

UTF::Power::Synaccess bun25_pwr \
    -lan_ip 192.168.0.51 \
    -relay privat_lan \
    -rev 1

UTF::Power::Synaccess bun09_77_pwr \
    -lan_ip 192.168.0.52 \
    -relay privat_lan \
    -rev 1

UTF::Power::Synaccess bun62_pwr \
    -lan_ip 192.168.0.53 \
    -relay privat_lan \
    -rev 1

UTF::Power::Synaccess bun78_pwr \
    -lan_ip 192.168.0.54 \
    -relay privat_lan \
    -rev 1

UTF::Power::Blackbox bun10_21_pwr \
    -lan_ip 192.168.0.55 \
    -relay privat_lan


# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 10.176.8.89 \
        -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    #
    # Do the attenuator last, sometimes it get hosed with
    # utf error couldn't open socket.
    #
    ALL attn 100;
    G1 attn 10;
    G2 attn 10;
    G3 attn 10;

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    # AP1 restart wl0_radio=0

    # Disable USB devices
#    43242bmac power_sta cycle
#    43143bmac power_sta cycle

    # delete myself
    unset ::UTF::SetupTestBed

    return
}

# Set testrig specific attenuation ranges for use with RvRNightly1.test
set ::cycle5G40AttnRange "5-85 85-5"
set ::cycle5G20AttnRange "5-85 85-5"
set ::cycle2G40AttnRange "5-85 85-5"
set ::cycle2G20AttnRange "5-85 85-5"

##### Relay / Consolelogger
##### Running FC15
UTF::Linux lb-bun-88 \
    -sta {privat_lan p6p1} \
    -lan_ip 10.176.8.88 \
    -user root \
    -ssh ssh
privat_lan configure -ipaddr 192.168.0.99

##### DUT Host Machine
##### BRCMSMAC
UTF::LinuxOS lb-bun-09 \
    -name "lb-bun-09" \
    -sta {lb-bun-09_sta wlan0} \
    -power {bun09_77_pwr 2} \
    -lan_ip "10.176.8.19" \
    -ssh ssh \
    -user root \
    -tag $::driver_tag \
    -brand $::driver_brand \
    -type [subst {brcmsmac$::driver_type}] \
    -bus "pci" \
    -console "/var/log/syslog" \
    -yart "-force -roam_off 1"

lb-bun-09_sta clone 4313_09 -sta {4313_09 wlan1}
lb-bun-09_sta clone 43224_09 -sta {43224_09 wlan2}
43224_09 configure -attngrp G1
4313_09 configure -attngrp G1

##### DUT Host Machine
##### BRCMSMAC & BRCMFMAC
UTF::LinuxOS lb-bun-10 \
    -sta {lb-bun-10_sta wlan0} \
    -power {bun10_21_pwr 1} \
    -lan_ip "10.176.8.20" \
    -ssh ssh \
    -user root \
    -tag $::driver_tag \
    -brand $::driver_brand \
    -type [subst {brcmsmac$::driver_type}] \
    -bus "pci" \
    -console "/var/log/syslog" \
    -yart "-force -roam_off 1"

lb-bun-10_sta clone 43224_10 -sta {43224_10 wlan0}
lb-bun-10_sta clone 43236_10 -sta {43236_10 wlan1} \
  -type [subst {brcmfmac$::driver_type}] -bus "usb"
43224_10 configure -attngrp G1
43236_10 configure -attngrp G1

##### DUT Host Machine
##### BRCMSMAC & BRCMFMAC
UTF::LinuxOS lb-bun-21 \
    -sta {lb-bun-21_sta wlan0} \
    -power {bun10_21_pwr 2} \
    -lan_ip "10.176.8.21" \
    -ssh ssh \
    -user root \
    -tag $::driver_tag \
    -brand $::driver_brand \
    -type [subst {brcmsmac$::driver_type}] \
    -bus "pci" \
    -console "/var/log/syslog" \
    -yart "-force -roam_off 1"

lb-bun-21_sta clone 43224_21 -sta {43224_21 wlan0}
# HACK:
#	for bcm4329 use bus "sdhci" to assure device
#	detection works (see LinuxOS constructor).
lb-bun-21_sta clone 4329_21 -sta {4329_21 wlan1} \
  -type [subst {brcmfmac$::driver_type}] -bus "sdhci"
43224_21 configure -attngrp G1
4329_21 configure -attngrp G1

##### DUT Host Machine
##### BRCMSMAC & BRCMFMAC
UTF::LinuxOS lb-bun-25 \
    -sta {lb-bun-25_sta wlan0} \
    -power {bun25_pwr 1} \
    -lan_ip "10.176.8.27" \
    -ssh ssh \
    -user root \
    -tag $::driver_tag \
    -brand $::driver_brand \
    -type [subst {brcmsmac$::driver_type}] \
    -bus "pci" \
    -console "/var/log/syslog" \
    -yart "-force -roam_off 1"

lb-bun-25_sta clone 43143_25 -sta {43143_25 wlan6} \
  -type [subst {brcmfmac$::driver_type}] -bus "usb"
lb-bun-25_sta clone 4313_25 -sta {4313_25 wlan2}
# HACK:
#       for bcm4329 use bus "sdhci" to assure device
#       detection works (see LinuxOS constructor).
lb-bun-25_sta clone 4329_25 -sta {4329_25 wlan1} \
  -type [subst {brcmfmac$::driver_type}] -bus "sdhci"
43143_25 configure -attngrp G3
4313_25 configure -attngrp G3
4329_25 configure -attngrp G3

##### DUT Host Machine
##### BRCMFMAC
UTF::LinuxOS lb-bun-205 \
    -sta {4335_205 wlan0} \
    -lan_ip "10.176.8.205" \
    -power {bun25_pwr 2} \
    -ssh ssh \
    -user root \
    -tag $::driver_tag \
    -brand $::driver_brand \
    -type [subst {brcmfmac$::driver_type}] \
    -bus "sdio" \
    -console "/var/log/messages" \
    -yart "-force -roam_off 1"

##### DUT Host Machine
##### BRCMFMAC
# UTF::PandaOS lb-bun-62 \
    # -sta {4330_62 wlan0} \
    # -lan_ip "10.176.8.62" \
    # -power {bun62_pwr 1} \
    # -ssh ssh \
    # -user root \
    # -tag "nightly" \
    # -brand "brcm80211-internal" \
    # -type "brcmfmac-debug" \
    # -bus "mmc" \
    # -console 192.168.0.53:2001 \
    # -serialrelay lb-bun-88
#
# 4330_62 configure -attngrp G2

##### DUT Host Machine
##### BRCMFMAC
# UTF::PandaOS lb-bun-77 \
    # -sta {4324_77 wlan0} \
    # -lan_ip "10.176.8.77" \
    # -power {bun09_77_pwr 1} \
    # -ssh ssh \
    # -user root \
    # -tag "nightly" \
    # -brand "brcm80211-internal" \
    # -type "brcmfmac-debug" \
    # -bus "mmc" \
    # -console 192.168.0.52:2001 \
    # -serialrelay lb-bun-88
#
# 4324_77 configure -attngrp G1

##### DUT Host Machine
##### BRCMFMAC
# UTF::PandaOS lb-bun-78 \
    # -sta {4334_78 wlan0} \
    # -lan_ip "10.176.8.78" \
    # -power {bun78_pwr 1} \
    # -ssh ssh \
    # -user root \
    # -tag "nightly" \
    # -brand "brcm80211-internal" \
    # -type "brcmfmac-debug" \
    # -bus "mmc" \
    # -console 192.168.0.54:2001 \
    # -serialrelay lb-bun-88
#
# 4334_78 configure -attngrp G2

##### Broadcom Router
# AP Section
# Netgear R6300
# 4706 AP with 4360 and 4331 cards
UTF::Router 4706 -sta {
    NGR6300/4360 eth2
    NGR6300/4331 eth1
} \
    -lan_ip 192.168.0.1 \
    -relay {lb-bun-88} \
    -lanpeer {privat_lan} \
    -brand linux26-internal-router \
    -console "10.176.8.88:40000" \
    -power {ap_pwr 1} \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -nvram {
	# watchdog=3000 (default)
	lan_ipaddr=192.168.0.1
	dhcp_start=192.168.0.100
	dhcp_end=192.168.0.150
	lan1_ipaddr=192.168.2.1
	wl_msglevel=0x101
	wl0_ssid=smoke24
	wl0_channel=3
	wl0_bw_cap=-1
	wl0_radio=0
	wl0_obss_coex=0
	wl1_ssid=smoke5
	wl1_channel=36
	wl1_bw_cap=-1
	wl1_radio=0
	wl1_obss_coex=0
	#Only 1 AP can serve DHCP Addresses
	#router_disable=1
	et_msglevel=0; # WAR for PR#107305
    } \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -noradio_pwrsave 1

