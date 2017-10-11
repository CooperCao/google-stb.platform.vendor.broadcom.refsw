# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#

######
#
# UTF Ramsey shared resources in SW Lab 5010
#

# Power switches
UTF::Power::Synaccess UTFPower6
UTF::Power::Synaccess npc1 -relay UTFTestC -lan_ip 192.168.1.12

# Relay/Controllers
UTF::Linux UTFTestF
UTF::Linux UTFTestC

package require UTF::Aeroflex
UTF::Aeroflex 192.168.1.10:20000/udp -relay UTFTestF \
    -group {R1 {1 2 3 4} R4 {9 10 11 12} R6 {5 6} R7 {7 8}
	ALL {1 2 3 4 5 6 7 8 9 10 11 12}} -retries 20


return

#####################
# Unused...

package require UTF::Panda

UTF::Panda panda -sta {4334panda wlan0} \
    -relay UTFTestS \
    -power {UTFPower6 8} \
    -console {UTFTestS:40002} \
    -hostconsole UTFTestS:40000 \
    -tag PHOENIX2_BRANCH_6_10 \
    -driver "dhd-cdc-sdmmc-android-panda-cfg80211-oob-debug-3.2.0-panda" \
    -noframeburst 1 \
    -nocal 1 \
    -nvram "src/shared/nvram/bcm94334fcagb_android.txt" \
    -type "4334b1-roml/sdio-ag-idsup-p2p-err-assert-sr-dmatxrc{,-autoabn}" \
    -postinstall {dhd -i wlan0 msglevel 0x20001; dhd -i wlan0 sd_divisor 1} \
    -yart {-attn5g 30-95 -attn2g 40-95 -pad 3}

4334panda configure -ipaddr 192.168.1.95

4334panda clone 4334pmfg -lan_ip "shell" \
    -type "4334b0-ram/sdio-ag-pool-mfgtest-seqcmds/rtecdc.bin"

4334panda clone 4334panda116 \
    -tag PHOENIX2_TWIG_6_10_116 \
    -type "4334b1min-roml/sdio-ag-idsup-p2p-err-assert-sr-dmatxrc"

4334panda clone 4334panda177 -tag PHOENIX2_TWIG_6_10_177

4334panda clone 43342panda \
    -type "43342a0-roml/sdio-ag-idsup-p2p-err-assert-sr-dmatxrc" \
    -nvram "bcm94334wlagb.txt" -nvram_add {muxenab=0x11} \

#    -lan_ip utftests:40002 \
#    -ssh /home/tima/src/unittest/apshell -rexec_add_errorcodes 0 \



