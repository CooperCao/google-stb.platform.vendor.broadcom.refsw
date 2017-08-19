#!/bin/env utf
# -*-tcl-*-
#
# UTF Framework Object Definitions
# Based on snit
# $Copyright Broadcom Corporation$
#

package provide UTF::LinuxOS 2.0

package require snit
package require UTF::doc
package require UTF::Base
package require UTF::Linux

UTF::doc {
    # [manpage_begin UTF::LinuxOS n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF Linux support}]
    # [copyright {2013 Broadcom Corporation}]
    # [require UTF]
    # [description]
    # [para]
    
    # UTF::LinuxOS is an implementation of the UTF host object, specific
    # to Linux systems. Using opensource drivers.
    
    # Once created, the LinuxOS object's methods are not normally
    # invoked directly by test scripts, instead the LinuxOS object is
    # designated the host for one or more platform independent STA
    # objects and it will be the STA objects which will be refered to
    # in the test scripts.
    
    # [list_begin definitions]
    
}

snit::type UTF::LinuxOS {
    
    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes
    
    # base handles any other options and methods
    component base -inherit yes
    
    option -name -configuremethod CopyOption
    option -bus
    option -sta
    option -type
    option -tag
    option -date
    option -modopts
    
    variable driver ""
    variable drv_mods ""
    variable drv_mods_extra ""
    variable drv_mod_params ""
    variable cloneopts
    #map channels to frequencies
    variable 2g_channels
    variable 5g_channels
    
    constructor {args} {
        set cloneopts $args
        install base using \
        UTF::Linux %AUTO% \
        -noframeburst 1 -nobighammer 1 -nobeaconratio 1 -nofragmentation 1 -nointerrupts 1
        $self configurelist $args
        
        if {$options(-name) eq ""} {
            $self configure -name [namespace tail $self]
        }
        
        # assume using upstream driver is used
        set driver [lindex [split $options(-type) -] 0]
        if {$driver eq "brcmsmac"} {
            set drv_mods [list "brcmutil.ko" "mac80211.ko" "cfg80211.ko" "bcma.ko"]
            if {[info exists ::env(SMAC_MOD_PARAMS)]} {
                set drv_mod_params "$::env(SMAC_MOD_PARAMS)"
            }
        } elseif {$driver eq "brcmfmac"} {
            set drv_mods [list "brcmutil.ko" "cfg80211.ko"]
            if {[info exists ::env(FMAC_MOD_PARAMS)]} {
                set drv_mod_params "$::env(FMAC_MOD_PARAMS)"
            }
        }
        
        # actually only needed for bcm4329 chip (why?)
        if {$options(-bus) eq "sdhci"} {
            lappend drv_mods "sdhci-pci"
        }
        
        #need this because iw link returns frequency not channel number
        array set 2g_channels {
            2412 1
            2417 2
            2422 3
            2427 4
            2432 5
            2437 6
            2442 7
            2447 8
            2452 9
            2457 10
            2462 11
            2467 12
            2472 13
            2484 14
        }
        array set 5g_channels {
            5170 34
            5180 36
            5190 38
            5200 40
            5210 42
            5220 44
            5230 46
            5240 48
            5260 52
            5280 56
            5300 60
            5320 64
            5500 100
            5520 104
            5540 108
            5560 112
            5580 116
            5600 120
            5620 124
            5640 128
            5660 132
            5680 136
            5700 140
        }
        
        
        foreach {sta dev} $options(-sta) {
            if {$dev eq ""} {
                error "$sta has no device name"
            }
            UTF::STA ::$sta -host $self -device $dev
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
    
    method lreverse {lst} {
        set reversed ""
        for {set i [llength $lst]} {[incr i -1] >= 0} {} {lappend reversed [lindex $lst $i]}
        return $reversed
    }
    
    method findimages {args} {
        # Findimages start
        UTF::Message LOG $options(-name) "findimages $args"
        
        # Defaults from object
        UTF::GetKnownopts [subst {
            {all "return all matches"}
            {ls "Report ls -l"}
            {brand.arg "[$self cget -brand]" "brand"}
            {tag.arg "$options(-tag)" "Build Tag"}
            {type.arg "$options(-type)" "Build Type"}
            {date.arg "$options(-date)" "Build Date"}
        }]
        
        set dir [file join /projects/hnd_swbuild_nl/brcm80211/build \
        $(tag) $(brand) $(date)* [$self kernel]]
        UTF::Message LOG $options(-name) "Search in $dir"
        eval UTF::SortImages [list $dir] \
        [expr {$(ls)?"-ls":""}] \
            [expr {$(all)?"-all":""}]
    }
                
                
    variable reloadlock 0
    method reload {mods} {
        try {
            # Avoid looping if the driver asserts immediately
            if {[incr reloadlock] != 1} {
                return
            }
            $self clean_udev_rules
                
            #unload first
            set ret [$self unload]
            if {$ret ne ""} {
                return $ret
            }
            UTF::Message INFO $options(-name) "Re-Load Linux Driver"
            set dir [$self findimages]
            foreach mod [$self lreverse $mods] {
                if {[regexp {(\w+)\.ko} $mod]} {
                    set modname [lindex [split $mod .] 0]
                    set mod [file join $dir $mod]
                    UTF::Message INFO $options(-name) "Inserting module $modname"
                    set ret [$self insmod $mod]
                } else {
                    UTF::Message INFO $options(-name) "Probing module $mod"
                    set ret [$self modprobe $mod]
                }
            }
            set mod [file join $dir "$driver.ko"]
            set ret [$self insmod $mod $drv_mod_params]
            return $ret
        } finally {
            incr reloadlock -1
        }
    }
            
    variable loaded ""
            
    method load {args} {
        UTF::GetKnownopts {
            {n "Just find the files, don't actually load the driver"}
        }
                
        UTF::Message INFO $options(-name) "Load Linux Driver"
        if {$drv_mods eq $loaded} {
            UTF::Message LOG $options(-name) "Skipping reload of same image"
            return
        }
                
        if {$(n)} {
            UTF::Message LOG $options(-name) "Not loading"
            return
        }
                
        foreach {- dev} $options(-sta) {
            # Make sure HOTPLUG, etc are correct before loading the
            # device the first time.
            $self check_network_scripts $dev
        }
                
        $self add_compat
        $self reload $drv_mods
        set loaded $drv_mods
        UTF::Sleep 1
        return
    }
            
    method add_compat {} {
        puts $drv_mods
        if {[lsearch $drv_mods compat.ko] >= 0} {
            return
        }
        # for Fedora 15/19 machines we use compat
        if {[regexp {\.fc1[59]\.} [$self kernel]]} {
            lappend drv_mods "compat.ko"
        }
    }
            
    method unload {} {
        UTF::Message INFO $options(-name) "Unload Linux Driver"
        set loaded ""
        set mod_loaded ""
        $self add_compat
        set unload_mods [linsert $drv_mods 0 "$driver.ko"]
        try {
            #stop wpa_supplicant
            $self wl -i wlan0 sup_wpa 0
                    
            # Avoid reloading if we crash during unload
            incr reloadlock
            foreach mmod $unload_mods {
                set modname [string map {- _} [lindex [split $mmod .] 0]]
                set mod_loaded [$self rexec "lsmod | awk '/^($modname) /{print \$1}'"]
                if {$mod_loaded ne ""} {
                    if {[catch {
                        if {[llength [split $mmod .]] eq 1} {
                            set ret [$self modprobe -r $modname]
                        } else {
                            set ret [$self rmmod $modname]
                        }
                        } ret]} {
                            error $ret
                    }
                }
            }
            UTF::Sleep 1
        } finally {
            incr reloadlock -1
        }
        return
    }
    #keep settings in here
    #wsec state
    #0 - none
    #1 - wep
    #2 - tkippsk
    #4 - aespsk2
    variable wsec_state "0"
    #sup_wpa state
    #0 - no wpa_supplicant
    #1 - start wpa_supplicant
    variable sup_wpa_state "0"
    #wep key to use
    variable wep_key ""
    #tkip pmk
    variable pmk ""
    #amode
    variable amode ""
    #wpa_supplicant PID file
    variable wpa_sup_pid "/tmp/utf_supplicant.pid"
    #wpa_supplicant log file
    variable wpa_sup_log "/tmp/utf_supplicant.log"
    #wpa_supplicant global control if file
    variable wpa_sup_gctrl "/tmp/utf_gtrcl"
    #wpa_supplicant per if control if dir
    variable wpa_sup_ctrldir "/tmp/utf_ctrl"
            
    UTF::doc {
        # [call [arg host] [method wl] [option -i] [arg name] [arg cmd]
        #      [arg {args ...}]]
        
        # Translate [cmd wl] commands into [cmd iw] commands,
        # since the native [cmd wl] command is not supported on
        # netbsd.
    }
    method wl {-i name cmd args} {
        if {${-i} ne "-i"} {
            error "usage: wl -i ifname cmd args"
        }
        if {$cmd eq "-u"} {
            set code [catch {eval $self wl -i $name $args} ret]
            if {$code && [regexp {Unsupported|not supported} $ret]} {
                set code 0
            }
            return -code $code $ret
        }
        
        set wlan $name
        UTF::Message LOG $options(-name) "wl -i $wlan $cmd $args"
        set ifconfig [list $base ifconfig $wlan]
        
        switch -- $cmd {
            ssid {
                if {$args eq ""} {
                    set ifcfg [$self iw $wlan link]
                    if {[regexp -line {SSID: (.*)$} $ifcfg - ssid]} {
                        UTF::Message LOG $options(-name) "Connected SSID: $ssid"
                        return $ssid
                    } else {
                        error "ssid not found"
                    }
                } else {
                    set ssid [lindex $args 0]
                    UTF::Message LOG $options(-name) "Join SSID: $ssid amode: $amode wsec: $wsec_state"
                    
                    set key_mgmt "NONE"
                    set auth_alg "OPEN"
                    set proto "WPA"
                    switch -- $amode {
                        wpapsk {
                            set auth_alg "OPEN"
                            set key_mgmt "WPA-PSK"
                            set proto "WPA"
                        }
                        wpa2psk {
                            set auth_alg "OPEN"
                            set key_mgmt "WPA-PSK"
                            set proto "WPA2"
                        }
                        open {
                            set auth_alg "OPEN"
                            set key_mgmt "NONE"
                        }
                        shared {
                            set auth_alg "SHARED"
                            set key_mgmt "NONE"
                        }
                        default {
                            error "unknown amode: $amode"
                        }
                    }
                    
                    #start wpa_supplicant and set basic params
                    $self wl -i $wlan sup_wpa 1
                    $self wpa_cmd $wlan "add_network"
                    $self wpa_cmd $wlan "set_network 0 ssid '\"$ssid\"'"
                    $self wpa_cmd $wlan "set_network 0 auth_alg $auth_alg"
                    $self wpa_cmd $wlan "set_network 0 key_mgmt $key_mgmt"
                    
                    switch -- $wsec_state {
                        0 {
                            #no encryption
                            #set ret [$self iw $wlan connect $ssid]
                            #return $ret
                            #TODO: this is workaround for firmware problem on USB devices
                            #43236, 43143, 43242 - 'auto' authentication mode does not fall back to open
                            #so use wpa_supplicant which will force OPEN mode
                        }
                        1 {
                            #WEP
                            if {$wep_key == ""} {
                                error "wsec == 1 but wep_key empty"
                            }
                            $self wpa_cmd $wlan "set_network 0 wep_key0 '$wep_key'"
                        }
                        2 {
                            #TKIP
                            if {$pmk == ""} {
                                error "wsec == 2 but pmk empty"
                            }
                            $self wpa_cmd $wlan "set_network 0 pairwise TKIP"
                            $self wpa_cmd $wlan "set_network 0 group TKIP"
                            $self wpa_cmd $wlan "set_network 0 psk '$pmk'"
                            $self wpa_cmd $wlan "set_network 0 proto $proto"
                        }
                        4 {
                            #AES
                            if {$pmk == ""} {
                                error "wsec == 4 but pmk empty"
                            }
                            $self wpa_cmd $wlan "set_network 0 pairwise CCMP"
                            $self wpa_cmd $wlan "set_network 0 group CCMP"
                            $self wpa_cmd $wlan "set_network 0 psk '$pmk'"
                            $self wpa_cmd $wlan "set_network 0 proto $proto"
                        }
                        default {
                            error "unknown wsec: $wsec_state"
                        }
                    }
                    #start connection
                    $self wpa_cmd $wlan "enable_network 0"
                    UTF::Sleep 2
                }
            }
            join {
                set ssid ""
                set wep_key ""
                set imode ""
                set amode ""
                set ssid [lindex $args 0]
                UTF::Message LOG $options(-name) "SSID: $ssid"
                
                if {[regexp {key (\w+)\s} $args - wep_key]} {
                    UTF::Message LOG $options(-name) "WEP key: $wep_key"
                }
                if {[regexp {imode (\w+)\s} $args - imode]} {
                    UTF::Message LOG $options(-name) "imode: $imode"
                }
                if {[regexp {amode (\w+)$} $args - amode]} {
                    UTF::Message LOG $options(-name) "amode: $amode"
                }
                $self ifconfig $wlan up
                $self wl -i $wlan ssid $ssid
            }
            bssid {
                set ifcfg [$self iw $wlan link]
                
                if {[regexp "Connected to (.{2}:.{2}:.{2}:.{2}:.{2}:.{2})" $ifcfg - bssid]} {
                    set bssid [string toupper $bssid]
                    UTF::Message LOG $options(-name) "Connected BSSID: $bssid"
                    return $bssid
                } else {
                    error "bssid not found"
                }
            }
            disassoc {
                catch {$self wpa_cmd $wlan "disconnect"}
                catch {$self wpa_cmd $wlan "disable_network 0"}
                catch {$self iw $wlan disconnect}
            }
            chanspec {
                if {$args == ""} {
                    set c ""
                    set ifcfg [$self iw $wlan link]
                    
                    if {[regexp {freq: (\S+)} $ifcfg - freq]} {
                        set 2g_chan [array get 2g_channels $freq]
                        set 5g_chan [array get 5g_channels $freq]
                        
                        if {$2g_chan != ""} {
                            set chan [lindex $2g_chan 1]
                } elseif {$5g_chan != ""} {
                    set chan [lindex $5g_chan 1]
                }
                set c [format "%s (0x%x)" $chan \
                [expr {$chan + 0x1b00}]]
                
                    }
                    if {$c != ""} {
                        UTF::Message LOG $options(-name) $c
                        return $c
                    } else {
                        error "No channel"
                    }
                } else {
                    #channel setting not supported
                    return
                }
            }
            chanspecs {
                # 40MHz primary channels
                set 5g_l_range {36 44 52 60 100 108 132 140 149 157}
                set 5g_u_range {40 48 56 64 104 112 136 144 153 161}
                # 80MHz primary channels
                set 5g_ll_range {36 52 100 132 149}
                set 5g_lu_range {40 56 104 136 153}
                set 5g_ul_range {44 60 108 140 157}
                set 5g_uu_range {48 64 112 144 161}
                set phy [$self phy_name $wlan]
                set iwinfo [$self iw $phy info]
                set bw80sup 0
                set max2g 0
                foreach {- htcap} [regexp -all -inline {Capabilities:\s0x([0-9a-fA-F]+)} $iwinfo] {
                    if {[expr [expr 0x$htcap] & 2] == 2} {
                        lappend bw40sup 1
                    } else {
                        lappend bw40sup 0
                    }
                }
                foreach {- vhtcap} [regexp -all -inline {VHT\sCapabilities\s\(0x([0-9a-fA-F]+)\):} $iwinfo] {
                    set bw80sup 1
                }
                foreach {- f c} [regexp -all -inline \
                {(\d+)\sMHz\s\[(\d+)\]} \
                $iwinfo] {
                    set c [expr int($c)]
                    if {$c <= 14 && $c > $max2g} {
                        set max2g $c
                    }
                    lappend channels $c
                }
                # 20MHz
                foreach c [lsort -integer -unique $channels] {
                    if {$c <= 14} {
                        append chanspecs [format "%s (0x%x)\n" $c \
                        [expr {$c + 0x1000}]]
                    } else {
                        append chanspecs [format "%s (0x%x)\n" $c \
                        [expr {$c + 0xd000}]]
                    }
                }
                # 40MHz - 2g
                if {[lindex $bw40sup 0] == 1} {
                    for {set c 4} {$c <= $max2g} {incr c} {
                        append chanspecs [format "%su (0x%x)\n" $c \
                        [expr {$c + 0x18fe}]]
                    }
                    for {set c 1} {$c <= [expr {$max2g - 4}]} {incr c} {
                        append chanspecs [format "%sl (0x%x)\n" $c \
                        [expr {$c + 0x1802}]]
                    }
                }
                # 40MHz - 5g
                if {[lindex $bw40sup 1] == 1} {
                    foreach c $channels {
                        if {$c > 14} {
                            if {[lsearch -integer $5g_l_range $c] >= 0} {
                                append chanspecs [format "%sl (0x%x)\n" $c \
                                [expr {$c + 0xd802}]]
                            }
                            if {[lsearch -integer $5g_u_range $c] >= 0} {
                                append chanspecs [format "%su (0x%x)\n" $c \
                                [expr {$c + 0xd8fe}]]
                            }
                        }
                    }
                }
                # 80MHz
                if {$bw80sup == 1} {
                    foreach c $channels {
                        if {$c > 14} {
                            if {[lsearch -integer $5g_ll_range $c] >= 0} {
                                append chanspecs [format "%s/80 (0x%x)\n" $c \
                                [expr {$c + 0xe006}]]
                            }
                            if {[lsearch -integer $5g_lu_range $c] >= 0} {
                                append chanspecs [format "%s/80 (0x%x)\n" $c \
                                [expr {$c + 0xe102}]]
                            }
                            if {[lsearch -integer $5g_ul_range $c] >= 0} {
                                append chanspecs [format "%s/80 (0x%x)\n" $c \
                                [expr {$c + 0xe1fe}]]
                            }
                            if {[lsearch -integer $5g_uu_range $c] >= 0} {
                                append chanspecs [format "%s/80 (0x%x)\n" $c \
                                [expr {$c + 0xe2fa}]]
                            }
                        }
                    }
                }
                UTF::Message LOG $options(-name) "Channels:\n$chanspecs"
                return $chanspecs
            }
            up {
                eval $ifconfig up
            }
            down {
                eval $ifconfig down
            }
            infra {
                return 1
            }
            a_rate -
            bg_rate -
            nrate {
                #rate setting not supported
                return
            }
            rate {
                set ifcfg [$self iw $wlan link]
                
                if {[regexp {tx bitrate: (\S+) MBit} $ifcfg - rate]} {
                    return rate
                } else {
                    return -1
                }
            }
            rssi {
                set ifcfg [$self iw $wlan link]
                
                if {[regexp {signal: (\S+) dBm} $ifcfg - rssi]} {
                    return rssi
                } else {
                    return 0
                }
                
            }
            rateset {
                #return empty - defaults to auto
                return ""
            }
            wsec {
                if { $args != "" } {
                    set wsec_state [lindex $args 0]
                }
                return $wsec_state
            }
            sup_wpa {
                if { $args != "" } {
                    set sup_wpa_state [lindex $args 0]
                }
                #kill wpa_supplicant
                set pid ""
                catch {set pid [$self cat $wpa_sup_pid]}
                if { $pid != "" } {
                    $self wpa_cmd_gctrl "terminate"
                    $self close_messages $wpa_sup_log
                    UTF::Sleep 2
                }
                
                #start wpa_supplicant
                if {$sup_wpa_state == 1} {
                    $self wpa_supplicant -d -f $wpa_sup_log -g $wpa_sup_gctrl -P $wpa_sup_pid -B
                    $self open_messages $wpa_sup_log
                    set pid [$self cat $wpa_sup_pid]
                    UTF::Sleep 2
                    #add interface
                    $self wpa_cmd_gctrl "interface_add $wlan \"\" nl80211 $wpa_sup_ctrldir"
                    
                }
                
                return $sup_wpa_state
            }
            wpa_auth {
                switch $args {
                    0 {}
                    "" {
                        return 0
                    }
                    default {
                        error "Unsupported"
                    }
                }
                return 0
            }
            set_pmk {
                set pmk [lindex $args 0]
            }
            mbss {
                # always enabled
                return 1
            }
            bss {
                if {[regexp {\mUP\M} [eval $ifconfig]]} {
                    set ret "up"
                } else {
                    set ret "down"
                }
                UTF::Message LOG $options(-name) $ret
                return $ret
            }
            radio {
                switch $args {
                    "on" {}
                    "" {
                        return 0x0000
                    }
                    default {
                        error "Unsupported"
                    }
                }
            }
            ap {
                switch $args {
                    1 {}
                    "" {
                        return 1
                    }
                    default {
                        error "Unsupported"
                    }
                }
            }
            keys {
                error "Unsupported"
            }
            bi {
                if {$args eq ""} {
                    return "100"
                } else {
                    error "Unsupported"
                }
            }
            ver {
                return [$self iw --version]
            }
            PM {
                return ""
            }
            frameburst {
                switch $args {
                    0 {}
                    "" {
                        return 0
                    }
                    default {
                        error "Unsupported"
                    }
                }
                return 0
            }
            band {
                return ""
            }
            roam_trigger {
                return 0
            }
            roam_off {
                return
            }
            default {
                #eval $base wl -i $name $cmd $args
                error "Unsupported"
            }
        }
    }
    
    method wpa_cmd {wlan cmd} {
        set ret ""
        if { [catch {$self wpa_cli -p $wpa_sup_ctrldir -i $wlan $cmd} ret]} {
            error "wpa_cli command: $cmd returned: $ret"
        }
        if { $ret == "FAIL" } {
            error "wpa_cli command: $cmd returned $ret"
        }
        return $ret
    }
    method wpa_cmd_gctrl {cmd} {
        set ret ""
        if { [catch {$self wpa_cli -g $wpa_sup_gctrl $cmd} ret] } {
            error "wpa_cli command: $cmd returned: $ret"
        }
        if { $ret == "FAIL" } {
            error "wpa_cli command: $cmd returned $ret"
        }
        return $ret
    }
    
    method phy_name {wlan} {
        set devs [$self iw dev]
        set phys ""
        foreach {- phy} [regexp -all -inline {phy#(\d+)} $devs] {
            lappend phys $phy
        }
        
        set i 0
        set phy ""
        foreach {- wl} [regexp -all -inline {Interface (wlan\d+)} $devs] {
            if {$wl == $wlan} {
                set phy [lindex $phys $i]
                break
            }
            incr i
        }
        if {$phy == ""} {
            error "cannot find phy for $wlan"
        } else {
            return "phy$phy"
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
        
        # Check the interface settings.  Devices "loaded" have already
        # been checked, but still need to check reference endpoints
        # and devices defined at runtime.
        $self check_network_scripts $dev
        
        set PF "/var/run/dhclient-${dev}.pid"
        if {[llength $args]} {
            # Setting something - kill off dhclient
            catch {$self "test -f $PF && kill `cat $PF` 2>/dev/null"}
        }
        if {$args eq "dhcp"} {
            
            # invalidate cache in case of failure
            if {[info exists ipaddr($dev)]} {
                unset ipaddr($dev)
            }
            
            if {$leases eq ""} {
                if {![catch {$self test -d /var/lib/dhclient}]} {
                    set leases "/var/lib/dhclient"
                } else {
                    set leases "/var/lib/dhcp"
                }
            }
            set LF [file join $leases "dhclient-${dev}.leases"]
            set CF "/etc/dhclient-${dev}.conf"
            if {[regexp -- {dhclient-4\.} $dhcpver]} {
                set dhclient "/sbin/dhclient -v"
            } else {
                set dhclient "/sbin/dhclient"
            }
            # Remove lease file.  We're better off not leaving leases
            # around since some dhclients take a DHCPNAK as an excuse
            # to trigger a DISASSOC.
            $self rm -f $LF
            
            # Allow for one retry on dhclient collision
            for {set i 0} {$i < 2} {incr i} {
                if {![catch {
                    $self $dhclient -1 -lf $LF -pf $PF $dev
                } ret]} {
                    break
                }
                if {$i == 0 &&
                [regexp {dhclient\((\d+)\) is already running} $ret - p]} {
                    # someone else started up a competing dhclient.
                    # Kill it and try again.
                    catch {$self kill $p}
                } else {
                    # Pass up only the last line of the error, which
                    # usually contains the actual failure message
                    error [lindex [split $ret "\n"] end]
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
            
}
#/* vim: set filetype=tcl : */
