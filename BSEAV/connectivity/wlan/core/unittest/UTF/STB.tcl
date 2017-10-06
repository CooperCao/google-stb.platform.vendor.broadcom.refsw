#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::STB 2.0

package require snit
package require UTF::doc
package require UTF::Base

UTF::doc {
    # [manpage_begin UTF::STB n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF STB support}]
    # [copyright {2010 Broadcom Corporation}]
    # [require UTF::STB]
    # [description]
    # [para]

    # UTF::STB is an implementation of the UTF host object, specific
    # to Broadcom set top boxes, running STBlinux.

    # Once created, the STB object's methods are not normally invoked
    # directly by test scripts, instead the STB object is designated
    # the host for one or more platform independent STA objects and it
    # will be the STA objects which will be refered to in the test
    # scripts.

    # [list_begin definitions]

}

snit::type UTF::STB {

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -tftpserver
    option -image
    option -dhd_image
    option -dhd_tag
    option -dhd_brand
    option -dhd_date "%date%"
    option -sta
    option -name -configuremethod CopyOption
    option -device
    option -app_tag "trunk"
    option -app_date "%date%"
    option -app_brand ""
    option -tag "trunk"
    option -rtebrand ""
    option -date "%date%"
    option -dongleimage "43236a0-bmac/roml-ag-nodis-assert"
    option -type "debug{-native,}-apdef-stadef-high-media-mips"
    option -customer "bcm"
    option -clm_blob ""
    option -nvram
    option -preinstall_hook
    option -postinstall
    option -postinstall_hook
    option -preunload
    option -postuninstall_hook
    option -assertrecovery 1
    option -nvram_add
    option -wldot "wl"
    option -node ""
    option -wlinitcmds
    option -console -configuremethod _adddomain
    option -rteconsole -configuremethod _adddomain
    option -modopts
    option -maxsocram
    option -reloadoncrash -type snit::boolean -default false
    variable arch ""
    variable kernel ""

    variable reclaim -array {}
    variable pre_reclaim -array {}

    # base handles any other options and methods
    component base -inherit yes

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	install base using \
	    UTF::Base %AUTO% -ssh ush \
	    -noafterburner true \
	    -nocal true \
	    -init [mymethod init] \
	    -brand "linux-internal-wl-media"
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	foreach {sta dev} $options(-sta) {
	    if {$dev eq ""} {
		error "$sta has no device name"
	    } else {
		UTF::STA ::$sta -host $self -device $dev
	    }
	    if {$options(-device) eq ""} {
		# Need at least one device name for loading driver.
		# If not specified then use the first STA.  We only
		# support one physical device, but there may also be
		# virtual devices defined.
		set options(-device) $dev
	    }
	}
	if {[regexp {extnvm} $options(-type)]} {
	    lappend options(-nvram_add) "serialize"
	}
    }

    destructor {
	catch {$base destroy}
	foreach {sta dev} $options(-sta) {
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
	    {rtebrand.arg "$options(-rtebrand)" "brand"}
	    {tag.arg "$options(-tag)" "Build Tag"}
	    {type.arg "$options(-type)" "Host build Type"}
	    {customer.arg "$options(-customer)" "Customer"}
	    {dongleimage.arg "$options(-dongleimage)" "Dongle build Type"}
	    {date.arg "$options(-date)" "Build Date"}
	    {dhd_image.arg "[$self cget -dhd_image]" "DHD image"}
	    {dhd_tag.arg "[$self cget -dhd_tag]" "DHD Tag"}
	    {dhd_brand.arg "[$self cget -dhd_brand]" "DHD Brand"}
	    {dhd_date.arg "[$self cget -dhd_date]" "Build Date"}
	    {app_tag.arg "[$self cget -app_tag]" "Build Tag"}
	    {app_brand.arg "[$self cget -app_brand]" "Build Tag"}
	    {app_date.arg "[$self cget -app_date]" "Build Date"}
	}]

	set file [lindex $args end]

	# By default, dongle image comes from same brand as host
	# driver
	if {$(rtebrand) eq ""} {
	    set (rtebrand) $(brand)
	}
	if {$(dhd_tag) eq ""} {
	    if {[regexp -- {-bmac} [$self cget -dongleimage]]} {
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

	if {$(app_brand) eq ""} {
	    set (app_brand) $(brand)
	}

	# Set search tag to firmware tag by default.  Once object type
	# is determined below, this may switch to dhd_tag.
	set tag $(tag)
	set brand $(brand)
	set date $(date)

	if {$file eq ""} {
	    set file $(dongleimage)
	} elseif {[file isdirectory $file]} {
	    # Look for a driver, assuming this is part of a developer
	    # workspace.
	    set type $(dongleimage)
	    if {$type eq "wl.ko"} {
		return [glob "$file{{{/wl,}/linux,}/obj-$(type)-[$self kernel],}/$type"]
	    } else {
		if {![regexp {([^.]*)\.(.*)} $type - typedir typesuffix]} {
		    set typedir $type
		    set typesuffix "bin"
		}
		regsub {/rtecdc$} $typedir {} typedir
		return [glob "$file{{/dongle/rte/wl/builds,/../build/dongle,}/$typedir,}/rtecdc.$typesuffix"]
	    }

	} elseif {$(dhd_image) != "" && [file extension $file] eq ".ko"} {
	    if {[file isfile $(dhd_image)]} {
		# Use specific .ko file user specified
		return $(dhd_image)
	    } elseif {[regexp {dhd} $(type)]} {
		# Use developer build for DHD driver
		return [glob "$(dhd_image){{{/dhd,}/linux,}/$(type)-[$self kernel],}/$file"]
	    } else {
		# Use developer build for BMAC High driver - either
		# src, release, or flat folder will do.
		return [glob "$(dhd_image){{{/wl,}/linux,}/obj-$(type)-[$self kernel],}/$file"]
	    }
	} elseif {[BuildFile::exists $file]} {
	    return $file
	}

	if {$file eq "wl" || $file eq "bwl" || $file eq "bcmdl" ||
	    $file eq "dhd"} {
	    set tag $(app_tag)
	    set date $(app_date)
	    set brand $(app_brand)
	    set apparch [$self arch]
	    if {![regexp {stbsoc|ternal-media} $brand]} {
		set tail [file join release exe $apparch $file]
	    } else {
		set tail [file join release $(customer) apps $apparch $file]
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
		    {{main/,}{src/shared,components}/nvram} file
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
	} elseif {[lsearch {.clmb .clm_blob} \
		       [file extension $file]] >= 0} {
	    if {[regexp {/rtecdc\.(bin|trx|romlsim\.trx)$} $(type)]} {
		# Firmware hasn't been copied - pull directly from firmware build
		set brand hndrte-dongle-wl
		set dir [file dirname $(type)]
		set tail "{build/dongle,src/dongle/rte/wl/builds}/$dir/$file"
	    } else {
		set tail [file join release $customer firmware $file]
	    }
	} elseif {[regexp {/rtecdc\.(bin|trx|romlsim\.trx)$} $file]} {
	    # Firmware hasn't been copied - pull directly from firmware build
	    set brand hndrte-dongle-wl
	    set tail "{build/dongle,src/dongle/rte/wl/builds}/$file"
	} elseif {[regexp {\.(trx|bin)$} $file]} {
	    if {[regexp {^hndrte} $(rtebrand)]} {
		# dongle-type build
		regsub {(?:\.romlsim)?\.(trx|bin|bin\.trx)$} $(dongleimage) {} type
		set tail "{build/dongle,src/dongle/rte/wl/builds}/$type/$file"
	    } else {
		# host-type build
		if {![regexp {stbsoc|ternal-media} $brand] &&
		    [regexp -- {-high} $options(-type)]} {
		    set tail [file join release firmware $file]
		} else {
		    set tail [file join release $(customer) firmware $file]
		}
	    }
	    set brand $(rtebrand)
	} elseif {[regexp {\.(?:exe|opt)$} $file]} {
	    set tag "{TEMP/prebuild/$brand/,}$tag"
	    set brand hndrte-dongle-wl
	    if {![regsub {/rtecdc\..*} $(dongleimage) {} type]} {
		regsub {(?:\.romlsim)?\.(trx|bin|bin\.trx)$} $(dongleimage) {} type
		puts $(dongleimage)
	    }
	    set tail "{build/dongle,src/dongle/rte/wl/builds}/$type/$file"
	    unset type
	} else {
	    set tag $(dhd_tag)
	    set brand $(dhd_brand)
	    set date $(dhd_date)
	    if {[regexp {dhd} $(type)]} {
		set drv "$(customer)/host/dhd_driver/$(type)"
	    } elseif {[regexp {stbsoc|ternal-media} $brand]} {
		set drv "$(customer)/host/wl_driver/obj-$(type)"
	    } else {
		set drv "obj-$(type)"
	    }
	    set kernel [$self kernel]
	    # Temporary Hack for 3.14.13-1.2
	    regsub -- {(-1\.2pre)$} $kernel {{,-1.2{,pre}}} kernel
	    set tail [file join release ${drv}-$kernel $file]
	}

	if {[regexp {_REL_} $tag]} {
	    set tag "{PRESERVED/,ARCHIVED/,}$tag"
	} else {
	    set tag "{PRESERVED/,}$tag"
	}

	set pattern [file join \
			 $::UTF::projswbuild/build_linux \
			 $tag $brand $date* "$tail{,.gz}"]

	if {$(showpattern)} {
	    UTF::Message INFO $options(-name) $pattern
	}
	UTF::SortImages [list $pattern] \
	    {*}[expr {$(ls)?"-ls":""}] \
	    {*}[expr {$(all)?"-all":""}]
    }

    UTF::doc {
	# [call [arg host] [method reload]

	# Reload the driver using files already copied to the host.
    }

    variable reloadlock 0
    method reload {} {

	set waitfortrap 0
	try {
	    # Avoid looping if the driver asserts immediately
	    if {[incr reloadlock] != 1} {
		return
	    }

	    set modopts [$self cget -modopts]
	    set bin "bin"
	    if {$options(-dongleimage) eq "wl.ko"} {
		set dhdexe ""
		set f wl.ko
		set bin ""
	    } elseif {[regexp -- {-high} $options(-type)]} {
		set dhdexe bcmdl
		set f wl.ko
		set bin "trx"
	    } elseif {[regexp -- {-usb} $options(-type)]} {
		set dhdexe bcmdl
		set f dhd.ko
		set bin "bin.trx"
	    } else {
		set dhdexe dhd
		set f dhd.ko
		lappend modopts "firmware_path=rtecdc.$bin"
		if {[$self cget -nvram] ne "" || [$self cget -nvram_add] ne ""} {
		    lappend modopts "nvram_path=nvram.txt"
		}
		if {[$self cget -clm_blob] ne ""} {
		    lappend modopts "clm_path=rtecdc.clmb"
		}
	    }
#	    $self clean_udev_rules
	    set waitfortrap 1

	    $self unload

	    # Preinstall hook
	    UTF::forall {STA dev} [$self cget -sta] \
		cmd [$self cget -preinstall_hook] {
		    set c [string map [list %S $STA] $cmd]
		    if {[catch [string map [list %S $STA] $cmd] ret]} {
			UTF::Message WARN $STA $ret
		    }
		}
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

		if {[catch {$self -t 60 {*}$dlcmd} dlret]} {
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
	    #foreach m [split [$self modinfo -F depends $f] ","] {
	#	if {$m eq "bcm_dbus"} {
	#	    $self insmod bcm_dbus.ko
	#	} else {
	#	    $self modprobe $m
	#	}
	 #   }

	    array set reclaim {}
	    array set pre_reclaim {}
	    $self rexec insmod $f {*}$modopts

	    if {$options(-postinstall) ne ""} {
		$self rexec $options(-postinstall)
	    }
	    UTF::forall {STA dev} [$self cget -sta] \
		cmd [$self cget -postinstall_hook] {
		    if {[catch [string map [list %S $STA] $cmd] ret]} {
			UTF::Message WARN $STA $ret
		    }
		}

	    if {0 && $dhdexe ne "" && $dhdexe ne "bcmdl"} {
		if {$options(-maxsocram) ne ""} {
		    if {[regexp {^(\d+)k$} $options(-maxsocram) - m]} {
			set m [expr {1024*$m}]
		    } else {
			set m $options(-maxsocram)
		    }
		    $self dhd -i $options(-device) maxsocram $m
		}
		if {[$self cget -nvram] ne "" ||
		    [$self cget -nvram_add] ne ""} {
		    $self dhd -i $options(-device) download \
			rtecdc.$bin nvram.txt
		} else {
		    $self dhd -i $options(-device) download \
			rtecdc.$bin
		}
	    }
	    # forgive transient ifconfig failure (partial WAR for SWWLAN-46111)
	    $self rexec -x ifconfig $options(-device) up

#	    if {$options(-rwlrelay) ne ""} {
#		catch {$self pkill -f wl_server_socket}
#		UTF::Sleep 1
#		$self rexec \
#		    "/usr/bin/wl_server_socket lo0 $options(-rwlport) >/dev/null 2>&1&"
#	    }

#	    if {[$self cget -clm_blob] ne ""} {
#		if {[catch {$self wl clmload 0 rtecdc.clmb} ret opt]} {
#		    set status \
#			[UTF::clmload_statuserr [$self wl -silent clmload_status]]

#		    UTF::Message FAIL $options(-name) $status
#		    throw FAIL $status
#		}
#	    }

	    if {[set wlinitcmds [string trim [$self cget -wlinitcmds]]] ne ""} {
		$self rexec $wlinitcmds
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

    method _reload {} {

	# reset tuned flag.
	set tuned ""

	catch {$self unload}

	# Invalidate tuning cache
	set tuned ""
	if {[regexp -- {dhd} $options(-type)]} {
	    $self rexec insmod dhd.ko
	    $self dhd -i $options(-device) download rtecdc.bin
	} else {
	    # Keep the next cmd under 68 chars else console will wrap
	    $self rexec {if [ -s bcm_dbus.ko ];then insmod bcm_dbus.ko;fi}
	    if {[catch {$self insmod wl.ko} ret]} {
		# Give kernel time to report why
		UTF::Sleep 1
		error $ret $::errorInfo
	    }
	    if {[regexp {high|usb} [$self cget -type]]} {
		set bcmdl bcmdl
		if {$options(-node) ne ""} {
		    lappend bcmdl -i $options(-node)
		}
		if {$options(-nvram) ne ""} {
		    lappend bcmdl -n nvram.txt
		}
		$self {*}$bcmdl rtecdc.trx
		UTF::Sleep 2
	    }
	}

	if {$options(-wlinitcmds) ne ""} {
	    $self rexec [string trim $options(-wlinitcmds)]
	}

	set ret [$self wl ver]
	regexp {version (.*)} $ret - ret
	return $ret
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
    variable stuckreading 0

    # Save src and exe references, in case host is down when we want
    # to analyse a crash
    variable hndrte_src
    variable hndrte_exe

    method load {args} {
	UTF::GetKnownopts [subst {
	    {nvram.arg "$options(-nvram)" "Nvram.txt"}
	    {n "Just copy the files, don't actually load the driver"}
	    {all "ignored"}
	    {ls "ignored"}
	}]

	UTF::Message INFO $options(-name) "Load STB Driver"
	if {[regexp -- {dhd} $options(-type)]} {
	    set driver "dhd.ko"
	    if {[regexp -- {usb} $options(-type)]} {
		set bcmdl "bcmdl"
	    } else {
		set bcmdl "dhd"
	    }
	} else {
	    set driver $options(-wldot).ko
	    set bcmdl "bcmdl"
	}
	set f $driver

	# Remove driver - this way if load fails due to missing
	# components, we won't accidentally reload an old driver
	# later.
	$self rm -f $f

	set image [$self findimages {*}$args]
	UTF::Message LOG $options(-name) "Found $image"
	if {$(nvram) ne ""} {
	    set nvram [$self findimages {*}$args $(nvram)]
	    UTF::Message LOG $options(-name) "NVRAM $nvram"
	} elseif {$options(-nvram_add) ne ""} {
	    # Allow nvram additions, even if there's no base nvram
	    set nvram "/dev/null"
	}

	if {[$self cget -clm_blob] ne ""} {
	    set blob [$self cget -clm_blob]
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

	if {$options(-dongleimage) ne "wl.ko"} {
	    set driver [$self findimages {*}$args $driver]
	    UTF::Message LOG $options(-name) "Host Driver $driver"

	    # Note: run modinfo on the controller, since appliance may not
	    # have it.  No need to uncompress since modinfo can handle
	    # compressed files.
	    set shim [UTF::BuildFile::modinfo -F depends $driver]
	    if {$shim ne ""} {
		set shim [$self findimages {*}$args ${shim}.ko]
		UTF::Message LOG $options(-name) "Host shim $shim"
	    }
	}

	set wl [$self findimages {*}$args "wl"]
	UTF::Message LOG $options(-name) "wl $wl"

	if {[regexp {high|usb|dhd} [$self cget -type]]} {
	    set bcmdl [$self findimages {*}$args $bcmdl]
	    UTF::Message LOG $options(-name) "bcmdl $bcmdl"
	} else {
	    set bcmdl ""
	}

	catch {$self -n rm -f hndrte-src.lnk}
	catch {$self -n rm -f hndrte-exe.lnk}
	if {$options(-dongleimage) ne "wl.ko"} {
	    # Find mogrified sources in hndrte
	    set m "<none>"
	    if {[regsub {([^/]*)/(linux-.*)/(.*)\.\d+/release/.*} $image \
		     {{TEMP/prebuild/\2/,}\1/hndrte-dongle-wl/\3.*/src} m]} {
	    } else {
		# Perhaps it's a private build?
		regsub {src/dongle/.*} $m {src} m
	    }

	    if {![catch {glob -type d $m} ret]} {
		set m [lindex [lsort $ret] end]
		UTF::Message LOG $options(-name) "Mogrified src: $m"
		$self rexec -n "echo '$m' >hndrte-src.lnk"
		set hndrte_src $m
	    } else {
		UTF::Message LOG $options(-name) "Mogrified src: $ret"
	    }
	    set exe [file join [file dirname $image] rtecdc.exe]
	    if {![file exists $exe]} {
		set exe rtecdc.exe
	    }
	    if {[catch {$self findimages {*}$args $exe} ret]} {
		UTF::Message LOG $options(-name) "Symbol file not found: $ret"
	    } else {
		UTF::Message LOG $options(-name) "Symbol file: $ret"
		$self rexec -n "echo '$ret' >hndrte-exe.lnk"
		set hndrte_exe $ret
	    }
	}

	# Stage files on the tftp server to be copied on reload
	if {$options(-dongleimage) ne "wl.ko"} {
	    if {[regexp -- {dhd} $options(-type)]} {
		UTF::BuildFile::copyto $self $driver dhd.ko
		if {[regexp -- {usb} $options(-type)]} {
		   UTF::BuildFile::copyto $self $image rtecdc.bin.trx
		    if {$bcmdl ne ""} {
			UTF::BuildFile::copyto $self $bcmdl bcmdl
		    }
		} else {
		    UTF::BuildFile::copyto $self $image rtecdc.bin
		    if {$bcmdl ne ""} {
			UTF::BuildFile::copyto $self $bcmdl dhd
		    }
		}
	    } else {
		UTF::BuildFile::copyto	$self $driver wl.ko
		UTF::BuildFile::copyto	$self $image rtecdc.trx
		if {[info exists blob]} {
		    UTF::BuildFile::copyto $self $blob rtecdc.clmb
		}
		if {$bcmdl ne ""} {
		   UTF::BuildFile::copyto $self $bcmdl bcmdl
		}
	    }
	    $self copyto $shim bcm_dbus.ko
	} else {
	   UTF::BuildFile::copyto $self $image wl.ko
	}

 	UTF::BuildFile::copyto $self $wl wl
	if {[info exists nvram]} {
	    set nvram [UTF::nvram_add $nvram $options(-nvram_add)]
	    UTF::BuildFile::copyto $self $nvram nvram.txt
	    if {[regexp {^/tmp/nvram\.txt_} $nvram]} {
		UTF::Message LOG "" "file delete $nvram"
		file delete $nvram
	    }
	} else {
	    $self rm -f nvram.txt
	}
	$self sync

	if {$(n)} {
	    UTF::Message LOG $options(-name) "Copied files - not loading"
	    return
	}

	$self reload

    }

    method stage {src name} {
	if {$src eq ""} {
	    $self tftpserver rexec -n ":>/var/lib/tftpboot/$name"
	} else {
	    $self tftpserver copyto $src /var/lib/tftpboot/$name
	}
    }

    UTF::doc {
	# [call [arg host] [method copyto] [arg src] [arg dest]]

	# Copy local file [arg src] to [arg dest] on [arg host].  Auto
	# uncompression is supported.
    }
    method copyto {src dst} {
	if {$src eq ""} {
	    set src /dev/null
	}
	if {[file extension $src] eq ".gz" &&
	    [file extension $dst] ne ".gz"} {
	    set cmd "zcat"
	} else {
	    set cmd "cat"
	}
	append cmd ">$dst"
	if {[file executable $src]} {
	    append cmd ";chmod +x $dst"
	}
	$self rexec -n -t 120 $cmd <$src
    }

    UTF::doc {
	# [call [arg host] [method copyfrom] [arg src] [arg dest]]

	# Copy file [arg src] on [arg host] to local [arg dest].  Auto
	# compression is not supported.
    }
    method copyfrom {src dst} {
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
	# [call [arg host] [method tftpto] [arg src] [arg dest]]

	# Copy local file [arg src] to [arg dest] on [arg host] using
	# tftp.  Auto uncompression is not supported.
    }
    method tftpto {src dst} {
	set name [file tail $src]
	if {![catch {lindex [$self -n md5sum $dst] 0} dstsum]} {
	    if {[string match "*/*" $src]} {
		set md5sum [lindex [exec md5sum $src] 0]
	    } else {
		set md5sum [lindex [$self tftpserver \
					-n md5sum /var/lib/tftpboot/$name] 0]
	    }
	    if {$dstsum eq $md5sum} {
		UTF::Message LOG $options(-name) "$dst ok"
		return
	    }
	}
	if {[string match "*/*" $src]} {
	    $self stage $src $name
	}

	if {[$self cget -relay] eq ""} {
	    if {[set ip [$self tftpserver cget -lan_ip]] eq ""} {
		set ip [$self tftpserver cget -name]
	    }
	    set ip [UTF::resolve $ip]
	} else {
	    set ip [[$self tftpserver] ipaddr]
	}
	$self -t 60 tftp -g -r $name -l $dst $ip
    }

    UTF::doc {
	# [call [arg host] [method unload]]

	# Unload the driver and reset the dongle
    }

    variable reloadlock 0
    method unload {} {


	try {
	    # Avoid reloading if we crash during unload
	    incr reloadlock

#	    if {![catch {$self killall wpa_supplicant}]} {
#		UTF::Sleep 2
#	    }
	    # Preunload
	    if {$options(-preunload) ne ""} {
		$self rexec $options(-preunload)
	    }

	    # Unload host driver and shim
	    if {[catch {$self rexec {lsmod|awk '/^(dhd|wl|bcm_dbus) /{print $1}'}} ret]} {
		lappend errs $ret
	    } else {
		foreach module $ret {
		    if {[catch {$self rexec rmmod $module} ret]} {
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
	    if {[regexp -- {-(:?usb|high)} $options(-type)]} {
		# Give USB dongle chance to reset
		UTF::Sleep 1
	    }
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
	if {0} {
	# Reboot the dongle via wl, rte and power cycle if necessary.
	try {
	    # Avoid reloading if we crash during unload
	    incr reloadlock
	    if {$options(-dongleimage) eq "wl.ko"} {
		catch {$self wl down}
	    } else {
		if {[catch {
		    $self wl down
		    $self wl bmac_reboot
		} ret]} {
		    if {[catch {$self rte reboot} ret]} {
			if {[$self cget -power_sta] ne ""} {
			    UTF::Message WARN $options(-name) \
				"Dongle not responding to UART.  Power cycling"
			    $self power_sta cycle

			    if {[$self cget -power_sta] eq [$self cget -power]} {
				# power_sta was the same as
				# power_host, so we'd better wait for
				# the host to recover.

				UTF::Sleep 20
				$self configure -initialized 0
				for {set i 0} {[catch {$self :}] && $i < 20} {incr i} {}
			    } else {
				# The Dongle should only take a few seconds to recover
				UTF::Sleep 2
			    }
			} else {
			    UTF::Message WARN $options(-name) \
				"Dongle not responding to UART."
			}
		    }
		}
		UTF::Sleep 2
	    }
	    # Unload host driver and shim
	    foreach module [$self rexec -n {lsmod|awk '/^(wl|dhd|bcm_usbshim|bcm_dbus) /{print $1}'}] {
		if {[catch {$self rexec rmmod $module} ret]} {
		    lappend errs $ret
		}
	    }
	    if {[info exists errs]} {
		if {[regexp {Timeout|child killed|resource busy} $errs]} {
		    $self power cycle
		    $self wait_for_boot 60
		    error "rmmod timeout: try power cycle"
		} else {
		    error $errs
		}
	    }
	} finally {
	    incr reloadlock -1
	}
    }
    }

    UTF::doc {
	# [call [arg host] [method ifconfig] [arg dev] [option dhcp]]

	# Enable DHCP on device [arg dev].

	# [call [arg host] [method ifconfig] [arg {args ...}]]

	# Run ifconfig on the host, disabling DHCP if necessary.
    }

    # IP address cache
    variable ipaddr -array {}
    variable dhcpver ""

    method ifconfig {dev args} {
	if {[llength $args]} {
	    # invalidate cache
	    if {[info exists ipaddr($dev)]} {
		unset ipaddr($dev)
	    }

	    # Need to use kill -9 otherwise dhcpcd will bring down
	    # the interface and disassociate
	    set PF "/var/run/dhcpcd-${dev}.pid"
	    catch {$self rexec -n "test -f $PF && kill -9 `cat $PF`; rm $PF"}
	}
	if {$args eq "dhcp"} {
	    # Start up dhcpcd without updating any system files.
	    $self udhcpc -i $dev -n -q -p $PF
	} else {
	    $self rexec ifconfig $dev $args
	}
    }

    UTF::doc {
	# [call [arg host] [method macaddr] [arg dev]]

	# Returns the MAC address for [arg dev]
    }
    method macaddr {dev} {
	if {[regexp {HWaddr\s+(\S+)} [$self rexec -n ifconfig $dev] - mac]} {
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
	} else {
	    error "$options(-name) No IP address available"
	}
    }

    UTF::doc {
	# [call [arg host] [method ping] [arg target]
	#	[lb][opt -c] [arg count][rb]
	#       [lb][opt -s] [arg size][rb]]

	# Ping from [arg host] to [arg target] with packet size [arg
	# size].  Target may be a STA or a Hostname/IP address.
	# Returns success if a response packet is received before [arg
	# count] (default 5) packets have been sent and error if not.
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

	# ping -W makes negative tests faster, but isn't supported on
	# some versions so impose a timeout instead
	set ping [list $self rexec -n -t 1 ping -c 1 -s $(s) $target]

	# Loop, since Router ping doesn't have a short-circuit "count"
	for {set c 0} {$c < $(c)} {incr c} {
	    catch $ping ret
	    if {[regexp {(\d+) packets received} $ret - r] && $r > 0} {
		return
	    }
	}
	error "ping failed"
    }

    typevariable msgfile
    variable oops ""
    variable rtecapture
    variable interruptedbydongle ""
    variable processingHostError false

    # Internal callback for fileevent below
    method getdata {request} {
	set fd $msgfile($request)
	if {![eof $fd]} {
	    set msg [gets $fd]

	    if {[regexp {root login on} $msg]} {
		# no need to log all these login messages
		return
	    }

	    # Lose timestamp, since we're adding our own.
	    regsub {^\[\s*(\d+)\.(\d+)\] } $msg {} msg

	    if {[regexp {TRAP type } $msg]} {
		$self consolemsg $msg
		return
	    }
	    # Pass CONSOLE messages on to rte handler CONSOLE messages
	    # frequently interrupt host messages, so collect partial
	    # messages and report them after the rte messages have
	    # been handled.
	    if {[regexp {(.*)CONSOLE: (.*)} $msg - pre post]} {
		append interruptedbydongle $pre
		if {[info exists rtecapture]} {
		    append rtecapture "\n$post"
		}
		$self consolemsg $post
		return
	    } elseif {$interruptedbydongle ne ""} {
		UTF::Message LOG $options(-name) $interruptedbydongle
		set interruptedbydongle ""
	    }
	    # NIC mode reload
	    if {$options(-dongleimage) eq "wl.ko" &&
		!$processingHostError && $options(-reloadoncrash) &&
		[regexp {Rebooting in \d+ seconds\.\.} $msg]} {
		UTF::Message WARN $options(-name) $msg
		# ignore these messages while waiting
		set processingHostError true
		$self wait_for_boot
		set processingHostError false
		# Clear IP addr cache
		array unset ipaddr
		$self reload
	    } elseif {[regexp {pciedev_shared invalid} $msg]} {
		if {!$processingHostError} {
		    try {
			set processingHostError true
			# Give dongle console a chance to report problems
			# first, if any
			UTF::Sleep 2

			# Only use the first of these.  Asserts get priority
			if {![info exists ::UTF::panic]} {
			    set ::UTF::panic $msg
			}
			UTF::Message FAIL $options(-name) $msg
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
			# DHD may retry the load
			after 10000 [list set [myvar processingHostError] false]
		    }
		}
		return 1
	    }
	    if {[regexp {Kernel panic:.*|assert|Bad trx header|insmod: unresolved symbol |RPC call .*return wait err|Out of Memory: Killed process} $msg] ||
		      ($oops ne "" && [regexp {Code: |rebooting} $msg])} {
		UTF::Message FAIL $options(-name) $msg
		# If there's already an assert, don't overwrite it
		if {![info exists ::UTF::panic] ||
		    ![regexp -nocase \
			  {assert|trap|Oops|Memory leak|RPC call .*return wait err} \
			  $::UTF::panic]} {
		    set ::UTF::panic $msg
		}
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
		$self rexec -n ps
	    } elseif {[regexp {oom-killer: } $msg]} {
		UTF::Message FAIL $options(-name) $msg
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic $msg
		}
		# Only known way to recover from this is to reboot and
		# reload.  Shouldn't need to semaphore this.
		catch {
		    $self -n "reboot;:"
		    UTF::Sleep 25
		    $self reload
		}
	    } elseif {![$base common_getdata $msg]} {
		UTF::Message LOG $options(-name) $msg
		if {[regexp {Oops in |Unhandled kernel|traps\.c} $msg]} {
		    set oops "$msg\n"
		} elseif {$oops ne "" &&
			  [regexp {module:\s+|epc\s+|\[<([\da-f]+)>\]} $msg]} {
		    append oops "$msg\n"
		}
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
	if {$src eq ""} {
	    regsub {/} $options(-name) {.} src
	    set src [$self relay -noinit -s -q cat "$src-src"]
	}
	if {![file isdirectory $src]} {
	    error "Source code not found"
	}
	set linux "$src/linux/linux"
	foreach msg [split $oopscpy "\n"] {
	    if {[regexp {\mOops in |Unhandled kernel|traps\.c} $msg]} {
		set oopscmd "set width 0\n"
		append oopscmd "file $linux/vmlinux_dbgsym\n"
	    } elseif {[regexp {\mmodule:\s+(\S+)\s+([0-9a-f]+)} $msg - m a]} {
		set sim "$m/${m}_dbgsym.o"
		if {$m eq "wl_high"} {
		    set sim "wl/$sim"
		}
		append oopscmd \
		    "add-symbol-file $linux/drivers/net/$sim 0x$a+0x60\n"
	    } elseif {[regexp {\mepc\s+:\s+([0-9a-f]+)\s+Not tainted} \
			   $msg - epc]} {
		append oopscmd "l *0x$epc\n"
	    } elseif {[regexp {\[<([\da-f]+)>\]} $msg]} {
		foreach t [regexp -all -inline {[\da-f]{8}} $msg] {
		    append oopscmd "info line *0x$t\n"
		}
	    }
	}
	set f [open "/tmp/oops.cmd" w]
	catch {file attributes "/tmp/oops.cmd" -permissions 00666}
	puts $f $oopscmd
	UTF::Message LOG $options(-name) $oopscmd
	close $f
	set cmd $::UTF::projtools/linux/bin/mipsel-linux-gdb
	append cmd {-q -n < /tmp/oops.cmd | sed s/\".*\\/src/\"src/}
	set oops ""
	UTF::Message LOG $options(-name) $cmd
	catch {exec {*}$cmd} ret
	regsub -line -all \
	    {.*add symbol table.*\n|.*\.text_addr =.*\n} $ret {} ret
	UTF::Message LOG $options(-name) $ret

	# Replace Code error with function and line, if found
	if {(![info exists ::UTF::panic] ||
	     [regexp -nocase {Code|Kernel panic} $::UTF::panic]) &&
	    [regexp -line {0x[[:xdigit:]]+ is in (.*\))} $ret - fandl]} {
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
	}
	set id [$self cget -lan_ip]
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

    method {route add} {net mask gw} {
	$self rexec -n route add -net $net netmask $mask gw $gw
    }

    method {route del} {net mask gw} {
	$self rexec -n route del -net $net netmask $mask gw $gw
    }

    method {route replace} {net mask gw} {
	catch {$self route del $net $mask $gw}
	$self route add $net $mask $gw
    }

    method init {} {
	$self open_messages
	if {$options(-rteconsole) ne ""} {
	    $self open_rte_messages
	}
	# Add to setup
	#catch {$self -t 5 dmesg -n 7}
    }

    method deinit {} {
	if {$options(-rteconsole) ne ""} {
	    $self close_rte_messages
	}
	$self close_messages
	$self configure -initialized 0
    }

    UTF::doc {
	# [call [arg host] [method boardname]]

	# Query device boardtype
    }

    method boardname {} {
	set ret [regexp -inline {\w+} [$self uname -m]]
	if {$ret eq "mips"} {
	    set ret [$self -n "awk '/system type/{print \$4}' /proc/cpuinfo"]
	} else {
	    set ret [$self -n cat /proc/device-tree/bolt/board]
	    regsub {\0} $ret {} ret
	}
	regsub {BCM9?} $ret {} ret
	set ret
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

    method setup {} {
	$self setup_iperf
	$self copyto $UTF::unittest/etc/stb.rc.user rc.user
	$self reboot
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
	expr {[catch {$self md5sum $remote} ret] ||
	      [lindex $ret 0] ne [lindex [exec md5sum $local] 0]}
    }

    variable setup_iperf 0
    method setup_iperf {} {
	set achive $::UTF::projarchives/unix/UTF
	if {$setup_iperf} {
	    return
	}
	set setup_iperf 1
	set s "$archive/iperf_[$self arch]_[$self kernel]"
	if {[$self sumcheck $s iperf]} {
	    $self -n -x killall -q iperf
	    $self copyto $s iperf
	}
    }



# 7445:

# Boot (from anything)
# Copy vmlinuz-7445d0 and vmlinuz-initrd-7445d0 to tftpserver
# # stbutil utftesto:/  and select option "7"
# mount
# Copy to sysinit.txt
#  boot usbdisk0:vmlinuz-7445d0 'rootwait root=/dev/sdb rw'

# setenv -p STARTUP "waitusb;boot usbdisk0:vmlinuz-7445d0 'rootwait root=/dev/sda1 rw'"

# tftp -g -r ushd_astb -l /root/ushd utftesto

# For static IP, use /etc/config/ifup.gphy
# iface=$1
# ifconfig $iface 10.19.86.35 netmask 255.255.252.0 up
# route add default gw 10.19.84.1

# example /root/rc.user
#
#   #! /bin/sh
#   killall -q ushd ntpd
#   ntpd -d -p 10.10.32.12
#   echo "## USHD ##"
#   cd /root && ushd -d
#   dmesg -n 7
#   # Prevent TCP slow-start
#   echo reno > /proc/sys/net/ipv4/tcp_congestion_control
#   echo 1 > /proc/sys/kernel/panic
#   echo 1 > /proc/sys/kernel/panic_on_oops

# 7346:
# example /mnt/usb/stbstartup.sh
#
#   cat stbstartup.sh
#   echo "***** Startup Script *********"
#   killall -q udhcpc ushd ntpd
#   sleep 1
#   ifconfig eth0 10.19.7.39 netmask 255.255.255.0
#   route add default gw 10.19.7.1
#   ntpd -d -p 10.10.32.12
#   # Prevent TCP slow-start
#   echo reno > /proc/sys/net/ipv4/tcp_congestion_control
#   cd /mnt/usb && ./ushd -d



    UTF::doc {
	# [call [arg host] [method tcptune] [arg window]]

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
		set tuned [$self cat /proc/sys/net/core/rmem_max]
	    }
	    set window [UTF::kexpand $window]
	    # Truncate any decimal component, but don't cast to int
	    # since it may not fit.
	    regsub {\..*} $window {} window
	    if {$tuned < $window} {
		$self rexec -n "echo $window >/proc/sys/net/core/rmem_max;\
                                echo $window >/proc/sys/net/core/wmem_max"
		set tuned $window
	    }
	    return 1
	}

	if {$window == 0} {
	    set window 131071
	    set tcp_rmem "4096\t87380\t2097152"
	    set tcp_wmem "4096\t16384\t2097152"
	} else {
	    set window [UTF::kexpand $window]
	    if {$window < 1024} {
		error "Impausibly small window $window bytes"
	    }
	    # Truncate any decimal component, but don't cast to int
	    # since it may not fit.
	    regsub {\..*} $window {} window
	    set tcp_rmem "4096\t$window\t$window"
	    set tcp_wmem "4096\t$window\t$window"
	}

	if {$tuned ne "$window\n$window\n$tcp_rmem\n$tcp_wmem"} {
	    $self rexec -n "echo $window >/proc/sys/net/core/rmem_max;\
                         echo $window >/proc/sys/net/core/wmem_max;\
                         echo $tcp_rmem >/proc/sys/net/ipv4/tcp_rmem;\
                         echo $tcp_wmem >\/proc/sys/net/ipv4/tcp_wmem"
	    if {![catch {$self killall -2 iperf}]} {
		# Try again, in case there are multiple threads
		catch {$self killall -2 iperf}
	    }
	    $self rexec -n "iperf -s >/dev/console 2>&1 &"
	    set tuned "$window\n$window\n$tcp_rmem\n$tcp_wmem"
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

	if {[info exists UTF::NoTCPTuning]} {
	    return [$self tcptune $window]
	}

	if {$window == 0} {
	    set window 131071
	    set tcp_rmem "4096\t87380\t2097152"
	    set tcp_wmem "4096\t16384\t2097152"
	} else {
	    set window [UTF::kexpand $window]
	    if {$window < 1024} {
		error "Impausibly small window $window bytes"
	    }
	    # Truncate any decimal component, but don't cast to int
	    # since it may not fit.
	    regsub {\..*} $window {} window
	    set tcp_rmem "4096\t$window\t$window"
	    set tcp_wmem "4096\t$window\t$window"
	}
	if {$tuned ne "$window\n$window\n$tcp_rmem\n$tcp_wmem"} {
	    $self rexec -n "echo $window >/proc/sys/net/core/rmem_max;\
                         echo $window >/proc/sys/net/core/wmem_max;\
                         echo $tcp_rmem >/proc/sys/net/ipv4/tcp_rmem;\
                         echo $tcp_wmem >\/proc/sys/net/ipv4/tcp_wmem"
	    set tuned "$window\n$window\n$tcp_rmem\n$tcp_wmem"
	}
	return 1
    }

    # Dongle methods

    method hndrte_src {} {
	if {![info exists hndrte_src]} {
	    if {[catch {$self rexec -n -s -q \
			    cat hndrte-src.lnk} hndrte_src]} {
		regsub {/dongle/rte/.*} [$self hndrte_exe] {} hndrte_src
	    }
	}
	set hndrte_src
    }

    method hndrte_exe {} {
	if {![info exists hndrte_exe]} {
	    set hndrte_exe [$self rexec -n -s -q \
				cat hndrte-exe.lnk]
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

    # Semaphore to prevent host-side error recovery if dongle-side
    # recovery is already underway.
    variable processingDongleError false

    variable TRAPLOG

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

	if {[regexp {ASSERT pc} $msg]} {
	    # Downgrade these to warnings, since they will be handled
	    # by the trap handler later
	    UTF::Message WARN "$options(-name)>" $msg
	} elseif {[regexp {FWID (\d\d-[[:xdigit:]]{1,8})} $msg]} {
	    UTF::Message INFO "$options(-name)>" $msg
	    set TRAPLOG "$msg\n"
	} elseif {!$processingDongleError &&
		  [regexp {TRAP |ASSERT.*|Trap type .*| Overflow |Bad |Dongle trap|PSM microcode watchdog fired|PSM WD!} $msg]} {
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
	    UTF::Sleep 0.2 quiet

	    package require UTF::RTE
	    set trap [UTF::RTE::parse_traplog $self $TRAPLOG]
	    if {$trap ne ""} {
		set msg $trap
	    }

	    # If there's already an assert, don't overwrite it
	    if {![info exists ::UTF::panic] ||
		![regexp -nocase {assert|trap} $::UTF::panic] ||
		[regexp -nocase {Dongle assert file ""} $::UTF::panic]} {
		set ::UTF::panic $msg
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

    # Internal callback for fileevent below
    method rte_getdata {request} {
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
	fileevent $msgfile($id:$file) readable [mymethod rte_getdata $id:$file]
	UTF::Sleep 1
	UTF::Message LOG $options(-name) "Opened $file $ret"
	return $ret
    }

    UTF::doc {
	# [call [arg host] [method close_rte_messages] [lb][arg file][rb]]

	# Close a system message log previously opened by [method
	# open_messages].  An error will be reported if [arg file]
	# does not match that of a previous invocation of [method
	# open_rte_messages].
    }
    method close_rte_messages { {file ""} } {
	if {$file == ""} {
	    set file $options(-rteconsole)
	}
	set id [$self cget -lan_ip]
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
	# [call [arg host] [method rte_available]]

	# Returns true if rte commands are supported
    }

    method rte_available {} {
	if {[regexp {pcie} [$self cget -type]] ||
	    ([regexp {usb|high} [$self cget -type]] &&
	     [$self cget -rteconsole] ne "")} {
	    return 1
	} else {
	    return 0
	}
    }

    method rte {args} {
	if {[regexp {usb|high} [$self cget -type]]} {
	    # Use console on USB
	    set ret [$self serialrelay rexec -t 5 -s \
			 $UTF::unittest/rteshell -$options(-rteconsole) $args]
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

    method !rte {args} {
	$self serialrelay rexec -t 5 -s \
	    $UTF::unittest/rteshell -$options(-rteconsole) $args
    }


    method upload {{file dongle.dmp}} {
	if {![regexp {sdio|pcie} [$self cget -dongleimage]]} {
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
	    UTF::RTE::parse_traplog $self $ret
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
    }

    UTF::doc {
	# [call [arg host] [method apshell] [arg args]]

	# Run cmdline [arg args] on the [arg host] console directly,
	# bypassing the usual communication methods.
    }

    method apshell {args} {
	$self serialrelay rexec -t 5 -s \
	    $UTF::unittest/apshell -$options(-console) $args
    }

    method arch {} {
	if {$arch eq ""} {
	    lassign [$self -n uname -mr] kernel arch
	}
	set arch
    }

    method kernel {} {
	if {$kernel eq ""} {
	    lassign [$self -n uname -mr] kernel arch
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
	foreach A [split $a "."] B [split $b "."] {
	    if {$A < $B} {
		return -1
	    } elseif {$A > $B} {
		return 1
	    }
	}
	return 0
    }

    # Peer passthroughs
    UTF::PassThroughMethod relay -relay
    UTF::PassThroughMethod tftpserver -tftpserver
    UTF::PassThroughMethod serialrelay -serialrelay

    method whatami {{STA ""}} {
	if {[catch {$self boardname} b]} {
	    set b "<unknown>"
	}
	if {$STA ne ""} {
	    if {[catch {$STA chipname} c]} {
		set c "<unknown>"
	    }
	    # Chipname contains rev information
	    if {[lindex $c 0] ne $b} {
		# Router / Wireless Rev
		set b "$b/$c"
	    } else {
		# Single-chip combo wireless router
		set b $c
	    }
	}
	return "STB $b"
    }

}

# Retrieve manpage from last object
UTF::doc [UTF::STB man]

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
