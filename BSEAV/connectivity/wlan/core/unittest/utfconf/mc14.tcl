# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for shared mc14 testbed
#

# mc14end2
# mc14eth1
# mc14eth2 (pb2aeth1)
# mc14ips1
# mc14tst1
# mc14tst2
# mc14tst3
# mc14tst4
# mc14tst5
# mc14sts6

# Alternate Controller - eth1 only runs at 100mbps
UTF::Linux sr1end05

# Original Controller (suspected intermittent crashes)
# Relay for control net and USB
UTF::Linux sr1end12

# New controller
UTF::Linux mc14end2 -sta {lan dummy}

UTF::Power::WTI mc14ips1

package require UTF::Aeroflex
UTF::Aeroflex 192.168.1.3:20000/udp -relay mc14end2 \
    -group {D {1 2} B {3 4} C {5 6}} -retries 100

