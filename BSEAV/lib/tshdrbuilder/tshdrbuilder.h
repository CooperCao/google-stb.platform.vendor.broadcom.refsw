/***************************************************************************
 *     Copyright (c) 2002-2007, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/

/*= Module Overview ********************************************************
<verbatim>

Overview
This utility contains functions for creating general purpose TS packet header
and constructing PSI informaiton, in particular, Program Associate Table (PAT)
and Program Map Tables (PMT). It is originally designed for use of settop
transcoders, hence there are limitations in terms of what one could achieve
with the available settings.

Example
// 1) Create a TS Header for a PES payload of length 4096
    TS_PID_info pidInfo;
    TS_PID_state pidState;
    uint16_t pes_pid = 0x21;
    uint8_t ts_hdr_buf[1024];
    size_t ts_buf_used, payload_pked;
    
    TS_PID_info_Init(&pidInfo, pes_pid, true);
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, ts_hdr_buf, 1024, &ts_buf_used, 4098, &payload_pked, 500);

// 2) Create a TS Header for a PSI payload of length 10
    TS_PID_info pidInfo;
    TS_PID_state pidState;
    uint16_t psi_pid = 0x2;
    uint8_t psi_hdr_buf[189];
    size_t psi_buf_used, payload_pked;
    unsigned maxPacket = 1;                 // Must be 1 for PSI

    TS_PID_info_Init(&pidInfo, psi_pid, true);
    pidInfo.pointer_field = true;           // Necessary for PSI
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, psi_hdr_buf, 189, &psi_buf_used, 10, &payload_pked, maxPacket);

// 3) Create a dedicated PCR packet (i.e. no payload)
    TS_PID_info pidInfo;
    TS_PID_state pidState;
    uint16_t pcr_pid = 0x1FFE;
    uint8_t pcr_buf[189];
    size_t pcr_buf_used, payload_pked;
    unsigned maxPacket = 1;                 // Must be 1 when pcr_valid is true

    TS_PID_info_Init(&pidInfo, pcr_pid, true);
    pidInfo.pcr_valid = true;
    pidInfo.pcr = 0x1DEADBEEF;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pcr_buf, 189, &pcr_buf_used, 0, &payload_pked, maxPacket);

// 4) Construct a PAT, add a program, and add a stream to that program
    uint8_t pat_buf[189], pmt_buf[189];
    TS_PSI_header psi_header;
    TS_PAT_state pat;
    TS_PAT_program program;
    TS_PMT_state pmt;
    TS_PMT_stream pmt_stream;
    unsigned streamNum;
    size_t pat_len, pmt_len;
    pes_pid = 0x21;
    pes_format = 0xFF;

    TS_PSI_header_Init(&psi_header);
    TS_PAT_Init(&pat, &psi_header, pat_buf, 189);
    TS_PAT_program_Init(&program, 1, 0x100);
    TS_PAT_addProgram(&pat, &pmt, &program, pmt_buf, 189);
    TS_PMT_stream_Init(&pmt_stream, pes_format, pes_pid);
    TS_PMT_addStream(&pmt, &pmt_stream, &streamNum);
    TS_PAT_finalize(pat, &pat_len);
    TS_PMT_finalize(pmt, &pmt_len);

See more examples in /rockford/applications/tshdrbuilder_example/tshdrbuilder_test.c

</verbatim>
***************************************************************************/


#ifndef TSHDRBUILDER_H__
#define TSHDRBUILDER_H__
#include "bstd.h"
#include "bkni.h"
#include "ts_psi.h"
#include "ts_priv.h"
#include "ts_pat.h"
#include "ts_pmt.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    uint16_t PID;                       /* 13-bit pid value */
    bool PUSI;                          /* Payload Unit Start Indicator, set when starting a PES/PSI section in payload */
    bool pointer_field;                 /* Indicates whether to generate the pointer field */
    bool pcr_valid;                     /* Indicates whether header should contain PCR information */
    uint64_t pcr;                       /* 33-bit PCR */
    uint16_t pcr_ext;                   /* 0...299 PCR extension */
} TS_PID_info;

typedef struct {
    uint8_t continuity_counter;         /* Tracks the CC value to use for next packetization of the same PID */
} TS_PID_state;

typedef struct {
    uint8_t *buf;                       /* PAT buffer */
    size_t bufSize;                     /* Memory size alloted for buf */
    size_t size;                        /* Table size, updated when TS_PAT_finalize is called */
} TS_PAT_state;

typedef struct {
    uint8_t *buf;                       /* PMT buffer */
    size_t bufSize;                     /* Memory size alloted for buf */
    size_t size;                        /* Table size, updated when TS_PMT_finalize is called */
} TS_PMT_state;

/**
Summary:
    Initialize default settings for struct TS_PID_info.

Description:
    Default values are
    pointer_field = false;
    pcr_valid = false;
    pcr = 0x0;
    pcr_ext = 0x0;
**/

void TS_PID_info_Init(
    TS_PID_info * info,                 /* OUT */
    uint16_t PID,                       /* IN: 13-bit pid value */
    bool PUSI                           /* IN: Payload Unit Start Indicator */
);

/**
Summary:
    Initialize struct TS_PSI_header.

Description:
    TS_PAT_addProgram attaches this information onto a PAT table.
**/
void TS_PAT_program_Init(
    TS_PAT_program *program,            /* OUT */
    uint16_t program_number,            /* IN: program number to add */
    uint16_t PID                        /* IN: corrosponding PMT's PID number */
);

/**
Summary:
    Initialize struct TS_PMT_stream.

Description:
    TS_PMT_addStream attaches this information onto a PMT table.
    See ts_psi.h for a complete listing of stream types.
**/
void TS_PMT_stream_Init(
    TS_PMT_stream *stream,              /* OUT */
    uint8_t stream_type,                /* IN: PES' format */
    uint16_t elementary_PID             /* IN: PES' PID number */
);

/**
Summary:
    Initialize struct TS_PSI_descriptor.

Description:
    TS_PMT_setDescriptor attaches this information to a PMT table.
**/
void TS_PSI_descriptor_Init(
    TS_PSI_descriptor *desc,            /* OUT */
    void *descBuf                       /* IN: array containing bytes of stream description */
);


/**
Summary:
    Initialize struct TS_PID_state.

Description:
    Default values are
    continuity_counter = 0;    
**/

void TS_PID_state_Init( 
    TS_PID_state * state                 /* OUT */
);

/**
Summary:
    Initialize struct TS_PSI_header.

Description:
    Defaults are
    table_id = 0x0;
    section_syntax_indicator = 0x1;
    private_indicator = 0x0;
    section_length = 9;
    table_id_extension = 0x0;
    version_number = 0x00;
    current_next_indicator = 0x1;
    section_number = 0x0;
    last_section_number = 0x0;
**/

void TS_PSI_header_Init(
    TS_PSI_header *header               /* OUT */
);

/**
Summary:
    Constructs TS packet headers.

Description:
    Output memory map:
    typedef struct {
        uint8_t header_size; 
        uint8_t headerBuf[header_size]; / * variable length header * /
    };
    tsHeaderBuf should have at least 188+4*N bytes, where N is the maximum number of packet headers.
    Caveat 1. When pcr_valid or pointer_field is set in pidInfo, packetMax must be equal to 1 or this function call
              returns an error.
    Caveat 2. When pointer_field is set, an extra byte is generated before payload if and only if PUSI is set. The
              application user should ensure that the payload content is applicable (i.e. PSI).
    Caveat 3. pcr_valid and pointer_field cannot both be set in a single TS-packet. This implies that you cannot
              attach PCR info onto a packet that contains a PSI table.
**/

BERR_Code TS_buildTSHeader(
    const TS_PID_info *pidInfo,         /* IN: see struct definition */
    TS_PID_state *pidState,             /* IN/OUT: see struct definition */
    void *tsHeaderBuf,                  /* OUT: pointer to memory block to store the TS Headers */
    size_t tsHeaderBufSz,               /* IN: size of memory block, in bytes */
    size_t *tsHeaderBufBuilt,           /* OUT: size of memory block used to build TS Headers, value will be <= 'tsHeaderBufSz', in bytes */
    size_t payloadSz,                   /* IN: size of payload to generate TS Headers for, in bytes */
    size_t *payloadBuiltSz,             /* OUT: size of payload consumed for generation of TS Headers, value will be <= 'payloadSz', in bytes */
    unsigned packetMax                  /* IN: an upperbound on the number of packet headers this function call can generate */
);

/**
Summary:
    Constructs a PAT with information in input parameter TS_PSI_header and 0 programs.
    
Description:
    To add a program to this table, see TS_PAT_addProgram. patBuf should be at least 188 bytes in size. 

 **/
BERR_Code TS_PAT_Init(
    TS_PAT_state *pat,                  /* IN: a blank PAT state */
    const TS_PSI_header *p_header,      /* IN: TS_PSI_header struct containing header information */
    void *patBuf,                       /* IN: buffer allocated for pmt */
    size_t patBufSz                     /* IN: pmt buffer size */
);

/**
Summary:
    Add a program to the PAT table and initialize a new PMT table with no streams. 
    
Description:
    pmtBuf should be at least 188 bytes in size.
    
 **/

BERR_Code TS_PAT_addProgram(
    TS_PAT_state *pat,                  /* IN: an initialized PAT state */
    TS_PMT_state *pmt,                  /* IN: a blank PMT state */
    const TS_PAT_program *p_program,    /* IN: TS_PAT_program struct containing program information */
    void *pmtBuf,                       /* IN: buffer allocated for pmt */
    size_t pmtBufSz                     /* IN: pmt buffer size */
);



/**
Summary:
    Set PCR pid number for a valid PMT table.
    
Description:

 **/
/* Extension to ts_pmt class */
BERR_Code TS_PMT_setPcrPid(
    TS_PMT_state *pmt,                  /* IN: a valid PMT state initalized by TS_PAT_addPorgram */
    uint16_t pcrPid                     /* IN: new PCR pid number */
);

/**
Summary:
    Add a stream to a valid PMT table.
    
Description:
    The new stream will contain no stream description by default.

 **/
/* returns stream number */
BERR_Code TS_PMT_addStream(
    TS_PMT_state *pmt,                  /* IN: a valid PMT state initalized by TS_PAT_addPorgram */
    const TS_PMT_stream *p_stream,      /* IN: TS_PMT_stream struct that contains the stream information */
    unsigned *streamNum                 /* OUT: the stream number in current table/program */
);

/**
Summary:
    Set ES descriptor for a valid stream in a valid PMT table.
    
Description:
    Set descriptor overrides the old descriptor for the same stream if exists.

 **/
/* Extension to ts_pmt class */
BERR_Code TS_PMT_setDescriptor(
    TS_PMT_state *pmt,                  /* IN: a valid PMT state initalized by TS_PAT_addPorgram */
    const TS_PSI_descriptor descriptor, /* IN: descriptor containing description for this stream */
    unsigned descLen,                   /* IN: length of the descriptor */
    unsigned streamNum                  /* IN: the desired stream to set the descriptor */
);

/**
Summary:
    Finalize a PAT table.
    
Description:
    Compute CRC32 value and assigns the table size to pat_size.

 **/
BERR_Code TS_PAT_finalize(
    TS_PAT_state *pat,                  /* IN: an initialized PAT state */
    size_t *pat_size                    /* OUT: size of the PAT table */
);

/**
Summary:
    Finalize a PMT table

Description:
    Compute CRC32 value and assigns the table size to pmt_size.
    
 **/
BERR_Code TS_PMT_finalize(
    TS_PMT_state *pmt,                  /* IN: an initialized PMT state */
    size_t *pmt_size                    /* OUT: size of the PMT table */
);

#ifdef __cplusplus
}
#endif

#endif /* TSHDRBUILDER_H__ */
