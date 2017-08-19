# -*-tcl-*-
#
# Testbed MD16 sub-rig configuration file (using 2 attngrp)
# Filename: md16a.tcl
# Charles Chai 12/30/2015
# Modified: Upgrade to 8-STAs, 08/16/2016
#
# Hardware
#   MD16END1  : FC19 controller
#   Router    : 4709C0/4366mc_p143
#   MD16TST7  : FC19 softap
#   MD16TST1  : FC19 STA (attngrp G1)
#   MD16TST2  : FC19 STA (attngrp G1)
#   MD16TST3  : FC19 STA (attngrp G1)
#   MD16TST4  : FC19 STA (attngrp G1)
#   MD16TST5  : FC19 STA (attngrp G1)
#   MD16TST6  : FC19 STA (attngrp G1)
#   MD16TST14 : FC19 STA (attngrp G1)
#   MD16TST15 : FC19 STA (attngrp G1)
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 8331 (0-95.5dB with 0.5 steps)
#
source "utfconf/md16.tcl"

package require UTF::Linux
package require UTF::utils

#set ::gnuplot_rvr_xtics 4

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/md16a"

# To use wl from trunk (set default); Use -app_tag to modify.
#set UTF::TrunkApps 1 ;# do not need it anymore as Tim set it as UTF default

# To enable ChannelSweep performance test
set UTF::ChannelPerf 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G1 attn 0

    # Make sure all devices are deinit (for REF) and down (for DUT).
    #foreach S {4366softap-mu 4709rtr-mu 4366a1x1 4366b1x1 4366c1x1 4366d1x1 4366e1x1 4366f1x1 4366g1x1 4366h1x1}
    foreach S {4366softap-mu 4366a1x1 4366b1x1 4366c1x1 4366d1x1 4366e1x1 4366f1x1 4366g1x1 4366h1x1} {
        UTF::Try "$S Down" {
            catch {$S wl down}
            catch {$S deinit}
        }
    }
    # unset S so it doesn't interfere
    unset S

    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}

# For Chantest (permutation: 8x10 = 80)
set ::randdevicelistSTA1 [list sta1x1mu sta1x1su sta1x1vht sta1x1ht sta1x1lt sta1x2mu sta1x2su sta1x2vht sta1x2ht sta1x2lt]
set ::randdevicelistSTA2 [list sta2x1mu sta2x1su sta2x1vht sta2x1ht sta2x1lt sta2x2mu sta2x2su sta2x2vht sta2x2ht sta2x2lt]
set ::randdevicelistSTA3 [list sta3x1mu sta3x1su sta3x1vht sta3x1ht sta3x1lt sta3x2mu sta3x2su sta3x2vht sta3x2ht sta3x2lt]
set ::randdevicelistSTA4 [list sta4x1mu sta4x1su sta4x1vht sta4x1ht sta4x1lt sta4x2mu sta4x2su sta4x2vht sta4x2ht sta4x2lt]
set ::randdevicelistSTA5 [list sta5x1mu sta5x1su sta5x1vht sta5x1ht sta5x1lt sta5x2mu sta5x2su sta5x2vht sta5x2ht sta5x2lt]
set ::randdevicelistSTA6 [list sta6x1mu sta6x1su sta6x1vht sta6x1ht sta6x1lt sta6x2mu sta6x2su sta6x2vht sta6x2ht sta6x2lt]
set ::randdevicelistSTA7 [list sta7x1mu sta7x1su sta7x1vht sta7x1ht sta7x1lt sta7x2mu sta7x2su sta7x2vht sta7x2ht sta7x2lt]
set ::randdevicelistSTA8 [list sta8x1mu sta8x1su sta8x1vht sta8x1ht sta8x1lt sta8x2mu sta8x2su sta8x2vht sta8x2ht sta8x2lt]


###########################################
# AP Section
###########################################
#
# Attn Group: G1 (1 2 3 4) (a b c d) good channel
# Attn Group: G2 (5 6 7 8) (e f g h) good channel
#
###########################################
# Router (4709C0 board with 4366mc_p143 C0)
#############################################
UTF::Router 4709 \
    -sta {4709/4366 eth1 4709/4366.%15 wl0.%} \
    -relay lan \
    -lan_ip 192.168.1.1 \
    -lanpeer {lan lan1} \
    -wanpeer wan \
    -console "md16end1:40008" \
    -relay "md16end1" \
    -power {npc912 1} \
    -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-internal-router-dhdap \
    -embeddedimage {4366c} \
    -datarate {-i 0.5} -udp 1.8g \
    -noradio_pwrsave 1 \
    -nvram {
        watchdog=3000
        wl0_ssid=4709/4366
        wl0_chanspec=36
        wl0_radio=0
        wl0_bw_cap=-1
        wl0_country_code=US/0
        wl0_atf=0
        wl0_vht_features=7
        wl0_mu_features="-1"
        wl1_ssid=4709/43602g
        wl1_chanspec=3
        wl1_radio=0
        wl1_atf=0
        wl1_vht_features=5
        wl1_mu_features="-1"
    }

4709/4366 clone 4709rtr-su
4709/4366 clone 4709rtr-mu -rvrnightly {-mumode mu}

4709/4366 clone 4709rtr-ext-su \
	-brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-partial-src
4709/4366 clone 4709rtr-ext-mu \
	-brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-partial-src \
	-rvrnightly {-mumode mu}

4709/4366 clone 4709rtr-twig-su -tag BISON04T_TWIG_7_14_131
4709/4366 clone 4709rtr-twig-mu -tag BISON04T_TWIG_7_14_131 -rvrnightly {-mumode mu}

# Gold-3 release 
4709/4366 clone 4709rtr-rel-su -tag BISON04T_REL_7_14_164_16
4709/4366 clone 4709rtr-rel-mu -tag BISON04T_REL_7_14_164_16 -rvrnightly {-mumode mu}

# NIC driver
4709/4366 clone 4709rtr-nic-su -tag EAGLE_BRANCH_10_10 \
    	-brand linux-2.6.36-arm-internal-router
4709/4366 clone 4709rtr-nic-mu -tag EAGLE_BRANCH_10_10 \
    	-brand linux-2.6.36-arm-internal-router \
	-rvrnightly {-mumode mu}

# for testing
4709/4366 clone 4709rtr-t1-su -brand linux-2.6.36-arm-internal-router-dhdap-atlas
4709/4366 clone 4709rtr-t2-su -brand linux-2.6.36-arm-internal-router-dhdap-atlas

4709/4366 clone 4709rtr-atlas-su -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
	-tag BISON04T_REL_7_14_164_14
4709/4366 clone 4709rtr-atlas-mu -brand linux-2.6.36-arm-internal-router-dhdap-atlas -rvrnightly {-mumode mu} \
	-tag BISON04T_REL_7_14_164_14

# Chunyu test request  - 11/19/2016
4709/4366 clone 4709rtr-301fd-mu -rvrnightly {-mumode mu} \
	-tag BISON04T_REL_7_14_164_306 -brand linux-2.6.36-arm-internal-router

4709/4366 clone 4709rtr-301nic-mu -rvrnightly {-mumode mu} \
	-tag EAGLE_REL_10_10_122_308 -brand linux-2.6.36-arm-internal-router
4709/4366 clone 4709rtr-301nicm4-mu -rvrnightly {-mumode mu} \
	-tag EAGLE_REL_10_10_122_308 -brand linux-2.6.36-arm-internal-router \
	-nvram {
        watchdog=3000
        wl0_ssid=4709/4366
        wl0_chanspec=36
        wl0_radio=0
        wl0_bw_cap=-1
        wl0_country_code=US/0
        wl0_atf=0
        wl0_vht_features=7
        wl0_mu_features="-1"
        wl0_mu_policy="max_muclients 4"
        wl1_ssid=4709/43602g
        wl1_chanspec=3
        wl1_radio=0
        wl1_atf=0
        wl1_vht_features=5
        wl1_mu_features="-1"
    }

4709/4366 clone 4709rtr-31fd-mu -rvrnightly {-mumode mu} \
	-tag BISON04T_REL_7_14_164_11 -brand linux-2.6.36-arm-internal-router

4709/4366 clone 4709rtr-31nic-mu -rvrnightly {-mumode mu} \
	-tag EAGLE_REL_10_10_122_13 -brand linux-2.6.36-arm-internal-router


4709/4366 clone 4709rtr-atf-su \
    -tag BISON04T_BRANCH_7_14 \
    -nvram {
        watchdog=3000
        wl0_ssid=4709/4366
        wl0_chanspec=36
        wl0_radio=0
        wl0_bw_cap=-1
        wl0_country_code=US/0
        wl0_atf=1
        wl0_vht_features=7
        wl0_mu_features="-1"
        wl1_ssid=4709/43602g
        wl1_chanspec=3
        wl1_radio=0
        wl1_atf=1
        wl1_vht_features=5
        wl1_mu_features="-1"
    }
4709rtr-atf-su clone 4709rtr-atf-mu -rvrnightly {-mumode mu}


###########################################
# MD16a - softap
# 4366mc_p143 C0 (12/30/2015)
###########################################
UTF::Linux md16tst7(ap) \
        -lan_ip md16tst7 \
        -sta {4366ap enp1s0} \
        -console "md16end1:40007" \
        -power {npc913 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
	-msgactions {"PHYTX error" FAIL}

4366ap clone 4366softap-su
4366softap-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1
4366ap clone 4366softap-mu
4366softap-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu}

4366ap clone 4366softap-atf-su
4366softap-atf-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl atf 1;}
4366ap clone 4366softap-atf-mu
4366softap-atf-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl atf 1;}
4366ap clone 4366softap-atf1-su
4366softap-atf1-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl atf 1; wl ampdu_atf_us 10000;}

4366ap clone 4366softap-3x3-su
4366softap-3x3-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-ap3coremode 1}
4366ap clone 4366softap-3x3-mu
4366softap-3x3-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu -ap3coremode 1}

4366ap clone 4366softap-trunk-su
4366softap-trunk-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-tag trunk
4366ap clone 4366softap-trunk-mu
4366softap-trunk-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-tag trunk \
	-rvrnightly {-mumode mu}

4366ap clone 4366softap-cst-mu
4366softap-cst-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu}

4366ap clone 4366softap-msm-mu
4366softap-msm-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu}

4366ap clone 4366softap-mvm-mu
4366softap-mvm-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu}

4366ap clone 4366softap-mhm-mu
4366softap-mhm-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu}

4366ap clone 4366softap-mlm-mu
4366softap-mlm-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu}

4366ap clone 4366softap-mat-mu
4366softap-mat-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu}

# for testing
4366ap clone 4366softap-t1-mu
4366softap-t1-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-image /projects/hnd_software_ext5/work/ssreekan/EAGLE_BRANCH_10_10/src/wl/linux//wl.ko_r668326_rcca_prot_spwt100_edDis_inwd \
	-rvrnightly {-mumode mu"}

4366ap clone 4366softap-t2-mu
4366softap-t2-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-image /projects/hnd_software_ext5/work/ssreekan/EAGLE_BRANCH_10_10/src/wl/linux/wl.ko_r668326_rcca_prot_spwt100_edDis \
        -rvrnightly {-mumode mu}

4366ap clone 4366softap-t3-mu
4366softap-t3-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl atf 1;} \
        -rvrnightly {-mumode mu}

4366ap clone 4366softap-t4-mu
4366softap-t4-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl mu_policy -max_muclients 4;} \
        -rvrnightly {-mumode mu -apafteriperf "wl pmac shmx 0xa8 -n 10"}

4366ap clone 4366softap-t5-mu
4366softap-t5-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl mu_policy -max_muclients 5;} \
        -rvrnightly {-mumode mu -apafteriperf "wl pmac shmx 0xa8 -n 10"}

4366ap clone 4366softap-t6-mu
4366softap-t6-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl mu_policy -max_muclients 6;} \
        -rvrnightly {-mumode mu -apafteriperf "wl pmac shmx 0xa8 -n 10"}

4366ap clone 4366softap-t7-mu
4366softap-t7-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl mu_policy -max_muclients 7;} \
        -rvrnightly {-mumode mu -apafteriperf "wl pmac shmx 0xa8 -n 10"}

4366ap clone 4366softap-t8-mu
4366softap-t8-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl mu_policy -max_muclients 8;} \
        -rvrnightly {-mumode mu -apafteriperf "wl pmac shmx 0xa8 -n 10"}

# Chunyu test request - 11/19/2016
4366ap clone 4366softap-301-mu
4366softap-301-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-tag EAGLE_REL_10_10_122_308 \
	-rvrnightly {-mumode mu}

4366ap clone 4366softap-301m4-mu
4366softap-301m4-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-tag EAGLE_REL_10_10_122_308 \
	-rvrnightly {-mumode mu} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl mu_policy -max_muclients 4;}

4366ap clone 4366softap-31-mu
4366softap-31-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-tag EAGLE_REL_10_10_122_13 \
	-rvrnightly {-mumode mu}

UTF::DHD md16tst7(ap) \
        -lan_ip md16tst7 \
        -sta {4366apfd eth0} \
        -hostconsole "md16end1:40007" \
        -power {npc913 1} \
        -power_button "auto" \
        -dhd_tag DHD_TWIG_1_363_45 \
        -dhd_brand linux-internal-media \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl bw_cap 2g -1; wl country US/0; wl vht_features 7;} \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin


# Use mfgtest/err-assert target - good for all 1x1 tests.
4366apfd clone 4366softapfd-su
4366softapfd-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -nvram_add ctdma=0
4366apfd clone 4366softapfd-mu
4366softapfd-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu} -nvram_add ctdma=1

# Use non-mfgtest/err-assert target - good for 2x2 test for getting maximum tput.
4366apfd clone 4366softapfd-perf-su
4366softapfd-perf-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -nvram_add ctdma=0 \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-dump-murx.bin
4366apfd clone 4366softapfd-perf-mu
4366softapfd-perf-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu} -nvram_add ctdma=1 \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-dump-murx.bin

4366apfd clone 4366softapfd-cst-mu
4366softapfd-cst-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu} -nvram_add ctdma=1 \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-dump-murx.bin

4366apfd clone 4366softapfd-atf-su
4366softapfd-atf-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-nvram_add ctdma=1 \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl bw_cap 2g -1; wl country US/0; wl vht_features 7; wl atf 1;}
4366apfd clone 4366softapfd-atf-mu
4366softapfd-atf-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu} -nvram_add ctdma=1 \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl bw_cap 2g -1; wl country US/0; wl vht_features 7; wl atf 1;}

4366apfd clone 4366softapfd-rel-su
4366softapfd-rel-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-brand linux-2.6.36-arm-internal-router-dhdap-atlas \
	-tag BISON04T_REL_7_14_164_16 \
	-type 4366c0-roml/config_pcie_internal
4366apfd clone 4366softapfd-rel-mu
4366softapfd-rel-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} \
	-brand linux-2.6.36-arm-internal-router-dhdap-atlas \
	-tag BISON04T_REL_7_14_164_15 \
	-type 4366c0-roml/config_pcie_internal

4366apfd clone 4366softapfd-rele-su
4366softapfd-rele-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
        -tag BISON04T_REL_7_14_164_16 \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-stamon-hostpmac-murx-splitassoc-hostmemucode-dyn160-dhdhdr
4366apfd clone 4366softapfd-rele-mu
4366softapfd-rele-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} \
        -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
        -tag BISON04T_REL_7_14_164_16 \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-stamon-hostpmac-murx-splitassoc-hostmemucode-dyn160-dhdhdr

# for testing
4366apfd clone 4366softapfd-t-mu
4366softapfd-t-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu} -nvram_add ctdma=1 \
	-dhd_date 2016.6.21.0


###########################################
# STA Section
###########################################

# txbf_bfr_cap
# 0 disable beamformer capability
# 1 enable su beamformer capability
# 2 enable mu and su beamformer capability 

# txbf_bfe_cap
# 0 disable beamformee capability
# 1 enable su beamformee capability
# 2 enable mu and su beamformee capability 

###########################################
# MD16a - STA 
# 4366mc_p143 C0 (12/31/2015)
# Attn Group: G1 (1 2 3 4)
############################################
UTF::Linux md16tst1 \
        -lan_ip md16tst1 \
        -sta {4366a enp1s0} \
        -console "md16end1:40001" \
        -power {npc97 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

4366a clone 4366a1x1 \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366a clone 4366a1x1su \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

4366a clone 4366a2x2 \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

4366a clone 4366a2x2su \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

4366a clone 4366a1x1trunk -tag trunk \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366a clone sta1x1mu \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366a clone sta1x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366a clone sta1x1vht \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366a clone sta1x1ht \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
4366a clone sta1x1lt \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
4366a clone sta1x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
4366a clone sta1x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366a clone sta1x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366a clone sta1x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
4366a clone sta1x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}

# for testing
4366a clone 4366a1x1L \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl vht_features 0; wl vhtmode 0; wl nmode 0;}

4366a clone 4366a1x1t \
	-rvrnightly {-staateachattn "wl phyreg 0x339 0xf a"} \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

UTF::DHD md16tst1 \
        -lan_ip md16tst1 \
        -sta {4366afd eth0} \
        -console "md16end1:40001" \
        -power {npc97 1} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -post_perf_hook {{%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

4366afd clone 4366afd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country US/0; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366afd clone 4366afd1x1t \
	-tag EAGLE_TWIG_10_10_122 \
	-brand linux-internal-media \
	-type 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-assert-dbgam-err-wl11k-slvradar.bin \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country US/0; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

###########################################
# MD16a - STA
# 4366mc_p143 C0 (12/31/2015)
# Attn Group: G1 (1 2 3 4)
###########################################
UTF::Linux md16tst2 \
        -lan_ip md16tst2 \
        -sta {4366b enp1s0} \
        -console "md16end1:40002" \
        -power {npc97 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

4366b clone 4366b1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366b clone 4366b1x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

4366b clone 4366b2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

4366b clone 4366b2x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

4366b clone 4366b1x1ht \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl vhtmode 0;}

4366b clone 4366b1x1trunk -tag trunk \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366b clone sta2x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366b clone sta2x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366b clone sta2x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366b clone sta2x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
4366b clone sta2x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
4366b clone sta2x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
4366b clone sta2x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366b clone sta2x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366b clone sta2x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
4366b clone sta2x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}

# for testing
4366b clone 4366b1x1L \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl vht_features 0; wl vhtmode 0; wl nmode 0;}

4366b clone 4366b1x1t \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

UTF::DHD md16tst2 \
        -lan_ip md16tst2 \
        -sta {4366bfd eth0} \
        -console "md16end1:40002" \
        -power {npc97 2} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -post_perf_hook {{%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country US/0; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

4366bfd clone 4366bfd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country US/0; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

##########################################
# MD16a - STA
# 4366mc_p143 C0 (1/12/2016)
# Attn Group: G1 (1 2 3 4)
##########################################
UTF::Linux md16tst3 \
        -lan_ip md16tst3 \
        -sta {4366c enp1s0} \
        -console "md16end1:40003" \
        -power {npc98 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

4366c clone 4366c1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366c clone 4366c1x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

4366c clone 4366c2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

4366c clone 4366c2x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

4366c clone sta3x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366c clone sta3x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366c clone sta3x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366c clone sta3x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
4366c clone sta3x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
4366c clone sta3x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
4366c clone sta3x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366c clone sta3x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366c clone sta3x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
4366c clone sta3x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}

# for testing
4366c clone 4366c1x1t \
      	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

UTF::DHD md16tst3 \
        -lan_ip md16tst3 \
        -sta {4366cfd eth0} \
        -console "md16end1:40003" \
        -power {npc98 1} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -post_perf_hook {{%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country US/0; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

4366cfd clone 4366cfd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country US/0; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}


##########################################
# MD16a - STA
# 4366mc_p143 C0 (1/12/2016)
# Attn Group: G1 (1 2 3 4)
##########################################
UTF::Linux md16tst4 \
        -lan_ip md16tst4 \
        -sta {4366d enp1s0} \
        -console "md16end1:40004" \
        -power {npc98 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

4366d clone 4366d1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366d clone 4366d1x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

4366d clone 4366d2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

4366d clone 4366d2x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

4366d clone 4366d1x1lt \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl vhtmode 0; wl nmode 0;}

4366d clone 4366d1x1ht \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl vhtmode 0;}

4366d clone sta4x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366d clone sta4x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366d clone sta4x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366d clone sta4x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
4366d clone sta4x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
4366d clone sta4x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
4366d clone sta4x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366d clone sta4x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366d clone sta4x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
4366d clone sta4x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}

# for testing
4366d clone 4366d1x1t \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

UTF::DHD md16tst4 \
        -lan_ip md16tst4 \
        -sta {4366dfd eth0} \
        -console "md16end1:40004" \
        -power {npc98 2} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -post_perf_hook {{%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country US/0; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

4366dfd clone 4366dfd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country US/0; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}


##########################################
# MD16a - STA
# 4366Emc_p101 (8/16/2016)
# Attn Group: G1 (1 2 3 4)
##########################################
UTF::Linux md16tst5 \
        -lan_ip md16tst5 \
        -sta {4366e enp1s0} \
        -console "md16end1:40005" \
        -power {npc99 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

4366e clone 4366e1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366e clone 4366e1x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

4366e clone 4366e2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

4366e clone 4366e2x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

4366e clone 4366e1x1lt \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl vhtmode 0; wl nmode 0;}

4366e clone 4366e1x1trunk -tag trunk \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366e clone 4366e1x1t \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366e clone sta5x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366e clone sta5x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366e clone sta5x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366e clone sta5x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
4366e clone sta5x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
4366e clone sta5x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
4366e clone sta5x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366e clone sta5x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366e clone sta5x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
4366e clone sta5x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}


##########################################
# MD16a - STA
# 4366Emc_p101 (8/16/2016)
##########################################
UTF::Linux md16tst6 \
        -lan_ip md16tst6 \
        -sta {4366f enp1s0} \
        -console "md16end1:40006" \
        -power {npc99 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

4366f clone 4366f1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366f clone 4366f1x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

4366f clone 4366f2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

4366f clone 4366f2x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

4366f clone 4366f1x1ht \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl vhtmode 0;}

4366f clone 4366f1x1trunk -tag trunk \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366f clone 4366f1x1t \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366f clone sta6x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366f clone sta6x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366f clone sta6x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366f clone sta6x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
4366f clone sta6x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
4366f clone sta6x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
4366f clone sta6x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366f clone sta6x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366f clone sta6x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
4366f clone sta6x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}

#########################################
# MD16a - STA
# 4366Emc_p101 (8/16/2016)
##########################################
UTF::Linux md16tst14 \
        -lan_ip md16tst14 \
        -sta {4366g enp1s0} \
        -console "md16end1:40014" \
        -power {npc914 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

4366g clone 4366g1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366g clone 4366g1x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

4366g clone 4366g2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

4366g clone 4366g2x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

4366g clone 4366g1x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl vhtmode 0;}

4366g clone sta7x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366g clone sta7x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366g clone sta7x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366g clone sta7x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
4366g clone sta7x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
4366g clone sta7x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
4366g clone sta7x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366g clone sta7x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366g clone sta7x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
4366g clone sta7x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}

# for testing
4366g clone 4366g1x1t \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}


#########################################
## MD16a - STA
## 4366Emc_p101 (8/16/2016)
###########################################
UTF::Linux md16tst15 \
        -lan_ip md16tst15 \
        -sta {4366h enp1s0} \
        -console "md16end1:40015" \
        -power {npc914 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

4366h clone 4366h1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

4366h clone 4366h1x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

4366h clone 4366h2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

4366h clone 4366h2x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

4366h clone 4366h1x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl vhtmode 0;}

4366h clone sta8x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366h clone sta8x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366h clone sta8x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
4366h clone sta8x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
4366h clone sta8x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
4366h clone sta8x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
4366h clone sta8x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366h clone sta8x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
4366h clone sta8x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
4366h clone sta8x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}

# for testing
4366h clone 4366h1x1t \
        -rvrnightly {-staateachattn "wl phy_watchdog 0"} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}


### ----------------------------
UTF::Q md16a



