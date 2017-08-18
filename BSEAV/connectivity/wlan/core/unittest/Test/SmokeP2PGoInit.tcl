#!/bin/env utf
# -*-tcl-*-

set __doc__ "Point-to-point group owner STA initialization.  Run at controller."
set __version__  {$Id: 7c6a0ff6abb2d905b7dcc076986dd97c1a2569d3 $}

package provide SmokeP2PGoInit 2.0

package require UTF
package require UTF::Test::P2PSmokeTestGOSetup

UTF::Test SmokeP2PGoInit {AP STA args} {
        # Report script info.
    UTF::Message INFO {} "In: [pwd]"
    UTF::Message INFO {} "Called as: [info script]\
        -utfconf $UTF::args(utfconf) [lrange [info level 0] 1 end]"
    foreach var {WEBROOT HTTPWEBROOT} {
        if {[catch {UTF::Message INFO {} "$var=$::env($var)"} res]} {
            UTF::Message WARN {} "$res"
        }
    }
        # Unpack options.
    UTF::Getopts {
        {tag.arg "" "Build tag"}
        {bin.arg "" "Build image path"}
        {dhd.arg "" "DHD arguments"}
        {ap_ssid.arg "UNKNOWN" "AP SSID"}
        {ap_bssid.arg "UNKNOWN" "AP BSSID"}
		{p2p_chan.arg "36" "P2P Channel"}
		{p2p_ssid.arg "DIRECT-GO" "P2P SSID"}
		{go_p2p_ip.arg "192.168.5.235"	"GO P2P IP address"}
    }
        # Adjust options.
    if {$(tag) ne ""} {
        $STA configure -tag $(tag)
    }
    if {$(bin) ne ""} {
        if {[$STA hostis Cygwin] && $(tag) ne ""} {
            $STA configure -altsys $(bin)
        } else {
            $STA configure -image $(bin)
        }
    }
    eval $STA configure $(dhd)

        # Load the driver.
	$STA load

        # Set up point-to-point group owner.
    P2PSmokeTestGOSetup -sta_go $STA -p2p_chan $(p2p_chan) \
        -p2p_ssid $(p2p_ssid) -go_p2p_ip $(go_p2p_ip)
}
