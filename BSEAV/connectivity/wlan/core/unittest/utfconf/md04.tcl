# -*-tcl-*-
#
# Testbed MD04 configuration file
# UTF config file: md04.tcl
# 12/20/2015: Built for Chunyu Hu by Charles Chai
# 07/29/2016: Added 4 STAs for 8MU support
# 12/01/2016: Added AtlasII router
# 12/10/2016: Replaced 6-port VA with 12-port VA
# 12/20/2016: Added 4908 router
# 
# Hardware
#   MD04END1  : FC19 controller
#   Router    : 4709/4366mc_p143
#   Router    : AtlasII/4366mc_p143
#   Router    : 4908/4366mc_p143
#   MD04TST7  : FC19 softap
#   MD04TST1  : FC19 STA
#   MD04TST2  : FC19 STA
#   MD04TST3  : FC19 STA
#   MD04TST4  : FC19 STA
#   MD04TST5  : FC19 STA
#   MD04TST6  : FC19 STA
#   MD04TST8  : FC19 STA
#   MD04TST9  : FC19 STA
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 8311 (0-103dB with 1dB step)
#

package require UTF::Linux
package require UTF::utils
package require UTF::Aeroflex

# SummaryDir sets location for test logs
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/md04"

# Add DataRatePlot to all embedded runs
set UTF::DataRatePlot 1

# Endpoint: FC19_x64 Desktop
UTF::Linux md04end1 -sta {lan em2}

# To use wl from trunk (set default); Use -app_tag to modify.
#set UTF::TrunkApps 1 ;# do not need it anymore as Tim set it as UTF default

# To enable ChannelSweep performance test 
set UTF::ChannelPerf 1

# Define power controllers (NPC)
# NOTE: Option '-rev 1' is required for newer NPC22(s) model
UTF::Power::Synaccess npc95  -relay "md04end1" -lan_ip 172.5.9.5  -rev 1
UTF::Power::Synaccess npc96  -relay "md04end1" -lan_ip 172.5.9.6  -rev 1
UTF::Power::Synaccess npc97  -relay "md04end1" -lan_ip 172.5.9.7  -rev 1
UTF::Power::Synaccess npc98  -relay "md04end1" -lan_ip 172.5.9.8  -rev 1
UTF::Power::Synaccess npc99  -relay "md04end1" -lan_ip 172.5.9.9  -rev 1
UTF::Power::Synaccess npc910 -relay "md04end1" -lan_ip 172.5.9.10 -rev 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.5.9.1 \
    -relay "md04end1" -group {G1 {1 2 3 4} G2 {5 6 7 8} ALL {1 2 3 4 5 6 7 8}}

# Set default attenuator ranges
set ::cycle5G80AttnRange "0-103 103-0"
set ::cycle5G40AttnRange "0-103 103-0"
set ::cycle5G20AttnRange "0-103 103-0"
set ::cycle2G40AttnRange "0-103 103-0"
set ::cycle2G20AttnRange "0-103 103-0"

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G1 attn 0
    G2 attn 0

    # Make sure all devices are deinit (for REF) and down (for DUT).
    foreach S {4366softap-mu 4709rtr-mu 4366a1x1 4366b1x1 4366c1x1 4366d1x1 4366e1x1 4366f1x1 4366g1x1 4366h1x1} {
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
# AP section
###########################################

###########################################
# 4709 Reference Router (4709C0 board with 4366mc_p143 C0)
# Attn Group: G1 (1 2 3 4) (a b c d)
# Attn Group: G2 (5 6 7 8) (e f g h)
###########################################
UTF::Router 4709ref \
    -sta {4709/4366 eth1 4709/4366.%15 wl0.%} \
    -relay lan \
    -lan_ip 192.168.1.1 \
    -lanpeer {lan} \
    -console "md04end1:40008" \
    -relay "md04end1" \
    -power {npc97 2} \
    -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-internal-router-dhdap \
    -datarate {-i 0.5} -udp 1.8g \
    -noradio_pwrsave 1 \
    -nvram {
        watchdog=3000
        wl0_ssid=4709/4366
        wl0_chanspec=36
        wl0_radio=0
	wl0_bw_cap=-1
	wl0_country_code=US/0
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

# NIC driver
4709/4366 clone 4709rtr-nic-su -tag EAGLE_BRANCH_10_10 \
	-brand linux-2.6.36-arm-internal-router
4709/4366 clone 4709rtr-nic-mu -tag EAGLE_BRANCH_10_10 \
	-brand linux-2.6.36-arm-internal-router \
        -rvrnightly {-mumode mu}

# for testing
4709/4366 clone 4709rtr-t1-su -rvrnightly {-mumode su -ch5g80 "157/80"} -tag BISON04T_REL_7_14_164 
4709/4366 clone 4709rtr-t1-mu -rvrnightly {-mumode mu} -tag BISON04T_BRANCH_7_14


###########################################
# AtlasII Router (MCH5h, MCH5l, MCH2)
# Attn Group: G1 (1 2 3 4) (a b c d)
# Attn Group: G2 (5 6 7 8) (e f g h)
###########################################
UTF::Router atlas2 \
    -sta {atlas2mch5h eth3 atlas2mch5h.%15 wl2.% atlas2mch5l eth2 atlas2mch5l.%15 wl1.% atlas2mch2 eth1 atlas2mch2.%15 wl0.%} \
    -relay lan \
    -lan_ip 192.168.1.15 \
    -lanpeer {lan} \
    -console "md04end1:40011" \
    -relay "md04end1" \
    -power {npc910 1} \
    -tag BISON04T_TWIG_7_14_171  \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -datarate {-i 0.5 -frameburst 1} -udp 1.8g \
    -noradio_pwrsave 1 -nosamba 1 \
    -slowassoc 5 \
    -nvram {
    	watchdog=3000
    	wl0_ssid=atlas2mch2
    	wl0_chanspec=1l
    	wl0_bw_cap=-1
    	wl0_radio=0
	wl0_country_code=US/0
	wl0_vht_features=5
	wl0_mu_features="-1"
    	wl1_ssid=atlas2mch5l
    	wl1_chanspec=36
    	wl1_bw_cap=-1
    	wl1_radio=0
	wl1_country_code=Q1
	wl1_country_rev=137
	wl1_vht_features=7
	wl1_mu_features="-1"
    	wl2_ssid=atlas2mch5h
    	wl2_chanspec=161
    	wl2_bw_cap=-1
    	wl2_radio=0
	wl2_country_code=Q1
	wl2_country_rev=137
	wl2_vht_features=7
	wl2_mu_features="-1"
        lan_ipaddr=192.168.1.15
        dhcp_start=192.168.1.100
        dhcp_end=192.168.1.150
    }

atlas2mch5l clone atlas2mch5l-su
atlas2mch5l clone atlas2mch5l-mu -rvrnightly {-mumode mu}

# NIC driver
atlas2mch5l clone atlas2mch5l-nic-su -tag EAGLE_TWIG_10_10_132 \
	-brand linux-2.6.36-arm-internal-router
atlas2mch5l clone atlas2mch5l-nic-mu -tag EAGLE_TWIG_10_10_132 \
	-brand linux-2.6.36-arm-internal-router -rvrnightly {-mumode mu}

atlas2mch5h clone atlas2mch5h-su -perfchans {157/80 157l 157}
atlas2mch5h clone atlas2mch5h-mu -perfchans {157/80 157l 157} -rvrnightly {-mumode mu}

##########################################
# 4908 Reference Router (4366MC)
# Attn Group: G1 (1 2 3 4) (a b c d)
# Attn Group: G2 (5 6 7 8) (e f g h)
###########################################
#UTF::Router 4908ref \
    -sta {4908/4366 eth1 4908/4366.%15 wl0.%} \
    -relay lan \
    -lan_ip 192.168.1.25 \
    -lanpeer {lan} \
    -console "md04end1:40012" \
    -relay "md04end1" \
    -power {npc910 2} \
    -tag BISON04T_BRANCH_7_14 \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -datarate {-i 0.5 -frameburst 1} -udp 1.8g \
    -noradio_pwrsave 1 -nosamba 1 \
    -slowassoc 5

###########################################
# softap
# 4366mc_p143 C0
# Attn Group: G1 (1 2 3 4) (a b c d)
# Attn Group: G2 (5 6 7 8) (e f g h)
###########################################
# NIC OBJECT
UTF::Linux md04tst7(ap) \
        -lan_ip md04tst7 \
        -sta {4366ap enp1s0} \
        -console "md04end1:40007" \
        -power {npc97 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_TWIG_10_10_132 \
        -reloadoncrash 1 \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl vht_features 7;}

4366ap clone 4366softap-su
4366softap-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1
4366ap clone 4366softap-mu
4366softap-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu}

4366ap clone 4366softap-3x3-su
4366softap-3x3-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-ap3coremode 1}
4366ap clone 4366softap-3x3-mu
4366softap-3x3-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu -ap3coremode 1}

4366ap clone 4366softap-obo-mu
4366softap-obo-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu}

4366ap clone 4366softap-msm-mu
4366softap-msm-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-mumode mu}

# for testing
4366ap clone 4366softap-t1-mu
4366softap-t1-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl vht_features 7;} \
	-tag EAGLE_BRANCH_10_10 \
        -rvrnightly {-mumode mu}

4366ap clone 4366softap-t2-mu
4366softap-t2-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -rvrnightly {-mumode mu} \
	-image /projects/ucode_group/shinuk/toChiu/02.164_2_surajvasip_rb101150_diff1/wl.ko

4366ap clone 4366softap-t3-mu
4366softap-t1-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
	-modopts {assert_type=1} \
        -rvrnightly {-mumode mu -apateachattn "wl ver; wl ver;"}


# DONGLE OBJECT
UTF::DHD md04tst7(ap) \
        -lan_ip md04tst7 \
        -sta {4366apfd eth0} \
        -hostconsole "md04end1:40007" \
        -power {npc97 1} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_TWIG_10_10_132 \
        -slowassoc 10 -reloadoncrash 1 \
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
        -nvram_add ctdma=1 \
	-rvrnightly {-mumode mu}

# Use non-mfgtest target - good for 2x2 test for getting maximum tput.
4366apfd clone 4366softapfd-nm-su
4366softapfd-nm-su configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -nvram_add ctdma=0 \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-dump-murx.bin
4366apfd clone 4366softapfd-nm-mu
4366softapfd-nm-mu configure -ipaddr 192.168.1.115 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -nvram_add ctdma=1 -rvrnightly {-mumode mu} \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-dump-murx.bin


###########################################
# STA SECTION
###########################################

###########################################
# 4366mc_P143 C0 (2/17/2016)
# Attn Group: G1 (1 2 3 4)
###########################################
UTF::Linux md04tst1 \
        -lan_ip md04tst1 \
        -sta {4366a enp1s0} \
        -console "md04end1:40001" \
        -power {npc95 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
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

UTF::DHD md04tst1 \
        -lan_ip md04tst1 \
        -sta {4366afd enp1s0} \
        -console "md04end1:40001" \
        -power {npc95 1} \
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

4366afd clone 4366afd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366afd clone 4366afd2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# 4366mc_P143 C0 (2/17/2016)
###########################################
UTF::Linux md04tst2 \
        -lan_ip md04tst2 \
        -sta {4366b enp1s0} \
        -console "md04end1:40002" \
        -power {npc95 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
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

UTF::DHD md04tst2 \
        -lan_ip md04tst2 \
        -sta {4366bfd enp1s0} \
        -console "md04end1:40002" \
        -power {npc95 2} \
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

4366bfd clone 4366bfd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366bfd clone 4366bfd2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# 4366mc_P143 C0 (2/17/2016)
############################################
UTF::Linux md04tst3 \
        -lan_ip md04tst3 \
        -sta {4366c enp1s0} \
        -console "md04end1:40003" \
        -power {npc96 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
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

UTF::DHD md04tst3 \
        -lan_ip md04tst3 \
        -sta {4366cfd enp1s0} \
        -console "md04end1:40003" \
        -power {npc96 1} \
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

4366cfd clone 4366cfd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366cfd clone 4366cfd2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# 4366mc_P143 C0 (2/17/2016)
#############################################
UTF::Linux md04tst4 \
        -lan_ip md04tst4 \
        -sta {4366d enp1s0} \
        -console "md04end1:40004" \
        -power {npc96 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
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

UTF::DHD md04tst4 \
        -lan_ip md04tst4 \
        -sta {4366dfd enp1s0} \
        -console "md04end1:40004" \
        -power {npc96 2} \
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

4366dfd clone 4366dfd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366dfd clone 4366dfd2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# 4366Emc_P101 C0 (7/29/2016) - LESI enabled
# Attn Group: G2 (5 6 7 8)
#############################################
UTF::Linux md04tst5 \
        -lan_ip md04tst5 \
        -sta {4366e enp1s0} \
        -console "md04end1:40005" \
        -power {npc98 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
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

UTF::DHD md04tst5 \
        -lan_ip md04tst5 \
        -sta {4366efd enp1s0} \
        -console "md04end1:40005" \
        -power {npc98 1} \
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

4366efd clone 4366efd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366efd clone 4366efd2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# 4366Emc_P101 C0 (7/29/2016) - LESI enabled
#############################################
UTF::Linux md04tst6 \
        -lan_ip md04tst6 \
        -sta {4366f enp1s0} \
        -console "md04end1:40006" \
        -power {npc98 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
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

UTF::DHD md04tst6 \
        -lan_ip md04tst6 \
        -sta {4366ffd enp1s0} \
        -console "md04end1:40006" \
        -power {npc98 2} \
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

4366ffd clone 4366ffd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366ffd clone 4366ffd2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# 4366Emc_P101 C0 (7/29/2016) - LESI enabled
#############################################
UTF::Linux md04tst8 \
        -lan_ip md04tst8 \
        -sta {4366g enp1s0} \
        -console "md04end1:40010" \
        -power {npc99 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
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

UTF::DHD md04tst8 \
        -lan_ip md04tst8 \
        -sta {4366gfd enp1s0} \
        -console "md04end1:40010" \
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

4366gfd clone 4366gfd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366gfd clone 4366gfd2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


###########################################
# 4366Emc_P101 C0 (7/29/2016) - LESI enabled
#############################################
UTF::Linux md04tst9 \
        -lan_ip md04tst9 \
        -sta {4366h enp1s0} \
        -console "md04end1:40009" \
        -power {npc99 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
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

UTF::DHD md04tst9 \
        -lan_ip md04tst9 \
        -sta {4366hfd enp1s0} \
        -console "md04end1:40009" \
        -power {npc99 2} \
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

4366hfd clone 4366hfd1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
4366hfd clone 4366hfd2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}


# =======================================
# UTF test scheduler
UTF::Q md04
# =======================================


