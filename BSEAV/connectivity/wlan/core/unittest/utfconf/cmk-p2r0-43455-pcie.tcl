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
UTF::Power::WebRelay Power -lan_ip 10.19.90.195 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.90.25\
    -sta {lan eth1}
lan configure -ipaddr 10.19.90.25


# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.90.194\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay lan\
    -lanpeer lan\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.90.25:40007"\
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
### 43455c0 PCIe-Dongle STA ###

UTF::DHD Fc19_43455_p2r0s3_sta \
-lan_ip 10.19.90.164\
-user root\
-hostconsole "10.19.90.25:40002" \
-sta {fc19_43455_pcie_sta3 eth0} \
-power {Power 3}\
-dhd_brand linux-internal-dongle-pcie \
-driver dhd-msgbuf-pciefd-debug \
-brand linux-external-dongle-pcie \
-dhd_tag SMOKE_REL_EXPT_43455_fc19x64_pciefd_TRUNK_DHD \
-tag  SMOKE_REL_EXPT_43455_fc19x64_pciefd_BISON06T_BRANCH_7_45\
-app_tag trunk \
-type 43455c0-roml/43455_pcie-43455_ftrs-pno-aoe-pktfilter-sr-pktctx-lpc-pwropt-wapi-mfp-clm_4335_ss-txpwr-rcc-fmc-wepso-noccxaka-sarctrl-proxd-gscan-linkstat-pwrstats-idsup-ndoe-pwrofs-hs20sta-nan-xorcsum-amsdutx-swdiv/rtecdc.bin \
-nvram bcm943457wlpagb.txt
