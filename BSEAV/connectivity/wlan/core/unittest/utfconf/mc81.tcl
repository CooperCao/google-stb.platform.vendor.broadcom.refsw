# -*-tcl-*-
#
# Testbed MC81 configuration file for Charles Chai
#   May 2012    : Initial setup AP1 4717 TST1 43228/4313
#   July 2012   : Replaced TST1 host E6400 with E6420
#   August 2012 : Added AP2 4706/4360 TST2 4360/4352
#   August 2012 : Replaced 4313 with 43142
#   January 2013: Moved MacAir to MC86
#   January 2013: Replaced 43142_A516 with A521 B0
# 
# Hardware
#   mc81end1  : FC11 Desktop   
#   mc81tst1  : FC15 Dell Laptop E6420
#   mc81tst2  : FC15 Dell Laptop E6420
#   AP1       : Linksys E2000/4717
#   AP2       : Netgear R6300/4706
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 10069-6 (0-63dB)
#

# Load Packages
package require UTF::Aeroflex
#package require UTF::AeroflexDirect
package require UTF::Linux
package require UTF::utils

# Add DataRatePlot to all embedded runs
set UTF::DataRatePlot 1

# Set summary dir
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/mc81"

# Define power controllers
# NOTE: Option '-rev 1' is required for newer NPC22(s) model
UTF::Power::Synaccess mc81npc1 -lan_ip 172.20.5.4 -rev 1
UTF::Power::Synaccess mc81npc2 -lan_ip 172.20.5.6 -rev 1
UTF::Power::Synaccess mc81npc3 -lan_ip 172.20.5.14 -rev 1
UTF::Power::Synaccess mc81npc4 -lan_ip 172.20.5.16 -rev 1

# Attenuator - Aeroflex (run 'af setup' if changing to UTF::AeroflexDirect)
UTF::Aeroflex af -lan_ip 172.20.5.7 \
    -relay "mc81end1" \
    -group {G1 {1 2 3} G2 {4 5 6} ALL {1 2 3 4 5 6}}
    G1  configure -default 0
    G2  configure -default 0
    ALL configure -default 0

# Attenuator - Aeroflex (run 'af setup' if switch to UTF::Aeroflex)
#UTF::AeroflexDirect af -lan_ip 172.20.5.7 \
#    -relay "mc81end1" \
#    -debug 1 \
#    -group {G1 {1 2 3} G2 {4 5 6} ALL {1 2 3 4 5 6}}

# Set attenuator ranges
set ::cycle5G80AttnRange "0-63 63-0"
set ::cycle5G40AttnRange "0-63 63-0"
set ::cycle5G20AttnRange "0-63 63-0"
set ::cycle2G40AttnRange "0-63 63-0"
set ::cycle2G20AttnRange "0-63 63-0"

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Set attenuator to 0
    ALL attn default

    # Make sure AP radios are off
    catch {4717    restart wl0_radio=0}
    catch {4706ap1 restart wl0_radio=0}
    catch {4706ap2 restart wl0_radio=0}

    # Make sure all STAs are deinit and down 
    # just in case debugging left them loaded
    foreach S {43228 43142 4360 4352} {
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

# Endpoint: FC11 Desktop - traffic generators (no wireless cards)
UTF::Linux mc81end1 \
    -sta {lan eth1}

# STA1: FC15 - Dell Laptop E6420 
# 43228_A404: 11n 2x2
# 43142hm_A521 B0: 11n 1x1 BT Combo 2G Only
UTF::Linux mc81tst1  \
        -lan_ip mc81tst1 \
        -sta {43228 eth1 43142 eth0} \
        -tcpwindow 2M \
	-console "mc81end1:40002" \
 	-power {mc81npc2 1} \
	-power_button "auto" \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0} \
        -brand linux-internal-wl

43228 clone 43228b -tag BISON_BRANCH_7_10
43228 clone 43228t -tag trunk
43142 clone 43142b -tag BISON_BRANCH_7_10 -nobighammer 1
43142 clone 43142t -tag trunk -nobighammer 1


# STA2: FC15 Laptop - Dell E6420
# 4360: 11ac 3x3 5G Only
# 4352: 11ac 2x2 BT Combo 5G Only 
UTF::Linux mc81tst2  \
        -lan_ip mc81tst2 \
        -sta {4360 eth0 4352 eth1} \
        -tcpwindow 4M \
        -console "mc81end1:40004" \
        -power {mc81npc4 1} \
        -power_button "auto" \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc; wl PM 0; wl mimo_bw_cap 1; wl amsdu 1; wl ampdu_mpdu 64; wl ack_ratio 2} \
        -brand linux-internal-wl \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl phy_cal_disable 0}}
	

4360 clone 4360b -tag BISON_BRANCH_7_10
4360 clone 4360t -tag NIGHTLY
4352 clone 4352b -tag BISON_BRANCH_7_10
4352 clone 4352t -tag NIGHTLY


# AP1: Linksys E2000/4717 11n 2x2
UTF::Router AP1 \
        -sta "4717 eth1" \
        -lan_ip 192.168.1.1 \
        -relay "mc81end1" \
        -lanpeer lan \
        -console "mc81end1:40001" \
	-power {mc81npc1 1} \
	-power_button "auto" \
        -brand linux-internal-router \
	-tag "AKASHI_BRANCH_5_110" \
        -nvram {
                et1macaddr=68:7F:74:1A:B5:F4
                macaddr=68:7F:74:1A:B5:F4
                lan_ipaddr=192.168.1.1
                lan_gateway=192.168.1.1
                dhcp_start=192.168.1.100
                dhcp_end=192.168.1.149
                lan1_ipaddr=192.168.2.1
                lan1_gateway=192.168.2.1
                dhcp1_start=192.168.2.100
                dhcp1_end=192.168.2.149
                fw_disable=1
                #router_disable=1
                wl_msglevel=0x101
                wl0_ssid=4717ap
                wl0_channel=1
                wl0_radio=0
		wl0_obss_coex=0
                # Used for RSSI -35 to -45 TP Variance
                antswitch=0
                # Used to WAR PR#86385
                wl0_obss_coex=0
                #MBSSID
	}


# AP2: Netgear R6300/4706 11ac wireless router
# 4706ap1 - 4331nic
# 4706ap2 - 4360nic
# -tag "AARDVARK_REL_6_30_44"
UTF::Router AP2 \
        -sta "4706ap1 eth1 4706ap2 eth2" \
        -lan_ip 192.168.1.2 \
        -relay "mc81end1" \
        -lanpeer lan \
        -console "mc81end1:40003" \
        -power {mc81npc3 1} \
        -power_button "auto" \
        -brand linux26-internal-router \
        -tag "AARDVARK_BRANCH_6_30" \
        -nvram {
                et2macaddr=00:90:4C:0E:51:BB
                macaddr=00:90:4C:0E:51:BB
                lan_ipaddr=192.168.1.2
                lan_gateway=192.168.1.2
                dhcp_start=192.168.1.150
                dhcp_end=192.168.1.199
                lan1_ipaddr=192.168.2.2
                lan1_gateway=192.169.2.2
                dhcp1_start=192.168.2.150
                dhcp1_end=192.168.2.199
                fw_disable=1
                #router_disable=1
                wl_msglevel=0x101
                wl0_ssid=4706/4331
                wl1_ssid=4706/4360
                wl0_channel=1
                wl0_radio=0
                # Used for RSSI -35 to -45 TP Variance
                antswitch=0
                # Used to WAR PR#86385
                wl0_obss_coex=0
                #MBSSID
		#Kevin wrote 9/20/2012: 
		#Bill Palter found an issue with the amplifier setting 
		#and the following needs to be added for the UTF config file 
		1:boardflags2=0x4000000
        }

4706ap2 clone 4706ap2-bis -tag BISON_BRANCH_7_10

# Turn on post log processing ('parse_wl_logs' link on report html page)
#set ::UTF::PostTestAnalysis /home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl
#set ::aux_lsf_queue sj-hnd

### 
UTF::Q mc81

