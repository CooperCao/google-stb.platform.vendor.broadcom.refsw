# -*-tcl-*-
#
# Testbed configuration file for MC82testbed
# External DHCP Server (AP with radios disabled)
#
# updated on 4/25/2016
# SoftAP is enabled with DHCP
# - zhuj

###############################################
# Load Packages
# Packages commonly used in interactive mode
if {[info exists ::tcl_interactive] && $::tcl_interactive} {
    package require UTF::Test::ConnectAPSTA
    package require UTF::Test::APChanspec
    package require UTF::FTTR
    package require UTF::Test::ConfigBridge
}

package require UTF::Linux
package require UTF::Power
#package require UTF::AeroflexDirect
package require UTFD
package require UTF::Aeroflex
package require UTF::Sniffer

# To setup UTFD port for this rig
set ::env(UTFDPORT) 9988


# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext20/$::env(LOGNAME)/mc82"

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

# Define power controllers on cart
UTF::Power::Synaccess npc45 -lan_ip 172.5.1.45 -rev 1
UTF::Power::Synaccess npc46 -lan_ip 172.5.1.46 -rev 1
UTF::Power::WebRelay  web47 -lan_ip 172.5.1.47
UTF::Power::Synaccess npc55 -lan_ip 172.5.1.55 -rev 1
UTF::Power::Synaccess npc56 -lan_ip 172.5.1.56 -rev 1
UTF::Power::WebRelay  web57 -lan_ip 172.5.1.57
UTF::Power::Synaccess npc65 -lan_ip 172.5.1.65 -rev 1
UTF::Power::Synaccess npc75 -lan_ip 172.5.1.75 -rev 1
UTF::Power::Synaccess npc85 -lan_ip 172.5.1.85 -rev 1
UTF::Power::Synaccess npc201 -lan_ip 192.168.1.201 -rev 1


# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.5.1.200 \
    -relay "lan" \
    -group {
	    G1 {1 2 3}
	    G2 {4 5 6}
	    G3 {7 8 9}
	    G4 {10 11 12}
	    }
	    G1 configure -default 0
	    G2 configure -default 0
	    G3 configure -default 0
	    G4 configure -default 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn default
    G2 attn default
    G3 attn default
    G4 attn default

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4360sap1 4360a 4360b} {
	UTF::Try "$S Down" {
	    catch {$S wl down}
	}
	$S deinit
    }
    # unset S so it doesn't interfere
    unset S

    # Now APs
    #foreach S {4706/4331 4706/4360 47061/4331 47061/4360} {
	#catch {$S apshell ifconfig [$S cget -device] down}
    #}
    # unset S so it doesn't interfere
    #unset S
    # temporary make sure X14 is power cycled before tests to cover load/unload issues
    #	UTF::Try "X14 Cycle" {
    #		MacX14 power cycle
    #	}
    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}


# Define Sniffer
UTF::Sniffer mc82snf1 -user root \
        -sta {4360SNF1 eth0} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc85 1} \
        -power_button {auto} \
        -console "mc82end1:40006"

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc82end1 \
    -lan_ip 10.19.60.71 \
    -sta {lan eth1}

# UTF Endpoint2 FC19 - Controller
UTF::Linux mc82end2 \
    -lan_ip 10.19.60.72

# FC19 Linux PC / SOFTAP
UTF::Linux mc82kvm1 -sta {4360sap1 enp1s0} \
    -power {npc65 2} \
    -console {mc82end1:40002} \
    -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3;wl dtim 3} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -yart {-attn5g {15-63} -attn2g {15-63}}

4360sap1 configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC19ap1
4360sap1 clone 4360esap1  -tag EAGLE_BRANCH_10_10


# FC19 Linux PC / Softap
UTF::Linux mc82kvm2 -sta {4360sap2 enp1s0} \
    -console "mc82end1:40004" \
    -power {npc75 2} \
    -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3;wl dtim 3} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -yart {-attn5g {15-63} -attn2g {15-63}}

4360sap2 configure -ipaddr 192.168.1.98 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC19ap2
4360sap2 clone 4360esap2  -tag EAGLE_BRANCH_10_10


# FC19 Linux PC
UTF::Linux mc82tst4 -sta {4360a enp1s0} \
    -power {npc55 2} \
    -console {mc82end2:40000} \
    -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -yart {-attn5g {15-63} -attn2g {15-63}}


4360a configure -attngrp G4
4360a clone 4360ae -tag EAGLE_BRANCH_10_10

# FC19 Linux PC
UTF::Linux mc82tst5 -sta {4360b enp1s0} \
    -power {npc46 1} \
    -console {mc82end2:40001} \
    -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -yart {-attn5g {15-63} -attn2g {15-63}}

4360b configure -attngrp G3
4360b clone 4360be -tag EAGLE_BRANCH_10_10


proc closeall {} {
    foreach s [UTF::STA info instances] {
	$s deinit
    }
}


#### ADD UTF::Q for this rig
#####
#UTF::Q mc82

