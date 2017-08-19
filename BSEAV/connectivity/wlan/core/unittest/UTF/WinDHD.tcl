#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: 2fbdbb2dfa80b62b3fde55808e93b3b0d51e8a95 $
# $Copyright Broadcom Corporation$
#

package provide UTF::WinDHD 2.0

package require snit
package require UTF::doc
package require UTF::Cygwin

UTF::doc {
    # [manpage_begin UTF::WinDHD n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Windows DHD support}]
    # [copyright {2007 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::WinDHD is an implementation of the UTF host object, specific
    # to DHD-supported dongles on Windows systems.

    # Once created, the WinDHD object's methods are not normally
    # invoked directly by test scripts, instead the WinDHD object is
    # designated the host for one or more platform independent STA
    # objects and it will be the STA objects which will be refered to
    # in the test scripts.

    # WinDHD is based on the UTF::Cygwin object, and in fact delegates
    # most methods and options to an internal instance of that object.

    # [list_begin definitions]

}

snit::type UTF::WinDHD {
    UTF::doc {
	# [call [cmd UTF::WinDHD] [arg host]
	#	[option -name] [arg name]
	#       [lb][option -image] [arg driver][rb]
	#       [lb][option -brand] [arg brand][rb]
	#       [lb][option -tag] [arg tag][rb]
	#       [lb][option -type] [arg type][rb]
	#       [lb][option -date] [arg date][rb]
        #       [arg ...]]

	# Create a new WinDHD host object.  The host will be
	# used to contact STAs residing on that host.
	# [list_begin options]

	# [opt_def [option -name] [arg name]]

	# Name of the host.

	# [opt_def [option -image] [arg driver]]

	# Specify a default driver to install when [method load] is
	# invoked.  This can be an explicit path to a [file Setup.exe]
	# or [file bcmsddhd.sys] file, or a suitable list of arguments
	# to [method findimages].

	# [opt_def [option -brand] [arg brand]]

	# Specify build brand.  Default is [file win_external_dongle_sdio].

	# [opt_def [option -tag] [arg tag]]

	# Select a build tag.  Default is [file trunk].

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2005.8.17.0].  Default is to
	# return the most recent build available.

	# [opt_def [arg ...]]

	# Unrecognised options will be passed on to the host's [cmd
	# UTF::Cygwin] object.

	# [list_end]
	# [list_end]

	# [para]
	# WinDHD objects have the following methods:
	# [para]
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    # base handles any other options and methods
    component base -inherit yes

    option -image
    option -relay
    option -sta
    option -name -configuremethod CopyOption
    option -tag "trunk"
    option -host_tag
    option -date "%date%"
    option -type BcmDHD/Bcm_Sdio_DriverOnly
    option -installer inf
    option -device
    option -console -configuremethod _adddomain
    option -postinstall
    option -assertrecovery -type snit::boolean -default true
    option -altsys
    option -deadbootloader -type snit::boolean -default false
    option -alwayspowercycledongle -type snit::boolean -default false
    option -use_devcon_restart -type snit::boolean -default false
    option -debuginf -type snit::boolean -default true; # unused
    option -dongleimage
    option -embeddedimage
    option -nvram
    option -hack -type snit::boolean -default true; # use hacks below by default

    # -usedll always returns a boolean, but it can be set to -1
    # (auto), which will then cause it to return true iff the brand
    # includes mfgtest.
    option -usedll -type {snit::integer -min -1 -max 1} -default -1 \
	-cgetmethod cgetusedll

    method cgetusedll {o} {
	if {$options($o) == -1} {
	    regexp {mfgtest} [$self cget -brand]
	} else {
	    return $options($o)
	}
    }

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	install base using \
	    UTF::Cygwin %AUTO% -init [mymethod init] \
	    -user user -msgcallback [mymethod msgcallback] -nobighammer 1 \
	    -nointerrupts 1 -brand win_external_dongle_sdio
	$self configure -sys ""; # clear NIC default
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	# default sys name
	if {[$self cget -sys] eq ""} {
	    $self configure -sys [$self defsysfile]
	}
	if {![regexp {^[5-7]} [$self cget -osver]]} {
	    # Shared WEP not supported after Win7
	    $self configure -nosharedwep 1
	}
	foreach {sta dev} $options(-sta) {
	    # Windows reports MAC addresses in uppercase.  Empty or
	    # numeric device names will be unaffected.
	    set dev [string toupper $dev]
	    UTF::STA ::$sta -host $self -device $dev
	}
	if {[$self cget -relay] ne ""} {
	    UTF::Message WARN $options(-name) "\
-relay no longer needed for serial consoles.
If you really need a relay, eg due to missing tools or complex
network topology, use -serialrelay instead"
	    $base configure -serialrelay [$self cget -relay]
	}

	# Need at least one device name for loading driver.  We only
	# expect one device right now anyway.
	set options(-device) $dev
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

    method init {} {
	$self open_messages
	$base init
    }

    method deinit {} {
	$self close_messages
	$self configure -initialized 0
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

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2007.4.24.0].  Default is to
	# return the most recent build available.

	# [opt_def [arg file]]

	# [arg file] should be the name of the driver [file .sys]
	# file, eg [file bcmwlhigh5.sys], or the name of the install
	# shield folder, eg [file InstallShield] or [file
	# Neptune_InstallShield].  If [arg file] is a folder other
	# than an InstallShield folder then it is assumed to be a
	# user's private source tree and will be searched accordingly.

	# [list_end]
    }

    method findimages {args} {
	set saved_args $args

	# Findimages start
	UTF::Message LOG $options(-name) "findimages $args"

	# First get control options
	UTF::GetKnownopts {
	    {all "return all matches"}
	    {ls "Report ls -l"}
	    {showpattern "Show search patterm"}
	    {wantdriver "Looking for a host driver"}
	}

	# Now apply defaults from -image
	if {$options(-image) ne ""} {
	    set args [concat $options(-image) $args]
	    # Updated args
	    UTF::Message LOG $options(-name) "findimages $args"
	}

	# Defaults from object
	UTF::GetKnownopts [subst {
	    {brand.arg "[$self cget -brand]" "brand"}
	    {tag.arg "$options(-tag)" "Build Tag"}
	    {host_tag.arg "$options(-host_tag)" "Host driver Tag"}
	    {type.arg "$options(-type)" "Build Type"}
	    {date.arg "$options(-date)" "Build Date"}
	    {app_tag.arg "[$self cget -app_tag]" "Build Tag"}
	    {app_date.arg "[$self cget -app_date]" "Build Date"}
	    {app_brand.arg "[$self cget -app_brand]" "Build Tag"}
	    {sys.arg "[$self cget -sys]" "Install file"}
	    {installer.arg "$options(-installer)" "Installer"}
	    {dongleimage.arg "$options(-dongleimage)" "Override dongle image"}
	    {gub.arg "[$self cget -gub]" "Alternate GUB build path"}
	}]

	set file [lindex $args end]

	if {$(app_brand) eq ""} {
	    set (app_brand) $(brand)
	}

	# Might get modified for some bits
	set tag $(tag)
	set brand $(brand)
	set date $(date)

	if {$file eq ""} {
	    if {!$(wantdriver) && $(host_tag) ne ""} {
		# Overriding host tag, so focus on dongle else we
		# won't know what version was loaded.
		set file "rtecdc.bin"
	    } else {
		switch $(installer) {
		    sys {
			set file $(sys)
		    }
		    InstallDriver -
		    inf {
			set file "*.inf"
		    }
		    InstallShield {
			set file InstallShield
		    }
		    default {
			error "Invalid -installer: $(installer)"
		    }
		}
	    }
	} elseif {[file isdirectory $file]} {
	    # Look for a driver, assuming this is part of a developer
	    # workspace.
	    if {[regexp {dhd} $(sys)]} {
		return [glob "$file{{{{{/dhd,}/wdm,}/obj,}/i386,}/checked,}/$(sys)"]
	    } else {
		return [glob "$file{{{{{{/wl,}/sys,}/wdm,}/build*,}/objchk_*,}/*,}/$(sys)"]
	    }
	} elseif {[file exists $file]} {
	    return $file
	}

	if {$file eq ""} {
	    set file $(sys)
	} elseif {[file exists $file]} {
	    return $file
	}

	# XXX Cygwin.tcl modifies release not type.  To be resolved.
	switch -re [$self cget -osver] {
	    {^5} {
		set (type) "{WinXP/,}$(type)"
	    }
	    {^[67]} {
		set (type) [file join "{Win7,WinVista}" $(type)]
	    }
	    default {
		set (type) [file join "Win*" $(type)]
	    }
	}

	# XXXX HACK: ROMTERM2 and F15 are missing the images they need
	# in the driver brand, so temporarily pull them from the
	# hndrte-dongle-wl brand.
        set hack [$self cget -hack]
	if {$file eq "rtecdc.exe" ||
	    ($file eq "rtecdc.bin" &&
	     (![regexp {_dongle} $brand] ||
	      ([regexp {4_218|4_219|5_50} $(tag)] && $hack)))} {
	    # Check PREBUILD and regular dongle build
	    set tag "{PREBUILD_DONGLE/$brand/,}$(tag)"
	    set brand hndrte-dongle-wl
	    set platform linux
	    if {$(dongleimage) ne ""} {
		set type $(dongleimage)
		if {[file extension $file] ne ".exe" ||
		    [regsub -all {.bin|.trx} $type {.exe} type]} {
		    regsub {.*/(rtecdc\..*)} $type {\1} file
		    regsub {(?:/rtecdc)?(?:\.exe|\.bin(?:\.trx)?)$} $type \
			{} type
		}
	    } else {
		# XXX This is a misuse of -image, but the effects of
		# failure are minor - it is only used to try to locate
		# a sys file we can scan to try to identify the
		# embedded dongle image to find its symbol table.
		# This needs to be cleaned up.
		if {$options(-image) ne ""} {
		    set sys [file join [file dirname $options(-image)] $(sys)]
		} elseif {[regexp {\.sys} $saved_args] ||
			  [file isdirectory [lindex $saved_args 0]]} {
		    # For a sys install we look in the same sys.  Same
		    # with a developer build.
		    set sys [$self findimages \
				 {*}[lreplace $saved_args end end]]
		} else {
		    # For other install we look in the build
		    set sys [$self findimages {*}$saved_args *.sys]
		}
		if {[regexp {dhdpcie} [$self cget -brand]]} {
		    set type ""
		    foreach f [glob [file dir $sys]/*rtecdc.bin] {
			if {[regexp -line {.* FWID.*\Z} [exec strings $f] id]} {
			    UTF::Message INFO $options(-name) $id
			    lappend type [lindex $id 0]
			}
		    }
		    error "Locating dhdpcie symbol tables not implemented"
		} elseif {[catch {localhost rexec \
				      "strings -a $sys |\
                                grep -x \"43\[-\[:alnum:]]*/\[-\[:alnum:]]*\""} type]} {
		    UTF::Message WARN $options(-name) $type
		    UTF::Message WARN $options(-name) \
			"imagename not found.  Using defaults"
		    # HACK - assuming dongle image type based on driver name
		    switch -g $(sys) {
			bcmwlhigh* {
			    set type 4322-bmac/roml-ag-nodis-assert
			}
			bcmusbdhd* {
			    set type 4322/roml-ndis-vista-noreclaim-noccx-g
			}
			bcmsddhd* {
			    set type 4325b0/sdio-g-cdc-ndis-reclaim-idsup-wme
			}
			default {
			    error "Don't know which dongle image is in $(sys)"
			}
		    }
		}

		# BMAC can include multiple embedded images.  Use
		# -embeddedimage to specify the correct one.  We can't
		# do this automatically since that would involve
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
	    }
	    set tail "{src/dongle/rte/wl/builds,build/dongle}/$type/$file"
	} elseif {$file eq "rtecdc.bin"} {
	    if {[file exists $(dongleimage)]} {
		return $(dongleimage)
	    }
	    set platform window
	    # Only add the .bin suffix if it doesn't already have a
	    # .bin or .trx suffix
	    if {[regexp {\.bin|\.trx} $(dongleimage)]} {
		set file $(dongleimage)
	    } else {
		set file "$(dongleimage).bin"
	    }
	    if {[regexp {/rtecdc\.bin$} $(dongleimage)]} {
	    # Firmware hasn't been copied - pull directly from firmware build
		set brand hndrte-dongle-wl
		set platform linux
		set tail "{src/dongle/rte/wl/builds,build/dongle}/$file"
	    } elseif {[regexp {DriverOnly/43} $(type)] &&
		      ![regexp {/} $file]} {
		# mgftest images with no / are in subfolder
		set tail [file join release $(type) $file]
	    } else {
		set tail [file join release BcmDHD Bcm_Firmware $file]
	    }
	} elseif {$file eq "brcm_wlu.dll"} {
	    # Override type since it's only in Bcm
	    set (type) "Bcm"
	    set platform window

	    if {[regexp {dongle} $brand]} {
		set tail [file join release BcmDHD $(type)_Apps $file]
	    } else {
		# XXX should probably move this to the top of
		# findimages to match Cygwin.tcl
		if {[regexp {^5} [$self cget -osver]]} {
		    set release "release/WinXP"
		} else {
		    set release "release/Win7"
		}
		set tail [file join $release $(type) $(type)_Apps $file]
	    }
	} elseif {[regexp {\.txt} $file] && ![regexp {/} $file]} {
	    # If the nvram file has no directory components, just
	    # look it up in the gallery.
	    return "$::UTF::projgallery/src/shared/nvram/$file"
	} elseif {[regexp {^src/.*\.txt$} $file]} {
	    # Allow the user to specify src/... to force a lookup
	    # relative to the src tree.
	    set platform window
	    set tail $file
        } elseif {[regexp {\.txt$} $file] && !$hack &&
		  ![regexp {DriverOnly/43} $(type)]} {
            # Look for NVRAM file in the normally expected place
	    set platform window
            set tail [file join release BcmDHD $file]

	} elseif {$file eq "wl.exe"} {
	    set platform window
	    set brand $(app_brand)
	    set tag $(app_tag)
	    set date $(app_date)
	    if {[regexp {thresh} $brand]} {
		set tail [file join src/wl/exe/Debugwb $file]
	    } else {
		switch -re [$self cget -osver] {
		    {^5} {
			set os "WinXP"
		    }
		    {^[67]} {
			set os "Win7"
		    }
		    default {
			set os "Win*"
		    }
		}
		if {[regexp {64} [$self cget -osver]]} {
		    set a x64
		} else {
		    set a ""
		}
		set tail [file join release $os checked DriverOnly $a $file]
	    }
	} elseif {$file eq "dhd.exe"} {
	    set platform window
	    set tag $(app_tag)
	    set date $(app_date)
	    set tail [file join src/dhd/exe/Debug $file]
	} else {
	    set platform windows
	    set tail [file join release $(type) $file]

	    if {$(host_tag) ne ""} {
		# pull host driver from a different tag if requested.
		# Unlock date on host driver since we asume it's
		# locked to a tag
		set tag $(host_tag)
		set date "%date%"
	    }
	}

	if {[regexp {_REL_} $tag]} {
	    set tag "{PRESERVED/,ARCHIVED/,}$tag"
	} else {
	    set tag "{PRESERVED/,}$tag"
	}

	set pattern [file join \
			 $::UTF::projswbuild $(gub) build_$platform \
			 $tag $brand $date* "$tail{,.gz}"]
	if {$(showpattern)} {
	    UTF::Message INFO $options(-name) $pattern
	}
	UTF::SortImages [list $pattern] \
	    {*}[expr {$(ls)?"-ls":""}] \
	    {*}[expr {$(all)?"-all":""}]
    }

    UTF::doc {
	# [call [arg host] [method load] [lb][arg file][rb]]
	# [call [arg host] [method load] [lb][arg {args ...}][rb]]

	# Load or replace the wl driver.  In the first form [arg file]
	# should be the pathname of a [file Setup.exe], in which case
	# it will be installed, or a [file *.SYS] file, where only
	# that file will be updated.  In the second form, the argument
	# list will be passed on to [method findimage] to find a
	# driver.  If no arguments are specified, those stored in the
	# [arg host]s [option -image] option will be used instead.
	# Filenames are relative to the control host and files will be
	# copied to [arg host] as needed.  If a version of [syscmd
	# wl.exe] is found with the new driver, the [syscmd wl.exe]
	# command on [arg host] will be updated accordingly.
    }

    method reset_dongle {} {
	$self probe clear
	if {$options(-alwayspowercycledongle)} {
	    # Override flag for dongles that have to power cycled no
	    # matter what state they are in
            set device_reset [$self cget -device_reset]
            set device_reset [string trim $device_reset]
            if {$device_reset != ""} {
                $self device_reset
                return
            }
	    $self power_sta cycle
	    UTF::Sleep 10
	    return
	}
	switch -re [$self cget -sys] {
	    bcmwlhighsd -
	    sdstddhd -
	    sddhd {
		# SDIO normally manages its own resets.  When we find
		# an externally powered SDIO card that needs a reset,
		# add code here.
	    }
	    bcmdl3 {
		# 4315 usb.  Don't know how to recover this yet.  Add
		# code here when we see what sort of recovery is
		# needed.
	    }
	    {usbdhd|wlhigh} {
		if {$options(-console) eq ""} {
		    UTF::Message WARN $options(-name) \
			"No console so cannot reset dongle cleanly."
		    UTF::Message WARN $options(-name) \
			"If you need the dongle power cycled every load, use -alwayspowercycledongle true."
		    return
		}
		if {$options(-deadbootloader)} {
		    # 4326 has a serial bug which means the console
		    # doesn't work in the bootloader.  We use this to
		    # detect if the dongle needs rebooting - ie the
		    # reboot only works if needed.
		    if {![catch {$self rte reboot}]} {
			UTF::Message WARN $options(-name) \
			    "Dongle still running after disable! PR\#54032"
		    }
		    # USB Dongle may be slow to recover
		    UTF::Sleep 5
		} else {
		    # 4322 fixes this bug so in this case a
		    # non-responding console means the dongle has
		    # crashed and needs power-cycling.  We can also
		    # try "reboot", but in this case a result of "?"
		    # means we're correctly at the boot loader.
		    if {[catch {$self rte reboot} ret]} {
			if {[$self cget -power_sta] ne ""} {
			    UTF::Message WARN $options(-name) \
				"Dongle not responding to UART.  Power cycling"

			    # Make sure files are written to disk
			    # before potentially power cycling the
			    # host.
			    $self sync
			    $self power_sta cycle
			    if {[$self cget -power_sta] eq [$self cget -power]} {
				# power_sta was the same as
				# power_host, so we'd better wait for
				# the host to recover.

				$self wait_for_boot
				# Trigger crashcheck after recovery
				catch {$self :}

			    } else {
				# The Dongle should only take a few seconds to recover
				UTF::Sleep 10
			    }
			} else {
			    UTF::Message WARN $options(-name) \
				"Dongle not responding to UART."
			}
		    } elseif {$ret ne "?"} {
			UTF::Sleep 10
		    } else {
			# Sometimes the dongle will already be at the
			# boot loader, but Windows has lost the
			# device.  In this case we need to power cycle
			# the dongle again (we can't reboot it because
			# we can't reboot from the boot loader).
			$self probe
			set c 0
			foreach node [$self Devices "any,node"] {
			    if {[regexp -nocase [$self cget -node] $node]} {
				incr c
			    }
			}
			if {$c == 0} {
			    if {[$self cget -power_sta] ne ""} {
				UTF::Message WARN $options(-name) \
				    "No device found - try power cycling dongle"
				$self probe clear
				$self power_sta cycle
				UTF::Sleep 10
			    } else {
				UTF::Message WARN $options(-name) \
				    "No devices found."
			    }
			}
		    }
		}
	    }
	    default {
		UTF::Message WARN $options(-name) \
		    "Skipping reset of unrecognised sys file: [$self cget -sys]"
	    }
	}
    }

    variable reloadlock 0
    method reload {} {

	try {
	    # Avoid looping if the driver asserts immediately
	    if {[incr reloadlock] != 1} {
		return
	    }
	    UTF::Message INFO $options(-name) "Reload"

	    $self openSetupLogs

	    $self probe clear
	    foreach node [$self Devices "any,node"] {
		if {[catch {$self rexec -e 30 -d "devcon disable" -t 120 \
				"devcon disable '@$node'"} ret] &&
		    ![info exists ::UTF::panic]} {
		    set ::UTF::panic $ret
		}
	    }

	    UTF::Sleep 2 $options(-name) "devcon may reset the dongle itself"

	    set reclaim1 0
	    set reclaim2 0
	    $self reset_dongle
	    $self probe clear

	    set check_nodes {}
	    foreach node [$self Devices "any,node"] {
		if {[regexp -nocase [$self cget -node] $node]} {
		    lappend check_nodes $node
		    $self rexec -e 40 -d "devcon enable" -t 120 \
			"devcon enable '@$node'"
		    if {$options(-use_devcon_restart)} {
			$self rexec -e 30 -d "devcon restart" -t 120 \
			    "devcon restart '@$node'"
		    }
		}
	    }
	    UTF::Sleep 7
	    foreach node $check_nodes {
		$self devcon_status "@$node"
	    }
	    $self closeSetupLogs
	    $self probe clear

	    if {[set w [string trim [$self cget -wlinitcmds]]] ne ""} {
		$self rexec $w
	    }
	} finally {
	    incr reloadlock -1
	}
    }

    variable imageinfo -array {
	tag ""
	date ""
    }

    method imageinfo {} {
	if {$imageinfo(tag) eq ""} {
	    error "image info not found"
	}
	return [array get imageinfo]
    }

    # Save src and exe references, in case host is down when we want
    # to analyse a crash
    variable hndrte_src
    variable hndrte_exe

    method load {args} {

	# These args get stripped
	# Note -altsys gets stripped from options(-image) as well,
	# since it is needed for load, but not for findimages
	UTF::GetKnownopts [subst {
	    {n "Just copy the files, don't actually load the driver"}
	    {sign.arg "[$self cget -sign]" "Sign driver"}
	    {altsys.arg "[from options(-image) -altsys $options(-altsys)]" "Alternate sys file"}
	    {wdiwifisys.arg "[from options(-image) -wdiwifisys [$self cget -wdiwifisys]]" "Alternate service sys file"}
	    {nvram.arg  "$options(-nvram)" "nvram"}
	    {all "ignored"}
	    {ls "ignored"}
	}]

	# These args need to be left in for the findimages to use
	set localargs $args
	UTF::GetKnownopts [subst {
	    {dongleimage.arg "[from options(-image) -dongleimage $options(-dongleimage)]" "Override dongle image"}
	    {installer.arg "$options(-installer)" "Installer"}
	}] "options:" localargs

	UTF::Message INFO $options(-name) "Load DHD Driver"
	set file [$self findimages -wantdriver {*}$args]
	UTF::Message LOG $options(-name) "Found Driver $file"

	if {$(altsys) ne ""} {
	    set altsys [$self findimages {*}$args $(altsys)]
	    UTF::Message LOG $options(-name) "Alternate sys $altsys"
	} else {
	    set altsys ""
	}

 	if {$(dongleimage) ne ""} {
	    if {[file exists $(dongleimage)]} {
		set dongleimage $(dongleimage)
	    } else {
		set dongleimage [$self findimages {*}$args rtecdc.bin]
	    }
	    UTF::Message LOG $options(-name) "Dongle Image $dongleimage"
	} else {
	    set dongleimage ""
	}

	if {$(nvram) ne ""} {
	    set nvram [$self findimages {*}$args $(nvram)]
	    UTF::Message LOG $options(-name) "NVRAM $nvram"
	} else {
	    set nvram ""
	}

	# Find mogrified sources in hndrte
	set m "<none>"

	# Check for source with the exact date first, fall back to
	# wildcarded last component if that fails.  Also check
	# PREBUILD.
	if {$altsys ne ""} {
	    set m $altsys
	} elseif {$dongleimage ne ""} {
	    set m $dongleimage
	} else {
	    set m $file
	}
	if {[regsub {build_window/(.*)/(win.*)/(.*)/release/.*} $m \
		 {build_linux/{PREBUILD_DONGLE/\2/,}/\1/hndrte-dongle-wl/\3/src} m]} {
	    regsub {(.*)\.\d+/src} $m {\1.*/src} md
	} elseif {![regsub {/dongle/rte/wl/builds/.*} $m {} m]} {
	    # Perhaps it's a private build?
	    regsub {/wl/.*\.sys} $m {} m
	}
	catch {$self rm /tmp/hndrte-src.lnk*}
	if {![catch {glob -type d $m} ret] ||
	    ([info exists md] && ![catch {glob -type d $md} ret])} {
	    set m [lindex [lsort $ret] end]
	    UTF::Message LOG $options(-name) "Mogrified src: $m"
	    $self rexec "ln -s '$m' /tmp/hndrte-src.lnk"
	    set hndrte_src $m
	} else {
	    UTF::Message LOG $options(-name) "Mogrified src: $ret"
	}
	catch {$self rm -f /tmp/hndrte-exe.lnk*}
	if {[catch {$self findimages {*}$args rtecdc.exe} ret]} {
	    UTF::Message WARN $options(-name) "Symbol file not found: $ret"
	} else {
	    UTF::Message LOG $options(-name) "Symbol file: $ret"
	    $self rexec "ln -s '$ret' /tmp/hndrte-exe.lnk"
	    set hndrte_exe $ret
	}

	set driverdir /tmp/DriverOnly
	set f [file tail $file]

	# Sanity check, since Windows won't warn us
	if {[regexp {bcmwl\d+} $f]} {
	    error "Config error: loading a NIC driver $f on a dongle"
	}

	$self rexec rm -rf $driverdir

	# If it's a .sys file, and it isn't in a release folder, then
	# just copy the sys file itself, otherwise copy the containing
	# folder.  This is to handle private builds without breaking
	# the manufacturing dongle builds, which are getting
	# increasingly deeper directory structures.
	if {[regexp {.sys(?:.gz)?$} $file] &&
	    ![regexp {Driver|InstallShield} [file tail [file dir $file]]]} {
	    $self mkdir $driverdir
	    if {[regexp {.gz$} $file]} {
		# preserve compressed state.
		$self copyto $file "$driverdir/[$self cget -sys].gz"
	    } else {
		$self copyto $file $driverdir/[$self cget -sys]
	    }
	    set f [$self cget -sys]

	    # If we have private symbols, use them
	    if {[regsub {\.sys} $file {.pdb} pdb] && [file exists $pdb]} {
		$self copyto $pdb $driverdir/
	    }
	} else {
	    $self copyto [file dir $file] $driverdir
	}
	if {[file extension $file] eq ".gz"} {
	    $self gunzip -r $driverdir
	    regsub {\.gz$} $f {} f
	}
	if {$altsys ne ""} {
	    $self copyto $altsys "$driverdir/[$self cget -sys]"
	    # If we have private symbols, use them
	    if {[regsub {\.sys} $altsys {.pdb} pdb] && [file exists $pdb]} {
		$self copyto $pdb $driverdir/
	    }
	}
	if {$(wdiwifisys) ne ""} {
	    $self copyto $(wdiwifisys) "$driverdir/WdiWiFi.sys"
	}
	if {$dongleimage ne ""} {
	    # Discover chipname
	    set fwname [$self rexec \
			    "shopt -s nullglob; cd $driverdir; echo $options(-embeddedimage)*rtecdc.bin"]
	    if {$fwname eq ""} {
		set fwname "rtecdc.bin"
	    } elseif {[llength $fwname] > 1} {
		error "Non unique chipname '$options(-embeddedimage)' - please set a more strict -embeddedimage"
	    }
	    $self copyto $dongleimage "$driverdir/$fwname"
	}
	if {$nvram ne ""} {
	    $self copyto $nvram "$driverdir/nvram.txt"
	}

	if {[$self cget -usedll]} {
	    # Also copy in the MFGTEST DLL
	    $self pkill mfcremote.exe mfcmfgc.exe
	    if {[catch {$self findimages {*}$args brcm_wlu.dll} ret]} {
		set ::UTF::panic "No DLL: $ret"
		UTF::Message FAIL $options(-name) $::UTF::panic
	    } else {
		$self copyto $ret "[$self cygpath -S]/brcm_wlu.dll"
	    }
	}

	# Kill any running wl or dhd
	$self pkill wl.exe dhd.exe

	if {$options(-host_tag) ne "" || [$self cget -app_tag] ne ""} {
	    # DHD and Firmware come from different branches.  Don't
	    # try to optimize out their searches
	    set wl [$self findimages {*}$args "wl.exe"]
	    UTF::Message LOG $options(-name) "wl $wl"
	    $self copyto $wl /usr/bin/wl.exe
	    if {![regexp -nocase {bmac} $options(-type)]} {
		set dhd [$self findimages {*}$args "dhd.exe"]
		UTF::Message LOG $options(-name) "dhd $dhd"
		$self copyto $dhd /usr/bin/dhd.exe
	    }
	} elseif {![catch {$self test -x $driverdir/wl.exe}]} {
	    # Internal brands include wl.exe in the driver package.
	    $self cp $driverdir/wl.exe /usr/bin/wl.exe
	    if {[regexp {dhdpcie} [$self cget -brand]] &&
		![catch {$self test -x $driverdir/dhd.exe}]} {
		$self cp $driverdir/dhd.exe /usr/bin/dhd.exe
	    }
	} else {
	    # Try to find a matching wl command, looking in the apps
	    # folder (external brands)
	    if {[regsub {BcmDHD/.*} $file "BcmDHD/Bcm_Apps/wl.exe{,.gz}" wl]} {
		set wl [file normalize [lindex [glob -nocomplain $wl] 0]]
	    } else {
		set wl ""
	    }
	    if {$wl != "" && [file exists $wl]} {
		UTF::Message LOG $options(-name) "Found matching wl"
		if {[catch {$self copyto $wl /usr/bin/wl.exe} ret]} {
		    error "Copy failed: $ret"
		}
		UTF::Message LOG $options(-name) "Installed matching wl"

		# If we find dhd.exe, also install it.
		set dhd [file join [file dir $wl] "dhd.exe"]
		if {[file exists $dhd]} {
		    $self copyto $dhd /usr/bin/dhd.exe
		    UTF::Message LOG $options(-name) "Installed matching dhd"
		}
	    } else {
		UTF::Message WARN $options(-name) "No wl found"
	    }
	}

	if {[$self cget -sign] && [$self cget -installer] ne "InstallDriver"} {
	    $self sign
	}

	if {$(n)} {
	    UTF::Message LOG $options(-name) "Copied files - not loading"
	    return
	}
	$self services stop

	if {[$self cget -installer] eq "InstallDriver"} {
	    $self installdriver /tmp/DriverOnly
	} else {

	$self openSetupLogs
	$self probe
	$self DeleteDriverOnBoot

	UTF::Message LOG $options(-name) \
	    "HACK: clean out old dongle images and nvram files"
	$self rm -f "[$self cygpath -S]/DRIVERS/{{rtecdc,43*}.bin,{nvram,bcm*}.txt}"
	if {[file extension $f] eq ".inf"} {
	    $self CheckDriverSigningPolicy
	    try {
		incr reloadlock

		UTF::Message INFO $options(-name) \
		    "Copy in new sys file in case we get a crash on unload"

		# Copy in driver and any auxiliary dongle files
		# bash's nullglob extension is used to enumerate
		# available files on the target without error if
		# they don't exist.
		$self rexec rm -f "[$self cygpath -S]/DRIVERS/[$self cget -sys]"
		$self rexec \
		    "shopt -s nullglob; set -x; \
			cp $driverdir/[$self cget -sys] $driverdir/*.{trx,nvm} \
			[$self cygpath -S]/DRIVERS"

		if {$dongleimage ne ""} {
		    $self cp $driverdir/rtecdc.bin "[$self cygpath -S]/DRIVERS"
		}
		if {$nvram ne ""} {
		    $self cp $driverdir/nvram.txt "[$self cygpath -S]/DRIVERS"
		}
		UTF::Message INFO $options(-name) "Reset dongle"
		# If BMAC is stuck this will allow rediscovery.
		# If BMAC is not stuck, it still needs a reset to free dll's.
		# reset is a NO-OP on SDIO.
		# Unload first, so the driver has a chance to reset
		# the dongle safely on its own.
		$self unload
		$self reset_dongle

		# Replace WdiWiFi service if requested.
		if {$(wdiwifisys) ne ""} {
		    $self -x sc stop WdiWiFi
		    $self cp $driverdir/WdiWiFi.sys "[$self cygpath -S]/DRIVERS"
		    $self sc start WdiWiFi
		}

		set found 0
		foreach node [$self Devices "any,node"] {
		    # Strip down to hardware ID
		    regsub {\\[^\\]*$} $node {} node
		    if {![regexp -nocase [$self cget -node] $node]} {
			# Only update matching nodes
			continue
		    }
		    incr found
		    $self cleaninfs $node

		    # If devcon requests a reboot that is normally a
		    # failure, but sometimes we need to let it go
		    # ahead and reboot anyway.
		    if {[$self cget -allowdevconreboot]} {
			set cmd "devcon -r update"
		    } else {
			set cmd "devcon update"
		    }

		    # WAR for ssh environment issues on Win10
		    set cmd "TEMP=/tmp $cmd"

		    if {[catch {
			$self rexec -e 180 -d "devcon update" -t 240 \
			    "$cmd `cygpath -w $driverdir/$f` '$node'"
		    } ret]} {
			# The new devcon.exe returns error 1 to
			# indicate reboot is required, even though the
			# install suceeded.  Report this as a fail,
			# but carry on anyway, since that's what we
			# would have done with the old devcon.
			if {[regexp {Drivers installed successfully.} $ret]} {
			    UTF::Message FAIL $options(-name) \
				"Reboot required"
			    if {![info exists ::UTF::panic]} {
				set ::UTF::panic "Reboot required"
			    }
			    if {[$self cget -allowdevconreboot]} {
				# if we allowed reboot and devcon told
				# us it's rebooting, then give it
				# chance to recover.
				$self wait_for_boot
				# Trigger crashcheck after recovery
				catch {$self :}
			    }
			} else {
			    # If devcon update failed for other
			    # reasons, report them but continue with
			    # the .sys verification anyway.
			    UTF::Message FAIL $options(-name) \
				"Devcon update failed: $ret"
			    set failed "devcon update reported failure"
			}
		    }
		    UTF::Sleep 10
		    set driverfiles [$self rexec "devcon driverfiles '$node'"]
		    if {![regexp -nocase {([\w:\\]+\.SYS)} $driverfiles - sys]} {
			set failed "devcon failed to report driver info"
			UTF::Message FAIL $options(-name) $failed
			# Fall back to configured value
			set sys "[$self cygpath -wS]\\DRIVERS\\[$self cget -sys]"
		    }
		    regsub {.*\\} $sys {} syssrc
		    set syssrc [file join $driverdir $syssrc]
		    # Convert to Cygwin path
		    set sys [$self cygpath "'$sys'"]
		    if {[catch {$self rexec "sum $sys"} ret] ||
			$ret ne [$self sum $syssrc]} {
			set failed "devcon update failed"
			UTF::Message FAIL $options(-name) $failed
			$self rexec "cp $syssrc $sys"
			set failed "devcon update failed - recovered"
			UTF::Message INFO $options(-name) $failed
		    }
		}
		if {$found < 1} {
		    error "No device found to update"
		}
	    } finally {
		incr reloadlock -1
	    }
	} else {
	    # Copy in driver and any auxiliary dongle files
	    # bash's nullglob extension is used to enumerate available
	    # files on the target without error if they don't exist.
	    $self rexec "shopt -s nullglob; set -x; \
			     cp $driverdir/$f $driverdir/*.{trx,nvm,bin} \
			     [$self cygpath -S]/DRIVERS"

	    if {$dongleimage ne ""} {
		$self cp $driverdir/rtecdc.bin "[$self cygpath -S]/DRIVERS"
	    }
	    if {$nvram ne ""} {
		$self cp $driverdir/nvram.txt "[$self cygpath -S]/DRIVERS"
	    }
	}
	$self regsetup

	if {$dongleimage ne ""} {
	    set p {\SystemRoot\system32\drivers\rtecdc.bin}
	} else {
	    set p "unset"
	}
	$self reg $options(-device) DongleImagePath $p

	if {$nvram ne ""} {
	    set p {\SystemRoot\system32\drivers\nvram.txt}
	} else {
	    set p "unset"
	}
	$self reg $options(-device) SROMImagePath $p

	$self reload

	UTF::Sleep 10
	$self UndoDeleteDriverOnBoot
        }

	if {[regexp \
		 {build_window/(?:PRESERVED/)?([^/]+)/([^/]+)/([\d.]+)/} \
		 $file - imageinfo(tag) imageinfo(type) imageinfo(date)]} {
	    # Truncate build name for key
	    regsub {_.*} $imageinfo(tag) {} imageinfo(tag)
	    if {$imageinfo(tag) eq "trunk"} {
		set imageinfo(tag) TOT
	    }
	}

	if {[catch {$self wl ver} ret]} {
	    if {[regexp {No .* found} $ret]} {
		UTF::Message WARN $options(-name) "Wait for devcon to recover"
		for {set i 0} {$i < 30 && [catch {$self wl ver} ret]} {incr i} {
		    UTF::Sleep 1
		}
		if {[catch {$self wl ver} ret]} {
		    set failed "extra reload needed"
		    $self reload
		} else {
		    $self warn "recovered after $i seconds"
		    $self probe clear
		}
	    } else {
		error $ver ::$errorInfo
	    }
	}

	if {[regexp {^7} [$self cget -osver]] &&
	    [catch {$self wl disassoc} ret] && ![regexp {No clock} $ret]} {
	    if {[regexp {Could not disconnect to network with error: "5023"} $ret]} {
		# MSFT Win7 WLANSVC connect problem
		$self warn "$ret: attempting recovery"
		$self rexec sc stop wlansvc
		$self rexec sc start wlansvc
		catch {$self wl disassoc}
	    } else {
		$self warn $ret
	    }
	}

	# Display network interface mapping
	set interfaces [$self Interfaces]
	foreach {wl name} $interfaces {
	    UTF::Message INFO $options(-name) "wl -i $wl is $name"
	}

	regexp {version (.*)} $ret - ret
	if {[info exists failed]} {
	    # Report version and any saved failure messages
	    error "$ret $failed"
	} elseif {$interfaces == {}} {
	    # Driver loaded, but probe had problems.
	    error "$ret No network interfaces found"
	}
	return $ret
    }

    UTF::doc {
	# [call [arg host] [method unload]]

	# Unload the current wl driver.
    }

    method unload {} {
	try {
	    # Avoid reloading if we crash during unload
	    incr reloadlock
	    $self openSetupLogs
	    $self probe
	    if {![catch {$self Devices "any,node"} devs]} {
		foreach node $devs {
		    if {[catch {
			set sys [file join [$self cygpath -W] [$self SYS $node]]
			# move .sys file out of the way, to aid
			# recovery if unload crashes.
			set missing [catch {$self mv $sys "$sys.off"}]
			$self rexec -e 30 -d "devcon disable" -t 120 \
			    "devcon disable '@$node'"
			if {!$missing} {
			    # Copy .sys file back after a successful
			    # unload so we can reload it again.  Use
			    # cp not mv due to a rename() bug in early
			    # versions of Cygwin 1.7.0
			    $self cp $sys.off $sys
			}
		    } ret]} {
			if {[regexp {reboot the system} $ret] &&
			    [$self cget -allowdevconreboot]} {
			    $self reboot
			    $self wait_for_boot 30
			    catch {$self :}
			} elseif {[regexp {Timeout|child killed} $ret]} {
			    # If devcon disable gets killed it's probably
			    # crashing, so give it chance to recover.  Vista
			    # systems may take 5 minutes or more.
			    if {![catch {$self devcon_status "@$node"} ret] &&
				[regexp {Device is currently stopped} $ret]} {
				set ret "Power cycle WAR for PR#99626"
				UTF::Message WARN $options(-name) $ret
				$self power cycle
			    }
			    $self wait_for_boot 30
			    # Trigger crashcheck after recovery
			    catch {$self :}
			}
			error $ret
		    }
		}
	    }
	    $self probe clear
	    if {[regexp {usbdhd} [$self cget -sys]]} {
		if {![string is false $options(-deadbootloader)]} {
		    if {![catch {$self rte reboot}]} {
			error "Dongle still running after disable! PR\#54032"
		    }
		} else {
		    UTF::Message WARN $options(-name) "WAR for PR\#59225"
		    $self power_sta cycle
		}
	    }
	    $self closeSetupLogs
	} finally {
	    incr reloadlock -1
	}
    }

    UTF::doc {
	# [call [arg host] [method rte_available]]

	# Returns true if rte commands are supported
    }

    method rte_available {} {
	if {[$self cget -console] eq ""} {
	    return 0
	} else {
	    return 1
	}
    }

    method rte {args} {
	if {[regexp {dhdpcie} [$self cget -brand]]} {
	    # Use dhd pass through
	    try {
		set rtecapture ""
		$self dhd cons $args
		UTF::Sleep 1
	    } finally {
		set ret $rtecapture
		unset rtecapture
	    }
	} else {
	    set ret [[$self cget -serialrelay] rexec -t 5 -s \
			 $UTF::usrutf/rteshell -$options(-console) $args]
	}
	return $ret
    }

    # Semaphore to prevent host-side error recovery if dongle-side
    # recovery is already underway.
    variable processingDongleError false
    variable rtecapture

    # Message handling callback.  Used by "base" to process log
    # messages.  Returns true if no further processing is required.
    method msgcallback {msg} {
	switch -re -- $msg {
	    "Download Not needed" {
		set ::UTF::panic $msg
		UTF::Message FAIL $options(-name) $msg
		UTF::Message INFO $options(-name) "Attempting recovery..."
		$self wl reboot
		$self probe clear
	    }
	    sdstd_check_errs {
		# Only use the first of these.  Asserts get priority
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic $msg
		}
		UTF::Message FAIL $options(-name) $msg
	    }
	    {dhd_bus_rxctl: resumed on timeout} {
		# Only use the first of these.  Asserts get priority
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic $msg
		}
		UTF::Message FAIL $options(-name) $msg
		if {!$processingDongleError} {
		    try {
			set processingDongleError true
			# This may mean the dongle locked up.  Reload.
			catch {
			    $self reload
			}
		    } finally {
			set processingDongleError false
		    }
		}
		return 1
	    }
	    CONSOLE: {
		# if we have an RTE console drop DHD's copy since it
		# is less accurate and delayed, otherwise treat it as
		# if it came from the serial port
		if {$options(-console) ne ""} {
		    return 1
		} else {
		    regsub {^CONSOLE: } $msg {} msg
		    if {[info exists rtecapture]} {
			regsub {^\d{6}\.\d{3} } $msg {} msg; # strip timestamps
			append rtecapture "\n$msg"
		    }
		    $self consolemsg $msg
		    return 1
		}
	    }
	}
	return 0
    }

    method hndrte_src {} {
	if {![info exists hndrte_src]} {
	    if {[catch {$self rexec -noinit -s -q \
			    ls -l /tmp/hndrte-src.lnk} hndrte_src]} {
		regsub {/dongle/rte/.*} [$self hndrte_exe] {} hndrte_src
	    } else {
		regsub {.*-> } $hndrte_src {} hndrte_src
	    }
	}
	set hndrte_src
    }

    method hndrte_exe {} {
	if {![info exists hndrte_exe]} {
	    set hndrte_exe [$self rexec -noinit -s -q \
				ls -l /tmp/hndrte-exe.lnk]
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

    UTF::doc {
	# [call [arg host] [method reclaim]]

	# Returns reclaim bytes from latest dongle load reclaim
	# messages.  Dongle should have been loaded and up for at
	# least a second to give the background tools time to process
	# the reclaim messages from the serial port.
    }
    variable reclaim1 0
    variable reclaim2 0
    method reclaim {} {
	expr {$reclaim1 + $reclaim2}
    }

    method consolemsg {msg} {
	set needreload 0

	if {$msg eq ""} {
	    return
	}

	regsub {^\d{6}\.\d{3} } $msg {} msg; # strip timestamps

	if {[info exists TRAPLOG]} {
	    # If FWID is set we're collecting new-style trap messages
	    append TRAPLOG "$msg\n"
	}

	if {!$processingDongleError &&
	    [regexp {TRAP |ASSERT.*|c\" in line |Trap type .*| Overflow |ep0: unhandled request type } $msg]} {
	    set processingDongleError true
	    if {![info exists TRAPLOG]} {
		# No FWID, but we want to capture these anyway
		set TRAPLOG "$msg\n"
	    }
	    UTF::Message FAIL "$options(-name)>" $msg

	    if {$options(-assertrecovery) &&
		[regexp -nocase {assert|trap|ep0: } $msg]} {
		set needreload 1
	    }

	    # Give DHD driver a moment to collect trap log
	    UTF::Sleep 0.2 quiet

	    package require UTF::RTE
	    set trap [UTF::RTE::parse_traplog $self $TRAPLOG]
	    if {$trap ne ""} {
		set msg $trap
	    }

	    # If there's already an assert, don't overwrite it
	    if {![info exists ::UTF::panic] ||
		![regexp -nocase {assert|trap} $::UTF::panic]} {
		set ::UTF::panic $msg
	    }

	    set processingDongleError false
	    if {$needreload} {
		# Give DHD driver a few seconds to flush messages.
		UTF::Sleep 5
		# Reload driver
		$self reload
	    }
	} elseif {[regexp {reclaim(.*): Returned (\d+) } $msg - s r]} {
	    if {[regexp {section 1} $s]} {
		set reclaim1 $r
	    } else {
		set reclaim2 $r
	    }
	    UTF::Message LOG "$options(-name)>" $msg
	} else {
	    UTF::Message LOG "$options(-name)>" $msg
	}
    }

    typevariable msgfile
    # Internal callback for fileevent below
    method getdata {request} {
	set fd $msgfile($request)
	if {![eof $fd]} {
	    $self consolemsg [gets $fd]
	} else {
	    # Leave non-blocking for safer close.  Any error messages
	    # should have been collected earlier.
	    close $fd
	    UTF::Message LOG $options(-name) "Log closed $fd"
	    unset msgfile($request)
	}
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
	    $base open_messages
	}
	if {$file == ""} {
	    # really, no console
	    return
	}
	set id [$self cget -lan_ip]
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
	    $base close_messages
	}
	set id [$self cget -lan_ip]
	if {[info exists msgfile($id:$file)] &&
	    [file channels $msgfile($id:$file)] ne ""} {
	    UTF::Message LOG $options(-name) "Close $file $msgfile($id:$file)"
	    if {[set pid [pid $msgfile($id:$file)]] ne ""} {
		# Processes to wait for
		catch {exec kill -HUP {*}$pid} ret
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
	# [call [arg host] [method defsysfile]]

	# Guess the default .sys driver filename, based on [option -type]
	# and [option -osver].
    }
    method defsysfile {} {
	set osver [$self cget -osver]
	if {[info exists UTF::Cygwin::ndismap($osver)]} {
	    set ndis $UTF::Cygwin::ndismap($osver)
	} else {
	    error "Unknown OS version: $options(-osver)"
	}
	if {[regexp {dhdpcie} [$self cget -brand]]} {
	    return "bcmpciedhd63.sys"
	}
	switch -re -- $options(-type) {
	    _Sdio_BMac {
		return "bcmwlhighsd${ndis}.sys"
	    }
	    _BMac {
		return "bcmwlhigh${ndis}.sys"
	    }
	    _DbusSdio {
		# DBUS only on WinXP
		return "bcmsdstddhdxp.sys"
	    }
	    default {
		# DHD only on WinXP
		return "bcmsddhd.sys"
	    }
	}
    }

    # Peer passthroughs
    UTF::PassThroughMethod serialrelay -serialrelay

}

# Retrieve manpage from last object
UTF::doc [UTF::WinDHD man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]
    package require UTF
    UTF::DHD White -lan_ip 10.19.12.138 -sta {STA1 eth2}
    STA1 load
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
