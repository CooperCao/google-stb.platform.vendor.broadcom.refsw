/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include "nexus_security_module.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"
#include "nexus_power_management.h"

#include "bhsm.h"
#include "bsp_s_commands.h"
#include "bsp_s_misc.h"
#include "bsp_s_hw.h"
#include "bsp_s_keycommon.h"
#include "bhsm_keyladder.h"

#include "nexus_base.h"
#include "nexus_pid_channel.h"
#include "priv/nexus_pid_channel_priv.h"
#include "priv/nexus_transport_priv.h"

#if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(1,0)
#if NEXUS_SECURITY_IPLICENSING
#include "bhsm_ip_licensing.h"
#endif
#include "priv/nexus_security_standby_priv.h"
#endif

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
#include "bhsm_exception_status.h"
#endif


BDBG_MODULE(nexus_security);

/* Feature defines.  There should be NO #if BCHP_CHIP macros controlling features.
 * Instead, maintain the chip/version list here, and switch features via these defines. */


/* DigitalTV chip like 3563 has only ONE HSM channel, no cancel cmd support */
#if (BCHP_CHIP!=3563)
#define HAS_TWO_HSM_CMD_CHANNELS 1
#endif

/* This define controls whether to include the AES, CSS, and C2 remapping in the functions.
 * The 3563 and 3548/3556 HSM PI has commented certain enums out, which has a ripple effect
 * in generic code. */
#if (BCHP_CHIP != 3563) && (BCHP_CHIP != 3548) && (BCHP_CHIP != 3556)
#define SUPPORT_NON_DTV_CRYPTO 1
#endif

/* A change requires AES to be broken out for these chips */
#if (BCHP_CHIP == 3548) || (BCHP_CHIP == 3556)
#define SUPPORT_AES_FOR_DTV 1
#endif

#if ((BCHP_CHIP == 7420) && (BCHP_VER >= BCHP_VER_A1)) || (BCHP_CHIP == 7340) || \
     (BCHP_CHIP == 7342) || (BCHP_CHIP == 7125) || (BCHP_CHIP == 7468)
#define HAS_TYPE7_KEYSLOTS 1
#endif


#if !HSM_IS_ASKM_40NM
#define HAS_TYPE5_KEYSLOTS 1
#include "bhsm_misc.h"
#endif

#if HSM_IS_ASKM_28NM_ZEUS_4_0
#define HAS_TYPE5_KEYSLOTS 1
#endif


#define MIN(a,b) ((a)<(b)?(a):(b))


#include "blst_slist.h"

/* structure to capture the pidChannels that are associated with a keySlot */
typedef struct NEXUS_Security_P_PidChannelListNode {
    BLST_S_ENTRY(NEXUS_Security_P_PidChannelListNode) next;
    NEXUS_PidChannelHandle pidChannel;
    unsigned long pidChannelIndex; /* only valid if pidChannel is NULL */
} NEXUS_Security_P_PidChannelListNode;

/* Structre to capture the private data associated with a KeySlot. */
typedef struct NEXUS_Security_P_KeySlotData {
    BLST_S_HEAD(NEXUS_Security_P_PidChannelList_t, NEXUS_Security_P_PidChannelListNode) pidChannelList;
    NEXUS_SecurityAlgorithm algorithm;
    NEXUS_SecurityClientType keySlotClient;
    NEXUS_PidChannelHandle dmaPidChannelHandle; /* Security module needs to allocate a pid channel for each M2M keyslot. */
} NEXUS_Security_P_KeySlotData;

#if 0
#define NEXUS_SECURITY_DUMP_KEYSLOTS \
    do { \
        NEXUS_KeySlotHandle keyslot; \
        for (keyslot=BLST_S_FIRST(&g_security.keyslotList);keyslot;keyslot=BLST_S_NEXT(keyslot,next)) { \
            NEXUS_Security_P_KeySlotData *pKeySlotData = keyslot->security.data; \
            NEXUS_Security_P_PidChannelListNode *pPidChannelNode; \
            BDBG_MSG(("%s: Keyslot [%p]",__FUNCTION__,keyslot)); \
            for (pPidChannelNode=BLST_S_FIRST(&pKeySlotData->pidChannelList);pPidChannelNode!=NULL;pPidChannelNode=BLST_S_NEXT(pPidChannelNode,next)) { \
                BDBG_MSG(("%s:   pidchannel [%d]",__FUNCTION__,keyslot,pPidChannelNode->pidChannelIndex)); \
            } \
        } \
    } while(0)
#else
#define NEXUS_SECURITY_DUMP_KEYSLOTS
#endif

NEXUS_ModuleHandle NEXUS_P_SecurityModule = NULL;
static struct {
    NEXUS_SecurityModuleSettings settings;
    NEXUS_SecurityModuleInternalSettings moduleSettings;
    BHSM_Handle hsm;
    #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(1,0)
    BHSM_ChannelHandle hsmChannel[BHSM_HwModule_eMax];
    #endif
    BLST_S_HEAD(NEXUS_Security_P_KeySlotList_t, NEXUS_KeySlot) keyslotList; /* cannot contain any "generic" keyslots */
} g_security;

static BERR_Code NEXUS_Security_P_InitHsm(const NEXUS_SecurityModuleSettings * pSettings);
static void NEXUS_Security_P_UninitHsm(void);

/*
 * Helper functions
 */

static void NEXUS_Security_P_RemovePidchannel(NEXUS_KeySlotHandle keyslot, NEXUS_Security_P_PidChannelListNode *pPidChannelNode)
{
    NEXUS_Security_P_KeySlotData *pKeySlotData = keyslot->security.data;

    BLST_S_REMOVE( &pKeySlotData->pidChannelList, pPidChannelNode, NEXUS_Security_P_PidChannelListNode, next );

    BKNI_Free( pPidChannelNode );

    return;
}

/****************************************
 * Module functions
 ****************************************/

void NEXUS_GetSecurityCapabilities( NEXUS_SecurityCapabilities *pCaps )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
    BERR_Code rc = BERR_SUCCESS;
    BHSM_Capabilities_t hsmCaps;
    unsigned x;
    unsigned numTypes;

    BDBG_ENTER(NEXUS_GetSecurityCapabilities);

    if( pCaps == NULL )
    {
        /* illegal argument  */
        (void)BERR_TRACE( NEXUS_INVALID_PARAMETER );
        return;
    }

    BKNI_Memset( pCaps, 0, sizeof(*pCaps) );
    BKNI_Memset( &hsmCaps, 0, sizeof(hsmCaps) );

    if( ( rc = BHSM_GetCapabilities( g_security.hsm, &hsmCaps ) ) != BERR_SUCCESS )
    {
        BDBG_ERR(("BHSM_GetCapabilities failed [%d]", rc ));
        return;
    }

    pCaps->version.zeus = NEXUS_ZEUS_VERSION_CALC ( NEXUS_SECURITY_ZEUS_VERSION_MAJOR, NEXUS_SECURITY_ZEUS_VERSION_MINOR );

    pCaps->version.firmware =  ( ( ( hsmCaps.version.firmware.bseck.major & 0xFF ) << 16 )
                                |  ( hsmCaps.version.firmware.bseck.minor & 0xFF ) );

    numTypes = hsmCaps.keyslotTypes.numKeySlotTypes;
    numTypes = MIN( numTypes , sizeof( pCaps->keySlotTableSettings.numKeySlotsForType ) );
    numTypes = MIN( numTypes , sizeof( hsmCaps.keyslotTypes.numKeySlot ) );

    for( x = 0; x < numTypes; x++)
    {
        pCaps->keySlotTableSettings.numKeySlotsForType[x] = hsmCaps.keyslotTypes.numKeySlot[x];
    }
    pCaps->keySlotTableSettings.numMulti2KeySlots = hsmCaps.keyslotTypes.numMulti2KeySlots;

#else
    if( pCaps )
    {
        BKNI_Memset( pCaps, 0, sizeof(*pCaps) );
    }
    BERR_TRACE( NEXUS_NOT_SUPPORTED );
#endif

    BDBG_LEAVE(NEXUS_GetSecurityCapabilities);
    return;
}


/*
Description:
    Function will itterate over all ARCHes of each available MEM Controller and print any detected violations.
*/
void NEXUS_Security_PrintArchViolation_priv( void )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BERR_Code  magnumRc;
    BHSM_ExceptionStatusRequest_t request;
    BHSM_ExceptionStatus_t status;
    BCHP_MemoryInfo memInfo;
    BHSM_Handle hHsm;
    unsigned maxMemc = 0;
    unsigned memcIndex;
    unsigned archIndex;

    BDBG_ENTER(NEXUS_Security_PrintArchViolation_priv);
    NEXUS_ASSERT_MODULE();

    NEXUS_Security_GetHsm_priv( &hHsm );
    if(!hHsm )
    {
        BERR_TRACE( NEXUS_INVALID_PARAMETER );
        return;
    }

    BKNI_Memset( &memInfo, 0, sizeof( memInfo ) );
    if( ( magnumRc = BCHP_GetMemoryInfo( g_pCoreHandles->reg, &memInfo ) ) !=  BERR_SUCCESS )
    {
        BERR_TRACE( magnumRc );
        return;
    }

    maxMemc = sizeof(memInfo.memc)/sizeof(memInfo.memc[0]);

    BKNI_Memset( &request, 0, sizeof(request) );

    request.deviceType = BHSM_ExceptionStatusDevice_eMemcArch;
    request.keepStatus = false;

    for( memcIndex = 0; memcIndex < maxMemc; memcIndex++ )  /* iterate over mem controllers. */
    {
        if( memInfo.memc[memcIndex].size > 0 )              /* if the MEMC is in use. */
        {
            request.u.memArch.memcIndex = memcIndex;

            for( archIndex = 0; archIndex < BHSM_MAX_ARCH_PER_MEMC; archIndex++ )   /* itterate over ARCHes. */
            {
                request.u.memArch.archIndex = archIndex;

                magnumRc = BHSM_GetExceptionStatus( hHsm, &request, &status );
                if( magnumRc != BERR_SUCCESS )
                {
                    if(magnumRc != BERR_NOT_SUPPORTED) {
                        magnumRc = BERR_TRACE( magnumRc );
                    }
                    return;
                }

                if( status.u.memArch.endAddress ) /* if there has been a violation */
                {
                    BDBG_ERR(("MEMC ARCH Violation. MEMC[%u]ARCH[%u] Addr start [" BDBG_UINT64_FMT "] end[" BDBG_UINT64_FMT "] numBlocks[%u] scbClientId[%u:%s] requestType[%#x:%s]",
                               memcIndex, archIndex,
                               BDBG_UINT64_ARG(status.u.memArch.startAddress), BDBG_UINT64_ARG(status.u.memArch.endAddress),
                               status.u.memArch.numBlocks, status.u.memArch.scbClientId, BMRC_Checker_GetClientName(memcIndex, status.u.memArch.scbClientId), status.u.memArch.requestType, BMRC_Monitor_GetRequestTypeName_isrsafe(status.u.memArch.requestType) ));
                }
            }
        }
    }

    BDBG_LEAVE(NEXUS_Security_PrintArchViolation_priv);
#endif /* BHSM_ZEUS_VERSION .. */

    return;
}

void NEXUS_SecurityModule_GetDefaultInternalSettings(NEXUS_SecurityModuleInternalSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}


void NEXUS_SecurityModule_GetDefaultSettings(NEXUS_SecurityModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    pSettings->common.enabledDuringActiveStandby = true;

    /* defaults number of keyslots per type. */
    pSettings->numKeySlotsForType[0] = 20;
    pSettings->numKeySlotsForType[1] = 10;
    pSettings->numKeySlotsForType[2] = 15;
    pSettings->numKeySlotsForType[3] = 25;
    pSettings->numKeySlotsForType[4] = 7;
    pSettings->numKeySlotsForType[5] = 2;
    pSettings->numKeySlotsForType[6] = 8;
    #if HAS_TYPE7_KEYSLOTS
    pSettings->numKeySlotsForType[7] = 4;
    #endif

    /*exceptions*/
#if HSM_IS_ASKM_40NM
    #if (BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(3,0))
    pSettings->numKeySlotsForType[3] = 0;
    #endif

    #if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0))
    pSettings->numKeySlotsForType[5] = 0;
    #endif

    #if (BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(1,0))
    pSettings->numKeySlotsForType[1] = 0;
    pSettings->numKeySlotsForType[4] = 0;
    pSettings->numKeySlotsForType[5] = 0;
    pSettings->numKeySlotsForType[6] = 0;
    #endif

  #if NEXUS_HAS_NSK2HDI
    BKNI_Memset(pSettings->numKeySlotsForType, 0, sizeof(pSettings->numKeySlotsForType));
    #if (BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(1,0))
    /* Zeus 1 layout */
    pSettings->numKeySlotsForType[0]= 0;
    pSettings->numKeySlotsForType[1]= 57;
    pSettings->numKeySlotsForType[2]= 0;
    pSettings->numKeySlotsForType[3]= 0;
    pSettings->numKeySlotsForType[4]= 1;
    pSettings->numKeySlotsForType[5]= 0;
    #elif (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0))
    /* Zeus 2/Zeus 3 layout */
    pSettings->numKeySlotsForType[0]= 0;
    pSettings->numKeySlotsForType[1]= 116;
    pSettings->numKeySlotsForType[2]= 0;
    pSettings->numKeySlotsForType[3]= 0;
    pSettings->numKeySlotsForType[4]= 1;
    pSettings->numKeySlotsForType[5]= 0;
    #else
    /* Zeus 4 layout */
    pSettings->numKeySlotsForType[0]= 0;
    pSettings->numKeySlotsForType[1]= 32;
    pSettings->numKeySlotsForType[2]= 0;
    pSettings->numKeySlotsForType[3]= 52;
    pSettings->numKeySlotsForType[4]= 1;
    pSettings->numKeySlotsForType[5]= 51;
    #endif
  #endif /* NEXUS_HAS_NSK2HDI */

#endif /* HSM_IS_ASKM_40NM */
    return;
}

#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0))
/* Function to authenticate/verify rave firmware. */
static NEXUS_Error secureFirmwareRave( void )
{
    NEXUS_Error rc;
    NEXUS_SecurityRegionConfiguration regionConfig;
    void *pRaveFirmware = NULL;
    unsigned raveFirmwareSize = 0;
    BDBG_ENTER(secureFirmwareRave);

    NEXUS_Security_RegionGetDefaultConfig_priv( NEXUS_SecurityRegverRegionID_eRave, &regionConfig );
    /* use defaults. */
    rc = NEXUS_Security_RegionConfig_priv( NEXUS_SecurityRegverRegionID_eRave, &regionConfig );
    if( rc ) { return BERR_TRACE(rc); }

    /* locate Rave Firmware. */
    NEXUS_TransportModule_GetRaveFirmware_isrsafe( &pRaveFirmware, &raveFirmwareSize );

    rc = NEXUS_Security_RegionVerifyEnable_priv( NEXUS_SecurityRegverRegionID_eRave, pRaveFirmware, raveFirmwareSize );
    if (rc) { return BERR_TRACE(rc); }

    BDBG_LEAVE(secureFirmwareRave);
    return BERR_SUCCESS;
}
#endif

NEXUS_ModuleHandle NEXUS_SecurityModule_Init(const NEXUS_SecurityModuleInternalSettings *pModuleSettings, const NEXUS_SecurityModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    BERR_Code rc;

    BDBG_ENTER(NEXUS_SecurityModule_Init);

    BDBG_ASSERT(!NEXUS_P_SecurityModule);

    BDBG_ASSERT(pSettings);

#if NEXUS_HAS_XPT_DMA
    if (pModuleSettings->transport==NULL) {
        /* security module requires transport module */
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
#endif

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eLow, &pSettings->common);
    NEXUS_P_SecurityModule = NEXUS_Module_Create("security", &moduleSettings);
    if (!NEXUS_P_SecurityModule) {
        return NULL;
    }

    g_security.settings = *pSettings;
    g_security.moduleSettings = *pModuleSettings;

    NEXUS_LockModule();
    rc = NEXUS_Security_P_InitHsm(pSettings);
    if (rc) {rc = BERR_TRACE(rc); goto err_init;}

    BLST_S_INIT( &g_security.keyslotList );

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
    rc = NEXUS_Security_RegionVerification_Init_priv( );
    if (rc) { rc = BERR_TRACE(rc); goto err_init; }
    #endif

    if (pModuleSettings->callTransportPostInit) {
        NEXUS_Module_Lock(g_security.moduleSettings.transport);

        rc = NEXUS_TransportModule_PostInit_priv(
           #if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0))
            secureFirmwareRave
           #else
            NULL
           #endif
        );
        NEXUS_Module_Unlock(g_security.moduleSettings.transport);
        if (rc) {
            rc = BERR_TRACE(rc);
            goto err_transport_postinit;
        }
    }

	/* IP licensing is supported for Zeus1.0 and newer. */
    #if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(1,0)
    #if NEXUS_SECURITY_IPLICENSING
    if( pSettings->ipLicense.valid )
    {
      #if NEXUS_SECURITY_IPLICENSING
		BHSM_Handle     hHsm;
        BHSM_ipLicensingOp_t ipLicensingOp;

		NEXUS_Security_GetHsm_priv ( &hHsm );

		if ( !hHsm )
		{
			BERR_TRACE(NEXUS_NOT_AVAILABLE);
			goto err_init;
		}

        BKNI_Memset( &ipLicensingOp, 0, sizeof(ipLicensingOp) );

		ipLicensingOp.dataSize = NEXUS_SECURITY_IP_LICENCE_SIZE;

		BKNI_Memcpy ( ipLicensingOp.inputBuf, pSettings->ipLicense.data, NEXUS_SECURITY_IP_LICENCE_SIZE );

        if ( BHSM_SetIpLicense ( hHsm, &ipLicensingOp ) != BERR_SUCCESS )
        {
			BERR_TRACE(NEXUS_NOT_SUPPORTED);
			goto err_init;
		}
      #else
       BDBG_WRN(("IP Licensing not enabled in build."));
      #endif
    }
    #endif
    #endif

    NEXUS_UnlockModule();

    return NEXUS_P_SecurityModule;

err_transport_postinit:
err_init:
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(NEXUS_P_SecurityModule);
    NEXUS_P_SecurityModule = NULL;

    BDBG_LEAVE(NEXUS_SecurityModule_Init);
    return NULL;
}

void NEXUS_SecurityModule_GetCurrentSettings(NEXUS_ModuleHandle module, NEXUS_SecurityModuleSettings *pSettings)
{
    BSTD_UNUSED(module);
    *pSettings = g_security.settings;
}

void NEXUS_SecurityModule_Uninit(void)
{
    BDBG_ENTER(NEXUS_SecurityModule_Uninit);

    NEXUS_LockModule();

    NEXUS_SecurityModule_Sweep_priv();

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
    NEXUS_Security_RegionVerification_UnInit_priv( );
    #endif

    NEXUS_Security_P_UninitHsm();

    NEXUS_UnlockModule();

    NEXUS_Module_Destroy(NEXUS_P_SecurityModule);
    NEXUS_P_SecurityModule = NULL;

    BDBG_LEAVE(NEXUS_SecurityModule_Uninit);
    return;
}

static BERR_Code NEXUS_Security_P_InitHsm(const NEXUS_SecurityModuleSettings * pSettings)
{
    BHSM_Settings hsmSettings;
    BHSM_InitKeySlotIO_t keyslot_io;
    BERR_Code rc;
    #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(1,0)
    BHSM_ChannelSettings hsmChnlSetting;
    #endif

    BDBG_ENTER(NEXUS_Security_P_InitHsm);
    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, true);

    BHSM_GetDefaultSettings(&hsmSettings, g_pCoreHandles->chp);
#if HSM_IS_ASKM_40NM
    hsmSettings.hHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mem;
#endif

    #if NEXUS_HAS_SAGE
    hsmSettings.sageEnabled = true;
    #endif

    rc = BHSM_Open(&g_security.hsm, g_pCoreHandles->reg, g_pCoreHandles->chp, g_pCoreHandles->bint, &hsmSettings);
    if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

#if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(1,0)
    rc = BHSM_GetChannelDefaultSettings(g_security.hsm, BHSM_HwModule_eCmdInterface1, &hsmChnlSetting);
    if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

    #if HAS_TWO_HSM_CMD_CHANNELS
    rc = BHSM_Channel_Open(g_security.hsm, &g_security.hsmChannel[BHSM_HwModule_eCmdInterface1], BHSM_HwModule_eCmdInterface1, &hsmChnlSetting);
    if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }
    #endif

    rc = BHSM_Channel_Open(g_security.hsm, &g_security.hsmChannel[BHSM_HwModule_eCmdInterface2], BHSM_HwModule_eCmdInterface2, &hsmChnlSetting);
    if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }
#endif

    BKNI_Memset(&keyslot_io, 0, sizeof(keyslot_io));

    keyslot_io.unKeySlotType0Num = pSettings->numKeySlotsForType[0];
    keyslot_io.unKeySlotType1Num = pSettings->numKeySlotsForType[1];
    keyslot_io.unKeySlotType2Num = pSettings->numKeySlotsForType[2];
    keyslot_io.unKeySlotType3Num = pSettings->numKeySlotsForType[3];
    keyslot_io.unKeySlotType4Num = pSettings->numKeySlotsForType[4];
#if HAS_TYPE5_KEYSLOTS
    keyslot_io.unKeySlotType5Num = pSettings->numKeySlotsForType[5];
#if !HSM_IS_ASKM_28NM_ZEUS_4_0
    keyslot_io.unKeySlotType6Num = pSettings->numKeySlotsForType[6];
#endif
#if HAS_TYPE7_KEYSLOTS
    keyslot_io.unKeySlotType7Num = pSettings->numKeySlotsForType[7];
#endif
#endif

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)

    keyslot_io.bMulti2SysKey     = pSettings->enableMulti2Key;
    keyslot_io.numMulti2KeySlots = pSettings->numMulti2KeySlots;

    /* For API backward compatability */
    if( keyslot_io.bMulti2SysKey && (keyslot_io.numMulti2KeySlots == 0) )
    {
        keyslot_io.numMulti2KeySlots = BCMD_MULTI2_MAXSYSKEY;
    }

    #endif

    /* Disregard errors, as this can only be run once per board boot. */
    rc = BHSM_InitKeySlot(g_security.hsm, &keyslot_io);
    /* Print out warning and ignore the error */
    if (rc)
    {
        BDBG_WRN(("**********************************************"));
        BDBG_WRN(("If you see this warning and the HSM errors above, you need to perform some"));
        BDBG_WRN(("board reconfiguration. This is not required. If you want, you can ignore them."));
        BDBG_WRN(("Use BBS to check if BSP_GLB_NONSECURE_GLB_IRDY = 0x07."));
        BDBG_WRN(("If not, BSP/Aegis is not ready to accept a command."));
        BDBG_WRN(("SUN_TOP_CTRL_STRAP_VALUE[bit28:29 strap_test_debug_en] should be 0b'00 if you plan"));
        BDBG_WRN(("to use BSP ROM code. If not, check with board designer on your strap pin."));
        BDBG_WRN(("**********************************************"));
        rc = BERR_SUCCESS;
    }

#if NEXUS_SECURITY_ZEUS_VERSION_MAJOR < 1
    {
        BHSM_SetMiscBitsIO_t setMiscBitsIO;

        setMiscBitsIO.setMiscBitsSubCmd = BCMD_SetMiscBitsSubCmd_eRaveBits;
        setMiscBitsIO.bEnableWriteIMem = 1;
        setMiscBitsIO.bEnableReadIMem = 1;
        setMiscBitsIO.bEnableReadDMem = 1;
        setMiscBitsIO.bDisableClear = 1;
        setMiscBitsIO.bEnableEncBypass = 0; /* bRAVEEncryptionBypass for 40nm plus */
        rc = BHSM_SetMiscBits(g_security.hsm, &setMiscBitsIO);
        if (rc) {
            BDBG_WRN(("**********************************************"));
            BDBG_WRN(("If you see this warning and the HSM errors above, you need to perform some"));
            BDBG_WRN(("board reconfiguration. This is not required. If you want, you can ignore them."));
            BDBG_WRN(("Use BBS to check if BSP_GLB_NONSECURE_GLB_IRDY = 0x07."));
            BDBG_WRN(("If not, BSP/Aegis is not ready to accept a command."));
            BDBG_WRN(("SUN_TOP_CTRL_STRAP_VALUE[bit28:29 strap_test_debug_en] should be 0b'00 if you plan"));
            BDBG_WRN(("to use BSP ROM code. If not, check with board designer on your strap pin."));
            BDBG_WRN(("**********************************************"));
            rc = BERR_SUCCESS;
        }
    }
#endif

#if NEXUS_HAS_XPT_DMA
    rc = BHSM_SetPidChannelBypassKeyslot(g_security.hsm, NEXUS_PidChannel_GetBypassKeySlotIndex_isrsafe(NEXUS_BypassKeySlot_eG2GR), NEXUS_BypassKeySlot_eG2GR);
    if (rc) {BERR_TRACE(rc);} /* keep going */
    rc = BHSM_SetPidChannelBypassKeyslot(g_security.hsm, NEXUS_PidChannel_GetBypassKeySlotIndex_isrsafe(NEXUS_BypassKeySlot_eGR2R), NEXUS_BypassKeySlot_eGR2R);
    if (rc) {BERR_TRACE(rc);} /* keep going */
#endif

    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, false);

    BDBG_LEAVE(NEXUS_Security_P_InitHsm);
    return rc;
}

void NEXUS_Security_P_UninitHsm()
{

#if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(1,0)
    #if HAS_TWO_HSM_CMD_CHANNELS
    (void)BHSM_Channel_Close(g_security.hsmChannel[BHSM_HwModule_eCmdInterface1]);
    #endif
    (void)BHSM_Channel_Close(g_security.hsmChannel[BHSM_HwModule_eCmdInterface2]);
#endif

    BHSM_Close(g_security.hsm);
    g_security.hsm = NULL;

    return;
}

void NEXUS_Security_GetHsm_priv(BHSM_Handle *pHsm)
{
    NEXUS_ASSERT_MODULE();
    *pHsm = g_security.hsm;
}

void NEXUS_Security_GetDefaultKeySlotSettings(NEXUS_SecurityKeySlotSettings *pSettings)
{
    BDBG_ENTER(NEXUS_Security_GetDefaultKeySlotSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->keySlotEngine = NEXUS_SecurityEngine_eCa;
#if HSM_IS_ASKM
    pSettings->keySlotSource = NEXUS_SecurityKeySource_eFirstRamAskm;
#else
    pSettings->keySlotSource = NEXUS_SecurityKeySource_eFirstRam;
#endif
    pSettings->keySlotType = NEXUS_SecurityKeySlotType_eAuto;

    BDBG_LEAVE(NEXUS_Security_GetDefaultKeySlotSettings);
    return;
}

static NEXUS_Error NEXUS_Security_P_GetInvalidateKeyFlag( const NEXUS_SecurityInvalidateKeyFlag nexusKeyFlag, BCMD_InvalidateKey_Flag_e *hsmKeyFlag )
{

    switch( nexusKeyFlag )
    {
        case NEXUS_SecurityInvalidateKeyFlag_eSrcKeyOnly:  { *hsmKeyFlag = BCMD_InvalidateKey_Flag_eSrcKeyOnly;  break; }
        case NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly: { *hsmKeyFlag = BCMD_InvalidateKey_Flag_eDestKeyOnly; break; }
        case NEXUS_SecurityInvalidateKeyFlag_eAllKeys:     { *hsmKeyFlag = BCMD_InvalidateKey_Flag_eBoth;        break; }

        default: return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    return NEXUS_SUCCESS;
}

static BCMD_KeyDestEntryType_e NEXUS_Security_MapNexusKeyDestToHsm(NEXUS_KeySlotHandle keyHandle, NEXUS_SecurityKeyType keydest)
{
    BCMD_KeyDestEntryType_e rv = BCMD_KeyDestEntryType_eOddKey;
    BSTD_UNUSED(keyHandle);


    switch( keydest )
    {
        case NEXUS_SecurityKeyType_eOddAndEven:
        case NEXUS_SecurityKeyType_eOdd:
        {
            rv = BCMD_KeyDestEntryType_eOddKey;
            break;
        }
        case NEXUS_SecurityKeyType_eEven:
        {
            rv = BCMD_KeyDestEntryType_eEvenKey;
            break;
        }
        case NEXUS_SecurityKeyType_eClear:
        {
           #if HSM_IS_ASKM_40NM
            rv = BCMD_KeyDestEntryType_eClearKey;
           #else
            rv = BCMD_KeyDestEntryType_eReserved0;
           #endif
            break;
        }
       #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(1,0) /*for pre Zeus*/
        case NEXUS_SecurityKeyType_eIv:
        {
            rv = BCMD_KeyDestEntryType_eIV;
            break;
        }
       #endif
        default:
        {
            BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* return default */
        }
    }

    BDBG_MSG(("NEXUS_Security_MapNexusKeyDestToHsm -- keydest NEX[%d] HSM[%d]" , keydest , rv ));

    return rv;
}

#define NEXUS_SECURITY_CACP_INVALID_PIDCHANNEL 0xFFFFFFFF

static NEXUS_Error NEXUS_Security_AllocateKeySlotForType(  NEXUS_KeySlotHandle *pKeyHandle
                                                           , const NEXUS_SecurityKeySlotSettings *pSettings
                                                           , BCMD_XptSecKeySlot_e type
                                                        ) /*  */
{
    BERR_Code rc;
    NEXUS_KeySlotHandle pHandle;
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
    BHSM_KeySlotAllocate_t  keySlotConf;
   #else
    unsigned int keyslotNumber = 0;
   #endif

    BDBG_ENTER(NEXUS_Security_AllocateKeySlotForType);

    NEXUS_SecurityModule_Sweep_priv();

    pHandle = NEXUS_KeySlot_Create();
    if ( !pHandle) { return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); }

    pHandle->deferDestroy = true;

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
    BKNI_Memset(&keySlotConf, 0, sizeof(keySlotConf));

    keySlotConf.client = pSettings->client;
    keySlotConf.keySlotType = type;

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    rc = BHSM_AllocateCAKeySlot( g_security.hsm, &keySlotConf );
    #else
    rc = BHSM_AllocateCAKeySlot_v2( g_security.hsm, &keySlotConf );
    #endif

   #else
    rc = BHSM_AllocateCAKeySlot( g_security.hsm, type, &keyslotNumber );
   #endif

    if (rc) goto error;

    pHandle->keyslotType = type;
    pHandle->settings = *pSettings;

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
    pHandle->keySlotNumber = keySlotConf.keySlotNum;
   #else
    pHandle->keySlotNumber = keyslotNumber;
   #endif

    *pKeyHandle = pHandle;

    BDBG_MSG(("%s: Allocated keyslot %p",__FUNCTION__, (void *)pHandle ));

    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, true);

    return NEXUS_SUCCESS;
error:
    pHandle->deferDestroy = false;
    NEXUS_KeySlot_Destroy(pHandle);
    *pKeyHandle = NULL;

    BDBG_LEAVE(NEXUS_Security_AllocateKeySlotForType);
    return BERR_TRACE(MAKE_HSM_ERR(rc));
}

static NEXUS_Security_P_PidChannelListNode *find_pid(NEXUS_KeySlotHandle keyslot, NEXUS_PidChannelHandle pidChannel, unsigned pidChannelIndex)
{
    NEXUS_Error rc;
    NEXUS_Security_P_KeySlotData *pKeySlotData;
    NEXUS_Security_P_PidChannelListNode *pPidChannelNode;
    NEXUS_PidChannelStatus status;

    pKeySlotData = keyslot->security.data;
    if (!pKeySlotData) return NULL;

    for( pPidChannelNode = BLST_S_FIRST(&pKeySlotData->pidChannelList); pPidChannelNode != NULL; pPidChannelNode = BLST_S_NEXT(pPidChannelNode,next) )
    {
        if( pidChannel )
        {
            if( pPidChannelNode->pidChannel == pidChannel )
            {
                return pPidChannelNode; /* found a match ... same NEXUS_PidChannelHandle instance */
            }

            /* retrieve actual HW pid Channel */
            rc = NEXUS_PidChannel_GetStatus( pidChannel, &status );
            if( rc != NEXUS_SUCCESS )
            {
                 BERR_TRACE( rc );
                 continue;
            }
            /* update imput parameter. */
            pidChannelIndex = status.pidChannelIndex;
        }

        if( pPidChannelNode->pidChannel )
        {
            /* retrieve actual HW pid Channel */
            rc = NEXUS_PidChannel_GetStatus( pPidChannelNode->pidChannel, &status );
            if( rc != NEXUS_SUCCESS )
            {
                 BERR_TRACE( rc );
                 continue;
            }

            pPidChannelNode->pidChannelIndex = status.pidChannelIndex;
        }

        if( pidChannelIndex == pPidChannelNode->pidChannelIndex )
        {
            return pPidChannelNode; /* found a match */
        }
    }

    return NULL;
}

NEXUS_KeySlotHandle NEXUS_Security_LocateCaKeySlotAssigned(unsigned long pidchannel)
{
    BERR_Code rc;
    NEXUS_KeySlotHandle keyslot;
    unsigned int keyslotNumber;
    unsigned int keyslotType;

    BDBG_ENTER(NEXUS_Security_LocateCaKeySlotAssigned);

    NEXUS_SecurityModule_Sweep_priv();

    NEXUS_SECURITY_DUMP_KEYSLOTS;

    rc = BHSM_LocateCAKeySlotAssigned(g_security.hsm, pidchannel, BHSM_PidChannelType_ePrimary, &keyslotType, &keyslotNumber);
    if (rc) {
        rc = BERR_TRACE(MAKE_HSM_ERR(rc));
        goto error;
    }
    for (keyslot=BLST_S_FIRST(&g_security.keyslotList);keyslot;keyslot=BLST_S_NEXT(keyslot,next)) {
        NEXUS_Security_P_PidChannelListNode *pPidChannelNode = find_pid(keyslot, NULL, pidchannel);
        if (pPidChannelNode) {
            if (keyslot->keySlotNumber != keyslotNumber || keyslot->keyslotType != keyslotType) {
                BDBG_WRN(("%s: number/type difference, keyslot tracking may be out of sync with HSM",__FUNCTION__));
            }
            return keyslot;
        }
    }
error:
    BDBG_WRN(("%s: PID Channel %lu is not yet associated with any key slot",__FUNCTION__,pidchannel));

    BDBG_LEAVE(NEXUS_Security_LocateCaKeySlotAssigned);
    return NULL;
}


static NEXUS_Error add_pid_channel(NEXUS_KeySlotHandle keyHandle, NEXUS_Security_P_KeySlotData *pKeySlotData, NEXUS_PidChannelHandle pidChannel, unsigned pidChannelIndex)
{
    BHSM_ConfigPidKeyPointerTableIO_t pidChannelConf;
    NEXUS_Security_P_PidChannelListNode *pPidChannelNode;
    BERR_Code rc;

    BDBG_ENTER(add_pid_channel);

    pPidChannelNode = BKNI_Malloc(sizeof(*pPidChannelNode));
    if (!pPidChannelNode) return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);

    if (pidChannel) {
        pidChannelIndex = NEXUS_PidChannel_GetIndex_isrsafe(pidChannel);
    }

    BKNI_Memset(&pidChannelConf, 0, sizeof(pidChannelConf));
    pidChannelConf.unPidChannel = pidChannelIndex;
    pidChannelConf.ucKeySlotType = keyHandle->keyslotType;
    pidChannelConf.unKeySlotNum = keyHandle->keySlotNumber;
    pidChannelConf.pidChannelType = BHSM_PidChannelType_ePrimary;
   #if HSM_IS_ASKM_40NM
    pidChannelConf.spidProgType = BHSM_SPIDProg_ePIDPointerA;
    pidChannelConf.bResetPIDToDefault = false;
    pidChannelConf.unKeySlotBType = 0;
   #else
    pidChannelConf.unKeySlotB = 0;
   #endif
    pidChannelConf.unKeySlotNumberB = 0;
    pidChannelConf.unKeyPointerSel = 0;

    rc = BHSM_ConfigPidKeyPointerTable(g_security.hsm, &pidChannelConf);
    if (rc) { rc = BERR_TRACE(MAKE_HSM_ERR(rc)); goto err_config;}

    BKNI_Memset(pPidChannelNode,0,sizeof(*pPidChannelNode));
    pPidChannelNode->pidChannel = pidChannel;
    pPidChannelNode->pidChannelIndex = pidChannelIndex;
    BLST_S_INSERT_HEAD(&pKeySlotData->pidChannelList, pPidChannelNode, next);
    NEXUS_SECURITY_DUMP_KEYSLOTS;

    return NEXUS_SUCCESS;

err_config:
    BKNI_Free(pPidChannelNode);

    BDBG_LEAVE(add_pid_channel);
    return rc;
}

static NEXUS_Error verify_pid_channel( NEXUS_KeySlotHandle keyslot, NEXUS_PidChannelHandle pidChannel, unsigned pidChannelIndex )
{
    NEXUS_KeySlotHandle hKeySlot;
    NEXUS_PidChannelHandle previousPidChannel = NULL;

    BDBG_ENTER(verify_pid_channel);

    for( hKeySlot = BLST_S_FIRST(&g_security.keyslotList); hKeySlot; hKeySlot=BLST_S_NEXT(hKeySlot,next) )
    {
        NEXUS_Security_P_PidChannelListNode *pPidChannelNode;

        pPidChannelNode = find_pid( hKeySlot, pidChannel, pidChannelIndex );

        if( pPidChannelNode )
        {
            if( hKeySlot == keyslot )
            {
                /* don't allow double adds */
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
            BDBG_MSG(("pidchannel %p/%d already has keyslot %p associated, breaking association", (void *)pidChannel, pidChannelIndex, (void *)hKeySlot));

            previousPidChannel = pPidChannelNode->pidChannel;

            NEXUS_Security_P_RemovePidchannel( hKeySlot,pPidChannelNode );
            if( previousPidChannel)
            {
                NEXUS_OBJECT_RELEASE( hKeySlot, NEXUS_PidChannel, previousPidChannel);
            }
        }
    }

    BDBG_LEAVE( verify_pid_channel );
    return 0;
}

NEXUS_Error NEXUS_Security_AddPidChannelToKeySlot(NEXUS_KeySlotHandle keyslot, unsigned pidChannelIndex)
{
    int rc;
    NEXUS_Security_P_KeySlotData *pKeySlotData;

    BDBG_ENTER(NEXUS_Security_AddPidChannelToKeySlot);

    BDBG_OBJECT_ASSERT(keyslot, NEXUS_KeySlot);
    pKeySlotData = keyslot->security.data;
    if (!pKeySlotData) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    BDBG_MSG(("AddPidChannelToKeySlot: %p(%d,%d)", (void *)keyslot,keyslot->keySlotNumber,keyslot->keyslotType));

    rc = verify_pid_channel(keyslot, NULL, pidChannelIndex);
    if (rc) return BERR_TRACE(rc);

    rc = add_pid_channel(keyslot, pKeySlotData, NULL, pidChannelIndex);
    if (rc) return BERR_TRACE(rc);

    BDBG_LEAVE(NEXUS_Security_AddPidChannelToKeySlot);
    return 0;
}

NEXUS_Error NEXUS_KeySlot_AddPidChannel( NEXUS_KeySlotHandle keyslot, NEXUS_PidChannelHandle pidChannel )
{
    int rc;
    NEXUS_Security_P_KeySlotData *pKeySlotData;

    BDBG_ENTER(NEXUS_KeySlot_AddPidChannel);

    BDBG_OBJECT_ASSERT(keyslot, NEXUS_KeySlot);
    pKeySlotData = keyslot->security.data;
    if (!pKeySlotData) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    BDBG_MSG(("AddPidChannelToKeySlot: %p(%d,%d)", (void *)keyslot,keyslot->keySlotNumber,keyslot->keyslotType));

    rc = verify_pid_channel(keyslot, pidChannel, 0);
    if (rc) return BERR_TRACE(rc);

    rc = add_pid_channel(keyslot, pKeySlotData, pidChannel, 0);
    if (rc) return BERR_TRACE(rc);

    NEXUS_OBJECT_ACQUIRE(keyslot, NEXUS_PidChannel, pidChannel);

    BDBG_LEAVE(NEXUS_KeySlot_AddPidChannel);
    return 0;
}

static void remove_pid_channel(NEXUS_KeySlotHandle keyHandle, NEXUS_Security_P_PidChannelListNode *pPidChannelNode)
{
    BHSM_ConfigPidKeyPointerTableIO_t pidChannelConf;
    int rc;

    BDBG_ENTER(remove_pid_channel);

    BDBG_MSG((" RemovePidChannelFromKeySlot: %p(%u,%u,%ld)",
                    (void *)keyHandle
                    ,keyHandle->keySlotNumber
                    ,keyHandle->keyslotType
                    ,pPidChannelNode->pidChannelIndex ));

    BKNI_Memset( &pidChannelConf, 0, sizeof(pidChannelConf) );

    pidChannelConf.unPidChannel = pPidChannelNode->pidChannelIndex;
    pidChannelConf.ucKeySlotType = keyHandle->keyslotType;
    pidChannelConf.unKeySlotNum = keyHandle->keySlotNumber;
    pidChannelConf.pidChannelType = BHSM_PidChannelType_ePrimary;
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
    pidChannelConf.spidProgType = BHSM_SPIDProg_ePIDPointerA;
    pidChannelConf.bResetPIDToDefault = false;
    pidChannelConf.unKeySlotBType = 0;
    #else
    pidChannelConf.unKeySlotB = 0;
    #endif
    pidChannelConf.unKeySlotNumberB = 0;
    pidChannelConf.unKeyPointerSel = 0;

    rc = BHSM_ConfigPidChannelToDefaultKeySlot( g_security.hsm, &pidChannelConf );
    if (rc)
    {
        rc = BERR_TRACE(rc); /* Internal error. Continue for best effort */
    }

    NEXUS_Security_P_RemovePidchannel( keyHandle, pPidChannelNode );

    NEXUS_SECURITY_DUMP_KEYSLOTS;

    BDBG_ENTER(remove_pid_channel);
    return;
}

NEXUS_Error NEXUS_Security_RemovePidChannelFromKeySlot(NEXUS_KeySlotHandle keyslot, unsigned pidChannelIndex)
{
    NEXUS_Security_P_PidChannelListNode *pPidChannelNode;

    BDBG_ENTER(NEXUS_Security_RemovePidChannelFromKeySlot);

    BDBG_OBJECT_ASSERT(keyslot, NEXUS_KeySlot);

    BDBG_MSG(("RemovePidChannelFromKeySlot: %p(%d,%d,%d)", (void *)keyslot,keyslot->keySlotNumber,keyslot->keyslotType,pidChannelIndex));
    pPidChannelNode = find_pid(keyslot, NULL, pidChannelIndex);
    if (!pPidChannelNode) return NEXUS_INVALID_PARAMETER; /* no BERR_TRACE */

    remove_pid_channel(keyslot, pPidChannelNode);

    BDBG_LEAVE(NEXUS_Security_RemovePidChannelFromKeySlot);
    return 0;
}

void NEXUS_KeySlot_RemovePidChannel( NEXUS_KeySlotHandle keyslot, NEXUS_PidChannelHandle pidChannel )
{
    NEXUS_Security_P_PidChannelListNode *pPidChannelNode;
    BDBG_OBJECT_ASSERT(keyslot, NEXUS_KeySlot);

    BDBG_ENTER(NEXUS_KeySlot_RemovePidChannel);

    BDBG_MSG(("NEXUS_KeySlot_RemovePidChannel: %p(%d,%d,%p)", (void *)keyslot,keyslot->keySlotNumber,keyslot->keyslotType,(void *)pidChannel));
    pPidChannelNode = find_pid(keyslot, pidChannel, 0);
    if (pPidChannelNode) {
        remove_pid_channel(keyslot, pPidChannelNode);
        NEXUS_OBJECT_RELEASE(keyslot, NEXUS_PidChannel, pidChannel);
    }

    BDBG_LEAVE(NEXUS_KeySlot_RemovePidChannel);
    return;
}

static NEXUS_Error NEXUS_Security_MapHsmKeySlotTypeToNexus( BCMD_XptSecKeySlot_e hsmKeyslotType, NEXUS_SecurityKeySlotType *nexusSlotType)
{
    NEXUS_SecurityKeySlotType rvType = NEXUS_SecurityKeySlotType_eType0;

    BDBG_ENTER(NEXUS_Security_MapHsmKeySlotTypeToNexus);

    /* It is a direct map */
    switch (hsmKeyslotType)
    {
        case BCMD_XptSecKeySlot_eType0:
        {
            rvType = NEXUS_SecurityKeySlotType_eType0;
            break;
        }
        case BCMD_XptSecKeySlot_eType1:
        {
            rvType = NEXUS_SecurityKeySlotType_eType1;
            break;
        }
        case BCMD_XptSecKeySlot_eType2:
        {
            rvType = NEXUS_SecurityKeySlotType_eType2;
            break;
        }
        case BCMD_XptSecKeySlot_eType3:
        {
            rvType = NEXUS_SecurityKeySlotType_eType3;
            break;
        }
        case BCMD_XptSecKeySlot_eType4:
        {
            rvType = NEXUS_SecurityKeySlotType_eType4;
            break;
        }

       #if HAS_TYPE5_KEYSLOTS
        case BCMD_XptSecKeySlot_eType5:
        {
            rvType = NEXUS_SecurityKeySlotType_eType5;
            break;
        }

        #if !HSM_IS_ASKM_28NM_ZEUS_4_0
        case BCMD_XptSecKeySlot_eType6:
        {
            rvType = NEXUS_SecurityKeySlotType_eType6;
            break;
        }
        #endif

       #endif /* HAS_TYPE5_KEYSLOTS */

       #if HAS_TYPE7_KEYSLOTS
        case BCMD_XptSecKeySlot_eType7:
        {
            rvType = NEXUS_SecurityKeySlotType_eType7;
            break;
        }
       #endif

        default:
        {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }

    *nexusSlotType = rvType;

    BDBG_MSG(("NEXUS_Security_MapHsmKeySlotTypeToNexus -- hsm slotType[%d] nexus slot type[%d]", hsmKeyslotType, rvType ));

    BDBG_LEAVE(NEXUS_Security_MapHsmKeySlotTypeToNexus);
    return 0;
}

static NEXUS_Error NEXUS_Security_MapNexusKeySlotTypeToHsm(NEXUS_SecurityKeySlotType slotType, NEXUS_SecurityEngine engine, BCMD_XptSecKeySlot_e *pType)
{
    BCMD_XptSecKeySlot_e rvType = BCMD_XptSecKeySlot_eType0;

    BDBG_ENTER(NEXUS_Security_MapNexusKeySlotTypeToHsm);


    if( slotType != NEXUS_SecurityKeySlotType_eAuto )
    {
        /* ... its a direct map */
        switch( slotType )
        {
            case NEXUS_SecurityKeySlotType_eType0:
            {
                rvType = BCMD_XptSecKeySlot_eType0;
                break;
            }
            case NEXUS_SecurityKeySlotType_eType1:
            {
                rvType = BCMD_XptSecKeySlot_eType1;
                break;
            }
            case NEXUS_SecurityKeySlotType_eType2:
            {
                rvType = BCMD_XptSecKeySlot_eType2;
                break;
            }
            case NEXUS_SecurityKeySlotType_eType3:
            {
                rvType = BCMD_XptSecKeySlot_eType3;
                break;
            }
            case NEXUS_SecurityKeySlotType_eType4:
            {
                rvType = BCMD_XptSecKeySlot_eType4;
                break;
            }
         #if HAS_TYPE5_KEYSLOTS
            case NEXUS_SecurityKeySlotType_eType5:
            {
                rvType = BCMD_XptSecKeySlot_eType5;
                break;
            }

            #if !HSM_IS_ASKM_28NM_ZEUS_4_0
            case NEXUS_SecurityKeySlotType_eType6:
            {
                rvType = BCMD_XptSecKeySlot_eType6;
                break;
            }
            #endif
         #endif /* HAS_TYPE5_KEYSLOTS */

         #if HAS_TYPE7_KEYSLOTS
            case NEXUS_SecurityKeySlotType_eType7:
            {
                rvType = BCMD_XptSecKeySlot_eType7;
                break;
            }
         #endif
            default:
            {
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }

    }
    else /* slotType != NEXUS_SecurityKeySlotType_eAuto */
    {
        switch( engine )   /* Determine the keyslot type based on engine */
        {
            case NEXUS_SecurityEngine_eM2m:
            {
              #if HSM_IS_ASKM_40NM
                rvType = BCMD_XptSecKeySlot_eType3;
              #else
                rvType = BCMD_XptSecKeySlot_eType1;
              #endif
                break;
            }
            case NEXUS_SecurityEngine_eCa:
            {
              #if HSM_IS_ASKM_40NM
                rvType = BCMD_XptSecKeySlot_eType0;
              #else
                rvType = BCMD_XptSecKeySlot_eType1;
              #endif
                break;
            }
            case NEXUS_SecurityEngine_eCaCp:
            case NEXUS_SecurityEngine_eRmx:
            {
              #if HSM_IS_ASKM_40NM
                rvType = BCMD_XptSecKeySlot_eType0;
              #else
               #if HAS_TYPE7_KEYSLOTS
                rvType = BCMD_XptSecKeySlot_eType7;
               #elif !HAS_TYPE5_KEYSLOTS
                rvType = BCMD_XptSecKeySlot_eType4;
               #else
                rvType = BCMD_XptSecKeySlot_eType6;
               #endif
              #endif
                break;
            }
            case NEXUS_SecurityEngine_eCp:
            default:
            {
                /* an unsupported or invalid security engine value was passed in. */
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
    }

    *pType = rvType;


    BDBG_MSG(("NEXUS_Security_MapNexusKeySlotTypeToHsm -- slotType[%d] polType[%d,%d]" , slotType  , engine , rvType ));


    BDBG_LEAVE(NEXUS_Security_MapNexusKeySlotTypeToHsm);
    return 0;
}

NEXUS_Error NEXUS_Security_AllocateM2mKeySlot(NEXUS_KeySlotHandle * pKeyHandle,const NEXUS_SecurityKeySlotSettings *pSettings, BCMD_XptSecKeySlot_e type)
{
    BERR_Code rc;
    NEXUS_KeySlotHandle pHandle;
    BHSM_M2MKeySlotIO_t M2MKeySlotIO;

    BDBG_ENTER(NEXUS_Security_AllocateM2mKeySlot);

    NEXUS_SecurityModule_Sweep_priv();

    pHandle = NEXUS_KeySlot_Create();
    if ( !pHandle) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_create;}

    pHandle->deferDestroy = true;

    /* client field is needed starting with Zeus 3.0 */
    M2MKeySlotIO.client = pSettings->client;
    /* keySlotType is needed starting with Zeus 4.0 */
    M2MKeySlotIO.keySlotType = type;
    rc = BHSM_AllocateM2MKeySlot(g_security.hsm, &M2MKeySlotIO);
    if (rc) {rc = BERR_TRACE(rc); goto err_allocm2m;}

    pHandle->keySlotNumber = M2MKeySlotIO.keySlotNum;
    pHandle->settings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    /* keySlotType is needed starting with Zeus 4.0 */
    pHandle->keyslotType  = M2MKeySlotIO.keySlotType;
    BDBG_MSG(("Allocated M2M keyslot %d", M2MKeySlotIO.keySlotNum));

    *pKeyHandle = pHandle;

    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, true);

    BDBG_LEAVE(NEXUS_Security_AllocateM2mKeySlot);
    return NEXUS_SUCCESS;

err_allocm2m:
    pHandle->deferDestroy = false;
    NEXUS_KeySlot_Destroy(pHandle);
err_create:
    *pKeyHandle = NULL;

    BDBG_LEAVE(NEXUS_Security_AllocateM2mKeySlot);
    return BERR_TRACE(MAKE_HSM_ERR(rc));
}

#if HSM_IS_ASKM

void NEXUS_Security_GetDefaultAlgorithmSettings(NEXUS_SecurityAlgorithmSettings * pSettings)
{

    BDBG_ENTER(NEXUS_Security_GetDefaultAlgorithmSettings);

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );

    pSettings->algorithm = NEXUS_SecurityAlgorithm_e3DesAba;
    pSettings->algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
    pSettings->terminationMode = NEXUS_SecurityTerminationMode_eClear;
    pSettings->aesCounterSize = NEXUS_SecurityAesCounterSize_e32Bits;
    pSettings->dvbScramLevel = NEXUS_SecurityDvbScrambleLevel_eTs;
    pSettings->operation = NEXUS_SecurityOperation_ePassThrough;
    pSettings->keyDestEntryType = NEXUS_SecurityKeyType_eOddAndEven;
    pSettings->testKey2Select = false;
    pSettings->bRestrictEnable = true;
    pSettings->bGlobalEnable = true;
    pSettings->bScAtscMode = false;
    pSettings->bAtscModEnable = false;
    pSettings->bGlobalDropPktEnable = false;
    pSettings->bGlobalRegionOverwrite = false;
    pSettings->bEncryptBeforeRave = false;
    pSettings->enableExtIv = false;
    pSettings->enableExtKey = false;
    pSettings->mscBitSelect = false;
    pSettings->bDisallowGG = false;
    #if HSM_IS_ASKM_40NM_ZEUS_3_0
    pSettings->bDisallowGR = true;
    pSettings->bDisallowRG = true;
    pSettings->bRestrictDropPktEnable = true;
    #else
    pSettings->bDisallowGR = false;
    pSettings->bDisallowRG = false;
    pSettings->bRestrictDropPktEnable = false;
    #endif
    pSettings->bDisallowRR = false;

    /* ASKM default virtual keyladder configuration -- this is suitable for apps which are not using
    * these features and are built for a generic part.  It is expected that the app will override
    * these values if actually using ASKM features.
    *
    * Added to NEXUS_Security_GetDefaultAlgorithmSettings to allow older apps to work on
    * newer parts without modification.
    */
    pSettings->ivMode = NEXUS_SecurityIVMode_eRegular;
    pSettings->solitarySelect = NEXUS_SecuritySolitarySelect_eClear;
    pSettings->caVendorID = 0x1234;
    pSettings->askmModuleID = NEXUS_SecurityAskmModuleID_eModuleID_4;
    pSettings->otpId = NEXUS_SecurityOtpId_eOtpVal;
    pSettings->testKey2Select = 0;

    BDBG_LEAVE(NEXUS_Security_GetDefaultAlgorithmSettings);
    return;
}

static BCMD_XptM2MSecCryptoAlg_e NEXUS_Security_MapNexusAlgorithmToHsm(NEXUS_SecurityAlgorithm algorithm)
{
    BCMD_XptM2MSecCryptoAlg_e rvAlgorithm = algorithm;

    switch (algorithm)
    {
    case NEXUS_SecurityAlgorithm_eMulti2:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eMulti2;
        break;
    case NEXUS_SecurityAlgorithm_eDes:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eDes;
        break;
    case NEXUS_SecurityAlgorithm_e3DesAba:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_e3DesAba;
        break;
    case NEXUS_SecurityAlgorithm_e3DesAbc:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_e3DesAbc;
        break;
    case NEXUS_SecurityAlgorithm_eAes:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eAes128;
        break;
    case NEXUS_SecurityAlgorithm_eAes192:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eAes192;
        break;
    case NEXUS_SecurityAlgorithm_eC2:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eC2;
        break;
    case NEXUS_SecurityAlgorithm_eCss:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eCss;
        break;
    case NEXUS_SecurityAlgorithm_eM6Ke:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eM6KE;
        break;
    case NEXUS_SecurityAlgorithm_eM6:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eM6;
        break;
    case NEXUS_SecurityAlgorithm_eWMDrmPd:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eWMDrmPd;
        break;
    #if HSM_IS_ASKM_40NM
    case NEXUS_SecurityAlgorithm_eDvbCsa3:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eDVBCSA3;
        break;
    case NEXUS_SecurityAlgorithm_eAesCounter:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eAesCounter0;
        break;
    case NEXUS_SecurityAlgorithm_eMSMultiSwapMac:
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eMSMULTISWAPMAC;
        break;
     #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    case NEXUS_SecurityAlgorithm_eAsa:
       #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eReserved19;
       #else
        rvAlgorithm =  BCMD_XptM2MSecCryptoAlg_eASA;
       #endif
        break;
      #endif

    #endif /*HSM_IS_ASKM_40NM*/

    /* The _eReservedX values should pass the literal value X into HSM,
     * allowing direct control of custom modes
     * Macro trickery to avoid copy/paste errors */
    #define NEXUS_REMAP_RESERVED(VAL) \
    case NEXUS_SecurityAlgorithm_eReserved##VAL : \
        rvAlgorithm = (BCMD_XptM2MSecCryptoAlg_e) VAL ; \
        break;
        NEXUS_REMAP_RESERVED(0)
        NEXUS_REMAP_RESERVED(1)
        NEXUS_REMAP_RESERVED(2)
        NEXUS_REMAP_RESERVED(3)
        NEXUS_REMAP_RESERVED(4)
        NEXUS_REMAP_RESERVED(5)
        NEXUS_REMAP_RESERVED(6)
        NEXUS_REMAP_RESERVED(7)
        NEXUS_REMAP_RESERVED(8)
    #undef NEXUS_REMAP_RESERVED
    default:
        break;
    }

    return rvAlgorithm;
}

static void NEXUS_Security_GetHsmAlgorithmKeySetting(
        NEXUS_KeySlotHandle keyHandle,
        const NEXUS_SecurityAlgorithmSettings *pSettings,
        BCMD_XptM2MSecCryptoAlg_e *pCryptAlg,
        BCMD_CipherModeSelect_e *pCipherMode,
        BCMD_TerminationMode_e *pTerminationMode
)
{

    /* fixups/remapping needed since the enums do not always agree across all platforms */
    *pCryptAlg = NEXUS_Security_MapNexusAlgorithmToHsm(pSettings->algorithm);
    *pCipherMode = (BCMD_CipherModeSelect_e)pSettings->algorithmVar;
    *pTerminationMode = (BCMD_TerminationMode_e)pSettings->terminationMode;

    if ( keyHandle->settings.keySlotEngine != NEXUS_SecurityEngine_eM2m )
    {
        if (pSettings->algorithm == NEXUS_SecurityAlgorithm_eDvb ||
            pSettings->algorithm == NEXUS_SecurityAlgorithm_eDvbCsa3)
        {
            /* overload cipherMode for CA/CP */
            if (pSettings->dvbScramLevel == NEXUS_SecurityDvbScrambleLevel_eTs)
            {
                *pCipherMode = (BCMD_CipherModeSelect_e)NEXUS_SecurityAlgorithmVariant_eXpt;
            }
            else
            {
                *pCipherMode = (BCMD_CipherModeSelect_e)NEXUS_SecurityAlgorithmVariant_ePes;
            }
        }
    }
    else /* Security Algorithm Key setting for M2M */
    {
        if (pSettings->algorithm == NEXUS_SecurityAlgorithm_eAes ||
                pSettings->algorithm == NEXUS_SecurityAlgorithm_eAes192)
        {
            if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eCounter)
            {
                /* overload termination mode for M2M to handle AESCounter                 */
                /*  For Zeus 2.0 and earlier CounterMode and CounterSize are the same */
                /*  For  Zeus 2.0, 3.0, and 4.x, they are different, and CounterSize is pisked up after this   */
                #if HSM_IS_ASKM_40NM_ZEUS_3_0
                *pTerminationMode = pSettings->aesCounterMode;
                #else
                *pTerminationMode = pSettings->aesCounterSize;
                #endif
            }
        }
    }
}

#else /* HSM_IS_ASKM  */

void NEXUS_Security_GetDefaultAlgorithmSettings(NEXUS_SecurityAlgorithmSettings * pSettings)
{
    BDBG_ENTER(NEXUS_Security_GetDefaultAlgorithmSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->algorithm = NEXUS_SecurityAlgorithm_e3DesAba;
    pSettings->algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
    pSettings->terminationMode = NEXUS_SecurityTerminationMode_eCipherStealing;
    pSettings->aesCounterSize = NEXUS_SecurityAesCounterSize_e32Bits;
    pSettings->dvbScramLevel = NEXUS_SecurityDvbScrambleLevel_eTs;
    pSettings->keyDestEntryType = NEXUS_SecurityKeyType_eOddAndEven;
    pSettings->operation = NEXUS_SecurityOperation_ePassThrough;
    pSettings->bRestrictEnable = true;
    pSettings->bGlobalEnable = true;
    pSettings->bScAtscMode = false;
    pSettings->bAtscModEnable = false;
    pSettings->bGlobalDropPktEnable = false;
    pSettings->bRestrictDropPktEnable = false;
    pSettings->bGlobalRegionOverwrite = false;
    pSettings->enableExtIv = false;
    pSettings->enableExtKey = false;
    pSettings->mscBitSelect = false;
    pSettings->bScPolarityEnable = false;
    pSettings->bSynEnable = false;
    pSettings->bCPDDisable = false;
    pSettings->bCPSDisable = false;

    BDBG_LEAVE(NEXUS_Security_GetDefaultAlgorithmSettings);
    return;
}

static bool NEXUS_Security_GetHsmCaCpRmxAlgorithmKeySetting(const NEXUS_SecurityAlgorithmSettings * pSettings, unsigned int * pCryptAlg, BHSM_ResidueMode_e * pResidualMode)
{
    BDBG_ENTER(NEXUS_Security_GetHsmCaCpRmxAlgorithmKeySetting);

    switch (pSettings->algorithm) {
    case NEXUS_SecurityAlgorithm_eDvb: {
        *pCryptAlg = BCMD_XptSecCryptoAlg_eDvb;
        if (pSettings->dvbScramLevel == NEXUS_SecurityDvbScrambleLevel_eTs)
            *pResidualMode = BHSM_DVBScrambleLevel_eTS;
        else
            *pResidualMode = BHSM_DVBScrambleLevel_ePes;
    }
        return true;

    case NEXUS_SecurityAlgorithm_eDes: {
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eEcb)
            *pCryptAlg = BCMD_XptSecCryptoAlg_eDesEcb;
        else
            *pCryptAlg = BCMD_XptSecCryptoAlg_eDesCbc;
    }
        break;
    case NEXUS_SecurityAlgorithm_e3DesAba: {
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eEcb)
            *pCryptAlg = BCMD_XptSecCryptoAlg_e3DesAbaEcb;
        else
            *pCryptAlg = BCMD_XptSecCryptoAlg_e3DesAbaCbc;
    }
        break;
#if SUPPORT_NON_DTV_CRYPTO || SUPPORT_AES_FOR_DTV
    case NEXUS_SecurityAlgorithm_eAes: {
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eEcb)
            *pCryptAlg = BCMD_XptSecCryptoAlg_eAesEcb;
        else
            *pCryptAlg = BCMD_XptSecCryptoAlg_eAesCbc;
    }
        break;
#endif /* SUPPORT_NON_DTV_CRYPTO */
    case NEXUS_SecurityAlgorithm_eMulti2: {
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eEcb)
            *pCryptAlg = BCMD_XptSecCryptoAlg_eMulti2Ecb;
        else
            *pCryptAlg = BCMD_XptSecCryptoAlg_eMulti2Cbc;
        break;
    }
    default:
#if SUPPORT_NON_DTV_CRYPTO
        *pCryptAlg = BCMD_XptSecCryptoAlg_eAesEcb;
#else
        * pCryptAlg = BCMD_XptSecCryptoAlg_eDesEcb;
#endif /* SUPPORT_NON_DTV_CRYPTO */
        BDBG_WRN(("Unrecognized or unsupported algorithm"));
        BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }

    BDBG_LEAVE(NEXUS_Security_GetHsmCaCpRmxAlgorithmKeySetting);
    return false;
}

static bool NEXUS_Security_GetHsmM2MAlgorithmKeySetting(const NEXUS_SecurityAlgorithmSettings *pSettings, unsigned int * pCryptAlg, BHSM_ResidueMode_e * pResidualMode)
{
    BDBG_ENTER(NEXUS_Security_GetHsmM2MAlgorithmKeySetting);

    switch (pSettings->algorithm) {
    case NEXUS_SecurityAlgorithm_e3DesAba: {
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eEcb)
            *pCryptAlg = BCMD_M2MSecCryptoAlg_e3DesAbaEcb;
        else
            *pCryptAlg = BCMD_M2MSecCryptoAlg_e3DesAbaCbc;
    }
        break;
    case NEXUS_SecurityAlgorithm_e3DesAbc: {
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eEcb)
            *pCryptAlg = BCMD_M2MSecCryptoAlg_e3DesAbcEcb;
        else
            *pCryptAlg = BCMD_M2MSecCryptoAlg_e3DesAbcCbc;
    }
        break;
    case NEXUS_SecurityAlgorithm_eDes: {
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eEcb)
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eDesEcb;
        else
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eDesCbc;
    }
        break;
#if SUPPORT_NON_DTV_CRYPTO
    case NEXUS_SecurityAlgorithm_eC2: {
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eEcb)
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eC2Ecb;
        else
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eC2Cbc;
    }
        break;
#endif /* SUPPORT_NON_DTV_CRYPTO */
#if SUPPORT_NON_DTV_CRYPTO
    case NEXUS_SecurityAlgorithm_eCss:
        *pCryptAlg = BCMD_M2MSecCryptoAlg_eCss;
        break;
#endif /* SUPPORT_NON_DTV_CRYPTO */
    case NEXUS_SecurityAlgorithm_eM6Ke:
        *pCryptAlg = BCMD_M2MSecCryptoAlg_eM6KE;
        break;
    case NEXUS_SecurityAlgorithm_eM6: {
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eEcb)
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eM6Ecb;
        else
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eM6Cbc;
    }
        break;
    case NEXUS_SecurityAlgorithm_eAes: {
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eEcb)
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eAes128Ecb;
        else if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eCbc)
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eAes128Cbc;
        else
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eAesCounter;
    }
        break;
    case NEXUS_SecurityAlgorithm_eAes192: {
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eEcb)
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eAes192Ecb;
        else if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eCbc)
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eAes192Cbc;
        else
            *pCryptAlg = BCMD_M2MSecCryptoAlg_eAes192Counter;
    }
        break;
#if MSDRM_PD_SUPPORT
        case NEXUS_SecurityAlgorithm_eWMDrmPd:
        * pCryptAlg = BCMD_M2MSecCryptoAlg_eWMDrmPd;
        break;
        case NEXUS_SecurityAlgorithm_eRc4:
        * pCryptAlg = BCMD_M2MSecCryptoAlg_eRc4;
        break;
#endif
    default:
        *pCryptAlg = BCMD_M2MSecCryptoAlg_eAes128Ecb;
        break;
    }

    if ( (*pCryptAlg == BCMD_M2MSecCryptoAlg_eAesCounter) || (*pCryptAlg == BCMD_M2MSecCryptoAlg_eAes192Counter)) {
        *pResidualMode = (BHSM_ResidueMode_e) pSettings->aesCounterSize;
        return true;
    }

    BDBG_LEAVE(NEXUS_Security_GetHsmM2MAlgorithmKeySetting);
    return false;
}

static void NEXUS_Security_GetHsmAlgorithmKeySetting( NEXUS_KeySlotHandle keyHandle,
                                const NEXUS_SecurityAlgorithmSettings *pSettings,
                                unsigned int * pCryptAlg,
                                BHSM_ResidueMode_e * pResidualMode )
{
    BDBG_ENTER(NEXUS_Security_GetHsmAlgorithmKeySetting);

    if( keyHandle->settings.keySlotEngine != NEXUS_SecurityEngine_eM2m )
    {
        if (NEXUS_Security_GetHsmCaCpRmxAlgorithmKeySetting(pSettings, pCryptAlg, pResidualMode))
        {
            return;
        }
    }
    else
    {
        if (NEXUS_Security_GetHsmM2MAlgorithmKeySetting(pSettings, pCryptAlg, pResidualMode))
        {
            return;
        }
    }

    switch( pSettings->terminationMode )
    {
    case NEXUS_SecurityTerminationMode_eClear:
        *pResidualMode = BHSM_ResidueMode_eUnscrambled;
        break;
    case NEXUS_SecurityTerminationMode_eBlock:
        *pResidualMode = BHSM_ResidueMode_eResidueBlock;
        break;
    case NEXUS_SecurityTerminationMode_eCipherStealing:
        *pResidualMode = BHSM_ResidueMode_eCipherTextStealing;
        break;
#if 0 /*RRLee BHSM_ResidueMode_eCipherStealingComcast is not defined???*/
        case NEXUS_SecurityTerminationMode_eCipherStealingComcast:
        * pResidualMode = BHSM_ResidueMode_eCipherStealingComcast;
        break;
#endif
    default:
        *pResidualMode = BHSM_ResidueMode_eUnscrambled;
        break;
   }

    BDBG_LEAVE(NEXUS_Security_GetHsmAlgorithmKeySetting);
    return;
}
#endif /* HSM_IS_ASKM */

static NEXUS_Error NEXUS_Security_GetHsmDestBlkType( NEXUS_KeySlotHandle keyslot, NEXUS_SecurityAlgorithmConfigDestination dest, BCMD_KeyDestBlockType_e *pType )
{
    BDBG_ENTER(NEXUS_Security_GetHsmDestBlkType);

    BDBG_MSG(("NEXUS_Security_GetHsmDestBlkType -- engine[%d] dest[%d] *pType[%d]" , keyslot->settings.keySlotEngine , dest , *pType ));

    switch( keyslot->settings.keySlotEngine)
    {
        case NEXUS_SecurityEngine_eCa:
        {
            *pType = BCMD_KeyDestBlockType_eCA;
            break;
        }
        case NEXUS_SecurityEngine_eM2m:
        {
            *pType = BCMD_KeyDestBlockType_eMem2Mem;
            break;
        }
        case NEXUS_SecurityEngine_eCaCp:
        {
          #if HSM_IS_ASKM
            if (dest == NEXUS_SecurityAlgorithmConfigDestination_eCa)
            {
                *pType = BCMD_KeyDestBlockType_eCA;
            }
            else if (dest == NEXUS_SecurityAlgorithmConfigDestination_eCpd)
            {
                *pType = BCMD_KeyDestBlockType_eCPDescrambler;
            }
            else if (dest == NEXUS_SecurityAlgorithmConfigDestination_eCps)
            {
                *pType = BCMD_KeyDestBlockType_eCPScrambler;
            }
          #else
                *pType = (dest==NEXUS_SecurityAlgorithmConfigDestination_eCa) ? BCMD_KeyDestBlockType_eCA : BCMD_KeyDestBlockType_eRmx;
          #endif
            break;
        }
        case NEXUS_SecurityEngine_eCp:
        {
          #if HSM_IS_ASKM
            if (dest == NEXUS_SecurityAlgorithmConfigDestination_eCpd)
            {
                *pType = BCMD_KeyDestBlockType_eCPDescrambler;
            }
            else if (dest == NEXUS_SecurityAlgorithmConfigDestination_eCps)
            {
                *pType = BCMD_KeyDestBlockType_eCPScrambler;
            }
          #else
            *pType = BCMD_KeyDestBlockType_eRmx;
          #endif
            break;
        }
        case NEXUS_SecurityEngine_eRmx:
        {
          #if HSM_IS_ASKM_40NM
            BDBG_ERR(("Remux is not supported on 40nm HSM"));
            *pType = BCMD_KeyDestBlockType_eCA;
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
          #else
            *pType = BCMD_KeyDestBlockType_eRmx;
          #endif
            break;
        }
        default:
        {
            return BERR_TRACE( NEXUS_NOT_SUPPORTED ); /* There is no meaningful default, error. */
        }
    }

    BDBG_MSG(("NEXUS_Security_GetHsmDestBlkType -- engine[%d] dest[%d] *pType[%d]" , keyslot->settings.keySlotEngine, dest , *pType ));

    BDBG_LEAVE(NEXUS_Security_GetHsmDestBlkType);
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Security_GetHsmDestEntryType(NEXUS_SecurityKeyType keytype, BCMD_KeyDestEntryType_e *pType)
{
    switch (keytype) {
    case NEXUS_SecurityKeyType_eEven:
        *pType = BCMD_KeyDestEntryType_eEvenKey;
        break;
    case NEXUS_SecurityKeyType_eOdd:
        *pType = BCMD_KeyDestEntryType_eOddKey;
        break;
    case NEXUS_SecurityKeyType_eClear:
       #if HSM_IS_ASKM_40NM
        *pType = BCMD_KeyDestEntryType_eClearKey;
       #else
        *pType = BCMD_KeyDestEntryType_eReserved0;
       #endif
        break;
   #if !HSM_IS_ASKM_40NM
    case NEXUS_SecurityKeyType_eIv:
        *pType = BCMD_KeyDestEntryType_eIV;
        break;
   #endif
    default:
        *pType = BCMD_KeyDestEntryType_eOddKey;
        break;
    }
    return NEXUS_SUCCESS;
}

#if HSM_IS_ASKM_40NM
static NEXUS_Error NEXUS_Security_GetHsmDestIVType(NEXUS_SecurityKeyIVType keyIVtype, BCMD_KeyDestIVType_e *pType)
{
    switch (keyIVtype) {
    case NEXUS_SecurityKeyIVType_eNoIV:
        *pType = BCMD_KeyDestIVType_eNoIV;
        break;
    case NEXUS_SecurityKeyIVType_eIV:
        *pType = BCMD_KeyDestIVType_eIV;
        break;
    case NEXUS_SecurityKeyIVType_eAesShortIV:
        *pType = BCMD_KeyDestIVType_eAesShortIV;
        break;
    default:
        *pType = BCMD_KeyDestIVType_eNoIV;
        break;
    }
    return NEXUS_SUCCESS;
}

#endif
#ifdef NEXUS_SECURITY_SC_VALUE
static int g_scValues[NEXUS_SecurityAlgorithmScPolarity_eMax] = { 0, 1, 2, 3 };
#endif


#if HSM_IS_ASKM_28NM_ZEUS_4_0

/* HSM_IS_ASKM_28NM_ZEUS_4_0 */
static void NEXUS_Security_P_SetScValues(BHSM_ConfigAlgorithmIO_t *pConfigAlgorithmIO, const NEXUS_SecurityAlgorithmSettings *pSettings, NEXUS_SecurityKeyType keyType) {

#ifdef NEXUS_SECURITY_SC_VALUE
    int keytypeScValues[NEXUS_SecurityKeyType_eMax] = { 3, 2, 0, 0, 0 };

    if (pSettings->modifyScValue[NEXUS_SecurityPacketType_eGlobal])
    {
        pConfigAlgorithmIO->cryptoAlg.GpipeSCVal = g_scValues[pSettings->scValue[NEXUS_SecurityPacketType_eGlobal]];
    }
    else
    {
        pConfigAlgorithmIO->cryptoAlg.GpipeSCVal = keytypeScValues[keyType];
    }
    if (pSettings->modifyScValue[NEXUS_SecurityPacketType_eRestricted])
    {
        pConfigAlgorithmIO->cryptoAlg.RpipeSCVal = g_scValues[pSettings->scValue[NEXUS_SecurityPacketType_eRestricted]];
    }
    else
    {
        pConfigAlgorithmIO->cryptoAlg.RpipeSCVal = keytypeScValues[keyType];
    }
#else
    BSTD_UNUSED(pConfigAlgorithmIO);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(keyType);
#endif
}


/* HSM_IS_ASKM_28NM_ZEUS_4_0 */
NEXUS_Error NEXUS_Security_ConfigAlgorithm(NEXUS_KeySlotHandle keyHandle, const NEXUS_SecurityAlgorithmSettings *pSettings)
{
    BHSM_ConfigAlgorithmIO_t         configAlgorithmIO;
    BCMD_XptSecKeySlot_e             keySlotType;
    unsigned int                     unKeySlotNum = 0;
    bool                             bConfigOddAndEven = false;
    NEXUS_Error                      rc = NEXUS_SUCCESS;
    BCMD_KeyDestBlockType_e          blockType = BCMD_KeyDestBlockType_eCA;
    BHSM_ConfigKeySlotIDDataIO_t     configKeySlotIDDataIO;
    BCMD_XptM2MSecCryptoAlg_e        cryptAlg;
    BCMD_CipherModeSelect_e          cipherMode;
    BCMD_TerminationMode_e           terminationMode;
    BCMD_IVSelect_e                  ivModeSelect;
    BCMD_Aes128_CounterSize_e        counterSize;
    BCMD_SolitarySelect_e            solitarySelect;
    NEXUS_SecurityKeySource          keySrc;
    bool                             bAVKeyladder = false;
    NEXUS_SecurityKeyType            keyDestEntryType;
    NEXUS_Security_P_KeySlotData *pKeySlotData;

    BDBG_ENTER(NEXUS_Security_ConfigAlgorithm);

    BDBG_OBJECT_ASSERT(keyHandle, NEXUS_KeySlot);

    keyDestEntryType = pSettings->keyDestEntryType;

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    if( keyHandle->keyslotType == BCMD_XptSecKeySlot_eType3 )
    {
        /* Clear key entry must be used for Type 3, m2m  */
        keyDestEntryType = NEXUS_SecurityKeyType_eClear;
    }
#endif

    pKeySlotData = keyHandle->security.data;

    if (!pKeySlotData) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    pKeySlotData->algorithm = pSettings->algorithm;

    BKNI_Memset( &configAlgorithmIO, 0, sizeof (configAlgorithmIO) );

    keySlotType  = keyHandle->keyslotType;      /* For M2M DMA key slot, this has to be set to the right type, starting with Zeus 4.0 */
    unKeySlotNum = keyHandle->keySlotNumber;
    keySrc       = pSettings->keySource;        /* to be used to support AV Keyladder */

    NEXUS_Security_GetHsmAlgorithmKeySetting( keyHandle, pSettings, &cryptAlg, &cipherMode, &terminationMode );
    ivModeSelect   = pSettings->ivMode;
    solitarySelect = pSettings->solitarySelect;
    counterSize    = pSettings->aesCounterSize;

    rc = NEXUS_Security_GetHsmDestBlkType(keyHandle, pSettings->dest, &blockType);
    if (rc)  return BERR_TRACE(rc);

    bConfigOddAndEven = (keyDestEntryType == NEXUS_SecurityKeyType_eOddAndEven);

    /* if req. is for AV Keyladder, config only for KeyDestEntryType requested */
    if (keySrc == NEXUS_SecurityKeySource_eAvCPCW || keySrc == NEXUS_SecurityKeySource_eAvCW)
    {
        bConfigOddAndEven = false;
        bAVKeyladder = true;
    }

    configAlgorithmIO.keyDestBlckType = blockType;
    configAlgorithmIO.keyDestEntryType = NEXUS_Security_MapNexusKeyDestToHsm( keyHandle, keyDestEntryType );
    configAlgorithmIO.caKeySlotType = keySlotType;
    configAlgorithmIO.unKeySlotNum  = unKeySlotNum;

    if (keyHandle->settings.keySlotEngine == NEXUS_SecurityEngine_eM2m)
    {
        if (pSettings->operation == NEXUS_SecurityOperation_eEncrypt)
        {
            configAlgorithmIO.cryptoAlg.cryptoOp = BHSM_M2mAuthCtrl_eScramble;
        }
        else if (pSettings->operation == NEXUS_SecurityOperation_eDecrypt)
        {
            configAlgorithmIO.cryptoAlg.cryptoOp = BHSM_M2mAuthCtrl_eDescramble;
        }
        else
        {
            configAlgorithmIO.cryptoAlg.cryptoOp = BHSM_M2mAuthCtrl_ePassThrough;
        }
    }

    configAlgorithmIO.cryptoAlg.xptSecAlg          = cryptAlg;
    configAlgorithmIO.cryptoAlg.cipherDVBCSA2Mode  = cipherMode;
    configAlgorithmIO.cryptoAlg.termCounterMode    = terminationMode;

    if( cipherMode == BCMD_CipherModeSelect_eCTR){
        configAlgorithmIO.cryptoAlg.IVModeCounterSize = pSettings->aesCounterSize;
    }else{
        configAlgorithmIO.cryptoAlg.IVModeCounterSize = ivModeSelect;
    }
    configAlgorithmIO.cryptoAlg.solitaryMode       = solitarySelect;
    configAlgorithmIO.cryptoAlg.keyOffset          = pSettings->keyOffset;
    configAlgorithmIO.cryptoAlg.ivOffset           = pSettings->ivOffset;

    /* Force key & IV to external if the algorithm is WMDRMPD*/
    if( (pSettings->algorithm == NEXUS_SecurityAlgorithm_eWMDrmPd) || (pSettings->algorithm == NEXUS_SecurityAlgorithm_eRc4) )
    {
        configAlgorithmIO.cryptoAlg.bUseExtKey = true;
        configAlgorithmIO.cryptoAlg.bUseExtIV  = true;
    }
    else
    {
        configAlgorithmIO.cryptoAlg.bUseExtKey = pSettings->enableExtKey;
        configAlgorithmIO.cryptoAlg.bUseExtIV  = pSettings->enableExtIv;
    }

    configAlgorithmIO.cryptoAlg.bGpipeEnable = pSettings->bGlobalEnable;
    configAlgorithmIO.cryptoAlg.bRpipeEnable = pSettings->bRestrictEnable;

    NEXUS_Security_P_SetScValues( &configAlgorithmIO, pSettings, bConfigOddAndEven ? NEXUS_SecurityKeyType_eOdd : keyDestEntryType );

    #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
    /* These settings are now moved into key slot's Global Mode word */
    configAlgorithmIO.cryptoAlg.bRpipeFromGregion  = pSettings->RpipeFromGregion;
    configAlgorithmIO.cryptoAlg.bRpipeFromRregion  = pSettings->RpipeFromRregion;
    configAlgorithmIO.cryptoAlg.bRpipeToGregion    = pSettings->RpipeToGregion;
    configAlgorithmIO.cryptoAlg.bRpipeToRregion    = pSettings->RpipeToRregion;
    configAlgorithmIO.cryptoAlg.bGpipeFromGregion  = pSettings->GpipeFromGregion;
    configAlgorithmIO.cryptoAlg.bGpipeFromRregion  = pSettings->GpipeFromRregion;
    configAlgorithmIO.cryptoAlg.bGpipeToGregion    = pSettings->GpipeToGregion;
    configAlgorithmIO.cryptoAlg.bGpipeToRregion    = pSettings->GpipeToRregion;
    configAlgorithmIO.cryptoAlg.bEncryptBeforeRave = pSettings->bEncryptBeforeRave;
    #endif

    configAlgorithmIO.cryptoAlg.MSCLengthSelect     = pSettings->mscLengthSelect;
    configAlgorithmIO.cryptoAlg.customerType        = (BCMD_XptKeyTableCustomerMode_e)pSettings->customerType;
    configAlgorithmIO.cryptoAlg.MACRegSelect        = pSettings->macRegSelect;
    configAlgorithmIO.cryptoAlg.MACNonSecureRegRead = pSettings->macNonSecureRegRead;
    if (pSettings->bMulti2Config)
    {
        configAlgorithmIO.bMulti2KeyConfig = true;
        configAlgorithmIO.cryptoAlg.Multi2RoundCount   = pSettings->multi2RoundCount;
        configAlgorithmIO.cryptoAlg.Multi2SysKeySelect = pSettings->multi2KeySelect;
    }
    configAlgorithmIO.cryptoAlg.IdertoModEnable    = pSettings->IrModEnable;

    configAlgorithmIO.cryptoAlg.DVBCSA3dvbcsaVar   = pSettings->dvbCsa3dvbcsaVar;
    configAlgorithmIO.cryptoAlg.DVBCSA3permutation = pSettings->dvbCsa3permutation;
    configAlgorithmIO.cryptoAlg.DVBCSA3modXRC      = pSettings->dvbCsa3modXRC;

    configAlgorithmIO.cryptoAlg.DVBCSA2keyCtrl     = pSettings->dvbCsa2keyCtrl;
    configAlgorithmIO.cryptoAlg.DVBCSA2ivCtrl      = pSettings->dvbCsa2ivCtrl;
    configAlgorithmIO.cryptoAlg.DVBCSA2modEnabled  = pSettings->dvbCsa2modEnabled;

    configAlgorithmIO.cryptoAlg.EsModEnable     = pSettings->EsModEnable;

    rc = BHSM_ConfigAlgorithm( g_security.hsm, &configAlgorithmIO );
    if (rc)
    {
        return BERR_TRACE(MAKE_HSM_ERR(rc));
    }

    BKNI_Memset( &configKeySlotIDDataIO, 0, sizeof(configKeySlotIDDataIO) );

    configKeySlotIDDataIO.keyDestBlckType  = blockType;
    configKeySlotIDDataIO.keyDestEntryType = NEXUS_Security_MapNexusKeyDestToHsm( keyHandle, keyDestEntryType );
    configKeySlotIDDataIO.keyDestIVType    = BCMD_KeyDestIVType_eNoIV;
    configKeySlotIDDataIO.unKeySlotNum     = unKeySlotNum;
    configKeySlotIDDataIO.caKeySlotType    = keySlotType;
    configKeySlotIDDataIO.CAVendorID       = pSettings->caVendorID;
    configKeySlotIDDataIO.STBOwnerIDSelect = pSettings->otpId;
    configKeySlotIDDataIO.ModuleID         = pSettings->askmModuleID;
    configKeySlotIDDataIO.key2Select       = pSettings->key2Select;

    rc = BHSM_ConfigKeySlotIDData( g_security.hsm, &configKeySlotIDDataIO );
    if( rc )
    {
        return BERR_TRACE(MAKE_HSM_ERR(rc));
    }

    /* We must send a LRUK to pass keyslot configuraiton to BSP */
    if( keyHandle->settings.keySlotEngine == NEXUS_SecurityEngine_eM2m )
    {
        if(    configAlgorithmIO.cryptoAlg.bUseExtKey
            || configAlgorithmIO.cryptoAlg.bUseExtIV
            || (configAlgorithmIO.cryptoAlg.cryptoOp == BHSM_M2mAuthCtrl_ePassThrough) )
        {
            BHSM_LoadRouteUserKeyIO_t loadRouteUserKeyIO;
            BKNI_Memset(&loadRouteUserKeyIO, 0, sizeof(loadRouteUserKeyIO));
            loadRouteUserKeyIO.bIsRouteKeyRequired = true;
            loadRouteUserKeyIO.keyDestBlckType = BCMD_KeyDestBlockType_eMem2Mem;
            loadRouteUserKeyIO.keySize.eKeySize = BCMD_KeySize_e128;
            loadRouteUserKeyIO.bIsRouteKeyRequired = true;
            loadRouteUserKeyIO.keyDestEntryType = configKeySlotIDDataIO.keyDestEntryType;
            loadRouteUserKeyIO.caKeySlotType = keySlotType;
            loadRouteUserKeyIO.unKeySlotNum = unKeySlotNum;
            loadRouteUserKeyIO.keyMode= BCMD_KeyMode_eRegular;

            if( configAlgorithmIO.cryptoAlg.cryptoOp == BHSM_M2mAuthCtrl_ePassThrough )
            {
                 configAlgorithmIO.cryptoAlg.xptSecAlg = BCMD_XptM2MSecCryptoAlg_eAes128;  /* select a generic algorithm  */
            }

            rc = BHSM_LoadRouteUserKey (g_security.hsm, &loadRouteUserKeyIO);
            if( rc )
            {
                return BERR_TRACE(MAKE_HSM_ERR(rc));
            }
        }
    }

    if (bConfigOddAndEven)
    {
        configAlgorithmIO.keyDestEntryType = BCMD_KeyDestEntryType_eEvenKey;
        if (keyHandle->settings.keySlotEngine != NEXUS_SecurityEngine_eM2m)
        {
            NEXUS_Security_P_SetScValues(&configAlgorithmIO, pSettings, NEXUS_SecurityKeyType_eEven );
        }
        rc = BHSM_ConfigAlgorithm(g_security.hsm, &configAlgorithmIO);
        if (rc)
        {
            return BERR_TRACE(MAKE_HSM_ERR(rc));
        }
        /* Call BHSM_configKeySlotIDData() to set up ID part of  configuration odd key */
        configKeySlotIDDataIO.keyDestEntryType = BCMD_KeyDestEntryType_eEvenKey;
        rc = BHSM_ConfigKeySlotIDData(g_security.hsm, &configKeySlotIDDataIO);
        if (rc)
        {
            return BERR_TRACE(MAKE_HSM_ERR(rc));
        }
    }

    BDBG_LEAVE(NEXUS_Security_ConfigAlgorithm);
    return rc;
}


#else   /* HSM_IS_ASKM_28NM_ZEUS_4_0 */

/* NOT HSM_IS_ASKM_28NM_ZEUS_4_0 */
static void NEXUS_Security_P_SetScValues(BHSM_ConfigAlgorithmIO_t *pConfigAlgorithmIO, const NEXUS_SecurityAlgorithmSettings *pSettings, NEXUS_SecurityKeyType keyType) {
#if HSM_IS_ASKM_40NM && defined(NEXUS_SECURITY_SC_VALUE)
    int keytypeScValues[NEXUS_SecurityKeyType_eMax] = { 3, 2, 0, 0, 0 };
    if (pSettings->modifyScValue[NEXUS_SecurityPacketType_eGlobal]) {
        pConfigAlgorithmIO->cryptoAlg.caCryptAlg.globalSCVal = g_scValues[pSettings->scValue[NEXUS_SecurityPacketType_eGlobal]];
    } else {
        pConfigAlgorithmIO->cryptoAlg.caCryptAlg.globalSCVal = keytypeScValues[keyType];
    }
    if (pSettings->modifyScValue[NEXUS_SecurityPacketType_eRestricted]) {
        pConfigAlgorithmIO->cryptoAlg.caCryptAlg.restrSCVal = g_scValues[pSettings->scValue[NEXUS_SecurityPacketType_eRestricted]];
    } else {
        pConfigAlgorithmIO->cryptoAlg.caCryptAlg.restrSCVal = keytypeScValues[keyType];
    }
#else
    BSTD_UNUSED(pConfigAlgorithmIO);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(keyType);
#endif
}

/* NOT HSM_IS_ASKM_28NM_ZEUS_4_0 */
NEXUS_Error NEXUS_Security_ConfigAlgorithm(NEXUS_KeySlotHandle keyHandle, const NEXUS_SecurityAlgorithmSettings *pSettings)
{
    BHSM_ConfigAlgorithmIO_t configAlgorithmIO;
    BCMD_XptSecKeySlot_e keySlotType;
    unsigned int unKeySlotNum = 0;
    bool bConfigOddAndEven = false;
#if !HSM_IS_ASKM_40NM
    bool bConfigClear = false;
#endif

#if HSM_IS_ASKM_40NM_ZEUS_3_0
    BCMD_Aes128_CounterSize_e        counterSize;
#endif
    NEXUS_Error rc = NEXUS_SUCCESS;
    BCMD_KeyDestBlockType_e blockType = BCMD_KeyDestBlockType_eCA;
#if HSM_IS_ASKM
    BHSM_ConfigKeySlotIDDataIO_t configKeySlotIDDataIO;
    BCMD_XptM2MSecCryptoAlg_e cryptAlg;
    BCMD_CipherModeSelect_e cipherMode;
    BCMD_TerminationMode_e terminationMode;
    BCMD_IVSelect_e ivModeSelect;
    BCMD_SolitarySelect_e solitarySelect;
#else
    unsigned int cryptAlg;
    BHSM_ResidueMode_e residualMode = BHSM_ResidueMode_eUnscrambled;
#endif
    NEXUS_SecurityKeySource keySrc;
    bool bAVKeyladder = false;
    NEXUS_Security_P_KeySlotData *pKeySlotData;

    BDBG_ENTER(NEXUS_Security_ConfigAlgorithm);


    BDBG_OBJECT_ASSERT(keyHandle, NEXUS_KeySlot);
    pKeySlotData = keyHandle->security.data;
    if (!pKeySlotData) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    pKeySlotData->algorithm = pSettings->algorithm;

    BKNI_Memset(&configAlgorithmIO, 0, sizeof (configAlgorithmIO));

    keySlotType = keyHandle->keyslotType;
    unKeySlotNum = keyHandle->keySlotNumber;
    keySrc = pSettings->keySource;

#if HSM_IS_ASKM
    NEXUS_Security_GetHsmAlgorithmKeySetting(keyHandle, pSettings,
            &cryptAlg,
            &cipherMode,
            &terminationMode);
    ivModeSelect   = pSettings->ivMode;
    solitarySelect = pSettings->solitarySelect;
#if HSM_IS_ASKM_40NM_ZEUS_3_0
    counterSize    = pSettings->aesCounterSize;
#endif
#else
    NEXUS_Security_GetHsmAlgorithmKeySetting(keyHandle, pSettings, &cryptAlg, &residualMode);
#endif

    rc = NEXUS_Security_GetHsmDestBlkType(keyHandle, pSettings->dest, &blockType);
    if (rc) return BERR_TRACE(rc);

    /* coverity[const] */
    switch (blockType) {
    case BCMD_KeyDestBlockType_eCA:
#if HSM_IS_ASKM
    case BCMD_KeyDestBlockType_eCPScrambler:
    case BCMD_KeyDestBlockType_eCPDescrambler:
#endif
        bConfigOddAndEven = (pSettings->keyDestEntryType == NEXUS_SecurityKeyType_eOddAndEven);
        break;
#if !HSM_IS_ASKM_40NM
    case BCMD_KeyDestBlockType_eRmx:
        bConfigOddAndEven = (pSettings->keyDestEntryType == NEXUS_SecurityKeyType_eOddAndEven);
        bConfigClear = true;
        break;
#endif
    default:
        break;
    }

    /* if req. is for AV Keyladder, config only for KeyDestEntryType requested */
    if (keySrc == NEXUS_SecurityKeySource_eAvCPCW || keySrc == NEXUS_SecurityKeySource_eAvCW) {
        bConfigOddAndEven = false;
#if !HSM_IS_ASKM_40NM
        bConfigClear = false;
#endif
        bAVKeyladder = true;
    }

#if !HSM_IS_ASKM_40NM
    if (!bAVKeyladder) /* keep the keySource setting for AV Keyladder */
        configAlgorithmIO.keySource = BCMD_KeyRamBuf_eFirstRam; /*BCMD_KeyRamBuf_eKey5KeyLadder2; */
    else {
#if HSM_IS_ASKM
        configAlgorithmIO.keySource = keySrc;
#else
        if (keySrc == NEXUS_SecurityKeySource_eAvCPCW)
            configAlgorithmIO.keySource = BCMD_KeyRamBuf_eReserved0;
        else
            configAlgorithmIO.keySource = BCMD_KeyRamBuf_eReserved1;
#endif
    }
#endif
    configAlgorithmIO.keyDestBlckType = blockType;
    if( bAVKeyladder ) /* keep the KeyDestEntryType as requested for AV Keyladder */
    {
        configAlgorithmIO.keyDestEntryType = NEXUS_Security_MapNexusKeyDestToHsm(keyHandle, pSettings->keyDestEntryType);
    }
    else
    {
        if (bConfigOddAndEven)
        {
            configAlgorithmIO.keyDestEntryType = BCMD_KeyDestEntryType_eOddKey;
        }
        else
        {
            configAlgorithmIO.keyDestEntryType = NEXUS_Security_MapNexusKeyDestToHsm(keyHandle, pSettings->keyDestEntryType);
        }
    }
    configAlgorithmIO.caKeySlotType = keySlotType;
    configAlgorithmIO.unKeySlotNum = unKeySlotNum;
    if (keyHandle->settings.keySlotEngine == NEXUS_SecurityEngine_eM2m)
    {
        BKNI_Memset(&(configAlgorithmIO.cryptoAlg.m2mCryptAlg), 0, sizeof(configAlgorithmIO.cryptoAlg.m2mCryptAlg));

        configAlgorithmIO.cryptoAlg.m2mCryptAlg.m2mSecAlg = cryptAlg;
#if HSM_IS_ASKM
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.m2mCipherMode = cipherMode;
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.TerminationAESCounterKeyMode = terminationMode;
#if BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(3,0)
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.counterSize  = counterSize;

        /* Counter modes 0, 2 and 4 do not support M2M on Zeus 3.0 */
        if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eCounter)
        {
            if ((pSettings->aesCounterMode == NEXUS_SecurityCounterMode_eGenericFullBlocks) ||
                (pSettings->aesCounterMode == NEXUS_SecurityCounterMode_eIvPlayBackFullBlocks) ||
                (pSettings->aesCounterMode == NEXUS_SecurityCounterMode_eSkipPesHeaderAllBlocks))
            {
                BDBG_ERR(("Counter modes 0, 2 and 4 do not support M2M on Zeus 3.0."));
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
        }
#endif
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.IVModeSelect = ivModeSelect;
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.SolitarySelect = solitarySelect;
#else
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.ucAESCounterKeyMode = residualMode;
#endif
        if (pSettings->operation == NEXUS_SecurityOperation_eEncrypt)
            configAlgorithmIO.cryptoAlg.m2mCryptAlg.ucAuthCtrl = BHSM_M2mAuthCtrl_eScramble;
        else if (pSettings->operation == NEXUS_SecurityOperation_eDecrypt)
            configAlgorithmIO.cryptoAlg.m2mCryptAlg.ucAuthCtrl = BHSM_M2mAuthCtrl_eDescramble;
        else
            configAlgorithmIO.cryptoAlg.m2mCryptAlg.ucAuthCtrl = BHSM_M2mAuthCtrl_ePassThrough;

        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bEnableTimestamp = pSettings->enableTimestamps;
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bMscCtrlSel = pSettings->mscBitSelect;

#if HSM_IS_ASKM
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bDisallowGG = pSettings->bDisallowGG;
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bDisallowGR = pSettings->bDisallowGR;
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bDisallowRG = pSettings->bDisallowRG;
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bDisallowRR = pSettings->bDisallowRR;
#endif

        /*set key & IV to external if the algorithm is WMDRMPD*/
        if ((pSettings->algorithm == NEXUS_SecurityAlgorithm_eWMDrmPd) || (pSettings->algorithm == NEXUS_SecurityAlgorithm_eRc4))
        {
            configAlgorithmIO.cryptoAlg.m2mCryptAlg.bUseExtKey = true;
            configAlgorithmIO.cryptoAlg.m2mCryptAlg.bUseExtIV = true;
        }
        else
        {
            configAlgorithmIO.cryptoAlg.m2mCryptAlg.bUseExtKey = pSettings->enableExtKey;
            configAlgorithmIO.cryptoAlg.m2mCryptAlg.bUseExtIV = pSettings->enableExtIv;
        }
    }
    else
    {
        BKNI_Memset(&(configAlgorithmIO.cryptoAlg.caCryptAlg), 0, sizeof(configAlgorithmIO.cryptoAlg.caCryptAlg));

        configAlgorithmIO.cryptoAlg.caCryptAlg.caSecAlg = cryptAlg;
#if HSM_IS_ASKM
#if HSM_IS_ASKM_40NM
        configAlgorithmIO.cryptoAlg.caCryptAlg.cipherDVBCSA2Mode = cipherMode;
#else
        configAlgorithmIO.cryptoAlg.caCryptAlg.cipherDVBMode = cipherMode;
#endif
        configAlgorithmIO.cryptoAlg.caCryptAlg.terminationMode = terminationMode;
        configAlgorithmIO.cryptoAlg.caCryptAlg.IVMode = ivModeSelect;
        configAlgorithmIO.cryptoAlg.caCryptAlg.solitaryMode = solitarySelect;

#else
        if (cryptAlg != BCMD_XptSecCryptoAlg_eDvb){
            configAlgorithmIO.cryptoAlg.caCryptAlg.residueMode.residueMode = residualMode;
        } else {
            configAlgorithmIO.cryptoAlg.caCryptAlg.residueMode.dvbScrambleLevel = residualMode;
        }
#endif
        configAlgorithmIO.cryptoAlg.caCryptAlg.bRestrictEnable = pSettings->bRestrictEnable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bGlobalEnable = pSettings->bGlobalEnable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.ucMulti2KeySelect = pSettings->multi2KeySelect;
#if HSM_IS_ASKM_40NM
        configAlgorithmIO.cryptoAlg.caCryptAlg.keyOffset = pSettings->keyOffset;
        configAlgorithmIO.cryptoAlg.caCryptAlg.ivOffset = pSettings->ivOffset;
        configAlgorithmIO.cryptoAlg.caCryptAlg.ucMSCLengthSelect = pSettings->mscLengthSelect;
        configAlgorithmIO.cryptoAlg.caCryptAlg.customerType = (BCMD_XptKeyTableCustomerMode_e)pSettings->customerType;
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA2keyCtrl = pSettings->dvbCsa2keyCtrl;
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA2ivCtrl = pSettings->dvbCsa2ivCtrl;
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA2modEnabled = pSettings->dvbCsa2modEnabled;
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA3dvbcsaVar = pSettings->dvbCsa3dvbcsaVar;
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA3permutation = pSettings->dvbCsa3permutation;
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA3modXRC = pSettings->dvbCsa3modXRC;

                configAlgorithmIO.cryptoAlg.caCryptAlg.bUseExtKey = pSettings->enableExtKey;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bUseExtIV = pSettings->enableExtIv;

#else
        configAlgorithmIO.cryptoAlg.caCryptAlg.bAtscMod = pSettings->bAtscModEnable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bAtscScrambleCtrl = pSettings->bScAtscMode;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bGlobalDropPktCtrl = pSettings->bGlobalDropPktEnable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bRestrictDropPktCtrl = pSettings->bRestrictDropPktEnable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bGlobalRegOverwrite = pSettings->bGlobalRegionOverwrite;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bRestrictScMod = pSettings->modifyScValue[NEXUS_SecurityPacketType_eRestricted];
        configAlgorithmIO.cryptoAlg.caCryptAlg.bGlobalScMod = pSettings->modifyScValue[NEXUS_SecurityPacketType_eGlobal];
#endif

#if HSM_IS_ASKM
        configAlgorithmIO.cryptoAlg.caCryptAlg.bEncryptBeforeRave = pSettings->bEncryptBeforeRave;
#else
        configAlgorithmIO.cryptoAlg.caCryptAlg.bEncScPolarity = pSettings->bScPolarityEnable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bSynEnable = pSettings->bSynEnable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bCPDDisable = pSettings->bCPDDisable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bCPSDisable = pSettings->bCPSDisable;
#endif

#if HSM_IS_ASKM_40NM_ZEUS_2_0
        configAlgorithmIO.cryptoAlg.caCryptAlg.bDropRregionPackets = pSettings->bRestrictSourceDropPktEnable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bGpipePackets2Rregion = pSettings->bRoutePipeToRestrictedRegion[NEXUS_SecurityPacketType_eGlobal];
        configAlgorithmIO.cryptoAlg.caCryptAlg.bRpipePackets2Rregion = pSettings->bRoutePipeToRestrictedRegion[NEXUS_SecurityPacketType_eRestricted];
#endif
#ifdef NEXUS_SECURITY_SC_VALUE
#if !HSM_IS_ASKM_40NM
        configAlgorithmIO.cryptoAlg.caCryptAlg.uScValue = g_scValues[pSettings->scValue[NEXUS_SecurityPacketType_eGlobal]];
        if (pSettings->scValue[NEXUS_SecurityPacketType_eRestricted] != NEXUS_SecurityAlgorithmScPolarity_eClear) {
            BDBG_WRN(("SC polarity cannot be differentiated by packet type on this chip.  Ignoring the attempt to set a non-global packet type."));
        }
#endif
#else
        if (pSettings->modifyScValue[NEXUS_SecurityPacketType_eGlobal] || pSettings->modifyScValue[NEXUS_SecurityPacketType_eRestricted]) {
            BDBG_ERR(("You were trying to set SC value without turning on BSP_SC_VALUE_SUPPORT"));
            BDBG_ERR(("Please rebuild nexus with BSP_SC_VALUE_SUPPORT=ON" ));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
#endif
    }

#if HSM_IS_ASKM
    /* Call BHSM_configKeySlotIDData() to set up ID part of  configuration odd key */
    configKeySlotIDDataIO.keyDestBlckType = blockType;
    if (bAVKeyladder)
    {   /* keep the KeyDestEntryType as requested for AV Keyladder */
        configKeySlotIDDataIO.keyDestEntryType = NEXUS_Security_MapNexusKeyDestToHsm(keyHandle, pSettings->keyDestEntryType);
    }
    else
    {
        if( bConfigOddAndEven )
        {
            configKeySlotIDDataIO.keyDestEntryType = BCMD_KeyDestEntryType_eOddKey;
        }
        else
        {
            configKeySlotIDDataIO.keyDestEntryType = NEXUS_Security_MapNexusKeyDestToHsm(keyHandle, pSettings->keyDestEntryType);
        }
    }
#if HSM_IS_ASKM_40NM
    configKeySlotIDDataIO.keyDestIVType = BCMD_KeyDestIVType_eNoIV;
#endif

    configKeySlotIDDataIO.unKeySlotNum = unKeySlotNum;
    configKeySlotIDDataIO.caKeySlotType = keySlotType;
    configKeySlotIDDataIO.CAVendorID = pSettings->caVendorID;
    configKeySlotIDDataIO.STBOwnerIDSelect = pSettings->otpId;
    configKeySlotIDDataIO.ModuleID = pSettings->askmModuleID;
#if HSM_IS_ASKM_40NM
    configKeySlotIDDataIO.key2Select = pSettings->key2Select;
#else
    configKeySlotIDDataIO.TestKey2Select = pSettings->testKey2Select;
#endif

    rc = BHSM_ConfigKeySlotIDData(g_security.hsm, &configKeySlotIDDataIO);
    if (rc)
    {
        return BERR_TRACE(MAKE_HSM_ERR(rc));
    }

#endif
    if (keyHandle->settings.keySlotEngine != NEXUS_SecurityEngine_eM2m)
    {
        NEXUS_Security_P_SetScValues(&configAlgorithmIO, pSettings, bConfigOddAndEven ? NEXUS_SecurityKeyType_eOdd : pSettings->keyDestEntryType );
    }
    rc = BHSM_ConfigAlgorithm(g_security.hsm, &configAlgorithmIO);
    if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

#if HSM_IS_ASKM
    /* We must send a dummy route key command for algorithm setting to take effect on 7420 family chips */
    if (keyHandle->settings.keySlotEngine == NEXUS_SecurityEngine_eM2m)
    {
        if ( (configAlgorithmIO.cryptoAlg.m2mCryptAlg.bUseExtKey == true) ||
                (configAlgorithmIO.cryptoAlg.m2mCryptAlg.bUseExtIV == true))
        {
            BHSM_LoadRouteUserKeyIO_t loadRouteUserKeyIO;
            BKNI_Memset(&loadRouteUserKeyIO, 0, sizeof(loadRouteUserKeyIO));
            loadRouteUserKeyIO.bIsRouteKeyRequired = true;
            loadRouteUserKeyIO.keyDestBlckType = BCMD_KeyDestBlockType_eMem2Mem;
           #if !HSM_IS_ASKM_40NM
            loadRouteUserKeyIO.keySource = BCMD_KeyRamBuf_eFirstRam;
           #endif
            loadRouteUserKeyIO.keySize.eKeySize = BCMD_KeySize_e128;
            loadRouteUserKeyIO.bIsRouteKeyRequired = true;
            loadRouteUserKeyIO.keyDestEntryType = BCMD_KeyDestEntryType_eOddKey;
            loadRouteUserKeyIO.caKeySlotType = keySlotType;
            loadRouteUserKeyIO.unKeySlotNum = unKeySlotNum;
            loadRouteUserKeyIO.keyMode= BCMD_KeyMode_eRegular;
            rc = BHSM_LoadRouteUserKey (g_security.hsm, &loadRouteUserKeyIO);
            if (rc)
            {
                BDBG_ERR(("External Key/IV may not be enabled by OTP"));
                return BERR_TRACE(MAKE_HSM_ERR(rc));
            }
        }
    }
#endif

    if (bConfigOddAndEven) {
#if HSM_IS_ASKM
        /* Call BHSM_configKeySlotIDData() to set up ID part of  configuration odd key */
        configKeySlotIDDataIO.keyDestEntryType = BCMD_KeyDestEntryType_eEvenKey;
        rc = BHSM_ConfigKeySlotIDData(g_security.hsm, &configKeySlotIDDataIO);
        if (rc)
        {
            return BERR_TRACE(MAKE_HSM_ERR(rc));
        }

#endif
        configAlgorithmIO.keyDestEntryType = BCMD_KeyDestEntryType_eEvenKey;
        if (keyHandle->settings.keySlotEngine != NEXUS_SecurityEngine_eM2m)
        {
            NEXUS_Security_P_SetScValues(&configAlgorithmIO, pSettings, NEXUS_SecurityKeyType_eEven );
        }

        rc = BHSM_ConfigAlgorithm(g_security.hsm, &configAlgorithmIO);
        if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }
    }

#if !HSM_IS_ASKM_40NM
    if (bConfigClear) {
#if HSM_IS_ASKM
        /* Call BHSM_configKeySlotIDData() to set up ID part of  configuration odd key */
#if HSM_IS_ASKM_40NM
        configKeySlotIDDataIO.keyDestEntryType = BCMD_KeyDestEntryType_eClearKey;
#else
        configKeySlotIDDataIO.keyDestEntryType = BCMD_KeyDestEntryType_eReserved0;
#endif
        rc = BHSM_ConfigKeySlotIDData(g_security.hsm, &configKeySlotIDDataIO);
        if (rc)
        {
            return BERR_TRACE(MAKE_HSM_ERR(rc));
        }

        /* If destination block is CA and 65-nm platform, we need to alter mode word for clear key slot */
        /* to configure the SC bit modification for CaCp and send out a dummy LRUK command.  This */
        /* works only for 65-nm BSECK 2.0 and later.   */

        if ( blockType == BCMD_KeyDestBlockType_eCA )
        {
            configAlgorithmIO.cryptoAlg.caCryptAlg.caSecAlg = BCMD_XptM2MSecCryptoAlg_eDes;
            configAlgorithmIO.cryptoAlg.caCryptAlg.IVMode = BCMD_IVSelect_eRegular;
#if HSM_IS_ASKM_40NM
            configAlgorithmIO.cryptoAlg.caCryptAlg.cipherDVBCSA2Mode = BCMD_CipherModeSelect_eECB;
            configAlgorithmIO.cryptoAlg.caCryptAlg.terminationMode = BCMD_TerminationMode_eCLEAR;
            configAlgorithmIO.cryptoAlg.caCryptAlg.solitaryMode = BCMD_SolitarySelect_eCLEAR;
#else
            configAlgorithmIO.cryptoAlg.caCryptAlg.cipherDVBMode = BCMD_CipherModeSelect_eECB;
            configAlgorithmIO.cryptoAlg.caCryptAlg.terminationMode = BCMD_TerminationMode_eClear;
            configAlgorithmIO.cryptoAlg.caCryptAlg.solitaryMode = BCMD_SolitarySelect_eClear;
#endif
            /* The following 2 settings are needed so that FW won't reject the key routing command */
            /* associated with this clear key slot for 65-nm platforms */
#if !HSM_IS_ASKM_40NM
            configAlgorithmIO.cryptoAlg.caCryptAlg.bRestrictEnable = false;
            configAlgorithmIO.cryptoAlg.caCryptAlg.bGlobalEnable = false;
#endif
        }

#endif

       #if HSM_IS_ASKM_40NM
        configAlgorithmIO.keyDestEntryType = BCMD_KeyDestEntryType_eClearKey;
       #else
        configAlgorithmIO.keyDestEntryType = BCMD_KeyDestEntryType_eReserved0;
       #endif
        NEXUS_Security_P_SetScValues(&configAlgorithmIO, pSettings, NEXUS_SecurityKeyType_eClear );

        rc = BHSM_ConfigAlgorithm(g_security.hsm, &configAlgorithmIO);
        if (rc)
        {
            return BERR_TRACE(MAKE_HSM_ERR(rc));
        }
#if HSM_IS_ASKM
        /* Must call load key for algorithm setting to take effect */
        if ( blockType==BCMD_KeyDestBlockType_eCA )
        {
            BHSM_LoadRouteUserKeyIO_t loadRouteUserKeyIO;
            BKNI_Memset(&loadRouteUserKeyIO, 0, sizeof(loadRouteUserKeyIO));
#if !HSM_IS_ASKM_40NM
            loadRouteUserKeyIO.keySource = BCMD_KeyRamBuf_eFirstRam;
#endif
            loadRouteUserKeyIO.keySize.eKeySize = BCMD_KeySize_e128;
            loadRouteUserKeyIO.bIsRouteKeyRequired = true;
#if HSM_IS_ASKM_40NM
            loadRouteUserKeyIO.keyDestEntryType = BCMD_KeyDestEntryType_eClearKey;
            loadRouteUserKeyIO.keyDestIVType    = BCMD_KeyDestIVType_eNoIV;
#else
            loadRouteUserKeyIO.keyDestEntryType = BCMD_KeyDestEntryType_eReserved0;
#endif
            loadRouteUserKeyIO.caKeySlotType = keySlotType;
            loadRouteUserKeyIO.unKeySlotNum = unKeySlotNum;
            loadRouteUserKeyIO.keyMode= BCMD_KeyMode_eRegular;
            loadRouteUserKeyIO.keyDestBlckType = blockType;
            rc = BHSM_LoadRouteUserKey (g_security.hsm, &loadRouteUserKeyIO);
        if (rc)
        {
                BDBG_ERR(("Configure Clear key failed.  You may need to update new versions of BSECK"));
                /*return BERR_TRACE(MAKE_HSM_ERR(rc));*/
                rc = NEXUS_SUCCESS; /* Ignore error for now */
            }
        }
#endif

    }
#endif

    BDBG_LEAVE(NEXUS_Security_ConfigAlgorithm);
    return rc;
}


#endif /* HSM_IS_ASKM_28NM_ZEUS_4_0 */

NEXUS_Error NEXUS_KeySlot_GetExternalKeyData(
                                NEXUS_KeySlotHandle keyslot,
                                NEXUS_SecurityAlgorithmConfigDestination dest, /*cpd/cps/cps */
                                NEXUS_SecurityKeyType  keyDestEntryType,       /*odd/eve/clear*/
                                NEXUS_KeySlotExternalKeyData *pKeyData  )
{
#if HSM_IS_ASKM_40NM_ZEUS_3_0
    NEXUS_Error rc = NEXUS_SUCCESS;

    BHSM_KeyLocation_t keyLocation;
    BHSM_ExternalKeyIdentifier_t extKey;

    BDBG_ENTER( NEXUS_KeySlot_GetExternalKeyData );

    if( pKeyData == NULL )
    {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BKNI_Memset( &keyLocation, 0, sizeof(keyLocation) );
    BKNI_Memset( pKeyData, 0, sizeof(*pKeyData) );

    if( dest != NEXUS_SecurityAlgorithmConfigDestination_eCa &&
        dest != NEXUS_SecurityAlgorithmConfigDestination_eCps &&
        dest != NEXUS_SecurityAlgorithmConfigDestination_eCpd &&
        dest != NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem )
    {
        /* invalid destiantiond for external key.  */
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* rc = NEXUS_Security_MapNexusKeySlotTypeToHsm( keyslot->keyslotType, keyslot->settings.keySlotEngine, &keyLocation.caKeySlotType );*/ /* 1,2,3,4,5, ... */
    keyLocation.caKeySlotType = keyslot->keyslotType;
    if( rc != NEXUS_SUCCESS )
    {
        /* invalid destiantion specified.  */
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    rc = NEXUS_Security_GetHsmDestBlkType( keyslot, dest, &keyLocation.keyDestBlckType ); /* CPD/CA/CPS/.../HDMI/...   */
    if( rc != NEXUS_SUCCESS )
    {
        /* invalid destiantion specified.  */
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    keyLocation.unKeySlotNum     = keyslot->keySlotNumber;
    keyLocation.keyDestEntryType = NEXUS_Security_MapNexusKeyDestToHsm( keyslot, keyDestEntryType );  /*odd/eve/clear*/

    rc = BHSM_GetExternalKeyIdentifier( g_security.hsm, &keyLocation, &extKey );
    if( rc != NEXUS_SUCCESS )
    {
        /* failed to get exteral key information for sepcifed parameters */
        return BERR_TRACE(MAKE_HSM_ERR(rc));
    }

    pKeyData->slotIndex = extKey.slotIndex;

    if(extKey.key.valid)
    {
        pKeyData->key.valid = true;
        pKeyData->key.offset = extKey.key.offset;
    }

    if(extKey.iv.valid)
    {
        pKeyData->iv.valid = true;
        pKeyData->iv.offset = extKey.iv.offset;
    }

    BDBG_LEAVE( NEXUS_KeySlot_GetExternalKeyData );
    return rc;

#else /* HSM_IS_ASKM_40NM_ZEUS_3_0 */
    BSTD_UNUSED( keyslot );
    BSTD_UNUSED( dest );
    BSTD_UNUSED( keyDestEntryType );
    BSTD_UNUSED( pKeyData );

    /*  Required only for external key BTP complilation .. not supported before 4.1*/
    return BERR_TRACE( NEXUS_NOT_SUPPORTED );
#endif
}



void NEXUS_Security_GetDefaultClearKey(NEXUS_SecurityClearKey *pClearKey)
{
    BKNI_Memset(pClearKey, 0, sizeof(*pClearKey));
}

NEXUS_Error NEXUS_Security_LoadClearKey(NEXUS_KeySlotHandle keyHandle, const NEXUS_SecurityClearKey * pClearKey)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BCMD_KeyDestBlockType_e   blockType;
    BCMD_KeyDestEntryType_e   entryType = BCMD_KeyDestEntryType_eOddKey;
    BHSM_LoadRouteUserKeyIO_t loadRouteUserKeyIO;
    NEXUS_SecurityKeyType keyEntryType;

#if HSM_IS_ASKM_40NM
    BCMD_KeyDestIVType_e      ivType;
#endif
    NEXUS_Security_P_KeySlotData *pKeySlotData;

    BDBG_ENTER(NEXUS_Security_LoadClearKey);

    BDBG_OBJECT_ASSERT(keyHandle, NEXUS_KeySlot);
#if HSM_IS_ASKM_40NM
    blockType = BCMD_KeyDestBlockType_eCA;
#else
    blockType = BCMD_KeyDestBlockType_eRmx;
#endif

    BDBG_OBJECT_ASSERT(keyHandle, NEXUS_KeySlot);
    pKeySlotData = keyHandle->security.data;
    if (!pKeySlotData) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    keyEntryType = pClearKey->keyEntryType;

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    if( keyHandle->keyslotType == BCMD_XptSecKeySlot_eType3 )
    {
        /* Clear key entry must be used for Type 3, m2m  */
        keyEntryType = NEXUS_SecurityKeyType_eClear;
    }
#endif

    NEXUS_Security_GetHsmDestBlkType(keyHandle, pClearKey->dest, &blockType);
    NEXUS_Security_GetHsmDestEntryType( keyEntryType, &entryType );
#if HSM_IS_ASKM_40NM
    NEXUS_Security_GetHsmDestIVType(pClearKey->keyIVType, &ivType);
#endif

    BKNI_Memset(&loadRouteUserKeyIO, 0, sizeof(loadRouteUserKeyIO));
    if (pClearKey->keySize) {
#if !HSM_IS_ASKM_40NM
        loadRouteUserKeyIO.keySource = BCMD_KeyRamBuf_eFirstRam;
#endif
        if (pClearKey->keySize==8)
            loadRouteUserKeyIO.keySize.eKeySize = BCMD_KeySize_e64;
        else if (pClearKey->keySize==16)
            loadRouteUserKeyIO.keySize.eKeySize = BCMD_KeySize_e128;
        else if (pClearKey->keySize==24)
            loadRouteUserKeyIO.keySize.eKeySize = BCMD_KeySize_e192;
#if HSM_IS_ASKM_40NM_ZEUS_2_0
        else if (pClearKey->keySize==32)
            loadRouteUserKeyIO.keySize.eKeySize = BCMD_KeySize_e256;
#endif
        else {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        BKNI_Memset(loadRouteUserKeyIO.aucKeyData, 0, sizeof(loadRouteUserKeyIO.aucKeyData));
        BKNI_Memcpy(loadRouteUserKeyIO.aucKeyData, pClearKey->keyData, pClearKey->keySize);
        loadRouteUserKeyIO.bIsRouteKeyRequired = true;
        loadRouteUserKeyIO.keyDestBlckType = blockType;
        loadRouteUserKeyIO.keyDestEntryType = entryType;
        loadRouteUserKeyIO.keyMode          = pClearKey->keyMode;
        loadRouteUserKeyIO.caKeySlotType = keyHandle->keyslotType;
        loadRouteUserKeyIO.unKeySlotNum = keyHandle->keySlotNumber;
        loadRouteUserKeyIO.keyMode= BCMD_KeyMode_eRegular;
#if HSM_IS_ASKM_40NM
        loadRouteUserKeyIO.keyDestIVType = ivType;
#endif
#if HSM_IS_ASKM_40NM_ZEUS_2_0
#ifdef NEXUS_SECURITY_SC_VALUE
        if ( pClearKey->sc01Polarity[NEXUS_SecurityPacketType_eGlobal] >= NEXUS_SecurityAlgorithmScPolarity_eMax ||
             pClearKey->sc01Polarity[NEXUS_SecurityPacketType_eRestricted] >= NEXUS_SecurityAlgorithmScPolarity_eMax)
        {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        loadRouteUserKeyIO.GpipeSC01Val = g_scValues[pClearKey->sc01Polarity[NEXUS_SecurityPacketType_eGlobal]];
        loadRouteUserKeyIO.RpipeSC01Val = g_scValues[pClearKey->sc01Polarity[NEXUS_SecurityPacketType_eRestricted]];
#endif
#endif
        rc = BHSM_LoadRouteUserKey(g_security.hsm, &loadRouteUserKeyIO);
        if (rc) {
            return BERR_TRACE(MAKE_HSM_ERR(rc));
        }
    }

    BDBG_LEAVE(NEXUS_Security_LoadClearKey);
    return NEXUS_SUCCESS;
}

NEXUS_KeySlotHandle NEXUS_Security_AllocateKeySlot(const NEXUS_SecurityKeySlotSettings *pSettings)
{
    NEXUS_KeySlotHandle keyHandle = NULL;
    NEXUS_Security_P_KeySlotData *pKeySlotData = NULL;
    BCMD_XptSecKeySlot_e type;
    int rc;

    BDBG_ENTER(NEXUS_Security_AllocateKeySlot);

    /* eGeneric is a special case */
    if( pSettings->keySlotEngine == NEXUS_SecurityEngine_eGeneric )
    {
        keyHandle = NEXUS_KeySlot_Create();

        if ( keyHandle == NULL )
        {
            (void)BERR_TRACE( NEXUS_NOT_AVAILABLE );
        }

        return keyHandle;
    }

    pKeySlotData = BKNI_Malloc(sizeof(*pKeySlotData));
    if (!pKeySlotData)
    {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY );
        return NULL;
    }

    switch (pSettings->keySlotEngine)
    {
        case NEXUS_SecurityEngine_eM2m:
            /* For Zeus 4.x pSettings->keySlotType has the setting needed by the app or NEXUS_SecurityKeySlotType_eAuto.  */
            /* If set to NEXUS_SecurityKeySlotType_eAuto, Type3 will be used. */
            rc = NEXUS_Security_MapNexusKeySlotTypeToHsm( pSettings->keySlotType, pSettings->keySlotEngine, &type );
            if( rc )
            {
                rc = BERR_TRACE( rc );
                goto err_alloc;
            }

           #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
            if ( ( type != BCMD_XptSecKeySlot_eType0 ) &&
                 ( type != BCMD_XptSecKeySlot_eType2 ) &&
                 ( type != BCMD_XptSecKeySlot_eType3 ) &&
                 ( type != BCMD_XptSecKeySlot_eType5 ) )
            {
                /*  Only above types allowed to be used for M2M DMA key slot -- Zeus 4.0+ */
                rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
                goto err_alloc;
            }
           #endif
            rc = NEXUS_Security_AllocateM2mKeySlot(&keyHandle, pSettings, type);
            if( rc )
            {
                rc = BERR_TRACE(rc);
                goto err_alloc;
            }
            break;
        case NEXUS_SecurityEngine_eRmx:
        case NEXUS_SecurityEngine_eCaCp:
        case NEXUS_SecurityEngine_eCa:
            rc = NEXUS_Security_MapNexusKeySlotTypeToHsm( pSettings->keySlotType, pSettings->keySlotEngine, &type );
            if( rc )
            {
                rc = BERR_TRACE( rc );
                goto err_alloc;
            }

            rc = NEXUS_Security_AllocateKeySlotForType( &keyHandle, pSettings, type );
            if( rc )
            {
                rc = BERR_TRACE( rc );
                goto err_alloc;
            }
            break;
        default:
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto err_alloc;
    }

    BKNI_Memset( pKeySlotData, 0, sizeof( *pKeySlotData ) );

    /* For Zeus 4.0+: transport key slots are to be shared between SAGE and host */
    pKeySlotData->keySlotClient = pSettings->client;
    keyHandle->security.data = pKeySlotData;

    keyHandle->settings = *pSettings;

    if (pSettings->keySlotType == NEXUS_SecurityKeySlotType_eAuto)
    {
        rc = NEXUS_Security_MapHsmKeySlotTypeToNexus (keyHandle->keyslotType, &keyHandle->settings.keySlotType);
	    if( rc )
	    {
	        rc = BERR_TRACE( rc );
	    }
	}

    BLST_S_INIT( &pKeySlotData->pidChannelList );
    BLST_S_INSERT_HEAD( &g_security.keyslotList, keyHandle, next );

#if NEXUS_HAS_XPT_DMA
    if( pSettings->keySlotEngine == NEXUS_SecurityEngine_eM2m )
    {
        /* XPT-based DMA requires an internal pid channel. */
        NEXUS_Module_Lock( g_security.moduleSettings.transport );
        pKeySlotData->dmaPidChannelHandle = NEXUS_PidChannel_OpenDma_Priv();
        NEXUS_Module_Unlock( g_security.moduleSettings.transport );
        if ( pKeySlotData->dmaPidChannelHandle == NULL ) {
            BERR_TRACE(NEXUS_NOT_AVAILABLE);
            BLST_S_REMOVE( &g_security.keyslotList, keyHandle, NEXUS_KeySlot, next );
            goto err_alloc;
        }

        keyHandle->dma.pidChannelIndex = NEXUS_PidChannel_GetIndex_isrsafe( pKeySlotData->dmaPidChannelHandle );
        NEXUS_Security_AddPidChannelToKeySlot( keyHandle, keyHandle->dma.pidChannelIndex );
        BDBG_MSG(( "Using DMA pid channel %u", keyHandle->dma.pidChannelIndex ));
    }
#endif

    NEXUS_SECURITY_DUMP_KEYSLOTS;
    return keyHandle;

err_alloc:
    if( pKeySlotData )
    {
        BKNI_Free( pKeySlotData );
    }

    BDBG_LEAVE( NEXUS_Security_AllocateKeySlot );
    return NULL;
}

static void NEXUS_Security_P_FreeKeySlot(NEXUS_KeySlotHandle keyHandle)
{
    BERR_Code rc = 0;
    NEXUS_Security_P_KeySlotData *pKeySlotData;
    NEXUS_Security_P_PidChannelListNode *pPidChannelNode;
    BHSM_M2MKeySlotIO_t  M2MKeySlotIO;
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
    BHSM_KeySlotAllocate_t  keySlotConf;
   #endif

    BDBG_ENTER(NEXUS_Security_P_FreeKeySlot);

    NEXUS_OBJECT_ASSERT(NEXUS_KeySlot, keyHandle);
    pKeySlotData = (NEXUS_Security_P_KeySlotData *)(keyHandle->security.data);

    /* remove all pid channels */
    while ((pPidChannelNode = BLST_S_FIRST(&pKeySlotData->pidChannelList))) {
        if (pPidChannelNode->pidChannel) {
            NEXUS_KeySlot_RemovePidChannel(keyHandle, pPidChannelNode->pidChannel);
        }
        else {
            NEXUS_Security_RemovePidChannelFromKeySlot(keyHandle, pPidChannelNode->pidChannelIndex);
        }
    }

    switch (keyHandle->settings.keySlotEngine)
    {
        case NEXUS_SecurityEngine_eCa:
        case NEXUS_SecurityEngine_eCaCp:
            BDBG_MSG(("Freeing CA keyslot (%d)", keyHandle->keySlotNumber));
            /* TODO: HSM should verify that no pid channels are associated */
          #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
            keySlotConf.client         = pKeySlotData->keySlotClient;
            keySlotConf.keySlotNum     = keyHandle->keySlotNumber;
            keySlotConf.keySlotType    = keyHandle->keyslotType;
            keySlotConf.pidChannelType = BHSM_PidChannelType_ePrimary;
           #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
            rc = BHSM_FreeCAKeySlot( g_security.hsm, &keySlotConf );
           #else
            rc = BHSM_FreeCAKeySlot_v2( g_security.hsm, &keySlotConf );
           #endif
          #else
            rc = BHSM_FreeCAKeySlot(  g_security.hsm
                                    , NEXUS_SECURITY_CACP_INVALID_PIDCHANNEL
                                    , BHSM_PidChannelType_ePrimary
                                    , keyHandle->keyslotType
                                    , keyHandle->keySlotNumber );
          #endif
            if( rc )
            {
                BDBG_ERR(("Error (%d) freeing CA keyslot", rc));
            }
            break;
        case NEXUS_SecurityEngine_eM2m:
            BDBG_MSG(("Freeing m2m keyslot (%d)", keyHandle->keySlotNumber));
            M2MKeySlotIO.keySlotNum  = keyHandle->keySlotNumber;
            M2MKeySlotIO.keySlotType = keyHandle->keyslotType;
            M2MKeySlotIO.client      = pKeySlotData->keySlotClient;
            rc = BHSM_FreeM2MKeySlot(g_security.hsm, &M2MKeySlotIO);
            if( rc )
            {
                BDBG_ERR(("Error (%d) freeing M2M keyslot", rc));
            }
            break;
        case NEXUS_SecurityEngine_eCp:
            BDBG_MSG(("Freeing cp keyslot"));
            BDBG_WRN(("CP keyslots are not currently supported"));
            rc = NEXUS_INVALID_PARAMETER; /* set an error so we don't decrease the power management refcount */
            break;
        case NEXUS_SecurityEngine_eRmx:
            BDBG_MSG(("Freeing rmx keyslot"));
          #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
            keySlotConf.client = pKeySlotData->keySlotClient;
            keySlotConf.keySlotNum = keyHandle->keySlotNumber;
            keySlotConf.keySlotType = keyHandle->keyslotType;
            keySlotConf.pidChannelType = BHSM_PidChannelType_ePrimary;
           #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
            rc = BHSM_FreeCAKeySlot( g_security.hsm, &keySlotConf );
           #else
            rc = BHSM_FreeCAKeySlot_v2( g_security.hsm, &keySlotConf );
           #endif
          #else
            rc = BHSM_FreeCAKeySlot(  g_security.hsm
                                    , NEXUS_SECURITY_CACP_INVALID_PIDCHANNEL
                                    , BHSM_PidChannelType_ePrimary
                                    , keyHandle->keyslotType
                                    , keyHandle->keySlotNumber );
           #endif
            if( rc )
            {
                BDBG_ERR(("Error (%d) freeing CA keyslot", rc));
            }
            break;
        case NEXUS_SecurityEngine_eGeneric:
        default:
            break;
    }

#if NEXUS_HAS_XPT_DMA
    if( keyHandle->settings.keySlotEngine==NEXUS_SecurityEngine_eM2m && pKeySlotData->dmaPidChannelHandle != NULL )
    {
        NEXUS_Security_RemovePidChannelFromKeySlot( keyHandle, keyHandle->dma.pidChannelIndex );
        NEXUS_Module_Lock(g_security.moduleSettings.transport);
        NEXUS_PidChannel_CloseDma_Priv( pKeySlotData->dmaPidChannelHandle );
        NEXUS_Module_Unlock(g_security.moduleSettings.transport);
        pKeySlotData->dmaPidChannelHandle = NULL;
    }
#endif
    if (!rc && (keyHandle->settings.keySlotEngine != NEXUS_SecurityEngine_eGeneric)) {
        NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, false);
    }

    BKNI_Free(pKeySlotData);
    BLST_S_REMOVE(&g_security.keyslotList, keyHandle, NEXUS_KeySlot, next);

    BDBG_LEAVE(NEXUS_Security_P_FreeKeySlot);

    return;
}

void NEXUS_Security_FreeKeySlot(NEXUS_KeySlotHandle keyslot)
{

    BDBG_ENTER(NEXUS_Security_FreeKeySlot);

    NEXUS_OBJECT_ASSERT(NEXUS_KeySlot, keyslot);

    if (keyslot->security.data)
    {
        keyslot->deferDestroy = false;
        NEXUS_Security_P_FreeKeySlot(keyslot);
    }

    NEXUS_KeySlot_Destroy(keyslot);
    NEXUS_SECURITY_DUMP_KEYSLOTS;

    BDBG_LEAVE(NEXUS_Security_FreeKeySlot);
    return;
}

static NEXUS_Error NEXUS_SetPidChannelBypassKeyslot_priv( NEXUS_PidChannelHandle pidChannel, NEXUS_BypassKeySlot bypassKeySlot );

void NEXUS_SecurityModule_Sweep_priv(void)
{
    NEXUS_KeySlotHandle keyslot;
    while ((keyslot = NEXUS_KeySlot_P_GetDeferredDestroy()))
    {
        BDBG_ASSERT(keyslot->security.data); /* if anyone else defers keyslot destroy, we need to enhance api */
        NEXUS_Security_P_FreeKeySlot(keyslot);
        NEXUS_KeySlot_P_DeferredDestroy(keyslot);
    }
    while (1) {
        NEXUS_PidChannelHandle pidChannel;
        NEXUS_Module_Lock(g_security.moduleSettings.transport);
        pidChannel = NEXUS_PidChannel_GetBypassKeyslotCleanup_priv();
        NEXUS_Module_Unlock(g_security.moduleSettings.transport);
        if (!pidChannel) break;
        NEXUS_SetPidChannelBypassKeyslot_priv(pidChannel, NEXUS_BypassKeySlot_eG2GR);
    }
}

void NEXUS_Security_GetDefaultMulti2Settings(NEXUS_SecurityMulti2Settings *pSettings /* [out] */ )
{
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error NEXUS_Security_ConfigMulti2(NEXUS_KeySlotHandle keyHandle, const NEXUS_SecurityMulti2Settings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHSM_ConfigMulti2IO_t config_multi2;
    NEXUS_Security_P_KeySlotData *pKeySlotData;

    BDBG_ENTER(NEXUS_Security_ConfigMulti2);

    BDBG_OBJECT_ASSERT(keyHandle, NEXUS_KeySlot);
    pKeySlotData = keyHandle->security.data;
    if (!pKeySlotData) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    BKNI_Memset(&config_multi2, 0, sizeof(config_multi2));
    config_multi2.ucMulti2RndCnt = pSettings->multi2Rounds;
    BKNI_Memcpy(config_multi2.aucMulti2SysKey, pSettings->multi2SysKey, 32);
    config_multi2.ucSysKeyDest = pSettings->multi2KeySelect;
    BHSM_ConfigMulti2(g_security.hsm, &config_multi2);
    if (config_multi2.unStatus != 0) {
        BDBG_ERR(("NEXUS_Security_ConfigMulti2: Error configuring Multi2 (0x%02x)", config_multi2.unStatus));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BDBG_LEAVE(NEXUS_Security_ConfigMulti2);
    return rc;
}

void NEXUS_Security_GetDefaultInvalidateKeySettings(NEXUS_SecurityInvalidateKeySettings *pSettings /* [out] */ )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#if HSM_IS_ASKM
    pSettings->keySrc = NEXUS_SecurityKeySource_eFirstRamAskm;
#else
    pSettings->keySrc = NEXUS_SecurityKeySource_eFirstRam; /* this is NEXUS_SecurityKeySource_eReserved0 for ASKM */
#endif
}

static NEXUS_Error NEXUS_Security_P_InvalidateKey(NEXUS_KeySlotHandle keyHandle, const NEXUS_SecurityInvalidateKeySettings *pSettings)
{
    BHSM_InvalidateKeyIO_t invalidateKeyIO;
    BERR_Code rc;
    NEXUS_Error nrc;
    NEXUS_Security_P_KeySlotData *pKeySlotData;

    BDBG_ENTER(NEXUS_Security_P_InvalidateKey);

    BDBG_OBJECT_ASSERT(keyHandle, NEXUS_KeySlot);
    pKeySlotData = keyHandle->security.data;
    if (!pKeySlotData) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    BDBG_MSG(("NEXUS_Security_P_InvalidateKey: %p(%d,%d)", (void *)keyHandle,keyHandle->keySlotNumber,keyHandle->keyslotType));

    BKNI_Memset(&invalidateKeyIO, 0, sizeof(invalidateKeyIO)); /* Coverity defect ID 35508; SW7435-1079 */

    nrc = NEXUS_Security_P_GetInvalidateKeyFlag(pSettings->invalidateKeyType, &invalidateKeyIO.invalidKeyType);
    if( nrc )
    {
        return BERR_TRACE(nrc);
    }

    nrc = NEXUS_Security_GetHsmDestBlkType(keyHandle, pSettings->keyDestBlckType, &invalidateKeyIO.keyDestBlckType);
    if( nrc )
    {
        return BERR_TRACE(nrc);
    }

    invalidateKeyIO.keyDestEntryType = NEXUS_Security_MapNexusKeyDestToHsm(keyHandle, pSettings->keyDestEntryType);
    invalidateKeyIO.caKeySlotType = (BCMD_XptSecKeySlot_e)keyHandle->keyslotType;
    invalidateKeyIO.unKeySlotNum = keyHandle->keySlotNumber;
   #if HSM_IS_ASKM
    invalidateKeyIO.virtualKeyLadderID = (BCMD_VKLID_e)pSettings->virtualKeyLadderID;
    invalidateKeyIO.keyLayer = (BCMD_KeyRamBuf_e)pSettings->keySrc;
   #else
    invalidateKeyIO.keySrc = (BCMD_KeyRamBuf_e)pSettings->keySrc;
   #endif

   #if HSM_IS_ASKM_28NM_ZEUS_4_0
    invalidateKeyIO.bInvalidateAllEntries = pSettings->invalidateAllEntries;
   #endif


    rc = BHSM_InvalidateKey(g_security.hsm, &invalidateKeyIO);
    if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

    BDBG_LEAVE(NEXUS_Security_P_InvalidateKey);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Security_InvalidateKey(NEXUS_KeySlotHandle keyHandle, const NEXUS_SecurityInvalidateKeySettings *pSettings)
{
    BERR_Code rc;
    NEXUS_SecurityKeyType keyEntryType;
    NEXUS_SecurityInvalidateKeySettings keySettings;

    BDBG_ENTER(NEXUS_Security_InvalidateKey);

    BDBG_OBJECT_ASSERT(keyHandle, NEXUS_KeySlot);

    BDBG_MSG(("NEXUS_Security_InvalidateKey: %p(%d,%d)", (void *)keyHandle,keyHandle->keySlotNumber,keyHandle->keyslotType));

    keyEntryType = pSettings->keyDestEntryType;


#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    if( keyHandle->keyslotType == BCMD_XptSecKeySlot_eType3 )
    {
        /* Clear key entry must be used for Type 3, m2m  */
        keyEntryType = NEXUS_SecurityKeyType_eClear;
    }
#endif

    BKNI_Memcpy(&keySettings, pSettings, sizeof(keySettings));

    if( keyEntryType == NEXUS_SecurityKeyType_eOddAndEven )
    {
        /* Special case: NEXUS_SecurityKeyType_eOddAndEven means all CA keys
         * Since pSettings is const, we make a local copy and iterate through the possible destinations. */

        keySettings.keyDestEntryType = NEXUS_SecurityKeyType_eOdd;

        rc = NEXUS_Security_P_InvalidateKey(keyHandle, &keySettings);
        if (rc)
        {
            return BERR_TRACE(MAKE_HSM_ERR(rc));
        }

        keySettings.keyDestEntryType = NEXUS_SecurityKeyType_eEven;

        rc = NEXUS_Security_P_InvalidateKey(keyHandle, &keySettings);
        if (rc)
        {
            return BERR_TRACE(MAKE_HSM_ERR(rc));
        }

        if( (keyHandle->settings.keySlotEngine == NEXUS_SecurityEngine_eRmx) ||
            (keyHandle->settings.keySlotEngine == NEXUS_SecurityEngine_eCaCp) )
        {
            keySettings.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
            rc = NEXUS_Security_P_InvalidateKey(keyHandle, &keySettings);
            if (rc)
            {
                return BERR_TRACE(MAKE_HSM_ERR(rc));
            }
        }
    }
    else
    {
        keySettings.keyDestEntryType = keyEntryType;

        rc = NEXUS_Security_P_InvalidateKey( keyHandle, &keySettings );
        if (rc)
        {
            return BERR_TRACE( MAKE_HSM_ERR(rc) );
        }
    }

    BDBG_LEAVE(NEXUS_Security_InvalidateKey);
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SecurityModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    BERR_Code rc;

    BDBG_ENTER(NEXUS_SecurityModule_Standby_priv);

    NEXUS_SecurityModule_Sweep_priv();

    if( enabled ) /* if *enter* standby */
    {
        if( pSettings->mode != NEXUS_StandbyMode_eDeepSleep )
        {
            /* NEXUS_PowerManagement_SetCoreState is called when keyslots are allocated and free'd.
               in non-S3, this does not have to occur, so we power down as many times as the number of keyslots opened */
            if (BLST_S_FIRST(&g_security.keyslotList))
            {
                   NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, false);
            }
        }
        else
        {
            /* enforce that apps free keyslots before standby and re-configure them after resume.
               this also takes care of NEXUS_PowerManagement_SetCoreState */
            if (BLST_S_FIRST(&g_security.keyslotList))
            {
                BDBG_ERR(("Keyslots must be freed before entering S3 standby"));
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
            NEXUS_Security_P_UninitHsm();
        }
    }
    else
    {
        if (g_security.hsm)
        { /* not S3 */
            if (BLST_S_FIRST(&g_security.keyslotList))
            {
                    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, true);
            }
        }
        else
        {
            rc = NEXUS_Security_P_InitHsm( &g_security.settings );
            if (rc) { return BERR_TRACE(NEXUS_NOT_SUPPORTED); }

            #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0)
            /* Reinitialise Region Verification module. */
            rc = NEXUS_Security_RegionVerification_Init_priv( );
            if (rc) { return BERR_TRACE(rc); }
            #endif
        }

        if( g_security.moduleSettings.callTransportPostInit )
        {
            NEXUS_Module_Lock(g_security.moduleSettings.transport);
            rc = NEXUS_TransportModule_PostResume_priv();
            NEXUS_Module_Unlock(g_security.moduleSettings.transport);
            if (rc) return BERR_TRACE(rc);
        }
    }

#else
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(enabled);
#endif

    BDBG_LEAVE(NEXUS_SecurityModule_Standby_priv);
    return NEXUS_SUCCESS;
}


void NEXUS_KeySlot_GetDefaultGlobalControlWordSettings(
    NEXUS_KeySlotGlobalControlWordSettings  *pSettings
    )
{
    BDBG_ENTER(NEXUS_KeySlot_GetDefaultGlobalControlWordSettings);

    BKNI_Memset(pSettings, 0, sizeof(NEXUS_KeySlotGlobalControlWordSettings));
    /* Set everything to global region */
    pSettings->inputRegion       = NEXUS_SECURITY_G_REGION;
    pSettings->gPipeOutput       = NEXUS_SECURITY_G_REGION;
    pSettings->rPipeOutput       = NEXUS_SECURITY_G_REGION;
    /* Packets are to not be encrypted before RAVE */
    pSettings->encryptBeforeRave = false;

    BDBG_LEAVE(NEXUS_KeySlot_GetDefaultGlobalControlWordSettings);
    return;
}




NEXUS_Error NEXUS_KeySlot_SetGlobalControlWordSettings(
    NEXUS_KeySlotHandle                            keyHandle,
    const NEXUS_KeySlotGlobalControlWordSettings  *pSettings
    )
{
    NEXUS_Error rc = NEXUS_NOT_SUPPORTED;
#if HSM_IS_ASKM_28NM_ZEUS_4_0
    NEXUS_Security_P_KeySlotData *pKeySlotData;
    BHSM_KeySlotGlobalCntrlWord_t    keySlotControlWord;

    BDBG_ENTER(NEXUS_KeySlot_SetGlobalControlWordSettings);

    BKNI_Memset(&keySlotControlWord, 0, sizeof(BHSM_KeySlotGlobalCntrlWord_t));
    BDBG_OBJECT_ASSERT(keyHandle, NEXUS_KeySlot);
    pKeySlotData = keyHandle->security.data;

    if (!pKeySlotData) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    /* set up info for BHSM call */
    keySlotControlWord.caKeySlotType     = keyHandle->keyslotType;      /* For M2M DMA key slot, this has to be set to the right type, starting with Zeus 4.0 */
    keySlotControlWord.unKeySlotNum      = keyHandle->keySlotNumber;
    keySlotControlWord.inputRegion       = pSettings->inputRegion;
    keySlotControlWord.RpipeOutput       = pSettings->rPipeOutput;
    keySlotControlWord.GpipeOutput       = pSettings->gPipeOutput;
    keySlotControlWord.encryptBeforeRAVE = pSettings->encryptBeforeRave;

    rc = BHSM_ConfigKeySlotGlobalCntrlWord(g_security.hsm, &keySlotControlWord);
    if (rc)
    {
        return BERR_TRACE(MAKE_HSM_ERR(rc));
    }

#else
    BSTD_UNUSED(keyHandle);
    BSTD_UNUSED(pSettings);
#endif

    BDBG_LEAVE(NEXUS_KeySlot_SetGlobalControlWordSettings);
    return (rc);
}

NEXUS_Error NEXUS_SetPidChannelBypassKeyslot( NEXUS_PidChannelHandle pidChannel, NEXUS_BypassKeySlot bypassKeySlot )
{
    NEXUS_SecurityModule_Sweep_priv();
    return NEXUS_SetPidChannelBypassKeyslot_priv(pidChannel, bypassKeySlot);
}

static NEXUS_Error NEXUS_SetPidChannelBypassKeyslot_priv( NEXUS_PidChannelHandle pidChannel, NEXUS_BypassKeySlot bypassKeySlot )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_CASSERT( (int)NEXUS_BypassKeySlot_eG2GR      == (int)BHSM_BypassKeySlot_eG2GR  );
    BDBG_CASSERT( (int)NEXUS_BypassKeySlot_eGR2R     == (int)BHSM_BypassKeySlot_eGR2R );
    BDBG_CASSERT( (int)NEXUS_BypassKeySlot_eMax      == (int)BHSM_BypassKeySlot_eInvalid  );

    if( ( rc = BHSM_SetPidChannelBypassKeyslot( g_security.hsm, NEXUS_PidChannel_GetIndex_isrsafe(pidChannel), bypassKeySlot ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( MAKE_HSM_ERR(rc) );
    }
    NEXUS_Module_Lock(g_security.moduleSettings.transport);
    NEXUS_PidChannel_SetBypassKeyslot_priv(pidChannel, bypassKeySlot != NEXUS_BypassKeySlot_eG2GR);
    NEXUS_Module_Unlock(g_security.moduleSettings.transport);
#else
    BSTD_UNUSED( pidChannel );
    BSTD_UNUSED( bypassKeySlot );
#endif

    return NEXUS_SUCCESS;
}

void NEXUS_GetPidChannelBypassKeyslot( NEXUS_PidChannelHandle pidChannel, NEXUS_BypassKeySlot *pBypassKeySlot )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)

    NEXUS_Error rc = NEXUS_SUCCESS;

    if( pBypassKeySlot == NULL ) return;

    *pBypassKeySlot = NEXUS_BypassKeySlot_eMax;

    if( ( rc = BHSM_GetPidChannelBypassKeyslot( g_security.hsm,
                                                NEXUS_PidChannel_GetIndex_isrsafe(pidChannel),
                                                (BHSM_BypassKeySlot_e*)pBypassKeySlot ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        return;
    }
#else
    BSTD_UNUSED( pidChannel );
    BSTD_UNUSED( pBypassKeySlot );
#endif
    return;
}

/* This function is dangerous and not meant for general use. It is needed for
 test code that needs the HSM handle. It must be extern'd. */
void b_get_hsm(BHSM_Handle *hsm)
{
    *hsm = g_security.hsm;
}
void b_get_reg(BREG_Handle *reg)
{
    *reg = g_pCoreHandles->reg;
}
void b_get_int(BINT_Handle *bint)
{
    *bint = g_pCoreHandles->bint;
}
void b_get_chp(BCHP_Handle *chp)
{
    *chp = g_pCoreHandles->chp;
}
