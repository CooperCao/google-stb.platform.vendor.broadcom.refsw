/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
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
 *****************************************************************************/

#ifndef BHSM_MISC_H__
#define BHSM_MISC_H__



/* added to remove header file dependency chain*/
#include "bhsm_priv.h"
#include "bsp_s_misc.h"

#include "bhsm.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Module Specific Functions */

/**************************************************************************************************
Summary:

Description:
Structure that defines BHSM_ReadScArrayIO members

See Also:
BHSM_ReadScArray
**************************************************************************************************/
typedef struct BHSM_ReadScArrayIO {

    /* Out: 0 for success, otherwise failed. */
    unsigned int		unStatus;
    
    /* Out: This array represents the scrambling control bits (SCB) for each of the 256 
    possible pid channels (2 bits per pid channel).  
    SCB for Pid0 = bits[1:0] of unScArrayStatus[0].
    SCB for Pid15 = bits[31:30] of unScArrayStatus[0].
    SCB for Pid255 = bits[31:30] of unScArrayStatus[15]. 
    
    Translation of SCB:
        2'b11: scrambled with odd key
	    2'b10: scrambled with even key 
	    2'b00: no scrambling */
	uint32_t			unScArrayStatus[16];

} BHSM_ReadScArrayIO_t;

/**************************************************************************************************
Summary:
This function is to read out a snapshot copy of scrambling controlling bits for all PID channels.

Description:
This function reads out the current SCB values for all PID channels (256).  This function can
be called multiple times, each time returning the current SCB values for each PID channel.

Calling Context:
It can be called anytime during actual (de)scrambling is occurring for one or multiple data 
streams.

Performance and Timing:
This is a synchronous/blocking function that will not return until it completes.

Input:
in_handle - BHSM_Handle, Host Secure module handle.
inoutp_readScArrayIO - BHSM_ReadScArrayIO_t.
			
Output:
inoutp_readScArrayIO - BHSM_ReadScArrayIO_t.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
N/A
**************************************************************************************************/
BERR_Code   BHSM_ReadScArray (
		BHSM_Handle				in_handle,
		BHSM_ReadScArrayIO_t		*inoutp_readScArrayIO
);


#if (BCHP_CHIP==3563)
/**************************************************************************************************
Summary:
This function is to disable the forced-encryption-mode for RMX operation.

Description:
Just after the Aegis boot, RMX is enabled but, in forced encryption mode, so that clear data is not
allowed to go out on RMX right after system boot. Thus this function is needed by applications or
systems to disable "RMX encrytpion enforcement" at certain situation.

Calling Context:
It can be called anytime after system boot and HSM initialization are finished. 

Performance and Timing:
This is a synchronous/blocking function that will not return until it completes.

Input:
in_handle - BHSM_Handle, Host Secure module handle.

                        
Output:
none

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
N/A
**************************************************************************************************/
BERR_Code   BHSM_DisableRmxForcedEncyptionMode (
                BHSM_Handle                             in_handle
);

#endif

#if (BCHP_CHIP != 3563) && (BCHP_CHIP != 3548) && (BCHP_CHIP != 3556) && (BCHP_CHIP != 35130)
/**************************************************************************************************
Summary:

Description:
Structure that defines BHSM_SetMiscBitsIO members

See Also:
BHSM_SetMiscBits
**************************************************************************************************/
typedef struct BHSM_SetMiscBitsIO {

	/* In: value must be '0'. */
	BCMD_SetMiscBitsSubCmd_e	setMiscBitsSubCmd;
	
	/* In: value must be '1'. */
	bool 						bEnableWriteIMem;
	
	/* In: value must be '1'. */
	bool 						bEnableReadIMem;
	
	/* In: value must be '1'. */
	bool 						bEnableReadDMem;
	
	/* In: value must be '0'. */
    bool                   		 		bEnableEncBypass;
#if (BCHP_CHIP!=7400) ||  (BCHP_VER != BCHP_VER_A0)		
	/* In: When set, this will remove the RAVE from reset.  RAVE is placed in reset
	whenever code is written to RAVE IMEM. */
    bool                    				bDisableClear;
#endif    

/*#if defined (BCM7400A0) || defined(BCM7401B0)|| defined(BCM7401C0)||defined(BCM7118A0)*/
#if  (BCHP_CHIP== 7400 && BCHP_VER >= BCHP_VER_A0) || \
      (BCHP_CHIP== 7401 && BCHP_VER >= BCHP_VER_B0) ||  \
      (BCHP_CHIP== 7118 && BCHP_VER >= BCHP_VER_A0) ||  \
      (BCHP_CHIP== 7403 && BCHP_VER >= BCHP_VER_A0) ||  \
      (BCHP_CHIP== 7405 && BCHP_VER >= BCHP_VER_A0)  || \
      (BCHP_CHIP == 7325)||(BCHP_CHIP == 7335) || \
      (BCHP_CHIP == 7420) || (BCHP_CHIP == 7336)

	/* In: reserved */
	bool               bSetMiscReservedBit0;

	/* In: reserved */
	bool               bSetMiscReservedBit1;
	
	/* In: reserved */	
	bool               bSetMiscReservedBit2;
	
	/* In: reserved */
	bool               bSetMiscReservedBit3;
	
	/* In: reserved */	
	bool               bSetMiscReservedBit4;
	
	/* In: reserved */	
	bool               bSetMiscReservedBit5;
#endif

	/* Out: 0 for success, otherwise failed. */
	uint32_t				unStatus;

	/* Out: returns original value of RAVE bits before overwriting with new values. */	
	uint8_t 				ucRaveStatus;

} BHSM_SetMiscBitsIO_t;


/**************************************************************************************************
Summary:
This function is used to remove RAVE from reset.

Description:
This function is used to remove the RAVE from reset.  RAVE is placed in reset whenever the Host
CPU writes to RAVE IMEM.  When RAVE is removed from reset, it will begin executing code from
IMEM.  Video/Audio will not be descrambled until the RAVE is out of reset.

Calling Context:
This function is called after RAVE firmware has been loaded into IMEM and descrambling of 
video/audio is ready to begin.

Performance and Timing:
This is a synchronous/blocking function that will not return until it completes.

Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_setMiscBitsIO  - BHSM_SetMiscBitsIO_t.
			
Output:
inoutp_setMiscBitsIO  - BHSM_SetMiscBitsIO_t.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
N/A
**************************************************************************************************/
BERR_Code   BHSM_SetMiscBits (
		BHSM_Handle			in_handle,
		BHSM_SetMiscBitsIO_t		*inoutp_setMiscBitsIO
);

#endif

/* End of Module Specific Functions */
#if BHSM_SECURE_PI_SUPPORT_MISC
#include   "bhsm_misc_enc.h"
#endif


#ifdef __cplusplus
}
#endif

#endif /* BHSM_MISC_H__ */
