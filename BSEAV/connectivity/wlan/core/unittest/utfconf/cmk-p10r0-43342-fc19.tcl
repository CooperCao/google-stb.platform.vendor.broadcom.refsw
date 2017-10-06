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
UTF::Power::WebRelay Power -lan_ip 10.19.102.131 -relay Controller
UTF::Power::WebRelay PowerUSB1 -lan_ip 10.19.102.131 -relay Controller
UTF::Power::WebRelay PowerUSB2 -lan_ip 10.19.102.131 -relay Controller

# Controller
UTF::Linux Controller\
    -lan_ip 10.19.102.24\
    -sta {lan eth1}
lan configure -ipaddr 10.19.102.24

# AP Section
# BCM Reference Router to compare Linksys E4200 Behavior
UTF::Router Ap_2_5\
    -lan_ip 10.19.102.130\
    -sta {Ap_2 eth1 Ap_5 eth2}\
    -relay Controller\
    -lanpeer Controller\
    -model bcm94718nrl_p150\
    -power {Power 8}\
    -console "10.19.102.24:40007"\
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

# FC19x64 43342 SDIO
UTF::DHD 43342_p10r0_sta1\
    -lan_ip 10.19.102.98\
    -sta {43342_p10r0_1 eth0}\
	-power {Power 1} \
    -power_sta {Power 1} \
    -hostconsole "10.19.102.24:40000"\
    -app_tag trunk\
    -clm_blob 43342_southpaw1E1H_R-220.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag PHO2203RC1_TWIG_6_25_178\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43342a0-roml/sdio-g-p2p-pno-nocis-keepalive-aoe-idsup-awdl-ndoe-pf2-proptxstatus-cca-pwrstats-noclminc-logtrace-srscan-mpf-pmmt-frag_dur-clm_min-sr-dsprot/rtecdc.bin \
	-nvram bcm943342SouthpawUSIK_ES5.1.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43342_p10r0_sta2\
    -lan_ip 10.19.102.99\
    -sta {43342_p10r0_2 eth0}\
	-power {Power 2} \
    -power_sta {Power 2} \
    -hostconsole "10.19.102.24:40001"\
    -app_tag trunk\
    -clm_blob 43342_southpaw1E1H_R-220.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag PHO2203RC1_TWIG_6_25_178\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43342a0-roml/sdio-g-p2p-pno-nocis-keepalive-aoe-idsup-awdl-ndoe-pf2-proptxstatus-cca-pwrstats-noclminc-logtrace-srscan-mpf-pmmt-frag_dur-clm_min-sr-dsprot/rtecdc.bin \
	-nvram bcm943342SouthpawUSIK_ES5.1.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43342_p10r0_sta3\
    -lan_ip 10.19.102.100\
    -sta {43342_p10r0_3 eth0}\
	-power {Power 3} \
    -power_sta {Power 3} \
    -hostconsole "10.19.102.24:40002"\
    -app_tag trunk\
    -clm_blob 43342_southpaw1E1H_R-220.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag PHO2203RC1_TWIG_6_25_178\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43342a0-roml/sdio-g-p2p-pno-nocis-keepalive-aoe-idsup-awdl-ndoe-pf2-proptxstatus-cca-pwrstats-noclminc-logtrace-srscan-mpf-pmmt-frag_dur-clm_min-sr-dsprot/rtecdc.bin \
	-nvram bcm943342SouthpawUSIK_ES5.1.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43342_p10r0_sta4\
    -lan_ip 10.19.102.101\
    -sta {43342_p10r0_4 eth0}\
	-power {Power 4} \
    -power_sta {Power 4} \
    -hostconsole "10.19.102.24:40003"\
    -app_tag trunk\
    -clm_blob 43342_southpaw1E1H_R-220.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag PHO2203RC1_TWIG_6_25_178\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43342a0-roml/sdio-g-p2p-pno-nocis-keepalive-aoe-idsup-awdl-ndoe-pf2-proptxstatus-cca-pwrstats-noclminc-logtrace-srscan-mpf-pmmt-frag_dur-clm_min-sr-dsprot/rtecdc.bin \
	-nvram bcm943342SouthpawUSIK_ES5.1.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43342_p10r0_sta5\
    -lan_ip 10.19.102.102\
    -sta {43342_p10r0_5 eth0}\
	-power {Power 5} \
    -power_sta {Power 5} \
    -hostconsole "10.19.102.24:40004"\
    -app_tag trunk\
    -clm_blob 43342_southpaw1E1H_R-220.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag PHO2203RC1_TWIG_6_25_178\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43342a0-roml/sdio-g-p2p-pno-nocis-keepalive-aoe-idsup-awdl-ndoe-pf2-proptxstatus-cca-pwrstats-noclminc-logtrace-srscan-mpf-pmmt-frag_dur-clm_min-sr-dsprot/rtecdc.bin \
	-nvram bcm943342SouthpawUSIK_ES5.1.txt \
	-wlinitcmds {wl msglevel +assoc}

UTF::DHD 43342_p10r0_sta6\
    -lan_ip 10.19.102.103\
    -sta {43342_p10r0_6 eth0}\
	-power {Power 6} \
    -power_sta {Power 6} \
    -hostconsole "10.19.102.24:40005"\
    -app_tag trunk\
    -clm_blob 43342_southpaw1E1H_R-220.clm_blob \
    -dhd_tag DHD_BRANCH_1_359\
    -tag PHO2203RC1_TWIG_6_25_178\
	-dhd_brand linux-external-dongle-sdio \
	-brand linux-external-dongle-sdio \
    -driver dhd-cdc-sdstd-debug\
	-type 43342a0-roml/sdio-g-p2p-pno-nocis-keepalive-aoe-idsup-awdl-ndoe-pf2-proptxstatus-cca-pwrstats-noclminc-logtrace-srscan-mpf-pmmt-frag_dur-clm_min-sr-dsprot/rtecdc.bin \
	-nvram bcm943342SouthpawUSIK_ES5.1.txt \
	-wlinitcmds {wl msglevel +assoc}
