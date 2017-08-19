#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: b9b2f3d69eac2ff0e01251d5b458aca9273384f0 $
# $Copyright Broadcom Corporation$
#

package provide UTF::Base 2.0

if {[lsearch $auto_path .] == -1} {
    lappend auto_path .
}
package require snit
package require UTF::doc
package require UTF::Base::Rexec

namespace eval UTF::Base {}


UTF::doc {
    # [manpage_begin UTF::Base n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Base support}]
    # [copyright {2005 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::Base is a basic implementation of the UTF host object,
    # containing those methods common to most host objects.  Host
    # objects can delegate their common options and methods to
    # UTF::Base.

    # [list_begin definitions]

}

snit::type UTF::Base {

    UTF::doc {
	# [call [cmd UTF::Base] [arg host]
	#	[option -name] [arg name]
	#	[lb][option -lan_ip] [arg address][rb]
	#	[lb][option -wl] [arg {wl path}][rb]
	# 	[lb][option -ssh] [arg path][rb]
	# 	[lb][option -ping] [arg cmdline][rb]
	# 	[lb][option -relay] [arg relay][rb]
	# 	[lb][option -user] [arg user][rb]
        #       [lb][option -power] [arg {{controller socket}}][rb]
        #       [lb][option -power_button] [arg {{controller socket}}][rb]
        #       [lb][option -power_button_pushes] [arg integer][rb]
        #       [lb][option -power_sta] [arg {{controller socket}}][rb]
        #       [lb][option -device_reset] [arg {power_controller port}][rb]
        #       [lb][option -arasan] [arg {power_controller port}][rb]
	#       [lb][option -init] [arg script][rb]
	#       [lb][option -initialized] [arg {0|1}][rb]
	#       [lb][option -infrastructure] [arg boolean][rb]
	#       [lb][option -msgactions] [arg action_list][rb]]
    }
    UTF::doc {
	# Create a new Base host object.
	# [list_begin options]

	# [opt_def [option -name] [arg name]]

	# Name of the host.

	# [opt_def [option -lan_ip] [arg address]]

	# Host name or IP address to be used to contact host.  This
	# should be a backbone address, not involved in the actual
	# testing.  Defaults to [arg name].

	# [opt_def [option -ssh] [arg path]]

	# Specify an alternate command to use to contact [arg host].
	# The command should have [cmd rsh]-like syntax.  The default
	# is to try [cmd fsh] first and if that fails, try [cmd ssh].[para]

        # If fsh gets errors, you can force ssh usage by specifying
        # "-ssh ssh" on the object in the config file.

	# [opt_def [option -ping] [arg cmdline]]

	# Specify an alternate ping commandline to use when testing
	# network connectivity.  Tokens %c and %s will be replaced by
	# the ping count and data size respectively.  If possible, the
	# ping commandline should not do hostname lookup and it should
	# exit at the first sucessful returned packet.  eg:

	# [example_begin]
	-ping {epi_ping -n -q -c 1 -C %c -s %s}
	# [example_end]

	# [opt_def [option -wl] [arg path]]

	# Specify an alternate path to the [cmd wl] command.  The
	# default is [file wl].

	# [opt_def [option -relay] [arg relay]]

	# Specify [arg relay] as an alternate host object to use to
	# relay commands destined for [arg host].  For example, if
	# [arg host] is not accessible from the control host, but it
	# is accessible from [arg relay].  The default is to not use a
	# relay.

	# [opt_def [option -user] [arg user]]

	# Specify a username for logging in to [arg host].  Default is
	# to use the username of the invoking user.

	# [opt_def [option -power] [arg {{controller socket}}]]

	# If the [arg host] is connected to a remote controlable power
	# supply, such as the WTI IPS1600, UTF will be able to
	# power-cycle the host when necessary.  [arg controller] is
	# the IP address or hostname of the power supply, [arg socket]
	# is the number of the power socket supporting [arg host].
        # Default is null.

        # [opt_def [option -power_button] [arg {{controller socket}}]]

        # If the power button of the host has been customized in order
        # to be connected to a remote controlable power supply and external
        # relay, you can automatically push the power button to restart a
        # device that has been powered off. Calling parameters are same
        # as -power. Default is null.[para]

        # If the host automatically recovers by itself and does
        # not need the power button pushed, then set this option to
        # [arg auto]. Examples of devices that recover by themselves are
        # the PixelUSA minitower when the BIOS has been set to "power on"
        # after power failure, or Dell laptops that have the power
        # button mechanically jammed or soldered into the on position.

        # [opt_def [option -power_button_pushes] [arg integer]]

        # Some PC require more than one push-and-hold of the power button
        # to truly force the PC power off. This option allows you to
        # configure those specific PC instances that need more than one
        # push-and-hold event to force the power off. Default is 1.

        # [opt_def [option -power_sta] [arg {{controller socket}}]]

        # Some STAs, such as the 4322 USB dongle, have an option for
        # external power input. This is useful for reseting the STA
        # if the serial console port is not defined or is unresponsive.
        # Calling parameters are same as -power. Default is null.

	# [opt_def [option -device_reset] [arg {power_controller port}]]

        # Specifies the power_controller network name and port for
        # the remote reset circuit attached to the device. Default is null.

	# [opt_def [option -arasan] [arg {power_controller port}]]

        # Specifies the WebRelay network name and port for the Arasan
        # board inside the host device. Default is null.  A 3-wire
        # connection is needed from the Arasan board to the WebRelay
        # contacts. The Normally Closed relay contact is expected to
        # be connected to the 3.3V power supply on the Arasan
        # card. The Normally Open contact is expected to be connected
        # to the 1.8V power supply on the Arasan card.

	# [opt_def [option -init] [arg script]]

	# Provide [arg script] as an initialization script to
	# configure communications with the Base system, if needed.
	# [arg script] is normally provided by a parent object.

	# [opt_def [option -initialised] [arg {0|1}]]

	# Option to indicate if the STA object needs initializing.
	# Re-initialization can be triggered by:

	# [example_begin]
	$base configure -initialized 0
	# [example_end]

	# [opt_def [option -infrastructure] [arg boolean]]

	# Flag to be set for infrastructure objects, not directly
	# involved in testing.  This is to make sure we don't
	# reconfigure or reboot them by accident.  Unset should be
	# interpreted as false.

	# [opt_def [option -msgactions] [arg action_list]]

	# Per-object customization of the log message parser.  [arg
	# action_list] consists of a list of [arg pattern] [arg
	# action] pairs where [arg pattern] is a [cmd regexp] to match
	# against the log messages and [arg action] is one of [cmd
	# PASS], [cmd WARN], or [cmd FAIL].  Comments are permitted.
	# Syntax is checked at configure time.  If [arg action] is not
	# in the above list it will be treated as a code block to be
	# evaluated.  This can be used for custom error handling.  The
	# code block should end with [cmd return] [arg 1] if the
	# hadling is complete, or [cmd return] [arg 0] if the message
	# should be passed on for further processing.

	# [example_begin]
    } {{ -msgactions {

	# Specifically watching for a normally ignored message
	{ampdu_watchdog} FAIL

	# These are common messages on this device so downgrade
	# the default fail to a WARNING
	{unexpected tx status returned \(0x1\)} WARN

	# custom handler to recover from a bad assert
	{assertion .* ACPHY_RfseqStatus} {
	    $self power cycle
	    return 0
	}

    }
    }} {
	# [example_end]

	# [list_end]
	# [list_end]

	# Base objects have the following methods:
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -ssh
    option -rexec_add_errorcodes -type snit::boolean -default false
    option -iperf iperf
    option -wl "wl"
    option -user
    option -gub ""; # alternate build path
    option -lan_ip -cgetmethod GetDefLanIP -configuremethod _adddomain
    option -ping "epi_ping -n -q -c 1 -C %c -s %s"
    option -relay
    option -serialrelay localhost
    option -lanpeer
    option -power
    option -power_button "auto"
    option -power_button_pushes 1
    option -power_sta
    option -device_reset
    option -arasan
    option -name
    option -init
    option -initialized 0
    option -onall -type snit::boolean -default true
    option -slowassoc -type snit::integer -default 10
    option -slowpasshash -type snit::integer -default 0
    option -iperfdaemon -type snit::boolean -default true

    # Alternative to -wlinitcmds for more complex setups.  Use %S to refer to
    # the primary STA object.
    option -initscript

    option -brand

    # Alternate STA object to use for sniffer
    option -portmirror

    # Flag to be set for infrastructure objects, not directly involved
    # in testing.  This is to make sure we don't reconfigure or reboot
    # them by accident.
    option -infrastructure -type snit::boolean -default false


    # ignored for compatibility
    option -dhcpretries

    # Hooks for attaching diagnostic commands
    option -pre_perf_hook
    option -post_perf_hook
    option -post_assoc_hook
    option -add_networks_hook

    # options to allow tests to be modified per device
    option -perfonly -type snit::boolean -default false
    option -nowep -type snit::boolean -default false
    option -nosharedwep -type snit::boolean -default false
    option -notkip -type snit::boolean -default false
    option -noaes -type snit::boolean -default false
    option -noafterburner -type snit::boolean -default true
    option -noframeburst -type snit::boolean -default false
    option -nomaxmem -type snit::boolean -default false
    option -nofragmentation -type snit::boolean -default false
    option -nopm1 -type snit::boolean -default false
    option -nopm2 -type snit::boolean -default false
    option -nochannels -type snit::boolean -default false
    option -nombss -type snit::boolean -default false
    option -noibss -type snit::boolean -default false
    option -nobx -type snit::boolean -default false
    option -nobighammer -type snit::boolean -default false
    option -nomimo_bw_cap -type snit::boolean -default false
    option -nobeaconratio -type snit::boolean -default false
    option -nocustom -type snit::boolean -default false
    option -usewep64 -type snit::boolean -default false
    option -nonrate -type snit::boolean -default false
    option -noradio_pwrsave -type snit::boolean -default false
    option -nokpps -type snit::boolean -default false
    option -extsup -type snit::boolean -default false
    option -escan -type snit::boolean -default false
    option -use11h -type snit::boolean -default false
    option -usecsa -type snit::boolean -default false

    # Test in STA in AP mode against Router in WET mode
    option -apmode -type snit::boolean -default false

    # Use as a fully functional AP
    option -ap -type snit::boolean -default false
    option -wlconf_by_nvram -type snit::boolean -default false

    option -tcpwindow auto
    option -dosystime -type snit::boolean -default false
    option -docpu -type snit::boolean -default false
    option -nocal -cgetmethod AutoExternal -type UTF::Base::auto -default auto
    option -nointerrupts -cgetmethod AutoExternal -type UTF::Base::auto -default true
    option -tcpslowstart -type {snit::double -min 0} -default 0
    option -channelsweep
    option -bxstress -type snit::integer -default 0
    option -datarate
    option -yart
    option -rvrnightly
    option -udp 0
    option -custom
    option -pm 0

    # Default chanspecs (Only used for router tests so far)
    option -chanspec220 3
    option -chanspec240 3l
    option -chanspec520 36
    option -chanspec540 36l

    option -perfchans ""

    option -msgactions -type {UTF::Base::msgaction} -default {}

    variable connect_checked 0
    variable expect_reinit 0

    # cget method for options where "auto" means is-external
    method AutoExternal {key} {
	if {$options($key) eq "auto"} {
	    return [regexp {external} $options(-brand)]
	} else {
	    return $options($key)
	}
    }

    method GetDefLanIP {args} {
	if {$options(-lan_ip) eq ""} {
	    set options(-lan_ip) [UTF::AddDomain $options(-name)]
	}
	set options(-lan_ip)
    }

    method _adddomain {name val} {
	set options($name) [UTF::AddDomain $val]
    }

    typeconstructor {
	proc msgaction {"validate" val} {
	    foreach {pattern action} [UTF::decomment $val] {
		if {$pattern eq ""} {
		    error "null pattern - would match everything"
		}
		switch $action {
		    {} {
			error "$action (should be PASS/WARN/FAIL or a code block)"
		    }
		}
	    }
	}
	proc auto {"validate" value} {
	    if {![string is boolean -strict $value] &&
		![string match -nocase "auto" $value]} {
		return -code error \
		    "invalid auto \"$value\", should be a \"auto\" or a tcl boolean"
	    }
	    return
	}
    }

    constructor {args} {
	if {[info exists ::UTF::Use11h]} {
	    set options(-use11h) $::UTF::Use11h
	}
	if {[info exists ::UTF::UseCSA]} {
	    set options(-usecsa) $::UTF::UseCSA
	}
        $self configurelist $args
    }

    UTF::doc {
	# [call [arg host] [method wl] [arg {args ...}]]

	# Run [cmd wl] command on host.  [option -i] [arg device] will
	# be needed to access a specific wireless device.  Note that
	# this differs from the [method wl] method of a STA object,
	# which already knows which device to access. [para]

	# To prevent logging use [option -silent] since the regular
	# [option -s] will get stripped by other layers. [para]

	# [option -u] can be used to suppress the error code in cases
	# where the [cmd wl] command returns "Unsupported".  This is
	# useful in cases where you want to turn off a feature that
	# may not be present at all in the particular driver under
	# test.

    }
    method wl {args} {
	set pargs $args
	UTF::GetKnownopts {
	    {silent "Suppress logging"}
	    {u "Unsupported is ok"}
	}
	if {$(silent)} {
	    set s -s
	} else {
	    set s ""
	}
	set code [catch {$self rexec {*}$s {*}$options(-wl) {*}$args} ret]
	set ec $::errorCode
	if {$code == 1} {
	    # Special processing for failures

	    # Translate error codes
	    if {[regexp {wl(?:\.exe)?: error (-\d+)} $ret - i]} {
		set ret "wl: [UTF::wlerror $i]"
		UTF::Message LOG $options(-name) $ret
	    }

	    # compatibility layer for new rsdb modularity
	    if {[regexp \
		     {parsing value "(\w+)" as an integer for set of "rsdb"} \
		     $ret - c] &&
		[set ix [lsearch $pargs "rsdb"]] >= 0 &&
		[lindex $pargs $ix+1] eq $c} {
		return [$self wl {*}[lreplace $pargs $ix $ix+1 "rsdb_$c"]]
	    }

	    # Allow unsupported, on user request.  Windows minioctl
	    # dongles sometimes just give a generic error instead of
	    # Unsupported.
	    if {$(u) &&
		[regexp {Unsupported|Error getting the last error} $ret]} {
		set code 0
	    }
	} else {
	    # Special processing for success

	    # Strip verbiage
	    if {[lsearch $args ssid] >= 0} {
		regexp {(?:Current |^)SSID: "(.*)"} $ret - ret
	    } elseif {[lsearch $args bssid] >= 0} {
		regsub {bssid is } $ret {} ret
	    } elseif {[lsearch $args cur_etheraddr] >= 0} {
		regsub {cur_etheraddr } $ret {} ret
	    }
	}

	return -code $code -errorcode $ec $ret
    }

    UTF::doc {
	# [call [arg host]  [method services] [arg [lb]start|stop[rb]]]

	# No-op.
    }

    method services {op} {
    }

    variable reading

    method kill {cmd fd} {
	set pids [pid $fd]
	UTF::Message LOG $options(-name) "Timeout: rexec $cmd ($fd): kill $pids"
	set reading($fd) "Timeout"
	# No need to reset the connection if it's just a ping failure.
	if {$options(-initialized) == 1 && ![regexp {ping} $cmd]} {
	    UTF::Message LOG $options(-name) "Reset connection"
	    set options(-initialized) 0
	}
	try {
	    exec kill {*}$pids
	} finally {
	    # Let reaper run unattended in the background.  We don't
	    # need to wait for it.
	    after 2000 [list catch [list exec kill -KILL {*}$pids]]
	}
    }

    UTF::doc {
	# [call [arg host] [method rexec]
	#               [lb][option {options ...}][rb]
        #               [cmd cmd] [arg {args ...}]]

	# Run [cmd cmd] [arg {args ...}] on [arg host].  Args and
	# options are passed directly to the [method open] method of a
	# newly created [cmd UTF::Base::Rexec].  If [option -async] is
	# used, the [method rexec] will return the object for later
	# collection, otherwise the results will be collected and
	# returned directly.  See [uri APdoc.cgi?UTF::Base::Rexec.tcl
	# UTF::Base::Rexec] for details.

    }


    method rexec {args} {
#	UTF::Message LOG $options(-name) $args
	set handler [UTF::Base::Rexec %AUTO% -base $self -args $args]
	$handler open
	if {[$handler async]} {
	    return $handler
	} else {
	    $handler close
	}
    }

    UTF::doc {
	# [call [arg host] [method rpopen]
	#               [lb][option -quiet][rb]
        #               [cmd cmd] [arg {args ...}]]

	# Run [cmd cmd] [arg {args ...}] on [arg host].  Returns
	# a channel opened for reading the results from the command.

	# [list_begin options]

	# [opt_def [option -quiet]]

	# Suppress logging of the command.  By default both the
	# command line and the results are reported to the test log.

	# [list_end]
    }

    method rpopen {args} {
	#	UTF::Message LOG $options(-name) [list rpopen {*}$args]

	if {[catch {cmdline::typedGetoptions args {
	    {quiet "Don't log commandline"}
	    {noinit "Don't run init code"}
	    {in "Don't close stdin"}
	    {rw "Open read+write"}
	    {2 "Don't dup 2 onto 1"}
	    {add_errorcodes "Add errorcodes"}
	} "rpopen options"} ret]} {
	    return -code error $ret
	} else {
	    array set pargs $ret
	}

	if {[info exists pargs(rw)]} {
	    set mode "r+"
	} else {
	    set mode "r"
	}

	if {$options(-lan_ip) == "localhost"} {
	    # Special case, for optimizing serial port tools, etc
	    if {![info exists pargs(quiet)]} {
		regsub {^{(.*)}$} $args {\1} msg
		UTF::Message LOG $options(-name) $msg
	    }
	    if {[info exists pargs(add_errorcodes)]} {
		set args [concat $args {\;echo\ @\\$?@}]
	    }
	    # Only append 2>@stdout if the user hasn't already provided
	    # redirection
	    if {![info exists pargs(2)] && ![regexp {2>} $args]} {
		lappend args {2>@stdout}
	    }
	    if {![info exists pargs(in)] && ![info exists pargs(rw)] &&
		![regexp {<} $args]} {
		lappend args {</dev/null}
	    }
	    # XXX fixme
	    set args [eval concat $args]
	    # Rewrite path for relays
	    if {$UTF::usrutf ne $UTF::unittest} {
		regsub "^$UTF::usrutf/" $args "$UTF::unittest/" args
	    }
	    return  [open "|$args" $mode]
	}

	if {![info exists pargs(noinit)] && !$options(-initialized)} {
	    # Mark as initialized.  Commands inside init should make
	    # sure they use -noinit to avoid recursion, but we have to
	    # mark it as initialized first so that if they time-out
	    # they will reset the init status for the next attempt.
	    set options(-initialized) 1
	    eval $options(-init)
	}

	set lan_ip [$self cget -lan_ip]
	set cmd $options(-ssh)
	if {$cmd == ""} {
	    $self connect
	    set cmd $options(-ssh)
	}
	if {$options(-relay) != ""} {
	    if {[info exists pargs(add_errorcodes)]} {
		set args [concat $args {\;echo\ @\\$?@}]
	    }
	    regsub -all {\"} $args {\"} args
	    set args [concat $cmd $lan_ip {\"} $args {\"}]
	    if {[info exists pargs(quiet)]} {
		set args [concat -quiet $args]
	    }
	    if {[info exists pargs(in)]} {
		set args [concat -in $args]
	    }
	    if {[info exists pargs(rw)]} {
		set args [concat -rw $args]
	    }
	    if {[info exists pargs(2)]} {
		set args [concat -2 $args]
	    }
	    # delegate to relay
	    return [$options(-relay) rpopen -n {*}$args]
	}
	if {$options(-user) != ""} {
	    lappend cmd -l $options(-user)
	}
	if {![info exists pargs(quiet)]} {
	    regsub {^{(.*)}$} $args {\1} msg
	    regsub { {;echo @\\\$\?@}} $msg {} msg
	    UTF::Message LOG $options(-name) $msg
	}
	if {[info exists pargs(add_errorcodes)]} {
	    set args [concat $args {\;echo\ @$?@}]
	}
	if {[info exists ::UTF::debugrexec]} {
	    UTF::Message DEBUG $options(-name) \
		[list {*}$cmd {*}$lan_ip {*}$args]
	}
	# NOTE: use of 2>@stdout requires fshd patch to prevent
	# hanging on first connection.
	# Only append 2>@stdout if the user hasn't already provided
	# redirection
	if {![info exists pargs(2)] && ![regexp {2>} $args]} {
	    lappend args {2>@stdout}
	}
	if {![info exists pargs(in)] && ![info exists pargs(rw)] &&
	    ![regexp {<} $args]} {
	    lappend args {</dev/null}
	}
	return [open [concat | $cmd $lan_ip $args] $mode]
    }

    UTF::doc {
	# [call [arg host] [method sumcheck] [arg local] [arg remote]]

	# Compare checksums of local and remote copies of a file.
	# Returns zero if sums match and 1 if sums differ, or if the
	# remote copy is missing.  Useful for checking to see if a
	# copy is needed before invoking [method copyto] for files
	# that rarely change.
    }
    method sumcheck {local remote} {
	expr {[catch {$self sum $remote} ret] ||
	      [lrange $ret 0 1] ne [lrange [exec sum $local] 0 1]}
    }

    UTF::doc {
	# [call [arg host] [method copyto] [arg src] [arg dest]]

	# Copy local file [arg src] to [arg dest] on [arg host]
	# If src ends in .gz, but dest doesn't then the file will
	# be uncompressed automatically.
    }

    typevariable fcp ""
    proc _check_fcp {} {
	if {[info exists ::UTF::UseFCP] && $fcp eq ""} {
	    if {$::UTF::UseFCP ne "nocheck" &&
		[auto_execok fcp] eq "/usr/bin/fcp" &&
		[file mtime /usr/libexec/fcpwrap] <
		[file mtime $::UTF::projtools/linux/libexec/fcpwrap]} {
		UTF::Message WARN "" "Old fcpwrap, forcing tools version"
		set fcp "$::UTF::projtools/linux/bin/fcp"
	    } else {
		set fcp "fcp"
	    }
	}
    }

    method copyto {args} {
	if {$options(-ssh) == ""} {
	    $self connect
	}
	set lan_ip [$self cget -lan_ip]

	_check_fcp

	# If -ssh contains ssh with options, use the options,
	# otherwise just use scp
	if {!([info exists ::UTF::UseFCP] &&
	      [regsub {fsh} $options(-ssh) $fcp scp]) &&
	    ![regsub {ssh} $options(-ssh) {scp} scp]} {
	    set scp scp
	}
	# Should use fcp here, except it doesn't work when target
	# uses a different username from src

	set dest [lindex $args end]
	set src [lreplace $args end end]

	# If we're asked to copy a compressed file to an uncompressed
	# file then uncompress it.  If dest is only specified as a
	# folder, don't uncompress it since we don't know the user
	# wants it uncompressed.  Handle the uncompression at the
	# remote end to avoid permissions problems.
	if {![regexp {/$} $dest] &&
	    [file extension $src] eq ".gz" &&
	    [file extension $dest] ne ".gz"} {
	    set orig $dest
	    set dest "$dest.gz"
	}
	# resolve ~ and other special path components before passing
	# to copy tools.
	set src [file nativename $src]
	if {[info exists UTF::UseRsyncforCopyto]} {
	    set timeout 30
	    set cmd "rsync "
	    if {[info exists UTF::CopytoBW]} {
		append cmd "--bwlimit=$UTF::CopytoBW "
	    }
	    if {![regexp {/$} $src] && [file isdirectory $src]} {
		append src "/"
	    }
	    append cmd "-prlc --progress -e \"$options(-ssh)\" $src "
	    append cmd "$options(-user)@$lan_ip:$dest"
	} else {
	    set timeout 120
	    set cmd "$scp "
	    if {[info exists UTF::CopytoBW]} {
		append cmd "-l $UTF::CopytoBW "
	    }
	    append cmd "-Bqr $src $options(-user)@$lan_ip:$dest"
	}

	if {[catch {localhost rexec -t $timeout $cmd} ret]} {
	    # add one retry to WAR persistent NIS problems
	    if {[regexp {do_ypcall:} $ret]} {
		UTF::Message LOG $options(-name) "Retry"
		localhost rexec -t $timeout $cmd
	    } else {
		error $ret
	    }
	}
	if {[info exists orig]} {
	    $self -noinit gunzip -f $dest
	}

    }

    UTF::doc {
	# [call [arg host] [method copyfrom] [arg src] [arg dest]]

	# Copy file [arg src] on [arg host] to local [arg dest]
	# If src ends in .gz, but dest doesn't then the file will
	# compressed automatically.
    }
    method copyfrom {src dest} {
	if {$src eq "/dev/null"} {
	    file copy -force /dev/null $dest
	    return
	}
	if {$options(-ssh) == ""} {
	    $self connect
	}
	set lan_ip [$self cget -lan_ip]

	_check_fcp

	# If -ssh contains ssh with options, use the options,
	# otherwise just use scp
	if {!([info exists ::UTF::UseFCP] &&
	      [regsub {fsh} $options(-ssh) $fcp scp]) &&
	    ![regsub {ssh} $options(-ssh) {scp} scp]} {
	    set scp scp
	}

	# If we're asked to copy a compressed file to an uncompressed
	# file then uncompress it.  Handle the uncompression at the
	# remote end to avoid permissions problems.
	if {[file extension $dest] eq ".gz" &&
	    [file extension $src] ne ".gz"} {
	    set orig $src
	    set src "$src.gz"
	    $self rexec "gzip -c $orig \> $src"
	}

	localhost rexec -t 600 \
	    $scp -Bqr $options(-user)@$lan_ip:$src $dest
    }

    UTF::doc {
	# [call [arg host] [method rsync] [arg src] [arg dest]]

	# Update files in [arg dest] on [arg host] from local [arg src]
    }
    method rsync {src dest args} {
	if {$options(-ssh) == ""} {
	    $self connect
	}
	set lan_ip [$self cget -lan_ip]
	if {$options(-user) ne ""} {
	    set lan_ip "$options(-user)@$lan_ip"
	}
	localhost rexec \
	    "/usr/bin/rsync -cprlv $args -e \"$options(-ssh)\" $src \"$lan_ip:$dest\""
    }

    UTF::doc {
	# [call [arg host] [method rpmq] [arg args...]]

	# Query RPM database on host.  This is equivalent to "rpm -q
	# $args" except that it also provides a standard format
	# template so that we can get consistent results across
	# different OSs.  This is only applicable to Linux, but needs
	# to be in the UTF::Base type so that it can be used on the
	# special [arg localhost] object.
    }

    method rpmq {args} {
	regsub -line -all {.*Freeing read locks.*\n} \
	    [$self rexec -noinit \
		 "rpm -q --qf \"%{name}-%{version}-%{release}\\n\" $args"] {}
    }

    UTF::doc {
	# [call [arg host] [method connect] [lb][arg -force][rb]]

	# Test connection path to [arg host], setting up authorization
	# if necessary.  Not needed in tests since it is automatically
	# called when needed.  Use [arg -force] to override the cached
	# results and re-test the connection.[para]

        # If fsh gets errors, you can force ssh usage by specifying
        # "-ssh ssh" on the object in the config file.
    }

    method connect {{-force ""}} {
	if {$connect_checked && ${-force} eq ""} {
	    return
	}
	set relay [$self cget -relay]
	if {$relay != ""} {
	    $relay connect ${-force}
	}
	if {$options(-ssh) == ""} {
	    if {[auto_execok ssh] == ""} {
		UTF::Message FAIL $options(-name) \
		    "ssh command not found, please specify with -ssh"
		return 1
	    }
	    set options(-ssh) "ssh"
	    if {[auto_execok fsh] != ""} {
		if {[catch {
		    $self rexec -n -t 60 "in.fshd -V>/dev/null"
		} ret]} {
		    if {[string match "*illed*" $ret]} {
			error $ret
		    } else {
			set fshprob "Unable to use fsh: $ret"
			UTF::Message FAIL $options(-name) $fshprob
		    }
		} else {
		    if {[regexp {Offending.*key .* (/[^:]+):(\d+)} \
			     $ret - file line]} {
			UTF::Message INFO localhost \
			    "Removing offending key $line"
			exec sed -i ${line}d $file
		    }
		    set options(-ssh) "fsh"
		}
	    }
	}
	$self rexec -n -q :
	if {[info exists fshprob]} {
	    # Only report an fsh problem if everything else suceeded.
	    set ::UTF::panic $fshprob
	}
	set connect_checked 1
    }

    UTF::doc {
	# [call [arg host] [method authorize] [lb][opt verbose][rb]]

	# Sets up a users personal ssh keys, if necessary, and
	# propagates them to ssh clients.  Personal ssh keys created
	# under Cygwin may have incorrect permissions for running
	# under Linux, so permissions on personal ssh keys will be
	# checked and corrected.

	# The [opt verbose] flag will provide details.

	# Generally called during client setup.  Not needed in tests.
    }

    method authorize {{verbose {}}} {
	UTF::Message LOG $options(-name) "authorize $verbose"
	if {![file exists ~/.ssh]} {
	    file mkdir ~/.ssh
	}

	if {[catch {exec grep StrictHostKeyChecking [glob ~/.ssh/config]}]} {
	    UTF::Message INFO localhost "Disable strict host key checking"
	    exec echo "StrictHostKeyChecking no" >> ~/.ssh/config
	}
	# Fix up permissions in cae it was created with the wrong
	# umask
	file attributes ~/.ssh/config -permissions og-w

	foreach {type file} {rsa1 identity rsa id_rsa dsa id_dsa} {
	    if {![file exists ~/.ssh/${file}.pub]} {
		exec ssh-keygen -t $type -f \
		    [file nativename ~/.ssh/$file] -N ""
	    }
	    # Fix up permissions in case we first ran this on a Cygwin
	    # controller but are now on a Linux controller.
	    if {[file attributes ~/.ssh/$file -permissions] != 00600} {
		file attributes ~/.ssh/$file -permissions 00600
	    }
	    if {[file attributes ~/.ssh/${file}.pub -permissions] != 00644} {
		file attributes ~/.ssh/${file}.pub -permissions 00644
	    }
	    set f [open ~/.ssh/${file}.pub]
	    lappend keys [string trim [read $f]]
	    close $f
	}
	set shellcmd "test -f ~/.ssh/authorized_keys&&chmod u+w ~/.ssh/authorized_keys"
	foreach key $keys {
	    set shellcmd "$shellcmd
grep \"^$key\" ~/.ssh/authorized_keys >/dev/null 2>&1 || \
echo \"$key\" >> ~/.ssh/authorized_keys"
	}
	if {$verbose ne {}} {
	    UTF::Message INFO $options(-name) "$shellcmd"
	}
	exec bash -c $shellcmd

	set lan_ip [$self cget -lan_ip]
	set ssh $options(-ssh)
	if {$ssh eq {}} {
	    set ssh ssh
	}
	if {[regexp {\m[sf]sh\M} $ssh]} {
	    set shellcmd "chmod og-w ~;test -d ~/.ssh||mkdir ~/.ssh;$shellcmd"
	    set cmd $ssh
	    if {![regexp {\mfsh\M} $ssh]} {
		# disable BatchMode only if we're still using ssh
		lappend cmd -o BatchMode=no
	    }
	    lappend cmd $lan_ip
	    if {$options(-user) != ""} {
		lappend cmd -l $options(-user)
	    }
	    if {$verbose ne {}} {
		UTF::Message INFO $options(-name) "$cmd {$shellcmd}"
	    }
	    if {[catch {exec {*}$cmd $shellcmd} ret]} {
		UTF::Message WARNING $options(-name) $ret
		if {[regexp -line {Offending.*key .* (/[^:]+):(\d+)} \
			 $ret - file line]} {
		    UTF::Message INFO localhost \
			"Removing offending key $line"
		    exec sed -i ${line}d $file
		    catch {exec {*}$cmd $shellcmd} ret
		    UTF::Message WARNING $options(-name) $ret
		}
	    } else {
		UTF::Message LOG $options(-name) $ret
	    }
	} else {
	    set ret $keys
	}
	return $ret
    }

    UTF::doc {
	# [call [arg host] [method ipaddr] [arg dev]]

	# Return IP address of device [arg dev] or Error if device is
	# down
    }

    method ipaddr {dev} {
	if {[regexp {inet (?:addr:)?([0-9.]+)} [$self ifconfig $dev] - addr]} {
	    return $addr
	} else {
	    error "$options(-name) No IP address available"
	}
    }

    UTF::doc {
	# [call [arg host] [method tcptune] [arg window]] ]

	# Configure host tcp window size.  [arg window] indicates
	# desired TCP Window size in bytes.  A k suffix can be used to
	# indicate KB instead.

	# Returns 1 if userland tools need to specify the window size
	# themselves.
    }

    method tcptune {window} {
	# Userland tools need to specify window size.
	return 1
    }

    UTF::doc {
	# [call [arg host] [method udptune] [arg window]] ]

	# Configure host udp window size.  [arg window] indicates
	# desired UDP Window size in bytes.  A k suffix can be used to
	# indicate KB instead.

	# Returns 1 if userland tools need to specify the window size
	# themselves.
    }

    method udptune {window} {
	# Userland tools need to specify window size.
	return 1
    }

    UTF::doc {
	# [call [arg host] [method reclaim]]

	# Default is no reclaim
    }
    method reclaim {} {
	return 0
    }

    UTF::doc {
	# [call [arg host] [method rte_available]]

	# Stub for host objects with no RTE console support.
    }

    method rte_available {} {
	return 0
    }

    UTF::doc {
	# [call [arg host] [method socket] [arg host] [lb][arg
	# port][rb]]

	# Stub for relay client socket creation.  Used by localhost,
	# which doesn't use relays so this is the same as calling the
	# [cmd socket] built-in.  server sockets (listeners) are not
	# supported.  [arg host] may be of the form "host:port", which
	# will override the seperate [arg port] argument.  Port may be
	# qualified with /tcp (default) or /udp.
    }

    method socket {host {port 23}} {
	regexp {(.*):(.*)} $host - host port
	set proto "tcp"
	regexp {(.*)/(.*)} $port - port proto
	if {$proto eq "tcp"} {
            UTF::Message LOG $options(-name) "socket $host $port"
            socket $host $port
        } elseif {$proto eq "udp"} {
	    # 99 sec is the largest timer compatible with both nc 1.x and 6.x
	    localhost rpopen -n -rw nc $host -u -w 99 $port
	} else {
	    error "unknown protocol: $proto"
	}
    }

    UTF::doc {
	# [call [arg host] [method power] [arg op] [lb][arg debug][rb]
	# [lb][arg power_option][rb] [lb][arg voltage][rb]]

	# Run [cmd power] command on the host to control the host
        # power.  [option op] [arg on|off|cycle] specifies the power
        # action performed on the host. There is no default
        # op. [option debug] [arg 0|1] is deprecated, since all
        # transactions are logged anyway.  [arg power_option]
        # indicates which $host option contains the power control
        # information.  By default this is [option -power] to control
        # the host, but for controling auxiliary devices it may
        # contain a different option, such as [option -power_sta]
        # Optional [arg voltage] is useful for Agilent power
        # controllers where the output voltage can be explicitly
        # controlled.[para]

        # NB: For Laptops, you need to remove the
        # battery(s). Otherwise the power control will not have any
        # effect.
    }

    # Common power control routine
    method power {op {debug 0} {power_option -power} {voltage ""}} {
	# Debug is redundant since we always log but leave it in for
	# now for compatibiity
        # puts "power op=$op debug=$debug power_option=$power_option\
        #    voltage=$voltage"
	if {[lsearch {on off cycle} $op] < 0} {
	    error "power $op != on|off|cycle"
	}
	if {![info exists options($power_option)]} {
	    error "Invalid power option $power_option"
	} else {
	    set power_data $options($power_option)
	}

	if {$power_data eq ""} {
	    error "power $op: $power_option not configured"
	}

        # Show details just for -power. Other methods have their own
        # log messages.
        if {$power_option == "-power"} {
            UTF::Message LOG "$options(-name)" "Power ($power_data) $op"
        }

	set host [lindex $power_data 0]
	set pargs [lreplace $power_data 0 0]

        # If first argument is a TCL command then assume it's a power
	# switch object.
	if {[info command [lindex $power_data 0]] ne ""} {
	    return [$host power $op {*}$pargs {*}$voltage]
	}

	# Fall back to original apshell mechanism.
	UTF::Message WARN $options(-name) "Legacy power tools"
	# Note: -relay here will only work for Routers and other
	# similar objects that use -relay for command execution.  For
	# objects like Dongles and Linux which only use -relay for
	# executing apshell the -relay option won't have propagated
	# down this far.  That is correct behaviour, albeit confusing.
	# Longer term I intend to move apshell logging back onto the
	# local host for these cases and return -relay to it's proper
	# use for only inaccessible devices.
	if {$options(-relay) ne ""} {
	    set relay $options(-relay)
	} else {
	    set relay localhost
	}
	$relay $UTF::unittest/apshell $host power $op $pargs
    }

    UTF::doc {
	# [call [arg host] [method power_button]  [lb][arg -shutdown][rb]]

	# Run [cmd power_button] command on the host to push the
        # remote controlable power button for 0.5 seconds and then
        # release it.[para]

        # The [arg -shutdown] option is used to hold the power button
        # down for 10 seconds. On some PCs, notably HP, this will
        # force the PC to power itself off. This option can be use to
        # conserve the number of ports used on your remote power
        # controller. If the power button will reliably force the PC
        # to power off, no matter how badly crashed the PC is, then
        # you no longer need to have a power controller port to drop
        # the 110VAC power to the PC. You can control the PC
        # completely via the power button.
    }

    method power_button {args} {

        # Get object data
        set name [$self cget -name]
        set power_data [$name cget -power_button]
        # puts "power_button self=$self name=$name power_data=$power_data args=$args"

        # If power_data is null or manual, script is in trouble, as the
        # human has to go to the lab and manually push the button.
        set power_data [string trim $power_data]
        if {$power_data == "" || [string match -nocase *manual* $power_data]} {
            error "power_button ERROR: You need to manually push the power\
               button for $name"
        }

        # If the power_data is auto, the device recovers with no further
        # action required here.
        if {[string match -nocase *auto* $power_data]} {
            UTF::Message LOG $name "Device powers up automatically"
            return
        }

        # The -shutdown option determines how long the power button is held
        # down before it is released.
        if {[string match -nocase *shutdown* $args]} {
            set delay 10
        } else {
            set delay 0.5
        }

        # NB: The null token below is the deprecated debug option.
        # It must be there because the token parsing by method power
        # is positional, and will mess up if it is not present.
        UTF::Message LOG $name "Power button ($power_data) depressed"
        $self power on "" -power_button
        UTF::Sleep $delay $name "$args"
        UTF::Message LOG $name "Power button ($power_data) released"
        $self power off "" -power_button
    }

    UTF::doc {
	# [call [arg host] [method power_sta] [arg op]
	# [lb][arg power_option][rb] [lb][arg voltage][rb]]

	# Run [cmd power_sta] command on the host to control the STA power.
        # Calling parameters are the same as the method power.
    }

    method power_sta {op {power_option ""} {voltage ""}} {
        # Call the common power method with appropriate options.
        # NB: The null token below is the deprecated debug option.
        # It must be there because the token parsing by method power
        # is positional, and will mess up if it is not present.
        set power_option [string trim $power_option]
        if {$power_option == ""} {
            set power_option "-power_sta"
        }
        set name [$self cget -name]
        set power_sta [$self cget $power_option]
        UTF::Message LOG $name "Power STA ($power_sta) $op $voltage"
        $self power $op "" $power_option $voltage
    }

    UTF::doc {
	# [call [arg host] [method device_reset]]

	# Run [cmd device_reset] command on the host to generate a hardware
        # reset on the specified device.[para]
    }

    method device_reset { } {

        # Get object data
        set name [$self cget -name]
        set reset_data [$name cget -device_reset]
        # puts "device_reset self=$self name=$name reset_data=$reset_data"

        # If reset_data is null, the script is in trouble, as the
        # human has to go to the lab and ground a jumper, then put
        # it back.
        set reset_data [string trim $reset_data]
        if {$reset_data == ""} {
            error "device_reset ERROR: You have NOT specified the power\
                controller name & port in the -device_reset option, so\
                you need to manually reset the $name device!"
        }

        # NB: The null token below is the deprecated debug option.
        # It must be there because the token parsing by method power
        # is positional, and will mess up if it is not present.
        UTF::Message LOG $name "Device reset ($reset_data) start"
        $self power on "" -device_reset
        UTF::Sleep 1 $name
        UTF::Message LOG $name "Device reset ($reset_data) end"
        $self power off "" -device_reset

        # Wait for wall wart to discharge & device to reset.
        UTF::Sleep 3 $name
    }


    UTF::doc {
	# [call [arg host] [method arasan] [arg voltage]]

	# Run [cmd arasan] command on the host to set the [arg voltage]
        # used by the Arasan card. [arg voltage] can be: 3.3 or 1.8 [para]

        # A 3-wire connection is needed from the Arasan board to
        # the WebRelay contacts. The Normally Closed relay contact
        # is expected to be connected to the 3.3V power supply on
        # the Arasan card. The Normally Open contact is expected to
        # be connected to the 1.8V power supply on the Arasan card.
    }

    method arasan { voltage } {

        # Get object data
        set name [$self cget -name]
        set arasan_data [$name cget -arasan]
        # puts "arasan self=$self name=$name arasan_data=$arasan_data"

        # If arasan_data is null, the script is in trouble, as the
        # human has to go to the lab and flip a switch.
        set arasan_data [string trim $arasan_data]
        if {$arasan_data == ""} {
            error "arasan ERROR: You have NOT specified the power\
                controller name & port in the -arasan option, so\
                you need to manually flip the switch on the $name\
                arasan card!"
        }

        # Set the desired voltage.
        set voltage [string trim $voltage]
        set voltage [string toupper $voltage]
        if {$voltage == "3.3"} {

            # NB: The null token below is the deprecated debug option.
            # It must be there because the token parsing by method power
            # is positional, and will mess up if it is not present.
            UTF::Message LOG $name "Arasan ($arasan_data) set to $voltage V"
            $self power off "" -arasan

        } elseif {$voltage == "1.8"} {
            UTF::Message LOG $name "Arasan ($arasan_data) set to $voltage V"
            $self power on "" -arasan
        } else {
            error "arasan ERROR: invalid voltage=$voltage, must be 3.3 or 1.8"
        }
    }

    UTF::doc {
	# [call [arg host] [method shutdown_reboot] [arg t1]
        # [arg t2] [arg cmd1] [arg cmd2] [lb][opt -force][rb] \
	#	  [lb][opt -noreboot][rb]]

	# Powers off the external STA power, if any, gracefully shuts
        # down the OS,turns the power off for the host, turns the
        # power back on for the host, pushes the host power button, if
        # any, and waits for the OS to reload. The OS is deemed to be
        # reloaded when it will respond to "rexec uname -a".  Finally
        # the external STA power is turned back on.  Also controls the
        # arasan card power setting, if any, during the boot
        # process.[para]

        # Some PC can be totally controlled by their power button and
        # don't need the 110 / 220 VAC connected to a remote power
        # controller. This routine can handle this scenario as
        # well.[para]

        # If you know in advance that the host is unresponsive, or are
        # not concerned about proper software shutdown procedures, you
        # can specify option -force to bypass the OS shutdown.[para]

        # If you want to shutdown the host without rebooting it, you
        # can specify option -noreboot to leave the host in a powered
        # off state.[para]

        # This common method is used by the Cygwin & Linux objects in
        # their respective shutdown_reboot methods. Each of these
        # methods supplies appropriate default calling parameters,
        # which are OS specific. Each method allows the user to
        # override the default values.[para]

        # [arg t1] Time in seconds to wait for the OS to gracefully
        # shut down.
        # [arg t2] Time in seconds to wait after the host has been
        # powered off.
        # [arg cmd1] Command string needed to gracefully shut down the
        # OS.
        # [arg cmd2] For those less-capable hosts that do NOT have
        # remote power control, this is the command string needed to
        # do a soft reboot of the host. While this is not as reliable
        # as power cycling the host, its better than nothing.
    }

    method shutdown_reboot {t1 t2 cmd1 cmd2 args} {
        # Save method start time
        set method_start_sec [clock seconds]

        # NB: Throughout this method, there are time delays to wait
        # for specific events to be recognized by the host
        # hardware. These time delays also benefit the power
        # controllers, WTI in particular.  WTI can not handle multiple
        # requests with 0 time delay between them. If the requests to
        # the WTI are too fast, the socket connection is refused. When
        # making changes to the time delays here, or in the order in
        # which power commands are issued, make sure there is at least
        # 0.5 seconds delay between requests sent to the WTI
        # controller to keep this method reliable.

        # Sanity checks on t1 & t2. NB: cmd1 & cmd2 are allowed to be
        # null.
        set t1 [string trim $t1]
        if {$t1 == "" || $t1 < 30} {
            set t1 30
        }
        set t2 [string trim $t2]
        if {$t2 == "" || $t2 < 5} {
            set t2 5
        }

        # Get object name & lan_ip. lan_ip is used for ping command.
        set name [$self cget -name] ;# get the high level object name
        set name [string trim $name]
        set lan_ip [$self cget -lan_ip]
        UTF::Message LOG $name "shutdown_reboot self=$self name=$name\
            lan_ip=$lan_ip t1=$t1 t2=$t2 cmd1=$cmd1 cmd2=$cmd2 args=$args"

        # Setup ping options based on local OS. NB: We need the OS of the
        # local machine where this script is running, not the machine that
        # is being shutdown / rebooted.
        set local_os $::tcl_platform(os)
        # set local_os sfsf ;# test code
        if {[string match -nocase "*linux*" $local_os]} {
            set ping_options "-c 2" ;# Linux
        } else {
            set ping_options "-n 2" ;# Windows
        }
        # puts "local_os=$local_os ping_options=$ping_options"

        # There have been drivers that cause the host to not boot. One trick
        # in dealing with this situation is to disconnect / power off the STA
        # first. This may let the host boot.
        # Power off the optional STA.
        set power_sta [$self cget -power_sta]
        set power_sta [string trim $power_sta]
        if {$power_sta != ""} {
            UTF::Message LOG $name "shutdown_reboot powering off STA\
                on host $name"
            $self power_sta off
            # We turn the STA power back on at the very end of this routine
            UTF::Sleep 1 $name "Be nice to power controller"
        }

        # Some hosts have an Arasan card that needs to be at 3.3V while
        # Linux is booting, and then back to 1.8V for the rest of the time.
        set arasan_data [$name cget -arasan]
        set arasan_data [string trim $arasan_data]
        if {$arasan_data != ""} {
            $self arasan 3.3
        }

        # Check that we can power off the host, either by dropping the
        # 110VAC / 220VAC power or via power_button control.
        set power_host [$self cget -power]
        set power_host [string trim $power_host]
        set power_button [$self cget -power_button]
        set power_button [string trim $power_button]
        if {$power_button == "" || [string match -nocase *manual* $power_button] ||\
            ($power_host == "" && [string match -nocase *auto* $power_button])} {

            # NB: The -noreboot options does not apply to hosts that
            # have no remote power control. This prevents hosts from
            # being accidentally powered off and requiring manual
            # intervention in a far away lab.

            # There have been cases where a host was in a weird state
            # and an rexec command took over 2 hours to time out. So we
            # ping the host first before we try the desired command.
            set catch_resp [catch "exec ping $lan_ip $ping_options" catch_msg]
            if {$catch_resp == 0} {

                # Host responded OK to ping. This host does NOT have remote
                # power control, so we use cmd2 in the faint hope that we can
                # do a soft reboot of the host.
                UTF::Message LOG $name "shutdown_reboot doing soft reboot on\
                    host $name"
                set catch_resp [catch {$self rexec -noinit $cmd2} catch_msg]
                if {$catch_resp != 0} {
                    UTF::Message WARN $name "shutdown_reboot cmd2=$cmd2\
                        catch_msg=$catch_msg"
                }
                UTF::Sleep $t1 $name "Waiting for soft reboot to take effect"

            } else {
                UTF::Message LOG $name "Soft reboot not done as ping failed:\
                    $catch_msg"
            }

        } else {
            # This host does have remote power control.

            # Gracefully shut down the OS. We skip this step if the -force
            # option was specified.
            if {[string match -nocase "*force*" $args]} {
                UTF::Message LOG $name \
		    "shutdown_reboot skipping OS shutdown, args=$args"

            } else {

                # There have been cases where a host was in a weird state
                # and an rexec command took over 2 hours to time out. So we
                # ping the host first before we try the gracefull shutdown
                # command.
                set catch_resp [catch "exec ping $lan_ip $ping_options" catch_msg]
                if {$catch_resp == 0} {

                    # Host responded OK to ping. # Keep going after the gracefull
                    # shutdown command even if there are issues.
                    UTF::Message LOG $name "shutdown_reboot shutting down OS on host $name"
                    set catch_resp [catch {$self rexec -noinit $cmd1} catch_msg]
                    if {$catch_resp != 0} {
                        UTF::Message WARN $name "shutdown_reboot cmd1=$cmd1\
                            catch_msg=$catch_msg"
                    }
                    UTF::Sleep $t1 $name "Waiting for OS to gracefully shut down"

                } else {
                    UTF::Message LOG $name "Gracefull shutdown not done as ping\
                        failed: $catch_msg"
                }
            }

            # Power off the host. This occurs regardless of prior issues.
            UTF::Message LOG $name "shutdown_reboot powering off host $name"
            if {$power_host != ""} {
                # Use the 110VAC / 220VAC power control for the host.
                $self power off

            } else {
                # Hold the power button down to force the host to
                # power off. We no longer always push the button a second
                # time for 10 seconds. The Dell E6400 starts booting on
                # the first long button push. The second long button push
                # forces it off again. But then Win7 loader shows a
                # menu choice of "repair disk" or "start normally" and
                # the default is to "repair disk". From there, manual
                # intervention is required to click on the "Restore"
                # button and later on the "Finish" button.

                # However, some (older) PC really still need a second
                # long push of the power button. So the compromise is
                # to have the option -power_button_pushes which defaults
                # to 1, and is configurable per PC.
                set power_button_pushes [$name cget -power_button_pushes]
                set power_button_pushes [string trim $power_button_pushes]
                if {![regexp {^\d+$} $power_button_pushes] || \
                    $power_button_pushes == "" || $power_button_pushes < 1} {
                    UTF::Message WARN $name "shutdown_reboot invalid\
                        power_button_pushes=$power_button_pushes, should be\
                        integer 1 or greater, set to 1"
                    set power_button_pushes 1
                }
                for {set i 1} {$i <= $power_button_pushes} {incr i} {
                    $self power_button -shutdown
                    if {$i < $power_button_pushes} {
                        UTF::Sleep 5
                    }
                }
            }

            # If user requested that host be left in the powered off state,
            # then we use pings to check the host is NOT responding.
            if {[string match -nocase "*noreboot*" $args]} {
                UTF::Message LOG $name "shutdown_reboot checking and leaving host in powered off state, args=$args"
                set catch_resp [catch "exec ping $lan_ip $ping_options" catch_msg]
                if {$catch_resp == 0} {
                    # pings succeeded
                    error "shutdown_reboot host $name did NOT power off, it is still responding to pings!"
                } else {
                    # pings failed
                    UTF::Message LOG $name "shutdown_reboot host $name powered off OK, there is NO response to pings!"
                    return
                }
            }

            # Wait t2 seconds. This is what some host need in order to
            # decide there was a power failure and automatically start
            # up again. This also benefits WTI power controller.
            UTF::Sleep $t2 $name "shutdown_reboot wait for BIOS to recognize\
                there is a power failure"

            # Power up the host and push the power button.
            if {$power_host != ""} {
                UTF::Message LOG $name "shutdown_reboot powering on host $name"
                $self power on
                # This also benefits WTI power controller.
                UTF::Sleep 5 $name "shutdown_reboot wait for BIOS to stabilize\
                    after power on."
            }
            UTF::Message LOG $name "shutdown_reboot pushing power button for\
                host $name"
            $self power_button
        }

        # Now we wait in a loop for the OS to boot and respond to commands.
        UTF::Sleep 60 $name "shutdown_reboot waiting for OS to load"
        set host_responded no
        set max_sec 420
        set max_tries 100
        for {set j 0} {$j < 2} {incr j} {
            set start_sec [clock seconds] ;# reset for each j iteration.
            for {set i 1} {$i <= $max_tries} {incr i} {

                # When we can run the uname command, the OS is largely loaded.
                # There have been cases where a host was in a weird state
                # and an rexec command took over 2 hours to time out. So we
                # ping the host first before we try the uname command.
                set catch_resp [catch "exec ping $lan_ip $ping_options" catch_msg]
                if {$catch_resp == 0} {

                    # Host responded OK to ping. It is probably safe to try the
                    # uname command.
                    UTF::Message LOG $name "j=$j i=$i shutdown_reboot host responded to ping"
                    set catch_resp [catch "set info \[$self rexec -noinit -timeout 30 -Timeout 45 uname -a\]" catch_msg]
                    set now_sec [clock seconds]
                    set elapsed_sec [expr $now_sec - $start_sec]
                    if {$catch_resp == 0} {
                        set host_responded yes
                        break
                    }

                } else {
                    UTF::Message LOG $name "j=$j i=$i uname not done as ping\
                        failed: $catch_msg"
                    set now_sec [clock seconds]
                    set elapsed_sec [expr $now_sec - $start_sec]
                }

                # Check if we have exceed the max_sec allowed.
                UTF::Message LOG $name "shutdown_reboot j=$j i=$i elapsed_sec=$elapsed_sec waiting for OS to load"
                if {$elapsed_sec > $max_sec} {
                    break
                }

                # Wait and try again.
                if {$i < $max_tries} {
                    UTF::Sleep 30
                }
            }

            # Did host respond? If yes, break outer loop.
            if {$host_responded == "yes"} {
                break
            }

            # Last part of outer loop is done only once.
            if {$j != 0} {
                break
            }

            # If host has no power_button control, we are done.
            if {$power_button == "" || [string match -nocase *auto* $power_button]} {
                break
            }

            # At this point the host did not respond after 10 minutes, so its almost
            # certainly powered off. We will try one more short reboot cycle. This will
            # deal with the Dell E4200 & E6400 that dont always shutdown when the power
            # button is pressed for 10 seconds. So push the button again and allow 3
            # minutes to reboot.
            UTF::Message LOG $name "j=$j shutdown_reboot pushing power button for\
                host $name"
            $self power_button
            set max_sec 180
        }

        # Set Arasan card back to 1.8V.
        if {$arasan_data != ""} {
            $self arasan 1.8
        }

        # Conditionally power on the optional STA. There was lots of delay from last
        # command sent to WTI power controller.
        if {$power_sta != ""} {
            if {$host_responded == "yes"} {
                UTF::Message LOG $name "shutdown_reboot powering on STA on host $name"
                $self power_sta on
            } else {
                UTF::Message LOG $name "shutdown_reboot leaving STA on host $name\
                    powered off in case of BSOD loop."
            }
        }

        # Did we get a response from the OS?
        set elapsed_sec [expr [clock seconds] - $method_start_sec]
        if {$host_responded == "yes"} {
            UTF::Message LOG $name "shutdown_reboot succesfull, j=$j i=$i elapsed_sec=$elapsed_sec"
            return
        } else {
            error "shutdown_reboot ERROR: Timed out waiting for $name to load the OS! j=$j i=$i elapsed_sec=$elapsed_sec"
        }
    }

    UTF::doc {
	# [call [arg staname] [method ping] [arg target]
	#	[lb][opt -c] [arg count][rb]
        #       [lb][opt -s] [arg size][rb]]

	# Ping from [arg staname] to [arg target].  Target may be a
	# STA or a Hostname/IP address.  Returns success if a response
	# packet is received before [arg count] (default 5) packets
	# have been sent.  Packet data size can be set with [arg size]
	# (default 56 bytes).
    }

    typevariable DefaultPingCount 5

    method ping {target args} {
	set msg "ping $target $args"
	UTF::Getopts [subst {
	    {c.arg "$DefaultPingCount" "Count"}
	    {s.arg "56" "Size"}
	}]
	if {[info commands $target] == $target &&
	    [$target info type] eq "::UTF::STA"} {
	    # If we're pinging an object, it's worth logging who the
	    # object was because it may not be obvious when we are
	    # caching IP addresses.
	    UTF::Message LOG $options(-name) $msg
	    set target [$target ipaddr]
	}
	set cmd [string map [list %s $(s) %c $(c) %t $target] \
		     [$self cget -ping]]
	lappend cmd $target
	# Ignore return data, propagate errors
	# Minimum 30 second timeout since Windows is sometimes just
	# very slow to start the ping.
	if {[catch {$self rexec -t [expr {$(c) + 30}] $cmd} ret] ||
	    [regexp {100% packet loss} $ret]} {
	    error "ping failed"
	}
	return
    }


    # WAN port mapping table
    variable portmap_table
    method {portmap get} {ip port tcp} {
	if {![info exists portmap_table]} {
	    $self portmap read
	}
	if {[info exists portmap_table([list $ip $port $tcp])]} {
	    set ret $portmap_table([list $ip $port $tcp])
	    #UTF::Message DBG $options(-name) "portmap $ip $port $tcp -> $ret"
	} else {
	    set ret [list $ip $port]
	}
	set ret
    }
    method {portmap set} {ip port tcp dip dport {count 1}} {
	for {set i 0} {$i < $count} {incr i} {
	    set portmap_table([list $ip $port $tcp]) [list $dip $dport]
	    incr port
	    incr dport
	}
    }
    method {portmap write} {} {
	set fd [$self rpopen -in -rw "cat > .portmap_table"]
	foreach n [lsort [array names portmap_table]] {
	    #UTF::Message LOG $options(-name) "[list $n] [list $portmap_table($n)]"
	    puts $fd "[list $n] [list $portmap_table($n)]"
	}
	close $fd
    }
    method {portmap read} {} {
	if {[catch {$self -q test -f .portmap_table}]} {
	    set portmap_table(none) ""
	} else {
	    array set portmap_table [$self rexec -s "cat < .portmap_table"]
	}
    }

    UTF::doc {
	# [call [arg host] [method wlname] [arg dev]]

	# Return wl<n> name corresponding to this device.  Default is
	# no translation.
    }

    method wlname {dev} {
	return $dev
    }

    method whatami {STA} {
	if {[catch {$STA chipname} c]} {
	    UTF::Message WARN $options(-name) $c
	    set c "<unknown>"
	}
	return "[$STA hostis] $c"
    }

    UTF::doc {
	# [call [arg host] [method UTF::worry] [arg msg]]

	# Low level panic.  Doesn't overwite any existing panic.  To
	# suppress the automatic logging of the fail message (eg, if
	# it has already been logged) use [arg [emph -]msg].
    }

    method worry {msg {name ""}} {
	if {$name eq ""} {
	    set name $options(-name)
	}
	if {![regsub {^-} $msg {} msg]} {
	    UTF::Message FAIL $name $msg
	}
	if {![info exists ::UTF::panic] || [regexp {reinit} $::UTF::panic]} {
	    set ::UTF::panic $msg
	}
    }

    UTF::doc {
	# [call [arg host] [method UTF::warn] [arg msg]]

	# Warning.  Doesn't overwite any existing warning.  To
	# suppress the automatic logging of the warning message (eg,
	# if it has already been logged) use [arg [emph -]msg].
    }

    method warn {msg {name ""}} {
	if {$name eq ""} {
	    set name $options(-name)
	}
	if {![regsub {^-} $msg {} msg]} {
	    UTF::Message WARN $name $msg
	}
	if {![info exists ::UTF::warn]} {
	    set ::UTF::warn $msg
	}
    }

    # array to hold counters.  Keys will be the match expressions
    variable err_counter

    method bunch {match num msg} {
	# Count contiguous errors, if they exceed num report it as an
	# error, otherwise reset counter.  Needs to be a method to get
	# access to counter array
	if {![info exists err_counter($match)]} {
	    set err_counter($match) 0
	}

	if {[regexp $match $msg]} {
	    incr err_counter($match)
	    if {$err_counter($match) > $num} {
		$self warn $msg
		return 1
	    }
	} elseif {$err_counter($match) > 0} {
	    set err_counter($match) 0
	}
	return 0
    }

    method expect_reinit {val} {
	set expect_reinit $val
    }

    # Message handling callback.  Used by "host" to process log
    # messages.  Returns true if no further processing is required.
    method common_getdata {msg {name ""}} {
	if {$msg eq ""} {
	    # Nothing to do
	    return 1
	}
	if {$name eq ""} {
	    set name $options(-name)
	}

	# Custom message handling
	foreach {pattern action} [UTF::decomment $options(-msgactions)] {
	    if {[regexp $pattern $msg]} {
		switch $action {
		    PASS {
			UTF::Message LOG $name $msg
			return 1
		    }
		    WARN {
			$self warn $msg $name
			return 1
		    }
		    FAIL {
			$self worry $msg $name
			return 1
		    }
		    default {
			# custom handler.  Return 1 if message has
			# been fully handled, 0 if it should undergo
			# further processing.
			if {[catch $action ret] == 1} {
			    UTF::Message WARN $name $ret
			    return 0
			}
			return $ret
		    }
		}
	    }
	}

	if {[info exists ::UTF::CountedErrors] &&
	    ([$self bunch {wlc_recv: dropping a frame with invalid src} 5 $msg] ||
	     [$self bunch {wlc_recvfilter: invalid class 2 frame} 5 $msg] ||
	     [$self bunch {wlc_wsec_recvdata_decrypt ICV error} 5 $msg] ||
	     [$self bunch {wlc_exptime_start: packet lifetime} 30 $msg] ||
	     [$self bunch {quiet: rcvd but ignoring:count } 5 $msg])} {
	    return 1
	}
	if {[regexp {(.*_dotxstatus.*: tx phy error )\((0x[[:xdigit:]]+)\)} \
		 $msg - msg err]} {

	    # Legacy Tx PhyError decoding - only works for corerev <
	    # 42.  Later corerevs should have a more accurate report
	    # captured in the next block.

	    append msg "($err=[UTF::txphyerr $err])"
	    UTF::Message FAIL $name $msg
	    after 1000 [list $self worry -$msg]
	    return 1
	} elseif {[regexp -nocase -expanded \
		       {
			   \#{11}\ ARM\ Hung\ !!!\ \#{4}
		       } $msg]} {
	    UTF::Message FAIL $name $msg
	    after 1000 [list $self worry -$msg]
	    return 1
	} elseif {[regexp -expanded \
		       {DMA64:\ } $msg]} {
	    $self warn $msg $name
	    return 1
	} elseif {[regexp { tx_in_transit [1-9]\d.* fifordy 0x0} $msg] &&
		  ![regexp { cpbusy 0x[1-9a-f]} $msg]} {
	    $self warn $msg $name
	    return 1
	} elseif {[regexp {reinit reason|fatal error, reinit} $msg]} {
	    if {$expect_reinit} {
		UTF::Message PASS $name $msg
	    } else {
		$self worry $msg $name
	    }
	    return 1
	} elseif {[regexp -nocase -expanded \
		       {
			   Tx\ PhyErr\ 0x|
			   PSMx?\ microcode\ watchdog\ fired|
			   PSMx?\ watchdog\ at\ |
			   \ txstuck\ at\ |
			   non\ recoverable\ Error|
			   Memory\ leak\ |
			   malloced\ 0\ bytes|
			   bug:\ kernel|
			   ASSERT|
			   Error\ getting\ the\ last\ error|
			   EIP\ is\ at\ |
			   ^PC\ is\ at\ |
			   ^RIP:\ |
			   Kernel\ panic|
			   kernel\ BUG|
			   wlc_suspend_mac_and_wait:\ waited|
			   wlc_bmac_suspend_mac_and_wait:\ waited|
			   MQ\ ERROR\ |
			   TKIP\ MIC\ failure|
			   _dotxstatus:\ unexpected\ tx\ status|
			   wlc_rpc_high_dispatch:\ PKTGET\ failed|
			   COEX:\ downgraded\ chanspec|
			   unsupported\ WPA2\ version|
			   !!!rpc_osl_wait\ \d+\ ms\ failed!!!|
			   cannot\ stop\ dma|
			   ERROR:\ wlc_attach_module:.*\ module/err\ num|
			   RPC\ Establish\ failed\ due\ to\ version\ mismatch|
			   \ in\ persist\ block:|
			   :\ reason\ =\ \w+\.\ corerev|
			   CHIP\ NOT\ GOING\ TO\ DEEPSLEEP!
		       } $msg]} {
	    $self worry $msg $name
	    return 1
	}
	return 0
    }


    method setup_iperf {} {
	# no-op
    }

    method wlconf_by_nvram {} {
	# Routers, etc should override this if they use nvram for wl
	# configuration.
	return $options(-wlconf_by_nvram)
    }

    method pre_reclaim {} {
	# no-op
    }

    UTF::doc {
	# [call [arg host] [method *] [lb][arg {args ...}][rb]]

	# Other commands are passed on to the host to be executed as
	# if the [method rexec] method had been used.  Note this makes
	# the [method rexec] method optional for simple commands.

	# [example_begin]
	$host ls -l
	# [example_end]

	# This technique cannot be used if the first word contains
	# spaces.  This means that complex commands, such as shell
	# scripts, still need to use rexec. eg:
	# [example_begin]
    } {{
	$host rexec {
	    if [ -f file ]; then
	        echo exists
	    fi
	}}} {
	# [example_end]
    }

    # Delegate unknown commands to the "rexec" method, thereby making
    # the rexec method name optional.
    delegate method * using {%s rexec %m}
}

# Retrieve manpage from last object
UTF::doc [UTF::Base man]

UTF::doc {
    # [list_end]

    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also wl]
    # [see_also [uri APdoc.cgi?UTF.tcl UTF]]
    # [see_also [uri APdoc.cgi?UTF::Cygwin.tcl UTF::Cygwin]]
    # [see_also [uri APdoc.cgi?UTF::Router.tcl UTF::Router]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
