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
UTF::Power::WebRelay Power -lan_ip 10.19.80.67 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.80.23\
    -sta {lan eth1}
lan configure -ipaddr 10.19.80.23

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.80.66\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay Controller\
    -lanpeer Controller\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.80.23:40007"\
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

# 43452 PCIe Full Dongle
UTF::DHD 43452_p3r0_st1\
    -lan_ip 10.19.80.34\
	-sta "Fc19_43452_p3r0_1 eth0" \
	-power {Power 1} \
    -power_sta {Power 1} \
    -hostconsole "10.19.80.23:40100"\
	-tag BIS747T144RC2_BRANCH_7_64 \
	-dhd_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
    -clm_blob 43452_hans.clm_blob \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-external-dongle-pcie \
	-type 43452a3-roml/pcie-ag-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-txbf-logtrace_pcie-srscan-clm_min-ecounters-bssinfo-ltecx-txpwrcap-err-proxd-idauth-txcal-p5txgtbl-assert/rtecdc.bin \
     -nvram "bcm94345Corona2TDKKK.txt" \
     -nvram_add {macaddr=00:90:4c:c5:12:38} \
    -wlinitcmds {wl country XZ/248}\

UTF::DHD 43452_p3r0_st2\
    -lan_ip 10.19.80.35\
	-sta "Fc19_43452_p3r0_2 eth0" \
	-power {Power 2} \
    -power_sta {Power 2} \
    -hostconsole "10.19.80.23:40101"\
	-tag BIS747T144RC2_BRANCH_7_64 \
	-dhd_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
    -clm_blob 43452_hans.clm_blob \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-external-dongle-pcie \
	-type 43452a3-roml/pcie-ag-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-txbf-logtrace_pcie-srscan-clm_min-ecounters-bssinfo-ltecx-txpwrcap-err-proxd-idauth-txcal-p5txgtbl-assert/rtecdc.bin \
     -nvram "bcm94345Corona2TDKKK.txt" \
     -nvram_add {macaddr=00:90:4c:c5:12:39} \
    -wlinitcmds {wl country XZ/248}

UTF::DHD 43452_p3r0_st3\
    -lan_ip 10.19.80.36\
	-sta "Fc19_43452_p3r0_3 eth0" \
	-power {Power 3} \
    -power_sta {Power 3} \
    -hostconsole "10.19.80.23:40102"\
	-tag BIS747T144RC2_BRANCH_7_64 \
	-dhd_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
    -clm_blob 43452_hans.clm_blob \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-external-dongle-pcie \
	-type 43452a3-roml/pcie-ag-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-txbf-logtrace_pcie-srscan-clm_min-ecounters-bssinfo-ltecx-txpwrcap-err-proxd-idauth-txcal-p5txgtbl-assert/rtecdc.bin \
     -nvram "bcm94345Corona2TDKKK.txt" \
     -nvram_add {macaddr=00:90:4c:c5:12:40} \
    -wlinitcmds {wl country XZ/248}

UTF::DHD 43452_p3r0_st4\
    -lan_ip 10.19.80.37\
	-sta "Fc19_43452_p3r0_4 eth0" \
	-power {Power 4} \
    -power_sta {Power 4} \
    -hostconsole "10.19.80.23:40103"\
	-tag BIS747T144RC2_BRANCH_7_64 \
	-dhd_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
    -clm_blob 43452_hans.clm_blob \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-external-dongle-pcie \
	-type 43452a3-roml/pcie-ag-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-txbf-logtrace_pcie-srscan-clm_min-ecounters-bssinfo-ltecx-txpwrcap-err-proxd-idauth-txcal-p5txgtbl-assert/rtecdc.bin \
     -nvram "bcm94345Corona2TDKKK.txt" \
     -nvram_add {macaddr=00:90:4c:c5:12:41} \
    -wlinitcmds {wl country XZ/248}

UTF::DHD 43452_p3r0_st5\
    -lan_ip 10.19.80.38\
	-sta "Fc19_43452_p3r0_5 eth0" \
	-power {Power 5} \
    -power_sta {Power 5} \
    -hostconsole "10.19.80.23:40104"\
	-tag BIS747T144RC2_BRANCH_7_64 \
	-dhd_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
    -clm_blob 43452_hans.clm_blob \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-external-dongle-pcie \
	-type 43452a3-roml/pcie-ag-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-txbf-logtrace_pcie-srscan-clm_min-ecounters-bssinfo-ltecx-txpwrcap-err-proxd-idauth-txcal-p5txgtbl-assert/rtecdc.bin \
     -nvram "bcm94345Corona2TDKKK.txt" \
     -nvram_add {macaddr=00:90:4c:c5:12:42} \
    -wlinitcmds {wl country XZ/248}

UTF::DHD 43452_p3r0_st6\
    -lan_ip 10.19.80.39\
	-sta "Fc19_43452_p3r0_6 eth0" \
	-power {Power 6} \
    -power_sta {Power 6} \
    -hostconsole "10.19.80.23:40105"\
	-tag BIS747T144RC2_BRANCH_7_64 \
	-dhd_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
    -clm_blob 43452_hans.clm_blob \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-external-dongle-pcie \
	-type 43452a3-roml/pcie-ag-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-txbf-logtrace_pcie-srscan-clm_min-ecounters-bssinfo-ltecx-txpwrcap-err-proxd-idauth-txcal-p5txgtbl-assert/rtecdc.bin \
     -nvram "bcm94345Corona2TDKKK.txt" \
     -nvram_add {macaddr=00:90:4c:c5:12:38} \
    -wlinitcmds {wl country XZ/248}
