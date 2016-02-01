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
 * Module Description:  general definitions, shared by cdl library files
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef CDL_COMMON_H
#define CDL_COMMON_H

/* include all OS related defines here */
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <pthread.h>
#include <signal.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <semaphore.h>
#ifdef HOST_ONLY
/* debug macros */
#define BDBG_NOP() (void ) 0
#define BDBG_MODULE(module) extern int bdbg_unused

#define BDBG_MSG(format) BDBG_NOP()
#define BDBG_WRN(format) BDBG_NOP() 
/*
#define BDBG_MSG(format) printf format 
#define BDBG_WRN(format) printf format 
*/
#define BDBG_ERR(format...)  printf format

#define BDBG_ENTER(function)
#define BDBG_LEAVE(function)
#else
#include "bstd.h" /* for BDBG... */
#include "b_os_lib.h"
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t  unsigned char

#define int32_t int
#define int16_t short
#define int8_t  char
#endif

/* Task Priority */
typedef enum _cdl_thread_priority
{
    PRI_LOWEST        = 0,
    PRI_BELOW_NORMAL  = 1,
    PRI_NORMAL        = 2, 
    PRI_ABOVE_NORMAL  = 3,
    PRI_HIGHEST       = 4
} cdl_thread_priority;


typedef struct _cdl_thread_info
{
	pthread_t   thread_id;		  /* thread Id/Handle */
	pthread_attr_t	attr;        /* thread attributes */
	struct  sched_param param;    /* thread sched param */
	void     *thread_fn;		  /* Task function */
	void     *thread_param;	  /* Parameter passed to task */
	void     *private_info;	  /* Extra Info associated with Task function */
	cdl_thread_priority   priority;		  /* Priority Level for Task */
	int exit; 
} cdl_thread_info;

/* these environment variables are imported from linux environemnt variable,
 * most of the varaibles are for debugging purpose. In the real system, they 
 * should be customized.
 * use export ENV_NAME="env value" to set the values before running the app.
 */
#define CDL_ENV_VENDOR_ID            "vendor_id"
#define CDL_ENV_VENDOR_ID_DFT        "0x00123456"
#define CDL_ENV_HW_VERSION_ID        "hw_version_id"
#define CDL_ENV_HW_VERSION_ID_DFT    "789"
#define CDL_ENV_FW_NAME              "fw_name"
#define CDL_ENV_FW_NAME_DFT          "ocap0.bin"
#define CDL_ENV_TRIGGER              "cdl_trigger"
#define CDL_ENV_TRIGGER_DFT          "ecm,estb" /* enable all download */
#define CDL_ENV_NEW_IMG              "cdl_new_img"
#define CDL_ENV_NEW_IMG_DFT          "flash0.avail0" 
#define CDL_ENV_DLD_SLEEP            "cdl_dld_sleep"
#define CDL_ENV_DLD_SLEEP_DFG        "0" 


int cdl_create_thread( cdl_thread_info *thread_info, void *thread_fn, void *thread_param );
int cdl_kill_thread( cdl_thread_info *thread_info, int wait );
int cdl_detach_thread( cdl_thread_info *thread_info );
int cdl_wait_thread_done( cdl_thread_info *thread_info );
int cdl_set_thread_priority(	cdl_thread_info *thread_info, cdl_thread_priority priority );

#endif  /* CDL_COMMON_H */
