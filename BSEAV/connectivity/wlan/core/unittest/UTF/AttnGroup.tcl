#!/bin/env utf

# UTF Framework Object Definition for the aeroflex variable attenuator
#
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::AttnGroup 2.0

package require snit
package require UTF::doc

UTF::doc {
    # [manpage_begin UTF::Aeroflex n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF AttnGroup}]
    # [copyright {2011 Broadcom Corporation}]
    # [require UTF::AttnGroup]
    # [description]
    # [para]

    # UTF::AttnGroup object that provides the front-end API for UTF
    # test scripts to control an attenuator.  The back-end
    # implementation will be handled by HW-dependent objects.

    # [list_begin definitions]
}

snit::type UTF::AttnGroup {

    UTF::doc {
	# [call [cmd UTF::AttnGroup] [arg group]
	#	[option -aflex] [arg {aflex object}]]

	# Create a new AttnGroup object, representing a group of
	# attenuator channels to be controlled as a unit.

	# [list_begin options]

	# [opt_def [option -aflex] [arg aflex_object]]

	# Parent object representing the Attenuator.

	# [opt_def [option -channels] [arg {chan1 chan2 ...}]]

	# List of channels in group.

	# [opt_def [option -default] [arg value]]

	# Attenuation to be applied if the word "default" is supplied
	# as an attenuation value in the attn method.  Default 0.

	# [list_end]

	# [para]
	# AttnGroup objects have the following methods:
	# [list_begin definitions]

	# [call [arg group] [method {attn default}]]

	# Set default attenuation, based on the [option -default]
	# configuration option.  Defaults to 0.

	# [call [arg group] [method attn] [lb][arg attenuation][rb]]

	# Set attenuation value for the group.  With no argument it
	# behaves the same as [method attn?]

	# [call [arg group] [method attn?]]

	# Get attenuation values for the group.  Returns a
	# list with the value for each channel.

	# [call [arg group] [method incr] [arg size]]

	# Increment or decrement attenuator values.

        # [call [arg group] [method whatami]]

        # Return model information for the Attenuator hardware.

	# [list_end]

	# [para]
    }

    pragma -canreplace yes
    option -name
    option -aflex -readonly yes
    option -channels -type UTF::AttnGroup::chanvallist
    option -default -type snit::integer -default 0

    component aflex -public aflex -inherit yes

    typeconstructor {
	snit::stringtype chanval -regexp {\d+([+-]\d+)?}
	snit::listtype chanvallist -type ${type}::chanval
    }

    variable mergelist {}

    constructor {args} {
	if {[lindex $args 0] eq "merge"} {
	    if {[llength $args] ne 2} {
		error "Usage: $type <name> merge <list>
       $type <name> -options ..."
	    }
	    set mergelist [lindex $args 1]
	} else {
	    $self configurelist $args
	    set aflex $options(-aflex)
	    if {$options(-name) eq ""} {
		$self configure -name [namespace tail $self]
	    }
	}
    }

    method attn {{val ""}} {
	UTF::Message LOG $options(-name) "attn $val"
	if {$mergelist ne ""} {
	    set ret ""
	    foreach m $mergelist {
		set ret [concat $ret [$m attn $val]]
	    }
	    return $ret
	}
	if {$val eq ""} {
	    $aflex getChanAttn $options(-channels)
	} else {
	    if {$val eq "default"} {
		set val $options(-default)
	    }
	    $aflex setChanAttn $options(-channels) $val
	}
    }
    method attn? {} {
	$self attn {}
    }

    method incr {size} {
	UTF::Message LOG $options(-name) "incr $size"
	if {$mergelist ne ""} {
	    foreach m $mergelist {
		$m incr $size
	    }
	} else {
	    $aflex incr $options(-channels) $size
	}
    }
}

# Retrieve manpage from last object
UTF::doc [UTF::AttnGroup man]

UTF::doc {
    # [list_end]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
