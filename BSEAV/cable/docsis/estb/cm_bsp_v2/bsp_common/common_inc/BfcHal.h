//****************************************************************************
//
// Copyright (c) 2007 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//  $Id$
//
//  Filename:       BfcHal.h
//  Author:         Kevin O'Neal
//  Creation Date:  October 14, 2003
//
//****************************************************************************


#ifndef   BFCHAL_H
#define   BFCHAL_H

#if !defined(MAC_STUB) && !defined(WIN32) && !defined(__linux__)
   #include <bcmos.h>
#endif
#include <bcmtypes.h>
#include <BfcHalMacros.h>

#if defined __cplusplus
extern "C" {
#endif

   /********************************************************************************
   *    BFC_STATUS and its values. BFC_STATUS is used by all the private CM HAL and
   *    CM MAC functions to return status.
   *
   ********************************************************************************/

   typedef ULONG                             BFC_STATUS, *PBFC_STATUS;

   /*
    * BFC_STATUS values
    *
    *   Status values are 32 bit values laid out as follows:
    *
    *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
    *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *  +---+-+-------------------------+-------------------------------+
    *  |Sev|C|       Facility          |               Code            |
    *  +---+-+-------------------------+-------------------------------+
    *
    *  where
    *
    *    Sev - is the severity code
    *      00 - Success
    *      01 - Informational
    *      10 - Warning
    *      11 - Error
    *
    *    C - is the Customer code flag
    *
    *    Facility - is the facility code
    *
    *    Code - is the facility's status code
    */

#define STATUS_UNSUCCESSFUL                  ((BFC_STATUS)0xC0000001L)
#define STATUS_SUCCESS                       ((BFC_STATUS)0x00000000L)
#ifndef STATUS_PENDING

   // This is in WINNT.h; don't re-define if compiling on NT
#define STATUS_PENDING                       ((BFC_STATUS)0x00000103L)
#endif
#define STATUS_BUFFER_OVERFLOW               ((BFC_STATUS)0xC0000104L)
#define STATUS_INSUFFICIENT_RESOURCES        ((BFC_STATUS)0x80000105L)
#define STATUS_NOT_SUPPORTED                 ((BFC_STATUS)0x40000106L)

#define BFC_STATUS_SUCCESS                    ((BFC_STATUS)STATUS_SUCCESS)
#define BFC_STATUS_PENDING                    ((BFC_STATUS)STATUS_PENDING)
#define BFC_STATUS_NOT_RECOGNIZED             ((BFC_STATUS)0x00010001L)
#define BFC_STATUS_NOT_COPIED                 ((BFC_STATUS)0x00010002L)
#define BFC_STATUS_NOT_ACCEPTED               ((BFC_STATUS)0x00010003L)
#define BFC_STATUS_CALL_ACTIVE                ((BFC_STATUS)0x00010007L)

#define BFC_STATUS_ONLINE                     ((BFC_STATUS)0x40010003L)
#define BFC_STATUS_RESET_START                ((BFC_STATUS)0x40010004L)
#define BFC_STATUS_RESET_END                  ((BFC_STATUS)0x40010005L)

#define BFC_STATUS_MEDIA_CONNECT              ((BFC_STATUS)0x4001000BL)
#define BFC_STATUS_MEDIA_DISCONNECT           ((BFC_STATUS)0x4001000CL)
#define BFC_STATUS_HARDWARE_LINE_UP           ((BFC_STATUS)0x4001000DL)
#define BFC_STATUS_HARDWARE_LINE_DOWN         ((BFC_STATUS)0x4001000EL)
#define BFC_STATUS_INTERFACE_UP               ((BFC_STATUS)0x4001000FL)
#define BFC_STATUS_INTERFACE_DOWN             ((BFC_STATUS)0x40010010L)
#define BFC_STATUS_MEDIA_BUSY                 ((BFC_STATUS)0x40010011L)

#define BFC_STATUS_NOT_RESETTABLE             ((BFC_STATUS)0x80010001L)
#define BFC_STATUS_SOFT_ERRORS                ((BFC_STATUS)0x80010003L)
#define BFC_STATUS_HARD_ERRORS                ((BFC_STATUS)0x80010004L)
#define BFC_STATUS_BUFFER_OVERFLOW            ((BFC_STATUS)STATUS_BUFFER_OVERFLOW)

#define BFC_STATUS_FAILURE                    ((BFC_STATUS)STATUS_UNSUCCESSFUL)
#define BFC_STATUS_RESOURCES                  ((BFC_STATUS)STATUS_INSUFFICIENT_RESOURCES)
#define BFC_STATUS_NOT_SUPPORTED              ((BFC_STATUS)STATUS_NOT_SUPPORTED)
#define BFC_STATUS_CLOSING                    ((BFC_STATUS)0xC0010002L)
#define BFC_STATUS_BAD_VERSION                ((BFC_STATUS)0xC0010004L)
#define BFC_STATUS_BAD_CHARACTERISTICS        ((BFC_STATUS)0xC0010005L)
#define BFC_STATUS_ADAPTER_NOT_FOUND          ((BFC_STATUS)0xC0010006L)
#define BFC_STATUS_OPEN_FAILED                ((BFC_STATUS)0xC0010007L)
#define BFC_STATUS_DEVICE_FAILED              ((BFC_STATUS)0xC0010008L)
#define BFC_STATUS_MULTICAST_FULL             ((BFC_STATUS)0xC0010009L)
#define BFC_STATUS_MULTICAST_EXISTS           ((BFC_STATUS)0xC001000AL)
#define BFC_STATUS_MULTICAST_NOT_FOUND        ((BFC_STATUS)0xC001000BL)
#define BFC_STATUS_REQUEST_ABORTED            ((BFC_STATUS)0xC001000CL)
#define BFC_STATUS_RESET_IN_PROGRESS          ((BFC_STATUS)0xC001000DL)
#define BFC_STATUS_CLOSING_INDICATING         ((BFC_STATUS)0xC001000EL)
#define BFC_STATUS_INVALID_PACKET             ((BFC_STATUS)0xC001000FL)
#define BFC_STATUS_OPEN_LIST_FULL             ((BFC_STATUS)0xC0010010L)
#define BFC_STATUS_ADAPTER_NOT_READY          ((BFC_STATUS)0xC0010011L)
#define BFC_STATUS_ADAPTER_NOT_OPEN           ((BFC_STATUS)0xC0010012L)
#define BFC_STATUS_NOT_INDICATING             ((BFC_STATUS)0xC0010013L)
#define BFC_STATUS_INVALID_LENGTH             ((BFC_STATUS)0xC0010014L)
#define BFC_STATUS_INVALID_DATA               ((BFC_STATUS)0xC0010015L)
#define BFC_STATUS_BUFFER_TOO_SHORT           ((BFC_STATUS)0xC0010016L)
#define BFC_STATUS_INVALID_OID                ((BFC_STATUS)0xC0010017L)
#define BFC_STATUS_ADAPTER_REMOVED            ((BFC_STATUS)0xC0010018L)
#define BFC_STATUS_UNSUPPORTED_MEDIA          ((BFC_STATUS)0xC0010019L)
#define BFC_STATUS_GROUP_ADDRESS_IN_USE       ((BFC_STATUS)0xC001001AL)
#define BFC_STATUS_FILE_NOT_FOUND             ((BFC_STATUS)0xC001001BL)
#define BFC_STATUS_ERROR_READING_FILE         ((BFC_STATUS)0xC001001CL)
#define BFC_STATUS_ALREADY_MAPPED             ((BFC_STATUS)0xC001001DL)
#define BFC_STATUS_RESOURCE_CONFLICT          ((BFC_STATUS)0xC001001EL)
#define BFC_STATUS_NO_CABLE                   ((BFC_STATUS)0xC001001FL)

#define BFC_STATUS_INVALID_SAP                ((BFC_STATUS)0xC0010020L)
#define BFC_STATUS_SAP_IN_USE                 ((BFC_STATUS)0xC0010021L)
#define BFC_STATUS_INVALID_ADDRESS            ((BFC_STATUS)0xC0010022L)
#define BFC_STATUS_VC_NOT_RESERVED            ((BFC_STATUS)0xC0010023L)
#define BFC_STATUS_DEST_OUT_OF_ORDER          ((BFC_STATUS)0xC0010024L)
#define BFC_STATUS_VC_NOT_AVAILABLE           ((BFC_STATUS)0xC0010025L)
#define BFC_STATUS_CELLRATE_NOT_AVAILABLE     ((BFC_STATUS)0xC0010026L)
#define BFC_STATUS_INCOMPATABLE_QOS           ((BFC_STATUS)0xC0010027L)

#define BFC_STATUS_MESSGQ_CREATE_FAILED       ((BFC_STATUS)0xC0010028L)
#define BFC_STATUS_MESSGQ_WRITE_FAILED        ((BFC_STATUS)0xC0010029L)
#define BFC_STATUS_MESSGQ_READ_FAILED         ((BFC_STATUS)0xC0010030L)

#define BFC_STATUS_CREATE_THREAD_FAILED       ((BFC_STATUS)0xC0010031L)

#define BFC_STATUS_INVALID_ATTR_LEN           ((BFC_STATUS)0xC0010032L)
#define BFC_STATUS_REQD_ATTR_MISSING          ((BFC_STATUS)0xC0010033L)


   /********************************************************************************
   *    BFC_HAL_OID and its values
   *
   ********************************************************************************/

   /*
    * BFC_HAL_OID values
    *   OID values are 32 bit values laid out as follows:
    *
    *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
    *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *  +-+-----------------------------+-------------------------------+
    *  |M|          Unused             |               OID             |
    *  +-+-----------------------------+-------------------------------+
    *
    *  where
    *
    *      M - the multiple instances flag
    *        0 - Only one instance of the HAL OID can exist
    *        1 - Multiple instances of the HAL OID can exist
    *
    *      OID - the 16-bit OID value
    *
    */


   typedef        UINT                                              BFC_HAL_OID;

   
   /* Used in the TCPIP Hal for XID/TEST support.*/
#define		   BFC_HAL_OID_GET_ETHPACKET                           ((BFC_HAL_OID)0x0000005F)


   // Wrapper for calls to BcmAlloc (QueryInfo).  For now, only supported by the
   // CM HAL.
   //
   // ulParam - the buffer size being requested
   // pvInfoBuffer - pointer to the buffer pointer; this is where the buffer address is returned.
   // ulInfoBufferLen - must be 4 (sizeof(void *)).
#define        BFC_HAL_OID_ALLOC_BUFFER                         ((BFC_HAL_OID)0x00000100)

   // Wrapper for calls to BcmFree (SetInfo).  For now, only supported by the CM
   // HAL.
   //
   // ulParam - not used.
   // pvInfoBuffer - pointer to the buffer to be freed.
   // ulInfoBufferLen - must be 4 (sizeof(void *)).
#define        BFC_HAL_OID_FREE_BUFFER                          ((BFC_HAL_OID)0x00000101)

   

   /********************************************************************************
   *    BFC_HAL_MODE and its values
   *
   ********************************************************************************/

   /*
    * BFC_HAL_MODE values
    *   MODE values are 32 bit values laid out as follows:
    *
    *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
    *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *  +-+-----------------------------+-------------------------------+
    *  |M|          Unused             |              Mode             |
    *  +-+-----------------------------+-------------------------------+
    *
    *  where
    *
    *      M - the multiple instances flag
    *        0 - Only one instance of the HAL MODE can exist
    *        1 - Multiple instances of the HAL MODE can exist
    *
    *      MODE - the 16-bit MODE value
    *
    */


   typedef        UINT                                                        BFC_HAL_MODE;
   
   /* Miscellaneous: */

#define	      BFC_HAL_MODE_ENABLE_INTERFACE                  	            ((BFC_HAL_MODE)0x0000003F)
#define	      BFC_HAL_MODE_REINITIALIZE_INTERFACE            	            ((BFC_HAL_MODE)0x00000040)


#define       BFC_HAL_MODE_POWERDOWN            	                        ((BFC_HAL_MODE)0x00000043)
typedef enum // ulParam values for this mode.
{
  kPowerdownFullPowerNoReduction = 0,
  kPowerdownLowPowerModerateReduction,
  kPowerdownLowPowerMaximumReduction
} BFC_HAL_MODE_POWERDOWN_PARAM;

   /********************************************************************************
   *    BFC_MAC_OID and its values
   *
   ********************************************************************************/

   /*
    * BFC_MAC_OID values
    *   OID values are 32 bit values laid out as follows:
    *
    *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
    *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *  +-+-----------------------------+-------------------------------+
    *  |M|          Unused             |               OID             |
    *  +-+-----------------------------+-------------------------------+
    *
    *  where
    *
    *      M - the multiple instances flag
    *        0 - Only one instance of the MAC OID can exist
    *        1 - Multiple instances of the MAC OID can exist
    *
    *      OID - the 16-bit OID value
    *
    */

   typedef        UINT                                              BFC_MAC_OID;



   /********************************************************************************
   *    BOARD_HAL_MODE and its values
   *
   ********************************************************************************/

   /*
    * BOARD_HAL_MODE values
    *   MODE values are 32 bit values laid out as follows:
    *
    *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
    *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *  +-+-----------------------------+-------------------------------+
    *  |M|          Unused             |              MODE             |
    *  +-+-----------------------------+-------------------------------+
    *
    *  where
    *
    *      M - the multiple instances flag
    *        0 - Only one instance of the BOARD HAL MODE can exist
    *        1 - Multiple instances of the BOARD HAL MODE can exist
    *
    *      MODE - the 16-bit MODE value
    *
    */

   typedef        UINT                                              BOARD_HAL_MODE;

#define        BOARD_HAL_MODE_ENABLE_WATCHDOG                    ((BOARD_HAL_MODE)0x00000000)
#define        BOARD_HAL_MODE_PAUSE_WATCHDOG                     ((BOARD_HAL_MODE)0x00000001)
#define        BOARD_HAL_MODE_ENABLE_SERIAL_PORT                 ((BOARD_HAL_MODE)0x00000009)



   /********************************************************************************
   *    BOARD_HAL_OID and its values
   *
   ********************************************************************************/

   /*
    * BOARD_HAL_OID values
    *   OID values are 32 bit values laid out as follows:
    *
    *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
    *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *  +-+-----------------------------+-------------------------------+
    *  |M|          Unused             |               OID             |
    *  +-+-----------------------------+-------------------------------+
    *
    *  where
    *
    *      M - the multiple instances flag
    *        0 - Only one instance of the MAC OID can exist
    *        1 - Multiple instances of the MAC OID can exist
    *
    *      OID - the 16-bit OID value
    *
    */

   typedef        UINT                                              BOARD_HAL_OID;

   // Set only
   //
   // ulParam = "don't care"
   // pvInfoBuffer = buffer (starting address)
   // ulInfoBufferLen = length
   //
#define		   BOARD_HAL_OID_INVALIDATE_DCACHE_BY_ADDRESS  		((BOARD_HAL_OID)0x00000000)
#define		   BOARD_HAL_OID_FLUSH_DCACHE_BY_ADDRESS       		((BOARD_HAL_OID)0x00000001)

   // (note: valid dirty cache lines are committed to memory if in write-back mode)
#define		   BOARD_HAL_OID_CLEAR_DCACHE_BY_INDEX  			((BOARD_HAL_OID)0x00000002)

   // ulParam = "don't care"
   // pvInfoBuffer = PBoardHalPeriodicTimerConfig
   // ulInfoBufferLen = "don't care"
#define		   BOARD_HAL_OID_INSTALL_PERIODIC_TIMER				((BOARD_HAL_OID)0x00000003)
#define		   BOARD_HAL_OID_UNINSTALL_PERIODIC_TIMER	  		((BOARD_HAL_OID)0x00000004)
#define		   BOARD_HAL_OID_START_PERIODIC_TIMER	  			((BOARD_HAL_OID)0x00000005)
#define		   BOARD_HAL_OID_SUSPEND_PERIODIC_TIMER		 	   	((BOARD_HAL_OID)0x00000006)

#define		   BOARD_HAL_OID_CHIP_ID		 	   	       ((BOARD_HAL_OID)0x00000007)

#if (BCM93380_DS_BONDING)
#define       CM_HAL_OID_BONDED_WRITE                          ((BOARD_HAL_OID)0x00000008)
#define       CM_HAL_OID_BONDED_READ                           ((BOARD_HAL_OID)0x00000009)
#endif
   
#if (BCM93380_DS_BONDING)
   typedef struct _BONDED_STRUCT_
   {
      unsigned int ChipSelect;
      unsigned int Addr;
      unsigned int Data;
   } BONDED_STRUCT;
#endif
   
   typedef struct _BoardHalPeriodicTimerConfig
   {
      uint32       (*pfIrqHandler)(BcmHandle);  // pointer to client IRQ Handler routine (INSTALL)
      uint32      periodMs;               // period for interrupt in ms  (INSTALL,START)
      BcmHandle      hTimer;                 // handle for Timer to service (INSTALL,UNINSTALL,START,SUSPEND)
   }BoardHalPeriodicTimerConfig, *PBoardHalPeriodicTimerConfig;

   /********************************************************************************
   *    NI_HAL_OID and its values
   *
   ********************************************************************************/

   /*
    * NI_HAL_OID values
    *   OID values are 32 bit values laid out as follows:
    *
    *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
    *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *  +-+-----------------------------+-------------------------------+
    *  |M|          Unused             |               OID             |
    *  +-+-----------------------------+-------------------------------+
    *
    *  where
    *
    *      M - the multiple instances flag
    *        0 - Only one instance of the MAC OID can exist
    *        1 - Multiple instances of the MAC OID can exist
    *
    *      OID - the 16-bit OID value
    *
    */

   typedef        UINT                                              NI_HAL_OID;
   typedef        UINT                                              NI_HAL_MODE;



   /********************************************************************************
   *    NI_HAL_OID and its values
   *
   ********************************************************************************/

   /*
    * NI_HAL_OID values
    *   OID values are 32 bit values laid out as follows:
    *
    *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
    *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *  +-+-----------------------------+-------------------------------+
    *  |M|          Unused             |               OID             |
    *  +-+-----------------------------+-------------------------------+
    *
    *  where
    *
    *      M - the multiple instances flag
    *        0 - Only one instance of the MAC OID can exist
    *        1 - Multiple instances of the MAC OID can exist
    *
    *      OID - the 16-bit OID value
    *
    */



   


   typedef BFC_STATUS (*PFN_BFC_HAL_INIT_HANDLER) ( void );
                              
   // TODO: get these out of here!  Something should be using void pointers
   // instead...
   
   // use with BFC_HAL_OID_DNSTREAM_CHANNEL_STATUS (Query Only)
   typedef struct _DownstreamChannelStatus_
   {
      BOOL bEnergyDetected; // PR 13920
      BOOL bSyncAcquired;
      BOOL bFECLock;
      BOOL bQAMLock;
      int  cmCarrierFrequencyOffset;
   } DownstreamChannelStatus, *PDownstreamChannelStatus;
   
   typedef struct _DNSTREAM_CHANNEL_PARAMS_
   {

      ULONG     ulFreq;                       /* Center Frequency in Hertz */
      ULONG     ulWidth;                      /* Channel width in Hertz */
      ULONG     ulPower;                      /* Channel power level in tenth dBmV units */
      USHORT    usModulationType;             /* From the modulation types defined above */
      UCHAR     ucInterleave;                 /* From the interleave values defined above */
      UCHAR     ucAnnex;                      /* If NULL, default = Annex-B */
      ULONG     ulSymbolRate;                 /* If NULL, use MCNS symbol rate */
      BOOL      bSingleScan;                  /* If TRUE, will scan single freq pair */
                                              /* (1 HRC, 1 STANDARD) default is FALSE */
      DownstreamChannelStatus downstreamChannelStatus;  /* current Downstream channel status */
   } DNSTREAM_CHANNEL_PARAMS, *PDNSTREAM_CHANNEL_PARAMS;

   typedef
   BFC_STATUS
   (*PFN_BFC_HAL_SCAN_DNSTREAM_CHANNEL_HANDLER) (
                                               PDNSTREAM_CHANNEL_PARAMS pDnstreamChannelParams );

   typedef
   BFC_STATUS
   (*PFN_BFC_HAL_SCAN_DNSTREAM_FREQUENCY_HANDLER) (
                                                 PDNSTREAM_CHANNEL_PARAMS pDnstreamChannelParams );


   typedef
   BFC_STATUS
   (*PFN_BFC_HAL_ACQUIRE_DNSTREAM_SYNC_HANDLER) (
                                               int32 syncMode,
                                               uint32 maxDurationSeconds );

   typedef
   BFC_STATUS
   (*PFN_BFC_HAL_SET_INFO_HANDLER) (
                                  BFC_HAL_OID cmHalOid,
                                  ULONG      ulParam,
                                  PVOID      pvInfoBuffer,
                                  ULONG      ulInfoBufferLen );

   typedef
   BFC_STATUS
   (*PFN_BFC_HAL_QUERY_INFO_HANDLER) (
                                    BFC_HAL_OID CmHalOid,
                                    ULONG      ulParam,
                                    PVOID      pvInfoBuffer,
                                    PULONG     pulInfoBufferLen );

   typedef
   BFC_STATUS
   (*PFN_BFC_HAL_SET_MODE_HANDLER) (
                                  BFC_HAL_MODE CmHalMode,
                                  ULONG       ulParam,
                                  BOOLEAN     bflag );

   typedef
   BFC_STATUS
   (*PFN_BFC_HAL_QUERY_MODE_HANDLER) (
                                    BFC_HAL_MODE CmHalMode,
                                    ULONG       ulParam,
                                    PBOOLEAN    pbflag );
                                    
   /*******************************************************************************
   *    Data buffer linked list structure used by the CM MAC to pass data (Ethernet)
   *    packets.
   */

   // This is for use by the CM HAL (and the HAL Interface module) to handle PHS.
   // In PHS, the packet header is suppressed, which would cause the packet data
   // to be modified.  Because the packet may also be used at the same time by
   // other HALs (for broadcast or multicast), and in some cases the client expects
   // that it will not be modified, we need to avoid modifying it.
   //
   // This structure supports associating a new "pre-buffer" with the main buffer,
   // which contains the modified header data, as well as offset and
   // length information.
   //
   // The pre-buffer is allocated by the HAL Interface module (via an Info call
   // to the CM HAL), and is freed by the CM HAL when transmit is complete.
   //
   //   <------ uiLen -------->
   //  +-----------------------+
   //  | 1 2 3 4 5 6 7 8 9 ... |
   //  +-----------------------+
   //    s   s   s   s                 s = Byte to be suppressed.
   //
   // Here is what the packet would look like if s/w suppressed in-place (note
   // the gap between the unsuppressed portion of the header and the remainder of
   // the packet):
   //
   //  +-----------------------+
   //  | 2 4 6 x x x x 8 9 ... |
   //  +-----------------------+
   //
   // Here is how the pre-buffer is used to avoid modifying the packet:
   //
   //  +-------+  +-----------------------+
   //  | 2 4 6 |  | 1 2 3 4 5 6 7 8 9 ... |
   //  +-------+  +-----------------------+
   //  ^          ^               ^
   //  |          |               |
   //  |          +-pvStart       mainBufferOffset = 7
   //  pPreBuffer                 remainder length = uiLen - 7
   //  numberOfBytes = 3
   //
   // ------------------------------
   // Here's another example, where all bytes of the header are being suppressed;
   // it shows that the pre-buffer can be empty/NULL, and only the main buffer
   // offset used:
   //
   //   <------ uiLen -------->
   //  +-----------------------+
   //  | 1 2 3 4 5 6 7 8 9 ... |
   //  +-----------------------+
   //    s s s s s s s                 s = Byte to be suppressed.
   //
   //             +-----------------------+
   //             | 1 2 3 4 5 6 7 8 9 ... |
   //  NULL       +-----------------------+
   //  ^          ^               ^
   //  |          |               |
   //  |          +-pvStart       mainBufferOffset = 7
   //  pPreBuffer                 remainder length = uiLen - 7
   //  numberOfBytes = 0
   //
   // ------------------------------
   // As a last example, it is possible for all bytes of the packet to be
   // suppressed, except for a few header bytes; in this case, pre-buffer may be
   // the only data to be transmitted:
   //
   //   <------ uiLen -------->
   //  +-----------------------+
   //  | 1 2 3 4 5 6 7 8 9 ... |
   //  +-----------------------+
   //    s   s   s   s s s sss         s = Byte to be suppressed.
   //
   //  +-------+  +-----------------------+
   //  | 2 4 6 |  | 1 2 3 4 5 6 7 8 9 ... |
   //  +-------+  +-----------------------+
   //  ^          ^                       ^
   //  |          |                       |
   //  |          +-pvStart               mainBufferOffset = uiLen
   //  pPreBuffer                         remainder length = 0
   //  numberOfBytes = 3
   //

   ////////////////////////////////////////////////////////////////////////////////
   // Specific DOCSIS CM PHS operation/flags for SW and HW PHS
   //

   typedef struct _BFC_HAL_PHS_INFO /* UNFINISHED - CABLEMODEM DEPENDENCY! */
   {
      // SW PHS support only:
      void *pPreBuffer;           // Buffer containing the unsuppressed portion of the header.
      uint16 numberOfBytes;       // The number of bytes of data in the pre-buffer.

      uint16 mainBufferOffset;    // The offset into the main buffer where the remainder of the packet starts.
      // The remainder length can be calculated (uiLen = mainBufferOffset).

      // HW PHS support only:
      byte hwPhsIndex;           // if > 0 specifies the phsIndex, if HW PHS is supported, otherwise = 0

      byte reserved[3];          // added reserved bytes to force alignment

   } BFC_HAL_PHS_INFO;

   ////////////////////////////////////////////////////////////////////////////////
   // Free Pool Manager Configuration settings.  May be passed to each interface
   // by application via optional params in BFC_MAC_CHARACTERISTICS  object 
   // via pvCableModemParms (i.e. CableModemParms, EnetNiInitStruct)
   // Supported Hardware platforms only.  Configuration per interface stored in nonVol  
   typedef enum 
   {
       kFpmConfigured      =    0x01,       // FPM is configured for the interface
       kFpmAutoFreeEnabled =    0x02,       // FPM auto free feature is enabled for the interface
       kFpmRxEnabled       =    0x04        // FPM Receive Operation is enable for the interface
   } FPM_CONFIGURATION;

   ////////////////////////////////////////////////////////////////////////////////
   // Specific DATA_BUFFER OBJECT ATTRIBUTES
   //
   typedef enum 
   {
        kFpmDataBuffer          = 0x0001,   // Buffer is managed by HW free Pool Manager (supported silicon platforms)

        kDocsisDSID             = 0x0002,   // Docsis Downstream Service ID attribute exists in Docsis Ehdr
        kDocsisDsTrafficPrio    = 0x0004,   // Docsis Downstream Traffic Priority exists in Docsis Ehdr
        kDocsisDsPktSequence    = 0x0008,   // Docsis Downstream Packet Sequence number exists in Docsis Ehdr (i.e. DS Bonding)

        kFpmDataBufferAutoFree  = 0x0010    // Buffer is auto freed on tx by HW free Pool Manager (supported silicon platforms)

   } DATA_BUFFER_ATTRIBUTES;

   typedef struct _DATA_BUFFER_
   {

      PVOID                   pvStart;         // Pointer to the packet's data buffer.
      UINT                    uiLen;           // The number of bytes of data in the buffer.

      BFC_HAL_PHS_INFO         cmHalPhsInfo;    // NOTE:  Only for use by CM HAL (for PHS support) /* UNFINISHED - CABLEMODEM DEPENDENCY! */

      UINT                    ipsecOffset;     // Used by IPSEC to hold the offset needed to free the data

      UINT                    WiFi80211PsInfo;    //Used by 802.11 driver when enqueueing power save data.

      UINT                    stEntry;          // session tracking entry 

      USHORT                  attributes;       // DATA_BUFFER defined attributes (as listed above)

      USHORT                  dCacheInvLength;  // Length of buffer area to perform dcache invalidate (see below)

      PVOID                   pDcacheInvBuffer; // pointer to buffer area to perform dcache invalidate. This is
                                                // bookkeeping area for the driver in question, and one use for
                                                // is when running FPM with auto free mode enabled - in which case
                                                // the transmitting driver MUST dcache invalidate portion of buffer
                                                // prior to queue to HW, since HW will free packet automatically upon
                                                // transmit complete.

      UINT                    qosPriority;      // Quality of Service Priority

      // For buffer chaining; this is no longer supported in most HALs, so it
      // should not be used.  We may remove this member in the future.
      struct _DATA_BUFFER_    *pNext;

   } DATA_BUFFER, *PDATA_BUFFER;
   

//-----------------------------------------------------------------------------
// DATA_BUFFER List Diagnostics:
//-----------------------------------------------------------------------------
typedef struct _DATA_BUFFER_LIST_DIAG
{
    DATA_BUFFER * pDataBufferListHead;    
    DATA_BUFFER * pDataBufferListTail;    
    uint32 dataBufferCount;
} DATA_BUFFER_LIST_DIAG;

   /*******************************************************************************
   *    DOCSIS Downstream Service Parameter structure.
   *    Contains related DS Service fields extracted from Docsis EHDR (if present)
   *    DATA_BUFFER attributes identifies which fields are present
   *  
   *    Memory for this object is located immediately after the DOCSIS EHDR for
   *    Docsis DS data packet.
   */
   typedef struct _DOCSIS_DOWNSTREAM_SERVICE_PARAMS /* UNFINISHED - CABLEMODEM DEPENDENCY! */
   {
      uint32    DSID;                   // Downstream Service ID
      uint16    pktSequence;            // Packet Sequence Number
      uint8     trafficPriority;        // Traffic Priority
      uint8     reserved;               // reserved
   } DOCSIS_DOWNSTREAM_SERVICE_PARAMS, *PDOCSIS_DOWNSTREAM_SERVICE_PARAMS;


#ifndef MAX_DOCSIS_EHDR_LENGTH      // may be defined already in BcmMac.h
    #define MAX_DOCSIS_EHDR_LENGTH   16
#endif

   /*******************************************************************************
   *    DOCSIS Extended Header Packet structure.
   *    (note: may contain multiple DOCSIS Ehdrs, uiLen represents total EHDR length)
   */
   typedef struct _DOCSIS_EXTENDED_HEADER_ /* UNFINISHED - CABLEMODEM DEPENDENCY! */
   {
      PBYTE    pbStart;
      UINT     uiLen;
   } DOCSIS_EXTENDED_HEADER, *PDOCSIS_EXTENDED_HEADER;

   /*******************************************************************************
   *    Ethernet packet structure.
   */
   typedef struct _ETHERNET_PACKET_
   {

      BcmHandle      hPacket;
      PDATA_BUFFER   pDataBufferHead;
      PDATA_BUFFER   pDataBufferTail;
      ULONG          ulCount;                // Packet usage counter
      USHORT         uiInterface;            // interface this packet came from
      USHORT         uiDestination;          // interface this packet is being sent to
      DOCSIS_EXTENDED_HEADER eHdr;           // DOCSIS Extended Header object (used for Downstream) /* UNFINISHED - CABLEMODEM DEPENDENCY! */
      struct _ETHERNET_PACKET_ *pNext;       // Reserved field for CM HAL

   }    ETHERNET_PACKET, *PETHERNET_PACKET;
   
                                    
   /*******************************************************************************
   *    DOCSIS management message structure.
   */
   typedef struct _MGMT_MSG_ /* UNFINISHED - CABLEMODEM DEPENDENCY! */
   {

      BcmHandle    hPacket;
      PVOID     pvPacketStart;
      UINT      uiPacketLen;
      UINT      uiInterface;                 // driver/port logical interface ID (assigned by MAC Layer)
      DOCSIS_EXTENDED_HEADER eHdr;           // DOCSIS Extended Header object (used for Downstream)
      int32     attributes;                  // DATA_BUFFER defined attributes (as listed above)  
      uint32    dsChan;                      // DS Bonding support - DS Channel index (0=primary)

   }    MGMT_MSG, *PMGMT_MSG;

   typedef
   VOID
   (*PFN_BFC_HAL_RELEASE_MGMT_MSG_HANDLER) (
                                          PMGMT_MSG pMgmtMsg ); /* UNFINISHED - CABLEMODEM DEPENDENCY! */
                                          
   typedef
   VOID (*PFN_BFC_HAL_RELEASE_DATA_PACKET_HANDLER) (
                                                  PETHERNET_PACKET pEthernetPacket );
                                                  
/* UNFINISHED - CABLEMODEM DEPENDENCY! */
                                                  
   /*******************************************************************************
   *    DOCSIS header passed down from CM MAC for each packet to be sent out.
   */
   typedef   struct _DOCSIS_HDR_ /* UNFINISHED - CABLEMODEM DEPENDENCY! */
   {

      BYTE      bFC;                          /* Frame Control: Identifies MAC Header type */
      BYTE      bMacParm;                     /* Parameter field: Dependent on FC */
      USHORT    usLen;                        /* Length of the MAC frame - always set to 0x0000, */
                                              /* will be overwritten by hardware */
      PBYTE     pbEhdr;                       /* Pointer to Extended Header if present or NULL */
      USHORT    usHcs;                        /* Header Checksum - always set to 0x0000, */
                                              /* will be overwritten by hardware */
   } DOCSIS_HDR, *PDOCSIS_HDR;
                                                  
   typedef
   BFC_STATUS
   (*PFN_BFC_HAL_SEND_MGMT_MSG_HANDLER) (
                                       PDOCSIS_HDR pDocsisHdrMgmtMsg, /* UNFINISHED - CABLEMODEM DEPENDENCY! */
                                       BcmHandle   hUsServiceFlow,
                                       PMGMT_MSG   pMgmtMsg,
                                       bool        appendCRC );       /* UNFINISHED - CABLEMODEM DEPENDENCY! */

   typedef
   BFC_STATUS
   (*PFN_BFC_HAL_SEND_DATA_PACKET_HANDLER) (
                                          PDOCSIS_HDR      pDocsisHdrDataPacket, /* UNFINISHED - CABLEMODEM DEPENDENCY! */
                                          PETHERNET_PACKET pEthernetPacket,
                                          BcmHandle           hUsServiceFlow,
                                          UCHAR            ucTxFlags );

   typedef   struct _BFC_HAL_CHARACTERISTICS_
   {

      USHORT                                             usMajorCmMacVersion;
      USHORT                                             usMinorCmMacVersion;
      PFN_BFC_HAL_INIT_HANDLER                            InitHandler;
      PFN_BFC_HAL_SCAN_DNSTREAM_CHANNEL_HANDLER           ScanDnstreamChannelHandler; /* UNFINISHED - CABLEMODEM DEPENDENCY! */
      PFN_BFC_HAL_SCAN_DNSTREAM_FREQUENCY_HANDLER         ScanDnstreamFrequencyHandler; /* UNFINISHED - CABLEMODEM DEPENDENCY! */
      PFN_BFC_HAL_ACQUIRE_DNSTREAM_SYNC_HANDLER           AcquireDnstreamSyncHandler; /* UNFINISHED - CABLEMODEM DEPENDENCY! */
      PFN_BFC_HAL_SET_INFO_HANDLER                        SetInfoHandler;
      PFN_BFC_HAL_QUERY_INFO_HANDLER                        QueryInfoHandler;
      PFN_BFC_HAL_SET_MODE_HANDLER                          SetModeHandler;
      PFN_BFC_HAL_QUERY_MODE_HANDLER                        QueryModeHandler;
      PFN_BFC_HAL_RELEASE_MGMT_MSG_HANDLER                  ReleaseMgmtMsgHandler;
      PFN_BFC_HAL_RELEASE_DATA_PACKET_HANDLER               ReleaseDataPacketHandler;
      PFN_BFC_HAL_SEND_MGMT_MSG_HANDLER                     SendMgmtMsgHandler; /* UNFINISHED - CABLEMODEM DEPENDENCY! */
      PFN_BFC_HAL_SEND_DATA_PACKET_HANDLER                  SendDataPacketHandler;

   } BFC_HAL_CHARACTERISTICS, *PBFC_HAL_CHARACTERISTICS;


/* UNFINISHED - CABLEMODEM DEPENDENCY! */
/*************************************************************************

   BcmGetDocsisDsServiceParams

   Description:

   MACRO - Returns the start of the DS Docsis Service Params structure, which 
   contains Docsis EHDR DS Service Fields. 
   (used for DS Channel Bonding support)

   Parameters:

    uint8 * pEhdrStart - pointer to start of Docsis Ehdr
                         This parameter comes from ETHERNET_PACKET structure:
                         (i.e. pEthernetPacket->eHdr.pbStart)
   Return:
   
    DOCSIS_DOWNSTREAM_SERVICE_PARAMS * - pointer to DS Service Params structure
    
    DATA_BUFFER attributes fields will contain one or more of the following settings
    if Docsis DS Service Params exists:
    (BfcHal.h):
    kDocsisDSID             = 0x0002,   // Docsis Downstream Service ID attribute exists in Docsis Ehdr
    kDocsisDsTrafficPrio    = 0x0004,   // Docsis Downstream Traffic Priority exists in Docsis Ehdr
    kDocsisDsPktSequence    = 0x0008    // Docsis Downstream Packet Sequence number exists in Docsis Ehdr (i.e. DS Bonding)

*************************************************************************/
//DOCSIS_DOWNSTREAM_SERVICE_PARAMS *  BcmGetDocsisDsServiceParams(uint8 * pEhdrStart) 
#define BcmGetDocsisDsServiceParams(pEhdrStart)  \
    (DOCSIS_DOWNSTREAM_SERVICE_PARAMS *)((uint32)pEhdrStart+MAX_DOCSIS_EHDR_LENGTH)

   /*******************************************************************************
   *    BFC_MAC_CHARACTERISTICS and all its prototypes.
   *
   ********************************************************************************/

#define MAJOR_BFC_MAC_VERSION        0x01
#define MINOR_BFC_MAC_VERSION        0x00

   typedef
   BFC_STATUS
   (*PFN_BFC_MAC_REGISTER_HAL_HANDLER) (
                                      PBFC_HAL_CHARACTERISTICS pCmHalCharacteristics, BcmHandle hMac );

   typedef
   VOID
   (*PFN_BFC_MAC_INDICATE_MGMT_MSG_HANDLER) (
                                           PMGMT_MSG pMgmtMsg, BcmHandle hMac ); /* UNFINISHED - CABLEMODEM DEPENDENCY! */

   typedef
   VOID
   (*PFN_BFC_MAC_INDICATE_DATA_PACKET_HANDLER) (
                                              PETHERNET_PACKET pEthernetPacket, BcmHandle hMac );

   typedef
   VOID
   (*PFN_BFC_MAC_DATA_PACKET_SEND_COMPLETE_HANDLER) (
                                                   PETHERNET_PACKET pEthernetPacket, BcmHandle hMac );

   typedef
   VOID
   (*PFN_BFC_MAC_MGMT_MSG_SEND_COMPLETE_HANDLER) (
                                                PMGMT_MSG pMgmtMsg, BcmHandle hMac ); /* UNFINISHED - CABLEMODEM DEPENDENCY! */

   typedef
   VOID
   (*PFN_BFC_MAC_SEND_RESOURCES_AVAILABLE_HANDLER) (
                                                  BcmHandle hMac );

   typedef
   VOID
   (*PFN_BFC_MAC_INDICATE_STATUS_HANDLER) (
                                         BFC_MAC_OID CmMacOid,
                                         ULONG      ulParam,
                                         PVOID      pvStatusBuffer,
                                         ULONG      ulStatusBufferLen,
                                         BcmHandle     hMac );

   typedef
   BFC_STATUS
   (*PFN_BFC_MAC_ALLOCATE_DATA_BUFFER_MEMORY) (
                                             PVOID     *ppvAddr,
                                             UINT      uiLen,
                                             BcmHandle    hMac );


   typedef struct _BFC_MAC_CHARACTERISTICS
   {
      USHORT                                       usMajorVersion;
      USHORT                                       usMinorVersion;
      UINT                                         uiInterfaceNumber;
      PFN_BFC_MAC_REGISTER_HAL_HANDLER              RegisterHalHandler;
      PFN_BFC_MAC_INDICATE_MGMT_MSG_HANDLER         IndicateMgmtMsgHandler;
      PFN_BFC_MAC_INDICATE_DATA_PACKET_HANDLER      IndicateDataPacketHandler;
      PFN_BFC_MAC_DATA_PACKET_SEND_COMPLETE_HANDLER DataPacketSendCompleteHandler;
      PFN_BFC_MAC_MGMT_MSG_SEND_COMPLETE_HANDLER    MgmtMsgSendCompleteHandler; /* UNFINISHED - CABLEMODEM DEPENDENCY! */
      PFN_BFC_MAC_SEND_RESOURCES_AVAILABLE_HANDLER  SendResourcesAvailableHandler;
      PFN_BFC_MAC_INDICATE_STATUS_HANDLER           IndicateStatusHandler;
      PFN_BFC_MAC_ALLOCATE_DATA_BUFFER_MEMORY       AllocateDataBufferMemoryHandler;
      PVOID                                        pvCableModemParms; /* UNFINISHED - CABLEMODEM DEPENDENCY! */
      BcmHandle                                       hMac;
      BcmHandle                                       hHalThreadPriority;
   } BFC_MAC_CHARACTERISTICS, *PBFC_MAC_CHARACTERISTICS;

   //////////////////////////////////////////////////////////////////////////////
   //  The adapter structure that controls the operational of network interface
   //  hardware.
   //
   typedef struct NI_ADAPTER 
   {

      BFC_MAC_CHARACTERISTICS     CmMacCharacteristics;
      INT               uiInterfaceNumber;
      UCHAR MacAddr[6];
      ULONG IPAddr;
   }   NI_ADAPTER, *PNI_ADAPTER;


   

   /*******************************************************************************
   *    CM HAL Driver Entry Function
   */

   typedef
   BFC_STATUS
   (*PFN_BFC_HAL_DRIVER_ENTRY) (
                              PBFC_MAC_CHARACTERISTICS pCmMacCharacteristics );

#ifdef	PACKET_PROBE

   ////////////////////////////////////////////////////////////////////////////////
   // Diagnostic/test Objects for system profiling
   //
#define	PACKET_PROBE_ADDRESS	0xba000000			// EBI location of packet probe:
   typedef struct _PacketProbe
   {
      uint32   address;                      // 	0xba000000
      uint16   length;                          //	0xba000004
      byte     signal;                          //	0xba000006
#define	PACKET_PROBE_SIGNAL_UPSTREAM_CAPTURE_ON		0x01
#define	PACKET_PROBE_SIGNAL_UPSTREAM_CAPTURE_OFF	0x02
#define	PACKET_PROBE_SIGNAL_DOWNSTREAM_CAPTURE_ON  	0x04
#define	PACKET_PROBE_SIGNAL_DOWNSTREAM_CAPTURE_OFF	0x08
      byte     unused0;
   }PacketProbe;
#endif

#if defined __cplusplus
}
#endif

//This structure defines the nonBonded Pkt channel encapsulation for use in the bonded
//channel system.
typedef struct nonBondedChannelPktDETStruct
{
   uint8    detDA[6];         //This is the address of the DET bonded channel FPGA.
   uint8    detSA[6];         //Address of the send CM MAC.
   uint16   detTypeLength;    //0x88AC. Used only for bonded channel systems between CM and FPGA.
   uint8    detRSV_1;         //Reserved field set to 0x00 for now.
   uint8    detQueue;         //Index of the detination CM MII queue.
   uint16   detPktLen;        //Length in bytes of the encapsulated data.
   uint16   detRSV_2;         //Reserved field set to 0x0000 for now.
} nonBondedChannelPktDETStruct;

BFC_STATUS ExtractLongFromVoid (PVOID pvInfoBuffer, ULONG ulInfoBufferLen, PULONG pulVal);
BFC_STATUS ExtractShortFromVoid (PVOID pvInfoBuffer, ULONG ulInfoBufferLen, PUSHORT pusVal);
BFC_STATUS ExtractByteFromVoid (PVOID pvInfoBuffer, ULONG ulInfoBufferLen, PUCHAR pucVal);
BFC_STATUS AssignLongToVoid (PVOID pvInfoBuffer, PULONG pulInfoBufferLen, ULONG ulVal);
BFC_STATUS AssignShortToVoid (PVOID pvInfoBuffer, PULONG pulInfoBufferLen, USHORT usVal);
BFC_STATUS AssignByteToVoid (PVOID pvInfoBuffer, PULONG pulInfoBufferLen, UCHAR ucVal);

#endif    /* BFCHAL_H */

