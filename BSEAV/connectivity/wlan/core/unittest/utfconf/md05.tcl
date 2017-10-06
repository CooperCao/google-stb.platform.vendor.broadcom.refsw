#
#  $Copyright Broadcom Corporation$
#  $Id$
#
#  Testbed configuration file for MD05 Testbed
#

package require UTF::Aeroflex
UTF::Aeroflex Af -lan_ip  172.16.1.144:20000/udp -relay md05end1 \
     -group {G1 {1 2} G2 {3 4} G3 {1 2 3 4} G4 {5 6} G5 {7 8} G6 {5 6 7 8} G9 {9}}

# lan 172.16.1.111 p16p1
UTF::Linux md05end1 -sta {lan p10p1} -lan_ip 10.19.61.77
