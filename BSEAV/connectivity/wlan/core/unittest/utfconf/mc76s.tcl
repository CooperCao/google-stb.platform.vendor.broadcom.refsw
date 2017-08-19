# -*-tcl-*-

#
#  $Copyright Broadcom Corporation$
#  $Id: e6045ed0e0dbd0b388cdacfd162b42b850117552 $ 2014/08/08  20:28:46 sotmishi Exp $
#
#  Testbed configuration file for MC76 Testbed
#

set UTF::CountedErrors 1
set UTF::FBDefault 1

source "utfconf/mc76.tcl"

set ::UTF::SummaryDir "/projects/hnd_sig_ext12/sotmishi/mc76s"

# Packages
package require UTF::Aeroflex
UTF::Aeroflex Afs -lan_ip 10.19.86.251 -relay mc76end1 \
    -group {G1 {1 2 3} G2 {4 5 6}} -retries 100

set UTF::SetupTestBed {
    G2 attn default
    foreach s {43602bAP 4364 4355c0} {
        catch {$s wl down}
        $s deinit
    }
    return
}

# Power switches/web relays
UTF::Power::Synaccess npc66 -lan_ip 172.16.1.66 -relay mc76end1 -rev 1
UTF::Power::Synaccess npc95 -lan_ip 172.16.1.95 -relay mc76end1 -rev 1
UTF::Power::Synaccess npc96 -lan_ip 172.16.1.96 -relay mc76end1 -rev 1

#####################

# SoftAP
UTF::Linux mc76tst3 -sta {43602tAP enp1s0 43602tAP#1 wl0.1} \
    -console {mc76end1:40006} \
    -power {npc66 1} \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -wlinitcmds {wl msglevel +assoc +regulatory;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3} \
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1

43602tAP configure -attngrp G2 -ipaddr 192.168.1.80 -hasdhcpd 1

43602tAP clone 43602bAP -tag BISON_BRANCH_7_10
43602tAP clone 43602b35AP -tag BISON05T_BRANCH_7_35
43602tAP clone 43602eAP -tag EAGLE_BRANCH_10_10
43602tAP clone 43602hAP -tag HORNET_BRANCH_12_10

#########
#
########

UTF::Linux mc76tst7 -sta {_43602 enp1s0} \
    -console {mc76end1:40001} \
    -power {npc95 1} \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -wlinitcmds {wl msglevel +assoc +regulatory; wl vht_features 3} \
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5} -udp 1.2g -docpu 1 \
    -yart {-attn5g 8-63 -attn2g 8-63} \
    -perfchans {36/80 3}

_43602 configure -attngrp G2 -ipaddr local -hasdhcpd 1
_43602 clone _43602b -tag BISON_BRANCH_7_10
_43602 clone _43602b35 -tag BISON05T_BRANCH_7_35
_43602 clone _43602e -tag EAGLE_BRANCH_10_10
_43602 clone _43602n -tag NANDP_BRANCH_1_2
_43602 clone _43602h -tag HORNET_BRANCH_12_10
package require UTF::AWDL
UTF::DHD _mc76tst7 -lan_ip mc76tst7 -sta {4364 eth0 4364.%2 wl0.%} \
    -hostconsole mc76end1:40001 \
    -dhd_tag DHD_BRANCH_1_579 \
    -power {npc95 1} \
    -brand hndrte-dongle-wl \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "src/shared/nvram/bcm94364fcpagb_2.txt" \
    -clm_blob 4364b0.clm_blob \
    -type 4364a0-ram/config_pcie_release_norsdb_row/rtecdc.bin \
    -tcpwindow 4M \
    -wlinitcmds {wl down;wl vht_features 3} \
    -datarate {-i 0.5 -auto} -udp 1.2g -escan 1 \
    -yart {-pad 23 -attn5g 25-63 -attn2g 40-63} \
    -perfchans {36/80 3} -nocal 1\
    -channelsweep {-no2g40} -nomimo_bw_cap 1 \
    -msgactions {
	{PCIe Bus Error} WARN
    } \
    -initscript {
	set bssid "c0:ff:ee:c0:ff:ee"
	set awdl [UTF::AWDL::create %S $bssid]
	UTF::AWDL::idle $awdl $bssid -A 149/80
    }


4364 clone 4364b2 \
    -tag DIN2915T250RC1_BRANCH_9_30 \
    -type 4364b2-roml/config_pcie_release_sdb_udm/rtecdc.bin \
    -clm_blob 4364b0.clm_blob

4364b2 clone 4364b2x \
    -type 4364b2-roml/config_pcie_perf_sdb_udm/rtecdc.bin \
    -perfchans {36/80}

4364b2x clone ref2
ref2 configure -ipaddr 192.168.2.81 -hasdhcpd 1

##############
#
########

UTF::DHD mc76tst8 -sta {_4364 eth1} \
    -hostconsole mc76end1:40003 \
    -dhd_tag DHD_BRANCH_1_579 \
    -power {npc95 2} \
    -brand hndrte-dongle-wl \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "src/shared/nvram/bcm94364fcpagb_2.txt" \
    -type 4364a0-ram/config_pcie_release_norsdb_row/rtecdc.bin \
    -tcpwindow 4M \
    -wlinitcmds {wl down;wl vht_features 3} \
    -datarate {-i 0.5 -auto} -udp 1.2g -escan 1 \
    -yart {-pad 23 -attn5g 25-63 -attn2g 40-63} \
    -perfchans {36/80 3} -nocal 1 \
    -channelsweep {-no2g40} -nomimo_bw_cap 1 \
    -postcopy {$self rexec ./skylake_aspm.sh enable} \
    -msgactions {
	{PCIe Bus Error} WARN
    }

_4364 clone _4364b2 \
    -tag DIN2915T250RC1_BRANCH_9_30 \
    -type 4364b2-roml/config_pcie_release_sdb_udm/rtecdc.bin \
    -clm_blob 4364b0.clm_blob

_4364b2 clone _4364b2x \
    -type 4364b2-roml/config_pcie_perf_sdb_udm/rtecdc.bin \
    -perfchans {36/80}

_4364b2x clone _ref2
_ref2 configure -ipaddr 192.168.2.82 -hasdhcpd 1

##################

UTF::DHD mc76tst9 -sta {4355c0 eth0 4355c0.%2 wl0.%} \
    -hostconsole mc76end1:40002 \
    -power {npc96 1} \
    -type 4355c0-roml/config_pcie_debug/rtecdc.bin \
    -nvram "bcm94355fcpagb.txt" \
    -clm_blob "4355_kristoff.clm_blob" \
    -tag DIN2930R18_BRANCH_9_44 \
    -dhd_tag DHD_BRANCH_1_579 \
    -brand hndrte-dongle-wl \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -escan 1 -udp 800m -tcpwindow 3M \
    -wlinitcmds {wl down;wl country US;wl vht_features 3} \
    -datarate {-i 0.5 -auto} \
    -yart {-pad 23 -attn5g 25-63 -attn2g 40-63} \
    -perfchans {36/80 3} -nocal 1 -nomimo_bw_cap 1 -usecsa 0 \
    -channelsweep {-no2g40} \
    -initscript {
	set bssid "c0:ff:ee:c0:ff:ee"
	set awdl [UTF::AWDL::create %S $bssid]
	# modify device for 4355 so we can always use .2 for AWDL
	%S.2 configure -device [$awdl cget -device]
	UTF::AWDL::idle $awdl $bssid -A 149/80
    }


set xconfig {
    -nopm1 1 -nopm2 1 -nobighammer 1 -nowep 1 -notkip 1
    -nofragmentation 1 -perfonly 1 -perfchans {36/80}
}


4355c0 clone 4355c0x \
    -type 4355c0-roml/config_pcie_release/rtecdc.bin {*}$xconfig

4355c0x clone ref3
ref3 configure -ipaddr 192.168.3.81 -hasdhcpd 1

4355c0 clone 4355c0d30 -tag DIN2915T250RC1_BRANCH_9_30 \
    -type 4355c0-roml/config_pcie_debug_ios/rtecdc.bin
4355c0x clone 4355c0d30x -tag DIN2915T250RC1_BRANCH_9_30 \
    -type 4355c0-roml/config_pcie_release_ios/rtecdc.bin

set UTF::PerfCacheMigrate {
    4355b3 4355c0
    \#1 .2
}

########################

set UTF::StaNightlyCustom {
    #package require UTF::Test::MiniP2P
    package require UTF::Test::APSTA
    #package require UTF::Test::TDLS
    #package require UTF::Test::MiniMu
    package require UTF::Test::MiniAWDL
    if {$(ap2) ne ""} {
	MiniAWDL $STA $(ap2) -ap $Router -sync 36/80 -P1I ${STA}.2 -P2I $(ap2).2
	APSTA $Router $(ap2) $STA -chan2 {36/80}
    }
}

UTF::Q mc76s
