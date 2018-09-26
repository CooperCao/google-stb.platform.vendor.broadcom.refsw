/******************************************************************************
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
#include "blst_slist.h"
#include "bhsm_exception_status.h"

#include "nexus_base.h"
#include "nexus_pid_channel.h"
#include "priv/nexus_pid_channel_priv.h"
#include "priv/nexus_transport_priv.h"
#include "bhsm_otp_id.h"

#if NEXUS_SECURITY_IPLICENSING
 #include "bhsm_ip_licensing.h"
 #include "bhsm_otpmsp.h"
#endif
#include "priv/nexus_security_standby_priv.h"


BDBG_MODULE(nexus_security);


#define MIN(a,b) ((a)<(b)?(a):(b))


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
            BDBG_MSG(("%s: Keyslot [%p]",BSTD_FUNCTION,keyslot)); \
            for (pPidChannelNode=BLST_S_FIRST(&pKeySlotData->pidChannelList);pPidChannelNode!=NULL;pPidChannelNode=BLST_S_NEXT(pPidChannelNode,next)) { \
                BDBG_MSG(("%s:   pidchannel [%d]",BSTD_FUNCTION,keyslot,pPidChannelNode->pidChannelIndex)); \
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
    BLST_S_HEAD(NEXUS_Security_P_KeySlotList_t, NEXUS_KeySlot) keyslotList; /* cannot contain any "generic" keyslots */
} g_security;

static BCMD_XptM2MSecCryptoAlg_e mapNexus2Hsm_Algorithm( NEXUS_SecurityAlgorithm algorithm );
static BCMD_KeyDestEntryType_e   mapNexus2Hsm_keyEntryType( NEXUS_SecurityKeyType keytype );
static BCMD_KeyDestIVType_e      mapNexus2Hsm_ivType( NEXUS_SecurityKeyIVType keyIVtype );
static NEXUS_SecurityKeySlotType mapHsm2Nexus_keySlotType( BCMD_XptSecKeySlot_e hsmKeyslotType );
static BCMD_XptKeyTableCustomerMode_e mapNexus2Hsm_CustomerMode( NEXUS_Security_CustomerMode customerMode );
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
    BERR_Code rc = BERR_SUCCESS;
    BHSM_Capabilities_t hsmCaps;
    unsigned x;
    unsigned numTypes;

    BDBG_ENTER(NEXUS_GetSecurityCapabilities);

    if( pCaps == NULL ) { (void)BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    BKNI_Memset( pCaps, 0, sizeof(*pCaps) );
    BKNI_Memset( &hsmCaps, 0, sizeof(hsmCaps) );

    rc = BHSM_GetCapabilities( g_security.hsm, &hsmCaps );
    if( rc != BERR_SUCCESS ) { (void)BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    pCaps->version.zeus = NEXUS_ZEUS_VERSION_CALC_3 ( NEXUS_SECURITY_ZEUS_VERSION_MAJOR,
                                                      NEXUS_SECURITY_ZEUS_VERSION_MINOR,
                                                      NEXUS_SECURITY_ZEUS_VERSION_SUBMINOR );
    pCaps->version.firmware = NEXUS_BFW_VERSION_CALC( hsmCaps.version.firmware.bseck.major,
                                                      hsmCaps.version.firmware.bseck.minor,
                                                      hsmCaps.version.firmware.bseck.subMinor );

    if( hsmCaps.version.firmware.bfwEpoch.valid )
    {
        pCaps->firmwareEpoch.valid = true;
        pCaps->firmwareEpoch.value = hsmCaps.version.firmware.bfwEpoch.value;
    }

    numTypes = hsmCaps.keyslotTypes.numKeySlotTypes;
    numTypes = MIN( numTypes , sizeof( pCaps->keySlotTableSettings.numKeySlotsForType ) );
    numTypes = MIN( numTypes , sizeof( hsmCaps.keyslotTypes.numKeySlot ) );

    for( x = 0; x < numTypes; x++ )
    {
        pCaps->keySlotTableSettings.numKeySlotsForType[x] = hsmCaps.keyslotTypes.numKeySlot[x];
    }
    pCaps->keySlotTableSettings.numMulti2KeySlots = hsmCaps.keyslotTypes.numMulti2KeySlots;

    BDBG_LEAVE(NEXUS_GetSecurityCapabilities);
    return;
}


/*
Description:
    Function will itterate over all ARCHes of each available MEM Controller and print any detected violations.
*/
void NEXUS_Security_PrintArchViolation_priv(void)
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
    if( !hHsm ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    BKNI_Memset( &memInfo, 0, sizeof( memInfo ) );
    magnumRc = BCHP_GetMemoryInfo( g_pCoreHandles->chp, &memInfo );
    if( magnumRc != BERR_SUCCESS ) { BERR_TRACE( magnumRc ); return; }

    maxMemc = sizeof(memInfo.memc)/sizeof(memInfo.memc[0]);

    BKNI_Memset( &request, 0, sizeof(request) );

    request.deviceType = BHSM_ExceptionStatusDevice_eMemcArch;
    request.keepStatus = false;

    for( memcIndex = 0; memcIndex < maxMemc; memcIndex++ )  /* iterate over mem controllers. */
    {
        if( memInfo.memc[memcIndex].valid)              /* if the MEMC is in use. */
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
                    BDBG_ERR(("MEMC ARCH Violation. MEMC[%u]ARCH[%u] Addr start [" BDBG_UINT64_FMT  \
                              "] end[" BDBG_UINT64_FMT "] numBlocks[%u] scbClientId[%u:%s] requestType[%#x:%s]",
                              memcIndex,
                              archIndex,
                              BDBG_UINT64_ARG(status.u.memArch.startAddress),
                              BDBG_UINT64_ARG(status.u.memArch.endAddress),
                              status.u.memArch.numBlocks,
                              status.u.memArch.scbClientId,
                              BMRC_Checker_GetClientName(memcIndex, status.u.memArch.scbClientId),
                              status.u.memArch.requestType,
                              BMRC_Monitor_GetRequestTypeName_isrsafe(status.u.memArch.requestType) ));
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
    pSettings->common.standbyLevel = NEXUS_ModuleStandbyLevel_eActive;

    /* defaults number of keyslots per type. */
    pSettings->numKeySlotsForType[0] = 20;
    pSettings->numKeySlotsForType[1] = 10;
    pSettings->numKeySlotsForType[2] = 15;
    pSettings->numKeySlotsForType[3] = 25;
    pSettings->numKeySlotsForType[4] = 7;
    pSettings->numKeySlotsForType[5] = 2;
    pSettings->numKeySlotsForType[6] = 8;

    /*exceptions*/
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
    pSettings->numKeySlotsForType[3]= 50;
    pSettings->numKeySlotsForType[4]= 1;
    pSettings->numKeySlotsForType[5]= 50;
    #endif
  #endif /* NEXUS_HAS_NSK2HDI */


    #if (BHSM_ZEUS_FULL_VERSION == BHSM_ZEUS_VERSION_CALC_3(4,2,2))
    pSettings->numKeySlotsForType[0] = 10;
    pSettings->numKeySlotsForType[1] = 4;
    pSettings->numKeySlotsForType[2] = 6;
    pSettings->numKeySlotsForType[3] = 10;
    pSettings->numKeySlotsForType[4] = 2;
    pSettings->numKeySlotsForType[6] = 2;
    #endif

    #if NEXUS_ENFORCE_REGION_VERIFICATION
    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eTransport]      = false;
    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eVideoDecoder]   = true;
    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_eAudioDecoder]   = true;
    pSettings->enforceAuthentication[NEXUS_SecurityFirmwareType_ePictureDecoder] = true;
    #endif
    return;
}

/* Function to authenticate/verify rave firmware. */
static NEXUS_Error secureFirmwareRave( void )
{
    NEXUS_Error rc;
    NEXUS_SecurityRegionConfiguration regionConfig;
    void *pRaveFirmware = NULL;
    unsigned raveFirmwareSize = 0;
    NEXUS_Addr regionAddress = 0;
   #if NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE || NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW /*dump binaries */
    void *pRaveFirmwareDeviceMem = NULL;
    NEXUS_MemoryAllocationSettings memSettings;
   #endif

    BDBG_ENTER(secureFirmwareRave);

    NEXUS_Security_RegionGetDefaultConfig_priv( NEXUS_SecurityRegverRegionID_eRave, &regionConfig );
    /* use defaults. */
    rc = NEXUS_Security_RegionConfig_priv( NEXUS_SecurityRegverRegionID_eRave, &regionConfig );
    if( rc ) { return BERR_TRACE(rc); }

    /* locate Rave Firmware. */
    NEXUS_TransportModule_GetRaveFirmware_isrsafe( &pRaveFirmware, &raveFirmwareSize );

   #if NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE || NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW /*dump binaries */
    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    rc = NEXUS_Memory_Allocate( raveFirmwareSize, &memSettings, &pRaveFirmwareDeviceMem );
    if( rc ) { return BERR_TRACE(rc); }

    BKNI_Memcpy( pRaveFirmwareDeviceMem, pRaveFirmware, raveFirmwareSize );
    regionAddress =  NEXUS_AddrToOffset( pRaveFirmwareDeviceMem );
   #endif

    rc = NEXUS_Security_RegionVerifyEnable_priv( NEXUS_SecurityRegverRegionID_eRave,
                                                 regionAddress,
                                                 raveFirmwareSize );
   #if NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE || NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW
    NEXUS_Memory_Free( pRaveFirmwareDeviceMem );
   #endif
    if (rc) { return BERR_TRACE(rc); }

    BDBG_LEAVE(secureFirmwareRave);
    return BERR_SUCCESS;
}

static void _print_otpid_a(void)
{
    NEXUS_Error rc = BERR_SUCCESS;
    BHSM_ReadOTPIdIO_t readOTPIdIO;
    readOTPIdIO.OtpId = BHSM_OtpIdRead_eOTPIdA;
    rc = BHSM_ReadOTPId(g_security.hsm, &readOTPIdIO);
    if( rc != BERR_SUCCESS || readOTPIdIO.unStatus)
    {
        BDBG_ERR(("BHSM_ReadOTPId failed, rc=%d, unStatus=%d", rc, readOTPIdIO.unStatus));
    } else {
        if (readOTPIdIO.unIdSize != 8) {
            BDBG_ERR(("%s: unknown OTP ID length %d", BSTD_FUNCTION, readOTPIdIO.unIdSize));
        } else {
            BDBG_LOG(("CHIPID: %02X%02X%02X%02X%02X%02X%02X%02X",
                      (unsigned int)readOTPIdIO.aucOTPId[0], (unsigned int)readOTPIdIO.aucOTPId[1],
                      (unsigned int)readOTPIdIO.aucOTPId[2], (unsigned int)readOTPIdIO.aucOTPId[3],
                      (unsigned int)readOTPIdIO.aucOTPId[4], (unsigned int)readOTPIdIO.aucOTPId[5],
                      (unsigned int)readOTPIdIO.aucOTPId[6], (unsigned int)readOTPIdIO.aucOTPId[7]));
        }
    }
}

NEXUS_ModuleHandle NEXUS_SecurityModule_Init(const NEXUS_SecurityModuleInternalSettings *pModuleSettings, const NEXUS_SecurityModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_SecurityRegionModuleSettings regVerSettings;
    int i;
    BERR_Code rc;

    BDBG_ENTER(NEXUS_SecurityModule_Init);

    BDBG_ASSERT(!NEXUS_P_SecurityModule);

    BDBG_ASSERT(pSettings);

#if NEXUS_HAS_XPT_DMA
    /* security module requires transport module */
    if( pModuleSettings->transport == NULL ) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return NULL; }
#endif

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eLow, &pSettings->common);
    NEXUS_P_SecurityModule = NEXUS_Module_Create("security", &moduleSettings);
    if( NEXUS_P_SecurityModule == NULL ) { BERR_TRACE(NEXUS_UNKNOWN);  return NULL; }

    g_security.settings = *pSettings;
    g_security.moduleSettings = *pModuleSettings;

    NEXUS_LockModule();
    rc = NEXUS_Security_P_InitHsm(pSettings);
    if (rc) {rc = BERR_TRACE(rc); goto err_init;}

    BLST_S_INIT( &g_security.keyslotList );

    NEXUS_Security_GetDefaultRegionVerificationModuleSettings( &regVerSettings );

    for( i = 0; i < NEXUS_SecurityFirmwareType_eMax; i++ ) {
        regVerSettings.enforceAuthentication[i] = pSettings->enforceAuthentication[i];
    }

    rc = NEXUS_Security_RegionVerification_Init_priv( &regVerSettings ); /* region verification private interface. */
    if (rc) { rc = BERR_TRACE(rc); goto err_init; }

    rc = NEXUS_RegionVerify_Init( );                     /* region verification public interface. */
    if (rc) { rc = BERR_TRACE(rc); goto err_init; }

    /* IP licensing is supported for Zeus1.0 and newer. */
    if( pSettings->ipLicense.valid )
    {
      #if NEXUS_SECURITY_IPLICENSING
        BHSM_Handle     hHsm;
        BHSM_ipLicensingOp_t ipLicensingOp;
        BHSM_ReadMspIO_t     msp;

        NEXUS_Security_GetHsm_priv ( &hHsm );
        if( !hHsm ) { BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_init; }

        BKNI_Memset( &msp, 0, sizeof(msp) );
        msp.readMspEnum = 70;
        rc = BHSM_ReadMSP( hHsm, &msp );
        if (rc) { BERR_TRACE(rc); goto err_init; }

        /* If enabled by the OTP msp bit */
        if(  (msp.aucLockMspData[3] & 0x01) && ( msp.aucMspData[3] & 0x01 ) )
        {

            BKNI_Memset(&ipLicensingOp, 0, sizeof(ipLicensingOp));

            ipLicensingOp.dataSize = NEXUS_SECURITY_IP_LICENCE_SIZE;

            BKNI_Memcpy(ipLicensingOp.inputBuf, pSettings->ipLicense.data, NEXUS_SECURITY_IP_LICENCE_SIZE);

            if (BHSM_SetIpLicense(hHsm, &ipLicensingOp) != BERR_SUCCESS)
            {
                BERR_TRACE(NEXUS_NOT_SUPPORTED);
                goto err_init;
            }
        }
      #else
       BDBG_WRN(("IP Licensing not enabled in build."));
      #endif
    }

    NEXUS_UnlockModule();

    _print_otpid_a();

    return NEXUS_P_SecurityModule;

err_init:
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(NEXUS_P_SecurityModule);
    NEXUS_P_SecurityModule = NULL;

    BDBG_LEAVE(NEXUS_SecurityModule_Init);
    return NULL;
}

NEXUS_Error NEXUS_SecurityModule_InitTransport_priv(void)
{
    NEXUS_Error rc;
    NEXUS_Module_Lock( g_security.moduleSettings.transport );
    rc = NEXUS_TransportModule_PostInit_priv( secureFirmwareRave );
    NEXUS_Module_Unlock( g_security.moduleSettings.transport );
    if (rc) return BERR_TRACE(rc);
    return NEXUS_SUCCESS;
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

    NEXUS_RegionVerify_Uninit( );

    NEXUS_Security_RegionVerification_UnInit_priv( );

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

    BDBG_ENTER(NEXUS_Security_P_InitHsm);
    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, true);

    BHSM_GetDefaultSettings(&hsmSettings, g_pCoreHandles->chp);
    hsmSettings.hHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mem;

    #if NEXUS_HAS_SAGE
    hsmSettings.sageEnabled = true;
    #endif

    rc = BHSM_Open(&g_security.hsm, g_pCoreHandles->reg, g_pCoreHandles->chp, g_pCoreHandles->bint, &hsmSettings);
    if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

    BKNI_Memset(&keyslot_io, 0, sizeof(keyslot_io));

    keyslot_io.unKeySlotType0Num = pSettings->numKeySlotsForType[0];
    keyslot_io.unKeySlotType1Num = pSettings->numKeySlotsForType[1];
    keyslot_io.unKeySlotType2Num = pSettings->numKeySlotsForType[2];
    keyslot_io.unKeySlotType3Num = pSettings->numKeySlotsForType[3];
    keyslot_io.unKeySlotType4Num = pSettings->numKeySlotsForType[4];
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    keyslot_io.unKeySlotType5Num = pSettings->numKeySlotsForType[5];
    #endif

    keyslot_io.bMulti2SysKey     = pSettings->enableMulti2Key;
    keyslot_io.numMulti2KeySlots = pSettings->numMulti2KeySlots;

    /* For API backward compatibility */
    if( keyslot_io.bMulti2SysKey && (keyslot_io.numMulti2KeySlots == 0) )
    {
        keyslot_io.numMulti2KeySlots = BCMD_MULTI2_MAXSYSKEY;
    }

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

#if NEXUS_HAS_XPT_DMA
    rc = BHSM_SetPidChannelBypassKeyslot(g_security.hsm, NEXUS_PidChannel_GetBypassKeySlotIndex_isrsafe(NEXUS_BypassKeySlot_eG2GR), NEXUS_BypassKeySlot_eG2GR);
    if (rc) {BERR_TRACE(rc);} /* keep going */
    rc = BHSM_SetPidChannelBypassKeyslot(g_security.hsm, NEXUS_PidChannel_GetBypassKeySlotIndex_isrsafe(NEXUS_BypassKeySlot_eGR2R), NEXUS_BypassKeySlot_eGR2R);
    if (rc) {BERR_TRACE(rc);} /* keep going */
    rc = BHSM_SetPidChannelBypassKeyslot(g_security.hsm, NEXUS_PidChannel_GetBypassKeySlotIndex_isrsafe(NEXUS_BypassKeySlot_eGT2T), NEXUS_BypassKeySlot_eGT2T);
    if (rc) {BERR_TRACE(rc);} /* keep going */
#endif

    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHsm, false);

    BDBG_LEAVE(NEXUS_Security_P_InitHsm);
    return rc;
}

void NEXUS_Security_P_UninitHsm()
{
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
    pSettings->keySlotSource = NEXUS_SecurityKeySource_eFirstRamAskm;
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

#define NEXUS_SECURITY_CACP_INVALID_PIDCHANNEL 0xFFFFFFFF

static NEXUS_Error NEXUS_Security_AllocateKeySlotForType(  NEXUS_KeySlotHandle *pKeyHandle
                                                           , const NEXUS_SecurityKeySlotSettings *pSettings
                                                           , BCMD_XptSecKeySlot_e type
                                                        ) /*  */
{
    BERR_Code rc;
    NEXUS_KeySlotHandle pHandle;
    BHSM_KeySlotAllocate_t  keySlotConf;

    BDBG_ENTER(NEXUS_Security_AllocateKeySlotForType);

    NEXUS_SecurityModule_Sweep_priv();

    pHandle = NEXUS_KeySlot_Create();
    if ( !pHandle) { return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); }

    pHandle->deferDestroy = true;

    BKNI_Memset(&keySlotConf, 0, sizeof(keySlotConf));

    keySlotConf.client = pSettings->client;
    keySlotConf.keySlotType = type;

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    rc = BHSM_AllocateCAKeySlot( g_security.hsm, &keySlotConf );
    #else
    rc = BHSM_AllocateCAKeySlot_v2( g_security.hsm, &keySlotConf );
    #endif
    if( rc ) { BERR_TRACE( rc ); goto error; }

    pHandle->keyslotType = type;
    pHandle->settings = *pSettings;

    pHandle->keySlotNumber = keySlotConf.keySlotNum;

    *pKeyHandle = pHandle;

    BDBG_MSG(("%s: Allocated keyslot %p",BSTD_FUNCTION, (void *)pHandle ));

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
            if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); continue; }
            /* update imput parameter. */
            pidChannelIndex = status.pidChannelIndex;
        }

        if( pPidChannelNode->pidChannel )
        {
            /* retrieve actual HW pid Channel */
            rc = NEXUS_PidChannel_GetStatus( pPidChannelNode->pidChannel, &status );
            if( rc != NEXUS_SUCCESS ) { BERR_TRACE( rc ); continue; }

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
    if (rc) { rc = BERR_TRACE(MAKE_HSM_ERR(rc)); goto error; }

    for (keyslot=BLST_S_FIRST(&g_security.keyslotList);keyslot;keyslot=BLST_S_NEXT(keyslot,next)) {
        NEXUS_Security_P_PidChannelListNode *pPidChannelNode = find_pid(keyslot, NULL, pidchannel);
        if (pPidChannelNode) {
            if (keyslot->keySlotNumber != keyslotNumber || keyslot->keyslotType != keyslotType) {
                BDBG_WRN(("%s: number/type difference, keyslot tracking may be out of sync with HSM",BSTD_FUNCTION));
            }
            return keyslot;
        }
    }
error:
    BDBG_WRN(("%s: PID Channel %lu is not yet associated with any key slot",BSTD_FUNCTION,pidchannel));

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
    pidChannelConf.spidProgType = BHSM_SPIDProg_ePIDPointerA;
    pidChannelConf.bResetPIDToDefault = false;
    pidChannelConf.unKeySlotBType = 0;
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
    pidChannelConf.spidProgType = BHSM_SPIDProg_ePIDPointerA;
    pidChannelConf.bResetPIDToDefault = false;
    pidChannelConf.unKeySlotBType = 0;
    pidChannelConf.unKeySlotNumberB = 0;
    pidChannelConf.unKeyPointerSel = 0;

    rc = BHSM_ConfigPidChannelToDefaultKeySlot( g_security.hsm, &pidChannelConf );
    if( rc ) { rc = BERR_TRACE(rc); } /* Internal error. Continue for best effort */

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

static NEXUS_SecurityKeySlotType mapHsm2Nexus_keySlotType( BCMD_XptSecKeySlot_e hsmKeyslotType )
{
    /* It is a direct map */
    switch( hsmKeyslotType )
    {
        case BCMD_XptSecKeySlot_eType0: return NEXUS_SecurityKeySlotType_eType0;
        case BCMD_XptSecKeySlot_eType1: return NEXUS_SecurityKeySlotType_eType1;
        case BCMD_XptSecKeySlot_eType2: return NEXUS_SecurityKeySlotType_eType2;
        case BCMD_XptSecKeySlot_eType3: return NEXUS_SecurityKeySlotType_eType3;
        case BCMD_XptSecKeySlot_eType4: return NEXUS_SecurityKeySlotType_eType4;
       #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
        case BCMD_XptSecKeySlot_eType5: return NEXUS_SecurityKeySlotType_eType5;
       #endif
        default: BERR_TRACE(NEXUS_INVALID_PARAMETER); break;
    }

    return NEXUS_SecurityKeySlotType_eType0;
}

BCMD_KeyRamBuf_e NEXUS_Security_P_mapNexus2Hsm_KeyLayer( NEXUS_SecurityKeyLayer keyLayer )
{

    switch( keyLayer )
    {
        case NEXUS_SecurityKeyLayer_eKey3: { return BCMD_KeyRamBuf_eKey3; }
        case NEXUS_SecurityKeyLayer_eKey4: { return BCMD_KeyRamBuf_eKey4; }
        case NEXUS_SecurityKeyLayer_eKey5: { return BCMD_KeyRamBuf_eKey5; }
        case NEXUS_SecurityKeyLayer_eKey6: { return BCMD_KeyRamBuf_eKey6; }
        case NEXUS_SecurityKeyLayer_eKey7: { return BCMD_KeyRamBuf_eKey7; }
        default: { BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
    }

    return BCMD_KeyRamBuf_eMax;
}

BCMD_KeyRamBuf_e NEXUS_Security_P_mapNexus2Hsm_KeySource( NEXUS_SecurityKeySource keySource )
{
    BCMD_KeyRamBuf_e hsmKeySource;

    if( keySource > NEXUS_SecurityKeySource_eMax ) {
        BERR_TRACE( NEXUS_INVALID_PARAMETER );
        /* continue */
    }

    /* check equivalance of most used sources.  */
    BDBG_CASSERT( (int)NEXUS_SecurityKeySource_eKey3 == (int)BCMD_KeyRamBuf_eKey3 );
    BDBG_CASSERT( (int)NEXUS_SecurityKeySource_eKey4 == (int)BCMD_KeyRamBuf_eKey4 );
    BDBG_CASSERT( (int)NEXUS_SecurityKeySource_eKey5 == (int)BCMD_KeyRamBuf_eKey5 );
    BDBG_CASSERT( (int)NEXUS_SecurityKeySource_eKey6 == (int)BCMD_KeyRamBuf_eKey6 );
    BDBG_CASSERT( (int)NEXUS_SecurityKeySource_eKey7 == (int)BCMD_KeyRamBuf_eKey7 );

    /* pass the keysource though as it was input. */
    /* coverity[mixed_enums] */
    hsmKeySource = keySource;

    return hsmKeySource;
}

/* Map the NEXUS keyslot type to a HSM keyslot type. */
BCMD_XptSecKeySlot_e NEXUS_Security_P_mapNexus2Hsm_KeyslotType( NEXUS_SecurityKeySlotType nexusType, /* the Nexus keyslot type */
                                                      NEXUS_SecurityEngine engine )        /* the engine */
{
    switch( nexusType )
    {
        case NEXUS_SecurityKeySlotType_eAuto:
        {
            switch( engine )   /* Determine the keyslot type based on engine */
            {
                case NEXUS_SecurityEngine_eM2m:  return BCMD_XptSecKeySlot_eType3;
                case NEXUS_SecurityEngine_eCa:   return BCMD_XptSecKeySlot_eType0;
                case NEXUS_SecurityEngine_eCaCp: return BCMD_XptSecKeySlot_eType0;
                default: BERR_TRACE( NEXUS_INVALID_PARAMETER ); break;
            }
            break;
        }
        case NEXUS_SecurityKeySlotType_eType0: return BCMD_XptSecKeySlot_eType0;
        case NEXUS_SecurityKeySlotType_eType1: return BCMD_XptSecKeySlot_eType1;
        case NEXUS_SecurityKeySlotType_eType2: return BCMD_XptSecKeySlot_eType2;
        case NEXUS_SecurityKeySlotType_eType3: return BCMD_XptSecKeySlot_eType3;
        case NEXUS_SecurityKeySlotType_eType4: return BCMD_XptSecKeySlot_eType4;
       #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
        case NEXUS_SecurityKeySlotType_eType5: return BCMD_XptSecKeySlot_eType5;
       #endif
        default: BERR_TRACE( NEXUS_INVALID_PARAMETER ); break;
    }

    return BCMD_XptSecKeySlot_eType0;  /* default to Type0 */
}

BCMD_VKLID_e NEXUS_Security_P_mapNexus2Hsm_VklId( NEXUS_SecurityVirtualKeyladderID vklId )
{
    BCMD_VKLID_e hsmVklId = BCMD_VKL_KeyRam_eMax;
    bool masked = vklId & BHSM_VKL_ID_ALLOCATION_FLAG;

    /* remove the BHSM allocation flag. */
    vklId = vklId &(~BHSM_VKL_ID_ALLOCATION_FLAG);

    switch( vklId ) {
        case NEXUS_SecurityVirtualKeyladderID_eVKL0:    { hsmVklId = BCMD_VKL0; break; }
        case NEXUS_SecurityVirtualKeyladderID_eVKL1:    { hsmVklId = BCMD_VKL1; break; }
        case NEXUS_SecurityVirtualKeyladderID_eVKL2:    { hsmVklId = BCMD_VKL2; break; }
        case NEXUS_SecurityVirtualKeyladderID_eVKL3:    { hsmVklId = BCMD_VKL3; break; }
        case NEXUS_SecurityVirtualKeyladderID_eVKL4:    { hsmVklId = BCMD_VKL4; break; }
        case NEXUS_SecurityVirtualKeyladderID_eVKL5:    { hsmVklId = BCMD_VKL5; break; }
        case NEXUS_SecurityVirtualKeyladderID_eVKL6:    { hsmVklId = BCMD_VKL6; break; }
        case NEXUS_SecurityVirtualKeyladderID_eVKL7:    { hsmVklId = BCMD_VKL7; break; }
        case NEXUS_SecurityVirtualKeyladderID_eVKLDummy:
        case NEXUS_SecurityVirtualKeyladderID_eSWKey:   { hsmVklId = BCMD_VKL_KeyRam_eMax; break; }
        default: {
            BDBG_MSG(("VKL[0x%X] masked[%d]", vklId, masked ));
            break;
        }
    }

    /* replace the BHSM allocation flag */
    if( masked ) hsmVklId |= BHSM_VKL_ID_ALLOCATION_FLAG;

    return hsmVklId;
}

NEXUS_SecurityVirtualKeyladderID NEXUS_Security_P_mapHsm2Nexus_VklId( BCMD_VKLID_e vklId )
{
    NEXUS_SecurityVirtualKeyladderID nxVklId = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;
    bool masked = vklId & BHSM_VKL_ID_ALLOCATION_FLAG;

    /* remove the BHSM allocation flag. */
    vklId = vklId &(~BHSM_VKL_ID_ALLOCATION_FLAG);

    switch( vklId ) {
        case BCMD_VKL0:             { nxVklId = NEXUS_SecurityVirtualKeyladderID_eVKL0;     break; }
        case BCMD_VKL1:             { nxVklId = NEXUS_SecurityVirtualKeyladderID_eVKL1;     break; }
        case BCMD_VKL2:             { nxVklId = NEXUS_SecurityVirtualKeyladderID_eVKL2;     break; }
        case BCMD_VKL3:             { nxVklId = NEXUS_SecurityVirtualKeyladderID_eVKL3;     break; }
        case BCMD_VKL4:             { nxVklId = NEXUS_SecurityVirtualKeyladderID_eVKL4;     break; }
        case BCMD_VKL5:             { nxVklId = NEXUS_SecurityVirtualKeyladderID_eVKL5;     break; }
        case BCMD_VKL6:             { nxVklId = NEXUS_SecurityVirtualKeyladderID_eVKL6;     break; }
        case BCMD_VKL7:             { nxVklId = NEXUS_SecurityVirtualKeyladderID_eVKL7;     break; }
        case BCMD_VKL_KeyRam_eMax:  { nxVklId = NEXUS_SecurityVirtualKeyladderID_eVKLDummy; break; }
        default:{
            BDBG_ERR(("VKL[0x%X]", vklId ));
            BERR_TRACE( NEXUS_INVALID_PARAMETER );
            break;
        }
    }

    /* replace the BHSM allocation flag */
    if( masked ) nxVklId |= BHSM_VKL_ID_ALLOCATION_FLAG;

    return nxVklId;
}

BCMD_KeyMode_e NEXUS_Security_P_mapNexus2Hsm_KeyMode( NEXUS_SecurityKeyMode keyMode )
{
    BCMD_KeyMode_e hsmKeyMode = BCMD_KeyMode_eMax;

    switch( keyMode ) {
        case NEXUS_SecurityKeyMode_eRegular:        { hsmKeyMode = BCMD_KeyMode_eRegular; break; }
        case NEXUS_SecurityKeyMode_eDes56:          { hsmKeyMode = BCMD_KeyMode_eDes56; break; }
        case NEXUS_SecurityKeyMode_eReserved2:      { hsmKeyMode = BCMD_KeyMode_eReserved2; break; }
        case NEXUS_SecurityKeyMode_eReserved3:      { hsmKeyMode = BCMD_KeyMode_eReserved3; break; }
        case NEXUS_SecurityKeyMode_eDvbConformance: { hsmKeyMode = BCMD_KeyMode_eDvbConformance; break; }
        case NEXUS_SecurityKeyMode_eCwc:            {
       #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
            hsmKeyMode = BCMD_KeyMode_eReserved5;
       #endif /* use default for less that zeus3  */
            break;
         }
        case NEXUS_SecurityKeyMode_eMax:            { hsmKeyMode = BCMD_KeyMode_eMax; break; }
        default: { BERR_TRACE( NEXUS_INVALID_PARAMETER ); break; }
    }

    return hsmKeyMode;
}

BCMD_CustomerSubMode_e NEXUS_Security_P_mapNexus2Hsm_CustomerSubMode( NEXUS_SecurityCustomerSubMode cusSubMode )
{
    BCMD_CustomerSubMode_e hsmCusSubMode;

    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CA_64_4   == (int)BCMD_CustomerSubMode_eGeneric_CA_64_4 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CP_64_4   == (int)BCMD_CustomerSubMode_eGeneric_CP_64_4 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CA_64_5   == (int)BCMD_CustomerSubMode_eGeneric_CA_64_5 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CP_64_5   == (int)BCMD_CustomerSubMode_eGeneric_CP_64_5 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CA_128_4  == (int)BCMD_CustomerSubMode_eGeneric_CA_128_4 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4  == (int)BCMD_CustomerSubMode_eGeneric_CP_128_4 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CA_128_5  == (int)BCMD_CustomerSubMode_eGeneric_CA_128_5 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_5  == (int)BCMD_CustomerSubMode_eGeneric_CP_128_5 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CA_64_7   == (int)BCMD_CustomerSubMode_eGeneric_CA_64_7 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CA_128_7  == (int)BCMD_CustomerSubMode_eGeneric_CA_128_7 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eReserved10        == (int)BCMD_CustomerSubMode_eReserved10 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eReserved11        == (int)BCMD_CustomerSubMode_eReserved11 );
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eSage_128_5        == (int)BCMD_CustomerSubMode_eReserved12 );
   #endif
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eReserved13        == (int)BCMD_CustomerSubMode_eReserved13 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneralPurpose1   == (int)BCMD_CustomerSubMode_eGeneralPurpose1 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneralPurpose2   == (int)BCMD_CustomerSubMode_eGeneralPurpose2 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eReserved16        == (int)BCMD_CustomerSubMode_eReserved16 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eReserved17        == (int)BCMD_CustomerSubMode_eReserved17 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CA_64_45  == (int)BCMD_CustomerSubMode_eGeneric_CA_64_45 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eGeneric_CP_64_45  == (int)BCMD_CustomerSubMode_eGeneric_CP_64_45 );
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eReserved20        == (int)BCMD_CustomerSubMode_eHWKL );
   #endif
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eReserved21        == (int)BCMD_CustomerSubMode_eReserved21 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eSecureRSA2        == (int)0x16 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eETSI_5            == (int)BCMD_CustomerSubMode_eETSI_5 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eDTA_M_CA          == (int)BCMD_CustomerSubMode_eReserved24 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eDTA_M_CP          == (int)BCMD_CustomerSubMode_eReserved25 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eDTA_C_CA          == (int)BCMD_CustomerSubMode_eReserved26 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eDTA_C_CP          == (int)BCMD_CustomerSubMode_eReserved27 );
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eScte52CA5         == (int)BCMD_CustomerSubMode_eSCTE52_CA_5 );
   #endif
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eSageBlDecrypt     == (int)BCMD_CustomerSubMode_eSAGE_BL_DECRYPT );
   #endif
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eVistaKeyLadder    == (int)BCMD_CustomerSubMode_eReserved30 );
   #endif
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eVistaCwc          == (int)BCMD_CustomerSubMode_eRESERVED31 );
   #endif
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eReserved32        == (int)BCMD_CustomerSubMode_eReserved32 );
   #endif
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eReserved33        == (int)BCMD_CustomerSubMode_eReserved33 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eReserved34        == (int)BCMD_CustomerSubMode_eReserved34 );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eDupleSource       == (int)BCMD_CustomerSubMode_eDupleSource );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eDupleDestination  == (int)BCMD_CustomerSubMode_eDupleDestination );
    BDBG_CASSERT( (int)NEXUS_SecurityCustomerSubMode_eOTPKeyFieldProgramDataDecrypt == (int)BCMD_CustomerSubMode_eOTPKeyFieldProgramDataDecrypt );
   #endif

    /* pass the cusSubMode though as it was input. */
    /* coverity[mixed_enums] */
    hsmCusSubMode = cusSubMode;

    return hsmCusSubMode;
}

BCMD_RootKeySrc_e NEXUS_Security_P_mapNexus2Hsm_RootKeySource( NEXUS_SecurityRootKeySrc rootKeySource )
{
    BCMD_RootKeySrc_e hsmRootKeySource = BCMD_RootKeySrc_eMax;

    switch( rootKeySource )
    {
        case NEXUS_SecurityRootKeySrc_eCuskey:    { hsmRootKeySource = BCMD_RootKeySrc_eCusKey; break; }
        case NEXUS_SecurityRootKeySrc_eOtpKeyA:   { hsmRootKeySource = BCMD_RootKeySrc_eOTPKeya; break; }
        case NEXUS_SecurityRootKeySrc_eOtpKeyB:   { hsmRootKeySource = BCMD_RootKeySrc_eOTPKeyb; break; }
        case NEXUS_SecurityRootKeySrc_eOtpKeyC:   { hsmRootKeySource = BCMD_RootKeySrc_eOTPKeyc; break; }
        case NEXUS_SecurityRootKeySrc_eOtpKeyD:   { hsmRootKeySource = BCMD_RootKeySrc_eOTPKeyd; break; }
        case NEXUS_SecurityRootKeySrc_eOtpKeyE:   { hsmRootKeySource = BCMD_RootKeySrc_eOTPKeye; break; }
        case NEXUS_SecurityRootKeySrc_eOtpKeyF:   { hsmRootKeySource = BCMD_RootKeySrc_eOTPKeyf; break; }
        case NEXUS_SecurityRootKeySrc_eReserved0: { hsmRootKeySource = BCMD_RootKeySrc_eReserved9; break; }
        case NEXUS_SecurityRootKeySrc_eMax:       { break; }
       #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
        case NEXUS_SecurityRootKeySrc_eOtpKeyG:   { hsmRootKeySource = BCMD_RootKeySrc_eOTPKeyg; break; }
        case NEXUS_SecurityRootKeySrc_eOtpKeyH:   { hsmRootKeySource = BCMD_RootKeySrc_eOTPKeyh; break; }
        case NEXUS_SecurityRootKeySrc_eReserved1: { hsmRootKeySource = BCMD_RootKeySrc_eReserved10; break; }
        case NEXUS_SecurityRootKeySrc_eReserved2: { hsmRootKeySource = BCMD_RootKeySrc_eReserved11; break; }
        case NEXUS_SecurityRootKeySrc_eGlobalKey: { hsmRootKeySource = BCMD_RootKeySrc_eASKMGlobalKey; break; }
       #else
        case NEXUS_SecurityRootKeySrc_eOtpKeyG:
        case NEXUS_SecurityRootKeySrc_eOtpKeyH:
        case NEXUS_SecurityRootKeySrc_eReserved1:
        case NEXUS_SecurityRootKeySrc_eReserved2:
        case NEXUS_SecurityRootKeySrc_eGlobalKey:
       #endif
        /* leave clear for above to fall though */
        default:{
            BDBG_ERR(("Root Source Key[0x%X]", rootKeySource ));
            BERR_TRACE( NEXUS_INVALID_PARAMETER );
            break;
        }
    }

    return hsmRootKeySource;
}


static NEXUS_Error NEXUS_Security_AllocateM2mKeySlot(NEXUS_KeySlotHandle * pKeyHandle,const NEXUS_SecurityKeySlotSettings *pSettings, BCMD_XptSecKeySlot_e type)
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
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
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

static BCMD_XptM2MSecCryptoAlg_e mapNexus2Hsm_Algorithm( NEXUS_SecurityAlgorithm algorithm )
{
    switch( algorithm )
    {
        case NEXUS_SecurityAlgorithm_eMulti2:         return BCMD_XptM2MSecCryptoAlg_eMulti2;
        case NEXUS_SecurityAlgorithm_eDes:            return BCMD_XptM2MSecCryptoAlg_eDes;
        case NEXUS_SecurityAlgorithm_e3DesAba:        return BCMD_XptM2MSecCryptoAlg_e3DesAba;
        case NEXUS_SecurityAlgorithm_e3DesAbc:        return BCMD_XptM2MSecCryptoAlg_e3DesAbc;
        case NEXUS_SecurityAlgorithm_eAes:            return BCMD_XptM2MSecCryptoAlg_eAes128;
        case NEXUS_SecurityAlgorithm_eAes192:         return BCMD_XptM2MSecCryptoAlg_eAes192;
        case NEXUS_SecurityAlgorithm_eC2:             return BCMD_XptM2MSecCryptoAlg_eC2;
        case NEXUS_SecurityAlgorithm_eM6Ke:           return BCMD_XptM2MSecCryptoAlg_eM6KE;
        case NEXUS_SecurityAlgorithm_eM6:             return BCMD_XptM2MSecCryptoAlg_eM6;
        case NEXUS_SecurityAlgorithm_eWMDrmPd:        return BCMD_XptM2MSecCryptoAlg_eWMDrmPd;
        case NEXUS_SecurityAlgorithm_eDvbCsa3:        return BCMD_XptM2MSecCryptoAlg_eDVBCSA3;
        case NEXUS_SecurityAlgorithm_eAesCounter:     return BCMD_XptM2MSecCryptoAlg_eAesCounter0;
        case NEXUS_SecurityAlgorithm_eMSMultiSwapMac: return BCMD_XptM2MSecCryptoAlg_eMSMULTISWAPMAC;
        case NEXUS_SecurityAlgorithm_eAsa:            return BCMD_XptM2MSecCryptoAlg_eReserved19;

        case NEXUS_SecurityAlgorithm_eReserved0:       return (BCMD_XptM2MSecCryptoAlg_e)0;
        case NEXUS_SecurityAlgorithm_eReserved1:       return (BCMD_XptM2MSecCryptoAlg_e)1;
        case NEXUS_SecurityAlgorithm_eReserved2:       return (BCMD_XptM2MSecCryptoAlg_e)2;
        case NEXUS_SecurityAlgorithm_eReserved3:       return (BCMD_XptM2MSecCryptoAlg_e)3;
        case NEXUS_SecurityAlgorithm_eReserved4:       return (BCMD_XptM2MSecCryptoAlg_e)4;
        case NEXUS_SecurityAlgorithm_eReserved5:       return (BCMD_XptM2MSecCryptoAlg_e)5;
        case NEXUS_SecurityAlgorithm_eReserved6:       return (BCMD_XptM2MSecCryptoAlg_e)6;
        case NEXUS_SecurityAlgorithm_eReserved7:       return (BCMD_XptM2MSecCryptoAlg_e)7;
        case NEXUS_SecurityAlgorithm_eReserved8:       return (BCMD_XptM2MSecCryptoAlg_e)8;
        case NEXUS_SecurityAlgorithm_eReserved9:       return (BCMD_XptM2MSecCryptoAlg_e)9;
        default: break;
    }

    /* return input value.  */
    return (BCMD_XptM2MSecCryptoAlg_e)algorithm;
}

static BCMD_XptKeyTableCustomerMode_e mapNexus2Hsm_CustomerMode( NEXUS_Security_CustomerMode customerMode )
{
    switch( customerMode ) {
        case NEXUS_Security_CustomerMode_eGeneric:   { return BCMD_XptKeyTableCustomerMode_eGeneric; }
        case NEXUS_Security_CustomerMode_eReserved1: { return BCMD_XptKeyTableCustomerMode_eReserved1; }
        case NEXUS_Security_CustomerMode_eReserved2: { return BCMD_XptKeyTableCustomerMode_eReserved2; }
        case NEXUS_Security_CustomerMode_eReserved3: { return BCMD_XptKeyTableCustomerMode_eReserved3; }
        case NEXUS_Security_CustomerMode_eMax:       { return BCMD_XptKeyTableCustomerMode_eMax; }
        default:                                     { BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    }
    return BCMD_XptKeyTableCustomerMode_eMax;
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
    *pCryptAlg = mapNexus2Hsm_Algorithm(pSettings->algorithm);
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
        if ( pSettings->algorithm == NEXUS_SecurityAlgorithm_eAes ||
             pSettings->algorithm == NEXUS_SecurityAlgorithm_eAes192)
        {
            if (pSettings->algorithmVar == NEXUS_SecurityAlgorithmVariant_eCounter)
            {
                /*  Overload termination mode for M2M to handle AESCounter */
                /*  For Zeus 2.0 and earlier CounterMode and CounterSize are the same */
                /*  For Zeus 3.0, and 4.x, they are different */
                #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
                *pTerminationMode = pSettings->aesCounterMode;
                #else
                *pTerminationMode = pSettings->aesCounterSize;
                #endif
            }
        }
    }
}


static BCMD_KeyDestBlockType_e mapNexus2Hsm_blockType( NEXUS_KeySlotHandle keyslot,
                                                       NEXUS_SecurityAlgorithmConfigDestination dest )
{

    switch( keyslot->settings.keySlotEngine )
    {
        case NEXUS_SecurityEngine_eCa:  { return BCMD_KeyDestBlockType_eCA; }
        case NEXUS_SecurityEngine_eM2m: { return BCMD_KeyDestBlockType_eMem2Mem; }
        case NEXUS_SecurityEngine_eCaCp:
        {
            switch( dest )
            {
                case NEXUS_SecurityAlgorithmConfigDestination_eCa:  return BCMD_KeyDestBlockType_eCA;
                case NEXUS_SecurityAlgorithmConfigDestination_eCpd: return BCMD_KeyDestBlockType_eCPDescrambler;
                case NEXUS_SecurityAlgorithmConfigDestination_eCps: return BCMD_KeyDestBlockType_eCPScrambler;
                default:  BERR_TRACE( NEXUS_NOT_SUPPORTED ); break;
            }
            break;
        }
        case NEXUS_SecurityEngine_eCp:
        {
            switch( dest )
            {
                case NEXUS_SecurityAlgorithmConfigDestination_eCpd: return BCMD_KeyDestBlockType_eCPDescrambler;
                case NEXUS_SecurityAlgorithmConfigDestination_eCps: return BCMD_KeyDestBlockType_eCPScrambler;
                default:  BERR_TRACE( NEXUS_NOT_SUPPORTED ); break;
            }
            break;
        }
        default:  BERR_TRACE( NEXUS_NOT_SUPPORTED ); break;
    }

    return BCMD_KeyDestBlockType_eCA;
}

static BCMD_KeyDestEntryType_e mapNexus2Hsm_keyEntryType( NEXUS_SecurityKeyType keytype )
{
    switch( keytype )
    {
        case NEXUS_SecurityKeyType_eOddAndEven: return BCMD_KeyDestEntryType_eOddKey;
        case NEXUS_SecurityKeyType_eEven:       return BCMD_KeyDestEntryType_eEvenKey;
        case NEXUS_SecurityKeyType_eOdd:        return BCMD_KeyDestEntryType_eOddKey;
        case NEXUS_SecurityKeyType_eClear:      return BCMD_KeyDestEntryType_eClearKey;
        default: BERR_TRACE( NEXUS_INVALID_PARAMETER ); break; /* invalid type key entry/polarity.  */
    }
    return BCMD_KeyDestEntryType_eOddKey;
}

static  BCMD_KeyDestIVType_e mapNexus2Hsm_ivType( NEXUS_SecurityKeyIVType keyIVtype )
{
    switch (keyIVtype)
    {
        case NEXUS_SecurityKeyIVType_eNoIV:          return BCMD_KeyDestIVType_eNoIV;
        case NEXUS_SecurityKeyIVType_eIV:            return BCMD_KeyDestIVType_eIV;
        case NEXUS_SecurityKeyIVType_eAesShortIV:    return BCMD_KeyDestIVType_eAesShortIV;
        default: BERR_TRACE(NEXUS_INVALID_PARAMETER); break;
    }

    return BCMD_KeyDestIVType_eNoIV;
}

static int g_scValues[NEXUS_SecurityAlgorithmScPolarity_eMax] = { 0, 1, 2, 3 };


#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)

/* Zeus 4 */
static void NEXUS_Security_P_SetScValues( BHSM_ConfigAlgorithmIO_t *pConfigAlgorithmIO,
                                          const NEXUS_SecurityAlgorithmSettings *pSettings,
                                          NEXUS_SecurityKeyType keyType )
{

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
}


/* Zeus 4 */
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
    BCMD_SolitarySelect_e            solitarySelect;
    NEXUS_SecurityKeySource          keySrc;
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

    blockType = mapNexus2Hsm_blockType( keyHandle, pSettings->dest );
    bConfigOddAndEven = (keyDestEntryType == NEXUS_SecurityKeyType_eOddAndEven);

    /* if req. is for AV Keyladder, config only for KeyDestEntryType requested */
    if (keySrc == NEXUS_SecurityKeySource_eAvCPCW || keySrc == NEXUS_SecurityKeySource_eAvCW)
    {
        bConfigOddAndEven = false;
    }

    configAlgorithmIO.keyDestBlckType = blockType;
    configAlgorithmIO.keyDestEntryType = mapNexus2Hsm_keyEntryType( keyDestEntryType );
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
        /* IVModeCounterSize used to carry IvMode or CounterSize depending on cipher mode.*/
        /* coverity[mixed_enums] */
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
    configAlgorithmIO.cryptoAlg.customerType        = mapNexus2Hsm_CustomerMode( pSettings->customerType );
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
    if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

    BKNI_Memset( &configKeySlotIDDataIO, 0, sizeof(configKeySlotIDDataIO) );

    configKeySlotIDDataIO.keyDestBlckType  = blockType;
    configKeySlotIDDataIO.keyDestEntryType = mapNexus2Hsm_keyEntryType( keyDestEntryType );
    configKeySlotIDDataIO.keyDestIVType    = BCMD_KeyDestIVType_eNoIV;
    configKeySlotIDDataIO.unKeySlotNum     = unKeySlotNum;
    configKeySlotIDDataIO.caKeySlotType    = keySlotType;
    configKeySlotIDDataIO.CAVendorID       = pSettings->caVendorID;
    configKeySlotIDDataIO.STBOwnerIDSelect = pSettings->otpId;
    configKeySlotIDDataIO.ModuleID         = pSettings->askmModuleID;
    configKeySlotIDDataIO.key2Select       = pSettings->key2Select;

    rc = BHSM_ConfigKeySlotIDData( g_security.hsm, &configKeySlotIDDataIO );
    if( rc ) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

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
            if( rc ) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }
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
        if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

        /* Call BHSM_configKeySlotIDData() to set up ID part of  configuration odd key */
        configKeySlotIDDataIO.keyDestEntryType = BCMD_KeyDestEntryType_eEvenKey;
        rc = BHSM_ConfigKeySlotIDData(g_security.hsm, &configKeySlotIDDataIO);
        if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }
    }

    BDBG_LEAVE(NEXUS_Security_ConfigAlgorithm);
    return rc;
}


#else
/* Zeus 4 */
static void NEXUS_Security_P_SetScValues(BHSM_ConfigAlgorithmIO_t *pConfigAlgorithmIO, const NEXUS_SecurityAlgorithmSettings *pSettings, NEXUS_SecurityKeyType keyType) {
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
}

/* Zeus 4 */
NEXUS_Error NEXUS_Security_ConfigAlgorithm(NEXUS_KeySlotHandle keyHandle, const NEXUS_SecurityAlgorithmSettings *pSettings)
{
    BHSM_ConfigAlgorithmIO_t configAlgorithmIO;
    BCMD_XptSecKeySlot_e keySlotType;
    unsigned int unKeySlotNum = 0;
    bool bConfigOddAndEven = false;

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    BCMD_Aes128_CounterSize_e        counterSize;
    #endif
    NEXUS_Error rc = NEXUS_SUCCESS;
    BCMD_KeyDestBlockType_e blockType = BCMD_KeyDestBlockType_eCA;
    BHSM_ConfigKeySlotIDDataIO_t configKeySlotIDDataIO;
    BCMD_XptM2MSecCryptoAlg_e cryptAlg;
    BCMD_CipherModeSelect_e cipherMode;
    BCMD_TerminationMode_e terminationMode;
    BCMD_IVSelect_e ivModeSelect;
    BCMD_SolitarySelect_e solitarySelect;
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

    NEXUS_Security_GetHsmAlgorithmKeySetting(keyHandle, pSettings,
            &cryptAlg,
            &cipherMode,
            &terminationMode);
    ivModeSelect   = pSettings->ivMode;
    solitarySelect = pSettings->solitarySelect;
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    counterSize    = pSettings->aesCounterSize;
    #endif
    blockType = mapNexus2Hsm_blockType( keyHandle, pSettings->dest );

    /* coverity[const] */
    switch (blockType) {
    case BCMD_KeyDestBlockType_eCA:
    case BCMD_KeyDestBlockType_eCPScrambler:
    case BCMD_KeyDestBlockType_eCPDescrambler:
        bConfigOddAndEven = (pSettings->keyDestEntryType == NEXUS_SecurityKeyType_eOddAndEven);
        break;
    default:
        break;
    }

    /* if req. is for AV Keyladder, config only for KeyDestEntryType requested */
    if (keySrc == NEXUS_SecurityKeySource_eAvCPCW || keySrc == NEXUS_SecurityKeySource_eAvCW) {
        bConfigOddAndEven = false;
        bAVKeyladder = true;
    }

    configAlgorithmIO.keyDestBlckType = blockType;
    if( bAVKeyladder ) /* keep the KeyDestEntryType as requested for AV Keyladder */
    {
        configAlgorithmIO.keyDestEntryType = mapNexus2Hsm_keyEntryType( pSettings->keyDestEntryType );
    }
    else
    {
        if (bConfigOddAndEven)
        {
            configAlgorithmIO.keyDestEntryType = BCMD_KeyDestEntryType_eOddKey;
        }
        else
        {
            configAlgorithmIO.keyDestEntryType = mapNexus2Hsm_keyEntryType( pSettings->keyDestEntryType );
        }
    }
    configAlgorithmIO.caKeySlotType = keySlotType;
    configAlgorithmIO.unKeySlotNum = unKeySlotNum;
    if (keyHandle->settings.keySlotEngine == NEXUS_SecurityEngine_eM2m)
    {
        BKNI_Memset(&(configAlgorithmIO.cryptoAlg.m2mCryptAlg), 0, sizeof(configAlgorithmIO.cryptoAlg.m2mCryptAlg));

        configAlgorithmIO.cryptoAlg.m2mCryptAlg.m2mSecAlg = cryptAlg;
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
        if (pSettings->operation == NEXUS_SecurityOperation_eEncrypt)
            configAlgorithmIO.cryptoAlg.m2mCryptAlg.ucAuthCtrl = BHSM_M2mAuthCtrl_eScramble;
        else if (pSettings->operation == NEXUS_SecurityOperation_eDecrypt)
            configAlgorithmIO.cryptoAlg.m2mCryptAlg.ucAuthCtrl = BHSM_M2mAuthCtrl_eDescramble;
        else
            configAlgorithmIO.cryptoAlg.m2mCryptAlg.ucAuthCtrl = BHSM_M2mAuthCtrl_ePassThrough;

        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bEnableTimestamp = pSettings->enableTimestamps;
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bMscCtrlSel = pSettings->mscBitSelect;
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bDisallowGG = pSettings->bDisallowGG;
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bDisallowGR = pSettings->bDisallowGR;
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bDisallowRG = pSettings->bDisallowRG;
        configAlgorithmIO.cryptoAlg.m2mCryptAlg.bDisallowRR = pSettings->bDisallowRR;

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
        configAlgorithmIO.cryptoAlg.caCryptAlg.cipherDVBCSA2Mode = cipherMode;
        configAlgorithmIO.cryptoAlg.caCryptAlg.terminationMode = terminationMode;
        configAlgorithmIO.cryptoAlg.caCryptAlg.IVMode = ivModeSelect;
        configAlgorithmIO.cryptoAlg.caCryptAlg.solitaryMode = solitarySelect;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bRestrictEnable = pSettings->bRestrictEnable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bGlobalEnable = pSettings->bGlobalEnable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.ucMulti2KeySelect = pSettings->multi2KeySelect;
        configAlgorithmIO.cryptoAlg.caCryptAlg.keyOffset = pSettings->keyOffset;
        configAlgorithmIO.cryptoAlg.caCryptAlg.ivOffset = pSettings->ivOffset;
        configAlgorithmIO.cryptoAlg.caCryptAlg.ucMSCLengthSelect = pSettings->mscLengthSelect;
        configAlgorithmIO.cryptoAlg.caCryptAlg.customerType = mapNexus2Hsm_CustomerMode( pSettings->customerType );
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA2keyCtrl = pSettings->dvbCsa2keyCtrl;
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA2ivCtrl = pSettings->dvbCsa2ivCtrl;
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA2modEnabled = pSettings->dvbCsa2modEnabled;
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA3dvbcsaVar = pSettings->dvbCsa3dvbcsaVar;
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA3permutation = pSettings->dvbCsa3permutation;
        configAlgorithmIO.cryptoAlg.caCryptAlg.DVBCSA3modXRC = pSettings->dvbCsa3modXRC;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bUseExtKey = pSettings->enableExtKey;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bUseExtIV = pSettings->enableExtIv;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bEncryptBeforeRave = pSettings->bEncryptBeforeRave;
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
        configAlgorithmIO.cryptoAlg.caCryptAlg.bDropRregionPackets = pSettings->bRestrictSourceDropPktEnable;
        configAlgorithmIO.cryptoAlg.caCryptAlg.bGpipePackets2Rregion = pSettings->bRoutePipeToRestrictedRegion[NEXUS_SecurityPacketType_eGlobal];
        configAlgorithmIO.cryptoAlg.caCryptAlg.bRpipePackets2Rregion = pSettings->bRoutePipeToRestrictedRegion[NEXUS_SecurityPacketType_eRestricted];
        #endif
    }

    /* Call BHSM_configKeySlotIDData() to set up ID part of  configuration odd key */
    configKeySlotIDDataIO.keyDestBlckType = blockType;
    if (bAVKeyladder)
    {   /* keep the KeyDestEntryType as requested for AV Keyladder */
        configKeySlotIDDataIO.keyDestEntryType = mapNexus2Hsm_keyEntryType( pSettings->keyDestEntryType );
    }
    else
    {
        if( bConfigOddAndEven )
        {
            configKeySlotIDDataIO.keyDestEntryType = BCMD_KeyDestEntryType_eOddKey;
        }
        else
        {
            configKeySlotIDDataIO.keyDestEntryType = mapNexus2Hsm_keyEntryType( pSettings->keyDestEntryType );
        }
    }
    configKeySlotIDDataIO.keyDestIVType = BCMD_KeyDestIVType_eNoIV;
    configKeySlotIDDataIO.unKeySlotNum = unKeySlotNum;
    configKeySlotIDDataIO.caKeySlotType = keySlotType;
    configKeySlotIDDataIO.CAVendorID = pSettings->caVendorID;
    configKeySlotIDDataIO.STBOwnerIDSelect = pSettings->otpId;
    configKeySlotIDDataIO.ModuleID = pSettings->askmModuleID;
    configKeySlotIDDataIO.key2Select = pSettings->key2Select;

    rc = BHSM_ConfigKeySlotIDData(g_security.hsm, &configKeySlotIDDataIO);
    if (rc)
    {
        return BERR_TRACE(MAKE_HSM_ERR(rc));
    }

    if (keyHandle->settings.keySlotEngine != NEXUS_SecurityEngine_eM2m)
    {
        NEXUS_Security_P_SetScValues(&configAlgorithmIO, pSettings, bConfigOddAndEven ? NEXUS_SecurityKeyType_eOdd : pSettings->keyDestEntryType );
    }
    rc = BHSM_ConfigAlgorithm(g_security.hsm, &configAlgorithmIO);
    if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

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
            loadRouteUserKeyIO.keySize.eKeySize = BCMD_KeySize_e128;
            loadRouteUserKeyIO.bIsRouteKeyRequired = true;
            loadRouteUserKeyIO.keyDestEntryType = BCMD_KeyDestEntryType_eOddKey;
            loadRouteUserKeyIO.caKeySlotType = keySlotType;
            loadRouteUserKeyIO.unKeySlotNum = unKeySlotNum;
            loadRouteUserKeyIO.keyMode= BCMD_KeyMode_eRegular;
            rc = BHSM_LoadRouteUserKey (g_security.hsm, &loadRouteUserKeyIO);
            if (rc) {
                BDBG_ERR(("External Key/IV may not be enabled by OTP"));
                return BERR_TRACE(MAKE_HSM_ERR(rc));
            }
        }
    }

    if (bConfigOddAndEven)
    {
        configKeySlotIDDataIO.keyDestEntryType = BCMD_KeyDestEntryType_eEvenKey;

        rc = BHSM_ConfigKeySlotIDData(g_security.hsm, &configKeySlotIDDataIO);
        if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

        configAlgorithmIO.keyDestEntryType = BCMD_KeyDestEntryType_eEvenKey;
        if (keyHandle->settings.keySlotEngine != NEXUS_SecurityEngine_eM2m)
        {
            NEXUS_Security_P_SetScValues(&configAlgorithmIO, pSettings, NEXUS_SecurityKeyType_eEven );
        }

        rc = BHSM_ConfigAlgorithm(g_security.hsm, &configAlgorithmIO);
        if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }
    }

    BDBG_LEAVE(NEXUS_Security_ConfigAlgorithm);
    return rc;
}


#endif

NEXUS_Error NEXUS_KeySlot_GetExternalKeyData(
                                NEXUS_KeySlotHandle keyslot,
                                NEXUS_SecurityAlgorithmConfigDestination dest, /*cpd/cps/cps */
                                NEXUS_SecurityKeyType  keyDestEntryType,       /*odd/eve/clear*/
                                NEXUS_KeySlotExternalKeyData *pKeyData  )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
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

    keyLocation.caKeySlotType = keyslot->keyslotType;


    keyLocation.keyDestBlckType  = mapNexus2Hsm_blockType( keyslot, dest );
    keyLocation.unKeySlotNum     = keyslot->keySlotNumber;
    keyLocation.keyDestEntryType = mapNexus2Hsm_keyEntryType( keyDestEntryType );  /*odd/eve/clear*/

    rc = BHSM_GetExternalKeyIdentifier( g_security.hsm, &keyLocation, &extKey );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

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

#else
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
    BCMD_KeyDestIVType_e      ivType;
    NEXUS_Security_P_KeySlotData *pKeySlotData;

    BDBG_ENTER(NEXUS_Security_LoadClearKey);

    BDBG_OBJECT_ASSERT(keyHandle, NEXUS_KeySlot);
    blockType = BCMD_KeyDestBlockType_eCA;

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

    blockType = mapNexus2Hsm_blockType( keyHandle, pClearKey->dest );
    entryType = mapNexus2Hsm_keyEntryType( keyEntryType );
    ivType = mapNexus2Hsm_ivType( pClearKey->keyIVType );

    BKNI_Memset(&loadRouteUserKeyIO, 0, sizeof(loadRouteUserKeyIO));
    if (pClearKey->keySize) {
        if (pClearKey->keySize==8)
            loadRouteUserKeyIO.keySize.eKeySize = BCMD_KeySize_e64;
        else if (pClearKey->keySize==16)
            loadRouteUserKeyIO.keySize.eKeySize = BCMD_KeySize_e128;
        else if (pClearKey->keySize==24)
            loadRouteUserKeyIO.keySize.eKeySize = BCMD_KeySize_e192;
       #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
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
        loadRouteUserKeyIO.caKeySlotType = keyHandle->keyslotType;
        loadRouteUserKeyIO.unKeySlotNum = keyHandle->keySlotNumber;
        loadRouteUserKeyIO.keyMode= BCMD_KeyMode_eRegular;
        loadRouteUserKeyIO.keyDestIVType = ivType;
       #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)

        if ( pClearKey->sc01Polarity[NEXUS_SecurityPacketType_eGlobal] >= NEXUS_SecurityAlgorithmScPolarity_eMax ||
             pClearKey->sc01Polarity[NEXUS_SecurityPacketType_eRestricted] >= NEXUS_SecurityAlgorithmScPolarity_eMax)
        {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        loadRouteUserKeyIO.GpipeSC01Val = g_scValues[pClearKey->sc01Polarity[NEXUS_SecurityPacketType_eGlobal]];
        loadRouteUserKeyIO.RpipeSC01Val = g_scValues[pClearKey->sc01Polarity[NEXUS_SecurityPacketType_eRestricted]];
       #endif
        rc = BHSM_LoadRouteUserKey(g_security.hsm, &loadRouteUserKeyIO);
        if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }
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
        if( keyHandle == NULL ) { (void)BERR_TRACE( NEXUS_NOT_AVAILABLE ); }

        return keyHandle;
    }

    pKeySlotData = BKNI_Malloc(sizeof(*pKeySlotData));
    if( !pKeySlotData ) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY ); return NULL; }

    switch (pSettings->keySlotEngine)
    {
        case NEXUS_SecurityEngine_eM2m:
            type = NEXUS_Security_P_mapNexus2Hsm_KeyslotType( pSettings->keySlotType, pSettings->keySlotEngine );

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
            if( rc != NEXUS_SUCCESS ) { rc = BERR_TRACE( rc ); goto err_alloc; }
            break;

        case NEXUS_SecurityEngine_eCaCp:
        case NEXUS_SecurityEngine_eCa:
            type = NEXUS_Security_P_mapNexus2Hsm_KeyslotType( pSettings->keySlotType, pSettings->keySlotEngine );

            rc = NEXUS_Security_AllocateKeySlotForType( &keyHandle, pSettings, type );
            if( rc != NEXUS_SUCCESS ) { rc = BERR_TRACE( rc ); goto err_alloc; }
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
        keyHandle->settings.keySlotType = mapHsm2Nexus_keySlotType( keyHandle->keyslotType );
    }

    BLST_S_INIT( &pKeySlotData->pidChannelList );
    BLST_S_INSERT_HEAD( &g_security.keyslotList, keyHandle, next );

#if NEXUS_HAS_XPT_DMA
    if( pSettings->keySlotEngine == NEXUS_SecurityEngine_eM2m )
    {
        /* XPT-based DMA requires an internal pid channel. */
        NEXUS_Module_Lock( g_security.moduleSettings.transport );
        pKeySlotData->dmaPidChannelHandle = NEXUS_PidChannel_OpenDma_Priv(0, NULL);
        NEXUS_Module_Unlock( g_security.moduleSettings.transport );
        if ( pKeySlotData->dmaPidChannelHandle == NULL ) {
            BERR_TRACE(NEXUS_NOT_AVAILABLE);
            BLST_S_REMOVE( &g_security.keyslotList, keyHandle, NEXUS_KeySlot, next );
            goto err_alloc;
        }

        keyHandle->dma.pidChannelIndex = NEXUS_PidChannel_GetIndex_isrsafe( pKeySlotData->dmaPidChannelHandle );
        keyHandle->dma.valid = true;
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
    BHSM_KeySlotAllocate_t  keySlotConf;

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
            keySlotConf.client         = pKeySlotData->keySlotClient;
            keySlotConf.keySlotNum     = keyHandle->keySlotNumber;
            keySlotConf.keySlotType    = keyHandle->keyslotType;
            keySlotConf.pidChannelType = BHSM_PidChannelType_ePrimary;
           #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
            rc = BHSM_FreeCAKeySlot( g_security.hsm, &keySlotConf );
           #else
            rc = BHSM_FreeCAKeySlot_v2( g_security.hsm, &keySlotConf );
           #endif
            if( rc != NEXUS_SUCCESS ) { BERR_TRACE(MAKE_HSM_ERR(rc)); }
            break;
        case NEXUS_SecurityEngine_eM2m:
            BDBG_MSG(("Freeing m2m keyslot (%d)", keyHandle->keySlotNumber));
            M2MKeySlotIO.keySlotNum  = keyHandle->keySlotNumber;
            M2MKeySlotIO.keySlotType = keyHandle->keyslotType;
            M2MKeySlotIO.client      = pKeySlotData->keySlotClient;
            rc = BHSM_FreeM2MKeySlot(g_security.hsm, &M2MKeySlotIO);
            if( rc != NEXUS_SUCCESS ) { BERR_TRACE(MAKE_HSM_ERR(rc)); }
            break;
        case NEXUS_SecurityEngine_eCp:  rc = BERR_TRACE( NEXUS_INVALID_PARAMETER ); break;
        case NEXUS_SecurityEngine_eRmx: rc = BERR_TRACE( NEXUS_INVALID_PARAMETER ); break;
        case NEXUS_SecurityEngine_eGeneric:
        default:
            break;
    }

#if NEXUS_HAS_XPT_DMA
    if( keyHandle->settings.keySlotEngine==NEXUS_SecurityEngine_eM2m && pKeySlotData->dmaPidChannelHandle != NULL )
    {
        NEXUS_Security_RemovePidChannelFromKeySlot( keyHandle, keyHandle->dma.pidChannelIndex );
        keyHandle->dma.valid = false;

        NEXUS_PidChannel_CloseDma_Priv(pKeySlotData->dmaPidChannelHandle);
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
    pSettings->keySrc = NEXUS_SecurityKeySource_eFirstRamAskm;
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
    if (!pKeySlotData) { return  BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    BDBG_MSG(("NEXUS_Security_P_InvalidateKey: %p(%d,%d)", (void *)keyHandle,keyHandle->keySlotNumber,keyHandle->keyslotType));

    BKNI_Memset(&invalidateKeyIO, 0, sizeof(invalidateKeyIO)); /* Coverity defect ID 35508; SW7435-1079 */

    nrc = NEXUS_Security_P_GetInvalidateKeyFlag(pSettings->invalidateKeyType, &invalidateKeyIO.invalidKeyType);
    if( nrc ) { return BERR_TRACE(nrc); }

    invalidateKeyIO.keyDestBlckType = mapNexus2Hsm_blockType( keyHandle, pSettings->keyDestBlckType );
    invalidateKeyIO.keyDestEntryType = mapNexus2Hsm_keyEntryType( pSettings->keyDestEntryType );
    invalidateKeyIO.caKeySlotType = (BCMD_XptSecKeySlot_e)keyHandle->keyslotType;
    invalidateKeyIO.unKeySlotNum = keyHandle->keySlotNumber;
    invalidateKeyIO.virtualKeyLadderID = NEXUS_Security_P_mapNexus2Hsm_VklId( pSettings->virtualKeyLadderID );
    invalidateKeyIO.keyLayer = NEXUS_Security_P_mapNexus2Hsm_KeySource( pSettings->keySrc );

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
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
        if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

        keySettings.keyDestEntryType = NEXUS_SecurityKeyType_eEven;

        rc = NEXUS_Security_P_InvalidateKey(keyHandle, &keySettings);
        if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

        if( keyHandle->settings.keySlotEngine == NEXUS_SecurityEngine_eCaCp )
        {
            keySettings.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
            rc = NEXUS_Security_P_InvalidateKey(keyHandle, &keySettings);
            if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }
        }
    }
    else
    {
        keySettings.keyDestEntryType = keyEntryType;

        rc = NEXUS_Security_P_InvalidateKey( keyHandle, &keySettings );
        if (rc) { return BERR_TRACE( MAKE_HSM_ERR(rc) ); }
    }

    BDBG_LEAVE(NEXUS_Security_InvalidateKey);
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SecurityModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    BERR_Code rc;
    NEXUS_SecurityRegionModuleSettings regVerSettings;
    int i;

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

            NEXUS_Security_GetDefaultRegionVerificationModuleSettings( &regVerSettings );
            for( i = 0; i < NEXUS_SecurityFirmwareType_eMax; i++ ) {
                regVerSettings.enforceAuthentication[i] = g_security.settings.enforceAuthentication[i];
            }
            /* Reinitialise Region Verification module. */
            rc = NEXUS_Security_RegionVerification_Init_priv( &regVerSettings );
            if (rc) { return BERR_TRACE(rc); }

            rc = NEXUS_RegionVerify_Init( );
            if (rc) { return BERR_TRACE(rc); }
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
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
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
    if (rc) { return BERR_TRACE(MAKE_HSM_ERR(rc)); }

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
    BDBG_CASSERT( (int)NEXUS_BypassKeySlot_eGT2T     == (int)BHSM_BypassKeySlot_eGT2T );
    BDBG_CASSERT( (int)NEXUS_BypassKeySlot_eMax      == (int)BHSM_BypassKeySlot_eInvalid  );

    rc = BHSM_SetPidChannelBypassKeyslot( g_security.hsm,
                                          NEXUS_PidChannel_GetIndex_isrsafe(pidChannel),
                                          (BHSM_BypassKeySlot_e)bypassKeySlot );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( MAKE_HSM_ERR(rc) ); }

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

    rc = BHSM_GetPidChannelBypassKeyslot( g_security.hsm,
                                          NEXUS_PidChannel_GetIndex_isrsafe(pidChannel),
                                          (BHSM_BypassKeySlot_e*)pBypassKeySlot );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); return; }
#else
    BSTD_UNUSED( pidChannel );
    BSTD_UNUSED( pBypassKeySlot );
#endif
    return;
}
