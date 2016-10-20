/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_INPUT_ROUTER_TYPES_H__
#define NEXUS_INPUT_ROUTER_TYPES_H__

#include "bstd.h"
#include "nexus_types.h"
#ifdef NEXUS_HAS_IR_INPUT
#include "nexus_ir_input.h"
#else
typedef unsigned NEXUS_IrInputEvent;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
client handle
**/
typedef struct NEXUS_InputClient *NEXUS_InputClientHandle;

typedef enum NEXUS_InputRouterDevice
{
    NEXUS_InputRouterDevice_eIrInput,
    NEXUS_InputRouterDevice_eUhfInput,
    NEXUS_InputRouterDevice_eKeypad,
    NEXUS_InputRouterDevice_ePointer, /* mouse */
    NEXUS_InputRouterDevice_eKeyboard,
    NEXUS_InputRouterDevice_eBluetoothRemote,
    NEXUS_InputRouterDevice_eEvdev,
    NEXUS_InputRouterDevice_eMax
} NEXUS_InputRouterDevice;

/**
Summary:
NEXUS_InputRouterCode is a union of various device types.
It allows a multiplexing of simple and complex user input devices.
It allows a multiplexing of OS controlled and Nexus controlled user input devices.
**/
typedef struct NEXUS_InputRouterCode
{
    uint32_t filterMask; /* generic bitmask for filtering. this mask sets the type for the code.
        There are three coordinating filter masks in the input router API: server, client and code.
            1) the client mask requests what type of codes it wants.
            2) the server mask controls what type of codes are sent to each client.
            3) the code mask sets the type for each code.
        The application (server & client) is free to determine any scheme it wants. Nexus does not determine the scheme.
        A suggested, simple scheme is to set filterMask = (1<<NEXUS_InputRouterDevice_eXxx). */
    
    NEXUS_InputRouterDevice deviceType; /* this determines union */
    union {
        struct {
            unsigned index;
            uint32_t code;
            bool repeat;
            unsigned mode;
            NEXUS_IrInputEvent event;
        } irInput;
        struct {
            unsigned tbd;
        } uhfInput;
        struct {
            unsigned index;
            uint32_t code;
            bool repeat;
        } keypad;
        struct {
            unsigned x, y;
            uint32_t button;
            bool down;
            signed wheel;
        } pointer;
        struct {
            unsigned code;
            bool down;
        } keyboard;
        struct {
            /* passthrough of Linux evdev */
            uint16_t type;
            uint16_t code;
            int32_t value;
        } evdev;
    } data;
} NEXUS_InputRouterCode;

#ifdef __cplusplus
}
#endif

#endif
