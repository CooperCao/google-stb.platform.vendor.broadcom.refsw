/******************************************************************************
 *    (c)2008-2011 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 *****************************************************************************/

#ifndef _INCLUDE_IR_EVENT_H_
#define _INCLUDE_IR_EVENT_H_

/*
*	For mapping hardware codes to diamond key codes.
*/
#define VIZIO_CUSTOM_CODE 0xFB04		/* Little endian swapped */
#define CODEMAP_LAST_ENTRY (-1)
#define REPEAT_MILLI_SECS   200

typedef struct KeyMapElement {
    int		ae_code;
    int		hwEventCode;
	bool	allowRepeats;
} KeyMapElement;

/* Updated Key codes to match Stagecraft 1.1 (include/ae/stagecraft/StagecraftKeyDefs.h) */

#define BCM_AEKEY_ALT                       0x00400012 /* used to dispatch Key.ALT */
#define BCM_AEKEY_BACKSPACE                 0x00400008 /* used to dispatch Key.BACKSPACE */
#define BCM_AEKEY_CAPSLOCK                  0x00400014 /* used to dispatch Key.CAPSLOCK */
#define BCM_AEKEY_CONTROL                   0x00400011 /* used to dispatch Key.CONTROL */
#define BCM_AEKEY_INSERT                    0x0040002D /* used to dispatch Key.INSERT */
#define BCM_AEKEY_DELETEKEY                 0x0040002E /* used to dispatch Key.DELETEKEY */
#define BCM_AEKEY_DOWN                      0x00400028 /* used to dispatch Key.DOWN */
#define BCM_AEKEY_END                       0x00400023 /* used to dispatch Key.END */
#define BCM_AEKEY_ENTER                     0x0040000D /* used to dispatch Key.ENTER */
#define BCM_AEKEY_ESCAPE                    0x0040001B /* used to dispatch Key.ESCAPE */
#define BCM_AEKEY_HOME                      0x00400024 /* used to dispatch Key.HOME */
#define BCM_AEKEY_LEFT                      0x00400025 /* used to dispatch Key.LEFT */
#define BCM_AEKEY_PGDN                      0x00400022 /* used to dispatch Key.PGDN */
#define BCM_AEKEY_PGUP                      0x00400021 /* used to dispatch Key.PGUP */
#define BCM_AEKEY_RIGHT                     0x00400027 /* used to dispatch Key.RIGHT */
#define BCM_AEKEY_SHIFT                     0x00400010 /* used to dispatch Key.SHIFT */
#define BCM_AEKEY_SPACE                     0x00400020 /* used to dispatch Key.SPACE */
#define BCM_AEKEY_TAB                       0x00400009 /* used to dispatch Key.TAB */
#define BCM_AEKEY_UP                        0x00400026 /* used to dispatch Key.UP */

#define BCM_AEKEY_TERMINATEANIMATION        0x00400100 /* ctrl: unconditional terminate */

#define BCM_AEKEY_POWER                     0x01000000 /* basic power toggle */
#define BCM_AEKEY_VOLUME_UP                 0x01000001 /* volume up */
#define BCM_AEKEY_VOLUME_DOWN               0x01000002 /* volume down */
#define BCM_AEKEY_VOLUME_MUTE               0x01000003 /* volume mute */
#define BCM_AEKEY_CHANNEL_UP                0x01000004 /* channel up */
#define BCM_AEKEY_CHANNEL_DOWN              0x01000005 /* channel down */
#define BCM_AEKEY_RECORD                    0x01000006 /* record item or engage record transport mode */
#define BCM_AEKEY_PLAY                      0x01000007 /* engage play transport mode */
#define BCM_AEKEY_PAUSE                     0x01000008 /* engage pause transport mode */
#define BCM_AEKEY_STOP                      0x01000009 /* engage stop transport mode */
#define BCM_AEKEY_FAST_FORWARD              0x0100000A /* engage fast-forward transport mode */
#define BCM_AEKEY_REWIND                    0x0100000B /* engage rewind transport mode */
#define BCM_AEKEY_SKIP_FORWARD              0x0100000C /* quick skip ahead (usually 30 seconds) */
#define BCM_AEKEY_SKIP_BACKWARD             0x0100000D /* quick skip backward (usually 7-10 seconds) */
#define BCM_AEKEY_NEXT                      0x0100000E /* skip to next track or chapter */
#define BCM_AEKEY_PREVIOUS                  0x0100000F /* skip to previous track or chapter */
#define BCM_AEKEY_LIVE                      0x01000010 /* return to live [position in broadcast] */
#define BCM_AEKEY_LAST                      0x01000011 /* watch last channel or show watched */
#define BCM_AEKEY_MENU                      0x01000012 /* engage menu */
#define BCM_AEKEY_INFO                      0x01000013 /* info button */
#define BCM_AEKEY_GUIDE                     0x01000014 /* engage program guide */
#define BCM_AEKEY_EXIT                      0x01000015 /* exits current application mode */
#define BCM_AEKEY_BACK                      0x01000016 /* return back to previous page in application */
#define BCM_AEKEY_AUDIO                     0x01000017 /* select the audio mode */
#define BCM_AEKEY_SUBTITLE                  0x01000018 /* toggle subtitles */
#define BCM_AEKEY_DVR                       0x01000019 /* engage dvr application mode */
#define BCM_AEKEY_VOD                       0x0100001A /* engage video on demand */
#define BCM_AEKEY_INPUT                     0x0100001B /* cycle input */
#define BCM_AEKEY_SETUP                     0x0100001C /* engage setup application or menu */
#define BCM_AEKEY_HELP                      0x0100001D /* engage help application or context-sensitive help */
#define BCM_AEKEY_MASTER_SHELL              0x0100001E /* engage "Master Shell" e.g. TiVo or other vendor button */
#define BCM_AEKEY_RED                       0x0100001F /* red function key button */
#define BCM_AEKEY_GREEN                     0x01000020 /* green function key button */
#define BCM_AEKEY_YELLOW                    0x01000021 /* yellow function key button */
#define BCM_AEKEY_BLUE                      0x01000022 /* blue function key button */

#define BCM_AEKEY_ASPECT                    0x01000023 /* Aspect ratio */


#endif /* #ifndef _INCLUDE_IR_EVENT_H_ */
