/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
/*****************************************************************************
 *
 * FILENAME: $Workfile: branches/dkiselev/ZRC2/stack/RF4CE/Profiles/ZRC/include/bbRF4CEZRCAttributes.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE private ZRC profile attributes handler.
 *
 * $Revision: 4430 $
 * $Date: 2014-11-10 14:33:34Z $
 *
 ****************************************************************************************/
#ifndef BBRF4CEZRCPRIVATEATTRIBUTES_H
#define BBRF4CEZRCPRIVATEATTRIBUTES_H

/************************* INCLUDES ****************************************************/
#include "bbSysTable.h"
#include "bbNvmApi.h"
#include "bbRF4CEPM.h"
#include "bbRF4CEGDPDefaultAttributeValues.h"
#include "bbRF4CEZRCAttributes.h"

#if defined(USE_RF4CE_PROFILE_ZRC2)

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Is a Local ZRC2 Attribute?
 */
#define RF4CE_ZRC2_ATTR_IS_LOCAL(pairRef)   (RF4CE_NWK_INVALID_PAIRING_REF == (pairRef))

/**//**
 * \brief Local ZRC2 Attributes index
 */
#define RF4CE_ZRC2_ATTR_LOCAL_INDEX         RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES

/**//**
 * \brief ZRC2 Simple Attributes look up table
 */
#define RF4CE_ZRC2_SIMPLE_ATTR_LOOKUP(var) \
    struct { uint8_t attrId; uint8_t offset; uint8_t size } var[] = \
    { \
        RF4CE_ZRC2_SIMPLE_ATTR_DECLARATIONS \
    }

/************************* TYPES *******************************************************/

/**//**
 * \brief Declaration of ZRC2 Simple Attributes
 */
#define RF4CE_ZRC2_SIMPLE_ATTR_DECLARE(attrId, name, type, defaultValue)    \
        type RF4CE_ZRC2_##name##_var;
typedef struct _RF4CE_ZRC2_SimpleAttributes_t
{
    RF4CE_ZRC2_SIMPLE_ATTR_DECLARATIONS
} RF4CE_ZRC2_SimpleAttributes_t;
#undef RF4CE_ZRC2_SIMPLE_ATTR_DECLARE

/**//**
 * \brief ZRC2 Action Mapping buffer
 */
typedef struct _Rf4ceZrc2ActionMappingBuffer_t
{
    uint8_t     data[RF4CE_ZRC2_ACTION_MAPPINGS_ENTRY_MAX_SIZE];
    uint8_t     size;
} Rf4ceZrc2ActionMappingBuffer_t;

/**//**
 * \brief ZRC2 HA Attribute record table
 */
#define RF4CE_ZRC2_ATTRIBUTE_HA_RECORD_BUSY_BIT     7
typedef struct _Rf4ceZrc2HaRecord_t
{
    struct
    {
        BitField8_t     seqNum  : RF4CE_ZRC2_ATTRIBUTE_HA_RECORD_BUSY_BIT;
        BitField8_t     busy    : 1;
    } service;

    uint8_t             pair;
    uint8_t             instanceId;
    uint8_t             haAttrId;
    SYS_DataPointer_t   value;
} Rf4ceZrc2HaRecord_t;

/**//**
 * \brief ZRC2 HA Attribute record table
 */
extern SYS_TableDescriptor_t rf4cezrc2AttributesHaTableDescr;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Pull request descriptor definition.
 */
typedef struct _RF4CE_ZRC2_PullAttributesReqDescr_t RF4CE_ZRC2_PullAttributesReqDescr_t;

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Pull request callback.
 */
typedef void (*RF4CE_ZRC2_PullAttributesReqCallback_t)(RF4CE_ZRC2_PullAttributesReqDescr_t *req, RF4CE_ZRC2_SetAttributesConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 2.0 Attributes Pull request descriptor.
 */
struct _RF4CE_ZRC2_PullAttributesReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;             /*!< Service field */
    uint8_t pairingRef;                             /*!< Service field */
#endif /* _HOST_ */
    RF4CE_ZRC2_GetAttributesReqParams_t params;     /*!< Request parameters */
    RF4CE_ZRC2_PullAttributesReqCallback_t callback; /*!< Request callback */
};

/**//**
 * \brief Init RF4CE ZRC 2.0 Attributes callback.
 */
typedef void (*RF4CE_ZRC2_InitAttributesCallback_t)(void);

/**//**
 * \brief On Generic Response reception callback.
 */
typedef void (*Rf4cezrc2GenericResponseRxCallback_t)(RF4CE_ZRC2GDP2_Status_t status);

/************************* FUNCTIONS PROTOTYPES ****************************************/

/************************************************************************************//**
 \brief Starts ZRC 2.0 local Get Attributes Request. Internal usage only!

 \param[in] request - pointer to the request descriptor.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2GetAttributesReq(RF4CE_ZRC2_GetAttributesReqDescr_t *request);

/************************************************************************************//**
 \brief Starts ZRC 2.0 local Set Attributes Request. Internal usage only!

 \param[in] request - pointer to the request descriptor.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2SetAttributesReq(RF4CE_ZRC2_SetAttributesReqDescr_t *request);

/************************************************************************************//**
 \brief Starts ZRC 2.0 Push Attributes Request. Internal usage only!

 \param[in] request - pointer to the request descriptor.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2PushAttributesReq(RF4CE_ZRC2_SetAttributesReqDescr_t *request);

/************************************************************************************//**
 \brief Starts ZRC 2.0 Pull Attributes Request. Internal usage only!

 \param[in] request - pointer to the request descriptor.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2PullAttributesReq(RF4CE_ZRC2_PullAttributesReqDescr_t *request);

/************************************************************************************//**
 \brief Inits ZRC 2.0 Attributes. Internal usage only!

 \param[in] toDefault   - true to init with deafult attributes values
 \param[in] callback    - pointer to the callback function to be called upon completion.
 ****************************************************************************************/
void rf4cezrc2InitAttributes(bool toDefault, RF4CE_ZRC2_InitAttributesCallback_t callback);

/************************************************************************************//**
 \brief Generic Response ZRC 2.0 data indication.

 \param[in] indication - pointer to the indication data structure.
 \param[in] length - the length in bytes of the incoming payload.
 \param[in] leaveReceiverOn - true if necessary to leave receiver on.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2GenericResponseInd(RF4CE_NWK_DataIndParams_t *indication, uint32_t length, bool leaveReceiverOn);

/************************************************************************************//**
 \brief Get Attributes ZRC 2.0 data indication.

 \param[in] indication - pointer to the indication data structure.
 \param[in] length - the length in bytes of the incoming payload.
 \param[in] leaveReceiverOn - true if necessary to leave receiver on.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2GetAttributesInd(RF4CE_NWK_DataIndParams_t *indication, uint32_t length, bool leaveReceiverOn);

/************************************************************************************//**
 \brief Get Attributes Response ZRC 2.0 data indication.

 \param[in] indication - pointer to the indication data structure.
 \param[in] length - the length in bytes of the incoming payload.
 \param[in] leaveReceiverOn - true if necessary to leave receiver on.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2GetAttributesResponseInd(RF4CE_NWK_DataIndParams_t *indication, uint32_t length, bool leaveReceiverOn);

/************************************************************************************//**
 \brief Push Attributes ZRC 2.0 data indication.

 \param[in] indication - pointer to the indication data structure.
 \param[in] length - the length in bytes of the incoming payload.
 \param[in] leaveReceiverOn - true if necessary to leave receiver on.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2PushAttributesInd(RF4CE_NWK_DataIndParams_t *indication, uint32_t length, bool leaveReceiverOn);

/************************************************************************************//**
 \brief Set Attributes ZRC 2.0 data indication.

 \param[in] indication - pointer to the indication data structure.
 \param[in] length - the length in bytes of the incoming payload.
 \param[in] leaveReceiverOn - true if necessary to leave receiver on.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2SetAttributesInd(RF4CE_NWK_DataIndParams_t *indication, uint32_t length, bool leaveReceiverOn);

/************************************************************************************//**
 \brief Pull Attributes ZRC 2.0 data indication.

 \param[in] indication - pointer to the indication data structure.
 \param[in] length - the length in bytes of the incoming payload.
 \param[in] leaveReceiverOn - true if necessary to leave receiver on.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2PullAttributesInd(RF4CE_NWK_DataIndParams_t *indication, uint32_t length, bool leaveReceiverOn);

/************************************************************************************//**
 \brief Pull Attributes Response ZRC 2.0 data indication.

 \param[in] indication - pointer to the indication data structure.
 \param[in] length - the length in bytes of the incoming payload.
 \param[in] leaveReceiverOn - true if necessary to leave receiver on.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2PullAttributesResponseInd(RF4CE_NWK_DataIndParams_t *indication, uint32_t length, bool leaveReceiverOn);

/************************************************************************************//**
 \brief Ends ZRC 2.0 Get Request with error.

 \param[in] status - error code.
 \param[in] doRxDisable - true if necessary to do RX Disable.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2GetAttributeError(uint8_t status, bool doRxDisable);

/************************************************************************************//**
 \brief Ends ZRC 2.0 Set Request with error.

 \param[in] status - error code.
 \param[in] doRxDisable - true if necessary to do RX Disable.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2SetAttributeError(uint8_t status, bool doRxDisable);

/************************************************************************************//**
 \brief Ends ZRC 2.0 Push Request with error.

 \param[in] status - error code.
 \param[in] doRxDisable - true if necessary to do RX Disable.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2PushAttributeError(uint8_t status, bool doRxDisable);

/************************************************************************************//**
 \brief Ends ZRC 2.0 Pull Request with error.

 \param[in] status - error code.
 \param[in] doRxDisable - true if necessary to do RX Disable.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2PullAttributeError(uint8_t status, bool doRxDisable);

/************************************************************************************//**
 \brief Helper function to extract the next attribute from Get/Pull requests responses.

 \param[in] ppData - pointer to the pointer to the array. Will be updated on exit.
 \param[in] pDataEnd - pointer to the array end.
 \param[in] ppAttribute - pointer to the pointer to the returning attribute structure.
 \return true on success otherwise false.
 ****************************************************************************************/
bool rf4cezrc2GetAttributeGetConf(uint8_t *const pDataEnd, RF4CE_ZRC2_GetAttributesConfId_t **ppAttribute);

/************************************************************************************//**
 \brief Helper function to calculate the size of the next attribute to be written to
        Get/Pull requests responses.

 \param[in] ppData - pointer to the pointer to the array. Will be updated on exit.
 \param[in] pDataEnd - pointer to the array end.
 \return size of the current attribute.
 ****************************************************************************************/
uint32_t rf4cezrc2GetAttributeGetConfGetSize(uint8_t **ppData, uint8_t *pDataEnd);

/************************************************************************************//**
 \brief Helper function to write the next attribute to Get/Pull requests responses.

 \param[in] ppData - pointer to the pointer to the array. Will be updated on exit.
 \param[in] pDataEnd - pointer to the array end.
 \param[in] ppAttribute - pointer to the pointer to the returning attribute structure.
 \return true on success otherwise false.
 ****************************************************************************************/
void rf4cezrc2GetAttributeGetConfGetAttribute(uint8_t **ppData, uint8_t *pDataEnd, RF4CE_ZRC2_GetAttributesConfId_t **ppAttribute);

/************************************************************************************//**
 \brief Helper function to increment the next attribute to Set/Push requests.

 \param[in] ppData - pointer to the pointer to the array. Will be updated on exit.
 \param[in] pDataEnd - pointer to the array end.
 \return true on success otherwise false.
 ****************************************************************************************/
bool rf4cezrc2SetAttributeIterate(RF4CE_ZRC2_SetAttributeId_t **ppData, uint8_t *pDataEnd);

/************************************************************************************//**
 \brief Gets a pointer and size of the specified simple GDP2/ZRC2 attribute

 \param[in] pair - pair reference of the requested attribute
 \param[in] attrId - attribute Id
 \param[out] attrSize - size of the attribute. If it is not needed, set to NULL

 \return pointer to the attribute
 ****************************************************************************************/
uint8_t* rf4cezrc2LocalGetAttributesSimplePointer(uint8_t pair, uint8_t attrId, uint8_t *attrSize);

/************************************************************************************//**
 \brief Saves simple ZRC 2.0 Attributes into NVM

 \param[in] pair        - which pair simple attributes to save
 ****************************************************************************************/
void rf4cezrc2SetLocalAttributesSimpleSave(uint8_t pair);

/************************************************************************************//**
 \brief Returns pointer to the specified aplMappableAction element

 \param[in] pair        - pair
 \param[in] arrayIndex  - array element index

 \return pointer RF4CE_ZRC2_MappableAction_t
 ****************************************************************************************/
RF4CE_ZRC2_MappableAction_t* rf4cezrc2LocalGetAttributesMappablePointer(const uint8_t pair, const uint16_t arrayIndex);

/************************************************************************************//**
    \brief Sets a callback to be called on Generic Response reception.
    \param[in] callback - pointer to the callback function.
 ****************************************************************************************/
void rf4cezrc2WaitForGenericResponse(Rf4cezrc2GenericResponseRxCallback_t callback);

#endif /* defined(USE_RF4CE_PROFILE_ZRC2) */

#endif // BBRF4CEZRCPRIVATEATTRIBUTES_H
