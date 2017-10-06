#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::Base::Rexec 2.0

package require snit
package require UTF::doc

UTF::doc {
    # [manpage_begin UTF::Base::Rexec n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Rexec support}]
    # [copyright {2014 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::Base::Rexec implements an object supporting remote command execution.

    # [list_begin definitions]

}

snit::type UTF::Base::Rexec {
    option -base -readonly yes
    option -args -readonly yes

    # base handles any other options and methods
    component base

    variable name
    variable fd
    variable data ""
    variable Timer
    variable timer
    variable timeout
    variable before
    variable timed_out 0
    variable pargs -array {}

    constructor {args} {
	$self configurelist $args
	set base $options(-base)
	set name [$base cget -name]
    }

    UTF::doc {
	# [call [arg host] [method open]
	#               [lb][option -keepnewline][rb]
	#		[lb][option -timeout] [arg seconds][rb]
	#		[lb][option -Timeout] [arg seconds][rb]
	#               [lb][option -quiet][rb]
	#		    [cmd cmd] [arg {args ...}]]

	# Run [cmd cmd] [arg {args ...}] on [arg host].  Output and
	# errors are handled in the same way as [cmd exec].

	# [list_begin options]

	# [opt_def [option -async]]

	# Instead of blocking and returning results when completed
	# [option -async] causes [method open] to immediately return
	# the temporary [cmd UTF::Base::Rexec] object.  Calling the
	# object's [method close] method will block and return
	# results.  Multiple [option -async] processes can run
	# concurrently and their [method close] methods can be called
	# in any order.  The temporary object is destroyed
	# automatically when the close completes.

	# [opt_def [option -keepnewline]]

	# Retains a trailing newline in the pipeline's
	# output. Normally a trailing newline will be deleted.

	# [opt_def [option -timeout] [arg seconds]]

	# If no data is received in [arg seconds] the method will
	# attempt to kill the [cmd cmd] and return an error.  The
	# default timeout is 30 seconds.

	# [opt_def [option -Timeout] [arg seconds]]

	# If command has not completed in [arg seconds] the method will
	# attempt to kill the [cmd cmd] and return an error.  The
	# default Timeout is 300 seconds.

	# [opt_def [option -expect] [arg seconds]]

	# If command completed with no foreground or background
	# errors, no but took longer than [arg seconds] a background
	# error condition will be set.

	# [opt_def [option -desc] [arg description]]

	# When reporting errors, use [arg description] as the name of
	# the command.  Otherwise the entire commandline is reported.

	# [opt_def [option -quiet]]

	# Suppress logging of the command.  By default both the
	# command line and the results are reported to the test log.


	# [opt_def [option -silent]]

	# Suppress logging of the results of the command.  By default
	# both the command line and the results are reported to the
	# test log.

	# [opt_def [option -noinit]]

	# Bypass device initialization, such as setting up kernel
	# logging.  This is useful for maintenence commands which do
	# not expect to involve drivers.

	# [opt_def [option -binary]]

	# Preserve binary data in IO channel instead of TCL's default
	# text conversion.

	# [opt_def [option -in]]

	# Leave stdin open for input to the executing process.  By
	# default stdin is closed.

	# [opt_def [option -x]]

	# Ignore exit codes.  UTF (and TCL) use the Unix convention
	# that any command giving a non-zero exit code must be
	# indicating an error.  Some tools (expecially on Windows) use
	# the exit code for other purposes.  Ignoring the code in this
	# case is cleaner than adding extra "catch" statements to
	# handle the errors.

	# [opt_def [option -2]]

	# Leave stderr open.  By default stderr (2) is dup'ed onto
	# stdout (1).  This is because TCL interprets any stdout
	# messages as errors.

	# [opt_def [option -prefix] [arg prefix]]

	# Prepend [arg prefix] to all log messages.  This may be used
	# to identify different instances when running multiple rexecs
	# concurrently.

	# [opt_def [option -clock]]

	# Prepend timestamps to returned data.

	# [list_end]
    }

    method open {} {
	set rargs $options(-args)
	if {[catch {cmdline::typedGetoptions rargs {
	    {async "Return handle to be closed later"}
	    {keepnewline "Retain a trailing newline"}
	    {timeout.integer 30 "Seconds to timeout if no data received"}
	    {Timeout.integer 300 "Seconds to timeout if command not completed"}
	    {expect.integer 0 "Seconds to report failure, even if command completed"}
	    {desc.arg "" "Description"}
	    {quiet "Don't log commandline"}
	    {silent "Don't log results"}
	    {noinit "Don't run init code"}
	    {binary "Preserve binary data"}
	    {in "Don't close stdin"}
	    {x "Ignore exit codes"}
	    {2 "Don't dup 2 onto 1"}
	    {K "Don't retry (used by retries to prevent recursion)"}
	    {prefix.arg "" "Logging prefix, to identify concurrent streams"}
	    {clock "Add timestamps"}
	} "rexec options"} ret]} {
	    error $ret $ret
	} else {
	    array set pargs $ret
	}

	if {[info exists pargs(prefix)]} {
	    set pargs(prefix) [join $pargs(prefix) " "]
	} else {
	    set pargs(prefix) ""
	}
	if {![info exists pargs(desc)]} {
	    set pargs(desc) [join $rargs]
	}
	if {[info exists pargs(binary)]} {
	    set pargs(keepnewline) 1
	    set pargs(2) 1
	}

	# Scale command timout
	set Timeout [expr {$pargs(Timeout)*1000}]
	# Scale IO timout
	set timeout [expr {$pargs(timeout)*1000}]

	# Pass through to rpopen:
	foreach a {quiet noinit in 2} {
	    if {[info exists pargs($a)]} {
		set rargs [concat -$a $rargs]
	    }
	}

	if {[$base cget -rexec_add_errorcodes]} {
	    set rargs [concat -add_errorcodes $rargs]
	}

	# Start cmdline
	set fd [$base rpopen {*}$rargs]

	if {[info exists pargs(binary)]} {
	    fconfigure $fd -translation binary
	}

	set before [clock seconds]

	# Total timeout can't be less than IO timeout
	if {$Timeout < $timeout} {
	    set Timeout $timeout
	}

	# Start command timer
	set Timer [after $Timeout [mymethod kill]]

	# Set up nonblocking reader event
	fconfigure $fd -blocking 0
	fileevent $fd readable [mymethod getdata]

	# Start IO timer
	set timer [after $timeout [mymethod kill]]

    }

    UTF::doc {
	# [call [arg host] [method pid]]

	# Returns a list of pids for the processes involved in the command pipeline.
    }

    method pid {} {
	return [pid $fd]
    }

    method kill {} {
	set pids [pid $fd]
	UTF::Message LOG $name "Timeout: $pargs(desc): kill $pids"
	after cancel $timer; # clear io timer
	after cancel $Timer; # clear command timer

	set data $data
	# Timeouts need the connection to be rechecked, even if they
	# are only ping failures since the DUT may have crashed
	# during the ping.
	if {[$base cget -initialized] == 1} {
	    UTF::Message LOG $name "Reset connection"
	    $base configure -initialized 0
	}
	try {
	    exec kill {*}$pids
	} finally {
	    # Let reaper run unattended in the background.  We don't
	    # need to wait for it.
	    after 2000 [list catch [list exec kill -KILL {*}$pids]]
	}
    }

    method getdata {} {
	after cancel $timer; # clear io timer
	set msg [gets $fd]
	if {$msg ne "" && [info exists pargs(clock)]} {
	    set ret "[UTF::timestamp]  $msg"
	} else {
	    set ret $msg
	}
	if {[eof $fd]} {
	    # Remainder may be a partial line, or Null
	    append data $ret; # signal event, even if null
	    if {$msg ne "" && ![info exists pargs(silent)] &&
		([info exists ::UTF::debugrexec] ||
		 ![$base cget -rexec_add_errorcodes] ||
		 ![regexp {^@\d+@$} $msg])} {
		UTF::Message LOG $name "$pargs(prefix)$msg"
	    }
	    return
	}
	set timer [after $timeout [mymethod kill]]; # restart io timer
	if {![fblocked $fd]} {
	    append data "$ret\n"
	    if {$msg ne "" && ![info exists pargs(silent)] &&
		([info exists ::UTF::debugrexec] ||
		 ![$base cget -rexec_add_errorcodes] ||
		 ![regexp {^@\d+@$} $msg])} {
		UTF::Message LOG $name "$pargs(prefix)$msg"
	    }
	}; # else incomplete line
    }

    method timed_out {} {
	if {!$timed_out && ![eof $fd] && [catch {after info $timer; after info $Timer}]} {
	    set timed_out 1
	}
	set timed_out
    }

    method wouldblock {} {
	if {[regexp -line {^[^Z]} [exec /bin/ps -o state= -p {*}[pid $fd]]]} {
	    return 1
	} else {
	    return 0
	}
    }

    method async {} {
	info exists pargs(async)
    }

    UTF::doc {
	# [call [arg host] [method waitfor] [arg pattern]]

	# vwaits collecting results until the results match the regexp
	# specified by [arg pattern].  If [arg pattern] already
	# matches then no vwait is done.  This is useful for waiting
	# for async processes to start up.  The caller should still
	# call [method close] later to clean up.
    }

    method waitfor {pattern} {
	if {[regexp $pattern $data]} {
	    # Already found
	    return $data
	}
	while {![$self timed_out] && ![eof $fd]} {
	    vwait [myvar data]
	    if {[regexp $pattern $data]} {
		return $data
	    }
	}
	throw NotFound "pattern not found"
    }

    UTF::doc {
	# [call [arg host] [method close]]

	# vwaits collecting results until the process exists or times
	# out.  [method close] will return the process output or
	# error, depending on exit status.  The temporary [cmd
	# UTF::Base::Rexec] object will be destroyed.
    }

    method close {} {
	if {![info exists fd]} {
	    error "Not open"
	}
	while {![$self timed_out] && ![eof $fd]} {
	    vwait [myvar data]
	}
	after cancel $Timer; # clear command timer
	after cancel $timer; # clear io timer

	# If we got a timeout we can't trust the process to end in a
	# reasonable time even with SIGKILL, so leave the fd
	# nonblocking so the close won't hang.  The same goes if the
	# IO ended without a timeout but the process still didn't
	# enter zombie state in a reasonable time.  Only if the
	# process enters zombie state do we set nonblocking and
	# collect the return code.

	if {!$timed_out} {
	    # give the process a decent chance of being ready on the
	    # first shot.
	    after 20

	    # Allow processes up to 2 more seconds to go into zombie
	    # state
	    for {set i 20} {$i > 0} {incr i -1} {
		if {![$self wouldblock]} {
		    # No non-zombie processes left
		    fconfigure $fd -blocking 1
		    break
		}
		after 100; # blocks, but so would close
	    }
	    if {$i == 0} {
		UTF::Message WARN $name [exec /bin/ps u -p {*}[pid $fd]]
		UTF::Message WARN $name "$pargs(desc): failed to exit.  Killing [pid $fd]"
		exec kill -KILL {*}[pid $fd]
		# Leave fd non-blocking, for safety
	    }
	}
	set code [catch {close $fd} ret]
	set ec $::errorCode

	if {$timed_out} {
	    append data "Timeout"
	    UTF::Message LOG $name "Timeout"
	    set code 1
	} elseif {$code > 0 && $ret ne "child process exited abnormally"} {
	    append data $ret; # add stderr to the end of stdout
	    UTF::Message LOG $name $ret
	} else {
	    if {[$base cget -rexec_add_errorcodes]} {
		if {![regexp {(.*)@(\d+)@\n$} $data - data c] || $c ne 0} {
		    set code 1
		}
	    }
	    # trim newline from stdout if needed
	    if {![info exists pargs(keepnewline)]} {
		regexp {(.*)\n$} $data - data
	    }
	    set elapsed [expr {[clock seconds] - $before}]
	    if {$pargs(expect)} {
		# join is required because typedGetoptions forces args
		# into lists
		if {$timed_out} {
		    set expmsg "$pargs(desc) timed out after $elapsed sec"
		} else {
		    set expmsg "$pargs(desc) completed in $elapsed sec"
		}
		if {$elapsed > $pargs(expect)} {
		    UTF::Message FAIL $name $expmsg
		    if {![info exists ::UTF::panic]} {
			set ::UTF::panic $expmsg
		    }
		} else {
		    UTF::Message INFO $name $expmsg
		}
	    }
	}
	if {[info exists pargs(x)]} {
	    # Used for running commands that give bogus exit codes
	    set code 0
	}


	# Strip out adb startup messages for DK8.  Ok to log these,
	# but we don't want them in the returned data.
	regsub {\* daemon not running. starting it now on port \d+ \*\n\* daemon started successfully \*\n?} $data {} data


	# Only reinit if initialized normally.  Use init 2 to revent
	# recursion.
	# XXX Hack to reinstall ushd if the router crashes
	if {[$base cget -initialized] == 1 && \
		[regexp {: Connection refused|connect\(\): No route to host} $data]} {
	    UTF::Message LOG $name "Reset connection"
	    $base configure -initialized 0
	} elseif {![info exists pargs(K)] &&
		  [regexp {fsh tunnel lost|do_ypcall:|Connection reset by peer} $data] &&
		  [lsearch $options(-args) reboot] < 0} {
	    # Don't retry reboot commands!
	    if {[$base cget -initialized] == 1 && ![regexp {do_ypcall:} $data]} {
		# Don't reinit if it was just a transient NIS problem
		$base configure -initialized 0
	    }
	    UTF::Message LOG $name "Retrying"
	    set pargs(K) 1; # Prevent recusrion
	    set timed_out 0; # Reset state
	    set data ""; # Reset data
	    $self open; # reopen
	    return [$self close]
	}
	set d $data
	$self destroy
	return -code $code -errorcode $ec $d
    }

    method peek {} {
	return $data
    }
}

# Retrieve manpage from last object
UTF::doc [UTF::Base::Rexec man]

UTF::doc {
    # [list_end]

    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also wl]
    # [see_also [uri APdoc.cgi?UTF.tcl UTF]]
    # [see_also [uri APdoc.cgi?UTF::Cygwin.tcl UTF::Cygwin]]
    # [see_also [uri APdoc.cgi?UTF::Router.tcl UTF::Router]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
