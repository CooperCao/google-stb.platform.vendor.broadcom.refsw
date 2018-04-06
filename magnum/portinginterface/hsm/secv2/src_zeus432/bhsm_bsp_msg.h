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

#ifndef BHSM_BSP_MSG_H__
#define BHSM_BSP_MSG_H__

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bint.h"
#include "bsp_s_commands.h"
#include "bhsm.h"

/* BSP interface Message API
Through this interface a BSP client can:
    - Create a BSP message
            - BHSM_BspMsg_Create
            - BHSM_BspMsg_Destroy
    - Configure the the content of the the message
            - BHSM_BspMsg_Header
            - BHSM_BspMsg_Pack8
            - BHSM_BspMsg_Pack16
            - BHSM_BspMsg_Pack24
            - BHSM_BspMsg_Pack32
            - BHSM_BspMsg_PackArray
    - Submit the message to the BSP Interface
            - BHSM_BspMsg_SubmitCommand
    - Read  the responce from the BSP inerface
            - BHSM_BspMsg_Get8
            - BHSM_BspMsg_Get16
            - BHSM_BspMsg_Get24
            - BHSM_BspMsg_Get32
            - BHSM_BspMsg_GetArray
*/

#define BHSM_P_BSP_MSG_SIZE (BCMD_BUFFER_BYTE_SIZE)  /* the maxumum size of a BSP command */
#define BHSM_P_BSP_INVALID_STATUS (0xFF)             /* Indicates an invalid BSP status */


typedef struct BHSM_P_BspMsg* BHSM_BspMsg_h;


typedef struct
{
    unsigned int commandLength;     /* length of command to send to BSP. If zero is specified the command length will be auto-calculated.*/
    uint32_t     continualMode;

    bool        isRaw;               /* When True, the command is assumed to be raw, the client is responsible for composing the header. Default:false */
    bool        verbose;             /* allow disabling of debug on very frequent calls. Default:true */
} BHSM_BspMsgHeader_t;


typedef struct
{

    struct{
        unsigned size;          /* Size. For now we only need sizeof(uint32_t) */
        void    *p;             /* pointer to memory */
    }secureMemory;              /* Secure memory. If none available (size = 0), we'll use regular memory. */

} BHSM_BspMsgConfigure_t;


void BHSM_BspMsg_DumpOutbox( BHSM_BspMsg_h hMsg );
void BHSM_BspMsg_DumpInbox( BHSM_BspMsg_h hMsg );

/* To be called only during HSM module initialisation. */
BERR_Code BHSM_BspMsg_Init( BHSM_Handle hHsm, BHSM_BspMsgConfigure_t *pConfig );

/* To be called only during HSM module uninit. */
void BHSM_BspMsg_Uninit( BHSM_Handle hHsm );

/* Create a BSP interface component.  */
BERR_Code BHSM_BspMsg_Create( BHSM_Handle hHsm, BHSM_BspMsg_h *phMsg );
/* Create a BSP interface component.  */
BERR_Code BHSM_BspMsg_Destroy( BHSM_BspMsg_h hMsg );

/*returns that default header configuration */
void BHSM_BspMsg_GetDefaultHeader( BHSM_BspMsgHeader_t *pHeader );

/* Configure the header of the BSP Message */
BERR_Code BHSM_BspMsg_Header( BHSM_BspMsg_h hMsg, BCMD_cmdType_e bspCommand, BHSM_BspMsgHeader_t *pHeader );

/* Pack an 8 bit unit8_t into the message at as specifed byte offset */
BERR_Code BHSM_BspMsg_Pack8 ( BHSM_BspMsg_h hMsg, unsigned int offset, uint8_t  data );

/* Pack an 16 bit uint16_t into the message at as specifed byte offset */
BERR_Code BHSM_BspMsg_Pack16( BHSM_BspMsg_h hMsg, unsigned int offset, uint16_t data );

/* Pack 24 bits into the message at as specifed byte offset (24 lsb of uint32_t) */
BERR_Code BHSM_BspMsg_Pack24( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t data );

/* Pack an 32 bit uint32_t into the message at as specifed byte offset */
BERR_Code BHSM_BspMsg_Pack32( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t data );

/* Pack an array of uint8_t into the message at as specifed byte offset */
BERR_Code BHSM_BspMsg_PackArray( BHSM_BspMsg_h hMsg, unsigned int offset, uint8_t *pData, unsigned int length );

/* Submit the Message to the BSP interface. Function blocks until a response is received (or it times out) */
BERR_Code BHSM_BspMsg_SubmitCommand( BHSM_BspMsg_h hMsg );

/* Read an 8 bit uint8_t from the BSP repoonse at as specifed byte offset */
BERR_Code BHSM_BspMsg_Get8( BHSM_BspMsg_h hMsg, unsigned int offset, uint8_t  *pData );

/* Read a 16 bit uint16_t from the BSP repoonse at as specifed byte offset */
BERR_Code BHSM_BspMsg_Get16( BHSM_BspMsg_h hMsg, unsigned int offset, uint16_t *pData );

/* Read 24 bits uint16_t from the BSP repoonse at as specifed byte offset (out pit is 24 lsb of uint32_t) */
BERR_Code BHSM_BspMsg_Get24( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t *pData );

/* Read an 32 bit uint32_t from the BSP repoonse at as specifed byte offset */
BERR_Code BHSM_BspMsg_Get32( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t *pData );

/* Read an uint8_t array from the BSP repoonse at as specifed byte offset */
BERR_Code BHSM_BspMsg_GetArray( BHSM_BspMsg_h hMsg, unsigned int offset, uint8_t  *pData, unsigned int length );

/* Map BSP Message API to implementation functions (*_impl) that facilitate augmented debug  */
#if BHSM_VERBOSE_BSP_TRACE
#define BHSM_BspMsg_Header(hMsg,bspCommand,pHeader) BHSM_BspMsg_Header_impl(hMsg,bspCommand,pHeader,#bspCommand)
#define BHSM_BspMsg_Pack8(hMsg,offset,data) BHSM_BspMsg_Pack8_impl(hMsg,offset,data, #offset)
#define BHSM_BspMsg_Pack16(hMsg,offset,data) BHSM_BspMsg_Pack16_impl(hMsg,offset,data,#offset)
#define BHSM_BspMsg_Pack24(hMsg,offset,data) BHSM_BspMsg_Pack24_impl(hMsg,offset,data,#offset)
#define BHSM_BspMsg_Pack32(hMsg,offset,data) BHSM_BspMsg_Pack32_impl(hMsg,offset,data,#offset)
#define BHSM_BspMsg_PackArray(hMsg,offset,pData,length) BHSM_BspMsg_PackArray_impl(hMsg,offset,pData,length,#offset)
#define BHSM_BspMsg_Get8(hMsg,offset,data) BHSM_BspMsg_Get8_impl(hMsg,offset,data, #offset)
#define BHSM_BspMsg_Get16(hMsg,offset,data) BHSM_BspMsg_Get16_impl(hMsg,offset,data, #offset)
#define BHSM_BspMsg_Get32(hMsg,offset,data) BHSM_BspMsg_Get32_impl(hMsg,offset,data, #offset)
#define BHSM_BspMsg_GetArray(hMsg,offset,data,length) BHSM_BspMsg_GetArray_impl(hMsg,offset,data,length,#offset)
#else   /*No DEBUG ... NULL the COMMAND*/
#define BHSM_BspMsg_Header(hMsg,bspCommand,pHeader) BHSM_BspMsg_Header_impl(hMsg,bspCommand,pHeader,NULL)
#define BHSM_BspMsg_Pack8(hMsg,offset,data) BHSM_BspMsg_Pack8_impl(hMsg,offset,data,NULL)
#define BHSM_BspMsg_Pack16(hMsg,offset,data) BHSM_BspMsg_Pack16_impl(hMsg,offset,data,NULL)
#define BHSM_BspMsg_Pack24(hMsg,offset,data) BHSM_BspMsg_Pack24_impl(hMsg,offset,data,NULL)
#define BHSM_BspMsg_Pack32(hMsg,offset,data) BHSM_BspMsg_Pack32_impl(hMsg,offset,data,NULL)
#define BHSM_BspMsg_PackArray(hMsg,offset,pData,length) BHSM_BspMsg_PackArray_impl(hMsg,offset,pData,length,NULL)
#define BHSM_BspMsg_Get8(hMsg,offset,data) BHSM_BspMsg_Get8_impl(hMsg,offset,data,NULL)
#define BHSM_BspMsg_Get16(hMsg,offset,data) BHSM_BspMsg_Get16_impl(hMsg,offset,data,NULL)
#define BHSM_BspMsg_Get24(hMsg,offset,data) BHSM_BspMsg_Get24_impl(hMsg,offset,data,NULL)
#define BHSM_BspMsg_Get32(hMsg,offset,data) BHSM_BspMsg_Get32_impl(hMsg,offset,data,NULL)
#define BHSM_BspMsg_GetArray(hMsg,offset,data,length) BHSM_BspMsg_GetArray_impl(hMsg,offset,data,length,NULL)
#endif

/* BSP Message debug/implementation functions */
BERR_Code BHSM_BspMsg_Header_impl( BHSM_BspMsg_h hMsg, BCMD_cmdType_e bspCommand, BHSM_BspMsgHeader_t *pHeader, char *pCommand  );
BERR_Code BHSM_BspMsg_Pack8_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint8_t data, char *pCommand );
BERR_Code BHSM_BspMsg_Pack16_impl ( BHSM_BspMsg_h hMsg, unsigned int offset, uint16_t data, char *pCommand );
BERR_Code BHSM_BspMsg_Pack24_impl ( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t data, char *pCommand );
BERR_Code BHSM_BspMsg_Pack32_impl ( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t data, char *pCommand );
BERR_Code BHSM_BspMsg_PackArray_impl ( BHSM_BspMsg_h hMsg, unsigned int offset, const uint8_t *pData, unsigned int length, char *pCommand );
BERR_Code BHSM_BspMsg_Get8_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint8_t *pData, char *pCommand );
BERR_Code BHSM_BspMsg_Get16_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint16_t *pData, char *pCommand );
BERR_Code BHSM_BspMsg_Get24_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t *pData, char *pCommand );
BERR_Code BHSM_BspMsg_Get32_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint32_t *pData, char *pCommand );
BERR_Code BHSM_BspMsg_GetArray_impl( BHSM_BspMsg_h hMsg, unsigned int offset, uint8_t  *pData, unsigned int length, char *pCommand );

#ifdef __cplusplus
}
#endif

#endif /* BHSM_BSP_MSG_H__ */
