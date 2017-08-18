# -*-tcl-*-
# MuMIMO Testbed configuration file for BLR27
# Created by Rohit B on 11-09-2016
# Edited by Rohit B on 11-09-2016



# Load Packages
package require UTF::Aeroflex
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix
package require UTF::doc

set UTF::ChannelPerf 1
set UTF::Use11 1

# Define power controllers on cart.
package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr27end1 -rev 1

#Load drivers from SJ
UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

#Summary Directory
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr27"

set UTF::TrunkApps 1


UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr27end1" \
        -group {
                G1 {1 2}
                G2 {3 4}
				G3 {5 6}
                G4 {7 8}
				G5 {1 2 3 4 5 6 7 8}
				ALL {1 2 3 4 5 6 7 8}
				
               }
G1 configure -default 0
G2 configure -default 0
G3 configure -default 0
G4 configure -default 0
G5 configure -default 0
ALL configure -default 0




# Default TestBed Configuration Options
set ::UTF::SetupTestBed {

	ALL attn default

	set ::UTF::APList "4366softap"
	set ::UTF::STAList "4361a 4361b 4361c 4361d"
	set ::UTF::DownList "4366softap $::UTF::APList $::UTF::STAList"

    # Make sure APs are on before testing
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
   
    foreach AP "$::UTF::APList" {
            UTF::Try "$AP Radio Down" {
                        catch {$AP power on}
                UTF::Sleep 5
                        catch {$AP restart wl0_radio=0}
                        catch {$AP restart wl1_radio=0}
                        catch {$AP deinit}
                }
    }
    # unset AP so it doesn't interfere
    unset AP  

    # To prevent interference.
    foreach S "$::UTF::DownList" {
            UTF::Try "$S Down" {
                    catch {$S wl down}
                    catch {$S deinit}
            }
    }
    # unset S so it doesn't interfere
    unset S  
    
    # delete myself
    unset ::UTF::SetupTestBed
    
    return
}




####################### CONTROLLER ######################

UTF::Linux blr27end1 \
    -lan_ip 10.132.116.211 \
    -sta {lan em1} \
    -power_button "auto" \

	 
	 
###################### SOFTAP ###########################

UTF::Linux blr27softap \
	-lan_ip 10.132.116.212 \
    -sta {4366softap enp1s0} \
    -power_button "auto" \
    -brand linux-internal-wl \
	-tag EAGLE_BRANCH_10_10 \
	-reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7;} \

4366softap configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -hasdhcpd 1 \

4366softap clone 4366softap-mutx0
4366softap-mutx0 configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -rvrnightly {-mumode su} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl bw_cap 2g -1; wl country '#a/0'; wl vht_features 7;}

4366softap clone 4366atf-mutx0 
4366atf-mutx0 configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -rvrnightly {-mumode su} \
	-modopts {assert_type=1 nompc=1 ctdma=0} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl bw_cap 2g -1; wl country US/0; wl vht_features 7; wl atf 1} \
	
4366softap clone 4366softap-mutx1
4366softap-mutx1 configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -hasdhcpd 1 -rvrnightly {-mumode mu} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl bw_cap 2g -1; wl country '#a/0'; wl vht_features 7; wl atf 0} \
	-post_perf_hook {{%S wl txchain}} \

4366softap clone 4366atf-mutx1
4366atf-mutx1 configure -ipaddr 192.168.1.125 -attngrp G5 -ap 1 -rvrnightly {-mumode mu} \
	-modopts {assert_type=1 nompc=1 ctdma=1} \
	-wlinitcmds {wl msglevel +assoc +mumimo; wl dtim 3; wl down; wl bw_cap 2g -1; wl country US/0; wl vht_features 7; wl atf 1} \
	-post_perf_hook {{%S wl txchain}} 


#UTF::Linux blr27tst1 \
#    -lan_ip 10.132.116.213 \
#    -sta {4366aa enp1s0} \
#    -power_button "auto" \
#    -tag EAGLE_BRANCH_10_10 \
#	-brand linux-internal-wl \
#    -tcpwindow 2m -udp 1.5g \
#    -nocal 1 -slowassoc 10 \
#    -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl reset_cnts}} \
#    -post_perf_hook {{%S wl dump rssi} {%S wl counters} {%S wl dump ampdu}} \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0;} \
#
#4366aa configure -attngrp G5 -ipaddr 192.168.1.126 \
#
##for 1x1
#4366aa clone 4366aa1x1 \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl infra 1; wl txchain 1; wl rxchain 1;}
#
##mimo mode
#4366aa clone 4366aa2x2 \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl infra 1; wl txchain 3; wl rxchain 3;}
#
##for Single User MIMO
#4366aa clone 4366aa1x1su \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl infra 1; wl txchain 1; wl rxchain 1; wl txbf_bfe_cap 1;}


#UTF::Linux blr27tst2 \
#    -lan_ip 10.132.116.214 \
#    -sta {4366ab enp1s0} \
#    -power_button "auto" \
#    -tag EAGLE_BRANCH_10_10 \
#	-brand linux-internal-wl \
#    -tcpwindow 2m -udp 1.5g \
#    -nocal 1 -slowassoc 10 \
#    -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl reset_cnts}} \
#    -post_perf_hook {{%S wl dump rssi} {%S wl counters} {%S wl dump ampdu}} \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0;} \
#
#4366ab configure -attngrp G5 -ipaddr 192.168.1.127 \
#
##for 1x1
#4366ab clone 4366ab1x1 \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl infra 1; wl txchain 1; wl rxchain 1;}
#
##mimo mode
#4366ab clone 4366ab2x2 \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl infra 1; wl txchain 3; wl rxchain 3;}
#
##for Single User MIMO
#4366ab clone 4366ab1x1su \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl infra 1; wl txchain 1; wl rxchain 1; wl txbf_bfe_cap 1;}




#UTF::Linux blr27tst3 \
#    -lan_ip 10.132.116.215 \
#    -sta {4366ac enp1s0} \
#    -power_button "auto" \
#    -tag EAGLE_BRANCH_10_10 \
#	-brand linux-internal-wl \
#    -tcpwindow 2m -udp 1.5g \
#    -nocal 1 -slowassoc 10 \
#    -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl reset_cnts}} \
#    -post_perf_hook {{%S wl dump rssi} {%S wl counters} {%S wl dump ampdu}} \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0;} \
#
#4366ac configure -attngrp G5 -ipaddr 192.168.1.128 \
#
##for 1x1
#4366ac clone 4366ac1x1 \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl infra 1; wl txchain 1; wl rxchain 1;}
#
##mimo mode
#4366ac clone 4366ac2x2 \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl infra 1; wl txchain 3; wl rxchain 3;}
#
##for Single User MIMO
#4366ac clone 4366ac1x1su \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl infra 1; wl txchain 1; wl rxchain 1; wl txbf_bfe_cap 1;}




#UTF::Linux blr24softap \
#    -lan_ip 10.132.116.199 \
#    -sta {4366ad enp1s0} \
#    -power_button "auto" \
#    -tag EAGLE_BRANCH_10_10 \
#	-brand linux-internal-wl \
#    -tcpwindow 2m -udp 1.5g \
#    -nocal 1 -slowassoc 10 \
#    -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl reset_cnts}} \
#    -post_perf_hook {{%S wl dump rssi} {%S wl counters} {%S wl dump ampdu}} \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0;} \
#
#4366ad configure -attngrp G5 -ipaddr 192.168.1.129 \
#
##for 1x1
#4366ad clone 4366ad1x1 \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl infra 1; wl txchain 1; wl rxchain 1;}
#
##mimo mode
#4366ad clone 4366ad2x2 \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl txbf_bfr_cap 0; wl infra 1; wl txchain 3; wl rxchain 3;}
#
##for Single User MIMO
#4366ad clone 4366ad1x1su \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 7; wl infra 1; wl txchain 1; wl rxchain 1; wl txbf_bfe_cap 1;}



UTF::DHD blr26tst1 \
	-lan_ip 10.132.116.194 \
	-sta {4361a eth0} \
	-power_button "auto" \
	-dhd_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-tag IGUANA08T_BRANCH_13_35 \
	-type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
	-brand hndrte-dongle-wl \
	-app_tag trunk \
	-nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
	-tcpwindow 2m -udp 1.5g \
	-nocal 1 -slowassoc 8 \
	-clm_blob ss_gs8.clm_blob \
	-pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl reset_cnts} {%S wl counters} {%S wl dump ampdu} } \
	-post_perf_hook  {{%S wl counters} {%S wl dump ampdu}} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7;} \

#for 1x1
4361a clone 4361a1x1 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1;  wl vht_features 7; wl txchain 1; wl rxchain 1; wl txbf_bfr_cap 0;}

#mimo mode
4361a clone 4361a2x2 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3; wl txbf_bfr_cap 0;}

#for Single User MIMO
4361a clone 4361a1x1su \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 1; wl rxchain 1; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1;}




########################## STA 2 #############################

UTF::DHD blr26tst2 \
	-lan_ip 10.132.116.195 \
	-sta {4361b eth0} \
	-power_button "auto" \
	-dhd_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-tag IGUANA08T_BRANCH_13_35 \
	-app_tag trunk \
	-type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
	-brand hndrte-dongle-wl \
	-nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt"  \
	-tcpwindow 2m -udp 1.5g \
	-nocal 1 -slowassoc 8 \
	-clm_blob ss_gs8.clm_blob \
	-pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl reset_cnts} {%S wl counters} {%S wl dump ampdu}} \
	-post_perf_hook  {{%S wl counters} {%S wl dump ampdu}} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7;} \

4361b clone 4361b1x1 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 1; wl rxchain 1; wl txbf_bfr_cap 0;}

4361b clone 4361b2x2 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3; wl txbf_bfr_cap 0;}

#for Single User MIMO
4361b clone 4361b1x1su \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 1; wl rxchain 1; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1;}





############################ STA 3 ########################

UTF::DHD blr26tst3 \
	-lan_ip 10.132.116.196 \
	-sta {4361c eth0} \
	-power_button "auto" \
	-dhd_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-tag IGUANA08T_BRANCH_13_35 \
	-type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
	-brand hndrte-dongle-wl \
	-app_tag trunk \
	-nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
	-tcpwindow 2m -udp 1.5g \
	-clm_blob ss_gs8.clm_blob \
	-nocal 1 -slowassoc 8 \
	-pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl reset_cnts} {%S wl counters} {%S wl dump ampdu}} \
	-post_perf_hook  {{%S wl counters} {%S wl dump ampdu}} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7;} \

4361c clone 4361c1x1 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 1; wl rxchain 1; wl txbf_bfr_cap 0;}

4361c clone 4361c2x2 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3; wl txbf_bfr_cap 0;}

#for Single User MIMO
4361c clone 4361c1x1su \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 1; wl rxchain 1; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1;}





########################### STA 4 #########################

UTF::DHD blr26tst4 \
	-lan_ip 10.132.116.197 \
	-sta {4361d eth0} \
	-power_button "auto" \
	-dhd_tag DHD_BRANCH_1_579 \
	-tag IGUANA08T_BRANCH_13_35 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
	-brand hndrte-dongle-wl \
	-app_tag trunk \
	-nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
	-tcpwindow 2m -udp 1.5g \
	-nocal 1 -slowassoc 8 \
	-clm_blob ss_gs8.clm_blob \
	-pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl reset_cnts} {%S wl counters} {%S wl dump ampdu}} \
	-post_perf_hook  {{%S wl counters} {%S wl dump ampdu}} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7;} \

4361d clone 4361d1x1 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 1; wl rxchain 1; wl txbf_bfr_cap 0;}

4361d clone 4361d2x2 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 3; wl rxchain 3; wl txbf_bfr_cap 0;}

# for Single User MIMO
4361d clone 4361d1x1su \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txchain 1; wl rxchain 1; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1;} \


UTF::Q blr26
