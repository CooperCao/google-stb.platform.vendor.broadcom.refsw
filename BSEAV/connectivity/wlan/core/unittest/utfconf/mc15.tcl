# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: f0491685dd9e5a345d1e76188196e333607560d5 $
#
# Testbed configuration file for mc15 testbed
#

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc15"


# mc15eth1
# mc15ips1
# mc15tst1
# mc15tst2
# mc15tst3
# mc15tst4

# Power
UTF::Power::WTI mc15ips1

# Endpoints
UTF::Linux sr1end11 -sta {lan em2} -onall false
lan configure -ipaddr 192.168.1.50

# Attenuator
package require UTF::Aeroflex
UTF::Aeroflex 192.168.1.10:20000/udp -relay lan \
    -group {B {1 2 3 4} A {5 6 7} C {9 10 11}} -retries 100

A configure -default 16
C configure -default 16

