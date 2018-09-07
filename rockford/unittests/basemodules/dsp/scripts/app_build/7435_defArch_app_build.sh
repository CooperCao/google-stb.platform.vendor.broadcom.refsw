#!/bin/bash
# *****************************************************************************
# *  Copyright (C) 2018 Broadcom.
# *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
# *
# *  This program is the proprietary software of Broadcom and/or its licensors,
# *  and may only be used, duplicated, modified or distributed pursuant to
# *  the terms and conditions of a separate, written license agreement executed
# *  between you and Broadcom (an "Authorized License").  Except as set forth in
# *  an Authorized License, Broadcom grants no license (express or implied),
# *  right to use, or waiver of any kind with respect to the Software, and
# *  Broadcom expressly reserves all rights in and to the Software and all
# *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
# *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
# *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
# *
# *  Except as expressly set forth in the Authorized License,
# *
# *  1.     This program, including its structure, sequence and organization,
# *  constitutes the valuable trade secrets of Broadcom, and you shall use all
# *  reasonable efforts to protect the confidentiality thereof, and to use this
# *  information only in connection with your use of Broadcom integrated circuit
# *  products.
# *
# *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
# *
# *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
# ******************************************************************************

#set -x
#set -e

error () { echo " ERROR: $*" 1>&2 ; }

usage ()
	{
	        echo
	        echo "usage: $0 <command options>"
		echo
		echo "--REFSW_REPO_PATH=<refsw repo path>"
		echo "--CHIP_VER=<version of the chip for which apps are being compiled. ex: A0, B0, A1, B1 etc>"
		echo "--APP_NAME=<name of app to be built. ex: playback, atlas etc | Give 'all' to build all apps>"
		echo
		echo "e.g. $0 --REFSW_REPO_PATH=/projects/bbefw/work/user/refsw/ --CHIP_VER=B1 --APP_NAME=all"
		echo

	    exit 1;
	}

# parse arguments
for arg
do
  case $arg in
    --REFSW_REPO_PATH=*) REFSW_REPO_PATH=`echo $arg | sed 's|--REFSW_REPO_PATH=||'` ;;
    --CHIP_VER=*) CHIP_VER=`echo $arg | sed 's|--CHIP_VER=||'` ;;
    --APP_NAME=*) APP_NAME=`echo $arg | sed 's|--APP_NAME=||'` ;;
    *) error "unrecognised argument '$arg'"; usage ;;
  esac
done

# Validate Input Params
echo " ######################################################################################################## "
if [ ! -d "$REFSW_REPO_PATH" ];
then
	echo " # ERROR : REFSW_REPO_PATH does not exist in input params.."
	usage
else
	echo " # INFO : User input param REFSW_REPO_PATH is: $REFSW_REPO_PATH "
fi

if [ "$CHIP_VER" = "" ];
then
	CHIP_VER="B0"
	echo " # INFO : CHIP_VER input parameter has not been provided. Using default value: $CHIP_VER.."
else
	echo " # INFO : User input param CHIP_VER is: $CHIP_VER "
fi

if [ "$APP_NAME" = "" ];
then
	APP_NAME="all"
	echo " # INFO : APP_NAME input parameter has not been provided. Using default value: $APP_NAME.."
else
	echo " # INFO : User input param APP_NAME is: $APP_NAME "
fi
echo " ######################################################################################################## "

DATE=`date +%Y%m%d`
CHIP="97435"
CHIP2="${CHIP:1:4}"
source $REFSW_REPO_PATH/rockford/unittests/basemodules/dsp/scripts/app_build/app_build_library.sh

# START: Main body of the script starts here.
echo
echo " INFO: Start build for $CHIP $CHIP_VER "
echo
echo " INFO: Setup build environment "
setenv
echo " INFO: Setup complete.. "

if [ "$APP_NAME" = "app_audio" -o "$APP_NAME" = "all" ];
then
	echo " INFO: Building the app_audio app "
	app_audio
	echo " INFO: Build of app_audio completed.. "
fi

if [ "$APP_NAME" = "ms11_dual" -o "$APP_NAME" = "all" ];
then
	echo " INFO: Building the ms11_dual app "
	ms11_dual
	echo " INFO: Build of ms11_dual completed.. "
fi

if [ "$APP_NAME" = "audio_descriptors" -o "$APP_NAME" = "all" ];
then
	echo " INFO: Building the audio_descriptors app "
	audio_descriptors
	echo " INFO: Build of audio_descriptors completed.. "
fi

if [ "$APP_NAME" = "dolby_ms11_dualplayback" -o "$APP_NAME" = "all" ];
then
	echo " INFO: Building the dolby_ms11_dualplayback app "
	dolby_ms11_dualplayback
	echo " INFO: Build of dolby_ms11_dualplayback  completed.. "
fi

if [ "$APP_NAME" = "playback" -o "$APP_NAME" = "all"  ];
then
	echo " INFO: Building the playback app "
	playback
	echo " INFO: Build of playback completed.. "
fi

if [ "$APP_NAME" = "encode_audio_es" -o "$APP_NAME" = "all"  ];
then
	echo " INFO: Building the encode_audio_es app "
	encode_audio_es
	echo " INFO: Build of encode_audio_es completed.. "
fi

if [ "$APP_NAME" = "3d_surround" -o "$APP_NAME" = "all"  ];
then
	echo " INFO: Building the 3d_surround app "
	3d_surround
	echo " INFO: Build of 3d_surround completed.. "
fi

if [ "$APP_NAME" = "raaga_test" -o "$APP_NAME" = "all" ];
then
	echo " INFO: Building the raaga_test app "
	raaga_test
	echo " INFO: Build of raaga_test completed.. "
fi

if [ "$APP_NAME" = "raaga_test_video" -o "$APP_NAME" = "all" ];
then
	echo " INFO: Building the raaga_test_video app "
	raaga_test_video
	echo " INFO: Build of raaga_test_video completed.. "
fi

if [ "$APP_NAME" = "transcode_ts" -o "$APP_NAME" = "all"  ];
then
	echo " INFO: Building the transcode_ts app "
	transcode_ts
	echo " INFO: Build of transcode_ts completed.. "
fi

if [ "$APP_NAME" = "transcode_playback_to_ts_6xaudio" -o "$APP_NAME" = "all"  ];
then
	echo " INFO: Building the transcode_playback_to_ts_6xaudio app "
	transcode_playback_to_ts_6xaudio
	echo " INFO: Build of transcode_playback_to_ts_6xaudio completed.. "
fi

if [ "$APP_NAME" = "echo_canceller" -o "$APP_NAME" = "all" ];
then
	echo " INFO: Building the echo_canceller app "
	echo_canceller
	echo " INFO: Build of echo_canceller completed.. "
fi

if [ "$APP_NAME" = "atlas" -o "$APP_NAME" = "all"  ];
then
	echo " INFO: Building the atlas app "
	atlas
	echo " INFO: Build of atlas completed.. "
fi

if [ "$APP_NAME" = "standby" -o "$APP_NAME" = "all"  ];
then
	echo " INFO: Building the standby app "
	standby
	echo " INFO: Build of standby completed.. "
fi

if [ "$APP_NAME" = "audio_dsp_selector" -o "$APP_NAME" = "all"  ];
then
	echo " INFO: Building the audio_dsp_selector app "
	audio_dsp_selector
	echo " INFO: Build of audio_dsp_selector completed.. "
fi

if [ "$APP_NAME" = "ip_streamer" -o "$APP_NAME" = "all"  ];
then
	echo " INFO: Building the ip_streamer app "
	ip_streamer
	echo " INFO: Build of ip_streamer completed.. "
fi

if [ "$APP_NAME" = "nxclient" -o "$APP_NAME" = "all"  ];
then
	echo " INFO: Building the nxclient app "
	nxclient
	echo " INFO: Build of nxclient completed.. "
fi
echo
echo " INFO: End of $CHIP build script "

# END: Main body of the script ends here
