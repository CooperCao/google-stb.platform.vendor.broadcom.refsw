# -*-tcl-*-
#
# Generated configuration file for mbeyer.20111229.161937.1.
#

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
UTF::Power::WebRelay Power -lan_ip 10.19.90.74 -relay Controller
UTF::Power::WebRelay Power_sta -lan_ip 10.19.90.0 -relay Controller

# Controller
UTF::Linux Controller\
	-ssh ssh \
    -lan_ip 10.19.90.24\
    -sta {lan eth1}
lan configure -ipaddr 10.19.90.24


# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.90.137\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay lan\
    -lanpeer lan\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.90.24:40007"\
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

UTF::Cygwin Win8_32_43142_p1r1_1\
	-installer inf\
	-ssh ssh\
    -tag AARDVARK_BRANCH_6_30\
	-brand win8_internal_wl\
    -lan_ip 10.19.90.107\
    -sta {Win8_32_43142_p1r1_1_sta}\
    -user smoketest\
	-osver 8\
    -power {Power 1}

UTF::Cygwin Win8_32_43142_p1r1_2\
	-installer inf\
	-ssh ssh\
    -tag AARDVARK_BRANCH_6_30\
	-brand win8_internal_wl\
    -lan_ip 10.19.90.108\
    -sta {Win8_32_43142_p1r1_2_sta}\
    -user smoketest\
	-osver 8\
    -power {Power 2}

UTF::Cygwin Win8_32_43142_p1r1_3\
	-installer inf\
	-ssh ssh\
    -tag AARDVARK_BRANCH_6_30\
	-brand win8_internal_wl\
    -lan_ip 10.19.90.109\
    -sta {Win8_32_43142_p1r1_3_sta}\
    -user smoketest\
	-osver 8\
    -power {Power 3}

UTF::Cygwin Win8_32_43142_p1r1_4\
	-installer inf\
	-ssh ssh\
    -tag AARDVARK_BRANCH_6_30\
	-brand win8_internal_wl\
    -lan_ip 10.19.90.110\
    -sta {Win8_32_43142_p1r1_4_sta}\
    -user smoketest\
	-osver 8\
    -power {Power 4}

UTF::Cygwin Win8_32_43142_p1r1_5\
	-installer inf\
	-ssh ssh\
    -tag AARDVARK_BRANCH_6_30\
	-brand win8_internal_wl\
    -lan_ip 10.19.90.111\
    -sta {Win8_32_43142_p1r1_5_sta}\
    -user smoketest\
	-osver 8\
    -power {Power 5}

UTF::Cygwin Win8_32_43142_p1r1_6\
	-installer inf\
	-ssh ssh\
    -tag AARDVARK_BRANCH_6_30\
	-brand win8_internal_wl\
    -lan_ip 10.19.90.112\
    -sta {Win8_32_43142_p1r1_6_sta}\
    -user smoketest\
	-osver 8\
    -power {Power 6}
