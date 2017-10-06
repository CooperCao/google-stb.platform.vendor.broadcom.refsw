# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$

source /projects/hnd_sig_ext20/zhuj/utfws/unittest/utfconf/iptv2.tcl

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
#          wl0_bsscfg_class=0 wl bsscfg configured as data service
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
    wl0.x_bsscfg_class=1
    wl0_scb_alloc_class=1
}

set nvgeneral {
    fw_disable=0
    wl_msglevel=0x101
    wl0_vht_features=7
}
#for mc5 cards, set vht_features=6 (so it won't enable 2G which is not supported on mc5 cards)
#=============================end of nvram vars========================================================

#======================================================================================================


#image with ATF feature (not in BISON04T_BRANCH_7_14)
#4709ap1 clone 4709dhdap1 -tag BISON04T_BRANCH_7_14 -image  /projects/hnd_swbuild/USERS/tigran/build_linux/BISON04T_BRANCH_7_14/linux-2.6.36-arm-internal-router-dhdap/2015.7.17.0/build/image/linux.trx

#---------------------------------------------------
#EAGLE_BRANCH_10_10 NIC
#---------------------------------------------------
#metascripts:

#UTFD::metascript %AUTO% -script "/projects/hnd_sig_ext20/zhuj/utfws/unittest/Test/RangeDiscovery.test -utfconf iptvtb2 -ap 4709ap1 -sta 4366sta2 -fixedstep -key EAGLE:NIC:BeaconRvRNightly -setup -web" -type triggered -watch 4709ap1 -watchinterval 900 -preemptable 1

#UTFD::metascript %AUTO% -script "/projects/hnd_sig_ext20/zhuj/utfws/unittest/Test/RangeDiscovery.test -utfconf iptvtb2 -ap 4709ap1 -sta 4366sta2 -key EAGLE:NIC:BeaconRangeNightly -setup -web" -type triggered -watch 4709ap1 -watchinterval 900 -preemptable 1

#UTFD::metascript %AUTO% -script "/projects/hnd_sig_ext20/zhuj/utfws/unittest/Test/ProximateBSS.test -utfconf iptvtb2 -ap 4709ap1 -proximateap 4709proxap -sta 4366sta2 -proximatesta 4709psta2 -5g_rate auto -towards -title EAGLE_BRANCH_10_10:4709:4366:NIC -key EAGLE_BRANCH_10_10:4709:4366:NIC -email hnd-utf-list" -type triggered -watch 4709ap1 -watchinterval 900 -preemptable 1

#AP NIC driver
# loading NIC driver crashing the router 7/1/2016??
#4709ap1 clone 4366r1/nic -tag EAGLE_BRANCH_10_10

#---------------------------------------------------
#BISON04T_BRANCH_7_14 DHD internal (17.1)
#---------------------------------------------------
4709ap1 clone 4366r1/714 -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas
4709proxap clone 4366r2/714 -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas
4709psta2 clone 4366r3/714 -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas

foreach r {4709ap1 4709proxap 4709psta2} {
    $r destroy
}

#----------------------------------------------------
#BISON04T_BRANCH04T_BRANCH_7_14 DHD external
#----------------------------------------------------
4366r1/714 clone 4366r1/714x -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
4366r2/714 clone 4366r2/714x -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
4366r3/714 clone 4366r3/714x -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
#------------------------------------------------------------
#BISON04T_BRANCH04T_TWIG_7_14_164 DHD internal (Golden3.x)
#------------------------------------------------------------
4366r1/714 clone 4366r1/164 -tag BISON04T_TWIG_7_14_164
4366r2/714 clone 4366r2/164 -tag BISON04T_TWIG_7_14_164
4366r3/714 clone 4366r3/164 -tag BISON04T_TWIG_7_14_164
#----------------------------------------------------------
#BISON04T_BRANCH04T_TWIG_7_14_164 DHD external (Golden3.x)
#----------------------------------------------------------
4366r1/164 clone 4366r1/164x -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
4366r2/164 clone 4366r2/164x -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
4366r3/164 clone 4366r3/164x -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src

#----------------------------------------------------
#BISON04T_TWIT_7_14_131 DHD internal (Golden2.x)
#----------------------------------------------------
4366r1/714 clone 4366r1/131 -tag BISON04T_TWIG_7_14_131
4366r2/714 clone 4366r2/131 -tag BISON04T_TWIG_7_14_131
4366r3/714 clone 4366r3/131 -tag BISON04T_TWIG_7_14_131

#----------------------------------------------------
#BISON04T_TWIT_7_14_131 DHD external (Golden2.x)
#----------------------------------------------------
4366r1/131 clone 4366r1/131x -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
4366r2/131 clone 4366r2/131x -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
4366r3/131 clone 4366r3/131x -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src

#BISON04T_TWIG_7_14_131 (mumimo nad needs testing on both ATLASI and ATLASII)

#private images

#4366r1/131 clone r1c0-jdbug34 -image "/projects/hnd_sw_routerdev_ext2/work/jihuac/test/vi-be/linux-131-0419-clean.trx"
#4366r2/131 clone r2b1-jdbug34 -image "/projects/hnd_sw_routerdev_ext2/work/jihuac/test/vi-be/linux-131-0419-clean.trx"

#4366r1/131 clone 4366r1/131/53 -tag BISON04T_REL_7_14_131_53

#-------------------------------------------------------
# packet loss testing
# legacy sta
4366sta1 clone 4366lega1 -tag EAGLE_BRANCH_10_10 -wlinitcmds {
    wl msglevel +assoc;
    wl msglevel +error;
    wl down;
    wl country US/0;
    wl vhtmode 0;
    wl nmode 0;
    wl rxchain 3; 
    wl txchain 3
}


