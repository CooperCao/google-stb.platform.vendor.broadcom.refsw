/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
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
 *****************************************************************************/

#include "nexus_security_module.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"

#include "bhsm.h"
#include "bchp.h"
#include "bhsm_misc.h"
#include "bsp_s_otp_common.h"
#include "bhsm_otpmsp.h"
#include "bhsm_keyladder_enc.h"


BDBG_MODULE(nexus_bsp_config);

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)

typedef struct pciESignatureVerificationKeyConfig_t {

    NEXUS_SecurityVirtualKeyladderID vkl;

    unsigned globalKeyIndex;                /* Identify Global Key by index*/
    BHSM_OwnerIDSelect_e globalKeyOwnerId;
    BCMD_STBOwnerID_e stbOwnerId;
    BCMD_ASKM_MaskKeySel_e maskKeySelect;
    uint32_t caVendorId;                    /* Ca Vendor separation on root key. */

}pciESignatureVerificationKeyConfig_t;

static BERR_Code GeneratePciESignatureVerificationKey( pciESignatureVerificationKeyConfig_t *pConfig );

static BERR_Code allocateVKL(
    const NEXUS_SecurityPciEMaxWindowSizeSettings_t *pConfig, BHSM_AllocateVKLIO_t *vkl);

#endif


NEXUS_Error NEXUS_Security_RaveControl(
    const NEXUS_SecurityRaveCtrlSettings *pSettings
    )
{
    BERR_Code                   rc;
    BHSM_Handle                 hHsm;
    BHSM_SetMiscBitsIO_t        setMiscBitsIO;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if (!hHsm)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /*  formulate the request structure */
    setMiscBitsIO.setMiscBitsSubCmd     = BCMD_SetMiscBitsSubCmd_eRaveBits;
    setMiscBitsIO.bDisableClear         = pSettings->bDisableClear;
    setMiscBitsIO.bRAVEEncryptionBypass = pSettings->bRAVEEncryptionBypass;
    setMiscBitsIO.bEnableReadDMem       = pSettings->bEnableReadDMem;
    setMiscBitsIO.bEnableReadIMem       = pSettings->bEnableReadIMem;
    setMiscBitsIO.bEnableWriteIMem      = pSettings->bEnableWriteIMem;

    rc = BHSM_SetMiscBits(hHsm, &setMiscBitsIO);
    if (rc != 0)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Security_DMABypassControl(
    const NEXUS_SecurityDMABypassSettings* pSettings
    )
{
    BERR_Code                   rc;
    BHSM_Handle                 hHsm;
    BHSM_SetMiscBitsIO_t        setMiscBitsIO;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if (!hHsm)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* formulate the request structure */
    setMiscBitsIO.setMiscBitsSubCmd         = BCMD_SetMiscBitsSubCmd_eReserved1;
    setMiscBitsIO.bDMAM2MPacketBypassEnable = pSettings->bDMAM2MPacketBypassEnable;
    setMiscBitsIO.bDMAXPTPacketBypassEnable = pSettings->bDMAXPTPacketBypassEnable;
    setMiscBitsIO.bNMAPacketBypassEnable    = pSettings->bNMAPacketBypassEnable;

    rc = BHSM_SetMiscBits(hHsm, &setMiscBitsIO);
    if (rc != 0)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Security_M2MEndianSwapControl(
    const NEXUS_SecurityM2MEndianSwapSettings* pSettings
    )
{
    BERR_Code                   rc;
    BHSM_Handle                 hHsm;
    BHSM_SetMiscBitsIO_t        setMiscBitsIO;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if (!hHsm)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }


    /* formulate the request structure */
    setMiscBitsIO.setMiscBitsSubCmd         = BCMD_SetMiscBitsSubCmd_eM2MEndianSwapBits;
    setMiscBitsIO.bCBCMACDataInSwapEnable   = pSettings->bCBCMACDataInSwapEnable;
    setMiscBitsIO.bCBCMACDataOutSwapEnable  = pSettings->bCBCMACDataOutSwapEnable;
    setMiscBitsIO.bCBCMACKeySwapEnable      = pSettings->bCBCMACKeySwapEnable;

    rc = BHSM_SetMiscBits(hHsm, &setMiscBitsIO);
    if (rc != 0)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Security_ForceSCControl(
    const NEXUS_SecurityForceSCSettings *pSettings
    )

{
    BERR_Code                   rc;
    BHSM_Handle                 hHsm;
    BHSM_SetMiscBitsIO_t        setMiscBitsIO;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if (!hHsm)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* formulate the request structure */
    setMiscBitsIO.setMiscBitsSubCmd         = BCMD_SetMiscBitsSubCmd_eForceSCBit;
    setMiscBitsIO.bBandSelect_PB            = pSettings->bBandSelect_PB;
    setMiscBitsIO.bandNumber                = pSettings->bandNumber;
    setMiscBitsIO.bSCForceEnAll             = pSettings->bSCForceEnAll;
    setMiscBitsIO.bSCForceEnNZ              = pSettings->bSCForceEnNZ;
    setMiscBitsIO.SCForceEnValue            = pSettings->SCForceEnValue;

    rc = BHSM_SetMiscBits(hHsm, &setMiscBitsIO);
    if (rc != 0)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Security_XPTM2MStatusQuery(
    NEXUS_SecurityXPTM2MStatusOutput *pStatusQuery
    )


{
    BERR_Code            rc = NEXUS_SUCCESS;
    BHSM_Handle          hHsm = NULL;
    BHSM_SetMiscBitsIO_t setMiscBitsIO;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* formulate the request structure */
    setMiscBitsIO.setMiscBitsSubCmd = BCMD_SetMiscBitsSubCmd_eXPTM2MStatusDump;

    rc = BHSM_SetMiscBits(hHsm, &setMiscBitsIO);
    if (rc != 0)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    pStatusQuery->unM2MSecurityErrStatus = setMiscBitsIO.unM2MSecurityErrStatus;
    pStatusQuery->unXPTSecurityErrStatus = setMiscBitsIO.unXPTSecurityErrStatus;

    return NEXUS_SUCCESS;
}



NEXUS_Error NEXUS_Security_DefineSecureRegion(
    const NEXUS_SecuritySRegionSettings *pSettings
    )
{
    BERR_Code                   rc;
    BHSM_Handle                 hHsm;
    BHSM_SetArchIO_t            setArchIO;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if (!hHsm)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* Some error checking */
    if (pSettings->sRegStartAddress & 0x0007)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }
    if (pSettings->sRegEndAddress & 0x0007)
    {
        return ( NEXUS_INVALID_PARAMETER );
    }

    #ifdef NEXUS_SECURITY_MSIPTV
    if ( (pSettings->archType == NEXUS_SecurityArchSelect_eSel02) && (pSettings->DRAMType == NEXUS_SecurityDRAMSelect_eSel00))
    {
        NEXUS_Security_MsiptvSetSecureRegion (pSettings->sRegStartAddress, pSettings->sRegEndAddress);
    }
    #endif
    /*
      * formulate the request structure
     */

    BKNI_Memset( &setArchIO, 0, sizeof(setArchIO));

    setArchIO.unLowerRangeAddress = pSettings->sRegStartAddress>>3;
    setArchIO.unUpperRangeAddress = pSettings->sRegEndAddress>>3;
    setArchIO.ArchSel             = pSettings->archType;
    setArchIO.PCIArch             = pSettings->pciArchType;
    setArchIO.DRAMSel             = pSettings->DRAMType;

    rc = BHSM_SetArch(hHsm, &setArchIO);
    if (rc != 0)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    return NEXUS_SUCCESS;
}



#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
static BERR_Code GeneratePciESignatureVerificationKey ( pciESignatureVerificationKeyConfig_t *pConfig )
{
    BERR_Code rc = NEXUS_SUCCESS;
    BHSM_Handle hHsm;
    BHSM_GenerateGlobalKey_t hsmGlobalKey;
    uint8_t  procInForKey3[NEXUS_PCIE_MAX_WINDOW_PROCIN_SIZE] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                  0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BDBG_CASSERT( (int)BHSM_OwnerIDSelect_eMSP0 == (int)NEXUS_SecurityGlobalKeyOwnerID_eMSP0 );
    BDBG_CASSERT( (int)BHSM_OwnerIDSelect_eMSP1 == (int)NEXUS_SecurityGlobalKeyOwnerID_eMSP1 );
    BDBG_CASSERT( (int)BHSM_OwnerIDSelect_eUse1 == (int)NEXUS_SecurityGlobalKeyOwnerID_eUse1 );

    BKNI_Memset ( &hsmGlobalKey, 0, sizeof( BHSM_GenerateGlobalKey_t ) );

    BKNI_Memcpy( hsmGlobalKey.keyData, procInForKey3, sizeof(procInForKey3) );
    hsmGlobalKey.keyLadderType      = BCMD_KeyLadderType_eAES128;
    hsmGlobalKey.maskKeySelect      = pConfig->maskKeySelect;
    hsmGlobalKey.moduleId           = 0x0F;
    hsmGlobalKey.keySize            = BCMD_KeySize_e128;
    hsmGlobalKey.operation          = BCMD_KeyLadderOp_eDecrypt;
    hsmGlobalKey.routeKeyRequired   = false;
    hsmGlobalKey.globalKeyIndex     = pConfig->globalKeyIndex;
    hsmGlobalKey.stbOwnerId         = pConfig->stbOwnerId;
    hsmGlobalKey.globalKeyOwnerId   = pConfig->globalKeyOwnerId;
    hsmGlobalKey.virtualKeyLadderId = (BCMD_VKLID_e)pConfig->vkl;
    hsmGlobalKey.caVendorId         = pConfig->caVendorId;

    if( ( rc = BHSM_GenerateGlobalKey( hHsm, &hsmGlobalKey ) ) != NEXUS_SUCCESS )
    {
        return BERR_TRACE( rc ); /* failed to generate signature verification key  */
    }

    return rc;
}

static BERR_Code allocateVKL(
    const NEXUS_SecurityPciEMaxWindowSizeSettings_t *pConfig,
	BHSM_AllocateVKLIO_t *vkl
	)
{
    BERR_Code rc = NEXUS_SUCCESS;
    BHSM_Handle hHsm;
    BDBG_ASSERT(pConfig) ;

    NEXUS_Security_GetHsm_priv(&hHsm);
    BDBG_ASSERT(hHsm) ;

    if (pConfig->vkl < NEXUS_SecurityVirtualKeyladderID_eMax)
    {
        BDBG_WRN(("VKL is deprecated from the API, to be removed. Client should not allocate."));
        return rc;
    }

	if (!vkl)
	{
		return BERR_TRACE( NEXUS_INVALID_PARAMETER );
	}

    /* allocate a VKL */
    BKNI_Memset( vkl, 0, sizeof( *vkl ) );

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    vkl->client = BHSM_ClientType_eHost;
   #else
    vkl->customerSubMode = BCMD_CustomerSubMode_eGeneralPurpose1;
   #endif

    if( (rc = BHSM_AllocateVKL( hHsm, vkl ) != BERR_SUCCESS ))
    {
        /* failed to allocate a keyladder. */
        return BERR_TRACE( rc );
    }

    return rc;
}

#endif /* #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2) */

void NEXUS_Security_GetDefaultPciEMaxWindowSizeSettings(
    NEXUS_SecurityPciEMaxWindowSizeSettings_t *pConfig
    )
{
    if( pConfig != NULL )
    {
        BKNI_Memset( pConfig, 0, sizeof( *pConfig ) );
        /* We actually need BCMD_VKLID_e, instead of NEXUS_SecurityVirtualKeyladderID. */
        pConfig->vkl = NEXUS_SecurityVirtualKeyladderID_eMax;
    }

    return;
}


typedef struct {

    struct version{
        unsigned major;
        unsigned minor;
    }version;

    BCMD_STBOwnerID_e      stbOwnerId;
    BCMD_ASKM_MaskKeySel_e maskKeySelect;
    unsigned caVendorId;
    unsigned globalKeyIndex;
    BHSM_OwnerIDSelect_e  globalKeyOwnerId;

} pciESignedCommandHeader;

NEXUS_Error  parsePciESignedCommandHeader(const uint8_t *pRawHeader, pciESignedCommandHeader *pHeader )
{
    BDBG_ASSERT( pRawHeader );
    BDBG_ASSERT( pHeader );

    /* word 0, version */
    pHeader->version.major = (pRawHeader[0]<<8) + pRawHeader[1];
    pHeader->version.minor = (pRawHeader[2]<<8) + pRawHeader[3];

    if( pHeader->version.major == 1 && pHeader->version.minor >= 1 ) /* only version 1.1 (+minor updates) supported  */
    {
        /* word 1 */
        pHeader->caVendorId       = (pRawHeader[4]<<8) + pRawHeader[5];

        switch( pRawHeader[6] ) /* Mask Key select */
        {
            case 0: pHeader->maskKeySelect = BCMD_ASKM_MaskKeySel_eRealMaskKey; break;
            case 1: pHeader->maskKeySelect = BCMD_ASKM_MaskKeySel_eFixedMaskKey; break;
            default:return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }

        switch( pRawHeader[7] ) /* STB owner Id Select */
        {
            case 0: pHeader->stbOwnerId = BCMD_STBOwnerID_eOTPVal; break;
            case 1: pHeader->stbOwnerId = BCMD_STBOwnerID_eOneVal; break;
            default:return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }

        /* word 2 */
        switch( pRawHeader[10] ) /* Global Key Owner Id select */
        {
            case 0: pHeader->globalKeyOwnerId = BHSM_OwnerIDSelect_eMSP0; break;
            case 1: pHeader->globalKeyOwnerId = BHSM_OwnerIDSelect_eMSP1; break;
            default:return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }

        pHeader->globalKeyIndex = pRawHeader[11];

        return NEXUS_SUCCESS;
    }

    return BERR_TRACE( NEXUS_INVALID_PARAMETER );
}


NEXUS_Error NEXUS_Security_SetPciEMaxWindowSize (
    const NEXUS_SecurityPciEMaxWindowSizeSettings_t *pConfig
    )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)

    BHSM_Handle  hHsm;
    BHSM_ReadMspIO_t msp0;
    BHSM_ReadMspIO_t msp1;
    BHSM_PciMaxWindowConfig_t hsmConf;
    BERR_Code rc = NEXUS_SUCCESS;
    pciESignatureVerificationKeyConfig_t keyLadderConfig;
    pciESignedCommandHeader header;
    BHSM_AllocateVKLIO_t vkl;

    BDBG_CASSERT( (NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE - NEXUS_PCIE_MAX_WINDOW_RAW_HEADER_SIZE) == BHSM_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE );

    if ( pConfig == NULL )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    rc = allocateVKL( pConfig, &vkl );

    if (rc)
    {
        return BERR_TRACE( rc );
    }

    if ( parsePciESignedCommandHeader( pConfig->signedCommand, &header ) != NEXUS_SUCCESS )
    {
        /* invalid header format */
        BERR_TRACE( NEXUS_INVALID_PARAMETER );
        goto EXIT;
    }

    BKNI_Memset( &keyLadderConfig, 0, sizeof(keyLadderConfig) );

    keyLadderConfig.vkl              = vkl.allocVKL;

    keyLadderConfig.globalKeyIndex   = header.globalKeyIndex;
    keyLadderConfig.globalKeyOwnerId = header.globalKeyOwnerId;
    keyLadderConfig.stbOwnerId       = header.stbOwnerId;
    keyLadderConfig.maskKeySelect    = header.maskKeySelect;
    keyLadderConfig.caVendorId       = header.caVendorId;

    if( ( rc = GeneratePciESignatureVerificationKey( &keyLadderConfig ) ) != NEXUS_SUCCESS )
    {
        /* failed to generate Key 3 */
        BERR_TRACE( rc );
        goto EXIT;
    }

    /* Check if OTP requires the maximum area of memory accessiable from PCI to be specified (with signed command). */
    BKNI_Memset( &msp0, 0, sizeof(msp0) );
    msp0.readMspEnum = BCMD_Otp_CmdMsp_ePCIe0MwinSizeEnforceEnable;
    BHSM_ReadMSP( hHsm, &msp0 );
    BDBG_WRN(("PCIe0 Signed Max Window [%03d] is %srequired",   msp0.readMspEnum, (msp0.aucMspData[3] & 0x01)?"":"NOT " ) );

    BKNI_Memset( &msp1, 0, sizeof(msp1) );
    msp1.readMspEnum = BCMD_Otp_CmdMsp_ePCIe1MwinSizeEnforceEnable;
    BHSM_ReadMSP( hHsm, &msp1 );
    BDBG_WRN(("PCIe1 Signed Max Window [%03d] is %srequired",   msp1.readMspEnum, (msp1.aucMspData[3] & 0x01)?"":"NOT " ) );

    #if 0 /* Enable at a later date. */
    if( (msp0.aucMspData[3] & 0x01) || (msp1.aucMspData[3] & 0x01)  )  /* if there is Max. Window size enforcement  for either PCI. */
    #endif
    {
        BKNI_Memset( &hsmConf, 0, sizeof(hsmConf) );

        hsmConf.vklId    = pConfig->vkl;
        hsmConf.keyLayer = BCMD_KeyRamBuf_eKey3;
        BKNI_Memcpy( hsmConf.signedCommand,
                     pConfig->signedCommand + NEXUS_PCIE_MAX_WINDOW_RAW_HEADER_SIZE,
                     sizeof(hsmConf.signedCommand) );

        if( BHSM_SetPciMaxWindowSize ( hHsm, &hsmConf ) != BERR_SUCCESS )
        {
           BERR_TRACE( NEXUS_INVALID_PARAMETER );
           goto EXIT;
        }
    }

EXIT:

    BHSM_FreeVKL( hHsm, pConfig->vkl);

    return NEXUS_SUCCESS;
#else
   BSTD_UNUSED( pConfig );
   return  BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}


NEXUS_Error NEXUS_Security_SetPciERestrictedRange( NEXUS_Addr baseOffset, size_t size, unsigned index )
{
    BHSM_Handle  hHsm;
    BCHP_MemoryInfo memInfo;
    BHSM_SetArchIO_t arch;
    bool foundMemc = false;
    unsigned memcIndex = 0;
    unsigned i = 0;
    unsigned numMemc = 0;
    NEXUS_Error rc;

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BKNI_Memset( &memInfo, 0, sizeof( memInfo ) );
    if( ( rc = BCHP_GetMemoryInfo( g_pCoreHandles->reg, &memInfo ) ) !=  BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    /* need to understand if there is an inconsistancy between Nexus and Magnum on number of MEM controllers. */
    BDBG_CASSERT( NEXUS_MAX_MEMC == sizeof(memInfo.memc)/sizeof(memInfo.memc[0]) );

    numMemc = sizeof(memInfo.memc)/sizeof(memInfo.memc[0]);

    /* locate the MEMC that controls the memory. */
    for( i = 0; i < numMemc; i++ )
    {
        if( ( baseOffset >= memInfo.memc[i].offset ) && ( baseOffset < ( memInfo.memc[i].offset + memInfo.memc[i].size ) ) )
        {
            memcIndex = i;
            foundMemc = true;
            break;
        }
    }

    if( foundMemc == false )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* failed to locate  MEMC for baseOffset. */
    }

    /* put ARCH on MEMC of PCIe bus */
    BKNI_Memset( &arch, 0, sizeof(arch) );
    arch.unLowerRangeAddress    = (baseOffset & 0xFFFFFFFF)>>3/*o-word shift*/;
    arch.unUpperRangeAddress    = ((baseOffset + size) & 0xFFFFFFFF)>>3/*o-word shift*/;
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    arch.unLowerRangeAddressMsb =  baseOffset >> 32;
    arch.unUpperRangeAddressMsb = (baseOffset + size) >> 32;

    #if BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(4,2) /*only 4.2*/
    {
        BHSM_Capabilities_t hsmCaps;

        rc = BHSM_GetCapabilities( hHsm, &hsmCaps );
        if( rc != NEXUS_SUCCESS ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

        if( hsmCaps.version.firmware.bseck.major < 4 ) /* Check on Zeus4.2 BFW 0 - 4 (excluding 4) */
        {
            /* Address range must not "span" extended memory boundary */
            if( ( arch.unLowerRangeAddressMsb == 0 ) &&
                ( arch.unUpperRangeAddressMsb != 0 ) )
            {
                return BERR_TRACE( NEXUS_INVALID_PARAMETER );
            }
        }
    }
    #endif
   #endif

    switch( index )
    {
        case 0: { arch.ArchSel = BHSM_ArchSelect_ePciE0; break; }
        case 1: { arch.ArchSel = BHSM_ArchSelect_ePciE1; break; }
        default: return BERR_TRACE( NEXUS_INVALID_PARAMETER );   /* invalid Index */
    }
    arch.DRAMSel = memcIndex;
    arch.pciEExcusive = true; /* we only what to support exclusive for Zeus4.2+ */
    arch.PciEConfig[index].activate = true;
    arch.PciEConfig[index].enableWindow = true;

    if( ( BHSM_SetArch( hHsm, &arch ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /*  close the other MEMCs */
    for( i = 0; i < numMemc; i++ )
    {
        if( ( i != memcIndex ) && ( memInfo.memc[i].size > 0 ) )
        {
            arch.DRAMSel = i;
            /* the start address needs to be greater than the end address by 8 bytes to close the MEMC for PCIe access.*/
            arch.unLowerRangeAddress    = ( ( memInfo.memc[i].offset & 0xFFFFFFFF ) + 8) >> 3;
            arch.unUpperRangeAddress    =   ( memInfo.memc[i].offset & 0xFFFFFFFF )      >> 3;
            #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
            arch.unLowerRangeAddressMsb = (( memInfo.memc[i].offset >> 32 ) & 0xFFFFFFFF );
            arch.unUpperRangeAddressMsb = (( memInfo.memc[i].offset >> 32 ) & 0xFFFFFFFF );
            #endif

           if( ( BHSM_SetArch( hHsm, &arch ) ) != BERR_SUCCESS )
           {
               return BERR_TRACE( NEXUS_INVALID_PARAMETER );
           }
        }
    }

   return NEXUS_SUCCESS;
}



NEXUS_Error NEXUS_Security_AVDSRegistersSetUp(
        const NEXUS_SecurityAVDSRegRangeSettings   *pSettings
    )
{
    BERR_Code                   rc;
    BHSM_Handle                 hHsm;
    BHSM_SetVichRegParIO_t      setVichRegParIO;
    uint32_t                    index;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if (!hHsm)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* Some error checking */
    if (pSettings->nRange > NEXUS_MAX_AVDSVD_RANGE)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BKNI_Memset( &setVichRegParIO, 0, sizeof(setVichRegParIO) );
    /* formulate the request structure */
    setVichRegParIO.virtualKeyLadderID = pSettings->vkl;
    setVichRegParIO.keyLayer           = pSettings->keyLayer;
    setVichRegParIO.nRanges            = pSettings->nRange;
    setVichRegParIO.VDECId             = pSettings->VDECId;
    for (index = 0; index < pSettings->nRange; index++)
    {
        setVichRegParIO.unRangeLo[index] = pSettings->loRange[index];
        setVichRegParIO.unRangeHi[index] = pSettings->hiRange[index];
    }
    /* Last part is the signature */
    BKNI_Memcpy((void *)setVichRegParIO.aucSignature,
                (void *)pSettings->signature,
                NEXUS_HMACSHA256_SIGNATURE_SIZE);

    rc = BHSM_SetVichRegPar(hHsm, &setVichRegParIO);
    if (rc != 0)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* collect output */
    if (setVichRegParIO.unStatus)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Security_AVDSRegistersModify(
    const NEXUS_SecurityAVDSRegModifySettings   *pSettings
    )

{
    BERR_Code                   rc;
    BHSM_Handle                 hHsm;
    BHSM_StartAVDIO_t           startAVDIO;
    uint32_t                    index;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if (!hHsm)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* Some error checking */
    if (pSettings->nAVDReg > NEXUS_MAX_AVDSVD_RANGE)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BKNI_Memset( &startAVDIO, 0, sizeof(startAVDIO) );
    /* formulate the request structure */
    startAVDIO.virtualKeyLadderID = pSettings->vkl;
    startAVDIO.keyLayer           = pSettings->keyLayer;
    startAVDIO.avdID              = pSettings->avdID;
    startAVDIO.numAVDReg          = pSettings->nAVDReg;
    for (index = 0; index < pSettings->nAVDReg; index++)
    {
        startAVDIO.avdRegAddr[index] = pSettings->regAddr[index];
        startAVDIO.avdRegVal[index]  = pSettings->regVal[index];
    }

    /* Last part is the signature */
    BKNI_Memcpy((void *)startAVDIO.aucSignature,
                (void *)pSettings->signature,
                NEXUS_HMACSHA256_SIGNATURE_SIZE);

    rc = BHSM_StartAVD(hHsm, &startAVDIO);
    if (rc != 0)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* collect output */
    if (startAVDIO.unStatus)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    return NEXUS_SUCCESS;
}
