/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include "bstd.h"
#include "bchp_common.h"
#include "bchp_memc_clients.h"

typedef enum tracelog_FilterMode {
    tracelog_FilterMode_eTrigger,
    tracelog_FilterMode_eTriggerAndCapture,
    tracelog_FilterMode_eCapture,
    tracelog_FilterMode_eMax
} tracelog_FilterMode;

typedef enum tracelog_AccessType {
    tracelog_AccessType_eWriteAndRead,
    tracelog_AccessType_eRead,
    tracelog_AccessType_eWrite,
    tracelog_AccessType_eMax
} tracelog_AccessType;

typedef struct tracelog_ClientList {
    uint32_t bits[8];
} tracelog_ClientList;

typedef struct tracelog_IoFilter {
   struct {
       tracelog_AccessType access;
       struct {
           bool enabled;
           bool value;
       } ack;
       struct {
           bool enabled;
           uint32_t data;
           uint32_t mask;
       } data;
       struct {
           bool enabled;
           bool exclusive;
           uint32_t high;
           uint32_t low;
       } address;
       struct {
           bool enabled;
           bool exclusive;
           tracelog_ClientList list;
       } clients;
   } match;
} tracelog_IoFilter;

typedef struct tracelog_MemoryFilter {
   bool snoop_ubus;
   bool snoop_sub;

   struct {
       tracelog_AccessType access;
       struct {
           bool nondata;
           bool group;
           bool block32;
           bool block16;
           bool raster;
           bool cache;
           bool linear;
       } transactions;

       struct {
           bool enabled;
           bool value;
       } ack;
       struct {
           bool enabled;
           bool exclusive;
           unsigned high;
           unsigned low;
       } size;
       struct {
           bool enabled;
           bool exclusive;
           BSTD_DeviceOffset high;
           BSTD_DeviceOffset low;
       } address;
       struct {
           bool enabled;
           bool exclusive;
           tracelog_ClientList list;
       } clients;
   } match;
} tracelog_MemoryFilter;

typedef struct tracelog_Status {
    struct {
        bool all; /* TraceLog Operation is COMPLETELY BLOCKED by secure control. */
        bool secure; /* TraceLog is NOT allowed to snoop Secure IO Transactions. */
    } blocked;
    struct {
        bool trigger; /* Remote Trigger input is asserted. */
        bool active; /* Remote Active input is asserted. */
    } remote;
    bool done;  /* TraceLog is done capturing data to the buffer. */
    bool dropped; /* A capture event has been dropped. This occurs if the TraceLog has lost an event due to the FIFO being Full. */
    bool triggered; /* TraceLog trigger conditions have been met */
    bool active; /* TraceLog is waiting for a trigger, filling the buffer, or flushing the internal buffer. */
    struct {
        uint64_t elapsed;
    } time;
    struct {
        uint32_t total; /* Count of all transactions snooped (matched and unmatched) */
        uint32_t trigger; /* Count of all events marked as triggers */
        uint32_t matches; /* Count of all transactions matched for capture */
        uint32_t dropped; /* Count of transactions matched for capture, but dropped */
        uint32_t preTrigger; /* Count of transactions matched and captured before the trigger */
        uint32_t postTrigger; /* Count of transactions matched and captured after the trigger */
    } events;
} tracelog_Status;

typedef enum tracelog_TriggerMode {
    tracelog_TriggerMode_eManual, /* Trigger when the tracelog_Trigger called */
    tracelog_TriggerMode_eEvents, /* Trigger when the Trigger Filter Set outputs a match triger.config.count times */
    tracelog_TriggerMode_eEventsClose, /* Trigger when two events happen too close together (less than or equal to  trigger.time */
    tracelog_TriggerMode_eEventsFar, /* Trigger when two events happen too far apart (greater than the time specified by trigger.time */
    tracelog_TriggerMode_eDelayed, /* Trigger after a time delay specified in trigger.config.time */
    tracelog_TriggerMode_eMax
} tracelog_TriggerMode;

typedef enum tracelog_CaptureMode {
    tracelog_CaptureMode_ePreTrigger, /* Events captured continuously until trigger. */
    tracelog_CaptureMode_eMidTrigger, /* Mid-Trigger: Continuous capture until trigger, after which half of the buffer is filled. */
    tracelog_CaptureMode_ePostTrigger, /* Post-Trigger: Capture starts after trigger. */
    tracelog_CaptureMode_eMax
} tracelog_CaptureMode;

typedef struct tracelog_MemoryBuffer {
    BSTD_DeviceOffset base;
    void *ptr;
    unsigned size;
} tracelog_MemoryBuffer;

typedef struct tracelog_StartSettings {
    struct {
        tracelog_TriggerMode mode;
        tracelog_CaptureMode capture;
        unsigned count;
        uint64_t time;
    } trigger;
    tracelog_MemoryBuffer buffer;
    bool compact;
} tracelog_StartSettings;

struct tracelog_P_Log_Log16Memory {
    bool AFE_Swapped;
    bool priorityArb; /* or Round Robin */
    bool aborted; /* or allowed */
};

struct tracelog_P_Log_Log16Io {
    bool _64bit;
    bool errorAck;
};

struct tracelog_P_Log16 {
    bool valid;
    bool postTrigger;
    bool lostEvents; /* transaction were dropped */
    bool write; /* or read */
    bool io;
    unsigned id;
    union {
        struct tracelog_P_Log_Log16Memory memory;
        struct tracelog_P_Log_Log16Io io;
    } data;
};

struct tracelog_P_Log_Log128Memory {
    uint64_t addr;
    uint8_t accessId;
    uint8_t burstSize;
    bool field;
    uint8_t command;
};

struct tracelog_P_Log_Log128Io {
    uint32_t data;
    uint32_t addr;
};

struct tracelog_P_Log128 {
    uint64_t timestamp;
    union {
        struct tracelog_P_Log_Log128Memory memory;
        struct tracelog_P_Log_Log128Io io;
    } data;
};

#define TRACELOG_SET_IO_CLIENTS(io, BLOCK) tracelog_SetIoHwBlock((io), tracelog_IoHwBlock_e##BLOCK)

#define TRACELOG_SET_IO_RANGE(io, CORE) do { (io)->match.address.low = BCHP_PHYSICAL_OFFSET + BCHP_##CORE##_REG_START; (io)->match.address.high = BCHP_PHYSICAL_OFFSET + BCHP_##CORE##_REG_END; (io)->match.address.enabled = true; (io)->match.address.exclusive = false;}while(0)

#define TRACELOG_SET_MEMORY_CLIENTS(mem, BLOCK) tracelog_SetMemoryHwBlock((mem), tracelog_MemoryHwBlock_e##BLOCK)


typedef enum tracelog_MemoryHwBlock {
#define BCHP_P_MEMC_DEFINE_HWBLOCK(block) tracelog_MemoryHwBlock_e##block,
#include "memc/bchp_memc_hwblock.h"
#undef BCHP_P_MEMC_DEFINE_HWBLOCK
    tracelog_MemoryHwBlock_eMax
} tracelog_MemoryHwBlock;

typedef enum tracelog_IoClient {
#define BCHP_P_IO_DEFINE_CLIENT(client, id)  tracelog_IoClient_e##client=id,
#include "bus/bchp_io_clients_chip.h"
#undef BCHP_P_IO_DEFINE_CLIENT
    tracelog_IoClient_eUnused
} tracelog_IoClient;

typedef enum tracelog_IoHwBlock {
#define BCHP_P_IO_DEFINE_HWBLOCK(block) tracelog_IoHwBlock_e##block,
#include "bus/bchp_io_hwblock.h"
#undef BCHP_P_IO_DEFINE_HWBLOCK
    tracelog_IoHwBlock_eMax
} tracelog_IoHwBlock;


BDBG_FILE_MODULE(TRACELOG);

#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
static bool tracelog_Supported(void)
{
    return true;
}
#include "bchp_memc_tracelog_0_0.h"
#else
static bool tracelog_Supported(void)
{
    return false;
}
#endif


static BERR_Code tracelog_P_GetOffset(unsigned memc, unsigned *offset)
{
    switch(memc) {
#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    case 0: *offset = 0; break;
#endif
#ifdef BCHP_MEMC_TRACELOG_0_1_REG_START
    case 1: *offset = BCHP_MEMC_TRACELOG_0_1_REG_START - BCHP_MEMC_TRACELOG_0_0_REG_START; break;
#endif
#ifdef BCHP_MEMC_TRACELOG_0_2_REG_START
    case 2: *offset = BCHP_MEMC_TRACELOG_0_2_REG_START - BCHP_MEMC_TRACELOG_0_0_REG_START; break;
#endif
    default:
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    return BERR_SUCCESS;
}

static void tracelog_GetDefaultIoFilter(tracelog_IoFilter *filter)
{
    BKNI_Memset(filter, 0, sizeof(*filter));
    filter->match.access = tracelog_AccessType_eWriteAndRead;
    return;
}

static void tracelog_GetDefaultMemoryFilter(tracelog_MemoryFilter *filter)
{
    BKNI_Memset(filter, 0, sizeof(*filter));
    filter->match.access = tracelog_AccessType_eWriteAndRead;
    filter->match.transactions.nondata = false;
    filter->match.transactions.group = true;
    filter->match.transactions.block32 = true;
    filter->match.transactions.block16 = true;
    filter->match.transactions.raster = true;
    filter->match.transactions.cache = true;
    filter->match.transactions.linear = true;
    return;
}

static BERR_Code tracelog_P_CheckFilter(unsigned filter)
{
    if(filter>=4) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    return BERR_SUCCESS;
}

#define tracelog_P_Read(r) BREG_Read32(reg, BCHP_MEMC_TRACELOG_0_0_##r + offset)
#define tracelog_P_Write(r,d) BREG_Write32(reg, BCHP_MEMC_TRACELOG_0_0_##r + offset, (d))
#define tracelog_P_ReadFilter(r)  BREG_Read32(reg, BCHP_MEMC_TRACELOG_0_0_FILTER_##r+ offset + filter * 4)
#define tracelog_P_WriteFilter(r,d)  BREG_Write32(reg, BCHP_MEMC_TRACELOG_0_0_FILTER_##r+ offset + filter * 4, (d))


static void tracelog_P_SetFilter(BREG_Handle reg, unsigned offset, unsigned filter, bool capture, bool trigger)
{
#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    uint32_t data;
    uint32_t capture_data;
    uint32_t trigger_data;
    uint32_t filter_bit;

    data = tracelog_P_Read(CONTROL);
    capture_data = BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_CONTROL, ENABLE_FILTERS_FOR_CAPTURE);
    trigger_data = BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_CONTROL, ENABLE_FILTERS_FOR_TRIGGER);
    filter_bit = 1<<filter;
    capture_data = capture_data & (~filter_bit);
    trigger_data = trigger_data & (~filter_bit);
    capture_data |= capture ? filter_bit : 0;
    trigger_data |= trigger ? filter_bit : 0;
    data &= ~(
              BCHP_MASK(MEMC_TRACELOG_0_0_CONTROL, ENABLE_FILTERS_FOR_CAPTURE) |
              BCHP_MASK(MEMC_TRACELOG_0_0_CONTROL, ENABLE_FILTERS_FOR_TRIGGER));
    data |=
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_CONTROL, ENABLE_FILTERS_FOR_CAPTURE, capture_data) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_CONTROL, ENABLE_FILTERS_FOR_TRIGGER, trigger_data);
    tracelog_P_Write(CONTROL, data);
#else
    BSTD_UNUSED(reg);
    BSTD_UNUSED(offset);
    BSTD_UNUSED(filter);
    BSTD_UNUSED(capture);
    BSTD_UNUSED(trigger);
#endif
    return;
}

static void tracelog_P_SetFilterMode(BREG_Handle reg, unsigned offset, unsigned filter, tracelog_FilterMode mode)
{
    tracelog_P_SetFilter(reg, offset, filter, mode!=tracelog_FilterMode_eTrigger, mode!=tracelog_FilterMode_eCapture);
    return;
}

static BERR_Code tracelog_Reset(BREG_Handle reg, unsigned memc)
{
    BERR_Code rc;
    unsigned offset;
    unsigned i;
    uint32_t data;

    rc = tracelog_P_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    for(i=0;i<4;i++) {
        tracelog_P_SetFilter(reg, offset, i, false, false);
    }
#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
#if defined(BCHP_MEMC_TRACELOG_0_0_CONTROL_CLOCK_GATE_MASK)
    data = tracelog_P_Read(CONTROL);
    BCHP_SET_FIELD_CONST_DATA(data, MEMC_TRACELOG_0_0_CONTROL, CLOCK_GATE, 0);
    tracelog_P_Write(CONTROL, data);
#endif
    data = BCHP_FIELD_CONST_DATA(MEMC_TRACELOG_0_0_COMMAND, CLEAR, 1);
    tracelog_P_Write(COMMAND, data);
#endif
    return BERR_SUCCESS;
}

static BERR_Code tracelog_DisableFilter(BREG_Handle reg, unsigned memc, unsigned filter)
{
    BERR_Code rc;
    unsigned offset;

    rc = tracelog_P_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    rc = tracelog_P_CheckFilter(filter);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    tracelog_P_SetFilter(reg, offset, filter, false, false);

    return BERR_SUCCESS;
}

static uint32_t tracelog_P_Read_Cleared_MEMC_TRACELOG_0_0_FILTER_MODE(BREG_Handle reg, unsigned offset, unsigned filter)
{
    uint32_t data = 0;
#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    data = tracelog_P_ReadFilter(MODEi_ARRAY_BASE);
    data &= ~(
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MEM_SIZE_EXCLUSIVE) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MEM_SIZE_ENABLE) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MEM_ACK_VALUE) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MEM_ACK_ENABLE) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_NONDATA) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_GROUP) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_BLOCK32) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_BLOCK16) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_RASTER) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_CACHE) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_LINEAR) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, IO_DATA_ENABLE ) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, IO_ACK_VALUE) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, IO_ACK_ENABLE) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, ADDRESS_EXCLUSIVE) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, ADDRESS_ENABLE) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_READS) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_WRITES) |
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_CLIENTS) |
#if defined(BCHP_MEMC_TRACELOG_0_0_FILTER_MODEi_SNOOP_UBUS_SHIFT)
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, SNOOP_UBUS) |
#elif defined(BCHP_MEMC_TRACELOG_0_0_FILTER_MODEi_SNOOP_SUB_SHIFT)
              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, SNOOP_SUB) |
#endif

              BCHP_MASK(MEMC_TRACELOG_0_0_FILTER_MODEi, SNOOP_IO_BUS));
#else
    BSTD_UNUSED(reg);
    BSTD_UNUSED(offset);
    BSTD_UNUSED(filter);
#endif
    return data;
}

static uint32_t tracelog_P_SetAccessType(uint32_t data, tracelog_AccessType access)
{
#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    data |=
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_READS, access==tracelog_AccessType_eWrite?1:0) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_WRITES, access==tracelog_AccessType_eRead?1:0);
#else
    BSTD_UNUSED(access);
#endif
    return data;
}

static void tracelog_P_SetAddress(BREG_Handle reg, unsigned offset, unsigned filter, uint64_t hi, uint64_t lo)
{
#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    tracelog_P_WriteFilter(ADDR_UPPERi_ARRAY_BASE, (uint32_t)hi);
    tracelog_P_WriteFilter(ADDR_UPPER_EXTi_ARRAY_BASE, (uint32_t)(hi>>32));
    tracelog_P_WriteFilter(ADDR_LOWERi_ARRAY_BASE, (uint32_t)lo);
    tracelog_P_WriteFilter(ADDR_LOWER_EXTi_ARRAY_BASE, (uint32_t)(lo>>32));
#else
    BSTD_UNUSED(reg);
    BSTD_UNUSED(offset);
    BSTD_UNUSED(filter);
    BSTD_UNUSED(hi);
    BSTD_UNUSED(lo);
#endif
    return;
}

static void tracelog_P_SetClientList(BREG_Handle reg, unsigned offset, unsigned filter, const tracelog_ClientList *list, bool exclusive)
{
#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    unsigned i;
    tracelog_P_WriteFilter(CLIENT_0_MASKi_ARRAY_BASE, exclusive ? list->bits[0] : ~list->bits[0]);
    tracelog_P_WriteFilter(CLIENT_1_MASKi_ARRAY_BASE, exclusive ? list->bits[1] : ~list->bits[1]);
    tracelog_P_WriteFilter(CLIENT_2_MASKi_ARRAY_BASE, exclusive ? list->bits[2] : ~list->bits[2]);
    tracelog_P_WriteFilter(CLIENT_3_MASKi_ARRAY_BASE, exclusive ? list->bits[3] : ~list->bits[3]);
    tracelog_P_WriteFilter(CLIENT_4_MASKi_ARRAY_BASE, exclusive ? list->bits[4] : ~list->bits[4]);
    tracelog_P_WriteFilter(CLIENT_5_MASKi_ARRAY_BASE, exclusive ? list->bits[5] : ~list->bits[5]);
    tracelog_P_WriteFilter(CLIENT_6_MASKi_ARRAY_BASE, exclusive ? list->bits[6] : ~list->bits[6]);
    tracelog_P_WriteFilter(CLIENT_7_MASKi_ARRAY_BASE, exclusive ? list->bits[7] : ~list->bits[7]);
#else
    BSTD_UNUSED(reg);
    BSTD_UNUSED(offset);
    BSTD_UNUSED(filter);
    BSTD_UNUSED(list);
    BSTD_UNUSED(exclusive);
#endif
    return;
}

static BERR_Code tracelog_EnableIoFilter(BREG_Handle reg, unsigned memc, unsigned filter, const tracelog_IoFilter *config, tracelog_FilterMode mode)
{
    BERR_Code rc;
    unsigned offset;
    uint32_t data;

    rc = tracelog_P_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    rc = tracelog_P_CheckFilter(filter);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    data = tracelog_P_Read_Cleared_MEMC_TRACELOG_0_0_FILTER_MODE(reg, offset, filter);
    data = tracelog_P_SetAccessType(data, config->match.access);
#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    data |=
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, IO_DATA_ENABLE, config->match.data.enabled?1:0) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, IO_ACK_VALUE, config->match.ack.value?1:0) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, IO_ACK_ENABLE, config->match.ack.enabled?1:0) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, ADDRESS_EXCLUSIVE, config->match.address.exclusive?1:0) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, ADDRESS_ENABLE, config->match.address.enabled?1:0 ) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_CLIENTS, config->match.clients.enabled?1:0);

    data |= BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, SNOOP_IO_BUS, 1);

    tracelog_P_WriteFilter(MODEi_ARRAY_BASE, data);

    tracelog_P_WriteFilter(IO_DATAi_ARRAY_BASE, config->match.data.data);
    tracelog_P_WriteFilter(IO_DATA_MASKi_ARRAY_BASE, config->match.data.mask);
#endif

    tracelog_P_SetFilterMode(reg, offset, filter, mode);
    tracelog_P_SetAddress(reg, offset, filter, config->match.address.high, config->match.address.low);
    tracelog_P_SetClientList(reg, offset, filter, &config->match.clients.list, config->match.clients.exclusive);

    return BERR_SUCCESS;
}

static BERR_Code tracelog_EnableMemoryFilter(BREG_Handle reg, unsigned memc, unsigned filter, const tracelog_MemoryFilter *config, tracelog_FilterMode mode)
{
    BERR_Code rc;
    unsigned offset;
    uint32_t data;

    rc = tracelog_P_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    rc = tracelog_P_CheckFilter(filter);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    data = tracelog_P_Read_Cleared_MEMC_TRACELOG_0_0_FILTER_MODE(reg, offset, filter);
    data = tracelog_P_SetAccessType(data, config->match.access);
#if !defined(BCHP_MEMC_TRACELOG_0_0_FILTER_MODEi_SNOOP_UBUS_SHIFT)
    if(config->snoop_ubus) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
#endif
#if !defined(BCHP_MEMC_TRACELOG_0_0_FILTER_MODEi_SNOOP_SUB_SHIFT)
    if(config->snoop_sub) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
#endif

#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    data |=
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MEM_SIZE_EXCLUSIVE, config->match.size.exclusive?1:0) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MEM_SIZE_ENABLE, config->match.size.enabled?1:0) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MEM_ACK_VALUE, config->match.ack.value?1:0) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MEM_ACK_ENABLE, config->match.ack.enabled?1:0) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_NONDATA, config->match.transactions.nondata?0:1) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_GROUP, config->match.transactions.group?0:1) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_BLOCK32, config->match.transactions.block32?0:1) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_BLOCK16, config->match.transactions.block16?0:1) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_RASTER, config->match.transactions.raster?0:1) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_CACHE, config->match.transactions.cache?0:1) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_MEM_LINEAR, config->match.transactions.linear?0:1) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, MASK_CLIENTS, config->match.clients.enabled?1:0) |
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, ADDRESS_EXCLUSIVE, config->match.address.exclusive?1:0) |
#if defined(BCHP_MEMC_TRACELOG_0_0_FILTER_MODEi_SNOOP_UBUS_SHIFT)
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, SNOOP_UBUS, config->snoop_ubus?1:0) |
#elif defined(BCHP_MEMC_TRACELOG_0_0_FILTER_MODEi_SNOOP_SUB_SHIFT)
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, SNOOP_SUB, config->snoop_sub?1:0) |
#endif
        BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, ADDRESS_ENABLE, config->match.address.enabled?1:0 );

    data |= BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_FILTER_MODEi, SNOOP_IO_BUS, 0);
    tracelog_P_WriteFilter(MODEi_ARRAY_BASE, data);

    data = tracelog_P_ReadFilter(MEM_SIZEi_ARRAY_BASE);
    BCHP_SET_FIELD_DATA(data, MEMC_TRACELOG_0_0_FILTER_MEM_SIZEi, SIZE_UPPER, config->match.size.high/32);
    BCHP_SET_FIELD_DATA(data, MEMC_TRACELOG_0_0_FILTER_MEM_SIZEi, SIZE_LOWER, config->match.size.low/32);
    tracelog_P_WriteFilter(MEM_SIZEi_ARRAY_BASE, data);
#endif

    tracelog_P_SetFilterMode(reg, offset, filter, mode);
    tracelog_P_SetAddress(reg, offset, filter, config->match.address.high, config->match.address.low);
    tracelog_P_SetClientList(reg, offset, filter, &config->match.clients.list, config->match.clients.exclusive);
    return BERR_SUCCESS;
}

static BERR_Code tracelog_P_SetClient(tracelog_ClientList *clients, unsigned id)
{
    unsigned width = 8*sizeof(clients->bits[0]);
    unsigned index = id / width;
    unsigned bit = id - index * width;

    if(index>=sizeof(clients->bits)/sizeof(clients->bits[0])) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    clients->bits[index] |= 1 << bit;
    return BERR_SUCCESS;
}


struct tracelog_P_MemoryClientEntry {
    const char *name;
    BCHP_MemcClient client;
    unsigned id;
};

static const struct tracelog_P_MemoryClientEntry tracelog_p_MemoryClientEntry[] = {
#if BCHP_P_MEMC_COUNT == 1
#define BCHP_P_MEMC_DEFINE_CLIENT(client,m0) {#client, BCHP_MemcClient_e##client, m0},
#elif BCHP_P_MEMC_COUNT == 2
#define BCHP_P_MEMC_DEFINE_CLIENT(client,m0,m1) {#client, BCHP_MemcClient_e##client, m0},
#elif BCHP_P_MEMC_COUNT == 3
#define BCHP_P_MEMC_DEFINE_CLIENT(client,m0,m1,m2) {#client, BCHP_MemcClient_e##client, m0},
#else
#error "not supported"
#endif
#include "memc/bchp_memc_clients_chip.h"
#undef  BCHP_P_MEMC_DEFINE_CLIENT
    {NULL,BCHP_MemcClient_eMax,0}
};

static const char *tracelog_P_GetMemcClientName(unsigned id)
{
    unsigned i;
    for(i=0;i<sizeof(tracelog_p_MemoryClientEntry)/sizeof(tracelog_p_MemoryClientEntry[0]) - 1;i++) {
        if(tracelog_p_MemoryClientEntry[i].id==id) {
            return tracelog_p_MemoryClientEntry[i].name;
        }
    }
    return NULL;
}

static BERR_Code tracelog_SetMemoryClient(tracelog_MemoryFilter *filter, BCHP_MemcClient client)
{
    unsigned i;
    for(i=0;i<sizeof(tracelog_p_MemoryClientEntry)/sizeof(tracelog_p_MemoryClientEntry[0]) - 1;i++) {
        if(tracelog_p_MemoryClientEntry[i].client==client) {
            filter->match.clients.enabled = true;
            return tracelog_P_SetClient(&filter->match.clients.list, tracelog_p_MemoryClientEntry[i].id);
        }
    }
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

static BERR_Code tracelog_SetMemoryHwBlock(tracelog_MemoryFilter *filter, tracelog_MemoryHwBlock hwBlock)
{
    static const struct {
        BCHP_MemcClient memcClient;
        tracelog_MemoryHwBlock hwBlock;
    } clients [] = {
#define BCHP_P_MEMC_DEFINE_CLIENT_MAP(client,block,svp_block) {BCHP_MemcClient_e##client, tracelog_MemoryHwBlock_e##block},
#include "memc/bchp_memc_clients_chip_map.h"
#undef BCHP_P_MEMC_DEFINE_CLIENT_MAP
        {BCHP_MemcClient_eMax, tracelog_MemoryHwBlock_eMax}
    };
    unsigned i;
    BERR_Code rc;
    bool set;

    for(i=0;i<sizeof(clients)/sizeof(clients[0]) - 1;i++) {
        if(clients[i].hwBlock ==  hwBlock) {
            BERR_Code  rc = tracelog_SetMemoryClient(filter, clients[i].memcClient);
            if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }
            set = true;
        }
    }
    if(!set) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    return BERR_SUCCESS;
}

struct tracelog_P_IoClientEntry {
    const char *name;
    unsigned id;
};

static const struct tracelog_P_IoClientEntry tracelog_P_IoClientEntry[] = {
#define BCHP_P_IO_DEFINE_CLIENT(client,id) {#client, id},
#include "bus/bchp_io_clients_chip.h"
#undef  BCHP_P_IO_DEFINE_CLIENT
    {NULL,0}
};

static const char *tracelog_P_GetRegisterClientName(unsigned id)
{
    int i;
    for(i=0;i<(int)(sizeof(tracelog_P_IoClientEntry)/sizeof(tracelog_P_IoClientEntry[0]) - 1);i++) {
        if(tracelog_P_IoClientEntry[i].id==id) {
            return tracelog_P_IoClientEntry[i].name;
        }
    }
    return NULL;
}

static BERR_Code tracelog_SetIoClient(tracelog_IoFilter *filter, tracelog_IoClient client)
{
    int i;
    for(i=0;i<(int)(sizeof(tracelog_P_IoClientEntry)/sizeof(tracelog_P_IoClientEntry[0]) - 1);i++) {
        if(tracelog_p_MemoryClientEntry[i].id==client) {
            filter->match.clients.enabled = true;
            return tracelog_P_SetClient(&filter->match.clients.list, client);
        }
    }
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

static BERR_Code tracelog_SetIoHwBlock(tracelog_IoFilter *filter, tracelog_IoHwBlock hwBlock)
{
    static const struct {
        tracelog_IoClient ioClient;
        tracelog_IoHwBlock hwBlock;
    } clients [] = {
#define BCHP_P_IO_DEFINE_CLIENT_MAP(client,block) {tracelog_IoClient_e##client, tracelog_IoHwBlock_e##block},
#include "bus/bchp_io_clients_chip_map.h"
#undef BCHP_P_IO_DEFINE_CLIENT_MAP
        {tracelog_IoClient_eUnused, tracelog_IoHwBlock_eMax}
    };
    int i;
    BERR_Code rc;
    bool set;

    for(i=0;i<(int)(sizeof(clients)/sizeof(clients[0]) - 1);i++) {
        if(clients[i].hwBlock ==  hwBlock) {
            BERR_Code  rc = tracelog_SetIoClient(filter, clients[i].ioClient);
            if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }
            set = true;
        }
    }
    if(!set) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    return BERR_SUCCESS;
}


static BERR_Code tracelog_GetStatus(BREG_Handle reg, unsigned memc, tracelog_Status *status)
{
    BERR_Code rc;
    unsigned offset;
    uint32_t data;

    rc = tracelog_P_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    data = tracelog_P_Read(STATUS);
    status->blocked.all = BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_STATUS, BLOCK_ALL);
    status->blocked.secure = BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_STATUS, BLOCK_SECURE);
    status->remote.trigger = BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_STATUS, REMOTE_TRIGGER_INPUT_VALUE);
    status->remote.active = BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_STATUS, REMOTE_ACTIVE_INPUT_VALUE);
    status->done = BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_STATUS, DONE);
    status->dropped = BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_STATUS, DROPPED);
    status->triggered = BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_STATUS, TRIGGERED);
    status->active = BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_STATUS, ACTIVE);

    data = tracelog_P_Read(ELAPSED_TIME_UPPER);
    status->time.elapsed = tracelog_P_Read(ELAPSED_TIME_LOWER);
    status->time.elapsed |= (((uint64_t) BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_ELAPSED_TIME_UPPER, TIME_UPPER))<<32);

    status->events.total = tracelog_P_Read(COUNT_TRANS_TOTAL);
    status->events.trigger = tracelog_P_Read(COUNT_TRIGGER_EVENTS);
    status->events.matches = tracelog_P_Read(COUNT_MATCHES_TOTAL);
    status->events.dropped = tracelog_P_Read(COUNT_MATCHES_DROPPED);
    status->events.preTrigger = tracelog_P_Read(COUNT_MATCHES_PRE_TRIGGER);
    status->events.postTrigger = tracelog_P_Read(COUNT_MATCHES_POST_TRIGGER);
#endif

    return rc;
}

static void tracelog_PrintStatus(const tracelog_Status *status)
{
    BDBG_MODULE_LOG(TRACELOG, ("Status(%s%s%s%s%s%s%s%s) elapsed:" BDBG_UINT64_FMT "",
              status->blocked.all?"BLOCKED," : "", status->blocked.secure?"SECURITY[BLOCKED] ":"", status->remote.trigger?"REMOTE[TRIGGER],":"",
              status->remote.active?"REMOTE[ACTIVE],":"", status->done?"DONE ":"", status->dropped?"DROPPED ":"",
              status->triggered?"TRIGGERED ":"", status->active?"ACTIVE ":"", BDBG_UINT64_ARG(status->time.elapsed)));
    BDBG_MODULE_LOG(TRACELOG, ("Events(total:%u,trigger:%u,matches:%u,dropped:%u,pre trigger:%u, post trigger:%u)",
              (unsigned)status->events.total, (unsigned)status->events.trigger, (unsigned)status->events.matches, (unsigned)status->events.dropped, (unsigned)status->events.preTrigger, (unsigned)status->events.postTrigger));
    return;
}



#define B_GET_BIT(w,b)  ((w)&(1<<(b)))
#define B_GET_BITS(w,e,b)  (((w)>>(b))&(((unsigned)(-1))>>((sizeof(unsigned))*8-(e+1-b))))

/* TraceLog Specification for MEMSYS
 *  5.  Transaction Buffer Formats */
static void  tracelog_P_ParseLog16(unsigned data, struct tracelog_P_Log16 *log)
{
    log->valid = B_GET_BIT(data, 15);
    log->postTrigger = B_GET_BIT(data, 14);
    log->lostEvents = B_GET_BIT(data, 13);
    log->io = B_GET_BIT(data, 11);
    log->write = B_GET_BIT(data, 9);
    log->id = B_GET_BITS(data, 7, 0);
    if(log->io) {
        log->data.io._64bit = B_GET_BIT(data, 12);
        log->data.io.errorAck = B_GET_BIT(data, 10);
    } else {
        log->data.memory.AFE_Swapped = B_GET_BIT(data, 12);
        log->data.memory.priorityArb = B_GET_BIT(data, 10);
        log->data.memory.aborted = B_GET_BIT(data, 8);
    }
    return;
}

static void  tracelog_P_ParseLog128(const uint32_t *data, struct tracelog_P_Log16 *log16, struct tracelog_P_Log128 *log128)
{
    uint32_t tmp;
    tracelog_P_ParseLog16(data[3], log16);
    if(!log16->valid) {
        return;
    }
    tmp = B_GET_BITS(data[3], 31, 15);
    log128->timestamp = data[2] | (((uint64_t)tmp)<<32);
    if(log16->io) {
        log128->data.io.addr = data[1];
        log128->data.io.data = data[0];
    } else {
        tmp = B_GET_BITS(data[0], 31, 24);
        log128->data.memory.addr = data[1] | (((uint64_t)tmp)<<32);
        log128->data.memory.accessId = B_GET_BITS(data[0], 23, 16);
        log128->data.memory.burstSize = B_GET_BITS(data[0],13, 8);
        log128->data.memory.field = B_GET_BIT(data[0],7);
        log128->data.memory.command = B_GET_BITS(data[0], 4, 0);
    }
    return;
}

static int tracelog_P_FormatLog16(char *buf, size_t size, const struct tracelog_P_Log16 *log)
{
    char io_mem[64];
    const char *client;
    io_mem[0]='\0';
    if(!log->valid) {
        return BKNI_Snprintf(buf, size, "Invalid");
    }
    if(log->io) {
        client = tracelog_P_GetRegisterClientName(log->id);
        BKNI_Snprintf(io_mem, sizeof(io_mem), "%s%s", log->data.io._64bit?"64 ":"", log->data.io.errorAck?"ERROR ":"");
    } else {
        client = tracelog_P_GetMemcClientName(log->id);
        BKNI_Snprintf(io_mem, sizeof(io_mem), "%s %s%s", log->data.memory.priorityArb?"PR":"RR", log->data.memory.AFE_Swapped?"AFE Swapped ":"", log->data.memory.aborted?"ABORTED ":"");
    }
    return BKNI_Snprintf(buf, size, "%s %s(%s%s%u) %s", log->write?"WR":"RD", log->io?"IO":"MEM", client?client:"", client?",":"", log->id, io_mem);
}

static int tracelog_P_FormatMemcSizeAndComamnd(char *buf, size_t size, uint8_t command, uint8_t burstSize)
{
    const char *name;
    bool linear = false;
    bool non_data = false;
    switch(command) {
    case 1: name="Linear";linear=true; break;
    case 2: name="Cache"; linear=true; break;
    case 3: name="Block16";break;
    case 4: name="Block32";break;
    case 5: name="Reaster";break;
    case 6: name="Group";break;
    case 7: return BKNI_Snprintf(buf, size, "Non-Data");
    default: return BKNI_Snprintf(buf, size, "Undefined(%u) %u", (unsigned)command, (unsigned)burstSize);
    }
    if(linear) {
        return BKNI_Snprintf(buf, size, "%s(%u bytes)", name, burstSize*32);
    } else {
        return BKNI_Snprintf(buf, size, "%s(%u jwords)", name, burstSize);
    }
    return 0;
}

static int tracelog_P_FormatLog128(char *buf, size_t size, const struct tracelog_P_Log16 *log16, const struct tracelog_P_Log128 *log128)
{
    int rc;
    unsigned offset = 0;
    uint64_t addr;

    if(!log16->valid) {
        return tracelog_P_FormatLog16(buf, size, log16);
    }
    addr = log16->io ? log128->data.io.addr : log128->data.memory.addr;
    rc = BKNI_Snprintf(buf+offset, size-offset, "T:" BDBG_UINT64_FMT " A:" BDBG_UINT64_FMT " ", BDBG_UINT64_ARG(log128->timestamp), BDBG_UINT64_ARG(addr));
    if(rc<0) {
        return rc;
    } else if(rc+offset>=size) {
        return rc+offset;
    } else {
        offset += rc;
    }

    rc = tracelog_P_FormatLog16(buf+offset, size-offset, log16);
    if(rc<0) {
        return rc;
    } else if(rc+offset>=size) {
        return rc+offset;
    } else {
        offset += rc;
    }

    if(log16->io) {
        if(log16->write) {
            rc = BKNI_Snprintf(buf+offset, size-offset, "D: %#x", log128->data.io.data);
        } else {
            rc = 0;
        }
    } else {
        rc = tracelog_P_FormatMemcSizeAndComamnd(buf+offset, size-offset, log128->data.memory.command, log128->data.memory.burstSize);
        if(rc<0) {
            return rc;
        } else if(rc+offset>=size) {
            return rc+offset;
        } else {
            offset += rc;
        }
        rc = BKNI_Snprintf(buf+offset, size-offset, " ID:%u%s", log128->data.memory.burstSize, log128->data.memory.field?" FIELD ":"" );
    }
    if(rc<0) {
        return rc;
    } else if(rc+offset>=size) {
        return rc+offset;
    } else {
        offset += rc;
    }

    return offset;
}

static BERR_Code tracelog_P_ReadInternalLog(BREG_Handle reg, unsigned offset, unsigned log_offset, unsigned words, uint32_t *buffer)
{
    unsigned align;
    uint32_t data;
    unsigned i;
    unsigned jword = 32;
    BDBG_ASSERT(log_offset%sizeof(uint32_t)==0);

    align = log_offset % jword; /* JWORD read-out buffer */
    data = log_offset - align;
    if(align+words*sizeof(uint32_t) > jword) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    tracelog_P_Write(BUFFER_PTR, data);

    tracelog_P_Write(COMMAND, BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_COMMAND, READBACK, 1));
    for(i=0;i<words;i++) {
        buffer[i] = BREG_Read32(reg, BCHP_MEMC_TRACELOG_0_0_BUFFER_DATA_WD_0 + offset + i*sizeof(uint32_t) + align);
    }
#endif
    return BERR_SUCCESS;
}

static void tracelog_GetDefaultMemoryBuffer(tracelog_MemoryBuffer *buffer)
{
    BKNI_Memset(buffer, 0, sizeof(*buffer));
    return;
}

static BERR_Code tracelog_IterateLog(BREG_Handle reg, unsigned memc, const tracelog_MemoryBuffer *buffer, bool (*iterator)(void *, unsigned, unsigned, const struct tracelog_P_Log16 *, const struct tracelog_P_Log128 *),void *context)
{
    BERR_Code rc;
    unsigned offset;
    uint32_t beforeTriger;
    uint32_t afterTriger;
    uint32_t tmp;
    unsigned i;
    unsigned count;
    unsigned buffer_size = 1024; /* internal buffer size */
    bool external = false;
    unsigned max_entries;
    bool format16;
    tracelog_Status status;
    tracelog_MemoryBuffer defaultMemoryBuffer;

    if(buffer==NULL) {
        tracelog_GetDefaultMemoryBuffer(&defaultMemoryBuffer);
        buffer = &defaultMemoryBuffer;
    }

    tracelog_GetStatus(reg, memc, &status);
    tracelog_PrintStatus(&status);

    rc = tracelog_P_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    beforeTriger = tracelog_P_Read(COUNT_MATCHES_PRE_TRIGGER);
    afterTriger = tracelog_P_Read(COUNT_MATCHES_POST_TRIGGER);
    tmp = tracelog_P_Read(CONTROL);
    format16 = BCHP_GET_FIELD_DATA(tmp, MEMC_TRACELOG_0_0_CONTROL, FORMAT16) != 0;
    external = BCHP_GET_FIELD_DATA(tmp, MEMC_TRACELOG_0_0_CONTROL, BUFFER_DRAM) != 0;
    if(external){
        BSTD_DeviceOffset buffer_base;

        buffer_base = tracelog_P_Read(BUFFER_PTR);
        tmp = tracelog_P_Read(BUFFER_PTR_EXT);
        buffer_base |= ((uint64_t)tmp)<<32;
        buffer_size = tracelog_P_Read(BUFFER_SIZE);
        if(buffer->base != buffer_base || buffer_size != buffer->size) {
            BDBG_ERR(("Provided buffer " BDBG_UINT64_FMT " .. " BDBG_UINT64_FMT " doesn't match current HW buffer " BDBG_UINT64_FMT  ".." BDBG_UINT64_FMT "", BDBG_UINT64_ARG(buffer->base), BDBG_UINT64_ARG(buffer->base + buffer->size), BDBG_UINT64_ARG(buffer_base), BDBG_UINT64_ARG(buffer_base+buffer_size)));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        if(buffer->ptr==NULL) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    count = beforeTriger + afterTriger;
#else
    count = 0;
    format16 = true;
#endif
    if(format16) {
        max_entries = buffer_size/sizeof(uint32_t);
    } else {
        max_entries= buffer_size/(4*sizeof(uint32_t));
    }
    if(count>max_entries) {
        count = max_entries;
    }

    for(i=0;i<count;i++) {
        uint32_t data[4];
        struct tracelog_P_Log16 log16;
        struct tracelog_P_Log128 log128;

        if(external) {
            if(format16) {
                data[0] = ((uint32_t *)buffer->ptr)[i];
            } else {
                unsigned j;
                for(j=0;j<4;j++) {
                    data[j] = ((uint32_t *)buffer->ptr)[4*i+j];
                }
            }
        } else {
            if(format16) {
                rc = tracelog_P_ReadInternalLog(reg, 0, i*sizeof(data[0]), 1, data);
            } else {
                rc = tracelog_P_ReadInternalLog(reg, 0, i*sizeof(data), sizeof(data)/sizeof(data[0]), data);
            }
            if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }
        }
        if(format16) {
            tracelog_P_ParseLog16(data[0], &log16);
        } else {
            BKNI_Memset(&log128, 0, sizeof(log128));
            tracelog_P_ParseLog128(data, &log16, &log128);
        }
        if(!iterator(context, count, i, &log16, format16?NULL:&log128)) {
            break;
        }
    }
    return BERR_SUCCESS;
}

struct tracelog_P_PrintOneEntryState {
    bool trigger;
};

static  bool tracelog_P_PrintOneEntry(void *context, unsigned count, unsigned n, const struct tracelog_P_Log16 *log16, const struct tracelog_P_Log128 *log128)
{
    char buf[128];
    struct tracelog_P_PrintOneEntryState *state = context;

    BDBG_ASSERT(log16);
    BSTD_UNUSED(context);
    if(log16->postTrigger && !state->trigger) {
        state->trigger = true;
        BDBG_LOG(("TRACELOG:-=-=-= TRIGER =-=-=-"));
    }
    if(log128) {
        tracelog_P_FormatLog128(buf, sizeof(buf), log16, log128);
    } else {
        tracelog_P_FormatLog16(buf, sizeof(buf), log16);
    }
    BDBG_LOG(("TRACELOG:%u/%u %s", n, count, buf));
    if(!log16->valid) {
        /* return false; */
    }
    return true;
}


static BERR_Code tracelog_PrintLog(BREG_Handle reg, unsigned memc, const tracelog_MemoryBuffer *buffer)
{
    struct tracelog_P_PrintOneEntryState state;
    state.trigger = false;
    return tracelog_IterateLog(reg, memc, buffer, tracelog_P_PrintOneEntry, &state);
}


static void tracelog_GetDefaultStartSettings(tracelog_StartSettings *settings)
{
    BKNI_Memset(settings, 0, sizeof(*settings));
    tracelog_GetDefaultMemoryBuffer(&settings->buffer);
    settings->trigger.mode = tracelog_TriggerMode_eManual;
    settings->trigger.capture = tracelog_CaptureMode_eMidTrigger;
    return;
}

static BERR_Code tracelog_Start(BREG_Handle reg, unsigned memc, tracelog_StartSettings *settings)
{
    BERR_Code rc;
    unsigned offset;
    uint32_t data;
    tracelog_StartSettings defaultSettings;

    if(settings==NULL) {
        tracelog_GetDefaultStartSettings(&defaultSettings);
        settings = &defaultSettings;
    }

    rc = tracelog_P_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    data = tracelog_P_Read(TRIGGER_MODE);
    data &= ~(
              BCHP_MASK(MEMC_TRACELOG_0_0_TRIGGER_MODE, EVENT_MODE) |
              BCHP_MASK(MEMC_TRACELOG_0_0_TRIGGER_MODE, CAPTURE_MODE));

    data |= BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_TRIGGER_MODE, CAPTURE_MODE, settings->trigger.capture);
    switch(settings->trigger.mode) {
    case tracelog_TriggerMode_eManual:
        data |= BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_TRIGGER_MODE, EVENT_MODE, 0);
        break;
    case tracelog_TriggerMode_eEvents:
        if(settings->trigger.count==1) {
            data |= BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_TRIGGER_MODE, EVENT_MODE, 1);
        } else {
            data |= BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_TRIGGER_MODE, EVENT_MODE, 2);
        }
        break;
    case tracelog_TriggerMode_eEventsClose:
         data |= BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_TRIGGER_MODE, EVENT_MODE, 3);
         break;
    case tracelog_TriggerMode_eEventsFar:
         data |= BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_TRIGGER_MODE, EVENT_MODE, 4);
         break;
    case tracelog_TriggerMode_eDelayed:
        if(settings->trigger.time==0) {
            data |= BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_TRIGGER_MODE, EVENT_MODE, 7);
        } else {
            data |= BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_TRIGGER_MODE, EVENT_MODE, 5);
        }
        break;
    default:
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    tracelog_P_Write(TRIGGER_MODE, data);

    tracelog_P_Write(TRIGGER_COUNT, settings->trigger.count);

    data = (uint32_t)settings->trigger.time;
    tracelog_P_Write(TRIGGER_DELAY_TIME_LOWER, data);
    data = (uint32_t)(settings->trigger.time>>32);
    data = BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_TRIGGER_DELAY_TIME_UPPER,EVENT_TIME_UPPER, data);
    tracelog_P_Write(TRIGGER_DELAY_TIME_UPPER, data);
    data = tracelog_P_Read(STATUS);
    BCHP_SET_FIELD_CONST_DATA(data, MEMC_TRACELOG_0_0_STATUS, DROPPED, 1);
    BCHP_SET_FIELD_CONST_DATA(data, MEMC_TRACELOG_0_0_STATUS, TRIGGERED, 1);
    tracelog_P_Write(STATUS, data);

    data = tracelog_P_Read(CONTROL);
#if defined(BCHP_MEMC_MEMC_TRACELOG_0_0_CONTROL_CLOCK_GATE_MASK)
    BCHP_SET_FIELD_CONST_DATA(data, MEMC_TRACELOG_0_0_CONTROL, CLOCK_GATE, 0);
#endif
    BCHP_SET_FIELD_CONST_DATA(data, MEMC_TRACELOG_0_0_CONTROL, GISB_FULL_SNOOP, 0);
    if(settings->buffer.size==0) {
        BCHP_SET_FIELD_CONST_DATA(data, MEMC_TRACELOG_0_0_CONTROL, BUFFER_DRAM, 0);
    } else {
        uint32_t tmp;
        if(settings->buffer.base==9) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        if(settings->buffer.base%256 != 0) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        if(settings->buffer.size%256 != 0) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        tmp = (uint32_t)settings->buffer.base;
        tracelog_P_Write(BUFFER_PTR, tmp);
        tmp = (uint32_t)(settings->buffer.base >> 32);
        tracelog_P_Write(BUFFER_PTR_EXT, tmp);
        tracelog_P_Write(BUFFER_SIZE, settings->buffer.size);

        BCHP_SET_FIELD_CONST_DATA(data, MEMC_TRACELOG_0_0_CONTROL, BUFFER_DRAM, 1);
    }
    BCHP_SET_FIELD_CONST_DATA(data, MEMC_TRACELOG_0_0_CONTROL, FORMAT16, settings->compact?1:0);
    tracelog_P_Write(CONTROL, data);

    data = BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_COMMAND, START, 1);
    tracelog_P_Write(COMMAND, data);
#endif /* ifdef BCHP_MEMC_TRACELOG_0_0_REG_START */
    return BERR_SUCCESS;
}


static BERR_Code tracelog_Trigger(BREG_Handle reg, unsigned memc)
{
    BERR_Code rc;
    unsigned offset;
    uint32_t data;

    rc = tracelog_P_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    data = tracelog_P_Read(TRIGGER_MODE);
    if(BCHP_GET_FIELD_DATA(data, MEMC_TRACELOG_0_0_TRIGGER_MODE, EVENT_MODE) !=0) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    data = BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_COMMAND, TRIGGER, 1);
    tracelog_P_Write(COMMAND, data);
#endif
    return BERR_SUCCESS;
}

static BERR_Code tracelog_Stop(BREG_Handle reg, unsigned memc)
{
    BERR_Code rc;
    unsigned offset;
    uint32_t data;

    rc = tracelog_P_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

#ifdef BCHP_MEMC_TRACELOG_0_0_REG_START
    data = BCHP_FIELD_DATA(MEMC_TRACELOG_0_0_COMMAND, STOP, 1);
    tracelog_P_Write(COMMAND, data);
#endif
    return BERR_SUCCESS;
}
