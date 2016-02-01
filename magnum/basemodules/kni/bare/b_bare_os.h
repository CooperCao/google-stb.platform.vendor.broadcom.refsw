/***************************************************************************
*     (c)2004-2011 Broadcom Corporation
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
* API Description:
*   API name: Platform linuxuser
*    linuxkernel OS routines
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef B_BARE_OS_H__
#define B_BARE_OS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t b_bare_os_tick;

typedef struct b_bare_os_lock_tag *b_bare_os_lock;
typedef struct b_bare_os_signal_tag *b_bare_os_signal;
typedef struct b_bare_os_thread_tag *b_bare_os_thread;

typedef void (*b_bare_os_special_interrupt_handler)(int irqNum);

#define B_BARE_OS_ETIMEOUT  1
#define B_BARE_OS_ERROR     (-1)

typedef struct b_bare_os_thread_settings {
    unsigned priority;
    unsigned stack_size;
} b_bare_os_thread_settings;

typedef struct b_bare_os_interface {
    /* calls for bare/bkni.c, nexus_platform_os.c */
    void *(*malloc)(size_t size);
    void (*free)(void *ptr);
    void (*print_debug)(bool high_priority, const char *str);
    b_bare_os_tick (*current_tick)(void);
    unsigned (*tick_diff)(b_bare_os_tick future, b_bare_os_tick past);
    void *(*mmap)(bool cached, uint32_t addr, size_t size);
    void (*unmmap)(void *addr, size_t size);
    void (*dcache_flush)(void *addr, size_t size);
    void (*delay)(unsigned delay_us);
    void (*sleep)(unsigned delay_ms);
    b_bare_os_lock (*lock_create)(void);
    int (*lock_acquire)(b_bare_os_lock lock);
    int (*lock_try_acquire)(b_bare_os_lock lock);
    void (*lock_release)(b_bare_os_lock lock);
    void (*lock_destroy)(b_bare_os_lock lock);
    b_bare_os_signal (*signal_create)(void);
    int (*signal_wait)(b_bare_os_signal signal, unsigned timeout_ms);
    void (*signal_set)(b_bare_os_signal signal);
    void (*signal_destroy)(b_bare_os_signal signal);
    int (*copy_from_process)(void *dest, const void *src, unsigned size);
    int (*copy_to_process)(void *dest, const void *src, unsigned size);
    
    /* calls for bare/nexus_base_os.c */
    b_bare_os_thread (*thread_create)(const char *name, void (*callback)(void *), void *context, b_bare_os_thread_settings *p_settings);
    void (*thread_destroy)(b_bare_os_thread thread);
    const char *(*getenv)(const char *name);
    void (*setenv)(const char *name, const char *value);
    void (*terminate_process)(unsigned id);
    
    /* calls for bare/nexus_platform_os.c */
    void (*atomic_update)(unsigned register_address, uint32_t mask, uint32_t value);
    int  (*connect_interrupt)(const char *name, unsigned irq_num, 
        void (*callback)(void *,int), void *context_ptr, int context_int, 
        bool shared, b_bare_os_special_interrupt_handler special_handler);
    int  (*enable_interrupt)(unsigned irq_num);
    void (*disable_interrupt)(unsigned irq_num);
    void (*disconnect_interrupt)(unsigned irq_num);
    b_bare_os_lock (*get_interrupt_lock)(void); /* special lock for interrupt context */
	void *end_of_functions;
} b_bare_os_interface;

/* Pointer and accessor to get to the bare interface structure */
b_bare_os_interface *b_get_bare_os(void);	/* Located in b_bare_os.c on the driver side; stubs_rev.S on the firmware side */
extern b_bare_os_interface *pb_bare_os;		/* Located in b_base_os.c on the driver side; kni/bare/bkni.c on the firmware side */

/* These functions should be be only called on the driver side */
int b_bare_os_init(void);
void b_bare_os_uninit(void);

#ifdef __cplusplus
}
#endif

#endif /* B_BARE_OS_H__ */

