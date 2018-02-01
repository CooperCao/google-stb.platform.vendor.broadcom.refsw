/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *   See Module Overview below
 *
 ***************************************************************************/

#ifndef BVBI_H__
#define BVBI_H__

#include "bstd.h"
#include "breg_mem.h"
#include "bmma.h"
#include "bint.h"
#include "bchp.h"
#include "berr_ids.h"
#include "bfmt.h"
#include "bavc.h"
#include "bavc_vbi.h"
#include "bavc_hdmi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: change description of ALL parity variables to refer to the content
         of the associated BVBI_Field_Handle, not to the interrupt that caused
     the function to be called. */


/*=************************ Module Overview ********************************
<verbatim>

  Proposed:

  Tested:
  (Nothing)

                                    Overview
                                    --------

  The BVBI module controls access to the chip's core VBI registers.  It
  permits both encoding and decoding data in the Closed Captioning,
  CGMS, WSS and Teletext formats for SD output modes.  It may (or may
  not) permit HD output of CGMS data in 1080i, 720p and 480p HD video
  format.

                                    Design
                                    ------

The usage of the VBI API can be divided into three parts:
 o Initializing the VBI module
 o Setting up recurring VBI encoding and decoding tasks
 o Performing VBI encoding and decoding

User programming of the first two parts will typically occur in a
normal (as opposed to interrupt) context of execution.  The third
part will typically occur in the context of an interrupt.

The above three parts of the API are described in more detail below,
in separate sections.

Initializing The VBI Module
---------------------------

The API defines a data type BVBI_Handle that controls the state
of all VBI encoding and decoding hardware on the chip.  The user
should create a single BVBI_Handle for the entire chip by calling
BVBI_Open() when the software application is launched.

The API also provides a function BVBI_Close() that frees resources
that were reserved by BVBI_Open().  Should the user wish to reset the
VBI hardware, he should call BVBI_Close() and then call BVBI_Open()
again.

After the VBI module is initialized through a call to BVBI_Open(), the
user should create a small collection of data types BVBI_Field_Handle.
Each BVBI_Field_Handle can contain VBI data for a single field of
video.  A BVBI_Field_Handle can be used for encoding and decoding VBI
data.  A single BVBI_Field_Handle may contain several types of VBI
data (for example, both Closed Caption data and Teletext data).

An empty BVBI_Field_Handle data type is created by calling
function BVBI_Field_Create().  It is destroyed by calling function
BVBI_Field_Destroy().

When VBI data is decoded by the chip hardware, it is placed
in a BVBI_Field_Handle.  The API provides several functions for
extracting this data, such as BVBI_Field_GetCCData(), which extracts
16 bits of closed caption data.  The API also provides a function
BVBI_Field_GetErrorInfo_isr() that discloses any errors that the hardware
detected while attempting to decode VBI data from a field of video.

When VBI data is encoded by the chip hardware, this data must come
from a BVBI_Field_Handle.  This BVBI_Field_Handle may have been
filled with VBI data by a previous call to BVBI_Field_Decode.
The user may modify this VBI data (or create it "from scratch")
by calling one of several provided API functions.  For example,
function BVBI_Field_SetCGMSAData() will place CGMS data into a
BVBI_Field_Handle.

When working with closed caption, CGMS, and WSS, the manipulations of
BVBI_Field_Handles are straight forward.  For all other forms of VBI
data, a level of indirection is involved.  This is because the other
forms of VBI data can be fairly large in size, and the designers
did now wish to burden applications with the (memory) expense of
supporting these forms if the capability is not in use.  To cite a
particular example, the BVBI_Field_Handle is not capable of holding
teletext data when the handle is first created.  If the user expects
to work with teletext, he must modify his BVBI_Field_Handle(s)
by calling BVBI_Field_TT_Allocate().  This function effectively
"enlarges" a BVBI_Field_Handle so that it can hold a user-specified
amount of teletext data.  When the user wishes to return memory
resources to the system, he simply calls BVBI_Field_Destroy().
The user takes no special action to free the "extra" resources
required for teletext data, BVBI_Field_Destroy() does it all.

Setting Up Recurring VBI Encoding And Decoding Tasks
----------------------------------------------------

The user expresses which VBI encoding and decoding operations he
wants by creating and modifying two data types BVBI_Encode_Handle
and BVBI_Decode_Handle.  These data types are created by calling
functions BVBI_Encode_Create() and BVBI_Decode_Create(),
respectively.  These data types are always associated with
a BVBI_Handle data type.  Several BVBI_Encode_Handles and
BVBI_Decode_Handles may be associated with a single BVBI_Handle.
Each BVBI_Encode_Handle is associated with a unique video destination
when created.  Similarly, each BVBI_Decode_Handle is associated with
a unique video source when created.  The software will not allow two
BVBI_Encode_Handles to be associated with the same video destination.
Similarly, the software will not allow two BVBI_Decode_Handles to
be associated with the same video source.

The API also provides function BVBI_Encode_Destroy() and
BVBI_Decode_Destroy() to release resources allocated by
BVBI_Encode_Create() and BVBI_Decode_Create().  Note that a call
to BVBI_Close() will fail if its BVBI_Handle argument has any open
BVBI_Encode_Handles or BVBI_Decode_Handles associated with it.
In other words, the user must shut down all 'code_Handles before
attempting to shut down (or re-open) the entire VBI module.

After a 'code_Handle is created, it is modified to express what
sort of coding the user desires.  Some examples:
 o Calling function BVBI_Decode_SetCGMS() causes a BVBI_Decode_Handle
   to enable subsequent decoding of CGMS data.
 o Calling function BVBI_Encode_SetVideoFormat() will cause any
   subsequent VBI encoding operations to assume a specific video
   format (PAL or NTSC).

The collection of functions that are described in the previous
paragraph do NOT change the operation of the chip hardware.
They serve to record the user's wishes.  When the user has finished
calling these functions, his recipe for VBI encoding and/or decoding
is complete.  At that time, the user should call the functions
BVBI_Decode_ApplyChanges() and/or BVBI_Encode_Apply_Changes()
to commit the software settings to chip hardware.

The function BVBI_Decode_Apply_Changes() takes a BVBI_Decode_Handle
as its argument.  All decode operations expressed by the and
BVBI_Decode_Handle are committed to chip hardware by calling
BVBI_Decode_ApplyChanges().

Similarly, the function BVBI_Encode_Apply_Changes() takes a
BVBI_Encode_Handle as its argument and modifies the operation of
the VBI encoding hardware within the chip.

Performing VBI Encoding And Decoding
------------------------------------

Typically, this will be the main "real time" bit of programming
involving the VBI API.  It is accomplished by calling two functions
BVBI_Encode_Data_isr() and BVBI_Decode_Data_isr().  As their names
suggest, these functions are designed to be called in interrupt
context.  The chip hardware provides one interrupt that marks the
start of active video in a field of video being encoded, and another
interrupt that marks the same event in decoded video.  These are
the interrupts from which te two functions should be called.

Calling BVBI_Encode_Data_isr() provides VBI data to the hardware for
encoding in the next field of specified parity (the caller states
this parity: even or odd field).  Therefore, the call chould be made
before the field of desired parity begins.  The user is encouraged to
use the interrupt that marks the start of active video in a field.
As a result, the user would typically call BVBI_Encode_Data_isr()
with even field data when the interrupt for (start active video;
odd field) fires.

Calling BVBI_Decode_Data_isr() reads VBI data from the hardware for
a field of specified parity that has already occurred.  Therefore,
the call should be made after the field of desired parity ends.
The user is encouraged to use the interrupt that marks the start of
actived video in a field.  As a result, the user would typically
call BVBI_Decode_Data_isr() requesting even field data when the
interrupt for (start active video; even field) fires.

The functions BVBI_Decode_Data_isr() and BVBI_Encode_Data_isr()
both use the data type BVBI_Field_Handle as a container for the VBI
data to be encoded or decoded.  It is the user's responsibility
to protect these data objects from concurrent use during the two
interrupt contexts (start active video decode and start active video
encode).  The user must also prevent access conflicts occurring
between interrupt and non interupt context.  Typically, this would
mean managing an array of BVBI_Field_Handle objects that are all
allocated before VBI processing begins.

Note that these two functions are the only ones in the API that
are safe to call from interrupt context.

The VBI API provides a collection of functions that put VBI
data into a BVBI_Field_Handle object, or take it back out.
These functions include (for example) BVBI_Field_GetCGMSAData()
and BVBI_Field_SetWSSData().  There is BVBI_Field_GetErrorInfo_isr(),
for obtaining detailed information about a failure to encode or
decode VBI data.

                        Usage/Sample Code
                        -----------------


Initializing The VBI Module
---------------------------

The following should be performed when the chip is initialized:

    // Error code, should be checked after calling API functions
    BERR_Code eErr;

    // Provided by other porting interface initialization functions:
    BCHP_Handle chipHandle;
    BREG_Handle regHandle;
    BMMA_Heap_Handle hMmaHeap;

    // Which video decoder (VDEC) to use.
    // Can be queried from the VDC module
    BAVC_SourceId eSource;

    // Which video decoder (VEC) to use.
    // Can be queried from the VDC module
    BAVC_VbiPath eDestination;

    // Result of initializing VBI API, save these for future use
    BVBI_Handle vbiHandle;
    BVBI_Field_Handle fieldHandle[16];
    BVBI_Decode_Handle decodeHandle;
    BVBI_Encode_Handle decodeHandle;

    // Initialize the entire VBI module
    eErr = BVBI_Open (&vbiHandle, chipHandle, regHandle, hMmaHeap, NULL);

    // Allocate some containers for VBI data
    for (index = 0 ; index < 16 ; ++index)
    {
        eErr = BVBI_Field_Create (&fieldHandle[index], vbiHandle);
    }

    // Allocate rules for encoding and decoding actions
    eErr = BVBI_Decode_Create (vbiHandle, &decodeHandle, eSource);
    eErr = BVBI_Encode_Create (vbiHandle, &encodeHandle, eDestination);

Setting Up Recurring VBI Encoding And Decoding Tasks
----------------------------------------------------

This code will be executed whenever the rules for VBI encode/decode
change.  Most of the variables are those initialized in the previous
section.

    // Express that closed caption data and CGMS data are to be
    // decoded
    eErr = BVBI_Decode_SetCC   (decodeHande, true);
    eErr = BVBI_Decode_SetCGMS (decodeHande, true);

    // Express that the relevant VDEC is processing NTSC video
    eErr = BVBI_Decode_SetVideoFormat (decodeHandle, BFMT_VideoFmt_eNTSC);

    // Apply all the above decoder settings to chip hardware
    eErr = BVBI_Decode_ApplyChanges (decodeHandle);

    // On the encoding side, we will only output closed caption data
    eErr = BVBI_Encode_SetCC   (encodeHande, true);
    eErr = BVBI_Encode_SetVideoFormat (encodeHandle, BFMT_VideoFmt_eNTSC);

    // Apply all the above encoder settings to chip hardware
    eErr = BVBI_Encode_ApplyChanges (decodeHandle);

Performing VBI Encoding And Decoding
------------------------------------

This code will be executed in the context of the interrupt that marks
the start of active video for incoming (decoded) video.

    // Which field data object to put data into.  The user must manage
    // this to avoid concurrent access to data by different contexts of
    // execution.
    int index;

    // Specific decoding error conditions (bitmask)
    uint32_t errInfo;

    // Some VBI data extracted from hardware
    uint32_t CGMSAData;

    // Even/odd field parity, can be obtained from the VDC module.
    BAVC_Polarity polarity;

    // The data that is ready is not the data that is currently being
    // decoded.  The data that is ready is from the PREVIOUS field.
    if (polarity == BAVC_Polarity_eTopField)
        polarity = BAVC_Polarity_eBottomField;
    else if (polarity == BAVC_Polarity_eBottomField)
        polarity = BAVC_Polarity_eTopField;
    // If polarity == BAVC_Polarity_eFrame, let it be.

    // Pull closed caption data and CGMS data from VDEC hardware
    eErr = BVBI_Decode_Data_isr (vbiHandle, fieldHandle[index], polarity);

    // Check for decoding errors.  Depending on results, the rest of
    // the code in this section may have to be skipped
    // (not shown here)
    eErr = BVBI_Field_GetErrorInfo_isr (fieldHandle[index], &errInfo);
    if (eErr != BERR_Success)
    {
        // Check for conditions such as BVBI_LINE_ERROR_CC_OVERRUN
        // defined in bvbi.h
    }

    // Extract the CGMS data (if any) for use by customer's application
    eErr = BVBI_Field_GetCGMSAData (fieldHandle[index], &CGMSAData);

This code will be executed in the context of the interrupt that marks
the start of active video for outgoing (encoded) video.

    // Which field data object to use as source of VBI data.  The user
    // must manage this to avoid concurrent access to data by
    // different contexts of execution.
    int index;

    // Specific decoding error conditions (bitmask)
    uint32_t errInfo;

    // Even/odd field parity, can be obtained from the VDC module.
    BAVC_Polarity polarity;

    // The VBI data that is about to be encoded must be ready before
    // its field is created by hardware.  Thus, if the interrupt has
    // just fired for an odd field, it is too late to encode odd field
    // VBI data.  We would provide some even field data instead, which
    // will be encoded in the NEXT field to be created.
    if (polarity == BAVC_Polarity_eTopField)
        polarity = BAVC_Polarity_eBottomField;
    else if (polarity == BAVC_Polarity_eBottomField)
        polarity = BAVC_Polarity_eTopField;
    // If polarity == BAVC_Polarity_eFrame, let it be.

    // Encode VBI data.  For this example, assume that the provided
    // BVBI_Field_Handle (argument 2) was filled in by a previous call
    // to BVBI_Decode_Data_isr().  The value of "index" must be
    // computed by the user to point to the right set of VBI data.
    eErr = BVBI_Encode_Data_isr (vbiHandle, fieldHandle[index], polarity);


                            Miscellaneous
                            -------------

When teletext is used with PAL video, the porting interface supports ETSI 300
706 "Enhanced" teletext. In particular, the teletext data may be placed on
lines 6-22 on the top field, and lines 318 through 335 on the bottom field.
This is relevant when calling these functions:
    BVBI_Field_GetTTData_isr()
    BVBI_Field_SetTTData_isr()

</verbatim>
***************************************************************************/

/* Constants */

/* The maximum number of lines of teletext data that is supported.  Also
   determines the space required to get and set teletext data by certain
   functions. */
#define BVBI_TT_MAX_LINES 18

/* The maximum number of characters per line of teletext data that is
   supported.  Also determines the space required to get and set teletext
   data by certain functions. */
#define BVBI_TT_MAX_LINESIZE BAVC_VBI_TT_MAX_LINESIZE
#define BVBI_TT_MAX_LINELENGTH (BAVC_VBI_TT_MAX_LINESIZE + 1)

/* The framing codes for teletext lines, for NTSC and PAL.  The final symbol is
   to indicate that no valid data is on the line. */
#define BVBI_TT_NABTS_FRAMING_CODE      0xE7
#define BVBI_TT_ETS_FRAMING_CODE        0x27
#define BVBI_TT_ETS_INVERTED_FRAMING_CODE   0xE4
#define BVBI_TT_INVALID_FRAMING_CODE        0xFF

/* The maximum number of lines of Gemstar data that is supported.  Also
   determines the space required to get and set Gemstar data by certain
   functions. */
#define BVBI_GS_MAX_LINES BAVC_VBI_GS_MAX_LINES

/* Structures */

/*****************************************************************************
  Summary:
    Structure for initial VBI settings

  Description:
    The BVBI_Settings structure is used by the VBI API to set up
    a VBI Handle initially.  It is initialized with
    BVBI_GetDefaultSettings, and passed to BVBI_Open().

  TODO:  document how interrupts work, VPS format, List of display formats
    for each VBI format, EIA document numbers

  See Also:
    BVBI_Open, BVBI_GetDefaultSettings
 *****************************************************************************/
typedef struct BVBI_Settings
{
    size_t   in656bufferSize;   /* Size of capture buffer (in bytes) for
                                   ancillary data packets captured from
                                   incoming ITU-R 656 digital video.         */
    bool  tteShiftDirMsb2Lsb;   /* If true, teletext encoder shift direction
                                   will be set to MSBToLSB. Otherwise, it
                                   will be set to LSBToMSB. The default
                                   value is FALSE.                           */
}
BVBI_Settings;

/***************************************************************************
Summary:
    This structure describes the capabilities of a BVBI

Description:
    BVBI_Capabilities is a structure that contains the capabilities of BVBI.

See Also:
    BVBI_GetCapabilities
***************************************************************************/
typedef struct BVBI_Capabilities
{
    uint32_t ulNumAmole;     /* Number of AMOL encoder cores, not including
                                ITU-R 656. */
    uint32_t ulNumAmole656;  /* Number of AMOL encoder cores that are
                                specifically ITU-R 656. */
    uint32_t ulNumCce;       /* Number of closed caption encoder cores, not
                                including ITU-R 656. */
    uint32_t ulNumCce656;    /* Number of closed caption encoder cores that are
                                specifically for ITU-R 656 output. */
    uint32_t ulNumCgmse;     /* Number of CGMS encoder cores, not including
                                ITU-R 656. */
    uint32_t ulNumCgmse656;  /* Number of CGMS encoder cores that are
                                specifically for ITU-R 656 output. */
    uint32_t ulNumGse;       /* Number of Gemstar encoder cores, not including
                                ITU-R 656. */
    uint32_t ulNumGse656;    /* Number of Gemstar encoder cores that are
                                specifically for ITU-R 656 output. */
    uint32_t ulNumTte;       /* Number of teletext encoder cores. Does not
                                include 656 (bypass) cores. */
    uint32_t ulNumTte656;    /* Number of 656 teletext encoder cores. */
    uint32_t ulNumWssE;      /* Number of WSS/VPS cores, not including
                                ITU-R 656. */
    uint32_t ulNumWssE_656;  /* Number of WSS/VPS cores that are specifically
                                for ITU-R 656 output. */
    bool     bTteHasSerO;      /* TTX core has serial output capability. */
    uint32_t ulNumVecEnc;      /* Number of VEC VBI encoder cores */
    uint32_t ulNumPassThruEnc; /* Number of 656 VBI encoder cores */

} BVBI_Capabilities;

/* See bavc_vbi.h */
#define BVBI_TT_Line BAVC_VBI_TT_Line

/*****************************************************************************
  Summary:
    Structure for holding Gemstar options

  Description:
    The BVBI_GSoptions structure contains options controlling Gemstar
    waveforms.

  See Also:
    BVBI_Encode_SetGemstarOptions
 *****************************************************************************/
typedef struct BVBI_GSOptions
{
    bool           bTvg2x; /* If true, the TVG2X waveform specified in
                              CEA-2020 will be used. If false, the waveform
                              specified in the proprietary Gemstar standard
                              will be used.                                  */
    uint16_t baseline_top; /* The video line number corresponding to the
                              first bit in the following bit mask.           */
    uint16_t linemask_top; /* Bit mask that indicates which video lines
                              carry Gemstar data in top video fields.        */
    uint16_t baseline_bot; /* The video line number corresponding to the
                              first bit in the following bit mask. Must be
                              greater than 256.                              */
    uint16_t linemask_bot; /* Bit mask that indicates which video lines
                              carry Gemstar data in bottom video fields.     */
}
BVBI_GSOptions;
#define BVBI_GSOptions_Version (1)

/* See bavc_vbi.h */
#define BVBI_GSData      BAVC_VBI_GSData
#define BVBI_MCCData     BAVC_VBI_MCCData
#define BVBI_CGMSB_Datum BAVC_VBI_CGMSB_Datum
#define BVBI_VPSData     BAVC_VBI_VPSData

/*****************************************************************************
  Summary:
    Handle for VBI module

  Description:
    The BVBI_Handle, once opened, represents VBI activity of some
    kind on the part of the system.  Usually it indicates that VBI
    registers are being set and that VBI output is occurring for at
    least one output.  It can be allocated with a call to BVBI_Open.
    It is passed as the first argument to a number of BVBI
    functions, and a BVBI_Encode_Handle or BVBI_Decode_Handle
    may be opened from it.

  See Also:
    BVBI_Open, BVBI_Encode_Create, BVBI_Decode_Create
 *****************************************************************************/
typedef struct BVBI_P_Handle *BVBI_Handle;  /* Opaque */


/*****************************************************************************
  Summary:
    Handle for VBI encoding functionality

  Description:
    The BVBI_Encode_Handle is used to control how VBI encoding is done,
    and in what formats.  It is used by a number of encoding-related
    functions.  It is allocated by BVBI_Encode_Create.  Encoding
    of specific data is controlled by BVBI_Encode_Data_isr.  Format
    output is controlled by BVBI_Encode_SetCC and related functions.

  See Also:
    BVBI_Encode_Create, BVBI_Decode_Create, BVBI_Encode_Data_isr,
    BVBI_Encode_SetCC
 *****************************************************************************/
typedef struct BVBI_P_Encode_Handle *BVBI_Encode_Handle;  /* Opaque */


/*****************************************************************************
  Summary:
    Options for creation of BVBI Encoder object

  Description:
    The BVBI_Encode_Settings is used to select options that will always
    be in effect for a specified BVBI Encoder object.

  See Also:
    BVBI_Encode_Create, BVBI_Encode_GetDefaultSettings
 *****************************************************************************/
 typedef struct
 {
    bool bArib480p;   /* 480I and 480P video output will follow ARIB
                         convention. This sestting must agree with the
                         equivalent setting in the BVDC porting interface
                         module. Default is "false."                         */
    bool bSupportTeletext;
                      /* It true, the BVBI encoder object will support
                         teletext output. This has a cost: two teletext
                         data structures of maximal size are allocated
                         immediately. Default is "true."                     */
 }
 BVBI_Encode_Settings;


/*****************************************************************************
  Summary:
    Handle for VBI decoding functionality

  Description:
    The BVBI_Decode_Handle is used to control how VBI decoding is done, and in
    what formats.  It is used by a number of decoding-related functions.
    It is allocated by BVBI_Decode_Create.  Decoding of specific data is
    controlled by BVBI_Decode_Data_isr.  Format output is controlled by
    BVBI_Decode_SetCC and related functions.

  See Also:
    BVBI_Encode_Create, BVBI_Decode_Create, BVBI_Decode_Data_isr,
    BVBI_Decode_SetCC
 *****************************************************************************/
typedef struct BVBI_P_Decode_Handle *BVBI_Decode_Handle;  /* Opaque */


/*****************************************************************************
  Summary:
    Structure for one field's VBI data

  Description:
    BVBI_Field_Handle can be created by BVBI_Field_Create and filled
    in by BVBI_Decode_Data_isr or BVBI_Field_Set_*Data.  The data
    can be read by functions like BVBI_Field_Get*Data.  It can be
    passed to BVBI_Encode_Data_isr to set the current output for the
    formats that it represents.

    Note that the BVBI_Field_Handle is not related to any
    BVBI_Decode_Handle it may have been read from.  The data
    may be used with other handles and read and written freely
    no matter what BVBI_Encode_Handle or BVBI_Decode_Handle
    may have been opened or closed.

    Note also that the BVBI_Field_Handle is not related to any
    BVBI_Handles.  A BVBI_Field_Handle is simply a container for VBI
    data that can occur in a single field of video.

  See Also:
    BVBI_Encode_Data_isr, BVBI_Decode_Data_isr, BVBI_Field_SetCCData,
    BVBI_Field_GetCCData.
 *****************************************************************************/
typedef struct BVBI_P_Field_Handle *BVBI_Field_Handle;  /* Opaque */


/* Enumerations and Constants */

/* Identifies which variant of the AMOL specification to use */
typedef enum BVBI_AMOL_Type
{
    BVBI_AMOL_Type_None=0,      /* No AMOL. Used to disable a feature, etc. */
    BVBI_AMOL_Type_I,           /* AMOL I                                   */
    BVBI_AMOL_Type_II_Lowrate,  /* AMOL II, 1 Mbit/s.                       */
    BVBI_AMOL_Type_II_Highrate  /* AMOL II, 2 Mbit/s.                       */

} BVBI_AMOL_Type;

/*
 * Programming note: the above must be in order of AMOL data size per video
 * ----------------  field
 */

/* Identifies which variant of the SCTE specification to use */
typedef enum BVBI_SCTE_Type
{
    BVBI_SCTE_Type_NONE=0,      /* No SCTE. Used to disable a feature, etc. */
    BVBI_SCTE_Type_CCONLY,      /* Closed caption like data only.           */
    BVBI_SCTE_Type_CCNRTV,      /* Closed caption like and NRTV data.       */
    BVBI_SCTE_Type_CCPAM,       /* Closed caption like and PAM data.        */
    BVBI_SCTE_Type_CCMONO,      /* Closed caption like and monochrome data. */
    BVBI_SCTE_Type_LAST         /* Do not use this!                         */

} BVBI_SCTE_Type;

/* The SCTE encoder core must "know" what colorspace is in use. */
typedef enum BVBI_CSC {
    BVBI_CSC_SCTE_MYIQ=0,
    BVBI_CSC_SCTE_NTSCYIQ=1,
    BVBI_CSC_SCTE_YUV=2,
    BVBI_CSC_SCTE_CO=3,
    BVBI_CSC_NONE=4
} BVBI_CSC;

#define BVBI_SCTE_MAX_ITEMS 32

typedef struct BVBI_SCTE_CC_Data
{
    uint8_t valid;
    uint8_t priority;
    uint8_t line_number;    /* Between 10 and 41, inclusive. */
    uint8_t cc_data_1;
    uint8_t cc_data_2;

} BVBI_SCTE_CC_Data;

typedef struct BVBI_SCTE_NRTV_Data
{
    uint8_t valid;
    uint8_t priority;
    uint8_t line_number;    /* Between 10 and 41, inclusive. */
    uint8_t sequence_number;
    uint8_t segment_number;
    uint8_t y_data[32];
    uint8_t cbcr_data[32];

} BVBI_SCTE_NRTV_Data;

typedef struct BVBI_SCTE_Mono_Data
{
    bool first_segment_flag;
    bool last_segment_flag;
    uint8_t line_number;    /* Between 10 and 41, inclusive. */
    uint16_t first_pixel_position;
    uint8_t n_pixels;
    uint8_t Y_data[256];

} BVBI_SCTE_Mono_Data;


/*****************************************************************************
  Summary:
    Structure for holding SCTE data

  Description:
    The BVBI_SCTE_Data structure is a container for an entire field of
    SCTE data.

    The BVBI_SCTE_Data structure contains _size elements, _count
    elements, and data pointers.

    The user is responsible for setting the data pointers to
    reasonable values. However, if the corresponding _count
    element is zero, then BVBI software will not make use of
    the data pointer. BVBI software does not allocate or free
    memory indicated by the data pointers in any case.

    The _size elements are for the convenience of the user,
    they are not used by BVBI software.

    The _count elements indicate the amount of data contained
    in the corresponding data pointers. The units of the _count
    elements vary, but they are defined below.

  See Also:
    BVBI_Field_GetSCTEData_isr, BVBI_Field_SetSCTEData_isr
 *****************************************************************************/
typedef struct BVBI_SCTE_Data
{
    size_t cc_count;                 /* Number of meaningful entries in the
                                        following cc_data array.             */
    size_t cc_size;                  /* Capacity of following cc_data array,
                                        in units of BVBI_SCTE_CC_Data.       */
    BVBI_SCTE_CC_Data* cc_data;      /* The closed caption data. If zero,
                                        then no space has been allocated.    */
    size_t nrtv_count;               /* Number of meaningful entries in the
                                        following nrtv_data array. Must be
                                        zero or one.                         */
    size_t nrtv_size;                /* Capacity of following nrtv_data
                                        array, in units of
                                        BVBI_SCTE_NRTV_Data                  */
    BVBI_SCTE_NRTV_Data* nrtv_data;  /* The NRTV data. If zero, then no
                                        space has been allocated.            */
    size_t pam_count;                /* Number of meaningful bytes in the
                                        following luma_PAM_data buffer. The
                                        maximum value allowed is 4095.       */
    size_t pam_size;                 /* Capacity of following luma_PAM_data
                                        buffer, in bytes. If zero, then no
                                        space has been allocated.            */
    uint8_t* luma_PAM_data;          /* Buffer containing one piece of
                                        luma_PAM_data, as defined in SCTE-21
                                        section 5.5.                         */
    size_t mono_count;               /* Number of meaningful entries in the
                                        following mono_data array. Must be
                                        zero, one, or two.                   */
    size_t mono_size;                /* Capacity of following mono_data
                                        array, in units of
                                        BVBI_SCTE_Mono_Data                  */
    BVBI_SCTE_Mono_Data* mono_data;  /* The monochrome data. If zero, then
                                        no space has been allocated.         */
    uint8_t field_number;            /* Field number, from SCTE data
                                        structure. In the case of SCTE 20
                                        ATSC CC data, it is field_type. In
                                        the case of monochrome data, this is
                                        the field_parity element.            */
} BVBI_SCTE_Data;

/* The supported methods for encapsulating VBI data into ITU-R 656
   (digital) bitstreams */
typedef enum BVBI_656Fmt
{
    BVBI_656Fmt_SAA7113 = 0,
    BVBI_656Fmt_Modified_SAA7113,
    BVBI_656Fmt_SAA7114,
    BVBI_656Fmt_SAA7114A,
    BVBI_656Fmt_SAA7114B,
    BVBI_656Fmt_SAA7114C,
    BVBI_656Fmt_SAA7115,
    BVBI_656Fmt_SMPTE291,
    BVBI_656Fmt_LAST

} BVBI_656Fmt;

/* An enum specifying type of VBI data encountered */
typedef enum BVBI_656_VbiType
{
    BVBI_656_VbiType_None = 0,  /* No VBI data present */
    BVBI_656_VbiType_EuroCC,    /* European closed caption data */
    BVBI_656_VbiType_WSS,       /* WSS data */
    BVBI_656_VbiType_USCC,      /* U.S. Closed caption data */
    BVBI_656_VbiType_TT,        /* (European) Teletext data */
    BVBI_656_VbiType_NABTS,     /* (U.S.) NABTS data */
    BVBI_656_VbiType_VPS,       /* VPS data */
    BVBI_656_VbiType_GS,        /* Gemstar data */
    BVBI_656_VbiType_AMOL,      /* AMOL data */
    BVBI_656_VbiType_MCC,       /* Multi-line, closed caption data */
    BVBI_656_VbiType_SCTE       /* SCTE data */

} BVBI_656_VbiType;

/* Information necessary to use parsed SMPTE 291M data */
typedef struct
{
    BVBI_656_VbiType vbiType;   /* Defined above */
    BAVC_Polarity polarity;
    int lineNumber;             /* Video line, enter 0 for "don't care" */

} BVBI_SMPTE291M_Description;

/*****************************************************************************
  Summary:
    Enumeration for expressing choice of data for teletext serial port output.

  Description:
    For chipsets that are capable of sending teletext data to an auxiliary
    serial port, there is a choice of what data to send to the port. This
    enumeration is a list of those choices.

  See Also:
    BVBI_Open, BVBI_GetDefaultSettings
 *****************************************************************************/
typedef enum BVBI_XSER_DataContent
{
    BVBI_TTserialDataContent_None,         /* No data. Disable the serial
                                              port feature. */
    BVBI_TTserialDataContent_DataOnly,     /* Output only the teletext data. */
    BVBI_TTserialDataContent_DataMag,      /* Output teletext data and
                                              magazine. */
    BVBI_TTserialDataContent_DataMagFrm,   /* Output teletext data  magazine
                                              and frame code. */
    BVBI_TTserialDataContent_DataMagFrmRun /* Output teletext data magazine
                                              frame code  and run-in
                                              sequence. */
} BVBI_XSER_DataContent;

/*****************************************************************************
  Summary:
    Enumeration for expressing choice of control for output of data to
    teletext serial port.

  Description:
    For chipsets that are capable of sending teletext data to an
    auxiliary serial port, there is a choice of methods to control
    the output. The serial teletext output may occur at a certain
    stage of ITU-R 656 digital video output, or the output may
    be sent in response to an electrical signal from an external
    device. This enumeration is a list of those choices.

  See Also:
    BVBI_Open, BVBI_GetDefaultSettings
 *****************************************************************************/
typedef enum BVBI_TTserialDataSync
{
    BVBI_TTserialDataSync_EAV,  /* Output synchronized with video EAV.  */
    BVBI_TTserialDataSync_SAV,  /* Output synchronized with video SAV.  */
    BVBI_TTserialDataSync_RQ    /* Output synchronized with external RQ
                                   (TTX request) signal.                */

} BVBI_TTserialDataSync;

/*****************************************************************************
  Summary:
    Structure for initial VBI encoder settings

  Description:
    The BVBI_XSER_Settings structure is used by the VBI API to set up
    the external UART (used with teletext output).  It can be initialized with
    BVBI_Encode_XSER_GetOptions, and passed to BVBI_Encode_XSER_SetOptions().

  See Also:
    BVBI_Encode_Create, BVBI_Encode_GetDefaultSettings
 *****************************************************************************/
typedef struct BVBI_XSER_Settings
{
    BVBI_XSER_DataContent
        xsSerialDataContent;    /* Specify serial output of data.  Ignored
                                   if chipset does not have this capability. */
    BVBI_TTserialDataSync
        ttSerialDataSync;       /* Specify timing of serial output of
                                   teletext data. Ignored if chipset does
                                   not have this capability.                 */
   uint16_t
    iTTserialDataSyncDelay;     /* Specify delay count (27 MHz cycles) for
                                   serial output of teletext data. Ignored
                                   if chipset does not have this capability. */
}
BVBI_XSER_Settings;
#define BVBI_XSER_Settings_Version (1)

/*****************************************************************************
  Summary:
    Prototype for callback that provides the parsing mechanism for incoming
    SMPTE 291M packets.

  Description:
    SMPTE 291M is a standard for encapsulating user data in
    ancillary data packets for inclusion in an ITU-R 656 bitstream.
    SMPTE 291M is an "open" standard that allows the user
    data to be expressed in a variety of ways.  The user must
    provide a callback function to parse incoming data
    according to the user's method.  This is the function prototype
    for such a callback function.

  Returns:
    BERR_SUCCESS for success; any other value will cause data to be ignored.

 *****************************************************************************/
typedef BERR_Code (*BVBI_Decode_656_SMPTE291M_Cb) (
    void*         arg0, /*  [in] The "arg0"value provided to
                                 BVBI_Decode_656_SetSMPTE291Moptions().      */
    BFMT_VideoFmt
          eVideoFormat, /*  [in] Video format of input 656 port              */
    BAVC_Polarity
              polarity, /*  [in] The field polarity according to BVBI
                                 interrupts.                                 */
    uint8_t    data_id, /*  [in] The first identification field in the
                                 SMPTE 291M packet header.                   */
    uint8_t  second_id, /*  [in] The second identification field in the
                                 SMPTE 291M packet header.                   */
    uint8_t data_count, /*  [in] The DATA COUNT field in the SMPTE 291M
                                 packet header.                              */
    uint8_t*    packet, /*  [in] The SMPTE 291M ancillary data packet,
                                 starting with the first byte of the
                                 payload.  Note that the three header bytes
                                 have already been parsed by the BVBI
                                 software when this callback function is
                                 called.  The final checksum byte has also
                                 been read and verified by the BVBI
                                 software and is not available to this
                                 callback function.                          */
    uint8_t**  vbiData, /* [out] The parsed VBI data.  For example, if the
                                 SMPTE 291M packet contains closed caption
                                 data, then this argument should be returned
                                 with the two bytes of closed caption data.
                                 This data can overwrite the data provided
                                 as the argument "packet" if desired.        */
    BVBI_SMPTE291M_Description*
        pktDesc         /* [out] Describes the above argument vbiData.       */
);

/*****************************************************************************
  Summary:
    Structure for VBI standby settings

  See Also:
    BVBI_Standby, BVBI_Resume
 *****************************************************************************/
typedef struct BVBI_StandbySettings
{
    bool dummy; /* placeholder */
} BVBI_StandbySettings;

/*****************************************************************************
    Module specific standard BERR codes
 *****************************************************************************/
/* This error indicates that field data has a problem */
#define BVBI_ERR_FIELD_BADDATA          BERR_MAKE_CODE(BERR_VBI_ID, 0x0000)
/* This error field data does not exist */
#define BVBI_ERR_FIELD_NODATA           BERR_MAKE_CODE(BERR_VBI_ID, 0x0001)
/* This error indicates that the the user attempted to use multiple handles to
   control a single encoder or decoder core */
#define BVBI_ERR_HW_CONFLICT            BERR_MAKE_CODE(BERR_VBI_ID, 0x0002)
/* This error indicates that the user attempted to set/get data from a field
   handle that does not have the right allocation */
#define BVBI_ERR_FLDH_CONFLICT          BERR_MAKE_CODE(BERR_VBI_ID, 0x0003)
/* This error indicates that the video format in use is not supported */
#define BVBI_ERR_VFMT_CONFLICT          BERR_MAKE_CODE(BERR_VBI_ID, 0x0004)
/* This error indicates that the user attempted to encode VBI data with the
   wrong field polarity */
#define BVBI_ERR_INVALID_FIELD_POLARITY BERR_MAKE_CODE(BERR_VBI_ID, 0x0005)
/* This error indicates unrecognizable ITU-R 656 ancillary packet data */
#define BVBI_ERR_656_PARSE              BERR_MAKE_CODE(BERR_VBI_ID, 0x0006)
/* This error indicates that the user attempted to use a single VBI encoder
   or decoder core to do two different jobs. Example: closed caption and
   multi-line, closed caption data. */
#define BVBI_ERR_CORE_CONFLICT          BERR_MAKE_CODE(BERR_VBI_ID, 0x0007)
/* This error indicates that the user attempted to use a feature that is
 * not supported by the chip hardware. */
#define BVBI_ERR_HW_UNSUPPORTED         BERR_MAKE_CODE(BERR_VBI_ID, 0x0008)

/*****************************************************************************
    Error constants for BVBI_Field_GetErrorInfo_isr

    Note that these are NOT standard BERR codes.
 *****************************************************************************/
#define BVBI_LINE_ERROR_CC_NODATA                      0x00000001
#define BVBI_LINE_ERROR_CC_OVERRUN                     0x00000002
#define BVBI_LINE_ERROR_CC_PARITY                      0x00000004
#define BVBI_LINE_ERROR_CGMS_NODATA                    0x00000008
#define BVBI_LINE_ERROR_CGMS_OVERRUN                   0x00000010
#define BVBI_LINE_ERROR_CGMS_CRC                       0x00000020
#define BVBI_LINE_ERROR_WSS_NODATA                     0x00000040
#define BVBI_LINE_ERROR_WSS_OVERRUN                    0x00000080
#define BVBI_LINE_ERROR_WSS_PARITY                     0x00000100
#define BVBI_LINE_ERROR_TELETEXT_NODATA                0x00000200
#define BVBI_LINE_ERROR_TELETEXT_OVERRUN               0x00000400
#define BVBI_LINE_ERROR_TELETEXT_INCOMPLETE            0x00000800
#define BVBI_LINE_ERROR_TELETEXT_INVALIDLINES          0x00001000
#define BVBI_LINE_ERROR_VPS_NODATA                     0x00002000
#define BVBI_LINE_ERROR_VPS_OVERRUN                    0x00004000
#define BVBI_LINE_ERROR_VPS_BIPHASE                    0x00008000
#define BVBI_LINE_ERROR_FLDH_CONFLICT                  0x00010000
#define BVBI_LINE_ERROR_PARITY_CONFLICT                0x00020000
#define BVBI_LINE_ERROR_GEMSTAR_NODATA                 0x00040000
#define BVBI_LINE_ERROR_GEMSTAR_PARITY                 0x00080000
#define BVBI_LINE_ERROR_GEMSTAR_OVERRUN                0x00100000
#define BVBI_LINE_ERROR_AMOL_NODATA                    0x00200000
#define BVBI_LINE_ERROR_AMOL_OVERRUN                   0x00400000
#define BVBI_LINE_ERROR_MCC_NODATA                     0x00800000
#define BVBI_LINE_ERROR_MCC_OVERRUN                    0x01000000
#define BVBI_LINE_ERROR_SCTE_NODATA                    0x02000000
#define BVBI_LINE_ERROR_SCTE_OVERRUN                   0x04000000

/* These combinations express conditions of a field handle
   which prevent encoding of the VBI data contained within. */
#define BVBI_LINE_ERROR_CC_NOENCODE         (   \
    BVBI_LINE_ERROR_FLDH_CONFLICT           |   \
    BVBI_LINE_ERROR_CC_NODATA               |   \
    BVBI_LINE_ERROR_CC_PARITY               )
#define BVBI_LINE_ERROR_CGMS_NOENCODE       (   \
    BVBI_LINE_ERROR_FLDH_CONFLICT           |   \
    BVBI_LINE_ERROR_CGMS_NODATA             |   \
    BVBI_LINE_ERROR_CGMS_CRC                )
#define BVBI_LINE_ERROR_WSS_NOENCODE        (   \
    BVBI_LINE_ERROR_FLDH_CONFLICT           |   \
    BVBI_LINE_ERROR_WSS_NODATA              |   \
    BVBI_LINE_ERROR_WSS_PARITY              )
#define BVBI_LINE_ERROR_TELETEXT_NOENCODE   (   \
    BVBI_LINE_ERROR_FLDH_CONFLICT           |   \
    BVBI_LINE_ERROR_TELETEXT_NODATA         |   \
    BVBI_LINE_ERROR_TELETEXT_INCOMPLETE     |   \
    BVBI_LINE_ERROR_TELETEXT_INVALIDLINES   )
#define BVBI_LINE_ERROR_VPS_NOENCODE        (   \
    BVBI_LINE_ERROR_FLDH_CONFLICT           |   \
    BVBI_LINE_ERROR_VPS_BIPHASE             |   \
    BVBI_LINE_ERROR_VPS_NODATA              )
#define BVBI_LINE_ERROR_GEMSTAR_NOENCODE    (   \
    BVBI_LINE_ERROR_FLDH_CONFLICT           |   \
    BVBI_LINE_ERROR_GEMSTAR_NODATA          |   \
    BVBI_LINE_ERROR_GEMSTAR_PARITY              )
#define BVBI_LINE_ERROR_AMOL_NOENCODE       (   \
    BVBI_LINE_ERROR_FLDH_CONFLICT           |   \
    BVBI_LINE_ERROR_AMOL_NODATA                 )
#define BVBI_LINE_ERROR_MCC_NOENCODE        (   \
    BVBI_LINE_ERROR_FLDH_CONFLICT           |   \
    BVBI_LINE_ERROR_MCC_NODATA                  )
#define BVBI_LINE_ERROR_SCTE_NOENCODE       (   \
    BVBI_LINE_ERROR_FLDH_CONFLICT           |   \
    BVBI_LINE_ERROR_SCTE_NODATA                 )

/*****************************************************************************
 * Public API
 *****************************************************************************/


/*****************************************************************************
  Summary:
    Initialize a settings structure to defaults

  Description:
    This function initializes the BVBI_Settings structure with the
    module's default values.  It returns an error code indicating
    success or failure.  These settings may be used with BVBI_Open.

  Returns:
    BERR_SUCCESS           - Success.
    BERR_INVALID_PARAMETER - Function argument was NULL.

  See Also:
    BVBI_Open
 *****************************************************************************/
BERR_Code BVBI_GetDefaultSettings(
    BVBI_Settings *pSettings    /* [out] Pointer to an allocated
                                         BVBI_Settings object    */
);


/*****************************************************************************
  Summary:
    Initialize the VBI module

  Description:
    This function Takes a BREG_Handle and VBI settings and opens a new
    BVBI_Handle with those settings.  The BVBI_Settings should be
    initialized using BVBI_GetDefaultSettings and optionally modified.  If
    NULL is passed for pSettings, the defaults are assumed.

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BERR_OUT_OF_SYSTEM_MEMORY - Memory allocation failed.

  See Also:
    BVBI_Close, BVBI_GetDefaultSettings
 *****************************************************************************/
BERR_Code BVBI_Open(
    BVBI_Handle   *pVbiHandle,  /* [out] A pointer to an allocated
                                         BVBI_Handle                     */
    BCHP_Handle    chipHandle,  /*  [in] A handle to the chip            */
    BREG_Handle     regHandle,  /*  [in] A handle to the chip's register
                                         settings                        */
    BMMA_Heap_Handle hMmaHeap,  /*  [in] The local memory heap handle    */
    const BVBI_Settings*
                   pSettings    /*  [in] A pointer to a BVBI_Settings
                                         structure, or NULL              */
);


/*****************************************************************************
  Summary:
    Close a BVBI_Handle.

  Description:
    This function frees the BVBI_Handle structure and all associated data.
    It also shuts down the VBI operations of the specified handle.  If no
    other handles are active, or none are active for a given output, no VBI
    data will be sent on that output.  Closing a BVBI_Handle assumes that
    any associated BVBI_Encode_Handle or BVBI_Decode_Handle objects are
    closed as well.  If not, a BDBG_ASSERT error occurs.

 *****************************************************************************/
void BVBI_Close(
    BVBI_Handle vbiHandle   /* [in] A valid BVBI_Handle object */
);

/***************************************************************************
Summary:
    Get the BVBI's capabilities.

Description:
    This function gets the BVBI's capabilities.

Input:
    vbiHandle - A valid BVBI handle created earlier.

Output:
    pCapabilities - point to the capabilities structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
**************************************************************************/
BERR_Code BVBI_GetCapabilities
    ( BVBI_Handle                      vbiHandle,
      BVBI_Capabilities               *pCapabilities );

/*****************************************************************************
  Summary:
    Get default standby settings

  See Also:
    BVBI_Standby
*****************************************************************************/
void BVBI_GetDefaultStandbySettings(
    BVBI_StandbySettings *pSettings
    );

/*****************************************************************************
  Summary:
    Enter standby mode

  Description:
    This function enters standby mode with the VBI module, if supported.
    The VBI module must not be in use in order to successfully enter
    standby mode.
    If standby mode is not supported, calling this function has no effect.

    When in standby mode, the device clocks are turned off,
    resulting in a minimal power state.

    No BVBI_* calls should be made until standby mode is exitted by calling
    BVBI_Resume().

  Returns:
    BERR_SUCCESS - If standby is successful, otherwise error

  See Also:
    BVBI_Resume
*****************************************************************************/
BERR_Code BVBI_Standby
    ( BVBI_Handle hVbi,
      BVBI_StandbySettings *pSettings);

/*****************************************************************************
  Summary:
    Exit standby mode

  Description:
    This function exits from standby mode with the VBI module, if supported.
    After exitting standby mode, upper-level SW is free to call
    BVBI_* functions.

  Returns:
    BERR_SUCCESS - If standby is successful, otherwise error

  See Also:
    BVBI_Standby
*****************************************************************************/
BERR_Code BVBI_Resume
    ( BVBI_Handle hVbi);


/*****************************************************************************
  Summary:
    Create a BVBI_Decode_Handle.

  Description:
    This function creates a BVBI_Decode_Handle for the specified
    source.  That handle will be used to decode specific VBI
    formats for that source.  The BVBI_Handle must have been
    activated with a BVBI_Open call.  It is a good idea to call
    BVBI_Decode_SetVideoFormat, although the default format
    BFMT_VideoFmt_eNTSC will be imposed when BVBI_Decode_Create
    is called.

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BERR_OUT_OF_SYSTEM_MEMORY - Memory allocation failed.
    BVBI_ERR_HW_CONFLICT      - A BVBI_Decode_Handle already exists, and
                                controls the same eSource that is passed in.
                                It is not possible for two BVBI_Decode_Handles
                                to control the same video source.
 *****************************************************************************/
BERR_Code BVBI_Decode_Create(
    BVBI_Handle             vbiHandle,  /*  [in] A valid BVBI_Handle object */
    BAVC_SourceId             eSource,  /*  [in] A BAVC_SourceId object
                                                 specifying the source of
                                                 data                       */
    BVBI_Decode_Handle *pDecodeHandle   /* [out] A pointer to a
                                                 BVBI_Decode_Handle to be
                                                 initialized                */
);


/*****************************************************************************
  Summary:
    Create a BVBI_Encode_Handle.

  Description:
    This function creates a BVBI_Encode_Handle for the specified
    destination.  That handle will be used to encode specific
    VBI formats for that destination.  The BVBI_Handle must have
    been activated with a BVBI_Open call.  It is a good idea to
    call BVBI_Encode_SetVideoFormat, although the default format
    BFMT_VideoFmt_eNTSC will be imposed when BVBI_Decode_Create

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BERR_OUT_OF_SYSTEM_MEMORY - Memory allocation failed.
    BVBI_ERR_HW_CONFLICT      - A BVBI_Encode_Handle already exists, and
                                controls the same eDestination that is passed
                                in.  It is not possible for two
                                BVBI_Encode_Handles to control the same video
                                destination.

  See Also:
    BVBI_Encode_GetDefaultSettings
 *****************************************************************************/
BERR_Code BVBI_Encode_Create(
    BVBI_Handle             vbiHandle,  /*  [in] A valid BVBI_Handle object  */
    BAVC_VbiPath         eDestination,  /*  [in] An enum specifying the
                                                 destination (VEC) of the
                                                 VBI data                    */
    const BVBI_Encode_Settings
                           *pSettings,  /*  [in] Settings to apply to BVBI
                                                 Encode_Handle being
                                                 created. If NULL, then
                                                 default settings will be
                                                 applied.                    */
    BVBI_Encode_Handle *pEncodeHandle   /* [out] A pointer to a
                                                 BVBI_Encode_Handle to be
                                                 initialized.                */
);


/*****************************************************************************
  Summary:
    Obtain default settings for a BVBI Encoder object.

  Description:
    This function copies out default settings for the creation of a BVBI
    Encoder object. These default settings can be modified by the user, and
    then passed in to BVBI_Encode_Create().

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.

  See Also:
    BVBI_Encode_Creaete
 *****************************************************************************/
BERR_Code BVBI_Encode_GetDefaultSettings (
    BAVC_VbiPath         eDestination,  /*  [in] An enum specifying the
                                                 destination (VEC) of the
                                                 VBI data.                   */
    BVBI_Encode_Settings   *pSettings   /* [out] Default settings.           */
);


/*****************************************************************************
  Summary:
    Apply new attributes to the BVBI_Decode_Handle.

  Description:
    This function applies any pending Set operations to the
    BVBI_Decode_Handle, such as BVBI_Decode_SetVideoFormat.  On failure
    it will attempt to fully restore the settings that existed
    prior to the BVBI_Decode_ApplyChanges call, if any.

  Returns:
    BERR_SUCCESS              - The hardware was successfully programmed.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_ApplyChanges(
    BVBI_Decode_Handle decodeHandle /* [in] A valid BVBI_Handle object */
);


/*****************************************************************************
  Summary:
    Set the video display format used for decoding VBI.

  Description:
    This function sets the video display format to use with the supplied
    BVBI_Decode_Handle.  The change to video format will be programmed into
    hardware when BVBI_Decode_ApplyChanges is next called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_SetVideoFormat(
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object                     */
    BFMT_VideoFmt      eVideoFormat     /* [in] The desired display format */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Get the video display format used for decoding VBI.

  Description:
    This function returns the video display format in use with the supplied
    BVBI_Decode_Handle.

  Returns:
    BERR_SUCCESS              - The information was successfully retrieved.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_GetVideoFormat(
    BVBI_Decode_Handle decodeHandle,    /*  [in] A valid BVBI_Decode_Handle
                                                 object                     */
    BFMT_VideoFmt    *peVideoFormat     /* [out] The display format in use
                                                 with the above object      */
);
#endif


/*****************************************************************************
  Summary:
    Set ITU-R 656 encapsulation method for VBI decoding

  Description:
    If a VBI decoder is created to support input via the ITU-R 656
    port, then this call will cause the decoder to expect the
    encapsulated VBI data to occur in a specific format.  Only
    one specific format can be used at a time.  The requested
    change is implemented the next time BVBI_Decode_ApplyChanges
    is called on the associated BVBI_Decode_Handle.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_656_SetFormat (
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object                       */
    BVBI_656Fmt       anci656Fmt        /* [in] The encapsulation format to
                                                be expected.                 */
);


/*****************************************************************************
  Summary:
    Callback that provides the default parsing mechanism for incoming SMPTE
    291M packets.

  Description:
    SMPTE 291M is a standard for encapsulating user data in ancillary
    data packets for inclusion in an ITU-R 656 bitstream.  SMPTE 291M
    is an "open" standard that allows the user data to be expressed in
    a variety of ways.  The user must provide a callback function to
    parse incoming data according to the user's method.  This callback
    function is the default function used by the porting interface.
    It expects the incoming SMPTE 291M packets to be in the proprietary
    Broadcom format.

    Because the SMPTE 291M standard is so flexible, it is
    necessary that this callback function make a specific
    assumption about the incoming ancillary data packets.
    It assumes that the DATA ID field of the packet identifies
    the packet content as follows:
        - 0x30 indicates closed caption data is in the packet.
        - 0x31 indicates teletext or NABTS data is in the packet.
        - 0x32 indicates WSS data is in the packet.
        - 0x33 indicates VPS data is in the packet.

    In addition, this function does not choose a video line number for
    the VBI data, which causes the BVBI module to choose a default.

    The above assumptions are completely arbitrary.  In most applications, the
    user will write his own callback function to replace this one, and install
    it by calling BVBI_Decode_656_SetSMPTE291Moptions().

  Returns:
    BERR_SUCCESS
    TODO

 *****************************************************************************/
BERR_Code BVBI_Decode_656_SMPTE291MbrcmCb_isr (
    void*         arg0, /*  [in] Not used.                                   */
    BFMT_VideoFmt
          eVideoFormat, /*  [in] Video format of input 656 port              */
    BAVC_Polarity
              polarity, /*  [in] The field polarity according to BVBI
                                 interrupts.                                 */
    uint8_t    data_id, /*  [in] The first identification field in the
                                 SMPTE 291M packet header.                   */
    uint8_t  second_id, /*  [in] The second identification field in the
                                 SMPTE 291M packet header.                   */
    uint8_t data_count, /*  [in] The DATA COUNT field in the SMPTE 291M
                                 packet header.                              */
    uint8_t*    packet, /*  [in] The SMPTE 291M ancillary data packet,
                                 starting with the first byte of the
                                 payload.  Note that the three header bytes
                                 have already been parsed by the BVBI
                                 software when this callback function is
                                 called.  The final checksum byte has also
                                 been read and verified by the BVBI
                                 software and is not available to this
                                 callback function.                          */
    uint8_t**  vbiData, /* [out] The parsed VBI data.  For example, if the
                                 SMPTE 291M packet contains closed caption
                                 data, then this argument will be the
                                 two bytes of closed caption data.  This
                                 function will overwrite the data pointed
                                 to by the above argument "packet."          */
    BVBI_SMPTE291M_Description*
        pktDesc         /* [out] Describes the above argument vbiData.       */
);


/*****************************************************************************
  Summary:
    Set options for decoding of SMPTE 291M ancillary data packets.

  Description:
    If a VBI decoder is created to support input via the ITU-R 656
    port, and if BVBI_Decode_656_SetFormat() has been called to
    set the expected format of encapsulated data to SMPTE 291M,
    then this function will allow the user to specify how exactly
    to decode the SMPTE 291M data packets.  SMPTE 291M is an
    "open" standard that does not specify exactly what to do
    with the "payload" of the  packet.  This function allows
    the user to do so.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_656_SetSMPTE291Moptions (
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object.                      */
    BVBI_Decode_656_SMPTE291M_Cb
               fParserCb_isr,           /* [in] The parsing function for the
                                                SMPTE 291M packet.  Note
                                                that the default callback is
                                    BVBI_Decode_656_SMPTE291MbrcmCb_isr().   */
    void*               arg0,           /* [in] First argument passed to
                                                the above callback function.
                                                Note that the defaut is 0x0. */
    bool         bLongHeader,           /* [in] If true, then incomping
                                                SMPTE 291M packets are
                                                expected to have the three
                                                byte "component stream"
                                                header 00 FF FF.  Otherwise,
                                                packets are expected to have
                                                the one byte "composite
                                                stream" header FF.  Note
                                                that the default for the
                                                decode handle is "false."    */
    bool    bBrokenDataCount            /* [in] If true, then the incoming
                                                SMPTE 291M packets are
                                                expected to be in the
                                                historical, incorrect format
                                                produced by Broadcom ITU-R
                                                656 encoders.  In particular,
                                                the packet DATA COUNT field
                                                counts bytes instead of
                                                DWORDs, and the accompanying
                                                parity bits are not present.
                                                Note that the default for
                                                the decode handle is "true." */
);


/*****************************************************************************
  Summary:
    Set ITU-R 656 encapsulation method for VBI decoding

  Description:
    This function returns the format in use for extracting VBI data from
    ancillary data packets in the ITU-R 656 bitstream input by the supplied
    BVBI_Decode_Handle.

  Returns:
    BERR_SUCCESS              - The information was successfully retrieved.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_656_GetFormat(
    BVBI_Decode_Handle decodeHandle,    /*  [in] A valid BVBI_Decode_Handle
                                                 object                     */
    BVBI_656Fmt*        pAnci656Fmt     /* [out] The format in use
                                                 with the above object      */
);


/*****************************************************************************
  Summary:
    Apply new attributes to the BVBI_Encode_Handle.

  Description:
    This function applies any pending Set operations to the
    BVBI_Encode_Handle, such as BVBI_Encode_SetVideoFormat. On failure
    it will attempt to fully restore the settings that existed
    prior to the BVBI_Encode_ApplyChanges call, if any.

  Returns:
    BERR_SUCCESS              - The chip hardware was successfully programmed.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_ApplyChanges(
    BVBI_Encode_Handle encodeHandle /* [in] A valid BVBI_Handle object */
);


/*****************************************************************************
  Summary:
    Set the video display format used for encoding VBI.

  Description:
    This function sets the video display format to use with the supplied
    BVBI_Encode_Handle.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetVideoFormat(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                      */
    BFMT_VideoFmt      eVideoFormat     /*  [in] The desired display format  */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    This function is not needed for any chip supported by this version of
    reference software. It does nothing.

  Description:
    This function is not needed for any chip supported by this version of
    reference software. It does nothing.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetHdmiPixelRepetition(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                      */
    BAVC_HDMI_PixelRepetition ePixRep   /*  [in] The desired HDMI pixel
                                                 repetition value            */
);
#endif


/*****************************************************************************
  Summary:
    Set the video color space used for encoding VBI.

  Description:
    This function sets the color space format to use with the
    supplied BVBI_Encode_Handle. Currently, this only setting is
    only required by the SCTE encoder core. All other forms of VBI
    data encoding function independendly of color space.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetCSC(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                      */
    BVBI_CSC                   eCsc     /*  [in] The desired color space.    */
);


/*****************************************************************************
  Summary:
    Get the video display format used for encoding VBI.

  Description:
    This function returns the video display format in use with the supplied
    BVBI_Encode_Handle.

  Returns:
    BERR_SUCCESS              - The information was successfully retrieved.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetVideoFormat(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    BFMT_VideoFmt    *peVideoFormat     /* [out] The display format in use
                                                 with the above object      */
);


/*****************************************************************************
  Summary:
    This function is no longer relevant for any chips supported by this
    version of reference software. It returns a nonsense value.

  Description:
    This function is no longer relevant for any chips supported by this
    version of reference software. It returns a nonsense value
    BAVC_HDMI_PixelRepetition_eNone.

  Returns:
    BERR_SUCCESS              - The information was successfully retrieved.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetHdmiPixelRepetition(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    BAVC_HDMI_PixelRepetition* pePixRep /* [out] Nonsense valoue. See
                                                 function description.      */
);


/*****************************************************************************
  Summary:
    Get the color space used for encoding VBI.

  Description:
    This function returns the color space in use with the supplied
    BVBI_Encode_Handle. Note that at this time, the only VBI encoding
    function affected by this setting is SCTE data encoding.

  Returns:
    BERR_SUCCESS              - The information was successfully retrieved.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetCSC(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object.                     */
    BVBI_CSC                 *peCsc     /* [out] The color space in use
                                                 with the above object.      */
);



/*****************************************************************************
  Summary:
    Destroy a BVBI_Decode_Handle.

  Description:
    This function destroys a valid BVBI_Decode_Handle created by
    BVBI_Decode_Create.

  Returns:
    BERR_SUCCESS              - The handle was successfully destroyed.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_Destroy(
    BVBI_Decode_Handle decodeHandle /* [in] A BVBI_Decode_Handle to be
                                            destroyed                        */
);


/*****************************************************************************
  Summary:
    Destroy a BVBI_Encode_Handle.

  Description:
    This function destroys a valid BVBI_Encode_Handle created by
    BVBI_Encode_Create.

  Returns:
    BERR_SUCCESS              - The handle was successfully destroyed.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_Destroy(
    BVBI_Encode_Handle encodeHandle /* [in] A BVBI_Encode_Handle to be
                                            destroyed                        */
);


/*****************************************************************************
  Summary:
    Set CC input on the given handle

  Description:
    This call enables or disables CC decode on the supplied active
    BVBI_Decode_Handle.  CC decode is enabled or disabled for the
    given BVBI_Decode_Handle the next time BVBI_Decode_ApplyChanges
    is called on the associated BVBI_Decode_Handle. If bEnabled is true
    (nonzero), CC decode is enabled.  If bEnabled is false, CC
    decode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_SetCC(
    BVBI_Decode_Handle decodeHandle, /* [in] A valid BVBI_Decode_Handle
                                             object                       */
    bool                   bEnabled  /* [in] Whether to enable or disable
                                             decode                       */
);


/*****************************************************************************
  Summary:
    Set CGMS input on the given handle

  Description:
    This call enables or disables CGMS decode on the supplied active
    BVBI_Decode_Handle.  CGMS decode is enabled or disabled for the
    given BVBI_Decode_Handle the next time BVBI_Decode_ApplyChanges
    is called on the associated BVBI_Decode_Handle. If bEnabled is true
    (nonzero), CGMS decode is enabled.  If bEnabled is false,
    CGMS decode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_SetCGMS(
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                decode                       */
);


/*****************************************************************************
  Summary:
    Set WSS input on the given handle

  Description:
    This call enables or disables WSS decode on the supplied active
    BVBI_Decode_Handle.  WSS decode is enabled or disabled for the
    given BVBI_Decode_Handle the next time BVBI_Decode_ApplyChanges
    is called on the associated BVBI_Decode_Handle. If bEnabled is true
    (nonzero), WSS decode is enabled.  If bEnabled is false, WSS
    decode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_SetWSS(
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                decode                       */
);


/*****************************************************************************
  Summary:
    Set VPS input on the given handle

  Description:
    This call enables or disables VPS decode on the supplied active
    BVBI_Decode_Handle.  VPS decode is enabled or disabled for the
    given BVBI_Decode_Handle the next time BVBI_Decode_ApplyChanges
    is called on the associated BVBI_Decode_Handle. If bEnabled is true
    (nonzero), VPS decode is enabled.  If bEnabled is false, VPS
    decode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_SetVPS(
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                decode                       */
);


/*****************************************************************************
  Summary:
    Set Teletext input on the given handle

  Description:
    This call enables or disables Teletext decode on the supplied
    active BVBI_Decode_Handle.  Teletext decode is enabled
    or disabled for the given BVBI_Decode_Handle the next time
    BVBI_Decode_ApplyChanges is called on the associated BVBI_Decode_Handle.
    If bEnabled is true (nonzero), Teletext decode is enabled.
    If bEnabled is false, Teletext decode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_SetTeletext(
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                decode                       */
);


/*****************************************************************************
  Summary:
    Gets the status of CC input on the given handle

  Description:
    This call determines whether CC is currently enabled or disabled
    on the current handle.  Any pending change of status will not be
    returned by this call until BVBI_Decode_ApplyChanges is called.
    pbEnabled will be set to 1 if input is enabled and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_GetCC(
    BVBI_Decode_Handle decodeHandle,    /*  [in] A valid BVBI_Decode_Handle
                                                 object                      */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                 */
);


/*****************************************************************************
  Summary:
    Gets the status of CGMS input on the given handle

  Description:
    This call determines whether CGMS is currently enabled or
    disabled on the current handle.  Any pending change of status
    will not be returned by this call until BVBI_Decode_ApplyChanges
    is called.  pbEnabled will be set to 1 if input is enabled and
    0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_GetCGMS(
    BVBI_Decode_Handle decodeHandle,    /*  [in] A valid BVBI_Decode_Handle
                                                 object                      */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                 */
);


/*****************************************************************************
  Summary:
    Gets the status of WSS input on the given handle

  Description:
    This call determines whether WSS is currently enabled or disabled
    on the current handle.  Any pending change of status will not be
    returned by this call until BVBI_Decode_ApplyChanges is called.
    pbEnabled will be set to 1 if input is enabled and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_GetWSS(
    BVBI_Decode_Handle decodeHandle,    /*  [in] A valid BVBI_Decode_Handle
                                                 object                      */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                 */
);


/*****************************************************************************
  Summary:
    Gets the status of VPS input on the given handle

  Description:
    This call determines whether VPS is currently enabled or disabled
    on the current handle.  Any pending change of status will not be
    returned by this call until BVBI_Decode_ApplyChanges is called.
    pbEnabled will be set to 1 if input is enabled and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_GetVPS(
    BVBI_Decode_Handle decodeHandle,    /*  [in] A valid BVBI_Decode_Handle
                                                 object                      */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                 */
);


/*****************************************************************************
  Summary:
    Gets the status of Teletext input on the given handle

  Description:
    This call determines whether Teletext is currently enabled or
    disabled on the current handle.  Any pending change of status
    will not be returned by this call until BVBI_Decode_ApplyChanges
    is called.  pbEnabled will be set to 1 if input is enabled and
    0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_GetTeletext(
    BVBI_Decode_Handle decodeHandle,    /*  [in] A valid BVBI_Decode_Handle
                                                 object                      */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                 */
);

/*****************************************************************************
  Summary:
    Set Gemstar input on the given handle

  Description:
    This call enables or disables Gemstar decode on the supplied
    active BVBI_Decode_Handle.  Gemstar decode is enabled
    or disabled for the given BVBI_Decode_Handle the next time
    BVBI_Decode_ApplyChanges is called on the associated BVBI_Decode_Handle.
    If bEnabled is true (nonzero), Gemstar decode is enabled.
    If bEnabled is false, Gemstar decode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_SetGemstar(
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                decode                       */
);


/*****************************************************************************
  Summary:
    Get Gemstar input on the given handle

  Description:
    This call determines whether Gemstar is currently enabled or disabled on
    the curent handle. Any pending change of status will not be returned by
    this call until BVBI_Decode_ApplyChanges is called. pbEnabled will be set
    to 1 if input is enabled and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_GetGemstar(
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object                       */
    bool*                 pbEnabled     /* [in] Whether decode is enabled
                                                or disabled.                 */
);


/*****************************************************************************
  Summary:
    Set CC output on the given handle

  Description:
    This call enables or disables CC encode on the supplied active
    BVBI_Encode_Handle.  CC encode is enabled or disabled for the
    given BVBI_Encode_Handle the next time BVBI_Encode_ApplyChanges
    is called on the associated BVBI_Handle. If bEnabled is true
    (nonzero), CC encode is enabled.  If bEnabled is false, CC
    encode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetCC(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


/*****************************************************************************
  Summary:
    Set CGMS-A output on the given handle

  Description:
    This call enables or disables CGMS-A encode on the supplied active
    BVBI_Encode_Handle.  CGMS-A encode is enabled or disabled for the
    given BVBI_Encode_Handle the next time BVBI_Encode_ApplyChanges
    is called on the associated BVBI_Handle. If bEnabled is true
    (nonzero), CGMS-A encode is enabled.  If bEnabled is false,
    CGMS-A encode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_HW_CONFLICT      - User attempted to activate CGMS-A and CGMS-B
                                on the same VEC.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetCGMSA(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);
/* Backwards compatibility, DEPRECATED! */
#define BVBI_Encode_SetCGMS BVBI_Encode_SetCGMSA


/*****************************************************************************
  Summary:
    Set CGMS-B output on the given handle

  Description:
    This call enables or disables CGMS-B encode on the supplied active
    BVBI_Encode_Handle.  CGMS-B encode is enabled or disabled for the
    given BVBI_Encode_Handle the next time BVBI_Encode_ApplyChanges
    is called on the associated BVBI_Handle. If bEnabled is true
    (nonzero), CGMS-B encode is enabled.  If bEnabled is false,
    CGMS-B encode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_HW_CONFLICT      - User attempted to activate CGMS-A and CGMS-B
                                on the same VEC.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetCGMSB(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


/*****************************************************************************
  Summary:
    Set CGMS-B style on the given encoder handle

  Description:
    This call changes the output style for CGMS-B data on the supplied active
    BVBI_Encode_Handle.  Change takes effect on the
    given BVBI_Encode_Handle the next time
    1. CGMS-B output is enabled.
    2. BVBI_Encode_ApplyChanges is called on the associated BVBI encode handle.

    Note: older chipsets are not capable of CEA-805-D "new style"
    ----  output. For these chipsets, this API function is not provided.

    Note: For newer chipsets that support "new style" output, this is the
    ----  default. Since "new style" output is recommended, this function
          will not be needed by most applications.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetCGMSBstyle(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool              bCea805dStyle     /* [in] Whether to enable the newer
                                                CRC computation ("method 2")
                                                defined in CEA-805-D. If
                                                true, the bit order is also
                                                changed from "LSB first" to
                                                "MSB first."                 */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Get CGMS-B style on the given encoder handle

  Description:
    This reports the output style for CGMS-B data on the supplied active
    BVBI_Encode_Handle.

    Note: older chipsets are not capable of CEA-805-D "new style"
    ----  output. For these chipsets, this API function is not provided.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetCGMSBstyle(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                      */
    bool*            pbCea805dStyle     /* [out] Whether the newer CRC
                                                 computation ("method 2")
                                                 defined in CEA-805-D is in
                                                 use.                        */
);
#endif


/*****************************************************************************
  Summary:
    Set WSS output on the given handle

  Description:
    This call enables or disables WSS encode on the supplied active
    BVBI_Encode_Handle.  WSS encode is enabled or disabled for the
    given BVBI_Encode_Handle the next time BVBI_Encode_ApplyChanges
    is called on the associated BVBI_Encode_Handle. If bEnabled is true
    (nonzero), WSS encode is enabled.  If bEnabled is false, WSS
    encode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetWSS(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


/*****************************************************************************
  Summary:
    Set Teletext output on the given handle

  Description:
    This call enables or disables Teletext encode on the supplied
    active BVBI_Encode_Handle.  Teletext encode is enabled
    or disabled for the given BVBI_Encode_Handle the next time
    BVBI_Encode_ApplyChanges is called on the associated BVBI_Encode_Handle.
    If bEnabled is true (nonzero), Teletext encode is enabled.
    If bEnabled is false, Teletext encode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL. Teletext could not be activated
                                because BVBI_Encode_Handle was created with
                                teletext disabled.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetTeletext(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


/*****************************************************************************
  Summary:
    Set VPS output on the given handle

  Description:
    This call enables or disables VPS encode on the supplied active
    BVBI_Encode_Handle.  VPS encode is enabled or disabled for the
    given BVBI_Encode_Handle the next time BVBI_Encode_ApplyChanges
    is called on the associated BVBI_Encode_Handle. If bEnabled is true
    (nonzero), VPS encode is enabled.  If bEnabled is false, VPS
    encode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetVPS (
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


/*****************************************************************************
  Summary:
    Set Gemstar output on the given handle

  Description:
    This call enables or disables Gemstar encode on the supplied active
    BVBI_Encode_Handle.  Gemstar encode is enabled or disabled for the
    given BVBI_Encode_Handle the next time BVBI_Encode_ApplyChanges
    is called on the associated BVBI_Encode_Handle. If bEnabled is true
    (nonzero), Gemstar encode is enabled.  If bEnabled is false, Gemstar
    encode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetGemstar (
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


/*****************************************************************************
  Summary:
    Set AMOL output on the given handle

  Description:
    This call enables or disables AMOL encode on the supplied active
    BVBI_Encode_Handle.  AMOL encode is enabled or disabled for the
    given BVBI_Encode_Handle the next time BVBI_Encode_ApplyChanges
    is called on the associated BVBI_Encode_Handle.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetAMOL (
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Set multi-line closed caption data output on the given handle

  Description:
    This call enables or disables multi-line closed caption encode on
    the supplied active BVBI_Encode_Handle.  multi-line closed caption
    encode is enabled or disabled for the given BVBI_Encode_Handle the
    next time BVBI_Encode_ApplyChanges is called on the associated
    BVBI_Encode_Handle.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetMCC (
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);
#endif


/*****************************************************************************
  Summary:
    Set SCTE output on the given handle

  Description:
    This call enables or disables SCTE encode on the supplied active
    BVBI_Encode_Handle.  SCTE encode is enabled or disabled for the
    given BVBI_Encode_Handle the next time BVBI_Encode_ApplyChanges
    is called on the associated BVBI_Encode_Handle.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetSCTE (
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of CC output on the given handle

  Description:
    This call determines whether CC is currently enabled or disabled
    on the current handle.  Any pending change of status will
    not be returned by this call until BVBI_Encode_ApplyChanges
    is called.  pbEnabled will be set to 1 if output is enabled
    and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetCC(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of CGMS-A output on the given handle

  Description:
    This call determines whether CGMS-A is currently enabled or
    disabled on the current handle.  Any pending change of status
    will not be returned by this call until BVBI_Encode_ApplyChanges
    is called.  pbEnabled will be set to 1 if output is enabled
    and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetCGMSA(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
/* Backwards compatibility, DEPRECATED! */
#define BVBI_Encode_GetCGMS BVBI_Encode_GetCGMSA
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of CGMS-B output on the given handle

  Description:
    This call determines whether CGMS-B is currently enabled or
    disabled on the current handle.  Any pending change of status
    will not be returned by this call until BVBI_Encode_ApplyChanges
    is called.  pbEnabled will be set to 1 if output is enabled
    and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetCGMSB(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of WSS output on the given handle

  Description:
    This call determines whether WSS is currently enabled or
    disabled on the current handle.  Any pending change of status
    will not be returned by this call until BVBI_Encode_ApplyChanges
    is called.  pbEnabled will be set to 1 if output is enabled
    and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetWSS(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of Teletext output on the given handle

  Description:
    This call determines whether Teletext is currently enabled or
    disabled on the current handle.  Any pending change of status
    will not be returned by this call until BVBI_Encode_ApplyChanges
    is called.  pbEnabled will be set to 1 if output is enabled
    and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetTeletext(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of VPS output on the given handle

  Description:
    This call determines whether VPS is currently enabled or
    disabled on the current handle.  Any pending change of status
    will not be returned by this call until BVBI_Encode_ApplyChanges
    is called.  pbEnabled will be set to 1 if output is enabled
    and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetVPS (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of Gemstar output on the given handle

  Description:
    This call determines whether Gemstar is currently enabled or
    disabled on the current handle.  Any pending change of status
    will not be returned by this call until BVBI_Encode_ApplyChanges
    is called.  pbEnabled will be set to 1 if output is enabled
    and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetGemstar (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of AMOL output on the given handle

  Description:
    This call determines what type of AMOL (if any) is currently enabled
    on the current handle.  Any pending change of status will not be returned
    by this call until BVBI_Encode_ApplyChanges is called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetAMOL (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of multi-line closed caption data output on the
    given handle

  Description:
    This call determines what type of multi-line closed caption data
    (if any) is currently enabled on the current handle.  Any pending
    change of status will not be returned by this call until
    BVBI_Encode_ApplyChanges is called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetMCC (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if (BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) /** { **/
#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of SCTE output on the given handle

  Description:
    This call determines what type of SCTE (if any) is currently enabled
    on the current handle.  Any pending change of status will not be returned
    by this call until BVBI_Encode_ApplyChanges is called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetSCTE (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif
#endif /** } (BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) **/


/*****************************************************************************
  Summary:
    Set ITU-R 656 closed caption output on the given handle

  Description:
    This call enables or disables ITU-R 656 closed caption encode on
    the supplied active BVBI_Encode_Handle.  closed caption encode
    is enabled or disabled for the given BVBI_Encode_Handle the
    next time BVBI_Encode_ApplyChanges is called on the associated
    BVBI_Encode_Handle. If bEnabled is true (nonzero), ITU-R 656 closed
    caption encode is enabled.  If bEnabled is false, ITU-R 656
    closed caption encode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_SetCC(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


/*****************************************************************************
  Summary:
    Set ITU-R 656 teletext output on the given handle

  Description:
    This call enables or disables ITU-R 656 teletext encode on
    the supplied active BVBI_Encode_Handle.  Teletext encode
    is enabled or disabled for the given BVBI_Encode_Handle the
    next time BVBI_Encode_ApplyChanges is called on the associated
    BVBI_Encode_Handle. If bEnabled is true (nonzero), ITU-R 656 teletext
    encode is enabled.  If bEnabled is false, ITU-R 656 teletext
    encode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_SetTeletext(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


/*****************************************************************************
  Summary:
    Set ITU-R 656 WSS output on the given handle

  Description:
    This call enables or disables ITU-R 656 WSS encode on
    the supplied active BVBI_Encode_Handle.  WSS encode
    is enabled or disabled for the given BVBI_Encode_Handle the
    next time BVBI_Encode_ApplyChanges is called on the associated
    BVBI_Encode_Handle. If bEnabled is true (nonzero), ITU-R 656 WSS
    encode is enabled.  If bEnabled is false, ITU-R 656 WSS
    encode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_SetWSS(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


/*****************************************************************************
  Summary:
    Set ITU-R 656 Gemstar output on the given handle

  Description:
    This call enables or disables ITU-R 656 Gemstar encode on
    the supplied active BVBI_Encode_Handle.  Gemstar encode
    is enabled or disabled for the given BVBI_Encode_Handle the
    next time BVBI_Encode_ApplyChanges is called on the associated
    BVBI_Encode_Handle. If bEnabled is true (nonzero), ITU-R 656 Gemstar
    encode is enabled.  If bEnabled is false, ITU-R 656 Gemstar
    encode is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_SetGemstar(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


/*****************************************************************************
  Summary:
    Set ITU-R 656 AMOL output on the given handle

  Description:
    This call enables or disables ITU-R 656 AMOL encode on
    the supplied active BVBI_Encode_Handle.  AMOL encode
    is enabled or disabled for the given BVBI_Encode_Handle the
    next time BVBI_Encode_ApplyChanges is called on the associated
    BVBI_Encode_Handle.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_SetAMOL (
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


/*****************************************************************************
  Summary:
    Set ITU-R 656 multi-line closed caption output on the given handle

  Description:
    This call enables or disables ITU-R 656 multi-line closed caption
    encode on the supplied active BVBI_Encode_Handle.  multi-line
    closed caption encode is enabled or disabled for the given
    BVBI_Encode_Handle the next time BVBI_Encode_ApplyChanges is
    called on the associated BVBI_Encode_Handle.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_SetMCC (
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);


#if (BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) /** { **/
/*****************************************************************************
  Summary:
    Set ITU-R 656 SCTE output on the given handle

  Description:
    This call enables or disables ITU-R 656 SCTE encode on
    the supplied active BVBI_Encode_Handle.  SCTE encode
    is enabled or disabled for the given BVBI_Encode_Handle the
    next time BVBI_Encode_ApplyChanges is called on the associated
    BVBI_Encode_Handle.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_SetSCTE (
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                encode                       */
);
#endif /** } (BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) **/


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of ITU-R 656 closed caption output on the
    given handle

  Description:
    This call determines whether ITU-R 656 closed caption
    is currently enabled or disabled on the current handle.
    Any pending change of status will not be returned by this call
    until BVBI_Encode_ApplyChanges is called.  pbEnabled will be
    set to 1 if output is enabled and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_GetCC (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of ITU-R 656 teletext output on the given handle

  Description:
    This call determines whether ITU-R 656 teletext is currently
    enabled or disabled on the current handle.  Any pending
    change of status will not be returned by this call until
    BVBI_Encode_ApplyChanges is called.  pbEnabled will be set to
    1 if output is enabled and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_GetTeletext (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of ITU-R 656 WSS output on the given handle

  Description:
    This call determines whether ITU-R 656 WSS is currently
    enabled or disabled on the current handle.  Any pending
    change of status will not be returned by this call until
    BVBI_Encode_ApplyChanges is called.  pbEnabled will be set to
    1 if output is enabled and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_GetWSS (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of ITU-R 656 Gemstar output on the given handle

  Description:
    This call determines whether ITU-R 656 Gemstar is currently
    enabled or disabled on the current handle.  Any pending
    change of status will not be returned by this call until
    BVBI_Encode_ApplyChanges is called.  pbEnabled will be set to
    1 if output is enabled and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_GetGemstar (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of ITU-R 656 AMOL output on the given handle

  Description:
    This call determines what type of ITU-R 656 AMOL (if any) is currently
    enabled on the current handle.  Any pending change of status will not be
    returned by this call until BVBI_Encode_ApplyChanges is called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_GetAMOL (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of ITU-R 656 multi-line closed caption output on
    the given handle

  Description:
    This call determines what type of ITU-R 656 multi-line closed
    caption data (if any) is currently enabled on the current handle.
    Any pending change of status will not be returned by this call
    until BVBI_Encode_ApplyChanges is called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_GetMCC (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets the status of ITU-R 656 SCTE output on the given handle

  Description:
    This call determines what type of ITU-R 656 SCTE (if any) is currently
    enabled on the current handle.  Any pending change of status will not be
    returned by this call until BVBI_Encode_ApplyChanges is called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_GetSCTE (
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                     */
    bool*                 pbEnabled     /* [out] Whether encode is enabled
                                                 or disabled                */
);
#endif

#if (BVBI_NUM_PTVEC > 0) /** { **/

#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Set ITU-R 656 encapsulation method for VBI encoding

  Description:
    This call determines which format is used to encapsulate VBI data
    into ancillary data packets within an ITU-R 656 bitstream output
    by the device.  An active VBI encoder handle is required.
    The requested change is implemented the next time
    BVBI_Encode_ApplyChanges is called on the associated BVBI_Encode_Handle.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_SetFormat (
    BVBI_Encode_Handle encodeHandle,  /* [in] A valid BVBI_Encode_Handle
                                              object                         */
    BVBI_656Fmt          anci656Fmt,  /* [in] The encapsulation format to
                                              be used.  Note that the
                                              default is
                                              BVBI_656Fmt_SAA7113.           */
    uint8_t                    sdid   /* [in] The value to be encoded in
                                              the SDID field of the
                                              packet header.  Only the least
                                              significant 6 bits are used.
                                              Note that the default is 0.    */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Get the format used for encapsulating VBI data into ancillary
    data packets within an ITU-R 656 bitstream output by the device.

  Description:
    This function returns the format in use for encapsulating VBI data into
    ancillary data packets in the ITU-R 656 bitstream output by the supplied
    BVBI_Encode_Handle.

  Returns:
    BERR_SUCCESS              - The information was successfully retrieved.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_656_GetFormat(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                      */
    BVBI_656Fmt*        pAnci656Fmt,    /* [out] The format in use
                                                 with the above object       */
    uint8_t*                  psdid     /* [out] The value to be encoded in
                                                 the SDID field of the
                                                 packet header.  The two most
                                                 significant bits are are
                                                 parity bits.                */
);
#endif

#endif /** } (BVBI_NUM_PTVEC > 0) **/

#if (BVBI_P_HAS_XSER_TT > 0) /** { **/

#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Set teletext external serial output on the given handle

  Description:
    This call enables or disables teletext output through external serial
    port for the supplied active BVBI_Encode_Handle.  Teletext output is
    enabled or disabled for the given BVBI_Encode_Handle the next time
    BVBI_Encode_ApplyChanges is called on the associated BVBI_Encode_Handle.
    If bEnabled is true (nonzero), teletext output is enabled.
    If bEnabled is false, teletext output is disabled.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_XSER_SetTeletext(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool                   bEnabled     /* [in] Whether to enable or disable
                                                output                       */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Get state of teletext output to external serial port on the given handle

  Description:
    This call determines whether teletext output to external serial port is
    currently enabled or disabled on the current handle. Any pending change
    of status will not be returned by this call until
    BVBI_Encode_ApplyChanges is called. pbEnabled will be set to 1 if output
    is enabled and 0 if it isn't.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_XSER_GetTeletext(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    bool*                 pbEnabled     /* [in] Whether output is enabled
                                                or disabled.                 */
);
#endif

#endif /** } (BVBI_P_HAS_XSER_TT > 0) **/

#if (BVBI_P_HAS_XSER_TT > 0) /** { **/

#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Sets options for VBI output through external serial port.

  Description:
    This function takes an active BVBI_Encode_Handle and sets options
    related to output to the external serial port. These options take effect
    at the next call to BVBI_Encode_ApplyChanges.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_XSER_SetOptions(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    BVBI_XSER_Settings*   pSettings     /* [in] Settings for serial port
                                                output.                      */
);
#endif

#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets options related to VBI output through external serial port.

  Description:
    This function takes an active BVBI_Encode_Handle and retrieves
    options related to output through the external serial port. Any
    pending change of status will not be returned by this call
    until BVBI_Encode_ApplyChanges is called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_XSER_GetOptions(
    BVBI_Encode_Handle encodeHandle,    /*  [in] A valid BVBI_Encode_Handle
                                                 object                      */
    BVBI_XSER_Settings*   pSettings     /* [out] Settings for serial port
                                                 output.                     */
);
#endif

#endif /** } (BVBI_P_HAS_XSER_TT > 0) **/


/*****************************************************************************
  Summary:
    Sets options for Gemstar encoding

  Description:
    This function takes an active BVBI_Encode_Handle and sets options
    related to Gemstar encoding. These options take effect at the next call to
    BVBI_Encode_ApplyChanges.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetGemstarOptions(
    BVBI_Encode_Handle  encodeHandle,   /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    const BVBI_GSOptions* pGsOptions    /* [in] Options for Gemstar output.  */
);


/*****************************************************************************
  Summary:
    Gets options for Gemstar encoding

  Description:
    This function takes an active BVBI_Encode_Handle and retrieves options
    related to Gemstar encoding.  Any pending change of status will not be
    returned by this call until BVBI_Encode_ApplyChanges is called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetGemstarOptions(
    BVBI_Encode_Handle encodeHandle,   /*  [in] A valid BVBI_Encode_Handle
                                                object                       */
    BVBI_GSOptions*      pGsOptions    /* [out] Options for Gemstar output.  */
);


/*****************************************************************************
  Summary:
    Sets options for AMOL encoding

  Description:
    This function takes an active BVBI_Encode_Handle and sets options
    related to AMOL encoding. These options take effect at the next call to
    BVBI_Encode_ApplyChanges.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetAMOLOptions(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    BVBI_AMOL_Type         amolType     /* [in] Which variant of AMOL to
                                                be output.                   */
);


/*****************************************************************************
  Summary:
    Sets options for SCTE encoding

  Description:
    This function takes an active BVBI_Encode_Handle and sets options
    related to SCTE encoding. These options take effect at the next call to
    BVBI_Encode_ApplyChanges.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_SetSCTEOptions(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    BVBI_SCTE_Type         scteType     /* [in] Which type of SCTE data to
                                                output.                      */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets options for AMOL encoding

  Description:
    This function takes an active BVBI_Encode_Handle and retrieves options
    related to AMOL encoding.  Any pending change of status will not be
    returned by this call until BVBI_Encode_ApplyChanges is called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetAMOLOptions(
    BVBI_Encode_Handle encodeHandle,   /*  [in] A valid BVBI_Encode_Handle
                                                object                       */
    BVBI_AMOL_Type*       pAmolType    /* [out] Which type of AMOL encoding
                                                is being output.             */
);
#endif


/*****************************************************************************
  Summary:
    Gets options for SCTE encoding

  Description:
    This function takes an active BVBI_Encode_Handle and retrieves options
    related to SCTE encoding.  Any pending change of status will not be
    returned by this call until BVBI_Encode_ApplyChanges is called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Encode_GetSCTEOptions(
    BVBI_Encode_Handle encodeHandle,   /*  [in] A valid BVBI_Encode_Handle
                                                object                       */
    BVBI_SCTE_Type*       pScteType    /* [out] Which type of SCTE data
                                                is being output.             */
);


/*****************************************************************************
  Summary:
    Encodes one field worth of VBI data

  Description:
    This function takes an active BVBI_Encode_Handle and valid
    BVBI_Field_Handle and encodes that data appropriately for the
    next available field(s).  Note that some formats may have a
    delay on output, and so the same supplied BVBI_Field_Handle may
    be output to more than one video field and may not be output
    immediately.  A valid BVBI_Encode_Handle may be created with
    BVBI_Encode_Create.  A valid (but empty) BVBI_Field_Handle may
    be acquired from BVBI_Field_Create.  A BVBI_Field_Handle can
    be made non-empty by calling BVBI_Decode_Data_isr or one of the
    functions BVBI_Field_Set*Data.

    This function is intended to be called from the interrupt
    that marks the beginning of a field of encoded active video.

    Setting the polarity parameter is a bit tricky.  The parameter
    itself refers to the VBI data that is to be encoded into a
    video field.  This field will be either even or odd, but it has
    not been created yet.  Also, this function is called at the
    start of active video for an even or odd field.  But almost
    certainly, the parity of these two fields will be opposite.
    For example, if this function was called from the interrupt for
    start of active video in odd field, the VBI data will be encoded
    soonest if the user specifies even parity.  As a special case,
    if the video being encoded is progressive, then this function
    parameter should always be set to BAVC_Polarity_eFrame.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The encode handle is configured to encode a
                                bulky data format such as teletext, but the
                                field handle is not properly sized for this.
    BVBI_ERR_INVALID_FIELD_POLARITY
                              - The field polarity of the supplied field handle
                                conflicts with the function's polarity argument.
 *****************************************************************************/
BERR_Code BVBI_Encode_Data_isr(
    BVBI_Encode_Handle encodeHandle,    /* [in] A valid BVBI_Encode_Handle
                                                object                       */
    BVBI_Field_Handle   fieldHandle,    /* [in] A valid BVBI_Field_Handle
                                                object.  If NULL, then the VBI
                                                encoders will be disabled for
                                                one field.                   */
    BAVC_Polarity          polarity     /* [in] Which field (even, odd or
                                                frame) to encode data into   */
);


/*****************************************************************************
  Summary:
    Get the interrupt name for VEC/VBI video field start.

  Description:
    This function returns the interrupt name required by the BVBI_Encode_
    submodule, from which context BVBI_Encode_Data_isr() must be called.

    Example;
      BVDC_Display_Handle hDisplay;
      BAVC_VbiPath eVbiPath;
      BINT_Id interruptName;
      BAVC_Polarity eFieldPolarity,
      hDisplay = ...
      BVDC_Display_GetVbiPath (hDisplay, &eVbiPath);
      eFieldPolarity = ...
      BVBI_Encode_GetInterruptName (eVbiPath, eFieldPolarity, &interruptName);
      BINT_CreateCallback (..., interruptName, ...);

  Returns:
    BERR_INVALID_PARAMETER - Some parameter is invalid.
    BERR_SUCCESS - Successfully returned interrupt id

  See Also:
    BVBI_Encode_Data_isr().
 *****************************************************************************/
BERR_Code BVBI_Encode_GetInterruptName(
    BAVC_VbiPath  eVbiPath,       /*  [in] The VEC path to program
                                           interrupts for, obtained from BVDC
                                           porting interface module          */
    BAVC_Polarity eFieldPolarity, /*  [in] The video field polarity to
                                           program interrupts for.           */
    BINT_Id*      pInterruptName  /* [out] The interrupt to use for VEC/VBI
                                           operations with the above VEC
                                           path and video field polarity.    */
);


/*****************************************************************************
  Summary:
    Decodes one field of VBI data

  Description:
    This function takes an active BVBI_Decode_Handle and an allocated
    BVBI_Field_Handle and decodes the next available field(s) into that
    BVBI_Field_Handle object.  The BVBI_Field_Handle object must be
    allocated but need not already be filled with  VBI data.  If it
    contains VBI data, that data is lost.

    This function is intended to be called from the interrupt that
    marks the beginning of a field of decoded active video.

    Setting the polarity parameter is a bit tricky.  The parameter
    itself refers to the VBI data that was collected.  This VBI
    data was collected from either an even or odd field.  Also,
    this function was called at the start of active video for an
    even or odd field.  But almost certainly, the parity of these
    two fields will be opposite.  For example, if this function was
    called from the interrupt for start of active video in odd field,
    then almost certainly, the most recent VBI data available is
    from the previous even field.  As a special case, if the video
    being decoded is progressive, then this function parameter should
    always be set to BAVC_Polarity_eFrame.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The decode handle is configured to decode a
                                bulky data format such as teletext, but the
                                field handle is not properly sized for this.
 *****************************************************************************/
BERR_Code BVBI_Decode_Data_isr(
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object                       */
    BVBI_Field_Handle   fieldHandle,    /* [in] A BVBI_Field_Handle object   */
    BAVC_Polarity          polarity     /* [in] Which field (even, odd, or
                                                frame) to extract data for   */
);


/*****************************************************************************
  Summary:
    Creates new empty field data

  Description:
    This creates new, empty field data for the specified BVBI_Handle.
    This BVBI_Field_Handle structure can be filled with data by
    functions like BVBI_Field_SetCCData, and encoded using
    BVBI_Encode_Data_isr.

    A BVBI_Field_Handle is a container that holds VBI data.
    It has the capacity to simultaneously hold all the closed
    caption data, WSS data, and CGMS data that can occur in a
    single field of video.  However, all other types of VBI
    data are so bulky that by default, a BVBI_Field_Handle
    will not hold them.  The user must call a function such
    as BVBI_Field_TT_Allocate() in order to change the holding
    capacity of his BVBI_Field_Handle to match his application.

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_Create(
    BVBI_Field_Handle* pFieldHandle,    /* [out] A pointer to a
                                                 BVBI_Field_Handle object */
    BVBI_Handle vbiHandle               /*  [in] A valid BVBI_Field_Handle
                                                 object                    */
);


/*****************************************************************************
  Summary:
    Destroys a field data structure

  Description:
    fieldHandle should be a valid, allocated BVBI_Field_Handle structure
    created by BVBI_Field_Create.  This function frees the structure and all
    associated data.

  Returns:
    BERR_SUCCESS              - The handle was successfully destroyed.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_Destroy(
    BVBI_Field_Handle fieldHandle   /* [in] A BVBI_Field_Handle object */
);


/*****************************************************************************
  Summary:
    Changes how much teletext data a field data structure is capable
    of holding.

  Description:
    fieldHandle should be a valid, allocated BVBI_Field_Handle
    structure created by BVBI_Field_Create. This function changes the
    internal properties of the structure so that it is capable
    of holding a specific amount of teletext data.

    When a BVBI_Field_Handle is first created (by calling
    BVBI_Field_Create()), it is not capable of holding any
    teletext data at all.  before it is capable of holding
    teletext data, BVBI_Field_TT_Allocate() must be called.

    Calling this function will cause any teletext data contained in
    the BVBI_Field_Handle to be lost!

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_TT_Allocate(
    BVBI_Field_Handle fieldHandle,  /* [in] A BVBI_Field_Handle object       */
    uint8_t            ucMaxLines,  /* [in] The maximum number of lines of
                                            teletext data that the above
                                            fieldHandle will be capable of
                                            holding.  Set this to zero to
                                            make the fieldHandle as small as
                                            possible (no teletext data may
                                            be stored within).  For NTSC
                                            video, this number never needs
                                            to be any larger than 11.  For
                                            PAL video, this number never
                                            needs to be any larger than 18.  */
    uint8_t            ucLineSize   /* [in] The number of characters per
                                            line of teletext data that the
                                            above fieldHandle will be hold.
                                            For NTSC video, this must be 34.
                                            For PAL video, this must be 43.
                                            Special case: if the above
                                            parameter ulMaxLines is zero,
                                            then this parameter is ignored   */
);

/*****************************************************************************
  Summary:
    Changes whether or not a field data structure is capable
    of of holding VPS data.

  Description:
    fieldHandle should be a valid, allocated BVBI_Field_Handle
    structure created by BVBI_Field_Create. This function changes the
    internal properties of the structure so that it is capable
    (or not) of holding VPS data.

    When a BVBI_Field_Handle is first created (by calling
    BVBI_Field_Create()), it is not capable of holding VPS
    data.  before it is capable of holding
    VPS data, BVBI_Field_VPS_Allocate() must be called.

    Calling this function will cause any VPS data contained in
    the BVBI_Field_Handle to be lost!
  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_VPS_Allocate(
    BVBI_Field_Handle fieldHandle,  /* [in] A BVBI_Field_Handle object       */
    bool                  bEnable   /* [in] If true, the BVBI_Field_Handle
                                            object will be made capable of
                                            holding VPS data.  If false, the
                                            object will lose this
                                            capability, thus saving memory.  */
);

/*****************************************************************************
  Summary:
    Changes whether or not a field data structure is capable
    of of holding Gemstar data.

  Description:
    fieldHandle should be a valid, allocated BVBI_Field_Handle
    structure created by BVBI_Field_Create. This function changes the
    internal properties of the structure so that it is capable
    (or not) of holding Gemstar data.

    When a BVBI_Field_Handle is first created (by calling
    BVBI_Field_Create()), it is not capable of holding Gemstar
    data.  before it is capable of holding
    Gemstar data, BVBI_Field_GS_Allocate() must be called.

    Calling this function will cause any Gemstar data contained in
    the BVBI_Field_Handle to be lost!
  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_GS_Allocate(
    BVBI_Field_Handle fieldHandle,  /* [in] A BVBI_Field_Handle object       */
    bool                  bEnable   /* [in] If true, the BVBI_Field_Handle
                                            object will be made capable of
                                            holding Gemstar data.  If false,
                                            the object will lose this
                                            capability, thus saving memory.  */
);

/*****************************************************************************
  Summary:
    Changes what type of AMOL data (if any) a field handle is capable of
    holding.

  Description:
    fieldHandle should be a valid, allocated BVBI_Field_Handle
    structure created by BVBI_Field_Create. This function changes the
    internal properties of the structure so that it is capable
    (or not) of holding AMOL data. Depending on its arguments, the field
    handle will then be capable of holding AMOL I, AMOL II 1 Mbit/s., or
    AMOL II 2 Mbit/s. data. Or, none at all (returns allocated memory).

    When a BVBI_Field_Handle is first created (by calling
    BVBI_Field_Create()), it is not capable of holding AMOL
    data.  before it is capable of holding AMOL data,
    BVBI_Field_AMOL_Allocate() must be called.

    Calling this function will cause any AMOL data contained in
    the BVBI_Field_Handle to be lost!
  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_AMOL_Allocate(
    BVBI_Field_Handle fieldHandle,  /* [in] A BVBI_Field_Handle object       */
    BVBI_AMOL_Type       amolType   /* [in] Which type of AMOL to allocate
                                            space for. As a special case,
                                            BVBI_AMOL_Type_None removes all
                                            capability, thus saving memory.  */
);

/*****************************************************************************
  Summary:
    Adds or removes space in a field handle to store multi-line closed caption
    data.

  Description:
    fieldHandle should be a valid, allocated BVBI_Field_Handle
    structure created by BVBI_Field_Create. This function changes the
    internal properties of the structure so that it is capable
    (or not) of holding multi-line closed caption data. Depending
    on its arguments, the field handle will then be capable of
    holding this data, or not.

    When a BVBI_Field_Handle is first created (by calling
    BVBI_Field_Create()), it is not capable of holding multi-line
    closed caption data.  before it is capable of holding this
    data, BVBI_Field_MCC_Allocate() must be called.

    Calling this function will cause any multi-line closed
    caption data contained in the BVBI_Field_Handle to be lost!

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_MCC_Allocate(
    BVBI_Field_Handle fieldHandle,  /* [in] A BVBI_Field_Handle object       */
    bool                    alloc   /* [in] If true, space for the MCC
                                            data is allocated. If false,
                                            this space (capability) is
                                            removed, thus saving memory.     */
);

/*****************************************************************************
  Summary:
    Changes what type of SCTE data (if any) a field handle is capable of
    holding.

  Description:
    fieldHandle should be a valid, allocated BVBI_Field_Handle
    structure created by BVBI_Field_Create. This function changes the
    internal properties of the structure so that it is capable
    (or not) of holding SCTE data. Depending on its arguments, the field
    handle will then be capable of holding closed caption (like) data only,
    closed caption plus NRTV, closed caption plus PAM, or closed caption plus
    monochrome. Or, none at all (returns allocated memory).

    Calling this routine is destructive. Regardless of the arguments supplied,
    all SCTE data contained in the field handle will be destroyed.

    When a BVBI_Field_Handle is first created (by calling
    BVBI_Field_Create()), it is not capable of holding SCTE
    data.  before it is capable of holding SCTE data,
    BVBI_Field_SCTE_Allocate() must be called.

    Calling this function will cause any SCTE data contained in
    the BVBI_Field_Handle to be lost!
  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_SCTE_Allocate(
    BVBI_Field_Handle fieldHandle,  /* [in] A BVBI_Field_Handle object       */
    size_t                cc_size,  /* [in] Number of pieces of closed
                                            caption data to make room for.   */
    bool           scteEnableNrtv,  /* [in] Whether to allocate space to
                                            make SCTE NRTV data processing
                                            possible.                        */
    size_t               pam_size,  /* [in] Number of bytes of luma_PAM_data
                                            to make room for.                */
    bool           scteEnableMono   /* [in] Whether to allocate space to
                                            make ETSI EN 301 775 monochrome
                                            data processing possible.        */
);

/*****************************************************************************
  Summary:
    Changes whether or not a field data structure is capable
    of of holding CGMS-B data.

  Description:
    fieldHandle should be a valid, allocated BVBI_Field_Handle
    structure created by BVBI_Field_Create. This function changes the
    internal properties of the structure so that it is capable
    (or not) of holding CGMS-B data.

    When a BVBI_Field_Handle is first created (by calling
    BVBI_Field_Create()), it is not capable of holding CGMS-B
    data.  before it is capable of holding
    CGMS-B data, BVBI_Field_CGMSB_Allocate() must be called.

    Calling this function will cause any Gemstar data contained in
    the BVBI_Field_Handle to be lost!
  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_CGMSB_Allocate(
    BVBI_Field_Handle fieldHandle,  /* [in] A BVBI_Field_Handle object       */
    bool                  bEnable   /* [in] If true, the BVBI_Field_Handle
                                            object will be made capable of
                                            holding CGMS-B data.  If false,
                                            the object will lose this
                                            capability, thus saving memory.  */
);


/*****************************************************************************
  Summary:
    Sets options for WSS decoding

  Description:
    This function takes an active BVBI_Decode_Handle and sets options
    related to WSS decoding. These options take effect at the next call to
    BVBI_Decode_ApplyChanges.

    This function allows the user to decode WSS data from a non-standard
    video line number.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_SetWssOptions(
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object                       */
    uint16_t              vline576i     /* [in] The video line number to use
                                                for 576I/PAL decoding.       */
);


/*****************************************************************************
  Summary:
    Reports options used for WSS decoding

  Description:
    This function takes an active BVBI_Decode_Handle and reports the options
    related to WSS decoding.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_GetWssOptions(
    BVBI_Decode_Handle decodeHandle,   /*  [in] A valid BVBI_Decode_Handle
                                                object                       */
    uint16_t*            pVline576i    /* [out] The video line number used
                                                for 576I/PAL decoding.       */
);


/*****************************************************************************
  Summary:
    Sets options for Gemstar decoding

  Description:
    This function takes an active BVBI_Decode_Handle and sets options
    related to Gemstar decoding. These options take effect at the next call to
    BVBI_Decode_ApplyChanges.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_SetGemstarOptions(
    BVBI_Decode_Handle  decodeHandle,   /* [in] A valid BVBI_Decode_Handle
                                                object                       */
    const BVBI_GSOptions* pGsOptions    /* [in] Options for Gemstar input.   */
);


/*****************************************************************************
  Summary:
    Gets options for Gemstar decoding

  Description:
    This function takes an active BVBI_Decode_Handle and sets options
    related to Gemstar decoding. These options take effect at the next call to
    BVBI_Decode_ApplyChanges.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Decode_GetGemstarOptions(
    BVBI_Decode_Handle decodeHandle,    /* [in] A valid BVBI_Decode_Handle
                                                object                       */
    BVBI_GSOptions*      pGsOptions    /* [out] Options for Gemstar input.   */
);


/*****************************************************************************
  Summary:
    Gets field parity attribute of VBI field data handle

  Description:
    This function returns the "field parity" attribute of a BVBI_Field_Handle.
    The attribute is a bitfield, and it will be one of: zero,
    (1<<BAVC_Polarity_eTopField), (1<<BAVC_Polarity_eBotField), or
    ((1<<BAVC_Polarity_eTopField) | (1<<BAVC_Polarity_eBotField)).  The last
    value indicates that the VBI data is suitable for encoding in either a top
    field or a bottom field.  Other values will cause encoding of this VBI data
    to fail.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_GetPolarity_isr(
    BVBI_Field_Handle vbiHandle,    /*  [in] A valid BVBI_Field_Handle
                                             object                    */
    uint32_t      *pPolarityMask    /* [out] The bitmask indicating
                                             field polarity            */
);


/*****************************************************************************
  Summary:
    Sets field parity attribute of VBI field data handle

  Description:
    This function sets the "field parity" attribute of a BVBI_Field_Handle.
    The attribute is a bitfield, and it should be one of:
    (1<<BAVC_Polarity_eTopField), (1<<BAVC_Polarity_eBotField), or
    ((1<<BAVC_Polarity_eTopField) | (1<<BAVC_Polarity_eBotField)).  The last
    value indicates that the VBI data is suitable for encoding in either a top
    field or a bottom field.  Other values will cause encoding of this VBI data
    to fail.

    A BVBI_Field_Handle "filled" by calling BVBI_Decode_Data_isr will have this
    attribute set already.  However, the user may still wish to use
    BVBI_Field_SetPolarity_isr in this case.  For example, setting the
    attribute to ((1<<BAVC_Polarity_eTopField) | (1<<BAVC_Polarity_eBotField))
    will cause the BVBI_Field_Handle to be encoded as quickly as possible;
    the user will not have to wait for a field of the desired polarity to occur.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_SetPolarity_isr(
    BVBI_Field_Handle vbiHandle,    /*  [in] A valid BVBI_Field_Handle
                                             object                    */
    uint32_t        polarityMask    /*  [in] The bitmask indicating
                                             field polarity            */
);

/*****************************************************************************
  Summary:
    Gets information about a decoding error

  Description:
    This function returns more information for the most recent encode
    or decode error on the specified BVBI_Field_Handle.  pErrInfo has
    a bit set for each of the various error types that has occurred
    on it.  See the earlier parts of this header for the error
    constants.

    Calling BVBI_Encode_Data_isr or BVBI_Decode_Data_isr on the
    BVBI_Field_Handle will clear the BVBI_Field_Handle's error data
    and replace it with data for the most recent call.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_GetErrorInfo_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                    */
    uint32_t            *pErrInfo   /* [out] The bitmask to use for
                                             returning line errors     */
);


/*****************************************************************************
  Summary:
    Gets CC data from the supplied field data

  Description:
    This function extracts one field's worth of CC data from a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.

    If the function returns an error, the user may immediately
    call BVBI_Field_GetErrorInfo_isr() to determine the cause of the error.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetCCData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                          */
    uint8_t           *pucLowByte,  /* [out] A pointer to an 8-bit int for
                                             CC data.  This is the "low
                                             byte" of closed caption data,
                                             as defined in the spec.  For
                                             some applications, the user
                                             could "AND" this data with the
                                             value 0x7F in order to clear
                                             the parity bits.  As an aid to
                                             troubleshooting, this function
                                             will return data even if there
                                             is a parity error.  For all
                                             other cases, 0x00 is returned.  */
    uint8_t          *pucHighByte   /* [out] A pointer to an 8-bit int for
                                             CC data.  This is the "high
                                             byte" of closed caption data,
                                             as defined in the spec.  The
                                             comments regarding the above
                                             argument hold for this one as
                                             well.                           */
);


/*****************************************************************************
  Summary:
    Sets CC data in the supplied field data

  Description:
    This function writes one field's worth of CC data to a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.  It may be
    encoded using BVBI_Encode_Data_isr.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_SetCCData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                          */
     uint8_t             cLowByte,  /*  [in] 8 bits of closed caption data
                                             to insert.  This is the "low
                                             byte" of closed caption data,
                                             as defined in the spec.         */
    uint8_t             cHighByte   /*  [in] 8 bits of closed caption data
                                             to insert.  This is the "high
                                             byte" of closed caption data,
                                             as defined in the spec.         */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Clears CC data in the supplied field data

  Description:
    This function flags a field data structure as having no closed caption
    data.  It effectively undoes the action of BVBI_Field_SetCCData_isr().

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_ClearCCData_isr(
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);
#endif


/*****************************************************************************
  Summary:
    Gets CGMS-A data from the supplied field data

  Description:
    This function extracts one field's worth of CGMS-A data from a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetCGMSAData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                        */
    uint32_t         *pulCGMSData   /* [out] A pointer to a 32-bit int for
                                             CGMS-A data                   */
);
/* Backward compatibility, DEPRECATED! */
#define BVBI_Field_GetCGMSData_isr BVBI_Field_GetCGMSAData_isr


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets CGMS-B data from the supplied field data

  Description:
    This function extracts one field's worth of CGMS-B data from a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetCGMSBData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                        */
    BAVC_VBI_CGMSB_Datum*  pDatum   /* [out] CGMS-B data contained in
                                             the above field handle.       */
);
#endif


/*****************************************************************************
  Summary:
    Sets CGMS-A data in the supplied field data

  Description:
    This function writes one field's worth of CGMS-A data to a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.  It may be
    encoded using BVBI_Encode_Data_isr.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_SetCGMSAData_isr(
    BVBI_Field_Handle fieldHandle,  /* [in] A valid BVBI_Field_Handle object */
    uint32_t           ulCGMSData   /* [in] A 32-bit int containing CGMS-A
                                            data                             */
);
/* Backward compatibility, DEPRECATED! */
#define BVBI_Field_SetCGMSData_isr BVBI_Field_SetCGMSAData_isr


/*****************************************************************************
  Summary:
    Sets CGMS-B data in the supplied field data

  Description:
    This function writes one field's worth of CGMS-B data to a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.  It may be
    encoded using BVBI_Encode_Data_isr.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_SetCGMSBData_isr(
    BVBI_Field_Handle fieldHandle,  /* [in] A valid BVBI_Field_Handle object */
    BAVC_VBI_CGMSB_Datum*  pDatum   /* [in] CGMS-B data to copy in to the
                                             above field handle.             */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Clears CGMS-A data in the supplied field data

  Description:
    This function flags a field data structure as having no closed caption
    data.  It effectively undoes the action of BVBI_Field_SetCGMSAData_isr().

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_ClearCGMSAData_isr(
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);
/* Backward compatibility, DEPRECATED! */
#define BVBI_Field_ClearCGMSData_isr BVBI_Field_ClearCGMSAData_isr
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Clears CGMS-B data in the supplied field data

  Description:
    This function flags a field data structure as having no closed caption
    data.  It effectively undoes the action of BVBI_Field_SetCGMSBData_isr().

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_ClearCGMSBData_isr(
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);
#endif


/*****************************************************************************
  Summary:
    Gets WSS data from the supplied field data

  Description:
    This function extracts one field's worth of WSS data from a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetWSSData_isr(
    BVBI_Field_Handle fieldHandle,  /* [in] A valid BVBI_Field_Handle object */
    uint16_t          *pusWSSData   /* [in] A pointer to a 16-bit int for
                                            WSS data                         */
);


/*****************************************************************************
  Summary:
    Sets WSS data in the supplied field data

  Description:
    This function writes one field's worth of WSS data to a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.  It may be
    encoded using BVBI_Encode_Data_isr.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_SetWSSData_isr(
    BVBI_Field_Handle fieldHandle,  /* [in] A valid BVBI_Field_Handle object */
    uint16_t            usWSSData   /* [in] A 16-bit int containing WSS data */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Clears WSS data in the supplied field data

  Description:
    This function flags a field data structure as having no closed caption
    data.  It effectively undoes the action of BVBI_Field_SetWSSData_isr().

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_ClearWSSData_isr(
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);
#endif


/*****************************************************************************
  Summary:
    Gets teletext data from the supplied field data

  Description:
    This function extracts one field's worth of teletext data from a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.  In particular, if there are
                                more than ttLine's of teletext data waiting for
                                the caller, then this function will fail
                                completely and return this error code.
    BVBI_ERR_FLDH_CONFLICT    - The field handle does not have the proper
                                allocation to call this function.  The
                                allocation should have been done by calling
                                BVBI_Field_TT_Allocate.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetTTData_isr (
    BVBI_Field_Handle fieldHandle, /*  [in] A valid BVBI_Field_Handle object */
    int                    nLines, /*  [in] The number of entries in the
                                            following array                  */
    BAVC_VBI_TT_Line *    ttLines  /* [out] An array of teletext data
                                            structures..  Each element of
                                            the array is a line of teletext
                                            data.  If an element has a
                                            ucFramingCode attribute equal
                                            to
                                            BVBI_TT_INVALID_FRAMING_CODE,
                                            then that line of data is
                                            invalid or not present.          */
);


/*****************************************************************************
  Summary:
    Gets Gemstar data from the supplied field data

  Description:
    This function extracts one field's worth of Gemstar data from a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.  In particular, if there are
                                more than ttLine's of Gemstar data waiting for
                                the caller, then this function will fail
                                completely and return this error code.
    BVBI_ERR_FLDH_CONFLICT    - The field handle does not have the proper
                                allocation to call this function.  The
                                allocation should have been done by calling
                                BVBI_Field_GS_Allocate.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetGSData_isr (
    BVBI_Field_Handle fieldHandle, /*  [in] A valid BVBI_Field_Handle object */
    BAVC_VBI_GSData *      pGSData /* [out] One field's worth of Gemstar
                                            data.                            */
);


/*****************************************************************************
  Summary:
    Sets teletext data in the supplied field data

  Description:
    This function writes one field's worth of teletext data to a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.  It may be
    encoded using BVBI_Encode_Data_isr.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The field handle does not have the proper
                                allocation to call this function.  The
                                allocation should have been done by calling
                                BVBI_Field_TT_Allocate.
 *****************************************************************************/
BERR_Code BVBI_Field_SetTTData_isr (
    BVBI_Field_Handle fieldHandle, /*  [in] A valid BVBI_Field_Handle object */
    BFMT_VideoFmt    eVideoFormat, /*  [in] The video standard in use.
                                            Determines line width.           */
    int                    nLines, /*  [in] The number of entries in the
                                            following array                  */
    BAVC_VBI_TT_Line *    ttLines  /*  [in] An array of teletext data
                                            structures..  Each element of
                                            the array is a line of teletext
                                            data.  If an element has a
                                            ucFramingCode attribute equal
                                            to
                                            BVBI_TT_INVALID_FRAMING_CODE,
                                            then that line of data will not
                                            be encoded.                      */
);


/*****************************************************************************
  Summary:
    Sets Gemstar data in the supplied field data

  Description:
    This function writes one field's worth of Gemstar data to a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.  It may be
    encoded using BVBI_Encode_Data_isr.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The field handle does not have the proper
                                allocation to call this function.  The
                                allocation should have been done by calling
                                BVBI_Field_TT_Allocate.
 *****************************************************************************/
BERR_Code BVBI_Field_SetGSData_isr (
    BVBI_Field_Handle fieldHandle, /*  [in] A valid BVBI_Field_Handle object */
    BAVC_VBI_GSData *      pGSData /*  [in] One field's worth of Gemstar
                                            data.                            */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Clears teletext data in the supplied field data

  Description:
    This function flags a field data structure as having no teletext
    data.  It effectively undoes the action of BVBI_Field_SetTTData_isr().

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_ClearTTData_isr(
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Clears Gemstar data in the supplied field data

  Description:
    This function flags a field data structure as having no Gemstar
    data.  It effectively undoes the action of BVBI_Field_SetGSData_isr().

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_ClearGSData_isr(
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);
#endif


/*****************************************************************************
  Summary:
    Gets VPS data from the supplied field data

  Description:
    This function extracts one field's worth of VPS data from a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The field handle does not have the proper
                                allocation to call this function.  The
                                allocation should have been done by calling
                                BVBI_Field_VPS_Allocate.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetVPSData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                      */
    BAVC_VBI_VPSData    *pVPSData   /* [out] A pointer to a the VPS data */
);


/*****************************************************************************
  Summary:
    Sets VPS data in the supplied field data

  Description:
    This function writes one field's worth of VPS data to a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.  It may be
    encoded using BVBI_Encode_Data_isr.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The field handle does not have the proper
                                allocation to call this function.  The
                                allocation should have been done by calling
                                BVBI_Field_VPS_Allocate.
 *****************************************************************************/
BERR_Code BVBI_Field_SetVPSData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                      */
    BAVC_VBI_VPSData    *pVPSData   /* [out] A pointer to a the VPS data */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Clears VPS data in the supplied field data

  Description:
    This function flags a field data structure as having no closed caption
    data.  It effectively undoes the action of BVBI_Field_SetVPSData_isr().

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_ClearVPSData_isr(
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets AMOL data from the supplied field data

  Description:
    This function extracts one field's worth of AMOL data from a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.

    The AMOL data byte array is converted to a waveform as follows.
     - 0x01 0x00 0x00 ...
       This is the pulse that occurs earliest in time.
     - 0x80 0x00 0x00 ...
       This is a pulse that occurs later in time than above.
     - 0x00 0x01 0x00 ...
       This is a pulse that occurs later in time than above.
     - and so forth.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The field handle does not have the proper
                                allocation to call this function.  The
                                allocation should have been done by calling
                                BVBI_Field_AMOL_Allocate.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetAMOLData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                          */
    BVBI_AMOL_Type     *pAmolType,  /* [out] Which type of AMOL data is
                                             returned.                       */
    uint8_t            *pAMOLData,  /* [out] A pointer to up to 24 bytes
                                             of AMOL data. See Description
                                             section for details.            */
    unsigned int*         pLength   /* [out] size of data pointed to by
                                             pAMOLData in bytes              */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Gets multi-line closed caption data from the supplied field data

  Description:
    This function extracts one field's worth of multi-line closed
    caption data from a BVBI_Field_Handle structure.  That field data
    may come from BVBI_Decode_Data_isr or BVBI_Field_Create originally.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetMCCData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                          */
    BAVC_VBI_MCCData*   pMCCData    /* [out] A pointer to multi-line closed
                                               caption data.                 */
);
#endif


/*****************************************************************************
  Summary:
    Gets SCTE data from the supplied field data

  Description:
    This function extracts one field's worth of SCTE data from a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The supplied BVBI_SCTE_Data does not have
                                room to hold the SCTE data in the field
                                handle.
    BVBI_ERR_FIELD_NODATA     - No SCTE data in the field handle.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetSCTEData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                          */
    BVBI_SCTE_Data     *pScteData   /* [out] A pointer to SCTE data. For
                                             each data pointer element in
                                             this stuct, if the
                                             corresponding _size element is
                                             checcked to determine if there
                                             is room to copy out the data.
                                             Because of this complexity, it
                                             is probably required to call
                                             BVBI_Field_GetSCTEData_Size_isr
                                             prior to calling
                                             BVBI_Field_GetSCTEData_isr.     */
);


/*****************************************************************************
  Summary:
    Gets SCTE count information from the supplied field data

  Description:
    This function discloses how much SCTE data is contained in a
    BVBI_Field_Handle structure.  The information returned is the
    amount of useful data, NOT the maximum holding capacity of
    the structure.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetSCTEData_Count_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                          */
    size_t*   pcc_count,            /* [out] Number of meaningful SCTE
                                             closed caption data entries in
                                             the field handle.               */
    size_t* pnrtv_count,            /* [out] Number of meaningful SCTE
                                             NRTV data entries in the field
                                             handle.                         */
    size_t*  ppam_count,            /* [out] Number of meaningful SCTE PAM
                                             data bytes in the field handle. */
    size_t* pmono_count             /* [out] Number of meaningful SCTE
                                             monochrome bytes contained in
                                             each of the two monochrome data
                                             line stores, in the field
                                             handle.                         */
);


/*****************************************************************************
  Summary:
    Gets SCTE count information from the supplied field data

  Description:
    This function discloses the capacity to hold SCTE data in a
    BVBI_Field_Handle structure.  The information returned is the
    amount of space available, NOT the amount of useful data currently in
    the structure.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FIELD_BADDATA    - Requested data has a problem.  Call
                                BVBI_Field_GetErrorInfo_isr for further info.
 *****************************************************************************/
BERR_Code BVBI_Field_GetSCTEData_Size_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                          */
    size_t*    pcc_size,            /* [out] Number of meaningful SCTE
                                             closed caption data entries
                                             that the field handle can hold. */
    size_t*  pnrtv_size,            /* [out] Number of SCTE NRTV data
                                             entries that the field handle
                                             can hold.                       */
    size_t*   ppam_size,            /* [out] Number of SCTE PAM data bytes
                                             that the field handle can hold. */
    size_t*  pmono_size             /* [out] Number of SCTE monochrome bytes
                                             contained in each of the two
                                             monochrome data line stores,
                                             that the field handle can hold. */
);


/*****************************************************************************
  Summary:
    Sets AMOL data in the supplied field data

  Description:
    This function writes one field's worth of AMOL data to a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.  It may be
    encoded using BVBI_Encode_Data_isr.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The field handle does not have the proper
                                allocation to call this function.  The
                                allocation should have been done by calling
                                BVBI_Field_AMOL_Allocate.
 *****************************************************************************/
BERR_Code BVBI_Field_SetAMOLData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                          */
    BVBI_AMOL_Type        amolType, /*  [in] Which type of AMOL data is
                                             supplied.                       */
    uint8_t             *pAMOLData, /*  [in] A pointer to up to 24 bytes
                                             of AMOL data. See
                                             BVBI_Field_GetAMOLData_isr()
                                             for details about this data.    */
    unsigned int            length  /*  [in] size of data pointed to by
                                             pAMOLData in bytes              */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Sets multi-line closed caption data in the supplied field data

  Description:
    This function writes one field's worth of multi-line closed
    caption data to a BVBI_Field_Handle structure.  That field data
    may come from BVBI_Decode_Data_isr or BVBI_Field_Create
    originally.  It may be encoded using BVBI_Encode_Data_isr.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_SetMCCData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                          */
    BAVC_VBI_MCCData*    pMCCData   /*  [in] A pointer to multi-line closed
                                             caption data.                   */
);
#endif


/*****************************************************************************
  Summary:
    Sets SCTE data in the supplied field data

  Description:
    This function writes one field's worth of SCTE data to a
    BVBI_Field_Handle structure.  That field data may come from
    BVBI_Decode_Data_isr or BVBI_Field_Create originally.  It may be
    encoded using BVBI_Encode_Data_isr.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The field handle does not have the proper
                                allocation to call this function.  The
                                allocation should have been done by calling
                                BVBI_Field_SCTE_Allocate.
 *****************************************************************************/
BERR_Code BVBI_Field_SetSCTEData_isr(
    BVBI_Field_Handle fieldHandle,  /*  [in] A valid BVBI_Field_Handle
                                             object                      */
    BVBI_SCTE_Data      *pScteData  /*  [in] SCTE data to apply.         */
);


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Clears AMOL data in the supplied field data

  Description:
    This function flags a field data structure as having no closed caption
    data.  It effectively undoes the action of BVBI_Field_SetAMOLData_isr().

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_ClearAMOLData_isr(
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Clears multi-line closed caption data in the supplied field data

  Description:
    This function flags a field data structure as having no closed caption
    data.  It effectively undoes the action of BVBI_Field_SetMCCData_isr().

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_ClearMCCData_isr(
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);
#endif


#if !B_REFSW_MINIMAL
/*****************************************************************************
  Summary:
    Clears SCTE data in the supplied field data

  Description:
    This function flags a field data structure as having no closed caption
    data.  It effectively undoes the action of BVBI_Field_SetSCTEData_isr().

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVBI_Field_ClearSCTEData_isr(
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);
#endif


/*****************************************************************************
  Summary:
    Determines whether a field handle contains only null data.

  Description:
    This function examines a field handle and determines if the field
    handle contains only null data. This function can be used when a
    software system is suffering from too much VBI data and some of it
    must be discarded. At the present time, the function returns false
    unless both of these conditions are true: The field handle either
    no VBI data or only closed daption data AND any closed caption data
    contains only null (0x80, 0x80) bytes.

  Returns:
    true if the field handle is null according to the above definition. False,
    otherwise.
 *****************************************************************************/
bool BVBI_Field_IsNull (
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);


/*****************************************************************************
  Summary:
    Determines whether a field handle contains only null data. ISR-safe
    version.

  Description:
    ISR-safe version of BVBI_Field_IsNull().

  Returns:
    true if the field handle is null according to the above definition. False,
    otherwise.
 *****************************************************************************/
bool BVBI_Field_IsNull_isr (
    BVBI_Field_Handle fieldHandle   /* [in] A valid BVBI_Field_Handle object */
);


/*****************************************************************************
 * Public convenience functions
 *****************************************************************************/
/* Debug code */
#if 0
void BVBI_Field_Check(BVBI_Field_Handle fieldHandle);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVBI_H__ */

/* end of file */
