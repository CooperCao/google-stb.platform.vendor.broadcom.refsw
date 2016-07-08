/****************************************************************************
 *     Copyright (c) 1999-2015, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Mon Aug 24 11:29:31 2015
 *                 Full Compile MD5 Checksum  cecd4eac458fcdc4b77c82d0630f17be
 *                     (minus title and desc)
 *                 MD5 Checksum               c9a18191e1cdbfad4487ef21d91e95fc
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     126
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_WKTMR_H__
#define BCHP_WKTMR_H__

/***************************************************************************
 *WKTMR - Wakeup timer
 ***************************************************************************/
#define BCHP_WKTMR_EVENT                         0x20417680 /* [RW] Wakeup Timer Register */
#define BCHP_WKTMR_COUNTER                       0x20417684 /* [RW] Wakeup Counter */
#define BCHP_WKTMR_ALARM                         0x20417688 /* [RW] Wakeup Timer Alarm */
#define BCHP_WKTMR_PRESCALER                     0x2041768c /* [RW] Wakeup Timer Prescaler */
#define BCHP_WKTMR_PRESCALER_VAL                 0x20417690 /* [RO] Wakeup Timer Prescaler Value */

/***************************************************************************
 *EVENT - Wakeup Timer Register
 ***************************************************************************/
/* WKTMR :: EVENT :: reserved0 [31:01] */
#define BCHP_WKTMR_EVENT_reserved0_MASK                            0xfffffffe
#define BCHP_WKTMR_EVENT_reserved0_SHIFT                           1

/* WKTMR :: EVENT :: wktmr_alarm_event [00:00] */
#define BCHP_WKTMR_EVENT_wktmr_alarm_event_MASK                    0x00000001
#define BCHP_WKTMR_EVENT_wktmr_alarm_event_SHIFT                   0
#define BCHP_WKTMR_EVENT_wktmr_alarm_event_DEFAULT                 0x00000000

/***************************************************************************
 *COUNTER - Wakeup Counter
 ***************************************************************************/
/* WKTMR :: COUNTER :: wktmr_counter [31:00] */
#define BCHP_WKTMR_COUNTER_wktmr_counter_MASK                      0xffffffff
#define BCHP_WKTMR_COUNTER_wktmr_counter_SHIFT                     0
#define BCHP_WKTMR_COUNTER_wktmr_counter_DEFAULT                   0x00000000

/***************************************************************************
 *ALARM - Wakeup Timer Alarm
 ***************************************************************************/
/* WKTMR :: ALARM :: wktmr_alarm [31:00] */
#define BCHP_WKTMR_ALARM_wktmr_alarm_MASK                          0xffffffff
#define BCHP_WKTMR_ALARM_wktmr_alarm_SHIFT                         0
#define BCHP_WKTMR_ALARM_wktmr_alarm_DEFAULT                       0x00000000

/***************************************************************************
 *PRESCALER - Wakeup Timer Prescaler
 ***************************************************************************/
/* WKTMR :: PRESCALER :: reserved0 [31:25] */
#define BCHP_WKTMR_PRESCALER_reserved0_MASK                        0xfe000000
#define BCHP_WKTMR_PRESCALER_reserved0_SHIFT                       25

/* WKTMR :: PRESCALER :: wktmr_prescaler [24:00] */
#define BCHP_WKTMR_PRESCALER_wktmr_prescaler_MASK                  0x01ffffff
#define BCHP_WKTMR_PRESCALER_wktmr_prescaler_SHIFT                 0
#define BCHP_WKTMR_PRESCALER_wktmr_prescaler_DEFAULT               0x019bfcc0

/***************************************************************************
 *PRESCALER_VAL - Wakeup Timer Prescaler Value
 ***************************************************************************/
/* WKTMR :: PRESCALER_VAL :: reserved0 [31:25] */
#define BCHP_WKTMR_PRESCALER_VAL_reserved0_MASK                    0xfe000000
#define BCHP_WKTMR_PRESCALER_VAL_reserved0_SHIFT                   25

/* WKTMR :: PRESCALER_VAL :: wktmr_prescaler_val [24:00] */
#define BCHP_WKTMR_PRESCALER_VAL_wktmr_prescaler_val_MASK          0x01ffffff
#define BCHP_WKTMR_PRESCALER_VAL_wktmr_prescaler_val_SHIFT         0
#define BCHP_WKTMR_PRESCALER_VAL_wktmr_prescaler_val_DEFAULT       0x00000000

#endif /* #ifndef BCHP_WKTMR_H__ */

/* End of File */
