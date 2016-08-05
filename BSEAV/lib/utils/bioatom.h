/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 *******************************************************************************/
#ifndef __BATOM_H__
#define __BATOM_H__

#include "balloc.h"
#include "bpool.h"
#ifdef __cplusplus
extern "C"
{
#endif

/*================== Module Overview =====================================
This module defines basic elements which are used to build stackable stream
parsers or processors. These processors obey the following rules:
 1. In-place parsing (e.g. zero memory copy parsing), where stream is separated in pieces and rejoined into a new stream format
 2. Symmetric input and outputs
 3. Asynchronous non-blocking model of operation

First rule is the key for high performance, actual data is not copied, instead all parsers and processors
operate with the same data, thus significantly reducing memory traffic and increasing performance.
Second rule allows arbitrary connection between processors.
Third rule provides flexibility in partitioning the system, since neither of calls block, arbitrary complex
system could be build in a single execution context, i.e. it doesn't require separate thread for each
processing element

Below is a sample interconnection scheme for the network of system processors.

                                                              +----------+
+---------+          +---------+          +--------)->[PIPE]->)          |          +-------------+
|  Source )->[PIPE]->( Filter1 )->[PIPE]->( Parser |          | Filter2  )->[PIPE]->( Destination |
+---------+          +---------+          +--------)->[PIPE}->)          |          +-------------+
                                                              +----------+

Each stream processor is independent module, that has one or more inputs and one or more outputs. Each input
is a pipe, the software abstraction which is used to accumulated data elements.

This modules provides the following basic elements:
  o batom_t - basic data element, in corresponds to single data element, it could be single network packet, single video or audio packet.
  o batom_pipe_t - FIFO of basics data elements
  o batom_cursor - means of representing sequence of memory blocks as continuous sequence of bytes
  o batom_accum_t - instrument to merge and slice batom_t

========================================================================*/


/**
Summary:
 batom_t is a basic element of the stream processing
Description:
 batom_t is immutable representation of one or more regions in the memory (called atom payload). There are operations that could be used to split batom_t into
 multiple batom_t, data could be appended into the batom_t or removed. However because batom_t is immutable in order to modify batom_t users
 create new batom_t which shares all or some payload of source batom_t. Each batom_t is a reference counted object and when batom_t payload is used to create new batom_t, its reference
 counter incremented. When b_atom_t first created it's reference counter is set to 1, batom_lock increments reference counter and batom_release decrements reference counter.
 When reference counter goes through zero, then batom_t is recycled, and if it uses payload from other atoms their reference counter recursively decremented.
**/
typedef struct batom *batom_t;

/**
Summary:
 batom_pipe_t is FIFO of batom_t
Description:
 batom_pipe_t is used to group atoms into the FIFO sequence, there is no limit to number of elements queued in the pipe, however given atom could participate only
 at one pipe at the time.
**/
typedef struct batom_pipe *batom_pipe_t;

/**
Summary:
 batom_accum_t a slicing/cutting board to construct atoms
Description:
 batom_accum_t is used to combins atoms and memory regions into the single object, and then to extract atom.
**/
typedef struct batom_accum *batom_accum_t;

/**
Summary:
 batom_factory_t is used to manage resources need by batom_t
**/
typedef struct batom_factory *batom_factory_t;

/**
Summary:

  batom_vec structure is a lowest-level building block of batom_t, it describes contiguous block of memory
**/
typedef struct batom_vec {
    /* members are arranged in a way that they would be packed and structure occupies 2x32 bits */
    void *base; /* pointer to the vectors start address */
    uint16_t len; /* size is 16 bit field, to there are 16 bit spare */
    bool  dirty;  /* set to true if block was modified, e.g. need to flush a cache */
    uint8_t unused_flags; /* misc flags */
} batom_vec;
#define BATOM_VEC_INITIALIZER(base, len) {(base), (len), false, 0}

/**
Summary:
  batom_user_free type of function pointer is called when batom_t is recycled
**/
typedef void (*batom_user_free)(
    batom_t atom,  /* instance of batom_t */
    void *user     /* pointer to the user supplied data */
);

/**
Summary:
  batom_user structure describes user supplied data that associated with the batom_t
Description:
  This structure is used to associate user specified descriptor with batom_t.
  When pointer to the user supplied data is passed to one of functions that create batom_t,
  user data copied into the batom_t private storage, number of bytes copied specified
  by the user_size structure member. When atom recycled and user_free function called
  it receives pointer to the batom private copy of user data.
**/
typedef struct batom_user {
    batom_user_free user_free; /* this function called when atom is recycled */
    size_t user_size; /* size of the user data */
} batom_user;


/**
Summary:
  This type allows read-only navigation over sequence of batom_vec
Description:
  To access data stored in the batom_t or batom_accum_t, user shall use batom_cursor family of functions.
  This structure is used for internal book-keeping by forementioned functions. Memebers of this structure
  shall not be accesses directly. Use of batom_cursor functions have benefit that it seamlessly handles
  discontinuity in the data.
**/
typedef struct batom_cursor {
    const uint8_t *cursor; /* pointer to the current read pointer */
    int left; /* number of bytes left in the cursor, -1 if cursor have reached last data in the vector array */
    uint16_t pos; /* current position in the vector array */
    uint16_t count; /* number of entries in the vector array */
    const batom_vec *vec; /* pointer to the vector array */
} batom_cursor;

/**
Summary:
  This structure is used to retrieve accumulated statistics from the batom_factory_t instance
**/
typedef struct batom_factory_stats {
    unsigned atom_live;     /* number of not recycled atoms */
    unsigned atom_allocated;/* number of allocated atoms */
    unsigned atom_freed;    /* number of recycled atoms */
    unsigned alloc_pool;    /* number of atoms allocated from the pool allocator */
    unsigned alloc_arena;   /* number of atoms allocated from the arena allocator */
    unsigned alloc_alloc;   /* number of atoms allocated from the standard allocator */
    unsigned free_pool;     /* number of atoms freed from the pool allocator */
    unsigned free_arena;    /* number of atoms freed from the arena allocator */
    unsigned free_alloc;    /* number of atoms freed from the standard allocator */
    unsigned full_pool;     /* number of times when poll allocator was exhausted */
    unsigned full_arena;    /* number of times when arena allocator was exhausted */
    bpool_status pool_status;/* status of the pool allocator */
} batom_factory_stats;


/**
Summary:
  This structure is used to save batom_cursor position
**/
typedef struct batom_checkpoint {
    int cp_left;        /* copy of left from the batom_cursor */
    unsigned cp_pos;    /* copy of pos from the batom_cursor */
} batom_checkpoint;



/**
Summary:
  This constant is used by batom_cursor functions to signal end of data condition
**/
#define BATOM_EOF   (-1)
/**
Summary:
  This constant defined maximum number of vectors that could be saved in the single atom
**/
#define BATOM_VEC_MAX_SIZE (65535)

/**
Summary:
 Creates atom factory
**/
batom_factory_t batom_factory_create(
    balloc_iface_t alloc,  /* memory allocator for the factory */
    size_t natoms /* number of preallocated atoms */
    );

/**
Summary:
 Releases resources allocated by factory.
Note:
 When this function called all atoms shall be already recycled.
**/
void batom_factory_destroy(
    batom_factory_t factory /* factory instance */
    );
/**
Summary:
 This function returns statistics for the factory
**/
void batom_factory_get_stats(batom_factory_t factory, batom_factory_stats *stats);

/**
Summary:
 This function dumps active objects in the factory
**/
void batom_factory_dump(batom_factory_t factory);

/**
Summary:
 This function creates accumulator object
**/
batom_accum_t batom_accum_create(
    batom_factory_t factory /* instance of factory */
    );

/**
Summary:
 This function creates contents of accumulator
Description:
 Function batom_accum_clear would recycle all accumulated atoms
**/
void batom_accum_clear(
    batom_accum_t acc /* instance of accumulator */
    );

/**
Summary:
 This function releases resources associated with accumulator
**/
void batom_accum_destroy(
    batom_accum_t acc /* instance of accumulator */
    );

/**
Summary:
 This function returns number of bytes accumulated in accumulator
**/
size_t batom_accum_len(
    batom_accum_t acc /* instance of accumulator */
    );

/**
Summary:
 This function returns vector array from accumulator
Description:
 Accumulator is collection of vectors, this function provides direct access to all vectors.
Note:
 Function returns read only pointer to the atom, therefore accumulator shall not be modified, also modification of memory that vector points to is discouraged as well.
**/
const batom_vec *
batom_accum_get_vectors(
    batom_accum_t acc,  /* accumulator */
    unsigned *nvecs /* number of vectors in the atom */
    );

/**
Summary:
 This function dumps data accumulated in the accumulator
**/
void batom_accum_dump(
    batom_accum_t acc,  /* instance of accumulator */
    const char *name    /* name printed in the dump */
    );

/**
Summary:
 This function adds vector into the accumulator
**/
void batom_accum_add_vec(
    batom_accum_t acc, /* instance of accumulator */
    const batom_vec *vec /* pointer to the vector */
    );

/**
Summary:
 This function adds memory range into accumulator
**/
void batom_accum_add_range(
    batom_accum_t acc,  /* instance of accumulator */
    const void *base,   /* start address */
    size_t len          /* length */
);

/**
Summary:
 This function adds atom into accumulator
Note:
 The same atom could be added multiple time into the same or different accumulators
**/
void batom_accum_add_atom(
    batom_accum_t acc,  /* instance of accumulator */
    batom_t atom        /* atom */
    );

/**
Summary:
 This function 'trims' data from accumulator
Description:
 The batom_accum_trim removes data stored in accumulator started from beginning of accumulator and up to cursor.
 Cursor shall be created from the accumulator. Atoms from recycled region would be recycled.
 After completion of function cursors still points into the same data.
**/
void batom_accum_trim(
    batom_accum_t acc, /* instance of accumulator */
    batom_cursor *cursor /* cursor which marks trimming point */
    );

/**
Summary:
 This function creates atom from slice of accumulator
Description:
 The batom_accum_extract functions produces new atom with content copied from the accumulator
 Both cursors shall be created from the accumulator.
**/
batom_t batom_accum_extract(
    batom_accum_t acc,          /* instance of accumulator */
    const batom_cursor *first,  /* cursor that marks start of range */
    const batom_cursor *last,   /* cursor that marks end of range */
    const batom_user  *user,    /* pointer to the user data descriptor */
    const void *udata           /* pointer to the user data */
);

/**
Summary:
 This function appends data from one accumulator to another
Description:
 The batom_accum_append functions appends data to the destination accumulator, from the source accumulator bounded by two cursors.
 Both cursors shall be created from the source accumulator.
 This function returns false if resources were exhausted and append has failed.
**/
bool batom_accum_append(
    batom_accum_t dst, /* destination accumulator */
    batom_accum_t src, /* source accumulator */
    const batom_cursor *first, /* cursor that marks start of range */
    const batom_cursor *last /* cursor that marks end of range */
);

/**
Summary:
 This function creates atom from single vector
**/
batom_t batom_from_vector(
    batom_factory_t factory, /* instance of factory */
    const batom_vec *vec,    /* vector that describes memory region */
    const batom_user  *user, /* pointer to the user data descriptor */
    const void *udata        /* pointer to the user data */
    );

/**
Summary:
 This function creates atom from array of vectors
**/
batom_t batom_from_vectors(
    batom_factory_t factory, /* instance of factory */
    const batom_vec *vecs,   /* array of vectors that describes in memory object */
    unsigned nvecs,          /* number of entries in the vector array */
    const batom_user  *user, /* pointer to the user data descriptor */
    const void *udata        /* pointer to the user data */
    );

/**
Summary:
 This function creates atom from consecutive memory range
**/
batom_t batom_from_range(
    batom_factory_t factory, /* instance of factory */
    const void *base,        /* first address of memory region */
    size_t len,              /* length of memory region */
    const batom_user  *user, /* pointer to the user data descriptor */
    const void *udata        /* pointer to the user data */
    );

/**
Summary:
 This function creates atom from accumulator
Description:
 This function creates atom from accumulator content and then clears accumulator.
**/
batom_t batom_from_accum(
    batom_accum_t acc,      /* instance of accumulator */
    const batom_user *user, /* pointer to the user data descriptor */
    const void *udata       /* pointer to the user data */
    );

/**
Summary:
 This function creates clone of atom
Description:
 This function creates atom with exactly the same content, but with different user data
**/
batom_t batom_clone(
    batom_t atom,            /* instance of accumulator */
    const batom_user *user,  /* pointer to the user data descriptor */
    const void *udata       /* pointer to the user data */
    );

/**
Summary:
 This function creates of atom from concatenation of memory range and atom
**/
batom_t batom_from_range_and_atom(
    const void *base,   /* first address of memory region */
    size_t len,         /* length of memory region */
    batom_t atom,       /* atom */
    const batom_user *user,  /* pointer to the user data descriptor */
    const void *udata   /* pointer to the user data */
    );

/**
Summary:
 This function creates of atom from concatenation of vector and atom
**/
batom_t batom_from_vec_and_atom(
        const batom_vec *vec, /* vector that describes memory region */
        batom_t atom,         /* atom */
        const batom_user *user,  /* pointer to the user data descriptor */
        const void *udata    /* pointer to the user data */
        );

/**
Summary:
 This function creates of empty atom, e.g. atom without associated data
**/
batom_t batom_empty(
        batom_factory_t factory, /* instance of factory */
        const batom_user *user,  /* pointer to the user data descriptor */
        const void *udata        /* pointer to the user data */
        );

/**
Summary:
 This function increments reference counter of atom
**/
void batom_lock(
    batom_t atom /* atom */
    );

/**
Summary:
 This function decrements reference counter of atom
Description:
 If reference counter passes through zero, memory occupied by atom is recycles and,
 if applicable, user supplied free function called.
**/
void batom_release(
    batom_t atom /* atom */
    );

/**
Summary:
 This function returns size (in bytes) of atom payload
**/
size_t batom_len(
    batom_t atom /* atom */
    );

/**
Summary:
 This function returns vector from atom
Description:
 Atom is collection of vectors, this function provides direct access to the vectors.
Note:
 Function returns read only pointer to the atom, therefore atom shall not be modified, also modification of memory that vector points to is discouraged as well.
**/
const batom_vec *batom_get_vec(
    batom_t atom,  /* atom */
    unsigned vecno /* request element */
    );
/**
Summary:
 This function returns size of the atom's vector array
**/
unsigned batom_get_nvecs(
    batom_t atom /* atom */
    );

/**
Summary:
 This function returns vector array from atom
Description:
 Atom is collection of vectors, this function provides direct access to all vectors.
Note:
 Function returns read only pointer to the atom, therefore atom shall not be modified, also modification of memory that vector points to is discouraged as well.
**/
const batom_vec *
batom_get_vectors(
    batom_t atom,  /* atom */
    unsigned *nvecs /* number of vectors in the atom */
    );

/**
Summary:
 This function returns pointer to the user data saved in the atom
**/
void *batom_userdata(
    batom_t atom /* atom */
    );

/**
Summary:
 This function extracts content from atom as specified by two cursors
**/
batom_t batom_extract(
    batom_t src,                /* source atom */
    const batom_cursor *first,  /* cursor that marks start of range */
    const batom_cursor *last,   /* cursor that marks end of range */
    const batom_user  *user,    /* pointer to the user data descriptor */
    const void *udata           /* pointer to the user data */
    );

/**
Summary:
 This function prints content of the atom
**/
void batom_dump(
    batom_t atom,  /* atom */
    const char *name /* name of atom printed in the dump */
    );

/**
Summary:
 This function creates cursor from the vector array
**/
void batom_cursor_from_vec(
    batom_cursor *cursor,  /* pointer to the cursor */
    const batom_vec *vec,  /* vector array */
    unsigned count           /* size of the vector array */
    );

/**
Summary:
 This function creates cursor from atom
**/
void batom_cursor_from_atom(
    batom_cursor *cursor, /* pointer to the cursor */
    batom_t atom          /* atom */
    );

/**
Summary:
 This function creates cursor from accumulator
**/
void batom_cursor_from_accum(
    batom_cursor *cursor, /* pointer to the cursor */
    batom_accum_t acc      /* accumulator */
    );

/**
Summary:
 This function creates cursor from accumulator
**/
void batom_cursor_empty(
    batom_cursor *cursor /* pointer to the cursor */
    );



/**
Summary:
 This function returns next byte from the cursor
Result:
 Function returns either byte fetched from the cursor, or BATOM_EOF if cursor advanced past end of data
**/
int batom_cursor_next(
    batom_cursor *cursor /* pointer to the cursor */
    );

/**
Summary:
 This function tests whether cursor advanced past end of data
**/
bool batom_cursor_eof(
    const batom_cursor *cursor /* pointer to the cursor */
    );

/**
Summary:
 This function advances cursor by specified number of bytes
Result:
 Function returns number of bytes advanced, it could be smaller than requested number if cursor advanced past end of data
**/
size_t batom_cursor_skip(
    batom_cursor *cursor, /* pointer to the cursor */
    size_t count          /* number of bytes to skip */
    );

/**
Summary:
 This function copies data from cursor
Result:
 Function returns number of bytes copied, it could be smaller than requested number if cursor advanced past end of data
**/
size_t batom_cursor_copy(
    batom_cursor *cursor, /* pointer to the cursor */
    void *dest,           /* address of the destination buffer */
    size_t count          /* number of bytes to copy */
    );

/**
Summary:
 This function copies data from cursor to the array of batom_vec
Result:
 Function returns number of bytes copied, it could be smaller than requested number if cursor advanced past end of data
Error:
 If there is not enough space in the destination arrays, then 'required_vec_size' would be larger then 'vec_size'
**/
size_t batom_cursor_extract (
    const batom_cursor *cursor, /* pointer to the cursor */
    batom_vec *vecs,            /* pointer to the batom_vec */
    size_t vec_size,            /* number of elements in vec */
    size_t *required_vec_size   /* number of elements used in vec */
    );

/**
Summary:
 This function returns distance between two cursors, cursors shall be created for the same underlying object
*
* #endif*/
size_t batom_cursor_distance(
    const batom_cursor *from, /* pointer to the first cursor */
    const batom_cursor *to    /* pointer to the second cursor */
    );


/**
Summary:
 This function saves position of cursor
Description:
 Position of cursor is saved into the batom_checkpoint data structure.
**/
void batom_cursor_save(
    const batom_cursor *cursor,  /*  pointer to the cursor */
    batom_checkpoint *checkpoint /* pointer to checkpoint */
    );
/**
Summary:
 This function restores position of cursor
Description:
 Position of cursor restored based on checkpoint. The same rollback pointer could be used multiple times as long as
 underlying element (atom,accumulator or vector) didn't change.
 If changes have occurred in the underlying element, behaviour is undefined.
**/
void batom_cursor_rollback(
    batom_cursor *cursor, /*  pointer to the cursor */
    const batom_checkpoint *checkpoint /* pointer to checkpoint */
    );

/**
Summary:
 This function makes an identical copy of cursor
**/
void batom_cursor_clone(
    batom_cursor *dst,  /* pointer to the destination cursor */
    const batom_cursor *src /* pointer to the source cursor */
    );

/**
Summary:
 This function returns current position of cursor
**/
size_t batom_cursor_pos(
    const batom_cursor *cursor /* pointer to cursor */
    );

/**
Summary:
 This function returns cursor's size, e.g. number of bytes addressable by cursor
**/
size_t batom_cursor_size(
    const batom_cursor *cursor /* pointer to cursor */
    );

/**
Summary:
 This function validates number of bytes available in the cursor
Description:
 This function is identical to batom_cursor_skip, with exception that cursor position doesn't change
**/
size_t batom_cursor_reserve(
    const batom_cursor *cursor, /* pointer to cursor */
    size_t count                /* number of bytes to reserve */
    );


/**
Summary:
 This function compares the contents of a cursor with the contents of a buff. It doesn't change cursor/
Result:
 Function returns 0 if the buffers are identical
**/
int batom_cursor_compare(
    const batom_cursor *cursor,
    const void *buff,  /* address of the compare buffer */
    size_t bytes       /* number of bytes to compare */
    );

/**
Summary:
 This function prints content of the cursor, starting from current position
**/
void batom_cursor_dump(
    const batom_cursor *c,  /* pointer to cursor */
    const char *name        /* name to be displayed in the dump */
    );

/**
Summary:
 This function returns next byte from the cursor
Description:
 Function would return undefined data if end of data reached.
 however it's safe to call this function when end of data reached as many times as required.
**/
uint8_t batom_cursor_byte(
    batom_cursor *c /* pointer to cursor */
    );

/**
Summary:
 This function returns next 16-bit little endian number from the cursor
Description:
 Function would return undefined data if end of data reached.
 however it's safe to call this function when end of data reached as many times as required.
 If cursor has content of 0x11 0x22, then this function would return 0x2211
**/
uint16_t batom_cursor_uint16_le(
    batom_cursor *c /* pointer to cursor */
    );

/**
Summary:
 This function returns next 16-bit big endian number from the cursor
Description:
 Function would return undefined data if end of data reached.
 however it's safe to call this function when end of data reached as many times as required.
 If cursor has content of 0x11 0x22, then this function would return 0x1122
**/
uint16_t batom_cursor_uint16_be(
    batom_cursor *c /* pointer to cursor */
    );

/**
Summary:
 This function returns next 32-bit little endian number from the cursor
Description:
 Function would return undefined data if end of data reached.
 however it's safe to call this function when end of data reached as many times as required.
 If cursor has content of 0x11 0x22 0x33 0x44, then this function would return 0x44332211
**/
uint32_t batom_cursor_uint32_le(
    batom_cursor *c /* pointer to cursor */
    );


/**
Summary:
 This function returns next 32-bit big endian number from the cursor
Description:
 Function would return undefined data if end of data reached.
 however it's safe to call this function when end of data reached as many times as required.
 If cursor has content of 0x11 0x22 0x33 0x44, then this function would return 0x11223344
**/
uint32_t batom_cursor_uint32_be(
    batom_cursor *c /* pointer to cursor */
    );

/**
Summary:
 This function returns next 64-bit little endian number from the cursor
Description:
 Function would return undefined data if end of data reached.
 however it's safe to call this function when end of data reached as many times as required.
 If cursor has content of 0x11 0x22 0x33 0x44 0x55 0x66 0x77 0x88, then this function would return 0x8877665544332211
**/
uint64_t batom_cursor_uint64_le(
    batom_cursor *c  /* pointer to cursor */
    );

/**
Summary:
 This function returns next 64-bit big endian number from the cursor
Description:
 Function would return undefined data if end of data reached.
 however it's safe to call this function when end of data reached as many times as required.
 If cursor has content of 0x11 0x22 0x33 0x44 0x55 0x66 0x77 0x88, then this function would return 0x1122334455667788
**/
uint64_t batom_cursor_uint64_be(
    batom_cursor *c /* pointer to cursor */
    );

/**
Summary:
 This function returns next 24-bit little endian number from the cursor
Description:
 Function would return undefined data if end of data reached.
 however it's safe to call this function when end of data reached as many times as required.
 If cursor has content of 0x11 0x22 0x33, then this function would return 0x332211
**/
uint32_t batom_cursor_uint24_le(batom_cursor *c);

/**
Summary:
 This function returns next 24-bit big endian number from the cursor
Description:
 Function would return undefined data if end of data reached.
 however it's safe to call this function when end of data reached as many times as required.
 If cursor has content of 0x11 0x22 0x33, then this function would return 0x112233
**/
uint32_t batom_cursor_uint24_be(
    batom_cursor *c /* pointer to cursor */
    );

/**
Summary:
 This function returns next 1,2,3 or 4 bytes combined as little endian number
**/
uint32_t batom_cursor_vword_le(
    batom_cursor *c,    /* pointer to cursor */
    unsigned bytes      /* number of bytes to read */
    );

/**
Summary:
 This function returns next 1,2,3 or 4 bytes combined as big endian number
**/
uint32_t batom_cursor_vword_be(
    batom_cursor *c, /* pointer to cursor */
    unsigned bytes     /* number of bytes to read */
    );

/**
Summary:
 This function returns continuous buffer of specified length.
Description:
 The batom_cursor_continuous would return pointer to the current read location
 if cursor points to the contiguous buffer of specified length, otherwise function
 will return NULL.
 It would return NULL, in either case if size of cursor is smaller then specified size
 or cursor passes through fragmented data.
**/
const void *batom_cursor_continuous(
    const batom_cursor *c, /* pointer to cursor */
    size_t bytes           /* number of continuous bytes requested */
    );

/**
Summary:
 This function compares first n bytes of memory addressed by cursor and regular pointer
*/
int batom_cursor_memcmp(
    const batom_cursor *cursor,
    const void *data,
    size_t n
    );

/**
Summary:
 This function prints content of the memory range
**/
void batom_range_dump(
    const void *data,  /* pointer to data */
    size_t length,     /* number of bytes in the buffer */
    const char *name   /* name to be displayed in the dump */
    );

/**
Summary:
 This function creates pipe
**/
batom_pipe_t batom_pipe_create(
    batom_factory_t factory /* factory instance */
    );

/**
Summary:
 This function releases resources allocated by a pipe
**/
void batom_pipe_destroy(
    batom_pipe_t pipe  /* pipe instance */
    );

/**
Summary:
 This function removes all atoms from the pipe.
Description:
 For every atom removed from the pipe, batom_release function called
**/
void batom_pipe_flush(
    batom_pipe_t pipe /* pipe instance */
    );

/**
Summary:
 This function adds atom into head of pipe
Description:
 One atom could be added only to single pipe.
 If one atom shall be added to several pipes, the batom_clone function could be used to create a new atom
**/
void batom_pipe_push(
    batom_pipe_t pipe, /* pipe instance */
    batom_t atom    /* atom */
    );
/**
Summary:
 This function returns first atom from the tail of pipe
Result:
  atom or NULL if pipe is empty
**/
batom_t batom_pipe_peek(
    batom_pipe_t pipe /* pipe instance */
    );

/**
Summary:
 This function extracts first atom from the tail of pipe
Result:
  atom or NULL if pipe is empty
**/
batom_t batom_pipe_pop(
    batom_pipe_t pipe /* pipe instance */
    );

/**
Summary:
 This function drops first atom from the tail of pipe
**/
void batom_pipe_drop(batom_pipe_t pipe);

/**
Summary:
 This function to initializes batom_vec structure
**/
void batom_vec_init(
    batom_vec *vec,     /* pointer to the batom_vec */
    const void *base,   /* start of the memory region */
    size_t len          /* length of memory region */
    );

/**
Summary:
 This function defines bit parser based on batom_cursor
Description:
 This structure is used for internal book-keeping by batom_bitstream functions and
 shall not be accesses directly.
**/
typedef struct batom_bitstream {
    batom_cursor *cursor; /* pointer to batom_cursor */
    uint32_t cache;       /* 32 bits prefetched from the cursor */
    int cache_pos;       /* number of bits left in the cache */
} batom_bitstream;

/**
Summary:
 This function initializes batom_bitstream structur
**/
void batom_bitstream_init(
    batom_bitstream *bs,  /* pointer to the batom_bitstream */
    batom_cursor *cursor  /* pointer to the batom_cursor which is used as a data source */
    );

/**
Summary:
 This function detects whether last bit was read from th batom_bitstream
**/
bool batom_bitstream_eof(
    batom_bitstream *bs /* pointer to the batom_bitstream */
    );

/**
Summary:
 This function shows (peeks) current bit in the bitstream, but it doesn't advances to the next bit
Result:
  BATOM_EOF if no bits left in the bitstream
  0 or 1 otherwise
**/
int batom_bitstream_show(
    batom_bitstream *bs    /* pointer to the batom_bitstream */
    );

/**
Summary:
 This function returns current bit in the bitstream and advances to the next bit
Result:
  BATOM_EOF if no bits left in the bitstream
  0 or 1 otherwise
**/
int batom_bitstream_bit(
    batom_bitstream *bs /* pointer to the batom_bitstream */
    );

/**
Summary:
 This function advances to the next bite
Result:
  BATOM_EOF if no bits left in the bitstream
**/
int batom_bitstream_drop(
    batom_bitstream *bs /* pointer to the batom_bitstream */
    );

/**
Summary:
 This function returns specified number of bits from the bitstream
**/
unsigned batom_bitstream_bits(
    batom_bitstream *bs, /* pointer to the batom_bitstream */
    unsigned nbits        /* number of bits to read */
    );

/**
Summary:
 This function skips specified number of bits from the bitstream
**/
void batom_bitstream_drop_bits(
    batom_bitstream *bs, /* pointer to the batom_bitstream */
    unsigned nbits       /* number of bits to skip */
    );


/**
Summary:
 This function reads single Exponential-Golomb coded value
**/
unsigned batom_bitstream_exp_golomb(
    batom_bitstream *bs /* pointer to the batom_bitstream */
   );


/**
Summary:
 This function prints out status of bitstream
**/
void batom_bitstream_dump(
    const batom_bitstream *bs /* pointer to bitstream */
    );

/**
Summary:
 This function returns position in the cursor, if bitstream was only reading from cursor byte at the time. This function should be used to help implement switch from bitstream back to the bytestream.
**/
size_t batom_bitstream_position(
    const batom_bitstream *bs /* pointer to bitstream */
    );

/**
Summary:
 This function returns position in the bitstream (in bits)
**/
size_t batom_bitstream_bit_position(
    const batom_bitstream *bs /* pointer to bitstream */
    );




#ifndef B_ATOM_FAST
#define B_ATOM_FAST 1
#endif


#if B_ATOM_FAST
/**
Summary:
 This macro reads next byte from the batom_cursor
See also:
 batom_cursor_next
**/
#define BATOM_NEXT(d,c) do {if((c)->left>0) {(c)->left--;d=*(c)->cursor++;} else {d=batom_cursor_next(c);}}while(0)
/**
Summary:
 This macro tests whether last byte was read from the batom_cursor
**/
#define BATOM_IS_EOF(c) ((c)->left<0)
/**
Summary:
 This macro saves current position of the batom_cursor
See also:
 batom_cursor_save
**/
#define BATOM_SAVE(cur,check) do {(check)->cp_left=(cur)->left;(check)->cp_pos=(cur)->pos;}while(0)
/**
Summary:
 This macro makes an identical copy of cursor
See also:
 batom_cursor_clone
**/
#define BATOM_CLONE(dst,src) *(dst)=*(src)
/**
Summary:
 This macro initializes batom_vec structure
See also:
 batom_vec_init
**/
#define BATOM_VEC_INIT(vec, vec_base, vec_len) B_ATOM_VEC_INIT(vec, vec_base, vec_len)

/**
Summary:
 This macro returns number of bytes available in the cursor
See also:
 batom_cursor_reserve
**/
#define BATOM_RESERVE(cur,bytes) ((cur)->left >= (int)(bytes) ? (bytes):batom_cursor_reserve((cur),(bytes)))

/**
Summary:
 This macro reads 32-bit big endian number from cursor
See also:
 batom_cursor_uint32_be
**/
#define BATOM_UINT32_BE(cur) (((cur)->left >= 4) ?  ( \
    (cur)->left -= 4, (cur)->cursor+=4, \
    ((((uint32_t)(cur)->cursor[0-4])<<24) |  \
    (((uint32_t)(cur)->cursor[1-4])<<16) |  \
    (((uint32_t)(cur)->cursor[2-4])<<8) | \
    (cur)->cursor[3-4])) \
    : batom_cursor_vword_be((cur), 4))

/**
Summary:
 This macro advances cursor's read pointer
See also:
 batom_cursor_skip
**/
#define BATOM_SKIP(cur,bytes) do{ if((cur)->left >= (bytes)) { (cur)->left-=(bytes);(cur)->cursor+=(bytes);} else {batom_cursor_skip((cur),(bytes));}} while(0)
/**
 *
Summary:
 This macro returns pointer to the cursor continuous buffer or NULL
See also:
 batom_cursor_continuous
**/
#define BATOM_CONTINUOUS(cur,bytes) ((cur)->left >= (int)(bytes) ? (cur)->cursor : batom_cursor_continuous((cur),(bytes)))

/**
Summary:
 This macro restores state of cursor
See also:
  batom_cursor_rollback
**/
#define BATOM_ROLLBACK(cur, chk) do { if( (cur)->left>=0 && (cur)->pos == (chk)->cp_pos ) { (cur)->cursor -= (chk)->cp_left - (cur)->left; (cur)->left=(chk)->cp_left;} else {batom_cursor_rollback((cur), (chk));}}while(0)


#else  /* B_ATOM_FAST */
#define BATOM_NEXT(d,c) ((d)=batom_cursor_next(c))
#define BATOM_IS_EOF(c) batom_cursor_eof(c)
#define BATOM_SAVE(cur,check) batom_cursor_save((cur),(check))
#define BATOM_CLONE(dst,src) batom_cursor_clone((dst),(src))
#define BATOM_VEC_INIT(vec, vec_base, vec_len) batom_vec_init((vec), (vec_base), (vec_len))
#define BATOM_RESERVE(cur,bytes) batom_cursor_reserve((cur), (bytes))
#define BATOM_UINT32_BE(cur) batom_cursor_uint32_be(cur)
#define BATOM_SKIP(cur, bytes) batom_cursor_skip((cur),(bytes()
#define BATOM_CONTINUOUS(cur,bytes) batom_cursor_continuous((cur),(bytes))
#define BATOM_ROLLBACK(cur, chk) batom_cursor_rollback((cur), (chk))
#endif  /* B_ATOM_FAST */

#define B_ATOM_VEC_INIT(vec, vec_base, vec_len) do { (vec)->base=(void *)(vec_base);(vec)->len = (vec_len);(vec)->dirty = true; (vec)->unused_flags = 0;} while(0)

#ifdef __cplusplus
}
#endif

#endif /* __BATOM_H__ */

