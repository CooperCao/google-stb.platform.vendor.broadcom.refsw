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
set ::UTF::SummaryDir "/projects/hnd_sig_ext16/rmcmahon/demo"

package require UTF::Power
package require UTF::AeroflexDirect
package require UTF::WebRelay

# Packages commonly used in interactive mode
if {[info exists ::tcl_interactive] && $::tcl_interactive} {
    package require UTF::Test::ConnectAPSTA
    package require UTF::Test::APChanspec
    package require UTF::Streams
}

UTF::Linux mc80-utf -lan_ip 10.19.87.1

UTF::Power::PicoPSU pwr_ap -lan_ip 172.16.2.190
UTF::Power::PicoPSU pwr_br1 -lan_ip 172.16.2.191
UTF::Power::PicoPSU pwr_br2 -lan_ip 172.16.2.193
UTF::Power::PicoPSU pwr_4708 -lan_ip 172.16.2.195


UTF::Router _4708/4708 -name AP-4708 \
    -sta {4708ap eth1} \
    -brand "linux-2.6.36-arm-internal-router" \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_4708} \
    -relay "mc80-lx4" \
    -lanpeer vlan177_if \
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
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
    }
4708ap configure -attngrp L1


UTF::Router _4706br1 -name Br1 \
    -sta {PSTA1 eth1} \
    -brand linux26-internal-router \
    -tag "BISON_BRANCH_7_10" \
    -power {pwr_br1} \
    -relay "mc80-lx2" \
    -lanpeer vlan171_if \
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
	boardtype=0x05b2; # 4706nr
	pktc_enable=0
    }

UTF::Router _4706br2 -name Br2-MCH \
    -sta {PSTA2 eth1} \
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


UTF::AeroflexDirect AF-A -lan_ip 172.16.2.114 -retries 0 -concurrent 0 -silent 0 -group {L1 {1 2 3} L2 {4 5 6}}
UTF::AeroflexDirect AF-B -lan_ip 172.16.2.115 -retries 0 -concurrent 0 -silent 0 -group {L3 {1 2 3} L4 {4 5 6} }

PSTA1 configure -attngrp L3
PSTA2 configure -attngrp L4

UTF::Linux mc80-lx2 -lan_ip 10.19.87.3 \
                    -sta {vlan171_if p21p1.171}
UTF::Linux mc80-lx3 -lan_ip 10.19.87.4 \
                    -sta {vlan173_if p21p1.173}
UTF::Linux mc80-lx4 -lan_ip 10.19.87.5 \
                    -sta {vlan177_if em1.177}

vlan171_if configure -ipaddr 192.168.1.171
vlan173_if configure -ipaddr 192.168.1.173
vlan177_if configure -ipaddr 192.168.1.177
