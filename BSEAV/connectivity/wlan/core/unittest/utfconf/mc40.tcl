# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: a2562105a26ebfa4485dfb1d798cf83ac556080d $
#
# Testbed configuration file for MC41testbed
#

source "utfconf/mc40shared.tcl"

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc40"

set UTF::SetupTestBed {
    G attn default
    foreach S {4352a 4352b 43596a0} {
	catch {$S wl down}
	$S deinit
    }
    return
}

#####################

# SoftAP
UTF::Linux mc40tst1 -sta {4352a enp1s0} \
    -reloadoncrash 1 \
    -console mc40end1:40004 \
    -power {nb 1} \
    -tag BISON05T_BRANCH_7_35 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3}

4352a configure -attngrp G -ipaddr 192.168.1.80 -hasdhcpd 1

#####################

#####################

# 2nd SoftAP
UTF::Linux mc40tst5 -sta {4352b enp1s0} \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -reloadoncrash 1 \
    -console mc40end1:40001 \
    -power {nb 3} \
    -tag BISON05T_BRANCH_7_35 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3}

4352b configure -attngrp G -ipaddr 192.168.2.80 -hasdhcpd 1

#####################

#	4349 => 2(1x1) only. Pure RSDB chip
#	4355 => 2x2 only. Pure MIMO chip
#	4359 => RSDB/MIMO/80p80 - Reconfigurable - Multi-mode chip


UTF::DHD mc40tst3 -sta {43596a0 eth0} \
    -power {nb 2} \
    -hostconsole "mc40end1:40003" \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-cfg80211-debug \
    -brand hndrte-dongle-wl \
    -tag DIN975T155RC31_BRANCH_9_87 \
    -dhd_tag DHD_BRANCH_1_579 \
    -nocal 1 -slowassoc 10 \
    -nvram "bcm943596fcpagbss.txt" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
    -clm_blob "ss_mimo.clm_blob" \
    -wlinitcmds {wl down;wl vht_features 3;wl PM 0} \
    -udp 800m -tcpwindow 2m \
    -yart {-attn5g 16-95 -attn2g 40-95 -pad 26}

43596a0 clone 43596a0x \
    -type 43596a0-roml/config_pcie_release/rtecdc.bin \
    -perfonly 1 -perfchans 36/80 -extsup 1 -nocustom 1 \
    -post_assoc_hook {%S innetworkwait $AP 6}

43596a0 clone 43596a0.2 -sta {_43596a0 eth0 43596a0.2 wl0.2} \
    -wlinitcmds {wl down;wl vht_features 3;wl interface_create sta} \
    -perfonly 1 -perfchans 3l -nocustom 1

43596a0.2 configure -dualband {4352b 43596a0 -c1 36/80 -c2 3l -b1 800m -b2 800m}

#set UTF::PerfCacheMigrate {
#    DIN07T48RC50 DIN975T155RC31
#}

###

UTF::Linux mc40tst3n -lan_ip mc40tst3 -sta {4359n enp1s0} \
    -type debug-apdef-stadef-norsdb \
    -power {nb 2} -tag DINGO_BRANCH_9_10 \
    -console {mc40end1:40003} \
    -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5}  \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3} \
    -udp 800m -tcpwindow 3m \
    -yart {-attn5g 16-95 -attn2g 40-95 -pad 26} \
    -perfchans 36/80 -nopm1 1 -nopm2 1 -channelsweep {-band a}

4359n clone 4359ex \
    -type debug-apdef-stadef-norsdb-extnvm -nvram "bcm943593fcpagbss.txt"

###

set UTF::StaNightlyCustom {
    if {$(ap2) ne ""} {
	package require UTF::Test::MiniP2P
	MiniP2P $(ap2) $STA -ap $Router
	MiniP2P $(ap2) $STA -ap $Router -apchanspec 3

	UTF::Try "+RSDB Mode Switch" {
	    package require UTF::Test::RSDBModeSwitch
	    RSDBModeSwitch $Router $(ap2) $STA ${STA}.2
	}
	package require UTF::Test::MultiSTANightly
	MultiSTANightly -ap1 $Router -ap2 $(ap2) -sta $STA \
	    -nosetuptestbed -nostaload -nostareload -nosetup \
	    -noapload -norestore -nounload
	package require UTF::Test::APSTA
	APSTA $Router $(ap2) $STA -chan2 {161/80 11}
    }
}


#####
UTF::Q mc40

