#
# UTF configuration for dvtc system at sophia france testing
# Location is somwhere in the south of France
# Khun-Sour is testbed owner
# September 2013 by Khun-Sour
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
# only poule-one and poule-three were used in this test.
#
# SummaryDir sets the location for test results, file system is
# mounted on the *UTF controller*
#
#set ::UTF::SummaryDir "/projects/hnd_sig_ext16/khunsour/"
set ::UTF::SummaryDir "/projects/media_share/utf/dvtc-sophia/"

package require UTF::Power
package require UTF::Streams
package require UTF::Test::ConnectAPSTA
package require UTF::Test::APChanspec

# Relay Object
UTF::Linux dvtc_sophia_dut -lan_ip dvtc-sophia-dut.sa.broadcom.com -sta {landut eth0}
UTF::Linux dvtc_sophia_ref -lan_ip dvtc-sophia-ref.sa.broadcom.com -sta {lanp2 p17p1}

# Relay is the UTF object
# Power switch methode is ths same as henhouse, use same object
UTF::Power::UsbPower pwr_dut -relay dvtc_sophia_dut
UTF::Power::UsbPower pwr_ref -relay dvtc_sophia_ref

UTF::Router _4708/4360 -name _DUT \
    -sta {dut5ghz eth1 dut2ghz eth2} \
    -brand "linux26-internal-router" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -power {pwr_dut} \
    -relay "dvtc_sophia_dut" \
    -lanpeer landut \
    -lan_ip 192.168.1.1 \
    -console dvtc-sophia-dut.sa.broadcom.com:40000 \
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

UTF::Router _4708/4360PSTA2 -name _REF \
    -sta {ref5ghz eth1 ref2ghz eth2} \
    -brand "linux26-internal-router" \
    -tag "AARDVARK01T_TWIG_6_37_14" \
    -power {pwr_ref} \
    -relay "dvtc_sophia_ref" \
    -lanpeer lanref \
    -lan_ip 192.168.1.2 \
    -console dvtc-sophia-ref.sa.broadcom.com:40000 \
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

