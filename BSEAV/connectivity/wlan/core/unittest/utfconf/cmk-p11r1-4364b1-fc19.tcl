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
UTF::Power::WebRelay Power -lan_ip 10.19.102.202 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.102.25\
    -sta {lan eth1}
lan configure -ipaddr 10.19.102.25

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.102.201\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay Controller\
    -lanpeer Controller\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.102.25:46007"\
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

# 4364b1 PCIe Full Dongle
UTF::DHD 4364_p11r1_st1 \
    -lan_ip 10.19.102.171 \
    -sta "4364_p11r1_1 eth0"\
	-power {Power 1} \
    -power_sta {Power 1} \
    -hostconsole "10.19.102.25:46000"\
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-dhd_tag DHD_BRANCH_1_359 \
    -app_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4364b1-roml/config_pcie_release/rtecdc.bin \
    -nvram "bcm94364fcpagb_2.txt"\
    -clm_blob 4364b0.clm_blob\
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 4364_p11r1_st2 \
    -lan_ip 10.19.102.172 \
    -sta "4364_p11r1_2 eth0"\
	-power {Power 2} \
    -power_sta {Power 2} \
    -hostconsole "10.19.102.25:46001"\
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-dhd_tag DHD_BRANCH_1_359 \
    -app_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4364b1-roml/config_pcie_release/rtecdc.bin \
    -nvram "bcm94364fcpagb_2.txt"\
    -clm_blob 4364b0.clm_blob\

UTF::DHD 4364_p11r1_st3 \
    -lan_ip 10.19.102.173 \
    -sta "4364_p11r1_3 eth0"\
	-power {Power 3} \
    -power_sta {Power 3} \
    -hostconsole "10.19.102.25:46002"\
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-dhd_tag DHD_BRANCH_1_359 \
    -app_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4364b1-roml/config_pcie_release/rtecdc.bin \
    -nvram "bcm94364fcpagb_2.txt"\
    -clm_blob 4364b0.clm_blob\
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 4364_p11r1_st4 \
    -lan_ip 10.19.102.174 \
    -sta "4364_p11r1_4 eth0"\
	-power {Power 4} \
    -power_sta {Power 4} \
    -hostconsole "10.19.102.25:46003"\
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-dhd_tag DHD_BRANCH_1_359 \
    -app_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4364b1-roml/config_pcie_release/rtecdc.bin \
    -nvram "bcm94364fcpagb_2.txt"\
    -clm_blob 4364b0.clm_blob\
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 4364_p11r1_st5 \
    -lan_ip 10.19.102.175 \
    -sta "4364_p11r1_5 eth0"\
	-power {Power 5} \
    -power_sta {Power 5} \
    -hostconsole "10.19.102.25:46004"\
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-dhd_tag DHD_BRANCH_1_359 \
    -app_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4364b1-roml/config_pcie_release/rtecdc.bin \
    -nvram "bcm94364fcpagb_2.txt"\
    -clm_blob 4364b0.clm_blob\

UTF::DHD 4364_p11r1_st6 \
    -lan_ip 10.19.102.176 \
    -sta "4364_p11r1_6 eth0"\
	-power {Power 6} \
    -power_sta {Power 6} \
    -hostconsole "10.19.102.25:46005"\
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-dhd_tag DHD_BRANCH_1_359 \
    -app_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
	-type 4364b1-roml/config_pcie_release/rtecdc.bin \
    -nvram "bcm94364fcpagb_2.txt"\
    -clm_blob 4364b0.clm_blob\
	-wlinitcmds {wl msglevel +assoc}
