#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::Android 2.0
package require snit
package require UTF::doc
package require UTF::Linux
package require UTF::Base

UTF::doc {
    # [manpage_begin UTF::Android n 2.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Android DHD support}]
    # [copyright {2007 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::Android is an implementation of the UTF host object, specific
    # to DHD-supported dongles on Android phones/Brix.

    # Once created, the Android object's methods are not normally
    # invoked directly by test scripts, instead the Android object is
    # designated the host for one or more platform independent STA
    # objects and it will be the STA objects which will be refered to
    # in the test scripts.

    # Android is based on the UTF::Linux object, and in fact delegates
    # most methods and options to an internal instance of that object.

    # [list_begin definitions]

}

snit::type UTF::Android {
    UTF::doc {
	# [call [cmd UTF::Android ] [arg host]
	#	[option -name] [arg name]
	#       [lb][option -image] [arg driver][rb]
	#       [lb][option -brand] [arg brand][rb]
	#       [lb][option -tag] [arg tag][rb]
	#       [lb][option -type] [arg type][rb]
	#       [lb][option -date] [arg date][rb]
	#       [lb][option -wlinitcmds] [arg cmds][rb]
	#       [lb][option -ush] [arg {true|false}][rb]
        #       [arg ...]]

	# Create a new Android host object.  The host will be
	# used to contact STAs residing on that host.
	# [list_begin options]

	# [opt_def [option -name] [arg name]]

	# Name of the host.

	# [opt_def [option -image] [arg driver]]

	# Specify a default driver to install when [method load] is
	# invoked.  This can be an explicit path to a [file wl.o] or
	# [file wl.ko] file, or a suitable list of arguments to
	# [method findimages].

	# [opt_def [option -brand] [arg brand]]

	# Specify build brand.  Default is [file linux-internal-dongle].

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
	# UTF::Linux] object.

	# [list_end]
	# [list_end]

	# [para]
	# Android objects have the following methods:
	# [para]
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    # base handles any other options and methods
    component base -inherit yes

    option -dhd_image
    option -dhd_tag "trunk"
    option -dhd_brand
    option -dhd_date "%date%"
    option -supp_tag "trunk"
    option -halutil_tag "trunk"
    option -supp_brand
    option -supp_date ""
    option -image
    option -nvram
    option -clm_blob ""
    option -sta
    option -name -configuremethod CopyOption
    option -tag "trunk"
    option -date "%date%"
    option -type
    option -driver "dhd-cdc-sdmmc-android-Android-cfg80211-oob-debug-3.0.8-Android"
    option -customer "android"
    option -modopts ""
    option -module "bcmdhd"
    option -parallocity
    option -device
    option -console
    option -hostconsole "/dev/kmsg"
    option -postinstall
    option -postinstall_hook
    option -postunload_hook
    option -assertrecovery 1
    option -nvram_add
    option -wlinitcmds
    option -adb_tcp_port 5555
    option -adbdevice


    variable kernel ""

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args

	install base using \
	    UTF::Linux %AUTO% -init [mymethod init] \
	    -ssh "adb" \
	    -lan_ip "" -user "" -rexec_add_errorcodes 1 \
	    -ping "ping -n -q -c 1 -w %c -s %s" \
	    -msgcallback [mymethod msgcallback] \
	    -brand linux-internal-dongle -iperfdaemon 0 -nointerrupts 1 \
	    -nobeaconratio 1
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	if {$options(-hostconsole) ne ""} {
	    $base configure -console $options(-hostconsole)
	}
	if {$options(-console) eq [$base cget -console]} {
	    UTF::Message WARN $options(-name) \
		"No need to set -console if it is the same as -hostconsole"
	    set options(-console) ""
	}
	foreach {sta dev} $options(-sta) {
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

    method init {} {
	$self adb connect $options(-adbdevice)
	$self open_messages
	$self adb -s $options(-adbdevice):$options(-adb_tcp_port) root
	$self adb -s $options(-adbdevice):$options(-adb_tcp_port) remount
    }

    method deinit {} {
	$self close_messages
	$self configure -initialized 0
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
	# to find a dingle image.  bcmdhd.ko may also be searched for,
	# depending on kernel version.

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
	    {nvram.arg "" "ignored"}
	    {driver.arg "$options(-driver)" "Host Driver"}
	    {brand.arg "[$self cget -brand]" "brand"}
	    {tag.arg "$options(-tag)" "Build Tag"}
	    {type.arg "$options(-type)" "Build Type"}
	    {date.arg "$options(-date)" "Build Date"}
	    {dhd_image.arg "[$self cget -dhd_image]" "DHD image"}
	    {dhd_tag.arg "[$self cget -dhd_tag]" "DHD tag"}
	    {dhd_brand.arg "[$self cget -dhd_brand]" "DHD brand"}
	    {supp_tag.arg "[$self cget -supp_tag]" "Supplicant tag"}
	    {supp_brand.arg "[$self cget -supp_brand]" "supplicant brand"}
	    {supp_date.arg "[$self cget -supp_date]" "supplicant Date"}
	    {halutil_tag.arg "[$self cget -halutil_tag]" "halutil tag"}
	    {dhd_date.arg "[$self cget -dhd_date]" "Build Date"}
	    {customer.arg "$options(-customer)" "Customer"}
	    {app_tag.arg "[$self cget -app_tag]" "Build Tag"}
	    {app_date.arg "[$self cget -app_date]" "Build Date"}
	    {gub.arg "[$self cget -gub]" "Alternate GUB build path"}
	}]

	set file [lindex $args end]

	set tag $(tag)
	set brand $(brand)
	set date $(date)

	# Pull fallback dhd_image out of -image
	if {$(dhd_image) eq ""} {
	    set image [$self cget -image]
	    set (dhd_image) [from image -dhd_image]
	    unset image
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
	    regsub {/rtecdc$} $typedir {} typedir
	    return [glob "$file{{{/../build/dongle,}/$typedir,}/rtecdc.$typesuffix,{{/bcm,}/firmware,}/$typedir.$typesuffix}"]

	} elseif {$(dhd_image) != "" && [file extension $file] eq ".img"} {
	    if {[file isfile $(dhd_image)]} {
		# Use specific .ko file user specified
		return $(dhd_image)
	    } else {
		# Use developer build for DHD driver - either src,
		# release, or flat folder will do.
		return [glob "$(dhd_image){{{/dhd,}/linux,{/$(customer),}/host,}/$(driver),}/$file{,.gz}"]
	    }
	} elseif {$file eq "logstrs.bin" ||
		  $file eq "rtecdc.map" ||
		  $file eq "roml.bin" ||
		  $file eq "roml.map" ||
		  $file eq "rtecdc.exe" ||
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
	} elseif {[UTF::BuildFile::exists $file]} {
	    return $file
	}

	if {$file eq "wl" || $file eq "dhd"} {
	    set tag $(app_tag)
	    set date $(app_date)
	    if {[regexp -nocase {trunk|nightly|WLAPP_REL*} $tag]} {
		set brand android-combined-apps
	    } else {
		set brand android-external-dongle
	    }
	    set tail "{release/$(customer)/apps,internal/bcm/android_x86_64}/${file}x86_android_ndk_r6b"
	} elseif {[regexp {/rtecdc\.(bin|trx|romlsim\.trx)$} $file]} {
	    # Firmware hasn't been copied - pull directly from firmware build
	    set brand hndrte-dongle-wl
	    set tail "build/dongle/$file"
	} elseif {[regexp {\.(bin|txt|trx)$} $file]} {
	    if {[regexp {^src/} $file]} {
		# Allow the user to specify src/... to force a lookup
		# relative to the src tree.
		if {[regsub {/src$} [lindex $args end-1] "/$file" tail] &&
		    [UTF::BuildFile::exists  $tail]} {
		    # developer tree
		    return [glob $tail]
		}
		set tail $file
	    } elseif {[regexp {\.txt} $file] && ![regexp {/} $file]} {
		# If the nvram file has no directory components, just
		# look it up in the gallery.
		return "$::UTF::projgallery/src/shared/nvram/$file"
	    } else {
		set tail [file join release bcm firmware $file]
	    }
	} elseif {[regexp {\.(?:exe|opt)$} $file]} {
	    set brand hndrte-dongle-wl
	    regsub {(\.romlsim)?\.(trx|bin)$} $(type) {} type
	    regsub {/rtecdc$} $type {} type
	    set tail [file join src dongle rte wl builds $type $file]
	    unset type
	} else {
	    set tag $(dhd_tag)
	    set brand $(dhd_brand)
	    set date $(dhd_date)
	    set tail [file join release $(customer) host \
			  $(driver) $file]
	}
	if {$file eq "wpa_supplicant"} {
	    set tag $(supp_tag)
		set date $(supp_date)
	    if {[regexp -nocase {trunk|nightly} $tag]} {
		set brand android-brix-kk-wpa-supp
	    } else {
		set brand android-brix-kk-wpa-supp
	    }
	    set tail "release/all/bin/$file"
	}

	if {$file eq "halutil"} {
	    set tag $(halutil_tag)
	    set brand android-brix-kk-hal-util 
	    set tail "release/$file"
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
	set waitfortrap 0
	try {
	    # Avoid looping if the driver asserts immediately
	    if {[incr reloadlock] != 1} {
		return
	    }
	    set waitfortrap 1

	    $self unload

	    # Invalidate tuning cache
	    set tuned ""

	    # WAR for TCP slow-start and restart after panic
	    $self sysctl -w net.ipv4.tcp_congestion_control=reno \
		kernel.panic=1


	    # Preinstall hook
	    UTF::forall {STA dev} [$self cget -sta] \
		cmd [$self cget -preinstall_hook] {
		    if {[catch [string map [list %S $STA] $cmd] ret]} {
			UTF::Message WARN $STA $ret
		    }
		}

	    array set reclaim {}
	    array set pre_reclaim {}
	    $self rexec stop
	    $self rexec insmod /installmedia/$options(-module).ko \
		$options(-modopts) \
		firmware_path=/installmedia/rtecdc.bin \
		nvram_path=/installmedia/nvram.txt \
		clm_path=/installmedia/rtecdc.clmb \
		logstrs_path=/installmedia/logstrs.bin \
		st_str_file_path=/installmedia/rtecdc.bin \
		map_file_path=/installmedia/rtecdc.map \
		rom_st_str_file_path=/installmedia/roml.bin \
		rom_map_file_path=/installmedia/roml.map

	    if {$options(-postinstall) ne ""} {
		$self rexec $options(-postinstall)
	    }
	    UTF::forall {STA dev} [$self cget -sta] \
		cmd [$self cget -postinstall_hook] {
		    if {[catch [string map [list %S $STA] $cmd] ret]} {
			UTF::Message WARN $STA $ret
		    }
		}

	    $self rexec ifconfig $options(-device) up
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

    method adb {args} {
	$self relay -n adb {*}$args
    }

    UTF::doc {
	# [call [arg host] [method copyto] [arg src] [arg dest]]

	# Copy local file [arg src] to [arg dest] on [arg host]
	# If src ends in .gz, but dest doesn't then the file will
	# be uncompressed automatically.
    }
    method copyto {src dst} {
	$self relay copyto $src /tmp/Androidstaging
	$self adb -s $options(-adbdevice):$options(-adb_tcp_port) push /tmp/Androidstaging $dst
    }


    UTF::doc {
	# [call [arg host] [method copyfrom] [arg src] [arg dest]]

	# Copy file [arg src] on [arg host] to local [arg dest]
	# If src ends in .gz, but dest doesn't then the file will
	# compressed automatically.
    }
    method copyfrom {src dst} {
	$self adb -s $options(-adbdevice):$options(-adb_tcp_port) pull $src /tmp/Androidstaging
	$self relay copyfrom /tmp/Androidstaging $dst
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
	# form [arg file] should be the pathname of a [file bcmdhd.ko]
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
	UTF::GetKnownopts {
	    {n "Just copy the files, don't actually load the driver"}
	    {all "ignored"}
	    {ls "ignored"}
	}

	UTF::Message INFO $options(-name) "Load Android Driver"

	set driver $options(-module).ko

	set image [$self findimages {*}$args]
	UTF::Message LOG $options(-name) "Dongle Image $image"
	#if {[regexp -line {.* FWID.*\Z} [exec strings $image] id]} {
	#   UTF::Message INFO $options(-name) $id
	#}

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
		UTF::Message LOG $options(-name) "CLM blob $blob"
	    }
	}

	set driver [$self findimages {*}$args $driver]
	UTF::Message LOG $options(-name) "Driver $driver"

	set zImage [file join [file dirname $driver] zImage]
	if {[file extension $driver] eq ".gz"} {
	    set zImage "$zImage.gz"
	}
	if {[file exists $zImage]} {
	    UTF::Message LOG $options(-name) "zImage $zImage"
	} else {
	    unset zImage
	}

	if {$options(-nvram) ne ""} {
	    set nvram [$self findimages {*}$args $options(-nvram)]
	    UTF::Message LOG $options(-name) "NVRAM $nvram"
	}
	set wl [$self findimages {*}$args "wl"]
	UTF::Message LOG $options(-name) "wl $wl"

	set dhd [$self findimages {*}$args "dhd"]
	UTF::Message LOG $options(-name) "dhd $dhd"

	set wpa_supplicant [$self findimages {*}$args "wpa_supplicant"]
	UTF::Message LOG $options(-name) "wpa_supplicant $wpa_supplicant"

	set halutil [$self findimages {*}$args "halutil"]
	UTF::Message LOG $options(-name) "halutil $halutil"

	UTF::BuildFile::copyto $self $driver /installmedia/$options(-module).ko
	UTF::BuildFile::copyto $self $image /installmedia/rtecdc.bin
	if {[info exists nvram]} {
	    set nvram [UTF::nvram_add $nvram $options(-nvram_add)]
	    UTF::BuildFile::copyto $self $nvram /installmedia/nvram.txt
	    if {[regexp {^/tmp/nvram\.txt_} $nvram]} {
		UTF::Message LOG "" "file delete $nvram"
		file delete $nvram
	    }
	}
	if {[info exists blob]} {
	    if {[llength $blob] > 1} {
		UTF::Message LOG $options(-name) "CLM blob $blob"
		$self -n $UTF::unittest/bin/clmtrx.sh rtecdc.clmb {*}$blob
	    } else {
		UTF::BuildFile::copyto $self $blob /installmedia/rtecdc.clmb
	    }
	}
	UTF::BuildFile::copyto $self  $wl /installmedia/wl
	UTF::BuildFile::copyto $self  $dhd /installmedia/dhd
	UTF::BuildFile::copyto $self  $wpa_supplicant /installmedia/wpa_supplicant
	UTF::BuildFile::copyto $self  $halutil /installmedia/halutil
	# RAM AUX files - must match current build
	foreach a {logstrs.bin rtecdc.map} {
	    set aux [file join [file dirname $image] $a]
	    if {[UTF::BuildFile::exists $aux]} {
		UTF::BuildFile::copyto $self $aux /installmedia/$a
	    } else {
		$self -n rm -f $a
	    }
	}
	foreach a {rtecdc.exe} {
	    set aux [file join [file dirname $image] $a]
	    if {[UTF::BuildFile::exists $aux]} {
		$self relay rm -f /root/hndrte-exe.lnk
		$self relay rexec -n "ln -s $aux /root/hndrte-exe.lnk"
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
		    UTF::BuildFile::copyto $self $aux /installmedia/$a
		}
	    }
	}
	if {$(n)} {
	    UTF::Message LOG $options(-name) "Copied files - not loading"
	    return
	}

	if {[info exists zImage]} {
	    $self KernelUpdate $zImage
	}
	if {$options(-parallocity) ne ""} {
	    if {[catch {$self lsmod|grep modzvm}]} {
		$self rexec "test -c /dev/zvmk||mknod /dev/zvmk c 245 0"
		$self UTF::BuildFile::copyto $options(-parallocity) /data/dhd/modzvm.ko
		$self insmod /data/dhd/modzvm.ko
	    }
	}

	$self reload
    }

    UTF::doc {
	# [call [arg host] [method unload]]

	# Unload the current dhd driver.
    }

    method unload {} {
	UTF::Message INFO $options(-name) "Unload DHD Driver"
	$self supplicant stop
	try {
	    # Avoid reloading if we crash during unload
	    incr reloadlock
	    catch {$self wl down}; # WAR for radio left on
	    if {[catch {$self lsmod} ret] ||
		([string match "$options(-module) *" $ret] &&
		 [catch {$self rmmod $options(-module)} ret])} {
		if {[regexp {Timeout|child killed|resource busy|errno 16} $ret]} {
		    set ret "rmmod timeout: try power cycle"
		    $self worry $ret
		    $self power cycle
		    UTF::Sleep 60
		    $self configure -initialized 0
		    for {set i 0} {[catch {$self :}] && $i < 10} {incr i} {}
		    return $ret
		} else {
		    error $ret
		}
	    }
	    foreach {STA dev} [$self cget -sta] {
		eval [string map [list %S $STA] \
			  [$self cget -postunload_hook]]
	    }
	} finally {
	    incr reloadlock -1
	}
    }

    UTF::doc {
	# [call [arg host] [method macaddr] [arg dev]]

	# Returns the MAC address for [arg dev]
    }
    method macaddr {dev} {
	if {[regexp {(?:ether|HWaddr)\s+(\S+)} \
		 [$self -n  busybox ifconfig $dev] - mac]} {
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
	$self rexec route add -net $net netmask $mask gw $gw
    }
	UTF::doc {
       # [call [arg STA] [method {supplicant start}] [arg device]]

       # Start supplicant for [arg device]
    }

    method {supplicant start} {device} {
	$self copyconf
	$self rexec wpa_supplicant -B -i$device -dd \
	    -Dnl80211 -c/data/misc/wifi/wpa_supplicant.conf \
	    -m/data/misc/wifi/p2p_supplicant.conf \
	    -puse_p2p_group_interface=1p2p_device=1 -e/data/misc/wifi/entropy.bin \
	    -g/data/misc/wifi/sockets/wpa_$device
	UTF::Sleep 10
    }

    UTF::doc {
	# [call [arg STA] [method wpa_cli] [arg -i] [arg device]
	#        [arg {args ...}]]

	# Wrapper around the wpa_cli tool.  This adds Android-specific
	# options.  Also wpa_cli doesn't use return codes, so we have
	# to explicitly check for "OK"
    }

    method wpa_cli {- device args} {
	if {$device ne ""} {
	    set if IFNAME=$device
	} else {
	    set if ""
	}
	set ret [$self rexec -s wpa_cli -iwpa_wlan0 \
		     -p/data/misc/wifi/sockets/ {*}$if {*}$args]
	if {[regexp {FAIL} $ret]} {
	    UTF::Message ERROR $options(-name) $ret
	    throw FAIL $ret
	} else {
	    regsub "Selected interface '.*'\n" $ret {} ret
	    regsub "OK" $ret {} ret
	    UTF::Message LOG $options(-name) $ret
	    return $ret
	}
    }
    
    method wpa_clif {args} {
	set ret [$self rexec -s wpa_cli -iwpa_wlan0 \
		     -p/data/misc/wifi/sockets/ {*}$args]
	if {[regexp {FAIL} $ret]} {
	    UTF::Message ERROR $options(-name) $ret
	    throw FAIL $ret
	} else {
	    regsub "Selected interface '.*'\n" $ret {} ret
	    regsub "OK" $ret {} ret
	    UTF::Message LOG $options(-name) $ret
	    return $ret
	}
    }

    UTF::doc {
	# [call [arg STA] [method {supplicant stop}] [arg device]]

	# Stop all supplicants ([arg device] is currently ignored)
    }

    method {supplicant stop} {{device ""}} {
	if {![catch {$self -n pidof wpa_supplicant} p]} {
		$self rexec kill $p
	    UTF::Sleep 2
	}
    }

    UTF::doc {
	# [call [arg host] [method ifconfig] [arg dev] [option dhcp]]

	# Enable DHCP on device [arg dev].

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

	set PF "/data/misc/dhcp/dhcpcd-${dev}.pid"
	if {[llength $args]} {
	    # Setting something - kill off dhcpcd
	    $self rexec -x "test -f $PF && kill \\`cat $PF\\` "
	}
	if {$args eq "dhcp" || $args eq "local"} {
	    # invalidate cache in case of failure
	    if {[info exists ipaddr($dev)]} {
		unset ipaddr($dev)
	    }
		if {$leases eq ""} {
		if {![catch {$self test -d /system/bin/dhcpcd}]} {
		    set leases "/var/lib/dhclient"
		} else {
		    set leases "/data/misc/dhcp"
		}
	    }
	    set LF [file join $leases "dhcpcd-${dev}.lease"]
	    set CF "/etc/dhcpcd/dhcpcd.conf"
	    # Allow for one retry on dhcpcd collision
	    for {set i 0} {$i < 2} {incr i} {
		if {![catch {
		    $self rexec dhcpcd $dev -d
		} ret]} {
		    break
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
	# [call [arg host] [method {route replace}] [arg net] [arg mask]
	#	  [arg gw]]

	# Add or Modify an IP route to network [arg net] netmask [arg
	# mask] through gateway [arg gw]
    }
    method {route replace} {net mask gw} {
	catch {$self rexec route delete -net $net netmask $mask}
	$self rexec route add -net $net netmask $mask gw $gw
    }

    UTF::doc {
	# [call [arg host] [method {route delete}] [arg net] [arg mask]
	#	  [lb][arg gw][rb]]

	# Delete an IP route to network [arg net] netmask [arg mask]
	# through optional gateway [arg gw]
    }
    method {route delete} {net mask {gw ""}} {
	$self rexec route delete -net $net netmask $mask $gw
    }

    UTF::doc {
	# [call [arg host] [method rte_available]]

	# Returns true if rte commands are supported
    }

    method rte_available {} {
	return 1
    }

    method rte {args} {
	# Use dhd pass through
	try {
	    set rtecapture ""
		$self dhd -i $options(-device) wdtick 10
		$self dhd -i $options(-device) dconpoll 10
	    $self dhd -i $options(-device) cons $args
	    UTF::Sleep 1
	} finally {
	    set ret $rtecapture
	    unset rtecapture
	}
	return $ret
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
		file attributes $f -permissions go+r
	    } ret]} {
		UTF::Message WARN $options(-name) $ret
	    }
	}
    }

    # Semaphore to prevent host-side error recovery if dongle-side
    # recovery is already underway.
    variable processingDongleError false
    variable processingEccFailure false
    variable isalive false
    variable rtecapture

    # Message handling callback.  Used by "base" to process log
    # messages.  Returns true if no further processing is required.
    method msgcallback {msg} {
	set isalive true
	switch -re -- $msg {
	    sdstd_check_errs -
	    {HW hdr error: |dhd: Unknown symbol} {
		# Only use the first of these.  Asserts get priority
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic $msg
		}
		UTF::Message FAIL $options(-name) $msg
		return 1
	    }
	    Android_unwanted_errs -
	    {PCIe: Assert the reset} {
		# This is not an assert, so log and skip other checks
		UTF::Message LOG $options(-name) $msg
		return 1
	    }
	    {Nested:} {
		$self worry $msg
		$self power cycle
		UTF::Sleep 6
		return 1
	    }
	    {bcm_rpc_call_with_return: RPC call} -
	    {dhd_bus_txctl: .* txcnt_timeout=1} -
	    {dhd_bus_rxctl: resumed on timeout} {
		# Only use the first of these.  Asserts get priority
		if {![info exists ::UTF::panic]} {
		    set ::UTF::panic $msg
		}
		UTF::Message FAIL $options(-name) $msg
		if {!$processingDongleError} {
		    try {
			set processingDongleError true
			if {[regexp {sdio} [$self cget -type]]} {
			    # This may mean the dongle locked up.  Dump and reload.
			    $self upload "dongle_dhd.dmp"
			}
			catch {
			    $self reload
			}
		    } finally {
			set processingDongleError false
		    }
		}
		return 1
	    }
	    {BUG: soft lockup - CPU#\d stuck} -
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
	    { PCIe: Assert the reset } {
		# This is not an assert, so log and skip other checks
		UTF::Message LOG $options(-name) $msg
	    }
	    {Dongle trap} -
	    CONSOLE: {
		# if we have an RTE console drop DHD's copy since it
		# is less accurate and delayed, otherwise treat it as
		# if it came from the serial port
		regsub {^CONSOLE: } $msg {} msg
		if {[info exists rtecapture]} {
		    append rtecapture "\n$msg"
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
			    ls -l --color=never /installmedia/hndrte-src.lnk} \
		     hndrte_src]} {
		regsub {/dongle/rte/.*} [$self hndrte_exe] {} hndrte_src
	    } else {
		regsub {.*-> } $hndrte_src {} hndrte_src
	    }
	}
	set hndrte_src
    }

    method hndrte_exe {} {
	if {![info exists hndrte_exe]} {
	    set hndrte_exe [$self relay rexec -noinit -s -q \
				ls -l --color=never /root/hndrte-exe.lnk]
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
	$self dhd -i $options(-device) membytes -h 0x$addr 0x10 00
    }

    method instant_panic {} {
	$self trash_symbol "wlc_scan"
	$self wl up
	$self wl scan
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

    variable TRAPLOG

    method consolemsg {msg} {
	set needreload 0

	if {$msg eq ""} {
	    return
	}

	regsub {^\d{6}\.\d{3} } $msg {} msg; # strip timestamps

	if {[info exists rtecapture]} {
	    append rtecapture "\n$msg"
	}

	if {[info exists TRAPLOG]} {
	    # If FWID is set we're collecting new-style trap messages
	    append TRAPLOG "$msg\n"
	}

	if {[regexp {ASSERT pc} $msg]} {
	    # Downgrade these to warnings, since they wil lbe handled
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

	    if {[regexp {TRAP |ASSERT.*|Trap } $msg] &&
		[regexp {sdio} [$self cget -type]]} {
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
	} elseif {[regexp {reclaim(.*): Returned (\d+) } $msg - s r]} {
	    if {[regexp {section 1} $s]} {
		set reclaim1 $r
	    } else {
		set reclaim2 $r
	    }
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
	if {[catch {$self rte mu} ret] || ![regexp {Free: (\d+)} $ret - free]} {
	    error "Free mem not found"
	}
	expr {$free / 1024.0}
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
	set id [$self cget -lan_ip]
	if {[info exists msgfile($id:$file)]} {
	    return
	}
	if {$file eq ""} {
	    # No serial console - using DHD pass-through.
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
	    if {$tuned eq "" || $tuned < $window} {
		$self rexec "echo $window >/proc/sys/net/core/rmem_max;\
                             echo $window >/proc/sys/net/core/wmem_max"

		set tuned $window
	    }
	    return 1
	}

	if {$window == 0} {
	    set window 108544
	    set tcp_rmem "4096\t87380\t190080"
	    set tcp_wmem "4096\t16384\t190080"
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
	    $self rexec "echo $window >/proc/sys/net/core/rmem_max;\
                         echo $window >/proc/sys/net/core/wmem_max;\
                         echo $tcp_rmem >/proc/sys/net/ipv4/tcp_rmem;\
                         echo $tcp_wmem >\/proc/sys/net/ipv4/tcp_wmem"
	    set tuned "$window\n$window\n$tcp_rmem\n$tcp_wmem"

	    # No iperf daemon is used on Android, instead a new server
	    # will be started for every connection.

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
	    set tcp_rmem "4096\t87380\t190080"
	    set tcp_wmem "4096\t16384\t190080"
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
	    $self rexec "echo $window >/proc/sys/net/core/rmem_max;\
                         echo $window >/proc/sys/net/core/wmem_max;\
                         echo $tcp_rmem >/proc/sys/net/ipv4/tcp_rmem;\
                         echo $tcp_wmem >\/proc/sys/net/ipv4/tcp_wmem"
	    set tuned "$window\n$window\n$tcp_rmem\n$tcp_wmem"
	}
	return 1
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
	# Loop, since Android ping doesn't handle "count" reliably
	for {set c 0} {$c < $(c)} {incr c 1} {
	    catch {$self rexec -t 10 ping -n -c 3 -w 3 -s $(s) $target} ret
	    if {[regexp {(\d+) received} $ret - n] && $n > 0} {
		return
	    }
	}
	error "ping failed"
    }

    method fastboot {args} {
	$self relay -n -t 120 fastboot {*}$args
	$self relay -n -t 120 fastboot devices
	UTF::Sleep 10
    }

    method KernelUpdate {{zImage 1.*}} {
	set Android_images \
	    /projects/hnd_software_ext8/work/edwardwa/Panda_android_image

	if {[regexp {^\d.\d} $zImage]} {
	    set zImage [lindex [glob $Android_images/$zImage/zImage] end]
	}
	UTF::Message INFO $options(-name) "Kernel: $zImage"

	# Compare local sum + running version with cached results.
	# Local sum determines if the image changed Running version
	# determines if someone upgraded ouside of UTF.  PRESERVED
	# will break the sum, but the extra upgrade will be harmless.
	set sum [localhost sum $zImage]
	set rev [$self uname -r]
	if {[catch {$self cat /data/dhd/utf.zImage.id} ret]} {
	    # Android "props" are not persistent - use a file
	    if {[set ret [$self getprop utf.zImage.id]] ne ""} {
		$self rexec "echo '$ret' > /data/dhd/utf.zImage.id"
	    }
	}
	if {$ret eq "$rev $sum"} {
	    UTF::Message INFO $options(-name) "Nothing to update"
	    return $rev
	}
	UTF::Message INFO $options(-name) "Upgrading Kernel"

	set model [$self getprop ro.product.model]
	if {![regexp {Pandaboard_(.*)} $model - baseOS]} {
	    error "Unknown base OS: $model"
	}
	set ramdisk [lindex [glob $Android_images/$baseOS/ramdisk.img] end]
	UTF::Message INFO $options(-name) "ramdisk: $ramdisk"

	$self relay UTF::BuildFile::copyto $zImage /tmp/zImage
	$self relay UTF::BuildFile::copyto $ramdisk /tmp/ramdisk.img

	if {[regexp {^1\.} $baseOS]} {
	    set hack_bootloader 1
	    # Static until we have an official release supporting
	    # fastboot continue
	    set xloader /home/tima/src/unittest/xloader.bin
	    set bootloader /home/tima/src/unittest/bootloader.bin
	    $self relay UTF::BuildFile::copyto $xloader /tmp/xloader.bin
	    $self relay UTF::BuildFile::copyto $bootloader /tmp/bootloader.bin
	}

	$self open_messages
	catch {
	    $self adb reboot bootloader
	    UTF::Sleep 5
	}

	if {[catch {$self fastboot devices} ret]} {
	    if {[regexp {not found} $ret]} {
		UTF::BuildFile::copyto $self relay \
		    $::UTF::projtools/linux/bin/fastboot \
		    /usr/bin/fastboot
		set ret [$self fastboot devices]
	    } else {
		error $ret $::errorInfo
	    }
	}
	$self fastboot -b 0x80000000 flash:raw boot /tmp/zImage /tmp/ramdisk.img

	if {[info exists hack_bootloader]} {
	    $self fastboot flash xloader /tmp/xloader.bin
	    $self fastboot flash bootloader /tmp/bootloader.bin
	}

	if {[catch {$self fastboot continue}]} {
	    $self power cycle
	}
	$self fastboot  boot /data/dhd/boot.img
	UTF::Sleep 10
	for {set i 0} {[catch {$self :}] && $i < 10} {incr i} {UTF::Sleep 10}
	set rev [$self uname -r]
	$self rexec "echo '$rev $sum' > /data/dhd/utf.zImage.id"
	return $rev
    }

    method OSUpgrade {{os 3.*}} {
	set Android_images \
	    /projects/hnd_software_ext8/work/edwardwa/Android_android_image

	if {[regexp {^\d.} $os]} {
	    set os [file join $Android_images $os]
	}
	set path [lindex [lsort [glob $os]] end]

	UTF::Message INFO $options(-name) "Path: $path"

	set sum [localhost sum $path/zImage]

	if {[catch {$self relay test -d $path}]} {
	    # Relay can't see files, so copy them
	    set dst /tmp/Androidos
	    $self relay rsync [glob $path/*.bin $path/*.img] $dst/
	    set path $dst
	}

	$self open_messages
	try {
	    catch {
		$self adb reboot bootloader
		UTF::Sleep 5
	    }
	    if {[$self fastboot devices] eq ""} {
		error "Device did not enter fastboot"
	    }

	    $self fastboot oem format

	    if {![regexp {^1\.} $os]} {
		# Don't downgrade bootloader below 2.x
		$self fastboot flash xloader $path/xloader.bin
		$self fastboot flash bootloader $path/bootloader.bin
	    }

	    $self fastboot flash system $path/system.img
	    $self fastboot flash userdata $path/userdata.img
	    $self fastboot flash boot $path/boot.img

	} finally {
	    # Gentler methods have problems due to the bootloader/os
	    # mismatches.  Power cycle seems to be the only reliable
	    # way.
	    if {[$self cget -power] eq ""} {
		$self fastboot reboot
	    } else {
		$self power cycle
	    }
	}
	UTF::Sleep 10
	for {set i 0} {[catch {$self :}] && $i < 10} {incr i} {UTF::Sleep 10}
	set rev [$self uname -r]
	$self rexec "echo '$rev $sum' > /data/dhd/utf.zImage.id"
	return $rev
    }

    # Peer passthroughs
    UTF::PassThroughMethod relay -relay
    UTF::PassThroughMethod serialrelay -serialrelay


    method setup {} {
	set p "$UTF::unittest/etc"
	UTF::BuildFile::copyto $self $p/wpa_supplicant.conf \
	    /installmedia/wpa_supplicant.conf
	UTF::BuildFile::copyto $self $p/wpa_supplicant_ap.conf \
	    /installmedia/wpa_supplicant_ap.conf
	UTF::BuildFile::copyto $self $p/p2p_supplicant.conf \
	    /installmedia/p2p_supplicant.conf
	UTF::BuildFile::copyto $self $p/hostapd_2g.conf \
	    /installmedia/hostapd_2g.conf
	UTF::BuildFile::copyto $self $p/hostapd_5g.conf \
	    /installmedia/hostapd_5g.conf
	 UTF::BuildFile::copyto $self $p/wpa_supplicant_adhoc_open.conf \
	    /installmedia/wpa_supplicant_adhoc_open.conf
	 UTF::BuildFile::copyto $self $p/wpa_supplicant_sta.conf \
	    /installmedia/wpa_supplicant_sta.conf
	UTF::BuildFile::copyto $self $p/iperf_2.0.5_android \
            /installmedia/iperf
	$self rexec "echo '2' > /installmedia/.memdump.info"
	unset p
    }

    method copyconf {} {
	$self rexec cp -v \
	    /installmedia/wpa_supplicant.conf \
	    /installmedia/wpa_supplicant_ap.conf \
	    /installmedia/p2p_supplicant.conf \
	    /installmedia/hostapd_2g.conf \
	    /installmedia/hostapd_5g.conf \
	    /installmedia/wpa_supplicant_adhoc_open.conf \
	    /installmedia/wpa_supplicant_sta.conf /data/misc/wifi/
	$self rexec chown system.wifi \
	    /data/misc/wifi/wpa_supplicant.conf \
	    /data/misc/wifi/wpa_supplicant_ap.conf \
	    /data/misc/wifi/p2p_supplicant.conf \
	    /data/misc/wifi/wpa_supplicant_adhoc_open.conf \
	    /data/misc/wifi/wpa_supplicant_sta.conf
	}

}

# Retrieve manpage from last object
UTF::doc [UTF::Android man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]
    package require UTF
    UTF::Android White -lan_ip 10.19.12.138 -sta {STA1 eth2}
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
    # [see_also [uri APdoc.cgi?UTF::Linux.tcl UTF::Linux]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
