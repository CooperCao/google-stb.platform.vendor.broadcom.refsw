#
# UTF configuration for HenHouse testing
# Location is somwhere in the south of France
# Khun-Sour is testbed owner
# August 2013 by Robert McMahon and Khun-Sour
#
# Some end station WAR notes that should be addressed in setup
#   o End stations need the following binaries in /usr/local/bin
#     - eppi_ping
#     - ush
#     - consolelogger
#  o  <utftree>/apshell expected on end stations attached to router
#  o don't forget systemctl enable/start consolelogger.service providing
#    UTF shared serial port (via apsell)
#
# Tested with xlinux based controller using script Test/xtcpyudp.test
# poule-one to poule-six are used.
#
# SummaryDir sets the location for test results, file system is
# mounted on the *UTF controller*
#
# set ::UTF::SummaryDir "/projects/hnd_sig_ext16/khunsour/"
set ::UTF::SummaryDir "/projects/media_share/utf/henhouse/Multilink"

package require UTF::Power
package require UTF::Streams
package require UTF::Test::ConnectAPSTA
package require UTF::Test::APChanspec

UTF::Linux poule_one -lan_ip poule-one.sa.broadcom.com -sta {lanp1 p17p1}
UTF::Linux poule_two -lan_ip poule-two.sa.broadcom.com -sta {lanp2 p17p1}
UTF::Linux poule_three -lan_ip poule-three.sa.broadcom.com -sta {lanp3 p17p1}
UTF::Linux poule_four -lan_ip poule-four.sa.broadcom.com -sta {lanp4 p17p1}
UTF::Linux poule_five -lan_ip poule-five.sa.broadcom.com -sta {lanp5 p17p1}
UTF::Linux poule_six -lan_ip poule-six.sa.broadcom.com -sta {lanp6 p17p1}

# Relay is the UTF object
UTF::Power::AviosysUSB pwr_poule1 -relay poule_one
UTF::Power::AviosysUSB pwr_poule2 -relay poule_two
UTF::Power::AviosysUSB pwr_poule3 -relay poule_three
UTF::Power::AviosysUSB pwr_poule4 -relay poule_four
UTF::Power::AviosysUSB pwr_poule5 -relay poule_five
UTF::Power::AviosysUSB pwr_poule6 -relay poule_six

#set ::UTF::RexecObjectBeta 1

UTF::Router _4706/4360 -name _AP \
    -sta {4706mch5 eth1 4706mch2 eth2} \
    -brand "linux26-internal-router" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -power {pwr_poule1} \
    -relay "poule_one" \
    -lanpeer lanp1 \
    -lan_ip 192.168.1.1 \
    -console poule-one.sa.broadcom.com:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
	lan_ipaddr=192.168.1.1
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4706/4360
	wl1_ssid=4706/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	wl_reg_mode=h
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
    }

UTF::Router _4708/4360 -name _4708ap \
    -sta {4708mch5 eth2 4708mch2 eth1} \
    -brand "linux-2.6.36-arm-internal-router" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -power {pwr_poule1} \
    -relay "poule_one" \
    -lanpeer lanp1 \
    -lan_ip 192.168.1.1 \
    -console poule-one.sa.broadcom.com:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
    lan_ipaddr=192.168.1.1
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4708/4360
	wl1_ssid=4708/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	wl_reg_mode=h
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
    }	
	
UTF::Router _4706/4360PSTA2 -name _PSTA2 \
    -sta {psta2mch5 eth1 psta2mch2 eth2} \
    -brand "linux26-internal-router" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -power {pwr_poule2} \
    -relay "poule_two" \
    -lanpeer lanp2 \
    -lan_ip 192.168.1.2 \
    -console poule-two.sa.broadcom.com:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.2
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4708/4360
	wl1_ssid=4708/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	wl_reg_mode=h
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
    }

UTF::Router _4706/4360PSTA3 -name _PSTA3 \
    -sta {psta3mch5 eth1 psta3mch2 eth2} \
    -brand "linux26-internal-router" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -power {pwr_poule3} \
    -relay "poule_three" \
    -lanpeer lanp3 \
    -lan_ip 192.168.1.3 \
    -console poule-three.sa.broadcom.com:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.3
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4708/4360
	wl1_ssid=4708/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	wl_reg_mode=h
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
    }

UTF::Router _4706/4360PSTA4 -name _PSTA4 \
    -sta {psta4mch5 eth1 psta4mch2 eth2} \
    -brand "linux26-internal-router" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -power {pwr_poule4} \
    -relay "poule_four" \
    -lanpeer lanp4 \
    -lan_ip 192.168.1.4 \
    -console {} \
    -console poule-four.sa.broadcom.com:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.4
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4708/4360
	wl1_ssid=4708/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	wl_reg_mode=h
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
    }

UTF::Router _4706/4360PSTA5 -name _PSTA5 \
    -sta {psta5mch5 eth1 psta5mch2 eth2} \
    -brand "linux26-internal-router" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -power {pwr_poule5} \
    -relay "poule_five" \
    -lanpeer lanp5 \
    -lan_ip 192.168.1.5 \
    -console {} \
    -console poule-five.sa.broadcom.com:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.5
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4708/4360
	wl1_ssid=4708/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	wl_reg_mode=h
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
    }

UTF::Router _4706/4360PSTA6 -name _PSTA6 \
    -sta {psta6mch5 eth1 psta6mch2 eth2} \
    -brand "linux26-internal-router" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -power {pwr_poule6} \
    -relay "poule_six" \
    -lanpeer lanp6 \
    -lan_ip 192.168.1.6 \
    -console poule-six.sa.broadcom.com:40000 \
    -trx linux-apsta \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.6
	watchdog=6000; # PR#90439
	wl0_channel=1
	wl0_radio=1
	wl0_obss_coex=0
	wl0_ssid=4708/4360
	wl1_ssid=4708/4331
	wl1_channel=36
	wl1_radio=1
	wl1_obss_coex=0
	router_disable=0
	wl_reg_mode=h
	wl_wmf_ucigmp_query=1
	wl_wmf_ucast_upnp=1
	wl_wmf_acs_fcs_mode=1
	wl_acs_excl_chans=0xd98e,0xd88e,0xe28a,0xe38a
	wl_pspretend_retry_limit=5
    }
