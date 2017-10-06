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
UTF::Power::WebRelay Power -lan_ip 10.19.90.209 -relay Controller
UTF::Power::WebRelay Power_sta -lan_ip 10.19.90.209 -relay Controller

# Controller
UTF::Linux Controller\
	-lan_ip 10.19.90.25\
    -sta {lan eth1}
lan configure -ipaddr 10.19.90.25


# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.90.208\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay lan\
    -lanpeer lan\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.90.25:4007"\
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
		wl1_channel=11
		wl1_obss_coex=0
		wl_msglevel2=0x8
	}

### STA Sections ###
UTF::MacOS MacOSEL_p2r2_1    \
	-lan_ip 10.19.90.179\
	-sta {MacOSEL_p2r2_1_sta en3} \
    -tag BIS715GALA_BRANCH_7_21\
	-brand macos-internal-wl-gala\
	-type Debug_10_11\
	-user root     \
	-kextload 1\
	-wlinitcmds {wl msglevel +assoc +inform; wl btc_mode 0; wl assert_type 1}

UTF::MacOS MacOSEL_p2r2_2    \
	-lan_ip 10.19.90.180\
	-sta {MacOSEL_p2r2_2_sta en3} \
    -tag BIS715GALA_BRANCH_7_21\
	-brand macos-internal-wl-gala\
	-type Debug_10_11\
	-user root     \
	-kextload 1\
	-wlinitcmds {wl msglevel +assoc +inform; wl btc_mode 0; wl assert_type 1}
		
UTF::MacOS MacOSEL_p2r2_3    \
	-lan_ip 10.19.90.181\
	-sta {MacOSEL_p2r2_3_sta en3} \
    -tag BIS715GALA_BRANCH_7_21\
	-brand macos-internal-wl-gala\
	-type Debug_10_11\
	-user root     \
	-kextload 1\
	-wlinitcmds {wl msglevel +assoc +inform; wl btc_mode 0;wl assert_type 1 }
		
UTF::MacOS MacOSEL_p2r2_4    \
	-lan_ip 10.19.90.182\
	-sta {MacOSEL_p2r2_4_sta en3} \
    -tag BIS715GALA_BRANCH_7_21\
	-brand macos-internal-wl-gala\
	-type Debug_10_11\
	-user root     \
	-kextload 1\
	-wlinitcmds {wl msglevel +assoc +inform; wl btc_mode 0;wl assert_type 1 }
	
UTF::MacOS MacOSEL_p2r2_5    \
	-lan_ip 10.19.90.183\
	-sta {MacOSEL_p2r2_5_sta en3} \
    -tag BIS715GALA_BRANCH_7_21\
	-brand macos-internal-wl-gala\
	-type Debug_10_11\
	-user root     \
	-kextload 1\
	-wlinitcmds {wl msglevel +assoc +inform; wl btc_mode 0;wl assert_type 1 }
	
UTF::MacOS MacOSEL_p2r2_6    \
	-lan_ip 10.19.90.184\
	-sta {MacOSEL_p2r2_6_sta en3} \
    -tag BIS715GALA_BRANCH_7_21\
	-brand macos-internal-wl-gala\
	-type Debug_10_11\
	-user root     \
	-kextload 1\
	-wlinitcmds {wl msglevel +assoc +inform; wl btc_mode 0;wl assert_type 1 }
		
UTF::MacOS MacOSEL_p2r2_7    \
	-lan_ip 10.19.90.185\
	-sta {MacOSEL_p2r2_7_sta en3} \
    -tag BIS715GALA_BRANCH_7_21\
	-brand macos-internal-wl-gala\
	-type Debug_10_11\
	-user root     \
	-kextload 1\
	-wlinitcmds {wl msglevel +assoc +inform; wl btc_mode 0;wl assert_type 1 }
