# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 99553f62159480808d1b8efb35ff9a4b85d6802c $
#

set ::UTF::SummaryDir "/projects/hnd_sig/utf/ramsey3"

set ::UTF::SetupTestBed {
    G attn default
    foreach S {4360 4334tx 43143b0 43142} {
	catch {$S wl down}
	$S deinit
    }
}

UTF::Power::Synaccess UTFPower3

# Relay
UTF::Linux UTFTestU -sta {lan eth1}

UTF::Power::Synaccess npc1 -lan_ip 192.168.1.10 -relay lan

package require UTF::Vaunix
UTF::Vaunix G -lan_ip UTFTestU

UTF::Linux UTFTestX -sta {4360 enp1s0} \
    -tag "BISON05T_BRANCH_7_35" \
    -console UTFTestU:40000 -power {npc1 1} -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl dtim 3;wl bw_cap 2g -1;wl txbf 0;wl vht_features 3}

4360 configure -ipaddr 192.168.1.99 -attngrp G

#######################################

# console UTFTestU:40002 disabled

UTF::DHD UTFTestAA -sta {4334tx eth0} \
    -power {npc1 2} \
    -hostconsole UTFTestU:40001 \
    -console {UTFTestU:40002} \
    -modopts {sd_uhsimode=1} \
    -type 4334a0-remap/sdio-ag-pool-p2p-idsup-internal-assoc-dump-clm_4334_mfg-consuartcc/rtecdc.bin \
    -nvram bcm94334fcbgabu.txt \
    -nvram_add {
	# Disable noise cal (PR#103469 PR#103468)
	noise_cal_enable_2g=0
	noise_cal_enable_5g=0
	# Retune PA params
	pa0b0=5691
	pa0b1=-700
	pa0b2=-196

	maxp5ga0=0x36
	maxp5gla0=0x36
	maxp5gha0=0x36
	muxenab=0x1
    } \
    -postinstall {dhd -i eth0 sd_blocksize 2 256; dhd -i eth0 socdevram 1 0 1} \
    -wlinitcmds {wl phy_crs_war 1; wl mimo_bw_cap 1} \
    -nocal 0 -slowassoc 5 -noframeburst 1 -udp 150m -nobighammer 1 \
    -yart {-attn5g 23-63 -attn2g 33-63 -pad 30} -usecsa 0 \
    -msgactions {
	{_ai_core_reset: WARN3} {
	    $self worry $msg;
	    $self power cycle;
	    return 1
	}
    }

# CSA causing low rates and asserts

4334tx configure -ipaddr 192.168.1.98

#######################################

UTF::Linux UTFTestN -sta {43142 enp3s0} \
    -console {UTFTestU:40004} \
    -slowassoc 5 -reloadoncrash 0 \
    -power {UTFPower3 1} \
    -udp 120m -nobighammer 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl bw_cap 2g -1} \
    -yart {-attn 28-63 -pad 36 -frameburst 1}

43142 configure -ipaddr 192.168.1.95
43142 clone 43142b -tag BISON_BRANCH_7_10
43142 clone 43142b35 -tag BISON05T_BRANCH_7_35

###

UTF::Power::CanaKit ck -lan_ip /dev/ttyACM0 -relay lan

UTF::DHD UTFTestNd -lan_ip UTFTestN -sta {43143b0 eth0} \
    -power {UTFPower3 1} -power_sta {ck 1} \
    -console "UTFTestU:40003" -hostconsole UTFTestU:40004 \
    -tag PHOENIX2_BRANCH_6_10 \
    -brand linux-internal-dongle-usb \
    -dhd_brand linux-internal-dongle-usb \
    -driver dhd-cdc-usb-comp-gpl \
    -nvram "bcm943143usbirdsw_p450.txt" \
    -type "43143b0-roml/usb-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-pno-aoe-vsdb-pclose-proptxstatus-assert.bin.trx" \
    -slowassoc 5 -noframeburst 1 -nocal 1 \
    -wlinitcmds {wl msglevel +assoc; wl down; wl mimo_bw_cap 1} \
    -udp 150m -yart {-attn 28-63 -pad 36}

43143b0 configure -ipaddr 192.168.1.95

43143b0 clone 43143b0phy \
    -brand "linux-mfgtest-dongle-usb" \
    -type "43143b0-roml/usb-g-mfgtest-seqcmds.bin.trx" \
    -noaes 1 -notkip 1 -nocal 0

43143b0 clone 43143b0b \
    -dhd_brand {} \
    -tag BISON_BRANCH_7_10 \
    -brand linux-internal-media -nobighammer 1 \
    -driver "debug-apdef-stadef-high-p2p-mchan-tdls-wowl-media" \
    -type "43143b0-bmac/g-assert-p2p-mchan-media.bin.trx"

43143b0b clone 43143b0t14 -tag AARDVARK01T_TWIG_6_37_14 \
    -driver "debug-apdef-stadef-high-p2p-mchan-tdls-media"

43143b0b clone 43143b0b35 -tag BISON05T_BRANCH_7_35

####
UTF::Q ramsey3
