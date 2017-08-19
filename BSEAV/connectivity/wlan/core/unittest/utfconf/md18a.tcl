
source "utfconf/md18.tcl"
#=============================nvram vars=============================================================
#ATLAS router board is different from 4709 reference router board. In order to load atlas image on 4709
#the following nvram settings are needed to enable gmac3 on 4709 boards.
#
set nvgmac3 {
    "fwd_cpumap=d:u:5:163:0 d:x:2:169:1 d:l:5:169:1"
    gmac3_enable=1
}

set nvg {
    wl_msglevel=0x101
    fw_disable=1
    watchdog=6000
}

set nvr1 {
    lan_ipaddr=192.168.1.11
    lan_gateway=192.168.1.11
    dhcp_start=192.168.1.121
    dhcp_end=192.168.1.140
    wl0_ssid=63138/4366c0
}
set UTF::StaNightlyCustom {
    if {$(ap2) ne ""} {
	catch {$STA wl -i wl1.2 interface_remove}
	#package require UTF::Test::MultiSTANightly
	#MultiSTANightly -ap1 $(ap2) -ap2 $Router -sta $STA \
	    -nosetuptestbed -nostaload -nostareload -nosetup \
	    -noapload -norestore -nounload
	package require UTF::Test::APSTA
	APSTA $Router $(ap2) $STA -chan2 11 -core2 1
	$STA reload
    }
}

#================================ATLASII=================
#################################
# BISON04T_BRANCH_7_14 internal
#################################
md18ap1 clone atlasII/r1/0 -sta {atlasII/r1/0 eth1 atlasII/r1/0.%15 wl0.%} \
    -perfchans {3l}
md18ap1 clone atlasII/r1/1 -sta {atlasII/r1/1 eth2 atlasII/r1/1.%15 wl1.%} \
    -perfchans {36/80} -channelsweep {-max 64}
md18ap1 clone atlasII/r1/2 -sta {atlasII/r1/2 eth3 atlasII/r1/2.%15 wl2.%} \
    -perfchans {161/80} -channelsweep {-min 100}

#################################
# BISON04T_TWIG_7_14_164 internal
#################################
atlasII/r1/0 clone atlasII/r1b164/0 -tag BISON04T_TWIG_7_14_164
atlasII/r1/1 clone atlasII/r1b164/1 -tag BISON04T_TWIG_7_14_164
atlasII/r1/2 clone atlasII/r1b164/2 -tag BISON04T_TWIG_7_14_164

#################################
# BISON04T_TWIG_7_14_131 internal
#################################
atlasII/r1/0 clone atlasII/r1b131/0 -tag BISON04T_TWIG_7_14_131
atlasII/r1/1 clone atlasII/r1b131/1 -tag BISON04T_TWIG_7_14_131
atlasII/r1/2 clone atlasII/r1b131/2 -tag BISON04T_TWIG_7_14_131

set comment {
################################
# BISON04T_BRANCH_7_14 external
################################

atlasII/r1/0 clone atlasII/r1x/0 {*}$xargs
atlasII/r1/1 clone atlasII/r1x/1 {*}$xargs
atlasII/r1/2 clone atlasII/r1x/2 {*}$xargs

atlasII/r1x/2 configure \
    -dualband {atlasII/r1x/1 -c2 36/80 -c1 161/80 -lan1 192int11 -lan2 192int12}
}

set comment {
#################################
# BISON04T_TWIG_7_14_131 external
#################################
atlasII/r1b131/0 clone atlasII/r1b131x/0 {*}$xargs
atlasII/r1b131/1 clone atlasII/r1b131x/1 {*}$xargs
atlasII/r1b131/2 clone atlasII/r1b131x/2 {*}$xargs

atlasII/r1b131x/0 configure \
    -dualband {atlasII/r1b131x/2 -c1 36/80 -c2 161/80 -lan1 192int11 -lan2 192int12}

#================================ATLASII=================

#----------------------------------------------------
#BISON04T_BRANCH_7_14 DHD internal
#----------------------------------------------------
md18ap1 clone 4366r1-714i -tag BISON04T_BRANCH_7_14
md18ap1 clone 4366r1-714x -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src


#----------------------------------------------------
#BISON04T_TWIT_7_14_131 DHD internal
#----------------------------------------------------
md18ap1 clone 4366r1-131i


#----------------------------------------------------
# Mu-MIMO
# SU/MU dynamic switching
# "wl mu_features 0" -- SU
# "wl mu_features 1" -- MU (AUTO, default)
# STA: EAGLE_BRANCH_10_10
# AP: BISON04T_BRANCH_7_14_131

#Router
md18ap1 clone 4366r1-131i-mutx0 -rvrnightly {-mumode su}
md18ap1 clone 4366r1-131i-mutx1 -rvrnightly {-mumode mu}

md18ap1 clone 4366r1-714i-mutx0 -tag BISON04T_BRANCH_7_14  -rvrnightly {-mumode su}
md18ap1 clone 4366r1-714i-mutx1 -tag BISON04T_BRANCH_7_14 -rvrnightly {-mumode mu}
}
##############
#4366 MU STAs#
##############
set num {a b c d}
set i 0
foreach STA {4366st1 4366st2 4366st3 4366st4} {
    set suffix [lindex $num $i]
    $STA clone 4366${suffix}1x1 -tag  EAGLE_BRANCH_10_10 \
        -wlinitcmds {wl msglevel +assoc; wl msglevel +error;  wl dtim 3; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

    $STA clone 4366${suffix}1x1su -tag  EAGLE_BRANCH_10_10 \
        -wlinitcmds {wl msglevel +assoc; wl msglevel +error;  wl dtim 3; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

    $STA clone 4366${suffix}2x2 -tag  EAGLE_BRANCH_10_10 \
        -wlinitcmds {wl msglevel +assoc; wl msglevel +error;  wl dtim 3; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

    $STA clone 4366${suffix}2x2su -tag  EAGLE_BRANCH_10_10 \
        -wlinitcmds {wl msglevel +assoc; wl msglevel +error;  wl dtim 3; wl country '#a/0'; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
    incr i
}

#Packet Loss Test
4366st2 clone 4366lega1x1su -tag  EAGLE_BRANCH_10_10 \
    -wlinitcmds {wl msglevel +assoc; wl msglevel +error;  wl country '#a/0'; wl down; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1; wl nmode 0; wl vhtmode 0}
4366st2 clone 4366lega2x2su -tag  EAGLE_BRANCH_10_10 \
    -wlinitcmds {wl msglevel +assoc; wl msglevel +error;  wl country '#a/0'; wl down; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3; wl nmode 0; wl vhtmode 0}

#private image for debugging
atlasII/r1/1 clone borg-debug -image /projects/hnd_sig_ext20/zhuj/imgs/linux-core-dump-dbg.trx

