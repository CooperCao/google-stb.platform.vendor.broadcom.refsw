# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: e738080f0ceaa0851915df0619d9e2e78b77f944 $
#
# Testbed configuration file for MC41testbed
#

# Switches:
# VLAN 1: LOCAL: 192.168.1.0/24
# VALN 10: CORP:  10.19.84.0/22

# OutCorp 192.168.1.248  10,10,10,10,10,T, T, T
# Router  192.168.1.250  1, 1, 1, 1, 1, 1, T, T
# OutTest 192.168.1.251  1, 1, 1, 1, 1,10, T, T
# Bottom  192.168.1.253  10,10,10,1, 1, 1, T, T
# Top     192.168.1.252  10,10,10,1, 1, 1, T, T

# Define power controllers on cart
UTF::Power::WebSwitch webswitch1 -lan_ip 192.168.1.11 -relay lan
UTF::Power::WebSwitch webswitch2 -lan_ip 192.168.1.15 -relay lan
UTF::Power::Synaccess npc3 -lan_ip 192.168.1.17 -relay lan
UTF::Power::Synaccess nb1 -lan_ip 192.168.1.14 -relay lan

# UTF Endpoint1 FC9 - Traffic generators (no wireless cards)
UTF::Linux mc41end1 -sta {lan eth1}
lan configure -ipaddr 192.168.1.50

# Attenuator - Aeroflex
package require UTF::Aeroflex
UTF::Aeroflex 192.168.1.16:20000/udp -relay lan \
    -group {G {1 2 3} B {4} C {5 6} E {8 7 9}} -retries 100
