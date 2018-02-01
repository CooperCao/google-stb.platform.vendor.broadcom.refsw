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
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "bstd.h"
#include "bkni.h"

#include "drm_secure_store.h"

#include "drm_common.h"

BDBG_MODULE(drm_secure_store);

#define SET_UINT32_IN_BUF(pBuf, val)                        \
    {   *pBuf       = (uint8_t)((val & 0xFF000000) >> 24);  \
        *(pBuf + 1) = (uint8_t)((val & 0xFF0000) >> 16);    \
        *(pBuf + 2) = (uint8_t)((val & 0xFF00) >> 8);       \
        *(pBuf + 3) = (uint8_t)((val & 0xFF));              \
    }

/******************************************************************************
 ** FUNCTION:
 **   DRM_SecureStore_BufferOperation
 **
 ** DESCRIPTION:
 **   Performs AES-ECB operation on a 16-byte aligned source buffer using a unique chip (OTP) key.
 **   If the 'src_length' is not 16-byte aligned the function will return an error.
 **   The caller will have the option to perform encryption/decryption 'in place'
 **   or 'external' by specifying a destination buffer which should also
 **   have been allocated with the proper size and be 16-byte aligned.
 **
 ** PARAMETERS:
 **   pSrc[in]
 **     Type: uint8_t*
 **     Purpose: Source buffer containing the data. Should be allocated from the heap.
 **
 **   src_length[in]
 **     Type: uint32_t
 **     Purpose: Specifies the length of the source buffer. Must be 16-byte aligned.
 **
 **   pDest[out]
 **     Type: uint8_t*
 **     Purpose: Destination buffer that will contain the resulting data. Should be
 **              allocated from the heap and be at least the same size as 'src_length'.
 **
 **   destType[in]
 **     Type: DrmDestinationType
 **     Purpose: If the caller specifies 'DrmDestinationType_eInPlace', pSrc must equal pDest.
 **              If the caller specifies 'DrmDestinationType_eExternal', pSrc must differ from pDest.
 **
 **   drmOp[in]
 **     Type: DrmCryptoOperation
 **     Purpose: DrmCryptoOperation_eEncrypt (encryption)
 **              DrmCryptoOperation_eDecrypt (decryption)
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_SecureStore_BufferOperation(
    uint8_t* pSrc,
    uint32_t src_length,
    uint8_t* pDest,
    DrmDestinationType destType,
    DrmCryptoOperation drmOp)
{
    DrmRC drm_rc = Drm_Success;
    DmaBlockInfo_t dmaBlock;
    DrmCommonOperationStruct_t drmCommonOpStruct;
    CommonCryptoKeyLadderSettings keyLadderInfo;
    uint8_t custom_proc_in1[16] = {0x2a, 0x34, 0x9a, 0x86, 0x31, 0x2a, 0x33, 0xf5,
                                   0x2c, 0x68, 0xa1, 0xbf, 0x36, 0x9a, 0xb2, 0xc1};
    uint8_t custom_proc_in2[16] = {0x34, 0x18, 0xd4, 0x2c, 0x1f, 0x7f, 0x00, 0xb2,
                                   0x09, 0x29, 0x28, 0x82, 0x23, 0xa6, 0x76, 0x6e};

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    /* Parameter checks */
    drm_rc = DrmAssertParam("DrmCryptoOperation", (uint32_t)drmOp, (uint32_t)DrmCryptoOperation_eMax);
    if(drm_rc != Drm_Success)goto ss_op_err;

    drm_rc = DrmAssertParam("DrmDestinationType", (uint32_t)destType, (uint32_t)DrmDestinationType_eMax);
    if(drm_rc != Drm_Success)goto ss_op_err;

    if( (destType == DrmDestinationType_eInPlace) && (pSrc != pDest) )
    {
        BDBG_ERR(("%s - When 'in place' destination is specified for crypto operation, source buffer address must equal destination address.", BSTD_FUNCTION));
        drm_rc = Drm_InvalidParameter;
        goto ss_op_err;
    }

    if( (destType == DrmDestinationType_eExternal) && (pSrc == pDest) )
    {
        BDBG_ERR(("%s - When 'external' destination is specified for crypto operation, source buffer address must differ from destination address.", BSTD_FUNCTION));
        drm_rc = Drm_InvalidParameter;
        goto ss_op_err;
    }

    if(src_length%16 != 0)
    {
        BDBG_ERR(("%s - Invalid source buffer size specified (%u).", BSTD_FUNCTION, src_length));
        drm_rc = Drm_InvalidParameter;
        goto ss_op_err;
    }

    /* Set DMA parameters */
    dmaBlock.pSrcData = pSrc;
    dmaBlock.pDstData = pDest;
    dmaBlock.uiDataSize = src_length;
    dmaBlock.sg_start = true;
    dmaBlock.sg_end = true;

    DRM_Common_GetDefaultStructSettings(&drmCommonOpStruct);
    BKNI_Memset((uint8_t*)&keyLadderInfo, 0x00, sizeof(CommonCryptoKeyLadderSettings));

    drmCommonOpStruct.keyConfigSettings.settings.opType     = drmOp;
#if (NEXUS_SECURITY_API_VERSION==1)
    drmCommonOpStruct.keyConfigSettings.settings.algType    = NEXUS_SecurityAlgorithm_eAes;
    drmCommonOpStruct.keyConfigSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eEcb;
    drmCommonOpStruct.keyConfigSettings.settings.termMode   = NEXUS_SecurityTerminationMode_eClear;
    drmCommonOpStruct.keySrc = CommonCrypto_eOtpKey;
#else
    drmCommonOpStruct.keyConfigSettings.settings.algType   = NEXUS_CryptographicAlgorithm_eAes128;
    drmCommonOpStruct.keyConfigSettings.settings.algVariant = NEXUS_CryptographicAlgorithmMode_eEcb;
    drmCommonOpStruct.keyConfigSettings.settings.termMode =  NEXUS_KeySlotTerminationMode_eClear;
    drmCommonOpStruct.keySrc = CommonCrypto_eOtpDirect;
#endif
    drmCommonOpStruct.pKeyLadderInfo = &keyLadderInfo;
    BKNI_Memcpy(keyLadderInfo.procInForKey3, custom_proc_in1, 16);
    BKNI_Memcpy(keyLadderInfo.procInForKey4, custom_proc_in2, 16);
    drmCommonOpStruct.pDmaBlock     = &dmaBlock;
    drmCommonOpStruct.num_dma_block = 1;

    DRM_MSG_PRINT_BUF("dmaBlock (before)", dmaBlock.pSrcData, 16);

    drm_rc = DRM_Common_OperationDma(&drmCommonOpStruct);
    if(drm_rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error loading key or with M2M DMA operation, examine for previous error messages", BSTD_FUNCTION));
        goto ss_op_err;
    }

    DRM_MSG_PRINT_BUF("dmaBlock (after)", dmaBlock.pDstData, 16);

 ss_op_err:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return drm_rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_SecureStore_FileOperation
 **
 ** DESCRIPTION:
 **   Similar to 'DRM_SecureStore_BufferOperation'.
 **   If a 'read' operation is specified by the caller, the function will fetch its source data
 **   by attempting to read and decrypt (AES-ECB)the contents specified by 'file_path'.
 **   If a 'write' operation is specified by the caller, the function will encrypt (AES-ECB) the
 **   source buffer passed to it and create a binary file at the location specified by 'file_path'
 **
 ** PARAMETERS:
 **   pBuf[in/out]
 **     Type: uint8_t*
 **     Purpose: Buffer containing either the data to write to file OR will contain the data
 **              read from a file.  Should be allocated from the heap.
 **
 **   buf_length[in]
 **     Type: uint32_t
 **     Purpose: Specifies the length of the buffer to write to a file (write operation)
 **              OR specifies the length of the data to read and decrypt from a file (read operation).
 **              Must be 16-byte aligned.
 **
 **   file_path[in]
 **     Type: char*
 **     Purpose: Specifies the location of where to create the file in the case of a write operation
 **              OR specifies the location of where to read the contents of the source buffer in the
 **              case of a read operation.
 **
 **   fileOp[in]
 **     Type: DrmFileOperation
 **     Purpose: DrmFileOperation_eWrite (write)
 **              DrmFileOperation_eRead (read)
 **
 **   crypto_op[in]
 **     Type: DrmCryptoOperation
 **     Purpose: DrmCryptoOperation_eEncrypt (encryption)
 **              DrmCryptoOperation_eDecrypt (decryption)
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_SecureStore_FileOperation(
    uint8_t* pBuf,
    uint32_t buf_length,
    char*  file_path,
    DrmFileOperation file_op,
    DrmCryptoOperation crypto_op )
{
    DrmRC drm_rc = Drm_Success;
    FILE* fptr = NULL;
    uint32_t bytes_written = 0;
    uint32_t bytes_read = 0;
    const char file_wb_string[] = "w+b";
    const char file_rb_string[] = "rb";
    char* ptr_curr_op = NULL;

    /* Parameter checks */
    drm_rc = DrmAssertParam("DrmFileOperation", (uint32_t)file_op, (uint32_t)DrmFileOperation_eMax);
    if(drm_rc != Drm_Success)goto ss_file_err;

    if(pBuf == NULL)
    {
        BDBG_ERR(("%s - Invalid source buffer detected.", BSTD_FUNCTION));
        drm_rc = Drm_InvalidParameter;
        goto ss_file_err;
    }

    if(file_path == NULL)
    {
        BDBG_ERR(("%s - Invalid path string detected.", BSTD_FUNCTION));
        drm_rc = Drm_InvalidParameter;
        goto ss_file_err;
    }

    /* Assign local varaible to 'rb' or 'w+b'*/
    if(file_op == DrmFileOperation_eWrite)ptr_curr_op = (char*)file_wb_string;
    else ptr_curr_op = (char*)file_rb_string;

    /* If we're writing, check if size is 16-byte aligned*/
    if( (file_op == DrmFileOperation_eWrite) && (buf_length%16 != 0) )
    {
        BDBG_ERR(("%s - Invalid source buffer length (%u), must be 16-byte aligned.", BSTD_FUNCTION, buf_length));
        drm_rc = Drm_InvalidParameter;
        goto ss_file_err;
    }

    /* Open file */
    BDBG_MSG(("%s - Preparing to open file '%s' in mode '%s'.", BSTD_FUNCTION, file_path, ptr_curr_op));
    fptr = fopen(file_path, ptr_curr_op);
    if(fptr == NULL)
    {
        BDBG_ERR(("%s - Error opening '%s' for %s operation", BSTD_FUNCTION, file_path, ptr_curr_op));
        drm_rc = Drm_FileErr;
        goto ss_file_err;
    }

    if(file_op == DrmFileOperation_eRead)
    {
        bytes_read = fread(pBuf, 1, buf_length, fptr);
        if(bytes_read != buf_length)
        {
            BDBG_ERR(("%s - Error reading '%u' bytes from file '%s'.   Only '%u' bytes read", BSTD_FUNCTION, buf_length, file_path, bytes_read));
            drm_rc = Drm_FileErr;
            goto ss_file_err;
        }
    }

    /* Call to encrypt or decrypt buffer */
    drm_rc = DRM_SecureStore_BufferOperation(pBuf, buf_length, pBuf, DrmDestinationType_eInPlace, crypto_op);
    if(drm_rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error calling DRM_SecureStore_BufferOperation(%u)", BSTD_FUNCTION, crypto_op));
        goto ss_file_err;
    }

    if(file_op == DrmFileOperation_eWrite)
    {
        bytes_written = fwrite(pBuf, 1, buf_length, fptr);
        if(bytes_written != buf_length)
        {
            BDBG_ERR(("%s - Error writing '%u' bytes to file '%s'.  Only '%u' bytes written", BSTD_FUNCTION, buf_length, file_path, bytes_written));
            drm_rc = Drm_FileErr;
            goto ss_file_err;
        }
    }

 ss_file_err:
    if(fptr != NULL){
        fclose(fptr);
    }
    return drm_rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_SecureStore_AlignedFileEncWrite
 **
 ** DESCRIPTION:
 **   Similar to 'DRM_SecureStore_BufferOperation'.
 **   This functions is used to encrypt a data buffer and save it encrypted
 **   in a file. Note that if the file already exists, it's content will be
 **   overwritten.
 **
 ** PARAMETERS:
 **   pPath[in]
 **     Type: char*
 **     Purpose: Specifies the location of where to create the file
 **
 **   pBuf[in/out]
 **     Type: uint8_t*
 **     Purpose: Buffer containing the data to encrypt and write to file.
 **
 **   data_length[in]
 **     Type: uint32_t
 **     Purpose: Specifies the length of the buffer to write to a file
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_SecureStore_AlignedFileEncWrite(
    char    *pPath,
    uint8_t *pBuf,
    uint32_t data_length
                                          )
{
    DrmRC rc = Drm_Success;
    uint8_t *pDmaBuf = NULL;
    uint32_t dma_buf_size;
    const uint32_t header_size = 4;
    dma_buf_size =  data_length;

    /* Keep 4 bytes as a file header to save the size of the plain text data segment */
    dma_buf_size += header_size;

    /* Compute the amount of padding needed if any */
    dma_buf_size += (16 - (dma_buf_size % 16));

    /* Allocate a continuous physical address buffer for DMA */
    if(DRM_Common_MemoryAllocate(&pDmaBuf, dma_buf_size) != Drm_Success)
    {
        BDBG_ERR(("%s - Error allocating '%u' bytes for buffer", BSTD_FUNCTION, dma_buf_size));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    SET_UINT32_IN_BUF(pDmaBuf, data_length); /* Save buffer size in the file header. */
    BKNI_Memcpy(&pDmaBuf[header_size], pBuf, data_length);

    rc = DRM_SecureStore_FileOperation(pDmaBuf, dma_buf_size, pPath,
                                       DrmFileOperation_eWrite, DrmCryptoOperation_eEncrypt);
    if(rc != Drm_Success)
    {
        goto ErrorExit;
    }

 ErrorExit:
    /* Free memory */
    if(pDmaBuf != NULL){
        NEXUS_Memory_Free(pDmaBuf);
    }

    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_SecureStore_AlignedFileDecRead
 **
 ** DESCRIPTION:
 **   This functions is used to read the content of a file, decrypt it and copy
 **   the decrypted data into the buffer pointed to by pBuf. The number of bytes
 **   copied is returned into bufSize. An application can call this function and
 **   set pBuf to NULL to figure out how much memory it needs to allocate.
 **
 **   For example:
 **   1. Figure out the size of the data to read:
 **      ...
 **      rc =  DRM_SecureStore_AlignedFileDecRead("file.bin", NULL, &bufSize);
 **      pBuf = (uint8_t *)BKNI_Malloc(bufSize);
 **
 **   2. Read data from file
 **      rc =  DRM_SecureStore_AlignedFileDecRead("file.bin", pBuf, &bufSize);
 **      ...
 **
 ** PARAMETERS:
 **   pPath[in]
 **     Type: char*
 **     Purpose: Specifies location of the file
 **
 **   pBuf[in/out]
 **     Type: uint8_t*
 **     Purpose: Buffer Where the decrypted data will be copied into. When the
 **              parameter is NULL, the function only returns the size of the
 **              data saved in the file.
 **
 **   bufSize[in]
 **     Type: uint32_t*
 **     Purpose: Specifies the length of the buffer in bytes
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_SecureStore_AlignedFileDecRead(
    char     *pPath,
    uint8_t  *pBuf,
    uint32_t *bufSize
                                         )
{
    DrmRC rc = Drm_Success;
    uint8_t *pDmaBuf = NULL;
    uint32_t data_length = 0;
    const uint32_t header_length = 4; /* Offset in buffer for data length */
    struct stat info;

    if(stat(pPath, &info) != 0){
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    /* Allocate a continuous physical address buffer for DMA */
    if(DRM_Common_MemoryAllocate(&pDmaBuf, info.st_size) != Drm_Success)
    {
        BDBG_ERR(("%s - Error allocating '%u' bytes for buffer", BSTD_FUNCTION, (unsigned int)info.st_size));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    rc = DRM_SecureStore_FileOperation(pDmaBuf, info.st_size, pPath,
                                       DrmFileOperation_eRead, DrmCryptoOperation_eDecrypt);
    if(rc != Drm_Success)
    {
        goto ErrorExit;
    }

    data_length = GET_UINT32_FROM_BUF(pDmaBuf);
    if((*bufSize) < data_length){
        *bufSize = data_length;
        rc = Drm_BufTooSmall;
        goto ErrorExit;
    }
    *bufSize = data_length;

    BKNI_Memcpy(pBuf, &pDmaBuf[header_length], data_length);

 ErrorExit:
    /* Free memory */
    if(pDmaBuf != NULL){
        NEXUS_Memory_Free(pDmaBuf);
    }

    return rc;
}
