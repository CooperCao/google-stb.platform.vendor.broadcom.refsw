/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/

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


#define NEXUS_PCIE_MAX_WINDOW_RAW_HEADER_SIZE   (3*4 + 20) /* 3*32bit + 160bit */
#define NEXUS_PCIE_MAX_WINDOW_PROCIN_SIZE       (16)

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)

typedef struct pciESignatureVerificationKeyConfig {

    NEXUS_SecurityVirtualKeyladderID vkl;

    unsigned globalKeyIndex;                /* Identify Global Key by index*/
    BHSM_OwnerIDSelect_e globalKeyOwnerId;
    BCMD_STBOwnerID_e stbOwnerId;
    BCMD_ASKM_MaskKeySel_e maskKeySelect;
    uint32_t caVendorId;                    /* Ca Vendor separation on root key. */

}pciESignatureVerificationKeyConfig;

static NEXUS_Error GeneratePciESignatureVerificationKey( pciESignatureVerificationKeyConfig *pConfig );

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
static NEXUS_Error GeneratePciESignatureVerificationKey ( pciESignatureVerificationKeyConfig *pConfig )
{
    BERR_Code rc = BERR_SUCCESS;
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
        return BERR_TRACE( MAKE_HSM_ERR(rc) ); /* failed to generate signature verification key  */
    }

    return NEXUS_SUCCESS;
}
#endif /* #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2) */


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


#define NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE_VERSION_1_1 (96)


NEXUS_Error parsePciESignedCommandHeader(const uint8_t *pSignedCommand,  unsigned signedCommandLength, pciESignedCommandHeader *pHeader )
{
    BDBG_ASSERT( pSignedCommand );
    BDBG_ASSERT( pHeader );

    /* word 0, version */
    pHeader->version.major = (pSignedCommand[0]<<8) + pSignedCommand[1];
    pHeader->version.minor = (pSignedCommand[2]<<8) + pSignedCommand[3];

    if( pHeader->version.major == 1 && pHeader->version.minor >= 1 ) /* only version 1.1 (+minor updates) supported  */
    {
        BDBG_CASSERT( NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE_VERSION_1_1 <= NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE );

        if( ( signedCommandLength != NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE_VERSION_1_1 ) &&
            ( signedCommandLength != 0 /* back compat */ ) )
        {
            return BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* signed command is the wrong length */
        }

        /* word 1 */
        pHeader->caVendorId       = (pSignedCommand[4]<<8) + pSignedCommand[5];

        switch( pSignedCommand[6] ) /* Mask Key select */
        {
            case 0: pHeader->maskKeySelect = BCMD_ASKM_MaskKeySel_eRealMaskKey; break;
            case 1: pHeader->maskKeySelect = BCMD_ASKM_MaskKeySel_eFixedMaskKey; break;
            default:return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }

        switch( pSignedCommand[7] ) /* STB owner Id Select */
        {
            case 0: pHeader->stbOwnerId = BCMD_STBOwnerID_eOTPVal; break;
            case 1: pHeader->stbOwnerId = BCMD_STBOwnerID_eOneVal; break;
            default:return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }

        /* word 2 */
        switch( pSignedCommand[10] ) /* Global Key Owner Id select */
        {
            case 0: pHeader->globalKeyOwnerId = BHSM_OwnerIDSelect_eMSP0; break;
            case 1: pHeader->globalKeyOwnerId = BHSM_OwnerIDSelect_eMSP1; break;
            default:return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }

        pHeader->globalKeyIndex = pSignedCommand[11];

        return NEXUS_SUCCESS;
    }

    return BERR_TRACE( NEXUS_INVALID_PARAMETER );
}



void NEXUS_Security_GetDefaultPciEMaxWindowSizeSettings(
    NEXUS_SecurityPciEMaxWindowSizeSettings *pConfig
    )
{
    if( pConfig != NULL )
    {
        BKNI_Memset( pConfig, 0, sizeof( *pConfig ) );
        pConfig->vkl = NEXUS_SecurityVirtualKeyladderID_eMax;  /* vkl is DEPRECATED. *Not* to be modified. */
    }

    return;
}




NEXUS_Error NEXUS_Security_SetPciEMaxWindowSize (
    const NEXUS_SecurityPciEMaxWindowSizeSettings *pConfig
    )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BHSM_Handle  hHsm;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BERR_Code magnumRc = BERR_SUCCESS;
    BHSM_ReadMspIO_t msp0;
    BHSM_ReadMspIO_t msp1;
    BHSM_PciMaxWindowConfig_t hsmConf;
    pciESignatureVerificationKeyConfig keyLadderConfig;
    pciESignedCommandHeader header;
    BHSM_AllocateVKLIO_t vklConf;

    if( pConfig == NULL )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    if( pConfig->vkl != NEXUS_SecurityVirtualKeyladderID_eMax )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );  /* Don't configure vkl, the paramater is deprecated.  */
    }

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* allocate a VKL */
    BKNI_Memset( &vklConf, 0, sizeof( vklConf ) );
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    vklConf.client = BHSM_ClientType_eHost;
   #else
    vklConf.customerSubMode = BCMD_CustomerSubMode_eGeneralPurpose1;
   #endif
    if( ( magnumRc = BHSM_AllocateVKL( hHsm, &vklConf ) != BERR_SUCCESS ) )
    {
        return BERR_TRACE( MAKE_HSM_ERR( magnumRc ) );
    }

    if( parsePciESignedCommandHeader( pConfig->signedCommand, pConfig->signedCommandLength, &header ) != NEXUS_SUCCESS )
    {
        BERR_TRACE( NEXUS_INVALID_PARAMETER );   /* invalid header format */
        goto EXIT;
    }

    BKNI_Memset( &keyLadderConfig, 0, sizeof(keyLadderConfig) );

    keyLadderConfig.vkl              = vklConf.allocVKL;
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

        hsmConf.vklId    = vklConf.allocVKL;
        hsmConf.keyLayer = BCMD_KeyRamBuf_eKey3;
        BKNI_Memcpy( hsmConf.signedCommand,
                     pConfig->signedCommand + NEXUS_PCIE_MAX_WINDOW_RAW_HEADER_SIZE,
                     sizeof(hsmConf.signedCommand) );

        if( ( magnumRc = BHSM_SetPciMaxWindowSize ( hHsm, &hsmConf ) ) != BERR_SUCCESS )
        {
            rc =  BERR_TRACE( MAKE_HSM_ERR( magnumRc ) );
            goto EXIT;
        }
    }

EXIT:

    BHSM_FreeVKL( hHsm, vklConf.allocVKL );

    return rc;
#else
   BSTD_UNUSED( pConfig );
   return  BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}


NEXUS_Error NEXUS_Security_SetPciERestrictedRange( NEXUS_Addr baseOffset, size_t size, unsigned index )
{
    BHSM_Handle  hHsm;
    const BCHP_MemoryLayout *memoryLayout;
    BHSM_SetArchIO_t arch;
    bool foundMemc = false;
    unsigned memcIndex = 0;
    unsigned i = 0;
    unsigned numMemc = 0;
    size_t addressUpdate = size < 1 ? 0 : size - 1;

    if( index >= BHSM_MAX_PCIE ){ return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    memoryLayout = &g_pCoreHandles->memoryLayout;

    /* need to understand if there is an inconsistancy between Nexus and Magnum on number of MEM controllers. */
    BDBG_CASSERT( NEXUS_MAX_MEMC == sizeof(memoryLayout->memc)/sizeof(memoryLayout->memc[0]) );

    numMemc = sizeof(memoryLayout->memc)/sizeof(memoryLayout->memc[0]);

    /* Testing if the PCIE ARCH can be on MEMC1. */
    /* baseOffset = memoryLayout->memc[1].region[0].addr + 16; */

    for( i = 0; i < numMemc; i++ )
    {
        if( ( baseOffset >= memoryLayout->memc[i].region[0].addr ) && ( baseOffset < ( memoryLayout->memc[i].region[0].addr + memoryLayout->memc[i].region[0].size ) ) )
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

  #if BHSM_ZEUS_VERSION <= BHSM_ZEUS_VERSION_CALC(2,0)
    if (memcIndex != 0)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* Only support on MEMC0 */
    }
  #endif

    /* put ARCH on MEMC of PCIe bus */
    BKNI_Memset( &arch, 0, sizeof(arch) );

    arch.PciEConfig[index].activate = true;
    arch.DRAMSel                    = memcIndex;

  #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)

    arch.unLowerRangeAddress    = (baseOffset & 0xFFFFFFFF)>>3/*o-word shift*/;
    arch.unUpperRangeAddress    = ((baseOffset + addressUpdate ) & 0xFFFFFFFF)>>3/*o-word shift*/;
    arch.unLowerRangeAddressMsb =  baseOffset >> 32;
    arch.unUpperRangeAddressMsb = (baseOffset + addressUpdate ) >> 32;

    #if BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(4,2) /*only 4.2*/
    {
        BHSM_Capabilities_t hsmCaps;
        NEXUS_Error rc;

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

    switch( index )
    {
        case 0: { arch.ArchSel = BHSM_ArchSelect_ePciE0; break; }
        case 1: { arch.ArchSel = BHSM_ArchSelect_ePciE1; break; }
        default: return BERR_TRACE( NEXUS_INVALID_PARAMETER );   /* invalid Index */
    }

    arch.pciEExcusive = true; /* we only what to support exclusive for Zeus4.2+ */
    arch.PciEConfig[index].enableWindow = true;

    if( ( BHSM_SetArch( hHsm, &arch ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /*  Followings are for closing the other MEMCs, and blocking other memory. */

    /* For exclusive supporting Zeus platforms. */
    for( i = 0; i < numMemc; i++ )
    {
        if( ( i != memcIndex ) && ( memoryLayout->memc[i].region[0].size > 0 ) )
        {
            arch.DRAMSel = i;
            /* the start address needs to be greater than the end address by 8 bytes to close the MEMC for PCIe access.*/
            arch.unLowerRangeAddress    = ( ( memoryLayout->memc[i].region[0].addr & 0xFFFFFFFF ) + 8) >> 3;
            arch.unUpperRangeAddress    =   ( memoryLayout->memc[i].region[0].addr & 0xFFFFFFFF )      >> 3;
            #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
            arch.unLowerRangeAddressMsb = (( memoryLayout->memc[i].region[0].addr >> 32 ) & 0xFFFFFFFF );
            arch.unUpperRangeAddressMsb = (( memoryLayout->memc[i].region[0].addr >> 32 ) & 0xFFFFFFFF );
            #endif

           if( ( BHSM_SetArch( hHsm, &arch ) ) != BERR_SUCCESS )
           {
               return BERR_TRACE( NEXUS_INVALID_PARAMETER );
           }
        }
    }

  #else /* #if BHSM_ZEUS_VERSION > BHSM_ZEUS_VERSION_CALC(4,2) */

    /* Non-exclusive mode only. Following range will be blocked. */
    /* Use the first ARCH to block the first section. */
    if (baseOffset > memoryLayout->memc[memcIndex].region[0].addr)
    {
        arch.unLowerRangeAddress        = (memoryLayout->memc[memcIndex].region[0].addr & 0xFFFFFFFF)>>3/*o-word shift*/;
        arch.unUpperRangeAddress        = ((baseOffset - 1) & 0xFFFFFFFF)>>3/*o-word shift*/;
        arch.ArchSel                    = BHSM_ArchSelect_ePciE0;

        if( ( BHSM_SetArch( hHsm, &arch ) ) != BERR_SUCCESS )
        {
            return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }
    }

    /* Followings are for closing the other MEMCs, and blocking other memory. */
    /* Using the second PCIE arch to block the remained section. */
    arch.unLowerRangeAddress = ((baseOffset + addressUpdate + 1 ) & 0xFFFFFFFF) >> 3;
    arch.unUpperRangeAddress = ((memoryLayout->memc[memcIndex].region[0].addr + memoryLayout->memc[memcIndex].region[0].size - 1) & 0xFFFFFFFF) >> 3;
    arch.ArchSel             = BHSM_ArchSelect_ePciE1;

    if( ( BHSM_SetArch( hHsm, &arch ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }


  #endif /* #if BHSM_ZEUS_VERSION > BHSM_ZEUS_VERSION_CALC(4,2) */

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
