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
 * Module Description:  OpenCable Common Download control interface. Control
 *                      and status info
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef CDL_CTL_H
#define CDL_CTL_H

#define CDL_CTL_MAX_DOWNLOAD_RETRY 3

/*
 * download command
 */
#define CDL_CTL_CMD_DLD_NOW      0  
#define CDL_CTL_CMD_DLD_DEFER    1

/* 
 * Internal download state 
 */
/* 
 * These values should match with cfe, ui_expand.c.
 *
 * Some of these states are shared between CFE and linux, represented by
 * "CDL_STATE" environment variable. 
 * The CDL_STATE state machine is following
 *       +----------+                            +---------+
 *  +--- | REBOOT0  | <----------+      +--------| RESTORE |<--------+
 *  |    +-----+----+            |      |        +---------+         |
 *  |          |                 |      |             /|\  |         |
 *  |          |(11)        (1) /|\    \|/ (9)         |   |(10)     | 
 *  |          |                 |      |              +---+         |
 *  |(2)       +-------------->+-----------+                         | (8)
 *  |           +------------->|  INIT (0) |<------------+           | 
 *  |           |              +-----------+             |           | 
 *  |           | (3)               /|\ (5)              | (7)       |
 *  | CFE       |                    |                   |           |
 *  |    +----------+   (4)     +-----------+ (6)  +----------+      |
 *  +--->| REBOOT1  | --------> | REBOOT2   |----->| REBOOT3  | -----+
 *  |    +----------+           +-----------+      +----------+
 *
 *  (0) Normal operation, CDL library is initialized and waiting for new image. 
 *      CDL_STATE = INIT.
 *  (1) CDL library downloaded and expanded mono image successfully. 
 *      CDL set CDL_STATE = REBOOT0 before reboot.
 *  (2) During the 1st reboot, CFE update CDL_STATE to REBOOT1
 *  (3) If 1st reboot succeed, CDL library update CDL_STATE to INIT, CDL is complete
 *  (4) If 1st reboot failed, during 2nd reboot, CFE update CDL_STATE to REBOOT2 
 *  (5) If 2nd reboot succeed, CDL library update CDL_STATE to INIT, CDL is complete
 *  (6) If 2nd reboot failed, during 3rd reboot, CFE update CDL_STATE to REBOOT3
 *  (7) If 3rd reboot succeed, CDL library update CDL_STATE to INIT, CDL is complete
 *  (8) If 3rd reboot failed, during 4th reboot, CFE update CDL_STATE to RESTORE,
 *      and boot from known-good image. 
 *  (9) If 4th reboot succeed, CDL library knows that last CDL upgrade has failed
 *      it will report to headend if possible, and set CDL_STATE back to INIT state. 
 *  (10) If 4th reboot failed, during 5th reboot, CFE will find out CDL_STATE is RESTORE.
 *       This could be unrecoverable error (none of the banks are bootable). Or 
 *       transient, power was off during 4th reboot, and CDL library does not get a
 *       chance to run step (9). CFE will leave CDL_STATE as RESTORE
 *  (11) To support backward compatibility, during 1st boot, if CFE failed to update 
 *       CDL_STATE (old CFE), CDL library will set CDL_STATE to INIT and consider last
 *       CDL download succeed.
 * 
 */

#define CDL_CTL_ST_INIT             0      /* just initialized, CDL_STATE = "INIT" */
#define CDL_CTL_ST_WAIT_TRIGGER     1      /* trigger method specified ,waiting for trigger */
#define CDL_CTL_ST_DEFERRED_WAIT    2      /* differed wait */
#define CDL_CTL_ST_VERIFY_CVC       3      /* waiting for cvc verification */
#define CDL_CTL_ST_PARSE_CVT        4      /* parse CVT or eCM CFG file */
#define CDL_CTL_ST_DOWNLOAD         5      /* downloading in progress */
#define CDL_CTL_ST_CERT_CHECK       6      /* certificate check */
#define CDL_CTL_ST_AUTH_IMAGE       7      /* authenticating image content*/
#define CDL_CTL_ST_IMAGE_CHECK      8      /* image check */
#define CDL_CTL_ST_DLD_COMPLETE     9      /* download complete, ready to reboot */
#define CDL_CTL_ST_DLD_MAX_RETRY    10     /* reached max download retry */
#define CDL_CTL_ST_REBOOT_0         11      /* download ok, reboot, CDL_STATE = "REBOOT0" */
#define CDL_CTL_ST_REBOOT_1         12      /* 1st retry, CDL_STATE = "REBOOT1" */
#define CDL_CTL_ST_REBOOT_2         13     /* 2nd retry, CDL_STATE = "REBOOT2" */
#define CDL_CTL_ST_REBOOT_3         14     /* 3rd retry, CDL_STATE = "REBOOT3" */
#define CDL_CTL_ST_RESTORE          15     /* expand fail, restore current image , CDL_STATE = "RESTORE"*/
#define CDL_CTL_ST_EXPAND_DONE      16     /* expand suceed */
#define CDL_CTL_ST_SUCCEED          17     /* firmware upgrade succeeded */
#define CDL_CTL_ST_MAX              18     /* maximum state */
         
#define CDL_CTL_ST_CERT_FAILED      -(CDL_CTL_ST_CERT_CHECK)      /* certificate check failed*/
#define CDL_CTL_ST_IMAGE_DAMAGED    -(CDL_CTL_ST_IMAGE_CHECK)      /* image damaged */

/*
 * trigger flag
 */
#define CDL_CTL_TRIGGER_NONE          0x00
#define CDL_CTL_TRIGGER_ECM_CFG_FILE  0x01
#define CDL_CTL_TRIGGER_SNMP          0x02
#define CDL_CTL_TRIGGER_CA_CVT        0x04
#define CDL_CTL_TRIGGER_DSG_CVT       0x08

typedef struct cdl_ctl * cdl_ctl_t;

int cdl_ctl_init(void);
int cdl_ctl_uninit(void);
int cdl_ctl_get_state(void);
uint8_t cdl_ctl_get_method(void);
uint8_t cdl_ctl_get_cmd(void);
uint8_t cdl_ctl_get_trigger(void);
uint32_t cdl_ctl_get_vendor_id(void);
uint32_t cdl_ctl_get_hw_version_id(void);
uint32_t cdl_ctl_get_group_id(void);

int cdl_ctl_set_state(int state);
void cdl_ctl_set_failed_state(void);
int cdl_ctl_set_method(uint8_t method);
int cdl_ctl_set_cmd(uint8_t cmd);
int cdl_ctl_set_trigger(uint8_t trigger);

int cdl_ctl_is_busy(void);
int cdl_ctl_wait_for_ecm_notification(void);
int cdl_ctl_wakeup_by_ecm_notification(void); 
int cdl_ctl_need_authenticate(void); 
int cdl_ctl_need_upgrade(cdl_adpu_cvt_t h_cvt);
int cdl_ctl_get_secure_header_len(void);
void cdl_ctl_set_secure_header_len(int len);
int cdl_ctl_allow_unsigned_image();
#endif  /* CDL_CTL_H */
