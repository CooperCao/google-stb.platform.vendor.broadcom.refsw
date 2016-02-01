/***************************************************************************
 *     (c)2007-2008 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:  OpenCable Common Download scurity. Functions
 *                      in this module are used to verify the signature 
 *                      of a monolithic image
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifdef _CFE_
#include "lib_types.h"
#include "lib_string.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bsp_config.h"
#include "disasm.h"
#include "addrspace.h"

#else
#err "_CFE_ not defined"
#endif
#include "cdl_image_brcm.h"
#include "cdl_sec_brcm.h"

/* #define DEBUG_BSECK 1    */

#ifndef BDBG_ERR
#define BDBG_ERR(format...)  printf format
#endif

#ifndef BDBG_MSG
#define BDBG_MSG(format...)  printf format
#endif

#ifdef _CFE_
#define off_t int
#endif


static char tmp_str[128];
static int image_validated = 0; /* flag for image validation
                                 * > 0, validation succeeded
                                 * = 0, to be validated
                                 * < 0, validation failed
                                 */
#define KEY_ID 4
#define REGION_ID 2

int cdl_sec_brcm_check_integrity_from_buffer(unsigned char * data, int data_len,
                                             unsigned char * sig, 
                                             unsigned int code_rule,
                                             int sw_key_id) {
    int ret = -1;
    /* 
     * calculate sha digest for data 
     */
    if (data_len != ((data_len/4)*4)) {
        
        BDBG_ERR(("Invalid data_len %lx", data_len));
        data_len = ((data_len/4)*4);
        BDBG_ERR(("Aborted\n"));
        return -1;
    }

    if (sw_key_id == KEY_ID_NO_KEY) {
        xprintf("\n!!! Unsigned image, bypass signature check !!!!\n");
        ret = 1;
        goto out;
    }
    else {
        xprintf("\n!!! unsupported sw key id %d\n", sw_key_id);
        ret = -1;
        goto out;
    }
 out:
    return ret;
}

extern int flash_erase_dev(char *flashdev);


/* 
 * validate image
 * return >= 0 if succeed, 
 *        < 0 if fail, it actually never returns if fail, as the box will be rebooted 
 *            by hw or sw in case of sw/hw failure.
 */
int cdl_sec_brcm_image_validation(int bank) {
    unsigned char * data = NULL;
    unsigned int data_len = 0;
    unsigned int * sig = NULL;
    int ret = -1;
    unsigned int code_rule = 0;
    int sw_key_id = KEY_ID_NO_KEY; /* for debug purpose */

    if (image_validated > 0) {
        /* xprintf("Image already validated\n"); */
        return image_validated;
    } else if (image_validated < 0) {
        /* xprintf("Previous image validation failed\n"); */
        return image_validated;        
    }

    /* image validation */
	xprintf("[%d]Validating image signature\n", bank);

    data = (uint8_t *) KERNADDR(FLASH_STAGING_BUFFER_IMAGE);
    data = (unsigned char * )(UNCADDR((unsigned long)data));
    xprintf("data = 0x%08x\n", (unsigned int)data);

    /* read signed  image from flash */
    /* reconstruct mono image from flash */
    ret = cdl_image_brcm_load_image(data, &data_len, bank);
    if (ret < 0) {
        xprintf("[%d]failed to validate image (load)\n", bank);
        goto out;
    }

    /* check image header */
    ret = cdl_image_brcm_check_header(data, data_len);
    if (ret < 0) {
        xprintf("failed to validate image (image_header)\n");
        goto out;
    }
    if (cdl_image_brcm_get_code_rule(data, data_len, &code_rule) < 0) {
        code_rule = 0;
        xprintf("failed to validate image (code rule)\n");
        ret = -1;
        goto out;
    }
    if (cdl_image_brcm_get_sw_key_id(data, data_len, &sw_key_id) < 0) {
        sw_key_id = KEY_ID_BRCM_TEST_KEY;
        xprintf("Cannot get sw key id\n");
        ret = -1;
        goto out;
    }
    if (sw_key_id == KEY_ID_NO_KEY) {
        xprintf("\n!!! Unsigned image, bypass signature check !!!!\n");
        ret = 1;
        image_validated = 1;
        goto out;
    }
    if (sw_key_id == KEY_ID_NO_KEY) {
        xprintf("\n!!! Unsigned image, bypass signature check !!!!\n");
        ret = 1;
        goto out;
    }

    /* check the checksum  */
    /*
     * last reboot was caused by image validation failure
     */
    if (cdl_image_brcm_validate_checksum(data, data_len) < 0) {
        xprintf("failed to validate image (checksum)\n");
        ret = -1;
        goto out;
    }



    if (cdl_image_brcm_status_init(data, data_len) < 0) {
        xprintf("Cannot init status\n");
        ret = -1;
        goto out;
    }


    /* the signature is attaced at the end of image */
    data_len -= DIG_SIG_LEN;
    sig =(unsigned int *) ((unsigned char *)data + data_len);    
    
    /* set bit 0 of scratch register, to indicate start of image validation.
     * If succeed, kernel will clear the bit (stblinux-2.6.18/arch/mips/brcmstb/common/prom.c).
     */
    /* compare */
    if (cdl_sec_brcm_check_integrity_from_buffer(data, data_len, (unsigned char *)sig, code_rule, 
                                                 sw_key_id) >= 0) {
        xprintf("[%d]Image signature ok\n", bank);
        ret = 1;
        image_validated = 1;
        goto out;
    } else {
        ret = -1;
    }
 out:

    if (ret < 0) {
        /*
         * Requested by CVC, erase whole kernel partition if image
         * validation failed.
         */
        xprintf("[%d]Image verification failed\n", bank);
        image_validated = -1;
        sprintf(tmp_str, "nandflash0.bank%d", bank);
        xprintf("Debug only: do not erase %s\n", tmp_str);
        /*        xprintf("Halt CPU...\n");
        halt_cpu();
        */
    }
    return ret;
}

/* #if CFG_SPLASH */
#if 0
int cdl_sec_brcm_splash_validation(void) {
    printf("%s: obsolete\n", __FUNCTION__);
    return 0;
}
#endif /* CFG_SPLASH*/
