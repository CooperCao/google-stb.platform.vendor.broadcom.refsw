#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: aced1ab01467dfd352d5b148145a78144264b9a7 $
# $Copyright Broadcom Corporation$
#

package provide UTF::HSIC 2.0

package require snit
package require UTF::doc
package require UTF::Router

UTF::doc {
    # [manpage_begin UTF::BSDAP n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF netbsd AP support}]
    # [copyright {2010 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::BSDAP host object, specific to APs running netbsd.

    # Once created, the BSDAP object's methods are not normally
    # invoked directly by test scripts, instead the BSDAP object is
    # designated the host for one or more platform independent STA
    # objects and it will be the STA objects which will be refered to
    # in the test scripts.

    # [list_begin definitions]

}

snit::type UTF::HSIC {
    UTF::doc {
	# [call [cmd UTF::BSDAP] [arg host] [arg ...]]

	# Create a new BSDAP host object.  The host will be
	# used to contact STAs residing on that host.
	# [list_begin options]

	# [opt_def [arg ...]]

	# There are no BSDAP specific options.  All options will be
	# passed on to the [cmd UTF::Router] object.

	# [list_end]
	# [list_end]

	# [para]
	# BSDAP objects have the following methods:
	# [para]
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    # base handles any other options and methods
    component base -public host -inherit yes

    option -name -configuremethod CopyOption

    delegate option -host_nvram to base as -nvram
    delegate option -host_brand to base as -brand
    delegate option -host_tag to base as -tag

    option -nvram bcm94330OlympicUNO3.txt
    option -nvram_add
    option -type 4330b0-roml/usb-g-pool-nodis
    option -driver dhd-cdc-usb-hsic-gpl-2.6.22
    option -tag FALCON_REL_5_90_??
    option -brand linux-internal-dongle-usb
    option -customer bcm
    option -sn 99
    option -device
    option -rwlrelay ""
    option -nolinks -type snit::boolean -default 0
    option -preinstall_hook ""

    # Note this deliberately hides the Router copy; otherwise
    # restore_defaults will fail
    option -wlinitcmds
    option -postinstall

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	install base using UTF::Router %AUTO% -relay dummy \
	    -brand linux26-internal-hsic -tag RTRFALCON_REL_5_130_* \
	    -iperfdaemon 0 -escan 1 \
	    -msgcallback [mymethod msgcallback] -lan_ip ""
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	if {[$self cget -lan_ip] == ""} {
	    # If IP address isn't specified, use host name
	    $self configure -lan_ip $options(-name)
	}
	foreach {sta dev} [$self cget -sta] {
	    if {$dev eq ""} {
		error "$sta has no device name"
	    }
	    UTF::STA ::$sta -host $self -device $dev

	    if {$options(-device) eq ""} {
		# Need at least one device name for loading driver.
		# If not specified then use the first STA.  We only
		# support one physical device, but there may also be
		# virtual devices defined.
		set options(-device) $dev
	    }
	}
	if {$options(-rwlrelay) eq "localhost"} {
	    $base configure -wl {wl --socket localhost}
	}
    }

    # Internal method for copying options to delegates
    method CopyOption {key val} {
	set options($key) $val
	$base configure $key $val
    }

    method findimages {args} {

	if {$args == "" || $args == "-all"} {
	    set args [concat $args [$self cget -image]]
	}

	# Findimages start
	UTF::Message LOG $options(-name) "findimages $args"

	# Defaults from object
	UTF::GetKnownopts [subst {
	    {all "return all matches"}
	    {ls "Report ls -l"}
	    {showpattern "Show search patterm"}
	    {nvram.arg "" "ignored"}
	    {driver.arg "$options(-driver)" "Host Driver"}
	    {brand.arg "[$self cget -brand]" "brand"}
	    {tag.arg "[$self cget -tag]" "Build Tag"}
	    {type.arg "[$self cget -type]" "Build Type"}
	    {date.arg "[$self cget -date]" "Build Date"}
	    {customer.arg "$options(-customer)" "Customer"}
	}]

	set file [lindex $args end]

	if {$file eq ""} {
	    # Only add the .bin suffix if it doesn't already have a
	    # .bin or .trx suffix
	    if {[regexp {\.bin|\.trx} $(type)]} {
		set file $(type)
	    } else {
		set file "$(type).bin.trx"
	    }
	} elseif {[file isdirectory $file]} {
	    # Look for a driver, assuming this is part of a developer
	    # workspace.
	    if {![regexp {([^.]*)\.(.*)} $(type) - typedir typesuffix]} {
		set typedir $(type)
		set typesuffix "bin.trx"
	    }
	    # Handle case of non-release images
	    regsub {/rtecdc$} $typedir {} typedir
	    return [glob "$file{{{{{/dongle,}/rte,}/wl,}/builds,}/$typedir,}/rtecdc.$typesuffix"]

        } elseif {[file exists $file]} {
	    return $file
	}

	if {[lsearch {wlmips26 bcmdlmips26 wl} $file] >= 0} {
	    # These typically don't go in the customer's release.
	    set tail "release/bcm/apps/$file"
	} elseif {[regexp {/rtecdc\.bin(?:\.trx)$} $file]} {
	    # Firmware hasn't been copied - pull directly from firmware build
	    set (brand) hndrte-dongle-wl
	    set tail "src/dongle/rte/wl/builds/$file"
	} elseif {[regexp {\.(bin|txt|trx)$} $file]} {
	    if {[regexp {^src/} $file]} {
		# Allow the user to specify src/... to force a lookup
		# relative to the src tree.
		if {[regsub {/src$} [lindex $args end-1] "/$file" tail] &&
		    [file exists $tail]} {
		    # developer tree
		    return $tail
		}
		set tail $file
	    } elseif {[regexp {\.txt} $file] && ![regexp {/} $file]} {
	    # If the nvram file has no directory components, just
	    # look it up in the gallery.
	    return "$::UTF::projgallery/src/shared/nvram/$file"
	} else {
		set tail [file join release $(customer) firmware $file]
	    }
	} elseif {[regexp {\.(?:exe|opt)$} $file]} {
	    set (brand) hndrte-dongle-wl
	    regsub {(\.romlsim)?\.(trx|bin|bin\.trx)$} $(type) {} type
	    set tail [file join src dongle rte wl builds $type $file]
	    unset type
	} else {
	    set tail [file join release $(customer) host \
			  $(driver) $file]
	}

	if {[regexp {_REL_} $(tag)]} {
	    set (tag) "{PRESERVED/,ARCHIVED/,}$(tag)"
	} else {
	    set (tag) "{PRESERVED/,}$(tag)"
	}

	set pattern [file join \
			 $::UTF::projswbuild/build_linux \
			 $(tag) $(brand) $(date)* "$tail{,.gz}"]

	if {$(showpattern)} {
	    UTF::Message INFO $options(-name) $pattern
	}
	UTF::SortImages [list $pattern] \
	    {*}[expr {$(ls)?"-ls":""}] \
	    {*}[expr {$(all)?"-all":""}]

    }

    method load {args} {
	UTF::Message INFO $options(-name) "Install HSIC Firmware"

	UTF::GetKnownopts [subst {
	    {n "Copy files, but don't reload the driver"}
	    {nolinks.arg "$options(-nolinks)" "Don't create links to exe and src"}
	    {all "ignored"}
	    {ls "ignored"}
	}]
	snit::boolean validate $(nolinks)

	set image [$self findimages {*}$args]
	UTF::Message LOG $options(-name) "Dongle Image $image"
	if {[regexp -line {.* FWID.*\Z} [exec strings $image] id]} {
	    UTF::Message INFO $options(-name) $id
	    set exetype [lindex $id 0]
	}

	set nvram [$self findimages {*}$args $options(-nvram)]
	UTF::Message LOG $options(-name) "NVRAM $nvram"
	if {$options(-rwlrelay) ne ""} {
	    set wl [$self findimages {*}$args wl]
	    UTF::Message LOG $options(-name) "wl $wl"
	}

	set u $::tcl_platform(user)

	set nvram [UTF::nvram_add $nvram $options(-nvram_add)]
	if {[file extension $nvram] eq ".gz"} {
	    set tmp [exec mktemp /tmp/nvram.txt_XXXXX]
	    exec gunzip -c $nvram >$tmp
	    set nvram $tmp
	}
	# Serialise nvram
	set nvm [exec mktemp /tmp/nvram.nvm_XXXXX]
	localhost nvserial -a -s $options(-sn) -o $nvm $nvram
	if {[regexp {^/tmp/nvram\.txt_} $nvram]} {
	    UTF::Message LOG "" "file delete $nvram"
	    file delete $nvram
	}

	# Find mogrified sources in hndrte
	set m "<none>"
	if {[regsub {([^/]*)/(linux-.*)/(.*)\.\d+/release/.*} $image \
		 {\1/hndrte-dongle-wl/\3.*/src} m]} {
	} else {
	    # Perhaps it's a private build?
	    regsub {src/dongle/.*} $m {src} m
	}

	regsub {/} $options(-name) {.} name
	$self relay -n rm -f "/tmp/$name-hndrte-src.lnk"
	if {![catch {glob -type d $m} ret]} {
	    # Note lsort is required since dates may not quite match,
	    # but it may mean we use prebuild instead of regular
	    # builds.  That should be ok.
	    set m [lindex [lsort $ret] end]
	    UTF::Message LOG $options(-name) "Mogrified src: $m"
	    if {!$(nolinks)} {
		$self relay rexec -n ln -s '$m' "/tmp/$name-hndrte-src.lnk"
	    }
	    set hndrte_src $m
	} else {
	    UTF::Message LOG $options(-name) "Mogrified src: $ret"
	}
	$self relay -n rm -f "/tmp/$name-hndrte-exe.lnk"
	set exe [file join [file dirname $image] rtecdc.exe]
	if {![file exists $exe]} {
	    set exe rtecdc.exe
	}
	if {[info exists exetype]} {
	    # override type for symbol table
	    lappend args -type $exetype
	}
	if {[catch {$self findimages {*}$args $exe} ret]} {
	    UTF::Message LOG $options(-name) "Symbol file not found: $ret"
	} else {
	    UTF::Message LOG $options(-name) "Symbol file: $ret"
	    set hndrte_exe $ret
	    if {!$(nolinks)} {
		$self relay rexec -n \
		    ln -s '$hndrte_exe' "/tmp/$name-hndrte-exe.lnk"
	    }
	}

	# RTRFALCON has built-in DHD and persistent storage - make
	# sure it's not already full!  Also note that "full" may
	# recover after a reboot - maybe fragmentation?
	if {[catch {
	    $self rm -f /etc/jffs2/*
	    $self copyto $image  /etc/jffs2/rtecdc.bin.trx
	}]} {
	    $self nvram commit
	    $self reboot
	    $self rm -f /etc/jffs2/*
	    $self copyto $image  /etc/jffs2/rtecdc.bin.trx
	}
	$self copyto $nvm /etc/jffs2/nvram.nvm
	UTF::Message LOG "" "file delete $nvm"
	file delete $nvm

	if {$options(-rwlrelay) ne ""} {
	    if {$options(-rwlrelay) ne "localhost"} {
		$self rwlrelay copyto $wl /usr/bin/wl
	    }
	}

	if {$(n)} {
	    UTF::Message LOG $options(-name) "Copied files - not loading"
	    return
	}

	$self reload
    }

    method reload {args} {
	# Unload - will toggle GPIOs and/or power cycle as needed
	$self unload

	# Preinstall hook
	UTF::forall {STA dev} [$self cget -sta] \
	    cmd [$self cget -preinstall_hook] {
		if {[catch [string map [list %S $STA] $cmd] ret]} {
		    UTF::Message WARN $STA $ret
		}
	    }

	# Invalidate tuning cache
	set tuned ""

	if {[catch {$self bcmdl -n /etc/jffs2/nvram.nvm \
			/etc/jffs2/rtecdc.bin.trx} ret]} {
	    if {![regexp {No devices found|Failed to set} $ret] &&
		(![regexp {4335} $options(-type)] ||
		 ![regexp {Error: rdl state} $ret])} {
		error $ret $::errorInfo
	    }
	    $self warn "Trying power cycle recovery"
	    $self power cycle
#	    $self warn "Trying reboot recovery"
#	    catch {$self rexec reboot}
	    $base configure -initialized 0
	    UTF::Sleep 30
	    $self bcmdl -n /etc/jffs2/nvram.nvm \
		/etc/jffs2/rtecdc.bin.trx
	}
	# Allow dongle to suspend itself
	UTF::Sleep 1
	$self free
	$self insmod /lib/modules/dhd.ko
	$self insmod /lib/modules/hndjtag.ko
#	$self reg 0x18000618
	if {$options(-postinstall) ne ""} {
	    $self rexec $options(-postinstall)
	}
	$self host ifconfig eth1 up

	# WAR for TCP slow-start
	$self sysctl -w net.ipv4.tcp_congestion_control=reno

	# Hard coded mac address for 4330UNO3.  It's not needed right
	# now, but if we ever find a way to remove the above power
	# cycle we will need this to prevent the Mac Spoofing from
	# locking up the bridge.

	# $self host ifconfig eth1 hw ether 00:90:4C:C5:12:38

	if {[$self cget -lanpeer] ne ""} {
	    # Mac Spoof
	    $self host brctl addif br0 eth1
	    $self host ifconfig eth1 hw ether [$self host lan macaddr]
	}

	if {[set wlinitcmds [string trim [$self cget -wlinitcmds]]] ne ""} {
	    $self rexec $wlinitcmds
	}
	set hostver [lindex [$self nvram get os_version] 0]
	set ret [$self wl ver]
	regexp {version (.*)} $ret - ret
	return "$hostver:$ret"
    }

    method unload {} {
	UTF::Message INFO $options(-name) "Unload DHD Driver"
	catch {$self wl down}; # WAR for PR#94343
	$self -x rmmod wl hndjtag
	if {[catch {$self /sbin/reset_dongle_fw.sh} ret]} {
	    # fall back to power cycle
	    $self power cycle
	    $base configure -initialized 0
	    UTF::Sleep 30
	    return
	}

	$self free
	return
    }

    # IP address cache
    variable ipaddr -array {}

    method ipaddr {dev} {
	if {[$self cget -lanpeer] ne "" && $dev eq $options(-device)} {
	    $base lan ipaddr
	} else {
	    # Check cache
	    if {[info exists ipaddr($dev)]} {
		return $ipaddr($dev)
	    }
	    $base ipaddr $dev
	}
    }

    method ifconfig {dev args} {
	if {[$self cget -lanpeer] ne "" && $dev eq $options(-device)} {
	    $base lan ifconfig {*}$args
	} else {
	    set PF "/var/run/udhcpc0.pid"
	    if {[llength $args]} {
		# Setting something - kill off dhclient
		if {![catch {$self cat $PF 2>/dev/null} pid]} {
		    catch {$self rexec "kill $pid"}
		}
	    }
	    if {$args eq "dhcp"} {

		# invalidate cache in case of failure
		if {[info exists ipaddr($dev)]} {
		    unset ipaddr($dev)
		}

		if {[catch {$base udhcpc -i $dev -p $PF \
				-s /sbin/udhcpc-p2p.script} ret] ||
		    ![regexp -line {^ip=(\d+\.\d+\.\d+\.\d+)} $ret - \
			  ipaddr($dev)]} {
		    error "dhcp failed"
		}
	    } else {
		# Since parsing a full ifconfig commandline is hard, just
		# invalidate the cache and let ipaddr do the work next
		# time.
		if {$args ne "" && [info exists ipaddr($dev)]} {
		    unset ipaddr($dev)
		}
		$base ifconfig $dev {*}$args
	    }
	}
    }

    method route {args} {
	if {[$self cget -lanpeer] ne ""} {
	    $base lan route {*}$args
	} else {
	    $base route {*}$args
	}
    }

    method ping {args} {
	if {[$self cget -lanpeer] ne ""} {
	    $base lan ping {*}$args
	} else {
	    $base ping {*}$args
	}
    }

    method whatami {STA} {
	if {[catch {$STA chipname} c]} {
	    set c "<unknown>"
	}
	return "HSIC $c"
    }

    method wl {args} {
	if {$options(-rwlrelay) ne "" && $options(-rwlrelay) ne "localhost"} {
	    $options(-rwlrelay) host wl --socket [$self cget -lan_ip] {*}$args
	} else {
	    $base wl {*}$args
	}
    }

    UTF::doc {
	# [call [arg host] [method freekb]]

	# Returns free mem kbytes using "mu" RTE command on the dongle.
    }

    method freekb {} {
	if {[catch {$self wl maxmem} ret] || ![regexp {Free: (\d+)} $ret - free]} {
	    error "Free mem not found"
	    expr {$free / 1024.0}
	}
    }

    method rte_available {} {
	return 0
    }


    UTF::doc {
	# [call [arg host] [method tcptune] [arg window]] ]

	# Configure host tcp window size.  [arg window] indicates
	# desired TCP Window size in bytes.  A k suffix can be used to
	# indicate KB instead.

	# Returns 1 if userland tools need to specify the window size
	# themselves.
    }

    variable tuned ""
    method tcptune {window} {

	if {[info exists UTF::NoTCPTuning]} {
	    if {$window == 0} {
		return 0
	    }
	    if {$tuned eq ""} {
		set tuned [$self sysctl -n net.core.rmem_max]
	    }
	    set window [UTF::kexpand $window]
	    # Truncate any decimal component, but don't cast to int
	    # since it may not fit.
	    regsub {\..*} $window {} window
	    if {$tuned eq "" || $tuned < $window} {
		$self rexec \
		    sysctl -w \
		    net.core.wmem_max=$window \
		    net.core.rmem_max=$window
	    }
	    set tuned $window
	    return 1
	}

	if {$window == 0} {
	    set window 65535
	    set tcp_rmem "4096\t87380\t174760"
	    set tcp_wmem "4096\t16384\t131072"
	} else {
	    set window [UTF::kexpand $window]
	    if {$window < 1024} {
		error "Implausibly small window $window bytes"
	    }
	    set tcp_rmem "4096\t$window\t$window"
	    set tcp_wmem "4096\t$window\t$window"
	}

	if {$tuned ne "$window\n$window\n$tcp_rmem\n$tcp_wmem"} {
	    $self rexec \
		sysctl -w \
		net.core.wmem_max=$window \
		net.core.rmem_max=$window \
		net.ipv4.tcp_rmem="$tcp_rmem" \
		net.ipv4.tcp_wmem="$tcp_wmem"
	    set tuned "$window\n$window\n$tcp_rmem\n$tcp_wmem"
	}

	# No iperf daemon is used on HSIC, instead a new server will
	# be started for every connection.

	return 0
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
	$self tcptune $window
	return 1
    }

    # Message handling callback.  Used by "base" to process log
    # messages.  Returns true if no further processing is required.
    method msgcallback {msg} {
	switch -re -- $msg {
	    {Txctl wait timed out} -
	    {intr poll wait timed out} {
		# Only use the first of these.  Asserts get priority
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic $msg
		}
		UTF::Message FAIL $options(-name) $msg
		catch {$self showcons}
		catch {$self reload}
		return 1
	    }
	}
	return 0
    }

    variable cons0 0
    method showcons {} {
	if {$cons0 == 0} {
	    if {![info exists hndrte_exe]} {
		set hndrte_exe [$self relay rexec -noinit -s -q \
				    ls -l "/tmp/$name-hndrte-exe.lnk"]
		regsub {.*-> } $hndrte_exe {} hndrte_exe
	    }
	    set map [file join [file dirname $hndrte_exe] rtecdc.map]
	    if {[catch {exec awk "/cons0/{print \"0x\" \$1}" $map} ret]} {
		error "cons0 not found: $ret"
		set cons0 0
	    }
	    set cons0 $ret
	}
	$self rexec showcons $cons0
    }

    UTF::PassThroughMethod rwlrelay -rwlrelay
}

# Retrieve manpage from last object
UTF::doc [UTF::HSIC man]

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
