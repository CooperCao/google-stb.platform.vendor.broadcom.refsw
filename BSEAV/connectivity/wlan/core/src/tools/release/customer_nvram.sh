#!/bin/sh
# * Tool to generate Nvram based on btcx config
# *
# * Broadcom Proprietary and Confidential. Copyright (C) 2017,
# * All Rights Reserved.
# * 
# * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# * the contents of this file may not be disclosed to third parties, copied
# * or duplicated in any form, in whole or in part, without the prior
# * written permission of Broadcom.
# *
# * $Id: customer_nvram.sh 513189 2014-11-05 20:50:33Z $
# */


EXPECTED_ARGS=2

if [ $# -lt $EXPECTED_ARGS ]
then
    echo "Number of arguments $# "
	echo "Minimum Number of arguments are 2"
    echo " Usage:
	-- customer_nvram.sh bcm943569xxx.txt jammerlevel (default cusb disabled)
	-- To enable cusb customer_nvram.sh bcm943569xxx.txt jammerlevel cusb"
    exit 1
fi


if [ "$2" -ne "-17" ]
then
	if [ "$2" -ne "-22" ]
	then
		if [ "$2" -ne "-27" ]
		then
			if [ "$2" -ne "-32" ]
			then
				if [ "$2" -ne "-13" ]
				then
					if [ "$2" -ne "-18" ]
					then
						if [ "$2" -ne "-23" ]
						then
							if [ "$2" -ne "-28" ];
then
  echo "$2 is not supported jammer level
			supported jammer levels are : -17,-22,-27,-32,-13,-18,-23,-28"
  exit 1
fi
fi
fi
fi
fi
fi
fi
fi

sed --in-place '/usbdesc_composite=0x010A/d' $1
sed --in-place '/btc_params82=0x0060/d' $1
sed --in-place '/btc_params84=0x8/d' $1
sed --in-place '/btc_params73=100 /d' $1
sed --in-place '/btc_params101=100/d' $1
sed --in-place '/btc_params51=0x48df/d' $1
sed --in-place '/btcdyn_flags=0x7/d' $1
sed --in-place '/btcdyn_flags=0x3/d' $1
sed --in-place '/btcdyn_flags=0x6/d' $1
sed --in-place '/btcdyn_dflt_dsns_level=0/d' $1
sed --in-place '/btcdyn_low_dsns_level=0/d' $1
sed --in-place '/btcdyn_mid_dsns_level=0/d' $1
sed --in-place '/btcdyn_high_dsns_level=0/d' $1
sed --in-place '/btcdyn_mid_dsns_level=22/d' $1
sed --in-place '/btcdyn_mid_dsns_level=21/d' $1
sed --in-place '/btcdyn_mid_dsns_level=23/d' $1
sed --in-place '/btcdyn_high_dsns_level=22/d' $1
sed --in-place '/btcdyn_high_dsns_level=21/d' $1
sed --in-place '/btcdyn_high_dsns_level=23/d' $1
sed --in-place '/btcdyn_default_btc_mode=4/d' $1
sed --in-place '/btcdyn_default_btc_mode=5/d' $1
sed --in-place '/btcdyn_btrssi_hyster=5/d' $1
sed --in-place '/btcdyn_dsns_rows=2/d' $1
sed --in-place '/btcdyn_dsns_rows=1/d' $1
sed --in-place '/btcdyn_msw_rows=1/d' $1

sed --in-place '/btcdyn_dsns_row0=4,-120,0,-53,-73/d' $1
sed --in-place '/btcdyn_dsns_row1=5,-120,0,-53,-73/d' $1
sed --in-place '/btcdyn_dsns_row0=5,-120,0,-53,-73/d' $1
sed --in-place '/btcdyn_msw_row0=5,-120,-65,0,-100/d' $1
sed --in-place '/btcdyn_dsns_row0=4,-120,0,-55,-73/d' $1
sed --in-place '/btcdyn_dsns_row1=5,-120,0,-55,-73/d' $1
sed --in-place '/btcdyn_msw_row0=5,-120,-70,0,-100/d' $1
sed --in-place '/btcdyn_dsns_row0=4,-120,0,-55,-100/d' $1
sed --in-place '/btcdyn_dsns_row1=5,-120,0,-55,-100/d' $1
sed --in-place '/btcdyn_msw_row0=5,-120,-75,0,-100/d' $1
sed --in-place '/btcdyn_dsns_row0=4,-120,0,-55,-100/d' $1
sed --in-place '/btcdyn_dsns_row1=5,-120,0,-55,-100/d' $1
sed --in-place '/btcdyn_msw_row0=5,-120,-80,0,-100/d' $1
sed --in-place '/btcdyn_dsns_row0=4,-120,0,-58,-75/d' $1
sed --in-place '/btcdyn_dsns_row1=5,-120,0,-58,-75/d' $1
sed --in-place '/btcdyn_msw_row0=5,-120,-60,0,-100/d' $1
sed --in-place '/btcdyn_dsns_row0=4,-120,0,-52,-75/d' $1
sed --in-place '/btcdyn_dsns_row1=5,-120,0,-52,-75/d' $1
sed --in-place '/btcdyn_msw_row0=5,-120,-65,0,-100/d' $1
sed --in-place '/btcdyn_dsns_row0=4,-120,0,-52,-100/d' $1
sed --in-place '/btcdyn_dsns_row1=5,-120,0,-52,-100/d' $1
sed --in-place '/btcdyn_msw_row0=5,-120,-70,0,-100/d' $1
sed --in-place '/btcdyn_dsns_row0=4,-120,0,-52,-100/d' $1
sed --in-place '/btcdyn_dsns_row1=5,-120,0,-52,-100/d' $1
sed --in-place '/btcdyn_msw_row0=5,-120,-75,0,-100/d' $1

sed --in-place '/btcdyn_dsns_row0=5,-120,0,-53,-73/d' $1
sed --in-place '/btcdyn_dsns_row0=5,-120,0,-55,-73/d' $1
sed --in-place '/btcdyn_dsns_row0=5,-120,0,-55,-100/d' $1
sed --in-place '/btcdyn_dsns_row0=5,-120,0,-55,-100/d' $1
sed --in-place '/btcdyn_dsns_row0=5,-120,0,-58,-75/d' $1
sed --in-place '/btcdyn_dsns_row0=5,-120,0,-52,-75/d' $1
sed --in-place '/btcdyn_dsns_row0=5,-120,0,-52,-100/d' $1
sed --in-place '/btcdyn_dsns_row0=4,-120,0,-52,-100/d' $1

echo >> $1
echo "#BTCX dynamic desense entries for jammer level $2">> $1
echo 'btcx_tool_rev="$Rev$"'>> $1

if [ "$3" = "cusb" ]
then
  echo 'usbdesc_composite=0x010A' >> $1
fi

echo "btc_params82=0x0060
btc_params84=0x8
btc_params73=100
btc_params101=100
btc_params51=0x48df" >> $1

case $2 in

"-17")
echo "btcdyn_flags=0x7
btcdyn_dflt_dsns_level=0
btcdyn_low_dsns_level=0
btcdyn_mid_dsns_level=22
btcdyn_high_dsns_level=23
btcdyn_default_btc_mode=4
btcdyn_btrssi_hyster=5
btcdyn_dsns_rows=2
btcdyn_msw_rows=1
btcdyn_dsns_row0=4,-120,0,-53,-73
btcdyn_dsns_row1=5,-120,0,-53,-73
btcdyn_msw_row0=5,-120,-65,0,-100" >> $1
;;

"-22")
echo "btcdyn_flags=0x7
btcdyn_dflt_dsns_level=0
btcdyn_low_dsns_level=0
btcdyn_mid_dsns_level=21
btcdyn_high_dsns_level=23
btcdyn_default_btc_mode=4
btcdyn_btrssi_hyster=5
btcdyn_dsns_rows=2
btcdyn_msw_rows=1
btcdyn_dsns_row0=4,-120,0,-55,-73
btcdyn_dsns_row1=5,-120,0,-55,-73
btcdyn_msw_row0=5,-120,-70,0,-100" >> $1
;;

"-27")
echo "btcdyn_flags=0x3
btcdyn_dflt_dsns_level=0
btcdyn_low_dsns_level=0
btcdyn_mid_dsns_level=0
btcdyn_high_dsns_level=23
btcdyn_default_btc_mode=4
btcdyn_dsns_rows=1
btcdyn_dsns_row0=4,-120,0,-55,-100" >> $1
;;


"-32")
echo "btcdyn_flags=0x3
btcdyn_dflt_dsns_level=0
btcdyn_low_dsns_level=0
btcdyn_mid_dsns_level=0
btcdyn_high_dsns_level=23
btcdyn_default_btc_mode=4
btcdyn_dsns_rows=1
btcdyn_dsns_row0=4,-120,0,-55,-100" >> $1
;;

"-13")
echo "btcdyn_flags=0x7
btcdyn_dflt_dsns_level=0
btcdyn_low_dsns_level=0
btcdyn_mid_dsns_level=22
btcdyn_high_dsns_level=23
btcdyn_default_btc_mode=4
btcdyn_btrssi_hyster=5
btcdyn_dsns_rows=2
btcdyn_msw_rows=1
btcdyn_dsns_row0=4,-120,0,-58,-75
btcdyn_dsns_row1=5,-120,0,-58,-75
btcdyn_msw_row0=5,-120,-60,0,-100" >> $1
;;

"-18")
echo "btcdyn_flags=0x7
btcdyn_dflt_dsns_level=0
btcdyn_low_dsns_level=0
btcdyn_mid_dsns_level=22
btcdyn_high_dsns_level=23
btcdyn_default_btc_mode=4
btcdyn_btrssi_hyster=5
btcdyn_dsns_rows=2
btcdyn_msw_rows=1
btcdyn_dsns_row0=4,-120,0,-52,-75
btcdyn_dsns_row1=5,-120,0,-52,-75
btcdyn_msw_row0=5,-120,-65,0,-100" >> $1
;;



"-23")
echo "btcdyn_flags=0x7
btcdyn_dflt_dsns_level=0
btcdyn_low_dsns_level=0
btcdyn_mid_dsns_level=0
btcdyn_high_dsns_level=23
btcdyn_default_btc_mode=4
btcdyn_btrssi_hyster=5
btcdyn_dsns_rows=2
btcdyn_msw_rows=1
btcdyn_dsns_row0=4,-120,0,-52,-100
btcdyn_dsns_row1=5,-120,0,-52,-100
btcdyn_msw_row0=5,-120,-70,0,-100" >> $1
;;


"-28")
echo "btcdyn_flags=0x3
btcdyn_dflt_dsns_level=0
btcdyn_low_dsns_level=0
btcdyn_mid_dsns_level=0
btcdyn_high_dsns_level=23
btcdyn_default_btc_mode=4
btcdyn_dsns_rows=1
btcdyn_dsns_row0=4,-120,0,-52,-100" >> $1
;;

esac
