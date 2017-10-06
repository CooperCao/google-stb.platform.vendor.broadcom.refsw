# -*-tcl-*-
#
# Testbed MC99 sub-rig configuration file
# Filename: mc99a.tcl
# Charles Chai 6/4/2014
# Modified 09/25/2015 (add 4366 C0)
# Modified 02/29/2016 (replace 4359C0 with 4366C0)
# Modified 08/08/2016 (remove 4359; add 4 4366; move 4357 to mc99b rig) 
#
# Hardware
#   MC99END1  : FC19 controller
#   MC99TST7  : FC19 softap
#   MC99TST1  : FC19 STA1
#   MC99TST2  : FC19 STA2
#   MC99TST3  : FC19 STA3
#   MC99TST4  : FC19 STA4
#   MC99TST5  : FC19 STA5
#   MC99TST6  : FC19 STA6
#   MC99TST9  : FC19 STA7
#   MC99TST10 : FC19 STA8
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 8331 (0-95.5dB with 0.5 steps)
#

source "utfconf/mc99.tcl"

package require UTF::Linux
package require UTF::utils

#set ::gnuplot_rvr_xtics 4

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/mc99a"

# Enable dBux reporting 
#set UTF::dBuxRegister 1

# To enable ChannelSweep performance test
set UTF::ChannelPerf 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G1 attn 0
    G2 attn 0

    # Make sure all devices are deinit (for REF) and down (for DUT).
    foreach S {4366softap-mu sta1 sta2 sta3 sta4 sta5 sta6 sta7 sta8} { 
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


###########################################
# AP Section
###########################################

###########################################
# MC99a - softap
# 4366mc_p143 C0 (1/27/2016)
# Attn Group: G1 (1 2 3 4) (e f g h) rev3.0 (good channel)
# Attn Group: G2 (5 6 7 8) (a b c d) rev3.0 (good channel)
###########################################
UTF::Linux softap \
        -lan_ip mc99tst7 \
        -sta {4366ap enp1s0} \
        -console "mc99end1:40007" \
        -power {npc96 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
	-msgactions {"PHYTX error" FAIL} \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}

# SU/MU dynamic switching
# "wl mu_features 0" -- SU
# "wl mu_features 1" -- MU (AUTO, default)

# For MU testing
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

4366ap clone 4366softap-trunk-su
4366softap-trunk-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-tag trunk
4366ap clone 4366softap-trunk-mu
4366softap-trunk-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-tag trunk \
        -rvrnightly {-mumode mu}

4366ap clone 4366softap-3x3-su
4366softap-3x3-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-ap3coremode 1}
4366ap clone 4366softap-3x3-mu
4366softap-3x3-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu -ap3coremode 1}

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

4366ap clone 4366softap-udp-mu
4366softap-udp-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu}

# For TxBF testing
4366ap clone 4366softap-txbf0
4366softap-txbf0 configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf 0; wl txbf_imp 0; wl txchain 3; wl rxchain 3;}
4366ap clone 4366softap-txbf1
4366softap-txbf1 configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3;}

# Use ATF
4366ap clone 4366softap-atf-su
4366softap-atf-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode su} \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl atf 1;}
4366ap clone 4366softap-atf-mu
4366softap-atf-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu} \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl atf 1;}

# for testing
4366ap clone 4366softap-t1-mu
4366softap-t1-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu}

4366ap clone 4366softap-t2-mu
4366softap-t2-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-image /projects/ucode_group/pmallik/code/mimoctl/d/main/src/wl/wl_mimoctl.ko \
        -rvrnightly {-mumode mu}

4366ap clone 4366softap-t3-mu
4366softap-t3-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu -ap3coremode 1}

4366ap clone 4366softap-t4-mu
4366softap-t4-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu}

4366ap clone 4366softap-t5-mu
4366softap-t5-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu -apafteriperf "wl pmac shmx 0xa8 -n 10"}

4366ap clone 4366softap-t-mu
4366softap-t-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-tag EAGLE_REL_10_10_122_303 \
        -rvrnightly {-mumode mu}


# 4366C0 Full Dongle
UTF::DHD softap \
        -lan_ip mc99tst7 \
        -sta {4366apfd eth0} \
        -hostconsole "mc99end1:40007" \
        -power {npc96 1} \
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
4366softapfd-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 -nvram_add ctdma=0
4366apfd clone 4366softapfd-mu
4366softapfd-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} -nvram_add ctdma=1

# Use non-mfgtest target - good for 2x2 test for getting maximum tput.
4366apfd clone 4366softapfd-pf-su
4366softapfd-pf-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 -nvram_add ctdma=0 \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-dump-murx.bin
4366apfd clone 4366softapfd-pf-mu
4366softapfd-pf-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} -nvram_add ctdma=1 \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-dump-murx.bin

# Dongle from 7.14.164.x
4366apfd clone 4366softapfd-rel-su
4366softapfd-rel-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
        -tag BISON04T_REL_7_14_164_16 \
        -type 4366c0-roml/config_pcie_internal
4366apfd clone 4366softapfd-rel-mu
4366softapfd-rel-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} \
        -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
        -tag BISON04T_REL_7_14_164_16 \
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
4366apfd clone 4366softapfd-t1-mu
4366softapfd-t1-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu}

4366apfd clone 4366softapfd-t2-mu
4366softapfd-t2-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu}

4366apfd clone 4366softapfd-t3-mu
4366softapfd-t3-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu}

4366apfd clone 4366softapfd-x-mu
4366softapfd-x-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu} \
        -nvram_add ctdma=1 \
        -pre_perf_hook {{UTF::Every 1 %S wl bus:dumptxrings}} \
        -post_perf_hook {{UTF::Every cancel all}}

4366apfd clone 4366softapfd-atf-mu
4366softapfd-atf-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu} \
        -nvram_add ctdma=1 \
        -pre_perf_hook {{UTF::Every 1 %S wl bus:dumptxrings}} \
        -post_perf_hook {{UTF::Every cancel all}}


###########################################
# STA Section
###########################################

###########################################
# MC99a - STA
# 4366mc_p143 C0 (installed 2/29/2016)
###########################################
UTF::Linux STA1 \
        -lan_ip mc99tst1 \
        -sta {sta1 enp1s0} \
        -console "mc99end1:40001" \
        -power {npc99 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

sta1 clone sta1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

sta1 clone sta1x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

sta1 clone sta1x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta1 clone sta1x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

sta1 clone sta1x1t \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl PM 2;}

# for testing
sta1 clone sta1x1x \
	-rvrnightly {-stabeforeiperf "wl ver" -staafteriperf "wl ver"} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}


UTF::DHD STA1 \
        -lan_ip mc99tst1 \
        -sta {sta1fd eth0} \
        -hostconsole "mc99end1:40001" \
        -power {npc99 1} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 10 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

sta1fd clone sta1fdx1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta1fd clone sta1fdx2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# MC99a - STA
# 4366mc_p143 C0 (installed 2/29/2016)
###########################################
UTF::Linux STA2 \
        -lan_ip mc99tst2 \
        -sta {sta2 enp1s0} \
        -console "mc99end1:40002" \
        -power {npc99 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

sta2 clone sta2x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

sta2 clone sta2x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

sta2 clone sta2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta2 clone sta2x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

sta2 clone sta2x1t \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl PM 2;}

UTF::DHD STA2 \
        -lan_ip mc99tst2 \
        -sta {sta2fd eth0} \
        -hostconsole "mc99end1:40002" \
        -power {npc99 2} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

sta2fd clone sta2fdx1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta2fd clone sta2fdx2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# MC99a - STA
# 4366mc_p143 C0 (installed 2/29/2016)
###########################################
UTF::Linux STA3 \
        -lan_ip mc99tst3 \
        -sta {sta3 enp1s0} \
        -console "mc99end1:40003" \
        -power {npc910 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

sta3 clone sta3x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

sta3 clone sta3x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

sta3 clone sta3x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta3 clone sta3x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

sta3 clone sta3x1t \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl PM 2;}

UTF::DHD STA3 \
        -lan_ip mc99tst3 \
        -sta {sta3fd eth0} \
        -hostconsole "mc99end1:40003" \
        -power {npc910 1} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

sta3fd clone sta3fdx1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta3fd clone sta3fdx2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# MC99a - STA
# 4366mc_p143 C0 (installed 2/29/2016)
###########################################
UTF::Linux STA4 \
        -lan_ip mc99tst4 \
        -sta {sta4 enp1s0} \
        -console "mc99end1:40004" \
        -power {npc912 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

sta4 clone sta4x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

sta4 clone sta4x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

sta4 clone sta4x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta4 clone sta4x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

sta4 clone sta4x1t \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl PM 2;}

UTF::DHD STA4 \
        -lan_ip mc99tst4 \
        -sta {sta4fd eth0} \
        -hostconsole "mc99end1:40004" \
        -power {npc912 1} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

sta4fd clone sta4fdx1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta4fd clone sta4fdx2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# MC99a - STA
# 4366Emc_p101 (installed 8/8/2016)
###########################################
UTF::Linux STA5 \
        -lan_ip mc99tst5 \
        -sta {sta5 enp1s0} \
        -console "mc99end1:40005" \
        -power {npc913 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

sta5 clone sta5x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

sta5 clone sta5x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

sta5 clone sta5x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta5 clone sta5x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

sta5 clone sta5x1t \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl PM 2;}

sta5 clone sta5x1trunk -tag trunk \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

sta5 clone sta5x1L \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl vht_features 0; wl vhtmode 0; wl nmode 0;}


UTF::DHD STA5 \
        -lan_ip mc99tst5 \
        -sta {sta5fd eth0} \
        -hostconsole "mc99end1:40005" \
        -power {npc913 1} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

sta5fd clone sta5fdx1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta5fd clone sta5fdx2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# MC99a - STA
# 4366Emc_p101 (installed 8/8/2016)
###########################################
UTF::Linux STA6 \
        -lan_ip mc99tst6 \
        -sta {sta6 enp1s0} \
        -console "mc99end1:40006" \
        -power {npc913 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

sta6 clone sta6x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

sta6 clone sta6x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

sta6 clone sta6x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta6 clone sta6x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

sta6 clone sta6x1t \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl PM 2;}

sta6 clone sta6x1trunk -tag trunk \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

sta6 clone sta6x1L \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl vht_features 0; wl vhtmode 0; wl nmode 0;}


UTF::DHD STA6 \
        -lan_ip mc99tst6 \
        -sta {sta6fd eth0} \
        -hostconsole "mc99end1:40006" \
        -power {npc913 2} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

sta6fd clone sta6fdx1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta6fd clone sta6fdx2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# MC99a - STA
# 4366Emc_p101 (installed 8/8/2016)
###########################################
UTF::Linux STA7 \
        -lan_ip mc99tst9 \
        -sta {sta7 enp1s0} \
        -console "mc99end1:40009" \
        -power {npc95 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

sta7 clone sta7x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

sta7 clone sta7x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

sta7 clone sta7x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta7 clone sta7x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

sta7 clone sta7x1t \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl PM 2;}

sta7 clone sta7x1trunk -tag trunk \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

UTF::DHD STA7 \
        -lan_ip mc99tst9 \
        -sta {sta7fd eth0} \
        -hostconsole "mc99end1:40009" \
        -power {npc95 1} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

sta7fd clone sta7fdx1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta7fd clone sta7fdx2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# MC99a - STA
# 4366Emc_p101 (installed 8/8/2016)
###########################################
UTF::Linux STA8 \
        -lan_ip mc99tst10 \
        -sta {sta8 enp1s0} \
        -console "mc99end1:40010" \
        -power {npc95 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

sta8 clone sta8x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

sta8 clone sta8x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

sta8 clone sta8x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta8 clone sta8x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}

sta8 clone sta8x1t \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1; wl PM 2;}

sta8 clone sta8x1trunk -tag trunk \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

UTF::DHD STA8 \
        -lan_ip mc99tst10 \
        -sta {sta8fd eth0} \
        -hostconsole "mc99end1:40010" \
        -power {npc95 2} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

sta8fd clone sta8fdx1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta8fd clone sta8fdx2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}



###
UTF::Q mc99a

# ----------- end ------------





