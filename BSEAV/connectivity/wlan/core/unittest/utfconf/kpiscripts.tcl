#
# UTFD script start for KPI testing
# Robert J. McMahon (setup next to my cube)
# July 2015
#

package require UTF
package require UTFD


proc destroyall {} {
    foreach m [::UTFD::metascript info instances] {
	catch {$m destroy}
    }
}

proc oneoff {} {
    foreach STA "MacAirX-Gala" {
	set distances {50 35 25}
	set AP "NetG7K"
	set stressor "43602lx2"
	set macstas "MacX-GalaGM MacX-Gala MacAirX-GalaGM MacAirX-Gala"
	set macsecurities {open aespsk2}
	set email {hnd-utf-list rmcmahon}
	set suitescript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPISuite.test -utfconf zpl"
	for {set ix 0} {$ix < 3} {incr ix} {
	    set thisscript [list {*}$suitescript -email $email -sta $STA -ap $AP -distance $distances -security $macsecurities -title "$STA w/$AP" -stressor $stressor]
	    UTFD::metascript %AUTO% -type now -script $thisscript -allowdups 1
	}
    }
}
proc restart_kpi_scripts {} {
    set distances {50 35 25}
    set SECONDAP "4360R"
    set AP "NetG7K"
    set stressor "43602lx2"
    set macstas "MacX-GalaGM MacX-Gala MacAirX-GalaGM MacAirX-Gala"
#    set linuxstas "43602lx1"
    set linuxdhdstas "ios_monarch ios_stowe"
    set macsecurities {open aespsk2}
    set dhdsecurities {open}
    set email {hnd-utf-list rmcmahon}
    set suitescript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPISuite.test -utfconf zpl"
    foreach m [::UTFD::metascript info instances] {
	catch {$m destroy}
    }
    foreach STA $macstas {
	set thisscript [list {*}$suitescript -email $email -sta $STA -ap "$AP $SECONDAP" -title "Roam Latency $STA w/$AP" -roamlatency]
	UTFD::metascript %AUTO% -type background -script $thisscript
	set thisscript [list {*}$suitescript -email $email -sta $STA -ap $AP -distance $distances -security $macsecurities -title "$STA w/$AP" -stressor $stressor]
	UTFD::metascript %AUTO% -type background -script $thisscript
    }
    foreach STA $macstas {
	set thisscript [list {*}$suitescript -email $email -sta $STA -ap $SECONDAP -distance $distances -security $macsecurities -title "$STA w/$SECONDAP" -chanspec 6 -stressor $stressor]
	UTFD::metascript %AUTO% -type background -script $thisscript
    }
    foreach STA $linuxdhdstas {
	set thisscript [list {*}$suitescript  -email $email -sta $STA -ap $AP -distance $distances -security $dhdsecurities -title "$STA w/$AP"]
	UTFD::metascript %AUTO% -type background -script $thisscript
    }
    if {[info exists linuxstas]} {
	foreach STA $linuxstas {
	    set thisscript [list {*}$suitescript  -email $email -sta $STA -ap $AP -distance $distances -security $macsecurities -title "$STA w/$AP" -chanspec random]
	    UTFD::metascript %AUTO% -type background -script $thisscript
	}
    }
    UTF::_Message CTRLR "" "Started [llength [UTFD::metascript info instances]] scripts"
}

proc awdl { } {
    set m [UTFD::metascript %AUTO% -type now -script "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf zpl -email {hnd-utf-list rmcmahon} -title {MCC Gala MacOS KPI Direct AWDL Tests AWDLCh=149} -sta_gc MacX-Gala -sta_go MacAirX-Gala -run_qos -qos_tests '\[P2P:BE:TX:0:60\]\|\[P2P:BE:RX:0:60\]\|\[P2P:TCP:TX:0:60\]\|\[P2P:TCP:RX:0:60\]' -p2p_chan 149 -p2p_bandwidth_BE 100M -awdl -no_wl_dump -nounload"]
    set m [UTFD::metascript %AUTO% -type now -script "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPI_AWDL.test -utfconf zpl  -email {hnd-utf-list rmcmahon} -title {MC Gala MacOS KPI Infra+AWDL Tests InfraCh=6 AWDLCh=149} -ap '4360SoftAP' -apdate '2014.3.27.0' -sta_gc 'MacX-Gala' -sta_go 'MacAirX-Gala' -ap_connect GO -run_qos -qos_tests '\[WLAN:BE:TX:0:60\]\[P2P:BE:TX:0:60\]\|\[WLAN:BE:RX:0:60\]\[P2P:BE:RX:0:60\]\|\[WLAN:TCP:TX:0:60\]\[P2P:TCP:TX:0:60\]\|\[WLAN:TCP:RX:0:60\]\[P2P:TCP:RX:0:60\]' -ap_chan 6 -p2p_chan 149 -wlan_bandwidth_BE 100M -p2p_bandwidth_BE 100M -awdl -no_wl_dump -nounload"]
}



proc firstpacket86412 {args} {
    UTF::Getopts {
	{ap.arg "NetG7K" "AP under test"}
	{sta.arg "MacX-Gala" "Sta"}
	{security.arg "aespsk2" "Security to use"}
	{chanspec.arg "161/80" "Chanspec to use"}
    }
    package require UTF::Test::ConnectAPSTA
    UTF::Test::ConnectAPSTA $(ap) $(sta) -security $(security) -chanspec $(chanspec)
    set m [UTFD::metascript %AUTO% -type now -script "/home/rmcmahon/UTF/svn/unittest/Test/KPI/FirstPacket.test -utfconf zpl -web -sta $(sta) -ap $(ap) -security $(security) -s 1"]
    $m run -next
}
