/***************************************************************************
 *  Copyright (C) 2003-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/*= Module Overview ********************************************************
This module supplies the function required to access memory mapped registers.
Before any registers can be accessed an appropriate register handle should
be created (normally done at system init time).  This register handle is
then given to any modules that require access to the corresponding memory
mapped registers.
***************************************************************************/

#ifndef BREG_MEM_H
#define BREG_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

/*
Summary:
This is an opaque handle that is used for memory mapped register functions.
*/
typedef struct BREG_Impl *BREG_Handle;

typedef struct BREG_OpenSettings {
    void *callbackContext;
    bool (*isRegisterAtomic_isrsafe)(void *, uint32_t reg); /* this function called to verify that proper function used to access atomic registers */
    void (*systemUpdate32_isrsafe)(void *, uint32_t reg, uint32_t mask, uint32_t value, bool atomic); /* this function called for each BREG_AtomicUpdate32 or BREG_Update32 call*/
    void (*runSerialized_isrsafe)(void *, void (*action)(BREG_Handle, void *), void *action_context);  /* this function called to execute 'action' serialized with any other calls to the same function */
    unsigned msatChannel; /* static HIF_MSAT channel used for serialized accesses */
} BREG_OpenSettings;

void BREG_GetDefaultOpenSettings(
    BREG_OpenSettings *pSettings
    );


/*
Summary:
This function creates a register handle for memory mapped registers.

Description:
This function must be called before any memory mapped registers can be
accessed.  This normally is done by platform specific code during init
time.  The resulting register handle is then passed to any modules that
must access the corresponding registers.
*/
BERR_Code BREG_Open(
                          BREG_Handle *pRegHandle, /* [out] Returns a register handle */
                          void *Address, /* Base address of the memory mapped register range */
                          size_t MaxRegOffset, /* Maximum offset for this memory mapped register range */
                          const BREG_OpenSettings *pSettings
                          );

/*
Summary:
This function destroys a previous created register handle.

Description:
This function frees any resources associate with a memory mapped register
handle.  After this function is called the register handle can no longer
be used for subsequent register accesses.
*/
void BREG_Close(
                           BREG_Handle RegHandle /* Register handle created by BREG_CreateRegHandle() */
                           );

/*
Summary:
This function writes 64 bits to a 64-bit register.

Description:
Although this function will never return an error it will assert if the
RegHandle is invalid or during a debug build if the reg offset is larger
than the MaxRegOffset specified in the BREG_CreateRegHandle function.
*/
void BREG_Write64_isrsafe(
                  BREG_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
                  uint32_t reg, /* Register offset to write */
                  uint64_t data /* Data value to write to register */
                  );

/*
Summary:
This function reads 64 bits from a register.

Description:
Although this function cannot return an error it will assert if the
RegHandle is invalid or during a debug build if the reg offset is larger
than the MaxRegOffset specified in the BREG_CreateRegHandle function.
*/
uint64_t BREG_Read64_isrsafe(
                 BREG_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
                 uint32_t reg /* Register offset to read */
                 );

#define BREG_Write64(handle, reg, data) BREG_Write64_isrsafe(handle, reg, data)
#define BREG_Read64(handle, reg) BREG_Read64_isrsafe(handle, reg)

#if BDBG_DEBUG_BUILD != 1

#include "breg_mem_opt.h"   /* Contains optimized versions of these routines */

#else

/*
Summary:
This function writes 32 bits to a register.

Description:
Although this function will never return an error it will assert if the
RegHandle is invalid or during a debug build if the reg offset is larger
than the MaxRegOffset specified in the BREG_CreateRegHandle function.
*/
void BREG_Write32(
                  BREG_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
                  uint32_t reg, /* Register offset to write */
                  uint32_t data /* Data value to write to register */
                  );

/*
Summary:
This function writes 16 bits to a register.

Description:
Although this function will never return an error it will assert if the
RegHandle is invalid or during a debug build if the reg offset is larger
than the MaxRegOffset specified in the BREG_CreateRegHandle function.
*/
void BREG_Write16(
                  BREG_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
                  uint32_t reg, /* Register offset to write */
                  uint16_t data /* Data value to write to register */
                  );

/*
Summary:
This function writes 8 bits to a register.

Description:
Although this function will never return an error it will assert if the
RegHandle is invalid or during a debug build if the reg offset is larger
than the MaxRegOffset specified in the BREG_CreateRegHandle function.
*/
void BREG_Write8(
                  BREG_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
                  uint32_t reg, /* Register offset to write */
                 uint8_t data /* Data value to write to register */
                 );

/*
Summary:
This function reads 32 bits from a register.

Description:
Although this function cannot return an error it will assert if the
RegHandle is invalid or during a debug build if the reg offset is larger
than the MaxRegOffset specified in the BREG_CreateRegHandle function.
*/
uint32_t BREG_Read32(
                 BREG_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
                 uint32_t reg /* Register offset to read */
                 );

/*
Summary:
This function reads 16 bits from a register.

Description:
Although this function cannot return an error it will assert if the
RegHandle is invalid or during a debug build if the reg offset is larger
than the MaxRegOffset specified in the BREG_CreateRegHandle function.
*/
uint16_t BREG_Read16(
                 BREG_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
                 uint32_t reg /* Register offset to read */
                 );

/*
Summary:
This function reads 8 bits from a register.

Description:
Although this function cannot return an error it will assert if the
RegHandle is invalid or during a debug build if the reg offset is larger
than the MaxRegOffset specified in the BREG_CreateRegHandle function.
*/
uint8_t BREG_Read8(
                 BREG_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
                 uint32_t reg /* Register offset to read */
                 );

#endif /* BDBG_DEBUG_BUILD != 1 */


/*
Summary:
This function updates a 32 bit register value.

Description:
This function updates 32 bit value stored in the hardware register.
It's equivalent to the following code:

   BREG_Write32(regHandle, reg, (BREG_Read32(regHandle, reg) & (~mask)) | value);

with the added value that the read/modify/write may be performed atomically system wide (across all magnum modules and beyond).

This function would internally use external interface to decide on type of update:
* non atomic update (just read followed by write)
* atomic update across all magnum modules
* system wide atomic update

This function could be used simultaneously with BREG_Write32 (for updating not atomic registers) and BREG_AtomicUpdate32 (for updating atomic registers)

Skipping accesses:
* if mask equal to 0xFFFFFFFF, then register read will be skipped
*/
void BREG_Update32_isrsafe(
                  BREG_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
                  uint32_t reg, /* Register offset to write */
                  uint32_t mask,
                  uint32_t value
                  );

#define BREG_Update32       BREG_Update32_isrsafe
#define BREG_Update32_isr   BREG_Update32_isrsafe

/***************************************************************************
Summary:
    Set register bitfield value using RDB name.

Description:
    This macro sets a register bitfield value using an RDB name.
    It intends to hide away the mechanics of
    ANDing/SHIFTing/ORing/MASKing to set the value.

Input:
    hRegister - instance of BREG_Handle

    Register - Register instance name defined in RDB. These names are
    available in html documentations, and the RDB generated header files.
    Note this a  register instance name; not a regtype.

    Field - Field name defined in RDB. The "Field" will be set to zero in
    "Register" before "FieldValue" is ORed in. These names are available in
    html documentations, and the RDB generated header files.

    FieldValue - This is the value that the register field is to be set
    to. The FieldValue will SHIFTed and MASKed before being ORed into
    "Memory"

Returns:
    <None>.

Example:
    BREG_SET_FIELD_DATA( hRegister, V3D_PLL_CTRL_PLL_6CH_CTRL_0, POST_RESETB, 0 );

See Also:
    BREG_SET_FIELD_ENUM.
**************************************************************************/
#define BREG_SET_FIELD_DATA(hRegister,Register,Field,FieldValue) BREG_Update32_isrsafe(hRegister, BCHP_##Register, BCHP_MASK(Register,Field), BCHP_FIELD_DATA(Register,Field,FieldValue))
#define BREG_SET_FIELD_CONST_DATA(hRegister,Register,Field,FieldValue) BREG_Update32_isrsafe(hRegister, BCHP_##Register, BCHP_MASK(Register,Field), BCHP_FIELD_CONST_DATA(Register,Field,FieldValue))

/***************************************************************************
Summary:
    Push a register bitfield value using RDB name.

Description:
    This macro sets a register bitfield value using an RDB name.
    It intends to hide away the mechanics of
    ANDing/SHIFTing/ORing/MASKing to set the value.

Input:
    hRegister - instance of BREG_Handle

    Register - Register instance name defined in RDB. These names are
    available in html documentations, and the RDB generated header files.
    Note this a  register instance name; not a regtype.

    Field - Field name defined in RDB. The "Field" will be set to zero in
    "Memory" before "FieldValue" os ORed in. These names are available in
    html documentations, and the RDB generated header files.

    Name - A value name that get push into the regiser bitfield.
    This name is available in html documentations (italicized) in the
    bitfield definition.


Returns:
    <None>.

Example:
	BCHP_SET_FIELD_ENUM( hRegister, VDEC_FE_0_SYNC_DETECT, HEAD_SWITCH_SEL, FOUR_HEAD );

See Also:
	BCHP_SET_FIELD_DATA
**************************************************************************/
#define BREG_SET_FIELD_ENUM(hRegister,Register,Field,Name) BREG_Update32_isrsafe(hRegister, BCHP_##Register, BCHP_MASK(Register,Field), BCHP_FIELD_ENUM(Register,Field,Name))

/*
Summary:
This function atomically updates a 32 bit register value.

Description:
See BREG_AtomicUpdate32_isrsafe.
*/
void BREG_AtomicUpdate32_isrsafe(
                  BREG_Handle RegHandle, /* Register handle created by BREG_CreateRegHandle() */
                  uint32_t reg, /* Register offset to write */
                  uint32_t mask,
                  uint32_t value
                  );

#define BREG_AtomicUpdate32_isr BREG_AtomicUpdate32_isrsafe
#define BREG_AtomicUpdate32 BREG_AtomicUpdate32_isrsafe

/* ISR() flavors go here so they are picked up for both debug and release builds */

/*
Summary:
This function writes 16 bits to a register.  Used for _isr context.
*/
#define BREG_Write16_isr BREG_Write16

/*
Summary:
This function writes 8 bits to a register.  Used for _isr context.
*/
#define BREG_Write8_isr BREG_Write8

/*
Summary:
This function writes 32 bits to a register.  Used for _isr context.
*/
#define BREG_Write32_isr BREG_Write32

/*
Summary:
This function reads 8 bits from a register.  Used for _isr context.
*/
#define BREG_Read8_isr BREG_Read8

/*
Summary:
This function reads 16 bits from a register.  Used for _isr context.
*/
#define BREG_Read16_isr BREG_Read16

/*
Summary:
This function reads 32 bits from a register.  Used for _isr context.
*/
#define BREG_Read32_isr BREG_Read32

/*
Summary:
This function writes device offset
*/
#define BREG_WriteAddr_isrsafe BREG_Write64_isrsafe

/*
Summary:
This function writes device offset
*/
#define BREG_ReadAddr_isrsafe BREG_Read64_isrsafe

/*
Summary:
This function writes device offset
*/
#define BREG_WriteAddr BREG_Write64

/*
Summary:
This function writes device offset
*/
#define BREG_ReadAddr BREG_Read64

/* Internal representation of the BREG handle */
BDBG_OBJECT_ID_DECLARE(BREG);
typedef struct BREG_Impl
{
    BDBG_OBJECT(BREG)
    uint8_t *BaseAddr; /* BaseAddr shall be the first member to keep it run-time compatible with the release builds */
    size_t MaxRegOffset;
    BREG_OpenSettings openSettings;
} BREG_Impl;

#ifdef __cplusplus
}
#endif

#include "breg_tracelog.h"

#endif
/* End of File */
