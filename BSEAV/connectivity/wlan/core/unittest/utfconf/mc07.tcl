#
# UTF configuration for lab 2005 cart mc07
#

# All devices have been assigned static IP addresses by IT, which are now
# visible in Unix NIS. Those devices can use DNS names in the UTF objects
# below.

# Miscellaneous setup.
set UTF::SummaryDir /projects/hnd_sig_ext3/$::env(LOGNAME)/mc07
UTF::Logfile "~/utf_test.log"

# Define power controllers on cart.
UTF::Power::WebRelay mc07rly1 -invertcycle "1 2 3 4"
UTF::Power::WebRelay mc07rly3
UTF::Power::WebRelay mc07rly4
UTF::Power::WebRelay mc07rly5
UTF::Power::WebRelay mc07rly6
UTF::Power::WebRelay mc07rly7
UTF::Power::WebSwitch mc07ips1

# Dell E6400 Laptop Vista with 43225NIC
UTF::Cygwin mc07tst4 \
    -osver 6 \
    -sta "43225vst" \
    -power_button "mc07rly6 1"\
    -user user \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate}} \
    -tag AARDVARK_BRANCH_6_30

# Dell E4200 Laptop Win7 with 43225NIC
UTF::Cygwin mc07tst5 \
    -osver 7 \
    -sta "43225win7" \
    -power_button "mc07rly7 1"\
    -user user\
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate}} \
    -tag AARDVARK_BRANCH_6_30

43225win7 configure -attngrp G1

# Dell Mini Laptop XP with 4322NIC
UTF::Cygwin mc07tst1 \
    -sta "4322xp" \
    -power_button "mc07rly3 1"\
    -user user \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate}} \
    -tag AARDVARK_BRANCH_6_30

4322xp configure -attngrp G2

# Dell E4200 Laptop XP with 4329SDIO
UTF::WinDHD mc07tst2 \
    -sta "4329sdio" \
    -console "mc07tst2:40000" \
    -power_button "mc07rly4 1"\
    -device_reset "mc07rly4 2"\
    -user user \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate}} \
    -tag ROMTERM3_BRANCH_4_220\
    -brand win_internal_dongle_sdio \
    -type BcmDHD/Bcm_Sdio_DriverOnly \
    -dongleimage 4329b1/sdio-g-cdc-reclaim-roml-dhdoid-btamp-idsup-idauth-minioctl.bin \
    -dongleimage 4329b1/sdio-g-cdc-reclaim-roml-ndis-btamp-idsup-idauth-minioctl.bin\
    -nvram Bcm_Firmware/bcm94329sdagb.txt \
    -nvram src/shared/nvram/bcm94329sdagb.txt \
    -hack 0 \
    -alwayspowercycledongle 1

4329sdio configure -attngrp G3


# For 4329, need to tell btampqtp.test to use dhd.exe for many commands
set ::mc07tst2_use_dhd 1

# Dell D630 Laptop with 4322NIC as SNIFFER
package require UTF::Sniffer
UTF::Sniffer mc07tst3 \
    -sta "sniffer eth0" \
    -power_button "mc07rly5 1"\
    -tag AARDVARK_BRANCH_6_30

# Sniffer as NIC
# UTF::Linux mc07tst3a \
    -lan_ip mc07tst3 \
    -sta "sniffnic eth0" \
    -power_button "mc07rly5 1" \
    -tag AARDVARK_BRANCH_6_20

# UTF script control PC with 3 Ethernet cards
# eth1 gives access to corporate network, home directory and build servers
# eth2 is currently not connected or used, eventually can be removed.
# eth3 gives access to AP
UTF::Linux mc07end1 \
    -sta "lan eth3" \
    -power "mc07ips1 2" \
    -power_button "auto"

# Aeroflex attenuator
package require UTF::Aeroflex
UTF::Aeroflex mc07att1 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

# Attenuator power control
set ::rvr_attn_power "mc07ips1 1"

set ::UTF::SetupTestBed {
    set err ""
    set cmds [list "mc07tst1 wl down" "mc07tst2 wl down" "mc07tst3 wl down"\
        "mc07tst4 wl down" "mc07tst5 wl down" "ALL attn 0" \
        "UTF::set_ap_nvram 4718 wl0_radio=0 wl1_radio=0"]
    foreach c $cmds {
        # puts "c=$c"
        set resp [catch "$c" msg]
        if {$resp != 0} {
            lappend err "$c: $msg"
        }
        # Avoid duplicate log messages by doing deinit
        set o [lindex $c 0]
        catch "$o deinit"
    }
    if {$err != ""} {
        error "$err"
    }
}

# BRCM 4718/43224 wireless router. Use external build
# for higher throughput.
# NB: On Router, MUST have "ethN" after sta name!
UTF::Router router \
    -lan_ip 192.168.1.1 \
    -sta "4718 eth2" \
    -power "mc07rly1 1" \
    -power_button "auto"\
    -relay "mc07end1" \
    -console "mc07end1:40000" \
    -lanpeer "lan" \
    -brand "linux-external-router" \
    -tag COMANCHE2_REL_5_22_90 \
    -nvram {
       fw_disable=1
       wl0_radio=0
       wl0_ssid=mc07_0
       wl1_ssid=mc07_1
       wl_msglevel=0x101
    }

package require UTF::BSDAP
UTF::BSDAP bsdap \
    -sta {4718bsd bwl1} \
    -relay "mc07end1" \
    -lanpeer lan \
    -console "mc07end1:40000" \
    -power "mc07rly1 1" \
    -brand netbsd-internal-router \
    -trx netbsd_74k \
    -tag PBR_TWIG_5_12_142 \
    -noafterburner 1 \
    -perfonly 0 \
    -nvram {
	wl_msglevel=0x101
	watchdog=3000
	wl0_ssid=mc07_bsd0
	wl0_radio=0
	wl1_ssid=mc07_bsd1
	fw_disable=1
    }

# Specify BTAMP QTP tests
set ::btamp_qtp_tests {{mc07tst4 mc07tst1 mc07tst5 "Vista 43225NIC"}\
    {mc07tst1 mc07tst4 mc07tst5 "XP 4322NIC"}\
    {mc07tst5 mc07tst4 mc07tst1 "Win7 43225NIC"}\
    {mc07tst2 mc07tst1 mc07tst4 "XP 4329SDIO"}}

# The btampqtp.test will dynamically go query the STA hardware for the devcon
# pci_id, but this slows down the script development cycle. The scripts run
# faster if you provide devcon pci_id. If you arent careful, you can disable
# the Broadcom GigE wired NIC on the PC, if any. This will require an in-person
# visit to the PC to use Device Manager to enable the GigE wired NIC.
set ::mc07tst1_pci_id "PCI\\VEN_14E4"
set ::mc07tst2_pci_id "PCI\\VEN_14E4"
set ::mc07tst4_pci_id "PCI\\VEN_14E4"
set ::mc07tst5_pci_id "PCI\\VEN_14E4"

# The btampqtp.test will dynamically go query the STA hardware for the mac
# address, but this slows down the script development cycle. The scripts run
# faster if you provide STA mac addresses. But testcases will fail if you 
# change the STA hardware and neglect to keep these variables up to date.
set ::mc07tst1_macaddr "00234d1e4344"
# 4329 macaddr is in the nvram file and is a moving target...
set ::mc07tst4_macaddr "001018962614"
set ::mc07tst5_macaddr "00101896260A"

# Optionally specify the default devices used by btampqtp.test
set ::btamp_dut mc07tst4
set ::btamp_ref1 mc07tst1
set ::btamp_ref2 mc07tst5

set ::UTF::PostTestAnalysis /home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl
set ::aux_lsf_queue sj-hnd

# Turn off most RvR intialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""

