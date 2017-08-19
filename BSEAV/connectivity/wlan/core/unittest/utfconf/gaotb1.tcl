###################################
# UTF configuration for lab 2005 cart mc10
###################################

# Miscellaneous setup.
set UTF::SummaryDir /projects/hnd_sig/qgao
UTF::Logfile "utf_test.log"

###################################
# Endpoint & Relay
###################################

UTF::Linux mc10end1 -sta {lan eth1}

###################################
# STAs
###################################

# Dell E6400 Laptop Vista with 4322NIC
UTF::Cygwin qgao-lp1 -user user \
	-sta "4312Vista" \
	-osver 6 \
	-noafterburner 1 \
	-tcpwindow 512k

###################################
# APs
###################################

# Linksys 320N 4717 wireless router. 
UTF::Router TB01AP01 \
    -lan_ip 192.168.1.1 \
    -sta {4717 eth1} \
    -power "testnpc1 1" \
    -relay "mc10end1" \
    -console "mc10end1:40000" \
    -lanpeer "lan" \
    -nvram {
      fw_disable=1
      wl_msglevel=0x101
      wl0_ssid=gao4717
      wl0_channel=1
      wl0_nbw_cap=1
 }
