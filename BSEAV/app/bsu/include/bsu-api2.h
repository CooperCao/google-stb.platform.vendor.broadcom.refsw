/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 ***************************************************************************/
#ifndef __BSU_API2_H__
#define __BSU_API2_H__

extern struct bsu_api *xapi;

#define nvram_open xapi->xfn_nvram_open
#define nvram_close xapi->xfn_nvram_close
#define nvram_read xapi->xfn_nvram_read
#define nvram_write xapi->xfn_nvram_write
#define nvram_erase xapi->xfn_nvram_erase

#define net_init xapi->xfn_net_init
#define net_uninit xapi->xfn_net_uninit
#define do_dhcp_request xapi->xfn_do_dhcp_request

#ifdef MIPS_SDE
    #define cfe_set_envdevname xapi->xfn_cfe_set_envdevname
    #define cfe_docommands xapi->xfn_cfe_docommands
    #define cfe_errortext xapi->xfn_cfe_errortext
#else
    #define bolt_set_envdevname xapi->xfn_bolt_set_envdevname
    #define bolt_docommands xapi->xfn_bolt_docommands
    #define bolt_errortext xapi->xfn_bolt_errortext
#endif
#define console_readline xapi->xfn_console_readline

//#define bolt_malloc_aligned xapi->xfn_malloc_aligned
//#define bolt_free xapi->xfn_free

#define fs_close xapi->xfn_fs_close
#define fs_open xapi->xfn_fs_open
#define fs_seek xapi->xfn_fs_seek
#define fs_read xapi->xfn_fs_read
#define fs_init xapi->xfn_fs_init
#define fs_uninit xapi->xfn_fs_uninit

#endif /* __BSU_API2_H__ */
