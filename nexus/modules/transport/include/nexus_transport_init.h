/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
 **************************************************************************/
#ifndef NEXUS_TRANSPORT_INIT_H__
#define NEXUS_TRANSPORT_INIT_H__

#include "nexus_types.h"
#include "nexus_transport_capabilities.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_TransportModuleSettings
{
    NEXUS_CommonModuleSettings common;
    unsigned mainHeapIndex;      /* Heap index used by XPT to allocate most buffers including RAVE CDB/ITB, playback, record and message buffers. */
    NEXUS_ModuleHandle dma;      /* Optional. Dma module is used for security.
                                    This handle can be set after NEXUS_TransportModule_Init using NEXUS_TransportModule_SetDmaModule. */
    NEXUS_ModuleHandle core;     /* Required */
    NEXUS_HeapHandle secureHeap; /* Optional. if set, transport will allocate all RAVE contexts from this heap. */

    bool dssSaveMptFlag; /* if true, the DSS MPT flag will be put in the front of the reconstructed message.
                            this is a global setting. */
    bool initVcxos;      /* If true, initialize the vcxo-pll rate managers.  If false, initialization must be done outside (e.g. platform layer) */
    bool postInitCalledBySecurity; /* if true, NEXUS_TransportModule_PostInit and _PostStandby will be called by security. */

    /* global TPIT filter settings. see NEXUS_RecpumpSettings.tpit for per-record TPIT settings. */
    struct {
        unsigned idleEventTimeout;   /* Number of 108 MHz clocks before triggering a record event timeout. See NEXUS_RecpumpSettings.tpit.idleEventEnabled. */
        unsigned recordEventTimeout; /* Number of 108 MHz clocks before triggering a record packet timeout. See NEXUS_RecpumpSettings.tpit.recordEventEnabled. */
    } tpit;

    struct {
        unsigned parserBand[NEXUS_MAX_PARSER_BANDS]; /* maximum data rate of RS and XC buffers in units of bits per second */
        unsigned playback[NEXUS_MAX_PLAYPUMPS]; /* maximum data rate of XC buffers in units of bits per second */
    } maxDataRate; /* this replaces NEXUS_ParserBandSettings.maxDataRate (65nm only) */

    struct {
        bool enabled;
        unsigned rxInterfaceWidth; /* width of MTSIF bus. supported values are 1, 2, 4 and 8 bits. */
    } mtsif[NEXUS_MAX_MTSIF];

    /* Describes which transport components will receive data from each input and playback parser. Applications can reduce the
       amount of memory required by setting unused paths to false. */
    struct {
        struct {
            bool rave; /* decode/record */
            bool message;
            bool remux[NEXUS_MAX_REMUX];
            bool mpodRs; /* if true, data is routed to MPOD RS buffer first, then to MPOD (if enabled). this affects RTS.
                            if false, data is routed directly to MPOD (if enabled) */
        } parserBand[NEXUS_MAX_PARSER_BANDS];
        struct {
            bool rave; /* decode/record */
            bool message;
            bool remux[NEXUS_MAX_REMUX];
        } playback[NEXUS_MAX_PLAYPUMPS];
    } clientEnabled;
    
    unsigned syncInCount; /* see BXPT_DefaultSettings for details */
    unsigned syncOutCount;
} NEXUS_TransportModuleSettings;

void NEXUS_TransportModule_GetDefaultSettings(
    NEXUS_TransportModuleSettings *pSettings /* [out] */
    );

/**
Summary:
Initialize the transport module in two stages

Description:
This function is called by NEXUS_Platform_Init, not by applications.
If you want to modify these settings from your application, you can do this 
through NEXUS_PlatformSettings as follows:

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.transportModuleSettings.xxx = xxx;
    NEXUS_Platform_Init(&platformSettings);
    
Transport must be initialized in two stages so that RAVE FW can be verified by the security module.
PreInit creates the Nexus module, PostInit calls BXPT_Open and initializes the hardware.
**/
NEXUS_ModuleHandle NEXUS_TransportModule_PreInit(
    const NEXUS_TransportModuleSettings *pSettings
    );

void NEXUS_TransportModule_Uninit(void);

#ifdef __cplusplus
}
#endif

#endif
