/******************************************************************************
 *    (c)2014 Broadcom Corporation
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
 *****************************************************************************/
#ifndef SNMP_H
#define SNMP_H

#include <pthread.h>
#include "bstd.h"
#include "bkni.h"
#include "atlas.h"
#include "atlas_main.h"
#include "b_snmp_lib.h"
#include "snmpProxyIf.h"
#include "avInterfaceTable.h"
#include "cablecard.h"

typedef struct
{
	avIfTypes avType;
	int index;
	bool connected;
} avConnectionStatus;

class CSnmp : public CMvcModel
{
    pthread_t snmp_thread;

public:
    CSnmp();
    ~CSnmp();
    eRet snmp_save_oob(CTunerOob * pTuner);
    eRet snmp_save_upstream(CTunerUpstream * pTuner);
    eRet snmp_save_control(CControl * pControl);
    eRet snmp_init(CModel * pModel);
    eRet snmp_uninit(void);
    bool snmp_check_display_connection(eBoardResource type);
    static void snmp_get_hw_indentifiers(void *context, int param);
    static void snmp_get_ieee1394_table(void *context, int param);
    static void snmp_get_ieee1394_connected_devices_table(void *context, int param);
    static void snmp_get_dvi_hdmi_table(void *context, int param);
    static void snmp_get_dvi_hdmi_available_video_format_table(void *context, int param);
    static void snmp_get_component_video_table(void *context, int param);
    static void snmp_get_rf_channel_out_table(void *context, int param);
    static void snmp_get_in_band_tuner_table(void *context, int param);
    static void snmp_get_analog_video_table(void *context, int param);
    static void snmp_get_mpeg2_content_table(void *context, int param);
    static void snmp_get_program_status_table(void *context, int param);
    static void snmp_get_qpsk_objects(void *context, int param);
    static void snmp_get_spdif_table(void *context, int param);
    static void snmp_get_eas_codes(void *context, int param);
    static void snmp_get_device_software_base(void *context, int param);
    static void snmp_get_fw_download_status(void *context, int param);
    static void snmp_get_sw_app_info(void *context, int param);
    static void snmp_get_sw_app_info_table(void *context, int param);
    static void snmp_get_security_subsystem(void *context, int param);
    static void snmp_get_power(void *context, int param);
    static void snmp_get_user_settings(void *context, int param);
    static void snmp_get_system_memory_report_table(void *context, int param);
    static void snmp_get_card_info(void *context, int param);
    static void snmp_get_card_cp_info(void *context, int param);
    static void snmp_get_cc_app_info_table(void *context, int param);
    static void snmp_get_snmpproxy_info(void *context, int param);
    static void snmp_set_snmpproxy_info(void *context, int param);
    static void snmp_get_host_info(void *context, int param);
    static void snmp_get_dump_trap_info(void *context, int param);
    static void snmp_set_dump_trap_info(void *context, int param);
    static void snmp_get_spec_info(void *context, int param);
    static void snmp_get_content_error_summary_info(void *context, int param);
    static void snmp_set_reboot_info(void *context, int param);
    static void snmp_get_reboot_info(void *context, int param);
    static void snmp_get_jvm_info(void *context, int param);
    void snmp_proxy_mibs_open(void);
    void snmp_proxy_mibs_close(void);
    void snmp_proxy_mibs_register(void);
    void snmp_proxy_mibs_unregister(void);

protected:
    CModel * _pModel;
};

#ifdef __cplusplus
extern "C"
{
#endif

int B_SNMP_Main(void);

#ifdef __cplusplus
}
#endif

#endif
