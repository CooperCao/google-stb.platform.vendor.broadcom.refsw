# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: f82b914800ca1759534f498823ddaaf48672b385 $
#

######
#
# UTF Ramsey6 in SW Lab 5010
#

# Shared resources
source utfconf/ramsey_shared.tcl

# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_software/utf/ramsey6"

R6 configure -default 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    R6 attn default
}

#set UTF::PerfCacheMigrate {
#    4360 4352
#}

#####################
# 1st small Ramsey box

UTF::Linux UTFTestBB -sta {4352 enp1s0} \
    -power {npc1 1} \
    -console "UTFTestC:40003" -reloadoncrash 1 \
    -wlinitcmds {
	wl msglevel +assoc;
	wl dtim 3;
	wl down;
	wl country '#a/0';
	wl bw_cap 2g -1;
	wl vht_features 3;
    }

4352 configure -ipaddr 192.168.1.50 -attngrp R6 -hasdhcpd 1 -date 2017.1.4.0

#####################
# 2nd small Ramsey box

UTF::DHD dhcp7-sv1-102 -sta {4350c5 eth0} \
    -app_tag BIS747T144RC2_BRANCH_7_64 \
    -app_brand linux-external-dongle-pcie \
    -tag BIS747T144RC2_BRANCH_7_64 \
    -dhd_tag DHD_BRANCH_1_579 \
    -power {npc1 2} \
    -hostconsole "UTFTestC:40005" \
    -dhd_brand linux-external-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "src/shared/nvram/bcm94350WhitezinBUSIKK.txt" \
    -type "4350c5-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-swdiv/rtecdc.bin" \
    -clm_blob "4350_whitezin_b.clm_blob" \
    -txcb "4350_whitezin_b.txcap_blob" \
    -customer olympic \
    -wlinitcmds {wl ampdu_mpdu 24;wl ampdu_release 12;wl vht_features 3} \
    -slowassoc 5 -docpu 1 -escan 1 -nobighammer 1 \
    -datarate {-i 0.5} \
    -tcpwindow 2m -udp 800m -nocal 1 \
    -yart {-attn5g 22-95 -attn2g 37-95 -pad 23}

4350c5 clone 4350c5x \
    -nvram "src/shared/nvram/bcm94350WhitezinBUSIKK.txt" \
    -type "../C-4350__s-C5/whitezinb.trx" \
    -clm_blob "whitezinb.clmb" \
    -txcb "whitezinb.txcb" \
    -perfonly 1 -perfchans {36/80}

#-txcal "/projects/hnd_dvt_bate/cache/P-whitezinb_M-STEL_V-u__m-3.11.msf" 

4350c5 clone 4350c5.1 -sta {_4350c5 eth0 4350c5.1 wl0.1}\
    -apmode 1 -nochannels 1 -slowassoc 5 \
    -wlinitcmds {wl ampdu_mpdu 24;wl ampdu_release 12;wl vht_features 3;wl apsta 1; wl ssid -C 1 4350AP} \
    -noaes 1 -notkip 1 -yart {} -nocustom 1

4350c5.1 configure -ipaddr 192.168.1.88

4350c5 clone 4350c5100 -tag BIS735T255RC1_TWIG_7_47_100
4350c5x clone 4350c5100 -tag BIS735T255RC1_TWIG_7_47_100

4350c5 clone 4350c5b54 -tag BIS747T99RC11_BRANCH_7_54
4350c5x clone 4350c5b54x -tag BIS747T99RC11_BRANCH_7_54

########################

UTF::DHD UTFTestW -sta {4350c2 eth0} \
    -app_tag BIS759T5RC2_BRANCH_7_63 \
    -app_brand linux-external-dongle-pcie \
    -tag BIS759T5RC2_BRANCH_7_63 \
    -dhd_tag DHD_BRANCH_1_579 \
    -power {UTFPower6 5} \
    -hostconsole "UTFTestF:40005" \
    -dhd_brand linux-external-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94350RieslingBMurataMT.txt" \
    -clm_blob 4350_riesling_b.clm_blob \
    -type 4350c2-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-swdiv-ecounters-bssinfo/rtecdc.bin \
    -customer olympic \
    -wlinitcmds {wl ampdu_mpdu 24;wl ampdu_release 12;wl vht_features 3} \
    -slowassoc 5 -docpu 1 -escan 1 -nobighammer 1 \
    -datarate {-i 0.5} \
    -tcpwindow 2m -udp 800m -nocal 1 \
    -yart {-attn5g 22-95 -attn2g 37-95 -pad 23}

4350c2 configure -ipaddr 192.168.1.98

4350c2 clone 4350c2x \
    -type ../C-4350__s-C2/rieslingb.trx -clm_blob rieslingb.clmb \
    -perfonly 0

##

4350c2 clone 4350c2b61 \
    -tag BIS759T2RC1_BRANCH_7_61 \
    -app_tag BIS759T2RC1_BRANCH_7_61
4350c2x clone 4350c2b61x \
    -tag BIS759T2RC1_BRANCH_7_61 \
    -app_tag BIS759T2RC1_BRANCH_7_61


##################

set UTF::StaNightlyCustom {
    package require UTF::Test::suspendresume
    suspendresume $Router $STA -notraffic
}

####
UTF::Q ramsey6 UTFTestC
