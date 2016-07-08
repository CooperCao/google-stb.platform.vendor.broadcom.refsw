/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "bstd.h"                /* standard types */
#include "breg_mem.h"            /* Chip register access memory mapped */
#include "bkni.h"                /* Memory management */
#include "bdbg.h"                /* Debug message */

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#include "brdc.h"
#include "brdc_dbg.h"
#include "brdc_private.h"
#include "brdc_blockout_priv.h"

/* Interrupt Id */
#include "bchp_int_id_bvnf_intr2_0.h"
#include "bchp_int_id_bvnf_intr2_1.h"
#ifdef BCHP_RDC_hw_configuration_max_descriptor_number_DEFAULT
#if(BCHP_RDC_hw_configuration_max_descriptor_number_DEFAULT == BCHP_RDC_hw_configuration_max_descriptor_number_MAX64)
#include "bchp_int_id_bvnf_intr2_8.h"
#endif
#endif

#ifdef BCHP_RDC_hw_configuration_max_trigger_number_DEFAULT
#if(BCHP_RDC_hw_configuration_max_trigger_number_DEFAULT == BCHP_RDC_hw_configuration_max_trigger_number_MAX96)
#include "bchp_int_id_bvnf_intr2_16.h"
#include "bchp_int_id_bvnf_intr2_9.h"
#elif(BCHP_RDC_hw_configuration_max_trigger_number_DEFAULT == BCHP_RDC_hw_configuration_max_trigger_number_MAX64)
#include "bchp_int_id_bvnf_intr2_9.h"
#endif
#endif

BDBG_MODULE(BRDC);
BDBG_OBJECT_ID(BRDC_RDC);
BDBG_OBJECT_ID(BRDC_SLT);
BDBG_OBJECT_ID(BRDC_LST);

/* HW7445-1476, use SW blockout before the fixes in */
#if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7366) || (BCHP_CHIP==74371) || \
    ((BCHP_CHIP==7439) && (BCHP_VER == BCHP_VER_A0))|| \
    (BCHP_CHIP==7364)|| (BCHP_CHIP==7250)|| (BCHP_CHIP==7563)|| (BCHP_CHIP==7543)
#define BRDC_P_SUPPORT_HW_BLOCKOUT_WORKAROUND              (1)
#else
#define BRDC_P_SUPPORT_HW_BLOCKOUT_WORKAROUND              (0)
#endif

/* Support register udpate blockout by HW? */
#if (BCHP_RDC_br_0_start_addr && (!BRDC_P_SUPPORT_HW_BLOCKOUT_WORKAROUND))
#define BRDC_P_SUPPORT_HW_BLOCKOUT         (1)
#else
#define BRDC_P_SUPPORT_HW_BLOCKOUT         (0)
#endif

#ifdef BCHP_RDC_desc_0_tm_snapshot
#define BRDC_P_SUPPORT_TIMESTAMP           (1)
#else
#define BRDC_P_SUPPORT_TIMESTAMP           (0)
#endif

/* STC flag support */
#if BCHP_RDC_stc_flag_5
#define BRDC_P_STC_FLAG_COUNT              (6)
#elif BCHP_RDC_stc_flag_4
#define BRDC_P_STC_FLAG_COUNT              (5)
#elif BCHP_RDC_stc_flag_3
#define BRDC_P_STC_FLAG_COUNT              (4)
#elif BCHP_RDC_stc_flag_2
#define BRDC_P_STC_FLAG_COUNT              (3)
#elif BCHP_RDC_stc_flag_1
#define BRDC_P_STC_FLAG_COUNT              (2)
#elif BCHP_RDC_stc_flag_0
#define BRDC_P_STC_FLAG_COUNT              (1)
#else
#define BRDC_P_STC_FLAG_COUNT              (0)
#endif

/* Number of trigger */
#ifdef BCHP_RDC_hw_configuration_max_trigger_number_MASK
#if (BCHP_RDC_hw_configuration_max_trigger_number_DEFAULT == BCHP_RDC_hw_configuration_max_trigger_number_MAX128)
#define BRDC_P_SUPPORT_TRIGGER_MASK                  (4)
#elif(BCHP_RDC_hw_configuration_max_trigger_number_DEFAULT == BCHP_RDC_hw_configuration_max_trigger_number_MAX96)
#define BRDC_P_SUPPORT_TRIGGER_MASK                  (3)
#elif(BCHP_RDC_hw_configuration_max_trigger_number_DEFAULT == BCHP_RDC_hw_configuration_max_trigger_number_MAX64)
#define BRDC_P_SUPPORT_TRIGGER_MASK                  (2)
#else
#define BRDC_P_SUPPORT_TRIGGER_MASK                  (1)
#endif
#else /* Does not have BCHP_RDC_hw_configuration_max_trigger_number_MASK*/
#define BRDC_P_SUPPORT_TRIGGER_MASK                  (1)
#endif

#if(BRDC_P_SUPPORT_TIMESTAMP)
/* RDC timers is 32 bit */
#define BRDC_P_MAX_TIMER_COUNTER           (0xffffffff)

/* Chips (version earlier than  3.1.0) run at 108Mhz :
 * 7125, 7340, 7342, 7408 , 7420, 7468, 7550 */
#if (BCHP_CHIP==7125) || (BCHP_CHIP==7340) || (BCHP_CHIP==7342) || \
    (BCHP_CHIP==7408) || (BCHP_CHIP==7420) || (BCHP_CHIP==7468) || \
    (BCHP_CHIP==7550)
#define BRDC_P_TIMESTAMP_CLOCK_RATE        (108ul)
#elif (BCHP_CHIP==7445) && (BCHP_VER>=BCHP_VER_D0) || \
      (BCHP_CHIP==7145) && (BCHP_VER>=BCHP_VER_B0) || \
      (BCHP_CHIP==7439) || (BCHP_CHIP==7366) ||(BCHP_CHIP==7271) || \
      (BCHP_CHIP==7268) || (BCHP_CHIP==7260) ||(BCHP_CHIP==7278)
/* Chips (version later than or equal to 3.1.0) run at 324Mhz */
#define BRDC_P_TIMESTAMP_CLOCK_RATE        (324ul)
#else
/* Chips (version later than or equal to 3.1.0) run at 216Mhz */
#define BRDC_P_TIMESTAMP_CLOCK_RATE        (216ul)
#endif
#endif

/* Shorthands to reduce typing. */
#ifdef BCHP_RDC_hw_configuration_max_descriptor_number_MASK
/* new HW config max despcriptor field */
#if(BCHP_RDC_hw_configuration_max_descriptor_number_DEFAULT == BCHP_RDC_hw_configuration_max_descriptor_number_MAX64)
/* 64 slots */
#define BRDC_P_MAKE_SLOT_WORD_0_INFO(slot_id)                       \
{                                                                   \
    (BCHP_INT_ID_BVNF_INTR2_0_RDC_DESC_##slot_id##_INTR),           \
    BRDC_P_TRACK_REG_ADDR(BRDC_P_SCRATCH_REG_START+(slot_id))       \
}
#define BRDC_P_MAKE_SLOT_WORD_1_INFO(slot_id)                       \
{                                                                   \
    (BCHP_INT_ID_BVNF_INTR2_8_RDC_DESC_##slot_id##_INTR),           \
    BRDC_P_TRACK_REG_ADDR(BRDC_P_SCRATCH_REG_START+(slot_id))       \
}
#else
/* 32 slots */
#define BRDC_P_MAKE_SLOT_WORD_0_INFO(slot_id)                       \
{                                                                   \
    (BCHP_INT_ID_BVNF_INTR2_0_RDC_DESC_##slot_id##_INTR),           \
    BRDC_P_TRACK_REG_ADDR(BRDC_P_SCRATCH_REG_START+(slot_id))       \
}
#define BRDC_P_MAKE_SLOT_WORD_1_INFO(slot_id)                       \
{                                                                   \
    (BRDC_P_NULL_BINTID),                                           \
    BRDC_P_TRACK_REG_ADDR(BRDC_P_SCRATCH_REG_START+(slot_id))       \
}
#endif
#else
/* old HW config */
#ifdef BCHP_INT_ID_BVNF_INTR2_0_RDC_DESC_0_INTR
#define BRDC_P_MAKE_SLOT_WORD_0_INFO(slot_id)                       \
{                                                                   \
    (BCHP_INT_ID_BVNF_INTR2_0_RDC_DESC_##slot_id##_INTR),           \
    BRDC_P_TRACK_REG_ADDR(BRDC_P_SCRATCH_REG_START+(slot_id))       \
}
#define BRDC_P_MAKE_SLOT_WORD_1_INFO(slot_id)                       \
{                                                                   \
    (BRDC_P_NULL_BINTID),                                           \
    BRDC_P_TRACK_REG_ADDR(BRDC_P_SCRATCH_REG_START+(slot_id))       \
}
#else
#define BRDC_P_MAKE_SLOT_WORD_0_INFO(slot_id)                       \
{                                                                   \
    (BCHP_INT_ID_RDC_DESC_##slot_id##_INTR),                        \
    BRDC_P_TRACK_REG_ADDR(BRDC_P_SCRATCH_REG_START+(slot_id))       \
}
#define BRDC_P_MAKE_SLOT_WORD_1_INFO(slot_id)                       \
{                                                                   \
    (BRDC_P_NULL_BINTID),                                           \
    BRDC_P_TRACK_REG_ADDR(BRDC_P_SCRATCH_REG_START+(slot_id))       \
}
#endif
#endif

#ifdef BCHP_RDC_hw_configuration_max_trigger_number_MASK
/* new HW config max trigger field */
#if(BCHP_RDC_hw_configuration_max_trigger_number_DEFAULT == BCHP_RDC_hw_configuration_max_trigger_number_MAX96)
/* 96 triggers */
#define BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(enum_name, int_id, rdb_name)  \
{                                                                       \
    (BRDC_Trigger_##enum_name),                                         \
    (BCHP_INT_ID_BVNF_INTR2_1_RDC_TRIG_##int_id##_INTR),                \
    (BRDC_P_TRIGGER(rdb_name)),                                         \
    (#enum_name)                                                        \
}
#define BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(enum_name, int_id, rdb_name)  \
{                                                                       \
    (BRDC_Trigger_##enum_name),                                         \
    (BCHP_INT_ID_BVNF_INTR2_9_RDC_TRIG_##int_id##_INTR),                \
    (BRDC_P_TRIGGER(rdb_name)),                                         \
    (#enum_name)                                                        \
}
#ifdef BCHP_INT_ID_BVNF_INTR2_16_RDC_TRIG_64_INTR
#define BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(enum_name, int_id, rdb_name)  \
{                                                                       \
    (BRDC_Trigger_##enum_name),                                         \
    (BCHP_INT_ID_BVNF_INTR2_16_RDC_TRIG_##int_id##_INTR),               \
    (BRDC_P_TRIGGER(rdb_name)),                                         \
    (#enum_name)                                                        \
}
#else
#define BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(enum_name, int_id, rdb_name)  \
{                                                                       \
    (BRDC_Trigger_##enum_name),                                         \
    (BCHP_INT_ID_RDC_TRIG_##int_id##_INTR),               \
    (BRDC_P_TRIGGER(rdb_name)),                                         \
    (#enum_name)                                                        \
}
#endif
#elif(BCHP_RDC_hw_configuration_max_trigger_number_DEFAULT == BCHP_RDC_hw_configuration_max_trigger_number_MAX64)
/* 64 triggers */
#define BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(enum_name, int_id, rdb_name)  \
{                                                                       \
    (BRDC_Trigger_##enum_name),                                         \
    (BCHP_INT_ID_BVNF_INTR2_1_RDC_TRIG_##int_id##_INTR),                \
    (BRDC_P_TRIGGER(rdb_name)),                                         \
    (#enum_name)                                                        \
}
#define BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(enum_name, int_id, rdb_name)  \
{                                                                       \
    (BRDC_Trigger_##enum_name),                                         \
    (BCHP_INT_ID_BVNF_INTR2_9_RDC_TRIG_##int_id##_INTR),                \
    (BRDC_P_TRIGGER(rdb_name)),                                         \
    (#enum_name)                                                        \
}
#else
/* 32 triggers */
#define BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(enum_name, int_id, rdb_name)  \
{                                                                       \
    (BRDC_Trigger_##enum_name),                                         \
    (BCHP_INT_ID_BVNF_INTR2_1_RDC_TRIG_##int_id##_INTR),                \
    (BRDC_P_TRIGGER(rdb_name)),                                         \
    (#enum_name)                                                        \
}
#endif
#else
/* old HW config */
#ifdef BCHP_INT_ID_BVNF_INTR2_0_RDC_DESC_0_INTR
#define BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(enum_name, int_id, rdb_name)  \
{                                                                       \
    (BRDC_Trigger_##enum_name),                                         \
    (BCHP_INT_ID_BVNF_INTR2_1_RDC_TRIG_##int_id##_INTR),                \
    (BRDC_P_TRIGGER(rdb_name)),                                         \
    (#enum_name)                                                        \
}
#else
#define BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(enum_name, int_id, rdb_name)  \
{                                                                       \
    (BRDC_Trigger_##enum_name),                                         \
    (BCHP_INT_ID_RDC_TRIG_##int_id##_INTR),                             \
    (BRDC_P_TRIGGER(rdb_name)),                                         \
    (#enum_name)                                                        \
}
#endif
#endif

#define BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(enum_name, int_id, rdb_name)  \
{                                                                       \
    (BRDC_Trigger_##enum_name),                                         \
    (BRDC_P_NULL_BINTID),                                               \
    (BRDC_P_TRIGGER_UNKNOWN_VAL),                                       \
    (#enum_name)                                                        \
}

#define BRDC_P_MAKE_TRIG_INFO_COMB(enum_name, int_id, rdb_name)         \
{                                                                       \
    (BRDC_Trigger_##enum_name),                                         \
    (BRDC_P_NULL_BINTID),                                               \
    (BRDC_P_TRIGGER(rdb_name)),                                         \
    (#enum_name)                                                        \
}


/* Slot Info */
typedef struct
{
    /* When slot done execute it will fire interrupt, and this interrupt id
     * is use in according with BINT (magnum interrupt manager). */
    BINT_Id            SlotIntId;

    /* This address is for the slot to update the count when it execute.  We're
     * using this register to check if the RUL last assigned to slot has been
     * executed or not. */
    uint32_t           ulTrackRegAddr;

} BRDC_P_SlotInfo;

/* INDEX: Slot id.  Slot's INT ID, tracking address mapping, */
static const BRDC_P_SlotInfo s_aRdcSlotInfo[] =
{
    BRDC_P_MAKE_SLOT_WORD_0_INFO(0),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(1),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(2),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(3),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(4),

    BRDC_P_MAKE_SLOT_WORD_0_INFO(5),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(6),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(7),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(8),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(9),

    BRDC_P_MAKE_SLOT_WORD_0_INFO(10),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(11),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(12),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(13),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(14),

    BRDC_P_MAKE_SLOT_WORD_0_INFO(15),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(16),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(17),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(18),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(19),

    BRDC_P_MAKE_SLOT_WORD_0_INFO(20),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(21),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(22),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(23),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(24),

    BRDC_P_MAKE_SLOT_WORD_0_INFO(25),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(26),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(27),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(28),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(29),

    BRDC_P_MAKE_SLOT_WORD_0_INFO(30),
    BRDC_P_MAKE_SLOT_WORD_0_INFO(31),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(32),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(33),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(34),

    BRDC_P_MAKE_SLOT_WORD_1_INFO(35),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(36),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(37),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(38),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(39),

    BRDC_P_MAKE_SLOT_WORD_1_INFO(40),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(41),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(42),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(43),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(44),

    BRDC_P_MAKE_SLOT_WORD_1_INFO(45),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(46),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(47),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(48),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(49),

    BRDC_P_MAKE_SLOT_WORD_1_INFO(50),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(51),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(52),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(53),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(54),

    BRDC_P_MAKE_SLOT_WORD_1_INFO(55),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(56),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(57),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(58),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(59),

    BRDC_P_MAKE_SLOT_WORD_1_INFO(60),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(61),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(62),
    BRDC_P_MAKE_SLOT_WORD_1_INFO(63),

    /* NULL */
    {BRDC_P_NULL_BINTID, BRDC_P_NO_TRACKING_ADDR}
};


/* INDEX: Trigger Id.  Trigger's INT ID!  This chip specifics.
 * DO NOT: ifdefs and nested ifdefs that become impossible to decipher.
 * Look at the BCHP_BRDC_desc_0_config for valid triggers. */
static const BRDC_TrigInfo s_aRdcTrigInfo[] =
{
    /*                    enum_name,    int_id,
     *                                      rdb_name         */
#if ((BCHP_CHIP==7346) || (BCHP_CHIP==7344) || (BCHP_CHIP==7231) || (BCHP_CHIP==73465))

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig0,    8, prim_vec_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig1,    9, prim_vec_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig0,   10, sec_vec_trig_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig1,   11, sec_vec_trig_1 ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDtgTrig0,    12, itu_r656_out_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDtgTrig1,    13, itu_r656_out_1 ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   20, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
#if (BCHP_CHIP==7231) && (BCHP_VER >= BCHP_VER_B0)
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig0, 21, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig1, 22, hd_dvi_0_trig_1),
#else
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig1, -1, UNKNOWN        ),
#endif
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig0,   25, letterbox_0    ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig1,   26, letterbox_1    ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig0,    14, hd_dvi_out_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig1,    15, hd_dvi_out_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     29, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd1Eof,     30, mfd_1_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7429) || (BCHP_CHIP==74295)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   20, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig0, 21, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig1, 22, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig0,   25, letterbox_0    ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig1,   26, letterbox_1    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     29, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd1Eof,     30, mfd_1_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,   8, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,   9, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  10, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  11, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7552) || (BCHP_CHIP==7358) || (BCHP_CHIP==7360) || \
      (BCHP_CHIP==7362) || (BCHP_CHIP==7228) || (BCHP_CHIP==73625)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
#if (BCHP_CHIP==73625)
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig0,    8, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig1,    9, vec_source_0_trig_1),
#else
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig0,    8, prim_vec_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig1,    9, prim_vec_trig_1),
#endif
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
#if (BCHP_CHIP==73625)
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig0,   10, vec_source_1_trig_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig1,   11, vec_source_1_trig_1 ),
#else
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig0,   10, sec_vec_trig_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig1,   11, sec_vec_trig_1 ),
#endif

    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   20, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig0,   25, letterbox_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig0,    14, hd_dvi_out_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig1,    15, hd_dvi_out_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     29, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7364)||(BCHP_CHIP==7250)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eM2mc0Trig,   72, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig0, 44, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig1, 45, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg0,    52, mfd_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg1,    53, mfd_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg0,    54, mfd_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg1,    55, mfd_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,  24, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,  25, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  26, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  27, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  28, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  29, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7420)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig0,    8, prim_vec_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig1,    9, prim_vec_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig0,   10, sec_vec_trig_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig1,   11, sec_vec_trig_1 ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec2Trig0,   16, ter_vec_trig_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec2Trig1,   17, ter_vec_trig_1 ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDtgTrig0,    12, itu_r656_out_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDtgTrig1,    13, itu_r656_out_1 ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(e6560Trig0,   18, itu_r656_0_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(e6560Trig1,   19, itu_r656_0_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(e6561Trig0,   27, itu_r656_1_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(e6561Trig1,   28, itu_r656_1_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   20, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig0, 21, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig1, 22, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig0,   25, letterbox_0    ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig1,   26, letterbox_1    ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig0,    14, hd_dvi_out_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig1,    15, hd_dvi_out_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     29, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd1Eof,     30, mfd_1_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(ePx3d0Trig0,  31, px3d_0         ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7422) || (BCHP_CHIP==7425)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap4Trig0,    8, cap_4_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap4Trig1,    9, cap_4_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),

#if (BCHP_VER >= BCHP_VER_B0)
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
#else
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig0,   10, prim_vec_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig1,   11, prim_vec_trig_1),
#endif

    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),

#if (BCHP_VER >= BCHP_VER_B0)
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
#else
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig0,   12, sec_vec_trig_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig1,   13, sec_vec_trig_1 ),
#endif
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),

#if (BCHP_VER >= BCHP_VER_B0)
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
#else
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDtgTrig0,    16, itu_r656_0_out_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDtgTrig1,    17, itu_r656_0_out_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDtg1Trig0,   18, itu_r656_1_out_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDtg1Trig1,   19, itu_r656_1_out_1),
#endif

    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   22, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig0, 25, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig1, 26, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),

#if (BCHP_VER >= BCHP_VER_B0)
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
#else
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eStg0Trig0,   23, stg_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eStg0Trig1,   24, stg_0_trig_1   ),
#endif

    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),

#if (BCHP_VER >= BCHP_VER_B0)
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
#else
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig0,    20, hd_dvi_out_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig1,    21, hd_dvi_out_1   ),
#endif

    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     28, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd1Eof,     29, mfd_1_eof      ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd2Eof,     30, mfd_2_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),

#if (BCHP_VER >= BCHP_VER_B0)
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,  10, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,  11, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  12, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  13, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig0,  14, vec_source_2_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig1,  15, vec_source_2_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_3Trig0,  16, vec_source_3_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_3Trig1,  17, vec_source_3_trig_1),
#else
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
#endif
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7435)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap4Trig0,    8, cap_4_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap4Trig1,    9, cap_4_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap5Trig0,   10, cap_5_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap5Trig1,   11, cap_5_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   24, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc1Trig,   27, m2mc_trig_1    ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig0, 25, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig1, 26, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     28, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd1Eof,     29, mfd_1_eof      ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd2Eof,     30, mfd_2_eof      ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd3Eof,     31, mfd_3_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,  12, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,  13, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  14, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  15, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig0,  16, vec_source_2_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig1,  17, vec_source_2_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_3Trig0,  18, vec_source_3_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_3Trig1,  19, vec_source_3_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_4Trig0,  20, vec_source_4_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_4Trig1,  21, vec_source_4_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_5Trig0,  22, vec_source_5_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_5Trig1,  23, vec_source_5_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7550)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig0,    8, prim_vec_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig1,    9, prim_vec_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig0,   10, sec_vec_trig_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig1,   11, sec_vec_trig_1 ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDtgTrig0,    12, itu_r656_out_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDtgTrig1,    13, itu_r656_out_1 ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(e6560Trig0,   18, itu_r656_0_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(e6560Trig1,   19, itu_r656_0_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(e6561Trig0,   27, itu_r656_1_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(e6561Trig1,   28, itu_r656_1_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   20, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig0, 21, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eHdDvi0Trig1, 22, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig0,   25, letterbox_0    ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig1,   26, letterbox_1    ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig0,    14, hd_dvi_out_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig1,    15, hd_dvi_out_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     29, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd1Eof,     30, mfd_1_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(ePx3d0Trig0,  31, px3d_0         ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7340) || (BCHP_CHIP==7342)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig0,    8, prim_vec_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig1,    9, prim_vec_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig0,   10, sec_vec_trig_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig1,   11, sec_vec_trig_1 ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   20, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig0,   25, letterbox_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig0,    14, hd_dvi_out_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig1,    15, hd_dvi_out_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     29, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(ePx3d0Trig0,  31, px3d_0         ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7125)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig0,    8, prim_vec_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig1,    9, prim_vec_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig0,   10, sec_vec_trig_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig1,   11, sec_vec_trig_1 ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(e6560Trig0,   18, itu_r656_0_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(e6560Trig1,   19, itu_r656_0_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   20, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig0,   25, letterbox_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig0,    14, hd_dvi_out_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig1,    15, hd_dvi_out_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     29, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(ePx3d0Trig0,  31, px3d_0         ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7468)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig0,    8, prim_vec_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig1,    9, prim_vec_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig0,   10, sec_vec_trig_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig1,   11, sec_vec_trig_1 ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   20, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig0,   25, letterbox_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig0,    14, hd_dvi_out_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig1,    15, hd_dvi_out_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     29, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(ePx3d0Trig0,  31, px3d_0         ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7408)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig0,    8, prim_vec_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec0Trig1,    9, prim_vec_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig0,   10, sec_vec_trig_0 ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eVec1Trig1,   11, sec_vec_trig_1 ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   20, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eLboxTrig0,   25, letterbox_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig0,    14, hd_dvi_out_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eDvoTrig1,    15, hd_dvi_out_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     29, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(ePx3d0Trig0,  31, px3d_0         ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7584) || (BCHP_CHIP==75845)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   20, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     29, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd1Eof,     30, mfd_1_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,   8, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,   9, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  10, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  11, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7563) || (BCHP_CHIP==7543) || (BCHP_CHIP==75635) || (BCHP_CHIP==75525)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eM2mc0Trig,   20, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi0Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eMfd0Eof,     29, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,   8, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,   9, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  10, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  11, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7445)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap4Trig0,    8, cap_4_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap4Trig1,    9, cap_4_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap5Trig0,   10, cap_5_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap5Trig1,   11, cap_5_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap6Trig0,   12, cap_6_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap6Trig1,   13, cap_6_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap7Trig0,   14, cap_7_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap7Trig1,   15, cap_7_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
#if (BCHP_VER < BCHP_VER_D0)
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eM2mc0Trig,   52, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eM2mc1Trig,   53, m2mc_trig_1    ),
#else
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eM2mc0Trig,   72, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eM2mc1Trig,   73, m2mc_trig_1    ),
#endif
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig0, 44, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig1, 45, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
#if (BCHP_VER < BCHP_VER_D0)
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Eof,     52, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Eof,     53, mfd_1_eof      ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd2Eof,     54, mfd_2_eof      ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd3Eof,     55, mfd_3_eof      ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd4Eof,     56, mfd_4_eof      ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd5Eof,     57, mfd_5_eof      ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
#else
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg0,    52, mfd_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg1,    53, mfd_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg0,    54, mfd_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg1,    55, mfd_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd2Mtg0,    56, mfd_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd2Mtg1,    57, mfd_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd3Mtg0,    58, mfd_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd3Mtg1,    59, mfd_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd4Mtg0,    60, mfd_4_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd4Mtg1,    61, mfd_4_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd5Mtg0,    62, mfd_5_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd5Mtg1,    63, mfd_5_trig_1   ),
#endif
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,  24, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,  25, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  26, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  27, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig0,  28, vec_source_2_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig1,  29, vec_source_2_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_3Trig0,  30, vec_source_3_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_3Trig1,  31, vec_source_3_trig_1),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eCmp_4Trig0,  32, vec_source_4_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eCmp_4Trig1,  33, vec_source_4_trig_1),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eCmp_5Trig0,  34, vec_source_5_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eCmp_5Trig1,  35, vec_source_5_trig_1),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eCmp_6Trig0,  36, vec_source_6_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eCmp_6Trig1,  37, vec_source_6_trig_1),
#if (BCHP_VER < BCHP_VER_D0)
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN            ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN            ),
#else
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eVec0Bypass0, 76, vec_hddvi_0_passthr_trig_0),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eVec0Bypass1, 77, vec_hddvi_0_passthr_trig_1),
#endif

#elif (BCHP_CHIP==7145)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap4Trig0,    8, cap_4_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap4Trig1,    9, cap_4_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap5Trig0,   10, cap_5_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap5Trig1,   11, cap_5_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap6Trig0,   12, cap_6_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap6Trig1,   13, cap_6_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap7Trig0,   14, cap_7_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap7Trig1,   15, cap_7_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
#if (BCHP_VER >= BCHP_VER_B0)
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eM2mc0Trig,   72, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eM2mc1Trig,   73, m2mc_trig_1    ),
#else
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eM2mc0Trig,   48, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eM2mc1Trig,   49, m2mc_trig_1    ),
#endif
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig0, 44, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig1, 45, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
#if (BCHP_VER >= BCHP_VER_B0)
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
#else
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Eof,     52, mfd_0_eof      ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Eof,     53, mfd_1_eof      ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd2Eof,     54, mfd_2_eof      ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd3Eof,     55, mfd_3_eof      ),
#endif
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
#if (BCHP_VER >= BCHP_VER_B0)
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg0,    52, mfd_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg1,    53, mfd_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg0,    54, mfd_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg1,    55, mfd_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd2Mtg0,    56, mfd_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd2Mtg1,    57, mfd_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd3Mtg0,    58, mfd_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd3Mtg1,    59, mfd_3_trig_1   ),
#else
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
#endif
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,  24, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,  25, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  26, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  27, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig0,  28, vec_source_2_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig1,  29, vec_source_2_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_3Trig0,  30, vec_source_3_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_3Trig1,  31, vec_source_3_trig_1),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eCmp_4Trig0,  32, vec_source_4_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eCmp_4Trig1,  33, vec_source_4_trig_1),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eCmp_5Trig0,  34, vec_source_5_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eCmp_5Trig1,  35, vec_source_5_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN            ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN            ),
#if (BCHP_VER >= BCHP_VER_B0)
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eVec0Bypass0, 76, vec_hddvi_0_passthr_trig_0),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eVec0Bypass1, 77, vec_hddvi_0_passthr_trig_1),
#else
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN            ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN            ),
#endif

#elif ((BCHP_CHIP==7439) && (BCHP_VER >= BCHP_VER_B0))

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap4Trig0,    8, cap_4_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap4Trig1,    9, cap_4_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap5Trig0,   10, cap_5_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap5Trig1,   11, cap_5_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eM2mc0Trig,   72, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eM2mc1Trig,   73, m2mc_trig_1    ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig0, 44, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig1, 45, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg0,    52, mfd_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg1,    53, mfd_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg0,    54, mfd_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg1,    55, mfd_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd2Mtg0,    56, mfd_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd2Mtg1,    57, mfd_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd3Mtg0,    58, mfd_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd3Mtg1,    59, mfd_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,  24, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,  25, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  26, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  27, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig0,  28, vec_source_2_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig1,  29, vec_source_2_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_3Trig0,  30, vec_source_3_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_3Trig1,  31, vec_source_3_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7366) || ((BCHP_CHIP==7439) && (BCHP_VER == BCHP_VER_A0)) || BCHP_CHIP==74371

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eM2mc0Trig,   72, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eM2mc1Trig,   73, m2mc_trig_1    ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig0, 44, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig1, 45, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg0,    52, mfd_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg1,    53, mfd_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg0,    54, mfd_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg1,    55, mfd_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,  24, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,  25, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  26, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  27, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig0,  28, vec_source_2_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_2Trig1,  29, vec_source_2_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7586)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig0,    4, cap_2_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap2Trig1,    5, cap_2_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig0,    6, cap_3_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap3Trig1,    7, cap_3_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eM2mc0Trig,   72, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig0, 44, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig1, 45, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg0,    52, mfd_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg1,    53, mfd_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg0,    54, mfd_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg1,    55, mfd_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,  24, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,  25, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  26, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  27, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Bypass1, -1, UNKNOWN        ),

#elif (BCHP_CHIP==7271) || (BCHP_CHIP==7268)

    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig1,    1, cap_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig0,    2, cap_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap1Trig1,    3, cap_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap3Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap4Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap5Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap6Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCap7Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec0Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec1Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig2,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVec2Trig3,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtgTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDtg1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec0Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eVdec1Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6560Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(e6561Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eM2mc0Trig,   72, m2mc_trig_0    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eM2mc1Trig,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig0, 44, hd_dvi_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eHdDvi0Trig1, 45, hd_dvi_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig0, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eHdDvi1Trig1, -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eStg0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eLboxTrig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig2,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDvoTrig3,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd0Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd1Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Eof,     -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg0,    52, mfd_0_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd0Mtg1,    53, mfd_0_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg0,    54, mfd_1_trig_0   ),
    BRDC_P_MAKE_TRIG_WORD_1_INFO_NORM(eMfd1Mtg1,    55, mfd_1_trig_1   ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd2Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd3Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd4Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg0,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eMfd5Mtg1,    -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr0Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eAnr1Trig1,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eDgp0Trig0,   -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(ePx3d0Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig0,  24, vec_source_0_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_0Trig1,  25, vec_source_0_trig_1),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig0,  26, vec_source_1_trig_0),
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCmp_1Trig1,  27, vec_source_1_trig_1),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_2Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_3Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_4Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_5Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig0,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(eCmp_6Trig1,  -1, UNKNOWN        ),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eVec0Bypass0, 76, vec_hddvi_0_passthr_trig_0),
    BRDC_P_MAKE_TRIG_WORD_2_INFO_NORM(eVec0Bypass1, 77, vec_hddvi_0_passthr_trig_0),

#elif (BCHP_CHIP == 11360)
/*
 * For all platforms which do not have BRDC, appropriate BCHP_CHIP must be added
 * either in the bchp .inc file, or in bchp_common.h in the RDB.
 * This will make compiler happy.
 */
    BRDC_P_MAKE_TRIG_WORD_0_INFO_NORM(eCap0Trig0,    0, cap_0_trig_0   ),
#else
#error "Port reqired for RDC."
#endif

    /* Common to all chips then put here. */
    BRDC_P_MAKE_TRIG_INFO_COMB(eComboTrig0,         -1, comb_trig_0    ),
    BRDC_P_MAKE_TRIG_INFO_COMB(eComboTrig1,         -1, comb_trig_1    ),
    BRDC_P_MAKE_TRIG_INFO_COMB(eComboTrig2,         -1, comb_trig_2    ),
    BRDC_P_MAKE_TRIG_INFO_COMB(eComboTrig3,         -1, comb_trig_3    ),
    BRDC_P_MAKE_TRIG_WORD_X_INFO_NULL(UNKNOWN,      -1, UNKNOWN        )
};

/* Count to make we have enough entries. */
#define BRDC_P_SLOT_COUNT \
    (sizeof(s_aRdcSlotInfo) / sizeof(BRDC_P_SlotInfo))

/* Count to make we have enough entries. */
#define BRDC_P_TRIGGER_COUNT \
    (sizeof(s_aRdcTrigInfo) / sizeof(BRDC_TrigInfo))

#ifdef IKOS_EMULATION
extern int timeout_ns(uint32_t size);    /* in ikos_main.c */
#endif

/***************************************************************************
 *
 */
BERR_Code BRDC_GetDefaultSettings
    ( BCHP_Handle                      hChp,
      BRDC_Settings                   *pRdcSettings )
{
    BDBG_ENTER(BRDC_GetDefaultSettings);

    BSTD_UNUSED(hChp); /* hush warnings */
    BKNI_Memset(pRdcSettings, 0, sizeof(BRDC_Settings));

    BDBG_LEAVE(BRDC_GetDefaultSettings);
    return BERR_SUCCESS ;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_Open
    ( BRDC_Handle                     *phRdc,
      BCHP_Handle                      hChp,
      BREG_Handle                      hReg,
      BMMA_Heap_Handle                 hMem,
      const BRDC_Settings             *pRdcSettings )
{
    uint32_t i;
    BRDC_Handle    hRdc = NULL;
    BERR_Code      err = BERR_SUCCESS;
    int            eSlotId;

    BDBG_ENTER(BRDC_Open);

    /* Check input parameters */
    BDBG_ASSERT(hChp);
    BDBG_ASSERT(hReg);
    BDBG_ASSERT(hMem);

    /* Make sure the lookup table is correct size. */
    if((BRDC_P_TRIGGER_COUNT != (BRDC_Trigger_UNKNOWN+1)) ||
       (BRDC_P_SLOT_COUNT != (BRDC_SlotId_eSlotMAX+1)))
    {
        BDBG_ERR(( "Table ill-constructed s_aRdcTrigInfo!" ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Matched the enum order! */
    for(i = 0; i < BRDC_P_TRIGGER_COUNT; i++)
    {
        if(i != s_aRdcTrigInfo[i].eTrigger)
        {
            BDBG_WRN(("Bad trigger info constructed for: %s",
                s_aRdcTrigInfo[i].pchTrigName));
            break;
        }

        if(BRDC_P_TRIGGER_UNKNOWN_VAL != s_aRdcTrigInfo[i].ulTrigVal)
        {
            BDBG_MSG(("%15s = %d", s_aRdcTrigInfo[i].pchTrigName,
                s_aRdcTrigInfo[i].ulTrigVal));
        }
        else
        {
            BDBG_MSG(("%15s = %d (n/a)", s_aRdcTrigInfo[i].pchTrigName,
                s_aRdcTrigInfo[i].ulTrigVal));
        }
    }

    /* Create RDC handle */
    hRdc = (BRDC_Handle)BKNI_Malloc(sizeof(BRDC_P_Handle));
    if( !hRdc )
    {
        BDBG_ERR(( "Out of System Memory" ));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Initialize context of RDC handle */
    BKNI_Memset((void*)hRdc, 0x0, sizeof(BRDC_P_Handle));
    BDBG_OBJECT_SET(hRdc, BRDC_RDC);

    /* Copy the Settings if non NULL */
    if(pRdcSettings)
    {
        hRdc->stRdcSettings = *pRdcSettings;
    }

    /* Initialize slots */
    for( eSlotId = BRDC_SlotId_eSlot0; eSlotId < BRDC_SlotId_eSlotMAX; eSlotId++ )
    {
        hRdc->bSlotUsed[eSlotId] = false;
        hRdc->apSlot[eSlotId]    = NULL;
    }

    /* Update RDC */
    hRdc->hReg = hReg;
    hRdc->hChp = hChp;
    hRdc->hMem = hMem;
    hRdc->aTrigInfo = s_aRdcTrigInfo;

    /* RDC requires power to BVN */
#ifdef BCHP_PWR_RESOURCE_BVN
    BCHP_PWR_AcquireResource(hRdc->hChp, BCHP_PWR_RESOURCE_BVN);
#endif

#ifdef BCHP_RDC_hw_configuration_max_descriptor_number_MASK
    hRdc->ulMaxAvailableSlot =
        BCHP_RDC_hw_configuration_max_trigger_number_DEFAULT ? 64 : 32;
    /* TODO: Support 64+.
    hRdc->ulMaxAvailableSlot =
        32 * (BCHP_RDC_hw_configuration_max_trigger_number_DEFAULT + 1); */
#else
    hRdc->ulMaxAvailableSlot = 32;
#endif

    err = BRDC_P_SoftReset(hRdc);
    if (err != BERR_SUCCESS)
    {
        goto error;
    }

    /* RDMA Block-out */
    err = BRDC_P_RdcBlockOutInit(hRdc);
    if (err != BERR_SUCCESS)
    {
        goto error;
    }

#ifdef BRDC_USE_CAPTURE_BUFFER
    err = BRDC_DBG_CreateCaptureBuffer(&hRdc->captureBuffer, BRDC_P_MAX_COUNT);
    if (err != BERR_SUCCESS)
    {
        goto error;
    }
#endif

#ifdef BCHP_RDC_stc_flag_0
    BDBG_CASSERT(BRDC_P_STC_FLAG_COUNT <= BRDC_MAX_STC_FLAG_COUNT);
    for(i = 0; i < BRDC_P_STC_FLAG_COUNT; i++)
    {
        hRdc->aeStcTrigger[i] = BRDC_Trigger_UNKNOWN;
        BRDC_P_Write32(hRdc, BCHP_RDC_stc_flag_0 + i*sizeof(uint32_t), 0);
    }
#endif

    *phRdc = hRdc;

    BDBG_MSG(("slots=%d, start=%d, end=%d, avail=%d",
        BRDC_P_NUM_OF_SLOTS,
        BRDC_P_SCRATCH_REG_START,
        BRDC_P_SCRATCH_REG_END,
        BRDC_P_SCRATCH_FIRST_AVAILABLE));

    BDBG_LEAVE(BRDC_Open);
    return err;

error:
#ifdef BCHP_PWR_RESOURCE_BVN
    /* a failed open releases power */
    BCHP_PWR_ReleaseResource(hRdc->hChp, BCHP_PWR_RESOURCE_BVN);
#endif
    BDBG_OBJECT_DESTROY(hRdc, BRDC_RDC);
    BKNI_Free(hRdc);

    return err;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_Close
    ( BRDC_Handle                      hRdc )
{
    int       eSlotId;
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_Close);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

    /* Check if all slots are freed */
    for( eSlotId = BRDC_SlotId_eSlot0; eSlotId < BRDC_SlotId_eSlotMAX; eSlotId++ )
    {
        if( hRdc->bSlotUsed[eSlotId] || (hRdc->apSlot[eSlotId] != NULL) )
        {
            BDBG_ERR(( "Leaked Resource: Slot %d is not free", eSlotId ));
            err = BERR_TRACE(BERR_LEAKED_RESOURCE);
            goto done;
        }
    }

    /* Close related RDMA block-out objects */
    err = BRDC_P_RdcBlockOutDestroy(hRdc);
    if (err != BERR_SUCCESS)
    {
        goto done;
    }

#ifdef BRDC_USE_CAPTURE_BUFFER
    BRDC_DBG_DestroyCaptureBuffer(&hRdc->captureBuffer);
#endif

#ifdef BCHP_PWR_RESOURCE_BVN
    BCHP_PWR_ReleaseResource(hRdc->hChp, BCHP_PWR_RESOURCE_BVN);
#endif
    BDBG_OBJECT_DESTROY(hRdc, BRDC_RDC);
    BKNI_Free(hRdc);

done:
    BDBG_LEAVE(BRDC_Close);
    return err;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_Standby
    ( BRDC_Handle                      hRdc,
      const BRDC_StandbySettings      *pSettings )
{
    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

    /* nothing to check here */
    BSTD_UNUSED(pSettings);
#ifdef BCHP_PWR_RESOURCE_BVN
    BCHP_PWR_ReleaseResource(hRdc->hChp, BCHP_PWR_RESOURCE_BVN);
#endif
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_Resume
    ( BRDC_Handle                      hRdc )
{
    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);
#ifdef BCHP_PWR_RESOURCE_BVN
    BCHP_PWR_AcquireResource(hRdc->hChp, BCHP_PWR_RESOURCE_BVN);
#endif
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
uint32_t BRDC_P_AllocScratchReg
    ( BRDC_Handle                      hRdc,
      const char*                      pchFilename, /* source filename where block is allocated from */
      int                              iLine )      /* line number in file where allocation occurs */
{
    uint32_t ulIndex;
    uint32_t ulReg = 0;

    BDBG_ENTER(BRDC_AllocScratchReg);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

    BKNI_EnterCriticalSection();
    for(ulIndex = 0; ulIndex <= BRDC_P_SCRATCH_REG_END - BRDC_P_SCRATCH_FIRST_AVAILABLE; ulIndex++)
    {
        if(!hRdc->abScratchRegUsed[ulIndex])
        {
            hRdc->abScratchRegUsed[ulIndex] = true;
            ulReg = BRDC_P_SCRATCH_REG_ADDR(ulIndex + BRDC_P_SCRATCH_FIRST_AVAILABLE);
            break;
        }
    }
    BKNI_LeaveCriticalSection();

    BDBG_MSG(("BRDC_AllocScratchReg idx %d in %s at line %d",
        ulIndex + BRDC_P_SCRATCH_FIRST_AVAILABLE, pchFilename, iLine));
#if !(BDBG_DEBUG_BUILD)
    BSTD_UNUSED(pchFilename);
    BSTD_UNUSED(iLine);
#endif

    BDBG_LEAVE(BRDC_AllocScratchReg);
    return ulReg;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_FreeScratchReg
    ( BRDC_Handle                         hRdc,
      uint32_t                            ulReg )
{
    uint32_t ulIndex;
    BERR_Code err = BERR_SUCCESS;
    BDBG_ENTER(BRDC_FreeScratchReg);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

    if((ulReg > BRDC_P_SCRATCH_REG_ADDR(BRDC_P_SCRATCH_REG_END)) ||
       (ulReg < BRDC_P_SCRATCH_REG_ADDR(BRDC_P_SCRATCH_FIRST_AVAILABLE)))
    {
        BDBG_ERR(( "Invalid parameter" ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ulIndex = (ulReg - BRDC_P_SCRATCH_REG_ADDR(BRDC_P_SCRATCH_FIRST_AVAILABLE)) / sizeof(uint32_t);
    BKNI_EnterCriticalSection();
    if(hRdc->abScratchRegUsed[ulIndex])
    {
        hRdc->abScratchRegUsed[ulIndex] = false;
    }
    else
    {
        BDBG_ERR(( "Scratch register 0x%x is not in use!", ulReg ));
        err = BERR_INVALID_PARAMETER;
    }
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRDC_FreeScratchReg);
    return err;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_EnableSameTrigger_isr
    ( BRDC_Handle                      hRdc,
      bool                             bEnable )
{
    uint32_t          ulRegVal;
    BERR_Code         err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_EnableSameTrigger_isr);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

#if (!BRDC_P_SUPPORT_SEGMENTED_RUL)
    ulRegVal = BRDC_P_Read32(hRdc, BCHP_RDC_config);
    ulRegVal &= ~BCHP_MASK(RDC_config, same_trigger);
    ulRegVal |= BCHP_FIELD_DATA(RDC_config, same_trigger, bEnable);
    BRDC_P_Write32(hRdc, BCHP_RDC_config, ulRegVal);
#else
    BSTD_UNUSED(err);
    BSTD_UNUSED(bEnable);
    BSTD_UNUSED(ulRegVal);
#endif

    BDBG_LEAVE(BRDC_EnableSameTrigger_isr);
    return err;

}


/***************************************************************************
 * Build RUL for read/modify/write a register.
 *   - Will use the RDC temporary variables BRDC_Variable_0/1/2.
 */
void BRDC_BuildRul_RdModWr_isr
    ( uint32_t                       **ppulCurrent,
      uint32_t                         ulAndMask,
      uint32_t                         ulOrMask,
      uint32_t                         ulRegAddr )
{
    uint32_t *pulCurrent = *ppulCurrent;

    /* Set up AND/OR mask */ \
    *(pulCurrent)++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_0); \
    *(pulCurrent)++ = (ulAndMask); \
    *(pulCurrent)++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_1); \
    *(pulCurrent)++ = (ulOrMask); \
    /* read/modify/write */ \
    *(pulCurrent)++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_2); \
    *(pulCurrent)++ = BRDC_REGISTER(ulRegAddr); \
    *(pulCurrent)++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_2, BRDC_Variable_0, BRDC_Variable_2); \
    *(pulCurrent)++ = BRDC_OP_VAR_OR_VAR_TO_VAR (BRDC_Variable_2, BRDC_Variable_1, BRDC_Variable_2); \
    *(pulCurrent)++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2); \
    *(pulCurrent)++ = BRDC_REGISTER(ulRegAddr); \

    /* set the rul pointer back */
    *ppulCurrent = pulCurrent;

    return;
}


/***************************************************************************
 *
 */
BERR_Code BRDC_SetComboTriggers_isr
    ( BRDC_Handle                      hRdc,
      BRDC_Trigger                     eComboTrig,
      BRDC_ComboTrigMode               eMode,
      BRDC_Trigger                     aeTriggers[BRDC_MAX_TRIGGER_COUNT],
      uint32_t                         ulNumTriggers,
      uint32_t                       **ppulCurrent )
{
    uint32_t i;
    uint32_t ulComboStatusOffset, ulComboMaskOffset;
    uint32_t *pulCurrent; /* for RUL building */
    uint32_t aulComboMask[BRDC_P_SUPPORT_TRIGGER_MASK];

    BDBG_ENTER(BRDC_SetComboTriggers_isr);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

    /* Bound check */
    BDBG_ASSERT(aeTriggers);
    BDBG_ASSERT(ppulCurrent);
    BDBG_ASSERT(ulNumTriggers < BRDC_Trigger_eComboTrig0);
    BDBG_ASSERT(
        (eComboTrig >= BRDC_Trigger_eComboTrig0) &&
        (eComboTrig <= BRDC_Trigger_eComboTrig3));

    /* Initialized the trigger mask, and start building it */
    BKNI_Memset((void*)aulComboMask, 0x0, sizeof(aulComboMask));
    pulCurrent = *ppulCurrent;

    /* Which combo trigger mask offset */
    ulComboStatusOffset = (eComboTrig - BRDC_Trigger_eComboTrig0) * (BCHP_RDC_comb_status_1 - BCHP_RDC_comb_status_0);
    ulComboMaskOffset   = (eComboTrig - BRDC_Trigger_eComboTrig0) * (BCHP_RDC_comb_mask_1 - BCHP_RDC_comb_mask_0);

    /* (0) Build combination trigger masks. */
    for(i = 0; i < ulNumTriggers; i++)
    {
        BDBG_ASSERT(aeTriggers[i] < BRDC_Trigger_eComboTrig0);
        BDBG_ASSERT(BRDC_P_TRIGGER_UNKNOWN_VAL != hRdc->aTrigInfo[aeTriggers[i]].ulTrigVal);
        aulComboMask[(hRdc->aTrigInfo[aeTriggers[i]].ulTrigVal / 32)] |=
            1 << (hRdc->aTrigInfo[aeTriggers[i]].ulTrigVal % 32);
    }

    /* Need to do breadth-first programming.  Don't combine the two loops. */
    for(i = 0; i < BRDC_P_SUPPORT_TRIGGER_MASK; i++)
    {
        /* i and ulOffset indicates: BCHP_RDC_comb_status_[x]_hi_[y], */
        if(aulComboMask[i])
        {
            /* 1. clear the trigger */
            *pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pulCurrent++ = BRDC_REGISTER(BCHP_RDC_comb_status_0 + (sizeof(uint32_t) * i) + ulComboStatusOffset);
            *pulCurrent++ = BCHP_FIELD_DATA(RDC_comb_status_0, triggers, aulComboMask[i]);
        }
    }

    for(i = 0; i < BRDC_P_SUPPORT_TRIGGER_MASK; i++)
    {
        /* i and ulOffset indicates: BCHP_RDC_comb_mask_[x]_hi_[y], */
        if(aulComboMask[i])
        {
            /* 2. set the trigger comb mask*/
            *pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pulCurrent++ = BRDC_REGISTER(BCHP_RDC_comb_mask_0 + (sizeof(uint32_t) * i) + ulComboMaskOffset);
            *pulCurrent++ = BCHP_FIELD_DATA(RDC_comb_mask_0, mask, aulComboMask[i]);
        }
    }

    /* 3. trigger mode.  Need to select which combo trigger and its mode. */
    {
        uint32_t ulModeShift, ulAndMask, ulOrMask;
        ulModeShift = BCHP_SHIFT(RDC_config, trig_combine_mode) + (eComboTrig - BRDC_Trigger_eComboTrig0);
        ulAndMask   = 1 << ulModeShift;
        ulOrMask    = eMode << ulModeShift;
        BRDC_BuildRul_RdModWr_isr(&pulCurrent, ~ulAndMask, ulOrMask, BCHP_RDC_config);
    }

    /* set the rul pointer back */
    *ppulCurrent = pulCurrent;

    BDBG_LEAVE(BRDC_SetComboTriggers_isr);
    return BERR_SUCCESS;

}


/***************************************************************************
 *
 */
BERR_Code BRDC_SetRdcBlockOut
    ( BRDC_Handle                      hRdc,
      const BRDC_BlockOut             *pstBlockOut,
      uint32_t                         ulRegBlock )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_SetRdcBlockOut);

    BDBG_ASSERT(pstBlockOut);
    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

    /* Check if registers are valid */
    err = BRDC_P_ValidateBlockOutRegisters(pstBlockOut, ulRegBlock);
    if (err != BERR_SUCCESS)
    {
        return err;
    }
    else
    {
        BKNI_EnterCriticalSection();

        /* Disable current blockout */
        hRdc->bRdcBlockOutEnabled = false;

        /* Store blockout info */
        BKNI_Memcpy((void *)&hRdc->astBlockOut[ulRegBlock], (void *)pstBlockOut, sizeof(BRDC_BlockOut));

        hRdc->bRdcBlockOutEnabled = true;

#ifdef BCHP_RDC_br_0_start_addr
        {
            uint32_t ulOffset = (BCHP_RDC_br_1_start_addr - BCHP_RDC_br_0_start_addr) * ulRegBlock;
            BRDC_P_Write32(hRdc, BCHP_RDC_br_0_start_addr + ulOffset, BRDC_REGISTER(pstBlockOut->ulStartRegAddr));
            BRDC_P_Write32(hRdc, BCHP_RDC_br_0_end_addr + ulOffset, BRDC_REGISTER(pstBlockOut->ulStartRegAddr + (pstBlockOut->ulBlockSize - 1) * sizeof(uint32_t)));
            BRDC_P_Write32(hRdc, BCHP_RDC_br_0_enable + ulOffset, pstBlockOut->bEnable);
        }
#endif

        BKNI_LeaveCriticalSection();
    }

    BDBG_LEAVE(BRDC_SetRdcBlockOut);
    return err;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_GetRdcBlockOut
    ( BRDC_Handle                      hRdc,
      BRDC_BlockOut                   *pstBlockOut,
      uint32_t                         ulRegBlock )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_GetRdcBlockOut);

    BDBG_ASSERT(pstBlockOut);
    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

    BKNI_Memcpy((void *)pstBlockOut, (void *)&hRdc->astBlockOut[ulRegBlock], sizeof(BRDC_BlockOut));

    BDBG_LEAVE(BRDC_GetRdcBlockOut);
    return err;
}

/***************************************************************************
 *
 */
uint32_t BRDC_AcquireStcFlag_isr
    ( BRDC_Handle                      hRdc,
      uint32_t                         ulPreferredId,
      BRDC_Trigger                     eTrig )
{
#ifdef BCHP_RDC_stc_flag_0
    int i=0;
    uint32_t ulReg;
    uint32_t ulAddr;
    BDBG_ENTER(BRDC_AcquireStcFlag_isr);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

    /* Check if trigger is valid */
    if ((eTrig < BRDC_Trigger_eMfd0Mtg0 || eTrig > BRDC_Trigger_eMfd5Mtg1) &&
        (eTrig < BRDC_Trigger_eCmp_0Trig0 || eTrig > BRDC_Trigger_eCmp_6Trig1))
    {
        BDBG_ERR(("Invalid trigger %d for STC flag! [%u, %u][%u, %u]", eTrig, BRDC_Trigger_eMfd0Mtg0, BRDC_Trigger_eMfd5Mtg1,
            BRDC_Trigger_eCmp_0Trig0, BRDC_Trigger_eCmp_6Trig1));
        goto acquire_error;
    }

    if(ulPreferredId < BRDC_MAX_STC_FLAG_COUNT)
    {
        i = ulPreferredId;
        if(hRdc->aeStcTrigger[ulPreferredId] != BRDC_Trigger_UNKNOWN)
        {
            goto used_error;
        }
    } else
    {
        /* from high to low */
        for(i = BRDC_P_STC_FLAG_COUNT-1; i >= 0; i--)
        {
            if(hRdc->aeStcTrigger[i] == BRDC_Trigger_UNKNOWN)
            {
                break;
            }
        }
    }
    if(i >= BRDC_P_STC_FLAG_COUNT || i < 0) goto used_error;

    ulAddr = BCHP_RDC_stc_flag_0 + sizeof(uint32_t) * i;
    hRdc->aeStcTrigger[i] = eTrig;
    ulReg = BRDC_P_Read32(hRdc, ulAddr);
    ulReg &= ~BCHP_MASK(RDC_stc_flag_0, trig_src_sel);
    ulReg |= (
        BCHP_FIELD_DATA(RDC_stc_flag_0, trig_src_sel, BCHP_RDC_stc_flag_0_trig_src_sel_VEC_0
            + ((eTrig - BRDC_Trigger_eCmp_0Trig0)>>1)) |
        BCHP_FIELD_DATA(RDC_stc_flag_0, trig_en, 1));
    BRDC_P_Write32(hRdc, ulAddr, ulReg);
    BDBG_MSG(("STC flag %u, trig_sel=%#x, eTrig=%u", i, ulReg, eTrig));

    BDBG_LEAVE(BRDC_AcquireStcFlag_isr);
    return (uint32_t)i;
used_error:
    BDBG_ERR(("RDC trigger %d for STC flag %u was already acquired!", eTrig, i));
acquire_error:
#else
    BSTD_UNUSED(hRdc);
    BSTD_UNUSED(ulPreferredId);
    BSTD_UNUSED(eTrig);
#endif
    return BRDC_MAX_STC_FLAG_COUNT;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_ReleaseStcFlag_isr
    ( BRDC_Handle                      hRdc,
      uint32_t                         ulId )
{
#ifdef BCHP_RDC_stc_flag_0
    BDBG_ENTER(BRDC_ReleaseStcFlag_isr);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);
    if(BRDC_P_STC_FLAG_COUNT <= ulId) return BERR_TRACE(BERR_INVALID_PARAMETER);
    hRdc->aeStcTrigger[ulId] = BRDC_Trigger_UNKNOWN;
    BRDC_P_Write32(hRdc, BCHP_RDC_stc_flag_0 + ulId * sizeof(uint32_t), 0);/* disable */
    BDBG_MSG(("stc flag %u is released ...", ulId));
    BDBG_LEAVE(BRDC_ReleaseStcFlag_isr);
#else
    BSTD_UNUSED(hRdc);
    BSTD_UNUSED(ulId);
#endif
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_EnableStcFlag_isr
    ( BRDC_Handle                      hRdc,
      uint32_t                         ulId,
      bool                             bEnable )
{
#ifdef BCHP_RDC_stc_flag_0
    uint32_t ulReg;
    uint32_t ulAddr = BCHP_RDC_stc_flag_0 + sizeof(uint32_t) * ulId;
    BDBG_ENTER(BRDC_EnableStcFlag_isr);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);
    if(BRDC_P_STC_FLAG_COUNT <= ulId) return BERR_TRACE(BERR_INVALID_PARAMETER);
    ulReg = BRDC_P_Read32(hRdc, ulAddr);
    ulReg &= ~BCHP_MASK(RDC_stc_flag_0, trig_en);
    ulReg |= BCHP_FIELD_DATA(RDC_stc_flag_0, trig_en, bEnable);
    BRDC_P_Write32(hRdc, ulAddr, ulReg);
    BDBG_LEAVE(BRDC_EnableStcFlag_isr);
#else
    BSTD_UNUSED(hRdc);
    BSTD_UNUSED(ulId);
    BSTD_UNUSED(bEnable);
#endif
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_Trigger_Execute_isr
    ( BRDC_Handle                      hRdc,
      BRDC_Trigger                     eRDCTrigger )
{
    uint32_t          ulRegVal;
    BERR_Code         err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_Trigger_Execute_isr);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

    if(eRDCTrigger == BRDC_Trigger_UNKNOWN)
    {
        BDBG_ERR(( "Invalid parameter" ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ulRegVal = BRDC_P_Read32(hRdc, BCHP_RDC_force_trigger);
    ulRegVal &= ~BCHP_MASK(RDC_force_trigger, trigger_index);
    ulRegVal |= BCHP_FIELD_DATA(RDC_force_trigger, trigger_index, hRdc->aTrigInfo[eRDCTrigger].ulTrigVal);
    BRDC_P_Write32(hRdc, BCHP_RDC_force_trigger, ulRegVal);

    BDBG_LEAVE(BRDC_Trigger_Execute_isr);
    return err;

}


/***************************************************************************
 *
 */
const BRDC_TrigInfo* BRDC_Trigger_GetInfo
    ( BRDC_Handle                      hRdc,
      BRDC_Trigger                     eRDCTrigger )
{
    BDBG_ENTER(BRDC_Trigger_GetInfo);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

    if(BRDC_Trigger_UNKNOWN == eRDCTrigger)
    {
        BDBG_ERR(( "Invalid parameter."));
        return NULL;
    }

    /* Trigger not supported on this chip. */
    if(BRDC_P_NULL_BINTID == hRdc->aTrigInfo[eRDCTrigger].TrigIntId)
    {
        BDBG_ERR(("Invalid trigger!  (%s) not supported on this chip",
            hRdc->aTrigInfo[eRDCTrigger].pchTrigName));
        return NULL;
    }

    BDBG_LEAVE(BRDC_Trigger_GetInfo);
    return &(hRdc->aTrigInfo[eRDCTrigger]);

}


/***************************************************************************
 *
 */
BERR_Code BRDC_List_Create
    ( BRDC_Handle                      hRdc,
      uint32_t                         ulMaxEntries,
      BRDC_List_Handle                *phList )
{
    BERR_Code         err = BERR_SUCCESS;
    BRDC_List_Handle  hList = NULL;

    BDBG_ENTER(BRDC_List_Create);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);
    BDBG_ASSERT(phList);

    /* (1) Create list handle */
    hList = (BRDC_List_Handle)BKNI_Malloc(sizeof(BRDC_P_List_Handle));
    if( !hList )
    {
        BDBG_ERR(( "Out of System Memory" ));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Initialize context of list */
    BKNI_Memset((void*)hList, 0x0, sizeof(BRDC_P_List_Handle));
    BDBG_OBJECT_SET(hList, BRDC_LST);

    /* (2) Create memory for entries in list. Must be 256 bit aligned */
    hList->hRULBlock = BMMA_Alloc(hRdc->hMem,
        sizeof(uint32_t)* ulMaxEntries, 1<<5, NULL);
    if( !hList->hRULBlock )
    {
        BDBG_ERR(( "Out of Device Memory" ));
        BDBG_OBJECT_DESTROY(hList, BRDC_LST);
        BKNI_Free(hList);
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }

    hList->pulRULAddr = (uint32_t *)BMMA_Lock(hList->hRULBlock);

    /* Initialized list */
    hList->hRdc              = hRdc;
    hList->ulEntries         = 0;
    hList->ulMaxEntries      = ulMaxEntries;
    hList->ulNumSlotAssigned = 0;
    hList->bLastExecuted     = false;

    hList->ulAddrOffset = (uint32_t)BMMA_LockOffset(hList->hRULBlock);

    hList->eNextEntry        = BRDC_DBG_ListEntry_eCommand;
    hList->pulCurListAddr    = NULL;
    hList->ulNumEntries      = 0;
    hList->ulCurrCommand     = 0;
    hList->iCommandIndex     = 0;
    hList->iDataCount        = 0;

    /* (3) Assigned slot linked-list */
    hList->pSlotAssigned = (BRDC_P_Slot_Head *) BKNI_Malloc(sizeof(BRDC_P_Slot_Head));
    if( hList->pSlotAssigned == NULL )
    {
        BMMA_Unlock(hList->hRULBlock, hList->pulRULAddr);
        BMMA_UnlockOffset(hList->hRULBlock, hList->ulAddrOffset);
        BMMA_Free(hList->hRULBlock);

        BKNI_Free(hList);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BLST_D_INIT(hList->pSlotAssigned);

    /* Return to user */
    *phList = hList;

    BDBG_LEAVE(BRDC_List_Create);
    return err ;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_List_Destroy
    ( BRDC_List_Handle                 hList )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_List_Destroy);

    BDBG_OBJECT_ASSERT(hList, BRDC_LST);

    if( (hList->ulNumSlotAssigned) ||
        ((hList->pSlotAssigned) && (BLST_D_FIRST(hList->pSlotAssigned) != NULL)) )
    {
        BDBG_ERR(( "List is assgined to a slot" ));
        return BERR_TRACE(BRDC_LIST_ERR_ASSIGNED_TO_SLOT);
    }

#ifndef IKOS_EMULATION
    if( hList->hRULBlock )
    {
        BMMA_Unlock(hList->hRULBlock, hList->pulRULAddr);
        BMMA_UnlockOffset(hList->hRULBlock, hList->ulAddrOffset);
        BMMA_Free(hList->hRULBlock);
        hList->hRULBlock = NULL;
    }
#endif

    if( hList->pSlotAssigned )
    {
        BKNI_Free(hList->pSlotAssigned);
    }

    BKNI_Free(hList);

    BDBG_LEAVE(BRDC_List_Destroy);
    return err;
}


/***************************************************************************
 *
 */
uint32_t *BRDC_List_GetStartAddress_isr
    ( BRDC_List_Handle                 hList )
{
    BDBG_ENTER(BRDC_List_GetStartAddress_isr);

    BDBG_OBJECT_ASSERT(hList, BRDC_LST);

    BDBG_LEAVE(BRDC_List_GetStartAddress_isr);

    return (hList->pulRULAddr);
}


/***************************************************************************
 *
 */
BERR_Code BRDC_List_SetNumEntries_isr
    ( BRDC_List_Handle                 hList,
      uint32_t                         ulNumEntries )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_List_SetNumEntries_isr);

    BDBG_OBJECT_ASSERT(hList, BRDC_LST);

    if( ulNumEntries > hList->ulMaxEntries )
    {
        BDBG_ERR(( "More than Max Entries %d > %d ",
            ulNumEntries, hList->ulMaxEntries ));
        BDBG_ERR(("Memory overrun already done, system could be unstable"));
        return BERR_TRACE(BRDC_LIST_ERR_ENTRIES_MORE_THAN_MAX);
    }

    hList->ulEntries = ulNumEntries;

    BDBG_LEAVE(BRDC_List_SetNumEntries_isr);
    return err;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_List_GetNumEntries_isr
    ( BRDC_List_Handle                 hList,
      uint32_t                        *pulNumEntries )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_List_GetNumEntries_isr);

    BDBG_OBJECT_ASSERT(hList, BRDC_LST);

    if( pulNumEntries )
    {
        *pulNumEntries = hList->ulEntries;
    }

    BDBG_LEAVE(BRDC_List_GetNumEntries_isr);
    return err;
}


/***************************************************************************
 *
 */
BERR_Code BRDC_List_GetMaxEntries_isr
    ( BRDC_List_Handle                 hList,
      uint32_t                        *pulMaxEntries )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_List_GetMaxEntries_isr);

    BDBG_OBJECT_ASSERT(hList, BRDC_LST);

    if( pulMaxEntries )
    {
        *pulMaxEntries = hList->ulMaxEntries;
    }

    BDBG_LEAVE(BRDC_List_GetMaxEntries_isr);
    return err;
}


/***************************************************************************
 *
 */
bool BRDC_List_GetLastExecStatus_isr
    ( BRDC_List_Handle                 hList )
{
    BDBG_ENTER(BRDC_List_GetLastExecStatus_isr);
    BDBG_OBJECT_ASSERT(hList, BRDC_LST);
    BDBG_LEAVE(BRDC_List_GetLastExecStatus_isr);
    return hList->bLastExecuted;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_Slot_Create
    ( BRDC_Handle                      hRdc,
      BRDC_Slot_Handle                *phSlot )
{
    BERR_Code         err   = BERR_SUCCESS;
    BRDC_SlotId       eSlotId;
    BRDC_Slot_Handle  hSlot = NULL;

    BDBG_ENTER(BRDC_Slot_Create);

    BDBG_OBJECT_ASSERT(hRdc, BRDC_RDC);

    /* Create slot */
    hSlot = (BRDC_Slot_Handle)BKNI_Malloc(sizeof(BRDC_P_Slot_Handle));
    if( !hSlot )
    {
        BDBG_ERR(( "Out of System Memory" ));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Initialize context of slot handle */
    BKNI_Memset((void*)hSlot, 0x0, sizeof(BRDC_P_Slot_Handle));
    BDBG_OBJECT_SET(hSlot, BRDC_SLT);

    /* Use critical section when find next available slot */
    BKNI_EnterCriticalSection();
    err = BERR_TRACE(BRDC_Slot_P_GetNextSlot(hRdc, &eSlotId));
    BKNI_LeaveCriticalSection();
    if( err != BERR_SUCCESS )
    {
        BKNI_Free(hSlot);
        goto done;
    }

    if(s_aRdcSlotInfo[eSlotId].SlotIntId == BRDC_P_NULL_BINTID)
    {
        BKNI_Free(hSlot);
        err = BERR_TRACE(BRDC_SLOT_ERR_ALL_USED);
        goto done;
    }

    /* Assign the slot to the calling module */
    hSlot->eSlotId     = eSlotId;
    hSlot->hRdc        = hRdc;
    hSlot->bRecurring  = false;
    hSlot->eRDCTrigger = BRDC_Trigger_UNKNOWN;
    hSlot->hList       = NULL;
    hSlot->ulRegOffset = ((BCHP_RDC_desc_1_addr - BCHP_RDC_desc_0_addr) *
        (eSlotId - BRDC_SlotId_eSlot0));
    hSlot->SlotIntId   = s_aRdcSlotInfo[eSlotId].SlotIntId;

    /* Keeping track if last RUL executed. */
    if(BRDC_P_NO_TRACKING_ADDR != s_aRdcSlotInfo[eSlotId].ulTrackRegAddr)
    {
#ifdef BRDC_DISABLE_TRACK_EXECUTION
        hSlot->bTrackExecution  = false ;
#else
        hSlot->bTrackExecution  = true ;
#endif
        hSlot->ulTrackCount     = 0;
        hSlot->ulTrackRegAddr   = s_aRdcSlotInfo[eSlotId].ulTrackRegAddr;
        BDBG_MSG(("Creating slot[%d] with track executions!", hSlot->eSlotId));
    }
    else
    {
        BDBG_WRN(("Creating slot[%d] without track executions!", hSlot->eSlotId));
    }

    /* Update RDC */
    hRdc->bSlotUsed[eSlotId] = true;
    hRdc->apSlot[eSlotId] = hSlot;

    /* All done */
    *phSlot = hSlot;

done:
    BDBG_LEAVE(BRDC_Slot_Create);
    return err ;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_Slot_Destroy
    ( BRDC_Slot_Handle                 hSlot )
{
    BERR_Code         err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_Slot_Destroy);

    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);

    /* Disable the slot */
    BKNI_EnterCriticalSection();
    err = BERR_TRACE(BRDC_Slot_Disable_isr(hSlot));
    BKNI_LeaveCriticalSection();
    if( err != BERR_SUCCESS )
        goto done;

    /* Update RDC: This slot becomes available */
    hSlot->hRdc->bSlotUsed[hSlot->eSlotId] = false;
    hSlot->hRdc->apSlot[hSlot->eSlotId]    = NULL;

    /* Clear list from the slot */
    if( hSlot->hList )
    {
        hSlot->hList->ulNumSlotAssigned--;

        /* Remove slot */
        BLST_D_REMOVE(hSlot->hList->pSlotAssigned, hSlot, link);
    }

    /* Free slot */
    BKNI_Free(hSlot);

done:
    BDBG_LEAVE(( "BRDC_Slot_Destroy" ));
    return err;
}


/***************************************************************************
 *
 */
BERR_Code BRDC_Slot_GetId
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_SlotId                     *pSlotId )
{
    BDBG_ENTER(BRDC_Slot_GetId);

    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);

    *pSlotId = hSlot->eSlotId;

    BDBG_LEAVE(BRDC_Slot_GetId);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BRDC_Slot_GetConfiguration_isr
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_Slot_Settings              *pSettings )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_Slot_GetConfiguration_isr);

    BDBG_ASSERT(pSettings);
    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);

    BKNI_Memcpy((void *)pSettings, (void *)&hSlot->stSlotSetting, sizeof(BRDC_Slot_Settings));

    BDBG_LEAVE(BRDC_Slot_GetConfiguration_isr);
    return err;
}


/***************************************************************************
 *
 */
BERR_Code BRDC_Slot_SetConfiguration_isr
    ( BRDC_Slot_Handle                 hSlot,
      const BRDC_Slot_Settings        *pSettings )
{
#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
    uint32_t   ulRegVal;
#endif
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_Slot_SetConfiguration_isr);

    BDBG_ASSERT(pSettings);
    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);

    hSlot->stSlotSetting.bHighPriority = pSettings->bHighPriority;

#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
    /* Acquire semaphore */
    err = BERR_TRACE(BRDC_P_AcquireSemaphore_isr(hSlot->hRdc, hSlot->hList, hSlot->eSlotId));
    if (err != BERR_SUCCESS)
    {
        /* Cannot acquire semaphore */
        BDBG_ERR(( "Cannot acquire semaphore for slot %d", hSlot->eSlotId ));
        goto done;
    }

    /* Get semaphore. Fill in hardware registers */
    /* Read RDC_desc_x_config */
    ulRegVal = BRDC_Slot_P_Read32(hSlot, BCHP_RDC_desc_0_config + hSlot->ulRegOffset);

    /* Clear high_priority */
    ulRegVal &= ~BCHP_MASK(RDC_desc_0_config, high_priority);

    /* Set high_priority bit and update register */
    ulRegVal |= BCHP_FIELD_DATA(RDC_desc_0_config, high_priority, hSlot->stSlotSetting.bHighPriority);
    BRDC_Slot_P_Write32(hSlot, BCHP_RDC_desc_0_config + hSlot->ulRegOffset, ulRegVal);

    /* Release semaphore */
    BRDC_P_ReleaseSemaphore_isr(hSlot->hRdc, hSlot->hList, hSlot->eSlotId);

done:
#endif

    BDBG_LEAVE(BRDC_Slot_SetConfiguration_isr);
    return err;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_Slot_GetList_isr
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_List_Handle                *phList )
{
    /* return list handle */
    *phList = hSlot->hList;

    /* success */
    return BERR_SUCCESS;
}

/***************************************************************************
 * This function calls the following _isr functions:
 *      BRDC_P_AcquireSemaphore_isr
 *      BRDC_P_ReleaseSemaphore_isr
 * This function set a list to ulNum of slots started with *phSlots
 */
BERR_Code BRDC_P_Slots_SetList_isr
    ( BRDC_Slot_Handle                *phSlots,
      BRDC_List_Handle                 hList,
      size_t                           ulNum)
{
    uint32_t ulRegVal;
    size_t i;
    BERR_Code  err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_P_Slots_SetList_isr);

    BDBG_ASSERT(phSlots);
    BDBG_OBJECT_ASSERT(hList, BRDC_LST);

    /* Can't set empty list. */
    if( hList->ulEntries < 1 )
    {
        BDBG_ERR(( "Empty List" ));
        return BERR_TRACE(BRDC_SLOT_ERR_EMPTY_LIST);
    }

#if(!BRDC_P_SUPPORT_HW_BLOCKOUT)
    if (BRDC_P_IsRdcBlockOutEnabled_isr(hList->hRdc))
    {
        err = BRDC_P_ParseAndReplaceRul_isr(hList);
        if (err != BERR_SUCCESS)
            goto done;
    }
#endif

#ifdef BRDC_USE_CAPTURE_BUFFER
    /* Write to log before RUL could possibly be executed;
     * log one slot only to reduce log size; */
    BRDC_P_DBG_WriteCaptures_isr(&hList->hRdc->captureBuffer, phSlots, hList, ulNum);
#endif

    /* Call the intercept if non NULL */
    for(i = 0; i < ulNum; i++)
    {
        BDBG_ASSERT(hList->hRdc == phSlots[i]->hRdc);
        if(phSlots[i]->hRdc->stRdcSettings.pfnSlot_SetList_Intercept_isr)
        {
            if( (err = BERR_TRACE(phSlots[i]->hRdc->stRdcSettings.pfnSlot_SetList_Intercept_isr(
                phSlots[i], hList, phSlots[i]->eRDCTrigger) )) )
            {
                goto done;
            }
        }

        /********************** slot i start *****************/
        /* Acquire dual semaphores */
        err = BERR_TRACE(BRDC_P_AcquireSemaphore_isr(phSlots[i]->hRdc, phSlots[i]->hList, phSlots[i]->eSlotId));
        if (err != BERR_SUCCESS)
        {
            /* Cannot acquire semaphore */
            BDBG_ERR(( "Cannot acquire semaphore for slot %d", phSlots[i]->eSlotId ));
            goto done;
        }

        /* Get semaphore. Fill in hardware registers */
        /* Set RDC_desc_x_addr */
        BRDC_Slot_P_Write32(phSlots[i], BCHP_RDC_desc_0_addr + phSlots[i]->ulRegOffset,
            hList->ulAddrOffset);

        /* Read RDC_desc_x_config */
        ulRegVal = BRDC_Slot_P_Read32(phSlots[i], BCHP_RDC_desc_0_config + phSlots[i]->ulRegOffset);

        /* Clear count */
        ulRegVal &= ~BCHP_MASK(RDC_desc_0_config, count);

        /* Set count and update register */
        ulRegVal |= BCHP_FIELD_DATA(RDC_desc_0_config, count, hList->ulEntries -1);
        BRDC_Slot_P_Write32(phSlots[i], BCHP_RDC_desc_0_config + phSlots[i]->ulRegOffset, ulRegVal);

        /* Release semaphore */
        BRDC_P_ReleaseSemaphore_isr(phSlots[i]->hRdc, phSlots[i]->hList, phSlots[i]->eSlotId);
        /********************** slot i end *****************/

        /* Keep track of software copy */
        /* Already have list assigned to slot */
        if( phSlots[i]->hList )
        {
            /* Clear the old list from slot */
            phSlots[i]->hList->ulNumSlotAssigned--;

            /* Remove slot */
            BLST_D_REMOVE(phSlots[i]->hList->pSlotAssigned, phSlots[i], link);
        }

        /* Assign list to slot */
        hList->ulNumSlotAssigned ++;

        /* Add slot */
        BLST_D_INSERT_HEAD(hList->pSlotAssigned, phSlots[i], link);
        phSlots[i]->hList = hList;
    }

done:
    BDBG_LEAVE(BRDC_P_Slots_SetList_isr);
    return err;
}

/***************************************************************************
 * This function calls the following _isr implementation functions:
 *      BRDC_P_Slots_SetList_isr
 */
BERR_Code BRDC_Slots_SetList_isr
    ( BRDC_Slot_Handle                *phSlot,
      BRDC_List_Handle                 hList,
      size_t                           ulNum)
{
    size_t i;
    uint32_t *pulStart   = hList->pulRULAddr;
    uint32_t *pulCurrent = pulStart + hList->ulEntries;
    BERR_Code  err = BERR_SUCCESS;
    BDBG_ENTER(BRDC_Slots_SetList_isr);
    BDBG_ASSERT(ulNum > 0 && ulNum <= BRDC_SlotId_eSlotMAX);
    BDBG_OBJECT_ASSERT(hList, BRDC_LST);

    /* Update the number of time this list, assigned to a slot. */
    for(i = 0; i < ulNum; i++)
    {
        BDBG_OBJECT_ASSERT(phSlot[i], BRDC_SLT);
        if(phSlot[i]->bTrackExecution)
        {
            *pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pulCurrent++ = BRDC_REGISTER(phSlot[i]->ulTrackRegAddr);
            *pulCurrent++ = ++(phSlot[i]->ulTrackCount);
            hList->ulEntries = (uint32_t)(pulCurrent - pulStart);
        }
    }

    /* Flush the list before setting it to dual slots. */
    BMMA_FlushCache_isr(hList->hRULBlock, hList->pulRULAddr, hList->ulEntries * sizeof(uint32_t));

    err = BRDC_P_Slots_SetList_isr(phSlot, hList, ulNum);

    BDBG_LEAVE(BRDC_Slots_SetList_isr);
    return err;
}

/***************************************************************************
 * This function calls the following _isr functions:
 *      BRDC_P_AcquireSemaphore_isr
 *      BRDC_P_ReleaseSemaphore_isr
 *      BRDC_Slot_P_Write_Registers_isr
 */
BERR_Code BRDC_Slot_ExecuteOnTrigger_isr
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_Trigger                     eRDCTrigger,
      bool                             bRecurring )
{
    BERR_Code         err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_Slot_ExecuteOnTrigger_isr);

    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);

    if( hSlot->hList->ulEntries < 1 )
    {
        BDBG_ERR(( "Empty List." ));
        return BERR_TRACE(BRDC_SLOT_ERR_EMPTY_LIST);
    }

    hSlot->eRDCTrigger = eRDCTrigger;

    /* Call the intercept if non NULL */
    if(hSlot->hRdc->stRdcSettings.pfnSlot_ExecuteOnTrigger_Intercept_isr)
    {
        if( (err = BERR_TRACE(hSlot->hRdc->stRdcSettings.pfnSlot_ExecuteOnTrigger_Intercept_isr(
            hSlot, eRDCTrigger, bRecurring))) )
        {
            goto done ;
        }
    }

    /* Acquire semaphore */
    err = BERR_TRACE(BRDC_P_AcquireSemaphore_isr(hSlot->hRdc, hSlot->hList, hSlot->eSlotId));
    if (err != BERR_SUCCESS)
    {
        /* Cannot acquire semaphore */
        goto done;
    }

    /* Fill in hardware registers */
    err = BERR_TRACE(BRDC_Slot_P_Write_Registers_isr(hSlot, eRDCTrigger,
        bRecurring, true));

    /* Release semaphore */
    BRDC_P_ReleaseSemaphore_isr(hSlot->hRdc, hSlot->hList, hSlot->eSlotId);

done:
    BDBG_LEAVE(BRDC_Slot_ExecuteOnTrigger_isr);
    return err;
}

/***************************************************************************
 * This function calls the following _isr functions:
 *      BRDC_Slot_P_Write_Registers_isr
 *      BRDC_P_AcquireSemaphore_isr
 *      BRDC_P_ReleaseSemaphore_isr
 */
BERR_Code BRDC_Slot_Execute_isr
    ( BRDC_Slot_Handle                 hSlot )
{
    BERR_Code         err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_Slot_Execute_isr);

    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);

    if( hSlot->hList->ulEntries < 1 )
    {
        BDBG_ERR(( "Empty List." ));
        return BERR_TRACE(BRDC_SLOT_ERR_EMPTY_LIST);
    }

    /* Call the intercept if non NULL */
    if(hSlot->hRdc->stRdcSettings.pfnSlot_Execute_Intercept_isr)
    {
        if( (err = BERR_TRACE(hSlot->hRdc->stRdcSettings.pfnSlot_Execute_Intercept_isr(
            hSlot, hSlot->eRDCTrigger) )) )
        {
            goto done ;
        }
    }

    /* Acquire semaphore */
    err = BERR_TRACE(BRDC_P_AcquireSemaphore_isr(hSlot->hRdc, hSlot->hList, hSlot->eSlotId));
    if (err != BERR_SUCCESS)
    {
        /* Cannot acquire semaphore */
        goto done;
    }

    /* Fill in hardware registers */
    /* Don't need trigger, non-repeat */
    err = BERR_TRACE(BRDC_Slot_P_Write_Registers_isr(hSlot,
        BRDC_Trigger_UNKNOWN, false, false));

    /* Release semaphore */
    BRDC_P_ReleaseSemaphore_isr(hSlot->hRdc, hSlot->hList, hSlot->eSlotId);

done:
    BDBG_LEAVE(BRDC_Slot_Execute_isr);
    return err;

}

/***************************************************************************
 * This function calls the following _isr functions:
 *      BRDC_P_AcquireSemaphore_isr
 *      BRDC_P_ReleaseSemaphore_isr
 */
BERR_Code BRDC_Slot_Disable_isr
    ( BRDC_Slot_Handle                 hSlot )
{
    bool              bSlotRecurring;
    uint32_t          ulRegVal;
    BERR_Code         err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_Slot_Disable_isr);

    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);

    /* Acquire semaphore */
    err = BERR_TRACE(BRDC_P_AcquireSemaphore_isr(hSlot->hRdc, hSlot->hList, hSlot->eSlotId));
    if (err != BERR_SUCCESS)
    {
        /* Cannot acquire semaphore */
        goto done;
    }

    /* Fill in hardware registers */
    ulRegVal = BRDC_Slot_P_Read32(hSlot, BCHP_RDC_desc_0_config + hSlot->ulRegOffset);

    /* All RDC_desc_x_config bit definitions are same */
    bSlotRecurring = (BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, repeat) != 0);
    if( bSlotRecurring )
    {
        /* All BCHP_RDC_desc_x_config bit definitions are same */
        ulRegVal &= ~BCHP_MASK(RDC_desc_0_config, enable);
        BRDC_Slot_P_Write32(hSlot, BCHP_RDC_desc_0_config + hSlot->ulRegOffset, ulRegVal);
    }

    /* Release semaphore */
    BRDC_P_ReleaseSemaphore_isr(hSlot->hRdc, hSlot->hList, hSlot->eSlotId);

done:
    BDBG_LEAVE(BRDC_Slot_Disable_isr);
    return err;

}


/***************************************************************************
 *
 */
BINT_Id BRDC_Slot_GetIntId
    ( const BRDC_Slot_Handle           hSlot )
{
    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);
    return hSlot->SlotIntId;
}


/***************************************************************************
 *
 */
bool BRDC_Slot_UpdateLastRulStatus_isr
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_List_Handle                 hList,
      bool                             bEnableTracking )
{
    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);
    BDBG_OBJECT_ASSERT(hList, BRDC_LST);

    if((hSlot->bTrackExecution != bEnableTracking) &&
       (BRDC_P_NO_TRACKING_ADDR != hSlot->ulTrackRegAddr))
    {
#ifdef BRDC_DISABLE_TRACK_EXECUTION
        hSlot->bTrackExecution = false;
#else
        hSlot->bTrackExecution = bEnableTracking;
#endif
    }

    if(hSlot->bTrackExecution)
    {
        if (hSlot->ulTrackCount <= 1)
        {
            /* This is the first RUL and HW count tracking register
             * is not programmed yet.
             */
            hList->bLastExecuted = true;
        }
        else
        {
            uint32_t ulHwTrackCount =
                BREG_Read32(hSlot->hRdc->hReg, hSlot->ulTrackRegAddr);
            hList->bLastExecuted = (hSlot->ulTrackCount == ulHwTrackCount)
                ? true : false;
        }
    }
    else
    {
        /* Default last executed if not tracking, true, there are code that
         * won't move state unless it's certain that the slot last executed. */
        hList->bLastExecuted = true;
    }

    return hList->bLastExecuted;
}

/***************************************************************************
 * This function program RUL to config the length of the RUL;
 */
BERR_Code BRDC_Slot_RulConfigRulSize_isr
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_List_Handle                 hList,
      uint32_t                         ulCount )
{
    uint32_t *pulStart;
    uint32_t *pulCurrent;
    uint32_t  ulRegVal;

    BDBG_ENTER(BRDC_Slot_RulConfigRulSize_isr);

    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);
    BDBG_OBJECT_ASSERT(hList, BRDC_LST);
    BDBG_ASSERT(hList->hRdc == hSlot->hRdc);

    /* Can't set empty list. */
    if( hList->ulEntries < 1 )
    {
        BDBG_ERR(( "Empty List" ));
        return BERR_INVALID_PARAMETER;
    }

    /* Read RDC_desc_x_config */
    ulRegVal = BRDC_Slot_P_Read32(hSlot, BCHP_RDC_desc_0_config + hSlot->ulRegOffset);

    /* Clear count */
    ulRegVal &= ~BCHP_MASK(RDC_desc_0_config, count);

    /* Set count and update register */
    ulRegVal |= BCHP_FIELD_DATA(RDC_desc_0_config, count, ulCount-1);

    pulStart   = hList->pulRULAddr;
    pulCurrent = pulStart + hList->ulEntries;

    /* hList RUL programs slave slot config register */
    *pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pulCurrent++ = BRDC_REGISTER(BCHP_RDC_desc_0_config + hSlot->ulRegOffset);
    *pulCurrent++ = ulRegVal;

    /* update list size */
    BRDC_List_SetNumEntries_isr(hList, (uint32_t)(pulCurrent - pulStart));

    BDBG_LEAVE(BRDC_Slot_RulConfigRulSize_isr);
    return BERR_SUCCESS;
}

/******************* Chained RUL programming *******************************/

/***************************************************************************
 * This function programs RUL to config the slave slot trigger;
 */
BERR_Code BRDC_Slot_RulConfigSlaveTrigger_isr
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_Slot_Handle                 hSlotSlave,
      BRDC_List_Handle                 hList,
      BRDC_Trigger                     eRDCTrigger,
      bool                             bRecurring )
{
    uint32_t ulTrigSelect;
    uint32_t *pulStart;
    uint32_t *pulCurrent;
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_Slot_RulConfigSlaveTrigger_isr);

    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);
    BDBG_OBJECT_ASSERT(hSlotSlave, BRDC_SLT);

    if( hList->ulEntries < 1 )
    {
        BDBG_ERR(( "Empty List." ));
        return BERR_TRACE(BRDC_SLOT_ERR_EMPTY_LIST);
    }

    /* Get trigger select value. */
    ulTrigSelect = hSlot->hRdc->aTrigInfo[eRDCTrigger].ulTrigVal;

    /* Update the desc_config register for trigger select. */
    pulStart   = hList->pulRULAddr;
    pulCurrent = pulStart + hList->ulEntries;

    /* hList RUL programs slave slot config register */
    *pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pulCurrent++ = BRDC_REGISTER(BCHP_RDC_desc_0_config + hSlotSlave->ulRegOffset);
    *pulCurrent++ =
            BCHP_FIELD_DATA(RDC_desc_0_config, enable,         1            ) |
            BCHP_FIELD_DATA(RDC_desc_0_config, repeat,         bRecurring   ) |
            BCHP_FIELD_DATA(RDC_desc_0_config, trigger_select, ulTrigSelect ) |
            /* place holder for the real slave RUL size; */
            BCHP_FIELD_DATA(RDC_desc_0_config, count,          0 );
    hList->ulEntries = (uint32_t)(pulCurrent - pulStart);

    /* store the location of the previous desc_config setting; */
    hSlot->pulRulConfigPrevVal = hSlot->pulRulConfigVal;
    hSlot->pulRulConfigVal = pulCurrent - 1;

    /* if master slot, always use current pointer */
    if(hSlot != hSlotSlave) hSlot->pulRulConfigPrevVal = hSlot->pulRulConfigVal;

    BDBG_LEAVE(BRDC_Slot_RulConfigSlaveTrigger_isr);
    return err;
}

/***************************************************************************
 * This function program previous RUL to config the length of the next RUL;
 */
BERR_Code BRDC_Slot_RulConfigCount_isr
    ( BRDC_Slot_Handle                 hSlot,
      uint32_t                         ulCount )
{
    BERR_Code         err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_Slot_RulConfigCount_isr);

    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);
    BDBG_ASSERT(hSlot->pulRulConfigPrevVal);

    /* (0) update previous RUL's desc_config setting, specifically 'count'; */
    *(hSlot->pulRulConfigPrevVal) = (*(hSlot->pulRulConfigPrevVal) &
        (~BCHP_MASK(RDC_desc_0_config, count))) |
        BCHP_FIELD_DATA(RDC_desc_0_config, count, ulCount - 1);

    BDBG_LEAVE(BRDC_Slot_RulConfigCount_isr);
    return err;
}

/***************************************************************************
 * This function build RUL to program slot descriptor to point to the immediate
 * next chained RUL;
 */
uint32_t BRDC_Slot_RulSetNextAddr_isr
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_List_Handle                 hList )
{
    uint32_t *pulStart;
    uint32_t *pulCurrent;
    uint32_t  ulBitMask;
    uint32_t  ulScrap;

    BDBG_ENTER(BRDC_Slot_RulSetNextAddr_isr);

    BDBG_OBJECT_ASSERT(hSlot, BRDC_SLT);
    BDBG_OBJECT_ASSERT(hList, BRDC_LST);
    BDBG_ASSERT(hList->hRdc == hSlot->hRdc);

    /* Can't set empty list. */
    if( hList->ulEntries < 1 )
    {
        BDBG_ERR(( "Empty List" ));
        return 0;
    }

    /* Update the desc_addr to point to the next sub_RUL.
     * NOTE: desc addr has to be 256-bit or 8-dword aligned!! */
    ulBitMask = (1 << 3) - 1;

    pulStart   = hList->pulRULAddr;
    pulCurrent = pulStart + hList->ulEntries;

    *pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pulCurrent++ = BRDC_REGISTER(BCHP_RDC_desc_0_addr + hSlot->ulRegOffset);
    hList->ulEntries = (uint32_t)(pulCurrent + 1 - pulStart);
    *pulCurrent = hList->ulAddrOffset +
        ((hList->ulEntries + ulBitMask) & (~ulBitMask)) * sizeof(uint32_t);

    /* return the scrap size in dwords */
    ulScrap = (hList->ulEntries & ulBitMask) ?
        (ulBitMask - (hList->ulEntries & ulBitMask) + 1) : 0;

    /* update list size */
    hList->ulEntries += ulScrap;

    BDBG_LEAVE(BRDC_Slot_RulSetNextAddr_isr);
    return ulScrap;
}

/***************************************************************************
 * This function build the master RUL to program the slave slot to point to
 * the head of the slave RULs
 */
BERR_Code BRDC_List_RulSetSlaveListHead_isr
    ( BRDC_List_Handle                 hList,
      BRDC_Slot_Handle                 hSlotSlave,
      BRDC_List_Handle                 hListSlave )
{
    uint32_t *pulStart;
    uint32_t *pulCurrent;
    BERR_Code  err = BERR_SUCCESS;
    BDBG_ENTER(BRDC_List_RulSetSlaveListHead_isr);

    pulStart   = hList->pulRULAddr;
    pulCurrent = pulStart + hList->ulEntries;

    /* (1) point desc_addr to the next RUL; CAREFUL!! */
    *pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pulCurrent++ = BRDC_REGISTER(BCHP_RDC_desc_0_addr + hSlotSlave->ulRegOffset);
    *pulCurrent++ = hListSlave->ulAddrOffset;
    hList->ulEntries = (uint32_t)(pulCurrent - pulStart);

    /* from this upon, the desc_config/addr have to be programmed by chained
       RULs themselves; */
    BDBG_LEAVE(BRDC_List_RulSetSlaveListHead_isr);
    return err;
}

/***************************************************************************
 * This function build the tail of the chained slave-RULs:
 * it simply disable the slave slot itself;
 */
BERR_Code BRDC_Slot_RulConfigSlaveListTail_isr
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_List_Handle                 hList )
{
    uint32_t ulTrigSelect;
    uint32_t *pulStart;
    uint32_t *pulCurrent;
    BERR_Code  err = BERR_SUCCESS;
    BDBG_ENTER(BRDC_Slot_RulConfigSlaveListTail_isr);

    /* Get trigger select value. */
    ulTrigSelect = hSlot->hRdc->aTrigInfo[BRDC_Trigger_UNKNOWN].ulTrigVal;

    pulStart   = hList->pulRULAddr;
    pulCurrent = pulStart + hList->ulEntries;

    /* (1) disable desc_config  */
    *pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pulCurrent++ = BRDC_REGISTER(BCHP_RDC_desc_0_config + hSlot->ulRegOffset);
    *pulCurrent++ =
            BCHP_FIELD_DATA(RDC_desc_0_config, enable,         0            ) |
            BCHP_FIELD_DATA(RDC_desc_0_config, repeat,         0            ) |
            BCHP_FIELD_DATA(RDC_desc_0_config, trigger_select, ulTrigSelect ) |
            BCHP_FIELD_DATA(RDC_desc_0_config, count,          0            );

    hList->ulEntries = (uint32_t)(pulCurrent - pulStart);

    /* (4) update previous config pointer */
    hSlot->pulRulConfigPrevVal = hSlot->pulRulConfigVal;

    BDBG_LEAVE(BRDC_Slot_RulConfigSlaveListTail_isr);
    return err;
}

/***************************************************************************
 * This function flushes the cached list;
 */
BERR_Code BRDC_Slot_FlushCachedList_isr
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_List_Handle                 hList )
{
    BERR_Code  err = BERR_SUCCESS;
    BSTD_UNUSED(hSlot);
    BDBG_ENTER(BRDC_Slot_FlushCachedList_isr);

    /* Flush the list before setting it to a slot. */

    /* flush the cache; don't change desc_config register; */
    BMMA_FlushCache_isr(hList->hRULBlock,
        hList->pulRULAddr,
        hList->ulEntries * sizeof(uint32_t));

#ifdef BRDC_USE_CAPTURE_BUFFER
    /* Write to log before RUL could possibly be executed */
    BRDC_DBG_WriteCapture_isr(&hList->hRdc->captureBuffer, hSlot, hList);
#endif

    BDBG_LEAVE(BRDC_Slot_FlushCachedList_isr);
    return err;
}

/***************************************************************************
 * This function returns the timer snapshot in microseconds for the slot
 */
uint32_t BRDC_Slot_GetTimerSnapshot_isr
    ( BRDC_Slot_Handle                 hSlot )
{
    uint32_t  ulTimer = 0;

#if(BRDC_P_SUPPORT_TIMESTAMP)
    uint32_t ulOffset = hSlot->eSlotId * (BCHP_RDC_desc_1_tm_snapshot - BCHP_RDC_desc_0_tm_snapshot);

    /* Get tick value */
    ulTimer = BRDC_Slot_P_Read32(hSlot, BCHP_RDC_desc_0_tm_snapshot + ulOffset);

    /* Convert to microseconds */
    ulTimer = ulTimer / BRDC_P_TIMESTAMP_CLOCK_RATE;
#else
    BSTD_UNUSED(hSlot);
#endif

    return ulTimer;
}



/***************************************************************************
 * This function returns the current value for RDC timer in microseconds.
 */
uint32_t BRDC_GetCurrentTimer_isr
    ( BRDC_Handle                      hRdc )
{
    uint32_t   ulTimer = 0;

#if(BRDC_P_SUPPORT_TIMESTAMP)

    /* Get tick value */
    ulTimer = BREG_Read32_isr(hRdc->hReg, BCHP_RDC_timer_data);
    return (ulTimer/ BRDC_P_TIMESTAMP_CLOCK_RATE);

#else
    BSTD_UNUSED(hRdc);

    BDBG_ERR(("RDC timer is not supported in HW"));
    return ulTimer;
#endif
}

/***************************************************************************
 * This function returns the max value for RDC timer in microseconds
 */
uint32_t BRDC_GetTimerMaxValue
    ( BRDC_Handle                      hRdc )
{
    BSTD_UNUSED(hRdc);

#if(BRDC_P_SUPPORT_TIMESTAMP)

    return (BRDC_P_MAX_TIMER_COUNTER / BRDC_P_TIMESTAMP_CLOCK_RATE);

#else

    BDBG_ERR(("RDC timer is not supported in HW"));
    return 0;
#endif
}

/* end of file */
