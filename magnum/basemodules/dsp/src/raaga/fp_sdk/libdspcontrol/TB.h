/****************************************************************************
 *                Copyright (c) 2013 Broadcom Corporation                   *
 *                                                                          *
 *      This material is the confidential trade secret and proprietary      *
 *      information of Broadcom Corporation. It may not be reproduced,      *
 *      used, sold or transferred to any third party without the prior      *
 *      written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                          *
 ****************************************************************************/

/**
 * @file
 * @ingroup libdspcontrol
 * @brief Implementation of the consumer side for Target Buffers.
 *
 * A Target Buffer represents a channel from the DSP to the Host through
 * which various types of services data can be transferred; notable examples
 * are prints, core dumps, profiling or instrumentation data.
 * A Target Buffer (shortly, a TB) can be imagined as an unidirectional FIFO.
 * Up to version 1 data was transmitted unframed and so each stream required
 * a separate TB instance. From version 2 onward data is framed and so multiple
 * streams can be multiplexed over a single TB instance.
 *
 * On most systems the Target Buffer module makes use of a shared circular
 * buffer and a shared control structure represented by the TB_shared type
 * (documented in tbuf.h). The level of visibility and the means of access to
 * the shared storage, both from the DSP and the Host point of view, vary from
 * platform to platform. Also the meaning of the storage pointer (to which
 * address space it belongs, for example) is platform-dependent. The Host
 * implementation of the Target Buffer mechanism has a read-only vision of the
 * buffer.
 *
 * A Target Buffer can be set up in memory through the TB_init function;
 * if another actor has already initialised the control structure, use the
 * TB_attach function instead. To release allocated resources, call TB_close.
 * The amount of data available on a specific Target Buffer can be checked via
 * TB_availableData and read through TB_readCircular. The TB_dumpToFile utility
 * function transfers the content of a Target Buffer to a file.\n
 * Another set of function permits to peek at data in the Target Buffer without
 * first extracting it. The TB_peek function obtains a TB_data_descriptor
 * descriptor; a read cursor can be moved inside this descriptor with TB_seek
 * and data can be read with TB_read. Once finished, Target Buffer space can
 * be freed using the TB_discard function.\n
 * If Target Buffer version 2 is in use, all the previous functions are still
 * usable to manipulate data as a raw stream of bytes. However, a new set of
 * functions is made available which understands frames and payload layout.
 * Once a TB_data_descriptor is obtained on the available Target Buffer data
 * using TB_peek, the TB_countFrames function can retrieve how many frames and
 * how much payload is present. The TB_peekFrames function can then be used to
 * analyse frames headers in more detail and finally TB_readAllFrames can read
 * frames payload. All these functions accept a filter function argument, so
 * that a pre-selection of frames can be operated. Another strategy is to invoke
 * the TB_readSingleFrames function, which iterates through frames and feeds a
 * callback function all the found frames.
 *
 * As said before, most systems implement the Target Buffer mechanism by means
 * of a shared circular buffer and a shared control structure. A minority of
 * other platforms use custom means to transfer data from the DSP to the Host
 * processor address space. In these systems, all the functions that deal with
 * data in circular buffers are not available. A TB_data_descriptor can be
 * obtained here using the TB_attachToBuffers functions and should be released
 * using TB_detachFromBuffers. The TB_seek and TB_read functions can be employed
 * as usual, as well as TB_countFrames, TB_peekFrames, TB_readAllFrames and
 * TB_readSingleFrames for version 2 implementations.
 *
 * The following table provides an overview of the available API:
 @verbatim
            | TB with circular buffer | TB with custom data transfer
 -----------+-------------------------+-----------------------------
            |  TB_init                |
            |  TB_attach              |
            |  TB_close               |
   Raw      |  TB_getStorage          |
   data     |  TB_size                |
  access    |  TB_availableData       |
            |  TB_discard             |
            |  TB_readCircular        |
            |  TB_dumpToFile          |
 -----------+-------------------------+-----------------------------
  Access    |  TB_peek                |  TB_attachToBuffers
 through a  |  TB_seek                |  TB_detachFromBuffers
 descriptor |  TB_read                |  TB_peek
            |                         |  TB_seek
            |                         |  TB_read
 -----------+-------------------------+-----------------------------
  Framed    |                  TB_countFrames
   data     |                  TB_peekFrames
  access    |                  TB_readAllFrames
            |                  TB_readSingleFrames
 -----------+-------------------------------------------------------
 @endverbatim
 */


#ifndef _TB_H_
#define _TB_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "fp_sdk_config.h"

#include "libdspcontrol/CHIP.h"

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stdbool.h>
#  include <stddef.h>
#  include <sys/types.h>
#else
#  include "bstd_defs.h"
#endif

#if !IS_HOST(DSP_LESS)
#  include "libdspcontrol/DSP.h"
#endif

#include "libsyschip/tbuf_chips.h"
#include "libsyschip/tbuf_services.h"


#if !HAS_FEATURE(TB_VARIANT)
#  error "A Target Buffer variant must be selected in CHIP.h"
#endif
#if FEATURE_IS(TB_VARIANT, CIRCULAR) && !defined(TB_HAS_SHARED_STRUCT)
#  error "The circular TB variant is intended only for chips with TB_shared structure support"
#endif
#if IS_HOST(DSP_LESS) && FEATURE_IS(TB_VARIANT, CIRCULAR)
#  error "The TB module can't be build DSP-less with the circular TB variant"
#endif


#ifdef __cplusplus
extern "C"
{
#endif


#if FEATURE_IS(TB_VARIANT, CIRCULAR)

/**
 * Value to pass to TB_init / TB_attach if no TB naming is requested.
 */
#define TB_NO_NAME  NULL


/**
 * TB support structure.
 */
typedef struct
{
    DSP *dsp;                               /**< DSP subsystem object used to access memories */
    uint32_t tb_addr;                       /**< location of the TB_shared structure */
    DSP_ADDRESS_SPACE tb_addr_space;        /**< address space the tb_addr field refers to */
    DSP_ADDRESS_SPACE buffer_addr_space;    /**< address space the storage, read_ptr and write_ptr fields in tb_addr refer to */
    uint32_t buffer_size;                   /**< storage size, cached value of @code ((TB_shared *) tb_addr)->storage_kbytes * 1024 @endcode */
    char *display_id;                       /**< identifier shown to users, never NULL */
} TB;

#endif  /* FEATURE_IS(TB_VARIANT, CIRCULAR) */


#if !FEATURE_IS(TB_VARIANT, NONE)

/**
 * Structure describing a contiguous block of memory.
 */
typedef struct
{
#if IS_HOST(DSP_LESS)
    uint8_t *data;      /**< pointer to data */
#else
    uint32_t data;      /**< pointer to data, meaning is system dependent (see TB_data_descriptor) */
#endif
    size_t   size;      /**< size of the pointed data */
} TB_data_region;


/**
 * Structure modelling data as a ordered list of buffers. These buffers could
 * be the representation of available data in a circular TB buffer or having
 * been provided from the user. The structure also models a logical read cursor
 * inside buffers, so that read/seek operations are possible.
 *
 * In linear TB flavours this structure must be initialised with TB_attachToBuffers
 * and de-initialised with TB_detachFromBuffers. In circular TB flavours the
 * structure should be initialised with TB_peek and it doesn't need to be
 * de-initialised; it remains valid until any function that frees TB space
 * (as TB_readCircular or TB_discard) is invoked.
 */
typedef struct
{
#if FEATURE_IS(TB_VARIANT, CIRCULAR)
    TB_data_region    buffers[2];               /**< 2 buffers are enough to cover the data wrapping-around case */
#endif
#if FEATURE_IS(TB_VARIANT, MULTIBUFFER)
    TB_data_region   *buffers;                  /**< array of buffers, user provided */
#endif
    unsigned          buffers_count;            /**< number of valid items in @p buffers, 0 is not a valid value */

#if !IS_HOST(DSP_LESS)
    DSP              *dsp;                      /**< the DSP subsystem used to access memories */
    DSP_ADDRESS_SPACE buffers_address_space;    /**< what address space the @p data fields in @p buffers elements refer to */
#endif

    size_t            total_size;       /**< sum of the data sizes in @p buffers */
    size_t            cur_offset;       /**< current read cursor, range [0, total_size] */
    unsigned          cur_buffer;       /**< index of the buffer where @p cur_pointer lives */

#if !IS_HOST(DSP_LESS)
    uint32_t          cur_pointer;
#else
    uint8_t          *cur_pointer;
#endif
                                        /**< the data pointer, belonging to one of the buffers, representing cur_offset; */
                                        /**< if cur_offset == total_size, cur_pointer points to the byte after the
                                             end of the last buffer */
} TB_data_descriptor;


/**
 * Whence parameter for the TB_seek function.
 */
typedef enum
{
    TB_SEEK_BEGIN,  /**< position 0 (the beginning of the first buffer) */
    TB_SEEK_CUR,    /**< current position */
    TB_SEEK_END     /**< end of buffers position */
} TB_seek_whence;

#endif  /* !FEATURE_IS(TB_VARIANT, NONE) */


#if FEATURE_IS(TB_VARIANT, CIRCULAR)

/**
 * Initialises a Target Buffer structure.
 * This function must be called only if the Host is responsible to set up the TB_shared
 * structure; it is not needed if the Target Buffer has already been set up (use TB_attach instead).
 *
 * @param[out] ret_value       the TB structure identifying this Target Buffer to be filled in
 * @param[in]  dsp             the DSP object associated with this TB, used to access memories
 * @param[in]  tb_addr         address of the TB_shared control structure in memory
 * @param[in]  tb_addr_space   address space tb_addr refers to
 * @param[in]  buff_addr       address of the shared storage
 * @param[in]  buff_addr_space address space buff_addr refers to
 * @param[in]  buf_len_KiB     length of storage in kibibytes
 * @param[in]  name            the name to assign to this TB, use TB_NO_NAME if no name is requested
 */
__attribute__((nonnull(1,2)))
void TB_init(TB *ret_value,
             DSP *dsp,
             uint32_t tb_addr,
             DSP_ADDRESS_SPACE tb_addr_space,
             uint32_t buff_addr,
             DSP_ADDRESS_SPACE buff_addr_space,
             uint16_t buff_len_KiB,
             const char *name);


/**
 * Attaches to an existing Target Buffer structure.
 * This function should be used if the Target Buffer TB_shared structure has already been set up
 * by another actor; if the shared structure needs to be initialised, use TB_init.
 *
 * @param[out] ret_value       the TB structure identifying this Target Buffer to be filled in
 * @param[in]  dsp             the DSP object associated with this TB, used to access memories
 * @param[in]  tb_addr         address of the TB_shared control structure in memory
 * @param[in]  tb_addr_space   address space tb_addr refers to
 * @param[in]  buff_addr_space address space the storage, read_ptr and write_ptr fields in tb_addr refers to
 * @param[in]  name            the name to assign to this TB, use TB_NO_NAME if no name is requested
 */
__attribute__((nonnull(1,2)))
void TB_attach(TB *ret_value,
               DSP *dsp,
               uint32_t tb_addr,
               DSP_ADDRESS_SPACE tb_addr_space,
               DSP_ADDRESS_SPACE buff_addr_space,
               const char *name);


/**
 * Stop using a Target Buffer.
 * After this call the passed TB structure shouldn't be used anymore.
 *
 * @param[in] tb   the TB structure to close
 */
__attribute__((nonnull))
void TB_close(TB *tb);


/**
 * Retrieves the Target Buffer storage location in memory.
 *
 * @param[in]  tb              the TB structure to access
 * @param[out] buff_addr       storage address; pass NULL if not interested
 * @param[out] buff_addr_space storage (buff_addr) address space; pass NULL if not interested
 */
__attribute__((nonnull(1)))
void TB_getStorage(TB *tb, uint32_t *buff_addr, DSP_ADDRESS_SPACE *buff_addr_space);


/**
 * Obtain the size of a Target Buffer shared storage.
 * No more than this amount of data will ever be read in a single TB_readCircular
 * or TB_read call.
 *
 * @param[in] tb the TB structure to access
 * @return       total storage available in the buffer in bytes
 */
__attribute__((nonnull))
size_t TB_size(TB *tb);


/**
 * Checks how much data is a available for reading.
 * A subsequent call to TB_readCircular will return at least this amount of data.
 *
 * @param[in] tb the TB structure to access
 * @return       minimum number of bytes available to read, 0 if no data
 */
__attribute__((nonnull))
size_t TB_availableData(TB *tb);


/**
 * Discards some of the data available for reading.
 *
 * @param[in] tb     the TB structure to access
 * @param[in] amount this amount of bytes will be discarded (or less if not enough data is available)
 * @return           actual number of discarded bytes
 */
__attribute__((nonnull))
size_t TB_discard(TB *tb, size_t amount);


/**
 * Fetch data from a given Target Buffer.
 * Reads as much data as is currently available or until the provided buffer is full.
 *
 * @param[in] tb                 the TB structure to access
 * @param[in] buff               buffer for returned data
 * @param[in] size               maximum number of bytes to read
 * @param[in] discard_after_read after reading, discard read bytes from the shared buffer, freeing space
 * @return                       number of read bytes
 */
__attribute__((nonnull))
size_t TB_readCircular(TB *tb, void *buff, size_t size, bool discard_after_read);


#if FEATURE_IS(FILE_IO, AVAILABLE)

/**
 * Utility function to extract data from a Target Buffer and dump it to a file.
 * It tries to read as much data as it is available from the TB.
 * Note: uses dynamic memory (malloc & free) to allocate a temporary proper sized buffer.
 *
 * @param[in] tb the TB structure to access
 * @param[in] fd file descriptor to append data to
 * @return       number of bytes transferred, -1 if write fails
 */
__attribute__((nonnull))
ssize_t TB_dumpToFile(TB *tb, int fd);

#endif


/**
 * Analyses the Target Buffer status and fills in a TB_data_descriptor
 * describing available data and its layout in memory. Such data can then
 * be manipulated using the TB_seek and TB_read functions; to free some
 * shared buffer space, use the TB_discard function.
 *
 * Upon returning from this function, the data_descriptor read cursor is
 * positioned at offset 0 (first available byte to read).
 *
 * @param[in]  tb              the TB structure to access
 * @param[out] data_descriptor out descriptor where to store data information
 * @return                     number of bytes available to read
 */
__attribute__((nonnull))
size_t TB_peek(TB *tb, TB_data_descriptor *data_descriptor);

#endif  /* FEATURE_IS(TB_VARIANT, CIRCULAR) */


#if FEATURE_IS(TB_VARIANT, MULTIBUFFER)

/**
 * Initialises a TB_data_descriptor instance based on the provided
 * buffers containing Target Buffer data to be read. Buffers should not
 * be freed nor their content modified until a call to TB_detachFromBuffers
 * is performed.
 *
 * Upon returning from this function, the data_descriptor read cursor is
 * positioned at offset 0 (first available byte to read).
 *
 * @param[out] ret_value             the TB_data_descriptor structure to be initialised
 * @param[in]  buffers               an array of buffer descriptors representing data to be read
 * @param[in]  buffers_count         number of buffers in the buffers array, must be greater than 0
 * @param[in]  dsp                   the DSP object used to access memories
 * @param[in]  buffers_address_space which address space the buffers 'data' field refer to
 * @return     data_descriptor itself, NULL in case of error
 */
__attribute__((nonnull, warn_unused_result))
TB_data_descriptor *TB_attachToBuffers(TB_data_descriptor *data_descriptor,
                                       TB_data_region *buffers,
                                       unsigned buffers_count
#if !IS_HOST(DSP_LESS)

                                     , DSP *dsp
                                     , DSP_ADDRESS_SPACE buffers_address_space
#endif
                                       );


/**
 * De-initialises a TB_data_descriptor instance. Don't use the descriptor after
 * calling this function.
 *
 * @param[out] descriptor the descriptor to finalise
 */
__attribute__((nonnull))
void TB_detachFromBuffers(TB_data_descriptor *data_descriptor);

#endif  /* FEATURE_IS(TB_VARIANT, MULTIBUFFER) */


#if !FEATURE_IS(TB_VARIANT, NONE)

/**
 * Reads data from the buffers described by the TB_data_descriptor
 * argument. The function starts reading from the descriptor current
 * read cursor position; size bytes (or less if not enough data is
 * available) are copied into dest. The read cursor is moved forward
 * only if requested.
 *
 * @param[in] descriptor  the source data descriptor
 * @param[in] dest        destination buffer where to copy data
 * @param[in] size        this amount of bytes will be copied (or less if not enough data is available)
 * @param[in] move_cursor whether to move forward the read cursor after the read
 * @return                the number of bytes actually read
 */
__attribute__((nonnull(1)))
size_t TB_read(TB_data_descriptor *descriptor, void *dest, size_t size, bool move_cursor);


/**
 * Conceptually similar to the lseek syscall, moves the read cursor
 * position inside the buffers space described by the TB_data_descriptor
 * argument.
 *
 * @param[in] descriptor the data descriptor
 * @param[in] offset     number of bytes to move the read pointer relative to whence
 * @param[in] whence     starting point for the move (current position, buffers start, buffers end)
 * @return               the new read cursor position
 */
__attribute__((nonnull(1)))
size_t TB_seek(TB_data_descriptor *descriptor, int offset, TB_seek_whence whence);


#if TARGET_BUFFER_VERSION == 2

#include "libsyschip/tbuf_v2_framing.h"


/**
 * Information contained in a frame trailer.
 */
typedef struct
{
    bool    overwritten : 1;    /**< true if the frame has been overwritten while it was */
                                /**< transferred or scheduled for transfer; in this case the frame
                                     payload could be corrupted */
    uint8_t chunks_count;       /**< number of data chunks contained in the payload */
} TB_trailer_info;


/**
 * Information about an extracted frame.
 */
typedef struct
{
    TB_id_unzipped  tb_id;                  /**< TB identifier */
    bool            discardable : 1;        /**< if the frame was marked as discardable or not */
    bool            has_trailer_info : 1;   /**< true if trailer_info is valid (the frame contained a trailer) */
    TB_trailer_info trailer_info;           /**< information extracted from the frame trailer,
                                                 valid only if has_trailer_info is true */
    size_t          payload_offset;         /**< payload data read position inside a source TB_data_descriptor */
    uint8_t        *payload_address;        /**< address of extracted payload data; valid only when copying frames
                                                 payload to a different buffer (see TB_readAllFrames), NULL otherwise */
    uint32_t        payload_length;         /**< length of payload data */
} TB_frame_info;



/**
 * Configuration structure for the TB_services_filter frames filter.
 * A properly filled instance of this structure must be passed as
 * 'filter data' when using TB_services_filter.
 */
typedef struct
{
    TB_service_flag accepted_services;          /**< bitmask of accepted services */
    bool            accept_overwritten_frames;  /**< whether to accepts frames with potentially corrupted data */
} TB_services_filter_config;


/**
 * Prototype of the callback function passed to TB_readSingleFrames.
 * At each invocation, @p frame_info contains information about the current
 * frame, @p descriptor_at_payload a TB_data_descriptor that can be used
 * (with TB_read) to read payload data, if desired. Note that
 * @p descriptor_at_payload is a local descriptor and any change to it
 * will not be reflected to the descriptor passed to TB_readSingleFrames,
 * so it's safe to manipulate it. The @p user_data parameter is provided by
 * the user and its meaning is callback-dependent.
 * The function should always return true; if false is returned, the frames
 * iteration will be stopped.
 */
typedef bool (*TB_frame_reader)(TB_frame_info *frame_info,
                                TB_data_descriptor *descriptor_at_payload,
                                void *user_data);


/**
 * Frame filter function prototype. Return true to accept the frame
 * described by @p frame_info, false to reject it.
 * The @p user_data parameter is provided by the user and its meaning is
 * filter-dependent.
 */
typedef bool (*TB_frame_filter)(TB_frame_info *frame_info, void *user_data);


/**
 * Built-in frame filter that accept frames based on the service data
 * in their payload. A properly set TB_services_filter_config structure
 * must be passed as data.
 */
bool TB_services_filter(TB_frame_info *frame_info, void *data);


/**
 * Calculates how many frames and how much payload is available and what is the
 * biggest payload in the buffers described by the TB_data_descriptor. The
 * function starts reading from the descriptor current read cursor position and
 * limits its search to @p max_frames frames. The descriptor read cursor position
 * is preserved. If a filtering function is provided, only frames not filtered
 * out are taken into account.
 *
 * Note: the total available payload amount could be 0 even if some frames are
 *       present, as frames with no payload are permitted.
 *
 * @param[in]  descriptor        descriptor of the buffers where to look for frames data
 * @param[in]  max_frames        limit the search to this number of frames; this number applies to the
 *                               number of frames after the filtering function has been applied
 * @param[out] frames_count      number of found frames
 * @param[out] available_payload number of payload bytes available to read
 * @param[out] max_payload_size  biggest payload size among the analysed frames
 * @param[in]  filter            filter function, NULL if no filtering is requested; only frames
 *                               accepted by the filter function are taken into account
 * @param[in]  filter_data       data passed to the filter function
 * @return                       read cursor position in descriptor after the last evaluated frame
 *                               (filtered out or not) by this function
 */
__attribute__((nonnull(1, 3, 4)))
size_t TB_countFrames(TB_data_descriptor *descriptor,
                      unsigned max_frames,
                      unsigned *frames_count,
                      size_t *available_payload,
                      size_t *max_payload_size,
                      TB_frame_filter filter,
                      void *filter_data);


/**
 * Retrieves frames information and stores them in the frame_info array.
 * The function starts reading from the descriptor current read cursor position
 * and limit its search to num_frame_info frames. The descriptor read cursor
 * position is preserved. If a filtering function is provided, only frames not
 * filtered out are taken into account.
 * The number of frames stored into frame_info and the size required to hold their
 * payload are returned as well by this function.
 *
 * Note: the returned payload amounts could be 0 for some frames, as frames
 *       with no payload are permitted.
 *
 * @param[in]  descriptor        descriptor of the buffers where to look for frames data
 * @param[out] frame_info        array of TB_frame_info where to store read frames information
 * @param[in]  num_frame_info    limit the search to this number of frames; this number applies to the
 *                               number of frames after the filtering function has been applied
 * @param[out] frames_count      number of frames stored into frame_info
 * @param[out] available_payload total number of payload bytes in the frame_info frames
 * @param[in]  filter            filter function, NULL if no filtering is requested; only frames
 *                               accepted by the filter function are taken into account
 * @param[in]  filter_data       data passed to the filter function
 * @return                       read cursor position in descriptor after the last evaluated frame
 *                               (filtered out or not) by this function
 */
__attribute__((nonnull(1, 2, 4, 5)))
size_t TB_peekFrames(TB_data_descriptor *descriptor,
                     TB_frame_info *frame_info,
                     unsigned num_frame_info,
                     unsigned *frames_count,
                     size_t *available_payload,
                     TB_frame_filter filter,
                     void *filter_data);


/**
 * Reads as much payload data as will fit into the provided buffer; at the same
 * time, it retrieves information about the extracted frames and stores them in
 * the frame_info array. The function starts reading from the descriptor current
 * read cursor position and limit its search to num_frame_info frames. The
 * descriptor read cursor position is preserved. If a filtering function is
 * provided, only frames not filtered out are taken into account. The number of
 * frames stored into frame_info and the actual amount of retrieved payload are
 * returned as well by this function.
 *
 * Note: the returned payload amount could be 0 even if some frames are
 *       present, as frames with no payload are permitted.
 *
 * @param[in]  descriptor        descriptor of the buffers where to look for frames data
 * @param[out] frame_info        array of TB_frame_info where to store read frames information
 * @param[in]  num_frame_info    limit the search to this number of frames; this number applies to the
 *                               number of frames after the filtering function has been applied
 * @param[out] frames_count      number of frames stored into frame_info
 * @param[out] buf               buffer where to store payload data
 * @param[in]  buf_size          maximum number of bytes the buffer can contain
 * @param[out] available_payload total number of payload bytes in the frame_info frames
 * @param[in]  filter            filter function, NULL if no filtering is requested; only frames
 *                               accepted by the filter function are taken into account
 * @param[in]  filter_data       data passed to the filter function
 * @return                       read cursor position in descriptor after the last evaluated frame
 *                               (filtered out or not) by this function
 */
__attribute__((nonnull(1, 2, 4, 5)))
size_t TB_readAllFrames(TB_data_descriptor *descriptor,
                        TB_frame_info *frame_info,
                        unsigned num_frame_info,
                        unsigned *frames_count,
                        void *buf,
                        size_t buf_size,
                        size_t *available_payload,
                        TB_frame_filter filter,
                        void *filter_data);


/**
 * Iterates through available frames and feeds them to the provided callback.
 * The function starts reading from the descriptor current read cursor position
 * and stops when no more frames are available or the callback function returns
 * false.
 *
 * Note: the provided payload amount could be 0 when invoking the callback for
 *       some frames, as frames with no payload are permitted.
 *
 * @param[in]  descriptor        descriptor of the buffers where to look for frames data
 * @param[in]  frames_reader     callback function that will be invoked for each found frame
 * @param[in]  reader_data       data passed to the callback function
 * @return                       read cursor position in descriptor after the last frame for
 *                               which the frames_reader callback returned true
 */
__attribute__((nonnull(1, 2)))
size_t TB_readSingleFrames(TB_data_descriptor *descriptor,
                           TB_frame_reader frames_reader,
                           void *reader_data);


#endif  /* TARGET_BUFFER_VERSION == 2 */
#endif  /* !FEATURE_IS(TB_VARIANT, NONE) */

#ifdef __cplusplus
}
#endif


#endif /* _TB_H_ */
