#testbed configuration file for blr03end1
#Edited Rohit B Date24DEC2014
#Last checkin 03SEP2013 12AM
####### Controller section:
# blr01end1: FC15 
#
#
####### SOFTAP section:

# AP1:4360c
#
####### STA section:
# blr01tst1: 43217 eth0 (10.132.30.34)
# blr03softap: 4360 (10.132.30.36) 
######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix


UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr01end1" \
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
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr01"

#pointing Apps to trunk
#set ::UTF::TrunkApps 1 \

UTF::Linux blr01end1 \
     -lan_ip 10.131.80.47 \
    -sta {lan eth0}

UTF::Linux blr01softap -sta {4360c eth1} \
    -lan_ip 10.131.80.48 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth1; wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
         wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl country US ;wl ampdu_mpdu 24; wl assert_type 1;:
                     }



4360c configure -ipaddr 192.168.1.91 -ap 1 -attngrp G1 \




UTF::Linux blr01tst1 \
        -lan_ip 10.131.80.49 \
        -sta {43217 eth1} \
        -tcpwindow 4M \
        -power_button "auto" \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "BISON_BRANCH_7_10" \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
		-wlinitcmds {wl stbc_rx 1; wl stbc_tx 1} \


43217 configure -ipaddr 192.168.1.92 -attngrp G1 \

43217 clone 43217t -tag trunk \

43217 clone 43217b -tag BISON_BRANCH_7_10 \

43217 clone 43217BIS -tag  BISON04T_BRANCH_7_14 \
	-brand linux-mfgtest-wl \




UTF::Q blr01n
