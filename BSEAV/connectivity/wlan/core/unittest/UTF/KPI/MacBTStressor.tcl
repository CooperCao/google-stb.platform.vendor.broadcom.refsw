#!/bin/env utf
#

# BT Stressor object for Olympic System Tesing
# Author Robert McMahon & Peter Kwan August 2015

#
# ehancements: 	look up routines cover both paired and connected devices
#				connect/disconnect individual device supported

package require snit
package require UTF
package require md5

package provide UTF::MacBTStressor 2.0

snit::type UTF::MacBTStressor::Traffic {
    typevariable EXCEPTATTRIBUTES "-name"

    option -sta -validatemethod __validatesta
    option -name -default {}
    option -verbose -default 0 -type boolean
    option -debug -default 0 -type boolean
	option -btpath -default "/Users/user"
	option -itpath -default "/Users/user/Music/iTunes"
#	option -itpath -default "/Users/user/Music/iTunes/iTunes\\\ Media/Music/Unknown\\\ Artist/Unknown\\\ Album"

    variable myid
    variable utfmsgtag
    variable errorcode
    variable pDevsExist 1
    variable pDevList ""
    variable cDevsExist 1
    variable cDevList ""
    
    # major change: variables to store values that multiple methods use
    # pDevsExist
    # pDevList
    # cDevsExist
    # cDevList
    
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
		return "UTF::MacBTStressor::Dev"
    }
    
    method setup {} {
	# This is for connecting existing paired devices
	# Check if there are paired device(s) present prior to 
	# constructing connection(s)
	set pDevs ""
	
	# compile paired list prior to proceed
	UTF::Message INFO $utfmsgtag "Finding known BT devices then connect them."
	$self listDevices paired
	if { $pDevsExist eq 0 || ! [info exists pDevList] } {
		error $utfmsgtag "No device with known profile found. Stop."
	}
	# connect known devices, then compile list of active devices
	# UTF::Message INFO $utfmsgtag "Known BT devices found. Connect them."
	$self connectDevice $pDevs
	$self listDevices connected
    }
    
    method listDevices { {ltype "paired"} } {
    # compiles a list of devices with known profile in BD address and type pairs
    set dFound 0
    set pDevs ""
    set devBD ""
    set dType ""
    set dList ""
    # set pList ""
    
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
					# append pList "$devBD "
					append dList "$devBD $dType "
					incr dFound
				}
			}
		} ;# end of line
	}
	if { $dFound eq 0 && [info exists {*}${list}] } {
		# UTF::Message FAIL $utfmsgtag "No device found."
		set cDevsExist 0; set cDevList ""
		# return $dFound
		return 0xff
	} else {
		# list of devices with known profile in address and type pairs
		# update paired device list and flag
		set ${flag} 1 ; set ${list} $dList
		# UTF::Message DEBUG $utfmsgtag "$flag value is set: [set $flag]\n; $list value is set: [set $list]\n"
		return $dList
	}
}
	
    method connectDevice {{cList ""}} {
    set cResp ""
    set cDev ""
    set dType ""
    set dList ""
    set fList ""
    set cCnt 0
    
    # first check if target type devices present: stop if none exists
    if { $pDevsExist eq 0 } {
	    error $utfmsgtag "No device with known profile found. Stop attempt to connect."
    } else {
	    if { $cList eq "" } { ;# connect all known devices
		    $self listDevices connected ;# compile current cDevList
		    set cList $pDevList
		    # if connection fails, OS throws timeout
		    foreach "cDev dType" $cList {
			    if { ! [regexp -nocase $dType $cDevList] } {
					set cResp [$options(-sta) $options(-btpath)/applebt --connect $cDev]
					UTF::Sleep 10 ;# allow devices to respond
				} else {
					# UTF::Message DEBUG $utfmsgtag "$dType \@ $cDev already connected. Do nothing."
				}
				if { [regexp -nocase {.*connectioncomplete.*} $cResp] || $cResp eq "" } {
					UTF::Message INFO $utfmsgtag "$dType \@ $cDev connected."
					append dList "$cDev $dType "
					incr cCnt
				} else { ;# device connect failed
					append fList "$cDev $dType "
				}
			} ;# end foreach loop
		} else {
			# lookup target device name and address, then connect
			set pList [$self lookupDevice paired $cList]
			foreach "cDev dType" $pList {
				if { ! [regexp -nocase $dType $cDevList] } {
					set cResp [$options(-sta) $options(-btpath)/applebt --connect $cDev]
					UTF::Sleep 10 ;# allow devices to respond

				} else {
					# UTF::Message DEBUG $utfmsgtag "$dType \@ $cDev already connected. Do nothing."
				}
				if { [regexp -nocase {.*connectioncomplete.*} $cResp] || $cResp eq "" } {
					# UTF::Message INFO $utfmsgtag "$dType \@ $cDev connected."
					append dList "$cDev $dType "
					incr cCnt
				} else {;# device connect failed
					append fList "$cDev $dType "
				}
			} ;# end foreach loop
		}
    }
    if { $cCnt > 0 } {
	    UTF::Message INFO $utfmsgtag "Devices connected successfully: $dList"
	    $self listDevices connected ;# update variable cDevList
	    return 0
    } else {
	    UTF::Message INFO $utfmsgtag "Devices failed to connect: $fList"
	    $self listDevices connected ;# update variable cDevList
	    return 0xff
    } 
}

    method disconnectDevice {{dev ""}} {
	    set dDevs ""
	    set cCnt 0
	    set cList ""
	    set dList ""
	    
	    $self listDevices connected ;# update cDevList
 	    if { $cDevsExist eq 0 || $cDevList eq "" } { ;# case no connected devices found
		    error $utfmsgtag "No connected device found."
	    } else { ;# case where connected devices present
		    if { $dev eq "" } { ;# default  - connect all if no device name or address given
			    $self listDevices connected ;# update cDevList prior to proceed
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
				if { $pList ne "" || $pList ne 0xff } {
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
		    $self listDevices connected ;# update variable cDevList
		    return 0
	    } else {
    	    # UTF::Message INFO $utfmsgtag "Devices failed to disconnect: $fList"
		    # return 0xff
	    } 
	}

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
		if { $dList ne "" } {
			UTF::Message INFO $utfmsgtag "Device found: $dList"
		}
		return $dList
	}

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
								append dList "$devRslts $dev "
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
			UTF::Message FAIL $utfmsgtag "Device not $ltype. Check device $devBD."
			# return 0xff
			
			# error $utfmsgtag "Device not $ltype. Check device $devBD."
		}
	}

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
	}

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
	}

    ### maybe redundent with new disconnectDevice method
    method disconnectAllDevices {} {
	    set dDevs ""
	    
	    UTF::Message INFO $utfmsgtag "Disconnecting all devices..."
		set dDevs [$options(-sta) $options(-btpath)/applebt --disconnectAll]
		if { $dDevs ne "" } {
			error $utfmsgtag "Failed to disconnect devices."
			return 1
		} else {
			UTF::Message INFO $utfmsgtag "All devices disconnected."
			set ::cDevList "" ; set ::cDevsExist 0
			return 0
		}
	}
    
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
	}

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
	}

	# acceptable name (no quotes): /Users/user/Music/iTunes/iTunes\\\ Media/Music/Unknown\\\ Artist/Unknown\\\ Album/rxWave.wav
    method startiTunesTraffic { {fName ""} } {
	    set iCode ""
	    set iState ""
	    set iMsg ""
	    set tCnt 0
		set {*}iFile [file join $options(-itpath) $fName]
		# UTF::Message DEBUG $utfmsgtag "fName full path: $iFile"
	
		while { $iState ne "PLAYING" } {
			catch {$options(-sta) $options(-btpath)/applebt --playFileIniTunes $iFile} iMsg
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
		}
		return $iState
	}

    method stopiTunesTraffic {} {
	    set rPid ""
	    
	    # Checking active iTunes process
	    set rPid [$self findActiveiTunes]
	    if { $rPid eq 0 } {
		    # UTF::Message INFO $utfmsgtag "iTunes is not running." ;# do nothing
		} else {
		    # kill running iTunes process found
			UTF::Message INFO $utfmsgtag "iTunes is running: $rPid; stopping active iTunes session..."
			if { [$options(-sta) rexec kill $rPid] ne "" } {
			    error $utfmsgtag "Error stopping iTunes session."
				return 1
			} else {
				UTF::Message INFO $utfmsgtag "iTunes stopped."
				return 0
			}
		}
	}
	
    method checkAudioDevices {} {
	    set aDev ""
	    set aList ""
	    
	    UTF::Message INFO $utfmsgtag "Looking for configured audio device..."
	    set aDev [$options(-sta) $options(-btpath)/applebt --listConfiguredAudioDevices]
		if { ! [regexp -nocase {.*address:\s([0-9,a-f,A-F,-]+).*Type:\s(.*)\s} $aDev - aBD aType] } {
		    error $utfmsgtag "No audio device found."
		} else {
		    # UTF::Message INFO $utfmsgtag "Audio device found. Address: $aBD; type: $aType."
	 	    append aList "$aBD $aType "
			return $aList
		}
	}

    method checkA2DPBitRate {{rTime 10}} {
	    set aDev ""
	    set aRate ""
	    
	    # UTF::Message INFO $utfmsgtag "Checking if active audio device present..."
		set aDev [$options(-sta) $options(-btpath)/applebt --isActiveAudioDevice]
	    if { [regexp -nocase {(inactive audio)} $aDev] } {
		    error $utfmsgtag "No active audio devices present."
		} else {
			UTF::Message INFO $utfmsgtag $aDev
			# proceed with A2DPBitRate only with audio device active
			UTF::Message INFO $utfmsgtag "Checking A2DP traffic bit rate..."
			set aRate [$options(-sta) $options(-btpath)/applebt --A2DPBitRate $rTime]
			return $aRate
		}
	}
	
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
		}
		UTF::Message INFO $utfmsgtag "HID Packet Error Rate results: \n$hRslts"
		return $hRslts
	}

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
				UTF::Message INFO $utfmsgtag "BT radio off."
				return 0
			}
		}
	}

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
				UTF::Message INFO $utfmsgtag "BT radio on."
				return 0
			}
		}
	}

    method checkBTRadioPowerState {} {
	    set pState ""
	    
	    UTF::Message INFO $utfmsgtag "Checking BT radio power state..."
		set pState [$options(-sta) $options(-btpath)/applebt --getPowerState]
		if { ! [regexp -nocase {.*not available} $pState] } {
			return $pState
		} else {
			error $utfmsgtag "No Bluetooth host device found."
		}
	}

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
	}
	
} ;# end snit type

