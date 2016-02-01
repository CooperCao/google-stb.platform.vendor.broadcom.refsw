/***************************************************************************
 *     (c)2002-2012 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
 * 
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef CABLECARD_H__
#define CABLECARD_H__

#include "nexus_frontend.h"
#include "nexus_frontend_3255.h"
#include "nexus_mpod.h"
#include "mpod.h"
#include "mpod_util.h"
#include "mpod_apinfo.h"
#include "mpod_diag.h"
#include "mpod_feature.h"
#include "mpod_dsg.h"
#include "mpod_ext_chan.h"
#include "mpod_mmi.h"
#include "mpod_link.h"
#include "mpod_download.h"
#include "mpod_feature.h"
#include "mpod_homing.h"
#include "mpod_hc.h"
#include "mpod_sas.h"
#include "mpod_diag.h"
#include "mpod_mmi.h"
#include "mpod_systime.h"
#include "mpod_ca.h"
#include "mpod_cp.h"
#include "mpod_res.h"
#include "mpod_snmp.h"
#include "test_cp.h"
#include "mpod_resrc_mgr.h"
#include "mpod_headend_comm.h"
#include "mpod_host_property.h"


#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_CABLECARD_ROUTE     6
#define MAX_DSGTUNNEL_HANDLER	6
/*
    1. We are supporting up to 10 pages at this point of time for the mpod diagnostics
    2. The number of lines in a page is limited to 15
    3. The number of chars in a line is limited to 50
*/
#define NUM_CABLECARD_LINES 15
#define NUM_CABLECARD_CHARS	51
#define NUM_CABLECARD_PAGES	10

typedef enum
{
    DSG_CLIENT_TYPE_NULL,
    DSG_CLIENT_TYPE_BROADCAST,
    DSG_CLIENT_TYPE_WELLKNOWN_MACADDR,
    DSG_CLIENT_TYPE_CAS,
    DSG_CLIENT_TYPE_APPLICATION
} DSG_CLIENT_TYPE;

typedef enum
{
    DSG_CLIENT_BROADCAST_TYPE_PROHIBITED,
    DSG_CLIENT_BROADCAST_TYPE_SCTE65,
    DSG_CLIENT_BROADCAST_TYPE_SCTE18,
    DSG_CLIENT_BROADCAST_TYPE_OBJECT_CAROUSEL,
    DSG_CLIENT_BROADCAST_TYPE_OCCD,
    DSG_CLIENT_BROADCAST_TYPE_XAIT_CVT,
    DSG_CLIENT_BROADCAST_TYPE_LAST
} DSG_CLIENT_BROADCAST_TYPE;

typedef struct Cablecard *cablecard_t;

#define MAX_AUDIO_STREAM	4

typedef struct cablecard_program_info {
    unsigned int program_number;
    unsigned int program_index;
    unsigned int source_id;
    unsigned int ltsid;
    unsigned int ca_pid;
    unsigned int video_pid;
    unsigned int audio_pid[MAX_AUDIO_STREAM];
    unsigned char pmt[4096];
    unsigned int pmt_size;
    int video_pidchannel;
    int audio_pidchannel[MAX_AUDIO_STREAM];
    bool valid;
} cablecard_program_info;

typedef struct cablecard_program_info *cablecard_program_info_t;

typedef void (*cablecard_mpeg_callback)(void * context, int param);

typedef void (*cablecard_html_callback)(uint8_t *html, uint16_t len, uint8_t dialogNb, uint8_t fileStatus);

typedef struct cablecard_setting {
    NEXUS_FrontendOutOfBandMode oob_mode;
    NEXUS_FrontendUpstreamMode us_mode;
} cablecard_setting;

typedef struct cablecard_setting *cablecard_setting_t;

typedef struct cablecard_dsg_handler {
    NEXUS_Callback callback;
    unsigned int client_type;
    unsigned int client_id;
} cablecard_dsg_handler;


typedef struct Cablecard {
    int pod;
    NEXUS_FrontendOutOfBandMode oob_mode;
    NEXUS_FrontendUpstreamMode us_mode;
    unsigned char *mpeg_section;
    unsigned int mpeg_section_len;
    cablecard_mpeg_callback si_callback;
    cablecard_html_callback html_callback;
    cablecard_dsg_handler  dsg_handler[MAX_DSGTUNNEL_HANDLER];
    int num_of_dsg_handler;
    bool cablecard_in;
    NEXUS_MpodHandle mpod;
    NEXUS_MpodInputHandle mpod_input[MAX_CABLECARD_ROUTE];
} Cablecard;

/**************************
* cabelcard_information_report
**************************/
typedef struct
{
    /*card Info*/
    unsigned char macaddr[6];
    unsigned ipType;
    unsigned char ipaddr[16];
    unsigned char cardId[8];
    B_MPOD_CP_VALIDATION_STATUS status;
    unsigned openedResrc;
    int timezoneOffset;
    unsigned DSTimeDelta;
    unsigned DSTimeEntry;
    unsigned DSTimeExit;
    char EALocCode[3];
    unsigned vctId;
    /*CP Info*/
    unsigned authKeyStatus;
    unsigned certCheck;
    unsigned cciCount;
    unsigned CPkeyCount;
    unsigned CPId;
    /*SNMP Proxy*/
    unsigned short vendorId;
    unsigned short version;
    char *rootoid;
    unsigned rootoidLen;
    unsigned char serialNum[256];
    unsigned char serialLen;
    /* CableCARD operation mode*/
    B_MPOD_EXT_OP_MODE mode;
    /* CA system ID*/
    unsigned CAId;
    /* extended seesion version*/
    unsigned extSession;
} CableCARDInfo;

cablecard_t cablecard_open(
    cablecard_setting_t setting
    );

void cablecard_close(
    void
    );

#ifdef MPODXSIX
void cablecard_unswitch_parser_band(
    NEXUS_ParserBand parserBand,
    NEXUS_PidChannelHandle *videoPidChannel,
    NEXUS_PidChannelHandle *audioPidChannel,
    NEXUS_PidChannelHandle *pcrPidChannel,
    NEXUS_PidChannelHandle *patPidChannel,
    NEXUS_PidChannelHandle *pmtPidChannel, 
    NEXUS_PidChannelHandle *caPidChannel
    );
NEXUS_PlaypumpHandle cablecard_switch_parser_band(
    NEXUS_ParserBand parserBand
    );
void cablecard_add_pid_channels(
    NEXUS_PlaypumpHandle playpumpHandle,
    NEXUS_PidChannelHandle videoPidChannel,
    NEXUS_PidChannelHandle audioPidChannel,
    NEXUS_PidChannelHandle pcrPidChannel,
    NEXUS_PidChannelHandle patPidChannel,
    NEXUS_PidChannelHandle pmtPidChannel, 
    NEXUS_PidChannelHandle caPidChannel
    );
#endif
int cablecard_go(
    void
    );

int cablecard_route_add_tuner(
    cablecard_t cablecard,
    NEXUS_FrontendHandle frontend
    );

int cablecard_route_remove_tuner(
    cablecard_t cablecard,
    NEXUS_FrontendHandle frontend
    );

int cablecard_inquire_program(
    cablecard_t cablecard,
    cablecard_program_info_t info
    );

int cablecard_enable_program(
    uint8_t* pmt,
    uint32_t pmtSize,
    NEXUS_ParserBand parserBand,
    NEXUS_PidChannelHandle videoPidChannel,
    NEXUS_PidChannelHandle audioPidChannel,
    uint32_t source_id,
    uint16_t *ca_pid);

int cablecard_disable_program(
    NEXUS_ParserBand parserBand
    );

int cablecard_set_mpeg_section_callback(
    cablecard_t cablecard,
    cablecard_mpeg_callback callback
    );

int cablecard_get_mpeg_section(
    cablecard_t cablecard,
    void *buffer,
    size_t size
    );

int cablecard_set_html_callback(
    cablecard_t cablecard,
    cablecard_html_callback callback
    );

int cablecard_set_dsgtunnel_handler(
    cablecard_t cablecard,
    cablecard_dsg_handler  *dsg_handler
    );

int cablecard_get(
    cablecard_t cablecard,
    cablecard_setting_t setting
    );
int cablecard_set(
    cablecard_t cablecard,
    cablecard_setting_t setting
    );

cablecard_t cablecard_get_instance(
    void
    );

void cablecard_init(
    void
    );

int cablecard_get_page(
    int app_num
    );

char *get_cablecard_info(
    int line_num
    );

unsigned int get_num_lines_filled(
    void
    );

CableCARDInfo *cablecard_get_info(
    cablecard_t cablecard
    );

/*hook with DSG-CC lib*/
unsigned char BcmSendDSGTunnelDataToHost(
    unsigned char *pBufData,
    unsigned int pktlen,
    unsigned long client_type,
    unsigned long client_id
    );

unsigned char BcmSendDSGTunnelDataToPOD(
    unsigned char *pBufData,
    unsigned int pktlen,
    unsigned long flow_id
    );

unsigned char BcmSendDataToPOD(
    unsigned char *pBufData,
    unsigned int pktlen,
    unsigned long flow_id
    );

void POD_Api_Lost_Flow_Ind(
    unsigned long id,
    unsigned char status
    );

void POD_Api_Send_DCD_Info(
    void *dcd_ptr,
    unsigned short dcd_len
    );

void POD_Api_Send_DSG_Message(
    void *dsg_message_ptr,
    unsigned short dsg_message_len
    );

void CableCardCallback_DSG_Packet_Error(
    uint8_t *data,
    uint32_t len
    );

void CableCardCallback_Set_DSG_Mode(
    uint8_t *data,
    uint32_t len
    );

void CableCardCallback_DSG_Directory(
    uint8_t *data,
    uint32_t len
    );

extern int openNexusPidChannelsCableCard(B_PlaybackIpPsiInfo *psi, IpStreamerConfig *ipStreamerCfg, IpStreamerCtx *ipStreamerCtx);
void closeNexusPidChannelsCableCard(IpStreamerCtx *ipStreamerCtx);

#ifdef __cplusplus
}
#endif

#endif

