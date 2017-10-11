#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::DHD 2.0

package require snit
package require UTF::doc
package require UTF::Linux

UTF::doc {
    # [manpage_begin UTF::DHD n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF DHD support}]
    # [copyright {2007 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::DHD is an implementation of the UTF host object, specific
    # to DHD-supported dongles on Linux systems.

    # Once created, the DHD object's methods are not normally
    # invoked directly by test scripts, instead the DHD object is
    # designated the host for one or more platform independent STA
    # objects and it will be the STA objects which will be refered to
    # in the test scripts.

    # DHD is based on the UTF::Linux object, and in fact delegates
    # most methods and options to an internal instance of that object.

    # [list_begin definitions]

}

snit::type UTF::DHD {
    UTF::doc {
	# [call [cmd UTF::DHD] [arg host]
	#	[option -name] [arg name]
	#       [lb][option -image] [arg driver][rb]
	#       [lb][option -brand] [arg brand][rb]
	#       [lb][option -tag] [arg tag][rb]
	#       [lb][option -type] [arg type][rb]
	#       [lb][option -date] [arg date][rb]
	#       [lb][option -wlinitcmds] [arg cmds][rb]
        #       [arg ...]]

	# Create a new DHD host object.  The host will be
	# used to contact STAs residing on that host.
	# [list_begin options]

	# [opt_def [option -name] [arg name]]

	# Name of the host.

	# [opt_def [option -image] [arg driver]]

	# Specify a default driver to install when [method load] is
	# invoked.  This can be an explicit path to a [file wl.ko]
	# file, or a suitable list of arguments to [method
	# findimages].

	# [opt_def [option -brand] [arg brand]]

	# Specify build brand.  Default is [file linux-internal-dongle].

	# [opt_def [option -tag] [arg tag]]

	# Select a build tag.  Default is [file trunk].

	# [opt_def [option -type] [arg type]]

	# Select a build type.  Default is [file
	# debug-native-apdef-stadef*].

	# [opt_def [option -driver] [arg driver]]

	# Select a host driver to use.  Default is [file
	# dhd-cdc-sdstd-debug].  Kernel version will be appended.

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2005.8.17.0].  Default is to
	# return the most recent build available.

	# [opt_def [option -wlinitcmds] [arg cmds]]

	# Specify cmds list to be executed after driver is loaded /
        # reloaded.  Default is null.

	# [opt_def [arg ...]]

	# Unrecognised options will be passed on to the host's [cmd
	# UTF::Linux] object.

	# [list_end]
	# [list_end]

	# [para]
	# DHD objects have the following methods:
	# [para]
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    # base handles any other options and methods
    component base -inherit yes

    option -dhd_image
    option -app_image
    option -dhd_tag
    option -dhd_brand
    option -dhd_date "%date%"
    option -dhd_customer "bcm"
    option -name -configuremethod CopyOption
    option -bus "sdstd"
    option -customer "bcm"
    option -clm_blob ""
    option -txcb ""
    option -txcal ""
    option -device
    option -console -configuremethod _adddomain
    delegate option -hostconsole to base as -console
    option -postcopy
    option -postinstall
    option -preunload
    option -postuninstall_hook
    option -uploadcmds
    option -assertrecovery 1
    option -nvram_add
    option -maxsocram ""
    option -rwlrelay ""
    option -rwlport "8000"
    option -driver ""
    option -node ""

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	install base using \
	    UTF::Linux %AUTO% -init [mymethod init] \
	    -msgcallback [mymethod msgcallback] \
	    -nointerrupts 1 -brand linux-internal-dongle \
	    -type "4325b0/sdio-internal-assert-g-cdc-reclaim-idsup" \
	    -modopts ""

	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	if {$options(-console) eq [$base cget -console]} {
	    UTF::Message WARN $options(-name) \
		"No need to set -console if it is the same as -hostconsole"
	    set options(-console) ""
	}
	foreach {sta dev} [UTF::Staexpand [$self cget -sta]] {
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
	if {[$self cget -relay] ne ""} {
	    UTF::Message WARN $options(-name) "\
-relay no longer needed for serial consoles.
If you really need a relay, eg due to missing tools or complex
network topology, use -serialrelay instead"
	    $base configure -serialrelay [$self cget -relay]
	}
	if {$options(-rwlrelay) eq "localhost"} {
	    $base configure -wl {wl --socket localhost $options(-rwlport)}
	}

	# We don't want the parent trying to reload its own driver.
	$base configure -reloadoncrash 0
    }

    destructor {
	catch {$base destroy}
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
    }

    method deinit {} {
	$self close_messages
	$self configure -initialized 0
    }

    method wl {args} {
	if {$options(-rwlrelay) ne "" && $options(-rwlrelay) ne "localhost"} {
	    # Specifiy port even if it's the default, otherwise wl
	    # will mis-parse commands like 5g_rate
	    $options(-rwlrelay) host wl \
		--socket [$self cget -lan_ip] $options(-rwlport) {*}$args
	} else {
	    $base wl {*}$args
	}
    }


    UTF::doc {

	# [call [arg host] [method findimages]
	#              [lb][option -all][rb]
	#              [lb][option -ls][rb]
	#              [lb][option -bus] [arg bus][rb]
	#              [lb][option -driver] [arg driver][rb]
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

	# [opt_def [option -bus] [arg bus]]

	# Specify bus type.  Default is the [option -bus] option
	# of [arg host].  OBSOLETED by [option -hostdriver]

	# [opt_def [option -driver] [arg bus]]

	# Specify host driver type.  Default is the [option -driver]
	# option of [arg host].

	# [opt_def [option -brand] [arg brand]]

	# Specify build brand.  Default is the [option -brand] option
	# of [arg host].

	# [opt_def [option -tag] [arg tag]]

	# Select a build tag.  Default is the [option -tag] option of
	# [arg host].

	# [opt_def [option -type] [arg type]]

	# Build types describe chips and feature sets such as
	# 4325b0/sdio-internal-assert-g-cdc-reclaim-idsup, etc.
	# Default is [option -type] option of [arg host].

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2007.4.24.0].  Default is to
	# return the most recent build available.

	# [opt_def [arg file]]

	# Specify the file type being searched for.  Defaults to *.bin
	# to find a dingle image.  dhd.ko may also be searched for,
	# depending on kernel version.

	# [list_end]
    }

    method findimages {args} {

	if {[regexp -- {-highsdio-dnglimage} [$self cget -type]]} {
	    # This loads just like a NIC
	    return [$base findimages {*}$args]
	}

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
	    {nvram.arg "" "ignored"}
	    {bus.arg "$options(-bus)" "bus"}
	    {driver.arg "$options(-driver)" "Host Driver"}
	    {brand.arg "[$self cget -brand]" "brand"}
	    {tag.arg "[$self cget -tag]" "Build Tag"}
	    {type.arg "[$self cget -type]" "Build Type"}
	    {date.arg "[$self cget -date]" "Build Date"}
	    {customer.arg "$options(-customer)" "Customer"}
	    {dhd_image.arg "[$self cget -dhd_image]" "DHD image"}
	    {dhd_tag.arg "[$self cget -dhd_tag]" "DHD Tag"}
	    {dhd_brand.arg "[$self cget -dhd_brand]" "DHD Brand"}
	    {dhd_date.arg "[$self cget -dhd_date]" "Build Date"}
	    {dhd_customer.arg "$options(-dhd_customer)" "DHD Customer"}
	    {app_image.arg "[$self cget -app_image]" "App Image"}
	    {app_tag.arg "[$self cget -app_tag]" "Build Tag"}
	    {app_brand.arg "[$self cget -app_brand]" "Build Tag"}
	    {app_date.arg "[$self cget -app_date]" "Build Date"}
	    {app_type.arg "[$self cget -app_type]" "App Type"}
	    {app_customer.arg "[$self cget -app_customer]" "App Customer"}
	    {gub.arg "[$self cget -gub]" "Alternate GUB build path"}
	}]

	set file [lindex $args end]

	if {$(dhd_tag) eq ""} {
	    if {[regexp -- {-bmac} [$self cget -type]]} {
		# BMAC uses High and Low from same tag
		set (dhd_tag) $(tag)
	    } else {
		# Everything else defaults host to trunk
		set (dhd_tag) trunk
	    }
	}
	if {$(dhd_brand) eq ""} {
	    # DHD brand defaults to firmware brand for now
	    set (dhd_brand) $(brand)
	}
	if {$(dhd_date) eq "%date%" && $(dhd_tag) eq $(tag)} {
	    # If tags are the same let DHD date default to firmware
	    # date
	    set (dhd_date) $(date)
	}
	# Set search tag to firmware tag by default.  Once object type
	# is determined below, this may switch to dhd_tag.
	set tag $(tag)
	set brand $(brand)
	set date $(date)
	set customer $(customer)

	# New hostdriver option obsoletes -bus option.
	# Maintain compatibility here:
	if {$(driver) eq ""} {
	    set (driver) dhd-cdc-$(bus)-debug
	}

	if {$file eq ""} {
	    # Only add the .bin suffix if it doesn't already have a
	    # .bin or .trx suffix
	    if {[regexp {\.bin|\.trx} $(type)]} {
		set file $(type)
	    } else {
		set file "$(type).bin"
	    }
	} elseif {[file isdirectory $file]} {
	    # Look for a driver, assuming this is part of a developer
	    # workspace.
	    if {![regexp {([^.]*)\.(.*)} $(type) - typedir typesuffix]} {
		set typedir $(type)
		set typesuffix "bin"
	    }
	    set rtecdc "rtecdc"
	    regsub {/rtecdc$} $typedir {} typedir

	    return [glob "$file{{{/dongle/rte/wl/builds,/../build/dongle,}/$typedir,}/$rtecdc.$typesuffix,{{/$customer,}/firmware,}/$typedir.$typesuffix}"]

        } elseif {$(dhd_image) != "" && [file extension $file] eq ".ko"} {
	    if {[file isfile $(dhd_image)]} {
		# Use specific .ko file user specified
		return $(dhd_image)
	    } elseif {[regexp {(wl|bcm_dbus)\.ko} $file]} {
		# Use developer build for BMAC High driver - either
		# src, release, or flat folder will do.
		return [glob "$(dhd_image){{{/wl,}/linux,}/obj-$(driver)-[$self kernel],}/$file"]
	    } else {
		# Use developer build for DHD driver - either src,
		# release, or flat folder will do.
		return [glob "$(dhd_image){{{/dhd,}/linux,{/$(dhd_customer),}/host,}/$(driver)-[$self kernel],}/$file"]
	    }
	} elseif {$(app_image) != "" && $file eq "wl"} {
	    if {[regexp {x86_64} [$self kernel]]} {
		return [glob "$(app_image){{{/src,}/wl,}/exe,}/wlx86_64"]
	    } else {
		return [glob "$(app_image){{{/src,}/wl,}/exe,}/wl"]
	    }
	} elseif {[UTF::BuildFile::exists $file]} {
	    return $file
	}

	if {$file eq "wl" || $file eq "dhd" || $file eq "wl_server_socket" ||
	    $file eq "bcmdl"} {
	    set tag $(app_tag)
	    set date $(app_date)
	    set brand $(app_brand)
	    set customer $(app_customer)
	    if {$brand eq "linux-combined-apps" && [regexp {fc15} [$self kernel]]} {
		# fc15 isn't in linux-combined-apps
		set brand "linux-internal-dongle-pcie"
	    }
	    if {$brand eq "linux-combined-apps"} {
		if {[regexp {x86_64} [$self kernel]]} {
		    set tail [file join $(app_type) $customer x86_64 $file]
		} else {
		    set tail [file join $(app_type) $customer $file]
		}
	    } elseif {![regexp {ternal-media} $brand] &&
		[regexp -- {-high} $options(-driver)]} {
		if {$file ne "bcmdl" && [regexp {x86_64} [$self kernel]]} {
		    set tail [file join release exe x86_64 $file]
		} else {
		    set tail [file join release exe $file]
		}
	    } elseif {[regexp {x86_64} [$self kernel]]} {
		set tail [file join release $customer apps "{x86_64/$file,${file}64}"]
	    } else {
		set tail [file join release $customer apps $file]
	    }
	} elseif {[lsearch {.clmb .clm_blob .txcb .txcap_blob .msf} \
		       [file extension $file]] >= 0} {
	    if {[regexp {/rtecdc\.(bin|trx|romlsim\.trx)$} $(type)]} {
		# Firmware hasn't been copied - pull directly from firmware build
		set brand hndrte-dongle-wl
		set dir [file dirname $(type)]
		set tail "{build/dongle,src/dongle/rte/wl/builds}/$dir/$file"
	    } else {
		set dir [file dirname $(type)]
		if {[regexp {dhdap} $brand]} {
		    set tail "build/43*/src/wl/clm/src/$file"
		} elseif {[file dirname $dir] eq ".."} {
		    # Olympic paths
		    set tail [file join release $customer firmware $dir $file]
		} else {
		    set tail [file join release $customer firmware $file]
		}
	    }
	} elseif {[regexp {/rtecdc\.(bin|trx|romlsim\.trx)$} $file]} {
	    # Firmware hasn't been copied - pull directly from firmware build
	    set brand hndrte-dongle-wl
	    set tail "{build/dongle,src/dongle/rte/wl/builds}/$file"
	} elseif {$file eq "logstrs.bin" ||
		  $file eq "rtecdc.map" ||
		  $file eq "roml.bin" ||
		  $file eq "roml.map" ||
		  [regexp {rtecdc\.(?:exe|opt)$} $file]} {
	    if {[regexp {dhdap} $brand]} {
		regsub {(\.romlsim)?\.(trx|bin|bin\.trx)$} $(type) {} type
		regsub {/rtecdc$} $type {} type
		set tail "{build/,}43*/{build/dongle,src/dongle/rte/wl/builds}/$type/$file"
		unset type
	    } else {
		set tag "{TEMP/prebuild/$brand/,}$tag"
		set brand hndrte-dongle-wl
		regsub {(\.romlsim)?\.(trx|bin|bin\.trx)$} $(type) {} type
		regsub {/rtecdc$} $type {} type
		set tail "{build/dongle,src/dongle/rte/wl/builds}/$type/$file"
		unset type
	    }
	} elseif {[regexp {\.(bin|txt|trx)$} $file]} {
	    if {[regexp {\.txt} $file] && ![regexp {/} $file]} {
		# The nvram gallery has gone - rewrite path
		set file "src/shared/nvram/$file"
	    }
	    if {[regexp {^src/|^components/} $file]} {
		# Allow the user to specify src/... to force a lookup
		# relative to the src tree.
		# nvram files may be in src/shared or components
		regsub {^src/shared/nvram|^components/nvram} $file \
		    {{src/shared,components}/nvram} file
		# Check for developer build
		if {![regsub {/src$} [lindex $args end-1] "/$file" tail]} {
		    set tail $file
		}
	    } elseif {[regexp {\.txt} $file] && ![regexp {/} $file]} {
		# If the nvram file has no directory components, try
		# src/shared and components

		# Note "?" is to force an existence check in case
		# we're using shell glob instead of tcl glob.
		set tail "$::UTF::projgallery/{src/shared,components}/nvra?/$file"
	    } else {
		if {[regexp {dhdap} $brand]} {
		    regsub {\.bin$} $file {} file
		    set tail "{build/,}43*/{build/dongle,src/dongle/rte/wl/builds}/$file/rtecdc.bin"
		} elseif {![regexp -- {-media$} $brand] &&
		    [regexp -- {-high} $options(-driver)]} {
		    set tail [file join release firmware $file]
		} else {
		    set tail [file join release $customer firmware $file]
		}
	    }
	} else {
	    set tag $(dhd_tag)
	    set brand $(dhd_brand)
	    set date $(dhd_date)
	    set customer $(dhd_customer)
	    if {[regexp -- {-media$} $brand]} {
		if {[regexp {(wl|bcm_dbus)\.ko} $file]} {
		    set drv "$customer/host/wl_driver/obj-$(driver)"
		} else {
		    set drv "$customer/host/dhd_driver/$(driver)"
		}
	    } elseif {[regexp {(wl|bcm_dbus)\.ko} $file]} {
		set drv "obj-$(driver)"
	    } else {
		set drv "$customer/host/$(driver)"
	    }
	    set tail [file join release \
			  ${drv}-[$self kernel] $file]
	}

	if {[regexp {_REL_} $tag]} {
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
	UTF::SortImages $pattern \
	    {*}[expr {$(ls)?"-ls":""}] \
	    {*}[expr {$(all)?"-all":""}]
    }

    UTF::doc {
	# [call [arg host] [method reload]]

	# Reload the driver using files already copied to the host.
    }

    variable reloadlock 0
    variable uploaddone 0
    method reload {} {

	if {[regexp -- {-highsdio-dnglimage} [$self cget -type]]} {
	    # This reloads just like a NIC
	    return [$base reload]
	}

	set waitfortrap 0
	try {
	    # Avoid looping if the driver asserts immediately
	    if {[incr reloadlock] != 1} {
		return
	    }

	    set bin "bin"
	    if {[regexp -- {-high} $options(-driver)]} {
		set dhdexe bcmdl
		set f wl.ko
		set bin "trx"
	    } elseif {[regexp -- {-usb} $options(-driver)]} {
		set dhdexe bcmdl
		set f dhd.ko
		set bin "bin.trx"
	    } else {
		set dhdexe dhd
		set f dhd.ko
	    }
	    $self clean_udev_rules
	    set waitfortrap 1

	    $self unload

	    # Preinstall hook
	    UTF::forall {STA dev} [$self cget -sta] \
		cmd [$self cget -preinstall_hook] {
		    if {[catch [string map [list %S $STA] $cmd] ret]} {
			UTF::Message WARN $STA $ret
		    }
		}
	    set uploaddone 0
	    if {$dhdexe eq "bcmdl"} {
		set dlcmd [list $dhdexe]
		if {$options(-node) ne ""} {
		    lappend dlcmd -i $options(-node)
		}
		if {[$self cget -nvram] ne "" ||
		    [$self cget -nvram_add] ne ""} {
		    lappend dlcmd -n nvram.txt
		}
		lappend dlcmd rtecdc.$bin

		if {[catch {$self -t 60 {*}$dlcmd} dlret] && 1} {
		    if {[regexp {No devices found|Failed to set config|Error: } $dlret]} {
			if {[catch {$self rte reboot} ret] ||
			    ($ret ne "?" &&
			     ([catch {UTF::Sleep 2; $self rte reboot} ret] ||
			      $ret ne "?")) ||
			    [regexp {Error: } $dlret]} {
			    if {[$self cget -power_sta] ne ""} {
				UTF::Message WARN $options(-name) \
				    "Dongle not responding to UART.  Power cycling"
				$self sync
				$self power_sta cycle
				if {[$self cget -power_sta] eq [$self cget -power]} {
				    # power_sta was the same as
				    # power_host, so we'd better wait for
				    # the host to recover.
				    $self wait_for_boot 30
				    # Trigger crashcheck after recovery
				    catch {$self :}
				} else {
				    UTF::Sleep 5
				}
			    } else {
				UTF::Message WARN $options(-name) \
				    "Dongle not responding to UART."
			    }
			}
			$self {*}$dlcmd
		    } else {
			error $dlret $::errorInfo
		    }
		}
		UTF::Sleep 1
	    }
	    # Fix up module dependencies
	    foreach m [split [$self modinfo -F depends $f] ","] {
		if {$m eq "bcm_dbus"} {
		    $self insmod bcm_dbus.ko
		} else {
		    $self modprobe $m
		}
	    }

	    $self rexec insmod $f {*}[$self cget -modopts]

	    if {$options(-postinstall) ne ""} {
		$self rexec $options(-postinstall)
	    }
	    UTF::forall {STA dev} [$self cget -sta] \
		cmd [$self cget -postinstall_hook] {
		    if {[catch [string map [list %S $STA] $cmd] ret]} {
			UTF::Message WARN $STA $ret
		    }
		}
	    if {$dhdexe ne "bcmdl"} {
		if {$options(-maxsocram) ne ""} {
		    if {[regexp {^(\d+)k$} $options(-maxsocram) - m]} {
			set m [expr {1024*$m}]
		    } else {
			set m $options(-maxsocram)
		    }
		    $self dhd -i $options(-device) maxsocram $m
		}
		array set reclaim {}
		array set pre_reclaim {}
		if {[$self cget -nvram] ne "" ||
		    [$self cget -nvram_add] ne ""} {
		    $self dhd -i $options(-device) download \
			rtecdc.$bin nvram.txt
		} else {
		    $self dhd -i $options(-device) download \
			rtecdc.$bin
		}
	    }
	    if {$options(-rwlrelay) ne ""} {
		catch {$self pkill -f wl_server_socket}
		UTF::Sleep 1
		$self rexec \
		    "/usr/bin/wl_server_socket lo0 $options(-rwlport) >/dev/null 2>&1&"
	    }

	    if {[$self cget -clm_blob] ne ""} {
		if {[catch {$self wl clmload 0 rtecdc.clmb} ret opt] &&
		    [set status [$self wl clmload_status]]} {
		    set status [UTF::clmload_statuserr $status]
		    UTF::Message FAIL $options(-name) $status
		    throw FAIL $status
		}
	    }
	    # forgive transient ifconfig failure (partial WAR for SWWLAN-46111)
	    $self rexec -x ifconfig $options(-device) up

	    if {$options(-txcb) ne ""} {
		$self wl txcapload rtecdc.txcb
	    }
	    if {$options(-txcal) ne ""} {
		$self wl calload rtecdc.msf
	    }

	    if {[set wlinitcmds [string trim [$self cget -wlinitcmds]]] ne ""} {
		$self rexec $wlinitcmds
	    }
	    if {[set initscript [string trim [$self cget -initscript]]] ne ""} {
		eval [string map [list %S [lindex [$self cget -sta] 0]] $initscript]
	    }

	    set ret [$self wl ver]
	    regexp {version (.*)} $ret - ret
	    set waitfortrap 0
	    return $ret
	} finally {
	    if {$waitfortrap} {
		# wait in case there is a Trap to collect
		UTF::Sleep 1
	    }
	    incr reloadlock -1
	}
    }

    method wait_for_boot {{t 30}} {
	UTF::Sleep $t
	$self configure -initialized 0
	for {set i 0} {[catch {$self -n :}] && $i < 20} {incr i} {}
    }


    variable imageinfo -array {
	tag ""
	date ""
	type ""
    }

    method imageinfo {} {
	if {$imageinfo(tag) eq ""} {
	    error "image info not found"
	}
	return [array get imageinfo]
    }

    UTF::doc {
	# [call [arg host] [method driver]]

	# Returns full path to the host driver, if the host driver
	# has been loaded.
    }
    variable driver ""
    method driver {} {
	return $driver
    }

    UTF::doc {
	# [call [arg host] [method load] [lb][arg file][rb]]
	# [call [arg host] [method load] [lb][arg {args ...}][rb]]

	# Load a dhd driver into the running kernel.  In the first
	# form [arg file] should be the pathname of a [file dhd.ko]
	# compiled for the current kernel.  In the second form, the
	# argument list will be passed on to [method findimages] to
	# find a driver.  The [arg host]s [option -modopts] option can
	# be used to add module options if needed.  Filenames are
	# relative to the control host and files will be copied to
	# [arg host] as needed.  If corresponding versions of [syscmd
	# dhd] and [syscmd wl] are found with the new driver, the they
	# will be installed on [arg host].  After driver and tool
	# installation, a dongle image will be downloaded to the
	# dongle.
    }

    # Save src and exe references, in case host is down when we want
    # to analyse a crash
    variable hndrte_src
    variable hndrte_exe

    method load {args} {
	if {[regexp -- {-highsdio-dnglimage} [$self cget -type]]} {
	    # This loads just like a NIC

	    # Remove links since it's better to not have them then to
	    # have them pointing to the wrong place.
	    catch {$self -n rm -f hndrte-src.lnk hndrte-exe.lnk}

	    return [$base load {*}$args]
	}

	UTF::GetKnownopts {
	    {n "Just copy the files, don't actually load the driver"}
	    {all "ignored"}
	    {ls "ignored"}
	}

	UTF::Message INFO $options(-name) "Load DHD Driver"

	set bin bin
	set dhdexe dhd
	if {[regexp -- {-high} $options(-driver)]} {
	    set driver "wl.ko"
	    set dhdexe "bcmdl"
	    set bin "trx"
	} elseif {[regexp -- {-usb} $options(-driver)]} {
	    set driver "dhd.ko"
	    set dhdexe "bcmdl"
	    set bin "bin.trx"
	} else {
	    set driver "dhd.ko"
	}
	set f $driver

	# Remove driver - this way if load fails due to missing
	# components, we won't accidentally reload an old driver
	# later.
	$self -n rm -f $f

	set image [$self findimages {*}$args]
	UTF::Message LOG $options(-name) "Dongle Image $image"
	if {[regexp -line {.* FWID.*\Z} [UTF::BuildFile::strings $image] id]} {
	    UTF::Message INFO $options(-name) $id
	}

	if {[$self cget -clm_blob] ne ""} {
	    set blob [$self cget -clm_blob]
	    if {[llength $blob] == 1} {
		# static blob file
		# First try the fw location
		if {[regsub {/rtecdc.*} $image "/$blob" b] &&
		    [UTF::BuildFile::exists $b]} {
		    set blob $b
		} else {
		    # otherwise try builds
		    set blob [$self findimages {*}$args $blob]
		}
		UTF::check_for_preserved $blob
		UTF::Message LOG $options(-name) "CLM blob $blob"
	    }
	}

	if {[$self cget -nvram] ne ""} {
	    set nvram [$self findimages {*}$args [$self cget -nvram]]
	    UTF::check_for_preserved $nvram
	    UTF::Message LOG $options(-name) "NVRAM $nvram"
	} elseif {$options(-nvram_add) ne ""} {
	    # Allow nvram additions, even if there's no base nvram
	    set nvram "/dev/null"
	}

	if {$options(-txcb) ne ""} {
	    set txcb [$self findimages {*}$args $options(-txcb)]
	    UTF::check_for_preserved $txcb
	    UTF::Message LOG $options(-name) "TXCB $txcb"
	}
	if {$options(-txcal) ne ""} {
	    set txcal [$self findimages {*}$args $options(-txcal)]
	    UTF::check_for_preserved $txcal
	    UTF::Message LOG $options(-name) "TXcal $txcal"
	}

	set driver [$self findimages {*}$args $driver]
	UTF::Message LOG $options(-name) "DHD Driver $driver"

	# Note: run modinfo on the controller, since DUT may not
	# have it.  No need to uncompress since modinfo can handle
	# compressed files.
	if {[lsearch [UTF::BuildFile::modinfo -F depends $driver] \
		 "bcm_dbus"] >= 0} {
	    set shim [$self findimages {*}$args bcm_dbus.ko]
	    UTF::Message LOG $options(-name) "Host shim $shim"
	}

	set wl [$self findimages {*}$args "wl"]
	UTF::Message LOG $options(-name) "wl $wl"

	set dhd [$self findimages {*}$args $dhdexe]
	UTF::Message LOG $options(-name) "$dhdexe $dhd"

	UTF::BuildFile::copyto $self $driver $f
	if {[info exists shim]} {
	    $self copyto $shim bcm_dbus.ko
	} else {
	    $self -n rm -f bcm_dbus.ko
	}
	UTF::BuildFile::copyto $self $image rtecdc.$bin
	if {[info exists blob]} {
	    if {[llength $blob] > 1} {
		UTF::Message LOG $options(-name) "CLM blob $blob"
		$self -n $UTF::unittest/bin/clmtrx.sh rtecdc.clmb {*}$blob
	    } else {
		UTF::BuildFile::copyto $self $blob rtecdc.clmb
	    }
	}
	if {[info exists txcb]} {
	    UTF::BuildFile::copyto $self $txcb rtecdc.txcb
	}
	if {[info exists txcal]} {
	    UTF::BuildFile::copyto $self $txcal rtecdc.msf
	}
	if {[info exists nvram]} {
	    UTF::BuildFile::nvram_add_copyto $self $nvram nvram.txt \
		$options(-nvram_add)
	}
	UTF::BuildFile::copyto $self $wl /usr/bin/wl
	UTF::BuildFile::copyto $self $dhd /usr/bin/$dhdexe

	if {[regexp {cfg80211} $driver]} {
	    $self ln -sf wpa_supplicant.local /usr/local/bin/wpa_supplicant
	} else {
	    # WEXT needs original supplicant
	    $self ln -sf /usr/sbin/wpa_supplicant /usr/local/bin/wpa_supplicant
	}
	if {$options(-rwlrelay) ne ""} {
	    if {$options(-rwlrelay) ne "localhost"} {
		UTF::BuildFile::copyto [$self rwlrelay] $wl /usr/bin/wl
	    }
	    set rwlserver [$self findimages {*}$args "wl_server_socket"]
	    UTF::Message LOG $options(-name) "wl_server $rwlserver"
	    catch {$self -n pkill -f wl_server_socket}
	    UTF::BuildFile::copyto $self $rwlserver /usr/bin/wl_server_socket

	}

	if {[regexp \
		 {build_linux/(?:PRESERVED/)?([^/]+)/[^/]+/([\d.]+)/.*firmware/(.*)\.(?:bin|trx)} \
		 $image - imageinfo(tag) imageinfo(date) imageinfo(type)]} {
	    # Truncate build name for key
	    regsub {_.*} $imageinfo(tag) {} imageinfo(tag)
	    if {$imageinfo(tag) eq "trunk"} {
		set imageinfo(tag) TOT
	    }
	}

	# Find mogrified sources in hndrte
	set m "<none>"
	if {[regsub {([^/]*)/(linux-.*)/(.*)\.\d+/release/.*} $image \
		 {{TEMP/prebuild/\2/,}\1/hndrte-dongle-wl/\3.*/src} m]} {
	} else {
	    # Perhaps it's a private build?
	    regsub {(src/dongle/rte/|build/dongle).*} $m {src} m
	}

	catch {$self -n rm -f hndrte-src.lnk}
	if {![catch {glob -type d $m} ret]} {
	    # Note lsort is required since dates may not quite match,
	    # but it may mean we use prebuild instead ofregular
	    # builds.  That should be ok.
	    set m [lindex [lsort $ret] end]
	    UTF::Message LOG $options(-name) "Mogrified src: $m"
	    $self rexec -n "ln -s '$m' hndrte-src.lnk"
	    set hndrte_src $m
	} else {
	    UTF::Message LOG $options(-name) "Mogrified src: $ret"
	}
	catch {$self -n rm -f hndrte-exe.lnk}
	set e "rtecdc.exe"
	set exe [file join [file dirname $image] $e]
	if {![UTF::BuildFile::exists $exe]} {
	    set exe $e
	}
	if {[catch {$self findimages {*}$args $exe} ret]} {
	    UTF::Message LOG $options(-name) "Symbol file not found: $ret"
	} else {
	    UTF::Message LOG $options(-name) "Symbol file: $ret"
	    $self rexec -n "ln -s '$ret' hndrte-exe.lnk"
	    set hndrte_exe $ret
	}

	# RAM AUX files - must match current build
	foreach a {logstrs.bin rtecdc.map} {
	    set aux [file join [file dirname $image] $a]
	    if {[UTF::BuildFile::exists $aux]} {
		UTF::BuildFile::copyto $self $aux $a
	    } else {
		$self -n rm -f $a
	    }
	}
	if {[regexp {roml} [$self cget -type]]} {
	    # ROM AUX files - stale files are better than none, so search
	    # is ok, and don't delete.
	    foreach a {roml.bin roml.map} {
		set aux [file join [file dirname $image] $a]
		if {![UTF::BuildFile::exists $aux]} {
		    set aux $a
		}
		if {![catch {$self findimages {*}$args $aux} aux]} {
		    UTF::BuildFile::copyto $self $aux $a
		}
	    }
	}

	if {$options(-postcopy) ne ""} {
	    $self -n sync
	    eval $options(-postcopy)
	}

	if {$(n)} {
	    UTF::Message LOG $options(-name) "Copied files - not loading"
	    return
	}
	# Sanity check for stale iperf processes.
	if {![catch {$self -n pgrep iperf} ret] && [llength $ret] > 5} {
	    $self rexec -n {ps -fp $(pgrep iperf)}
	    $self warn "stale iperf processes"
	}
	$self -n sync
    	$self reload
    }

    UTF::doc {
	# [call [arg host] [method unload]]

	# Unload the current dhd driver.
    }

    method unload {} {
	if {[regexp -- {-highsdio-dnglimage} [$self cget -type]]} {
	    # This loads just like a NIC
	    return [$base unload]
	}

	UTF::Message INFO $options(-name) "Unload DHD Driver"
	try {
	    # Avoid reloading if we crash during unload
	    incr reloadlock

	    # Preunload
	    if {$options(-preunload) ne ""} {
		$self rexec $options(-preunload)
	    }

	    $self supplicant stop

	    # Unload host driver and shim
	    if {[catch {$self rexec {lsmod|awk '/^(dhd|wl|bcm_dbus) /{print $1}'}} ret]} {
		lappend errs $ret
	    } else {
		foreach module $ret {
		    if {[catch {$self rexec -t 5 rmmod $module} ret]} {
			lappend errs $ret
		    }
		}
	    }
	    if {[info exists errs]} {
		if {[regexp {Timeout|child killed|resource busy} $errs]} {
		    if {[$self cget -power] eq ""} {
			set ret "rmmod timeout: try reboot"
			$self worry $ret
			$self -x reboot
		    } else {
			set ret "rmmod timeout: try power cycle"
			$self worry $ret
			$self power cycle
		    }
		    $self wait_for_boot 60
		    return $ret
		} else {
		    error $errs
		}
	    }
	    # Give driver time to report leaks, etc.
	    UTF::Sleep 1

	    # Postuninstall hook
	    UTF::forall {STA dev} [$self cget -sta] \
		cmd [$self cget -postuninstall_hook] {
		    if {[catch [string map [list %S $STA] $cmd] ret]} {
			UTF::Message WARN $STA $ret
		    }
		}
	} finally {
	    incr reloadlock -1
	}
    }

    UTF::doc {
	# [call [arg host] [method rte_available]]

	# Returns true if rte commands are supported
    }

    method rte_available {} {
	if {[regexp {usb|high} [$self cget -driver]] && [$self cget -console] eq ""} {
	    return 0
	} else {
	    return 1
	}
    }

    method rte {args} {
	if {[regexp {usb|high} [$self cget -driver]]} {
	    # Use console on USB
	    set ret [$self serialrelay rexec -t 10 -s \
			 $UTF::unittest/rteshell -$options(-console) $args]
	    regsub -line -all {^\d{6}\.\d{3} } $ret {} ret; # strip timestamps
	} else {
	    # Use dhd pass through everywhere else
	    try {
		set rtecapture ""
		$self dhd -i $options(-device) cons $args
		UTF::Sleep 1
	    } finally {
		set ret $rtecapture
		unset rtecapture
	    }
	}
	return $ret
    }

    method upload {{file dongle.dmp}} {
	if {![regexp {sdio|pcie} [$self cget -type]]} {
	    # We only know how to upload on SDIO and PCIe
	    return
	}
	if {[regexp {remap} [$self cget -type]]} {
	    # Remap dongles need remapping turned
	    # off before we can attempt an upload
	    catch {$self dhd -i $options(-device) socdevram_remap 0}
	}
	# Attempt to display message log.
	if {![catch {$self dhd -i $options(-device) consoledump} ret] &&
	    $file ne "dongle.dmp" && [regexp {ASSERT|TRAP} $ret]} {
	    # Analyse trap log.  In the "dongle.dmp" case this
	    # should already have been done by the caller.
	    regsub -all -line {^\d{6}.\d{3} } $ret {} ret
	    set trap [UTF::RTE::parse_traplog $self $ret]
	    # If there's already an assert, don't overwrite it
	    if {![info exists ::UTF::panic] ||
		![regexp -nocase {assert|trap|suspend_mac|microcode watchdog|malloced 0 bytes} $::UTF::panic] ||
		[regexp -nocase {Dongle assert file} $::UTF::panic]} {
		set ::UTF::panic $trap
	    }
	}
	# Make sure we don't get left with a stale file if the upload
	# fails.
	catch {$self rm -f $file}
	if {![catch {$self -t 60 dhd -i $options(-device) upload $file}] &&
	    [info exists ::UTF::Logdir]} {
	    if {[catch {
		set f [exec mktemp $UTF::Logdir/dongle.dmp_XXXXX]
		$self copyfrom $file ${f}.gz
		file attributes ${f}.gz -permissions go+r
		UTF::Message WARN $options(-name) \
		    "socram upload: [UTF::LogURL ${f}.gz]"
	    } ret]} {
		UTF::Message WARN $options(-name) $ret
	    }
	}
	if {$options(-uploadcmds) ne ""} {
	    $self rexec [string trim $options(-uploadcmds)]
	}
    }

    # Semaphore to prevent host-side error recovery if dongle-side
    # recovery is already underway.
    variable processingHostError false
    variable processingDongleError false
    variable processingIntrRecovery false
    variable rtecapture

    # Message handling callback.  Used by "base" to process log
    # messages.  Returns true if no further processing is required.
    method msgcallback {msg} {
	switch -re -- $msg {
	    sdstd_check_errs {
		$self worry $msg

# Commented out since upload is unlikely to work if the bus is already
# giving errors...
#		if {!$processingDongleError} {
#		    try {
#			set processingDongleError true
#			$self upload "dongle_sderr.dmp"
#		    } finally {
#			set processingDongleError false
#		    }
#		}
		return 1
	    }
	    {HW hdr error: |dhd: Unknown symbol} {
		# Only use the first of these.  Asserts get priority
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic $msg
		}
		UTF::Message FAIL $options(-name) $msg
		return 1
	    }
	    {dhd_bus_txctl: ctrl_frame_stat == TRUE txcnt_timeout=1} -
	    {dhd_msgbuf_wait_ioctl_cmplt: timeout > MAX_CNTL_TX_TIMEOUT} -
	    {sdstd_card_regread: Timeout on Buf_Read_Ready} -
	    {sdstd_card_buf: Error or timeout} -
	    {sdstd_abort: Error} -
	    {bcm_rpc_call_with_return: RPC call} -
	    {dhd_open: failed with code } -
	    {dhd_bus_start, dhd_bus_init failed -1} -
	    {dhd_bus_rxctl: resumed on timeout} -
	    {dhd_query_bus_erros: Resumed on timeout} {
		if {!$processingHostError && !$uploaddone} {
		    set uploaddone 1
		    try {
			UTF::Message FAIL $options(-name) $msg
			set processingHostError true
			# Give dongle console a chance to report problems
			# first, if any
			UTF::Sleep 2

			# Only use the first of these.  Asserts get priority
			if {![info exists ::UTF::panic]} {
			    set ::UTF::panic $msg
			}
			if {!$processingDongleError} {
			    try {
				set processingDongleError true
				# This may mean the dongle locked up.
				# Dump and reload.
				$self upload "dongle_dhd.dmp"
				catch {
				    $self reload
				}
			    } finally {
				set processingDongleError false
			    }
			}
		    } finally {
			set processingHostError false
		    }
		    return 1
		}
		return 0
	    }
	    {BUG: soft lockup - CPU#\d stuck} -
	    {Waiting for Data Inhibit cmd = 53} -
	    {check_client_intr: Not ready for intr:} {
		# Generally not recoverable - power cycle
		# Only use the first of these.  Asserts get priority
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic $msg
		}
		UTF::Message FAIL $options(-name) $msg
		if {!$processingIntrRecovery} {
		    set processingIntrRecovery true
		    catch {$self power cycle}
		    catch {$self power_sta cycle}
		    # ignore these messages for 30 seconds
		    after 30000 set processingIntrRecovery false
		    return 1
		}
	    }
	    sdstd_card_ {
		# These are harmless for Dongles
		UTF::Message LOG $options(-name) $msg
		return 1
	    }
	    CONSOLE_E: {
		# disabled due to delays: SWWLAN-116558
		UTF::Message LOG $options(-name) $msg
		return 1
	    }
	    {Dongle trap} -
	    {TRAP type } -
	    CONSOLE: {
		# if we have an RTE console drop DHD's copy since it
		# is less accurate and delayed, otherwise treat it as
		# if it came from the serial port
#		regsub {^CONSOLE_E: \[0x[[:xdigit:]]*\] } $msg {} msg
		regsub {^CONSOLE: } $msg {} msg
		if {[info exists rtecapture]} {
		    regsub {^\d{1,6}\.\d{3} } $msg {} m; # strip timestamps
		    append rtecapture "\n$m"
		}
		if {$options(-console) eq ""} {
		    $self consolemsg $msg
		}
		return 1
	    }
	}
	return 0
    }


    method hndrte_src {} {
	if {![info exists hndrte_src]} {
	    if {[catch {$self rexec -noinit -s -q \
			    ls -l hndrte-src.lnk} hndrte_src]} {
		set exe [$self hndrte_exe]
		if {![regsub {/dongle/rte/.*} $exe {} hndrte_src]} {
		    regsub {/build/dongle/.*} $exe {/src} hndrte_src
		}
	    } else {
		regsub {.*-> } $hndrte_src {} hndrte_src
	    }
	}
	set hndrte_src
    }

    method hndrte_exe {} {
	if {![info exists hndrte_exe]} {
	    set hndrte_exe [$self rexec -noinit -s -q ls -l hndrte-exe.lnk]
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
	# [call [arg host] [method parse_traplog] [arg log] [lb][arg exe][rb]]

	# Utility method for decoding dongle traps from an external
	# log file.  Normally traps are handled automatically during a
	# test.  An optional symbol table [arg exe] may be provided,
	# otherwise the rtecde.exe recorded with the last FW load will
	# be used.
    }
    method parse_traplog {file {exe {}}} {
	if {$exe ne ""} {
	    set hndrte_exe $exe
	}
	set f [open $file]
	set log [read $f]
	regsub -line -all {.*>\s+} $log {} log
	UTF::Message LOG "$options(-name)>" $log
	UTF::RTE::parse_traplog $self $log
	close $f
    }

    method trash_symbol {symbol} {
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

    method instant_panic {} {
	$self trash_symbol "wlc_scan"
	$self wl up
	catch {$self wl scan}
    }

    UTF::doc {
	# [call [arg host] [method reclaim]]

	# Returns reclaim bytes from latest dongle load reclaim
	# messages.  Dongle should have been loaded and up for at
	# least a second to give the background tools time to process
	# the reclaim messages from the serial port.
    }
    variable reclaim -array {}
    variable pre_reclaim -array {}

    method reclaim {} {
	set r 0
	foreach {s r} [array get reclaim] {
	    set r [expr {$r + $r}]
	}
	return $r
    }

    variable TRAPLOG

    method consolemsg {msg} {
	set needreload 0

	if {$msg eq ""} {
	    return
	}

	regsub {^\d{1,6}\.\d{3} } $msg {} msg; # strip timestamps

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
		  [regexp {TRAP |ASSERT.*|Trap type .*| Overflow |Dongle trap|PSM microcode watchdog fired|PSM WD!|\): pc .* lr } $msg]} {
	    set processingDongleError true
	    if {![info exists TRAPLOG]} {
		# No FWID, but we want to capture these anyway
		set TRAPLOG "$msg\n"
	    }
	    UTF::Message FAIL "$options(-name)>" $msg

	    if {$options(-assertrecovery) &&
		[regexp -nocase {assert|trap} $msg]} {
		set needreload 1
	    }

	    # Give DHD driver a moment to collect trap log
	    UTF::Sleep 0.5 quiet

	    package require UTF::RTE
	    set trap [UTF::RTE::parse_traplog $self $TRAPLOG]
	    unset TRAPLOG
	    if {$trap ne ""} {
		set msg $trap
	    }

	    # If there's already an assert, don't overwrite it
	    if {![info exists ::UTF::panic] ||
		![regexp -nocase {assert|trap|suspend_mac|microcode watchdog|: out of memory, malloced 0 bytes| reason |DEEPSLEEP} $::UTF::panic] ||
		[regexp -nocase {Dongle assert file} $::UTF::panic]} {
		set ::UTF::panic $msg
	    }

	    if {[regexp {TRAP |ASSERT.*|Trap } $msg]} {
		# Fetch core
		$self upload
	    }

	    if {$needreload} {
		catch {
		    # Try to reload driver
		    $self reload
		}
	    }
	    set processingDongleError false
	} elseif {[regexp {reclaim section (.*): Returned (\d+) bytes \(pre-reclaim: (\d+)\)} $msg - s r p]} {
	    set reclaim($s) $r
	    set pre_reclaim($s) $p
	    UTF::Message LOG "$options(-name)>" $msg
	} elseif {[regexp {reclaim(.*): Returned (\d+) } $msg - s r]} {
	    set reclaim($s) $r
	    UTF::Message LOG "$options(-name)>" $msg
	} elseif {![$base common_getdata $msg "$options(-name)>"]} {
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
	# Allow for one retry since Free data is sometimes missing
	if {([catch {$self rte mu} ret] ||
	     ![regexp {Free: (\d+)\(} $ret - free]) &&
	    ([catch {$self rte mu} ret] ||
	     ![regexp {Free: (\d+)\(} $ret - free])} {
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
	if {([catch {$self rte mu} ret] ||
	     ![regexp {Free: (\d+)\(.* In use: (\d+)\(.*, HWM: (\d+)\(} $ret - \
		   free inuse hwm]) &&
	    ([catch {$self rte mu} ret] ||
	     ![regexp {Free: (\d+)\(.* In use: (\d+)\(.*, HWM: (\d+)\(} $ret - \
		   free inuse hwm])} {
	    error "Mem stats not found"
	}
	list free $free inuse $inuse hwm $hwm
    }

    method pre_reclaim {} {
	array get pre_reclaim
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
	    $base open_messages
	    if {[set file $options(-console)] eq ""} {
		return
	    }
	}
	set id [$self cget -lan_ip]
	if {[info exists msgfile($id:$file)]} {
	    return
	}
	if {$file eq ""} {
	    # No serial console - using DHD pass-through.
	    return
	}
	if {[catch {$self serialrelay socket $file} ret]} {
	    $self worry "$file: $ret"
	    return
	}
	set msgfile($id:$file) $ret
	UTF::Message LOG $options(-name) "Open $file $ret"
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
	    $base close_messages
	    if {[set file $options(-console)] eq ""} {
		return
	    }
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

    # Peer passthroughs
    UTF::PassThroughMethod serialrelay -serialrelay
    UTF::PassThroughMethod rwlrelay -rwlrelay

}

# Retrieve manpage from last object
UTF::doc [UTF::DHD man]

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
