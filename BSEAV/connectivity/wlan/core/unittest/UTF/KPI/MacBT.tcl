#!/bin/env utf
#

# BT objects for Olympic System Testing
# Author Robert McMahon & Peter Kwan September 2015

# $Id: f0f6b344f97121a0029f20ce51701953cc377fe4 $

#
# ehancements: 	look up routines cover both paired and connected devices
#				connect/disconnect individual device supported
#
#				BTMouse, BTKeyboard, and BTSpeaker object prototypes
#				Flattened object inheritance/delegation structure: objects < basemethods
#				Directly use address given in option for all speaker object operations
#
#				Use UTF pty enhancements for Mac devices which line-buffers outputs
#				HID and Audio tests return FD/handles to allow test runs beyond 30 seconds
#				Outputs from HID and Audio tests allow high-resolution timestamps disabled
#
#				play_and_measure methods in all object types that returns results
#				play_and_measure methods include BT radio power reset option
#				play_and_measure runs in background
#				Optional output to file in all measuring tests
#				Multiple checks in starting iTunes traffic, including process ID
#				Separating Play, Measure and Reporting in all object types
#				Boolean values: 1 = success/on; 0 = failure/off
#				HID devices play_and_measure allows exclusive test on calling device


package require snit
package require UTF
package require md5

package provide UTF::MacBT 2.0

snit::type UTF::BTSpeaker {
    option -address -default "04-88-e2-81-24-76"
    variable myname "speaker"
    
    component mybase -inherit true
    constructor {args} {
	    set mybase [UTF::BTBaseMethods %AUTO%]
		$self configurelist $args
    }
	
    method checkMyBattery {} {
		$mybase checkBattery $myname
    }
    
    method isconnected? {} {
		$mybase getDevConnectionStatus $myname
    }
    
    method runMyTraffic { {fName ""} } { ;# runs iTunes traffic in foreground
	    $mybase getReady $myname
	    catch { $mybase startTraffic $myname $fName } results
	    return $results
    }
    
    method stopMyTraffic {} {
	    $mybase stopTraffic $myname
    }
    
    method runMyTest { args } { ;# run test in foreground
    	UTF::Getopts {
	    	{rtime.arg		"10" "Run time in seconds"}
	    	{filename.arg	"" "Name of iTunes file"}
		}

	    $self getReady
	    $mybase startTraffic $myname $(filename)
	    UTF::Sleep 2
	    
	    catch { $mybase checkA2DPBitRate $myname $(rtime) } results
		return $results
    }

    method ispaired? {} {
	    #puts "My address from option: $options(-address)"
	    set myaddress $options(-address)
	    if { $myaddress eq "" || $myaddress eq 0xff } {
		    error $utfmsgtag "Failed to obtain valid device address."
	    } else {
			$mybase isPaired $myaddress
		}
    }
    
    method getMyname {} {
	    return $myname
    }
    
    method getMyStatus {} {
	    $mybase queryDevice $myname
    }
    method getReady {} {
		set myAddress $options(-address)
		if { $myAddress != "" } {
			set myDev $myAddress
		} else {
			set myDev $myname
		}			
		$mybase getReady $myDev
	}

	method connectMe { {limit 3} } {
		set tCnt 1
		set cMsg ""

		#puts "myAddress as in option: $options(-address)"
		set myAddress $options(-address)
		if { $myAddress != "" } {
			set myDev $myAddress
		} else {
			set myDev $myname
		}			
		catch { $mybase connectDevice $myDev } cMsg
		#puts "cMsg returned from connectDevice: $cMsg\n"
		if { $cMsg ne 0 } {
			UTF::Message FAIL $myname "Connecting $myname failed."
		}
		
		while { $cMsg ne 0 } {
			UTF::Message INFO $myname "Retry connecting device $myname: \[$tCnt of $limit\]"
			catch { $mybase connectDevice $myDev } rcMsg
			UTF::Message DEBUG $myname "Inside rcMsg returned: $rcMsg"
			if { $rcMsg ne 0 } {
				UTF::Sleep 2
				incr tCnt
				if { $tCnt <= $limit } {
					continue
				} else {
					UTF::Message FAIL $myname "Exhausted retry."
					return 0xff
					break
				}
			} else {
				UTF::Message INFO $myname "Succeed after $tCnt retry attempt(s)."
				return 0
				break
			}
		}
		
		return $cMsg
	}
	
	method pairMe {} {
		$mybase pairDev $options(-address)
	}
		
	method disconnectMe {} {
		if { $options(-address) != "" } {
			set myDev $options(-address)
		} else {
			set myDev $myname
		}
		$mybase disconnectDevice $myDev
	}
	
	method unpairMe {} {
		$mybase unpairDev $options(-address)
	}

    method getMyTestFD { {rtime ""} } {
	    # use UTF rpopen procedure
	    $self getReady
	    $mybase startTraffic $myname
	    UTF::Sleep 2
	    if { $rtime eq "" } { set rtime 10 }

		set fd_mytest [$mybase getTestFD $myname $rtime]; #puts "FD returned: $fd_mytest"
		return $fd_mytest
    }
    
	method getMyFDResults { fd } {
		catch {$mybase getFDResults $fd} results ; #puts "Results:\n$results"
		return $results
	}
	
	method reinit {} {
 		catch {$mybase __reinit -address [$self getMyAddress] -device [$self getMyname] -retry } reinitMsg ;# use this
# 		catch { $mybase __reinit -address [$self getMyAddress] -device [$self getMyname] -sleep 15 } reinitMsg ;# no retry
# 		catch {$mybase __reinit -address "qqqq" -device [$self getMyname] -retry } reinitMsg ;# use this to debug
	}
	

	method getMyAddress {} {
		catch { $mybase __getDevAddress -device $myname -address $options(-address) } myAddress
		return $myAddress
	}

	method play { {fName ""} } {
		catch { $mybase __startiTunesTraffic $fName } results
	    UTF::Message DEBUG $myname "Catch message returned from startiTunesTraffic:\n$results"
	    return $results
    }
    
    method measure { args } {
		UTF::Getopts {
			{noHRTimeStamps 	"" "Turn off high resolution timestamps; default is on"}
			{rtime.arg 			"" "Run time in seconds"}
			{retries.arg 		"3" "Maximum retry attempts"}
		}
		
	    # first check for active audio device, abort if none found
	    catch { $self __checkAudioDevice } aDev
	    #puts "Value returned from __checkAudioDevice: $aDev"
	    if { $aDev eq 0 } {
		    UTF::Message WARN "" "Audio Device may not be ready. Retry..."
			for {set rCnt 1} {$rCnt <= $(retries)} {incr rCnt} {
			    UTF::Message INFO "" "Retry $rCnt"
				UTF::Sleep 3
			    catch { $self __checkAudioDevice } aDev
			    if { $aDev eq 1 } {
				    UTF::Message INFO "" "Retry $rCnt: Audio Device ready."
				    break
			    }
		    }
		    if { $rCnt >= $(retries) } {
			    UTF::Message FAIL "" "Audio Device not ready. Exceeded retry: $(retries)"
		    }
	    }
	    
	    if { $aDev eq 0 } { ;# stop when Audio Device not ready
		    error "Audio device not active."
		    return 0
	    } else {
		    if { $(noHRTimeStamps) } {
			    UTF::Message INFO $myname "High resolution timestamps chosen."
			    catch { $self unsetHRTimeStamps } stmpMsg
			    UTF::Message DEBUG $myname "setHRTimeStamps returned message:\n$stmpMsg"
		    }
		    catch { $mybase getTestFD $myname $(rtime) } bRate ;# this works
		    UTF::Message DEBUG $myname "Catch message returned from checkA2DPBitRate:\n$bRate"
		    return $bRate
		    if { $(noHRTimeStamps) } { ;# reset HRTimeStamps
			    catch { $self setHRTimeStamps }
		    }
	    }
    }
	
    method getRate { fd {rtime ""} } {
	    catch { $mybase getFDResults $fd } results
	    #UTF::Message DEBUG $myname "Catch message returned from getFDResults:\n$results"
	    return $results
    }
    
    method play_and_measure { args } {
		UTF::Getopts {
			{filename.arg 	"" "Name of iTunes file"}
			{rtime.arg 		"" "Run time in seconds"}
			{sleep.arg 5 	"Pause time"}
			{noHRTimeStamps	"" "Turn off high resolution time stamps; default on"}
			{reinit			"" "Reinitialize BT Radio state"}
			{outfile.arg	"" "Name of output file"}
			{retries.arg	"3" "Maximum retry attempts"}
		}
	 
		if { $(reinit) } {
			catch { $self reinit }
		}

		$self play $(filename); UTF::Sleep 3
		if { $(noHRTimeStamps) } {
			set fd [$self measure -rtime $(rtime) -noHRTimeStamps -retries $(retries)]
		} else {
			set fd [$self measure -rtime $(rtime) -retries $(retries)]
		}
		if { $(outfile) != "" } {
			set output [$self getRate $fd $(rtime)]
			set ofd [open /tmp/$(outfile) w];#puts "ofd is: $ofd"
			foreach line [split $output "\n"] {
				puts $ofd $line
			}
			close $ofd
		} else {
			$self getRate $fd $(rtime)
		}
    }
    
}

snit::type UTF::BTMouse {
	option -address -default "24-a0-74-89-b1-b2"
    variable myname "mouse"
    
    component mybase -inherit true
    constructor {args} {
		set mybase [UTF::BTBaseMethods %AUTO%]
		$self configurelist $args
    }
    
# # #     destructor {
# # #     }
# # #     method inspect {} {
# # #     }
# # #     method pair {} {
# # #     }
# # #     method move {args} {
# # # 	UTF::Getopts {
# # # 	    {inrange "Move device in to BT range"}
# # # 	    {outrange "Move devce out of BT range"}
# # # 	}
# # # 	$self radio off
# # #     }
    
    method checkMyBattery {} {
	    $mybase checkBattery $myname
    }
    
    method isconnected? {} {
	    $mybase getDevConnectionStatus $myname
    }
    
    method runMyTraffic { {rtime ""} } { ;# new code 4/22/16 -- runs test in background
	    $self getReady
	    
		catch {$mybase getTestFD $myname $rtime} fd; puts "FD returned: $fd"
		catch {$self getMyFDResults $fd} results
		return $results
    }

    method runOnlyMyTraffic { {rtime ""} } {
		if { [$mybase connectOnlyOneHIDDevice $myname] ne 0 } {
			$self connectMe
		}
		catch { $self runMyTraffic $rtime } results
		return $results
    }
    
    method ispaired? {} {
	    set myaddress [$self cget -address]
	    if { $myaddress eq "" || $myaddress eq 0xff } {
		    error $utfmsgtag "Failed to object valid device address."
	    } else {
			$mybase isPaired $myaddress
		}
    }
    
    method getMyname {} {
		return $myname
    }
    
    method getMyStatus {} {
	    $mybase queryDevice $myname
    }
    
    method getReady {} {
	    $mybase getReady $myname
	}

	method connectMe { {limit 3} } {
		set tCnt 1
		set cMsg ""
		catch { $mybase connectDevice $myname } cMsg
		if { $cMsg ne 0 } {
			UTF::Message FAIL $myname "Connecting $myname failed."
		}
		
		while { $cMsg ne 0 } {
			UTF::Message INFO $myname "Retry connecting device $myname: \[$tCnt of $limit\]"
			catch { $mybase connectDevice $myname } rcMsg
			UTF::Message DEBUG $myname "Retry rcMsg returned: $rcMsg"
			if { $rcMsg ne 0 } {
				UTF::Sleep 2
				incr tCnt
				if { $tCnt <= $limit } {
					continue
				} else {
					UTF::Message FAIL $myname "Exhausted retry."
					return 0xff
					break
				}
			} else {
				UTF::Message INFO $myname "Succeed after $tCnt retry attempt(s)."
				return 0
				break
			}
		}
		return $cMsg
	}
	
	method pairMe {} {
		$mybase pairDev $options(-address)
	}
	
	method disconnectMe {} {
		$mybase disconnectDevice $myname
	}
	
	method unpairMe {} {
		$mybase unpairDev [$self cget -address]
	}
	
	method getMyTrafficFD { {rtime ""} {sleep 5} } {
		$self getReady
		set fd [$mybase getTestFD $myname $rtime]; #puts "FD returned: $fd"
		return $fd
	}
	
    method getOnlyMyTrafficFD { {rtime ""} {sleep 5} } {
	    # connectOnlyOneHIDDevice returns "1" when only one device is connected
		if { [$mybase connectOnlyOneHIDDevice $myname] ne 0 } {
			$self connectMe
		}

		set fd [$self getMyTrafficFD $rtime]
		return $fd
	}
	
	method getMyFDResults { fd } {
		catch {$mybase getFDResults $fd} results 
		#puts "Results:\n$results"
		return $results
	}
	
	method reinit {} {
		catch {$mybase __reinit -address [$self getMyAddress] -device [$self getMyname] -retry } reinitMsg
# 		catch {$mybase __reinit -address [$self getMyAddress] -device [$self getMyname] } reinitMsg ;# case: no retry
	}
	
	method getMyAddress {} {
		catch { $mybase __getDevAddress -device $myname -address $options(-address) } myAddress
		return $myAddress
	}

	method start { {rtime ""} } { ;# runs HID traffic in background
		catch {$self getMyTrafficFD $rtime} fd
	    UTF::Message DEBUG $myname "Catch message returned from getMyTrafficFD:\n$fd"
	    return $fd
    }
    
    method measure { fd } {
		catch { $self getMyFDResults $fd } results
		return $results
    }

    method play_and_measure { args } {
		UTF::Getopts {
			{rtime.arg 10 	"Run time in seconds"}
			{reinit 		"" "Reinitialize BT Radio state"}
			{retries.arg 	"3" "Maximum retry attempts"}
			{outfile.arg 	"" "Name of output file"}
			{onlyMe 		"" "Measure HID traffic exclusively for this device, default is whatever device(s) active"}
			{noHRTimeStamps	"" "Turn off high resolution timestamps; default is on"}
			{all 			"" "Activate all known HID devices for HIDPacketErrorRate measurement"}
		}

		if { $(reinit) } {
			catch { $self reinit }
		}
		
		if { $(noHRTimeStamps) } {
			catch { $self unsetHRTimeStamps }
		}
		
		if { $(onlyMe) } {
			#puts "This is onlyMe branch"
			catch { $self getOnlyMyTrafficFD $(rtime) } fd
		} else {
			if { $(all) } { ;# case testing all active HID devices
				if { [regexp -nocase {.*not_connected.*} [$self isconnected?]] } {
					UTF::Message INFO $myname "$myname myself not ready. Reconnect before proceeding..."
					$self connectMe
				}
				set oDev [$mybase findOtherHIDDevice $myname]
				UTF::Message INFO $myname "The other device found: $oDev"
				if { [regexp -nocase {.*not_.*} [ $self getDevConnectionStatus $oDev ] ] } { ;# connect other device if not active
					UTF::Message INFO $myname "The other device [lindex $oDev 0] is not connected. Connecting..."
					catch { $self connectDevice [lindex $oDev 0] }
				}
				set hDevs [ $self checkActiveHIDDevices ]
				UTF::Message INFO $myname "Active HID device count: $hDevs"
				if { $hDevs < 2 } {
					UTF::Message INFO $myname "Not all HID devices are ready."
				}
			} ;# end all active HID devices
			# test whatever is active; may not be ME
			catch { $self start $(rtime) } fd
		}
		
		if { $(outfile) != "" } {
			set results [ $self measure $fd ]
			set ofd [open /tmp/$(outfile) w]
			#puts "ofd is: $ofd"
			foreach line [split $results "\n"] {
				puts $ofd $line
			}
			close $ofd
		} else {
			set results [ $self measure $fd ]
			return $results
		}
		
		if { $(noHRTimeStamps) } { ;# restores high resolution timestamps
			catch { $self setHRTimeStamps }
		}
    }
}

snit::type UTF::BTKeyboard {
    option -address -default "40-30-04-11-b7-46"
    variable myname "keyboard"
    
    component mybase -inherit true
    constructor {args} {
		set mybase [UTF::BTBaseMethods %AUTO%]
		$self configurelist $args
    }
    
# # #     destructor {
# # #     }
# # #     method inspect {} {
# # #     }
# # #     method pair {} {
# # #     }
# # #     method move {args} {
# # # 	UTF::Getopts {
# # # 	    {inrange "Move device in to BT range"}
# # # 	    {outrange "Move devce out of BT range"}
# # # 	}
# # # 	$self radio off
# # #     }

# begin
    method checkMyBattery {} {
	    $mybase checkBattery $myname
    }
    
    method ispaired? {} {
		set myaddress [$self cget -address]
		if { $myaddress eq "" || $myaddress eq 0xff } {
		    error $utfmsgtag "Failed to object valid device address."
	    } else {
			$mybase isPaired $myaddress
		}
    }
    
    method isconnected? {} {
	    $mybase getDevConnectionStatus $myname
    }
    
    method runMyTraffic { {rtime ""} } { ;# new code 4/22/16 -- runs test in background
	    $self getReady

		catch {$mybase getTestFD $myname $rtime} fd; puts "FD returned: $fd"
		catch {$self getMyFDResults $fd} results
		return $results
    }
    
    method runOnlyMyTraffic { {rtime ""} } {
		if { [$mybase connectOnlyOneHIDDevice $myname] ne 0 } {
			$self connectMe
		}
		catch { $self runMyTraffic $rtime } results
		return $results
	}
    
    method getMyname {} {
	    return $myname
    }
    
    method getMyStatus {} {
	    $mybase queryDevice $myname
    }
    
    method getReady {} {
	    $mybase getReady $myname
	}
		
	method connectMe { {limit 3} } {
		set tCnt 1
		set cMsg ""
		catch { $mybase connectDevice $myname } cMsg
		if { $cMsg ne 0 } {
			UTF::Message FAIL $myname "Connecting $myname failed."
		}
		
		while { $cMsg ne 0 } {
			UTF::Message INFO $myname "Connecting device $myname: Retry \[$tCnt of $limit\]"
			catch { $mybase connectDevice $myname } rcMsg
			UTF::Message DEBUG $myname "Inside rcMsg returned: $rcMsg"
			if { $rcMsg ne 0 } {
				UTF::Sleep 2
				incr tCnt
				if { $tCnt <= $limit } {
					continue
				} else {
					UTF::Message FAIL $myname "Exhausted retry."
					return 0xff
					break
				}
			} else {
				UTF::Message INFO $myname "Succeed after $tCnt retry attempt(s)."
				return 0
				break
			}
		}
		return $cMsg
	}
	
	method pairMe {} {
		set myaddress $options(-address)
		if { $myaddress ne "" } {
			if { [ $mybase isPaired $myaddress ] eq "Not Paired" } {
				set rtnCode 2
				UTF::Message WARNING $myname "Pairing $myname requires interaction with System Preferences GUI. Please do this at the machine."
			} else {
				#Device already paired. Do nothing.
				UTF::Message INFO $myname "Device already paired."
				set rtnCode 1
			}
		} else { ;# case address unknown
			UTF::Message ERROR $myname "Device address is required to proceed.\n"
			rtnCode 0xff
		}
		return $rtnCode
	}
	
	method disconnectMe {} {
		$mybase disconnectDevice $myname
	}
	
	method unpairMe {} {
		set myaddress $options(-address)
		if { $myaddress ne "" } {
			if { [ $mybase isPaired $myaddress ] eq "Not Paired" } {
				UTF::Message INFO "" "Device already unpaired."
				set rtnCode 1
			} else {
				#Device already paired. Do nothing,
				set rtnCode 0xff
				UTF::Message WARNING $myname "Pairing $myname after unpairing requires interaction with System Preferences GUI. Please do this at the machine."
			}
		} else { ;# case address unknown
			UTF::Message ERROR "" "Device address is required to proceed.\n"
			set rtnCode 0xff
		}
		return $rtnCode
	}

	method getMyTrafficFD { {rtime ""} {sleep 5} } {
		$self getReady
		set fd [$mybase getTestFD $myname $rtime]
		#puts "FD returned: $fd"
		return $fd
	}
	
    method getOnlyMyTrafficFD { {rtime ""} {sleep 5} } {
		if { [$mybase connectOnlyOneHIDDevice $myname] ne 0 } {
			$self connectMe
		}

		set fd [$self getMyTrafficFD $rtime]
		#puts "OnlyMyTraffic fd: $fd"
		return $fd
		
    }
    
	method getMyFDResults { fd } {
		catch {$mybase getFDResults $fd} results 
		#puts "Results:\n$results"
		return $results
	}
	
	method reinit {} {
		catch {$mybase __reinit -address [$self getMyAddress] -device [$self getMyname] -retry } reinitMsg
	}
	
	method getMyAddress {} {
		catch { $mybase __getDevAddress -device $myname -address $options(-address) } myAddress
		return $myAddress
	}
	
	method start { {rtime ""} } { ;# runs HID traffic in background
		catch {$self getMyTrafficFD $rtime} fd
	    UTF::Message DEBUG $myname "Catch message returned from getMyTrafficFD:\n$fd"
	    return $fd
    }
    
    method measure { fd } {
		catch { $self getMyFDResults $fd } results
		return $results
    }

    method play_and_measure { args } {
		UTF::Getopts {
			{rtime.arg 		10 "Run time in seconds"}
			{reinit 		"" "Reinitialize BT Radio state"}
			{retries.arg	"3" "Maximum retry attempts"}
			{outfile.arg	"" "Name of output file"}
			{onlyMe			"" "Measure HID traffic exclusively for this device, default is whatever device(s) active"}
			{noHRTimeStamps	"" "Turn off high resolution timestamps; default is on"}
			{all			"" "Activate all known HID devices for HIDPacketErrorRate measurement"}
		}

		if { $(reinit) } {
			catch { $self reinit }
		}
		
		if { $(noHRTimeStamps) } {
			catch { $self unsetHRTimeStamps }
		}
		
		if { $(onlyMe) } {
			#puts "This is onlyMe branch"
			catch { $self getOnlyMyTrafficFD $(rtime) } fd
		} else {
			if { $(all) } { ;# case testing all active HID devices
				if { [regexp -nocase {.*not_connected.*} [$self isconnected?]] } {
					UTF::Message INFO $myname "$myname myself not ready. Reconnect before proceeding..."
					$self connectMe
				}
				set oDev [$mybase findOtherHIDDevice $myname]
				#puts "The other device found: $oDev"
				UTF::Message INFO $myname "The other device found: $oDev"
				if { [regexp -nocase {.*not_.*} [ $self getDevConnectionStatus $oDev ] ] } { ;# connect other device if not active
					UTF::Message INFO $myname "The other device [lindex $oDev 0] is not connected. Connecting..."
					catch { $self connectDevice [lindex $oDev 0] }
				}
				set hDevs [ $self checkActiveHIDDevices ]
				UTF::Message INFO $myname "Active HID device count: $hDevs"
				if { $hDevs < 2 } {
					UTF::Message INFO $myname "Not all HID devices are ready."
				}
			} ;# end all active HID devices
			# test whatever is active; may not be ME
			catch { $self start $(rtime) } fd
		}
		
		if { $(outfile) != "" } {
			set results [ $self measure $fd ]
			set ofd [open /tmp/$(outfile) w]
			#puts "ofd is: $ofd"
			foreach line [split $results "\n"] {
				puts $ofd $line
			}
			close $ofd
		} else {
			set results [ $self measure $fd ]
			return $results
		}
		
		if { $(noHRTimeStamps) } { ;# restores high resolution timestamps
			catch { $self setHRTimeStamps }
		}
    }
}

snit::type UTF::BTBaseMethods {
    typevariable EXCEPTATTRIBUTES "-name"

    option -sta -validatemethod __validatesta
    option -name -default {}
    option -verbose -default 0 -type boolean
    option -debug -default 0 -type boolean
    option -attngrp ""
	option -btpath -default "/usr/local/bin"
	option -itpath -default "/Users/user/Music/iTunes"
#	option -itpath -default "/Users/user/Music/iTunes/iTunes\\\ Media/Music/Unknown\\\ Artist/Unknown\\\ Album"

    variable myid
    variable utfmsgtag
    variable errorcode
    variable pDevsExist 1
    variable pDevList ""
    variable cDevsExist 1
    variable cDevList ""
	variable hidDevs "mouse keyboard"
    variable audDevs "loudspeaker speaker headset"

    # component mystream -inherit true
    constructor {args} {
	set errorstate ""
	$self configurelist $args
	if {$options(-name) eq ""} {
	    set utfmsgtag "[namespace tail $self]"
	    set options(-name) $utfmsgtag
	} else {
	    set utfmsgtag "[namespace tail $self]"
	}
	# set options(-protocol) [string toupper $options(-protocol)]
	# set mystream [UTF::stream %AUTO% -tx $options(-sta) -rx $options(-ap) -protocol $options(-protocol) -rate $options(-rate) -reportinterval 0.01]
	# $mystream configure -name "${options(-name)}-[namespace tail $mystream]"
	# $mystream id
	set mystate "CONFIGURED"
    }
    destructor {
	catch {$mystream destroy}
    }
    
    method __validatesta {option value} {
		if {$option eq "-sta" && [llength $value] != 1} {
		    error $utfmsgtag "STA option requires a single value"
		}
		if {[$value hostis != Linux] && [$value hostis != MacOS]} {
		    error $utfmsgtag "STA needs to be Linux or MacOS"
		}
    }
    
	method inspect {} {
	}

    method whatami {} {
		return "UTF::BTBaseMethods"
    }
    
    method setup {} {
		# This is for connecting existing paired devices
		# Check if there are paired device(s) present prior to 
		# constructing connection(s)
		set pDevs ""
		
		# compile paired list prior to proceed
		UTF::Message INFO $utfmsgtag "Finding known BT devices then connect them."
		if { $pDevList eq "" || $pDevsExist ne 1 } {
		# $self listDevices paired
			catch { $self listDevices paired } pMsg
			UTF::Message INFO "" "Catch message returned:\n$pMsg"
		}
		if { $pDevsExist eq 0 || ! [info exists pDevList] } { ;# if listDevices returned empty list
			error $utfmsgtag "No device with known profile found. Stop."
		}
		# connect known devices, then compile list of active devices
		# UTF::Message INFO $utfmsgtag "Known BT devices found. Connect them."
		# $self connectDevice $pDevs
		catch { $self connectDevice $pDevs } cMsg
		UTF::Message INFO "" "connectDevice Catch message returned:\n$cMsg"
		#UTF::Message DEBUG $utfmsgtag "pDevList is: $pDevList\n\ncDevList is: $cDevList\n"
    };# end  setup
    
    method listDevices { {lsType ""} } {
	    # compiles a list of devices with known profile in BD address and type pairs
	    set dFound 0
	    set pDevs ""
	    set devBD ""
	    set dType ""
	    set dList ""
	    set finalList ""
	    if { $lsType eq "" } { set lsType "paired connected" } 
	    
	    foreach ltype $lsType {
		    if { $ltype eq "paired" } {
			    set cmd "$options(-btpath)/applebt --listPairedDevices"
			    set cmpStr "All Bluetooth devices * removed"
			    set list pDevList
			    set flag pDevsExist
		    } else {
			    set cmd "$options(-btpath)/applebt --listConnectedDevices"
			    set cmpStr "No active connections"
			    set list cDevList
			    set flag cDevsExist
		    }
		    
			set lDevs [$options(-sta) {*}$cmd] ;# compile device list
			if { [regexp -nocase $cmpStr $lDevs] || $lDevs eq "" } { ;# case no device listed
			    if { $ltype eq "paired" } {
				    set pDevsExist 0
					# UTF::Message FAIL $utfmsgtag "No paired devices present."
					error $utfmsgtag "No paired devices present. Abort."
				} else {
				    set cDevsExist 0
					UTF::Message FAIL $utfmsgtag "No connected devices present."
				}
			} else { ;# case device found
				foreach line [split $lDevs "\n"] {
					if { [regexp -nocase {.*address:\s([0-9,a-f,A-F,-]+)} $line - devBD] } {
						set dType [$self getDeviceType $devBD]
						if { $dType eq 0xff } {
							# error $utfmsgtag "Failed to determine device type."
							UTF::Message FAIL $utfmsgtag "Failed to determine device type."
						} else {
							# UTF::Message DEBUG $utfmsgtag "Device found at: $devBD; $dType"
							append dList "$devBD $dType "
							incr dFound
						}
					}
				} ;# end of line
			};# end case device found
		
			if { $dFound eq 0 && [info exists {*}${list}] } {
				# UTF::Message FAIL $utfmsgtag "No device found."
				set ${flag} 0; set {list} ""
				return 0xff
			} else {
				# list of devices with known profile in address and type pairs
				# update paired device list and flag
				set ${flag} 1 ; set ${list} $dList
				set ltypeList $dList ; set dList ""
				# UTF::Message DEBUG $utfmsgtag "$flag value is set: [set $flag]\n; $list value is set: [set $list]\n"
				append finalList "$ltype: $ltypeList\n"
			}
			set ltypeList "" ;# clean out list contents
		};# end foreach ltype loop
		# UTF::Message DEBUG $utfmsgtag "FinalList is here: $finalList"
		return $finalList
	};# end listDevices
	
    method connectDevice {{cList ""}} {
	    set cResp ""
	    set cDev ""
	    set dType ""
	    set dList ""
	    set fList ""
	    set cCnt 0
	    
	    # refresh lists
	    $self listDevices
	    
	    # first check if target type devices present: stop if none exists
	    if { $pDevsExist eq 0 } {
		    error $utfmsgtag "No device with known profile found. Stop attempt to connect."
	    } else { ;# where paired devices exist
		    if { $cList eq "" } { ;# case no device given, connect all known devices
# 			    $self listDevices connected ;# compile current cDevList
			    set cList $pDevList
			    # if connection fails, OS throws timeout
			    foreach "cDev dType" $cList {
				    if { ! [regexp -nocase $dType $cDevList] } {
						set cResp [$options(-sta) $options(-btpath)/applebt --connect $cDev]
						UTF::Sleep 10 ;# allow devices to respond
					} else {
						UTF::Message DEBUG $utfmsgtag "$dType \@ $cDev already connected. Do nothing.\(1\)"
					}
					if { [regexp -nocase {.*connectioncomplete.*} $cResp] || $cResp eq "" } {
						UTF::Message INFO $utfmsgtag "$dType \@ $cDev connected."
						append dList "$cDev $dType "
						incr cCnt ; #puts "cCnt\(1\) value: $cCnt"
					} else { ;# device connect failed
						append fList "$cDev $dType "
					}
				} ;# end foreach loop
			} else { ;# case where device given as argument
				# lookup target device name and address, then connect
				set pList [$self lookupDevice paired $cList]
				foreach "cDev dType" $pList { ;# case where device not active
					if { ! [regexp -nocase $dType $cDevList] } {
						# set cResp [$options(-sta) $options(-btpath)/applebt --connect $cDev]
						catch {[$options(-sta) $options(-btpath)/applebt --connect $cDev]} cResp
						#puts "Show me cResp:\n===============================\n$cResp\n==============================="
						UTF::Sleep 10 ;# allow devices to respond
	
					} else { ;# case where device is active
						UTF::Message DEBUG $utfmsgtag "$dType \@ $cDev already connected. Do nothing.\(2\)"
					}
					if { [regexp -nocase {.*connectioncomplete.*} $cResp] || $cResp eq "" } { ;# case where connection success message returned
						UTF::Message INFO $utfmsgtag "$dType \@ $cDev connected."
						append dList "$cDev $dType "
						incr cCnt ;#puts "cCnt\(2\) value: $cCnt"
					} else {;# device connect failed
						#puts "Connect failed."
						append fList "$cDev $dType "
						#puts "fList contains:\n=================\n$fList\n================="
					}
				} ;# end foreach loop
			}
	    }
	    if { $cCnt > 0 } {
		    UTF::Message INFO $utfmsgtag "Devices connected successfully: $dList"
		    return 0
	    } else {
		    UTF::Message INFO $utfmsgtag "Devices failed to connect: $fList"
		    return 0xff
	    } 
	};# end connectDevice 

    method disconnectDevice {{dev ""}} {
	    set dDevs ""
	    set cCnt 0
	    set cList ""
	    set dList ""
	    
	    # update lists
	    $self listDevices
	    
# 	    $self listDevices connected ;# update cDevList
 	    if { $cDevsExist eq 0 || $cDevList eq "" } { ;# case no connected devices found
		    error $utfmsgtag "No connected device found."
	    } else { ;# case where connected devices present
		    if { $dev eq "" } { ;# default  - connect all if no device name or address given
			    # $self listDevices connected ;# update cDevList prior to proceed
			    set cList $cDevList 
			    # if connection fails, OS throws timeout
			    foreach "cDev dType" $cList {
					set cResp [$options(-sta) $options(-btpath)/applebt --disconnect $cDev]
					UTF::Sleep 2 ;# allow devices to respond
					if { $cResp eq "" } {
						UTF::Message INFO $utfmsgtag "$dType \@ $cDev disconnected."
						append dList "$cDev $dType "
						incr cCnt
					}
				}
			} else { ;# disconnect device given as argument
				set pList [$self lookupDevice connected $dev] ;# see if device is active
				if { $pList ne "" && $pList ne 0xff } {
					foreach "cDev dType" $pList { ;# disconnect active device if found
						set cResp [$options(-sta) $options(-btpath)/applebt --disconnect $cDev]
						UTF::Sleep 2 ;# allow devices to respond
						if { $cResp eq "" } {
							UTF::Message INFO $utfmsgtag "$dType \@ $cDev disconnected."
							append dList "$cDev $dType "
							incr cCnt
						}
					}
				} else { ;# case given device not connected
					# error $utfmsgtag "Device $dev is not connected. Check $dev."
					UTF::Message FAIL $utfmsgtag "Device $dev is not connected. Check $dev."
				}
			} ;# end connecting given device
	    } ;# end connected device present
	    if { $cCnt > 0 } {
		    UTF::Message INFO $utfmsgtag "Devices disconnected successfully: $dList"
		    return 0
	    } else {
    	    # UTF::Message INFO $utfmsgtag "Devices failed to disconnect: $fList"
		    return 0xff
	    } 
	};# end disconnectDevice

	# generic lookup by BD address or type
	method lookupDevice {args} {;# expecting lookup type, followed by address/device type
		set lList ""
		set dList ""
		set ltype ""
		set devBD ""
		
		set ltype [lindex $args 0]
		set devBD [lindex $args 1]
		
		if { $ltype eq "paired" } {
			if { $pDevsExist eq 0 } { ;# case no connected device found
			    error $utfmsgtag "No paired device found."
		    } else {
				# compile list of connected devices for look up
	    		if { [regexp {.*([a-f,A-F,0-9,-]+).*} $devBD match] } {
					# UTF::Message DEBUG $utfmsgtag "Matched pattern: $match"
					set dList [$self lookupDeviceByAddress $ltype $devBD]
				} else { ;# case where regexp returned no match
					error $utfmsgtag "Should not be here. Check that it should be \"lookupDevice paired \[device\]\""
				}
			}
		} else {
			if { $cDevsExist eq 0 } { ;# case no connected device found
			    error $utfmsgtag "No connected device found."
		    } else {
				# compile list of connected devices for look up
				if { [regexp {.*([a-f,A-F,0-9,-]+).*} $devBD match] } {
					# UTF::Message DEBUG $utfmsgtag "Matched pattern: $match"
					set dList [$self lookupDeviceByAddress $ltype $devBD]
				} else { ;# case where regexp returned no match
					error $utfmsgtag "Should not be here. Check that it should be \"lookupDevice connected \[device\]\""
				}
		    }
		}
		if { $dList eq "" || $dList eq 0xff } {
			UTF::Message FAIL $utfmsgtag "No $ltype device $devBD found."
		} else {
			UTF::Message INFO $utfmsgtag "Device found: $dList"
		}
		return $dList
	};# end lookupDevice

	# lookup device(s) by BD address
	method lookupDeviceByAddress {args} {
		set cFound 0
		set pList ""
		set dType ""
		set dList ""
		set devRslts ""
		set ltype ""
		set devBD ""
		
		set ltype [lindex $args 0]
		set devBD [lindex $args 1]
		
		if { $ltype eq "paired" } {
			set flag pDevsExist
			set list pDevList
			set msgtag "No paired device found."
	    } else {
		    set flag cDevsExist
		    set list cDevList
		    set msgtag "No connected device found."
	    }

		if { [set $flag] eq 0 } {
		    error $utfmsgtag $msgtag
	    } else {
			foreach dev $devBD { ;#BD address given
				# going through each given device
				# UTF::Message DEBUG $utfmsgtag "Device looked up: $dev in $devBD"
				if { [regexp -nocase $dev [set $list] ] } {
					set dType [ $self getDeviceType $dev ]
					# UTF::Message DEBUG $utfmsgtag "dType returned from deviceType call on dev $dev: $dType"
					if { $dType eq 0xff } { ;# case failing to determine device type
						UTF::Message FAIL $utfmsgtag "Failed to determine device type: $dev"
					} else { ;# everything else
						if { $dType ne "Unknown" } {
							#UTF::Message DEBUG $utfmsgtag "Device found at address: $dev ; Type: $dType"
							append dList "$dev $dType "
							#UTF::Message DEBUG $utfmsgtag "dList: $dList\n"
							incr cFound
						} else {
							UTF::Message INFO $utfmsgtag "Trying to match dev: $dev with typeName: $dType"
							#puts "Calling lookup method lookupDeviceByType $ltype $dev\n"
							set devRslts [$self lookupDeviceByType $ltype $dev]
							# UTF::Message DEBUG $utfmsgtag "Result from lookup by type $dev: $devRslts"
							if { $devRslts ne 0xff } {
								UTF::Message INFO $utfmsgtag "$dev found at $devRslts"
								append dList "$devRslts$dev "
								incr cFound 
							}
						}
					}
				} else {
					UTF::Message FAIL $utfmsgtag "Device not $ltype."
				}
			} ;# end foreach loop
		}
		if { $cFound ne 0 } {
			# UTF::Message DEBUG $utfmsgtag "dList to be returned from ::lookupConnectedDeviceByAddress::\n $dList\n"
			UTF::Message INFO $utfmsgtag "Device $dType found at address:$dev"
			return $dList
		} else {
			return 0xff
			# error $utfmsgtag "Device not $ltype. Check device $devBD."
		}
	};# end lookupDeviceByAddress

	# lookup connected device(s) by type
	method lookupDeviceByType {args} {
		set pList ""
		set ltype [lindex $args 0]
		set cDev [lindex $args 1]
		
		if { $ltype eq "paired" } {
			set flag pDevsExist
			set list pDevList
			set msgtag "No paired device found."
	    } else {
		    set flag cDevsExist
		    set list cDevList
		    set msgtag "No connected device found."
	    }
		
		string tolower "$cDev"
		# UTF::Message INFO $utfmsgtag "Calling lookupDevBD with cDev $cDev"
		set dList [$self lookupDevBD $ltype $cDev]
		if { $dList ne 0xff } {
			UTF::Message INFO $utfmsgtag "Device $cDev found at address: $dList"
			return $dList
		}
	};# end lookupDeviceByType

	# routine that performs look up of device matching by type
	method lookupDevBD {args} {
		set dFound 0
		set fList ""
		set ltype ""
		set dev ""
		set flag ""
		set list ""
		set msgtag ""
		
		set ltype [lindex $args 0]
		set dev [lindex $args 1]
		
		if { $ltype eq "paired" } {
			set flag pDevsExist
			set list pDevList
			set msgtag "No paired device found."
	    } else {
		    set flag cDevsExist
		    set list cDevList
		    set msgtag "No connected device found."
	    }
		
		if { [set $flag] eq 0 } {
		    error $utfmsgtag $msgtag
	    } else {
			# UTF::Message INFO $utfmsgtag "cList from lookupDevBD: $cList"
			set list [set $list]
			foreach "cBD cType" $list {
				if { [regexp -nocase $dev $cType] } {
					# UTF::Message INFO utfmsgtag "lookupDevBD $dev device found at address: $cBD"
					append fList "$cBD "
					incr dFound
				}
			}
		}
		if { $dFound eq 0 } { 
			error $utfmsgtag "No $dev found"
		} else {
			UTF::Message INFO $utfmsgtag "Device $dev found at $fList"
			return $fList
		}
	};# end lookupDevBD

    ### maybe redundent with new disconnectDevice method
    method disconnectAllDevices {} {
	    set dDevs ""
	    
	    UTF::Message INFO $utfmsgtag "Disconnecting all devices..."
		set dDevs [$options(-sta) $options(-btpath)/applebt --disconnectAll]
		if { $dDevs ne "" } {
			error $utfmsgtag "Failed to disconnect devices."
			return 0xff
		} else {
			UTF::Message INFO $utfmsgtag "All devices disconnected."
			set $cDevList "" ; set $cDevsExist 0
			return 0
		}
	};# end disconnectAllDevices
    
    method findActiveiTunes {} {
	    set iPid ""
	    set rPid ""
	    
	    UTF::Message INFO $utfmsgtag "Checking active iTunes process..."
		set iPid [$options(-sta) $options(-btpath)/applebt --checkPID iTunes]
		if { ! [regexp -nocase {pid:\s(.*)\s} $iPid - rPid] } {
			UTF::Message INFO $utfmsgtag "iTunes is not running."
			return 0
		} else {
			return $rPid
		}
	};# end findActiveiTunes

    method checkAudioDevices {} {
	    set aDev ""
	    set aList ""
	    set aBD ""
	    set aType ""
	    
	    UTF::Message INFO $utfmsgtag "Checking configured audio device..."
	    set aDev [$options(-sta) $options(-btpath)/applebt --listConfiguredAudioDevices]
		if { ! [regexp -nocase {.*address:\s([0-9,a-f,A-F,-]+).*Type:\s(.*)\s} $aDev - aBD aType] } {
		    error $utfmsgtag "No audio device found."
		} else {
		    UTF::Message INFO $utfmsgtag "Audio device found. Address: $aBD; type: $aType."
		    append aList "$aBD $aType "
		    return $aList
		}
	};# end checkAudioDevices

	# acceptable name (no quotes): /Users/user/Music/iTunes/iTunes\\\ Media/Music/Unknown\\\ Artist/Unknown\\\ Album/rxWave.wav
    method startiTunesTraffic { {fName ""} } {
	    set iCode ""
	    set iState ""
	    set iMsg ""
	    set tCnt 0
		set {*}iFile [file join $options(-itpath) $fName]
		# UTF::Message DEBUG $utfmsgtag "fName full path: $iFile"
	
		while { $iState ne "PLAYING" } {
			catch {$options(-sta) sudo $options(-btpath)/applebt --playFileIniTunes $iFile} sMsg ;# user = root
			UTF::Message DEBUG "" "catch message returned:\n$sMsg"
			if { [regexp -nocase {.*execution error.*} $sMsg] } {
				UTF::Message FAIL "User root:" "Cannot start iTunes."
				UTF::Message DEBUG "User root:" "$sMsg"
				UTF::Message INFO "" "Try starting iTunes using another user account."
				catch {$options(-sta) sudo -u user $options(-btpath)/applebt --playFileIniTunes $iFile} iMsg ;# user = user 
				if { [regexp -nocase {.*execution error} $iMsg] } {
					UTF::Sleep 3
					incr tCnt
					if { $tCnt <= 3 } {
						continue
					} else { 
						UTF::Message FAIL $utfmsgtag "Exhausted retry."
						set iState "FAIL_TO_PLAY"
						break
					}
				} else {
					set iState "PLAYING"
					break
				}
			} else {
				catch {$self findActiveiTunes} pID
				if { $pID ne 0 } {
					UTF::Message INFO "User root:" "iTunes started."
					set iState "PLAYING"
				} else {
					UTF::Message INFO "User root:" "iTunes process not found."
					set iState "FAIL_TO_PLAYING"
				}
			}
		}
		return $iState
	};# end startiTunesTraffic

    method stopiTunesTraffic {} {
	    set rPid ""
	    
	    # Checking active iTunes process
	    #set rPid [$self findActiveiTunes]
	    catch { $self findActiveiTunes } rPid
	    if { $rPid eq 0 } { ;#do nothing when no active iTune process is found
		    UTF::Message INFO $utfmsgtag "iTunes is not running." 
		} else {
		    # kill running iTunes process found
			UTF::Message INFO $utfmsgtag "iTunes is running: $rPid; stopping active iTunes session..."
			if { [$options(-sta) rexec kill -9 $rPid] ne "" } {
			    error $utfmsgtag "Error stopping iTunes session."
				return 1
			} else {
				UTF::Message INFO $utfmsgtag "iTunes stopped."
				return 0
			}
		}
	};# end stopiTunesTraffic
	
# # #     method checkA2DPBitRate {{rTime 10}} { ;# old code
# # # 	    set aDev ""
# # # 	    set aRate ""
# # # 	    
# # # 	    # UTF::Message INFO $utfmsgtag "Checking if active audio device present..."
# # # 		set aDev [$options(-sta) sudo $options(-btpath)/applebt --isActiveAudioDevice]
# # # 	    if { [regexp -nocase {(inactive audio)} $aDev] } {
# # # # 		    error $utfmsgtag "No active audio devices present."
# # # 			UTF::Message FAIL $utfmsgtag "No active audio devices present."
# # # 		} else {
# # # 			UTF::Message INFO $utfmsgtag $aDev
# # # 			# proceed with A2DPBitRate only with audio device active
# # # 			UTF::Message INFO $utfmsgtag "Checking A2DP traffic bit rate..."
# # # 			set aRate [$options(-sta) sudo $options(-btpath)/applebt --A2DPBitRate $rTime]
# # # 			return $aRate
# # # 		}
# # # 	};# end checkA2DPBitRate

    method checkA2DPBitRate { dev {rTime 10}} { ;# new code: run test in background and return results
	    set aDev ""
	    set aRate ""
	    
	    # UTF::Message INFO $utfmsgtag "Checking if active audio device present..."
		set aDev [$options(-sta) sudo $options(-btpath)/applebt --isActiveAudioDevice]
	    if { [regexp -nocase {(inactive audio)} $aDev] } {
# 		    error $utfmsgtag "No active audio devices present."
			UTF::Message FAIL $utfmsgtag "No active audio devices present."
		} else {
			UTF::Message INFO $utfmsgtag $aDev
			# proceed with A2DPBitRate only with audio device active
			UTF::Message INFO $utfmsgtag "Checking A2DP traffic bit rate..."
# 			set aRate [$options(-sta) sudo $options(-btpath)/applebt --A2DPBitRate $rTime]
# 			return $aRate
			catch { $self getTestFD $dev $rTime } fd
			catch { $self getFDResults $fd } aRate
			return $aRate
		}
	};# end checkA2DPBitRate
	
    method startHIDTraffic {{tTime 5}} {
		# check if HID device(s) present
		# start traffic only with HID devices present
		set aHid ""
		set hRslts ""
		
		# check if HID device(s) present
		set aHid [$self checkActiveHIDDevices]
		if { $aHid eq 0xff } {
		    error $utfmsgtag "No active HID device present."
		} else {
			UTF::Message INFO $utfmsgtag "$aHid HID device(s) present. Starting HID packet error rate traffic..."
			set hRslts [$options(-sta) sudo $options(-btpath)/applebt --HIDPacketErrorRate $tTime]
			UTF::Message INFO $utfmsgtag "HID Packet Error Rate results: \n$hRslts"
			return $hRslts
		}
	};# end startHIDTraffic

    method checkActiveHIDDevices {} {
		# Look for number of active HID devices
		set aHid ""
		
	    # UTF::Message INFO $utfmsgtag "Checking for active HID devices..."
		set aHid [$options(-sta) $options(-btpath)/applebt --HIDActive]
	    if { $aHid eq 0 } {
		    UTF::Message FAIL $utfmsgtag "No active HID devices present."
			return 0xff
		} else {
			UTF::Message INFO $utfmsgtag "$aHid HID device(s) found."
		}
		return $aHid
	};# end checkActiveHIDDevices

    method powerOffBTRadio {} {
	    set pState ""
	    set oState ""
	    
	    # look up BT radio power state
		set pState [$self checkBTRadioPowerState]
	    if { [regexp -nocase {.*\sturned off} $pState] } {
		    error $utfmsgtag "BT radio is already off."
			return 0xff
		} else {
			UTF::Message INFO $utfmsgtag "Turning off BT radio."
			set oState [$options(-sta) $options(-btpath)/applebt --setPowerState 0]
			if { ! [regexp -nocase {.*\sturned off} $oState] } {
			    error $utfmsgtag "Check system. BT radio cannot be turned off."
			} else {
				catch { $self checkBTRadioPowerState } pState
				#puts "checkBTRadioPowerState returned message:\n$pState"
				if { [regexp -nocase {.*turned off.*} $pState] || [regexp -nocase {.*already off.*} $pState] } {
					UTF::Message INFO $utfmsgtag "BT radio off."
					return 0
				} else {
					UTF::Message INFO $utfmsgtag "BT radio failed to trun off."
					return 0xff
				}
			}
		}
	};# end powerOffBTRadio

    method powerOnBTRadio {} {
	    set pState ""
	    set oState ""
	    
	    # look up BT radio power state
		set pState [$self checkBTRadioPowerState]
	    if { [regexp -nocase {.*\sturned on} $pState] } {
		    error $utfmsgtag "BT radio is already on."
		} else {
			UTF::Message INFO $utfmsgtag "Turning on BT radio."
			set oState [$options(-sta) $options(-btpath)/applebt --setPowerState 1]
			UTF::Sleep 3
			if { ! [regexp -nocase {.*\sturned on} $oState] } {
			    error $utfmsgtag "Check system. BT radio cannot be turned on."
			} else {
				catch { $self checkBTRadioPowerState } pState
				#puts "checkBTRadioPowerState returned message:\n$pState"
				if { [regexp -nocase {.*turned on.*} $pState] || [regexp -nocase {.*already on.*} $pState] } {
					UTF::Message INFO $utfmsgtag "BT radio on."
					return 0
				} else {
					UTF::Message INFO $utfmsgtag "BT radio failed to turn on."
					return 0xff
				}
			}
		}
	};# end powerOnBTRadio

    method checkBTRadioPowerState {} {
	    set pState ""
	    
	    UTF::Message INFO $utfmsgtag "Checking BT radio power state..."
		set pState [$options(-sta) $options(-btpath)/applebt --getPowerState]
		if { ! [regexp -nocase {.*not available} $pState] } {
			return $pState
		} else {
			error $utfmsgtag "No Bluetooth host device found."
		}
	};# end checkBTRadioPowerState

    method getDeviceType {devBD} {
		set dType ""
		set typeName ""
		
		set dType [$options(-sta) $options(-btpath)/applebt --deviceType $devBD]
		if { [regexp {Device Type:\s(.*)} $dType - typeName] } {
			# UTF::Message DEBUG $utfmsgtag "getDeviceType returns value: $typeName; given value: $devBD"
			return $typeName
		} else {
			UTF::Message INFO $utfmsgtag "Failed to determine device type for: $devBD"
			return 0xff
		}
	};# end getDeviceType
	
	method isReady {dev} { ;# new code 3/22/16: did not make assumption of battery support based on device type
		set badFlg 0
		set btStatus ""
		set cState ""
		set ltype ""
		set deAddr ""
		set batLvl ""
		
		if { $dev eq $options(-sta) } { ;# case where device is host controller at STA
			set btStatus [$options(-sta) $options(-btpath)/applebt --bluetoothAvailable]
			#puts "btStatus value: $btStatus\n"
			if { [regexp -nocase {.*good state} $btStatus] } {
				return HOST_READY
			} else {
				set badFlg 1; return HOST_NOT_READY
			}
		} else { ;#other devices
			#list devices only when necessary
			if { $pDevList eq "" || $pDevsExist ne 1 } {
				catch { $self listDevices paired } pDevMsg
				UTF::Message DEBUG "" "Message returned from listDevices paired:\n$pDevMsg"
			}
			set ltype "paired"
			catch  { $self lookupDevice $ltype $dev } devAddr; #puts "devAddr found at: $devAddr"
			if { $devAddr eq 0xff } { 
				error $utfmsgtag "Device profile unknown."
			} else {
				if { [lindex $devAddr 1] eq $dev || [lindex $devAddr 0] eq $dev } { 
					#puts "dev found is: $dev; devAddr1: [lindex $devAddr 1]"
					catch { $self getDevConnectionStatus $dev } cState
					UTF::Message DEBUG $dev "Return message from getDevConnectionStatus:\n$cState"
					#puts "cState returned: $cState"
					if { [regexp -nocase {.*not_} $cState] } { 
						UTF::Message FAIL $dev "Device may not be ready."
						return $cState
					}
				}
			}
			#puts "badFlg value: $badFlg"
			#puts "devAddr found is: $devAddr"
			if { $badFlg ne 1 } { ;# skipped if device not connected or bad
				# check battery
				catch { $self getBatteryLevel $devAddr } batLvl
				UTF::Message DEBUG "" "batLvl returned from getBatteryLevel:\n$batLvl"
				#puts "batLvl is: $batLvl"
				if { [regexp -nocase {.*not\_support.*} $batLvl] } { ;# where battery level supported
					UTF::Message FAIL $utfmsgtag "No battery info available for [lindex $devAddr 1]."
				} else {
					if {$batLvl eq 0xff} {
						UTF::Message WARNING $utfmsgtag "Problem getting battery information. Check device [lindex $devAddr 1].\n"
						#set badFlg 1
						return NOT_READY
					} else {
						if {$batLvl > 50} { 
							UTF::Message INFO $utfmsgtag "Battery is strong.\n"
							}
						if {$batLvl <= 50 && $batLvl >= 10} { 
							UTF::Message INFO $utfmsgtag "Battery is OK.\n"
							}
						if {$batLvl < 10} { 
							UTF::Message WARNING $utfmsgtag "Battery is getting weak. Consider changing.\n"
							#set badFlg 1
							return NOT_READY
							}
					}
				}
				return READY
			} else {
				# return NOT_READY
				return $cState
			}
		};# end other devices
	};# end isReady 3/22/16 new code

	method startTraffic { dev {arg ""} } {
		set status ""
		set rdyState [$self isReady $dev]
		#puts "rdyState: $rdyState\n"

		if { [regexp -nocase {.*not_} $rdyState] } { 
			error $utfmsgtag "Device $dev not in ready state. Check $dev.\n" 
		}
		if { ! [regexp -nocase {host.*} $rdyState] } {
			if { [regexp -nocase $dev $hidDevs] } {
				#puts "Run HIDPacketErrorRate"
				set rtime $arg
				set hRslts [ $self startHIDTraffic $rtime ]; return $hRslts
			} else {
				if { [regexp -nocase $dev $audDevs] } {
					#puts "Run A2DPBitRate"
					###set aRslts [ $self startiTunesTraffic $rtime ] ; return $aRslts ;# try new __startiTunesTraffic
					set fname $arg
					set aRslts [ $self __startiTunesTraffic $fname ] ; return $aRslts
				} else { 
					UTF::Message ERROR $utfmsgtag "Unkown device or unknown error."
					set status UNKNOWN_OR_ERROR
				}
				set status RUNNING
			}
		} else {
				#puts "Host controller. No traffic tests."
				UTF::Message INFO $utfmsgtag "Host controller. No traffic tests."
				set status NOT_RUNNING
		}
	};# end startTraffic
	
	method stopTraffic {dev} {
		if { [$self isReady $dev] ne "NOT_READY" } {
			if { [regexp -nocase $dev $hidDevs] } {
				#puts "Stop HIDPacketErrorRate (what to do?)"
			} else {
				if { [regexp -nocase $dev $audDevs] } {
					$self stopiTunesTraffic
				} else { 
					UTF::Message DEBUG $utfmsgtag "stopTraffic: Should not be here."
				}
			}
			return STOPPED
		} else {
			return CANNOT_STOP
		}
	};# end stopTraffic
	
	method getBatteryLevel {devAddr} {
		# generalized test for battery support -- no longer assumed support based on device genre

		set batLvl ""
		$self listDevices connected
		if { [regexp [lindex $devAddr 0] $cDevList] && $cDevsExist ne 0 } {
			catch {[$options(-sta) $options(-btpath)/applebt --HIDGetBatteryLevel [lindex $devAddr 0]]} batIndex
			#UTF::Message DEBUG getBatteryLevel "******************************************\nbatIndex returned: $batIndex"
			set lines [split $batIndex "\n"]
			foreach line $lines {
				#puts "Incoming line:\n$line"
				if { [regexp -nocase {.*usage:.*} $line] || [regexp -nocase {.*not support.*} $line] } {
					set batLvl "NOT_SUPPORTED"
				} else {
					regexp {.*\:\s(.*)\%} $batIndex - batLvl
					#UTF::Message DEBUG getBatteryLevel "batIndex: $batIndex; batLvl: $batLvl\n"
				}
					
		}
			#UTF::Message DEBUG getBatteryLevel "resulting batLvl value:\n$batLvl"
			return $batLvl
		} else {
			UTF::Message FAIL getBatteryLevel "Device [lindex $devAddr 1] is not connected."
			return 0xff
		}
	};# end getBatteryLevel
	
	method getRSSI {devAddr} {
		set btCmd "$options(-sta) $options(-btpath)/applebt"
		append btCmd " --deviceReadRawRSSI"
		set devRSSI [{*}$btCmd [lindex $devAddr 0]]
		regexp {.*RSSI:\s(.*)\s} $devRSSI - sigLvl
		#puts "sigLvl: $sigLvl\n"
		return $sigLvl
	};# end getRSSI
	
	method getDevClass {dev} {
		# sta: host controller
		# HID: keyboard, mouse
		# Audio: speaker, headset
		if { $dev eq $options(-sta) } {
			return HOST
		} else {		
			if { [regexp -nocase $dev $hidDevs] } {
				#puts "$dev is a HID"
				return HID 
			} else {
				#puts "$dev is an Aud"
				return AUDIO
			}
		}
	};# end getDevClass
	
	method queryDevice {dev} {
		# check host controller
		# get device class
		# get device type
		# get device BD address
		# readiness check
		# get traffic status
		# get device raw RSSI
		
		set QUERY_RESULTS ""
		set hState [$self isReady $dev]
		
		if { $dev eq $options(-sta) } { ;# case where device is host controller at STA
			set hostType STA
			set hostType "Device is: $hostType"
			set hState "Device state is: $hState"
			append QUERY_RESULTS "$hostType\n$hState"
		} else { ;# case other devices
			set dPair [$self lookupDevice paired $dev]
			#puts "dPair returned: $dPair\n"
			set dClass [$self getDevClass [lindex $dPair 1]]
			#puts "dClass: $dClass"
			set dType [$self getDeviceType [lindex $dPair 0]]
			#puts "dType: $dType"
			set cState [$self getDevConnectionStatus [lindex $dPair 1]]
			#puts "cState: $cState"
			if { ! [regexp -nocase {.*NOT_CONNECTED} $cState] } {
				set bLvl [$self getBatteryLevel $dPair]
				#puts "bLvl: $bLvl"
				set rLvl [$self getRSSI $dPair]
				#puts "rLvl: $rLvl"
			} else { 
				set bLvl "UNKNOWN" ; set rLvl "UNKNOWN"
			}
			
			set cState "Connection state: $cState"
			set dClass "Device class: $dClass"
			set dType "Device type: $dType"
			set dAddr "Device address: [lindex $dPair 0]"
			set rLvl "Raw RSSI: $rLvl"
			set bLvl "Battery Level: $bLvl"
			set hState "Device readiness: $hState"
			append QUERY_RESULTS "###############\n$dClass\n$dType\n$dAddr\n$bLvl\n$cState\n$rLvl\n$hState"
		}
		#puts "Outgoing QUERY_RESULTS: ${QUERY_RESULTS}\n"
		return $QUERY_RESULTS
	};# end queryDevice
	
	method getDevConnectionStatus {dev} {
		set conStatus ""
		set pState ""
		set cState ""
		
		if { $pDevsExist ne 1 || $pDevList eq "" } { ;# check if paired devices exist
			$self listDevices ;# refresh lists
		}
		if { [regexp -nocase $dev $pDevList] } {
			set pState "PAIRED"
		} else { ;# recheck case device not already listed
			set pList [$self listDevices paired]
			if  { ! [regexp -nocase {$dev} $pList } {
				set pState "NOT_PAIRED"
			} else {
				set pState "PAIRED"
			}
		}

		if { [regexp -nocase $dev $cDevList] && $cDevList ne "" } {
			set cState "CONNECTED"
		} else { ;# recheck case device not already listed
			set cList [$self listDevices connected]
			if { ! [regexp -nocase {$dev} $cList] } {
				set cState "NOT_CONNECTED"
			} else {
				set cState "CONNECTED"
			}
		}
		
		append conStatus "$pState & $cState"
		return $conStatus
	};# end getDevConnectionStatus

	method discoverDevice { {dev ""} } {
		set btCmd "$options(-sta) $options(-btpath)/applebt"
		append btCmd " --deviceStartDiscovery"
		set disList [{*}$btCmd]
		#puts "List of devices discovered: $disList\n"
	};# end discoverDevice
	
	method pairDevice { {dev ""} } {
		set pStatus ""
		
		set btCmd "$options(-sta) $options(-btpath)/applebt"
		append btCmd " --pair"
		
		set pStatus [{*}$btCmd $dev]
		#puts "pStatus is: $pStatus\n"
		return $pStatus
	};# end pairDevice
	
	method isPaired { dev } {
		set cmdRslt ""
		
		set btCmd "$options(-sta) $options(-btpath)/applebt"
    	if { [regexp -nocase {(\d[a-f,A-F,0-9,-]+).*} $dev match] } {;# case BD address
			UTF::Message DEBUG $utfmsgtag "Matched pattern: $match"
		} else {;# case BD name
			set dev [lindex [$self lookupDevice paired $dev] 0]
			if { $dev eq "" || $dev eq 0xff } {
				error $utfmsgtag "Device profile unknown. Abort."
			}
		}
		append btCmd " --isPaired $dev"
		set cmdRslt [{*}$btCmd] ;#puts "cmdRslt: $cmdRslt\n"
		return $cmdRslt
	}
		
	method removeDeviceProfile { {dev ""} } {
		set rmStatus ""
		set btCmd "$options(-sta) $options(-btpath)/applebt"
		
		if { $dev eq "" } {;# case no device given
			append btCmd " --removeAllDevices"
		} else {;# case device given
    		if { [regexp -nocase {(\d[a-f,A-F,0-9,-]+).*} $dev match] } {;# case BD address
				UTF::Message DEBUG $utfmsgtag "Matched pattern: $match"
			} else {;# case BD name
				set dev [lindex [$self lookupDevice paired $dev] 0]
				#puts "dev found: $dev"
				if { $dev eq "" || $dev eq 0xff } {
					error $utfmsgtag "Device profile unknown. Abort."
				}
			}
		append btCmd " --removeDevice $dev"
		}
		set rmStatus [{*}$btCmd]
		#puts "rmStatus: $rmStatus"
	} ;# end removeDeviceProfile
	
    method checkHIDDevice { {hDev ""} } {
		# Look for number of active HID devices
		set hList ""
		set rCnt 0
		
		if { $hDev eq "" } {;# case check all active HID device
			set devList $hidDevs
			#puts "devList is: $devList"
		} else {
			set devList $hDev 
			#puts "devList is: $devList"
		}
		foreach dev $devList {
			set dRdy [$self isReady $dev]
			#puts "dReady returned: $dRdy\n"
			if { $dRdy eq "NOT_READY" || [regexp -nocase {.*not_.*} $dRdy] } {
				UTF::Message FAIL $dev "HID device $dev not ready"
				# incr fCnt
			} else {
				incr rCnt
				#puts "HID device $dev is ready"
				UTF::Message INFO $utfmsgtag "HID device $dev is ready"
				append hList "$dev "
			}
		};# end of foreach hidDevs 

	    if { $rCnt eq 0 } {
		    UTF::Message FAIL $utfmsgtag "No active HID device is ready to run tests."
			return 0xff
		} else {
			UTF::Message INFO $utfmsgtag "HID device(s) ready to run tests: $hList"
			return $hList
		}
	};# end checkHIDDevice

	method findOtherHIDDevice { dev } {
		# new code: stop at checking if other device present: leave off checking readiness
		set fCnt 0
		set dFound ""
		foreach d $hidDevs {
			if { [string trim "$d"] == [string trim "$dev"] } {
				# do nothing
				#puts "d is:$d; dev is: $dev\n"
			} else {
				incr fCnt
				#puts "The other device found: $d\n"
				set dFound $d
			}
		}
		if { $fCnt eq 0 } { 
			UTF::Message WARNING $utfmsgtag "No other active HID device."
			return 0xff
		} else {
			return $dFound
		}
	}
	
	method connectOnlyOneHIDDevice { dev } {
		set oHDev [ $self findOtherHIDDevice $dev ]
		#puts "Other HID Device value: $oHDev"

		catch { $self checkHIDDevice } hDev ;# list all active HID devices

		if { $oHDev ne 0xff } {
			set aHDev [ $self checkActiveHIDDevices ]
			#puts "aHDev value returned: $aHDev"
			if { $aHDev eq 1 && [string trim "$hDev"] == [string trim "$dev"] } { ;# case only the target device active
				UTF::Message INFO $utfmsgtag "There is no other active HID device."
				return 0
			} else { ;# case other device active: disconnect other device
					UTF::Message INFO $dev "Disconnecting the other HID Device: $oHDev."
					set disCon [ $self disconnectDevice $oHDev ]
					#puts "Diconnect $oHDev return value disCon: $disCon"
					if { $disCon ne 0 } { ;# case disconnect failed
						UTF::Message FAIL $oHDev "Device $oHDev failed to disconnect."
						return 0xff
					} else { ; # now check if intended device active
						set cHID [$self checkHIDDevice $dev]
						#puts "cHID returned value:\n$cHID\n"
						if { $cHID eq "NOT_READY" || $cHID eq 0xff } {
							UTF::Message FAIL $utfmsgtag "Device $dev disconnected or NOT_READY."
							return 0xff
						} else {
							return 0 ;# case device itself is ready
						}
					}
			} ;# end case other device active
		} else {
			UTF::Message FAIL $dev "No active HID Device found. Cannot proceed."
		}
	}
	
    method checkBattery {dev} {
	    if { $pDevList eq "" || $pDevExist ne 1 } { ;# list devices only if no knowledge of any profile
	    	catch { $self listDevices paired } pListMsg
    	}
    	if { $pListMsg ne 0xff || $pListMsg ne "" } { ;#proceed only if paired devices present
	        UTF::Message DEBUG $utfmsgtag "Paired devices found:\n$pListMsg"
	        UTF::Message INFO $utfmsgtag "Looking for device: $dev"
		    set devAddrName [$self lookupDevice paired $dev] 
		    #UTF::Message DEBUG $utfmsgtag "devAddrName returned: $devAddrName"
		    if { $devAddrName eq 0xff } {
			    #UTF::Message FAIL $utfmsgtag "Device not paired."
				return $devAddrName
		    } else {
			    if { [regexp -nocase {.*not_connected.*} [$self getDevConnectionStatus $dev]] } {
				    UTF::Message INFO "" "$dev not connected. Connect prior to proceeding further."
				    $self connectDevice $dev
				}
		    }
	    $self getBatteryLevel $devAddrName
       	} else {
	        UTF::Message FAIL $utfmsgtag "Cannot proceed. No paired devices found:\n$pListMsg"
	        return 0xff
    	}
    }	
	
    method getReady {dev} {
		set myReady [$self isReady $dev]
		UTF::Message DEBUG $dev "My readiness is: $myReady"
		if { [regexp -nocase {.*not_.*} $myReady] } {
			UTF::Message FAIL "" "Device $dev is not ready."
			set cStatus [$self connectDevice $dev]
			#UTF::Message DEBUG $myname "cStatus: $cStatus"
			set myReady [$self isReady $dev] ;#puts "myReady is: $myReady"
			if { ! [regexp -nocase {.*not_.*} $myReady] } {
				UTF::Message INFO "" "Device $dev is now ready."
			}
		}
		return $myReady
	}
	
	method pairDev {addr} {
		if { $addr ne "" } {
			if { [ $self isPaired $addr ] eq "Not Paired" } {
				UTF::Message INFO "" "Pairing device with address: $addr"
				# set rtnCode [ $self pairDevice $addr ]
				catch { $self pairDevice $addr } rtnCode
				UTF::Message INFO "" "rtnCode returned from pairDevice:\n$rtnCode"
			} else {
				#Device already paired. Do nothing,
				UTF::Message INFO "" "Device already paired."
				set rtnCode 1
			}
		} else { ;# case address unknown
			UTF::Message ERROR "" "Device address is required to proceed.\n"
			set rtnCode 0xff
		}
		if { $rtnCode ne "" && $rtnCode ne 1 } {
			UTF::Message FAIL "" "Pairing device with address: $addr may have failed."
		}
		return $rtnCode
	}
	
	method unpairDev {addr} {
		if { $addr ne "" } {
			if { [ $self isPaired $addr ] eq "Not Paired" } {
				UTF::Message INFO "" "Device already unpaired."
				set rtnCode 1
			} else {
				#Device already paired. Do nothing,
				set rtnCode [ $self removeDeviceProfile $addr ]
			}
		} else { ;# case address unknown
			UTF::Message ERROR "" "Device address is required to proceed.\n"
			set rtnCode 0xff
		}
		if { $rtnCode ne "" && $rtnCode ne 1 } {
			UTF::Message FAIL "" "Unpairing device with address: $addr may have failed."
		}
		return $rtnCode
	}
	
    method getTestFD { dev {rtime ""} } {
# 	    set test ""
	    if { $rtime eq "" } { set rtime 10 }
		set timeout_sec [expr $rtime + 10];#puts "timeout_sec: $timeout_sec"
	    set tCmd "[$self cget -sta] rexec -c -async -t $timeout_sec pty [$self cget -btpath]/applebt"
	    
		if { ! [regexp -nocase {host.*} [$self getDevClass $dev] ] } {
			if { [regexp -nocase $dev $hidDevs] } {
				#puts "Run HIDPacketErrorRate"
				#set fd_mytest [[$self cget -sta] rexec -c -async -t $timeout_sec pty [$self cget -btpath]/applebt --HIDPacketErrorRate $rtime]
				append tCmd " --HIDPacketErrorRate $rtime"
				catch { {*}$tCmd } fd_mytest
			} else {
				if { [regexp -nocase $dev $audDevs] } {
					#puts "Run A2DPBitRate"
					#set fd_mytest [[$self cget -sta] rexec -c -async -t $timeout_sec pty [$self cget -btpath]/applebt --A2DPBitRate $rtime]
					append tCmd " --A2DPBitRate $rtime"
					catch { {*}$tCmd } fd_mytest
				} else { 
					UTF::Message ERROR $utfmsgtag "Unkown device or unknown error."
				}
			}
		} else {
			#puts "Host controller. No traffic tests."
			UTF::Message INFO $utfmsgtag "Host controller. No traffic tests."
		}
		return $fd_mytest
	}
    
    method getFDResults {fd} {
	    if { $fd != "" } {
			#puts "Reading content from fd $fd"
			#set results "Results start here:\n"
	    	append results [$fd close]
	    	if { ! [regexp -nocase {.*} $results] } {
		    	UTF::Message FAIL "" "No results recorded."
				return 0xff
			} 
		} else {
		    	UTF::Message FAIL "" "No file descriptor found."
				#puts "No restults." 
				return 0xff
		}
		return $results ;# turned off for debugging
	}
	
	method setHRTimeStamps {} {
		catch { set ::UTF::MSTimeStamps 1 } setHR
		if { $setHR != 1 } {
			UTF::Message ERROR $utfmsgtag "Setting MSTimeStamps failed: $setHR."
			return 0
		} else {
			UTF::Message INFO $utfmsgtag "Setting MSTimeStamps succeeded."
			return $setHR
		}
	}
	
	method unsetHRTimeStamps {} {
		catch { unset ::UTF::MSTimeStamps } unsetHR
		if { $unsetHR != "" } {
			UTF::Message ERROR &utfmsgtag "Unsetting MSTimeStamps failed: $unsetHR."
		} else {
			UTF::Message INFO $utfmsgtag "Unsetting MSTimeStamps succeeded."
		}
	}
	
	method __reinit { args } {
		
		UTF::Getopts {
			{device.arg 		"" "Device name"}
			{address.arg		"" "Device address"}
			{sleep.arg 5		"Pause time"}
			{retry				"Do retry at connect failure"}
			{retry_limit.arg	3 "Maximum retry attempts"}
		}
		
		if { $(retry_limit) eq "" } {
			set retry_limit 3
		}
		
		if { $(address) eq ""  && $(device) eq "" } {
			error "No device given. Cannot proceed."
		} else {
			if { [regexp {.*([a-f,A-F,0-9,-]+).*} $(address) match] } {
				#puts "Address given: $(address)"
			} else {
				#puts "Address given: $(address) is invalid."
				error "Invalid device address."
			}
		}

		UTF::Message INFO $(device) "checking if device $(device) profile is known to host."
		catch {$self isPaired $(address)} pMsg
		if { [regexp -nocase {.*not.*} $pMsg] } {
			UTF::Message FAIL $(device) "Device $(device) is not paired: $pMsg"
		} else {
			UTF::Message DEBUG $(device) "Device profile known to host.\n$pMsg"
		}
		
		$self __toggleBTPower

		if { $(sleep)ne "" } {
			UTF::Sleep $(sleep)
		} else {
			UTF::Sleep 5 
		}
		
		UTF::Message INFO $(device) "Check if device $(device) is ready."
		catch {$self isReady $(device)} rdMsg
		UTF::Message DEBUG $(device) "Checking $(device): $rdMsg"
		
		if { ! [regexp -nocase {.*not_.*} $rdMsg] } {
			UTF::Message INFO $(device) "Device $(device) is ready."
			return $rdMsg
		} else { ;# connect check and retry
			UTF::Message WARN $(device) "Device $(device) may not be ready."
			UTF::Message INFO $(device) "Attempting to connect device $(device)."
			catch {$self connectDevice $(device)} conMsg
			UTF::Message DEBUG $(device) "Returned message from connectDevice:\n$conMsg"
				
			if { $conMsg ne 0 } {
				if { $(retry) } {
					set rconMsg ""
					UTF::Message WARN $(device) "First connect unsuccessful. Retrying..."
				
					for { set cnt 1 } { $cnt <= $(retry_limit) } { incr cnt } {
						UTF::Message INFO $(device) "Connection retry attempt $cnt of $(retry_limit)"
						catch { $self connectDevice $(device) } rconMsg
						if { $rconMsg eq 0 } {
							UTF::Message DEBUG $(device) "Return message from connectDevice:\n$rconMsg"
							UTF::Message INFO $(device) "Device $(device) at address $(address):\n Connection successful at retry attempt: $cnt"
							set conMsg $rconMsg
							break
						}
					}
					if { $cnt >= $(retry_limit) } {
						UTF::Message INFO $(device) "Exceeded connection retry limit \($(retry_limit)\)."
						set conMsg 0xff
					}
				} ;#end retry block
			} ;# case conMsg ne 0
		} ;#end connect check

		return $conMsg
	} ;# end of reinit

	method __toggleBTPower {} { ;# always result in BT radio on at end of execution
		set states ""

		catch { $self __getBTRadioPowerState } pState ;# determine current state
		#puts "pState value returned: $pState"
		set states " $pState"
		#puts "Initial states setting: $states"
		if { $pState eq 0 } { ;# alter initial power state
			set states "1"
		} else {
			set states "0 $pState"
		}
		foreach state $states {
			#puts "Intended power setting: $state"
			$self __setBTRadioPowerState $state
		} ;# always end with BT radio on
	}
	
	method __getDevAddress { args } {
		UTF::Getopts {
			{device.arg 	"" "Device name"}
			{address.arg 	"" "Device address"}
		}

		if { $(address) ne ""  && [regexp {.*([a-f,A-F,0-9,-]+).*} $(address) match] } {
			return $(address)
		} else {
			if { $(device) ne "" } {
				if { $pDevList eq "" || $pDevsExist ne 1 } {
					catch { $self listDevices paired } pList
					#puts "Paired list returned from listDevices:\n$pList"
				}
				catch { $self lookupDevBD paired $(device) } bdAddr
				#puts "lookupDeviceByType returned value: $bdAddr"
				if { $bdAddr ne "" || $bdAddr ne 0xff } {
					if { [regexp {.*([a-f,A-F,0-9,-]+).*} $bdAddr match] } {
						#puts "Address returned by lookupDeviceByType: $bdAddr"
						return $bdAddr
					} else {
						UTF::Message ERROR "" "Invalid address."
						return 0xff
					}
				} else {
					UTF::Message FAIL $myname "$myname address cannot be found."
					return 0xff
				}
			} else {
				UTF::Message ERROR "" "Device type unknown. Cannot proceed."
			}
		}
	}
	
    method powerOffBTRadio {} {
	    set pState ""
	    set oState ""
	    
	    # look up BT radio power state
		set pState [$self checkBTRadioPowerState]
	    if { [regexp -nocase {.*\sturned off} $pState] } {
		    error $utfmsgtag "BT radio is already off."
			return 0xff
		} else {
			UTF::Message INFO $utfmsgtag "Turning off BT radio."
			set oState [$options(-sta) $options(-btpath)/applebt --setPowerState 0]
			if { ! [regexp -nocase {.*\sturned off} $oState] } {
			    error $utfmsgtag "Check system. BT radio cannot be turned off."
			} else {
				catch { $self checkBTRadioPowerState } pState
				#puts "checkBTRadioPowerState returned message:\n$pState"
				if { [regexp -nocase {.*turned off.*} $pState] || [regexp -nocase {.*already off.*} $pState] } {
					UTF::Message INFO $utfmsgtag "BT radio off."
					return 0
				} else {
					UTF::Message INFO $utfmsgtag "BT radio failed to trun off."
					return 0xff
				}
			}
		}
	};# end powerOffBTRadio

    method __setBTRadioPowerState { {state 1} } {
	    set pState ""
	    set oState ""
	    
	    # look up BT radio power state
		set pState [$self __getBTRadioPowerState]
		if { $pState eq 0xff } {
			error $utfmsgtag "No Bluetooth host device found."
		}
		if { $pState eq $state } { ;# no action required
			if { $pState eq 1 } {
			    UTF::Message INFO $utfmsgtag "BT radio is already ON. Do nothing."
		    } else {
			    UTF::Message INFO $utfmsgtag "BT radio is already OFF. Do nothing."
			}
		} else { ;# case action required
			switch $state {
				0 { set msgTxt "Turning off BT radio." }
				1 { set msgTxt "Turning on BT radio." }
			}
		UTF::Message INFO $utfmsgtag $msgTxt
		set oState [$options(-sta) $options(-btpath)/applebt --setPowerState $state]
		catch { $self __getBTRadioPowerState } pState ;# check on resulting power state
		#puts "__getBTRadioPowerState returned message:\n$pState"
		if { $pState ne $state } {
			error $utfmsgtag [ append msgTxt { Failed.} ]
			return 0xff
		} else {
			UTF::Message INFO "" [ append msgTxt { Done.} ]
			return $state
		}
		} ;# end action decision block
	};# end powerOnBTRadio

    method __getBTRadioPowerState {} {
	    set pState ""
	    
	    UTF::Message INFO $utfmsgtag "Checking BT radio power state..."
		set pState [$options(-sta) $options(-btpath)/applebt --getPowerState]
		#puts "Actual pState message:\n$pState"
		if { ! [regexp -nocase {.*not available} $pState] } {
			if { [regexp -nocase {.*turned on.*} $pState] } {
				return 1
			} else {
				return 0
			}
			# return $pState
		} else {
			error $utfmsgtag "No Bluetooth host device found."
			return 0xff
		}
	};# end checkBTRadioPowerState

    method __checkAudioDevice {} {
	    set aDev ""
	    
	    # UTF::Message INFO $utfmsgtag "Checking if active audio device present..."
		set aDev [$options(-sta) sudo $options(-btpath)/applebt --isActiveAudioDevice]

	    if { [regexp -nocase {(inactive audio)} $aDev] } {
# 		    error $utfmsgtag "No active audio devices present."
			UTF::Message FAIL $utfmsgtag "No active audio devices present."
			# error "No active audio devices present."
			return 0
		} else {
			if { [regexp -nocase {(a2dp audio)} $aDev] } {
				UTF::Message INFO $utfmsgtag $aDev
				#proceed with A2DPBitRate only with audio device active
				UTF::Message INFO $utfmsgtag "Active audio devices present."
				return 1
			} else {
				UTF::Message ERR $utfmsgtag "Unknown audio device state."
				return 0
			}
		}
	};# end checkAudioDevice
	
    method __startiTunesTraffic { {fName ""} } { ;# new code rewritten 4/15/16 thru 4/18/16
	    set iCode ""
	    set iState ""
	    set iMsg ""
	    set tCnt 0
		set {*}iFile [file join $options(-itpath) $fName]
		set uList "root user"
	
		UTF::Message DEBUG "_startiTunesTraffic" "routine called"
		
		foreach u $uList {
			if { $u == "root" } {
				set iCmd "$options(-sta) sudo $options(-btpath)/applebt --playFileIniTunes $iFile"
			} else {
				set iCmd "$options(-sta) sudo -u user $options(-btpath)/applebt --playFileIniTunes $iFile"
			}
			#puts "Current user: $u\nCommand line:\n$iCmd\n"
			UTF::Message INFO "User $u:" "Starting iTunes."
			#catch { set sMsg [{*}$iCmd] }
			catch {eval {*}$iCmd} sMsg
			#UTF::Message DEBUG "" "catch message returned:\n$sMsg"
			if { ! [regexp -nocase {.*usage:.*} $sMsg] } {
				#puts "no usage error"
				set eFlg 0
			} else {
				UTF::Message FAIL "User $u" "iTunes starting usage error: $sMsg"
				UTF::Message INFO "User $u" "Check iTunes filename."
				#puts "usage error"
				set eFlg 1
			}
			if { ! [regexp -nocase {.*execution error.*} $sMsg] } { ; #puts "No execution error."
					#puts "no execution error"
					set eFlg [expr $eFlg + 0]
			} else {
				UTF::Message INFO "User $u" "iTunes starting error: $sMsg"
				if { [regexp -nocase {.*expected type.*} $sMsg] } {
					UTF::Message INFO "User $u" "Check iTunes filename?"
				}
				#puts "execution error"
				set eFlg [expr $eFlg + 1]
				#puts "eFlg value at start error: $eFlg"
			}
			
			if { $eFlg > 0} {#skip on iTunes errors?
			#do nothing
			} else {
				# check pId here
				UTF::Sleep 3
				set pId [$self findActiveiTunes]; #puts "pId returned: $pId"
				if { [regexp -nocase {.*not running.*} $pId] || $pId eq 0 } {
					#puts "failed starting iTunes."
					UTF::Message FAIL "User $u" "iTunes did not start."
					set iState 0
				}
			}
			
			if { $eFlg > 0 || $iState eq 0 } {

				#puts "failed starting iTunes using account: $u."
				#retry if iTunes pID not available or iTunes start failed
				for { set tCnt 1 } { $tCnt <= 3 } { incr tCnt } {
					UTF::Sleep 3 ;# wait briefly before retry
					set rFlg ""
					set rState ""
					#set fCause ""
					#puts "Retry $tCnt"
					#set sMsg [{*}$iCmd]
					catch {eval {*}$iCmd} sMsg
						
					if { ! [regexp -nocase {.*usage:.*} $sMsg] } {
						#puts "no usage error"
						#UTF::Message INFO "" "iTunes RETRY $tCnt: no usage error."
						set rFlg 0
					} else {
						UTF::Message FAIL "User $u" "iTunes starting usage error:\n$sMsg"
						UTF::Message INFO "" "Check iTunes filename."
						#puts "usage error"
						set rFlg 1
					}
						
					if { ! [regexp -nocase {.*execution error.*} $sMsg] } { 
						#puts "no retry execution error"
						#UTF::Message INFO "" "iTunes RETRY $tCnt: no execution error."
						set rFlg [expr $rFlg + 0]
					} else {
						UTF::Message FAIL "User $u" "iTunes RETRY $tCnt error:\n$sMsg"
						#puts "retry execution error"
						if { [regexp -nocase {.*expected type.*} $sMsg] } {
							UTF::Message INFO "User $u" "Check iTunes filename?"
						}
						set rFlg [expr $rFlg + 1]
					}
					
					if { $rFlg > 0 } {#skip checking pID on execution errors?
					# do nothing 
					} else {
						UTF::Sleep 3
						
						set pId [$self findActiveiTunes]; #puts "pId returned: $pId"
						if { [regexp -nocase {.*not running.*} $pId] || $pId eq 0 } {
							#puts "retry failed starting iTunes."
							UTF::Message FAIL "User $u" "iTunes RETRY $tCnt: iTunes failed to start."
							set rState 0
						} else {
							#puts "retry starting iTunes."
							UTF::Message INFO "User $u" "iTunes RETRY $tCnt: iTunes started with pID $pId."
							set rState 1
						}
						#append fCause "rFlg $rFlg; rState $rState"
					}
					
					#puts "\nrFlg: $rFlg;\nrState: $rState\n"
					if { $rFlg eq 0 && $rState eq 1 } {
						#puts "iTunes run and pID OK"
						UTF::Message INFO "User $u" "iTunes start succeeded on retry: $tCnt."
						set iState 1
						break
					}
				if { $tCnt >= 3 } {
					set fCause ""
					if { $rFlg ne 0 } {
						append fCause "iTunes start error "
					}
					if { $rState ne 1 } {
						append fCause "iTunes pId not found"
					}
					UTF::Message FAIL "User $u" "Exhausted retry. Failures: $fCause"
					set iState 0
					}
				}
			} else {
				#message
				UTF::Message INFO "User $u:" "iTunes started."
				set iState 1
				break 
			};# retry loop
			if { $iState eq 1} { ;# skip the next user account if iTunes starts successfully
				#puts "iState value: $iState"
				break
			}
		} ;# end user_command block
		#puts "Failures: $fCause"
		return $iState
	};# end __startiTunesTraffic
	
		
} ;# end snit type


