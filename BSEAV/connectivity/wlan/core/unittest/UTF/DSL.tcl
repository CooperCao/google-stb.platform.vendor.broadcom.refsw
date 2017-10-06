#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::DSL 2.0

package require snit
package require UTF::doc
package require UTF::Base
package require UTF::RTE
package require json

snit::type UTF::DSL {

    pragma -canreplace yes

    option -image
    option -tag "WLAN_L04_Misc"
    option -p4tag ""
    option -date "%date%"
    option -web -type snit::boolean -default false
    option -sta
    option -name -configuremethod CopyOption
    option -serial_num -readonly yes
    option -ush -type snit::boolean -default true
    option -model
    option -console -configuremethod _adddomain
    option -device
    option -lan_ip 192.168.1.1
    option -passwd "admin"
    option -wanpeer
    option -nvram
    option -type "bcm963138GW_PURE181_nand_fs_image_128_ubi.w"
    option -jenkins "http://bcacpe-hudson.broadcom.com:8081"
    option -wlinitcmds
    option -fwoverlay -type snit::boolean -default false
    option -embeddedimage 4366c0
    option -defer_restart -type snit::boolean -default false

    # base handles any other options and methods
    component base -inherit yes

    # Default nvram settings
    variable nvram_defaults {
    }

    variable stas {}

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	install base using \
	    UTF::Base %AUTO% -ssh $UTF::usrutf/dslshell \
	    -init [mymethod init] \
	    -brand "linux-internal-dslcpe" \
	    -wlconf_by_nvram true
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}
	foreach {sta dev} [UTF::Staexpand $options(-sta)] {
	    if {$dev eq ""} {
		error "$sta has no device name"
	    }
	    UTF::STA ::$sta -host $self -device $dev
	    lappend stas ::$sta
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
    }

    destructor {
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
	# [call [arg host] [method load] [lb][option -web][rb]
	#	  [lb][arg file][rb]]
	# [call [arg host] [method load] [lb][option -web][rb]
	#	  [lb][arg {args ...}][rb]]

	# Install an OS image onto the Router.  If [option -web] is
	# specified, a web upgrade will be performed, otherwise the
	# load will be performed via CFE.  In the first form [arg
	# file] should be the pathname of a suitable [file linux.trx]
	# file.  In the second form, the argument list will be passed
	# on to [method findimages] to find a driver.  If no arguments
	# are specified, those stored in the [arg host]s [option
	# -image] option will be used instead.
    }

    variable tftpdmsg ""
    variable hndrte_exe


    method findimages {args} {
	if {$options(-fwoverlay)} {
	    $self findimages_fw {*}$args
	} else {
	    $self findimages_os {*}$args
	}
    }

    method load {args} {
	if {$options(-fwoverlay)} {
	    $self load_fw {*}$args
	} else {
	    $self load_os {*}$args
	}
    }

    ########################################
    # Overlay test FW on pre-loaded OS
    ########################################

    method findimages_fw {args} {

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
	    {brand.arg "[$self cget -brand]" "brand"}
	    {tag.arg "[$self cget -tag]" "Build Tag"}
	    {type.arg "[$self cget -type]" "Build Type"}
	    {date.arg "[$self cget -date]" "Build Date"}
	    {gub.arg "[$self cget -gub]" "Alternate GUB build path"}
	}]

	set file [lindex $args end]

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
	    # Handle case of non-release images
	    regsub {/rtecdc$} $typedir {} typedir
	    return [glob "$file{{{/dongle/rte/wl/builds,/../build/dongle,}/$typedir,}/rtecdc.$typesuffix,{/firmware,}/$typedir.$typesuffix}"]

        } elseif {[file exists $file]} {
	    return $file
	}

	if {[regexp {/rtecdc\.bin(?:\.trx)$} $file]} {
	    # Firmware hasn't been copied - pull directly from firmware build
	    set (brand) hndrte-dongle-wl
	    set tail "{build/dongle,src/dongle/rte/wl/builds}/$file"
	} elseif {[regexp {\.(bin|txt|trx)$} $file]} {
	    if {[regexp {^src/} $file]} {
		# Allow the user to specify src/... to force a lookup
		# relative to the src tree.
		if {[regsub {/src$} [lindex $args end-1] "/$file" tail] &&
		    [file exists $tail]} {
		    # developer tree
		    return [glob $tail]
		}
		set tail $file
	    } elseif {[regexp {\.txt} $file] && ![regexp {/} $file]} {
		# If the nvram file has no directory components, just
		# look it up in the gallery.
		return "$::UTF::projgallery/src/shared/nvram/$file"
	    } else {
		if {[regexp {dhdap} $(brand)]} {
		    regsub {\.bin$} $file {} file
		    set tail "build/43*/{build/dongle,src/dongle/rte/wl/builds}/$file/rtecdc.bin"
		} else {
		    set tail [file join release bcm firmware $file]
		}
	    }
	} elseif {[regexp {\.(?:exe|opt)$} $file]} {
	    set (brand) hndrte-dongle-wl
	    regsub {(\.romlsim)?\.(trx|bin|bin\.trx)$} $(type) {} type
	    set tail [file join src dongle rte wl builds $type $file]
	    unset type
	} else {
	    error "Host driver search unsupported"
	}

	if {[regexp {_REL_} $(tag)]} {
	    set (tag) "{PRESERVED/,ARCHIVED/,}$(tag)"
	} else {
	    set (tag) "{PRESERVED/,}$(tag)"
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

    method load_fw {args} {
	UTF::Message INFO $options(-name) "Install Dongle Firmware"

	UTF::GetKnownopts [subst {
	    {n "Copy files, but don't reload the driver"}
	    {erase "ignored"}
	    {all "ignored"}
	    {ls "ignored"}
	}]

	set image [$self findimages {*}$args]
	UTF::Message LOG $options(-name) "Dongle Image $image"

	$self copyto $image  /data/rtecdc.bin

	if {$(n)} {
	    UTF::Message LOG $options(-name) "Copied files - not loading"
	    return
	}
	if {[regexp {4908} [$self cget -model]]} {
	    $self nvram set firmware_path0="/data/rtecdc.bin"
	    #$self nvram set nvram_path0="/data/nvram.txt"
	    $self nvram set firmware_path1="/data/rtecdc.bin"
	    #$self nvram set nvram_path1="/data/nvram.txt"
	    $self nvram set firmware_path2="/data/rtecdc.bin"
	    #$self nvram set nvram_path2="/data/nvram.txt"
	    $self nvram commit
	} else {
	    if {![regexp {^(.*)-r(:?oml|am)/} $options(-type) - chip]} {
		error "Unable to determine chip name from type=$options(-type)"
	    }
	    $self mount -t ubifs ubi:rootfs_ubifs / -o remount,rw
	    $self cp /data/rtecdc.bin /etc/wlan/dhd/$chip/rtecdc.bin
	    $self rm -f /data/src-id; # flag build as dirty
	}
	$self sync
	$self reboot

	$base configure -initialized 0
	UTF::Sleep 30
	$self relay ping [$self cget -lan_ip] -c 60
	UTF::Sleep 10

	set ver [$self wl ver]
        regexp {version (.*)} $ver - ver
        set ver
    }


    ########################################
    # Load Fresh OS image
    ########################################

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

	# [arg file] should be a [file .trx] file, eg [file linux.trx].

	# [para]

	# [list_end]

    }

    method findimages_os {args} {
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
	    {p4tag.arg "$options(-p4tag)" "Perforce Tag"}
	    {date.arg "$options(-date)" "Build Date"}
	    {type.arg "$options(-type)" "Default image name"}
	}]

	set file $args

	if {$file eq ""} {
	    set file $(type)
	} elseif {[regexp {^http:.*/$} $file]} {
	    $self relay -s curl -f -I $file$(type)
	    return $file$(type)
	} elseif {[regexp {^http:} $file]} {
	    $self relay -s curl -f -I $file
	    return $file
	} elseif {[file exists $file]} {
	    return $file
	} elseif {[file exists [file dir $file]]} {
	    error "File not found"
	}

	if {$options(-jenkins) eq ""} {

	    set tail [file join images $options(-type)]
	    if {$(p4tag) eq ""} {
		set p4 $(tag)
	    } else {
		set p4 "$(p4tag)_$(tag)"
	    }

	    set pattern [file join \
			     /projects/bcawlan_builds/Irvine/ \
			     "{PRESERVED/,}$p4" $(tag)__$(brand) \
			     $(date)* "$tail{,.gz}"]

	    if {$(showpattern)} {
		UTF::Message INFO $options(-name) $pattern
	    }
	    UTF::SortImages [list $pattern] \
		{*}[expr {$(ls)?"-ls":""}] \
		{*}[expr {$(all)?"-all":""}]
	} else {
	    set api \
		[::json::json2dict \
		     [$self relay -s curl -f -s \
			  "$options(-jenkins)/job/$(tag)/api/json"]]

	    set path "artifact/CommEngine/allimages/$file"
	    if {$(all)} {
		foreach build [dict get $api builds] {
		    set num [dict get $build number]
		    set url [dict get $build url]
		    #UTF::Message DBG $options(-name) "Build $num: $url"
		    lappend ret "$url$path"
		}
	    } elseif {$(date) != "%date%"} {
		foreach build [dict get $api builds] {
		    set num [dict get $build number]
		    set url [dict get $build url]
		    if {$num eq $(date)} {
			#UTF::Message DBG $options(-name) "Build $num: $url"
			lappend ret "$url$path"
		    }
		}
		if {![info exists url]} {
		    error "Build $(date) not found"
		}
	    } else {
		set url "[dict get $api lastSuccessfulBuild url]"
		#UTF::Message DBG $options(-name) "Last Sucesfull: $url"
		set ret "$url$path"
	    }
	    join $ret "\n"
	}
    }

    method load_os {args} {
	UTF::Message INFO $options(-name) "Install Router Image"

	UTF::GetKnownopts [subst {
	    {erase "ignored"}
	    {n "Just copy the files, don't actually load the driver"}
	    {force "Force upload, even if not changed"}
	    {all "ignored"}
	    {ls "ignored"}
	}]

	set file [eval $self findimages $args]
	UTF::Message LOG $options(-name) "Found $file"

	if {[regexp {^http://} $file]} {
	    set info "$file\nxxx"
	} else {
	    set info "$file\n[exec sum [file normalize $file]]"
	}
	regsub {/} $options(-name) {.} name
	set idfile "/data/src-id"

	if {!$(force)} {
	    if {[catch {
		set ret [$self -t 5 dhd -i $options(-device) version]
		set oldinfo "$info\n$ret"
		set newinfo [$self cat $idfile]
	    } ret]} {
		UTF::Message WARN $options(-name) $ret
	    } elseif {$oldinfo eq $newinfo} {
		UTF::Message LOG $options(-name) \
		    "Skipping reload of same image"
		if {![catch {$self rexec "rm /data/rtecdc.*"}]} {
		    # Reboot after removing override fw
		    $self sync
		    $self reboot
		}
		set ver [$self wl ver]
		regexp -line {version (.*)} $ver - ver
		return "$ver (skipped)"
	    }
	}

	set f /tmp/[file tail $file]
	if {[regexp {^http://} $file]} {
	    # fetch
	    $self relay curl -f -o $f $file
	} else {
	    # uncompress
	    regsub {\.gz$} $f {} f
	    $self relay copyto [file normalize $file] $f

	    $self relay rm -rf "/tmp/$name-hndrte-exe.lnk"
	    if {[regsub {/images/.*} $file \
		     "/bcmdrivers/broadcom/net/wl/impl51/43*/build/dongle/$options(-embeddedimage)*/*/rtecdc.exe" tail]} {
		if {![catch {glob $tail} ret]} {
		    set exe [lindex $ret 0]
		} else {
		    UTF::Message LOG $options(-name) \
			"Symbol table not found in Router src: $ret"
		}
		if {[info exists exe]} {
		    UTF::Message LOG $options(-name) "Symbol file: $exe"
		    $self relay ln -s '$exe' "/tmp/$name-hndrte-exe.lnk"
		    set hndrte_exe $exe
		    UTF::RTE::init
		}
	    }
	}

	if {$(n)} {
	    UTF::Message LOG $options(-name) "Copied files - not loading"
	    return
	}

	$self open_messages

	# pass control to dslshell which will use tftp from the OS, or
	# the CFE as appropriate
	$self relay $UTF::usrutf/dslshell $options(-console) \
	    load $f -quiet

	$base configure -initialized 0
	UTF::Sleep 30
	$self relay ping $options(-lan_ip) -c 60
	UTF::Sleep 10

	if {![catch {$self relay rexec "rm /data/rtecdc.*"}]} {
	    # Reboot after removing override fw
	    $self sync
	    $self reboot
	}

	set dhdver [$self dhd -i $options(-device) version]
	$self rexec "echo '$info\n$dhdver' > $idfile"
	set ver [$self wl ver]
	regexp {version (.*)} $ver - ver
	if {![regexp {4908} [$self cget -model]]} {
	    # sleep > 3 required here, else later nvram defaults don't work
	    UTF::Sleep 10
	}
        set ver
}

    ############################################
    # common
    ############################################

    UTF::doc {
	# [call [arg host] [method reload]]

	# Reload the wl driver on the router.
    }

    method reload {} {
	# Just reboot for now.
	$self reboot
    }

    method restart {args} {
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
	if {$options(-defer_restart)} {
	    UTF::Message LOG $options(-name) "Deferred restart"
	    return
	}
	if {[regexp {4908} [$self cget -model]]} {
	    if {[llength $qargs]} {
		$self nvram commit
	    }
	    $self /sbin/rc restart
	    $base configure -initialized 0
	    UTF::Sleep 10
	} else {
	    # On 63138 nvram commit automatically restarts, but keep
	    # "restart" keyword for compatibility.
	    $self nvram commit restart
	    $base configure -initialized 0
	    UTF::Sleep 3
	}
	if {$options(-wlinitcmds) ne ""} {
	    $self rexec [string trim $options(-wlinitcmds)]
	}
	if {[info exists UTF::DSLDtimDebug]} {
	    if {[set ret [$self wl -i wl0 dtim]] != 3 && [$self wl -i wl0 ap]} {
		error "wl0 dtim = $ret"
	    } elseif {![catch {$self wl -i wl0.1 dtim} ret] && $ret != 3} {
		error "wl0.1 dtim = $ret"
	    }
	}
	return
    }

    UTF::doc {
	# [call [arg host] [method reboot]]

	# Reboot DSL Router.

    }

    method reboot {args} {
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
	if {$qargs ne {}} {
	    $self nvram commit
	}
	$self rexec -x reboot
	$base configure -initialized 0
	UTF::Sleep 60
	if {$options(-wlinitcmds) ne ""} {
	    $self rexec [string trim $options(-wlinitcmds)]
	}
    }

    UTF::doc {
	# [call [arg host] [method copyto] [arg src] [arg dest]]

	# Copy local file [arg src] to [arg dest] on [arg host].  Auto
	# uncompression is supported.
    }

    method copyto {src dst} {$self install_ushd
	$self install_ushd
	if {[$self cget -ssh] ne "ush"} {
	    error "ush failed - copyto cannot continue"
	}
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

	# Copy file [arg src] on [arg host] to local [arg dest]
	# If src ends in .gz, but dest doesn't then the file will
	# compressed automatically.
    }
    method copyfrom {src dest} {
	# If we're asked to copy a compressed file to an uncompressed
	# file then uncompress it.  Handle the uncompression at the
	# remote end to avoid permissions problems.
	if {[file extension $dest] eq ".gz" &&
	    [file extension $src] ne ".gz"} {
	    set cmd "gzip -c $src"
	} else {
	    set cmd "cat $src"
	}

	# process file in 10k chunks
	set in [$self rpopen -2 $cmd]
	set out [open $dest w]
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

	# Return IP address of device [arg dev].  Device is assumed to
	# be on the main bridge.
    }

    method ipaddr {dev} {
	if {[regexp {inet addr:([0-9.]+)} [$self -s ifconfig br0] - addr]} {
	    return $addr
	} else {
	    error "No IP address available"
	}
    }

    UTF::doc {
	# [call [arg host] [method restore_defaults]
	#      [lb][option -noerase][rb] [lb][option -nosetup][rb]]

	# Restore defaults

    }
    method restore_defaults {args} {
	UTF::Getopts {
	    {noerase "ignored"}
	    {nosetup "Don't apply local setup"}
	    {n "Don't apply - just return settings"}
	}
	# Clear cached data
	foreach S $stas {
	    $S configure -ssid {} -security {} -wepkey {} -wepidx {} -wpakey {}
	}
	set nvram ""
	if {$options(-fwoverlay) && [regexp {4908} [$self cget -model]]} {
	    set fwovernv {
		firmware_path0="/data/rtecdc.bin"
		firmware_path1="/data/rtecdc.bin"
		firmware_path2="/data/rtecdc.bin"
	    }
	} else {
	    set fwovernv {}
	}
	if {!$(nosetup)} {
	    # Merge config nvram with built-in defaults
	    foreach a [concat \
			   $fwovernv \
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
	if {1 || !$(noerase)} {
	    # always erase for now
	    if {[regexp {4908} [$self cget -model]]} {
		#$self rm -f /data/.kernel_setting.nvram
		$self erase nvram
		$self sync
		# Reboot with empty nvram to pick up defaults
		$self reboot
		if {$nvram ne ""} {
		    # Set our defaults and reboot again
		    $self reboot {*}$nvram
		}
	    } else {
		$self open_messages
		$self dslshell dslsh restoredefault
		$base configure -initialized 0
		UTF::Sleep 40
		if {$nvram ne ""} {
		    # set our defaults
		    $self restart {*}$nvram
		}
	    }
	}
	if {$nvram ne ""} {
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


    variable msgfile

    # Semaphore to prevent power cycle storms
    variable processingpowercycle false
    variable rtecapture
    variable interruptedbydongle ""

    # Internal callback for fileevent below
    method getdata {request} {
	set fd $msgfile($request)
	if {![eof $fd]} {
	    set msg [gets $fd]

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

	    if {[regexp {assert_type|Upload and compare succeeded} $msg]} {
		# This is not an assert, so log and skip other checks
		UTF::Message LOG $options(-name) $msg
	    } elseif {[regexp {Detected firmware trap/assert} $msg]} {
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
	    } elseif {[regexp {Kernel panic*|ASSERT|BUG: |^PC is at } $msg]} {
		$self worry $msg
		# Reestabish connection
		$base configure -initialized 0
	    } elseif {[regexp {potentially unexpected fatal } $msg]} {
		$self worry $msg
		UTF::Sleep 2
		$base ls -l /data
	    } elseif {[regexp {tftpd:error} $msg]} {
		set tftpdmsg $msg
		UTF::Message WARN $options(-name) $msg
	    } elseif {[regexp {: page allocation failure|oom-killer: |No RxAssignedBDs} $msg]} {
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
	    } else {
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

    method open_messages { {file ""} } {
	if {$file == ""} {
	    set file $options(-console)
	}
	if {[info exists msgfile($file)]} {
	    return
	}
	UTF::Message LOG $options(-name) "Open $file"
	if {[catch {$self serialrelay socket $file} ret]} {
	    $self worry "$file: $ret"
	    return
	}
	set msgfile($file) $ret
	$base configure -lan_ip $file
	fconfigure $msgfile($file) -blocking 0 -buffering line
	puts $msgfile($file) ""; # Trigger NPC telnet reconnect
	fileevent $msgfile($file) readable [mymethod getdata $file]
	UTF::Sleep 1
	UTF::Message LOG $options(-name) "Opened $file $ret"
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
	if {[info exists msgfile($file)] &&
	    [file channels $msgfile($file)] ne ""} {
	    UTF::Message LOG $options(-name) "Close $file $msgfile($file)"
	    if {[set pid [pid $msgfile($file)]] ne ""} {
		# Processes to wait for
		catch {eval exec kill $pid} ret
		UTF::Message LOG $options(-name) $ret
	    }
	    # Leave non-blocking for safer close.  Any error messages
	    # should have been collected earlier.
	    close $msgfile($file)
	    unset msgfile($file)
	    $base configure -lan_ip $options(-lan_ip)
	    $base configure -initialized 0
	} else {
	    UTF::Message LOG $options(-name) "Close $file (not open)"
	}
    }


    # Dongle methods

    method hndrte_exe {} {
	if {![info exists hndrte_exe]} {
	    regsub {/} $options(-name) {.} name
	    set hndrte_exe [$self relay rexec -noinit -s -q \
				ls -l /tmp/$name-hndrte-exe.lnk]
	    regsub {.*-> } $hndrte_exe {} hndrte_exe
	}
	set hndrte_exe
    }

    method findtrap {trap stack} {
	UTF::RTE::findtrap $options(-name) $trap $stack [$self hndrte_exe]
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
	$self rte_trash_symbol "wlc_scan"
	$self wl -i $options(-device) up
	$self wl -i $options(-device) scan
    }


    variable processingDongleError false
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


    method version {} {
	set page [$self http get info.html]
	if {[regexp {Wireless Driver Version:</td>[^<]*<td>([\d.]+)</td>} \
	     $page - v]} {
	    return $v
	} else {
	    UTF::Message LOG $options(-name) $page
	    error "Version not found in info.hmtl"
	}
    }

    method fetch_crashreport {} {
	if {[info exists ::UTF::Logdir]} {
	    if {![catch {$self -n test -e /data/config.tgz}]} {
		if {[catch {
		    set f [exec mktemp $UTF::Logdir/config_XXXXX].tgz
		    $self copyfrom /data/config.tgz $f
		    file attributes $f -permissions go+r
		    UTF::Message WARN $options(-name) \
			"config.tgz upload: [UTF::LogURL $f]"
		    $self -n rm /data/config.tgz
		} ret]} {
		    UTF::Message WARN $options(-name) $ret
		}
	    }
	}
    }

    method init {} {
	$self open_messages
	$self install_ushd
	$self fetch_crashreport
    }

    method deinit {} {
	$self close_messages
	$self configure -initialized 0
    }

    # Configure persistent settings
    method setup {} {
	$self dslsh lan config --ipaddr primary $options(-lan_ip) 255.255.255.0
	$self dslsh lan config --dhcpserver disable
	$self dslsh save
	$self rexec "echo 1 > /proc/sys/kernel/panic_on_oops"
    }


    UTF::doc {
	# [call [arg host] [method dslshell] [arg args]]

	# Run cmdline [arg args] on the [arg host] console directly,
	# bypassing [cmd ushd].
    }

    method dslshell {args} {
	$self relay rexec -t 5 -s \
	    $UTF::usrutf/dslshell -$options(-console) $args
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

	if {[regexp {4908} [$self cget -model]]} {
	    set toolchain "$::UTF::projtools/linux/BCG/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25"
	    set LDLP "${toolchain}/usr/lib"
	    set gcc "${toolchain}/usr/bin/arm-buildroot-linux-gnueabi-gcc"
	    set ushd /tmp/ushd_49xx_$::tcl_platform(user)
	} else {
	    set toolchain "$::UTF::projtools/linux/BCG/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL"
	    set LDLP "${toolchain}/usr/lib"
	    set gcc "${toolchain}/usr/bin/arm-unknown-linux-uclibcgnueabi-gcc -static"
	    set ushd /tmp/ushd_dsl_$::tcl_platform(user)
	}
	set ushd_c $UTF::unittest/src/ushd.c

	$self open_messages

	set epi_ttcp [$self relay cget -epi_ttcp]

	if {[catch {$self relay ping $ip -c 10} ret]} {
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
		# try sysrq to see if there is any life
		$self dslshell "^l"
		UTF::Sleep 1

		$self power cycle
		UTF::Sleep 20
	    } else {
		UTF::Sleep 5
	    }
	    if {[catch {$self relay ping -c 10 $ip}]} {
		if {0 && $options(-rebootifnotpingable)} {
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
	    [catch {$self relay -t 1 $ssh $ip :}]} {
	    # Recompile the server, if neccessary
	    if {($(force) ||
		      [catch {file mtime $ushd} ret] ||
		      [file mtime $ushd_c] > $ret)} {
		UTF::Message LOG $options(-name) "$gcc -o $ushd $ushd_c"
		set ::env(LD_LIBRARY_PATH) $LDLP
		exec {*}$gcc -o $ushd $ushd_c
		unset ::env(LD_LIBRARY_PATH)

	    }

	    # Kill off any existing service, also exit in case we
	    # interrupted the boot sequence
	    catch {$self rexec -t 2 {killall ushd}} ret
	    if {[regexp {prompt detected} $ret]} {
		error $ret
	    }
	    catch {$self rexec -t 2 exit} ret
	    if {$(force) || [catch {$self test -s /data/ushd}]} {
		# Use epi_ttcp to copy the rshd executable to the router
		set fd [$self rpopen {epi_ttcp -r -w5000>/data/ushd}]
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
		    UTF::Message WARN $options(-name) $ret
		}
		# Try starting ushd even if copy appeared to fail, since
		# console data loss may falsify the copy status.
		$self rexec -t 10 "chmod +x /data/ushd;/data/ushd -d"
	    } else {
		$self rexec -t 10 "/data/ushd -d"
	    }
	}

	# configure
	$self configure -ssh $ssh
	$base configure -lan_ip $ip
	# clear init 2 state to re-enable auto recovery.
	$base configure -initialized 1

    }

    method UseApshell {} {
	$self configure -ssh $UTF::usrutf/dslshell
	$base configure -lan_ip $options(-console)
	# Use init 2 to prevent recursion
	$base configure -initialized 2

    }

    UTF::doc {
	# [call [arg host] [method boardname]]

	# Query device boardtype
    }

    method boardname {} {
	if {[regexp {9(\S+)} [$self cat /proc/nvram/boardid] - name]} {
	    if {[regexp {(\S+)_(P.*)} $name - name rev]} {
		return "$name $rev"
	    } else {
		return $name
	    }
	} else {
	    error "boardname not found"
	}
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

    # Peer passthroughs
    UTF::PassThroughMethod lan -lanpeer
    UTF::PassThroughMethod wan -wanpeer
    UTF::PassThroughMethod relay -relay
    UTF::PassThroughMethod serialrelay -serialrelay

    method whatami {{STA ""}} {
	if {[catch {$self boardname} b]} {
	    set b "<unknown>"
	}
	if {$STA ne ""} {
	    if {[catch {$STA chipname} c]} {
		set c "<unknown>"
	    }
	    if {$c ne $b} {
		set b "$b/$c"
	    }
	}
	return "DSL $b"
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
	    {s.arg "56" "Size"}
	} args
	if {[info commands $target] == $target} {
	    set target [$target ipaddr]
	}
	$self rexec ping -q -c 1 -w $(c) -s $(s) $target
    }


    UTF::doc {
	# [call [arg host] [method {http upgrade}] [arg file]]

	# Ugrade Router from the specified firmware [arg file].
	# Success or Failure may not be reported.  Requires the device
	# be at the CFE> prompt.
    }

    method {http upgrade} {file} {
	set filename [file tail $file]
	$self relay rexec "curl -f -u 'admin:$options(-passwd)' -F 'file=@\"$file\";filename=\"$filename\"' http://$options(-lan_ip)/upgrade.cgi"
    }

}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
