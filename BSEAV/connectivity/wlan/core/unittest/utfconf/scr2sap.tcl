# -*-tcl-*-
#
# Testbed configuration file for SCR2MC1 SoftAP Test 
#

# KVMS use admin as username and wlan root passwords


# load packages
package require UTF
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Multiperf
package require UTF::Test::ConnectAPSTA
package require UTF::Test::APChanspec
package require UTF::Test::APConfigureSecurity
package require UTF::Test::EmbeddedNightly


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
#set ::UTF::SummaryDir "/projects/hnd_sig_ext2/sclab1/mc1"
set ::UTF::SummaryDir "/projects/hnd_sig_ext4/$::env(LOGNAME)/scr2mc1"

# this is needed for multiple STA to multiple AP tests
#set ::sta_list "4312XP 4312Vista 4312WIN7 4313XP 4313Vista 4313WIN7 4311XP 4311Vista 4311WIN7 4322XP 4322Vista 4322WIN8 43224XP 43224Vista 43224WIN7 4321XP 4321Vista 4321WIN7 4306XP 4306Vista 4306WIN7 4309XP 4309Vista 4309WIN7 4318XP 4318Vista 4318WIN7"
set ::ap_list "47171 47172 47173 43211 43212 43181 4712"


# Power controllers
UTF::Power::Synaccess npc102 -lan_ip 10.22.23.102
UTF::Power::Synaccess npc103 -lan_ip 10.22.23.103
UTF::Power::Synaccess npc104 -lan_ip 10.22.23.104
UTF::Power::Synaccess npc105 -lan_ip 10.22.23.105
UTF::Power::Synaccess npc106 -lan_ip 10.22.23.106
UTF::Power::Synaccess npc107 -lan_ip 10.22.23.107
UTF::Power::Synaccess npc108 -lan_ip 10.22.23.108
UTF::Power::Synaccess npc109 -lan_ip 10.22.23.109
UTF::Power::Synaccess npc160 -lan_ip 10.22.23.160
UTF::Power::Synaccess npc161 -lan_ip 10.22.23.161
UTF::Power::Synaccess npc162 -lan_ip 10.22.23.162
UTF::Power::Synaccess npc163 -lan_ip 10.22.23.163
UTF::Power::Synaccess npc164 -lan_ip 10.22.23.164


# UTF Setup Defaults for TestBed during tests
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    #ALL attn 0;

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    47171 restart wl0_radio=0
    47172 restart wl0_radio=0
    47173 restart wl0_radio=0
    43211 restart wl0_radio=0
    43181 restart wl0_radio=0
    43212 restart wl1_radio=0
    
   # This is needed to ensure we start without any inteference.
foreach S {47171 47172 47173 43211 43212 43181 4712 4312XP 4312Vista 4312WIN7 4313XP 4313Vista 4313WIN7 4311XP 4311Vista 4311WIN7 4322Vista 4322WIN8 43224XP 43224Vista 43224WIN7 4321XP 4321Vista 4321WIN7 4306XP 4309XP 4318XP 4306Vista 4309Vista 4318Vista 4306WIN7 4309WIN7 4318WIN7} {
        catch {$S wl down}
        $S deinit    
       }

    
  return
 }

# Define Sniffer FC9 E4200
UTF::Sniffer scr2mc1snf1 -user root \
        -sta {43224SNF1 eth1} \
        -tag BASS_BRANCH_5_60 \
        -power {npc106 8} \
        -power_button {auto}


# UTF Endpoint - Traffic generators (no wireless cards)
UTF::Linux scr2mc1end1 \
    -sta "lan eth1" \
    -power {npc105 1} \
    -power_button {auto} 

# AP Section

# Linksys 320N 4717/4322 wireless router AP1.
UTF::Router AP1 \
    -sta "47171 eth1" \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.2 \
    -console "scr2mc1end1:40001" \
    -power {npc102 5} \
    -power_button {auto} \
    -brand linux-external-router \
    -tag "MILLAU_REL_5_70_48_*" \
    -nvram {
        et0macaddr=00:90:4c:01:00:8b
        macaddr=00:90:4c:01:01:9b
        lan_ipaddr=192.168.1.2
        lan_gateway=192.168.1.2
        fw_disable=1
        router_disable=1
        wl_msglevel=0x101
        wl0_ssid=test47171
        wl0_channel=1
        wl0_radio=0
        antswitch=0
    }

# Linksys 320N 4717/4322 wireless router AP2.
UTF::Router AP2 \
    -sta "47172 eth1" \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.3 \
    -console "scr2mc1end1:40002" \
    -power {npc102 6} \
    -power_button {auto} \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_2_2" \
    -nvram {
        et0macaddr=00:90:4c:02:00:8b
        macaddr=00:90:4c:02:01:9b
        lan_ipaddr=192.168.1.3
        lan_gateway=192.168.1.3
        fw_disable=1
        router_disable=1
        wl_msglevel=0x101
        wl0_ssid=test47172
        wl0_ssid=47121
        wl0_channel=1
        wl0_radio=0
        antswitch=0
    }


# Linksys 320N 4717/4322 wireless router AP3.
UTF::Router AP3 \
    -sta "47173 eth1" \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.4 \
    -console "scr2mc1end1:40003" \
    -power {npc102 7} \
    -power_button {auto} \
    -brand linux-external-router \
    -nvram {
        et0macaddr=00:90:4c:03:00:8b
        macaddr=00:90:4c:03:01:9b
        lan_ipaddr=192.168.1.4
        lan_gateway=192.168.1.4
        fw_disable=1
        router_disable=1
        wl_msglevel=0x101
        wl0_ssid=test47173
        wl0_channel=1
        wl0_radio=0
        antswitch=0
    }

# BCM9705 w/4321MP as AP4.
UTF::Router AP4 \
    -sta "43211 eth1" \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.5 \
    -console "scr2mc1end1:40004" \
    -power {npc102 8} \
    -power_button {auto} \
    -brand linux-external-router \
    -model bcm94705GMP \
    -serial_num 1271637 \
    -nvram {
        lan_ipaddr=192.168.1.5
        wl_msglevel=0x101
        wl0_ssid=43211
        wl0_channel=1
        wl0_radio=0
        fw_disable=1
        router_disable=1
    }

# BCM9704 w/4321MP w/4318
UTF::Router AP5 \
    -sta "43212 eth2 43181 eth3" \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.6 \
    -console "scr2mc1end1:40005" \
    -power {npc160 1} \
    -power_button {auto} \
    -brand linux-external-router \
    -nvram {
        lan_ipaddr=192.168.1.6
        wl_msglevel=0x101
        wl0_ssid=43181
        wl0_channel=1
        wl0_radio=0
        wl1_ssid=43212
        wl1_channel=1
        wl1_radio=0
        fw_disable=1
        router_disable=1
    }


# BCM4712AGR
UTF::Router AP6 \
    -sta "4712 eth1" \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.7 \
    -console "scr2mc1end1:40006" \
    -power {npc160 2} \
    -power_button {auto} \
    -brand linux-external-router \
    -nvram {
        lan_ipaddr=192.168.1.7
        wl_msglevel=0x101
        wl0_ssid=47121
        wl0_radio=0
        fw_disable=1
        router_disable=1
    }


# SoftAP Section 

# STA DB43LD Desktop
UTF::DHD scr2mc1tst28 -sta {4330sap eth1} \
    -type "4330b1-roml/sdio-ag-{,pool-}ccx-btamp-p2p-idsup-idauth-pno{,-aoe-toe-pktfilter-keepalive-wapi}" \
    -nvram "bcm94330fcbga_McLaren.txt" \
    -tag FALCON_TWIG_5_90_125 \
    -preinstall_hook {{%S power_sta cycle; UTF::Sleep 10}} \
    -console "scr2mc1tst29:40000" \
    -hostconsole "scr2mc1end1:40017" \
    -nocal 1 \
    -power {npc162 5} \
    -power_sta {npc162 8} \
    -postinstall {dhd -i eth1 serial 1}
    
4330sap configure -ipaddr 192.168.1.8 -hasdhcpd 1 -ap 1 -ssid test4330sap


# STA DB43LD Desktop
UTF::DHD scr2mc1tst29 -sta {4336sap eth1} \
    -tag FALCON_{TWIG,REL}_5_90_125{,_*} \
    -nvram bcm943362sdg.txt \
    -type "43362a0-roml/sdio{,-pool}-apsta-idsup-idauth{,-proptxstatus}-pno{,-aoe-toe-pktfilter-keepalive-wapi}" \
    -nocal 1 \
    -nomaxmem 1 \
    -postinstall {dhd -i eth1 serial 1} \
    -power {npc162 6} \
    -power_sta {npc162 3} \
    -preinstall_hook {{%S power_sta cycle; UTF::Sleep 10}} 
#   -hostconsole "scr2mc1end1:40001" \
#    -console scr2mc1end1:40002 \

4336sap configure -ipaddr 192.168.1.9 -hasdhcpd 1 -ap 1 -ssid test4336sap

# STA Section

#STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 1
UTF::Cygwin scr2mc1tst1 -user user -sta {4312XP} \
    -osver 5 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc109 1} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -tag KIRIN_BRANCH_5_100 \
    -date 2011.5.20

# STA Laptop DUT Dell E6400
# scr2mc1kvm150 2
UTF::Cygwin scr2mc1tst2 -user user -sta {4312Vista} \
    -osver 6 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc109 2} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
    -tag KIRIN_BRANCH_5_100 \
    -date 2011.5.20


#STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 4
UTF::Cygwin scr2mc1tst4 -user user -sta {4313XP} \
    -osver 5 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc109 4} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -tag KIRIN_REL_5_100_82_*


#STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 5
UTF::Cygwin scr2mc1tst5 -user user -sta {4313Vista} \
    -osver 6 \
    -installer inf \
    -tcpwindow 512k \
    -debuginf true \
    -power {npc109 5} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
    -tag KIRIN_REL_5_100_82_*

# STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 6
UTF::Cygwin scr2mc1tst6 -user user -sta {4313WIN7} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc109 6} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -tag KIRIN_REL_5_100_82_*

# STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 7
# aspm 0x100 is disabling L0s and L1 due to PR#65045 and PR#64659
UTF::Cygwin scr2mc1tst7 -user user -sta {4311XP} \
    -osver 5 \
    -installer inf \
    -tcpwindow 512k \
    -wlinitcmds {wl aspm 0x100} \
    -power {npc109 7} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -tag KIRIN_BRANCH_5_100 \
    -date 2011.5.20


#STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 8
# aspm 0x100 is disabling L0s and L1 due to PR#65045 and PR#64659
UTF::Cygwin scr2mc1tst8 -user user -sta {4311Vista} \
    -osver 6 \
    -installer inf \
    -tcpwindow 512k \
    -wlinitcmds {wl aspm 0x100} \
    -power {npc109 8} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
    -tag KIRIN_BRANCH_5_100 \
    -date 2011.5.20


# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 1
UTF::Cygwin scr2mc1tst9 -user user -sta {4322XP} \
    -osver 5 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc107 1} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -tag KIRIN_BRANCH_5_100 \
    -date 2011.5.20

#STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 2
UTF::Cygwin scr2mc1tst10 -user user -sta {4322Vista} \
    -osver 6 \
    -installer inf \
    -tcpwindow 512k \
    -tcpslowstart 4 \
    -power {npc107 2} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
    -tag KIRIN_BRANCH_5_100 \
    -date 2011.5.20

# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 3
UTF::Cygwin scr2mc1tst11 -user user -sta {4322WIN8} \
    -osver 7 \
    -installer inf \
    -ssh ssh \
    -tcpwindow 512k \
    -power {npc107 3} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}}

# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 4
UTF::Cygwin scr2mc1tst12 -user user -sta {43224XP} \
    -osver 5 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc107 4} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -tag KIRIN_BRANCH_5_100 \
    -date 2011.5.20


# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 7
# aspm 0x100 is disabling L0s and L1 due to PR#65045 and PR#64659
UTF::Cygwin scr2mc1tst15 -user user -sta {4321XP} \
    -osver 5 \
    -installer inf \
    -tcpwindow 512k \
    -wlinitcmds {wl aspm 0x100} \
    -power {npc107 7} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -tag KIRIN_BRANCH_5_100 \
    -date 2011.5.20


# STA DB43LD Desktop
# scr2mc1kvm152 Port 2
UTF::Cygwin scr2mc1tst20 -user user -sta {4318XP} \
    -osver 5 \
    -installer inf \
    -tcpwindow auto \
    -power {npc104 4} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -tag KIRIN_BRANCH_5_100 \
    -date 2011.5.20



# STA DB43LD Desktop
# scr2mc1kvm152 Port 3
UTF::Cygwin scr2mc1tst21 -user user -sta {4306Vista} \
    -osver 6 \
    -installer inf \
    -tcpwindow auto \
    -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
    -power {npc104 5} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -tag KIRIN_BRANCH_5_100 \
    -date 2011.5.20

