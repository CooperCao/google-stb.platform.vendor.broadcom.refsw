# -*-tcl-*-
#
# Testbed MC89 sub-rig configuration file
# Filename: mc89a.tcl
# Charles Chai 
# 01/19/2013: Initial setup
# 11/03/2016: Refurbished to 16-client MU-MIMO rig
# 
# Hardware
#   MC89END1   : controller
#   MC89END2   : softap
#   MC89TST1-32: STA
#   switch     : Cisco SG300-10
#   power      : netCommander Model NPC22(s)
#   attenuator : AeroFlex/Weinsche Model 8331 (0-95.5dB with 0.5 steps, 9 ports)
#   Channel    : Butler matrix channel design Rev 3.0
#
source "utfconf/mc89.tcl"

package require UTF::Linux
package require UTF::utils

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/mc89a"

# Set default to use wl from trunk; Use -app_tag to modify.
#set UTF::TrunkApps 1 ;# do not need it anymore as Tim set it as UTF default

# To enable ChannelSweep performance test
set UTF::ChannelPerf 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G1 attn 0

    # Make sure all systems are deinit and down 
    foreach S {4366softap-mu sta1 sta2 sta3 sta4 sta5 sta6 sta7 sta8 sta9 sta10 sta11 sta12 sta13 sta14 sta15 sta16 sta17 sta18 sta19 sta20 sta21 sta22 sta23 sta24 sta25 sta26 sta27 sta28 sta29 sta30 sta31 sta32} {
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
# softap
# 4366Emc_p101, 11ac 4x4
###########################################
UTF::Linux softap \
        -lan_ip mc89end2 \
        -sta {4366ap enp1s0} \
        -console "mc89end1:40000" \
        -power {npcap2 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
	-tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
	-msgactions {"PHYTX error" FAIL} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl atf 1;}

4366ap clone 4366softap-su
4366softap-su configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1
4366ap clone 4366softap-mu
4366softap-mu configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu}

4366ap clone 4366softap-cst-su
4366softap-cst-su configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1
4366ap clone 4366softap-cst-mu
4366softap-cst-mu configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu}

4366ap clone 4366softap-atf-su
4366softap-atf-su configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1
4366ap clone 4366softap-atf-mu
4366softap-atf-mu configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu}

# for testing
4366ap clone 4366softap-atf1-su
4366softap-atf1-su configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl atf 1; wl ampdu_atf_us 10000; wl ampdu_atf_min_us 100;}


UTF::DHD softap \
        -lan_ip mc89end2 \
        -sta {4366apfd eth0} \
        -hostconsole "mc89end1:40017" \
        -power {npcap2 2} \
        -power_button "auto" \
        -dhd_tag DHD_TWIG_1_363_45 \
        -dhd_brand linux-internal-media \
        -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
	-driver dhd-msgbuf-pciefd-debug \
        -tag BISON04T_BRANCH_7_14 \
        -reloadoncrash 1 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl bw_cap 2g -1; wl country US/0; wl vht_features 7; wl atf 1;} \
	-type 4366c0-roml/config_pcie_internal

4366apfd clone 4366softapfd-su
4366softapfd-su configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1
4366apfd clone 4366softapfd-mu
4366softapfd-mu configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu}

4366apfd clone 4366softapfdex-su
4366softapfdex-su configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-stamon-hostpmac-murx-splitassoc-hostmemucode-dyn160-dhdhdr-fbt-htxhdr-amsdufrag
4366apfd clone 4366softapfdex-mu
4366softapfdex-mu configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} \
	-brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-stamon-hostpmac-murx-splitassoc-hostmemucode-dyn160-dhdhdr-fbt-htxhdr-amsdufrag

4366apfd clone 4366softapfd-tob-su
4366softapfd-tob-su configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-brand linux-internal-dongle-pcie \
	-tag EAGLE_BRANCH_10_10 \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx
4366apfd clone 4366softapfd-tob-mu
4366softapfd-tob-mu configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} \
	-brand linux-internal-dongle-pcie \
	-tag EAGLE_BRANCH_10_10 \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx

# for testing
4366apfd clone 4366softapfd-t1-su
4366softapfd-t1-su configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-tag EAGLE_BRANCH_10_10 \
	-brand linux-internal-dongle-pcie \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx


4366apfd clone 4366softapfd-t2-su
4366softapfd-t2-su configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-tag BISON04T_REL_7_14_164_14 \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-stamon-hostpmac-murx-splitassoc-hostmemucode-dyn160-dhdhdr

4366apfd clone 4366softapfd-t3-su
4366softapfd-t3-su configure -ipaddr 192.168.1.135 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-tag BISON04T_REL_7_14_164_14 \
	-brand linux-2.6.36-arm-internal-router-dhdap-atlas \
	-type 4366c0-roml/config_pcie_internal

###########################################
# STA Section 
###########################################
UTF::Linux STA1 \
        -lan_ip mc89tst1 \
        -sta {sta1 enp1s0} \
        -console "mc89end1:40001" \
        -power {npcdut1a 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta1 clone sta1x1 \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta1 clone sta1x2 \
	-wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta1 clone sta1x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta1 clone sta1x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta1 clone sta1x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta1 clone sta1x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
sta1 clone sta1x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
sta1 clone sta1x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
sta1 clone sta1x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta1 clone sta1x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta1 clone sta1x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
sta1 clone sta1x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA2 \
        -lan_ip mc89tst2 \
        -sta {sta2 enp1s0} \
        -console "mc89end1:40002" \
        -power {npcdut1a 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta2 clone sta2x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta2 clone sta2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta2 clone sta2x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta2 clone sta2x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta2 clone sta2x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta2 clone sta2x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
sta2 clone sta2x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
sta2 clone sta2x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
sta2 clone sta2x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta2 clone sta2x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta2 clone sta2x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
sta2 clone sta2x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA3 \
        -lan_ip mc89tst3 \
        -sta {sta3 enp1s0} \
        -console "mc89end1:40003" \
        -power {npcdut1b 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta3 clone sta3x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta3 clone sta3x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta3 clone sta3x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta3 clone sta3x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta3 clone sta3x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta3 clone sta3x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
sta3 clone sta3x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
sta3 clone sta3x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
sta3 clone sta3x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta3 clone sta3x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta3 clone sta3x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
sta3 clone sta3x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA4 \
        -lan_ip mc89tst4 \
        -sta {sta4 enp1s0} \
        -console "mc89end1:40004" \
        -power {npcdut1b 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta4 clone sta4x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta4 clone sta4x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta4 clone sta4x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta4 clone sta4x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta4 clone sta4x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta4 clone sta4x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
sta4 clone sta4x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
sta4 clone sta4x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
sta4 clone sta4x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta4 clone sta4x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta4 clone sta4x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
sta4 clone sta4x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA5 \
        -lan_ip mc89tst5 \
        -sta {sta5 enp1s0} \
        -console "mc89end1:40005" \
        -power {npcdut2a 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta5 clone sta5x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta5 clone sta5x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta5 clone sta5x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta5 clone sta5x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta5 clone sta5x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta5 clone sta5x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
sta5 clone sta5x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
sta5 clone sta5x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
sta5 clone sta5x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta5 clone sta5x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta5 clone sta5x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
sta5 clone sta5x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA6 \
        -lan_ip mc89tst6 \
        -sta {sta6 enp1s0} \
        -console "mc89end1:40006" \
        -power {npcdut2a 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta6 clone sta6x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta6 clone sta6x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta6 clone sta6x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta6 clone sta6x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta6 clone sta6x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta6 clone sta6x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
sta6 clone sta6x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
sta6 clone sta6x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
sta6 clone sta6x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta6 clone sta6x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta6 clone sta6x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
sta6 clone sta6x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA7 \
        -lan_ip mc89tst7 \
        -sta {sta7 enp1s0} \
        -console "mc89end1:40007" \
        -power {npcdut2b 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta7 clone sta7x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta7 clone sta7x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta7 clone sta7x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta7 clone sta7x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta7 clone sta7x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta7 clone sta7x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
sta7 clone sta7x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
sta7 clone sta7x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
sta7 clone sta7x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta7 clone sta7x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta7 clone sta7x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
sta7 clone sta7x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA8 \
        -lan_ip mc89tst8 \
        -sta {sta8 enp1s0} \
        -console "mc89end1:40008" \
        -power {npcdut2b 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta8 clone sta8x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta8 clone sta8x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

sta8 clone sta8x1mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta8 clone sta8x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta8 clone sta8x1vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}
sta8 clone sta8x1ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}
sta8 clone sta8x1lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 1; wl txchain 1;}
sta8 clone sta8x2mu \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
sta8 clone sta8x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta8 clone sta8x2vht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
sta8 clone sta8x2ht \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 3; wl txchain 3;}
sta8 clone sta8x2lt \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl nmode 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA9 \
        -lan_ip mc89tst9 \
        -sta {sta9 enp1s0} \
        -console "mc89end1:40009" \
        -power {npcdut3a 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta9 clone sta9x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta9 clone sta9x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA10 \
        -lan_ip mc89tst10 \
        -sta {sta10 enp1s0} \
        -console "mc89end1:40010" \
        -power {npcdut3a 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta10 clone sta10x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta10 clone sta10x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA11 \
        -lan_ip mc89tst11 \
        -sta {sta11 enp1s0} \
        -console "mc89end1:40011" \
        -power {npcdut3b 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta11 clone sta11x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta11 clone sta11x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA12 \
        -lan_ip mc89tst12 \
        -sta {sta12 enp1s0} \
        -console "mc89end1:40012" \
        -power {npcdut3b 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta12 clone sta12x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta12 clone sta12x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA13 \
        -lan_ip mc89tst13 \
        -sta {sta13 enp1s0} \
        -console "mc89end1:40013" \
        -power {npcdut4a 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta13 clone sta13x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta13 clone sta13x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA14 \
        -lan_ip mc89tst14 \
        -sta {sta14 enp1s0} \
        -console "mc89end1:40014" \
        -power {npcdut4a 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta14 clone sta14x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta14 clone sta14x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA15 \
        -lan_ip mc89tst15.sjs.broadcom.net \
        -sta {sta15 enp1s0} \
        -console "mc89end1:40015" \
        -power {npcdut4b 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta15 clone sta15x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta15 clone sta15x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA16 \
        -lan_ip mc89tst16.sjs.broadcom.net \
        -sta {sta16 enp1s0} \
        -console "mc89end1:40016" \
        -power {npcdut4b 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta16 clone sta16x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta16 clone sta16x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA17 \
        -lan_ip mc89tst17.sj.broadcom.com \
        -sta {sta17 enp1s0} \
        -console "mc89end1:40017" \
        -power {npcdut5a 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta17 clone sta17x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta17 clone sta17x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA18 \
        -lan_ip mc89tst18.sj.broadcom.com \
        -sta {sta18 enp1s0} \
        -console "mc89end1:40018" \
        -power {npcdut5a 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta18 clone sta18x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta18 clone sta18x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA19 \
        -lan_ip mc89tst19.sjs.broadcom.net \
        -sta {sta19 enp1s0} \
        -console "mc89end1:40019" \
        -power {npcdut5b 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta19 clone sta19x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta19 clone sta19x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA20 \
        -lan_ip mc89tst20.sjs.broadcom.net \
        -sta {sta20 enp1s0} \
        -console "mc89end1:40020" \
        -power {npcdut5b 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta20 clone sta20x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta20 clone sta20x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA21 \
        -lan_ip mc89tst21.sj.broadcom.com \
        -sta {sta21 enp1s0} \
        -console "mc89end1:40021" \
        -power {npcdut6a 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta21 clone sta21x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta21 clone sta21x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA22 \
        -lan_ip mc89tst22.sjs.broadcom.net \
        -sta {sta22 enp1s0} \
        -console "mc89end1:40022" \
        -power {npcdut6a 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta22 clone sta22x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta22 clone sta22x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

UTF::Linux STA23 \
        -lan_ip mc89tst23.sj.broadcom.com \
        -sta {sta23 enp1s0} \
        -console "mc89end1:40023" \
        -power {npcdut6b 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta23 clone sta23x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta23 clone sta23x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
sta23 clone sta23x3 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 7; wl txchain 7;}

UTF::Linux STA24 \
        -lan_ip mc89tst24.sj.broadcom.com \
        -sta {sta24 enp1s0} \
        -console "mc89end1:40024" \
        -power {npcdut6b 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta24 clone sta24x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta24 clone sta24x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}
sta24 clone sta24x3 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 7; wl txchain 7;}


UTF::Linux STA25 \
        -lan_ip mc89tst25.sj.broadcom.com \
        -sta {sta25 enp1s0} \
        -console "mc89end1:40025" \
        -power {npcdut8a 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta25 clone sta25x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta25 clone sta25x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA26 \
        -lan_ip mc89tst26.sj.broadcom.com \
        -sta {sta26 enp1s0} \
        -console "mc89end1:40026" \
        -power {npcdut8a 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta26 clone sta26x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta26 clone sta26x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA27 \
        -lan_ip mc89tst27.sj.broadcom.com \
        -sta {sta27 enp1s0} \
        -console "mc89end1:40027" \
        -power {npcdut8b 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta27 clone sta27x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta27 clone sta27x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA28 \
        -lan_ip mc89tst28.sj.broadcom.com \
        -sta {sta28 enp1s0} \
        -console "mc89end1:40028" \
        -power {npcdut8b 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta28 clone sta28x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta28 clone sta28x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA29 \
        -lan_ip mc89tst29.sj.broadcom.com \
        -sta {sta29 enp1s0} \
        -console "mc89end1:40029" \
        -power {npcdut7a 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta29 clone sta29x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta29 clone sta29x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA30 \
        -lan_ip mc89tst30.sj.broadcom.com \
        -sta {sta30 enp1s0} \
        -console "mc89end1:40030" \
        -power {npcdut7a 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta30 clone sta30x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta30 clone sta30x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA31 \
        -lan_ip mc89tst31.sj.broadcom.com \
        -sta {sta31 enp1s0} \
        -console "mc89end1:40031" \
        -power {npcdut7b 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta31 clone sta31x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta31 clone sta31x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


UTF::Linux STA32 \
        -lan_ip mc89tst32.sj.broadcom.com \
        -sta {sta32 enp1s0} \
        -console "mc89end1:40032" \
        -power {npcdut7b 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 -slowassoc 5 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;}
sta32 clone sta32x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta32 clone sta32x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###
UTF::Q mc89a


