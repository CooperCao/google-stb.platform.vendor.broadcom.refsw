#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: 53b023c9c5e618e9d17ed66913663df77011f4b7 $
# $Copyright Broadcom Corporation$
#

package provide UTF::Airport 2.0

package require snit
package require UTF::doc
package require UTF::Router

UTF::doc {
    # [manpage_begin UTF::Airport n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Airport AP support}]
    # [copyright {2010 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::Airport host object, specific to Apple Airport APs.

    # Once created, the Airport object's methods are not normally
    # invoked directly by test scripts, instead the Airport object is
    # designated the host for one or more platform independent STA
    # objects and it will be the STA objects which will be refered to
    # in the test scripts.

    # [list_begin definitions]

}

snit::type UTF::Airport {
    UTF::doc {
	# [call [cmd UTF::Airport] [arg host] [arg ...]]

	# Create a new Airport host object.  The host will be
	# used to contact STAs residing on that host.
	# [list_begin options]

	# [opt_def [arg ...]]

	# There are no Airport specific options.  All options will be
	# passed on to the [cmd UTF::Router] object.

	# [list_end]
	# [list_end]

	# [para]
	# Airport objects have the following methods:
	# [para]
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    # base handles any other options and methods
    component base -inherit no
    delegate option * to base
    delegate method relay to base
    delegate method power to base
    delegate method wan to base

    option -sta
    option -name -configuremethod CopyOption

    variable stas {}

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	install base using UTF::Router %AUTO% -relay localhost \
	    -lan_ip 192.168.2.1 -passwd public -init [mymethod init]
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	foreach {sta dev} [UTF::Staexpand $options(-sta)] {
	    if {$dev eq ""} {
		error "$sta has no device name"
	    } else {
		UTF::STA ::$sta -host $self -device $dev
		lappend stas ::$sta
	    }
	}
	if {[$self cget -relay] eq ""} {
	    error "No -relay specified for $options(-name)"
	}
    }

    destructor {
	catch {$base destroy}
	foreach sta $stas {
	    catch {::$sta destroy}
	}
    }

    # Internal method for copying options to delegates
    method CopyOption {key val} {
	set options($key) $val
	$base configure $key $val
    }

    method wlname {dev} {
	return "radios.\[$dev\]"
    }

    method acp {args} {
	if {[$self cget -passwd] ne "public"} {
	    set args [lreplace $args 0 -1 -p [$self cget -passwd]]
	}
	# Hack for hex keys
	regsub {\.raWE} $args {.raWE --hex} args
	set ret [eval $self relay $::UTF::projtools/linux/bin/acp \
		     -a [$self cget -lan_ip] $args]
	if {[lsearch $args -r] >= 0} {
	    # Wait for reboot
	    UTF::Sleep 20
	    for {set i 0} {$i < 120 && [catch {$self acp syNm}]} {incr i} {}
	}
	return $ret
    }

    method wl {args} {
	UTF::Message LOG $options(-name) "wl $args"
	UTF::GetKnownopts {
	    {silent "Suppress logging"}
	    {u "Unsupported is ok"}
	    {i.arg "0" "Interface"}
	}
	set cmd [lindex $args 0]
	set args [lreplace $args 0 0]

	set wlname [$self wlname $(i)]
	set code 0
	set ret ""
	switch $cmd {
	    ssid {
		set ret [$self acp -q getplistvalue WiFi "$wlname.raNm"]
	    }
	    bssid {
		set ret [join [regexp -inline -all {[[:xdigit:]]{2}} \
				  [string toupper \
				       [$self acp -q getplistvalue WiFi \
					    "$wlname.raMA"]]] :]
	    }
	    chanspec {
		set ret [$self acp -q getplistvalue WiFi "$wlname.raCh"]
	    }
	    infra {
		set ret 1
	    }
	    ver {
		set ret [$self acp syDs]
	    }
	    up {
	    }
	    band {
		# hardcoded
		if {$(i) eq "0"} {
		    set ret "b"
		} else {
		    set ret "a"
		}
	    }
	    chanspecs {
		# Hardcoded
		if {$(i) eq "0"} {
		    set ret {
			1 (0x2b01)
			2 (0x2b02)
			3 (0x2b03)
			4 (0x2b04)
			5 (0x2b05)
			6 (0x2b06)
			7 (0x2b07)
			8 (0x2b08)
			9 (0x2b09)
			10 (0x2b0a)
			11 (0x2b0b)}
		} else {
		    set ret {
			36 (0x1b24)
			40 (0x1b28)
			44 (0x1b2c)
			48 (0x1b30)
			52 (0x1b34)
			56 (0x1b38)
			60 (0x1b3c)
			64 (0x1b40)
			149 (0x1b95)
			153 (0x1b99)
			157 (0x1b9d)
			161 (0x1ba1)
			165 (0x1ba5)
		    }
		}
		regsub -line -all {^\s+} $ret {} ret
	    }
	    default {
		set ret "wl $cmd not implemented"
		set code 1
	    }
	}
	if {$code && !$(u)} {
	    UTF::Message LOG $options(-name) $ret
	    error $ret
	} else {
	    UTF::Message LOG $options(-name) $ret
	    return $ret
	}
    }

    method ipaddr {dev} {
	$base cget -lan_ip
    }

    method restart {args} {
	error "restart not implemented"
    }

    method restore_defaults {} {

	array set nvram [UTF::decomment [$base cget -nvram]]

	set updated 0
	foreach nv [array names nvram] {
	    switch -regexp $nv {
		{^[a-zA-Z]{4}$} {
		    if {[$self acp -q $nv] ne $nvram($nv)} {
			$self acp $nv=\"$nvram($nv)\"
			set updated 1
		    }
		}
		{^radios} {
		    if {[$self acp -q getplistvalue WiFi $nv] ne $nvram($nv)} {
			$self acp setplistvalue WiFi $nv \"$nvram($nv)\"
			set updated 1
		    }
		}
		default {
		    error "Illegal nvram name: $nv"
		}
	    }
	}
	if {$updated} {
	    $self acp -r
	}
    }

    method findimages {} {
	# None
    }

    method load {} {
	# No-op
    }

    method nvram {args} {
	error "nvram not implemented"
    }

    method pci {dev} {
	error "Not implemented"
    }

    method ping {args} {
	eval $self relay ping $args
    }

    method whatami {args} {
	# use expr to convert from hex
	set pid [expr {[$self acp -q syAP]}]
	switch $pid {
	    1 { set type "P19" }
	    2 { set type "P85" }
	    3 { set type "P81"}
	    4 { set type "Q57" }
	    100 { set type "S5" }
	    101 { set type "Q60" }
	    102 { set type "Q61" }
	    104 { set type "M28" }
	    105 { set type "M91" }
	    106 { set type "M52" }
	    107 { set type "M48" }
	    108 { set type "K10" }
	    109 { set type "K30" }
	    110 { set type "K54" }
	    111 { set type "K55" }
	    112 { set type "K56" }
	    113 { set type "K30A" }
	    114 { set type "K10A" }
	    default { set type "unknown-$pid" }
	}
	return "Airport $type"
    }

}

# Retrieve manpage from last object
UTF::doc [UTF::Airport man]

UTF::doc {
    # [list_end]
}

UTF::doc {
    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also wl]
    # [see_also [uri APdoc.cgi?UTF.tcl UTF]]
    # [see_also [uri APdoc.cgi?UTF::Base.tcl UTF::Base]]
    # [see_also [uri APdoc.cgi?UTF::Cygwin.tcl UTF::Cygwin]]
    # [see_also [uri APdoc.cgi?UTF::Linux.tcl UTF::Linux]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
