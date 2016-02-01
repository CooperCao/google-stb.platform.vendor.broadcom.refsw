/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 *  ip applib test file
 * 
 * Revision History:
 * 
 * $brcm_Log: 
 * 
 ******************************************************************************/
#ifndef IP_APPLIB_TEST_H__
#define IP_APPLIB_TEST_H__
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/time.h>
#ifdef __mips__
#include <asm/cachectl.h>
#endif

#include "bstd.h"
#include "bkni.h"
#include "blst_list.h"
#include "blst_queue.h"

#include "nexus_platform.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_parser_band.h"

#include "nexus_video_decoder.h"
#include "nexus_video_image_input.h"
#if NEXUS_STILL_DECODER
#include "nexus_still_decoder.h"
#endif
#if B_HAS_ANALOG
#include "nexus_analog_video_decoder.h"
#endif
#include "nexus_audio_decoder.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#include "nexus_svideo_output.h"
#include "nexus_surface.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_message.h"
#include "nexus_stc_channel.h"
#include "nexus_timebase.h"
#include "nexus_audio_output.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_dac.h"
#include "nexus_spdif_output.h"
#include "nexus_i2s_output.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#include "nexus_parser_band.h"
#include "nexus_video_input.h"
#include "nexus_audio_input.h"
#include "nexus_display_vbi.h"
#include "nexus_video_adj.h"
#include "nexus_picture_ctrl.h"
#include "nexus_ccir656_input.h"
#include "nexus_video_input_crc.h"
#include "nexus_core_utils.h"
#include "nexus_audio_decoder_trick.h"
#include "b_playback_ip_lib.h"
#ifdef B_HAS_NETACCEL_SUPPORT
#include "bnetaccel_info.h"
#endif

#ifndef _WIN32_WCE
   #ifndef B_SYSTEM_linuxkernel
     #include <sys/socket.h>
     #include <netinet/in.h>
     #include <arpa/inet.h>
   #endif
   #ifdef __vxworks
     #include <sockLib.h>
     #include <sysLib.h>
#ifndef VXWORKS6
     typedef int socklen_t;
#endif
       #endif
#else
       #include <winsock2.h>
       #include <ws2tcpip.h>
#endif

#ifdef B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#endif
#include "b_os_lib.h"

#ifdef B_AUTO_PSI_SUPPORT
#include "b_psip_lib.h"
#endif
#include "nexus_core_utils.h"
#include "nexus_audio_decoder_trick.h"

#endif /* IP_APPLIB_TEST_H__*/
