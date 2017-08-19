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
UTF::Power::WebRelay Power -lan_ip 10.19.90.145 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.90.24\
    -sta {lan eth1}
lan configure -ipaddr 10.19.90.24

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.90.144\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay Controller\
    -lanpeer Controller\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.90.24:40207"\
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

# 4355c0 PCIe Full Dongle
UTF::DHD 4355c0_p1r2_st1 \
    -lan_ip 10.19.90.116 \
    -sta "4355c0_p1r2_1 eth0"\
	-power {Power 1} \
    -power_sta {Power 1} \
    -hostconsole "10.19.90.24:40200"\
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

UTF::DHD 4355c0_p1r2_st2 \
    -lan_ip 10.19.90.117 \
    -sta "4355c0_p1r2_2 eth0"\
	-power {Power 2} \
    -power_sta {Power 2} \
    -hostconsole "10.19.90.24:40201"\
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

UTF::DHD 4355c0_p1r2_st3 \
    -lan_ip 10.19.90.118 \
    -sta "4355c0_p1r2_3 eth0"\
	-power {Power 3} \
    -power_sta {Power 3} \
    -hostconsole "10.19.90.24:40202"\
	-tag DIN2915T250RC1_BRANCH_9_30 \
    -dhd_tag DHD_BRANCH_1_359 \
    -app_tag DHD_BRANCH_1_359 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4355c0-roml/config_pcie_release/rtecdc.bin \
    -nvram "bcm94355KristoffMurataMM.txt" \
    -clm_blob "4355_kristoff.clm_blob" \

UTF::DHD 4355c0_p1r2_st4 \
    -lan_ip 10.19.90.119 \
    -sta "4355c0_p1r2_4 eth0"\
	-power {Power 4} \
    -power_sta {Power 4} \
    -hostconsole "10.19.90.24:40203"\
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

UTF::DHD 4355c0_p1r2_st5 \
    -lan_ip 10.19.90.120 \
    -sta "4355c0_p1r2_5 eth0"\
	-power {Power 5} \
    -power_sta {Power 5} \
    -hostconsole "10.19.90.24:40204"\
	-tag DIN2915T250RC1_BRANCH_9_30 \
    -dhd_tag DHD_BRANCH_1_359 \
    -app_tag DHD_BRANCH_1_359 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4355c0-roml/config_pcie_release/rtecdc.bin \
    -nvram "bcm94355KristoffMurataMM.txt" \
    -clm_blob "4355_kristoff.clm_blob" \

UTF::DHD 4355c0_p1r2_st6 \
    -lan_ip 10.19.90.121 \
    -sta "4355c0_p1r2_6 eth0"\
	-power {Power 6} \
    -power_sta {Power 6} \
    -hostconsole "10.19.90.24:40205"\
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
