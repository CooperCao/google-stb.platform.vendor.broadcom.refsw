/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef _linux_osl_utils_h_
#define _linux_osl_utils_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/string.h>       /* for mem*, str* */
#include <typedefs.h>
#include <linuxver.h>

/* Global ASSERT type */
extern uint32 g_assert_type;

#ifdef CONFIG_PHYS_ADDR_T_64BIT
#define PRI_FMT_x       "llx"
#define PRI_FMT_X       "llX"
#define PRI_FMT_o       "llo"
#define PRI_FMT_d       "lld"
#else
#define PRI_FMT_x       "x"
#define PRI_FMT_X       "X"
#define PRI_FMT_o       "o"
#define PRI_FMT_d       "d"
#endif /* CONFIG_PHYS_ADDR_T_64BIT */
/* ASSERT */
#if defined(BCMDBG_ASSERT) || defined(BCMASSERT_LOG)
	#define ASSERT(exp) \
	  do { if (!(exp)) osl_assert(#exp, __FILE__, __LINE__); } while (0)
extern void osl_assert(const char *exp, const char *file, int line);
#else
	#ifdef __GNUC__
		#define GCC_VERSION \
			(__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
		#if GCC_VERSION > 30100
			#define ASSERT(exp)	do {} while (0)
		#else
			/* ASSERT could cause segmentation fault on GCC3.1, use empty instead */
			#define ASSERT(exp)
		#endif /* GCC_VERSION > 30100 */
	#endif /* __GNUC__ */
#endif /* BCMDBG_ASSERT || BCMASSERT_LOG */

#define OSL_ERROR(bcmerror)	osl_error(bcmerror)
extern int osl_error(int bcmerror);

/* the largest reasonable packet buffer driver uses for ethernet MTU in bytes */
#define	PKTBUFSZ	2048   /* largest reasonable packet buffer, driver uses for ethernet MTU */

#define OSH_NULL   NULL

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 29)
extern uint64 osl_sysuptime_us(void);
#define OSL_SYSUPTIME()		((uint32)jiffies_to_msecs(jiffies))
#define OSL_SYSUPTIME_US()	osl_sysuptime_us()
#else
#define OSL_SYSUPTIME()		((uint32)jiffies * (1000 / HZ))
#error "OSL_SYSUPTIME_US() may need to be defined"
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 29) */

/* get processor cycle count */
#define OSL_GETCYCLES(x)	((x) = 0)

#define OSL_RAND()		osl_rand()
extern uint32 osl_rand(void);
extern void osl_cpu_relax(void);
#define OSL_CPU_RELAX() osl_cpu_relax()

extern void osl_preempt_disable(osl_t *osh);
extern void osl_preempt_enable(osl_t *osh);
#define OSL_DISABLE_PREEMPTION(osh)	osl_preempt_disable(osh)
#define OSL_ENABLE_PREEMPTION(osh)	osl_preempt_enable(osh)

/* microsecond delay */
#define	OSL_DELAY(usec)		osl_delay(usec)
extern void osl_delay(uint usec);

#define OSL_SLEEP(ms)			osl_sleep(ms)
extern void osl_sleep(uint ms);

#define OSL_SMP_WMB()	smp_wmb()

typedef struct sk_buff_head PKT_LIST;
#define PKTLIST_INIT(x)		skb_queue_head_init((x))
#define PKTLIST_ENQ(x, y)	skb_queue_head((struct sk_buff_head *)(x), (struct sk_buff *)(y))
#define PKTLIST_DEQ(x)		skb_dequeue((struct sk_buff_head *)(x))
#define PKTLIST_UNLINK(x, y)	skb_unlink((struct sk_buff *)(y), (struct sk_buff_head *)(x))
#define PKTLIST_FINI(x)		skb_queue_purge((struct sk_buff_head *)(x))

#ifdef __cplusplus
}
#endif

#endif	/* _linux_osl_utils_h_ */
