#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::Cygwin 2.0

package require snit
package require UTF::doc
package require UTF::Base

UTF::doc {
    # [manpage_begin UTF::Cygwin n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Cygwin/Windows support}]
    # [copyright {2005 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::Cygwin is an implementation of the UTF host object,
    # specific to Windows systems running Cygwin.

    # Once created, the Cygwin object's methods are not normally
    # invoked directly by test scripts, instead the Cygwin object is
    # designated the host for one or more platform independent STA
    # objects and it will be the STA objects which will be refered to
    # in the test scripts.

    # [list_begin definitions]

}

snit::macro UTF::DevCache {name} {
    method $name {{key "all"}} "
	    \$self probe

	    if {\$key eq \"\"} {
                set key \$defnum
            }
	    if {\$key ne \"all\"} {
		return \$${name}(\$key)
	    } else {
                return \[array get $name\]
            }
	"
}

snit::type UTF::Cygwin {
    UTF::doc {

	# [call [cmd UTF::Cygwin] [arg host]
	#	[option -lan_ip] [arg address]
	# 	[lb][option -sta] [arg {{STA dev ...}}][rb]
	# 	[lb][option -ssh] [arg path][rb]
	#       [lb][option -image] [arg driver][rb]
	#       [lb][option -brand] [arg brand][rb]
	#       [lb][option -tag] [arg tag][rb]
	#       [lb][option -type] [arg type][rb]
	#       [lb][option -date] [arg date][rb]
	#       [lb][option -kdpath] [arg path][rb]
        #       [arg ...]]

	# Create a new Windows/Cygwin host object.  The host will be
	# used to contact STAs residing on that host.
	# [list_begin options]

	# [opt_def [option -sta] [arg {{STA dev ...}}]]

	# List of STA devices to configure for this host.  If there is
	# more than one wireless interface on the system, the devices
	# should be listed as [arg {name device}] pairs.  If there is
	# only one wireless device on the system, the device number can
	# be omitted.

	# [example_begin]
	-sta {WL0 0 WL1 1}
	-sta {WL}
	# [example_end]

	# [opt_def [option -lan_ip] [arg address]]

	# IP address to be used to contact host.  This should be a
	# backbone address, not involved in the actual testing.

	# [opt_def [option -ssh] [arg path]]

	# Specify an alternate command to use to contact [arg host],
	# such as [cmd rsh] or [cmd fsh].  The default is [cmd ssh].

	# [opt_def [option -image] [arg driver]]

	# Specify a default driver to install when [method load] is
	# invoked.  This can be an explicit path to a [file Setup.exe]
	# or [file bcmwl5.sys] file, or a suitable list of arguments
	# to [method findimages].

	# [opt_def [option -brand] [arg brand]]

	# Specify build brand.  Default is [file win_internal_wl].

	# [opt_def [option -tag] [arg tag]]

	# Select a build tag.  Default is [file trunk].

	# [opt_def [option -type] [arg type]]

	# Select a build type.  Build types are [file checked] or
	# [file free].  Default is [file checked].

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2005.8.17.0].  Default is to
	# return the most recent build available.

	# [opt_def [option -kdpath] [arg path]]

	# Specify a path for the Windows kernel debugger tool.
	# Default is {C:\Program Files\Debugging Tools for
	# Windows\kd.exe}

	# [opt_def [arg ...]]

	# Unrecognised options will be passed on to the host's [cmd
	# UTF::Base] object.

	# [list_end]
	# [list_end]

	# [para]
	# Cygwin objects have the following methods:
	# [para]
	# [list_begin definitions]

    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -epi_ttcp "epi_ttcp_c"
    option -image
    option -sta
    option -name -configuremethod CopyOption
    option -dbgview
    option -tag "trunk"
    option -date "%date%"
    option -app_tag "trunk"
    option -app_date "%date%"
    option -app_brand ""
    option -type "checked"
    option -sys
    option -cat; # Catalogue file, used for signing drivers
    option -sign -type snit::boolean -default false
    option -osver -default 5 \
	-type {snit::enum -values {5 564 6 664 7 764 8 864 81 8164 9 964 10 1064}}
    option -installer inf
    option -wlinitcmds
    option -postinstall
    option -postinstall_hook
    option -preunload_hook
    option -altsys
    option -msgcallback
    option -console "/var/log/messages"
    option -reg
    option -node {PCI|USB|SD}; # Exclude virtual nodes
    option -dhcpretries 3
    option -debuginf -type snit::boolean -default true; # unused
    option -allowdevconreboot -type snit::boolean -default false
    option -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}
    option -mfgcpath {C:\Program Files\Echo_2.4.23}
    option -mfgcscriptdir {`cygpath -wD`/dvtc_scripts}
    option -mfgcscript "goldenRefScript_agn.txt"
    option -mfgcref ""
    option -mfgcusecc -type snit::boolean -default false
    option -coreserver
    option -usemodifyos -type snit::boolean -default false
    option -deleteonboot -type snit::boolean -default false
    option -memorydumpfile {$WINDIR/MEMORY.DMP}
    option -tcptuneserver -type snit::boolean -default true
    option -embeddedimage
    option -wdiwifisys

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

    variable reg {MsgLevel 0x101 IBSSMode 5 EnableAutoConnect 0}
    variable probed 0
    variable CaptureDevice
    variable Interfaces
    variable Nodes
    variable PCI
    variable MAC
    variable SYS
    variable RegCache
    variable Devices -array {
	setup.exe,node {}
    }
    variable driverdir "/tmp/DriverOnly"
    variable archive; # Initialize in constructor so default can be overridden.

    # base handles any other options and methods
    component base -inherit yes

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    constructor {args} {
	set cloneopts $args
	set archive $::UTF::projarchives/win/UTF
	install base using UTF::Base %AUTO% -init [mymethod init] \
	    -user user -brand "win_internal_wl" -nointerrupts auto
	$self configurelist $args
	if {$options(-name) eq ""} {
	    $self configure -name [namespace tail $self]
	}

	# default sys name
	if {$options(-sys) eq ""} {
	    set options(-sys) [$self defsysfile]
	}
	# default cat name
	if {$options(-cat) eq ""} {
	    if {[regexp {64$} $options(-osver)]} {
		set options(-cat) "bcm43xx64.cat"
	    } else {
		set options(-cat) "bcm43xx.cat"
	    }
	}
	if {![regexp {^[5-7]} $options(-osver)]} {
	    # Shared WEP not supported after Win7
	    $self configure -nosharedwep 1
	}
	if {![regexp {^[5-9]} $options(-osver)]} {
	    # IBSS not supported from Win10
	    $self configure -noibss 1
	}

	foreach {sta dev} $options(-sta) {
	    # Windows reports MAC addresses in uppercase.  Empty or
	    # numeric device names will be unaffected.
	    UTF::STA ::$sta -host $self -device [string toupper $dev]
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
	# [call [arg host]  [method services] [arg [lb]start|stop[rb]]]

	# Start or Stop Tray App and Zero Configuration services.
    }

    method services {op} {
	switch $op {
	    "stop" {
		set want STOPPED
		set start DISABLED
	    }
	    "start" {
		set want RUNNING
		set start AUTO
	    }
	    default {
		error "usage: services \[stop|start\]"
	    }
	}
	set ret ""

	catch {$self rexec sc config wltrysvc start= $start}
	catch {$self rexec sc config wzcsvc start= $start}
	foreach s { wltrysvc wzcsvc } {
	    if {[$self rexec \
		     "sc query $s | awk '/STATE/{print \$4}'"] != $want} {
		lappend ret [$self rexec \
				 "sc $op $s | awk '/STATE/{print \$4}'"]
	    }
	}
	return $ret
    }

    UTF::doc {
	# [call [arg host] [method devcon] [arg op][arg dev]]

	# Control devices on [arg host] using [cmd devcon].  If
	# necessary, [arg host] will be probed to discover device
	# mappings.
    }


    method devcon {dev op} {
	$self rexec "devcon $op '@[$self Nodes $dev]'"
    }

    # http://support.microsoft.com/kb/310123
    # http://technet.microsoft.com/en-us/library/hh240923%28v=ws.10%29.aspx
    typevariable devconstatus -array {
	1 "This device is not configured correctly."
	3 "The driver for this device might be corrupted, or your system may\
be running low on memory or other resources."
	10 "This device cannot start."
	12 "This device cannot find enough free resources that it can use. If\
you want to use this device, you will need to disable one of the other devices\
on this system."
	14 "This device cannot work properly until you restart your computer."
	16 "Windows cannot identify all the resources this device uses."
	18 "Reinstall the drivers for this device."
	19 "Windows cannot start this hardware device because its\
configuration information \(in the registry) is incomplete or damaged. To fix\
this problem you can first try running a Troubleshooting Wizard. If that does\
not work, you should uninstall and then reinstall the hardware device."
	21 "Windows is removing this device."
	22 "This device is disabled."
	24 "This device is not present, is not working properly, or does not\
have all its drivers installed."
	28 "The drivers for this device are not installed."
	29 "This device is disabled because the firmware of the device did not\
give it the required resources."
	31 "This device is not working properly because Windows cannot load\
the drivers required for this device."
	32 "A driver (service) for this device has been disabled. An alternate\
driver may be providing this functionality."
	33 "Windows cannot determine which resources are required for this\
device."
	34 "Windows cannot determine the settings for this device. Consult\
the documentation that came with this device and use the Resource tab to set\
the configuration."
	35 "Your computer's system firmware does not include enough\
information to properly configure and use this device. To use this device,\
contact your computer manufacturer to obtain a firmware or BIOS update."
	36 "This device is requesting a PCI interrupt but is configured for\
an ISA interrupt \(or vice versa). Please use the computer's system setup\
program to reconfigure the interrupt for this device."
	37 "Windows cannot initialize the device driver for this hardware."
	38 "Windows cannot load the device driver for this hardware because\
a previous instance of the device driver is still in memory."
	39 "Windows cannot load the device driver for this hardware. The\
driver may be corrupted or missing."
	40 "Windows cannot access this hardware because its service key\
information in the registry is missing or recorded incorrectly."
	41 "Windows successfully loaded the device driver for this hardware\
but cannot find the hardware device."
	42 "Windows cannot load the device driver for this hardware because\
there is a duplicate device already running in the system."
	43 "Windows has stopped this device because it has reported problems."
	44 "An application or service has shut down this hardware device."
	45 "Currently, this hardware device is not connected to the computer."
	46 "Windows cannot gain access to this hardware device because the\
operating system is in the process of shutting down."
	47 "Windows cannot use this hardware device because it has been\
prepared for safe removal, but it has not been removed from the computer."
	48 "The software for this device has been blocked from starting\
because it is known to have problems with Windows. Contact the hardware\
vendor for a new driver."
	49 "Windows cannot start new hardware devices because the system\
hive is too large \(exceeds the Registry Size Limit)."
	51 "This device is currently waiting on another device or set of devices to start."
	52 "Windows cannot verify the digital signature for the drivers required for this device."
    }

    method devcon_status {node} {
	set ds [$self rexec -n "devcon status '$node'"]
	if {[regexp {The device has the following problem: (\d+)} $ds \
		 - status] && [info exists devconstatus($status)]} {
	    append ds "\n$devconstatus($status) (Code $status)"
	    UTF::Message WARN $options(-name) \
		"$devconstatus($status) (Code $status)"
	}
	return $ds
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

	# Build types are [file checked] or [file free] for internal
	# builds and are vendor specific, eg, [file Bcm] or [file
	# Dell] for external builds.  Default is [option -type] option
	# of [arg host].

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2005.8.17.0].  Default is to
	# return the most recent build available.

	# [opt_def [arg file]]

	# Specify the file type being searched for.

	# [arg file] should be the name of the driver [file .sys]
	# file, eg [file bcmwl5.sys], or the name of the install
	# shield folder, eg [file InstallShield] or [file
	# Neptune_InstallShield].  If [arg file] is a folder other
	# than an InstallShield folder then it is assumed to be a
	# user's private source tree and will be searched accordingly.

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
	    {web.arg "" "ignored"}
	    {brand.arg "[$self cget -brand]" "brand"}
	    {tag.arg "$options(-tag)" "Build Tag"}
	    {date.arg "$options(-date)" "Build Date"}
	    {sys.arg "$options(-sys)" "Install file"}
	    {type.arg "$options(-type)" "Build Type"}
	    {installer.arg "$options(-installer)" "Installer"}
	    {app_tag.arg "$options(-app_tag)" "Build Tag"}
	    {app_date.arg "$options(-app_date)" "Build Date"}
	    {app_brand.arg "$options(-app_brand)" "Application Build Brand"}
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
	set platform windows

	if {$file eq ""} {
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
	} elseif {![regexp {InstallShield} $file] && [file isdirectory $file]} {
	    # Look for a driver, assuming this is part of a developer
	    # workspace.
	    if {[regexp {win_} $brand]} {
		return [glob "$file{{{{{{/wl,}/sys,}/wdm,}/build*,}/objchk_*,}/*,}/$(sys)"]
	    } else {
		return [glob "$file{{{{{/wl,}/sys,}/wdm,}/obj/win8x_nic/checked}/*,}/$(sys)"]
	    }
	} elseif {[file exists $file]} {
	    return $file
	}

	switch -re [$self cget -osver] {
	    {^5} {
		set release "release{/WinXP,}"
	    }
	    {^[67]} {
		set release "release/{Win7,WinVista}"
	    }
	    default {
		set release "release/Win*"
	    }
	}

	switch -g $file {
	    rtecdc.exe {
		if {[regexp {\.sys} $saved_args] ||
		    [file isdirectory [lindex $saved_args 0]]} {
		    # For a sys install we look in the same sys.  Same
		    # with a developer build.
		    set sys [$self findimages \
				 {*}[lreplace $saved_args end end]]
		} else {
		    # For other install we look in the build
		    set sys [$self findimages {*}$saved_args *.sys]
		}

		if {[catch {localhost rexec \
				"strings -a $sys |\
                                grep -x \"43\[-\[:alnum:]]*/\[-\[:alnum:]]*\""} type]} {
		    UTF::Message WARN $options(-name) $type
		    UTF::Message WARN $options(-name) "imagename not found."
		}

		# Windows drivers can include multiple embedded
		# images.  Use -embeddedimage to specify the correct
		# one.  We can't do this automatically since that
		# would involve runtime checks that may not be
		# possible on a crashed device.  This is checked as a
		# regexp, so you only need to specify just enough to
		# idendify the chip, not the whole firmware image.
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
		set brand hndrte-dongle-wl
		set platform linux
		set tail "src/dongle/rte/wl/builds/$type/$file"
	    }
	    wl.exe {
		set brand $(app_brand)
		set tag $(app_tag)
		set date $(app_date)
		if {[regexp {Internal} $(type)]} {
		    set tail $file
		} elseif {[regexp {mfgtest} $brand]} {
		    set tail [file join $(type)_Apps $file]
		} else {
		    set tail [file join DriverOnly $file]
		}
	    }
	    *.inf -
	    *.sys {
		if {[regexp {internal} $brand]} {
		    set tail [file join DriverOnly $file]
		} elseif {$(type) ne "Internal"} {
		    set tail [file join $(type)_DriverOnly $file]
		} else {
		    set tail $file
		}
	    }
	    brcm_wlu.dll {
		# Override brand and type since it's only in mfgtest Bcm
		set (type) "Bcm"
		set brand "win_mfgtest_wl"
		set tail [file join $(type)_Apps $file]
	    }
	    InstallShield {
		if {[regexp {internal} $brand]} {
		    set tail [file join $file {[Ss]etup.exe}]
		} else {
		    set tail [file join $(type)_$file {[Ss]etup.exe}]
		}
	    }
	    default {
		error "$file: unknown driver file (expected *.sys, *.inf or InstallShield)"
	    }
	}

	if {[regexp {_REL_} $tag]} {
	    set tag "{PRESERVED/,ARCHIVED/,}$tag"
	} else {
	    set tag "{PRESERVED/,}$tag"
	}
	if {$platform eq "windows"} {
	    set tail [file join $release $(type) $tail]
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

    variable reloadlock 0
    method reload {} {

	try {
	    # Avoid looping if the driver asserts immediately
	    if {[incr reloadlock] != 1} {
		return
	    }
	    UTF::Message INFO $options(-name) "Reload"

	    $self openSetupLogs

	    # Make sure mfgtest isn't still running
	    $self pkill mfcremote.exe mfcmfgc.exe

	    foreach node [$self Devices "any,node"] {
		$self rexec -e 30 -d "devcon disable" -t 120 \
		    "devcon disable '@$node'"
	    }
	    UTF::Sleep 1
	    set check_nodes {}
	    foreach node [$self Devices "any,node"] {
		if {[regexp -nocase $options(-node) $node]} {
		    lappend check_nodes $node
		    $self rexec -e 30 -d "devcon enable" -t 120 \
			"devcon enable '@$node'"
		}
	    }
	    if {$options(-postinstall) ne ""} {
		$self rexec $options(-postinstall)
	    }
	    UTF::forall {STA dev} [$self cget -sta] \
		cmd [$self cget -postinstall_hook] {
		    if {[catch [string map [list %S $STA] $cmd] ret]} {
			UTF::Message WARN $STA $ret
		    }
		}
	    UTF::Sleep 4
	    foreach node $check_nodes {
		$self devcon_status "@$node"
	    }
	    if {$options(-wlinitcmds) ne ""} {
		$self rexec [string trim $options(-wlinitcmds)]
	    }
 	    $self closeSetupLogs
	    $self probe clear
	}  finally {
	    incr reloadlock -1
	}
    }

    UTF::doc {
	# [call [arg host] [method sign]]

	# Sign a wl driver file specified in [arg host]s option
	# [option -sys] with catalog file from option [option -cat].
	# Files are located in directory $driverdir.
    }
    method sign {} {
	# Translate osver into format used by signing tool
	if {![regsub {64$} $options(-osver) {_X64} signos]} {
	    set signos $options(-osver)_X86
	}
	regsub {81|9|10} $signos {8} signos; # Win8.1 and Win9 use Win8 signatures
	UTF::Message LOG $options(-name) "Signing driver in $driverdir"
	if {[info exists UTF::TestSigning]} {
	    if {[catch {$self test -f UTFTestCert.cer}]} {
		$self SetupCert
	    }
	    set d [$self cygpath -w $driverdir]
	    $self inf2cat '/driver:$d' /os:$signos
	    $self signtool sign /s "'UTF Test Certificate'" {$(cygpath -w /tmp/DriverOnly/*.{cat,sys})}

	} else {
	    $self rexec -n -t 180 ./sign.sh \
		$driverdir/$options(-cat) $driverdir/$options(-sys) $signos
	}
    }


    method installdriver {{dir "/tmp/DriverOnly"}} {
	set id "/projects/hnd_sig_ext13/UTF_Ready_OS_Images/Tools/InstallDriver"

	# check for and kill prior instances
	$self pkill InstallDriver

	if {[catch {$self sum ./InstallDriver.exe} ret] ||
	    $ret ne [exec sum $id]} {
	    $self copyto $id InstallDriver.exe
	}

	foreach {k v} [concat $reg [UTF::decomment $options(-reg)]] {
	    if {$v eq "unset"} {
		lappend delreg $k
	    } else {
		lappend addreg "$k $v"
	    }
	}

	# Clean out old drivers to make sure Windows doesn't try to
	# reinstall them in preference to the one being requested.
	$self rm -f \
	    [$self cygpath -S]/DriverStore/FileRepository/*/$options(-sys)

	set cmd "TEMP=/tmp ./InstallDriver.exe"
	append cmd " path='[$self cygpath -w $dir]'"
	if {$options(-sign)} {
	    append cmd " sign=true"
	}
	if {[info exists addreg]} {
	    append cmd " addreg='[join $addreg {;}]'"
	}
	if {[info exists delreg]} {
	    append cmd " delreg='[join $delreg {;}]'"
	}
	if {![string match "*|*" $options(-node)]} {
	    append cmd " devicefilter='*$options(-node)*'"
	    append cmd " disableallotherwlancards=yes"
	}
	if {$options(-deleteonboot)} {
	    append cmd " removewinpeprotection=false"
	}
	append cmd " setupapilog=yes"
	if {[catch {
	    $self -t 360 -T 420 $cmd
	} ret]} {
	    set e $::errorInfo
	    UTF::Message WARN $options(-name) $::errorCode
	    error [lindex [split $ret "\n"] end]
	}
	if {$options(-postinstall) ne ""} {
	    $self rexec $options(-postinstall)
	}
	UTF::forall {STA dev} [$self cget -sta] \
	    cmd [$self cget -postinstall_hook] {
		if {[catch [string map [list %S $STA] $cmd] ret]} {
		    UTF::Message WARN $STA $ret
		}
	    }
	if {$options(-wlinitcmds) ne ""} {
	    $self rexec [string trim $options(-wlinitcmds)]
	}
    }

    UTF::doc {
	# [call [arg host] [method hassysnative]]

	# Returns true if the Windows/sysnative pseudo filesystem
	# exists and should be used.  This will only be the case if
	# you are running a 32bit version of Cygwin on a 64bit version
	# of Windows
    }

    method hassysnative {} {
	variable _hassysnative
	if {[info exists _hassysnative]} {
	    return $_hassysnative
	}
	if {[regexp {64} $options(-osver)] &&
	    ![catch {$self rexec test -d \$(cygpath -W)/sysnative}]} {
	    set _hassysnative 1
	} else {
	    set _hassysnative 0
	}
    }

    UTF::doc {
	# [call [arg host] [method load] [lb][arg file][rb]]
	# [call [arg host] [method load] [lb][arg {args ...}][rb]]

	# Load or replace the wl driver.  In the first form [arg file]
	# should be the pathname of a [file Setup.exe], in which case
	# it will be installed, or a [file BCMWL5.SYS] file, where
	# only that file will be updated.  In the second form, the
	# argument list will be passed on to [method findimages] to
	# find a driver.  If no arguments are specified, those stored
	# in the [arg host]s [option -image] option will be used
	# instead.  Filenames are relative to the control host and
	# files will be copied to [arg host] as needed.  If a version
	# of [syscmd wl.exe] is found with the new driver, the [syscmd
	# wl.exe] command on [arg host] will be updated accordingly.
    }

    variable cygpath -array {}
    method cygpath {args} {
	if {[info exists cygpath($args)]} {
	    return $cygpath($args)
	}
	set p [$self rexec -n cygpath {*}$args]
	if {[$self hassysnative]} {
	    # Bypass WoW64's System32 remapping
	    regsub -nocase {\mSystem32\M} $p {sysnative} p
	}
	set cygpath($args) $p
    }

    # Save exe reference, in case host is down when we want to analyse
    # a crash
    variable hndrte_exe

    method load {args} {

	# Note -altsys gets stripped from options(-image) as well,
	# since it is needed for load, but not for findimages
	UTF::GetKnownopts [subst {
	    {force "ignored"}
	    {sign.arg "$options(-sign)" "Sign driver"}
	    {n "Just copy the files, don't actually load the driver"}
	    {altsys.arg "[from options(-image) -altsys $options(-altsys)]" "Alternate sys file"}
	    {wdiwifisys.arg "[from options(-image) -wdiwifisys [$self cget -wdiwifisys]]" "Alternate service sys file"}
	    {all "ignored"}
	    {ls "ignored"}
	}]

	UTF::Message INFO $options(-name) "Load Windows Driver"
	set file [$self findimages {*}$args]
	UTF::Message LOG $options(-name) "Found $file"

	if {$(altsys) ne ""} {
	    set altsys [$self findimages {*}$args $(altsys)]
	    UTF::Message LOG $options(-name) "Alternate sys $altsys"
	} else {
	    set altsys ""
	}

	if {$(wdiwifisys) ne ""} {
	    $self copyto $(wdiwifisys) "$driverdir/WdiWiFi.sys"
	}

	if {[$self cget -usedll]} {
	    # Also need MFGTEST DLL
	    set dll [$self findimages {*}$args brcm_wlu.dll]
	    UTF::Message LOG $options(-name) "DLL $dll"
	} else {
	    set dll ""
	}

	if {[$self cget -app_tag] ne ""} {
	    set wl [$self findimages {*}$args wl.exe]
	    UTF::Message LOG $options(-name) "wl $wl"
	} else {
	    set wl ""
	}

	catch {$self rm -f /tmp/hndrte-exe.lnk*}
	if {$options(-embeddedimage) ne ""} {
	    if {[catch {$self findimages {*}$args rtecdc.exe} ret]} {
		UTF::Message WARN $options(-name) "Symbol file not found: $ret"
	    } else {
		UTF::Message LOG $options(-name) "Symbol file: $ret"
		$self rexec "ln -s '$ret' /tmp/hndrte-exe.lnk"
		set hndrte_exe $ret
	    }
	}

	if {[string match "*etup.exe" $file] ||
	    [string match "*etup.exe.gz" $file]} {
	    set driverdir /tmp/InstallShield
	} else {
	    set driverdir /tmp/DriverOnly
	}

	# Kill off up any existing popups or stuck installs
	$self pkill WerFault InstallShield

	set f [file tail $file]
	$self rexec rm -rf $driverdir
	set repodir [file tail [file dir $file]]
	if {[regexp {Driver|InstallShield|Internal} $repodir]} {
	    $self copyto [file join [file dir $file] .] $driverdir

	    if {[file extension $file] eq ".gz"} {
		$self gunzip -r $driverdir
		regsub {\.gz$} $f {} f
	    }

	    if {$repodir eq "Internal"} {
		# MFGTEST Internal build, switch to checked_
		# versions of sys and symbols, if available,
		# otherwise fall back to free_ symbols.
		regsub {.sys} $options(-sys) {.pdb} pdb
		if {![catch {$self mv $driverdir/checked_$options(-sys) \
				 $driverdir/$options(-sys)}]} {
		    $self mv $driverdir/checked_$pdb $driverdir/$pdb
		} else {
		    $self mv $driverdir/free_$pdb $driverdir/$pdb
		}
	    }
	    if {$dll ne ""} {
		# Also copy in the MFGTEST DLL
		$self pkill mfcremote.exe mfcmfgc.exe
		$self copyto $dll "[$self cygpath -S]/brcm_wlu.dll"
	    }
	    if {$altsys ne ""} {
		if {$(sign)} {
		    # Replace the sys file before signing
		    $self copyto $altsys "$driverdir/$options(-sys)"
		    set altsys ""
		} else {
		    # Defer sys file replacement until after initial load
		    $self copyto $altsys "$driverdir/alt_$options(-sys)"
		}
		# If we have private symbols, use them
		if {[regsub {\.sys} $altsys {.pdb} pdb] && [file exists $pdb]} {
		    $self copyto $pdb $driverdir/
		}
	    }

	    if {$(sign) && [$self cget -installer] ne "InstallDriver"} {
		$self sign
	    }
	} else {
	    $self mkdir $driverdir
	    $self copyto $file $driverdir

	    # If we have private symbols, use them
	    if {[regsub {\.sys} $file {.pdb} pdb] && [file exists $pdb]} {
		$self copyto $pdb $driverdir
	    }
	}

	# Kill any running wl
	$self pkill wl.exe

	if {$wl ne ""} {
	    $self copyto $wl /usr/bin/wl.exe
	} elseif {![catch {$self test -x $driverdir/wl.exe}]} {
	    $self cp $driverdir/wl.exe /usr/bin/wl.exe
	} else {
	    # Try to find a matching wl command
	    set a [file split $file]
	    set wl [lindex \
			[glob -nocomplain \
			     [file join \
				  {*}[lreplace $a end end wl.exe]] \
			     [file join \
				  {*}[lreplace $a end-1 end wl.exe]] \
			     [file join \
				  {*}[lrange $a 0 [expr {[lsearch $a release]+1}]] \
				  Internal wl.exe]] 0]
	    if {$wl != "" && [file exists $wl]} {
		UTF::Message LOG $options(-name) "Found matching wl"
		if {[catch {$self copyto $wl /usr/bin/wl.exe} ret]} {
		    error "Copy failed: $ret"
		}
		UTF::Message LOG $options(-name) "Installed matching wl"
	    } else {
		UTF::Message WARN $options(-name) "No wl found"
	    }
	}

	if {$(n)} {
	    UTF::Message LOG $options(-name) "Copied files - not loading"
	    return
	}
	$self services stop

	if {[$self cget -installer] eq "InstallDriver"} {
	    $self installdriver /tmp/DriverOnly
	    if {$altsys ne ""} {
		# Replace sys file after load.
		$self cp $driverdir/alt_$options(-sys) \
		    [file join [$self cygpath -S] DRIVERS $options(-sys)]
		$self reload
	    }
	} else {

	$self openSetupLogs
	$self DeleteDriverOnBoot

	if {[string match "*etup.exe" $file]} {
	    $self CheckDriverSigningPolicy
	    $self cleaninfs
	    set islog {$TEMP/bcmwl.log}
	    $self open_messages $islog
	    set before [clock seconds]
	    $self rexec -t 600 $driverdir/$f /s
	    set sec [expr {[clock seconds] - $before}]
	    set msg "InstallShield completed in [format {%d:%02d} [expr {$sec/60}] [expr {$sec%60}]]"

	    if {$sec > 420} {
		UTF::Message FAIL $options(-name) $msg
		set ::UTF::panic $msg
	    } else {
		UTF::Message INFO $options(-name) $msg
	    }
	    $self close_messages $islog
	    $self services stop
	} elseif {[file extension $f] eq ".inf"} {
	    $self CheckDriverSigningPolicy

	    # Replace WdiWiFi service if requested.
	    if {$(wdiwifisys) ne ""} {
		$self -x sc stop WdiWiFi
		$self cp $driverdir/WdiWiFi.sys "[$self cygpath -S]/DRIVERS"
		$self sc start WdiWiFi
	    }

	    foreach node [$self Devices "any,node"] {
		# Strip down to hardware ID
		regsub {\\[^\\]*$} $node {} node
		if {![regexp -nocase $options(-node) $node]} {
		    # Only update matching nodes
		    continue
		}
		set found_one 1
		$self cleaninfs $node
		if {[catch {
		    # Try to remove the sys file to make it easier
		    # to recover if the unload crashes
		    set driverfiles [$self rexec \
					 "devcon driverfiles '$node'"]
		    if {[regexp {([\w:\\]+\.SYS)} $driverfiles - sys]} {
			set sys [$self cygpath "'$sys'"]
		    } else {
			# Fall back to configured value
			set sys "[$self cygpath -wS]/DRIVERS/$options(-sys)"
		    }
		    $self rexec "rm -f $sys"
		} ret]} {
		    UTF::Message LOG $options(-name) $ret
		}

		# If devcon requests a reboot that is normally a
		# failure, but sometimes we need to let it go
		# ahead and reboot anyway.
		if {$options(-allowdevconreboot)} {
		    set cmd "devcon -r update"
		} else {
		    set cmd "devcon update"
		}

		# WAR for ssh environment issues on Win10
		set cmd "TEMP=/tmp $cmd"

		if {[catch {
		    $self rexec -e 180 -d $cmd -t 240 \
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
			if {$options(-allowdevconreboot)} {
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
		if {$options(-postinstall) ne ""} {
		    $self rexec $options(-postinstall)
		}
		set driverfiles [$self rexec "devcon driverfiles '$node'"]
		if {![regexp -nocase {([\w:\\]+\.SYS)} $driverfiles - sys]} {
		    set failed "devcon failed to report driver info"
		    UTF::Message FAIL $options(-name) $failed
		    # Fall back to configured value
		    set sys "[$self cygpath -wS]\\DRIVERS\\$options(-sys)"
		}
		regsub {.*\\} $sys {} syssrc
		set syssrc [file join $driverdir $syssrc]
		# Convert to Cygwin path
		set sys [$self cygpath "'$sys'"]
		# If ModifyOS deleted the driver for safety, make sure
		# we don't put it back!
		if {([catch {$self rexec "sum $sys"} ret] && !$options(-usemodifyos)) ||
		    $ret ne [$self sum $syssrc]} {
		    set failed "devcon update failed"
		    UTF::Message FAIL $options(-name) $failed
		    $self rexec "cp $syssrc $sys"
		    set failed "devcon update failed - recovered"
		    UTF::Message INFO $options(-name) $failed
		}
	    }
	    if {![info exists found_one]} {
		error "No devices matched $options(-node)"
	    }
	    if {$altsys ne ""} {
		# Replace sys file after load.
		$self cp $driverdir/alt_$options(-sys) \
		    [file join [$self cygpath -S] DRIVERS $options(-sys)]
	    }
	} else {
	    $self cp $driverdir/$f \
		[file join [$self cygpath -S] DRIVERS $options(-sys)]
	}

	$self regsetup

	$self reload

	UTF::Sleep 10

	$self UndoDeleteDriverOnBoot
        }

	if {[catch {$self wl ver} ver]} {
	    # Sometimes seem to need another reload.  Not sure if it's
	    # Win7 or just my multi-nic system.  Add a temporary retry
	    # here to gather stats.
	    if {[regexp {No .* found} $ver]} {
		UTF::Message WARN $options(-name) "Wait for devcon to recover"
		for {set i 0} {$i < 60 && [catch {$self wl ver} ver]} {incr i} {
		    UTF::Sleep 1
		}
		if {[catch {$self wl ver} ver]} {
		    set failed "extra reload needed"
		    $self reload
		} else {
		    $self warn "recovered after $i seconds"
		    $self probe clear
		}
	    } else {
		error $ver $::errorInfo
	    }
	}

	if {[regexp {^7} $options(-osver)] &&
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

	if {![regexp {version (.*)} $ver - ver]} {
	    # only report first line of error
	    set ver [lindex [split $ver "\n"] 0]
	}
	if {[info exists failed]} {
	    # Report version and any saved failure messages
	    error "$ver $failed"
	} elseif {$interfaces == {}} {
            # Driver loaded, but probe had problems.
            error "$ver No network interfaces found"
	}
	return $ver
    }


    method DeleteDriverOnBoot {} {
	if {$options(-usemodifyos)} {
	    # Temporary transition aid
	    #$self copyto $archive/cygdrive/c/DeleteDriverOnBoot.bat /cygdrive/c/DeleteDriverOnBoot.bat
	    $self /cygdrive/c/DeleteDriverOnBoot.bat $options(-sys)
	}
    }

    method UndoDeleteDriverOnBoot {} {
	if {$options(-usemodifyos) && !$options(-deleteonboot)} {
	    $self /cygdrive/c/UndoModifyOS.bat
	}
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
            set t1 100
        }
        set t2 [string trim $t2]
        if {$t2 == ""} {
            set t2 40 ;# Dell T3400 needs more time to accept power failure is over
        }
        set cmd1 [string trim $cmd1]
        if {$cmd1 == ""} {
            set cmd1 "[$self cygpath -S]/shutdown /s /f /t 0"
        }
        set cmd2 [string trim $cmd2]
        if {$cmd2 == ""} {
            set cmd2 "[$self cygpath -S]/shutdown /r /f /t 0"
        }

        # Call the common method
        $base shutdown_reboot $t1 $t2 $cmd1 $cmd2 $args
    }


    UTF::doc {
	# [call [arg host] [method cycleSetupLogs] [arg {file1.log ...}]]
	#
	# cycle setupapi logs if they get larger than 10M
    }
    method cycleSetupLogs {logs} {
	if {[lsearch -not $logs {*.log}] >= 0} {
	    error "usage: cycleSetupLogs 'file1.log ...'"
	}
	set stamp [clock format [clock seconds] -format "%Y%m%d_%H%M%S"]
	foreach l [$self rexec -x "find $logs -size +10M 2>/dev/null"] {
	    regsub {\.log} $l ".$stamp.log" s
	    $self mv $l $s
	}
    }

    method wait_for_boot {{t 30}} {
	UTF::Sleep $t
	$self configure -initialized 0
	for {set i 0} {[catch {$self -n :}] && $i < 20} {incr i} {}
    }

    UTF::doc {
	# [call [arg host] [method unload]]

	# Unload the current wl driver.
	# Not implemented.
    }

    method unload {args} {
	UTF::Getopts {
	    {rm "Remove sys files"}
	}
	try {
	    # Avoid reloading if we crash during unload
	    incr reloadlock
 	    # Preunload
	    UTF::forall {STA dev} [$self cget -sta] \
		cmd [$self cget -preunload_hook] {
		    if {[catch [string map [list %S $STA] $cmd] ret]} {
			UTF::Message WARN $STA $ret
		    }
		}
	    $self openSetupLogs
	    if {$(rm) && [catch {$self Devices "any,node"}]} {
		# No devices found - try to delete the configured sys
		# file anyway.
		$self rm -f [file join [$self cygpath -S] DRIVERS $options(-sys)]
	    } else {
		foreach node [$self Devices "any,node"] {
		    set sys [file join [$self cygpath -W] [$self SYS $node]]
		    # move .sys file out of the way, to aid recovery if
		    # unload crashes.
		    if {$(rm)} {
			$self rm -f $sys
			set missing 1
		    } else {
			set missing [catch {$self mv $sys "$sys.off"}]
		    }
		    $self sync; # always sync before a potential crash
		    $self rexec -e 30 -d "devcon disable" -t 120 \
			"devcon disable '@$node'"
		    if {!$missing} {
			# Copy .sys file back after a successful unload so we
			# can reload it again.  Use cp not mv due to a
			# rename() bug in early versions of Cygwin 1.7.0
			$self cp $sys.off $sys
		    }
		}
	    }
	    $self probe clear
	    # Wait in case the messages are delayed
	    UTF::Sleep 5
	    $self closeSetupLogs
	} finally {
	    incr reloadlock -1
	}
    }

    UTF::doc {
	# [call [arg host] [method ifconfig]]
	# [call [arg host] [method ifconfig] [option -a]]

	# List IP network interfaces

	# [call [arg host] [method ifconfig] [arg dev]]

	# List IP network interface [arg dev]

	# [call [arg host] [method ifconfig] [arg dev] [option dhcp]]

	# Enable DHCP on interface [arg dev]

	# [call [arg host] [method ifconfig] [arg dev]
	# 	[arg ip-address]]

	# Set interface [arg dev] to static IP address
	# [arg ip-address] with a default netmask of 255.255.255.0

	# [call [arg host] [method ifconfig] [arg dev]
	# 	[arg ip-address] [option netmask] [arg netmask]]

	# Set interface [arg dev] to static IP address
	# [arg ip-address] with netmask [arg netmask]

    }

    # Extract just the ipconfig for our adapter
    proc parseipconfig {dev data} {
	# Strip doubled newlines
	regsub -all {[\n\r]+} $data "\n" data
	# Find start of correct interface
	set start [string first "$dev:" $data]
	if {$start < 0} {
	    return "$dev not found"
	} else {
	    # Strip data before our adapter
	    set data [string range $data $start end]

	    # Strip from the next line with no leading whitespace
	    # leaving just the adapter we want
	    regsub {\n\S.*} $data {} data
	    return $data
	}
    }

    method ipconfig {cmd dev} {
	set dhcpreply [$self rexec -t 120 "ipconfig $cmd '$dev'"]
	regsub {\s*Windows IP Configuration\s*} $dhcpreply {} dhcpreply
	return $dhcpreply
    }

    # IP address cache
    variable ipaddr -array {}

    # Simulate ifconfig using netsh
    method ifconfig {args} {
	$self probe

	set cmd "netsh interface ip"
	# "ifconfig" or "ifconfig -a"
	if {[llength $args] == 0 || [lindex $args 0] eq "-a"} {
	    set msg [$self rexec ipconfig]
	    regsub -all {\n+} $msg "\n" msg
	    return $msg
	} else {
	    set dev [lindex $args 0]
	    set ifname [$self Interfaces $dev]
	    if {$ifname eq ""} {
		error "Empty Interface Name!"
	    }
	    if {[llength $args] == 1} {
		return [parseipconfig $ifname [$self rexec ipconfig]]
	    } elseif {[llength $args] == 2 ||
		      [llength $args] == 3} {
		set ip [lindex $args 1]
		if {$ip == "dhcp"} {
		    # "ifconfig <ifname> dhcp"

		    # invalidate cache in case of failure
		    if {[info exists ipaddr($dev)]} {
			unset ipaddr($dev)
		    }

		    if {![regexp {DHCP enabled:\s+Yes} \
			      [$self rexec -s "$cmd show address '$ifname'"]]} {
			$self rexec "$cmd set address '$ifname' dhcp"
		    }
		    $self ipconfig /release $ifname
		    set dhcpreply [$self ipconfig /renew $ifname]
		    if {[regexp {server is unavailable|procedure call failed} \
			     $dhcpreply]} {
			$self sc start Dhcp
			UTF::Sleep 10
			set dhcpreply [$self ipconfig /renew $ifname]
		    }
		    # "recovered" is used to record transient failures
		    set recovered ""
		    for {set d 1} {$d < $options(-dhcpretries) &&
				   [regexp {in use|is denied|has timed out|Unable to query host name} \
					$dhcpreply]} {incr d} {

			# If the IP is in use wait before trying
			# again, otherwise retry immediately.
			if {[regexp {in use} $dhcpreply]} {
			    #set recovered "IP address in use"
			    UTF::Sleep 10
			}
			set dhcpreply [$self ipconfig /renew $ifname]
		    }
		    set ipconfig [parseipconfig $ifname $dhcpreply]
		    if {[regexp {Gateway .* : ([\d\.]*)} $ipconfig - gw]} {
			if {$d > 3} {
			    set recovered "DHCP required $d attempts"
			}
			if {$gw eq ""} {
			    UTF::Message WARNING $options(-name) "No gateway"
			} else {
			    # Remove any default route to this gateway
			    catch {$self rexec route delete 0.0.0.0 \
				       mask 0.0.0.0 $gw}
			    catch {$self rexec route print -4}
			}
			if {$recovered ne ""} {
			    error "$recovered: recovered"
			}
		    } else {
			regsub {.*An error [^:]*: } $dhcpreply {} dhcpreply
			error [string trim $dhcpreply]
		    }
		    # Return ipaddr and Update cache
		    return [$self ipaddr $dev $ipconfig]
		} else {
		    # "ifconfig <ifname> <ipaddress>"
		    set mask 255.255.255.0
		    if {![regexp {IP Address:\s*([\d.]+)\s+SubnetMask:\s*([\d.]+)} \
			      [$self rexec -s "$cmd show address '$ifname'"] \
			      - i m] || $i ne $ip || $m ne $mask} {
			$self rexec -t 120 \
			    "$cmd set address '$ifname' static '$ip' '$mask'"
		    }
		    # Return ipaddr and Update cache
		    return [set ipaddr($dev) $ip]
		}
	    } elseif {[llength $args] == 4 && \
			  [lindex $args 2] eq "netmask"} {
		# "ifconfig <ifname> <ipaddress> netmask <netmask>"
		set ip [lindex $args 1]
		set mask [lindex $args 3]
		if {![regexp {IP Address:\s*([\d.]+)\s+SubnetMask:\s*([\d.]+)}\
			  [$self rexec -s "$cmd show address '$ifname'"] \
			  - i m] || $i ne $ip || $m ne $mask} {
		    $self rexec -t 120 \
			"$cmd set address '$ifname' static '$ip' '$mask'"
		}
		# Return ipaddr and Update cache
		return [set ipaddr($dev) $ip]
	    } else {
		UTF::Message WARNING $options(-name) \
		    "Feature not available: [list ifconfig $args]"
	    }
	}
    }

    UTF::doc {
	# [call [arg host] [method {route add}] [arg net] [arg mask]
	#	  [arg gw]]

	# Add an IP route to network [arg net] netmask [arg mask]
	# through gateway [arg gw]
    }
    method {route add} {net mask gw} {
	$self rexec route add $net mask $mask $gw
    }

    UTF::doc {
	# [call [arg host] [method {route replace}] [arg net] [arg mask]
	#	  [arg gw]]

	# Add or Modify an IP route to network [arg net] netmask [arg
	# mask] through gateway [arg gw]
    }
    method {route replace} {net mask gw} {
	$self rexec route delete $net mask $mask
	$self rexec route add $net mask $mask $gw
    }

    UTF::doc {
	# [call [arg host] [method {route delete}] [arg net] [arg mask]
	#	  [lb][arg gw][rb]]

	# Delete an IP route to network [arg net] netmask [arg mask]
	# through optional gateway [arg gw]
    }
    method {route delete} {net mask {gw ""}} {
	$self rexec route delete $net mask $mask $gw
    }



    UTF::doc {
	# [call [arg host] [method pci] [arg dev]]

	# Return PCI bus.dev numbers for the device
    }

    method pci {dev} {
	$self probe
	return $PCI($dev)
    }

    UTF::doc {
	# [call [arg host] [method macaddr] [arg dev]]

	# Returns the MAC address for [arg dev]
    }
    method macaddr {dev} {
	if {[regexp {^([[:xdigit:]]{2}:){5}[[:xdigit:]]{2}} $dev]} {
	    return $dev
	}
	if {$dev eq ""} {
	    set dev $defnum
	}
	if {[info exists MAC($dev)]} {
	    $self MAC $dev
	} else {
	    $self wl cur_etheraddr
	}
    }

    UTF::doc {
	# [call [arg host] [method ipaddr] [arg dev]]

	# Return IP address of device [arg dev] or Error if device is
	# down
    }

    method ipaddr {dev {ipconfig ""}} {
	if {$ipconfig eq ""} {
	    # Check cache, unless we're given a prescanned config to parse
	    if {[info exists ipaddr($dev)]} {
		return $ipaddr($dev)
	    }
	    set ipconfig [$self ifconfig $dev]
	}
	if {[regexp {IP(?:v4)? Address[. ]*:\s+([0-9.]+)} $ipconfig - addr] &&
	    $addr ne "0.0.0.0"} {
	    if {[regexp {^10\.} $addr]} {
		error "Unexpected corporate address: $addr"
	    }
	    # Return ipaddr and Update cache
	    return [set ipaddr($dev) $addr]
	} else {
	    error "$options(-name) No IP address available: $ipconfig"
	}
    }

    method ping {target args} {
	# BindIperf being used as a WAR for SWWLAN-125912
	if {![info exists ::UTF::BindIperf]} {
	    return [$base ping $target {*}$args]
	}
	set msg "ping $target $args"
	UTF::Getopts {
	    {c.arg "5" "Count"}
	    {s.arg "56" "Size"}
	} args
	if {[info commands $target] == $target} {
	    set target [$target ipaddr]
	}
	set src [$self ipaddr {}]
	# Loop, since Router ping doesn't handle "count" itself
	for {set c 0} {$c < $(c)} {incr c} {
	    if {![catch {$self rexec -t 2 ping -n 1 -l $(s) -S $src $target}]} {
		return
	    }
	}
	error "ping failed"
    }



    UTF::doc {
	# [call [arg host] [method wlconfig] [arg {args ...}]]

	# Run wlconfig on [arg host].  If option [option -A] [arg
	# device] is supplied as the first arguments, they will be
	# translated into [option -A] [arg {interface name}].
    }
    method wlconfig {args} {
	$self probe

	if {[lindex $args 0] == "-A"} {
	    set args [lreplace $args 1 1 "'$Interfaces([lindex $args 1])'"]
	}
	$self rexec wlconfig {*}$args
    }

    method reg {dev {key ""} {value "<>"}} {
	$self probe
	set CCS "/HKLM/SYSTEM/CurrentControlSet"
	set devcache [$self RegCache $dev]
	if {$devcache eq ""} {
	    error "No devcache found"
	}
	set path "$CCS/Control/Class/[$self RegCache $dev]"
	if {$key eq ""} {
	    $base regtool list -v '$path'
	} elseif {$value eq "<>"} {
	    $base regtool get '$path/$key'
	} elseif {$value eq "unset"} {
	    $base -x regtool unset '$path/$key'
	} else {
	    $base regtool set -s -- '$path/$key' '$value'
	}
    }

    method regsetup {} {
	$self probe clear
	foreach {dev node} [$self Nodes] {
	    if {![regexp -nocase $options(-node) $node]} {
		# Only update matching nodes
		continue
	    }
	    foreach {k v} [concat $reg [UTF::decomment $options(-reg)]] {
		$self reg $dev $k $v
	    }
	}
    }

    UTF::doc {
	# [call [arg host] [method probe]]

	# Attempt to auto-discover properties of [arg host].  Caches
	# mapping between Device names, Device nodes and Network
	# Interface names.  Should be rerun whenever a network device
	# is enabled or disabled.
    }
    variable defnum   ""
    variable probe_dl 0
    method probe {{opt ""}} {
	switch $opt {
	    "clear" {
		set probed 0
		return
	    }
	    "" {
		if {$probed} {
		    return
		}
	    }
	    force {
	    }
	    default {
		error "usage: probe \[clear|force\]"
	    }
	}
	# If we already downloaded the probe script, try it now.
	if {$probe_dl && [catch {$self -s ./probe.tcl} req]} {
	    if {[regexp {cannot execute|not found} $req]} {
		# probe.tcl got corrupted.  Clear download flag and try again.
		set probe_dl 0
	    } else {
		error $req
	    }
	}

	# download probe script first time, or if found to be
	# corrupted.
	if {!$probe_dl} {
	    set tclscript {#! /usr/bin/tclsh
		if {$tcl_version < 8.5} {
		    # Older Cygwin tcl is has registry functions
		    package require registry
		} else {
		    # Newer Cygwin tcl is compiled for unix.
		    # Implement subset of registry using /proc
		    proc registry {get path key} {
			regsub -all {\\} $path {/} path
			set fd [open "/proc/registry/$path/$key"]
			set value [read $fd]
			if {$key eq "Count"} {
			    # Convert from DWORD
			    binary scan $value n value
			} else {
			    # Convert from SZ
			    binary scan $value A* value
			}
			close $fd
			return $value
		    }
		}
		if {[catch {exec wl xlist} list]} {
		    catch {exec wl list} list
		}
		set NetCfgInstanceId 0
		if {[regexp Error $list]} {
		    puts stderr $list
		} else {
		    foreach line [split $list "\n"] {
			if {[regexp {(\d+):\s+(.*)} $line - num rest]} {
			    if {[regexp {({.*})} $rest - NetCfgInstanceId]} {
				set wl($NetCfgInstanceId) $num
			    }
			    if {[regexp {MAC: (\S*)} $rest - m]} {
				set mac($NetCfgInstanceId) $m
			    }
			}
		    }
		}
		set un 0
		set CCS "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet"
		foreach Service {
		    BCMPCIEDHD63 BCMPCIE43XX BCM43XX BCM43XV BCMH43XX
		    BCMSDH43XX dhdusb dhdusb.NTx86 dhdusb51.NTx86
            USB_RNDIS vwifimp BcmVWL BCMWL63A BCMWL63
		} {
		    if {[catch {
			set ServicePath "$CCS\\Services\\$Service"
			catch {registry get $ServicePath "ImagePath"} ImagePath
			if {[catch {
			    registry get "$ServicePath\\Enum" "Count"
			} count]} {
			    continue
			}
			for {set i 0} {$i<$count} {incr i} {
			    set dev [registry get "$ServicePath\\Enum" $i]
			    set key "$CCS\\Enum\\$dev"
			    set ClassGUID [registry get $key "ClassGUID"]
			    set CCSCNCG "$CCS\\Control\\Network\\$ClassGUID"
			    set Driver [registry get $key "Driver"]
			    set pbus 0
			    set pdev 0
			    catch {
				regexp {PCI bus (\d+), device (\d+)} \
				    [registry get $key "LocationInformation"] - \
				    pbus pdev
			    }
			    if {[catch {
				registry get "$CCS\\Control\\Class\\$Driver" \
				    "NetCfgInstanceId"
			    } NetCfgInstanceId]} {
				# Registry problem?  Report and move on
				puts $NetCfgInstanceId
				continue
			    }
			    if {[info exists mac($NetCfgInstanceId)]} {
				set m $mac($NetCfgInstanceId)
			    } elseif {$Service eq "vwifimp"} {
				set m "VWIFI"
			    } else {
				set m "<unknown$un>"
			    }
			    if {[info exists wl($NetCfgInstanceId)]} {
				set wlnum $wl($NetCfgInstanceId)
			    } elseif {[info exists wl($i)]} {
				set wlnum $wl($i)
			    } else {
				# Device not enabled
				set wlnum "<unknown$un>"
			    }
			    catch {
				registry get \
				    "$CCSCNCG\\$NetCfgInstanceId\\Connection" \
				    "Name"} Name
			    puts [list >> $NetCfgInstanceId $wlnum $Driver \
				      $Name $dev $ImagePath "$pbus.$pdev" $m]
			    incr un
			}
		    } ret] == 1} {
			# Print errors, but not continue, etc
			puts $ret
		    }
		}
	    }
	    $self rexec -q "echo '$tclscript' > probe.tcl"
	    $self chmod +x probe.tcl
	    set probe_dl 1

	    if {[catch {$self -s ./probe.tcl} req]} {
		if {[regexp {cannot execute|not found} $req]} {
		    # probe.tcl got corrupted.  Clear download flag.
		    set probe_dl 0
		}
		error $req
	    }
	}
	array unset Devices any,*
	array unset CaptureDevice
	array unset Nodes
	array unset Interfaces
	array unset RegCache
	array unset PCI
	array unset MAC
	array unset SYS
	set defnum "<unknown0>"
	foreach line [split $req \n] {
	    if {[regexp {>>} $line]} {
		UTF::Message LOG $options(-name) $line
		foreach {- ncid num driver name dev path pci mac} $line {}
		set sysfile [string tolower [lindex [split $path {\\}] end]]
		if {$mac ne "VWIFI"} {
		    lappend Devices(any,node) $dev
		}
		if {![regexp {^5} $options(-osver)]} {
		    set num $mac
		}
		# default device - pick the first enabled, if there
		# are any, otherwise the last disabled.  If the -node
		# filter is enabled, honor it.  The search order is
		# arbitrary.
		if {[regexp -nocase $options(-node) $dev] &&
		    [regexp {<unknown} $defnum]} {
		    set defnum $num
		}
		set CaptureDevice($num) "\\Device\\NPF_$ncid"
		set Nodes($num) $dev
		regsub {^\\SystemRoot\\} $path {} path
		if {[$self hassysnative]} {
		    # Bypass WoW64's System32 remapping
		    regsub -nocase {\mSystem32\M} $path {sysnative} path
		}
		set Devices($sysfile,path) [string map {\\ /} $path]
		set Interfaces($num) $name
		set RegCache($num) [string map {\\ /} $driver]
		set PCI($num) $pci
		if {![regexp {unknown} $mac]} {
		    set MAC($num) $mac
		}
		set SYS($dev) $Devices($sysfile,path)
	    } elseif {![regexp {different address} $line]} {
		UTF::Message WARNING $options(-name) $line
	    }
	}
	set probed 1
    }

    UTF::DevCache CaptureDevice
    UTF::DevCache Nodes
    UTF::DevCache Devices
    UTF::DevCache Interfaces
    UTF::DevCache RegCache
    UTF::DevCache PCI
    UTF::DevCache MAC
    UTF::DevCache SYS

    method rte_available {} {
	expr {![catch {$self wl offloads} ret] && $ret &&
	      [$self wl ol_disable] ne "1"}
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

   method hndrte_src {} {
	if {![info exists hndrte_src]} {
	    regsub {/dongle/rte/.*} [$self hndrte_exe] {} hndrte_src
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

    variable TRAPLOG
    variable processingDongleError 0
    typevariable msgfile
    # Internal callback for fileevent below
    method getdata {request} {
	set fd $msgfile($request)
	if {![eof $fd]} {
	    set msg [gets $fd]
	    if {[regexp {:/var/log/messages} $request]} {
		# Lose sequence numbers and dates, since we're adding our own.
		regsub {^\d{8}\s+(\d+\.\d+|\d{8})\s+} $msg {} msg
		# Ignore these, from TrayApp or VNC
		if {[regexp {OleException|OLE Exception|e_UPDATE_.*_TIMER_EVT|DrvAssertMode|appxexecutionutil|modernexecserver|thumbcacheapi|optimizedstyle|KTM:  } $msg]} {
		    return
		}
		if {$options(-msgcallback) ne ""} {
		    # Specialist host failures.  Returns true if no
		    # further processing is required.
		    if {[{*}$options(-msgcallback) $msg]} {
			return
		    }
		}

		if {[info exists TRAPLOG]} {
		    # If FWID is set we're collecting new-style trap messages
		    regsub {^\d+\.\d+ } $msg {} msg
		    append TRAPLOG "$msg\n"
		}
		if {[regexp {FWID (\d\d-[[:xdigit:]]{1,8})} $msg - FWID]} {
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
		} elseif {[regexp {NetSetupEngine.dll!.* assertion } $msg]} {
		    # Not our assert - log only
		    UTF::Message LOG $options(-name) $msg
		} elseif {![$base common_getdata $msg]} {
		    UTF::Message LOG $options(-name) $msg
		}
	    } elseif {[regexp {setupapi} $request]} {
		# replace unprintable data
		regsub -all {[^[:print:]]} $msg {.} msg
		UTF::Message LOG $options(-name) $msg
	    } elseif {[regexp {bcmwl} $request]} {
		# InstallShield logs
		regsub {^\d\d:\d\d:\d\d: } $msg {} msg
		if {[regexp {Error: file ".*\.cpp"|Unknown error|Rollback} $msg]} {
		    # Only pass up the first failure
		    if {![info exists ::UTF::panic]} {
			set ::UTF::panic $msg
		    }
		    UTF::Message FAIL $options(-name) $msg
		} else {
		    UTF::Message LOG $options(-name) $msg
		}
	    } elseif {[regexp {logfile.*\.txt} $request]} {
		# MFGC log file
		set ::UTF::Test::Tick [clock seconds]
		if {[regexp {TEST_START:(.*)} $msg - t]} {
		    # Output status line
		    UTF::Message INFO $options(-name) $msg
		    # Update global test state (do this last, since
		    # someone may be vwaiting on it.)
		    set ::UTF::Test::MFGCcurrenttest $t
		} elseif {[regexp {> (Total Test Time: .*)} $msg - t]} {
		    # Output status line
		    UTF::Message INFO $options(-name) $msg
		    # Update global test state (do this last, since
		    # someone may be vwaiting on it.)
		    set ::UTF::Test::MFGCcurrenttest $t
		} elseif {[regexp {>\s*(.*(?:,.*){19},(Pass|Fail))$} \
			       $msg - res]} {
		    # Output results line
		    UTF::Message INFO $options(-name) $msg
		    lappend ::UTF::Test::MFGCresults $res
		} else {
		    UTF::Message LOG $options(-name) $msg
		}
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

    variable setuplogs ""
    variable setuploglevel 0

    method openSetupLogs {} {
	if {[incr setuploglevel] != 1} {
	    # track nested calls, so we don't close when someone is
	    # still interested.
	    return
	}
	$self ConfigureLoglevel
	if {$setuplogs eq ""} {
	    # Ultimately, this will be in setup, but make it runtime
	    # for now to give smoketest a chance to update their
	    # images.
	    set W [$self cygpath -W]
	    if {[regexp {^5} [$self cget -osver]]} {
		# WinXP
		set setuplogs "$W/setupapi.log"
	    } else {
		# Vista, Win7
		set setuplogs \
		    [list "$W/inf/setupapi.dev.log" "$W/inf/setupapi.app.log"]
	    }
	    $self cycleSetupLogs $setuplogs
	}
	foreach log $setuplogs {
	    $self open_messages $log
	}
	set setuplogsareopen 1
    }

    method closeSetupLogs {} {
	if {[incr setuploglevel -1] < 1} {
	    foreach log $setuplogs {
		$self close_messages $log
	    }
	}
    }

    UTF::doc {
	# [call [arg host] [method open_messages] [lb][arg file][rb]]

	# Open system message log on [arg host] and log new messages
	# to the UTF log.  [method open_messages] will return
	# immediately, while messages will continue to be logged in
	# the background until [method close_messages] is called.  If
	# [arg file] defaults to [file /var/log/messages].  On Windows
	# a kernel logging tool such as [cmd DebugView] should be
	# installed and configured to write to this file.

	# [para]

	# Multiple loggers can be opened, so long as they log
	# different [arg file]s.  Attempts to log the same [arg file]
	# multiple times will be logged and ignored.
    }

    method open_messages { {file ""} } {
	if {$file eq ""} {
	    set file $options(-console)
	}
	set id [$self cget -lan_ip]
	if {[info exists msgfile($id:$file)]} {
	    return
	}
	UTF::Message LOG $options(-name) "Open $file"
	if {[string match "*/*" $file]} {
	    set ret [$self rpopen -q -noinit tail -n 0 -F $file]
	    set msgfile($id:$file) $ret
	    fconfigure $msgfile($id:$file) -blocking 0
	} else {
	    if {[catch {$self serialrelay socket $file} ret]} {
		$self worry "$file: $ret"
		return
	    }
	    set msgfile($id:$file) $ret
	    fconfigure $msgfile($id:$file) -blocking 0 -buffering line
	    puts $msgfile($id:$file) ""; # Trigger NPC telnet reconnect
	}
	fileevent $msgfile($id:$file) readable [mymethod getdata $id:$file]
	return $ret
    }

    UTF::doc {
	# [call [arg host] [method pause_messages] [lb][arg file][rb]]

	# Temporarily pause a message log previously opened by [method
	# open_messages].  Messages will remain in the input stream to
	# be processed later.
    }

    method pause_messages { {file ""} } {
	set id [$self cget -lan_ip]
	if {[info exists msgfile($id:$file)]} {
	    fileevent $msgfile($id:$file) readable ""
	} else {
	    UTF::Message WARN $options(-name) "$file not open"
	}
	return
    }

    UTF::doc {
	# [call [arg host] [method continue_messages] [lb][arg
	# file][rb]]

	# Resume processing a message log previously paused by [method
	# pause_messages].  [method pause_messages] and [method
	# continue_messages] may be used to temporarily defer message
	# processing.[para]

	# WARNING: This will invalidate timestamps in the reports.
    }

    method continue_messages { {file ""} } {
	set id [$self cget -lan_ip]
	if {[info exists msgfile($id:$file)]} {
	    fileevent $msgfile($id:$file) readable [mymethod getdata $id:$file]
	} else {
	    UTF::Message WARN $options(-name) "$file not open"
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

    method close_messages { {file ""} } {
	if {$file eq ""} {
	    set file $options(-console)
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
	    UTF::Message WARN $options(-name) "$file not open"
	}
	return
    }

    method init {} {
	$self probe clear
	if {[info exists ::UTF::RecordStack]} {
	    $self crashcheck
	}
	$self open_messages
    }

    method deinit {} {
	$self close_messages
	$self configure -initialized 0
    }

    method SaveHostKeys {} {
	$self rexec {cd /etc;tar -cjf /tmp/savedhostkeys \
			 ssh_host_{dsa_,rsa_,}key{,.pub}}
	$self copyfrom /tmp/savedhostkeys /tmp/savedhostkeys
    }

    method RestoreHostKeys {} {
	$self configure -ssh {ssh -o UserKnownHostsFile=/tmp/ukhf}
	$self authorize
	$self copyto /tmp/savedhostkeys /tmp/savedhostkeys
	$self rexec {cd /etc;tar -xjvf /tmp/savedhostkeys \
			 ssh_host_{dsa_,rsa_,}key{,.pub}}
	file delete /tmp/ukhf
    }

    UTF::doc {
	# [call [arg host] [method tcptune] [arg window]]

	# Configure host tcp window size.  [arg window] indicates
	# desired TCP Window size in bytes.  A k suffix can be used to
	# indicate KB instead.

	# Returns 1 if userland tools need to specify the window size
	# themselves.

	# Also check to make sure the IPerfService is properly
	# installed ready for ther next TCP test.
    }
    variable tuned ""
    method tcptune {window} {
	if {[info exists UTF::NoTCPTuning] && [info exists UTF::TcpReadStats]} {
	    return 1
	}
	set window [UTF::kexpand $window]
	if {$window ne $tuned} {
	    if {[regexp {^5} $options(-osver)]} {
		# WinXP doesn't need tuning, but we do need to make
		# sure the Iperf service is installed
		if {[$self cget -iperfdaemon] &&
		    ![info exists UTF::TcpReadStats]} {
		    $self IPerfService InstallCheck
		}
	    } else {
		if {$options(-tcptuneserver) && $window ne 0} {
		    # Configure iperf server environment.  Probably
		    # works for Win7 and Vista.  Not needed for WinXP.
		    # Needs more testing, expecially on bmac before we
		    # can enable this generally.
		    $self rexec "setx /M TCP_WINDOW_SIZE $window"
		} else {
		    $self rexec "setx /M TCP_WINDOW_SIZE ''"
		}
		if {[$self cget -iperfdaemon] &&
		    ![info exists UTF::TcpReadStats]} {
		    $self IPerfService restart
		}
	    }
	    set tuned $window
	}

	# Userland tools also need to specify window size.
	return 1
    }

    # variable to record if we've already removed the iperf service.
    # Anything that attempts to reinstall it will reset this flag,
    # even if unsucessful.
    variable iperfserviceremoved 0

    UTF::doc {
	# [call [arg host] [method {IPerfService restart}]]

	# Restarts the IPerfService (reinstalling the service if
	# necessary).
    }

    method {IPerfService restart} {} {
	if {[catch {$self sc stop IPerfService} ret] ||
	    [catch {$self sc start IPerfService}]} {
	    if {[regexp {service does not exist} $ret]} {
		# If the service is missing, reinstall it.
		$self IPerfService Install
	    } else {
		# If either the stop or the start give other
		# errors, kill off the existing iperf process and
		# then try the start again.
		$self IPerfService kill
		if {[catch {$self sc start IPerfService} ret]} {
		    # After all this the start should not fail.
		    # If it does, then set a panic to flag the
		    # whole test as bad!
		    set ::UTF::panic "IPerfService failed: $ret"
		    error $ret
		}
	    }
	}
    }

    UTF::doc {
	# [call [arg host] [method {IPerfService InstallCheck}]]

	# Installs the IPerfService if necessary.  If the service is
	# already installed this will return without making any
	# changes.
    }

    method {IPerfService InstallCheck} {} {
	set iperfserviceremoved 0
	if {[catch {$self sc qc IPerfService} ret]} {
	    if {[regexp {does not exist} $ret]} {
		$self IPerfService Install
	    } else {
		error $ret
	    }
	} elseif {![regexp -line \
			"BINARY_PATH_NAME\\s+:.*\\\\[$self cget -iperf](.bin)?$" \
			$ret]} {
	    $self IPerfService Remove
	    $self IPerfService Install
	}
    }

    UTF::doc {
	# [call [arg host] [method {IPerfService Install}]]

	# Installs the IPerfService.  If the service is already
	# installed this will give an error.
    }

    method {IPerfService Install} {} {
	set iperfserviceremoved 0
	set iperf [file join [$self cygpath -S] [$self cget -iperf]]
	$self rexec "sc create IPerfService binPath= `cygpath -w $iperf` start= auto"
     	$self sc failure IPerfService \
	    reset= 0 actions= restart/6000
	$self sc start IPerfService
    }

    UTF::doc {
	# [call [arg host] [method {IPerfService Remove}]]

	# Removed the IPerfService.  If the service is already removed
	# this will return without making any changes.
    }

    method {IPerfService Remove} {} {
	if {$iperfserviceremoved} {
	    # Skip if already removed.
	    return
	}
	# Unset the tuned flag, so tcptune will have to recheck next
	# time
	set tuned ""
	if {[catch {$self sc query IPerfService} ret]} {
	    if {[regexp {does not exist} $ret]} {
		set iperfserviceremoved 1
		return
	    } else {
		error $ret
	    }
	}
	if {[regexp {RUNNING} $ret]} {
	    $self sc stop IPerfService
	}
	$self IPerfService kill
	$self sc delete IPerfService
	set iperfserviceremoved 1
    }

    UTF::doc {
	# [call [arg host] [method {IPerfService kill}]]

	# Kills the iperf server process associated with the
	# IPerfService.  This may be needed to recover from some
	# system problems.
    }

    method {IPerfService kill} {} {
	if {![catch {$self sc queryex IPerfService} ret] &&
	    [regexp {PID\s+:\s+(\d+)} $ret - pid] &&
	    $pid > 0} {
	    $self rexec /bin/kill -f $pid
	}
    }

    variable setup_iperf 0
    method setup_iperf {} {
	if {$setup_iperf} {
	    return
	}
	set setup_iperf 1
	set S [$self cygpath -S]
	$self IPerfService kill
	foreach i {iperf208} {
	    set s "$archive/cygdrive/c/WINDOWS/system32/$i.exe"
	    if {[catch {$self sum "$S/$i.exe"} ret] || $ret ne [exec sum $s]} {
		$self copyto $s "$S/$i.exe"
	    }
	}
	if {[set iperf [$self cget -iperf]] eq "iperf"} {
	    set iperf iperf208
	}
	$self cp -p $S/$iperf.exe $S/iperf.exe
    }

    UTF::doc {
	# [call [arg host] [method sc] [arg args]]

	# Wrapper for "sc" command to add return codes on WinXP.
	# WinVista and later provide retrun codes directly.
    }
    method sc {args} {
	if {[regexp {^5} $options(-osver)]} {
	    # WinXP doesn't support returns codes for sc, so check for
	    # FAIL message
	    set ret [$self rexec -n sc {*}$args]
	    if {[regexp {\sFAILED\s\d+:} $ret]} {
		error $ret
	    } else {
		return $ret
	    }
	} else {
	    $self rexec -n sc {*}$args
	}
    }

    UTF::doc {
	# [call [arg host] [method reboot]]

	# Reboots the DUT.  Based on Windows shutdown, in case the
	# Cygwin "reboot" command wasn't installed.
    }

    method reboot {args} {
	$self rexec -n "[$self cygpath -S]/shutdown /r /f /t 0"
    }

    UTF::doc {
	# [call [arg host] [method setup]]

	# Installs or updates any local tools needed on a UTF client.
    }

    method setup {args} {
	UTF::Getopts {
	    {norsync "Don't rsync files"}
	}
	set ssh [$self cget -ssh]
	$self configure -ssh ssh
	$self authorize
	$self configure -initialized 1
	# sanity check, in case this isn't cygwin!
	UTF::Message INFO $options(-name) \
	    "Cygwin rooted at [$self -n cygpath -w /]"

	$self rexec -n {egrep "^UseDNS n" /etc/sshd_config || sed -i "s/#UseDNS yes/UseDNS no/" /etc/sshd_config}
	#catch {$self pkill -HUP sshd}
	$self configure -initialized 1

	catch {$self sc stop IPerfService}

	$self pkill dbgview.exe
	$self pkill iperf.exe

	if {!$(norsync)} {
	    if {[catch {
		$self rsync "$archive/cygdrive/c" /cygdrive
	    } ret]} {
		UTF::Message LOG $self $ret
	    }
	    if {[catch {
		$self rsync "$archive/usr" /
	    } ret]} {
		UTF::Message LOG $self $ret
	    }
	    $self rexec -n touch /var/log/messages
	}

	# switch to a faster shell now, if avalable
	$self configure -ssh $ssh
	$self configure -initialized 0

	if {[regexp {64} $options(-osver)]} {
	    $self -n ln -fs devcon64.exe /usr/bin/devcon.exe
	    if {[$self hassysnative]} {
		$self -x -n "type bcdedit||nircmdc elevate cp /cygdrive/c/Windows/sysnative/bcdedit.exe \
		/cygdrive/c/Windows/System32/bcdedit.exe"
	    }
	} else {
	    $self -n ln -fs devcon32.exe /usr/bin/devcon.exe
	}

	catch {$self SetupRxvt}

	if {![regexp {64} $options(-osver)]} {
	    $self install_Debugging_Tools_for_Windows
	}

	$self SetupDbgView
	$self SetupTime

	# Add sshd startup
	$self -n -x "nircmdc shortcut \
\"\$(cygpath -w \$(type --path run).exe)\" \
'~\$folder.startup$' 'sshd' \
\"bash -l -c /usr/sbin/sshd\" '' '' '' \
\"\$(cygpath -w /bin)\""

	$self copyto $UTF::unittest/etc/cleaninfs.sh \
	    /usr/local/bin/cleaninfs.sh

	# iperf service
	$self setup_iperf

	if {[regexp {^5} $options(-osver)]} {
	    set Tcpip \
		"/HKLM/SYSTEM/CurrentControlSet/Services/Tcpip/Parameters"
	    $self -n regtool set -- '$Tcpip/TCPWindowSize' 1048576
	}
	$self ConfigureLoglevel
    }

    UTF::doc {
	# [call [arg host] [method InstallConsolelogger] [lb][arg -port] \
		  [arg port][rb] [lb][arg -dev] [arg dev][rb]]

	#  Copies files consolelogger tool, installs shortcut and
	#  starts service.  Used only if you need to use a Windows
	#  system as a serial-port relay.
    }

    method InstallConsolelogger {args} {
	UTF::Getopts {
	    {dev.arg "/dev/ttyS0" "Serial device"}
	    {port.arg "40000" "Port number"}
	}
	$self copyto $UTF::unittest/consolelogger consolelogger
	$self -n -x "nircmdc shortcut \
\"\$(cygpath -w /bin/rxvt.exe)\" \
'~\$folder.startup$' 'consolelogger$(port)' \
'-sl 500 -e expect /home/user/consolelogger -p $(port) $(dev)'"
	$self -n -x \
	    "nircmdc execmd \"\$(cygpath -wsP)\\Startup\\consolelogger$(port).lnk\" >- 2>-"
    }

    method ConfigureLoglevel {} {
	# Configure loglevel to give non-verbose errors, warnings and
	# other information.
	# Setting SetupAPI Logging Levels (Windows Drivers)
	# http://msdn.microsoft.com/en-us/library/windows/hardware/ff550808%28v=vs.85%29.aspx
	$self regtool set -- \
	    /HKLM/Software/Microsoft/Windows/CurrentVersion/Setup/LogLevel \
	    0x20003030
    }

    method CheckDriverSigningPolicy {} {
	# Only applicable to WinXP since we don't sign XP drivers.

	# We can read the policy from the registry, but we can't set
	# it, since MSFT keeps security settings like this protected
	# by secret hashes so only the GUI can change them.

	# We're checking the "user" policy.  There is also a "system"
	# policy in HKLM, but we don't need to check it because the UI
	# will never make the user policy weaker than the system
	# policy.

	if {[regexp {^5} $options(-osver)] &&
	    ([catch {
		$self -n regtool get -- \
		    '/HKCU/Software/Microsoft/Driver Signing/Policy'
	    } ret] ||
	     $ret > 0)} {
	    error "Host blocking unsigned drivers.\nPlease change \"My Computer//Properties//Hardware//Driver Signing\" to \"Ignore\" and check \"Make this action the system default\""
	}
    }

    method SetupDbgView {} {
	$self pkill dbgview.exe

	if {[regexp {64} $options(-osver)]} {
	    # Copy over Dbgview so nircmdc can find it.  We should set
	    # up a proper 64bit set of apps, including nircmdc.
	    $self rexec "cp -r '/cygdrive/c/Program\ Files/dbgview' '/cygdrive/c/Program\ Files (x86)'"
	}

	# create shortcut
	if {[regexp {^[567]} $options(-osver)]} {
	    $self -n -x "nircmdc shortcut\
		'C:\\Program Files\\dbgview\\Dbgview.exe'\
	    	'~\$folder.startup$' 'Dbgview'\
	    	\"/on /k /g /v /t /h 1000 /l \$(cygpath -w /var/log/messages)\""
	} else {
	    $self -n -x "nircmdc cmdshortcut '~\$folder.startup$' Dbgview \
		elevate '~\$sys.programfiles\$\\dbgview\\Dbgview.exe' \
                /on /g /k /v /t /h 1000 /l \$(cygpath -w /var/log/messages)"
	}

	# Accept EULA
	$self -n regtool add -- '/HKCU/Software/Sysinternals'
	$self -n regtool add -- '/HKCU/Software/Sysinternals/DbgView'
 	$self -n regtool set -- \
	    '/HKCU/Software/Sysinternals/DbgView/EulaAccepted' 1

 	# Start Dbgview with the new settings
	$self -n -x {cmd /c "$(cygpath -wsP)\\Startup\\Dbgview.lnk"}
  }

    method SetupRxvt {} {
	# Comment out broken Rxvt defaults
	set Rxvt "/usr/X11R6/lib/X11/app-defaults/Rxvt"
	$self rexec -noinit \
	    "if \[ -f $Rxvt ]; then sed -i 's/^\\(Rxvt.*\\(font\\|ground\\|scroll\\)\\)/!\\1/' $Rxvt; fi"
    }

    UTF::doc {
	# [call [arg host] [method SetupAutoLogon] [arg passwd]]

	# Set up the host to automatically logon as the UTF test user.
	# The user password will be required.
    }

    method SetupAutoLogon {passwd} {
	set Winlogon \
	    "/HKLM/SOFTWARE/Microsoft/Windows NT/CurrentVersion/Winlogon"
	$self regtool set -s '$Winlogon/AutoAdminLogon' 1
	$self regtool set -s '$Winlogon/ForceAutoLogon' 1
	$self regtool set -- '$Winlogon/DefaultUserName' [$self cget -user]
	$self regtool set -- '$Winlogon/DefaultPassword' $passwd
    }


    method SetupCert {} {

	# Install a private copy of signing tools if it wasn't
	# preinstalled for us.
	if {[catch {$self type makecert}]} {
	    $self rsync $archive/signing/. [$self cygpath -W]
	}

	# Temporary fix for signtool SxS install error
	$self -n rm -f [file join [$self cygpath -W] signtool.exe.manifest]

	$self CleanupCert

	$self -n {
	    makecert -r -pe -ss "UTF Test Certificate" -sr LocalMachine \
		-b 04/30/2013 -e 12/31/2019 \
		-eku "1.3.6.1.5.5.7.3.3","1.3.6.1.4.1.311.10.3.5" \
		-n "CN=UTF Test Certificate,O=Broadcom Corporation,L=Sunnyvale,S=California,C=US" \
		-sky signature UTFTestCert.cer
	}
	# Add certificates to store so they can be used for signing
	$self -n certmgr -add UTFTestCert.cer -s -r localmachine root
	$self -n certmgr -add UTFTestCert.cer -s -r localmachine trustedpublisher

	# Make sure testsigning is enabled (may need a reboot to be
	# effective)
	$self bcdedit /set testsigning 1
	return
    }


    method CleanupCert {} {

	$self -n rm -f UTFTestCert.cer
	while {[regexp {Serial Number} [$self -x -n {certutil -store "UTF Test Certificate"}]]} {
	    $self -n {certutil -delstore "UTF Test Certificate" 0}
	}
	foreach who {root trustedpublisher} {
	    while {[regexp {SHA1 Thumbprint::\s+([[:xdigit:]]{8}) ([[:xdigit:]]{8}) ([[:xdigit:]]{8}) ([[:xdigit:]]{8}) ([[:xdigit:]]{8})} [$self -x -n "certmgr -s -r localmachine $who | grep -A 4 UTF"] - s1 s2 s3 s4 s5]} {
		$self -n certmgr -del -sha1 $s1$s2$s3$s4$s5 -c -s -r localmachine $who
	    }
	}
    }

    method SetupTime {} {
	set r "/HKLM/SYSTEM/CurrentControlSet/Services/W32Time/Config"
	set peers "10.16.16.12,10.16.16.13,10.17.26.16,10.17.23.16,10.17.22.16"

	$self -n date

	# Set the w32Time service to start automatically with OS boot.
	# If the service is missing, register it.
	if {[catch {$self sc config w32Time start= auto}]} {
	    catch {$self sc stop w32Time}
	    $self -n w32tm /register
	}
	if {![regexp {RUNNING} [$self sc query w32Time]]} {
	    # Start w32Time service if not running:
	    $self sc start w32Time
	}

	# Allow unlimited initial sync
	$self -n regtool.exe set $r/MaxPosPhaseCorrection 0xFFFFFFFF
	$self -n regtool.exe set $r/MaxNegPhaseCorrection 0xFFFFFFFF

	# Configure multiple NTP servers:
	$self -n w32tm /config /manualpeerlist:"$peers,time.windows.com" \
	    /syncfromflags:manual /update

	#14:47:17.581  LOG   mc14end2   MaxNegPhaseCorrection: 54000 (Local)
	#14:47:17.581  LOG   mc14end2   MaxPosPhaseCorrection: 54000 (Local)

	if {![regexp {RUNNING} [$self sc query w32Time]]} {
	    # Start w32Time service if not running:
	    $self sc start w32Time
	}

	# Verify the current NTP configuration is correct:
	#$self -n w32tm /query /configuration
	#$self -n w32tm /query /peers
	$self -n w32tm /resync

	# Restrict sync back to default 15hrs
	$self -n regtool.exe set $r/MaxPosPhaseCorrection 54000
	$self -n regtool.exe set $r/MaxNegPhaseCorrection 54000
	$self -n w32tm /config /update

	$self -n date
    }

    UTF::doc {
	# [call [arg host] [method dellater] [arg file]]

	# Mark [file file] for deletion on next boot.  Note that this
	# is not useful for preventing driver init crash loops because
	# the drivers are loaded before the file deletion occurs.
    }

    method dellater {file} {

	if {![regexp {\\} $file]} {
	    set file [$self -noinit cygpath -a -w '$file']
	}

	# Use key format compatible with both regtool and /proc
	set key "HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/Control/Session Manager/PendingFileRenameOperations"

	# use /proc to read because regtool get ignores null entries.
	if {![catch {$self -noinit cat '/proc/registry/$key'} ret]} {
	    # format is a null-seperated list of src dest pairs, with
	    # dst empty meaning delete.

	    # remove end-of-list marker
	    regsub "\0\0$" $ret {} ret

	    # Split and requote.
	    foreach {src dst} [split $ret \0] {
		if {$src eq "\\??\\$file"} {
		    UTF::Message LOG $options(-name) \
			"$file is already marked for deletion"
		    return
		}
		lappend pending "'$src'" "'$dst'"
	    }
	}
	lappend pending "'\\??\\$file'" "''"
	UTF::Message LOG $options(-name) "Marking $file for deletion"
	$self -noinit regtool set -m "'/$key'" {*}$pending
    }

    variable crashtime 0

    method crashcheck {} {
	if {$options(-memorydumpfile) eq ""} {
	    return
	}
	set lastcrash $crashtime
	if {[catch {$self -noinit -t 5 \
			stat -c {"%Y %y"} \
			[$self cygpath \
			     $options(-memorydumpfile)]} ret]} {
	    if {[regexp {No such file} $ret]} {
		# No crash file
		set crashtime -1
		# help user figure out if the system rebooted or not.
		catch {$self -n -t 5 uptime}
	    }
	    return
	}

	# save new crashtime locally, but don't update the object var
	# until we've actually scanned the crash dump.
	set thiscrashtime [lindex $ret 0]
	if {![info exists ::UTF::RecordStack] ||
	    ($lastcrash && $thiscrashtime > $lastcrash)} {
	    set msg "[$self cget -name]: [$self scandump]"
	    # If there's already an assert, don't overwrite it
	    if {![info exists ::UTF::panic] ||
		![regexp -nocase {assert|trap} $::UTF::panic]} {
		set ::UTF::panic $msg
	    }
	    set crashtime $thiscrashtime; # update object var
	    return $msg
	}
	UTF::Message INFO $options(-name) "Skipping old memory.dmp"
	set crashtime $thiscrashtime; # update object var
	 # help user figure out if the system rebooted or not.
	catch {$self -n -t 5 uptime}
	return
    }

    method scandump {} {
	catch {$self -noinit -t 120 \
		   "strings '[$self cygpath \
			     $options(-memorydumpfile)]' |\
	    egrep -m 1 'assertion \".*\" failed: file \".*\", line \[0-9\]'"} \
	    assrt
	set kd [$self kd]
	if {$assrt ne ""} {
	    return $assrt
	} else {
	    return $kd
	}
    }

    UTF::doc {
	# [call [arg host] [method install_Debugging_Tools_for_Windows]
	#       [lb][arg -force][rb]]

	# Install Debugging_Tools_for_Windows on [arg host], unless
	# already installed.  Use [arg -force] to reinstall.

	# Different versions of dbg install in different locations.
	# The default should be correct, but you can use -kdpath if
	# you want to force use of an older version.

	# In theory, the INSTDIR property can be specified on the
	# msiexec commandline, but this appears to be impossible to
	# use from bash due to inconsistent quoting requirements.
    }

    method install_Debugging_Tools_for_Windows {args} {
	UTF::Getopts {
	    {force "Install, even if already installed"}
	}
	if {$options(-kdpath) eq "kd.exe"} {
	    # Bail out if using kd on path - needs cleanup
	    return
	}
	if {$(force) || [catch {$self -noinit test -e '$options(-kdpath)'}]} {
	    $self copyto [glob $archive/dbg_*.msi{,.gz}] dbg_.msi

	    set old {C:\Program Files\Debugging Tools for Windows\kd.exe}

	    # Allow for 2 loops.  This is because the first time
	    # around we may not be able to uninstall an old version,
	    # but we can upgrade it.  If we do upgrade it we will end
	    # up with the old paths, but now we should be able to
	    # uninstall it and get the right paths.
	    for {set i 0} {$i < 2} {incr i} {
		if {$options(-kdpath) ne $old &&
		    ![catch {$self -n test -e '$old'}]} {
		    UTF::Message WARN $options(-name) \
			"Try to uninstall a copy in the wrong place"
		    catch {$self rexec -noinit msiexec /quiet /x dbg_.msi}
		}
		$self rexec -noinit msiexec /quiet /i dbg_.msi /log dbg_.log
		if {![catch {$self -noinit test -e '$options(-kdpath)'}]} {
		    return
		}
	    }
	    $self cat dbg_.log
	    error "kd install failed, or incorrect -kdpath"
	}
    }

    UTF::doc {
	# [call [arg host] [method kd] [lb][arg file][rb]]

	# Attempt to run Windows kernel debugger on Windows dump file
	# [arg file].  [arg file] defaults to C:\WINDOWS\MEMORY.DMP.
	# The driver symbols will be retrieved from the last driver
	# installed by UTF.  If these do not match the driver that was
	# being used at the time of the crash then the dump analysis
	# will give incorrect results.
    }

    method kd {args} {
	UTF::Getopts [subst {
	    {dmp.arg "$options(-memorydumpfile)" "Memory Dump"}
	    {driver.arg "/tmp/DriverOnly" "Driver Files"}
	}]

	if {$options(-coreserver) ne ""} {
	    # If we cannot run kd ourselves (eg no Internet conection)
	    # then copy the dump to a system that can and run kd
	    # there.  Need to copy from DUT to controller, then to
	    # Coreserver since we may not be able to access Coreserver
	    # directly from DUT.
	    set localkd /tmp/kd_$::tcl_platform(user)
	    file mkdir $localkd
	    $self copyfrom /tmp/DriverOnly/*.pdb $localkd/
	    $self copyfrom $dmp $localkd/

	    $options(-coreserver) mkdir -p /tmp/remotekd
	    $options(-coreserver) rsync $localkd/ /tmp/remotekd/
	    return [$options(-coreserver) kd \
			-driver /tmp/remotekd -dmp /tmp/remotekd/MEMORY.DMP]
	}

	$self install_Debugging_Tools_for_Windows

	$self -noinit mkdir -p /tmp/crash
	set c [$self cygpath -w /tmp/crash]
	set do [$self cygpath -w "'$(driver)'"]
	set dmp [$self cygpath -w $(dmp)]
	set MSFT "SRV*$c*http://msdl.microsoft.com/download/symbols"
	set kdu "\"\$(cygpath -u '$options(-kdpath)')\""
	set cmd "!analyze -v;q"
	set kd [$self rexec -t 120 -noinit \
		    "$kdu -y '$MSFT;$c;$do' -i '$c;$do' -z '$dmp' -c '$cmd'"]
	if {[regexp -line \
		 {ASSERT_DATA:  (.*)\n\nASSERT_FILE_LOCATION:  (.*) at Line (\d+)\n\n} \
		 $kd - data file line]} {
	    return "assertion \"$data\" failed: file \"$file\", line \"$line\""
	} elseif {[regexp -line {Probably caused by (.*)} $kd - probable]||
	    [regexp -line {SYMBOL_NAME:\s+(.*)} $kd - probable]} {
	    if {[regexp {^6} [$self cget -osver]] &&
		[regexp {ndisQuerySupportedGuidToOidList} $probable]} {
		return "Probably caused by $probable (Windows Sustained Engineering bug 262234)"
	    } else {
		return "Probably caused by $probable"
	    }
	} else {
	    return "Unknown crash"
	}
    }

    method whatami {{STA ""}} {
	lassign [$self getosver] osver iam
	if {$osver ne $options(-osver)} {
	    $self warn "osver $osver != $options(-osver)"
	}
	if {$STA ne ""} {
	    if {[catch {$STA chipname} c]} {
		set c "<unknown>"
	    }
	    set iam "$iam $c"
	}
	return $iam
    }


    UTF::doc {

	# [call [arg host] [method pkill] [lb][arg -sig][rb] [arg
	# pattern]]

	# kill process by name or grep regexp.  This can be used for
	# both Cygwin processes and native Windows processes, unlike
	# the Cygwin builtin pkill which only handles Cygwin
	# processes.
    }

    method pkill {args} {
	if {[regexp -- {^-\S+} [lindex $args 0] sig]} {
	    set args [lreplace $args 0 0]
	} else {
	    set sig ""
	}
	set pattern [join $args {\|}]
	if {![catch {$self rexec -n "ps -fW | grep -i '$pattern'"} ret]} {
	    foreach line [split $ret "\n"] {
		lappend pids [lindex $line 1]
	    }
	    if {[info exists pids]} {
		$self rexec -n "/bin/kill $sig -f $pids"
	    }
	}
    }

    variable cleaninfs_dl 0
    method cleaninfs {{node {}}} {
	# Cleans up unused oem*.inf packages.
	if {!$cleaninfs_dl} {
	    $self copyto $UTF::unittest/etc/cleaninfs.sh \
		/usr/local/bin/cleaninfs.sh
	}
	set cleaninfs_dl 1
	regsub {\&SUBSYS.*} $node {} devid
	if {[catch {$self rexec -n -T 120 /usr/local/bin/cleaninfs.sh '$devid'} ret]} {
	    set e $::errorInfo
	    if {[regexp {Timeout$} $ret]} {
		$self warn "Cleaninfs timed out"
	    } else {
		error $ret $e
	    }
	}
	return
    }

    UTF::doc {
	# [call [arg host] [method defsysfile]]

	# Guess the default .sys driver filename, based on [option -osver].
    }

    typevariable ndismap -array {
	5 5	564 564
	6 6	664 664
	7 6	764 664
	8 63	864 63a
	81 63	8164 63a
	9 63	964 63a
	10 63	1064 63a
    }

    method defsysfile {} {
	if {[info exists ndismap($options(-osver))]} {
	    return "bcmwl$ndismap($options(-osver)).sys"
	} else {
	    error "Unknown OS version: $options(-osver)"
	}
    }

    method getosver {{ver ""} {arch ""}} {

	if {$ver eq ""} {
	    if {![regexp {\[Version ([.\d]+)\]\s+(\S+)} [$self cmd /c "ver;arch"] \
		      - ver arch]} {
		error "Unknown OS Version"
	    }
	}
	if {[UTF::ddcmp $ver 6.4] >= 0} {
	    set osver 10
	    set name "10"
	} elseif {[UTF::ddcmp $ver 6.3] >= 0} {
	    set osver 81
	    set name "8.1"
	} elseif {[UTF::ddcmp $ver 6.2] >= 0} {
	    set osver 8
	    set name "8"
	} elseif {[UTF::ddcmp $ver 6.1] >= 0} {
	    set osver 7
	    set name "7"
	} elseif {[UTF::ddcmp $ver 6.0] >= 0} {
	    set osver 6
	    set name "Vista"
	} elseif {[UTF::ddcmp $ver 5.1] >= 0} {
	    set osver 5
	    set name "XP"
	} else {
	    error "OS Version $ver older than WinXP not supported"
	}
	if {[regexp {64} $arch]} {
	    append osver 64
	    append name "x64"
	}
	UTF::Message INFO $options(-name) "Win$name (osver=$osver)"
	list $osver "Win$name"
    }

    UTF::doc {

	# [call [arg host] [method closenetloc]]

	# Close any "Set Network Location" windows which might be
	# littering the desktop on WinVista and Win7.  These are
	# typically caused by the random SSID changes used in the
	# Afterburner tests.
    }

    method closenetloc {} {
	$self rexec -x "nircmd win close title \"Set Network Location\""
    }


    method {wdi_trace start} {} {
	$self -x logman -ets stop LwtNetLog
	$self netsh trace start wireless_dbg \
	    persistent=yes \
	    provider={21ba7b61-05f8-41f1-9048-c09493dcfe38} \
	    provider={c3dc72b1-b3d9-4ca0-a066-d35241eeabbb} \
	    level=0xff keywords=0xff
    }

    method {wdi_trace stop} {} {
	$self -t 120 netsh trace stop
	if {[info exists ::UTF::Logdir]} {
	    set d $UTF::Logdir
	} else {
	    set d "."
	}
	set f [exec mktemp $d/NetTrace_XXXXX.etl]
	$self copyfrom [file join [file dir [$self cygpath -D]] \
			    AppData Local Temp NetTraces NetTrace.etl] $f.gz
	file attributes $f.gz -permissions a+r
	file tail $f.gz
    }


    variable imgstore ""
    method imgstore {} {
	if {$imgstore eq ""} {
	    set imgstore [file tail [file dirname [$self -n ls /cygdrive/*/hddimgstorebrcm.id]]]
	} else {
	    set imgstore
	}
    }

    method OSCopyto {{imgdir ""}} {

	set imgbase "/projects/hnd_sig_ext13/UTF_Ready_OS_Images"
	set pedisk [$self imgstore]

	if {$imgdir eq ""} {
	    switch $options(-osver) {
		8 {set imgdir "$imgbase/win8x86"}
		864 {set imgdir "$imgbase/win8x64"}
		81 {set imgdir "$imgbase/win81x86u1"}
		8164 {set imgdir "$imgbase/win81x64u1"}
		9 {set imgdir "$imgbase/win_next_x86*"}
		964 {set imgdir "$imgbase/win_next_x64*"}
		10  {set imgdir "$imgbase/win10x86"}
		1064 {set imgdir "$imgbase/win10x64"}
		default {
		    error "OS update not supported for osver $(options-osver)"
		}
	    }
	}
	set imgdir [lindex [lsort [glob $imgdir]] end]
	if {![file isdirectory $imgdir]} {
	    error "OS image $imgdir not found"
	}

	$self copyto $imgbase/Scripts/LoadOS.bat /cygdrive/$pedisk/LoadOS.bat
	foreach i {dwi  fciv  syscfg} {
	    $self copyto $imgbase/Tools/$i "/cygdrive/$pedisk/$i.exe"
	}
	$self -n mkdir -p /cygdrive/$pedisk/images/update
	if {![catch {$self -n stat -c %s /cygdrive/$pedisk/images/update/*} ret] &&
	    $ret eq [exec stat -c %s {*}[glob $imgdir/*]]} {
	    UTF::Message INFO $options(-name) "Image up to date"
	    return
	}
	localhost rexec -n -t 300 -T 3600 "/usr/bin/rsync -Pprlv --partial -e '[$self cget -ssh]' $imgdir/ [$self cget -user]@[$self cget -lan_ip]:/cygdrive/$pedisk/images/update"
    }

    method OSUpdate {{imgdir ""}} {
	set pedisk [$self imgstore]
	$self OSCopyto $imgdir

	$self rexec -n -t 60 \
	    "/cygdrive/$pedisk/dwi.exe 'folder=$pedisk:\\images\\update' loadimage=yes"
    }


    # MFGC tools Support

    UTF::doc {

	# [call [arg host] [method startmfgc] [arg cmd]]

	# [lb]re[rb]start MFGC tools on MFG/DVT test rig systems.
	# "cmd" should be "mfcremote" for REFs and Controllers,
	# "mfcmfgc" for DUTs.  If the process exits within 2 seconds
	# its log file output will be reported as an error.
    }

    method startmfgc {cmd} {
	$self pkill $cmd
	$self rexec -n \
	    "\"$options(-mfgcpath)/Binaries/$cmd\" >/tmp/$cmd.log 2>&1&"
	if {[catch {$self rexec -n "ps -efW | grep $cmd"}]} {
	    UTF::Sleep 2
	    if {[catch {$self rexec -n "ps -efW | grep $cmd"}]} {
		catch {$self rexec -s -n cat /tmp/$cmd.log} ret
		error "$cmd failed to start: $ret"
	    }
	}
    }

    UTF::doc {

	# [call [arg host] [method mfgcshell] [arg cmd]]

	# Runs MFGC command [cmd cmd] on DUT via the MFGC tcp
	# socket.
    }

    method mfgcshell {cmd} {
	if {[set ip [$self cget -lan_ip]] eq ""} {
	    set ip [$self cget -name]
	}
	localhost rexec -noinit \
	    "$UTF::unittest/mfgcshell $ip \"$cmd\""
    }

    variable mfgcsettingsfile ""
    variable mfgcsettings

    UTF::doc {

	# [call [arg host] [method {mfgc findsettingsfile}]]

	# Returns the path to the MFGC settings file by querying the
	# registry.
    }

    method {mfgc findsettingsfile} {} {
	set mfgcsettingsfile \
	    [$self -n "cygpath -sm \
		 \"\$(regtool.exe get \
		       /HKCU/Software/Broadcom/mfgc/MFGC/Settings_File)\""]
    }

    UTF::doc {

	# [call [arg host] [method {mfgc loadsettings}]]

	# Loads the MFGC settings file as a TCL array.
    }

    method {mfgc loadsettings} {} {
	if {$mfgcsettingsfile eq ""} {
	    $self mfgc findsettingsfile
	}
	set data [$self -n "cat '$mfgcsettingsfile'"]
	foreach {- key value} [regexp -line -all -inline {(.*) = (.*)} $data] {
	    set mfgcsettings($key) $value
	}
    }

    UTF::doc {

	# [call [arg host] [method {mfgc changesetting}] [arg key] [arg value]]

	# Changes a key and value in the MFGC settings file.  Results
	# are saved to disk (to be picked up by MFGC next time it
	# restarts)
    }

    method {mfgc changesetting} {key value} {
	if {![array size mfgcsettings]} {
	    $self mfgc loadsettings
	}
	set mfgcsettings($key) $value
	set file "/tmp/mfgsettings_${::tcl_platform(user)}.txt"
	UTF::Message INFO $options(-name) "$key = $value"
	set fd [open $file w]
	foreach key [lsort [array names mfgcsettings]] {
	    puts $fd "$key = $mfgcsettings($key)\r"
	}
	close $fd

	$self copyto $file $mfgcsettingsfile
	file delete $file
    }

    UTF::doc {

	# [call [arg host] [method {mfgc getsetting}] [arg key]]

	# Retrieve a value from the MFGC settings array.  Need to run
	# [method {mfgc loadsettings}] first to populate the array.
    }

    method {mfgc getsetting} {key} {
	return $mfgcsettings($key)
    }

    UTF::doc {

	# [call [arg host] [method {mfgc findlogfile}]]

	# Retrieve a location of the current logfile.  The %DT keyword
	# is expanded with the current date.  %SN is removed.
    }

    method {mfgc findlogfile} {} {
	set date [clock format [clock seconds] -format {%Y%h%d}]
	if {![info exists mfgcsettings]} {
	    $self mfgc loadsettings
	}
	set file $mfgcsettings(/mfgc_settings/general/log_filename)
	set file [string map [list "%DT" $date "%SN" {}] $file]
	set file [$self -n "cygpath '$file'"]
    }

    UTF::doc {

	# [call [arg host] [method {mfgc script}]]

	# Retrieve the location of the DVTc script file, by combining
	# the [arg -mfgcscriptdir] and [arg -mfgcscript] options,
	# expanding any [cmd cygpath] expressions, and converting to
	# Windows path format.
    }

    method {mfgc script} {} {
	$self cygpath -w "\"$options(-mfgcscriptdir)/$options(-mfgcscript)\""
    }

    # Method to copy over the MFGC release.  Needs work.
    method copy_mfgc {tag} {
	foreach f [glob /projects/hnd_mfg/MFGC_RELEASE/$tag/*.exe] {
	    $self copyto  $f "'[file join [$self cygpath -smD] {MFGc\ Releases}]'"
	}
    }

    UTF::PassThroughMethod serialrelay -serialrelay

}

# Retrieve manpage from last object
UTF::doc [UTF::Cygwin man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]
    package require UTF
    UTF::Cygwin Laptop -lan_ip 10.19.12.198 -sta {STA2}
    STA2 wl status
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
    # [see_also [uri APdoc.cgi?UTF::Linux.tcl UTF::Linux]]
    # [see_also [uri APdoc.cgi?UTF::Router.tcl UTF::Router]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
