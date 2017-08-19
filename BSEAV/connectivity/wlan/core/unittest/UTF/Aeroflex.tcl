#!/bin/env utf

# UTF Framework Object Definition for the aeroflex variable attenuator
#
# $Id: e84e77554aeb0bb54ef2394a4ae1252121d80259 $
# $Copyright Broadcom Corporation$
#

package provide UTF::Aeroflex 2.0

package require snit
package require UTF
package require UTF::AttnGroup
package require UTF::doc

UTF::doc {
    # [manpage_begin UTF::Aeroflex n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Aeroflex}]
    # [copyright {2009 Broadcom Corporation}]
    # [require UTF::Aeroflex]
    # [description]
    # [para]

    # [cmd UTF::Aeroflex] is a object that provides an HW-specific
    # backend for [cmd UTF::AttnGroup] to control the Aeroflex
    # variable attenuator

    # This object is usually defined in your utfconf file, but it will
    # create one or more [cmd AttnGroup] objects.  The [cmd AttnGroup]
    # objects are used directly in test scripts, not the Aeroflex
    # object.

    # [list_begin definitions]
}

snit::type UTF::Aeroflex {

    typeconstructor {
	snit::double attnval -min 0 -max 121

	snit::stringtype chanval -regexp {\d+([+-]\d+)?}
	snit::listtype chanvallist -type ${type}::chanval
    }

    UTF::doc {
        # [call [cmd UTF::Aeroflex] [arg name]
	#	[option -lan_ip] [arg address]
	#	[option -relay] [arg string]
	#	[option -group]
	#	  [arg {{Group1 {Channels ...}} {Group2 {Channels ...}}}]]

        # Create a new Aeroflex object. It will be used to control
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

	# Defines group objects and their channel lists.  Channels
	# specified as chan+/-offset will have their attn values
	# adjusted by the given offset.

        # [list_end]

        # [para]
        # Aeroflex objects have the following methods:
        # [list_begin definitions]
    }

    pragma -canreplace yes
    option -lan_ip
    option -name
    option -relay localhost
    option -numchannels 4; # ignored
    option -group -configuremethod GroupOption
    option -power
    option -usevashell -type snit::boolean -default false
    option -usechan -type snit::boolean -default false
    option -retries -type snit::integer -default -1
    option -timeout -type snit::integer -default -1

    # Flag to be set for infrastructure objects, not directly involved
    # in testing.  This is to make sure we don't reconfigure or reboot
    # them by accident.
    option -infrastructure -type snit::boolean -default false

    variable grouplist ""

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
	if {[regexp {/udp} $options(-lan_ip)]} {
	    if {$options(-retries) < 0} {
		set options(-retries) 10
	    }
	    if {$options(-timeout) < 0} {
		set options(-timeout) 100
	    }
	} else {
	    if {$options(-retries) < 0} {
		set options(-retries) 1
	    }
	    if {$options(-timeout) < 0} {
		set options(-timeout) 1000
	    }
	}
    }

    UTF::doc {
        # [call [arg name] [method createGroup] [arg groupName]
	#	  [arg channels]]

        # Great a group of channels.  [arg groupName] will be created
        # as a UTF::AttnGroup object which can be used directly in
        # tests.
    }

    method createGroup {groupName channelsList} {
	chanvallist validate $channelsList
	UTF::AttnGroup ::$groupName -aflex $self -channels $channelsList
	lappend grouplist $groupName
    }

    UTF::doc {
        # [call [arg name] [method setChanAttn] [arg chanlList]
	#	  [arg attnVal]]

        # Sets the attenuation value of one of more channels. [para]

    }

    method setChanAttn {chanList attnVal} {
	chanvallist validate $chanList
	attnval validate $attnVal
	if {[llength $chanList] > 8} {
	    set AT ""
	} else {
	    set AT "AT"
	}
	foreach chan $chanList {
	    if {[regexp {(\d+)([+-]\d+)} $chan - chan o]} {
		set v [expr {$attnVal + $o}]
	    } else {
		set v $attnVal
	    }
	    if {$options(-usechan)} {
		lappend cmd "chan $chan; attn $v"
	    } else {
		lappend cmd "attn ${AT}$chan $v"
	    }
	}
	$self vashell [join $cmd ";"]
    }

    UTF::doc {
        # [call [arg name] [method incr] [arg chanlList] [arg size]]

        # Increments the attenuation value by [arg size] which may be
        # positive or negative.  Stepping past the end of the
        # attenuator range will generate a CME error.[para]

	# Note: a temporary group called %% will be created to ensure
	# the channels are all affected simultaneously.
    }

    method incr {chanList size} {
	chanvallist validate $chanList
	snit::integer validate $size
	if {$size == 0} {
	    return
	} elseif {$size < 0} {
	    set size [expr {abs($size)}]
	    set incrcmd "decr"
	} else {
	    set incrcmd "incr"
	}
	foreach chan $chanList {
	    regsub {[+-]\d+} $chan {} chan
	    if {$options(-usechan)} {
		lappend cmd "chan $chan;stepsize $size; $incrcmd"
	    } else {
		lappend cmd "stepsize AT$chan $size; $incrcmd AT$chan"
	    }
	}
	$self vashell [join $cmd ";"]
    }


    UTF::doc {
        # [call [arg name] [method getChanAttn] [arg chanlList]]

        # Return attenuation values of the numbered channels.
    }

    method getChanAttn {chanList} {
	chanvallist validate $chanList
	if {[llength $chanList] > 8} {
	    set AT ""
	} else {
	    set AT "AT"
	}
	foreach chan $chanList {
	    regsub {[+-]\d+} $chan {} chan
	    if {$options(-usechan)} {
		lappend cmd "chan $chan; attn?"
	    } else {
		lappend cmd "attn? $AT$chan"
	    }
	}
	split [$self vashell [join $cmd ";"]] ";"
    }

    UTF::doc {
        # [call [arg name] [method setGrpAttn] [arg group] [arg attnVal]]

        # Sets the attenuation value of one of more channels.  [arg
        # attnVal] should be an integer between 0 and 103.  Equivalent
        # to [arg group] [method attn] [arg attnVal]
    }

    method setGrpAttn {groupName attnVal} {
	$groupName attn $attnVal
    }

    UTF::doc {
        # [call [arg name] [method getGrpAttn] [arg group]]

        # Gets the attenuation value of all channels in the group.
        # Equivalent to [arg group] [method attn?]
    }

    method getGrpAttn {groupName} {
	$groupName attn?
    }

    UTF::doc {
        # [call [arg name] [method getGrpChans] [arg group]]

        # Return the list of channels belonging to the named group.
        # Equivalent to [arg group] [method cget] [arg -channels]
    }

    method getGrpChans {groupName} {
	$groupName cget -channels
    }

    UTF::doc {
        # [call [arg name] [method whatami]]

        # Return model information for the Attenuator hardware.
    }

    method whatami {} {
	return "Aeroflex [$self vashell *idn?]"
    }

    UTF::doc {
        # [call [arg name] [method reset]]

        # Reset the attenuator to its power-on state.
    }

    method reset {} {
	$self vashell "*rst"
    }

    method power {op} {
	if {$options(-power) eq ""} {
	    error "power $op: -power not configured"
	}
	eval [linsert $options(-power) 1 power $op]
    }

    variable socket
    variable stack
    variable depth 1

    method __getdata {} {
	# Internal read handler for the attenuator socket.  Appends
	# non-empty responses from the attenuator to the response
	# stack.  On EOF it closes the socket.
	set msg [gets $socket]
	if {$msg ne ""} {
	    # Record latest non-empty message
	    UTF::Message LOG $options(-name) $msg
	    lappend stack $msg
	}
	if {[eof $socket]} {
	    UTF::Message LOG $options(-name) EOF
	    close $socket; # nonblocking, so close should never fail
	    unset socket
	}
    }

    method vashell {cmd} {

	if {$options(-usevashell)} {
	    # old vashell relay version
	    # If the -lan_ip does not contain a port number, and is not a
	    # local device, add the default port
	    if {![regexp {:\d+$|/} $options(-lan_ip)]} {
		append options(-lan_ip) ":10001"
	    }
	    return [$options(-relay) rexec -noinit \
			"$UTF::unittest/vashell $options(-lan_ip) \"$cmd\""]
	}

	# Note: sending the *esr? is not only to detect errors,
	# but also to guarantee the attenator will complete the
	# setting before it returns.
	set cmd "*cls;$cmd;*esr?"

	# If socket is missing, or not readable, reopen it.  The call
	# to __getdata forces a read on the socket in order to provoke
	# an error if the socket needs reopening.  We don't expect
	# __getdata to have any data to read, but if it does it will
	# be reported as normal.  The "set socket" will catch the case
	# where the socket timed-out just as we were about to use it.
	# If calls are nested, only the outer layer gets to open the
	# socket.

	for {set i 0} {$i < $options(-retries)} {incr i} {
	    if {$depth < 2 && [catch {$self __getdata; set socket} ret]} {
		# Missing socket variable and non-opened channel are
		# normal.  Other failures should be reported.
		if {![regexp {"socket": no such variable|find channel} $ret]} {
		    UTF::Message WARN $options(-name) $ret
		}

		# (re)init stack
		set stack ""

		set socket [$options(-relay) socket $options(-lan_ip) 10001]
		fconfigure $socket -blocking 0 -buffering line
		fileevent $socket readable [mymethod __getdata]

		# Removed DRAIN code.  It cannot be defend against a
		# deliberate attack and it makes it almost impossible
		# to support nested invocations.  XPort input flushing
		# will clean up most stale data.  Anything that
		# manages to escape the XPort flush may make the first
		# call fail, but subsequent calls will recover.
	    }

	    if {$i eq 0} {
		UTF::Message LOG $options(-name) "send $cmd"
	    }
	    # We don't expect the write to fail since it's nonblocking and
	    # we already verified the socket was connected
	    puts $socket $cmd

	    # Collect results.  Depth is used to measure recursive calls.
	    # Collect enough results to satisfy the current depth then the
	    # outer calls can unwind the stack to return their results in
	    # the right order.  Timeouts will be recorded on the stack
	    # just like data so we shouldn't get out of sync.
	    while {[llength $stack] < $depth} {
		# Need more results
		incr depth
		set timer [after $options(-timeout) \
			       [list lappend [myvar stack] TIMEOUT]]
		vwait [myvar stack]
		after cancel $timer
		incr depth -1
	    }

	    set response [lindex $stack end]
	    set stack [lreplace $stack end end]

	    if {$response ne "TIMEOUT"} {
		break
	    }
	}
	if {$i > 0} {
	    UTF::Message LOG $options(-name) "(retries: $i)"
	    if {![info exists ::UTF::warn]} {
		set ::UTF::warn "Attn retries $i"
	    }
	}

	if {$response eq "TIMEOUT"} {
	    error $response
	}
	if {[regexp {^(\d+)$} $response - code]} {
	    set ret ""
	} elseif {![regexp {^(.*);(\d+)$} $response - ret code]} {
	    error "Bad response: $response"
	}
	switch $code {
	    0 { return $ret }
	    8 { error "DDE: Device Dependent error" }
	    16 { error "EXE: Execution error" }
	    32 { error "CME: Command error" }
	    default { error "$code: Unknown error" }
	}
    }


    UTF::doc {
        # [call [arg name] [method setup] [lb][arg {args ...}][rb]]

        # Perform any necessary configuration of the Attenuator
        # hardware.  In particular this sets timeouts and flush
        # options on the XPort telnet inmterface embedded in the
        # Attenuator. [para]

	# Equivalent to running [cmd bin/xportsetup] [arg {args ...}]
    }

    method setup {args} {
	$options(-relay) $UTF::unittest/bin/xportsetup \
	    $options(-lan_ip) {*}$args
	return
    }

    UTF::doc {
	# [list_end]
    }
}

# Retrieve manpage from last object
UTF::doc [UTF::Aeroflex man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]
    package require UTF::Aeroflex
    UTF::Aeroflex af -lan_ip 192.168.21.60 -relay UTFTestD -group {G {1 2 3}}

    G attn 45
    ->

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
