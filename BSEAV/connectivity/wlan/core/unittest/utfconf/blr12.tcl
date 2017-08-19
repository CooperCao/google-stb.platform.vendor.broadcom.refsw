#Testbed configuration file for blr012end1 UTF StaNighly Teststation
#Created by Sumesh Nair on 02 APRIL 2015 10:00 PM  
#
####### Controller section:
# blr12end1: FC19
# IP ADDR 10.132.116.93
# NETMASK 255.255.254.0 
# GATEWAY 10.132.116.1
#
####### SOFTAP section:


#
# blr05ref0 : FC 15 4360mc_1(99)  10.131.80.135 
# blr05ref1 : FC 15 4360mc_1(99)  10.131.80.136
#
####### STA section:
#
# blr12tst1: FC 15       eth0 (10.131.80.137)
# blr12tst2: FC 15       eth0 (10.131.80.138)
# blr12tst3: FC 15       eth0 (10.131.80.139)
# blr12tst4: FC 15 4355  eth0 (10.131.80.140)
##########################################################
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

################### Aeroflex attenuator ###################


UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr12end1" \
        -group {
                G1 {1 2 3 4}
				ALL {1 2 3 4}
		       }
				
				
				
G1 configure -default 0
# Default TestBed Configuration Options


set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
	#
	foreach S {4366a 4366b 4366c 4360} {
	catch {$S wl down}
	$S deinit
    }
	G1 attn default 
    return
	
}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/gurraman/blr12"




#pointing Apps to trunk

set ::UTF::TrunkApps 1 \

set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \




####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr12end1 -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr12end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr12end1 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.41 -relay blr12end1 -rev 1
UTF::Power::Synaccess npc51 -lan_ip 172.1.1.51 -relay blr12end1 -rev 1

########################### Test Manager ################

UTF::Linux blr12end1 \
     -lan_ip 10.132.116.93 \
     -sta {lan p5p1}


#############################
#
# AtlasII
#
#############################

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck


UTF::Linux blr12aphost2 \
    -sta {lan2 p1p1} \
    -lan_ip 10.132.116.184 \

lan2 configure -ipaddr 192.168.1.250
	
	
	
UTF::Linux blr12aphost1 \
    -sta {lan3 p1p1} \
    -lan_ip 10.132.116.183 \

lan3 configure -ipaddr 192.168.1.250

UTF::Linux blr07tst1 \
   -sta {lan4 p1p1} \
   -lan_ip 10.132.116.61 \
   
lan4 configure -ipaddr 192.168.1.210

	
UTF::Router atlasII -sta {atlasII/0 eth1 atlasII/0.%15 wl0.%  atlasII/1 eth2 atlasII/1.%15 wl1.%  atlasII/2 eth3 atlasII/2.%15 wl2.%} \
    -tag BISON04T_BRANCH_7_14 \
	-lan_ip 192.168.1.1 \
    -relay blr12end1 \
    -lanpeer {lan4 lan3} \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -console "blr12end1.ban.broadcom.com:40000" \
    -power {npc51 1} \
    -nvram {
	watchdog=2000
	lan_stp=0
	lan1_stp=0
	wl0_ssid=atlasII/0
	wl0_chanspec=11
	wl0_radio=0
	wl0_vht_features=5
	wl1_ssid=atlasII/1
	wl1_chanspec=36
	wl1_radio=0
	wl1_vht_features=6
	wl2_ssid=atlasII/2
	wl2_chanspec=161
	wl2_radio=0
	wl2_vht_features=6
    # wl2_bw_cap=15
    # wl2_country_code=Q1
	# wl2_country_rev=139
	samba_mode=2
    } \
    -model bcm94709acdcrh_p503_nvram \
    -datarate {-i 0.5} -udp 1.8g \
    -yart {-attn5g 2-93 -attn2g 18-93 -pad 21} \
    -noradio_pwrsave 1 -nosamba 1 \
	-pre_perf_hook {{%S wl vht_features}} \
	-post_perf_hook {{%S wl vht_features}} \
	

atlasII/0  configure -attngrp G1
atlasII/1  configure -attngrp G1
atlasII/2  configure -attngrp G1


atlasII/0 clone atlasIIb14/0 -tag BISON04T_BRANCH_7_14 \
   -perfchans {3l}
   
atlasII/1 clone atlasIIb14/1 -tag BISON04T_BRANCH_7_14 \
    -perfchans {36/80} -channelsweep {-max 64}

atlasII/2 clone atlasIIb14/2 -tag BISON04T_BRANCH_7_14 \
    -perfchans {161/80} -channelsweep {-min 100} \

atlasII/0  clone atlasIIb164/0 -tag BISON04T_TWIG_7_14_164 \
    -perfchans {3l}

atlasII/1  clone atlasIIb164/1 -tag BISON04T_TWIG_7_14_164 \
    -perfchans {36/80} -channelsweep {-max 64}

atlasII/2  clone atlasIIb164/2 -tag BISON04T_TWIG_7_14_164 \
    -perfchans {161/80} -channelsweep {-min 100}

atlasII/0  clone atlasIIb170/0 -tag BISON04T_TWIG_7_14_170 \
    -perfchans {3l}

atlasII/1  clone atlasIIb170/1 -tag BISON04T_TWIG_7_14_170 \
    -perfchans {36/80} -channelsweep {-max 64}

atlasII/2  clone atlasIIb170/2 -tag BISON04T_TWIG_7_14_170 \
    -perfchans {161/80} -channelsweep {-min 100}

set xargs {
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
    -perfonly 1 -datarate 0 -docpu 1 -nosamba 0
}
lappend xargs -nvram "[atlasII/2 cget -nvram]
    wl2_country_code=Q1
    wl2_country_rev=137
"

atlasII/0 clone atlasIIx/0 {*}$xargs
atlasII/1 clone atlasIIx/1 {*}$xargs
atlasII/2 clone atlasIIx/2 {*}$xargs

atlasIIx/2 configure \
    -dualband {atlasIIx/1 -c2 36/80 -c1 161/80 -lan1 lan -lan2 lan3} \
	-sta {atlasIIx/2 eth3 atlasIIx/2.%15 wl2.%} \
    -perfchans {161/80 128/160} -notkip 1 -nowep 1 \

atlasIIb164/0 clone atlasIIb164x/0 {*}$xargs
atlasIIb164/1 clone atlasIIb164x/1 {*}$xargs
atlasIIb164/2 clone atlasIIb164x/2 {*}$xargs

atlasIIb164x/2 configure \
    -dualband {atlasIIb164x/1 -c2 36/80 -c1 161/80 -lan1 lan -lan2 lan3} \
	-sta {atlasIIb164x/2 eth3 atlasIIb164x/2.%15 wl2.%} \
    -perfchans {161/80 128/160} -notkip 1 -nowep 1 \


####


# atlasIIb164/0 clone atlasIIb164x/0 {*}$xargs
# atlasIIb164/1 clone atlasIIb164x/1 {*}$xargs
# atlasIIb164/2 clone atlasIIb164x/2 {*}$xargs

# atlasIIb164x/0 configure \
    # -dualband {atlasIIb164x/2 -c1 36/80 -c2 161/80 -lan1 lan -lan2 lan3}

	
# Clone for 160
# atlasII/2 clone atlasII/2+ -sta {atlasII/2+ eth3 atlasII/2+.%15 wl2.%} \
    # -perfchans {128/160} -channelsweep {-min 100 -bw 160 -chanspec {
	# 100/160 104/160 108/160 112/160 116/160 120/160 124/160 128/160
	# 100/80 161/80
    # }} -notkip 1 -nowep 1 \
    # -nvram [concat [atlasII/2 cget -nvram] 2:ccode=Q1 2:regrev=139]


# atlasII/2 clone atlasIIb160/2 \
    # -perfchans {36/160} \
	# lappend xargs -nvram "[atlasII/1 cget -nvram]
    # wl1_country_code=Q1
    # wl1_country_rev=139"	

# atlasII/2 clone atlasIIb160/2 \
    # -perfchans {128/160} \
	# lappend xargs -nvram "[atlasII/2 cget -nvram]
    # wl2_country_code=Q1
    # wl2_country_rev=139"
	

	


#############################
#
# 4709/4366+43602
#
#############################


UTF::Router 4709 -sta {4709/43602mch2 eth1 4709/43602mch2.%15 wl0.% 4709/4366mc eth2 4709/4366mc.%15 wl1.%} \
     -lan_ip 192.168.1.2 \
     -relay blr12end1 \
     -lanpeer {lan lan2} \
     -power {npc11 2} \
	 -tag BISON04T_BRANCH_7_14 \
     -console "blr12end1.ban.broadcom.com:40001" \
     -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
     -nvram {
	watchdog=2000
	wl0_ssid=4709/43602mch2
	wl0_chanspec=3
	wl0_radio=1
	wl0_atf=0
	wl0_vht_features=5
	wl1_ssid=4709/4366mc
	wl1_chanspec=36
	wl1_radio=1
	wl1_atf=0
	wl1_vht_features=6
	   } \
	-udp 1.8g \
	-datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g {15-90} -attn2g {35-90} -pad 29} \
	-wlinitcmds {sleep 5; wl -i eth2 ver; wl -i eth1 ver;} \
    -pre_perf_hook {{%S wl vht_features}} \
	-post_perf_hook {{%S wl vht_features}} \
    -noradio_pwrsave 1 \


4709/43602mch2 configure -attngrp G1
4709/4366mc configure -attngrp G1


set atlas [list \
	       -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
	       -nvram "[4709/4366mc cget -nvram]
	{fwd_cpumap=d:u:5:163:0 d:x:2:169:1 d:l:5:169:1}
	gmac3_enable=1"]
	

4709/43602mch2 clone 4709b14/43602mch2 -tag BISON04T_BRANCH_7_14 \
   -perfchans {3l}
4709/4366mc clone 4709b14/4366mc -tag BISON04T_BRANCH_7_14 \
   -perfchans {36/80}


4709/43602mch2 clone 4709b164/43602mch2 -tag BISON04T_TWIG_7_14_164 \
      -perfchans {3l 3}

4709/4366mc clone 4709b164/4366mc -tag BISON04T_TWIG_7_14_164 \
      -perfchans {36/80}

4709/43602mch2 clone 4709b170/43602mch2 -tag BISON04T_TWIG_7_14_170 \
      -perfchans {3l 3}

4709/4366mc clone 4709b170/4366mc -tag BISON04T_TWIG_7_14_170 \
      -perfchans {36/80}

set xdargs {
    -brand linux-2.6.36-arm-external-vista-router-dhdap-full-src
    -perfonly 1 -datarate 0 -docpu 1 -nosamba 0 -nocustom 0
}


4709b14/43602mch2 clone 4709b14x/43602mch2 {*}$xdargs
4709b14/4366mc clone 4709b14x/4366mc {*}$xdargs

4709/43602mch2 clone 4709b164x/43602mch2 {*}$xdargs \
   -tag BISON04T_TWIG_7_14_131 \
   -perfchans {3}
4709/4366mc clone 4709b164x/4366mc {*}$xdargs \
   -tag BISON04T_TWIG_7_14_131 \
   -perfchans {36/80}






	 
	 
UTF::Router 4709a -sta {4709/4360mcm5 eth1 4709/4360mcm5.%15 wl0.% 4709/4331mc eth2 4709/4331mc.%15 wl1.%} \
     -lan_ip 192.168.1.3 \
     -relay blr12end1 \
     -lanpeer lan \
     -power {npc21 1} \
	 -tag BISON04T_BRANCH_7_14 \
     -console "blr12end1.ban.broadcom.com:40002" \
     -brand linux-2.6.36-arm-internal-router \
     -nvram {
	watchdog=2000
	wl0_ssid=4709/4360mcm5
	wl0_chanspec=36
	wl0_radio=1
	wl0_atf=0
	wl0_vht_features=3
	wl1_ssid=4709/4331mc
	wl1_chanspec=3
	wl1_radio=1
	wl1_atf=0
	wl1_vht_features=3
	   } \
	-udp 1.2g \
	-datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g {15-90} -attn2g {35-90} -pad 29} \
	-wlinitcmds {sleep 5; wl ver; wl country US} \
    -pre_perf_hook {{%S wl vht_features}} \
	-post_perf_hook {{%S wl vht_features}} \
	-noradio_pwrsave 1
     


4709/4360mcm5 configure -attngrp G1
4709/4331mc configure -attngrp G1

4709/4360mcm5 clone 4709b14/4360mcm5 -tag BISON04T_BRANCH_7_14 \
    -perfchans {36/80}
4709/4331mc clone 4709b14/4331mc -tag BISON04T_BRANCH_7_14 \
    -channelsweep {-band b} \
    -perfchans {3l}
	

4709/4360mcm5 clone 4709b164/4360mcm5 -tag BISON04T_TWIG_7_14_164 \
    -perfchans {36/80}
4709/4331mc clone 4709b164/4331mc -tag BISON04T_TWIG_7_14_164 \
    -channelsweep {-band b} \
    -perfchans {3l}
	 
4709/4360mcm5 clone 4709b170/4360mcm5 -tag BISON04T_TWIG_7_14_170 \
    -perfchans {36/80}
4709/4331mc clone 4709b170/4331mc -tag BISON04T_TWIG_7_14_170 \
    -channelsweep {-band b} \
    -perfchans {3l}
###clone for external###
4709b14/4360mcm5 clone 4709b14x/4360 -sta {4709b14x/4360 eth2} \
    -tag BISON04T_BRANCH_7_14 \
	-brand linux-2.6.36-arm-external-vista-router-full-src \
    -perfonly 1 -perfchans {36/80} -noaes 1 -docpu 1 \
    -datarate {-i 0.5 -frameburst 1} -nocustom 1
	
4709b14x/4360 configure -dualband {4709b14x/4331 -c1 36/80}
################################ blr12tst1 ######################################
# blr12tst1      - 4360
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19 Livelock i686
# Adapter        - Adex PCIe Half Miniflex
# RF Enclosure 3 - Ramsey  STE 5000
# Power          - npc11 port 2     172.1.1.11
################################################################################



UTF::Linux blr12tst1 \
         -lan_ip 10.132.116.96 \
         -sta {4360 enp1s0} \
		 -power {npc31 2} \
		 -power_button "auto" \
         -slowassoc 5 -reloadoncrash 1 \
         -tag BISON_BRANCH_7_10 \
         -brand "linux-internal-wl" \
         -preinstall_hook {{%S dmesg -n 7}} \
		 -datarate {-b 1.8g -i 0.5 -frameburst 1} \
         -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl counters}} \
         -pre_perf_hook {{%S wl rate} {%S wl nrate} {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
         -tcpwindow 4M \
         -wlinitcmds {wl msglevel +assoc; wl down; wl country US; wl obss_coex 0} \

4360 configure -ipaddr 192.168.1.198 -attngrp G1 \



################################ blr12tst3 ######################################
# blr12tst2      - 4366
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19 Livelock i686
# Adapter        - Adex PCIe Half Miniflex
# RF Enclosure 3 - Ramsey  STE 5000
# Power          - npc11 port 2     172.1.1.11
################################################################################

UTF::Linux blr12tst2 \
     -lan_ip 10.132.116.97 \
	 -power {npc31 1} \
     -sta {4366a enp1s0} \
	 -power_button "auto" \
	 -tcpwindow 4M \
     -datarate {-b 1.8g -frameburst 1} \
	 -tag EAGLE_BRANCH_10_10 \
     -wlinitcmds {wl msglevel +assoc; wl down; wl country US; wl bw_cap 2g -1; wl vht_features 7} \
     -slowassoc 5 -reloadoncrash 1 \
     -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl vht_features}} \
     -pre_perf_hook {{%S wl rate} {%S wl nrate} {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl vht_features}} \
         

4366a configure -ipaddr 192.168.1.191 -attngrp G1 \


4366a clone 4366aa \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country Q1/139; wl bw_cap 2g -1; wl vht_features 7} \

4366a clone 4366b164 \
   -tag BISON04T_REL_7_14_164 \


################################ blr12tst3 ######################################
# blr12tst3      - 4366
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19 Livelock i686
# Adapter        - Adex PCIe Half Miniflex
# RF Enclosure 3 - Ramsey  STE 5000
# Power          - npc11 port 2     172.1.1.11
################################################################################

UTF::Linux blr12tst3 \
     -lan_ip 10.132.116.98 \
	 -power {npc41 1} \
     -sta {4366b enp1s0} \
	 -power_button "auto" \
     -tcpwindow 4M \
	 -tag EAGLE_BRANCH_10_10 \
	 -datarate {-b 1.8g -frameburst 1} \
     -wlinitcmds {wl msglevel +assoc; wl bw_cap 2g -1; wl down; wl country US; wl vht_features 7} \
     -slowassoc 5 -reloadoncrash 1 \
     -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl counters} {%S wl vht_features}} \
     -pre_perf_hook {{%S wl rate} {%S wl nrate} {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl vht_features}} \
	 

4366b configure -ipaddr 192.168.1.192 -attngrp G1 \


4366b clone 4366bb \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country Q1/139; wl bw_cap 2g -1; wl vht_features 7} \

4366b clone 4366b164 \
   -tag BISON04T_REL_7_14_164 \

################################ blr12tst3 ######################################
# blr12tst4      - 4366
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19 Livelock i686
# Adapter        - Adex PCIe Half Miniflex
# RF Enclosure 3 - Ramsey  STE 5000
# Power          - npc11 port 2     172.1.1.11
##################################################################################


UTF::Linux blr12tst4 \
     -lan_ip 10.132.116.99 \
	 -power {npc41 2} \
     -sta {4366c enp1s0} \
	 -power_button "auto" \
     -tcpwindow 4M \
	 -tag EAGLE_BRANCH_10_10 \
	 -datarate {-b 1.8g -frameburst 1} \
     -wlinitcmds {wl msglevel +assoc; wl bw_cap 2g -1; wl down; wl country US; wl vht_features 7} \
     -slowassoc 5 -reloadoncrash 1 \
     -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl counters} {%S wl vht_features}} \
     -pre_perf_hook {{%S wl rate} {%S wl nrate} {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl vht_features}} \
	 
	

4366c configure -ipaddr 192.168.1.193 -attngrp G1 \


4366c clone 4366cc \
		-wlinitcmds {wl msglevel +assoc; wl down; wl country Q1/139; wl bw_cap 2g -1; wl vht_features 7} \



set UTF::RouterNightlyCustom {

    if {[regexp {(.*x)/} $Router - base]} {
	# external
      	 if {$STA3 ne ""} {

           package require UTF::Test::TripleBand

	    TripleBand ${base}/0 ${base}/1 ${base}/2 $STA2 $STA1 $STA3 \
	  	 -c1 3l -c2 44/80 -c3 157/80 -lan1 lan3 -lan2 lan -lan3 lan4

	 }
    } else {
	# Internal

	 # UTF::Try "$Router: Vendor IE" {
	   # package require UTF::Test::vndr_ie
	   # UTF::Test::vndr_ie $Router $STA1
	# }
     }

    # package require UTF::Test::MiniUplinks
    # UTF::Test::MiniUplinks $Router $STA1 -sta $STA2
	
      # package require UTF::Test::Repeaters
      # UTF::Test::Repeaters $Router $STA1 -sta $STA2 -otherlans {lan}

}





UTF::Q blr12
