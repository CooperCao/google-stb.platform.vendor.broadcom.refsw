#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id: 2163dbe172c1c14d342e0ebbc39d7d7cf44887c7 $
# $Copyright Broadcom Corporation$
#

package provide UTF::Sniffer 2.0

package require snit
package require UTF::doc
package require UTF::Base

package require UTF::Linux
package require UTF::utils
package require UTF


UTF::doc {
    # [manpage_begin UTF::Sniffer n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Sniffer}]
    # [copyright {2008 Broadcom Corporation}]
    # [require UTF::Sniffer]
    # [description]
    # [para]

    # UTF::Sniffer is an object that provides UTF test scripts the
    # ability to control various aspects of a Sniffer with one or more
    # wireless interfaces.[para]

    # UTF::Sniffer is based on the UTF::Linux object, and in fact
    # delegates most methods and options to an internal instance of
    # that object.[para]

    # Normally another script will create a new object and then call
    # its methods to control/perform operations on the Sniffer.[para]

    # Sniffer is currently called by Test/rvr.test and other tests
    # that uses the Sniffer Object.[para]

    # [list_begin definitions]
}

snit::type UTF::Sniffer {

    UTF::doc {
	# [call [cmd UTF::Sniffer] [arg name]
	#	[option -name] [arg name]
	#	[lb][option -ringbuffersize] [arg MBytes]
	#	[lb][option -numfilesrb] [arg integer]
	#	[lb][option -tsharkImage] [arg path]
	#	[lb][option -wlinitcmds] [arg cmds][rb]
	#	[lb][option -project_dir] [arg project_dir][rb]
	#	[arg ...]]

	# Create a new Sniffer host object. The host will be used used
	# to contact STAs residing on that host.

	# [list_begin options]

	# Note: relative paths will be relative to the current working
	# directory, which is the same as the directory the script was
	# launched in, not the SummaryDir.

	# [opt_def [option -name] [arg name]]

	# Name of the host.

	# [opt_def [option -ringbuffersize] [arg MBytes]]

	# Size (in MBytes) of the file in each individual file in the
	# circular ring Buffer. Default is 50Mb.

	# [opt_def [option -numfilesrb] [arg integer]]

	# Number of files in the circular ring buffer. Default is 6.

	# [opt_def [option -tsharkImage] [arg path]]

	# Directory containing the tshark image to use

	# [opt_def [option -wlinitcmds] [arg cmds]]

	# Specify cmds list to be executed after driver is loaded /
	# reloaded.  Default is null.

	# [opt_def [arg ...]]

	# Unrecognised options will be passed on to the host's [cmd
	# UTF::Linux] object.

        # [list_end]
        # [list_end]

        # [para]
        # Sniffer objects have the following methods:
        # [list_begin definitions]
    }

    pragma -canreplace yes

    # base handles any other options and methods
    # same as:
    # 	delegate option * to base
    # 	delegate method * to base

    component base -inherit yes

    option -sta
    option -name -configuremethod CopyOption
    option -ringbuffersize 50 ;# MBytes
    option -numfilesrb 6 ;# files
    option -noinstalltshark
    option -device

    variable 11ac_sniffer_flag 0

    constructor {args} {
	# creates a Linux object and installs it as component base
	install base using \
	    UTF::Linux %AUTO% 
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

    method init {} {
	$self open_messages
    }

    method deinit {} {
	$self close_messages
	$self configure -initialized 0
    }

    UTF::doc {
	# [call [cmd name] [method setupSniffer]  [arg chanspecList]
	#	  [arg channelsList]]

	# This function does the following:
	# For each wireless interface in Sniffer host
	# [list_begin enum]
	# [enum] Brings up wireless interface in monitor mode
	# [enum] configure to sniff on the desired chanspec
	# [enum] Bring up wireless interface
	# [enum] Bring up virtual prism interface
	# [list_end]
    }

    method setupSniffer {chanspecList} {
        UTF::Message INFO "" "************************LibFunc: $options(-name) setupSniffer $chanspecList***************************"

        # chanspecList contain a list of chanspec; one for each STA interface
        # listed  in the config file
        # for each STA interface in the config file ($device):
        #	(a) wl -i $device monitor 1
        #	(b) wl down
        #	(c) wl -i $device mimo_bw_cap 1
        #	(d) wl -i $device chanspecList[i]
        #	(e) wl up
        #	(f) ./ifconfig $device up
        #	(g) ./ifconfig prism0 up
        set i 0
        foreach {sta device} $options(-sta) {
        
            if {$device eq ""} {
                error "$sta has no device name"
            }

            set channel [lindex $chanspecList $i]
            UTF::Message LOG $options(-name) "setupSniffer sta=$sta device=$device channel=$channel i=$i"

            set linuxver [$self rexec uname -r]

            $self wl down
            UTF::Sleep 1

            if {[string match -nocase "*FC19*" $linuxver]} {
                set catch_resp [catch "$self ifconfig radiotap$i" catch_msg]
                if {$catch_resp == 0} {
                    $self ifconfig radiotap$i down
                }
            } else {
                catch {$self ifconfig prism$i down}
            }
            
            UTF::Sleep 1
            set catch_resp [catch "$self ifconfig $device" catch_msg]
            if {$catch_resp == 0} {
                $self ifconfig $device down
            }
            UTF::Sleep 1

            # NB: Can't use the 2G BW=40 channels without mimo_bw_cap=1
            set catch_resp [catch "$self wl -i $device mimo_bw_cap 1" catch_msg]
            if {$catch_resp != 0} {
                UTF::Message WARN $options(-name) "catch_msg=$catch_msg"
            }

            if {[string match -nocase "*FC19*" $linuxver]} {
                set 11ac_sniffer_flag 1
                UTF::Message LOG $options(-name) "FC19 11ac sniffer"
                $self wl -i $device mpc 0
                $self wl -i $device PM 0
                $self wl -i $device amsdu 0
                $self wl -i $device ampdu 0
                $self wl -i $device chanspec $channel
                $self wl up
                UTF::Sleep 2
                $self wl -i $device monitor 3
                $self ifconfig $device up
                UTF::Sleep 2
                $self ifconfig radiotap$i up
                UTF::Sleep 1
                $self ifconfig radiotap$i
                UTF::Sleep 20
            } else {
                set 11ac_sniffer_flag 0
                $self wl -i $device chanspec $channel
                $self wl up
                $self ifconfig $device up
                $self wl -i $device monitor 1
                UTF::Sleep 2
                $self ifconfig prism$i up
            }
            
            incr i
        }
    }

    UTF::doc {
	# [call [cmd name] [method setup]]

	# This function does the following:
	# [list_begin enum]
	# [enum] Calls the Linux setup method with passed in arg list
	# [enum] Finds the current linux distribution
	# [enum] Installs the appropirate wireshark rpm based on the distro
	# [enum] Installs the appropriate libsmi rpm based on the distro
	# [list_end]
    }

    # Copies over a stable tshark and supported binary
    method setup {args} {
	# Run the Linux setup method first.
	eval $base setup $args

	# The remaining items are specific to Sniffer
	puts "Performing Specific Sniffer setup!!\n"
	set linuxver [$self rexec uname -r]
	set archive "$::UTF::projarchives/unix/UTF"

        # General purpose code to install OS level dependant rpms.
        # More rpms can be added to rpm_list as needed by each OS level.
	# (1) Add code to quilify on the -noTsharkInstall option. If
	# turned on then don't install tshark from archive
	# (2) Check if there are any other packages that needed to be
	# installed as a prerequisite to installing the Wireshark rpm.

        set rpm_list ""
        if {[string match -nocase "*FC4*" $linuxver]} {
            error "setup $self ERROR: need rpms for linuxver=$linuxver"
        } elseif {[string match -nocase "*FC6*" $linuxver]} {
            lappend rpm_list libsmi-0.4.5-2.fc6.i386.rpm
            lappend rpm_list wireshark-0.99.5-1.fc6.i386.rpm
        } elseif {[string match -nocase "*FC7*" $linuxver]} {
			lappend rpm_list libsmi-0.4.5-2.fc7.i386.rpm
            lappend rpm_list wireshark-1.0.0-1.fc7.i386.rpm
            # error "setup $self ERROR: need rpms for linuxver=$linuxver"
        } elseif {[string match -nocase "*FC8*" $linuxver]} {
            lappend rpm_list libsmi-0.4.5-3.fc8.i386.rpm
            lappend rpm_list wireshark-1.0.3-1.fc8.i386.rpm
        } elseif {[string match -nocase "*FC9*" $linuxver]} {
            lappend rpm_list libsmi-0.4.5-5.fc9.i386.rpm
            lappend rpm_list wireshark-1.0.8-1.fc9.i386.rpm
            lappend rpm_list wireshark-gnome-1.0.8-1.fc9.i386.rpm
        } elseif {[string match -nocase "*FC11*" $linuxver]} {
            lappend rpm_list libsmi-0.4.8-2.fc11.i586.rpm
            lappend rpm_list wireshark-1.2.2-1.fc11.i586.rpm
            lappend rpm_list wireshark-gnome-1.2.2-1.fc11.i586.rpm
        } elseif {[string match -nocase "*FC15*" $linuxver]} {
            lappend rpm_list wireshark-1.8.1.BRCM.013.i686_20130709.rpm
        } elseif {[string match -nocase "*FC19*" $linuxver]} {
            lappend rpm_list c-ares-1.10.0-1.fc19.x86_64.rpm
            lappend rpm_list qt-settings-19-23.fc19.noarch.rpm
            lappend rpm_list qt-4.8.4-19.fc19.x86_64.rpm
            lappend rpm_list libmng-1.0.10-11.fc19.x86_64.rpm
            lappend rpm_list qt-x11-4.8.4-19.fc19.x86_64.rpm
            lappend rpm_list wireshark-1.99.2_brcm_546138-1.x86_64.rpm
            lappend rpm_list wireshark-gnome-1.99.2_brcm_546138-1.x86_64.rpm
            lappend rpm_list wireshark-qt-1.99.2_brcm_546138-1.x86_64.rpm
        } else {
            error "setup $self ERROR: need rpms for linuxver=$linuxver"
        }

        # Check if each rpm package is already installed. If not, install.
        # Also check that installed rpm is the same version as stored
        # in the archive. If not, upgrade the rpm.

        foreach rpm_file $rpm_list {

            # Get archive directory rpm package version.
            # NB: We may get a package signature warning before or
            # after the package name, depending on the flushing order
            # of stderr and stdout.

            set archive_name ""
            catch {localhost rpmq -p "$archive/$rpm_file"} archive_name
		    regsub -line {^warning: .*} $archive_name {} archive_name
		    set archive_name [string trim $archive_name]

            # Check if host has this package.

            regsub -all {.rpm} $rpm_file "" rpm_name
            set catch_msg ""
            set catch_resp [catch {$self rpmq $rpm_name} catch_msg]
            set catch_msg [string trim $catch_msg]

             puts "rpm_file=$rpm_file rpm_name=$rpm_name catch_resp=$catch_resp\n\
             catch_msg=$catch_msg archive_name=$archive_name"

            # Install / upgrade if necessary.
            if {$archive_name != $catch_msg} {
                $self copyto "$archive/$rpm_file" "/tmp/$rpm_file"
                $self rexec rpm -Uvh --replacepkgs --replacefiles "/tmp/$rpm_file"
            }
        }
    }

    UTF::doc {
	# [call [cmd name] [method start]]

	# This function does the following:
	# [list_begin enum]
	# [enum] invokes the passed-in tshark commandline
	# [enum] invoke "ps -ef tshark"
	# [enum] parse result using regular expression to get the process id
	# [list_end]
    }

    method start {tsharkCmdline} {
	# invoke thsark commandline
	# use regular expression to get the pid
	# return the pid of the tshark instance

	UTF::Message INFO "" "************************LibFunc: $options(-name) start '$tsharkCmdline'***************************"

	# this is a strange way to start sniffer by letting the user pass the sniffer interface command from higher level script.
	# higher level test should not know if the sniffer is 11ac or not and this object should figure it out.
	# to keep the code backward compatable with non 11ac aniffer, for 11ac "prism" is replaced with "radiotap"
    if {$11ac_sniffer_flag == 1} {
		regsub {\-i prism} $tsharkCmdline "\-i radiotap" tsharkCmdline
		UTF::Message LOG $options(-name) "new tsharkCmdline='$tsharkCmdline'"
	}

	set fd [eval $self rpopen $tsharkCmdline]
	# Set up nonblocking reader event
	fconfigure $fd -blocking 0
	fileevent $fd readable {set reading READY}

	UTF::Sleep 1

	set psCmdLine "ps -ef | grep tshark"
	set catch_resp [catch "set processId \[$self rexec $psCmdLine\]" catch_msg]
        if {$catch_resp == 0} {
	    puts "processId=$processId"
        } else {
	    error "ERROR: Problems starting tshark: $catch_msg"
        }

	set matchresult [regexp {([0-9]+)} $processId match PID]
	puts "matchresult=$matchresult"
	if {$matchresult != 1} {
	    error "ERROR: Problems starting tshark regexp: no PID found"
	} else {
	    puts "PID=$PID"
	}
	return $PID
    }

    method startRoamCmdline {tsharkCmdFN} {
	# invoke thsark commandline in the file tsharkCmdFN
	# use regular expression to get the pid
	# return the pid of the tshark instance

	if {$tsharkCmdFN == "whatever"} {
		set tsharkCmd "/root/bin/tsharkCmdLine"
	} else {
		set tsharkCmd "$tsharkCmdFN"
	}
	
	#fragments from Base.tcl
	set fd [eval $self rpopen $tsharkCmd]
	# Set up nonblocking reader event
	fconfigure $fd -blocking 0
	fileevent $fd readable {set reading READY}

	UTF::Sleep 1

	set psCmdLine "ps -ef | grep tshark"
	set catch_resp [catch "set processId \[$self rexec $psCmdLine\]" catch_msg]
        if {$catch_resp == 0} {
	    puts "processId=$processId"
        } else {
	    error "ERROR: Problems starting tshark in startRoamCmdline: $catch_msg"
        }

	set matchresult [regexp {([0-9]+)} $processId match PID]
	puts "matchresult=$matchresult"
	if {$matchresult != 1} {
	    error "ERROR: Problems starting tshark in startRoamCmdline regexp: no PID found"
	} else {
	    puts "PID=$PID"
	}
	return $PID
    }

    UTF::doc {
	# [call [cmd name] [method stop]]

	# This function does the following:
	# [list_begin enum]
	# [enum] invoke "kill -9 tshark <pid>" where <pid> is the passed in process id
	# [list_end]
    }

    method stop {processId} {
        UTF::Message INFO "" "************************LibFunc: $options(-name) stop $processId***************************"
    	set catch_resp [catch "set processId \[$self rexec kill -9 $processId\]" catch_msg]
        if {$catch_resp != 0} {
        	error "ERROR: Problems stopping tshark: $catch_msg"
        }
    }

    UTF::doc {
	# [call [cmd name] [method stopall]]

	# This function does the following:
	# [list_begin enum]
	# [enum] invoke "killall tshark"
	# [enum] this will kill all instance of tshark
	# [list_end]
    }

    method stopall {} {
	set catch_resp [catch "set processId \[$self rexec killall tshark\]" catch_msg]
        if {$catch_resp != 0} {
	    error "ERROR: Problems stopping tshark: $catch_msg"
        }
	# kill all instances of tshark
	# $self rexec killall tshark
    }

    UTF::doc {
	# [call [cmd name] [method changeChanspec]]

	# This function does the following:
	# [list_begin enum]
	# [enum] down the current wireless interface
	# [enum] invoke "wl chanspec <interface> <chanspec>" where
	# <interface> is the wireless interface you want to change
	# the  change the chanspec and <chanspec> is the chanspec you
	# want to change to.
	# [list_end]
    }
    method changeChanspec {chanspec} {
	# wl down passed in sta interface
	# change the chanspec
	# wl up passed in sta interface
    }

    UTF::doc {
	# [call [cmd name] [method reset]]

	# This function does the following:
	# [list_begin enum]
	# [enum] resets to the original state of the sniffer before
	# modification
	# [list_end]
    }
    method reset {} {
	# kill all instances of tshark
	# bring down all wireless interfaces
	# bring down all prism interfaces
    }

    UTF::doc {
        # [call [cmd name] [method setupDir]]
	
        # This function does the following:
        # Create a directory to store captured files
    }

    method setupDir {} {
        # Setup .pcap log directory 

        # Make pcap subdirectory in main UTF summary directory.
        # Need to include test number, to keep pcap directories
        # separate for multiple runs done by a higher level script.
        # This keeps the pcap files separated for each test.
        set ::pcap_log_dir [file dirname "$UTF::Logfile"]
        if {$UTF::Logfile eq ""} {
            set ::pcap_log_dir "."
        }
        set ::pcap_log_dir [file nativename $::pcap_log_dir] ;# gets rid of tilde
	    set testnum [UTF::get_testnum]
	    set ::log_dir "$::pcap_log_dir/pcap_${testnum}" ;# do as 2 steps or you end up with unmatched double quote in path
	    set catch_resp [catch "file mkdir $::log_dir" catch_msg]
        UTF::Message LOG "$::localhost" "mkdir $::log_dir catch_resp=$catch_resp catch_msg=$catch_msg"
	   
        # Return a web link to the current directory for easy access to the
        # pcap files.
        return "html:<a href=\"pcap_${testnum}\">.pcap files</a>"
    }

    UTF::doc {
	# [call [cmd name] [method collectFile]]

	# This function does the following:
	# Copy a single captured (.pcap) file from sniffer root directory
	# to the directory user created, fix the file permission
	# and remove the file from sniffer root directory.
	# For ring buffer, please don't call this function, instead, 
	# call "collectRingFiles"
    }

    method collectFile {} {

        # Setup checks
        if {![info exists ::log_dir]} {
            error "collectFile Please call method setupDir first."
        }
        if {![info exists ::sniffer_file]} {
            error "collectFile Please call method snifferCtl first."
        }

	    # Collect a single .pcap file.
        set error_list ""
        set dest_file $::log_dir/$::sniffer_file
        set catch_resp [catch "$self copyfrom /root/$::sniffer_file $dest_file" catch_msg]
        UTF::Message LOG "$::localhost" "collectFile copyfrom catch_resp=$catch_resp catch_msg=$catch_msg"
        if {$catch_resp != 0} {
            append error_list " $catch_msg"
        }

        # Fix file permission for this file.
        set catch_resp [catch {file attributes $dest_file -permissions 00644} catch_msg]
        UTF::Message LOG "$::localhost" "collectFile dest_file=$dest_file file permissions catch_resp=$catch_resp catch_msg=$catch_msg"
        if {$catch_resp != 0} {
            append error_list " $catch_msg"
        }
	
        # Remove the single .pcap file from the sniffer.
        set catch_resp [catch "$self rm -f /root/$::sniffer_file" catch_msg]
        UTF::Message LOG "$::localhost" "collectFile remove .pcap catch_resp=$catch_resp catch_msg=$catch_msg"
        if {$catch_resp != 0} {
            append error_list " $catch_msg"
        }

        # Report any errors to calling routine.
        if {$error_list != ""} {
            error "collectFile ERROR: $error_list"
        } else {
            return
        }
    }

    UTF::doc {
        # [call [cmd name] [method collectRingFiles]]

        # This function does the following:
        # When using ring buffer, copy the captured files (in this case, 
        # system doesn't add .pcap to the file name) from sniffer root directory
        # to the directory user created, add .pcap extension, fix these files permission
        # and remove those files from sniffer root directory.
        # If not using ring buffer, please don't call this function, instead,
        # call "collectFile" 
    }

    method collectRingFiles {} {

        # Setup checks
        if {![info exists ::log_dir]} {
            error "collectRingFiles Please call method setupDir first."
        }
        if {![info exists ::ring_files]} {
            error "collectRingFiles Please call method snifferCtl first."
        }

        # Copy over multiple files.
        set error_list ""
        set catch_resp [catch "$self copyfrom /root/${::ring_files}* $::log_dir" catch_msg]
        UTF::Message LOG "$::localhost" "collectRingFiles copyfrom catch_resp=$catch_resp catch_msg=$catch_msg"
        if {$catch_resp != 0} {
            append error_list " $catch_msg"
        }

        # Ring files dont have .pcap extension. Having a .pcap extension
        # allows web browsers to launch the file into Wireshark. So we
        # rename the file & fix file permission for each file.
        set pcap_list [glob -nocomplain -directory $::log_dir ${::ring_files}*]
        foreach file $pcap_list {
            # Because of the glob wildcard, we may pick up files that have
            # already been processed. So if the file has a .pcap extension,
            # then move on to the next file.
            if {[regexp {\.pcap$} $file]} {
                # puts "skipping $file"
                continue
            }
            set dest_file "${file}.pcap"
            set catch_resp [catch {file rename -force $file $dest_file} catch_msg]
            UTF::Message LOG "$::localhost" "collectRingFiles $file renamed $dest_file catch_resp=$catch_resp catch_msg=$catch_msg"
            if {$catch_resp != 0} {
                append error_list " $catch_msg"
            }

            set catch_resp [catch {file attributes $dest_file -permissions 00644} catch_msg]
            UTF::Message LOG "$::localhost" "collectRingFiles dest_file=$dest_file file permissions set catch_resp=$catch_resp catch_msg=$catch_msg"
            if {$catch_resp != 0} {
                append error_list " $catch_msg"
            }
        }

        # Remove specific ring buffer .pcap files from the sniffer.
        set catch_resp [catch "$self rm -f /root/${::ring_files}*" catch_msg]
        UTF::Message LOG "$::localhost" "collectRingFiles remove .pcap catch_resp=$catch_resp catch_msg=$catch_msg"
        if {$catch_resp != 0} {
            append error_list " $catch_msg"
        }

        # Report any errors to calling routine.
        if {$error_list != ""} {
            error "collectRingFiles ERROR: $error_list"
        } else {
            return
        }
    }

    UTF::doc {
        # [call [cmd name] [method trySetupSniffer] [opt sniffer_ch]]

        # This function does the following:
        # Try up to three times to setup sniffer.[para]

        # Optional argument sniffer_ch is the desired channel to use.
        # Default is 1.
    }
    
    method trySetupSniffer { args } {

        # To facilitate interactive testing, sniffer_ch can be
        # passed via the command line args.
        set args [string trim $args]
        if {$args != ""} {
            set ::utf_sniffer_ch $args
        }

        # Check we have sniffer channel
        if {![info exists ::utf_sniffer_ch]} {
            set ::utf_sniffer_ch 1
        } 

        # Hook to temporarily ignore the sniffer.
        if {$::utf_sniffer_ch <= 0} {
            set msg "Not using the sniffer in this test run"
            catch "unset ::utf_sniffer_if"
            return "$msg"
        }

        # Check we have sniffer ifconfig virtual interface.
        if {![info exists ::utf_sniffer_if]} {
            set ::utf_sniffer_if "prism0"
        } 

        # Use existing sniffer driver by doing a reload
        UTF::Message LOG "$::localhost" "Sniffer driver reload"
        $self reload
            
        # Setup sniffer on appropriate channel
        # Sometimes the snifferSetup fails the first time, but works
        # if we try again.
        for {set i 1} {$i <= 3} {incr i} {
            UTF::Message LOG "$::localhost" "trySetupSniffer try#: $i"
            set resp [$self setupSniffer $::utf_sniffer_ch]
            # set resp "error" ;# test code
            set resp [string trim $resp]
            # puts "i=$i resp=$resp"
            if {$resp == ""} {
                break
            }
        }

        # If multiple attemps to setup the sniffer failed, we turn it off,
        # so that we dont get errors from each test case later on.
        if {[string match -nocase *error* $resp] ||\
            [string match -nocase *unknown* $resp]} {
            catch "unset ::utf_sniffer_if"
            UTF::Message LOG "$::localhost" "trySetupSniffer Turning off sniffer\
                to avoid a cascade of errors."
            error "$resp"
        }

        # If we did succeed on a retry, discretely let the user know.
        if {$i == 1} {
            return "$resp"
        } else {
            return "setupSniffer succeded on try#: $i $resp"
	    }
	}

    UTF::doc {
        # [call [cmd name] [method snifferCtl] [arg cmd] [opt args]]

        # This function does the following:
        # Start or stop sniffer, trace sniffer or not, use ring buffer 
        # or not to save captured data.[para]

        # [arg cmd] a command to operate on sniffer: normally "start" or "stop"
        # sniffer.[para]

        # [opt args] how to trace sniffer: no - don't need to trace sniffer;
        # ring - use ring buffer, object options determine file number & file
        # size, defaults to 6 * 50MB, keep removing the oldest file while
        # saving data to the newest file; yes - save captured data to a single
        # file without using ring buffer. Last arg can be prismN which 
        # facilitates sniffer control from the command line.
    }

    method snifferCtl {cmd args} {

        # Clean up calling parameters
        set cmd [string trim $cmd]
        set args [string trim $args]

        # We need to be able to pass ::sniffer_if via args to facilitate 
        # interactive testing.
        if {[llength $args] > 1} {
            set ::utf_sniffer_if [lindex $args end] ;# grab last arg
            set args [lrange $args 0 end-1] ;# save remainder of args
        }

        # Check for sniffer interface.
        if {![info exists ::utf_sniffer_if]} {
            UTF::Message LOG $::localhost "snifferCtl ::utf_sniffer_if doesn't exist. sniffer will not be used."
            return
        }
        UTF::Message LOG $::localhost "snifferCtl cmd=$cmd args=$args utf_sniffer_if=$::utf_sniffer_if"

        # Modify the cmd & args appropriately, run the command.
        if {$cmd == "start"} {

            # If args is no, we dont need the sniffer trace.
            if {$args == "no"} {
                set ::sniffer_file ""
                return
            }

            # Make sure tc_num is defined.
            if {![info exists ::tc_num]} {
                set ::tc_num "0"
            }
	    
            # Setup tshark command with output log_file.
            set date [clock format [clock seconds] -format "%Y%m%d_%H%M%S"]
            set ::sniffer_file "${date}_${self}_TC${::tc_num}.pcap" ;# for single .pcap
            regsub -all {:} $::sniffer_file "" ::sniffer_file ;# dont need colons
            regsub -all {/} $::sniffer_file "" ::sniffer_file ;# in case tc_num has /
            set ::ring_files "${self}_TC${::tc_num}" ;# for ring buffer files
            regsub -all {:} $::ring_files "" ::ring_files ;# dont need colons
            regsub -all {/} $::ring_files "" ::ring_files ;# in case tc_num has /

            # Start single file or ring buffer.
            if {$args == "ring"} {
                # Startup multiple ring buffer .pcap files.
                set fsize [expr $options(-ringbuffersize) * 1000] ;# convert MB to KB
                set fcnt $options(-numfilesrb)
            	set catch_resp [catch "$self start \"tshark -b filesize:$fsize -b files:$fcnt -i $::utf_sniffer_if -w /root/$::ring_files\"" catch_msg]
            } else {
                # Startup a single .pcap file.
            	set catch_resp [catch "$self start \"tshark -i $::utf_sniffer_if -w /root/$::sniffer_file\"" catch_msg]
            }

        } elseif {$cmd == "stop"} {
            set catch_resp [catch "$self pkill tshark" catch_msg]
            # Ignore complaints from pkill when there are no tshark processes.
            set catch_resp 0

        } else {
            # Run a UTF command to operate on the sniffer.
            set catch_resp [catch "$self $cmd $args" catch_msg]
        }

        # Throw error if necessary.
        if {$catch_resp == 0} {
            return
        } else {
            error "snifferCtl $catch_msg"
        }
    }
}

# Retrieve manpage from last object
UTF::doc [UTF::Sniffer man]

UTF::doc {
    # [list_end]

    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also [uri APdoc.cgi {Unit Test Framework Tools}]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
