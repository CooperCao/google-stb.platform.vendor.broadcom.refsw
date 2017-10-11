#testbed configuration file for blr03end1
#Edited Rohit B Date 2 Feb 2014
#Last checkin 02 Feb 2014
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


UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr03end1" \
        -group {
                G1 {1 2 3}
		ALL {1 2 3}
                }

G1 configure -default 20

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    
    # Make Sure Attenuators are set to 0 value
	
	foreach S {7252/4360 7252/43217} {
	catch {$S wl down}
	$S deinit
    }
	G1 attn default 
    return
	
}

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr03"

#pointing Apps to trunk
#set ::UTF::TrunkApps 1 \

set ::UTF::ChannelPerf 1 \

UTF::Linux blr03end1 \
     -lan_ip 10.132.116.30 \
    -sta {lan em1}

##

package require UTF::STB
	


UTF::STB 7252 -sta {7252/4360 eth1} \
	-lan_ip 10.132.117.107 \
    -console "blr03end1.ban.broadcom.com:40001" \
    -power_button "auto" \
    -reloadoncrash 1 \
    -brand linux-internal-media \
    -dhd_tag "BISON04T_REL_7_14_124_66" \
	-app_tag "BISON04T_REL_7_14_124_66" \
    -type "debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-wet-stb-armv7l-slvradar" \
	-dongleimage "wl.ko" \
    -datarate {-i 0.5 } \
	-tcpwindow 2m -udp 800m \
	-wlinitcmds {wl down; wl vht_features 3} \
    -slowassoc 5 \
    -perfchans {36/80 3 161/80} \
	-post_perf_hook {{%S wl channel} {%S wl chanspec} {%S wl dump rssi}} \
    -pre_perf_hook {{%S wl channel} {%S wl chanspec} {%S wl dump rssi}} \
	-yart {-attn2g 20-95} \
	-modopts {secdma_addr=0x80000000 secdma_size=0x880000} \

    
	
7252/4360 configure -ipaddr 192.168.1.101

7252/4360 clone 7252/4360ap \
	-ap 1 \
	-sta {7252/4360ap eth1} \
	
7252/4360 clone 7252/4360b124 \
     -dhd_tag "BISON04T_TWIG_7_14_124" \
	 -type debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-wet-11k-stb-armv7l-slvradar \

	 
7252/4360b124 clone 7252/4360b124ap \
	-ap 1 \
	-sta {7252/4360b124ap eth1} \
	

7252/4360b124 clone 7252/4360_REL \
    -dhd_tag "BISON04T_REL_7_14_124_89" -date 2016.12.2.0 \
	-brand linux-external-media  \
	-type nodebug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-wet-11k-stb-armv7l-slvradar \


###

UTF::STB 7252a -sta {7252/43217 eth1} \
	-lan_ip 10.132.117.102 \
    -console "blr03end1.ban.broadcom.com:40000" \
    -power_button "auto" \
    -reloadoncrash 1 \
    -brand linux-internal-media \
    -dhd_tag "BISON04T_REL_7_14_124_66" \
    -type "debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-wet-stb-armv7l-slvradar" \
	-dongleimage "wl.ko" \
    -datarate {-i 0.5} \
    -tcpwindow 1m -udp 250m -slowassoc 5 \
    -perfchans {3l 3} \
	-post_perf_hook {{%S wl channel} {%S wl chanspec}} \
    -pre_perf_hook {{%S wl channel} {%S wl chanspec}} \
	-yart {-attn2g 20-95} \
    -modopts {secdma_addr=0x80000000 secdma_size=0x880000} \
	

7252/43217 configure -ipaddr 192.168.1.102

7252/43217 clone 7252/43217ap \
	-ap 1 \
	-sta {7252/43217ap eth1} \
	-modopts {secdma_addr=0x80000000 secdma_size=0x880000} \

7252/43217 clone 7252/43217sec \
     -modopts {secdma_addr=0x80000000 secdma_size=0x880000} \
	 
7252/43217sec clone 7252/43217b124 \
     -dhd_tag "BISON04T_TWIG_7_14_124" \
	 -type debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-wet-11k-stb-armv7l-slvradar \
	 -modopts {secdma_addr=0x80000000 secdma_size=0x880000} \

	 	
7252/43217b124 clone 7252/43217b124ap \
	-ap 1 \
	-sta {7252/43217b124ap eth1} \
	-modopts {secdma_addr=0x80000000 secdma_size=0x880000} \
	
7252/43217 clone 7252/43217nsec \
   -type debug-apdef-stadef-stb-armv7l \
   
7252/43217 clone 7252/43217_14 \
   -dhd_tag "BISON04T_BRANCH_7_14" \
   -type "debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-armv7l" \
   -modopts {secdma_addr=0x80000000 secdma_size=0x880000} \
   

7252/43217 clone 7252/43217_74 \
   -dhd_tag "BISON04T_REL_7_14_124_74" \
   -type debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-wet-11k-stb-armv7l-slvradar \
   -modopts {secdma_addr=0x80000000 secdma_size=0x880000} \
   


   
##
   
# UTF::Linux blr03tst2 \
        # -lan_ip 10.132.116.33 \
        # -sta {4366 eth1} \
        # -power_button "auto" \
        # -slowassoc 5 -reloadoncrash 1 \
        # -nobighammer 1 \
		# -tag EAGLE_BRANCH_10_10 \
	    # -datarate {-i 0.5 -frameburst 1} \
        # -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl counters} {%S wl vht_features}} \
        # -pre_perf_hook {{%S wl rate} {%S wl nrate} {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl vht_features}} \
		# -wlinitcmds {wl txchain 3; wl rxchain 3; wl down; wl vht_features 7} \
	    # -perfchans {36/80 3} \
	    # -tcpwindow 4M \
	    # -udp 1.7g \


# -preinstall_hook {{%S dmesg -n 7}} \	  
	 
# 4366 configure -ipaddr 192.168.1.103 -attngrp G1 \


# 4366 clone 4366sta \
	# -ap 0 \
	# -yart {-attn5g 30-95 -attn2g 30-95} \
	# -tcpwindow 4m -udp 1.7g \
	# -perfchans {3} \
	
# 4366 clone 4366a \
     # -wlinitcmds {wl txchain 7; wl rxchain 7; wl down; wl vht_features 7} \

# 4366a clone 4366_sta \
    # -ap 0 \
	# -yart {-attn5g 30-95 -attn2g 30-95} \
	# -tcpwindow 4m -udp 1.7g \
	# -perfchans {36/80 3} \


   
UTF::Linux blr03tst2 \
    -lan_ip 10.132.116.33 \
	  -sta {4360 eth1} \
    -power_button "auto" \
    -reloadoncrash 1 \
    -datarate {-i 0.5 } \
	-tcpwindow 2m -udp 800m \
	-wlinitcmds {wl down; wl vht_features 3} \
    -slowassoc 5 \
    -perfchans {36/80 3 161/80} \
	-post_perf_hook {{%S wl channel} {%S wl chanspec} {%S wl dump rssi}} \
    -pre_perf_hook {{%S wl channel} {%S wl chanspec} {%S wl dump rssi}} \
	-yart {-attn2g 20-95} \
	

4360 configure -ipaddr 192.168.1.101

4360 clone 4360ap \
	-ap 1 \
	-sta {4360ap eth1} \
	-wlinitcmds {wl txchain 3; wl rxchain 3; wl down; wl vht_features 3} \
	
4360 clone 4360sta \
     -tag "BISON04T_TWIG_7_14_124" \
	 -wlinitcmds {wl txchain 3; wl rxchain 3; wl down; wl vht_features 3} \
	 
4360ap clone 4360_ap \
	-sta {4360_ap eth1} \
	-wlinitcmds {wl txchain 7; wl rxchain 7; wl down; wl vht_features 3} \

4360_ap clone 4360_sta \
    -tag "BISON04T_TWIG_7_14_124" \
	 

UTF::Q blr03
