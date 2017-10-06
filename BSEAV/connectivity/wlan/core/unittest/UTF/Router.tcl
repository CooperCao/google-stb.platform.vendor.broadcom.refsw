#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::Router 2.0

package require snit
package require UTF::doc
package require UTF::Base
package require UTF::RTE

UTF::doc {
    # [manpage_begin UTF::Router n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF AP/Router support}]
    # [copyright {2006 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::Router host object, specific to AP/Routers running Linux.

    # Once created, the Router object's methods are not normally
    # invoked directly by test scripts, instead the Router object is
    # designated the host for one or more platform independent STA
    # objects and it will be the STA objects which will be refered to
    # in the test scripts.

    # [list_begin definitions]

}

snit::type UTF::Router {
    UTF::doc {
	# [call [cmd UTF::Router] [arg host]
	#	[option -lan_ip] [arg address]
	# 	[lb][option -ssh] [arg path][rb]
	#       [lb][option -brand] [arg brand][rb]
	#       [lb][option -tag] [arg tag][rb]
	#       [lb][option -image] [arg image][rb]
	# 	[lb][option -serial] [arg port][rb]
	# 	[lb][option -cfe] [arg {cfe image}][rb]
	# 	[lb][option -model] [arg model][rb]
	# 	[lb][option -serial_num] [arg {serial number}][rb]
	#       [lb][option -nvram] [arg {key=value ...}][rb]
	#       [lb][option -ush] [arg {true|false}][rb]
	#       [lb][option -lanpeer] [arg peer][rb]
	#       [lb][option -wanpeer] [arg peer][rb]
	#       [lb][option -txt_override] [arg {key=value ...}][rb]
        #       [arg ...]]

	# Create a new Router host object.  The host will be
	# used to contact STAs residing on that host.
	# [list_begin options]

	# [opt_def [option -lan_ip] [arg address]]

	# IP address to be used to contact host.  This should be a
	# backbone address, not involved in the actual testing.

	# [opt_def [option -ssh] [arg path]]

	# Specify an alternate command to use to contact [arg host],
	# such as [cmd rsh] or [cmd fsh].  The default is [cmd
	# apshell].

	# [opt_def [option -brand] [arg brand]]

	# Specify which brand of build image to use for [method
	# findimages] and [method load].  Defaults to [arg
	# linux-internal-router].

	# [opt_def [option -tag] [arg tag]]

	# Specify a tagged build image to use for [method findimages]
	# and [method load].  Defaults to [arg trunk].

	# [opt_def [option -image] [arg image]]

	# Specify a default image to install when [method load] is
	# invoked.  This can be an explicit path to a [file linux.trx]
	# file, or a suitable list of arguments to [method
	# findimages].  Overrides [option -brand] and [option -tag].

	# [opt_def [option -serial] [arg port]]

	# Identify a serial port connected to the [arg host].  If a
	# serial port is specified, connnections to the router will
	# be made via the serial port, otherwise connections will use
	# the telnet port on [arg address]

	# [opt_def [option -cfe] [arg {cfe image}]]

	# Specify a default CFE image to install when [method
	# load_cfe] is invoked.  If not specified, UTF will attempt to
	# discover a CFE image based on OS image specified by the [arg
	# image] option.

	# [opt_def [option -model] [arg model]]

	# Specify Router model name, used for serialising CFE images.
	# Model names should correspond to [file .txt] files from the
	# [file build/image/] directory of a router build.

	# [opt_def [option -serial_num] [arg {serial number}]]

	# Specify Router's serial number.

	# [opt_def [option -nvram] [arg {key=value ...}]]

	# Specify default nvram settings.  These will be prepended to
	# any nvram lists provided to the [method reboot] method.

	# [opt_def [option -ush] [arg {true|false}]]

	# Specify if ush should be used to communicate with router.
	# Default is "true".

	# [opt_def [option -lanpeer] [arg peer]]

	# Identify STA interface on the LAN used to communicate with
	# the router.  This is probably an interface on the [arg
	# relay].  This is used in cases where the interface is under
	# DHCP control and needs to be refreshed whenever the router's
	# IP configuration changes.

	# [opt_def [option -wanpeer] [arg peer]]

	# Identify STA interface on the WAN used to communicate with
	# the router.

	# [opt_def [option -txt_override] [arg {key=value ...}]]

	# Specify additional serialization options to be appended to
	# the CFE .txt file during a CFE upgrade.  eg:

	# [example_begin]

	-txt_override {clkfreq=300}

	# [example_end]

	# [opt_def [option -coldboot] [arg {true|false}]]

	# If true, cause [method reboot] to trigger a power cycle.
	# Used for BU boards having problem with warm reboots.

	# [opt_def [arg ...]]

	# Unrecognised options will be passed on to the host's [cmd
	# UTF::Base] object.

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

    option -epi_ttcp "epi_ttcp"
    option -nvserial "/projects/hnd/tools/linux/bin/nvserial"
    option -image
    option -tag "trunk"
    option -cfetag ""
    option -cfebrand ""
    option -trx "linux"
    option -date "%date%"
    option -web -type {snit::integer -min 0 -max 2} -default 0
    option -cfe
    option -sta
    option -name -configuremethod CopyOption
    option -serial_num -readonly yes
    option -model
    option -console -configuremethod _adddomain
    option -conslogport 0
    option -rteconsole -configuremethod _adddomain
    option -msgcallback
    option -device
    option -lan_ip 192.168.1.1
    option -passwd "admin"
    option -nvram
    option -ush -type snit::boolean -default true
    option -utelnet -type snit::boolean -default false
    option -epirelay
    option -wanpeer
    option -txt_override
    option -wlinitcmds
    option -postboot
    option -coldboot -type snit::boolean -default false
    option -coldinstall -type snit::boolean -default false
    option -bootwait -type snit::double -default 10
    option -restartwait -type snit::double -default 7
    option -embeddedimage 43602a1
    option -defer_restart -type snit::boolean -default false

    # options to allow tests to be modified per device
    option -nosamba -type snit::boolean -default false

    # Option to force a reboot if the AP is not pingable during init.
    # This is a partial WAR for some Linksys ethernet issues.  It's
    # probably not safe to use in the general case because there may
    # be other reasons why an AP may not be pingable.
    option -rebootifnotpingable -type snit::boolean -default false

    # base handles any other options and methods
    component base -inherit yes

    # Default nvram settings
    # regulatory 0x80000 disabled as temporary WAR for EAGLE perf issues
    variable nvram_defaults {
	wl_msglevel=0x101
	console_loglevel=7
	wl0_obss_coex=0
	wl1_obss_coex=0
	wl0_bw_cap=-1
	wl1_bw_cap=-1
	wl0_reg_mode=off
	wl1_reg_mode=off
	rclocal=/media/nand/rc.local
    }

    variable Bridge
    variable stas {}
    variable igmpq -array {}
    variable expectBoot 0

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	install base using \
	    UTF::Base %AUTO% -ssh $UTF::usrutf/apshell \
	    -init [mymethod init] -brand "linux-internal-router" \
	    -wlconf_by_nvram true
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
	    if {$options(-device) eq ""} {
		# Full dongle routers need at least one device
		# name.
		set options(-device) $dev
	    }
	}

	$base configure -lan_ip $options(-lan_ip)
	if {[$self cget -relay] eq ""} {
	    error "No -relay specified for $options(-name)"
	}
	array unset igmpq
	array set Bridge "stale 1"
    }

    destructor {
	catch {$self igmp_querier disable}
	catch {$base destroy}
	foreach sta $stas {
	    catch {$sta destroy}
	}
    }

    # Internal method for copying options to delegates
    method CopyOption {key val} {
	set options($key) $val
	$base configure $key $val
    }

    method _adddomain {name val} {
	set options($name) [UTF::AddDomain $val]
    }

    UTF::doc {
	# [call [arg host] [method prepare_cfe] [lb][arg file][rb]]

	# Prepare CFE image for loading on to router, including
	# serialization.  [arg host] option [option -model] must be
	# set to an appropriate txt file to serialise the CFE image
	# for the router.[para]

	# [method prepare_cfe] is used to dry-run the CFE update
	# process before attempting the actual load.
    }

    method prepare_cfe {args} {
	UTF::Message INFO $options(-name) "Prepare CFE"
	switch -re $options(-model) {
	    {^$} {
		error {CFE needs -model to be set, eg "-model bcm94704agr"}
	    }
	    {471[02]|4704|535[0-4]} {
		set bin "cfe.bin"
		set zbin "cfez.bin"
	    }
	    {4705|4785} {
		set bin "cfe-gige.bin"
		set zbin "cfez-gige.bin"
	    }
	    {470[6-7]|471[6-8]|5356|5357[12]} {
		set bin "cfe-gmac.bin"
		set zbin "cfez-gmac.bin"
	    }
	    {470[89]} {
		set bin "cfe-nflash.bin"
		set zbin "cfez-nflash.bin"
	    }
	    {5357[34]} {
		set bin "cfe-a7.bin"
		set zbin "cfez-a7.bin"
	    }
	    default {
		error "Unrecognised router - please update prepare_cfe"
	    }
	}
	if {$args == ""} {
	    set args $options(-cfe)
	}
	if {$options(-cfetag) eq ""} {
	    set options(-cfetag) $options(-tag)
	}
	if {$options(-cfebrand) eq ""} {
	    set options(-cfebrand) [$self cget -brand]
	}
	if {[file exists $args]} {
	    set file $args
	} else {
	    set file [$self findimages -brand $options(-cfebrand) \
			  -tag $options(-cfetag) {*}$args $bin]

	    set size [file size $file]
	    if {$size >= 256*1024/2} {
		UTF::Message LOG $options(-name) "$file: $size too large"
		regsub {cfe} $file {cfez} file
		set size [file size $file]
		if {[file size $file] >= 256*1024} {
		    error "$file: $size too large"
		}
	    }
	    UTF::Message LOG $options(-name) "$file: $size ok"
	}
	# If we're pointed at a compressed image, uncompress before copying
	if {[file extension $file] eq ".gz"} {
	    set newfile "/tmp/[file tail [file rootname $file]]_ungz_$::tcl_platform(user)"
	    UTF::Message LOG $options(-name) "gunzip -c $file > $newfile"
	    exec gunzip -c $file > $newfile
	    set file $newfile
	    unset newfile
	}
	if {$options(-serial_num) eq ""} {
	    set options(-serial_num) [$self nvram get boardnum]
	}
	set nvramdir [file join {*}[lreplace [file split $file] end-2 end \
					build image]]
	set sfile [file join /tmp [file tail $file]]
	regsub {_ungz} $sfile {} sfile
	set txt [file join $nvramdir "$options(-model).txt"]
	if {![file exists $txt]} {
	    UTF::Message WARN $options(-name) \
		"No serialization file for $options(-model) in $nvramdir"

	    if {[catch {$self findimages -tag $options(cfetag) \
			    "$options(-model).txt"} txt] &&
		[catch {$self findimages -tag trunk "$options(-model).txt"} \
		     txt]} {
		UTF::Message WARN $options(-name) \
		    "No serialization file for $options(-model)"

		foreach txt [$self findimages -all "*$options(-model)*.txt"] {
		    set txts([file rootname [file tail $txt]]) 1
		}
		error "Try: [lsort [array names txts]]"
	    }
	}
	if {$options(-txt_override) ne ""} {
	    set envram "\n"
	    foreach a [UTF::decomment $options(-txt_override)] {
		if {![regexp {^([^=]*)=\"?([^=\"]*)\"?$} $a - k v]} {
		    error "Bad nvram setting \"$a\": should be key=value"
		} else {
		    append envram "$k=$v\n"
		}
	    }
	    UTF::Message INFO $options(-name) "Append to $txt:\n$envram"
	    set tf [open $txt]
	    fconfigure $tf -translation binary
	    set data [read $tf]
	    close $tf
	    append data $envram
	    set tt [open /tmp/txt w]
	    fconfigure $tt -translation binary
	    puts $tt $data
	    close $tt
	    set txt /tmp/txt
	}
	UTF::Message INFO $options(-name) "$options(-nvserial) -z \
	    -i $file -o $sfile -s $options(-serial_num) $txt"
	exec $options(-nvserial) -z \
	    -i $file -o $sfile -s $options(-serial_num) $txt
	return $sfile
   }

    UTF::doc {
	# [call [arg host] [method load_cfe] [lb][arg -jtag][rb]
	# [lb][arg file][rb]]

	# Load CFE image on to router.  [arg host] option [option
	# -model] must be set to an appropriate txt file to serialise
	# the CFE image for the router.[para]

	# If [arg -jtag] is specified, the image will be uploaded over
	# JTAG.  [arg host] option [option -epirelay] must be provided
	# to use JTAG.
    }

    method load_cfe {args} {
	UTF::GetKnownopts {
	    {jtag "Use JTAG"}
	    {norestore "Don't restore defaults after update"}
	}
	set sfile [$self prepare_cfe {*}$args]
	set f "/tmp/[file tail $sfile]"
	regsub _$::tcl_platform(user) $f {} f

	UTF::Message INFO $options(-name) "Load CFE"

	if {$(jtag)} {
	    if {$options(-epirelay) eq ""} {
		error "Cannot use JTAG without -epirelay"
	    }
	    $options(-epirelay) copyto $sfile $f
	    $self epidiag "flwritefile $f; cpuboot"
	} else {
	    $self relay copyto $sfile $f
	    if {[catch {$self apshell :}]} {
		$self power cycle
	    }
	    $self relay rexec -t 180 \
		$UTF::usrutf/apshell $options(-console) load $f
	}
	if {!$(norestore)} {
	    $self restore_defaults
	}
    }

    UTF::doc {
	# [call [arg host] [method reboot] [lb][arg {key=value ...}][rb]]

	# Set and commit nvram variables and reboot.  Variables will
	# be set at the [option {# }] prompt, if possible, otherwise
	# they will be set at the [option {CFE> }] prompt.

	# If an nvram variable list is provided, then any nvram
	# defaults specified in the [arg host]s configuration option
	# [option -nvram] will be prepended to the list.

	# Note that, as with all TCL proc arguments, nvram args should
	# be valid tcl list elements.  In particular,

	# [example_begin]

	$host reboot lan_ifnames="vlan1 eth1"

	# [example_end]

	# is not valid since the quotes are embedded in the literal
	# text, but these are valid because the whole argument is
	# protected:

	# [example_begin]

	$host reboot "lan_ifnames=vlan1 eth1"
	$host reboot {lan_ifnames=vlan1 eth1}
	$host reboot {lan_ifnames="vlan1 eth1"}

	# [example_end]

	# If the router is unresponsive and its power can be
	# controlled then it will be power cycled.  This is useful for
	# running tests with nvram settings which may crash the
	# router.
    }

    method UseApshell {} {
	$self configure -ssh $UTF::usrutf/apshell
	$base configure -lan_ip $options(-console)
	# Use init 2 to prevent recursion
	$base configure -initialized 2

	# Still need to check rte, if there is one
	if {$options(-rteconsole) ne ""} {
	    $self open_rte_messages
	}
    }

    method {rc restart} {} {
	if {[regexp {netbsd} $options(-trx)]} {
	    $self rexec kill -1 1
	} else {
	    $self rexec rc restart
	}
	$base configure -initialized 0
    }

    UTF::doc {
	# [call [arg host] [method restart] [lb][arg {key=value ...}][rb]]

	# Set and commit nvram variables and run rc restart.  This is
	# similar to [arg host] [method reboot] but closer to the UI
	# behaviour.  Note that some nvram settings require a full
	# reboot and will not take effect during a restart.
    }

    method restart {args} {
	set qargs {}
	foreach a $args {
	    if {![regexp {^([^=]*)=(.*)$} $a - k v]} {
		error "Bad nvram setting \"$a\": should be key=value"
	    } else {
		# Strip extra quotes
		regsub {^\"(.*)\"$} $v {\1} v
		if {$k eq "lan_ipaddr"} {
		    UTF::Message INFO $options(-name) "Address change: $v"
		    set lan_ip $v
		}
		lappend qargs "$k=\"$v\""
	    }
	}
	foreach a $qargs {
	    $self rexec nvram set $a
	}
	if {[llength $qargs]} {
	    $self nvram commit
	}
	if {$options(-defer_restart)} {
	    UTF::Message LOG $options(-name) "Deferred restart"
	    return
	}
	if {[info exists lan_ip]} {
	    # May hang as IP address changes
	    catch {$self -t 2 rc restart}
	} elseif {[info exists ::UTF::RcRestartHangWAR]} {
	    $self /sbin/rc restart
	} else {
	    $self rc restart
	}
	set Bridge(stale) 1
	$base configure -initialized 0
	UTF::Sleep $options(-restartwait)

	if {[info exists lan_ip]} {
	    $self configure -lan_ip $lan_ip
	    # Don't try to reconnect yet since the caller may have to
	    # fix the relay's IP first.
	} elseif {[catch {$self rexec :} ret]} {
	    error "connection problem after rc restart" $::errorInfo
	}
	# WAR for PR#73275
	if {[regexp {netbsd} [$self cget -brand]]} {
	    if {[regexp -line {ZW\s.*\s\(ushd\)} [$self rexec ps -auxww]]} {
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic "Killing zombies"
		}
		$self apshell killall ushd
		$self apshell ushd -d
	    }
	}
	if {$options(-wlinitcmds) ne ""} {
	    $self rexec [string trim $options(-wlinitcmds)]
	}
	return
    }

    method reboot {args} {

	$self open_messages
	$self UseApshell
	if {[llength $args] > 1 && [lindex $args 0] == "-t"} {
	    set timeout "-t [lindex $args 1]"
	    set args [lreplace $args 0 1]
	} else {
	    set timeout ""
	}
	if {$options(-coldboot)} {
	    if {[lindex $args 0] eq "erase"} {
		set args [lreplace $args 0 0]
		catch {$self erase nvram}
	    } else {
		set qargs {}
		foreach a $args {
		    if {![regexp {^([^=]*)=(.*)$} $a - k v]} {
			error "Bad nvram setting \"$a\": should be key=value"
		    } else {
			# Strip extra quotes
			regsub {^\"(.*)\"$} $v {\1} v
			lappend qargs "$k=\"$v\""
		    }
		}
		foreach a $qargs {
		    $self rexec nvram set $a
		}
		if {[llength $qargs]} {
		    $self nvram commit
		}
	    }
	    set expectBoot 1
	    $self power cycle
	}
	try {
	    set expectBoot 1
	    if {[catch {
		$self rexec -s $timeout reboot $args
	    } ret]} {
		if {[regexp {Bad nvram} $ret] || [$self cget -power] eq ""} {
		    error $::errorInfo
		}
		# only report end of error message
		regsub {.*\n} $ret {} ret
		UTF::Message LOG $options(-name) "error: $ret"
		UTF::Message WARN $options(-name) "try power cycle"
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic "Reboot failed - try power cycle"
		}
		$self power cycle
		if {[catch {
		    $self rexec -s $timeout reboot $args
		}]} {
		    $self power cycle
		    UTF::Sleep 20
		}
	    }

	    # Additional wait to make sure rc has run
	    UTF::Sleep $options(-bootwait)

	    # Detecting a Vx prompt at this stage is not necessarily an
	    # error - we may be restoring defaults prior to an OS switch
	    if {[catch {$self rexec :} ret] && ![regexp {prompt} $ret]} {
		# Perhaps it hung?
		$self power cycle
		UTF::Sleep 20
		$self rexec :
	    }
	    if {[regexp {code\.bin|netbsd} \
		     "$options(-image) $options(-trx)"] ||
		$options(-rteconsole) ne ""} {
		# Netbsd and Linksys may need an extra exit to make the
		# boot complete
		catch {$self apshell exit}
	    }
	} finally {
	    set expectBoot 0
	}
	UTF::Sleep 5

	$base configure -initialized 0
	set Bridge(stale) 1

	if {$options(-postboot) ne ""} {
	    $self rexec [string trim $options(-postboot)]
	}
	if {$options(-wlinitcmds) ne ""} {
	    $self rexec [string trim $options(-wlinitcmds)]
	}
	return
    }

    UTF::doc {
	# [call [arg host] [method restore_defaults]
	#      [lb][option -noerase][rb] [lb][option -nosetup][rb]]

	# erase nvram and reboot

	# [list_begin options]

	# [opt_def [option -noerase]]

	# Don't erase nvram before applying local defaults

	# [opt_def [option -nosetup]]

	# Don't apply local defaults after erasing nvram

	# [list_end]
    }
    method restore_defaults {args} {
	UTF::Getopts {
	    {noerase "Don't erase nvram first"}
	    {nosetup "Don't apply local setup"}
	    {n "Don't apply - just return settings"}
	}
	# Clear cached data
	foreach S $stas {
	    $S configure -ssid {} -security {} -wepkey {} -wepidx {} -wpakey {}
	}
	set nvram ""
	if {!$(nosetup)} {
	    # Merge config nvram with built-in defaults
	    foreach a [concat \
			   [UTF::decomment $nvram_defaults] \
			   [UTF::decomment $options(-nvram)]] {
		if {![regexp {^([^=]*)=(.*)$} $a - k v]} {
		    error "Bad nvram setting \"$a\": should be key=value"
		} else {
		    set nv($k) $v
		}
	    }
	    # defaults stored in options
	    if {$options(-passwd) ne "admin"} {
		set nv(http_passwd) $options(-passwd)
	    }
	    if {$options(-lan_ip) ne "192.168.1.1"} {
		set nv(lan_ipaddr) [UTF::resolve $options(-lan_ip)]
	    }
	    foreach k [lsort [array names nv]] {
		lappend nvram "$k=$nv($k)"
	    }
	}

	if {$(n)} {
	    return $nvram
	}
	if {!$(noerase)} {
	    # erase and set our defaults before first boot, since some
	    # settings may be required to boot.
	    $self reboot erase {*}$nvram
	}
	if {$nvram ne ""} {
	    # set our defaults again, in case the os overwrote them
	    $self reboot {*}$nvram

	    $self lanwanreset
	}
	return
    }

    method lanwanreset {} {
	# Reset IP on lanpeer in case we're using DHCP, or if we
	# had to use a temporary IP for the erase.
	foreach l [$self cget -lanpeer] {
	    $l ifconfig [$l cget -ipaddr]
	    $l add_networks $self
	}
	if {$options(-wanpeer) ne ""} {
	    $options(-wanpeer) add_networks $self
	    if {[catch {$self wan_forwarding $options(-wanpeer)} ret]} {
		$self warn $ret
	    }
	}
    }

    UTF::doc {
	# [call [arg host] [method wan_forwarding] [arg wan]]

	# Set up wan forwarding.  This provides NAT rules on the WAN
	# endpoint and port forwarding rules on the Router such that
	# WAN->STA iperf traffic is transparently diverted through the
	# Router's NAT tables, engaging CTF forwarding and improving
	# performance significantly.  Note this only speeds up access
	# to the default iperf port 5001 on the first 5 DHCP addresses
	# on LAN and GUEST nets.
    }

    method wan_forwarding {wan} {
	set wanip [string trim [$self nvram get wan0_ipaddr]]
	set dhcp_start [string trim [$self nvram get dhcp_start]]
	set dhcp1_start [string trim [$self nvram get dhcp1_start]]
	set num_ports 64
	set num_stas 5
	set nvnum 0

	set iport_start 5001
	set iport_end [expr {$iport_start + $num_ports - 1}]
	set p $iport_start

	array set nv {}
	foreach {- k v} \
	    [regexp -all -line -inline {(forward_port\d+)=(.*)} \
		 [$self rexec -s \
		      "nvram show 2>/dev/null| grep forward_port"]] {
			  set nv($k) unset
		      }
	# LAN
	regexp {(.*)\.(\d+)$} $dhcp_start - net num
	for {set i 0} {$i < $num_stas} {incr i} {
	    lappend ips $net.[expr {$num+$i}]
	}
	# LAN1
	regexp {(.*)\.(\d+)$} $dhcp1_start - net num
	for {set i 0} {$i < $num_stas} {incr i; incr p $num_ports} {
	    lappend ips $net.[expr {$num+$i}]
	}
	# LAN endpoint (if not already included)
	set ip [$self lan cget -ipaddr]
	if {$ip ne "dhcp" && [lsearch $ips $ip] < 0} {
	    lappend ips $ip
	}

	foreach ip $ips {
	    set pend [expr {$p + $num_ports - 1}]
	    set k "forward_port$nvnum"
	    set nv($k) "$p-$pend>$ip:$p-$pend,tcp,on"
	    incr nvnum
	    $wan portmap set $ip $iport_start 1 $wanip $p $num_ports
	    set k "forward_port$nvnum"
	    set nv($k) "$p-$pend>$ip:$p-$pend,udp,on"
	    incr nvnum
	    $wan portmap set $ip $iport_start 0 $wanip $p $num_ports

	    incr p $num_ports
	}

	# Flush WAN IP tables
	$wan iptables -t nat -F

	# Record port mapping
	$wan portmap write

	# Update Router forwarding table
	foreach k [lsort [array names nv]] {
	    set v $nv($k)
	    if {$v eq "unset"} {
		$self rexec "nvram unset $k"
	    } else {
		$self rexec "nvram set '$k=$v'"
	    }
	}
	$self restart
    }

    UTF::doc {
	# [call [arg host] [method envram_setup]]

	# Set up embedded nvram.  This is the defaults for the nvram
	# embedded in the CFE image.  This allows you to change the
	# defaults used by the router after a full nvram erase.
	# Normally these would have to be added to the serialised CFE
	# image before loading the new CFE, but the (internal-only)
	# envram command allows us to chenge these on vendor platforms
	# where we may not want to touch the vendor CFE. [para]

	# Uses the same [option -txt_override] as used in [method
	# load_cfe]
    }

    method envram_setup {} {
	set envram [UTF::decomment $options(-txt_override)]
	if {$envram ne ""} {
	    foreach a $envram {
		if {![regexp {^([^=]*)=\"?([^=\"]*)\"?$} $a - k v]} {
		    error "Bad nvram setting \"$a\": should be key=value"
		} else {
		    lappend qargs "$k=\"$v\""
		}
	    }
	    foreach a $qargs {
		$self rexec envram set $a
	    }
	    $self envram commit
	}
    }

    UTF::doc {
	# [call [arg host] [method findimages]
	#              [lb][option -all][rb]
	#              [lb][option -ls][rb]
	#              [lb][option -brand] [arg brand][rb]
	#              [lb][option -tag] [arg tag][rb]
	#	       [lb][option -date] [arg date][rb]
	#	       [arg file]]

	# Returns the pathnames of an os image from the standard build
	# repository.  Globbing patterns are accepted in all
	# arguments.

	# [list_begin options]

	# [opt_def [option -all]]

	# Return all matches, sorted by date with the youngest first, then
	# by features.  By default, only the first match is returned.

	# [opt_def [option -ls]]

	# Run ls -l on results.  By default, only the file name is
	# returned.

	# [opt_def [option -brand] [arg brand]]

	# Specify build brand.  Default is the [option -brand] option
	# of [arg host].

	# [opt_def [option -tag] [arg tag]]

	# Select a build tag.  Default is the [option -tag] option of
	# [arg host].

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2005.8.17.0].  Default is to
	# return the most recent build available.

	# [para]

	# The default is to return all valid build types.

	# [opt_def [arg file]]

	# Specify the file type being searched for.

	# [arg file] should be a [file .trx] file, eg [file
	# linux.trx].  If [arg file] is a directory, it is assumed to
	# be a user's private source tree and will be searched
	# accordingly.

	# [para]

	# [list_end]

    }

    method findimages {args} {
	if {$args == "" || $args == "-all"} {
	    set args [concat $args $options(-image)]
	}

	# Findimages start
	UTF::Message LOG $options(-name) "findimages $args"

	# Defaults from object
	UTF::GetKnownopts [subst {
	    {all "return all matches"}
	    {ls "Report ls -l"}
	    {showpattern "Show search patterm"}
	    {web.arg "" "ignored"}
	    {brand.arg "[$self cget -brand]" "brand"}
	    {tag.arg "$options(-tag)" "Build Tag"}
	    {date.arg "$options(-date)" "Build Date"}
	    {trx.arg "$options(-trx)" "Trx name"}
	    {gub.arg "[$self cget -gub]" "Alternate GUB build path"}
	}]

	set file $args

	if {$file eq ""} {
	    set file "$(trx).trx"
	} elseif {[file isdirectory $file]} {
	    # Look for an image, assuming this is part of a developer
	    # workspace.
	    if {[regexp {netbsd} $(trx)]} {
		# Netbsd private builds won't have the _47k
		return [glob "$file{{/netbsd,}/images,}/netbsd.trx"]
	    }
	    return [glob "$file{{/router,}/{mipsel,arm}{,-uclibc},}/$(trx).trx"]
	} elseif {[UTF::BuildFile::exists $file]} {
	    return $file
	}

	if {$file eq "rtecdc.exe"} {
	    set tail [file join "{build/dongle,src/dongle/rte/wl/builds}/$options(-embeddedimage)*/*" $file]
	    if {[regexp {dhdap} [$self cget -brand]]} {
		set tail [file join "{build/,}43*" $tail]
	    }
	} elseif {[regexp {\.txt$} $file]} {
	    set tail [file join build image $file]
	} elseif {[regexp {^netbsd} $(brand)]} {
	    set tail [file join release images $file]
	} elseif {[regexp {RTRFALCON} $(tag)]} {
	    # RTRFALCON doesn't have a build folder
	    set tail [file join src router mipsel-uclibc $file]
	} else {
	    set tail [file join build image $file]
	}

	if {[regexp {_REL_} $(tag)]} {
	    set (tag) "{PRESERVED/,ARCHIVED/,}$(tag)"
	} else {
	    set (tag) "{PRESERVED/,}$(tag)"
	}

	if {[regexp {^vx} $(brand)]} {
	    error "Cannot load VxWorks on a UTF::Router.  Use UTF::Vx."
	}

	set pattern [file join \
			 $::UTF::projswbuild $(gub) build_linux \
			 $(tag) $(brand) $(date)* "$tail{,.gz}"]

	if {$(showpattern)} {
	    UTF::Message INFO $options(-name) $pattern
	}
	UTF::SortImages [list $pattern] \
	    {*}[expr {$(ls)?"-ls":""}] \
	    {*}[expr {$(all)?"-all":""}]
    }

    UTF::doc {
	# [call [arg host] [method load] [lb][option -web][rb]
	#	  [lb][option -force][rb] [lb][arg file][rb]]
	# [call [arg host] [method load] [lb][option -web][rb]
	#	  [lb][option -force][rb] [lb][arg {args ...}][rb]]

	# Install an OS image onto the Router.  If [option -web] is
	# specified, a web upgrade will be performed, otherwise the
	# load will be performed via CFE.  In the first form [arg
	# file] should be the pathname of a suitable [file linux.trx]
	# file.  In the second form, the argument list will be passed
	# on to [method findimages] to find a driver.  If no arguments
	# are specified, those stored in the [arg host]s [option
	# -image] option will be used instead.  [para]

	# By default, the load will be skipped if the image file has
	# exactly the same path and checksum as the previously loaded
	# image, and [syscmd {wl ver}] returns exactly the same
	# results as were returned after the previous load.  Use
	# [option -force] to force the new image to be loaded anyway.
    }
    variable loaded ""

    method load_needed {args} {
	expr {[catch {$self findimages {*}$args} file] ||
	      $file ne $loaded}
    }

    # Save src and exe references, in case host is down when we want
    # to analyse a crash
    variable oops_src
    variable hndrte_src
    variable hndrte_exe

    variable stuckreading 0

    # semaphore to indicate router is already looping
    variable bootlooping 0

    method load {args} {
	UTF::Message INFO $options(-name) "Install Router Image"

	UTF::GetKnownopts [subst {
	    {web.arg "$options(-web)" "Use web upload"}
	    {force "Force upload, even if not changed"}
	    {erase "Erase nvram before loading"}
	    {nvram.arg "" "Nvram key=value list"}
	    {coldinstall.arg "$options(-coldinstall)" \
		 "Power cycle before loading"}
	    {n "Copy files, but don't reload the driver"}
	    {all "ignored"}
	    {ls "ignored"}
	}]
	snit::boolean validate $(coldinstall)

	if {$(erase)} {
	    set (nvram) [concat [$self restore_defaults -n] $(nvram)]
	}

	set nowl 0

	# We don't know what state the router is in, so start with
	# reset checks disabled.
	set expectBoot 1

	set file [$self findimages {*}$args]
	UTF::Message LOG $options(-name) "Found $file"
	set info "$file\n[UTF::BuildFile::sum $file]"
	regsub {/} $options(-name) {.} name
	set idfile "/tmp/$name-id"
	set src "/tmp/$name-src.lnk"

	# Try to leave a link to the source code for gdboops
	if {[regexp {dhdap} [$self cget -brand]]} {
	    # DHDAP uses a seperate src tree for the dongle
	    if {![regsub {/src.*} $file {/src} rs]} {
		set rs [file join {*}[lreplace [file split $file] end-2 end main src]]
	    }
	} else {
	    # USBAP uses same src tree for the dongle
	    if {[regexp {/build/} $file]} {
		set rs [file join {*}[lreplace [file split $file] end-2 end main src]]
	    } else {
		set rs [file join {*}[lreplace [file split $file] end-3 end src]]
	    }
	}
	$self relay rm -f $src
	if {[UTF::BuildFile::isdirectory $rs] ||
	    [UTF::BuildFile::isdirectory [set rs [regsub {/main/} $rs {/}]]]} {
	    UTF::Message LOG $options(-name) "Router Source: $rs"
	    $self relay ln -s '$rs' $src
	    set oops_src $rs
	} else {
	    UTF::Message LOG $options(-name) "Router Source not found: $rs"
	}
	if {[regexp {usbap|dhdap} [$self cget -brand]]} {
	    $self relay rm -rf \
		"/tmp/$name-hndrte-src.lnk" "/tmp/$name-hndrte-exe.lnk"
	    # Try to find symbol table.
	    if {[info exists oops_src]} {
		# Try looking in the src of the found router first
		# before falling back to a findimages search
		set tail [file join "{build/dongle,src/dongle/rte/wl/builds}/$options(-embeddedimage)*/*" rtecdc.exe]
		if {[regexp {dhdap} [$self cget -brand]]} {
		    set tail [file join "{build/,}43*" $tail]
		}
		set tail [file join $oops_src ".." ".." $tail]
		if {![catch {UTF::BuildFile::glob [file join $tail]} ret]} {
		    set exe [lindex $ret 0]
		} else {
		   UTF::Message LOG $options(-name) \
			"Symbol table not found in Router src: $tail"
		}
	    }
	    if {[info exists exe] ||
		![catch {$self findimages {*}$args rtecdc.exe} exe]} {
		UTF::Message LOG $options(-name) "Symbol file: $exe"
		$self relay ln -s '$exe' "/tmp/$name-hndrte-exe.lnk"
		set hndrte_exe $exe

		# Base dongle src tree on dongle symbol table
		if {[regsub {(src/dongle/rte/wl/builds|build/dongle)/.*} \
			 $hndrte_exe {src} hndrte_src]} {
		    UTF::Message LOG $options(-name) "Dongle Source: $hndrte_src"
		    $self relay ln -s '$hndrte_src' \
			"/tmp/$name-hndrte-src.lnk"
		} else {
		    UTF::Message LOG $options(-name) \
			"Dongle Source not found"
		}
		UTF::RTE::init
	    } else {
		UTF::Message LOG $options(-name) \
		    "Symbol file not found: $exe"
	    }
	} elseif {[regexp {hsic} [$self cget -brand]]} {
	    # HSIC host has no wl chip
	    set nowl 1
	}

	if {$(n)} {
	    UTF::Message LOG $options(-name) "Copied files - not loading"
	    return
	}

	$self open_messages
	$self UseApshell

	if {!$(force) && !$bootlooping} {
	    if {[catch {
		if {$nowl } {
		    set ver [$self nvram get os_version]
		} else {
		    set ver [$self -t 5 wl ver]
		}
		set oldinfo "$info\n$ver"
		set newinfo [$self relay rexec "cat \"$idfile\""]
	    } ret]} {
		UTF::Message WARN $options(-name) $ret
	    } elseif {$oldinfo eq $newinfo} {
		UTF::Message LOG $options(-name) \
		    "Skipping reload of same image"
		if {$(erase)} {
		    # Still need to restore, etc
		    $self reboot erase {*}$(nvram)
		}
		# reinit, since we turned off ushd for the version
		# check.
		$base configure -initialized 0
		regexp {version (.*)} $ver - ver
		return "$ver (skipped)"
	    }
	}
	catch {
	    $self relay rexec "rm -f \"$idfile\""
	}

	if {$(web) eq "1"} {
	    # If we're pointed at a compressed image, uncompress before copying
	    if {[file extension $file] eq ".gz"} {
		set newfile "/tmp/[file tail [file rootname $file]]_ungz_$::tcl_platform(user)"
		UTF::Message LOG $options(-name) "gunzip -c $file > $newfile"
		exec gunzip -c $file > $newfile
		set file $newfile
		unset newfile
	    }
	    $self http upgrade $file
	    if {[info exists ::UTF::panic] &&
		[regexp {Bad trx} $::UTF::panic]} {
		# Web load failed.  Don't update $loaded.  Return
		# current OS so the reader knows what is really
		# being tested.  Allow panic to report failure.
		return [$self nvram get os_version]
	    }
	    $self reboot
	} else {
	    if {[regexp {code\.bin(?:.gz)?$} $file]} {
		# Linksys OS image.
		set f "code.bin"
	    } else {
		set f /tmp/$options(-trx).trx
	    }
	    UTF::BuildFile::copyto [$self relay] $file $f

	    if {[regexp -nocase {netgear} [$self cget -model]]} {
		# Netgear's CFE tries to tftp pull an image on every
		# boot.  The relay must have tftp-server installed.
		# The relay must be 192.168.1.2
		$self relay cp -p $f /var/lib/tftpboot/vmlinuz
	    }

	    # Don't bother power cycling if the reason we're not
	    # responding is that we're waiting for a download
	    set stuckreading 0
	    if {$(coldinstall) || $options(-coldboot) ||
		(!$(force) && !$bootlooping &&
		 (([catch {$self rexec -t 10 :} ret] &&
		   $ret ne "Vx prompt detected!" &&
		   !$stuckreading)))} {
		$self power cycle
	    }
	    if {[info exists ::UTF::panic] && [regexp {power cycle} $::UTF::panic]} {
		UTF::Message WARN $options(-name) "Ignoring pre-load power cycle"
		unset ::UTF::panic
	    }
	    set cmd [list $self -t 480 -T 600 load $f -quiet]
	    if {$(web) eq 2} {
		lappend cmd -web
	    }
	    if {$(erase)} {
		set lip [$self cget -lan_ip]
		if {[regsub {\.\d+$} $lip {} net]} {
		    if {![catch {$self lan ipaddr} ip] &&
			[string match "$net.*" $ip]} {
			UTF::Message INFO $options(-name) \
			    "Lan already on default net"
		    } elseif {[string match "$net.*" [$self lan cget -ipaddr]]} {
			$self lan ifconfig [$self lan cget -ipaddr]
		    } else {
			$self lan ifconfig "$net.50"
		    }
		} else {
		    UTF::Message WARN $options(-name) \
			"Unable to parse ip addr: $lip"
		}
		# Flash update can take more than 7 minutes!
		lappend cmd -erase
	    }
	    {*}$cmd $(nvram)
	}
	set bootlooping 0

	# Give the console a little time to flush messages
	UTF::Sleep $options(-bootwait)

	# Request reinit
	$base configure -initialized 0
	set Bridge(stale) 1
	set loaded $file

	if {$options(-postboot) ne ""} {
	    $self rexec [string trim $options(-postboot)]
	}
	# Boot completed - enable reset checks.
	set expectBoot 0
	if {$nowl} {
	    set ver [$self nvram get os_version]
	} else {
	    set ver [$self wl ver]
	}
	$self relay rexec "echo \"$info\n$ver\" > \"$idfile\""
	regexp {version (.*)} $ver - ver
	set ver
    }

    UTF::doc {
	# [call [arg host] [method unload]]

	# Unload the router image.
	# Not implemented.
    }

    method unload {} {
	# Not supported
	set loaded ""
    }


    UTF::doc {
	# [call [arg host] [method reload]]

	# Reload the wl driver on the router.  Used for debugging and
	# resetting counters.  Only works with monolithic driver for
	# now.
    }

    method reload {} {
	if {[regexp {usbap|dhdap} [$self cget -brand]]} {
	    # Non-nic routers need more work
	    $self reboot
	} else {
	    $self rmmod wl
	    $self insmod wl
	    $self restart
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
	# Loop, since Router ping doesn't handle "count" itself
	for {set c 0} {$c < $(c)} {incr c} {
	    if {[regexp {linux26|linux-2\.6} [$self cget -brand]]} {
		# Linux 26 builds have a more capable ping
		catch {$self rexec -t 1 ping -c 1 -s $(s) $target} ret
	    } else {
		catch {$self rexec -t 1 ping $target} ret
	    }
	    if {[regexp {is alive|1 packets received} $ret]} {
		return
	    }
	}
	error "ping failed"
    }

    UTF::doc {
	# [call [arg host] [method copyfrom] [arg src] [arg dest]]

	# Copy file [arg src] on [arg host] to local [arg dest].  Auto
	# compression is not supported.
    }
    method copyfrom {src dst} {

	if {[$self cget -ssh] ne "ush"} {
	    if {!$options(-ush)} {
		error "copyfrom requires ushd to be enabled"
	    }
	    $self install_ushd
	    if {[$self cget -ssh] ne "ush"} {
		error "ush failed - copyfrom cannot continue"
	    }
	}

	# Simple version - reads entire file into memory
	#set in [$self rexec -s -binary cat $src]
	#set out [open $dst w]
	#fconfigure $out -translation binary
	#puts -nonewline $out $in
	#close $out

	# Scalable version - process file in 10k chunks
	set in [$self rpopen -2 cat $src]
	set out [open $dst w]
	fconfigure $in -translation binary
	fconfigure $out -translation binary
	try {
	    while {![eof $in]} {
		puts -nonewline $out [read $in 10240]
	    }
	    close $in
	} finally {
	    close $out
	}
    }

    UTF::doc {
	# [call [arg host] [method copyto] [arg src] [arg dest]]

	# Copy local file [arg src] to [arg dest] on [arg host].  If
	# src ends in .gz, but dest doesn't then the file will be
	# uncompressed automatically.
    }
    method copyto {src dst} {
	# Handle the uncompression at the local end because the router
	# doesn't have gunzip and do it inline to avoid permissions
	# problems.

	if {[$self cget -ssh] ne "ush"} {
	    if {!$options(-ush)} {
		error "copyto requires ushd to be enabled"
	    }
	    $self install_ushd
	    if {[$self cget -ssh] ne "ush"} {
		error "ush failed - copyto cannot continue"
	    }
	}

	if {[file extension $src] eq ".gz" &&
	    [file extension $dst] ne ".gz"} {
	    set f [open "|gunzip -c $src"]
	    UTF::Message LOG $options(-name) "gunzip -c $src \>@$f"
	    $self rexec "cat\>$dst" <@$f
	    close $f
	} else {
	    $self rexec "cat\>$dst" <$src
	}
	if {[file executable $src]} {
	    $self chmod +x $dst
	}
    }

    UTF::doc {
	# [call [arg host] [method macaddr] [arg dev]]

	# Returns the MAC address for [arg dev]
    }
    method macaddr {dev} {
	if {[regexp {HWaddr\s+(\S+)} [$self rexec ifconfig $dev] - mac]} {
	    return $mac
	} else {
	    error "No MAC address found"
	}
    }

    UTF::doc {
	# [call [arg host] [method ipaddr] [arg dev]]

	# Return IP address of device [arg dev] or Error if device is
	# down.  If device is a member of a bridge, the bridge
	# interface is queried instead.
    }

    method ipaddr {dev} {
	if {$Bridge(stale)} {
	    $self probe
	}
	if {[regexp {netbsd} $options(-trx)]} {
	    # BSD uses the wlname
	    set nvname [$self wlname $dev]
	} else {
	    # Linux uses the dev name
	    set nvname $dev
	}

	if {[info exists Bridge($nvname)]} {
	    set dev $Bridge($nvname)
	}
	# Support both linux and BSD ifconfig
	if {[regexp {inet (?:addr:)?([0-9.]+)} [$self -s ifconfig $dev] \
		 - addr]} {
	    return $addr
	} else {
	    error "No IP address available"
	}
    }

    UTF::doc {
	# [call [arg host] [method ifconfig] [arg {args ...}]]

	# Run ifconfig on the host
    }

    # Run native ifconfig command
    method ifconfig {args} {
	$self rexec /sbin/ifconfig {*}$args
    }

    UTF::doc {
	# [call [arg host] [method pci] [arg dev]]

	# Return PCI bus.dev numbers for the device
    }

    method pci {dev} {
	set ret [uplevel 1 {$self revinfo}]
	regexp -line {^vendorid\s+0x(\w+)$} $ret - vendorid
	regexp -line {^deviceid\s+0x(\w+)$} $ret - deviceid
	set ret [$self rexec grep "$vendorid$deviceid" \
		     /proc/bus/pci/devices]
	regexp -line {^(\d\d)(\d\d)} $ret - bus devfn
	set slot [expr {("0x$devfn">>3)&0x1f}]
	set bus [expr {$bus+0}]
	return "$bus.$slot"
    }

    UTF::doc {
	# [call [arg host] [method probe]]

	# Attempt to auto-discover properties of [arg host].
	# Records bridge/interface herarchy for later use.
    }

    method probe {} {
	# HSIC doesn't use nvram for bridges
	if {![regexp {hsic} [$self cget -brand]]} {
	    # Build bridge array based on nvram since brctl is less
	    # portable and may depend on STP.
	    foreach {lan br} {lan br0 lan1 br1} {
		if {[catch {$self nvram get ${lan}_ifnames} iflist($lan)]} {
		    UTF::Message ERROR [$self cget -name] "probe: $iflist($lan)"
		    return
		}
	    }
	    foreach {lan br} {lan br0 lan1 br1} {
		foreach if $iflist($lan) {
		    set Bridge($if) $br
		    set Bridge(${if},lan) $lan
		}
	    }
	    set Bridge(stale) 0
	}
    }

    UTF::doc {
	# [call [arg host] [cmd getnvram]]

	# Fetch NVRAM parameters from Router.  The results are
	# returned as list suitable for assigning to an array with
	# array set.
    }

    method getnvram {args} {
	set nvram [split [$self rexec -s nvram show 2>/dev/null] "\n"]
	foreach nv $nvram {
	    if {[regexp {([^=]+)=([^\r]*)\r?} $nv -> key value]} {
		lappend NVram $key $value
	    } elseif {[regexp {^$|^size:} $nv]} {
		continue
	    } else {
		error "Bad nvram entry: $nv"
		puts stderr "Bad nvram entry: $nv"
	    }
	}
	return $NVram
    }

    typevariable msgfile
    variable oops ""

    # Semaphore to prevent power cycle storms
    variable processingpowercycle false
    variable processingDongleError false
    variable rtecapture
    variable interruptedbydongle ""

    # Internal callback for fileevent below
    method getdata {request} {
	set fd $msgfile($request)
	if {![eof $fd]} {
	    set msg [gets $fd]

	    # Strip out bad data from HSIC showcons
	    regsub -all {s\010\020} $msg {} msg

	    # Strip out useless timestamps
	    regsub {^\[\s*\d+\.\d{6}\] } $msg {} msg
	    regsub {^\d{6}\.\d{3} } $msg {} msg

	    if {[regexp {Invalid boot block on disk} $msg]} {
		set bootlooping 1
		$self warn $msg
		return
	    }

	    if {$options(-msgcallback) ne ""} {
		# Specialist host failures.  Returns true if no
		# further processing is required.
		if {[{*}$options(-msgcallback) $msg]} {
		    return
		}
	    } elseif {[regexp {dhdap} [$self cget -brand]]} {
		# Pass CONSOLE messages on to rte handler CONSOLE
		# messages frequently interrupt host messages, so
		# collect partial messages and report them after the
		# rte messages have been handled.
		if {[regexp {(.*)CONSOLE: (.*)} $msg - pre post]} {
		    append interruptedbydongle $pre
		    if {[info exists rtecapture]} {
			append rtecapture "\n$post"
		    }
		    $self rte_consolemsg $post
		    return
		} elseif {$interruptedbydongle ne ""} {
		    UTF::Message LOG $options(-name) $interruptedbydongle
		    set interruptedbydongle ""
		}
	    }

	    if {[regexp {assert_type|Upload and compare succeeded} $msg]} {
		# This is not an assert, so log and skip other checks
		UTF::Message LOG $options(-name) $msg
	    } elseif {[regexp {Detected firmware trap/assert|input overrun} $msg]} {
		# Downgrade these below main trap analysis
		$self warn $msg
	    } elseif {[regexp {Trap/Assert on interface (\S+)} \
			   $msg - ifname]} {
		# Downgrade these below main trap analysis
		$self warn $msg
		if {!$processingDongleError} {
		    # Console trap hasn't appeared yet - request it
		    if {![catch {$self dhd -i $ifname consoledump} ret]} {
			regsub -all -line {^\d{6}.\d{3} } $ret {} ret
			set trap [UTF::RTE::parse_traplog $self $ret]
			# If there's already an assert, don't overwrite it
			if {![info exists ::UTF::panic] ||
			    ![regexp -nocase {assert|trap|suspend_mac|microcode watchdog|: out of memory, malloced 0 bytes} $::UTF::panic] ||
			    [regexp -nocase {Dongle assert file} $::UTF::panic]} {
			    set ::UTF::panic $trap
			}
		    }
		}
	    } elseif {!$expectBoot && !$bootlooping &&
		      [regexp {Decompressing\.\.\.done} $msg]} {
		$self warn "$msg: ***Unexpected reset***"
	    } elseif {[regexp {Kernel panic:.*|assert|Bad trx header|insmod: unresolved symbol |RPC call .*return wait err|Out of Memory: Killed process|panic: TLB:|Stopped at |Stopped in |Eeek! } $msg] ||
		      ($oops ne "" && [regexp {Code: |rebooting} $msg])} {
		$self worry $msg
		set expectBoot 1

		if {[$base cget -initialized] eq 1} {
		    # If we were connected normally, signal to
		    # reestabish connection, otherwise leave it alone.
		    $base configure -initialized 0
		}
		if {$oops ne ""} {
		    if {[catch {$self gdboops} ret]} {
			UTF::Message WARNING $options(-name) $ret
		    }
		    set oops ""
		}
	    } elseif {[regexp {Reading ::} $msg]} {
		set stuckreading 1
		UTF::Message LOG $options(-name) $msg
	    } elseif {[regexp {proc: table is full} $msg]} {
		UTF::Message LOG $options(-name) $msg
		$self rexec ps
	    } elseif {[regexp {SQUASHFS error: Unable to read page} $msg]} {
		# power cycle disabled for SWWLAN-50865, to see if the
		# problem is recoverable
		$self worry $msg
	    } elseif {[regexp {swapper: page allocation failure|oom-killer: } $msg]} {
		# Generally not recoverable - power cycle
		$self worry $msg
		if {!$processingpowercycle} {
		    set processingpowercycle true
		    if {[catch {$self power cycle} ret]} {
			UTF::Message WARNING $options(-name) $ret
		    }
		    # ignore these messages for 30 seconds
		    after 30000 set [myvar processingpowercycle] false
		}
	    } elseif {[$base common_getdata $msg]} {
		# Handled
	    } elseif {[regexp {bcn inactivity detected} $msg]} {
		$self worry $msg
	    } else {
		UTF::Message LOG $options(-name) $msg
	    }

	    # Additional handling for oops.  Needs to be called even
	    # if common_getdata fired.
	    if {[regexp \
		     {Oops in |Oops\[\#|Unhandled kernel|Unable to handle|traps\.c|bug detected} $msg]} {
		UTF::Message WARNING $options(-name) "Collecting oops..."
		set oops "$msg\n"
	    } elseif {$oops ne "" &&
		      [regexp {module:\s+|epc\s+|ra\s+|\[<([\da-f]+)>\]} $msg]} {
		append oops "$msg\n"
	    }
	} else {
	    # Leave non-blocking for safer close.  Any error messages
	    # should have been collected earlier.
	    close $fd
	    UTF::Message LOG $options(-name) "Log closed $fd"
	    unset msgfile($request)
	}
    }

    UTF::doc {
	# [call [arg host] [cmd gdboops] [lb][file file][rb]
	#                                [lb][file src][rb]]

	# Process Oops log with gdb and attempt to report function and
	# line number where the router crashed.  When called by hand,
	# [file file] should be provided, as a file containing the
	# Oops log.  [file src] may also be provided as the location
	# of the corresponding source tree.  [file src] defaults to
	# the auto detected source tree from the last image load.
	# The Oops log and source tree will be made available internally
	# when called as part of the background message handling.
    }
    method gdboops {{file ""} {src ""}} {

	# copy oops locally, then clear the object oops since we may
	# get nested by the io calls below.
	set oopscpy $oops
	set oops ""

	if {$file ne ""} {
	    set fd [open $file]
	    set oopscpy [read $fd]
	    close $fd
	}
	if {![info exists oops_src]} {
	    regsub {/} $options(-name) {.} name
	    set oops_src [$self relay -noinit -s -q ls -l "/tmp/$name-src.lnk"]
	    regsub {.*-> } $oops_src {} oops_src
	}
	if {![file isdirectory $oops_src]} {
	    error "Source code not found"
	}
	set main [file dir $oops_src]
	if {[regexp {linux-.*-arm} [$self cget -brand]]} {
	    set linux "$main/components/opensource/linux/linux-2.6.36"
	    # Latest 32bit version.  Works on 64bit in compat mode.
	    set gdb "/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.0/bin/arm-eabi-gdb"
	    set ko "ko"
	} elseif {[regexp {linux26} [$self cget -brand]]} {
	    if {[file isdirectory "$main/components"]} {
		set linux "$main/components/opensource/linux/linux-2.6"]
	    } else {
		set linux "$oops_src/linux/linux-2.6"
	    }
	    set gdb "$::UTF::projtools/linux/bin/mipsel-linux-gdb"
	    set ko "ko"

	} else {
	    set linux "$oops_src/linux/linux"
	    set gdb "$::UTF::projtools/linux/bin/mipsel-linux-gdb"
	    set ko "o"
	}
	set oopscmd "set width 0\ncd $linux\nfile vmlinux_dbgsym\n"
	set oopscmd2 ""
	foreach msg [split $oopscpy "\n"] {
	    if {[regexp {\mmodule:\s+(\S+)\s+([0-9a-f]+)} $msg - m a]} {
		set sim "$m/${m}_dbgsym.$ko"
		if {$m eq "wl_high"} {
		    set sim "wl/$sim"
		}
		append oopscmd \
		    "add-symbol-file drivers/net/$sim 0x$a+0x60\n"
	    } elseif {[regexp -line {(?:^| )pc : \[<([\da-f]+)>\]\s+lr : \[<([\da-f]+)>\]} $msg - epc lr]} {
		append oopscmd2 "l *0x$epc\nl *0x$lr\n"
	    } elseif {[regexp -line {(?:^| )(?:epc|ra)\s+:\s+([\da-f]+)} $msg - t]} {
		append oopscmd2 "l *0x$t\n"
	    } elseif {[regexp -line {(?:^| )\[<([\da-f]+)>\]} $msg - t]} {
		append oopscmd2 "info line *0x$t\n"
	    }
	}
	append oopscmd $oopscmd2
	set f [open "/tmp/oops.cmd" w]
	catch {file attributes "/tmp/oops.cmd" -permissions 00666}
	puts $f $oopscmd
	UTF::Message LOG $options(-name) $oopscmd
	close $f
	set cmd "$gdb -q -n </tmp/oops.cmd"
	set oops ""
	UTF::Message LOG $options(-name) $cmd
	# Note: deliberately using blocking exec, otherwise slow gdb
	# output will end up in the middle of next boot
	catch {exec {*}$cmd} ret
	regsub -line -all {[\.\w_/]*src/} $ret {src/} ret; # trim paths
	regsub -line -all \
	    {.*(add symbol table|.text_addr =|Reading symbols from|not from terminal).*\n} $ret {} ret
	UTF::Message LOG $options(-name) $ret

	# Replace Code error with function and line, if found
	# If gdb doesn't help, but we found a EPC, use that.
	if {(![info exists ::UTF::panic] ||
	     [regexp -nocase {Code|Kernel panic} $::UTF::panic]) &&
	    ([regexp -line {0x[[:xdigit:]]+ is in (.*\))} $ret - fandl] ||
	     [regexp -line {PC is at (.*)} $oopscpy - fandl] ||
	     [regexp -line {epc : [\da-f]+ (.*)} $oopscpy - fandl])} {
	    set ::UTF::panic "Oops in $fandl"
	}
    }

    UTF::doc {
	# [call [arg host] [method open_messages] [lb][arg file][rb]]

	# Open system message log on [arg host] and log new messages
	# to the UTF log.  [method open_messages] will return
	# immediately, while messages will continue to be logged in
	# the background until [method close_messages] is called.
	# [arg file] defaults to [file /var/log/messages].  Multiple
	# loggers can be opened, so long as they log different [arg
	# file]s.  Attempts to log the same [arg file] multiple times
	# will be logged and ignored.
    }

    method open_messages { {file ""} } {
	if {$file == ""} {
	    set file $options(-console)
	}
	set id "[$self cget -lanpeer]:[$self cget -lan_ip]"
	if {[info exists msgfile($id:$file)]} {
	    return
	}
	UTF::Message LOG $options(-name) "Open $file"
	if {[catch {$self serialrelay socket $file} ret]} {
	    $self worry "$file: $ret"
	    return
	}
	set msgfile($id:$file) $ret
	if {[regexp {shell} [$base cget -ssh]]} {
	    $base configure -lan_ip $file
	}
	fconfigure $msgfile($id:$file) -blocking 0 -buffering line
	puts $msgfile($id:$file) ""; # Trigger NPC telnet reconnect
	fileevent $msgfile($id:$file) readable [mymethod getdata $id:$file]
	UTF::Sleep 1
	return $ret
    }

    UTF::doc {
	# [call [arg host] [method close_messages] [lb][arg file][rb]]

	# Close a system message log previously opened by [method
	# open_messages].  An error will be reported if [arg file]
	# does not match that of a previous invocation of [method
	# open_messages].
    }
    method close_messages { {file ""} } {
	if {$file == ""} {
	    set file $options(-console)
	}
	set id "[$self cget -lanpeer]:[$self cget -lan_ip]"
	if {[info exists msgfile($id:$file)] &&
	    [file channels $msgfile($id:$file)] ne ""} {
	    UTF::Message LOG $options(-name) "Close $file $msgfile($id:$file)"
	    if {[set pid [pid $msgfile($id:$file)]] ne ""} {
		# Processes to wait for
		catch {exec kill {*}$pid} ret
		UTF::Message LOG $options(-name) $ret
	    }
	    # Leave non-blocking for safer close.  Any error messages
	    # should have been collected earlier.
	    close $msgfile($id:$file)
	    unset msgfile($id:$file)
	    $base configure -lan_ip $options(-lan_ip)
	    $base configure -initialized 0
	} else {
	    UTF::Message LOG $options(-name) "Close $file (not open)"
	}
    }

    UTF::doc {
	# [call [arg host] [method meminfo] [lb][arg sdram_init][rb]]

	# Decode sdram_init values into human-readable strings.  If
	# [arg sdram_init] is not specified, [method meminfo] will
	# query the router's nvram for the current value.
    }
    method meminfo {{sdram_init ""}} {
	if {$sdram_init eq ""} {
	    set sdram_init [$self rexec nvram get sdram_init]
	}
	if {[expr {$sdram_init & 0x1}]} {
	    set sd "DDR"
	} else {
	    set sd "SDR"
	}
	if {[expr {$sdram_init & 0x2}]} {
	    set width 16
	} else {
	    set width 32
	}
	# Use this for columns, if we need it:
	# set cols [expr {2 << ($sdram_init  & 0x18)}]
	return "$sd$width"
    }

    method {route add} {net mask gw} {
	$self rexec route add -net $net netmask $mask gw $gw
    }

    method {route del} {net mask gw} {
	$self rexec route del -net $net netmask $mask gw $gw
    }

    method {route replace} {net mask gw} {
	catch {$self route del $net $mask $gw}
	$self route add $net $mask $gw
    }


    method fetch_crashreport {} {
	if {[regexp {dhdap} [$self cget -brand]] &&
	    [info exists ::UTF::Logdir]} {
	    if {![catch {$self test -e /media/nand/config.tgz}]} {
		if {[catch {
		    set f [exec mktemp $UTF::Logdir/config_XXXXX].tgz
		    $self copyfrom /media/nand/config.tgz $f
		    file attributes $f -permissions go+r
		    UTF::Message WARN $options(-name) \
			"config.tgz upload: [UTF::LogURL $f]"
		    $self rm /media/nand/config.tgz
		} ret]} {
		    UTF::Message WARN $options(-name) $ret
		}
	    }
	}
    }

    method init {} {
	$self open_messages
	if {$options(-rteconsole) ne ""} {
	    $self open_rte_messages
	}
	if {$options(-utelnet)} {
	    $self install_telnet
	} elseif {$options(-ush)} {
	    $self install_ushd
	    $self fetch_crashreport
	}
	set expectBoot 0
    }

    method deinit {} {
	if {$options(-rteconsole) ne ""} {
	    $self close_rte_messages
	}
	$self close_messages
	$self configure -initialized 0
    }

    method install_telnet {} {
	$base configure -lan_ip [$self cget -lan_ip]
	# clear init 2 state to re-enable auto recovery.
	$base configure -initialized 1
	if {[catch {$self -t 1 -n :}]} {
	    $self apshell {utelnetd\&:\;}
	    UTF::Sleep 1
	    $self -n :
	}
    }

    method install_ushd {args} {
	UTF::Getopts {
	    {force "Force recompile and reinstall"}
	}
	if {!$options(-ush)} {
	    # Abort in case caller missed a check.
	    error "ush disabled"
	}

	set ip [$self cget -lan_ip]
	set ssh "ush"
	set tools $::UTF::projtools/linux/hndtools-mipsel

	if {[regexp {netbsd} [$self cget -brand]]} {
	    # configure
	    $self configure -ssh $ssh
	    $base configure -lan_ip $ip
	    # clear init 2 state to re-enable auto recovery.
	    $base configure -initialized 1
	    return
	}
	$self UseApshell

	if {[regexp {arm} [$self cget -brand]]} {
	    set gcc 	$::UTF::projtools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc
	    set ushd /tmp/ushd_northstar_$::tcl_platform(user)

	} elseif {[regexp {linux26} [$self cget -brand]]} {
	    set gcc ${tools}-linux-uclibc-4.2.3/bin/mipsel-linux-uclibc-gcc
	    set ushd /tmp/ushd_mips26_$::tcl_platform(user)

	} else {
	    set gcc ${tools}-uclibc-3.2.3/bin/mipsel-uclibc-gcc
	    set ushd /tmp/ushd_mips_$::tcl_platform(user)
	}
	set ushd_c $UTF::unittest/src/ushd.c

	$self open_messages

	set epi_ttcp [$self relay cget -epi_ttcp]

	if {[catch {$self relay ping $ip} ret]} {
	    UTF::Message WARN $options(-name) $ret
	    # Not pingable - make sure the boot has completed
	    if {[catch {$self rexec exit} ret] && \
		    [$self cget -power] ne "" && \
		    [regexp {Timeout|child killed} $ret]} {
		UTF::Message WARN $options(-name) \
		    "no response, try power cycle"
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic "no response, try power cycle"
		}
		$self power cycle
		# Check for a truncated oops log
		if {$oops ne ""} {
		    if {[catch {$self gdboops} ret]} {
			UTF::Message WARNING $options(-name) $ret
		    }
		    set oops ""
		}
		UTF::Sleep 20
	    } else {
		UTF::Sleep 5
	    }
	    if {[catch {$self relay ping -c 10 $ip}]} {
		if {$options(-rebootifnotpingable)} {
		    UTF::Message WARN $options(-name) \
			"not pingable and -rebootifnotpingable set: rebooting"
		    $self reboot
		    error \
			"not pingable and -rebootifnotpingable set: rebooting"
		} else {
		    UTF::Message WARN $options(-name) \
			"not pingable, reverting to apshell"
		    return
		}
	    }
	}

	# Test first
	if {$(force) ||
	    [catch {$self relay -t 5 $ssh $ip :}]} {
	    # Recompile the server, if neccessary
	    if {$(force) ||
		[catch {file mtime $ushd} ret] ||
		[file mtime $ushd_c] > $ret} {
		UTF::Message LOG $options(-name) "$gcc -o $ushd $ushd_c"
		if {[info exists LDLP]} {
		    set ::env(LD_LIBRARY_PATH) $LDLP
		}
		exec $gcc -o $ushd $ushd_c
		if {[info exists LDLP]} {
		    unset ::env(LD_LIBRARY_PATH)
		}
	    }

	    # Kill off any existing service, also exit in case we
	    # interrupted the boot sequence
	    catch {$self rexec -t 2 {killall ushd}} ret
	    if {[regexp {prompt detected} $ret]} {
		error $ret
	    }
	    catch {$self rexec -t 2 exit} ret

	    # Use epi_ttcp to copy the rshd executable to the router
	    set fd [$self rpopen {epi_ttcp -r -w5000>/tmp/ushd}]
	    # Wait for a response
	    # UTF::Message DBG $options(-name) [gets $fd]

	    # Don't wait for response, just use fixed timer.  This is
	    # because some router/consoles may lose characters and
	    # never report back
	    UTF::Sleep 1.5

	    # Start sender
	    $self relay rexec $epi_ttcp -t $ip < $ushd

	    # Collect any bg messages.  We can discard them because
	    # they are already logged, but we still need to collect
	    # them or the close may fail.
	    set msg [read $fd]
	    UTF::Message LOG $options(-name) $msg

	    if {[catch {close $fd} ret]} {
#		if {[regexp {/tmp/ushd: not found} $msg]} {
#		    UTF::Message INFO $options(-name) \
#			"ushd won't start - try telnet"
#		    #return [$self install_telnet]
#		} else {
		    UTF::Message WARN $options(-name) $ret
		    #error $ret
#		}
	    }
	    # Try starting ushd even if copy appeared to fail, since
	    # console data loss may falsify the copy status.
	    $self rexec -t 10 "chmod +x /tmp/ushd;/tmp/ushd -d"

	    if {![catch {$self test -d /media/nand}]} {
		# Stash a copy of ushd in nand to run on boot.
		# Harmless if not supported.
		$self copyto $UTF::unittest/etc/router.rc.local /media/nand/rc.local
		$self cp /tmp/ushd /media/nand/ushd
		$self nvram set rclocal=/media/nand/rc.local
		$self nvram commit
	    }
	}

	# configure
	$self configure -ssh $ssh
	$base configure -lan_ip $ip
	# clear init 2 state to re-enable auto recovery.
	$base configure -initialized 1

    }

    UTF::doc {
	# [call [arg host] [method boardname]]

	# Query device boardtype and translate into common name via
	# bcmdevs.h
    }

    method boardname {} {
	global ::UTF::sromdefs

	set url "http://$::UTF::WLANSVN/groups/tcl/trunk/src/tools/47xxtcl"

	set boardrev [$self nvram get boardrev]
	regsub {^0x1(...)$} $boardrev {P\1} boardrev

	# Check for a vendor name
	if {[set boardtype [$self nvram get boot_hw_model]] ne ""} {
	    if {$boardrev != ""} {
		return "$boardtype $boardrev"
	    } else {
		return $boardtype
	    }
	}

	# Otherwise try looking up boardtype.
	set boardtype [format "0x%04x" [$self nvram get boardtype]]

	UTF::Message LOG $options(-name) \
	    "Try boardtype $boardtype in sromdefs.tcl"
	if {![info exists sromdefs]} {
	    set sromdefs [localhost -s svn cat $url/sromdefs.tcl]
	}
	if {[regexp -line -nocase \
		 "^set def\\((?:(?:bcm)?9)?(.*)_ssid\\)\\s+$boardtype$" \
		 $sromdefs line name]} {
	    UTF::Message LOG $options(-name) "Found: $line"
	    return "$name $boardrev"
	}
	# Failsafe - just report boardtype
	return "$boardtype $boardrev"
    }

    UTF::doc {
	# [call [arg host] [method epidiag] [arg script]]

	# Run epidiag script [arg script] on [arg host]'s [option
	# -epirelay] host

	# If [option -epirelay] is not set, an error will be returned.
    }

    method epidiag {SCRIPT} {
	if {$options(-epirelay) eq ""} {
	    error "-epirelay not set"
	}
	$options(-epirelay) epidiag $SCRIPT
    }


    UTF::doc {
	# [call [arg host] [method wlname] [arg dev]]

	# Return wl<n> name corresponding to this device, eg eth2 may
	# correspond to wl0
    }
    # Cache wl names
    variable wlc -array {}
    method wlname {dev} {
	if {![info exists wlc($dev)]} {
	    if {[regexp {^wl} $dev]} {
		set wlc($dev) $dev
	    } elseif {[regexp {^b(wl.*)} $dev - wlname]} {
		set wlc($dev) $wlname
	    } else {
		if {[catch {$self wl -i $dev ver} ret]} {
		    if {[regexp {Not Enough Resources} $ret]} {
			# Fallback for reduced memory systems
			set wlc($dev) "wl0"
		    }
		    error $ret
		}
		if {[regexp -line {^(wl\d+):} $ret - wlname]} {
		    set wlc($dev) $wlname
		} else {
		    error "Unable to determine wl driver name for device $dev"
		}
	    }
	}
	return $wlc($dev)
    }


    UTF::doc {
	# [call [arg host] [method brname] [arg dev] [lb][arg -lan][rb]]

	# Return br<n> name for the bridge to which this interface is
	# configured to be connected.  If [arg -lan] is specified,
	# return "lan" or "lan1" instead.  Returns an error if the
	# device is not configured for any bridge.  Note this is
	# determined by querying nvram, so does not take into account
	# any manual changes made using brctl.
    }
    method brname {dev args} {
	UTF::Getopts {
	    {lan "Return lan<n> name, instead of br<n>"}
	}
	if {$Bridge(stale)} {
	    $self probe
	}
	if {[regexp {netbsd} $options(-trx)]} {
	    # BSD uses the wlname
	    set nvname [$self wlname $dev]
	} else {
	    # Linux uses the dev name
	    set nvname $dev
	}
	if {$(lan)} {
	    append nvname ",lan"
	}
	if {[info exists Bridge($nvname)]} {
	    return $Bridge($nvname)
	} else {
	    error "$dev not attached to a bridge"
	}
    }

    UTF::doc {
	# [call [arg name] [method {igmp_querier enable}]
	# [lb][arg -group] [arg group][rb]
	# [lb][option -interval] [arg interval][rb]
        # [lb][option -silent][rb]]

	# Start an IGMP Querier per the Router
    }
    #   Enable an IGMP Querier.  Multiple queriers are supported.
    #   Stale queriers will be removed on first invocation of enable
    #
    #   All Hosts query of 224.0.0.1 Reports from clients will be a list
    #   of all groups of interest.
    #   Group specific, e.g. 239.0.0.1 Reports from clients will be only
    #   if interested in that specific group.
    #
    method {igmp_querier enable} {args} {
	UTF::Getopts {
	    {interval.arg "30" "Querier interval in seconds"}
	    {group.arg "224.0.0.1" "Querier group"}
	    {silent "don't log querier messages"}
	}
	# If the querier for this group isn't already instantiated
	# then do so now.
	if {![info exists igmpq($(group))]} {
	    # If this is the first invocation kill *all* stale
	    # queriers as part of cleanup
	    if {![array exists igmpq]} {
		$self lan rexec -x "pkill -TERM igmp_querier"
	    }
	    # Instantiate the querier
	    package require UTF::IGMPQuerier
	    set igmpq($(group)) [UTF::IGMPQuerier create %AUTO% -ap $self -reportinterval $(interval) -querygroup $(group) -silent $(silent)]
	}
	# Call start.  No need to worry about starting an active querier
	$igmpq($(group)) start
    }
    UTF::doc {
	# [call [arg name] [method {igmp_querier disable}]
	# [lb][arg -group] [arg group][rb]]

	# Disable an active IGMP Querier per the Router
    }
    method {igmp_querier disable} {args} {
	UTF::Getopts {
	    {group.arg "" "Querier group"}
	}
	# Return if no action is needed
	if {$(group) ne "" && ![info exists igmpq($(group))]} {
	    return
	}
	if {$(group) eq "" && ([catch {array get igmpq} x] || $x eq "")} {
	    return
	}
	# If the group is unspecified then stop
	# all active queriers else stop
	# the querier for the group specified
	if {$(group) eq ""} {
	    foreach g [array names igmpq] {
		$igmpq($g) stop
	    }
	    array set igmpq {}
	} else {
	    $igmpq($(group)) stop
	    unset igmpq($(group))
	}
    }
    UTF::doc {
	# [call [arg name] [method {igmp_querier send}]
	# [lb][arg -group] [arg group][rb]
	# [lb][arg -noblock][rb]]

	# Force an IGMP Querier to send a Query immediately
    }
    method {igmp_querier send} {args} {
	UTF::Getopts {
	    {group.arg "" "Querier group"}
	    {noblock "don't block on IGMP Query being sent"}
	}
	# Make sure a querier exists per the request
	if {$(group) ne "" && ![info exists igmpq($(group))]} {
	    error "IGMP querier not enabled for group $(group)"
	}
	if {$(group) eq "" && ([catch {array get igmpq} x] || $x eq "")} {
	    error "No IGMP queriers are enabled"
	}
	if {$(noblock)} {
	    set cmd "send -noblock"
	} else {
	    set cmd "send"
	}
	# If the group is unspecified then post
	# to all active queriers else post to only
	# the querier specified
	if {$(group) eq ""} {
	    foreach g [array names igmpq] {
		$igmpq($g) {*}$cmd
	    }
	} else {
	    $igmpq($(group)) {*}$cmd
	}
    }

    # Dongle methods

    method hndrte_src {} {
	if {![info exists hndrte_src]} {
	    regsub {/} $options(-name) {.} name
	    if {[catch {$self relay rexec -noinit -s -q \
			    ls -l /tmp/$name-hndrte-src.lnk} hndrte_src]} {
		regsub {/dongle/rte/.*} [$self hndrte_exe] {} hndrte_src
	    } else {
		regsub {.*-> } $hndrte_src {} hndrte_src
	    }
	}
	set hndrte_src
    }

    method hndrte_exe {} {
	if {![info exists hndrte_exe]} {
	    regsub {/} $options(-name) {.} name
	    set hndrte_exe [$self relay rexec -noinit -s -q \
				ls -l /tmp/$name-hndrte-exe.lnk]
	    regsub {.*-> } $hndrte_exe {} hndrte_exe
	}
	set hndrte_exe
    }

    method findassert {file line} {
	UTF::RTE::findassert $options(-name) $file $line [$self hndrte_src]
    }

    method findtrap {trap stack} {
	UTF::RTE::findtrap $options(-name) $trap $stack [$self hndrte_exe]
    }

    method parse_traplog {file} {
	set f [open $file]
	set log [read $f]
	regsub -line -all {.*>\s+} $log {} log
	UTF::Message LOG "$options(-name)>" $log
	UTF::RTE::parse_traplog $self $log
	close $f
    }

    variable TRAPLOG

    method rte_consolemsg {msg} {

	if {$msg eq ""} {
	    return
	}

	regsub {^\d{6}\.\d{3} } $msg {} msg; # strip timestamps

	if {[info exists TRAPLOG]} {
	    # If FWID is set we're collecting new-style trap messages
	    append TRAPLOG "$msg\n"
	}

	if {[regexp {ASSERT pc} $msg]} {
	    # Downgrade these to warnings, since they will be handled
	    # by the trap handler later
	    UTF::Message WARN "$options(-name)>" $msg
	} elseif {[regexp {FWID (\d\d-[[:xdigit:]]{1,8})} $msg]} {
	    UTF::Message INFO "$options(-name)>" $msg
	    if {![info exists TRAPLOG]} {
		set TRAPLOG "$msg\n"
	    }
	} elseif {!$processingDongleError &&
		  [regexp {TRAP |ASSERT.*|Trap type .*| Overflow } $msg]} {
	    set processingDongleError true
	    if {![info exists TRAPLOG]} {
		# No FWID, but we want to capture these anyway
		set TRAPLOG "$msg\n"
	    }
	    UTF::Message FAIL "$options(-name)>" $msg

	    # Give DHD driver a moment to collect trap log
	    UTF::Sleep 0.2 quiet

	    set trap [UTF::RTE::parse_traplog $self $TRAPLOG]
	    unset TRAPLOG
	    if {$trap ne ""} {
		set msg $trap
	    }

	    # If there's already an assert, don't overwrite it
	    if {![info exists ::UTF::panic] ||
		![regexp -nocase {assert|trap} $::UTF::panic]} {
		set ::UTF::panic "> $msg"
	    }
	    if {[regexp {hsic} [$self cget -brand]]} {
		# Attempt reload, but only for HSIC test platforms,
		# not real routers.
		[lindex $options(-sta) 0] reload
	    }
	    set processingDongleError false
	} elseif {[regexp {No memory to satisfy request .* inuse \d+} $msg m]} {

	    UTF::Message WARN "$options(-name)>" $msg

	    # If there's already a warning, don't overwrite it
	    if {![info exists ::UTF::warn]} {
		set ::UTF::warn "> $m"
	    }
	} elseif {![$base common_getdata $msg "$options(-name)>"]} {
	    UTF::Message LOG "$options(-name)>" $msg
	}
    }

    # Internal callback for fileevent below
    method rte_getdata {request} {
	set fd $msgfile($request)
	if {![eof $fd]} {
	    $self rte_consolemsg [gets $fd]
	} else {
	    # Leave non-blocking for safer close.  Any error messages
	    # should have been collected earlier.
	    close $fd
	    UTF::Message LOG $options(-name) "Log closed $fd"
	    unset msgfile($request)
	}
    }

    UTF::doc {
	# [call [arg host] [method open_rte_messages] [lb][arg file][rb]]

	# Open system message log on [arg host] and log new messages
	# to the UTF log.  [method open_messages] will return
	# immediately, while messages will continue to be logged in
	# the background until [method close_messages] is called.
	# [arg file] defaults to [file /var/log/messages].  Multiple
	# loggers can be opened, so long as they log different [arg
	# file]s.  Attempts to log the same [arg file] multiple times
	# will be logged and ignored.
    }

    method open_rte_messages { {file ""} } {
	if {$file == ""} {
	    set file $options(-rteconsole)
	}
	set id "[$self cget -lanpeer]:[$self cget -lan_ip]"
	if {[info exists msgfile($id:$file)]} {
	    return
	}
	UTF::Message LOG $options(-name) "Open $file"
	if {[catch {$self serialrelay socket $file} ret]} {
	    $self worry "$file: $ret"
	    return
	}
	set msgfile($id:$file) $ret
	fconfigure $msgfile($id:$file) -blocking 0 -buffering line
	puts $msgfile($id:$file) ""; # Trigger NPC telnet reconnect
	fileevent $msgfile($id:$file) readable [mymethod rte_getdata $id:$file]
	UTF::Sleep 1
	return $ret
    }

    UTF::doc {
	# [call [arg host] [method close_rte_messages] [lb][arg file][rb]]

	# Close a system message log previously opened by [method
	# open_messages].  An error will be reported if [arg file]
	# does not match that of a previous invocation of [method
	# open_messages].
    }
    method close_rte_messages { {file ""} } {
	if {$file == ""} {
	    set file $options(-rteconsole)
	}
	set id "[$self cget -lanpeer]:[$self cget -lan_ip]"
	if {[info exists msgfile($id:$file)] &&
	    [file channels $msgfile($id:$file)] ne ""} {
	    UTF::Message LOG $options(-name) "Close $file $msgfile($id:$file)"
	    if {[set pid [pid $msgfile($id:$file)]] ne ""} {
		# Processes to wait for
		catch {exec kill {*}$pid} ret
		UTF::Message LOG $options(-name) $ret
	    }
	    # Leave non-blocking for safer close.  Any error messages
	    # should have been collected earlier.
	    close $msgfile($id:$file)
	    unset msgfile($id:$file)
	} else {
	    UTF::Message LOG $options(-name) "Close $file (not open)"
	}
	return
    }

    UTF::doc {
	# [call [arg host] [method maxmem]]

	# Returns maxmem bytes using "mu" RTE command on the dongle
	# serial port.
    }

    method maxmem {} {
	if {[regexp {Max memory in use: (\d+)} [$self rte mu] - mm]} {
	    return $mm
	} else {
	    error "Max memory not found"
	}
    }

    UTF::doc {
	# [call [arg host] [method freekb]]

	# Returns free mem kbytes using "mu" RTE command on the dongle.
    }

    method freekb {} {
	catch {$self rte mu} ret
	if {![regexp {Free: (\d+)} $ret - free]} {
	    error "Free mem not found ($ret)"
	}
	expr {$free / 1024.0}
    }

    UTF::doc {
	# [call [arg host] [method rte_available]]

	# Returns true if rte commands are supported
    }

    method rte_available {} {
	# disable HSIC memory checks due to extremely slow UART
	if {[regexp {dhdap|usbap} [$self cget -brand]]} {
	    return 1
	} else {
	    return 0
	}
    }

    method rte {args} {
	if {$options(-rteconsole) eq ""} {
	    # No console - try to send command via dhd.  Capture the
	    # latest host logs.
	    try {
		set rtecapture ""
		$self dhd -i $options(-device) cons $args
		UTF::Sleep 1
	    } finally {
		set ret $rtecapture
		unset rtecapture
	    }
	} else {
	    set ret [$self serialrelay rexec -t 5 -s \
			 $UTF::usrutf/rteshell -$options(-rteconsole) $args]
	}
	regsub -line -all {^\d{6}\.\d{3} } $ret {} ret; # strip timestamps
	return $ret
    }

    method rte_trash_symbol {symbol} {
	regsub {.exe} [$self hndrte_exe] {.map} hndrte_map
	set m [open $hndrte_map r]
	set data [read $m]
	close $m
	if {[regexp -line "^(\\S+)\\s+\[TW]\\s+$symbol$" $data match addr]} {
	    UTF::Message LOG $options(-name) "Found $symbol at 0x$addr"
	} else {
	    error "$symbol: not found in $hndrte_map\n$data"
	}
	# long align
	set addr [format "%x" [expr {"0x$addr" - ("0x$addr" % 4)}]]
	$self dhd -i $options(-device) membytes 0x$addr 16 00
    }

    method rte_instant_panic {} {
	if {![regexp {dhdap} [$self cget -brand]]} {
	    error "Unsupported"
	}
	$self rte_trash_symbol "wlc_scan"
	$self wl -i $options(-device) up
	$self wl -i $options(-device) scan
    }


    UTF::doc {
	# [call [arg host] [method apshell] [arg args]]

	# Run cmdline [arg args] on the [arg host] console directly,
	# bypassing [cmd ushd].
    }

    method apshell {args} {
	$self relay rexec -t 5 -s \
	    $UTF::usrutf/apshell -$options(-console) $args
    }


    #################
    ## Linksys tools
    #################

    method linksys_bootnv {} {
	set getnv [$self rexec -n {nvram show|grep \\\^get_}]
	if {$getnv eq ""} {
	    error "No bootnv set.  Perhaps this isn't a Linksys image?"
	}
	foreach {- k v} [regexp -inline -all -line \
			     {get_([^=]+)=(.*)} $getnv] {
	    if {$k eq "mac"} {
		lappend nv "eth0macaddr=$v"
	    } elseif {[regexp {^pa(\d)g(h?)a(\d)idxval$} $k - b h c]} {
		lappend nv "pa${b}g${h}w0a${c}=[lindex $v 1]"
		lappend nv "pa${b}g${h}w1a${c}=[lindex $v 2]"
		lappend nv "pa${b}g${h}w2a${c}=[lindex $v 3]"
	    }
	}
	return [join [lsort $nv] \n]
    }


    # Peer passthroughs
    UTF::PassThroughMethod lan -lanpeer
    UTF::PassThroughMethod wan -wanpeer
    UTF::PassThroughMethod relay -relay
    UTF::PassThroughMethod serialrelay -serialrelay

    ##################
    ## Web page tools
    ##################

    UTF::doc {
	# [call [arg host] [method {http get}] [arg path] [lb][arg data][rb]]

	# Fetch web page source.  [arg path] should be a partial URL,
	# eg index.asp.  [arg data] may be used to supply unit
	# numbers, etc, eg "[arg wl_unit] [option 1]"
    }

    variable wget_anc --auth-no-challenge

    method {http get} {PATH {DATA {}}} {
	package require http
	UTF::Message LOG $options(-name) \
	    "POST http://$options(-lan_ip)/$PATH $DATA"
	set ret [$self relay -s "wget -nv -O- \
		     $wget_anc \
		     --http-user= \
		     --http-passwd='$options(-passwd)' \
		     --post-data='[http::formatQuery {*}$DATA]' \
		     'http://$options(-lan_ip)/$PATH'"]
	if {[regexp {unrecognized option `--auth-no-challenge'} $ret]} {
	    UTF::Message LOG $options(-name) \
		"Legacy wget, retry without --auth..."
	    set wget_anc ""
	    return [$self http get $PATH $DATA]
	}
	return $ret
    }

    # Internal variable, stores results from HTML parsing
    variable form

    # Internal callback
    # Called for each HTML tag in the page
    method rgetformcmd {tag slash param textBehindTheTag} {
	set tag [string tolower $tag]
	switch -- $tag {
	    input { # Input field
		if {![regexp {type="*submit|reset|file"*} $param] &&
		    [regexp {name="([^\"]*)"} $param - name] &&
		    [regexp {value="([^\"]*)"} $param - value]} {
		    if {[info exists form($name)]} {
			UTF::Message WARN $options(-name) \
			    "Ignoring duplicate field: $name=$value"
		    } else {
			set form($name) $value
		    }
		}
	    }
	    select { # Selection field
		if {$slash == "/"} {
		    unset form(-selection)
		} elseif {[regexp {name="([^\"]*)"} $param - name]} {
		    set form(-selection) $name
		}
	    }
	    option { # Selection option
		if {[regexp {value="?([^\"]*)"? selected} $param - value] &&
		    $form(-selection) != ""} {
		    set form($form(-selection)) $value
		}
	    }
	    h4 { # Error message
		UTF::Message LOG $options(-name) [string trim $textBehindTheTag]
	    }
	    default {
		# Ignore all other tags
		return
	    }
	}
    }

    UTF::doc {
	# [call [arg host] [method {http getform}] [arg path]
	#	  [lb][arg data][rb]]

	# Fetch web page and extract all of the current parameters
	# from any forms within the page.  [arg url] is a partial URL,
	# eg wireless.asp.  [arg data] may be used to supply unit
	# numbers, etc, eg "[arg wl_unit] [option 1]".  The results
	# are returned as a list suitable for assigning to an array
	# with [cmd {array set}].
    }

    method {http getform} {PATH {DATA {}}} {
	package require htmlparse
	array unset form
	if {[catch {$self http get $PATH $DATA} ret]} {
	    if {[regexp {fsh tunnel lost} $ret]} {
		# retry once
		set ret [$self http get $PATH $DATA]
	    } else {
		error $ret
	    }
	}
	htmlparse::parse -cmd [mymethod rgetformcmd] $ret
	return [array get form]
    }

    proc parsereply {reply} {
	package require htmlparse
	package require struct::tree
	# Scan web page tree, looking for text inside the form
	htmlparse::2tree $reply [struct::tree mytree]
	set inform ""
	mytree walk root -order both {action node} {
	    set type [mytree get $node type]
	    if {$type == "form"} { # Found the form
		set inform $action
	    } elseif {$action == "enter" &&
		      $inform == "enter" &&
		      $type == "PCDATA"} {
		set data [mytree get $node data]
		lappend messages $data
	    }
	}
	rename mytree ""
	return [join $messages "\n"]
    }

    UTF::doc {
	# [call [arg host] [method {http apply}] [arg path] [arg data]]

	# Attempt to set specified list of web form parameters using a
	# web post.

	# The [arg path] web page will first be queried to find the
	# current values of any required parameters not specified in
	# [arg DATA].  The complete set of parameters will then by
	# uploaded to the Web page with [arg action=][option Apply].
	# If sucessful, the Router will restart.  Failures will be
	# reported where possible.

	# [arg path] is a partial URL, eg [file apply.cgi] or one of
	# the [file .asp] files in case special processing is
	# required.

	# [example_begin]

	$Router http apply wireless.asp "wl_unit 0 wl_ssid MyNewSSID"

	# [example_end]

	# See src/router/shared/broadcom.c for constraints and special
	# processing.
    }

    method {http apply} {PATH {DATA {}}} {
	set expectBoot 1
	array set data [concat [$self http getform $PATH $DATA] \
			    $DATA action Apply]
	set ret [parsereply [$self http get $PATH [array get data]]]
	UTF::Sleep 5
	return $ret
    }

    method {http restore_defaults} {} {
	set expectBoot 1
	set ret [parsereply [$self http get apply.cgi \
				 {action "Restore Defaults"}]]
	UTF::Sleep 10
	return $ret
    }

    UTF::doc {
	# [call [arg host] [method {http getnvram}]]

	# Fetch NVRAM parameters from Router via http.  The results
	# are returned as a string of key=value pairs seperated by
	# newlines.  The result will contain a checksum and will be
	# suitable for uploading again via [method {http putnvram}].
    }

    method {http getnvram} {{file {}}} {
	#$self http get nvramdl.cgi
        if {$options(-passwd) eq ":unset:"} {
            set options(-passwd) [$self nvram get http_passwd]
        }
	set cmd [list curl -f -u :"$options(-passwd)" \
		     http://$options(-lan_ip)/nvramdl.cgi]
	set dst [exec mktemp /tmp/nvram.fetch_XXXXX]
	try {
	    lappend cmd -o $dst
	    $self relay {*}$cmd
	    if {$file ne ""} {
		$self relay copyfrom $dst $file
	    } else {
		$self relay cat $dst
	    }
	} finally {
	    $self relay rm -f $dst
	}
    }

    method {http upload} {name file path} {
	if {$options(-passwd) eq ":unset:"} {
            set options(-passwd) [$self nvram get http_passwd]
        }
	$self relay curl -f -u "':$options(-passwd)'" \
	    -F "'file=@$file;filename=$name'" http://$options(-lan_ip)/$path
    }

    UTF::doc {
	# [call [arg host] [method {http putnvram}] [arg file]]

	# Upload stored NVRAM [arg file] to Router via http.
    }
    method {http putnvram} {file} {
	set dst [exec mktemp /tmp/nvram.put_XXXXX]
	try {
	    $self relay copyto $file $dst
	    $self http upload "nvfile" $dst "nvramul.cgi"
	} finally {
	    $self relay rm $dst
	}
    }

    UTF::doc {
	# [call [arg host] [method {http upgrade}] [arg file]]

	# Ugrade Router from the specified firmware [arg file].
	# Success or Failure will be reported.
    }

    method {http upgrade} {file} {
	$self http upload "file" $file "upgrade.cgi"
    }

    method whatami {{STA ""}} {
	if {[catch {$self boardname} b]} {
	    set b "<unknown>"
	}
	if {$STA ne ""} {
	    if {[catch {$STA chipname} c]} {
		set c "<unknown>"
	    }
	    if {$c ne $b} {
		set b "$b/$c"; # Router / Wireless
	    } else {
		set b $c; # Single-chip combo wireless router
	    }
	}
	return "Router $b"
    }

}

# Retrieve manpage from last object
UTF::doc [UTF::Router man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]
    package require UTF
    UTF::Router Rout1 -lan_ip 192.168.1.1 -sta {STA3 eth2}
    STA3 wl status
    -> Associated with bssid: 6A:41:EA:2F:A0:D4        SSID: "Test381"

    # [example_end]
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
