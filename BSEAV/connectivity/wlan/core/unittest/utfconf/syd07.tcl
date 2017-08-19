#
# UTF configuration for syd07 testbed @ Broadcom Sydney
#

##### Test bed is physically setup for rate vs. range
##### testing of a BCM94329sdagb DUT, uses an Aeroflex attenuator,
##### and a Linksys WRT320N as the AP.

package require UTF::Power
package require UTF::Aeroflex

##### Directory where test results will be saved
set ::UTF::SummaryDir "/projects/hnd_sig/$::env(LOGNAME)/syd07"

##### Define power controllers
UTF::Power::Synaccess sig-rvr-ref-term \
    -lan_ip 10.160.4.146

UTF::Power::Synaccess sig-rvr-dut-term \
    -lan_ip 10.160.4.139

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL attn 0;

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    # AP1 restart wl0_radio=0
    # delete myself
    unset ::UTF::SetupTestBed

    return
}

##### LAN Endpoint/Relay (Rackmount Dell R200 Controller running FC9)
UTF::Linux sig-rvr-ctrl-1 \
    -sta {lan eth1}

##### FC9 Linux(DUT Host Machine)
UTF::DHD sig-rvr-dut-1 \
    -sta {4329 eth1} \
    -type "4329b1/sdio-g-cdc-full11n-reclaim-roml-wme-idsup" \
    -nvram "4329b1/bcm94329sdagb.txt" \
    -brand linux-internal-dongle \
    -tag ROMTERM_BRANCH_4_218 \
    -power {sig-rvr-dut-term 1}

##### Linksys 320N 4717 wireless router.
UTF::Router sig-rvr-ref-1 \
        -sta "syd07_AP eth1" \
        -lan_ip 192.168.1.1 \
        -console "sig-rvr-ctrl-1:40001" \
        -relay "sig-rvr-ctrl-1" \
        -power {sig-rvr-ref-term 1} \
        -lanpeer lan \
        -brand "linux-internal-router" \
        -nvram {
                lan_ipaddr=192.168.1.1
                lan_gateway=192.168.1.1
                dhcp_start=192.168.1.100
                dhcp_end=192.168.1.149
                lan1_ipaddr=192.168.2.1
                lan1_gateway=192.169.2.1
                dhcp1_start=192.168.2.100
                dhcp1_end=192.168.2.149
                router_disable=0
                fw_disable=1
                wl_msglevel=0x101
                wl0_ssid=syd07_1_ap
                wl0_channel=1
                wl0_radio=0
                antswitch=0
                wl0_obss_coex=0
        }

##### Attenuator - Aeroflex 6977
UTF::Aeroflex sig-rvr-atten-1 \
        -lan_ip 10.160.4.135 \
        -relay "sig-rvr-ctrl-1" \
        -group {G1 {1} ALL {1 2 3 4 5 6}}

