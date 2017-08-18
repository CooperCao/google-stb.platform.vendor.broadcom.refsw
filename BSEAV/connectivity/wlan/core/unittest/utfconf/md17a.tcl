
source "utfconf/md17.tcl"
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
    wl0_ssid=4366c0
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
#----------------------------------------------------
#trunk
#----------------------------------------------------
4366st2 clone 4366st2/ti -tag trunk
4366st3 clone 4366st3/ti -tag trunk

#----------------------------------------------------
#BISON04T_BRANCH_7_14 DHD internal
#----------------------------------------------------
md17ap1 clone 4366r1-714i -tag BISON04T_BRANCH_7_14
md17ap1 clone 4366r1-714x -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
#----------------------------------------------------
#BISON04T_TWIT_7_14_131 DHD internal
#----------------------------------------------------
md17ap1 clone 4366r1-131i


#4357a.2 clone 4357i.2 -tag IGUANA_BRANCH_13_10
#4357a clone 4357iddst1 -tag IGUANA_BRANCH_13_10
#4357iddst1 clone 4357itdst1 \
#    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
#    -perfonly 1 -perfchans {36/80} -notkip 1 -noaes 1


#----------------------------------------------------
# Mu-MIMO
# SU/MU dynamic switching
# "wl mu_features 0" -- SU
# "wl mu_features 1" -- MU (AUTO, default)
# STA: EAGLE_BRANCH_10_10
# AP: BISON04T_BRANCH_7_14_131

#Router
md17ap1 clone 4366r1-131i-mutx0 -rvrnightly {-mumode su}
md17ap1 clone 4366r1-131i-mutx1 -rvrnightly {-mumode mu}

md17ap1 clone 4366r1-714i-mutx0 -tag BISON04T_BRANCH_7_14  -rvrnightly {-mumode su}
md17ap1 clone 4366r1-714i-mutx1 -tag BISON04T_BRANCH_7_14 -rvrnightly {-mumode mu}

md17ap1 clone 4366r1-164i-mutx0 -tag BISON04T_TWIG_7_14_164  -rvrnightly {-mumode su}
md17ap1 clone 4366r1-164i-mutx1 -tag BISON04T_TWIG_7_14_164 -rvrnightly {-mumode mu}

#NIC SoftAP
md17ap2 clone 4366sap-nic-mutx0
4366sap-nic-mutx0 configure -ipaddr 192.168.1.12 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode su}
md17ap2 clone 4366sap-nic-mutx1
4366sap-nic-mutx1  configure -ipaddr 192.168.1.12 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu}

#DHD SoftAP
4366sapfd clone 4366sap-fd-mutx0
4366sap-fd-mutx0 configure -ipaddr 192.168.1.12 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode su}
#this is the option to modify the nvram file
#ctdma is off by default for SU now, so no need to set here
#4366sap-fd-mutx0 confiture -nvram_add ctdma=0

4366sapfd clone 4366sap-fd-mutx1
4366sap-fd-mutx1 configure -ipaddr 192.168.1.12 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu}
#ctdma is on by default for MU now, so no need to set here
#4366sap-fd-mutx1 -nvram_add ctdma=1

#################
## 4357 MU STAs #
#################
# replaced 4357 with 4366 on 10/14/2016
#4357a clone 4357a2x2 -type 4357a0-ram/config_pcie_tput_mu/rtecdc.bin -clm_blob 4347a0.clm_blob \
#    -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0;}
#4357b clone 4357b2x2 -type 4357a0-ram/config_pcie_tput_mu/rtecdc.bin -clm_blob 4347a0.clm_blob \
#    -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0;wl rxchain 3; wl txchain 3;}
##4357a1 clone 4357a2x2 -type 4357a1-ram/config_pcie_tput_mu/rtecdc.bin
##4357b1 clone 4357b2x2 -type 4357a1-ram/config_pcie_tput_mu/rtecdc.bin

##############
#4366 MU STAs#
##############
set num {a b c d e f g h}
set i 0
foreach STA {4366st2 4366st3 4366st4 4366st5 4366st6 4366st7 4366st8 4366st9} {
    set suffix [lindex $num $i]
    $STA clone 4366${suffix}1x1 -tag  EAGLE_BRANCH_10_10 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

    $STA clone 4366${suffix}1x1su -tag  EAGLE_BRANCH_10_10 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

    $STA clone 4366${suffix}2x2 -tag  EAGLE_BRANCH_10_10 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

    $STA clone 4366${suffix}2x2su -tag  EAGLE_BRANCH_10_10 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
    incr i
}

#Packet Loss Test
4366st2 clone 4366lega1x1su -tag  EAGLE_BRANCH_10_10 \
    -wlinitcmds {wl msglevel +assoc; wl country US/0; wl down; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1; wl nmode 0; wl vhtmode 0}
4366st2 clone 4366lega2x2su -tag  EAGLE_BRANCH_10_10 \
    -wlinitcmds {wl msglevel +assoc; wl country US/0; wl down; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3; wl nmode 0; wl vhtmode 0}


