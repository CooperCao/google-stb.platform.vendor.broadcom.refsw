# UTF Config file for RIVER Station (Single DUT)

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_dvt/DVT/river"

#UTF::Logfile "~/utf_test.log" 
UTF::Power::DLI ps-river -passwd hrun*10

# Example mfgcpath for NIGHTLY mfgc
# C:/Program Files/HOTEL/2010_09_17
# C:\Documents and Settings\hwlab\Desktop\MFGc\NIGHTLY\HOTEL if nightly test

# current date for nightly releases {eg. YYYY_MM_DD -- 1986_02_25}
set current_date [clock format [clock seconds] -format {%Y_%m_%d}]

set mfgcbrand_default HOTEL
set mfgcdate_default $current_date
set mfgctag_default Hotel_2.7.16
set mfgcpathdir {C:\Documents and Settings\hwlab\Desktop\MFGc\NIGHTLY\HOTEL}

# ==================================================================== riverD0 -- Device under test
UTF::Cygwin riverd0 -sta DUT0 -brand win_mfgtest_wl -type "Internal" \
    -mfgcref REF -usedll 1 -tag KIRIN_REL_5_106_98_28 \
	-mfgcpath $mfgcpathdir -mfgctag $mfgctag_default \
	-power {ps-river 6} \
    -user hwlab -installer inf -debuginf 1 \
	-mfgcbrand $mfgcbrand_default -mfgcdate $mfgcdate_default 
DUT0 configure -ipaddr 192.168.1.101

# ==================================================================== riverR0 -- Reference
UTF::Cygwin riverr0 -sta REF -brand win_mfgtest_wl -type "Internal" \
    -usedll 1 -mfgcpath $mfgcpathdir -mfgctag $mfgctag_default \
    -power {ps-river 7} \
    -user hwlab -installer inf -debuginf 1 -tag KIRIN_REL_5_106_98_28 \
	-mfgcbrand $mfgcbrand_default -mfgcdate $mfgcdate_default 
REF configure -ipaddr 192.168.1.100

# ==================================================================== riverC0 -- Controller
UTF::Cygwin riverc0 -sta CON \
    -mfgcpath $mfgcpathdir -mfgctag $mfgctag_default \
	-power {ps-river 7} \
    -user hwlab -mfgcbrand $mfgcbrand_default -mfgcdate $mfgcdate_default 
