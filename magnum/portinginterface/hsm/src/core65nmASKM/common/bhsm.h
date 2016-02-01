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

/*================== Module Overview =====================================
<verbatim>

Module Overview



Usage



Sample Code


</verbatim>
***************************************************************************/
#ifndef BHSM_H__
#define BHSM_H__

#include "bchp.h"
#include "breg_mem.h"

/* These platforms have basic ASKM support */
#define HSM_IS_ASKM 1


/* added to remove header file dependency chain*/
#include "bhsm_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BHSM_ZEUS_VERSION_CALC(major,minor) (((major)<<16)|(minor))
#define BHSM_ZEUS_VERSION  (BHSM_ZEUS_VERSION_CALC(0,0))       /* Non Zeus chip */


/* ??? Cannot use EnterCriticalSection while submitting any commands */  /* ??? BSYT, get rid of this */

/***************************************************************************
Summary:
Host Secure Module (HSM) module context handle.

Description:
Opaque handle that is created in BHSM_Open.
BHSM_Handle holds the context of the Host Secure Module.  The system
should have only one BHSM_Handle. Caller of BHSM_Open is responsible to store 
this BHSM_Handle and uses it for the future function call after BHSM_Open function 
returns successfully.

See Also:
BHSM_Open, BHSM_Channel_Open

****************************************************************************/
typedef struct BHSM_P_Handle            *BHSM_Handle;


/***************************************************************************
Summary:
HSM channel context handle.

Description:
Opaque handle that is created in BHSM_Channel_Open.
BHSM_ChannelHandle holds the context of the Host Secure Module channel.  The system
may have more than one BHSM_ChannelHandle if the chip can support multiple 
interfaces to submit Aegis commands.  Caller of BHSM_Channel_Open is 
responsible to store this BHSM_ChannelHandle and uses it for the future function 
call after BHSM_Channel_Open function returns successfully.

See Also:
BHSM_Open, BHSM_Channel_Open

****************************************************************************/
typedef struct BHSM_P_ChannelHandle     *BHSM_ChannelHandle;






/* Basic Module Functions */


/*****************************************************************************
Summary:
This function returns a recommended default settings for HSM module.

Description:
This function shall return a recommended default settings for HSM module.
This function shall be called before BHSM_Open
and the caller can then over-ride any of the default settings 
required for the build and configuration by calling BHSM_Open.

These default settings are always the same regardless of how 
many times this function is called or what other functions have 
been called in the porting interface. 


Calling Context:
The function shall be called from application level (for example in 
VxWorks or no-os) or from driver level (for example in Linux,
recommended ioctl: BHSM_IOCTL_GET_DEFAULT_SETTINGS)

Performance and Timing:
This is a synchronous function that will return when it is done.

Input:
in_chipHandle - BCHP_Handle,  chip handle.

Output:
outp_sSettings - BHSM_Settings, a ref/pointer to the default setting. 

Returns:
BERR_SUCCESS - success. Otherwise, there is an error.

See Also:
BHSM_Open 
BHSM_UpdateDefaultSettings

*****************************************************************************/
BERR_Code BHSM_GetDefaultSettings(
		BHSM_Settings		*outp_sSettings,
		BCHP_Handle		in_chipHandle		
); 


/*****************************************************************************
Summary:
This function updates the HSM driver special controls, e.g. HSM run in ISR mode or polling mode.

Description:
The default setting is using ISR inside HSM driver when receiving data from BSP. This function is provided to
select polling mode, or toggle ISR/Polling mode.   It can also be used to modify the timeout value for polling 
or ISR (the default is 2000 ms). It can only be invoked from non-critical calling code section.

Calling Context:
Any time it can be called after BHSM initialization.

Performance and Timing:
This will return immediately.

Input:
netSettings- BHSM_NewSettings_t, select what to be updated and provide their corresponding values in this
                  structured data                  
                  whichControl - BHSM_SpecialCtrol_Flag_e,  select what BHSM controls to update, one or multipe. 
                  control bits shall not be overlapped. 1 to update the corresponding control. 0 for no update of it.
		     ctrlValue - unsigned long,  what value is set for this control field. Bit0 is for ISR or Polling selection. 
		     Other bits RFU.		    
		     timeoutMs	- unsigned long,  new timeout value for polling or wairt-for-ISR-from-BSP

Output:
outp_sSettings - BHSM_Settings, a ref/pointer to the updated setting. 

Returns:
BERR_SUCCESS - success. Otherwise, there is an error.

See Also:
BHSM_GetDefaultSettings

*****************************************************************************/
BERR_Code BHSM_SetSettings(
		BHSM_Handle				   hHsm,
	  	BHSM_NewSettings_t		* netSettings

);


/*****************************************************************************
Summary:
This function creates the Host Secure Module handle.

Description:
This function shall create the Host Secure module handle.
It also initializes the Host Secure module 
and hardware using settings stored in the outp_sSettings pointer.
All the associated channels are not ready to be access until
BHSM_ChannelOpen is called and returns successfully.
 
The caller can pass a NULL pointer for inp_sSettings. If the 
p_Settings pointer is NULL, default settings should be used.

It is the caller responsibility to store the outp_handle and uses
it for the future function call after this function returns
successfully.

Before calling this function, the only function that the caller 
can call is BHSM_GetDefaultSettings. System shall not call
any other Host Secure functions prior to this function.

System shall not call this function more than once without calling BHSM_Close
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
in_regHandle  - BREG_Handle, use to access Host Secure register.
in_chipHandle - BCHP_Handle,  Chip handle.
in_interruptHandle - BINT_Handle, Interrupt handle to handle Host Secure interrupt.
in_xptHandle -	BXPT_Handle, Transport handle.
inp_sSettings - BHSM_Settings, the settings that apply to multiple
				channels.  If NULL, a default setting shall be used.
			
Output:
outp_handle - BHSM_Handle, a ref/pointer to the Host Secure module handle. 

Returns:
BERR_SUCCESS - success

See Also:
BHSM_GetDefaultSettings 
BHSM_Close

******************************************************************************/
BERR_Code BHSM_Open(
		BHSM_Handle			*outp_handle, 
		BREG_Handle			in_regHandle,     
		BCHP_Handle			in_chipHandle,
		BINT_Handle			in_interruptHandle,
		const BHSM_Settings	*inp_sSettings
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
the allocated resources and inout_handle shall be NULL.

Other than BHSM_GetDefaultSettings, system shall not call any other HSM 
functions after this function returns, regardless of the return result.


Calling Context:
The function shall be called from application level (for example in 
VxWorks or no-os) or from driver level (for example in Linux,
during rmmod)

Performance and Timing:
This is a synchronous function that will return when it is done.

Input:
in_handle  - BHSM_Handle, Host Secure module handle.

Returns:
BERR_SUCCESS - success

See Also:
BHSM_Open 
BHSM_GetDefaultSettings

******************************************************************************/
BERR_Code BHSM_Close(
		BHSM_Handle in_handle
);


/*****************************************************************************
Summary:
This function returns the number of Host Secure Module channels 
that we can support for the build and configuration.

Description:
This function returns the number of Aegis command interfaces 
that we can support for the build and configuration.

The caller shall then call BHSM_Channel_Open or BHSM_GetChannelDefaultSettings
with in_channelNo that smaller than outp_totalChannels.

Currently we support 2 Hardware Aegis Command interfaces.

Calling Context:
The function shall be called from application level (for example in 
VxWorks or no-os) or from driver level (for example in Linux,
recommended ioctl: BHSM_IOCTL_GET_TOTAL_CHANNELS or device open)

Performance and Timing:
This is a synchronous function that will return when it is done.

Input:
in_handle  - BHSM_Handle, a ref/pointer to the Host Secure module handle.
			
Output:
outp_ucTotalChannels - unsigned char, a ref/pointer to number of supported 
					Aegis command interfaces

Returns:
BERR_SUCCESS - success


See Also:
BHSM_GetChannel

******************************************************************************/
BERR_Code BHSM_GetTotalChannels(
		BHSM_Handle		in_handle, 
		unsigned char		*outp_ucTotalChannels
);


/*****************************************************************************
Summary:
This function returns a recommended default settings for HSM channel. 

Description:
This function shall return a recommended default settings for HSM channel. 
This function shall be called before BHSM_Channel_Open
and the caller can then over-ride any of the default settings 
required for the build and configuration by calling BHSM_Channel_Open.

The caller shall pass in_channelNo that is smaller than outp_totalChannels in
BHSM_GetTotalChannels. The in_channelNo for the first channel shall be zero.

These default settings are always the same regardless of how 
many times this function is called or what other functions have 
been called in the porting interface. 

Calling Context:
The function shall be called from application level (for example in 
VxWorks or no-os) or from driver level (for example in Linux,
recommended ioctl: BHSM_IOCTL_GET_CHANNEL_DEFAULT_SETTINGS)

Performance and Timing:
This is a synchronous function that will return when it is done.

Input:
in_handle - BHSM_Handle,  Host Secure module handle.
in_channelNo  - BHSM_HwModule, an index that indicates which channel or Aegis
				command interface that the caller want to access.
Output:
outp_sSettings - BHSM__ChannelSettings, a ref/pointer to the default channel
					setting. 

Returns:
BERR_SUCCESS - success

See Also:
BHSM_Open
BHSM_GetTotalChannels
BHSM_Channel_Open

*****************************************************************************/
BERR_Code BHSM_GetChannelDefaultSettings(
		BHSM_Handle				in_handle, 
		BHSM_HwModule			in_channelNo, 
		BHSM_ChannelSettings		*outp_sSettings
);



/*****************************************************************************
Summary:
This function creates the Host Secure channel handle.

Description:
This function shall create the Host Secure channel handle.
It also initializes the specified Host Secure interface and all the associated registers 
using settings stored in the inp_channelDefSettings pointer.

The caller shall pass in_channelNo that is smaller than outp_totalChannels in
BHSM_GetTotalChannels. The in_channelNo for the first channel shall be zero.

It is the responsibility of caller to specify the correct in_channelNo for 
different Aegis command interface.  

The caller can pass a NULL pointer for inp_channelDefSettings. If the 
inp_channelDefSettings pointer is NULL, default settings should be used.

It is the caller responsibility to store the outp_channelHandle and uses
it for the future function call after this function returns
successfully.

Before calling this function, the only channel related functions that the system 
can call are BHSM_GetTotalChannels and BHSM_GetChannelDefaultSettings.
System shall not call any other channel related functions prior to this function.

System shall not call this function more than once without calling BHSM_Channel_Close
previously. 

If illegal settings are passed in an error should be 
returned and the hardware state should not be modified. 


Calling Context:
The function shall be called from application level (for example in 
VxWorks or no-os) or from driver level (for example in Linux,
recommended ioctl: BHSM_IOCTL_CHANNEL_OPEN

Performance and Timing:
This is a synchronous function that will return when it is done.

Input:
in_handle  - BHSM__Handle, Host Secure module handle.
in_channelNo  - BHSM_HwModule, an index that indicates which channel 
				that the caller want to access.       
inp_channelDefSettings - BHSM_ChannelSettings, the channel settings that apply to this 
					specific channel.  If NULL, a default channel setting shall be used.
			
Output:
outp_channelHandle -BHSM_ChannelHandle, a ref/pointer to the host secure module. 

Returns:
BERR_SUCCESS - success

See Also:
BHSM_Open
BHSM_GetTotalChannels
BHSM_Channel_Close


*****************************************************************************/
BERR_Code BHSM_Channel_Open(
		BHSM_Handle                       	in_handle, 
		BHSM_ChannelHandle           	*outp_channelHandle, 
		BHSM_HwModule				in_channelNo, 
		const BHSM_ChannelSettings 	*inp_channelDefSettings 
);


/*****************************************************************************
Summary:
This function frees the channel handle and any resources contained 
in the channel handle.

Description:
This function shall free the channel handle and any resources contained 
in the channel handle.

This function always returns successfully even if some of the resources cannot 
be freed successfully.

Other than BHSM_GetTotalChannels and BHSM_GetChannelDefaultSettings, system 
shall not call any other channel related functions after this function returns, 
regardless of the return result.
  
Calling Context:
The function shall be called from application level (for example in 
VxWorks or no-os) or from driver level (for example in Linux,
recommended ioctl: BHSM_IOCTL_CHANNEL_CLOSE)

Performance and Timing:
This is a synchronous function that will return when it is done.

Input:
in_channelHandle  - BHSM_ChannelHandle,  host secure module channel handle.

Returns:
BERR_SUCCESS - success

See Also:
BHSM_Channel_Open

******************************************************************************/
BERR_Code BHSM_Channel_Close(
		BHSM_ChannelHandle in_channelHandle
);


/*****************************************************************************
Summary:
This function gets the module handle based on the channel handle. 

Description:
This function shall get the module handle based on the channel handle. 

Calling Context:
The function shall be called from application level (for example in 
VxWorks or no-os) or from driver level (for example in Linux,
recommended ioctl: BHSM_IOCTL_GET_DEVICE)

Performance and Timing:
This is a synchronous function that will return when it is done.

Input:
in_channelHandle  - BHSM_ChannelHandle, host secure module channel handle.
			
Output:
outp_handle - BHSM_Handle, a ref/pointer to the host secure module handle. 

Returns:
BERR_SUCCESS - success

See Also:
BHSM_Open
BHSM_Channel_Open

*****************************************************************************/
BERR_Code BHSM_GetDevice(
		BHSM_ChannelHandle           in_channelHandle, 
		BHSM_Handle                       *outp_handle 
);


/*****************************************************************************
Summary:
This function returns the specific Host Secure Module channel handle.

Description:
This function returns the specific Host Secure module channel handle.
The caller shall pass in_index that is smaller than outp_totalChannels in
BHSM_GetTotalChannels. The in_index for the first channel 
shall be zero and the second channel shall be 1.
It is the caller responsibility to store the return outp_channelHandle
for the future function calls after this function returns successfully. 

Before calling this function, the only functions that the system 
can call are BHSM_GetDefaultSettings, BHSM_Open, BHSM_GetTotalChannels,
and BHSM_Close.

  
Calling Context:
The function shall be called from application level (for example in 
VxWorks or no-os) or from driver level (for example in Linux,
recommended ioctl: BHSM_IOCTL_GET_CHANNEL )

Performance and Timing:
This is a synchronous function that will return when it is done.

Input:
in_handle  - BHSM_Handle, a ref/pointer to the Host Secure module handle.
in_channelNo -  BHSM_HwModule, an index that indicate which channel or host secure module
			slot that the caller want to access.
			
Output:
outp_channelHandle - BHSM_ChannelHandle, a ref/pointer to the Host Secure 
					 channel handle. 

Returns:
BERR_SUCCESS - success


See Also:
BHSM_GetTotalChannels

******************************************************************************/
BERR_Code BHSM_GetChannel(
		BHSM_Handle			in_handle, 
		BHSM_HwModule		in_channelNo, 
		BHSM_ChannelHandle	*outp_channelHandle
);

/* End of Basic Module Functions */

/* Module Specific Functions */



/*****************************************************************************
Summary:

This function is reserved for Broadcom internal usage only. 

******************************************************************************/
BERR_Code BHSM_SetIntrCallback(
		BHSM_Handle			in_handle,
		BHSM_IntrType		in_eIntType,
		BHSM_IsrCallbackFunc 	in_callback		
);




#ifdef BHSM_AUTO_TEST
/*****************************************************************************
Summary:

This function is reserved for Broadcom internal usage only. 
 
******************************************************************************/
BERR_Code BHSM_SubmitRawCommand (
		BHSM_Handle 		in_handle,
		BHSM_HwModule	in_interface,
		uint32_t			in_unInputParamLenInWord,
		uint32_t			*inp_unInputParamsBuf,
		uint32_t			*outp_unOutputParamLenInWord,
		uint32_t			*outp_unOutputParamsBuf
);
#endif

/* End of Module Specific Functions */


#ifdef __cplusplus
}
#endif


#endif /* BHSM_H__ */
