# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for mc18 testbed
#

#mc18eth1.sj.broadcom.com has address 10.19.84.46
# mc18eth1:15 is trunked to sr1eth05:15
#mc18ips1.sj.broadcom.com has address 10.19.84.59
#mc18tst10.sj.broadcom.com has address 10.19.84.73
#mc18tst1.sj.broadcom.com has address 10.19.84.129
#mc18tst2.sj.broadcom.com has address 10.19.84.130
#mc18tst3.sj.broadcom.com has address 10.19.84.131
#mc18tst4.sj.broadcom.com has address 10.19.84.132
#mc18tst5.sj.broadcom.com has address 10.19.84.133
#mc18tst6.sj.broadcom.com has address 10.19.84.134
#mc18tst7.sj.broadcom.com has address 10.19.84.157
#mc18tst8.sj.broadcom.com has address 10.19.84.158
#mc18tst9.sj.broadcom.com has address 10.19.84.159
#mc18end2.sj.broadcom.com has address 10.19.84.237

# pb5beth1

UTF::Power::WTI mc18ips1

UTF::Linux SR1End16 -sta {wan1 eth1}

package require UTF::Aeroflex
UTF::Aeroflex 192.168.254.25:20000/udp -relay wan1 \
    -group {A {1 2} B {3 4 5 6}}



