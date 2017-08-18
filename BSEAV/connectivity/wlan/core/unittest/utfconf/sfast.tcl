#
# UTF configuration for MC80 (4360) testing
# Robert J. McMahon (setup next to my cube)
# March 2012
#
# IP Address range 10.19.87.1-10
#
# controlbyweb relay 2 172.16.2.181
# consoles: 172.176.2.174, 175
# SummaryDir sets the location for test results in nightly testing.
# set ::UTF::SummaryDir "/projects/hnd_sig_ext4/rmcmahon/mc80"
# set ::UTF::SummaryDir "/projects/hnd_sig_ext10/rmcmahon/mc80"
set ::UTF::SummaryDir "/projects/hnd_sig_ext16/rmcmahon/sfast"

package require UTF::Power
package require UTF::AeroflexDirect
package require UTF::WebRelay

# Packages commonly used in interactive mode
if {[info exists ::tcl_interactive] && $::tcl_interactive} {
    package require UTF::Test::ConnectAPSTA
    package require UTF::Test::APChanspec
    package require UTF::Streams
    package require UTF::FTTR
    package require UTF::Test::ConfigBridge
}

UTF::AeroflexDirect AF-A -lan_ip 172.16.2.114 -retries 0 -concurrent 0 -silent 0 -group {L1 {1 2 3} L2 {4 5 6}}
UTF::AeroflexDirect AF-B -lan_ip 172.16.2.116 -retries 0 -concurrent 0 -silent 0 -group {L3 {1 2 3} L4 {4 5 6}}

UTF::Power::PicoPSU pwr_ap -lan_ip 172.16.2.190
UTF::Power::PicoPSU pwr_br1 -lan_ip 172.16.2.191
UTF::Power::PicoPSU pwr_br2 -lan_ip 172.16.2.193

UTF::Linux sfast-utf -lan_ip 10.19.85.171

UTF::Linux sfast-lx1 -lan_ip 10.19.87.176 \
                    -sta {vlan202 em2.202}
UTF::Linux sfast-lx2 -lan_ip 10.19.85.169 \
                    -sta {vlan302 em2.302}

vlan202 configure -ipaddr 192.168.1.22
vlan302 configure -ipaddr 192.168.1.32


UTF::Router _4708/4708 -name AP-4708 \
    -sta {4708ap eth1 4331ap2 eth2} \
    -brand "linux-2.6.36-arm-internal-router" \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_4708} \
    -relay "sfast-lx2" \
    -lanpeer vlan302 \
    -lan_ip 192.168.1.5 \
    -console 10.19.87.5:40000 \
    -trx linux\
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.5
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4708/4360
	wl1_ssid=4708/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	wl_reg_mode=h
	wl_wmf_bss_enable=1
	wl_wet_tunnel=1
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_dcs_csa_unicast=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
	wl_pspretend_retry_limit=5
    }
4708ap configure -attngrp L1


UTF::Router _4708br1 -name Br1 \
    -sta {4331br1 eth1 4360br1 eth2} \
    -brand "linux-2.6.36-arm-internal-router" \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_br1} \
    -relay "sfast-lx1" \
    -lanpeer vlan202 \
    -lan_ip 192.168.1.3 \
    -console 10.19.87.3:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.3
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4706/4360
	wl1_ssid=4706/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	pktc_enable=0
    }

UTF::Router _4706br2 -name Br2-MCH \
    -sta {4360br2 eth1 4331br2 eth2} \
    -brand linux26-internal-router \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_br2} \
    -relay "mc80-lx3" \
    -lanpeer vlan173_if \
    -lan_ip 192.168.1.4 \
    -console 10.19.87.4:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.4
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4706/4360
	wl1_ssid=4706/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	boardtype=0x0617 # 4706nr2hmc
	# pktc_enable=0
    }
UTF::Linux mc80-lx3 -lan_ip 10.19.87.4 \
                    -sta {vlan173_if p21p1.173 vlan_a em1.2000 wired_a p21p1.1000}
vlan173_if configure -ipaddr 192.168.1.173

#Create the clones
set TWIGNIGHTLY AARDVARK01T_TWIG_6_37_14
set DUTS "4708ap 4360br1 4360br2 4360br2"
foreach DUT $DUTS {
    if {[regexp {\-arm\-} [$DUT cget -brand]]} {
	$DUT clone ${DUT}_twign -tag $TWIGNIGHTLY -brand linux-2.6.36-arm-up-internal-router
	$DUT clone ${DUT}_twignsmp -tag $TWIGNIGHTLY -brand linux-2.6.36-arm-internal-router
	$DUT clone ${DUT}ext -brand linux-2.6.36-arm-external-vista-router-full-src
	$DUT clone ${DUT}_taf -image /home/rmcmahon/Code/brcm_drivers/linux_ARM.trx -date {}
    } else {
	$DUT clone ${DUT}ext -brand linux26-external-vista-router-full-src
	$DUT clone ${DUT}_twign -tag $TWIGNIGHTLY
	$DUT clone ${DUT}_taf -image /home/rmcmahon/Code/brcm_drivers/linux_mips.trx -date {}
    }
}

