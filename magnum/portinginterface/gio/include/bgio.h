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
#ifndef BGIO_H__
#define BGIO_H__

#include "bchp.h"          /* Chip information */
#include "breg_mem.h"      /* Chip register access (memory mapped). */

#ifdef __cplusplus
extern "C" {
#endif


/****************************=Module Overview: *****************************
GPIO module provides porting interface API for higher level software to
control hardware GPIO pins. The API exposes hardware functionalities and
hides the hardware and software details. It is targeted for Hydra core based
chip, and is also intended to fit future chips.

On a 7038 chip there are 67 pins that could be used as GPIO pins. The GPIO
pins (hardware) are identified with enum of BGIO_PinId, and are represented
as BGIO_Pin sub-module in software.

A GPIO pin could be physically wired to a bus or a connection port of some
device outside the 7038 chip. Through this GPIO pin, user could then write
an application program to accept information from and/or to interact with
the devices on the bus or the directly wired device, by reading from and/or
writing to the GPIO pin. The value read from or written to a pin is either
0 (low) or 1 (high).

The application software need to get a GPIO module handle by calling
BGIO_Open, and to use this module handle to create a sub-module handle to
the GPIO pin by calling BGIO_Pin_Create. The sub-module handle could then
be used to configure, to read from, and to write to the pin. All GPIO pin sub-
module functions are prefixed with BGIO_Pin_.

When BGIO needs to read a register and to modify it during pin configuring
or value setting, the reading from and writing to register are guarded with
BKNI critical section.

As creating the BGIO_Pin sub-module for a GPIO pin, its PinId is passed to
BGIO_Pin_Create as an input parameter, the corresponding pin control mux
register is automatically configured by BGIO module to set the pin to work
as a GPIO pin (notice that the physical pins are typically shared with other
function modules of the chip). If some other function block is using the
pin, the block's function might not perform correctly.

As an usage example, user could physically connect two GPIO pins to the BSC
bus, one to SCL clock line and another to SDA data line. And then write an
application program to implement a BSC slave device, to interact with all
other devices on the BSC bus.

The virtual BSC slave device could be used to read and write registers,
memory, input / output ports in the devices on the BSC bus, for debug or
function purpose. Notice that some modules IN the 7038 chip might also be
connected with the BSC bus through some other pins.

GPIO Pin Type
-------------

Among the 67 GPIO pins, 59 of them are standard GPIO pins and the
remaining 8 are special GPIO pins. A standard GPIO could be programmed to be
a type of input, push-pull output, or open-drain output, and a special GPIO
pin could only work in type of input and open-drain output.

When programmed to be type of input, it can only be read from, and will not
drive the bus or the port connected to it. The value read back could 0 (low)
or 1 (high).

During working as type of push-pull output, it is ideally the unique driver
of the connected bus or port, till the type changes. The value to drive out
is either 0 (low) or 1 (high). It could change by setting the value of the
GPIO pin. However, it could also be read from, in order to monitor the
activity. Driving 0 out is also called pull-down, and driving 1 out is
called push-up.

A pin of open drain output type is typically connected to a wired-AND bus
line. The line is high (1) if no one pulls down it, and if any device pulls
down it, it becomes low (0). The application program could read it and
pulling it down (and then releasing it).

The software configured GPIO pin type must match the type of bus line or
port the GPIO pin is physically connected to. For example, if a GPIO pin is
configured as push-pull type and its connected bus is a wired-AND bus, or if
a GPIO pin is configured as open-drain type and its connected bus is not a
wired-AND type, BGIO's behavior is undetermined.

Please notice that the type could be dynamically changed by software. It is
possible that the application program sets the GPIO pin as input type,
reads a few bits, and then changes the pin type to push-pull output and then
writes a few bits, and so on.

Interrupt
---------

A GPIO pin could generate interrupt when certain condition happens. The
conditions are represented in enum BGIO_IntrMode. BGIO_Pin sub-module
provides API function to check the level 3 interrupt status, to clear the
level 3 interrupt status, to set interrupt mode, to enable the interrupt
for the GPIO pin.

Application software is responsible to connect / register / enable level 1
and level 2 interrupt.
****************************************************************************/

/***************************************************************************
Summary:
    The handle for the GPIO module.

Description:
    This is an opaque handle that application created with BGIO_Open.
    BGIO_Handle represents the context of the GPIO module. It is needed
    before using any other API functions of GPIO module. The higher level
    software is responsive to ensure that there exists only one BGIO_Handle
    per chip.

    When the GPIO module handle is no longer needed, it should be closed
    by BGIO_Close.

See also:
    BGIO_Open, BGIO_Close
    BGIO_Pin_Handle, BGIO_Pin_Create
****************************************************************************/
typedef struct BGIO_P_Context *BGIO_Handle;

/***************************************************************************
Summary:
    GPIO pin sub-module handle, or GPIO pin handle for simplicity.

Description:
    This is an opaque handle that holds the context for a GPIO pin. It is
    needed when the GPIO pin is configured or used.

    GPIO pin handle is created with BGIO_Pin_Create. There are 67 GPIO pins
    (59 standard and 8 special) on a 7038 chip. When the GPIO pin handle is
    created, its pin ID is needed.

    When the GPIO pin sub-module handle is no longer needed, it should be
    destroyed by BGIO_Destroy.

    Please refer to the GPIO module overview for more usage info.

See Also:
    GPIO, BGIO_Handle
    BGIO_Pin_Create, BGIO_Pin_Destroy
    BGIO_Pin_GetId,
    BGIO_Pin_SetType, BGIO_Pin_GetType,
    BGIO_Pin_GetValue,
    BGIO_Pin_PushPull_SetValue,
    BGIO_Pin_OpenDrain_PullDown, BGIO_Pin_OpenDrain_Release,
    BGIO_Pin_SetIntrMode, BGIO_Pin_GetIntrMode,
    BGIO_Pin_GetIntrStatus, BGIO_Pin_ClearIntrStatus.
***************************************************************************/
typedef struct BGIO_P_Pin_Context        *BGIO_Pin_Handle;

/***************************************************************************
Summary:
    This enumeration represents GPIO pins

Description:
    BGIO_PinId is an enumeration which represents all GPIO pins managed by
    GPIO module.

    Among the 67 GPIO pins, 59 of them are standard GPIO pins and the
    remaining 8 are special GPIO pins. A standard GPIO could be programmed
    to be a type of input, push-pull output, or open-drain output, and a
    special GPIO pin could only work in type of input and open-drain output.

See Also:
    BGIO_Pin_Create, BGIO_Pin_GetId
***************************************************************************/
typedef enum BGIO_PinId
{
    BGIO_PinId_eGpio000,    /* standard GPIO pin 000 */
    BGIO_PinId_eGpio001,    /* standard GPIO pin 001 */
    BGIO_PinId_eGpio002,    /* standard GPIO pin 002 */
    BGIO_PinId_eGpio003,    /* standard GPIO pin 003 */
    BGIO_PinId_eGpio004,    /* standard GPIO pin 004 */
    BGIO_PinId_eGpio005,    /* standard GPIO pin 005 */
    BGIO_PinId_eGpio006,    /* standard GPIO pin 006 */
    BGIO_PinId_eGpio007,    /* standard GPIO pin 007 */
    BGIO_PinId_eGpio008,    /* standard GPIO pin 008 */
    BGIO_PinId_eGpio009,    /* standard GPIO pin 009 */
    BGIO_PinId_eGpio010,    /* standard GPIO pin 010 */
    BGIO_PinId_eGpio011,    /* standard GPIO pin 011 */
    BGIO_PinId_eGpio012,    /* standard GPIO pin 012 */
    BGIO_PinId_eGpio013,    /* standard GPIO pin 013 */
    BGIO_PinId_eGpio014,    /* standard GPIO pin 014 */
    BGIO_PinId_eGpio015,    /* standard GPIO pin 015 */
    BGIO_PinId_eGpio016,    /* standard GPIO pin 016 */
    BGIO_PinId_eGpio017,    /* standard GPIO pin 017 */
    BGIO_PinId_eGpio018,    /* standard GPIO pin 018 */
    BGIO_PinId_eGpio019,    /* standard GPIO pin 019 */
    BGIO_PinId_eGpio020,    /* standard GPIO pin 020 */
    BGIO_PinId_eGpio021,    /* standard GPIO pin 021 */
    BGIO_PinId_eGpio022,    /* standard GPIO pin 022 */
    BGIO_PinId_eGpio023,    /* standard GPIO pin 023 */
    BGIO_PinId_eGpio024,    /* standard GPIO pin 024 */
    BGIO_PinId_eGpio025,    /* standard GPIO pin 025 */
    BGIO_PinId_eGpio026,    /* standard GPIO pin 026 */
    BGIO_PinId_eGpio027,    /* standard GPIO pin 027 */
    BGIO_PinId_eGpio028,    /* standard GPIO pin 028 */
    BGIO_PinId_eGpio029,    /* standard GPIO pin 029 */
    BGIO_PinId_eGpio030,    /* standard GPIO pin 030 */
    BGIO_PinId_eGpio031,    /* standard GPIO pin 031 */
    BGIO_PinId_eGpio032,    /* standard GPIO pin 032 */
    BGIO_PinId_eGpio033,    /* standard GPIO pin 033 */
    BGIO_PinId_eGpio034,    /* standard GPIO pin 034 */
    BGIO_PinId_eGpio035,    /* standard GPIO pin 035 */
    BGIO_PinId_eGpio036,    /* standard GPIO pin 036 */
    BGIO_PinId_eGpio037,    /* standard GPIO pin 037 */
    BGIO_PinId_eGpio038,    /* standard GPIO pin 038 */
    BGIO_PinId_eGpio039,    /* standard GPIO pin 039 */
    BGIO_PinId_eGpio040,    /* standard GPIO pin 040 */
    BGIO_PinId_eGpio041,    /* standard GPIO pin 041 */
    BGIO_PinId_eGpio042,    /* standard GPIO pin 042 */
    BGIO_PinId_eGpio043,    /* standard GPIO pin 043 */
    BGIO_PinId_eGpio044,    /* standard GPIO pin 044 */
    BGIO_PinId_eGpio045,    /* standard GPIO pin 045 */
    BGIO_PinId_eGpio046,    /* standard GPIO pin 046 */
    BGIO_PinId_eGpio047,    /* standard GPIO pin 047 */
    BGIO_PinId_eGpio048,    /* standard GPIO pin 048 */
    BGIO_PinId_eGpio049,    /* standard GPIO pin 049 */
    BGIO_PinId_eGpio050,    /* standard GPIO pin 050 */
    BGIO_PinId_eGpio051,    /* standard GPIO pin 051 */
    BGIO_PinId_eGpio052,    /* standard GPIO pin 052 */
    BGIO_PinId_eGpio053,    /* standard GPIO pin 053 */
    BGIO_PinId_eGpio054,    /* standard GPIO pin 054 */
    BGIO_PinId_eGpio055,    /* standard GPIO pin 055 */
    BGIO_PinId_eGpio056,    /* standard GPIO pin 056 */
    BGIO_PinId_eGpio057,    /* standard GPIO pin 057 */
    BGIO_PinId_eGpio058,    /* standard GPIO pin 058 */
    BGIO_PinId_eGpio059,    /* standard GPIO pin 059 */
    BGIO_PinId_eGpio060,    /* standard GPIO pin 060 */
    BGIO_PinId_eGpio061,    /* standard GPIO pin 061 */
    BGIO_PinId_eGpio062,    /* standard GPIO pin 062 */
    BGIO_PinId_eGpio063,    /* standard GPIO pin 063 */
    BGIO_PinId_eGpio064,    /* standard GPIO pin 064 */
    BGIO_PinId_eGpio065,    /* standard GPIO pin 065 */
    BGIO_PinId_eGpio066,    /* standard GPIO pin 066 */
    BGIO_PinId_eGpio067,    /* standard GPIO pin 067 */
    BGIO_PinId_eGpio068,    /* standard GPIO pin 068 */
    BGIO_PinId_eGpio069,    /* standard GPIO pin 069 */
    BGIO_PinId_eGpio070,    /* standard GPIO pin 070 */
    BGIO_PinId_eGpio071,    /* standard GPIO pin 071 */
    BGIO_PinId_eGpio072,    /* standard GPIO pin 072 */
    BGIO_PinId_eGpio073,    /* standard GPIO pin 073 */
    BGIO_PinId_eGpio074,    /* standard GPIO pin 074 */
    BGIO_PinId_eGpio075,    /* standard GPIO pin 075 */
    BGIO_PinId_eGpio076,    /* standard GPIO pin 076 */
    BGIO_PinId_eGpio077,    /* standard GPIO pin 077 */
    BGIO_PinId_eGpio078,    /* standard GPIO pin 078 */
    BGIO_PinId_eGpio079,    /* standard GPIO pin 079 */
    BGIO_PinId_eGpio080,    /* standard GPIO pin 080 */
    BGIO_PinId_eGpio081,    /* standard GPIO pin 081 */
    BGIO_PinId_eGpio082,    /* standard GPIO pin 082 */
    BGIO_PinId_eGpio083,    /* standard GPIO pin 083 */
    BGIO_PinId_eGpio084,    /* standard GPIO pin 084 */
    BGIO_PinId_eGpio085,    /* standard GPIO pin 085 */
    BGIO_PinId_eGpio086,    /* standard GPIO pin 086 */
    BGIO_PinId_eGpio087,    /* standard GPIO pin 087 */
    BGIO_PinId_eGpio088,    /* standard GPIO pin 088 */
    BGIO_PinId_eGpio089,    /* standard GPIO pin 089 */
    BGIO_PinId_eGpio090,    /* standard GPIO pin 090 */
    BGIO_PinId_eGpio091,    /* standard GPIO pin 091 */
    BGIO_PinId_eGpio092,    /* standard GPIO pin 092 */
    BGIO_PinId_eGpio093,    /* standard GPIO pin 093 */
    BGIO_PinId_eGpio094,    /* standard GPIO pin 094 */
    BGIO_PinId_eGpio095,    /* standard GPIO pin 095 */
    BGIO_PinId_eGpio096,    /* standard GPIO pin 096 */
    BGIO_PinId_eGpio097,    /* standard GPIO pin 097 */
    BGIO_PinId_eGpio098,    /* standard GPIO pin 098 */
    BGIO_PinId_eGpio099,    /* standard GPIO pin 099 */
    BGIO_PinId_eGpio100,    /* standard GPIO pin 100 */
    BGIO_PinId_eGpio101,    /* standard GPIO pin 101 */
    BGIO_PinId_eGpio102,    /* standard GPIO pin 102 */
    BGIO_PinId_eGpio103,    /* standard GPIO pin 103 */
    BGIO_PinId_eGpio104,    /* standard GPIO pin 104 */
    BGIO_PinId_eGpio105,    /* standard GPIO pin 105 */
    BGIO_PinId_eGpio106,    /* standard GPIO pin 106 */
    BGIO_PinId_eGpio107,    /* standard GPIO pin 107 */
    BGIO_PinId_eGpio108,    /* standard GPIO pin 108 */
    BGIO_PinId_eGpio109,    /* standard GPIO pin 109 */
    BGIO_PinId_eGpio110,    /* standard GPIO pin 110 */
    BGIO_PinId_eGpio111,    /* standard GPIO pin 111 */
    BGIO_PinId_eGpio112,    /* standard GPIO pin 112 */
    BGIO_PinId_eGpio113,    /* standard GPIO pin 113 */
    BGIO_PinId_eGpio114,    /* standard GPIO pin 114 */
    BGIO_PinId_eGpio115,    /* standard GPIO pin 115 */
    BGIO_PinId_eGpio116,    /* standard GPIO pin 116 */
    BGIO_PinId_eGpio117,    /* standard GPIO pin 117 */
    BGIO_PinId_eGpio118,    /* standard GPIO pin 118 */
    BGIO_PinId_eGpio119,    /* standard GPIO pin 119 */
    BGIO_PinId_eGpio120,    /* standard GPIO pin 120 */
    BGIO_PinId_eGpio121,    /* standard GPIO pin 121 */
    BGIO_PinId_eGpio122,    /* standard GPIO pin 122 */
    BGIO_PinId_eGpio123,    /* standard GPIO pin 123 */
    BGIO_PinId_eGpio124,    /* standard GPIO pin 124 */
    BGIO_PinId_eGpio125,    /* standard GPIO pin 125 */
    BGIO_PinId_eGpio126,    /* standard GPIO pin 126 */
    BGIO_PinId_eGpio127,    /* standard GPIO pin 127 */
    BGIO_PinId_eGpio128,    /* standard GPIO pin 128 */
    BGIO_PinId_eGpio129,    /* standard GPIO pin 129 */
    BGIO_PinId_eGpio130,    /* standard GPIO pin 130 */
    BGIO_PinId_eGpio131,    /* standard GPIO pin 131 */
    BGIO_PinId_eGpio132,    /* standard GPIO pin 132 */
    BGIO_PinId_eGpio133,    /* standard GPIO pin 133 */
    BGIO_PinId_eGpio134,    /* standard GPIO pin 134 */
    BGIO_PinId_eGpio135,    /* standard GPIO pin 135 */
    BGIO_PinId_eGpio136,    /* standard GPIO pin 136 */
    BGIO_PinId_eGpio137,    /* standard GPIO pin 137 */
    BGIO_PinId_eGpio138,    /* standard GPIO pin 138 */
    BGIO_PinId_eGpio139,    /* standard GPIO pin 139 */
    BGIO_PinId_eGpio140,    /* standard GPIO pin 140 */
    BGIO_PinId_eGpio141,    /* standard GPIO pin 141 */
    BGIO_PinId_eGpio142,    /* standard GPIO pin 142 */
    BGIO_PinId_eGpio143,    /* standard GPIO pin 143 */
    BGIO_PinId_eGpio144,    /* standard GPIO pin 144 */
    BGIO_PinId_eGpio145,    /* standard GPIO pin 145 */
    BGIO_PinId_eGpio146,    /* standard GPIO pin 146 */
    BGIO_PinId_eGpio147,    /* standard GPIO pin 147 */
    BGIO_PinId_eGpio148,    /* standard GPIO pin 148 */
    BGIO_PinId_eGpio149,    /* standard GPIO pin 149 */

    BGIO_PinId_eSgpio00,    /* special GPIO pin 00 */
    BGIO_PinId_eSgpio01,    /* special GPIO pin 01 */
    BGIO_PinId_eSgpio02,    /* special GPIO pin 02 */
    BGIO_PinId_eSgpio03,    /* special GPIO pin 03 */
    BGIO_PinId_eSgpio04,    /* special GPIO pin 04 */
    BGIO_PinId_eSgpio05,    /* special GPIO pin 05 */
    BGIO_PinId_eSgpio06,    /* special GPIO pin 06 */
    BGIO_PinId_eSgpio07,    /* special GPIO pin 07 */
    BGIO_PinId_eSgpio08,    /* special GPIO pin 08 */
    BGIO_PinId_eSgpio09,    /* special GPIO pin 09 */

    BGIO_PinId_eAgpio00,    /* special AON GPIO pin 00 */
    BGIO_PinId_eAgpio01,    /* special AON GPIO pin 01 */
    BGIO_PinId_eAgpio02,    /* special AON GPIO pin 02 */
    BGIO_PinId_eAgpio03,    /* special AON GPIO pin 03 */
    BGIO_PinId_eAgpio04,    /* special AON GPIO pin 04 */
    BGIO_PinId_eAgpio05,    /* special AON GPIO pin 05 */
    BGIO_PinId_eAgpio06,    /* special AON GPIO pin 06 */
    BGIO_PinId_eAgpio07,    /* special AON GPIO pin 07 */
    BGIO_PinId_eAgpio08,    /* special AON GPIO pin 08 */
    BGIO_PinId_eAgpio09,    /* special AON GPIO pin 09 */
    BGIO_PinId_eAgpio10,    /* special AON GPIO pin 10 */
    BGIO_PinId_eAgpio11,    /* special AON GPIO pin 11 */
    BGIO_PinId_eAgpio12,    /* special AON GPIO pin 12 */
    BGIO_PinId_eAgpio13,    /* special AON GPIO pin 13 */
    BGIO_PinId_eAgpio14,    /* special AON GPIO pin 14 */
    BGIO_PinId_eAgpio15,    /* special AON GPIO pin 15 */
    BGIO_PinId_eAgpio16,    /* special AON GPIO pin 16 */
    BGIO_PinId_eAgpio17,    /* special AON GPIO pin 17 */
    BGIO_PinId_eAgpio18,    /* special AON GPIO pin 18 */
    BGIO_PinId_eAgpio19,    /* special AON GPIO pin 19 */
    BGIO_PinId_eAgpio20,    /* special AON GPIO pin 20 */
    BGIO_PinId_eAgpio21,    /* special AON GPIO pin 21 */
    BGIO_PinId_eAgpio22,    /* special AON GPIO pin 22 */
    BGIO_PinId_eAgpio23,    /* special AON GPIO pin 23 */
    BGIO_PinId_eAgpio24,    /* special AON GPIO pin 24 */
    BGIO_PinId_eAgpio25,    /* special AON GPIO pin 25 */
    BGIO_PinId_eAgpio26,    /* special AON GPIO pin 26 */
    BGIO_PinId_eAgpio27,    /* special AON GPIO pin 27 */

    BGIO_PinId_eAsgpio00, /* standard AON special GPIO 00 */
    BGIO_PinId_eAsgpio01, /* standard AON special GPIO 01 */
    BGIO_PinId_eAsgpio02, /* standard AON special GPIO 02 */
    BGIO_PinId_eAsgpio03, /* standard AON special GPIO 03 */
    BGIO_PinId_eAsgpio04, /* standard AON special GPIO 04 */
    BGIO_PinId_eAsgpio05, /* standard AON special GPIO 05 */
    BGIO_PinId_eAsgpio06, /* standard AON special GPIO 06 */
    BGIO_PinId_eAsgpio07, /* standard AON special GPIO 07 */

    BGIO_PinId_eInvalid
}
BGIO_PinId;

/* for backward compatible */
#define BGIO_PinId_eGpio00   BGIO_PinId_eGpio000
#define BGIO_PinId_eGpio01   BGIO_PinId_eGpio001
#define BGIO_PinId_eGpio02   BGIO_PinId_eGpio002
#define BGIO_PinId_eGpio03   BGIO_PinId_eGpio003
#define BGIO_PinId_eGpio04   BGIO_PinId_eGpio004
#define BGIO_PinId_eGpio05   BGIO_PinId_eGpio005
#define BGIO_PinId_eGpio06   BGIO_PinId_eGpio006
#define BGIO_PinId_eGpio07   BGIO_PinId_eGpio007
#define BGIO_PinId_eGpio08   BGIO_PinId_eGpio008
#define BGIO_PinId_eGpio09   BGIO_PinId_eGpio009
#define BGIO_PinId_eGpio10   BGIO_PinId_eGpio010
#define BGIO_PinId_eGpio11   BGIO_PinId_eGpio011
#define BGIO_PinId_eGpio12   BGIO_PinId_eGpio012
#define BGIO_PinId_eGpio13   BGIO_PinId_eGpio013
#define BGIO_PinId_eGpio14   BGIO_PinId_eGpio014
#define BGIO_PinId_eGpio15   BGIO_PinId_eGpio015
#define BGIO_PinId_eGpio16   BGIO_PinId_eGpio016
#define BGIO_PinId_eGpio17   BGIO_PinId_eGpio017
#define BGIO_PinId_eGpio18   BGIO_PinId_eGpio018
#define BGIO_PinId_eGpio19   BGIO_PinId_eGpio019
#define BGIO_PinId_eGpio20   BGIO_PinId_eGpio020
#define BGIO_PinId_eGpio21   BGIO_PinId_eGpio021
#define BGIO_PinId_eGpio22   BGIO_PinId_eGpio022
#define BGIO_PinId_eGpio23   BGIO_PinId_eGpio023
#define BGIO_PinId_eGpio24   BGIO_PinId_eGpio024
#define BGIO_PinId_eGpio25   BGIO_PinId_eGpio025
#define BGIO_PinId_eGpio26   BGIO_PinId_eGpio026
#define BGIO_PinId_eGpio27   BGIO_PinId_eGpio027
#define BGIO_PinId_eGpio28   BGIO_PinId_eGpio028
#define BGIO_PinId_eGpio29   BGIO_PinId_eGpio029
#define BGIO_PinId_eGpio30   BGIO_PinId_eGpio030
#define BGIO_PinId_eGpio31   BGIO_PinId_eGpio031
#define BGIO_PinId_eGpio32   BGIO_PinId_eGpio032
#define BGIO_PinId_eGpio33   BGIO_PinId_eGpio033
#define BGIO_PinId_eGpio34   BGIO_PinId_eGpio034
#define BGIO_PinId_eGpio35   BGIO_PinId_eGpio035
#define BGIO_PinId_eGpio36   BGIO_PinId_eGpio036
#define BGIO_PinId_eGpio37   BGIO_PinId_eGpio037
#define BGIO_PinId_eGpio38   BGIO_PinId_eGpio038
#define BGIO_PinId_eGpio39   BGIO_PinId_eGpio039
#define BGIO_PinId_eGpio40   BGIO_PinId_eGpio040
#define BGIO_PinId_eGpio41   BGIO_PinId_eGpio041
#define BGIO_PinId_eGpio42   BGIO_PinId_eGpio042
#define BGIO_PinId_eGpio43   BGIO_PinId_eGpio043
#define BGIO_PinId_eGpio44   BGIO_PinId_eGpio044
#define BGIO_PinId_eGpio45   BGIO_PinId_eGpio045
#define BGIO_PinId_eGpio46   BGIO_PinId_eGpio046
#define BGIO_PinId_eGpio47   BGIO_PinId_eGpio047
#define BGIO_PinId_eGpio48   BGIO_PinId_eGpio048
#define BGIO_PinId_eGpio49   BGIO_PinId_eGpio049
#define BGIO_PinId_eGpio50   BGIO_PinId_eGpio050
#define BGIO_PinId_eGpio51   BGIO_PinId_eGpio051
#define BGIO_PinId_eGpio52   BGIO_PinId_eGpio052
#define BGIO_PinId_eGpio53   BGIO_PinId_eGpio053
#define BGIO_PinId_eGpio54   BGIO_PinId_eGpio054
#define BGIO_PinId_eGpio55   BGIO_PinId_eGpio055
#define BGIO_PinId_eGpio56   BGIO_PinId_eGpio056
#define BGIO_PinId_eGpio57   BGIO_PinId_eGpio057
#define BGIO_PinId_eGpio58   BGIO_PinId_eGpio058
#define BGIO_PinId_eGpio59   BGIO_PinId_eGpio059
#define BGIO_PinId_eGpio60   BGIO_PinId_eGpio060
#define BGIO_PinId_eGpio61   BGIO_PinId_eGpio061
#define BGIO_PinId_eGpio62   BGIO_PinId_eGpio062
#define BGIO_PinId_eGpio63   BGIO_PinId_eGpio063
#define BGIO_PinId_eGpio64   BGIO_PinId_eGpio064
#define BGIO_PinId_eGpio65   BGIO_PinId_eGpio065
#define BGIO_PinId_eGpio66   BGIO_PinId_eGpio066
#define BGIO_PinId_eGpio67   BGIO_PinId_eGpio067
#define BGIO_PinId_eGpio68   BGIO_PinId_eGpio068
#define BGIO_PinId_eGpio69   BGIO_PinId_eGpio069
#define BGIO_PinId_eGpio70   BGIO_PinId_eGpio070
#define BGIO_PinId_eGpio71   BGIO_PinId_eGpio071
#define BGIO_PinId_eGpio72   BGIO_PinId_eGpio072
#define BGIO_PinId_eGpio73   BGIO_PinId_eGpio073
#define BGIO_PinId_eGpio74   BGIO_PinId_eGpio074
#define BGIO_PinId_eGpio75   BGIO_PinId_eGpio075
#define BGIO_PinId_eGpio76   BGIO_PinId_eGpio076
#define BGIO_PinId_eGpio77   BGIO_PinId_eGpio077
#define BGIO_PinId_eGpio78   BGIO_PinId_eGpio078
#define BGIO_PinId_eGpio79   BGIO_PinId_eGpio079
#define BGIO_PinId_eGpio80   BGIO_PinId_eGpio080
#define BGIO_PinId_eGpio81   BGIO_PinId_eGpio081
#define BGIO_PinId_eGpio82   BGIO_PinId_eGpio082
#define BGIO_PinId_eGpio83   BGIO_PinId_eGpio083
#define BGIO_PinId_eGpio84   BGIO_PinId_eGpio084
#define BGIO_PinId_eGpio85   BGIO_PinId_eGpio085
#define BGIO_PinId_eGpio86   BGIO_PinId_eGpio086
#define BGIO_PinId_eGpio87   BGIO_PinId_eGpio087
#define BGIO_PinId_eGpio88   BGIO_PinId_eGpio088
#define BGIO_PinId_eGpio89   BGIO_PinId_eGpio089
#define BGIO_PinId_eGpio90   BGIO_PinId_eGpio090
#define BGIO_PinId_eGpio91   BGIO_PinId_eGpio091
#define BGIO_PinId_eGpio92   BGIO_PinId_eGpio092
#define BGIO_PinId_eGpio93   BGIO_PinId_eGpio093
#define BGIO_PinId_eGpio94   BGIO_PinId_eGpio094
#define BGIO_PinId_eGpio95   BGIO_PinId_eGpio095
#define BGIO_PinId_eGpio96   BGIO_PinId_eGpio096
#define BGIO_PinId_eGpio97   BGIO_PinId_eGpio097
#define BGIO_PinId_eGpio98   BGIO_PinId_eGpio098
#define BGIO_PinId_eGpio99   BGIO_PinId_eGpio099

/***************************************************************************
Summary:
    This enumeration represents GPIO pin types

Description:
    BGIO_PinType is an enumeration which represents all GPIO pin types
    supported by GPIO module.

    There are three GPIO pin types. They are input, push-pull output, or
    open-drain output. A standard GPIO could be programmed to be any one of
    the three types, and a special GPIO pin could only work in type of input
    and open-drain output.

    When programmed to be type of input, it can only be read from, and will
    not drive the bus or the port connected to it. The value read back could
    0 (low) or 1 (high).

    During working as type of push-pull output, it is ideally the unique
    driver of the connected bus or port, till the type changes. The value
    to drive out is either 0 (low) or 1 (high). It could change by setting
    the value of the GPIO pin. However, it could also be read from, in order
    to monitor the activity. Driving 0 out is also called pull-down, and
    driving 1 out is called push-up.

    A pin of open drain output type is typically connected to a wired-AND
    bus line. The line is high (1) if no one pulls down it, and if any
    device pulls down it, it becomes low (0). The application program could
    read it and pulling it down (and then releasing it).

    Please notice that the type could be dynamically changed by software. It
    is possible that the application program sets the GPIO pin as input type,
    reads a few bits, and then changes the pin type to push-pull output and
    then writes a few bits, and so on.

See Also:
    BGIO_Pin_SetPinType, BGIO_Pin_GetPinType.
***************************************************************************/
typedef enum BGIO_PinType
{
    BGIO_PinType_eInput,         /* Read only */
    BGIO_PinType_ePushPull,      /* Set and read  */
    BGIO_PinType_eOpenDrain,     /* Read, and pull down or release */
    BGIO_PinType_eInvalid
}
BGIO_PinType;

/***************************************************************************
Summary:
    This enumeration represents GPIO pin value.

Description:
    There are two possible values 0 (low) and 1 (high) for a GPIO pin.
    BGIO_PinValue is an enumeration which represents them.

    A push-pull GPIO pin could drive both 0 (low) and 1 (high) out to the
    bus or connection port wired to this GPIO pin. A open-drain GPIO pin
    could pull-down the connected AND-wired bus line, till it releases it.

    The value of any GPIO pin could be read at any time. If a push-pull GPIO
    pin is read, its value would be the value it drives out. When a open-
    drain GPIO pin pulls-down, the value of 0 (low) will be returned by
    reading. After it releases the bus, the value read from the pin depends
    whether there is any other device on the bus that is pulling down the
    bus.

See Also:
    BGIO_Pin_GetValue, BGIO_Pin_PushPull_SetValue,
    BGIO_Pin_OpenDrain_Release, BGIO_Pin_OpenDrain_PullDown.
***************************************************************************/
typedef enum BGIO_PinValue
{
    BGIO_PinValue_e0 = 0,         /* low, driving low out means pull-down */
    BGIO_PinValue_e1 = 1,         /* high, driving high out means push-up */
    BGIO_PinValue_eInvalid
}
BGIO_PinValue;

/***************************************************************************
Summary:
    This enumeration represents GPIO pin interrupt mode.

Description:
    BGIO_IntrMode is an enumeration which represents whether level 3
    interrupt is disabled or enabled for a GPIO pin, and in what condition
    the GPIO pin should generate interrupt if it is enabled.

    7038 chips only support edge triggering interrupt mode. In this mode the
    pin fires interrupt when its value changes, and user is expected to call
    BGIO_Pin_ClearIntrStatus to clear the interrupt bit after the interrupt
    is served. Besides edge triggering mode, 3560 chips also support level
    triggering. In this mode the pin drive the interrupt bit when it has the
    specified value. To clear a level-triggered interrupt, user is expected
    to change the value of the pin.

See Also:
    BGIO_Pin_SetIntrMode, BGIO_Pin_GetIntrMode,
    BGIO_Pin_ClearIntrStatus
***************************************************************************/
typedef enum BGIO_IntrMode
{
    BGIO_IntrMode_eDisabled,      /* interrupt is disabled */
    BGIO_IntrMode_e0To1,          /* interrupt fires on rising edge */
    BGIO_IntrMode_e1To0,          /* interrupt fires on falling edge */
    BGIO_IntrMode_e0To1_Or_1To0,  /* interrupt fires on both edges */
#if (BCHP_CHIP!=7038) && (BCHP_CHIP!=7438)
    BGIO_IntrMode_e0,             /* interrupt generated on value 0 */
    BGIO_IntrMode_e1,             /* interrupt generated on value 1 */
#endif
    BGIO_IntrMode_eInvalid
}
BGIO_IntrMode;

/***************************************************************************
 * Public API: BGIO generic
 **************************************************************************/

/***************************************************************************
Summary:
    This function opens and initializes the GPIO module.

Description:
    Before any thing could be done to a GPIO pin in the chip, a GPIO module
    handle, BGIO_Handle, must be opened. From this handle, the sub-module
    handle of a GPIO pin could be created.

    It is user's responsibility to ensure that there is only one GPIO module
    handle opened per chip.

    This GPIO module handle should eventually be closed by calling BGIO_Close,
    when it is no longer needed.

Input:
    hChip - The chip handle that application created earlier during chip
    initialization sequence. This handle is used for getting chip
    information, chip revision, and miscellaneous chip configurations.

    hRegister - The register handle that application created earlier during
    chip initialization sequence. This handle is used to access chip
    registers (GPIO registers).

Output:
    phGpio - The opened GPIO module handle.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully opened BGIO.

See Also:
    BGIO_Close,
    BGIO_Pin_Create
**************************************************************************/
BERR_Code BGIO_Open(
    BGIO_Handle *         phGpio ,
    BCHP_Handle           hChip,
    BREG_Handle           hRegister );

/***************************************************************************
Summary:
    Close the GPIO handle.

Description:
    Once this function is called the GPIO handle can no longer be used.

    All GPIO pin sub-module handles should be destroyed before closing the
    BGIO module handle by BGIO_Close.

Input:
    hGpio - The GPIO handle to be closed.

Output:

Returns:
    BERR_INVALID_PARAMETER - hGpio is not a valid GPIO handle.
    BERR_LEAKED_RESOURCE - There is still GPIO sub-module handle not
    destroyed.
    BERR_SUCCESS - Successfully destroyed.

See Also:
    BGIO_Open,
    BGIO_Pin_Destroy.
**************************************************************************/
BERR_Code BGIO_Close(
    BGIO_Handle           hGpio );


/***************************************************************************
 * Public API: GPIO pin sub-module
 **************************************************************************/

/***************************************************************************
Summary:
    Creates a GPIO pin sub-module handle to for a GPIO pin.

Description:
    Once created, the handle to the GPIO pin is required by GPIO pin sub-
    module functions in order to configure and to use the GPIO pin.

    If the sub-module handle has already been created with the same hGpio,
    error is returned. This function also initializes the sub-module to
    default configure.

    During creating the BGIO_Pin sub-module for a GPIO pin, its PinId is
    passed to BGIO_Pin_Create as an input parameter, the corresponding pin
    control mux register is automatically configured by BGIO module to set
    the pin to work as a GPIO pin (notice that the physical pins are
    typically shared with other function modules of the chip). If some other
    function block is using the pin, the block's function might not perform
    correctly.

    Refer to the GPIO module overview for more usage info.

Input:
    hGpio - Handle to the GPIO module.
    ePinId - The hardware GPIO pin ID.

Output:
    phPin - The created GPIO pin sub-module handle. If failure
    occurred phPin will holds NULL.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully created the handle.

See Also:
    BGIO_Pin_Destroy
**************************************************************************/
BERR_Code BGIO_Pin_Create(
    BGIO_Handle           hGpio,
    BGIO_PinId            ePinId,
    BGIO_Pin_Handle *     phPin );

/***************************************************************************
Summary:
    Destroys the GPIO pin sub-module handle of a GPIO pin.

Description:
    Once this function is called the sub-module handle can no longer be
    used.

    GPIO pin sub-module handle should be destroyed before closing the main
    module handle BGIO_Handle by BGIO_Close.

Input:
    hPin - The GPIO pin sub-module handle to destroy.

Output:

Returns:
    BERR_INVALID_PARAMETER - hPin is not a valid GPIO pin sub-module
    handle.
    BERR_SUCCESS - Successfully destroyed

See Also:
    BGIO_Pin_Create
**************************************************************************/
BERR_Code BGIO_Pin_Destroy(
    BGIO_Pin_Handle       hPin );

/***************************************************************************
Summary:
    Query the hardware pin ID from a GPIO pin sub-module handle.

Description:
    The hardware GPIO pin ID is passed to BGIO_Pin_Create when the GPIO pin
    sub-module handle is created.

Input:
    hPin - Sub-module handle to the GPIO pin.

Output:
    pePinId - The returned hardware GPIO pin ID.

Returns:
    BERR_SUCCESS - Got the pin ID successfully.
    BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
    BGIO_Pin_Create.
****************************************************************************/
BERR_Code BGIO_Pin_GetId(
    BGIO_Pin_Handle       hPin,
    BGIO_PinId *          pePinId );

/***************************************************************************
Summary:
    Set GPIO pin type.

Description:
    There are several GPIO pin types supported by GPIO module. Each type
    works differently. This function sets the GPIO pin type.

    The default pin type is BGIO_Pin_eInput.

Input:
    hPin - Sub-module handle to set the type for.
    ePinType - GPIO pin type.

Output:

Returns:
    BERR_SUCCESS - Pin type is set successfully.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BGIO_Pin_GetPinType.
****************************************************************************/
BERR_Code BGIO_Pin_SetType(
    BGIO_Pin_Handle       hPin,
    BGIO_PinType          ePinType );

BERR_Code BGIO_Pin_SetType_isr(
    BGIO_Pin_Handle       hPin,
    BGIO_PinType          ePinType );

/***************************************************************************
Summary:
    Query for GPIO pin type.

Description:
    GPIO pin type is initialized when the GPIO pin sub-module is created,
    and could then be changed by BGIO_Pin_SetType.  This function returns
    the current pin type of the GPIO pin.

Input:
    hPin - The GPIO pin sub-module handle.

Output:
    pePinType - Returned GPIO pin type.

Returns:
    BERR_SUCCESS - Pin type is got successfully.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BGIO_Pin_SetPinType.
****************************************************************************/
BERR_Code BGIO_Pin_GetType(
    BGIO_Pin_Handle       hPin,
    BGIO_PinType *        pePinType );

/***************************************************************************
Summary:
    Get the current value of a GPIO pin.

Description:
    All GPIO pin type could be read from. This function returns the current
    value of a GPIO pin. The value is either BGIO_PinValue_e0 (low) or
    BGIO_PinValue_e1 (high).

Input:
    hPin - The GPIO pin sub-module handle to get value from.

Output:
    pePinValue - Returned current GPIO pin value.

Returns:
    BERR_SUCCESS - The current pin value is read successfully.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BGIO_Pin_PushPull_SetValue,
    BGIO_Pin_OpenDrain_PullDown, BGIO_Pin_OpenDrain_Release
****************************************************************************/
BERR_Code BGIO_Pin_GetValue(
    BGIO_Pin_Handle       hPin,
    BGIO_PinValue *       pePinValue );

/***************************************************************************
Summary:
    Set value for a GPIO pin of type BGIO_PinType_ePushPull.

Description:
    This function sets value for a GPIO pin of type BGIO_PinType_ePushPull.
    This value will then be driven to the bus or connection port connected
    to this GPIO pin, till the value is reset or the pin type is changed.

    If this function is never called for a GPIO pin of BGIO_PinType_ePushPull,
    its default value is BGIO_PinValue_e0.

    If the GPIO pin type is not BGIO_PinType_ePushPull, this function will
    return error code BERR_INVALID_PARAMETER.

Input:
    hPin - The GPIO pin sub-module handle to set value for.
    ePinValue - GPIO pin value.

Output:

Returns:
    BERR_SUCCESS - Pin value is set successfully.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BGIO_Pin_GetValue,
    BGIO_Pin_OpenDrain_PullDown, BGIO_Pin_OpenDrain_Release
    BGIO_Pin_SetType, BGIO_Pin_GetPinType.
****************************************************************************/
BERR_Code BGIO_Pin_PushPull_SetValue(
    BGIO_Pin_Handle       hPin,
    BGIO_PinValue         ePinValue );

BERR_Code BGIO_Pin_PushPull_SetValue_isr(
    BGIO_Pin_Handle       hPin,
    BGIO_PinValue         ePinValue );

/***************************************************************************
Summary:
    Perform pull down for GPIO pin of type BGIO_PinType_eOpenDrain.

Description:
    This function pulls down the AND-wired bus line connected to a GPIO pin
    of type BGIO_PinType_eOpenDrain, till BGIO_Pin_OpenDrain_Release is
    called or the pin type is changed. When a GPIO pin of open drain is
    in the state of pull-down, its value is BGIO_PinValue_e0.

    If this function is never called for a GPIO pin of
    BGIO_PinType_eOpenDrain, it is released.

    If the GPIO pin type is not BGIO_PinType_eOpenDrain, this function will
    return error code BERR_INVALID_PARAMETER.

Input:
    hPin - The GPIO pin sub-module handle to perform pull down.

Output:

Returns:
    BERR_SUCCESS - Performs pull down successfully.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BGIO_Pin_GetValue,
    BGIO_Pin_SetType, BGIO_Pin_GetType,
    BGIO_Pin_OpenDrain_Release, BGIO_Pin_PushPull_SetValue.
****************************************************************************/
BERR_Code BGIO_Pin_OpenDrain_PullDown(
    BGIO_Pin_Handle       hPin );

BERR_Code BGIO_Pin_OpenDrain_PullDown_isr(
    BGIO_Pin_Handle       hPin );

/***************************************************************************
Summary:
    Perform release for a GPIO pin of type BGIO_PinType_eOpenDrain.

Description:
    If the AND-wired bus line is pulling down by this GPIO pin of type
    BGIO_PinType_eOpenDrain, this function releases it. If the GPIO pin is
    not pulling down the bus, this function does nothing.

    When a GPIO pin of open drain type is in the state of release, its
    value could be either BGIO_PinValue_e0 or BGIO_PinValue_e1, depending on
    whether there is another device that is pulling down the AND-wired bus
    line.

    After a GPIO pin sub-module is created, it is in release state by
    default.

    If the GPIO pin type is not BGIO_PinType_eOpenDrain, this function will
    return error code BERR_INVALID_PARAMETER.

Input:
    hPin - The GPIO pin sub-module handle to perform release.

Output:

Returns:
    BERR_SUCCESS - Performs pull down successfully.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BGIO_Pin_GetValue,
    BGIO_Pin_SetType, BGIO_Pin_GetType,
    BGIO_Pin_OpenDrain_PullDown, BGIO_Pin_PushPull_SetValue.
****************************************************************************/
BERR_Code BGIO_Pin_OpenDrain_Release(
    BGIO_Pin_Handle       hPin );

BERR_Code BGIO_Pin_OpenDrain_Release_isr(
    BGIO_Pin_Handle       hPin );

/***************************************************************************
Summary:
    Set the interrupt mode for the GPIO pin.

Description:
    This function sets the level 3 interrupt mode for a GPIO pin. The level
    3 interrupt mode is represented by enum BGIO_IntrMode. One of its value
    is BGIO_IntrMode_eDisabled, the others represent that the interrupt is
    enabled for this GPIO pin and in what condition the GPIO pin should
    generate interrupt.

    The default behavior is to have interrupt be disabled.

Input:
    hPin - Sub-module handle to set the interrupt mode for.
    eIntrMode - Interrupt generation mode.

Output:

Returns:
    BERR_SUCCESS - Set the interrupt mode successfully.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BGIO_Pin_GetIntrMode,
    BGIO_Pin_GetIntrStatus, BGIO_Pin_ClearIntrStatus
****************************************************************************/
BERR_Code BGIO_Pin_SetIntrMode(
    BGIO_Pin_Handle       hPin,
    BGIO_IntrMode         eIntrMode );

BERR_Code BGIO_Pin_SetIntrMode_isr(
    BGIO_Pin_Handle       hPin,
    BGIO_IntrMode         eIntrMode );

/***************************************************************************
Summary:
    Query for GPIO pin interrupt mode.

Description:
    This function returns the current level 3 interrupt mode of a GPIO pin.

Input:
    hPin - Sub-module handle to set the type for.

Output:
    peIntrMode - Returned interrupt mode.

Returns:
    BERR_SUCCESS - Got interrupt mode successfully.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BGIO_Pin_SetIntrMode,
    BGIO_Pin_GetIntrStatus, BGIO_Pin_ClearIntrStatus
****************************************************************************/
BERR_Code BGIO_Pin_GetIntrMode(
    BGIO_Pin_Handle       hPin,
    BGIO_IntrMode *       peIntrMode );

/***************************************************************************
Summary:
    Get the current level 3 interrupt status of a GPIO pin.

Description:
    If some GPIO pin fires interrupt, the level 2 interrupt call back
    function would be called. The call back function should call
    BGIO_Pin_GetIntrStatus for each GPIO pin handle to check the level 3
    interrupt status, and handle it if *pbFire is true for a GPIO pin. After
    it is handled the level 3 interrupt status should be cleared.

Input:
    hPin - The GPIO pin sub-module handle to get level 3 interrupt status for.

Output:
    pbFire - Whether the level 3 interrupt status of this GPIO pin is true.

Returns:
    BERR_SUCCESS - Got the current level 3 interrupt status of the GPIO pin
    successfully.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BGIO_Pin_SetIntrMode, BGIO_Pin_GetIntrMode,
    BGIO_Pin_ClearIntrStatus
****************************************************************************/
BERR_Code BGIO_Pin_GetIntrStatus(
    BGIO_Pin_Handle       hPin,
    bool *                bFire );

/***************************************************************************
Summary:
    Clear the level 3 interrupt status of a GPIO pin.

Description:
    When some GPIO pin fires interrupt, the level 2 interrupt call back
    function would be called. The call back function should check each
    GPIO pin handle for its level 3 interrupt status, handle it and then
    clear the level 3 interrupt status if the status is true for a GPIO pin.

    This function works only for edge triggering interrupt mode. For a GPIO
    pin with level triggering mode, user has to change the pin value to
    clear the interrupt status.

    Edge triggering interrupt mode is supported by both 7038 chips and 3560
    chips. Level triggering is only supported by 3560 chips.

Input:
    hPin - The GPIO pin sub-module handle to get value from.

Output:


Returns:
    BERR_SUCCESS - Got the current level 3 interrupt status of the GPIO pin
    successfully.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BGIO_Pin_GetIntrStatus
****************************************************************************/
BERR_Code BGIO_Pin_ClearIntrStatus(
    BGIO_Pin_Handle       hPin );

BERR_Code BGIO_Pin_ClearIntrStatus_isr(
    BGIO_Pin_Handle       hPin );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGIO_H__ */

/* end of file */
