/***************************************************************************
 * Copyright (C) 2004-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * This module controls and returns the User Data coming in the stream
 * and captured by the decoder.
 *
 ***************************************************************************/
#ifndef BXVD_USERDATA_H__
#define BXVD_USERDATA_H__

#include "bfmt.h"		   /* Video timing format */
#include "bavc.h"		   /* Analog Video Common */


#ifdef __cplusplus
extern "C" {
#endif

/* SEI message T35 */
#define BXVD_USERDATA_H264_TYPE_REGISTERED           4
#define BXVD_USERDATA_H264_TYPE_UNREGISTERED         5
#define BXVD_USERDATA_H264_TYPE_FRAME_PACK          45

/***************************************************************************
Summary: 
	User Data Handle

Description: 
	An opaque handle for each channel
****************************************************************************/
typedef struct BXVD_P_UserDataContext *BXVD_Userdata_Handle;

/***************************************************************************
Summary: 
	User Data default settings structure

Description:
	Contains the maximum userdata buffer size and any other default settings as
 required. The maximum userdata default size is 4K.

****************************************************************************/
typedef struct BXVD_Userdata_Settings
{
      int maxDataSize;      /* Default is 4K */
      int maxQueueDepth;    /* Default is 64 entries */
      int maxQueueItemSize; /* Default is 512 bytes */
} BXVD_Userdata_Settings;

/***************************************************************************
Summary: 
	Returns default User data settings

Returns:
	BERR_SUCCESS - If function is successful.

See Also: 
	BXVD_Userdata_Open
	
****************************************************************************/
BERR_Code BXVD_Userdata_GetDefaultSettings (
	BXVD_Userdata_Settings *pDefSettings    /* [out] default User Data settings */
);

/***************************************************************************
Summary: 
	Opens user data channel

Description: 
	This function initializes data structures for collecting user
	data present in the stream. In MPEG stream user data can come
	after sequence header, after gop header and after picture 
	header. This function initializes user data handle for the
	channel passed. This handle should be used to make subsequent
	calls to other user data APIs.

Returns:
	BERR_SUCCESS - If function is successful.

See Also: 
	BXVD_Userdata_CloseUserData
	
****************************************************************************/
BERR_Code BXVD_Userdata_Open(
	BXVD_ChannelHandle            hXVDCh,      /* [in] XVD Channel handle */
	BXVD_Userdata_Handle         *phUserData,  /* [out] User data handle */
	const BXVD_Userdata_Settings *pDefSettings /* [in] default User Data settings */
);

/***************************************************************************
Summary: 
	Closes user data channel

Description: 
	This function closes the user data channel. It frees any 
	resources acquired by the channel and disables User Data extraction .

Returns:
	BERR_SUCCESS - If function is successful.

See Also: 
	BXVD_Userdata_Open
	
****************************************************************************/
BERR_Code BXVD_Userdata_Close (
	BXVD_Userdata_Handle   hUserData /* [In] User data handle */
);


/***************************************************************************
Summary: 
	Read buffer information

Description: 
	This function returns the next available packet of User Data. This is called
 from the application's userdata callback function.

Returns:
	BERR_SUCCESS              - If function is successful.
	BXVD_ERR_USERDATA_NONE    - Returns an error if no user data is available in
                             the buffer.
 BXVD_ERR_USERDATA_INVALID - The userdata buffer contains invalid data

See Also: 
	BXVD_Userdata_InstallInterruptCallback
	BXVD_Userdata_UninstallInterruptCallback
****************************************************************************/
BERR_Code BXVD_Userdata_Read(
	BXVD_Userdata_Handle   hUserData,      /* [In] User data handle */
	BAVC_USERDATA_info *pUserDataInfo  /* [Out] User data buffer info */
);


/***************************************************************************
Summary: 
	Read buffer information from an Isr

Description: 
	See description of BXVD_Userdata_Read()

Returns:
	BERR_SUCCESS - If function is successful.
	BXVD_ERR_USERDATA_NONE - Returns an error if no user data is
		available in the buffer.

See Also: 
	BXVD_USerdata_Read
****************************************************************************/
BERR_Code BXVD_Userdata_Read_isr(
	BXVD_Userdata_Handle   hUserData,     /* [In] User data handle */
	BAVC_USERDATA_info     *pUserDataInfo  /* [Out] User data buffer info */
);

/***************************************************************************
Summary: 
	Enable/Disable user data capture

Description: 
	This function enables or disables the capture of user data

Returns:
	BERR_SUCCESS - If opened XVD is successful.

See Also:
	
****************************************************************************/
BERR_Code BXVD_Userdata_Enable (
	BXVD_Userdata_Handle  hUserData, /* [In] User data handle */
	bool                  bEnable    /* [In] Enable/disable userdata */
	);

/***************************************************************************
Summary:
  Install user data callback

Description:
  This function installs a callback to the application that occurs when
  user data is available.

Returns:
  BERR_SUCCESS - If call is successful

See Also:
 	BXVD_Userdata_UninstallInterruptCallback
		BXVD_Userdata_Read
****************************************************************************/
BERR_Code BXVD_Userdata_InstallInterruptCallback
(
 BXVD_Userdata_Handle hUserData, /* [in] XVD userdata handle */
 BINT_CallbackFunc    xvdInterruptCallBack_isr, /* [in] user's callback function */
 void                   *pParm1, /* [in] unused */
 int                     parm2  /* [in] unused */
 );

/***************************************************************************
Summary:
  Uninstall user data callback

Description:
  This function uninstalls an interrupt callback installed by
  BXVD_UserData_InstallInterruptCallback.

Returns:
  BERR_SUCCESS - If call was successful

See Also:
  BXVD_Userdata_InstallInterruptCallback
  BXVD_Userdata_Read
****************************************************************************/
BERR_Code BXVD_Userdata_UninstallInterruptCallback
(
 BXVD_Userdata_Handle    hUserData, /* [in] XVD userdata handle */
 BINT_CallbackFunc       xvdInterruptCallBack_isr /* [in] callback to uninstall */
 );

/* Function Prototypes }}} */

#ifdef __cplusplus
}
#endif

#endif /* BXVD_USERDATA_H__ */
/* End of file. */
 


