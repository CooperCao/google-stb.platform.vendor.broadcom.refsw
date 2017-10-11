#!/bin/env utf
#
# Object for background ping
#
# Author: Robert McMahon
# Date: September 2014
#
# $Id$
# $Copyright Broadcom Corporation$
#
package require UTF
package require math
package require math::statistics
package require snit
package require md5
package require ip
package require UTF::ControlChart

package provide UTF::bkping 2.0

namespace eval UTF::bkping {
    proc clock {} {
	set secs [::clock seconds]
	set ms [::clock clicks -microseconds]
	set base [expr { $secs * 1000000 }]
	set fract [expr { $ms - $base }]
	if { $fract >= 1000000 } {
	    set diff [expr { $fract / 1000000 }]
	    incr secs $diff
	    incr fract [expr { -1000000 * $diff }]
	}
	return $secs.[format %06d $fract]
    }
}
snit::type UTF::bkping {
    option -tx -default {}
    option -rx -default {}
    option -i -default -1
    option -w -default 2
    option -name -readonly true -default {}
    option -key -readonly true -default {}
    option -debug -type boolean -default 0
    option -silent -type boolean  -default 0
    option -statscache ""
    option -s -default 4
    option -samples -default -1
    option -history -default 30

    variable samples {}
    variable timestamps {}
    variable percentlost
    variable txtotal
    variable rxtotal
    variable mystate
    variable mypid
    variable starttime
    variable cctest "PASS"
    variable ccplot ""
    variable logmessages
    variable prevseqno
    variable seqoutages
    variable lastgood
    variable durations
    variable events
    variable DSTIP
    variable linkcheck
    variable interval
    variable myplot
    variable utfmsgtag

    constructor {args} {
	$self configurelist $args
	if {$options(-name) eq ""} {
	    set utfmsgtag [namespace tail $self]
	    set options(-name) $utfmsgtag
	} else {
	    set utfmsgtag $options(-name)
	}
	if {$options(-statscache) eq ""} {
	    if {[info exists ::tcl_interactive] && $::tcl_interactive} {
		set options(-statscache) [file join [exec pwd] statscache]
	    } elseif {[info exists ::UTF::Logdir]} {
		set options(-statscache) [file join $::UTF::Logdir statscache]
	    } else {
		error "Unable to find default for -statscache.  Please use -statscache or set UTF::SummaryDir"
	    }
	}
	set mypid {}
	set mystate "INIT"
	set events {}
	set txtotal 0
	set rxtotal 0
	set starttime {}
	set linkcheck 0
    }
    destructor {
	$self stop
    }
    method id {args} {
	UTF::GetKnownopts {
	    {exclude.arg "" "attributes to exclude from the hash"}
	}
	set key "$options(-key) $args"\
	set exceptlist "-utfmsgtag -name -silent -statscache"
	foreach x $(exclude) {
	    lappend exceptlist -$x
	}
	foreach n [lsort [array names options]] {
	    if {[lsearch -exact $exceptlist $n] == -1} {
		lappend key $n $options($n)
	    }
	}
	regsub -all {[/<>]} $key "." key
	set hash [::md5::md5 -hex $key]
	UTF::Message INFO $utfmsgtag "${hash}=$key"
	return $hash
    }
    method inspect {} {
	puts $mystate
    }
    method start {args} {
	UTF::Getopts {
	    {clear "erase previous values"}
	}
	if {$options(-tx) eq "" || $options(-rx) eq ""} {
	    error "Both -tx and -rx must be configured"
	}
	if {![$options(-tx) lan hostis Linux] || ![$options(-rx) lan hostis Linux]} {
	    error "Both -tx and -rx must be a linuux host"
	}
	if {[$self status]} {
	    UTF::Message WARN "$utfmsgtag" "Currently running and start invoked, pid=$mypid"
	}
	if {$mystate eq "INIT"} {
	    if {$(clear)} {
		$self clear
	    }
	    catch {unset myplot}
	    set percentlost -1
	    set seqoutages 0
	    set durations {}
	    if {$options(-w) eq "-1"} {
		set cmd "ping"
	    } else {
		set cmd "ping -w $options(-w)"
	    }
	    set logmessages 1
	    set interval 1
	    if {$options(-i) eq "-1"} {
		# maximum possible samples use -A
		if {[string toupper $options(-samples)] eq "MAX"} {
		    set interval "-A"
		    set logmessages 0
		    # undefined duration of ping use 1 second interval
		} elseif {$options(-w) eq "-1"} {
		    set interval "1.0"
		} elseif {$options(-samples) eq "-1"} {
		    # Figure out an optimum ping interval per the number
		    # samples needed for xbar and r-charts, double it
		    # to account for potential outages
		    set interval [format %0.4f [expr {$options(-w) / (2.0 * 20 * $options(-s))}]]
		    if {[expr {$interval < 0.0001}]} {
			set interval "-A"
			set logmessages 0
		    } elseif {[expr {$interval > 1}]} {
			set interval "1.0"
		    }
		}
	    } else {
		set interval "$options(-i)"
	    }
	    if {[string is double $interval]} {
		append cmd " -i $interval"
	    } else {
		append cmd " -A"
	    }
	    set SRCDEV [$options(-tx) lan cget -device]
	    # See if this is a VLAN or a regular ethernet
	    # and bind the source interface accordingly
	    set type [llength [split $SRCDEV .]]
	    switch $type {
		case 0 {
		    error "Device not found for $options(-tx)"
		}
		case 1 {}
		default {
		    set SRCDEV [$options(-tx) lan ipaddr]
		    if {![::ip::IPv4? $SRCDEV]} {
			error "No valid ip address for $options(-tx) : $SRCDEV"
		    }
		}
	    }
	    append cmd " -I $SRCDEV"
	    set DSTIP {}
	    if {[::ip::IPv4? $options(-rx)]} {
		set DSTIP $options(-rx)
	    } else {
		set DSTIP [$options(-rx) lan ipaddr]
		if {![::ip::IPv4? $DSTIP]} {
		    error "No valid ip address for $options(-rx) : $DSTIP"
		}
	    }
	    append cmd " $DSTIP"
	    if {[catch {[$options(-tx) lan] rpopen -noinit "$cmd"} fid]} {
		error "PING start failed"
	    } elseif {!$options(-silent)} {
		UTF::_Message INFO $utfmsgtag "Background ping started and running for $options(-w) seconds, interval=$interval"
		UTF::_Message INFO $utfmsgtag "$options(-tx) $cmd"
	    }
	    set mypid [pid $fid]
	    fconfigure $fid -blocking 1 -buffering line
	    set mystate "START"
	    fileevent $fid readable [mymethod __bkping_handler $fid]
	    return
	}
    }
    method stop {} {
	if {$mypid ne ""} {
	    if {[catch {exec kill -s HUP $mypid} err]} {
		UTF::_Message ERROR $utfmsgtag $err
		$self __killremote
	    }
	    set aid [after 500 [list set [myvar mypid] "TIMEOUT"]]
	    vwait [myvar mypid]
	    if {$mypid eq "TIMEOUT"} {
		$self __killremote
		UTF::Sleep 0 -quiet
		set mypid ""
	    } else {
		after cancel $aid
	    }
	}
    }
    method run {args} {
	UTF::Getopts {
	    {clear "erase previous values"}
	}
	if {$(clear)} {
	    $self stop
	    UTF::Sleep 0 -quiet
	    $self clear
	}
	$self start
	while {[$self status]} {
	    UTF::Sleep 0 quiet
	}
	$self controlchart
    }
    method status {} {
	if {$mypid ne ""} {
	    return 1
	} else {
	    return 0
	}
    }
    method linkcheck {args} {
	UTF::Getopts {
	    {now "Check for traffic at this moment in time"}
	    {strict "Fail check if there is any packet loss"}
	}
	if {$(now)} {
	    set linkcheck 0
	}
	if {$linkcheck} {
	    if {!$options(-silent)} {
		UTF::_Message INFO $utfmsgtag "ping linkcheck passed"
	    }
	    return
	}
	if {![$self status]} {
	    if {$(now)} {
		error "$utfmsgtag not running"
	    }  else {
		error "$utfmsgtag linkcheck fail"
	    }
	}
	if {![string is double $interval]} {
	    set timer 1000
	} else {
	    set timer [expr {round(4000 * $interval)}]
	}
	set aid [after $timer [list set [myvar linkcheck] "TIMEOUT"]]
	vwait [myvar linkcheck]
	if {$linkcheck eq "TIMEOUT"} {
	    error "$utfmsgtag linkcheck fail"
	} else {
	    after cancel $aid
	    if {!$options(-silent)} {
		UTF::_Message INFO $utfmsgtag "ping linkcheck passed"
	    }
	}
	return
    }
    method __bkping_handler {fid} {
	if {[eof $fid]} {
	    fconfigure $fid -blocking 0
	    if {[catch {close $fid} err]} {
		UTF::_Message ERROR $utfmsgtag $err
	    }
	    set mystate "INIT"
	    if {$txtotal} {
		set lost [expr {$txtotal - $rxtotal}]
		set perc [format %0.2f [expr {$lost / $txtotal}]]
		set txt "${txtotal}/${rxtotal}/${lost}/${perc}%"
	    } else {
		set txt "nan/nan/nan/nan%"
	    }
	    UTF::_Message INFO $utfmsgtag "Background ping stopped, tx/rx/lost/perc=${txt}, outages=$seqoutages, durations=$durations (sec)"
	    set mypid ""
	} else {
	    set buf [gets $fid]
	    if {$options(-debug) || $logmessages} {
		UTF::_Message HNDLR $utfmsgtag $buf
	    }
	    if {[regexp {icmp_(seq|req)=([0-9]+) ttl=[0-9]+ time=([0-9]+.[0-9]+) ms} $buf - sr seqno rtt]} {
		set t [UTF::bkping::clock]
		set linkcheck 1
		if {$starttime eq ""} {
		    set starttime [UTF::bkping::clock]
		}
		if {$mystate eq "START"} {
		    # Ignore the first sample's RTT
		    set mystate "SAMPLING"
		    set prevseqno $seqno
		    set lastgood $t
		} else {
		    lappend samples $rtt
		    lappend timestamps $t
		    if {[expr {$seqno - $prevseqno} > 1]} {
			lappend durations [format %0.6f [expr {$t - $lastgood}]]
			incr seqoutages +1
			lappend events [list OFF $lastgood $t]
		    } else {
			set lastgood $t
		    }
		    set prevseqno $seqno
		}
	    } elseif {[regexp {([0-9]+) packets transmitted, ([0-9]+) received, ([0-9]+)% packet loss} $buf - txcnt rxcnt percentlost]} {
		set txtotal [expr {$txtotal + $txcnt}]
		set rxtotal [expr {$rxtotal + $rxcnt}]
	    }
	}
    }
    method __killremote {} {
	$options(-tx) lan rexec -x pkill -fx 'ping .+ $DSTIP'
    }
    method stats {args} {
	if {$args eq "-count"} {
	    return [llength $samples]
	} elseif {$args eq "-clear"} {
	    $self clear
	    return
	}
	set res {}
	foreach t $timestamps v $samples {
	    lappend results [list [format %0.6f [expr {$t - $starttime}]] $v]
	}
	return $results
    }
    method mmm {args} {
	set avg [format %0.6f [::math::statistics::mean $samples]]
	set min [format %0.6f [::math::statistics::min $samples]]
	set max [format %0.6f [::math::statistics::max $samples]]
	return [list $avg $min $max]
    }
    method xbar {args} {
	UTF::Getopts {
	    {s.arg "4" "subsamples"}
	}
	return [::math::statistics::control-xbar $samples $(s)]
    }
    method rchart {args} {
	UTF::Getopts {
	    {s.arg "4" "subsamples"}
	}
	return [::math::statistics::control-Rchart $samples $(s)]
    }
    method clear {} {
	set samples {}
	set timestamps {}
	set starttime {}
	set events {}
	set txtotal 0
	set rxtotal 0
    }
    method write {args} {
	UTF::Getopts {
	    {format.arg  "csv" "output file format"}
	    {clear "erase previous values"}
	}
	if {![file exists $options(-statscache)]} {
	    if {[catch {file mkdir $options(-statscache)} res]} {
		error "Unable to make directory $options(-statscache) $res"
	    }
	} elseif {![file writable $options(-statscache)]} {
	    error "Sdirectory $options(-statscache) not writeable"
	}
	set filename [file join $options(-statscache) [$self id].bkping]
	if {$(clear)} {
	    set rw "w"
	} else {
	    set rw "a+"
	}
	if {[catch {open $filename $rw} fid]} {
	    error $fid
	} else {
	    puts $fid [$self stats]
	    close $fid
	}
    }
    method controlchart {} {
	UTF::ControlChart CC -s $options(-s) -key [$self id] -history $options(-history) -title "Background Ping RTT" -units "ms" -allowzero 1
	set mmm [UTF::MeanMinMax $samples]
	foreach {A B C} $mmm {}
	set A [format %0.3f $A]
	set B [format %0.3f $B]
	set C [format %0.3f $C]
	set mmm [list $A $B $C]
	UTF::_Message STATS $utfmsgtag "$mmm ms"
	set boundsresults [CC addsample $mmm]
	if {[regexp {(HIGH|LOW|WIDE|ZERO)} $boundsresults]} {
	    set cctest $boundsresults
	} else {
	    set cctest "PASS"
	}
	set ccplot [CC plotcontrolchart $boundsresults]
	CC destroy
    }
    method cctest {} {
	$self controlchart
	$self plot
	set res "[$self plot] [string range $ccplot 5 end]"
	if {$cctest ne "PASS"} {
	    error $res
	} else {
	    return $res
	}
    }
    method plot {args} {
	UTF::Getopts {
	    {title.arg "Background Ping RTT (ms)" "Plot title"}
	    {type.arg "png" ""}
	    {graphsize.arg "640,480" ""}
	    {thumbsize.arg "64,32" ""}
	    {htmltxt.arg "RTT"}
	    {smoothing "smooth the graph"}
	    {linetype.arg "1" "gnuplot linetype"}
	}
	if {[info exists myplot]} {
	    return $myplot
	}
	UTF::Message STATS $utfmsgtag "RTT=$samples"
	UTF::Message STATS $utfmsgtag "Start=${starttime}, Timestamps=$timestamps"
	set otype $(type)
	if {[info exists ::tcl_interactive] && $::tcl_interactive} {
	    set graphcache [file join [exec pwd] graphcache]
	} elseif {[info exists ::UTF::Logdir]} {
	    set graphcache [file join $::UTF::Logdir graphcache]
	} else {
	    error "Graph: Unable to find directory for graphcache."
	}
	if {![file exists $graphcache]} {
	    if {[catch {file mkdir $graphcache} res]} {
		error "Graph : unable to make directory $graphcache $res"
	    }
	} elseif {![file writable $graphcache]} {
	    error "Graph : directory $graphcache not writeable"
	}
	set ix 0
	while {1} {
	    set outfile [file join $graphcache "bkping_[$self id]"]
	    if {[file exists ${outfile}_$ix.$otype]} {
		incr ix
	    } else {
		set datafile "${outfile}_$ix.data"
		set out ${outfile}_$ix.$otype
		break
	    }
	}
	set fid [open $datafile w]
	if {![file exist $datafile]} {
	    set msg "Cannot create gnu plot data file"
	    UTF::Message ERROR "" $msg
	    error $msg
	} else {
	    set plottxt "plot \"$datafile\" index 0 using 1:2 with lines lt $(linetype) notitle"
	    if {$(smoothing)} {
		set smoothed [UTF::math::filters::tcl_lowpass -values $samples -cutoff 5]
		foreach t $timestamps s $smoothed {
		    puts $fid "[format %0.6f [expr {$t - $starttime}]] $s"
		}
	    } else {
		set segmentcount 0
		set t0 -1
		foreach t $timestamps s $samples {
		    foreach segment $events {
			foreach {E A B} $segment {}
			if {($t >= $B) && ($t0 == $A)} {
			    puts -nonewline $fid "\n\n"
			    incr segmentcount
			    set plottxt "${plottxt}, \"$datafile\" index $segmentcount using 1:2 with lines lt $(linetype) notitle"
			}
		    }
		    set t0 $t
		    set sampletime [format %06f [expr {$t - $starttime}]]
		    if {$sampletime < 0} {
			set sampletime 0
		    }
		    puts $fid "$sampletime $s"
		}
	    }
	    puts $fid "\n"
	    close $fid
	    set utf_log_start [::clock format [expr {round([lindex $timestamps 0])}] -format %H:%M:%S]
	    set utf_log_end [::clock format [expr {round([lindex $timestamps end])}] -format %H:%M:%S]
	}
	set G  [open ${outfile}_$ix.gphelper w]
	puts $G "set output \"${out}\""
	if {[UTF::GnuplotVersion] > 4.0} {
	    puts $G  "set terminal png size $(graphsize)"
	} else {
	    foreach {x y} [split $(graphsize) ,] {}
	    puts $G "set terminal png picsize ${x} ${y}"
	}
	puts $G {set key top right}
	puts $G {set grid}
	if {$(title) eq {}} {
	    puts $G "set title \"[::clock format [::clock seconds]]\""
	} else {
	    puts $G "set title \"$(title)\\n[::clock format [::clock seconds]]\""
	}
	puts $G "set xlabel \"time (s)\\n$utf_log_start - $utf_log_end\""
	puts $G $plottxt
	puts $G "set output \"${outfile}_${ix}_thumb.png\""
	if {[UTF::GnuplotVersion] > 4.0} {
	    puts $G "set terminal png transparent size $(thumbsize) crop"
	} else {
	    foreach {x y} [split $(thumbsize) ,] {}
	    puts $G "set terminal png transparent picsize ${x} ${y}"
	}
	puts $G {unset xtics; unset ytics; unset key; unset xlabel; unset ylabel; unset border; unset grid; unset yzeroaxis; unset xzeroaxis; unset title; set lmargin 0; set rmargin 0; set tmargin 0; set bmargin 0;}
	puts $G $plottxt
	close $G
	if {[catch {exec $::UTF::Gnuplot ${outfile}_$ix.gphelper} results]} {
	    UTF::Message WARN "" "GNUPLOT catch message: $results"
	}
	UTF::Message INFO "" "Graph done: $out"
	set fd [open "${outfile}_${ix}_thumb.png"]
	fconfigure $fd -translation binary
	set thumbdata [base64::encode -maxlen 0 [read $fd]]
	close $fd
	set myplot "html:<!--\[if IE\]><img src=\"${outfile}_${ix}.${otype}\" alt=\"url\" /><a href=\"${outfile}_${ix}.$otype\">$(htmltxt)</a><!\[endif\]--><!\[if !IE\]><img src=\"data:image/png;base64,$thumbdata\" alt=\"data\" /><a href=\"${outfile}_${ix}.$otype\">$(htmltxt)</a>\<!\[endif\]>"
	return $myplot
    }
    method liveplot {args} {
	UTF::Getopts {
	    {style.arg "lines" "gnuplot style"}
	    {title.arg "Ping RTT" "graph title"}
	}
	if {$(stat) eq "all"} {
	    set stats [array names samples]
	} else {
	    set stats $(stat)
	}
	set gfid [open "|/usr/bin/gnuplot" w+]	puts $gfid($stat) "set title \"$(title)\""
	puts $gfid($stat) {set terminal png}
	puts $gfid($stat) "plot '-' with $(style)"
	foreach y $samples() {
	    puts $gfid($stat) "$y"
	}
	puts $gfid($stat) "e"
	flush $gfid($stat)
    }
}
