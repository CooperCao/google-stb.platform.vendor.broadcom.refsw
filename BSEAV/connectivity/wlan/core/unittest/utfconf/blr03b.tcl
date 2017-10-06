#testbed configuration file for blr03end1
#Edited Vishnu Prasad V Date 8 July 2015
#Last checkin 08 july 2015
####### Controller section:
# blr03end1: FC15 
#
#
####### SOFTAP section:

# AP1:4360c
#
####### STA section:
#
# blr03tst1: 43224 eth0 (10.132.30.32)
# blr03tst2: 43217 eth0 (10.132.30.33)
# blr03tst3: 43228 eth0 (10.132.30.34)
# blr03tst4: 4331  eth0 (10.132.30.35)
# blr03softap: 4360 (10.132.30.36) 
######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix


UTF::Aeroflex af -lan_ip 172.1.1.32 \
        -relay "blr03end1" \
        -group {
                G1 {1 2 3}
                G2 {4 5 6}
                G3 {7 8 9}
		ALL {1 2 3 4 5 6 7 8 9}
                }
#G1 configure default 0
#G2 configure default 0
#G3 configure default 0
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G3 attn 0
    catch {G3 attn 0;}
    catch {G2 attn 0;}
    catch {G1 attn 0;}
}
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr03"

#pointing Apps to trunk
#set ::UTF::TrunkApps 1 \

UTF::Linux blr03end1 \
     -lan_ip 10.131.80.52 \
    -sta {lan eth0}

UTF::Linux blr03softap -sta {4360c eth1} \
    -lan_ip 10.131.80.53 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag "BISON05T_BRANCH_7_35" \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \-
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}  {%S wl scansuppress 1} {%S wl ver}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl ver}} \
    -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth1; wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
         wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl ampdu_mpdu 24; wl ver; 
    }

4360c configure -ipaddr 192.168.1.91 -ap 1 -attngrp G3 -hasdhcpd 1 \



UTF::Linux blr03tst3 \
        -lan_ip 10.131.80.56 \
        -sta {43228 eth0} \
        -tcpwindow 4M \
        -power_button "auto" \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "BISON05T_BRANCH_7_35" \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl dump phycal}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
		-wlinitcmds {wl down; wl chanspec 36} \

43228 configure -ipaddr 192.168.1.94 \


43228 clone 43228BIS -tag BISON04T_BRANCH_7_14 \


43228 clone 43228t -tag trunk \
	-type debug-p2p-mchan \

43228 clone 43228b -tag BISON05T_BRANCH_7_35 \
	-brand linux-internal-wl \

43228 clone 43228br -tag BISON04T_REL_7_14_78 \
	-brand linux26-internal-router \
	
43228 clone 43228br2 -tag BISON04T_REL_7_14_89_2 \
    -brand linux-external-dongle-usb-media \

43228 clone 43228s -tag DINGO_BRANCH_9_10

43228 clone 43228u -tag EAGLE_BRANCH_10_10 
	
UTF::Linux blr03tst2 \
         -lan_ip 10.131.80.55 \
         -sta {43224 eth1} \
         -tcpwindow 4M \
         -power_button "auto" \
         -slowassoc 5 -reloadoncrash 1 \
         -nobighammer 1 \
         -brand linux-internal-wl \
         -tag "BISON05T_BRANCH_7_35" \
         -wlinitcmds {wl msglevel 0x101 ; wl chanspec 36} \
         -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
         -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
         

 43224 configure -ipaddr 192.168.1.92 \

 43224 clone 43224t -tag trunk \

 43224 clone 43224b -tag BISON05T_BRANCH_7_35 \

 43224 clone 43224BIS -tag BISON04T_BRANCH_7_14 \
	 -brand linux-mfgtest-wl \
	 
 43224 clone 43224s -tag DINGO_BRANCH_9_10

 43224 clone 43224u -tag EAGLE_BRANCH_10_10 	 
   
UTF::Linux blr03tst1 \
        -lan_ip 10.131.80.54 \
        -sta {43217 eth1} \
        -tcpwindow 4M \
        -power_button "auto" \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -tag "BISON05T_BRANCH_7_35" \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
		-wlinitcmds {wl stbc_rx 1; wl stbc_tx 1} \


43217 configure -ipaddr 192.168.1.93 \

43217 clone 43217t -tag trunk \

43217 clone 43217b -tag BISON05T_BRANCH_7_35 \

43217 clone 43217BIS -tag  BISON04T_BRANCH_7_14 \
	-brand linux-mfgtest-wl \

43217 clone 43217v -tag DINGO_BRANCH_9_10

43217 clone 43217w -tag EAGLE_BRANCH_10_10 	


UTF::Linux blr03tst4 \
        -lan_ip 10.131.80.57 \
        -sta {4331 eth1} \
        -tcpwindow 4M \
        -power_button "auto" \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "BISON05T_BRANCH_7_35" \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \

4331 configure -ipaddr 192.168.1.95 \

4331 clone 4331t  -tag trunk \

4331 clone 4331b -tag BISON05T_BRANCH_7_35 \

4331 clone 4331BIS -tag BISON04T_BRANCH_7_14 \
	-brand linux-mfgtest-wl \

4331 clone 4331s -tag DINGO_BRANCH_9_10

4331 clone 4331u -tag EAGLE_BRANCH_10_10 
	



UTF::Q blr01n
