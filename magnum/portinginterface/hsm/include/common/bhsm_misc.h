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


#ifndef BHSM_MISC_H__
#define BHSM_MISC_H__



/* added to remove header file dependency chain*/
#include "bsp_s_misc.h"

#include "bhsm.h"


#ifdef __cplusplus
extern "C" {
#endif

#define BHSM_MAX_PCIE                        2   /* maxumum number of PCIe devices handled */
#define BHSM_SECURITY_MAX_PCI_CONTROLLERS    3
#define BHSM_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE  (8*4 + 32)   /* size of PCIe command signature .. 8 words of command + 32 bytes of signature. */


/**************************************************************************************************
Summary:

Description:
Structure that defines BHSM_SetMiscBitsIO members

See Also:
BHSM_SetMiscBits
**************************************************************************************************/
typedef struct BHSM_SetMiscBitsIO {

    /* In: SubCommand ID for BCMD_SetMiscBits command. */
    BCMD_SetMiscBitsSubCmd_e    setMiscBitsSubCmd;

    /****  Rave Ctrl SubCmd ****/
    /* In: value must be '1'.  Enable RAVE0 IMEM to be written by host b(0) */
    bool                         bEnableWriteIMem;

    /* In: value must be '1'. Enable RAVE0 IMEM to be read by host      b(1)*/
    bool                         bEnableReadIMem;

    /* In: value must be '1'. Enable RAVE0 DMEM to be read by host     b(2)*/
    bool                         bEnableReadDMem;

    /* In: Bypass encryption engine for RAVE                                    b(3)*/
    bool                        bRAVEEncryptionBypass;

    /* In: When set, this will remove the RAVE from reset.  RAVE is placed in reset
    whenever code is written to RAVE IMEM.                                     b(4) */
    bool                           bDisableClear;

    /**** SubCommand 0x01 Not MPEG TS  ****/
    /* In:   b(0) Controls AUX packet scrambling/descrambling using AES ECB MSC algorithm */
    bool                         bDMAXPTPacketBypassEnable;

	/* In:   b(1) Controls AUX packet scrambling/descrambling*/
	bool               			bNMAPacketBypassEnable;

    /* In:   b(2) Controls AUX packet scrambling/descrambling using AES ECB MSC algorithm - For M2M */
    bool                           bDMAM2MPacketBypassEnable;

    /**** M2M Endian Swap SubCommand ****/
    /* In:   b(0) Swap each 32-bit word byte order of the input data */
    bool                           bCBCMACDataInSwapEnable;

    /* In:   b(1) Swap each 32-bit word byte order of the output data */
    bool                           bCBCMACDataOutSwapEnable;

    /* In:   b(2) Swap each 32-bit word byte order of the key */
    bool                           bCBCMACKeySwapEnable;

    /**** ForceSC SubCommand ****/
    /* In:   b(0..7)   Band Select - 0 : IB  1 : PB */
    bool                           bBandSelect_PB;

    /* In:   b(8..15) Band number ( IB[0...15] or PB[0...31] */
    unsigned char                bandNumber;

    /* In:   b(0)      Control bits SC_FORCE_EN_ALL */
    bool                        bSCForceEnAll;

    /* In:   b(1)      Control Bits SC_FORCE_EN_NZ */
    bool                        bSCForceEnNZ;

    /* In:   b(2, 3)  Control Bits SC_FORCE_EN_VALUE */
    unsigned char                 SCForceEnValue;

    /**** XPT Pipe Bypass Cntrl Subcommand ****/
    /* In: b(0) Controls bypass of all pipes */
    bool                        bXPTPipesBypass;

    /**** XPT Reset Cntrl Subcommand ****/
    /* In:  b(0) Controls XPT reset */
    bool                        bXPTReset;


    /* Out: 0 for success, otherwise failed. */
    uint32_t                    unStatus;

    /* Out: For Dump XPT/M2M Status SubCmd. */
    uint32_t                     unM2MSecurityErrStatus;

    /* Out: For Dump XPT/M2M Status SubCmd. */
    uint32_t                    unXPTSecurityErrStatus;

} BHSM_SetMiscBitsIO_t;


/**************************************************************************************************
Summary:
This function is used to remove RAVE from reset.

Description:
This function is used to remove the RAVE from reset.  RAVE is placed in reset whenever the Host
CPU writes to RAVE IMEM.  When RAVE is removed from reset, it will begin executing code from
IMEM.  Video/Audio will not be descrambled until the RAVE is out of reset.

Calling Context:
This function is called after RAVE firmware has been loaded into IMEM and descrambling of
video/audio is ready to begin.

Performance and Timing:
This is a synchronous/blocking function that will not return until it completes.

Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_setMiscBitsIO  - BHSM_SetMiscBitsIO_t.

Output:
inoutp_setMiscBitsIO  - BHSM_SetMiscBitsIO_t.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
N/A
**************************************************************************************************/
BERR_Code   BHSM_SetMiscBits (
        BHSM_Handle                    in_handle,
        BHSM_SetMiscBitsIO_t        *inoutp_setMiscBitsIO
);

/* deprecated. Use PciEConfig::activate */
typedef enum BCMD_PCIArchType_e
{
    BCMD_PCIArchType_eNonPCIWin,
    BCMD_PCIArchType_ePCIWin,
    BCMD_PCIArchType_ePCIeWin,
    BCMD_PCIArchType_eMax
}BCMD_PCIArchType_e;


typedef enum BHSM_ArchSelect_e
{
    BCMD_ArchSelect_eSel00 = 0,  /* deprecated */
    BCMD_ArchSelect_eSel01 = 1,  /* deprecated */
    BCMD_ArchSelect_eSel02 = 2,  /* deprecated */
    BCMD_ArchSelect_eSel03 = 3,  /* deprecated */

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    BHSM_ArchSelect_eRestrictedRegion = 2,
    BHSM_ArchSelect_ePciE0            = 3,
    BHSM_ArchSelect_ePciE1            = 4,
    BHSM_ArchSelect_eSCpu             = 5,
   #else
    BHSM_ArchSelect_ePciE0            = 2,
    BHSM_ArchSelect_ePciE1            = 3,
   #endif


    BHSM_ArchSelect_eMax
}BCMD_ArchSelect_e, BHSM_ArchSelect_e;


/* Depreacated */
typedef enum BCMD_DRAMSelect_e
{
    BCMD_DRAMSelect_eSel00,
    BCMD_DRAMSelect_eSel01,
    BCMD_DRAMSelect_eMax
}BCMD_DRAMSelect_e;



/**************************************************************************************************
Summary:

Description:
Structure that defines BHSM_SetArchIO members

See Also:
BHSM_SetArch
**************************************************************************************************/
typedef struct BHSM_SetArchIO {

    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    /* In: defines the MSB of the start address for the secure region. */
    uint32_t            unLowerRangeAddressMsb;

    /* In: defines the MSB of the end address (inclusive) for the secure region. */
    uint32_t            unUpperRangeAddressMsb;
    #endif

    /* In: defines the start address for the secure region. */
    uint32_t            unLowerRangeAddress;

    /* In: defines the end address (inclusive) for the secure region. */
    uint32_t            unUpperRangeAddress;

    /* In: Select PCI, Restrictred, sage, etc,  */
    BHSM_ArchSelect_e   ArchSel;

    BCMD_PCIArchType_e    PCIArch; /*deprecated. Use PciEConfig::activate */

    /* Only required if ArchSel is BHSM_ArchSelect_ePciEx. True for exclusive(post-Zeus4), false for non-exclusive (pre-Zeus4)*/
    bool pciEExcusive;

    /* Only required if ArchSel is BHSM_ArchSelect_ePciEx. Configuration for each PCIe */
    struct {
        bool activate;
        bool enableWindow;
    }PciEConfig[BHSM_MAX_PCIE];

    /*In: defines DRAM Select, range[0..2] depending on chip support. */
    unsigned              DRAMSel;

    /* Out: 0   Deprecated   */
    uint32_t            unStatus;
} BHSM_SetArchIO_t;

/**************************************************************************************************
Summary:
This function configures the address range of secure region. It is currently designed for future
enhanced features.

Description:
The host DRAM is partitioned into Secure Region and Global Region. This function specifies
the address range of the secure region.  Everything outside the specified region would be
the global region.

Calling Context:
It shall be called after the heap is allocated for secure region and specific buffer pointers are
initialized.

Performance and Timing:
This is a synchronous/blocking function that will not return until it completes.

Input:
in_handle  - BHSM_Handle, Host Secure module handle.
pSetArch  -  BHSM_SetArchIO_t.

Output:
pSetArch  - BHSM_SetArchIO_t.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
N/A
**************************************************************************************************/
BERR_Code BHSM_SetArch ( BHSM_Handle in_handle, BHSM_SetArchIO_t *pSetArch );



#define MAX_AVD_SVD_RANGE                8
#define BHSM_HMACSHA256_SIGNATURE_SIZE    32

typedef enum BCMD_AVDType_e
{
    BCMD_AVDType_eVDEC0  = 0, /* Zeus4.1+ */
    BCMD_AVDType_eVDEC1  = 4, /* Zeus4.1+ */
    BCMD_AVDType_eVDEC2  = 5, /* Zeus4.1+ */

    BCMD_AVDType_eSVD    = 0,
    BCMD_AVDType_eAVD    = 1, /* Reserved for Zeus4.2+, pre Zeus4.2 */

    BCMD_AVDType_eVICE   = 2,
    BCMD_AVDType_eVICE1  = 3,

    BCMD_AVDType_eMax
}BCMD_AVDType_e;


/**************************************************************************************************
Summary:

Description:
Structure that defines BHSM_SetVichRegParIO members

See Also:
BHSM_SetVichRegPar
**************************************************************************************************/
typedef struct BHSM_SetVichRegParIO {

    /* In: key from which virtual key ladder to invalidate */
    BCMD_VKLID_e        virtualKeyLadderID;

    /* In: key from which key layer of the above virtual key ladder to invalidate */
    BCMD_KeyRamBuf_e    keyLayer;

    /* In: VDEC ID -  0 : SVD   1 : AVD   2 : VICE */
    BCMD_AVDType_e        VDECId;

    /* In: Number of ranges  - max 8 for AVD/SVD  5 for VICE */
    unsigned char        nRanges;

    /* In: start address (Low, Inclusive) */
    uint32_t            unRangeLo[MAX_AVD_SVD_RANGE];

    /* In: end address (High, Inclusive)  */
    uint32_t            unRangeHi[MAX_AVD_SVD_RANGE];

    /* In: Signature for the entire command */
    unsigned char        aucSignature[BHSM_HMACSHA256_SIGNATURE_SIZE];

    /* Out: 0 for success, otherwise failed */
    uint32_t            unStatus;

} BHSM_SetVichRegParIO_t;


/**************************************************************************************************
Summary:
This function is used to protect certain AVD registers.

Description:
This function is used to protect AVD registers.  Once the AVD registers are protected, the
Host can no longer write to those registers.  It is assumed that the Host initially configures
these AVD registers and calls this command with authenticated code.

Calling Context:
This function is called after the Host CPU configures the AVD registers to the desired values.

Performance and Timing:
This is a synchronous/blocking function that will not return until it completes.

Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_setVichIO  - BHSM_SetVichRegParIO_t.

Output:
inoutp_setVichIO  - BHSM_SetVichRegParIO_t.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
N/A
**************************************************************************************************/
BERR_Code  BHSM_SetVichRegPar (
        BHSM_Handle                    in_handle,
        BHSM_SetVichRegParIO_t        *inoutp_setVichRegParIO
);





/**************************************************************************************************
Summary:

Description:
Structure that defines BHSM_StartAVDIO members

See Also:
BHSM_StartAVD
**************************************************************************************************/
typedef struct BHSM_StartAVDIO {

    /* In: key from which virtual key ladder to use for key generation */
    BCMD_VKLID_e        virtualKeyLadderID;

    /* In: key from which key layer of the above virtual key ladder to use for key generation */
    BCMD_KeyRamBuf_e    keyLayer;

    /*In: Which AVD to restart */
    BCMD_AVDType_e        avdID;

    /*In: Number of AVD Register/Address pairs to use */
    uint32_t            numAVDReg;

    /* In: address to program */
    uint32_t            avdRegAddr[MAX_AVD_SVD_RANGE];

    /* In: value to program  */
    uint32_t            avdRegVal[MAX_AVD_SVD_RANGE];

    /* In: Signature for the entire command */
    unsigned char        aucSignature[BHSM_HMACSHA256_SIGNATURE_SIZE];

    /* Out: 0 for success, otherwise failed */
    uint32_t            unStatus;

} BHSM_StartAVDIO_t;


/**************************************************************************************************
Summary:

Description:

Calling Context:

Performance and Timing:
This is a synchronous/blocking function that will not return until it completes.

Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_startAVDIO  - BHSM_StartAVDIO_t.

Output:
inoutp_startAVDIO  - BHSM_StartAVDIO_t.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
N/A
**************************************************************************************************/
BERR_Code  BHSM_StartAVD (
        BHSM_Handle                in_handle,
        BHSM_StartAVDIO_t        *inoutp_startAVDIO
);

typedef struct BHSM_PciMaxWindowConfig {

    BCMD_VKLID_e vklId;          /* virtual keyladder */
    BCMD_KeyRamBuf_e keyLayer;       /* virtual keylader layer */
    uint8_t      signedCommand[BHSM_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE];  /* Command and signature combined */

}BHSM_PciMaxWindowConfig_t;

/* This function allows the specification of the MAXIMUM amount of memory that can be
   accessed from PCI. This is a signed command. It is most likely that the CA-vendor is the signing authority.
   Use SetARCH to secify that actual size and location of the memory that can be accesse from PCI. */
BERR_Code BHSM_SetPciMaxWindowSize ( BHSM_Handle hHsm,
                                     BHSM_PciMaxWindowConfig_t *pMaxConf );


#ifdef __cplusplus
}
#endif

#endif /* BHSM_MISC_H__ */
