# -*-tcl-*-
#
# testbed configuration for Peter Kwan MC50 testbed
# RvR specific version, based on mc50.tcl created 7/29/10
# 
# created 8/11/10

# Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::DHD
# package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
# set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}
# -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext4/$::env(LOGNAME)/mc50"

# list of devices and IP addresses assigned
# 10.19.86.198	mc50end1
# 10.19.86.199	mc50end2
# 10.19.86.200	mc50tst1
# 10.19.86.201	mc50tst2
# 10.19.86.202	mc50tst3
# 10.19.86.203	mc50tst4
# 10.19.86.204	mc50tst5
# 10.19.86.205	mc50tst6

# NPC definitions
UTF::Power::Synaccess npc1 -lan_ip 172.16.50.10
UTF::Power::Synaccess npc2 -lan_ip 172.16.50.11 ;# enclosure 1 --> 4319SDIO
UTF::Power::Synaccess npc3 -lan_ip 172.16.50.21 ;# enclosure 2 --> 43224Vista
# UTF::Power::Synaccess npc4 -lan_ip 172.16.50.13
# UTF::Power::Synaccess npc5 -lan_ip 172.16.50.14
# Be sure to check the npc definition in STA and AP

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.50.21 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

# Test bed initialization 
# set Aeroflex attenuation, and turn off radio on APs
set ::UTF::SetupTestBed {
	
	ALL attn 20
#     ALL attn 0
	
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# use GigE interface eth2 to connect to LAN side of AP
# both end points are FC-9, tcpslowstart 4 does not apply
UTF::Linux mc50end1 \
    -sta {lan0 eth1} 

# second endpoint
UTF::Linux mc50end2 \
	-sta {lan1 eth1}


# sniffer
# uses wireless interface eth1
# package require UTF::Sniffer ; already included at top of file
# UTF::Sniffer mc28tst6 -sta {snif eth1} \
# 	-power {npc3 2} \
# 	-power_button {auto}

# sample: john brearley mc35.tcl
UTF::DHD mc28tst5 \
    -sta "4336b1 eth1" \
    -tag FALCON_BRANCH_5_90 \
    -type "4336b1-roml/sdio-pool" \
    -nvram "4336b1-roml/bcm94336sdgfc.txt" \
	-power {npc3 1} \
	-power_button {auto} \
    -console "mc28tst5:40000" \
    -postinstall {dhd -i eth1 ioctl_timeout 1000; dhd -i eth1 serial 1} \
    -wlinitcmds {wl mpc 0} \
    -post_perf_hook {{%S wl nrate} {%S wl rate} {%S rte mu}} \
    -noafterburner 1

#     -hostconsole "mc35end1:40001" \
#     -power_button "mc35rly6 1" \
#     -power_button_pushes 2 \
#     -power_sta "mc35rly7 1" \	
#     -postinstall {dhd -i eth1 ioctl_timeout 10000; dhd -i eth1 serial 1} \
#     -nvram "4336b1-roml/bcm94336sdgp.txt" \
#     -nvram "4336b1-roml/bcm94336sdgfc.txt" \

# STA Laptop DUT Dell E6400
# assumes nightly top-of-tree build
# device changed from 4312 to 4313 on 2/26/10
# UTF::Linux mc32tst1 -sta {43227fc9 eth1} \
#     -tag KIRIN_BRANCH_5_100 \
# 	-tcpwindow 512k \
# 	-power {npc3 2} \
# 	-power_button {auto}  \
# 	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
#     -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}} \
#     -console {mc32end1:40002}
# 	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}
#	-tag BASS_BRANCH_5_60
# 
# # clone STAs
# 4313Vista clone 4313Vista-noWEP -nowep 1
# 4313Vista clone 4313Vista-noTKIP -notkip 1
# 4313Vista clone 4313Vista-noAES -noaes 1
# 4313Vista clone 4313Vista-BASS -tag BASS_BRANCH_5_60
# 4313Vista clone 4313Vista-BASS-noWEP -tag BASS_BRANCH_5_60 -nowep 1
# 4313Vista clone 4313Vista-BASS-noTKIP -tag BASS_BRANCH_5_60 -notkip 1
# 4313Vista clone 4313Vista-BASS-noAES -tag BASS_BRANCH_5_60 -noaes 1
# 4313Vista clone 4313Vista-BASS-REL -tag BASS_REL_5_60_106
# 4313Vista clone 4313Vista-BASS-REL-noWEP -tag BASS_REL_5_60_48_* -nowep 1
# 4313Vista clone 4313Vista-BASS-REL-noTKIP -tag BASS_REL_5_60_48_* -notkip 1
# 4313Vista clone 4313Vista-BASS-REL-noAES -tag BASS_REL_5_60_48_* -noaes 1
# 4313Vista clone 4313Vista-KIRIN -tag KIRIN_BRANCH_5_100
# 4313Vista clone 4313Vista-KIRIN-noWEP -tag KIRIN_BRANCH_5_100 -nowep 1
# 4313Vista clone 4313Vista-KIRIN-noTKIP -tag KIRIN_BRANCH_5_100 -notkip 1
# 4313Vista clone 4313Vista-KIRIN-noAES -tag KIRIN_BRANCH_5_100 -noaes 1
# 
# # STA2: Laptop DUT Dell E6400; takes a release build
# UTF::Cygwin mc32tst2 -user user -sta {43224XP} \
# 	-osver 5 \
# 	-installer inf \
# 	-tcpwindow 512k \
# 	-power {npc4 1} \
# 	-power_button {auto} \
# 	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
#     -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}} \
# 	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}
# #	-tag BASS_BRANCH_5_60
# 
# # clone STA2s
# # 43224XP clone 43224XP-noWEP -nowep 1
# # 43224XP clone 43224XP-noTKIP -notkip 1
# # 43224XP clone 43224XP-noAES -noaes 1
# 43224XP clone 43224XP-BASS -tag BASS_BRANCH_5_60
# # 43224XP clone 43224XP-BASS-noWEP -tag BASS_BRANCH_5_60 -nowep 1
# # 43224XP clone 43224XP-BASS-noTKIP -tag BASS_BRANCH_5_60 -notkip 1
# # 43224XP clone 43224XP-BASS-noAES -tag BASS_BRANCH_5_60 -noaes 1
# 43224XP clone 43224XP-BASS-REL -tag BASS_REL_5_60_106
# # 43224XP clone 43224XP-BASS-REL-noWEP -tag BASS_REL_5_60_48_* -nowep 1
# # 43224XP clone 43224XP-BASS-REL-noTKIP -tag BASS_REL_5_60_48_* -notkip 1
# # 43224XP clone 43224XP-BASS-REL-noAES -tag BASS_REL_5_60_48_* -noaes 1
# 43224XP clone 43224XP-KIRIN -tag KIRIN_BRANCH_5_100
# # 43224XP clone 43224XP-KIRIN-noWEP -tag KIRIN_BRANCH_5_100 -nowep 1
# # 43224XP clone 43224XP-KIRIN-noTKIP -tag KIRIN_BRANCH_5_100 -notkip 1
# # 43224XP clone 43224XP-KIRIN-noAES -tag KIRIN_BRANCH_5_100 -noaes 1
# 
# STA3: Laptop DUT Dell E6410; installed 43224
UTF::Cygwin mc50tst3 -user user -sta {43224Vista} \
	-osver 6 \
	-installer inf \
	-tcpwindow 512k \
	-power {npc3 2} \
	-power_button {auto} \
	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}
#	-tag BASS_BRANCH_5_60
# 
# # clone STA3s
# 43224Vista clone 43224Vista-noWEP -nowep 1
# 43224Vista clone 43224Vista-noTKIP -notkip 1
# 43224Vista clone 43224Vista-noAES -noaes 1
# 43224Vista clone 43224Vista-BASS -tag BASS_BRANCH_5_60
# 43224Vista clone 43224Vista-BASS-noWEP -tag BASS_BRANCH_5_60 -nowep 1
# 43224Vista clone 43224Vista-BASS-noTKIP -tag BASS_BRANCH_5_60 -notkip 1
# 43224Vista clone 43224Vista-BASS-noAES -tag BASS_BRANCH_5_60 -noaes 1
43224Vista clone 43224Vista-BASS-REL -tag BASS_REL_5_60_106
# 43224Vista clone 43224Vista-BASS-REL-noWEP -tag BASS_REL_5_60_106 -nowep 1
# 43224Vista clone 43224Vista-BASS-REL-noTKIP -tag BASS_REL_5_60_106 -notkip 1
# 43224Vista clone 43224Vista-BASS-REL-noAES -tag BASS_REL_5_60_106 -noaes 1
43224Vista clone 43224Vista-KIRIN -tag KIRIN_BRANCH_5_100
# 43224Vista clone 43224Vista-KIRIN-noWEP -tag KIRIN_BRANCH_5_100 -nowep 1
# 43224Vista clone 43224Vista-KIRIN-noTKIP -tag KIRIN_BRANCH_5_100 -notkip 1
# 43224Vista clone 43224Vista-KIRIN-noAES -tag KIRIN_BRANCH_5_100 -noaes 1
    
# STA4: Linux desktop; 
# 4319B0 board marking: BCM94319SDELNA6L
UTF::DHD mc50tst4 \
	-lan_ip 10.19.86.203 \
	-sta "4319sdio eth1" \
	-tcpwindow 512k \
	-tag ROMTERM2_BRANCH_4_219 \
	-power {npc2 2} \
	-power_button {auto} \
	-nvram "src/shared/nvram/bcm94319sdelna6l.txt" \
	-brand linux-internal-dongle \
    -customer "bcm" \
    -type "4319b0sdio/sdio-g-cdc-roml-wme-full11n-reclaim-idsup-promiscuous.bin" \
    -wlinitcmds {wl mpc 0} \
    -noafterburner 1 
# 	-pre_perf_hook {%S wl rssi} \
#     -post_perf_hook {{%S wl nrate} {%S wl rate}} 
#     -postinstall {dhd -i eth1 ioctl_timeout 10000; dhd -i eth1 sd_divisor 1}
#     -type "4319b0sdio/sdio-ag-cdc-11n-mfgtest-roml-seqcmds.bin"
# 	-nvram "src/shared/nvram/bcm94319sdelna6l.txt" \
# 	-tag ROMTERM2_REL_4_219_* \ ; This one works ...

#   this is the preferred combo
# 	-tag ROMTERM2_BRANCH_4_219 \
#     -brand linux-external-dongle-sdio \
#     -type "4319b0sdio/sdio-ag-cdc-full11n-reclaim-roml-wme-idsup-medioctl-40m-memredux40m.bin" \

# mfg driver info
# 	-brand linux-mfgtest-dongle-sdio \
#     -type "4319b0sdio/sdio-ag-cdc-11n-mfgtest-roml-seqcmds.bin" \

# # clone STA4s
4319sdio clone 4319sdio-Ext -brand linux-external-dongle-sdio -type "4319b0sdio/sdio-ag-cdc-full11n-reclaim-roml-wme-idsup-medioctl-40m-memredux40m.bin"

# Linksys 320N 4717/4322 wireless router.
UTF::Router AP1 \
    -sta "4717 eth1" \
	-lan_ip 192.168.1.1 \
    -relay "mc50end1" \
    -lanpeer lan0 \
    -console "mc50end1:40001" \
    -power {npc1 1} \
    -tag COMANCHE2_REL_5_22_?? \
	-brand linux-external-router \
    -nvram {
        et0macaddr=00:90:4c:07:00:00
        macaddr=00:90:4c:07:01:0a
		lan_ipaddr=192.168.1.1
		lan_gateway=192.168.1.1
		dhcp_start=192.168.1.100
  		dhcp_end=192.168.1.149
		lan1_ipaddr=192.168.2.1
		lan1_gateway=192.169.2.1
		dhcp1_start=192.168.2.100
	    dhcp1_end=192.168.2.149
       	fw_disable=1
       	wl_msglevel=0x101
        wl0_ssid=mc50test
        wl0_channel=1
        wl0_radio=1
        antswitch=0
        obss_coex=0
}
# 	-tag "COMANCHE2_REL_5_22_80" \

# UTF::Router AP2 \
#     -sta "4705 eth2" \
#     -lan_ip 192.168.1.2 \
#     -relay "mc32end2" \
#     -lanpeer lan1 \
#     -console "mc32end2:40000" \
#     -power {npc2 1} \
#     -brand linux-internal-router \
#     -nvram {
#         et0macaddr=00:90:4c:07:02:00
#         macaddr=00:90:4c:07:03:0a
# 		lan_ipaddr=192.168.1.2
# 		lan_gateway=192.168.1.2
# 		dhcp_start=192.168.1.199
# 	    dhcp_end=192.168.1.249
# 		lan1_ipaddr=192.168.2.2
# 		lan1_gateway=192.169.2.2
# 		dhcp1_start=192.168.2.199
# 	    dhcp1_end=192.168.2.249
#        	fw_disable=1
#        	wl_msglevel=0x101
#        	wl1_ssid=mc32_4705
#        	wl0_channel=1
#        	wl0_radio=0
# #        	wl1_radio=0
#        	{landevs=vlan1 wl0 wl1}
#        	wandev=et0
#        	{vlan1ports=1 2 3 4 8*}
#        	{vlan2ports=0 8u}
# #         wl0_ssid=mc32_4705
# #         wl0_channel=1
# #         antswitch=0
# }
#     -tag COMANCHE2_REL_5_22_?? \
#     -console "mc32end2:40000" \
# #     -tag "COMANCHE2_REL_5_22_80" \

# clone AP1s
4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
4717 clone 4717MILLAU -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
4717 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router

# clone AP2s
# 4705 clone 4705COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
# 4705 clone 4705MILLAU -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router