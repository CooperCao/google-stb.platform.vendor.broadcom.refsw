#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: 7798ff6893a60d104423fca03813d60fa15700af $
# $Copyright Broadcom Corporation$
#

package provide UTF::Power 2.0

package require snit
package require UTF::doc
package require UTF::Linux
package require http

UTF::doc {
    # [manpage_begin UTF::Power n 2.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF power switch support}]
    # [copyright {2009 Broadcom Corporation}]
    # [require UTF::Power]
    # [description]
    # [para]

    # The Power package includes several snit types for managing
    # remote control power switches. [para]

    # Each object provides [method {power on}], [method {power off}]
    # and [method {power cycle}] methods, which may take additional
    # arguments to indicate which port to control (depending on wether
    # or not the switch supports multiple ports.  These methods refer
    # to the ultimate effect they have on the device under test, not
    # the physical state needed to achieve that affect.  For example,
    # a normally closed relay might need to be turned "off" in order
    # to power "on" the device under test.  [para]

    # Meta objects can be constructed for situations where one or more
    # power switches need sequencing to achieve the desired effect.
    # [para]

    # Objects may offer additional methods to expose unique
    # functionality of the underlying switch hardware.

    # [list_begin definitions]

}



snit::type UTF::Power::CanaKit {

    UTF::doc {

	# [call [cmd UTF::Power::CanaKit] [arg name]
	#	[lb][option -lan_ip] [arg host:port][rb]
	# 	[lb][option -relay] [arg relay][rb]
	#       [lb][option -invert] [arg {{ports ...}}][rb]]

	# Create a new UTF::Power::CanaKit object.[para]

	# UTF::Power::CanaKit is an implementation of the UTF Power
	# object using CanaKit UK1104 USB-controlled relay module or
	# similar device.

	# [list_begin options]

	# [opt_def [option -lan_ip] [arg host:port]]

	# Specify the CanaKit's host:port.  This should be a
	# consolelogger instance.

	# [opt_def [option -relay] [arg relay]]

	# Specify a relay host for controlling the device.  The
	# default is [arg localhost], which will work for CanaKit's
	# USB connected to hosts on the public net.  If the CanaKit
	# host is on a private net then a relay host with access to
	# that net is required.

	# [opt_def [option -invert] [arg {{ports ...}}]]

	# List of ports for which the sense of [method {power off}]
	# and [method {power on}] should be inverted.  [para]

	# Inverted ports are useful for controlling things like dongle
	# power interrupters where you can use the NC connector to
	# maintain constant power and use [method {power cycle}] to
	# briefly interrupt the connection.  With ports used this way
	# you want [method {power off}] to turn the relay on and
	# [method {power on}] to turn the relay off.

	# [list_end]
	# [para]
	# UTF::Power::CanaKit objects have the following methods:
	# [list_begin definitions]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -lan_ip ""
    option -relay localhost
    option -invert -type {snit::listtype -type UTF::Power::CanaKit::portnum}

    typeconstructor {
	snit::integer portnum -min 1 -max 4
    }

    UTF::doc {
	# [call [arg name] [method {power off}] [arg port]]

	# Power off the device controlled by the numbered port.
    }

    method {power off} {port} {
	if {[lsearch $options(-invert) $port] >= 0} {
	    $self on $port
	} else {
	    $self off $port
	}
    }

    UTF::doc {
	# [call [arg name] [method {power on}] [arg port]]

	# Power on the device controlled by the numbered port.
    }

    method {power on} {port} {
	if {[lsearch $options(-invert) $port] >= 0} {
	    $self off $port
	} else {
	    $self on $port
	}
    }

    UTF::doc {
	# [call [arg name] [method {power cycle}] [arg port]]

	# Power cycle the device controlled by the numbered port.
    }

    method {power cycle} {port} {
	$self on $port
	UTF::Sleep 0.5
	$self off $port
    }

    UTF::doc {
	# [call [arg name] [method on] [arg port]]

	# Switch the numbered relay on.
    }

    method on {port} {
	$self shell REL${port}.ON
    }

    UTF::doc {
	# [call [arg name] [method off] [arg port]]

	# Switch the numbered relay off.
    }

    method off {port} {
	$self shell REL${port}.OFF
    }

    UTF::doc {
	# [call [arg name] [method toggle] [arg port]]

	# Toggle the numbered relay.
    }

    method toggle {port} {
	$self shell REL${port}.TOGGLE
    }

    UTF::doc {
	# [call [arg name] [method get] [arg port]]

	# Query the numbered relay state.
    }

    method get {port} {
	$self shell REL${port}.GET
    }

    UTF::doc {
	# [call [arg name] [method ver] [arg port]]

	# Query the relays HW info.
    }

    method ver {} {
	$self shell ABOUT
    }

    # shell actually does all the work
    method shell {cmd} {
	$options(-relay) rexec -n \
	    $UTF::usrutf/canakitshell $options(-lan_ip) $cmd
    }


    UTF::doc {
	# [list_end]
	# [para]
    }
}


snit::type UTF::Power::Synaccess {
    UTF::doc {

	# [call [cmd UTF::Power::Synaccess] [arg name]
	#	[lb][option -lan_ip] [arg address][rb]
	# 	[lb][option -relay] [arg relay][rb]
	#	[lb][option -user] [arg user][rb]
	#	[lb][option -passwd] [arg passwd][rb]]

	# Create a new UTF::Power::Synaccess object.[para]

	# UTF::Power::Synaccess is an implementation of the UTF Power
	# object using Synaccess NPC22 or similar devices, accessed
	# via http.

	# [list_begin options]

	# [opt_def [option -lan_ip] [arg address]]

	# Specify the Synaccess device's hostname or IP address.  The
	# default is to use the [arg name] of the device.

	# [opt_def [option -relay] [arg relay]]

	# Specify a relay host for controlling the device.  The
	# default is [arg localhost], which will work for Synaccess
	# devices on the public net.  If the Synaccess device is on a
	# private net then a relay host with access to that net is
	# required.

	# [opt_def [option -user] [arg username]]

	# Specify username for access to the Synaccess web page.
	# Defaults to "admin".

	# [opt_def [option -passwd] [arg password]]

	# Specify password for access to the Synaccess web page.
	# Defaults to "admin".

	# [opt_def [option -rev] [arg 0]]

	# API revision.  Rev 0 is for older devices which use shtml
	# pages.  Rev 1 is for newer devices with a direct programming
	# interface.

	# [list_end]
	# [para]
	# UTF::Power::Synaccess objects have the following methods:
	# [list_begin definitions]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -user "admin"
    option -passwd "admin"

    option -lan_ip ""
    option -relay localhost
    option -rev -type {snit::integer -min 0 -max 1} -default 0

    typeconstructor {
	snit::integer portnum -min 1 -max 8
    }

    constructor {args} {
	$self configurelist $args
	if {$options(-lan_ip) eq ""} {
	    set options(-lan_ip) [namespace tail $self]
	}
    }

    UTF::doc {
	# [call [arg name] [method {power off}] [arg port]]

	# Power the numbered port off.
    }

    method {power off} {port} {
	if {$options(-rev) == 1} {
	    $self api "%24A3+$port+0"
	    return
	}
	if {[$self State $port]} {
	    $self Toggle $port
	} else {
	    UTF::Message LOG $self "$port: Already off"
	}
    }

    UTF::doc {
	# [call [arg name] [method {power on}] [arg port]]

	# Power the numbered port on.
    }

    method {power on} {port} {
	if {$options(-rev) == 1} {
	    $self api "%24A3+$port+1"
	    return
	}
	if {![$self State $port]} {
	    $self Toggle $port
	} else {
	    UTF::Message LOG $self "$port: Already on"
	}
    }

    UTF::doc {
	# [call [arg name] [method {power cycle}] [arg port]]

	# Power the numbered port off and then on.
    }

    method {power cycle} {port} {
	portnum validate $port
	if {$options(-rev) == 1} {
	    $self api "%24A4+$port"
	    return
	}
	$self State $port [$self http get "pwrRb$port.cgi"]
    }


    UTF::doc {
	# [call [arg name] [method State] [arg port]]

	# Return the on/off state of the given port.  The return
	# value may be used as a TCL boolean.
    }
    method State {port {reply ""}} {
	portnum validate $port
	if {$options(-rev) == 1} {
	    set ret [$self api "%24A5"]
	    if {![regexp -line {\$A0,(\d+),(.*)} $ret - s r]} {
		error $ret
	    }
	    UTF::Message LOG "" "$s,$r"
	    set s [string index $s end-[expr {$port-1}]]
	    if {![string is boolean -strict $s]} {
		error "invalid port"
	    } else {
		return $s
	    }
	}
	if {$reply eq ""} {
	    set reply [$self http get "synOpStatus.shtml"]
	}
	if {[regexp -line "pwrRb$port.*" $reply l]} {
	    if {[regexp {ledon} $l]} {
		return "On"
	    } else {
		return "Off"
	    }
	} else {
	    error "Port $port not found"
	}
    }

    UTF::doc {
	# [call [arg name] [method Toggle] [arg port]]

	# Toggle the on/off state of the numbered port.  This is the
	# web-page primitive for changing the port state.  The current
	# state should be queried first in order to make a
	# predicatable change.
    }
    method Toggle {port} {
	portnum validate $port
	if {$options(-rev) == 1} {
	    set p [expr {$port - 1}]
	    $self api "rly=$p"
	    return
	} else {
	    $self State $port [$self http get "pwrSw$port.cgi"]
	}
    }

    UTF::doc {
	# [call [arg name] [method version]]

	# Report firmware version.
    }
    method version {} {
	if {$options(-rev) == 1 && [regexp {<td>Version</td><td>([^<]+)</td>} \
			      [$self http get index.htm]  - ver]} {
	    return $ver
	} elseif {[regexp {<TD>Firmware Ver</td><TD>([\w.]+)</TD>} \
		       [$self http get sysStatus.shtml] - ver]} {
	    return $ver
	}
	error $ver
    }

    UTF::doc {
	# [call [arg name] [method {http get}] [lb][arg page][rb]]

	# Send a http request to the device.
    }

    method {http get} {{page ""}} {
	$options(-relay) rexec -n -s wget -nv -O- \
	    --http-user=$options(-user) \
	    --http-passwd=$options(-passwd) \
	    http://$options(-lan_ip)/$page
    }

    UTF::doc {
	# [call [arg name] [method api] [cmd command]]

	# Send an API command to the device via cmd.cgi.  Check for
	# error return codes.
    }

    method api {cmd} {
	set ret [$self http get "cmd.cgi?$cmd"]
	if {[regexp {^\$AF} $ret]} {
	    error "failed: $ret"
	}
	set ret
    }

    UTF::doc {
	# [call [arg name] [method reset]]

	# Reset the device.  Sometimes needed to clear blocked TCP
	# ports.
    }
    method reset {} {
	$options(-relay) rexec -n \
	    $UTF::usrutf/apshell $options(-lan_ip) reset
    }

    UTF::doc {
	# [list_end]
	# [para]
    }
}


snit::type UTF::Power::WTI {

    UTF::doc {
	# [call [cmd UTF::Power::WTI] [arg name]
	#	[lb][option -lan_ip] [arg address][rb]
	# 	[lb][option -relay] [arg relay][rb]
	#	[lb][option -user] [arg user][rb]
	#	[lb][option -passwd] [arg passwd][rb]]

	# Create a new UTF::Power::WTI object.[para]

	# UTF::Power::WTI is an implementation of the UTF Power object
	# using WTI IPS-800, IPS-1600 and similar devices, accessed
	# via apshell.

	# [list_begin options]

	# [opt_def [option -lan_ip] [arg address]]

	# Specify the WTI device's hostname or IP address.  The
	# default is to use the [arg name] of the device.

	# [opt_def [option -relay] [arg relay]]

	# Specify a relay host for controlling the device.  The
	# default is [arg localhost], which will work for WTI devices
	# on the public net.  If the WTI device is on a private net
	# then a relay host with access to that net is required.

	# [list_end]
	# [para]
	# UTF::Power::WTI objects have the following methods:
	# [list_begin definitions]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -lan_ip
    option -relay localhost

    typeconstructor {
	snit::integer portnum -min 1 -max 16
    }

    constructor {args} {
	$self configurelist $args
	if {$options(-lan_ip) eq ""} {
	    set options(-lan_ip) [namespace tail $self]
	}
    }


    UTF::doc {
	# [call [arg name] [method {power off}] [arg port]]

	# Power the numbered port off.
    }

    method {power off} {port} {
	$self apshell off $port
    }

    UTF::doc {
	# [call [arg name] [method {power on}] [arg port]]

	# Power the numbered port on.
    }

    method {power on} {port} {
	$self apshell on $port
    }

    UTF::doc {
	# [call [arg name] [method {power cycle}] [arg port]]

	# Power the numbered port off and then on.
    }

    method {power cycle} {port} {
	$self apshell cycle $port
    }

    # apshell actually does all the work
    method apshell {op port} {
	portnum validate $port
	$options(-relay) rexec -n \
	    $UTF::usrutf/apshell $options(-lan_ip) power $op $port
    }
    UTF::doc {
	# [list_end]
	# [para]
    }
}

snit::type UTF::Power::WebRelay {
    UTF::doc {

	# [call [cmd UTF::Power::WebRelay] [arg name]
	#	[lb][option -lan_ip] [arg address][rb]
	# 	[lb][option -relay] [arg relay][rb]
	#       [lb][option -invert] [arg {{ports ...}}][rb]
	#	[lb][option -invertcycle] [arg {{ports ...}}][rb]]

	# Create a new UTF::Power::WebRelay object.[para]

	# UTF::Power::WebRelay is an implementation of the UTF Power
	# object using a Controlbyweb WebRelay device.  See [uri
	# http://www.controlbyweb.com/products.html]

	# [list_begin options]

	# [opt_def [option -lan_ip] [arg address]]

	# Specify the WebRelay device's hostname or IP address.  If
	# [option -relay] is specified then this defaults to
	# 192.168.1.2, which is the factory default for the WebRelay
	# device and allows it to be used unmodified if you connect it
	# to a Broadcom Router in your test rig (use caution to avoid
	# deadlocks if using the relay to power switch the same
	# router!). [para]

	# If [option -relay] is not specified then [option -lan_ip]
	# defaults to the [arg name] of the device, similar to other
	# UTF objects.

	# [opt_def [option -relay] [arg relay]]

	# Specify a relay host for controlling the device.  The
	# default is [arg localhost], which will work for WebRelay
	# devices on the public net.  If the WebRelay device is on a
	# private net then a relay host with access to that net is
	# required.  eg, in the example above with the WebRelay
	# connected to the AP, a suitable relay would be the AP's LAN
	# endpoint.

	# [opt_def [option -invert] [arg {{ports ...}}]]

	# List of ports for which the sense of [method {power off}]
	# and [method {power on}] should be inverted.  [para]

	# Inverted ports are useful for controlling things like dongle
	# power interrupters where you can use the NC connector to
	# maintain constant power and use [method {power cycle}] to
	# briefly interrupt the connection.  With ports used this way
	# you want [method {power off}] to turn the relay on and
	# [method {power on}] to turn the relay off.

	# [opt_def [option -invertcycle] [arg {{ports ...}}]]

	# List of ports for which the sense of [method {power cycle}]
	# should be inverted.  [para]

	# Cycle-inverted ports turn the relay off then on.  Note that
	# ports inverted in this way cannot take advantage of the
	# underlying [cmd pulse] feature so the power cycle will
	# no-longer be atomic.  Care should therefore be taken to make
	# sure the "off" segment will not interrupt connectivity to
	# the relay itself.  This option and vulnerability can always
	# be avoided by using NC instead of NO.

	# [list_end] [para]

	# UTF::Power::WebRelay objects have the following methods:
	# [list_begin definitions]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -lan_ip
    option -relay localhost
    option -invert -type {snit::listtype -type UTF::Power::WebRelay::portnum}
    option -invertcycle \
	-type {snit::listtype -type UTF::Power::WebRelay::portnum}

    typeconstructor {
	snit::integer portnum -min 1 -max 10
	snit::integer wrstate -min 0 -max 2
    }

    constructor {args} {
	$self configurelist $args
	if {$options(-lan_ip) eq ""} {
	    if {$options(-relay) eq "localhost"} {
		set options(-lan_ip) [namespace tail $self]
	    } else {
		set options(-lan_ip) "192.168.1.2"
	    }
	}
    }

    UTF::doc {
	# [call [arg name] [method {power off}] [arg port]]

	# Power off the device controlled by the numbered port, by
	# turning the port's relay off (or on if the port is in the
	# [option -invert] list).
    }

    method {power off} {port} {
	if {[lsearch $options(-invert) $port] >= 0} {
	    $self on $port
	} else {
	    $self off $port
	}
    }

    UTF::doc {
	# [call [arg name] [method {power on}] [arg port]]

	# Power on the device controlled by the numbered port, by
	# turning the port's relay on (or off if the port is in the
	# [option -invert] list).
    }

    method {power on} {port} {
	if {[lsearch $options(-invert) $port] >= 0} {
	    $self off $port
	} else {
	    $self on $port
	}
    }

    UTF::doc {
	# [call [arg name] [method {power cycle}] [arg port]]

	# By default this pulses the numbered port on then off, ie if
	# you are using the NC connection this will briefly open.  If
	# you are using the NO connection this will briefly close.
	# [para]

	# If the [arg port] is in the [option -invertcycle] list, the
	# sense will be inverted and the relay will be turned off then
	# on.
    }

    method {power cycle} {port} {
	if {[lsearch $options(-invertcycle) $port] >= 0} {
	    $self off $port
	    UTF::Sleep 0.5
	    $self on $port
	} else {
	    $self pulse $port
	}
    }

    UTF::doc {
	# [call [arg name] [method off] [arg port]]

	# Low-level method switching the relay to the "off" state,
	# irrespective of the [option -invert] list.  In the "off"
	# state, NC is closed and NO is open.
    }

    method off {port} {
	$self webrelayshell $port 0
    }

    UTF::doc {
	# [call [arg name] [method off] [arg port]]

	# Low-level method switching the relay to the "on" state,
	# irrespective of the [option -invert].  In the "on" state, NC
	# is open and NO is closed.
    }

    method on {port} {
	$self webrelayshell $port 1
    }

    UTF::doc {
	# [call [arg name] [method pulse] [arg port]]

	# Low-level method pulsing the relay, irrespective of the
	# [option -invertcycle] list.  This switches the relay into
	# the "on" state, then the "off" state.
    }

    method pulse {port} {
	$self webrelayshell $port 2
    }

    # Method actually doing the work
    method webrelayshell {port state} {
	UTF::Power::WebRelay::portnum validate $port
	UTF::Power::WebRelay::wrstate validate $state
	$options(-relay) rexec -noinit $UTF::usrutf/webrelayshell \
	    $options(-lan_ip) relay${port}State=$state
	return
    }

    UTF::doc {
	# [list_end]
	# [para]
    }
}

snit::type UTF::Power::PicoPSU {
    UTF::doc {

	# [call [cmd UTF::Power::PicoPSU] [arg name]
	#	[lb][option -lan_ip] [arg address][rb]]

	# Create a new UTF::Power::PicoPSU object.[para]

	# UTF::Power::PicoPSU is an implementation of the UTF Power
	# object using a Controlbyweb WebRelay device, a picoPSU and
	# a relay capable of supporting enough current to power
	# a 4706 router. See [uri http://www.controlbyweb.com/products.html]
	# and [uri http://www.mini-box.com/picoPSU-160-XT]

	# [list_begin options]

	# [opt_def [option -lan_ip] [arg address]]

	# Specify the WebRelay device's hostname or IP address.
	# [para]

	# [list_end] [para]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -lan_ip -default "" -readonly true
    option -port -type snit::integer -default 1 -readonly true
    option -pulse -default 0.250 -readonly true

    variable urlstring_off
    variable urlstring_on
    variable urlstring_pulse
    variable sleepvar
    variable utfmsg_tag

    constructor {args} {
	$self configurelist $args
	if {$options(-lan_ip) eq ""} {
	    set options(-lan_ip) [namespace tail $self]
	}
        set urlstring_off "http://${options(-lan_ip)}/state.xml?relay${options(-port)}State=1"
        set urlstring_on "http://${options(-lan_ip)}/state.xml?relay${options(-port)}State=0"
        set urlstring_pulse "http://${options(-lan_ip)}/state.xml?relay${options(-port)}State=2&pulseTime${options(-port)}=${options(-pulse)}"
	set utfmsg_tag [namespace tail $self]
    }
    method {power cycle} {args} {
       if {![catch {::http::geturl $urlstring_pulse} token]} {
	    UTF::Message INFO $utfmsg_tag "[::http::data $token]"
	    ::http::cleanup $token
	    set sleepvar 0
	    after  [expr {int(1000 * $options(-pulse))}] [list set [myvar sleepvar] 1]
	    vwait [myvar sleepvar]
	} else {
	    error $token
	}
    }
    method {power off} {args} {
        set token [::http::geturl $urlstring_off]
	UTF::Message INFO $utfmsg_tag "[::http::data $token]"
	::http::cleanup $token
    }
    method {power on} {args} {
        set token [::http::geturl $urlstring_on]
	UTF::Message INFO $utfmsg_tag "[::http::data $token]"
	::http::cleanup $token
    }
}

snit::type UTF::Power::AviosysUSB {
    UTF::doc {

	# [call [cmd UTF::AviosysUSB] [arg name]
	#	[lb][option -lan_ip] [arg address][rb]]

	# Create a new UTF::Power::AviosysUSB power object.[para]

	# [list_begin options]

	# [opt_def [option -lan_ip] [arg address]]

	# Specify the WebRelay device's hostname or IP address.
	# [para]

	# [list_end] [para]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -relay localhost
    option -pulse -default 1.0 -readonly true

    variable sleepvar
    variable utfmsg_tag

    constructor {args} {
	$self configurelist $args
	set utfmsg_tag [namespace tail $self]
    }
    method {power cycle} {args} {
	$self power off
	UTF::Sleep $options(-pulse)
	$self power on
    }
    method {power off} {args} {
	$options(-relay) rexec -n /usr/local/bin/p0
    }
    method {power on} {args} {
	$options(-relay) rexec -n /usr/local/bin/p1
    }
}


snit::type UTF::Power::WebSwitch {
    UTF::doc {

	# [call [cmd UTF::Power::WebSwitch] [arg name]
	#	[lb][option -lan_ip] [arg address][rb]
	# 	[lb][option -relay] [arg relay][rb]]

	# Create a new UTF::Power::WebSwitch object.[para]

	# UTF::Power::WebSwitch is an implementation of the UTF Power
	# object using a Controlbyweb WebSwitch device.
	# See [uri http://www.controlbyweb.com/products.html] [para]

	# WARNING: The WebSwitch defaults to having both ports OFF
	# when the WebSwitch starts up.  This is probably not what you
	# want.  This can be configured via the WebSwitch's setup.html

	# [list_begin options]

	# [opt_def [option -lan_ip] [arg address]]

	# Specify the WebSwitch device's hostname or IP address.  If
	# [option -relay] is specified then this defaults to
	# 192.168.1.2, which is the factory default for the WebSwitch
	# device and allows it to be used unmodified if you connect it
	# to a Broadcom Router in your test rig (use caution to avoid
	# deadlocks if using the relay to power switch the same
	# router!). [para]

	# If [option -relay] is not specified then [option -lan_ip]
	# defaults to the [arg name] of the device, similar to other
	# UTF objects.

	# [opt_def [option -relay] [arg relay]]

	# Specify a relay host for controlling the device.  The
	# default is [arg localhost], which will work for WebSwitch
	# devices on the public net.  If the WebSwitch device is on a
	# private net then a relay host with access to that net is
	# required.  eg, in the example above with the WebSwitch
	# connected to the AP, a suitable relay would be the AP's LAN
	# endpoint.

	# [opt_def [option -controlpasswd] [arg passwd]]

	# Specify a password for the control web page.  Default is
	# "webswitch" but this is not checked unless control password
	# protection is enabled on the Switch.

	# [opt_def [option -setuppasswd] [arg passwd]]

	# Specify a password for the setup web page.  Default is "webswitch".

	# [list_end]
	# [para]
	# UTF::Power::WebSwitch objects have the following methods:
	# [list_begin definitions]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -lan_ip
    option -relay localhost
    option -controlpasswd "webswitch"
    option -setuppasswd "webswitch"

    typeconstructor {
	snit::integer portnum -min 1 -max 2
    }

    constructor {args} {
	$self configurelist $args
	if {$options(-lan_ip) eq ""} {
	    if {$options(-relay) eq "localhost"} {
		set options(-lan_ip) [namespace tail $self]
	    } else {
		set options(-lan_ip) "192.168.1.2"
	    }
	}
    }

    UTF::doc {
	# [call [arg name] [method {power off}] [arg port]]

	# Power off the numbered port.
    }

    method {power off} {port} {
	portnum validate $port
	# second field is off
	set fieldname [expr {2+($port-1)*4}]
	$self Status $port \
	    [$options(-relay) rexec -s -noinit wget -nv -O- \
		 --http-user= --http-passwd=$options(-controlpasswd) \
		 http://$options(-lan_ip)/index.srv?$fieldname=Off]
    }

    UTF::doc {
	# [call [arg name] [method {power on}] [arg port]]

	# Power on the numbered port.
    }

    method {power on} {port} {
	portnum validate $port
	# first field is on
	set fieldname [expr {1+($port-1)*4}]
	$self Status $port \
	    [$options(-relay) rexec -s -noinit wget -nv -O- \
		 --http-user= --http-passwd=$options(-controlpasswd) \
		 http://$options(-lan_ip)/index.srv?$fieldname=On]
    }

    UTF::doc {
	# [call [arg name] [method {power cycle}] [arg port]]

	# Power the numbered port off and then on.
    }

    method {power cycle} {port} {
	portnum validate $port
	# third field is reboot
	set fieldname [expr {3+($port-1)*4}]
	$self Status $port \
	    [$options(-relay) rexec -s -noinit wget -nv -O- \
		 --http-user= --http-passwd=$options(-controlpasswd) \
		 http://$options(-lan_ip)/index.srv?$fieldname=Reboot]
    }

    method Status {port {reply ""}} {
	portnum validate $port
	if {$reply eq ""} {
	    set reply [$options(-relay) rexec -s -noinit wget -nv -O- \
			   --http-user= --http-passwd=$options(-controlpasswd) \
			   http://$options(-lan_ip)/index.srv]
	}
	set fieldname [expr {1+($port-1)*4}]
	if {[regexp ">(\\w+)</td><td id=c><input type=submit name=$fieldname " $reply - status]} {
	    return $status
	} else {
	    error "status not found: $reply"
	}
    }

    UTF::doc {
	# [call [arg name] [method setup]]

	# Configure both ports to be powered on at start up.  Other
	# options are available via the web page, but this should be
	# the most likely setting needed for UTF.
    }

    method setup {} {
	$options(-relay) rexec -s -noinit wget -nv -O- \
	    --http-user= --http-passwd=$options(-setuppasswd) \
	    http://$options(-lan_ip)/out1.srv?outDefOn=yes
	$options(-relay) rexec -s -noinit wget -nv -O- \
	    --http-user= --http-passwd=$options(-setuppasswd) \
	    http://$options(-lan_ip)/out2.srv?outDefOn=yes
	return
    }

    UTF::doc {
	# [list_end]
	# [para]
    }
}

snit::type UTF::Power::DLI {
    UTF::doc {

	# [call [cmd UTF::Power::DLI] [arg name]
	#	[lb][option -lan_ip] [arg address][rb]
	# 	[lb][option -relay] [arg relay][rb]
	#       [lb][option -user] [arg user][rb]
	#       [lb][option -passwd] [arg password][rb]]

	# Create a new UTF::Power::DLI object.[para]

	# UTF::Power::DLI is an implementation of the UTF Power object
	# using a Digital Loggers Inc Ethernet Power Controller.

	# [list_begin options]

	# [opt_def [option -lan_ip] [arg address]]

	# Specify the DLI device's hostname or IP address.  [option
	# -lan_ip] defaults to the [arg name] of the device.

	# [opt_def [option -relay] [arg relay]]

	# Specify a relay host for controlling the device.  The
	# default is [arg localhost].

	# [opt_def [option -passwd] [arg passwd]]

	# Specify a password for the outlet control web page.

	# [list_end]
	# [para]
	# UTF::Power::DLI objects have the following methods:
	# [list_begin definitions]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -lan_ip
    option -relay localhost
    option -user "root"
    option -passwd "xxxxx"

    constructor {args} {
	$self configurelist $args
	if {$options(-lan_ip) eq ""} {
	    set options(-lan_ip) [namespace tail $self]
	}
    }

    UTF::doc {
	# [call [arg name] [method {power off}] [arg port]]

	# Power off the numbered port.
    }
    method {power off} {port} {
	$self outlet $port off
    }

    UTF::doc {
	# [call [arg name] [method {power on}] [arg port]]

	# Power on the numbered port.
    }
    method {power on} {port} {
	$self outlet $port on
    }

    UTF::doc {
	# [call [arg name] [method {power cycle}] [arg port]]

	# Power the numbered port off and then on.
    }
    method {power cycle} {port} {
	$self outlet $port pulse
    }

    UTF::doc {
	# [call [arg name] [method status] [lb][arg port][rb]]

	# Query switch status
    }
    method status {{port ""}} {
	$self outlet $port status
    }

    method outlet {port op} {
	$options(-relay) rexec -n $UTF::usrutf/bin/DLI.pl $options(-lan_ip) \
	    "$options(-user):$options(-passwd)" $port$op
	return
    }

    UTF::doc {
	# [list_end]
	# [para]
    }
}

snit::type UTF::Power::Blackbox {
    UTF::doc {

	# [call [cmd UTF::Power::Blackbox] [arg name]
	#	[lb][option -lan_ip] [arg address][rb]
	# 	[lb][option -relay] [arg relay][rb]
	#	[lb][option -user] [arg user][rb]
	#	[lb][option -passwd] [arg passwd][rb]]

	# Create a new UTF::Power::Blackbox object.[para]

	# UTF::Power::Blackbox is an implementation of the UTF Power
	# object using Blackbox PSE506 or similar devices, accessed
	# via http.

	# [list_begin options]

	# [opt_def [option -lan_ip] [arg address]]

	# Specify the Blackbox device's hostname or IP address.  The
	# default is to use the [arg name] of the device.

	# [opt_def [option -relay] [arg relay]]

	# Specify a relay host for controlling the device.  The
	# default is [arg localhost], which will work for Blackbox
	# devices on the public net.  If the Blackbox device is on a
	# private net then a relay host with access to that net is
	# required.

	# [opt_def [option -user] [arg username]]

	# Specify username for access to the Blackbox web page.
	# Defaults to "admin".

	# [opt_def [option -passwd] [arg password]]

	# Specify password for access to the Blackbox web page.
	# Defaults to "admin".

	# [list_end]
	# [para]
	# UTF::Power::Blackbox objects have the following methods:
	# [list_begin definitions]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -user "admin"
    option -passwd "admin"

    option -lan_ip ""
    option -relay localhost

    typeconstructor {
	snit::integer portnum -min 1 -max 4
    }

    constructor {args} {
	$self configurelist $args
	if {$options(-lan_ip) eq ""} {
	    set options(-lan_ip) [namespace tail $self]
	}
    }

    UTF::doc {
	# [call [arg name] [method {power off}] [arg port]]

	# Power the numbered port off.
    }

    method {power off} {port} {
	$self outlet $port 0
    }

    UTF::doc {
	# [call [arg name] [method {power on}] [arg port]]

	# Power the numbered port on.
    }

    method {power on} {port} {
	$self outlet $port 1
    }

    UTF::doc {
	# [call [arg name] [method {power cycle}] [arg port]]

	# Power the numbered port off and then on.
    }

    method {power cycle} {port} {
	$self outlet $port r
    }

    UTF::doc {
	# [call [arg name] [method status] [arg port]]

	# Query the status of the numbered port.
    }
    method status {port} {
	$self outlet $port g
    }

    UTF::doc {
	# [call [arg name] [method toggle] [arg port]]

	# Toggle the on/off state of the numbered port.
    }
    method toggle {port} {
	$self outlet $port t
    }

    UTF::doc {
	# [call [arg name] [method outlet] [arg port] [arg op]]

	# Perform the requested operation on the numbered port.
	# op can be:	'0' for off, '1' for on, 'r' for cycle,
	# 		't' for toggle and 'g' for get (status)
	# returns: 	'0' (off), '1' (on) or '2' (restarting)
    }
    method outlet {port op} {
	portnum validate $port
	set commandstr {$options(-relay) rexec -s -noinit	\
	    wget -nv -O-					\
	    --http-user=$options(-user) 			\
	    --http-passwd=$options(-passwd) 			\
	    --post-data=p$port=$op 				\
	    http://$options(-lan_ip)/config/home_f.html}
	if {[catch $commandstr outputmsg]} {
	    if {[regexp {Authorization failed} $outputmsg]} {
		UTF::Message LOG $self "BlackBox auth failure, retrying once"
		set outputmsg [eval $commandstr]
	    } else {
		error $outputmsg
	    }
	}
	if {![regexp -line "socket.$port.*" $outputmsg portline ]} {
	    error "Port status not found: $outputmsg"
	}
	if {![regexp {[^,]*,\"([^,]*)",([^,]*),} $portline match \
	    portname portstate ]} {
	    error "Port status cannot be processed: $portline"
	}
	set portname [string trim $portname]
	UTF::Message LOG $self "State of port $port\(\"$portname\"\) is \
	    now [lindex [list "off" "on" "restarting"] $portstate]"
	return $portstate
    }
}

snit::type UTF::Power::X10 {
    UTF::doc {

	# [call [cmd UTF::Power::X10] [arg name]
	# 	[lb][option -relay] [arg relay][rb]]

	# Create a new UTF::Power::X10 object.[para]

	# UTF::Power::X10 is an implementation of the UTF Power object
	# using an X10 CM11A or compatible products from IBM and
	# RadioShack.  Requires the heyu2 package to be installed on
	# the relay.  See [uri http://www.x10.com/products.html], [uri
	# http://tanj.com/heyu/heyu2]

	# [list_begin options]

	# [opt_def [option -relay] [arg relay]]

	# Specify a relay host.  The relay host is the host with the
	# physical connection to CM11A control module.  The heyu2
	# software should be installed on the relay.  The default is
	# [arg localhost].

	# [list_end]
	# [para]
	# UTF::Power::X10 objects have the following methods:
	# [list_begin definitions]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -relay localhost

    typeconstructor {
	snit::stringtype housecodeunit -regexp {^[A-P](\d|1[0-6])$}
    }


    UTF::doc {
	# [call [arg name] [method {power off}] [arg HU]]

	# Power off the numbered Housecode/Unit.
    }

    method {power off} {HU} {
	housecodeunit validate $HU
	$options(-relay) rexec -noinit heyu turn $HU off
    }

    UTF::doc {
	# [call [arg name] [method {power on}] [arg HU]]

	# Power on the numbered Housecode/Unit.
    }

    method {power on} {HU} {
	housecodeunit validate $HU
	$options(-relay) rexec -noinit heyu turn $HU on
    }

    UTF::doc {
	# [call [arg name] [method {power cycle}] [arg HU]]

	# Power the numbered Housecode/Unit off and then on.
    }

    method {power cycle} {HU} {
	housecodeunit validate $HU
	$self power off $HU
	UTF::Sleep 1
	$self power on $HU
    }

    UTF::doc {
	# [list_end]
	# [para]
    }
}

snit::type UTF::Power::Invert {
    UTF::doc {

	# [call [cmd UTF::Power::Invert] [arg name]
	#	[option -power] [arg {button args}]]

	# Create a new UTF::Power::Invert object.[para]

	# UTF::Power::Invert is a UTF Power object intended for
	# inverting the off/on sense for other UTF::Power objects.

	# The Invert object only supports a single port, so the [arg
	# port] arg is not specified on the [method power] methods.
	# To support multiple inverted ports, create multiple Invert
	# objects.

	# [list_begin options]

	# [opt_def [option -power] [arg {object args}]]

	# [arg power] is the parent UTF::Power object describing a
	# hardware remote-control power switch to be inverted.  [arg
	# args] is used to supply additional arguments to the power
	# methods, usually a port number on the switch.  Option
	# [option -power] is mandatory.
	# [list_end] [para]
	# UTF::Power::Invert objects have the following methods:
	# [list_begin definitions]
    }

    variable power ""
    variable power_args ""

    option -power -configuremethod _powerargs
    option -delay -type snit::double -default 1

    constructor {args} {
	$self configurelist $args
	if {$power eq ""} {
	    error "$self: Invert power switch needs -power to invert"
	}
    }

    # Internal method for splitting power objects and their arguments
    method _powerargs {key val} {
	# strip leading -
	regsub {^-} $key {} key
	set $key [lindex $val 0]
	set ${key}_args [lreplace $val 0 0]
    }

    UTF::doc {
	# [call [arg name] [method {power off}]]

	# Power off the device, by calling the parent's power on
	# method.
    }

    method {power off} {} {
	$power power on $power_args
    }

    UTF::doc {
	# [call [arg name] [method {power on}]]

	# Power on the device, by calling the parent's power off
	# method.
    }

    method {power on} {} {
	$power power off $power_args
    }

    UTF::doc {
	# [call [arg name] [method {power cycle}]]

	# Power cycle the device, by calling the power off and power
	# on methods.
    }

    method {power cycle} {} {
	$self power off
	UTF::Sleep $options(-delay)
	$self power on
    }

    UTF::doc {
	# [list_end]
	# [para]
    }
}

snit::type UTF::Power::Laptop {
    UTF::doc {

	# [call [cmd UTF::Power::Laptop] [arg name]
	#	[option -button] [arg {button args}]
	# 	[lb][option -supply] [arg {supply args}][rb]]

	# Create a new UTF::Power::Laptop object.[para]

	# UTF::Power::Laptop is a UTF Power object intended for
	# power controlling Laptops and other similar devices which
	# use a single power button to power on and off the device.

	# This is a Meta object, where the button and optionally the
	# power supply are actually controlled via additional
	# hardware-specific Power objects. [para]

	# The Laptop object only supports a single port, so the [arg
	# port] arg is not specified on the [method power] methods.
	# To support multiple Laptops, create multiple Laptop objects.

	# [list_begin options]

	# [opt_def [option -button] [arg {button args}]]

	# [arg button] is a UTF::Power object describing a hardware
	# remote-control power switch that will be used to control the
	# Laptop's power button.  [arg args] is used to supply
	# additional arguments to the power methods, usually a port
	# number on the switch.  Option [option -button] is mandatory.

	# [opt_def [option -supply] [arg {supply args}]]

	# [arg supply] is a UTF::Power object describing a hardware
	# remote-control power switch that will be used to control the
	# Laptop's power supply.  [arg args] is used to supply
	# additional arguments to the power methods, usually a port
	# number on the switch.  Option [option -supply] is optional,
	# but recommended (see below).

	# [list_end]
	# [para]
	# UTF::Power::Laptop objects have the following methods:
	# [list_begin definitions]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    variable supply ""
    variable supply_args ""
    variable button ""
    variable button_args ""

    option -supply -configuremethod _powerargs
    option -button -configuremethod _powerargs

    constructor {args} {
	$self configurelist $args
	if {$button eq ""} {
	    error "$self: Laptop power switch has no -button defined"
	}
    }

    # Internal method for splitting power objects and their arguments
    method _powerargs {key val} {
	# strip leading -
	regsub {^-} $key {} key
	set $key [lindex $val 0]
	set ${key}_args [lreplace $val 0 0]
    }

    UTF::doc {
	# [call [arg name] [method {power off}]]

	# Power off the device.  If [option -supply] is available, the
	# device will be powered off by cutting the power supply.
	# This is the most reliable way to power off the device.

	# If [option -supply] is not available, The object will
	# attempt to power off the device by pushing and holding the
	# power button for several seconds.  This is not as reliable
	# on some platforms.  For example, if a Mac is already powered
	# off then pushing and holding the power button will actually
	# turn it on!
    }

    method {power off} {} {
	if {$supply ne ""} {
	    # If we have a power supply, use it
	    $supply power off $supply_args
	} else {
	    # Otherwise hold the button in
	    $button power on $button_args
	    UTF::Sleep 10
	    $button power off $button_args
	}
    }

    UTF::doc {
	# [call [arg name] [method {power on}]]

	# Power on the device by briefly pushing the power button.  If
	# [option -supply] is available, the power supply will first be
	# turned on. [para]

	# WARNING: If the system is already powered on this may
	# trigger a popup asking to power down, or even power down
	# immediately, depending on the system configuration.
    }

    method {power on} {} {
	if {$supply ne ""} {
	    # If we have a power supply, turn it on
	    $supply power on $supply_args
	    # Make sure laptop actually has power before pushing the
	    # button
	    UTF::Sleep 5
	}
	# Pulse the button to wake up the device
	$button power on $button_args
	UTF::Sleep 1
	$button power off $button_args
    }

    UTF::doc {
	# [call [arg name] [method {power cycle}]]

	# Power cycle the device by powering off and on using the
	# above methods.  If [option -supply] is available, this
	# should be reliable.
    }

    method {power cycle} {} {
	$self power off
	UTF:::Sleep 5
	$self power on
    }

    UTF::doc {
	# [list_end]
	# [para]
    }
}

snit::type UTF::Power::Agilent {
    UTF::doc {
        # [call [cmd UTF::Power::Agilent] [arg host]
        # [option -name] [arg name]
        # [lb][option -model] [arg string][rb]
        # [lb][option -voltage] [arg voltage][rb]
        # [lb][option -script_path] [arg path][rb]]

        # Create a new Agilent power controller object.
        # [list_begin options]

        # [opt_def [option -name] [arg name]]
    
        # Name of the power controller object.

        # [opt_def [option -model] [arg string]]
    
        # Model number of the Agilent power controller, can be
        # E3644A or 66321D, default=66321D. The E3644A is supported
        # only via a serial port and an associated consolelogger process,
        # while the 66321D is only supported via GPIB to a Linux host.

        # [opt_def [option -voltage] [arg voltage]]
    
        # Default "ON" voltage for the power controller=3.3

        # [opt_def [option -script_path] [arg path]]
    
        # Location of the power controller scripts on the host
        # PC, only relevant for model 66321D, default: /root/Agilent

        # [para]
        # [list_end]

        # Unrecognised options will be passed on to the host's [cmd
        # UTF::Linux] object. Only relevant for model 66321D.

        # [list_end]

        # [para]
        # Agilent objects have the following methods:
        # [para]
        # [list_begin definitions]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -name -configuremethod CopyOption ;# MUST HAVE!
    option -model 66321D
    option -voltage 3.3
    option -script_path "/root/Agilent"
    option -channel 0

    variable max_ports ;# set by constructor
	variable n6700_host ""
	variable n6700_port "5024"
	variable n6700_sockfd "-1"
	variable n6700_statesockfd ""
	variable n6700_read_socket_timeout_ms 0
	variable n6700_cmd_start_ms 0
	variable n6700_response ""

    # NB: DO NOT add option -lan_ip here! It will elclipse the Base.tcl
    # option -lan_ip and really mess up rexec functionality!
    
    # For model 66321D, need Linux base for rexec.
    # Not needed for model E3644A and N6700B.       
    # base handles any other options and methods
    component base -inherit yes

    constructor {args} {
        set cloneopts $args
        # For model 66321D, need Linux base for rexec.
        # Not needed for model E3644A and N6700B.       
        if {$options(-model) == "66321D"} {
           install base using UTF::Linux %AUTO%
        }
        $self configurelist $args
        if {$options(-name) eq ""} {
	       $self configure -name [namespace tail $self]
        }
        if {$options(-model) == "E3644A" || $options(-model) == "66321D"} {
            set max_ports 1
        } elseif {$options(-model) == "N6700B"} {
            set max_ports 2
        } else {
            error "ERROR Power.tcl Agilent constructor self=$self unknown model=$options(-model)"
        }
        # puts "Power.tcl Agilent constructor self=$self args=$args"
    }

    destructor {
        catch {$base destroy}
    }

    # Internal method for copying options to delegates
    # Cant delegate this to base component!
    method CopyOption {key val} {
        set options($key) $val
        $base configure $key $val
    }

    UTF::doc {
        # [call [arg name] [method power] [arg op] [arg port] [arg voltage]]

        # The power control function specified by [arg op] for [arg port] is 
        # implemented by running a script on the associated Linux host
        # [arg name].[para]

        # [arg op] can be: on off cycle [para]

        # [arg port] usually 1, as the Agilent E3644A & 66321D have only one output
        # connector each.[para]

        # [opt voltage] defaults to the value in option -voltage
    }

    method power {op port {voltage ""}} {
        # Ensure a valid value for voltage.
        set voltage [string trim $voltage]
        if {$voltage == "" } {
            set voltage $options(-voltage)
        }
        set voltage [string trim $voltage]
        if {$voltage == "" } {
            set voltage 3.3
        }

        UTF::Message LOG "$options(-name)" "Power.tcl Agilent power op=$op\
            port=$port voltage=$voltage lan_ip=[$self cget -lan_ip]"
        # Check port range.
        if {![regexp {^\d+$} $port] || $port < 0 || $port > $max_ports} {
            error "Agilent power ERROR: self=$self invalid port=$port, Agilent\
            $options(-model) have max_ports=$max_ports "
        }

        regsub -all -nocase {v} $voltage "" voltage ;# remove V,v
        if {![regexp {^[\d\.]+$} $voltage]} {
            error "Agilent power ERROR: self=$self invalid voltage=$voltage"
        } 

        # Process power op.
        set op [string trim $op]
        set op [string tolower $op]
        if {$op == "on"} {
            # Turn Agilent on to specified voltage.
            $self set_voltage $port $voltage
        } elseif {$op == "off"} {
            # Turn Agilent off
            $self set_voltage $port 0
        } elseif {$op == "cycle"} {
            # Turn Agilent off then on to specified voltage.
            $self set_voltage $port 0
            UTF::Sleep 1
            $self set_voltage $port $voltage
        } else {
            error "Agilent power ERROR: self=$self port=$port invalid op=$op,\
                should be: on off cycle"
        }
        return
    }

    # Method runs a script to set voltage. Script used depends on the 
    # Agilent model number.
    method set_voltage {port voltage} {
		UTF::Message INFO "" "========================Func:set_voltage $port $voltage==========================="
        set model $options(-model)
        # puts "Power.tcl Agilent set_voltage self=$self model=$model port=$port voltage=$voltage"

        # Use E3644A specific commands.
        if {$options(-model) == "E3644A"} {
            set lan_ip [$self cget -lan_ip] ;# ip & port of the Agilent consolelogger
            if {$voltage == "0" || $voltage == "0." || $voltage == "0.0"} {
                # Send commands to turn off output and query output state.
                set resp [exec $UTF::unittest/agshell $lan_ip "\"outp off\;outp\?\""]
                set resp "Output turned off: $resp"
            } else {
                # Send commands to turn set desired voltage, turn on output and query desired voltage.
                # NB: desired voltage is NOT the actual voltage, need meas:volt? for that.
                set resp [exec $UTF::unittest/agshell $lan_ip "\"volt $voltage\;outp on\;volt\?\""]
                set resp "Output turned on, Voltage set to: $resp"
            }
            UTF::Message LOG "$options(-name)" "$resp"
        } elseif {$options(-model) == "N6700B"} {
			$self __n6700_socket_connect
			set rc [$self __n6700_set_voltage $options(-voltage)]
			return $rc
        } else {
			# Run a script on remote Linux host for 66321D.
			set script $options(-script_path)/set_voltcurrent_$model.sh
			$self rexec $script $voltage
		}
		
        return
    }

    UTF::doc {
        # [call [arg name] [method setup_current_trigger] [arg port]
        # [arg SampleTime] [arg SamplePoints] [arg OffsetPoints]
        # [arg CurrentRange] [arg CurrentTrigger]]

        # Sets up the current trigger on the power controller [arg port]
        # on Linux host [arg name]. This is done as a parallel process
        # to allow the actual DUT test to be started up after the current 
        # trigger was setup.[para]

        # [arg port] usually 1, as the Agilent 66321D & E3644A have only one 
        # output connector each.[para]

        # The 66321D has a high speed 4K sample capture buffer with triggers.
        # The E3644A has a trigger, but no capture buffer. For the E3644A,
        # we simply measure the current repeatedly, as fast as we can, and
        # get about 5 samples per second. Other arguments are passed to
        # script get_current_<model>.sh.[para]

        # Typical values are: 78E-6 4096 -100 1.0 0.063 [para]

        # Returns: a file desciptor
    }

    # Method runs script to setup the current trigger.
    method setup_current_trigger {port SampleTime SamplePoints OffsetPoints\
        CurrentRange CurrentTrigger} {
		UTF::Message INFO "" "========================Func:setup_current_trigger==========================="
        UTF::Message LOG "$options(-name)" "Power.tcl Agilent\
            setup_current_trigger self=$self port=$port\
            SampleTime=$SampleTime SamplePoints=$SamplePoints\
            OffsetPoints=$OffsetPoints CurrentRange=$CurrentRange\
            CurrentTrigger=$CurrentTrigger"

        # Check port range.
        if {![regexp {^\d+$} $port] || $port < 0 || $port > $max_ports} {
            error "Agilent setup_current_trigger ERROR: self=$self invalid\
            port=$port, Agilent $options(-model) have max_ports=$max_ports "
        }

        # Script used depends on model. Spawn a parallel process.
        set model $options(-model)
        if {$options(-model) == "E3644A"} {
            # Script runs on local host. Script has 3 calling parameters to
            # tell it where to find agshell, ip:port to go to & sample_cnt.
            set script $UTF::unittest/bin/get_current_$model.sh
            set lan_ip [$self cget -lan_ip]
            set fd [open "|tclsh $script $UTF::unittest $lan_ip $SamplePoints" r]
        } elseif {$options(-model) == "N6700B"} {
			$self __n6700_socket_connect
			set actual_sample_time [$self __n6700_setup_current_trigger $SampleTime $SamplePoints $OffsetPoints $CurrentTrigger]
			# no fd for this instrument
			set fd "$actual_sample_time"
        } else {
            # Script is sent to Agilent GPIB Linux host.
            set script $options(-script_path)/get_current_$model.sh
            set fd [$self rpopen $script $SampleTime $SamplePoints $OffsetPoints\
                $CurrentRange $CurrentTrigger]
        }
        UTF::Message LOG "$options(-name)" "setup_current_trigger\
            created parallel process fd=$fd"
        return $fd
    }

    UTF::doc {
        # [call [arg name] [method invoke_current_trigger] [arg port]]

        # invoke the instrument to measure current
        # [arg port] on Linux host [arg name]. This is NOT a triggered
        # event.[para]

        # [arg port] usually 1

        # Returns: PASS 0 or FAIL -1.
    }

    # Method runs script to measure the instrument current reading.
    method invoke_current_trigger {port} {
		UTF::Message INFO "" "========================Func:invoke_current_trigger $port==========================="
        UTF::Message LOG "$options(-name)" "Power.tcl Agilent invoke_current_trigger\
            self=$self port=$port"
        # Check port range.
        if {![regexp {^\d+$} $port] || $port < 0 || $port > $max_ports} {
            error "Agilent invoke_current_trigger ERROR: self=$self invalid port=$port, Agilent\
            $options(-model) have max_ports=$max_ports "
        }

        if {$options(-model) == "N6700B"} {
			$self __n6700_socket_connect
			set rc [$self __n6700_invoke_current_trigger]
			return $rc
        } else {
			error "ERROR: Agilent model $options(-model) does not support invoke_current_trigger"
        }
    }

    UTF::doc {
        # [call [arg name] [method force_current_trigger] [arg port]]

        # force the instrument to measure current
        # [arg port] on Linux host [arg name]. This is NOT a triggered
        # event.[para]

        # [arg port] usually 1

        # Returns: PASS 0 or FAIL -1.
    }

    # Method runs script to measure the instrument current reading.
    method force_current_trigger {port} {
		UTF::Message INFO "" "========================Func:force_current_trigger $port==========================="
        UTF::Message LOG "$options(-name)" "Power.tcl Agilent force_current_trigger\
            self=$self port=$port"
        # Check port range.
        if {![regexp {^\d+$} $port] || $port < 0 || $port > $max_ports} {
            error "Agilent force_current_trigger ERROR: self=$self invalid port=$port, Agilent\
            $options(-model) have max_ports=$max_ports "
        }

        if {$options(-model) == "N6700B"} {
			$self __n6700_socket_connect
			set rc [$self __n6700_force_current_trigger]
			return $rc
        } else {
			error "ERROR: Agilent model $options(-model) does not support force_current_trigger"
        }
    }

    UTF::doc {
        # [call [arg name] [method get_current_trigger_data_high_value]
        # [arg file_descriptor]]

        # Gets the current trigger data high value after the DUT test is 
        # completed. This method assumes that method
        # setup_current_trigger has already been run and a valid
        # [arg file_descriptor] is available for reading.[para]

        # Returns: current high value
    }

    method get_current_trigger_data_high_value {file_descriptor} {

		UTF::Message INFO "" "========================Func:get_current_trigger_data_high_value==========================="
        if {$options(-model) == "N6700B"} {
			$self __n6700_socket_connect
			set current_high_value [$self __n6700_get_current_trigger_data_high_value]
            return $current_high_value
        } else {
            error "ERROR: get_current_trigger_data_high_value is not supported for $options(-model) instrument"
        }
    }

    UTF::doc {
        # [call [arg name] [method get_current_trigger_data_low_value]
        # [arg file_descriptor]]

        # Gets the current trigger data low value after the DUT test is 
        # completed. This method assumes that method
        # setup_current_trigger has already been run and a valid
        # [arg file_descriptor] is available for reading.[para]

        # Returns: current low value
    }

    method get_current_trigger_data_low_value {file_descriptor} {

		UTF::Message INFO "" "========================Func:get_current_trigger_data_low_value==========================="
        if {$options(-model) == "N6700B"} {
			$self __n6700_socket_connect
			set current_low_value [$self __n6700_get_current_trigger_data_low_value]
            return $current_low_value
        } else {
            error "ERROR: get_current_trigger_data_low_value is not supported for $options(-model) instrument"
        }
    }

    UTF::doc {
        # [call [arg name] [method get_current_trigger_data]
        # [arg file_descriptor]]

        # Gets the current trigger data after the DUT test is 
        # completed. This method assumes that method
        # setup_current_trigger has already been run and a valid
        # [arg file_descriptor] is available for reading.[para]

        # Returns: A comma separated datastring, values are usually
        # in scientific notation:
        # data=value1, value2, ... , valueN,
    }

    method get_current_trigger_data {file_descriptor} {

		UTF::Message INFO "" "========================Func:get_current_trigger_data==========================="
        if {$options(-model) == "N6700B"} {
			$self __n6700_socket_connect
			set current_array [$self __n6700_get_current_trigger_data]
            return $current_array
        }

        # Collect data from existing file descriptor
        package require UTF::utils
        set resp1 ""
        set resp2 ""
        UTF::collect_rpopen_data 5 "$self $file_descriptor" resp1 "" resp2

        # Parse out the desired data string
        puts "\n\n\n\n\n\n\n"
        set temp [lindex $resp1 end] ;# grab last item
        if {[regexp -nocase {time=\d+\s+ms\s(.*)} $temp - result]} {
            return $result
        } else {
            error "ERROR: get_current_trigger_data did not find desired data in: $temp"
        }
    }

    UTF::doc {
        # [call [arg name] [method measure_current] [arg port]]

        # Measures the instantaneous current from the power controller
        # [arg port] on Linux host [arg name]. This is NOT a triggered
        # event.[para]

        # [arg port] usually 1, as the Agilent 66321D & E3644A have only one output
        # connector each.[para]

        # Returns: A single value in Amps, usually in scientific notation.
    }

    # Method runs script to measure the instantaneous current reading.
    method measure_current {port} {
		UTF::Message INFO "" "========================Func:measure_current $port==========================="
        UTF::Message LOG "$options(-name)" "Power.tcl Agilent measure_current\
            self=$self port=$port"
        # Check port range.
        if {![regexp {^\d+$} $port] || $port < 0 || $port > $max_ports} {
            error "Agilent measure_current ERROR: self=$self invalid port=$port, Agilent\
            $options(-model) have max_ports=$max_ports "
        }

        # Run the appropriate script.
        if {$options(-model) == "E3644A"} {
            set lan_ip [$self cget -lan_ip] ;# ip & port of the Agilent consolelogger
            set current [exec $UTF::unittest/agshell $lan_ip "meas:curr\?"]
            UTF::Message LOG "$options(-name)" "$current"
            return $current
        } elseif {$options(-model) == "N6700B"} {
			$self __n6700_socket_connect
			set current [$self __n6700_measure_current]
            return $current
        } else {
			set script $options(-script_path)/measure_current_$options(-model).sh
			set data [$self rexec $script]

			# Parse out the current
			if {[regexp -nocase {current=(\S+)} $data - current]} {
				return $current
			} else {
				error "ERROR: measure_current could not parse out current in: $data"
			}
        }
    }

    UTF::doc {
        # [call [arg name] [method measure_voltage] [arg port]]

        # Measures the instantaneous voltage from the power controller
        # [arg port] on Linux host [arg name]. This is NOT a triggered
        # event.[para]

        # [arg port] usually 1, as the Agilent 66321D & E3644A has only one output
        # connector.[para]

        # Returns: A single value in Volts, usually in scientific notation.
    }

    # Method runs script to measure the instantaneous voltage reading.
    method measure_voltage {port} {
		UTF::Message INFO "" "========================Func:measure_voltage $port==========================="
        UTF::Message LOG "$options(-name)" "Power.tcl Agilent measure_voltage\
            self=$self port=$port"
        # Check port range.
        if {![regexp {^\d+$} $port] || $port < 0 || $port > $max_ports} {
            error "Agilent measure_voltage ERROR: self=$self invalid port=$port, Agilent\
            $options(-model) have max_ports=$max_ports "
        }

        # Run the appropriate script
       if {$options(-model) == "E3644A"} {
            set lan_ip [$self cget -lan_ip] ;# ip & port of the Agilent consolelogger
            set voltage [exec $UTF::unittest/agshell $lan_ip "meas:volt\?"]
            UTF::Message LOG "$options(-name)" "$voltage"
            return $voltage
        } elseif {$options(-model) == "N6700B"} {
			$self __n6700_socket_connect
			set voltage [$self __n6700_measure_voltage]
            return $voltage
        } else {
			set script $options(-script_path)/measure_voltage_$options(-model).sh
			set data [$self rexec $script]

			# Parse out the voltage
			if {[regexp -nocase {voltage=(\S+)} $data - voltage]} {
				return $voltage
			} else {
				error "ERROR: measure_voltage could not parse out voltage in: $data"
			}
        }
    }

    UTF::doc {
        # [call [arg name] [method setup]]

        # Runs the Linux setup method and then installs the power controller
        # scripts on host [arg name].[para]

        # setup applies only to the 66321D. For the E3644A, setup is
        # done manually by setting up a consolelogger process for the Agilent
        # serial port. For more info, type: agshell -man
        # No setup for Agilent N6700B is needed.
    }

    # Method runs script to get current consumption.
    method setup {} {
        set lan_ip [$self cget -lan_ip]
        # puts "Power.tcl Agilent setup self=$self lan_ip=$lan_ip"

        # Setup does not apply to the E3644A or N6700B
        if {$options(-model) == "E3644A" || $options(-model) == "N6700B"} {
            UTF::Message LOG "$options(-name)" "Agilent $options(-model) setup is\
                done manually by setting up a consolelogger process for the Agilent\
                serial port. For more info, type: agshell -man"
            return 
        }

        # Run the Linux setup method first.
        eval $base setup

        # NB: The power controler host have a non-standard setup.
        # If root cant mv files in the directories below, then 
        # turn off the daemons for ypbind & autofs as shown below:
        # type: chkconfig --level 0123456 ypbind off
        # type: chkconfig --level 0123456 autofs off
        # type: chkconfig --list ypbind
        # type: chkconfig --list autofs
        # then reboot the PC
        # Then load the standard dopostinstall-fN.sh & postinstall.sh
        # scripts to /root, run them and reboot again.
        # If /usr/local/lib is a symlink to another server, remove the
        # symlink. Then you can write to the local directory.

        # Setup destination directory
        UTF::Message LOG "$options(-name)" "Setting up Agilent specific scripts"
        set dest_dir1 "$options(-script_path)"
        $self rexec mkdir -p $dest_dir1

        # Get Agilent scripts from server
        set archive_path "$::UTF::projarchives/unix/UTF/Agilent"
        $self scp $archive_path/*.* $dest_dir1

        # Move selected scripts to appropriate directory
        set dest_dir2 "/usr/local/bin"
        $self mkdir -p $dest_dir2
        $self mv -f $dest_dir1/epidiag.tmp $dest_dir2/epidiag-gpibfix

        set dest_dir3 "/usr/local/lib/epigram"
        $self mkdir -p $dest_dir3
        $self mv -f $dest_dir1/gpib.tcl $dest_dir3
        return
    }

	################################################################################
	#
	#	Agilent N6700B internal methods
	#
	################################################################################
	
	method __n6700_timedvwait {{timeout 10000}} {
		set timer [after $timeout [list set [myvar n6700_statesockfd] "Timeout"]]
		vwait [myvar n6700_statesockfd]
		after cancel $timer
	}

	method __n6700_getdata {} {

		if {![eof $n6700_sockfd]} {
			set msg [gets $n6700_sockfd]
			if {$msg != ""} {
				set hhmmss [clock format [clock seconds] -format "%T"]
				lappend n6700_response "$hhmmss $n6700_host $msg"
				UTF::Message LOG "DATA" "$hhmmss $n6700_host $msg"
				set n6700_statesockfd "DATA"
			}
		} else {
			fconfigure $n6700_sockfd -blocking 1
			catch {
			close $n6700_sockfd
			set hhmmss [clock format [clock seconds] -format "%T"]
			UTF::Message LOG "DATA" "$hhmmss $n6700_host close $n6700_sockfd"
			set n6700_sockfd "-1"
			}
		}
	}

	method __n6700_socket_connect {} {
		UTF::Message INFO "" "************************LibFunc:__n6700_socket_connect***************************"

		if {$n6700_sockfd == -1} {        
			# get the ip + port of the Agilent instrument
			set host_and_port [$self cget -lan_ip]
			set tmp [split $host_and_port " "]
			set n6700_host [lindex $tmp 0]
			set n6700_port [lindex $tmp 1]

			for {set retry_cnt 1} {$retry_cnt < 4} {incr retry_cnt} {
			
				if {[catch {socket $n6700_host $n6700_port} resp]} {
					UTF::Message ERROR "" "***Fail: error=$resp"
					error "ERROR: $resp"
				} else {
					set n6700_sockfd $resp
					fconfigure $n6700_sockfd -blocking 1 -buffering line
					fileevent $n6700_sockfd readable [mymethod __n6700_getdata]
					fconfigure $n6700_sockfd -blocking 0
					UTF::Sleep 2
					if {$n6700_sockfd == -1} {
						UTF::Message ERROR "" "Socket is closed. Retry $retry_cnt"
						continue
					}
					puts $n6700_sockfd ""
					UTF::Sleep 2
					flush $n6700_sockfd

					UTF::Message INFO "" "PASS: sockfd=$n6700_sockfd"
					break
				}
			}
		} else {
			UTF::Message INFO "" "Socket is already open. sockfd=$n6700_sockfd"
		}
	}

	method __n6700_socket_send {timeout_ms command} {
		$self __n6700_socket_send_2 300 "SYST:ERR?"
		set rsp [$self __n6700_get_response]
		if {![regexp {No error} $rsp]} {
			UTF::Sleep 2
			UTF::Message INFO "" "***Failed. SYST:ERR='$rsp'"
			return -1
		}

		$self __n6700_socket_send_2 $timeout_ms $command
	}
	
	method __n6700_socket_send_2 {timeout_ms command} {
		UTF::Message INFO "" "************************LibFunc:__n6700_socket_send $timeout_ms '$command'***************************"		
		set n6700_response ""
		set n6700_cmd_start_ms [clock clicks -milliseconds]
		set n6700_read_socket_timeout_ms $timeout_ms
		set n6700_statesockfd "WAIT"
		UTF::Message LOG "" "--->vwait $n6700_statesockfd"
		puts $n6700_sockfd $command
		$self __n6700_timedvwait
		UTF::Message LOG "" "--->returned vwait $n6700_statesockfd"

		# wait for the timeout (if any) to collect more data in n6700_response buffer
		while { 1 } {
			set elapsed_ms [expr [clock clicks -milliseconds] - $n6700_cmd_start_ms]
			if {$n6700_read_socket_timeout_ms == 0 || $elapsed_ms > $n6700_read_socket_timeout_ms} {
				break
			}

			after 50
			# call the __n6700_getdata since it is possible that there is data available but the
			# __n6700_getdata routine did not get called automatically
			$self __n6700_getdata
		}
	}

	method __n6700_get_response {} {
		UTF::Message INFO "" "************************LibFunc:__n6700_get_response***************************"
		for {set cnt 0} {$cnt < [llength $n6700_response]} {incr cnt} {
			set rsp [lindex $n6700_response $cnt]
			UTF::Message INFO "" "Response index $cnt=$rsp"		
		}

		# return only the 2nd line
		set rsp [lindex $n6700_response 1]

		# remove the date and ip address
		set Index [string first " " $rsp]
		if {$Index >= 0} {
			set rsp [string replace $rsp 0 $Index ""]
			set Index [string first " " $rsp]
			if {$Index >= 0} {
				set rsp [string replace $rsp 0 $Index ""]
			}
		}

		UTF::Message INFO "" "Rsp=$rsp"		
		set n6700_response ""

		return "$rsp"
	}

	method __n6700_set_voltage {voltage} {
		UTF::Message INFO "" "************************LibFunc:__n6700_set_voltage $voltage***************************"
		set resp ""

		UTF::Message INFO "" "Reset the Power Supply"
		$self __n6700_socket_send 5000 *RST
		$self __n6700_socket_connect
		UTF::Message INFO "" "Clear the Power Supply"
		$self __n6700_socket_send 300 *CLS
		UTF::Message INFO "" "Setup multiple-channel view"
		#$self __n6700_socket_send 300 "DISP:VIEW METER$options(-channel)|METER$options(-channel)"
		UTF::Message INFO "" "Get the Agilent instrument model number"
		$self __n6700_socket_send 300 "SYST:CHAN:MOD? (@$options(-channel))"
		$self __n6700_get_response
			
		UTF::Message INFO "" "Set the Voltage"
		$self __n6700_socket_send 500 "VOLT $voltage ,(@$options(-channel))"
		$self __n6700_socket_send 300 "VOLT? (@$options(-channel))"
		set rsp [$self __n6700_get_response]	
		UTF::Message INFO "" "Set the Voltage Bandwidth"
		$self __n6700_socket_send 500 "VOLT:BWID HIGH3 ,(@$options(-channel))"

		UTF::Message INFO "" "Set the Current to 1 Amp"
		$self __n6700_socket_send 500 "SENS:CURR:RANG 1,(@$options(-channel))"
		$self __n6700_socket_send 300 "SENS:CURR:RANG? (@$options(-channel))"
		$self __n6700_get_response

		UTF::Message INFO "" "Turn on channel $options(-channel)"
		$self __n6700_socket_send 500 "OUTP ON,(@$options(-channel))"
		$self __n6700_socket_send 300 "OUTP? (@$options(-channel))"
		$self __n6700_get_response

		UTF::Message INFO "" "rsp=$rsp"

		if {[string first $voltage $rsp] == -1} {
			UTF::Message INFO "" "***Failed. Voltage is set to $rsp instead of $voltage"
			return -1
		} else {
			UTF::Message INFO "" "PASS. Voltage is set to $voltage"
			return 0
		}
	}

	method __n6700_measure_voltage {} {
		UTF::Message INFO "" "************************LibFunc:__n6700_measure_voltage***************************"
		set resp ""

		UTF::Message INFO "" "Measure the Voltage"
		$self __n6700_socket_send 1000 "MEAS:VOLT? (@$options(-channel))"
	    UTF::Sleep 1
		set rsp [$self __n6700_get_response]	
		UTF::Message INFO "" "PASS. Voltage is $rsp"
		return $rsp
	}

	method __n6700_measure_current {} {
		UTF::Message INFO "" "************************LibFunc:__n6700_measure_current***************************"
		set resp ""

		UTF::Message INFO "" "Measure the Current"
		$self __n6700_socket_send 500 "MEAS:CURR? (@$options(-channel))"
		set rsp [$self __n6700_get_response]	
		UTF::Message INFO "" "PASS. Current is $rsp"
		return $rsp
	}

    method __n6700_setup_current_trigger {SampleTime SamplePoints OffsetPoints CurrentTrigger} {
		UTF::Message INFO "" "************************LibFunc:__n6700_setup_current_trigger***************************"

		UTF::Message INFO "" "Setup current function"
		$self __n6700_socket_send 300 "SENS:FUNC:CURR ON,(@$options(-channel))"
		UTF::Message INFO "" "Setup the sampling interval (5.12 to 40000 us)"
		$self __n6700_socket_send 300 "SENS:SWE:TINT $SampleTime,(@$options(-channel))"
		UTF::Message INFO "" "Setup the number of points to measure the current (1 to 512K)"
		$self __n6700_socket_send 300 "SENS:SWE:POIN $SamplePoints,(@$options(-channel))"
		$self __n6700_socket_send 300 "SENS:SWE:OFFS:POIN $OffsetPoints,(@$options(-channel))"
		$self __n6700_socket_send 300 "TRIG:ACQ:SOUR CURR$options(-channel),(@$options(-channel))"
		$self __n6700_socket_send 300 "TRIG:ACQ:CURR:LEV $CurrentTrigger,(@$options(-channel))"
		$self __n6700_socket_send 300 "TRIG:ACQ:CURR:SLOP POS,(@$options(-channel))"
		$self __n6700_socket_send 300 "SENS:SWE:TINT? (@$options(-channel))"
		set actual_sample_time [$self __n6700_get_response]	
		$self __n6700_socket_send 300 "STAT:OPER:COND? (@$options(-channel))"
		return $actual_sample_time
	}
	
    method __n6700_invoke_current_trigger {} {
		UTF::Message INFO "" "************************LibFunc:__n6700_invoke_current_trigger***************************"

		$self __n6700_socket_send 1000 "INIT:ACQ (@$options(-channel))"
		UTF::Sleep 1
		$self __n6700_socket_send 300 "STAT:OPER:COND? (@$options(-channel))"
		set rsp [$self __n6700_get_response]
		# bit 3 (8) = The measurement system is waiting for a trigger
		if {$rsp != ""} {
			set rc [expr $rsp & 8]
		} else {
			set rc 0
		}
		UTF::Message INFO "" "trigger bit='$rsp & 8'=$rc"
		if {$rc == 8} {
			UTF::Message INFO "" "Failed to trigger."
			return -1
		} else {
			UTF::Message INFO "" "Trigger successful."
			return 0
		}
	}

    method __n6700_force_current_trigger {} {
		UTF::Message INFO "" "************************LibFunc:__n6700_force_current_trigger***************************"
		set resp ""

		$self __n6700_socket_send_2 100 "INIT:ACQ (@$options(-channel))"

		UTF::Sleep 1
		$self __n6700_socket_send_2 100 "STAT:OPER:COND? (@$options(-channel))"
		set rsp [$self __n6700_get_response]
		# bit 3 (8) = The measurement system is waiting for a trigger
		if {$rsp != ""} {
			set rc [expr $rsp & 8]
		} else {
			set rc 0
		}
		UTF::Message INFO "" "trigger bit='$rsp & 8'=$rc"
		if {$rc == 8} {
			UTF::Message INFO "" "Did not trigger. Do Force Trigger."
			$self __n6700_socket_send_2 100 "TRIG:ACQ (@$options(-channel))"
			return -1
		} else {
			UTF::Message INFO "" "Trigger successful."
			return 0
		}
	}
	
    method __n6700_get_current_trigger_data_high_value {} {
		UTF::Message INFO "" "************************LibFunc:__n6700_get_current_trigger_data_high_value***************************"
		set resp ""

		# This query initiates and triggers a measurement, and returns the High level of a current pulse waveform in amperes.
		# The High level calculation generates a histogram of the waveform using 16 bins between the maximum and minimum data points. 
		# The bin containing the most data points above the 50% point is the high bin. The average of all the data points in the 
		# high bin is returned as the High level. If no high bin contains more than 1.25% of the total number of acquired points, then 
		# the maximum data point is returned.		

		UTF::Message INFO "" "Get current measurement data high value"
		UTF::Message INFO "" "Get high level current"
		$self __n6700_socket_send 1000 "FETC:CURR:HIGH? (@$options(-channel))"
		set rsp [$self __n6700_get_response]	
		UTF::Message INFO "" "PASS. Get current measurement data high value"
		return $rsp
	}
	
    method __n6700_get_current_trigger_data_low_value {} {
		UTF::Message INFO "" "************************LibFunc:__n6700_get_current_trigger_data_low_value***************************"
		set resp ""

		# This query initiates and triggers a measurement, and returns the Low level of a current pulse waveform in amperes.
		# The Low level calculation generates a histogram of the waveform using 16 bins between the maximum and minimum data points. 
		# The bin containing the most data points below the 50% point is the low bin. The average of all the data points in the 
		# low bin is returned as the Low level. If no low bin contains less than 1.25% of the total number of acquired points, then 
		# the minimum data point is returned.
		
		UTF::Message INFO "" "Get current measurement data low value"
		UTF::Message INFO "" "Get low level current"
		$self __n6700_socket_send 1000 "FETC:CURR:LOW? (@$options(-channel))"
		set rsp [$self __n6700_get_response]	
		UTF::Message INFO "" "PASS. Get current measurement data low value"
		return $rsp
	}
	
    method __n6700_get_current_trigger_data {} {
		UTF::Message INFO "" "************************LibFunc:__n6700_get_current_trigger_data***************************"
		set resp ""

		UTF::Message INFO "" "Get current measurement data"
		$self __n6700_socket_send 6000 "FETC:ARR:CURR? (@$options(-channel))"
		set rsp [$self __n6700_get_response]	
		UTF::Message INFO "" "PASS. Get current measurement data"
		return $rsp
	}
	
    UTF::doc {
	# [list_end]
    }
}

# Retrieve manpage from last object
UTF::doc [UTF::Power::Agilent man]

UTF::doc {
    # [list_end]
    # [para]
    # [section EXAMPLES]
    # SDIO dongle host using NPC22 in legacy (apshell) mode:
    # [example_begin]
UTF::DHD UTFTestNsdio -lan_ip UTFTestN -sta {4325sdio eth1} \\
    -console UTFTestC:40002 -power {UTFPower3 2} \\
    -hostconsole UTFTestC:40000
    # [example_end]

    # Same SDIO dongle host using NPC22 in new web-based mode:
    # [example_begin]
package require UTF::Power

UTF::Power::Synaccess UTFPower3
UTF::DHD UTFTestNsdio -lan_ip UTFTestN -sta {4325sdio eth1} \\
    -console UTFTestC:40002 -power {UTFPower3 2} \\
    -hostconsole UTFTestC:40000
    # [example_end]

    # Toshiba laptop using a WTI IPS-800 for the host power and dongle
    # supplies and a WebRelay for host power button:

    # [example_begin]
package require UTF::Power
UTF::Power::WTI UTFPower1
UTF::Power::WebRelay wr -relay UTFTestF
UTF::Power::Laptop toshiba -button {webrelay 1} -supply {UTFPower1 8}

UTF::WinDHD UTFTestP -user user -sta {4322bmac} \\
    -console "UTFTestF:40001" \\
    -osver 6 -installer inf \\
    -power {toshiba} \\
    -power_sta {UTFPower1 2}
    # [example_end]

    # Same example, but laptop is only controlled by power button:

    # [example_begin]
package require UTF::Power

UTF::Power::WTI UTFPower1
UTF::Power::WebRelay wr -relay UTFTestF
UTF::Power::Laptop toshiba -button {webrelay 1}

UTF::WinDHD UTFTestP -user user -sta {4322bmac} \\
    -console "UTFTestF:40001" \\
    -osver 6 -installer inf \\
    -power {toshiba} \\
    -power_sta {UTFPower1 2}
    # [example_end]

    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also [uri APdoc.cgi?UTF.tcl UTF]]
    # [see_also [uri APdoc.cgi?UTF::Base.tcl UTF::Base]]
    # [see_also [uri APdoc.cgi?apshell apshell]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
