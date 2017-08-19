#!/bin/env utf

# UTF Framework Object Definition for the ChanAssy variable
# attenuator, used by DVT.
#
# $Id: 26f22be99d6837753f95d55a642f55797680daa4 $
# $Copyright Broadcom Corporation$
#

package provide UTF::ChanAssy 2.0

package require snit
package require UTF
package require UTF::AttnGroup
package require UTF::doc

UTF::doc {
    # [manpage_begin UTF::ChanAssy n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF ChanAssy}]
    # [copyright {2012 Broadcom Corporation}]
    # [require UTF::ChanAssy]
    # [description]
    # [para]

    # [cmd UTF::ChanAssy] is a object that provides an HW-specific
    # backend for [cmd UTF::AttnGroup] to control the ChanAssy
    # variable attenuator, used by DVT.

    # This object is usually defined in your utfconf file, but it will
    # create one or more [cmd AttnGroup] objects.  The [cmd AttnGroup]
    # objects are used directly in test scripts, not the Aeroflex
    # object.

    # [list_begin definitions]
}

snit::type UTF::ChanAssy {

    typeconstructor {
	snit::double attnval -min -1 -max 121
	snit::listtype attnvallist -type ${type}::attnval

	snit::integer chanval -min 0
	snit::listtype chanvallist -type ${type}::chanval
    }

    UTF::doc {
        # [call [cmd UTF::ChanAssy] [arg name]
	#	[option -relay] [arg string]
	#	[option -group]
	#	  [arg {{Group1 {Channels ...}} {Group2 {Channels ...}}}]]

        # Create a new ChanAssy object. It will be used to control
	# various functionalities of the ChanAssy variable attenuator.
	# [list_begin options]

        # [opt_def [option -name] [arg name]]

	# Name of the variable attenuator.

	# [opt_def [option -relay] [arg Host]]

	# Relay host used to talk to the attenuator.  Defaults to
	# localhost.

	# [opt_def [option -group]
	#	  [arg {{Group1 {Channels ...}} {Group2 {Channels ...}}}]]

	# Defines group objects and their channel lists.

        # [list_end]

        # [para]
        # Aeroflex objects have the following methods:
        # [list_begin definitions]
    }

    pragma -canreplace yes
    option -name
    option -relay localhost
    option -group -configuremethod GroupOption

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

    method setChanAttn {chanList attnVal} {
	chanvallist validate $chanList
	attnval validate $attnVal
	$options(-relay) rexec "cd /home/chunyuhu/driver5/tot/src/tools/wlan && ./bld_control.sh $attnVal [join $chanList {,}]"
    }

    UTF::doc {
        # [call [arg name] [method getChanAttn] [arg chanlList]]

        # Return attenuation values of the numbered channels. (Not
        # implemented)
    }

    method getChanAttn {chanList} {
	# Not implemented
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
	return "ChanAssy"
    }

    UTF::doc {
	# [list_end]
    }
}

# Retrieve manpage from last object
UTF::doc [UTF::ChanAssy man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]
    package require UTF::ChanAssy
    UTF::ChanAssy ca -relay scrabble -group {G {0 1 2}}

    G attn 45
    ->

    # [example_end]

    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
