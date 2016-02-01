/*********************************************************************************
*     Copyright (c) 2010-2012, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*   This program is the proprietary software of Broadcom Corporation and/or its licensors, 
*   and may only be used, duplicated, modified or distributed pursuant to the terms and 
*   conditions of a separate, written license agreement executed between you and Broadcom 
*   (an "Authorized License").  Except as set forth in an Authorized License, 
*   Broadcom grants no license (express or implied), right to use, or waiver of any kind 
*   with respect to the Software, and Broadcom expressly reserves all rights in and to the 
*   Software and all intellectual property rights therein.  
* 
*   IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, 
*   AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*   Except as expressly set forth in the Authorized License,
*   1.     This program, including its structure, sequence and organization, constitutes the 
*       valuable trade secrets of Broadcom, and you shall use all reasonable efforts to protect 
*       the confidentiality thereof, and to use this information only in connection with your use 
*       of Broadcom integrated circuit products.
*   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL 
*       FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, 
*       STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND 
*       ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*       LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO 
*       DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE LIABLE 
*       FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT 
*       OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN 
*       ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID 
*       FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING 
*       ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description: encrypt/decrypt operations using unique HW key
*
* Revision History:
*
* Created: 07/30/10 by Claudio Discepola
*
*
*********************************************************************************/
#ifndef DRM_SECURE_STORE_H_
#define DRM_SECURE_STORE_H_

#include "drm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************************************
DRM_SecureStore_BufferOperation

Performs AES-ECB operation on a 16-byte aligned source buffer using a unique chip (OTP) key.  
If the 'src_length' is not 16-byte aligned the function will return an error.  
The caller will have the option to perform encryption/decryption 'in place'
or 'external' by specifying a destination buffer which should also
have been allocated with the proper size and be 16-byte aligned.   
 
Input Parameters:
pSrc
    Type: uint8_t*
    Purpose:Source buffer containing the data. Should be allocated from the heap.

src_length
    Type: uint32_t
    Purpose: specifies the length of the source buffer. Must be 16-byte aligned

destType
    Type:
    Purpose: If the caller specifies 'DrmDestinationType_eInPlace', pSrc must 
            equal pDest.
            If the caller specifies 'DrmDestinationType_eExternal', pSrc must 
            differ from pDest.
drmOp
    Type: DrmCryptoOperation
    Purpose: DrmCryptoOperation_eEncrypt (encryption)
             DrmCryptoOperation_eDecrypt (decryption)

Output Parameters:
pDest
    Type: uint8_t* 
    Purpose: Destination buffer that will contain the resulting data. Should be 
             allocated from the heap and be at least the same size as 'src_length'.

Returns: 
              On Success:   Drm_Success
              On Failure:   any other error code.              
****************************************************************************************/
DrmRC DRM_SecureStore_BufferOperation(
    uint8_t* pSrc,
    uint32_t src_length,    
    uint8_t* pDest,
    DrmDestinationType destType,    
    DrmCryptoOperation drmOp
    );


/**********************************************************************************************
DrmRC DRM_SecureStore_FileOperation

Similar to 'DRM_SecureStore_BufferOperation'.
If a 'read' operation is specified by the caller, the function will fetch its source data by attempting 
to read and decrypt (AES-ECB)the contents specified by 'file_path'.
If a 'write' operation is specified by the caller, the function will encrypt (AES-ECB) the source buffer 
passed to it and create a binary file at the location specified by 'file_path'

Input/Output Parameter:
pBuf 
    Type:  uint8_t*
    Purpose:  Buffer containing either the data to write to file OR will contain the data read from a file. 
              Should be allocated from the heap.

Input Parameters: 

buf_length 
    Type: uint32_t
    Purpose: specifies the length of the buffer to write to a file (write operation) OR specifies the length 
            of the data to read and decrypt from a file (read operation).  Must be 16-byte aligned
file_path
    Type:  char*
    Purpose:  Specifies the location of where to create the file in the case of a write operation OR
              specifies the location of where to read the contents of the source buffer in the case 
              of a read operation.
fileOp
    Type:  DrmFileOperation
    Purpose: DrmFileOperation_eWrite (write)
              DrmFileOperation_eRead (read)
drmOp
    Type:  DrmCryptoOperation
    Purpose: DrmCryptoOperation_eEncrypt (encryption)
             DrmCryptoOperation_eDecrypt (decryption)

Returns: 
              On Success:   Drm_Success
              On Failure:   any other error code. 
*****************************************************************************************************/
DrmRC DRM_SecureStore_FileOperation(
    uint8_t* pBuf,
    uint32_t buf_length,    
    char*  file_path,
    DrmFileOperation file_op,
    DrmCryptoOperation drm_op    
    );


/**********************************************************************************************
* DrmRC DRM_SecureStore_AlignedFileEncWrite
* 
* This functions is used to encrypt a data buffer and save it encrypted in a file. Note that if 
* the file already exists, it's content will be overwritted. 
* 
* Input/Output Parameter:
* pBuf 
*     Purpose:  Buffer containing the data to encrypt and write to file.
* 
* Input Parameters: 
* 
* bufSize 
*     Purpose: specifies the length of the buffer to write to a file.
* 
* pPath
*     Purpose:  Specifies the location of where to create the file.
* 
* Returns: 
*     On Success:   Drm_Success
*     On Failure:   any other error code. 
*****************************************************************************************************/
DrmRC DRM_SecureStore_AlignedFileEncWrite(
    char    *pPath,
    uint8_t *pBuf,
    uint32_t bufSize
);

/**********************************************************************************************
* DrmRC DRM_SecureStore_AlignedFileDecRead
* 
* This functions is used to read the content of a file, decrypt it and copy the decrypted data into
* the buffer pointed to by pBuf. The number of bytes copieds is returned into bufSize. An application can
* call this function and set pBuf to NULL to figure out how much memory it needs to allocate.
* 
* For example:
* 
* Figure out the size of the data to read:
* ...
* rc =  DRM_SecureStore_AlignedFileDecRead("file.bin", NULL, &bufSize);
* 
* pBuf = (uint8_t *)BKNI_Malloc(bufSize);
* 
* Read data from file
* rc =  DRM_SecureStore_AlignedFileDecRead("file.bin", pBuf, &bufSize);
* ...
* 
* Input/Output Parameter:
* pBuf 
*     Type:  uint8_t*
*     Purpose:  Buffer Where the decrypted data will be copied into. When the parameter is NULL,
*     the function only returns the size of the data saved in the file.
* 
* bufSize 
*     Type: uint32_t
*     Purpose: Specifies the length of the buffer in bytes.
* 
* Input Parameters: 
*
* pPath
*     Type:  char*
*     Purpose:  Specifies the location of the fi.
* 
* Returns: 
*     On Success:   Drm_Success
*     On Failure:   any other error code. 
*****************************************************************************************************/
DrmRC DRM_SecureStore_AlignedFileDecRead(
    char     *pPath,
    uint8_t  *pBuf,
    uint32_t *bufSize
);

#ifdef __cplusplus
}
#endif

#endif /*DRM_SECURE_STORE_H_*/
