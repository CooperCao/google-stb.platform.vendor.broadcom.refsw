# -*-tcl-*-
#
#  $Copyright Broadcom Corporation$
#  $Id$
#
#  Testbed configuration file for MD07r Testbed
#  Router Testbed

source "utfconf/md07.tcl"

set ::UTF::SummaryDir "/projects/hnd_sig_ext12/sotmishi/md07r"
set UTF::SetupTestBed {
    G1 attn default
    return
}

G1 configure -default 0

#####################
# Power
#####################

# STA pwrswt
UTF::Power::Synaccess NPC15 -lan_ip 172.16.1.15 -relay md07end3 -rev 1
# Router pwrswt
UTF::Power::Synaccess NPC19 -lan_ip 172.16.1.19 -relay md07end3

#####################
# Ethernet Endpoints
#####################

# lan in md07 shared

UTF::Linux md07end4 -sta {wan p1p1}
wan configure -ipaddr 192.168.254.244

UTF::Linux md07end1 -sta {lan1 p10p1}
lan1 configure -ipaddr 192.168.1.98

UTF::Linux md07end2 -sta {lan2 p10p1}
lan2 configure -ipaddr 192.168.1.97

UTF::Linux md07tst5 -sta {lan3 eth1}
lan3 configure -ipaddr 192.168.1.96


#####################
# STAs
#####################
set stacmn {
    -tag EAGLE_BRANCH_10_10
    -tcpwindow 4M
    -slowassoc 5 -reloadoncrash 1
    -datarate {-i 0.5} -udp 1.8g
    -wlinitcmds {
	wl msglevel +assoc;
	wl down;
	wl country '#a/0';
	wl bw_cap 2g -1;
	wl vht_features 7
    }
}

UTF::Linux md07tst1 -sta {sta1 enp1s0} \
    -console {md07end3:40001} -power {NPC15 1} \
    {*}$stacmn

UTF::Linux md07tst2 -sta {sta2 enp1s0} \
    -console {md07end3:40002} -power {NPC15 2} \
    {*}$stacmn

UTF::Linux 10.19.61.90 -sta {sta3 enp1s0} \
    -console {md07end3:40000} -power {NPC19 2} \
    {*}$stacmn

######################################
#   4908
######################################

package require UTF::DSL
UTF::DSL 4908 -sta {dummy dummy} \
    -model 4908 \
    -jenkins "" \
    -relay lan2 \
    -tag BISON04T_BRANCH_7_14 \
    -p4tag "Dev" \
    -brand LINUX_4.1.0_ROUTER_DHDAP/94908HND_int \
    -type bcm94908HND_nand_fs_image_128_ubi.w \
    -lan_ip 192.168.1.1 \
    -lanpeer {lan1 lan} \
    -console "md07end3:40012" \
    -relay lan2 \
    -power {NPC19 2} \
    -nvram {
	watchdog=60000
	wl0_ssid=4908/0
	wl1_ssid=4908/1
	wl2_ssid=4908/2
	wl0_chanspec=161
	wl1_chanspec=36
	wl2_chanspec=3
	wl0_radio=0
	wl1_radio=0
	wl2_radio=0
	wl0_vht_features=6
	wl1_vht_features=6
	wl2_vht_features=7
	wl0_frameburst=on
	wl1_frameburst=on
	wl2_frameburst=on
	wan_ifname=
	wan_ifnames=
	{lan_ifnames=eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7}
	wl0_country_code=ALL
	wl1_country_code=ALL
	wl2_country_code=ALL
	wl0_dyn160=1
	wl1_dyn160=1
	wl2_dyn160=1
	wl0_obss_dyn_bw=0
	wl1_obss_dyn_bw=0
	wl2_obss_dyn_bw=0
    } \
    -datarate {-i 0.5} -udp 1.8g \
    -yart {-attn5g {0-90} -attn2g {48-90} -pad 29} \
    -noradio_pwrsave 1 \
    -wlinitcmds {
	echo 2 > /proc/irq/8/smp_affinity;
	echo 4 > /proc/irq/9/smp_affinity;
	echo 8 > /proc/irq/10/smp_affinity;
    }

dummy configure -attngrp G1

dummy clone 4908/0 -sta {4908/0 eth5 4908/0.%8 wl0.%} \
    -channelsweep {-min 100 -pretest 2} -perfchans {161/80}
dummy clone 4908/1 -sta {4908/1 eth6 4908/1.%8 wl1.%} \
    -channelsweep {-max 64 -pretest 2} -perfchans {36/80}
dummy clone 4908/2 -sta {4908/2 eth7 4908/2.%8 wl2.%} \
    -channelsweep {-band b} -perfchans {3}

dummy destroy

set external {
    -fwoverlay 1
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
    -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-stamon-hostpmac-murx-splitassoc-dyn160
}
set external {
    -brand LINUX_4.1.0_ROUTER_DHDAP/94908HND_external_full
    -type bcm94908HND_nand_cferom_fs_image_128_ubi*.w
}

4908/0 clone 4908x/0 {*}$external -perfonly 1
4908/1 clone 4908x/1 {*}$external -perfonly 1
4908/2 clone 4908x/2 {*}$external -perfonly 1

set internal {
    -fwoverlay 1
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas
    -type 4366c0-roml/config_pcie_internal
}

4908/0 clone 4908i/0 {*}$internal
4908/1 clone 4908i/1 {*}$internal
4908/2 clone 4908i/2 {*}$internal

4908x/0 configure \
    -dualband {4908x/1 -c2 36/80 -c1 161/80 -lan1 {lan lan1} -lan2 {lan2 lan3}}

##

4908/0 clone 4908b131/0 -tag BISON04T_TWIG_7_14_131 -p4tag REL502L02
4908/1 clone 4908b131/1 -tag BISON04T_TWIG_7_14_131 -p4tag REL502L02
4908/2 clone 4908b131/2 -tag BISON04T_TWIG_7_14_131 -p4tag REL502L02

4908x/0 clone 4908b164x/0 -tag BISON04T_TWIG_7_14_164 -p4tag REL502L02
4908x/1 clone 4908b164x/1 -tag BISON04T_TWIG_7_14_164 -p4tag REL502L02
4908x/2 clone 4908b164x/2 -tag BISON04T_TWIG_7_14_164 -p4tag REL502L02

4908x/0 clone 4908b170x/0 -tag BISON04T_TWIG_7_14_170 -p4tag REL502L02
4908x/1 clone 4908b170x/1 -tag BISON04T_TWIG_7_14_170 -p4tag REL502L02
4908x/2 clone 4908b170x/2 -tag BISON04T_TWIG_7_14_170 -p4tag REL502L02

########

# NIC mode

set UTF::PerfCacheMigrate {
    4908   4908b170
}

# WAR for broken nvram
set nic {
    -wlinitcmds {
	echo 2 > /proc/irq/8/smp_affinity;
	echo 4 > /proc/irq/9/smp_affinity;
	echo 8 > /proc/irq/10/smp_affinity;
	wl -i eth5 fcache 1;
	wl -i eth6 fcache 1;
	wl -i eth6 fcache 1;
    }
}

4908/0 clone 4908nx/0 {*}$nic
4908/1 clone 4908nx/1 {*}$nic
4908/2 clone 4908nx/2 {*}$nic

4908nx/0 configure \
    -dualband {4908nx/1 -c2 36/80 -c1 161/80 -lan1 {lan lan1} -lan2 {lan2 lan3}}






#############################

set UTF::RouterNightlyCustom {
    if {[regexp {(.*x)/} $Router - base]} {
	# external
	if {$STA3 ne ""} {

	    package require UTF::Test::TripleBand

	    TripleBand ${base}/0 ${base}/1 ${base}/2 $STA1 $STA2 $STA3 \
		-c1 161/80 -c2 36/80 -c3 3l \
		-lan1 {lan lan1} \
		-lan2 {lan2 lan3} \
		-lan3 {lan1 lan3}

	}
    }

    # PSR not working on 4908.
    # Test PSTA alone before Repeater tests
    package require UTF::Test::MiniUplinks
    UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 -nowet -nodwds

    package require UTF::Test::Repeaters
    UTF::Test::Repeaters $Router $STA1 -sta $STA2 -nopsta

}

UTF::Q md07r md07end3
