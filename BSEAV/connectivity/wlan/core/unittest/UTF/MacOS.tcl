#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::MacOS 2.0

package require snit
package require UTF::doc
package require UTF::Base
package require UTF::RTE

UTF::doc {
    # [manpage_begin UTF::MacOS n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF MacOS support}]
    # [copyright {2005 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::MacOS is an implementation of the UTF host object, specific
    # to MacOS systems.

    # Once created, the MacOS object's methods are not normally
    # invoked directly by test scripts, instead the MacOS object is
    # designated the host for one or more platform independent STA
    # objects and it will be the STA objects which will be refered to
    # in the test scripts.

    # [list_begin definitions]

}

snit::type UTF::MacOS {
    UTF::doc {
	# [call [cmd UTF::MacOS] [arg host]
	#	[option -name] [arg name]
	#	[option -lan_ip] [arg address]
	# 	[lb][option -ssh] [arg path][rb]
	#       [lb][option -image] [arg driver][rb]
	#       [lb][option -brand] [arg brand][rb]
	#       [lb][option -tag] [arg tag][rb]
	#       [lb][option -type] [arg type][rb]
	#       [lb][option -date] [arg date][rb]
        #       [arg ...]]

	# Create a new MacOS host object.  The host will be
	# used to contact STAs residing on that host.
	# [list_begin options]

	# [opt_def [option -name] [arg name]]

	# Name of the host.

	# [opt_def [option -lan_ip] [arg address]]

	# IP address to be used to contact host.  This should be a
	# backbone address, not involved in the actual testing.

	# [opt_def [option -ssh] [arg path]]

	# Specify an alternate command to use to contact [arg host],
	# such as [cmd rsh] or [cmd fsh].  The default is [cmd ssh].

	# [opt_def [option -image] [arg driver]]

	# Specify a default driver to install when [method load] is
	# invoked.  This can be an explicit path to a [file .kext]
	# folder or a suitable list of arguments to [method
	# findimages].

	# [opt_def [option -brand] [arg brand]]

	# Specify build brand.  Default is [file macos-internal-wl].

	# [opt_def [option -tag] [arg tag]]

	# Select a build tag.  Default is [file trunk].

	# [opt_def [option -type] [arg type]]

	# Select a build type.  Build types are [file Debug] or [file
	# Release].  Default is [file Debug].

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2005.8.17.0].  Default is to
	# return the most recent build available.

	# [opt_def [arg ...]]

	# Unrecognised options will be passed on to the host's [cmd
	# UTF::Base] object.

	# [list_end]
	# [list_end]

	# [para]
	# MacOS objects have the following methods:
	# [para]
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -epi_ttcp "/tools/linux/local-rh9.0/bin/epi_ttcp"
    option -image
    option -sta
    option -name -configuremethod CopyOption
    option -app_tag "trunk"
    option -app_date "%date%"
    option -app_brand ""
    option -tag "trunk"
    option -date "%date%"
    option -type "Debug_10_9"
    option -gub
    option -wlinitcmds
    option -console "/var/log/system.log /var/log/wifi.log"
    option -coreserver
    option -embeddedimage
    option -nativetools -type snit::boolean -default false
    option -kextload -type snit::boolean -default true
    option -coldboot -type snit::boolean -default false
    option -kextunload -type {snit::integer -max 2} -default 0

    # base handles any other options and methods
    component base -inherit yes

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts
    variable panicdir

    constructor {args} {
	set cloneopts $args
	install base using \
	    UTF::Base %AUTO% -init [mymethod init] -name $options(-name) \
	    -user root -ping "ping -n -q -o -c %c -s %s" -nomimo_bw_cap 1 \
	    -brand "macos-internal-wl" -nointerrupts auto
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
	if {[regexp {10_5} $options(-type)]} {
	    set panicdir /Library/Logs/PanicReporter
	} else {
	    set panicdir /Library/Logs/DiagnosticReports
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

    UTF::doc {
	# [call [arg host] [method findimages]
	#              [lb][option -all][rb]
	#              [lb][option -ls][rb]
	#              [lb][option -brand] [arg brand][rb]
	#              [lb][option -tag] [arg tag][rb]
	#              [lb][option -type] [arg type][rb]
	#              [lb][option -date] [arg date][rb]
	#              [lb][option -wantiokit][rb]
	#		   [arg file]]

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

	# Select a build tag.  Default is [option -tag] option of [arg
	# host].

	# [opt_def [option -type] [arg type]]

	# Build types are [file Debug] or [file Release].  Default is
	# [option -type] option of [arg host].

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2005.8.17.0].  Default is to
	# return the most recent build available.

	# [opt_def [option -wantiokit]]

	# Serach for an IOKit package instrwsad of a driver package.

	# [opt_def [arg file]]

	# Specify the file name being searched for.  Defaults to
	# BRCM_IOKit_Src_<ver>.dmg

	# [list_end]

    }

    method findimages {args} {
	set saved_args $args

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
	    {brand.arg "[$self cget -brand]" "brand"}
	    {type.arg "$options(-type)" "type"}
	    {tag.arg "$options(-tag)" "Build Tag"}
	    {date.arg "$options(-date)" "Build Date"}
	    {app_tag.arg "$options(-app_tag)" "Build Tag"}
	    {app_date.arg "$options(-app_date)" "Build Date"}
	    {app_brand.arg "$options(-app_brand)" "Build Date"}
	    {wantiokit "Looking for an IOKit"}
	    {gub.arg "[$self cget -gub]" "Alternate GUB build path"}
	}]

	set file [lindex $args end]

	if {$options(-kextload)} {
	    # new kext-based loader

	    if {$(app_brand) eq ""} {
		set (app_brand) $(brand)
	    }

	    # Might get modified for some bits
	    set tag $(tag)
	    set date $(date)
	    set brand $(brand)

	    if {$file eq ""} {
		if {$(wantiokit)} {
		    set file IO80211Family.pkg
		} else {
		    set file AirPortBroadcom43XX.kext
		}
	    } elseif {[file extension $file] ne ".kext" &&
		      [file extension $file] ne ".pkg" &&
		      [file isdirectory $file]} {
		if {$(wantiokit)} {
		    # Look for a private IOKit
		    return [glob "$file{{{{/wl,}/macos,}/package,}/$(type),}/IO80211Family.pkg"]
		} else {
		    # Look for a driver, assuming this is part of a developer
		    # workspace.
		    return [glob "$file{{{{/wl,}/macos,}/build,}/$(type),}/AirPortBroadcom43XX.kext"]
		}
	    } elseif {[file exists $file]} {
		return $file
	    }

	    if {$file eq "wl"} {
		set tag $(app_tag)
		set date $(app_date)
		set brand $(app_brand)
		set tail "exe/macos{,/10.*}/wl"
	    } elseif {[regexp {.exe$} $file]} {
		set kext [file join \
			      [$self findimages \
				   {*}[lreplace $saved_args end end]] \
			      "Contents/MacOS/AirPortBroadcom43XX"]
		if {[catch {localhost rexec \
				"strings -a $kext |\
                                grep -x \"43\[-\[:alnum:]]*/\[-\[:alnum:]]*\""} type]} {
		    UTF::Message WARN $options(-name) $type
		    UTF::Message WARN $options(-name) "imagename not found."
		}
		# MacOS drivers can include multiple embedded images.
		# Use -embeddedimage to specify the correct one.  We
		# can't do this automatically since that would involve
		# runtime checks that may not be possible on a crashed
		# device.  This is checked as a regexp, so you only
		# need to specify just enough to idendify the chip,
		# not the whole firmware image.
		if {[llength $type] > 1} {
		    if {$options(-embeddedimage) eq ""} {
			UTF::Message WARN $options(-name) \
			    "Multiple embedded images.  Please use -embeddedimage to select the right one"
			# Use the first one.  Probably wrong, but
			# backwards compatible.
			set type [lindex $type 0]
		    } else {
			set type [lsearch -inline -regexp $type \
				      $options(-embeddedimage)]
			if {$type eq ""} {
			    $self warn "-Unable to find any embedded firmware matching $options(-embeddedimage)"
			    error "Unable to find any embedded firmware matching $options(-embeddedimage)"
			}
		    }
		}
		UTF::Message LOG $options(-name) "Embedded image $type"
		set tail "../../../src/dongle/rte/wl/builds/$type/rtecdc.exe"
	    } elseif {[regexp {IO80211Family} $file]} {
		set tail [file join macos/package $(type) $file]
	    } else {
		set tail [file join macos/build $(type) $file \
			      Contents/MacOS/AirPortBroadcom43XX]
	    }

	    if {[regexp {_REL_} $tag]} {
		set tag "{PRESERVED/,ARCHIVED/,}$tag"
	    } else {
		set tag "{PRESERVED/,}$tag"
	    }

	    set pattern [file join \
			     $::UTF::projswbuild $(gub) build_macos \
			     $tag $brand $date* build/src/wl \
			     "$tail{,.gz}"]
	    if {$(showpattern)} {
		UTF::Message INFO $options(-name) $pattern
	    }
	    regsub -line -all {/Contents/MacOS/AirPortBroadcom43XX(:?.gz)?$} \
		[UTF::SortImages $pattern \
		     {*}[expr {$(ls)?"-ls":""}] \
		     {*}[expr {$(all)?"-all":""}]] {} ret
	    return $ret
	} else {
	    # old dmg-based loader

	    if {$file eq ""} {
		set file BRCM_IOKit_Src_*.dmg
	    } elseif {[file extension $file] ne ".kext" &&
		      [file extension $file] ne ".pkg" &&
		      [file isdirectory $file]} {
		# Look for a driver, assuming this is part of a developer
		# workspace.
		return [lindex [glob "$file{{{{/wl,}/macos,}/package,}/$(type),}/AirPortBroadcom43XX.pkg" "$file{{{{/wl,}/macos,}/build,}/$(type),}/AirPortBroadcom43XX.kext"] 0]
	    } elseif {[file exists $file]} {
		return $file
	    }

	    if {[regexp {IO80211Family} $file]} {
		set tail [file join build/src/wl/macos/package $(type) $file]
	    } else {
		set tail [file join release $file]
	    }

	    if {[regexp {_REL_} $(tag)]} {
		set (tag) "{PRESERVED/,ARCHIVED/,}$(tag)"
	    } else {
		set (tag) "{PRESERVED/,}$(tag)"
	    }

	    set pattern [file join \
			     $::UTF::projswbuild $(gub) build_macos \
			     $(tag) $(brand) $(date)* "$tail{,.gz}"]

	    if {$(showpattern)} {
		UTF::Message INFO $options(-name) $pattern
	    }
	    UTF::SortImages {*}[list $pattern] \
		{*}[expr {$(ls)?"-ls":""}] \
		{*}[expr {$(all)?"-all":""}]
	}
    }

    UTF::doc {
	# [call [arg host] [method load] [lb][arg file][rb]]
	# [call [arg host] [method load] [lb][arg {args ...}][rb]]

	# Load a wl driver into the running kernel.  In the first form
	# [arg file] should be the pathname of a [file .dmg] disk
	# image, or a [file .pkg] folder.  In the second form, the
	# argument list will be passed on to [method findimages] to
	# find a driver.  If no arguments are specified, those stored
	# in the [arg host]s [option -image] option will be used
	# instead.  Filenames are relative to the control host and
	# files will be copied to [arg host] as needed.  If a version
	# of [syscmd wl] is found with the new driver, the [syscmd wl]
	# command on [arg host] option will be updated accordingly.
    }

    variable reloadlock 0
    method reload {args} {
	UTF::Message INFO $options(-name) "Reload MacOSX Driver"
	try {
	    # Avoid looping if the driver asserts immediately
	    if {[incr reloadlock] != 1} {
		return
	    }
	    if {$options(-kextload)} {
		# new kext-based loader

		# Invalidate tuning cache
		set tuned ""

		# unload may fail due to crashes, etc - catch and try
		# loading anyway.
		if {[catch {$self unload} ret]} {
		    UTF::Message WARN $options(-name) $ret
		}

		if {[regexp {10_5} $options(-type)]} {
		    # Leopard
		    set kextcmd "kextload"
		} else {
		    # Snow Leopard
		    set kextcmd "kextutil"
		    if {[$self stat -f %m /System/Library/Extensions] > [$self stat -f %m /System/Library/Caches/com.apple.kext.caches/Directories/System/Library/Extensions/KextIdentifiers.plist.gz]} {
			UTF::Message INFO $options(-name) \
			    "Updating caches (may take a couple of minutes)"
			$self -t 180 kextcache -system-caches
		    }
		}

		if {[catch {
		    $self rexec -e 60 -d $kextcmd -t 120 \
			$kextcmd -t -v /var/tmp/AirPortBroadcom43XX.kext
		} ret]} {
		    # Strip unhelpful messages
		    regsub -line {^Cache file .* using\.$\n} $ret {} ret
		    regsub -line {^Loading .*kext\.$\n} $ret {} ret
		    regsub -line {.*appears to be loadable.*\n} $ret {} ret
		    regsub -all {\n*\(kernel\) kxld\[com\.apple\.driver\.AirPortBroadcom43XX\]:\s*} $ret { } ret
		    error $ret
		}
		# Leave time for MacOS to make driver available
		UTF::Sleep 1

		if {[info exists ::UTF::MacOSLoadPower]} {
		    # Enable and up interface
		    foreach {sta dev} $options(-sta) {
			# Disabled as it was causing trunk AXI errors
			#$self rexec networksetup -setairportpower $dev on
			$self rexec ifconfig $dev up
		    }
		    UTF::Sleep 1
		}
		#$self wl up
		if {$options(-wlinitcmds) ne ""} {
		    $self rexec [string trim $options(-wlinitcmds)]
		}
		if {![info exists ::UTF::MacOSLoadPower]} {
		    # Leave time for MacOS to complete setup.  Without
		    # this, a "wl up" immediately after load would fail
		    # with "Radio Off"
		    UTF::Sleep 3
		}
	    } else {
		# old dmg-based loader
		$self wl down
		$self wl restart
		$self SystemStarter start wlconf
		$self wl up
	    }
	    set ret [$self wl ver]
	    regexp {version (.*)} $ret - ret
	    return $ret
	} finally {
	    incr reloadlock -1
	}
    }

    # Save exe reference, in case host is down when we want to analyse
    # a crash
    variable hndrte_exe

    method load {args} {
	UTF::GetKnownopts {
	    {n "Just copy the files, don't actually load the driver"}
	    {all "ignored"}
	    {ls "ignored"}
	}
	if {$options(-kextload)} {
	    # new kext-based loader
	    UTF::Message INFO $options(-name) "Load MacOSX Driver"
	    set file [$self findimages {*}$args]
	    UTF::Message LOG $options(-name) "Found $file"
	    set wl [$self findimages {*}$args "wl"]
	    UTF::Message LOG $options(-name) "Found wl $wl"

	    catch {$self -n rm -f hndrte-exe.lnk}
	    if {$options(-embeddedimage) ne ""} {
		if {[catch {$self findimages {*}$args "rtecdc.exe"} ret]} {
		    UTF::Message LOG $options(-name) "Symbol file not found: $ret"
		} else {
		    UTF::Message LOG $options(-name) "Symbol file: $ret"
		    $self rexec -n "ln -s '$ret' hndrte-exe.lnk"
		    set hndrte_exe $ret
		}
	    }

	    $self rexec "cd /Applications/Utilities && if test -d 'AirPort Utility.app'; then rm -rf 'AirPort Utility Disabled.app'; mv 'AirPort Utility.app' 'AirPort Utility Disabled.app'; fi"

	    # Remove old kexts
	    $self rm -rf /var/tmp/AirPortBroadcom43XX.kext{,.dSYM} \
		/System/Library/Extensions/AirPortBroadcom43XX.kext \
		/System/Library/Extensions/IO80211Family.kext/Contents/PlugIns/{Apple,}AirPortBrcm43*.kext
	    # invalidate cache
	    $self touch /System/Library/Extensions

	    # Remove old wlconf startup script
	    $self rm -rf /Library/StartupItems/wlconf

	    if {[file extension $file] eq ".pkg"} {
		$self rm -rf /var/tmp/AirPortBroadcom43XX.pkg
		$self copyto $file /var/tmp/AirPortBroadcom43XX.pkg
		$self rexec {cd /var/tmp && pax -rz < AirPortBroadcom43XX.pkg/Contents/Archive.pax.gz}
	    } else {
		$self copyto $file /var/tmp/AirPortBroadcom43XX.kext
		if {[regexp {PRESERVED} $file]} {
		    $self gunzip -r /var/tmp/AirPortBroadcom43XX.kext
		}
		$self chmod -R a-w /var/tmp/AirPortBroadcom43XX.kext
	    }
	    if {[file isdir $file.dSYM]} {
		$self copyto $file.dSYM /var/tmp/AirPortBroadcom43XX.kext.dSYM
	    }
	    $self copyto $wl /usr/bin/wl
	    $self chmod +x /usr/bin/wl
	} else {
	    # old dmg-based loader
	    UTF::GetKnownopts {
		{noreboot "Don't reboot after install"}
	    }

	    UTF::Message INFO $options(-name) "Load MacOSX Driver"

	    set (type) $options(-type)
	    if {[set j [set i [lsearch $args -type]]] >= 0} {
		incr j
		set (type) [lindex $args $j]
		set args [lreplace $args $i $j]
	    }

	    set file [$self findimages {*}$args]
	    UTF::Message LOG $options(-name) "Found $file"

	    set pkgname AirPortBroadcom43XX*
	    set m /tmp/BRCM_IOKit_Src
	    set pkg $m/$pkgname/$(type)/$pkgname.pkg

	    $self rexec "if mount|grep -q $m; then hdiutil detach $m; fi"
	    $self rm -rf $m
	    set install 1
	    switch -re -- $file {
		{\.kext$} {
		    # Driver
		    $self copyto $file /System/Library/Extensions
		    # invalidate cache
		    $self touch /System/Library/Extensions
		    set install 0
		}
		{\.dmg$} {
		    # Disk image
		    $self copyto $file ${m}.dmg

		    # hdiutil sometimes times-out for no apparent reason.
		    # Retry to see if it helps.
		    set hdicmd "/bin/echo -n brcm_iokit_src |\
hdiutil attach -stdinpass -mountpoint $m ${m}.dmg"
		    if {[catch {
			$self rexec $hdicmd
		    } ret]} {
			if {[regexp -line {^verified} $ret] &&
			    [regexp {Timeout|child killed: software termination} $ret]} {
			    UTF::Try "hdiutil timeout" {
				$self rexec $hdicmd
				error "recovered"
			    }
			} else {
			    error $ret
			}
		    }
		}
		{\.tar\.gz$} {
		    # Tarball
		    $self rexec \
			"mkdir -p $m && cd $m && tar -xz && mv $m/*/* $m/." < $file
		}
		{\.pkg$} {
		    # Package
		    $self mkdir -p $m/$pkgname/{$(type),Tools}/
		    $self copyto $file $m/$pkgname/$(type)/$pkgname.pkg
		    # Try to find a matching wl command
		    regsub {/macos/package/.*} $file {/exe/macos/wl} wl
		    if {[file exists $wl]} {
			UTF::Message LOG $options(-name) "Found matching wl"
			$self copyto $wl $m/$pkgname/Tools/wl
		    }
		}
		default {
		    error "Don't know how to load $file"
		}
	    }
	    catch {$self cp $m/$pkgname/Tools/wl /usr/bin/wl}
	    if {$install} {
		$self rexec -e 240 -d installer -t 300 \
		    installer -verbose -pkg $pkg -target /
	    }

	    $self rexec "if mount|grep -q $m; then hdiutil detach $m; fi"
	    $self rm -rf $m*
	    if {$(noreboot)} {
		return "Reboot Required!"
	    }
	    $self wlconf
	    catch {$self rexec -t 60 reboot}
	    $self wait_for_boot
	}
	if {!$(n)} {
	    $self sync
	    try {
		$self reload
	    } finally {
		foreach {sta dev} $options(-sta) {
		    catch {$self networksetup \
			       -removeallpreferredwirelessnetworks $dev}
		}
	    }
	}
    }

    method wait_for_boot {{t 30}} {
	UTF::Sleep $t
	$self configure -initialized 0
	for {set i 0} {[catch {$self -n :}] && $i < 5} {incr i} {}
    }

    UTF::doc {
	# [call [arg host] [method unload]]

	# Unload the current wl driver (not implemented)
    }

    method unload {} {
	if {!$options(-kextload)} {
	    UTF::Message WARN $options(-name) "Package Unload not implemented"
	    return
	}

	try {
	    # Avoid reloading if we crash during unload
	    incr reloadlock
	    # On older versions of MacOS multiple tries helped because
	    # the trap-app disconnected after the first try.  In
	    # recent versions, repeating the unload doesn't help.
	    # If there were any drivers loaded we must reboot.
	    set tries 1
	    set bundles \
		[$self rexec \
		     "kextstat -l | awk '/com.apple.driver.AirPort/{print \$6}'"]
	    set unloading 0

	    if {$bundles eq ""} {
		# Nothing to do
		return
	    }

	    if {$options(-coldboot)} {
		# Just power cycle
		$self power cycle
		$self wait_for_boot
		$self :
		return
	    }

	    if {$options(-kextunload) > 0} {
		# unload kext from live system to catch memory leaks.
		if {[info exists ::UTF::MacOSLoadPower] &&
		    $options(-kextunload) > 1} {
		    # Down interface, may power down radio on supported systems
		    foreach {sta dev} $options(-sta) {
			$self -x ifconfig $dev down
		    }
		}
		foreach bundle $bundles {
		    for {set i 0} {$i < $tries} {incr i} {
			set unloading 1
			catch {
			    $self kextunload -v -b $bundle
			} ret
			if {![catch {$self kextstat -l -b $bundle} ret] &&
			    $ret eq ""} {
			    # safe to return without completing the outer
			    # loop because these kexts are mutually
			    # exclusive
			    break
			}
		    }
		    if {$i == $tries} {
			UTF::Message WARN $options(-name) \
			    "Failed to unload: $ret"
			set reboot_required 1
		    }
		}


		if {![info exists reboot_required]} {
		    # reinit since a unload crash might recover too
		    # fast for conventional timeouts.
		    $self configure -initialized 0
		    $self :
		    return
		}
	    }

	    # Sleep here doesn't appear to help.
	    #if {$unloading} {
	    #	UTF::Sleep 5 $options(-name) "in case unload crashes"
	    #}
	    $self :
	    # reboot to safely unload the old driver.  Long timeout
	    # because if a kext unload fails the reboot may be delayed
	    # with "/ is busy".  This is still faster than waiting for
	    # "reboot -q" to complete.  "-K" is needed to prevent
	    # retry on disconnect.
	    $self -x -t 120 -K reboot
	    $self wait_for_boot 10
	    $self :
	} finally {
	    incr reloadlock -1
	}
	return
    }

    method load_iokit {args} {
	set pkgname "IO80211Family.pkg"
	set pkg [$self findimages -wantiokit {*}$args]
	$self rm -rf /tmp/$pkgname
	$self copyto $pkg /tmp/
	$self rexec -T 300 installer -verbose -pkg /tmp/$pkgname -target /
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
            set t1 30
        }
        set t2 [string trim $t2]
        if {$t2 == ""} {
            set t2 20
        }
        # The issue here is that if you do the "shutdown -h now" command,
        # the Mac's wont turn on again after their power has been cycled.
        # This is contrary to standard PC behavior. So we dont do a graceful
        # shutdown for Mac's.
        set cmd1 [string trim $cmd1]
        if {$cmd1 == ""} {
            set cmd1 ""
        }
        set cmd2 [string trim $cmd2]
        if {$cmd2 == ""} {
            set cmd2 "reboot"
        }

        # Call the common method
        $base shutdown_reboot $t1 $t2 $cmd1 $cmd2 $args
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

	if {[regexp {leopard} [$self cget -brand]]} {
	    set port AirPort
	} else {
	    set port "Wi-Fi"
	}

	if {$args eq "dhcp"} {

	    # On Leopard and later we need to use networksetup because
	    # ipconfig would drop the link and cause disassociation
	    $self networksetup -setdhcp $port

	    # invalidate cache in case of failure
	    if {[info exists ipaddr($dev)]} {
		unset ipaddr($dev)
	    }
	    for {set i 0} {$i < 10} {incr i} {
		UTF::Sleep 3
		if {![catch {$base ipconfig getifaddr $dev} ip] &&
		    ![regexp {^169\.254\.} $ip]} {
		    # Return ipaddr and Update cache
		    return [set ipaddr($dev) $ip]
		}
	    }
	    error "DHCP failure: $ip"
	} elseif {[llength $args] eq 1} {
	    # Set IP address
	    $self networksetup -setmanual $port $args 255.255.255.0
	    set ipaddr($dev) $args
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
	# [call [arg host] [method macaddr] [arg dev]]

	# Returns the MAC address for [arg dev]
    }
    method macaddr {dev} {
	if {[regexp {ether\s+(\S+)} [$self rexec ifconfig $dev] - mac]} {
	    return $mac
	} else {
	    error "No MAC address found"
	}
    }

    UTF::doc {
	# [call [arg host] [method {route add}] [arg net] [arg mask]
	#	  [arg gw]]

	# Add an IP route to network [arg net] netmask [arg mask]
	# through gateway [arg gw]
    }
    method {route add} {net mask gw} {
	$self rexec route add -net $net $gw $mask
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
	$self rexec route delete -net $net/$len
	$self rexec route add -net $net/$len $gw
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
	# ignore attempts to remove the default route
	if {$len} {
	    $self rexec route delete -net $net/$len $gw
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
	if {[regexp {inet (?:addr:)?([0-9.]+)} [$self ifconfig $dev] - addr]} {
	    # Return ipaddr and Update cache
	    return [set ipaddr($dev) $addr]
	} else {
	    error "$options(-name) No IP address available"
	}
    }

    method rte_available {} {
	expr {![catch {$self wl offloads} ret] && $ret}
    }

    method rte {args} {
	if {[set m [$self wl mpc]]} {
	    $self wl mpc 0
	}
	try {
	    $self wl up
	    $self wl ol_cons $args
	    regsub {^\d{1,6}\.\d{3} } [$self wl ol_cons] {} ret
	} finally {
	    if {$m} {
		$self wl mpc 1
		UTF::Sleep 1
	    }
	}
	return $ret
    }

    UTF::doc {
	# [call [arg host] [method maxmem]]

	# Returns maxmem bytes using "mu" RTE command on the dongle
	# serial port.
    }

    method maxmem {} {
	if {[catch {$self rte mu} ret]} {
	    if {$ret eq ""} {
		error "No response"
	    } else {
		error $ret
	    }
	}
	if {[regexp {Max memory in use: (\d+)} $ret - mm]} {
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
	if {[catch {$self rte mu} ret] || ![regexp {Free: (\d+)} $ret - free]} {
	    error "Free mem not found"
	}
	expr {$free / 1024.0}
    }

     UTF::doc {
	# [call [arg host] [method memstats]]

	# Returns various memory stats.
    }

    method memstats {} {
	# Allow for one retry since mem data is sometimes missing
	if {[catch {$self rte mu} ret] ||
	     ![regexp {Free: (\d+)\(.* In use: (\d+)\(.*, HWM: (\d+)\(} $ret - \
		   free inuse hwm]} {
	    error "Mem stats not found"
	}
	list free $free inuse $inuse hwm $hwm
    }

    variable TRAPLOG
    variable FWID

    method hndrte_src {} {
	if {![info exists hndrte_src]} {
	    regsub {/dongle/rte/.*} [$self hndrte_exe] {} hndrte_src
	}
	set hndrte_src
    }

    method hndrte_exe {} {
	if {![info exists hndrte_exe]} {
	    set hndrte_exe [$self rexec -noinit -s -q \
				ls -l /root/hndrte-exe.lnk]
	    regsub {.*-> } $hndrte_exe {} hndrte_exe
	}
	set hndrte_exe
    }

    method findassert {file line} {
	package require UTF::RTE
	UTF::RTE::findassert $options(-name) $file $line [$self hndrte_src]
    }

    method findtrap {trap stack} {
	package require UTF::RTE
	UTF::RTE::findtrap $options(-name) $trap $stack [$self hndrte_exe]
    }

    variable processingDongleError 0
    typevariable msgfile
    # Internal callback for fileevent below
    method getdata {request} {
	set fd $msgfile($request)
	if {![eof $fd]} {
	    set msg [gets $fd]
	    if {[regexp {:/var/log/sys} $request]} {
		if {![regexp {kernel\[\d+\]:(?:\s+ARPT:) (.*)} $msg - msg] &&
		    ![regexp {configd\[\d+\]: ([LD].*)} $msg - msg]} {
		    return
		}
		# trim timestamp
		regsub {^\d+\.\d+: } $msg {} msg
	    } elseif {[regexp {:/var/log/wifi.log} $request]} {
		# trim timestamp
		regsub {^.*\d\d:\d\d:\d\d\.\d+ } $msg {} msg
	    }

	    if {[info exists TRAPLOG]} {
		# If FWID is set we're collecting new-style trap messages
		regsub {^\d+\.\d+ } $msg {} msg
		append TRAPLOG "$msg\n"
	    }
	    if {[regexp {assert_type} $msg]} {
		# This is not an assert, so log and skip other checks
		UTF::Message LOG $options(-name) $msg
	    } elseif {[regexp {ASSERT pc} $msg]} {
		# Downgrade these to warnings, since they will be handled
		# by the trap handler later
		UTF::Message WARN "$options(-name)>" $msg
	    } elseif {[regexp {FWID (\d\d-[[:xdigit:]]{1,8})} $msg - FWID]} {
		UTF::Message INFO "$options(-name)>" $msg
		if {![info exists TRAPLOG]} {
		    set TRAPLOG "$msg\n"
		}
	    } elseif {!$processingDongleError &&
		      [regexp {^\d+\.\d+ (TRAP |ASSERT.*|Trap type .*| Overflow |Dongle trap|PSM microcode watchdog fired|PSM WD!)} $msg]} {
		set processingDongleError true
		if {![info exists TRAPLOG]} {
		    # No FWID, but we want to capture these anyway
		    set TRAPLOG "$msg\n"
		}
		UTF::Message FAIL "$options(-name)>" $msg
		# Give DHD driver a few seconds to flush messages.
		UTF::Sleep 0.2 quiet

		package require UTF::RTE
		set trap [UTF::RTE::parse_traplog $self $TRAPLOG]
		unset TRAPLOG
		if {$trap ne ""} {
		    set msg $trap
		}

		# If there's already an assert, don't overwrite it
		if {![info exists ::UTF::panic] ||
		    ![regexp -nocase {assert|trap|: out of memory, malloced 0 bytes} $::UTF::panic] ||
		    [regexp -nocase {Dongle assert file} $::UTF::panic]} {
		    set ::UTF::panic $msg
		}

		set processingDongleError false
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
	# [arg file] defaults to [file /var/log/system.log].  Multiple
	# loggers can be opened, so long as they log different [arg
	# file]s.  Attempts to log the same [arg file] multiple times
	# will be logged and ignored.
    }

    method open_messages { {files ""} } {
	if {[$self cget -infrastructure]} {
	    return
	}
	if {$files eq "" && [set files $options(-console)] eq ""} {
	    return
	}
	set id [$self cget -lan_ip]
	foreach file $files {
	    if {[info exists msgfile($id:$file)]} {
		UTF::Message LOG $options(-name) "Open $file (already open)"
		continue
	    }
	    if {[string match "/*" $file]} {
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
	# No need to do crashcheck, power cycling or logging for
	# infrastructure systems.
	if {[$self cget -infrastructure]} {
	    return
	}
	# No point going through recovery process on a Mac if there is
	# no power switch.
	if {[info exists ::UTF::RecordStack] && [$self cget -power] ne ""} {
	    for {set i 0} {[catch {$self -n :} ret] && $i < 2} {incr i} {}
	    if {[regexp {Timeout|child killed|No route to host} $ret]} {
		$self warn "connection timeout: try power cycle"
		$self power cycle
		# If the first thing we do is recover from a crash,
		# then we should report that crash, even it it was
		# from an older run.
		if {!$crashtime} {
		    set crashtime -1
		}
		$self wait_for_boot
	    } elseif {$i > 0} {
		UTF::Message WARN $options(-name) \
		    "unexpected init failure: $ret"
	    }
	    $self open_messages
	    $self crashcheck
	} else {
	    $self open_messages
	}
    }

    method deinit {} {
	$self close_messages
	$self configure -initialized 0
    }

    UTF::doc {
	# [call [arg host] [method wlconf]]

	# Installs a StartupItem script to set initial wl parameters
	# based on -wlinitcmds
    }

    method wlconf {} {
	# Creates a Startup Secript to run wl commands
	set dir "/Library/StartupItems/wlconf"
	$self mkdir -p $dir

	# Enable DHCP debugging
	set cmds "ipconfig setverbose 1"

	# Add wl commands
	if {$options(-wlinitcmds) ne ""} {
	    append cmds "; " [string trim $options(-wlinitcmds)]
	}

	$self rexec {\
cat > /Library/StartupItems/wlconf/StartupParameters.plist <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict><key>Provides</key><array><string>wlconf</string></array></dict>
</plist>
EOF}
	$self rexec "\
cat > /Library/StartupItems/wlconf/wlconf <<'EOF'
#!/bin/sh
. /etc/rc.common
ConsoleMessage \"$cmds\"
$cmds
EOF"
	$self chmod +x "/Library/StartupItems/wlconf/wlconf"
	$self SystemStarter start wlconf

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
	    return 1
	}

	# Leave kern.ipc.maxsockbuf maxed out at 4194304, which is now
	# the MacOS default.

	if {$window == 0} {
	    set sendspace 65536
	    set recvspace 65536
	} else {
	    set window [UTF::kexpand $window]
	    if {$window < 1024} {
		error "Impausibly small window $window bytes"
	    }
	    set sendspace $window
	    set recvspace $window
	}

	if {$tuned ne "$sendspace\n$recvspace"} {
	    $self rexec -n \
		sysctl -w \
		net.inet.tcp.sendspace=$sendspace \
		net.inet.tcp.recvspace=$recvspace

	    # launchctl based iperf will restart automatically
	    $self rexec -n -x launchctl stop net.nlanr.iperf
	    set tuned "$sendspace\n$recvspace"
	}
	return 0
    }

    variable crashtime 0

    method crashcheck {} {
	set lastcrash $crashtime
	if {[catch {$base -noinit -t 5 stat -f {"%m %Sm"} $panicdir} ret]} {
	    # No crash file
	    set crashtime -1
	    # help user figure out if the system rebooted or not.
	    catch {$self -n -t 5 uptime}
	    return
	}

	# save new crashtime locally, but don't update the object var
	# until we've actually scanned the crash dump.
	set thiscrashtime [lindex $ret 0]
	# Macbook Air with no battery may lose its clock on crash, so
	# wach out for negative timestamp changes.
	if {![info exists ::UTF::RecordStack] ||
	    ($lastcrash && $thiscrashtime != $lastcrash)} {
	    set msg [$self scandump]
	    set crashtime $thiscrashtime; # update object var
	    if {$msg ne ""} {
		set msg "[$self cget -name]: $msg"
		# If there's already an assert, don't overwrite it
		if {![info exists ::UTF::panic] ||
		    ![regexp -nocase {assert|trap} $::UTF::panic]} {
		    set ::UTF::panic $msg
		}
	    }
	    return $msg
	}
	UTF::Message INFO $options(-name) "Skipping old memory.dmp"
	set crashtime $thiscrashtime; # update object var
	# help user figure out if the system rebooted or not.
	catch {$self -n -t 5 uptime}
	return
    }

    method scandump {} {
	catch {$base -noinit "cat `ls $panicdir/*.panic | tail -1`"} ret
	if {[regexp {No such file or directory} $ret]} {
	    return ""
	} else {
	    # Compress old panic files so they don't show up again
	    $base -noinit gzip -f $panicdir/*.panic
	}
	$self stacktrace $ret

	if {[regexp -line {panic[^:]*: (.*)} $ret - ret]} {
	    return $ret
	} else {
	    error "panic not found: $ret"
	}
    }

    # Report stacktrace based on atos running on the DUT (may require xcode)
    method stacktrace {panic} {
	if {![regexp -line {AirPortBroadcom43XX.*@0x([[:xdigit:]]+)->} $panic \
		  - loadaddr]} {
	    error "No load addr found"
	}
	set addrlist {}
	foreach {- a} [regexp -line -inline -all \
			   {^0x[[:xdigit:]]+ : (0x[[:xdigit:]]+)} $panic] {
	    lappend addrlist $a;
	}
	$self -n atos -o /var/tmp/AirPortBroadcom43XX.kext/Contents/MacOS/AirPortBroadcom43XX -l $loadaddr {*}$addrlist
    }

    # Report stacktrace based on gdb via coreserver
    method _stacktrace {panic} {
	if {$options(-coreserver) eq ""} {
	    UTF::Message INFO $options(-name) "No coreserver defined."
	    return
	}
	if {0 && ![catch {$options(-coreserver) type lldb}]} {
	    set uselldb 1
	} elseif {![catch {$options(-coreserver) type gdb}]} {
	    set uselldb 0
	} else {
	    UTF::Message INFO $options(-coreserver) "No debugger installed"
	    return
	}

	set tmpdir "~/utftmp"
	set stmpdir "~/utftmps"
	if {[regexp {10_5} $options(-type)]} {
	    # Leopard
	    set kextcmd "kextload -ns $tmpdir"
	} else {
	    # Snow Leopard
	    set kextcmd "kextutil -ns $tmpdir"
	}
	set kextload_needed 0
	foreach {- d a} [regexp -line -inline -all \
			     {(com\..*)\(.*\)\[.*\]@(0x[[:xdigit:]]+)} $panic] {
	    lappend kextaddrs -a "$d@$a"
	    set kextload_needed 1
	}
	if {$kextload_needed} {
	    if {$uselldb} {
		set gdbcmds "
target create com.apple.driver.AirPortBroadcom43XX.sym
target modules add com.apple.iokit.IOPCIFamily.sym
target modules add com.apple.iokit.IONetworkingFamily.sym
target modules add com.apple.iokit.IO80211Family.sym
"
	    } else {
		set gdbcmds "
add-symbol-file com.apple.iokit.IOPCIFamily.sym
add-symbol-file com.apple.iokit.IONetworkingFamily.sym
add-symbol-file com.apple.iokit.IO80211Family.sym
add-symbol-file com.apple.driver.AirPortBroadcom43XX.sym
"
	    }
	}

	if {![regexp {Mac OS version:\n(\w+)} $panic - version]} {
	    UTF::Message WARN $options(-name) "MacOS version not found in panic"
	    return
	}

	if {$uselldb} {
	    #append gdbcmds \
		"target modules add /Users/user/Desktop/KernelDebugKit$version/mach_kernel\n"
	} else {
	    append gdbcmds \
		"add-symbol-file /home/hwnbuild/macos/KernelDebugKit$version/mach_kernel\n"
	}

	foreach {- a} [regexp -line -inline -all \
			   {^0x[[:xdigit:]]+ : (0x[[:xdigit:]]+)} $panic] {
	    append gdbcmds "l *$a\n";
	}

	if {[catch {
	    if {$options(-kextload)} {
		set kext "/var/tmp/AirPortBroadcom43XX.kext"
	    } else {
		set kext "/System/Library/Extensions/AirPortBroadcom43XX.kext"
	    }

	    $options(-coreserver) rm -rf $stmpdir
	    $options(-coreserver) mkdir $stmpdir
	    $options(-coreserver) rexec "echo '$gdbcmds' > $stmpdir/gdbcmds"

	    if {$kextload_needed} {
		# kextload has to be run on a Mac.  DUT would be best.
		$self rm -rf $tmpdir
		$self mkdir $tmpdir
		if {[catch {$self rexec $kextcmd $kextaddrs $kext} ret]} {
		    # if addresses from the panic log don't work, try
		    # the running kernel.
		    UTF::Message WARN $options(-name) "Try running kernel"
		    $self rexec $kextcmd -A $kext
		}

		# Use tar to copy from DUT to coreserver
		set tarball [$self rpopen tar -C $tmpdir -czf - .]
		$options(-coreserver) tar -C $stmpdir -xvzf - <@$tarball
		close $tarball
	    }
	    if {!$uselldb} {
		# Make symlink since Apple's gdb doesn't support
		# substitute-path
		set link [$options(-coreserver) rexec \
			      "strings $stmpdir/com.apple.driver.AirPortBroadcom43XX.sym|sed -n '/GLUB/{s/\\/build.*//p;q;}'"]
		if {$link ne ""} {
		    catch {$options(-coreserver) mkdir /tmp/GLUB && ln -fs $::UTF::projswbuild/build_macos/$options(-tag) $link} ret
		}
	    }
	    try {
		# gdb has to be run on a Mac with /projects.  Cat
		# instructions into gdb so that it will continue on
		# error.  -x stops on error and directing stdin to a
		# file doesn't work with the MacOS gdb.  Pipe through
		# newgrp to get around thye Mac's NFS group
		# permissions issues.
		if {$uselldb} {
		    $options(-coreserver) rexec \
			"cd $stmpdir && lldb -s gdbcmds 2>&1"
		} else {
		    $options(-coreserver) rexec \
			"cd $stmpdir;cat gdbcmds|gdb -q -n 2>&1|\
grep -v 'is more recent'"
		}
	    } finally {
		if {!$uselldb && $link ne ""} {
		    $options(-coreserver) rm -f $link
		}
	    }
	} ret]} {
	    UTF::Message LOG $options(-name) $ret
	}
    }

    UTF::doc {
	# [call [arg host] [method setup]]

	# Installs or updates any local tools needed on a UTF client.
    }

    method setup {} {
	$self configure -ssh ssh
	$self authorize
	if {[catch {$self nvram boot-args} ret]} {
	    set ret ""
	} else {
	    regsub {^boot-args\s+} $ret {} ret
	}
	set new $ret
	# Enable verbose boot (no need to enable any other debug
	# features since they are ineffective in a closed rig)
	if {![regexp -- {-v} $new]} {
	    set new "$new -v"
	}
	# Disable Driver signing requirement
	if {![regexp {kext-dev-mode} $new]} {
	    set new "$new kext-dev-mode=1"
	}
	# Disable root restrictions
	if {![regexp {rootless} $new]} {
	    set new "$new rootless=0"
	}
	# Disable old-platform restrictions
	if {![regexp {platform-check} $new]} {
	    set new "$new bcom.platform-check=0"
	}
	if {$new ne $ret} {
	    puts "'$new' ne '$ret'"
	    $self rexec "nvram boot-args='$new'"
	    $self sync
	    $self -x reboot -q
	    $self wait_for_boot
	}

	set archive "$::UTF::projarchives/Mac/UTF"
	$self rsync "$archive/" /

	# Enable launchd-based iperf
	$self rexec "launchctl list | grep net.nlanr.iperf || \
launchctl load -w /System/Library/LaunchDaemons/iperf.plist"

	$self setup_system

	set ver [$self os_version -tools]
	$self copyto \
	    "$::UTF::projarchives/Tools/apple80211/$ver/apple80211" \
	    /bin/apple80211

    }

    method setup_system {} {
	# Disable Screensaver
	$self -x defaults delete com.apple.screensaver modulePath
	$self -x defaults delete com.apple.screensaver moduleName
	$self -x defaults write com.apple.screensaver askForPassword 0

	# Prevent system sleep
	$self systemsetup -setcomputersleep never
	# Configure Restarts
	$self systemsetup -setrestartfreeze on
	$self -x systemsetup -setrestartpowerfailure on
	$self systemsetup -setwakeonnetworkaccess on
	$self systemsetup -setnetworktimeserver 10.16.16.11
    }

    variable setup_iperf 0
    method setup_iperf {} {
	if {$setup_iperf} {
	    return
	}
	set setup_iperf 1
	foreach i {iperf208} {
	    set s "$::UTF::projarchives/Mac/UTF/usr/bin/$i"
	    if {[$self sumcheck $s /usr/bin/$i]} {
		$self -x pkill -9 $i
		$self copyto $s "/usr/bin/$i"
	    }
	}
	if {[set iperf [$self cget -iperf]] eq "iperf"} {
	    set iperf iperf208
	}
	$self ln -sf $iperf /usr/bin/iperf
    }

    UTF::doc {
	# [call [arg host] [method InstantPanic]]

	# Panics the host for testing panic recovery.
    }

    method InstantPanic {} {
	$self rexec {dtrace -w -n "BEGIN{ panic(); }"}
    }

    UTF::doc {
	# [call [arg host] [method os_version] [lb][option -tools][rb]]

	# Returns the MacOS version number and build id.  If [option
	# -tools] is specified, only the major.minor OS version is
	# returned.  This may be used for locating OS-dependent tools.
    }
    method os_version {args} {
	UTF::Getopts {
	    {tools "Return only major.minor version used for locating tools"}
	}
	set pver [$self sw_vers -productVersion]
	if {$(tools)} {
	    return [join [lrange [split $pver .] 0 1] .]
	}
	set bver [$self sw_vers -buildVersion]
	return "$pver ($bver)"
    }

    method whatami {{STA ""}} {
	$self -x sw_vers
	$self -x system_profiler SPHardwareDataType
	set iam "MacOS"
	if {$STA ne ""} {
	    if {[catch {$STA chipname} c]} {
		UTF::Message WARN $options(-name) $c
		set c "<unknown>"
	    }
	    set iam "$iam $c"
	}
	return $iam
    }


}

# Retrieve manpage from last object
UTF::doc [UTF::MacOS man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]
} {{
    package require UTF
    UTF::MacOS Mac -lan_ip 10.19.12.134 \\
	-sta {STA1 en1}  \\
	-brand macos-internal-wl-leopard -type Debug_10_5

    utf> STA1 wl status
    SSID: "tima-28457-13:20:16"
    Mode: Managed   RSSI: -20 dBm   noise: -92 dBm  Channel: 36
    BSSID: 00:90:4C:98:00:7B        Capability: ESS
    Supported Rates: [ 6(b) 9 12(b) 18 24(b) 36 48 54 ]
    802.11N Capable:
        Chanspec: 5GHz channel 36 20MHz (0x1b24)
        Control channel: 36
        802.11N Capabilities:
        Supported MCS : [ 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 ]

}} {
    # [example_end]

    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also wl]
    # [see_also [uri APdoc.cgi?UTF.tcl UTF]]
    # [see_also [uri APdoc.cgi?UTF::Base.tcl UTF::Base]]
    # [see_also [uri APdoc.cgi?UTF::Cygwin.tcl UTF::Cygwin]]
    # [see_also [uri APdoc.cgi?UTF::Linux.tcl UTF::Linux]]
    # [see_also [uri APdoc.cgi?UTF::Router.tcl UTF::Router]]
    # [manpage_end]
}

## skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
