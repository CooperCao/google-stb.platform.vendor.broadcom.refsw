#
# Utf configuration for multicast testing with AP/STA
# Robert J. McMahon (setup in my cube)
#

# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_sig_ext4/rmcmahon/multicast"

package require UTF::Power

#
# Keep Windows STA at released level as it's not under test
#
UTF::Cygwin MCrjmTst1 -lan_ip 10.19.85.173 -sta {4312Vista} -user user \
     -osver 6 -installer inf -brand win_internal_wl -tag "BASS_REL_5_60_18_8"

# -tag "KIRIN_BRANCH_5_100"
# -tag  "KIRIN_REL_5_100_56"
# -date "2010.7.17"
UTF::Linux MCrjmLx4 -lan_ip 10.19.84.250 \
                    -sta {vlan117_if eth0.117}
UTF::Linux MCrjmLx6 -lan_ip 10.19.86.110 \
                     -sta {4322Linux eth1} \
                     -brand "linux-internal-wl" \
                     -tag "KIRIN_BRANCH_5_100" \

vlan117_if configure -ipaddr 192.168.1.117
vlan117_if configure -peer 4717_A
4322Linux configure  -ipaddr 192.168.1.120

package require UTF::AeroflexDirect
UTF::AeroflexDirect AF -lan_ip 10.19.85.174 -group {G1 {1 2} G2 {3 4} G3 { 5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
    G1 attn 20
    return
}



#    -tag "DUCATI_BRANCH_5_24"
#    -tag "MILLAU_REL_5_70_48_1"
#    -tag "MILLAU_BRANCH_5_70"
UTF::Router _4717_A -name 4717_A \
    -sta {4717_A eth1} \
    -brand linux-internal-router \
    -tag "AKASHI_BRANCH_5_110" \
    -power {MCrjmNPC1 1} \
    -relay "MCrjmLx4" \
    -lanpeer vlan117_if \
    -lan_ip 192.168.1.2 \
    -console 10.19.84.250:40000 \
    -nvram {
	wl_msglevel=0x101
	wl0_ssid=MCAST_GOLDEN
 	router_disable=0
	macaddr=00:90:4C:2F:0B:01
	et1macaddr=00:90:4C:2F:0C:01
        lan_ipaddr=192.168.1.2
	aci_daemon=down
    }

4717_A clone 4717_A_DUCATI -tag "DUCATI_BRANCH_5_24"
4717_A clone 4717_A_AKASHI_MEDIA -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router-media
4717_A clone 4717_A_AKASHI -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router

4717_A_DUCATI configure -peer vlan117_if
4717_A_AKASHI configure -peer vlan117_if
4717_A_AKASHI_MEDIA configure -peer vlan117_if
