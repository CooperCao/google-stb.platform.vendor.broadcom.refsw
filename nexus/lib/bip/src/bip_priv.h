/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
***************************************************************************/
#ifndef BIP_PRIV_H
#define BIP_PRIV_H

#include "bip.h"   /* Define the BIP public interface */

#include "b_os_lib.h"
#include "b_playback_ip_lib.h"
#ifdef NEXUS_HAS_ASP
#include "b_asp.h"
#include "b_asp_input.h"
#include "b_asp_output.h"
#endif

#include "bdbg.h"
#include "bkni.h"
#include "berr.h"
#include "bkni_multi.h"
#include "bpool.h"
#include "barena.h"
#include "bioatom.h"
#include "blst_queue.h"
#include "namevalue.h"

#include "nexus_core_utils.h"
#include "nexus_parser_band.h"
#include "nexus_message.h"

/* Include the BIP private includes. */
#include "bip_arb.h"
#include "bip_class.h"

#include "bip_file.h"
#include "bip_console.h"
#include "bip_atom.h"
#include "bip_probe.h"
#include "bip_xml.h"

#include "bip_media_info_priv.h"

#include "bip_rtsp_lm_session.h"
#include "bip_rtsp_lm_socket.h"
#include "bip_rtsp_lm_listener.h"
#include "bip_rtsp_lm_server.h"
#include "bip_rtsp_socket.h"

/* This include file, "bip_priv.h", is responsible for defining the private
   (internal) interfaces to BIP.  This includes the following:
   1. Global datatypes and enums used only within BIP.
   2. Macros used only within BIP
   3. Function prototypes for BIP functions not exposed to the public.
*/


/*************************************************************************
*  Define BIP class-specific macros, datatypes and enums.
**************************************************************************/
/* For example:  #include "bip_url_priv.h"  */
#include "bip_http_server_priv.h"
#include "bip_http_socket_priv.h"
#include "bip_http_streamer_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
*  Define BIP-global message modules.
**************************************************************************/

/* BIP_MSG_SUM (summary) for printing overall summary of BIP activity. */
BDBG_FILE_MODULE(bip_sum);
#define BIP_MSG_SUM(bdbg_args)             BDBG_MODULE_MSG(bip_sum, bdbg_args);

/* BIP_MSG_TRC (trace) for printing trace of BIP activity (more detailed than BIP_MSG_SUM). */
BDBG_FILE_MODULE(bip_trc);
#define BIP_MSG_TRC(bdbg_args)             BDBG_MODULE_MSG(bip_trc, bdbg_args);

/* BIP_MSG_LOG (log) for printing messages regardless of BDBG's levels. */
BDBG_FILE_MODULE(bip_log);
#define BIP_MSG_LOG(bdbg_args)             BDBG_MODULE_LOG(bip_log, bdbg_args);


#ifdef __cplusplus
}
#endif

#endif /* !defined BIP_BASE_PRIV_H */
