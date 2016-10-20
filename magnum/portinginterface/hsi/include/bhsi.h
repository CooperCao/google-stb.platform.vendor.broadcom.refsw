/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/

#ifndef BHSI_H__
#define BHSI_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"


#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
Host Sage Interface (HSI) module context handle.

Description:
Opaque handle that is created in BHSI_Open.
BHSI_Handle holds the context of the Host Sage Interface.  The system
should have only one BHSI_Handle. Caller of BHSI_Open is responsible to store
this BHSI_Handle and uses it for the future function call after BHSI_Open function 
returns successfully.

See Also:
BHSI_Open

****************************************************************************/
typedef struct BHSI_P_Handle            *BHSI_Handle;

/* context is the opaque .responseCallbackContext value given in BHSI_Setting */
typedef void (*BHSI_IsrCallbackFunc)(void *context);

/* context is the opaque .flushCallbackContext value given in BHSI_Setting */
typedef void (*BHSI_FlushCallbackFunc)(const void *address, size_t size);
typedef BHSI_FlushCallbackFunc BHSI_InvalidateCallbackFunc;

/* Note:
 * SAGE / Host communication uses two channels:
 *     Forward channel uses two buffers
 *         Host->SAGE: Request buffer
 *         Host<-SAGE: Request ACK buffer
 *     Reverse channel uses two buffers
 *         Host<-SAGE: Response buffer
 *         Host<-SAGE: Response ACK buffer
 */
typedef struct BHSI_Setting
{
    unsigned long timeOutMsec;      /* Timeout for Send Operations */ 

    /* Forward (request) Channel; ACK is optional
         Host --> request      --> SAGE
         Host <-- request Ack  <-- SAGE */
    uint8_t       *requestBuf;   /* buffer virtual address */
    uint32_t      requestBufLen; /* length of the buffer */
    uint8_t       *requestAckBuf;
    uint32_t      requestAckBufLen;

    /* Reverse (response) Channel; ACK is optional
         Host <-- response     <-- SAGE
         Host --> response Ack --> SAGE */
    uint8_t       *responseBuf;
    uint32_t      responseBufLen;
    uint8_t       *responseAckBuf;
    uint32_t      responseAckBufLen;

    /* receive callback is used when a message is received on the response buffer */
    BHSI_IsrCallbackFunc responseCallback_isr;/* Receive callback. Fired under interrupt. */
    void *responseCallbackContext;        /* Receive callback opaque context */

    /* flush callback fired any time right before a communication buffer ownership goes to SAGE.
       This parameter is optionnal, only for cached addresses
       however it is recommanded to use cached addresses */
    BHSI_FlushCallbackFunc flushCallback_isrsafe;/* Flush cache callback. Could be Fired under Interrupt.*/

    /* invalidate callback fired any time right before a reading a buffer from SAGE.
       This parameter is optionnal, only for cached addresses
       however it is recommanded to use cached addresses */
    BHSI_FlushCallbackFunc invalidateCallback_isrsafe;/* Invalidate cache callback. Could be Fired under Interrupt */

} BHSI_Settings;

#define BHSI_HEAD_SIZE (sizeof(uint32_t))/* size of header added by HSI */


/* Basic Module Functions */


/*****************************************************************************
Summary:
This function returns a recommended default settings for HSI module.

Description:
This function shall return a recommended default settings for HSI module.
This function shall be called before BHSI_Open
and the caller can then over-ride any of the default settings
required for the build and configuration by calling BHSI_Open.

These default settings are always the same regardless of how
many times this function is called or what other functions have
been called in the porting interface.


Calling Context:
The function shall be called from application level (for example in
VxWorks or no-os) or from driver level (for example in Linux,
recommended ioctl: BHSI_IOCTL_GET_DEFAULT_SETTINGS)

Performance and Timing:
This is a synchronous function that will return when it is done.

Output:
outp_sSettings - BHSI_Settings, a ref/pointer to the default setting.

Returns:
BERR_SUCCESS - success. Otherwise, there is an error.

See Also:
BHSI_Open

*****************************************************************************/
BERR_Code BHSI_GetDefaultSettings(
    BHSI_Settings *settings
);


/*****************************************************************************
Summary:
This function creates the Host Sage Interface Module handle.

Description:
This function shall create the Host Sage Interface Module handle.
It also initializes the Host Sage Interface Module
and hardware using settings stored in the pSettings pointer.

The caller can pass a NULL pointer for pSettings. If the
pSettings pointer is NULL, default settings should be used.

It is the caller responsibility to store the hHsi and uses
it for the future function call after this function returns
successfully.

Before calling this function, the only function that the caller
can call is BHSI_GetDefaultSettings. System shall not call
any other Host Secure functions prior to this function.

System shall not call this function more than once without calling BHSI_Close
previously.

If illegal settings are passed in an error should be
returned and the hardware state should not be modified.

The BINT_Handle is only required if this module needs to
associate ISR callback routines with L2 interrupts.


Calling Context:
The function shall be called from application level (for example in
VxWorks or no-os) or from driver level (for example in Linux,
during insmod )

Performance and Timing:
This is a synchronous function that will return when it is done.

Input:
hRegister  - BREG_Handle, use to access Host Secure register.
hInt - BINT_Handle, Interrupt handle to handle Host Secure interrupt.
pSettings - BHSI_Settings, the settings that apply to multiple
                channels.  If NULL, a default setting shall be used.

Output:
hHsi - BHSI_Handle, a ref/pointer to the Host Sage Interface Module handle.

Returns:
BERR_SUCCESS - success

See Also:
BHSI_GetDefaultSettings
BHSI_Close

******************************************************************************/
BERR_Code BHSI_Open(
    BHSI_Handle *hHsi, /* [out] */
    BREG_Handle hRegister,
    BINT_Handle hInt,
    const BHSI_Settings *pSettings
);


/*****************************************************************************
Summary:
This function frees the main handle and any resources contained
in the main handle.

Description:
This function shall free the main handle and any resources contained
in the main handle. This function shall try to free any resources associated
with sub handles created from the main handle. However, this function does not
free any resources associated with channel handle.

Regardless of the return value, this function always attempts to free all
the allocated resources and hHsi shall be NULL.

Other than BHSI_GetDefaultSettings, system shall not call any other HSI
functions after this function returns, regardless of the return result.


Calling Context:
The function shall be called from application level (for example in
VxWorks or no-os) or from driver level (for example in Linux,
during rmmod)

Performance and Timing:
This is a synchronous function that will return when it is done.

Input:
in_handle  - BHSI_Handle, Host Sage Interface Module handle.

Returns:
BERR_SUCCESS - success

See Also:
BHSI_Open
BHSI_GetDefaultSettings

******************************************************************************/
BERR_Code BHSI_Close(
    BHSI_Handle hHsi
);


/* End of Basic Module Functions */

/* Module Specific Functions */


BERR_Code BHSI_Send(
    BHSI_Handle hHsi,
    const uint8_t*sendBuf,
    uint32_t sendBufLen,
    uint8_t *ackBuf,
    uint32_t maxAckBufLen,
    uint32_t *ackBufLen
);

BERR_Code BHSI_Receive_isrsafe(
    BHSI_Handle hHsi,
    uint8_t *rcvBuf,
    uint32_t maxRcvBufLen,
    uint32_t *rcvBufLen
);

BERR_Code BHSI_Ack_isrsafe(
    BHSI_Handle hHsi,
    const uint8_t *ackBuf,
    uint32_t ackBufLen
);

/* must be called to reset HSI instance after a watchdog event */
BERR_Code BHSI_Reset_isr(
    BHSI_Handle hHsi
);

/* End of Module Specific Functions */


#ifdef __cplusplus
}
#endif


#endif /* BHSI_H__ */
