/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 **************************************************************************/

#include "nexus_scm_module.h"

#include "priv/nexus_core.h" /* get access to g_pCoreHandles */
#include "nexus_security.h"
#include "priv/nexus_security_regver_priv.h"
#include "nexus_security_bseckcmd.h"
#include "nexus_otpmsp.h"
#include "nexus_bsp_config.h"
#include "nexus_keyladder.h"
#include "nexus_scm_image.h"
#include "nexus_scm_boot_types.h"

#include "bkni.h"
#include "bhsm.h"

#if NEXUS_POWER_MANAGEMENT
#include "bchp_pwr.h"
#include "bchp_pwr_resources.h"
#endif

NEXUS_ScmModule_P_Handle g_NEXUS_scmModule;


BDBG_MODULE(nexus_scm_module);

/* Host to Scm communication buffer size */
#define SCM_HOST_BUF_SIZE (0x2000)

#define SCM_HEADER_TYPE_BL      (0x53570000)
#define SCM_HEADER_TYPE_OS_APP  (0x424C0000)
#define SCM_BINARY_TYPE_ZB      (0x3E3F)
#define SCM_BINARY_TYPE_ZS      (0x0202)

#include "bchp_common.h"
#define SCM_BL_LENGTH (BCHP_SCPU_LOCALRAM_REG_END - BCHP_SCPU_LOCALRAM_REG_START + 4)

#define _SIG_SZ (256)


typedef enum ScmChipType {
    ScmChipType_eZS = 0,
    ScmChipType_eZB,
    ScmChipType_eCustomer
}ScmChipType;


typedef struct NEXUS_ScmMemoryBlock {
    size_t len;
    void *buf;
} NEXUS_ScmMemoryBlock;



typedef struct NEXUS_ScmCodeRegionSettings
{
    /*NEXUS_SecurityRegverRegionID      regionID;*/
    /* unsigned char                     msbCodeStartPhyAddress; */
    unsigned int                      codeStartPhyAddress;
    unsigned int                      codeSize;
    /*unsigned char                     msbSigStartPhyAddress;*/
    unsigned int                      sigStartPhyAddress;
    /* NEXUS_SecurityRegverSignatureSize sigSize; */
    /* unsigned char                     intervalCheckBw;*/
    /* NEXUS_SecurityRegverSCBSize       SCBBurstSize;*/
    /* unsigned char                     cpuAccessRights;*/
    /* NEXUS_SecurityRegverCPUType       cpuType; */
    /* unsigned                          codeRule; */
    /* NEXUS_SecurityRegverFailAction    verifyFailAction; */
    unsigned int                      epoch;
    unsigned int                      epochMask;
    unsigned char                     epochSel;
    /* unsigned char                     sigVersion; */
    /* unsigned char                     sigType;*/
    unsigned int                      marketID;
    unsigned int                      marketIDMask;
    /* NEXUS_SecurityCodeLocationRule    codeRelocatable;*/
    /* NEXUS_SecurityRegverSigningKeyID  keyID; */
    NEXUS_SecurityVirtualKeyladderID  keyLadder;
    NEXUS_SecurityKeySource           keyLayer;
    /* NEXUS_SecurityRegverBGChecker     bgCheck;*/
    /* NEXUS_SecurityRegverInstrChecker  instrCheck; */
    unsigned int                      SCMVersion;
} NEXUS_ScmCodeRegionSettings;



/* This type is not available in any security header. */
typedef struct BCMD_SecondTierKey_t
{
    uint8_t     ucKeyData[256];
    uint8_t     ucRights;       /* 0 => MIPS; 2 => AVD, RAPTOR, RAVE; 4 => BSP; 8 => Boot; 10 => SCM */
    uint8_t     ucReserved0;
    uint8_t     ucPubExponent;  /* 0 => 3; 1 => 64K+1 */
    uint8_t     ucReserved1;
    uint8_t     ucMarketID[4];
    uint8_t     ucMarketIDMask[4];
#if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2))
    unsigned char ucReserved2;
    unsigned char ucEpochSelect;
    unsigned char ucEpochMask;
    unsigned char ucEpoch;
    unsigned char ucSignatureVersion;
    unsigned char ucSignatureType;
    unsigned char ucReserved3[2];
#else
    unsigned char usReserved2[2];
    unsigned char ucEpochMask;
    unsigned char ucEpoch;
#endif
    uint8_t     ucSignature[256];
} BCMD_SecondTierKey_t;


static struct {
    NEXUS_ScmModuleSettings settings;    /* Nexus scm module settings, given in NEXUS_ScmModule_Init */

    /* Secure Boot Type */
    uint8_t     otp_scm_decrypt_enable;
    uint8_t     otp_scm_verify_enable;
    uint8_t     otp_scm_secure_enable;

    /* Chip Type */
    ScmChipType otp_chip_type;

    /* SCM reserved VKL */
    NEXUS_VirtualKeyLadderHandle vklHandle1;
    NEXUS_VirtualKeyLadderHandle vklHandle2;
    uint32_t scmReservedVklMask;

    /* second tier key for set boot params */
    BCMD_SecondTierKey_t *scm_bl_second_tier_key;

    /* security features */
    uint32_t zeus;
    uint32_t firmware;
} g_scm_module;


typedef struct NEXUS_ScmSecureHeader
{
    unsigned char ucHeaderIndex[4];
    unsigned char ucSecurityType;
    unsigned char ucImageType;
    unsigned char ucReserved[2];
    unsigned char ucScmSfwVersion[4];
    unsigned char ucScmBlVersion[4];
    unsigned char ucPlatformId[4];
    unsigned char ucCaVendorId[2];
    unsigned char ucStbOwnerIdSelect;
    unsigned char ucSwizzle0aVariant;
    unsigned char ucSwizzle0aVersion;
    unsigned char ucCustKeyVar;
    unsigned char ucKeyVarHi;
    unsigned char ucKeyVarlo;
    unsigned char ucModuleId;
    unsigned char ucReserved1[3]; /*[1] - scm type; [2] - scm version  */
    unsigned char ucProcIn1[16];
    unsigned char ucProcIn2[16];
    unsigned char ucProcIn3[16];
    BCMD_SecondTierKey_t second_tier_key; /* Zeus 2/3/4.1 - size: 528 bytes - Zeus 4.2 - total size: 532 bytes */
    unsigned char ucSize[4];
    unsigned char ucInstrSectionSize[4];
    unsigned char ucDataSectionAddr[4];
    unsigned char ucDataSectionSize[4];
} NEXUS_ScmSecureHeader;

/* NEXUS_ScmImageHolder is a special structure that is used to hold a
 * binary in memory
 * Basically it is used to load two types of binary
 *  - SCM Bootloader: id=SCM_IMAGE_FirmwareID_eBootLoader
 *  - SCM OS/Application: id=SCM_IMAGE_FirmwareID_eOS_App
 * The raw binary is dumped in a NEXUS_ScmMemoryBlock.
 * header, signature and data fields points to areas in raw memory
 +------------+---------------+-------------------------------------------------
 |  header    |   signature   |    binary data (kernel, bootloader)      . . .
 +------------+---------------+-------------------------------------------------
 |            |               |
 header ptr   signature ptr   data ptr
*/
typedef struct NEXUS_ScmImageHolder {
    const char *name;         /* printable name */
    SCM_IMAGE_FirmwareID id; /* SCM_IMAGE_FirmwareID_eOS_App or SCM_IMAGE_FirmwareID_eBootLoader */
    /* binfile representation: */
    NEXUS_ScmMemoryBlock *raw;
    NEXUS_ScmSecureHeader *header;
    uint8_t *signature;
    uint8_t *data;
    uint32_t data_len;
} NEXUS_ScmImageHolder;


/****************************************
 * Local functions
 ****************************************/

static NEXUS_Error NEXUS_ScmModule_P_Load(
    NEXUS_ScmImageHolder *holder,
    BIMG_Interface *img_interface,
    void * img_context);

static void NEXUS_ScmModule_P_SetBootParams(void);

static NEXUS_Error NEXUS_ScmModule_P_Reset(NEXUS_ScmImageHolder *header);
#if 0
static NEXUS_Error NEXUS_ScmModule_P_InitializeScmVkl(void);
#endif
static NEXUS_Error NEXUS_ScmModule_P_InitializeTimer(void);
static NEXUS_Error NEXUS_ScmModule_P_GetOtpMspParams(void);

#define KEY_TIER 2

#if (KEY_TIER == 3)
static NEXUS_Error NEXUS_ScmModule_P_VerifySageKey(uint32_t key_offset);
#endif
static NEXUS_Error NEXUS_ScmModule_P_VeryfyScmRegion(NEXUS_ScmCodeRegionSettings * rs);

NEXUS_Error NEXUS_ScmModule_P_DumpVKL(void);
NEXUS_Error NEXUS_ScmModule_P_EpochRead(void);

/****************************************
 * Macros
 ****************************************/

#define _ScmModule_EndianSwap(PVAL) (((PVAL)[0] << 24) | ((PVAL)[1] << 16) | ((PVAL)[2] << 8) | ((PVAL)[3]))
#define _ScmModule_ChipTypeSwap(PVAL) (((PVAL)[0] << 8) | ((PVAL)[1]))

/* build code for writing all NEXUS_ScmGlobalSram_e* register value
 * used in NEXUS_ScmModule_P_SetBootParams() and NEXUS_Scm_P_CleanBootVars() */
#if 0
#define BootParamDbgPrintf(format) BDBG_MSG(format)
#else
#define BootParamDbgPrintf(format)
#endif
#define _ScmModule_SetBootParam(REGID, VAL) {      \
        uint32_t addr = NEXUS_ScmGlobalSram_GetRegister(NEXUS_ScmGlobalSram_e##REGID); \
        BREG_Write32(g_pCoreHandles->reg, addr, VAL);                   \
        BootParamDbgPrintf(("%s - Read %s - Addr: %p, Value: %08x", BSTD_FUNCTION, #REGID, addr, BREG_Read32(g_pCoreHandles->reg, addr))); }


/****************************************
 * Module 'public' functions
 * called by the Nexus architecture
 * upon initialization/finalization of the
 * platform (see platform_init.c, ... )
 ****************************************/

void NEXUS_ScmModule_GetDefaultSettings(NEXUS_ScmModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->clientHeapIndex = NEXUS_MAX_HEAPS;
}

/* Init the timer Module */
static NEXUS_Error NEXUS_ScmModule_P_InitializeTimer(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BTMR_TimerSettings timerSettings = { BTMR_Type_eSharedFreeRun, NULL, NULL, 0, false };
    if(BTMR_CreateTimer(g_pCoreHandles->tmr, &g_NEXUS_scmModule.hTimer, &timerSettings) != BERR_SUCCESS) {
        BDBG_ERR(("%s - BTMR_CreateTimer failure", BSTD_FUNCTION));
        rc = NEXUS_NOT_INITIALIZED;
    }
    else {
        g_NEXUS_scmModule.timerMax = BTMR_ReadTimerMax();
        /* failure is acceptable here */
    }

    return rc;
}

#define OTP_SWIZZLE0A_MSP0_VALUE_ZS (0x02)
#define OTP_SWIZZLE0A_MSP1_VALUE_ZS (0x02)
#define OTP_SWIZLLE0A_MSP0_VALUE_ZB (0x3E)
#define OTP_SWIZLLE0A_MSP1_VALUE_ZB (0x3F)

/* Check OtpMsp parameters to see if decryption and verification are enabled. */
static NEXUS_Error NEXUS_ScmModule_P_GetOtpMspParams(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_ReadMspParms msp_params;
    NEXUS_ReadMspIO     msp_io;

    /* SCM decryption enabled? */
    BKNI_Memset(&msp_params, 0, sizeof(NEXUS_ReadMspParms));
    BKNI_Memset(&msp_io, 0, sizeof(NEXUS_ReadMspIO));
    msp_params.readMspEnum = NEXUS_OtpCmdMsp_eReserved210;

    rc = NEXUS_Security_ReadMSP(&msp_params, &msp_io);
    if(rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - NEXUS_Security_ReadMSP() fails %d", BSTD_FUNCTION, rc));
        g_scm_module.otp_scm_decrypt_enable = 0;
        goto err;
    }

    g_scm_module.otp_scm_decrypt_enable = _ScmModule_EndianSwap(msp_io.mspDataBuf);

    /* SCM verification enabled? */
    BKNI_Memset(&msp_params, 0, sizeof(NEXUS_ReadMspParms));
    BKNI_Memset(&msp_io, 0, sizeof(NEXUS_ReadMspIO));
    msp_params.readMspEnum = NEXUS_OtpCmdMsp_eReserved209;

    rc = NEXUS_Security_ReadMSP(&msp_params, &msp_io);
    if(rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - NEXUS_Security_ReadMSP() fails %d", BSTD_FUNCTION, rc));
        g_scm_module.otp_scm_verify_enable = 0;
        goto err;
    }

    g_scm_module.otp_scm_verify_enable = _ScmModule_EndianSwap(msp_io.mspDataBuf);

    /* SCM secure enabled? */
    BKNI_Memset(&msp_params, 0, sizeof(NEXUS_ReadMspParms));
    BKNI_Memset(&msp_io, 0, sizeof(NEXUS_ReadMspIO));
    msp_params.readMspEnum = NEXUS_OtpCmdMsp_eReserved212;

    rc = NEXUS_Security_ReadMSP(&msp_params, &msp_io);
    if(rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - NEXUS_Security_ReadMSP() fails %d", BSTD_FUNCTION, rc));
        goto err;
    }

    g_scm_module.otp_scm_secure_enable = _ScmModule_EndianSwap(msp_io.mspDataBuf);

    BDBG_MSG(("%s - OTP SCM [SECURE ENABLE: %d, DECRYPT_ENABLE: %d, VERIFY_ENABLE: %d]",
              BSTD_FUNCTION, g_scm_module.otp_scm_secure_enable,
              g_scm_module.otp_scm_decrypt_enable,
              g_scm_module.otp_scm_verify_enable));

    /* TODO: Get type type */
    {
        uint8_t otp_swizzle0a_msp0;
        uint8_t otp_swizzle0a_msp1;

        BKNI_Memset(&msp_params, 0, sizeof(NEXUS_ReadMspParms));
        BKNI_Memset(&msp_io, 0, sizeof(NEXUS_ReadMspIO));
        msp_params.readMspEnum = NEXUS_OtpCmdMsp_eReserved233;

        rc = NEXUS_Security_ReadMSP(&msp_params, &msp_io);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - NEXUS_Security_ReadMSP() fails %d", BSTD_FUNCTION, rc));
            goto err;
        }

        otp_swizzle0a_msp0 = _ScmModule_EndianSwap(msp_io.mspDataBuf);

        BKNI_Memset(&msp_params, 0, sizeof(NEXUS_ReadMspParms));
        BKNI_Memset(&msp_io, 0, sizeof(NEXUS_ReadMspIO));
        msp_params.readMspEnum = NEXUS_OtpCmdMsp_eReserved234;

        rc = NEXUS_Security_ReadMSP(&msp_params, &msp_io);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - NEXUS_Security_ReadMSP() fails %d", BSTD_FUNCTION, rc));
            goto err;
        }

        otp_swizzle0a_msp1 = _ScmModule_EndianSwap(msp_io.mspDataBuf);

        BDBG_MSG(("%s - OTP SCM [MSP0: %d, MSP1: %d]",
                  BSTD_FUNCTION, otp_swizzle0a_msp0,
                  otp_swizzle0a_msp1));

        if((otp_swizzle0a_msp0 == OTP_SWIZZLE0A_MSP0_VALUE_ZS) && (otp_swizzle0a_msp1 == OTP_SWIZZLE0A_MSP1_VALUE_ZS))
        {
            BDBG_MSG(("%s - Chip Type: ZS", BSTD_FUNCTION));
            g_scm_module.otp_chip_type = ScmChipType_eZS;
        }
        else if((otp_swizzle0a_msp0 == OTP_SWIZLLE0A_MSP0_VALUE_ZB) && (otp_swizzle0a_msp1 == OTP_SWIZLLE0A_MSP1_VALUE_ZB))
        {
            BDBG_MSG(("%s - Chip Type: ZB", BSTD_FUNCTION));
            g_scm_module.otp_chip_type = ScmChipType_eZB;
        }
        else
        {
            BDBG_MSG(("%s - Chip Type: Customer specific chip", BSTD_FUNCTION));
            g_scm_module.otp_chip_type = ScmChipType_eCustomer;
        }
    }

err:
    return rc;
}

/* Initialize Scm Module. This is called during platform initialication. */
NEXUS_ModuleHandle NEXUS_ScmModule_Init(const NEXUS_ScmModuleSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_ScmModuleSettings defaultSettings;

    BDBG_ASSERT(!g_NEXUS_scmModule.moduleHandle);

    BKNI_Memset(&g_NEXUS_scmModule, 0, sizeof(g_NEXUS_scmModule));
    g_NEXUS_scmModule.reset = 1;

    /* acquire resources for SCM side */
#ifdef BCHP_PWR_RESOURCE_AVD
    BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD);
#endif
#ifdef BCHP_PWR_RESOURCE_HSM
    BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_HSM);
#endif
#ifdef BCHP_PWR_RESOURCE_VICE1_CLK
    BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_VICE1_CLK);
#endif
#ifdef BCHP_PWR_RESOURCE_VICE0_CLK
    BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_VICE0_CLK);
#endif


    if (!pSettings) {
        NEXUS_ScmModule_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eDefault;
    g_NEXUS_scmModule.moduleHandle = NEXUS_Module_Create("scm", &moduleSettings);
    if (!g_NEXUS_scmModule.moduleHandle) {
        rc = NEXUS_NOT_AVAILABLE;
        goto err_unlocked;
    }

    NEXUS_LockModule();

    BKNI_Memset(&g_scm_module, 0, sizeof(g_scm_module));
    g_scm_module.settings = *(NEXUS_ScmModuleSettings *)pSettings;

    rc = NEXUS_Scm_P_ConfigureAlloc();
    if(rc != NEXUS_SUCCESS) {  goto err; }

    /* configure timer for messcm timestamp and SCM start time */
    rc = NEXUS_ScmModule_P_InitializeTimer();
    if (rc != NEXUS_SUCCESS) { goto err; }
#if 0
    /* allocate VKLs for SCM */
    rc = NEXUS_ScmModule_P_InitializeScmVkl();
    if (rc != NEXUS_SUCCESS) { goto err; }
#endif
    /* Init Scm-Host Communication Interface (nexus_scm.c) */
    rc = NEXUS_Scm_P_VarInit();
    if (rc != NEXUS_SUCCESS) { goto err; }

    /* Get OTP parameters */
    rc = NEXUS_ScmModule_P_GetOtpMspParams();
    if(rc != NEXUS_SUCCESS) {  goto err; }

    /* success */
err:
    NEXUS_UnlockModule();
err_unlocked:
    if (rc != NEXUS_SUCCESS) {
        /* failed to init... */;
       NEXUS_ScmModule_Uninit();
    }

    return g_NEXUS_scmModule.moduleHandle;
}

static NEXUS_Error Nexus_ScmModule_P_Img_Create(
    const char *id,             /* Image Name */
    void **ppContext,           /* [out] Context */
    BIMG_Interface  *pInterface /* [out] Pointer to Image interface */
    )
{
    NEXUS_Error rc;
#if defined(NEXUS_MODE_driver)
    rc = Nexus_Core_P_Img_Create(id, ppContext, pInterface);
#else
    BSTD_UNUSED(id);
    *ppContext = SCM_IMAGE_Context;
    *pInterface = SCM_IMAGE_Interface;
    rc = NEXUS_SUCCESS;
#endif
    return rc;
}

static void Nexus_ScmModule_P_Img_Destroy(
    void *pContext              /* Context returned by previous call */
    )
{
#if defined(NEXUS_MODE_driver)
    Nexus_Core_P_Img_Destroy(pContext);
#else
    BSTD_UNUSED(pContext);
#endif
}

/* Start Scm instance: initialize and start Scm-side boot process
 * Can be called multiple time to restart Scm ( in watchdog for instance )
 * In case of restart, NEXUS_Scm_P_CleanBootVars() must be called first. */
NEXUS_Error NEXUS_ScmModule_P_Start(void)
{
    NEXUS_Error rc;
    NEXUS_ScmMemoryBlock bl = {0, NULL}; /* raw bootloader binary in memory */
    NEXUS_ScmImageHolder blImg     =
        {"Bootloader",   SCM_IMAGE_FirmwareID_eScmNone, NULL, NULL, NULL, NULL, 0};
    /* Image Interface */
    void * img_context = NULL;
    BIMG_Interface img_interface;

    blImg.raw = &bl;

    switch(g_NEXUS_scmModule.instance->channel.type){
    case NEXUS_ScmType_Undefined:
        break;
    case NEXUS_ScmType_Generic:
        blImg.name = "Generic";
        blImg.id = SCM_IMAGE_FirmwareID_eScmGeneric;
        break;
    case NEXUS_ScmType_Arris:
        blImg.name = "Arris";
        blImg.id = SCM_IMAGE_FirmwareID_eScmArris;
        break;
    case NEXUS_ScmType_Cisco:
        blImg.name = "Cisco";
        blImg.id = SCM_IMAGE_FirmwareID_eScmCisco;
        break;
    default:
        rc = NEXUS_INVALID_PARAMETER;
        BDBG_ERR(("%s:%d", __FILE__, __LINE__));
        break;
    }
    /* Initialize IMG interface; used to pull out an image on the file system from the kernel. */
    rc = Nexus_ScmModule_P_Img_Create(NEXUS_CORE_IMG_ID_SAGE, &img_context, &img_interface);
    if (rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - Cannot use IMG interface", BSTD_FUNCTION));
        goto err;
    }

    /* Load SCM bootloader into memory */
    rc = NEXUS_ScmModule_P_Load(&blImg, &img_interface, img_context);
    if(rc != NEXUS_SUCCESS) { goto err; }

    /* Allocate memory for SCM BL and SCM OS/APP second_tier keys */
    {
        void *pMem = NULL;

        /* Memory allocation for SCM BL */
        pMem = NEXUS_Scm_P_Malloc(sizeof(BCMD_SecondTierKey_t));
        if (pMem == NULL) {
            BDBG_ERR(("%s    - Error, allocating buffer (%u bytes)",
                      BSTD_FUNCTION, sizeof(BCMD_SecondTierKey_t)));
            rc = NEXUS_NOT_AVAILABLE;
            goto err;
        }
        g_scm_module.scm_bl_second_tier_key = pMem;
        BKNI_Memset(g_scm_module.scm_bl_second_tier_key, 0, sizeof(BCMD_SecondTierKey_t));
        BDBG_MSG(("g_scm_module.scm_bl_second_tier_key: %p", g_scm_module.scm_bl_second_tier_key));
    }

    /* Initialize communication buffer and pass it to the scm */
    if(NULL == g_NEXUS_scmModule.sendBuf){
        /* must allocate aligned buffer for mapping through TLB */
        g_NEXUS_scmModule.sendBuf = NEXUS_Scm_P_MallocAligned(SCM_HOST_BUF_SIZE, 0x2000);
        if(NULL == g_NEXUS_scmModule.sendBuf){
            BDBG_ERR(("%s:%d", __FILE__, __LINE__));
            rc = NEXUS_NOT_AVAILABLE;
            goto err;
        }
        g_NEXUS_scmModule.recvBuf = (void*)((char*)g_NEXUS_scmModule.sendBuf + SCM_HOST_BUF_SIZE/4);
        BDBG_MSG(("send buf:%08x", g_NEXUS_scmModule.sendBuf));
    }
    NEXUS_ScmModule_P_SetBootParams();
    /* Get start timer */
    BTMR_ReadTimer(g_NEXUS_scmModule.hTimer, &g_NEXUS_scmModule.initTimeUs);
    BDBG_MSG(("%s - Initial timer value: %u", BSTD_FUNCTION, g_NEXUS_scmModule.initTimeUs));

    /* Take SCM out of reset */
    rc = NEXUS_ScmModule_P_Reset(&blImg);
    if(rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - NEXUS_ScmModule_P_Reset() fails %d", BSTD_FUNCTION, rc));
        goto err;
    }

    if(!NEXUS_Scm_P_CheckScmBooted()){
        BDBG_ERR(("%s fails %d", __PRETTY_FUNCTION__, __LINE__));
        goto err;
    }

    /* read SCM OTP epoch */
    NEXUS_ScmModule_P_EpochRead();

    /* success */
    g_NEXUS_scmModule.reset = 0;

    /* end of init is deferred in first NEXUS_Scm_Open() call */

err:
    if (img_context) {
        Nexus_ScmModule_P_Img_Destroy(img_context);
    }

    /* Wipe secure headers */
    if (blImg.header) {
        BKNI_Memset(blImg.header, 0, sizeof(NEXUS_ScmSecureHeader));
    }

    NEXUS_Memory_Free(bl.buf);
    bl.buf = NULL;
    bl.len = 0;

    /* in case of error, g_scm_module.kernel, g_scm_module.scm_bl_second_tier_key and g_scm_module.scm_os_app_second_tier_key
       are freed in NEXUS_ScmModule_Uninit()->NEXUS_Scm_P_CleanBootVars() */

    return rc;
}

/* Clean all temporary variables that are used during boot process. */
/* This event handler is called whenever bootCleanAction registered event is set.
 * The bootCleanAction event is set inside NEXUS_Scm_P_MonitorBoot which is executed
 * inside a dedicated thread.
 * The main idea is that bootCleanAction Event handling is realised in sync
 * i.e. Nexus Scm module is locked in the meanwhile. */
static void NEXUS_Scm_P_CleanBootVars(void *dummy)
{
    BSTD_UNUSED(dummy);
    BDBG_MSG(("%s - cleaning", BSTD_FUNCTION));

    /* g_scm_module.scm_bl_second_tier_key is cleared and freed inside NEXUS_ScmModule_P_Reset() */

    /* wipe associated registers */

}

/* Uninitialize Scm Module and wipe associated resources. */
void NEXUS_ScmModule_Uninit(void)
{
    if (!g_NEXUS_scmModule.moduleHandle) {
        return;
    }
    NEXUS_LockModule();

    NEXUS_Scm_P_CleanBootVars(NULL);

    if(g_scm_module.vklHandle1) {
        NEXUS_Security_FreeVKL(g_scm_module.vklHandle1);
        g_scm_module.vklHandle1 = NULL;
    }

    if(g_scm_module.vklHandle2) {
        NEXUS_Security_FreeVKL(g_scm_module.vklHandle2);
        g_scm_module.vklHandle2 = NULL;
    }

    if (g_NEXUS_scmModule.hTimer) {
        BTMR_DestroyTimer(g_NEXUS_scmModule.hTimer);
        g_NEXUS_scmModule.hTimer = NULL;
    }
    if (NULL != g_NEXUS_scmModule.sendBuf){
        NEXUS_Memory_Free(g_NEXUS_scmModule.sendBuf);
        g_NEXUS_scmModule.sendBuf = NULL;
        g_NEXUS_scmModule.recvBuf = NULL;
    }
    NEXUS_Scm_P_VarCleanup();
    NEXUS_UnlockModule();

    NEXUS_Module_Destroy(g_NEXUS_scmModule.moduleHandle);
    g_NEXUS_scmModule.moduleHandle = NULL;

    /* release resources acquired for SCM-side during init */
#ifdef BCHP_PWR_RESOURCE_VICE1_CLK
    BCHP_PWR_ReleaseResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_VICE1_CLK);
#endif
#ifdef BCHP_PWR_RESOURCE_VICE0_CLK
    BCHP_PWR_ReleaseResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_VICE0_CLK);
#endif
#ifdef BCHP_PWR_RESOURCE_HSM
    BCHP_PWR_ReleaseResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_HSM);
#endif
#ifdef BCHP_PWR_RESOURCE_AVD
    BCHP_PWR_ReleaseResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD);
#endif
}

/* Load a Scm binary (bootloader or kernel) located on the file system into memory
 * NEXUS_ScmImageHolder is a special structure that is used to hold the
 * binary in memory and reference all needed areas (secure header, data, signature)
Raw file in memory:
    +----------+---------------+-------------------------------------------------
    |  header  |   signature   |    binary data (kernel, bootloader)      . . .
    +----------+---------------+-------------------------------------------------
*/
static NEXUS_Error NEXUS_ScmModule_P_Load(
    NEXUS_ScmImageHolder *holder,
    BIMG_Interface *img_interface,
    void *img_context)
{
    void *image = NULL;
    NEXUS_Error rc = NEXUS_SUCCESS;

    uint8_t *raw_ptr = NULL;
    uint32_t raw_remain = 0;

    /* Prepare memory to load binfile */
    {
        uint32_t *size = NULL;

        /* Open file */
        rc = img_interface->open(img_context, &image, holder->id);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - Error opening SCM '%s' file",
                      BSTD_FUNCTION, holder->name));
            goto err;
        }

        /* Get size */
        rc = img_interface->next(image, 0, (const void **)&size, sizeof(uint32_t));
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - Error while reading '%s' file to get size",
                      BSTD_FUNCTION, holder->name));
            goto err;
        }

        /* Allocate buffer to save data */
        { /* PADD CHEAT : for bootloader, allocate at least SCM_BL_LENGTH and pad the rest with zeroes */
            uint32_t alloc_size = *size;
            if (alloc_size < SCM_BL_LENGTH) {
                alloc_size = SCM_BL_LENGTH;
                BDBG_MSG(("%s - adjusting BL size to %d bytes\n", BSTD_FUNCTION, SCM_BL_LENGTH));
            }
            holder->raw->buf = NEXUS_Scm_P_Malloc(alloc_size);
            if(NULL == holder->raw->buf) {
                BDBG_ERR(("%s - Error allocating %u bytes memory for '%s' buffer",
                          BSTD_FUNCTION, *size, holder->name));
                goto err;
            }

            if (*size < alloc_size) {
                BKNI_Memset((void *)((uint8_t *)holder->raw->buf+*size), 0, alloc_size - *size);
            }
            holder->raw->len = *size;
        } /* PADD CHEAT : end */
    }

    /* Load file into memory: read SCM_IMG_BUFFER_SIZE bytes at once */
    {
        uint32_t loop_size = holder->raw->len;
        uint8_t *buffer_ex = holder->raw->buf;
        unsigned chunk = 0;

        while (loop_size) {
            void *data = NULL;
            const uint16_t to_read =
                (loop_size >= SCM_IMG_BUFFER_SIZE) ? (SCM_IMG_BUFFER_SIZE - 1) : loop_size;

            rc = img_interface->next(image, chunk, (const void **)&data, to_read);
            if(rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s - Error while reading '%s' file", BSTD_FUNCTION, holder->name));
                goto err;
            }

            /* BDBG_MSG(("%s - Read %u bytes from file (chunk: %u)", BSTD_FUNCTION, to_read, chunk)); */
            BKNI_Memcpy(buffer_ex, data, to_read);
            loop_size -= to_read;
            buffer_ex += to_read;
            chunk++;
        }
    }

    /* save pointer to walk on buffer then get references on header, signature and data */
    raw_ptr = holder->raw->buf;
    raw_remain = holder->raw->len;

    /* If secure boot is enabled, get pointer to buffer header */
    if(g_scm_module.otp_scm_verify_enable || g_scm_module.otp_scm_decrypt_enable) {

        uint32_t index;

        /* get/check header */
        holder->header = (NEXUS_ScmSecureHeader *)raw_ptr;
        raw_ptr += sizeof(NEXUS_ScmSecureHeader);
        raw_remain -= sizeof(NEXUS_ScmSecureHeader);

        index = _ScmModule_EndianSwap(holder->header->ucHeaderIndex);

        holder->signature = raw_ptr;
        raw_ptr += _SIG_SZ;
        raw_remain -= _SIG_SZ;

        /* if SCMSW, 2 signatures are appended right after the other*/
        if(index == SCM_HEADER_TYPE_OS_APP) {
            raw_ptr += _SIG_SZ;
            raw_remain -= _SIG_SZ;
        }
    }
    /* else:? TBD */

    /* get pointer to data, last block, i.e. whats remaining */
    holder->data = raw_ptr;
    holder->data_len = raw_remain;

    /* Sync physical memory for all areas */
    NEXUS_Memory_FlushCache(holder->raw->buf, holder->raw->len);
    BDBG_MSG(("%s - '%s' Raw@0x%08x,  size=%d", BSTD_FUNCTION,
              holder->name, holder->raw->buf, holder->raw->len));

err:
    /* in case of error, Memory block is freed in NEXUS_ScmModule_Uninit() */
    if (image) {
        img_interface->close(image);
    }
    return rc;
}

/* Write the SCM boot parameters into register */
static void NEXUS_ScmModule_P_SetBootParams(void)
{
    /* status */
    _ScmModule_SetBootParam(LastError, 0xFFFFFFFF);
    _ScmModule_SetBootParam(BootStatus, NEXUS_ScmBoot_eNotStarted);
    /* communication buffers */
    _ScmModule_SetBootParam(RequestBuffer, NEXUS_AddrToOffset(g_NEXUS_scmModule.sendBuf));
    _ScmModule_SetBootParam(RequestBufferSize, SCM_HOST_BUF_SIZE/2);

    /* Misc */
    _ScmModule_SetBootParam(BlVersion, 0);

    return;
}

static NEXUS_Error NEXUS_ScmModule_P_Reset(NEXUS_ScmImageHolder *bl_img)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_ScmSecureHeader *header = bl_img->header;
    NEXUS_SecurityVirtualKeyladderID vkl_id;
    NEXUS_VirtualKeyLadderHandle vkl = NULL;
    NEXUS_KeySlotHandle keyslot = NULL;

#if (KEY_TIER == 2)
    /* if OTP_SCM_VERIFY_ENABLE_BIT: verify SCM boot loader */
    if(g_scm_module.otp_scm_verify_enable) {
        NEXUS_SecurityVerifySecondTierKeySettings second_tier_key_settings;

        BKNI_Memcpy(g_scm_module.scm_bl_second_tier_key, (const void*)&header->second_tier_key,
                    sizeof(BCMD_SecondTierKey_t));
        /* force physical memory to sync */
        NEXUS_Memory_FlushCache(g_scm_module.scm_bl_second_tier_key, sizeof(BCMD_SecondTierKey_t));

        NEXUS_Security_GetDefaultVerifySecondTierKeySettings(&second_tier_key_settings);
        second_tier_key_settings.keyDestination = NEXUS_SecuritySecondTierKeyID_eKey3;
        second_tier_key_settings.firstTierRootKeySource = NEXUS_SecurityFirstTierKeyID_eKey0Prime;
        second_tier_key_settings.secondTierRootKeySource = NEXUS_SecuritySecondTierKeyID_eNone;
        second_tier_key_settings.keyAddress = NEXUS_AddrToOffset(g_scm_module.scm_bl_second_tier_key);
        rc = NEXUS_Security_VerifySecondTierKey(&second_tier_key_settings);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - NEXUS_Security_VerifySecondTierKey() fails %d", BSTD_FUNCTION, rc));
            goto err;
        }
    }
#else
    BKNI_Memcpy(g_scm_module.scm_bl_second_tier_key, (const void*)&header->second_tier_key, sizeof(BCMD_SecondTierKey_t));
    NEXUS_Memory_FlushCache(g_scm_module.scm_bl_second_tier_key, sizeof(BCMD_SecondTierKey_t));
    rc = NEXUS_ScmModule_P_VerifySageKey(NEXUS_AddrToOffset(g_scm_module.scm_bl_second_tier_key));
    if(rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s %d err %d", __FILE__, __LINE__, rc));
        goto err;
    }
#endif
    { /* prepare a Vkl for SCM BL */
        NEXUS_VirtualKeyLadderInfo vkl_info;
        NEXUS_SecurityVKLSettings vkl_settings;

        NEXUS_Security_GetDefaultVKLSettings(&vkl_settings);
        vkl_settings.client = NEXUS_SecurityClientType_eHost;
        vkl = NEXUS_Security_AllocateVKL(&vkl_settings);
        if(vkl == NULL) {
            BDBG_ERR(("%s - NEXUS_Security_AllocateVKL() fails", BSTD_FUNCTION));
            rc = NEXUS_INVALID_PARAMETER;
            goto err;
        }
        NEXUS_Security_GetVKLInfo(vkl, &vkl_info);
        vkl_id = vkl_info.vkl;
        BDBG_MSG(("%s - SCM BL VKL ID: %d", BSTD_FUNCTION, vkl_id));
    }

    /* If OTP_SCM_DECRYPT_ENABLE_BIT: decrypt the SCM BL  */
    if(g_scm_module.otp_scm_decrypt_enable) {
        NEXUS_SecurityAlgorithmSettings algo_settings;
        NEXUS_SecurityKeySlotSettings keyslot_settings;
        NEXUS_SecurityEncryptedSessionKey session_key_settings;
        NEXUS_SecurityEncryptedControlWord cw_settings;

        /* setup keyslot */

        NEXUS_Security_GetDefaultKeySlotSettings(&keyslot_settings);
        keyslot_settings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
        keyslot_settings.keySlotType = NEXUS_SecurityKeySlotType_eAuto;
        keyslot_settings.keySlotSource = NEXUS_SecurityKeySource_eKey5;
        keyslot_settings.client = NEXUS_SecurityClientType_eHost;

        keyslot = NEXUS_Security_AllocateKeySlot(&keyslot_settings);
        if(keyslot == NULL) {
            BDBG_ERR(("%s - NEXUS_Security_AllocateKeySlot() fails %d", BSTD_FUNCTION, rc));
            rc = NEXUS_NOT_AVAILABLE;
            goto err;
        }

        NEXUS_Security_GetDefaultAlgorithmSettings(&algo_settings);
        algo_settings.algorithm = NEXUS_SecurityAlgorithm_eAes;
        algo_settings.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
        algo_settings.terminationMode = NEXUS_SecurityTerminationMode_eClear;
        algo_settings.caVendorID = (header->ucCaVendorId[0] << 8) | (header->ucCaVendorId[1]);
#if (NEXUS_SECURITY_CHIP_SIZE == 40)
        algo_settings.keyDestEntryType = NEXUS_SecurityKeyType_eOddAndEven;
#elif (NEXUS_SECURITY_CHIP_SIZE == 28)
        algo_settings.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
#endif
        algo_settings.operation = NEXUS_SecurityOperation_eDecrypt;
        algo_settings.dest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
        algo_settings.keySource = NEXUS_SecurityKeySource_eKey5;

        rc = NEXUS_Security_ConfigAlgorithm(keyslot, &algo_settings);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - NEXUS_Security_ConfigAlgorithm() fails %d", BSTD_FUNCTION, rc));
            goto err;
        }

        /* generate route key */

        NEXUS_Security_GetDefaultSessionKeySettings(&session_key_settings);
        session_key_settings.client = NEXUS_SecurityClientType_eHost;
        session_key_settings.keyladderType = NEXUS_SecurityKeyladderType_eAes128;
        if((header->ucCustKeyVar) || (header->ucKeyVarHi) || (header->ucKeyVarlo)) {
            session_key_settings.rootKeySrc = NEXUS_SecurityRootKeySrc_eCuskey;
            session_key_settings.cusKeySwizzle0aEnable = true;
            session_key_settings.cusKeySwizzle0aType = header->ucSwizzle0aVariant;
            session_key_settings.cusKeySwizzle0aVer = header->ucSwizzle0aVersion;
        }
        else {
            session_key_settings.rootKeySrc = NEXUS_SecurityRootKeySrc_eOtpKeyA;
            session_key_settings.bASKMMode = true;
        }

        session_key_settings.swizzleType = NEXUS_SecuritySwizzleType_eSwizzle0;
        session_key_settings.operation = NEXUS_SecurityOperation_eDecrypt;
        session_key_settings.operationKey2 = NEXUS_SecurityOperation_eEncrypt;
        session_key_settings.bkeyGenBlocked = false;
#if (NEXUS_SECURITY_CHIP_SIZE == 40)
        session_key_settings.keyEntryType = NEXUS_SecurityKeyType_eOdd;
#elif (NEXUS_SECURITY_CHIP_SIZE == 28)
        session_key_settings.keyEntryType = NEXUS_SecurityKeyType_eClear;
#endif
        session_key_settings.custSubMode = NEXUS_SecurityCustomerSubMode_eSageBlDecrypt;
        session_key_settings.virtualKeyLadderID = vkl_id;
        BKNI_Memcpy(session_key_settings.keyData, header->ucProcIn1, 16);

        session_key_settings.cusKeyL = header->ucCustKeyVar;
        session_key_settings.cusKeyH = header->ucCustKeyVar;
        session_key_settings.cusKeyVarL = header->ucKeyVarlo;
        session_key_settings.cusKeyVarH = header->ucKeyVarHi;

        session_key_settings.sage.askmConfigurationEnable = true; /* true for ASKM and for swizzle0a */
        session_key_settings.sage.otpId = NEXUS_SecurityOtpId_eOneVal;
        session_key_settings.sage.caVendorId = (header->ucCaVendorId[1] << 8) | (header->ucCaVendorId[0]);
        session_key_settings.sage.moduleID = header->ucModuleId;

#if (NEXUS_SECURITY_CHIP_SIZE == 28)
        session_key_settings.globalKeyOwnerId = 0; /* in HSM: MSP0: 0, MSP1: 1, Reserved: 2, Use1: 3 */
        session_key_settings.askmGlobalKeyIndex = 0; /* TODO: verify. Global Key Index in document */
        session_key_settings.globalKeyVersion = 0;

        session_key_settings.keyTweakOp = NEXUS_SecurityKeyTweak_eNoTweak;
#endif
        session_key_settings.bRouteKey = false;
        rc = NEXUS_Security_GenerateSessionKey(keyslot, &session_key_settings);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - NEXUS_Security_GenerateSessionKey() fails %d", BSTD_FUNCTION, rc));
            goto err;
        }

        NEXUS_Security_GetDefaultControlWordSettings(&cw_settings);
        cw_settings.client = NEXUS_SecurityClientType_eHost;
        cw_settings.keyladderType = NEXUS_SecurityKeyladderType_eAes128;
        cw_settings.virtualKeyLadderID = vkl_id;
        cw_settings.keySize = 16;
        BKNI_Memcpy(cw_settings.keyData, header->ucProcIn2, 16);
        cw_settings.operation = NEXUS_SecurityOperation_eDecrypt;
        cw_settings.keylayer = NEXUS_SecurityKeyLayer_eKey4;
        cw_settings.bRouteKey = false;
#if (NEXUS_SECURITY_CHIP_SIZE == 28)
        cw_settings.keyEntryType = NEXUS_SecurityKeyType_eClear;
        cw_settings.dest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
#endif
        rc = NEXUS_Security_GenerateControlWord(keyslot, &cw_settings);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - NEXUS_Security_GenerateControlWord() fails %d", BSTD_FUNCTION, rc));
            goto err;
        }

        NEXUS_Security_GetDefaultControlWordSettings(&cw_settings);
        cw_settings.client = NEXUS_SecurityClientType_eHost;
        cw_settings.keyladderType = NEXUS_SecurityKeyladderType_eAes128;
        cw_settings.virtualKeyLadderID = vkl_id;
        cw_settings.keySize = 16;
        BKNI_Memcpy(cw_settings.keyData, header->ucProcIn3, 16);
        cw_settings.operation = NEXUS_SecurityOperation_eDecrypt;
        cw_settings.keylayer = NEXUS_SecurityKeyLayer_eKey5;
        cw_settings.bRouteKey = false;
#if (NEXUS_SECURITY_CHIP_SIZE == 28)
        cw_settings.keyEntryType = NEXUS_SecurityKeyType_eClear;
        cw_settings.dest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
#endif
        rc = NEXUS_Security_GenerateKey5(keyslot, &cw_settings);
        if(rc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s - NEXUS_Security_GenerateKey5() fails %d", BSTD_FUNCTION, rc));
            goto err;
        }
    }

    { /* setup and verify region */
        NEXUS_ScmCodeRegionSettings region_settings;
        /*unsigned int region_id = 0;*/

        BKNI_Memset( &region_settings, 0, sizeof(region_settings) );

        /* region_settings.regionID = NEXUS_SecurityRegverRegionID_eScpuFsbl; */
        region_settings.codeStartPhyAddress = NEXUS_AddrToOffset(bl_img->data);
        region_settings.codeSize = SCM_BL_LENGTH;
        if (bl_img->signature) {
            region_settings.sigStartPhyAddress = NEXUS_AddrToOffset(bl_img->signature);
        }
        else {
            region_settings.sigStartPhyAddress = 0;
        }
        /* region_settings.sigSize = NEXUS_SecurityRegverSignatureSize_e256Bytes; */
        /* region_settings.cpuType = NEXUS_SecurityRegverCPUType_eScpu;*/

        if(g_scm_module.otp_scm_verify_enable || g_scm_module.otp_scm_decrypt_enable) {
            /* region_settings.bgCheck = NEXUS_SecurityRegverBGChecker_eDisable;*/
            /* region_settings.instrCheck = NEXUS_SecurityRegverInstrChecker_eDisable; */

            /* CE- TODO: Verify which region_settings to set in case only otp_scm_verify_enable or only otp_scm_decrypt_enable is set */
            if(g_scm_module.otp_scm_verify_enable) {
                /* region_settings.codeRule = 0;*/
                region_settings.epoch = 0;
                region_settings.epochMask = 0;
                region_settings.epochSel = 0;
                region_settings.marketID =  (header->second_tier_key.ucMarketID[0]<<0) |
                                            (header->second_tier_key.ucMarketID[1]<<8) |
                                            (header->second_tier_key.ucMarketID[2]<<16) |
                                            (header->second_tier_key.ucMarketID[3]<<24);
                region_settings.marketIDMask =  (header->second_tier_key.ucMarketIDMask[0]<<0) |
                                                (header->second_tier_key.ucMarketIDMask[1]<<8) |
                                                (header->second_tier_key.ucMarketIDMask[2]<<16) |
                                                (header->second_tier_key.ucMarketIDMask[3]<<24);

                /* region_settings.keyID = NEXUS_SecurityRegverSigningKeyID_e3;*/
                /* region_settings.sigVersion = 0x01; */
                /* region_settings.sigType = 0x04; */
                BDBG_MSG(("%s - SCM BL length: %d, EPOCH:0x%x, EPOCH mask: 0x%x, MarketID: 0x%08x, MarketIDMask: 0x%08x\n", BSTD_FUNCTION,
                          SCM_BL_LENGTH, region_settings.epoch, region_settings.epochMask, region_settings.marketID, region_settings.marketIDMask));
                region_settings.SCMVersion = ((unsigned int)header->ucReserved1[1] << 8) | (unsigned int)header->ucReserved1[2];
                BDBG_MSG(("SCM VERSION %08x", region_settings.SCMVersion));
                switch(region_settings.SCMVersion & 0xff00){
                case 0x200:
                    if((region_settings.SCMVersion & 0xff) > 128){
                        /* arris version too large */
                        BDBG_ERR(("%s:%d %08x", __FILE__, __LINE__, region_settings.SCMVersion));
                        goto err;
                    }
                    break;
                case 0x300:
                    if((region_settings.SCMVersion & 0xff) > 32){
                        /* cisco version too large */
                        BDBG_ERR(("%s:%d %08x", __FILE__, __LINE__, region_settings.SCMVersion));
                        goto err;
                    }
                    break;
                default:
                    /* for all other cases we do not need to do anything */
                    break;
                }
                /* save version for future reporting */
                g_NEXUS_scmModule.instance->channel.version = region_settings.SCMVersion & 0xff;
            }
            if(g_scm_module.otp_scm_decrypt_enable) {
                region_settings.keyLadder = vkl_id;
                region_settings.keyLayer = NEXUS_SecurityKeySource_eKey5;
            }

#if 0
            /* mimic NEXUS_Security_VerifyRegion: define -> enable -> isVerified */
            rc = NEXUS_Security_DefineRegion(&region_settings, &region_id);
            if (rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s - Define region 0x%x failed (%d)", BSTD_FUNCTION, region_id, rc));
                rc = NEXUS_SUCCESS;
                goto err;
            }

            rc = NEXUS_Security_EnableRegionVerification();
            if (rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s - Enable region 0x%x failed (%d)", BSTD_FUNCTION, region_id, rc));
                goto err;
            }

            /* wait until region is verified */
            {
                bool verified;
                do {
                    rc = NEXUS_Security_IsRegionVerified (region_id, &verified);
                    if (rc != NEXUS_SUCCESS) {
                        BDBG_ERR(("%s - Verification of region 0x%x failed (%d)",
                                  BSTD_FUNCTION, region_id, rc));
                        goto err;
                    }
                    BKNI_Sleep (5);
                } while (!verified);
            }
#else
            rc = NEXUS_ScmModule_P_VeryfyScmRegion(&region_settings);
            if (rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s:%d (%d)", __FILE__, __LINE__, rc));
                goto err;
            }

#endif
        }
        else {
            NEXUS_SecurityRegionConfiguration regionConfig;

            NEXUS_Security_RegionGetDefaultConfig_priv( NEXUS_SecurityRegverRegionID_eScpuFsbl, &regionConfig );
            regionConfig.signature.p = bl_img->signature;
            regionConfig.signature.size = 256;
            regionConfig.rsaKeyIndex = 3;
            regionConfig.scmVersion = ((unsigned int)header->ucReserved1[1] << 8) | (unsigned int)header->ucReserved1[2];
            regionConfig.keyLadderId = vkl_id;
            regionConfig.keyLadderLayer = NEXUS_SecurityKeySource_eKey5;

            rc = NEXUS_Security_RegionConfig_priv( NEXUS_SecurityRegverRegionID_eScpuFsbl, &regionConfig );
            if(rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s - NEXUS_Security_RegionConfig_priv() fails (0x%X)", BSTD_FUNCTION, rc));
                goto err;
            }

            rc = NEXUS_Security_RegionVerifyEnable_priv( NEXUS_SecurityRegverRegionID_eScpuFsbl,
                                                         NEXUS_AddrToOffset( bl_img->data ),
                                                         SCM_BL_LENGTH );
            if(rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s - NEXUS_Security_RegionVerifyEnable_priv() fails (0x%X)", BSTD_FUNCTION, rc));
                goto err;
            }
        }

        BDBG_MSG(("%s: SCM reset completed successfully", BSTD_FUNCTION));
    }
err:
    if (g_scm_module.scm_bl_second_tier_key) {
        BKNI_Memset(g_scm_module.scm_bl_second_tier_key, 0x0, sizeof(BCMD_SecondTierKey_t));
        NEXUS_Memory_Free(g_scm_module.scm_bl_second_tier_key);
        g_scm_module.scm_bl_second_tier_key = NULL;
    }

    if (keyslot) {
        NEXUS_Security_FreeKeySlot(keyslot);
    }
    if (vkl) {
        NEXUS_Security_FreeVKL(vkl);
    }
    return rc;
}

#if 0
/* Initialize VKL that will be use SCM-side.
 * A mask is given to SCM using globalram, see NEXUS_ScmModule_P_SetBootParams */
static NEXUS_Error NEXUS_ScmModule_P_InitializeScmVkl(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SecurityVKLSettings settings;
    NEXUS_VirtualKeyLadderInfo vklInfo1;
    NEXUS_VirtualKeyLadderInfo vklInfo2;

    NEXUS_Security_GetDefaultVKLSettings(&settings);
    settings.client = NEXUS_SecurityClientType_eSage;
    g_scm_module.vklHandle1 = NEXUS_Security_AllocateVKL(&settings);
    if(g_scm_module.vklHandle1 == NULL) {
        BDBG_ERR(("%s - NEXUS_Security_AllocateVKL() fails for VKL 1", BSTD_FUNCTION));
        rc = NEXUS_INVALID_PARAMETER;
        goto err;
    }
    NEXUS_Security_GetVKLInfo(g_scm_module.vklHandle1, &vklInfo1);

    g_scm_module.vklHandle2 = NEXUS_Security_AllocateVKL(&settings);
    if(g_scm_module.vklHandle2 == NULL) {
        BDBG_ERR(("%s - NEXUS_Security_AllocateVKL() fails for VKL 2", BSTD_FUNCTION));
        rc = NEXUS_INVALID_PARAMETER;
        goto err;
    }
    NEXUS_Security_GetVKLInfo(g_scm_module.vklHandle2, &vklInfo2);

    g_scm_module.scmReservedVklMask = ((1 << vklInfo1.vkl) | (1 << vklInfo2.vkl));
    BDBG_MSG(("%s - SCM VKLs %d, %d --> Mask: %x", BSTD_FUNCTION,
              vklInfo1.vkl, vklInfo2.vkl, g_scm_module.scmReservedVklMask));
err:
    return rc;
}
#endif

#include "bchp_scpu_top_ctrl.h"

#define SCM_TIMEOUT 80

int NEXUS_Scm_P_CheckScmBooted(void)
{
    uint32_t val;
    uint32_t timeout;

    if (g_NEXUS_scmModule.booted) {
        goto ExitFunc;
    }

    for(timeout = 0; timeout < SCM_TIMEOUT; timeout++){
        val = BREG_Read32(g_pCoreHandles->reg, BCHP_SCPU_TOP_CTRL_SCPU_HOST_IRDY);
        /* check if sage is ready */
        if(0 != (val & BCHP_SCPU_TOP_CTRL_SCPU_HOST_IRDY_SCPU_HOST_IRDY_MASK)){
            break;
        }
        BKNI_Sleep(5);
    }
    if(SCM_TIMEOUT == timeout){
        BDBG_ERR(("%s %d IRDY1 %d", __FILE__, __LINE__, val));
        goto ExitFunc;
    }
    g_NEXUS_scmModule.booted = 1;

ExitFunc:
    return g_NEXUS_scmModule.booted;
}

#define BCMD_DTA_MODE_RESET_SAGE_PATTERN 0xa5
#define BCMD_DTA_MODE_NORESET_SAGE_PATTERN 0x5a
#include "nexus_security_rawcommand.h"

NEXUS_Error NEXUS_Scm_P_DtaModeSelect(uint32_t mode)
{
    NEXUS_Error nerr;
    unsigned int output_size;
    uint32_t cmd_in[20];
    uint32_t cmd_out[20];

    cmd_in[0] = 0x10;
    cmd_in[1] = 0x11;
    cmd_in[2] = 0xABCDEF00;
    cmd_in[3] = 0xd255aa2d;
    cmd_in[4] = 0x789A000c;
    cmd_in[5] = 1;
    cmd_in[6] = mode; /* 1 generic, 2 moto, 3 cisco */
    cmd_in[7] = BCMD_DTA_MODE_RESET_SAGE_PATTERN;

    cmd_in[2] |= 0xabcdef00;
    cmd_in[3] = (cmd_in[3] & 0xff) | ((~(cmd_in[3] & 0xff)) << 24) | 0x55aa00;
    cmd_in[4] |= 0x789A0000;

    BKNI_Memset(cmd_out, 0, 20); /* clear only 1/4 of the buf */
    nerr = NEXUS_Security_SubmitRawCommand(cmd_in, 8, cmd_out, 20, &output_size);
    if(NEXUS_SUCCESS != nerr){
        BDBG_ERR(("%s:%d", __FILE__, __LINE__));
        goto ExitFunc;
    }
    if(0 != cmd_out[5]){
        BDBG_ERR(("%s:%d err %08x", __FILE__, __LINE__, cmd_out[5]));
    }
    nerr = cmd_out[5];
ExitFunc:
    return nerr;

}

#define CMODE_PADDING_MASK	0x000000FF
#define CMODE_PADDING		0xABCDEF00
#define CTAGID_PADDING_MASK	0x000000FF
#define CTAGID_PADDING		0x0055AA00
#define CTAGID_SHFT			24
#define CPLEN_PADDING_MASK	0x0000FFFF
#define CPLEN_PADDING		0x789A0000

void NEXUS_ScmModule_P_PadCommand(uint32_t * cmd)
{
    cmd[2] = (cmd[2] & CMODE_PADDING_MASK) | CMODE_PADDING;
    cmd[3] = (cmd[3] & CTAGID_PADDING_MASK) | CTAGID_PADDING | (~(cmd[3]) << CTAGID_SHFT);
    cmd[4] = (cmd[4] & CPLEN_PADDING_MASK) | CPLEN_PADDING;
    return;
}

#define SAGE_KEY_IDX 3

#if (KEY_TIER == 3)
#include "BCM74371CM_SAGE_key1.c"

NEXUS_Error NEXUS_ScmModule_P_VerifySageKey(uint32_t key_offset)
{
    NEXUS_Error nerr;
    unsigned int output_size;
    uint32_t cmd_in[20];
    uint32_t cmd_out[20];


    NEXUS_MemoryAllocationSettings msettings;
    void * mem;

    NEXUS_Memory_GetDefaultAllocationSettings(&msettings);
    msettings.alignment = 32;
    mem = NEXUS_Scm_P_MallocAligned(sizeof(BCM74371CM_SAGE_key1), 32);
    if (NULL == mem) {
        BDBG_ERR(("%s:%d", __FILE__, __LINE__));
        goto ExitFunc;
    }

    cmd_in[0] = 0x10;
    cmd_in[1] = 0x33;
    cmd_in[2] = 0;
    cmd_in[3] = 0x36;
    cmd_in[4] = 12;
    cmd_in[5] = SAGE_KEY_IDX;
    cmd_in[6] = 0x000;
    cmd_in[7] = NEXUS_AddrToOffset(mem);
    NEXUS_ScmModule_P_PadCommand(cmd_in);
    BKNI_Memcpy(mem, BCM74371CM_SAGE_key1, sizeof(BCM74371CM_SAGE_key1));
    NEXUS_Memory_FlushCache(mem, sizeof(BCM74371CM_SAGE_key1));
    BKNI_Memset(cmd_out, 0, 20); /* clear only 1/4 of the buf */
    nerr = NEXUS_Security_SubmitRawCommand(cmd_in, 8, cmd_out, 20, &output_size);
    NEXUS_Memory_Free(mem);

    if(NEXUS_SUCCESS != nerr){
        BDBG_ERR(("%s:%d", __FILE__, __LINE__));
        goto ExitFunc;
    }
    if(0 != cmd_out[5]){
        BDBG_ERR(("%s:%d err %08x", __FILE__, __LINE__, cmd_out[5]));
        goto ExitFunc;
    }

    cmd_in[0] = 0x10;
    cmd_in[1] = 0x33;
    cmd_in[2] = 0;
    cmd_in[3] = 0x36;
    cmd_in[4] = 12;
    cmd_in[5] = SAGE_KEY_IDX;
    cmd_in[6] = (1 << 8) | (SAGE_KEY_IDX);;
    cmd_in[7] = key_offset;
    BKNI_Memset(cmd_out, 0, 20);
    NEXUS_ScmModule_P_PadCommand(cmd_in);
    nerr = NEXUS_Security_SubmitRawCommand(cmd_in, 8, cmd_out, 20, &output_size);
    if(NEXUS_SUCCESS != nerr){
        BDBG_ERR(("%s:%d", __FILE__, __LINE__));
        goto ExitFunc;
    }
    if(0 != cmd_out[5]){
        BDBG_ERR(("%s:%d err %08x", __FILE__, __LINE__, cmd_out[5]));
    }
    nerr = cmd_out[5];
ExitFunc:
    return nerr;
}
#endif

NEXUS_Error NEXUS_ScmModule_P_VeryfyScmRegion(NEXUS_ScmCodeRegionSettings * rs)
{
    NEXUS_Error nerr;
    unsigned int output_size;
    uint32_t cmd_in[40];
    uint32_t cmd_out[40];


    /* define region */
    cmd_in[0] = 0x10;
    cmd_in[1] = 0x44;
    cmd_in[2] = 0;
    cmd_in[3] = 0x9;
    cmd_in[4] = 84;             /* size for setup region command */
    cmd_in[5] = 0x01;           /* setup region */
    cmd_in[6] = 0x18;           /* sage fsbl */
    cmd_in[7] = 0;
    cmd_in[8] = rs->codeStartPhyAddress;
    cmd_in[9] = 0;
    cmd_in[10] = rs->codeStartPhyAddress + rs->codeSize - 1; /* end address */
    cmd_in[11] = 0;
    cmd_in[12] = rs->sigStartPhyAddress;
    cmd_in[13] = 0;
    cmd_in[14] = cmd_in[12] + 0xff;  /* signature end */
    cmd_in[15] = 0x01;              /* interval check must be 10? */
    cmd_in[16] = ((rs->keyLadder & 0xff) << 24) | ((rs->keyLayer & 0xff) << 16) | SAGE_KEY_IDX;
    cmd_in[17] = 0x0700;            /* rules */
    cmd_in[18] = rs->marketID;
    cmd_in[19] = rs->marketIDMask;
    cmd_in[20] = (rs->epochMask << 8) | rs->epoch; /* epoch mask/epoch */
    cmd_in[21] = (0xd << 8) | 0x0c;          /* instruction checker */
    cmd_in[22] = 0;                         /* reserved */
    cmd_in[23] = rs->SCMVersion;
    cmd_in[24] = 0;
    cmd_in[25] = 0;

    BKNI_Memset(cmd_out, 0, 20);
    NEXUS_ScmModule_P_PadCommand(cmd_in);
    nerr = NEXUS_Security_SubmitRawCommand(cmd_in, 26, cmd_out, 40, &output_size);
    if(NEXUS_SUCCESS != nerr){
        BDBG_ERR(("%s:%d", __FILE__, __LINE__));
        goto ExitFunc;
    }
    if(0 != cmd_out[5]){
        BDBG_ERR(("%s:%d err %08x", __FILE__, __LINE__, cmd_out[5]));
        nerr = cmd_out[5];
        goto ExitDisable;
    }

    /* enable region */
    cmd_in[0] = 0x10;
    cmd_in[1] = 0x44;
    cmd_in[2] = 0;
    cmd_in[3] = 0x9;
    cmd_in[4] = 4;              /* size for enable region command */
    cmd_in[5] = 0x07;           /* enable region */

    BKNI_Memset(cmd_out, 0, 20);
    NEXUS_ScmModule_P_PadCommand(cmd_in);

    nerr = NEXUS_Security_SubmitRawCommand(cmd_in, 6, cmd_out, 40, &output_size);
    if(NEXUS_SUCCESS != nerr){
        BDBG_ERR(("%s:%d", __FILE__, __LINE__));
        goto ExitFunc;
    }
    if(0 != cmd_out[5]){
        BDBG_ERR(("%s:%d err %08x", __FILE__, __LINE__, cmd_out[5]));
        nerr = cmd_out[5];
        goto ExitFunc;
    }

    /* check if region is verified */
    do {
        cmd_in[0] = 0x10;
        cmd_in[1] = 0x44;
        cmd_in[2] = 0;
        cmd_in[3] = 0x9;
        cmd_in[4] = 8;              /* size of command */
        cmd_in[5] = 0x04;           /* check region */
        cmd_in[6] = 0x18;           /* fsbl region */
        BKNI_Memset(cmd_out, 0, 20);
        NEXUS_ScmModule_P_PadCommand(cmd_in);
        nerr = NEXUS_Security_SubmitRawCommand(cmd_in, 7, cmd_out, 40, &output_size);
        if(NEXUS_SUCCESS != nerr){
            BDBG_ERR(("%s:%d", __FILE__, __LINE__));
            break;
        }
        nerr = NEXUS_UNKNOWN;
        switch(cmd_out[5]){
        case 0:
            nerr = NEXUS_SUCCESS;
            BDBG_MSG(("verified"));
            goto ExitFunc;
        case 0xb3:
            BKNI_Sleep(20);
            break;
        case 0xbb:
            BDBG_ERR(("%s:%d failed", __FILE__, __LINE__));
            break;
        case 0xb6:
            BDBG_MSG(("missing enable command"));
            break;
        case 0xb7:
            BDBG_MSG(("not defined"));
            goto ExitFunc;
        case 0xbd:
            BDBG_MSG(("scm type mismatch"));
            break;
        default:
            break;
        }
    }while(0xb3 == cmd_out[5]);

ExitDisable:
    {
        NEXUS_Error nerr2;
        cmd_in[0] = 0x10;
        cmd_in[1] = 0x44;
        cmd_in[2] = 0;
        cmd_in[3] = 0x9;
        cmd_in[4] = 8;              /* size for disable region command */
        cmd_in[5] = 0x00;           /* disable region */
        cmd_in[6] = 0x18;           /* fsbl region */
        BKNI_Memset(cmd_out, 0, 20);
        NEXUS_ScmModule_P_PadCommand(cmd_in);
        nerr2 = NEXUS_Security_SubmitRawCommand(cmd_in, 7, cmd_out, 40, &output_size);
        if(NEXUS_SUCCESS != nerr2){
            BDBG_ERR(("%s:%d err %08x", __FILE__, __LINE__, nerr));
            goto ExitFunc;
        }
        if(0 != cmd_out[5]){
            BDBG_ERR(("%s:%d err %08x", __FILE__, __LINE__, cmd_out[5]));
            nerr2 = cmd_out[5];
            goto ExitFunc;
        }
    }
ExitFunc:
    return nerr;
}

/* Generate keyX template for CW generation */
static uint32_t query_VKL[] =
{
    0x00000010,
    0x00000011,
    0xabcdef00,
    0xf855aa07,
    0x789a00b0,
    0x00000102,
    0x01011801,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,                 /* 10 */
    0x00000000,
    0x00000000,
    0x00000004,
    0x00000003,
    0x00000000,
    0x00000000,
    0x00000000,
    0x33445511,
    0x00112233,
    0x11112222,                 /* 20 */
    0x12345678,
    0x98765432,
    0x09234556,
    0x00000001,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,                 /* 30 */
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000001,
    0x00000001,
    0x00000214,
    0x00000000,                 /* 40 */
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,                 /* 48 */
};

NEXUS_Error NEXUS_ScmModule_P_DumpVKL(void)
{
    uint32_t cmd_out[20];
    NEXUS_Error nerr;
    unsigned int output_size, i;

    do{
        BKNI_Memset(cmd_out, 0, 20);
        NEXUS_ScmModule_P_PadCommand(query_VKL);

        nerr = NEXUS_Security_SubmitRawCommand(query_VKL, 48, cmd_out, 20, &output_size);
        if(NEXUS_SUCCESS != nerr){
            BDBG_ERR(("%s:%d", __FILE__, __LINE__));
            goto ExitFunc;
        }
        query_VKL[13]++;
        query_VKL[13] &= 3;
    }while(cmd_out[5] == 0xe3);
    for(i = 0; i < 14; i++){
        BDBG_ERR(("out[%d]: %08x", i, cmd_out[i]));
    }
ExitFunc:
    return nerr;
}


NEXUS_Error NEXUS_ScmModule_P_EpochRead(void)
{
    uint32_t cmd_in[40];
    uint32_t cmd_out[20];
    NEXUS_Error nerr;
    unsigned int output_size;
    uint32_t *pEpoch;
    uint32_t bits, count, i;

    pEpoch = &(g_NEXUS_scmModule.instance->channel.otp_version[0]);

    switch(g_NEXUS_scmModule.instance->channel.type){
    case NEXUS_ScmType_Arris:
        bits = 168;
        count = 4;
        break;
    case NEXUS_ScmType_Cisco:
        bits = 167; count = 1;
        break;
    default:
        bits = 0;
        count = 0;
        *pEpoch = 0;
        break;
    }
    nerr = NEXUS_UNKNOWN;
    for( i = 0 ; i < count; i++){
        cmd_in[0] = 0x10;
        cmd_in[1] = 0x44;
        cmd_in[2] = 0x0;
        cmd_in[3] = 0x1b;
        cmd_in[4] = 4;
        cmd_in[5] = bits;;

        BKNI_Memset(cmd_out, 0, 20);
        NEXUS_ScmModule_P_PadCommand(cmd_in);

        nerr = NEXUS_Security_SubmitRawCommand(cmd_in, 6, cmd_out, 20, &output_size);
        if(NEXUS_SUCCESS != nerr){
            BDBG_ERR(("%s:%d", __FILE__, __LINE__));
            goto ExitFunc;
        }

        BDBG_MSG(("value:%08x lock:%08x", cmd_out[6], cmd_out[7]));
        pEpoch[i] = cmd_out[6];
        bits++;
    }
ExitFunc:
    return nerr;
}
