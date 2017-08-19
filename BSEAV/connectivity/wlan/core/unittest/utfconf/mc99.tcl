# -*-tcl-*-
#
# Testbed MC99 shared configuration file
# Filename: mc99.tcl
# Charles chai 6/4/2014
# 

package require UTF::Aeroflex
#package require UTF::AeroflexDirect

# SummaryDir sets the location for test results
#set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/mc99"

# Add DataRatePlot to all embedded runs
set UTF::DataRatePlot 1

# Endpoint: FC19_x64 Desktop
UTF::Linux mc99end1 -sta {lan p1p1}

# Define power controllers
# NOTE: Option '-rev 1' is required for newer NPC22(s) model
UTF::Power::Synaccess npc94  -lan_ip 172.5.9.4  -rev 1
UTF::Power::Synaccess npc95  -lan_ip 172.5.9.5  -rev 1
UTF::Power::Synaccess npc96  -lan_ip 172.5.9.6  -rev 1
UTF::Power::Synaccess npc99  -lan_ip 172.5.9.9  -rev 1
UTF::Power::Synaccess npc910 -lan_ip 172.5.9.10 -rev 1
UTF::Power::Synaccess npc912 -lan_ip 172.5.9.12 -rev 1
UTF::Power::Synaccess npc913 -lan_ip 172.5.9.13 -rev 1

# Attenuator - Aeroflex (run 'af setup' if changing to UTF::AeroflexDirect)
# Syntax "-lan_ip 172.5.9.7:20000/udp" is to support parallel test using UDP
#
UTF::Aeroflex af -lan_ip 172.5.9.7:20000/udp \
    -relay "mc99end1" -group {G1 {1 2 3 4} G2 {5 6 7 8} G3 {9 10 11 12} ALL {1 2 3 4 5 6 7 8 9 10 11 12}}

#G1 configure -default 25

# Attenuator - Aeroflex (run 'af setup' if switch to UTF::Aeroflex)
#UTF::AeroflexDirect af -lan_ip 172.5.9.7 \
#    -relay "mc99end1" \
#    -debug 1 \
#    -group {G1 {1 2 3 4} G2 {5 6 7 8} ALL {1 2 3 4 5 6 7 8}}

# Set attenuator ranges
set ::cycle5G80AttnRange "0-90 90-0"
set ::cycle5G40AttnRange "0-90 90-0"
set ::cycle5G20AttnRange "0-90 90-0"
set ::cycle2G40AttnRange "0-95 95-0"
set ::cycle2G20AttnRange "0-95 95-0"

# mc99tst7 - 192.168.1.115
# mc99tst8 - 192.168.1.125 
# mc99tst9 - 192.168.1.135
# mc99tst10- 192.168.1.145
# mc99tst4 - 192.168.1.146
# mc99tst1 - 192.168.1.147
# mc99tst2 - 192.168.1.148
# mc99tst3 - 192.168.1.149

# -- end --


