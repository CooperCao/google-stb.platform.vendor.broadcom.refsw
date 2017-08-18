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
UTF::Power::WebRelay Power -lan_ip 10.19.91.67 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.91.23\
    -sta {lan eth1}
lan configure -ipaddr 10.19.91.23

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.91.66\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay Controller\
    -lanpeer Controller\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.91.23:40007"\
    -brand linux-2.6.36-arm-internal-router\
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

# 4355c0 PCIe Full Dongle
UTF::DHD 4355c0_p6r0_st1 \
    -lan_ip 10.19.91.34 \
    -sta "4355c0_p6r0_1 eth0"\
	-power {Power 1} \
    -power_sta {Power 1} \
    -hostconsole "10.19.91.23:40000"\
	-tag DIN2915T250RC1_BRANCH_9_30 \
    -dhd_tag DHD_BRANCH_1_359 \
    -app_tag DHD_BRANCH_1_359 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4355c0-roml/config_pcie_release/rtecdc.bin \
    -nvram "bcm94355KristoffMurataMM.txt" \
    -clm_blob "4355_kristoff.clm_blob" \
	-wlinitcmds {wl msglevel +assoc}
