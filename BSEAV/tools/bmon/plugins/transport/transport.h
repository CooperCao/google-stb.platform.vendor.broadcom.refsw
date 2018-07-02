/******************************************************************************
* Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
******************************************************************************/
#ifndef TRANSPORT_H
#define TRANSPORT_H

#define TRANSPORT_PLUGIN_NAME        "transport"
#define TRANSPORT_PLUGIN_VERSION     "0.2"
#define TRANSPORT_PLUGIN_DESCRIPTION "Collect transport data"
#define BMON_TRANSPORT_NUM_VIDEO_DECODERS 20

typedef struct bmon_transport_err_params_t
{
    bool cc_err;               // Indicative that packets are not following continuiy count; May be errors in live feed or hdd errors
    bool pusi_err;             //  Payload Unit Start Indicator error :- PB/live stream not following MPEG-2 TS syntax
    bool tei_err;              // Transport Error Indicator : Errors added by demodulators indicating packet corruption
    bool oos_err;              // Out of sync Error : 0x47 is not at every 188 byte interval or DSS sync byte is not following DirecTV protocol
    bool rs_overflow;          // Overflow in RS buffer indicating hang or sudden jump in broadcasting rate (leadin to RTS assumption violation)
    bool cdb_overflow;         // Oveflow in CDB/ITB indicating either decoder hang
    bool itb_overflow;
    bool input_buff_overflow;  // Broadcaster not following defined input peak rate. Need to re-analyze RS Write RTS
} bmon_transport_err_params_t;

// Transport channel related parameters

typedef struct bmon_transport_t
{
    struct
    {
        unsigned char                  ch_status;              // Is the channel ON or OFF
        unsigned char                  live_vs_playback;       // Playback Vs Live

        unsigned char                  pb_full_percentage;
        unsigned char                  cdb_queueDepth;

        unsigned short int             instant_rate;           // Unit is Mbps
        unsigned short int             instant_input_buffer_depth;
        unsigned short int             videoDecoderIndex;
        unsigned short int             pidChannelIndex;

        bmon_transport_err_params_t xpt_err;
    } transport[BMON_TRANSPORT_NUM_VIDEO_DECODERS];

} bmon_transport_t;

int transport_get_data( const char *filter, char *json_output, unsigned int json_output_size );

#endif                                                     // ifndef TRANSPORT_H
