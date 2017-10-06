####################################################################################################
# A crude script for testing P2P discovery between two peers
# Note that this script requires existence of 'usleep'. For Macos, it's committed to tools/misc
####################################################################################################
#!/bin/sh
wl mpc 0
wl msglevel +apsta +p2p
wl up

if [ $# -lt 1 ]
then
    CHANNEL=1
else
    CHANNEL=$1
fi

# Default SSID is P2P-<mac>
if [ $# -lt 2 ]
then
    ssid=`wl cur_etheraddr| awk '{print "P2P-"$2}'`
    echo $ssid
else
    ssid=$2
fi

# Validate the listen channel to be one of the social channels
case $CHANNEL in
    1) OTHER_CHANNELS="6,11";;
    6) OTHER_CHANNELS="1,11";;
    11) OTHER_CHANNELS="1,6";;
    *) echo "Error: $CHANNEL has to be one of 1, 6, 11"; exit 1;;
esac

echo "Using listen channel $CHANNEL, search channels $OTHER_CHANNELS"
wl channel $CHANNEL

echo "Using SSID $ssid"
wl p2p_ssid $ssid

echo "Starting discovery. Press Any Key to Continue.."
read
wl p2p_disc 1
LISTEN=300
SEARCH=100
echo $LISTEN_usec
while true
do
  LISTEN_usec=`expr $LISTEN \* 1000`
  echo "Listening for $LISTEN on $CHANNEL"
  wl p2p_lm 1 0
  usleep $LISTEN_usec
  echo "Searching for $SEARCH on $OTHER_CHANNELS"
  wl p2p_scan -a $SEARCH -c $OTHER_CHANNELS P2P-
  sleep 1
  echo "Dumping the scan results, if any"
  wl scanresults
  LISTEN=`expr $LISTEN - 100`
  if [ $LISTEN -lt 100 ]
  then
      LISTEN=300
  fi
  wl chanspec $CHANNEL
done
