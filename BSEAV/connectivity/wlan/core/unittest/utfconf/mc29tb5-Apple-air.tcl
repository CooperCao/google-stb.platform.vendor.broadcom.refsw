# -*-tcl-*-

#
# Basic UTF config file for learning
#
package require UTF::Linux
package require UTF::Aeroflex
package require UTF::Sniffer
#package require UTF::Cygwin
#package require UTF::MacOS
# SummaryDir sets the location for test results
#set ::UTF::SummaryDir "/projects/hnd_sig_ext2/qgao/tb5_mc29/apple/leopard/4331/air3way"
set ::UTF::SummaryDir "/projects/hnd_sig_ext/$::env(LOGNAME)/mc29tb5"

# Linux Endpoint
UTF::Linux ENDROAM -lan_ip 10.19.84.227
#UTF::Linux ENDROAM -lan_ip 10.19.84.227 -sta {lan eth2}

# Sniffer Endpoint
# -lan_ip 172.16.1.103 -relay "ENDROAM" -user root -tag PBR_BRANCH_5_10 -sta {Sta4310 eth1}

#UTF::Sniffer SNIF -lan_ip 172.16.1.103 -relay "ENDROAM" -user root -tag PBR_BRANCH_5_10 -sta {Sta4321 eth1}

#
# Aeroflex Variable Attenuator
#
UTF::Aeroflex Aflex -lan_ip 10.19.85.228 \
	-relay "ENDROAM" \
        -group {F {1 2 3 4} ALL {1 2 3 4}}

# STA Laptop DUT (Windows:Cygwin system in the RF 4400 enclosure)
#
# -tag PBR_REL_5_10_94
#UTF::Cygwin RoamDUT -lan_ip 172.16.1.122 -user user -tag PBR_REL_5_10_* -sta {Sta43224}
#UTF::Cygwin stvtb2-hp6510win7 -lan_ip 172.16.1.133 -user user -tag BASS_REL_5_60_48_* -osver 7 -sta {StaHP6510Win7}
#UTF::Cygwin RoamDUT -lan_ip 172.16.1.122 -user user -tag PBR_REL_5_10_* -sta {Sta43224}

#UTF::MacOS mc29-apple16 -lan_ip 10.19.85.225 -user root -tag BASS_REL_5_60_* -brand "macos-internal-wl-snowleopard" -coreserver AppleCore -type Debug_10_6 -kextload true -sta {StaAppleLeo en1} 
UTF::MacOS mc29-mainapple -lan_ip 10.19.85.229 -user root -tag NIGHTLY -brand "macos-internal-wl-snowleopard" -coreserver AppleCore -type Debug_10_6 -kextload true -sta {lan en0}
UTF::MacOS mc29-apple18 -lan_ip 10.19.85.225 -user root -tag KIRIN_REL_5_100_* -brand "macos-external-wl-snowleopard" -coreserver AppleCore -type Debug_10_6 -kextload true -sta {StaAppleLeo en1} -noafterburner 1
UTF::MacOS mc29-apple19 -lan_ip 10.19.85.227 -user root -tag KIRIN_REL_5_100_* -brand "macos-internal-wl-snowleopard" -coreserver AppleCore -type Debug_10_6 -kextload true -sta {StaAppleX19 en1} -noafterburner 1
UTF::MacOS mc29-apple28 -lan_ip 10.19.84.243 -user root -tag KIRIN_REL_5_100_98_78 -brand "macos-internal-wl-snowleopard" -coreserver AppleCore -type Debug_10_6 -kextload true -sta {StaAppleX28 en1} -noafterburner 1
# Dell E6400 Laptop win 7 with 4313NIC
UTF::Cygwin qgao-lp3 -user user -lan_ip 10.19.84.243 -sta {4331-DE6400Win7} -osver 7 -noafterburner 1 -tcpwindow 1024k
#
# 2nd router with IP 192.168.1.20 controls 4717
#
UTF::Router _4717 \
    -name 4717 \
    -lan_ip 192.168.1.20 \
    -sta {4717 eth1} \
    -power {172.16.1.20 1} \
    -relay "ENDROAM" \
    -lanpeer lan \
    -console "10.22.20.97:40020" \
    -tag COMANCHE2_REL_5_22_* \
    -nvram {
             wl_msglevel=0x101
             fw_disable=1
             watchdog=3000
             wl0_ssid=SSID_4717
             wl0_channel=1
             wl0_nbw_cap=1
             dhcp_start=192.168.1.130
             dhcp_end=192.168.1.150
             macaddr=00:90:4c:07:00:2a
    }

#
# 3rd router with SSID Test3WayNet2
# 

UTF::Router _air2port \
    -name air2port \
    -lan_ip 192.168.1.1 \
    -sta {air2port eth1} \
    -power {172.16.1.10 1} \
    -relay "ENDROAM" \
    -lanpeer lan \
    -console "172.16.1.10:2001" \
    -tag BASS_REL_5_10_85 \
    -nvram {
               watchdog=3000
               macaddr=d4:9a:20:78:51:fa
               wl0_channel=1
               wl0_nbw_cap=1
               wl0_ssid=Test3WayNet2
               lan_ipaddr=192.168.1.1
               lan_gateway=192.168.1.1
               dhcp_start=192.168.1.2
               dhcp_end=192.168.1.240
    }

#
# 3rd router with SSID Test3WayNet2
#

UTF::Router _airBport \
    -name airBport \
    -lan_ip 192.168.1.1 \
    -sta {airBport eth1} \
    -power {172.16.1.10 1} \
    -relay "ENDROAM" \
    -lanpeer lan \
    -console "172.16.1.10:2001" \
    -tag BASS_REL_5_10_85 \
    -nvram {
               watchdog=3000
               macaddr=d4:9a:20:78:51:fa
               wl0_channel=1
               wl0_nbw_cap=1
               wl0_ssid=Broadcom33
               lan_ipaddr=192.168.1.1
               lan_gateway=192.168.1.1
               dhcp_start=192.168.1.2
               dhcp_end=192.168.1.240
    }

#
# 4th router with SSID Test3WayNet1
#

UTF::Router _air1port \
    -name air1port \
    -lan_ip 192.168.1.1 \
    -sta {air1port eth1} \
    -power {172.16.1.10 1} \
    -relay "ENDROAM" \
    -lanpeer lan \
    -console "172.16.1.10:2001" \
    -tag BASS_REL_5_10_90 \
    -nvram {
               watchdog=3000
               macaddr=d4:9a:20:78:51:fa
               wl0_channel=1
               wl0_nbw_cap=1
               wl0_ssid=Test3WayNet1
               lan_ipaddr=192.168.1.1
               lan_gateway=192.168.1.1
               dhcp_start=192.168.1.2
               dhcp_end=192.168.1.240
    }
     
#StaAppleLeo configure -ipaddr 192.168.1.2
#Sta43224 configure -ipaddr 192.168.1.122
#rvr_4315USB configure -ipaddr 192.168.1.110
