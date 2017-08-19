# -*-tcl-*-
#
# Testbed MC89 shared configuration file
# Filename: mc89.tcl
# Charles chai
# 01/19/2013: Initial setup
# 11/03/2016: Refurbished to 16-client MU-MIMO rig
#  

package require UTF::Aeroflex
#package require UTF::AeroflexDirect

# Add DataRatePlot to all embedded runs
set UTF::DataRatePlot 1

# Endpoint: FC19 Desktop
UTF::Linux mc89end1 -sta {lan p1p1}

# Define power controllers
# NOTE: Option '-rev 1' is required for newer NPC22(s) model
UTF::Power::Synaccess npcap1   -lan_ip 172.5.8.4  -rev 1
UTF::Power::Synaccess npcap2   -lan_ip 172.5.8.24 -rev 1
UTF::Power::Synaccess npcdut1a -lan_ip 172.5.8.6  -rev 1
UTF::Power::Synaccess npcdut1b -lan_ip 172.5.8.36 -rev 1
UTF::Power::Synaccess npcdut2a -lan_ip 172.5.8.16 -rev 1
UTF::Power::Synaccess npcdut2b -lan_ip 172.5.8.26 -rev 1
UTF::Power::Synaccess npcdut3a -lan_ip 172.5.8.46 -rev 1
UTF::Power::Synaccess npcdut3b -lan_ip 172.5.8.56 -rev 1
UTF::Power::Synaccess npcdut4a -lan_ip 172.5.8.66 -rev 1
UTF::Power::Synaccess npcdut4b -lan_ip 172.5.8.76 -rev 1
UTF::Power::Synaccess npctst1  -lan_ip 172.5.8.34 -rev 1

# Attenuator - Aeroflex (run 'af setup' if changing to UTF::AeroflexDirect)
# Syntax "-lan_ip 172.5.8.7:20000/udp" is to support parallel test using UDP
#
UTF::Aeroflex af -lan_ip 172.5.8.7:20000/udp \
    -relay "mc89end1" -group {G1 {1 2 3 4} G2 {5 6 7 8} ALL {1 2 3 4 5 6 7 8 9}}

# Set attenuator ranges
set ::cycle5G80AttnRange "0-90 90-0"
set ::cycle5G40AttnRange "0-90 90-0"
set ::cycle5G20AttnRange "0-90 90-0"
set ::cycle2G40AttnRange "0-90 90-0"
set ::cycle2G20AttnRange "0-90 90-0"


# Softap DHCP recording
# mc89end2 - 192.168.1.135

# ----End----

