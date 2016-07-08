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

#ifndef BAVC_VBI_H__
#define BAVC_VBI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The purpose of this common utility is to express VBI related data
structures that are shared between the BVBI porting interface module,
and the newer DTV software. This utility does not contain any functions.

These data structures define the storage of VBI data such as closed
caption and teletext.

****************************************************************************/

/*
 * Data structures
 */

/*****************************************************************************
  Summary:
    Structure for holding teletext data

  Description:
    The BAVC_VBI_TT_Line structure is a container for a single line of
    teletext data.
 *****************************************************************************/
#define BAVC_VBI_TT_MAX_LINESIZE 42
typedef struct BAVC_VBI_TT_Line
{
    uint8_t ucFramingCode;
    uint8_t aucData[BAVC_VBI_TT_MAX_LINESIZE];
}
BAVC_VBI_TT_Line;

/*****************************************************************************
  Summary:
    Structure for holding Gemstar data

  Description:
    The BAVC_VBI_GSData structure is a container for an entire field of Gemstar
    data. Note that when sending this data to the Gemstar encoder hardware,
    there are two conditions to observe:
    1. If the ulErrorLines field is non-zero, then the data will not be
       encoded. This is an error condition.
    2. If the Gemstar encoder hardware has been programmed in such a way that
       some of the data cannot be encoded, then none of it will be encoded.
       This is an error condition.
 *****************************************************************************/
#define BAVC_VBI_GS_MAX_LINES 5
typedef struct BAVC_VBI_GSData
{
    uint32_t ulDataLines;  /* A bitmask indicating the significance of the
                              following field ulData. If the ith "one" bit
                              in ulDataLines is in position j, then entry i
                              of ulData contains Gemstar data for video
                              line j.  For bottom field, add 263 to j. Note
                              that the lowest and higest several bits  of
                              this field should always be low.               */
    uint32_t ulErrorLines; /* A bitmask indicating which entries of the
                              following array ulData have errors.
                              Typically, these would be parity errors
                              reported by the Gemstar decoder hardware.
                              The  ith bit in ulErrorLines refers to the
                              ith entry in ulData. There is no
                              relationship with ulDataLines.                 */
    uint32_t ulData[BAVC_VBI_GS_MAX_LINES];
                           /* Each entry contains Gemstar data from one
                              video line in the VBI. Some of this data
                              may be in error. The valid data can be
                              located by ANDing ulDataLines with the ones'
                              complement of ulErrorLines.                    */

} BAVC_VBI_GSData;

/*****************************************************************************
  Summary:
    Structure for holding multi-line closed caption data

  Description:
    The BAVC_VBI_MCCData structure is a container for an entire field of
    multi-line closed caption data.
 *****************************************************************************/
typedef struct BAVC_VBI_MCCData
{
    uint16_t uhLineMask;      /* Bitmask indicating which video lines will
                                 (eventually) receive MCC data.  For any top
                                 field, the LSB refers to video line 8. For
                                 NTSC bottom field, the LSB refers to line
                                 271. For PAL bottom field, the LSB refers to
                                 line 321.  Due to hardware constraints, the
                                 number of bits in this value that may be set
                                 to 1 varies by chipset.                     */
    uint8_t  ucData[32];      /* 32 bytes dataL0, dataH0, ... dataL16,
                                 dataH16. DataL0 and dataH0 are the low byte
                                 and high byte (respectively) to be sent to
                                 the first video line indicated by the above
                                 bitmask.  DataL16 and dataH16 will be sent to
                                 the sixth video line indicated (if
                                 indicated) by the above bitmask.            */

} BAVC_VBI_MCCData;


/*****************************************************************************
  Summary:
    Structure for holding CGMS-B data

  Description:
    The BAVC_VBI_CGMSB_Datum structure is a container for a single line of
    CGMS-B data.
 *****************************************************************************/
typedef uint32_t BAVC_VBI_CGMSB_Datum[5];


/*****************************************************************************
  Summary:
    Structure for one field's VPS data

  Description:
    BAVC_VBI_VPSData is a holder for the 6 bytes of VPS data that can
    be processed in one video field.
 *****************************************************************************/
typedef struct
{
    uint8_t  ucByte05;
    uint8_t  ucByte11;
    uint8_t  ucByte12;
    uint8_t  ucByte13;
    uint8_t  ucByte14;
    uint8_t  ucByte15;

} BAVC_VBI_VPSData;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BAVC_VBI_H__ */

/* end of file */
