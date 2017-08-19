#!/bin/env utf
# -*-tcl-*-

#
# UTF Framework Object Definitions
# Based on snit
# $Id: ab470a8b1952a88a77b11ac622ce9222fb87dc0e $
# $Copyright Broadcom Corporation$
#

namespace eval UTF {}

# Hack for Tcl 8.5 compat - provide the version already indexed
if {[set v [package versions UTF]] ne ""} {
    package provide UTF [lindex $v 0]
} else {
    package provide UTF 2.0
}

# We're supposed to be commandline anyway, but avoid problems with
# "smart" tools, particularly on Cygwin where there's a good chance
# DISPLAY is set, but there's no X server running.
unset -nocomplain ::env(DISPLAY)

# Remove PWD to avoid confusing bash subprocesses if someone does a cd
# in tcl
unset -nocomplain ::env(PWD)

if {[regexp {CYGWIN|Windows} $::tcl_platform(os)]} {
    error "Running UTF directly on Cygwin is not supported"
    set ::env(HOME) p:
}

# Set up a suitable path for local overides
set auto_path [lreplace $auto_path 0 -1 .]


if {[info exists ::env(UTFCONFIG)]} {
    set utfconf $::env(UTFCONFIG)
} else {
    set utfconf ~/.utfconf.tcl
}
set UTF::options [subst {
    {sessionid.arg "" "Override session ID"}
    {utfconf.arg "$utfconf" "UTF configuration file"}
    {web "Wrap with web report"}
    {webtitle.arg "" "Report title"}
    {webemail.arg "" "Report email"}
    {websetup.arg "" "Setup code, to run before the main test"}
    {nolock "Bypass UTF::Q lock protection"}
}]
unset utfconf

if {[info command __package_orig] == "" && $::tcl_version > 8.4} {
    # tcl 8.5 is too strict about versioning.  Replace with more
    # lenient wrapper.

    rename ::package ::__utf_package_orig
    proc package {args} {
	if {![catch {::__utf_package_orig {*}$args} ret]} {
	    return $ret
	} elseif {[regexp {attempt to provide package (\S+) (\d+(?:\.\d+)?) failed: package (\S+) (\d+(?:\.\d+)?) provided instead} $ret - p1 v1 p2 v2]} {
	    puts stderr "WARNING: $ret"
	    package provide $p1 $v1
	    return $ret
	} else {
	    return -code error $ret
	}
    }
}

if {$tcl_version < 8.6} {
    # Additional language features
    package require trycatch 2.0
}

# UTF modules
package require UTF::doc
package require UTF::Cygwin
package require UTF::Linux
package require UTF::DHD
package require UTF::WinDHD
package require UTF::MacOS
package require UTF::Router
package require UTF::WebReport
package require UTF::Power
package require UTF::Multiperf


# Autoload
proc UTF::Q {args} {package require UTF::Q; UTF::Q {*}$args}

variable UTF::Logfile ""

# Select interactive mode only if
#   we are not buiding package indexes
#   we are called utf or UTF.tcl
#   we have no arguments
if {[info command __package_orig] == "" &&
    [lsearch {utf UTF.tcl} [file tail $argv0]] >= 0 &&
    $argv eq "" } {
    set tcl_interactive 1
}

UTF::doc {
    # [manpage_begin UTF n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {A framework for tests}]
    # [copyright {2005 Broadcom Corporation}]
    # [require UTF]

    # [description]
    # [para]
    # UTF is the main component of the HND Wireless Test Framework.

    # [list_begin definitions]

}

proc UTF::LoadUTFConf {} {
    # If UTF::doc contains anything but whitespace then we are running
    # in manpage mode, so don't bother to load configuration files.
    if {[regexp {^\s+$} [info body UTF::doc]]} {
	# By loading the config files inside a proc, unqualified
	# variables in the config files will be local, so will be
	# cleaned up automatically at the end of the proc.  The
	# namespace command will ensure that the created objects will
	# still be global.
	namespace eval :: {
	    # Load site config
	    package require utfconf

	    set utfconf $UTF::args(utfconf)
	    # Expand queue names by adding utfconf/ and .tcl
	    if {![file isfile $utfconf ] &&
		![regexp {/} $utfconf] &&
		[file extension $utfconf] ne ".tcl"} {
		set utfconf "utfconf/$UTF::args(utfconf).tcl"
	    }
	    # Load per-user config
	    if {[file isfile $utfconf]} {
		if {$tcl_interactive} {
		    UTF::Message INFO "" "Loading $utfconf"
		}
		source $utfconf
	    } else {
		puts stderr "Configuration file $utfconf not found.  Check UTFCONFIG"
		exit 1
	    }
	}
    }
}

proc bgerror {msg} {
    UTF::Message FAIL "bgerror:" $::errorInfo
}

UTF::doc {
    # [call [cmd UTF::color2ansicode] [arg color]]

    # return ansi color code
}
proc UTF::color2ansicode {color} {
    switch -exact $color {
	"black" {
	    set colorcode 30
	}
	"red" {
	    set colorcode 31
	}
	"green" {
	    set colorcode 32
	}
	"yellow" {
	    set colorcode 33
	}
	"blue" {
	    set colorcode 34
	}
	"magenta" {
	    set colorcode 35
	}
	"cyan" {
	    set colorcode 36
	}
	"white" {
	    set colorcode 37
	}
	"Black" {
	    set colorcode "01;30"
	}
	"Red" {
	    set colorcode "01;31"
	}
	"Green" {
	    set colorcode "01;32"
	}
	"Yellow" {
	    set colorcode "01;33"
	}
	"Blue" {
	    set colorcode "01;34"
	}
	"Magenta" {
	    set colorcode "01;35"
	}
	"Cyan" {
	    set colorcode "01;36"
	}
	"White" {
	    set colorcode "01;37"
	}
	default {
	    set colorcode 0
	}
    }
    return $colorcode
}

UTF::doc {
    # [call [cmd UTF::color_message] color]
    # wrap a message with ansi color codes
}

proc UTF::color_message {msg color} {
    set colorcode [UTF::color2ansicode $color]
    set msg "\033\[${colorcode}m$msg\033\[0m"
    return $msg
}

UTF::doc {
    # [call [cmd UTF::Message] [arg code] [arg where] [arg msg]]

    # Log a message to both standard output and the log file named by
    # UTF::Logfile.

    # A sequence number will be added to the output to help identify
    # failures.  [arg where] should identify which host or device
    # generated the message.  [arg code] should be one of the
    # following:

    # [list_begin definitions]

    # [def PASS]
    # The test succeeded, and was expected to succeed.

    # [def XPASS]

    # The test was expected to fail, but succeeded. This may
    # indicate progress; inspect the test case to determine
    # whether you should amend it to stop expecting failure.

    # [def FAIL]

    # The test failed, although it was expected to succeed. This
    # may indicate regress; inspect the test case and the failing
    # software to locate the bug.

    # [def XFAIL]

    # The test failed, but it was expected to fail. This result
    # indicates no change in a known bug. If a test fails because
    # the system where the test runs lacks some facility required
    # by the test, the outcome is UNSUPPORTED instead.

    # [def UNRESOLVED]

    # Output from a test requires manual inspection; the testsuite
    # could not automatically determine the outcome. For example,
    # your tests can report this outcome when a test does not
    # complete as expected.

    # [def UNTESTED]

    # A test case is not yet complete, and in particular cannot
    # yet produce a PASS or FAIL. You can also use this outcome in
    # dummy ``tests'' that note explicitly the absence of a real
    # test case for a particular property.

    # [def UNSUPPORTED]

    # A test depends on a conditionally available feature that
    # does not exist in the configured testing environment.

    # [list_end]

    # In addition, tests may cause the following types of
    # messages:

    # [list_begin definitions]

    # [def ERROR]

    # Indicates a major problem (detected by the test case itself)
    # in running the test. This is usually an unrecoverable error,
    # such as a missing file or loss of communication to the
    # target.
    # [def WARN]

    # Indicates a possible problem in running the test. Usually
    # warnings correspond to recoverable errors, or display an
    # important message about the following tests.

    # [def NOTE]
    # An informational message about the test case.

    # [list_end]

}

set UTF::_llogfile ""
set UTF::_llogfd ""
set UTF::_llogid ""

set UTF::_message_last_code ""
set UTF::_message_last_where ""
set UTF::_message_last_msg ""
set UTF::_message_last_count 0
set UTF::_message_last_time 0
set UTF::_message_last_timer ""

proc UTF::Message { code where msg } {

    # Basic rate limiting - suppress identical copies if received in
    # the same second.  Report the number of copies suppressed once a
    # second, or when a different message appears.

    set now [clock seconds]
    set msg_sav $msg
    if {$msg ne "" &&
	$code eq $UTF::_message_last_code &&
	$where eq $UTF::_message_last_where &&
	$msg eq $UTF::_message_last_msg} {
	set same 1
    } else {
	set same 0
    }

    if {$same && $now == $UTF::_message_last_time} {
	# Suppress repeated messages in the same second
	if {!$UTF::_message_last_count} {
	    # add a timer to flush queue on one second if nothing else
	    # flushes it first
	    set UTF::_message_last_timer [after 1000 [list UTF::Message $code $where ""]]
	}
	incr UTF::_message_last_count
	return
    } elseif {$UTF::_message_last_count > 0} {
	# Report message suppression
	UTF::_Message $UTF::_message_last_code $UTF::_message_last_where \
	    "$UTF::_message_last_msg (repeated $UTF::_message_last_count times)"
	set UTF::_message_last_count 0
	after cancel $UTF::_message_last_timer
    }
    if {$msg eq ""} {
	return
    }
    set UTF::_message_last_time $now

    if {!$same} {
	# Report new message
	set UTF::_message_last_code $code
	set UTF::_message_last_where $where
	set UTF::_message_last_msg $msg_sav
	UTF::_Message $code $where $msg
    }
}

UTF::doc {

    # [call [cmd UTF::timestamp]]

    # Returns a current timestamp for use in reports, etc.

    # [example_begin]
    utf> UTF::timestamp
    14:42:10.198
    # [example_end]
}

proc UTF::timestamp {} {
    if {[info exists UTF::MSTimeStamps]} {
	set ms [clock milliseconds]
	set seconds [expr {$ms / 1000}]
	set r [expr {$ms % 1000}]
	return "[clock format $seconds -format %T].[format %03d $r]"
    } else {
	clock format [clock seconds] -format "%T"
    }
}

UTF::doc {

    # [call [cmd UTF::timestamp2ms] [arg timestamp]]

    # Converts a timestamp generated by [cmd UTF::timestamp] into an
    # interger number of milliseconds.  The same format as could have
    # been output by [cmd {clock miliseconds}] at the time the stamp
    # was created.

    # [example_begin]
    utf> UTF::timestamp2ms 14:42:10.198
    1471556530198
    # [example_end]
}


proc UTF::timestamp2ms {timestamp} {
    if {![regexp {(.*)\.(\d\d\d)} $timestamp - timestamp fract]} {
	set fract 0
    }
    expr {1000 * [clock scan $timestamp -format %T] + $fract}
}

proc UTF::_Message { code where msg } {

    #
    # Parse the code parameter for any color attributes (which are
    # set by a "+<color>" string appended to it.)
    #
    # Note: See color2ansicode for the enumerations of <color>

    set ix [string last "+" $code]
    if {$ix != -1} {
	set color [string trim [string range $code [expr $ix +1] end] " "]
	set code [string trim [string range $code 0 [expr $ix -1]] " "]
	if {$color == ""}  {
	    unset color
	} elseif {[UTF::color2ansicode $color] == 0} {
	    UTF::Message WARN "" \
		"Unsupported ansi color $color sent to UTF:Message"
	    unset color
	}
    }
    if {$code == "FAIL"} {
	# Make failures stand out
	set code "*$code*"
    } else {
	set code " $code"
    }

    set time [UTF::timestamp]

    # Reveal any CR's that have managed to get this far.  Also ESC and
    # SO, which may upset terminal emulators.
    set msg [string map { "\r" "^M" "\0" "" "\033" "^[" "\016" "^N"} $msg]
    foreach m [split $msg "\n"] {
	set f [format {%-3s %-6s %-10s %s} $time $code $where $m]
	if {[info exists color]} {
	    puts stdout [UTF::color_message $f $color]
	} else {
	    puts stdout $f
	}
	# Close/Open log file only when log file changes
	if {$UTF::Logfile ne $UTF::_llogfile} {
	    if {$UTF::_llogfd ne ""} {
		after cancel $UTF::_llogid; # cancel flush
		if {[catch {close $UTF::_llogfd} ret]} {
		    # Nowhere safe to log this
		    puts stdout "Close $UTF::_llogfile failed: $ret"
		}
		set UTF::_llogfd ""
	    }
	    if {[set UTF::_llogfile $UTF::Logfile] ne ""} {
		set UTF::_llogfd [open $UTF::_llogfile a]
	    }
	}
	if {$UTF::_llogfd ne ""} {
	    after cancel $UTF::_llogid; # cancel flush
	    puts $UTF::_llogfd $f
	    # flush in 5 min
	    set UTF::_llogid [after 300000 flush $UTF::_llogfd]
	}
	# Reduce priority of follow-on lines so that we can get a
	# realistic PASS/FAIL count.
	set code " LOG"
    }
}

UTF::doc {

    # [call [cmd UTF::Assert]
    # 		[lb][option -i][rb]
    # 		[lb][option -x][rb]
    #		[lb][option -u][rb]
    # 		[lb][option {-exact|-glob|-regexp}][rb]
    # 			[arg value] [arg expect] [arg msg]]


    # [emph DEPRECATED:] TCL errors are now managed by [cmd UTF::Try]
    # and [cmd UTF::Record] so in most cases tests should just use
    # regular conditionals and [cmd error] [arg message].[para]

    # Basic pass/fail assertion.  [arg value] is compared to the [arg
    # expect] pattern.  If the comparison succeeds, the [arg msg] will
    # be passed on to [cmd UTF::Message] with a code of PASS,
    # otherwise [arg msg] will be passed on with a code of FAIL.

    # [list_begin options]

    # [opt_def [option -i]]

    # Invert the sense of the test, ie return PASS instead of FAIL, or
    # SUPPORTED instead of UNSUPPORTED.

    # [opt_def [option -x]]

    # Test is expected to fail, so report XPASS/XFAIL, instead of
    # PASS/FAIL.

    # [opt_def [option -u]]

    # Testing supportability, so report PASS/UNSUPPORTED, instead of
    # PASS/FAIL.

    # [opt_def [option -exact]]

    # Use exact matching when comparing value to the expect
    # pattern. This is the default.

    # [opt_def [option -glob]]

    # When matching value to the expect pattern, use glob-style
    # matching (i.e. the same as implemented by the [cmd {string
    # match}] command).

    # [opt_def [option -regexp]]

    # When matching value to the expect pattern, use regular
    # expression matching (as described in the [term re_syntax]
    # reference page).

    # [list_end]

}

# Basic Pass/Fail test
proc UTF::Assert {args} {
    set usage \
	{usage: Assert [-i] [-x] [-u] [-exact|-glob|-regexp] value expect where msg}
    set pass PASS
    set fail FAIL
    set op -exact
    set true 1

    if {[llength $args] < 4} {
	error $usage
    }
    set value [lindex $args end-3]
    set expect  [lindex $args end-2]
    set where  [lindex $args end-1]
    set msg  [lindex $args end]

    foreach arg [lreplace $args end-2 end] {
	switch -glob -- $arg {
	    -i {
		set true 0
	    }
	    -x {
		set pass XPASS
		set fail XFAIL
	    }
	    -u* {
		set fail UNSUPPORTED
	    }
	    -g* -
	    -e* -
	    -r* {
		set op $arg
	    }
	    -- {
		break
	    }
	    -* {
		error $usage
	    }
	}
    }
    switch $op -- $value $expect {
	set result 1
    } default {
	set result 0
    }
    if {$result eq $true} {
	UTF::_Message $pass $where "$msg: $value"
	return 1
    } else {
	UTF::_Message $fail $where "$msg: \"$value\" !~ \"$expect\""
	if {$fail eq "FAIL"} {
	    error "$fail $where $msg: \"$value\" !~ \"$expect\""
	} else {
	    return 0
	}
    }
}

UTF::doc {

    # [call [cmd UTF::assert] [arg expr] [arg where] [arg msg]]

    # [emph DEPRECATED:] TCL errors are now managed by [cmd UTF::Try]
    # and [cmd UTF::Record] so in most cases tests should just use
    # regular conditionals and [cmd error] [arg message].[para]

    # Expression-based pass/fail assertion.  [arg expr] is
    # evaluated in the current context.  If expr evaluates to true
    # then [arg msg] will be passed on to [cmd UTF::Message] with a
    # location of [arg where] and a code of PASS, otherwise it will be
    # passed on with a code of FAIL.
}

proc UTF::assert {TEST WHERE MSG} {
    if {[uplevel 1 expr $TEST]} {
	UTF::Message "PASS" $WHERE $MSG
    } else {
	UTF::Message "FAIL" $WHERE $MSG
	error "FAIL $WHERE $MSG"
    }
}


UTF::doc {
    # [call [cmd UTF::XPASS] [arg where] [arg result] [arg context]]

    # Record XPASS lines of context/result data in the current test
    # log.  [arg context] is a dict composed of key/value pairs
    # describing context for the [arg result].
}

proc UTF::XPASS {where result context} {
    UTF::_Message XPASS $where "$context: $result"
}


UTF::doc {
    # [call [cmd UTF::Logfile] [lb][arg file][rb]]

    # Returns file destination for log messages.  If [arg file] is
    # specified, set file destination for log messages.  The default
    # is "", which means no log file.  If set within a UTF::Test, the
    # setting becomes local to the test and to any sub-tests.  Note
    # that log messages are always also sent to stdout, irrespective
    # of the setting of the log file.

    # [example_begin]
    utf> UTF::Logfile "test1.log"
    test1.log
    utf> UTF::Logfile
    test1.log
    utf>
    # [example_end]

    # UTF::Logfile can also be treated directly as a variable, eg:

    # [example_begin]
    utf> set UTF::Logfile "test2.log"
    test2.log
    utf> puts $UTF::Logfile
    test2.log
    utf>
    # [example_end]
}
# Self-setting variable, implemented as a simple alias
interp alias {} UTF::Logfile {} set UTF::Logfile

proc UTF::StripHtml {msg} {
    if {[regsub {^html:} $msg {} msg]} {
	regsub -all {<[^>]*>} $msg {} msg
    }
    return $msg
}

UTF::doc {
    # [call [cmd UTF::Try] [arg message] [arg block]]

    # Reports [arg message] and then attempts to execute [arg block].
    # If the [arg block] fails with an error a FAIL will be generated
    # but caught so the script can continue with further tests.
}
if {[info exists ::env(UTFTRYFAIL)]} {
    interp alias {} UTF::Try {} UTF::Record
} else {
    proc UTF::Try {msg block {finally {}} {fblock {}}} {
	if {($finally eq {}) ? $fblock ne {} : $finally ne {finally}} {
	    return -code error "usage UTF::Try msg {...} ?finally {...}?"
	}
	if {[catch {uplevel 1 [list UTF::Record $msg $block $finally $fblock]} ret]} {
	    if {[regexp {^EXIT\d+$} $::errorCode]} {
		error "" "" $::errorCode
	    } elseif {$::errorCode ne "FAIL"} {
		# Simple FAIL results propagate the short form error.
		# Any other error codes propagate the entire stack
		# trace.
		set ret $::errorInfo
	    }
	    UTF::Message FAIL "" "$msg: [UTF::StripHtml $ret]"
	    set UTF::TryStack [lreplace $UTF::TryStack end end $ret]
	} else {
	    UTF::Message PASS "" "$msg: [UTF::StripHtml $ret]"
	}
    }
}

UTF::doc {
    # [call [cmd UTF::ElapsedTime] [arg start]]

    # Reports the elapsed time between [arg start] the current time in
    # the format HH:MM:SS.  [arg start] should be the result of an
    # earlier call to [clock seconds]
}
proc UTF::ElapsedTime {start} {
    set sec [expr {[clock seconds] - $start}]
    format {%02d:%02d:%02d} [expr {$sec/3600}] \
	[expr {($sec%3600)/60}] [expr {$sec%60}]
}

UTF::doc {
    # [call [cmd UTF::Break] [arg none]]

    # Breaks a script and enters TCL interactive mode
    # prints the local variables and set the prompt per args
    # type c to exit the breakpoint

}
proc UTF::Break {{name ""}} {
    # Only issue break if a tty exists
    if {[catch {exec tty -s}]} {
	return
    }
    package require TclReadLine
    catch {package require TclReadLine::UTF}
    if {[info exists ::UTF::__breakcount]} {
	incr ::UTF::__breakcount
    } else {
	set ::UTF::__breakcount 1
    }
    if {[info exists ::UTF::__accumbreakcount]} {
	incr ::UTF::__accumbreakcount
    } else {
	set ::UTF::__accumbreakcount 1
    }
    if {$name eq {}} {
	set name "${::UTF::__accumbreakcount}/${::UTF::__breakcount}/[info level]"
    }
    set ::UTF::__breakname($::UTF::__breakcount) "$name"
    while {$::UTF::__breakcount} {
	uplevel {
	    foreach stackvar [info locals] {
		if {$stackvar eq {stackvar}} {
		    continue
		}
		if {[array exists $stackvar]} {
		    catch {parray $stackvar}
		} else {
		    catch {puts "$stackvar=[set $stackvar]"}
		}
	    }
	}
	proc ::UTF::prompt {} {puts -nonewline "utf_break($::UTF::__breakname($::UTF::__breakcount)), enter 'c' to continue> " }
	set ::tcl_prompt1 ::UTF::prompt
	::TclReadLine::breakpoint
	incr ::UTF::__breakcount -1
    }
}

UTF::doc {
    # [call [cmd UTF::WrapSummary] [arg LOGDIR] [arg TITLE] [arg INFO]
    #         [arg EMAIL] [arg {{BLOCK}}]]

    # Run [arg BLOCK] under the Web summary report generator.  Reports
    # and log files will be stored in a timestamped subdirectory under
    # [arg LOGDIR].  The report will also be emailed to [arg EMAIL],
    # which defaults to self but can be set to "none" to disable email
    # reporting.  [arg INFO] can be used to add additional header
    # information in text or html.  [para]

    # Nested copies of [cmd UTF::WrapSummary] will be ignored and all
    # results will be included on the top-level web report.
}

set ::UTF::_nested_test 0
proc UTF::WrapSummary {logdir TITLE INFO EMAIL BLOCK} {
    set code 0
    try {
	if {[incr ::UTF::_nested_test] == 1} {
	    rename ::exit ::_exit
	    proc ::exit {{code 0}} {
		throw EXIT$code ""
	    }
	    UTF::StartSummary $logdir $TITLE $INFO
	    if {[regexp {hnd-utf-list} $EMAIL]} {
		# Register test with Database
		UTF::TestRegister
		if {[info exists ::UTF::dBuxRegister]} {
		    UTF::dBuxRegister
		}
		if {[info exists ::UTF::UTFStatus]} {
		    # Check UTF workspace
		    UTF::Try "UTF" {
			package require UTF::Test::utfstatus
			UTF::Test::utfstatus
		    }
		}
	    }
	    # Register report information with UTFD
	    if {[info exists ::env(UTFDPORT)]} {
		UTFD::TestRegister $logdir $::UTF::SessionID \
		    $::UTF::WebServer $EMAIL
	    }
	}
	if {$UTF::args(websetup) ne ""} {
	    UTF::Record "WebSetup" $UTF::args(websetup)
	}
	if {[catch {uplevel $BLOCK} ret]} {
	    set e $::errorInfo
	    if {![regexp {^EXIT(\d+)$} $::errorCode - exitcode]} {
		UTF::Try "Framework" {
		    error $ret $e
		}
	    }
	}
	if {$UTF::_nested_test == 1} {
	    rename ::exit ""
	    rename ::_exit ::exit
	    set code [UTF::EndSummary $logdir $TITLE $EMAIL]
	}
    } finally {
	incr ::UTF::_nested_test -1
    }
    if {[info exists exitcode]} {
	exit $exitcode
    } elseif {$code} {
	exit $code
    }
}

UTF::doc {
    # [call [cmd UTF::Test] [arg name] [arg args] [arg body]]

    # Use like [cmd proc] to define a new test command.

    # When the script is run directly, the command will be executed
    # with its arguments taken from the command-line.

    # When the script is referenced by other test scripts, the command
    # [cmd UTF::Test::[arg name]] will be made available to be used
    # as a component of the larger test.
}

# Create main test proc and if we are the top-level test run it.
proc UTF::Test {name args body} {
    set msglist ""
    set runnow [expr {[info exists ::argv0] &&
		      [string match [info script] $::argv0]}]

    # If UTFD is enabled on this test rig invoke it for possible
    # submission. Also, if this is a UTFD child process thats being
    # run by UTFD the UTFD::schedule? will setup the environment
    # (e.g. UTF::args and utfconf) and return 0 so the script will
    # execute
    if {$runnow && [info exists ::env(UTFDPORT)] && [::UTFD::schedule?]} {
	return
    }

    foreach arg $args {
	if {$arg ne "args"} {
	    set arg [lindex $arg 0]
	}
	append msglist " $$arg"
    }

    # Create a private namespace for support procs.  Set path to
    # access public tests
    namespace eval ::UTF::Test::$name {
	namespace path ::UTF::Test
    }

    proc ::UTF::Test::$name $args [subst {

	# Prepend path for main test to access private namespace
	set savedNamespacePath \[namespace path]
	namespace path \[lreplace \$savedNamespacePath -1 -1 ::UTF::Test::$name]
	set savedUTFLogfile \$UTF::Logfile
	UTF::Message INFO {} "$name $msglist"
	try {$body} finally {
	    set ::UTF::Logfile \$savedUTFLogfile
	    # Restore ns path
	    namespace path \$savedNamespacePath
	}
    }]

    if {$runnow} {
	if {$UTF::args(web)} {
	    if {[set title $UTF::args(webtitle)] eq ""} {
		set title "$name $::argv"
		if {$UTF::args(websetup) ne ""} {
		    set title "$UTF::args(websetup); $title"
		}
	    }
	    UTF::WrapSummary \
		$UTF::SummaryDir $title "" $UTF::args(webemail) {
		    UTF::Test::$name {*}$::argv
		}
	} else {
	    UTF::Record "" {
		UTF::Test::$name {*}$::argv
	    }
	}
    }
}

UTF::doc {
    # [call [cmd UTF::Test::PreservedReport]]

    # Reports files used in driver search and load which do not have
    # corresponding PRESERVED bits.  In a Web context this will result
    # in one FAIL line per file.  The list of failed files is reset
    # after reporting.  The list is stored in the global
    # ::UTF::NotPreserved if you need to manipulate it directly.
}
namespace eval UTF::Test {
    proc PreservedReport {} {
	if {![info exists ::UTF::NotPreserved]} {
	    return
	}
	if {[info exists ::UTF::_nested_test] && $::UTF::_nested_test > 1} {
	    return
	}
	foreach f $::UTF::NotPreserved {
	    # Double check in case the preserved file turned up late
	    if {![file exists $f]} {
		UTF::Try "PRESERVED missing" {
		    error $f
		}
	    } elseif {[file size $f] == 0} {
		UTF::Try "PRESERVED empty file" {
		    error $f
		}
	    } else {
		UTF::Try "PRESERVED late" {
		    return $f
		}
	    }
	}
	set ::UTF::NotPreserved {}
    }
}

proc UTF::SetupTestBed {} {
    if {[info exists ::UTF::SetupTestBed]} {
	uplevel 1 $::UTF::SetupTestBed
    }
}

# Stub for initializing smoketest rigs.  Rigs that need initialization
# should override this.
proc UTF::SmokeInit {} {}

UTF::doc {
    # [call [cmd UTF::NewSSID]]

    # Generate a unique SSID for this test.  SSID will contain the
    # username, time, and a random number to guarantee uniqueness.
    # The username will be truncated if necessary to keep under the 32
    # byte SSID limit.
}

proc UTF::NewSSID {} {
    return [format {%.12s-%s-%d} \
		$::tcl_platform(user) \
		[clock format [clock seconds] -format {%T}] \
		[expr {int(rand()*(1<<30))}]]
}

UTF::doc {
    # [call [cmd UTF::NewWepKey] [lb][arg len][rb]]

    # Generate a random WEP key for this test.  [arg len] specifies
    # the length of the generated key in hex digits, with valid values
    # being 10, 26, 32 or 64.  If [arg len] is not specified it
    # defaults to 26.

    # 10 corresponds to WEP-40 (WEP64)
    # 26 corresponds to WEP-104 (WEP128)
}

proc UTF::NewWepKey {{len 26}} {
    set valid {10 26 32 64}
    if {[lsearch -exact $valid $len] < 0} {
	error "" "Invalid wep key length $len, should be $valid"
    }
    for {set i 0} {$i < $len} {incr i 2} {
	append k [format {%02x} [expr {int(rand()*256)}]]
    }
    return $k
}

UTF::doc {
    # [call [cmd UTF::NewWPAPassphrase]]

    # Generate a random WPA Passphrase.  The passphrase will consist
    # of between 8 and 63 random ascii word characters.  Non-word
    # chars are disallowed because they are too much trouble to pass
    # as arguments via the various shells.
}

proc UTF::NewWPAPassphrase {} {
    set len [expr {8+int(rand()*55)}]
    set i 0
    while {$i < $len} {
	set c [format {%c} [expr {32+int(rand()*94)}]]
	if {[string is wordchar $c]} {
	    append k $c
	    incr i
	}
    }
    return $k
}

UTF::doc {
    # [call [cmd UTF::decomment] [arg data] [lb][arg ...][rb]]

    # Strip comments from data.  Frequently used in parameter lists.
    # Multiple arguments will be concatenated into a single result and
    # returned.

    # [example_begin]
} {{
    set data {
	# Comment
	A "Hello"

	B "World"
    }
    puts [UTF::decomment $data]
    A "Hello" B "World"
}} {
    # [example_end]

    # [call [cmd UTF::decomment] [arg variable]]

    # As above, except that if an existing variable is named, the
    # variable will be assumed to contain the input data and will be
    # updated in-place.

    # [example_begin]
} {{
    set data {
	# Comment
	A "Hello"

	B "World"; # Another comment
    }
    UTF::decomment data
    puts $data
    A "Hello"

    B "World"
}} {
    # [example_end]
}

proc UTF::decomment {args} {
    if {$args eq "{}"} {
	return ""
    } elseif {[uplevel info exists $args]} {
	upvar $args var
    } else {
	eval set var $args
    }
    regsub -line -all {(^|;)\s*\#.*} $var {} var
    return $var
}

UTF::doc {
    # [call [cmd UTF::jsliteral] [arg file]]

    # proc for encoding the contents of a file as a JavaScript
    # literal.  Used in an abortive attempt at speeding up web pages
    # containing small images.  This has been abandoned in favour of
    # data: URL's but the proc is being left here for a while in case
    # it turns out to be useful for anything else.
}

proc UTF::jsliteral {file} {
    set fd [open $file]
    fconfigure $fd -translation binary
    while {1} {
	set s [read $fd 1]
	if {[eof $fd]} {
	    break
	}
	if {$s <= " " || $s eq "\\" || $s > "z" ||
	    [lsearch {\\ \" ' ` \# %} $s] >= 0} {
	    scan $s "%c" c
	    append out [format "\\\%o" $c]
	} else {
	    append out $s
	}
    }
    close $fd
    return $out
}

UTF::doc {
    # [call [cmd UTF::ThumbData] [arg file]]

    # Convert thumbnail into a base64-encoded data: url.  This allows
    # inlining of the image data which makes rendering of the page
    # much faster.  These will not display in IE7 or earlier.[para]

    # Output is an HTML fragment which can be used in place of the
    # <img ...> token.[para]

    # Example:
    # [example_begin]
} {{
    return "html:[UTF::ThumbData ${file}_sm.png] <a href=\"${file}.png\">$msg</a>"
}} {
    # [example_end]

    # References:
    # http://www.websiteoptimization.com/speed/tweak/inline-images/
}

variable mogrify
proc UTF::ThumbData {file} {
    package require base64


    if {![info exists mogrify]} {
	set mogrify [auto_execok mogrify]
    }
    if {$mogrify ne ""} {
	# Shrink thumnail file size by stripping comments
	catch {exec $mogrify -strip $file} ret
	UTF::Message LOG "" $ret
    }

    set f [open $file]
    fconfigure $f -translation binary
    set data [base64::encode -maxlen 0 [read $f]]
    close $f

    # no need to keep original file now it's been in-lined.
    file delete $file

    subst {<img src="data:image/png;base64,$data" alt="data" />}
}

# shim layer for access to the build file repository.  May be
# overridden for remote access.
namespace eval UTF::BuildFile {
    proc glob {args} {
	::glob {*}$args
    }
    proc size {file} {
	file size $file
    }
    proc exists {file} {
	file exists $file
    }
    proc ls {file} {
	exec ls -l $file
    }
    proc sum {file} {
	exec sum $file
    }
    proc isdirectory {file} {
	file isdirectory $file
    }
    proc strings {file} {
	exec strings $file
    }
    proc modinfo {args} {
	localhost -t 120 /sbin/modinfo {*}$args
    }
    proc gdb {cmd script} {
	localhost rexec -s -t 180 "$cmd <$script"
    }
    proc stat {file RET} {
	upvar $RET ret
	file stat $file ret
    }
    proc copyto {dut file args} {
	$dut copyto $file {*}$args
    }
    proc nvram_add_copyto {dut src dst replace} {
	if {[set replace [UTF::decomment $replace]] eq ""} {
	    copyto $dut $src $dst
	} else {
	    set nvram [UTF::nvram_add $src $replace]
	    try {
		$dut copyto $nvram $dst
	    } finally {
		#UTF::Message LOG "" "file delete $nvram"
		file delete $nvram
	    }
	}
    }
}

UTF::doc {
    # [call [cmd UTF::SortImages] [arg pattern] [lb][arg -ls][rb]
    #	      [lb][arg -all][rb] [lb][arg -bt][rb]]

    # Performs a glob scan on [arg pattern] then returns file, or
    # files sorted by date with the youngest first, then by features.

    # [list_begin options]

    # [opt_def [arg -ls]]

    # Report ls -l of the file, rather than just the filename

    # [opt_def [arg -all]]

    # Return all matches.  The default is to return the first
    # non-empty file.

    # [opt_def [arg -bt]]

    # Parse file names for BlueTooth build version instead of HND
    # build date.

    # [list_end]
}

proc UTF::SortImages {pattern args} {
    set _args $args
    UTF::Getopts {
	{ls "Report ls -l"}
	{all "Return all matches"}
	{bt  "Parser looks for BlueTooth build version, not HND date."}
    }

    if {[string match {*%date%*} $pattern]} {
	set oldpattern $pattern
	if {!$(all)} {
	    # This month
	    set date [join [clock format $UTF::_start_time -format {%Y %N}] .]
	    set pattern [string map [list %date% $date] $oldpattern]
	    if {![catch {UTF::SortImages $pattern {*}$_args} ret]} {
		return $ret
	    }
	    # This year
	    set date [clock format $UTF::_start_time -format {%Y}]
	    set pattern [string map [list %date% $date] $oldpattern]
	    if {![catch {UTF::SortImages $pattern {*}$_args} ret]} {
		return $ret
	    }
	}
	# All
	set date "20"
	set pattern [string map [list %date% $date] $oldpattern]
    }

    set files [BuildFile::glob -nocomplain $pattern]

    if {[llength $files] == 0} {
	error "No matching images found in $pattern"
    }
    # puts "files=$files"

    foreach file $files {
	# Drop any PRESERVED file that is also available non-preserved
	if {[regsub {/PRESERVED/} $file {/} origfile]} {
	    regsub {\.gz$} $origfile {} origfile
	    if {[lsearch -exact $files $origfile] >= 0} {
		continue
	    }
	}

	# Chop dates out of paths so we can uniq them
	# save date column number so we can put it back later
	set l [file split $file];# puts "$l"

        # Parsing depends on -bt option.
        if {!$(bt)} {
            # Look for HND build date
            set dcol [lsearch -regexp $l {\d+\.\d+\.\d+\.\d+}]
            set date [lindex $l $dcol]
            if {[regexp {(\d+)\.(\d+)\.(\d+)\.(\d+)} $date - y m d i]} {
		set sdate [format "%04d%02d%02d%03d" $y $m $d $i]
	    } else {
		# No date
		set sdate $date
	    }
        } else {
            # Look for BlueTooth build version.
            set dcol [lsearch -regexp $l {.*_\d+\.\d+\.\d+\.\d+}]
            #puts "$dcol"
            # debug code
            # set cap [regexp {.*_(\d+)\.\.*} $l whole part]
	    # puts "cap value: $cap; whole: $whole; part: $part"
	    # end debug code
            if { $dcol != -1 } {
		set date [lindex $l $dcol];# puts $date
		regexp {(\d+)\.(\d+)\.(\d+)\.(\d+)} $date - i j k m ;# dont use l!
		# The format command gets confused when it sees
		# multiple leading zeros in the numbers, so we remove
		# all leading zeros.
		foreach var {i j k m} {
		    set $var [UTF::clean_number [set $var]]
		}
		set sdate [format "%05d%05d%05d%05d" $i $j $k $m]
	        } else { ;# try where initial date/build string match failed
		    set dcol [lsearch -regexp $l {.*_\d+\.\d+\.\d+\.*}]
		    # puts "Now dcol is: $dcol"
	            set date [lindex $l $dcol];# puts $date
	            # check if matching alternate pattern succeeds
	            if {[regexp {(\d+)\.(\d+)\.(\d+)\.*} $date - i j k]} { ;# dont use l!
			# puts "alt variable i value: $i; alt variable j value: $j; alt variable k value: $k"
			# The format command gets confused when it
			# sees multiple leading zeros in the numbers,
			# so we remove all leading zeros.
			foreach var {i j k} {
			    set $var [UTF::clean_number [set $var]]
			}
			set sdate [format "%05d%05d%05d" $i $j $k]
			# puts "New merged alt date value: $sdate"
		    } else {
			set sdate $date ;# when all else failed
		    }
	        } ;# end of dcol value test
        } ;# end of BT branch
        # puts "dcol=$dcol date=$date sdate=$sdate"
	lappend sfiles [list [lreplace $l $dcol $dcol] $sdate $date $dcol]
    }
    # Reverse sort on dateless part, date.
    set sfiles [lsort -decreasing -index 0 $sfiles]
    set sfiles [lsort -decreasing -index 1 $sfiles]
    set files {}
    foreach file $sfiles {
	# Re-insert dates back into paths
	if {[lindex $file 3] > 0} {
	    set f [file join {*}[lreplace [lindex $file 0] \
				     [lindex $file 3] 0 \
				     [lindex $file 2] ]]
	} else {
	    # There was no date, so use as-is
	    set f [file join {*}[lindex $file 0]]
	}
	lappend files $f
    }

    # if we're not returning them all, then return the first non-empty
    if {!$(all)} {
	foreach f $files {
	    if {[BuildFile::size $f]} {
		set files $f
		break
	    } else {
		UTF::Message WARN "" "Empty file $f"
		set files ""
	    }
	}
	UTF::check_for_preserved $files
    }
    if {$files eq ""} {
	error "No images available"
    }
    if {$(ls)} {
	set ls {}
	foreach f $files {
	    lappend ls [exec ls -l $f]
	}
	set files $ls
	unset ls
    }
    join $files \n
}

UTF::doc {
    # [call [cmd UTF::check_for_preserved] [arg file]]

    # If [arg file] is in the primary build repository, check to see
    # if a PRESERVED copy exists.  If not, issue a warning and add to
    # the the global [cmd ::UTF::NotPreserved] list.  If the file is
    # not a primary build file, do nothing.
}

proc UTF::check_for_preserved {file} {
    # Check for a PRESERVED version
    # Skip symbol tables since they are no use without source
    if {[file tail $file] ne "rtecdc.exe" &&
	[regsub {(swbuild/build_\w+)/(NIGHTLY|TRUNK|trunk|\w+_BRANCH_|\w+_TWIG_|\w+_REL_)} \
	     $file {\1/PRESERVED/\2} p]} {
	if {[file extension $p] ne ".gz"} {
	    append p ".gz"
	}
	if {![BuildFile::exists $p] || [BuildFile::size $p] == 0} {
	    UTF::Message WARN "" "Not preserved: $p"
	    # Update global list
	    if {![info exists ::UTF::NotPreserved] ||
		[lsearch $::UTF::NotPreserved $p] < 0} {
		lappend ::UTF::NotPreserved $p
	    }
	}
    }
}

UTF::doc {
    # [call [cmd UTF::clean_number] [arg number]]

    # Removes leading zeros from a number and returns it.
    # If number was all zeros, returns a single zero. This
    # prevents tcl expr from getting confused and thinking
    # that 0029 is a malformed octal number.
}

proc UTF::clean_number {number} {
    regsub {^(-)?0+} $number {\1} number
    if {$number == "" || $number == "-"} {
        set number 0
    }
    set number
}

UTF::doc {
    # [call [cmd UTF::URI] [arg link]]

    # Encode link suitably for use in a URI.  This should be used in
    # cases where the link is a filename containing spaces or other
    # elements not allowed in URIs.
}

proc UTF::URI {link} {
    # Strip out common path
    catch {set link [string map [list "$::UTF::Logdir/" {}] $link]}
    string map {" " %20 "#" %23 "{" %7b "}" %7d} $link
}

UTF::doc {
    # [call [cmd UTF::ddcmp] [arg a] [arg b]]

    # Compare dotted decimals, such as kernel versions, os versions,
    # driver versions, etc.  Returns an integer less than, equal to,
    # or greater than zero if [arg a] is found, respectively, to be
    # less than, to match, or be greater than [arg b].  Eg:

    # [example_begin]
    utf> UTF::ddcmp 3.11.1 2.6
    1
    utf> UTF::ddcmp 7.35.17 7.10
    1
    utf> UTF::ddcmp 7.10 7.35.17
    -1
    utf> UTF::ddcmp 10.0.5 10.0.11
    -1
    # [example_end]
}

proc UTF::ddcmp {a b} {
    foreach A [split $a "."] B [split $b "."] {
	if {$A < $B} {
	    return -1
	} elseif {$A > $B} {
	    return 1
	}
    }
    return 0
}

UTF::doc {
    # [call [cmd UTF::kexpand] [arg number]]

    # Expands k/m/g abreviations by the appropriate multipliers of
    # 1024, or kb/mb/gb by multipliers of 1000 (cf [cmd du] and [cmd
    # df]).  Eg:

    # [example_begin]
    utf> UTF::kexpand 1152k
    1179648
    utf> UTF::kexpand 2.5m
    2621440.0
    utf> UTF::kexpand 2.5mb
    2500000.0
    # [example_end]
}

proc UTF::kexpand {n} {
    if {[regexp -nocase {^(\d+(?:.\d+)?)kb$} $n - n]} {
	expr {1000 * $n}
    } elseif {[regexp -nocase {^(\d+(?:.\d+)?)mb$} $n - n]} {
	expr {1000000 * $n}
    } elseif {[regexp -nocase {^(\d+(?:.\d+)?)gb$} $n - n]} {
	# Return a double to avoid int wrapping
	expr {1000000000.0 * $n}
    } elseif {[regexp -nocase {^(\d+(?:.\d+)?)k$} $n - n]} {
	expr {1024 * $n}
    } elseif {[regexp -nocase {^(\d+(?:.\d+)?)m$} $n - n]} {
	expr {1024 * 1024 * $n}
    } elseif {[regexp -nocase {^(\d+(?:.\d+)?)g$} $n - n]} {
	# Return a double to avoid int wrapping
	expr {1024.0 * 1024 * 1024 * $n}
    } else {
	set n
    }
}

if {[info commands lreverse] eq ""} {
    # Implementation of lreverse for TCL < 8.5
    # http://wiki.tcl.tk/43
    proc lreverse L {
	set res {}
	set i [llength $L]
	while {$i} {lappend res [lindex $L [incr i -1]]}
	set res
    }
}

UTF::doc {
    # [call [cmd UTF::Combinations] [arg k] [arg list]]

    # Returns a list of all combinations (subsets) of length [arg k]
    # of the elements of the input list.  The relative ordering of
    # elements is preserved.  [arg list] may be any valid tcl list,
    # including one containing other lists.
}

# Permutation selector based on code from Keith Vetter posted to
# http://wiki.tcl.tk/.
# Returns all permutations of length k from list l
proc UTF::Combinations {k l} {
    if {$k == 0} {
	return {}
    }
    if {$k == [llength $l]} {
	return [list $l]
    }
    set all {}
    incr k -1
    for {set i 0} {$i < [llength $l]-$k} {incr i} {
        set first [lindex $l $i]
        if {$k == 0} {
            lappend all [list $first]
        } else {
            foreach s [Combinations $k [lrange $l [expr {$i+1}] end]] {
		set ans [concat [list $first] $s]
                lappend all $ans
            }
        }
    }
    return $all
 }

# [call [cmd UTF::_Permutations] [arg list]]

# Returns a list of all permutations (re-orderings) of the input list.
# [arg list] may be any valid tcl list, including containing other
# lists.

proc UTF::_Permutations {list {prefix ""}} {
    if {![llength $list]} {
	return [list $prefix]
    }
    set res [list]
    set n 0
    foreach e $list {
        lappend res {*}[_Permutations [lreplace $list $n $n] [linsert $prefix end $e]]
        incr n
    }
    return $res
}

UTF::doc {
    # [call [cmd UTF::Permutations] [arg k] [arg list]]

    # Returns a list of all permutations (re-orderings) of subsets of
    # length [arg k] of the elements of the input [arg list].  [arg
    # list] may be any valid tcl list, including one containing other
    # lists.
}


proc UTF::Permutations {k list} {
    set res {}
    foreach p [UTF::Combinations $k $list] {
	set res [concat $res [UTF::_Permutations $p]]
    }
    return $res
}

UTF::doc {
    # [call [cmd UTF::ForeachPermutation] [arg varlist] [arg list] [arg body]]

    # The [cmd UTF::ForeachPermutation] command implements a loop
    # where the loop variables take on values consisting of each of
    # the permutated subsets of [arg list] containing as many elements
    # as there are loop variables.

    # For example:

    # [example_begin]
    utf> set x {}
    utf> UTF::ForeachPermutation {i j} {a b c} {
        lappend x $j $i
    }
    \# The value of x is "b a a b c a a c c b b c"
    \# There are 6 iterations of the loop
    # [example_end]

    # [arg list] may be any valid tcl list, including one containing
    # other lists.
}

# Should be the same number of elements as loop vars, so just use
# foreach to get them.  Use uplevel to run the command in the caller's
# frame.
proc UTF::ForeachPermutation {a l b} {
    foreach p [Permutations [llength $a] $l] {
	uplevel [list foreach $a $p $b]
    }
}

UTF::doc {
    # [call [cmd UTF::Permute] [arg list]]

    # Returns a random permutation of [arg list].
}

proc UTF::Permute {list} {
    set l [llength $list]
    if {!$l} {
	return {}
    }
    set i [expr {int(rand() * $l)}]
    return [concat [list [lindex $list $i]] \
		[UTF::Permute [lreplace $list $i $i]]]
}

UTF::doc {
    # [call [cmd UTF::GaussRand] [arg m] [arg A] [arg N]]

    # Pick N samples of a Normal (Gaussian) distribution with mean m
    # amplitude A.  Used in testing ControlChart tools.
}

proc UTF::GaussRand {m A N} {
    set l {}
    while {[llength $l] < $N} {
	# Generate a pair of independent Normally distributed random
	# numbers.  Mean m, Amplitude A
	set w 1
	while {$w >= 1.0} {
	    set x [expr {2.0 * rand() - 1}]
	    set y [expr {2.0 * rand() - 1}]
	    set w [expr {$x*$x + $y*$y}]
	}
	set w [expr {sqrt((-2.0 * log($w))/$w)}]
	set d1 [expr {$m + $A/3.0 * $x * $w}]
	set d2 [expr {$m + $A/3.0 * $y * $w}]
	lappend l $d1
	if {[llength $l] == $N} {
	    return $l
	}
	lappend l $d2
    }
    return $l
}

UTF::doc {
    # [call [cmd UTF::HistMode] [arg histogram]]

    # Given a histogram as list of key,count pairs, returns the key
    # with the largest count, and the ratio of that key's count to the
    # total count.

    # [example_begin]
} {{
    utf> set dump [sta1 wl_dump_ampdu]
    utf> UTF::HistMode [concat [$dump TXVHT] [$dump TXMCS]]
    9x3 0.33161265466565415
}} {
    # [example_end]
}


proc UTF::HistMode {histogram} {
    # Returns the most popular TX rate and its usage %
    set tot 0
    foreach {key count} $histogram {
	if {$count > 0} {
	    set tot [expr {$tot + $count}]
	    if {![info exists best] || $count >= $best} {
		set best $count
		set mode $key
	    }
	}
    }
    if {[info exists mode]} {
	return [list $mode [expr {1.0 * $best / $tot}]]
    } else {
	return {}
    }
}

UTF::doc {
    # [call [cmd UTF::forall] [arg varlist1] [arg list1]
    #	      [lb][arg varlist2] [arg list2] ...[rb] [arg body]]

    # The [cmd UTF::Forall] command implementa a loop where the loop
    # variables on values from one or more lists.  The difference
    # between [cmd UTF::forall] and the standard [cmd foreach] is that
    # [cmd foreach] iterates through the variable lists in step, while
    # [cmd UTF::forall] iterates through all the combinations (ie the
    # Cartesian Product of the variable lists):

    # [example_begin]
    utf> foreach a {1 2} b {3 4} {puts "a=$a, b=$b"}
    a=1, b=3
    a=2, b=4

    utf> UTF::forall a {1 2} b {3 4} {puts "a=$a, b=$b"}
    a=1, b=3
    a=1, b=4
    a=2, b=3
    a=2, b=4
    # [example_end]
}

proc UTF::forall {args} {
    if {[llength $args] < 3 || [llength $args] % 2 == 0} {
	return -code error "wrong \# args: should be \"forall varList list ?varList list ...? body\""
    }
    set body [lindex $args end]
    set args [lrange $args 0 end-1]
    while {[llength $args]} {
	set varName [lindex $args end-1]
	set list    [lindex $args end]
	set args    [lrange $args 0 end-2]
	set body    [list foreach $varName $list $body]
    }
    uplevel 1 $body
}



UTF::doc {
    # [call [cmd UTF::Sleep] [arg seconds] [lb][arg where] [arg msg][rb]]

    # Do nothing except process the event loop for the given number of
    # seconds.  Decimal values of [arg seconds] are allowed eg for
    # specifying durations of less than a second.  Optional arguments
    # [arg where] and [arg msg] can be used to add an explanatory note
    # to the default log entry.  If [arg where] is "quiet" then the
    # log message is suppressed.  If [arg seconds] is negative the
    # command returns immediately without logging any messages. [para]

    # If [arg seconds] is zero the command will process pending events
    # and return without any additional wait.  Only pending events
    # will be processed - new events that arrive during processing
    # will not, unlike the built-in [cmd update] which continues
    # processing events until the event queue is completely empty.
}

proc UTF::Sleep {sec {where ""} {msg ""}} {
    if {$sec < 0} {
	return
    } elseif {$where != "quiet"} {
	set txt "Sleep $sec sec"
	if {$msg != ""} {
	    append txt ": $msg"
	}
	UTF::Message INFO $where $txt
    }
    # Pick a private (global) variable
    # Retry if necessary to make sure it's unique
    while {![info exists sleepvar] || [info exists $sleepvar]} {
	set sleepvar "[namespace current]::___sleep[clock seconds][expr int(rand()*1000)]"
    }
    # Make sure var exists so we can prevent multiple access.
    set $sleepvar 0
    after [expr {int($sec*1000)}] [list set $sleepvar 1]
    vwait $sleepvar
    unset $sleepvar
}

UTF::doc {
    # [call [cmd UTF::Every] [arg seconds] [cmd script]
    #	      [lb][arg {script ...}][rb]]

    # All the script arguments are concatenated in the same fashion as
    # the concat command.  The resulting script is executed, then
    # scheduled to be repeated at intevals of [arg seconds].
}
proc UTF::Every {args} {
    global _everyids  _everyid

    if {![llength $args]} {
	if {[info exists _everyids]} {
	    parray _everyids
	}
	return
    }

    set interval [lindex $args 0]
    if {$interval == "info"} {
	return [array names _everyids]
    }
    #
    # See if arg1 is a -milliseconds option
    #
    set arg1 [lindex $args 1]
    if {$arg1 == "-milliseconds"} {
	set script [lrange $args 2 end]
    } else {
	#
	#  In this case a numeric arg1 is given in seconds
	#  so convert to an integer number of ms.
	#
	if {$interval != "cancel" && $interval != "idle"} {
	    set interval [expr {round($interval * 1000)}]
	}
	set script [lrange $args 1 end]
    }

    #
    #  Process any cancel requests options are
    #  o  every cancel all
    #  o  every cancel <everyid>
    #
    if {$interval eq "cancel"} {
	if {![info exists _everyids]} {
	    return
	}
	if {$script eq "all"} {
	    set idlist [array names _everyids]
	    foreach id $idlist {
		if {$_everyids($id) != "RUNNING"} {
		    after cancel $_everyids($id)
		    unset _everyids($id)
		} else {
		    set _everyids($id) "CANCELPENDING"
		}
	    }
	} else {
	    set index $script
 	    if {[info exists _everyids($index)]} {
		# Cancel now if the script is not running
		# otherwise signal the underlying _every not to reschedule
		if {$_everyids($index) != "RUNNING"} {
		    after cancel $_everyids($index)
		    unset _everyids($index)
		} else {
		    set _everyids($index) "CANCELPENDING"
		}
	    }
	}
	return
    }

    #
    #  Now that user command processing is done, call the
    #  underlying every routine to start the script on
    #  its periodic (per interval) and return a unique everyid.
    #
    if {[info exists _everyid]} {
	incr _everyid
    } else {
	set _everyid 100
    }
    UTF::_every $interval $script "every#$_everyid"
    return "every#$_everyid"
}

proc UTF::_every {interval script id} {
    global _everyids

    #
    #  Run the script and measure the time taken to run
    #
    set starttime [clock clicks -milliseconds]
    set _everyids($id) "RUNNING"
    set rc [catch {uplevel #0 eval $script} result]
    set finishtime [clock clicks -milliseconds]

    #
    #  Detect and process any catch codes from the script
    #
    #  Note: The script returning a break catch code is
    #  used to indicate a silent stop of the rescheduling
    #
    if {$rc == [catch error]} {
	UTF::Message ERROR $result $script
	return
    } elseif {$rc == [catch break]} {
	if {[info exists _everyids($id)]} {
	    unset _everyids($id)
	}
	return
    } elseif {$rc == [catch continue]} {
	# Ignore - just consume the return code
	set rc 0
    }

    #
    #  Adjust the reschedule time per the actual runtime
    #  Provide a minimum of 30 ms for a yield
    #
    if {$interval != "idle"} {
	set runtime [expr {$finishtime - $starttime}]
	set adj_interval [expr {$interval - $runtime}]
	if {$adj_interval < 0} {
	    UTF::Message WARN $id "$script runtime ($runtime ms) exceeded reschedule interval ($interval ms)"
	}
	#
	#  Set a minimum of 30 ms to reschedule
	#
	if {$adj_interval < 30} {
	    set adj_interval 30
	}
    } else {
	set adj_interval "idle"
    }

    #
    #  Reschedule next iteration unless there is a cancel pending.
    #
    #  Note:  The rescheduling of the script is done after
    #  calling it. This can be swapped but is a bit more complex,
    #  particularly when execution time > interval.
    #
    if {$_everyids($id) != "CANCELPENDING"} {
	set _everyids($id) [after $adj_interval [list UTF::_every $interval $script $id]]
    } else {
	unset _everyids($id)
    }
    return
}

UTF::doc {
    # [call [cmd UTF::OnAll] [arg {type ...}] [cmd script]
    #	      [lb][arg {script ...}][rb]]

    # All the script arguments are concatenated in the same fashion as
    # the concat command.  An attempt is then made to execute the
    # resulting script against all UTF objects of type(s) [arg type].
    # Errors will be caught so that each UTF object can be tried.

    # [example_begin]
    UTF::OnAll Linux setup
    # [example_end]
}

proc UTF::OnAll {type args} {
    set hosts {}
    set fail {}
    foreach t $type {
	lappend hosts {*}[$t info instances]
    }
    foreach host [lsort $hosts] {
	if {![$host cget -onall]} {
	    continue
	}
	set name [$host cget -name]
	if {[catch {
	    $host {*}$args
	} ret]} {
	    UTF::Message ERR $name $::errorInfo
	    lappend fail $name
	} else {
	    UTF::Message LOG $name $ret
	}
    }
    if {[llength $fail]} {
	error "Errors on [join $fail { }]"
    }
}

UTF::doc {
    # [call [cmd UTF::Numexpand] [arg list]]

    # [cmd UTF::Numexpand] is used to expand ranges in lists of
    # numerical parameter values.  List elements of the form N-M will
    # be expanded by inserting elements between N and M, ascending or
    # decending by 1, as necessary.  List elements of the form N-M+D
    # will be expanded by inserting elements between N and M,
    # ascending or decending by D, as necessary.  List elements of the
    # form N-M*D will be expanded by inserting elements between N and
    # M, ascending or decending in a geometric progression by a factor
    # of D.  List elements of the form NxM will be replaced by M
    # copies of element N (which may be a single element or a range).
    # List elements not of these forms will be unaffected.

    # [example_begin]
    utf> UTF::Numexpand {10 20 45-60+10x2}
    10 20 45 55 45 55
    utf> UTF::Numexpand {1-1024*2}
    1 2 4 8 16 32 64 128 256 512 1024
    # [example_end]
}
proc UTF::Numexpand {var} {
    set list {}
    if {[regsub -nocase -all {g} $var {} var]} {
	set unit "g"
    } elseif {[regsub -nocase -all {m} $var {} var]} {
	set unit "m"
    } elseif {[regsub -nocase -all {k} $var {} var]} {
	set unit "k"
    } else {
	set unit ""
    }
    foreach v $var {
	if {[regexp {^(\S+)x(\d+)$} $v - s m]} {
	    set sublist [Numexpand $s]
	    for {set i 0} {$i < $m} {incr i} {
		set list [concat $list $sublist]
	    }
	} elseif {[regexp {^(\d+(?:\.\d+)?)-(\d+(?:\.\d+)?)(?:\*(\d+(?:\.\d+)?))$} $v - s e d]} {
	    if {$d < 1} {
		error "geometric step must be > 1"
	    }
	    if {$s <= $e} {
		for {set i $s} {$i <= $e} {set i [expr {$i * $d}]} {
		    lappend list $i
		}
	    } else {
		if {$e == 0} {
		    error "can't geometrically decend to zero"
		}
		for {set i $s} {$i >= $e} {set i [expr {$i / $d}]} {
		    lappend list $i
		}
	    }
	} elseif {[regexp {^(\d+(?:\.\d+)?)-(\d+(?:\.\d+)?)(?:\+(\d+(?:\.\d+)?))?$} $v - s e d]} {
	    if {$d eq {}} {
		set d 1
	    }
	    if {$s <= $e} {
		for {set i $s} {$i <= $e} {set i [expr {$i + $d}]} {
		    lappend list $i
		}
	    } else {
		set d [expr {-$d}]
		for {set i $s} {$i >= $e} {set i [expr {$i + $d}]} {
		    lappend list $i
		}
	    }
	} else {
	    lappend list $v
	}
    }
    if {$list eq ""} {
	return {{}}
    }
    if {$unit ne ""} {
	foreach v $list {
	    if {$v eq "0"} {
		lappend result $v
	    } else {
		lappend result "$v$unit"
	    }
	}
	set result
    } else {
	set list
    }
}

UTF::doc {
    # [call [cmd UTF::Rateopt] [arg list]]

    # [cmd UTF::Rateopt] translates a list of rates into a
    # concatenated list of triples:

    # -r, -m or -v to specify legacy, mcs or vht rate used in the
    # various wl *rate commands

    # rate number to be used in the various wl *rate commands

    # rate specifier to be displayed [para]

    # Note the rate list must be explicit rates - range specifiers
    # should be expanded with [cmd UTF::Rateexp] first.

    # [example_begin]
    utf> UTF::Rateopt {(36) (48) (54) 13 14 15 auto}
    -r 36 (36) -r 48 (48) -r 54 (54) -m 13 13 -m 14 14 -m 15 15 {} auto auto
    # [example_end]
}

proc UTF::Rateopt {var} {
    set list {}
    foreach v $var {
	if {[regexp {^\(([.\d]+)\)$} $v - l]} {
	    LegacyIndex $l
	    lappend list -r $l $v
	} elseif {[regexp {^\d+x\d$} $v]} {
	    lappend list -v $v $v
	} elseif {[regexp {^\d+$} $v]} {
	    lappend list -m $v $v
	} elseif {[regexp {^auto$} $v]} {
	    lappend list -r $v $v
	} else {
	    error "Unable to parse rate specifier: $v"
	}
    }
    if {[llength $list] > 0} {
	return $list
    } else {
	# Must return something or there'll be nothing to test
	return {-r auto auto}
    }
}

UTF::doc {
    # [call [cmd UTF::Rateexp] [arg list]]

    # [cmd UTF::Rateexp] expands a list of rate specifiers in a
    # similar way to [cmd UTF::Numexpand].  Parentheses can be used to
    # indicate Legacy rates, and ranges of Legacy rates will be
    # expanded appropriately.[para]

    # Output consists of a list of rates.

    # [example_begin]
    utf> UTF::Rateexp {(36-54) 13-15 auto}
    (36) (48) (54) 13 14 15 auto
    # [example_end]
}

set UTF::LEGACY {1 2 5.5 6 9 11 12 18 24 36 48 54}
proc UTF::LegacyIndex {l} {
    set i [lsearch $UTF::LEGACY $l]
    if {$i>-1} {
	return $i
    } else {
	error "Illegal legacy rate: $l"
    }
}

proc UTF::Rateexp {var} {
     set list {}
    foreach v $var {
	if {[regexp {^(\S+)\*(\d+)$} $v - s m]} {
	    for {set i 0} {$i < $m} {incr i} {
		set list [concat $list [Rateexp $s]]
	    }
	} elseif {[regexp {^(\S+)_(\S+)$} $v - s m]} {
	    set list [concat $list [Rateexp $s] [Rateexp $m]]
	} elseif {[regexp {^(\d+)x(\d)-(\d+)x(\d)$} $v - sr ss er es]} {
	    # Map axb into an int
	    set s [expr {$sr+12*$ss}]
	    set e [expr {$er+12*$es}]
	    if {$s <= $e} {
		for {set i $s} {$i <= $e} {incr i} {
		    set r [expr {$i % 12}]
		    set st [expr {$i / 12}]
		    set list [concat $list ${r}x$st]
		}
	    } else {
		for {set i $s} {$i >= $e} {incr i -1} {
		    set r [expr {$i % 12}]
		    set st [expr {$i / 12}]
		    set list [concat $list ${r}x$st]
		}
	    }
	} elseif {[regexp {^(\d+)-(\d+)$} $v - s e]} {
	    if {$s <= $e} {
		for {set i $s} {$i <= $e} {incr i} {
		    set list [concat $list $i]
		}
	    } else {
		for {set i $s} {$i >= $e} {incr i -1} {
		    set list [concat $list $i]
		}
	    }
	} elseif {[regexp {^\(([.\d]+)-([.\d]+)\)$} $v - sl el]} {
	    set s [LegacyIndex $sl]
	    set e [LegacyIndex $el]
	    if {$s <= $e} {
		for {set i $s} {$i <= $e} {incr i} {
		    set list [concat $list ([lindex $UTF::LEGACY $i])]
		}
	    } else {
		for {set i $s} {$i >= $e} {incr i -1} {
		    set list [concat $list ([lindex $UTF::LEGACY $i])]
		}
	    }
	} else {
	    lappend list $v
	}
    }
    return $list
}

UTF::doc {
    # [call [cmd UTF::Rateexpand] [arg list]]

    # [cmd UTF::Rateexpand] expands a list of rate specifiers in a
    # similar way to [cmd UTF::Numexpand].  Parentheses can be used to
    # indicate Legacy rates, and ranges of Legacy rates will be
    # expanded appropriately.  Output consists of a concatenated list
    # of triples:

    # -r, -m or -v to specify legacy, mcs or vht rate used in the
    # various wl *rate commands

    # rate number to be used in the various wl *rate commands

    # rate specifier to be displayed[para]

    # Equivalent to [cmd UTF::Rateopt] [lb] [cmd UTF::Rateexp] [rb].

    # [example_begin]
    utf> UTF::Rateexpand {(36-54) 13-15 auto}
    -r 36 (36) -r 48 (48) -r 54 (54) -m 13 13 -m 14 14 -m 15 15 {} auto auto
    # [example_end]
}

proc UTF::Rateexpand {var} {
    UTF::Rateopt [UTF::Rateexp $var]
}

proc UTF::_ratecomp {a b} {
    if {![info exists UTF::_allrates]} {
	set ::UTF::_allrates {(1-54) 32 0-7 87 88 8-15 99 100 16-23 101 102 24-31 0x1-9x1 10x1 11x1 0x2-9x2 10x2 11x2 0x3-9x3 10x3 11x3 0x4-9x4 10x4 11x4 auto}
	set _i 0
	foreach _r [UTF::Rateexp $UTF::_allrates] {
	    set ::UTF::_rateindex($_r) $_i
	    incr _i
	}
	unset _r
	unset _i
    }
    set A $UTF::_rateindex($a)
    set B $UTF::_rateindex($b)
    if {$A < $B} {
	return -1
    } elseif {$A > $B} {
	return 1
    } else {
	return 0
    }
}

UTF::doc {
    # [call [cmd UTF::Ratesort] [arg list]]

    # [cmd UTF::Ratesort] expands a list of rate specifiers in a
    # similar way to [cmd UTF::Numexpand].  Parentheses can be used to
    # indicate Legacy rates, and ranges of Legacy rates will be
    # expanded appropriately.  Output consists of a list of rates
    # sorted appropriately for per-per-rate plots.

    # [example_begin]
    utf> UTF::Ratesort {13-15 (36-54) auto 32 88 100}
    (36) (48) (54) 32 88 13 14 15 100 auto
    # [example_end]
}

proc UTF::Ratesort {l} {
    lsort -command UTF::_ratecomp [UTF::Rateexp $l]
}


UTF::doc {
    # [call [cmd UTF::Staexpand] [arg list]]

    # [cmd UTF::Staexpand] expands abbreviations used in the -sta {}
    # list.  This is primarily for MBSS on routers, but may be used
    # for other types of virtual interfaces.  [para]

    # Pairs of the form "name%N dev%M" will be replaced by a list of
    # the form "name0 devM name1 dev(M+1) ... nameN dev(M+N)".  If "M"
    # is not specified, 0 is assumed.  Pairs without % are passed
    # through unchanged. [para]

    # Note: Linux routers tend to use wlx.y for MBSS device names, so
    # M can be ommitted.  Vx APs tend to need an offset for the second
    # radio, eg:

    # [example_begin]
    utf> UTF::Staexpand {4718/4322 wl1 4718/4322.%3 wl1.%}
    4718/4322 wl1 4718/4322.1 wl1.1 4718/4322.2 wl1.2 4718/4322.3 wl1.3
    utf> UTF::Staexpand {4718/4322vx wl1 4718/4322vx.%3 wlmbss%256}
    4718/4322vx wl1 4718/4322vx.1 wlmbss257 4718/4322vx.2 wlmbss258 4718/4322vx.3 wlmbss259
    # [example_end]
}

proc UTF::Staexpand {var} {
    set list {}
    foreach {sta dev} $var {
	if {[regexp {%(\d+)} $sta - mbss]} {
	    if {![regexp {%(\d+)} $dev - j]} {
		set j 0
	    }
	    # Replace % token with MBSS interfaces
	    for {set i 1; incr j} {$i <= $mbss} {incr i; incr j} {
		regsub {%\d+} $sta $i msta
		regsub {%\d*} $dev $j mdev
		lappend list $msta $mdev
	    }
	} else {
	    lappend list $sta $dev
	}
    }
    return $list
}

UTF::doc {
    # [call [cmd UTF::PerfChans] [arg AP] [arg STA] ...]

    # Pick a sample set of channels for performance testing based on
    # the AP and STA common capabilities.  For simultaneous DualBand
    # routers, [arg AP] may be specified as a list of two elements -
    # the 2g radio first and the 5g radio second.  For DPT tests, more
    # than one STA argument may be specified.
}

proc UTF::PerfChans {AP args} {
    # Pick a sample set of channels for performance testing based on
    # the AP and STA common capabilites

    switch [llength $AP] {
	1 {
	    set apc [$AP allchanspecs]
	}
	2 {
	    set apc [concat \
			 [[lindex $AP 0] allchanspecs -band b] \
			 [[lindex $AP 1] allchanspecs -band a]]
	}
	default {
	    error "AP should be one AP or a 2g/5g pair"
	}
    }
    set lists [list $apc]
    foreach STA $args {
	lappend lists [$STA allchanspecs]
    }
    puts $lists

    set com [UTF::Common {*}$lists]
    set perfchans ""

    # find first 80MHz channel
    if {[set c [lsearch -inline -regexp $com {/80}]] != ""} {
	lappend perfchans $c
    }

    # find 40MHz channel
    if {[lsearch $com {36l}] >= 0} {
	lappend perfchans 36l
    } elseif {[lsearch $com {3l}] >= 0} {
	lappend perfchans 3l
    }

    # find 20MHz channel
    if {[lsearch $com {3}] >= 0} {
	lappend perfchans 3
    }

    if {[llength $perfchans] < 2 && [lsearch $com {36}] >= 0} {
	lappend perfchans 36
    }

    if {$perfchans eq ""} {
	error "AP and STA have no suitable channels in common"
    }
    UTF::Message LOG "" "Picking $perfchans"
    set perfchans

}

UTF::doc {
    # [call [cmd UTF::Email] [arg to] [arg subject] [arg body]]

    # Send email message to recipient(s) [arg to] with subject [arg
    # subject] and message body [arg body].  [arg to] may be a single
    # recipient or a list.  @$UTF::smtpdomain will be appended to the
    # [arg to] recipients if needed.  The sender will be set to the
    # user running UTF.

    # If [arg body] starts with "<html>" the message will be sent with
    # a content-type of "text/html", otherwise the message will be
    # sent as "text/plain".

}

proc UTF::Email {to subject body} {

    package require smtp
    package require mime

    if {$to eq ""} {
	set to $::tcl_platform(user)
    }

    # Add domain if neccessary
    foreach i [lsearch -all -not $to {*@*}] {
	lset to $i "[lindex $to $i]@$UTF::smtpdomain"
    }
    if {[regexp -nocase {<html[^>]*>} $body]} {
	set type html
    } else {
	set type plain
    }

    if {[lsearch $to "hnd-utf-list.pdl@broadcom.com"] >= 0} {
	# Special case for annotated reports
	set replyto "hnd-utf-list.pdl@broadcom.com, hnd-utf-annotate-list.pdl@broadcom.com"
    } else {
	# Default reply to sender
	set replyto "$::tcl_platform(user)@$UTF::smtpdomain"
    }

    # Use base64 encoding because quoted-printable causes excessive
    # CPU and memory usage on very large html documents.
    set token [mime::initialize -encoding base64 -canonical text/$type \
		   -string $body]
    if {![regexp {(.*):(.*)} $UTF::smtphost - host port]} {
	set host $UTF::smtphost
	set port 25
    }
    UTF::Message LOG "" "Email to: $to; subject: \[UnitTest\] ${subject}"
    smtp::sendmessage $token \
	-debug 0 \
	-header [list From "$::tcl_platform(user)@$UTF::smtpdomain"] \
	-header [list To [join $to ", "]] \
	-header [list Reply-To $replyto] \
	-header [list Subject "\[UnitTest\] ${subject}"] \
	-servers $host -ports $port
    mime::finalize $token
}

UTF::doc {
    # [call [cmd UTF::EmailFile] [arg to] [arg subject] [arg file]]

    # Similar to [cmd UTF::Email] except that the message body is
    # read from [arg file].
}

proc UTF::EmailFile {to subject file} {
    set fd [open $file r]
    set body [read $fd]
    close $fd
    UTF::Email $to $subject $body
}

UTF::doc {
    # [call [cmd UTF::Endpoints] [arg In] [arg Out]]

    # Returns an even length list consisting of all pairs of endpoints
    # SRC and SNK such that SRC is a peer of In and SNK is a peer of
    # Out and SRC and SNK are not on the same host.  Used for
    # performance testing where endpoints are needed to source and
    # sink data flowing though In and Out.  If either argument has no
    # peers, the argument itself will be used as the endpoint.

    # [example_begin]
    utf> MIMO_0 configure -peer UTFTestB1
    utf> MIMO_2 configure -peer {dhcpl7-sv1-135_2 dhcpl7-sv1-206_2}
    utf> UTF::Endpoints MIMO_2 MIMO_1
    dhcpl7-sv1-135_2 UTFTestA1 dhcpl7-sv1-206_2 UTFTestA1
    # [example_end]
}
proc UTF::Endpoints {In Out} {
    set l {}
    set SRCS [$In cget -peer]
    set SNKS [$Out cget -peer]
    # If there's no peer, or the peer is the other endpoint, use the
    # interface itself
    if {$SRCS eq "disconnected!" || $SRCS eq $Out} {
	    set SRCS $In
    }
    if {$SNKS eq "disconnected!" || $SNKS eq $In} {
	    set SNKS $Out
    }

    forall SRC $SRCS SNK $SNKS {
	if {[$SRC cget -host] ne [$SNK cget -host]} {
	    lappend l $SRC $SNK
	}
    }
    return $l
}


UTF::doc {
    # [call [cmd UTF::Getopts] [arg OPTS] [lb][arg usage][rb]
    #    [lb][arg args][rb]]

    # Parse argument list named in [arg args] (default "args") using
    # options list [arg OPTS].  Results are returned in the unnamed
    # array.  Unrecognised arguments will cause an error and a usage
    # message.  [arg usage] can be used to change the header of the
    # usage message (default "options: ").
    # [list_begin definitions]

    # [call [option xx.arg] [arg default] [arg "Description"]]
    # Declares an option with an argument.
    # [call [option xx.parg] [arg default] [arg "Description"]]
    # Same as "xx.arg" except that the default value will be
    # inherited from the caller's parent, if it exists.
    # [call [option xx.nonly] [arg "Description"]]
    # Expands to a pair of boolean options -noxx and -xxonly, useful for
    # selection of subtests.
    # [call [option xx] [arg "Description"]]
    # Options with no suffix declare simple booleans, default false.
    # [list_end]

    # [example_begin]
    UTF::Getopts {
	{verbose "Show messages"}
	{security.arg "open" "security"}
	{perfloop.parg "1" "Perf loop (inherited)"}
    } "Simple example\noptions: "
    if {$(verbose)} {
	puts "Verbose is set"
    }
    puts "security is: $(security)"
    # [example_end]
}

# Blank out getArgv0, since it's inappropriate for proc args
namespace eval ::cmdline {proc getArgv0 {} {}}

proc UTF::Getopts {OPTS {usage options:} {ARGS args}} {
    upvar $ARGS args
    upvar {} {}
    upvar 2 {} parent
    set opts {}
    foreach o $OPTS {
	set key [lindex $o 0]
	set desc [lindex $o end]
	if {[regexp {(.*)\.nonly} $key - key]} {
	    lappend opts \
		[list no$key "No $desc"] \
		[list ${key}only "Only $desc"]
	} elseif {[regexp {(.*)\.parg} $key - key]} {
	    if {[info exists parent($key)]} {
		set val $parent($key)
	    } else {
		set val [lindex $o 1]
	    }
	    lappend opts [list $key.arg $val $desc]
	} else {
	    lappend opts $o
	}
    }
    set ret ""
    if {[catch {
	array set {} [cmdline::getoptions args $opts $usage]
    } ret] && ![llength $args]} {
	return -code error $ret
    }
    if {[llength $args]} {
	return -code error "Bad option [lindex $args 0]\n$ret"
    }
}

UTF::doc {
    # [call [cmd UTF::GetKnownopts] [arg OPTS] [lb][arg usage][rb]
    #    [lb][arg args][rb]]

    # Same as Getopts except unrecognised options are left in the args
    # list.
}

proc UTF::GetKnownopts {OPTS {usage options:} {ARGS args}} {
    upvar $ARGS args
    upvar {} {}
    array set {} [cmdline::getKnownOptions args $OPTS $usage]
}

proc UTF::MonitorNVRAM {AP block} {
    array set before [$AP getnvram]
    uplevel 1 $block
    array set after [$AP getnvram]
    # Changed entries
    foreach n [array names after] {
	if {[info exists before($n)]} {
	    if {$before($n) eq $after($n)} {
		unset after($n)
	    }
	    unset before($n)
	}
    }
    # Delete entries
    foreach n [array names before] {
	set after($n) ""
    }
    array get after
}

UTF::doc {
    # [call [cmd UTF::Common] [arg args]]

    # Returns all entries common to lists in args, without duplicates.
}

proc UTF::Common {args} {
    set common {}
    foreach element [lindex $args 0] {
	if {[lsearch -exact $common $element] >= 0} {
	    # Remove duplicates
	    continue
	}
	set found 1
	foreach list [lrange $args 1 end] {
	    if {[lsearch -exact $list $element] < 0} {
		set found 0; break
	    }
	}
	if {$found} {lappend common $element}
    }
    return $common
}

UTF::doc {
    # [call [cmd UTF::MeanMinMax] [arg list]]

    # Returns three values, the [emph Mean], the [emph Minimum], and
    # the [emph Maximum] of the values in the input list.  This is
    # used in Control Chart generation.
}

proc UTF::MeanMinMax {list} {
    set sum 0.0
    set num 0
    foreach p $list {
	set sum [expr {$sum + $p}]
	if {$num} {
	    if {$p < $min} {
		set min $p
	    }
	    if {$p > $max} {
		set max $p
	    }
	} else {
	    set max $p
	    set min $p
	}
	incr num
    }
    if {$num} {
	list [expr {$sum/$num}] $min $max
    } else {
	list 0 0 0
    }
}

proc UTF::BranchName {path} {
    if {[regexp {(NIGHTLY|TRUNK|trunk)/} $path]} {
	return TOT
    } elseif {[regexp {[^/]*_(BRANCH|TWIG)_[^/]*} $path name]} {
	return $name
    } elseif {[regexp {([^/]*)_REL_(\d+_\d+_\d+)_} $path - name rev]} {
	return "${name}_TWIG_$rev"
    } elseif {[regexp {([^/]*)_REL_(\d+_\d+)_} $path - name rev]} {
	return "${name}_BRANCH_$rev"
    } elseif {[regexp {^([A-Z]\w+)/\d+$} $path - name]} {
	# DSL
	return ${name}
    } else {
	# No match
	UTF::Message WARN "" "No tag"
	return $path
    }
}

UTF::doc {
    # [call [cmd UTF::CheckImage] [arg STA] [lb][arg date][rb] [lb][arg cols][rb]]

    # Locates the driver images to test (via [method findimages]) and
    # compares with [arg date].  Creates a report header describing
    # the STA and the test image, with optional supplied extra
    # columns.  Old images will be flagged as such in the header and
    # reported as a FAIL.
}

proc UTF::CheckImage {STA {date ""} {cols ""}} {
    if {[catch {$STA findimages} path]} {
	if {[info exists ::UTF::Summary]} {
	    $UTF::Summary header [$STA cget -name] {*}$cols "None Found" \
		{} {} {}
	}
	throw FAIL "None Found"
    } else {
	UTF::Message INFO [$STA cget -name] $path
	if {[llength [$STA cget -image]] <= 1} {
	    # Only optimize out image search if the spec is simple
	    $STA configure -image $path
	}
	$STA targetname $path
	$STA preserved $path
	if {![regsub {.*/(USERS/)} $path {\1} id] &&
	    ![regsub {.*build_[^/]+/(PRESERVED/|ARCHIVED/)?} $path {} id]} {
	    regsub {.*/Irvine/([^/]+)/\1_*(.*)/images/.*} $path {\1_\2} id
	}
	regsub {^http.*/job/} $id {} id
	regsub {/build[^_]*/.*|(?:/\d+)?/src.*|/release.*|/bcm\w+_fs_kernel|/hybrid-.*|/artifact/.*} $id {} id
	$STA configure -_path $id
	if {![regexp "^$::UTF::projswbuild/build_" $path] ||
	    ![regexp {_BRANCH_|_TWIG_|NIGHTLY|TRUNK|trunk} $id] ||
	    $date eq "" ||
	    [string match "*/$date" $id] || [string match "*/$date.*" $id]} {
	    if {[info exists ::UTF::Summary]} {
		$UTF::Summary header [$STA cget -name] {*}$cols $id \
		    {} {} {}
		return
	    } else {
		return $id
	    }
	} else {
	    BuildFile::stat $path stat
	    # Use the older of ctime and mtime
	    if {$stat(mtime) < $stat(ctime)} {
		set create $stat(mtime)
	    } else {
		set create $stat(ctime)
	    }
	    set days [expr {([clock seconds] - $create) / 86400}]
	    if {$days < 2} {
		# 1 day old includes yesterday even if less than 24 hours ago.
		set old "1 day old"
		set oldh "1&nbsp;day&nbsp;old"
	    } else {
		set old "$days days old"
		set oldh "$days&nbsp;days&nbsp;old"
	    }
	    if {[info exists ::UTF::Summary]} {
		$UTF::Summary header [$STA cget -name] {*}$cols $id \
		    {} $oldh {}
		throw FAIL "$old"
	    } else {
		throw FAIL "$id $old"
	    }
	}
    }
}

UTF::doc {
    # [call [cmd UTF::ReportWhatami] [arg STA] [lb][arg -role] [arg
    #      {DUT/REF}][rb] [lb][arg -noload] [arg {0/1}][rb]]

    # Updates a report header with HW-specific imformation, collected
    # by [method whatami].  If the test is registered with the UTFDB,
    # the information will also be passed to the DB.  [arg -role]
    # Should be used to indicate to the DB if the STA is a DUT or a
    # REF fopr this test.  [arg -noload] indicates whether to update
    # an existing report header (eg from [cmd UTF::Checkimages]) or
    # to create a new header (eg if -nostaload was specified to a test
    # script).
}

proc UTF::ReportWhatami {STA args} {
    UTF::Getopts {
	{role.arg "DUT" "Role (DUT/REF"}
	{noload.arg "Driver load information not already reported"}
    }
    set what [$STA whatami]
    # Pull out chip info and replace whitespace to make it
    # easier for external parsers.
    regsub -all {\s+} [lreplace $what 0 0] {_} w
    if {$(noload)} {
	$UTF::Summary header $STA $w
    } else {
	$UTF::Summary header_update 1 $w
    }
    $STA dbrecord $(role)
    set what
}

UTF::doc {
    # [call [cmd UTF::ReportDriver] [arg STA] [lb][arg date][rb]]

    # Similar to [cmd UTF::CheckImages] but for supplimental drivers.
    # For example, in a Full Dongle test, [cmd UTF::CheckImages]
    # would report on the Firmware while [cmd UTF::ReportDriver]
    # would report on the DHD.  The results will be inserted in the
    # header previously created by [cmd UTF::CheckImages].
}

proc UTF::ReportDriver {STA {date ""}} {
    if {![info exists ::UTF::Summary]} {
	return
    }
    if {![$STA hostis DHD STB Panda]} {
	# No seperate host driver to report
	return
    }
    if {[$STA hostis STB] && [$STA cget -dongleimage] eq "wl.ko"} {
	# STB in NIC mode
	return
    }
    set path [$STA driver]

    regsub {.*build_[^/]+/(PRESERVED/|ARCHIVED/)?} $path {} id
    regsub {/build.*|/src.*|/release.*|/bcm\w+_fs_kernel|/hybrid-.*} $id {} id
    regsub {^([^/]+)/[^/]+/(\d+\.\d+\.\d+\.\d+).*} $id {\1:\2} id

    if {![regexp "^$::UTF::projswbuild/build_" $path] ||
	![regexp {_BRANCH_|_TWIG_|NIGHTLY|TRUNK|trunk} $id] ||
	$date eq "" ||
	[string match "*:$date" $id] || [string match "*:$date.*" $id]} {
	$UTF::Summary header_update 3 $id
	return
    } else {
	BuildFile::stat $path stat
	# Use the older of ctime and mtime
	if {$stat(mtime) < $stat(ctime)} {
	    set create $stat(mtime)
	} else {
	    set create $stat(ctime)
	}
	set days [expr {([clock seconds] - $create) / 86400}]
	if {$days < 2} {
	    # 1 day old includes yesterday even if less than 24 hours ago.
	    set old "1 day old"
	    set oldh "1&nbsp;day&nbsp;old"
	} else {
	    set old "$days days old"
	    set oldh "$days&nbsp;days&nbsp;old"
	}
	$UTF::Summary header_update 3 $id
	$UTF::Summary header_update 5 $oldh
	UTF::Message WARN $STA "$id is $old"
    }
}

proc UTF::dBuxRegister {} {
    set c "curl -s -S"
    lappend c -d testTime=[clock format $UTF::_start_time -format {%Y-%m-%d %H:%M:%S}]
    set script $::argv0
    regsub {.*/unittest/+} $script {} script; # trim full path
    lappend c -d "script=$script"
    lappend c -d stationName=[file rootname [file tail $UTF::args(utfconf)]]
    lappend c -d "userName=$::tcl_platform(user)"
    lappend c -d "arguments=$::argv"
    lappend c -d "email=tima@broadcom.com"
    lappend c -d "url=[UTF::LogURL summary.html]"
    lappend c -d frameworkName=UTF
    lappend c -d functionName=Nightly
    lappend c -d elapsedTime=0

    lappend c "http://wlan-systems.sj.broadcom.com/api/index.php/api/ci/session"
    catch {localhost $c} ret

    if {[regexp {"code":"100.*id":"(\d+)"} $ret - ::UTF::dBid]} {
	UTF::Message INFO "" "dBux ID: $::UTF::dBid"
    } else {
	set f [open $UTF::Logdir/dbuxerr.html a+]
	puts $f $ret
	close $f
    }
}

proc UTF::dBuxResults {pass fail} {
    if {![info exists ::UTF::dBid]} {
	return
    }

    set c "curl -s -S"
    lappend c -d id=$::UTF::dBid
    lappend c -d passcount=$pass
    lappend c -d failcount=$fail
    lappend c -d elapsedTime=[expr {[clock seconds] - $::UTF::_start_time}]

    lappend c "http://wlan-systems.sj.broadcom.com/api/index.php/api/ci/session"
    catch {localhost $c} ret

    if {[regexp {"Status":"Success"} $ret]} {
	UTF::Message INFO "" "dBux updated"
    } else {
	set f [open $UTF::Logdir/dbuxerr.html a+]
	puts $f $ret
	close $f
    }
}

proc UTF::dBRecord {args} {
    if {![info exists ::UTF::dBid]} {
	return
    }

    set c "curl -s -S"
    lappend c -d id=$::UTF::dBid
    foreach {k v} $args {
	lappend c -d $k=$v
    }
    lappend c "http://wlan-systems.sj.broadcom.com/api/index.php/api/ci/session"
    catch {localhost $c} ret

    if {[regexp {"code":"100"} $ret]} {
	UTF::Message INFO "" "dBux updated"
    } else {
	set f [open $UTF::Logdir/dbuxerr.html a+]
	puts $f $ret
	close $f
    }
}



proc UTF::TestRegister {} {

    # need to replace the Cc readonly with a generic flag
    if {[info exists ::UTF::ControlChart::readonly] && \
	    $::UTF::ControlChart::readonly} {
	UTF::Message INFO "" \
	    "Running with nocache - not registering with Database"
	return
    }
    set script $::argv0
    regsub {.*/unittest/+} $script {} script; # trim full path

    set rig [file rootname [file tail $UTF::args(utfconf)]]

    set insert_cmd "insert into test set rig='$rig',script='$script',args='$::argv',user='$::tcl_platform(user)',session='$UTF::SessionID',URL='[UTF::LogURL summary.html]'"
    set select_cmd "select last_insert_id()"

    if {[auto_execok mysql] eq ""} {
	UTF::Message INFO "" "mysql not installed"
	return
    }
    set cmd "$insert_cmd; $select_cmd"
    if {[catch {
	localhost rexec \
	    [list mysql -s -s -h sr1end01.sj.broadcom.com -u utf --password=utf -e $cmd utf]
    } ret]} {
	UTF::Message WARN "" $ret
    } else {
	set ::UTF::DBid $ret
	UTF::Message INFO "" "Database ID: $UTF::DBid"
    }
}

proc UTF::TestResults {pass fail elapsed} {
    if {![info exists ::UTF::DBid]} {
	return
    }
    set cmd "update test set pass=$pass,fail=$fail,elapsed='$elapsed' where id=$::UTF::DBid;"
    if {[catch {
	localhost rexec \
	    [list mysql -s -s -h sr1end01.sj.broadcom.com -u utf --password=utf -e $cmd utf]
    } ret]} {
	UTF::Message WARN "" $ret
    }
}

proc UTF::DBRecord {args} {
    foreach {k v} $args {
	lappend sets "$k='$v'"
    }
    set cmd "insert into dut set [join $sets {, }];"
    set m [open "|mysql -h sr1end01.sj.broadcom.com -u utf --password=utf utf" w]
    UTF::Message INFO "" $cmd
    puts $m $cmd
    if {[catch {close $m} ret]} {
	UTF::Message WARN "" $ret
    }
}

proc UTF::DBDump {} {
    set cmd "select * from dut;"
    set m [open "|mysql -h sr1end01.sj.broadcom.com -u utf --password=utf utf" w]
    UTF::Message INFO "" $cmd
    puts $m $cmd
    close $m
}

UTF::doc {
    # [call [cmd UTF::BuildTitle] [arg STAS]]

    # Returns a comma seperated string representing the builds used by
    # all the listed STAs, for use in test titles. Tags will have
    # static text removed, leaving just numbers eg: "TOT, 5_90,
    # 4_230".  Private builds will be left intact.
}

proc UTF::BuildTitle {STAS} {
    set build {}

    foreach S $STAS {
	if {[set b [$S cget -image]] eq ""} {
	    set b [$S cget -tag]
	    if {$b eq "NIGHTLY" || $b eq "TRUNK" || $b eq "trunk"} {
		set b "TOT"
	    } else {
		regsub {^[^_]+_(?:{?(?:REL|BRANCH|TWIG)(?:,(?:REL|BRANCH|TWIG))?}?)_} \
		    $b {} b
		regsub {{,_[\*\?]+}|{_[\*\?]+,}} $b {} b
	    }
	}
	if {[lsearch $build $b] < 0} {
	    lappend build $b
	}
    }
    return [join $build ", "]
}

UTF::doc {
    # [call [cmd UTF::PrivateBuildTag] [arg file]]

    # Returns the TAG used to check out a private build tree.  [arg
    # file] should be any checked-out file in the tree
}

proc UTF::PrivateBuildTag {file} {
    # Use explicit path for svn to avoid broken OSS tools
    if {![regexp -line {^URL: (.*)} \
	      [exec $::UTF::projtools/linux/bin/svn info $file] - url]} {
	error "SVN URL not found"
    }
    if {[regexp {wlansvn/proj/branches/([^/]+)} $url - branch]} {
	return $branch
    } elseif {[regexp {wlansvn/proj/trunk} $url]} {
	return NIGHTLY
    } else {
	error "Unexpected URL: $url"
    }
}

proc UTF::ContRvRiperf {PAIR args} {
    UTF::GetKnownopts {
        {noping "Skip pretest ping check"}
        {means "Report mean values, instead of intermediate datapoints"}
        {t.arg 10 "Timeout"}
        {direction.arg "Up" "Traffic direction Upstream or Downstream"}
        {startAttn.arg 0 "StartAttn for AP"}
        {stopAttn.arg 28 "StopAttn for AP"}
        {dt.arg 5 "Time to dwell before stepping attenuation"}
	{va.arg "Aflex" "variable attenuator name"}
        {w.arg "" "Window size"}
    }
    lappend args -t $(t)
    set Timeout [expr {int($(t)*1000 + 30000)}]

    set grp1Attn $(startAttn)
	set stepUp 1

    $(va) createGroup blah "1 2 3 4"

    set cmds {}

    set SRC [lindex $PAIR 0]
    set SNK [lindex $PAIR 1]

    set sargs $args
    if {$(w) ne ""} {
	$SNK tcptune $(w)
	if {[$SRC tcptune $(w)] && $(w) ne 0} {
	    lappend sargs -w $(w)
	}
    }
    if {![info exists ip($SNK)]} {
	set ip($SNK) [$SNK ipaddr]
    }
    if {!$(noping)} {
	# Longer ping to avoid slow setup issues with PM=1 on M93xp
	# XXX Revisit this to determine if it's a regression.
	$SRC ping $ip($SNK) -c 10
    }
    lappend cmds [concat [list $SRC] rpopen iperf -c $ip($SNK) -fb $sargs]

    set pair 0
    foreach cmd $cmds {
        set fd [eval $cmd]
        # Set up nonblocking reader event
        fconfigure $fd -blocking 0
        fileevent $fd readable {set reading READY}

        set src($pair) $SRC
        set fds($fd) 1
        set pairs($fd) $pair
        lappend pids [pid $fd]
        incr pair
    }

    # Start Timer
    set timer [after $Timeout exec kill $pids]

    set count [expr $(dt) + 7]
    # set count [expr 1 + 7]
    while {[array names fds] ne {}} {
        vwait reading
        foreach fd [array names fds] {
            set msg [gets $fd]
            if {[eof $fd]} {
                # Put fd back to blocking so that close can get a
                # valid return status
                fconfigure $fd -blocking 1
		if {[catch {close $fd} ret]} {
                    UTF::Message LOG $src($pairs($fd)) $ret
                }
                unset fds($fd)
            } elseif {![fblocked $fd]} {
                set count [expr $count -1]
                if {$count == 0} {
		    UTF::Message LOG Elapse "Step Attenuator"
		    if {$grp1Attn==$(stopAttn)} {
			set stepUp 0
		    }

		    # step up grp1
		    if {$stepUp == 1} {
			incr grp1Attn
		    } else {
			set grp1Attn [expr $grp1Attn -1]
			if {$grp1Attn < 0} {
			    set grp1Attn 0
			}
		    }

		    $(va) setGrpAttn blah $grp1Attn
		    set count $(dt)
		    # set count 1
		    # Here we put code to perform "wl rssi" on STA and
		    # append results to staRssiList so staRssiList
		    # contains rssi value as measured by the sta on
		    # every attn level
		    if {$(direction) == "Up"} {
			set STA $SRC
		    } else {
			set STA $SNK
		    }
		    set catch_resp [catch {$STA wl rssi} catch_msg]
		    # puts "catch_msg = $catch_msg"
		    set assocResult [regexp {rx pkt:\ ([0-9]+)\ kbps} $catch_msg match]
		    if {$assocResult == 1} {
			set wlSta_getRssi_retVal 0
		    } else {
			set catch_resp [catch {$STA wl rssi} catch_msg]
			set result [regexp {(-?[0-9]+)} $catch_msg match rssi]
			set wlSta_getRssi_retVal $rssi
		    }
		    lappend staRssiList $wlSta_getRssi_retVal
                }
                append data($pairs($fd)) "$msg\n"
                UTF::Message LOG $src($pairs($fd)) $msg
            }
        }
    }
    after cancel $timer

    set means {}
    set tot {}
    foreach pair [lsort [array names data]] {
        set i 0
        set results {}
        foreach p [regexp -inline -all {[\d.]+(?= .?bits/sec)} $data($pair)] {
            set point [expr {$p / 1000000.0}]
            lappend results $point
            if {[llength $tot] > $i} {
                set tot [lreplace $tot $i $i [expr {[lindex $tot $i]+$point}]]
            } else {
                lappend tot $point
            }
            incr i
        }
        # Drop last (mean)
        lappend means [lindex $results end]
        set results [lreplace $results end end]
        UTF::Message LOG $src($pair) "Results: $results"
    }
    if {$tot eq {}} {
        set tot {0 0}
    }

    # Drop last (mean)
    lappend means [lindex $tot end]
    set tot [lreplace $tot end end]
    UTF::Message LOG Totals "Results: $tot"

    if {$(means)} {
		set roamIperfList {}
		lappend roamIperfList $means
		lappend roamIperfList $staRssiList
        return $roamIperfList
    } else {
		set roamIperfList {}
		lappend roamIperfList $tot
		lappend roamIperfList $staRssiList
        return $roamIperfList
    }

    UTF::doc {
	# [call [arg host] [method nvram_add] [arg src]]

	# Utility method for merging -nvram_add entries into nvram.txt
	# files.  Returns the name of the new nvram.txt file, or the
	# old one if there were no changes.  The special value
	# "DELETE" can be used to completely remove an nvram key for
	# cases where setting it to an empty value is not sufficient.
	# Also the keyword "decomment" can be used to strip comments
	# from the nvram file, which may be needed for -extnvm- NIC
	# drivers.
    }
}

proc UTF::nvram_add {src replace} {
    if {[set replace [UTF::decomment $replace]] eq ""} {
	return $src
    }

    set strip 0
    set nl "\n"
    foreach n $replace {
	if {$n eq "decomment"} {
	    set strip 1
	} elseif {$n eq "serialize"} {
	    set nl "\0";
	    set add(NVRAMRev) "DELETE"
	    set strip 1
	} elseif {[regexp {([^=]+)=(.*)} $n - k v]} {
	    set add($k) $v
	} else {
	    error "bad nvram_add entry: $n"
	}
    }
    UTF::Message LOG "" "Open $src"
    set ret "";
    if {[file extension $src] eq ".gz"} {
	set src "|gunzip -c $src"
    }
    set dst [exec mktemp /tmp/nvram.txt_XXXXX]
    set chan [open $src]
    set dstchan [open $dst w]
    while {[gets $chan line] >= 0} {
	if {$strip && [regexp {^(?:\#|$)} $line]} {
	    UTF::Message LOG "" "Stripped $line"
	    continue
	}
	foreach k [array names add] {
	    if {[regexp "^$k=" $line]} {
		if {$add($k) ne "DELETE"} {
		    UTF::Message LOG "" "$line => $add($k)"
		    set line "$k=$add($k)"
		} else {
		    UTF::Message LOG "" "$line deleted"
		    unset line
		}
		unset add($k)
		break
	    }
	}
	if {[info exists line]} {
	    append ret "$line$nl"
	}
    }
    foreach k [array names add] {
	UTF::Message LOG "" "Add $k=$add($k)"
	append ret "$k=$add($k)$nl"
    }
    puts -nonewline $dstchan $ret
    close $dstchan

    return $dst
}

UTF::doc {
    # [call [cmd UTF::CountBitsSets] [arg i]]
    #
    # Returns the number of bits set in the argument
}
proc UTF::CountBitsSet {v} {
    for {set c 0} {$v} {incr c} {
	set v [expr {$v & ($v - 1)}]
    }
    set c
}

UTF::doc {
    # [call [cmd UTF::ParseDumpRSDB] [arg dump] [lb][arg CORE][rb]]

    # Utility function to extract ssid->core mapping from an rsdb
    # dump.  [arg dump] should be the results of [cmd {wl dump rsdb}].
    # Results are returned as a flattened array, and can optionally
    # also be returned in the array named by the [arg CORE] argument.
}

proc UTF::ParseDumpRSDB {dump {CORE ""}} {
    if {$CORE ne ""} {
	upvar $CORE core
    }
    array set core {}
    foreach l [split $dump \n] {
	switch -regexp -matchvar m $l {
	    {wlc:(\d)} {
		set wlc [lindex $m 1]
	    }
	    {BSSCFG (\d): "(.*)"} {
		set ssid [lindex $m 2]
	    }
	    {STA enable 1} {
		set core($ssid) "$wlc"
	    }
	}
    }
    array get ret
}

UTF::doc {
    # [call [cmd UTF::ParseAMPDURates] [arg dump] [lb][arg MCS][rb]
    #	      [lb][arg SGI][rb]]

    # Utility function to extract mcs and sgi rate histograms from an
    # ampdu dump.  [arg dump] should be the results of [cmd {wl dump
    # ampdu}].  Results are returned as a pair of flattened arrays,
    # and can optionally also be returned in arrays names by the [arg
    # MCS] and [arg SGI] arguments.

}



proc UTF::ParseAMPDURates {dump {MCS ""} {SGI ""}} {
    if {$MCS ne ""} {
	upvar $MCS mcs
    }
    if {$SGI ne ""} {
	upvar $SGI sgi
    }
    # Extract MCS rate histogram
    for {set m 0} {$m < 24} {incr m} {
	set mcs($m) 0
	set sgi($m) 0
    }
    if {![regexp {TX MCS  :  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)} \
	      $dump - \
	      mcs(0) mcs(1) mcs(2) mcs(3) mcs(4) mcs(5) mcs(6) mcs(7) \
	      mcs(8) mcs(9) mcs(10) mcs(11) mcs(12) mcs(13) mcs(14) mcs(15) \
	      mcs(16) mcs(17) mcs(18) mcs(19) mcs(20) mcs(21) mcs(22) mcs(23)]} {
	# If 3row fails, try older 2row
	regexp {TX MCS  :  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)} \
	    $dump - \
	    mcs(0) mcs(1) mcs(2) mcs(3) mcs(4) mcs(5) mcs(6) mcs(7) \
	    mcs(8) mcs(9) mcs(10) mcs(11) mcs(12) mcs(13) mcs(14) mcs(15)
    }

    # Check to see if sgi was used
    if {![regexp {TX MCS SGI:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)} \
	      $dump - \
	      sgi(0) sgi(1) sgi(2) sgi(3) sgi(4) sgi(5) sgi(6) sgi(7) \
	      sgi(8) sgi(9) sgi(10) sgi(11) sgi(12) sgi(13) sgi(14) sgi(15) \
	      sgi(16) sgi(17) sgi(18) sgi(19) sgi(20) sgi(21) sgi(22) sgi(23)]} {
	# If 3row fails, try older 2row
	if {![regexp {TX MCS SGI:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)\s+:  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)  (\d+)\(\d+%\)} \
		  $dump - \
		  sgi(0) sgi(1) sgi(2) sgi(3) sgi(4) sgi(5) sgi(6) sgi(7) \
		  sgi(8) sgi(9) sgi(10) sgi(11) sgi(12) sgi(13) sgi(14) sgi(15)]} {
	    # Old version of dump ampdu doesn't list SGI rates
	    # explicitly so guess on 15 and 7
	    if {[regexp {txampdu_sgi (\d+)} $dump - s]} {
		if {$mcs(15) > 0} {
		    set sgi(15) $s
		}
		if {$mcs(7) > 0} {
		    set sgi(7) $s
		}
	    }
	}
    }
    list [array get mcs] [array get sgi]
}

UTF::doc {
    # [call [cmd UTF::ParseAMPDUDensity] [arg dump] [lb][arg DEN][rb]]

    # Utility function to extract ampdu density histogram from an
    # ampdu dump.  [arg dump] should be the results of [cmd {wl dump
    # ampdu}].  Results are returned as percentages in an ordered list
    # from 1 mpdu/ampdu up to the max ampdu/mpdu the driver is
    # currently using.  The results can optionally also be returned in
    # the list named by the [arg DEN] argument.
}

proc UTF::ParseAMPDUDensity {dump {DEN ""}} {

    if {$DEN ne ""} {
	upvar $DEN den
    }

    # Initialise
    set den {}

    # First strip out continuator line breaks
    regsub -all {\n\s+:} $dump { } dump

    # MPDUdens dump is variable length so we can't just use a regexp

    if {[regexp -line {MPDUdens:\s+(.*)} $dump - dlist]} {
	foreach a $dlist {
	    # Ignore percentages.  We'll compute our own to get better
	    # resolution
	    if {[string is integer $a]} {
		lappend den $a
	    }
	}
    }

    # Normalise to %
    set total 0
    foreach a $den {
	incr total $a
    }
    if {$total != 0} {
	foreach a $den {
	    lappend pct [expr {100.0 * $a / $total}]
	}
	set den $pct
    }

    set den
}

UTF::doc {
    # [call [cmd UTF::ParseAMPDUmaxPER] [arg dump]]

    # Utility function to extract the largest PER % from an ampdu
    # dump.
}

proc UTF::ParseAMPDUmaxPER {dmp} {
    # First strip out continuator line breaks
    regsub -all {\n\s+:} $dmp { } dmp
    set max 0
    if {[regexp -line {.* PER :\s+(\d.*)} $dmp line dlist]} {
	UTF::Message LOG "" $line
	foreach a $dlist {
	    if {[regexp {\d+\((\d+)%\)} $a - p]} {
		if {$p > $max} {
		    set max $p
		}
	    } else {
		UTF::Message WARN "" "Unparseable: $a"
	    }
	}
    }
    UTF::Message LOG "" "Max PER: $max"
    return $max
}


UTF::doc {
    # [call [cmd UTF::ParmStrip] [arg array]]

    # Returns the specified [arg array] as a list of name-value pairs,
    # similar to [cmd {array get}], except that the names are
    # sorted and null values are ommitted.

   # [example_begin]
    utf> array set A {key1 1 key3 "" key4 2}
    utf> array get A
    key3 {} key4 2 key1 1
    utf> UTF::ParmStrip A
    key1 1 key4 2
    utf>
    # [example_end]
}

proc UTF::ParmStrip {A} {
    upvar $A a
    set l {}
    foreach k [lsort [array names a]] {
	if {$a($k) ne ""} {
	    lappend l $k $a($k)
	}
    }
    return $l
}

UTF::doc {
    # [call [cmd UTF::resolve] [lb][arg host][rb]]

    # Caching host name to IP address resolver.  [arg host] defaults
    # to the name of the controller.
}

proc UTF::resolve {{host ""}} {
    if {[info exists ::UTF::__hosts($host)]} {
	set ::UTF::__hosts($host)
    } elseif {$host eq ""} {
	set ::UTF::__hosts() [UTF::resolve [info hostname]]
    } elseif {[regexp {^\d+\.\d+\.\d+\.\d+$} $host]} {
	set ::UTF::__hosts($host) $host
    } else {
	set ::UTF::__hosts($host) \
	    [lindex [localhost rexec -q -s getent hosts $host] 0]
    }
}

proc UTF::MemSave {mem test parmlist} {
    array set parm $parmlist
    set logfile $UTF::Logfile
    file mkdir $::UTF::Perfache
    catch {file attributes $options(-perfcache) -permissions 02777}
    UTF::Logfile [file join $::UTF::Perfcache "$test.log"]
    UTF::Assert -x -r $mem {^\d} $test [UTF::ParmStrip parm]
    catch {file attributes $UTF::Logfile -permissions 00666}
    UTF::Logfile $logfile
}

proc UTF::Ktrap {msg} {
    package require http
    set data [http::formatQuery "Message" $msg]
    set ret [localhost wget -nv -O- --post-data=$data http://kbd3/f2.php]
}

UTF::doc {
    # [call [cmd UTF::GnuplotVersion]]

    # Returns the version of the Gnuplot command as a decimal number,
    # eg 4.2.
}
proc UTF::GnuplotVersion {} {
    if {![info exists ::UTF::GnuplotVersion]} {
	set ret [exec $::UTF::Gnuplot --version]
	if {![regexp {gnuplot (\d+\.\d+)} $ret -- ::UTF::GnuplotVersion]} {
	    error "GnuplotVersion not found: $ret"
	}
	UTF::Message INFO "" \
	    "GnuplotVersion: $::UTF::Gnuplot $::UTF::GnuplotVersion"
    }
    set ::UTF::GnuplotVersion
}

proc UTF::CanonicalChanspec {spec} {
    set spec [lindex $spec 0]; # Strip hex, if provided
    # Convert legacy format
    if {[regexp {^(\d+)([ab]?)([ul])?$} $spec - chan band bw]} {
	set spec ""
	switch $band {
	    "a" {
		set spec "5g"
		set band 5
	    }
	    "b" {
		set spec "2g"
		set band 2
	    }
	    "" {
		if {$chan <= 14} {
		    set band 2
		} else {
		    set band 5
		}
	    }
	}
	append spec $chan
	if {$bw ne ""} {
	    append spec "/40"
	    if {$band eq "2"} {
		# Direction specifier is only neeeded for 2g40
		append spec $bw
	    }
	}
    }
    # Discard band if implied band is correct
    if {[regexp {^(\d)g(\d+)(.*)} $spec - band chan rest] &&
	$band == (($chan <= 14) ? 2 : 5)} {
	set spec "$chan$rest"
    }
    return $spec
}

UTF::doc {
    # [call [cmd UTF::chanspecx] [arg chanspec]]

    # Returns the hex version of the specified chanspec.  Implemented
    # as a wrapper around the "chspec" shell command.
}

proc UTF::chanspecx {chanspec} {
    localhost chspec -a $chanspec -t
}

proc UTF::SiteConfig {} {
    set mount [exec mount -t nfs]
    if {![regexp {\.([^\.]+)\.broadcom\.net} $mount - site] &&
	![regexp {^([^\.]+)inas\d+:/ifs} $mount - site] &&
	![regexp {^fs-([^-]+)-\d+:/vol} $mount - site]} {
	UTF::Message WARN "" "Unable to determine controller site.\n$mount"
	set site unknown
    }
    set ::UTF::WebServer "www.sj.broadcom.com"
    set ::UTF::WLANSVN "svn.sj.broadcom.com/svn/wlansvn"
    switch $site {
	nvl -
	lvn -
	sj1 {}
	blr -
	inb {
	    # Bangalore
	    set ::UTF::WebServer "www.blr.broadcom.com"
	    set ::UTF::WLANSVN "svn.blr.broadcom.com/svn-sj1/wlansvn"
	}
	syd {
	    # Sydney
	    set ::UTF::WebServer "www.syd.broadcom.com"
	    set ::UTF::WLANSVN "svn.syd.broadcom.com/svn-sj1/wlansvn"
	}
	bun {
	    # Bunnik
	    set ::UTF::WebServer "www.bun.broadcom.com"
	}
	hsoa {
	    # HsinChu
	    set ::UTF::WebServer "www.hc.broadcom.com"
	    set ::UTF::WLANSVN "svn.hc.broadcom.com/svn-sj1/wlansvn"
	}
	il {
	    # Israel
	    set ::UTF::WebServer "www.il.broadcom.com"
	    set ::UTF::WLANSVN "svn.il.broadcom.com/svn-sj1/wlansvn"
	}
	seo {
	    # Seoul
	    set ::UTF::WLANSVN "svn.seo.broadcom.com/svn-sj1/wlansvn"
	}
	default {
	    UTF::Message WARN "" "Unknown site.  Using sj defaults"
	}
    }
    return $site
}

proc UTF::AddDomain {lan_ip} {
    if {![info exists ::UTF::DNSDomain]} {
	return $lan_ip
    }
    if {![regexp {^(\w+)(:.*)?$} $lan_ip - host rest] ||
	[string match *.* $host] ||
	$host eq "localhost"} {
	return $lan_ip
    } else {
	return "$host.$::UTF::DNSDomain$rest"
    }
}

UTF::doc {
    # [call [cmd UTF::wlerror] [arg -code]]

    # Returns the wl error message corresponding to the specified
    # error code.  This is used for Dongles and other memory-limited
    # devices where only numerical error messages are returned.

}

proc UTF::wlerror {n} {
    switch -- $n {
	0 {return "OK"}
	-1 {return "Undefined error"}
	-2 {return "Bad Argument"}
	-3 {return "Bad Option"}
	-4 {return "Not up"}
	-5 {return "Not down"}
	-6 {return "Not AP"}
	-7 {return "Not STA"}
	-8 {return "Bad Key Index"}
	-9 {return "Radio Off"}
	-10 {return "Not band locked"}
	-11 {return "No clock"}
	-12 {return "Bad Rate valueset"}
	-13 {return "Bad Band"}
	-14 {return "Buffer too short"}
	-15 {return "Buffer too long"}
	-16 {return "Busy"}
	-17 {return "Not Associated"}
	-18 {return "Bad SSID len"}
	-19 {return "Out of Range Channel"}
	-20 {return "Bad Channel"}
	-21 {return "Bad Address"}
	-22 {return "Not Enough Resources"}
	-23 {return "Unsupported"}
	-24 {return "Bad length"}
	-25 {return "Not Ready"}
	-26 {return "Not Permitted"}
	-27 {return "No Memory"}
	-28 {return "Associated"}
	-29 {return "Not In Range"}
	-30 {return "Not Found"}
	-31 {return "WME Not Enabled"}
	-32 {return "TSPEC Not Found"}
	-33 {return "ACM Not Supported"}
	-34 {return "Not WME Association"}
	-35 {return "SDIO Bus Error"}
	-36 {return "Dongle Not Accessible"}
	-37 {return "Incorrect version"}
	-38 {return "TX Failure"}
	-39 {return "RX Failure"}
	-40 {return "Device Not Present"}
	-41 {return "Command not finished"}
	-42 {return "Nonresident overlay access"}
	-43 {return "Disabled in this build"}
	default {
	    return "unknown error code $n"
	}
    }
}

UTF::doc {
    # [call [cmd UTF::clmload_statuserr] [arg num]]

    # Translates a clmload_status number into an human-readable error
    # message.

}
proc UTF::clmload_statuserr {n} {
    switch -- $n {
	0 {return "DOWNLOAD_SUCCESS ($n)"}
	1 {return "DOWNLOAD_IN_PROGRESS ($n)"}
	2 {return "IOVAR_ERROR ($n)"}
	3 {return "BLOB_FORMAT ($n)"}
	4 {return "BLOB_HEADER_CRC ($n)"}
	5 {return "BLOB_NOMEM ($n)"}
	6 {return "BLOB_DATA_CRC ($n)"}
	7 {return "CLM_BLOB_FORMAT ($n)"}
	8 {return "CLM_MISMATCH ($n)"}
	9 {return "CLM_DATA_BAD ($n)"}
	default {
	    return "unknown status code $n"
	}
    }
}

UTF::doc {
    # [call [cmd UTF::txphyerr] [arg num]]

    # Translates a tx phy error number into a list of human-readable
    # tokens.

}

proc UTF::txphyerr {c} {
    # Assumes corerev >= 42.  We can't check corerev directly since
    # this will be called by host-level handlers, not device level.
    set txphyerr {
	15 sigbl_error
	14 txctrlNplcp_Incon_mumimo
	13 invalidRate
	12 illegal_frame_type
	11 COMBUnsupport
	10 txInCal
	9  unsupportedmcs
	8  sdbc_error
	7  NDPError
	6  RsvdBitError
	5  BWUnsupport
	4  txctrlNplcp_Incon
	3  bfm_error
	2  lengthmismatch_long
	1  lengthmismatch_short
	0  send_frame_low
    }
    set errlist {}
    foreach {i name} $txphyerr {
	set m [expr {1<<$i}]
	if {$c & $m} {
	    lappend errlist $name
	    set c [expr {$c &~$m}]
	}
    }
    if {$c} {
	# Flag any remaining bits
	lappend errlist [format "?0x%x?" $c]
    }
    join $errlist +
}


proc UTF::destroy {{namespace ::UTF}} {
    foreach t [namespace children $namespace] {
	if {[info commands ${t}::Snit_typeconstructor] ne ""} {
	    # UTF snit type
	    if {[catch {$t destroy} ret]} {
		UTF::Message WARN UTF::destroy "$t: $ret"
	    }
	} else {
	    # Try deeper
	    UTF::destroy $t
	}
    }
}

# For interactive use, create a distinctive prompt
if {$tcl_interactive} {
    proc UTF::prompt {} { puts -nonewline "utf> " }
    set tcl_prompt1 UTF::prompt
}

# Terminology
#
# STA: Station - the basic addressable unit in IEEE 802.11
# BSS: Basic Service Set, "Coverage area", contains one or more STAs
# IBSS: Independent BSS, when STA's communicate directly in "Ad hoc"
# DS: Distribution System, STAs associated with interconnected BSSs
# AP: Access Point, STA providing access to a DS
# Router: AP + Wan interface + NAT
# Host: Contains zero or more STA

# Router:
# Implements Wan + Nat
# Delegates AP


snit::type UTF::STA {

    UTF::doc {

	# [call [cmd UTF::STA] [arg staname]
	#              [option -host] [arg host]
	#	       [option -device] [arg device]
	#              [lb][option -peer] [arg peer][rb]
        #              [arg ...]]

	# Create a new STA object named [arg staname].

	# [list_begin options]

	# [opt_def [option -host] [arg host]]

	# The STA is connected to [arg host], which could be any [cmd
	# UTF::Linux], [cmd UTF::Cygwin], or [cmd UTF::Router] object.

	# [opt_def [option -device] [arg device]]

	# The device name of the STA on [arg host].

	# [opt_def [option -peer] [arg peer]]

	# Specify network peer for this interface.  If this is a
	# direct back-to-back ethernet connection, [arg peer] will be
	# the STA object corresponding to the interface at the other
	# end of the connection (which must belong to another host).
	# If this STA is a Wireless device, or is connected to a
	# network, [arg peer] may be a list of STAs.

	# [opt_def [option -attngrp] [arg Group]]

	# Specify an Attenuator Group for this STA.  This allows for
	# easy configuration of RvR tests in simple network
	# topologies.  More complex topologies may need a lot more
	# work to map attenuators onto datapaths.

	# [opt_def [arg ...]]

	# Unrecognised options will be passed on to the STA's [arg
	# host] object.

	# [list_end]
	# [list_end]

	#  STA objects have methods:
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    component host -public host -inherit yes

    option -host -readonly yes
    option -device
    option -name -readonly yes
    option -peer "disconnected!"
    option -revinfo -readonly yes
    option -ssid
    option -security
    option -wepkey
    option -wepidx
    option -wpakey
    option -dpt_pmk
    option -ipaddr dhcp
    option -attngrp
    option -dualband; # Partner AP in Simultaneous Dualband tests.
    option -upstream; # Partner AP in Router WET, WDS, etc
    option -ibsswith; # Partner STA in Nightly IBSS tests
    option -tdlswith;  # Partner STA in Nightly TDLS tests

    option -dptwith -configuremethod _depconf; # Deprecated - use -tdlswith

    method _depconf {option value} {
	if {$option eq "-dptwith"} {
	    UTF::Message WARN $self "Deprecated option -dptwith.  Please use -tdlswith"
	    $self configure -tdlswith $value
	} else {
	    error "unexpected _depconf option $option"
	}
    }

    # Indicate this STA hosts a DHCP server which may need restarting
    option -hasdhcpd -type boolean -default false


    # Options introduced to allow Radius server configurations for AP
    # Relevant scripts are APConfigureSecurity_CM.test,
    # APRetrieveSecurity_CM.test ConnectAPSTA_CM.test.
    option -auth
    option -auth_mode
    option -wep
    option -akm
    option -crypto
    option -radius
    option -radiusport
    option -radiuskey

    option -_path

    variable db -array {
	chipnum_raw ""
	chipnum ""
	chiprev_raw ""
	chiprev ""
	corerev ""
	board ""
	boardid_raw ""
	boardid ""
	boardrev ""
	boardvendor ""
	driverrev ""
	country ""
	phy ""
	target ""
	preserved ""
    }

    constructor {args} {
	set options(-name) [namespace tail $self]
	set options(-host) [namespace which [from args -host]]
	set host $options(-host)
	$self configurelist $args
    }


    UTF::doc {
	# [call [arg staname] [method wl] [arg {args ...}]]

	# Run [cmd wl] against STA's wireless device.  Note that
	# [option -i] [arg device] will be provided automatically if
	# needed.
    }

    method wl {args} {
	if {$options(-device) ne ""} {
	    if {![string match {-[ia]} [lindex $args 0]] } {
		set args [concat -i $options(-device) $args]
	    }
	}
	$host wl {*}$args
    }

    UTF::doc {
	# [call [arg staname] [method dhd] [arg {args ...}]]

	# Run [cmd dhd] against STA's wireless device.  Note that
	# [option -i] [arg device] will be provided automatically if
	# needed.
    }

    method dhd {args} {
	if {$options(-device) ne ""} {
	    if {[lindex $args 0] ne "-i"} {
		set args [concat -i $options(-device) $args]
	    }
	    $host dhd {*}$args
	}
    }

    UTF::doc {
	# [call [arg staname] [method wl_counter] [arg {key1 ...}]]

	# Executes wl counters, but returns only a list of the values
	# corresponding to the listed keys.

	# [example_begin]
	utf> $STA wl_counter rxbeaconmbss
	268
	# [example_end]
    }

    method wl_counter {args} {
	foreach l [split  [$self wl -silent counters] \n] {
	    # Seperate out the multi-value counters
	    if {[regexp {^(\S+):\s+(.*)} $l - k v]} {
		set c($k) $v
	    } else {
		# Process single-value counters
		foreach {k v} [split $l] {
		    set c($k) $v
		}
	    }
	}
	set ret {}
	foreach a $args {
	    UTF::Message LOG $options(-name) "$a $c($a)"
	    lappend ret $c($a)
	}
	set ret
    }

    UTF::doc {
	# [call [arg staname] [method wl_escanresults] [arg {scan args ...}]]

	# [cmd {wl escanresults}] combines the [cmd {wl scan}] / wait
	# / [cmd {wl scanresults}] sequence into one atomic command.
	# This is useful for test scripts, but is not supported on
	# most platforms, and on the ones it is it often reports
	# beacons instead of probe responses.  The [method
	# wl_escanresults] method simulates the [cmd {wl
	# escanresults}] command using legacy scan.  Arguments are the
	# same as [cmd {wl scan}]

	# [example_begin]
    } {{
	utf> $STA wl_escanresults -c 3
	SSID: "UTFRamsey2b"
	Mode: Managed	RSSI: -20 dBm	SNR: 0 dB	noise: -92 dBm	Flags: RSSI on-channel 	Channel: 3
	BSSID: 00:90:4C:30:12:34	Capability: ESS ShortSlot
	Supported Rates: [ 1(b) 2(b) 5.5 11 18 24 36 54 6 9 12 48 ]
	802.11N Capable:
		Chanspec: 2.4GHz channel 3 20MHz (0x2b03)
		Control channel: 3
		802.11N Capabilities: SGI20 SGI40
		Supported MCS : [ 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 ]
    }} {
	# [example_end]
    }

    method wl_escanresults {args} {
	if {![catch {$self cget -escan} ret] && $ret} {
	    return [$self wl escanresults {*}$args]
	}

	$self wl scan {*}$args

	# Only sleep 1 second, but retry up to 6 seconds.  Return as
	# soon as we have results.
	for {set i 1} {$i < 6} {incr i} {
	    UTF::Sleep 1
	    if {[catch {$self wl scanresults} ret]} {
		if {![regexp {Not Ready} $ret]} {
		    error $ret
		}
	    } elseif {[regexp {bss_info_version} $ret]} {
		error $ret
	    } elseif {$ret ne ""} {
		break
	    }
	}
	return $ret
    }


    method wl_dump_ampdu {} {
	package require UTF::Dump::AMPDU
	if {[info exists ::UTF::DumpAMPDUTXRX]} {
	    # Fetch tx and rx seperately, if possible, to reduce buffer
	    # length issues.
	    catch {$self wl dump ampdu_tx} rettx
	    if {[regexp {Unsupported|Not Found} $rettx]} {
	        catch {$self wl dump ampdu} ret
	    } else {
	        catch {$self wl dump ampdu_rx} retrx
	        set ret "$rettx\n$retrx"
	    }
	} else {
	    catch {$self wl dump ampdu} ret
	}
	UTF::Dump::AMPDU %AUTO% -raw $ret
    }

    method wl_dump_amsdu {} {
	package require UTF::Dump::AMSDU
	catch {$self wl dump amsdu} ret
	UTF::Dump::AMSDU %AUTO% -raw $ret
    }

    method wl_dump_phychanest {} {
	package require UTF::Dump::PhyChanEst
	# use wrapper for capture, since a single wl dump may not be
	# sufficient.
	UTF::Dump::PhyChanEst capture $self
    }

    UTF::doc {
	# [call [arg staname] [method wl_interface_create] [arg mode]
	#	[lb][arg name][rb] [arg ...]]

	# Wrapper around [cmd wl] [method interface_create] but
	# returns a UTF::STA object for the new interface.  Additional
	# arguments are passed directly to [cmd wl] [method
	# interface_create]
    }

    method wl_interface_create {mode {name #1} args} {
	if {[string match {-*} $name]} {
	    # name looks like an option - move it back to args
	    set args [lreplace $args 0 -1 $name]
	    set name #1
	}
	if {[$self hostis Android] && [$self cget -extsup]} {
	    set ifname bcm$name
	    regsub {\#} $ifname {} ifname
	    if [regexp {[$ifname]} \
		    [$self host wpa_cli - "" IFNAME=wlan0 Driver interface_create $ifname] ] {
		UTF::Message LOG $self "created $mode interface $ifname"
	    } else {
		throw FAIL "Unable to create interface"
	    }
	    $self host wpa_cli - "" interface_add $ifname \
		/data/misc/wifi/wpa_supplicant_$mode.conf nl80211
	    UTF::Sleep 10
	} else {
	    if {![regexp {ifname: (\S+).*mac_addr (\S+)} \
		      [$self wl -u interface_create $mode {*}$args] - \
		      ifname mac]} {
		error "ifname not found"
	    }
	    if {$ifname eq "e_create"} {
		throw FAIL "Unable to create interface"
	    }
	    UTF::Sleep 1
	}
	if {[regexp {^#} $name]} {
	    set name "[namespace tail $self]$name"
	}
	UTF::STA ::$name -host $options(-host) -device $ifname

	# Fix up mac address for older FW
	if {$mode eq "awdl" && [info exists mac] &&
	    [set mix [lsearch $args -m]] > 0} {
	    incr mix
	    set m [lindex $args $mix]
	    if {$mac ne $m} {
		$name ifconfig hw ether $m
	    }
	}
	return $name
    }

    method wl_interface_remove {} {
	if {[$self hostis Android] && [$self cget -extsup]} {
	    $self host wpa_cli - "" interface_remove $options(-device)
	    $self host wpa_cli - "" IFNAME=wlan0 Driver interface_delete $options(-device)
	} else {
	    $self wl interface_remove
	}
	$self destroy
    }

    UTF::doc {
	# [call [arg staname] [method revinfo]]

	# Same as [arg staname] [method wl] [option revinfo] but uses
	# a cache to improve performance.  Don't cache if ucoderev is
	# zero, since it probably wasn't up at the time.
    }

    method revinfo {} {
	if {$options(-revinfo) eq "" ||
	    [regexp {ucoderev 0x0} $options(-revinfo)]} {
	    set options(-revinfo) [$self wl -silent revinfo]
	    UTF::Message LOG [$self cget -name] \
		"[subst [regsub {ucoderev (0x[[:xdigit:]]+)} $options(-revinfo) \
		{ucoderev \1 (BOM [$self ucoderev \1])}]]"
	}
	return $options(-revinfo)
    }

    UTF::stamethod ifconfig

    UTF::stamethod ipaddr
    UTF::stamethod pci

    UTF::stamethod {sniffer start}
    UTF::stamethod {sniffer stop}

    UTF::stamethod wlname
    UTF::stamethod brname
    UTF::stamethod devcon

    UTF::stamethod reg

    UTF::stamethod macaddr
    UTF::stamethod if_txrx

    UTF::stamethod {supplicant start}
    UTF::stamethod {supplicant stop}

    method wpa_cli {args} {
	$host wpa_cli -i $options(-device) {*}$args
    }

    # Cache the mac address
    variable macaddr ""
    method macaddr {} {
	if {$macaddr ne ""} {
	    set macaddr
	} else {
	    set macaddr [$host macaddr $options(-device)]
	}
    }

    UTF::doc {
	# [call [arg staname] [method wlconfig] [arg {args ...}]]

	# Run [cmd wlconfig] against STA's IP adaptor.
    }

    method wlconfig {args} {
	$host wlconfig -A $options(-device) {*}$args
    }

    UTF::doc {
	# [call [arg staname] [method epi_ttcp] [arg target]
	#		[lb][option -timeout] [arg seconds][rb]
	#		[lb][option -Timeout] [arg seconds][rb]
	#               [lb][arg options...][rb]]
        #
	# Run an epi_ttcp listener on STA [arg target] and an epi_ttcp
	# sender on [arg staname].  Note that in order to avoid
	# conflicts with the [cmd epi_ttcp] command's own arguments,
	# these options cannot be abreviated.

	# [list_begin options]

	# [opt_def [option -timeout] [arg seconds]]

	# If the [cmd epi_ttcp] listener has not received any data in
	# [arg seconds] the method will attempt to kill the [cmd
	# epi_ttcp] processes and return an error.  The default
	# timeout is 10 seconds.

	# [opt_def [option -Timeout] [arg seconds]]

	# If command has not completed in [arg seconds] the method
	# will attempt to kill the [cmd epi_ttcp] processes and return
	# an error.  The default Timeout is 300 seconds.

	# [opt_def [option -ip] [arg address]]

	# Specify IP address of target.  If not specified, target will
	# be queried for its IP address.

	# Additional [arg options...] are passed onto the [cmd
	# epi_ttcp] sender commandline.

	# [list_end]
    }

    proc kill {name args} {
	UTF::Message LOG $name "epi_ttcp timeout: kill $args"
	exec kill {*}$args
    }

    variable reading
    method epi_ttcp {target args} {
	if {[catch {cmdline::getKnownOptions args {
	    {timeout.arg 300 "Seconds to timeout if no data received"}
	    {Timeout.arg 300 "Seconds to timeout if command not completed"}
	    {r.secret "Ignore"}
	    {t.secret "Ignore"}
	    {ip.arg "" "Override IP address"}
	    {per "Report PER instead of throughput"}
	    {b.arg "0" "socket buffer size"}
	} "epi_ttcp options"} ret]} {
	    error "$options(-name) $ret"
	} else {
	    array set pargs $ret
	}
	# Convert k to bytes
	if {[regsub {k$} $pargs(b) {} pargs(b)]} {
	    set pargs(b) [expr {$pargs(b) * 1024}]
	}

	if {$pargs(ip)!=""} {
	    set tip $pargs(ip)
	} else {
	    set tip [$target ipaddr]
	}
	set tname [$target cget -name]
	# Scale command timout
	set Timeout [expr {$pargs(Timeout)*1000}]
	# Scale IO timeout
	set timeout [expr {$pargs(timeout)*1000}]

	# Don't set socket buffer size on Linux,
	# since it's safer to let tcptune do it.
	if {$pargs(b) && [$self hostis Cygwin WinDHD Router]} {
	    set txargs [lappend $args -b $pargs(b)]
	} else {
	    set txargs $args
	}
	if {$pargs(b) && [$target hostis Cygwin WinDHD Router]} {
	    set rxargs [lappend $args -b $pargs(b)]
	} else {
	    set rxargs $args
	}

	# Start Receiver
	set recv [$target rpopen [$target cget -epi_ttcp] \
		      -rsfm {*}$rxargs]
	# Set up nonblocking reader
	fconfigure $recv -blocking 0
	fileevent $recv readable [list set [myvar reading] $recv]
	# Start command timer
	set Timer [after $Timeout [myproc kill $tname {*}[pid $recv]]]

	while {![eof $recv]} {
	    # Start io timer
	    set timer [after $timeout \
			   [eval myproc kill $tname [pid $recv]]]
	    vwait [myvar reading]
	    set msg [gets $reading]
	    after cancel $timer; # clear io timer
	    if {$msg ne ""} {
		UTF::Message LOG $tname $msg
		if {[regexp {\# (tcp|udp) receiver \#} $msg]} {
		    break
		}
	    }
	}
	if {[eof $recv]} {
	    # Receiver failed
	    # Put fd back to blocking so that close can get a valid return
	    # status
	    fconfigure $recv -blocking 1
	    set rcode [catch {close $recv} rret]
	    after cancel $Timer; # clear command timer
	    UTF::Message LOG $tname $rret
	    $target rexec ps -ef
	    UTF::Assert -re $rret {^[\d.]+ Mbit/sec} $tname "epi_ttcp $args"
	    return -code $rcode $rret
	}

	# Receiver ready, now start Sender
	set snd [$self rpopen [$self cget -epi_ttcp] -tsfm {*}$txargs $tip]

	# Set up nonblocking reader
	fconfigure $snd -blocking 0
	fileevent $snd readable [list set [myvar reading] $snd]

	# Restart command timer
	after cancel $Timer
	set Timer \
	    [after $Timeout \
		 [myproc kill $options(-name) {*}[pid $recv] {*}[pid $snd]]]

	set ret ""
	while {![eof $recv] ||![eof $snd]} {
	    # Start io timer
	    set timer [after $timeout \
			   [myproc kill $options(-name) \
				{*}[pid $recv] {*}[pid $snd]]]
	    vwait [myvar reading]
	    set msg [gets $reading]
	    after cancel $timer; # clear io timer
	    if {$msg ne ""} {
		regsub -all {\#+$} $msg {} msg
		if {$msg ne ""} {
		    append data "$msg"
		    UTF::Message LOG $options(-name) $msg
		    if {$pargs(per)} {
			regexp -line {^ttcp-r: ([\d.]+)% .* loss} $msg - ret
		    } else {
			regexp -line {^ttcp-r: .* = ([\d.]+ Mbit/sec)} $msg - ret
		    }
		}
	    }
	}
	# Put fd back to blocking so that close can get a valid return
	# status
	fconfigure $snd -blocking 1
	set scode [catch {close $snd} sret]
	fconfigure $recv -blocking 1
	set rcode [catch {close $recv} rret]

	# Clear command timer
	after cancel $Timer

	if {$scode > 0} {
	    UTF::Message LOG $options(-name) $sret
	    append ret "Sender error: $sret\n"
	}
	if {$rcode > 0} {
	    UTF::Message LOG [$target cget -name] $sret
	    append ret "Receiver error: $rret\n"
	}
	if {$scode > 0 || $rcode > 0} {
	    error [string trim $ret]
	}
	if {!$pargs(per)} {
	    UTF::Assert -re $ret {^[\d.]+ Mbit/sec} \
		$options(-name) "epi_ttcp $args"
	}
	return $ret
    }

    method iperf_all {args} {
	set ret [$self rexec iperf -fb -t 5 {*}$args]
	set results {}
	foreach p [regexp -inline -all {[\d.]+(?= .?bits/sec)} $ret] {
	    lappend results [expr {$p / 1000000.0}]
	}
	lreplace $results end end
    }

    method iperf {args} {
	if {[catch {cmdline::getKnownOptions args {
	    {per "Report PER instead of throughput"}
	    {t.arg {10} "Test time"}
	} "iperf options"} ret]} {
	    error "$options(-name) $ret"
	} else {
	    array set pargs $ret
	}
	set Timeout [expr {$pargs(t)+10}]
	catch {
	    $self rexec -t $Timeout iperf -fb {*}$args -t $pargs(t)
	} ret

	if {[regexp {.* ([\d.]+) (M)?bits/sec(?: [^\n]+\(([\d.]+)%\))?} \
		       $ret - tput M per]} {
	    if {$pargs(per)} {
		return $per
	    } elseif {$M eq "M"} {
		return "$tput Mbit/sec"
	    } else {
		return "[expr {$tput/1000000.0}] Mbit/sec"
	    }
	} elseif {[regexp {child killed} $ret]} {
	    return "0 Mbit/sec"
	} else {
	    error "iperf $args: $ret"
	}
    }

    UTF::doc {
	# [call [arg host] [method tcpautowindow] [lb][arg rateset][rb]]
	#
	# Guesses an appropriate tcp window size based on the device's
	# current rateset.  Rateset will indicate the number of 11n
	# streams and is more likely to be portable than querying the
	# streams, chains or cores directly.  Older dongles may not
	# report MCS rates in the rateset, but they won't support the
	# larger window sizes anyway.  Host option [option -tcpwindow]
	# will override the auto detection.  If the output of [cmd {wl
	# rateset}] is already known it can be provided as an argument
	# to avoid needing to run [cmd {wl rateset}] again.
    }
    method tcpautowindow {{rateset ""}} {
	if {[set w [$self cget -tcpwindow]] ne "auto"} {
	    return $w
	}
	if {$rateset eq ""} {
	    if {[catch {$self wl rateset} rateset]} {
		UTF::Message WARN $options(-name) \
		    "rateset not supported.  Using default window size"
		return 0
	    }
	}
	if {[regexp {MCS SET : .* 23 } $rateset]} {
	    # 3x3
	    return "1152k"
	} elseif {[regexp {MCS SET : .* 7 } $rateset]} {
	    # 2x2, 1x1
	    return "512k"
	} else {
	    # Legacy or cut-down dongles
	    return 0
	}
    }

    UTF::doc {
	# [call [arg staname] [method allrates] [lb][option -auto][rb]
	# [lb][arg rateset][rb]]

	# Returns all tx rates available to this STA.  If the STA is
	# currently associated, these will be the rates nogotiated for
	# this link.  If the STA is not associated, these will reflect
	# the default rates for the HW.  If the output of [cmd {wl
	# rateset}] is already known it can be provided as an argument
	# to avoid needing to run [cmd {wl rateset}] again.  [option
	# -auto] limits the returned rates to those availableused in
	# auto rate selection.
    }

    method allrates {args} {
	UTF::GetKnownopts {
	    {auto "Only report rates used by auto rate selection"}
	}
	switch [llength $args] {
	    0 {
		set rateset [$self wl rateset]
	    }
	    1 {
		set rateset [lindex $args 0]
	    }
	    default {
		error "Usage: STA allrates ?-auto? ?rateset?"
	    }
	}

	set a "auto"
	set LEGACY ""
	set MCSSET ""
	set VHTSET ""

	# First strip out continuator line breaks
	regsub -all {\n\s+:} $rateset { } rateset

	# Parse out VHT rates
	regexp -line {VHT SET : (.*)} $rateset - VHTSET
	# Parse out MCS rates
	regexp -line {MCS SET : \[(.*)\]} $rateset - MCSSET
	# Parse out legacy rates
	regexp -line {^\[(.*)\]} $rateset - LEGACY
	# Strip out basic rate indicators
	regsub -all {\(b\)} $LEGACY {} LEGACY

	set rates ""


	if {$(auto)} {
	    set a ""
	    if {[llength $VHTSET] > 0} {
		# If we're in VHT, auto doesn't use MCS
		set MCSSET ""
	    } else {
		set MCSSET [lsearch -all -inline -not $MCSSET 32]
	    }
	    if {([llength $VHTSET] > 0 || [llength $MCSSET] > 0) &&
		[$self band] eq "a"} {
		set LEGACY ""
	    }
	}

	 # translate legacy rates to UTF format
	foreach r $LEGACY {
	    lappend rates "($r)"
	}

	# concat rates and sort
	set rates [UTF::Ratesort [concat $rates $MCSSET $VHTSET $a]]

	return $rates
    }

    UTF::doc {
	# [call [arg host] [method add_networks] [arg {AP ...}]]

	# Set up additional routing to allow the STA to access all the
	# networks (LAN, LAN1 & WAN) on all the listed routers [arg
	# {AP ...}].  This used to be done via a general /16 test
	# route, but we need to be more specific for multi-sta
	# testing.  The old /16 net will be removed for the
	# transition.

	# The routes added will depend on the net the STA is connected
	# to.  This can be used by the WAN endpoint as well as LAN and
	# LAN1 STAs.  Normally only one router is used, but in
	# TravelRouter and other multi-router situations more than one
	# router many be involved.  STA should be associated to one of
	# the routers.
    }

    method add_networks {args} {
	package require ip
	set staip ""
	set gateway ""
	set networks ""
	foreach AP $args {
	    # We may have been passed a Router host or an AP Interface
	    # on a Router
	    set type [namespace tail [$AP info type]]
	    if {$type eq "STA"} {
		set type [$AP hostis]
	    }
	    # Only Routers need new networks, not APs.
	    if {($type ne "Router" && $type ne "BSDAP") ||
		[string is true -strict [$AP nvram get router_disable]]} {
		continue
	    }
	    if {$staip eq ""} {
		# fetch IP addr after router check, since we don't
		# need it for APs.
		set staip [$self ipaddr]
	    }
	    set interfaces {lan lan1}
	    if {[$AP cget -wanpeer] ne ""} {
		lappend interfaces wan0
	    }
	    foreach net $interfaces {
		set ipaddr [string trim [$AP nvram get ${net}_ipaddr]]

		if {$ipaddr eq "" || $ipaddr eq "0.0.0.0"} {
		    UTF::Message LOG [$AP cget -name] "$net not in use"
		} else {
		    set netmask [string trim [$AP nvram get ${net}_netmask]]
		    if {$netmask eq ""} {
			UTF::Message LOG [$AP cget -name] "$net empty netmask"
		    } elseif {[ip::equal $staip/$netmask $ipaddr/$netmask]} {
			UTF::Message LOG $options(-name) \
			    "gateway is on $AP's $net"
			# Primary net.  This provides the gateway.
			# The wan endpoint will use _ipaddr, everyone
			# else uses _gateway
			if {$net eq "wan0"} {
			    set gateway $ipaddr
			} else {
			    set gateway [string trim \
					     [$AP nvram get ${net}_gateway]]
			}
		    } else {
			# These are the additional networks
			set prefix [ip::prefix $ipaddr/$netmask]
			lappend networks $prefix $netmask
		    }
		}
	    }
	}

	if {$gateway ne ""} {
	    foreach {net mask} $networks {
		$self route replace $net $mask $gateway
	    }
	}
	if {[$self hostis Linux DHD]} {
	    # Move multicast route to the test net (Only Linux for now)
	    $self -x ip route replace multicast 224.0.0.0/4 dev \
		$options(-device)
	}
	if {[$self cget -add_networks_hook] ne ""} {
	    eval [string map [list %S $self] [$self cget -add_networks_hook]]
	}
	return
    }


    UTF::doc {
	# [call [arg staname] [method hostis] [lb][arg target][rb] ...]

	# With no argument, returns the type of the STA's Host object
	# (without the ::UTF:: prefix).  With arguments, returns true
	# if the STA's Host object is the same as any of the supplied
	# arguments (with or without namespace qualifiers) or false
	# otherwise.
    }

    method hostis {args} {
	set type [$host info type]
	if {$args eq {}} {
	    return [namespace tail $type]
	} else {
	    foreach target $args {
		if {$type eq [namespace which $target] ||
		    [namespace tail $type] eq $target} {
		    return 1
		}
	    }
	}
	return 0
    }


    UTF::doc {
	# [call [arg staname] [method !ping] [arg target]
	#	[lb][arg {args...}][rb]]

	# Ping from [arg staname] to [arg target].  Target may be a
	# STA or a Hostname/IP address.  Returns success if a response
	# packet is [emph not] received, otherwise gives an error.
	# Note this is the opposite of [method ping].

    }
    method !ping {args} {
	set code [catch {$host ping {*}$args} ret]
	return -code [expr {!$code}] $ret
    }

    # Sources:
    # http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/DellInformation
    # http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/HPMobileInformation
    # http://wcgbu.broadcom.com/WLAN/Station/Apple
    variable vendorboards -array {

	0x1028 Dell
	0x1028,0x4320,0x0001 4306MP_Dell
	0x1028,0x4324,0x0001 4309MP_Dell
	0x1028,0x4320,0x0002 4306CBSG_Dell
	0x1028,0x4320,0x0003 4306MPSGC0_Dell
	0x1028,0x4324,0x0003 4309MPC0_Dell
	0x1028,0x4318,0x0004 4309CBC0_Dell
	0x1028,0x4318,0x0005 4318MPG_Dell
	0x1028,0x4319,0x0005 4318MPAGH_Dell
	0x1028,0x4311,0x0007 4311MCG_Dell
	0x1028,0x4312,0x0007 4311MCAG_Dell
	0x1028,0x4311,0x0008 4311MCG_Dell
	0x1028,0x4328,0x0009 4321MC_Dell
	0x1028,0x4328,0x000a 4321MC123_Dell
	0x1028,0x4315,0x000b 4312MCG_Dell
	0x1028,0x4315,0x000c 4312HMG_Dell
	0x1028,0x4727,0x0010 4313HMG2L_Dell
	0x1028,0x432b,0x000d 4322HM8LD_Dell
	0x1028,0x43b1,0x0017 4352HMB_Dell

	0x103c HP
	0x103c,0x4311,0x1363 4311MCG_HP
	0x103c,0x4311,0x1364 4311MCG_HP
	0x103c,0x4311,0x1365 4311MCG_HP
	0x103c,0x4311,0x1374 4311MCG_HP
	0x103c,0x4311,0x1375 4311MCG_HP
	0x103c,0x4311,0x1376 4311MCG_HP
	0x103c,0x4311,0x1377 4311MCG_HP
	0x103c,0x4312,0x135f 4311MCAG_HP
	0x103c,0x4312,0x1360 4311MCAG_HP
	0x103c,0x4312,0x1361 4311MCAG_HP
	0x103c,0x4312,0x1362 4311MCAG_HP
	0x103c,0x4312,0x1370 4311MCAG_HP
	0x103c,0x4312,0x1371 4311MCAG_HP
	0x103c,0x4312,0x1372 4311MCAG_HP
	0x103c,0x4312,0x1373 4311MCAG_HP
	0x103c,0x4315,0x137c 4312MCG_HP
	0x103c,0x4315,0x137d 4312MCG_HP
	0x103c,0x4315,0x1507 4312HMGL_HP
	0x103c,0x4315,0x1507 4312HMG_HP
	0x103c,0x4315,0x1508 4312HMGL_HP
	0x103c,0x4315,0x1508 4312HMG_HP
	0x103c,0x4315,0x365e 4312HMGBP1_HP
	0x103c,0x4318,0x1355 4318g_HP
	0x103c,0x4318,0x1356 4318g_HP
	0x103c,0x4318,0x1357 4318g_HP
	0x103c,0x4319,0x1358 4318ag_HP
	0x103c,0x4319,0x1359 4318ag_HP
	0x103c,0x4319,0x135a 4318ag_HP
	0x103c,0x4320,0x12f4 4306C0_HP
	0x103c,0x4320,0x12f8 4306C0_HP
	0x103c,0x4320,0x12fa 4306C0_HP
	0x103c,0x4320,0x12fb 4306C0_HP
	0x103c,0x4324,0x12f9 4309C0_HP
	0x103c,0x4324,0x12fc 4309C0_HP
	0x103c,0x4324,0x12fd 4309C0_HP
	0x103c,0x4328,0x1366 4321MC_HP
	0x103c,0x4328,0x1367 4321MC_HP
	0x103c,0x4328,0x1368 4321MC_HP
	0x103c,0x4328,0x1369 4321MC_HP
	0x103c,0x432b,0x137f 4322MC_HP
	0x103c,0x432b,0x1380 4322MC_HP
	0x103c,0x432b,0x1509 4322HM8L_HP
	0x103c,0x432b,0x1510 4322HM8L_HP
	0x103c,0x4353,0x1509 43224HMS_HP
	0x103c,0x4353,0x1510 43224HMS_HP
	0x103c,0x4357,0x145e 43225HMP1_HP
	0x103c,0x4359,0x182c 43228HM4LP1_HP
	0x103c,0x4727,0x145c 4313HMG2LP1_HP
	0x103c,0x4727,0x1483 4313HMGBEPAP1_HP
	0x103c,0x4727,0x1795 4313HMGBLP1_HP
	0x14e4,0x4357,0x0570 43225HMB_HP
	0x14e4,0x4359,0x05e2 43228HMBP1_HP

	0x106b Apple
	0x106b,0x4328,0x008b 4321M93
	0x106b,0x432b,0x008d 4322X9
	0x106b,0x432b,0x008e 4322M35e
	0x106b,0x4353,0x0093 43224X16
	0x106b,0x4353,0x00d1 43224X21
	0x106b,0x4353,0x00e9 43224X21b
	0x106b,0x4331,0x00d6 4331X19b
	0x106b,0x4331,0x00e4 4331X28
	0x106b,0x4331,0x00ef 4331X29b
	0x106b,0x4331,0x00f4 4331X33
	0x106b,0x4331,0x00f5 4331X19c
	0x106b,0x4331,0x010e 4331X28b
	0x106b,0x4331,0x010f 4331X29d
	0x106b,0x4331,0x0093 4331X29d
	0x106b,0x4331,0x00ec 4331X12b2G
	0x106b,0x43ae,0x00ed 4335X12b5G
	0x106b,0x43a0,0x0111 4360X51
	0x106b,0x43a0,0x0112 4360X29c
	0x106b,0x43a0,0x0117 4360X52c
	0x106b,0x43a0,0x0135 4360X51a
	0x106b,0x43a0,0x0136 4360X51b
	0x106b,0x43a0,0x0137 4360X52d
	0x106b,0x43a3,0x0131 4350X14
	0x106b,0x43ba,0x0132 43602X238
	0x106b,0x43ba,0x0133 43602X87

	0x14e4,0x4360,0x0551 4330Uno3
	0x14e4,0x43a0,0x061b 4360X29c

    }

    UTF::doc {
	# [call [arg staname] [method chipname] [lb][arg {revinfo data}][rb]]

	# Query device revinfo and translate into common name via
	# vendor tables, sromdefs.tcl, devdefs.tcl and bcmdevs.h

	# revinfo data can me supplied as a string argument, otherwise
	# the device will be queried.
    }

    method chipname {{revinfo ""}} {
	global ::UTF::sromdefs ::UTF::devdefs

	set src $::UTF::projgallery/src
	set url "http://$::UTF::WLANSVN/groups/tcl/trunk/src/tools/47xxtcl"
	if {$revinfo eq ""} {
	    set revinfo [$self revinfo]
	}
	regexp -line {boardvendor:? (0x\w+)} $revinfo - boardvendor
	regexp -line {chipnum (0x(\w+))} $revinfo - chiphex db(chipnum_raw)
	regexp -line {chiprev (0x\w+)} $revinfo - db(chiprev_raw)
	regexp -line {corerev (0x\w+|\d+\.\d+)} $revinfo - corerev
	regexp -line {deviceid:? (0x\w+)} $revinfo - deviceid
	regexp -line {boardid:? (0x\w+)} $revinfo - db(boardid_raw)
	regexp -line {boardrev:? ([\w.]+)} $revinfo - boardrev
	regexp -line {driverrev:? ([\w.]+)} $revinfo - driverrev

	set boardid [format "0x%04x" $db(boardid_raw)]

	if {$chiphex > 0x9999} {
	    # 5-digit chipnums are stored in hex
	    set db(chipnum) [expr {$chiphex}]
	} else {
	    # 4-digit chipnums are stored in decimal
	    set db(chipnum) $db(chipnum_raw)
	}
	set db(boardid) $boardid
	set db(boardrev) $boardrev
	set db(boardvendor) $boardvendor
	set db(driverrev) $driverrev

	# These are stored in hex, but usually read in decimal
	set db(chiprev) [expr {$db(chiprev_raw)}]
	set db(corerev) [expr {$corerev}]

	UTF::Message LOG $options(-name) \
	    "Lookup vendor codes $boardvendor,$deviceid,$boardid"
	if {[info exists vendorboards($boardvendor,$deviceid,$boardid)]} {
	    # Vendor branded card
	    set db(board) $vendorboards($boardvendor,$deviceid,$boardid)
	    UTF::Message LOG $options(-name) "Found: $db(board)"
	    return "$db(board) $boardrev"
	} elseif {[info exists vendorboards($boardvendor)]} {
	    UTF::Message WARN $options(-name) \
		"Unknown $vendorboards($boardvendor) board"
	}

	UTF::Message LOG $options(-name) \
	    "Try boardid $boardid in sromdefs.tcl"
	if {![info exists sromdefs]} {
	    if {[catch {localhost -s svn cat $url/sromdefs.tcl} sromdefs]} {
		$self warn $sromdefs
		UTF::Message LOG $options(-name) "Using cashed sromdefs"
		set f [open /tmp/$::tcl_platform(user)_sromdefs.tcl]
		set sromdefs [read $f]
		close $f
	    } else {
		set n [exec mktemp /tmp/$::tcl_platform(user)_sromdefsXXXXX]
		set f [open $n w]
		puts $f $sromdefs
		close $f
		file rename -force $n /tmp/$::tcl_platform(user)_sromdefs.tcl
	    }
	}
	if {[regexp -line -nocase \
		 "^set def\\((?:(?:bcm)?9)?(.*)_ssid\\)\\s+$boardid$" \
		 $sromdefs line name]} {
	    UTF::Message LOG $options(-name) "Found: $line"
	    set db(board) $name
	    return "$db(board) $boardrev"
	}

	UTF::Message LOG $options(-name) \
	    "Try deviceid $deviceid in devdevs.tcl"
	if {![info exists devdefs]} {
	    if {[catch {localhost -s svn cat $url/devdefs.tcl} devdefs]} {
		$self warn $devdefs
		UTF::Message LOG $options(-name) "Using cashed devdefs"
		set f [open /tmp/$::tcl_platform(user)_devdefs.tcl]
		set devdefs [read $f]
		close $f
	    } else {
		set n [exec mktemp /tmp/$::tcl_platform(user)_devdefsXXXXX]
		set f [open $n w]
		puts $f $devdefs
		close $f
		file rename -force $n /tmp/$::tcl_platform(user)_devdefs.tcl
	    }
	}
	# Notes: use -all to pick the last match in case there are
	# duplicates.
	if {[regexp -line -all \
		 "^set\\s+def\\(device_bcm(\\w+)\\)\\s+$deviceid$" \
		 $devdefs line name]} {
	    UTF::Message LOG $options(-name) "Found: $line"
	    set db(board) $name
	    return "$db(board) $boardrev"
	}

	UTF::Message LOG $options(-name) \
	    "Try deviceid $deviceid in bcmdevs.h"
	set f [open "$src/include/bcmdevs.h" r]
	set inc [read $f]
	close $f
	# Notes: use -all to pick the last match in case there are
	# duplicates.
	if {[regexp -line -all \
		 "^\#define\\s+(?:BCM)?(\\w+)_D11(\\w*)_ID\\s+$deviceid\\s" \
		 $inc line name1 name2]} {
	    UTF::Message LOG $options(-name) "Found: $line"
	    set db(board) "$name1$name2"
	    return "$db(board) $boardrev"
	}
	UTF::Message LOG $options(-name) "fallback to chipnum"
	set db(board) $chipnum
	return "$db(board) $boardrev"
    }


    UTF::doc {
	# [call [arg staname] [method phyname] [lb][arg {revinfo data}][rb]]

	# Query device revinfo and translate phytype into common name.

	# revinfo data can me supplied as a string argument, otherwise
	# the device will be queried once.  Repeated requests will use
	# a cached value.
    }
    method phyname {{revinfo ""}} {
	if {$revinfo eq ""} {
	    set revinfo [$self revinfo]
	}
	if {$revinfo eq "unknown"} {
	    set n "unknown"
	} else {
	    if {![regexp {phytype (0x\w+)} $revinfo - phytype]} {
		error "Unable to determine phytype"
	    }
	    switch $phytype {
		0x0 {set n "A"}
		0x1 {set n "B"}
		0x2 {set n "G"}
		0x4 {set n "N"}
		0x5 {set n "LP"}
		0x6 {set n "SSN"}
		0x7 {set n "HT"}
		0x8 {set n "LCN"}
		0x9 {set n "LCNX"}
		0xa {set n "LCN40"}
		0xb {set n "AC"}
		0xc {set n "LCN20"}
		default {
		    error "Unrecognised phytype $phytype"
		}
	    }
	}
	set db(phy) $n
	UTF::Message INFO $options(-name) "Phyname: ${n}PHY"
	return "${n}PHY"
    }

    UTF::doc {
	# [call [arg staname] [method txphyerr] [arg num]]

	# Translates a tx phy error number into a list of
	# human-readable tokens.  The intention was to replace the
	# static UTF::txphyerr, however this doesn't work since at the
	# time we receive the message we may not know which STA it
	# came from.
    }

    method txphyerr {c} {
	if {![regexp {corerev (0x[:xdigit:]+)} [$self revinfo] - corerev]} {
	    set corerev 0; #unknown
	}
	if {$corerev >= 42} {
	    # 4360b0 - 4366c0
	    set txphyerr {
		15 sigbl_error
		14 txctrlNplcp_Incon_mumimo
		13 invalidRate
		12 illegal_frame_type
		11 COMBUnsupport
		10 txInCal
		9  unsupportedmcs
		8  sdbc_error
		7  NDPError
		6  RsvdBitError
		5  BWUnsupport
		4  txctrlNplcp_Incon
		3  bfm_error
		2  lengthmismatch_long
		1  lengthmismatch_short
		0  send_frame_low
	    }
	} else {
	    # Ref: 4360a0
	    # (4321, 4312 are strict subsets)
	    set txphyerr {
		11 NDPError
		10 RsvdBitError
		9  illegal_frame_type
		8  COMBUnsupport
		7  BWUnsupport
		6  txInCal
		5  send_frame_low
		4  lengthmismatch_short
		3  lengthmismatch_long
		2  invalidRate
	    }
	}
	set errlist {}
	foreach {i name} $txphyerr {
	    set m [expr {1<<$i}]
	    if {$c & $m} {
		lappend errlist $name
		set c [expr {$c &~$m}]
	    }
	}
	if {$c} {
	    # Flag any remaining bits
	    lappend errlist [format "?0x%x?" $c]
	}
	return [join $errlist +]
    }



    UTF::doc {
	# [call [arg staname] [method allchanspecs]
	#	  [lb][option -band] [arg band][rb]]

	# Returns all chanspecs available to this STA
    }

    method allchanspecs {args} {
	UTF::Getopts {
	    {band.arg auto "band"}
	}
	set oldband [$self wl band]
	if {$oldband ne $(band)} {
	    $self wl band $(band)
	}
	if {[info exists UTF::ChanspecsPerBandBW]} {
	    foreach {band bw} {2 20 2 40 5 20 5 40 5 80 5 160 5 80+80} {
		if {![catch {$self wl chanspecs -b $band -w $bw} ret]} {
		    append chanspecs "$ret\n"
		}
		set list {}
		foreach {chanspec -} $chanspecs {
		    lappend list $chanspec
		}
	    }
	} elseif {[catch {$self wl chanspecs} chanspecs]} {
	    set list [$self wl channels]
	} else {
	    set list {}
	    foreach {chanspec -} $chanspecs {
		lappend list $chanspec
	    }
	}
	if {$oldband ne $(band)} {
            $self wl band $oldband
        }
	return $list
    }

    UTF::doc {
	# [call [arg staname] [method band] [lb][arg chanspec][rb]]

	# Returns the band of the given chanspec.  If chanspec is not
	# supplied, then the device will be queried.  This is used
	# because "wl band" may return "auto".
    }

    method band {{chanspec ""}} {
	if {$chanspec eq ""} {
	    set chanspec [$self wl chanspec]
	}
	if {![regexp {^(\d+)} $chanspec - chanspec]} {
	    error "bad chanspec $chanspec"
	}
	if {$chanspec < 15} {
	    return "b"
	} else {
	    return "a"
	}
    }

    UTF::doc {
	# [call [arg staname] [method bw] [lb][arg chanspec][rb]]

	# Returns the bandwidth of the given chanspec.  If chanspec is
	# not supplied, then the device will be queried.
    }

    method bw {{chanspec ""}} {
	if {$chanspec eq ""} {
	    set chanspec [lindex [$self wl chanspec] 0]
	}
	if {[regexp {^\d+/(\d+(?:[\.p]\d+)?)[ul]?$} $chanspec - bw]} {
	    return $bw
	} elseif {[regexp {^\d+[ul]$} $chanspec]} {
	    return 40
	} elseif {[regexp {^\d+$} $chanspec]} {
	    return 20
	} else {
	    error "malformed chanspec: $chanspec"
	}
    }

    UTF::doc {
	# [call [arg staname] [method frequency] [lb][arg chanspec][rb]]

	# Returns the frequency of the given chanspec.  If chanspec is
	# not supplied, then the device will be queried.  (Relies on
	# the [cmd chspec] linux command)
    }

    method frequency {{chanspec ""}} {
	if {$chanspec eq ""} {
	    set chanspec [lindex [$self wl chanspec] 0]
	}
	localhost chspec -a $chanspec -f
    }

    UTF::doc {
       # [call [arg staname] [method txrate] [arg band] [arg rate]]

       # Set the device txrate to the given rate.  [arg band] is
       # required since different bands may have different rates.
       # [arg rate] may be specified as:

       # [list_begin options]
       # [opt_def [option (m)]] .11a/bg rate m mbps
       # [opt_def [option m]]   .11n HT MCS rate m
       # [opt_def [option mxs]] .11ac VHT MCS m NSS S
       # [list_end]

       # a suffix of "s" indicates SGI should be enabled.  LDPC will
       # be enabled by default for VHT rates.
    }

    method txrate {band rate} {
	set sgi [regsub {s$} $rate {} rate]
	lassign [UTF::Rateopt $rate] ropt rate
	set cmd wl
	set opts ""
	if {$ropt eq "-r"} {
	    if {$band eq "a"} {
		lappend cmd a_rate
	    } else {
		lappend cmd bg_rate
	    }
	} elseif {[$self phyname] eq "ACPHY"} {
	    if {$band eq "a"} {
		lappend cmd 5g_rate
	    } else {
		lappend cmd 2g_rate
	    }
	    if {$ropt eq "-m"} {
		lappend cmd -h
	    } else {
		# LDPC always on for VHT
		lappend cmd -l -v
	    }
	    if {$sgi} {
		lappend opts -g
	    }
	} elseif {$rate == 32} {
	    lappend cmd nrate -m
	} else {
	    lappend cmd nrate -w -m
	}
	$self {*}$cmd $rate {*}$opts
    }

    method glitches {} {
	if {[regexp {rxcrsglitch (\d+)} \
		 [$self wl -silent counters] - before]} {
	    UTF::Sleep 1
	    if {[regexp {rxcrsglitch (\d+)} \
		     [$self wl -silent counters] - after]} {
		set diff [expr {$after - $before}]
		UTF::Message LOG $options(-name) "$diff rxcrsglitch/sec"
		return $diff
	    }
	}
	error "Unable to read rxcrsglitch count"
    }


    UTF::doc {
	# [call [arg staname] [method idletime] [arg AP]]

	# Queries an AP to find how many second the link between the
	# STA and the AP has been idle.
    }

    method idletime {AP} {
	set ret [$AP wl -silent sta_info [$self macaddr]]
	if {![regexp -line {idle (\d+) .*} $ret line idle]} {
	    error "$ret\nidle time not found"
	}
	UTF::Message LOG $options(-name) $line
	set idle
    }

    UTF::doc {
	# [call [arg staname] [method idlewait] [arg AP]
	#	  [lb][option -i] [arg {idletime}][rb]
	#	  [lb][option -t] [arg timeout][rb]
	#	  [lb][option -warn][rb]]

	# Waits until the link between the STA and the AP is idle.
	# Option [option -i] specifies the required idle time.  Option
	# [option -t] specifies a maximum time to wait.  By default if
	# the timer expires a FAIL exception is thrown.  Use [option
	# -warn] to downgrade this to a warning.
    }

    method idlewait {AP args} {
	UTF::Getopts {
	    {i.arg 4 "Idle time"}
	    {t.arg 60 "Max time to wait"}
	    {warn "Warn rather than fail if wait time expires"}
	}
	set wait [expr {$(i) + 1}]
	for {set id 0} {[$self idletime $AP] < $(i) && $id < $(t)} {incr id $wait} {
	    UTF::Sleep $wait LOG "Waiting for idle link ($id)"
	}
	if {$id >= $(t)} {
	    if {$(warn)} {
		$self warn "Link busy"
	    } else {
		throw FAIL "Link busy"
	    }
	}
    }


    UTF::doc {
	# [call [arg staname] [method innetworktime] [arg AP]]

	# Queries an AP to find how many seconds the link between the
	# STA and the AP has been up.
    }

    method innetworktime {AP} {
	set ret [$AP wl -silent sta_info [$self macaddr]]
	if {![regexp -line {in network (\d+) .*} $ret line innet]} {
	    error "$ret\nin network time not found"
	}
	UTF::Message LOG $options(-name) $line
	set innet
    }

    UTF::doc {
	# [call [arg staname] [method innetworkwait] [arg AP]
	#	  [lb][arg {time}][rb]]

	# Waits until the link between the STA and the AP has been up
	# for the given number of seconds (default 20).
    }

    method innetworkwait {AP {time 20}} {
	UTF::Sleep [expr {$time - [$self innetworktime $AP]}] \
	    $options(-name) "Wait for in network $time"
    }


    UTF::doc {
	# [call [arg staname] [method whatami] [lb][arg -notype][rb]
	#	  [lb][arg -nospace][rb]]

	# Returns a description of the device under test.  User [arg
	# -notype] to suppress reporting of the UTF Object type and
	# [arg -nospace] to replace whitespace with underscores for
	# easier parsing in report headers.
    }

    variable whatami {}
    method whatami {args} {
	UTF::Getopts {
	    {notype "Don't report object type"}
	    {nospace "Replace whitespace for easier parsing"}
	}
	if {$whatami eq ""} {
	    set whatami [$host whatami $self]
	    # Add Locale
	    if {![catch {$self wl country} ret]} {
		set db(country) [lindex $ret 1]
		append whatami $db(country)
		regsub {\((.*)\)} $db(country) {\1} db(country)
	    }
	    $self phyname
	}
	set w $whatami
	if {$(notype)} {
	    set w [lreplace $w 0 0]
	}
	if {$(nospace)} {
	    regsub -all {\s+} $w {_} w
	}
	if {$db(country) eq "#n/0"} {
	    $self worry "$w - Undefined country"
	}
	return $w
    }

    method targetname {path} {
	if {[regexp {([^/]+/[^/]+)(:?/rtecdc)?\.bin} $path - db(target)] ||
	    [regexp {firmware/.*/([^/]+/[^/]+)\.trx$} $path - db(target)] ||
	    [regexp {obj-([^/]+)/wl.ko$} $path - db(target)]} {
	    UTF::Message INFO $options(-name) "target: $db(target)"
	}
    }

    method preserved {path} {
	set db(preserved) [regexp {PRESERVED|ARCHIVED} $path]
    }

    method dbrecord {role} {
	if {[info exists UTF::dBid]} {
	    UTF::dBRecord \
		role $role \
		os [$self hostis] \
		chipnum $db(chipnum_raw) \
		chiprev $db(chiprev_raw) \
		boardid $db(boardid_raw) \
		boardrev $db(boardrev) \
		boardvendor $db(boardvendor) \
		driverrev $db(driverrev) \
		phy $db(phy) \
		countryCode $db(country) \
		branchName [UTF::BranchName $options(-_path)] \
		path $options(-_path) \
		target $db(target) \
		preserved $db(preserved)
	}

	if {![info exists UTF::DBid]} {
	    UTF::Message LOG $options(-name) "Test not registered in database"
	    return
	}

	UTF::DBRecord role $role name $options(-name) \
	    host [$self hostis] \
	    chipnum $db(chipnum) \
	    chiprev $db(chiprev) \
	    corerev $db(corerev) \
	    board $db(board) \
	    boardrev $db(boardrev) \
	    phy $db(phy) \
	    country $db(country) \
	    branch [UTF::BranchName $options(-_path)] \
	    path $options(-_path) \
	    target $db(target) \
	    preserved $db(preserved) \
	    id $UTF::DBid
    }

    method dbget {} {
	$self chipname
	$self phyname
	return [array get db]
    }

    method ucoderev {{ucoderev ""}} {
	if {$ucoderev eq ""} {
	    if {![regexp {ucoderev (0x[[:xdigit:]]+)} \
		      [$self wl revinfo] - ucoderev]} {
		error "ucoderev not found"
	    }
	}
	return "[expr {$ucoderev>>16}].[expr {$ucoderev & 0xffff}]"
    }

    UTF::doc {
	# [call [arg staname] [method lan] [arg {cmd args ...}]]

	# Execute [arg {cmd args ...}] on the host object named in
        # [arg staname]'s host's [option -lanpeer] option.  If [option
        # -lanpeer] is empty the [arg staname] itself is used.
    }
    method lan {args} {
	if {[set p [lindex [$self cget -lanpeer] 0]] eq ""} {
	    set p $self
	}
	if {$args eq {}} {
	    return $p
	} else {
	    $p {*}$args
	}
    }

    UTF::doc {
	# [call [arg staname] [method attngrp] [arg {cmd args ...}]]

	# Execute [arg {cmd args ...}] on the attngrp object named in
        # [arg staname]'s [option -attngrp] option.  If no [arg args]
        # are provided, returns the value of [option -attngrp].  If
        # [arg args] are provided but [option -attngrp] is empty,
        # [method attngrp] throws an error.
    }
    method attngrp {args} {
	if {$args eq {}} {
	    return $options(-attngrp)
	} elseif {$options(-attngrp) ne ""} {
	    $options(-attngrp) {*}$args
	} else {
	    error "-attngrp not set"
	}
    }

    UTF::doc {
	# [call [arg staname] [method clone] [arg name] [arg {args ...}]]

	# Creates a new STA object with the name [arg name] and a new
	# host object to support it.  The new host object will be of
	# the same type as [arg staname]'s host and will be
	# initialized with the same options, with the exception of
	# [arg -sta] which will reflect the new [arg name].  The
	# remaining arguments will be passed on to the new host.

	# For example, to use the same configuration file to test the
	# same device with software from different branches clone the
	# object and change the -tag option:

	# [example_begin]
    } {{
	# 4322tot is used to test TOT
	# 4322tob is used to test TOB

	UTF::Linux UTF2TestL -sta {4322tot eth1} -tag "NIGHTYL"

	4322tot clone 4322tob -tag "PBR_BRANCH_5_10"
    }} {
	# [example_end]
    }

    method clone {STA args} {
	# fetch original host options
	if {[set clone_path [$host info vars cloneopts]] eq ""} {
	    error "[$host info type] is not cloneable"
	}
	eval set cloneopts $$clone_path

	set oldsta [from cloneopts -sta]
	if {[lsearch $args "-sta"] < 0} {
	    # Replace -sta with new name.  Try to replace current
	    # device and matching MBSS expansions
	    regexp {::([^.]+)(?:\.\d+)?} $self - base
	    regexp {(\w+)(?:\.\d+)?} $STA - stabase
	    foreach {o dev} $oldsta {
		if {[regexp "$base\\.%(.*)\$" $o - s]} {
		    lappend newsta "$STA.%$s" $dev
		    # MBSS
		} elseif {$o eq "_$base"} {
		    # RSDB main
		    lappend newsta "_$stabase" $dev
		} elseif {"::$o" eq $self} {
		    lappend newsta $STA $dev
		} else {
		    # no match - skip
		}
	    }
	    if {$newsta ne ""} {
		# Nothing matched - fall back to old behavior
		lappend cloneopts -sta $newsta
	    }
	}

	from cloneopts -name
	set name [$host cget -name]
	# Make sure -name is set, otherwise a random name will show up
	# in the logs
	lappend cloneopts -name $name

	# Fill in -lan_ip, since empty default won't work for renamed
	# STA-type objects.  Router-type objects are ok.
	if {![$self hostis Router DSL STBAP Vx BSDAP DK8 Panda Capri]} {
	    lappend cloneopts -lan_ip [from cloneopts -lan_ip $name]
	}

	# Remove from OnAll processing, since the parent will already
	# be there
	from cloneopts -onall
	lappend cloneopts -onall false

	# Create new object
	set new [[$host info type] create %AUTO% {*}$cloneopts {*}$args]

	# Try to transfer some per-STA options
	if {[catch {
	    foreach p {-ipaddr -attngrp -hasdhcpd} {
		$STA configure $p [$self cget $p]
	    }
	} ret]} {
	    UTF::Message WARN $STA "clone: $ret"
	}

	return $new
    }

    UTF::doc {
	# [call [arg staname] [method branchname]]

	# Returns the name of the branch being tested.  This is the
	# first component of the -tag option, or TOT.  This is a
	# convenience for reporting, performance caches, etc.
    }
    method branchname {} {
	set tag [$host cget -tag]
	if {[regexp {^DHD_REL_1_\d+$} $tag]} {
	    set branch "TOT"
	} else {
	    regsub {_.*} [$host cget -tag] {} branch
	    if {$branch eq "NIGHTLY" || $branch eq "trunk" ||
		$branch eq "TRUNK"} {
		set branch "TOT"
	    }
	    return $branch
	}
    }

    UTF::doc {
	# [call [arg staname] [method *] [arg {args ...}]]

	# All other methods are passed on to the STA's host
    }

}

# skip rest if running under pkg_mkIndex
if {[info command __package_orig] != ""} {
    return
}

# Can't import while running pkg_mkIndex
if {$tcl_version < 8.6} {
    namespace import trycatch::*
}

UTF::SiteConfig

UTF::LoadUTFConf
if {[info exists ::env(UTFD)] || [info exists ::env(UTFDPORT)]} {
    package require UTFD
}

if {[info exists ::UTF::BuildFileServer]} {
    # override local build search tools
    namespace eval UTF::BuildFile {
	proc glob {args} {
	    set pattern [lindex $args end]
	    $UTF::BuildFileServer \
		rexec -n -s -q "shopt -s nullglob; echo $pattern"
	}
	proc size {file} {
	    lindex [$UTF::BuildFileServer \
			rexec -n -s -q du -b [file join [pwd] $file]] 0
	}
	proc exists {file} {
	    expr {![catch {$UTF::BuildFileServer \
			       rexec -n -s -q test -e \
			       [file join [pwd] $file]}]}
	}
	proc ls {file} {
	    $UTF::BuildFileServer \
		rexec -n -s ls -l [file join [pwd] $file]
	}
	proc sum {file} {
	    $UTF::BuildFileServer \
		rexec -n -s sum [file join [pwd] $file]
	}
	proc isdirectory {file} {
	    expr {![catch {$UTF::BuildFileServer \
			       rexec -n -s test -d \
			       [file join [pwd] $file]}]}
	}
	proc stat {file RET} {
	    upvar $RET ret
	    array set ret [$UTF::BuildFileServer \
			       rexec -n -s -q \
			       "stat -c 'atime %X blksize %o blocks %b ctime %Z dev %d gid %g ino %i mode %a mtime %Y nlink %h size %s type {%F} uid %u' [file join [pwd] $file]"]
	    # Note (type) uses a different format from TCL stat
	}
	proc copyto {dut file args} {
	    set c [exec mktemp /tmp/utfremoteXXXXX]
	    try {
		if {[file extension $file] eq ".gz"} {
		$UTF::BuildFileServer copyfrom $file $c.gz
		$dut copyto $c.gz {*}$args.gz
		$dut gunzip -f {*}$args.gz
		} else {
		$UTF::BuildFileServer copyfrom $file $c
		$dut copyto $c {*}$args
		}
	    } finally {
		#UTF::Message DBG "" "file delete $c"
		file delete $c
		file delete $c.gz
	    }
	}
	proc nvram_add_copyto {dut src dst replace} {
	    if {[set replace [UTF::decomment $replace]] eq ""} {
		copyto $dut $src $dst
	    } else {
		set c [exec mktemp /tmp/utfremoteXXXXX]
		try {
		    $UTF::BuildFileServer copyfrom $src $c
		    set nvram [UTF::nvram_add $c $replace]
		    try {
			$dut copyto $nvram $dst
		    } finally {
			#UTF::Message DBG "" "file delete $nvram"
			file delete $nvram
		    }
		} finally {
		    #UTF::Message DBG "" "file delete $c"
		    file delete $c
		}
	    }
	}
	proc strings {file} {
	    $UTF::BuildFileServer \
		rexec -n -s "strings $file"
	}
	proc modinfo {args} {
	    $UTF::BuildFileServer \
		rexec -n -s -t 120 "/sbin/modinfo $args"
	}
	proc gdb {cmd script} {
	    # GDB can read from a file or a terminal, but not from a
	    # pipe, so we need to copy the gdb script to a temporary
	    # file on the repo server.
	    set c [$UTF::BuildFileServer rexec -n mktemp /tmp/utfremoteXXXXX]
	    try {
		$UTF::BuildFileServer copyto $script $c
		$UTF::BuildFileServer rexec -n -s -t 180 "$cmd <$c"
	    } finally {
		$UTF::BuildFileServer rexec -n rm $c
	    }
	}
    }
}

# Create special localhost object, used for running commands on the
# control host with all the logging and protections normally available
# only on DUTs and relays.
UTF::Base localhost -lan_ip localhost

# Initialize gnuplot
set UTF::Gnuplot [auto_execok gnuplot]
if {$UTF::Gnuplot eq ""} {
    set UTF::Gnuplot "/usr/bin/gnuplot"
    UTF::Message WARN "" "gnuplot not found on path - using $UTF::Gnuplot"
}

# Global session info
set UTF::_start_time [clock seconds]
if {$UTF::args(sessionid) ne ""} {
    set UTF::SessionID $UTF::args(sessionid)
} else {
    set UTF::SessionID \
	[clock format $UTF::_start_time -format {%Y%m%d%H%M%S}]
}

# skip rest if being sourced
if {!$tcl_interactive &&
    (![info exists argv0] || ![string match [info script] $argv0])} {
    return
}

if {[llength $argv]} {
    UTF::Logfile ""
    if {[llength $argv] == 1} {
	set cmd [lreplace $argv -1 -1 eval]
    } else {
	set cmd $argv
    }
    if {$UTF::args(web)} {
	if {[set title $UTF::args(webtitle)] eq ""} {
	    set title $argv
	}
	UTF::WrapSummary $UTF::SummaryDir $title "" $UTF::args(webemail) {
	    UTF::Try "+" {
		{*}$cmd
	    }
	}
    } else {
	set code [catch {{*}$cmd} ret]
	UTF::Message LOG "" ""
	if {$code} {
	    puts $::errorInfo
	    exit $code
	} else {
	    puts $ret
	}
    }

    if {[info exists ::UTF::panic]} {
	error $::UTF::panic $::UTF::panic
    }
    exit
} elseif {$tcl_interactive && !$UTF::args(man)} {
    package require TclReadLine
    catch {package require TclReadLine::UTF}
    if {[info exists ::env(UTFDPORT)]} {
	::UTFD::listen
    }
    TclReadLine::interact
    exit
}

# Only documentation from here on

# Retrieve manpage from last object
UTF::doc [UTF::STA man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]

    # Personal $HOME/.utfconf.tcl configuration file:
    # [example_begin]
} {{
# Personal UTF configuration
#
UTF::Logfile test.log

# Linux box
UTF::Linux TestA \\
    -lan_ip 10.19.12.143 \\
    -sta {TestA1 eth1} \\
    -pcidriver {-type obj-debug-native-stadef-2.4.20-8 wl.o}

# Windows box with Cygwin
UTF::Cygwin Cygtop \\
    -lan_ip 10.19.12.198 \\
    -sta {Cygtop1 1} \\
    -pcidriver {-external -type Dell Dell_InstallShield}
}} {
    # [example_end]

    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also wl]
    # [see_also [uri APdoc.cgi?UTF::Base.tcl UTF::Base]]
    # [see_also [uri APdoc.cgi?UTF::Cygwin.tcl UTF::Cygwin]]
    # [see_also [uri APdoc.cgi?UTF::Linux.tcl UTF::Linux]]
    # [see_also [uri APdoc.cgi?UTF::Router.tcl UTF::Router]]
    # [manpage_end]
}

# Output manpage
UTF::man
