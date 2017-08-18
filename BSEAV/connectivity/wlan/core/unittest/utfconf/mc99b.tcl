# -*-tcl-*-
#
# Testbed MC99 sub-rig configuration file
# Filename: mc99b.tcl
# Charles Chai 6/4/2014
# Modified 08/08/2016 (remove 4359; add 4 4366; move 4357 to mc99b rig)
# Modified 08/11/2016 (replace 4357A0 with 4357B0)
#
# Hardware
#   MC99END1  : FC19 controller
#   MC99TST8  : FC19 softap
#   MC99TST11 : FC19 STA 
#   MC99TST12 : FC19 STA
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 8331 (0-95.5dB with 0.5 steps)
#
source "utfconf/mc99.tcl"

package require UTF::Linux
package require UTF::utils

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/mc99b"

# Set default to use wl from trunk; Use -app_tag to modify.
#set UTF::TrunkApps 1 ;# do not need it anymore as Tim set it as UTF default

# To enable ChannelSweep performance test
set UTF::ChannelPerf 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G3 attn 0

    # Make sure all systems are deinit and down
    foreach S {4366ap sta1 sta2} {
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
## AP Section
###########################################

###########################################
## MC99b - softap 
## 4366mc_p143 C0 (3/16/2016)
## Attn Group: G3 {9 10 11 12}
###########################################
UTF::Linux softap \
        -lan_ip mc99tst8 \
        -sta {4366ap enp1s0} \
        -console "mc99end1:40008" \
        -power {npc96 2} \
        -brand linux-internal-wl \
	-tag EAGLE_BRANCH_10_10 \
	-reloadoncrash 1 \
        -wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
	-modopts {assert_type=1 nompc=1}

4366ap clone 4366softap-egl
4366softap-egl configure -ipaddr 192.168.1.125 -attngrp G3 -ap 1 -hasdhcpd 1

# For SU TxBF
4366ap clone 4366softap-egl-bf1
4366softap-egl-bf1 configure -ipaddr 192.168.1.125 -attngrp G3 -ap 1 -hasdhcpd 1 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txchain 11; wl rxchain 11;}
4366ap clone 4366softap-egl-bf0
4366softap-egl-bf0 configure -ipaddr 192.168.1.125 -attngrp G3 -ap 1 -hasdhcpd 1 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl vht_features 7; wl txbf 0; wl txbf_imp 0; wl txchain 11; wl rxchain 11;}

# For MU
4366ap clone 4366softap-su
4366softap-su configure -ipaddr 192.168.1.125 -attngrp G3 -ap 1 -hasdhcpd 1 \
	-modopts {assert_type=1 nompc=1 ctdma=0}
4366ap clone 4366softap-mu
4366softap-mu configure -ipaddr 192.168.1.125 -attngrp G3 -ap 1 -hasdhcpd 1 \
	-modopts {assert_type=1 nompc=1 ctdma=1} \
	-rvrnightly {-mumode mu}


# 4366 Full Dongle
UTF::DHD softap \
        -lan_ip mc99tst8 \
        -sta {4366apfd eth0} \
        -hostconsole "mc99end1:40008" \
        -power {npc96 2} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -nopm1 1 -nopm2 1 -nocal 1 -docpu 1 \
        -datarate {-i 0.5} \
	-tcpwindow 4m -udp 1.8g \
        -wlinitcmds {wl msglevel +assoc; wl down; wl country US/0; wl bw_cap 2g -1; wl vht_features 7;} \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump.bin

4366apfd clone 4366softap-fd
4366softap-fd configure -ipaddr 192.168.1.125 -attngrp G3 -ap 1 -hasdhcpd 1

# internal
4366apfd clone 4366softapfd-su
4366softapfd-su configure -ipaddr 192.168.1.125 -attngrp G3 -ap 1 -hasdhcpd 1 \
        -brand linux-internal-dongle-pcie -nvram_add ctdma=0
4366apfd clone 4366softapfd-mu
4366softapfd-mu configure -ipaddr 192.168.1.125 -attngrp G3 -ap 1 -hasdhcpd 1 \
        -brand linux-internal-dongle-pcie -nvram_add ctdma=1 \
	-rvrnightly {-mumode mu}


##########################################
# STA Section
##########################################

##########################################
# MC99b
# 4357fcpagbe_p402 b0 - 2/24/2016 from Tan Lu
##########################################
UTF::DHD STA1 \
        -lan_ip mc99tst11 \
        -sta {sta1 eth0} \
        -hostconsole "mc99end1:40011" \
        -power {npc94 2} \
        -dhd_tag DHD_BRANCH_1_579 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -tag IGUANA_BRANCH_13_10 \
	-brand linux-internal-dongle-pcie \
        -nvram "src/shared/nvram/bcm94357fcpagbe_p402.txt" \
        -nvram_add "macaddr=00:90:4c:12:d0:01" \
	-clm_blob 4357a0.clm_blob \
        -type 4357b0-roml/config_pcie_debug/rtecdc.bin \
        -tcpwindow 3m -udp 1g \
        -nocal 1 -nobighammer 1 \
        -slowassoc 5 \
        -datarate {-i 0.5 -auto -sgi} \
        -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0;}
sta1 clone sta1x2
sta1 clone sta1x1 -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta1 clone sta1x2r -type 4357b0-roml/config_pcie_release/rtecdc.bin


#########################################
# MC99b
# 4357fcpagbe_p402 b0 - 2/24/2016 from Tan Lu
#########################################
UTF::DHD STA2 \
        -lan_ip mc99tst12 \
        -sta {sta2 eth0} \
        -hostconsole "mc99end1:40012" \
        -power {npc94 1} \
        -dhd_tag DHD_BRANCH_1_579 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -tag IGUANA_BRANCH_13_10 \
        -brand linux-internal-dongle-pcie \
        -nvram "src/shared/nvram/bcm94357fcpagbe_p402.txt" \
        -nvram_add "macaddr=00:90:4c:12:d0:02" \
	-clm_blob 4357a0.clm_blob \
        -type 4357b0-roml/config_pcie_debug/rtecdc.bin \
        -tcpwindow 3m -udp 1g \
        -nocal 1 -nobighammer 1 \
        -slowassoc 5 \
        -datarate {-i 0.5 -auto -sgi} \
        -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0;}
sta2 clone sta2x2
sta2 clone sta2x1 -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta2 clone sta2x2r -type 4357b0-roml/config_pcie_release/rtecdc.bin


###
UTF::Q mc99b

# --------- end -----------

