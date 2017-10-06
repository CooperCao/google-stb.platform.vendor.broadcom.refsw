# -*-tcl-*-
#
# Testbed MD16 shared configuration file
# Filename: md16.tcl
# Charles chai 12/30/2015
#

package require UTF::Aeroflex

# SummaryDir sets the location for test results
#set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/md16"

# Add DataRatePlot to all embedded runs
set UTF::DataRatePlot 1

# Endpoint: FC19_x64 Desktop
UTF::Linux md16end1 -sta {lan p4p1}
lan configure -ipaddr 192.168.1.50

# Define power controllers
# NOTE: Option '-rev 1' is required for newer NPC22(s) model
UTF::Power::Synaccess npc97  -lan_ip 172.5.9.7  -rev 1
UTF::Power::Synaccess npc98  -lan_ip 172.5.9.8  -rev 1
UTF::Power::Synaccess npc99  -lan_ip 172.5.9.9  -rev 1
UTF::Power::Synaccess npc910 -lan_ip 172.5.9.10 -rev 1
UTF::Power::Synaccess npc911 -lan_ip 172.5.9.11 -rev 1
UTF::Power::Synaccess npc912 -lan_ip 172.5.9.12 -rev 1
UTF::Power::Synaccess npc913 -lan_ip 172.5.9.13 -rev 1
UTF::Power::Synaccess npc914 -lan_ip 172.5.9.14 -rev 1

# Attenuator - Aeroflex (run 'af setup' if changing to UTF::AeroflexDirect)
# Syntax "-lan_ip 172.5.9.7:20000/udp" is to support parallel test using UDP
#
UTF::Aeroflex af -lan_ip 172.5.9.1:20000/udp \
    -relay "md16end1" -group {G1 {1 2 3 4} G2 {5 6 7 8} G3 {9 10 11 12} ALL {1 2 3 4 5 6 7 8 9 10 11 12}}

# Set attenuator ranges
set ::cycle5G80AttnRange "0-90 90-0"
set ::cycle5G40AttnRange "0-90 90-0"
set ::cycle5G20AttnRange "0-90 90-0"
set ::cycle2G40AttnRange "0-95 95-0"
set ::cycle2G20AttnRange "0-95 95-0"

# -- end --
