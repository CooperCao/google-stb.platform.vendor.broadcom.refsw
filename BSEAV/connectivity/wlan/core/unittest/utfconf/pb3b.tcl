

# -*-tcl-*-

#
# pb3b Configuration
#
set ::UTF::SummaryDir "/projects/hnd_sig_ext12/$::env(LOGNAME)/pb3b"

set ::UTF::_llogfile ""

set UTF::DataRatePlot 1
set UTF::AggressiveCleaninfs 1

# Endpoints
UTF::Linux sr1end24 -sta {lan eth1} -onall false

# STAs
# 4312
UTF::Cygwin pb3btst1 -lan_ip pb3btst1 \
   -sta "4312Win7" -user user \
   -installer inf -debuginf 1 \
   -power {pb3bips1 1} \
   -osver 7 \
   -noafterburner 1

4312Win7 clone 4312Win7-bass -tag BASS_BRANCH_5_60*
4312Win7 clone 4312Win7-krn -tag KIRIN_BRANCH_5_100
4312Win7 clone 4312Win7-rub -tag RUBY_BRANCH_6_20
4312Win7 clone 4312Win7-rel14 -tag RUBY_REL_6_20_??
4312Win7 clone 4312Win7-rel5 -tag BASS_REL_5_60_48_*
4312Win7 clone 4312Win7-rel7 -tag BASS_REL_5_60_???
4312Win7 clone 4312Win7-rel8 -tag KIRIN_REL_5_100_???
4312Win7 clone 4312Win7-rel9 -tag KIRIN_REL_5_100_9_*
4312Win7 clone 4312Win7-rel10 -tag KIRIN_REL_5_100_57_*
4312Win7 clone 4312Win7-rel11 -tag KIRIN_REL_5_100_82_*
4312Win7 clone 4312Win7-rel12 -tag KIRIN_REL_5_100_98_*
4312Win7 clone 4312Win7-rel13 -tag KIRIN_REL_5_100_198_*
4312Win7 clone 4312Win7-bu -tag "BU4360_{BRANCH,REL}_6_25{,_*}"
4312Win7 clone 4312Win7-aa -tag  "AARDVARK_{BRANCH,REL}_6_30{,_??}"

4312Win7 clone 4312Win7ext  -brand win_external_wl -type Bcm  -tag NIGHTLY
4312Win7 clone 4312Win7ext-krn  -brand win_external_wl -type Bcm -tag KIRIN_BRANCH_5_100
4312Win7 clone 4312Win7ext-rub  -brand win_external_wl -type Bcm -tag RUBY_BRANCH_6_20
4312Win7 clone 4312Win7ext-rel14  -brand win_external_wl -type Bcm -tag RUBY_REL_6_20_??
4312Win7 clone 4312Win7ext-rel8  -brand win_external_wl -type Bcm -tag KIRIN_REL_5_100_???

# 4312
UTF::Linux pb3btst2 -lan_ip pb3btst2 \
   -sta "4312sdFC4 eth1" -user root \
   -type debug-{,native-}apdef-stadef-sdstd \
   -power {pb3bips1 2} \
   -power_button {auto} \
   -console "sr1end24:40007" \
   -noafterburner 1

4312sdFC4 clone 4312sdFC4-bass -tag BASS_BRANCH_5_60*
4312sdFC4 clone 4312sdFC4-krn -tag KIRIN_BRANCH_5_100
4312sdFC4 clone 4312sdFC4-rel5 -tag BASS_REL_5_60_48_*
4312sdFC4 clone 4312sdFC4-rel7 -tag BASS_REL_5_60_???
4312sdFC4 clone 4312sdFC4-rel8 -tag KIRIN_REL_5_100_???
4312sdFC4 clone 4312sdFC4-rel9 -tag KIRIN_REL_5_100_9_*
4312sdFC4 clone 4312sdFC4-rel10 -tag KIRIN_REL_5_100_57_*
4312sdFC4 clone 4312sdFC4-rel11 -tag KIRIN_REL_5_100_82_*
4312sdFC4 clone 4312sdFC4-rel12 -tag KIRIN_REL_5_100_98_*
4312sdFC4 clone 4312sdFC4-rel13 -tag KIRIN_REL_5_100_198_*

# 43224XP
UTF::Cygwin pb3btst3 -lan_ip pb3btst3 \
   -sta {43224XP} -user user \
   -installer inf -debuginf 1 \
   -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl dump ampdu}} \
   -pre_perf_hook {{%S wl ampdu_clear_dump}} \
   -power {pb3bips1 3} -power_button {auto}

43224XP clone 43224XP-bass -tag BASS_BRANCH_5_60*
43224XP clone 43224XP-krn -tag  KIRIN_BRANCH_5_100
43224XP clone 43224XP-rub -tag  RUBY_BRANCH_6_20
43224XP clone 43224XP-rel14 -tag RUBY_REL_6_20_??
43224XP clone 43224XP-rel5 -tag BASS_REL_5_60_48_*
43224XP clone 43224XP-rel7 -tag BASS_REL_5_60_???
43224XP clone 43224XP-rel8 -tag  KIRIN_REL_5_100_???
43224XP clone 43224XP-rel9 -tag  KIRIN_REL_5_100_9_*
43224XP clone 43224XP-rel10 -tag  KIRIN_REL_5_100_57_*
43224XP clone 43224XP-rel11 -tag  KIRIN_REL_5_100_82_*
43224XP clone 43224XP-rel12 -tag  KIRIN_REL_5_100_98_*
43224XP clone 43224XP-rel13 -tag  KIRIN_REL_5_100_198_*
43224XP clone 43224XP-bu -tag "BU4360_{BRANCH,REL}_6_25{,_*}"
43224XP clone 43224XP-aa -tag  "AARDVARK_{BRANCH,REL}_6_30{,_??}"

43224XP clone 43224XPext  -brand win_external_wl -type Bcm -tag NIGHTLY
43224XP clone 43224XPext-krn  -brand win_external_wl -type Bcm -tag KIRIN_BRANCH_5_100
43224XP clone 43224XPext-rub  -brand win_external_wl -type Bcm -tag RUBY_BRANCH_6_20
43224XP clone 43224XPext-rel14  -brand win_external_wl -type Bcm -tag RUBY_REL_6_20_??
43224XP clone 43224XPext-rel8  -brand win_external_wl -type Bcm -tag KIRIN_REL_5_100_???

# 43225
UTF::Cygwin pb3btst4 -lan_ip pb3btst4 \
   -sta {43225XP} -user user \
   -installer inf -debuginf 1 \
   -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl dump ampdu}} \
   -pre_perf_hook {{%S wl ampdu_clear_dump}} \
   -power {pb3bips1 4}

43225XP clone 43225XP-bass -tag BASS_BRANCH_5_60*
43225XP clone 43225XP-krn -tag  KIRIN_BRANCH_5_100
43225XP clone 43225XP-rub -tag RUBY_BRANCH_6_20
43225XP clone 43225XP-rel14 -tag RUBY_REL_6_20_??
43225XP clone 43225XP-rel5 -tag BASS_REL_5_60_48_*
43225XP clone 43225XP-rel7 -tag BASS_REL_5_60_???
43225XP clone 43225XP-rel8 -tag  KIRIN_REL_5_100_???
43225XP clone 43225XP-rel9 -tag  KIRIN_REL_5_100_9_*
43225XP clone 43225XP-rel10 -tag  KIRIN_REL_5_100_57_*
43225XP clone 43225XP-rel11 -tag  KIRIN_REL_5_100_82_*
43225XP clone 43225XP-rel12 -tag  KIRIN_REL_5_100_98_*
43225XP clone 43225XP-rel13 -tag  KIRIN_REL_5_100_198_*
43225XP clone 43225XP-bu -tag "BU4360_{BRANCH,REL}_6_25{,_*}"
43225XP clone 43225XP-aa -tag  "AARDVARK_{BRANCH,REL}_6_30{,_??}"

43225XP clone 43225XPext  -brand win_external_wl -type Bcm -tag NIGHTLY
43225XP clone 43225XPext-krn  -brand win_external_wl -type Bcm -tag KIRIN_BRANCH_5_100
43225XP clone 43225XPext-rub  -brand win_external_wl -type Bcm -tag RUBY_BRANCH_6_20
43225XP clone 43225XPext-rel14  -brand win_external_wl -type Bcm -tag RUBY_REL_6_20_??
43225XP clone 43225XPext-rel8  -brand win_external_wl -type Bcm -tag KIRIN_REL_5_100_???

# WRT320N/4717
UTF::Router pb3btap2 \
    -sta {4717 eth1} \
    -lan_ip 192.168.1.1 \
    -relay "sr1end24" \
    -power {pb3bips1 8} \
    -lanpeer lan \
    -console "sr1end24:40008" \
    -brand linux26-internal-router \
	-trx "linux-gzip" \
	-nvram {
      fw_disable=1
      wl_msglevel=0x101
      wl0_ssid=PB3B_AP1_0
      wl0_channel=1
	  wl0_obss_coex=0
 }

4717 clone 4717T -tag NIGHTLY
4717 clone 4717Tx -tag NIGHTLY -brand linux26-external-vista-router-full-src
4717 clone 4717A -tag AKASHI_BRANCH_5_110
4717 clone 4717r -tag AKASHI_REL_5_110_16 -trx "linux"
4717 clone 4717B -tag  "BU4360_{BRANCH,REL}_6_25{,_*}"
#4717 clone 4717AA -tag  "AARDVARK_{BRANCH,REL}_6_30{,_??}"
4717 clone 4717AA -tag  AARDVARK_BRANCH_6_30
4717 clone 4717AAx -tag  AARDVARK_BRANCH_6_30 -brand linux26-external-vista-router-full-src 

#4717T configure -postboot {
#    rmmod wl
#    insmod /lib/modules/2.6.22/kernel/drivers/net/wl/wl.ko  passivemode=0
#}

# pb3btst6 Lava Box port 1:40008 port 2:40007 pb3bips1 6
