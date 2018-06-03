/****************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ****************************************************************************/

/**
 * @file
 * @ingroup libsyschip
 * @brief Framing structures, macros and data types for the Target Buffers version 2 protocol.
 *
 * The memory layout of a frame in memory is the one depicted below
 * (addresses increase going from left to right).
 * The first bytes are always occupied by a TB_header structure, whose size is
 * platform-dependent. The payload lives after the header, possibly preceded and
 * followed by some padding bytes. Padding is used to comply with platform specific
 * data transfers size and alignment restrictions. When a TB_trailer structure
 * is present, it will occupy the last byte(s) in the post-payload padding.
 *
 * <pre>
 * pre_padding_length  payload_length   post_padding_length
 *                / \ /              \ /    \
 * +-------------+---+---------- - - -+------+
 * |             |###|                |######|
 * |  TB_header  |###|    payload     |######|
 * |             |###|                |######|
 * +-------------+---+---------- - - -+------+
 *  \           /                           ^
 * sizeof(TB_header)                optional TB_trailer
 * </pre>
 *
 * In the figure above, # = padding data, undefined value.
 *
 * Chip-specific assumptions are made about the frame header and payload
 * location / alignment / wrap-around restrictions in local memory.
 * Instead, no assumptions should be made regarding a frame position inside a
 * "shared" circular buffer. A frame can start at any offset inside the buffer and a
 * wrap-around can occur intra-frame (even inside the TB_header structure).
 */

#ifndef _TBUF_V2_FRAMING_H_
#define _TBUF_V2_FRAMING_H_

#include "fp_sdk_config.h"


/* Note: Magnum "INLINE" macro contains "static" in its expansion
 * (WTH???) so we have to put a workaround here (sigh!). */
#if defined(__FIREPATH__)
#  include <stdbool.h>
#  include <stdint.h>

#  define TB_FUNC_ATTRS     static inline __alwaysinline
#else
/* Let's assume we are being included from libdspcontrol */
#  include "libdspcontrol/CHIP.h"
#  include "libdspcontrol/UTIL_c.h"
#  if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#    include "bstd_defs.h"
#    define TB_FUNC_ATTRS   BFPSDK_STATIC_ALWAYS_INLINE
#  else
#    include <stdbool.h>
#    include <stdint.h>
#    define TB_FUNC_ATTRS   static inline __alwaysinline
#  endif
#endif

#include "libfp/c_utils.h"

#include "libsyschip/tbuf_chips.h"
#include "libsyschip/tbuf_services.h"



#ifdef __cplusplus
extern "C" {
#endif


#if TARGET_BUFFER_VERSION == 2


/* Frame header flags */
/** The frame can be discarded if required. Depending on
 *  the TB overflow strategy, this flag could be ignored. */
#define TB_HEADER_FLAG_DISCARDABLE  (1 << 0)
/** The last sizeof(TB_trailer) bytes in the frame padding contain a TB_trailer structure */
#define TB_HEADER_FLAG_HAS_TRAILER  (1 << 1)
/** The frame payload doesn't contain data but rather information on where to find it.
 *  Specifically it consists of a single TB_indirect_frame_payload structure. */
#define TB_HEADER_FLAG_INDIRECT     (1 << 2)
/** The bits occupied by all the defined flags */
#define TB_HEADER_FLAGS_MASK        (TB_HEADER_FLAG_DISCARDABLE | \
                                     TB_HEADER_FLAG_HAS_TRAILER | \
                                     TB_HEADER_FLAG_INDIRECT)


/* Frame trailer structure */
#ifdef TB_FRAMES_HAVE_TRAILER
/** Trailer data optionally present at the end of
 *  post-payload padding. Typedef'd here to be able
 *  to augment the structure in the future. */
typedef uint8_t TB_trailer;

/** Bitmask of the trailer containing the frame chunks count. */
#define TB_TRAILER_CHUNKS_COUNT_MASK    ((uint8_t) ~(1 << 7))
/** Maximum value for the frame chunks count. */
#define TB_TRAILER_CHUNKS_COUNT_MAX     ((1 << 7) - 1)
/** The frame has been overwritten while it was transferred.
 *  Part of or all the payload could be corrupted. */
#define TB_TRAILER_OVERWRITTEN_BIT      (1 << 7)
#endif  /* TB_FRAMES_HAVE_TRAILER */



/* TB id structures and utility functions
 * --------------------------------------
 * A TB id (identifier) holds information about what service in which core of
 * which DSP subsystem crafted and sent a specific TB frame.
 *
 * Since bit fields are not portable across compilers and platforms, the following
 * workflow is necessary.
 * - On the DSP, fill in a TB_id_unzipped structure using provided struct members
 * - Zip the TB_id_unzipped structure using the TB_zip_id function. The resulting
 *   zipped id will be embedded into frames header and transmitted to the Host.
 * - At the receiver side, explode the id using TB_unzip_id to inspect the
 *   contained values.
 */

/* Raaga Magnum moans about custom data types for bitfields.
 * (C90, 6.5.2.1) "A bit-field shall have a type that is a qualified or unqualified
 *                 version of one of int, unsigned int, or signed int"
 */
#if defined(__FIREPATH__)
#  define TB_id_unzipped_field_type uint8_t
#else
#  define TB_id_unzipped_field_type unsigned
#endif

/**
 * Exploded form of a TB identifier.
 * Fields are platform-specific.
 */
typedef struct
__attribute__((packed))
{
#if defined(MCPHY)
    TB_id_unzipped_field_type dsp_subsys : 1;
    TB_id_unzipped_field_type service_id : 3;
#elif NUM_CORES == 1
    TB_id_unzipped_field_type service_id : 4;
#elif NUM_CORES > 1
    TB_id_unzipped_field_type dsp_core   : 1;
    TB_id_unzipped_field_type service_id : 3;
#endif
} TB_id_unzipped;


/** Compressed form of a TB identifier. */
typedef uint8_t TB_id_zipped;


/**
 * Converts an unzipped TB id into a zipped one.
 */
TB_FUNC_ATTRS __attribute__((unused))
TB_id_zipped TB_zip_id(TB_id_unzipped unzipped)
{
#if defined(MCPHY)
    return (unzipped.service_id & 0x07)      |
           (unzipped.dsp_subsys & 0x01) << 3;
#elif NUM_CORES == 1
    return unzipped.service_id & 0x0F;
#elif NUM_CORES > 1
    return (unzipped.service_id & 0x07)      |
           (unzipped.dsp_core   & 0x01) << 3;
#endif
}


/**
 * Converts (explodes) a zipped TB id into an unzipped one.
 */
TB_FUNC_ATTRS __attribute__((unused))
TB_id_unzipped TB_unzip_id(TB_id_zipped zipped)
{
    TB_id_unzipped ret_value;

#if defined(MCPHY)
    ret_value.service_id = zipped & 0x7;
    ret_value.dsp_subsys = (zipped >> 3) & 0x1;
#elif NUM_CORES == 1
    ret_value.service_id = zipped & 0xF;
#elif NUM_CORES > 1
    ret_value.service_id = zipped & 0x7;
    ret_value.dsp_core   = (zipped >> 3) & 0x1;
#endif

    return ret_value;
}


/**
 * Checks if a zipped TB id is valid in the current platform.
 *
 * @return true if the id look plausible, false otherwise.
 */
TB_FUNC_ATTRS __attribute__((unused))
bool TB_is_id_valid(TB_id_zipped zipped_id)
{
#if NUM_CORES == 1
    return (zipped_id & ~0x07) == 0;
#else
    return (zipped_id & ~0x0F) == 0;
#endif
}


#ifdef __FIREPATH__

/**
 * Creates a zipped TB id for the specified service.
 * Takes care of fetching other required coordinates (DSP subsystem,
 * DSP core, etc.) and store them in the returned structure.
 *
 * @param service   one of the services identifiers defined in tbuf_services.h
 */
TB_id_zipped TB_create_id_for_service(TB_service_id service);
#endif  /* __FIREPATH__ */



/* Frame header prologue structure and manipulation macros
 * -------------------------------------------------------
 * In most platforms the id and flags fields of a TB header can fit in a single
 * byte, in some other platforms more bytes are required. For this reason the
 * "beginning" of a TB frame header is encapsulated into the TB_prologue abstract
 * data type. To access this TB_header field use the provided TB_PROLOGUE_* macros.
 */
typedef
#if defined(CELIVERO) || defined(CELTRIX) || defined(RAAGA) || defined(DUNA) || \
    defined(MCPHY) || defined(WHITNEY) || defined(PIKE) ||                      \
    defined(BSP) || defined(LEAP_PHY) ||                                        \
    defined(GENERIC) || defined(__COMPILE_HEADER__)
uint8_t
#else
#  error "TB_prologue definition unknown for this platform"
/*
//struct
//__attribute__((packed))
//{
//    uint8_t  flags;
//    uint8_t  id;
//}
*/
#endif
TB_prologue;


#if   defined(CELIVERO) || defined(CELTRIX) || defined(RAAGA) || defined(DUNA) || \
      defined(MCPHY) || defined(WHITNEY) || defined(PIKE) ||                      \
      defined(BSP) || defined(LEAP_PHY) ||                                        \
      defined(GENERIC) || defined(__COMPILE_HEADER__)
#  define TB_PROLOGUE_INIT(prologue)                (prologue) = 0
#  define TB_PROLOGUE_GET_ID(prologue)              (((prologue) & 0xF0) >> 4)
#  define TB_PROLOGUE_SET_ID(prologue, _id)         (prologue) = (((prologue) & 0x0F) | (((_id) & 0x0F) << 4))
#  define TB_PROLOGUE_GET_FLAGS(prologue)           ((prologue) & 0x0F)
#  define TB_PROLOGUE_SET_FLAGS(prologue, _flags)   (prologue) = (((prologue) & 0xF0) | ((_flags) & 0x0F))
#elif !defined(__COMPILE_HEADER__)
#  error "TB_PROLOGUE_* macros not yet defined for this platform"
/*
//#  define TB_PROLOGUE_INIT(prologue)                (prologue) = 0
//#  define TB_PROLOGUE_GET_ID(prologue)              ((prologue).id)
//#  define TB_PROLOGUE_SET_ID(prologue, _id)         (prologue).id = (_id)
//#  define TB_PROLOGUE_GET_FLAGS(prologue)           ((prologue).flags)
//#  define TB_PROLOGUE_SET_FLAGS(prologue, _flags)   (prologue).flags = (_flags)
*/
#endif



/* Frame header structure */
/**
 * Structure found at the beginning of each TB frame,
 * contains information about frame data layout.
 */
typedef struct
__attribute__((packed))
{
    TB_prologue prologue;               /**< platform-specific container for source address, service id and flags */
    uint8_t     pre_padding_length;     /**< number of padding bytes between the header end and the beginning */
                                        /**< of payload; it will always be 0 for a buffered TB */
    uint32_t    payload_length;         /**< number of payload data bytes */
    uint8_t     post_padding_length;    /**< number of padding bytes after the payload end */
} TB_header;


/* Special cases payload structure */
/**
 * Payload structure in case of an indirect frame.
 */
typedef struct
__attribute__((packed))
{
    uint32_t    address;                /**< address where the data is located, expressed in terms of the DSP address space */
    uint32_t    length;                 /**< data length in bytes */
} TB_indirect_frame_payload;


#endif  /* TARGET_BUFFER_VERSION == 2 */

#ifdef __cplusplus
}
#endif


/* Don't propagate */
#undef TB_FUNC_ATTRS

#endif /* _TBUF_V2_FRAMING_H_ */
