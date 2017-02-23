/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/


#ifndef BHSM_OTPMSP_H__
#define BHSM_OTPMSP_H__

#include "bhsm.h"
#include "bsp_s_version_number.h"
#include "bsp_s_otp_common.h"
#include "bsp_s_otp.h"

#ifdef __cplusplus
extern "C" {
#endif



typedef struct BHSM_ProgramOtpPatSeqIO {

    /* In:  OTP Programming Pattern Cache Request */
    bool                        bProgCacheReq;

    /* Out: 0 for success, otherwise failed */
    uint32_t                    unStatus;
} BHSM_ProgramOtpPatSeqIO_t;


/*****************************************************************************
Summary:

This function is available only from 7420B0 and on and must be called
before any OTP/MSP related functions are called.

See Also:


*****************************************************************************/
BERR_Code      BHSM_ProgramOTPPatternSequence (
        BHSM_Handle                    hHsm,
        BHSM_ProgramOtpPatSeqIO_t    *inoutp_progOtpPatSeqIO

);




#define BHSM_OTP_DATA_LEN              4  /* 4 bytes OTP data read at a time */

/**************************************************************************************************
Summary:

Description:
Structure that defines which OTP field to read and holds the returned value of that OTP field.

See Also:
BHSM_ReadOTP
**************************************************************************************************/
typedef struct BHSM_ReadOtpIO {

    /* In:  which OTP field to read */
    BPI_Otp_CmdReadRegister_e        readOtpEnum;

    /* In: key type */
    BPI_Otp_KeyType_e                keyType;

    /* Out: 0 for success, otherwise failed */
    uint32_t                        unStatus;

    /* Out:  low 32-bit value of the requested 64-bit register */
    unsigned char                    regValueLo[BHSM_OTP_DATA_LEN];
    /* Out: high 32-bit value of the requested 64-bit register */
    unsigned char                    regValueHi[BHSM_OTP_DATA_LEN];
    /* Out: register value 2  */
    unsigned char                    regValue2[BHSM_OTP_DATA_LEN];
    /* Out: register value 3 */
    unsigned char                    regValue3[BHSM_OTP_DATA_LEN];

} BHSM_ReadOtpIO_t;


/*****************************************************************************
Summary:

This function returns one OTP key identifiers or one MC0 OTP value.

Description:

This function shall be used to read either OTP key identifier or other OTP field value.
Depends on the access control matrix, only certain OTP fields can be read in a specific
customer mode.  Note that this function can only read back one OTP value at a time.

Calling Context:

This function can be called anytime after HSM is initialized.


Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.


Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_readOtpIO  - BHSM_ReadOtpIO_t.


Output:
inoutp_readOtpIO  - BHSM_ReadOtpIO_t.


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_ReadMSP

*****************************************************************************/
BERR_Code      BHSM_ReadOTP (
        BHSM_Handle            hHsm,
        BHSM_ReadOtpIO_t    *inoutp_readOtpIO
);

#define BHSM_MSP_DATA_LEN    4

typedef enum BCMD_Msp_CmdProgMode_e
{
    BCMD_Msp_CmdProgMode_eEnum    = 0x00010112,

    BCMD_Msp_CmdProgMode_eReserved   /* 3 others reserved, don't use this for MSP programming*/
} BCMD_Msp_CmdProgMode_e ;


/**************************************************************************************************
Summary:

Description:
Structure that defines which MSP field to program, with what data,  using what mask and proper mode,  and holds the returned status
of a programming request

See Also:
BHSM_ProgramMSP
**************************************************************************************************/
typedef struct BHSM_ProgramMspIO {

    /* in: This field should contain the value of BCMD_OTP_PROGMSP_MODE (i.e., 0x00010112)
    for the bits to be programmed. This is a sanity check on the command.
    */
    BCMD_Msp_CmdProgMode_e         progMode;

    /* in: specifies which MSP bits to program. The values this field can take are specified by the
    typedef enum BPI_Otp_CmdMsp_e in the share C header file.  Each chip will have different enums
    and customers will only have access to the files for the chips that they use.
    */
    BCMD_Otp_CmdMsp_e            progMspEnum;

    /* in: number of bits of the MSP enum to program, from 1 to 32*/
    unsigned char                ucDataBitLen;

    /* in :0x0000_0001 to 0xFFFF_FFFF
        A value 1 in a bit position specifies that the data value at that bit position is to be programmed.
        BSP will not program bits that have a zero bit in the mask.
            For example 0x0F means to program the 4 LSBits of an enum.
            For example 0x8F means to program the bit 7 and 4 LSBits of an enum.

    */
    unsigned char                aucDataBitMask [BHSM_MSP_DATA_LEN];

    /* in: the value that needs to be programmed*/
    unsigned char                aucMspData[BHSM_MSP_DATA_LEN];

    /* Out: MSP programming command PASSed/0, otherwise FAILed. */
    uint32_t                    unStatus;

} BHSM_ProgramMspIO_t;


/*****************************************************************************
Summary:

This function allows the programming of each of the field programmable
OTP (MSP) bits.

Description:
This function allows the programming of each of the field programmable
OTP (MSP) bits.  Based on the Access Control Matrix (ACL), programming of the
bit is allowed or disallowed.

Calling Context:
This function can be called anytime after HSM is initialized.

Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.


Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_progMspIO  - BHSM_ProgramMspIO_t.

Output:
inoutp_progMspIO  - BHSM_ProgramMspIO_t.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_ProgramOTP

*****************************************************************************/
BERR_Code      BHSM_ProgramMSP (
        BHSM_Handle                hHsm,
        BHSM_ProgramMspIO_t        *inoutp_progMspIO
);


/**************************************************************************************************
Summary:

Description:
Structure that defines which MSP field to read and its required key/data if command authentication is needed by BSP,
and holds the returned value of the MSP field

See Also:
BHSM_ReadMSP
**************************************************************************************************/
typedef struct BHSM_ReadMspIO {

    /* In: which MSP to read */
    BCMD_Otp_CmdMsp_e            readMspEnum;

    /* Out: 0 for success, otherwise failed */
    uint32_t                    unStatus;

    /* Out: the return MSP field*/
    unsigned char                aucMspData[BHSM_MSP_DATA_LEN];

    /* Out: the MSP Lock Value */
    unsigned char                aucLockMspData[BHSM_MSP_DATA_LEN];

} BHSM_ReadMspIO_t;


/*****************************************************************************
Summary:

This function returns one MSP value.

Description:

This function shall be used to read MSP field value. Depends on the access control matrix, only
certain MSP fields can be read in a specific customer mode.  Note that this function can only
read back one MSP value at a time.

Calling Context:

This function can be called anytime after HSM is initialized.


Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_readMspIO  - BHSM_ReadMspIO_t.


Output:
inoutp_readMspIO  - BHSM_ReadMspIO_t.


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_ReadOTP

*****************************************************************************/
BERR_Code      BHSM_ReadMSP (
        BHSM_Handle                hHsm,
        BHSM_ReadMspIO_t        *inoutp_readMspIO
);



#define BHSM_READ_DATA_SECTION_DATA_LEN        (32)        /* in byte now, may change to word32*/

/**************************************************************************************************
Summary:

Description:
Structure that defines which OTP data section to read, and will hold the returned the data from that section

See Also:
BHSM_ReadDataSect
**************************************************************************************************/
typedef struct BHSM_ReadDataSectIO {

    /* In:  which OTP data section to read, between BPI_Otp_DataSection_e0 ~ BPI_Otp_DataSection_e7*/
    BPI_Otp_DataSection_e        readDsEnum;

    /* Out: 0 for success, otherwise failed */
    uint32_t                    unStatus;

    /* Out: true means the data section is read protected, the output is sha1 of the data section. */
    bool                        isReadProtected;

    /* Out: the returned 32 bytes from the OTP data section as specified */
    unsigned char                aucDataSectData[BHSM_READ_DATA_SECTION_DATA_LEN];

} BHSM_ReadDataSectIO_t;

/*****************************************************************************
Summary:

This function returns one 32-byte data section value.

Description:

There are total of 8 32-byte data sections. This function shall be used to read each 32-byte data section.


Calling Context:

This function can be called anytime after HSM is initialized.


Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_readDataSectIO  - BHSM_ReadDataSectIO_t


Output:
inoutp_readDataSectIO  - BHSM_ReadDataSectIO_t


See Also:
BHSM_ProgramDataSect
*****************************************************************************/
BERR_Code      BHSM_ReadDataSect (
        BHSM_Handle                hHsm,
        BHSM_ReadDataSectIO_t    *inoutp_readDataSectIO
);




#define BHSM_DATA_SECTION_DATA_LEN         32   /* 256 bits, in bytes */
#define BHSM_CRC_DATA_LEN                 4    /* 4 bytes */

/**************************************************************************************************
Summary:

Description:
Structure that defines which OTP data section to program with what data using a proper mode, and holds
the returned status

See Also:
BHSM_ProgramDataSect
**************************************************************************************************/
typedef struct BHSM_ProgramDataSectIO {

    /* In: select which OTP data section to program, between BPI_Otp_DataSection_e0 ~
            BPI_Otp_DataSection_e7 */
    BPI_Otp_DataSection_e        progDsEnum;

    /* In: provide the actual 32-byte data to be programmed into the specified OTP data section*/
    unsigned char                aucDataSectData[BHSM_DATA_SECTION_DATA_LEN];

    /* In: provide the crc of data section */
    unsigned char                aucCrc[BHSM_CRC_DATA_LEN];

    /* In: a kind of program magic number, must be BCMD_OTP_DATASECTIONPROG_MODE (0x00010112).
            if it is not this value the command will be rejected by  a sanity check at BSP
     */

    uint32_t                    mode;

    /* Out: 0 for success, otherwise failed  */
    uint32_t                    unStatus;

} BHSM_ProgramDataSectIO_t;

/*****************************************************************************
Summary:

This function is used to programm one 32-byte data section value.

Description:

There are total of 8 32-byte data sections. This function shall be used to write one 32-byte data
section.


Calling Context:

This function can be called anytime after HSM is initialized.


Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_progDataSectIO  - BHSM_ProgramDataSectIO_t


Output:
inoutp_progDataSectIO  - BHSM_ProgramDataSectIO_t

See Also:
BHSM_ReadDataSect
*****************************************************************************/
BERR_Code BHSM_ProgramDataSect (
        BHSM_Handle                hHsm,
        BHSM_ProgramDataSectIO_t     *inoutp_progDataSectIO
);

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
  #define BHSM_MAX_MSP32_GROUP 71
  #define BHSM_MAX_MSP32_RANGE 40
#else
  #define BHSM_MAX_MSP32_GROUP 78
  #define BHSM_MAX_MSP32_RANGE 40
#endif


/**************************************************************************************************
Summary:

Description:
Structure that defines which OTP32 to read, and will hold the returned the data from that section

See Also:
BHSM_ReadMSP32
**************************************************************************************************/
typedef struct BHSM_ReadMsp32IO {

    /* In:  */
    BPI_Otp_MspGroupType_e mspGroupType; /*    -- Selects between Data and Lock. Must be of type BPI_Otp_MspGroupType_e */
    uint32_t unStartGroup;         /* -- A value between 0 and 78 for Zeus4*/
    uint32_t unRange;            /* -- A value between 1 and 40. */

    /* Out: 0 for success, otherwise failed */
    uint32_t                    unStatus;

    /* Out: the returned 32 bytes from the OTP data section as specified */
    uint32_t                *punMspValueGroup;

} BHSM_ReadMsp32IO_t;

/*****************************************************************************
Summary:

This function returns one 32-byte data section value.

Description:

There are total of 8 32-byte data sections. This function shall be used to read each 32-byte data section.


Calling Context:

This function can be called anytime after HSM is initialized.


Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_readDataSectIO  - BHSM_ReadDataSectIO_t


Output:
inoutp_readDataSectIO  - BHSM_ReadDataSectIO_t


See Also:
BHSM_ProgramDataSect
*****************************************************************************/
BERR_Code      BHSM_ReadMSP32 (
        BHSM_Handle                hHsm,
        BHSM_ReadMsp32IO_t    *inoutp_readMSP32IO
);

#endif /* #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1) */

/**************************************************************************************************
Summary:

Description:

See Also:
**************************************************************************************************/
typedef struct BHSM_AntiRollOverCounter
{

#if HSM_IS_ASKM_28NM_ZEUS_4_1
   BPI_Otp_RolloverCounterPartition_e partition;   /* Select the OTP partition  */
   BPI_Otp_RolloverCounterFlag_e flags;            /* Identifies host of customer specific partition. */
#else
   unsigned unused;
#endif
}BHSM_AntiRollOverCounter_t;



/**************************************************************************************************
Summary:

Description:

See Also:
**************************************************************************************************/
typedef struct BHSM_AntiRollOverCounterValue
{
   uint32_t count;                                 /* the counter value.  */

}BHSM_AntiRollOverCounterValue_t;


/*****************************************************************************
Summary:

This function read the count on a RolloverCounter. Only available on Zeus 4.1+

Description:


Calling Context:
This function can be called anytime after HSM is initialized.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or has failed.

Input:
hHsm      - BHSM_Handle, Host Secure module handle.
pCounter       - Identifies that counter to read

Output:
pCounterValue  - The returned Counter value


See Also:
BHSM_AntiIncrementRollOverCounter
*****************************************************************************/
BERR_Code BHSM_ReadAntiRollOverCounter(
      BHSM_Handle                      hHsm,
      BHSM_AntiRollOverCounter_t      *pCounter,      /* IN */
      BHSM_AntiRollOverCounterValue_t *pCounterValue  /* OUT */
);

/*****************************************************************************
Summary:

This function increments the Rollover Counter. Only available on Zeus 4.1+

Description:

Calling Context:

This function can be called anytime after HSM is initialized.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or has failed.

Input:
hHsm      - BHSM_Handle, Host Secure module handle.
pCounter       - Identifies the counter to increment

Output:
-

See Also:
BHSM_AntiReadRollOverCounter
*****************************************************************************/
BERR_Code BHSM_IncrementAntiRollOverCounter(
      BHSM_Handle                  hHsm,
      BHSM_AntiRollOverCounter_t  *pCounter        /* IN */
);

#ifdef __cplusplus
}
#endif

#endif /* BHSM_OTPMSP_H__ */
