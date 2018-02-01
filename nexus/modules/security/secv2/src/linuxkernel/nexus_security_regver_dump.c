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
#include "priv/nexus_security_regver_priv.h"
#include "priv/nexus_core.h"
#include "bhsm.h"
#include "bhsm_verify_reg.h"
#include "bhsm_misc.h"
#include "bhsm_otpmsp.h"
#include "bsp_s_otp_common.h"
#include "bhsm_bseck.h"

#if NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE || NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/file.h>
#include <linux/fs.h>

BDBG_MODULE(nexus_security_verify_reg_dump);

#ifdef B_REFSW_ANDROID
/* because of file system access limitations (linux, selinux) the location of the dump files
 * cannot arbitraty, so we fix it here to keep it simple.
 */
#define DUMP_ROOT_PREFIX "/data/misc/nxfw"
#else
#define DUMP_ROOT_PREFIX ""
#endif

/*void NEXUS_Security_DumpFirmwareBinary_priv( NEXUS_SecurityRegverRegionID regionId, NEXUS_Addr regionAddress, unsigned regionSize )*/
void NEXUS_Security_DumpFirmwareBinary_priv( NEXUS_SecurityDumpFirmwareData *pData )
{
    struct file *filp;
    char fileName[256] = {0};
    loff_t pos = 0;
    ssize_t written = 0;
    char buffer[4];
    unsigned i;
    void *pRegion;

    BDBG_ENTER( dumpFirmwareBinary );

    pRegion = NEXUS_OffsetToCachedAddr( pData->regionAddress );
    if( !pRegion )  { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    if( pData->regionSize > 0)
    {
        snprintf( fileName, 256, "%s/firmware_zeus%d%d_cpuType%02d_region0x%02X_%s%s.bin", DUMP_ROOT_PREFIX
                                                                             , NEXUS_SECURITY_ZEUS_VERSION_MAJOR
                                                                             , NEXUS_SECURITY_ZEUS_VERSION_MINOR
                                                                             , pData->cpuType
                                                                             , pData->regionId
                                                                             , pData->pDescription
                                                                             #if NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW
                                                                             , "_raw"
                                                                             #else
                                                                             , "_formatted"
                                                                             #endif
                                                                             );

        BDBG_WRN(( "Dumping Firmware to %s  size[%d]", fileName, pData->regionSize ));

        filp = filp_open(fileName, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
        if (IS_ERR(filp))
        {
            BDBG_ERR(( "Could not open file to dump firmware. Error: %ld", PTR_ERR(filp)));
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
                written += __kernel_write(filp, buffer, 4, &pos);
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
                written += __kernel_write(filp, buffer, 4, &pos);
            }
        }

        if( written != pData->regionSize )
        {
            BDBG_ERR(( "*****Failed to write FW [%s] size[%d] written[%d]", fileName, pData->regionSize, written ));
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
                written += __kernel_write(filp, &header[i], 1, &pos);
            }

            if( written != headerLength )
            {
                BDBG_ERR(( "*****Failed to write FW extension [%s] size[%d] written[%d]", fileName, headerLength, written ));
            }
        }
        #endif

        filp_close(filp, NULL);
    }

    BDBG_LEAVE( dumpFirmwareBinary );
    return;
}
#endif
