# -*-tcl-*-
#
# Testbed configuration file for dogfoodtestbed
#

# Load Packages
package require UTF::Linux
#package require UTF::TclReadLines

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/DogFood/logs/"

# Define power controllers on cart
UTF::Power::Synaccess npc221 -lan_ip 10.19.9.7
UTF::Power::Synaccess npc222 -lan_ip 10.19.9.9
UTF::Power::Synaccess npc223 -lan_ip 10.19.9.11
UTF::Power::Synaccess npc224 -lan_ip 10.19.9.13
UTF::Power::Synaccess npc225 -lan_ip 10.19.9.15
UTF::Power::Synaccess npc226 -lan_ip 10.19.9.17
UTF::Power::Synaccess npc227 -lan_ip 10.19.9.19
UTF::Power::Synaccess npc228 -lan_ip 10.19.9.21
UTF::Power::Synaccess npc229 -lan_ip 10.19.9.23
UTF::Power::Synaccess npc2210 -lan_ip 10.19.9.25
UTF::Power::Synaccess npc2211 -lan_ip 10.19.9.27
UTF::Power::Synaccess npc2212 -lan_ip 10.19.9.29
UTF::Power::Synaccess npc2213 -lan_ip 10.19.9.31
UTF::Power::Synaccess npc2214 -lan_ip 10.19.9.33
UTF::Power::Synaccess npc2215 -lan_ip 10.19.9.35
UTF::Power::Synaccess npc2216 -lan_ip 10.19.9.37
UTF::Power::Synaccess npc2217 -lan_ip 10.19.9.39
UTF::Power::Synaccess npc2218 -lan_ip 10.19.9.41
UTF::Power::Synaccess npc2219 -lan_ip 10.19.9.43
UTF::Power::Synaccess npc2220 -lan_ip 10.19.9.45

# UTF Endpoint1 FC9 - Traffic generators (no wireless cards)
UTF::Linux dfserver \
    -sta {lan eth1} 
    
# Define the tag for Firmware on APs
set FW_VER {AKASHI_REL_5_110_7}
set AC_FW_VER {AARDVARK_REL_6_30_39_29}


# Linksys E3000 4717/4322 wireless router AP1.
UTF::Router AP1 \
	-sta "5037B eth1" \
	-lan_ip 10.19.9.6 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40001" \
	-power {npc221 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
	-nvram {
		wl0_ssid=Dogfood
		wl1_ssid=Dogfood
		wl1.1_ssid=Dogfood5G
		wl_ssid=Dogfood5G
		wl1.1_wpa_psk=eatdogfood
		wl_wpa_psk=eatdogfood
		wl0_wpa_psk=eatdogfood
		wl1_wpa_psk=eatdogfood
		router_disable=1
		lan_ipaddr=10.19.9.6
		lan_gateway=10.19.9.1
		wl1_channel=149
		wl_channel=149
		wl0_channel=6
		wl1.1_bss_enabled=1
		wl0_bss_enabled=1
		wl1_bss_enabled=1
		wl_bss_enabled=1
		wl_msglevel=0x101
		lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
	}

# NetGear 6300 4706/4360 wireless router 
UTF::Router AP1ac \
	-sta "5037B eth1" \
	-lan_ip 10.19.9.6 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40001" \
	-power {npc221 1} \
	-brand linux26-internal-router \
	-tag $AC_FW_VER \
	-nvram {
		wl0_ssid=Dogfood
		wl1_ssid=Dogfood
		wl1.1_ssid=Dogfood5G
		wl_ssid=Dogfood5G
		wl1.1_wpa_psk=eatdogfood
		wl_wpa_psk=eatdogfood
		wl0_wpa_psk=eatdogfood
		wl1_wpa_psk=eatdogfood
		router_disable=1
		lan_ipaddr=10.19.9.6
		lan_gateway=10.19.9.1
		wl1_channel=149
		wl_channel=149
		wl0_channel=6
		wl1.1_bss_enabled=1
		wl0_bss_enabled=1
		wl1_bss_enabled=1
		wl_bss_enabled=1
		wl_msglevel=0x101
		lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
	}

# Linksys E3000 4717/4322 wireless router AP2.
UTF::Router AP2 \
	-sta "5037A eth1" \
	-lan_ip 10.19.9.8 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40002" \
	-power {npc222 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.8
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
		wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }

# NetGear 6300 4706/4360 wireless router 
UTF::Router AP3ac \
	-sta "4034 eth1" \
	-lan_ip 10.19.9.10 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40003" \
	-power {npc223 1} \
	-brand linux26-internal-router \
	-tag $AC_FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.10
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }

# Linksys E3000 4717/4322 wireless router AP3.
UTF::Router AP3 \
	-sta "4034 eth1" \
	-lan_ip 10.19.9.10 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40003" \
	-power {npc223 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.10
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }

# Linksys E3000 4717/4322 wireless router AP4.
UTF::Router AP4 \
	-sta "4046 eth1" \
	-lan_ip 10.19.9.38 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40004" \
	-power {npc224 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.38
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }	

# Linksys E3000 4717/4322 wireless router AP5.
UTF::Router AP5 \
	-sta "3032 eth1" \
	-lan_ip 10.19.9.14 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40005" \
	-power {npc225 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.14
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }

# Linksys E3000 4717/4322 wireless router AP6.
UTF::Router AP6 \
	-sta "2030 eth1" \
	-lan_ip 10.19.9.16 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40006" \
	-power {npc226 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.16
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }
	
# Linksys E3000 4717/4322 wireless router AP7.
UTF::Router AP7 \
	-sta "2046 eth1" \
	-lan_ip 10.19.9.18 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40007" \
	-power {npc227 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.18
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }

# Linksys E3000 4717/4322 wireless router AP8.
UTF::Router AP8 \
	-sta "24021 eth1" \
	-lan_ip 10.19.9.20 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40008" \
	-power {npc228 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.20
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }

# Linksys E3000 4717/4322 wireless router AP9.
UTF::Router AP9 \
	-sta "25022 eth1" \
	-lan_ip 10.19.9.22 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40009" \
	-power {npc229 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.22
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }
	
# Linksys E3000 4717/4322 wireless router AP10.
UTF::Router AP10 \
	-sta "26023 eth1" \
	-lan_ip 10.19.9.24 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40010" \
	-power {npc2210 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.24
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }

# Linksys E3000 4717/4322 wireless router AP11.
UTF::Router AP11 \
	-sta "26032 eth1" \
	-lan_ip 10.19.9.26 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40011" \
	-power {npc2211 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.26
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }

# Linksys E3000 4717/4322 wireless router AP12.
UTF::Router AP12 \
	-sta "25040 eth1" \
	-lan_ip 10.19.9.28 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40012" \
	-power {npc2212 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.28
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }
	
# Linksys E3000 4717/4322 wireless router AP13.
UTF::Router AP13 \
	-sta "1047 eth1" \
	-lan_ip 10.19.9.30 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40013" \
	-power {npc2213 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.30
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }

# Linksys E3000 4717/4322 wireless router AP14.
UTF::Router AP14 \
	-sta "2011 eth1" \
	-lan_ip 10.19.9.32 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40014" \
	-power {npc2214 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.32
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }


# Linksys E3000 4717/4322 wireless router AP15.
UTF::Router AP15 \
	-sta "3031 eth1" \
	-lan_ip 10.19.9.34 \
	-relay "dfserver" \
	-lanpeer lan \
	-console "dfserver:40015" \
	-power {npc2215 1} \
	-brand linux-internal-router \
	-tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.34
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }

# Linksys E3000 4717/4322 wireless router AP16.
UTF::Router AP16 \
        -sta "1004 eth1" \
        -lan_ip 10.19.9.36 \
        -relay "dfserver" \
        -lanpeer lan \
        -console "dfserver:40016" \
        -power {npc2216 1} \
        -brand linux-internal-router \
        -tag $FW_VER \
        -nvram {
                wl0_ssid=Dogfood
                wl1_ssid=Dogfood
                wl1.1_ssid=Dogfood5G
                wl_ssid=Dogfood5G
                wl1.1_wpa_psk=eatdogfood
                wl_wpa_psk=eatdogfood
                wl0_wpa_psk=eatdogfood
                wl1_wpa_psk=eatdogfood
                router_disable=1
                lan_ipaddr=10.19.9.36
                lan_gateway=10.19.9.1
                wl1_channel=149
                wl_channel=149
                wl0_channel=6
                wl1.1_bss_enabled=1
                wl0_bss_enabled=1
                wl1_bss_enabled=1
                wl_bss_enabled=1
                wl_msglevel=0x101
                lan_route=172.16.1.0:255.255.255.0:10.19.9.5:1
        }

# E6400 FC9 43224NIC SNIFFER
package require UTF::Sniffer
UTF::Sniffer dfsniffer \
    -sta "sniffer eth1"\
    -lan_ip 10.19.9.45
