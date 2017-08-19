# -*-tcl-*-
#
# Testbed MC86 shared configuration file
# Filename: mc86.tcl
# Charles Chai 01/11/2013
# 
package require UTF::Aeroflex
#package require UTF::AeroflexDirect

# SummaryDir sets the location for test results
#set ::UTF::SummaryDir "/projects/hnd_sig_ext7/$::env(LOGNAME)/mc86"

# Add DataRatePlot to all embedded runs
set UTF::DataRatePlot 1

# Endpoint: FC11 Desktop
UTF::Linux mc86end1 -sta {lan eth1}

# Define power controllers
# NOTE: Option '-rev 1' is required for newer NPC22(s) model
UTF::Power::Synaccess softap1  -lan_ip 172.5.5.4  -rev 1
UTF::Power::Synaccess softap2  -lan_ip 172.5.5.14 -rev 1
UTF::Power::Synaccess npctst11 -lan_ip 172.5.5.34 -rev 1
UTF::Power::Synaccess npcdut1a -lan_ip 172.5.5.6  -rev 1
UTF::Power::Synaccess npcdut1b -lan_ip 172.5.5.16 -rev 1
UTF::Power::Synaccess npcdut2a -lan_ip 172.5.5.26 -rev 1
UTF::Power::Synaccess npcdut2b -lan_ip 172.5.5.36 -rev 1
UTF::Power::WebRelay  mc86wr1  -lan_ip 10.19.60.117

# Attenuator - Aeroflex (run 'af setup' if changing to UTF::AeroflexDirect)
# Syntax "-lan_ip 172.5.5.7:20000/udp" is to support parallel test using UDP
#
UTF::Aeroflex af -lan_ip 172.5.5.7:20000/udp \
    -relay "mc86end1" -group {G1 {1 2 3} G2 {4 5 6} G3 {7 8 9} ALL {1 2 3 4 5 6 7 8 9}}

G3 createGroup G3a {7 8+35 9+35}

# Attenuator - Aeroflex (run 'af setup' if switch to UTF::Aeroflex)
#UTF::AeroflexDirect af -lan_ip 172.5.5.7 \
#    -relay "mc86end1" \
#    -debug 1 \
#    -group {G1 {1 2 3} G2 {4 5 6} ALL {1 2 3 4 5 6}}

# Set attenuator ranges
set ::cycle5G80AttnRange "0-90 90-0"
set ::cycle5G40AttnRange "0-90 90-0"
set ::cycle5G20AttnRange "0-90 90-0"
set ::cycle2G40AttnRange "0-90 90-0"
set ::cycle2G20AttnRange "0-90 90-0"

# Softap DHCP recording
# mc86tst7 - 192.168.1.127
# mc86tst8 - 192.168.1.128
# mc86tst9 - 192.168.1.129
# mc86tst10- 192.168.1.130
# mc86tst4 - 192.168.1.135

# ----End----
 
