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
UTF::Power::WebRelay Power -lan_ip 10.19.91.145 -relay Controller
UTF::Power::WebRelay PowerUSB1 -lan_ip 10.19.91.145 -relay Controller
UTF::Power::WebRelay PowerUSB2 -lan_ip 10.19.91.145 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.91.24\
    -sta {lan eth1}
lan configure -ipaddr 10.19.91.24

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.91.144\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay Controller\
    -lanpeer Controller\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.91.24:42007"\
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
UTF::DHD 43430_p7r2_sta1\
    -lan_ip 10.19.91.116\
    -sta {43430_p7r2_1 eth0}\
	-power {Power 1} \
    -power_sta {Power 1} \
    -hostconsole "10.19.91.24:42000"\
    -app_tag trunk\
    -dhd_tag DHD_BRANCH_1_363\
    -tag DINGO07T_BRANCH_9_35\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-assert/rtecdc.bin \
	-nvram bcm943430fsdng_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43430_p7r2_sta2\
    -lan_ip 10.19.91.117\
    -sta {43430_p7r2_2 eth0}\
	-power {Power 2} \
    -power_sta {Power 2} \
    -hostconsole "10.19.91.24:42001"\
    -app_tag trunk\
    -dhd_tag DHD_BRANCH_1_363\
    -tag DINGO07T_BRANCH_9_35\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-assert/rtecdc.bin \
	-nvram bcm943430fsdng_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43430_p7r2_sta3\
    -lan_ip 10.19.91.118\
    -sta {43430_p7r2_3 eth0}\
	-power {Power 3} \
    -power_sta {Power 3} \
    -hostconsole "10.19.91.24:42006"\
    -app_tag trunk\
    -dhd_tag DHD_BRANCH_1_363\
    -tag DINGO07T_BRANCH_9_35\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-assert/rtecdc.bin \
	-nvram bcm943430fsdng_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43430_p7r2_sta4\
    -lan_ip 10.19.91.119\
    -sta {43430_p7r2_4 eth0}\
	-power {Power 4} \
    -power_sta {Power 4} \
    -hostconsole "10.19.91.24:42003"\
    -app_tag trunk\
    -dhd_tag DHD_BRANCH_1_363\
    -tag DINGO07T_BRANCH_9_35\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-assert/rtecdc.bin \
	-nvram bcm943430fsdng_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43430_p7r2_sta5\
    -lan_ip 10.19.91.120\
    -sta {43430_p7r2_5 eth0}\
	-power {Power 5} \
    -power_sta {Power 5} \
    -hostconsole "10.19.91.24:42004"\
    -app_tag trunk\
    -dhd_tag DHD_BRANCH_1_363\
    -tag DINGO07T_BRANCH_9_35\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43430b0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srmem-srfast-11nprop-tdls-assert/rtecdc.bin \
	-nvram bcm943430fsdng_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}
