/***************************************************************************
 *     Copyright (c) 2002-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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

#include "tspsimgr.h"
#include "atlas_cfg.h"
#include "atlas.h"
#include "mvc.h"
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

class CTunerOob;
class CTunerUpstream;
class CParserBand;
class CChannel;
class CChannelMgr;
class CModel;

#define MAX_MPEG_SECTION_LEN 4096
#define MAX_AUDIO_STREAM 4
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

#define CALLBACK_CABLECARD_IN    "CallbackCableCardIn"
#define CALLBACK_CABLECARD_OUT   "CallbackCableCardOut"
#define CALLBACK_CHANNEL_MAP_UPDATE    "CallbackChannelMapUpdate"
#define CALLBACK_CHANNEL_SCAN_START    "CallbackChannelScanStart"

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
}Cablecard;

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
	unsigned long *rootoid;
	unsigned rootoidLen;
	unsigned char serialNum[256];
	unsigned char serialLen;
	/* CableCARD operation mode*/
	B_MPOD_EXT_OP_MODE mode;
	/* CA system ID*/
	unsigned CAId;
	/* extended seesion version*/
	unsigned extSession;
	char *card_mmi_message1;
	char *card_mmi_message2;
	char *card_mmi_message3;
	char *card_mmi_message4;
	int mmi_message_length;
} CableCARDInfo;

class CCablecard : public CMvcModel{
    bool cableCardIn;
    bool chMgrUninitialized;
public:
    CCablecard(void);
    ~CCablecard(void);
    eRet initialize(CTunerOob * pTuner);
    eRet initializeUpstream(CTunerUpstream * pTuner);
    cablecard_t cablecard_open(cablecard_setting_t setting, CChannelMgr * pChannelMgr, CModel *  pModel);
    void cablecard_close();
    int cablecard_go(cablecard_t cablecard);
	CableCARDInfo  * cablecard_get_info();
    int cablecard_route_add_tuner(cablecard_t cablecard, CChannel *pChannel);
    int cablecard_route_remove_tuner(cablecard_t cablecard, CChannel *pChannel);
    int cablecard_inquire_program(cablecard_t cablecard, CChannel *pChannel);
    int cablecard_enable_program(CChannel *pChannel);
    int cablecard_disable_program(CChannel *pChannel);
    int cablecard_set_mpeg_section_callback(cablecard_t cablecard, cablecard_mpeg_callback callback);
    int cablecard_set_html_callback(cablecard_t cablecard, cablecard_html_callback callback);
	int cablecard_set_dsgtunnel_handler(cablecard_t cablecard,  cablecard_dsg_handler  *dsg_handler);
    int cablecard_get(cablecard_t cablecard, cablecard_setting_t setting);
    int cablecard_set(cablecard_t cablecard, cablecard_setting_t setting);
    void cablecard_init();
    int cablecard_get_page(int app_num);
    unsigned int get_num_lines_filled(void);
    void parse_app_info_query(uint8_t *html, uint16_t len);
    friend void cablecard_in(void);
    friend void cablecard_out(void);
    int bcm_3255_tristate_oob_pins();
    int bcm_3255_enable_oob_pins();
    void getProperties(MStringHash * pProperties, int page, bool bClear = true);
    NEXUS_FrontendLockStatus isLocked(void);
    CParserBand * getParserBand(void) { return _pParserBand; }; 
#ifdef SNMP_SUPPORT
    friend void enforceCciCb(uint8_t cci_data, uint16_t prog_num, uint8_t cci_ltsid);
#endif
protected:
    CParserBand         * _pParserBand;
    CModel              * _pModel;
	CChannel            * _pChannel;
};
int cablecard_get_mpeg_section(cablecard_t cablecard, void *buffer, size_t size);
cablecard_t cablecard_get_instance(void);

/*hook with DSG-CC lib*/
#ifdef __cplusplus
extern "C"
{
#endif

unsigned char BcmSendDSGTunnelDataToHost( unsigned char *pBufData, unsigned int pktlen, unsigned long client_type, unsigned long client_id);
unsigned char BcmSendDSGTunnelDataToPOD( unsigned char *pBufData, unsigned int pktlen, unsigned long flow_id );
unsigned char BcmSendDataToPOD( unsigned char *pBufData, unsigned int pktlen, unsigned long flow_id );
void POD_Api_Lost_Flow_Ind(unsigned long id,unsigned char status);
void POD_Api_Send_DCD_Info(void *dcd_ptr, unsigned short dcd_len);
void POD_Api_Send_DSG_Message(void *dsg_message_ptr, unsigned short dsg_message_len);
void CableCardCallback_DSG_Packet_Error(uint8_t *data, uint32_t len);
void CableCardCallback_Set_DSG_Mode(uint8_t *data, uint32_t len);
void CableCardCallback_DSG_Directory(uint8_t *data, uint32_t len);


#ifdef __cplusplus
}
#endif

#endif
