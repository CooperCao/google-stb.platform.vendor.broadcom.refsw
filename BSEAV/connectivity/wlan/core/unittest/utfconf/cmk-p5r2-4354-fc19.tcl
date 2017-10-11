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
UTF::Power::WebRelay Power -lan_ip 10.19.80.209 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.80.25\
    -sta {lan eth1}
lan configure -ipaddr 10.19.80.25

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.80.208\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay Controller\
    -lanpeer Controller\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.80.25:45007"\
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

# 4345 FC15 SDIO 3.0
UTF::DHD 4354a1_p5r2_sta1 \
  -lan_ip 10.19.80.179\
  -sta "4354a1_fc19_1 eth0"\
  -power {Power 1} \
  -power_sta {Power 1} \
  -hostconsole "10.19.80.25:45100"\
  -dhd_tag NIGHTLY \
  -dhd_brand linux-external-dongle-sdio \
  -tag BISON_BRANCH_7_10 \
  -app_tag trunk \
  -nvram "bcm94354wlsagbl.txt" \
  -brand linux-external-dongle-sdio \
  -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err \
  -nvram_add {macaddr=01:90:4C:18:64:12}

UTF::DHD 4354a1_p5r2_sta2 \
  -lan_ip 10.19.80.180\
  -sta "4354a1_fc19_2 eth0"\
  -power {Power 2} \
  -power_sta {Power 2} \
  -hostconsole "10.19.80.25:45101"\
  -dhd_tag NIGHTLY \
  -dhd_brand linux-external-dongle-sdio \
  -tag BISON_BRANCH_7_10 \
  -app_tag trunk \
  -nvram "bcm94354wlsagbl.txt" \
  -brand linux-external-dongle-sdio \
  -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err \
  -nvram_add {macaddr=02:90:4C:18:64:08}

UTF::DHD 4354a1_p5r2_sta3 \
  -lan_ip 10.19.80.181\
  -sta "4354a1_fc19_3 eth0"\
  -power {Power 3} \
  -power_sta {Power 3} \
  -hostconsole "10.19.80.25:45102"\
  -dhd_tag NIGHTLY \
  -dhd_brand linux-external-dongle-sdio \
  -tag BISON_BRANCH_7_10 \
  -app_tag trunk \
  -nvram "bcm94354wlsagbl.txt" \
  -brand linux-external-dongle-sdio \
  -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err \
  -nvram_add {macaddr=03:90:4C:18:64:3d}

UTF::DHD 4354a1_p5r2_sta4 \
  -lan_ip 10.19.80.182\
  -sta "4354a1_fc19_4 eth0"\
  -power {Power 4} \
  -power_sta {Power 4} \
  -hostconsole "10.19.80.25:45103"\
  -dhd_tag NIGHTLY \
  -dhd_brand linux-external-dongle-sdio \
  -tag BISON_BRANCH_7_10 \
  -app_tag trunk \
  -nvram "bcm94354wlsagbl.txt" \
  -brand linux-external-dongle-sdio \
  -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err \
  -nvram_add {macaddr=04:90:4C:18:64:28}

UTF::DHD 4354a1_p5r2_sta5 \
  -lan_ip 10.19.80.183\
  -sta "4354a1_fc19_5 eth0"\
  -power {Power 5} \
  -power_sta {Power 5} \
  -hostconsole "10.19.80.25:45104"\
  -dhd_tag NIGHTLY \
  -dhd_brand linux-external-dongle-sdio \
  -tag BISON_BRANCH_7_10 \
  -app_tag trunk \
  -nvram "bcm94354wlsagbl.txt" \
  -brand linux-external-dongle-sdio \
  -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err \
  -nvram_add {macaddr=05:90:4C:18:64:19}

UTF::DHD 4354a1_p5r2_sta6 \
  -lan_ip 10.19.80.184\
  -sta "4354a1_fc19_6 eth0"\
  -power {Power 6} \
  -power_sta {Power 6} \
  -hostconsole "10.19.80.25:45105"\
  -dhd_tag NIGHTLY \
  -dhd_brand linux-external-dongle-sdio \
  -tag BISON_BRANCH_7_10 \
  -app_tag trunk \
  -nvram "bcm94354wlsagbl.txt" \
  -brand linux-external-dongle-sdio \
  -type 4354a1-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-txbf-pktctx-dmatxrc-okc-tdls-ccx-ve-mfp-ltecxgpio-idsup-idauth-assert-err \
  -nvram_add {macaddr=06:90:4C:18:64:48}
