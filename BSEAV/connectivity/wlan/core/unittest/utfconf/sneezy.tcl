#   UTF Config file for Colossus "sneezy"
# 	$Id$

# Note - private network.
# Linux controller: 10.19.36.199
# Windows remote desktop: 10.19.36.53

# Power controller appears to be a "Raritan Dominion PX"
# with ip address 192.168.2.109.  UTF doesn't support this yet.

# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_sig/mfg/sneezy"

# MFGC releases are here:
# /projects/hnd_dvt/MFGC_RELEASE/

# ref script here:
# cvs co -p src/tools/wlan/dvtc_scripts/goldenRefScript_agn.txt

# Controller
UTF::Cygwin sneezyc0 -lan_ip 10.19.97.22 -sta c0 \
    -usedll 1 -mfgcpath "C:/Program Files/Firefly_2.5.26" \
    -user hwlab -installer inf -debuginf 1 -tag PBR_REL_5_10_133

#####################
set asdf "10.19.97.22"

# Reference
UTF::Cygwin sneezyR0 -lan_ip $asdf -sta r0 \
    -usedll 1 -mfgcpath "C:/Program Files/Firefly_2.5.26" \
    -user hwlab -installer inf -debuginf 1 -tag PBR_REL_5_10_133
r0 configure -ipaddr 192.168.1.100 

#####################

# First Device under test
UTF::Cygwin sneezyD0 -lan_ip 192.168.2.10 -sta D0 \
    -usedll 1 -mfgcpath "C:/Program Files/Firefly_2.5.29" \
    -user hwlab -installer inf -debuginf 1 \
    -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}

# D0 appears to be configured for DHCP
#D0 configure -ipaddr 192.168.1.10


# Second Device under test
UTF::Cygwin sneezyD1 -lan_ip 192.168.2.11 -sta D1 \
    -usedll 1 -mfgcpath "C:/Program Files/Firefly_2.5.29" \
    -user hwlab -installer inf -debuginf 1 \
    -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}

# D1 appears to be configured for DHCP
#D1 configure -ipaddr 192.168.1.11

# Third Device under test
UTF::Cygwin sneezyD2 -lan_ip 192.168.2.12 -sta D2 \
    -usedll 1 -mfgcpath "C:/Program Files/Firefly_2.5.29" \
    -user hwlab -installer inf -debuginf 1\
    -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}

# D2 appears to be configured for DHCP
#D2 configure -ipaddr 192.168.1.12

# Fourth Device under test
UTF::Cygwin sneezyD3 -lan_ip 192.168.2.13 -sta D3 \
    -usedll 1 -tag "BASS_REL_5_60_350_6" \
    -mfgcpath "C:/Program Files/Firefly_2.5.29" \
    -user hwlab -installer inf -debuginf 1 \
    -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}

# D3 appears to be configured for DHCP
#D3 configure -ipaddr 192.168.1.13

#package require UTF::MFGC

#UTF::MFGC colossus -lan_ip mfg05d0 -dut TOT -ref REF

