# UTF Config file for MFG02 Station (Single DUT) - shuynh

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_dvt/DVT/mfg02"

#UTF::Logfile "~/utf_test.log" 
UTF::Power::DLI ps-mfg02 -passwd hrun*10

# BCM43224   Regression  driver: KIRIN_REL_5_100_82_15   	   	-script sweep43224.txt    /  single43224.txt
# BCM4313ipa Regression  driver: KIRIN_REL_5_100_82_15          -script sweep4313IPA.txt  /  single4313IPA.txt
# BCM4313epa Regression  driver: PRESERVED/BASS_REL_5_60_48_56	-script sweep_4313EPA.txt /  single_4313EPA.txt
# BCM43228   Regression  driver: KIRIN_REL_5_100_88             -script sweep43228.txt    /  single43228.txt
# BCM43227   Regression  driver: KIRIN_REL_5_100_9_23           -script sweep43227.txt    /  single43227.txt
# BCM4322MC  Calibration driver: PBR_REL_5_10_131_36

#set mfgcbrand_default GOLF
#set mfgcdate_default default
#set mfgctag_default Golf_2.6.22
set mfgcpathdir {C:\Program Files\Golf_2.6.22}

# MFG01D0 -- Device under test
UTF::Cygwin mfg02d0 -sta D0 -brand win_mfgtest_wl -type "Internal" \
    -mfgcref REF -usedll 1 -tag KIRIN_REL_5_100_82_15 \
	-mfgcpath $mfgcpathdir \
	-power {ps-mfg02 6} \
    -user hwlab -installer sys -debuginf 1 
#	-mfgcbrand $mfgcbrand_default -mfgcdate $mfgcdate_default -mfgctag $mfgctag_default

D0 configure -ipaddr 192.168.1.5

# MFG01R0 -- Reference
UTF::Cygwin mfg02r0 -sta R0 -brand win_mfgtest_wl -type "Internal" \
    -usedll 1 -mfgcpath $mfgcpathdir \
    -power {ps-mfg02 7} \
    -user hwlab -installer inf -debuginf 1 -tag PBR_REL_5_10_105 
R0 configure -ipaddr 192.168.1.2