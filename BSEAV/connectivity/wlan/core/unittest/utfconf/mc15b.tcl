# -*-tcl-*-

# $Copyright Broadcom Corporation$
# $Id$

# Common config
source utfconf/mc15.tcl

set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc15b"

B configure -default 16

set ::UTF::SetupTestBed {
    B attn default
}

set ::UTF::DSLDtimDebug 1
set ::UTF::usrutf /usr/UTF

UTF::Linux sr1end22 -sta {lan2 eth1} -onall false
lan2 configure -ipaddr 192.168.1.51
UTF::Linux sr1end04 -sta {wan eth1} -onall false

# STAs
# CSA doesn't work if STA is on #a/0 but AP is not
# wl country '#a/0'

UTF::Linux mc15tst3 -sta {sta1 enp1s0} \
    -tag "EAGLE_BRANCH_10_10" \
    -power {mc15ips1 1} \
    -console {sr1end11:40000} \
    -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 7} \
    -tcpwindow 4m -modopts {ctdma=1}

UTF::Linux mc15tst4 -sta {sta2 enp1s0} \
    -tag "EAGLE_BRANCH_10_10" \
    -power {mc15ips1 2} \
    -console {sr1end11:40001} \
    -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 7} \
    -tcpwindow 4m -modopts {ctdma=1}

##################

#- wanpeer wan

#set UTF::PerfCacheMigrate {
#    63138.4366 63138b25.4366
#    63138.4366 63138b131.4366
#    63138b131.4366 63138b.4366
#    63138.4366 63138N.4366
#}

package require UTF::DSL
UTF::DSL 63138 -sta {63138/4366 wl0 63138/4366.%3 wl0.%} \
    -console sr1end11:40002 \
    -power {mc15ips1 3} \
    -relay lan \
    -lanpeer {lan lan2}\
    -use11h yes \
    -datarate {-i 0.5} -udp 1.8g \
    -yart {-attn5g 18-93 -attn2g 32-93 -pad 23 -leakcheck 0} \
    -noradio_pwrsave 1 -perfchans {36/80 3} \
    -nvram {
	wl0_ssid=63138/4366
	wl0_obss_coex=0
	wl0_vht_features=7
	wl0_dtim=3
    } \
    -tag WLAN_L04_Misc \
    -type bcm963138GW_PURE181_nand_fs_image_128_ubi.w \
    -wlinitcmds {
	pwr config --cpuspeed 1;
	dhd -i wl0 dconpoll 250;
    }

63138/4366 configure -attngrp B

63138/4366 clone 63138L05/4366 -tag WLAN_L05_Misc

63138/4366 clone 63138N/4366 \
    -image /projects/hnd_sw_routerdev_ext2/work/nisar/dev4/depot/CommEngine/devel/targets/963138GW/bcm963138GW_nand_cferom_fs_image_128_ubi.w

# Alternate test target using new FW overlayed on pre-installed OS.
63138/4366 clone 63138b25/4366 \
    -fwoverlay 1 \
    -tag BISON04T_{REL,TWIG}_7_14_131_25{,??} \
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
    -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-hostpmac

63138b25/4366 clone 63138b131/4366 -tag BISON04T_TWIG_7_14_131

#    -pre_perf_hook {{UTF::Every 1 63138b131/4366 wl bus:dumptxrings}} \
#    -post_perf_hook {{UTF::Every cancel all}}

63138b25/4366 clone 63138b/4366 -tag BISON04T_BRANCH_7_14 \
    -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-stamon-hostpmac-murx-splitassoc

63138b25/4366 clone 63138b164/4366 -tag BISON04T_TWIG_7_14_164 \
    -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-stamon-hostpmac-murx-splitassoc-dyn160

set UTF::RouterNightlyCustom {
#    UTF::Try "$Router: Vendor IE" {
#	package require UTF::Test::vndr_ie
#	UTF::Test::vndr_ie $Router $STA1
#    }

    package require UTF::Test::MiniUplinks
    UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 -nowet -nopsta

    # DWDS Repeater not required for 63138
    #package require UTF::Test::Repeaters
    #UTF::Test::Repeaters $Router $STA1 -sta $STA2 -nowet -nopsta

}

#####
UTF::Q mc15b

