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
 **************************************************************************/
/*! \file b_secbuf.h
 *  \brief define secbuf data structure.
 */
#ifndef B_SECBUF_H
#define B_SECBUF_H

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------module description -------------------------------------------*/

/*! \brief b_secbuf library is designed to provide secure buffer for encrypted contents.
 *
 *
 */


/***************************************************************************
Summary:
Generic Error Codes

Description:
These error codes will match those returned by nexus (NEXUS_Error) and
magnum (BERR_Code).  These may be used throughout application libraries
for consistency.
***************************************************************************/
typedef unsigned B_Error;

/**
Summary:
Standard Nexus error codes.
**/
#ifndef B_ERROR_SUCCESS
#define B_ERROR_SUCCESS              0  /* success (always zero) */
#define B_ERROR_NOT_INITIALIZED      1  /* parameter not initialized */
#define B_ERROR_INVALID_PARAMETER    2  /* parameter is invalid */
#define B_ERROR_OUT_OF_MEMORY        3  /* out of heap memory */
#define B_ERROR_TIMEOUT              5  /* reached timeout limit */
#define B_ERROR_OS_ERROR             6  /* generic OS error */
#define B_ERROR_LEAKED_RESOURCE      7  /* resource being freed has attached resources that haven't been freed */
#define B_ERROR_NOT_SUPPORTED        8  /* requested feature is not supported */
#define B_ERROR_UNKNOWN              9  /* unknown */
#endif

/**
Summary:
Determines secbuf type.
**/

typedef enum B_Secbuf_Type
{
	B_Secbuf_Type_eGeneric = 0, /* Generic memory, accessible by host */
	B_Secbuf_Type_eSecure = 1, /* Secure memory, not accessible by host */
}B_Secbuf_Type;

typedef void *B_SecbufToken;
typedef struct B_Secbuf_Info_
{
    B_Secbuf_Type type;
    size_t size;
    B_SecbufToken token; /* share nexus memory among processes */
} B_Secbuf_Info;
B_Error B_Secbuf_Alloc(size_t size, B_Secbuf_Type type, void ** buffer);
B_Error B_Secbuf_AllocWithToken(size_t size, B_Secbuf_Type type, B_SecbufToken token, void ** buffer);
B_Error B_Secbuf_Free(void * buffer);
B_Error B_Secbuf_ImportData(void * buffer,  unsigned int offset, unsigned char * pDataIn, size_t len, bool last);
B_Error B_Secbuf_ExportData(void * buffer, unsigned int offset, unsigned char * pDataOut, size_t len, bool last);
B_Error B_Secbuf_ImportDataChunk(void * buffer, unsigned char * pDataIn, void * chunk_info, unsigned int chunk_count);
B_Error B_Secbuf_GetBufferInfo(void * buffer, B_Secbuf_Info * info);
B_Error B_Secbuf_FreeDesc(void * buffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* B_SECBUF_H */
