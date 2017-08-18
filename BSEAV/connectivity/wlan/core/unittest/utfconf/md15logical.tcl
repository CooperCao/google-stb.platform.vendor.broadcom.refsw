set ::UTFD::OSVER_CANDIDATE 16A125a
set ::UTFD::OSVER_REFERENCE 15A284
set ::AP "4360ap1"
set ::SECONDAP "4360ap2"
set ::stressor "::DOME_STRESS"
set ::maxruntime 11.0


# CLONES
MacX-A clone MacX-$::UTFD::OSVER_REFERENCE -name MacX-$::UTFD::OSVER_REFERENCE -tag "BIS715GALA_REL_7_21_94_25" -custom 1 -type Release_10_11
MacX-$::UTFD::OSVER_REFERENCE configure -ipaddr 192.168.1.200 -attngrp L5 


MacX-B clone MacX-$::UTFD::OSVER_CANDIDATE -name MacX-$::UTFD::OSVER_CANDIDATE -tag "BIS715GALA_REL_7_21_139_1" -custom 1 -type Release_10_12
MacX-$::UTFD::OSVER_CANDIDATE configure -ipaddr 192.168.1.201 -attngrp L5 

MacAirX-A clone MacAirX-${::UTFD::OSVER_REFERENCE} -name MacAirX-$::UTFD::OSVER_REFERENCE -tag "BIS715GALA_REL_7_21_94_25" -custom 1 -type Release_10_11
MacAirX-${::UTFD::OSVER_REFERENCE} configure -ipaddr 192.168.1.202 -attngrp L4 


MacAirX-B clone MacAirX-${::UTFD::OSVER_CANDIDATE} -name MacAirX-$::UTFD::OSVER_CANDIDATE -tag "BIS715GALA_REL_7_21_139_1" -custom 1 -type Release_10_12
MacAirX-${::UTFD::OSVER_CANDIDATE} configure -ipaddr 192.168.1.203 -attngrp L4 

set ::UTFD::MACDUTS {}
foreach STA [UTF::STA info instances] {
    if {![$STA hostis MacOS] || $STA eq "::AppleCore" || $STA eq $stressor || ([lsearch $::UTFD::physicalmacstas $STA] ne "-1")} {
	continue
    } else {
	lappend ::UTFD::MACDUTS $STA
    }
}

## Throttling
set throttletag BIS715GALA_TWIG_7_21_171
MacX-B clone MacX-Throttle -name MacBookB -tag $throttletag -custom 1 -type Release_10_12
MacX-Throttle configure -ipaddr 192.168.1.201 -attngrp L5


######## AWDL  MacAirX-A ########
MacAirX-${::UTFD::OSVER_CANDIDATE}  clone MacAirX-${::UTFD::OSVER_CANDIDATE}-WLAN \
		-name "Air-DUT" \
		-sta "MacAirX-${::UTFD::OSVER_CANDIDATE}-WLAN en0 MacAirX-${::UTFD::OSVER_CANDIDATE}-AWDL awdl0"
MacAirX-${::UTFD::OSVER_CANDIDATE}-WLAN configure -ipaddr 192.168.1.237
MacAirX-${::UTFD::OSVER_CANDIDATE}-AWDL configure -ipaddr 192.168.5.237
MacAirX-${::UTFD::OSVER_CANDIDATE} clone MacAirX-${::UTFD::OSVER_CANDIDATE}-WLAN-USER -user user

MacAirX-$::UTFD::OSVER_CANDIDATE-WLAN clone MacAirX-$::UTFD::OSVER_CANDIDATE-ext-WLAN \
	  -sta "MacAirX-${::UTFD::OSVER_CANDIDATE}-ext-WLAN en0 MacAirX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL awdl0" \
	  -brand  "macos-external-wl-10_12" \
	  -app_tag "BIS715GALA_REL_7_21_139_1" \
	  -type Release_10_12

MacAirX-${::UTFD::OSVER_CANDIDATE}-ext-WLAN configure -ipaddr 192.168.1.237
MacAirX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL configure -ipaddr 192.168.5.237





######## AWDL  2 ########
MacAirX-${::UTFD::OSVER_REFERENCE} clone MacAirX-${::UTFD::OSVER_REFERENCE}-WLAN \
		-name "Air-AWDL" \
		-sta "MacAirX-${::UTFD::OSVER_REFERENCE}-WLAN en0 MacAirX-${::UTFD::OSVER_REFERENCE}-AWDL awdl0"
MacAirX-${::UTFD::OSVER_REFERENCE}-WLAN configure -ipaddr 192.168.1.235
MacAirX-${::UTFD::OSVER_REFERENCE}-AWDL configure -ipaddr 192.168.5.235
MacAirX-${::UTFD::OSVER_REFERENCE} clone MacAirX-${::UTFD::OSVER_REFERENCE}-WLAN-USER -user user

MacAirX-$::UTFD::OSVER_REFERENCE-WLAN clone MacAirX-$::UTFD::OSVER_REFERENCE-ext-WLAN \
      -sta "MacAirX-${::UTFD::OSVER_REFERENCE}-ext-WLAN en0 MacAirX-${::UTFD::OSVER_REFERENCE}-ext-AWDL awdl0" \
	  -brand  "macos-external-wl-gala" \
	  -app_tag "BIS715GALA_REL_7_21_94_25" \
	  -type Release_10_11

MacAirX-${::UTFD::OSVER_REFERENCE}-ext-WLAN configure -ipaddr 192.168.1.235
MacAirX-${::UTFD::OSVER_REFERENCE}-ext-AWDL configure -ipaddr 192.168.5.235



MacAirX-$::UTFD::OSVER_REFERENCE-WLAN clone MacAirX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL_Peer \
      -sta "MacAirX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL_Peer en0 MacAirX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL awdl0" \
	  -brand  "macos-external-wl-gala" \
	  -tag "BIS715GALA_REL_7_21_139_1" \
	  -app_tag "BIS715GALA_REL_7_21_139_1" \
	  -type Release_10_12

MacAirX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL_Peer configure -ipaddr 192.168.1.235
MacAirX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL configure -ipaddr 192.168.5.235


######## AWDL  3 ########
MacX-${::UTFD::OSVER_REFERENCE} clone MacX-${::UTFD::OSVER_REFERENCE}-WLAN \
		-name "MacX-AWDL" \
    -sta "MacX-${::UTFD::OSVER_REFERENCE}-WLAN en0 MacX-${::UTFD::OSVER_REFERENCE}-AWDL awdl0"
MacX-${::UTFD::OSVER_REFERENCE}-WLAN configure -ipaddr 192.168.1.234
MacX-${::UTFD::OSVER_REFERENCE}-AWDL configure -ipaddr 192.168.5.234
MacX-${::UTFD::OSVER_REFERENCE} clone MacX-${::UTFD::OSVER_REFERENCE}-WLAN-USER -user user

MacX-$::UTFD::OSVER_REFERENCE-WLAN clone MacX-$::UTFD::OSVER_REFERENCE-ext-WLAN \
		-sta "MacX-${::UTFD::OSVER_REFERENCE}-ext-WLAN en0 MacX-${::UTFD::OSVER_REFERENCE}-ext-AWDL awdl0" \
	  -brand  "macos-external-wl-gala" \
	  -app_tag "BIS715GALA_REL_7_21_94_25" \
	  -type Release_10_11

MacX-${::UTFD::OSVER_REFERENCE}-ext-WLAN configure -ipaddr 192.168.1.234
MacX-${::UTFD::OSVER_REFERENCE}-ext-AWDL configure -ipaddr 192.168.5.234



MacX-$::UTFD::OSVER_REFERENCE-WLAN clone MacX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL_Peer \
		-sta "MacX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL_Peer en0 MacX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL_Peer-AWDL awdl0" \
	  -brand  "macos-external-wl-10_12" \
	  -app_tag "BIS715GALA_REL_7_21_139_1" \
	  -tag "BIS715GALA_REL_7_21_139_1" \
	  -type Release_10_12

MacX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL_Peer configure -ipaddr 192.168.1.234
MacX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL_Peer-AWDL configure -ipaddr 192.168.5.234



######## AWDL  4 ########
MacX-$::UTFD::OSVER_CANDIDATE clone MacX-${::UTFD::OSVER_CANDIDATE}-WLAN \
    -name "MacX-DUT" \
    -sta "MacX-${::UTFD::OSVER_CANDIDATE}-WLAN en0 MacX-${::UTFD::OSVER_CANDIDATE}-AWDL awdl0"
MacX-${::UTFD::OSVER_CANDIDATE}-WLAN configure -ipaddr 192.168.1.236
MacX-${::UTFD::OSVER_CANDIDATE}-AWDL configure -ipaddr 192.168.5.236
MacX-${::UTFD::OSVER_CANDIDATE} clone MacX-${::UTFD::OSVER_CANDIDATE}-WLAN-USER -user user


MacX-$::UTFD::OSVER_CANDIDATE-WLAN clone MacX-$::UTFD::OSVER_CANDIDATE-ext-WLAN \
	  -sta "MacX-${::UTFD::OSVER_CANDIDATE}-ext-WLAN en0 MacX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL awdl0" \
	  -brand  "macos-external-wl-10_12" \
	  -app_tag "BIS715GALA_REL_7_21_139_1" \
	  -type Release_10_12
	  
MacX-${::UTFD::OSVER_CANDIDATE}-ext-WLAN configure -ipaddr 192.168.1.236
MacX-${::UTFD::OSVER_CANDIDATE}-ext-AWDL configure -ipaddr 192.168.5.236

#  METASCRIPTS

proc ::restart_kpi_scripts {args} {
    UTF::Getopts {
	{awdlscripts.arg "1" "Number of loops for AWDL scripts"}
    }
    set distances {70 55 35}
    set thisutfconf [file rootname [file tail $::utfconf]]
    UTF::Message INFO "" "Instantiating metascripts for rig $thisutfconf"
    UTF::Message INFO "" "MacDUTs are $::UTFD::MACDUTS"
    set macsecurities {open aespsk2}
    #    set macsecurities {open}
    set email {hnd-utf-list rmcmahon anujgupt}
    set suitescript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPISuite.test -utfconf $thisutfconf -suiteid meta"
    UTFD::destroyall
    if {$(awdlscripts) > 0} {
	awdlscripts $(awdlscripts)
    }
    foreach STA $::UTFD::MACDUTS {
	set thisscript [list {*}$suitescript -email $email -sta $STA -ap "$::AP $::SECONDAP" -distance $distances -security open -noapload -manualload -title "$STA w/$::AP and $::SECONDAP" -stressor $::stressor -beaconedge 70 ]
	UTFD::metascript %AUTO% -type background  -maxruntime $::maxruntime -script $thisscript
	set thisscript [list {*}$suitescript -email $email -sta $STA -ap "$::AP $::SECONDAP" -distance $distances -security aespsk2 -noapload -manualload -title "$STA w/$::AP and $::SECONDAP" -stressor $::stressor -beaconedge 70 ]
	UTFD::metascript %AUTO% -type background  -maxruntime $::maxruntime -script $thisscript
    }
    foreach STA $::UTFD::MACDUTS {
	set thisscript [list {*}$suitescript -email $email -sta $STA -ap $::AP -distance $distances -security open -noapload -manualload -title "$STA w/$::AP" -chanspec 6 -stressor $::stressor -beaconedge 70 ]
	UTFD::metascript %AUTO% -type background  -maxruntime $::maxruntime -script $thisscript
	set thisscript [list {*}$suitescript -email $email -sta $STA -ap $::AP -distance $distances -security aespsk2 -noapload -manualload -title "$STA w/$::AP" -chanspec 6 -stressor $::stressor -beaconedge 70 ]
	UTFD::metascript %AUTO% -type background  -maxruntime $::maxruntime -script $thisscript
    }
    if {[info exists linuxdhdstas]} {
	foreach STA $linuxdhdstas {
	    set thisscript [list {*}$suitescript  -email $email -sta $STA -ap $::AP -distance $distances -security $dhdsecurities -noapload -manualload -title "$STA w/$::AP"]
	    UTFD::metascript %AUTO% -type background -script $thisscript
	}
    }
    UTF::_Message CTRLR "" "Started [llength [UTFD::metascript info instances]] scripts"
}

proc awdlscripts {{loop 1}} {
    set thisutfconf [file rootname [file tail $::utfconf]]
    set OSVER_CANDIDATE $::UTFD::OSVER_CANDIDATE
    set OSVER_REFERENCE $::UTFD::OSVER_REFERENCE
    set metascripttype "now"
    if {$loop > 1} {
	set allowdups 1
    } else {
	set allowdups 0
    }
    while {$loop} {
	incr loop -1
	
	UTF::Message INFO "" "Instantiating awdl scripts for rig $thisutfconf"
	
	#AWDL Only and AWDL+Infra :
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacX KPI AWDL Only Tests AWDLCh=149} -sta_go MacX-${OSVER_CANDIDATE}-WLAN -sta_gc MacX-${OSVER_REFERENCE}-WLAN -run_qos -qos_tests '\[P2P:BE:RX:0:60\]\|\[P2P:BE:TX:0:60\]\|\[P2P:TCP:RX:0:60\]\|\[P2P:TCP:TX:0:60\]' -p2p_chan 149 -p2p_bandwidth_BE 40M -awdl -no_wl_dump -nounload -kpi_flag "
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacX KPI AWDL+Infra Tests InfraCh=6 AWDLCh=149} -ap $::AP -sta_go MacX-${OSVER_CANDIDATE}-WLAN -sta_gc MacX-${OSVER_REFERENCE}-WLAN -ap_connect GO -run_qos -qos_tests '\[WLAN:BE:RX:0:60\]\[P2P:BE:RX:0:60\]\|\[WLAN:BE:TX:0:60\]\[P2P:BE:TX:0:60\]\|\[WLAN:TCP:RX:0:60\]\[P2P:TCP:RX:0:60\]\|\[WLAN:TCP:TX:0:60\]\[P2P:TCP:TX:0:60\]' -ap_chan 6 -p2p_chan 149 -wlan_bandwidth_BE 40M -p2p_bandwidth_BE 40M -awdl -no_wl_dump -nounload -kpi_flag "
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacAirX KPI AWDL Only Tests AWDLCh=149} -sta_go MacAirX-${OSVER_CANDIDATE}-WLAN -sta_gc MacAirX-${OSVER_REFERENCE}-WLAN -run_qos -qos_tests '\[P2P:BE:RX:0:60\]\|\[P2P:BE:TX:0:60\]\|\[P2P:TCP:RX:0:60\]\|\[P2P:TCP:TX:0:60\]' -p2p_chan 149 -p2p_bandwidth_BE 40M -awdl -no_wl_dump -nounload -kpi_flag "
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacAirX KPI AWDL+Infra Tests InfraCh=6 AWDLCh=149} -ap $::AP -sta_go MacAirX-${OSVER_CANDIDATE}-WLAN -sta_gc MacAirX-${OSVER_REFERENCE}-WLAN -ap_connect GO -run_qos -qos_tests '\[WLAN:BE:RX:0:60\]\[P2P:BE:RX:0:60\]\|\[WLAN:BE:TX:0:60\]\[P2P:BE:TX:0:60\]\|\[WLAN:TCP:RX:0:60\]\[P2P:TCP:RX:0:60\]\|\[WLAN:TCP:TX:0:60\]\[P2P:TCP:TX:0:60\]' -ap_chan 6 -p2p_chan 149 -wlan_bandwidth_BE 40M -p2p_bandwidth_BE 40M -awdl -no_wl_dump -nounload -kpi_flag "
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	
	#KPI2 testbed BT tests:
	#BT:

	
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacX BT Only RX} -run_qos -qos_tests '\[BT:A2DP:RX:0:60\]' -no_wl_dump -kpi_flag -bt_user_sta MacX-${OSVER_REFERENCE}-WLAN-USER -bt_root_sta MacX-${OSVER_REFERENCE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacX BT Only RX} -run_qos -qos_tests '\[BT:A2DP:RX:0:60\]' -no_wl_dump -kpi_flag -bt_user_sta MacX-${OSVER_CANDIDATE}-WLAN-USER -bt_root_sta MacX-${OSVER_CANDIDATE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacAirX BT Only RX} -run_qos -qos_tests '\[BT:A2DP:RX:0:60\]' -no_wl_dump -kpi_flag -bt_user_sta MacAirX-${OSVER_REFERENCE}-WLAN-USER -bt_root_sta MacAirX-${OSVER_REFERENCE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacAirX BT Only RX} -run_qos -qos_tests '\[BT:A2DP:RX:0:60\]' -no_wl_dump -kpi_flag -bt_user_sta MacAirX-${OSVER_CANDIDATE}-WLAN-USER -bt_root_sta MacAirX-${OSVER_CANDIDATE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"

	#BT+AWDL:

	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacX BT+AWDL RX AWDLCh=149/80} -sta_go MacX-${OSVER_CANDIDATE}-WLAN -sta_gc MacX-${OSVER_REFERENCE}-WLAN -run_qos -qos_tests '\[BT:A2DP:RX:0:60\]\[P2P:BE:RX:0:60\]' -p2p_chan 149 -p2p_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacX-${OSVER_REFERENCE}-WLAN-USER -bt_root_sta MacX-${OSVER_REFERENCE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacX BT+AWDL RX AWDLCh=149/80} -sta_go MacX-${OSVER_REFERENCE}-WLAN -sta_gc MacX-${OSVER_CANDIDATE}-WLAN -run_qos -qos_tests '\[BT:A2DP:RX:0:60\]\[P2P:BE:RX:0:60\]' -p2p_chan 149 -p2p_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacX-${OSVER_CANDIDATE}-WLAN-USER -bt_root_sta MacX-${OSVER_CANDIDATE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacAirX BT+AWDL RX AWDLCh=149/80} -sta_go MacAirX-${OSVER_CANDIDATE}-WLAN -sta_gc MacAirX-${OSVER_REFERENCE}-WLAN -run_qos -qos_tests '\[BT:A2DP:RX:0:60\]\[P2P:BE:RX:0:60\]' -p2p_chan 149 -p2p_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacAirX-${OSVER_REFERENCE}-WLAN-USER -bt_root_sta MacAirX-${OSVER_REFERENCE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacAirX BT+AWDL RX AWDLCh=149/80} -sta_go MacAirX-${OSVER_REFERENCE}-WLAN -sta_gc MacAirX-${OSVER_CANDIDATE}-WLAN -run_qos -qos_tests '\[BT:A2DP:RX:0:60\]\[P2P:BE:RX:0:60\]' -p2p_chan 149 -p2p_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacAirX-${OSVER_CANDIDATE}-WLAN-USER -bt_root_sta MacAirX-${OSVER_CANDIDATE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"

	#BT+Infra:

	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacX BT+Infra RX InfraCh=6} -ap 4360ap1 -ap_connect GO -sta MacX-${OSVER_CANDIDATE}-WLAN -run_qos -qos_tests '\[WLAN:BE:RX:0:60\]\[BT:A2DP:RX:0:60\]' -ap_chan 6 -wlan_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacX-${OSVER_REFERENCE}-WLAN-USER -bt_root_sta MacX-${OSVER_REFERENCE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacX BT+Infra RX InfraCh=6} -ap 4360ap1 -ap_connect GO -sta MacX-${OSVER_REFERENCE}-WLAN -run_qos -qos_tests '\[WLAN:BE:RX:0:60\]\[BT:A2DP:RX:0:60\]' -ap_chan 6 -wlan_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacX-${OSVER_CANDIDATE}-WLAN-USER -bt_root_sta MacX-${OSVER_CANDIDATE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacAirX BT+Infra RX InfraCh=6} -ap 4360ap1 -ap_connect GO -sta MacAirX-${OSVER_CANDIDATE}-WLAN -run_qos -qos_tests '\[WLAN:BE:TX:0:60\]\[BT:A2DP:TX:0:60\]' -ap_chan 6 -wlan_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacAirX-${OSVER_REFERENCE}-WLAN-USER -bt_root_sta MacAirX-${OSVER_REFERENCE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacAirX BT+Infra RX InfraCh=6} -ap 4360ap1 -ap_connect GO -sta MacAirX-${OSVER_REFERENCE}-WLAN -run_qos -qos_tests '\[WLAN:BE:TX:0:60\]\[BT:A2DP:TX:0:60\]' -ap_chan 6 -wlan_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacAirX-${OSVER_CANDIDATE}-WLAN-USER -bt_root_sta MacAirX-${OSVER_CANDIDATE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"

	#BT+AWDL+Infra:

	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacX BT+AWDL+Infra RX InfraCh=6 AWDLCh=149/80} -ap 4360ap1 -ap_connect GO -sta_go MacX-${OSVER_CANDIDATE}-WLAN -sta_gc MacX-${OSVER_REFERENCE}-WLAN -run_qos -qos_tests '\[WLAN:BE:RX:0:60\]\[BT:A2DP:RX:0:60\]\[P2P:BE:RX:0:60\]' -ap_chan 6 -wlan_bandwidth_BE 40M -p2p_chan 149 -p2p_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacX-${OSVER_REFERENCE}-WLAN-USER -bt_root_sta MacX-${OSVER_REFERENCE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacX BT+AWDL+Infra RX InfraCh=6 AWDLCh=149/80} -ap 4360ap1 -ap_connect GO -sta_go MacX-${OSVER_REFERENCE}-WLAN -sta_gc MacX-${OSVER_CANDIDATE}-WLAN -run_qos -qos_tests '\[WLAN:BE:RX:0:60\]\[BT:A2DP:RX:0:60\]\[P2P:BE:RX:0:60\]' -ap_chan 6 -wlan_bandwidth_BE 40M -p2p_chan 149 -p2p_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacX-${OSVER_CANDIDATE}-WLAN-USER -bt_root_sta MacX-${OSVER_CANDIDATE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacAirX BT+AWDL+Infra RX InfraCh=6 AWDLCh=149/80} -ap 4360ap1 -ap_connect GO -sta_go MacAirX-${OSVER_CANDIDATE}-WLAN -sta_gc MacAirX-${OSVER_REFERENCE}-WLAN -run_qos -qos_tests '\[WLAN:BE:RX:0:60\]\[BT:A2DP:RX:0:60\]\[P2P:BE:RX:0:60\]' -ap_chan 6 -wlan_bandwidth_BE 40M -p2p_chan 149 -p2p_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacAirX-${OSVER_REFERENCE}-WLAN-USER -bt_root_sta MacAirX-${OSVER_REFERENCE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	set awdlscript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf $thisutfconf -email {hnd-utf-list rmcmahon anujgupt} -noapload -nostaload -nostareload -title {md15 MacAirX BT+AWDL+Infra RX InfraCh=6 AWDLCh=149/80} -ap 4360ap1 -ap_connect GO -sta_go MacAirX-${OSVER_REFERENCE}-WLAN -sta_gc MacAirX-${OSVER_CANDIDATE}-WLAN -run_qos -qos_tests '\[WLAN:BE:RX:0:60\]\[BT:A2DP:RX:0:60\]\[P2P:BE:RX:0:60\]' -ap_chan 6 -wlan_bandwidth_BE 40M -p2p_chan 149 -p2p_bandwidth_BE 40M -awdl -no_wl_dump -kpi_flag -bt_user_sta MacAirX-${OSVER_CANDIDATE}-WLAN-USER -bt_root_sta MacAirX-${OSVER_CANDIDATE}-WLAN"
	UTFD::metascript %AUTO% -allowdups $allowdups -type $metascripttype -script "$awdlscript"
	
	

    }
}

# SNIFFER autostart per try id

#set ::UTFD::snifftest(80890FB48793BD8881D2433BEC42F290 ) "1"

namespace eval UTF::Test {
    variable snifftest -array
    array set tryid {8FED1C30BAEEA462363E24F96F4A4511 E51AB856810D5E4C995FEBDA62713C8D}
    proc AutoSniffStart {tryid} {
	UTF::Message INFO "" "Auto Sniff start called for $tryid"
	foreach tryid {
	set ::UTFD::snifftest($tryid ) "1"
	if {$::UTFD::snifftest($tryid)} {
	    set sniffers [UTF::Sniffer info instances]
	    foreach sniffer $sniffers {
		if {[catch {$sniffer load} err]} {
		    UTF::Message WARN $sniffer $err
		}
	    }
	    foreach sniffer $sniffers {
		if {[regexp {AP} [$sniffer cget -name]]} {
		    set chanspec 6
		} else {
		    set chanspec 149
		}
		if {[catch {$sniffer setupSniffer $chanspec} err]} {
		    UTF::Message WARN $sniffer $err
		}
	    }
	    foreach sniffer $sniffers {
		if {[catch {$sniffer start [list tshark -i radiotap0 -w "/tmp/[namespace tail ${sniffer}].pcap"]} err]} {
		    UTF::Message WARN $sniffer $err
		}
	    }
	}
	}
    }
    proc AutoSniffStop {tryid} {
	UTF::Message INFO "" "Auto Sniff stop called for $tryid"
	if {$::UTFD::snifftest($tryid)} {
	    set sniffers [UTF::Sniffer info instances]
	    foreach sniffer $sniffers {
		if {[catch {$sniffer stopall} err]} {
		    UTF::Message WARN $sniffer $err
		}
	    }
	    set sniffers [UTF::Sniffer info instances]
	    foreach sniffer $sniffers {
		if {[catch {$sniffer unload} err]} {
		    UTF::Message WARN $sniffer $err
		}
	    }
	    foreach sniffer $sniffers {
		set TO [file join $::UTF::Logdir [namespace tail ${sniffer}]_${tryid}]
		set ix 0;
		while {[file exists ${TO}_${ix}.pcap]} {
		    incr ix
		}
		set TO "${TO}_${ix}.pcap"
		if {[catch {eval [concat $sniffer copyfrom "/tmp/[namespace tail ${sniffer}].pcap" $TO]} err]} {
		    UTF::Message ERROR $sniffer $err
		}
		if {[catch {eval [concat $sniffer rexec rm -f "/tmp/[namespace tail ${sniffer}].pcap"]} err]} {
		    UTF::Message ERROR :q$sniffer $err
		}
		if {[catch {eval [concat exec chmod a+r $TO]} err]} {
		    UTF::Message ERROR $sniffer $err
		}
	    }
	}
    }
}


