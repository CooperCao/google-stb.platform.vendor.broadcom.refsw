# UTF Config file for MFG Station 5
# 	$Id$


# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_sig/mfg/mfg05"

UTF::Power::DLI ps-mfg05 -passwd hrun*10


# MFGC releases are here:
# /projects/hnd_dvt/MFGC_RELEASE/

# ref script here:
# cvs co -p src/tools/wlan/dvtc_scripts/goldenRefScript_agn.txt

# Reference
UTF::Cygwin mfg05r0 -sta REF -brand win_mfgtest_wl -type "Internal" \
    -mfgcpath "C:/Program Files/Hotel_2.7.16" \
    -power {ps-mfg05 7} \
    -user hwlab -tag PBR_REL_5_10_105
REF configure -ipaddr 192.168.1.2


# Device under test
UTF::Cygwin mfg05d0 -sta TOT -brand win_mfgtest_wl -type "Internal" \
    -mfgcpath "C:/Program Files/Hotel_2.7.16" \
    -power {ps-mfg05 6} \
    -mfgcref REF \
    -mfgcscript goldenRefScript_gn_hotel.txt \
    -user hwlab -mfgcusecc 0

TOT configure -ipaddr 192.168.1.4

TOT clone KIRIN  -tag KIRIN_BRANCH_5_100
KIRIN configure -ipaddr 192.168.1.4

KIRIN clone KI -brand win_internal_wl -type checked
KI configure -ipaddr 192.168.1.4

TOT clone K82  -tag KIRIN_REL_5_100_82_*
K82 configure -ipaddr 192.168.1.4

TOT clone RUBY  -tag RUBY_BRANCH_6_20
RUBY configure -ipaddr 192.168.1.4

RUBY clone RI -brand win_internal_wl -type checked
RI configure -ipaddr 192.168.1.4


