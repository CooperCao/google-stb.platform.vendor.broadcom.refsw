/******************************************************************************
 *    (c)2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *****************************************************************************/

#ifndef __NAND_UTILS_H
#define __NAND_UTILS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


#define EBADBLK     165  /*Bad block was found*/

typedef enum sfu_packet_type_enum
{
	SFU_START_PACK = 0,
	SFU_MID_PACK = 1,
	SFU_END_PACK = 2
}sfu_packet_type_e;

typedef void (*callbackFunc)(
                void *callback_context, 
                size_t total_input_bytes, 
                bool   erasing, 
                size_t bytes_erased_so_far, 
                bool   writing, 
                size_t bytes_written_so_far,
                size_t total_device_space);

typedef int (*decryptFunc)(
                unsigned char *pData,
                unsigned int size,
                int packet_type,
                unsigned char* pHash);
                
typedef int (*allocFunc)(unsigned char** buf, unsigned int size);

typedef void (*freeFunc)(void* buf);

typedef struct callback_struct{
     callbackFunc pCallback;
     decryptFunc pDecrypt;
     allocFunc pAlloc;
     freeFunc pFree;
}nand_utils_callback_t;

typedef struct{
    int (*vopen)(const char *pathname, int flags, ...);
    off_t (*vlseek)(int fildes, off_t offset, int whence);   
    ssize_t (*vread)(int fd, void *buf, size_t count);        
    int (*vclose)(int fd);
} nand_utils_virtual_file_funcs_t;

typedef struct nand_write_param 
{
    const  char  *mtd_device;
    const  char  *img;
    int    mtdoffset;
    int    quiet;
    int    writeoob;
    int    autoplace;
    int    forcejffs2;
    int    forceyaffs;
    int    forcelegacy;
    int    noecc;
    int    pad;
    int    blockalign;
} NAND_WRITE_PARAM;


typedef struct nand_write_extra_param
{
    int bErase;
    unsigned char *img_ptr;
    size_t img_size;
}NAND_WRITE_EXTRA_PARAM;

typedef struct nand_erase_param 
{
    const  char  *mtd_device;
    int    quiet;
    int    jffs2;
}NAND_ERASE_PARAM;

// Prototypes
int nandutils_setExtraParms(struct nand_write_extra_param* p_ExtraParam);
int nandutils_nandwrite(NAND_WRITE_PARAM *param, void *callback_context, callbackFunc callback);
int nandutils_nandwrite_and_decrypt(NAND_WRITE_PARAM *param, void *callback_context, nand_utils_callback_t* funcs);
int nandutils_nandwrite_and_decrypt_file_funcs(NAND_WRITE_PARAM *param, void *callback_context, nand_utils_callback_t* funcs, nand_utils_virtual_file_funcs_t *file_funcs);
int nandutils_flash_erase(NAND_ERASE_PARAM *param, void *callback_context, callbackFunc callback);

// Inline
#if 0
inline int INVALID_PAGE_SIZE(int oobsize, int writesize)
{
        if ( !(oobsize == 8 && writesize == 256) &&
             !(oobsize == 16 && writesize == 512) &&
             !(oobsize == 64 && writesize == 2048)&&
             !(oobsize == 128 && writesize == 4096)&&
             !(oobsize == 216 && writesize == 4096)&&
             !(oobsize == 224 && writesize == 4096)&&
             !(oobsize == 432 && writesize == 8192)&&
             !(oobsize == 448 && writesize == 8192))
        {
            return 1;
        }
        else
        {
            return 0;
        }
}
#endif

#ifdef __cplusplus
}
#endif

#endif //__NAND_UTILS_H
