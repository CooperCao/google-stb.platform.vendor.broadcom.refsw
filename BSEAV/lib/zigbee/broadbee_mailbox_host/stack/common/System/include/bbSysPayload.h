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
 *      System Payloads interface.
 *
*******************************************************************************/

#ifndef _SYS_PAYLOAD_H
#define _SYS_PAYLOAD_H


/************************* INCLUDES *****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */

#if defined(_MEMORY_MANAGER_)
# include "bbSysMemMan.h"           /* Memory Manager interface. */
#else
# include <malloc.h>                /* Conventional dynamic memory API. */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Maximum allowed (supported) length of dynamic data.
 */
#define SYS_DYNAMIC_DATA_LENGTH_MAX  UINT16_MAX


/**//**
 * \brief Maximum allowed (supported) length of static data.
 */
#define SYS_STATIC_DATA_LENGTH_MAX  UINT8_MAX


/**//**
 * \brief Type for the length of Payload object data.
 */
#if defined(_MEMORY_MANAGER_)
typedef MM_Size_t  SYS_DataLength_t;
#else
typedef size_t     SYS_DataLength_t;
#endif


/**//**
 * \brief Structure for the descriptor of Payload object.
 */
#if defined(_MEMORY_MANAGER_)
typedef union _SYS_DataPointer_t
{
# if defined(__arc__)
    uint32_t              plain;            /*!< The plain data / 32-bit for ARC platform. */
# else
    uint64_t              plain;            /*!< The plain data / 64-bit for i386 platform. */
# endif

    struct
    {
        union
        {
            MM_ChunkId_t  block;            /*!< Memory Manager Chunk Id of the dynamic Payload object. */
# if defined(__arc__)
            uint16_t      staticDataShift;  /*!< Shift from the base of the data region to the linked static memory. */
# else
            uint32_t      staticDataShift;  /*!< Shift from the base of the data region to the linked static memory. */
# endif
        };

        uint8_t           size;             /*!< Current size of the allocated static Payload object. */

        union
        {
            uint8_t       capacity;         /*!< Predefined capacity of the linked static memory (i.e., the maximum
                                                allowed size for allocation of static Payload object). The linked static
                                                memory shall have non-ZERO capacity. */

            Bool8_t       isStatic;         /*!< ZERO if the structure describes a dynamic Payload object;
                                                non-ZERO (but not necessary equal to TRUE) if the structure
                                                describes a static Payload object. */
        };
    };

#else /* ! _MEMORY_MANAGER_ */
typedef struct _SYS_DataPointer_t
{
    uint8_t              *data;             /*!< Pointer to the linked dynamic or static memory. */

    SYS_DataLength_t      size;             /*!< Current size of the allocated dynamic or static Payload object. */

    union
    {
        SYS_DataLength_t  capacity;         /*!< Predefined capacity of the linked static memory (i.e., the maximum
                                                allowed size for allocation of static Payload object). The linked static
                                                memory shall have non-ZERO capacity. */

        SYS_DataLength_t  isStatic;         /*!< ZERO if the structure describes a dynamic Payload object;
                                                non-ZERO (but not necessary equal to TRUE) if the structure
                                                describes a static Payload object. */
    };

#endif /* ! _MEMORY_MANAGER_ */
} SYS_DataPointer_t;


/**//**
 * \brief Returns the pattern for payload descriptor initialization with empty payload at
 *  the compile-time.
 * \return  Structured image for the Payload descriptor initialization at compile-time.
 */
#if defined(_MEMORY_MANAGER_)
# define SYS_EMPTY_PAYLOAD  ((const SYS_DataPointer_t){ .plain = 0 })
#
#else
# define SYS_EMPTY_PAYLOAD  ((const SYS_DataPointer_t){ .data = NULL, .size = 0, .capacity = 0 })
#
#endif


#if defined(_MEMORY_MANAGER_)

/**//**
 * \brief The base address of the project dCCM (region of the type 'DATA').
 * \note This constant is defined by the Linker at the compile-time according to the
 *  Project settings figured out in the SVR3 linker command-file.
 */
extern uint8_t _PRJ_DCCM_BASE_[];


/**//**
 * \brief The size of the project dCCM (region of the type 'DATA').
 * \note This constant is defined by the Linker at the compile-time according to the
 *  Project settings figured out in the SVR3 linker command-file.
 */
extern uint8_t _PRJ_DCCM_SIZE_[];


/**//**
 * \brief Returns the shift to the origin of the static memory to be linked in relation to
 *  the base of dCCM by the given absolute starting address of such memory.
 * \param   address     Absolute address value, in the same address space and origin with
 *                      \c _PRJ_DCCM_BASE_.
 * \return  Shift from \c _PRJ_DCCM_BASE_ to \p address, in bytes.
 */
# define SYS_STATIC_PAYLOAD_GET_SHIFT_FROM_ADDRESS(address)                     \
        ((uint32_t)((uint8_t*)(address) - _PRJ_DCCM_BASE_))


/**//**
 * \brief Returns the absolute starting address of the linked static memory by the stored
 *  value of shift to its origin relative to the base of dCCM.
 * \param   shift       Relative address value (shift) from the \c _PRJ_DCCM_BASE_,
 *                      in bytes.
 * \return  Absolute address value, in the same address space and origin with
 *          \c _PRJ_DCCM_BASE_.
 */
# define SYS_STATIC_PAYLOAD_GET_ADDRESS_FROM_SHIFT(shift)                       \
        (_PRJ_DCCM_BASE_ + (uint32_t)(shift))


/**//**
 * \brief Returns the pattern for initialization of the descriptor of a static Payload.
 * \param   startAddress    Absolute starting address of the memory region to be linked
 *                          to static Payload object.
 * \param   capacity        Size of the memory region to be linked to static Payload
 *                          object, in bytes.
 * \return  Plain 32-bit image for the Payload descriptor to be assigned with.
 */
# if defined(__arc__)
#  define SYS_STATIC_PAYLOAD_DESCR_INIT(startAddress, capacity)                 \
        ((uint32_t)SYS_STATIC_PAYLOAD_GET_SHIFT_FROM_ADDRESS(startAddress) |    \
        ((uint32_t)capacity << 24))
#
# else /* ! __arc__ */
#  define SYS_STATIC_PAYLOAD_DESCR_INIT(startAddress, capacity)                 \
        ((uint64_t)SYS_STATIC_PAYLOAD_GET_SHIFT_FROM_ADDRESS(startAddress) |    \
        ((uint64_t)capacity << 40))
#
# endif /* ! __arc__ */

#endif /* _MEMORY_MANAGER_ */


/**//**
 * \brief Returns the pattern for descriptor linking at the compile-time to the statically
 *  allocated scalar or structured data.
 * \param   staticScalar    The name of a static variable of scalar or structured data
 *                          type (but not an array).
 * \return  Structured image for the Payload descriptor initialization at compile-time.
 * \note
 *  When need to link to a static array, use the macro \c SYS_STATIC_PAYLOAD_LINK_ARRAY().
 */
#if defined(_MEMORY_MANAGER_)
# define SYS_STATIC_PAYLOAD_LINK_SCALAR(staticScalar)                           \
        { .plain = SYS_STATIC_PAYLOAD_DESCR_INIT(&(staticScalar), sizeof(staticScalar)) }
#
#else /* ! _MEMORY_MANAGER_ */
# define SYS_STATIC_PAYLOAD_LINK_SCALAR(staticScalar)                           \
        { .data = &(staticScalar), .size = ZERO, .capacity = sizeof(staticScalar) }
#
#endif /* ! _MEMORY_MANAGER_ */


/**//**
 * \brief Returns the pattern for descriptor linking at the compile-time to the statically
 *  allocated array of bytes.
 * \param   staticArray     The name of a static variable of type <em>uint8_t[SIZE]</em>.
 * \return  Structured image for the Payload descriptor initialization at compile-time.
 * \note
 *  When need to link to a static scalar or structure, use the macro
 *  \c SYS_STATIC_PAYLOAD_LINK_SCALAR().
 */
#if defined(_MEMORY_MANAGER_)
# define SYS_STATIC_PAYLOAD_LINK_ARRAY(staticArray)                             \
        { .plain = SYS_STATIC_PAYLOAD_DESCR_INIT((staticArray), sizeof(staticArray)) }
#
#else /* ! _MEMORY_MANAGER_ */
# define SYS_STATIC_PAYLOAD_LINK_ARRAY(staticArray)                             \
        { .data = (staticArray), .size = ZERO, .capacity = sizeof(staticArray) }
#
#endif /* ! _MEMORY_MANAGER_ */

/** Macros defined below are intended to PARSE/COMPOSE payload of SYS_DataPointer_t type.  */

/**//**
 * \brief Checks the if deserialization is possible with given parameters.
 * \return True if the payload contains enough data to be deserialized.
 */
#define SYS_IS_DESERIALIZATION_POSSIBLE(payload, offset, lengthToParse) \
        (SYS_GetPayloadSize(payload) >= ((offset) + (lengthToParse)))

/**//**
 * \brief Deserializer initiation. Shall be placed first.
 * \param[in] payload - pointer to payload structure of SYS_DataPointer_t type.
 * \param[in] offset - start offset within given payload.
 * \param[in] lengthToParse - the total amount of data to be parsed.
 */
#define SYS_START_DESERIALIZATION(payload, offset, lengthToParse) \
    { \
        uint8_t deserializationBufferLength = (lengthToParse); \
        uint8_t *deserializationBuffer = ALLOCA(deserializationBufferLength); \
        uint8_t deserializationBufferOffset = 0; \
        SYS_CopyFromPayload(deserializationBuffer, (payload), (offset), deserializationBufferLength);

/**//**
 * \brief Returns deserialization buffer.
 * \return Pointer to the deserialization buffer origin.
 */
#define SYS_DESERIALIZATION_BUFFER \
        (deserializationBuffer)

/**//**
 * \brief Deserializes a value with fixed length.
 *        Shall be used only after SYS_START_DESERIALIZATION() macro is invoked.
 * \param[in] dst - pointer to the variable to be filled with parsed data.
 */
#define SYS_DESERIALIZE(dst) \
        ((void)({memcpy((dst), deserializationBuffer + deserializationBufferOffset, sizeof(*(dst))); \
          deserializationBufferOffset += sizeof(*(dst));}))

/**//**
 * \brief   Macro performing validation and deserialization of a single field.
 * \param[out]      dstField            Reference to variable where to deserialize the
 *  corresponding field of ZCL Frame Payload.
 * \param[in]       fieldSize           Size of the deserialized field according to the
 *  specification.
 * \param[in/out]   remainder           Reference to counter of reminded octets in the
 *  Frame Payload to be deserialized.
 * \param[in]       failureLabel        Label of branch with the code of the failure
 *  processor.
 */
#define SYS_DESERIALIZE_FIELD(dstField, fieldSize, remainder, failureLabel)\
        SYS_WRAPPED_BLOCK(\
            if ((remainder) < sizeof(dstField))\
                goto failureLabel;\
            SYS_DbgAssertStatic((fieldSize) == sizeof(dstField));\
            SYS_DESERIALIZE(&(dstField));\
            remainder -= sizeof(dstField);\
        )

/**//**
 * \brief Skips a specified number of bytes within deserialization buffer.
 *        Shall be used only after SYS_START_DESERIALIZATION() macro is invoked.
 * \param[in] bytesToBeSkipped - number of bytes to be skipped.
 */
#define SYS_DESERIALIZE_SKIP(bytesToBeSkipped) \
        deserializationBufferOffset += bytesToBeSkipped;

/**//**
 * \brief   Macro performing validation and skipping of specified number of bytes.
 * \param[in]       fieldSize           Size of the skipped field according to the
 *  specification.
 * \param[in/out]   remainder           Reference to counter of reminded octets in the
 *  Frame Payload to be deserialized.
 * \param[in]       failureLabel        Label of branch with the code of the failure
 *  processor.
 */
#define SYS_DESERIALIZE_SKIP_FIELD(fieldSize, remainder, failureLabel)\
        SYS_WRAPPED_BLOCK(\
            if ((remainder) < (fieldSize))\
                goto failureLabel;\
            SYS_DESERIALIZE_SKIP(fieldSize);\
            remainder -= (fieldSize);\
        )

/**//**
 * \brief Set deserialization buffer offset to the specified value
 *
 * \param[in] offset - deserialization buffer offset
 */
#define SYS_DESERIALIZE_SET_OFFSET(offset)  deserializationBufferOffset = offset;

/**//**
 * \brief Get the current address to deserialize
 */
#define SYS_DESERIALIZE_SRC() (deserializationBuffer + deserializationBufferOffset)

/**//**
 * \brief These macros are used to deserialize bits inside a serialization block
 */
#define SYS_DESERIALIZE_BITS_START()                                                                \
        {                                                                                           \
            uint8_t bitPos = 0
#define SYS_DESERIALIZE_BITS(dst, len)                                                              \
            do                                                                                      \
            {                                                                                       \
                SYS_DbgAssert((bitPos & 7) + len <= 8, SYS_PAYLOAD_DESERIALIZE_BITS_BYTE_OVERLAP);  \
                *(dst) = GET_BITFIELD_VALUE(SYS_DESERIALIZE_SRC()[bitPos >> 3], bitPos & 7, len);   \
                bitPos += len;                                                                      \
            } while ( false )
#define SYS_DESERIALIZE_BITS_FINISH()                                                               \
            SYS_DbgAssert(!(bitPos & 7), SYS_PAYLOAD_DESERIALIZE_BITS_BYTE_PADDING);                \
            SYS_DESERIALIZE_SKIP(bitPos >> 3);                                                      \
        }

/**//**
 * \brief Deserializes the specified number of bytes to the specified payload from the given offset
 *        Shall be used only after SYS_START_DESERIALIZATION() macro is invoked.
 * \param[in] payload   - pointer to the destination payload
 * \param[in] offset    - offset in the specified payload
 * \param[in] count     - number of bytes to deserialize
 * \note Payload must be allocated beforehand and its size shall be not less than the number of bytes
 *       to be deserialized plus the offset.
 */
#define SYS_DESERIALIZE_TO_PAYLOAD(payload, offset, count)                      \
        do                                                                      \
        {                                                                       \
            uint8_t *src = deserializationBuffer + deserializationBufferOffset; \
            SYS_CopyToPayload(payload, offset, src, count);                     \
            deserializationBufferOffset += count;                               \
        } while (0)

/**//**
 * \brief Checks the result offset within the deserializationBuffer.
 *        Shall be used only after all data is parsed.
 * \return True if the offset is strictly equal to the buffer length.
 */
#define SYS_IS_DESERIALIZATION_VALID() (deserializationBufferLength == deserializationBufferOffset)

/**//**
 * \brief Returns actual size to deserialize
 */
#define SYS_DESERIALIZE_REMAINS_SIZE() (deserializationBufferLength - deserializationBufferOffset)

/**//**
 * \brief Returns actual size of data which has been deserialized
 */
#define SYS_GET_DESERIALIZED_SIZE() (deserializationBufferOffset)

/**//**
 * \brief Finalizes the deserialization procedure.
 *        Shall be placed last.
 */
#define SYS_FINISH_DESERIALIZATION() \
    }

/** Macros defined below are intended to COMPOSE payload of SYS_DataPointer_t type.  */

/**//**
 * \brief Serializer initiation. Shall be placed first.
 * \param[in] lengthToCompose - the total amount of data to be composed.
 */
#define SYS_START_SERIALIZATION(lengthToCompose) \
    { \
        uint8_t serializationBufferLength = (lengthToCompose); \
        uint8_t *serializationBuffer = ALLOCA(serializationBufferLength); \
        uint8_t serializationBufferOffset = 0;

/**//**
 * \brief Serializes a value with fixed length.
 *        Shall be used only after SYS_START_SERIALIZATION() macro is invoked.
 * \param[in] src - pointer to the variable to be serialized.
 */
#define SYS_SERIALIZE(src) \
        ((void)({memcpy(serializationBuffer + serializationBufferOffset, (src), sizeof(*(src))); \
          serializationBufferOffset += sizeof(*(src));}))

/**//**
 * \brief   Macro performing validation and serialization of a single field.
 * \param[in]       srcField            Reference to variable which to serialize into the
 *  corresponding field of Frame Payload.
 * \param[in]       fieldSize           Size of the serialized field according to the
 *  specification.
 */
#define SYS_SERIALIZE_FIELD(srcField, fieldSize)\
        SYS_WRAPPED_BLOCK(\
            SYS_DbgAssertStatic((fieldSize) == sizeof(srcField));\
            SYS_SERIALIZE(&(srcField));\
        )

/**//**
 * \brief Skips a specified number of bytes within serialization buffer.
 *        Shall be used only after SYS_START_SERIALIZATION() macro is invoked.
 * \param[in] bytesToBeSkipped - number of bytes to be skipped.
 */
#define SYS_SERIALIZE_SKIP(bytesToBeSkipped) \
        serializationBufferOffset += bytesToBeSkipped;

/**//**
 * \brief Serializes specified number of array elements.
 *        Shall be used only after SYS_START_SERIALIZATION() macro is invoked.
 * \param[in] array - source array.
 * \param[in] count - number of elements to be serialized.
 */
#define SYS_SERIALIZE_ARRAY(array, count) \
        {memcpy(serializationBuffer + serializationBufferOffset, (array), (count) * sizeof((array)[0])); \
         serializationBufferOffset += (count) * sizeof((array)[0]);}

/**//**
 * \brief Actual pointer to serialize
 */
#define SYS_SERIALIZE_DST() (serializationBuffer + serializationBufferOffset)

/**//**
 * \brief These macros are used to serialize bits inside a serialization block
 */
#define SYS_SERIALIZE_BITS_START()                                                                  \
        {                                                                                           \
            uint8_t bitPos = 0
#define SYS_SERIALIZE_BITS(value, len)                                                              \
            do                                                                                      \
            {                                                                                       \
                SYS_DbgAssert((bitPos & 7) + len <= 8, SYS_PAYLOAD_SERIALIZE_BITS_BYTE_OVERLAP);    \
                SET_BITFIELD_VALUE(SYS_SERIALIZE_DST()[bitPos >> 3], bitPos & 7, len, value);       \
                bitPos += len;                                                                      \
            } while ( false )
#define SYS_SERIALIZE_BITS_FINISH()                                                                 \
            SYS_DbgAssert(!(bitPos & 7), SYS_PAYLOAD_SERIALIZE_BITS_BYTE_PADDING);                  \
            SYS_SERIALIZE_SKIP(bitPos >> 3);                                                        \
        }

/**//**
 * \brief Serializes the specified number of bytes of the specified payload from the given offset
 *        Shall be used only after SYS_START_SERIALIZATION() macro is invoked.
 * \param[in] payload   - pointer to the source payload
 * \param[in] offset    - offset in the specified payload
 * \param[in] count     - number of bytes to serialize
 */
#define SYS_SERIALIZE_FROM_PAYLOAD(payload, offset, count)                  \
        do                                                                  \
        {                                                                   \
            uint8_t *dst = serializationBuffer + serializationBufferOffset; \
            SYS_CopyFromPayload(dst, payload, offset, count);               \
            serializationBufferOffset += count;                             \
        } while (0)

/**//**
 * \brief Checks the result offset within the serializationBuffer.
 *        Shall be used only after all data is serialized.
 * \return True if the offset is strictly equal to the buffer length.
 */
#define SYS_IS_SERIALIZATION_VALID() (serializationBufferLength == serializationBufferOffset)

/**//**
 * \brief Checks the result offset within the serializationBuffer.
 *        Shall be used only after all data is serialized.
 * \return True if the offset is equal to or less than the buffer length.
 */
#define SYS_DOES_SERIALIZATION_FIT() (serializationBufferLength >= serializationBufferOffset)

/**//**
 * \brief Returns actual size of data which has been serialized
 */
#define SYS_GET_SERIALIZED_SIZE()     serializationBufferOffset

/**//**
 * \brief Finalizes the serialization procedure.
 *        Shall be placed last.
 * \param[in] payload - pointer to payload structure of SYS_DataPointer_t type.
 * \param[in] offset - start offset within given payload.
 */
#define SYS_FINISH_SERIALIZATION(payload, offset) \
        SYS_CopyToPayload((payload), (offset), serializationBuffer, serializationBufferLength); \
    }

/**//**
 * \brief Finalizes the serialization procedure using actually serialized length.
 *        Shall be placed last.
 * \param[in] payload - pointer to payload structure of SYS_DataPointer_t type.
 * \param[in] offset - start offset within given payload.
 */
#define SYS_FINISH_SERIALIZATION_EXACT(payload, offset) \
        SYS_CopyToPayload((payload), (offset), serializationBuffer, serializationBufferOffset); \
    }

#if defined(_MEMORY_MANAGER_)
/**//**
 * \brief   Structure for Fragmentation Descriptor.
 * \details This data structure holds the following:
 *  - block Ids of two inter-blocks preallocated for needs of the fragment extraction
 *      operation.
 *  - information on how the original payload was processed during a fragment extraction,
 *      and how it must be restored while the fragment is returned back.
 *
 * \note    A new fragmentation descriptor must be Constructed with
 *  \c SYS_FragDescrConstruct() function. This function may return FALSE if there was an
 *  error during descriptor construction. After the whole session is completed, the
 *  redundand fragmentation descriptor must be destructed with the
 *  \c SYS_FragDescrDestruct() function.
 * \note    Only one fragment may be extracted from a payload at a time. Prior to extract
 *  a next fragment or destruct the fragmentation descriptor, the extracted fragment must
 *  be returned back into the original payload.
 * \details The \c interBlock array holds Ids of two dynamic memory blocks that may be
 *  utilized during a fragment extraction as inter-blocks for splitting the payload. These
 *  blocks are allocated initially during construction of the fragmentation descriptor and
 *  freed during its destruction. There are two blocks because, in general, to extract a
 *  fragment the payload must be split in two points - the fragment start, and the
 *  fragment end. If during a call (the first or the second) to the Split function the
 *  corresponding inter-block was actually utilized, the block Id stored in this
 *  descriptor will be reassigned with zero (the Null Block Id); otherwise it will keep
 *  its initial value. Later, during returning the fragment into the payload, the Append
 *  function (which is also called twice, in general) checks the corresponding block Id
 *  stored value, if it equals zero or nonzero - it regulates how the fragment is joined
 *  with the payload. If after a call to Append function a block escapes, it is saved into
 *  the corresponding block Id element (a block will escape if and only if the inter-block
 *  was actually utilized during the complementary Split operation).
 * \details For the extended debug the fragmentation descriptor keeps also parameters of
 *  the last fragment extraction operation performed: chunk Id of the fragmented payload,
 *  chunk Id of the extracted fragment, and offset of the extracted fragment.
 */
typedef struct _SYS_FragDescr_t {
    MM_ChunkId_t  interBlock[2];    /*!< Array of inter-blocks utilized during fragment extraction. */
# ifdef _DEBUG_PAYLOAD_FRAG_
    MM_ChunkId_t  payload;          /*!< Keeps the chunk Id of the payload that was fragmented. */
    MM_ChunkId_t  fragment;         /*!< Keeps the chunk Id of the fragment that was extracted. */
    MM_Size_t     offset;           /*!< Keeps the offset of the extracted fragment. */
# endif
} SYS_FragDescr_t;
#endif /* _MEMORY_MANAGER_ */

/************************* FUNCTIONS PROTOTYPES *****************************************/
#ifdef SUBSTITUTE_PAYLOAD_FUNCS
typedef struct __MY_DebugType_t
{
    MM_ChunkId_t blockId;
    const char* file;
    int line;
} _MY_DebugType_t;

extern _MY_DebugType_t ____debugData[];
extern uint32_t ____iDebugData;

bool ___AddMyDebugData(SYS_DataPointer_t *payload, const char *file, int line);
bool ___RemoveMyDebugData(SYS_DataPointer_t *payload);

#define SYS_MemAlloc(payload, size) \
___SYS_MemAlloc((payload), (size)) ? ___AddMyDebugData((payload), __FILE__, __LINE__) : false
#define SYS_FreePayload(payload) \
{ \
    ___RemoveMyDebugData((payload)); \
    ___SYS_FreePayload((payload)); \
}
#if defined(_MEMORY_MANAGER_)
#define SYS_AppendPayload(dst, src) \
{ \
    ___RemoveMyDebugData((src)); \
    ___SYS_AppendPayload((dst), (src)); \
}
#define SYS_FreePayloadHead(payload, count) \
{ \
    ___RemoveMyDebugData((payload)); \
    ___SYS_FreePayloadHead((payload), (count)); \
    ___AddMyDebugData((payload), __FILE__, __LINE__); \
}
#define SYS_DuplicatePayload(dst, src) \
___SYS_DuplicatePayload((dst), (src)) ? ___AddMyDebugData((dst), __FILE__, __LINE__) : false
#define SYS_SplitPayload(tail, head, offset) \
___SYS_SplitPayload((tail), (head), (offset)) ? ___AddMyDebugData((tail), __FILE__, __LINE__) : false
#endif /* defined(_MEMORY_MANAGER_) */
#else /* RF4CE_CONTROLLER */
#define SYS_MemAlloc(payload, size) \
___SYS_MemAlloc((payload), (size))
#define SYS_FreePayload(payload) \
___SYS_FreePayload((payload))
#if defined(_MEMORY_MANAGER_)
#define SYS_AppendPayload(dst, src) \
___SYS_AppendPayload((dst), (src))
#define SYS_FreePayloadHead(payload, count) \
___SYS_FreePayloadHead((payload), (count))
#define SYS_DuplicatePayload(dst, src) \
___SYS_DuplicatePayload((dst), (src))
#define SYS_SplitPayload(tail, head, offset) \
___SYS_SplitPayload((tail), (head), (offset))
#endif /* defined(_MEMORY_MANAGER_) */
#endif /* RF4CE_CONTROLLER */

/*************************************************************************************//**
  \brief Initializes the dynamic Payload descriptor as empty.
  \param    payload     Pointer to the dynamic Payload descriptor to be initialized.
*****************************************************************************************/
SYS_PUBLIC void SYS_SetEmptyPayload(SYS_DataPointer_t *payload);


/*************************************************************************************//**
  \brief Initializes and links the static Payload descriptor.
  \param    payload         Pointer to the static Payload descriptor to be initialized.
  \param    startAddress    Pointer to the static memory region to be linked to the
                            \p payload object. Must not be NULL, and must reside in the
                            dCCM for the ARC platform with Memory Manager.
  \param    capacity        Capacity of the linked static \p memory region, in bytes. Must
                            not be ZERO, must not exceed \c SYS_STATIC_DATA_LENGTH_MAX,
                            and must fit into the dCCM for the ARC platform with Memory
                            Manager.
  \note
    The size of data stored in the \p payload object must not exceed the \p capacity.
*****************************************************************************************/
SYS_PUBLIC void SYS_LinkStaticPayload(SYS_DataPointer_t *payload,
                                      uint8_t           *startAddress,
                                      SYS_DataLength_t   capacity);


/*************************************************************************************//**
  \brief Returns the Payload size, in bytes; validates the Payload.
  \param    payload     Pointer to the dynamic or static Payload descriptor to be
                        validated and returned with  its size.
  \return   The size of the Payload object pointed by \p payload, in bytes.
  \details
    This function returns the value of \c size field of the \p payload descriptor for
    static Payloads and for dynamic Payloads in the case of using the conventional memory
    API. In the case of using the Memory Manager for dynamic Payloads the size is obtained
    from the Memory Manager internal descriptors.
  \details
    For the debug build, this function also performs general validation of the Payload
    object pointed by \p payload, so it may be used in all other functions as a
    precondition.
*****************************************************************************************/
SYS_PUBLIC SYS_DataLength_t SYS_GetPayloadSize(const SYS_DataPointer_t *payload);


/*************************************************************************************//**
  \brief Discovers whether the specified Payload is allocated; validates the Payload.
  \param    payload     Pointer to the dynamic or static Payload descriptor to be
                        validated and returned with if it is allocated or empty.
  \return   TRUE if the specified \p payload is allocated; FALSE if it is empty.
  \details
    This function takes the size of the Payload object allocated memory, and returns TRUE
    if the size is equal to ZERO.
  \details
    For the debug build, this function also performs general validation of the Payload
    object pointed by \p payload, so it may be used in all other functions as a
    precondition.
*****************************************************************************************/
SYS_PUBLIC bool SYS_CheckPayload(const SYS_DataPointer_t *payload);


/*************************************************************************************//**
  \brief Allocates dynamic or static memory of the specified size.
  \param    payload     Pointer to the empty dynamic or static Payload descriptor to be
                        assigned with the allocated memory.
  \param    size        Size of the memory region, in bytes, to be allocated. Allowed to
                        be ZERO. Must not exceed \c SYS_DYNAMIC_DATA_LENGTH_MAX or
                        \c SYS_STATIC_DATA_LENGTH_MAX according to the type of the
                        specified \p payload descriptor.
  \return   TRUE if the memory has been allocated successfully, FALSE otherwise.
  \details
    In the case of the dynamic Payload this function tries to allocate a new chunk of the
    dynamic memory of the specified \p size, and if succeeded, assigns the \p payload
    descriptor with the starting point of such memory chunk and its size and returns TRUE.
    For the ARC platform with Memory Manager the \c size is stored by the Memory Manager
    in its internal descriptors. If this function fails to allocate the specified \p size
    of bytes, it leaves the \p payload unchanged (i.e., empty) and returns FALSE.
  \details
    In the case of the static Payload this function allocates an array in the previously
    linked static memory region. The \p size of such array must not exceed the \c capacity
    of the linked static memory region; otherwise, for the debug build, this function will
    halt execution. The allocated array resides in the linked static memory region from
    its starting address, as it was specified initially during the \p payload descriptor
    linking. For the static Payloads this function returns TRUE and never returns FALSE.
  \details
    The \p payload must be initialized previously as the dynamic Payload descriptor or
    linked to the static memory. If the \p payload has then being used (i.e., its size is
    not equal to ZERO currently), it must be freed prior to allocating a new memory for
    it. For the debug build, if the \p payload is not empty this function will halt
    execution.
  \details
    The \p size may be specified as ZERO. In this case no memory is allocated, the
    \p payload is emptied (here the static Payload stays linked), and the function returns
    TRUE. Nevertheless, for the debug build, the \p payload is validated to be an empty
    Payload descriptor.
*****************************************************************************************/
SYS_PUBLIC bool ___SYS_MemAlloc(SYS_DataPointer_t *payload, SYS_DataLength_t size);


/*************************************************************************************//**
  \brief Frees the specified Payload object.
  \param    payload     Pointer to the allocated dynamic or static Payload descriptor to
                        be freed.
  \details
    In the case of the dynamic Payload this function tries to free the previously
    allocated dynamic memory (if it was allocated). After that the \p payload descriptor
    is emptied (in this case, initialized).
  \details
    In the case of the static Payload this function just resets the \c size of the
    allocated array in the linked static memory. The initially linked static memory region
    is left linked and its \c capacity is left unchanged. After that the \p payload
    descriptor is considered empty.
  \details
    The \p payload object is not allowed to be empty when calling this function. For the
    debug build, if this function is called for the empty \p payload, it will halt
    execution.
*****************************************************************************************/
SYS_PUBLIC void ___SYS_FreePayload(SYS_DataPointer_t *payload);


/*************************************************************************************//**
  \brief Copies the array of bytes from arbitrary memory region into the Payload object.
  \param    payload     Pointer to the allocated dynamic or static Payload descriptor,
                        that is used as the destination for copying.
  \param    offset      Offset from the beginning of the Payload object to the first byte
                        to be overwritten during copying, in bytes. Allowed to be ZERO.
  \param    src         Pointer to the source memory array. Must not be NULL.
  \param    count       The number of bytes to copy.
  \details
    This function copies the \p count of bytes from the \p src memory array into the
    \p payload starting with the \p offset byte inside the Payload. The \p payload must be
    initialized (for dynamic Payload) or linked (for static payload) and then allocated
    with appropriate \c size of bytes. For the debug build, if the \p payload was not
    allocated this function will halt execution.
  \details
    The \p offset must not exceed the \c size of the allocated Payload. The \p count bytes
    must fit into the remaining part of the \p payload starting from the \p offset. The
    \p count is allowed to be ZERO, but all the conditions above are still validated. For
    the debug build, if any condition is broken this function will halt execution.
  \note
    It is the responsibility of the caller to provide the \p src memory array of the
    appropriate size. The size of the array must be equal or greater than \p count bytes.
*****************************************************************************************/
SYS_PUBLIC void SYS_CopyToPayload(SYS_DataPointer_t *payload,
                                  SYS_DataLength_t   offset,
                                  const void        *src,
                                  SYS_DataLength_t   count);


/*************************************************************************************//**
  \brief Copies the array of bytes from the Payload object into arbitrary memory region.
  \param    dst         Pointer to the destination memory array. Must not be NULL.
  \param    payload     Pointer to the allocated dynamic or static Payload descriptor,
                        that is used as the source for copying.
  \param    offset      Offset from the beginning of the Payload object to the first byte
                        of its data to be copied from it, in bytes. Allowed to be ZERO.
  \param    count       The number of bytes to copy.
  \details
    This function copies the \p count of bytes from the \p payload starting with the
    \p offset byte inside the Payload into the \p dst memory array. The \p payload must be
    initialized (for dynamic Payload) or linked (for static payload) and then allocated
    with appropriate \c size of bytes. For the debug build, if the \p payload was not
    allocated this function will halt execution.
  \details
    The \p offset must not exceed the \c size of the allocated Payload. The \p count bytes
    must fit into the remaining part of the \p payload starting from the \p offset. The
    \p count is allowed to be ZERO, but all the conditions above are still validated. For
    the debug build, if any condition is broken this function will halt execution.
  \note
    It is the responsibility of the caller to provide the \p dst memory array of the
    appropriate size. The size of the array must be equal or greater than \p count bytes.
*****************************************************************************************/
SYS_PUBLIC void SYS_CopyFromPayload(void                    *dst,
                                    const SYS_DataPointer_t *payload,
                                    SYS_DataLength_t         offset,
                                    SYS_DataLength_t         count);

/*************************************************************************************//**
  \brief Copies the array of bytes from the Payload object into another Payload object.
  \param    dst         Pointer to the allocated dynamic or static Payload descriptor,
                        that is used as the destination for copying.
  \param    src         Pointer to the allocated dynamic or static Payload descriptor,
                        that is used as the source for copying.
  \param    count       The number of bytes to copy.
  \details
    This function copies the \p count of bytes from the \p src payload into the \p dst
    payload. Payloads must be initialized (for dynamic payload) or linked (for static
    payload) and then allocated with appropriate size of bytes. For the debug build, if
    a payload was not allocated this function will halt execution.
  \details
    The \p count bytes must fit into payloads' sizes. The \p count is allowed to be zero.
*****************************************************************************************/
SYS_PUBLIC void SYS_CopyPayloadToPayload(SYS_DataPointer_t       *dst,
                                         const SYS_DataPointer_t *src,
                                         SYS_DataLength_t         count);


#if defined(_MEMORY_MANAGER_)
/*************************************************************************************//**
  \brief Appends the first Payload with data from the second Payload.
  \param    dst     Pointer to the destination (the first) dynamic Payload descriptor.
  \param    src     Pointer to the source (the second) dynamic Payload descriptor.
  \note
    This function is implemented only on the ARC platform with Memory Manager. This
    function does not support static Payloads.
*****************************************************************************************/
SYS_PUBLIC void ___SYS_AppendPayload(SYS_DataPointer_t *dst, SYS_DataPointer_t *src);


/*************************************************************************************//**
  \brief Cuts out a specified number of bytes from the Payload starting from its head.
  \param    payload     Pointer to the dynamic Payload descriptor.
  \param    count       The number of bytes to cut out from the Payload head.
  \note
    This function is implemented only on the ARC platform with Memory Manager. This
    function does not support static Payloads.
*****************************************************************************************/
SYS_PUBLIC void ___SYS_FreePayloadHead(SYS_DataPointer_t *payload, SYS_DataLength_t count);


/*************************************************************************************//**
  \brief Reduces the size of Payload to the specified value cutting its tail.
  \param    payload     Pointer to the dynamic Payload descriptor.
  \param    newSize     The new size of remaining Payload, in bytes.
  \note
    This function is implemented only on the ARC platform with Memory Manager. This
    function does not support static Payloads.
*****************************************************************************************/
SYS_PUBLIC void SYS_FreePayloadTail(SYS_DataPointer_t *payload, SYS_DataLength_t newSize);


/*************************************************************************************//**
  \brief Duplicates the given Payload.
  \param    dst     Pointer to the destination empty dynamic Payload descriptor.
  \param    src     Pointer to the source (to be duplicated) dynamic Payload descriptor.
  \return   TRUE if the payload is successfully duplicated; FALSE otherwise.
  \note
    This function is implemented only on the ARC platform with Memory Manager. This
    function does not support static Payloads.
*****************************************************************************************/
SYS_PUBLIC bool ___SYS_DuplicatePayload(SYS_DataPointer_t       *dst,
                                     const SYS_DataPointer_t *src);


/*************************************************************************************//**
  \brief Detaches the tail from the Payload at the given offset.
  \param    tail    Pointer to the empty dynamic Payload descriptor for the detached tail.
  \param    head    Pointer to the source (to be split) dynamic Payload descriptor.
  \param    offset  Offset at which to split the source payload, in bytes.
  \return   TRUE if the operation succeeds; FALSE otherwise.
  \note
    This function does not support static Payloads.
*****************************************************************************************/
SYS_PUBLIC bool ___SYS_SplitPayload(SYS_DataPointer_t *tail,
                                 SYS_DataPointer_t *head,
                                 SYS_DataLength_t   offset);


/*************************************************************************************//**
  \brief Splits the linked chain of memory blocks at the given block id

  \param[in] pChain - pointer to the descriptor of the linked chain to be split.
  \param[in] pTrailer - pointer to the descriptor of the part to be detached.
*****************************************************************************************/
SYS_PUBLIC void SYS_SplitLinked(SYS_DataPointer_t *pChain, SYS_DataPointer_t *pTrailer);
#endif /* defined(_MEMORY_MANAGER_) */


/*************************************************************************************//**
  \brief    Resizes the payload size when it does not require additional blocks.
  \param[in/out]    pPayload    Pointer to descriptor of the resized payload.
  \param[in]        size        New size of the payload, in bytes.
  \details
    This function changes the payload size within small limits without changing the number
    of blocks forming this payload.
  \note
    The caller shall be assured that the operation is actually feasible. In the case of
    failure the application is halted.
*****************************************************************************************/
SYS_PUBLIC void SYS_ResizePayload(SYS_DataPointer_t *const pPayload, const SYS_DataLength_t size);


#if defined(_MEMORY_MANAGER_)
/*************************************************************************************//**
  \name Group of functions supporting fragmentation of payloads.
*****************************************************************************************/
/**@{*/

/*************************************************************************************//**
  \brief    Constructs a fragmentation descriptor.
  \param[in/out]    pFragDescr      Pointer to the fragmentation descriptor object.
  \return   TRUE if descriptor was successfully constructed; FALSE otherwise.
  \details
    This function initializes the given descriptor object, allocates the necessary amount
    of dynamic memory, links it to the descriptor, and returns TRUE if there were no
    failures. If this function failed to allocate the required dynamic memory, it returns
    FALSE and frees all the previously allocated resources.
*****************************************************************************************/
SYS_PUBLIC bool SYS_FragDescrConstruct(SYS_FragDescr_t *const pFragDescr);

/*************************************************************************************//**
  \brief    Destructs a fragmentation descriptor.
  \param[in/out]    pFragDescr      Pointer to the fragmentation descriptor object.
  \details
    This function frees all resources linked to the given descriptor.
*****************************************************************************************/
SYS_PUBLIC void SYS_FragDescrDestruct(SYS_FragDescr_t *const pFragDescr);

/*************************************************************************************//**
  \brief    Extracts a fragment from a payload.
  \param[in/out]    pPayload        Pointer to descriptor of the payload to be fragmented.
  \param[in/out]    pFragment       Pointer to the payload descriptor object to be
      assigned with the descriptor of the extracted fragmented.
  \param[in]        offset          Offset of the fragment within the payload, in bytes.
  \param[in]        len             Length of the extracted fragment, in bytes.
  \param[in/out]    pFragDescr      Pointer to the fragmentation descriptor object.
  \note
    Only one fragment may be extracted from a payload at a time. Prior to extract a next
    fragment or destruct the fragmentation descriptor, the extracted fragment must be
    returned back into the original payload.
  \note
    This function modifies the payload object. It does not copy a part from the original
    payload into a new fragment object, but namely cut out the fragment from the payload.
  \note
    When the \p offset equals zero - i.e., the fragment starts at the payload beginning -
    the \p pPayload is actually changed by this function. Later when the extracted
    fragment is returned back, this actual \p pPayload must be used for the payload being
    restored.
  \details
    This function cuts the given payload in two places (in general): after the \p offset
    bytes from its beginning, and after \p offset + \p len bytes. After that the central
    part - the required fragment - is returned with \p pFragment; while the remaining head
    and tail parts are joined together and left in the \p pPayload. When \p offset equals
    zero (the fragment starts at the beginning of the payload) the remaining head part is
    empty; when \p offset + \p len equals the given payload length (the fragment ends at
    the end of the payload), the remaining tail part is empty. When \p len equals the
    payload length (i.e., the fragment takes in the whole payload) the remaining payload
    is empty; in this case the \p pPayload will point to the empty payload after this
    function returns.
  \details
    The fragmentation descriptor object provides this function with necessary amount of
    preallocated dynamic memory, and after this function returns it keeps the information
    on how the original payload must be restored later when the extracted fragment will be
    returned back.
*****************************************************************************************/
SYS_PUBLIC void SYS_FragmentExtract(SYS_DataPointer_t *const pPayload, SYS_DataPointer_t *const pFragment,
        const SYS_DataLength_t offset, const SYS_DataLength_t len, SYS_FragDescr_t *const pFragDescr);

/*************************************************************************************//**
  \brief    Returns a fragment back into the payload from which it was previously
      extracted.
  \param[in/out]    pPayload        Pointer to descriptor of the payload to be restored.
  \param[in/out]    pFragment       Pointer to descriptor of the fragment to be returned
      back into the payload.
  \param[in]        offset          Offset of the fragment within the payload, in bytes.
  \param[in/out]    pFragDescr      Pointer to the fragmentation descriptor object.
  \note
    This function shall use the same fragmentation descriptor object that was used
    previously when the returned fragment was extracted from the payload.
  \details
    This function splits the given payload at \p offset bytes from its beginning for the
    head and tail parts. Then it joins head, fragment, and tail parts sequentially
    restoring the original payload.
  \details
    The fragmentation descriptor object provides this function with the information on how
    the original payload was processed during extraction of the fragment, and how it must
    be treated now to restore its original structure. Also this descriptor accepts back
    the auxiliary dynamic memory that might be used during fragment extraction and which
    is freed now with returning the fragment back.
*****************************************************************************************/
SYS_PUBLIC void SYS_FragmentReturn(SYS_DataPointer_t *const pPayload, SYS_DataPointer_t *const pFragment,
        const SYS_DataLength_t offset, SYS_FragDescr_t *const pFragDescr);

/*************************************************************************************//**
  \brief    Inserts a newly received fragment into the payload composed from previously
      received fragments.
  \param[in/out]    pPayload        Pointer to descriptor of the composed payload.
  \param[in/out]    pFragment       Pointer to descriptor of a newly received fragment.
  \param[in]        offset          Offset of the fragment within the payload, in bytes.
  \details
    This function splits the given payload at \p offset bytes from its beginning for the
    head and tail parts. Then it joins head, fragment, and tail parts sequentially
    composing the payload.
  \note
    The \p offset shall point on a fragment boundary between previously received and
    joined fragments. This point will also be a memory block boundary. Due to this fact,
    this function does not need additional dynamic memory for its operation.
*****************************************************************************************/
SYS_PUBLIC void SYS_FragmentInsert(SYS_DataPointer_t *const pPayload, SYS_DataPointer_t *const pFragment,
        const SYS_DataLength_t offset);

/**@}*/
#endif /* _MEMORY_MANAGER_ */


/************************* INLINES ******************************************************/
/*************************************************************************************//**
  \brief Checks whether  given pointers the same data.

  \param[in] firstPayload - the first payload descriptor.
  \param[in] secondPayload - the second payload descriptor.
  \return true, if the payload is equal.
*****************************************************************************************/
INLINE bool SYS_IsEqualPayload(SYS_DataPointer_t *firstPayload, SYS_DataPointer_t *secondPayload)
{
    SYS_DbgAssert(firstPayload, SYS_ISEQUALPAYLOAD_0);
    SYS_DbgAssert(secondPayload, SYS_ISEQUALPAYLOAD_1);
#if defined(_MEMORY_MANAGER_)
    return !memcmp(firstPayload, secondPayload, sizeof(SYS_DataPointer_t));
#else
    return firstPayload->size == secondPayload->size &&
           0 == memcmp(firstPayload->data, secondPayload->data, firstPayload->size);
#endif
}


/*
 * Repeat pragma GCC optimize because function definitions (including inlined) turn these pragrmas off automatically
 * when compiled by G++ but not GCC.
 */
#if (defined(__arm__) || defined(__i386__)) && !defined(__clang__)
# pragma GCC optimize "short-enums"     /* Implement short enums. */
# pragma GCC diagnostic ignored "-Wattributes"
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SYS_PAYLOAD_H */

/* eof bbSysPayload.h */