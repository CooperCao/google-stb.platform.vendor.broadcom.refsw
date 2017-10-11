# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#

# mc16eth1 - switch

### tftp server
UTF::Linux UTFTestO

### power switch

UTF::Power::WTI mc16ips1

### Controller
UTF::Linux sr1end09 -sta {lan em2}

### Attenuator

package require UTF::Aeroflex
UTF::Aeroflex mc35att1:20000/udp -relay lan -group {A {1 2 3} B {4 5 6} C {7 8 9 10}}


