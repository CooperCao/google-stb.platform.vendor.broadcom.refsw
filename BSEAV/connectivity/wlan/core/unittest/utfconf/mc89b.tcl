# -*-tcl-*-
#
# Testbed MC89 sub-rig configuration file
# Filename: mc89b.tcl
# Charles Chai 
# 01/19/2013: Initial setup
# 11/15/2016: Refurbished
# 01/09/2017: Add 4361b0 clients
#
# Hardware
#   MC89END1  : controller
#   MC89END3  : softap
#   MC89TST36 : STA
#   MC89TST37 : STA
#   MC89TST38 : STA
#   MC89TST39 : STA
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 8331 (0-95.5dB with 0.5 steps)
#
source "utfconf/mc89.tcl"

package require UTF::Linux
package require UTF::utils

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/mc89b"

# Set default to use wl from trunk; Use -app_tag to modify.
#set UTF::TrunkApps 1 ;# do not need it anymore as Tim set it as UTF default

# To enable ChannelSweep performance test
set UTF::ChannelPerf 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G3 attn 0

    # Make sure all systems are deinit and down 
    foreach S {4366softap-mu sta1 sta2 sta3 sta4} {
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
############################################
## softap
## 
############################################
UTF::Linux softap \
        -lan_ip mc89end3 \
        -sta {4366ap enp3s0} \
        -console "mc89end1:39999" \
        -power {npcap1 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
        -msgactions {"PHYTX error" FAIL} \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl atf 1;}

4366ap clone 4366softap-su
4366softap-su configure -ipaddr 192.168.1.145 -attngrp G3 -ap 1 -hasdhcpd 1
4366ap clone 4366softap-mu
4366softap-mu configure -ipaddr 192.168.1.145 -attngrp G3 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu}

# EAGLE_TWIG_10_10_132 
4366ap clone 4366softap-132-su -tag EAGLE_TWIG_10_10_132 
4366softap-132-su configure -ipaddr 192.168.1.145 -attngrp G3 -ap 1 -hasdhcpd 1
4366ap clone 4366softap-132-mu -tag EAGLE_TWIG_10_10_132 
4366softap-132-mu configure -ipaddr 192.168.1.145 -attngrp G3 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu}


########################################
# STA Section 
########################################
# MC89b
# 
########################################
UTF::DHD STA1 \
        -lan_ip mc89tst36 \
        -sta {sta1 eth0} \
        -hostconsole "mc89end1:40036" \
        -power {npctst1 1} \
        -dhd_tag DHD_BRANCH_1_579 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -tag IGUANA08T_BRANCH_13_35 \
        -brand hndrte-dongle-wl \
        -nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
        -nvram_add "macaddr=00:90:4c:12:d0:01" \
        -clm_blob ss_mimo.clm_blob \
        -type 4361b0-roml/config_pcie_utf_ipa/rtecdc.bin \
        -tcpwindow 3m -udp 1g \
        -nocal 1 -nobighammer 1 -slowassoc 5 \
        -datarate {-i 0.5 -auto} \
        -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0;}

sta1 clone sta1x1 -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta1 clone sta1x2
sta1 clone sta1x2r -type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
	-wlinitcmds {wl down; wl vht_features 7; wl wnm_bsstrans_resp 0;}

sta1 clone sta1x2rt -type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin -dhd_date 2017.1.20.0


UTF::DHD STA2 \
        -lan_ip mc89tst37 \
        -sta {sta2 eth0} \
        -hostconsole "mc89end1:40037" \
        -power {npctst1 2} \
        -dhd_tag DHD_BRANCH_1_579 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -tag IGUANA08T_BRANCH_13_35 \
        -brand hndrte-dongle-wl \
        -nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
        -nvram_add "macaddr=00:90:4c:12:d0:02" \
        -clm_blob ss_mimo.clm_blob \
        -type 4361b0-roml/config_pcie_utf_ipa/rtecdc.bin \
        -tcpwindow 3m -udp 1g \
        -nocal 1 -nobighammer 1 -slowassoc 5 \
        -datarate {-i 0.5 -auto} \
        -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0;}

sta2 clone sta2x1 -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta2 clone sta2x2
sta2 clone sta2x2r -type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin

sta2 clone sta2x2rt -type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin -dhd_date 2017.1.19.0

UTF::DHD STA3 \
        -lan_ip mc89tst38 \
        -sta {sta3 eth0} \
        -hostconsole "mc89end1:40038" \
        -power {npcap2 1} \
        -dhd_tag DHD_BRANCH_1_579 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -tag IGUANA08T_BRANCH_13_35 \
        -brand hndrte-dongle-wl \
        -nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
        -nvram_add "macaddr=00:90:4c:12:d0:03" \
        -clm_blob ss_mimo.clm_blob \
        -type 4361b0-roml/config_pcie_utf_ipa/rtecdc.bin \
        -tcpwindow 3m -udp 1g \
        -nocal 1 -nobighammer 1 -slowassoc 5 \
        -datarate {-i 0.5 -auto} \
        -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0;}

sta3 clone sta3x1 -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta3 clone sta3x2
sta3 clone sta3x2r -type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin


UTF::DHD STA4 \
        -lan_ip mc89tst39 \
        -sta {sta4 eth0} \
        -hostconsole "mc89end1:40039" \
        -power {npcap1 1} \
        -dhd_tag DHD_BRANCH_1_579 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -tag IGUANA08T_BRANCH_13_35 \
        -brand hndrte-dongle-wl \
        -nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
        -nvram_add "macaddr=00:90:4c:12:d0:04" \
        -clm_blob ss_mimo.clm_blob \
        -type 4361b0-roml/config_pcie_utf_ipa/rtecdc.bin \
        -tcpwindow 3m -udp 1g \
        -nocal 1 -nobighammer 1 -slowassoc 5 \
        -datarate {-i 0.5 -auto} \
        -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0;}

sta4 clone sta4x1 -wlinitcmds {wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}
sta4 clone sta4x2
sta4 clone sta4x2r -type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin


###
UTF::Q mc89b


