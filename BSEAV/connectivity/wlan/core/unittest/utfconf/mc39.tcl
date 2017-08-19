# -*tcl-*-
# -*tcl-*-
#
#  $Copyright Broadcom Corporation$
#  $Id: a9bd3eac007b3419600759657077365ec8106d73 $
#
#  Testbed configuration file for MC39 Testbed Sheida Otmishi

# VLANS:
# 1    => 171.16.1/24    Ctrl
# 1019 => 10.19.84/22    Corp
# 1921 => 191.168.1/24   LAN
# 1922 => 192.168.254/24 WAN

# Inside enclosure NPC 172.16.1.x5 Network Switch 172.16.1.x0
# T is for 1U,1019T,1921T,1922T
# 172.16.1.10 SW10 T T 1019U 1091U 1921U 1922U T T
# 172.16.1.20 SW20 T T 1019U 1019U 1019U 1019U T T
# 172.16.1.30 SW30 T T 1921U 1921U 1922U 1922U T T
# 172.16.1.40 SW40 T T 1019U 1019U 1921U 1922U T T
# 172.16.1.50 SW50 T T 1019U 1019U 1921U 1922U T T
# 172.16.1.60 SW60 T T 1019U 1019U 1921U 1922U T T
# 172.16.1.70 SW70 T T 1019U 1019U 1921U 1922U T T

# 172.16.1.45 NPC45
# 172.16.1.55 NPC55
# 172.16.1.65 NPC65
# 172.16.1.75 NPC75

# Consolelogger Setup
#exec /usr/local/bin/consolelogger -p 40001 NPC65:2001 >/tmp/npc1.log 2>&1$
#exec /usr/local/bin/consolelogger -p 40451 NPC45:2001 >/tmp/npc451.log 2>&1$
#exec /usr/local/bin/consolelogger -p 40452 NPC45:2002 >/tmp/npc452.log 2>&1$
#exec /usr/local/bin/consolelogger -p 40002 NPC65:2002 >/tmp/npc2.log 2>&1$
#exec /usr/local/bin/consolelogger -p 40551 NPC55:2001 >/tmp/npc51.log 2>&1$
#exec /usr/local/bin/consolelogger -p 40552 NPC55:2002 >/tmp/npc52.log 2>&1$
#exec /usr/local/bin/consolelogger -p 40751 NPC75:2001 >/tmp/npc71.log 2>&1$

# /etc/hosts
#172.16.1.10 SW10
#172.16.1.20 SW20
#172.16.1.30 SW30
#172.16.1.40 SW40
#172.16.1.60 SW60
#172.16.1.70 SW70

#172.16.1.45 NPC45
#172.16.1.55 NPC55
#172.16.1.65 NPC65
#172.16.1.75 NPC75

# Define power controllers inside phone booth
package require UTF::Power
UTF::Power::Synaccess power4 -lan_ip 172.16.1.45 -relay mc39end1 -rev 1
UTF::Power::Synaccess power5 -lan_ip 172.16.1.55 -relay mc39end1
UTF::Power::Synaccess power6 -lan_ip 172.16.1.65 -relay mc39end1 -rev 1

UTF::Linux mc39end1 -sta {lan eth1} -lan_ip 10.19.86.71
