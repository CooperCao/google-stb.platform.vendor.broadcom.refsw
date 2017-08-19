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

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.80.25\
    -sta {lan eth1}
lan configure -ipaddr 10.19.80.25

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.80.194 \
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

# 4365a0 PCIe Full Dongle
UTF::DHD 4366_p5r0_st1 \
    -lan_ip 10.19.80.162 \
    -sta "4366_p5r0_1 eth0"\
	-power {Power 1} \
    -power_sta {Power 1} \
    -hostconsole "10.19.80.25:40100"\
    -app_tag trunk \
	-tag trunk \
	-dhd_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4366c0-ram/pcie-ag-splitrx-fdap-mbss-mfgtest-seqcmds-phydbg-phydump-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-11nprop-ringer-dmaindex16-dbgam-dbgams-bgdfs-murx/rtecdc.bin \
	-wlinitcmds {wl msglevel +assoc}
