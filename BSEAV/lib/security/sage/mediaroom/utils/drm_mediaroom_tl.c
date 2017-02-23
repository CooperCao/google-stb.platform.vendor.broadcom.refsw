/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"

#include "common_crypto.h"
#include "bcrypt.h"
#include "bcrypt_x509_sw.h"
#include "bcrypt_rsa_sw.h"

#include "drm_mediaroom_tl.h"
#include "drm_mediaroom_cmds.h"
#include "drm_data.h"
#include "drm_metadata_tl.h"
#include "drm_common_swcrypto_types.h"

#include "nexus_memory.h"
#include "nexus_otpmsp.h"

#include "bsagelib_types.h"
#include "sage_srai.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define DEFAULT_MEDIAROOM_DRM_BIN_FILESIZE (64*1024)
#define DRM_MEDIAROOM_OVERWRITE_BIN_FILE (1)

static uint8_t             *mediaroom_drm_bin_file_buff = NULL;
SRAI_ModuleHandle          hMediaroomModule = NULL;
static SRAI_PlatformHandle hMediaroomPlatform = NULL;
static drm_bin_header_t    mediaroom_drm_bin_header = {0};
static BSAGElib_State      mediaroomPlatformStatus = BSAGElib_State_eUninit;
static BCRYPT_Handle       mediaroom_bcrypt_handle = NULL;

DrmRC DRM_Mediaroom_P_TA_Install(char * ta_bin_filename);
DrmRC DRM_Mediaroom_ModuleInit(uint32_t module_id, char * drm_bin_filename,
    BSAGElib_InOutContainer *container, SRAI_ModuleHandle *moduleHandle);

BDBG_MODULE(drm_mediaroom);

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define DUMP_SIZE 32
#define DUMP_MEDIAROOM_DATA(string,data,size) {\
    char tmp[DUMP_SIZE*4]={0};\
    uint32_t i=0, j=0;\
    BDBG_LOG(("%s",string));\
    BDBG_LOG(("--------------------------------"));\
    while( i<size ) {\
        if(i !=0 && i%DUMP_SIZE == 0) {j=0; BDBG_LOG(("%s",tmp));}\
        sprintf(tmp+3*j++," %02x", data[i++]);}\
    BDBG_LOG(("%s",tmp));\
    BDBG_LOG(("--------------------------------"));\
}

DrmRC
DRM_Mediaroom_Initialize(char *drm_bin_file)
{
    DrmRC                   rc = Drm_Success;
    BSAGElib_InOutContainer *container = NULL;
    ChipType_e              chip_type;
    char                    *ta_bin_file_path;
    BCRYPT_STATUS_eCode     eCode = 0;
    BERR_Code               sage_rc = BERR_SUCCESS;

    BDBG_ENTER(DRM_MediaroomTl_Initialize);
    if (!drm_bin_file )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", __FUNCTION__  ));
        goto ErrorExit;
    }

    eCode = BCRYPT_Open(&mediaroom_bcrypt_handle);
    if(eCode == BCRYPT_STATUS_eOK)
    {
        BDBG_MSG(("%s - Opened BCRYPT, mediaroom_bcrypt_handle = '0x%08x' ^^^^^^^", __FUNCTION__, mediaroom_bcrypt_handle));
    }
    else {
        BDBG_ERR(("%s - Failed to open BCRYPT, rc = '%d' ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__, eCode));
        mediaroom_bcrypt_handle = 0;
        rc = Drm_BcryptErr;
        goto ErrorExit;
    }

    chip_type = DRM_Mediaroom_GetChipType();
    if(chip_type == ChipType_eZS)
    {
        ta_bin_file_path = bdrm_get_ta_mediaroom_dev_bin_file_path();
    }
    else
    {
        ta_bin_file_path = bdrm_get_ta_mediaroom_bin_file_path();
    }
    if (ta_bin_file_path == NULL)
    {
        BDBG_ERR (("Mediaroom TA path not specified"));
        goto ErrorExit;
    }
    BDBG_MSG(("%s TA bin file %s ",__FUNCTION__, ta_bin_file_path));

    /* Install Mediaroom TA */
    rc = DRM_Mediaroom_P_TA_Install(ta_bin_file_path);
    if (rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error installing Mediaroom TA %s %d", __FUNCTION__,ta_bin_file_path, rc ));
        goto ErrorExit;
    }

    /* Open Mediaroom Platform */
    if (hMediaroomPlatform == NULL)
    {
        sage_rc = SRAI_Platform_Open(BSAGE_PLATFORM_ID_MEDIAROOM, &mediaroomPlatformStatus, &hMediaroomPlatform);
        if (sage_rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error calling platform_open, ta_bin_file_path: %s", __FUNCTION__, ta_bin_file_path));
            rc = Drm_Err;
            goto ErrorExit;
        }
        if(mediaroomPlatformStatus == BSAGElib_State_eUninit)
        {
            BDBG_WRN(("%s - platform_status == BSAGElib_State_eUninit *******)", __FUNCTION__));
            container = SRAI_Container_Allocate();
            if(container == NULL)
            {
                BDBG_ERR(("%s - Error fetching container", __FUNCTION__));
                rc = Drm_Err;
                goto ErrorExit;
            }
            BDBG_MSG(("%s: allocated container for mediaroom at %p",__FUNCTION__,container));
            /* Initialize Mediaroom Platform */
            sage_rc = SRAI_Platform_Init(hMediaroomPlatform, container);
            if (sage_rc != BERR_SUCCESS)
            {
                BDBG_ERR(("%s: %d - Error calling platform init", __FUNCTION__,__LINE__));
                rc = Drm_Err;
                goto ErrorExit;
            }
        }
    }
    else
    {
        BDBG_WRN(("%s: Mediaroom Platform already initialized *************************", __FUNCTION__));
    }

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    /* Initialize SAGE Mediaroom drm module */
    hMediaroomModule = NULL;
    rc = DRM_Mediaroom_ModuleInit(Mediaroom_ModuleId_eDRM,(char *)drm_bin_file, container, &hMediaroomModule);
    if (rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module (0x%08x)", __FUNCTION__, container->basicOut[0]));
        goto ErrorExit;
    }

ErrorExit:
    if (container != NULL)
    {
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(DRM_MediaroomTl_Initialize);
    return rc;
}

void DRM_Mediaroom_Uninit()
{

    if (hMediaroomModule != NULL)
    {
        SRAI_Module_Uninit(hMediaroomModule);
        hMediaroomModule = NULL;
    }

    if (hMediaroomPlatform)
    {
        SRAI_Platform_Close(hMediaroomPlatform);
        hMediaroomPlatform = NULL;
    }
    SRAI_Platform_UnInstall(BSAGE_PLATFORM_ID_MEDIAROOM);
}

DrmRC DRM_Mediaroom_GetMagic(uint8_t *magic, uint32_t magicLength)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if (!magic )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", __FUNCTION__  ));
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    if (magicLength != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(magicLength, SRAI_MemoryType_Shared);
        if (container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating buffer for magic field", __FUNCTION__));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[0].len = magicLength;
    }

    sage_rc = SRAI_Module_ProcessCommand(hMediaroomModule, DrmMediaroomTl_CommandId_GetMagic, container);

    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during Get Magic operation", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if (rc == BERR_SUCCESS) {
        BKNI_Memcpy(magic, container->blocks[0].data.ptr, container->blocks[0].len);

#ifdef B_DRM_MEDIAROOM_DEBUG
        DUMP_MEDIAROOM_DATA("nMagic: ", magic, container->blocks[0].len);
#endif
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", __FUNCTION__, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}

uint32_t DRM_Mediaroom_VerifyMagic(uint8_t *magic)
{
    if (magic[0] == 0x45 && magic[1] == 0x52 && magic[2] == 0x49 && magic[3] == 0x43)
        return 1;
    else
        return 0;
}

DrmRC DRM_Mediaroom_GetVersion(uint8_t *version, uint32_t versionLength)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if (!version )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", __FUNCTION__  ));
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    if (versionLength != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(versionLength, SRAI_MemoryType_Shared);
        if (container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating buffer for version field", __FUNCTION__));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[0].len = versionLength;
    }

    sage_rc = SRAI_Module_ProcessCommand(hMediaroomModule, DrmMediaroomTl_CommandId_GetVersion, container);

    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during Get Version operation", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if (rc == BERR_SUCCESS) {
        BKNI_Memcpy(version, container->blocks[0].data.ptr, container->blocks[0].len);

#ifdef B_DRM_MEDIAROOM_DEBUG
        DUMP_MEDIAROOM_DATA("nVersion: ", version, container->blocks[0].len);
#endif
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", __FUNCTION__, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}

uint32_t DRM_Mediaroom_VerifyVersion(uint8_t *version)
{
    if (version[0] == 0x00 && version[1] == 0x00 && version[2] == 0x00 && version[3] == 0x05)
        return 1;
    else
        return 0;
}

DrmRC DRM_Mediaroom_GetCertificate(uint8_t *cert, uint32_t certLength, bool fAV)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if (!cert )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", __FUNCTION__  ));
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    if (certLength != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(certLength, SRAI_MemoryType_Shared);
        if (container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating buffer for %s certificate", __FUNCTION__,(fAV==true)?"AV":"Non-AV"));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[0].len = certLength;
    }

    if (fAV)
    {
        sage_rc = SRAI_Module_ProcessCommand(hMediaroomModule, DrmMediaroomTl_CommandId_GetAVPbCert, container);
    }
    else
    {
        sage_rc = SRAI_Module_ProcessCommand(hMediaroomModule, DrmMediaroomTl_CommandId_GetNonAVPbCert, container);
    }
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during Get %s Certificate operation", __FUNCTION__, (fAV==true)?"AV":"Non-AV"));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if (rc == BERR_SUCCESS) {
        BKNI_Memcpy(cert, container->blocks[0].data.ptr, container->blocks[0].len);

#ifdef B_DRM_MEDIAROOM_DEBUG
        DUMP_MEDIAROOM_DATA("nCert: ", cert, container->blocks[0].len);
#endif
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", __FUNCTION__, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}

DrmRC DRM_Mediaroom_VerifyPb(uint32_t *result, bool fAV)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;
    DrmCommon_RsaKey_t  rsa_key;
    X509* pCertificate = NULL;
    uint8_t *pPublicKey = NULL;
    uint8_t *pCert = NULL;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;

    *result = 0; /* set result as invalid at first */

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(MAX(MEDIAROOM_CERT_SIZE,MEDIAROOM_PUBLIC_KEY_SIZE), SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating buffer for certificate and pubkey", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    /* Get pubkey from Mediaroom data section */
    container->blocks[0].len = MEDIAROOM_PUBLIC_KEY_SIZE;

    if (fAV)
    {
        sage_rc = SRAI_Module_ProcessCommand(hMediaroomModule, DrmMediaroomTl_CommandId_GetAVPb, container);
    }
    else
    {
        sage_rc = SRAI_Module_ProcessCommand(hMediaroomModule, DrmMediaroomTl_CommandId_GetNonAVPb, container);
    }
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during Get %s Pb operation", __FUNCTION__, (fAV==true)?"AV":"Non-AV"));
        rc = Drm_Err;
        goto ErrorExit;
    }
    rc = container->basicOut[0];
    if (rc == BERR_SUCCESS && container->blocks[0].len == MEDIAROOM_PUBLIC_KEY_SIZE) {
        rc = NEXUS_Memory_Allocate(container->blocks[0].len, NULL, (void **) &pPublicKey);
        if (rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error allocating buffer for pubkey", __FUNCTION__));
            goto ErrorExit;
        }
        BKNI_Memcpy(pPublicKey, container->blocks[0].data.ptr, container->blocks[0].len);

#ifdef B_DRM_MEDIAROOM_DEBUG
        DUMP_MEDIAROOM_DATA("AVPb: ", pPublicKey, container->blocks[0].len);
#endif
    }
    else
    {
        BDBG_ERR(("%s - Get Pb was sent succuessfully but actual operation failed (0x%08x)", __FUNCTION__, rc));
        goto ErrorExit;
    }

    /* Get cert from Mediaroom data section */
    container->blocks[0].len = MEDIAROOM_CERT_SIZE;

    if (fAV)
    {
        sage_rc = SRAI_Module_ProcessCommand(hMediaroomModule, DrmMediaroomTl_CommandId_GetAVPbCert, container);
    }
    else
    {
        sage_rc = SRAI_Module_ProcessCommand(hMediaroomModule, DrmMediaroomTl_CommandId_GetNonAVPbCert, container);
    }
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during Get %s Certificate operation", __FUNCTION__, (fAV==true)?"AV":"Non-AV"));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if (rc == BERR_SUCCESS && container->blocks[0].len == MEDIAROOM_CERT_SIZE) {
        rc = NEXUS_Memory_Allocate(container->blocks[0].len, NULL, (void **) &pCert);
        if (rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error allocating buffer for cert", __FUNCTION__));
            goto ErrorExit;
        }

        BKNI_Memcpy(pCert, container->blocks[0].data.ptr, container->blocks[0].len);

#ifdef B_DRM_MEDIAROOM_DEBUG
        DUMP_MEDIAROOM_DATA("nCert: ", pCert, container->blocks[0].len);
#endif

        errCode = BCrypt_x509ASN1DerDecode(mediaroom_bcrypt_handle,
                                            pCert,
                                            container->blocks[0].len,
                                            &pCertificate);
        if(errCode != BCRYPT_STATUS_eOK)
        {
            BDBG_ERR(("%s - Error x509 DER decode (0x%08x)", __FUNCTION__, errCode));
            BDBG_ERR(("%s - Error converting cert to internal form", __FUNCTION__));
            rc = Drm_BcryptErr;
            goto ErrorExit;
        }

        errCode = BCrypt_x509GetRsaPublicKey(mediaroom_bcrypt_handle,
                                             pCertificate,
                                             (BCRYPT_RSAKey_t*)&rsa_key);
        if(errCode != BCRYPT_STATUS_eOK)
        {
            BDBG_ERR(("%s - Error x509 get RSA public key (0x%08x)", __FUNCTION__, errCode));
            BDBG_ERR(("%s - Error extracting pubkey from cert", __FUNCTION__));
            rc = Drm_BcryptErr;
            goto ErrorExit;
        }

#ifdef B_DRM_EDRM_DEBUG
        BDBG_LOG(("modulus_size=%d pub_exponent_size=%d", rsa_key.n.len, rsa_key.e.len));
        DUMP_EDRM_DATA("nModulus: ", rsa_key.n.pData, rsa_key.n.len);
        DUMP_EDRM_DATA("nExponent: ", rsa_key.e.pData, rsa_key.e.len);
#endif


    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", __FUNCTION__, rc));
        goto ErrorExit;
    }

    if(rsa_key.n.len == MEDIAROOM_PUBLIC_KEY_SIZE)
        if(BKNI_Memcmp(pPublicKey, rsa_key.n.pData, rsa_key.n.len) == 0)
        {
            *result = 1; /* pubkey is valid */
        }

ErrorExit:
    if (pCertificate)
    {
        BCrypt_x509Free(mediaroom_bcrypt_handle, pCertificate);
    }
    if (pPublicKey)
    {
        NEXUS_Memory_Free(pPublicKey);
    }
    if (pCert)
    {
        NEXUS_Memory_Free(pCert);
    }
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}



DrmRC DRM_Mediaroom_P_GetFileSize(char * filename, uint32_t *filesize)
{
    DrmRC rc = Drm_Success;
    FILE *fptr = NULL;
    int pos = 0;

    BDBG_ENTER(DRM_Mediarom_P_GetFileSize);

    fptr = fopen(filename, "rb");
    if(fptr == NULL)
    {
        BDBG_ERR(("%s - Error opening file '%s'.  (%s)", __FUNCTION__, filename, strerror(errno)));
        rc = Drm_FileErr;
        goto ErrorExit;
    }
    pos = fseek(fptr, 0, SEEK_END);
    if(pos == -1)
    {
        BDBG_ERR(("%s - Error seeking to end of file '%s'.  (%s)", __FUNCTION__, filename, strerror(errno)));
        rc = Drm_FileErr;
        goto ErrorExit;
    }
    pos = ftell(fptr);
    if(pos == -1)
    {
        BDBG_ERR(("%s - Error determining position of file pointer of file '%s'.  (%s)", __FUNCTION__, filename, strerror(errno)));
        rc = Drm_FileErr;
        goto ErrorExit;
    }
    /* check vs. arbitrary large file size */
    if(pos >= 2*1024*1024)
    {
        BDBG_ERR(("%s - Invalid file size detected for of file '%s'.  (%u)", __FUNCTION__, filename, pos));
        rc = Drm_FileErr;
        goto ErrorExit;
    }
    (*filesize) = pos;
ErrorExit:
    BDBG_MSG(("%s - Exiting function (%u bytes)", __FUNCTION__, (*filesize)));
    if(fptr != NULL)
    {
        if(fclose(fptr) != 0){
            BDBG_ERR(("%s - Error closing drm bin file '%s'.  (%s)", __FUNCTION__, filename, strerror(errno)));
            rc = Drm_Err;
        }
    }
    return rc;
}

DrmRC DRM_Mediaroom_P_TA_Install(char * ta_bin_filename)
{
    DrmRC rc = Drm_Success;
    FILE * fptr = NULL;
    uint32_t file_size = 0;
    uint32_t read_size = 0;
    uint8_t *ta_bin_file_buff = NULL;
    BERR_Code sage_rc = BERR_SUCCESS;

    BDBG_ENTER(DRM_Mediaroom_TL_P_Install);

    BDBG_MSG(("%s - TA bin file name '%s'", __FUNCTION__, ta_bin_filename));

    rc = DRM_Mediaroom_P_GetFileSize(ta_bin_filename, &file_size);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error determine file size of TA bin file", __FUNCTION__));
        goto ErrorExit;
    }

    ta_bin_file_buff = SRAI_Memory_Allocate(file_size, SRAI_MemoryType_Shared);
    if(ta_bin_file_buff == NULL)
    {
        BDBG_ERR(("%s - Error allocating '%u' bytes for loading TA bin file", __FUNCTION__, file_size));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    BDBG_MSG(("%s: allocated ta_bin_file_buff at %p size %u",__FUNCTION__,ta_bin_file_buff,file_size));
    fptr = fopen(ta_bin_filename, "rb");
    if(fptr == NULL)
    {
        BDBG_ERR(("%s - Error opening TA bin file (%s)", __FUNCTION__, ta_bin_filename));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    read_size = fread(ta_bin_file_buff, 1, file_size, fptr);
    if(read_size != file_size)
    {
        BDBG_ERR(("%s - Error reading TA bin file size (%u != %u)", __FUNCTION__, read_size, file_size));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    /* close file and set to NULL */
    if(fclose(fptr) != 0)
    {
        BDBG_ERR(("%s - Error closing TA bin file '%s'.  (%s)", __FUNCTION__, ta_bin_filename, strerror(errno)));
        rc = Drm_FileErr;
        goto ErrorExit;
    }
    fptr = NULL;

    BDBG_MSG(("%s - Install TA file %s size %u", __FUNCTION__,ta_bin_filename,file_size));

    sage_rc = SRAI_Platform_Install(BSAGE_PLATFORM_ID_MEDIAROOM, ta_bin_file_buff, file_size);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling SRAI_Platform_Install Error 0x%x", __FUNCTION__, sage_rc ));
        rc = Drm_SraiModuleError;
        goto ErrorExit;
    }

ErrorExit:

    if(ta_bin_file_buff != NULL){
        SRAI_Memory_Free(ta_bin_file_buff);
        ta_bin_file_buff = NULL;
    }

    if(fptr != NULL){
        fclose(fptr);
        fptr = NULL;
    }

    BDBG_LEAVE(DRM_Mediaroom_P_TA_Install);

    return rc;
}


#define OTP_MSP0_VALUE_ZS (0x02)
#define OTP_MSP1_VALUE_ZS (0x02)
#define OTP_MSP0_VALUE_ZB (0x3E)
#define OTP_MSP1_VALUE_ZB (0x3F)


ChipType_e DRM_Mediaroom_GetChipType()
{

    NEXUS_ReadMspParms     readMspParms;
    NEXUS_ReadMspIO        readMsp0;
    NEXUS_ReadMspIO        readMsp1;
    NEXUS_Error rc =  NEXUS_SUCCESS;

    readMspParms.readMspEnum = NEXUS_OtpCmdMsp_eReserved233;
    rc = NEXUS_Security_ReadMSP(&readMspParms,&readMsp0);

    readMspParms.readMspEnum = NEXUS_OtpCmdMsp_eReserved234;
    rc = NEXUS_Security_ReadMSP(&readMspParms,&readMsp1);

    BDBG_MSG(("OTP MSP0 %d %d %d %d OTP MSP0 %d %d %d %d",readMsp0.mspDataBuf[0], readMsp0.mspDataBuf[1], readMsp0.mspDataBuf[2], readMsp0.mspDataBuf[3],
                                                          readMsp1.mspDataBuf[0], readMsp1.mspDataBuf[1], readMsp1.mspDataBuf[2], readMsp1.mspDataBuf[3]));

    if((readMsp0.mspDataBuf[3] == OTP_MSP0_VALUE_ZS) && (readMsp1.mspDataBuf[3] == OTP_MSP1_VALUE_ZS)){
        return ChipType_eZS;
    }
    return ChipType_eZB;
}


DrmRC
DRM_Mediaroom_ModuleInit(uint32_t module_id, char * drm_bin_filename,
    BSAGElib_InOutContainer *container, SRAI_ModuleHandle *moduleHandle)
{
    DrmRC rc = Drm_Success;
    FILE * fptr = NULL;
    uint32_t filesize = 0;
    uint32_t read_size = 0;
    uint32_t filesize_from_header = 0;
    uint32_t write_size = 0;
    BERR_Code sage_rc = BERR_SUCCESS;
    NEXUS_Error nexerr = NEXUS_SUCCESS;
    NEXUS_MemoryAllocationSettings memSettings;

    BDBG_ENTER(DRM_Mediaroom_ModuleInit);

    BDBG_ASSERT(container != NULL);
    BDBG_ASSERT(moduleHandle != NULL);

    /* if module uses a bin file - read it and add it to the container */
    if(drm_bin_filename != NULL)
    {
        BDBG_MSG(("%s - DRM bin filename '%s'", __FUNCTION__, drm_bin_filename));
        /*
         * 1) allocate drm_bin_file_buff
         * 2) read bin file
         * 3) check size
         * */
        rc = DRM_Mediaroom_P_GetFileSize(drm_bin_filename, &filesize);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error determine file size of bin file", __FUNCTION__));
            goto ErrorExit;
        }

        NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
        nexerr = NEXUS_Memory_Allocate(filesize, NULL,(void **) &mediaroom_drm_bin_file_buff);
        if(nexerr != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - Error allocating buffer err %u", __FUNCTION__, nexerr));
        }
        BDBG_MSG(("%s allocated file size %u drm_bin_file_buff %p", __FUNCTION__, filesize,mediaroom_drm_bin_file_buff));

        if(mediaroom_drm_bin_file_buff == NULL)
        {
            BDBG_ERR(("%s - Error allocating '%u' bytes", __FUNCTION__, filesize));
            (*moduleHandle) = NULL;
            rc = Drm_MemErr;
            goto ErrorExit;
        }

        fptr = fopen(drm_bin_filename, "rb");
        if(fptr == NULL)
        {
            BDBG_ERR(("%s - Error opening drm bin file (%s)", __FUNCTION__, drm_bin_filename));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }

        read_size = fread(mediaroom_drm_bin_file_buff, 1, filesize, fptr);
        if(read_size != filesize)
        {
            BDBG_ERR(("%s - Error reading drm bin file size (%u != %u)", __FUNCTION__, read_size, filesize));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }

        /* close file and set to NULL */
        if(fclose(fptr) != 0)
        {
            BDBG_ERR(("%s - Error closing drm bin file '%s'.  (%s)", __FUNCTION__, drm_bin_filename, strerror(errno)));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }

        fptr = NULL;

        /* verify allocated drm_bin_file_buff size with size in header */
        BKNI_Memcpy((uint8_t*)&mediaroom_drm_bin_header, mediaroom_drm_bin_file_buff, sizeof(drm_bin_header_t));
        filesize_from_header = GET_UINT32_FROM_BUF(mediaroom_drm_bin_header.bin_file_size);
        BDBG_MSG(("%s file size from header %u - line = %u", __FUNCTION__, filesize_from_header, __LINE__));

        if(filesize_from_header > DEFAULT_MEDIAROOM_DRM_BIN_FILESIZE)
        {
            BDBG_MSG(("%s - bin file size too large - line = %u", __FUNCTION__, __LINE__));
            NEXUS_Memory_Free(mediaroom_drm_bin_file_buff);
            mediaroom_drm_bin_file_buff = NULL;
            nexerr = NEXUS_Memory_Allocate(filesize_from_header, NULL,(void **) &mediaroom_drm_bin_file_buff);
            if (nexerr == NEXUS_SUCCESS) {
                BDBG_MSG(("%s setting drm bin file buff to 0s",__FUNCTION__));
                BKNI_Memset(mediaroom_drm_bin_file_buff, 0x00, filesize_from_header);
            }
            else
            {
                BDBG_MSG(("%s failed to allocate mem for invalid drm bin file size nexerr=%d",__FUNCTION__,nexerr));
                (*moduleHandle) = NULL;
                rc = Drm_SraiModuleError;
                goto ErrorExit;
            }
        }

        if(filesize_from_header != filesize)
        {
            BDBG_ERR(("%s - Error validating file size in header (%u != %u)", __FUNCTION__, filesize_from_header, filesize));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }

        /* All index '0' shared blocks will be reserved for drm bin file data */

        /* first verify that it has not been already used by the calling module */
        if(container->blocks[0].data.ptr != NULL)
        {
            BDBG_ERR(("%s - Shared block[0] reserved for all DRM modules with bin file to pass to Sage.", __FUNCTION__));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }

        container->blocks[0].len = filesize_from_header;
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(filesize_from_header, SRAI_MemoryType_Shared);
        if (container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating SRAI memory", __FUNCTION__));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[0].data.ptr, mediaroom_drm_bin_file_buff, filesize_from_header);

        BDBG_MSG(("%s - Copied '%u' bytes into SRAI container (address %p)", __FUNCTION__, filesize_from_header, container->blocks[0].data.ptr));
    }

    /* Initialize Mediaroom DRM module */
    sage_rc = SRAI_Module_Init(hMediaroomPlatform, module_id, container, moduleHandle);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling SRAI_Module_Init", __FUNCTION__));
        (*moduleHandle) = NULL;
        rc = Drm_SraiModuleError;
        goto ErrorExit;
    }

    /* Extract DRM bin file manager response from basic[0].  Free memory if failed */
    sage_rc = container->basicOut[0];
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - SAGE error processing DRM bin file", __FUNCTION__));

        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        (*moduleHandle) = NULL;
        rc = Drm_SraiModuleError;

        goto ErrorExit;
    }

#ifndef CMNDRM_SKIP_BINFILE_OVERWRITE
    /* Overwrite the drm bin file in rootfs, free up the buffer since a copy will exist on the sage side */
    if(drm_bin_filename != NULL)
    {
        if(container->basicOut[1] == DRM_MEDIAROOM_OVERWRITE_BIN_FILE)
        {
            BDBG_MSG(("%s - Overwriting file '%s'", __FUNCTION__, drm_bin_filename));

            if(fptr != NULL)
            {
                BDBG_ERR(("%s - File pointer already opened, invalid state.  '%s'", __FUNCTION__, drm_bin_filename));
                rc = Drm_Err;
                goto ErrorExit;
            }

            /* Overwrite drm bin file once bounded */
            fptr = fopen(drm_bin_filename, "w+b");
            if(fptr == NULL)
            {
                BDBG_ERR(("%s - Error opening DRM bin file (%s) in 'w+b' mode.  '%s'", __FUNCTION__, drm_bin_filename, strerror(errno)));
                rc = Drm_Err;
                goto ErrorExit;
            }

            write_size = fwrite(container->blocks[0].data.ptr, 1, filesize_from_header, fptr);
            if(write_size != filesize)
            {
                BDBG_ERR(("%s - Error writing drm bin file size to rootfs (%u != %u)", __FUNCTION__, write_size, filesize));
                (*moduleHandle) = NULL;
                rc = Drm_SraiModuleError;
                goto ErrorExit;
            }
            fclose(fptr);
            fptr = NULL;
        }
        else{
            BDBG_MSG(("%s - No need to overwrite file '%s'", __FUNCTION__, drm_bin_filename));
        }
    }
#endif

ErrorExit:

    /* if shared block[0] is not null, free since there was an error processing (i.e. can't trust the data) or the file was copied*/
    if(container->blocks[0].data.ptr != NULL){
        BDBG_MSG(("%s: freeing container->blocks[0].data.ptr",__FUNCTION__));
        SRAI_Memory_Free(container->blocks[0].data.ptr);
        container->blocks[0].data.ptr = NULL;
    }

    if(mediaroom_drm_bin_file_buff != NULL){
        BDBG_MSG(("%s: freeing mediaroom_drm_bin_file_buff",__FUNCTION__));
        NEXUS_Memory_Free(mediaroom_drm_bin_file_buff);
        mediaroom_drm_bin_file_buff = NULL;
    }

    if(fptr != NULL){
        BDBG_MSG(("%s: Freeing fptr",__FUNCTION__));
        fclose(fptr);
        fptr = NULL;
    }

    BDBG_LEAVE(DRM_Mediaroom_ModuleInitialize);

    return rc;
}
