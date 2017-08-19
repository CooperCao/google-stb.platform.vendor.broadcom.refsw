#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: 1d4e30758690ceb1f4a57ec84c899e129c86e70a $
# $Copyright Broadcom Corporation$
#

package provide UTF::WebRelay 2.0

package require snit
package require UTF::doc
package require UTF::Base

snit::type UTF::WebRelay {
    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -lan_ip 192.168.1.2
    option -relay localhost
    option -port 1

    constructor {args} {
	$self configurelist $args
	if {![string is digit $options(-port)]} {
	    error "Illegal port: $options(-port)"
	}
    }

    method {power off} {} {
	$options(-relay) rexec $UTF::unittest/webrelayshell \
	    $options(-lan_ip) relay${options(-port)}State=0
    }

    method {power on} {} {
	$options(-relay) rexec $UTF::unittest/webrelayshell \
	    $options(-lan_ip) relay${options(-port)}State=1
    }

    method {power cycle} {} {
        # NB: State=2 is a pulse "on" then "off", which is not the
        # normal behavior expected when UTF power cycles a device.
        # This leaves devices in the powered "off" state, and then
        # things really go downhill from there. So we implement the
        # power cycle as an explicit "off" followed by an explicit "on".
	$options(-relay) rexec $UTF::unittest/webrelayshell \
	    $options(-lan_ip) relay${options(-port)}State=0
	UTF::Sleep 5
        $options(-relay) rexec $UTF::unittest/webrelayshell \
	    $options(-lan_ip) relay${options(-port)}State=1

    }

}
