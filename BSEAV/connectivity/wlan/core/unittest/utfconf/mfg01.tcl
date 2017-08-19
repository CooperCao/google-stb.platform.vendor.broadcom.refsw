# UTF Config file for MFG01 Station - shuynh

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_dvt/DVT/mfg01"

#UTF::Logfile "~/utf_test.log" 
UTF::Power::DLI ps-mfg01 -passwd hrun*10

# BCM43224   Regression  driver: PBR_REL_5_10_131_33   -script sweep4224.txt     /  single4224.txt
# BCM4313ipa Regression  driver: KIRIN_REL_5_100_59    -script sweep4313IPA.txt  /  single4313IPA.txt
# BCM4313epa Regression  driver: BASS_REL_5_60_48_56   -script sweep_4313EPA.txt /  single_4313EPA.txt
# BCM43227   Regression  driver: KIRIN_REL_5_100_9_23  -script sweep43227.txt    /  single43227.txt
# BCM43228   Regression  driver: KIRIN_REL_5_100_88    -script sweep43228.txt    /  single43228.txt
# BCM4322MC  Calibration driver: PBR_REL_5_10_131_36

# (loadmfgc) Parameters:
# -mfgcbrand HOTEL
# -mfgctag Firefly_2.5.40
# -mfgcdate 2010_09_21

# Example mfgcpath for NIGHTLY mfgc
# C:/Program Files/HOTEL/2010_09_17

#set mfgcbrand_default FIREFLY
#set mfgcdate_default default
#set mfgctag_default Firefly_2.5.222
set mfgcpathdir {C:\Program Files\Golf_2.6.15}

# MFG01D0 -- Device under test
UTF::Cygwin mfg01d0 -sta DUT0 -brand win_mfgtest_wl -type "Internal" \
    -usedll 1 -tag PBR_REL_5_10_131_33 \
	-mfgcpath $mfgcpathdir \
	-power {ps-mfg01 6} \
    -user hwlab -installer inf -debuginf 1 
	
# MFG01D1 -- Device under test	
UTF::Cygwin mfg01d1 -sta DUT1 -brand win_mfgtest_wl -type "Internal" \
    -usedll 1 -tag KIRIN_REL_5_100_59 \
	-mfgcpath $mfgcpathdir \
	-power {ps-mfg01 6} \
    -user hwlab -installer inf -debuginf 1 
	
# MFG01D2 -- Device under test
UTF::Cygwin mfg01d2 -sta DUT2 -brand win_mfgtest_wl -type "Internal" \
    -usedll 1 -tag BASS_REL_5_60_48_56 \
	-mfgcpath $mfgcpathdir \
	-power {ps-mfg01 6} 

# MFG01D3 -- Device under test
UTF::Cygwin mfg01d3 -sta DUT3 -brand win_mfgtest_wl -type "Internal" \
    -usedll 1 -tag KIRIN_REL_5_100_88 \
	-mfgcpath $mfgcpathdir \
	-power {ps-mfg01 6} \
    -user hwlab -installer sys	-debuginf 1 
	
DUT0 configure -ipaddr 192.168.1.5
DUT1 configure -ipaddr 192.168.1.15 
DUT2 configure -ipaddr 192.168.1.1
DUT3 configure -ipaddr 192.168.1.10

# MFG01R0 -- Reference
UTF::Cygwin mfg01r0 -sta REF -brand win_mfgtest_wl -type "Internal" \
    -usedll 1 -mfgcpath "C:/Program Files/Golf_2.6.14" \
    -power {ps-mfg01 7} \
    -user hwlab -installer inf -debuginf 1 -tag PBR_REL_5_10_105 
REF configure -ipaddr 192.168.1.2

# MFG01 -- Controller
UTF::Cygwin mfg01 -sta CON \
    -mfgcpath "C:/Program Files/Golf_2.6.14" \
	-power {ps-mfg01 7} \
    -user hwlab