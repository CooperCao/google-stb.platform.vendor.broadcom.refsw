# -*-tcl-*-
# Load Packages
#package require UTF::Aeroflex
package require UTF::Linux

#set ::UTF::SummaryDir "/projects/hnd_software/work/chongmok/logs/"

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
UTF::Power::WebRelay Power -lan_ip 10.19.91.131 -relay Controller
UTF::Power::WebRelay PowerUSB1 -lan_ip 10.19.91.132 -relay Controller
UTF::Power::WebRelay PowerUSB2 -lan_ip 10.19.91.132 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.91.24\
    -sta {lan eth1}
lan configure -ipaddr 10.19.91.24

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.91.130\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay Controller\
    -lanpeer Controller\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.91.24:40007"\
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

# 43526 USB BMac Dongle on FC15
UTF::DHD 10.19.91.98 -sta {43526_p7r0_1 eth0}\
    -tag BISON_BRANCH_7_10 \
	-user root \
	-power {Power 1} \
    -power_sta {PowerUSB1 1} \
    -console "10.19.91.24:41000"\
    -hostconsole "10.19.91.24:40000"\
    -wlinitcmds {wl msglevel +assoc}\
    -nvram bcm943526usb_p452.txt \
	-brand linux-internal-wl \
    -driver "debug-apdef-stadef-high-p2p-mchan-tdls"\
	-type "43526b-bmac/ag-assert-p2p-mchan/rtecdc.bin.trx"
