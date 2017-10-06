#!/bin/env utf
#
# Routines to make objects out of complex wl statistics
#
# Author: Robert McMahon
# Date: September 2013
#
# $Id$
# $Copyright Broadcom Corporation$
#
package require UTF
package require math
package require snit
package require md5

package provide UTF::wlstats 2.0

# Example output
# AMPDU queue b8:ca:3a:d3:a1:f9
# prec:   rqstd,  stored, dropped, retried, rtsfail,rtrydrop, psretry,   acked,utlisatn,q length,Data Mbits/s,Phy Mbits/s (v4)
#   00:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1536,     0.00,      0.00
#   01:       0,       0,       0,       0,       0,       0,       0,       0,       0,    4096,     0.00,      0.00
#   02:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1536,     0.00,      0.00
#   03:       0,       0,       0,       0,       0,       0,       0,       0,       0,    4096,     0.00,      0.00
#   04:   38531,   38531,       0,     175,     187,       0,       0,   38502,      31,    1536,     0.08,    526.28
#   05:       0,       0,       0,       0,       0,       0,       0,       0,       0,    4096,     0.00,      0.00
#   06:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1536,     0.00,      0.00
#   07:       0,       0,       0,       0,       0,       0,       0,       0,       0,    4096,     0.00,      0.00
#    08:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1536,     0.00,      0.00
#    09:       0,       0,       0,       0,       0,       0,       0,       0,       0,    4096,     0.00,      0.00
#    10:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1536,     0.00,      0.00
#    11:       0,       0,       0,       0,       0,       0,       0,       0,       0,    4096,     0.00,      0.00
#    12:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1536,     0.00,      0.00
#    13:       0,       0,       0,       0,       0,       0,       0,       0,       0,    4096,     0.00,      0.00
#    14:       0,       0,       0,       0,       0,       0,       0,       0,       0,    1536,     0.00,      0.00
#    15:       0,       0,       0,       0,       0,       0,       0,       0,       0,    4096,     0.00,      0.00
snit::type UTF::wlstats::pktqstat {
    typevariable PREVIOUSSELF -array {}
    typevariable WARNRATELIMITER {}
    typevariable INTERESTINGINDEXCACHE -array {}
    typemethod clock {} {
	set secs [clock seconds]
	set ms [clock clicks -milliseconds]
	set base [expr { $secs * 1000 }]
	set fract [expr { $ms - $base }]
	if { $fract >= 1000 } {
	    set diff [expr { $fract / 1000 }]
	    incr secs $diff
	    incr fract [expr { -1000 * $diff }]
	}
	return $secs.[format %03d $fract]
    }
    typemethod plot {args} {
	UTF::Getopts {
	    {all "plot all stats, default is non-zero"}
	}
	set all [$type info instances]
    }
    typemethod allstats {args} {
	set instances [$type info instances]
	foreach instance $instances {
	    eval [concat $instance $args]
	}
	return
    }
    typemethod derivative {args} {
	UTF::Getopts {
	    {stats.arg "" "stats to use for the derivative"}
	    {log "output log message if derivative is non-zero"}
	    {type.arg "first" "first or second derivative"}
	}
	if {$(stats) eq {}} {
	    set (stats) [$type info instances]
	}
	set stat0 [lindex $(stats) 0]
	set stat1 [lindex $(stats) 1]
	if {[$stat0 id] ne [$stat0 id]} {
	    UTF::Message ERROR pktqstat "stat id mismatch $stat0 $stat1"
	}
	# RJM FIX THIS
	set q [$stat0 get -queue]
	array set s0 [$stat0 get]
	array set s1 [$stat1 get]
	set t0 [$stat0 get -timestamp]
	set t1 [$stat1 get -timestamp]
	foreach index [array names s0] {
	    if {$s0($index) ne $s1($index)} {
		UTF::Message LOG pktqstat "DELTA [expr $s0($index) - $s1($index)] $q\($index\) ($s0($index) $s1($index) [format %0.3f [expr {$t0 - $t1}]] s)"
	    }
	}
    }
    option -wloutput -default {}
    option -wlcmd -readonly true -default {}
    option -ap -default {}
    option -sta -default {}
    option -tx -default {}
    option -rx -default {}
    option -name -readonly true -default {}
    option -statscache -default {} -readonly true
    option -key -readonly true -default {}
    option -common -type boolean -default false
    option -verbose -type boolean -default false

    variable rows  -array {}
    variable cols  -array {}
    variable values -array {}
    variable derivatives -array {}
    variable queue {}
    variable utftimestamp {}
    variable rtrtimestamp {}
    variable utf_msgtag
    variable index2rlabel -array {}
    variable index2clabel -array {}
    variable outfile {}
    variable samples {}
    variable myid
    variable tsfwrapcount 0
    variable previoustsf 4294.967296
    variable firsttimestamp

    constructor {args} {
	$self configurelist $args
	set utftimestamp [$type clock]
	if {$options(-name) eq ""} {
	    if {$options(-ap) eq {}} {
		set utf_msgtag "[namespace tail $self]"
		set options(-name) "[namespace tail $self]"
	    } else {
		set options(-name) [[$options(-ap) cget -host] cget -name]
	    }
	}
	if {$options(-wlcmd) eq ""} {
	    if {$options(-common)} {
		set options(-wlcmd) [concat $options(-ap) wl pktq_stats c:+]
	    } elseif {$options(-sta) ne ""} {
		set options(-wlcmd) [concat $options(-ap) wl pktq_stats a:+[$options(-sta) macaddr]]
	    } elseif {$options(-tx) ne "" && $options(-rx) ne ""} {
		set options(-wlcmd) "$options(-tx) wl pktq_stats a:+[[$options(-rx) lan] macaddr]"
	    }
	}
	if {$options(-wlcmd) eq ""} {
	    error "must set pktq stats wl cmd, or ap and sta, or ap and common"
	}
	set utf_msgtag $options(-name)
	set k [concat $options(-wlcmd) $options(-name) $options(-key)]
	regsub -all {[/<>]} $k "." k
	set myid [::md5::md5 -hex $k]
	if {$options(-wloutput) ne {}} {
	    $self set
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
	if {$options(-wlcmd) eq {} && $options(-ap) ne {} && $options(-sta) ne {}} {
	    if {!$options(-common)} {
		set options(-wlcmd) [concat $options(-ap) wl pktq_stats a:[$options(-sta) macaddr]]
	    } else {
		set options(-wlcmd) [concat $options(-ap) wl pktq_stats c:]
	    }
	}
	if {$options(-ap) ne ""} {
	    catch {$options(-ap) wl msglevel +time}
	}
	if {$options(-sta) ne ""} {
	    catch {$options(-sta) wl msglevel +time}
	}
	set firsttimestamp [$type clock]
    }
    destructor {
	foreach s $samples {
	    $s destroy
	}
	catch {unset INTERESTINGINDEXCACHE($myid)}
	catch {unset PREVIOUSSELF($myid,table)}
	catch {unset PREVIOUSSELF($myid,timestamp)}
	if {$WARNRATELIMITER ne "" && ![llength [$type info instances]]} {
	    catch {$WARNRATELIMITER destroy}
	}
    }
    option -wlcmd -readonly true -default {}
    option -name -readonly true -default {}
    option -statscache -default {} -readonly true
    option -key -readonly true -default {}

    method id {args} {
	return $myid
    }
    method inspect {args} {
	UTF::Getopts {
	    {all ""}
	}
	puts "ID:$myid"
	if {$rtrtimestamp ne {}} {
	    puts "[clock format [expr {round($utftimestamp)}]]\n$rtrtimestamp, $utftimestamp"
	} else {
	    puts "[clock format [expr {round($utftimestamp)}]]\n$utftimestamp"
	}
	if {$queue ne {}} {
	    puts $queue
	    parray rows
	    parray cols
	    parray derivatives
	}
	puts "samples: $samples"
	catch {puts "interesting([llengthINTERESTINGINDEXCACHE($myid)]): $INTERESTINGINDEXCACHE($myid)"}
	if {$(all)} {
	    parray values
	    catch {puts "prev timestamp=$PREVIOUSSELF($myid,timestamp)"}
	    # catch {puts "prev table=$PREVIOUSSELF($myid,table)"}
	}
    }
    # take raw wl output and initialize the stat
    method set {args} {
	UTF::Getopts {
	    {wloutput.arg ""}
	}
	$self id
	set utftimestamp [$type clock]; set mytimestamp $utftimestamp
	if {$(wloutput) ne ""} {
 	    set options(-wloutput) $(wloutput)
	}
	set lines [split $options(-wloutput) "\n"]
	if {[llength $lines] <=1} {
	    UTF::Message WARN $utf_msgtag "Empty"
	    return
	}
	if {[regexp {(^\[([0-9:\.]+)\]:)?(.+)} [lindex $lines 0] - field rtrtimestamp queue]} {
	    set queue [string trim [regsub {queue} [string trim $queue { ,:}] {}]]
	    if {$field ne ""} {
		set tmp [split $rtrtimestamp :]
		# TSF timestamp is 32 bits in units of microseconds
		# utf> expr {pow(2,32) / 1000000}
		# 4294.967296
		set rtrtimestamp [expr {($tsfwrapcount * 4294.967296) + (60 * [lindex $tmp 0]) + [lindex $tmp 1]}]
		# Check for TSF wrap
		if {$rtrtimestamp < $previoustsf} {
		    incr tsfwrapcount
		    set rtrtimestamp [expr {($tsfwrapcount * 4294.967296) + (60 * [lindex $tmp 0]) + [lindex $tmp 1]}]
		}
		set mytimestamp $rtrtimestamp
		set previoustsf $rtrtimestamp
	    } else {
		if {$WARNRATELIMITER eq ""} {
		    package require UTF::RateLimiter
		    set WARNRATELIMITER [UTF::RateLimiter %AUTO% -threshold 1]
		}
		$WARNRATELIMITER message WARN $utf_msgtag "Issue 'wl msglevel +time' for more accurate timestamps"
	    }
	} else {
	    UTF::Message WARN "Invalid line [lindex $lines 0]"
	    return
	}
	set ix 0
	foreach r [split [regsub prec: [lindex $lines 1] {}] ,] {
	    set l [string trim $r]
	    set cols($l) $ix
	    set index2clabel($ix) $l
	    incr ix
 	}
	set ix 0
	foreach r [lrange $lines 2 end] {
	    if {$r eq {}} {
		continue
	    }
	    set l [lindex $r 0]
	    set rows([lindex $r 0]) $ix
	    set index2rlabel($ix) $l
	    set jx 0
	    foreach v [lrange $r 1 end] {
		if {[set v [string trim $v {, }]] eq "-"} {
		    set v 0
		}
		set values($index2rlabel($ix),$index2clabel($jx)) $v
		incr jx
	    }
	    # Compute any row compound stats
	    #
	    # %error = 1 - #acked/(#stored+#retried)
	    if {![catch {expr {1.0 - (1.0 * $values($index2rlabel($ix),acked)/($values($index2rlabel($ix),stored) + $values($index2rlabel($ix),retried)))}} percerr]} {
		set percerr [format %0.02f [expr {round(10000 * $percerr) / 100.0}]]
		if {[expr {$percerr != 0}] && $options(-verbose)} {
		    UTF::Message STAT $utf_msgtag "$index2rlabel($ix) percent error = ${percerr}%    (1 - #acked/(#stored+#retried)) * 100%"
		}
		set values($index2rlabel($ix),percerr) $percerr
	    } else {
		set values($index2rlabel($ix),percerr) -
	    }
	    incr ix
	}
	if {[catch {UTF::Message LOG $utf_msgtag "Timestamps($queue): [format %0.3f $mytimestamp] [format %0.3f $PREVIOUSSELF($myid,timestamp)] Diff: [format %0.3f [expr {$mytimestamp - $PREVIOUSSELF($myid,timestamp)}]]"}]} {
	    UTF::Message LOG $utf_msgtag "Timestamps($queue): [format %0.3f $mytimestamp]"
	}
	# If there is no table in this output, reset things and return
	if {!$ix} {
	    catch {unset PREVIOUSSELF($myid,table)}
	    return
	}
	if {[info exists PREVIOUSSELF($myid,table)]} {
	    array set s1 $PREVIOUSSELF($myid,table)
	    foreach index [array names s1] {
		if {$s1($index) ne $values($index)} {
		    set delta [expr {$values($index) - $s1($index)}]
		    set timedelta [expr {$mytimestamp - $PREVIOUSSELF($myid,timestamp)}]
		    set derivatives($index,first,valuedelta) $delta
		    set derivatives($index,first,timedelta) $timedelta
		    # log non-zero first derivatives
		    if {$options(-verbose)} {
			UTF::Message LOG $utf_msgtag "DELTA $delta ${queue}\(${index}\) ($values($index) $s1($index) [format %0.3f $timedelta] s)"
		    }
		    # cache in a typevar for faster post analysis
		    set k "$queue,$index"
		    if {![info exists INTERESTINGINDEXCACHE($myid)] || [lsearch $INTERESTINGINDEXCACHE($myid) $k] == -1} {
			lappend INTERESTINGINDEXCACHE($myid) $k
		    }
		}
	    }
	}
	# Store a copy of self to be used for running deltas
	set PREVIOUSSELF($myid,table) [array get values]
	set PREVIOUSSELF($myid,timestamp) $mytimestamp
    }
    method sample {args} {
	set s [UTF::wlstats::pktqstat %AUTO% -name $options(-name) -key $options(-key) -wlcmd $options(-wlcmd)]
	set err {}
	if {![catch {eval $options(-wlcmd)} output] && ![catch {$s set -wloutput $output} err]} {
	    lappend samples $s
	} else {
	    $s destroy
	    UTF::Message ERROR $utf_msgtag "$err $output"
	}
    }
    method clear {} {
	UTF::Message INFO $utf_msgtag "pktqstat samples cleared"
	foreach s $samples {
	    $s destroy
	}
	set samples {}
	catch {unset INTERESTINGINDEXCACHE($myid)}
	catch {unset PREVIOUSSELF($myid,table)}
	catch {unset PREVIOUSSELF($myid,timestamp)}
	return
    }
    method interesting {args} {
	UTF::Getopts {
	    {show "Write a message with any stats that had a non-zero first derivative"}
	    {search.arg "" "error if this stat is found"}
	}
	if {![info exists INTERESTINGINDEXCACHE($myid)]} {
	    return 0
	}
	if {$(show)} {
	    UTF::Message INFO $utf_msgtag "interesting: $INTERESTINGINDEXCACHE($myid)"
	}
	if {$(search) ne {} && [regexp "$(search)" $INTERESTINGINDEXCACHE($myid)]} {
	    return 1
	} else {
	    return 0
	}
    }
    method get {args} {
	UTF::Getopts {
	    {nonzero "all nonzero values"}
	    {timestamp "return best timestamp available"}
	    {queue "return queue"}
	}
	if {$(timestamp)} {
	    if {$rtrtimestamp ne {}} {
		return $rtrtimestamp
	    } else {
		return $utftimestamp
	    }
	} elseif {$(queue)} {
	    if {$queue ne {}} {
		return $queue
	    } elseif {[llength $samples]} {
		return [[lindex $samples 0] get -queue]
	    }
	} else {
	    return [array get values]
	}
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
	if {$options(-key) ne {}} {
	    set outfile [file join $options(-statscache) $options(-key)]
	} else {
	    set outfile [file join $options(-statscache) running]
	}
	if {$(clear)} {
	    set rw "w"
	} else {
	    set rw "a+"
	}
 	set G [open $outfile $rw]
 	fconfigure $G -buffering line
	if {[llength samples]} {
	    if {[info exists INTERESTINGINDEXCACHE($myid)]} {
		puts $G "interesting: $INTERESTINGINDEXCACHE($myid)"
	    }
	    foreach sample $samples {
		$sample _write_table $G
	    }
	} else {
	    $self _write_table $G
	}
	close $G
    }
    method _write_table {G} {
	puts $G "$queue"
	if {[info exists rtrtimestamp]} {
	    puts $G "[clock format [expr {int($utftimestamp)}] -format "%T"] $rtrtimestamp : $utftimestamp"
	} else {
	    puts $G "[clock format [expr {int($utftimestamp)}] -format "%T"] $utftimestamp"
	}
	set ix 0; set jx 0;
	for {set jx 0} {$jx < [llength [array names rows]]} {incr jx} {
	    for {set ix 0} {$ix < [llength [array names cols]]} {incr ix} {
		puts $G "$index2rlabel($jx),$index2clabel($ix),$values($index2rlabel($jx),$index2clabel($ix))"
	    }
	}
	puts $G {}
    }
    method plot {args} {
	UTF::Getopts {
	    {interesting "plot only stats that change"}
	}
	if {[llength $samples] && [info exists INTERESTINGINDEXCACHE($myid)]} {
	    foreach which $INTERESTINGINDEXCACHE($myid) {
		set stats [$self _get_timeseries $which]
	    }
	    UTF::Message STATS $self $stats
	}
    }
    method _get_timeseries {label} {
	set res {}
	foreach sample $samples {
	    set res [concat [$sample _get_entry -label $label -offset $firsttimestamp]]
	}
	return $res
    }

    method _get_entry {args} {
	UTF::Getopts {
	    {label.arg "" "stat of interest"}
	    {offset.arg "0" "time offset"}
	}
	return "$values($index2rlabel($(label)),$index2clabel($(label))) [expr {$mytimestamp - $(offset)}]"
    }

    method filename {} {
	return $outfile
    }
    method myhtmllink {args} {
	UTF::Getopts {
	    {text.arg "pktqstats" "text for html link"}
	    {append ""}
	}
	if {$(append)} {
	    return "<a href=\"http://${::UTF::WebServer}$outfile\">$(text)</a>"
	} else {
	    return "html:<a href=\"http://${::UTF::WebServer}$outfile\">$(text)</a>"
	}
    }
}
