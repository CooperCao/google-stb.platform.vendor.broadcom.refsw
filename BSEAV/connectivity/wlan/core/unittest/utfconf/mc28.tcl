# -*-tcl-*-
#
# testbed configuration for Peter Kwan MC28testbed
#

# changes 11/2/10:
#  added device info on top of file 
# ====================================================================================================
# 
# Test rig ID: SIG MC28
# MC28 testbed configuration
#
# mc28end1  10.19.85.219   Endpoint1		- lan0
# mc28end2  10.19.85.220   Endpoint2		- lan1
# mc28tst1  10.19.85.221   BCM4313      	- 4313Vista
# mc28tst2  10.19.85.222   BCM4312		    - 4312Vista2
# mc28tst3  10.19.85.223   BCM43224			- 43224Vista3
# mc24tst6  10.19.85.249   Sniffer			- snif
# 			172.16.0.1	   npc1
# 			172.16.0.2	   npc2
# 			172.16.0.3	   npc3
# 			172.16.0.4	   npc4
# 			172.16.0.199   Aeroflex			- af
 
# -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}

# Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
### set ::UTF::SummaryDir "/projects/hnd_sig_ext3/$::env(LOGNAME)/mc28"
# moved to dedicated filer volume 10/30/13
set ::UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc28"

# NPC definitions
UTF::Power::Synaccess npc1 -lan_ip 172.16.0.1
UTF::Power::Synaccess npc2 -lan_ip 172.16.0.2
UTF::Power::Synaccess npc3 -lan_ip 172.16.0.3 -rev 1
UTF::Power::Synaccess npc4 -lan_ip 172.16.0.4
# Be sure to check the npc definition in STA and AP

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.0.199 -group {G1 {1 2 3} G2 {4 5 6} ALL {1 2 3 4 5 6}}

# Test bed initialization for RvR tests
# set Aeroflex attenuation, and turn off radio on APs
set ::UTF::SetupTestBed {
	
# 	ALL attn 0 		
	G1 attn 0
	G2 attn 13
	
	AP1 restart wl0_radio=0
 	AP2 restart wl0_radio=0

 	# delete myself
    unset ::UTF::SetupTestBed
    	
	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# use GigE interface eth2 to connect to LAN side of AP
# -tcpslowstart option applies only to FC11 hosts
UTF::Linux mc28end1 \
    -sta {lan0 eth2} 
# # #     \
# # #     -tcpslowstart 4

# second endpoint
# -tcpslowstart option applies only to FC11 hosts
UTF::Linux mc28end2 \
	-sta {lan1 eth1}
# # # 	 \
# # # 	-tcpslowstart 4


# sniffer
# uses wireless interface eth1
# package require UTF::Sniffer ; already included at top of file
UTF::Sniffer mc28tst6 -sta {snif eth1} \
	-power {npc3 2} \
	-power_button {auto}

# STA Laptop DUT Dell E6400
# assumes nightly top-of-tree build
# device changed from 4312 to 4313 on 2/26/10
UTF::Cygwin mc28tst1 -user user -sta {4313Vista} \
	-osver 6 \
	-installer inf \
	-tcpwindow 512k \
	-power {npc1 2} \
	-power_button {auto} \
	-allowdevconreboot 1 \
	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}
#	-tag BASS_BRANCH_5_60

# clone STAs
4313Vista clone 4313Vista-noWEP -nowep 1
4313Vista clone 4313Vista-noTKIP -notkip 1
4313Vista clone 4313Vista-noAES -noaes 1
4313Vista clone 4313Vista-BASS -tag BASS_BRANCH_5_60
4313Vista clone 4313Vista-BASS-noWEP -tag BASS_BRANCH_5_60 -nowep 1
4313Vista clone 4313Vista-BASS-noTKIP -tag BASS_BRANCH_5_60 -notkip 1
4313Vista clone 4313Vista-BASS-noAES -tag BASS_BRANCH_5_60 -noaes 1
4313Vista clone 4313Vista-BASS-REL -tag BASS_REL_5_60_106
4313Vista clone 4313Vista-BASS-REL-noWEP -tag BASS_REL_5_60_48_* -nowep 1
4313Vista clone 4313Vista-BASS-REL-noTKIP -tag BASS_REL_5_60_48_* -notkip 1
4313Vista clone 4313Vista-BASS-REL-noAES -tag BASS_REL_5_60_48_* -noaes 1
4313Vista clone 4313Vista-KIRIN -tag KIRIN_BRANCH_5_100
4313Vista clone 4313Vista-KIRIN-noWEP -tag KIRIN_BRANCH_5_100 -nowep 1
4313Vista clone 4313Vista-KIRIN-noTKIP -tag KIRIN_BRANCH_5_100 -notkip 1
4313Vista clone 4313Vista-KIRIN-noAES -tag KIRIN_BRANCH_5_100 -noaes 1
4313Vista clone 4313Vista-KIRIN-REL -tag KIRIN_REL_5_100_82.12
4313Vista clone 4313Vista-RUBY -tag RUBY_BRANCH_6_20
4313Vista clone 4313Vista-AARDVARK -tag AARDVARK_BRANCH_6_30
4313Vista clone 4313Vista-BISON -tag BISON_BRANCH_7_10

# STA2: Laptop DUT Dell E6400; takes a release build
UTF::Cygwin mc28tst2 -user user -sta {4312Vista2} \
	-osver 6 \
	-installer inf \
	-tcpwindow 512k \
	-power {npc2 2} \
	-power_button {auto} \
	-allowdevconreboot 1 \
	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}
# 	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
#     -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}
#	-tag BASS_BRANCH_5_60

# clone STA2s
4312Vista2 clone 4312Vista2-noWEP -nowep 1
4312Vista2 clone 4312Vista2-noTKIP -notkip 1
4312Vista2 clone 4312Vista2-noAES -noaes 1
4312Vista2 clone 4312Vista2-BASS -tag BASS_BRANCH_5_60
4312Vista2 clone 4312Vista2-BASS-noWEP -tag BASS_BRANCH_5_60 -nowep 1
4312Vista2 clone 4312Vista2-BASS-noTKIP -tag BASS_BRANCH_5_60 -notkip 1
4312Vista2 clone 4312Vista2-BASS-noAES -tag BASS_BRANCH_5_60 -noaes 1
4312Vista2 clone 4312Vista2-BASS-REL -tag BASS_REL_5_60_106
4312Vista2 clone 4312Vista2-BASS-REL-noWEP -tag BASS_REL_5_60_48_* -nowep 1
4312Vista2 clone 4312Vista2-BASS-REL-noTKIP -tag BASS_REL_5_60_48_* -notkip 1
4312Vista2 clone 4312Vista2-BASS-REL-noAES -tag BASS_REL_5_60_48_* -noaes 1
4312Vista2 clone 4312Vista2-KIRIN -tag KIRIN_BRANCH_5_100
4312Vista2 clone 4312Vista2-KIRIN-noWEP -tag KIRIN_BRANCH_5_100 -nowep 1
4312Vista2 clone 4312Vista2-KIRIN-noTKIP -tag KIRIN_BRANCH_5_100 -notkip 1
4312Vista2 clone 4312Vista2-KIRIN-noAES -tag KIRIN_BRANCH_5_100 -noaes 1
4312Vista2 clone 4312Vista2-KIRIN-REL -tag KIRIN_REL_5_100_82.12
4312Vista2 clone 4312Vista2-RUBY -tag RUBY_BRANCH_6_20
4312Vista2 clone 4312Vista2-AARDVARK -tag AARDVARK_BRANCH_6_30
4312Vista2 clone 4312Vista2-BISON -tag BISON_BRANCH_7_10

# STA3: Laptop DUT Dell E6400; installed a different STA 43224
UTF::Cygwin mc28tst3 -user user -sta {43224Vista3} \
	-osver 6 \
	-installer inf \
	-tcpwindow 512k \
	-power {npc2 1} \
	-power_button {auto} \
	-allowdevconreboot 1 \
	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}
#	-tag BASS_BRANCH_5_60
# # # 	-tcpslowstart 4 \

# clone STA3s
43224Vista3 clone 43224Vista3-noWEP -nowep 1
43224Vista3 clone 43224Vista3-noTKIP -notkip 1
43224Vista3 clone 43224Vista3-noAES -noaes 1
43224Vista3 clone 43224Vista3-BASS -tag BASS_BRANCH_5_60
43224Vista3 clone 43224Vista3-BASS-noWEP -tag BASS_BRANCH_5_60 -nowep 1
43224Vista3 clone 43224Vista3-BASS-noTKIP -tag BASS_BRANCH_5_60 -notkip 1
43224Vista3 clone 43224Vista3-BASS-noAES -tag BASS_BRANCH_5_60 -noaes 1
# 43224Vista3 clone 43224Vista3-BASS-REL -tag BASS_REL_5_60_48_*
43224Vista3 clone 43224Vista3-BASS-REL -tag BASS_REL_5_60_106
43224Vista3 clone 43224Vista3-BASS-REL-noWEP -tag BASS_REL_5_60_106 -nowep 1
43224Vista3 clone 43224Vista3-BASS-REL-noTKIP -tag BASS_REL_5_60_106 -notkip 1
43224Vista3 clone 43224Vista3-BASS-REL-noAES -tag BASS_REL_5_60_106 -noaes 1
43224Vista3 clone 43224Vista3-KIRIN -tag KIRIN_BRANCH_5_100
43224Vista3 clone 43224Vista3-KIRIN-noWEP -tag KIRIN_BRANCH_5_100 -nowep 1
43224Vista3 clone 43224Vista3-KIRIN-noTKIP -tag KIRIN_BRANCH_5_100 -notkip 1
43224Vista3 clone 43224Vista3-KIRIN-noAES -tag KIRIN_BRANCH_5_100 -noaes 1
43224Vista3 clone 43224Vista3-KIRIN-REL -tag KIRIN_REL_5_100_82.12
43224Vista3 clone 43224Vista3-RUBY -tag RUBY_BRANCH_6_20
43224Vista3 clone Vista3Restore -tag KIRIN_REL_5_100_82_12
43224Vista3 clone 43224Vista3-AARDVARK -tag AARDVARK_BRANCH_6_30
43224Vista3 clone 43224Vista3-BISON -tag BISON_BRANCH_7_10

# Linksys 320N 4717/4322 wireless router.
UTF::Router AP1 \
    -sta "4717 eth1" \
	-lan_ip 192.168.1.1 \
    -relay "mc28end1" \
    -lanpeer lan0 \
    -console "mc28end1:40000" \
    -power {npc3 1} \
    -nvram {
        et0macaddr=00:90:4c:1C:00:00
        macaddr=00:90:4c:1C:01:0a
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
        wl0_ssid=mc28test
        wl0_channel=1
        antswitch=0
        wl0_obss_coex=0
}
# 	-tag "COMANCHE2_REL_5_22_80" \
# 	-brand linux-internal-router \

UTF::Router AP2 \
    -sta "47171 eth1" \
    -lan_ip 192.168.1.2 \
    -relay "mc28end2" \
    -lanpeer lan1 \
    -console "mc28end2:40000" \
    -power {npc4 1} \
    -nvram {
        et0macaddr=00:90:4c:1C:02:00
        macaddr=00:90:4c:1C:03:0a
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
        wl0_ssid=mc28test1
        wl0_channel=1
        antswitch=0
        wl0_obss_coex=0
}
#     -tag "COMANCHE2_REL_5_22_80" \
#     -brand linux-internal-router \

# clone AP1s
4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_80" -brand linux-internal-router
4717 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
# 5.70.38 depleted as of 8/21/11
# 4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_38" -brand linux-external-router
# 4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_38" -brand linux-internal-router
4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_??" -brand linux-external-router
4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_??" -brand linux-internal-router
4717 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
4717 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
4717 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

# clone AP2s
47171 clone 47171COMANCHE2 -tag "COMANCHE2_REL_5_22_80" -brand linux-internal-router
47171 clone 47171Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
# 5.70.38 depleted as of 8/21/11
# 47171 clone 47171MILLAU -tag "MILLAU_REL_5_70_38" -brand linux-external-router
# 47171 clone 47171MILLAUInt -tag "MILLAU_REL_5_70_38" -brand linux-internal-router
47171 clone 47171MILLAU -tag "MILLAU_REL_5_70_??" -brand linux-external-router
47171 clone 47171MILLAUInt -tag "MILLAU_REL_5_70_??" -brand linux-internal-router
47171 clone 47171MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
47171 clone 47171MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
47171 clone 47171Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
47171 clone 47171AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router