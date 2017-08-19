#
# UTF configuration for lab 2005 cart mc13
#

# All devices have been assigned static IP addresses by IT, which are now
# visible in Unix NIS. Those devices can use DNS names in the UTF objects
# below.

# Miscellaneous setup.
set UTF::SummaryDir /projects/hnd_sig_ext3/$::env(LOGNAME)/mc13
UTF::Logfile "~/utf_test.log"

# Define power controllers on cart.
UTF::Power::WebRelay mc13rly1
UTF::Power::WebRelay mc13rly3 -invertcycle "2"
UTF::Power::WebRelay mc13rly4
UTF::Power::WebRelay mc13rly5
UTF::Power::WebRelay mc13rly6
UTF::Power::WebRelay mc13rly7
UTF::Power::WebSwitch mc13ips1

# Dell E4200 Laptop Vista with 4312+BT NIC
UTF::Cygwin mc13tst4 \
    -osver 6 \
    -sta "4312vst" \
    -power_button "mc13rly6 1"\
    -user user\
    -tag AARDVARK_BRANCH_6_30

# Dell E4200 Laptop Win7 with 4312+BT NIC
UTF::Cygwin mc13tst5 \
    -osver 7 \
    -sta "4312win7" \
    -power_button "mc13rly7 1"\
    -user user\
    -tag AARDVARK_BRANCH_6_30

4312win7 configure -attngrp G1

# Dell E4200 Laptop with 4330SDIO dongle
UTF::DHD mc13tst1 \
    -sta "4330sdio eth1" \
    -power_button "mc13rly3 1" \
    -power_sta "mc13rly3 2" \
    -brand linux-internal-dongle \
    -type 4330b2-roml/sdio-ag-ccx-btamp-p2p-idsup-idauth-pno.bin\
    -tag FALCON_BRANCH_5_90 \
    -nvram 4330b2-roml/bcm94330fcbga_McLaren.txt \

#    -console "mc13tst1:40000" \
#    -postinstall {dhd -i eth1 serial 1} \


# For 4330, need to tell btampqtp.test to use dhd.exe for many commands
set ::mc13tst1_use_dhd 1

# Dell E4200 Laptop XP with 4312+BT NIC
UTF::Cygwin mc13tst2 \
    -osver 5 \
    -sta "4312xp" \
    -power_button "mc13rly4 1"\
    -user user\
    -tag AARDVARK_BRANCH_6_30

4312xp configure -attngrp G2

# Dell D630 Laptop with 4322NIC as SNIFFER
package require UTF::Sniffer
UTF::Sniffer mc13tst3 \
    -sta "sniffer eth0"\
    -power_button "mc13rly5 1" \
    -tag AARDVARK_BRANCH_6_30

# Sniffer as NIC
# UTF::Linux mc13tst3a \
    -lan_ip mc13tst3 \
    -sta "sniffnic eth0"\
    -power_button "mc13rly5 1" \
    -tag AARDVARK_BRANCH_6_30

# UTF script control PC with 2 Ethernet cards
# eth1 gives access to AP
# eth2 gives access to corporate network, home directory and build servers
UTF::Linux mc13end1 \
    -sta "lan eth1" \
    -power "mc13ips1 1" \
    -power_button "auto"

# Aeroflex attenuator
package require UTF::Aeroflex
UTF::Aeroflex mc13att1 -group {G1 {1 2} G2 {3 4} ALL {1 2 3 4}}

# Attenuator power control is on pb4aws1 port 1
set ::rvr_attn_power "mc13ips1 2"

package require UTF::utils
set ::UTF::SetupTestBed {
    set err ""
    set cmds [list "mc13tst1 wl down" "mc13tst2 wl down" "mc13tst3 wl down"\
        "mc13tst4 wl down" "mc13tst5 wl down" "ALL attn 0" \
        "UTF::set_ap_nvram 4717 wl0_radio=0 wl1_radio=0"]
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

# Linksys 320N 4717/4322 wireless router. Use external build
# for higher throughput.
# NB: On Router, MUST have "ethN" after sta name!
UTF::Router router \
    -lan_ip 192.168.1.1 \
    -sta "4717 eth1" \
    -power "mc13rly1 1" \
    -power_button "auto"\
    -relay "mc13end1" \
    -console "mc13end1:40000" \
    -lanpeer "lan" \
    -brand "linux-internal-router" \
    -tag COMANCHE2_REL_5_22_90 \
    -date * \
    -nvram {
       fw_disable=1
       wl0_ssid=mc13_0
       wl1_ssid=mc13_1
       wl_msglevel=0x101
    }

# Specify BTAMP QTP tests
set ::btamp_qtp_tests {{mc13tst5 mc13tst4 mc13tst2 "Win7 4312+BT NIC"}\
    {mc13tst4 mc13tst5 mc13tst2 "Vista 4312+BT NIC"}\
    {mc13tst2 mc13tst4 mc13tst5 "XP 4312+BT NIC"}\
    {mc13tst1 mc13tst2 mc13tst4 "XP 4330SDIO"}}

# The btampqtp.test will dynamically go query the STA hardware for the devcon
# pci_id, but this slows down the script development cycle. The scripts run
# faster if you provide devcon pci_id. If you arent careful, you can disable
# the Broadcom GigE wired NIC on the PC, if any. This will require an in-person
# visit to the PC to use Device Manager to enable the GigE wired NIC.set ::mc13tst1_pci_id "PCI\\VEN_14E4"
set ::mc13tst2_pci_id "PCI\\VEN_14E4"
# mc13tst4 is a D630 with BRCM wired GigE ethernet, dont put pci_id here!
set ::mc13tst5_pci_id "PCI\\VEN_14E4"

# The btampqtp.test will dynamically go query the STA hardware for the mac
# address, but this slows down the script development cycle. The scripts run
# faster if you provide STA mac addresses. But testcases will fail if you 
# change the STA hardware and neglect to keep these variables up to date.
set ::mc13tst1_macaddr "00904CC51238"
set ::mc13tst2_macaddr "00265E70A1D9"
set ::mc13tst4_macaddr "00265E127914"
set ::mc13tst5_macaddr "00265E12D429"

# Optionally specify the default devices used by btampqtp.test
set ::btamp_dut mc13tst4
set ::btamp_ref1 mc13tst2
set ::btamp_ref2 mc13tst5

set ::UTF::PostTestAnalysis /home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl
set ::aux_lsf_queue sj-hnd

# Turn off most RvR intialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""
