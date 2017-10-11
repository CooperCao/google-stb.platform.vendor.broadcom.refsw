#!/bin/env utf

# UTF Framework Object Definition for the Vaunix variable attenuator
#
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::Vaunix 2.0

package require snit
package require UTF
package require UTF::AttnGroup
package require UTF::doc

UTF::doc {
    # [manpage_begin UTF::Vaunix n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Vaunix}]
    # [copyright {2009 Broadcom Corporation}]
    # [require UTF::Vaunix]
    # [description]
    # [para]

    # [cmd UTF::Vaunix] is a object that provides an HW-specific
    # backend for [cmd UTF::AttnGroup] to control the Vaunix
    # variable attenuator

    # This object is usually defined in your utfconf file.  If a
    # -group list is specified, it will create one or more [cmd
    # AttnGroup] objects.  If there is only one Vaunix attenuator
    # present then the Vaunix object can be used directly without
    # having to specify a group.

    # [list_begin definitions]
}

snit::type UTF::Vaunix {

    typeconstructor {
	snit::double attnval -min 0 -max 103
	snit::listtype attnvallist -type ${type}::attnval

	snit::integer chanval -min 1
	snit::listtype chanvallist -type ${type}::chanval
    }

    UTF::doc {
        # [call [cmd UTF::Vaunix] [arg name]
	#	[option -lan_ip] [arg address]
	#	[option -relay] [arg string]
	#	[option -group]
	#	  [arg {{Group1 {Channels ...}} {Group2 {Channels ...}}}]]

        # Create a new Vaunix object. It will be used to control
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
	#	  [arg {{Group1 {SNs ...}} {Group2 {SNs ...}}}]]

	# Defines group objects and their serial number lists.  Since
	# each Vaunix device only provides a single channel, channel
	# groups are defined by their serial numbers.

        # [list_end]

        # [para]
        # Vaunix objects have the following methods:
        # [list_begin definitions]
    }

    pragma -canreplace yes
    option -lan_ip
    option -name
    option -relay localhost
    option -port 57000
    option -sn
    option -group -configuremethod GroupOption
    option -power
    option -default 0

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


    variable readable
    method setChanAttn {chan {val ""}} {
	set s [$options(-relay) socket $options(-lan_ip) $options(-port)]
	fconfigure $s -blocking 0 -buffering none
	fileevent $s readable [list set [myvar readable] 1]
	set timer [after 1000 [list set [myvar readable] "Timeout"]]
	foreach sn $chan {
	    UTF::Message LOG $options(-name) "attn #$sn $val"
	    puts $s "$sn $val"
	    flush $s
	    vwait [myvar readable]
	    if {$readable eq "Timeout"} {
		error $readable
	    }
	    lappend ret [gets $s]
	}
	after cancel $timer
	close $s
	UTF::Message LOG $options(-name) "$ret"
	return $ret
    }

    method getChanAttn {chan} {
	$self setChanAttn $chan
    }

    method attn {{attnVal ""}} {
	if {$attnVal eq "default"} {
	    set attnVal $options(-default)
	}
	$self setChanAttn 0 $attnVal
    }

    UTF::doc {
	# [list_end]
    }
}

# Retrieve manpage from last object
UTF::doc [UTF::Vaunix man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]
    package require UTF::Vaunix
    UTF::Vaunix G -lan_ip utftestf
    UTF::Vaunix af1 -lan_ip utfdev -group {G2 {55102 55602}}

    # [example_end]

    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
