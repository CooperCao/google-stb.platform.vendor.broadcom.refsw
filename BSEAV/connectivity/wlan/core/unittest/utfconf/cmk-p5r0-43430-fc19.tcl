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

# FC19x64 43430 SDIO 2.0
UTF::DHD 43430_p5r0_sta4\
    -lan_ip 10.19.80.165\
    -sta {43430_p5r0_4 eth0}\
	-power {Power 4} \
    -power_sta {Power 4} \
    -hostconsole "10.19.80.25:40103"\
    -app_tag trunk\
    -dhd_tag DHD_BRANCH_1_363\
    -tag DINGO07T_BRANCH_9_35\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-assert/rtecdc.bin \
	-nvram bcm943430fsdng_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}
