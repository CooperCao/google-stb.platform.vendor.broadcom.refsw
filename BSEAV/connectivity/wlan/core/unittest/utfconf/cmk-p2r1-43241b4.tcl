# -*-tcl-*-
# Load Packages
#package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Panda

#
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
UTF::Power::WebRelay Power -lan_ip 10.19.90.195 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.90.25\
    -sta {lan eth1}
lan configure -ipaddr 10.19.90.25


# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.90.201\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay lan\
    -lanpeer lan\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.90.25:41007"\
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
		wl1_ssid=smoke5
		wl1_radio=1
		wl1_channel=11
		wl1_obss_coex=0
		wl_msglevel2=0x8
	}

### STA Sections ###

UTF::Linux 10.19.90.171
UTF::Panda 43241_p2r1_relay1\
-sta {43241panda_1 wlan0} \
-relay 10.19.90.171\
-power {Power 1}\
-console "10.19.90.25:40200"\
-hostconsole "10.19.90.25:40100"\
-driver "dhd-cdc-sdmmc-android-panda-cfg80211-oob-debug-3.2.0-panda" \
-dhd_tag  NIGHTLY \
-dhd_brand android-external-dongle-sdio\
-brand linux-internal-dongle \
-tag PHOENIX2_BRANCH_6_10 \
-type 43241b4-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr-proptxstatus-vsdb-srvsdb-opt1-autoabn \
-nvram "bcm943241ipaagb_p100_panda.txt" \
-nvram_add "macaddr=00:90:4c:c5:17:30"\
-postinstall {dhd -i wlan0 msglevel 0x20001; dhd -i wlan0 sd_divisor 1} \
-customer android

UTF::Linux 10.19.90.172
UTF::Panda 43241_p2r1_relay2 \
-sta {43241panda_2 wlan0} \
-relay 10.19.90.172\
-power {Power 2}\
-console "10.19.90.25:40201" \
-hostconsole "10.19.90.25:40101"\
-driver "dhd-cdc-sdmmc-android-panda-cfg80211-oob-debug-3.2.0-panda" \
-dhd_tag  NIGHTLY \
-dhd_brand android-external-dongle-sdio\
-brand linux-internal-dongle \
-tag PHOENIX2_BRANCH_6_10 \
-type 43241b4-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr-proptxstatus-vsdb-srvsdb-opt1-autoabn \
-nvram "bcm943241ipaagb_p100_panda.txt" \
-nvram_add "macaddr=00:90:4c:c5:17:31"\
-postinstall {dhd -i wlan0 msglevel 0x20001; dhd -i wlan0 sd_divisor 1} \
-customer android

UTF::Linux 10.19.90.173
UTF::Panda 43241_p2r1_relay3 \
-sta {43241panda_3 wlan0} \
-relay 10.19.90.173\
-power {Power 3}\
-console "10.19.90.25:40202" \
-hostconsole "10.19.90.25:40102" \
-driver "dhd-cdc-sdmmc-android-panda-cfg80211-oob-debug-3.2.0-panda" \
-dhd_tag  NIGHTLY \
-dhd_brand android-external-dongle-sdio\
-brand linux-internal-dongle \
-tag PHOENIX2_BRANCH_6_10 \
-type 43241b4-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr-proptxstatus-vsdb-srvsdb-opt1-autoabn\
-nvram "bcm943241ipaagb_p100_panda.txt" \
-nvram_add "macaddr=00:90:4c:c5:17:32"\
-postinstall {dhd -i wlan0 msglevel 0x20001; dhd -i wlan0 sd_divisor 1} \
-customer android

UTF::Linux 10.19.90.174
UTF::Panda 43241_p2r1_relay4 \
-sta {43241panda_4 wlan0} \
-relay 10.19.90.174\
-power {Power 4}\
-console "10.19.90.25:40203" \
-hostconsole "10.19.90.25:40103" \
-driver "dhd-cdc-sdmmc-android-panda-cfg80211-oob-debug-3.2.0-panda" \
-dhd_tag  NIGHTLY \
-dhd_brand android-external-dongle-sdio\
-brand linux-internal-dongle \
-tag PHOENIX2_BRANCH_6_10 \
-type 43241b4-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr-proptxstatus-vsdb-srvsdb-opt1-autoabn\
-nvram "bcm943241ipaagb_p100_panda.txt" \
-nvram_add "macaddr=00:90:4c:c5:17:33"\
-postinstall {dhd -i wlan0 msglevel 0x20001; dhd -i wlan0 sd_divisor 1} \
-customer android

UTF::Linux 10.19.90.175
UTF::Panda 43241_p2r1_relay5 \
-sta {43241panda_5 wlan0} \
-relay 10.19.90.175\
-power {Power 5}\
-console "10.19.90.25:40204" \
-hostconsole "10.19.90.25:40104" \
-driver "dhd-cdc-sdmmc-android-panda-cfg80211-oob-debug-3.2.0-panda" \
-dhd_tag  NIGHTLY \
-dhd_brand android-external-dongle-sdio\
-brand linux-internal-dongle \
-tag PHOENIX2_BRANCH_6_10 \
-type 43241b4-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr-proptxstatus-vsdb-srvsdb-opt1-autoabn\
-nvram "bcm943241ipaagb_p100_panda.txt" \
-nvram_add "macaddr=00:90:4c:c5:17:34"\
-postinstall {dhd -i wlan0 msglevel 0x20001; dhd -i wlan0 sd_divisor 1} \
-customer android

UTF::Linux 10.19.90.176
UTF::Panda 43241_p2r1_relay6 \
-sta {43241panda_6 wlan0} \
-relay 10.19.90.176\
-power {Power 6}\
-console "10.19.90.25:40205" \
-hostconsole "10.19.90.25:40105"\
-driver "dhd-cdc-sdmmc-android-panda-cfg80211-oob-debug-3.2.0-panda" \
-dhd_tag  NIGHTLY \
-dhd_brand android-external-dongle-sdio\
-brand linux-internal-dongle \
-tag PHOENIX2_BRANCH_6_10 \
-type 43241b4-roml/sdio-ag-idsup-p2p-err-assert-dmatxrc-sr-proptxstatus-vsdb-srvsdb-opt1-autoabn\
-nvram "bcm943241ipaagb_p100_panda.txt" \
-nvram_add "macaddr=00:90:4c:c5:17:35"\
-postinstall {dhd -i wlan0 msglevel 0x20001; dhd -i wlan0 sd_divisor 1} \
-customer android
