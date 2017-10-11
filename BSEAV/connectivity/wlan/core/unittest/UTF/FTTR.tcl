#!/bin/env utf
#
# Objects and routines to support traffic sweep testing
# Written mostly to support advanced UTF::streams tests
#
# Author: Robert McMahon
# Date: 1/21/2013
#
# $Id$
# $Copyright Broadcom Corporation$
#
package require UTF
package require snit
package require md5
package require UTF::Streams
package require UTF::math
package require math
package require math::statistics
package require UTF::wlstats

package provide UTF::FTTR 2.0

#
# FTTR object
#
snit::type UTF::FTTR {
    typevariable GNUPLOT_COMMAND "/usr/local/bin/gnuplot"
    typevariable JSDIR "http://www.sj.broadcom.com/projects/hnd_sig_ext4/rmcmahon/gnuplotfiles"
    typevariable current_wlinitcmds -array {}
    typevariable _legendlocation {}
    typevariable _thumbsize "64,32"

    option -ap -default ""
    option -sta -default ""
    option -protocol "tcp"
    option -udprate "1G"
    option -tcprate "-1"
    option -pktsize "1460"
    option -stream -default ""
    option -attnfunc ""
    option -steps -default "" -configuremethod __configuresteps
    option -stepsize -default 1
    option -holdtime -default 2
    option -attngrp ""
    option -stats -default "rate"
    option -tos -default "0x0"
    option -utfmsgtag -default ""
    option -linetype -default "1"
    option -direction -default "UP"
    option -fttrcache -default {} -readonly true
    option -graphcache -default {} -readonly true
    option -graphsize -default "1024,768"
    option -title -default ""
    option -key -default ""
    option -ramp -default "down/up"
    option -startattn -default ""
    option -forceconfig -type boolean -default 0
    option -debug -type boolean -default 0
    option -settle -default 2
    option -reconnect -type boolean -default 1
    option -security -default "open"
    option -chanspec -default ""
    option -tracelevel -default "external"
    option -name -default ""
    option -phycaltype -type integer -default "-1"
    option -tcptune -type boolean -default 1
    option -w -default 1M
    option -wlstats -default ""
    option -multicast -type boolean -default 0
    option -numsamples -type integer -default 20
    option -pktqstats -type boolean -default 0
    option -tcptrace -type boolean -default 0
    option -country -default -1
    option -txbf -default -1
    option -reportinterval -default ""
    option -scansuppress -type boolean -default 0
    option -disableserial -type boolean -default 0
    option -ratedump -type boolean -default 1

    variable _attnfunc {}
    variable _discretes -array {}
    variable _continuous -array {}
    variable _stats -array {}
    variable _threshnorxbeacon {}
    variable _utflogstart {}
    variable _utflogend {}
    variable fattn -array {}
    variable fexpected -array {}
    variable wlcounts -array {}
    variable samples
    variable pktqstats -array {}
    variable farthestrssi
    variable closestrssi

    typemethod hformat {value} {
	set tmp [format %0.0f $value]
	if {$tmp eq "0" || [expr {abs($value) < 1000}]} {
	    return $value
	} elseif {[string index $value 0] eq "-"} {
	    set sign {-}
	    set value [string range $tmp 1 end]
	} else {
	    set sign { }
	    set value $tmp
	}
	set numchars [string length $value]
	switch -exact $numchars {
	    "4" { return "${sign}[format %0.2f [expr {1.0 * $value / 1000}]]K"}
	    "5" { return "${sign}[format %0.1f [expr {1.0 * $value / 1000}]]K"}
	    "6" { return "${sign}[format %0.0f [expr {1.0 * $value / 1000}]]K"}
	    "7" { return "${sign}[format %0.2f [expr {1.0 * $value / 1000000}]]M"}
	    "8" { return "${sign}[format %0.1f [expr {1.0 * $value / 1000000}]]M"}
	    "9" { return "${sign}[format %0.0f [expr {1.0 * $value / 1000000}]]M"}
	    "10" { return "${sign}[format %0.2f [expr {1.0 * $value / 1000000000}]]G"}
	    "11" { return "${sign}[format %0.1f [expr {1.0 * $value / 1000000000}]]G"}
	    "12" { return "${sign}[format %0.0f [expr {1.0 * $value / 1000000000}]]G"}
	    "13" { return "${sign}[format %0.2f [expr {1.0 * $value / 1000000000000}]]T"}
	    "14" { return "${sign}[format %0.1f [expr {1.0 * $value / 1000000000000}]]T"}
	    "15" { return "${sign}[format %0.0f [expr {1.0 * $value / 1000000000000}]]T"}
	    "16" { return "${sign}[format %0.2f [expr {1.0 * $value / 1000000000000000}]]P"}
	    "17" { return "${sign}[format %0.1f [expr {1.0 * $value / 1000000000000000}]]P"}
	    "18" { return "${sign}[format %0.0f [expr {1.0 * $value / 1000000000000000}]]P"}
	    default {
		return $value
	    }
	}
    }
    typemethod diff {args} {
	UTF::Getopts {
	    {fttrs.arg "" "FTTRs to be diffed"}
	    {title.arg "" "Graph Title"}
	}
	if {$(fttrs) eq ""} {
	    set tmp [UTF::FTTR info instances]
	    set FTTRS(0) [lindex $tmp 1]
	    set FTTRS(1) [lindex $tmp 0]
	    # Verify the passed parameters
	} elseif {[llength $(fttrs)] == 2} {
	    set FTTRS(0) [lindex $(fttrs) 0]
	    set FTTRS(1) [lindex $(fttrs) 1]
	} else {
	    error "expect two fttrs, got $args"
	}
	foreach indice [array names FTTRS] {
	    if {[catch {$FTTRS($indice) info type} snittype] || $snittype ne "::UTF::FTTR"} {
		# See if exists in UTF::Test namespace
		set fqn "::UTF::Test::$FTTRS($indice)"
		if {[catch {$fqn info type} snittype] || $snittype ne "::UTF::FTTR"} {
		    error "$FTTRS($indice) not of type UTF::FTTR"
		} else {
		    set FTTRS($indice) $fqn
		}
	    }
	}
	if {![file executable $GNUPLOT_COMMAND]} {
	    if {[file executable /usr/bin/gnuplot]} {
		set GNUPLOT_COMMAND "/usr/bin/gnuplot"
	    } else {
		UTF::Message ERROR UTF::FTTR "$GNUPLOT_COMMAND not executable"
		error "no gnuplot"
	    }
	}
	if {[info exists ::tcl_interactive] && $::tcl_interactive} {
	    set cache [file join [exec pwd] graphcache]
	} elseif {[info exists ::UTF::Logdir]} {
	    # [FIXME] Shouldn't really be using SummaryDir here,
	    # since it was intended to be just an argument to
	    # WrapSummary, but it is getting used as a global
	    # elsewhere.  Fix as part of Web report reorg.
	    set cache [file join $::UTF::Logdir graphcache]
	}
	foreach STAT [$FTTRS(0) cget -stats] {
	    set outfile [file join $cache ${STAT}_cmp]
	    set ix 0
	    while {[file exists ${outfile}_$ix.png]} {
		incr ix
	    }
	    set outfile ${outfile}_$ix
	    set gpdata "${outfile}.dat"
	    set G [open "$gpdata" w]
	    fconfigure $G -buffering line
	    set xtimes [$FTTRS(0) getdiscretes -time]
	    set ix 0
	    foreach indice {0 1} {
		incr ix
		set y($FTTRS($indice)) [$FTTRS($indice) getdiscretes]
		set numsamples [llength $y($FTTRS($indice))]
		#		UTF::Message DEBUG "" "t:[llength $xtimes] s:$numsamples y:$y($FTTRS($indice))"
		if {[$FTTRS($indice) cget -name] eq ""} {
		    set linetitle($ix) [namespace tail $FTTRS($indice)]
		} else {
		    set linetitle($ix) [$FTTRS($indice) cget -name]
		}
	    }
	    array unset integrals {}
	    for {set ix 0} {$ix < $numsamples} {incr ix} {
		set x0 [lindex $xtimes $ix]
		puts -nonewline $G "$x0 "
		foreach indice {0 1} {
		    set yn [lindex $y($FTTRS($indice)) $ix]
		    lappend integrals($indice) $x0 $yn
		    puts -nonewline $G "$yn "
		}
		puts -nonewline $G "\n"
	    }
	    close $G
	    foreach indice {0 1} {
		set area($indice) [lindex [::math::integrate $integrals($indice)] 0]
	    }
	    set areadiff [format %0.0f [expr {$area(0) - $area(1)}]]
	    set areaperc [format %0.0f [expr {$areadiff / $area(1) * 100}]]
	    set text4htmllink "[UTF::FTTR hformat $areadiff] (${areaperc}%)"
	    # Generate the graph title
	    if {$(title) eq {}} {
		set title "[clock format [clock seconds]] $text4htmllink"
	    } else {
		set title "$(title) $text4htmllink\\n[clock format [clock seconds]]"
	    }
	    set gphelper "${outfile}.gpc"
	    set G [open "$gphelper" w]
	    fconfigure $G -buffering line
	    set out ${outfile}.png
	    puts $G "set output \"${out}\""
	    if {[UTF::GnuplotVersion] > 4.0} {
		puts $G  "set terminal png size 640,480"
	    } else {
		puts $G {set terminal png}
	    }
	    puts $G {set grid}
	    #	    puts $G {set autoscale fix}
	    puts $G "set format y \"%.0s%c\""
	    puts $G {set style fill solid 0.25 noborder}
	    puts $G "set key on $_legendlocation samplen 1 spacing .75"
	    puts $G "set title \"$title\""
	    puts $G "set xlabel \"time (s)\""
	    set lt0 [$FTTRS(0) cget -linetype]
	    set lt1 [$FTTRS(1) cget -linetype]
	    puts $G "plot \"$gpdata\" using 1:2 with lines lt $lt0 lw 3 title \"$linetitle(1)\", \"$gpdata\" using 1:3 with lines lt $lt1 lw 3 title \"$linetitle(2)\", \"$gpdata\" using 1:2:3 with filledcurves above title 'above', \"$gpdata\" using 1:2:3 with filledcurves below title 'below'"
	    puts $G "reset"
	    set out ${outfile}_thumb.png
	    puts $G "set output \"${out}\""
	    if {[UTF::GnuplotVersion] > 4.0} {
		puts $G "set terminal png transparent size 64,32 crop"
	    } else {
		puts $G "set terminal png transparent picsize 64 32"
	    }
	    puts $G {unset xtics; unset ytics; unset key; unset xlabel; unset ylabel; unset border; unset grid; unset yzeroaxis; unset xzeroaxis; unset title; set lmargin 0; set rmargin 0; set tmargin 0; set bmargin 0}
	    puts $G {set style fill solid 0.5}
	    puts $G "plot \"$gpdata\" using 1:2 with lines lt $lt0 lw 3, \"$gpdata\" using 1:3 with lines lt $lt1 lw 3, \"$gpdata\" using 1:2:3 with filledcurves above, \"$gpdata\" using 1:2:3 with filledcurves below"
	    close $G
	    catch {exec $GNUPLOT_COMMAND $gphelper} results
	    UTF::Message PLOT FTTRS "${outfile}.png"
	    return "html:<a href=\"[UTF::URI ${outfile}.png]\"> [UTF::ThumbData ${outfile}_thumb.png] $text4htmllink</a>"
	}
    }
    #
    # Take a group of FTTRs and generate a single expected value function
    # and the std deviation along the samples
    #
    typemethod compute_fexpected {args} {
	UTF::Getopts {
	    {fttrs.arg "" "FTTRs to be used to generate expected values"}
	}
	set FTTRS {}
	foreach FTTR $(fttrs) {
	    if {[catch {$FTTR info type} snittype] || $snittype ne "::UTF::FTTR"} {
		# See if exists in UTF::Test namespace
		set fqn "::UTF::Test::$FTTR"
		if {[catch {$fqn info type} snittype] || $snittype ne "::UTF::FTTR"} {
		    error "$FTTR not of type UTF::FTTR"
		} else {
		    lappend FTTRS $fqn
		}
	    } else {
		lappend FTTRS $FTTR
	    }
	}
	set times [[lindex $FTTRS 0] getdiscretes -time]
	foreach fttr $FTTRS {
	    set values($fttr) [$fttr getdiscretes]
	}
	array unset fexpected *
	for {set ix 0} {$ix < [llength $times]} {incr ix} {
	    set samples {}
	    foreach fttr $FTTRS {
		lappend samples [lindex $values($fttr) $ix]
	    }
	    lappend fexpected(mean) [math::statistics::mean $samples]
	    lappend fexpected(stdev) [math::statistics::stdev $samples]
	}
	set fexpected(times) $times
	foreach fttr $FTTRS {
	    $fttr setfexpected [array get fexpected]
	}
	return [array get fexpected]
    }
    typemethod mise {args} {
	UTF::Getopts {
	    {fttrs.arg "" "FTTRs to be diffed"}
	}
	if {$(fttrs) eq ""} {
	    set tmp [UTF::FTTR info instances]
	    set FTTRS(0) [lindex $tmp 1]
	    set FTTRS(1) [lindex $tmp 0]
	    # Verify the passed parameters
	} elseif {[llength $(fttrs)] == 2} {
	    set FTTRS(0) [lindex $(fttrs) 0]
	    set FTTRS(1) [lindex $(fttrs) 1]
	} else {
	    error "expect two fttrs, got $args"
	}
	foreach indice [array names FTTRS] {
	    if {[catch {$FTTRS($indice) info type} snittype] || $snittype ne "::UTF::FTTR"} {
		# See if exists in UTF::Test namespace
		set fqn "::UTF::Test::$FTTRS($indice)"
		if {[catch {$fqn info type} snittype] || $snittype ne "::UTF::FTTR"} {
		    error "$FTTRS($indice) not of type UTF::FTTR"
		} else {
		    set FTTRS($indice) $fqn
		}
	    }
	}
	foreach indice {0 1} {
	    set y($FTTRS($indice)) [$FTTRS($indice) getdiscretes]
	}
	set mise_err 0
	set errs {}
	foreach y0 $y($FTTRS(0)) y1 $y($FTTRS(1)) {
	    set error [expr {$y0 - $y1}]
	    lappend errs $error
	    set mise_err [expr {$mise_err + pow($error,2)}]
	}
	UTF::Message STATS "" "MISE: $errs"
	UTF::Message STATS "" "MISE=[UTF::FTTR hformat [format %0.0f $mise_err]]"
	return $mise_err
    }
    typemethod self_similar {args} {
	UTF::Getopts {
	    {fttrs.arg "" "FTTRs to be used to generate expected values"}
	}
	UTF::FTTR compute_fexpected -fttrs $(fttrs)
    }
    constructor {args} {
	set vals {}
	$self configurelist $args
	if {$options(-utfmsgtag) eq ""} {
	    set options(-utfmsgtag) [namespace tail $self]
	}
	if {$options(-multicast)} {
	    set options(-protocol) "UDP"
	    set options(-direction) "DOWN"
	}
	if {$options(-stream) eq ""} {
	    if {[string toupper $options(-direction)] eq "DOWN"} {
		set TX $options(-ap)
		set RX $options(-sta)
	    } else {
		set RX $options(-ap)
		set TX $options(-sta)
	    }
	    if {[string toupper $options(-protocol)] eq "TCP"} {
		set options(-stream) [UTF::stream %AUTO% -rx $RX -tx $TX -protocol "tcp" -tcptune $options(-tcptune) -w $options(-w) -name "tcp_traffic" -tos $options(-tos) -reportinterval 0.1 -tcptrace $options(-tcptrace) -rate $options(-tcprate)]
	    }  else {
		set options(-stream) [UTF::stream %AUTO% -rx $RX -tx $TX -protocol "udp" -rate $options(-udprate) -pktsize $options(-pktsize) -tcptune $options(-tcptune) -w $options(-w) -name "test_traffic" -multicast $options(-multicast) -tos $options(-tos) -reportinterval 0.1]
	    }
	    if {[expr {($options(-holdtime) / [$options(-stream) cget -reportinterval]) < $options(-numsamples)}]} {
		$options(-stream) configure -reportinterval 0.025
	    }
	    if {$options(-reportinterval) ne ""} {
		$options(-stream) configure -reportinterval $options(-reportinterval)
	    }
	} else {
	    set TX [$options(-stream) cget -tx]
	    set RX [$options(-stream) cget -rx]
	}
	# Discover the attenuator group to use
	if {$options(-attngrp) eq ""} {
	    if {[llength $RX] == 1} {
		set tmp [$RX cget -attngrp]
		if {$tmp ne ""} {
		    set options(-attngrp) $tmp
		}
	    }
	    if {$options(-attngrp) eq ""} {
		set tmp [$TX cget -attngrp]
		if {$tmp ne ""} {
		    set options(-attngrp) $tmp
		}
	    }
	}
	if {$options(-attngrp) eq ""} {
	    error "no attenuator group discovered"
	}
	if {$options(-pktqstats)} {
	    if {[$RX hostis Router]} {
		set mac [[$RX lan] macaddr]
	    } else {
		set mac [$RX macaddr]
	    }
	    set pktqstats($RX) [UTF::wlstats::pktqstat %AUTO% -key "$RX [$self id]" -wlcmd "$TX wl pktq_stats a:+$mac" -name $TX]
	    set pktqstats(common) [UTF::wlstats::pktqstat %AUTO% -key "common [$self id]" -wlcmd "$TX wl pktq_stats c:+" -name $TX]
	    catch {$TX wl msglevel +time}
	}
	# Setup graphing
	if {$options(-graphcache) eq ""} {
	    if {[info exists ::tcl_interactive] && $::tcl_interactive} {
		set options(-graphcache) [file join [exec pwd] graphcache]
	    } elseif {[info exists ::UTF::Logdir]} {
		set options(-graphcache) [file join $::UTF::Logdir graphcache]
	    } else {
		error "FTTR: Unable to find default for -graphcache.  Please use -graphcache or set UTF::SummaryDir"
	    }
	}
	if {![file exists $options(-graphcache)]} {
	    if {[catch {file mkdir $options(-graphcache)} res]} {
		error "FTTR : unable to make directory $options(-graphcache) $res"
	    }
	} elseif {![file writable $options(-graphcache)]} {
	    error "FTTR : directory $options(-graphcache) not writeable"
	}
	# Setup persistent stats
	if {$options(-fttrcache) eq ""} {
	    if {[info exists ::tcl_interactive] && $::tcl_interactive} {
		set options(-fttrcache) [file join [exec pwd] fttrcache]
	    } elseif {[info exists ::UTF::SummaryDir]} {
		# [FIXME] Shouldn't really be using SummaryDir here,
		# since it was intended to be just an argument to
		# WrapSummary, but it is getting used as a global
		# elsewhere.  Fix as part of Web report reorg.
		set options(-fttrcache) [file join $UTF::SummaryDir fttrcache]
	    } else {
		error "FTTR: Unable to find default for -fttrcache.  Please use -fttrcache or set UTF::SummaryDir"
	    }
	}
	if {![file exists $options(-fttrcache)]} {
	    if {[catch {file mkdir $options(-fttrcache)} res]} {
		error "FTTR : unable to make directory $options(-fttrcache) $res"
	    }
	} elseif {![file writable $options(-fttrcache)]} {
	    error "FTTR : directory $options(-fttrcache) not writeable"
	}
	foreach DUT [concat $options(-ap) $options(-sta)] {
	    set current_wlinitcmds($DUT) ""
	}
	if {$options(-attnfunc) ne ""} {
	    set _attnfunc $options(-attnfunc)
	}
    }
    destructor {
	catch {$options(-stream) destroy}
	if {$options(-multicast)} {
	    catch {$options(-ap) igmp_querier disable}
	}
    }
    #
    # Generare a unique id to identify the instance
    #
    method id {args} {
	UTF::GetKnownopts {
	    {exclude.arg "" "attributes to exclude from the hash"}
	}
	set key $args
	set exceptlist "-utfmsgtag -stream -linetype"
	foreach x $(exclude) {
	    lappend exceptlist -$x
	}
	foreach n [lsort [array names options]] {
	    if {[lsearch -exact $exceptlist $n] == -1} {
		lappend key $n $options($n)
	    }
	}
	lappend key [$options(-stream) id]
	regsub -all {[/<>]} $key "." key
	set hash [::md5::md5 -hex $key]
	UTF::Message INFO $options(-utfmsgtag) "${hash}=$key"
	return $hash
    }
    method __configuresteps {option value} {
	if {$options($option) eq $value} {
	    return
	}
	if {$value ne ""} {
	    set options($option) $value
	    set _threshnorxbeacon $value
	    $self _setattnfunc
	}
    }
    method _setattnfunc {} {
	if {$_threshnorxbeacon eq ""} {
	    $self discover_norxbeacon
	}
	set down {};
	# Add a .5dB step at the peak if the attenuator supports it
	set halfstep 0
	if {![catch {$options(-attngrp) incr 0.5}]} {
	    $options(-attngrp) incr -0.5
	    if {$options(-stepsize) eq 1} {
		set halfstep 1
	    }
	}
	if {$halfstep} {
	    set up [list "-0.5 $options(-holdtime)"]
	} else {
	    set up {}
	}
	for {set ix 1} {$ix <= $_threshnorxbeacon} {incr ix} {
	    lappend down "$options(-stepsize) $options(-holdtime)"
	    lappend up "-$options(-stepsize) $options(-holdtime)"
	}
	if {$halfstep} {
	    lappend down "0.5 $options(-holdtime)"
	}
	switch -exact [string tolower $options(-ramp)] {
	    "up/down" {
		set _attnfunc [concat [list "$_threshnorxbeacon $options(-holdtime)"] $up $down]
		set _legendlocation "top left"
	    }
	    "down" {
		set _attnfunc [concat [list "0 $options(-holdtime)"] $down]
		set _legendlocation "top right"
	    }
	    "up" {
		set _attnfunc [concat [list "$_threshnorxbeacon $options(-holdtime)"] $up]
		set _legendlocation "top left"
	    }
	    default {
		set _attnfunc [concat [list "0 $options(-holdtime)"] $down $up]
		set _legendlocation "top right"
	    }
	}
	set options(-attnfunc) $_attnfunc
    }
    method setfexpected {args} {
	array unset fexpected *
	array set fexpected [lindex $args 0]
	set tmp {}
	foreach x $fexpected(times) y $fexpected(mean) {
	    lappend tmp $x $y
	}
	set fexpected(area) [lindex [::math::integrate $tmp] 0]
	set fexpected(max) [::math::statistics::max $tmp]
	return
    }
    #
    #  RJM - move to test outside of object
    #
    method discover_norxbeacon {args} {
	UTF::Getopts {
	    {speedup "Speed up the process"}
	    {rssi "include the rssi value in the return"}
	}
	if {$_threshnorxbeacon ne {}} {
	    return $_threshnorxbeacon
	}
	if {$options(-startattn) eq ""} {
	    set baseattn [expr {int([lindex [$options(-attngrp) attn?] 0])}]
	} else {
	    set baseattn $options(-startattn)
	}
	$self doconnect
	foreach dut [concat $options(-ap) $options(-sta)] {
	    $dut wl status
	    $dut wl dump rssi
	}
	if {$options(-txbf) ne -1} {
	    $options(-ap) wl txbf $options(-txbf)
	}
	set closestrssi 0
	for {set ix 0} {$ix < 5} {incr ix} {
	    $options(-stream) start
	    UTF::Sleep 0.1
	    $options(-stream) stop
	    $options(-sta) wl dump rssi
	    incr closestrssi [$options(-sta) wl rssi]
	}
	set closestrssi [expr {$closestrssi / 5}]
	$options(-stream) stop
	if {[$self _beaconrate] <= 0} {
	    error "Check RF: No beacons at start"
	}
	set farthestrssi -1
	set err {}
	set max [expr {(103 - $baseattn) / $options(-stepsize)}]
	try {
	    # bi = beacon interval, units is ms
	    set apbi [$options(-ap) wl bi]
	    if {$(speedup)} {
		$options(-ap) wl down
		$options(-ap) wl bi 10
		$options(-ap) wl up
		$options(-sta) wl join [$options(-ap) wl ssid]
	    }
	    for {set ix 0} {$ix <= $max} {incr ix} {
		eval $options(-attngrp) incr $options(-stepsize)
		if {[$self _beaconrate] > 0} {
		    set farthestrssi [$options(-sta) wl rssi]
		    continue
		} else {
		    set _threshnorxbeacon $ix
		    UTF::Message INFO $options(-sta) "Norxbeacon=[expr {int([lindex [$options(-attngrp) attn?] 0])}]"
		    break
		}
	    }
	} finally {
	    eval $options(-attngrp) attn $baseattn
	    if {$(speedup)} {
		$options(-ap) wl down
		$options(-ap) wl bi $apbi
		$options(-ap) wl up
		$options(-sta) wl join [$options(-ap) wl ssid]
	    }
	    $options(-sta) wl status
	    $options(-sta) wl dump rssi
	}
	if {$_threshnorxbeacon eq {}} {
	    error "norxbeacon not discovered"
	}
	if {$(rssi)} {
	    return [list $_threshnorxbeacon $closestrssi $farthestrssi]
	} else {
	    return $_threshnorxbeacon
	}
    }
    method config {args} {
	foreach DUT [concat $options(-ap) $options(-sta)] {
	    set wlinitcmds [string trim [$DUT cget -wlinitcmds]]
	    if {($options(-forceconfig) || $current_wlinitcmds($DUT) ne $wlinitcmds) && $wlinitcmds ne ""}  {
		$DUT rexec $wlinitcmds
		set current_wlinitcmds($DUT) $wlinitcmds
	    }
	}
    }
    method load {args} {
	foreach DUT [concat $options(-ap) $options(-sta)] {
	    $DUT load
	    set current_wlinitcmds($DUT) {}
	}
    }
    method doconnect {args} {
	if {$options(-country) ne -1} {
	    $options(-ap) wl country $options(-country)
	}
	set reconnect_time 10
	# Hi Bob,
	# Following are the details reagarding wl phy_percal
	#	0, disable
	#	1, enable, singlephase cal only,
	#	2, enable mutliphase cal allowed
	#	3, manual (testing mode), blocking all driver initiated
	#          periodical cal, give phy_forcecal the full control
	#-Nitin
	if {$options(-phycaltype) ne "-1"} {
	    $options(-sta) wl -u phy_percal $options(-phycaltype)
	    $options(-ap) wl -u phy_percal $options(-phycaltype)
	}
	set phycalsneeded {}
	foreach dut [concat $options(-ap) $options(-sta)] {
	    # If watchdog is disabled initiate a phycal now
	    if {![catch {$dut wl phy_watchdog} res] && !$res} {
		lappend phycalsneeded $dut
	    }
	}
	foreach dut $phycalsneeded {
	    $dut wl phymsglevel +cal
	    $dut wl -u phy_forcecal 1
	    $dut wl phymsglevel -cal
	}
	# Somewhat arbitrary one second sleep per PHY teams timing suggestion
	UTF::Sleep 1.0
	set err {}
	foreach dut $phycalsneeded {
	    if {![catch {$dut wl phy_activecal} res] && $res} {
		append err "$dut phycal not completing within 1 second"
	    }
	}
	if {$err ne ""} {
	    UTF::Message ERROR $utfmsgtag $err
	}
	$options(-sta) wl scansuppress $options(-scansuppress)
	while {$reconnect_time} {
	    if {[catch {$options(-sta) wl bssid} bssid] || $bssid ne [$options(-ap) macaddr]} {
		UTF::Sleep 1.0
		incr reconnect_time -1
	    } else {
		break
	    }
	}
	if {!$reconnect_time} {
	    if {$options(-chanspec) eq ""} {
		set CHANSPEC [lindex [$options(-ap) wl chanspec] 0]
	    } else {
		set CHANSPEC $options(-chanspec)
	    }
	    set SECURITY $options(-security)
	    if {![$options(-sta) hostis Router]} {
		package require UTF::Test::ConnectAPSTA
		UTF::Test::ConnectAPSTA $options(-ap) $options(-sta) -chanspec $CHANSPEC -security $SECURITY
	    } else {
		if {[catch {$options(-sta) wl psta} pstaflag] || !$pstaflag} {
		    package require UTF::Test::ConfigBridge
		    UTF::Test::ConfigBridge -ap $options(-ap) -br $options(-sta) -chanspec $CHANSPEC -security $SECURITY
		} else {
		    $options(-sta) wl join [$options(-ap) wl ssid]
		    UTF::Sleep 10
		    if {[catch {$options(-sta) wl_escanresults} res]} {
			UTF::Message ERROR $options(-sta) $res
		    }
		}
	    }
	}
    }
    method sample {args} {
	set s UTF::FTTR %AUTO%
	foreach opt [array names option] {
	    $s configure -option $opt $options($opt)
	}
	if {![catch {$s run}] err} {
	    lappend samples $s
	} else {
	    UTF::Message ERROR "" $err
	}
    }
    method run {args} {
	UTF::Getopts {
	    {reconnect "" "Block on reconnect"}
	}
	if {$options(-reconnect) || $(reconnect)} {
	    $self doconnect
	}
	if {![info exists closestrssi]} {
	    set restore [expr {int([lindex [$options(-attngrp) attn?] 0])}]
	    set closestrssi 0
	    $options(-stream) start
	    UTF::Sleep 0.1
	    $options(-stream) stop
	    for {set ix 0} {$ix < 5} {incr ix} {
		$options(-sta) wl dump rssi
		incr closestrssi [$options(-sta) wl rssi]
	    }
	    set closestrssi [expr {$closestrssi / 5}]
	    $options(-attngrp) attn $restore
	}
	array unset _discretes *
	array unset _continuous *
	array unset _stats *
	array unset fattn *b
	if {$_attnfunc eq ""} {
	    $self _setattnfunc
	}
	if {$options(-wlstats) eq "txfbw"} {
	    $options(-ap) wl -u reset_cnts
	    array unset wlcounts *;
	    set wlcounts(prevtxfbw) 0
	    set wlcounts(max) 0
	    lappend wlcounts(txfbw) 0 0
	}
	if {$options(-multicast)} {
	    $options(-ap) igmp_querier enable
	}
	# do aan AP command to force ushd init
	$options(-ap) wl status

	$self dosweep
	$self _integrate
	if {$options(-protocol) eq "tcp" && $options(-tcptrace)} {
	    UTF::Try "PCAPs/tcprace" {
		if {[catch {$options(-stream) tcptrace xfer} err]} {
		    UTF::Message ERROR $options(-utfmsgtag) $err
		}
		return
	    }
	}
	set res [$self plot]
	if {$options(-multicast)} {
	    $options(-ap) igmp_querier disable
	}
	return $res
    }
    method _integrate {args} {
	set msg {}
	foreach STAT $options(-stats) {
	    if {$STAT ne "pktlatency"} {
		set msg [::math::integrate $_continuous($STAT)]
		UTF::Message INFO $options(-utfmsgtag) "${STAT}/integrate cont: $msg"
	    }
	    set msg [::math::integrate $_discretes($STAT)]
	    set _stats(integral) $msg
	    UTF::Message INFO $options(-utfmsgtag) "D: $options(-stream)/${STAT}/integrate: $msg"
	}
	return [lindex $msg 0]
    }
    method dosweep {args} {
	set id [$self id]
	UTF::Message INFO $options(-utfmsgtag) "Do sweep: $_attnfunc"
	if {$options(-startattn) eq ""} {
	    set baseattn [expr {int([lindex [$options(-attngrp) attn?] 0])}]
	} else {
	    set baseattn $options(-startattn)
	}
	set currattn $baseattn
	$options(-sta) wl scansuppress $options(-scansuppress)
	if {$options(-txbf) ne -1} {
	    $options(-ap) wl txbf $options(-txbf)
	}
	if {$options(-disableserial)} {
	    $options(-ap) rexec {echo 3 > /proc/sys/kernel/printk}
	}
	set ssid [$options(-ap) wl ssid]
	$options(-stream) start
	UTF::Sleep $options(-settle) "traffic settle hold"
	if {[catch {$options(-stream) linkcheck -now} err]} {
	    UTF::Message ERROR "" "linkcheck fail: $err"
	}
	if {$options(-txbf) eq 1} {
	    $options(-ap) wl -u dump txbf
	}
	set duts [concat [$options(-stream) cget -tx] [$options(-stream) cget -rx]]
	if {$options(-tracelevel) eq "internal" || $options(-tracelevel) eq "ampdu"} {
	    foreach dut $duts {
		if {![$dut hostis Router]} {
		    $dut wl -u ampdu_clear_dump
		}
	    }
	    if {$options(-tracelevel) eq "internal"} {
		set tx [$options(-stream) cget -tx]
	    }
	}
	if {$options(-pktqstats)} {
	    foreach STAT [array names pktqstats] {
		$pktqstats($STAT) sample
	    }
	}
	# clear all streams stats which will clear any streams
	# running outside this FTTR.  Allows for time synchronized graphs
	UTF::stream allstreams stats -clear
	set _utflogstart [clock format [clock seconds] -format %H:%M:%S]
	set starttime [UTF::stream clock]
	lappend fattn(sample) "0 $baseattn"
	foreach step $_attnfunc {
	    set stepincr [lindex $step 0]
	    set steptime [lindex $step 1]
	    if {$options(-tracelevel) eq "ampdu"} {
		foreach dut [concat $options(-ap) $options(-sta)] {
		    if {![$dut hostis Router]} {
			$dut wl -u ampdu_clear_dump
		    }
		}
		#	    $options(-ap) wl -u reset_cnts
		foreach DUT [concat $options(-ap) $options(-sta)] {
		    foreach cmd [$DUT cget -pre_perf_hook] {
			if {[catch [string map [list %S $DUT] $cmd] ret]} {
			    UTF::Message WARN $DUT $ret
			}
		    }
		}
	    }
	    set prevattn $currattn
	    set currattn [expr {$currattn + $stepincr}]
	    if {$options(-protocol) eq "tcp" && $options(-tcptrace)} {
		$options(-stream) tcptrace start
	    }
	    if {$options(-protocol) eq "tcp"} {
		UTF::Sleep 0.1
	    }
	    lappend fattn(sample) "[expr {[UTF::stream clock] - $starttime}] $prevattn"
	    eval $options(-attngrp) incr $stepincr
	    set t1 [UTF::stream clock]
	    set t1_rel [expr {$t1 - $starttime}]
	    lappend fattn(v) $currattn
	    lappend fattn(step) $t1_rel
	    lappend fattn(sample) "$t1_rel $currattn"
	    UTF::Sleep $steptime "attn step $stepincr value $currattn"
	    set t2 [UTF::stream clock]
	    set t2_rel [expr {$t2 - $starttime}]
	    if {$options(-txbf) eq 1} {
		$options(-ap) wl curpower
	    }
	    if {$options(-ratedump)} {
		if {$options(-direction) eq "DOWN"} {
		    catch {$options(-ap) -u ratedump [$options(-sta) macaddr]}
		} else {
		    catch {$options(-sta) wl -u ratedump [$options(-ap) macaddr]}
		}
	    }
	    if {$options(-tracelevel) eq "ampdu"} {
		$options(-sta) wl -u dump rssi
		if {$options(-wlstats) eq "txfbw"} {
		    set wlcntoutput [$options(-ap) wl -u counters]
		    if {[regexp {txfbw ([0-9]+)} $wlcntoutput - tmp]} {
			set txfbw [expr {$tmp - $wlcounts(prevtxfbw)}]
			lappend wlcounts(txfbw) [expr {$t2 - $starttime}] $txfbw
			set wlcounts(prevtxfbw) $tmp
			if {$txfbw > $wlcounts(max)} {
			    set wlcounts(max) $txfbw
			}
		    }
		}
		#	    $options(-sta) wl -u dump scb
		foreach dut [concat $options(-ap) $options(-sta)] {
		    if {![$dut hostis Router]} {
			$dut wl -u dump ampdu
		    }
		}
		foreach DUT [concat $options(-ap) $options(-sta)] {
		    foreach cmd [$DUT cget -post_perf_hook] {
			if {[catch [string map [list %S $DUT] $cmd] ret]} {
			    UTF::Message WARN $DUT $ret
			}
		    }
		}
		#$options(-stream) incr_dstport
		#$options(-stream) start
		#$options(-stream) linkcheck -now
	    }
	    foreach STAT $options(-stats) {
		set values [$options(-stream) stats -$STAT -include $t1 $t2]
		if {$options(-multicast)} {
		    set values [lindex [lindex $values 0] 1]
		}
		if {[string tolower $STAT] eq "pktlatency"} {
		    set tmp {}
		    foreach value $values {
			lappend tmp [lindex [lindex [split $value /] 0] 0]
		    }
		    set values $tmp
		}
		if {[catch {eval [concat ::math::mean $values]} mean]} {
		    UTF::Message WARN $options(-utfmsgtag) "values=$values"
		    set mean -0
		}
		UTF::Message STAT "[string toupper $STAT]" "attn=$currattn mean=$mean values=$values"
		lappend _discretes($STAT) $t2_rel $mean
		lappend _discretes(attn,$STAT) $currattn
	    }
	    if {$options(-pktqstats)} {
		foreach STAT [array names pktqstats] {
		    $pktqstats($STAT) sample
		}
	    }
	    if {$options(-protocol) eq "tcp" && $options(-tcptrace)} {
		$options(-stream) tcptrace stop
	    }
	}
	set _utflogend [clock format [clock seconds] -format %H:%M:%S]
	$options(-stream) samplers -disable
	$options(-stream) stop
	if {$options(-disableserial)} {
	    $options(-ap) rexec dmesg
	}
	if {$options(-tracelevel) eq "internal"} {
	    foreach dut $duts {
		$dut wl -u dump ampdu
	    }
	    foreach dut $duts {
		$dut wl -u dump txbf
	    }
	}
	if {$options(-txbf) eq 1} {
	    $options(-ap) wl -u dump txbf
	}
	foreach STAT $options(-stats) {
	    set values [$options(-stream) stats -$STAT -timestamps]
	    if {$options(-multicast)} {
		set values [lrange [lindex $values 0] 1 2]
	    }
	    set xvalues [lindex $values 1]
	    set yvalues [lindex $values 0]
	    foreach x "$xvalues" y "$yvalues" {
		lappend _continuous($STAT) [expr {$x - $starttime}] $y
	    }
	}
	eval $options(-attngrp) attn $baseattn
    }
    method write {} {
	if {[array exists _discretes]} {
	    foreach STAT [array names _discretes] {
		set STAT [lindex [split $STAT ,] end]
		set filename [file join $options(-fttrcache) "${STAT}_[$self id]"]
		if {![file exists $filename]} {
		    set fd [open $filename w]
		    foreach {x y} $_discretes($STAT) z "$_discretes(attn,$STAT)" {
			puts $fd "1 $x $y $y $y $z"
		    }
		} else {
		    set fd [open $filename r+]
		    foreach {x y} $_discretes($STAT) z "$_discretes(attn,$STAT)" {
			set bytes [gets $fd line]
			if {$bytes >= 0} {
			    seek $fd [expr {0 - $bytes - 1}] current
			    foreach {cnt mean min max} $line {}
			    if {$y < $max} {
				set max $y
			    }
			    if {$y < $min} {
				set min $y
			    }
			    set mean [expr {(($cnt * $mean) + $y) / ($cnt + 1)}]
			    puts $fd "$cnt $x $mean $min $max $z"
			} else {
			    puts $fd "[expr {$cnt + 1}] $x $y $y $y $z"
			}
		    }
		}
		close $fd
	    }
	}
    }
    method plot {args} {
	UTF::Getopts {
	    {expected "" "Expected"}
	    {continuous "" "No averaging"}
	    {canvas "Use html5 canvas"}
	}
	set baseattn [expr {int([lindex [$options(-attngrp) attn?] 0])}]
	if {[catch {$options(-ap) wl -u chanspec} chanspec]} {
	    UTF::Message ERROR $options(-utfmsgtag) "wl chanspec lookup error $chanspec"
	    set chanspec "unk"
	}
	if {![file executable $GNUPLOT_COMMAND]} {
	    if {[file executable /usr/bin/gnuplot]} {
		set GNUPLOT_COMMAND "/usr/bin/gnuplot"
	    } else {
		UTF::Message ERROR $options(-utfmsgtag) "$GNUPLOT_COMMAND not executable"
		error "no gnuplot"
	    }
	}
	if {$options(-name) eq ""} {
	    set linetitle [$options(-stream) label]
	} else {
	    set linetitle $options(-name)
	}
	if {$(continuous)} {
	    set dataindex 0
	} else {
	    set dataindex 1
	}
	set stats $options(-stats)
	set lt $options(-linetype)
	foreach STAT $options(-stats) {
	    set text4htmllink "[UTF::FTTR hformat [::math::statistics::max $_discretes($STAT)]]/[UTF::FTTR hformat [lindex [::math::integrate $_discretes($STAT)] 0]]"
	    set stattitle "$options(-title) $STAT $text4htmllink"
	    set outfile [file join $options(-graphcache) $STAT]
	    set ix 0
	    if {$(canvas)} {
		set out ${outfile}_canvas.html
		while {[file exists ${outfile}_${ix}_canvas.html]} {
		    incr ix
		}
	    } else {
		while {[file exists ${outfile}_$ix.png]} {
		    incr ix
		}
	    }
	    set outfile ${outfile}_$ix
	    set gpdata "${outfile}.dat"
	    set G [open "$gpdata" w]
	    fconfigure $G -buffering line
	    foreach {x y} $_continuous($STAT) {
		puts $G "$x $y"
	    }
	    puts $G "\n"
	    if {![info exists fexpected(mean)]} {
		foreach {x y} $_discretes($STAT) z "$_discretes(attn,$STAT)" {
		    if {$STAT eq "pktlatency" && $y > 1000} {
			puts $G "$x -0 $z"
		    } else {
			puts $G "$x $y $z"
		    }
		}
	    } else {
		foreach {x y} $_discretes($STAT) z "$_discretes(attn,$STAT)" avg $fexpected(mean) stdev $fexpected(stdev) {
		    puts $G "$x $y $z $avg $stdev"
		}
	    }
	    puts $G "\n"
	    if {$options(-wlstats) eq "txfbw"} {
		foreach {x y} $wlcounts(txfbw) {
		    puts $G "$x $y"
		}
	    }
	    close $G
	    set gphelper "${outfile}.gpc"
	    set G [open "$gphelper" w]
	    fconfigure $G -buffering line
	    if {$(canvas)} {
		set out ${outfile}_canvas.html
		puts $G "set output \"${out}\""
		puts $G "set terminal canvas standalone mousing size 1024,768 jsdir \"$JSDIR\""
	    } else {
		set out ${outfile}.png
		puts $G "set output \"${out}\""
		if {[UTF::GnuplotVersion] > 4.0} {
		    puts $G  "set terminal png size $options(-graphsize)"
		} else {
		    puts $G {set terminal png}
		}
	    }
	    puts $G {set grid x y y2}
	    # puts $G {set autoscale fix}
	    # puts $G {unset key}
	    puts $G "set xlabel \"time (s)\\n$_utflogstart - $_utflogend\""
	    if {$STAT eq "rate"} {
		if {[UTF::stream hexpand $options(-udprate)] > 600000} {
		    puts $G {set yrange [0:1000000000]}
		    puts $G {set ytics (100000000,200000000,300000000,400000000,500000000,600000000,700000000,800000000,900000000,1000000000)}
		    puts $G {set y2range [0:100]}
		} else {
		    puts $G {set yrange [0:600000000]}
		    puts $G {set ytics (100000000,200000000,300000000,400000000,500000000,600000000)}
		    puts $G {set y2range [0:100]}
		}
		for {set kx 0} {$kx <= 60} {incr kx +10} {
		    set offset [expr {$baseattn + $kx}]
		    puts $G "set y2tics add \(\"${offset}/[expr {$closestrssi - $kx}] dB\" $offset\)"
		}
		puts $G "set title \"$stattitle\""
		puts $G "set key on $_legendlocation samplen 1 spacing .75"
		if {[UTF::GnuplotVersion] > 4.4} {
		    puts $G {set key font ',9'}
		}
		puts $G "set format y \"%.0s%c\""
	    }
	    if {![info exists fexpected(mean)]} {
		if {$options(-wlstats) eq "txfbw"} {
		    puts $G {set multiplot}
		    puts $G {set lmargin 9}
		    puts $G {set rmargin 2}
		    puts $G {set size 1, 0.3}
		    puts $G {set origin 0, 0.0}
		    puts $G {set tmargin 0}
		    puts $G {set bmargin}
		    puts $G {set format x}
		    if {$wlcounts(max) < 100} {
			puts $G {set yrange [0:100]}
			puts $G {set ytics 25}
		    } elseif {$wlcounts(max) < 200} {
			puts $G {set yrange [0:200]}
			puts $G {set ytics 50}
		    } elseif {$wlcounts(max) < 500} {
			puts $G {set yrange [0:500]}
			puts $G {set ytics 100}
		    }
		    puts $G "plot \"$gpdata\" index 2 using 1:2 with impulses lt 3 lw 2 title \"TXFBW\""
		    puts $G {set size 1, 0.7}
		    puts $G {set origin 0, 0.3}
		    puts $G {set tmargin}
		    puts $G {set bmargin 0}
		    puts $G {set format x ""}
		    puts $G {unset xlabel}
		}
		if {$options(-wlstats) eq "txfbw"} {
		    puts $G "plot \"$gpdata\" index $dataindex using 1:2 with lines lt $lt lw 3 title \"$linetitle\", \"$gpdata\" index 0 using 1:2 with lines lt 2 lw 0.1 notitle, \"-\" using 1:2 axes x1y2 notitle with lines lt -1"
		} else {
		    puts $G "plot \"$gpdata\" index $dataindex using 1:2 with lines lt $lt lw 3 title \"$linetitle\", \"-\" using 1:2 axes x1y2 notitle with lines lt -1"
		}
	    } elseif {!$(expected)} {
		set areadiff [format %0.0f [expr {[lindex [::math::integrate $_discretes($STAT)] 0] - $fexpected(area)}]]
		set areaperc [format %0.0f [expr {$areadiff / $fexpected(area) * 100}]]
		set text4htmllink "[UTF::FTTR hformat $areadiff] (${areaperc}%)"
		puts $G {set style fill solid 0.25 noborder}
		puts $G "plot \"$gpdata\" using 1:4:2 with filledcurves above title 'above', \"$gpdata\" using 1:4:2 with filledcurves below title 'below', \"$gpdata\" index $dataindex using 1:2 with lines lt 1 lw 3 title \"$linetitle\", \"$gpdata\" index 0 using 1:2 with lines lt 1 lw 0.1 notitle, \"$gpdata\" index $dataindex using 1:4 with lines lt 3 lw 2 title 'expected', \"-\" using 1:2 axes x1y2 notitle with lines lt -1"
	    } else {
		puts $G "plot \"$gpdata\" index $dataindex using 1:4 with lines lt 3 lw 2 title 'expected', \"-\" using 1:2 axes x1y2 notitle with lines lt -1"
	    }
	    foreach s $fattn(sample) {
		puts $G "[lindex $s 0] [lindex $s 1]"
	    }
	    puts $G  {e}
	    puts $G "reset"
	    puts $G {unset multiplot}
	    set out ${outfile}_thumb.png
	    puts $G "set output \"${out}\""
	    if {[UTF::GnuplotVersion] > 4.0} {
		puts $G "set terminal png transparent size $_thumbsize crop"
	    } else {
		puts $G "set terminal png transparent picsize 64 32"
	    }
	    puts $G {unset xtics; unset ytics; unset key; unset xlabel; unset ylabel; unset border; unset grid; unset yzeroaxis; unset xzeroaxis; unset title; set lmargin 0; set rmargin 0; set tmargin 0; set bmargin 0}
	    if {![info exists fexpected(mean)]} {
		puts $G "plot \"$gpdata\" index $dataindex using 1:2 with lines lt $lt lw 3"
	    } elseif {!$(expected)} {
		puts $G {set style fill solid 3.0 noborder}
		puts $G "plot \"$gpdata\" using 1:4:2 with filledcurves above notitle, \"$gpdata\" using 1:4:2 with filledcurves below notitle"
	    } else {
		puts $G "plot \"$gpdata\" index $dataindex using 1:4 with lines lt 3 lw 3"
		set text4htmllink "[UTF::FTTR hformat $fexpected(max)]/[UTF::FTTR hformat $fexpected(area)]"
	    }
	    close $G
	    catch {exec $GNUPLOT_COMMAND $gphelper} results
	    UTF::Message PLOT $options(-utfmsgtag) "${outfile}.png"
	}
	if {[file exists ${outfile}_canvas.html]} {
	    return "html:<a href=\"[UTF::URI ${outfile}_canvas.html]\">[UTF::ThumbData ${outfile}_thumb.png]$text4htmllink</a>"
	} else {
	    return "html:<a href=\"[UTF::URI ${outfile}.png]\">[UTF::ThumbData ${outfile}_thumb.png]$text4htmllink</a>"
	}
    }
    method getdiscretes {args} {
	UTF::Getopts {
	    {stat.arg "rate" "stat to return"}
	    {time ""}
	}
	set STAT $(stat)
	set res {}
	if {!$(time)} {
	    foreach {x y} $_discretes($STAT) {
		lappend res $y
	    }
	} else {
	    return $fattn(step)
	}
	return $res
    }
    method _beaconrate {} {
	set sleeptimes "0.25 0.5 1.0"
	foreach sleeptime $sleeptimes {
	    set output [$options(-sta) wl -silent counters]
	    if {![regexp {rxbeaconmbss\s([0-9]+)} $output - b0]} {
		error "cannot find beacon counter"
	    }
	    set time1 [clock clicks -milliseconds]
	    UTF::Sleep $sleeptime
	    set output [$options(-sta) wl counters]
	    set time2 [clock clicks -milliseconds]
	    if {![regexp {rxbeaconmbss\s([0-9]+)} $output - b1]} {
		error "cannot find beacon counter"
	    }
	    set delta [expr {$time2 - $time1}]
	    set bcnrate [expr {1000/$delta * ($b1 - $b0)}]
	    UTF::Message INFO "$options(-sta)" "BeaconRxRate: ${bcnrate}/s"
	    if {$bcnrate > 0} {
		return $bcnrate
	    }
	}
	return $bcnrate
    }
    method mise {args} {
	set STAT "rate"
	if {![info exists _discretes($STAT)]} {
	    error "need to run FTTR"
	}
	if {![info exists fexpected(mean)]} {
	    error "no fexpected"
	}
	set errs {}
	set mise_err 0
	foreach {x y} $_discretes($STAT) avg $fexpected(mean) {
	    set err [expr {$y - $avg}]
	    lappend errs $err
	    set mise_err [expr {$mise_err + pow($err,2)}]
	}
	UTF::Message STATS $options(-utfmsgtag) "MISE: $errs"
	UTF::Message STATS $options(-utfmsgtag) "MISE=[UTF::FTTR hformat [format %0.0f $mise_err]]"
	return $mise_err
    }
}
