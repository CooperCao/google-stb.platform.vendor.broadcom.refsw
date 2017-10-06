#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::WinBT 2.0

package require snit
package require UTF::doc
package require UTF::Base
package require UTF::Cygwin
package require UTF::utils

UTF::doc {
    # [manpage_begin UTF::WinBT n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF WinBT/Windows BlueTooth Cygwin support}]
    # [copyright {2009 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]

    # UTF::WinBT is an implementation of the UTF host object, specific
    # to Windows systems running BlueTooth and Cygwin.

    # [list_begin definitions]

}

# Common procs (not methods) need to be before the SNIT object header.
UTF::doc {
    # [call [cmd map_bt_packet_type] [arg pkt_type]]

    # This method maps the BT desired packet type to the correct list
    # of exclude and include packet types to be send to BlueTool. It
    # seems that if you dont exclude some packet types, then BlueTooth
    # feels free to switch packet types dynamically as it sees fit
    # without necessarily telling you.[para]

    # Returns: underscore separated list of packet types.
}

proc map_bt_packet_type {pkt_type} {

    # Define the list of known packet types and the mappings of
    # exclude include packet types needed to make BlueTooth use this
    # specific packet type and no others.
    set pkt_map {
	{DM1 "no*2-DH1 no*2-DH3 no*2-DH5 no*3-DH1 no*3-DH3 no*3-DH5 DM1"}
	{DM3 "no*2-DH1 no*2-DH3 no*2-DH5 no*3-DH1 no*3-DH3 no*3-DH5 DM3"}
	{DM5 "no*2-DH1 no*2-DH3 no*2-DH5 no*3-DH1 no*3-DH3 no*3-DH5 DM5"}
	{DH1 "no*2-DH1 no*2-DH3 no*2-DH5 no*3-DH1 no*3-DH3 no*3-DH5 DH1"}
	{DH3 "no*2-DH1 no*2-DH3 no*2-DH5 no*3-DH1 no*3-DH3 no*3-DH5 DH3"}
	{DH5 "no*2-DH1 no*2-DH3 no*2-DH5 no*3-DH1 no*3-DH3 no*3-DH5 DH5"}
	{2-DH5 "no*2-DH1 no*2-DH3 no*3-DH1 no*3-DH3 no*3-DH5"}
	{3-DH5 "no*2-DH1 no*2-DH3 no*2-DH5 no*3-DH1 no*3-DH3"}
	{AUTO "DM1 DM3 DM5 DH1 DH3 DH5"}
	{ACL "DM1 DM3 DM5 DH1 DH3 DH5"}
	{SCO HV3}
	{ESCO "no*2-EV5 no*3-EV3 no*3-EV5"}
	{BDR "no*2-DH1 no*3-DH1 DM1 DH1 no*2-DH3 no*3-DH3 DM3 DH3 no*2-DH5 no*3-DH5 DM5 DH5"}
	{EDR 0}
    }

    # Search pkt_map for match.
    set pkt_type_uc [string toupper $pkt_type]
    foreach pair $pkt_map {
	set type [lindex $pair 0]
	set list [lindex $pair 1]
	# puts "type=$type list=$list"
	if {$pkt_type_uc == $type} {
	    # Make list items underscore separated.
	    regsub -all {\s} $list "_" list
	    return $list
	}
    }

    # No match occured. Return the calling pkt_type in the faint hope
    # that it might be a valid type that we dont know about yet.
    UTF::Message WARN $::localhost "map_bt_packet_type No mapping\
            found for: $pkt_type"
    return $pkt_type
}

# Routine is used to ask user simple questions interactively.
# Returns 0 for no, 1 for yes response. Default is yes.
proc prompt {msg quiet} {
    # User may choose to suppress prompts
    set quiet [string trim $quiet]
    if {$quiet != ""} {
	return 1
    }

    # Prompt the user repeated for a response.
    # When running in batch mode, the gets recieves
    # a null input, which is mapped to yes. This
    # ensures that setup will do everything by default
    # when run in batch mode.
    while {1} {
	puts "$msg Y/n "
	gets stdin resp
	set resp [string trim $resp]
	set resp [string tolower $resp]
	set resp [string range $resp 0 0]
	# puts "resp=$resp"
	if {$resp == "" || $resp == "y"} {
	    return 1
	} elseif {$resp == "n"} {
	    return 0
	}
    }
}

snit::type UTF::WinBT {
    UTF::doc {

        # [call [cmd UTF::WinBT] [arg host]
        # 	[lb][option -sta] [arg {{STA dev ...}}][rb]
        #       [lb][option -image] [arg driver][rb]
        #       [lb][option -tag] [arg tag][rb]
        #       [lb][option -date] [arg date][rb]
        #       [lb][option -type] [arg type][rb]
        #       [lb][option -version] [arg version][rb]
        #       [lb][option -brand] [arg brand][rb]
        #       [lb][option -file] [arg file][rb]
        #       [lb][option -archive] [arg path][rb]
        #       [lb][option -active_perl] [arg path][rb]
        #       [lb][option -bluetool] [arg path][rb]
        #       [lb][option -perl_scripts] [arg path][rb]
        #       [lb][option -bt_comm] [arg comN@speed][rb]
        #       [lb][option -bt_comm_startup_speed] [arg start_speed][rb]
        #       [lb][option -bt_driver_path] [arg path][rb]
        #       [lb][option -bt_init_cmds] [arg cmd_list][rb]
        #       [lb][option -bt_hw_addr] [arg hex_digit_string][rb]
        #       [lb][option -bt_rw_mode] [arg string][rb]
        #       [lb][option -bt_ds_location] [arg hex_digit_string][rb]
        #       [lb][option -bt_ss_location] [arg hex_digit_string][rb]
        #       [lb][option -bt_w_size] [arg integer][rb]
        #       [lb][option -bt_xtal_freq] [arg decimal_number][rb]
        #       [lb][option -bt_power_adjust] [arg integer][rb]
        #       [lb][option -project_dir] [arg project_dir][rb]
        #       [arg ...]][para]

        # Create a new Windows/BlueTooth Cygwin host object.  The host
        # will be used to contact STAs residing on that host.
        # [list_begin options]

        # [opt_def [option -sta] [arg {{STA dev ...}}]]

        # List of STA devices to configure for this host.  If there is
        # more than one wireless interface on the system, the devices
        # should be listed as [arg {name device}] pairs.  If there is
        # only one wireless device on the system, the device number
        # can be omitted.

        # [example_begin]
        -sta {WL0 0 WL1 1}
        -sta {WL}
        # [example_end]

        # [opt_def [option -image] [arg driver]]

        # Specify a default driver to install when [method load] is
        # invoked.  This can be an explicit path to a [file *.cgr] or
        # a suitable list of arguments to [method findimages].

        # [opt_def [option -tag] [arg tag]]

        # Select a build tag. Currently BlueTooth doesnt have software
        # streams like HND. This option is here for future
        # expansion. Currently this option does nothing. Default is *.

        # [opt_def [option -date] [arg date]]

        # Select a build date, eg: 2009.1.26. BlueTooth builds dont
        # include dates in the pathname. This option allows you to
        # select the latest build that was available on the specified
        # date. The modify time of the build files are checked to find
        # the appropriate file. Default is *.

        # [opt_def [option -type] [arg type]]

        # Select a build type.  Build types usually specify a board
        # number and revision code. Default is BCM4325/D0

        # [opt_def [option -version] [arg version]]

        # Select a build version. Specific values look like
        # BCM4325D0_004.001.0104.000. Default is *, which finds the
        # latest version.

        # [opt_def [option -brand] [arg brand]]

        # Specify build brand.  Default is NoCodePatches26MHz/Class1_5

        # [opt_def [option -file] [arg file]]

        # Select a build file. Default is *.cgr

        # [opt_def [option -archive] [arg path]]

        # Used by setup method to locate relatively static programs.
        # Default is "/projects/hnd_archives/win/UTF"

        # [opt_def [option -active_perl] [arg path]]

        # Specify path for Windows ActivePERL on the target PC.
        # Default is c:/perl/bin[para]

        # NB: BlueTool doesnt want the Cygwin version of PERL.[para]

        # NB: changing this path is only effective for manually
        # installations of Perl. If the setup method is used to
        # install Perl, it will be installed in the default location,
        # regardless of what this option is set to.[para]

        # NB: If you dont want ActivePERL to be installed, set this
        # option to null.

        # [opt_def [option -bluetool] [arg path]]

        # Specify path for BlueTool executable on the target PC.
        # Default is c:/\\\"Program
        # Files\\\"/Broadcom/BlueTool/BlueTool.exe[para]

        # NB: changing this path is only effective for manually
        # installations of BlueTool. If the setup method is used to
        # install Perl, it will be installed in the default location,
        # regardless of what this option is set to.[para]

        # NB: If you dont want BlueTool to be installed, set this
        # option to null.[para]

        # NB: put escaped double quotes around \\\"Program Files\\\"
        # or Cygwin will complain about the path![para]

        # NB: put c: in the path, or stat & mv commands will fail!

        # [opt_def [option -perl_scripts] [arg path]]

        # Specify path for perl scripts used by BlueTool on the target
        # PC. Default is c:/perl_scripts NB: If you dont want perl
        # scripts to be installed, set this option to null.

        # [opt_def [option -bt_comm] [arg comN@speed]]

        # Specify the com port and speed on the target PC to use when
        # accessing the BlueTooth device, eg: "com3@3000000" or
        # "usb0".  Default is null.[para]

        # NB: If this parameter is not specified in your config file,
        # the constructor will throw an error!

        # [opt_def [option -bt_comm_startup_speed] [arg start_speed]]

        # Some devices require the use of a lower startup speed. After
        # the device is loaded loaded, the baud rate can be increased
        # to the normal operating speed. If the device requires a
        # lower startup speed, this option can be used to specify the
        # lower startup speed, eg: "115200". Default is null.[para]

        # [opt_def [option -bt_driver_path] [arg path]]

        # Specify the path on the target PC to store the driver file.
        # Default is c:/bt_driver

        # [opt_def [option -bt_init_cmds] [arg cmd_list]]

        # Specify an optional list of perl script commands to run
        # after the device is loaded. Default is null.

        # [opt_def [option -bt_hw_addr] [arg hex_digit_string]]

        # By default, the scripts will set the hardware address the BT
        # device will use on the airwaves to the MAC address used by
        # host PC. This ensures that each BT device will have a
        # globally unique hardware address and will not interfere with
        # other testrigs when the testrig chamber doors are
        # open.[para]

        # However there may be some reason that you dont want this
        # default used. If so, use this option to specify the specific
        # hardware address the BT device will use on the airwaves. The
        # [arg hex_digit_string] may include colon and/or hyphen
        # separators, which will be ignored by the scripts. Eg:
        # 12:34:56:78:90:ab

        # [opt_def [option -bt_rw_mode] [arg string]]

        # For most devices and .cgr/.cgs files, use the default "ARM
        # HCI".  For 4330 and .hcd files, use "Cortex M3 HCI".

        # [opt_def [option -bt_ss_location] [arg hex_digit_string]]

        # This parameter is needed for the download process. The value
        # appears to vary for each chip rev. The following values are
        # currently known:
        # For 4325,   use: 0x00086800
        # For 4329A0, use: 0x00086800
        # For 4329B0, use: 0x00087310
        # For 4329B1, use: 0x00087410
        # For 4330B2, use: 0x00210000

        # [opt_def [option -bt_w_size] [arg integer]]

        # For most devices and .cgr/.cgs files, use the default 251.
        # For 4330 and .hcd files, use "200".

        # [opt_def [option -bt_xtal_frequency] [arg decimal_number]]

        # The crystal frequency is needed for the download process.
        # For 4325, use: 26
        # For 4329, use: 38.4

        # [opt_def [option -bt_power_adjust] [arg integer]]

        # The power adjust is needed for the download process.
        # For 4325, use: 31
        # For 4329, use: 40

        # [opt_def [option -project_dir] [arg project_dir]]

        # Specify project directory path for build location.  Default
        # is /projects/blueth_release/BroadcomInternal

        # [opt_def [arg ...]]

        # Unrecognised options will be passed on to the host's [cmd
        # UTF::Cygwin] object.

        # [list_end]
        # [list_end]

        # [para]
        # WinBT objects have the following methods:
        # [para]
        # [list_begin definitions]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -sta
    option -image
    option -name -configuremethod CopyOption
    option -tag "*"
    option -date "*"
    option -type "BCM4325/D1"
    option -version "*"
    option -brand "NoCodePatches26MHz/Class1_5"
    option -file "*.cgr"
    option -archive "/projects/hnd_archives/win/UTF"
    option -active_perl c:/perl/bin

    # NB: put escaped double quotes around Program Files or Cygwin
    # will complain about the path!
    # NB: put c: in the path, or stat & mv commands will fail!
    option -bluetool "c:/\"Program Files\"/Broadcom/BlueTool/BlueTool.exe"
    option -perl_scripts "c:/perl_scripts"
    option -bt_comm ""
    option -bt_comm_startup_speed ""
    option -bt_driver_path "c:/bt_driver"
    option -bt_init_cmds ""
    option -bt_hw_addr ""
    option -bt_rw_mode "ARM HCI"
    option -bt_ds_location ""
    option -bt_ss_location ""
    option -bt_w_size 251
    option -bt_xtal_freq ""
    option -bt_power_adjust ""
    option -project_dir "/projects/blueth_release/BroadcomInternal"

    # Method CheckOs populates these variables
    variable kernel_rel ""
    variable kernel_ver ""
    variable kernel_name ""
    variable os_ver ""

    # The constructor will populate this variable.  Having the object
    # name available to all methods simplifies coding of the numerous
    # UTF::Messages.
    variable name ""

    # Method load populates this variable for use by method reload.
    variable bt_driver_name ""

    # Method load populates these variables for use by method
    # imageinfo.
    variable imageinfo -array {
	tag ""
	date ""
	type ""
    }

    # Method reload populates this variable
    variable board_address ""

    # Methods show_acl/sco_handles, save_acl/sco_handles &
    # delete_acl/sco_handles manipulate these variables.
    variable acl_handles ""
    variable sco_handles ""

    # cygwin handles any other options and methods
    component cygwin -inherit yes

    # cloneopts holds the original options used to create the object.
    # This may be used to clone a new object.
    variable cloneopts

    # Object constructor uses Cygwin as base package
    constructor {args} {
	set cloneopts $args

        # Use Cygwin as base object
        install cygwin using UTF::Cygwin %AUTO% -init [mymethod init]
        $self configurelist $args

        # Populate the -name option
        if {$options(-name) eq ""} {
            $self configure -name [namespace tail $self]
        }

        # Having the object name available to all methods in an object
        # level variable simplifies coding of the numerous
        # UTF::Messages.
        set name $options(-name)

        # Option -bt_comm must not be null.
        set bt_comm $options(-bt_comm)
        if {$bt_comm == ""} {
            error "$name constructor ERROR: option -bt_comm must have a\
                valid value, eg: \"com3@3000000\" or \"usb0\""
        }

        # Create STA objects for each item in the -sta option list.
	foreach {sta dev} $options(-sta) {
	    # Windows reports MAC addresses in uppercase.  Empty or
	    # numeric device names will be unaffected.
	    UTF::STA ::$sta -host $self -device [string toupper $dev]
	}
    }

    # Object destructor gets rid of children STA objects
    destructor {
	catch {$cygwin destroy}
	foreach {sta dev} $options(-sta) {
	    catch {::$sta destroy}
	}
    }

    # Internal method for copying options to delegates.  If this
    # method is not present, the -name option and possibly other
    # -options will not get initialized correctly.
    method CopyOption {key val} {
	set options($key) $val
	$cygwin configure $key $val
    }

    # Test method to verify that constructor correctly set the object
    # level variable name for each WinBT object.
    method name {} {
        return $name
    }

    UTF::doc {
	# [call [arg host] [method checkOs]]
	# Checks OS release, version & name sets object instance
	# variables.
    }

    method checkOs {} {

        # Checks OS release, version & name sets object instance
        # variables.

        # If kernel_rel is already populated, we are done.
        if {$kernel_rel != ""} {
            # puts "$name checkOs found: kernel_rel=$kernel_rel kernel_ver=$kernel_ver kernel_name=$kernel_name os_ver=$os_ver"
            return
        }

        # Get kernel info from host machine.
        set kernel_rel [$self uname -r]
        set kernel_ver [$self uname -v]
        set kernel_name [$self uname -s]

        # Extract leading digits for os_ver from kernel_name. Ignore
        # the decimal and sub-release digits.
        if {![regexp -nocase {CYGWIN_NT-(\d+)} $kernel_name - os_ver]} {
            set os_ver -1
        }

        # Some of the utils procedures and possibly some of the
        # delegate methods need a -osver value, we give them the real
        # value for -osver.
        $self configure -osver $os_ver

        # Log results
        UTF::Message LOG $name "checkOs  kernel_rel=$kernel_rel\
            kernel_ver=$kernel_ver kernel_name=$kernel_name os_ver=$os_ver"

        # Check for supported versions.
        if {$os_ver < 5 || $os_ver > 6} {
            error "$name checkOs $kernel_name $kernel_rel $kernel_ver not supported by WinBT!"
        }
    }

    UTF::doc {
	# [call [arg host] [method findimages]
	#              [lb][option -all][rb]
	#              [lb][option -ls][rb]
	#              [lb][option -type] [arg type][rb]
	#              [lb][option -version] [arg version][rb]
	#              [lb][option -brand] [arg brand][rb]
	#              [lb][option -file] [arg file][rb]
	#              [lb][option -tag] [arg tag][rb]
	#              [lb][option -date] [arg date][rb]
	#              [lb][option -diag][rb]
	#		   [arg file]]

	# Returns the pathnames of an os image from the BlueTooth
        # specific build repository.  Globbing patterns are accepted
        # in all arguments.

	# [list_begin options]

	# [opt_def [option -all]]

	# Return all matches, sorted by date with the youngest first,
	# then by features.  By default, only the first match is
	# returned.

	# [opt_def [option -ls]]

	# Run ls -l on results.  By default, only the file name is
	# returned.

	# [opt_def [option -type] [arg type]]

	# Build type. Default is [option -type] option of [arg host].

	# [opt_def [option -version] [arg version]]

	# Build version. Default is [option -version] option of [arg
	# host].

	# [opt_def [option -brand] [arg brand]]

	# Specify build brand.  Default is the [option -brand] option
	# of [arg host].

	# [opt_def [option -tag] [arg tag]]

	# Select a build tag.  Default is [option -tag] option of [arg
	# host]. Currently this option does nothing, as BlueTooth does
	# not have multiple streams as HND does.

	# [opt_def [option -date] [arg date]]

	# Select a build date, eg [file 2009.1.26.0].  Default is
	# null.

        # [opt_def [option -diag]]

        # The [option -diag] option helps figure out why images are
        # not found by checking each subdirectory in the desired tree
        # exists.

	# [opt_def [arg file]]

	# Specify the file type being searched for.

	# [arg file] should be the name of the driver *.hcd

	# [list_end]

    }

    method findimages {args} {

        # The args logic is rather awkward for a routine that is
        # trying to be generic and capable of finding images for the
        # BlueTooth device.

        # Massage calling args. For compatible syntax with other
        # objects, this logic is left here.
	if {$args == "" || $args == "-all"} {
	    set args [concat $args $options(-image)]
	}
        # puts "self=$self args=$args"

        # Define supported options that have no value token.
	set optlist {
	    {all "return all matches"}
            {diag "verify each subdirectory in the desired tree exists"}
	    {ls "Report \"ls -l\" information"}
	}

	# Get default values from the object instance and add more
        # supported options.
	lappend optlist [list type.arg $options(-type) "Build Type"]
	lappend optlist [list version.arg $options(-version) "Build version"]
	lappend optlist [list brand.arg $options(-brand) "brand"]
	lappend optlist [list tag.arg $options(-tag) "Build Tag"]
	lappend optlist [list date.arg $options(-date) "Build Date"]
        # puts "optlist=$optlist"

        # For BT reference boards, there are no regular builds.
        if {[$self is_ref_board $bt_driver_name]} {
            UTF::Message WARN $name "This is a BT reference board,\
                there are no regular builds to load."
            return
        }

	# Findimages start
	UTF::Message LOG $name "findimages $args"
	if {[catch {cmdline::getoptions args $optlist \
			"findimages options"} ret]} {
	    error $ret
	} else {
	    array set pargs $ret
	}
        # parray pargs ;# debug trace code

        # Validate project_dir exists.
        set project_dir $options(-project_dir)
        # puts "project_dir=$project_dir"
        if {![file isdirectory $project_dir]} {
            error "$name findimages ERROR: project_dir=$project_dir not found!"
        }

        # Validate project_dir is readable by your userid.
        if {![file readable $project_dir]} {
            error "$name findimages ERROR: project_dir=$project_dir is not readable by you!"
        }

        # Check the real OS of the object. Currently not needed, as
        # there are no host drivers or host specific files. If this
        # situation should change, uncomment the line below.\
        # $self checkOs

        # Object may have specific file stored in -image, which is
        # appended to args. If so, this file is used.
	set file [lindex $args end]
	if {$file eq ""} {
            set file $options(-file)
	} elseif {[file exists $file]} {
            puts "found file=$file"
	    return $file
	}
        # puts "file=$file"

        # BlueTooth group does not have an ARCHIVED or PRESERVED
        # folder.

        # Handle special date values
        set date $pargs(date)
        set date_sec 0
        if {[string match -nocase "*current*" $date] ||
            [string match -nocase "*today*" $date] || $date == "*"} {
           set date ""
        }
        # puts "self=$self date=$date"

        # To simulate a date search, we collect all matching files.
        # with the "ls -l" info.
        if {$date != ""} {
            set pargs(all) 1

            # Convert WLAN yyyy.mm.dd date format to mm/dd/yyyy to
            # keep scan function happy. User may specify only yyyy or
            # yyyy.mm.
            set orig_date $date
            regsub {\*} $date "" date
            set date [split $date "\."]
            set yy [lindex $date 0]
            set mm [lindex $date 1]
            if {$mm == "" || $mm == "0"} {
                set mm 12
            }
            set dd [lindex $date 2]
            if {$dd == "" || $dd == "0"} {
                set dd 28
            }
            set date "$mm/$dd/$yy"
            if {[catch {set date_sec [clock scan $date]} catch_msg]} {
                error "$name findimages ERROR: invalid date $orig_date, should be\
                   in format yyyy.mm.dd! catch_msg=$catch_msg"
            }
        }
        # puts "self=$self date=$date date_sec=$date_sec"

        # Currently the option -tag does nothing, as BlueTooth does
        # not have multiple streams as HND does.
        if {$pargs(tag) != "" && $pargs(tag) != "*"} {
            UTF::Message WARN $name "findimages ignoring\
            -tag $pargs(tag), as BlueTooth does not have multiple streams!"
        }

        # NB: The "file join" function has a very interesting feature,
        # whereby any intermediate token that starts with "/" will
        # cause all preceeding tokens to be ignored. For this reason
        # we do own join that wont trash parts of the string. We dont
        # make a list from the tokens to be joined, as that can add
        # extra "/" to the pattern when a we have a null/blank token.
        set pattern [join [list $project_dir $pargs(type) $pargs(version) \
			       $pargs(brand) $file] "/"]
        # puts "pattern=$pattern"

        # If requested, do diagnostic tests on the pattern.
        if {$pargs(diag)} {
            UTF::check_image_pattern $pattern
        }

        # Use SortImages routine to find matching files. Use the -bt
        # option to correctly parse & sort on BlueTooth build version.
        set file_list [UTF::SortImages [list $pattern] \
			   {*}[expr {$pargs(ls)?"-ls":""}] \
			   {*}[expr {$pargs(all)?"-all":""}] -bt]

        # If we are not doing a date search, just return the file_list.
        if {$date == ""} {
            return $file_list
        }

        # Now we parse the file_list, looking for a file with a modify
        # date that is less or equal the specified date. That gives
        # the latest file that would have been used on that date in
        # time.
        UTF::Message LOG $name "looking for date: $date"
        set temp_list [split $file_list "\n"]
        foreach item $temp_list {

            # Get the item filename. If "ls -l" information is included,
            # we need to ignore it.
            if {$pargs(ls)} {
               set item_file [lindex $item end]
            } else {
               set item_file $item
            }

            # Get mtime of file in seconds, convert to date, then
            # convert back to standard Unix time in seconds
            # again. This drops any HH:MM:SS from the time. This
            # allows for a more appropriate match with the specified
            # date, which does NOT include HH:MM:SS.
            set item_sec [file mtime $item_file]
            set item_date [clock format $item_sec -format %m/%d/%Y]
            set item_sec [clock scan $item_date]
            UTF::Message LOG $name "$item_file $item_date"

            # If file date is less or equal the specified date, we are
            # done.
            if {$item_sec <= $date_sec} {
                return $item
            }
        }

        error "$name findimages ERROR: No files match date $date"
    }


    UTF::doc {
        # [call [arg host] [method reload]]

        # Uses the existing driver file on the target PC in order to
        # restart the BlueTooth driver.
    }

    method reload {} {

        # The BlueTooth device does not have a serial console port.
        # Consequently, there is no need to worry about trap messages
        # or blocking concurrent reload requests.

        # Basic setup
        UTF::Message LOG $name "Reload BlueTooth driver"
        set bt_driver_path [$self cget -bt_driver_path]
        set bt_comm [$self cget -bt_comm]
        set orig_bt_comm $bt_comm ;# save original speed
        set is_custom 0 ;# initial local variable for custom board check

        # Optionaly use lower startup speed. This applies only to
        # ports using -bt_comm in the format comN@mmmmm.
        set startup_speed [$self cget -bt_comm_startup_speed]
        # set bt_comm "usb5" ;# test code
        # set startup_speed 77 ;# test code
        if {$startup_speed != ""} {
            if {[regexp "@" $bt_comm]} {
                # For comN@mmmmm format, replace the original speed
                # with the startup speed.
                set bt_comm [split $bt_comm "@"]
                set normal_speed [lindex $bt_comm 1]
                set bt_comm "[lindex $bt_comm 0]@$startup_speed"
                UTF::Message LOG $name \
		    "reload using startup speed -bt_comm=$bt_comm"

            } elseif {[regexp -nocase {usb\d+} $bt_comm]} {
                # For usbN, give user warning that startup_speed is being ignored.
                UTF::Message WARN $name \
		    "Ignoring -bt_comm_startup_speed=$startup_speed,\
                    it does not apply to ports addressed by usbN"

            } else {
                # Unknown bt_comm format ==> ERROR
                error "$name reload ERROR: Invalid format for -bt_comm=$bt_comm,\
                    should be \"comN@mmmmm\" or \"usbN\""
            }
        }

        # Set perl script subdirectory to use based on BT board type.
        set is_ref_board [$self is_ref_board $bt_driver_name]
        set perl_subdir [$self perl_subdir]
        # puts "is_ref_board=$is_ref_board perl_subdir=$perl_subdir"

        # If method load was not previously called, the object level
        # variable for the driver name will not have the required
        # info.
        if {$bt_driver_name == "" && $is_ref_board == 0} {

            # Go see what driver files are on the target PC.
            set pattern "$bt_driver_path/$options(-file)"
            # puts "catch_resp=$catch_resp catch_msg=$catch_msg"
            if {[catch {
		set driver_list [$self rexec ls $pattern]
	    }]} {
		error "$name reload ERROR: No drivers in: $pattern $catch_msg"
	    }
	    set bt_driver_name [lindex $driver_list 0] ;# take first one
	    set bt_driver_name [file tail $bt_driver_name] ;# get file name only
	    UTF::Message LOG $name "reload using: $bt_driver_name"
        }

        # If possible, do a hardware reset on the BlueTooth device.
        set device_reset [$self cget -device_reset]
        set power_sta [$self cget -power_sta]
        # puts "device_reset=$device_reset power_sta=$power_sta"
        if {$device_reset != ""} {
            $self device_reset

        } elseif {$power_sta != ""} {
            $self power_sta off
            UTF::Sleep 10 $name "wait for external power supply to discharge so\
                BlueTooth device will be reset"
            $self power_sta on

        } else {
           UTF::Message WARN $name "Both options -power_sta & -device_reset\
               are null, so no automated hardware reset of the BlueTooth\
               device is possible. This may cause issues loading the driver."
        }

        # Restart BlueTool
        $self restart_bluetool

        # Do soft reset of BlueTooth device
        $self run_perl_script common/Reset.pl $bt_comm

        # Next 2 steps do not apply to BT reference boards.
        if {$is_ref_board == 0} {

            # Download the mini-driver
            $self run_perl_script $perl_subdir/download_mini_driver.pl $bt_comm

            # For devices that are downloaded, the ss_location is
            # required.  A different value is needed for each revision
            # of each chip.
            
            # add check for -bt_ds_location used for custom board
            if { [$self cget -bt_ds_location] != "" } {
	            set ds_location [$self cget -bt_ds_location]
	            if {![regexp -nocase {^0x[a-f,0-9]{8}$} $ds_location]} {
	                UTF::Message WARN $name "reload -bt_ds_location=$ds_location\
	                    is not of the format: 0x<8 hex digits>, this may cause\
	                    the download to fail."
	            }
	            set is_custom 1
            } else {
	            set ss_location [$self cget -bt_ss_location]
	            if {![regexp -nocase {^0x[a-f,0-9]{8}$} $ss_location]} {
	                UTF::Message WARN $name "reload -bt_ss_location=$ss_location\
	                    is not of the format: 0x<8 hex digits>, this may cause\
	                    the download to fail."
	            }
	
	            # For devices that are downloaded, the xtal_freq is
	            # required.  A different value is needed for each revision
	            # of each chip.
	            set xtal_freq [$self cget -bt_xtal_freq]
	            if {![regexp -nocase {^[\.0-9]+$} $xtal_freq]} {
	                UTF::Message WARN $name "reload -bt_xtal_freq=$xtal_freq\
	                    is not a decimal number, this may cause the download to fail."
	            }

            # For devices that are downloaded, the power_adjust is
            # required.  A different value is needed for each revision
            # of each chip.
            set power_adjust [$self cget -bt_power_adjust]
            if {![regexp -nocase {^\d+$} $power_adjust]} {
                UTF::Message WARN $name "reload -bt_power_adjust=$power_adjust\
                    is not an integer number, this may cause the download to fail."
            }
			} ;# end custom board check
			
            # For devices that are downloaded, the board_address is
            # required.
            set bd_address [UTF::get_host_mac_addr $self]
	    # remove colons, hyphens
            regsub -all {[:\-]} $bd_address "" bd_address

            # Download the main driver
            # add custom load type
            if { $is_custom != 1 } {
	            ### added check for hcd type 4/11/14
	            set fType [$self cget -file]
	            if {$fType != "\*\.hcd"} {
		            $self run_perl_script $perl_subdir/download_cfg.pl \
					$bt_comm $bt_driver_path/$bt_driver_name \
					$ss_location $xtal_freq $power_adjust $bd_address \
					\"[$self cget -bt_rw_mode]\" [$self cget -bt_w_size]
	            } else {
	                $self run_perl_script $perl_subdir/download_hcd.pl \
				    $bt_comm $bt_driver_path/$bt_driver_name $ss_location \
				    \"[$self cget -bt_rw_mode]\" [$self cget -bt_w_size]
	            }
	        } else { ;# load custom configuration: cgs/cgr only
		        $self run_perl_script $perl_subdir/download_cfg_custom.pl \
				$bt_comm $bt_driver_path/$bt_driver_name \
				$ds_location
# 				UTF::Sleep 10 ;# long pause before reading bt_version
			}
        }

        # Use the normal high speed baud rate. This applies only to ports using
        # -bt_comm in the format comN@mmmmm.
        if {$startup_speed != ""} {

            # Run the UpdateBaudRate.pl script for non-reference boards.
            if {$is_ref_board == 0 && $is_custom != 1} {
                $self run_perl_script $perl_subdir/UpdateBaudRate.pl \
                    $bt_comm $normal_speed
            }
        }
        set bt_comm $orig_bt_comm
        UTF::Message LOG $name \
	    "reload using normal speed -bt_comm=$bt_comm"

	    # skipped for custom boards
	    if {$is_custom != 1} {
	        # Set the board address. By default it will be set to the MAC
	        # address of the host PC wired Etherenet port to ensure
	        # uniqueness.
	        $self set_board_address
	
	        # Set the board local name to the object name.
	        $self run_perl_script common/WriteLocalName.pl $bt_comm $name
	        
	    } ;# end is_custom check

        # Do optional bt_init_cmds items.
        set cmd_list $options(-bt_init_cmds)
        if {$cmd_list != ""} {
            foreach cmd $cmd_list {
                set script [lindex $cmd 0]
                set parms [lrange $cmd 1 end]
                $self run_perl_script $script $bt_comm $parms
            }
        }

        if {$is_custom != 1} { ;# skip for custom boards for now
	        # Run the Read_Local_Version.pl script for version info.
	        set bt_ver [$self get_bt_version]
	        return $bt_ver
	    }
#     	catch {unset is_custom} ;# remove local variable
		unset is_custom ;# remove local variable
    }

    UTF::doc {
        # [call [arg host] [method load] [arg args]]

        # Load or replace the BlueTooth driver. The argument list will
        # be passed on to [method findimages] to find a driver.  If no
        # arguments are specified, those stored in the [arg host]s
        # [option -image] option will be used instead.  Filenames are
        # relative to the control host and files will be copied to
        # [arg host] as needed. The [option -n] option will just copy
        # the driver files, but not actually load them.
    }

    method load {args} {

	UTF::Message INFO $name "Load BlueTooth Driver args: $args"

        # Look for -n option.
        set noload 0
        if {[regexp {\-n} $args]} {
            set noload 1
            regsub -all {\-n} $args "" args
        }

        # Currently the driver is OS level independant. If that
        # changes, then uncomment the line below.
        # $self checkOs

        # For BT reference boards, there are no regular builds, so
        # certain steps are skipped.
        set is_ref_board [$self is_ref_board $bt_driver_name]

        # If the object has no image specified, OR the user specified
        # some command line args, then we attempt to use the args to
        # find a suitable image.
        # NB: The command line args MUST override the config file -image
        # path or you will be constantly changing the config file.
        set bt_image [$self cget -image]
        if {($bt_image == "" || $args != "") && $is_ref_board == 0} {
            set bt_image [$self findimages {*}$args]
        }

        # Extract short form name of image. Store in object level
        # variable for the benefit of the method reload.
        ### UTF::Message WinBT_DEBUG "" "file option value: [$self cget -file]\n\n"; exit 999
        set bt_driver_name [file tail $bt_image]
        UTF::Message LOG $name "BlueTooth Image: $bt_image"

        # Extract the actual brand from the bt_image. If no arguments
        # were specified, then use the object -brand option.
        set proj_dir $options(-project_dir)
        set temp [file dirname $bt_image]
        if {![regexp {\d+\.\d+\.\d+\.\d+\.\d+[^/]*/(.*)} $temp - brand]} {
            set brand ""
        }
        # puts "proj_dir=$proj_dir brand=$brand"

        # Save selected data in the object level array imageinfo.  We
        # map the BlueTooth build version to the date field.
	if {[regexp "$proj_dir/(.*)/$brand" $bt_image - type_ver]} {
            # puts "type_ver=$type_ver"
            set imageinfo(type) [file dirname $type_ver]
            regexp {(\d+\.\d+\.\d+\.\d+\.\d+)} $type_ver - imageinfo(date)
            set imageinfo(tag) TOT ;# there are no streams in BlueTool
	}
        UTF::Message INFO $name "imageinfo(tag)=$imageinfo(tag)\
            imageinfo(date)=$imageinfo(date) imageinfo(type)=$imageinfo(type)"

        # Subdirectory for BT driver may not be created for us.
        set bt_driver_path [$self cget -bt_driver_path]
        $self mkdir -p $bt_driver_path

        # Dont leave older driver files lying around to confuse the
        # reload method. If user manually renames them to a different
        # filetype, then they will be left alone.
        set bt_type [file extension $bt_driver_name]
        if {$bt_type == "" || $bt_type == "."} {
            set bt_type ".cgr"
        }
        # puts "bt_driver_name=$bt_driver_name bt_type=$bt_type"
        UTF::Message LOG $name "removing old $bt_type files"
        $self rexec rm -f "$bt_driver_path/*$bt_type"

        # Driver name is updated to include brand info for
        # traceability on the target PC. Copy the driver file over to
        # the target PC.
        set bt_driver_name "${brand}_${bt_driver_name}"
        regsub -all {/} $bt_driver_name "_" bt_driver_name
        if {$is_ref_board == 0} {
            $self copyto $bt_image "$bt_driver_path/$bt_driver_name"
            $self chmod 755 "$bt_driver_path/$bt_driver_name"
        }

        # Use the reload method to start up the driver.
        if {$noload} {
            UTF::Message LOG $name "Copied files - not loading"
            return
        } else {
            $self reload
        }
    }

    UTF::doc {
	# [call [arg host] [method imageinfo]]

	# Accesses the object level imageinfo array data.
    }

    method imageinfo {} {
	if {$imageinfo(tag) eq ""} {
	    error "$name imageinfo ERROR: image info not found"
	}
	return [array get imageinfo]
    }


    UTF::doc {
	# [call [arg host] [method unload]]

	# Unload the current BlueTooth driver.
	# Not implemented.
    }

    method unload {} {
        error "$name unload ERROR: not implemented!"
    }

    UTF::doc {
        # [call [arg host] [method setup] [arg args]]

        # Installs / updates any local tools needed on a UTF client.
        # Optional [arg args] are passed transparently to the
        # underlying Cygwin setup method. Interactively prompts to see
        # if user wants to install specific components. Use arg -q to
        # suppress interactive prompts.[para]

        # NB: Any local modifications in the perl_scripts directory
        # get blown away by the setup method.[para]

        # If you dont want to install ActivePERL, then set the
        # -active_perl option on your object to null.[para]

        # If you dont want to install BlueTool, then set the -bluetool
        # option on your object to null.[para]

        # If you dont want to install perl scripts, then set the
        # -perl_scripts option on your object to null.[para]
    }

    method setup {args} {

        # Look for -q flag
        set quiet ""
        if {[regexp {\-q} $args]} {
            set quiet "-q"
            regsub {\-q} $args "" args
        }

        # Check OS to ensure correct value is passed down to Cygwin.
        $self checkOs

        # Run the Cygwin setup method first.
        if {[prompt "\n\nRun standard Windows setup ?" $quiet]} {
            $cygwin setup {*}$args
        } else {
            UTF::Message WARN $name \
		"User requested that Windows setup be skipped!"
        }

        # The remaining items are specific to BlueTool & BlueTooth.

        # Define current BlueTool ActivePERL required version
        set required_perl_version 5.8.4.810
        # set required_perl_version 5.6.1.638 ;# test code

        # Get common archive path
        set archive [$self cget -archive]

        # Set timeout, in seconds for packages installations.
        set install_sec 300

        # Check interactively if user wants ActivePERL installed.
        set resp [$self check_perl_version $required_perl_version]
        if {$resp != "OK" && [prompt "\n\nRun PERL setup ?" $quiet]} {

            # Copy file from archive server to host PC.
            set perl_pkg "ActivePerl-${required_perl_version}-MSWin32-x86.msi"
            set src_path "$archive/$perl_pkg"
            set dest_path "/tmp/$perl_pkg"
            $self copyto $src_path $dest_path

            # On occasion, perl said it installed, but didnt do an
            # upgrade.  Deleting all files in perl/bin helps trigger
            # the desired install behavior. If we try to remove the
            # whole perl/bin in one shot, Cygwin often thinks the
            # directory is use by someone else and cant remove the
            # directory. So we just go for all the files in perl/bin.
            set perl_bin [$self cget -active_perl]
            $self rexec rm -f $perl_bin/*.*

            # We seem to have better luck remove other Perl related directories.
            $self rexec rm -rf $perl_bin/../eg
            $self rexec rm -rf $perl_bin/../html
            $self rexec rm -rf $perl_bin/../lib
            $self rexec rm -rf $perl_bin/../site

            # Launch the perl install package via msiexec as a
            # background process with no GUI prompts. There is a side
            # effect of using the /quiet option, namely that
            # "C:\perl\bin" doesnt get added to the Windows path
            # variable. However, we are directly accessing the
            # ActivePERL executable via full pathname, so it doesnt
            # really matter that the Windows path variable is not
            # updated.
            regsub -all {/} $dest_path {\\\\} dest_path ;# fix path for Windows
            $self rexec -t $install_sec -T $install_sec \
		msiexec.exe /quiet /package $dest_path

            # After install, check we now have the correct version of
            # ActivePERL
            set resp [$self check_perl_version $required_perl_version]
            if {$resp != "OK"} {
                error "setup $name could not install correct ActivePERL\
                    $required_perl_version"
            }
        }

        # Check interactively if user wants BlueTool installed.
        set dest_path [$self cget -bluetool]
        if {$dest_path != "" && [prompt "\n\nRun BlueTool setup ?" $quiet]} {

            # Unfortunately, there is no command line option on
            # BlueTool to check the version of the tool. So we look at
            # executable file modify dates and make a decision to
            # install based on the file modify dates. Testing has
            # shown that the .exe files inside the setup.exe are not
            # perfectly time synched in their file mtime.  So we
            # compare date only, not time.
            set format "%Y%m%d" ;# dont include time!
            set src_exe "BlueTool_setup.exe"
            set src_path "$archive/$src_exe"
            set src_mtime_sec [exec stat --format %Y $src_path]
            set src_date [clock format $src_mtime_sec -format $format]
            UTF::Message LOG $name \
		"BlueTool on server: $src_path date: $src_date"
            if {![catch {$self stat --format %Y $dest_path} dest_mtime_sec]} {
                set dest_date [clock format $dest_mtime_sec -format $format]
            } else {
                set dest_mtime_sec 0
                set dest_date "file not found"
            }
	    set delta_sec [expr {abs($src_mtime_sec - $dest_mtime_sec)}]
            UTF::Message LOG $name "BlueTool on $name: $dest_path date: $dest_date delta_sec: $delta_sec"

            # If BlueTool versions are different, install the correct
            # package Time can be off by an hour for daylight savings
            # time.
            if {$src_date != $dest_date && $delta_sec > 3660} {

                # Uninstall older version of BlueTool.
                $self uninstall_bluetool

                # Install current version of BlueTool
                $self install_bluetool $src_path $install_sec

                # Check dest_date again.
                set dest_mtime_sec [$self stat --format %Y $dest_path]
                set dest_date [clock format $dest_mtime_sec -format $format]
                set delta_sec [expr {abs($src_mtime_sec - $dest_mtime_sec)}]
                UTF::Message LOG $name \
		    "BlueTool on $name after install: $dest_path date: $dest_date delta_sec: $delta_sec"
                if {$src_date != $dest_date && $delta_sec > 3660} {
                    error "setup ERROR: $name BlueTool dates dont match,\
                        src_date=$src_date NE dest_date=$dest_date"
                }
            }

        } else {
            UTF::Message WARN $name \
		"User requested that BlueTool not be installed!"
        }

        # Check interactively if user wants perl scripts installed.
        set dest_dir [$self cget -perl_scripts]
        if {$dest_dir != "" &&
	    [prompt "\n\nRun PERL scripts setup ?" $quiet]} {

            # Now copy over the BlueTool supporting perl scripts. The
            # UTF copyto method will leave the latest scripts inside a
            # subfolder, instead of overwriting the directory
            # structure. This is the standard Unix copy command
            # challenge. So we copy the scripts to a staging directory
            # and then rename it.

            # NB: Any local modifications in the perl_scripts
            # directory get blown away in the process.
            set src_subdir coex_perl_scripts
            set src_path "$archive/$src_subdir"
            $self rexec rm -rf $dest_dir ;# clean out the dest_dir
            $self rexec rm -rf "/$src_subdir" ;# clean the staging directory
            catch {$self copyto $src_path /$src_subdir}
            $self rexec mv "/$src_subdir" $dest_dir

            # Move brcmlib.pl from common to c:\perl\lib. This ensures
            # perl will always find this library.
            set perl_bin [$self cget -active_perl]
            $self mv $dest_dir/common/brcmlib.pl $perl_bin/../lib

            # Move pesq.exe from common to c:\windows\system32. This
            # that the default windows path will always find this
            # tool.
            set system32 "c:/windows/system32"
            $self mv $dest_dir/common/pesq.exe $system32

        } else {
            UTF::Message WARN $name \
		"User requested that perl scripts not be installed!"
        }

        # Ask user if BTWUSB driver should be copies. Driver will be
        # installed via the Windows GUI when user plugs in the BT USB
        # cable for the first time. Yes, the user has to point to the
        # driver folder.
        if {[prompt "\n\nCopy over BTWUSB driver ?" $quiet]} {
            set src_subdir BTWUSB
            set src_path "$archive/$src_subdir"
            set dest_dir "c:/\"Program Files\"/Broadcom/$src_subdir"
            $self rexec rm -rf $dest_dir ;# clean out the dest_dir
            $self rexec rm -rf "/$src_subdir" ;# clean the staging directory
            catch {$self copyto $src_path /$src_subdir}
            $self rexec mv "/$src_subdir" $dest_dir
            UTF::Message LOG $name "When you plug in the BT USB cable for the\
                first time, the Windows GUI will prompt you to point to the $src_subdir\
                directory with the driver."
            return
        } else {
            UTF::Message WARN $name \
		"User requested that BTWUSB driver not be installed!"
        }
    }

    UTF::doc {
	# [call [arg host] [method check_perl_version] [arg required_version]]

        # [arg required_version] is in the format I.J.K.L, which are
        # the 3 version numbers and the build number.  Returns OK if
        # host Windows ActivePERL version matches the desired_version,
        # otherwise returns ERROR.[para]

        # NB: We ignore the Cygwin version of PERL, as BlueTool wants
        # ActivePERL.
    }

    method check_perl_version { required_version } {

        # Check if user wants perl installed or not
        set perl_dir [$self cget -active_perl] ;# ignore Cygwin perl
        if {$perl_dir == "" } {
            UTF::Message WARN $name "check_perl_version\
                User specified that Perl not be installed!"
            return OK
        }

        # Get current perl version from host, if any.
        catch {$self rexec -silent $perl_dir/perl.exe -v} catch_msg
        # puts "catch_resp=$catch_resp catch_msg=$catch_msg"

        # Extract the perl version info, vI.J.K and build number
        if {[regexp -nocase {\s+v(\d+\.\d+\.\d+)\s+.*build\s+(\d+)\s+} \
		 $catch_msg - ver build]} {
            set actual_version "${ver}.${build}"
        } else {
            set actual_version ""
        }

        # Compare versions and return.
        UTF::Message LOG $name "check_perl_version\
            actual_version=$actual_version required_version=$required_version"
        if {$actual_version == $required_version } {
            return OK
        } else {
            return ERROR
        }
    }

    UTF::doc {
	# [call [arg host] [method restart_bluetool]]

        # Terminates any existing BlueTool processes. Starts a new
        # instance of BlueTool. Returns the new process id.
    }

    method restart_bluetool { } {

        # Get BlueTool executable from -bluetool option
        set bt_path [$self cget -bluetool]
        set bt_exe [file tail $bt_path]
        # set bt_path bbbb ;# test code
        # set bt_exe aaaa ;# test code

        # Check BlueTool is installed.
        if {$bt_path == ""} {
            error "$name restart_bluetool ERROR: BlueTool is not installed!"
        }

        # Try multiple times to restart BlueTool
        for {set i 1} {$i <= $::max_tries} {incr i} {

            # We can only have one instance of BlueTool running at any
            # given time. So we terminate any existing instance of
            # BlueTool before we launch a new instance.
            $self stop_bluetool

            # The objective is to start BlueTool and leave it running,
            # possibly for hours at a time, while other parts of the
            # script are run. So the startup method must return
            # program control even though BlueTool is still running.

            # The rexec method does not work for starting BlueTool.
            # The issue is that BlueTool starts up, but rexec does not
            # return until BlueTool ends. This prevents other portions
            # of the script from running. Then rexec times out and
            # kills the BlueTool process.

            # The method rpopen runs the command as a pipeline chain
            # and gives a file descriptor, which is long lived. This
            # allows the TCL code to return even though the BlueTool
            # utility is still running on the host PC.

            # Testing has shown that you dont get any errors from
            # rpopen even if you have an invalid bt_path!
            UTF::Message LOG $name "Starting BlueTool try: #$i"
            if {[catch {$self rpopen $bt_path} catch_msg]} {
                UTF::Message ERROR $name "restart_bluetool: $catch_msg"
            }

            # Locate the pid for the new BlueTool process.
            UTF::Sleep 2 $name;# sometimes BlueTool is slow to start up.
	    if {![catch {
		UTF::get_new_pids_by_name $self bluetool.exe "" 1
	    } new_pid]} {
                return $new_pid
            } else {
                UTF::Message ERROR $name "restart_bluetool: $new_pid"
            }

            # Wait a bit before next attempt.
            if {$i < $::max_tries} {
                UTF::Sleep 5 $name
            }
         }

        # Multiple attempst failed.
        error "$name restart_bluetool ERROR: failed after $::max_tries tries"
    }

    UTF::doc {
	# [call [arg host] [method stop_bluetool]]

        # Terminates any existing BlueTool processes.
    }

    method stop_bluetool { } {

        # Get BlueTool executable from -bluetool option
        set bt_exe [file tail [$self cget -bluetool]]

        # Check BlueTool is installed.
        set name [$self cget -name]
        if {$bt_exe == ""} {
            error "$name stop_bluetool ERROR: BlueTool is not installed!"
        }

        # Terminate any existing instances of BlueTool
        UTF::terminate_pid_by_name $self $bt_exe

        # Terminate any remaining perl processes
        UTF::terminate_pid_by_name $self perl
        return
    }

    UTF::doc {
	# [call [arg host] [method install_bluetool] [arg path] [arg
        # timeout]]

        # Installs the BlueTool utility. This method assumes that the
        # previous version of BlueTool has already been uninstalled,
        # see method uninstall_bluetool.  [arg path] pathname of
        # BlueTool setup.exe to use.  [arg timeout] is the timeout, in
        # seconds, for the installer to complete the job.
    }

    method install_bluetool {path timeout} {

        # Check BlueTool is supposed to be installed.
        set bt_path [$self cget -bluetool]
        if {$bt_path == ""} {
            error "$name install_bluetool ERROR: User requested that\
                BlueTool not be installed!"
        }

        # This method assumes that the previous version of BlueTool
        # has already been uninstalled, see method uninstall_bluetool.
        UTF::Message LOG $::localhost "install BlueTool"

        # Copy BlueTool setup.exe from archive server to host PC. The
        # BThelp.exe expects a very specific series of subdirectories
        # to be present, and it is looking for setup.exe explicitly,
        # so we rename the file in the process.
        set tmp_path "c:/tmp/BLUETOOL/BLUETOOL_MI_1.1.1.1"
        $self rexec mkdir -p $tmp_path
        if {[catch {$self copyto $path $tmp_path/setup.exe} catch_msg]} {
            error "$name install_bluetool ERROR: copyto $path failed: $catch_msg"
        }

        # Copy BThelp.exe from from archive server to host PC.
        set bt_help BThelp.exe
        # Create src_path as 2 steps to avoid an unmatched double
        # quote showing up at the start of the path. TCL bug?
        set src_path [file dirname $path]
        append src_path "/$bt_help"
        if {[catch {$self copyto $src_path /tmp/$bt_help} catch_msg]} {
            error "$name install_bluetool ERROR: copyto $src_path failed: $catch_msg"
        }

        # Run the BThelp.exe which runs BlueTool setup.exe with GUI
        # prompts turned off.
        $self rexec -T $timeout -t $timeout /tmp/$bt_help -i\\\\tmp\\\\BLUETOOL -dc:\\\\tmp\\\\btinstall.log
        return
    }

    UTF::doc {
	# [call [arg host] [method uninstall_bluetool]]

        # Uninstalls the existing BlueTool files & registry entries,
        # if any.
    }

    method uninstall_bluetool { } {

        # Terminate any existing instances of BlueTool
        UTF::Message LOG $::localhost "uninstall BlueTool"
        catch {$self stop_bluetool}

        # Delete bluetool directory, subdirectories & all files.
        set bt_dir [file dirname "[$self cget -bluetool]"]
        # puts "bt_dir=$bt_dir"
        if {$bt_dir != "" && $bt_dir != "."} {
            $self rexec rm -rf $bt_dir
        }

        # Open a TCL file on the local disk.
        set bt_reg /tmp/bt_registry_cleanup.tcl
        if {[catch {set out [open $bt_reg w]} catch_msg]} {
            error "Could not open $bt_reg catch_msg=$catch_msg"
        }
        # puts "bt_reg=$bt_reg out=$out"

        # Add TCL code to the file.
        puts $out "# This TCL program was automatically generated by UTF WinBT.tcl method uninstall_bluetool\n"
        puts $out "# Used to delete the Windows registry entries installed by BlueTool\n"
        puts $out "package require registry\n"
        puts $out "# Define registry keys to be deleted"
        puts $out "lappend key_list HKEY_CLASSES_ROOT\\\\.btp"
        puts $out "lappend key_list HKEY_CLASSES_ROOT\\\\BlueTool.exe"
        puts $out "lappend key_list HKEY_CLASSES_ROOT\\\\Installer\\\\Products\\\\E37BFF8238C831042AA378B14D02C320"
        puts $out "lappend key_list HKEY_CURRENT_USER\\\\Software\\\\\Broadcom\\\\BlueTool"
        puts $out "lappend key_list HKEY_LOCAL_MACHINE\\\\SOFTWARE\\\\\Broadcom\\\\BlueTool"
        puts $out "\n# Delete the keys"
        puts $out "foreach key \$key_list \{"
        puts $out "    puts \"deleting key: \$key\""
        puts $out "    registry delete \$key"
        puts $out "\}\n"

        # Close the file
        catch "close $out"

        # Copy TCL file to target machine
        set catch_resp [catch {$self copyto $bt_reg $bt_reg} catch_msg]
        # puts "copy $bt_reg catch_msg=$catch_msg"
        if {$catch_resp != 0} {
            error "ERROR: $self copy $bt_reg failed: $catch_msg"
        }

        # Now run the TCL file on the target PC.
        $self rexec tclsh $bt_reg
        return
    }

   UTF::doc {
	# [call [arg host] [method run_perl_script] [arg script] [arg
        # args] [lb][arg -rpopen][rb]]

        # This method runs perl scripts on the target PC.  [arg
        # script] relative path of the perl script to be run on the
        # target PC. The path is relative to the directory where the
        # perl scripts are stored on the target PC, usually
        # c:\\perl_scripts [arg args] any additional arguments needed
        # by the perl script.[para]

        # If the option [arg -rpopen] is specified, the perl script
        # will be launched as a parallel task on the target PC. This
        # is useful when you need to run multiple items in parallel,
        # possibly on different target PCs.[para]

        # Returns the stdout string generated by the perl script. For
        # the case of option [arg -rpopen], returns the file
        # descriptor for the locally created process.[para]

        # NB: The perl script will be started with the working
        # directory set to the path of the script being run. This
        # ensures that perl will be able to locate any perl modules
        # that the script normally loads from the same directory as
        # the script.[para]

        # NB: Any arguments for the perl script will automatically
        # have Unix path forward slashes converted to Windows back
        # slashes.
    }

    method run_perl_script {script args} {

        # Put marker line in log file
        UTF::Message LOG $name "================"

        # Get path info for target PC.
        set perl_path [$self cget -active_perl]
        if {$perl_path != ""} {
            set perl "$perl_path/perl.exe"
        } else {
            set perl perl;# use Cygwin perl instead
        }
        set script_path [$self cget -perl_scripts]
        if {$script_path != ""} {
            set script "$script_path/$script"
        }

        # Flatten args list explicitly.  This is needed because the
        # regsub and rexec calls below assume $args is a string, not a
        # list and TCL's implicit flattening may try to protect
        # special characters like quotes and trailing spaces,
        # preventing us passing them cleanly through the shell.
        set args [join $args " "]

        # Convert Unix path with forward slash to Windows back slash.
        regsub -all {/} $args {\\\\} args
        # puts "args=$args"

        # brcmlib.pl is now stored in c:\perl\lib where perl will
        # always find it. To keep things simple, the script is to
        # started in the working directory where the script itself is
        # located.
        set working_dir [file dirname $script]

        # Run the script in the working directory.
        # NB: When daisy-chaining commands to rexec, the whole
        # sequence must be enclosed in double quotes. Otherwise TCL
        # will send the first command to the target PC and execute the
        # remainder on the local machine.
        # NB: ActivePerl doesnt seem to mind pathnames in Unix format
        # with forward slashes instead of the usual Windows
        # backslashes.

        # Look for -rpopen option in case user wants script launched
        # as a parallel task.
        if {![string match -nocase "*-rpopen*" $args]} {
            set resp [$self rexec "cd $working_dir ; $perl $script $args"]
            return $resp
        }

        # Use method rpopen to launch script.
        regsub -all -nocase {\-rpopen} $args "" args ;# remove -rpopen
        set fd [$self rpopen "cd $working_dir ; $perl $script $args"]
        puts "fd=$fd"
        UTF::Message LOG $name "run_perl_script created fd=$fd"
        # Setup file descriptor for use by UTF::collect_rpopen_data.
        # Setup file descriptor for use by UTF::collect_rpopen_data.
        # We deliberately use the same ::utils_reading variable for
        # all processes running in parallel at any given time. This is
        # done so that proc UTF::collect_rpopen_data can monitor
        # multiple parallel processes at any given time.
        fconfigure $fd -blocking 0
        fileevent $fd readable {set ::utils_reading READY}
        return $fd
    }

    UTF::doc {
	# [call [arg host] [method set_board_address] [lb][arg
	# address][rb]]

        # Goes to the BT device to set the board address. Verifies the
        # board address is correctly set. By default, the hardware
        # address specified in option -bt_hw_addr will be used. If
        # -bt_hw_addr is not specified, then the MAC address of the
        # host PC will be queried and used to ensure global uniqueness
        # for the BT board address. This prevents testrigs from
        # interfering with each other when the chamber doors are
        # open.[para]

        # [arg address] String of hex digits. If specified, the
        # optional address will be used instead of the -bt_hw_addr or
        # host PC MAC address.
    }

    method set_board_address {{address ""}} {

        # Choose the board address to use.
        if {$address == ""} {
            # User may have specified option -bt_hw_addr
            set address [$self cget -bt_hw_addr]
            if {$address == ""} {
                # Default is to use MAC address of host PC, as the
                # Ethernet card has a globally unique address courtesy
                # of the IEEE.
                set address [UTF::get_host_mac_addr $self]
                UTF::Message LOG $name \
		    "set_board_address using host MAC address=$address"

            } else {
                UTF::Message LOG $name \
		    "set_board_address using -bt_hw_addr address=$address"
            }

        } else {
            UTF::Message LOG $name \
		"set_board_address using cmd_line address=$address"
        }

        # Set the BT board address.
        # $self configure -bt_comm "com3@3000000" ;#test code
        set bt_comm [$self cget -bt_comm]
        regsub -all {[:\-]} $address "" address ;# remove colons, hyphens
        $self run_perl_script common/WriteBDAddr.pl $bt_comm $address

        # Verify the BT board address. This also stores the address in
        # the object level variable board_address.
        $self get_board_address $address
        return $address
    }

    UTF::doc {
	# [call [arg host] [method get_board_address] [lb][arg
	# expected_value][rb]]

        # Always goes to the BT device to get the current board
        # address.  Stores the board address in an object level
        # variable. If you want the currently cached board address,
        # then use method show_board_address.[para]

        # [arg expected_value] If specified, the optional
        # expected_value will be checked against the actual board
        # address. If they are not the same, an error will be thrown.

    }

    method get_board_address {{expected_value ""}} {

        # Run the ReadBDAddr.pl script, save address in the object
        # level variable board_address
        # $self configure -bt_comm "com3@3000000" ;#test code
        set bt_comm [$self cget -bt_comm]
        set resp [$self run_perl_script common/ReadBDAddr.pl $bt_comm]
        # puts "resp=$resp"
        if {![regexp -nocase {BD_ADDR:\s*([a-f,A-F,0-9]+)} $resp \
		  - board_address]} {
            error "$name get_board_address ERROR: could not parse board\
               address, resp=$resp"
        }

        # Log the calling data & result.
        UTF::Message LOG $name \
	    "get_board_address expected_value=$expected_value actual_board_address=$board_address"

        # If requested, check epected_value against actual value.
        if {$expected_value != "" &&
	    [string tolower $expected_value] != [string tolower $board_address]} {
            error "$name get_board_address ERROR: expected_value=$expected_value\
                NOT EQUAL actual_board_address=$board_address"
        }
        return $board_address
    }

    UTF::doc {
	# [call [arg host] [method show_board_address]]

        # This method returns the cached BT board address that is
        # saved in an object level variable. If the cached value is
        # not available, the board address will be retrieved from the
        # BT device.
    }

    method show_board_address {} {

        # If we dont have a cached value for board_address, then go
        # get it.
        if {$board_address == "" || $board_address == 0} {
            $self get_board_address
        }

        # Return the board_address
        return $board_address
    }

    UTF::doc {
	# [call [arg host] [method get_bt_version]]

        # This method returns the BT hardware & firmware version
        # info.[para]
    }

    method get_bt_version {} {

        # Run the Read_Local_Version.pl script for version info.
        set bt_comm [$self cget -bt_comm]
        set resp [$self run_perl_script common/Read_Local_Version.pl $bt_comm]
        # puts "resp=$resp"

        # Parse out and decode HCI_Revision & LMP_Subversion info.
        if {[regexp -nocase {HCI_Revision = (\d+).*LMP_Subversion = (\d+)\n} \
		 $resp - hci lmp]} {
	    # Decode subfields per email from B.Tietz 2009-02-19 set
	    # hci 32009 ;# test code
	    set hci_hw     [expr {$hci & 0xf000}] ;# bits 12-15
	    set hci_hw     [expr {$hci_hw >> 12}]
	    set hci_config [expr {$hci & 0x0fff}] ;# bits 0-11
	    set fw_major   [expr {$lmp & 0xf000}] ;# bits 12-15
	    set fw_major   [expr {$fw_major >> 12}]
	    set fw_minor   [expr {$lmp & 0x0f00}] ;# bits 8-11
	    set fw_minor   [expr {$fw_minor >> 8}]
	    set fw_build   [expr {$lmp & 0x00ff}] ;# bits 0-7
	    set bt_ver "HCI $hci_hw.$hci_config LMP $fw_major.$fw_minor.$fw_build"
        } else {
           # error "$name get_bt_version ERROR: Could not parse version info, resp=$resp"
        }
        UTF::Message LOG $name "get_bt_version bt_ver=$bt_ver"
        return $bt_ver
    }

    UTF::doc {
	# [call [arg host] [method is_ref_board] [lb][arg path][rb]]

        # This method returns 1 if the BT board appears to be a 2046
        # series reference board, otherwise returns 0.  [arg path]
        # Optional path of image file used to load the board, if
        # available.
    }

    method is_ref_board {{path ""}} {

        # Check image path name for clues about the board.  Add Ref
        # board type BCM2070 used for BLE tests
        # maybe superfluous: check turned off
#         if {$path != ""} {
#             if {[string match -nocase *BCM2046* $path] ||
# 		[string match -nocase *BCM2070* $path]} {
#                 return 1
#             }
#         } ;# end code change turning off path check

        # Check the object type for clues about the board.
        set type [$self cget -type]
	UTF::Message DEBUG "" "is_ref_board type given: ${type}\n"
        if {[string match -nocase *BCM2046* $type] ||
	    [string match -nocase *BCM2070* $type]} {
	    return 1
        }

        # Its not a reference board
        return 0
    }

    UTF::doc {
	# [call [arg host] [method perl_subdir]]

        # This method returns the name of the appropriate perl script
        # subdirectory to use for this object.
    }

    method perl_subdir {} {
        # Currently, perl subdirectory choice simply depends on board
        # type.  When BTE Insight is supported, need more code here.
        if {[$self is_ref_board]} {
            return bt_ref
        } else {
            return bt_cohost
        }
    }

    UTF::doc {
	# [call [arg host] [method show_acl_handles]]

        # This method returns the list of current active ACL
        # connection handles.
    }

    method show_acl_handles {} {
        UTF::Message LOG $name "show_acl_handles: $acl_handles"
        return $acl_handles
    }

    UTF::doc {
	# [call [arg host] [method show_sco_handles]]

        # This method returns the list of current active SCO/ESCO
        # connection handles.
    }

    method show_sco_handles {} {
        UTF::Message LOG $name "show_sco_handles: $sco_handles"
        return $sco_handles
    }

    UTF::doc {
	# [call [arg host] [method save_acl_handles] [arg handle1]
        # ... [lb][arg handleN][rb]]

        # This method stores one or more new active ACL connection
        # handles.  A handle is a positive integer. Duplicate handles
        # are discarded.[para]

        # This method returns the updated list of current active ACL
        # connection handles.
    }

    method save_acl_handles {args} {

        # Check handles are positive integers, append to object level
        # variable
        foreach handle $args {
            if {[regexp {^\d+$} $handle] && $handle > 0} {
                lappend acl_handles $handle
            } else {
                error "save_acl_handles ERROR: invalid handle=$handle,\
                    must be positive integer!"
            }
        }

        # Remove duplicate items, log & return updated list.
        set acl_handles [lsort -integer -unique $acl_handles]
        UTF::Message LOG $name "save_acl_handles added: $args\
            acl_handles: $acl_handles"
        return $acl_handles
    }

    UTF::doc {
	# [call [arg host] [method save_sco_handles] [arg handle1]
        # ... [lb][arg handleN][rb]]

        # This method stores one or more new active SCO/ESCO
        # connection handles.  A handle is a positive
        # integer. Duplicate handles are discarded.[para]

        # This method returns the updated list of current active
        # SCO/ESCO connection handles.
    }

    method save_sco_handles {args} {

        # Check handles are positive integers, append to object level
        # variable
        foreach handle $args {
            if {[regexp {^\d+$} $handle] && $handle > 0} {
                lappend sco_handles $handle
            } else {
                error "save_sco_handles ERROR: invalid handle=$handle,\
                    must be positive integer!"
            }
        }

        # Remove duplicate items, log & return updated list.
        set sco_handles [lsort -integer -unique $sco_handles]
        UTF::Message LOG $name \
	    "save_sco_handles added: $args sco_handles: $sco_handles"
        return $sco_handles
    }

    UTF::doc {
	# [call [arg host] [method delete_acl_handles] [arg handle1] 
        # ... [lb][arg handleN][rb]]

        # This method deletes one or more active ACL connection
        # handles from the current list. A handle is a positive
        # integer. If handle is "all", then all ACL handles are
        # deleted.[para]

        # This method returns the updated list of current active ACL
        # connection handles.
    }

    method delete_acl_handles {args} {

        # Delete handles from object level variable
        set args [string tolower $args]
        if {$args == "all"} {
            set acl_handles ""

        } else {
            foreach handle $args {
                set i [lsearch -exact $acl_handles $handle]
                if {$i >= 0} {
                    set acl_handles [lreplace $acl_handles $i $i]
                } else {
                    UTF::Message WARN $name "delete_acl_handles\
                        handle=$handle was not in current active ACL list!"
                }
            }
        }

        # Log & return updated list.
        UTF::Message LOG $name \
	    "delete_acl_handles deleted: $args acl_handles: $acl_handles"
        return $acl_handles
    }

    UTF::doc {
	# [call [arg host] [method delete_sco_handles] [arg handle1]
        # ... [lb][arg handleN][rb]]

        # This method deletes one or more active SCO/ESCO connection
        # handles from the current list. A handle is a positive
        # integer. If handle is "all", then all SCO/ESCO handles are
        # deleted.[para]

        # This method returns the updated list of current active
        # SCO/ESCO connection handles.
    }

    method delete_sco_handles {args} {

        # Delete handles from object level variable
        set args [string tolower $args]
        if {$args == "all"} {
            set sco_handles ""

        } else {
            foreach handle $args {
                set i [lsearch -exact $sco_handles $handle]
                if {$i >= 0} {
                    set sco_handles [lreplace $sco_handles $i $i]
                } else {
                    UTF::Message WARN $name \
			"delete_sco_handles handle=$handle was not in current active SCO/ESCO list!"
                }
            }
        }

        # Log & return updated list.
        UTF::Message LOG $name \
	    "delete_sco_handles deleted: $args sco_handles: $sco_handles"
        return $sco_handles
    }

    UTF::doc {
	# [call [arg host] [method ayt] [arg remote_BT] [lb][arg
        # -scan][rb] [lb][arg scantype][rb]]

        # The are-you-there (ayt) method retrieves the name of the
        # specified remote BT object. This is a non-disruptive
        # operation that determines if the remote device is still
        # responding over the airwaves or not. The ayt can be done at
        # any time for a BT device that is using the lower stack
        # only. This is similar to the functionality provided by the
        # upper stack ICMP ping.[para]

        # If scans have not previously been enabled by other scripts,
        # then the remote name returned is quite likely to be
        # null. The remote board address will be correctly returned
        # and validated. To work around the null remote name issue,
        # the option [arg -scan] is provided to enable the scans on
        # both BT devices before the ayt operation is performed. The
        # option [arg scantype] defaults to 3.[para]

        # Returns the remote name text string.
    }

    method ayt {remote_BT {scan ""} {scantype "3"} } {

        # Get the remote board address, etc
        set remote_address [$remote_BT show_board_address]
        set local_bt_comm [$self cget -bt_comm]

        # Optionally initialise scans.
        set scan [string tolower $scan]
        if {$scan eq "-scan"} {
            # Start scans for self
            $self run_perl_script \
		common/InitScan.pl $local_bt_comm $scantype

            # Start scans for remote device
            set remote_bt_comm [$remote_BT cget -bt_comm]
            $remote_BT run_perl_script \
		common/InitScan.pl $remote_bt_comm $scantype

        }

        # Get the remote device name.
        set resp [$self run_perl_script \
		      common/ReadRemoteName.pl $local_bt_comm $remote_address]
        regexp -nocase {remote_name=([^\n]*)\n} $resp - remote_name

        # Return the remote name.
        UTF::Message LOG $name "ayt remote_BT=$remote_BT\
            remote_address=$remote_address remote_name=$remote_name"
        return $remote_name
    }

    UTF::doc {
	# [call [arg host] [method change_bt_packet_type] [arg handle]
        # [arg packet_type]]

        # This method changes the packet type of an existing BT
        # connection.[para]

        # Returns nothing, throws error on failure.
    }

    method change_bt_packet_type {handle packet_type} {

        # Get the expanded list of packet types to ensure that only
        # the desired packet type is used.
        set packet_type [map_bt_packet_type $packet_type]

        # Run the common script
        set bt_comm [$self cget -bt_comm]
        $self run_perl_script \
	    common/ChangeConnectionPacketType.pl $bt_comm $handle $packet_type
        return
    }

    UTF::doc {
	# [call [arg host] [method whatami]]

        # This method dyanamically detects the host OS version, in
        # addition to the STA chipname.
    }

    method whatami {{STA ""}} {

        # Get host OS and map to text string.
        $self checkOs
        # set os_ver 2 ;# test code
        if {$os_ver == 6} {
            set iam "Vista"
        } elseif {$os_ver == 5} {
	    set iam "WinXP"
        } else {
            set iam "<unknown>"
        }

        # There is no way to query a BlueTooth chip revision.

        # Log & return result .
	return $iam
    }

}

# Retrieve manpage from last object
UTF::doc [UTF::WinBT man]

UTF::doc {
    # [list_end]

    # [section EXAMPLES]
    # [example_begin]
    package require UTF
    package require UTF::WinBT
    UTF::WinBT Laptop -lan_ip 10.19.12.198 -sta {STA2} -bt_comm com3@3000000

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
