#
# UTF configuration for syd03 testbed @ Broadcom Sydney
#

package require UTF::Power

##### Directory where test results will be saved
set UTF::SummaryDir "/projects/hnd_sig/$::env(LOGNAME)/syd03"

##### Define power controllers
UTF::Power::Synaccess sig-term-3 \
    -lan_ip 10.160.4.114

##### LAN Endpoint/Relay (Rackmount Dell R200 Controller running FC9)
UTF::Linux sig-ctrl-3 \
    -sta {lan eth1} -onall false

###### FC9 Linux(DUT Host Machine)
#UTF::DHD sig-dut-3 \
#    -sta {4329 eth1} \
#    -tag ROMTERM_BRANCH_4_218 \
#    -nvram "4329b1/bcm94329sdagb.txt" \
#    -brand linux-internal-dongle \
#    -type "4329b1/sdio-g-cdc-full11n-reclaim-roml-wme-idsup" \
#    -power {sig-term-3 1}

# 43224XP
UTF::Cygwin sig-dut-3 \
   -sta {43224XP} -user hwlab \
   -installer inf -debuginf 1 \
   -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu}} \
   -pre_perf_hook {{%S wl ampdu_clear_dump}} \
   -power {sig-term-3 1} -power_button {auto}

#43224XP clone 43224XP-bass -tag BASS_BRANCH_5_60*
#43224XP clone 43224XP-krn -tag  KIRIN_BRANCH_5_100
#43224XP clone 43224XP-rel5 -tag BASS_REL_5_60_48_*
#43224XP clone 43224XP-rel7 -tag BASS_REL_5_60_???
#43224XP clone 43224XP-rel8 -tag  KIRIN_REL_5_100_???
#43224XP clone 43224XP-rel9 -tag  KIRIN_REL_5_100_9_*
#43224XP clone 43224XP-rel10 -tag  KIRIN_REL_5_100_57_*
#43224XP clone 43224XP-rel11 -tag  KIRIN_REL_5_100_82_*
#43224XP clone 43224XP-rel12 -tag  KIRIN_REL_5_100_98_*
#43224XP clone 43224XP-rel13 -tag  KIRIN_REL_5_100_198_*

#43224XP clone 43224XPext  -brand win_external_wl -type Bcm -tag NIGHTLY
#43224XP clone 43224XPext-krn  -brand win_external_wl -type Bcm -tag KIRIN_BRANCH_5_100
#43224XP clone 43224XPext-rel8  -brand win_external_wl -type Bcm -tag KIRIN_REL_5_100_???

43224XP clone 43224XPext-xxx  -brand win_internal_wl -type Bcm -tag KIRIN_REL_5_100_135


##### Linksys 320N 4717 wireless router.
UTF::Router sig-ref-3 \
    -lan_ip 192.168.1.1 \
    -sta {syd03_AP eth1} \
    -console "sig-ctrl-3:40001" \
    -relay "sig-ctrl-3" \
    -power {sig-term-3 2} \
    -lanpeer lan \
    -brand linux-internal-router \
    -nvram {
       dhcp_start=192.168.1.110
       dhcp_end=192.168.1.114
       fw_disable=1
       wl0_ssid=syd03_1_ap
       wl0_channel=1
       wl_msglevel=0x101
       antswitch=0
    }

