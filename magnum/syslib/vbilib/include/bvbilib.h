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

/*= Module Overview *********************************************************
<verbatim>


Overview
BVBIlib module manages queues of data to be encoded and/or decoded in the
vertical blanking interval (VBI) of video material.  It is a convenience layer
for the BVBI module, which it calls into.  BVBIlib also calls into BLST for
queue management functionality, and BKNI for for managing communications
between ISR software and the main thread of execution.

If the BVBIlib module is used, then several BVBI data structures and
functions will also have to be called.  For example, extracting
closed caption data in a convenient format will require the use of BVBI
function BVBI_Field_GetCCData, which in turn requires the use of data structure
BVBI_Field_Handle.  Certain BVBIlib functions return the appropriate reference
to a BVBI_Field_Handle.

Because the user of BVBIlib must also use BVBI, BVBIlib is not simply a layer
on top of BVBI.  The following hardware/software
stack illustrates an application that uses BVBIlib.  Note that the customer
application calls into both BVBIlib and BVBI.  On the other hand, only the BVBI
module interfaces with the chip hardware.

                +-------------------+
                |     customer      |
                |    application    |
                +---------+---------+
                | BVBIlib |         |
                | module  |   BVBI  |
                +---------+  module |
                |                   |
                +---------+---------+
                |                   |
                |     hardware      |
                +-------------------+

Design
The design for BVBIlib is broken into six parts.
  Part 1 (VBI List sub-API)
    These functions manage a list of BVBI_Field_Handles.  The list is used by
    the BVBIlib module as a source of containers for holding VBI data.
  Part 2 (open/close module)
    These functions are used for opening and closing the BVBIlib module.
    These functions need only be called at boot time.  (The close function is
    used rarely).
  Part 3 (open/close/configure decoders)
    These functions are used for opening, closing, and configuring VBI
    decoders.  Opening one decoder makes it possible to decode all types of VBI
    (closed caption, teletext, etc.).
  Part 4 (open/close/configure encoders)
    Similar to the decoder functions, these functions are used for opening,
    closing, and configuring VBI encoders.
  Part 5 (decoder ISR functions)
    These functions will be called by the customer's ISR that responds to start
    of decoded active video.  The ISR is installed using the BINT module.
    There is a BVBIlib function that pulls one field of VBI data out of the
    decoder hardware and makes it available to the rest of the module.  This
    function should be called at the beginning of the ISR.  Subsequently, the
    user may call BVBIlib functions to manipulate the captured VBI data.  There
    is a BVBILIB function to send the VBI data to a VBI encoder queue.
  Part 6 (encoder ISR function)
    This single function will be called by the customer's ISR that responds to
    start of encoded active video.  As above, the ISR is installed using the
    BINT module.  The function sends one field of VBI data from the encoder's
    queue to the encoder hardware.

Note that the only BVBIlib software to run in non-interrupt context is for
configuration.  The only BVBIlib software to run in the encoder ISRs is a
single function call to dequeue a field's worth of VBI data and send it to the
encoder hardware.  Therefore, most of the customer programming will take place
in the decoder ISR.

The BVBIlib module exists primarily to manage lists of VBI field data.  The
contents of these lists are thus BVBI_Field_Handle's.  There are two types of
lists in the design: Encoder queues, and the BVBIlib_List sub-API free list.
  Encoder queues
    When the user creates a BVBIlib encoder, BVBIlib creates an encoder queue.
    For every field of video the encoder hardware processes, it will remove one
    BVBI_Field_Handle from this queue and encode the VBI data it contains (if
    any) in the video.  The user's only interaction with this queue is to
    insert BVBI_Field_Handle's into it.  Typically, these BVBI_Field_Handles
    will originate from a decoder queue, although the user may do some
    processing of his own.
  The BVBIlib_List sub-API free list
    The user must create one or more BVBIlib_List objects to hold
    BVBI_Field_Handle's.  Subsequently, these handles may be allocated by
    a BVBIlib decoder to decode VBI data into.  The user may also allocate
    a handle for his own use.  A handle may also be passed to a BVBIlib
    encoder queue, where it will eventually be used as a source of data
    for VBI encoding.  Afterwards, the BVBIlib encoder will "return" the
    handle to the free list, unless there is another entity (the user,
    or another BVBIlib encoder) that is still using the handle.  See the
    next section for a discussion on managing the BVBI_Field_Handle's.

Managing The Collection of BVBI_Field_Handles
The BVBIlib module manages lists of BVBI_Field_Handles for use in encoding
and decoding VBI.  This was discussed in the previous section.  The management
of these handles has the following features:

 o All memory allocation and deallocation is performed by the BVBIlib_List
   sub-API.  Memory leaks are avoided.
 o The user may reserve a BVBI_Field_Handle for his own use, and then return it
   to the BVBIlib_List sub-API after he is done with it.
 o Each BVBI_Field_Handle allocated by BVBIlib has a usage count associated
   with it.  This allows a handle to be used by several "clients" at once.  The
   most extreme case of this is that a handle can be enqueued for encoding on
   both VEC cores, and also retained for reading by user software.  BVBIlib
   recycles a handle when its usage count drops to zero.

The following diagram indicates buffer usage in the BVBIlib module.

+-+
| | indicates a logically empty BVBI_Field_Handle
+-+

+-+
|x| indicates a BVBI_Field_Handle that contains useful data
+-+

                        BVBIlib_List sub-API free list
                            +-+-+-+-+-+-+-+-+-+-+
      +-------------------< | | | | | | | | | | | <--------------------+
      |                     +-+-+-+-+-+-+-+-+-+-+                      |
      |                                                                |
      |                                                                |
      |                                                                |
      |                                                                |
      |             +-+                               +-+              |
      +--->((O))--->| |---->(user s/w)---+----------->| | --->((O))----+
      |  BVBIlib    +-+                  |            +-+   BVBIlib    |
      |  software                        |                  software   |
      |  (reserve)                       |                  (return)   |
      |                                  |                             |
      |     ___     +-+                  |    +-+-+-+-+-+      ___     |
      +--->((O))--->|x|---->(user s/w)---+--->|x|x|x|x|x| --->((O))----+
          Decoder   +-+                  |    +-+-+-+-+-+    Encoder   |
         hardware   Decoder              |     Encoder      hardware   |
                    output               |      queue                  |
                                         |                             |
                                         |                             |
                                         |                             |
                                         |    +-+-+-+-+-+      ___     |
                                         +--->|x|x|x|x|x| --->((O))----+
                                              +-+-+-+-+-+    Encoder
                                               Encoder      hardware
                                                queue


Working with BVBI_Field_Handles and Usage Counts

The BVBIlib will take care of managing its collection of BVBI_Field_Handles and
prevent memory leaks related to these handles.  The concept of usage count is
central to the management scheme, and the user of BVBIlib must follow certain
rules for this scheme to succeed.

Certain functions cause a BVBI_Field_Handle to be given out to the user.
These functions are
 o BVBIlib_Decode_Data_isr()
 o BVBIlib_List_Obtain_isr()
These functions produce a BVBI_Field_Handle for the user to manipulate as he
wishes.  The usage count of a BVBI_Field_Handle obtained in this manner is
zero.

A function is provided so that the user can return a
BVBI_Field_Handle and enqueue it for encoding:
 o BVBIlib_Encode_Enqueue_isr()
This function accepts a BVBI_Field_Handle from the user.  In most cases, the
user should no longer access the BVBI_Field_Handle after calling this
function.  There are exceptions, described below.  In all cases, the BVBIlib
increments the usage count of the BVBI_Field_Handle as it is enqueued.

A function is provided that encodes the VBI data in the oldest
BVBI_Field_Handle in an encoder's queue:
 o BVBIlib_Encode_Data_isr()
After this function encodes the VBI data, it decrements the use count of the
BVBI_Field_handle.  If the usage count is then zero, BVBIlib recycles the
BVBI_Field_Handle for other use.  This is the purpose of the usage count.

Another function is provided so that the user can return a BVBI_Field_Handle
without encoding it:
 o BVBIlib_List_Return_isr()
This function puts a BVBI_Field_Handle onto a BVBIlib_List free list.  The
BVBI_Field_Handle is thus "recycled," and available for use again.

The simplest use case of the BVBIlib is to simply decode a field of VBI data
and immediately encode it.  From the point of view of memory (leak) management,
the usage is

   BVBIlib_Decode_Data_isr()     // Returns a BVBI_Field_Handle, usage count 0.
   BVBIlib_Encode_Enqueue_isr()  // Enqueues the above BVBI_Field_Handle, usage
                                 // count will increment to 1.
   BVBIlib_Encode_Data_isr()     // Dequeues the BVBI_field_Handle and encodes
                                 // its VBI data.  The usage count drops to
                                 // zero, so the BVBI_Field_Handle is recycled
                                 // to a BVBIlib_List free list.

In the above example, the user must NOT access the BVBI_Field_Handle after
BVBIlib_Encode_Enqueue_isr() is called, because the BVBI_Field_Handle is
subject to reuse by BVBIlib.  The above programming example is a simple obtain,
return sequence.

Another use case is the situation where the user decodes VBI data, but does not
re-encode it into an outgoing video destination.  From the point of view of
memory (leak) management, the usage is

   BVBIlib_Decode_Data_isr()     // Returns a BVBI_Field_Handle, usage count 0.
   ...                           // User processes BVBI_Field_Handle as he
                                 // wishes.
   BVBIlib_List_Return_isr()     // User returns the above BVBI_Field_Handle to
                                 // BVBIlib for recycling to BVBIlib_list free
                                 // list.

Once again, the user must NOT access the BVBI_Field_Handle after
BVBIlib_List_Return_isr() is called.  The above example is once again a simple
obtain, return sequence.

A similar use case is the situation where the user does not decode any VBI
data, but synthesizes it to be encoded into outgoing video.  From the point of
view of memory (leak) management, the usage is

   BVBIlib_List_Obtain_isr()     // Gives a BVBI_Field_Handle to the user.
                                 // Usage count is 0.
   ...                           // User fills the BVBI_Field_Handle with data
                                 // of his own creation.
   BVBIlib_Encode_Enqueue_isr()  // Enqueues the above BVBI_Field_Handle, usage
                                 // count will increment to 1.
   BVBIlib_Encode_Data_isr()     // Dequeues the BVBI_field_Handle and encodes
                                 // its VBI data.  The usage count drops to
                                 // zero, so the BVBI_Field_Handle is recycled
                                 // to a BVBIlib_List free list.

Once again, the user must NOT access the BVBI_Field_Handle after
BVBIlib_Encode_Enqueue_isr() is called.  Again, the example
illustrates a simple obtain, return.

Here is a slight complication.  Suppose that an application is to
decode VBI from the incoming video source, and re-encode it into
TWO video destinations.  The use count prevents BVBI_Field_Handles
from being recycled too soon:

   BVBIlib_Decode_Data_isr()     // Returns a BVBI_Field_Handle, usage count 0.
   BVBIlib_Encode_Enqueue_isr()  // Usage count increments to 1.
   BVBIlib_Encode_Enqueue_isr()  // Usage count increments to 2.
   BVBIlib_Encode_Data_isr()     // Usage count decrements to 1.
   BVBIlib_Encode_Data_isr()     // Usage count decrements to 0.

The last two calls to BVBIlib_Encode_Data_isr() apply to two
different encoder queues.  Thus, the order in which the two calls
occur cannot be predicted.  Nevertheless, when the BVBI_Field_Handle
passes through both queues and its data is encoded (twice), the
usage count will drop to zero and the BVBIlib software will recycle
the BVBI_Field_Handle.

Here is a final, maximally complicated usage case.  Please note
that a typical user will not have to bother with the following
method.  Suppose that the user develops his own software to
process VBI data.  Suppose also that this customer wishes to
use VEC hardware to encode the same VBI data in the usual way.
This syslib provides functions BVBIlib_List_IncrementUsage_isr()
and BVBIlib_List_DecrementUsage_isr() to support this usage.
These two functions provide a backdoor into the memory management
scheme of this syslib.  Therefore, they should not normally be used.
Here is the previous example of decoding and (dual) encoding,
with added VBI processing by the customer:

   BVBIlib_Decode_Data_isr()     // Returns a BVBI_Field_Handle, usage count 0.
   BVBIlib_List_IncrementUsage_isr()
                                 // Usage count increments to 1
   BVBIlib_Encode_Enqueue_isr()  // Usage count increments to 2.
   BVBIlib_Encode_Enqueue_isr()  // Usage count increments to 3.
   BVBIlib_Encode_Data_isr()     // Usage count decrements to 2.
   BVBIlib_List_DecrementUsage_isr()
                                 // Usage count decrements to 1
   BVBIlib_Encode_Data_isr()     // Usage count decrements to 0.

The last three function calls all decrement the usage count, and
may occur in any order.  As in the previous example, when the usage
count decrements to zero, one of the BVBIlib functions will recycle
the BVBI_Field_Handle.

Usage
The usage of BVBIlib involves the following.  This programming takes place in
the main thread of execution, not in interrupt context.
   * Open and Configure BVBIlib
      * Open the BVBI module
      * Create BVBI encoder handles and BVBI decoder handles
      * Configure the BVBI encoder handles and BVBI decoder handles
      * Open the BVBIlib module
      * Create one or more BVBIlib_List free lists for use by encoders and
        decoders.
      * Create BVBIlib encoders and decoders, selecting types of VBI data to
        be encoded and/or decoded (closed caption, teletext, etc.).
      * Install the two BVBIlib ISR functions, usually using the BINT module.
      * Apply changes to the BVBI encoder handles and BVBI decoder handles.
        This causes VBI data extraction and/or insertion to begin.
   * Repeatedly decode and process a field of VBI data (in decode ISR)
      * Obtain a field of decoded VBI data, using a BVBIlib function.
      * Reformat (extract) VBI data, using BVBI functions.
      * Process VBI data, using customer software.
      * Reformat (insert) VBI data, using BVBI functions.
      * Enqueue a field of VBI data for encoding, using a BVBIlib function.
   * Repeatedly encode a field of VBI data (in encode ISR)
      * Dequeue and encode a field of VBI data, using a BVBIlib function.
      * - or - instruct the encoder hardware to send no data for one field.
   * Close and deallocate
      * Turn off VBI encoding and decoding by calling BVBI functions.
      * Return any outstanding BVBI_Field_Handles to BVBIlib_List.
      * Destroy BVBIlib encoders and decoders.
      * Destroy BVBIlib_List free list(s).
      * Close the BVBIlib module.
      * Destroy BVBI encoders and decoders.
      * Close the BVBI module.

Some observations
o The encoder queues will have a fixed length chosen by the user.  If the queue
  fills up, BVBIlib will discard the oldest VBI field data.
o The design allows the user to instantly pass a field of captured (decoded)
  VBI data to one or more VBI encoders.  The user can also "reserve" fields of
  data for his use, so that BVBIlib will NOT recycle these fields after they
  are encoded.  However, these fields do not live in any queue or list that is
  accessable to the user.  The user must construct his own queue or list if he
  wishes to keep several fields of VBI data hanging around his application
  software.
o When BVBIlib_List is created, a number of empty BVBI_Field_Handles are
  created.  The user chooses how many.  The user also has to choose whether or
  not these handles will be capable of holding teletext data.  The user also
  has to choose whether or not these handles will be capable of holding VPS
  data.  For each of these two decisions, it's "all or nothing."  All handles
  will be sized to hold teletext, or none of them.  Similarly for VPS.
o The implementation will depend on the fact that the interrupts that mark the
  beginning of active video (encoded and decoded) for each VEC and VDEC will
  not override each other.  In other words, each associated ISR will run to
  completion before any other BVBIlib-hosting ISR starts to run.

Sample Code

///////////////////////////////////////////////////////////////
// Begin initialization code: runs in non-interrupt context. //
///////////////////////////////////////////////////////////////

BMMA_Heap_Handle memHandle;
BVBI_Handle vbiHandle;
BVBI_Decode_Handle decHdl;
BVBI_Encode_Handle encHdl;
BVBIlib_Handle vbilHandle;
BVBIlib_list_Handle listHandle;
BVBIlib_Decode_Handle declHdl;
BVBIlib_Encode_Handle EnclHdl;

// Initialize BMMA module.
// This results in a valid memHandle.
// See programmers guide to BMMA base module for info.

// Initialize BVBI porting interface module.
// This results in valid vbiHandle, decHdl, and encHdl.
// See programmers guide to BVBI porting interface for example code.

// Open the BVBIlib module
BVBIlib_Open (&vbilHandle, vbiHandle);

// Create a BVBIlib free list to manage 16 BVBI field handles
BVBIlib_List_Create (&listHandle, vbiHandle, 16, NULL);

// Create a BVBIlib decoder and encoder
BVBIlib_Decode_Create (vbilHandle, listHandle, decHdl,    &declHdl);
BVBIlib_Encode_Create (vbilHandle, listHandle, encHdl, 4, &enclHdl);

/////////////////////////////
// End initialization code //
/////////////////////////////

////////////////////////////
// Begin Encoder ISR code //
////////////////////////////

// This bit of code should be installed into the ISR that is associated with
// the interrupt that marks the beginning of active video for the VEC that is
// associated with handle encHdl above.

BAVC_Polarity polarity;
BVBIlib_Encode_Handle enclHdl;

// In the following, enclHdl is the same variable that was initialized above.
// The BINT module provides a method for passing a variable to an ISR.
// Setting the polarity argument is tricky.  See the programmers guide to the
// BVBI porting interface for details.
BVBIlib_Encode_Data_isr (enclHdl, polarity);

//////////////////////////
// End Encoder ISR code //
//////////////////////////

////////////////////////////
// Begin Decoder ISR code //
////////////////////////////

// This code should be installed into the ISR that is associated with the
// interrupt that marks the beginning of active video for the VDEC that is
// associated with handle decHdl above.

uint16_t ccData;
BAVC_Polarity polarity;
BVBI_Field_Handle fieldHandle;
BVBIlib_Decode_Handle declHdl;
BVBIlib_Encode_Handle enclHdl;

// In the following, declHdl is the same variable that was initialized above.
// The BINT module provides a method for passing a variable to an ISR.
// Setting the polarity argument is tricky.  See the programmers guide to the
// BVBI porting interface for details.
//
// This function call obtains decoded VBI data for the previous field.
BVBIlib_Decode_Data_isr (declHdl, &fieldHandle, polarity);

// Modify the decoded VBI data (a silly example)
BVBI_Field_GetCCData_isr (fieldHandke, &ccData);
ccData = ccData + 1;
ccData = BVBI_SecCCParityBits (ccData);
BVBI_Field_SetCCData_isr (fieldHandke, ccData);

// In the following, enclHdl is the same variable that was initialized above.
// This function call sends the modified VBI data to the VEC for encoding.
BVBIlib_Encode_Enqueue_isr (enclHdl, fieldHandle);

//////////////////////////
// End Decoder ISR code //
//////////////////////////

</verbatim>
***************************************************************************/

#ifndef BVBILIB_H__
#define BVBILIB_H__

#include "berr.h"
#include "bvbi.h"
/* Only for backwards compatibility */
#include "budp.h"

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
    Module specific standard BERR codes
 *****************************************************************************/
/* This error indicates that a handle has been closed, but the user has failed
   to return all its resources first. */
#define BVBIlib_USER_LEAK            BERR_MAKE_CODE(BERR_VBIlib_ID, 0x0000)
#define BVBIlib_NO_HANDLES           BERR_MAKE_CODE(BERR_VBIlib_ID, 0x0001)
#define BVBIlib_QUEUE_FULL           BERR_MAKE_CODE(BERR_VBIlib_ID, 0x0002)

/* The following codes are used by the bitstream parsers */
#define BERR_BVBIlib_NO_DATA         BERR_BUDP_NO_DATA
#define BERR_BVBIlib_PARSE_ERROR     BERR_BUDP_PARSE_ERROR
#define BERR_BVBIlib_SOURCE_CHECK    BERR_BUDP_SOURCE_CHECK

/* Programming note:
 * ----------------
 *  The above error code BERR_BVBIlib_SOURCE_CHECK indicates that parsing is
 *  successful. However, because the data was encoded into an MPEG sequence
 *  header or GOP header, this data is not suitable for text output.
 */

/*****************************************************************************
 * Structures
 *****************************************************************************/

/*****************************************************************************
  Summary:
    Handle for BVBIlib module

  Description:
    The BVBIlib_Handle, once opened, represents the state of the BVBIlib
    module.

  See Also:
    BVBIlib_Open
 *****************************************************************************/
typedef struct BVBIlib_P_Handle *BVBIlib_Handle;  /* Opaque */

/*****************************************************************************
  Summary:
    Settings for BVBIlib_List module

  Description:
    The BVBIlib_List_Settings struct contains various settings for the
    BVBIlib_List free list.

  See Also:
    BVBIlib_List_Create, BVBIlib_List_GetDefaultSettings
 *****************************************************************************/
typedef struct BVBIlib_List_Settings
{
    bool bAllowTeletext; /*  Whether or not it will be possible to
                             encode/decode teletext.                         */
    bool bAllowVPS;      /*  Whether or not it will be possible to
                             encode/decode VPS.                              */
    bool bAllowGemstar;  /*  Whether or not it will be possible to
                             encode/decode Gemstar.                          */
    bool bAllowAmol;     /*  Whether or not it will be possible to
                             encode/decode AMOL.                             */
    bool bAllowMcc;      /*  Whether or not it will be possible to
                             encode/decode multi-line closed caption.        */
    bool bAllowCgmsB;    /*  Whether or not it will be possible to
                             encode/decode CGMS-B.                           */
    size_t scteCcSize;   /*  Maximum number of SCTE closed caption entries
                             to reserve space for in each field handle.
                             May be zero.                                    */
    size_t scteNrtvSize; /*  Maximum number of SCTE NRTV entries to reserve
                             space for in each field handle.  May be zero.   */
    size_t sctePamSize;  /*  Maximum number of SCTE luma_PAM_data bytes to
                             reserve space for in each field handle.  May
                             be zero.                                        */
    size_t scteMonoSize; /*  Maximum number of SCTE monochrome bytes to
                             reserve space for in each each of two line
                             stores, in each field handle.  May be zero.     */
}
BVBIlib_List_Settings;

/*****************************************************************************
  Summary:
    Handle for BVBIlib_List module

  Description:
    The BVBIlib_List_Handle, once opened, represents the state of a
    BVBIlib_List free list (of BVBI_Field_Handles).

  See Also:
    BVBIlib_LIst_Create
 *****************************************************************************/
typedef struct BVBIlib_P_List_Handle *BVBIlib_List_Handle;  /* Opaque */


/*****************************************************************************
  Summary:
    Handle for VBI encoding functionality

  Description:
    The BVBIlib_Encode_Handle is used to control how VBI encoding is done.
    It is used by a number of encoding-related functions.   It is allocated
    by BVBIlib_Encode_Create.

  See Also:
    BVBIlib_Encode_Create, BVBIlib_Encode_Destroy
 *****************************************************************************/
typedef struct BVBIlib_P_Encode_Handle *BVBIlib_Encode_Handle;  /* Opaque */


/*****************************************************************************
  Summary:
    Handle for VBI decoding functionality

  Description:
    The BVBIlib_Decode_Handle is used to control how VBI decoding is done.
    It is used by a number of decoding-related functions.  It is allocated
    by BVBIlib_Decode_Create.

  See Also:
    BVBIlib_Encode_Create, BVBIilb_Encode_Destroy
 *****************************************************************************/
typedef struct BVBIlib_P_Decode_Handle *BVBIlib_Decode_Handle;  /* Opaque */


/*****************************************************************************
 * Public API
 *****************************************************************************/


/*****************************************************************************
  Summary:
    Initialize the BVBIlib module

  Description:
    This module intializes the BVBIlib module.

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BERR_OUT_OF_SYSTEM_MEMORY - Memory allocation failed.

  See Also:
    BVBIlib_Close
 *****************************************************************************/
BERR_Code BVBIlib_Open(
    BVBIlib_Handle  *pVbilHandle,   /* [out] A pointer to an allocated
                                             BVBIlib_Handle                  */
    BVBI_Handle        vbiHandle    /*  [in] An initialized BVBI module
                                             handle.                         */
);


/*****************************************************************************
  Summary:
    Close a BVBIlib_Handle.

  Description:
    This function frees the BVBIlib_Handle structure and all associated data.
    It does NOT shutown any BVBI handles.

 *****************************************************************************/
void BVBIlib_Close(
    BVBIlib_Handle vbilHandle   /* [in] A valid BVBIlib_Handle object */
);


/*****************************************************************************
  Summary:
    Get the default settings for the BVBIlib_List free list

  Description:
    This module fills in a settings data structure for the BVBIlib_List module
    with default values.

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.

  See Also:
    BVBIlib_List_Create
 *****************************************************************************/
BERR_Code BVBIlib_List_GetDefaultSettings (
    BVBIlib_List_Settings*  pSettings   /*  [out] Pointer to an allocated
                                                  BVBIlib_List_Settings
                                                  object.                    */
);


/*****************************************************************************
  Summary:
    Create a BVBIlib_List free list

  Description:
    This module creates a BVBIlib_List free list (of BVBI_Field_Handles).

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BERR_OUT_OF_SYSTEM_MEMORY - Memory allocation failed.

  See Also:
    BVBIlib_List_Destroy
 *****************************************************************************/
BERR_Code BVBIlib_List_Create(
    BVBIlib_List_Handle *  listHandle,  /* [out] A pointer to an allocated
                                                 BVBIlib_List_Handle.        */
    BVBI_Handle             vbiHandle,  /*  [in] The main BVBI context
                                                 handle.                     */
    int                      nHandles,  /*  [in] How many BVBI_Field_Handles
                                                 to create in the list.      */
    const BVBIlib_List_Settings*
                            pSettings   /*  [in] Various settings for the
                                                 list. Use NULL to get the
                                                 default settings.           */
);


/*****************************************************************************
  Summary:
    Destroy a BVBIlib_List_Handle.

  Description:
    This function destroys a valid BVBIlib_List_Handle created by
    BVBIlib_List_Create.  Any BVBI_Field_Handles contained in the list are
    destroyed as well.

  Returns:
    BERR_SUCCESS              - The handle was successfully destroyed.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.
    BERR_VBIlib_USER_LEAK     - Some of the BVBI_Field_Handles were not
                                returned to the free list.

 *****************************************************************************/
BERR_Code BVBIlib_List_Destroy(
    BVBIlib_List_Handle listHandle /* [in] A BVBIlib_List_Handle to be
                                           destroyed                         */
);


#if (BVBI_NUM_IN656 > 0)
/*****************************************************************************
  Summary:
    Create a BVBIlib_Decode_Handle.

  Description:
    This function creates a BVBIlib_Decode_Handle for the specified
    BVBI_Decode_Handle.  The state of the BVBI_Decode_Handle determines
    what VBI data is decoded.

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BERR_OUT_OF_SYSTEM_MEMORY - Memory allocation failed.

 *****************************************************************************/
BERR_Code BVBIlib_Decode_Create(
    BVBIlib_Handle         vbilHandle,  /*  [in] A valid BVBIlib_Handle
                                                 object                      */
    BVBIlib_List_Handle   vbillHandle,  /*  [in] A valid BVBIlib_List_Handle
                                                 to use as a source of
                                                 BVBI_Field_Handles          */
    BVBI_Decode_Handle         decHdl,  /*  [in] A valid BVBI_Decode_Handle  */
    BVBIlib_Decode_Handle *  pDeclHdl   /* [out] A pointer to a
                                                 BVBIlib_Decode_Handle to
                                                 be initialized              */
);
#endif


/*****************************************************************************
  Summary:
    Create a BVBIlib_Encode_Handle.

  Description:
    This function creates a BVBIlib_Encode_Handle for the specified
    BVBI_Encode_Handle.  The state of the BVBI_Encode_Handle determines
    what VBI data is encoded.

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BERR_OUT_OF_SYSTEM_MEMORY - Memory allocation failed.

 *****************************************************************************/
BERR_Code BVBIlib_Encode_Create(
    BVBIlib_Handle         vbilHandle,  /*  [in] A valid BVBIlib_Handle
                                                 object                      */
    BVBIlib_List_Handle   vbillHandle,  /*  [in] A valid BVBIlib_List_Handle
                                                 to recycle
                                                 BVBI_Field_Handless to
                                                 after encoding              */
    BVBI_Encode_Handle         encHdl,  /*  [in] A valid BVBI_Encode_Handle  */
    int                     nMaxQueue,  /*  [in] The maximum number of
                                                 fields of VBI data that may
                                                 be enqueued for future
                                                 encoding.  If this limit is
                                                 exceeded, the oldest
                                                 enqueued VBI data will be
                                                 discarded.                  */
    BVBIlib_Encode_Handle *  pEnclHdl   /* [out] A pointer to a
                                                 BVBIlib_Encode_Handle to
                                                 be initialized              */
);



#if (BVBI_NUM_IN656 > 0)
/*****************************************************************************
  Summary:
    Destroy a BVBIlib_Decode_Handle.

  Description:
    This function destroys a valid BVBIlib_Decode_Handle created by
    BVBIlib_Decode_Create.  It does NOT destroy any BVBI_Decode_Handles.

  Returns:
    BERR_SUCCESS              - The handle was successfully destroyed.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.

 *****************************************************************************/
BERR_Code BVBIlib_Decode_Destroy(
    BVBIlib_Decode_Handle declHdl     /* [in] A BVBIlib_Decode_Handle to be
                                              destroyed                     */
);
#endif


/*****************************************************************************
  Summary:
    Destroy a BVBIlib_Encode_Handle.

  Description:
    This function destroys a valid BVBIlib_Encode_Handle created by
    BVBIlib_Encode_Create.  It does NOT destroy any BVBI_Encode_Handles.

  Returns:
    BERR_SUCCESS              - The handle was successfully destroyed.
    BERR_INVALID_PARAMETER    - The supplied parameter was invalid,
                                possibly NULL.

 *****************************************************************************/
BERR_Code BVBIlib_Encode_Destroy(
    BVBIlib_Encode_Handle enclHdl      /* [in] A BVBIlib_Encode_Handle to be
                                               destroyed                     */
);


#if (BVBI_NUM_IN656 > 0)
/*****************************************************************************
  Summary:
    Decodes a field of VBI data and provides it to the caller.

  Description:
    This function takes an active BVBIlib_Decode_Handle and decodes
    VBI data into it.  The data structure (handle) required to hold
    this data is allocated by this function, not by the user.

    This function is intended to be called from the interrupt
    that marks the beginning of a field of decoded active
    video.  For most applications, the ISR corresponding to
    this interrupt calls this function before any other VBI
    related processing.

    The programmer's guide includes a section _Working with
    BVBI_Field_Handles and Usage Counts_ that elaborates this
    function.  The user should understand this material in
    order to avoid memory management problems.

    Setting the polarity parameter is a bit tricky.  See the
    documentation for function BVBI_Decode_Data_isr in the
    BVBI module.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The encode handle is configured to encode a
                                bulky data format such as teletext, but the
                                field handle is not properly sized for this.

 *****************************************************************************/
BERR_Code BVBIlib_Decode_Data_isr(
    BVBIlib_Decode_Handle     declHdl, /*  [in] A valid
                                                BVBIlib_Decode_Handle object */
    BVBI_Field_Handle *  pFieldHandle, /* [out] A pointer to a
                                                BVBI_Field_Handle
                                                object.  Upon return, this
                                                will point to the decoded
                                                VBI data.                    */
    BAVC_Polarity            polarity  /*  [in] Which field (even, odd, or
                                                frame) to decode data from.  */
);
#endif


/*****************************************************************************
  Summary:
    Enqueues a field of VBI data for encoding

  Description:
    This function takes an active BVBIlib_Encode_Handle and valid
    BVBI_Field_Handle and enqueues the VBI data appropriately for
    future encoding.

    This function is intended to be called from the interrupt
    that marks the beginning of a field of DECODED active
    video.  However, it requires a handle for an ENCODER.

    After working its way through the queue, the BVBI_Field_Handle will
    eventually be encoded during a future call to BVBIlib_Encode_Data_isr().

    The programmer's guide includes a section _Working with
    BVBI_Field_Handles and Usage Counts_ that elaborates this
    function.  The user should understand this material in
    order to avoid memory management problems.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBIlib_QUEUE_FULL        - The queue is full.  The user's field handle
                                was not enqueued.

 *****************************************************************************/
BERR_Code BVBIlib_Encode_Enqueue_isr(
    BVBIlib_Encode_Handle      enclHdl, /* [in] A valid BVBIlib_Encode_Handle
                                                object                       */
    BVBI_Field_Handle      fieldHandle  /* [in] A valid BVBI_Field_Handle
                                                object.  The handle will be
                                                reused if its usage count is
                                                zero, after encoding is
                                                complete.                    */
);


/*****************************************************************************
  Summary:
    Dequeues oldest field of VBI data and encodes it.

  Description:
    This function takes an active BVBIlib_Encode_Handle and
    possibly encodes the oldest queued field of VBI data.
    If the parity setting of the VBI data agrees with the
    current field, then the encoding takes place.  Otherwise,
    the data remains queued until the next field.

    This function is intended to be called from the interrupt
    that marks the beginning of a field of encoded active
    video.  For most applications, the ISR corresponding to
    this interrupt will do no other VBI related work except to
    call this function.

    In most applications, either this function or BVBIlib_Encode_NoData_isr
    should be called once per output video field.

    Setting the polarity parameter is a bit tricky.  See the
    documentation for function BVBI_Encode_Data_isr in the
    BVBI module.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBI_ERR_FLDH_CONFLICT    - The encode handle is configured to encode a
                                bulky data format such as teletext, but the
                                field handle is not properly sized for this.

 *****************************************************************************/
BERR_Code BVBIlib_Encode_Data_isr(
    BVBIlib_Encode_Handle      enclHdl, /* [in] A valid BVBIlib_Encode_Handle
                                                object                       */
    BAVC_Polarity             polarity  /* [in] Which field (even, odd, or
                                                frame) to encode data into   */
);


/*****************************************************************************
  Summary:
    Returns pointer to oldest piece of data in encoder queue.

  Description:
    This function takes an active BVBIlib_Encode_Handle and
    returns a pointer to the oldest piece of VBI data in the encode
    queue.  Neither the state of the queue, nor the state of the VBI
    data is changed.

    This function is provided for the convenience of applications that
    must occasionally "mix in" a piece of VBI data at random times. In
    this situation, the application can add this data to the head of
    the encode queue, with the help of this function. If the queue is
    empty, the application can create a new BVBI field handle put his
    data into it, and finally put the field handle into the queue in
    the normal fashion.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.

 *****************************************************************************/
BERR_Code BVBIlib_Encode_GetOldestDatum_isr(
    BVBIlib_Encode_Handle   enclHdl,  /*  [in] A valid BVBIlib_Encode_Handle
                                               object                        */
    BVBI_Field_Handle* pFieldHandle   /* [out] The oldest piece of data in
                                               the encode queue. NULL if the
                                               queue is empty.               */
);


/*****************************************************************************
  Summary:
    Causes the VBI encoder hardware to output no VBI data for current field.

  Description:
    This function takes an active BVBIlib_Encode_Handle and
    causes the hardware to output NO VBI data
    for the next video field.  This function is called
    in place of BVBIlib_Encode_Data_isr.

    This function is intended to be called from the interrupt
    that marks the beginning of a field of encoded active
    video.

    In most applications, either this function or BVBIlib_Encode_Data_isr
    should be called once per output video field.

    Setting the polarity parameter is a bit tricky.  It is the
    polarity of the field to be affected, which is probably
    the opposite of the polarity in force then this function
    is called.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.

 *****************************************************************************/
BERR_Code BVBIlib_Encode_NoData_isr(
    BVBIlib_Encode_Handle      enclHdl, /* [in] A valid BVBIlib_Encode_Handle
                                                object.                      */
    BAVC_Polarity             polarity  /* [in] Which field (even, odd, or
                                                frame) to modify.            */
);


/*****************************************************************************
  Summary:
    Remove all pending VBI data from the queue to the encoder hardware.

  Description:
    This function takes an active BVBIlib_Encode_Handle and
    recycles any and all pending VBI data enqueued for encoding.

    This function may be called from an ISR.  It can be used to stop encoding
    without actually shutting down the VBI syslib, or the chip VBI cores.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.

 *****************************************************************************/
BERR_Code BVBIlib_Encode_Flush_isr(
    BVBIlib_Encode_Handle      enclHdl  /* [in] A valid BVBIlib_Encode_Handle
                                                object                       */
);


/*****************************************************************************
  Summary:
    Obtain number of field handles in encoding queue.  For diagnostic use.

  Description:
    This function takes an active BVBIlib_Encode_Handle and
    discloses how many field handles are contained within, waiting to be
    encoded.  This function is intended for use in bring-up and
    trouble-shooting.

    This function may be called from an ISR.

  Returns:
    Number of items in queue.

 *****************************************************************************/
int BVBIlib_GetQueueLength(
    BVBIlib_Encode_Handle      enclHdl  /* [in] A valid BVBIlib_Encode_Handle
                                                object                       */
);


/*****************************************************************************
  Summary:
    Obtains a BVBI_Field_Handle for the user.

  Description:
    This function takes an active BVBIlib_Handle and selects an unused
    BVBI_Field_Handle for the caller to use.  The caller should
    eventually return the BVBI_Field_Handle to the BVBIlib module by
    calling BVBIlib_List_Return_isr.

    This function is intended to be called from the interrupt
    that marks the beginning of a field of decoded active
    video.

    The programmer's guide includes a section _Working with
    BVBI_Field_Handles and Usage Counts_ that elaborates this
    function.  The user should understand this material in
    order to avoid memory management problems.


  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BVBIlib_NO_HANDLES        - There are no unused BVBI_Field_Handles
                                available.

  See Also:
    BVBIlib_List_Return_isr

 *****************************************************************************/
BERR_Code BVBIlib_List_Obtain_isr(
    BVBIlib_List_Handle   vbillHandle,  /*  [in] A valid BVBIlib_List_Handle
                                                 object                      */
    BVBI_Field_Handle *  pFieldHandle   /* [out] A BVBI_Field_Handle object.
                                                 The user may use this as he
                                                 wishes.                     */
);


/*****************************************************************************
  Summary:
    Returns a BVBI_Field_Handle after use.

  Description:
    This function takes an active BVBIlib_Handle and a BVBI_Field_Handle
    and tells the BVBIlib module that the BVBI_Field_Handle is no longer
    in use.

    This function is intended to be called from the interrupt
    that marks the beginning of a field of decoded active
    video.

    The programmer's guide includes a section _Working with
    BVBI_Field_Handles and Usage Counts_ that elaborates this
    function.  The user should understand this material in
    order to avoid memory management problems.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.

  See Also:
    BVBIlib_List_Obtain_isr

 *****************************************************************************/
BERR_Code BVBIlib_List_Return_isr(
    BVBIlib_List_Handle   vbillHandle,  /*  [in] A valid BVBIlib_List_Handle
                                                 object                      */
    BVBI_Field_Handle     fieldHandle   /*  [in] A BVBI_Field_Handle object.
                                                 The user indicates that it
                                                 is no longer in use.        */
);


/*****************************************************************************
  Summary:
    Reserves a BVBI_Field_Handle for use in a customer supplied
    VBI processor.

  Description:
    This function takes an active a BVBI_Field_Handle and increments
    its internal usage count.  This prevents the BVBIlib system
    library from recycling the field handle after VBI encoding
    on the field handle is complete.

    This function is not needed under "normal" circumstances.
    In this context, "normal" refers to a scenario where VBI data
    is simply sent to one or more VECs for hardware encoding,
    via calls to BVBIlib_Encode_Enqueue_isr().  This function
    should only be used when a field handle is to be accessed by
    a customer's application code AFTER it is sent to hardware
    for encoding via calls to BVBIlib_Encode_Data_isr().

    A specific example of when to use this function is when the
    customer develops a software VBI encoder, but the customer
    also wishes to use one ore more VECs to encode VBI data
    in hardware.

    Eventually, the user must call BVBIlib_List_DecrementUsage()
    or BVBIlib_List_DecrementUsage_isr() in order to prevent
    a memory leak.

    The programmer's guide includes a section _Working with
    BVBI_Field_Handles and Usage Counts_ that elaborates this
    function.  The user should understand this material in
    order to avoid memory management problems.

  See Also:
    BVBIlib_List_DecrementUsage()
    BVBIlib_List_DecrementUsage_isr()

 *****************************************************************************/
void BVBIlib_List_IncrementUsage_isr(
    BVBI_Field_Handle     fieldHandle   /*  [in] A BVBI_Field_Handle object.
                                                 The user indicates that it
                                                 is is reserved for use by
                                                 the caller.                 */
);


/*****************************************************************************
  Summary:
    Releases a BVBI_Field_Handle after use in a customer supplied
    VBI processor.

  Description:
    This function takes an active a BVBI_Field_Handle and decrements
    its internal usage count.  If the usage count is zero, then
    the field handle is "recycled" for reuse by the BVBIlib syslib.

    This function is not needed under "normal" circumstances.
    In this context, "normal" refers to a scenario where VBI data
    is simply sent to one or more VECs for hardware encoding,
    via calls to BVBIlib_Encode_Enqueue_isr().  This function
    should only be used when a field handle is to be accessed by
    a customer's application code AFTER it is sent to hardware
    for encoding via calls to BVBIlib_Encode_Data_isr().

    A specific example of when to use this function is when the
    customer develops a software VBI encoder, but the customer
    also wishes to use one ore more VECs to encode VBI data
    in hardware.

    Calls to this function must be exactly balanced by calls
    to BVBIlib_List_IncrementUsage_isr() in order to avoid
    memory leaks.

    The programmer's guide includes a section _Working with
    BVBI_Field_Handles and Usage Counts_ that elaborates this
    function.  The user should understand this material in
    order to avoid memory management problems.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.

  See Also:
    BVBIlib_List_IncrementUsage_isr()
    BVBIlib_List_DecrementUsage()

 *****************************************************************************/
BERR_Code BVBIlib_List_DecrementUsage_isr(
    BVBIlib_List_Handle   vbillHandle,  /*  [in] A valid BVBIlib_List_Handle
                                                 object                      */
    BVBI_Field_Handle     fieldHandle   /*  [in] A BVBI_Field_Handle object.
                                                 The user indicates that he
                                                 will not access this field
                                                 handle again.               */
);


/*****************************************************************************
  Summary:
    Releases a BVBI_Field_Handle after use in a customer supplied
    VBI processor.

  Description:
    This function does the same task as
    BVBIlib_List_DecrementUsage_isr(), but
    it includes a critical section.  Therefore, it can (and must)
    be called outside of interrupt context.

  Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.

  See Also:
    BVBIlib_List_IncrementUsage_isr()
    BVBIlib_List_DecrementUsage_isr()

 *****************************************************************************/
BERR_Code BVBIlib_List_DecrementUsage(
    BVBIlib_List_Handle   vbillHandle,  /*  [in] A valid BVBIlib_List_Handle
                                                 object                      */
    BVBI_Field_Handle     fieldHandle   /*  [in] A BVBI_Field_Handle object.
                                                 The user indicates that he
                                                 will not access this field
                                                 handle again.               */
);

#ifdef __cplusplus
}
#endif

#endif /* BVBILIB_H__ */
