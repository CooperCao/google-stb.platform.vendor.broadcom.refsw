# -*-tcl-*-
set __Pod_version__  {$Id$}

set webroot "testweb"
if {[info exists ::env(WEBROOT)]} {
    set webroot $::env(WEBROOT)
}
set ::UTF::SummaryDir "$webroot/$sta_type"

# Controller.
UTF::Linux $ctrl_ip\
    -sta {Controller eth1}
Controller configure -ipaddr $ctrl_ip

# Power controllers.
UTF::Power::WebRelay Power -lan_ip $pwr_ip -relay $ctrl_ip
UTF::Power::Laptop Button1 -button {Power 1}
UTF::Power::Laptop Button2 -button {Power 2}
UTF::Power::Laptop Button3 -button {Power 3}
UTF::Power::Laptop Button4 -button {Power 4}
UTF::Power::Laptop Button5 -button {Power 5}
UTF::Power::Laptop Button6 -button {Power 6}
if {[info exists pwr_usb1_ip]} {
    UTF::Power::WebRelay PowerUSB1 -lan_ip $pwr_usb1_ip -relay $ctrl_ip
}
if {[info exists pwr_usb2_ip]} {
    UTF::Power::WebRelay PowerUSB2 -lan_ip $pwr_usb2_ip -relay $ctrl_ip
}

# AP.
UTF::Router AP\
    -sta {AP_proto eth1}\
    -relay Controller\
    -lanpeer Controller\
    -power {Power 8}\
    -brand linux26-internal-router\
    -tag "AKASHI_REL_5_110_35"\
    -nvram {
		lan_ipaddr=192.168.1.1
        watchdog=3000
        console_loglevel=7
        fw_disable=1
        router_disable=0
        wl0_ssid=smoke
        wl0_radio=1
        wl0_channel=1
        wl0_obss_coex=0
        wl1_ssid=smoke5
        wl1_radio=1
        wl1_channel=36
        wl1_obss_coex=0
    }

# STA setting.

# IP, power.
proc set_stas {} {
    upvar 1 sta_type sta_t sta_table sta_tb
    foreach {seq ip} [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        $sta_t clone $name -name $ip\
            -lan_ip $ip -power [list Power $seq]
    }
}

# wlan_ip,IP, power.
proc set_stas_wlan {} {
    upvar 1 sta_type sta_t sta_table sta_tb
    foreach {seq wlan_ip ip} [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        $sta_t clone $name -name $ip\
            -lan_ip $ip -power [list Power $seq]
        $name configure -ipaddr $wlan_ip
    }
}

# wlan_ip,IP, button.
proc set_stas_wlan_lap {} {
    upvar 1 sta_type sta_t sta_table sta_tb
    foreach {seq wlan_ip ip} [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        $sta_t clone $name -name $ip\
            -lan_ip $ip -power [list Button$seq]
        $name configure -ipaddr $wlan_ip
    }
}

# IP, power, console.
proc set_stas_wcons {} {
    upvar 1 sta_type sta_t sta_table sta_tb ctrl_ip ctrl_i
    foreach {seq ip port} [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        $sta_t clone $name -name $ip\
            -lan_ip $ip -power [list Power $seq]\
            -console "$ctrl_i:$port"
    }
}

# IP, power, console, rteconsole.
proc set_stas_rtr {} {
    upvar 1 sta_type sta_t sta_table sta_tb ctrl_ip ctrl_i
    foreach {seq ip serial_port uart_port} [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        $sta_t clone $name -name $ip\
            -lan_ip $ip -power [list Power $seq]\
            -console "$ctrl_i:$serial_port"
            -rteconsole "$ctrl_i:$uart_port"
    }
}

# IP, power, console, hostconsole, power usb, usb port
proc set_stas_bmac {} {
    upvar 1 sta_type sta_t sta_table sta_tb ctrl_ip ctrl_i
    foreach {seq ip port host_port power_usb usb_port}\
      [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        $sta_t clone $name -name $ip\
            -lan_ip $ip -power [list Power $seq]\
            -console "$ctrl_i:$port"\
            -hostconsole "$ctrl_i:$host_port"\
            -power_sta [list $power_usb $usb_port]
    }
}

# IP, power, hostconsole, mac
proc set_stas_dhd_pcie {} {
    upvar 1 sta_type sta_t sta_table sta_tb ctrl_ip ctrl_i
    foreach {seq ip host_port mac}\
      [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        $sta_t clone $name -name $ip\
            -lan_ip $ip -power [list Power $seq]\
            -hostconsole "$ctrl_i:$host_port"\
            -nvram_add "macaddr=$mac"
    }
}

# IP, power, hostconsole, mac, ccode, regrev
proc set_stas_dhd_pcie_cust {} {
    upvar 1 sta_type sta_t sta_table sta_tb ctrl_ip ctrl_i
    foreach {seq ip host_port mac code rev}\
      [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        $sta_t clone $name -name $ip\
            -lan_ip $ip -power [list Power $seq]\
            -hostconsole "$ctrl_i:$host_port"\
            -nvram_add "macaddr=$mac ccode=$code regrev=$rev"
    }
}
# IP, power, console, hostconsole, power usb, usb port, mac.
proc set_stas_dhd {} {
    upvar 1 sta_type sta_t sta_table sta_tb ctrl_ip ctrl_i
    foreach {seq ip port host_port power_usb usb_port mac}\
      [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        $sta_t clone $name -name $ip\
            -lan_ip $ip -power [list Power $seq]\
            -console "$ctrl_i:$port"\
            -hostconsole "$ctrl_i:$host_port"\
            -power_sta [list $power_usb $usb_port]\
            -nvram_add "macaddr=$mac"
    }
}

# IP, power, hostconsole, power usb, usb port, mac.
proc set_stas_dhd_nocons {} {
    upvar 1 sta_type sta_t sta_table sta_tb ctrl_ip ctrl_i
    foreach {seq ip host_port power_usb usb_port mac}\
      [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        $sta_t clone $name -name $ip\
            -lan_ip $ip -power [list Power $seq]\
            -hostconsole "$ctrl_i:$host_port"\
            -power_sta [list $power_usb $usb_port]\
            -nvram_add "macaddr=$mac"
    }
}

# IP, power, console, power, port.
proc set_stas_dhd_nocons_stb {} {
    upvar 1 sta_type sta_t sta_table sta_tb ctrl_ip ctrl_i
    foreach {seq ip host_port mac wlan_ip}\
      [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        $sta_t clone $name -name $ip\
            -lan_ip $ip -power [list Power $seq]\
            -console "$ctrl_i:$host_port"\
            -power_sta [list Power $seq]\
            -nvram_add "macaddr=$mac"
        $name configure -ipaddr $wlan_ip
    }
}

# shell, power, console, hostconsole, mac, relay_ip.
proc set_stas_panda {} {
    upvar 1 relay_type relay_t
    upvar 1 sta_type sta_t sta_table sta_tb ctrl_ip ctrl_i
    foreach {seq wlan_ip port host_port mac relay_ip}\
      [UTF::decomment $sta_tb] {
        UTF::Linux $relay_ip
        set name ${sta_t}_$seq
        $sta_t clone $name \
            -lan_ip shell -power [list Power $seq]\
            -console "$ctrl_i:$port"\
            -hostconsole "$ctrl_i:$host_port"\
            -nvram_add "macaddr=$mac"\
            -relay "$relay_ip"
        $name configure -ipaddr $wlan_ip
    }
}

# shell, power, hostconsole, mac, relay_ip.
proc set_stas_panda_nocons {} {
    upvar 1 relay_type relay_t
    upvar 1 sta_type sta_t sta_table sta_tb ctrl_ip ctrl_i
    foreach {seq wlan_ip host_port mac relay_ip}\
      [UTF::decomment $sta_tb] {
        UTF::Linux $relay_ip
        set name ${sta_t}_$seq
        $sta_t clone $name \
            -lan_ip shell -power [list Power $seq]\
            -hostconsole "$ctrl_i:$host_port"\
            -nvram_add "macaddr=$mac"\
            -relay "$relay_ip"
        $name configure -ipaddr $wlan_ip
    }
}

proc set_stas_dhd_pcie_dual {} {
    upvar 1 sta_type sta_t sta_type_aux sta_t_a sta_table sta_tb \
    ctrl_ip ctrl_i
    set maindev [$sta_t cget -device]
    set auxdev [$sta_t_a cget -device]
    foreach {seq ip host_port mac}\
    [UTF::decomment $sta_tb] {
        set name ${sta_t}_$seq
        set auxname ${sta_t_a}_$seq
        $sta_t clone $name -sta [list $name $maindev $auxname $auxdev] \
        -name $ip -lan_ip $ip -power [list Power $seq]\
        -hostconsole "$ctrl_i:$host_port"\
        -nvram_add "macaddr=$mac"

     }
}
