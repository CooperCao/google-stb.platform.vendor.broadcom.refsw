# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 6fb97528601741faa34217aa123ee2aeaace72ef $
#
# Testbed configuration file for MC41testbed
#

source "utfconf/mc40shared.tcl"

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc40c"

C configure -default 16

set UTF::SetupTestBed {
    C attn default
    foreach S {43602bAP 43602 _43602} {
	catch {$S wl down}
	$S deinit
    }
    return
}

proc build {{path "/tmp/tima/trunk"} {r ""}} {
    if {$r eq "" && [info exists ::env(SVNREV)]} {
	set r $::env(SVNREV)
    }
    regsub {^[-r]*(?=.)} $r -r r
    UTF::Record "update $r" {
	localhost -t 720 hnd scm up {*}$r $path
    }
    UTF::Record "build" {
	set f [localhost -async -t 720 make -j8 -C $path/src/wl/linux SHOWWLCONF=1 LINUXVER=3.11.1-200.fc19.x86_64 GCCVER=4.8.2 debug-apdef-stadef debug-apdef-stadef-p2p-mchan-tdls]
	localhost -t 720 make -j8 -C $path/src/wl/linux SHOWWLCONF=1 LINUXVER=4.0.4-301.fc22.x86_64 LINUXDIR=/tools/linux/src/linux-4.0.4-301.fc22.x86_64 CROSS_COMPILE=/tools/linux/local/cross/rhel6-64/fc22/bin/x86_64-fc22.4.0.4.301-linux- GCCVER=5.1.1 TARGET_OS=fc22 TARGET_ARCH=x86_64 debug-apdef-stadef-p2p-mchan-tdls
	$f close
    }
}

#####################

# SoftAP
UTF::Linux mc40tst8 -sta {43602tAP enp1s0 43602tAP#1 wl0.1} \
    -console mc40end1:40009 \
    -power {nb 7} \
    -wlinitcmds {wl msglevel +assoc +regulatory;wl down;wl country '#a/0';wl dtim 3;wl bw_cap 2g -1;wl vht_features 3} \
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1

43602tAP configure -attngrp C -ipaddr 192.168.1.99 -hasdhcpd 1

43602tAP clone 43602bAP -tag BISON_BRANCH_7_10
43602tAP clone 43602b35AP -tag BISON05T_BRANCH_7_35
43602tAP clone 43602eAP -tag EAGLE_BRANCH_10_10
43602tAP clone 43602hAP -tag HORNET_BRANCH_12_10 \
    -type debug-apdef-stadef-p2p-mchan-tdls


####################
# Brix
####################

UTF::Linux mc40tst7 -sta {_43602 enp2s0 _43602tAP#1 wl0.1} \
    -console mc40end1:40005 -power {nb 8} \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl vht_features 3} \
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5} -udp 1.2g -docpu 1 \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95} \
    -perfchans {36/80 3}

if {1} {
    snit::method UTF::STA add_networks {args} {
	if {[$self cget -ipaddr] == "192.168.2.80"} {
	    $self ip route change to 192.168.2.0/24 via 192.168.2.80 \
		initcwnd 1500
	} else {
	    foreach AP $args {
		if {[$AP cget -ipaddr] == "192.168.2.80"} {
		    $AP ip route change to 192.168.2.0/24 via 192.168.2.80 \
			initcwnd 1500
		}
	    }
	}
    }
}

_43602 configure -attngrp C -ipaddr 192.168.2.80 -hasdhcpd 1
_43602 clone _43602b -tag BISON_BRANCH_7_10
_43602 clone _43602b35 -tag BISON05T_BRANCH_7_35
_43602 clone _43602e -tag EAGLE_BRANCH_10_10
_43602 clone _43602n -tag NANDP_BRANCH_1_2
_43602 clone _43602h -tag HORNET_BRANCH_12_10

UTF::DHD mc40tst7d -lan_ip mc40tst7 -sta {_43602fd eth0} \
    -hostconsole {mc40end1:40005} \
    -power {nb 8} \
    -dhd_brand linux-internal-media \
    -driver dhd-msgbuf-pciefd-debug \
    -brand linux-internal-dongle-pcie \
    -type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-nan-assert-err-logtrace-redux/rtecdc.bin \
    -wlinitcmds {wl down;wl vht_features 3} \
    -slowassoc 5 -extsup 1 \
    -datarate {-i 0.5} \
    -perfchans {36/80 3} \
    -tcpwindow 4m -udp 1.2g -nocal 1 -docpu 1 \
    -yart {-pad 23 -attn5g 37-95 -attn2g 50-95}

_43602fd configure -ipaddr 192.168.2.80 -hasdhcpd 1

_43602fd clone _43602fxb -perfonly 1 -perfchans {36/80}

_43602fd clone _43602nfd -tag NANDP_BRANCH_1_2 -dhd_tag DHD_BRANCH_1_363

#################################
# ASUS
#################################
UTF::Linux mc40tst9 -sta {43602 enp1s0 43602#1 wl0.1} \
    -console mc40end1:40002 \
    -power {nb 6} \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -wlinitcmds {wl msglevel +assoc +regulatory;wl down;wl country '#a/0';wl vht_features 3} \
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5 -auto} -udp 1.2g -docpu 1 \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95} \
    -perfchans {36/80 3}

####

43602 clone 43602b -tag BISON_BRANCH_7_10
43602 clone 43602b35 -tag BISON05T_BRANCH_7_35
43602 clone 43602e -tag EAGLE_BRANCH_10_10
43602 clone 43602n -tag NANDP_BRANCH_1_2 \
    -wlinitcmds {wl msglevel +assoc +regulatory;wl down;wl vht_features 3;wl msch_collect 1}
43602 clone 43602h -tag HORNET_BRANCH_12_10

#set UTF::PerfCacheMigrate {
#    TOT HORNET
#    43602 43602h
#    _43602 _43602h
#    43602tAP 43602hAP
#}


43602 clone 43602.11n \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl vhtmode 0} -nocustom 1
43602.11n clone 43602e.11n -tag EAGLE_BRANCH_10_10

UTF::DHD mc40tst9d -lan_ip mc40tst9 -sta {43602fb eth0} \
    -hostconsole {mc40end1:40002} \
    -power {nb 6} \
    -tag BISON_BRANCH_7_10 \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -brand linux-internal-dongle-pcie \
    -type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-err-assert/rtecdc.bin \
    -wlinitcmds {wl down;wl vht_features 3} \
    -slowassoc 5 -extsup 1 \
    -datarate {-i 0.5 -auto} \
    -perfchans {36/80 3} \
    -tcpwindow 4m -udp 1.2g -nocal 1 -docpu 1 \
    -yart {-pad 23 -attn5g 37-95 -attn2g 50-95}

43602fb clone 43602fb35 -tag BISON05T_BRANCH_7_35 \
    -brand linux-internal-media \
    -type 43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-err-assert-dbgam.bin


43602fb clone 43602fxb \
    -type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus/rtecdc.bin \
    -perfonly 1 -perfchans {36/80}

_43602fxb clone _43602fb35x -tag BISON05T_BRANCH_7_35 \
    -brand linux-external-media \
    -type 43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive.bin

43602fb clone 43602tfd \
    -tag trunk \
    -type 43602a1-ram/pcie-ag-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-err-logtrace-redux-clm_min/rtecdc.bin \
    -clm_blob 43602a1.clm_blob \
    -perfchans {36/80}

# DHDAP
43602fb clone 43602fb14 -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -dhd_tag DHD_TWIG_1_363_45 \
    -dhd_brand linux-internal-media \
    -type 43602a3-roml/pcie-ag-splitrx-fdap-mbss-mfp-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-ringer-dmaindex16-bssload-wnm-splitassoc-dbwsw-stamon-txpwr-err-assert-dbgam-dbgams-ringer-acksupr-authrmf-dhdhdr-el

43602fxb clone 43602fb14x -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
    -dhd_tag DHD_TWIG_1_363_45 \
    -dhd_brand linux-internal-media \
    -type 43602a3-roml/pcie-ag-splitrx-fdap-mbss-mfp-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-ringer-dmaindex16-bssload-wnm-splitassoc-dbwsw-stamon-acksupr-authrmf-dhdhdr \

_43602fxb clone _43602fb14x -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
    -dhd_brand linux-internal-dongle-pcie \
    -dhd_tag trunk \
    -type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-ringer-dmaindex16-bssload-dhdhdr \
    -hostconsole mc40end1:40005 -power {nb 8} -extsup 0

43602fb clone 43602fb89 -tag BISON04T_{TWIG,REL}_7_14_89{,_*} \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -dhd_tag DHD_BRANCH_1_363 \
    -type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-err-assert-dbgam-dbgams

43602fxb clone 43602fb89x -tag BISON04T_{TWIG,REL}_7_14_89{,_*} \
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
    -dhd_tag DHD_BRANCH_1_363 \
    -type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop


_43602fd clone _43602fb131x -tag BISON04T_TWIG_7_14_131 \
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
    -type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-ringer-dmaindex16 \
    -perfonly 1 -perfchans {36/80}

# Eagle
43602fb clone 43602fe -tag EAGLE_BRANCH_10_10 \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -type 43602a1-ram/pcie-ag-splitrx-fdap-mbss-mfp-chkd2hdma

43602fxb clone 43602fex -tag EAGLE_BRANCH_10_10 \
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
    -type 43602a1-ram/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus



43602tfd clone 43602tfdx \
    -type 43602a1-ram/pcie-ag-err-splitrx/rtecdc.bin \
    -perfchans {36/80} -perfonly 1

# Dingo
43602tfd clone 43602dfd -tag DINGO_BRANCH_9_10
43602tfdx clone 43602dfdx -tag DINGO_BRANCH_9_10

# NANDP
43602tfd clone 43602nfd -tag NANDP_BRANCH_1_2 -dhd_tag DHD_BRANCH_1_363 \
    -type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-nan-assert-err-logtrace-redux/rtecdc.bin \
    -wlinitcmds {wl down;wl vht_features 3;wl msch_collect 1;:}


43602tfd clone 43602tdls \
    -type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-norsdb-tdls-assert/rtecdc.bin

set UTF::StaNightlyCustom {
    if {$(ap2) ne ""} {
	if {[$STA hostis Linux]} {
	    package require UTF::Test::MiniP2P
	    MiniP2P $(ap2) $STA -ap $Router
	    MiniP2P $(ap2) $STA -ap $Router -apchanspec 3
	}
	if {[$STA hostis Linux DHD] && ![$STA cget -extsup]} {
	    # Can't do MultiSTA with wpa_supplicant yet
	    package require UTF::Test::MultiSTANightly
	    MultiSTANightly -ap1 $Router -ap2 $(ap2) -sta $STA \
		-nosetuptestbed -nostaload -nostareload -nosetup \
		-noapload -norestore -nounload
	    package require UTF::Test::APSTA
	    APSTA $Router $(ap2) $STA
	}
	if {[$STA hostis Linux]} {
	    # Move ap2 to the same subnet and turn off dhcp
	    set ip [$(ap2) cget -ipaddr]
	    $(ap2) configure -ipaddr 192.168.1.80 -hasdhcpd 0
	    try {
		TDLS $Router $STA $(ap2) -warmup 10
	    } finally {
		$(ap2) configure -ipaddr $ip -hasdhcpd 1
	    }
	}


    }

    if {!$(norvr) && [$STA hostis Linux]} {
	if {[catch {
	    package require UTF::Test::YART
	    $STA wl down
	    $STA wl vhtmode 0
	    YART $Router $STA -msg "$STA: 11n" -key 11n -chanspec 36l
	    $STA wl down
	    $STA wl vhtmode 1
	} ret]} {
	    UTF::Message WARN $STA $ret
	}
    }
}

#####
UTF::Q mc40c
