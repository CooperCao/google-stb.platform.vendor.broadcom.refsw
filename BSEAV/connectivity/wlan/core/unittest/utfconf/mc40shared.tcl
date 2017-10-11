# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for MC41testbed
#

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc40"

# Available:
# 10.19.86.78	mc40end1	tima	utflab	linux desktop
# 10.19.86.79	mc40end2	tima	utflab	linux desktop
# 10.19.86.80	mc40tst1	tima	utflab	laptop
# 10.19.86.81	mc40tst2	tima	utflab	laptop
# 10.19.86.82	mc40tst3	tima	utflab	laptop
# 10.19.86.83	mc40tst4	tima	utflab	laptop
# 10.19.86.84	mc40tst5	tima	utflab	laptop
# 10.19.86.85	mc40tst6	tima	utflab	laptop

# Switch 192.168.1.254

# Controller
UTF::Linux mc40end1 -sta {lan eth0}

# Power
UTF::Power::WebSwitch ws -relay mc40end1 -lan_ip 192.168.1.12
UTF::Power::Synaccess nb -relay mc40end1 -lan_ip 192.168.1.13

# Attenuator (console mc40end1:40009)
package require UTF::Aeroflex
UTF::Aeroflex 192.168.1.11:20000/udp -relay mc40end1 \
    -group {G {1 2} B {3 4 5} C {6 7 8}} -retries 100


