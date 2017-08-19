#!/bin/env utf

# UTF Framework Object Definition for the aeroflex variable attenuator
#
# $Id: 5816052c8c7cf28f9819f64c00ded93c497e6de3 $
# $Copyright Broadcom Corporation$
#
# Goals of this implementation are:
# o Timing determinate (predictable/repeatable/minimal variance)
# o Fast direct socket access
# o Realtime state maintained in UTF controller
# o Assumes event model usages of UTF
# o Supports multi-user of Aeroflex (can be disabled and discouraged)
# o Usage of app level timers to drive TCP/AF state
#   (vs. generic, global level kernel level timers)
# o Much better responsiveness during transient network outages
# o Supports retries at the object level
# o Option to -holdopen the socket (though usage is discouraged)
# o Complete -debug logging/messaging (when needed)
# o Exception detection and explicit exception reporting
# o Properly handle serial/488.2 resynch/tcp "flush"
# o Add support for Aeroflex inactivity timer

package provide UTF::AeroflexDirect 2.0

package require snit
package require UTF::doc
package require UTF::AttnGroup

UTF::doc {
    # [manpage_begin UTF::AeroflexDirect n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF AeroflexDirect}]
    # [copyright {2009 Broadcom Corporation}]
    # [require UTF::AeroflexDirect]
    # [description]
    # [para]

    # UTF::AeroflexDirect is a object that provides UTF test scripts the
    # ability to control various aspects of the aeroflex variable
    # attenuator

    # Normally another script will create a new object and then call
    # its methods to control/perform operations on the variable
    # attenuator

    # AeroflexDirect is currently called by Test/rvr.test

    # [list_begin definitions]
}

snit::type UTF::AeroflexDirect {

    typeconstructor {
	snit::integer attnval -min 0 -max 103
	snit::integer chanval -min 1
	snit::listtype chanvallist -type ${type}::chanval
    }

    UTF::doc {
        # [call [cmd UTF::AeroflexDirect] [arg name]
	#	[option -lan_ip] [arg address]
	#	[option -relay] [arg string]
	#	[option -group]
	#	  [arg {{Group1 {Channels ...}} {Group2 {Channels ...}}}]]

        # Create a new AeroflexDirect object. It will be used to control
	# various functionalities of the variable attenuator.
	# [list_begin options]

        # [opt_def [option -name] [arg name]]

	# Name of the variable attenuator.

	# [opt_def [option -lan_ip] [arg address]]

	# Host name or IP address to be used to contact the variable
	# attenuator This should be a backbone address, not involved
	# in the actual testing.  Defaults to [arg name].

	# [opt_def [option -relay] [arg Host]]

	# Relay host used to talk to the attenuator.  Defaults to
	# localhost.

	# [opt_def [option -group]
	#	  [arg {{Group1 {Channels ...}} {Group2 {Channels ...}}}]]

	# Defines group objects and their channel lists.

        # [list_end]

        # [para]
        # AeroflexDirect objects have the following methods:
        # [list_begin definitions]
    }

    pragma -canreplace yes
    option -lan_ip
    option -name
    option -group -configuremethod GroupOption
    option -power
    option -relay localhost
    option -debug -type boolean -default 0
    option -retries -type integer -default 3
    option -retrybackoff -type integer -default 100 ;#in ms
    # Note:  The setting of holdopen to true is HIGHLY DISCOURAGED
    # for a few reasons.
    # 1) It eliminates shared usage (though this is also discouraged)
    # 2) The per cmd performance benefits is ~1-2%
    # 3) Socket open/close failures are likely more informative
    #    to UTF/TCL than when TCP state is held by the kernel
    # 4) It should make exception detection, isolation, and recovery
    #    more timely
    option -holdopen -type boolean -default 0
    option -concurrent -type boolean -default 0
    option -cmdresptimeout -type integer -default 300 ;# timeout in ms
    option -draintimer -type integer -default 2000 ;# timeout in ms
    option -exceptionrecovery -type boolean -default 0
    option -silent -type boolean -default 0
    option -finack_delay -type integer -default 6 ;#ms before FIN-ACK
    option -lantronixserial -type boolean -default false

    # Flag to be set for infrastructure objects, not directly involved
    # in testing.  This is to make sure we don't reconfigure or reboot
    # them by accident.
    option -infrastructure -type boolean -default false

    variable grouplist ""
    variable sid
    variable cmdblock -array {}
    variable cmdid 100
    variable noreentrancy 0
    variable exceptblock -array {}
    variable version
    variable finack_wait 0

    method GroupOption {key val} {
	foreach g $grouplist {
	    catch {::$g destroy}
	}
	set grouplist ""
	foreach {group channels} $val {
	    UTF::AttnGroup ::$group -aflex $self -channels $channels
	    lappend grouplist $group
	}
    }

    destructor {
	# clean up child objects
	foreach g $grouplist {
	    catch {::$g destroy}
	}
    }

    constructor {args} {
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	if {$options(-lan_ip) eq ""} {
	    $self configure -lan_ip $options(-name)
	}
	# If the -lan_ip does not contain a port number, and is not a
	# local device, add the default port
	if {![regexp {:\d+$|/} $options(-lan_ip)]} {
	    append options(-lan_ip) ":10001"
	}
	set exceptblock(exceptiontime) ""
	set exceptblock(consecutive) 0
	set exceptblock(total) 0
	# Start with the assumption that the serial if
	# needs resynching
	set exceptblock(resynch_serial) 0
	set cmdblock(errcnt) 0
	set cmdblock(errconsecutive) 0
	set version ""
    }

    UTF::doc {
        # [call [arg name] [method createGroup] [arg groupName]
	#	  [arg channels]]

        # Great a group of channels.  [arg groupName] will be created
        # as a UTF::Aeroflex::Group object which can be used directly
        # in tests.
    }

    method createGroup {groupName channelsList} {
	chanvallist validate $channelsList
	UTF::AttnGroup ::$groupName -aflex $self -channels $channelsList
	lappend grouplist $groupName
    }

    UTF::doc {
        # [call [arg name] [method whatami]]

        # Return model information for the Attenuator hardware.
    }

    method whatami {} {
	set rc [$self sendcmd {*idn?}]
    }

    UTF::doc {
        # [call [arg name] [method reset]]

        # Reset the attenuator to its power-on state.
    }

    method reset {} {
	$self sendcmd "*rst;esr?"
    }

    method power {op} {
	if {$options(-power) eq ""} {
	    error "power $op: -power not configured"
	}
	eval [linsert $options(-power) 1 power $op]
    }

    method getChanAttn {channels} {
	set attncmd "attn? AT[join $channels {;attn? AT}]"
	set rc [$self sendcmd "${attncmd}"]
	if {!$options(-silent)} {
	    UTF::Message INFO $self "attn? : $rc"
	}
	return $rc
    }

    method setChanAttn {channels value} {
	if {[llength $channels] > 1} {
	    # For multiple channels, create a temporary group so that
	    # changes are synchronised.
	    set attncmd "group %% AT[join $channels {,AT}];attn %% $value"
	} else {
	    # For single channel, just act on that channel directly
	    # complete cmdline: set channel + attenuate + query
	    set attncmd "attn AT$channels $value"
	}
	if {!$options(-silent)} {
	    UTF::Message INFO $self "Setting attn to $value"
	}
	return [$self sendcmd "${attncmd}"]
    }

    method incr {channels stepsize} {
	if {$stepsize < 0} {
	    set stepsize [expr {abs($stepsize)}]
	    set incrcmd "decr"
	} elseif {$stepsize > 0} {
	    set incrcmd "incr"
	} else {
	    return
	}
	if {[llength $channels] > 1} {
	    # For multiple channels, create a temporary group so that
	    # changes are synchronised.
	    set attncmd "group %% AT[join $channels {,AT}]; stepsize %% $stepsize; $incrcmd %%"
	} else {
	    # For single channel, just act on that channel directly
	    # complete cmdline: set channel + attenuate + query
	    set attncmd "stepsize AT$channels ${stepsize}; $incrcmd AT$channels"
	}
	if {!$options(-silent)} {
	    UTF::Message INFO $self "Attn $incrcmd of $stepsize"
	}
	return [$self sendcmd "${attncmd}"]
    }

    method sendcmd {cmd} {
	# The commands must run to completion. If UTF or a UTF::Test
	# makes a reentrant call error out now.
	if {$noreentrancy} {
	    set msg "Reentrant call to AF not supported"
	    UTF::Message ERROR $self $msg
	    error $msg
	}
	set noreentrancy 1
	# Build the command with the appropriate 488.2 register
	# commands
	set cmd "*cls\;$cmd\;*esr?"
	set err 0
	# Lock the AF "semaphore" by opening the port.
	if {![info exists sid]} {
	    if {[catch {$self directopen} cmdresults]} {
		set err 1
	    }
	}
	# If there is any chance the 488.2 is out
	# of synch, synch it up now
	if {!$err && $exceptblock(resynch_serial)} {
	    if {[catch {$self __synch_serial_if} cmdresults]} {
		set err 1
	    }
	    # make sure the AF doesn't echo commands
	    catch {$self __sendcmd "ECH0 0"}
	}
	# If the synch worked, send the command which
        # blocks on the *esr? response
	if {!$err} {
	    set err [catch {$self __sendcmd $cmd} cmdresults]
	}
	# release the AF port to other users
	if {!$options(-holdopen)} {
	    catch {$self directclose}
	    catch {$self close_sshtunnel}
	}
	# Renable use of the command to others
	set noreentrancy 0
	if {$err} {
	    # Signal resynching of serial_if on *any*
	    # detected __sendcmd related error
	    set exceptblock(resynch_serial) 1
	    UTF::Message ERROR $self $cmdresults
	    error $cmdresults
	} else {
	    # Concurrency means another AF session could
	    # leave the serial_if in an out of synch state
	    # so force a resynch before the next command
	    # otherwise the cmd->resp mechanism keeps
	    # everything synch.
	    set exceptblock(resynch_serial) $options(-concurrent)
	    return $cmdresults
	}
    }
    # This is a royal pain but after playing with all of the AF
    # settings the only certain way to get the TCP/Serial pipeline
    # to a known synch'd state is to wait for the AF to close
    # the socket per its inactivity timer.
    method __synch_serial_if {} {
	if {$options(-debug)} {
	    UTF::Message INFO  $self "Synch UTF/[namespace tail $self] 488.2 serial interface"
	}
	# See if there are any stale responses
	set cmdblock(state) "DRAIN"
	set watchdog [after $options(-draintimer) [list set [myvar cmdblock(state)] "TIMEOUT"]]
	while {1} {
	    if {$options(-debug)} {
		UTF::Message VWAIT  $self "Draining : timer (re)set to $options(-draintimer)"
	    }
	    vwait [myvar cmdblock]
	    switch -exact $cmdblock(state) {
		"DRAINING" {
		    after cancel $watchdog
		    set watchdog [after $options(-draintimer) [list set [myvar cmdblock(state)] "TIMEOUT"]]
		    set cmdblock(state) "DRAIN"
		}
		"CLOSED" {
		    after cancel $watchdog
		    if {$options(-debug)} {
			UTF::Message DEBUG  $self "Draining done"
		    }
		    if {[catch {$self directopen} err]} {
			set msg "Reopen failed after serial resynch"
			error $msg
		    }
		    break
		}
		"TIMEOUT" {
		    if {$version eq ""} {
			set msg "Drain failed due to lack of AF socket close"
			set version "A"
			set options(-draintimer) 1000
			UTF::Message WARN $self $msg
		    }
		    break
		}
	    }
	}
    }
    # Note; the socket should be open before this.
    method __sendcmd {cmd} {
	# Set this now and clear at the successful
	# completion of the command->resp.  This
	# way any exception will trigger the resynch
	set exceptblock(resynch_serial) 1
	set cmdblock(reqtime) [clock clicks -milliseconds]
	incr cmdid
	# Note, the cmd block response will be rewritten
	# in the afhandler per the response sent.  Preset
	# the values here.
	set cmdblock(cmdid) $cmdid
	set cmdblock(cmd) $cmd
	set cmdblock(state) "PENDING"
	set cmdblock(watchdog) ""
	set cmdblock(resp) "-1"
	set cmdblock(respcode) ""
	set watchdog [after $options(-cmdresptimeout) [list set [myvar cmdblock(watchdog)] "TIMEOUT"]]
	puts $sid $cmd
	flush $sid
	if {$options(-debug)} {
	    UTF::Message SEND $self "$cmd"
	}
	vwait [myvar cmdblock]
	if {$cmdblock(watchdog) eq "TIMEOUT"} {
	    set msg "Attenuator cmd response not received within $options(-cmdresptimeout) ms : Will resynch upon next cmd"
	    set debugstring "AF cmdblock($cmdblock(cmdid)) : "
	    foreach index [array names cmdblock] {
		append debugstring "${index}=: $cmdblock($index) "
	    }
	    UTF::Message DEBUG $self $debugstring
	    error $msg
	} else {
	    after cancel $watchdog
	}
	set results $cmdblock(resp)
	if {$cmdblock(state) eq "ERROR"} {
	    incr cmdblock(errcnt) +1
	    incr cmdblock(errconsecutive) +1
	} else {
	    set cmdblock(errconsecutive) 0
	}
	if {$options(-debug) || $cmdblock(state) eq "ERROR"} {
	    set debugstring "AF cmdblock($cmdblock(cmdid)) : "
	    foreach index [array names cmdblock] {
		append debugstring "${index}=: $cmdblock($index) "
	    }
	    catch {append debugstring " [expr {$cmdblock(resptime) - $cmdblock(reqtime)}] ms"}
	    UTF::Message DEBUG $self $debugstring
	}
	# clear the resp block in the event
	# the attenuator sends an unsolicated
	# response or closes the socket
	# per its inactivity timer.
	set cmdblock(resp) ""
	if {$cmdblock(state) eq "ERROR"} {
	    switch $cmdblock(respcode) {
		0 { set type "OK" }
		8 { set type "DDE: Device Dependent error" }
		16 { set type "EXE: Execution error" }
		32 { set type "CME: Command error" }
		default { set type "Unknown error code" }
	    }
	    set msg "Attenuator response indicates error : $type $cmdblock(respcode)"
	    error $msg
	} else {
	    return $results
	}
    }
    method __directopen_exception {errmsg} {
	# First, report and count the error
	incr exceptblock(consecutive) +1
	incr exceptblock(total) +1
	set currtime [clock clicks -milliseconds]
	if {$exceptblock(exceptiontime) ne ""} {
	    set exceptblock(exceptiontime) $currtime
	}
	UTF::Message ERROR $self "Open exception (total=${exceptblock(total)},consecutive=${exceptblock(consecutive)}) sock err: $errmsg"
	if {$options(-debug) || 1} {
	    if {[catch {eval [concat "localhost rexec /bin/netstat -npt | grep $options(-lan_ip)"]} err]} {
		UTF:Message ERROR $self "netstat err : $err"
	    } else {
		# UTF::Message DEBUG $self "netstat: $err"
	    }
	}
	# Now decide what to do
	if {$options(-exceptionrecovery)} {
	    UTF::Message EXCPT $self "NOT IMPLEMENTED YET"
	    set exceptblock(exceptiontime) ""
	}
    }
    method directopen {} {
	# Open the AeroflexDirect port.  Note that the AF
	# will only allow one socket open on this port,
	# i.e. it will not allow a subsequent open until any previous open
	# has closed, effectively acting as a semaphore for cmds.
	# This will work across scripts so the AF can be used
	# by multiple scripts running at the same time.
	set trycount 0
	while {$finack_wait} {
	    vwait [myvar finack_wait]
	}
	while {1} {
	    foreach {ip port} [split $options(-lan_ip) :] {}
	    set cmd "$options(-relay) socket $ip $port"
	    if {![catch {eval $cmd} results]} {
	        set sid $results
		set exceptblock(consecutive) 0
		fconfigure $sid -blocking 0 -buffering line
		fileevent $sid readable [mymethod __afhandler $sid]
		if {$options(-debug)} {
		    UTF::Message OPEN $self "UTF opened AF port"
		}
		set cmdblock(state) ""
		break
	    } else {
		incr trycount
		if {$trycount > $options(-retries)} {
		    set msg "Cannot open AF socket per $cmd"
		    error $msg
		}
		# To catch here or not is TBD, depending upon exception
		# handling
		catch {$self __directopen_exception $results}
		# Add some variability to the backoff in the
		# event the attenuator is being shared by
		# different UTF controllers
		set backoff [expr {int(rand() * $options(-retrybackoff)) + $options(-retrybackoff)}]
		UTF::Message WARN $self "retrying in $backoff ms"
		after $backoff
	    }
	}
    }
    method __afhandler {sockid} {
	#  A command response in the flush buffer may
	#  be corrupted and may not have new line
	#  terminator.  Use read instead of gets.
	if {$cmdblock(state) eq "DRAIN"} {
	    set buf [read -nonewline $sockid]
	    # Just set rc so partial AF response test
	    # is ignored.
	    set rc [string length $buf]
	} else {
	    set rc [gets $sockid buf]
	}
	if {$options(-debug)} {
	    UTF::Message HNDLR $self "bytes($rc) : $buf"
	}
	if {[eof $sockid]} {
	    if {$options(-debug)} {
		UTF:::Message DEBUG $self "Attenuator closed socket"
	    }
	    if {[catch {close $sockid} err]} {
		UTF::Message WARN $self $err
	    }
	    if {$rc < 0 && $cmdblock(state) eq "PENDING"} {
		UTF::Message ERROR $self "partial AF response of $cmdblock(resp) and eof"
	    }
	    unset sid
	    set cmdblock(state) "CLOSED"
	    set finack_wait 0
	    return
	}
	if {$cmdblock(state) eq "DRAIN"} {
	    set cmdblock(state) "DRAINING"
	    return
	}
	if {$rc >= 0} {
	    set currtime [clock clicks -milliseconds]
	    set cmdblock(resptime) $currtime
	    set cmdblock(resp) [split $buf \;]
	    set cmdblock(respcode) [lindex  $cmdblock(resp) end]
	    if {[llength $cmdblock(resp)] > 1} {
		set cmdblock(resp) [lrange  $cmdblock(resp) 0 end-1]
	    }
	    if {$cmdblock(respcode) eq "0"} {
		set cmdblock(state) "DONE"
	    } else {
		set cmdblock(state) "ERROR"
	    }
	}
    }
    method directclose {} {
	if {![info exists sid]} {
	    # AF already closed the socket
	    return
	}
	if {[catch {close $sid} err]} {
	    set msg "utf socket close error: $err"
	    UTF::Message ERROR $self $msg
	    unset sid
	    error $msg
	}
	# Give the kernel and AF 1 ms to complete their FIN, FIN ACK
	# This allows the AF to close its port and hang a new listen.
	# This is not perfect but it will decrease the probability
	# of a open/close/open causing an exception. If the close
	# fails despite this, directopen exception handling will kick in.
	#
	set finack_wait 1
	after $options(-finack_delay) [list set [myvar finack_wait] 0]
	unset sid
	if {$options(-debug)} {
	    UTF::Message CLOSE $self "UTF closed AF port"
	}
    }
    method setup {args} {
	if {$options(-lantronixserial)} {
	    eval $options(-relay) $UTF::unittest/bin/xportsetup \
		[lindex [split $options(-lan_ip) :] 0] -idletime 0:01 -flushmode 66
	} else {
	    set s [socket [lindex [split $options(-lan_ip) :] 0] 23]
	    puts $s {set tcp timeout 30}
	    puts $s {set tcp connect 1}
	    puts $s {set tcp echo off}
	    puts $s {reboot}
	    close $s
	    UTF::Sleep 15
	}
	return
    }
    UTF::doc {
	# [list_end]
    }
}

# Retrieve manpage from last object
UTF::doc [UTF::AeroflexDirect man]

UTF::doc {
    # [list_end]
    # [section EXAMPLES]
    # [example_begin]
    package require UTF::AeroflexDirect
    UTF::AeroflexDirect af -lan_ip 192.168.21.60 -relay UTFTestD -group {G {1 2 3}}
    G attn 45
    -> 45.00 45.00 45.00

    G attn?
    -> 45.00 45.00 45.00

    # [example_end]

    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
