# -*-tcl-*-
#
#  $Copyright Broadcom Corporation$
#  $Id: 61931c59eb7eb76e93eae301c041169762604ab0 $
#
#  Testbed configuration file for MD07 Testbed
#

package require UTF::Aeroflex
UTF::Aeroflex Af -lan_ip  172.16.1.144:20000/udp -relay md07end3 \
	-group {G1 {1 2 3 4} G2 {5 6 7}} -retries 100

UTF::Linux md07end3 -sta {lan p1p1}
lan configure -ipaddr 192.168.1.99
