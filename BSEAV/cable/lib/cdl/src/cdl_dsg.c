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
 * Module Description:  OpenCable Common Download cablecard interface. Functions
 *                      in this module provides ADPU interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "cdl_common.h"
#include "b_cdl_lib.h"
#if defined(MPOD_SUPPORT) 
#include "cdl_adpu.h"
#endif
#include "cdl_estb.h" /* cvc and digital signature */
#include "cdl_ctl.h"
#include "cdl_dsg.h"


BDBG_MODULE(cdl_dsg);


int cdl_dsg_init() {

	BDBG_MSG(("Enter %s\n", __FUNCTION__));	
	return 0;
}

int cdl_dsg_uninit(){
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	return 0;
}

#if defined(MPOD_SUPPORT) 
/* return 1 if succed, 0 if fail */
int cdl_dsg_cvt_process(void * unused1, unsigned char *data, unsigned long len){
	unsigned char * cvt_data = data;
	unsigned long length = len;
	unsigned long sig_len;
	/* BDBG_ERR(("not implemented")); */


    /* skip 3 byte MPEG section wrapper (5.1.2.1.3) */
    cvt_data += 3;
    length -= 3; 

#if defined(CDL_SUPPORT_ENABLE_SIGNED_IMAGE)
	/* verify digital signature */
	if ((sig_len = cdl_estb_authenticate_cvt(cvt_data, length)) < 0) {
		BDBG_ERR(("CVT authenticate failed\n"));
		return 0;
	}
#else
    sig_len = 0;
    BDBG_ERR((" skip cvt authentication, sig_len = %d", sig_len));
#endif
	BDBG_ERR(("digital signature length %d", sig_len));
	cvt_data += sig_len;
    /* Skip another 4 bytes of ADPU header(tag + len) */
    cvt_data += 4;
    length -= 4; 

	/* Parse CVT */
	data = cvt_data;
	len = length - sig_len;
    
	return cdl_adpu_cvt_type2_process(NULL, data, len);
}
#else
int cdl_dsg_cvt_type2_process(void * unused1, unsigned char *data, unsigned long len){
	BDBG_ERR(("Not supported"));
	return 0;

}
#endif /* MPOD_SUPPORT */
