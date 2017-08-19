#
# UTF Daemon
#
# UTF Dameon package: Run UTF controller as a daemon with a scheduler
#
# Introduce metascripts which are UTF objects defining a scheduable script
# Removes the need for both SGE and CRON by UTF
#
# Written by: Robert J. McMahon November 2013
#
# $Id: 2cb1ebf78ea92c1b22f2d8739a2ae9b702ddaf6a $
# $Copyright Broadcom Corporation$
package require UTF
package require snit
package require md5
package require math::statistics

package provide UTFD 2.0

# Use Tclx if available:
catch {
      package require Tclx
      signal error SIGINT
      signal trap SIGINT {::UTFD::pause 10; UTFD::abort}
}

namespace eval UTFD {
    variable PAUSEDEFAULT 10; #units minutes
    variable queues
    variable runningpids {}
    variable debug 0
    variable port 9988
    variable httpdport 8080
    variable UTFDPID -1
    variable logdir
    variable email
    variable webserver
    variable savewebserver "http://www.sj.broadcom.com/"
    variable next {}
    variable runningpri 99
    variable watchdogs
    variable LAUNCHRATE 5000
    variable launchratelimiter 0
    variable pause
    variable reluptime
    variable absuptime
    variable launchtime
    variable timers
    variable mrt ; #maximum runtime
    variable preempt 0
    variable restorewatchlist {}
    variable fshbug 1
    variable finished
    variable finishedcount 1
    variable statusreports
    variable noreentrant 0
    variable utfdnoreentrant 0
    variable devicelogs
    variable exiting 0
    variable liveviewpage
    variable kicker

    array set queues {
	cron ""
	triggered ""
	background ""
	now ""
	running ""
    }
    array set pause {
	aid {}
	pending 0
	start 0
	minutes 0
	enabled 0
    }
    array set timers {
	1 0
	2 0
	3 0
	98 0
    }
    array set liveviewpage {
	filter {}
	filename default
	date {}
	days {}
	button {}
    }
    array set logdir {}
    array set email {}
    array set webserver {}
    array set watchdogs {}
    array set finished {}
    array set statusreports {}
    array set devicelogs {}
    #
    #  UTFD user/api procs below
    #
    proc utfdexit {} {
	::UTFD::savestate
	::UTFD::push_watch
	set ::UTFD::exiting 1
	set ::UTFD::pause(enabled) 0
	::UTFD::clear -reports
	::UTFD::destroyall
	UTF::Message CTRLR "" "UTFD exit done"
    }
    proc writeqstatus {} {
	set filename [file join [::UTFD::metascript utfddir]/qstatus]
	set fid [open $filename w]
	foreach q [array names ::UTFD::queues] {
	    puts $fid "$q [llength $::UTFD::queues($q)]"
	}
	close $fid
    }
    proc savestate {} {
	if {$::UTF::args(nolock)} {
	    return
	}
	if {$::UTFD::exiting} {
	    return
	}
	set filename [file join [::UTFD::metascript utfddir]/livestate]
	set fid [open $filename w]
	puts $fid "<finished>[array get ::UTFD::finished]</finished>"
	foreach metascript [::UTFD::metascript info instances] {
	    $metascript save $fid
	}
	foreach q [array names ::UTFD::queues] {
	    lappend queues [list $q [::UTFD::metascript typepriority $q]]
	}
	set queues [lsort -integer -index 1 $queues]
	foreach q $queues {
	    set q [lindex $q 0]
	    puts $fid "<queue>$q</queue>"
	    foreach element $::UTFD::queues($q) {
		puts $fid "<enq>[$element id]</enq>"
	    }
	}
	foreach metascript [::UTFD::metascript info instances] {
	    if {[$metascript watch info] ne ""} {
		puts $fid "<watch>[$metascript id]</watch>"
	    }
	}
	puts $fid "<statusreports>[array get ::UTFD::statusreports]</statusreports>"
	close $fid
	::UTFD::writeqstatus
    }
    proc destroyall {} {
	set last {}
	foreach m [::UTFD::metascript info instances] {
	    if {[$m mystate] eq "RUNNING"} {
		lappend last $m
	    } else {
		catch {$m destroy}
	    }
	}
	foreach m $last {
	    catch {$m destroy}
	}
	set aid [after [expr {10 * 1000}] {}]
	while {[llength $::UTFD::queues(running)] && ![catch {after info $aid}]} {
	    UTF::Sleep 1 quiet
	}
    }
    proc restorestate {} {
	if {![::UTFD::isDaemon?]} {
	    return
	}
	if {$::UTF::args(nolock)} {
	    return
	}
	set filename [file join [::UTFD::metascript utfddir]/livestate]
	if {[catch {open $filename r} fid]} {
	    error $fid
	}
	UTF::Message CTRLR "RESTORE" "UTFD restoring previous state"
	set cmd {}
	set q {}
	while {[gets $fid line] > 0} {
	    if {[regexp {<finished>(.*)</finished>} $line - finished]} {
		array set ::UTFD::finished $finished
		incr ::UTFD::finishedcount [expr {[llength $finished] / 2}]
	    }
	    if {[regexp {<statusreports>(.*)</statusreports>} $line - reports]} {
		array set ::UTFD::statusreports $reports
	    }
	    if {[regexp {<snittype>(.+)</snittype>} $line - snittype]} {
		if {$cmd ne {}} {
		    eval [concat $snittype %AUTO% -createonly 1 $cmd]
		    set cmd {}
		}
	    } elseif {[regexp {<attribute>(.+)</attribute><value>(.*)</value>} $line - a v]} {
		lappend cmd $a $v
	    } elseif {[regexp {<queue>(.+)</queue>} $line - q]} {
		if {$cmd ne {}} {
		    eval [concat $snittype %AUTO% -createonly 1 $cmd]
		    set cmd {}
		}
	    } elseif {[regexp {<enq>(.+)</enq>} $line - hash]} {
		if {[set m [::UTFD::metascript hash2instance $hash]] ne ""} {
		    ::UTFD::enqueue $m
		}
	    } elseif {[regexp {<watch>(.+)</watch>} $line - hash]} {
		if {[set m [::UTFD::metascript hash2instance $hash]] ne ""} {
		    $m watch enable
		}
	    }
	}
	close $fid
    }
    proc qstat {args} {
	set metascripts [::UTFD::metascript info instances]
	set watchlist {}
	foreach metascript $metascripts {
	    if {[$metascript watch info] ne ""} {
		lappend watchlist $metascript
	    }
	}
	parray ::UTFD::queues
	puts "Watching: $watchlist"
    }
    proc counters {args} {
	UTF::Getopts {
	    {reset ""}
	}
	if {$(reset)} {
	    array set ::UTFD::timers {
		1 0
		2 0
		3 0
		4 0
	    }
	    set ::UTFD::reluptime [clock seconds]
	    # Clear older than 24 hours
	    foreach index [array names ::UTFD::finished] {
		if {[expr {[clock seconds] - $index} > 86400]} {
		    unset ::UTFD::finished($index)
		}
	    }
	}
	return
    }
    proc pause {args} {
	if {$args eq ""} {
	    set minutes $::UTFD::PAUSEDEFAULT
	} elseif {[string is integer $args]} {
	    set minutes $args
	} else {
	    if {[catch {clock scan $args} seconds]} {
		error $seconds
	    } else {
		set minutes [expr {round(($seconds - [clock seconds])/60)}]
	    }
	}
	if {$minutes < 0} {
	    error "Pause time must be equal or greater than zero"
	}
	if {!$minutes } {
	    ::UTFD::go
	    return
	}
	set ::UTFD::pause(minutes) $minutes
	if {[llength $::UTFD::queues(running)]} {
	    set ::UTFD::pause(pending) 1
	    if {![$::UTFD::queues(running) cget -preemptable]} {
		return
	    } else {
		::UTFD::abort "pause request"
		::UTFD::_dopause
	    }
	} else {
	    ::UTFD::_dopause
	}
    }
    proc push_watch {} {
	set metascripts [::UTFD::metascript info instances]
	foreach metascript $metascripts {
	    if {[$metascript watch info] ne ""} {
		$metascript watch remove
		if {[lsearch $::UTFD::restorewatchlist $metascript] < 0} {
		    lappend ::UTFD::restorewatchlist $metascript
		}
	    }
	}
    }
    proc pop_watch {} {
	if {[llength $::UTFD::restorewatchlist]} {
	    foreach m $::UTFD::restorewatchlist {
		if {[::UTFD::metascript info instances $m] eq $m} {
		    $m watch enable
		}
	    }
	    set ::UTFD::restorewatchlist {}
	}
    }
    proc _dopause {} {
	if {[info exist ::UTFD::pause(aid)] && $::UTFD::pause(aid) ne ""} {
	    UTF::Message CTRLR INFO "Existing pause cancelled"
	}
	set ::UTFD::pause(pending) 0
	set ::UTFD::pause(start) [clock seconds]
	set ::UTFD::pause(enabled) 1
	set ::UTFD::pause(aid) [after [expr {round($::UTFD::pause(minutes) * 60000)}] [list ::UTFD::go]]
	UTF::Message CTRLR INFO "Pausing for $::UTFD::pause(minutes) minutes ($::UTFD::pause(aid))"
	::UTFD::push_watch
    }
    proc go {} {
	set ::UTFD::pause(enabled) 0
	set ::UTFD::pause(pending) 0
	catch {after cancel $::UTFD::pause(aid)}
	::UTFD::pop_watch
	after 40 [list ::UTFD::utfdevent "pause disabled"]
    }
    proc clearrange {a b} {
	foreach i [array names UTFD::finished] {
	    if {[lindex $UTFD::finished($i) 0] < $a || [lindex $UTFD::finished($i) 0] > $b} {
		continue
	    } else {
		unset UTFD::finished($i)
	    }
	}
    }
    proc clear {args} {
	UTF::Getopts {
	    {queues "Clear the UTFD queues"}
	    {reports "Clear the finished reports"}
	}
	if {$(queues)} {
	    foreach index [array names ::UTFD::queues] {
		if {$index eq "running"} {
		    continue
		}
		set ::UTFD::queues($index) {}
	    }
	}
	if {$(reports)} {
	    array unset ::UTFD::statusreports *
	    array unset ::UTFD::finished *
	}
	if {!$(queues) && !$(reports)} {
	    UTF::Message CTRLR "" "Usage is UTFD::clear [-queues | -reports]"
	}
    }
    proc tformat {time} {
	set days [expr {$time / 86400}]
	set r [expr {$time % 86400}]
	set hours [expr {$r / 3600}]
	set r [expr {$r % 3600}]
	set minutes [expr {$r / 60}]
	set seconds [expr {$r % 60}]
	set final [format "%02d:%02d:%02d" $hours $minutes $seconds]
	if {$days <= 0} {
	    return $final
	} elseif {$days > 1} {
	    return "$days days and $final"
	} else {
	    return "1 day and $final"
	}
    }
    #
    # Server (ctrlr) procedures below
    #
    proc listen {} {
	if {[info exists ::env(UTFDPORT)]} {
	    set ::UTFD::port $::env(UTFDPORT)
	}
	set ::UTFD::UTFDPID [pid]
	set ::env(UTFDPID) [pid]
	set pidfile [file join $::UTF::SummaryDir utfd${::UTFD::port}.pid]
	if {[catch {socket -server ::UTFD::accept $::UTFD::port} sid]} {
	    UTF::Message CTRLR ERROR "$sid port=$::UTFD::port"
	    set pid "-1"
	    if {[file exists $pidfile]} {
		set fid [open $pidfile r]
		gets $fid pid
		close $fid
		UTF::Message CTRLR WARN "UTFD instance already running, pid may be $pid"
	    } elseif {![catch {exec netstat -ltp} output]} {
		foreach line [split $output "\n"] {
		    if {[regexp ":$::UTFD::port" $line]} {
			UTF::Message CTRLR INFO $line
		    }
		}
	    }
	    catch {unset ::env(UTFDPID)}
	    catch {unset ::env(UTFD)}
	    catch {unset ::env(UTFDPORT)}
	} else {
	    set fid [open $pidfile w]
	    puts $fid [pid]
	    close $fid
	    UTF::Message CTRLR ACCEPT "UTF controller accepting commands on port $::UTFD::port with pid [pid]"
	    set ::UTFD::absuptime [clock seconds]
	    set ::UTFD::reluptime $::UTFD::absuptime
	    set idletime 0
	    if {![catch {socket -server ::UTFD::httpaccept $::UTFD::httpdport} err]} {
		UTF::Message CTRLR HTTP "UTF controller httpd started on port $::UTFD::httpdport"
	    } else {
		UTF::Message CTRLR HTTP $err
	    }
	    # Expand rig name by adding utfconf/ and .tcl
	    if {![file exists $::UTF::args(utfconf) ] &&
		![regexp {/}  $::UTF::args(utfconf)] &&
		[file extension $::UTF::args(utfconf)] ne ".tcl"} {
		set ::UTF::args(utfconf) "utfconf/${::UTF::args(utfconf)}.tcl"
		if {![file exists $::UTF::args(utfconf)]} {
		    error "UTFD needs a valid utfconf $::UTF::args(utfconf)"
		}
	    }
	    if {[file exists [file join [::UTFD::metascript utfddir]/livestate]]} {
		::UTFD::restorestate
	    } elseif {[info exists ::UTFD::scripts]} {
		UTF::Message CTRLR "INIT" "UTFD first time initialization"
		eval $::UTFD::scripts
	    }
	    # Only allow one restorestate per UTFD start
	    rename ::UTFD::restorestate {}
	    if {![catch {package present TclReadLine}]} {
		::TclReadLine::addExitHandler ::UTFD::utfdexit
	    }
	}
    }
    proc accept {chan addr port} {
	if {$addr ne "127.0.0.1"} {
	    UTF::Message REJECT  $addr "UTFD Commands must come from localhost"
	    catch {close $chan}
	    return
	}
	set wd [after 500 [list ::UTFD::wdclose $chan "Timeout after accept"]]
	fileevent $chan readable [list ::UTFD::__command $chan $addr $port $wd]
	if {$::UTFD::debug} {
	    UTF::Message CTRLR "ACPT" "$chan $addr $port"
	}
    }
    proc httpaccept {chan addr port} {
	fileevent $chan readable [list ::UTFD::httpevent $chan $addr $port]
	if {$::UTFD::debug} {
	    UTF::Message CTRLR "HTTP" "$chan $addr $port"
	}
    }
    proc wdclose {chan {message ""}} {
	if {$message ne ""} {
	    UTF::Message CTRLR CLOSE "$message $chan"
	}
	close $chan
    }
    proc __command {chan addr port wd} {
	fileevent $chan readable {}
	fconfigure $chan -buffering full -blocking 0
	catch {after cancel $wd}
	if {$::UTFD::debug} {
	    UTF::Message CTRLR "READ" "$chan $addr $port"
	}
	set buf  {}
	set rwd [after 500 [list ::UTFD::wdclose $chan "Timeout during read"]]
	while {![catch {append buf [read -nonewline $chan]}] && ![eof $chan]} {
	    UTF::Sleep 0 quiet
	}
	# If watchdog triggered second close will error
	if {[catch {close $chan}]} {
	    UTF::Message CTRLR "READ" "Watchdog $chan $buf"
	    return
	}
	after cancel $rwd
	UTF::Message CTRLR IPC "$buf"
	switch -exact [lindex $buf 0] {
	    "script" {
		if {[catch {eval [lindex $buf 1]} err]} {
		    UTF::Message CTRLR ERROR $err
		}
	    }
	    default {
		UTF::Message CTRLR ERROR "Invalid IPC type"
	    }
	}
    }
    proc savepage {args} {
	UTF::Getopts {
	    {name.arg "" "Name to use for link"}
	    {filter.arg "" "Filter results"}
	}
	set head ""
	set body ""
	::UTFD::mainhead head
	::UTFD::finishedbody body $(filter)
	set savedir [file join [::UTFD::metascript utfddir] saved]
	if {![file exists $savedir]} {
	    if {[catch {file mkdir $savedir} res]} {
		UTF::Message CTRLR ERROR "UTFD: unable to make directory $directory"
		error "save fail: can't make directory"
	    }
	}
	if {$(name) eq ""} {
	    set (name) [clock seconds]
	}
	set filename [file join $savedir ${(name)}.html]
	set fid [open $filename w]
	puts $fid "$head\n$body"
	close $fid
	return "[string trim ${::UTFD::savewebserver} /]${filename}"
    }
    proc httprespond {sock code body {head ""}} {
	if {$::UTFD::debug} {
	    UTF::Message HTTP SEND $head
	    UTF::Message HTTP SEND $body
	}
	fconfigure $sock -buffering full -buffersize 1000000
	puts -nonewline $sock "HTTP/1.0 $code ???\nContent-Type: text/html; \
		charset=Big-5\nConnection: close\nContent-length: [expr {[string length $body] + [string length $head] + 1}]\n\n$head\n$body"
	flush $sock
    }
    proc httpevent {chan addr port} {
	fileevent $chan readable {}
	fconfigure $chan -buffering line -blocking 0
	if {$::UTFD::debug} {
	    UTF::Message CTRLR "READ" "$chan $addr $port"
	}
	if {[gets $chan buf] > 0 && ![fblocked $chan]} {
	    if {$::UTFD::debug} {
		UTF::Message CTRLR READ $buf
	    }
	    while {[gets $chan tmp] > 0 && ![eof $chan]} {
		if {$::UTFD::debug} {
		    UTF::Message CTRLR READ $tmp
		}
	    }
	    foreach {method url version} $buf {break}
	    if {$method eq "POST"} {
		::UTFD::mainpage $chan $addr $port
	    } elseif {$method eq "GET"} {
		if {$::UTFD::debug} {
		    UTF::Message CTRLR GET $url
		}
		if {[regexp {/liveview.asp\?(.*)} $url - urlparms]} {
		    foreach parm [split $urlparms &] {
			set parm [split $parm =]
			set p [lindex $parm 0]
			set v [lindex $parm 1]
			set ::UTFD::liveviewpage($p) [string map {+ " " %2F /} $v]
		    }
		    if {$::UTFD::liveviewpage(button) eq "save"} {
			::UTFD::savepage -name $::UTFD::liveviewpage(filename) -filter $::UTFD::liveviewpage(filter)
		    }
		    ::UTFD::mainpage $chan $addr $port
		} elseif {[regexp {/utfdsvn.cgi\??(.)*} $url - urlparms]} {
		    set urlparms [lindex [split $url ?] 1]
		    set cgiscript [file join [exec pwd] cgi utfdsvn.cgi]
		    if {$urlparms ne ""} {
			lappend cgiscript $urlparms
		    }
		    set buf {}
		    # set buf [localhost rexec -x -silent -quiet $cgiscript]
		    if {![catch {open "|$cgiscript" r} fid]} {
			fconfigure $fid -buffering full -blocking 0 -buffersize 1000000
			while {![eof $fid]} {
			    append buf [read $fid]
			}
			close $fid
			::UTFD::httprespond $chan 200 $buf
		    } else {
			::UTFD::httprespond $chan 500 "cgi error"
		    }
		} elseif {[regexp {/kpireport.cgi\??(.)*} $url - urlparms]} {
		    if {$::UTFD::debug} {
			UTF::Message CTRLR CGI $url
		    }
		    set urlparms [lindex [split $url ?] 1]
		    set cgiscript [file join [exec pwd] bin kpi_report.py]
		    if {$urlparms ne ""} {
			lappend cgiscript "--new "/projects/hnd_sig_ext21/sumesh/md15/20151007095659/ /projects/hnd_sig_ext21/sumesh/md15/20151007122708/\" --outlierIQR" --output \"/projects/hnd_sig_ext21/sumesh/md15/reports\""
		    }
		    set buf {}
		    if {$::UTFD::debug} {
			UTF::Message CTRLR "CGI" "$cgiscript"
		    }
		    # set buf [localhost rexec -x -silent -quiet $cgiscript]
		    if {![catch {open "|$cgiscript" r} fid]} {
			fconfigure $fid -buffering full -blocking 0 -buffersize 1000000
			while {![eof $fid]} {
			    append buf [read $fid]
			}
			close $fid
			::UTFD::httprespond $chan 200 $buf
		    } else {
			::UTFD::httprespond $chan 500 "cgi error"
		    }
		} else {
		    set ::UTFD::liveviewpage(filter) ""
		    set ::UTFD::liveviewpage(date) ""
		    set ::UTFD::liveviewpage(days) ""
		    set ::UTFD::liveviewpage(filename) "default"
		    ::UTFD::mainpage $chan $addr $port
		}
	    }
	    if {[catch {close $chan} err]} {
		UTF::Message CTRLR ERROR $err
		return
	    }
	}
    }
    proc mainpage {chan addr port} {
	set head ""; set body ""
	::UTFD::mainhead head
	::UTFD::mainbody body
	::UTFD::httprespond $chan 200 $body $head
    }
    proc mainhead {thishead} {
	upvar $thishead head
	set rig [file rootname [lindex [file split $::utfconf] end]]
	set head {<?xml version="1.0" encoding="utf-8"?>}
	append head "\n"
	append head {<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">}
	append head {<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">}
	append head "\n"
	append head "<head>\n"
	append head "<title>utfd-${rig}</title>\n"
	append head "<link href=\"data:image/png;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAQAAMMOAADDDgAAAAAAAAAAAAD////////////////////////////9/f//////3Y2N/9yCfP/QVk//4I6F/+27tP////3//////////////////////////////////////////////fz//////9h6fP/yx7j/7MXN/81LQf/qraT//vv4/////////////////////////////////////////////vz8///////biY7/5ZaG///////ch4j/7biq///////+/fz////////////+//////////////////////////78/P//////4Z2j/9x3aP//////57W9/9hqW////////v3+////////////+fT7///+///////////////////+/Pz//////+i2vv/TW0z//vv5/9FiZf/acF7///////79/v////////////ft9////v///vz8//79/P///v7//vz8///////z3OL/zExG/85PR//PTUH/9dTK/////////f3////////////26/X//v7+///////////////////+/f//////3ImN/9FYTv/Xb2f/7LWq///////+/f3/////////////////+fX8//7////psrL/78O//9+QkP/zzcL//////9uAfv/98Ob/7MfP/918bv///////vz9///////////////////////w0tr/yT02/9RjWv/HOC//7ram///////SYl3//ezh/+3K0v/ZcWP///////79/v//////////////////////56yw/9lsXP/68/X/yUI//+qpmP//////0VxZ//jbzv/qwMn/0llM//728P/+/f7///39/////////////////+erp//egG///////9Z1ef/dfGr//////9JkZP/vu6z/79LZ/8pAOf/SXlX/zlBG//HGuv///////vz8///////nq6j/3X1t///////lrLT/0lZH///////Wdnv/4417//39///QVVD/89DG//TY2P/88u/////////+/v//////6K2q/9dpWP//////8tnh/8tDOv/54tj/35ef/9tyX///////zlZV/+uunv///////vr6/////////////////+etsv/Ya1z///////z5/v/OVlb/yj8y/8xLRP/KPzT/6q2h/+zCx//LRj//zEU7/+67rv///////vz8///////67/H/+uvp////////////5q2u/+qwqP/uxMP/6K2p//TTzf//////9uLj//DJxf/78Oz////////+/v///////////////////////////////v////////////////////////7+////////////////////////////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==\" rel=\"icon\" type=\"image/png\"/>\n"
	append head {<style type="text/css">.TFtable{width:100%;border-collapse:collapse;}.TFtable td{padding:7px; border:#4e95f4 1px solid;}.TFtable tr{background: #b8d1f3;}.TFtable tr:nth-child(odd){background: #b8d1f3;}.TFtable tr:nth-child(even){background: #dae5f4;}.left{float: left;}.right{float: right;}.center{float: center;}.nobreak{white-space: nowrap;}</style>}
	append head "\n</head>"
    }
    proc mainbody {thisbody} {
	upvar $thisbody body
	set rig [file rootname [lindex [file split $::utfconf] end]]
	set body "<body>\n"
	set m $::UTFD::queues(running)
	set up [expr {[clock seconds] - $::UTFD::absuptime}]
	set relup [expr {[clock seconds] - $::UTFD::reluptime}]
	if {$m ne ""} {
	    set accum [expr {[clock seconds] - $::UTFD::launchtime}]
	    set adj $accum
	} else {
	    set accum 0
	}
	foreach q [lsort -integer [array names ::UTFD::timers]] {
	    incr accum $::UTFD::timers($q)
	}
	if {$accum} {
	    foreach q [lsort -integer [array names ::UTFD::timers]] {
		if {$m ne "" && $q eq $::UTFD::runningpri} {
		    append pritxt "[expr {round(100.0 * ($adj + $::UTFD::timers($q))/$accum)}]/"
		} else {
		    append pritxt "[expr {round(100.0 * $::UTFD::timers($q)/$accum)}]/"
		}
	    }
	} else {
	    set pritxt "0/0/0/0"
	}
	set pritxt [string trim $pritxt /]
	set relup [expr {[clock seconds] - $::UTFD::reluptime}]
	set idle [expr {$relup - $accum}]
	set idleperc [expr {round(100.0 * $idle/$relup)}]
	set accumperc [expr {round(100.0 * $accum/$relup)}]
	set up [::UTFD::tformat $up]
	set relup [::UTFD::tformat $relup]
	set idle [::UTFD::tformat $idle]
	set svnurl "http://${::env(HOSTNAME)}:${::UTFD::httpdport}/utfdsvn.cgi?rig=$rig"
	set saveurl "[string trim ${::UTFD::savewebserver} /][file join [::UTFD::metascript utfddir] saved]"
	append body "<h2>UTFD: $rig"
	if {$::UTFD::pause(pending) && $m ne ""} {
	    append body " <small>(pause pending)</small>"
	}
	append body "   <small>(uptime:$up run/idle: $relup/$idle) (r/i: $accumperc/${idleperc}  1/2/3/4: $pritxt %) (Status a=aborted b=bounds c=completed e=error n=no previous run)</small><span class=\"right\">[clock format [clock seconds] -format [list %a %b %d %H:%M:%S]]&nbsp;<a href=\"$saveurl\"><small>saved</small></a></span></h2>\n"
	if {$m ne ""} {
	    if {[set t [$m cget -maxruntime]] > 0} {
		set max [UTFD::tformat [expr {round($t * 3600)}]]
	    } else {
		set max ""
	    }
	    append body "<table summary=\"Current Running\" class=\"TFtable\">"
	    append body "<tr>"
	    if {[catch {set url "http://$::UTFD::webserver($::UTFD::runningpids)$::UTFD::logdir($::UTFD::runningpids)/summary.html"}]} {
		set url ""
	    }
	    append body "<th align=\"left\"><span class=\"nobreak\">($::UTFD::runningpri) RUNNING</span></th>"
	    append body "<th align=\"center\">START</th>"
	    append body "<th align=\"left\">RUNNING</th>"
	    append body "<th align=\"center\"><small>(pid)</small></th>"
	    append body "<th align=\"left\"><small>(pre/crt)</small></th>"
	    if {$max ne ""} {
		append body "<th align=\"center\"><small>(max)</small></th>"
	    }
	    append body "<th align=\"center\">SCRIPT</th>"
	    append body "<th align=\"left\">STATS</th>"
	    append body "<th align=\"center\">LOG</th>"
	    append body "</tr>"
	    append body "<tr>"
	    append body "<td>$m</td>"
	    if {$url ne ""} {
		append body "<td><span class=\"nobreak\"><a href=\"$url\">[$m timestamp -type start]</a> <strong>[$m status -abbreviated]</strong></span></td>"
	    } else {
		append body "<td><span class=\"nobreak\">[$m timestamp -type start]</span></td>"
	    }
	    append body "<td>[$m timestamp -type end -format stopwatch]</td>"
	    append body "<td>$::UTFD::runningpids</td>"
	    append body "<td align=\"center\">[$m cget -preemptable]/[$m cget -committedruntime]</td>"
	    if {$max ne ""} {
		append body "<td>$max</td>"
	    }
	    append body "<td>[$m cget -script]</td>"
	    set stats [$m stats]
	    if {$stats ne ""} {
		append body "<td><span class=\"nobreak\">$stats</span></td>"
	    } else {
		append body "<td></td>"
	    }
	    append body "<td>[$m cget -log]</td>"
	    append body "</tr>"
	    append body "</table>\n"
	    append body "<br />\n"
	} else {
	    append body "<table summary=\"Nothing Running\" class=\"TFtable\">"
	    append body "<tr>"
	    append body "<th align=\"left\">RUNNING</th>"
	    if {!$::UTFD::pause(enabled)} {
		append body "<th align=\"center\">START</th>"
		append body "<th align=\"left\">RUNTIME</th>"
	    } else {
		append body "<th align=\"left\">START</th>"
		append body "<th align=\"left\">REMAINING</th>"
	    }
	    append body "<th align=\"center\">SCRIPT</th>"
	    append body "<th align=\"left\">STATS</th>"
	    append body "<th align=\"center\">LOG</th>"
	    append body "</tr>"
	    append body "<tr>"
	    if {$::UTFD::pause(enabled)} {
		append body "<td>PAUSED</td>"
		append body "<td>[clock format $::UTFD::pause(start) -format "%a %b %d %H:%M:%S"]</td>"
		append body "<td>[::UTFD::tformat [expr {round(60 * $::UTFD::pause(minutes)) - ([clock seconds] - $::UTFD::pause(start))}]]</td>"
	    } else {
		append body "<td>NONE</td>"
		append body "<td></td>"
		append body "<td></td>"
	    }
	    append body "<td></td>"
	    append body "<td></td>"
	    append body "<td></td>"
	    append body "</tr>"
	    append body "</table>\n"
	    append body "<br />\n"
	}
	# Sort the tables in priority order
	set tables {}
	foreach q [array names ::UTFD::queues] {
	    lappend tables [list $q [::UTFD::metascript typepriority $q]]
	}
	set tables [lsort -integer -index 1 $tables]
	foreach table $tables {
	    set queue [lindex $table 0]
	    if {$queue eq "running"} {
		continue
	    }
	    if {[llength $::UTFD::queues($queue)]} {
		append body "<table summary=\"Queued listing\" class=\"TFtable\">"
		append body "<tr>"
		append body "<th align=\"left\"><span class=\"nobreak\">([::UTFD::metascript typepriority $queue]) [string toupper $queue]</span></th>"
		append body "<th align=\"center\">PREV</th>"
		append body "<th align=\"left\">RUNTIME</th>"
		append body "<th align=\"center\">SCRIPT</th>"
		append body "<th align=\"left\">STATS</th>"
		append body "<th align=\"center\">LOG</th>"
		append body "</tr>"
		foreach m $::UTFD::queues($queue) {
		    append body "<tr>"
		    append body "<td>$m</td>"
		    if {[set url [$m myurl]] eq ""} {
			append body "<td>[$m timestamp -type end -format date]</td>"
		    } else {
			append body "<td><span class=\"nobreak\"><a href=\"$url\">[$m timestamp -type end -format date]</a> <strong>[$m status -abbreviated]</strong></span></td>"
		    }
		    append body "<td>[$m timestamp -type end -format stopwatch]</td>"
		    append body "<td>[$m cget -script]</td>"
		    append body "<td><span class=\"nobreak\">[$m stats]</span></td>"
		    if {$url ne ""} {
			append body "<td>[$m cget -log]</td>"
		    } else {
			append body "<td></td>"
		    }
		    append body "</tr>\n"
		}
		append body "</table>\n"
		append body "<br />\n"
	    }
	}
	set metascripts [::UTFD::metascript info instances]
	set watchlist {}
	set watchlist $::UTFD::restorewatchlist
	foreach metascript $metascripts {
	    if {[$metascript watch info] ne ""} {
		lappend watchlist $metascript
	    }
	}
	if {[llength $watchlist]} {
	    set rows {}
	    foreach row $watchlist {
		set e [$row timestamp -type end -format seconds]
		if {$e eq ""} {
		    set e [clock seconds]
		}
		lappend rows [list $row $e]
	    }
	    set rows [lsort -integer -index 1 $rows]
	    append body "<table summary=\"Watch listing\" class=\"TFtable\">"
	    append body "<thead>"
	    append body "<tr>"
	    append body "<th align=\"left\">WATCHING</th>"
	    append body "<th align=\"center\">PREV</th>"
	    append body "<th align=\"left\">RUNTIME</th>"
	    append body "<th></th>"
	    append body "<th align=\"center\">SCRIPT</th>"
	    append body "<th align=\"left\">STATS</th>"
	    append body "<th align=\"center\">LOG</th>"
	    append body "</tr>"
	    append body "</thead>"
	    foreach row $rows {
		set w [lindex $row 0]
		append body "<tr>"
		append body "<td>[$w cget -watch]</td>"
		if {[$w status] eq "noprevious"} {
		    append body "<td></td>"
		    append body "<td></td>"
		} else {
		    if {[set url [$w myurl]] eq ""} {
			append body "<td>[$w timestamp -type end -format date]</td>"
		    } else {
			append body "<td><span class=\"nobreak\"><a href=\"$url\">[$w timestamp -type end -format date]</a> <strong>[$w status -abbreviated]</strong></span></td>"
		    }
		    append body "<td>[$w timestamp -type end -format stopwatch]</td>"
		}
		append body "<td>$w</td>"
		append body "<td>[$w cget -script]</td>"
		append body "<td><span class=\"nobreak\">[$w stats]</span></td>"
		append body "<td>[$w cget -log]</td>"
		append body "</tr>\n"
	    }
	    append body "</table>\n"
	    append body "<br />\n"
	}
	if {[llength [array names ::UTFD::finished]]} {
	    if {$::UTFD::liveviewpage(date) eq ""} {
		set ::UTFD::liveviewpage(date) [clock format [clock seconds] -format "%Y.%m.%d"]
	    }
	    if {$::UTFD::liveviewpage(days) eq ""} {
		set ::UTFD::liveviewpage(days) 7
	    }
	    append body "<form name=\"liveview\" action=\"liveview.asp\" method=\"get\">\n"
	    append body "<a href=\"$saveurl\">Name</a>: <input type=\"text\" name=\"filename\" value=\"$::UTFD::liveviewpage(filename)\" /><input type=\"submit\" value=\"save\" name=\"button\" />"
	    append body "<table summary=\"Finished\" class=\"TFtable\">"
	    append body "<thead>"
	    append body "<tr>"
	    append body "<th align=\"left\">FINISHED</th>"
	    append body "<th align=\"left\">RUNTIME</th>"
	    append body "<th align=\"left\">&nbsp;SCRIPT&nbsp;&nbsp;&nbsp;String:&nbsp;<input type=\"text\" size=\"40\" name=\"filter\" value=\"$::UTFD::liveviewpage(filter)\" />Date: <input type=\"text\" name=\"date\" value=\"$::UTFD::liveviewpage(date)\" size=\"10\" />&nbsp;+<input type=\"text\" name=\"days\" value=\"$::UTFD::liveviewpage(days)\" size=\"1\" />day(s)<input type=\"submit\" value=\"Filter\" name=\"button\" /></th>"
	    append body "<th align=\"left\">STATS</th>"
	    append body "<th align=\"left\">Status</th>"
	    append body "<th align=\"left\">No</th>"
	    append body "</tr>"
	    append body "</thead>"
	    foreach index [lsort -integer -decreasing [array names ::UTFD::finished]] {
		foreach {count start runtime script stats status url failurl branch} $::UTFD::finished($index) {}
		if {$::UTFD::liveviewpage(filter) ne "" && ![regexp "$::UTFD::liveviewpage(filter)" $script]} {
		    continue
		}
		set t1 [split $::UTFD::liveviewpage(date) .]
		set t1 [clock scan [join [list [lindex $t1 1] [lindex $t1 2] [lindex $t1 0]] /]]
		set b1 [clock scan "-$::UTFD::liveviewpage(days) day" -base $t1]
		set b2 [clock scan "+1 day" -base $t1]
		set startseconds [clock scan $start]
		if {$startseconds < $b1 || $startseconds > $b2} {
		    continue
		}
		append body "<tr>"
		if {$url eq ""} {
		    append body "<td><span class=\"nobreak\">$start <strong>[string index $status 0]</strong></span></td>"
		} else {
		    append body "<td><span class=\"nobreak\"><a href=\"$url\">$start</a></span></td>"
		}
		append body "<td>$runtime</td>"
		set svnurl "http://${::env(HOSTNAME)}:${::UTFD::httpdport}/utfdsvn.cgi?rig=$rig&amp;script=[string map {" " %20}  $script]&amp;=$branch"
		append body "<td>$script</td>"
		if {$stats ne ""} {
		    append body "<td><span class=\"nobreak\">$stats</span></td>"
		} else {
		    append body "<td></td>"
		}
		if {$failurl ne ""} {
		    append body "<td align=\"center\"><a href=\"$failurl\"><strong>[string index $status 0]</strong></a></td>"
		} else {
		    append body "<td align=\"center\"><strong>[string index $status 0]</strong></td>"
		}
		append body "<td align=\"center\"><a href=\"$svnurl\"><strong>$count</strong></a></td>"
		append body "</tr>\n"
	    }
	    append body "</table></form>\n"
	    append body "<br />\n"
	}
	append body "</body>\n"
	append body "</html>\n"
    }
    proc finishedbody {thisbody {filter ""}} {
	upvar $thisbody body
	set rig [file rootname [lindex [file split $::utfconf] end]]
	set rigurl "http://${::env(HOSTNAME)}:${::UTFD::httpdport}"
	set body "<body>\n"
	set m $::UTFD::queues(running)
	set up [expr {[clock seconds] - $::UTFD::absuptime}]
	set relup [expr {[clock seconds] - $::UTFD::reluptime}]
	if {$m ne ""} {
	    set accum [expr {[clock seconds] - $::UTFD::launchtime}]
	    set adj $accum
	} else {
	    set accum 0
	}
	foreach q [lsort -integer [array names ::UTFD::timers]] {
	    incr accum $::UTFD::timers($q)
	}
	if {$accum} {
	    foreach q [lsort -integer [array names ::UTFD::timers]] {
		if {$m ne "" && $q eq $::UTFD::runningpri} {
		    append pritxt "[expr {round(100.0 * ($adj + $::UTFD::timers($q))/$accum)}]/"
		} else {
		    append pritxt "[expr {round(100.0 * $::UTFD::timers($q)/$accum)}]/"
		}
	    }
	} else {
	    set pritxt "0/0/0/0"
	}
	set pritxt [string trim $pritxt /]
	set relup [expr {[clock seconds] - $::UTFD::reluptime}]
	set idle [expr {$relup - $accum}]
	set idleperc [expr {round(100.0 * $idle/$relup)}]
	set accumperc [expr {round(100.0 * $accum/$relup)}]
	set up [::UTFD::tformat $up]
	set relup [::UTFD::tformat $relup]
	set idle [::UTFD::tformat $idle]
	set svnurl "http://${::env(HOSTNAME)}:${::UTFD::httpdport}/utfdsvn.cgi?rig=$rig"
	set saveurl "[string trim ${::UTFD::savewebserver} /][file join [::UTFD::metascript utfddir] saved]"
	append body "<h2><a href=\"$rigurl\">UTFD: $rig</a>"
	if {$::UTFD::pause(pending) && $m ne ""} {
	    append body " <small>(pause pending)</small>"
	}
	append body "   <small>(uptime:$up run/idle: $relup/$idle) (r/i: $accumperc/${idleperc}  1/2/3/4: $pritxt %) (Status a=aborted b=bounds c=completed e=error n=no previous run)</small><span class=\"right\">[clock format [clock seconds] -format [list %a %b %d %H:%M:%S]]&nbsp;<a href=\"$saveurl\"><small>saved</small></a></span></h2>\n"
	if {[llength [array names ::UTFD::finished]]} {
	    append body "<table summary=\"Finished\" class=\"TFtable\">"
	    append body "<thead>"
	    append body "<tr>"
	    append body "<th align=\"left\">FINISHED</th>"
	    append body "<th align=\"left\">RUNTIME</th>"
	    append body "<th align=\"center\">SCRIPT (filter=$filter)</th>"
	    append body "<th align=\"left\">STATS</th>"
	    append body "<th align=\"left\">Status</th>"
	    append body "<th align=\"left\">No</th>"
	    append body "</tr>"
	    append body "</thead>"
	    foreach index [lsort -integer -decreasing [array names ::UTFD::finished]] {
		foreach {count start runtime script stats status url failurl branch} $::UTFD::finished($index) {}
		if {$::UTFD::liveviewpage(filter) ne "" && ![regexp "$::UTFD::liveviewpage(filter)" $script]} {
		    continue
		}
		append body "<tr>"
		if {$url eq ""} {
		    append body "<td><span class=\"nobreak\">$start <strong>[string index $status 0]</strong></span></td>"
		} else {
		    append body "<td><span class=\"nobreak\"><a href=\"$url\">$start</a></span></td>"
		}
		append body "<td>$runtime</td>"
		set svnurl "http://${::env(HOSTNAME)}:${::UTFD::httpdport}/utfdsvn.cgi?rig=$rig&amp;script=[string map {" " %20} $script]&amp;branch=$branch"
		append body "<td>$script</td>"
		if {$stats ne ""} {
		    append body "<td><span class=\"nobreak\">$stats</span></td>"
		} else {
		    append body "<td></td>"
		}
		if {$failurl ne ""} {
		    append body "<td align=\"center\"><a href=\"$failurl\"><strong>[string index $status 0]</strong></a></td>"
		} else {
		    append body "<td align=\"center\"><strong>[string index $status 0]</strong></td>"
		}
		append body "<td align=\"center\"><a href=\"$svnurl\"><strong>$count</strong></a></td>"
		append body "</tr>\n"
	    }
	    append body "</table>\n"
	    append body "<br />\n"
	}
	append body "</body>\n"
	append body "</html>\n"
    }
    #
    #  UTFD scheduling below
    #
    proc enqueue {metascript args} {
	UTF::Getopts {
	    {type.arg "" "override the queue"}
	    {preempt "preempt the running script"}
	    {next "insert to be next (head of line)"}
	}

	if {[::UTFD::metascript info instances $metascript] ne $metascript} {
	    UTF::Message CTRLR ERROR "$metascript not a metascript"
	    error "$metascript enqueue failed"
	}
	set init_watch 0
	if {[catch {$metascript hash -cache} h]} {
	    UTF::_Message CTRLR WARN $h
	    set init_watch 1
	}
	if {$(type) eq ""} {
	    set type [$metascript cget -type]
	} else {
	    set type $(type)
	}
	if {![$metascript cget -allowdups]} {
	    foreach element $::UTFD::queues($type) {
		if {[$element hash -cache] eq $h} {
		    UTF::_Message CTRLR INFO "Duplicate enqueue ignored: $type [$metascript cget -script], use -allowdups to override"
		    return
		}
	    }
	}
	if {$init_watch && [$metascript cget -watch] ne ""} {
	    $metascript watch enable
	} else {
	    $metascript watch remove
	    $metascript  timestamp -type enqueue -set
	    if {$(preempt) || $(next)} {
		set msg "$metascript head-of-line"
		set ::UTFD::queues(now) [concat $metascript $::UTFD::queues(now)]
		if {$(preempt)} {
		    append msg "/preempt"
		    set ::UTFD::preempt 1
		}
	    } else {
		lappend ::UTFD::queues($type) $metascript
		set msg "$metascript $type"
	    }
	    UTF::Message CTRLR ENQUEUE $msg
	    eval [concat $metascript setstate "ENQUEUED"]
	    after 250 [list ::UTFD::utfdevent "queuechange metascript $msg"]
	}
    }
    proc dequeue {args} {
	UTF::Getopts {
	    {peek ""}
	}
	set next {}
	if {[llength $::UTFD::queues(now)]} {
	    set next [lindex $::UTFD::queues(now) 0]
	    if {!$(peek)} {
		set ::UTFD::queues(now) [lrange $::UTFD::queues(now) 1 end]
	    }
	} elseif {[llength $::UTFD::queues(cron)]} {
	    set next [lindex $::UTFD::queues(cron) 0]
	    if {!$(peek)} {
		set ::UTFD::queues(cron) [lrange $::UTFD::queues(cron) 1 end]
	    }
	} elseif {[llength $::UTFD::queues(triggered)]} {
	    set next [lindex $::UTFD::queues(triggered) 0]
	    if {!$(peek)} {
		set ::UTFD::queues(triggered) [lrange $::UTFD::queues(triggered) 1 end]
	    }
	} elseif {[llength $::UTFD::queues(background)]} {
	    set next [lindex $::UTFD::queues(background) 0]
	    if {!$(peek)} {
		set ::UTFD::queues(background) [lrange $::UTFD::queues(background) 1 end]
	    }
	}
	if {$next ne "" && !$(peek)} {
	    UTF::Message CTRLR DEQUEUE "$next [$next cget -type]"
	}
	return $next
    }
    proc rmenqueue {m} {
	foreach q [array names ::UTFD::queues] {
	    if {$q eq "running"} {
		continue
	    }
	    set ::UTFD::queues($q) [lsearch -not -all -inline $::UTFD::queues($q) $m]
	    $m setstate "DELETED"
	}
    }
    proc rmdups {m hash2check} {
	if {![$m cget -allowdups]} {
	    set q [$m cget -type]
	    foreach element $::UTFD::queues($q) {
		if {[$element hash -cache] eq $hash2check && ![$element cget -allowdups]} {
		    set ::UTFD::queues($q) [lsearch -not -all -inline $::UTFD::queues($q) $m]
		    UTF::Message CTRLR INFO "Duplicate hash: $m removed from $q [$m cget -script] $hash2check"
		    $m setstate "DELETED"
		}
	    }
	}
    }
    proc utfdevent {msg} {
 	UTF::Message CTRLR EVENT $msg
	if {$::UTFD::utfdnoreentrant} {
	    if {[catch {after info $::UTFD::kicker}]} {
		set ::UTFD::kicker [after 2000 [list ::UTFD::utfdevent "re-kick"]]
	    }
	    return
	}
	set ::UTFD::utfdnoreentrant 1
	# If in user mode, don't launch a new script
	if {$::UTFD::pause(pending) && ![llength $::UTFD::queues(running)]} {
	    ::UTFD::_dopause
	}
	if {[set peek [::UTFD::dequeue -peek]] ne "" && ![$peek ignorepause?] && $::UTFD::pause(enabled)} {
	    ::UTFD::savestate
	    set ::UTFD::utfdnoreentrant 0
	    return
	}
	if {[llength $::UTFD::queues(running)]} {
	    # If a script is currently running and it shouldn't
	    # be preempted, don't launch.  If preemption
	    # is needed do the killing now.  Note that
	    # script event handlers will re-signal this event
	    # handler as part of their close handling
	    # Note: lower value means higher priority
	    if {[set peek [::UTFD::dequeue -peek]] ne ""} {
		set peekpri [$peek mypriority]
		if {$peekpri < $::UTFD::runningpri && [$::UTFD::queues(running) cget -preemptable]} {
		    set crt [$::UTFD::queues(running) cget -committedruntime]
		    set s [$::UTFD::queues(running) timestamp -type start -format seconds]
		    set now [clock seconds]
		    if {[expr {($now - $s) > ($crt * 60)}]} {
			::UTFD::abort "Pre-emption due to $peekpri vs $::UTFD::runningpri"
		    } else {
			after [expr {(($crt * 60) - ($now - $s)) * 1000}] [list ::UTFD::utfdevent "re-kick"]
		    }
		} elseif {$::UTFD::preempt} {
		    set ::UTFD::preempt 0
		    ::UTFD::abort "Pre-emption due to user request"
		}
	    }
	    ::UTFD::savestate
	    set ::UTFD::utfdnoreentrant 0
	    return
	}
	# Rate limit how fast UTFD can launch metascripts
	# This is particularly useful with scripts that
	# error on the open of the pipe
	while {$::UTFD::launchratelimiter} {
	    vwait ::UTFD::launchratelimiter
	}
	set ::UTFD::launchratelimiter 1
	after $::UTFD::LAUNCHRATE [list set ::UTFD::launchratelimiter 0]

        # call to test generating function
        ::UTFD::intermediate_queuer

	# If were here then we're ok to dequeue and launch the next script
	if {[set ::UTFD::queues(running) [::UTFD::dequeue]] ne ""} {
	    if {[$::UTFD::queues(running) cget -watch] ne ""} {
		# Set the current hash now at script launch.  Set it even
		# if the script fails to launch so the watch will
		# be updated.
		#
		# Note: This hash can still become stale because the UTF load
		# comes later in the child script.  Ideally, a UTF load would
		# invoke an IPC callback into the daemon to pass in hash updates
		# used during the test script.  It would also need to pass in
		# the hash close.  All of this may be a bit too complicated
		# and a stale hash isn't terrible it just means that UTFD might
		# reschedule something that didn't change since the last run.
		#
		set h [$::UTFD::queues(running) hash -set]
		# Get rid of any dups
		::UTFD::rmdups $::UTFD::queues(running) $h
	    }
	    #
	    # If this is a compound script then launch them
	    # in order
	    #
	    set scripts {}
	    set suiteid [$::UTFD::queues(running) id]

	    set scriptline [string trim [$::UTFD::queues(running) cget -script] \;]
	    if {[set ix [lsearch $scriptline "-suiteid"]] ne "-1"} {
		incr ix
		if {[lindex $scriptline  $ix] eq "meta"} {
		    set scriptline [lreplace  $scriptline $ix $ix $suiteid]
		}
	    }
	    while {[set ix [string first \; $scriptline]] ne -1} {
		lappend scripts "[string range $scriptline 0 [expr {$ix - 1}]]"
		set scriptline [string trim [string range $scriptline [expr $ix +1] end]]
	    }
	    lappend scripts "$scriptline"
	    set ::UTFD::runningpri [$::UTFD::queues(running) mypriority]
	    $::UTFD::queues(running) timestamp -type end -clear
	    if {[set t [$::UTFD::queues(running) cget -maxruntime]] > 0} {
		set ::UTFD::mrt [after [expr {round($t * 3600000)}] [list eval ::UTFD::abort "$::UTFD::queues(running) exceeded $t hours"]]
	    }
	    foreach script $scripts {
		if {$script ne [lindex $scripts end]} {
		    set pending 1
		} else {
		    set pending 0
		}
		if {[catch {::UTFD::validatescript script} err]} {
		    if {$pending} {
			::UTFD::abort "Compound script errror $script"
		    } else {
			set ::UTFD::queues(running) {}
		    }
		    set ::UTFD::utfdnoreentrant 0
		    error $err
		}
		if {![regexp {2>} $script]} {
		    lappend script {2>@stdout}
		}
		if {![catch {open "|$script" r} fid]} {
		    set pid [pid $fid]
		    if {[set ix [lsearch $script -email]] ne "-1"} {
			set ::UTFD::email($pid) [lindex $script [expr {$ix + 1}]]
		    } elseif {[set ix [lsearch $script -webemail]] ne "-1"} {
			set ::UTFD::email($pid) [lindex $script [expr {$ix + 1}]]
		    } else {
			set ::UTFD::email($pid) $::env(USER)
		    }
		    set lf [$::UTFD::queues(running) cget -log]
		    if {[llength $scripts] > 1 && [$::UTFD::queues(running) cget -concurrent]} {
			set lf "[file rootname $lf].log.$pid"
		    }
		    set logfid [open $lf w]
		    fconfigure $logfid -buffering full -blocking 0
		    eval [concat $::UTFD::queues(running) setstate "RUNNING"]
		    UTF::Message CTRLR LAUNCH "$script ($pid) $fid $lf"
		    lappend ::UTFD::runningpids $pid
		    set ::UTFD::launchtime [clock seconds]
		    # Setup an event handler for  this script
		    fconfigure $fid -buffering line -blocking 0 -buffersize 4096
		    fileevent $fid readable [list ::UTFD::__script_handler $fid $logfid $::UTFD::queues(running) $pending]
		    if {$pending  && ![$::UTFD::queues(running) cget -concurrent]} {
			vwait ::UTFD::runningpids
		    }
		} else {
		    $::UTFD::queues(running) setstate "ERROR"
		    UTF::Message ERROR "" $fid
		    UTFD::abort -noemail "Subscript error"
		    break
		}
	    }
	}
	# save the state to persistent memory
	::UTFD::savestate
	set ::UTFD::utfdnoreentrant 0
    }
    # Abort all running jobs as gracefully as possible
    proc abort {args} {
	if {$::UTFD::noreentrant} {
	    return
	}
	set ::UTFD::noreentrant 1
	UTF::GetKnownopts {
	    {noemail ""}
	}
	set msg $args
	catch {after cancel $::UTFD::mrt}
	if {[array exists ::UTFD::watchdogs]} {
	    foreach wd [array names ::UTFD::watchdogs] {
		catch {after cancel $::UTFD::watchdogs($wd)}
	    }
	    array unset watchdogs *
	}
	if {$::UTFD::queues(running) ne ""} {
	    $::UTFD::queues(running) setstate "ABORT"
	}
	UTF::Message CTRLR EVENT "ABORT\($::UTFD::runningpids\): $msg"
	set abortedpids {}
	foreach pid $::UTFD::runningpids {
	    if {[catch {exec kill -s HUP $pid}]} {
		catch {exec kill -s KILL $pid}
	    }
	    lappend abortedpids $pid
	}
	set wd [after 2000 [list set ::UTFD::runningpids {}]]
	while {[llength $::UTFD::runningpids]} {
	    UTF::Sleep 0 quiet
	}
	# ***** RJM - should NOT need this ****  Look into why fsh sessions are hanging around
	if {$::UTFD::fshbug} {
	    catch {exec pkill fsh}
	}
	if {[catch {after cancel $wd}]} {
	    UTF::Message CTRLR ERROR "Some scripts ignoring SIGHUP and SIGKILL"
	}
	set ::UTFD::queues(running) ""
	catch {unset ::UTFD::statusreports(latest)}
	if {!$(noemail)} {
	    foreach pid $abortedpids {
		if {[info exists ::UTFD::logdir($pid)] && [info exists ::UTFD::email($pid)]} {
		    foreach EMAIL $::UTFD::email($pid) {
			::UTF::RecoverEndSummary $::UTFD::logdir($pid) $EMAIL
		    }
		}
	    }
	}
	set ::UTFD::noreentrant 0
    }
    proc validatescript {script} {
	upvar $script scriptline
	# All commands and scripts should set the utfconf file
	if {[set ix [lsearch $scriptline -utfconf]] eq "-1"} {
	    set scriptline [concat [lindex $scriptline 0] -utfconf $::UTF::args(utfconf) [lrange $scriptline 1 end]]
	} else {
	    set thisutfconf  [lindex $scriptline [expr {$ix + 1}]]
	    # Expand rig name by adding utfconf/ and .tcl
	    if {![file exists $thisutfconf] &&
		![regexp {/}  $thisutfconf] &&
		[file extension $thisutfconf] ne ".tcl"} {
		set thisutfconf "utfconf/${thisutfconf}.tcl"
	    }
	    if {![file exists $thisutfconf ]} {
		error "script needs a valid -utfconf $thisutfconf"
	    }
	}
	if {[lsearch $scriptline -email] eq "-1" && [lsearch $scriptline -webemail] eq "-1"} {
	    set scriptline [concat [lindex $scriptline 0] -webemail $::env(USER) [lrange $scriptline 1 end]]
	}
    }
    #
    #  UTFD event handlers below
    #
    proc __script_handler {fid logfid metascript pending} {
	set len [gets $fid buf]
	if {[eof $fid]} {
	    set pid [pid $fid]
	    set ::UTFD::runningpids [lsearch -not -all -inline $::UTFD::runningpids $pid]
	    if {![catch {set url "http://$::UTFD::webserver($pid)$::UTFD::logdir($pid)/"} err]} {
		$metascript myurl $url
	    } else {
		UTF::Message CTRLR WARN "No UTF Report from $pid per $metascript"
		$metascript myurl -clear
	    }
	    catch {unset ::UTFD::webserver($pid)}
	    if {[info exists ::UTFD::watchdogs($fid)]} {
		catch {after cancel $::UTFD::watchdogs($fid)}
		unset ::UTFD::watchdogs($fid)
	    }
	    set buf [read -nonewline $fid]
	    puts $logfid $buf
	    fconfigure $fid -blocking 1
	    if {[catch {close $fid} err]} {
		if {[lindex $::errorCode end] eq [catch error]} {
		    UTF::Message CTRLR HNDLR "$err $fid $::errorCode"
		    $metascript setstate "ERROR"
		}
	    }
	    close $logfid
	    # Kill the metascript if this is the last
	    if {![llength $::UTFD::runningpids] && (!$pending || [$metascript cget -concurrent])} {
		UTF::Message CTRLR EXEC "Script exit:$metascript $buf $fid"
		set runningtime [expr {[clock seconds] - $::UTFD::launchtime}]
		incr ::UTFD::timers($::UTFD::runningpri) $runningtime
		eval [concat $metascript setstate "COMPLETED"]
		if {[info exists ::UTFD::statusreports(latest)]} {
		    foreach {passcount failcount report} $::UTFD::statusreports(latest) {break}
		    # total will be nonzero as a null report won't be passed in
		    set total [expr {$passcount + $failcount}]
		    set id [$metascript id]
		    set failratio [expr {round(1000.0 * $failcount / $total) / 10.0}]
		    lappend ::UTFD::statusreports(total,$id) $total
		    lappend ::UTFD::statusreports(failratio,$id) $failratio
		    lappend ::UTFD::statusreports(failcount,$id) $failcount
		    if {[llength $::UTFD::statusreports(total,$id)] > 5} {
			foreach stat "total failratio failcount" {
			    set mean [expr {[::math::statistics::mean $::UTFD::statusreports($stat,$id)]}]
			    set stdev  [expr {[::math::statistics::stdev $::UTFD::statusreports($stat,$id)]}]

#			    UTF::Message CTRLR STATS "$stat=[set $stat] [format %0.3f $mean] [format %0.3f $stdev]"
			    if {$stdev > 0 && [expr {abs([set $stat] - $mean) > $stdev}]} {
				UTF::Message CTRLR BOUNDS "$stat=[set $stat] [format %0.3f $mean] [format %0.3f $stdev]"
				$metascript setstate "BOUNDSERR"
			    }
			}
		    }
		    # Store failure info for next comparison
		    set ::UTFD::statusreports(last,$id) $::UTFD::statusreports(latest)
		    unset ::UTFD::statusreports(latest)
		    set failratio [format %2.1f $failratio]
		} else {
		    set failcount "nan"
		    set failratio "nan"
		    set total "nan"
		}
		set ::UTFD::queues(running) [lsearch -not -all -inline $::UTFD::queues(running) $metascript]
		set ::UTFD::finished([clock seconds]) [list "$::UTFD::finishedcount" "[$metascript timestamp -type start -format date] \(${failcount}/$total ${failratio}%\)" "[$metascript timestamp -type end -format stopwatch]" "[$metascript cget -script]" "[$metascript stats]" "[string tolower [$metascript status]]" "[$metascript myurl]" "[$metascript myurl -failurereport]" "[$metascript mybranch]"]
		incr ::UTFD::finishedcount
		set ::UTFD::runningpri 99
		if {[$metascript status] eq "ERROR" && ![catch {open [$metascript cget -log] r} logfid]} {
		    while {![eof $logfid]} {
			if {[gets $logfid buf] > 0} {
			    UTF::Message CTRLR INFO $buf
			}
		    }
		    close $logfid
		}
		if {[set cb [$metascript cget -donecallback]] ne ""} {
		    eval $cb
		}
		catch {after cancel $::UTFD::mrt}
		after 40 [list ::UTFD::utfdevent "Check next"]
	    }
	} elseif {![fblocked $fid] && $len > 0} {
	    catch {after cancel $::UTFD::watchdogs($fid)}
	    if {[set t [$metascript cget -watchdog]] > 0} {
		set ::UTFD::watchdogs($fid) [after [expr {$t * 60000}] [list eval ::UTFD::abort "deadman timeout per $t minutes"]]
	    }
	    puts $logfid $buf
	    if {$::UTFD::debug} {
		UTF::Message UTFD HNDLR "$buf $fid"
	    }
	}
    }
    #
    #  UTFD "client code" to support remote submission, e.g cron jobs
    #  as well as client script to daemon commands
    #
    proc ipc {args} {
	UTF::GetKnownopts {
	    {type.arg "script" "IPC type"}
	}
	if {[info exists ::env(UTFDPORT)]} {
	    set ::UTFD::port $::env(UTFDPORT)
	}
	if {[catch {eval [concat socket localhost $::UTFD::port]} sid]} {
	    UTF::Message CTRLR IPC_ERR "port=$::UTFD::port $sid"
	    return
	}
	if {$::UTFD::debug} {
	    UTF::Message CLIENT "OPEN" "opened port $::::UTFD::port to UTF controller"
	}
	fconfigure $sid -blocking 1 -buffering full
	set buf [concat $(type) $args]
	puts $sid $buf
	flush $sid
	set wd [after 500 [list ::UTFD::wdclose $sid "Timeout sending command"]]
	close $sid
	if {$::UTFD::debug} {
	    UTF::Message CLIENT "SEND" "$buf $sid"
	}
	catch {after cancel $wd}
    }
    proc isChild? {} {
	if {![::UTFD::isSubmitter?] && ![::UTFD::isDaemon?] } {
	    return 1
	}
	return 0
    }
    proc isDaemon? {} {
	if {[::UTFD::isSubmitter?] } {
	    return 0
	} else {
	    return [expr {$::UTFD::UTFDPID == [pid]}]
	}
    }
    proc isSubmitter? {} {
	if {[info exists ::env(UTFDPORT)] && ![info exists ::env(UTFDPID)]} {
	    return 1
	} else {
	    return 0
	}
    }
    proc schedule? {} {
	if {[::UTFD::isSubmitter?]} {
	    # Don't submit to UTFD (allow collisions)
	    # if the user set -nolock
	    if {$::UTF::args(nolock)} {
		return 0
	    }
	    # See if this is a cron job request
	    # or a user initiated one
	    # use tty existence as proxy
	    if {[catch {exec tty -s}]} {
		set type cron
	    } else {
		set type now
	    }
	    ::UTFD::ipc -type script "::UTFD::metascript %AUTO% -script [list [concat $::argv0 $::__argv]] -type $type"
	    return 1
	} elseif {[::UTFD::isChild?]} {
	    set ::UTFD::logdir([pid]) ""
	    return 0
	}
    }
    proc TestRegister {logdir SessionID {webserver ""} {EMAIL ""}} {
	if {[::UTFD::isChild?]} {
	    if {[catch {::UTFD::ipc -type script "set ::UTFD::logdir([pid]) [file join $logdir $SessionID]"} err]} {
		UTF::Message ERROR IPC $err
	    }
	    if {[catch {::UTFD::ipc -type script "set ::UTFD::webserver([pid]) $webserver"} err]} {
		UTF::Message ERROR IPC $err
	    } elseif {$webserver ne $::UTFD::savewebserver} {} {
		set ::UTFD::savewebserver $webserver
	    }
	    if {$EMAIL ne {}} {
		if {[catch {::UTFD::ipc -type script "set ::UTFD::email([pid]) [list $EMAIL]"} err]} {
		    UTF::Message ERROR IPC $err
		}
	    }
	}
    }
    proc PostTestResults {Summary} {
	if {[::UTFD::isChild?]} {
	    set passcount [$Summary passcount]
	    set failcount [$Summary failcount]
	    if {[expr {($passcount + $failcount) > 0}]} {
		set cmd [list set ::UTFD::statusreports(latest) "$passcount $failcount [$Summary failreport]"]
		if {[catch {::UTFD::ipc -type script $cmd} err]} {
		    UTF::Message ERROR IPC $err
		}
	    }
	}
    }

    # start of build command
    # build win8x_internal_wl  BISON_BRANCH_7_10  465306
    proc build { buildbrand buildbranch svnrev } {
	global env
	#        set testsvnrev [$self cget -svnrev]
	#        set buildbranch [$self cget -tag]
	#        set buildbrand  [$self cget -brand]
	#        puts "svnrev = $svnrev"
	puts "buildbranch = $buildbranch"
	#   drillname=TRUNK_NIC_FC15
	#   drillname=BISON_BRANCH_7_10_NIC_FC15
	#   location=$baselocation/obj-apdef-stadef-debug-2.6.38.6-26.rc1.fc15.i686.PAE
	#   drillname=BISON_BRANCH_7_10_NIC_Win8X_3264_Chk_CIT
	#   drillname=BISON_BRANCH_7_10_NIC_FC15
	#   location=$baselocation/wdm/obj/win8x_nic/checked/x86
	# set svn rev:  fc15-29-BISON configure -svnrev 442587
	# build code: fc15-29-BISON build
	set drillname "TRUNK_NIC_FC15"
	set objpath "obj-apdef-stadef-debug-2.6.38.6-26.rc1.fc15.i686.PAE"
	#        set drillname "BISON_BRANCH_7_10_NIC_Win8X_3264_Chk"
	if {$buildbrand == "win8x_internal_wl"} {
	    if {$buildbranch == "BISON_BRANCH_7_10"} {
		set drillname "BISON_BRANCH_7_10_NIC_Win8X_3264_Chk"
		set objpath "wdm/obj/win8x_nic/checked/x86"
	    }
	}
	if {$buildbrand == "linux-internal-wl"} {
	    if {$buildbranch == "BISON_BRANCH_7_10"} {
		set drillname "BISON_BRANCH_7_10_NIC_FC15"
		set objpath "obj-apdef-stadef-debug-2.6.38.6-26.rc1.fc15.i686.PAE"
	    }
	    if {$buildbranch == "EAGLE_BRANCH_10_10"} {
		set drillname "EAGLE_BRANCH_10_10_NIC_FC15"
		set objpath "obj-apdef-stadef-debug-2.6.38.6-26.rc1.fc15.i686.PAE"
	    }
	    if {$buildbranch == ""} {
		set drillname "TRUNK_NIC_FC15"
		set objpath "obj-apdef-stadef-debug-2.6.38.6-26.rc1.fc15.i686.PAE"
	    }

            # add error condition here. use switch block
	}
	puts "Building :drill: SVN $svnrev for $drillname"
	#        if {$testsvnrev == 0} {
	#            set baselocation [drill -b $buildbranch -a -e $drillname | grep drill-savs | head -c -2 | head -c +4]
	#            puts "baselocation svnrev ? is $baselocation"
	#        } else {
	set baselocation [drill -r $svnrev -b $buildbranch -a -e $drillname | grep drill-savs | head -c -2]
	puts "baselocation svnrev $svnrev is $baselocation"
	#        }
	set location $baselocation/$objpath
	puts "location is $location"
    }
    # end of build command

    proc rig_job_count {{debug_rig_count 0}} {
        set count [expr [llength $::UTFD::queues(now)] + [llength $::UTFD::queues(running)] + \
                        [llength $::UTFD::queues(cron)] + [llength $::UTFD::queues(triggered)]  + \
                        [llength $::UTFD::queues(background)]]
        if {$debug_rig_count != 0} {
            UTF::Message CTRLR BLDPROD "\nreturning rig job count is $count"
            UTF::Message CTRLR BLDPROD "now is [llength $::UTFD::queues(now)]"
            UTF::Message CTRLR BLDPROD "running is [llength $::UTFD::queues(running)]"
            UTF::Message CTRLR BLDPROD "cron is [llength $::UTFD::queues(cron)]"
            UTF::Message CTRLR BLDPROD "triggered is [llength $::UTFD::queues(triggered)]"
            UTF::Message CTRLR BLDPROD "background is [llength $::UTFD::queues(background)]"
        }
        return $count
    }


    proc build_svn_list { inputlist {debug_svn_list 0}} {
        # Split into svnrecs on newlines
        set svnrecords [split $inputlist "\n"]

        # Iterate over the records
        set idx 0
        foreach svnrec $svnrecords {
            # Get SVN numbers
            set firstchar [string index $svnrec 0]
            if {$firstchar != "r"} {
                continue
            }

            set  svnnum [string range $svnrec 1 6]
            set  ::UTFD::svn_array_new($idx) $svnnum
            if {$debug_svn_list != 0} {
                UTF::Message CTRLR BLDPROD "::UTFD::svn_array_use $idx is $::UTFD::svn_array_new($idx)"
            }
            incr idx
        }

        UTF::Message CTRLR BLDPROD  "array size is [array size ::UTFD::svn_array_new]"
    }

    # intermediate_queuer global variables
    variable svn_array_use
    variable svn_array_save
    variable svn_array_new
    variable svn_list
    variable svn_list_size
    variable build_result_location      ""
    variable STAindex_count             0
    variable intermediate_script_count  0
    variable max_STAindex_count         0
    variable stop_build_producer        0
    variable max_testbuild_age         48
    
    
    proc intermediate_queuer { {debug_queuer 0} } {
        # if config file doesn't have max_STAindex_count set, just exit out
        if {$::UTFD::max_STAindex_count == 0} {
            return
        }
        
        set rigstatus [rig_job_count]
        # rigstatus returns number of jobs scheduled on the rig
        if {$rigstatus > 1} {
            UTF::Message CTRLR QUEUER "Current job count is $rigstatus.  Rig is busy.  Exiting."
            return
        } else {
            UTF::Message CTRLR QUEUER "Current job count is $rigstatus."
        }

        set i $::UTFD::STAindex_count
        
        set sta_list($i) $::UTFD::intermediate_sta_list($i)
        set buildbranch($i) [$sta_list($i) cget -tag]
        set buildbrand($i)  [$sta_list($i) cget -brand]
        if {$buildbranch($i) != "trunk"} {
            set buildbranch($i) "[string toupper [$sta_list($i) cget -tag]]"
        } else {
            set buildbranch($i) "TRUNK"
        }
        set ap($i)    $::UTFD::intermediate_ap
        
        # open file of test builds to find one that we want
        set filep [open $::UTFD::driver_build_path r]
        set file_data [read $filep]
        close $filep

        set svnfound 0
        set datalines [split $file_data "\n"]
        foreach line $datalines {
            ## Split into fields on colons
            set fields [split $line :]

            ## Assign fields to variables and print some out...
            lassign $fields osname branchloc svnnum binpath
            if {$debug_queuer} {
                UTF::Message CTRLR QUEUER "$osname $branchloc $svnnum $binpath"
            }
            
            if {$osname != $::UTFD::intermediate_sta_OS($i)} {
                if {$debug_queuer} {
                    UTF::Message CTRLR QUEUER "Failed osname check"
                }
                continue
            }

            if {$branchloc != $buildbranch($i)} {
                if {$debug_queuer} {
                    UTF::Message CTRLR QUEUER "Failed branchloc check  i is $i  branch is $buildbranch($i)"
                }
                continue
            }

            # skip old builds (more than two days old)
            set binparts [split $binpath ,]
            lassign $binparts pathinfo dateinfo
            set dateval [string range $dateinfo 0 7]
            set timeval [string range $dateinfo 9 12]
            if {$debug_queuer} {
                UTF::Message CTRLR QUEUER "path split $pathinfo  $dateinfo  dateval is $dateval"
            }
            set nowtime [clock scan now]
            set clocknowdate [clock format $nowtime -format {%Y%m%d.%H%M}]
            set clocknowtime [clock format $nowtime -format {%H%M}]
            if {$debug_queuer} {
                UTF::Message CTRLR QUEUER "clock date right now is $clocknowdate"
            }
            set diffdate [expr $clocknowdate - $dateval]
            set build_age [expr $::UTFD::max_testbuild_age / 24]
            if {$diffdate > $build_age} {
                if {$debug_queuer} {
                    UTF::Message CTRLR QUEUER "Skipping this entry - $dateval"
                }
                if {$svnnum == $::UTFD::intermediate_last_svn($i)} {
                    set ::UTFD::intermediate_last_svn($i) 0
                }
                continue
            }

            # if time of build is later than current time, it fits with the range, else too old
            if {$diffdate == $build_age} {
                set difftime [expr $clocknowtime - $timeval]
                if {$difftime > 0} {
                    if {$svnnum == $::UTFD::intermediate_last_svn($i)} {
                        set ::UTFD::intermediate_last_svn($i) 0
                    }
                    continue
                }
            }

            # if we found the previous one, the next one (this one) is the one to use
            if {$svnfound == 1} {
                if {$debug_queuer} {
                    UTF::Message CTRLR QUEUER "Processing the next one for this branch"
                }
                set ::UTFD::intermediate_last_svn($i) $svnnum
                set svnfound 2
                break
            }

            # if no previous ones were processed, take the first one
            if {$::UTFD::intermediate_last_svn($i) == 0} {
                if {$debug_queuer} {
                    UTF::Message CTRLR QUEUER "Selecting first entry"
                }
                set ::UTFD::intermediate_last_svn($i) $svnnum
                set svnfound 2
                break
            }

            # found the last one processed.  Note it and find the next one.
            if {$::UTFD::intermediate_last_svn($i) == $svnnum} {
                if {$debug_queuer} {
                    UTF::Message CTRLR QUEUER "Found the last svnnum processed for this branch"
                }
                set svnfound 1
                continue
            }
        }

        # states:  0 means no builds found, 1 means previous build found, 2 means found one to test
        if {$svnfound != 2} {
            UTF::Message CTRLR QUEUER  "Current STA $::UTFD::STAindex_count has no available builds.  Exiting."
            ::UTFD::pause
        } else {
            set build_script_name "IntermediateTest0"
            # deleting IntermediateTest0 if it exists
            catch {::UTFD::$build_script_name destroy}
            UTFD::metascript $build_script_name -svn 0 -createonly 1

            UTF::Message CTRLR QUEUER  "STAindex_count is $::UTFD::STAindex_count and listitem is $svnnum"
            if {$debug_queuer != 0} {
                set build_result_location "Script testing"
                UTF::Message CTRLR QUEUER "cron/launch stanightly  Test/IntermediateTester.test -utfconf $::UTFD::rigname  -sta $sta_list($::UTFD::STAindex_count) -ap 4706/4360 -title 'MD04:IntermediateTester $::UTFD::intermediate_ap_name to Linux  [$sta_list($::UTFD::STAindex_count) cget -tag] 4360 STA SVN:$svnnum' -nocache -bin $binpath"
            } else {
                UTFD::$build_script_name configure -svn $svnnum -dut $sta_list($::UTFD::STAindex_count) -watch $sta_list($::UTFD::STAindex_count)
            
                if {$buildbrand($::UTFD::STAindex_count) == "win8x_internal_wl"} {
                # for win8, need to add x86 or x64 to the path
                    set osver [$sta_list($i) cget -osver]
                    if {$osver == "8164" || $osver == "864" || $osver == "1064"} {
                        set script_text "{Test/IntermediateTester.test -utfconf $::UTFD::rigname  -sta $sta_list($::UTFD::STAindex_count) -ap $ap($::UTFD::STAindex_count) -title {$::UTFD::rigname:IntermediateTester $::UTFD::intermediate_ap_name to $::UTFD::intermediate_sta_list($::UTFD::STAindex_count) [$sta_list($::UTFD::STAindex_count) cget -tag] SVN:$svnnum} -nocache -bin {-altsys $binpath/x64} -email hnd-utf-list}"
                    } else {
                        set script_text "{Test/IntermediateTester.test -utfconf $::UTFD::rigname  -sta $sta_list($::UTFD::STAindex_count) -ap $ap($::UTFD::STAindex_count) -title {$::UTFD::rigname:IntermediateTester $::UTFD::intermediate_ap_name to $::UTFD::intermediate_sta_list($::UTFD::STAindex_count) [$sta_list($::UTFD::STAindex_count) cget -tag] SVN:$svnnum} -nocache -bin {-altsys $binpath/x86} -email hnd-utf-list}"
                    }
                } else {
                    set script_text "{Test/IntermediateTester.test -utfconf $::UTFD::rigname  -sta $sta_list($::UTFD::STAindex_count) -ap $ap($::UTFD::STAindex_count) -title {$::UTFD::rigname:IntermediateTester $::UTFD::intermediate_ap_name to $::UTFD::intermediate_sta_list($::UTFD::STAindex_count) [$sta_list($::UTFD::STAindex_count) cget -tag] SVN:$svnnum} -nocache -bin $binpath -email hnd-utf-list}"
                }
                UTF::Message CTRLR QUEUER  "script_text is $script_text"

                set script_name "IntermediateTest_$::UTFD::intermediate_script_count"
                UTFD::metascript $script_name -script "$script_text" -type triggered
                                                                                                                                                                           
                incr ::UTFD::intermediate_script_count
            }
        }

        incr  ::UTFD::STAindex_count
        if {$::UTFD::STAindex_count >= $::UTFD::max_STAindex_count} {
            set ::UTFD::STAindex_count 0
        }
    }
    #end of intermediate_queuer




    # UTFD::build_producer [Win8x/FC15/FC19] [TRUNK/BISON05T_BRANCH_7_35/EAGLE_BRANCH_10_10] [win8x_internal_wl/linux-internal-wl] 465306
    proc build_producer { ostype buildbranch svnrev } {
	if {$buildbranch == "trunk"} {
	    set buildbranch "TRUNK"
	}
	UTF::Message CTRLR BLDPROD  "buildbranch = $buildbranch"
        # all drill names start with x_NIC
	set drillname "${buildbranch}_NIC"
	UTF::Message CTRLR BLDPROD  "drillname is $drillname"
        switch -exact -- $ostype {
            "Win8x" {
                #   drillname=BISON_BRANCH_7_10_NIC_Win8X_3264_Chk_CIT
                #   location=$baselocation/wdm/obj/win8x_nic/checked/x86
                if {$buildbranch == "TRUNK"} {
                    set drillname "${drillname}_Win8X_3264_Chk_ST"
                } else {
                    set drillname "${drillname}_Win8x_3264_Chk"
                }
                set objpath "wdm/obj/win8x_nic/checked"
            }
            "FC15" {
                if {$buildbranch == "EAGLE_BRANCH_10_10"} {
                    UTF::Message CTRLR BLDPROD  "ERROR: EAGLE builds are not supported yet!"
                    return
                }

                set drillname "${drillname}_${ostype}"
                # osinfo is result of uname -r
                set osinfo  "2.6.38.6-26.rc1.fc15.i686.PAE"
                set objpath "obj-apdef-stadef-debug-${osinfo}"
            }
            "FC19" {
                UTF::Message CTRLR BLDPROD "Error: FC19 builds are not supported yet!"
                return

                set drillname "${drillname}_${ostype}"
                # osinfo is result of uname -r
                set osinfo  "3.11.1-200.fc19.x86_64"
                set objpath "obj-apdef-stadef-debug-${osinfo}"
            }
            default {
                UTF::Message CTRLR BLDPROD "Error: $ostype builds are not supported."
            }
        }

        UTF::Message CTRLR BLDPROD "drillname is $drillname"
	UTF::Message CTRLR BLDPROD "objpath is $objpath"
        UTF::Message CTRLR BLDPROD "Building :drill: SVN $svnrev for $drillname with buildbranch of $buildbranch"
        set  baselocation [drill -r $svnrev -b $buildbranch -a -e $drillname | grep drill-savs | head -c -2]
	UTF::Message CTRLR BLDPROD "baselocation svnrev $svnrev is $baselocation"
	
        if {$baselocation != ""} {
            set build_result_location $baselocation/$objpath
    	    UTF::Message CTRLR INFO "build_result_location is $build_result_location"
            set producer_out "$ostype:$buildbranch:$svnrev:$build_result_location"
            UTF::Message CTRLR INFO "producer_out is $producer_out"
            
            # open the build list output file
            set filep [open $::UTFD::driver_build_path a+]
            # dump the info to the build list file
            puts $filep $producer_out
            close $filep
        } else {
            #set  build_result_location ""
            UTF::Message CTRLR BLDPROD "Buildlocation is null.  Nothing added to the file."
        }
    }

    # driver_build_producer
    #    This routine generates builds for other rigs to use for IntermediateTester (or StaNightly).
    #    It runs as an infinite loop, checking for more checkins on each buildbranch queue.  If all
    #    of the queues are empty, it pauses for 10 minutes.  Otherwise, it pauses 30 seconds after each build.
    #
    #    It determines which svn revision to build by getting a list of checkins for the time range
    #    bounded by max_testbuild_age, shifting it as time passes.  Two arrays are maintained for each
    #    buildbranch, with one array having the list of checkins, and the other having the list of checkins
    #    but for checkins that have been built, replacing the checkin number with a 0.
    #
    #    The svn to build is determined by treating the array as a bitmap to find the largest range
    #    of checkins that haven't been built, and selecting the middle checkin.
    #
    #    Once an svn number is chosen, the info is passed to the build_producer procedure to execute drill
    #    and store the resulting build path info into the testbuildlist file.
    #
    #    To execute driver_build_producer, python 2.7 is required (for drill) so FC15 or higher is needed.
    #
    proc driver_build_producer { {skipbuilds 0} {testing 0} } {
        set ::UTFD::max_STAindex_count 4
        set buildbranch(0) "trunk"
        set buildbranch(1) "trunk"
        set buildbranch(2) "BISON05T_BRANCH_7_35"
        set buildbranch(3) "trunk"
            
        set os_list(0)  "FC15"
        set os_list(1)  "Win8x"
        set os_list(2)  "Win8x"
        set os_list(3)  "FC19"
            
        for {set i 0} {$i < $::UTFD::max_STAindex_count} {incr i} {
            set svnloc($i) $::UTFD::svn_path_base/branches/$buildbranch($i)
            if {$buildbranch($i) == "trunk"} {
                set svnloc($i) $::UTFD::svn_path_base/$buildbranch($i)
            }
            set ::UTFD::svn_list_size($i) 0
        }

        set delay_time 30000

        array unset ::UTFD::svn_array_use
        array set ::UTFD::svn_array_use {}

        array unset ::UTFD::svn_array_save
        array set ::UTFD::svn_array_save {}

        set empty_branch_count 0
        set ::UTFD::STAindex_count  0
        while {1} {

            set clocknow [clock scan now]
            # 3600 seconds per hour
            set aged_time [expr $::UTFD::max_testbuild_age * 3600]
            set clockold [expr $clocknow - $aged_time]
            # clock old date is $aged_time hours ago
            set clockolddate [clock format $clockold -format {%Y-%m-%d %H:%M}]
            set clocknowdate [clock format $clocknow -format {%Y-%m-%d %H:%M}]

            UTF::Message CTRLR BLDPROD "clocknowdate is $clocknowdate  clockolddate is $clockolddate"

            #  svn log http://svn.sj.broadcom.com/svn/wlansvn/proj/trunk --quiet -r "{2015-03-16 10:50}:{2015-03-14 10:50}" | grep " 20"
            set biglistsvnrevs [svn log $svnloc($::UTFD::STAindex_count) --quiet -r "{$clockolddate}:{$clocknowdate}" | grep " 20"]

            array unset ::UTFD::svn_array_new
            array set ::UTFD::svn_array_new {}
            build_svn_list $biglistsvnrevs

# svn_array_new now contains current 48 hours of checkins
            # search _save to find out first common array entry
            set checkin_new_count  [array size ::UTFD::svn_array_new]
            set checkin_save_count $::UTFD::svn_list_size($::UTFD::STAindex_count)
            
            #set checkin_save_count [array size ::UTFD::svn_array_save($::UTFD::STAindex_count)]
            UTF::Message CTRLR BLDR "STAindex_count is $::UTFD::STAindex_count"
            UTF::Message CTRLR BLDR "new checkin count is $checkin_new_count"
            UTF::Message CTRLR BLDR "save checkin count is $checkin_save_count"

            if {$checkin_save_count != 0 && $checkin_new_count != 0} {
                # no need to pass through if no entries dropped from the start of the list
                if {$::UTFD::svn_array_save($::UTFD::STAindex_count,0) != $::UTFD::svn_array_new(0)} {
                    UTF::Message CTRLR BLDR "first save is $::UTFD::svn_array_save($::UTFD::STAindex_count,0)  first new is $::UTFD::svn_array_new(0)"
                    for {set i 0} {$i < $checkin_save_count} {incr i} {
                        # find first entry of array_new in array_save
                        if {$::UTFD::svn_array_save($::UTFD::STAindex_count,$i) != $::UTFD::svn_array_new(0)} {
                            continue
                        }

                        set basecnt 0
                        UTF::Message CTRLR BLDR "first entry of new list is $::UTFD::svn_array_new(0), which matches $i entry of old list $::UTFD::svn_array_save($::UTFD::STAindex_count,$i)"
                        for {set j $i} {$j < $checkin_save_count} {incr j} {
                            set ::UTFD::svn_array_save($::UTFD::STAindex_count,$basecnt) $::UTFD::svn_array_save($::UTFD::STAindex_count,$j)
                            set ::UTFD::svn_array_use($::UTFD::STAindex_count,$basecnt) $::UTFD::svn_array_use($::UTFD::STAindex_count,$j)
                            incr basecnt
                        }

                        # subtract out old builds
                        set checkin_save_count [expr $checkin_save_count - $i]
                        break
                    }
                    UTF::Message CTRLR BLDPROD "after shifting, savecount is $checkin_save_count"
                }

                if {$checkin_new_count > $checkin_save_count} {
                    for {set i $checkin_save_count} {$i < $checkin_new_count} {incr i} {
                        set ::UTFD::svn_array_save($::UTFD::STAindex_count,$i) $::UTFD::svn_array_new($i)
                        set ::UTFD::svn_array_use($::UTFD::STAindex_count,$i) $::UTFD::svn_array_new($i)
                        incr checkin_save_count
                    }
                }
                UTF::Message CTRLR BLDPROD "updated checkin count is $checkin_new_count"
                #record new list size
                set ::UTFD::svn_list_size($::UTFD::STAindex_count) $checkin_save_count
            } else {
                UTF::Message CTRLR BLDPROD "skipbuilds is $skipbuilds  checkin_save_count is $checkin_save_count"
                if {$checkin_save_count == 0 && $skipbuilds == 1} {
                    UTF::Message CTRLR BLDPROD "skipbuilds is $skipbuilds  checkin_save_count is $checkin_save_count  checkin_new_count is $checkin_new_count"
                    for {set i 0} {$i < $checkin_new_count} {incr i} {
                        set ::UTFD::svn_array_use($::UTFD::STAindex_count,$i) 0
                        set ::UTFD::svn_array_save($::UTFD::STAindex_count,$i) $::UTFD::svn_array_new($i)
                    }
                } else {
                    for {set i 0} {$i < $checkin_new_count} {incr i} {
                        set ::UTFD::svn_array_use($::UTFD::STAindex_count,$i) $::UTFD::svn_array_new($i)
                        set ::UTFD::svn_array_save($::UTFD::STAindex_count,$i) $::UTFD::svn_array_new($i)
                    }
                }

                set ::UTFD::svn_list_size($::UTFD::STAindex_count) $checkin_new_count
            }

            UTF::Message CTRLR BLDPROD "checkin_count is $::UTFD::svn_list_size($::UTFD::STAindex_count)"
         
            set usemin 0xff
            set usecnt 0
            set thismin 0xff
            set thiscnt 0
            set foundtoken 0

            for {set cnt 0} {$cnt < $::UTFD::svn_list_size($::UTFD::STAindex_count)} {incr cnt} {
                if {$::UTFD::svn_array_use($::UTFD::STAindex_count,$cnt) != 0} {
                    if {$thismin == 0xff} {
                        set thismin $cnt
                    }
                    set thismax $cnt
                    incr thiscnt
                } else {
                    if {$foundtoken == 0} {
                        set foundtoken 1
                        set printstring "cnt "
                    } else {
                        set printstring [concat $printstring ", "]
                        incr foundtoken
                    }
                    set printstring [concat $printstring "$cnt"]
                    if {$thiscnt > $usecnt} {
                        set usemin $thismin
                        set usemax $thismax
                        set usecnt $thiscnt
                    }
                    set thismin 0xff
                    set thiscnt 0
                }
                # put max of 10 checkins as zero per line
                if {$foundtoken == 10} {
                    set printstring [concat $printstring " is 0"]
                    UTF::Message CTRLR BLDPROD "$printstring"
                    set foundtoken 0
                }
            }
            if {$foundtoken != 0} {
                set printstring [concat $printstring " is 0"]
                UTF::Message CTRLR BLDPROD "$printstring"
            }
         
            if {$thiscnt > $usecnt} {
                set usemin $thismin
                set usemax $thismax
                set usecnt $thiscnt
            }
         
            # if nothing to process, exit without trying
            if {$usecnt != 0} {
                set empty_branch_count 0

                set svnbuildval [expr ($usemin + ($usecnt / 2))]
                UTF::Message CTRLR BLDPROD "use:  min $usemin max $usemax cnt $usecnt"
                UTF::Message CTRLR BLDPROD "this: min $thismin max $thismax cnt $thiscnt"
         
                UTF::Message CTRLR BLDPROD "svnbuildval is $svnbuildval so selectedsvn is $::UTFD::svn_array_use($::UTFD::STAindex_count,$svnbuildval)"
                set testsvn $::UTFD::svn_array_use($::UTFD::STAindex_count,$svnbuildval)
                set ::UTFD::svn_array_use($::UTFD::STAindex_count,$svnbuildval) 0
         
         
                UTF::Message CTRLR BLDPROD "STAindex_count is $::UTFD::STAindex_count and listitem is $::UTFD::svn_array_use($::UTFD::STAindex_count,$svnbuildval)"
                # this is where the build is done
                # build_producer writes to the log file, so returned info is not used ... just printed for info
                
                set build_result_location [build_producer $os_list($::UTFD::STAindex_count) $buildbranch($::UTFD::STAindex_count) $testsvn]
         
                UTF::Message CTRLR BLDPROD "build_result_location is $build_result_location"
            } else {
                # if all branches have nothing to build, delay one hour
                incr empty_branch_count
                if {$empty_branch_count >= $::UTFD::max_STAindex_count} {
                    set empty_branch_count 0
                    # wait for 10 mins until trying the next time around
                    set delay_time [expr 60000 * 10]
                }
            }
         
            incr  ::UTFD::STAindex_count
            if {$::UTFD::STAindex_count >= $::UTFD::max_STAindex_count} {
                set ::UTFD::STAindex_count 0
                set skipbuilds 0
            }
            
            UTF::Message CTRLR BLDPROD "Delaying [expr $delay_time / 1000] seconds...."
            after $delay_time
            set delay_time 30000
        }
   }
#end of driver_build_producer

}
# end of UTFD namespace


#
#  Metascript object below.  These are schedulable by UTFD
#  They basically contain the meta data and states such
#  that UTFD knows when to launch a traditional UTF script
#
snit::type ::UTFD::metascript {
    # Used for nightly build variation allowance, units of hours
    typevariable BUILDVARIANCE 4
    typevariable WATCHMIN 60
    typevariable ENQUEUEDELAY 0 ;# units minutes
    typevariable UTFDBACKUPS
    typevariable EXCEPTATTRIBUTES "-log -noload -donecallback -createonly"
    typevariable HASHWARNTIME 3000;# if hash time exceeds this display a warning message, units ms
    typemethod typepriority {qtype} {
	switch -exact $qtype {
	    "running" {
		set priority 0
	    }
	    "now" {
		set priority 1
	    }
	    "cron" {
		set priority 2
	    }
	    "triggered" {
		set priority 3
	    }
	    "background" {
		set priority 98
	    }
	    default {
		error "$qtype not valid"
	    }
	}
	return $priority
    }
    typemethod snit_hash {instance} {
	set optionlist [$instance info options]
	set token [md5::MD5Init]
	set opts {}
	foreach opt $optionlist {
	    lappend opts "$opt [$instance cget $opt]"
	}
	md5::MD5Update $token $opts
	set hash [md5::Hex [md5::MD5Final $token]]
	return $hash
    }
    typemethod hash2instance {hash} {
	foreach metascript [::UTFD::metascript info instances] {
	    if {[$metascript id] eq $hash} {
		return $metascript
	    }
	}
	return
    }
    typemethod utfddir {{directory ""}} {
	if {[info exists UTFDBACKUPS]} {
	    return $UTFDBACKUPS
	}
	if {$directory eq ""} {
	    set directory [file join $::UTF::SummaryDir .utfd]
	}
	if {![file exists $directory]} {
	    if {[catch {file mkdir $directory} res]} {
		UTF::Message CTRLR ERROR "UTFD: unable to make directory $directory"
		error "backup fail: can't make directory"
	    }
	}
	if {![file writable $directory]} {
	    UTF::Message CTRLR ERROR "UTFD: unable to write to directory $directory"
	    error "backup fail: can't write"
	} else {
	    UTF::Message CTRLR INFO "UTFD status directory set to $directory"
	    return [set UTFDBACKUPS $directory]
	}
    }
    option -watch -default {}
    option -script -readonly true
    option -log
    option -name
    option -key
    option -type -default ""
    option -watchinterval -type integer -default 180
    option -donecallback -default {}
    # inactivity watchdog, units is minutes
    option -watchdog -type integer -default 15
    option -maxruntime -default -1 ;# Maximum runtime, units are hours
    option -committedruntime -default 20 ;#Minimum runtime (crt) units are minutes
    option -preemptable -type boolean -default 0
    option -concurrent -type boolean -default 0
    option -noload -type boolean -default 0 -readonly 1
    option -period -default -1
    option -allowdups -type boolean -default 0 -readonly 1
    option -createonly -type boolean -default 0 -readonly 1
    option -svn -default {} ;#can be a list
    option -dut -default {} ;#can be a list
    option -enqueuedelay -type integer -default 0

    variable myhash
    variable hashage
    variable afterid
    variable mystate
    variable timestamps -array {}
    variable runtimes
    variable watchstart
    variable myurl
    variable laststatus
    variable ignorepause 0
    variable enqueuedelaytimer

    constructor {args} {
	set mystate "INIT"
	$self configurelist $args
	if {$options(-log) eq {}} {
	    if {$options(-name) ne {}} {
		$self configure -log [file join /tmp $options(-name).log]
	    } else {
		$self configure -log [file join /tmp [namespace tail $self].log]
	    }
	}
	if {$options(-type) eq ""} {
	    if {$options(-watch) eq ""} {
		set options(-type) "now"
	    } else {
		set options(-type) "triggered"
	    }
	}
	switch -exact $options(-type) {
	    "now" {
		set options(-watch) ""
		set options(-donecallback) "$self destroy"
	    }
	    "cron" {
		set options(-watch) ""
		set options(-donecallback) "$self destroy"
	    }
	    "triggered" {
		set options(-donecallback) "$self watch enable"
		if {$options(-period) eq "-1"} {
		    set options(-period) "24"
		}
		set options(-enqueuedelay) $ENQUEUEDELAY
	    }
	    "background" {
		set options(-donecallback) "::UTFD::enqueue $self"
                set options(-preemptable) 1
                set options(-committedruntime) 90
	    }
	    default {
		error "$type not of [array names ::UTFD::queues]"
	    }
	}
	if {$options(-watch) ne ""} {
	    if {$options(-watchinterval) < $WATCHMIN} {
		set options(-watchinterval) $WATCHMIN
		UTF::Message CTRLR WARN "Watch interval min is $WATCHMIN, overiding requested value"
	    }
	    if {$options(-dut) eq ""} {
		set options(-dut) $options(-watch)
	    }
	}
	if {$options(-maxruntime) > 0 && [expr {round(60 * $options(-maxruntime)) < $options(-committedruntime)}]} {
	    UTF::Message CTRLR WARN "$self: max $options(-maxruntime) hour(s) less than committed (crt) of $options(-committedruntime) minutes"
	    set options(-committedruntime) $options(-maxruntime)
	}
	set myurl {}
	set runtimes {}
	set laststatus "noprevious"
	if {!$options(-createonly)} {
	    if {!$options(-noload)} {
		$self enqueue
	    } elseif {$options(-watch) ne ""} {
		$self watch enable
	    }
	}
    }
    destructor {
	set options(-donecallback) {}
	$self watch remove
	if {$mystate eq "ENQUEUED"} {
	    ::UTFD::rmenqueue $self
	}
	if {$::UTFD::queues(running) eq $self} {
	    UTFD::abort
	    UTF::Sleep 1.0
	    set wait 10
	    while {$::UTFD::queues(running) eq $self && $wait > 0} {
		UTF::Sleep 1.0
		incr wait -1
	    }
	}
	catch {after cancel $enqueuedelaytimer}
    }
    method inspect {} {
	puts "STATE=$mystate"
	if {![catch {after info $afterid}]} {
	    puts "AFTERID=$afterid [$self watch info] s"
	}
	if {[info exists myhash]} {
	    puts "Watch hash=$myhash"
	}
	foreach index [array names timestamps] {
	    puts "$index [clock format $timestamps($index)]"
	}
	if {$mystate eq "RUNNING"} {
	    puts "$self timestamp -type end -format stopwatch"
	}
	puts "[$self myurl]"
    }
    method id {args} {
	set key $args
	foreach n [lsort [array names options]] {
	    if {[lsearch -exact $EXCEPTATTRIBUTES $n] == -1} {
		lappend key $n $options($n)
	    }
	}
	regsub -all {[/<>]} $key "." key
	set myid [::md5::md5 -hex $key]
	return $myid
    }
    method mypriority {} {
	return [::UTFD::metascript typepriority $options(-type)]
    }
    method ignorepause? {} {
	if {[info exists ::tcl_interactive] && $::tcl_interactive} {
	    return $ignorepause
	}
	return 0
    }
    method run {args} {
	UTF::Getopts {
	    {preempt ""}
	    {next ""}
	}
	$self watch remove
	if {$(preempt)} {
	    $self rm
	    ::UTFD::enqueue $self -type "now" -next -preempt
	} elseif {$(next)} {
	    $self rm
	    ::UTFD::enqueue $self -type "now" -next
	} elseif {$mystate ne "ENQUEUED"} {
	    ::UTFD::enqueue $self -type "now"
	} elseif {[$self mypriority] > [::UTFD::metascript typepriority now]} {
	    $self rm
	    ::UTFD::enqueue $self -type "now"
	} elseif {$mystate eq "ENQUEUED" && $options(-allowdups)} {
	    ::UTFD::enqueue $self -type "now"
	} else {
	    UTF::Message CTRLR INFO "$self already enqueued, run request ignored"
	}
	if {[info exists ::tcl_interactive] && $::tcl_interactive} {
	    set ignorepause 1
	} else {
	    set ignorepause 0
	}
	return
    }
    method rm {} {
	if {$mystate eq "ENQUEUED"} {
	    ::UTFD::rmenqueue $self
	}
	catch {after cancel $enqueuedelaytimer}
    }
    method status {args} {
	UTF::GetKnownopts {
	    {abbreviated ""}
	}
	if $(abbreviated) {
	    return [string tolower [string index $laststatus 0]]
	} else {
	    return $laststatus
	}
    }
    method myurl {args} {
	UTF::GetKnownopts {
	    {clear ""}
	    {failurereport ""}
	}
	set url {}
	if {$(clear)} {
	    set myurl ""
	} elseif {$(failurereport) && $myurl ne ""} {
	    set url "${myurl}fail.html"
	} elseif {$args ne {}} {
	    set myurl $args
	} elseif {$myurl ne ""} {
	    set url "${myurl}summary.html"
	}
	return $url
    }
    method mybranch {args} {
	if {[catch {[lindex $options(-watch) 0] cget -tag} branch] || $branch eq "NIGHTLY"} {
	    return ""
	} else {
	    return $branch
	}
    }
    method mybrand {args} {
	if {$args != ""} {
	    set buildbrand [$args cget -brand]
	} else {
	    puts "Unknown STA, so no brand returned."
	    set buildbrand ""
	}
	if {$buildbrand != "linux-internal-wl" && $buildbrand != "win8x_internal_wl"} {
	    set buildbrand ""
	}
	return $buildbrand
    }
    # metascript has details on STA
    #       set handle [rexec -async drill [$self mybrand] [$self mybranch] svnrev]
    #        set output [$handle wouldblock]
    #                        set output [$handle close]
    #   parse output to get file locations
    #
    # start of buildme method
    # UTFD::build fc15-041-BISON  465306     (if no svnrev is specified, use top of branch)
    method buildme { STAname {svnrev 0} } {
	set buildbranch [string toupper [$self mybranch $STAname]]
	set buildbrand  [$self mybrand $STAname]
	if {$buildbranch == ""} {
	    set buildbranch "TRUNK"
	}
	UTF::Message CTRLR BUILDME "buildbranch = $buildbranch"
	UTF::Message CTRLR BUILDME "brand is $buildbrand"
	set drillname "${buildbranch}_NIC"
	UTF::Message CTRLR BUILDME "drillname is $drillname"
	#       #   drillname=BISON_BRANCH_7_10_NIC_Win8X_3264_Chk_CIT
	#       #   location=$baselocation/wdm/obj/win8x_nic/checked/x86
	if {$buildbrand == "win8x_internal_wl"} {
	    set drillname "${drillname}_Win8X_3264_Chk_CIT"
	    set objpath   "wdm/obj/win8x_nic/checked/x86"
	}
	if {$buildbrand == "linux-internal-wl"} {
	    set drillname "${drillname}_FC15"
	    set osinfo  [$STAname uname -r]
	    set objpath "obj-apdef-stadef-debug-${osinfo}"
	}
	UTF::Message CTRLR BUILDME "objpath is $objpath"
	if {$svnrev == 0} {
	    UTF::Message CTRLR BUILDME "Building :drill: SVN (top) for $drillname"
	    set  baselocation [drill -b $buildbranch -a -e $drillname | grep drill-savs | head -c -2 | head -c +4]
	    UTF::Message CTRLR BUILDME "baselocation svnrev (top) is $baselocation"
	} else {
	    UTF::Message CTRLR BUILDME "Building :drill: SVN $svnrev for $drillname"
	    set  baselocation [drill -r $svnrev -b $buildbranch -a -e $drillname | grep drill-savs | head -c -2]
	    UTF::Message CTRLR BUILDME "baselocation svnrev $svnrev is $baselocation"
	    if {0} {
		#               set handle [rexec -async drill -r $svnrev -b $buildbranch -a -e $drillname | grep drill-savs | head -c -2 | head -c +4]
		#               set handle [rexec -async drill [$self mybrand] [$self mybranch] svnrev]
		UTF::Message CTRLR BUILDME "handle is $handle"
		set willblock [$handle wouldblock]
		UTF::Message CTRLR BUILDME "willblock is $willblock"
		set drilloutput [$handle close]
		UTF::Message CTRLR BUILDME "drill output is $drilloutput"
	    }
	}
	set build_result_location $baselocation/$objpath
	UTF::Message CTRLR BUILDME "build_result_location is $build_result_location"
    }
    method stats {} {
	set cnt [llength $runtimes]
	if {$cnt >=2 } {
	    set mean [expr {round([::math::statistics::mean $runtimes])}]
	    set stdev  [expr {round([::math::statistics::stdev $runtimes])}]
	    if {$cnt > 5 && [expr {abs([lindex $runtimes end] - $mean) > (3 * $stdev)}]} {
		return "<strong>[UTFD::tformat ${mean}]/[UTFD::tformat ${stdev}] (${cnt})</strong>"
	    } else {
		return "[UTFD::tformat ${mean}]/[UTFD::tformat ${stdev}] (${cnt})"
	    }
	}
    }
    method timestamp {args} {
	UTF::Getopts {
	    {set ""}
	    {clear ""}
	    {type.arg "" "type of timestamp"}
	    {format.arg "date" "time format specifier"}
	}
	if {$(clear)} {
	    catch {unset timestamps($(type))}
	} elseif {$(set)} {
	    set timestamps($(type)) [clock seconds]
	} elseif {![catch {set t1 $timestamps(start)}]} {
	    if {$mystate eq "RUNNING" || [catch {set t2 $timestamps(end)}]} {
		set t2 [clock seconds]
	    }
	    if {$(type) eq "end"} {
		set t $t2
	    } else {
		set t $t1
	    }
	    switch -exact $(format) {
		"date" { return [clock format $t -format "%a %b %d %H:%M:%S"] }
		"stopwatch" { return [::UTFD::tformat [expr {$t2 - $t1}]]}
		"seconds" { return $t}
		default { error "invalid format of $(format)"}
	    }
	}
    }
    method setstate {newstate} {
	if {$mystate ne "RUNNING" && $newstate eq "RUNNING"} {
	    set timestamps(start) [clock seconds]
	} elseif {$mystate eq "RUNNING" && $newstate ne "RUNNING"} {
	    set timestamps(end) [clock seconds]
	    set laststatus $newstate
	    if {$newstate eq "COMPLETED"} {
		lappend runtimes [expr {$timestamps(end) - $timestamps(start)}]
	    }
	} else {
	    set laststatus $newstate
	}
	set mystate "$newstate"
    }
    method mystate {} {
	return $mystate
    }
    method hash {args} {
	UTF::Getopts {
	    {set ""}
	    {reset ""}
	    {cache ""}
	}
	if {$(cache) && [info exists myhash]} {
	    return $myhash
	}
	if {$(reset)} {
	    set myhash {}
	    set hashage [clock seconds]
	    return
	}
	set rc "ok"
	set t1 [clock clicks -milliseconds]
	set token [md5::MD5Init]
	md5::MD5Update $token [$self id]
	foreach DUT $options(-watch) {
	    md5::MD5Update $token [::UTFD::metascript snit_hash $DUT]
	    if {[catch {$DUT findimages} image]} {
		UTF::Message CTRLR WARN "$self hash error: $DUT findimage, $image"
		set rc "error"
	    }
	    md5::MD5Update $token $image
	}
	set hash [md5::Hex [md5::MD5Final $token]]
	if {![info exists myhash] || $myhash ne $hash} {
	    set hashage [clock seconds]
	}
	if {$(set) || ![info exists myhash]} {
	    set myhash $hash
	}
	set hashtime [expr {([clock clicks -milliseconds] - $t1)}]
	if {$hashtime > $HASHWARNTIME} {
	    UTF::Message CTRLR WARN "$self hash took $hashtime ms"
	}
	return -code $rc $hash
    }
    method enqueue {args} {
	catch {after cancel $enqueuedelaytimer}
	if {$options(-enqueuedelay) > 0} {
	    set enqueuedelaytimer [after [expr {$options(-enqueuedelay) * 60 * 1000}] [list ::UTFD::enqueue $self]]
	} else {
	    ::UTFD::enqueue $self
	}
    }
    method {watch remove} {} {
	catch {after cancel $afterid}
    }
    method {watch enable} {} {
	if {$options(-watch) eq ""} {
	    return
	}
	set mystate "WATCHING"
	if {$::UTFD::pause(enabled)} {
	    if {[lsearch $::UTFD::restorewatchlist $self] < 0} {
		lappend ::UTFD::restorewatchlist $self
	    }
	    return
	}
	if {[catch {after info $afterid}]} {
	    # enqueue self if either the hash has changed
	    # or it's been period hours since the last start
	    # If the hash call returns a catch error just
	    # reenable the watch and issue a warning -
	    # hopefully, eventually, the findimages will
	    # find a build
	    set prev {}
	    set curr {}
	    if {[catch {$self hash -cache} prev] || [catch {$self hash -set} curr]} {
		UTF::Message CTRLR WARN "$self hash catch error $prev $curr"
		set watchstart [clock seconds]
		# TCL after timer events tend to bunch up.  Add some
		# randomness to the
		# watch interval breaking the events up a bit.
		set half [expr {$options(-watchinterval) / 2}]
		set timer [expr {1000 * (1 + $half + (int(rand()*$half)))}]
		set afterid [after $timer [mymethod watch enable]]
	    } elseif {$prev eq $curr && ![$self buildtimeout?]} {
		set watchstart [clock seconds]
		# TCL after timer events tend to bunch up.  Add some
		# randomness to the
		# watch interval breaking the events up a bit.
		set half [expr {$options(-watchinterval) / 2}]
		set timer [expr {1000 * (1 + $half + (int(rand()*$half)))}]
		set afterid [after $timer [mymethod watch enable]]
	    } else {
		catch {unset watchstart}
		if {[$self buildtimeout?]} {
		    ::UTFD::enqueue $self
		} else {
		    $self enqueue
		}
	    }
	}
    }
    method {watch info} {} {
	if {![catch {after info $afterid}]} {
	    return [expr {$options(-watchinterval) - ([clock seconds] - $watchstart)}]
	}
    }
    method buildtimeout? {} {
	# If it's been over period hours since the last start time and the watch
	# hash hasn't changed for period + BUILDVARIANCE hours return true. Basically causes
	# a reschedule of the same image on the periodic frequency while allowing for build variance
	if {$options(-period) > 0} {
	    if {[catch {set timestamps(start)}]} {
		set timestamps(start) [clock seconds]
	    }
	    return [expr {([clock seconds] - $timestamps(start)) > ($options(-period) * 3600) && ([clock seconds] - $hashage) > (($options(-period) + $BUILDVARIANCE) * 3600)}]
	} else {
	    return 0
	}
    }
    method save {fid} {
	puts $fid "<snittype>[$self info type]</snittype>"
	puts $fid "<snitid>[$self id]</snitid>"
	foreach n [lsort [array names options]] {
	    if {[lsearch -exact $EXCEPTATTRIBUTES $n] == -1} {
		puts $fid "<attribute>$n</attribute><value>$options($n)</value>"
	    }
	}
	flush $fid
    }
}
if {[info exists ::env(UTFDPORT)]} {
    set ::env(UTFD) 1
}
if {[info exists ::env(UTFD)] && ![info exists ::env(UTFDPORT)]} {
    set ::env(UTFDPORT) $::UTFD::port
}
