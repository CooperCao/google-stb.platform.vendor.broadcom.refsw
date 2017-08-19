# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: f09c23c2ff65f656295465037c5831a4e8a95799 $
#

######
#
# UTF Ramsey4 in SW Lab 5010
#

# Shared resources
source utfconf/ramsey_shared.tcl

set ::UTF::SummaryDir "/projects/hnd_software/utf/ramsey4"

R4 configure -default 10

set tag "EAGLE_BRANCH_10_10"
set ::build_dir "/home/tima/lwork4/media/EAGLE_BRANCH_10_10"

set UTF::SetupTestBed {
    if {[info exists ::env(SVNREV)]} {
	::build
    }
    UTF::Try "Reset attenuator" {
	R4 attn default
	return
    }
}

#####################
# 1st small Ramsey box

UTF::Linux UTFTestM -sta {4366a enp1s0} \
    -tag $tag\
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -console "UTFTestF:40003" -power {UTFPower6 2} -reloadoncrash 1 \
    -wlinitcmds {wl dtim 3;wl down;wl country '#a/0';wl bw_cap 2 -1;wl vht_features 7}
4366a configure -ipaddr 192.168.1.80 -attngrp R4 -hasdhcpd 1



4366a clone 4366a160 \
    -wlinitcmds {wl dtim 3;wl down;wl bw_cap 2 -1;wl vht_features 7;wl country '#a/0'}

4366a160 clone 4366a131 -tag EAGLE_TWIG_10_10_69


UTF::DHD _UTFTestM -lan_ip UTFTestM -sta {4366af eth0} \
    -hostconsole "UTFTestF:40003" \
    -power {UTFPower6 2} \
    -tag BISON04T_BRANCH_7_14 \
    -dhd_tag DHD_BRANCH_1_363 \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -type 4366c0-roml/config_pcie_internal.bin \
    -clm_blob 4366_access.clm_blob \
    -wlinitcmds {wl down;wl bw_cap 2 -1;wl vht_features 7;wl country '#a/0'} \
    -slowassoc 5 \
    -tcpwindow 4m -udp 1.8g -nocal 1

4366af configure -ipaddr 192.168.1.80 -attngrp R4 -hasdhcpd 1

4366af clone 4366af131 -tag BISON04T_TWIG_7_14_131 \
    -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-mfgtest-dump-splitassoc-acksupr-stamon-authrmf-hostpmac \
    -clm_blob ""

#####################

UTF::Linux UTFTestQ -sta {4366b enp1s0} -tag $tag \
    -power {UTFPower6 7} \
    -console "UTFTestS:40001" \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -yart {-attn5g {17-95} -attn2g {27-95} -pad 23} \
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5 -auto} -udp 1.8g \
    -wlinitcmds {wl dtim 3;wl down;wl country '#a/0';wl bw_cap 2 -1;wl vht_features 7}

4366b configure -ipaddr 192.168.2.80 -attngrp R4 -hasdhcpd 1

UTF::DHD _UTFTestQ -lan_ip UTFTestQ -sta {4366bf eth0} \
    -hostconsole "UTFTestS:40001" \
    -power {UTFPower6 7} \
    -tag BISON04T_BRANCH_7_14 \
    -dhd_tag DHD_BRANCH_1_363 \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -wlinitcmds {wl down;wl bw_cap 2 -1;wl vht_features 7} \
    -type 4366b1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-mfgtest-dump-acksupr-stamon-authrmf-hostpmac




#####################
# 2nd small Ramsey box

set UTF::ChanspecsPerBandBW 1; # WAR for trunk

UTF::Linux UTFTestP -sta {4366 enp1s0} \
    -power {UTFPower7 2} \
    -console "UTFTestF:40007" \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -yart {-attn5g {17-95} -attn2g {27-95} -pad 23} \
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5 -auto} -udp 1.8g \
    -wlinitcmds {wl msglevel +assoc;wl country '#a/0';wl down;wl bw_cap 2 -1;wl vht_features 7} \
    -perfchans {36/80 36/160 3} \
    -msgactions {
	"Too much or too little power" WARN
    }

4366 clone 4366c -tag EAGLE_BRANCH_10_10

UTF::DHD _UTFTestP -lan_ip UTFTestP -sta {4366f eth0} \
    -hostconsole "UTFTestF:40007" \
    -power {UTFPower7 2} \
    -tag EAGLE_BRANCH_10_10 \
    -dhd_tag DHD_BRANCH_1_363 \
    -brand linux-internal-media \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-cfg80211-debug \
    -type 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-wowlpf-pktfilter-ampduhostreorder-keepalive-chkd2hdma-ringer-dmaindex16-err-assert-dbgam.bin \
    -wlinitcmds {wl down;wl country '#a/0';wl bw_cap 2 -1;wl vht_features 7} \
    -slowassoc 5 \
    -datarate {-i 0.5 -auto} \
    -perfchans {36/80 3} \
    -tcpwindow 4m -udp 1.8g -nocal 1 -docpu 1 -noibss 1 \
    -yart {-attn5g {17-95} -attn2g {30-95} -pad 20}

# Temporarily use router FW as WAR for SWWLAN-95375
4366f configure \
    -brand linux-internal-dongle-pcie \
    -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin \
    -extsup 1

4366f clone 4366fx \
    -brand linux-external-media \
    -type 4365c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-mfp-proptxstatus-wowlpf-pktfilter-ampduhostreorder-chkd2hdma-ringer-dmaindex16-keepalive.bin \
    -perfonly 1 -perfchans {36/80} \
    -wlinitcmds {wl down;wl bw_cap 2 -1;wl vht_features 7} \

# DHDAP
# Disable WPA supplicant due to: SWWLAN-141888
4366f clone 4366fb14 -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -dhd_tag DHD_TWIG_1_363_45 -extsup 0 -noaes 1 -notkip 1 \
    -dhd_brand linux-internal-media \
    -type 4366c0-roml/config_pcie_internal

#    -clm_blob 4366_access.clm_blob

4366fx clone 4366fb14x -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
    -dhd_tag DHD_TWIG_1_363_45 -extsup 0 -noaes 1 -notkip 1 \
    -dhd_brand linux-internal-media \
    -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-stamon-hostpmac-murx-splitassoc-hostmemucode-dyn160-dhdhdr-fbt-htxhdr-amsdufrag

proc build {{r ""}} {
    if {$r eq "" && [info exists ::env(SVNREV)]} {
	set r $::env(SVNREV)
    }
    regsub {^[-r]*(?=.)} $r -r r
    set init ". .bash_profile;set -x;cd $::build_dir"
    UTF::Try "clean" {
	localhost rexec -t 240 ssh wlan-sw2 "$init && hnd -V scm scrub --revert --force"
	return
    }
    UTF::Try "update $r" {
	set i [localhost rexec -t 240 ssh wlan-sw2 "$init && hnd scm up $r && SUBVERSIONVER=1.7.8 svn info main/src"]
	if {[info exists UTF::Summary] &&
	    [regexp -line {Revision: (.*)} $i - r]} {
	    $UTF::Summary configure -title \
		[regsub {\)} [$UTF::Summary cget -title] "@$r)"]
	}
    }
    UTF::Try "compile NIC" {
	localhost rexec -t 240 ssh wlan-sw2 "$init && make -j8 -C main/src/wl/linux SHOWWLCONF=1 LINUXVER=3.11.1-200.fc19.x86_64 GCCVER=4.8.2 debug-apdef-stadef"
    }
#    UTF::Try "compile FD" {
#	localhost rexec -t 240 ssh wlan-sw2 "$init && make -j8 -C main/src/dongle/make/wl 4365b1-roml/pcie-ag-err-pktctx-splitrx-amsdutx-idauth-idsup-tdls-mfp-proptxstatus-wowlpf-pktfilter-ampduhostreorder-keepalive-chkd2hdma-ringer-dmaindex16-assert-dbgam"
#    }
}

if {[info exists ::env(SVNREV)]} {
    4366a configure -image $::build_dir/main/src
    4366b configure -image $::build_dir/main/src
    4366 configure -image $::build_dir/main/src
}

set UTF::StaNightlyCustom {
    package require UTF::Test::MultiSTANightly
    package require UTF::Test::APSTA
    package require UTF::Test::TDLS

    if {$(ap2) ne ""} {
	if {[$STA hostis Linux DHD] && ![$STA cget -extsup]} {
	    # Can't do MultiSTA with wpa_supplicant yet
	    MultiSTANightly -ap1 $Router -ap2 $(ap2) -sta $STA \
		-nosetuptestbed -nostaload -nostareload -nosetup \
		-noapload -norestore -nounload
	    APSTA $Router $(ap2) $STA
	}
	if {[$STA hostis Linux]} {
	    # Move ap2 to the same subnet and turn off dhcp
	    set ip [$(ap2) cget -ipaddr]
	    $(ap2) configure -ipaddr 192.168.1.81 -hasdhcpd 0
	    try {
		TDLS $Router $STA $(ap2) -security open
	    } finally {
		$(ap2) configure -ipaddr $ip -hasdhcpd 1
	    }
	}
    }
}

#####
UTF::Q ramsey4 UTFTestC
