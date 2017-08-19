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
UTF::Power::WebRelay Power -lan_ip 10.19.102.209 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.102.25\
    -sta {lan eth1}
lan configure -ipaddr 10.19.102.25

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.102.208\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay Controller\
    -lanpeer Controller\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.102.25:47007"\
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

# 4357a0 PCIe Full Dongle
UTF::DHD 4357_p11r2_st1 \
    -lan_ip 10.19.102.179 \
    -sta {p11r2s1_4357MAIN eth0 p11r2s1_4357AUX wl1.2} \
	-power {Power 1} \
    -power_sta {Power 1} \
    -hostconsole "10.19.102.25:47000"\
	-tag IGUANA_BRANCH_13_10 \
	-dhd_tag DHD_BRANCH_1_579  \
    -app_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4347a0.clm_blob \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 4357_p11r2_st2 \
    -lan_ip 10.19.102.180 \
    -sta {p11r2s2_4357MAIN eth0 p11r2s2_4357AUX wl1.2} \
	-power {Power 2} \
    -power_sta {Power 2} \
    -hostconsole "10.19.102.25:47001"\
	-tag IGUANA_BRANCH_13_10 \
	-dhd_tag DHD_BRANCH_1_579  \
    -app_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4347a0.clm_blob \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 4357_p11r2_st3 \
    -lan_ip 10.19.102.181 \
    -sta {p11r2s3_4357MAIN eth0 p11r2s3_4357AUX wl1.2} \
	-power {Power 3} \
    -power_sta {Power 3} \
    -hostconsole "10.19.102.25:47002"\
	-tag IGUANA_BRANCH_13_10 \
	-dhd_tag DHD_BRANCH_1_579  \
    -app_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4347a0.clm_blob \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 4357_p11r2_st4 \
    -lan_ip 10.19.102.182 \
    -sta {p11r2s4_4357MAIN eth0 p11r2s4_4357AUX wl1.2} \
	-power {Power 4} \
    -power_sta {Power 4} \
    -hostconsole "10.19.102.25:47003"\
	-tag IGUANA_BRANCH_13_10 \
	-dhd_tag DHD_BRANCH_1_579  \
    -app_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4347a0.clm_blob \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 4357_p11r2_st5 \
    -lan_ip 10.19.102.183 \
    -sta {p11r2s5_4357MAIN eth0 p11r2s5_4357AUX wl1.2} \
	-power {Power 5} \
    -power_sta {Power 5} \
    -hostconsole "10.19.102.25:47004"\
	-tag IGUANA_BRANCH_13_10 \
	-dhd_tag DHD_BRANCH_1_579  \
    -app_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4347a0.clm_blob \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 4357_p11r2_st6 \
    -lan_ip 10.19.102.184 \
    -sta {p11r2s6_4357MAIN eth0 p11r2s6_4357AUX wl1.2} \
	-power {Power 6} \
    -power_sta {Power 6} \
    -hostconsole "10.19.102.25:47005"\
	-tag IGUANA_BRANCH_13_10 \
	-dhd_tag DHD_BRANCH_1_579  \
    -app_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4347a0.clm_blob \
	-wlinitcmds {wl msglevel +assoc}
