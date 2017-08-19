#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#############################################################################

APPS = \
    ac3_encode \
    audio_capture \
    audio_crc \
    audio_crc_playpump \
    audio_descriptors \
    audio_dual_dsp \
    audio_dualtrack_swap \
    audio_equalizer \
    audio_mixer \
    audio_playback \
    audio_playback_loop \
    audio_playback_upmix \
    audio_track_change \
    dts_encode \
    i2s_input \
    i2s_input_to_playback \
    play_mp3 \
    tru_volume \
    audio_multistream_decode \
    audio_output_group \
    auto_volume_level \
    3d_surround \
    convert_ac3plus_to_ac3 \
    btsc_encode \
    atsc_a85 \
    audio_mux_playback encode_audio \
    audio_nonrealtime_transcode \
    ancillary_data \
    spdif_input \
    audio_capture_multichannel \
    audio_digital_output \
    audio_decoder_suspend \
    audio_playback_suspend \
    audio_playpump \
    audio_playpump_x4 \
    audio_playback_trick \
    audio_karaoke \
    audio_decode_ambisonic \
    audio_decode_fade \
    audio_dsp_selector \
    audio_loudness \
    audio_decode_low_latency \
    audio_multidecode_loopback \
    audio_mixer_input \
    audio_mixer_samplerate_test

ifeq ($(shell test -f ms11_dual.c && echo y),y)
APPS += ms11_dual
endif
ifeq ($(shell test -f ms12_dual.c && echo y),y)
APPS += ms12_dual
endif
ifeq ($(shell test -f dolby_ms11.c && echo y),y)
APPS += dolby_ms11
endif

include ../Makefile.inc
