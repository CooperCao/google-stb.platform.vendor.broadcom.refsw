# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 3f8c85d369239949911cd88775fb2886fdb14850 $
#

######
#
# UTF Ramsey1 in SW Lab 5010
#

# Shared resources
source utfconf/ramsey_shared.tcl


# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_software/utf/ramsey1"

R1 configure -default 12

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    R1 attn default
}

#####################
# 1st small Ramsey box

UTF::Linux UTFTestZ -sta {4366a enp1s0} \
    -console "UTFTestC:40000" -tag EAGLE_BRANCH_10_10 \
    -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl down;wl country '#a/0';wl dtim 3;wl vht_features 7} \
    -tcpwindow 4m

4366a configure -ipaddr 192.168.1.50 -hasdhcpd 1 -attngrp R1

#####################
# 3rd small Ramsey box

UTF::Linux UTFTestY -sta {4366b enp1s0} \
    -console "UTFTestC:40001" -tag EAGLE_BRANCH_10_10 \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -power {UTFPower6 6} -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl down;wl country '#a/0';wl dtim 3;wl vht_features 7;wl txchain 9;wl rxchain 9} \
    -tcpwindow 3m -udp 1g

4366b configure -ipaddr 192.168.2.94 -hasdhcpd 1
#4366b configure -ipaddr 192.168.1.81 -hasdhcpd 0
##################

UTF::DHD UTFTestG -sta {4361b0 eth0} \
    -power {UTFPower6 1} -hostconsole "UTFTestF:40001" \
    -brand hndrte-dongle-wl \
    -dhd_brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -tag IGUANA08T_BRANCH_13_35 \
    -nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
    -nvram_add "macaddr=00:90:4c:12:d0:01" \
    -clm_blob ss_mimo.clm_blob \
    -type 4361b0-roml/config_pcie_utf_ipa/rtecdc.bin \
    -wlinitcmds {wl down;wl vht_features 7;wl wnm_bsstrans_resp 0} \
    -nocal 1 -datarate {-i 0.5 -auto} -udp 1g -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 21-95 -attn2g 22-95 -pad 23} \
    -channelsweep {-no2g40} -escan 1 \
    -perfchans {36/80 3} -usecsa 0


4361b0 clone 4361b0r \
    -type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
    -perfchans {36/80} -perfonly 1 -noaes 1 -notkip 1

4361b0 clone 4361b0t \
    -type 4361b0-roml/config_pcie_tput_ipa/rtecdc.bin \
    -perfchans {36/80} -perfonly 1 -noaes 1 -notkip 1

###

UTF::DHD UTFTestR -sta {4361b0b eth1} \
    -power {UTFPower6 3} -hostconsole "UTFTestF:40002" \
    -brand hndrte-dongle-wl \
    -dhd_brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -tag IGUANA08T_BRANCH_13_35 \
    -nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
    -nvram_add "macaddr=00:90:4c:12:d0:02" \
    -clm_blob ss_mimo.clm_blob \
    -type 4361b0-roml/config_pcie_utf_ipa/rtecdc.bin \
    -wlinitcmds {wl down;wl vht_features 7;wl wnm_bsstrans_resp 0} \
    -nocal 1 -datarate {-i 0.5 -auto} -udp 1g -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 18-95 -attn2g 19-95 -pad 26} \
    -channelsweep {-no2g40} \
    -perfchans {36/80 3} -usecsa 0

4361b0b clone 4361b0br \
    -type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
    -perfchans {36/80} -perfonly 1 -noaes 1 -notkip 1

4361b0 configure -dualband {4366b 4361b0 -c1 36/80 -c2 3 -b1 800m -b2 800m}
4361b0r configure -dualband {4366b 4361b0r -c1 36/80 -c2 3 -b1 800m -b2 800m}

set UTF::StaNightlyCustom {
    package require UTF::Test::MiniP2P
    package require UTF::Test::APSTA
    package require UTF::Test::TDLS
    package require UTF::Test::MiniMu
    package require UTF::Test::BeaconRatio2

    if {$(ap2) ne ""} {
	if {1} {
	    MiniP2P $(ap2) $STA -ap $Router -apchanspec 3 -security open
	    BeaconRatio2 $Router $(ap2) $STA
	    APSTA $Router $(ap2) $STA -chan2 11
	}

	if {1 || [regexp {_tput|_release} [$STA cget -type]]} {
	    # Move ap2 to the same subnet and turn off dhcp
	    set ip [$(ap2) cget -ipaddr]
	    $(ap2) configure -ipaddr 192.168.1.81 -hasdhcpd 0
	    try {
		TDLS $Router $STA $(ap2) -security open
		MiniMu $Router [list $STA $(ap2)] -PM 0
		#MiniMu $Router [list $STA $(ap2)] -PM 2 -msg "MuMimo PM2"
	    } finally {
		$(ap2) configure -ipaddr $ip -hasdhcpd 1
	    }
	}
    }
}



####
UTF::Q ramsey1 UTFTestC
