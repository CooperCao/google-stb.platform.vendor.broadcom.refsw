# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$

source /projects/hnd_sig_ext20/zhuj/utfws/unittest/utfconf/iptv1.tcl
#=============================nvram vars=============================================================
#ATLAS router board is different from 4709 reference router board. In order to load atlas image on 4709
#the following nvram settings are needed to enable gmac3 on 4709 boards.
#
set nvgmac3 {
    "fwd_cpumap=d:u:5:163:0 d:x:2:169:1 d:l:5:169:1"
    gmac3_enable=1
}

#SCB test needs following nvram settings:
#http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/PcieFullDongleScbExpand#4-1_Architecture_overview
#-nvram {
#    wl0_scb_alloc=0|1   <--- global nvram var to disable/enable scb allocation in host memory for dongle routers
#    wlx_bsscfg_class=0|1|2 <--- nvram var per wlan interface. 0x0 data service, 0x1 public service, 0x2 video service.
#        SCB associating to public bsscfg are called public SCBs, associating to data or video are called private SCB
#        e.g.
#          wl0.1_bsscfg_class=1 <--- wl0.1 bsscfg configured as public service
#          wl0_bsscfg_class=0 wl0 bsscfg configured as data service
#    wl0_scb_alloc_class=0|1|2|3 <--- global nvram var
#        0x0: scb only allocated from dongle memory
#        0x1: public scbs are allocated from host memory, private scbs are allocated from dongle memory
#        0x2: either allocate scb from dongle upto max limit #, 
#             or allocate scb from dongle when available memory greater than minimum threshold
#        0x3: phase-2
#currently not supported on 4366, only on 43602 dongle router
#}
set nvscbguest {
    wl0_scb_alloc=1
    wl0_bsscfg_class=0
    wl0.1_bsscfg_class=1
    wl0_scb_alloc_class=1
}

set nvnetwork {
    lan_ipaddr=192.168.1.210
    lan_gateway=192.168.1.210
    dhcp_start=192.168.1.215
    dhcp_end=192.168.1.219
}

# Allow vht_features to use defaults - 2g will be
# using prop11n instead of VHT
# for using VHT with 2g, need to set wl0_vht_features=7

set nvgeneral {
    fw_disable=0
    wl_msglevel=0x101
}

#2x2 radio
set nv2x2 {
    wl0_txchain=3
    wl0_rxchain=3
}

#=============================end of nvram vars========================================================

#----------------------------------------------------
#BISON04T_BRANCH_7_14 DHD external (17.1)
#----------------------------------------------------
4709ap clone 43602r/714 -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -datarate {-i 0.5 -frameburst 1} \
    -perfchans {36/80 36l 3} \
    -nvram "$nvgmac3 $nvgeneral $nvnetwork"

#enable scb on 43602 dhd
#4709ap clone 4709bdhdxscb -tag BISON04T_BRANCH_7_14 \
#    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
#    -nvram "$nvgmac3 $nvgeneral $nvnetwork $nvscbguest"

#----------------------------------------------------
#BISON04T_BRANCH_7_14 DHD external (17.1)
#----------------------------------------------------
43602r/714 clone 43602r/714x \
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
    -nocustom 1 \
    -perfchans {36/80}
#    -noaes 1 -docpu 1 \
#    -datarate {-i 0.5 -frameburst 1} -nocustom 1

#----------------------------------------------------
#BISON04T_TWIG_7_14_164 DHD internal (Gold3.x)
#----------------------------------------------------
43602r/714 clone 43602r/164 -tag BISON04T_TWIG_7_14_164
#----------------------------------------------------
#BISON04T_TWIG_7_14_164 DHD external (Gold3.x)
#----------------------------------------------------
43602r/714x clone 43602r/164x -tag BISON04T_TWIG_7_14_164
#----------------------------------------------------
#BISON04T_TWIG_7_14_131 DHD External (Gold2.x)
#----------------------------------------------------
#Nightly DHD external
43602r/714x clone 43602r/131x -tag BISON04T_TWIG_7_14_131
#this is to run with 2x2 43602 nocache so that we could compare the result with 
#131 nightly
#43602r/714x clone 43602r/131x -tag BISON04T_REL_7_14_131_59 \
#    -nvram "$nvgmac3 $nvgeneral $nvnetwork $nv2x2 "
#----------------------------------------------------
#BISON04T_REL_7_14_164_7 DHD External (Gold3.1)
#----------------------------------------------------
43602r/164x clone 43602r/164x/7 -tag BISON04T_REL_7_14_164_7
43602r/164x/7 clone 43602r/164x/7-off -nvram "$nvgmac3 $nvgeneral $nvnetwork wl0_amsdu=off"
#----------------------------------------------------
#BISON04T_TWIG_7_14_131 DHD Internal (Gold2.x)
#----------------------------------------------------
#Nightly DHD internal
43602r/714 clone 43602r/131 -tag BISON04T_TWIG_7_14_131

#this is to run with 2x2 43602 nocache so that we could compare the result with 
#131 nightly
# 43602r/714 clone 43602r/131 -tag BISON04T_REL_7_14_131_59 \
#    -nvram "$nvgmac3 $nvgeneral $nvnetwork $nv2x2" 

#10/17/2016 7.14.131.59 2x2 radio

43602r/131 clone 43602r/131/59 -tag BISON04T_REL_7_14_131_59 \
    -nvram "$nv2x2 $nvgmac3 $nvgeneral $nvnetwork"
43602r/131 clone 43602r/131/59x -tag BISON04T_REL_7_14_131_59 \
    -nvram "$nv2x2 $nvgmac3 $nvgeneral $nvnetwork"

#----------------------------------------------------
#NIC 714 and 131 TOB
#----------------------------------------------------
43602r/714 clone 43602r/714nic -brand linux-2.6.36-arm-internal-router \
    -nvram "$nvgeneral $nvnetwork"
43602r/714nic clone 43602r/131nic -tag BISON04T_TWIG_7_14_131

#================================================
#STA Instances
#================================================
4360st1 clone 4360esta1 -tag EAGLE_BRANCH_10_10
4360st2 clone 4360esta2 -tag  EAGLE_BRANCH_10_10
43602 clone 43602esta3 -tag  EAGLE_BRANCH_10_10
set comments {
this driver has some kernel panic and psm watchdog issues
set stacommon {
    -date 2015.7.10.0
    -type debug-apdef-stadef-p2p-mchan-tdls
    -wlinitcmds {
	wl msglevel +assoc;
	wl down;
	wl country '#a/0';
	wl bw_cap 2g -1;
	wl vht_features 3;
    }
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1
}
4360esta1 clone st1 -tag trunk {*}$stacommon
4360esta2 clone st2 -tag trunk {*}$stacommon
43602esta3 clone st3 -tag trunk {*}$stacommon
}
#------------------------------------------
set stacommon {
    -wlinitcmds {
	wl msglevel +assoc;
	wl down;
	wl country '#a/0';
	wl bw_cap 2g -1;
	wl vht_features 3;
    }
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1
}
4360esta1 clone st1 -tag BISON05T_BRANCH_7_35
4360esta2 clone st2 -tag BISON05T_BRANCH_7_35
43602esta3 clone st3 -tag BISON05T_BRANCH_7_35
#----------------------------------------------------
#11b STA EAGLE_BRANCH_10_10 TOB (11n and 11ac disabled with nmode and vhtmode)
#----------------------------------------------------
4360esta1 clone 4360lega1 -wlinitcmds {
    wl msglevel +assoc; wl msglevel +error; wl down; wl nmode 0; wl vhtmode 0;
}








