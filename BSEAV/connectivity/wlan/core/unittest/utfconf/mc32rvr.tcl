# -*-tcl-*-
#
# testbed configuration for MC32 testbed; specific for RvR tests
# created 7/12/10

# -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}

# Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::DHD
# package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
# set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext4/$::env(LOGNAME)/mc32"

# NPC definitions
UTF::Power::Synaccess npc1 -lan_ip 172.16.1.31
UTF::Power::Synaccess npc2 -lan_ip 172.16.1.20
UTF::Power::Synaccess npc3 -lan_ip 172.16.1.33
UTF::Power::Synaccess npc4 -lan_ip 172.16.1.34
UTF::Power::Synaccess npc5 -lan_ip 172.16.1.35
# Be sure to check the npc definition in STA and AP

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.1.21 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

# Test bed initialization for RvR tests
# set Aeroflex attenuation, and turn off radio on AP2
set ::UTF::SetupTestBed {
	
#     ALL attn 0
    G1 attn 10
    G2 attn 20

    AP2 restart wl1_radio=0 
    
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# use GigE interface eth2 to connect to LAN side of AP
# both end points are FC-9, tcpslowstart 4 does not apply
UTF::Linux mc32end1 \
    -sta {lan0 eth1} 

# second endpoint
UTF::Linux mc32end2 \
	-sta {lan1 eth1}

UTF::Linux mc32tst1 -sta {43227fc9 eth1} \
    -tag KIRIN_BRANCH_5_100 \
	-tcpwindow 512k \
	-power {npc3 2} \
	-power_button {auto}  \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}} \
    -console {mc32end1:40002}
    
# 
# # STA2: Laptop DUT Dell E6400; takes a release build
UTF::Cygwin mc32tst2 -user user -sta {43224XP} \
	-osver 5 \
	-installer inf \
	-tcpwindow 512k \
	-power {npc4 1} \
	-power_button {auto} \
	-allowdevconreboot 1 \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}} \
	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}
# #	-tag BASS_BRANCH_5_60
# 	-allowdevconreboot 1 \ ;# new option as of 9/1/10
# 
# # clone STA2s
43224XP clone 43224XP-BASS -tag BASS_BRANCH_5_60
43224XP clone 43224XP-BASS-REL -tag BASS_REL_5_60_106
43224XP clone 43224XP-KIRIN -tag KIRIN_BRANCH_5_100

# 
# # STA3: Laptop DUT Dell E6400; installed a different STA 43224
# UTF::Cygwin mc32tst3 -user user -sta {43224Vista3} \
# 	-osver 6 \
# 	-installer inf \
# 	-tcpwindow 512k \
# 	-tcpslowstart 4 \
# 	-power {npc4 1} \
# 	-power_button {auto} \
# 	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
# 	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
#     -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}
# #	-tag BASS_BRANCH_5_60
# 
# # clone STA3s
# 43224Vista3 clone 43224Vista3-noWEP -nowep 1
# 43224Vista3 clone 43224Vista3-noTKIP -notkip 1
# 43224Vista3 clone 43224Vista3-noAES -noaes 1
# 43224Vista3 clone 43224Vista3-BASS -tag BASS_BRANCH_5_60
# 43224Vista3 clone 43224Vista3-BASS-noWEP -tag BASS_BRANCH_5_60 -nowep 1
# 43224Vista3 clone 43224Vista3-BASS-noTKIP -tag BASS_BRANCH_5_60 -notkip 1
# 43224Vista3 clone 43224Vista3-BASS-noAES -tag BASS_BRANCH_5_60 -noaes 1
# # 43224Vista3 clone 43224Vista3-BASS-REL -tag BASS_REL_5_60_48_*
# 43224Vista3 clone 43224Vista3-BASS-REL -tag BASS_REL_5_60_106
# 43224Vista3 clone 43224Vista3-BASS-REL-noWEP -tag BASS_REL_5_60_106 -nowep 1
# 43224Vista3 clone 43224Vista3-BASS-REL-noTKIP -tag BASS_REL_5_60_106 -notkip 1
# 43224Vista3 clone 43224Vista3-BASS-REL-noAES -tag BASS_REL_5_60_106 -noaes 1
# 43224Vista3 clone 43224Vista3-KIRIN -tag KIRIN_BRANCH_5_100
# 43224Vista3 clone 43224Vista3-KIRIN-noWEP -tag KIRIN_BRANCH_5_100 -nowep 1
# 43224Vista3 clone 43224Vista3-KIRIN-noTKIP -tag KIRIN_BRANCH_5_100 -notkip 1
# 43224Vista3 clone 43224Vista3-KIRIN-noAES -tag KIRIN_BRANCH_5_100 -noaes 1

# STA4: Linux desktop; installed a different STA 4336
# UTF::Linux mc28tst4 \
# 	-sta "4336fc9 eth1" \
# 	-power {npc5 1} \
# 	-power_button {auto}
#	-tag BASS_BRANCH_5_60

UTF::Linux mc32tst3 \
    -sta "4313fc9 eth1" \
	-tcpwindow 512k \
	-power {npc4 2} \
	-power_button {auto} \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate} {%S rte mu}} \
    -console {mc32end1:40001}
#   -sta "43224fc9 eth1" \ ;# original device
	
# clone 4313
4313fc9 clone 4313fc9-BASS -tag BASS_BRANCH_5_60
4313fc9 clone 4313fc9-BASS-REL -tag BASS_REL_5_60_106
4313fc9 clone 4313fc9-KIRIN -tag KIRIN_BRANCH_5_100 -date 2010.5.27


# STA4: Linux desktop; installed a different STA 4336
# 4336A0 board marking: BCM94336SDGFC
UTF::DHD mc32tst4 \
	-sta "4336sdio eth1" \
	-power {npc5 1} \
	-power_button {auto} \
	-tag F17_BRANCH_5_91 \
	-type "4336b0-roml/sdio-g" \
	-nvram "4336b0-roml/bcm94336sdgfc.txt" \
	-postinstall {dhd -i eth1 ioctl_timeout 10000} \
    -wlinitcmds {wl mpc 0} \
    -noafterburner 1

# # clone STA4s
4336sdio clone 4336b0 \
	-tag F17_BRANCH_5_91 \
	-type "4336b0-roml/sdio-g" \
	-nvram "4336b0-roml/bcm94336sdgfc.txt" 
4336sdio clone 4336sdio-Ext -brand linux-external-dongle-sdio
4336sdio clone 4336sdio-F17-REL -tag F17_REL_5_91_*
4336sdio clone 4336sdio-F17-REL-Ext -tag F17_REL_5_91_* -brand linux-external-dongle-sdio
4336sdio clone 4336A0 -tag F15_BRANCH_5_50 -type "4336a0-roml/sdio-g" -nvram "4336a0-roml/bcm94336sdgfc.txt"
# -date 2010.5.14
4336sdio clone 4336A0Ext -tag F15_BRANCH_5_50 -type "4336a0-roml/sdio-g" -nvram "4336a0-roml/bcm94336sdgfc.txt" -brand linux-external-dongle-sdio

# Linksys 320N 4717/4322 wireless router.
UTF::Router AP1 \
    -sta "4717 eth1" \
	-lan_ip 192.168.1.1 \
    -relay "mc32end1" \
    -lanpeer lan0 \
    -console "mc32end1:40000" \
    -power {npc1 1} \
    -tag COMANCHE2_REL_5_22_?? \
	-brand linux-internal-router \
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
        wl0_ssid=mc32test
        wl0_channel=1
        antswitch=0
}
#        wl0_radio=0

UTF::Router AP2 \
    -sta "4705 eth2" \
    -lan_ip 192.168.1.2 \
    -relay "mc32end2" \
    -lanpeer lan1 \
    -console "mc32end2:40000" \
    -power {npc2 1} \
    -brand linux-internal-router \
    -nvram {
        et0macaddr=00:90:4c:07:02:00
        macaddr=00:90:4c:07:03:0a
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.199
	    dhcp_end=192.168.1.249
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.169.2.2
		dhcp1_start=192.168.2.199
	    dhcp1_end=192.168.2.249
       	fw_disable=1
       	wl_msglevel=0x101
       	wl1_ssid=mc32_4705
       	wl0_channel=1
       	wl0_radio=0
       	wl1_radio=0
       	{landevs=vlan1 wl0 wl1}
       	wandev=et0
       	{vlan1ports=1 2 3 4 8*}
       	{vlan2ports=0 8u}
}

# clone AP1s
4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
4717 clone 4717MILLAU -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router

# clone AP2s
4705 clone 4705COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
4705 clone 4705MILLAU -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router

# notes for LinkSys WRT610N/4322/4705 routers
# -nvram { 
	# Basic debugging 
# 	wl_msglevel=0x101 
	# Identify Router 
# 	wl1_ssid=WRT610/4322 
	# Set a default channel since autochannel is bad for testing. 
# 	wl0_channel=11 
	# Disable first radio, so it won't interfere with second in 2.4GHz mode 
# 	wl0_radio=0 
	# Attach both radios to the LAN 
# 	{landevs=vlan1 wl0 wl1} 
	# Reorder vlan ports to match Broadcom conventions 
# 	wandevs=et0 {vlan1ports=1 2 3 4 8*} {vlan2ports=0 8u} 
# 	}