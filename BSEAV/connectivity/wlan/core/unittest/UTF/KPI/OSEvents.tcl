#!/bin/env utf
#
#
# UTF object for wl event handling
#
# Written by: Robert J. McMahon May 2015
#
# $Copyright Broadcom Corporation$
#
#
package require snit
package require UTF

package provide UTF::KPI::OSEvents 2.0


#
# 	debug	Enable debug logging. A debug log setting may be enabled by prefixing it with a '+', and disabled
#		by prefixing it with a '-'.
#
#		AirPort Userland Debug Flags
#			DriverDiscovery
#			DriverEvent
#			Info
#			SystemConfiguration
#			UserEvent
#			PreferredNetworks
#			AutoJoin
#			IPC
#			Scan
#			802.1x
#			Assoc
#			Keychain
#			RSNAuth
#			WoW
#			P2P
#			Roam
#			BTCoex
#			AllUserland - Enable/Disable all userland debug flags
#
#		AirPort Driver Common Flags
#			DriverInfo
#			DriverError
#			DriverWPA
#			DriverScan
#			AllDriver - Enable/Disable all driver debug flags
#
#		AirPort Driver Vendor Flags
#			VendorAssoc
#			VendorConnection
#			AllVendor - Enable/Disable all vendor debug flags
#
#		AirPort Global Flags
#			LogFile - Save all AirPort logs to /var/log/wifi.log
#
namespace eval UTF::KPI {
    variable AIRPORTCMD "/usr/local/bin/airport"
    variable ADDAIRPORTOPTS "debug +AllUserLand +AllDriver +AllVendor +LogFile"
    variable RMAIRPORTOPTS "debug -AllUserLand -AllDriver -AllVendor -LogFile"
    variable RESTORECONSOLE
    array set RESTORECONSOLE {}
    proc enable_autojoin_osevents {dut} {
	variable RESTORECONSOLE
	if {[$dut hostis MacOS] && ![info exists RESTORECONSOLE($dut)]} {
	    if {[catch {$dut test -f $::UTF::KPI::AIRPORTCMD}]} {
		set ::UTF::KPI::AIRPORTCMD "/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport"
	    }
	    if {[catch {$dut rexec $::UTF::KPI::AIRPORTCMD [$dut cget -device] {*}$::UTF::KPI::ADDAIRPORTOPTS} err]} {
		UTF::Message ERROR $dut $err
	    } else {
		set logfile "/var/log/wifi.log"
		set current [$dut cget -console]
		if {[lsearch $current $logfile] < 0} {
		    $dut close_messages
		    set RESTORECONSOLE($dut) $current
		    $dut configure -console  [concat $current "/var/log/wifi.log"]
		    $dut open_messages
		}
		UTF::Message INFO $dut "Airport log enabled per -console [$dut cget -console]"
	    }
	}
    }
    proc disable_autojoin_osevents {dut} {
	variable RESTORECONSOLE
	if {[$dut hostis MacOS] && [info exists RESTORECONSOLE($dut)]} {
	    if {[catch {$dut rexec $::UTF::KPI::AIRPORTCMD [$dut cget -device] {*}$::UTF::KPI::RMAIRPORTOPTS} err]} {
		UTF::Message ERROR $dut $err
	    } else {
		$dut close_messages
		$dut configure -console $RESTORECONSOLE($dut)
		unset RESTORECONSOLE($dut)
		$dut open_messages
		UTF::Message INFO $dut "Airport log disabled per -console [$dut cget -console]"
	    }
	}
    }
}

snit::type UTF::KPI::OSEvents {

    option -sta -readonly true -validatemethod __validatesta
    option -name -default ""
    option -type -default apple80211_events -validatemethod __validatetype
    option -timeout -default 3
    option -device -default "awdl0"
    option -outfile -default ""
    option -tcpdump -default ""
    option -oldwl -type boolean -default 0
    option  -debug -type boolean -default 0

    variable pid
    variable state
    variable pid2fid -array {}
    variable watchdog
    variable utfmsgtag
    variable where
    variable netevent -array {}
    variable neteventstate -array {}
    variable prevmsgactions
    variable mytcpdump

    constructor {args} {
	$self configurelist $args
	if {$options(-name) eq ""} {
	    set utfmsgtag "[namespace tail $self ]"
	    set options(-name) $utfmsgtag
	} else {
	    set utfmsgtag $options(-name)
	}
	set state "INIT"
	switch $options(-type) {
	    "apple80211_events" {
		set where 80211
	    }
	    "bluetooth" {
		set where BLUEd
	    }
	    "netevents" {
		set where NETLNK
	    }
	    "dhdevents" {
		set where DHDEVENT
	    }
	    "apple80211_awdl" {
		set where 80211AWDL
	    }
	    "tcpdump_awdl" {
		set where TCPDUMPAWDL
	    }
	}
	if {$options(-tcpdump) ne {}} {
	    package require UTF::tcpdump
	    set mytcpdump [UTF::tcpdump %AUTO% -sta $options(-sta) -name [string toupper $options(-tcpdump)] -debug 0]
	}
    }
    destructor {
	if {[info exists mytcpdump]} {
	    $mytcpdump destroy
	}
	$self stop
    }
    method __validatetype {option value} {
	switch $value {
	    "apple80211_events" -
	    "netevents" -
	    "dhdevents" -
	    "bluetooth" {}
	    default {
		error "invalid type of $value"
	    }
	}
    }
    method __validatesta {option value} {
	if {$value eq ""} {
	    error "option -sta of $value cannot be empty"
	}
    }
    method status {} {
	UTF::_Message $where $utfmsgtag "State: $state"
	return [info exists pid]
    }
    method clear {} {
	catch {array unset netevent *}
	catch {array unset neteventstate *}
	set neteventstate(LINK) "UNK"
    }
    method apple80211_events? {} {
    }
    method start {args} {
	switch $options(-type) {
	    "apple80211_events" {
		if {$options(-oldwl)} {
		    set osverdir [join [lrange [split [$options(-sta) os_version] .] 0 1] .]
		    if {[catch {eval [concat $options(-sta) copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/wl/${osverdir}/wl /usr/bin/wl]}]} {
			eval [concat $options(-sta) copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/wl/10.10/wl /usr/bin/wl]
		    }
		}
		set remotecommand "/usr/bin/wl $options(-type)"
	    }
	    "apple80211_awdl" {
		if {$options(-outfile) eq ""} {
		    set options(-outfile) [file join /tmp ${options(-device)}.pcap]
		}
		set remotecommand "/bin/apple80211 $options(-device) -logf=\"v set\" -outfile=$options(-outfile) -dlog"
	    }
	    "tcpdump_awdl" {
		set remotecommand "/usr/sbin/tcpdump -i $options(-device)"
	    }
	    "bluetooth" {
		set remotecommand "/usr/sbin/blued"
	    }
	    "dhdevents" {
		# set remotecommand "/usr/local/bin/wl_kghosh -i [$options(-sta) cget -device] wl_event_check"
		set remotecommand "/usr/bin/wl -i [$options(-sta) cget -device] wl_event_check"
	    }
	    "netevents" {
		set remotecommand "/projects/hnd_sig_ext16/rmcmahon/Code/KPI/linux/netevents/netevents"
		set prevmsgactions [$options(-sta) cget -msgactions]
		set wlname [$options(-sta) wlname]
		# Caution "wlname" is probably correct most of the time, but I can't guarantee
		# it if virtual interfaces are involved.  Eg a driver function acting on wl1.2
		# might report messages from wl1.  Likewise it's possible a function acting on
		# the parent interface wl1 might be reported as wl1.0.  I think "link up" ought
		# to be ok, but you'll have to check it.
		set newmsgactions [lreplace $prevmsgactions 0 -1 \
				       "${wlname}: link up" \
                                         [subst {set [myvar netevent(LINKCHANGE_UP)] \
                                                     \[clock milliseconds\]
                                                     return 0
                                                 }]
                                  ]
		$options(-sta) configure -msgactions $newmsgactions
	    }
	}
	if {[catch {$options(-sta) rpopen -noinit $remotecommand} fid]} {
	    error $fid
	} else {
	    set pid [pid $fid]
	    set pid2fid($pid) $fid
	    set state "START"
	    $self clear
	    fconfigure $fid -blocking 0 -buffering line
	    fileevent $fid readable [mymethod __event_handler $fid]
	    set message "Started $options(-type)"
	    if {[info exists mytcpdump]} {
		append message " with tcpdump events for $options(-tcpdump)"
	    }
	    UTF::_Message $where $utfmsgtag "$message $fid"
	}
    }
    method stop {args} {
	if {[info exists prevmsgactions]} {
	    $options(-sta) configure -msgactions $prevmsgactions
	    unset prevmsgactions
	}
	if {![info exists pid]} {
	    return
	}
	if {[catch {exec kill -s INT $pid} err]} {
	    error $err
	}
	set state STOPPING
	UTF::_Message $where $utfmsgtag "Stopping $options(-type) $pid2fid($pid)"
	set watchdog [after [expr {$options(-timeout) * 1000}] [list set [myvar state] "TIMEOUT"]]
	while {$state ne "CLOSED"} {
	    vwait [myvar state]
	    if {$state ne "TIMEOUT"} {
		continue
	    } else {
		break
	    }
	}
	catch {after cancel $watchdog}
    }
    method __event_handler {fid} {
	set ms [clock milliseconds]
	set len [gets $fid buf]
	if {[eof $fid]} {
	    unset pid
	    set state CLOSED
	    UTF::_Message $where $utfmsgtag "Closed $options(-type) $fid"
	    close $fid
	}
	if {$len > 0} {
	    set starttcpdump 0
	    switch $options(-type) {
		"apple80211_events"  {
		    if {[regexp {APPLE80211_M_POWER_CHANGED} $buf]} {
			set netevent(POWER_CHANGED) $ms
		    } elseif {[regexp {APPLE80211_M_SCAN_DONE} $buf] && $neteventstate(LINK) ne "UP"} {
			set netevent(SCAN_DONE) $ms
		    } elseif {[regexp {APPLE80211_M_LINK_CHANGED link UP} $buf] && $neteventstate(LINK) ne "UP"} {
			set netevent(LINKCHANGE_UP) $ms
			set neteventstate(LINK) "UP"
			set starttcpdump 1
		    } elseif {[regexp {APPLE80211_M_LINK_CHANGED link DOWN} $buf] && $neteventstate(LINK) ne "DOWN"} {
			set netevent(LINKCHANGE_DOWN) $ms
			set neteventstate(LINK) "DOWN"
		    }
		}
		"dhdevents"  {
		    if {[regexp {APPLE80211_M_POWER_CHANGED} $buf]} {
			set netevent(POWER_CHANGED) $ms
		    } elseif {[regexp {WLC_E_SCAN_COMPLETE} $buf] && $neteventstate(LINK) ne "UP"} {
			set netevent(SCAN_DONE) $ms
		    } elseif {[regexp {WLC_E_LINK:LINK_UP} $buf] && $neteventstate(LINK) ne "UP"} {
			set netevent(LINKCHANGE_UP) $ms
			set neteventstate(LINK) "UP"
		    } elseif {[regexp {WLC_E_LINK:LINK_DOWN} $buf] && $neteventstate(LINK) ne "DOWN"} {
			set netevent(LINKCHANGE_DOWN) $ms
			set neteventstate(LINK) "DOWN"
		    }
		}
		"netevents" {
		    if {![info exists netevent(SCAN_DONE)]} {
			set netevent(SCAN_DONE) $ms
		    }
		    if {[regexp "Interface [$options(-sta) cget -device] link up" $buf]} {
			set netevent(LINK_CHANGED) $ms
		    }
		}
	    }
	    UTF::_Message $where $utfmsgtag $buf
	    if {$starttcpdump && [info exists mytcpdump]} {
		$mytcpdump start -$options(-tcpdump)
	    }
	}
    }
    method {waitfor linkdown} {args} {
	UTF::Getopts {
	    {timeout.arg "30" "Default timeout"}
	}
	if {$neteventstate(LINK) eq "DOWN"} {
	    UTF::_Message WARN $utfmsgtag "Waitfor linkdown called when link is already down"
	    return
	}
	if {[$options(-sta) hostis MacOS]} {
	    set dev [$options(-sta) cget -device]
	    if {[regexp "${dev}: +flags=(\[0-9A-F\]+)<" [$options(-sta) ifconfig] - flags]} {
		if {![expr 0x$flags & 0x20]} {
		    UTF::_Message WARN $utfmsgtag "Waitfor linkdown when device ($dev) does not have RUNNING flag set"
		    return
		}
	    }
	}
	set neteventstate(watchdog) "RUNNING"
	set aid [after [expr {$(timeout) * 1000}] [list set [myvar neteventstate(watchdog)] "TIMEOUT"]]
	while {$neteventstate(LINK) ne "DOWN"} {
	    UTF::_Message INFO $utfmsgtag "Waitfor linkdown event"
	    vwait [myvar neteventstate]
	    if {$neteventstate(watchdog) eq "TIMEOUT"} {
		UTF::_Message ERROR $utfmsgtag "Waitfor linkdown timeout"
		error "Waitfor linkdown timeout"
	    }
	}
	catch {after cancel $aid}
    }
    method gettime {index} {
	if {[info exists netevent($index)]} {
	    if {$options(-debug)} {
		set seconds [expr {$netevent($index) / 1000}]
		set r [expr {$netevent($index) % 1000}]
		set time "[clock format $seconds -format %T].[format %03d $r]"
		UTF::Message $utfmsgtag DEBUG "$index=$time"
	    }
	    return $netevent($index)
	} else {
	    if {$options(-debug)} {
		UTF::Message $utfmsgtag DEBUG "$index=UNK"
	    }
	    return "UNK"
	}
    }
}

