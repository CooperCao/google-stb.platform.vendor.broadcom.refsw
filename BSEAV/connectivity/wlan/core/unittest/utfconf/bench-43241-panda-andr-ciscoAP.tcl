# -*-tcl-*-
#
# Generated configuration file for mbeyer.20111229.161937.1.
#

# Load Packages
#package require UTF::Aeroflex
package require UTF::Panda

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
UTF::Power::WebRelay Power -lan_ip 10.19.90.74 -relay Controller
UTF::Power::WebRelay Power_sta -lan_ip 10.19.90.0 -relay Controller

# Controller
UTF::Linux Controller\
	-ssh ssh \
    -lan_ip 10.19.90.23\
    -sta {lan eth1}
lan configure -ipaddr 10.19.90.23


# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.90.72\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay lan\
    -lanpeer lan\
    -model cisco_e4200\
    -power {Power 8}\
    -brand linux26-internal-router\
    -tag "AKASHI_REL_5_110_35"\
    -txt_override {wandevs=et0}\
    -nvram {
		lan_ipaddr=192.168.1.1
		lan_gateway=192.168.1.1
		dhcp_start=192.168.1.100
		dhcp_end=192.168.1.149
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
		wl1_channel=44
		wl1_obss_coex=0
		wl_msglevel2=0x8
	}

### STA Sections ###
UTF::Linux 10.19.90.49

UTF::Panda 43241_Panda_android_bnc \
-sta {43241panda wlan0} \
-relay 10.19.90.49\
-power {Power 1}\
-console "10.19.90.23:40102" \
-hostconsole "10.19.90.23:40101" \
-brand linux-external-dongle-sdio \
-tag PHOENIX2_REL_6_10_116 \
-type 43241b0min-roml/sdio-ag-pno-p2p-proptxstatus-dmatxrc-rxov-pktfilter-keepalive-aoe-vsdb-wapi-wl11d-sr-srvsdb-opt2.bin\
-nvram "/projects/hnd/swbuild/build_linux/PHOENIX2_REL_6_10_116/linux-external-dongle-sdio/2012.6.29.0/release/android/firmware/43241b0min-roml/bcm943241ipaagb_p100_hwoob.txt" \
-nvram_add "macaddr=00:90:4c:c5:17:30"\
-postinstall {dhd -i wlan0 msglevel 0x20001; dhd -i wlan0 sd_divisor 1} \
-customer android
