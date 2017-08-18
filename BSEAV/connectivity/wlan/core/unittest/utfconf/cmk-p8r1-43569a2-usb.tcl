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
UTF::Power::WebRelay Power -lan_ip 10.19.91.202 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.91.25\
    -sta {lan eth1}
lan configure -ipaddr 10.19.91.25


# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.91.201\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay lan\
    -lanpeer lan\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.91.25:41007"\
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
### 43569a0 USB-Dongle STA ###

UTF::DHD Fc19_43569_p8r1s1_sta \
-lan_ip 10.19.91.171\
-user root\
-hostconsole "10.19.91.25:41000" \
-console "10.19.91.25:43000"\
-sta {fc19_43569a2_usb_sta1 eth0} \
-power {Power 1}\
-power_sta {Power 1} \
-dhd_brand linux-internal-media \
-driver dhd-cdc-usb-gpl \
-brand linux-internal-media \
-tag  BISON_BRANCH_7_10\
-type 43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx \
-nvram bcm943569usbir_p156.txt

UTF::DHD Fc19_43569_p8r1s2_sta \
-lan_ip 10.19.91.172\
-user root\
-hostconsole "10.19.91.25:41001" \
-console "10.19.91.25:43001"\
-sta {fc19_43569a2_usb_sta2 eth0} \
-power {Power 2}\
-power_sta {Power 2} \
-dhd_brand linux-internal-media \
-driver dhd-cdc-usb-gpl \
-brand linux-internal-media \
-tag  BISON_BRANCH_7_10\
-type 43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx \
-nvram bcm943569usbir_p156.txt

UTF::DHD Fc19_43569_p8r1s3_sta \
-lan_ip 10.19.91.173\
-user root\
-hostconsole "10.19.91.25:41002" \
-console "10.19.91.25:43002"\
-sta {fc19_43569a2_usb_sta3 eth0} \
-power {Power 3}\
-power_sta {Power 3} \
-dhd_brand linux-internal-media \
-driver dhd-cdc-usb-gpl \
-brand linux-internal-media \
-tag  BISON_BRANCH_7_10\
-type 43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx \
-nvram bcm943569usbir_p156.txt

UTF::DHD Fc19_43569_p8r1s4_sta \
-lan_ip 10.19.91.174\
-user root\
-hostconsole "10.19.91.25:41003" \
-console "10.19.91.25:43003"\
-sta {fc19_43569a2_usb_sta4 eth0} \
-power {Power 4}\
-power_sta {Power 4} \
-dhd_brand linux-internal-media \
-driver dhd-cdc-usb-gpl \
-brand linux-internal-media \
-tag  BISON_BRANCH_7_10\
-type 43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx \
-nvram bcm943569usbir_p156.txt

UTF::DHD Fc19_43569_p8r1s5_sta \
-lan_ip 10.19.91.175\
-user root\
-hostconsole "10.19.91.25:41004" \
-console "10.19.91.25:43004"\
-sta {fc19_43569a2_usb_sta5 eth0} \
-power {Power 5}\
-power_sta {Power 5} \
-dhd_brand linux-internal-media \
-driver dhd-cdc-usb-gpl \
-brand linux-internal-media \
-tag  BISON_BRANCH_7_10\
-type 43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx \
-nvram bcm943569usbir_p156.txt

UTF::DHD Fc19_43569_p8r1s6_sta \
-lan_ip 10.19.91.176\
-user root\
-hostconsole "10.19.91.25:41005" \
-console "10.19.91.25:43005"\
-sta {fc19_43569a2_usb_sta6 eth0} \
-power {Power 6}\
-power_sta {Power 6} \
-dhd_brand linux-internal-media \
-driver dhd-cdc-usb-gpl \
-brand linux-internal-media \
-tag  BISON_BRANCH_7_10\
-type 43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx \
-nvram bcm943569usbir_p156.txt
