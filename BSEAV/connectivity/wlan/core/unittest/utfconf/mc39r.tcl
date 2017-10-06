# -*tcl-*-
#
#  $Copyright Broadcom Corporation$
#  $Id$ 2014/08/22 18:00:00 sotmishi Exp $
#
#  Testbed configuration file for MC39r Testbed Sheida Otmishi
#

source utfconf/mc39.tcl
set ::UTF::SummaryDir "/projects/hnd_sig_ext12/sotmishi/mc39r"

UTF::Linux mc39end2 -sta {wan eth1} -lan_ip 10.19.86.72

package require UTF::Aeroflex
UTF::Aeroflex afr -lan_ip 172.16.1.210 \
    -relay mc39end1 \
    -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
    G3 attn default
    ALL attn default

#    package require UTF::Test::RebootTestbed
#    UTF::Try "Reboot Testbed" {
#	UTF::Test::RebootTestbed -hostlist "4360j 4360i"
#    }
    set ::UTF::trailer_info ""

    foreach s {5358 5358/43526} {
	catch {$s restart [$s wlname]_radio=0}
	catch {$s deinit}
    }


    foreach s {4360i 4360j} {
	$s deinit
    }
    foreach s {4360i 4360j lan wan} {
        catch {$s iptables -F}
        catch {$s tcptune 2M}
        $s deinit
    }
	return
}

UTF::Linux mc39tst1 -sta {4360i enp11s0} \
    -power  "power4 1" \
    -power_button "auto" \
    -console "mc39end1:40451" \
    -wlinitcmds {wl msglevel +assoc;  wl bw_cap 2g -1; wl obss_coex 0} \
    -yart {-attn5g 30-63 -attn2g 30-63 -frameburst 1} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g

4360i clone 4360i-b -tag  "EAGLE_BRANCH_10_10"
4360i-b configure -attngrp G3

UTF::Linux mc39tst2 -sta {4360j enp11s0} \
    -power  "power4 2" \
    -power_button "auto" \
    -console "mc39end1:40452" \
    -wlinitcmds {wl msglevel +assoc;  wl bw_cap 2g -1; wl obss_coex 0} \
    -yart {-attn5g 30-63 -attn2g 30-63 -frameburst 1} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g

4360j clone 4360j-b -tag  "EAGLE_BRANCH_10_10"
4360j-b configure -attngrp G3

UTF::Router _5358 -name 5358 \
    -lan_ip 192.168.1.1 \
    -sta {5358 eth1 5358.%15 wl0.%} \
    -brand linux26-internal-usbap \
    -power "power6 1" \
    -tag trunk \
    -relay "mc39end1" \
    -console "mc39end1:40001" \
    -lanpeer lan \
    -wanpeer wan \
    -nofragmentation 1 \
    -bootwait 30 \
    -rteconsole  "mc39end1:40002" \
    -embeddedimage 43526b \
    -nvram {
        fw_disable=1
        wl_msglevel=0x101
        wl0_ssid=5358
	watchdog=6000
	console_loglevel=7
	assert_type=2
	wl0_obss_coex=0
        wl0_channel=1
	wl0_radio=0
        antswitch=0
	samba_mode=2
        wl1_ssid=43526
        wl1_radio=0
        wl1_channel=36
	wl1_obss_coex=0
    } \
    -yart {-attn5g 30-63 -attn2g 30-63 -frameburst 1} \
    -datarate {-i 0.5 -frameburst 1} -nosamba 1 -perfchans 3l -udp 100m

5358 configure -attngrp G3

5358 clone 5358/43526 -sta {5358/43526 eth2 5358/43526.%3 wl1.%} \
    -nofragmentation 1 \
    -perfchans {36/80} -channelsweep {-band a} \
    -noradio_pwrsave 1 -nosamba 1 -nokpps 1

5358/43526 configure -dualband {5358 -c1 36/80}

# placeholder for another TOB. There is no usbap on EAGLE
5358 clone 5358e -sta {5358e eth1 5358e.%15 wl0.%} \
    -brand linux26-internal-usbap \
    -tag "EAGLE_BRANCH_10_10"

5358 clone 5358e/43526 -sta {5358e/43526 eth2 5358e/43526.%3 wl1.%} \
    -perfchans {36 36l 36/80} -channelsweep {-band a} \
    -brand linux26-internal-usbap \
    -tag "EAGLE_BRANCH_10_10" -noradio_pwrsave 1 \
    -nosamba 1

5358e/43526 configure -dualband {5358B -c1 36/80}

5358 clone 5358ex -sta {5358ex eth1 5358ex.%15 wl0.%} \
    -brand linux26-external-usbap-full-src \
    -tag "EAGLE_BRANCH_10_10" -perfonly 1

5358 clone 5358ex/43526 -sta {5358ex/43526 eth2 5358ex/43526.%3 wl1.%} \
    -perfchans {36 36l 36/80} -channelsweep {-band a} \
    -brand linux26-external-usbap-full-src \
    -tag "EAGLE_BRANCH_10_10" -perfonly 1 -noradio_pwrsave 1

5358 clone 5358x -sta {5358x eth1 5358x.%15 wl0.%} \
    -brand linux26-external-usbap-full-src \
    -tag "trunk" -perfonly 1

5358 clone 5358x/43526 -sta {5358x/43526 eth2 5358x/43526.%3 wl1.%} \
    -perfchans {36 36l 36/80} -channelsweep {-band a} \
    -brand linux26-external-usbap-full-src \
    -tag "trunk" -perfonly 1 -noradio_pwrsave 1

5358 clone 5358AT -sta {5358AT eth1 5358AT.%15 wl0.%} \
    -brand linux26-internal-usbap \
    -tag "AARDVARK01T_TWIG_6_37_14"

5358 clone 5358AT/43526 -sta {5358AT/43526 eth2 5358AT/43526.%3 wl1.%} \
    -perfchans {36 36l 36/80} -channelsweep {-band a} \
    -brand linux26-internal-usbap \
    -tag "AARDVARK01T_TWIG_6_37_14" -noradio_pwrsave 1

5358 clone 5358ATx -sta {5358ATx eth1 5358ATx.%15 wl0.%} \
    -brand linux26-external-usbap-full-src \
    -tag "AARDVARK01T_TWIG_6_37_14" -perfonly 1

5358 clone 5358ATx/43526 -sta {5358ATx/43526 eth2 5358ATx/43526.%3 wl1.%} \
    -perfchans {36 36l 36/80} -channelsweep {-band a} \
    -brand linux26-external-usbap-full-src \
    -tag "AARDVARK01T_TWIG_6_37_14" -perfonly 1 -noradio_pwrsave 1

5358 clone 5358b -sta {5358b eth1 5358b.%15 wl0.%} \
    -brand linux26-internal-usbap \
    -tag "BISON04T_BRANCH_7_14"

5358 clone 5358b/43526 -sta {5358b/43526 eth2 5358b/43526.%3 wl1.%} \
    -perfchans {36 36l 36/80} -channelsweep {-band a} \
    -brand linux26-internal-usbap \
    -tag "BISON04T_BRANCH_7_14" -perfonly 1 \
    -noradio_pwrsave 1 -nosamba 1 -nokpps 1

5358 clone 5358bx -sta {5358bx eth1 5358bx.%15 wl0.%} \
    -brand linux26-external-usbap-full-src \
    -tag "BISON04T_BRANCH_7_14" -perfonly 1

5358 clone 5358bx/43526 -sta {5358bx/43526 eth2 5358bx/43526.%3 wl1.%} \
    -perfchans {36 36l 36/80} -channelsweep {-band a} \
    -brand linux26-external-usbap-full-src \
    -tag "BISON04T_BRANCH_7_14" -perfonly 1 -noradio_pwrsave 1


set UTF::RouterNightlyCustom {

    if {!$(perfonly)} {
	catch {
	    package require UTF::Test::MiniUplinks
	    UTF::Test::MiniUplinks $Router $STA1 -sta $STA2
	}
    }
}


####
# tima: experimental remapping wan to be a second lan
####

wan clone lan2
lan2 configure -ipaddr 192.168.1.45

# setup
# lan2 iptables -t nat -F
# lan2 service dhcpd stop

# nvram changes are to move wan port into the lan block.  Or you could
# just move the cable.

5358bx clone 5358bx2 \
    -lanpeer {lan lan2} \
    -wanpeer {} \
    -nvram [concat {
	"vlan1ports=0 1 2 3 5*"
	"vlan2ports=4 5"
    } [5358bx cget -nvram]]

5358bx/43526 clone 5358bx/435262 \
    -lanpeer {lan lan2} \
    -wanpeer {} \
    -nvram [concat {
	"vlan1ports=0 1 2 3 5*"
	"vlan2ports=4 5"
    } [5358bx/43526 cget -nvram]]

#####


UTF::Q mc39r mc39end1
