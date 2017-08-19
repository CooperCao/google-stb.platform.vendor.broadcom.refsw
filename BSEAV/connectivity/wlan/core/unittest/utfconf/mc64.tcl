# -*-tcl-*-
#
# Testbed configuration file for Rodney Baba MC64testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# Add DataRatePlot to all embedded runs
set UTF::DataRatePlot 1

# this is a tool given by Tim to allow running private scripts ontop of StaNightly
set ::UTF::StaNightlyCustom {
	# Custom code here
	package require UTF::Test::throttle
	if {[$STA cget -custom] != ""} {
		throttle $Router $STA -chanspec 3 -nonthrottlecompare 1
	}
	UTF::Message INFO $STA "throttle test skipped"
}

# SummaryDir sets the location for test results

set ::UTF::SummaryDir "/projects/hnd_sig_ext/$::env(LOGNAME)/mc64"

# This section is to test drive John Brearley's test log analysis tool
#set ::UTF::PostTestAnalysis /home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl
# This section is required to do post test analysis with EmbeddedNightly.test
#set ::UTF::PostTestHook {
#    package require UTF::utils
#    UTF::do_post_test_analysis [info script] ""}

# required to offload analysis to sj-hnd servers
#set ::aux_lsf_queue sj-hnd







# Define power controllers on cart
UTF::Power::Synaccess npc45 -lan_ip 172.1.1.45
UTF::Power::Synaccess npc46 -lan_ip 172.1.1.46
UTF::Power::WebRelay  web47 -lan_ip 172.1.1.47
UTF::Power::Synaccess npc55 -lan_ip 172.1.1.55
UTF::Power::Synaccess npc56 -lan_ip 172.1.1.56
UTF::Power::WebRelay  web57 -lan_ip 172.1.1.57
UTF::Power::Synaccess npc65 -lan_ip 172.1.1.65
UTF::Power::Synaccess npc75 -lan_ip 172.1.1.75
UTF::Power::Synaccess npc85 -lan_ip 172.1.1.85

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.1.1.200 \
    -relay "lan" \
    -group {
	    G1 {1 2 3}
	    G2 {4 5 6}
	    }
	   	G1 configure -default 0
	    G2 configure -default 0
	    
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn default
    G2 attn default

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    AP1 restart wl0_radio=0
    AP1 restart wl1_radio=0
    AP2 restart wl0_radio=0
    AP2 restart wl1_radio=0
    
	# now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4331FC15 MacX28 MacX21b 43142WIN7 MacX51 MacX21bZ} {
	    UTF::Try "$S Down" {
		    $S wl down
	    }
	    $S deinit
	}
    # unset S so it doesn't interfere
    unset S
    
    # delete myself
    unset ::UTF::SetupTestBed

    return
}


## Needed for RvRFastSweep mystepprofile
set UTF::StepProfile(myprofile) ""
set dwell 1
set max 70
set stepsize 5
set upramp ""
set downramp ""
for {set ix 0} {$ix < $max} {incr ix +$stepsize} {
  lappend upramp "$stepsize $dwell"
}
for {set ix $max} {$ix > 0} {incr ix -$stepsize} {
  lappend downramp "-$stepsize $dwell"
}
set UTF::StepProfile(myprofile) [concat $upramp $downramp]



# Define Sniffer
UTF::Sniffer mc64snf1 -user root \
        -sta {4331SNF1 eth1} \
        -tag RUBY_REL_6_20_42 \
        -power {npc85 2} \
        -power_button {auto} \
        -console "mc64end1:40006"


# Quick test to see If I can turn the Sniffer device into a SOFTAP
#UTF::Linux mc64snf1 -user root \
#        -sta {4331SOFTAP1 eth1} \
#        -tag RUBY_BRANCH_6_20 \
#        -power {npc85 2} \
#        -power_button {auto} \
#        -console "mc64end1:40006" \

#4331SOFTAP1 configure -ipaddr 192.168.1.20 -ap 1 -ssid test4331SOFTAP1 \
#    -wlinitcmds {wl mpc 0; wl stbc_rx 1;:} 





# UTF Endpoint1 FC9 - Traffic generators (no wireless cards)
UTF::Linux mc64end1 \
    -sta {lan eth1} 

# UTF Endpoint2 FC9 - Traffic generators (no wireless cards)
UTF::Linux mc64end2 \
    -sta {lan1 eth1}

# STA Laptop DUT Dell E6400 FC9 - 43228hm4l_P403
UTF::Linux mc64tst1 -sta {4331FC15 eth0} \
  	-power {npc45 1} \
	-power_button {auto} \
    -console "mc64end1:40007" \
    -wlinitcmds {wl msglevel 0x101 } \
    -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} {%S wl phy_cal_disable 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
    -yart {-attn "25-75" -mirror -frameburst 1 -b 450m} \
    -datarate {-b 450m -i 0.5 -frameburst 1} \
    -perfonly 1 -custom 1


# Clones for mc64tst1
4331FC15 clone 4331FC15-AARDVARK -tag AARDVARK_BRANCH_6_30 -perfonly 1
4331FC15 clone 4331FC15-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 \
    -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} {%S wl phy_cal_disable 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl PM 2} {%S wl PM} {%S wl phy_cal_disable 1}}
4331FC15 clone 4331FC15-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
4331FC15 clone 4331FC15-BISON -tag BIS120RC4_BRANCH_7_15
4331FC15 clone 4331FC15-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*
4331FC15 clone 4331FC15-RUBY -tag RUBY_BRANCH_6_20
4331FC15 clone 4331FC15-KIRIN -tag KIRIN_BRANCH_5_100
4331FC15 clone 4331FC15-KIRIN-REL -tag KIRIN_REL_5_100_*
4331FC15 clone 4331FC15-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4331FC15 clone 4331FC15-BASS -tag BASS_BRANCH_5_60
4331FC15 clone 4331FC15-BASS-REL -tag BASS_REL_5_60_*   

# Needed to get around MAC Power Cycle to Debugger    
UTF::Power::Laptop X28power -button {web47 1}
UTF::MacOS mc64tst2 -sta {MacX28 en1} \
	-power {X28power} \
	-power_button {auto} \
	-wlinitcmds {wl msglevel 0x101; wl assert_type 1; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl scansuppress 1} {%S wl phy_cal_disable 1}} \
	-brand  "macos-external-wl-gala" \
	-type Debug_10_11 \
	-coreserver AppleCore \
	-kextload true -custom 1 \
    -datarate 0 -custom 1

# Clones for mc64tst2
MacX28 clone MacX28-AARDVARK -tag AARDVARK_BRANCH_6_30  -custom 1 -perfonly 1
MacX28 clone MacX28-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*  -custom 1
MacX28 clone MacX28-BISON -tag BIS120RC4_BRANCH_7_15 -custom 1
MacX28 clone MacX28-BISON-REL-TAG -tag BIS120RC4_REL_7_15_201_* -custom 1
MacX28 clone MacX28-RUBY -tag RUBY_BRANCH_6_20
MacX28 clone MacX28-KIRIN -tag KIRIN_BRANCH_5_100
MacX28 clone MacX28-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX28 clone MacX28-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX28 clone MacX28-BASS -tag BASS_BRANCH_5_60   
MacX28 clone MacX28-BASS-REL -tag BASS_REL_5_60_*      

# X21b MacBook Air
# Needed to get around MAC Power Cycle to Debugger
# The {%S wl btc_mode 0} entires in pre-perf hooks is to work around a 2.G 20 RvR issue with BT Coex
# Even with BT enabled on the Host, we still see a lower RvR RX Graph
UTF::Power::Laptop MacX21bpower -button {web47 2}
UTF::MacOS mc64tst3 -sta {MacX21b en1 MacX21b-awdl p2p1} \
	-power {MacX21bpower} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101 ; wl assert_type 1; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl msglevel} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl btc_mode 0} {%S wl scansuppress 1}} \
	-brand  "macos-external-wl-gala" \
	-type Debug_10_11 \
	-coreserver AppleCore \
	-kextload true \
    -datarate 0 -custom 1

# Clones for mc64tst3
MacX21b clone MacX21b-AARDVARK -tag AARDVARK_BRANCH_6_30  -custom 1 -perfonly 1
MacX21b clone MacX21b-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*  -custom 1
MacX21b clone MacX21b-BISON -tag BIS715GALA_BRANCH_7_21 -custom 1
MacX21b clone MacX21b-BISON-REL-TAG -tag BIS715GALA_REL_7_21_47_* -custom 1
MacX21b clone MacX21b-RUBY -tag RUBY_BRANCH_6_20
MacX21b clone MacX21b-KIRIN -tag KIRIN_BRANCH_5_100
MacX21b clone MacX21b-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX21b clone MacX21b-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX21b clone MacX21b-BASS -tag BASS_BRANCH_5_60   
MacX21b clone MacX21b-BASS-REL -tag BASS_REL_5_60_*     

# STA Laptop DUT Dell E6410 WIN7 - 43142 P451
# mc64kvm1 is the kvm for this device
UTF::Cygwin mc64tst4 -sta {43142WIN7} -user user \
	-osver 7 \
  	-power {npc55 1} \
	-power_button {auto} \
    -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0}\
    -perfchans { 3l 3 } \
    -post_perf_hook {{%S wl dump ampdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -datarate {-i 0.5 -frameburst 1}

# Clones for mc64tst4
43142WIN7 clone 43142WIN7-AARDVARK -tag AARDVARK_BRANCH_6_30 -perfonly 1
43142WIN7 clone 43142WIN7-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
43142WIN7 clone 43142WIN7-BISON -tag BIS120RC4_BRANCH_7_15
43142WIN7 clone 43142WIN7-RUBY -tag RUBY_BRANCH_6_20
43142WIN7 clone 43142WIN7-RUBY-REL-TAG -tag "RUBY_REL_6_20_55_*"
43142WIN7 clone 43142WIN7-KIRIN -tag KIRIN_BRANCH_5_100
43142WIN7 clone 43142WIN7-KIRIN-REL -tag KIRIN_REL_5_100_*
43142WIN7 clone 43142WIN7-KIRIN-REL-TAG -tag KIRIN_REL_5_100_82_*
43142WIN7 clone 43142WIN7-BASS -tag BASS_BRANCH_5_60   
43142WIN7 clone 43142WIN7-BASS-REL -tag BASS_REL_5_60_*       


# 4360BU MacBook Air , Now MBP
# Needed to get around MAC Power Cycle to Debugger
# The {%S wl btc_mode 0} entires in pre-perf hooks is to work around a 2.G 20 RvR issue with BT Coex
# Even with BT enabled on the Host, we still see a lower RvR RX Graph
UTF::Power::Laptop 4360power -button {web57 2}
UTF::MacOS mc64tst5 -sta {MacX51 en0} \
	-power {4360power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0 ; wl down ; wl vht_features 3; wl country US ALL ; wl assert_type 1; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl btc_mode} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-external-wl-gala" \
	-type Debug_10_11 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 3640K
		
# Clones for mc64tst5
MacX51 clone MacX51-TOT -nochannels 1 -custom 1 \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0 ; wl down ; wl vht_features 3; wl country US ALL ; wl assert_type 1; wl oppr_roam_off 1 }
MacX51 clone MacX51-AARDVARK -tag AARDVARK_BRANCH_6_30 -custom 1 -perfonly 1
MacX51 clone MacX511-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX51 clone MacX511-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX51 clone MacX51-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -custom 1 \
	-yart {-attn "10-75" -mirror -frameburst 1}
MacX51 clone MacX51-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_*
MacX51 clone MacX51-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_*
MacX51 clone MacX51-AARDVARK-REL-TWIG-EXT -tag AARDVARK_TWIG_6_30_*
MacX51 clone MacX51-BISON -tag BIS715GALA_BRANCH_7_21 -custom 1
MacX51 clone MacX51-BISON-REL-TAG -tag BIS715GALA_REL_7_21_47_* -custom 1
MacX51 clone MacX51-RUBY -tag RUBY_BRANCH_6_20
MacX51 clone MacX51-KIRIN -tag KIRIN_BRANCH_5_100
MacX51 clone MacX51-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX51 clone MacX51-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX51 clone MacX51-BASS -tag BASS_BRANCH_5_60   
MacX51 clone MacX51-BASS-REL -tag BASS_REL_5_60_???      


# X21b MacBook Air
# Needed to get around MAC Power Cycle to Debugger
# The {%S wl btc_mode 0} entires in pre-perf hooks is to work around a 2.G 20 RvR issue with BT Coex
# Even with BT enabled on the Host, we still see a lower RvR RX Graph
UTF::Power::Laptop MacX21bZpower -button {web57 1}
UTF::MacOS mc64tst6 -sta {MacX21bZ en0 MacX21bZ-awdl p2p1} \
	-power {MacX21bZpower} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101 ; wl assert_type 1; wl oppr_roam_off 1  } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl msglevel} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl btc_mode 0} {%S wl scansuppress 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
    -datarate 0 -custom 1

# Clones for mc64tst6
MacX21bZ clone MacX21bZ-AARDVARK -tag AARDVARK_BRANCH_6_30 -custom 1 -perfonly 1
MacX21bZ clone MacX21bZ-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -custom 1 \
	-yart {-attn "10-75" -mirror -frameburst 1}
MacX21bZ clone MacX21bZ-BISON -tag BIS120RC4_BRANCH_7_15 -custom 1
MacX21bZ clone MacX21bZ-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1
MacX21bZ clone MacX21bZ-RUBY -tag RUBY_BRANCH_6_20
MacX21bZ clone MacX21bZ-KIRIN -tag KIRIN_BRANCH_5_100
MacX21bZ clone MacX21bZ-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX21bZ clone MacX21bZ-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX21bZ clone MacX21bZ-BASS -tag BASS_BRANCH_5_60   
MacX21bZ clone MacX21bZ-BASS-REL -tag BASS_REL_5_60_???    


# Lynksys E4200 4718/4331 Router AP1

UTF::Router AP1 \
    -sta "43311 eth2" \
	-lan_ip 192.168.1.1 \
    -relay "mc64end1" \
    -lanpeer lan \
    -console "mc64end1:40004" \
    -power {npc75 2} \
    -tag "AKASHI_REL_5_110_65" \
    -brand "linux-external-router" \
	-nvram {
		boot_hw_model=E4200
		et0macaddr=00:90:4c:05:00:8a
		macaddr=00:90:4c:05:00:9a
		sb/1/macaddr=00:90:4c:05:10:00
		pci/1/1/macaddr=00:90:4c:05:11:00
        sb/1/macaddr=00:90:4c:05:03:00
		lan_ipaddr=192.168.1.1
		lan_gateway=192.168.1.1
		dhcp_start=192.168.1.100
  		dhcp_end=192.168.1.149
		lan1_ipaddr=192.168.2.1
		lan1_gateway=192.169.2.1
		dhcp1_start=192.168.2.100
	    dhcp1_end=192.168.2.149
		fw_disable=1
		#Only 1 AP can serve DHCP Addresses
		#router_disable=1
		#simultaneous dual-band router with 2 radios
		wl0_radio=0
		wl1_radio=0
		wl1_nbw_cap=0
        wl_msglevel=0x101
		wl0_ssid=test43311-ant0
		wl1_ssid=test43311-ant1
		wl0_channel=1
		wl1_channel=1
		# Used for RSSI -35 to -45 TP Variance
		antswitch=0
	    # Used to WAR PR#86385
		wl0_obss_coex=0
	    wl1_obss_coex=0
	    # set to legacy mode
	}
# Clones for 43311
43311 clone 43311-EXT -brand linux-internal-router

# Used for RvRFastSweep.test
43311 configure -attngrp G1


# Lynksys E4200 4718/4331 Router AP2
UTF::Router AP2 \
    -sta "43312 eth2" \
	-lan_ip 192.168.1.2 \
    -relay "mc64end1" \
    -lanpeer lan \
    -console "mc64end1:40002" \
    -power {npc65 2} \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_65" \
	-nvram {
		boot_hw_model=E4200
		et0macaddr=00:90:4c:06:00:8b
		macaddr=00:90:4c:06:00:9b
		pci/1/1/macaddr=00:90:4c:06:11:00
        sb/1/macaddr=00:90:4c:06:03:00
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.150
  		dhcp_end=192.168.1.199
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.169.2.2
		dhcp1_start=192.168.2.150
	    dhcp1_end=192.168.2.199
		fw_disable=1
		#Only 1 AP can serve DHCP Addresses
		router_disable=1
		wl0_radio=0
		wl1_radio=0
		wl1_nbw_cap=0
        wl_msglevel=0x101
		wl0_ssid=test43312-ant0
		wl1_ssid=test43312-ant1
		wl0_channel=1
		wl1_channel=1
		# Used for RSSI -35 to -45 TP Variance
		antswitch=0
	    # Used to WAR PR#86385
		wl0_obss_coex=0
	    wl1_obss_coex=0
	}
# Clones for 43312
43312 clone 43312-EXT -brand linux-internal-router

# Used for RvRFastSweep.test
43312 configure -attngrp G2


# This is an experimental AP sometimes swapped with AP2 
#Linksys 320N 4717/4322 wireless router AP2.
#UTF::Router AP3 \
#	-sta "47171 eth1" \
#	-lan_ip 192.168.1.2 \
#	-relay "mc64end1" \
#	-lanpeer lan \
#	-console "mc64end1:40004" \
#	-power {npc75 2} \
#	-brand linux-external-router \
#	-tag "MILLAU_REL_5_70_48_*" \
#	-nvram {
#       et0macaddr=00:90:4c:09:00:8b
#		macaddr=00:90:4c:09:00:9b
#		lan_ipaddr=192.168.1.2
#		lan_gateway=192.168.1.2
#		dhcp_start=192.168.1.150
#  		dhcp_end=192.168.1.199
#		lan1_ipaddr=192.168.2.2
#		lan1_gateway=192.169.2.2
#		dhcp1_start=192.168.2.150
#	    dhcp1_end=192.168.2.199
#		fw_disable=1
#		router_disable=1
#		wl_msglevel=0x101
#		wl0_ssid=test4717-1
#		wl0_channel=1
#		wl0_radio=0
#		# Used for RSSI -35 to -45 TP Variance
#		antswitch=0
#	    # Used to WAR PR#86385
#		wl0_obss_coex=0
#	}
#	
# Clones for 47171
#47171 clone 47171-MILLAU -tag "MILLAU_REL_5_70_*" -brand linux-internal-router

#### ADD UTF::Q for this rig
#####
UTF::Q mc64

