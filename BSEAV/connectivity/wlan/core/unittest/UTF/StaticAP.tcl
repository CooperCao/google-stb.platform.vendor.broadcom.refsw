#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::StaticAP 2.0

package require snit
package require UTF::doc

UTF::doc {
    # [manpage_begin UTF::StaticAP n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Static AP/Router support}]
    # [copyright {2016 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::Static host object, wrapper for unmanaged AP/Routers.

    # This is a stub, hosting pre-configured static settings but unable
    # to query or change the device in any way.

    # [list_begin definitions]

}

snit::type UTF::StaticAP {
    UTF::doc {
	# [call [cmd UTF::StaticAP] [arg host]
	#	[option -sta] [arg {{name {options} ...}}]
	#	[lb][option -lan_ip] [arg address][rb]
	#	[lb][option -name] [arg name][rb]
	#       [lb][option -lanpeer] [arg peer][rb]]

	# Create a new StaticAP host object.
	# [list_begin options]

	# [opt_def [option -sta] {{name {options} ...}}]

	# List of WiFi interfaces on the Device, and their
	# configurations.  Usually one 2g and one 5g interface.

	# Each interface may be provided with a list of settings.  See
	# example below.  At least one interface must be specified.

	# [opt_def [option -lan_ip] [arg address]]

	# IP address of the device.  If the device is a Router, this
	# will be the internal-facing LAN address.  Defaults to
	# 192.168.1.1

	# [opt_def [option -name] [arg name]]

	# Specify an alternate [arg name] to use when logging.  The
	# default is the came as the [cmd host].

	# [opt_def [option -lanpeer] [arg peer]]

	# Specify an alternate host to use for performance tests, etc.
	# Usually a PC connected to one of the Router's LAN ports.
	# The default is to run performance tests directly against the
	# device.

	# [list_end]
	# [list_end]

	# [para]
	# Router objects have the following methods:
	# [para]
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -lan_ip 192.168.1.1
    option -sta
    option -name
    option -lanpeer

    # Dummy console option, for Smoketest
    option -console -readonly true

    variable stas {}

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	foreach {sta dev} [UTF::Staexpand $options(-sta)] {
	    UTF::STA ::$sta -host $self {*}$dev -revinfo unknown
	    lappend stas ::$sta
	}
    }

    destructor {
	foreach {sta dev} $options(-sta) {
	    catch {$sta destroy}
	}
    }

    UTF::doc {
	# [call [arg host] [method restore_defaults] [arg args]]

	# No-op.
    }

    method restore_defaults {args} {
	return
    }

    UTF::doc {
	# [call [arg host] [method lanwanreset]]

	# Reset IP on lanpeer.
    }

    method lanwanreset {} {
	foreach l [$self cget -lanpeer] {
	    $l ifconfig [$l cget -ipaddr]
	    $l add_networks $self
	}
    }

    UTF::doc {
	# [call [arg host] [method ipaddr] [arg dev]]

	# Return IP address of device.
    }

    method ipaddr {dev} {
	return $options(-lan_ip)
    }

    UTF::doc {
	# [call [arg host] [method rte_available]]

	# Returns false as this device does not support RTE
    }

    method rte_available {} {
	return 0
    }

    UTF::doc {
	# [call [arg host] [method boardname]]

	# Returns "StaticAP"
    }

    method boardname {} {
	return "StaticAP"
    }

    UTF::doc {
	# [call [arg host] [method wlname] [arg dev]]

	# Returns empty since there is no wl in this device.
    }

    method wlname {dev} {
	return
    }


    UTF::doc {
	# [call [arg host] [method whatami]]

	# Returns "StaticAP"
    }

    method whatami {{STA ""}} {
	return "StaticAP"
    }

    UTF::doc {
	# [call [arg host] [method wan]]

	# Returns empty as we don't support a WAN on this device.
    }

    # AP only - no WAN
    method wan {} {}

    # Peer passthroughs
    UTF::PassThroughMethod lan -lanpeer

    UTF::doc {
	# [call [arg host] [method wlconf_by_nvram]]

	# Throws an error as this device is not configurable
    }

    method wlconf_by_nvram {} {
	error "Attempting configure a StaticAP"
    }

}

# Retrieve manpage from last object
UTF::doc [UTF::StaticAP man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]

package require UTF::StaticAP

UTF::StaticAP %AUTO% \\
    -lan_ip 192.168.0.1 \\
    -lanpeer lan \\
    -sta {
		5g {
		    -ssid My5gAP
		    -security aespsk2
		    -wpakey JSQB_UGCDcAilr7nIFKnqKEdKGQohoNqLxmhnrG77m0P9p9e
		}
		2g {
		    -ssid My2gAP
		    -security open
		}
    }

    # [example_end]
}

UTF::doc {
    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also [uri APdoc.cgi?UTF.tcl UTF]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
