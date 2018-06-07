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
#include "priv/nexus_security_regver_priv.h"
#include "priv/nexus_core.h"
#include "bhsm.h"
#include "bhsm_verify_reg.h"
#include "bhsm_misc.h"
#include "bhsm_otpmsp.h"
#include "bsp_s_otp_common.h"
#include "bhsm_bseck.h"

#if NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE || NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW
#include <stdio.h>

BDBG_MODULE(nexus_security_verify_reg_dump);

/* void NEXUS_Security_DumpFirmwareBinary_priv( NEXUS_SecurityRegverRegionID regionId, NEXUS_Addr regionAddress, unsigned regionSize ) */
void NEXUS_Security_DumpFirmwareBinary_priv( NEXUS_SecurityDumpFirmwareData *pData )
{
    FILE *pFile = NULL;
    char fileName[256] = {0};
    unsigned written = 0;
    char buffer[4];
    unsigned i;
    void *pRegion;

    BDBG_ENTER( dumpFirmwareBinary );

    pRegion = NEXUS_OffsetToCachedAddr( pData->regionAddress );
    if( !pRegion )  {
        BERR_TRACE( NEXUS_INVALID_PARAMETER );
        return;
    }

    if( pData->regionSize > 0)
    {

        sprintf( fileName, "firmware_zeus%d%d_cpuType%02d_region0x%02X_%s%s.bin", NEXUS_SECURITY_ZEUS_VERSION_MAJOR
                                                                             , NEXUS_SECURITY_ZEUS_VERSION_MINOR
                                                                             , (unsigned)pData->cpuType
                                                                             , pData->regionId
                                                                             , pData->pDescription
                                                                             #if NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW
                                                                             , "_raw"
                                                                             #else
                                                                             , "_formatted"
                                                                             #endif
                                                                             );

        BDBG_WRN(( "Dumping Firmware to %s  size[%d]", fileName, pData->regionSize ));

        pFile = fopen( fileName, "wb" );

        if( pFile == NULL )
        {
            BDBG_ERR(( "Could not open file to dump firmware. Make sure to do chmod 777 -R <directory from which the app executable is being run>" ));
            return;
        }

        if( pData->regionId == NEXUS_SecurityRegverRegionID_eRave )  /* reverse endian for RAVE .. and mask out some bytes!*/
        {
            for ( i = 0; i < (pData->regionSize/4); i++ )
            {
                unsigned tmp = ((uint32_t*)pRegion)[i];
                buffer[0] = 0/*tmp >> 24*/;
                buffer[1] = (tmp>>16) & 0x3F;
                buffer[2] = (tmp>>8 ) & 0xFF;
                buffer[3] = (tmp    ) & 0xFF;
                written += fwrite( buffer, 1, 4, pFile );
            }
        }
        else
        {
            for ( i = 0; i < (pData->regionSize/4); i++ )
            {
                unsigned tmp = ((uint32_t*)pRegion)[i];
                buffer[0] = (tmp >> 24);
                buffer[1] = (tmp >> 16) & 0xFF;
                buffer[2] = (tmp >> 8 ) & 0xFF;
                buffer[3] = (tmp      ) & 0xFF;
                written += fwrite( buffer, 1, 4, pFile );
            }
        }

        if( written != pData->regionSize )
        {
            BDBG_ERR(( "*****Failed to write FW [%s] size[%d] written[%d]", fileName, pData->regionSize, written ));
        }
    }

    #ifndef NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW
    {
        uint8_t header[256] = {0}; /* data to be added to end of firmware file for signing. */
        unsigned headerLength = 0;

        /* See command manual for description of header*/
        header[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_CPU_TYPE]  = pData->cpuType & 0xFF;
        BKNI_Memset( &header[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_MARKET_ID_MASK], 0xFF, 4);
        header[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_EPOCH_MASK]   = 0xFF;
        headerLength = 16;
       #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
        header[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_EPOCH_SELECT] = pData->epochSelect;
        header[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_SIG_VERSION]  = 0x01;  /*Default signature version*/
        header[NEXUS_SECURITY_P_SIGNATURE_HEADER_OFFSET_SIG_TYPE]     = BCMD_SigType_eCode; /* Default signature type */
        headerLength = 20; /*update length*/
       #endif

        for( i = 0, written = 0; i < headerLength; i++ )
        {
            written += fwrite( &header[i], 1, 1, pFile );
        }

        if( written != headerLength )
        {
            BDBG_ERR(( "*****Failed to write FW extension [%s] size[%d] written[%d]", fileName, headerLength, written ));
        }
    }
    #endif

    if( pFile ) fclose( pFile );

    BDBG_LEAVE( dumpFirmwareBinary );
    return;
}
#endif
