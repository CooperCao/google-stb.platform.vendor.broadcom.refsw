/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BHDM_HDCP_H__
#define BHDM_HDCP_H__

/*=************************* Module Overview ********************************
  The HDCP (High-bandwidth Data Content Protection) functions provide support
  for implementing the HDCP Spec Version 1.086 as of 12/31/2002.

  HDCP requires the use of purchased production type keys or the use of test
  keys contained in the specification.  Both the transmitting and receiving
  devices must have keys.

  The specification can be found at www.digital-cp.com
***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "bhdm.h"

#define BHDM_HDCP_RX_NO_HDCP_SUPPORT  BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 1)
#define BHDM_HDCP_RX_BKSV_ERROR		  BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 2)
#define BHDM_HDCP_RX_BKSV_REVOKED     BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 3)
#define BHDM_HDCP_TX_AKSV_ERROR       BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 4)
#define BHDM_HDCP_RECEIVER_AUTH_ERROR BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 5)
#define BHDM_HDCP_REPEATER_AUTH_ERROR BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 6)

#define BHDM_HDCP_LINK_FAILURE		  BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 7)

#define BHDM_HDCP_RX_DEVICES_EXCEEDED BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 8)
#define BHDM_HDCP_REPEATER_DEPTH_EXCEEDED \
	                                  BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 9)

#define BERR_HDCP_NOT_ENOUGH_MEM           BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 10)
#define BHDM_HDCP_NO_AUTHENTICATED_LINK    BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 11)
#define BHDM_HDCP_REPEATER_FIFO_NOT_READY  BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 12)

#define BHDM_HDCP_REPEATER_DEVCOUNT_0      BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 13)
#define BHDM_HDCP_AUTH_ABORTED     BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 14)

#define BHDM_HDCP_LINK_RI_FAILURE		  BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 15)
#define BHDM_HDCP_LINK_PJ_FAILURE		  BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 16)

#define BHDM_HDCP_PLL_PWRDN 	BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 17)
#define BHDM_HDCP_FIFO_UNDERFLOW 	BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 18)
#define BHDM_HDCP_FIFO_OVERFLOW  	BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 19)
#define BHDM_HDCP_MULTIPLE_AN_REQUEST 	BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 20)

#define BHDM_HDCP_RX_BKSV_I2C_READ_ERROR BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 21)
#define BHDM_HDCP_TX_AKSV_I2C_WRITE_ERROR BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS+ 22)


/* keep older definition in case in use */
#define BHDM_HDCP_AUTHENTICATE_ERROR  BHDM_HDCP_RECEIVER_AUTH_ERROR

/******************************************************************************
Summary:
HDCP Definitions
*******************************************************************************/

#define BHDM_HDCP_RX_I2C_ADDR       0x3A   /* 7 Bit Addr */

#define BHDM_HDCP_NUM_KEYS  40
#define BHDM_HDCP_KSV_LENGTH 5
#define BHDM_HDCP_AN_LENGTH 8

#define BHDM_HDCP_MAX_PJ_MISMATCH 3
#define BHDM_HDCP_MAX_I2C_RETRY 3

#define BHDM_HDCP_MAX_PJ_LINK_FAILURES_BEFORE_DISABLE_HDCP_1_1 5

/******************************************************************************
Summary:
I2C Rx HDCP Registers
*******************************************************************************/

#define BHDM_HDCP_1_4_OFFSET_START 0x00
#define BHDM_HDCP_2_2_OFFSET_START 0x50


/* Rx Bksv value (Read Only) */
#define BHDM_HDCP_RX_BKSV0			0x00
#define BHDM_HDCP_RX_BKSV1			0x01
#define BHDM_HDCP_RX_BKSV2			0x02
#define BHDM_HDCP_RX_BKSV3			0x03
#define BHDM_HDCP_RX_BKSV4			0x04

/* Rx Link verification value (Read Only) */
#define BHDM_HDCP_RX_RI0		    0x08
#define BHDM_HDCP_RX_RI1		    0x09

/*  Rx Enhanced Link Verification Response (Read Only) */
#define BHDM_HDCP_RX_PJ             0x0A


/* Tx Aksv value (Write Only) */
#define BHDM_HDCP_RX_AKSV0			0x10
#define BHDM_HDCP_RX_AKSV1			0x11
#define BHDM_HDCP_RX_AKSV2			0x12
#define BHDM_HDCP_RX_AKSV3			0x13
#define BHDM_HDCP_RX_AKSV4			0x14

/*  Rx HDCP Enable HDCP 1.1 features (Write Only) */
#define BHDM_HDCP_RX_AINFO             0x15
#define BHDM_HDCP_RX_ENABLE_1_1_FEATURES 0x02

/* Session Random Number (An) value generated by the Tx (Write Only) */
#define BHDM_HDCP_RX_AN0			0x18
#define BHDM_HDCP_RX_AN1			0x19
#define BHDM_HDCP_RX_AN2			0x1a
#define BHDM_HDCP_RX_AN3			0x1b
#define BHDM_HDCP_RX_AN4			0x1c
#define BHDM_HDCP_RX_AN5			0x1d
#define BHDM_HDCP_RX_AN6			0x1e
#define BHDM_HDCP_RX_AN7			0x1f

/* HDCP Repeater SHA-1 Hash value V' */
#define BHDM_HDCP_REPEATER_SHA1_V_H0 0x20
#define BHDM_HDCP_REPEATER_SHA1_V_H1 0x24
#define BHDM_HDCP_REPEATER_SHA1_V_H2 0x28
#define BHDM_HDCP_REPEATER_SHA1_V_H3 0x2c
#define BHDM_HDCP_REPEATER_SHA1_V_H4 0x30

/* Rx Capabilities Register (Read Only) */
#define BHDM_HDCP_RX_BCAPS          0x40

/* Rx Status Registers (Read Only) */
#define BHDM_HDCP_RX_BSTATUS        0x41 /* 2 Bytes */
#define BHDM_HDCP_RX_BSTATUS_DEPTH        0x0700
#define BHDM_HDCP_RX_BSTATUS_DEVICE_COUNT 0x007F

#define BHDM_HDCP_REPEATER_KSV_FIFO 0x43 /* 2 Bytes */

#define BHDM_HDCP_RX_HDCP2VERSION	0x50 /* 1 byte */

/* HDCP Repeater Registers */
#define BHDM_HDCP_REPEATER_MAX_DEVICE_COUNT 127
#define BHDM_HDCP_REPEATER_MAX_DEPTH 7

/* HDCP 2.x */
#define BHDM_HDCP2X_REAUTH_REQ	0x08

/******************************************************************************
Summary:
Enumerated Type containing the type of HDCP An value to be generated

Description:
The DVI/HDMI transmitter is capable of generating a psuedo-random number (An)
which is used as a initial seed for the HDCP calculations.  This Enumerated
Type specifies the type of An value which can be generated.

   BHDM_HDCP_AnSelect_eRandomAn   - generate random An value
   BHDM_HDCP_AnSelect_eTestA1B1An - generate fixed A1/B1 HDCP Spec An value
   BHDM_HDCP_AnSelect_eTestA1B2An - generate fixed A1/B2 HDCP Spec An value
   BHDM_HDCP_AnSelect_eTestA2B1An - generate fixed A2/B1 HDCP Spec An value
   BHDM_HDCP_AnSelect_eTestA2B2An - generate fixed A2/B2 HDCP Spec An value


*******************************************************************************/
typedef enum BHDM_HDCP_AnSelect
{
   BHDM_HDCP_AnSelect_eTestA1B1An, /* generate fixed A1/B1 HDCP Spec An value */
   BHDM_HDCP_AnSelect_eTestA1B2An, /* generate fixed A1/B2 HDCP Spec An value */
   BHDM_HDCP_AnSelect_eTestA2B1An, /* generate fixed A2/B1 HDCP Spec An value */
   BHDM_HDCP_AnSelect_eTestA2B2An, /* generate fixed A2/B2 HDCP Spec An value */
   BHDM_HDCP_AnSelect_eRandomAn   /* generate random An value */
} BHDM_HDCP_AnSelect ;


/******************************************************************************
Summary:
Enumerated Type specifying the capabilites of the attached receiver

Description:
The enumeration types listed can be compared with the value of the returned
BCaps register from the Receiver.

See Also:
	BHDM_HDCP_GetRxCaps

*******************************************************************************/
typedef enum BHDM_HDCP_RxCaps
{
   BHDM_HDCP_RxCaps_eHdmiCapable        = 0x80,  /* Rx is HDMI Capable      */
   BHDM_HDCP_RxCaps_eHdcpRepeater       = 0x40,  /* Rx is Repeater          */
   BHDM_HDCP_RxCaps_eKsvFifoReady       = 0x20,  /* Rx Ksv FIFO is ready    */
   BHDM_HDCP_RxCaps_eI2c400KhzSupport   = 0x10,  /* Rx I2C supports 400KHz  */
   BHDM_HDCP_RxCaps_eHDCP_1_1_Features  = 0x02,  /* Rx has HDCP1.1 Features
												     EESS
													 Advance Cipher
													 Enhanced Link Verification
												   */
   BHDM_HDCP_RxCaps_eFastReauth         = 0x01   /* Rx can receive un-encrypted
												  video during Re-authemtication
												  */
} BHDM_HDCP_RxCaps  ;


/******************************************************************************
Summary:
Enumerated Type specifying the current status iof the attached receiver.

Description:
The enumeration types listed can be compared with the value of the returned
BStatus register from the Receiver

See Also:
	BHDM_HDCP_GetRxStatus

*******************************************************************************/
typedef enum BHDM_HDCP_RxStatus
{
   BHDM_HDCP_RxStatus_eHdmiMode             = 0x1000, /* Rx in HDMI mode        */
   BHDM_HDCP_RxStatus_eMaxRepeatersExceeded = 0x0800, /* Rx has too many repeaters */
   BHDM_HDCP_RxStatus_eMaxDevicesExceeded   = 0x0080  /* Rx has too many devices */
} BHDM_HDCP_RxStatus ;


/******************************************************************************
Summary:
Enumerated Type specifying which version of HDCP to use

Description:
The enumeration types listed can be used to configure the core to use features
of HDCP 1.1 or HDCP 1.1 with optional features. BHDM_HDCP_Version_e1_0 is deprecated.

*******************************************************************************/
typedef enum BHDM_HDCP_Version
{
   BHDM_HDCP_Version_eUnused,
   BHDM_HDCP_Version_e1_0,
   BHDM_HDCP_Version_e1_1 = BHDM_HDCP_Version_e1_0,
   BHDM_HDCP_Version_e1_1_Optional_Features,
   BHDM_HDCP_Version_e2_2
} BHDM_HDCP_Version ;


typedef struct _BHDM_HDCP_OPTIONS_
{
    bool PjChecking ;       /* Set to true to enable HDCP 1.1 feature when applicable */
    uint8_t numPjFailures;  /* Number of Pj Link failures.
			                            If greater than BHDM_HDCP_MAX_PJ_LINK_FAILURES_BEFORE_DISABLE_HDCP_1_1,
  				                          HDCP 1.1 feature will not be enabled */

} BHDM_HDCP_OPTIONS ;

/******************************************************************************
Summary:
Read the HDCP BKsv value from the Receiver

Description:
This function
1) reads the Bksv value from the receiver,
2) verifies it is a valid Ksv (20 zeros & 20 ones)
3) writes the Bksv value to the HDMI transmitter


Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	pRevokedKsvList - List of Revoked KSVs obtained from DCP LLC SRM message

	uiNumRevokedKsvs - Number of Ksvs contained in the pRevokedKsvList

Output:
	<none>

Returns:
	BERR_SUCCESS                  - Bksv value read and successfully written
	BERR_INVALID_PARAMETER        - Invalid function parameter.
	BHDM_HDCP_RX_BKSV_ERROR       - Attached Rx Device has invalid Bksv

Note:
The pRevokedKsvList should be passed in as big-endian (the same endian-ness
contained in the original SRM message).

Use NULL and 0 for pRevokedKsvList and uiNumRevokedKsvs if no DCP LLC SRM
messages are available.

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_ReadRxBksv(
   BHDM_Handle hHDMI,              /* [in] HDMI handle */
   const uint8_t *pRevokedKsvList, /* [in] pointer to Revoked KSV List */
   const uint16_t uiNumRevokedKsvs /* [in] number of KSVs in Revoked Ksv List */
) ;


/******************************************************************************
Summary:
Generate the HDCP An value for HDCP calculations

Description:
This function generates the random HDCP An value.  This function also supports
generation of the fixed An values contained in the HDCP Specification


Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	AnSelection - HDCP An value to be generated by the HDCP core

Output:
	<none>

Returns:
	BERR_SUCCESS                  - Generated and wrote HDCP An value
	BERR_INVALID_PARAMETER        - Invalid function parameter.

See Also:
	o BHDM_Handle
	o BHDM_HDCP_AnSelect

*******************************************************************************/
BERR_Code BHDM_HDCP_GenerateAn(
	BHDM_Handle hHDMI,              /* [in] HDMI handle */
	BHDM_HDCP_AnSelect AnSelection  /* [in] HDCP An type value to use */
) ;



/******************************************************************************
Summary:
Configure the HDMI Core to receive the HDCP Keys from the XPT Key serializer.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	<none>

Returns:
	BERR_SUCCESS                  - HDCP protected link created.
	BERR_INVALID_PARAMETER        - Invalid function parameter.

See Also:
	o BHDM_Handle
*******************************************************************************/
BERR_Code BHDM_HDCP_EnableSerialKeyLoad(
	BHDM_Handle hHDMI              /* [in] HDMI handle */
) ;


/******************************************************************************
Summary: Write the HDCP Aksv value to the receiver

Description:
This function
1) verifies the Aksv is a valid Ksv (20 zeros & 20 ones)
1) writes the Aksv value to the receiver,


Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	pTxAksv  - pointer to the Transmitter Key Selection Vector (Aksv) shipped
	with the HDCP key set from Digital-CP (www.digital-cp.com)

Output:
	<none>

Returns:
	BERR_SUCCESS                  - HDCP protected link created.
	BERR_INVALID_PARAMETER        - Invalid function parameter.
	BHDM_HDCP_TX_AKSV_ERROR       - Transmtter Aksv value is invalid

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_WriteTxAksvToRx(
   BHDM_Handle hHDMI,           /* [in] HDMI handle */
   const uint8_t *pTxAksv       /* [in] pointer HDCP Key Set Aksv Value */
)  ;


/******************************************************************************
BERR_Code BHDM_HDCP_AuthenticateLink
Summary: Authenticate the HDCP Link; verify TxR0 and RxR0 values are equal

*******************************************************************************/
BERR_Code BHDM_HDCP_AuthenticateLink
(
	BHDM_Handle hHDMI              /* [in] HDMI handle */
) ;



/******************************************************************************
Summary:
Clear the Authenticated link between the Transmitter and the Receiver.

Description:
This function sets the transmitted in an un-authenticated state

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	<none>

Returns:
	BERR_SUCCESS                  - HDCP protected link created.
	BERR_INVALID_PARAMETER        - Invalid function parameter.

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_ClearAuthentication(
   BHDM_Handle hHDMI              /* [in] HDMI handle */
) ;


/******************************************************************************
Summary:
Start transmitting video encrypted with HDCP.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	<none>

Returns:
	BERR_SUCCESS - transmission of HDCP protected video has begun
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_XmitEncrypted(
   BHDM_Handle hHDMI             /* [in] HDMI handle */
) ;


/******************************************************************************
Summary:
Start transmitting video without HDCP.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	<none>

Returns:
	BERR_SUCCESS - transmission of video is now done without HDCP encryption
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_XmitClear(
   BHDM_Handle hHDMI              /* [in] HDMI handle */
) ;

/******************************************************************************
Summary:
Use the HDCP Ri value to verify a protected HDCP link is still valid.

Description:
This function should be called approximately every two seconds (~128 frames)
to verify the HDCP link is still valid.  The validation is done by reading
the Ri values from the Rx and the Tx.  The Ri values are calculated using the
HDCP Key Set.  The Ri values should be equal and should also change
approximately every two seconds.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	<none>

Returns:
	BERR_SUCCESS            - HDCP Link is still valid
	BERR_INVALID_PARAMETER  - Invalid function parameter.
	BHDM_HDCP_LINK_FAILURE  - HDCP Link has failed

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_RiLinkIntegrityCheck(
   BHDM_Handle hHDMI              /* [in] HDMI handle */
) ;

/******************************************************************************
Summary:
Use the HDCP Pj value to verify the protected HDCP link is still valid.

Description:
This function should be called approximately every TBD (16 frames)
to verify the HDCP link is still valid.  The validation is done by reading
the Pj values from the Rx and the Tx.  The Pj values are calculated using the
HDCP Key Set.  The Pj values should be equal and should also change
every 16 frames.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	<none>

Returns:
	BERR_SUCCESS            - HDCP Link is still valid
	BERR_INVALID_PARAMETER  - Invalid function parameter.
	BHDM_HDCP_LINK_FAILURE  - HDCP Link has failed

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_PjLinkIntegrityCheck(
   BHDM_Handle hHDMI              /* [in] HDMI handle */
) ;



/******************************************************************************
Summary:
Retrieve the Rx Capabilities.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pRxCapsReg - contents of Rx capabilities;
	Use logical AND with RxCapsRegister and BHDM_HDCP_RxCaps feature
	enumeration type to check for specific capability:

Example:
	feature - Rx Capabilities

	BHDM_HDCP_RxCaps_eHdmiCapable		- Rx is HDMI Capable
	BHDM_HDCP_RxCaps_eHdcpRepeater		- Rx is a Repeater
	BHDM_HDCP_RxCaps_eKsvFifoReady		- Rx Ksv FIFO is ready
	BHDM_HDCP_RxCaps_eI2c400KhzSupport	- Rx I2C supports 400KHz Xfer
	BHDM_HDCP_RxCaps_eHDCP_1_1_Features - Rx supports HDCP 1.1 Features
	                                       EESS, Advance Cipher, and
	                                       Enhanced Link Verification
    BHDM_HDCP_RxCaps_eFastReauth        - Rx supports fast Re-authemtication

	BHDM_HDCP_GetRxCaps(hHDMI, &RxCaps) ;
	if (RxCaps & BHDM_HDCP_RxCaps_eHdmiCapable)
		HDMI Capable
	else
		DVI Only


Returns:
	BERR_SUCCESS
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	o BHDM_Handle
	o BHDM_HDCP_RxCaps

*******************************************************************************/
BERR_Code BHDM_HDCP_GetRxCaps(
   BHDM_Handle hHDMI,         /* [in] HDMI handle */
   uint8_t *pRxCapsReg     /* [out] HDCP Rx Capability */
) ;


/******************************************************************************
Summary:
Check the status of the attached receiver.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pRxStatusReg - contents of Rx Status Register

	Use logical AND with RxStatusReg and BHDM_HDCP_RxStatus
	enumeration type to check for specific capability:

	BHDM_HDCP_RxStatus_eHdmiMode			 -  Rx in HDMI mode
	BHDM_HDCP_RxStatus_eMaxRepeatersExceeded -  Rx has too many repeaters
	BHDM_HDCP_RxStatus_eMaxDevicesExceeded	 -  Rx has too many devices

Returns:
	BERR_SUCCESS

See Also:
	o BHDM_Handle
	o BHDM_HDCP_RxStatus

*******************************************************************************/
BERR_Code BHDM_HDCP_GetRxStatus(
   BHDM_Handle hHDMI,                /* [in] HDMI handle */
   uint16_t *pRxStatusReg          /* [out] HDCP Rx Status */
) ;


/******************************************************************************
Summary:
Get the depth (number of levels) of HDCP Repeaters

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pRepeaterDepth - pointer to integer containing the number of levels (depth)
	of receive devices.

Returns:
	BERR_SUCCESS - Repeater Depth successfully retrieved
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	o BHDM_Handle
	o BHDM_HDCP_GetRepeaterDeviceCount

*******************************************************************************/
BERR_Code BHDM_HDCP_GetRepeaterDepth(
   BHDM_Handle hHDMI,      /* [in] HDMI handle */
   uint8_t *pRepeaterDepth  /* [out] pointer to hold Levels of HDCP Repeaters */
) ;


/******************************************************************************
Summary:
Get the number of devices attached to the HDCP Repeater.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pNumDevices - Number of devices attached to the HDCP Receiver

Returns:
	BERR_SUCCESS - Number of Repeaters successfully retrieved
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	o BHDM_Handle
	o BHDM_HDCP_GetRepeaterDepth

*******************************************************************************/
BERR_Code BHDM_HDCP_GetRepeaterDeviceCount(
   BHDM_Handle hHDMI,   /* [in] HDMI handle */
   uint8_t *pNumDevices /* [out] pointer to # of devices attached to the HDCP
                           Repeater */
) ;


/******************************************************************************
Summary:
Read the KSV list from the attached repeater.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.
	uiDeviceCount - Number of KSVs to be read from the Ksv FIFO
	pRevokedKsvList - List of Revoked KSVs obtained from DCP LLC SRM message
	uiNumRevokedKsvs - Number of Ksvs contained in the pRevokedKsvList


Output:
	pKsvList - pointer to memory location to load the KSV list read from the
	HDCP repeater KSV fifo.

Returns:
	BERR_SUCCESS
	BERR_INVALID_PARAMETER    - Invalid function parameter.
	BERR_HDCP_NOT_ENOUGH_MEM  - Not enough memory to hold results

Note:
	The pKsvList must point to a buffer (allocated and freed by the calling
	function) that is DEVICE_COUNT * 5 bytes long.  The DEVICE_COUNT can be
	retrieved using the BHDM_HDCP_GetRepeaterDeviceCount function.


See Also:
	o BHDM_Handle
	o BHDM_HDCP_GetRepeaterDeviceCount

*******************************************************************************/
BERR_Code BHDM_HDCP_ReadRxRepeaterKsvFIFO(
   BHDM_Handle hHDMI,     /* [in] HDMI handle */
   uint8_t *pKsvList,     /* [out] pointer to memory to hold KSV List */
   uint16_t uiDeviceCount, /* [in ] number of devices size of memory in bytes to hold KSV List */
   const uint8_t *pRevokedKsvList, /* [in] pointer to Revoked KSV List */
   const uint16_t uiNumRevokedKsvs /* [in] number of KSVs in Revoked Ksv List */
) ;



/******************************************************************************
Summary:
Write the KSV listread  from the attached repeater to the transmitter core

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.
	pKsvList - pointer to memory location holding the KSV list from the
	HDCP repeater KSV fifo.
	uiDeviceCount - number of Ksvs from the downstream Rx Devices

Output:
	<none>

Returns:
	BERR_SUCCESS
	BERR_INVALID_PARAMETER    - Invalid function parameter.
	BERR_HDCP_NOT_ENOUGH_MEM  - Not enough memory to hold results

Note:
	The pKsvList must point to a buffer (allocated and freed by the calling
	function) that is DEVICE_COUNT * 5 bytes long.  The DEVICE_COUNT can be
	retrieved using the BHDM_HDCP_GetRepeaterDeviceCount function.


See Also:
	o BHDM_Handle
	o BHDM_HDCP_GetRepeaterDeviceCount

*******************************************************************************/
BERR_Code BHDM_HDCP_WriteTxKsvFIFO(
	BHDM_Handle hHDMI,     /* [in] HDMI handle */
	uint8_t *pKsvList,     /* [in] pointer to memory to holding KSV List */
	uint16_t uiDeviceCount /* [in] number of Ksvs from the downstream Rx
	                          Devices */
) ;



/******************************************************************************
Summary:
Check if the current link is authenticated.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.


Output:
	bAuthenticated - true is link authenticated. False otherwise

Returns:
	BERR_SUCCESS


See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_IsLinkAuthenticated(
	const BHDM_Handle hHDMI,
	bool *bAuthenticated
);


/******************************************************************************
Summary:
Check if the current Receiver is a repeater

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.


Output:
	uiRepeater - flag indicating the attached receiver is a repeater

Returns:
	BERR_SUCCESS
	BERR_INVALID_PARAMETER    - Invalid function parameter.

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_CheckForRepeater(
   BHDM_Handle hHDMI,           /* [in] HDMI handle */
   uint8_t *uiRepeater          /* [out] HDCP Rx Status */
) ;


/******************************************************************************
Summary:
Create an authenticated link with the attached repeater.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	bRepeaterAuthenticated - flag indicating the success/failure of the repeater
	authentication

Returns:
	BERR_SUCCESS
	BERR_INVALID_PARAMETER    - Invalid function parameter.

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_RepeaterAuthenticateLink(
	BHDM_Handle hHDMI,
	uint8_t *RepeaterAuthenticated
) ;

/******************************************************************************
Summary:
Initialize Authentication with the attached repeater

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.


Output:
	<none>

Returns:
	BERR_SUCCESS
	BERR_INVALID_PARAMETER    - Invalid function parameter.

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_InitializeRepeaterAuthentication(
   BHDM_Handle hHDMI           /* [in] HDMI handle */
) ;


/******************************************************************************
Summary:
Get a copy of the attached Rx device's KSV

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	AttachedDeviceKsv - Ksv of the Attached DVI/HDMI Rx

Returns:
	BERR_SUCCESS
	BHDM_RX_NO_DEVICE    - No attached device(s).

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_GetRxKsv(BHDM_Handle hHDMI,
	uint8_t *AttachedDeviceKsv
) ;


/******************************************************************************
Summary:
Get a copy of the KSV(s) attached to the HDMI Transmitter

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	DownstreamKsvsBuffer - pointer to buffer to hold KSV information
	DownstreamKsvsBufferSize - size in bytes of the DownstreamKsvsBuffer

Returns:
	BERR_SUCCESS
	BERR_INVALID_PARAMETER - Invalid buffer or buffer size
	BHDM_RX_NO_DEVICE    - No attached device(s).

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_GetRepeaterKsvFifo(BHDM_Handle hHDMI,
	uint8_t *DownstreamKsvsBuffer, uint16_t DownstreamKsvsBufferSize
) ;


/******************************************************************************
Summary:
Force the V Calculation for attached repeaters with device count zero

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:

Returns:

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_ForceVCalculation(BHDM_Handle hHDMI) ;


/******************************************************************************
Summary:
Get the options used for HDCP Authentication

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.
	HdcpOptions - pointer to structure containing HDCP Options

Output:

Returns:

See Also:
	o BHDM_Handle
	o BHDM_HDCP_OPTIONS

*******************************************************************************/
BERR_Code BHDM_HDCP_GetOptions(
	BHDM_Handle hHDMI, BHDM_HDCP_OPTIONS *HdcpOptions) ;


/******************************************************************************
Summary:
Set the options used for HDCP Authentication

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.
	HdcpOptions - pointer to structure containing HDCP Options to set

Output:

Returns:

See Also:
	o BHDM_Handle
	o BHDM_HDCP_OPTIONS

*******************************************************************************/
BERR_Code BHDM_HDCP_SetOptions(
	BHDM_Handle hHDMI, BHDM_HDCP_OPTIONS *HdcpOptions) ;



/******************************************************************************
Summary:
Force all video pixel a particular value, 0 or 1. This function is meant to be used for
debugging purposes only.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.
	value  - value to force video pixels, should be 0 or 1

Output:

Returns:

See Also:
	o BHDM_Handle
	o BHDM_HDCP_P_DEBUG_PjCleanVideo

*******************************************************************************/
BERR_Code BHDM_HDCP_P_DEBUG_PjForceVideo(BHDM_Handle hHDMI, uint8_t value);


/******************************************************************************
Summary:
Clean video pixel which was forced by BHDM_HDCP_P_DEBUG_PjCleanVideo.
This function is meant to be used for debugging purposes only.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:

Returns:

See Also:
	o BHDM_Handle
	o BHDM_HDCP_P_DEBUG_PjForceVideo

*******************************************************************************/
BERR_Code BHDM_HDCP_P_DEBUG_PjCleanVideo(BHDM_Handle hHDMI, uint8_t value);


/******************************************************************************
Summary:
Read which HDCP Version the attached receiver supported

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	eVersion - Supported HDCP Version by attached TV.

Returns:

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_GetHdcpVersion(const BHDM_Handle hHDMI, BHDM_HDCP_Version *eVersion);



/******************************************************************************
Summary:

Simulate HPD assert/de-assert to the Tx by overriding the hotplug input level to the Tx

Note: The attached Rx does not see this HPD signal

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	enable - True to enable the override mode
	bAssertSimulatedHpd - True to assert HPD signal (CONNECT)
					      false to de-assert HDP signal (REMOVE)

See Also:
	o BHDM_HDCP_RiLinkIntegrityCheck()

*******************************************************************************/
BERR_Code BHDM_HDCP_AssertSimulatedHpd_isr(const BHDM_Handle hHDMI, bool enable, bool bAssertSimulatedHpd);



/******************************************************************************
Summary:
Update HDCP 2x authentication status to HDMI Cipher

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.
	authenticated - result of HDCP 2.x authentication process

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_UpdateHdcp2xAuthenticationStatus(const BHDM_Handle hHDMI, const bool authenticated);


/******************************************************************************
Summary:
	Enable/Disable HDCP 2.x encryption

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	enable - "true" to enable, "false" to disable

Returns:

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_EnableHdcp2xEncryption(const BHDM_Handle hHDMI, const bool enable);


/******************************************************************************
Summary:
	Enable/Disable HDCP 2.x encryption - call in isr context only

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	enable - "true" to enable, "false" to disable

Returns:

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_EnableHdcp2xEncryption_isr(const BHDM_Handle hHDMI, const bool enable);


/******************************************************************************
Summary:
	Determine current encryption status of HDCP 2.x engine

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	bEncrypted - true if HDCP2.x encryption is currently enabled. False otherwise.

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_GetHdcp2xEncryptionStatus(const BHDM_Handle hHDMI, bool *bEncrypted);




typedef struct
{
	bool bAuthentictated;
	bool bEncrypted;
}
BHDM_HDCP_AuthenticationStatus ;


/******************************************************************************
Summary:
	Get the current HDCP authentication status

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	bAuthenticated - true if attached device is authenticated  False otherwise.
	bEncrypted - true if Tx is transmitting encrypted data  False otherwise.

Note:
	Tx can be authenticated, but transmitting in the clear

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_GetAuthenticationStatus(
	const BHDM_Handle hHDMI,
	BHDM_HDCP_AuthenticationStatus *stHdcpAuthStatus) ;



/******************************************************************************
Summary:
	Set HDCP 2.x Tx Capability in the HW.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	version - transmitter HDCP Version
	txCapsMask - transmitter capability mask

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_SetHdcp2xTxCaps(const BHDM_Handle hHDMI, const uint8_t version, const uint16_t txCapsMask);


/******************************************************************************
Summary:
	Kick Start Hdcp 2.x HDMI HW Cipher

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_KickStartHdcp2xCipher(const BHDM_Handle hHDMI);


/******************************************************************************
Summary:
	Determine whether the source receive a REAUTH_REQ

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	bReauthReqPending - true if received a REAUTH_REQ from the receiver but hasn't yet react to it.
						False otherwise

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_HDCP_IsReauthRequestPending(const BHDM_Handle hHDMI, bool *bReauthReqPending);



#ifdef __cplusplus
}
#endif



#endif /* BHDM_HDCP_H__ */

