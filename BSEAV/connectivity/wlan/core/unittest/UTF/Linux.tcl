#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::Linux 2.0

package require snit
package require UTF::doc
package require UTF::Base

UTF::doc {
    # [manpage_begin UTF::Linux n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Linux support}]
    # [copyright {2005 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::Linux is an implementation of the UTF host object, specific
    # to Linux systems.

    # Once created, the Linux object's methods are not normally
    # invoked directly by test scripts, instead the Linux object is
    # designated the host for one or more platform independent STA
    # objects and it will be the STA objects which will be refered to
    # in the test scripts.

    # [list_begin definitions]

}

snit::type UTF::Linux {
    UTF::doc {
	# [call [cmd UTF::Linux] [arg host]
	#	[lb][option -name] [arg name][rb]
	#	[lb][option -lan_ip] [arg address][rb]
	# 	[lb][option -sta] [arg {{STA dev ...}}][rb]
	# 	[lb][option -ssh] [arg path][rb]
	#       [lb][option -image] [arg driver][rb]
	#       [lb][option -brand] [arg brand][rb]
	#       [lb][option -tag] [arg tag][rb]
	#       [lb][option -type] [arg type][rb]
	#       [lb][option -date] [arg date][rb]
	#       [lb][option -wlinitcmds] [arg cmds][rb]
        #       [arg ...]]

	# Create a new Linux host object.  The host will be
	# used to contact STAs residing on that host.
	# [list_begin options]

	# [opt_def [option -name] [arg name]]

	# Name of the host.

	# [opt_def [option -lan_ip] [arg address]]

	# IP address to be used to contact host.  This should be a
	# backbone address, not involved in the actual testing.

	# [opt_def [option -sta] [arg {{STA dev ...}}]]

	# List of STA devices (interfaces) to configure for this host.
	# The devices should be listed as [arg {name device}] pairs.
	# On Fedora15, the first wireless device is normally eth0.  On
	# earlier releases it was usually eth1.  These [cmd STA] names
	# are the objects that will be used in test, rather than the
	# [cmd host] object, since the [cmd host] object often hosts
	# multiple interfaces.

	# [example_begin]
	-sta {4360 eth0 4331 eth1}
	# [example_end]

	# [opt_def [option -ssh] [arg path]]

	# Specify an alternate command to use to contact [arg host],
	# such as [cmd rsh] or [cmd fsh].  The default is [cmd ssh].

	# [opt_def [option -image] [arg driver]]

	# Specify a default driver to install when [method load] is
	# invoked.  This can be an explicit path to a [file wl.ko]
	# file, or a suitable list of arguments to [method
	# findimages].

	# [opt_def [option -brand] [arg brand]]

	# Specify build brand.  Default is [file linux-internal-wl].

	# [opt_def [option -tag] [arg tag]]

	# Select a build tag.  Default is [file trunk].

	# [opt_def [option -type] [arg type]]

	# Select a build type.  Default is [file
	# debug-native-apdef-stadef*].

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2005.8.17.0].  Default is to
	# return the most recent build available.

	# [opt_def [option -wlinitcmds] [arg cmds]]

	# Specify cmds list to be executed after driver is loaded /
        # reloaded.  Default is null.

	# [opt_def [arg ...]]

	# Unrecognised options will be passed on to the host's [cmd
	# UTF::Base] object.

	# [list_end]
	# [list_end]

	# [para]
	# Linux objects have the following methods:
	# [para]
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    # base handles any other options and methods
    component base -inherit yes

    option -epi_ttcp "epi_ttcp"
    option -image
    option -sta
    option -name -configuremethod CopyOption
    option -app_image
    option -app_tag "trunk"
    option -app_date "%date%"
    option -app_brand "linux-combined-apps"
    option -app_customer "bcm"
    option -app_type "internal"
    option -tag "trunk"
    option -date "%date%"
    option -type "debug{-native,}-apdef-stadef"
    option -modopts ""
    option -nvram
    option -nvram_add
    option -msgcallback
    option -console -configuremethod _adddomain -default "/var/log/messages"
    option -wldot "wl"
    option -preinstall_hook
    option -postinstall_hook
    option -preunload
    option -wlinitcmds
    option -reloadoncrash -type snit::boolean -default false
    option -47xxtcl

    variable kernel ""
    variable tuned ""

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	install base using UTF::Base %AUTO% -user root -init [mymethod init] \
	    -brand "linux-internal-wl" -nointerrupts auto
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	foreach {sta dev} [UTF::Staexpand $options(-sta)] {
	    if {$dev eq ""} {
		error "$sta has no device name"
	    }
	    UTF::STA ::$sta -host $self -device $dev
	}
	if {[regexp {extnvm} $options(-type)]} {
	    lappend options(-nvram_add) "serialize"
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

    method _adddomain {name val} {
	set options($name) [UTF::AddDomain $val]
    }

    UTF::doc {

	# [call [arg host] [method findimages]
	#              [lb][option -all][rb]
	#              [lb][option -ls][rb]
	#              [lb][option -brand] [arg brand][rb]
	#              [lb][option -tag] [arg tag][rb]
	#              [lb][option -type] [arg type][rb]
	#              [lb][option -date] [arg date][rb]
	#		   [arg file]]

	# Returns a list of pathnames of available images or drivers from
	# the standard builds.  Globbing patterns are accepted in all
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

	# [opt_def [option -type] [arg type]]

	# Build types describe feature sets such as apdef-stadef, etc.
	# Default is [option -type] option of [arg host].

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2005.8.17.0].  Default is to
	# return the most recent build available.

	# [opt_def [arg file]]

	# Specify the file type being searched for.  Defaults to
	# "wl.ko".  If [arg file] is a directory, it is assumed to be
	# a user's private source tree and will be searched
	# accordingly.

	# [list_end]
    }

    method findimages {args} {

	UTF::GetKnownopts {
	    {all "return all matches"}
	    {ls "Report ls -l"}
	    {showpattern "Show search patterm"}
	}

	# Prepend -image to args as defaults
	set args [concat [$self cget -image] $args]

	# Findimages start
	UTF::Message LOG $options(-name) "findimages $args"

	# Defaults from object
	UTF::GetKnownopts [subst {
	    {brand.arg "[$self cget -brand]" "brand"}
	    {tag.arg "$options(-tag)" "Build Tag"}
	    {type.arg "$options(-type)" "Build Type"}
	    {date.arg "$options(-date)" "Build Date"}
	    {app_image.arg "[$self cget -app_image]" "App Image"}
	    {app_brand.arg "$options(-app_brand)" "Build Tag"}
	    {app_tag.arg "[$self cget -app_tag]" "Build Tag"}
	    {app_date.arg "[$self cget -app_date]" "Build Date"}
	    {app_type.arg "$options(-app_type)" "App Type"}
	    {app_customer.arg "$options(-app_customer)" "App Customer"}
	    {gub.arg "[$self cget -gub]" "Alternate GUB build path"}
	}]

	# Might get modified for some bits
	set tag $(tag)
	set brand $(brand)
	set date $(date)

	set file [lindex $args end]

	if {$file eq ""} {
	    set file "$options(-wldot).ko"
	} elseif {[file isdirectory $file]} {
	    # Look for a driver, assuming this is part of a developer
	    # workspace.
	    return [UTF::BuildFile::glob "$file{{{{{/build,}/src,}/wl,}/linux,/release}/obj-$(type)-[$self kernel],}/$options(-wldot).ko{,.gz}"]
	} elseif {[UTF::BuildFile::exists $file]} {
	    return $file
	}

	if {$file eq "wl"} {
	    if {$(app_image) != ""} {
		if {[regexp {x86_64} [$self kernel]]} {
		    return [glob "$(app_image){{{/src,}/wl,}/exe,}/wlx86_64"]
		} else {
		    return [glob "$(app_image){{{/src,}/wl,}/exe,}/wl"]
		}
	    }
	    set brand $(app_brand)
	    set tag $(app_tag)
	    set date $(app_date)
	    set customer $(app_customer)
	    if {$brand eq "linux-combined-apps" && [regexp {fc15} [$self kernel]]} {
		# fc15 isn't in linux-combined-apps
		set brand "linux-internal-wl"
	    }
	    if {$brand ne "linux-combined-apps"} {
		set (app_type) "release"
		set customer "exe"
	    }
	    if {[regexp {x86_64} [$self kernel]]} {
		set tail [file join $(app_type) $customer x86_64 $file]
	    } else {
		set tail [file join $(app_type) $customer $file]
	    }
	} elseif {$file eq "wpa_supplicant" || $file eq "wpa_cli"} {
	    set tag "HOSTAP_REL_0_8_0_*"
	    set brand "linux-x86-wpa-supp"
	    if {[regexp {x86_64} [$self kernel]]} {
		set tail [file join release all 64 bin $file]
	    } else {
		set tail [file join release all 32 bin $file]
	    }
	} elseif {[regexp {.*\.txt} $file]} {
	    # nvram
	    if {![regexp {/} $file]} {
		# The nvram gallery has gone - rewrite path
		set file "src/shared/nvram/$file"
	    }
	    if {[regexp {^src/|^components} $file]} {
		# use src to indicate local src tree
		regsub {^src/shared/nvram|^components/nvram} $file \
		    {{src/shared,components}/nvram} file
		# Check for developer build
		if {![regsub {/src$} [lindex $args end-1] "/$file" tail]} {
		    set tail $file
		}
	    } else {
		# Otherwise just look in the gallery

		# Note "?" is to force an existence check in case
		# we're using shell glob instead of tcl glob.
		set tail "$::UTF::projgallery/{src/shared,components}/nvra?/$file"
	    }
	} else {
	    # Add obj- if necessary
	    if {![regexp {^obj-} $(type)]} {
		set (type) "obj-$(type)"
	    }

	    # Add kernel if necessary
	    if {![regexp {[23]\.\d+\.\d+} $(type)]} {
		set (type) "$(type)-[$self kernel]"
	    }
	    if {[regexp {media} $brand]} {
		set tail [file join release bcm host wl_driver $(type) $file]
	    } else {
		set tail [file join release $(type) $file]
	    }
	}

	if {[regexp {_REL_} $(tag)]} {
	    set tag "{PRESERVED/,ARCHIVED/,}$tag"
	} else {
	    set tag "{PRESERVED/,}$tag"
	}

	set pattern [file join \
			 $::UTF::projswbuild $(gub) build_linux \
			 $tag $brand $date* "$tail{,.gz}"]

	if {$(showpattern)} {
	    UTF::Message INFO $options(-name) $pattern
	}
	UTF::SortImages [list $pattern] \
	    {*}[expr {$(ls)?"-ls":""}] \
	    {*}[expr {$(all)?"-all":""}]
    }

    UTF::doc {
	# [call [arg host] [method reload]]

	# Reload the driver using files already copied to the host.
    }

    variable reloadlock 0
    method reload {} {
	try {
	    # Avoid looping if the driver asserts immediately
	    if {[incr reloadlock] != 1} {
		return
	    }
	    set f /root/$options(-wldot).ko

	    $self clean_udev_rules

	    try {
		$self unload
	    } finally {

		# Invalidate tuning cache
		set tuned ""

		# Fix up module dependencies
		foreach m [split [$self modinfo -F depends $f] ","] {
		    $self modprobe $m
		}

		set modargs $f
		if {$options(-nvram) ne ""} {
		    if {[regexp {extnvm} $options(-type)]} {
			# extnvm driver assumes nvram is in CWD
			#$self ln -sf /root/nvram.txt .
		    } else {
			# BMAC provides an option for nvram
			lappend modargs "nvram_path=/root/nvram.txt"
		    }
		}
		if {$options(-modopts) ne ""} {
		    lappend modargs $options(-modopts)
		}

		UTF::forall {STA dev} $options(-sta) \
		    cmd $options(-preinstall_hook) {
			if {[catch [string map [list %S $STA] $cmd] ret]} {
			    UTF::Message WARN $STA $ret
			}
		    }

		$self rexec insmod {*}$modargs

		UTF::forall {STA dev} $options(-sta) \
		    cmd $options(-postinstall_hook) {
			if {[catch [string map [list %S $STA] $cmd] ret]} {
			    UTF::Message WARN $STA $ret
			}
		    }

		if {$options(-wlinitcmds) ne ""} {
		    # Change hard fail to soft fail with error -
		    # another attempt to debug intermittent NIC load
		    # timeouts.
		    $self rexec -e 30 -t 120 -d wlinitcmds \
			[string trim $options(-wlinitcmds)]
		}
	    }
	    set ret [$self wl ver]
	    regexp {version (.*)} $ret - ret
	    return $ret
	} finally {
	    incr reloadlock -1
	}
    }

    UTF::doc {
	# [call [arg host] [method load] [lb][arg file][rb]]
	# [call [arg host] [method load] [lb][arg {args ...}][rb]]

	# Load a wl driver into the running kernel.  In the first form
	# [arg file] should be the pathname of a [file wl.ko] compiled
	# for the current kernel.  In the second form, the argument
	# list will be passed on to [method findimages] to find a
	# driver.  The [arg host]s [option -modopts] option can be
	# used to add module options if needed.  Filenames are
	# relative to the control host and files will be copied to
	# [arg host] as needed.  If a version of [syscmd wl] is found
	# with the new driver, the [syscmd wl] command on [arg host]
	# option will be updated accordingly.
    }

    variable loaded ""

    method load_needed {args} {
	expr {[catch {$self findimages {*}$args} file] ||
	      $file ne $loaded}
    }

    method load {args} {
	UTF::GetKnownopts {
	    {n "Just copy the files, don't actually load the driver"}
	    {all "ignored"}
	    {ls "ignored"}
	}

	UTF::Message INFO $options(-name) "Load Linux Driver"
	set file [$self findimages {*}$args]
	UTF::Message LOG $options(-name) "Found $file"
	if {$options(-nvram) ne ""} {
	    set nvram [$self findimages {*}$args $options(-nvram)]
	    UTF::Message LOG $options(-name) "NVRAM $nvram"
	} elseif {$options(-nvram_add) ne ""} {
	    # Allow nvram additions, even if there's no base nvram
	    set nvram "/dev/null"
	}
	set wl [$self findimages {*}$args "wl"]
	UTF::Message LOG $options(-name) "wl $wl"

	if {$file eq $loaded} {
	    UTF::Message LOG $options(-name) "Skipping reload of same image"
	} else {
	    set f /root/$options(-wldot).ko
	    UTF::BuildFile::copyto $self $file $f
	    # strip debug info else driver may be too large to load
	    $self -x strip -d $f
	    if {[info exists nvram]} {
		UTF::BuildFile::nvram_add_copyto $self $nvram /root/nvram.txt \
		    $options(-nvram_add)
	    }

	    UTF::BuildFile::copyto $self $wl /usr/bin/wl
	    set loaded $file
	}

	if {$(n)} {
	    UTF::Message LOG $options(-name) "Copied files - not loading"
	    return
	}

	foreach {- dev} $options(-sta) {
	    # Make sure HOTPLUG, etc are correct before loading the
	    # device the first time.
	    $self check_network_scripts $dev
	}
	# Sanity check for stale iperf processes.
	if {![catch {$self pgrep iperf} ret] && [llength $ret] > 5} {
	    $self rexec {ps -fp $(pgrep iperf)}
	    $self warn "stale iperf processes"
	}
	$self sync
	$self reload

    }

    method load_wl {args} {
	UTF::Message INFO $options(-name) "Load wl executable"
	set wl [$self findimages {*}$args "wl"]
	UTF::Message LOG $options(-name) "wl $wl"
	$self copyto $wl /usr/bin/wl
    }

    UTF::doc {
	# [call [arg host] [method clean_udev_rules]]

	# Remove old udev rules, so that we don't keep creating new
	# eth<n> entries for new devices.
    }

    method clean_udev_rules {} {
	set rules "/etc/udev/rules.d/70-persistent-net.rules"
	if {![catch {$self rexec -q -n test -e $rules}]} {
	    $self rexec sed -i '/^$/d\;/(wl)\\|0x0a5c:.*(usb)/,/^$/d' $rules
	}
	# Remove stale write check in case hotplug was
	# interrupted.
	$self rm -f /etc/udev/rules.d/.is-writeable
    }

    UTF::doc {
	# [call [arg host] [method shutdown_reboot] [arg t1] [arg t2]
	# [arg cmd1] [arg cmd2] [lb][opt -force][rb]]

	# All calling parameters are optional and have OS specific defaults.
        # For detailed documentation, see UTF/Base.tcl method shutdown_reboot.
    }

    method shutdown_reboot {{t1 ""} {t2 ""} {cmd1 ""} {cmd2 ""} args} {

        # In order for calling routines to just specify the -force option,
        # we need to allow null values on the command line for the first
        # four parameters. So insert the appropriate default values here.
        set t1 [string trim $t1]
        if {$t1 == ""} {
            set t1 40
        }
        set t2 [string trim $t2]
        if {$t2 == ""} {
            set t2 40 ;# Dell T3400 needs more time to accept power failure is over
        }
        set cmd1 [string trim $cmd1]
        if {$cmd1 == ""} {
            set cmd1 "poweroff -h"
        }
        set cmd2 [string trim $cmd2]
        if {$cmd2 == ""} {
            set cmd2 "reboot"
        }

        # Call the common method
        $base shutdown_reboot $t1 $t2 $cmd1 $cmd2 $args
    }

    method wait_for_boot {{t 30}} {
	UTF::Sleep $t
	$self configure -initialized 0
	for {set i 0} {[catch {$self -n :}] && $i < 20} {incr i} {}
    }

   UTF::doc {
	# [call [arg host] [method unload]]

	# Unload the current wl driver.
    }

    method unload {} {
	UTF::Message INFO $options(-name) "Unload Linux Driver"
	set loaded ""
	try {
	    # Avoid reloading if we crash during unload
	    incr reloadlock

	    $self supplicant stop

	    # Preunload
	    if {$options(-preunload) ne ""} {
		$self rexec $options(-preunload)
	    }

	    set modmatch "wl|dhd"
	    if {$options(-wldot) ne "wl"} {
		append modmatch "|$options(-wldot)"
	    }
	    if {[catch {$self rexec "lsmod|awk '/^($modmatch) /{print \$1}'"} modlist]} {
		set modlist "$modlist: try power cycle"
		$self worry $modlist
		$self power cycle
		$self wait_for_boot 10
		return $modlist
	    }
	    if {$modlist ne "" && [catch {$self rexec rmmod $modlist} ret]} {
		# If rmmod gets killed it's probably crashing, so give it
		# chance to recover.
		if {[regexp {Timeout|child killed|resource busy|Module .* is in use} $ret]} {
		    if {[regexp {Device or resource busy|Module .* is in use} $ret]} {
			# Resource busy doesn't mean it's crashing,
			# but will still need a reboot.
			$self -x reboot
		    }
		    UTF::Sleep 10
		    $self configure -initialized 0
		    for {set i 0} {[catch {$self :}] && $i < 10} {incr i} {}
		}
		error $ret
	    }
	    UTF::Sleep 1
	} finally {
	    incr reloadlock -1
	}
    }

    UTF::doc {
	# [call [arg host] [method ifconfig] [arg dev] [option dhcp]]

	# Enable DHCP on device [arg dev].

	# [call [arg host] [method ifconfig] [arg dev] [option local]]

	# Enable and assign an ipv4 link local address to the device [arg dev].

	# [call [arg host] [method ifconfig] [arg {args ...}]]

	# Run ifconfig on the host, disabling DHCP if necessary.
    }

    # The location of DHCP lease files varies between releases
    variable leases ""
    variable dhcpconfig -array {}

    # IP address cache
    variable ipaddr -array {}
    variable dhcpver ""

    method ifconfig {dev args} {

	# Check the interface settings.  Devices "loaded" have already
	# been checked, but still need to check reference endpoints
	# and devices defined at runtime.
	$self check_network_scripts $dev

	set PF "/var/run/dhclient-${dev}.pid"
	if {[llength $args]} {
	    # Setting something - kill off dhclient
	    $self rexec -x "test -f $PF && kill `cat $PF` "
	}
	if {$args eq "local"} {
	    $self rexec "avahi-autoipd -c $dev || avahi-autoipd -D --wait $dev"
	    $self rexec -x route delete -net default dev $dev
	    if {[regexp -line {inet (\S+)/16 .*:avahi} \
		     [$self ip addr list $dev] - ip]} {
		# Return ipaddr and Update cache
		return [set ipaddr($dev) $ip]
	    } else {
		error "No local ip found"
	    }
	} elseif {$args eq "dhcp"} {
	    # invalidate cache in case of failure
	    if {[info exists ipaddr($dev)]} {
		unset ipaddr($dev)
	    }

	    if {$leases eq ""} {
		if {![catch {$self test -d /var/lib/dhclient}]} {
		    set leases "/var/lib/dhclient"
		} else {
		    set leases "/var/lib/dhcp"
		}
	    }
	    set LF [file join $leases "dhclient-${dev}.leases"]
	    set CF "/etc/dhclient-${dev}.conf"
	    set dhclient "/sbin/dhclient -v"

	    if {[$self kernelcmp 4.0.0] > 0} {
		# Leave lease file in place, else Fc22 will consume
		# all the leases on your DHCP server.
	    } else {
		# Remove lease file.  We're better off not leaving leases
		# around since some dhclients take a DHCPNAK as an excuse
		# to trigger a DISASSOC.
		$self rm -f $LF
	    }

	    # Flush ip tables, since dhclient in Fc19 doesn't do this
	    # automatically
	    $self -x ip -4 addr flush dev $dev
	    $self -x ip -4 route flush dev $dev
	    $self -x ip -4 neigh flush dev $dev

	    # Allow for one retry on dhclient collision
	    for {set i 0} {$i < 2} {incr i} {
		if {![catch {
		    $self rexec {*}$dhclient -1 -lf $LF -pf $PF -cf $CF $dev
		} ret]} {
		    break
		}
		if {$i == 0 &&
		    [regexp {dhclient\((\d+)\) is already running} $ret - p]} {
		    # someone else started up a competing dhclient.
		    # Kill it and try again.
		    catch {$self ps -l -C dhclient}
		    catch {$self rexec kill $p}
		    UTF::Sleep 1
		    catch {$self ps -l -C dhclient}
		    catch {$self rexec kill -9 $p}
		    UTF::Sleep 1
		    catch {$self ps -l -C dhclient}
		} else {
		    # Pass up only the last line of the error, which
		    # usually contains the actual failure message
		    set e [lindex [split $ret "\n"] end]
		    error $e $e
		}
	    }
	    if {![regexp {bound to (\S+)} $ret - ip]} {
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
	# [call [arg host] [method {route add}] [arg net] [arg mask]
	#	  [arg gw]]

	# Add an IP route to network [arg net] netmask [arg mask]
	# through gateway [arg gw]
    }
    method {route add} {net mask gw} {
	$self rexec route add -net $net netmask $mask gw $gw
    }

    UTF::doc {
	# [call [arg host] [method {route replace}] [arg net] [arg mask]
	#	  [arg gw]]

	# Add or Modify an IP route to network [arg net] netmask [arg
	# mask] through gateway [arg gw]
    }
    method {route replace} {net mask gw} {
	package require ip
	set len [ip::maskToLength $mask]
	$self rexec ip route replace $net/$len via $gw
    }

    UTF::doc {
	# [call [arg host] [method {route delete}] [arg net] [arg mask]
	#	  [lb][arg gw][rb]]

	# Delete an IP route to network [arg net] netmask [arg mask]
	# through optional gateway [arg gw]
    }
    method {route delete} {net mask {gw ""}} {
	package require ip
	set len [ip::maskToLength $mask]
	if {$gw eq ""} {
	    $self rexec ip route delete $net/$len
	} else {
	    $self rexec ip route delete $net/$len via $gw
	}
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
	# [call [arg host] [method macaddr] [arg dev]]

	# Returns the MAC address for [arg dev]
    }
    method macaddr {dev} {
	if {[regexp {(?:ether|HWaddr)\s+(\S+)} \
		 [$self rexec ifconfig $dev] - mac]} {
	    return $mac
	} else {
	    error "No MAC address found"
	}
    }

    UTF::doc {
	# [call [arg host] [method ipaddr] [arg dev]]

	# Return IP address of device [arg dev] or Error if device is
	# down
    }

    method ipaddr {dev} {
	# Check cache
	if {[info exists ipaddr($dev)]} {
	    return $ipaddr($dev)
	}
	set ret [$self ifconfig $dev]
	if {[regexp {inet (?:addr:)?([0-9.]+)} $ret - addr] ||
	    [regexp {: ip ([0-9.]+) mask} $ret - addr]} {
	    # Return ipaddr and Update cache
	    return [set ipaddr($dev) $addr]
	} elseif {[regexp -line {inet (\S+)/16 .*:avahi} \
		       [$self ip addr list $dev] - addr]} {
	    # link local
	    return [set ipaddr($dev) $addr]
	} else {
	    error "$options(-name) No IP address available"
	}
    }

    variable processingRecovery false
    typevariable msgfile
    # Internal callback for fileevent below
    method getdata {request} {
	set fd $msgfile($request)
	if {![eof $fd]} {
	    set msg [gets $fd]
	    if {[regexp {:/var/log/messages} $request]} {
		# Only show messages we might be interested in
		# Lose dates, etc, since we're adding our own.
		if {![regexp {.*(?:kernel|wlum): (?:\[\s*\d+\.\d+\] )?(.*)} \
			  $msg - msg]} {
		    return
		}
	    } elseif {[regexp {:/dev/kmsg} $request]} {
		#UTF::Message DBG $options(-name) $msg
		# <priority/facility>,<seq no>,<timestamp>,<flag>(,...);<message>
		if {![regexp {^\d+,\d+,\d+,-;(.*)} $msg - msg]} {
		     return
		}
	    } elseif {[regexp {:dmesg} $request]} {
		regsub {^<\d+>\[\s*\d+\.\d+\] } $msg {} msg
	    } else {
		# May still need to trim off kernel prefix
		regsub {.* kernel: } $msg {} msg
		regsub {\[\s*\d+\.\d+\] } $msg {} msg
	    }
	    if {$options(-msgcallback) ne ""} {
		# Specialist host failures.  Returns true if no
		# further processing is required.
		if {[{*}$options(-msgcallback) $msg]} {
		    return
		}
	    }
	    if {!$processingRecovery && $options(-reloadoncrash) &&
		      ([regexp {Rebooting in \d+ seconds\.\.} $msg] ||
		       [regexp {BIOS-provided physical RAM map:} $msg] ||
		       ([regexp -- {---\[ cut here \]---} $msg] &&
			[catch {$self -n -t 5 :}]))} {
		UTF::Message WARN $options(-name) $msg
		# ignore these messages while waiting
		set processingRecovery true
		UTF::Sleep 90
		set processingRecovery false
		# Clear IP addr cache
		array unset ipaddr
		$self reload
	    } elseif {[regexp {sdstd_card_} $msg]} {
		# Host failures to report only on basic NIC devices
		# Only use the first of these.  Asserts get priority
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic $msg
		}
		UTF::Message FAIL $options(-name) $msg
	    } elseif {![$base common_getdata $msg]} {
		UTF::Message LOG $options(-name) $msg
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

    method open_messages { {files ""} } {
	if {$files eq "" && [set files $options(-console)] eq ""} {
	    return
	}
	set id [$self cget -lan_ip]
	foreach file $files {
	    if {[info exists msgfile($id:$file)]} {
		UTF::Message LOG $options(-name) "Open $file (already open)"
		continue
	    }
	    if {[string match "/dev/kmsg" $file]} {
		# tail won't work on devices
		set msgfile($id:$file) [$self rpopen -n cat $file]
		# /dev/kmsg reports the entire buffer, so we need to
		# discard previous logs.  This may take a few seconds.
		# Ideally we would seek to end on open, but we don't
		# have tools to do that.  /proc/kmsg doesn't have the
		# problem, but /proc/kmsg not safe for concurrent
		# access.
		fconfigure $msgfile($id:$file) -blocking 0
		set discard "start"
		set discarded 0
		set t [lindex [time {
		    UTF::Sleep 0.5
		    for {set i 0} {$discard ne "" && $i < 20} {incr i} {
			set discard [read $msgfile($id:$file)]
			incr discarded [string length $discard]
			#UTF::Message DBG $options(-name) \
			#    "Discarded [llength [split $discard \n]] lines"
			UTF::Sleep 0.25
		    }
		}] 0]
		UTF::Message DBG $options(-name) \
		    [format "Discarded $discarded bytes in %.2g seconds" \
			 [expr {$t / 1000000.0}]]
	    } elseif {[string match "dmesg" $file]} {
		$self rexec dmesg -c
		set msgfile($id:$file) [$self rpopen -n \
					    "while :; do dmesg -c; sleep 1; done"]
	    } elseif {[string match "/*" $file]} {
		set msgfile($id:$file) [$self rpopen -noinit tail -0f $file]
		fconfigure $msgfile($id:$file) -blocking 0
	    } elseif {[catch {$self serialrelay socket $file} ret]} {
		$self worry "Open $file $ret"
		continue
	    } else {
		UTF::Message LOG $options(-name) "Open $file $ret"
		set msgfile($id:$file) $ret
		fconfigure $msgfile($id:$file) -blocking 0 -buffering line
		puts $msgfile($id:$file) ""; # Trigger NPC telnet reconnect
	    }
	    fileevent $msgfile($id:$file) readable [mymethod getdata $id:$file]
	}
	return
    }

    UTF::doc {
	# [call [arg host] [method close_messages] [lb][arg file][rb]]

	# Close a system message log previously opened by [method
	# open_messages].  An error will be reported if [arg file]
	# does not match that of a previous invocation of [method
	# open_messages].
    }
    method close_messages { {files ""} } {
	if {$files eq "" && [set files $options(-console)] eq ""} {
	    return
	}
	set id [$self cget -lan_ip]
	foreach file $files {
	    if {[info exists msgfile($id:$file)] &&
		[file channels $msgfile($id:$file)] ne ""} {
		UTF::Message LOG $options(-name) "Close $file $msgfile($id:$file)"
		if {[set pid [pid $msgfile($id:$file)]] ne ""} {
		    # Processes to wait for
		    catch {exec kill -HUP {*}$pid} ret
		    UTF::Message LOG $options(-name) "$pid $ret"
		}
		# Leave non-blocking for safer close.  Any error messages
		# should have been collected earlier.
		close $msgfile($id:$file)
		unset msgfile($id:$file)
	    } else {
		UTF::Message LOG $options(-name) "Close $file (not open)"
	    }
	}
	return
    }

    method init {} {
	$self open_messages
    }

    method deinit {} {
	$self close_messages
	$self configure -initialized 0
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
	# [call [arg host] [method tcptune] [arg window]] ]

	# Configure host tcp window size.  [arg window] indicates
	# desired TCP Window size in bytes.  A k suffix can be used to
	# indicate KB instead.

	# Returns 1 if userland tools need to specify the window size
	# themselves.
    }

    method tcptune {window} {

	if {[info exists UTF::NoTCPTuning] && [info exists UTF::TcpReadStats]} {
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
		set tuned $window
	    }
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
	    # Truncate any decimal component, but don't cast to int
	    # since it may not fit.
	    regsub {\..*} $window {} window
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
	    if {[$self cget -iperfdaemon] &&
		![info exists UTF::TcpReadStats]} {
		catch {
		    $self service iperf restart
		}
	    }
	    set tuned "$window\n$window\n$tcp_rmem\n$tcp_wmem"
	} elseif {[$self cget -iperfdaemon] &&
		  ![info exists UTF::TcpReadStats] &&
		  ([catch {$self -s service iperf status} ret] ||
		   [regexp {Active: active \(exited\)} $ret])} {
	    UTF::Message LOG $options(-name) $ret
	    $self service iperf restart
	}
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

	if {[info exists UTF::NoTCPTuning] && [info exists UTF::TcpReadStats]} {
	    return [$self tcptune $window]
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
	    # Truncate any decimal component, but don't cast to int
	    # since it may not fit.
	    regsub {\..*} $window {} window
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
	return 1
    }


    UTF::doc {
	# [call [arg host] [method setup_dut]]

	# Combines the entire setup sequence I use on all my Linux
	# test devices into a single command.
    }


    method setup_dut {} {
	$self authorize
	$self setup
	$self add_serialconsole
	$self disable_gui
	$self blacklist
	$self -x reboot
    }

    UTF::doc {
	# [call [arg host] [method setup]]

	# Installs or updates any local tools needed on a UTF client.
    }

    method load_epi {} {
	if {[file exists "$::UTF::projtools/linux-$kernel/bin/epi_ping"]} {
	    $self copyto "$::UTF::projtools/linux-$kernel/bin/epi_ping" \
		"/usr/local/bin/epi_ping"
	} else {
	    $self copyto "$::UTF::projtools/linux/bin/epi_ping" \
		"/usr/local/bin/epi_ping"
	}
	if {[file exists "$::UTF::projtools/linux-$kernel/bin/epi_ttcp"]} {
	    $self copyto "$::UTF::projtools/linux-$kernel/bin/epi_ttcp" \
		"/usr/local/bin/epi_ttcp"
	} else {
	    $self copyto "$::UTF::projtools/linux/bin/epi_ttcp" \
		"/usr/local/bin/epi_ttcp"
	}
    }

    method setup {} {
	set archive "$::UTF::projarchives/unix/UTF"
	$self configure -ssh ssh
	$self authorize
	$self connect

	$self kernel

	if {![catch {$self -n test -r /etc/redhat-release}]} {
	    # RedHat/Fedora
	    $self setup_redhat
	} elseif {![catch {$self -n test -r /etc/debian_version}]} {
	    # Debian/UBUNTU
	    $self setup_debian
	} else {
	    error "Unsupported OS"
	}

	# Announce Acceptable Use Policy
	if {[catch {$self grep accept-use /etc/motd}]} {
	    $self copyto $UTF::unittest/etc/motd /etc/motd
	}

	# Local copy of consolelogger so we're not dependent on NFS at
	# boot time.  Add a 1sec delay so upstart doesn't kill us off
	# at the first sign of trouble
	$self copyto $UTF::unittest/consolelogger /usr/local/bin/consolelogger
	$self rexec -x {sed -i 's/^exec \/.*\/consolelogger/exec \/usr\/local\/bin\/consolelogger/;/^pre-start/d;/^exec/apre-start exec sleep 1' /etc/event.d/consolelogger*}

	if {[file exists "$::UTF::projtools/linux-$kernel/bin/epi_ping"]} {
	    $self copyto "$::UTF::projtools/linux-$kernel/bin/epi_ping" \
		"/usr/local/bin/epi_ping"
	} else {
	    $self copyto "$::UTF::projtools/linux/bin/epi_ping" \
		"/usr/local/bin/epi_ping"
	}
	if {[file exists "$::UTF::projtools/linux-$kernel/bin/epi_ttcp"]} {
	    $self copyto "$::UTF::projtools/linux-$kernel/bin/epi_ttcp" \
		"/usr/local/bin/epi_ttcp"
	} else {
	    $self copyto "$::UTF::projtools/linux/bin/epi_ttcp" \
		"/usr/local/bin/epi_ttcp"
	}
	# Install ush
	$self copyto "$archive/usr/local/bin/ush" "/usr/local/bin/ush"

	# Install nitro for Nintendo testing
	$self copyto "$archive/usr/local/bin/nitro" "/usr/local/bin/nitro"

        # Install iperf_timestamps.tcl for WLAN / BT coex testing.
	$self copyto "$archive/usr/local/bin/iperf_timestamps.tcl"\
            "/usr/local/bin/iperf_timestamps.tcl"
	# Install igmp querier needed for multicast testing
	$self copyto "$::UTF::projtools/linux/bin/igmp_querier" \
		"/usr/local/bin/igmp_querier"

	if {[$self isskylake]} {
	    $self copyto $UTF::unittest/etc/skylake_aspm.sh skylake_aspm.sh
	}

    }

    method grub2mkconfig {} {
	if {[$self kernelcmp 3.0.0] < 0} {
	    return
	}
	# Grub
	$self rexec "grub2-mkconfig -o /boot/grub2/grub.cfg"
	# EFI
	$self rexec "if test -f /boot/efi/EFI/fedora/grub.cfg; then grub2-mkconfig -o /boot/efi/EFI/fedora/grub.cfg; fi"
    }

    method setup_redhat {} {
	set archive "$::UTF::projarchives/unix/UTF"

	if {[$self uname -i] eq "x86_64"} {
	    set fshrpm "fsh-1.2-5_bcm5.x86_64.rpm"
	} else {
	    set fshrpm "fsh-1.2-5_bcm5.i386.rpm"
	}
	if {[catch {$self rpmq fsh} ret] || \
		[localhost rpmq -p $archive/$fshrpm] \
		ne $ret} {
	    $self copyto "$archive/$fshrpm" "/tmp/$fshrpm"
	    $self rexec rpm -Uvh "/tmp/$fshrpm"
	}
	# Remove DNS dependency on ssh logins
	$self rexec -n {sed -i "s/#UseDNS yes/UseDNS no/" /etc/ssh/sshd_config&&service sshd restart}
	UTF::Sleep 1
	$self configure -ssh ""
	$self connect -force

	$self setup_iperf

	if {$kernel eq "2.6.21-1.3194.fc7"} {
	    # FC7 netconsole hangs on rmmod
	    $self sed -i "'/rmmod netconsole/s/^\#*/:/'" /etc/init.d/netconsole
	}

	# Increase default loglevel
	if {[$self kernelcmp 3.0.0] > 0} {
	    $self rexec "sed -i '/GRUB_CMDLINE_LINUX=/s/ quiet//' /etc/default/grub"
	    $self grub2mkconfig
	} elseif {[$self kernelcmp 2.6.43.5] <= 0} {
	    $self sed -i "'/kernel/s/ quiet//'" /boot/grub/grub.conf
	}
	if {[$self kernelcmp 2.6.38.6] < 0} {
	    $self sed -i "'s/LOGLEVEL=3/LOGLEVEL=7/'" /etc/sysconfig/init
	}
	$self dmesg -n 7

	# Disable modeset - tends to cause crash loops on FC11 and above
	if {[$self kernelcmp 4.0.0] > 0} {
	    # Leave modeset alone for now - review later
	} elseif {[$self kernelcmp 3.0.0] > 0} {
	    if {[catch {$self rexec grep nomodeset /etc/default/grub}]} {
		$self rexec "sed -i '/GRUB_CMDLINE_LINUX=/s/\"$/ nomodeset\"/' /etc/default/grub"
		$self grub2mkconfig
	    }
	} elseif {[$self kernelcmp 2.6.29] > 0} {
	    if {[catch {$self rexec grep nomodeset /boot/grub/grub.conf}]} {
		$self sed -i "'/^\tkernel/s/$/ nomodeset/'" /boot/grub/grub.conf
	    }
	}

	$self setup_sysctl

	if {[catch {$self grep GATEWAYDEV /etc/sysconfig/network}]} {
	    set GWDEV [$self rexec {route|awk '/default/{print $NF}'}]
	    $self rexec "echo GATEWAYDEV=$GWDEV >> /etc/sysconfig/network"
	}

	if {[regexp {fc9} $kernel]} {
	    # Ineffective before fc9, obsoleted by /dev/serial in fc11
	    $self setup_usb_rules
	}

	$self setup_svn

	if {[regexp {fc15} $kernel]} {
	    # fix up autofs dependencies
	    $self sed -i 's/\$ypbind/ypbind/' /etc/init.d/autofs
	}

	if {[$self kernelcmp 2.6.38] > 0} {
	    $self -x systemctl disable NetworkManager.service
	}

	# Disable all other wireless drivers
	if {[$self kernelcmp 3.0.0] > 0} {
	    $self -x gzip -r \
		/lib/modules/*/kernel/drivers/{bcma,ssb,mmc,net/wireless/{b43,b43legacy,brcm80211}}
	    $self -x gzip /usr/lib/firmware/brcm/brcmfmac43*
	} elseif {[$self kernelcmp 2.6.43.5] > 0} {
	    $self -x gzip -r /lib/modules/$kernel/kernel/drivers/net/wireless
	}

	# disable firewall
	if {[$self kernelcmp 2.6.43.5] > 0} {
	    $self -x systemctl disable firewalld.service
	}

	$self setup_services

	if {[$self kernelcmp 4.0.0] < 0} {
	    # Install dhclient hooks.  These avoid default-router
	    # conflicts when using DHCP on test network interfaces.
	    # Default is to convert default routes into /16 routes, unless
	    # the interface is eth0.  These can be changed by BACKBONE and
	    # TESTNETMASK entries in /etc/sysconfig/network
	    $self copyto "$UTF::unittest/etc/dhclient-enter-hooks" "/etc"
	    $self copyto "$UTF::unittest/etc/dhclient-exit-hooks" "/etc"
	}

	# List of RPMs that ought to have been pre-installed, but
	# might have been forgotten.  Use yum to fetch from the
	# original distribution.
	set yum "yum"
	if {[$self kernelcmp 3.0.0] < 0} {
	    set yum_rpm_list {
		tftp expect tcl tclx php-cli php-mysql mysql
		hping3 sysstat gnuplot binutils tcpdump
	    }
	} elseif {[$self kernelcmp 4.0.0] < 0} {
	    set yum_rpm_list {
		tftp expect tcl tclx php-cli php-mysqlnd mariadb
		hping3 sysstat gnuplot binutils subversion tcpdump
		compat-expat1 expat.i686 ncurses-libs.i686 zlib.i686
	    }
	} else {
	    set yum "dnf"
	    set yum_rpm_list {
		tftp expect tcl tclx php-cli php-mysqlnd mariadb
		hping3 sysstat gnuplot binutils subversion tcpdump
		compat-expat1 expat.i686 ypbind glibc.i686
		elfutils-libelf.i686 zlib.i686 ncurses-libs.i686
	    }
	}

	# Check for essential packages
	if {[catch {$self rpmq {*}$yum_rpm_list} ret]} {
	    UTF::Message WARN $options(-name) \
		"Fetching missing packages from the original distribution"
	    $self -t 120 $yum install --disablerepo=updates -y {*}$yum_rpm_list
	}
    }

    method setup_sysctl {} {

	if {[$self kernelcmp 3.0.0] < 0} {
	    set sysctlp 0

	    # Fix congestion cotrol in FC11 or higher
	    if {[$self kernelcmp 2.6.29] > 0 &&
		[catch {$self grep tcp_congestion_control /etc/sysctl.conf}]} {
		$self rexec { /bin/echo -e '\n# Prevent TCP slow-start\nnet.ipv4.tcp_congestion_control = bic' >> /etc/sysctl.conf }
		set sysctlp 1
	    }

	    # Expand socket buffers/windows
	    if {[catch {$self grep net.core.wmem_max /etc/sysctl.conf}]} {
		$self rexec { /bin/echo -e '\n# Allow large socket buffers/windows\nnet.core.wmem_max = 4194304\nnet.core.rmem_max = 4194304' >> /etc/sysctl.conf }
		set sysctlp 1
	    }
	    if {$sysctlp} {
		$self rexec sysctl -e -p
	    }
	} elseif {[$self kernelcmp 4.0.0] < 0} {
	    $self copyto $UTF::unittest/etc/sysctl.d_19-utf.conf \
		/etc/sysctl.d/19-utf.conf
	    $self systemctl restart systemd-sysctl.service
	} else {
	    $self copyto $UTF::unittest/etc/sysctl.d_22-utf.conf \
		/etc/sysctl.d/22-utf.conf
	    $self systemctl restart systemd-sysctl.service
	}
    }


    UTF::doc {
	# [call [arg host] [method setup_svn]]

	# Install wrapper for svn-1.7 on Fc15 and earlier OSs
    }
    method setup_svn {} {
	if {[$self kernelcmp 3.0.0] < 0 &&
	    [file isdirectory /tools/oss/packages/i686-rhel4]} {
	    if {[catch {$self test -f /usr/bin/svn.dist}]} {
		catch {$self mv /usr/bin/svn /usr/bin/svn.dist}
	    }
	    $self copyto "$UTF::unittest/etc/svn_for_fc15" "/usr/bin/svn"
	}
    }

    variable setup_iperf 0
    method setup_iperf {} {
	if {$setup_iperf} {
	    return
	}
	set setup_iperf 1
	$self -x pkill -9 iperf
	foreach i {iperf208} {
	    set search "$::UTF::projtools/linux{-[$self kernel],}/bin/$i"
	    set s [lindex [glob $search] 0]
	    if {[$self sumcheck $s /usr/local/bin/$i]} {
		$self -x pkill -9 $i
		$self copyto $s "/usr/local/bin/$i"
	    }
	}
	if {[set iperf [$self cget -iperf]] eq "iperf"} {
	    set iperf iperf208
	}
	$self ln -sf $iperf /usr/local/bin/iperf

	# Install iperf init script.
	if {[$self sumcheck "$UTF::unittest/etc/iperf" "/etc/init.d/iperf"]} {
	    $self copyto "$UTF::unittest/etc/iperf" "/etc/init.d/iperf"
	}
	if {[$self cget -iperfdaemon] &&
	    ![info exists UTF::TcpReadStats]} {
	    $self rexec {/sbin/chkconfig iperf reset; service iperf restart}
	}
    }

    UTF::doc {
	# [call [arg host] [method setup_supplicant]]

	# Install locally built wpa_supplicant.  This is installed in
	# /usr/local leaving the original in /usr/sbin.  This is
	# because the new supplicant doesn't support wext and we may
	# still need access to wext.  Only fc19 currently supported.
    }

    method setup_supplicant {} {
	if {[regexp fc19 [$self kernel]]} {
	    UTF::BuildFile::copyto $self [$self findimages "wpa_supplicant"] \
		/usr/local/bin/wpa_supplicant.local
	    # Fix up library reference
	    $self rexec \
		"test -e /lib64/libssl.so.1.0.0 || ln -sf libssl.so.1.0.1e /lib64/libssl.so.1.0.0"
	}
    }

    method isskylake {} {
	regexp {Product Name: MKLP7AI-B9} [$self rexec -s dmidecode]
    }

    method setup_services {} {

	# Remove NIS from hosts lookup, otherwise we have a circular
	# dependency.
	$self -n sed -i '/^hosts/s/nis //' /etc/nsswitch.conf

	set domain [$self -n domainname]

	if {$domain ne "sanjose"} {
	    UTF::Message WARN $options(-name) \
		"Skipping service setup for unknown domain: $domain"
	    return
	}

	# Remove DNS settings from sysconfig, since we specify them in
	# resolv.conf instead.
	$self -n sed -i '/^DNS.=10\.19\./d' /etc/sysconfig/network

	if {![catch {$self -n grep -q -F '10.19.' /etc/resolv.conf}]} {
	    UTF::Message LOG $options(-name) "Replace legacy resolv.conf"
	    $self rexec -n "echo 'search sj.broadcom.com broadcom.com
nameserver 10.17.21.20
nameserver 10.17.18.20' > /etc/resolv.conf"
	}

	if {![catch {$self -n grep -F 'sanjose server 10.19.' /etc/yp.conf}]} {
	    UTF::Message LOG $options(-name) "Replace legacy yp.conf"
	    $self rexec -n "echo 'domain sanjose server nis1.sj.broadcom.com
domain sanjose server nis2.sj.broadcom.com
domain sanjose server nis3.sj.broadcom.com' > /etc/yp.conf"
	}

	if {[catch {$self -n -s cat /etc/ntp.conf} conf]} {
	    $self -n yum install -y ntp
	    set conf [$self -n -s cat /etc/ntp.conf]
	}
	set old $conf
	regsub -line -all {^(server .*fedora\.pool.*)} $conf "\#\\1" conf
	regsub -all {\nserver\s+[^\n]+\n} $conf {} conf
	append conf "
server   nis1	iburst
server   nis2	iburst
server   nis3	iburst"
	if {$conf ne $old} {
	    set fd [$self rpopen -n -in -rw "cat > /etc/ntp.conf"]
	    puts $fd $conf
	    close $fd
	}
	$self -n chkconfig ntpd on
	$self -n service ntpd start
    }

    method setup_debian {} {

	$self copyto /usr/bin/fsh /usr/bin/fsh
	$self copyto /usr/bin/fshd /usr/bin/fshd
	$self copyto /usr/bin/in.fshd /usr/bin/in.fshd
	$self copyto /usr/share/fsh /usr/share/

	catch {$self -n {test -f /etc/init.d/iperf && service iperf stop}}
	if {[catch {$self -n test -f /usr/local/bin/iperf}]} {
	    $self -n apt-get install iperf
	    $self -n ln -s /usr/bin/iperf /usr/local/bin/iperf
	}
	# Install iperf init script.  Remove any old iperfd scripts.
	$self copyto "$UTF::unittest/etc/iperf" "/etc/init.d/iperf"
	$self -n service iperf restart

    }


    UTF::doc {
	# [call [arg host] [method setup_usb_rules]]

	# Setup rules for USB Serial device mapping
    }

    method setup_usb_rules {} {
	$self copyto $UTF::unittest/etc/99-usb-serial.rules \
	    /etc/udev/rules.d/99-usb-serial.rules
    }

    method setup_relay {} {
	$self -n mkdir -p /usr/UTF/bin
	$self rsync [glob "$UTF::unittest/{bin,*shell,consolelogger}"] \
	    /usr/UTF --exclude .git* --delete-excluded
    }

    method SaveHostKeys {} {
	$self authorize
	$self rexec {cd /etc/ssh;tar -cjf /tmp/savedhostkeys \
			 ssh_host_{dsa_,rsa_,}key{,.pub}}
	$self copyfrom /tmp/savedhostkeys /tmp/savedhostkeys
    }

    method RestoreHostKeys {} {
	$self configure -ssh {ssh -o UserKnownHostsFile=/tmp/ukhf}
	$self authorize
	$self copyto /tmp/savedhostkeys /tmp/savedhostkeys
	$self rexec {cd /etc/ssh;tar -xjvf /tmp/savedhostkeys \
			 ssh_host_{dsa_,rsa_,}key{,.pub}}
	file delete /tmp/ukhf
    }

    method dhcpd_reset {} {
	$self rexec service dhcpd restart
    }


    UTF::doc {
	# [call [arg host] [method add_epidiag]]

	# Add modules and libraries required to run epidiag
    }

    method add_epidiag {} {
	set archive "$::UTF::projarchives/etc/linux"
	set rel [$self uname -r]
	set mach [$self uname -m]
	# Add dmamem device driver
	set drv "dmamem-$rel.ko"
	if {$mach eq "x86_64"} {
	    set epidiag epidiag64
	} else {
	    set epidiag epidiag
	}
	set dotd false
	switch -g $rel {
	    *FC4* {
		$self copyto "$archive/libgpib-3.2.06.so" /usr/lib
	    }
	    *fc7* -
	    *fc9* {
		$self copyto "$archive/libgpib-3.2.10.so" /usr/lib
	    }
	    *fc11* {
		$self copyto "$archive/libgpib-3.2.14.so" /usr/lib
	    }
	    *fc19* {
		$self copyto "$archive/libgpib-3.2.16.so" /usr/lib
		$self ln -sf libgpib-3.2.16.so /usr/lib/libgpib.so.0
		set dotd true
	    }
	    default {
		error "No libgpib for $rel"
	    }
	}
	$self ldconfig
	$self copyto "$archive/$drv" \
	    "/lib/modules/$rel/kernel/drivers/char/dmamem.ko"
	$self rexec "test -e /dev/dmamem || mknod -m 666 /dev/dmamem c 10 111"
	$self mkdir -p /etc/udev/devices
	$self rexec "test -e /etc/udev/devices/dmamem ||\
                     mknod -m 666 /etc/udev/devices/dmamem c 10 111"
	$self rexec {grep dmamem /etc/udev/rules.d/50-udev.rules || \
	      echo 'KERNEL=="dmamem", MODE="0666"' >> /etc/udev/rules.d/50-udev.rules}
	if {$dotd} {
	    $self rexec {echo "alias char-major-10-111 dmamem" > /etc/modprobe.d/dmamem.conf}
	} else {
	    $self rexec {grep dmamem /etc/modprobe.conf || \
			     echo "alias char-major-10-111 dmamem" >> /etc/modprobe.conf}
	}
	$self depmod

	$self copyto $::UTF::projtools/linux/bin/nvserial /usr/local/bin/
	$self copyto $::UTF::projtools/linux-$rel/bin/$epidiag /usr/local/bin/epidiag
    }

    UTF::doc {
	# [call [arg host] [method add_netconsole] [arg collecthost]
	#      [lb][arg port][rb]]

	# Add netconsole startup to [arg host] to send netconsole UDP
	# packets to a collector on [arg collecthost] port [arg port].
	# [arg port] defaults to 6666.

	# [arg collecthost] should be running [cmd consolelogger], eg:

	# [example_begin]
	consolelogger -p 40010 UDP:6666
	# [example_end]
    }

    method add_netconsole {host {port 6666}} {
	# Try to convert to IP address
	catch {
	    set host [lindex [exec getent hosts $host] 0]
	}

	$self sed -i "'s/.*SYSLOGADDR=.*/SYSLOGADDR=$host/;s/.*SYSLOGPORT=.*/SYSLOGPORT=$port/'" /etc/sysconfig/netconsole
	$self chkconfig netconsole on
	$self service netconsole start
	$self sed -i "'s/LOGLEVEL=.*/LOGLEVEL=6/'" /etc/sysconfig/init
	$self dmesg -n 6
    }

    UTF::doc {
	# [call [arg host] [method add_serialconsole]]

	# Update GRUB to enable serial console on ttyS0.
    }

    method add_serialconsole {} {
	if {[$self kernelcmp 3.0.0] > 0} {
	    $self rexec \
		"sed -i '/GRUB_CMDLINE_LINUX=/s/\"$/ console=tty0 console=ttyS0,115200n8\"/' /etc/default/grub"
	    $self grub2mkconfig
	    # Disable suspend-on-lid-close
	    $self rexec \
		"sed -i 's/#HandleLidSwitch=suspend/HandleLidSwitch=ignore/' /etc/systemd/logind.conf"
	} else {
	    if {[catch {$self rexec grep console= /boot/grub/grub.conf}]} {
		$self sed -i \
		    "'/^\tkernel/s/$/ console=tty0 console=ttyS0,115200n8/'" \
		    /boot/grub/grub.conf
	    }
	}
    }

    UTF::doc {
	# [call [arg host] [method add_usbconsole]]

	# Update GRUB to enable serial console on ttyUSB0.
    }

    method add_usbconsole {} {
	if {[$self kernelcmp 3.0.0] > 0} {
	    $self rexec \
		"sed -i '/GRUB_CMDLINE_LINUX=/s/\"$/ console=tty0 console=ttyUSB0,115200n8\"/' /etc/default/grub"
	    $self grub2mkconfig
	} else {
	    if {[catch {$self rexec grep console= /boot/grub/grub.conf}]} {
		$self sed -i \
		    "'/^\tkernel/s/$/ console=tty0 console=ttyUSB0,115200n8/'" \
		    /boot/grub/grub.conf
	    }
	}
    }

    method blacklist {} {
	if {[$self kernelcmp 3.0.0] > 0} {
	    return; # already disabled in setup
	}
	set archive "$::UTF::projarchives/unix/UTF"
	# Sync blacklist files
	$self rsync $archive/blacklist/ /etc/modprobe.d
	if {[$self kernelcmp 2.6.38] > 0} {
	    # Update initrd on FC15 and later
	    $self make -C /etc/modprobe.d
	}
    }

    UTF::doc {
	# [call [arg host] [method disable_gui]]

	# Disables the X11 GUI on startup, leaving the text mode
	# console.
    }

    method disable_gui {} {
	if {[$self kernelcmp 4.0.0] > 0} {
	    # FC22 and above has set-default
	    $self systemctl set-default multi-user.target
	} elseif {[$self kernelcmp 2.6.38] > 0} {
	    $self ln -fs /lib/systemd/system/multi-user.target \
		/etc/systemd/system/default.target
	} else {
	    # Earlier used init levels
	    $self sed -i "'s/^id:5:/id:3:/'" /etc/inittab
	}
    }

    UTF::doc {
	# [call [arg host] [method check_network_scripts] [arg dev]]

	# Check host's network configuration script for the named
	# device, and fix if neccessary.  It's important to turn off
	# HOTPLUG to avoid race conditions when we load the driver,
	# it's important to turn off PEERDNS to avoid connections on
	# the test net overriding backbone name services, etc.[para]

	# This will only add terms if they are missing, so users may
	# easily override this by setting terms manually.
    }

    variable checked_network_scripts -array {}

    method check_network_scripts {dev} {
	if {[info exists checked_network_scripts($dev)]} {
	    return
	}

	set ns /etc/sysconfig/network-scripts
	set cfg [file join $ns ifcfg-$dev]
	if {[catch {$self -n cat $cfg} ret]} {
	    if {[regexp {No such file} $ret]} {
		if {[catch {$self -n test -d $ns}]} {
		    UTF::Message LOG $options(-name) \
			"System does not use sysconfig"
		    set checked_network_scripts($dev) 1
		    return
		}
		set ret ""
	    } else {
		error $ret
	    }
	}
	set new $ret

	# Make sure existing data ends in a newline
	if {$new ne "" && ![regexp {\n$} $new]} {
	    append new "\n"
	}
	if {![regexp -line {^DEVICE=} $new]} {
	    append new "DEVICE=$dev\n"
	}
	if {![regexp -line {^HOTPLUG=} $new]} {
	    append new "HOTPLUG=no\n"
	}
	if {![regexp -line {^PEERDNS=} $new]} {
	    append new "PEERDNS=no\n"
	}
	if {![regexp -line {^ONBOOT=} $new]} {
	    append new "ONBOOT=no\n"
	}
	if {![regexp -line {^DEFROUTE=} $new]} {
	    append new "DEFROUTE=no\n"
	}
	if {$new ne $ret} {
	    $self rexec -n "echo '$new' > $cfg"
	}
	set checked_network_scripts($dev) 1
    }


    UTF::doc {
	# [call [arg host] [method iscontroller]]

	# Returns true if this host is where the UTF script is
	# running.  Used for shortcutting -relay operations.
    }

    method iscontroller {} {
	if {[set ip [$base cget -lan_ip]] eq ""} {
	    set ip [$base cget -name]
	}
	expr {[UTF::resolve $ip] eq [UTF::resolve]}
    }

    UTF::doc {
	# [call [arg host] [method socket] [arg host] [lb][arg
	# port][rb]]

	# Relay client socket creation.  If the relay is the same as
	# the Controller then this is the same as using the [cmd
	# socket] built-in, otherwise it uses a remote [cmd nc]
	# process to relay the socket connection.  server sockets
	# (listeners) are not supported.  [arg host] may be of the
	# form "host:port", which will override the seperate [arg
	# port] argument.  Port may be qualified with /tcp (default)
	# or /udp.
    }

    method socket {host {port 23}} {
	regexp {(.*):(.*)} $host - host port
	set proto "tcp"
	regexp {(.*)/(.*)} $port - port proto
	if {$proto eq "tcp"} {
	    if {[$self iscontroller]} {
		UTF::Message LOG $options(-name) "socket $host $port"
		socket $host $port
	    } else {
		$self rpopen -n -rw nc $host $port
	    }
        } elseif {$proto eq "udp"} {
	    # 99 sec is the largest timer compatible with both nc 1.x and 6.x
	    if {[$self iscontroller]} {
		set h localhost
	    } else {
		set h $self
	    }
	    # kill off stale nc relay processes (which show up as ncat
	    # in ps)
	    $self rexec -x "pkill -f 'ncat $host -u -w 99 $port'"
	    set fd [$self rpopen -n -rw "echo; exec nc $host -u -w 99 $port"]
	    gets $fd
	    return $fd
	} else {
	    error "unknown protocol: $proto"
	}
    }

    UTF::doc {
	# [call [arg host] [method epidiag] [arg script]]

	# Run epidiag script [arg script] on [arg host].

	# "s include" and "exit" will be added as necessary.
    }

    method epidiag {SCRIPT} {
	if {$options(-47xxtcl) eq ""} {
	    error "Please set -47xxtcl"
	}
	$self rexec -t 240 "
		echo 'catch {s include}; $SCRIPT; exit' > /tmp/episcript
		cd $options(-47xxtcl) && \
		/usr/local/bin/epidiag -j -n -l -f /tmp/episcript
    	"
    }

    UTF::doc {
	# [call [arg STA] [method if_txrx] [arg device]
        # [arg txp] [arg op_txp] [arg rxp] [arg op_rxp]
        # [arg txb] [arg op_txb] [arg rxb] [arg op_rxb]]

	# Gets the STA current ifconfig statistics for TX/RX packets and
        # TX/RX bytes. The new values are stored in object level variables
        # if_tx_pkts, if_rx_pkts, if_tx_bytes & if_rx_bytes.[para]

        # The [arg device] parameter is automatically supplied by the
        # UTF framework. All other arguments are optional.
        # [arg txp] [arg rxp] [arg txb] [arg rxb] default to null.
        # [arg op_txp] [arg op_rxp] [arg op_txb] [arg op_rxb] default to EQ.
        # If your test has expectations about how much the individual stats
        # should change, you can specify these values in the arguments.
        # Null values will cause the stats delta checks to be skipped.
        # You may specify values for some deltas while ignoring other
        # deltas.[para]

        # If the counters did not change as expected, this method will
        # throw an error. [arg txp] & [arg rxp] specify how much the 
        # packet counters should change. [arg txb] & [arg rxb] specify
        # how much the byte counters should change. The corresponding
        # [arg op_xxx] specifies how the expected delta should be compared.
        # op_xxx=EQ means an error will occur if the actual delta is not
        # equal to the specified value. op_xxx=GE means an error will occur
        # if the actual delta is less than the specified minimum value.
        # op_xxx=LE means an error will occur if the actual delta is
        # greater than the specified maximum value.
    }

    # Object level variables used by method if_txrx
    variable if_tx_bytes 0
    variable if_rx_bytes 0
    variable if_tx_pkts 0
    variable if_rx_pkts 0

    # In order to make this method work on the STA object, UTF.tcl has
    # the statement: UTF::stamethod if_txrx
    # This somehow inserts the option -device parameter into the calling
    # arguments.
    method if_txrx {device {txp ""} {op_txp ""} {rxp ""} {op_rxp ""}\
        {txb ""} {op_txb ""} {rxb ""} {op_rxb ""}} {

        # Clean up & log calling parameters
        set txp [string trim $txp]
        set op_txp [string trim $op_txp]
        set op_txp [string toupper $op_txp]
        if {$op_txp == ""} {
           set op_txp EQ
        }
        set rxp [string trim $rxp]
        set op_rxp [string trim $op_rxp]
        set op_rxp [string toupper $op_rxp]
        if {$op_rxp == ""} {
           set op_rxp EQ
        }
        set txb [string trim $txb]
        set op_txb [string trim $op_txb]
        set op_txb [string toupper $op_txb]
        if {$op_txb == ""} {
           set op_txb EQ
        }
        set rxb [string trim $rxb]
        set op_rxb [string trim $op_rxb]
        set op_rxb [string toupper $op_rxb]
        if {$op_rxb == ""} {
           set op_rxb EQ
        }
        set host [$self cget -name]
        UTF::Message INFO "$host" "if_txrx self=$self device=$device\
            txp=$txp op_txp=$op_txp rxp=$rxp op_rxp=$op_rxp\
            txb=$txb op_txb=$op_txb rxb=$rxb op_rxb=$op_rxb"

        # Get ifconfig output for STA device.
        set ifconfig_data [$self rexec ifconfig $device]
        
        # Parse out packet & byte counts.
        if {![regexp -nocase {^.*RX packets:(\d+).*TX packets:(\d+).*RX bytes:(\d+).*TX bytes:(\d+).*$} $ifconfig_data - x1 x2 x3 x4]} {
            error "if_txrx ERROR: failed to parse data!"
        }

        # Compute deltas against last run. Data was saved in object level variables.
        # Currently FC4 & FC6 use 32-bit unsigned integer counters on ifconfig. 
        # Use 64-bit math to avoid overflows inside TCL. If we get a negative delta,
        # we warn the user that the counter has wrapped around and adjust the result
        # accordingly. If newer versions of Linux use 64-bit counters, then this
        # algorithm will need updating.
        #
        # To test the wrap code below:
        # 1) run pings on the STA for a bit
        # 2) run this routine from interactive utf
        # 3) reload the STA driver to force counters back to 0
        # 4) run the routine again, get negative deltas --> counters wrapped
        set wrap_adjustment [expr wide(pow(2,32))] ;# 4 Billion
        # puts "wrap_adjustment=$wrap_adjustment"
        set delta_rxp [expr wide($x1 - $if_rx_pkts)]
        if {$delta_rxp < 0} {
            UTF::Message WARN "$host" "Counter if_rx_pkts wrapped, adjusting delta."
            set delta_rxp [expr wide($delta_rxp + $wrap_adjustment)]
        }
        set delta_txp [expr wide($x2 - $if_tx_pkts)]
        if {$delta_txp < 0} {
            UTF::Message WARN "$host" "Counter if_tx_pkts wrapped, adjusting delta."
            set delta_txp [expr wide($delta_txp + $wrap_adjustment)]
        }
        set delta_rxb [expr wide($x3 - $if_rx_bytes)]
        if {$delta_rxb < 0} {
            UTF::Message WARN "$host" "Counter if_rx_bytes wrapped, adjusting delta."
            set delta_rxb [expr wide($delta_rxb + $wrap_adjustment)]
        }
        set delta_txb [expr wide($x4 - $if_tx_bytes)]
        if {$delta_txb < 0} {
            UTF::Message WARN "$host" "Counter if_tx_bytes wrapped, adjusting delta."
            set delta_txb [expr wide($delta_txb + $wrap_adjustment)]
        }

        # Save new values in object level variables.
        set if_rx_pkts $x1
        set if_tx_pkts $x2
        set if_rx_bytes $x3
        set if_tx_bytes $x4
        UTF::Message INFO $host "if_txrx\
            if_tx_pkts=$if_tx_pkts if_rx_pkts=$if_rx_pkts\
            if_tx_bytes=$if_tx_bytes if_rx_bytes=$if_rx_bytes"
        UTF::Message INFO $host "if_txrx\
            delta_txp=$delta_txp delta_rxp=$delta_rxp\
            delta_txb=$delta_txb delta_rxb=$delta_rxb"

        # If expected delta parameters were specified, check that they occured.
        # The op parameter controls the logic used to check the deltas. There
        # is a separate op_xxx parameter for each pair of deltas.
        foreach {name expected op actual} [list txp $txp $op_txp $delta_txp\
            rxp $rxp $op_rxp $delta_rxp txb $txb $op_txb $delta_txb\
            rxb $rxb $op_rxb $delta_rxb] {
            # puts "name=$name expected=$expected op=$op actual=$actual"
            if {$expected == ""} {
                continue
            }
            if {$op == "GE"} {
                if {$actual < $expected} {
                    error "if_txrx ERROR: delta mismatch\
                        expected delta_$name=$expected op=$op actual delta_$name=$actual"
                }
            } elseif {$op == "LE"} {
                if {$actual > $expected} {
                    error "if_txrx ERROR: delta mismatch\
                        expected delta_$name=$expected op=$op actual delta_$name=$actual"
                }
            } elseif {$op == "EQ"} {
               if {$actual != $expected} {
                    error "if_txrx ERROR: delta mismatch\
                        expected delta_$name=$expected op=$op actual delta_$name=$actual"
                }
            } else {
                # All other op values are invalid.
                error "if_txrx ERROR: invalid op=$op, should be EQ|GE|LE"
            }
        }
    }


    method kernel {} {
	if {$kernel eq ""} {
	    set kernel [$self -noinit uname -r]
	}
	set kernel
    }

    UTF::doc {
	# [call [arg STA] [method kernelcmp] [lb][arg k1][rb] [arg
	# k2]]

	# Compare kernel versions.  Returns an integer less than,
	# equal to, or greater than zero if k1 is found, respectively,
	# to be less than, to match, or be greater than k2.  If [arg
	# k1] is not specified, the kernel version of the current
	# device will be used.
    }

    method kernelcmp {a {b ""}} {
	if {$b eq ""} {
	    set b $a
	    set a [$self kernel]
	}
	UTF::ddcmp $a $b
    }

    UTF::doc {
	# [call [arg STA] [method {supplicant start}] [arg device]]

	# Start supplicant for [arg device]
    }

    method {supplicant start} {device} {
	$self rexec wpa_supplicant -B -i$device \
	    -c /etc/wpa_supplicant/wpa_supplicant.conf
	UTF::Sleep 10
    }

    UTF::doc {
	# [call [arg STA] [method {supplicant stop}] [arg device]]

	# Stop all supplicants ([arg device] is currently ignored)
    }

    method {supplicant stop} {{device ""}} {
	if {![catch {$self killall wpa_supplicant}]} {
	    UTF::Sleep 2
	}
    }

    UTF::doc {
	# [call [arg STA] [method wpa_cli] [arg -i] [arg device]
	#         [arg {args ...}]]

	# Wrapper around the wpa_cli tool.  This is needed because
	# wpa_cli doesn't uise return codes, so we have to explicirtly
	# check for "OK"
    }

    method wpa_cli {args} {
	set ret [$self rexec -n -s wpa_cli {*}$args]
	if {$ret eq "FAIL"} {
	    UTF::Message ERROR [$self cget -name] $ret
	    error $ret $::errorInfo
	} else {
	    regsub "Selected interface '.*'\n" $ret {} ret
	    regsub "OK" $ret {} ret
	    UTF::Message LOG [$self cget -name] $ret
	    return $ret
	}
    }

    UTF::PassThroughMethod serialrelay -serialrelay

}

# Retrieve manpage from last object
UTF::doc [UTF::Linux man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]
    package require UTF
    UTF::Linux White -lan_ip 10.19.12.138 -sta {STA1 eth3}
    STA1 wl status
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
    # [see_also [uri APdoc.cgi?UTF::Router.tcl UTF::Router]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
