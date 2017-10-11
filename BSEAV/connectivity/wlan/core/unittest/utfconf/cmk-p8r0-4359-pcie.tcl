# -*-tcl-*-
# Load Packages
#package require UTF::Aeroflex
package require UTF::Linux

#set ::UTF::SummaryDir "/projects/hnd_software/work/mbeyer/logs/"

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
UTF::Power::WebRelay Power -lan_ip 10.19.91.195 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.91.25\
    -sta {lan eth1}
lan configure -ipaddr 10.19.91.25


# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.91.194\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay lan\
    -lanpeer lan\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.91.25:40007"\
    -brand linux26-internal-router\
    -tag "AKASHI_REL_5_110_35"\
    -txt_override {wandevs=et0}\
    -nvram {
		lan_ipaddr=192.168.1.1
		lan_gateway=192.168.1.1
		dhcp_start=192.168.1.100
		dhcp_end=192.168.1.150
		lan1_ipaddr=192.168.2.1
		lan1_gateway=192.169.2.1
		dhcp1_start=192.168.2.100
   		dhcp1_end=192.168.2.150
		watchdog=3000
		console_loglevel=7
		wl0_ssid=smoke
		wl0_radio=1
		wl0_channel=11
		wl0_obss_coex=0
		fw_disable=1
		# No DHCP server when router disable 1 is set
		router_disable=0
		wl1_ssid=4331
		wl1_radio=1
		wl1_channel=36
		wl1_obss_coex=0
		wl_msglevel2=0x8
	}

### STA Sections ###
### 4359b0 PCIe-Dongle STAs ###

UTF::DHD Fc19_4359_p8r0s1_sta \
-lan_ip 10.19.91.162\
-user root\
-hostconsole "10.19.91.25:40000" \
-sta {fc19_4359_pcie_sta1 eth0} \
-power {Power 1}\
-dhd_brand linux-internal-dongle-pcie \
-driver dhd-msgbuf-pciefd-debug \
-brand linux-external-dongle-pcie \
-dhd_tag DHD_BRANCH_1_363 \
-tag  DINGO07T_BRANCH_9_35\
-app_tag trunk \
-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx/rtecdc.bin \
-nvram bcm943593fcpagbss.txt

UTF::DHD Fc19_4359_p8r0s2_sta \
-lan_ip 10.19.91.163\
-user root\
-hostconsole "10.19.91.25:40001" \
-sta {fc19_4359_pcie_sta2 eth0} \
-power {Power 2}\
-dhd_brand linux-internal-dongle-pcie \
-driver dhd-msgbuf-pciefd-debug \
-brand linux-external-dongle-pcie \
-dhd_tag DHD_BRANCH_1_363 \
-tag  DINGO07T_BRANCH_9_35\
-app_tag trunk \
-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx/rtecdc.bin \
-nvram bcm943593fcpagbss.txt

UTF::DHD Fc19_4359_p8r0s3_sta \
-lan_ip 10.19.91.164\
-user root\
-hostconsole "10.19.91.25:40002" \
-sta {fc19_4359_pcie_sta3 eth0} \
-power {Power 3}\
-dhd_brand linux-internal-dongle-pcie \
-driver dhd-msgbuf-pciefd-debug \
-brand linux-external-dongle-pcie \
-dhd_tag DHD_BRANCH_1_363 \
-tag  DINGO07T_BRANCH_9_35\
-app_tag trunk \
-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx/rtecdc.bin \
-nvram bcm943593fcpagbss.txt

UTF::DHD Fc19_4359_p8r0s4_sta \
-lan_ip 10.19.91.165\
-user root\
-hostconsole "10.19.91.25:40003" \
-sta {fc19_4359_pcie_sta4 eth0} \
-power {Power 4}\
-dhd_brand linux-internal-dongle-pcie \
-driver dhd-msgbuf-pciefd-debug \
-brand linux-external-dongle-pcie \
-dhd_tag DHD_BRANCH_1_363 \
-tag  DINGO07T_BRANCH_9_35\
-app_tag trunk \
-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx/rtecdc.bin \
-nvram bcm943593fcpagbss.txt

UTF::DHD Fc19_4359_p8r0s5_sta \
-lan_ip 10.19.91.166\
-user root\
-hostconsole "10.19.91.25:40004" \
-sta {fc19_4359_pcie_sta5 eth0} \
-power {Power 5}\
-dhd_brand linux-internal-dongle-pcie \
-driver dhd-msgbuf-pciefd-debug \
-brand linux-external-dongle-pcie \
-dhd_tag DHD_BRANCH_1_363 \
-tag  DINGO07T_BRANCH_9_35\
-app_tag trunk \
-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx/rtecdc.bin \
-nvram bcm943593fcpagbss.txt

UTF::DHD Fc19_4359_p8r0s6_sta \
-lan_ip 10.19.91.167\
-user root\
-hostconsole "10.19.91.25:40005" \
-sta {fc19_4359_pcie_sta6 eth0} \
-power {Power 6}\
-dhd_brand linux-internal-dongle-pcie \
-driver dhd-msgbuf-pciefd-debug \
-brand linux-external-dongle-pcie \
-dhd_tag DHD_BRANCH_1_363 \
-tag  DINGO07T_BRANCH_9_35\
-app_tag trunk \
-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx/rtecdc.bin \
-nvram bcm943593fcpagbss.txt
