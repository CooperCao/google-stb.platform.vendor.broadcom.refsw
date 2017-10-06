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
UTF::Power::WebRelay Power -lan_ip 10.19.91.138 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.91.24\
    -sta {lan eth1}
lan configure -ipaddr 10.19.91.24

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.91.137\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay Controller\
    -lanpeer Controller\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.91.24:43007"\
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

# FC19x64 43602a1 PCIe Full Dongle
UTF::DHD 43602_p7r1_st1 \
    -lan_ip 10.19.91.107 \
    -sta "43602_p7r1_1 eth0"\
	-power {Power 1} \
    -power_sta {Power 1} \
    -hostconsole "10.19.91.24:43000"\
	-tag BISON_BRANCH_7_10 \
	-dhd_tag NIGHTLY \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 43602a1-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx-err-assert/rtecdc.bin \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43602_p7r1_st2 \
    -lan_ip 10.19.91.108 \
    -sta "43602_p7r1_2 eth0"\
	-power {Power 2} \
    -power_sta {Power 2} \
    -hostconsole "10.19.91.24:43001"\
	-tag BISON_BRANCH_7_10 \
	-dhd_tag NIGHTLY \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 43602a1-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx-err-assert/rtecdc.bin \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43602_p7r1_st3 \
    -lan_ip 10.19.91.109 \
    -sta "43602_p7r1_3 eth0"\
	-power {Power 3} \
    -power_sta {Power 3} \
    -hostconsole "10.19.91.24:43002"\
	-tag BISON_BRANCH_7_10 \
	-dhd_tag NIGHTLY \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 43602a1-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx-err-assert/rtecdc.bin \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43602_p7r1_st4 \
    -lan_ip 10.19.91.110 \
    -sta "43602_p7r1_4 eth0"\
	-power {Power 4} \
    -power_sta {Power 4} \
    -hostconsole "10.19.91.24:43003"\
	-tag BISON_BRANCH_7_10 \
	-dhd_tag NIGHTLY \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 43602a1-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx-err-assert/rtecdc.bin \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43602_p7r1_st5 \
    -lan_ip 10.19.91.111 \
    -sta "43602_p7r1_5 eth0"\
	-power {Power 5} \
    -power_sta {Power 5} \
    -hostconsole "10.19.91.24:43004"\
	-tag BISON_BRANCH_7_10 \
	-dhd_tag NIGHTLY \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 43602a1-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx-err-assert/rtecdc.bin \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43602_p7r1_st6 \
    -lan_ip 10.19.91.112 \
    -sta "43602_p7r1_6 eth0"\
	-power {Power 6} \
    -power_sta {Power 6} \
    -hostconsole "10.19.91.24:43005"\
	-tag BISON_BRANCH_7_10 \
	-dhd_tag NIGHTLY \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 43602a1-roml/pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive-splitrx-err-assert/rtecdc.bin \
	-wlinitcmds {wl msglevel +assoc}
