#
# Utf configuration for E4200 setup
# Robert J. McMahon (setup in my cube)
#
package require UTF::Router
package require UTF::Linux
package require UTF::WebRelay

# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_sig_ext4/rmcmahon/mc72"

UTF::WebRelay rtrpower -lan_ip 10.19.86.240 -port 1
UTF::WebRelay stapower -lan_ip 10.19.86.240 -port 2
UTF::Linux mc72-wlan -lan_ip 10.19.86.237 -sta {4331sta eth1} -power stapower
UTF::Linux mc72-lan -lan_ip 10.19.86.238 -sta {vlan170_if em1.170}
UTF::Linux mc72-wan -lan_ip 10.19.86.239 -sta {vlan171_if em1.171}

UTF::Router _E4200 -name E4200 \
    -sta {4331rtr eth1} \
    -brand linux-internal-router \
    -tag "AKASHI_BRANCH_5_110" \
    -power rtrpower \
    -relay "mc72-lan" \
    -lanpeer vlan170_if \
    -console 10.19.85.133:40000 \
    -nvram {
	boot_hw_model=E4200
        et0macaddr=00:90:4c:0b:0b:01
        macaddr=00:90:4c:0b:0b:02
        sb/1/macaddr=00:90:4c:0b:0b:03
        pci/1/1/macaddr=00:90:4c:0b:0b:04
	wan_hwaddr=00:90:4c:0b:0b:05
        wandevs=et0
        {lan_ifnames=vlan1 eth1 eth2}
	wan_ifnames=vlan2
	wl_msglevel=0x101
	fw_disable=1
	wl0_ssid=MC72
	wan_ifname=vlan2
	wan0_ifname=vlan2
	wan_proto=static
	wan0_proto=static
	wan_ipaddr=192.168.4.2
	wan0_ipaddr=192.168.4.2
	wan_netmask=255.255.255.0
	wan0_netmask=255.255.255.0
	router_disable=0
    }
