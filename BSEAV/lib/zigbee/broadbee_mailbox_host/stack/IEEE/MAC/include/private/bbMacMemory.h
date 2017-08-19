/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

/*******************************************************************************
 *
 * DESCRIPTION:
 *      MAC Memory interface.
 *
*******************************************************************************/

#ifndef _BB_MAC_MEMORY_H
#define _BB_MAC_MEMORY_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacPibDefs.h"
#include "private/bbMacMpdu.h"
#include "private/bbMacCfgFsm.h"
#include "bbPhySapForMac.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for descriptor of MAC-FE Main and Pending Request Queues.
 */
typedef struct _MacMemoryQueueDescr_t
{
    /* Structured / 32-bit data. */
    SYS_QueueElement_t  queueHead;      /*!< Head of the MAC-FE Requests Queue. */

    SYS_QueueElement_t  queueTail;      /*!< Tail of the MAC-FE Requests Queue. */

} MacMemoryQueueDescr_t;


/**//**
 * \brief   Size of MAC-FE Pending Destination Address Hash Set, in bits.
 */
#define MAC_MEMORY_PENDING_DEST_ADDR_HASH_SET_SIZE      (1 << (sizeof(MacAddrHash_t) * 8))

SYS_DbgAssertStatic(256 == MAC_MEMORY_PENDING_DEST_ADDR_HASH_SET_SIZE);


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/**//**
 * \brief   Structure for the MLME-SCAN.request Processor extended state.
 */
typedef struct _MacFeScanExtState_t
{
    /* 32-bit data. */
    PHY_ChannelMask_t      scanChannels;                /*!< Set of channels to be scanned. */

    PHY_ChannelMask_t      unscannedChannels;           /*!< Set of channels that were not or left not scanned. */

    /* 32-bit data. */
    union
    {
        SYS_DataPointer_t  payload;                     /*!< Scan Results List in general. */

        SYS_DataPointer_t  energyDetectList;            /*!< Energy Detect List to be returned with confirmation. */

        SYS_DataPointer_t  panDescriptorsList;          /*!< PAN Descriptors List to be returned with confirmation. */
    };

    /* 16-bit data. */
    MAC_PanId_t            originalPanId;               /*!< Original value of MAC-PIB attribute macPANId. */

    /* 8-bit data. */
    PHY_PageChannel_t      originalChannelOnPage;       /*!< Original Channel Page and Logical Channel. */

    PHY_PageChannel_t      currentChannelOnPage;        /*!< Currently scanned Channel Page and Logical Channel. */

    MAC_RxOnWhenIdle_t     originalRxOnWhenIdle;        /*!< Original value of MAC-PIB attribute macRxOnWhenIdle. */

    uint8_t                resultsCounter;              /*!< Counter for Result List Size. */

    /* 8(1)-bit data. */
    Bool8_t                limitReached;                /*!< Flag for LIMIT_REACHED confirmation status. */

    Bool8_t                haveBeacon;                  /*!< Flag for !(NO_BEACON) confirmation status. */

} MacFeScanExtState_t;
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */


/**//**
 * \brief   Structure for the MAC-FE and MAC-PIB private memory.
 * \note    The MAC-FE Pending Requests Queue is not implemented for single-context MAC
 *  for RF4CE, because RF4CE stack does not use indirect transmission.
 * \note    The MAC-PIB attributes for Beacons and Scanning support are not implemented by
 *  single-context MAC for RF4CE-Controller, because RF4CE-Controller does not support
 *  beacon frames and scanning.
 */
typedef struct _MacMemoryFeData_t
{
    /* Structured / 2x32-bit data. */
    MacMemoryQueueDescr_t          macFeMainReqQueue;                           /*!< Descriptor of the MAC-FE Main
                                                                                    Requests Queue. */
#if defined(_MAC_CONTEXT_ZBPRO_)
    /* Structured / 2x32-bit data. */
    MacMemoryQueueDescr_t          macFePendingReqQueue;                        /*!< Descriptor of the MAC-FE Pending
                                                                                    Requests Queue. */
#endif

#if defined(_HAL_USE_PRNG_)
    /* Structured / 32-bit data. */
    HAL_PrngDescr_t                macFePrngDescr[MAC_CONTEXTS_NUMBER];         /*!< Descriptors of Pseudo-Random Number
                                                                                    Generator (PRNG). */
#endif

    /* 32-bit data. */
    MacServiceField_t             *macFeActiveReq;                              /*!< Pointer to the service field of
                                                                                    the descriptor of the MAC-FE
                                                                                    Active Request/Response. */
#if defined(_MAC_CONTEXT_ZBPRO_)
    /* Array / 32x8-bit data. */
    BITMAP_DECLARE(macFePendingDestAddrHashSet, MAC_MEMORY_PENDING_DEST_ADDR_HASH_SET_SIZE);
                                                                                /*!< MAC-FE Pending Destination Address
                                                                                    Hash Set. */
#endif

#if defined(_MAC_CONTEXT_ZBPRO_)
    /* Array / 16x8-bit data. */
    PHY_Octet_t                     macPibBeaconPayloadZBPRO[MAC_ATTR_MAXALLOWED_VALUE_BEACON_PAYLOAD_LENGTH_ZBPRO];
                                                                                /*!< Value of the macBeaconPayload
                                                                                    attribute of the ZigBee PRO
                                                                                    MAC Context. */
#endif

#if defined(_MAC_CONTEXT_RF4CE_TARGET_)
    /* Array / 4x8-bit data. */
    PHY_Octet_t                     macPibBeaconPayloadRF4CE[MAC_ATTR_MAXALLOWED_VALUE_BEACON_PAYLOAD_LENGTH_RF4CE];
                                                                                /*!< Value of the macBeaconPayload
                                                                                    attribute of the RF4CE-Target
                                                                                    MAC Context. */
#endif

#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
    /* Structured data. */
    MacServiceField_t              macFeBeaconResp[MAC_CONTEXTS_NUMBER];        /*!< MLME-BEACON.response internal
                                                                                    descriptors. */
#endif

    /* Structured data. */
    MacPibPermanent_t              macPibPermanent[MAC_CONTEXTS_NUMBER];        /*!< Permanent part of the MAC-PIB. */

    MacPibEssential_t              macPibEssential[MAC_CONTEXTS_NUMBER];        /*!< Essential part of the MAC-PIB. */

#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
    /* Structured data. */
    MacPibBeaconing_t              macPibBeaconing[MAC_CONTEXTS_NUMBER];        /*!< The MAC-PIB for Beacons support. */
#endif

#if defined(_MAC_CONTEXT_ZBPRO_)
    /* Structured data. */
    MacPibZBPRO_t                  macPibZBPRO;                                 /*!< ZigBee PRO specific part of the
                                                                                    MAC-PIB. */

    MacPibSecurity_t               macPibSecurity;                              /*!< MAC Security specific part of the
                                                                                    MAC-PIB. */
#endif

#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_) || defined(_MAC_CONTEXT_ZBPRO_)

    /* Extended state for different MAC-FE processors. Notice that there may be only one active processor at the same
     * time. */

    union {

# if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
        /* Structured data. */
        MacFeScanExtState_t        macFeScanExtState;       /*!< Currently active MLME-SCAN.request processor extended
                                                                state. */
# endif
    };
#endif /* !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_) || defined(_MAC_CONTEXT_ZBPRO_) */

    /* Structured data. */
    SYS_SchedulerTaskDescriptor_t  macFeTasksDescriptor;                        /*!< The MAC Tasks Descriptor. */

    /* 8-bit data. */
    SYS_FSM_StateId_t              macFeReqProcFsmState;                        /*!< State of the FSM of the
                                                                                    currently active MAC-FE
                                                                                    Request/Response Processor. */
} MacMemoryFeData_t;


/**//**
 * \brief   Pointer to the MAC-FE Main Requests Queue.
 */
#define MAC_MEMORY_FE_MAIN_REQ_QUEUE()                  (&(macMemoryFeData.macFeMainReqQueue))


#if defined(_MAC_CONTEXT_ZBPRO_)
/**//**
 * \brief   Pointer to the MAC-FE Pending Requests Queue.
 */
# define MAC_MEMORY_FE_PENDING_REQ_QUEUE()              (&(macMemoryFeData.macFePendingReqQueue))


/**//**
 * \brief   Reference to the MAC-FE Pending Destination Address Hash Set.
 */
# define MAC_MEMORY_PENDING_DEST_ADDR_HASH_SET()        (macMemoryFeData.macFePendingDestAddrHashSet)
#endif /* _MAC_CONTEXT_ZBPRO_ */


/**//**
 * \brief   Returns pointer to the specified MAC-FE Requests Queue head element.
 * \param[in]   queueDescr      Pointer to the MAC-FE Requests Queue descriptor.
 * \return  Pointer to the specified MAC-FE Requests Queue head element.
 */
#define MAC_MEMORY_FE_REQ_QUEUE_HEAD(queueDescr)        ((queueDescr)->queueHead.nextElement)


/**//**
 * \brief   Returns pointer to the specified MAC-FE Requests Queue tail element.
 * \param[in]   queueDescr      Pointer to the MAC-FE Requests Queue descriptor.
 * \return  Pointer to the specified MAC-FE Requests Queue tail element.
 */
#define MAC_MEMORY_FE_REQ_QUEUE_TAIL(queueDescr)        ((queueDescr)->queueTail.nextElement)


/**//**
 * \brief   Returns pointer to the MAC-FE Active Request/Response.
 * \details Serves as a shortcut for pointer to the service field of descriptor of the
 *  MAC-FE Active Request/Response.
 * \details If there is currently no active Request/Response (i.e., the MAC is in the IDLE
 *  state), the returned value is NULL.
 */
#define MAC_MEMORY_FE_ACTIVE_REQ()                      (macMemoryFeData.macFeActiveReq)                    /* TODO: Implement via the local automated variable. */


/**//**
 * \brief   Returns the type of the MAC-FE Active Request/Response.
 * \details Takes the pointer to the MAC-FE Active Request/Response descriptor service
 *  field and returns the numeric identifier of the type of this active Request/Response.
 * \note    If there is currently no active Request/Response (i.e., the MAC is in the IDLE
 *  state), the returned value is undefined. Use precondition
 *  <tt>(NULL != MAC_MEMORY_FE_ACTIVE_REQ())</tt>
 *  to avoid improper behavior of this macro.
 */
#define MAC_MEMORY_FE_ACTIVE_REQ_TYPE()                 (MAC_MEMORY_FE_ACTIVE_REQ()->primitiveId)           /* TODO: Implement via the local automated variable. */


#if defined(_MAC_DUAL_CONTEXT_)
/**//**
 * \brief   Returns the context of the currently active MAC-FE Request/Response.
 * \details Takes the pointer to the MAC-FE Active Request/Response descriptor service
 *  field and returns the numeric identifier of the context of this active
 *  Request/Response.
 * \note    If there is currently no active Request/Response (i.e., the MAC is in the IDLE
 *  state), the returned value is undefined. Use precondition
 *  <tt>(NULL != MAC_MEMORY_FE_ACTIVE_REQ())</tt>
 *  to avoid improper behavior of this macro.
 * \details For the context-number-safe code use wrapper <tt>MAC_CONTEXT_ID()</tt>.
 */
# define MAC_MEMORY_FE_ACTIVE_REQ_CONTEXT()             (MAC_MEMORY_FE_ACTIVE_REQ()->contextId)             /* TODO: Implement via the local automated variable. */
#endif /* _MAC_DUAL_CONTEXT_ */


#if defined(_MAC_CONTEXT_ZBPRO_)
/**//**
 * \brief   Reference to the macBeaconPayload attribute static storage of the
 *  ZigBee PRO MAC-PIB.
 */
# define MAC_MEMORY_PIB_BEACON_PAYLOAD_ZBPRO()          (macMemoryFeData.macPibBeaconPayloadZBPRO)
#endif


#if defined(_MAC_CONTEXT_RF4CE_TARGET_)
/**//**
 * \brief   Reference to the macBeaconPayload attribute static storage of the
 *  RF4CE-Target MAC-PIB.
 */
# define MAC_MEMORY_PIB_BEACON_PAYLOAD_RF4CE()          (macMemoryFeData.macPibBeaconPayloadRF4CE)
#endif


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/**//**
 * \brief   Reference to the MLME-BEACON.response internal descriptors.
 * \details Uses the __givenContextId hidden variable to select the corresponding MAC
 *  Context.
 */
# define MAC_MEMORY_FE_BEACON_RESP(context)             (macMemoryFeData.macFeBeaconResp[MAC_CONTEXT_IDX(context)])
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */


/**//**
 * \brief   Reference to the attributes storage of the MAC-PIB permanent part.
 * \details Uses the __givenContextId hidden variable to select the corresponding MAC
 *  Context.
 */
#define MAC_MEMORY_PIB_PERMANENT()                      (macMemoryFeData.macPibPermanent[MAC_GIVEN_CONTEXT_IDX])


/**//**
 * \brief   Reference to the attributes storage of the MAC-PIB essential part.
 * \details Uses the __givenContextId hidden variable to select the corresponding MAC
 *  Context.
 */
#define MAC_MEMORY_PIB_ESSENTIAL()                      (macMemoryFeData.macPibEssential[MAC_GIVEN_CONTEXT_IDX])


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/**//**
 * \brief   Reference to the attributes storage of the MAC-PIB for Beacons support.
 * \details Uses the __givenContextId hidden variable to select the corresponding MAC
 *  Context.
 */
# define MAC_MEMORY_PIB_BEACONING()                     (macMemoryFeData.macPibBeaconing[MAC_GIVEN_CONTEXT_IDX])
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */


#if defined(_MAC_CONTEXT_ZBPRO_)
/**//**
 * \brief   Reference to the attributes storage of the MAC-PIB ZigBee PRO specific part.
 */
# define MAC_MEMORY_PIB_ZBPRO()                         (macMemoryFeData.macPibZBPRO)

/**//**
 * \brief   Reference to the attributes storage of the MAC-PIB Security specific part.
 */
# define MAC_MEMORY_PIB_SECURITY()                      (macMemoryFeData.macPibSecurity)
#endif /* _MAC_CONTEXT_ZBPRO_ */


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/**//**
 * \brief   Reference to the currently active MLME-SCAN.request processor extended state.
 */
# define MAC_MEMORY_FE_SCAN_EXT_STATE()                 (macMemoryFeData.macFeScanExtState)
#endif


/**//**
 * \brief   Reference to the MAC Tasks Descriptor.
 */
#define MAC_MEMORY_FE_TASKS_DESCR()                     (macMemoryFeData.macFeTasksDescriptor)


/**//**
 * \brief   Reference to the MAC-FE Active Request/Response Processor FSM state.
 */
#define MAC_MEMORY_FE_FSM_STATE()                       (macMemoryFeData.macFeReqProcFsmState)


#if defined(_HAL_USE_PRNG_)
/**//**
 * \brief   Pointer to the PRNG Descriptor.
 * \details Uses the __givenContextId hidden variable to select the corresponding MAC
 *  Context.
 */
# define MAC_MEMORY_FE_PRNG_DESCR()                     (&(macMemoryFeData.macFePrngDescr[MAC_GIVEN_CONTEXT_IDX]))


/**//**
 * \brief   Pointer to the PRNG Descriptor belonging to the active MAC-FE context.
 */
# define MAC_MEMORY_FE_ACTIVE_PRNG_DESCR()              (&(macMemoryFeData.macFePrngDescr[MAC_ACTIVE_CONTEXT_IDX]))


/**//**
 * \brief   Pointer to the PRNG Descriptor belonging to the specified MAC-FE context.
 * \param[in]   context     Context index macro.
 */
# define MAC_MEMORY_FE_CONTEXT_PRNG_DESCR(context)      (&(macMemoryFeData.macFePrngDescr[context]))

#else /* _HAL_USE_TRNG_ */
/**//**
 * \brief   Stub for pointer to the PRNG Descriptor in the case of build using TRGN.
 */
# define MAC_MEMORY_FE_PRNG_DESCR()                     NULL


/**//**
 * \brief   Stub for pointer to the PRNG Descriptor belonging to the active MAC-FE context
 *  in the case of build using TRGN.
 */
# define MAC_MEMORY_FE_ACTIVE_PRNG_DESCR()              NULL


/**//**
 * \brief   Stub for pointer to the PRNG Descriptor belonging to the specified MAC-FE
 *  context in the case of build using TRGN.
 * \param[in]   context     Context index macro (ignored).
 */
# define MAC_MEMORY_FE_CONTEXT_PRNG_DESCR(context)      NULL

#endif /* _HAL_USE_TRNG_ */


/**//**
 * \brief   Structure for the set of reasons to hold transceiver in the TRX_ON_WHEN_IDLE
 *  state.
 */
typedef struct _MacMemoryLeTrxOnReasons_t
{
    BitField8_t  persistent : 1;    /*!< Hold in TRX_ON_WHEN_IDLE after macRxOnWhenIdle attribute. */

    BitField8_t  timedSap   : 1;    /*!< Hold in TRX_ON_WHEN_IDLE after MLME-RX-ENABLE.request. */

    BitField8_t  timedFsm   : 1;    /*!< Hold in TRX_ON_WHEN_IDLE after MAC-LE Real-Time Dispatcher FSM. */

} MacMemoryLeTrxOnReasons_t;


/**//**
 * \brief   Structure for the MAC-LE Transceiver Mode Dispatcher memory.
 */
typedef struct _MacMemoryLeTrxModeDisp_t
{
    HAL_Symbol__Tstamp_t   timeoutSap[MAC_CONTEXTS_NUMBER];     /*!< Timestamps of TRX_ON_WHEN_IDLE timeout after
                                                                    MLME-RX-ENABLE.request for two contexts. */
#if defined(_MAC_CONTEXT_ZBPRO_)
    HAL_Symbol__Tstamp_t   timeoutFsmZBPRO;         /*!< Timestamp of TRX_ON_WHEN_IDLE timeout after
                                                        MAC-LE Real-Time Dispatcher FSM for ZigBee PRO context. */
#endif

    union
    {
#if defined(_MAC_DUAL_CONTEXT_)
        BitField16_t  trxOnWhenIdle;        /*!< Hold in TRX_ON_WHEN_IDLE consolidated value for dual-context MAC. */
#else /* _MAC_SINGLE_CONTEXT_ */
        BitField8_t   trxOnWhenIdle;        /*!< Hold in TRX_ON_WHEN_IDLE consolidated value for single-context MAC. */
#endif
        MacMemoryLeTrxOnReasons_t  reasons[MAC_CONTEXTS_NUMBER];    /*!< Hold in TRX_ON_WHEN_IDLE consolidated values
                                                                        for each context. */
    };
} MacMemoryLeTrxModeDisp_t;


/**//**
 * \brief   Reference to the MAC-LE Transceiver Mode Dispatcher memory.
 */
#define MAC_MEMORY_LE_TRX_MODE_DISP()           (macMemoryLeTrxModeDisp)


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Memory of the MAC-FE and the MAC-PIB.
 */
MAC_MEMDECL( MacMemoryFeData_t  macMemoryFeData );


/**//**
 * \brief   Memory for the MAC-LE Transceiver Mode Dispatcher.
 */
MAC_MEMDECL( volatile MacMemoryLeTrxModeDisp_t  macMemoryLeTrxModeDisp );


#if defined(_MAC_TESTER_)
/**//**
 * \brief   Memory for MAC-PIB attribute macPromiscuousMode.
 * \note    Attribute macPromiscuousMode is shared by both contexts for the case of dual
 *  context MAC.
 */
MAC_MEMDECL( volatile MAC_PromiscuousMode_t  macMemoryPromiscuousMode );
#endif


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Puts request/response into the specified MAC-FE Requests Queue tail.
 * \params[in]  queueDescr  Pointer to the MAC-FE Requests Queue descriptor.
 * \params[in]  reqService  Pointer to the service field of request/response descriptor to
 *  be put into the queue.
*****************************************************************************************/
MAC_PRIVATE void macMemoryReqQueuePutToTail(MacMemoryQueueDescr_t *const  queueDescr,
                                            MacServiceField_t *const      reqService);
#else /* ! _MAC_CONTEXT_ZBPRO_ */
/*************************************************************************************//**
 * \brief   Puts request/response into the MAC-FE Main Requests Queue tail.
 * \params[in]  reqService  Pointer to the service field of request/response descriptor to
 *  be put into the queue.
*****************************************************************************************/
MAC_PRIVATE void macMemoryReqQueuePutToTail(MacServiceField_t *const reqService);
#endif


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Puts request/response into the specified MAC-FE Requests Queue head.
 * \params[in]  queueDescr  Pointer to the MAC-FE Requests Queue descriptor.
 * \params[in]  reqService  Pointer to the service field of request/response descriptor to
 *  be put into the queue.
*****************************************************************************************/
MAC_PRIVATE void macMemoryReqQueuePutToHead(MacMemoryQueueDescr_t *const  queueDescr,
                                            MacServiceField_t *const      reqService);
#else /* ! _MAC_CONTEXT_ZBPRO_ */
/*************************************************************************************//**
 * \brief   Puts request/response into the MAC-FE Main Requests Queue head.
 * \params[in]  reqService  Pointer to the service field of request/response descriptor to
 *  be put into the queue.
*****************************************************************************************/
MAC_PRIVATE void macMemoryReqQueuePutToHead(MacServiceField_t *const reqService);
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Puts new request/response into the MAC-FE Main Requests Queue tail.
 * \params[in]  reqService  Pointer to the service field of request/response descriptor to
 *  be put into the queue.
 */
#if defined(_MAC_CONTEXT_ZBPRO_)
# define macMemoryMainReqQueuePutToTail(reqService)\
        macMemoryReqQueuePutToTail(MAC_MEMORY_FE_MAIN_REQ_QUEUE(), (reqService))
#else
# define macMemoryMainReqQueuePutToTail(reqService)\
        macMemoryReqQueuePutToTail(reqService)
#endif


/**//**
 * \brief   Puts new request/response into the MAC-FE Main Requests Queue head.
 * \params[in]  reqService  Pointer to the service field of request/response descriptor to
 *  be put into the queue.
 */
#if defined(_MAC_CONTEXT_ZBPRO_)
# define macMemoryMainReqQueuePutToHead(reqService)\
        macMemoryReqQueuePutToHead(MAC_MEMORY_FE_MAIN_REQ_QUEUE(), (reqService))
#else
# define macMemoryMainReqQueuePutToHead(reqService)\
        macMemoryReqQueuePutToHead(reqService)
#endif


#if defined(_MAC_CONTEXT_ZBPRO_)
/**//**
 * \brief   Puts indirect request/response into the MAC-FE Pending Requests Queue tail.
 * \params[in]  reqService  Pointer to the service field of request/response descriptor to
 *  be put into the queue.
 */
# define macMemoryPendingReqQueuePutToTail(reqService)\
        macMemoryReqQueuePutToTail(MAC_MEMORY_FE_PENDING_REQ_QUEUE(), (reqService))


/**//**
 * \brief   Puts indirect request/response into the MAC-FE Pending Requests Queue head.
 * \params[in]  reqService  Pointer to the service field of request/response descriptor to
 *  be put into the queue.
 */
# define macMemoryPendingReqQueuePutToHead(reqService)\
        macMemoryReqQueuePutToHead(MAC_MEMORY_FE_PENDING_REQ_QUEUE(), (reqService))
#endif /* _MAC_CONTEXT_ZBPRO_ */


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Extracts the first request/response from the MAC-FE Main Requests Queue head.
 * \return  Pointer to the service field of request/response descriptor extracted from the
 *  queue.
*****************************************************************************************/
MAC_PRIVATE MacServiceField_t* macMemoryMainReqQueueGetFromHead(void);


/*************************************************************************************//**
 * \brief   Discovers if the MAC-FE Main Requests Queue is empty or not.
 * \return  TRUE if the MAC-FE Main Requests Queue is empty; FALSE otherwise.
*****************************************************************************************/
MAC_PRIVATE bool macMemoryMainReqQueueIsNotEmpty(void);


/*************************************************************************************//**
 * \brief   Resets MAC-PIB attributes to their default values for the specified context.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
*****************************************************************************************/
MAC_PRIVATE void macMemoryPibReset(MAC_WITHIN_GIVEN_CONTEXT);


#endif /* _BB_MAC_MEMORY_H */

/* eof bbMacMemory.h */