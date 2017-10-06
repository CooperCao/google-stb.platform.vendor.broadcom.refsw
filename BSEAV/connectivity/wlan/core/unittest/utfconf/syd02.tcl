#
# UTF configuration for syd02 testbed @ Broadcom Sydney
#

package require UTF::Power

##### Directory where test results will be saved
set UTF::SummaryDir "/projects/hnd_sig/$::env(LOGNAME)/syd02"

##### Define power controllers
UTF::Power::Synaccess sig-term-2 \
    -lan_ip 10.160.4.109

##### LAN Endpoint/Relay (Rackmount Dell R200 Controller running FC9)
UTF::Linux sig-ctrl-2 \
    -sta {lan eth1}

##### FC9 Linux(DUT Host Machine)
UTF::DHD sig-dut-2 \
    -sta {4329 eth1} \
    -tag ROMTERM_BRANCH_4_218 \
    -nvram "4329b1/bcm94329sdagb.txt" \
    -brand linux-internal-dongle \
    -type "4329b1/sdio-g-cdc-full11n-reclaim-roml-wme-idsup" \
    -power {sig-term-2 1}

##### Linksys 320N 4717 wireless router.
UTF::Router sig-ref-2 \
    -lan_ip 192.168.1.1 \
    -sta "syd02_AP eth1" \
    -relay "sig-ctrl-2" \
    -power {sig-term-2 2} \
    -lanpeer lan \
    -console "sig-ctrl-2:40001" \
    -brand "linux-internal-router" \
    -nvram {
       dhcp_start=192.168.1.105
       dhcp_end=192.168.1.109
       fw_disable=1
       wl0_ssid=syd02_1_ap
       wl0_channel=1
       wl_msglevel=0x101
       antswitch=0
    }
