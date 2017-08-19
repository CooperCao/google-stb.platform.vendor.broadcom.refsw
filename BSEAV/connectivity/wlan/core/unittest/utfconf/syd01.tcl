#
# UTF configuration for syd01 testbed @ Broadcom Sydney
#

package require UTF::Power

##### Directory where test results will be saved
set UTF::SummaryDir "/projects/hnd_sig/$::env(LOGNAME)/syd01"

##### Define power controllers
UTF::Power::Synaccess sig-term-1 \
    -lan_ip 10.160.4.104

##### LAN Endpoint/Relay (Rackmount Dell R200 Controller running FC9)
UTF::Linux sig-ctrl-1 \
    -sta {lan eth1}

##### FC9 Linux(DUT Host Machine) with BCM943224HMS DUT
UTF::Linux sig-dut-1 \
    -sta {4322 eth1} \
    -power {sig-term-1 1}
    
##### Linksys 320N 4717 wireless router.
UTF::Router sig-ref-1 \
    -lan_ip 192.168.1.1 \
    -sta "syd01_AP eth1" \
    -relay "sig-ctrl-1" \
    -power {sig-term-1 2} \
    -lanpeer lan \
    -console sig-ctrl-1:40001 \
    -brand "linux-internal-router" \
    -nvram {
       dhcp_start=192.168.1.100
       dhcp_end=192.168.1.104
       fw_disable=1
       wl0_ssid=syd01_1_ap
       wl0_channel=1
       wl_msglevel=0x101
       antswitch=0
    }
