# UTF Config file for Happy Station - shuynh

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_dvt/DVT/happy"

#UTF::Logfile "~/utf_test.log" 
UTF::Power::DLI ps-mfg01 -passwd hrun*10

# Linux Master -- SnowWhite: 10.19.36.21

# Controller
UTF::Cygwin happyc0 -lan_ip 10.19.97.27 -sta c0 \
    -usedll 1 -mfgcpath "C:/Program Files/Firefly_2.5.26" \
    -user hwlab -installer inf -debuginf 1 -tag PBR_REL_5_10_133

#####################

# Reference
UTF::Cygwin happyr0 -lan_ip 10.19.97.27 -sta r0 \
    -usedll 1 -mfgcpath "C:/Program Files/Firefly_2.5.26" \
    -user hwlab -installer inf -debuginf 1 -tag PBR_REL_5_10_133 
r0 configure -ipaddr 192.168.1.100 

#####################

# First Device under test
UTF::Cygwin happyd0 -lan_ip 192.168.2.10 -sta d0 \
    -usedll 1 -mfgcpath "C:/Program Files/Firefly_2.5.29" \
    -user hwlab -installer inf -debuginf 1  -tag KIRIN_REL_5_100_75 

#D0 configure -ipaddr 192.168.1.10


# Second Device under test
UTF::Cygwin happyd1 -lan_ip 192.168.2.11 -sta d1 \
    -usedll 1 -mfgcpath "C:/Program Files/Firefly_2.5.29" \
    -user hwlab -installer inf -debuginf 1 

#D1 configure -ipaddr 192.168.1.11

# Third Device under test
UTF::Cygwin happyd2 -lan_ip 192.168.2.12 -sta d2 \
    -usedll 1 -mfgcpath "C:/Program Files/Firefly_2.5.29" \
    -user hwlab -installer inf -debuginf 1

#D2 configure -ipaddr 192.168.1.12

# Fourth Device under test
UTF::Cygwin happyd3 -lan_ip 192.168.2.13 -sta d3 \
    -usedll 1 -tag "BASS_REL_5_60_350_6" \
    -mfgcpath "C:/Program Files/Firefly_2.5.29" \
    -user hwlab -installer inf -debuginf 1 

#D3 configure -ipaddr 192.168.1.13

#package require UTF::MFGC

#UTF::MFGC colossus -lan_ip happyd0 -dut TOT -ref REF
