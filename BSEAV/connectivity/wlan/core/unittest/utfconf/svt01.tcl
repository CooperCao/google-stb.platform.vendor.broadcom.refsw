#
# UTF configuration for lab 5008 svt01-QTP
#Sai-----------This is the file you need to modify or use

# All devices have been assigned static IP addresses by IT, which are now
# visible in Unix NIS. Those devices can use DNS names in the UTF objects
# below.

# Miscellaneous setup.
set UTF::SummaryDir /projects/hnd_svt_ext/Temp/QTP
UTF::Logfile "~/utf_test.log"

# Define power controllers on cart.
UTF::Power::WebRelay svt01rly1
UTF::Power::WebRelay svt01rly2
UTF::Power::WebRelay svt01rly3
UTF::Power::WebRelay svt01rly4
UTF::Power::WebRelay svt01rly5
UTF::Power::WebRelay svt01rly6
UTF::Power::WebSwitch svt01ips1

# Dell E4200 Laptop Vista with 4313+BT NIC
UTF::Cygwin svt01tst4 \
    -osver 6 \
    -sta "svt014_4313_BT_NIC" \
    -power_button "svt01rly4 1"\
    -user user\
    -tag BASS_BRANCH_5_60
#    -image /projects/hnd_software_ext2/work/sshih/tot2/src/wl/sys/wdm/buildwin7/objchk_win7_x86/i386/bcmwl6.sys

# Dell E4200 Laptop XP with 4313+BT NIC
UTF::Cygwin svt01tst5 \
    -osver 7 \
    -sta "svt015_4313_BT_NIC" \
    -power_button "svt01rly5 1"\
    -user user\
    -tag BASS_BRANCH_5_60

# Dell E4200 Laptop XP with 4325SDIO dongle
#UTF::WinDHD mc13tst1 \
#    -sta "mc131_4325_SDIO" \
#    -power_button "mc13rly3 1"\
#    -brand win_mfgtest_dongle_sdio \
#    -brand win_internal_dongle_sdio \
#    -type BcmDHD/Bcm_Sdio_DriverOnly \
#    -user user\
#    -console "mc13end1:40001" \
#    -device_reset "mc13rly3 2"\
#    -tag RAPTOR2_BRANCH_4_217 \
#    -dongleimage 4325b0-roml/sdio-g-cdc-reclaim-btamp-idsup-idauth-ndis \
#    -installer inf \
#    -debuginf 1

# For 4325, need to tell btampqtp.test to use dhd.exe for many commands
#set ::mc13tst1_use_dhd 1 

# Dell E4200 Laptop Win7 with 4313+BT NIC
UTF::Cygwin svt01tst2 \
    -osver 5 \
    -sta "svt012_4313_BT_NIC" \
    -power_button "svt01rly2 1"\
    -user user\
    -tag BASS_BRANCH_5_60
#    -image /projects/hnd_software_ext2/work/sshih/5.60.180twig/src/wl/sys/wdm/buildxp/objchk_wxp_x86/i386/bcmwl5.sys

# Dell E4200 Laptop FC9 with 4329SDIO
# UTF::DHD mc13tst2 \
    -sta "mc132_4329_SDIO" \
    -power_button "mc13rly4 1"\
    -tag ROMTERM_BRANCH_4_218 \
    -nvram "4329b1/bcm94329sdagb.txt" \
    -brand linux-external-dongle-sdio\
    -type "4329b1/sdio-g-cdc-full11n-reclaim-roml-wme-idsup"

# Dell D630 Laptop FC7 with 4322NIC as SNIFFER
package require UTF::Sniffer
UTF::Sniffer svt01tst3 \
    -sta "sniffer eth1"\
    -power_button "svt01rly5 1"

# UTF script control PC with 2 Ethernet cards
# eth2 gives access to AP
# eth1 gives access to corporate network, home directory and build servers
UTF::Linux svt01end1 \
    -sta "lan eth2" \
    -power "svt01ips1 1" \
    -power_button "auto"

# Linksys 320N 4717/4322 wireless router. Use external build
# for higher throughput.
# NB: On Router, MUST have "ethN" after sta name!
UTF::Router Router_4717 \
    -lan_ip 192.168.1.1 \
    -sta "4717_cbtnr_P304 eth1" \
    -power "svt01rly6 1" \
    -power_button "auto"\
    -relay "svt01end1" \
    -console "svt01end1:40000" \
    -lanpeer "lan" \
    -brand "linux-external-router" \
    -tag COMANCHE2_REL_5_20_?? \
    -date * \
    -nvram {
       fw_disable=1
       wl0_ssid=svt01_0
       wl1_ssid=svt01_1
       wl_msglevel=0x101
    }

# Specify BTAMP QTP tests
#set ::btamp_qtp_tests {{svt01tst4 svt01tst2 svt01tst5 "vista 4313+BT NIC"}}
set ::btamp_qtp_tests {{svt01tst2 svt01tst4 svt01tst5 "XP 4313+BT NIC"}}
#set ::btamp_qtp_tests {{svt01tst5 svt01tst4 svt01tst2 "Win7 4313+BT NIC"}}

#set ::btamp_qtp_tests {{mc13tst5 mc13tst4 mc13tst2 "Win7 4312+BT NIC"}\
    {mc13tst4 mc13tst5 mc13tst2 "Vista 4312+BT NIC"}\
    {mc13tst2 mc13tst4 mc13tst5 "XP 4312+BT NIC"}\
    {mc13tst1 mc13tst2 mc13tst4 "XP 4325SDIO"}}

# The btampqtp.test will dynamically go query the STA hardware for the devcon
# pci_id, but this slows down the script development cycle. The scripts run
# faster if you provide devcon pci_id. If you arent careful, you can disable
# the Broadcom GigE wired NIC on the PC, if any. This will require an in-person
# visit to the PC to use Device Manager to enable the GigE wired NIC.set ::svt01tst1_pci_id "PCI\\VEN_14E4"
set ::svt01tst2_pci_id "PCI\\VEN_14E4"
# mc13tst4 is a D630 with BRCM wired GigE ethernet, dont put pci_id here!
set ::svt01tst5_pci_id "PCI\\VEN_14E4"

# The btampqtp.test will dynamically go query the STA hardware for the mac
# address, but this slows down the script development cycle. The scripts run
# faster if you provide STA mac addresses. But testcases will fail if you 
# change the STA hardware and neglect to keep these variables up to date.
set ::svt01tst2_macaddr "00904C5B00F9"
#set ::svt01tst1_macaddr "00904C5B00EA"
set ::svt01tst4_macaddr "00904C5B00EE"
set ::svt01tst5_macaddr "00904C5B00FA"

# Optionally specify the default devices used by btampqtp.test
set ::btamp_dut svt01tst2
set ::btamp_ref1 svt01tst5
set ::btamp_ref2 svt01tst4
