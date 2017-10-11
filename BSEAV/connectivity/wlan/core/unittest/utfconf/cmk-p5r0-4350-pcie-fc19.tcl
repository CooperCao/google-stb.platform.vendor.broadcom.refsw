# -*-tcl-*-
# Load Packages
#package require UTF::Aeroflex
package require UTF::Linux

set ::UTF::SummaryDir "/projects/hnd_software/work/chongmok/logs/"

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    #ALL attn 0;

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    #AP1 restart wl0_radio=0
    #AP1 restart wl1_radio=0

    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}

# Define power controllers on cart
UTF::Power::WebRelay Power -lan_ip 10.19.80.195 -relay Controller
UTF::Power::WebRelay PowerUSB1 -lan_ip 10.19.80.196 -relay Controller
UTF::Power::WebRelay PowerUSB2 -lan_ip 10.19.80.197 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.80.25\
    -sta {lan eth1}
lan configure -ipaddr 10.19.80.25

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.80.194\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay Controller\
    -lanpeer Controller\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.80.25:40007"\
    -brand linux26-internal-router\
    -txt_override {wandevs=et0}\
    -nvram {
		lan_ipaddr=192.168.1.1
        watchdog=3000
        console_loglevel=7
        fw_disable=1
        router_disable=0
        wl0_ssid=smoke
        wl0_radio=1
        wl0_channel=1
        wl0_obss_coex=0
        wl1_ssid=smoke5
        wl1_radio=1
        wl1_channel=36
        wl1_obss_coex=0
    }

### STA Sections ###

# 4350c2 PCIe-Dongle FC19
#
UTF::DHD 10.19.80.164 \
-sta {4350c2-pcie-sta3 eth0} \
-hostconsole "10.19.80.25:40102"\
-power {Power 3}\
-tag BIS120RC4PHY_BRANCH_7_16 \
-brand linux-external-dongle-pcie \
-driver dhd-msgbuf-pciefd-debug \
-nvram "bcm94350wlpagbe_KA.txt" \
-postinstall {dhd -i eth0 msglevel -event}\
-type 4350c2-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-swdiv/rtecdc.bin \
-wlinitcmds {wl msglevel +assoc}\
-nvram_add {macaddr=00:90:4c:16:60:02} \
-dhd_brand linux-internal-dongle-pcie
