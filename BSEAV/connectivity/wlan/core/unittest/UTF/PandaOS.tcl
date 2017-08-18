#!/bin/env utf
# -*-tcl-*-
#
# UTF Framework Object Definitions
# Based on snit
# $Copyright Broadcom Corporation$
#

package provide UTF::PandaOS 2.0

package require snit
package require UTF::doc
package require UTF::Base
package require UTF::Linux
package require UTF::LinuxOS

UTF::doc {
    # [manpage_begin UTF::PandaOS n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Panda support}]
    # [copyright {2013 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::PandaOS is an implementation of the UTF host object, specific
    # to Linux systems on Pandaboard. Using opensource drivers.

    # Once created, the PandaOS object's methods are not normally
    # invoked directly by test scripts, instead the PandaOS object is
    # designated the host for one or more platform independent STA
    # objects and it will be the STA objects which will be refered to
    # in the test scripts.

    # [list_begin definitions]

}

snit::type UTF::PandaOS {

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -sta -configuremethod CopyOption
    option -name -configuremethod CopyOption

    # base handles any other options and methods
    component base -inherit yes
    variable cloneopts

    constructor {args} {
	set cloneopts $args
 	install base using UTF::LinuxOS %AUTO%
	$self configurelist $args

        if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}

	foreach {sta dev} $options(-sta) {
	    if {$dev eq ""} {
		error "$sta has no device name"
	    }
	    UTF::STA ::$sta -host $self -device $dev
	}
    }

    destructor {
	catch {$base destroy}
	foreach {sta dev} $options(-sta) {
	    catch {::$sta destroy}
	}
    }

    # Internal method for copying options to delegates
    method CopyOption {key val} {
	set options($key) $val
	$base configure $key $val
    }

    method findimages {args} {
    	set dir [$base findimages $args]
    	$self rexec "rm -rf /tmp/UTFmods"
    	# we copy the module folder to the device
    	$self copyto $dir "/tmp/UTFmods"
    	return "/tmp/UTFmods"
    }

    UTF::doc {
	# [call [arg host] [method ifconfig] [arg dev] [option dhcp]]

	# Enable DHCP on device [arg dev].

	# [call [arg host] [method ifconfig] [arg {args ...}]]

	# Run ifconfig on the host, disabling DHCP if necessary.
    }


    # IP address cache
    variable ipaddr -array {}

    method ifconfig {dev args} {

	# Check the interface settings.  Devices "loaded" have already
	# been checked, but still need to check reference endpoints
	# and devices defined at runtime.
	#$self check_network_scripts $dev

	set PF "/var/run/dhclient-${dev}.pid"
	if {[llength $args]} {
	    # Setting something - kill off dhclient
	    catch {$self "test -f $PF && kill `cat $PF` 2>/dev/null"}
	}
	if {$args eq "dhcp"} {

	    # invalidate cache in case of failure
	    if {[info exists ipaddr($dev)]} {
		unset ipaddr($dev)
	    }

	    set dhclient "/sbin/udhcpc"
	    # Allow for one retry on dhclient collision
	    for {set i 0} {$i < 2} {incr i} {
		if {![catch {
		    $self $dhclient -p $PF -i $dev
		} ret]} {
		    break
		}
		# Pass up only the last line of the error, which
		# usually contains the actual failure message
		error [lindex [split $ret "\n"] end]
	    }

	    if {![regexp {Lease of (\S+) obtained} $ret - ip]} {
		# can't update cache
		return $ret
	    }
	    # Return ipaddr and Update cache
	    return [set ipaddr($dev) $ip]
	} else {
	    # Since parsing a full ifconfig commandline is hard, just
	    # invalidate the cache and let ipaddr do the work next
	    # time.
	    if {$args ne "" && [info exists ipaddr($dev)]} {
		unset ipaddr($dev)
	    }
	    $base ifconfig $dev $args
	}
    }
    UTF::doc {
	# [call [arg host] [method ping] [arg target]
	#	[lb][opt -c] [arg count][rb]
	#       [lb][opt -s] [arg size][rb]]

	# Ping from [arg host] to [arg target].  Target may be a STA
	# or a Hostname/IP address.  Returns success if a response
	# packet is received and error if not.  Returns success if a
	# response packet is received before [arg count] (default 5)
	# packets have been sent.  Packet size option [opt -s] [arg
	# size] is ignored.
    }

    method ping {target args} {
	set msg "ping $target $args"
	UTF::Getopts {
	    {c.arg "5" "Count"}
	    {s.arg "56" "Size (ignored)"}
	} args
	if {[info commands $target] == $target} {
	    set target [$target ipaddr]
	}
	# Loop, since Panda ping doesn't handle "count" reliably
	for {set c 0} {$c < $(c)} {incr c 3} {
	    catch {$self rexec -t 10 ping -q -c 1 -w 3 -s $(s) $target} ret
	    if {[regexp {1 packets received} $ret]} {
		return
	    }
	}
	error "ping failed"
    }

}
#/* vim: set filetype=tcl : */
