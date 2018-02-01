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
 **************************************************************************/
#include <string.h>
#include <fcntl.h>
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"

#include "drm_key_region.h"

#include "drm_data.h"
#include "drm_key_binding.h"
#include "drm_common_swcrypto.h"
#include "drm_secure_store.h"
#include "drm_common.h"

BDBG_MODULE(drm_key_region);

/* Defines */
#define DRM_BIN_FILE_COOKIE_ALL_IN_ONE  (0xAAC50004)
#define DRM_BIN_FILE_COOKIE_NONREF   (0xAAC5050A)
#define DRM_BIN_FILE_COOKIE_CIP_STANDALONE (0xAAC5050B)
#define DRM_BIN_FILE_COOKIE_CIP_STANDARD (0xAAC5050C)
#define DRM_BIN_FILE_COOKIE_WV_STANDALONE (0xAAC50006)
#define DRM_BIN_FILE_COOKIE_SELF_OFFSET_AWARE (0xAAC50007)
#define DRM_BIN_FILE_COOKIE_SECURE_SW_RSA_V2 (0xAAC50008)

/* Structure definitions */
typedef struct DrmKeyRegionOffsetStruct_t
{
    uint32_t DrmType;
    uint32_t OffsetRelativeToEOT;
}DrmKeyRegionOffsetStruct_t;
static DrmKeyRegionOffsetStruct_t *pOffsetStruct = NULL;

/* Static variable definitions */
static uint8_t* drm_bin_file_buff = NULL;
static char generic_drm_filepath[256] = {0x00};
static bool drm_region_ok = false;
static uint32_t file_size_from_header = 0;
static bool nonref_detected = false;
static bool wv_standalone_detected = false;
static bool cip_standalone_detected = false;
static bool split_mode_active = false;
static DrmKeyProvisioningType keyProvisioningType = DrmKeyProvisioningType_eBrcm;
static bool bDrmBinFileSelfAware = false;
static uint32_t number_of_drms = 0;
static uint32_t gCookie = 0x00;
static BKNI_MutexHandle mutex = 0;

static drm_header_t drm_bin_file_header;
static struct CommonCryptoKeyLadderSettings keyInfo;

/* Static function definitions */
static DrmRC DRM_KeyRegion_P_GetDrmRegionPath(char *path);
static DrmRC DRM_KeyRegion_P_ReadDrmRegionData(char *path, uint8_t* pBuf);
static DrmRC DRM_KeyRegion_P_ValidateDrmData(uint8_t* pBuf, bool decrypt_first);
static DrmRC DRM_KeyRegion_P_ValidateCookie(drm_types_e drm_type);
static DrmRC DRM_KeyRegion_P_GetHeaderInfo(DrmTypeFlash  type,  uint8_t* pdata);
static DrmRC DRM_KeyRegion_P_ProcessSelfOffsetAware(void);
static DrmRC DRM_KeyRegion_P_SetSelfAwareOffset(uint32_t *offset, drm_types_e drm_type);

/* Variable and private API definitions specific for reading DRM bin file from flash */
static DrmRC DRM_KeyRegion_P_WriteDrmRegion(uint8_t* pBuf);

#if (NEXUS_SECURITY_API_VERSION==1)
static DrmRC _Crypto_AesEcb( uint8_t *pBuf, uint32_t buf_size,  NEXUS_SecurityOperation op_type);
#else
static DrmRC _Crypto_AesEcb( uint8_t *pBuf, uint32_t buf_size,  NEXUS_CryptographicOperation op_type);
#endif

/* Static variable definitions */
static drm_key_binding_t drm_binding_obj;

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyRegion_Init
 **
 ** DESCRIPTION:
 **   Initializes the 'KeyRegion' module by fetching the confidential data from
 **   the rootfs or flash
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 ******************************************************************************/
DrmRC DRM_KeyRegion_Init(void)
{
    DrmRC rc = Drm_Success;
    char path[256] = {0x00};
    uint8_t hash_value[DRM_HASH_SIZE] = {0x00};
    bool bBoundFileRetrieved = false;

    if(mutex == 0){
        BKNI_CreateMutex(&mutex);
    }

    BDBG_MSG(("%s - Entered function, provision type = '%u'", BSTD_FUNCTION, keyProvisioningType));

    /* Fetch Device IDs */
    rc = DRM_KeyBinding_FetchDeviceIds(&drm_binding_obj);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s -  Error fetching device IDs", BSTD_FUNCTION));
        rc =  Drm_InitErr;
        goto ErrorExit;
    }

    if(keyProvisioningType == DrmKeyProvisioningType_eBrcm)
    {
        /* Allocate memory for DRM bin file buffer */
        if(drm_bin_file_buff == NULL)
        {
            if(DRM_Common_MemoryAllocate(&drm_bin_file_buff, DRM_BIN_FILE_SIZE) != Drm_Success)
            {
                BDBG_ERR(("%s - Error allocating '%u' bytes for DRM bin file", BSTD_FUNCTION, DRM_BIN_FILE_SIZE));
                rc = Drm_InitErr;
                goto ErrorExit;
            }

            /* Clear buffer */
            BKNI_Memset(drm_bin_file_buff, 0x00, DRM_BIN_FILE_SIZE);
        }

        /* Fetch the path of the device which corresponds to the DRM Region in the MTD partition table */
        rc = DRM_KeyRegion_P_GetDrmRegionPath(path);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error fetching path of DRM bin", BSTD_FUNCTION));
            goto ErrorExit;
        }

        /* Read and validate DRM region */
        rc = DRM_KeyRegion_P_ReadDrmRegionData(path, drm_bin_file_buff);
        if(rc != Drm_Success)
        {
            BDBG_MSG(("%s - Error validating DRM binary data from DRM region", BSTD_FUNCTION));
            drm_region_ok = false;
        }
        else
        {
            BDBG_MSG(("%s - Reading DRM binary file from drm_region (%s) successful", BSTD_FUNCTION, path));
            drm_region_ok = true;
        }

        /********************************************************************************
         At this point in the code, 'drm_bin_file_buff' contains data from the DRM region
        ********************************************************************************/

        if( (BKNI_Memcmp(&drm_bin_file_buff[BINDING_MARKER_OFFSET    ], drm_binding_obj.devIdA, 8) == 0) ||
            (BKNI_Memcmp(&drm_bin_file_buff[BINDING_MARKER_OFFSET + 8], drm_binding_obj.devIdB, 8) == 0) )
            {
                /* Skip to decryption below */
                BDBG_MSG( ("%s - BOUND bin file detected...", BSTD_FUNCTION));
                bBoundFileRetrieved = true;
            }
            else
            {
                /******************************************************************************
                 * Bind DRM data by placing (Device ID A ||Device ID B) in binary data header *
                 * regenerate signature and update                                            *
                 *****************************************************************************/
                BKNI_Memcpy(&drm_bin_file_buff[BINDING_MARKER_OFFSET], drm_binding_obj.devIdA, 8);
                BKNI_Memcpy(&drm_bin_file_buff[BINDING_MARKER_OFFSET + 8], drm_binding_obj.devIdB, 8);

                // Re-compute SHA-1 hash value always from beginning of DRM bin file to just before padded-hash-size
                BKNI_Memset(hash_value, 0x00, DRM_HASH_SIZE);
                if(DRM_Common_SwSha1(drm_bin_file_buff,
                                     hash_value,
                                     file_size_from_header - DRM_HASH_SIZE) != Drm_Success)
                {
                    BDBG_ERR( ("%s - Error calculating SHA-1", BSTD_FUNCTION));
                    rc = Drm_InitErr;
                    goto ErrorExit;
                }
                else
                {
                    /* Copy hash to last 20 bytes of DRM bin file */
                    BKNI_Memcpy(&drm_bin_file_buff[file_size_from_header - DRM_HASH_SIZE], hash_value, DRM_HASH_SIZE);
                }

                /*********************/
                /*  Encrypt DRM data */
                /*********************/
                BDBG_MSG(("Before encryption buf[index 256 ... 259] = 0x%02x 0x%02x 0x%02x 0x%02x ...\n",
                          drm_bin_file_buff[256], drm_bin_file_buff[257], drm_bin_file_buff[258], drm_bin_file_buff[259]));
                if(DRM_SecureStore_BufferOperation(&drm_bin_file_buff[256],
                                                   file_size_from_header - DRM_PADDED_HASH_SIZE - 256,
                                                   &drm_bin_file_buff[256],
                                                   DrmDestinationType_eInPlace,
                                                   DrmCryptoOperation_eEncrypt) != Drm_Success)
                {
                    BDBG_ERR( ("%s - Error binding DRM data (line:%u)", BSTD_FUNCTION, __LINE__));
                    rc = Drm_InitErr;
                    goto ErrorExit;
                }

                BDBG_MSG(("After encryption buf[index 256 ... 259] = 0x%02x 0x%02x 0x%02x 0x%02x ...\n",
                          drm_bin_file_buff[256], drm_bin_file_buff[257], drm_bin_file_buff[258], drm_bin_file_buff[259]));

                /**************************************************************************************************/
                /* Overwrite DRM region                                                                           */
                /**************************************************************************************************/
                rc = DRM_KeyRegion_P_WriteDrmRegion(drm_bin_file_buff);
                if(rc != Drm_Success)
                {
                    BDBG_ERR(("%s - Error overwriting DRM data", BSTD_FUNCTION));
                    rc = Drm_InitErr;
                    goto ErrorExit;
                }
                else{
                    BDBG_MSG(("%s - DRM binary data successfully overwritten", BSTD_FUNCTION));
                }

            } /* end of else (bound drm bin file NOT detected) */

        /*
         * If we did not detected BOUND DRM data, we just encrypted it above.
         * Therefore decrypt it and verify hash.
         * Otherwise it has already been decrypted & verified during the ReadDrmRegionData operation
         * */
        if(bBoundFileRetrieved == false)
        {
            rc = DRM_KeyRegion_P_ValidateDrmData(drm_bin_file_buff, true);
            if(rc != Drm_Success)
            {
                BDBG_ERR(("%s - Error validating DRM binary data from (%s)", BSTD_FUNCTION, path));
                goto ErrorExit;
            }
            else{
                BDBG_MSG(("%s - Valid DRM binary data read from (%s)", BSTD_FUNCTION, path));
            }
        }

        BDBG_MSG(("After decryption buf[index 256 ... 259] = 0x%02x 0x%02x 0x%02x 0x%02x ...\n",
                  drm_bin_file_buff[256], drm_bin_file_buff[257], drm_bin_file_buff[258], drm_bin_file_buff[259]));


    } /* end of 'DrmKeyProvisioningType_eBrcm' */

 ErrorExit:
    if((drm_region_ok == false))
    {
        BDBG_ERR(("*******************************************************************************"));
        BDBG_ERR(("*******************************************************************************"));
        BDBG_ERR(("%s - No valid DRM binary data in DRM region", BSTD_FUNCTION));
        BDBG_ERR(("*******************************************************************************"));
        BDBG_ERR(("*******************************************************************************\n\n\n"));
        DRM_KeyRegion_UnInit();
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyRegion_UnInit
 **
 ** DESCRIPTION:
 **   Clear the key region data from memory
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 ******************************************************************************/
DrmRC DRM_KeyRegion_UnInit(void)
{
    /* Destroy the memory that contains the DRM binary file */
    BDBG_MSG(("%s - Freeing memory that contains the DRM binary file", BSTD_FUNCTION));
    if(drm_bin_file_buff != NULL)
    {
        NEXUS_Memory_Free(drm_bin_file_buff);
        drm_bin_file_buff = NULL;
    }

    if(pOffsetStruct != NULL)
    {
        BKNI_Free(pOffsetStruct);
        pOffsetStruct = NULL;
    }

    if(mutex != 0){
        BKNI_DestroyMutex(mutex);
        mutex = 0;
    }

    return Drm_Success;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyRegion_P_GetDrmRegionPath
 **
******************************************************************************/
static DrmRC DRM_KeyRegion_P_GetDrmRegionPath(char *path_string)
{
    DrmRC rc = Drm_Success;
    char *tmp_buf = NULL;

    BDBG_MSG(("%s - Fetching DRM path", BSTD_FUNCTION));

    if(strlen(generic_drm_filepath) != 0)
    {
        BKNI_Memcpy(path_string, generic_drm_filepath, strlen(generic_drm_filepath));
        BDBG_MSG(("%s - Custom path '%s' retrieved (PLAT)", BSTD_FUNCTION, path_string));
    }
    else
    {
        tmp_buf = bdrm_get_drm_bin_file_path();

        if(tmp_buf == NULL)
        {
            BDBG_ERR(("%s - bdrm_get_drm_bin_file_path returned NULL. Cannot locate drm.bin", BSTD_FUNCTION));
            rc = Drm_NotFound;
        }
        else
        {
            BKNI_Memcpy(generic_drm_filepath, tmp_buf, strlen(tmp_buf));
            BKNI_Memcpy(path_string, tmp_buf, strlen(tmp_buf));
            BDBG_MSG(("%s - Default rootfs path '%s' retrieved ***********************************", BSTD_FUNCTION, tmp_buf));
        }
    }
    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyRegion_P_ReadDrmRegionData
 **
 ** DESCRIPTION:
 **   Retrieve DRM data from rootfs
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 **
 ******************************************************************************/
static DrmRC DRM_KeyRegion_P_ReadDrmRegionData(char *path, uint8_t* pBuf)
{
    DrmRC  rc = Drm_Success;
    int    fd = -1;
    size_t nb = 0;
    bool bound_drm_file_from_flash = false;

    BDBG_MSG(("%s - Entering", BSTD_FUNCTION));

    if(pBuf == NULL)
    {
        BDBG_ERR(("%s - Invalid 'pBuf' parameter passed to function", BSTD_FUNCTION));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    if(path == NULL)
    {
        BDBG_ERR(("%s - Invalid 'path' parameter passed to function", BSTD_FUNCTION));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    if(strlen(generic_drm_filepath) != 0)
    {
        fd = open(path, O_RDONLY|O_SYNC, 0444);
        if (fd < 0)
        {
            BDBG_ERR(("%s - Error opening '%s'", BSTD_FUNCTION, path));
            rc = Drm_FileErr;
            goto ErrorExit;
        }

        BDBG_MSG(("%s - '%s' opened (PLAT)", BSTD_FUNCTION, path));
    }
    else
    {
        BDBG_ERR(("%s - generic_drm_filepath is empty. Cannot locate drm.bin", BSTD_FUNCTION));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    /* Read header */
    nb = read(fd, pBuf, DRM_BIN_FILE_HEADER_SIZE);
    if((nb == (size_t)(-1)) || (nb != DRM_BIN_FILE_HEADER_SIZE) )
    {
        BDBG_ERR(("%s - Error reading header from '%s' (number of bytes read = '%u')", BSTD_FUNCTION, path, (unsigned int)nb));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    /* Copy DRM bin file header into structure */
    BKNI_Memcpy((uint8_t*)&drm_bin_file_header, pBuf, sizeof(drm_header_t));

    /* Determine bin file size from header and read rest */
    file_size_from_header = GET_UINT32_FROM_BUF(drm_bin_file_header.bin_file_size);
    BDBG_MSG(("%s - drm_bin_file_header.bin_file_size == '%u'", BSTD_FUNCTION, file_size_from_header));

    /* Test for header size > 192k */
    if( file_size_from_header & 0xFFFC0000 )
    {
        BDBG_ERR(("%s - Invalid 'bin file size' form header detected (0x%08x)", BSTD_FUNCTION, file_size_from_header));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    /**************************************************************/
    /* Case where a DRM bin file of non-standard size was flashed */
    /* Wipe drm_bin_file_buf restart with new detected            */
    /* 'file_size_from_header' value                              */
    /**************************************************************/
    if( file_size_from_header > DRM_BIN_FILE_SIZE )
    {
        if(drm_bin_file_buff != NULL)
        {
            BDBG_MSG(("%s - Freeing and reallocating '%u' bytes for DRM bin file (%p)", BSTD_FUNCTION, file_size_from_header, drm_bin_file_buff));
            NEXUS_Memory_Free(drm_bin_file_buff);
            drm_bin_file_buff = NULL;

            if(DRM_Common_MemoryAllocate(&drm_bin_file_buff, file_size_from_header) != Drm_Success)
            {
                BDBG_ERR(("%s - Error allocating '%u' bytes for DRM bin file", BSTD_FUNCTION, file_size_from_header));
                rc = Drm_InitErr;
                goto ErrorExit;
            }

            /* Clear buffer */
            BKNI_Memset(drm_bin_file_buff, 0x00, file_size_from_header);
            BKNI_Memcpy(drm_bin_file_buff, (uint8_t*)&drm_bin_file_header, sizeof(drm_header_t));
            BDBG_MSG(("%s - DRM bin file (%p), file_size_from_header = %u", BSTD_FUNCTION, drm_bin_file_buff, file_size_from_header));
        }
    }

    /* Determine if file is bound or not */
    if( (BKNI_Memcmp(&drm_bin_file_buff[BINDING_MARKER_OFFSET    ], drm_binding_obj.devIdA, 8) == 0) ||
        (BKNI_Memcmp(&drm_bin_file_buff[BINDING_MARKER_OFFSET + 8], drm_binding_obj.devIdB, 8) == 0) )
    {
        BDBG_MSG(("%s - DRM binary data is BOUND", BSTD_FUNCTION));
        bound_drm_file_from_flash = true;
    }
    else
    {
        BDBG_MSG(("%s - DRM binary data is NOT BOUND", BSTD_FUNCTION));
        bound_drm_file_from_flash = false;
    }

    /* Use cookie to determine the drm_bin type */
    gCookie = GET_UINT32_FROM_BUF(drm_bin_file_header.cookie);
    BDBG_MSG(("%s - gCookie = '0x%08x'", BSTD_FUNCTION, gCookie));

    rc = DRM_KeyRegion_P_ValidateCookie(DRM_UNKNOWN);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Cookie is not supported.", BSTD_FUNCTION));
        rc = Drm_InvalidStateErr;
        goto ErrorExit;
    }

    if(gCookie == DRM_BIN_FILE_COOKIE_CIP_STANDALONE)
    {
        BDBG_ERR(("%s - CI+ is not supported.", BSTD_FUNCTION));
        rc = Drm_InvalidStateErr;
        goto ErrorExit;
    }
    else if(gCookie == DRM_BIN_FILE_COOKIE_WV_STANDALONE)
    {
        BDBG_WRN(("%s - Copying standalone WV data to offset DRM_BIN_FILE_HEADER_SIZE", BSTD_FUNCTION));

        if( (file_size_from_header > DRM_BIN_FILE_HEADER_SIZE) && (file_size_from_header < 0xFFFC0000) )
        {
            nb = read(fd, &pBuf[DRM_BIN_FILE_HEADER_SIZE], (file_size_from_header - DRM_BIN_FILE_HEADER_SIZE) );
            if((nb == (size_t)(-1)) || (nb != (file_size_from_header - DRM_BIN_FILE_HEADER_SIZE)) )
            {
                BDBG_ERR(("%s - Error reading '%s' (number of bytes read = '%u') in standalone WV", BSTD_FUNCTION, path, (unsigned int)nb));
                rc = Drm_FileErr;
                goto ErrorExit;
            }
        }
    }
    else
    {
        /* Copy rest of data to just after the header  */
        if( (file_size_from_header > DRM_BIN_FILE_HEADER_SIZE) && (file_size_from_header < 0xFFFC0000) )
        {
            nb = read(fd, &pBuf[DRM_BIN_FILE_HEADER_SIZE], (file_size_from_header - DRM_BIN_FILE_HEADER_SIZE) );
            if((nb == (size_t)(-1)) || (nb != (file_size_from_header - DRM_BIN_FILE_HEADER_SIZE)) )
            {
                BDBG_ERR(("%s - Error reading '%s' (number of bytes read = '%u')", BSTD_FUNCTION, path, (unsigned int)nb));
                rc = Drm_FileErr;
                goto ErrorExit;
            }
        }
        /***************************************************************************************
         * If we're dealing with the Bound DRM bin file, tell                                  *
         * the Validation API to decrypt the content first before verifying the Sha1           *
         ***************************************************************************************/
        if(bound_drm_file_from_flash == true){
            rc = DRM_KeyRegion_P_ValidateDrmData(drm_bin_file_buff, true);
        }
        else{
            rc = DRM_KeyRegion_P_ValidateDrmData(drm_bin_file_buff, false);
        }
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error validating DRM binary data from (%s) bound_drm_file_from_flash = %u", BSTD_FUNCTION, path, bound_drm_file_from_flash));
            goto ErrorExit;
        }
        else{
            BDBG_MSG(("%s - Valid DRM binary data read from (%s) bound_drm_file_from_flash = %u", BSTD_FUNCTION, path, bound_drm_file_from_flash));
        }

        if((gCookie == DRM_BIN_FILE_COOKIE_SELF_OFFSET_AWARE) || (gCookie == DRM_BIN_FILE_COOKIE_SECURE_SW_RSA_V2))
        {
            BDBG_MSG(("%s - Processing offset header immediately after main header, file_size_from_header = %u", BSTD_FUNCTION, file_size_from_header));
            bDrmBinFileSelfAware = true;
            rc = DRM_KeyRegion_P_ProcessSelfOffsetAware();
            if(rc != Drm_Success)
            {
                BDBG_ERR(("%s - Error processing offset header", BSTD_FUNCTION));
                goto ErrorExit;
            }
        }
    }

 ErrorExit:
    BDBG_MSG(("%s - Exiting", BSTD_FUNCTION));
    if(fd >= 0)
    {
        close(fd);
    }

    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_KeyRegion_P_ValidateDrmData
 **
 *****************************************************************************/
static DrmRC DRM_KeyRegion_P_ValidateDrmData(uint8_t* pBuf, bool decrypt_first)
{
    DrmRC rc = Drm_Success;
    uint8_t hash_value[DRM_HASH_SIZE] = {0x00};

    BDBG_MSG(("%s - Entered function.  Bin file size from header = '%u' (0x%08x)", BSTD_FUNCTION, file_size_from_header, file_size_from_header));

    if(pBuf == NULL)
    {
        BDBG_ERR(("%s - NULL 'pBuf' parameter passed to function", BSTD_FUNCTION));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    DRM_MSG_PRINT_BUF("pBuf[256]", &pBuf[256], 16);
    DRM_MSG_PRINT_BUF("pBuf[End-hash]", &pBuf[file_size_from_header-DRM_HASH_SIZE], 20);

    if(decrypt_first == true)
    {
        BDBG_MSG( ("%s - Decrypt first....", BSTD_FUNCTION));
        rc = DRM_SecureStore_BufferOperation(&drm_bin_file_buff[256],
                                             file_size_from_header - DRM_PADDED_HASH_SIZE - 256,
                                             &drm_bin_file_buff[256],
                                             DrmDestinationType_eInPlace,
                                             DrmCryptoOperation_eDecrypt);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error decrypting DRM binary data ************************************", BSTD_FUNCTION));
            goto ErrorExit;
        }
    }

    BDBG_MSG( ("%s - Calculating Sha1 ....", BSTD_FUNCTION));

    /* Calculate SHA-1 hash value*/
    rc = DRM_Common_SwSha1(pBuf, hash_value,  file_size_from_header - DRM_HASH_SIZE);
    if(rc != Drm_Success)
    {
        BDBG_ERR( ("%s - Error calculating SHA-1 from 0 to '%u - DRM_HASH_SIZE'", BSTD_FUNCTION, file_size_from_header));
        goto ErrorExit;
    }

    DRM_MSG_PRINT_BUF("hash_value", hash_value, 20);

    /* Compare hash from byte 0 to (end of file size - 20 bytes) */
    if(BKNI_Memcmp(hash_value, &pBuf[file_size_from_header - DRM_HASH_SIZE], (size_t)DRM_HASH_SIZE) != 0)
    {
        BDBG_ERR(("%s - Hash ** MISMATCH ** (0 to (%u-DRM_HASH_SIZE))", BSTD_FUNCTION, file_size_from_header));
        rc = Drm_RegionHashErr;
        goto ErrorExit;
    }
    else{
        BDBG_MSG(("%s - Hash MATCH !!!", BSTD_FUNCTION));
    }

 ErrorExit:
    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_KeyRegion_P_GetHeaderInfo(
 **
 ** DESCRIPTION:
 **   Get crypto info from DRM bin file header
 **
 ** PARAMETERS:
 **   type    - type of data to retrieve (i.e. High, Low, CKS, etc...)
 **   pdata   - pointer to data
 **
 ** RETURNS:
 **   Drm_Success when the operation is succesful or an error.
 **
 ******************************************************************************/
DrmRC DRM_KeyRegion_P_GetHeaderInfo(
    DrmTypeFlash type,
    uint8_t* pdata)
{
    DrmRC rc = Drm_Success;

    if (! pdata || type>=DrmTypeFlash_eMax) return Drm_KeyRegionErr;

    switch(type)
    {
    case DrmTypeFlash_eCustKeySelect:
        BKNI_Memcpy(pdata, &(drm_bin_file_header.cust_key_select), 1);
        BDBG_MSG(("CUSTOMER KEY SELECT: 0x%02x ********************", drm_bin_file_header.cust_key_select));
        break;

    case DrmTypeFlash_eKeyVarLow:
        BKNI_Memcpy(pdata, &(drm_bin_file_header.key_var_low), 1);
        BDBG_MSG(("KEY VAR LOW: 0x%02x ********************", drm_bin_file_header.key_var_low));
        break;

    case DrmTypeFlash_eKeyVarHigh:
        BKNI_Memcpy(pdata, &(drm_bin_file_header.key_var_high), 1);
        BDBG_MSG(("KEY VAR HIGH: 0x%02x ********************", drm_bin_file_header.key_var_high));
        break;

    case DrmTypeFlash_eProcIn1Data:
        BKNI_Memcpy(pdata, drm_bin_file_header.proc_in1, 16);
        BDBG_MSG(("PROC 1: %02x %02x %02x %02x %02x %02x %02x %02x     %02x %02x %02x %02x %02x %02x %02x %02x \n",
                  pdata[0],pdata[1],pdata[2],pdata[3], pdata[4],pdata[5],pdata[6],pdata[7],
                  pdata[8],pdata[9],pdata[10],pdata[11], pdata[12],pdata[13],pdata[14],pdata[15]));
        break;

    case DrmTypeFlash_eProcIn2Data:
        BKNI_Memcpy(pdata, drm_bin_file_header.proc_in2, 16);
        BDBG_MSG(("PROC 2: %02x %02x %02x %02x %02x %02x %02x %02x     %02x %02x %02x %02x %02x %02x %02x %02x \n",
                  pdata[0],pdata[1],pdata[2],pdata[3], pdata[4],pdata[5],pdata[6],pdata[7],
                  pdata[8],pdata[9],pdata[10],pdata[11], pdata[12],pdata[13],pdata[14],pdata[15]));

        break;

    case DrmTypeFlash_eProcIn3Data:
        BKNI_Memcpy(pdata, drm_bin_file_header.proc_in3, 16);
        BDBG_MSG(("PROC 3: %02x %02x %02x %02x %02x %02x %02x %02x     %02x %02x %02x %02x %02x %02x %02x %02x ",
                  pdata[0],pdata[1],pdata[2],pdata[3], pdata[4],pdata[5],pdata[6],pdata[7],
                  pdata[8],pdata[9],pdata[10],pdata[11], pdata[12],pdata[13],pdata[14],pdata[15]));

        break;

    default:
        BDBG_ERR(("%s - Error, invalid case in switch", BSTD_FUNCTION));
        rc = Drm_KeyRegionErr;
        break;
    }

    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyRegion_Read
 **
 ** DESCRIPTION:
 **   Extract key region data
 **
 ** PARAMETERS:
 **   pdata [out] - Pointer to output buffer
 **   offset [in] - Starting offset within static drm_bin_file_buff
 **   nbytes [in] - Length of data
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 ******************************************************************************/
DrmRC DRM_KeyRegion_Read(
    uint8_t* pdata,
    uint32_t offset,
    uint32_t nbytes)
{
    DrmRC rc = Drm_Success;

    if(drm_bin_file_buff == NULL)
    {
        BDBG_ERR(("%s - Error, invalid key region state", BSTD_FUNCTION));
        rc = Drm_KeyRegionErr;
        goto kr_read_err;
    }

    if((offset + nbytes) > file_size_from_header)
    {
        BDBG_ERR(("%s - Error, invalid offset (0x%08x + 0x%08x > file_size_from_header(0x%08x))", BSTD_FUNCTION, offset, nbytes, file_size_from_header));
        rc = Drm_KeyRegionErr;
        goto kr_read_err;
    }

    BDBG_MSG(("%s (0x%08x) - %02x %02x %02x %02x ...\n", BSTD_FUNCTION, offset,
              drm_bin_file_buff[offset], drm_bin_file_buff[offset+1], drm_bin_file_buff[offset+2], drm_bin_file_buff[offset+3]));

    BKNI_Memcpy(pdata, &drm_bin_file_buff[offset], nbytes);

 kr_read_err:
    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyRegion_SetCustDrmFilePath
 **
 ** DESCRIPTION:
 **   Set static char generic_drm_filepath[256]
 **
 ** PARAMETERS:
 **   custom_drm_file_path [in] - New path for drm bin file
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 ******************************************************************************/
DrmRC DRM_KeyRegion_SetCustDrmFilePath(
    char* custom_drm_file_path)
{
    DrmRC rc = Drm_Success;
    BKNI_Memset(generic_drm_filepath, 0x00, sizeof(generic_drm_filepath));

    if(strlen(custom_drm_file_path) > (sizeof(generic_drm_filepath)-1) ){
        BDBG_ERR(("%s - Custom DRM bin filepath length '%u' exceeds max length", BSTD_FUNCTION, (unsigned int)strlen(custom_drm_file_path)));
        rc = Drm_InvalidParameter;
    }
    else{
        BKNI_Memcpy(generic_drm_filepath, custom_drm_file_path, strlen(custom_drm_file_path));
        BDBG_MSG(("%s - Custom DRM bin filepath '%s' set", BSTD_FUNCTION, custom_drm_file_path));
    }
    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_KeyRegion_GetKeyData
 **
 ** DESCRIPTION:
 **   Fetch a DRM module's specific DRM data
 **
 ** PARAMETERS:
 **   drm_type - Enum from the 'drm_types_e' (i.e. DRM_NETFLIX)
 **   pBuf     - The caller should cast the DRM specific data structure
 **              (found in drm_metadata.h) as a (uint8_t *)&struct and pass it
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_KeyRegion_GetKeyData(drm_types_e drm_type, uint8_t *pBuf)
{
    BKNI_AcquireMutex(mutex);
    DrmRC rc = Drm_Success;
    uint8_t *ptr_to_drm_data_struct = NULL;
    uint32_t size_of_data_struct = 0;
    uint32_t offset = 0;
    drm_region_t drm_region_header;
    uint32_t DRM_DataType = 0;
    uint32_t current_drm_data_size = 0;
    uint32_t tmp_size = 0;

    drm_nf_data_t       *netflix_data_struct = NULL;
    drm_wmdrm_pd_data_t *wmdrm_data_struct = NULL;
    drm_wv_data_t       *wv_data_struct = NULL;
    drm_wvcenc_data_t   *wvcenc_data_struct = NULL;
    drm_dtcp_ip_data_t  *dtcp_ip_data_struct = NULL;
    drm_ocap_data_t     *ocap_data_struct = NULL;
    drm_hdcp_rx_data_t  *hdcp_rx_struct = NULL;
    drm_hdcp_tx_data_t  *hdcp_tx_struct = NULL;

    BDBG_MSG(("%s - Entering function", BSTD_FUNCTION));

    /* Sanity check */
    rc = DRM_KeyRegion_P_ValidateCookie(drm_type);
    if(rc != Drm_Success){
        goto ErrorExit;
    }

    /**********************************************************************************/
    /* Step 1. Fetch the DRM region header info for the serial number(procInForKey3) */
    /**********************************************************************************/
    switch(drm_type)
    {
    case DRM_WMDRM_PD:
        size_of_data_struct = sizeof(drm_wmdrm_pd_data_t);
        offset = DRM_WMDRM_PD_OFFSET + DRM_FIRST_REGION_OFFSET;
        break;

    case DRM_NETFLIX:
        size_of_data_struct = sizeof(drm_nf_data_t);
        offset = DRM_NETFLIX_OFFSET + DRM_FIRST_REGION_OFFSET;
        break;

    case DRM_WIDEVINE:
        size_of_data_struct = sizeof(drm_wv_data_t);
        if(wv_standalone_detected == false)
        {
            offset = DRM_WIDEVINE_OFFSET + DRM_FIRST_REGION_OFFSET;
        }
        else
        {
            BDBG_MSG(("%s - OVERWRITING INTERNAL WV OFFSET *************(%u)", BSTD_FUNCTION, (DRM_FIRST_REGION_OFFSET)));
            offset = DRM_FIRST_REGION_OFFSET;
        }
        break;

    case DRM_WIDEVINE_CENC:
        size_of_data_struct = sizeof(drm_wvcenc_data_t);
        break;

    case DRM_DTCP_IP:
        size_of_data_struct = sizeof(drm_dtcp_ip_data_t);
        offset = DRM_DTCP_IP_OFFSET + DRM_FIRST_REGION_OFFSET;
        break;

    case DRM_OCAP:
        size_of_data_struct = sizeof(drm_ocap_data_t);
        offset = DRM_OCAP_OFFSET + DRM_FIRST_REGION_OFFSET;
        break;

    case DRM_HDCP_RX: /* Offset doesn't matter since HDCP_xx was added AFTER SelfOffsetAware bin file support */
        size_of_data_struct = sizeof(drm_hdcp_rx_data_t);
        offset = DRM_FIRST_REGION_OFFSET;
        break;

    case DRM_HDCP_TX: /* Offset doesn't matter since HDCP_xx was added AFTER SelfOffsetAware bin file support */
        size_of_data_struct = sizeof(drm_hdcp_tx_data_t);
        offset = DRM_FIRST_REGION_OFFSET;
        break;

    default:
        BDBG_ERR(("%s - Error switch case (0x%08x)", BSTD_FUNCTION, drm_type));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    /*****************************************************************************/
    /* IF THE DRM BIN FILE IS SELF-OFFSET AWARE, OVERWRITE THE 'offset' VARIABLE */
    /*****************************************************************************/
    if(bDrmBinFileSelfAware == true)
    {
        rc = DRM_KeyRegion_P_SetSelfAwareOffset(&offset, drm_type);
        if(rc != Drm_Success)goto ErrorExit;
    }

    /* Read the DRM region header */
    rc = DRM_KeyRegion_Read((uint8_t*)&drm_region_header, offset, sizeof(drm_region_t));
    if(rc != Drm_Success)goto ErrorExit;

    /* Fetch High, Low, CKS */
    rc = DRM_KeyRegion_ReadKey2Info(&keyInfo);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error loading key2 information", BSTD_FUNCTION));
        goto ErrorExit;
    }

    /* Overwrite ProcIns */
    DRM_DataType = GET_UINT32_FROM_BUF((uint8_t*)&drm_region_header.DRM_type);
    BDBG_MSG(("%s - Drm REGION type = 0x%08x (0x%08x).", BSTD_FUNCTION, DRM_DataType, drm_region_header.DRM_type));

    BKNI_Memcpy(keyInfo.procInForKey3, drm_region_header.SerialNumber, DRM_SERIAL_NUMBER_LENGTH);
    DRM_MSG_PRINT_BUF("keyInfo.procInForKey3", keyInfo.procInForKey3, DRM_SERIAL_NUMBER_LENGTH);

    /**********************************************************************************/
    /* Step 2. Read the specific DRM type blob and decrypt using RPK (procInForKey4). */
    /*   Blobs follow region headers by 'DRM_FIRST_REGION_OFFSET' bytes.              */
    /**********************************************************************************/

    offset += DRM_FIRST_REGION_OFFSET;

    /* Allocate a continuous physical address buffer starting from the pointer to the current data type structure */
    BDBG_MSG(("%s - size_of_data_struct = %u", BSTD_FUNCTION, size_of_data_struct));

    if(DRM_Common_MemoryAllocate(&ptr_to_drm_data_struct, size_of_data_struct) != Drm_Success)
    {
        BDBG_ERR(("%s - Error allocating '%u' bytes for current data struct", BSTD_FUNCTION, size_of_data_struct));
        rc = Drm_InitErr;
        goto ErrorExit;
    }

    rc = DRM_KeyRegion_Read(ptr_to_drm_data_struct, offset, size_of_data_struct);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error reading data from offset = 0x%08x", BSTD_FUNCTION, offset));
        goto ErrorExit;
    }

    DRM_DataType = GET_UINT32_FROM_BUF((uint8_t*)&drm_region_header.DRM_type);
    BDBG_MSG(("%s - Drm DATA type = 0x%02x%02x%02x%02x.", BSTD_FUNCTION,
              ptr_to_drm_data_struct[0], ptr_to_drm_data_struct[1], ptr_to_drm_data_struct[2], ptr_to_drm_data_struct[3]));

    switch(drm_type)
    {
    case DRM_WMDRM_PD:
        wmdrm_data_struct = (drm_wmdrm_pd_data_t *)ptr_to_drm_data_struct;
        BKNI_Memcpy(keyInfo.procInForKey4, wmdrm_data_struct->key, DRM_REGION_PRIV_KEY_SIZE);
        break;

    case DRM_NETFLIX:
        netflix_data_struct = (drm_nf_data_t *)ptr_to_drm_data_struct;
        BKNI_Memcpy(keyInfo.procInForKey4, netflix_data_struct->key, DRM_REGION_PRIV_KEY_SIZE);
        break;

    case DRM_WIDEVINE:
        wv_data_struct = (drm_wv_data_t *)ptr_to_drm_data_struct;
        BKNI_Memcpy(keyInfo.procInForKey4, wv_data_struct->key, DRM_REGION_PRIV_KEY_SIZE);
        break;

    case DRM_WIDEVINE_CENC:
        wvcenc_data_struct = (drm_wvcenc_data_t *)ptr_to_drm_data_struct;
        BKNI_Memcpy(keyInfo.procInForKey4, wvcenc_data_struct->rpk, DRM_REGION_PRIV_KEY_SIZE);
        break;

    case DRM_DTCP_IP:
        dtcp_ip_data_struct = (drm_dtcp_ip_data_t *)ptr_to_drm_data_struct;
        BKNI_Memcpy(keyInfo.procInForKey4, dtcp_ip_data_struct->key, DRM_REGION_PRIV_KEY_SIZE);
        break;

    case DRM_OCAP:
        ocap_data_struct = (drm_ocap_data_t  *)ptr_to_drm_data_struct;
        BKNI_Memcpy(keyInfo.procInForKey3, ocap_data_struct->serial_number, DRM_SERIAL_NUMBER_LENGTH);
        DRM_MSG_PRINT_BUF("keyInfo.procInForKey3 (overwritten)", keyInfo.procInForKey3, DRM_SERIAL_NUMBER_LENGTH);
        BKNI_Memcpy(keyInfo.procInForKey4, ocap_data_struct->rpk, DRM_REGION_PRIV_KEY_SIZE);
        break;

    case DRM_HDCP_RX:
        hdcp_rx_struct = (drm_hdcp_rx_data_t *)ptr_to_drm_data_struct;
        BKNI_Memcpy(keyInfo.procInForKey4, hdcp_rx_struct->rpk, DRM_REGION_PRIV_KEY_SIZE);
        break;

    case DRM_HDCP_TX:
        hdcp_tx_struct = (drm_hdcp_tx_data_t *)ptr_to_drm_data_struct;
        BKNI_Memcpy(keyInfo.procInForKey4, hdcp_tx_struct->rpk, DRM_REGION_PRIV_KEY_SIZE);
        break;

    /* coverity[dead_error_begin] */
    default:
        BDBG_ERR(("%s - Invalid switch RPK case", BSTD_FUNCTION));
        break;
    }

    DRM_MSG_PRINT_BUF("keyInfo.procInForKey4", keyInfo.procInForKey4, DRM_REGION_PRIV_KEY_SIZE);

    /**********************************************************************************/
    /* Step 3. Decrypt the data specific to the DRM type                              */
    /*         THIS SECTION IS VERY SPECIFIC TO THE STRUCTURE OF THE DRM DATA BEING   */
    /*         DECRYPTED CONSULT THE DRM METADATA TO SEE THE SPECIFICS.               */
    /**********************************************************************************/

#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_SecurityOperation opType = NEXUS_SecurityOperation_eDecrypt;
#else
    NEXUS_CryptographicOperation opType = NEXUS_CryptographicOperation_eDecrypt;
#endif

    switch(drm_type)
    {
    case DRM_WMDRM_PD:
        current_drm_data_size = DRM_WMDRM_PD_ENC_KEY_SIZE*2;
        rc = _Crypto_AesEcb(wmdrm_data_struct->group_priv_key, current_drm_data_size, opType);
        if(rc != Drm_Success) goto ErrorExit;
        BKNI_Memcpy(pBuf, (uint8_t *)wmdrm_data_struct, size_of_data_struct);
        break;

    case DRM_NETFLIX:
        current_drm_data_size = DRM_NETFLIX_ENC_KPE_SIZE + DRM_NETFLIX_ENC_KPH_SIZE + DRM_NETFLIX_ENC_ESN_SIZE;
        rc = _Crypto_AesEcb(netflix_data_struct->kpe, current_drm_data_size, opType);
        if(rc != Drm_Success) goto ErrorExit;
        BKNI_Memcpy(pBuf, (uint8_t *)netflix_data_struct, size_of_data_struct);
        break;

    case DRM_WIDEVINE:
        current_drm_data_size = DRM_WIDEVINE_ENC_KEYID_SIZE + DRM_WIDEVINE_DEVICE_ID_SIZE;
        rc = _Crypto_AesEcb(wv_data_struct->wvkeyID, current_drm_data_size, opType);
        if(rc != Drm_Success) goto ErrorExit;
        BKNI_Memcpy(pBuf, (uint8_t *)wv_data_struct, size_of_data_struct);
        break;

    case DRM_WIDEVINE_CENC:
        current_drm_data_size = sizeof(drm_wvcenc_data_t) - \
            (2*DRM_REGION_PRIV_KEY_SIZE +
             sizeof(wvcenc_data_struct->DRM_DataType) +  \
             sizeof(wvcenc_data_struct->StructureSizeInBytes) ) ;
        rc = _Crypto_AesEcb(wvcenc_data_struct->DeviceId, current_drm_data_size, opType);
        if(rc != Drm_Success) goto ErrorExit;
        BKNI_Memcpy(pBuf, (uint8_t *)wvcenc_data_struct, size_of_data_struct);
        break;

    case DRM_DTCP_IP:
        current_drm_data_size = sizeof(drm_dtcp_ip_data_t) \
            - sizeof(dtcp_ip_data_struct->DRM_DataType)  \
            - sizeof(dtcp_ip_data_struct->StructureSizeInBytes)  \
            - sizeof(dtcp_ip_data_struct->key);
        BDBG_MSG(("%s - decrypting '%u' bytes for DTCP-IP data", BSTD_FUNCTION, current_drm_data_size));
        rc = _Crypto_AesEcb(dtcp_ip_data_struct->BaselineFullCert, current_drm_data_size, opType);
        if(rc != Drm_Success) goto ErrorExit;
        BKNI_Memcpy(pBuf, (uint8_t *)dtcp_ip_data_struct, size_of_data_struct);
        break;

    case DRM_OCAP:
        rc = _Crypto_AesEcb(ocap_data_struct->ca, OCAP_PADDED_ENC_MANUFACTURE_CA_SIZE, opType);
        if(rc != Drm_Success) goto ErrorExit;

        rc = _Crypto_AesEcb(ocap_data_struct->root, OCAP_PADDED_ENC_MANUFACTURE_ROOT_SIZE, opType);
        if(rc != Drm_Success) goto ErrorExit;

        rc = _Crypto_AesEcb(ocap_data_struct->cert, OCAP_PADDED_ENC_DEVICE_CERT_SIZE, opType);
        if(rc != Drm_Success) goto ErrorExit;

        rc = _Crypto_AesEcb(ocap_data_struct->key, OCAP_PADDED_ENC_DEVICE_KEY_SIZE, opType);
        if(rc != Drm_Success) goto ErrorExit;

        rc = _Crypto_AesEcb(ocap_data_struct->base, OCAP_PADDED_ENC_DH_BASE, opType);
        if(rc != Drm_Success) goto ErrorExit;

        rc = _Crypto_AesEcb(ocap_data_struct->prime, OCAP_PADDED_ENC_DH_PRIME, opType);
        if(rc != Drm_Success) goto ErrorExit;

        BKNI_Memcpy(pBuf, (uint8_t *)ocap_data_struct, size_of_data_struct);
        break;

    case DRM_HDCP_RX:
        tmp_size = GET_UINT32_FROM_BUF(hdcp_rx_struct->size_of_lc);
        if(tmp_size % 16 != 0){tmp_size += (16-(tmp_size%16));}
        BDBG_MSG(("%s - decrypting HDCP_RX part 1 (%u)", BSTD_FUNCTION, tmp_size));

        rc = _Crypto_AesEcb(hdcp_rx_struct->licensed_constant, tmp_size, opType);
        if(rc != Drm_Success) goto ErrorExit;

        tmp_size = GET_UINT32_FROM_BUF(hdcp_rx_struct->size_of_keyset);
        if(tmp_size % 16 != 0){tmp_size += (16-(tmp_size%16));}
        BDBG_MSG(("%s - decrypting HDCP_RX part 2 (%u)", BSTD_FUNCTION, tmp_size));

        rc = _Crypto_AesEcb(hdcp_rx_struct->rx_id, tmp_size, opType);
        if(rc != Drm_Success) goto ErrorExit;

        BKNI_Memcpy(pBuf, (uint8_t *)hdcp_rx_struct, size_of_data_struct);
        break;

    case DRM_HDCP_TX:
        tmp_size = GET_UINT32_FROM_BUF(hdcp_tx_struct->size_of_lc);
        if(tmp_size % 16 != 0){tmp_size += (16-(tmp_size%16));}
        BDBG_MSG(("%s - decrypting HDCP_TX part 1 (%u)", BSTD_FUNCTION, tmp_size));

        rc = _Crypto_AesEcb(hdcp_tx_struct->licensed_constant, tmp_size, opType);
        if(rc != Drm_Success) goto ErrorExit;

        tmp_size = GET_UINT32_FROM_BUF(hdcp_tx_struct->size_of_keyset);
        if(tmp_size % 16 != 0){tmp_size += (16-(tmp_size%16));}
        BDBG_MSG(("%s - decrypting HDCP_TX part 2 (%u)", BSTD_FUNCTION, tmp_size));

        rc = _Crypto_AesEcb(hdcp_tx_struct->root_public_modulus, tmp_size, opType);
        if(rc != Drm_Success) goto ErrorExit;

        BKNI_Memcpy(pBuf, (uint8_t *)hdcp_tx_struct, size_of_data_struct);
        break;

    /* coverity[dead_error_begin] */
    default:
        BDBG_ERR(("%s - Invalid switch decryption case", BSTD_FUNCTION));
        break;
    }

 ErrorExit:
    if(ptr_to_drm_data_struct != NULL){
        NEXUS_Memory_Free(ptr_to_drm_data_struct);
        ptr_to_drm_data_struct = NULL;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(mutex);
    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   _Crypto_AesEcb
 **
 ** DESCRIPTION:
 **   Do an AES ECB crypto operation on a buffer
 **
 ** PARAMETERS:
 **   pBuf - Buffer to encrypt/decrypt
 **   buf_size - Size of the buffer
 **   op_type - Security operation
 **
 ** RETURNS:
 **   Drm_Success when the operation is succesful or an error.
 **
 ******************************************************************************/
#if (NEXUS_SECURITY_API_VERSION==1)
static DrmRC _Crypto_AesEcb(
    uint8_t *pBuf,
    uint32_t buf_size,
    NEXUS_SecurityOperation op_type)
#else
static DrmRC _Crypto_AesEcb(
    uint8_t *pBuf,
    uint32_t buf_size,
    NEXUS_CryptographicOperation op_type)
#endif

{
    DrmRC rc = Drm_Success;
    uint8_t *dmaBuf = NULL;
    DmaBlockInfo_t dmaBlock;
    DrmCommonOperationStruct_t drmCommonOpStruct;

    BDBG_ASSERT(pBuf != NULL);
    BDBG_ASSERT(buf_size != 0);
    BDBG_MSG(("%s - Entering, decrypting '%u' bytes", BSTD_FUNCTION, buf_size));

    /* Allocate a continuous physical address buffer for DMA */
    if(DRM_Common_MemoryAllocate(&dmaBuf, buf_size) != Drm_Success)
    {
        BDBG_ERR(("%s - Error allocating '%u' bytes", BSTD_FUNCTION, buf_size));
        rc = Drm_InitErr;
        goto ErrorExit;
    }

    /* Copy content to DMA buffer */
    BKNI_Memcpy(dmaBuf, pBuf, buf_size);

    /* Set DMA parameters */
    dmaBlock.pDstData = dmaBuf;
    dmaBlock.pSrcData = dmaBuf;
    dmaBlock.uiDataSize = buf_size;
    dmaBlock.sg_start = true;
    dmaBlock.sg_end = true;

    DRM_Common_GetDefaultStructSettings(&drmCommonOpStruct);
    drmCommonOpStruct.keyConfigSettings.settings.opType    = op_type;
#if (NEXUS_SECURITY_API_VERSION==1)
    drmCommonOpStruct.keyConfigSettings.settings.algType   = NEXUS_SecurityAlgorithm_eAes;
    drmCommonOpStruct.keyConfigSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eEcb;
    drmCommonOpStruct.keyConfigSettings.settings.termMode = NEXUS_SecurityTerminationMode_eClear;
    drmCommonOpStruct.keySrc = CommonCrypto_eCustKey;
#else
    drmCommonOpStruct.keyConfigSettings.settings.algType   = NEXUS_CryptographicAlgorithm_eAes128;
    drmCommonOpStruct.keyConfigSettings.settings.algVariant = NEXUS_CryptographicAlgorithmMode_eEcb;
    drmCommonOpStruct.keyConfigSettings.settings.termMode =  NEXUS_KeySlotTerminationMode_eClear;
    drmCommonOpStruct.keySrc = CommonCrypto_eGlobalKey;
#endif
    drmCommonOpStruct.pKeyLadderInfo = &keyInfo;
    drmCommonOpStruct.pDmaBlock = &dmaBlock;
    drmCommonOpStruct.num_dma_block = 1;

    DRM_MSG_PRINT_BUF("dmaBuf (ENC)", dmaBuf, 16);

    rc = DRM_Common_OperationDma(&drmCommonOpStruct);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error loading key or with M2M DMA operation, examine for previous error messages", BSTD_FUNCTION));
        goto ErrorExit;
    }

    DRM_MSG_PRINT_BUF("dmaBuf (DEC)", dmaBuf, 16);

    /* Copy the result back */
    BKNI_Memcpy(pBuf, dmaBuf, buf_size);

 ErrorExit:
    if(dmaBuf != NULL){
        NEXUS_Memory_Free(dmaBuf);
    }

    BDBG_MSG(("%s - Exiting", BSTD_FUNCTION));
    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_KeyRegion_ReadKey2Info
 **
 ** DESCRIPTION:
 **   Read flash key2 information
 **
 ** PARAMETERS:
 **   pKi - Pointer to structure containing the key data
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_KeyRegion_ReadKey2Info(
    CommonCryptoKeyLadderSettings *pKi)
{
    DrmRC rc = Drm_Success;

    BDBG_ASSERT(pKi != NULL);
    BDBG_MSG(("%s - Entering", BSTD_FUNCTION));

    BDBG_MSG(("%s - DrmKeyProvisioningType_eBrcm case", BSTD_FUNCTION));
    rc = DRM_KeyRegion_P_GetHeaderInfo(DrmTypeFlash_eCustKeySelect, &pKi->custKeySelect);
    if(rc != Drm_Success){
        goto ErrorExit;
    }

    rc = DRM_KeyRegion_P_GetHeaderInfo(DrmTypeFlash_eKeyVarLow,  &pKi->keyVarLow);
    if(rc != Drm_Success){
        goto ErrorExit;
    }

    rc = DRM_KeyRegion_P_GetHeaderInfo(DrmTypeFlash_eKeyVarHigh, &pKi->keyVarHigh);
    if(rc != Drm_Success){
        goto ErrorExit;
    }

    BDBG_MSG(("%s - Retrieving Procs", BSTD_FUNCTION));
    rc = DRM_KeyRegion_P_GetHeaderInfo(DrmTypeFlash_eProcIn1Data, pKi->procInForKey3);
    if(rc != Drm_Success){
        goto ErrorExit;
    }

    rc = DRM_KeyRegion_P_GetHeaderInfo(DrmTypeFlash_eProcIn2Data, pKi->procInForKey4);
    if(rc != Drm_Success){
        goto ErrorExit;
    }

 ErrorExit:
    BDBG_MSG(("%s - Exiting", BSTD_FUNCTION));
    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_KeyRegion_P_ValidateCookie
 **
 ** DESCRIPTION:
 **   Validate if the current bin file supports the desired DRM data type
 **
 ** PARAMETERS:
 **   drm_type
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
static DrmRC DRM_KeyRegion_P_ValidateCookie(drm_types_e drm_type)
{
    DrmRC rc = Drm_Success;
    const uint32_t sanity_check = 0xAAC50000;
    BSTD_UNUSED(drm_type);

    BDBG_MSG(("%s - Entering", BSTD_FUNCTION));

    gCookie = GET_UINT32_FROM_BUF(drm_bin_file_header.cookie);

    BDBG_MSG(("%s - gCookie = '0x%08x'", BSTD_FUNCTION, gCookie));

    /* Sanity check */
    if( (gCookie & sanity_check) != sanity_check)
    {
        BDBG_ERR(("%s - Invalid DRM bin file cookie detected '0x%08x'", BSTD_FUNCTION, gCookie));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    switch(gCookie)
    {
    case DRM_BIN_FILE_COOKIE_WV_STANDALONE:
        wv_standalone_detected = true;
        BDBG_MSG(("%s - 'DRM_BIN_FILE_COOKIE_WV_STANDALONE' detected.  All offsets in metadata and datatypes are valid", BSTD_FUNCTION));
        break;

    case DRM_BIN_FILE_COOKIE_ALL_IN_ONE:
        BDBG_MSG(("%s - 'DRM_BIN_FILE_COOKIE_ALL_IN_ONE' detected.  All offsets in metadata and datatypes are valid", BSTD_FUNCTION));
        break;

    case DRM_BIN_FILE_COOKIE_NONREF:
        nonref_detected = true;
        if(cip_standalone_detected == true)
        {
            split_mode_active = true;
            BDBG_MSG(("%s - SPLIT MODE DETECTED (from non-reference case), set split_mode_active to %u", BSTD_FUNCTION, split_mode_active));
        }
        break;

    case DRM_BIN_FILE_COOKIE_CIP_STANDALONE:
        cip_standalone_detected = true;
        if(nonref_detected == true)
        {
            split_mode_active = true;
            BDBG_MSG(("%s - SPLIT MODE DETECTED (from CI+ standalone case), set split_mode_active to %u", BSTD_FUNCTION, split_mode_active));
        }
        break;

    case DRM_BIN_FILE_COOKIE_CIP_STANDARD:
        BDBG_MSG(("%s - 'DRM_BIN_FILE_COOKIE_CIP_STANDARD' detected.  No offsets in metadata apply", BSTD_FUNCTION));
        break;

    case DRM_BIN_FILE_COOKIE_SELF_OFFSET_AWARE:
        BDBG_MSG(("%s - 'DRM_BIN_FILE_COOKIE_SELF_OFFSET_AWARE' detected.  No offsets in metadata apply", BSTD_FUNCTION));
        break;

    case DRM_BIN_FILE_COOKIE_SECURE_SW_RSA_V2:
        BDBG_MSG(("%s - 'DRM_BIN_FILE_COOKIE_SECURE_SW_RSA_V2' detected.  No offsets in metadata apply", BSTD_FUNCTION));
        break;

    default:
        BDBG_ERR(("%s - Cookie '0x%08x' not supported", BSTD_FUNCTION, gCookie));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

 ErrorExit:
    BDBG_MSG(("%s - Exiting", BSTD_FUNCTION));
    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyRegion_P_WriteDrmRegion
 **
 ** DESCRIPTION:
 **   Copies/Overwrites data from local memory to the non-volatile memory
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 **
 ******************************************************************************/
static DrmRC DRM_KeyRegion_P_WriteDrmRegion(uint8_t* pBuf)
{
    DrmRC  result = Drm_Success;
    FILE   *fptr = NULL;
    size_t nb = 0;

    if(pBuf == NULL)
    {
        BDBG_ERR(("%s - NULL 'pBuf' parameter passed to function", BSTD_FUNCTION));
        result = Drm_KeyRegionErr;
        goto err;
    }

    /* State check */
    if(strlen(generic_drm_filepath) != 0)
    {
        BDBG_MSG(("%s - Overwriting key file (%s) with bound version", BSTD_FUNCTION, generic_drm_filepath));
        fptr = fopen(generic_drm_filepath, "w+b");
        if (fptr == NULL)
        {
            BDBG_ERR(("%s - Error opening '%s' for writing (w+b). Verify file permissions", BSTD_FUNCTION, generic_drm_filepath));
            result = Drm_BindingErr;
            goto err;
        }

        BDBG_MSG(("%s - Opened '%s'", BSTD_FUNCTION, generic_drm_filepath));

        nb = fwrite(pBuf, 1, file_size_from_header, fptr);
        if(nb != file_size_from_header)
        {
            BDBG_ERR(("%s - Error writing to '%s', only '%u' bytes written.", BSTD_FUNCTION, generic_drm_filepath, (unsigned int)nb));
            result = Drm_BindingErr;
            goto err;
        }
    }
    else
    {
        BDBG_ERR(("%s - generic_drm_filepath is empty.  Cannot access drm.bin file", BSTD_FUNCTION));
        result = Drm_BindingErr;
        goto err;
    }

 err:
    if(fptr != NULL)
    {
        fclose(fptr);
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return result;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyRegion_SetKeyProvisionType
 **
 ** DESCRIPTION:
 **   Set the provisioning type for way in which the keys will be fetched.
 **
 ** PARAMETERS:
 **   pType
 **      Type: DrmKeyProvisioningType
 **      Purpose: Set to either (DrmKeyProvisioningType_eUtv or DrmKeyProvisioningType_eBrcm)
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 **
 ******************************************************************************/
DrmRC DRM_KeyRegion_SetKeyProvisionType(DrmKeyProvisioningType pType)
{
    DrmRC drm_rc = Drm_Success;

    BDBG_MSG(("%s - Entered function, setting KP type to '%u' *****\n", BSTD_FUNCTION, pType));

    keyProvisioningType = pType;

    BDBG_MSG(("%s - Exiting function, provisioningType set to '%u'", BSTD_FUNCTION, keyProvisioningType));

    return drm_rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyRegion_GetKeyProvisionType
 **
 ** DESCRIPTION:
 **   Return the provisioning type
 **
 ** PARAMETERS:
 **   pType
 **      Type: DrmKeyProvisioningType
 **      Purpose: Set to either (DrmKeyProvisioningType_eUtv or DrmKeyProvisioningType_eBrcm)
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 **
 ******************************************************************************/
DrmRC DRM_KeyRegion_GetKeyProvisionType(DrmKeyProvisioningType *pType)
{
    DrmRC drm_rc = Drm_Success;

    BDBG_MSG(("%s - Entered function, retrieving provisioningType type '%u'", BSTD_FUNCTION, keyProvisioningType));

    *pType = keyProvisioningType;

    BDBG_MSG(("%s - Exiting function, return type set to '%u'", BSTD_FUNCTION, (*pType) ));

    return drm_rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyRegion_P_ProcessSelfOffsetAware
 **
 ** DESCRIPTION:
 **   Processes the offset header of the DRM bin file
 **
 ** PARAMETERS:
 **   N/A
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 **
 ******************************************************************************/
static DrmRC DRM_KeyRegion_P_ProcessSelfOffsetAware(void)
{
    DrmRC rc = Drm_Success;
    const uint32_t MAX_NUM_DRM_TYPES = 256;
    uint32_t count = 0;
    uint32_t offset_to_dynamic_header = 0;

    BDBG_MSG(("%s - Entered function file_size_from_header = %u", BSTD_FUNCTION, file_size_from_header));

    DRM_MSG_PRINT_BUF("drm_bin_file_buff[256]", &drm_bin_file_buff[256], 16);

    /* Determine number of DRM types in current bin file */
    number_of_drms = GET_UINT32_FROM_BUF(&drm_bin_file_buff[DRM_BIN_FILE_HEADER_SIZE]);
    BDBG_MSG(("%s - Number of DRMs detected = '%u'", BSTD_FUNCTION, number_of_drms));

    /* Sanity check */
    if((number_of_drms == 0) || (number_of_drms > MAX_NUM_DRM_TYPES))
    {
        BDBG_ERR(("%s - Number of DRMs detected is invalid (%u)", BSTD_FUNCTION, number_of_drms));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    offset_to_dynamic_header = DRM_BIN_FILE_HEADER_SIZE+16;

    /* Allocate dynamic memory for offset structure to be used as LUT later */
    pOffsetStruct = (DrmKeyRegionOffsetStruct_t*)BKNI_Malloc(number_of_drms*sizeof(DrmKeyRegionOffsetStruct_t));
    if(pOffsetStruct == NULL)
    {
        BDBG_ERR(("%s - Error allocating Offset structure memory", BSTD_FUNCTION));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    BKNI_Memset(pOffsetStruct, 0x00, number_of_drms*(sizeof(DrmKeyRegionOffsetStruct_t)));

    /* Parse offset header to determine DRM types and their respective offsets */
    for(count = 0; count < number_of_drms; count++)
    {
        pOffsetStruct[count].DrmType = GET_UINT32_FROM_BUF(&drm_bin_file_buff[offset_to_dynamic_header + count*16]);
        pOffsetStruct[count].OffsetRelativeToEOT = GET_UINT32_FROM_BUF(&drm_bin_file_buff[offset_to_dynamic_header + count*16 + 12]);

        BDBG_MSG(("%s - pOffsetStruct[%u].DrmType = '0x%08x'", BSTD_FUNCTION, count, pOffsetStruct[count].DrmType));
        BDBG_MSG(("%s - pOffsetStruct[%u].OffsetRelativeToEOT = '0x%08x'", BSTD_FUNCTION, count, pOffsetStruct[count].OffsetRelativeToEOT));

        /* Sanity checks */
        if(pOffsetStruct[count].DrmType & 0xFFFF0000)
        {
            BDBG_ERR(("%s - Current DRM type detected is invalid (0x%08x)", BSTD_FUNCTION, pOffsetStruct[count].DrmType));
            rc = Drm_FileErr;
            goto ErrorExit;
        }

        if(pOffsetStruct[count].OffsetRelativeToEOT > (file_size_from_header-(DRM_BIN_FILE_HEADER_SIZE + (number_of_drms+1*16))) )
        {
            BDBG_ERR(("%s - Current offset relative to end of table  (0x%08x) for DRM type (0x%08x)", BSTD_FUNCTION, pOffsetStruct[count].OffsetRelativeToEOT , pOffsetStruct[count].DrmType));
            rc = Drm_FileErr;
            goto ErrorExit;
        }
    }

 ErrorExit:
    BDBG_MSG(("%s - Exiting function, file_size_from_header = %u", BSTD_FUNCTION, file_size_from_header));
    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyRegion_P_SetSelfAwareOffset
 **
 ** DESCRIPTION:
 **   Processes the offset header of the DRM bin file
 **
 ** PARAMETERS:
 **   *offset:  pointer to offset value to overwrite
 **   drm_type: type of DRM being looked-up
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 **
 ******************************************************************************/
static DrmRC DRM_KeyRegion_P_SetSelfAwareOffset(uint32_t *offset, drm_types_e drm_type)
{
    DrmRC rc = Drm_Success;
    uint32_t count = 0;

    for(count = 0; count < number_of_drms; count++)
    {
        if(pOffsetStruct[count].DrmType == drm_type)
        {
            (*offset) = pOffsetStruct[count].OffsetRelativeToEOT + DRM_BIN_FILE_HEADER_SIZE+ (16*(number_of_drms+1));
            BDBG_MSG(("%s - DRM type (0x%08x) exists at offset '0x%08x' relative to start of entire file", BSTD_FUNCTION, drm_type, (*offset)));
            break;
        }

        /* Check for error on last possible index */
        if(count == (number_of_drms-1))
        {
            BDBG_ERR(("%s - No DRM type (0x%08x) found", BSTD_FUNCTION, drm_type));
            rc = Drm_FileErr;
            break;
        }
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return rc;
}
