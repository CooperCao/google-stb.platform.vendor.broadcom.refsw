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
UTF::Power::WebRelay Power -lan_ip 10.19.91.74 -relay Controller
UTF::Power::WebRelay Power_sta -lan_ip 10.19.91.74 -relay Controller

# Controller
UTF::Linux Controller\
	-lan_ip 10.19.91.23\
    -sta {lan eth1}
lan configure -ipaddr 10.19.91.23


# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.91.73\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay lan\
    -lanpeer lan\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.91.23:42007"\
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
# FC19x64 43435 SDIO 3.0
UTF::DHD 43435_p6r1_sta1\
    -lan_ip 10.19.91.43\
    -sta {43435_p6r1_1 eth0}\
	-power {Power 1} \
    -power_sta {Power 1} \
    -hostconsole "10.19.91.23:42000"\
    -app_tag DHD_BRANCH_1_359\
    -clm_blob 43435b0.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag DIN2930R18_BRANCH_9_44\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43435b0-roml/config_sdio_release/rtecdc.bin \
	-nvram bcm943430fsdnge_elna_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43435_p6r1_sta2\
    -lan_ip 10.19.91.44\
    -sta {43435_p6r1_2 eth0}\
	-power {Power 2} \
    -power_sta {Power 2} \
    -hostconsole "10.19.91.23:42001"\
    -app_tag DHD_BRANCH_1_359\
    -clm_blob 43435b0.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag DIN2930R18_BRANCH_9_44\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43435b0-roml/config_sdio_release/rtecdc.bin \
	-nvram bcm943430fsdnge_elna_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43435_p6r1_sta3\
    -lan_ip 10.19.91.45\
    -sta {43435_p6r1_3 eth0}\
	-power {Power 3} \
    -power_sta {Power 3} \
    -hostconsole "10.19.91.23:42002"\
    -app_tag DHD_BRANCH_1_359\
    -clm_blob 43435b0.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag DIN2930R18_BRANCH_9_44\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43435b0-roml/config_sdio_release/rtecdc.bin \
	-nvram bcm943430fsdnge_elna_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43435_p6r1_sta4\
    -lan_ip 10.19.91.46\
    -sta {43435_p6r1_4 eth0}\
	-power {Power 4} \
    -power_sta {Power 4} \
    -hostconsole "10.19.91.23:42003"\
    -app_tag DHD_BRANCH_1_359\
    -clm_blob 43435b0.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag DIN2930R18_BRANCH_9_44\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43435b0-roml/config_sdio_release/rtecdc.bin \
	-nvram bcm943430fsdnge_elna_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43435_p6r1_sta5\
    -lan_ip 10.19.91.47\
    -sta {43435_p6r1_5 eth0}\
	-power {Power 5} \
    -power_sta {Power 5} \
    -hostconsole "10.19.91.23:42004"\
    -app_tag DHD_BRANCH_1_359\
    -clm_blob 43435b0.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag DIN2930R18_BRANCH_9_44\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43435b0-roml/config_sdio_release/rtecdc.bin \
	-nvram bcm943430fsdnge_elna_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43435_p6r1_sta6\
    -lan_ip 10.19.91.48\
    -sta {43435_p6r1_6 eth0}\
	-power {Power 6} \
    -power_sta {Power 6} \
    -hostconsole "10.19.91.23:42005"\
    -app_tag DHD_BRANCH_1_359\
    -clm_blob 43435b0.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag DIN2930R18_BRANCH_9_44\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43435b0-roml/config_sdio_release/rtecdc.bin \
	-nvram bcm943430fsdnge_elna_Bx.txt \
	-wlinitcmds {wl msglevel +assoc}
