# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 4c468324460a0fab90a22d614b425718bdcd16bf $
#

### tftp server
UTF::Linux UTFTestO

### power switch
UTF::Power::Synaccess pwr -relay stbutf01end1 -lan_ip stbutf01pwr

### Controller
UTF::Linux stbutf01end1 -sta {lan enp10s0}

### Attenuator
package require UTF::Aeroflex
UTF::Aeroflex stbutf01att:20000/udp -relay lan -group {A {1 2 3 4} B {5 6 7 8} C {9 10 11 12}}


