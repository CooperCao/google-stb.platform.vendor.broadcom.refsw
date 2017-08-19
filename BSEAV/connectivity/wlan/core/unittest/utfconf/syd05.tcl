#
# UTF configuration for syd05 testbed @ Broadcom Sydney
#

package require UTF::Power

##### Directory where test results will be saved
set UTF::SummaryDir "/projects/hnd_sig/$::env(LOGNAME)/syd05"

###### Define power controllers
UTF::Power::Synaccess sig-term-5 \
    -lan_ip 10.160.4.125

#### LAN Endpoint/Relay (Rackmount Dell R200 Controller running FC9)
UTF::Linux sig-ctrl-5 \
    -sta {lan eth1}

##### FC9 Linux(DUT Host Machine)
UTF::DHD sig-dut-5 \
    -sta {4329 eth1} \
    -tag ROMTERM_BRANCH_4_218 \
    -nvram "4329b1/bcm94329sdagb.txt" \
    -brand linux-internal-dongle \
    -type "4329b1/sdio-g-cdc-full11n-reclaim-roml-wme-idsup" \
    -power {sig-term-5 1}

##### Linksys 320N 4717 wireless router.
UTF::Router sig-ref-5 \
    -lan_ip 192.168.1.1 \
    -sta "syd05_AP eth1" \
    -relay "sig-ctrl-5" \
    -power {sig-term-5 2} \
    -lanpeer lan \
    -console "sig-ctrl-5:40001" \
    -brand "linux-internal-router" \
    -nvram {
       dhcp_start=192.168.1.121
       dhcp_end=192.168.1.125
       fw_disable=1
       wl0_ssid=syd05_1_ap
       wl0_channel=1
       wl_msglevel=0x101
       antswitch=0
    }
