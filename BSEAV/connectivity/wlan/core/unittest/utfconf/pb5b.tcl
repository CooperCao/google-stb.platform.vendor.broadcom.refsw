#
# UTF configuration for lab 2005 chamber PB5B
#

# To get STA details, type: UTF.tcl <sta> whatami

# ASUS wireless router, 192.168.1.1
# sr1end18 Dell server, 10.19.85.28

# Miscellaneous setup.
set ::UTF::SummaryDir "/projects/hnd_sig_ext12/$::env(LOGNAME)/pb5b"
UTF::Logfile "~/utf_test.log"

set ::UTF::_llogfile ""
set UTF::DataRatePlot 1
set UTF::AggressiveCleaninfs 1

# Define power controllers inside phone booth
package require UTF::Power
set power_ctl pb5bips1
UTF::Power::WTI $power_ctl

set ::UTF::SetupTestBed {
    package require UTF::Test::RebootTestbed
	UTF::Try "Reboot Testbed" {
		UTF::Test::RebootTestbed -hostlist "4312comboWin7" 
    }
    set ::UTF::trailer_info ""
	unset ::UTF::SetupTestBed
	return
}

# 4313 Win7
UTF::Cygwin pb5btst1 \
    -sta "4313XP2s" \
    -user user \
    -installer inf \
	-power "$power_ctl 1" \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl dump ampdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -allowdevconreboot 1 


4313XP2s clone 4313XP2 -tag NIGHTLY
4313XP2s clone 4313XP2-bass -tag BASS_BRANCH_5_60
4313XP2s clone 4313XP2-krn -tag KIRIN_BRANCH_5_100
4313XP2s clone 4313XP2-rub -tag RUBY_BRANCH_6_20
4313XP2s clone 4313XP2-rel14 -tag RUBY_REL_6_20_??
4313XP2s clone 4313XP2-rel5 -tag BASS_REL_5_60_48_*
4313XP2s clone 4313XP2-rel7 -tag BASS_REL_5_60_???
4313XP2s clone 4313XP2-rel8 -tag KIRIN_REL_5_100_???
4313XP2s clone 4313XP2-rel9 -tag KIRIN_REL_5_100_9_*
4313XP2s clone 4313XP2-rel10 -tag KIRIN_REL_5_100_57_*
4313XP2s clone 4313XP2-rel11 -tag KIRIN_REL_5_100_82_*
4313XP2s clone 4313XP2-rel12 -tag KIRIN_REL_5_100_98_*
4313XP2s clone 4313XP2-rel13 -tag KIRIN_REL_5_100_198_*
4313XP2s clone 4313XP2-bu -tag  "BU4360_{BRANCH,REL}_6_25{,_*}"
4313XP2s clone 4313XP2-aa -tag  "AARDVARK_BRANCH_6_30"

4313XP2s clone 4313XP2ext  -brand win_external_wl -type Bcm -tag NIGHTLY
4313XP2s clone 4313XP2ext-krn  -brand win_external_wl -type Bcm -tag KIRIN_BRANCH_5_100
4313XP2s clone 4313XP2ext-rub  -brand win_external_wl -type Bcm -tag RUBY_BRANCH_6_20
4313XP2s clone 4313XP2ext-rel14  -brand win_external_wl -type Bcm -tag RUBY_REL_6_20_??
4313XP2s clone 4313XP2ext-rel8  -brand win_external_wl -type Bcm -tag KIRIN_REL_5_100_???

UTF::Cygwin pb5btst2 \
    -sta "4313win7combos" \
    -power "$power_ctl 6" \
    -power_button "auto"\
    -user user \
    -osver 7 \
    -installer inf \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl dump ampdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -allowdevconreboot 1 

4313win7combos clone 4313win7combo -tag NIGHTLY
4313win7combos clone 4313win7combo-bass -tag BASS_BRANCH_5_60
4313win7combos clone 4313win7combo-krn -tag  KIRIN_BRANCH_5_100 
4313win7combos clone 4313win7combo-rub -tag  RUBY_BRANCH_6_20
4313win7combos clone 4313win7combo-rel14 -tag RUBY_REL_6_20_??
4313win7combos clone 4313win7combo-rel5 -tag BASS_REL_5_60_48_*
4313win7combos clone 4313win7combo-rel7 -tag BASS_REL_5_60_???
4313win7combos clone 4313win7combo-rel8 -tag KIRIN_REL_5_100_???
4313win7combo clone 4313win7combo-rel9 -tag KIRIN_REL_5_100_9_*
4313win7combo clone 4313win7combo-rel10 -tag KIRIN_REL_5_100_57_*
4313win7combo clone 4313win7combo-rel11 -tag KIRIN_REL_5_100_82_*
4313win7combo clone 4313win7combo-rel12 -tag KIRIN_REL_5_100_98_*
4313win7combo clone 4313win7combo-rel13 -tag KIRIN_REL_5_100_198_*
4313win7combo clone 4313win7combo-bu -tag  "BU4360_{BRANCH,REL}_6_25{,_*}"
4313win7combo clone 4313win7combo-aa -tag  "AARDVARK_BRANCH_6_30"

4313win7combos clone 4313comboWin7ext -brand win_external_wl -type Bcm -tag NIGHTLY
4313win7combos clone 4313comboWin7ext-krn -brand win_external_wl -type Bcm -tag KIRIN_BRANCH_5_100
4313win7combos clone 4313comboWin7ext-rub -brand win_external_wl -type Bcm -tag RUBY_BRANCH_6_20
4313win7combos clone 4313comboWin7ext-rel14 -brand win_external_wl -type Bcm -tag RUBY_REL_6_20_??
4313win7combos clone 4313comboWin7ext-rel8 -brand win_external_wl -type Bcm -tag KIRIN_REL_5_100_???

UTF::Cygwin pb5btst3 \
    -sta "4313xps" \
    -user user \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl dump ampdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -installer inf

4313xps clone 4313xp -tag NIGHTLY
4313xps clone 4313xp-bass -tag BASS_BRANCH_5_60
4313xps clone 4313xp-krn -tag KIRIN_BRANCH_5_100
4313xps clone 4313xp-rub -tag RUBY_BRANCH_6_20
4313xps clone 4313xp-rel14 -tag RUBY_REL_6_20_??
4313xps clone 4313xp-rel5 -tag BASS_REL_5_60_48_*
4313xps clone 4313xp-rel7 -tag BASS_REL_5_60_???
4313xps clone 4313xp-rel8 -tag KIRIN_REL_5_100_???
4313xps clone 4313xp-rel9 -tag KIRIN_REL_5_100_9_*
4313xps clone 4313xp-rel10 -tag KIRIN_REL_5_100_57_*
4313xps clone 4313xp-rel11 -tag KIRIN_REL_5_100_82_*
4313xps clone 4313xp-rel12 -tag KIRIN_REL_5_100_98_*
4313xps clone 4313xp-rel13 -tag KIRIN_REL_5_100_198_*
4313xps clone 4313xp-bu -tag  "BU4360_{BRANCH,REL}_6_25{,_*}"
4313xps clone 4313xp-aa -tag  "AARDVARK_BRANCH_6_30"


4313xps clone 4313XPext -tag NIGHTLY -brand win_external_wl -type Bcm
4313xps clone 4313XPext-krn  -brand win_external_wl -type Bcm -tag KIRIN_BRANCH_5_100
4313xps clone 4313XPext-rub -brand win_external_wl -type Bcm -tag RUBY_BRANCH_6_20
4313xps clone 4313XPext-rel14 -brand win_external_wl -type Bcm -tag RUBY_REL_6_20_??
4313xps clone 4313XPext-rel8  -brand win_external_wl -type Bcm -tag KIRIN_REL_5_100_???

#
# 4312comboWin7
UTF::Cygwin pb5btst5 \
   -user user -sta "4312comboWin7" \
   -installer inf \
   -power {pb5bips1 5} \
   -power_button {auto} \
   -osver 7 \
   -allowdevconreboot 1 

4312comboWin7 clone 4312comboWin7-bass -tag BASS_BRANCH_5_60
4312comboWin7 clone 4312comboWin7-krn -tag KIRIN_BRANCH_5_100
4312comboWin7 clone 4312comboWin7-rub -tag RUBY_BRANCH_6_20
4312comboWin7 clone 4312comboWin7-rel14 -tag RUBY_REL_6_20_??
4312comboWin7 clone 4312comboWin7-rel5 -tag BASS_REL_5_60_48_*
4312comboWin7 clone 4312comboWin7-rel7 -tag BASS_REL_5_60_???
4312comboWin7 clone 4312comboWin7-rel8 -tag KIRIN_REL_5_100_???
4312comboWin7 clone 4312comboWin7-rel9 -tag KIRIN_REL_5_100_9_*
4312comboWin7 clone 4312comboWin7-rel10 -tag KIRIN_REL_5_100_57_*
4312comboWin7 clone 4312comboWin7-rel11 -tag KIRIN_REL_5_100_82_*
4312comboWin7 clone 4312comboWin7-rel12 -tag KIRIN_REL_5_100_98_*
4312comboWin7 clone 4312comboWin7-rel13 -tag KIRIN_REL_5_100_198_*
4312comboWin7 clone 4312comboWin7-bu -tag  "BU4360_{BRANCH,REL}_6_25{,_*}"
4312comboWin7 clone 4312comboWin7-aa -tag  "AARDVARK_BRANCH_6_30"

4312comboWin7 clone 4312comboWin7ext  -brand win_external_wl -type Bcm -tag NIGHTLY
4312comboWin7 clone 4312comboWin7ext-krn  -brand win_external_wl -type Bcm -tag KIRIN_BRANCH_5_100
4312comboWin7 clone 4312comboWin7ext-rub  -brand win_external_wl -type Bcm -tag RUBY_BRANCH_6_20
4312comboWin7 clone 4312comboWin7ext-rel14  -brand win_external_wl -type Bcm -tag RUBY_REL_6_20_??
4312comboWin7 clone 4312comboWin7ext-rel8  -brand win_external_wl -type Bcm -tag KIRIN_REL_5_100_???

# Traffic generators (no wireless cards)
UTF::Linux sr1end18 \
    -sta "lan eth1"

UTF::Router pb5btap1 \
    -lan_ip 192.168.1.1\
    -sta {4717 eth1}\
    -brand linux26-internal-router\
    -power "$power_ctl 8"\
    -power_button "auto"\
    -relay "sr1end18"\
    -console "sr1end18:40005" \
    -lanpeer lan\
	-trx "linux-gzip" \
    -nvram {
	fw_disable=1
	wl_msglevel=0x101
	wl0_ssid=PB5B_AP1_1
	wl0_channel=1
	wl0_obss_coex=0 
	}

4717 clone 4717T -tag NIGHTLY
4717 clone 4717Tx -tag NIGHTLY -brand linux26-external-vista-router-full-src
4717 clone 4717A -tag AKASHI_BRANCH_5_110
4717 clone 4717r -tag AKASHI_REL_5_110_16 -trx "linux"
4717 clone 4717B -tag "BU4360_{BRANCH,REL}_6_25{,_*}"
#4717 clone 4717AA -tag "AARDVARK_{BRANCH,REL}_6_30{,_*}"
4717 clone 4717AA -tag AARDVARK_BRANCH_6_30
4717 clone 4717AAx -tag  AARDVARK_BRANCH_6_30 -brand linux26-external-vista-router-full-src

#4717T configure -postboot {
#    rmmod wl
#    insmod /lib/modules/2.6.22/kernel/drivers/net/wl/wl.ko  passivemode=0
#}
