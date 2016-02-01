/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
