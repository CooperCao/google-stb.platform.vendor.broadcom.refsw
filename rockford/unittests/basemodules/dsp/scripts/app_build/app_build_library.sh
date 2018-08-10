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

##################################################
# START: Function definition for apps to be built.
##################################################

echo " ~~~~~~~~ INFO: chip : $CHIP            "
echo " ~~~~~~~~ INFO: chip_ver : $CHIP_VER    "
if [ "$CHIP" = "97255" ];
then
echo " ~~~~~~~~ INFO: chip_arch : $CHIP_ARCH  "
	OBJ_FOLDER_NAME="obj.$CHIP.$CHIP_VER.$CHIP_ARCH"
elif [ "$CHIP" = "97346" ];
then
echo " ~~~~~~~~ INFO: chip_endianness : $ENDIANNESS  "
	END=${ENDIANNESS^^}
	OBJ_FOLDER_NAME="obj.$CHIP.$CHIP_VER.$END"
else
	OBJ_FOLDER_NAME="obj.$CHIP.$CHIP_VER"
fi
echo " ~~~~~~~~ INFO: Obj folder name is $OBJ_FOLDER_NAME "

3d_surround ()
	{
		# Building the 3d_surround app
		cd $REFSW_REPO_PATH/nexus/examples/audio
		make APPS=3d_surround -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/3d_surround" > /dev/null ; then
			 echo "INFO: 3d_surround has been built successfully"
		else
			 echo "ERROR: 3d_surround couldn't be built".
		fi
	}

app_audio ()
	{
		# Build the app_audio app
		cd $REFSW_REPO_PATH/rockford/unittests/nexus/audio/
		make APPS=app_audio -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/app_audio" > /dev/null ; then
			 echo " INFO: app_audio has been built successfully"
		else
			 echo " ERROR: app_audio couldn't be built".
		fi
	}

atlas ()
	{
		# Build atlas app
		cd $REFSW_REPO_PATH/BSEAV/app/atlas/build
		make install
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/BSEAV/bin/refsw-$DATE.$CHIP*" > /dev/null ; then
			 echo " INFO: refsw-$DATE.$CHIP* has been built successfully"
		else
			 echo " ERROR: refsw-$DATE.$CHIP* couldn't be built".
		fi
	}

audio_descriptors ()
	{
		# build audio_descriptors app
		cd $REFSW_REPO_PATH/nexus/examples/audio
		make APPS=audio_descriptors -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/audio_descriptors" > /dev/null ; then
			 echo " INFO: audio_descriptors has been built successfully"
		else
			 echo " ERROR: audio_descriptors couldn't be built".
		fi
	}

audio_dsp_selector ()
	{
		# build audio_dsp_selector app
		cd $REFSW_REPO_PATH/nexus/examples/audio/
		make THEAPPS=audio_dsp_selector -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/audio_dsp_selector" > /dev/null ; then
			 echo " INFO: audio_dsp_selector has been built successfully"
		else
			 echo " ERROR: audio_dsp_selector couldn't be built".
		fi
	}

dolby_ms11_dualplayback ()
	{
		# Building the dolby_ms11_dualplayback app
		cd $REFSW_REPO_PATH/rockford/unittests/nexus/dolby
		make APPS=dolby_ms11_dualplayback -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/dolby_ms11_dualplayback" > /dev/null ; then
			 echo "INFO: dolby_ms11_dualplayback has been built successfully"
		else
			 echo "ERROR: dolby_ms11_dualplayback couldn't be built".
		fi
	}

dolby_ms12_dualplayback ()
	{
		# build dolby_ms12_dualplayback app
		cd $REFSW_REPO_PATH/rockford/unittests/nexus/dolby/
		make APPS=dolby_ms12_dualplayback -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/dolby_ms12_dualplayback" > /dev/null ; then
			 echo " INFO: dolby_ms12_dualplayback has been built successfully"
		else
			 echo " ERROR: dolby_ms12_dualplayback couldn't be built".
		fi
	}

echo_canceller ()
	{
		# Building the echo_canceller app
		cd $REFSW_REPO_PATH/rockford/unittests/nexus/audio
		make APPS=echo_canceller -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/echo_canceller" > /dev/null ; then
			 echo "INFO: echo_canceller has been built successfully"
		else
			 echo "ERROR: echo_canceller couldn't be built".
		fi
	}

encode ()
	{
		# Building the encode app
		cd $REFSW_REPO_PATH/rockford/unittests/nexus/encoder
		make APPS=encode -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/encode" > /dev/null ; then
			 echo "INFO: encode has been built successfully"
		else
			 echo "ERROR: encode couldn't be built".
		fi
	}

encode_audio_es ()
	{
		# Building the encode_audio_es app
		cd $REFSW_REPO_PATH/nexus/examples/encoder
		make -j8 THEAPPS=encode_audio_es -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/encode_audio_es" > /dev/null ; then
			 echo "INFO: encode_audio_es has been built successfully"
		else
			 echo "ERROR: encode_audio_es couldn't be built".
		fi
	}

ip_streamer ()
	{
		cd $REFSW_REPO_PATH/nexus/lib/playback_ip/apps
		make APPS=ip_streamer -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/ip_streamer" > /dev/null ; then
			 echo "INFO: ip_streamer has been built successfully"
		else
			 echo "ERROR: ip_streamer couldn't be built".
		fi
	}

ms11_dual ()
	{
		# Building the ms11_dual app
		cd $REFSW_REPO_PATH/nexus/examples/audio
		make APPS=ms11_dual -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/ms11_dual" > /dev/null ; then
			 echo "INFO: ms11_dual has been built successfully"
		else
			 echo "ERROR: ms11_dual couldn't be built".
		fi
	}

nxclient ()
	{
		# Build nxclient app
		cd $REFSW_REPO_PATH/nexus/nxclient
		make -j32
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/nxclient" > /dev/null ; then
			 echo " INFO: nxclient has been built successfully"
		else
			 echo " ERROR: nxclient couldn't be built".
		fi
	}

playback ()
	{
		# Build the playback app
		cd $REFSW_REPO_PATH/nexus/utils
		make playback -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/playback" > /dev/null ; then
			 echo " INFO: playback has been built successfully"
		else
			 echo " ERROR: playback couldn't be built".
		fi
	}

audio_playback ()
	{
		# Build the audio_playback app
		cd $REFSW_REPO_PATH/nexus/examples/audio
		make audio_playback -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/audio_playback" > /dev/null ; then
			 echo " INFO: audio_playback has been built successfully"
		else
			 echo " ERROR: audio_playback couldn't be built".
		fi
	}

raaga_test ()
	{
		# build raaga_test app
		cd $REFSW_REPO_PATH/rockford/unittests/basemodules/dsp/raaga/
		make
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/rockford/unittests/basemodules/dsp/raaga/$CHIP.$CHIP_VER.debug/raaga_test" > /dev/null ; then
			 echo " INFO: raaga_test has been built successfully"
		else
			 echo " ERROR: raaga_test couldn't be built".
		fi

		# Copying the bcmdriver.ko driver into obj.$CHIP towards building the app raaga_test
		cd $REFSW_REPO_PATH/BSEAV/linux/driver/build
		make clean; make
		sync

		if [ "`find $REFSW_REPO_PATH/$OBJ_FOLDER_NAME/BSEAV/linux/driver -name bcmdriver.ko -print`" != "" ]; then
		cp -v `find $REFSW_REPO_PATH/$OBJ_FOLDER_NAME/BSEAV/linux/driver -name bcmdriver.ko -print` $REFSW_REPO_PATH/$OBJ_FOLDER_NAME/rockford/unittests/basemodules/dsp/raaga/$CHIP.$CHIP_VER.debug
		else
			 echo "ERROR cant determine the path of the bcmdriver.ko"
		fi

		sync
	}

raaga_test_video ()
	{
		# Building the raaga_test_video app
		cd $REFSW_REPO_PATH/rockford/unittests/basemodules/dsp/raaga/
		make
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/rockford/unittests/basemodules/dsp/raaga/$CHIP.$CHIP_VER.debug/raaga_test_video" > /dev/null ; then
			 echo "INFO: raaga_test_video has been built successfully"
		else
			 echo "ERROR: raaga_test_video couldn't be built".
		fi
	}

setenv ()
	{
		export BRCM_PLAT_DOT_OVERRIDE=1
		shopt -s  expand_aliases

		# set plat
		echo " INFO: Path of plat script is: $REFSW_REPO_PATH/BSEAV/tools/build/plat "
		source $REFSW_REPO_PATH/BSEAV/tools/build/plat $CHIP $CHIP_VER $CHIP_ARCH $ENDIANNESS

		# set the exports
		echo " INFO: setting the build exports for $CHIP2"
		source $REFSW_REPO_PATH/rockford/unittests/basemodules/dsp/scripts/config_exports $CHIP2
	}

standby ()
	{
		# build standby app
		cd $REFSW_REPO_PATH/BSEAV/app/standby
		make APPS=standby -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/BSEAV/app/standby" > /dev/null ;
		then
			 echo "INFO: standby has been built successfully"
		else
			 echo "ERROR: standby couldn't be built".
		fi
	}

transcode_playback_to_ts_6xaudio ()
	{
		cd $REFSW_REPO_PATH/nexus/examples/encoder
		make APPS=transcode_playback_to_ts_6xaudio -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/transcode_playback_to_ts_6xaudio" > /dev/null ; then
			 echo "INFO: transcode_playback_to_ts_6xaudio has been built successfully"
		else
			 echo "ERROR: transcode_playback_to_ts_6xaudio couldn't be built".
		fi
	}

transcode_ts ()
	{
		# build transcode_ts app
		cd $REFSW_REPO_PATH/BSEAV/app/transcode
		make APPS=transcode_ts -j8
		sleep 2
		if compgen -G "$REFSW_REPO_PATH/$OBJ_FOLDER_NAME/nexus/bin/transcode_ts" > /dev/null ; then
			 echo " INFO: transcode_ts has been built successfully"
		else
			 echo " ERROR: transcode_ts couldn't be built".
		fi
	}



##################################################
# END: Function definition for apps to be built.
##################################################
