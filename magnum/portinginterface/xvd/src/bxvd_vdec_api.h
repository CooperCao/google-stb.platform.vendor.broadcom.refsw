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
 ***************************************************************************/

/* vdec_api.h
 * AVD API
 */

#ifndef  __INC_BXVD_VDEC_API_H__
#define  __INC_BXVD_VDEC_API_H__

#include "bxvd_platform.h"

#define BXVD_P_MAX_HST_CMDQ_SIZE       (64)

#define BXVD_P_DBGLOG_RD_PTR           (0)
#define BXVD_P_DBGLOG_WR_PTR           (4)
#define BXVD_P_DBGLOG_BUFFER_OVERFLOW  (0x80000000)
#define BXVD_P_DBGLOG_ITEM_SIZE        (4)
#define BXVD_P_DBGLOG_INITIAL_INDEX    (2)

/*
Summary:
This is the basic format of all command issued to the AVC
*/
typedef struct
{
      uint32_t  cmd;
      uint32_t  params[BXVD_P_MAX_HST_CMDQ_SIZE-2];
} BXVD_Cmd, BXVD_Rsp;

/* common response structure */
typedef struct
{
      uint32_t             ulCmd;
      uint32_t             ulStatus;
} BXVD_P_RspGeneric;


/* Decoder context structure */
typedef struct
{
      BINT_CallbackHandle pCbAVC_MBX_ISR;         /* AVC Mailbox L2 ISR */
      BINT_CallbackHandle pCbAVC_PicDataRdy_ISR;  /* AVC Picture Data Ready Display 0 L2 ISR */
      BINT_CallbackHandle pCbAVC_PicDataRdy1_ISR; /* AVC Picture Data Ready Display 1 L2 ISR */
      BINT_CallbackHandle pCbAVC_StillPicRdy_ISR; /* AVC Still Picture L2 ISR */
      BINT_CallbackHandle pCbAVC_Watchdog_ISR;    /* AVC Watchdog L2 ISR */
      BINT_CallbackHandle pCbAVC_VICHReg_ISR;     /* AVC VICH Register L2 ISR */
      BINT_CallbackHandle pCbAVC_VICHSCB_ISR;     /* AVC VICH SCB L2 ISR */
      BINT_CallbackHandle pCbAVC_VICHInstrRd_ISR; /* AVC VICH instruction read L2 ISR */
      BINT_CallbackHandle pCbAVC_VICHILInstrRd_ISR; /* AVC VICH IL instruction read L2 ISR */
      BINT_CallbackHandle pCbAVC_VICHBLInstrRd_ISR; /* AVC VICH BL instruction read L2 ISR */
      BINT_CallbackHandle pCbAVC_StereoSeqError_ISR; /* AVC StereoSeqError L2 ISR */
      BKNI_EventHandle    hFWCmdDoneEvent;        /* FW Command handshake done event */
      bool                bIfBusy;                /* TRUE if awaiting AVC response */
      bool                bInitialized;           /* Initialized already? */
      BXVD_Handle         hXvd;                   /* Pointer back to XVD command structure */
      unsigned long       ulCmdBufferAddr;    /* addr of cmd buffer */
      unsigned long       ulRspBufferAddr;    /* addr of rsp buffer */
} BXVD_DecoderContext;

#define  BXVD_CMD_INITIALIZE         (0x73760001)
#define  BXVD_CMD_CHANNELOPEN        (0x73760002)
#define  BXVD_CMD_CHANNELCLOSE       (0x73760003)
#define  BXVD_CMD_CHANNELSPEED       (0x73760004)
#define  BXVD_CMD_CHANNELSTART       (0x73760005)
#define  BXVD_CMD_CHANNELSTOP        (0x73760006)
#define  BXVD_CMD_CONFIG             (0x73760007)
#define  BXVD_CMD_DBGLOGCONTROL      (0x73760008)
#define  BXVD_CMD_DBGLOGCOMMAND      (0x73760009)
#define  BXVD_CMD_DRAMPERF           (0x7376000a)

#if BXVD_P_FW_HIM_API
#define  BXVD_CMD_RESPONSE           (0x80000000)
#else
#define  BXVD_CMD_RESPONSE           (0x00000000)
#endif

/* Bit0 is used for picture interrupt. */
#define  BXVD_INTERRUPT_PICTURE      (0x00000001)
#define  BXVD_INTERRUPT_USER_DATA    (0x00000002)
#define  BXVD_INTERRUPT_DISP_MGR     (0x00000004) /* used only for 7401 */
#define  BXVD_INTERRUPT_STILL_PIC    (0x00000008) /* used only for 7401 */
#define  BXVD_INTERRUPT_MAILBOX      (0x00000800) /* used only for 7401 */

/* Error codes */
enum
{
   BXVD_ErrUnknownCommand = -1,

   BXVD_ErrInvalidChannelId = 1,
   BXVD_ErrInvalidParameter,
   BXVD_ErrInvalidMemoryConfig,
   BXVD_ErrTestFailed,
   BXVD_ErrUnableToRunTest,
   BXVD_ErrNoChannels,
   BXVD_ErrInsufficientResources,
   BXVD_ErrInvalidPortNum,
   BXVD_ErrChanInuse
};

/*------------------------------------------------------*
 *    Initialize                                        *
 *------------------------------------------------------*/

#define VDEC_INIT_STRIPE_WIDTH_64       0       /* stripe_width=64 bytes  */
#define VDEC_INIT_STRIPE_WIDTH_128      1       /* stripe_width=128 bytes */
#define VDEC_INIT_STRIPE_WIDTH_256      2       /* stripe_width=256 bytes */

#define VDEC_INIT_SENTINEL_NOT_SUPPORTED 0xffffffff /* If MEMC doesn't have sentinel */

typedef struct
{
   uint32_t  command; /* 0x73760001 */
   uint32_t  cdb_little_endian;
   uint32_t  stripe_width; /* 0=>64, 1=>128 */
   uint32_t  stripe_height;
   uint32_t  bvnf_intr_context_base;
   uint32_t  host_L2_intr_set;
   uint32_t  chip_prod_revision;
   uint32_t  rave_context_reg_size;
   uint32_t  rave_cx_hold_clr_status;
   uint32_t  rave_packet_count;
   uint32_t  bvnf_intr_context_1_base;
   uint32_t  memc_sentinel_reg_start;
} BXVD_Cmd_Initialize;


typedef struct
{
   uint32_t  command;
   uint32_t  status;
   uint32_t  sw_version;
#if BXVD_P_FW_HIM_API
   uint32_t  display_info_0_offset;
   uint32_t  display_info_1_offset;
   uint32_t  display_info_2_offset;
#else
   uint32_t  dms_delivery_address0;
   uint32_t  dms_delivery_address1;
#endif
} BXVD_Rsp_Initialize;


/*------------------------------------------------------*
 *    ChannelOpen                                       *
 *------------------------------------------------------*/

#define BXVD_P_AVD_DECODE_RES_4K 0x100

typedef struct
{
   uint32_t  command; /* 0x73760002 */
   uint32_t  channel_number;
   uint32_t  max_resolution_enum;
   uint32_t  still_picture_mode;
   uint32_t  context_memory_base;
   uint32_t  context_memory_size;
   uint32_t  video_memory_base;
   uint32_t  video_block_size;
   uint32_t  video_block_count;
   uint32_t  cabac_memory_base;
   uint32_t  cabac_memory_size;
   uint32_t  bl_mv_store_base;
   uint32_t  bl_mv_store_size;
   uint32_t  reserved;
   uint32_t  cabac_wl_base;
   uint32_t  cabac_wl_size;
   uint32_t  direct_mode_storage_base;
   uint32_t  direct_mode_storage_size;
   uint32_t  il_wl_base;
   uint32_t  il_wl_size;
   uint32_t  bl_video_store_base;
   uint32_t  bl_video_store_size;
   uint32_t  chroma_memory_base;
   uint32_t  chroma_block_size;
} BXVD_Cmd_ChannelOpen;


#if BXVD_P_FW_HIM_API

typedef struct
{
   uint32_t  command;
   uint32_t  status;
   uint32_t  picture_delivery_buffer;
   uint32_t  picture_release_buffer;
   uint32_t  drop_count_byte_offset;
   uint32_t  avd_status_addr;
   uint32_t  shadow_write_byte_offset;
   uint32_t  delivery_q_read_byte_offset;
   uint32_t  delivery_q_write_byte_offset;
   uint32_t  release_q_read_byte_offset;
   uint32_t  release_q_write_byte_offset;
   uint32_t  bin_fullness_offset;
} BXVD_Rsp_ChannelOpen;

#else

typedef struct
{
   uint32_t  command;
   uint32_t  status;
   uint32_t  picture_delivery_buffer;
   uint32_t  picture_release_buffer;
   uint32_t  dm_return_address;
   uint32_t  avd_status_address;
} BXVD_Rsp_ChannelOpen;

#endif /* ~BXVD_P_FW_HIM_API */


/*------------------------------------------------------*
 *    ChannelClose                                      *
 *------------------------------------------------------*/

typedef struct
{
   uint32_t  command; /* 0x73760003 */
   uint32_t  channel_number;
} BXVD_Cmd_ChannelClose;

typedef struct
{
   uint32_t  command;
   uint32_t  status;
} BXVD_Rsp_ChannelClose;


/*------------------------------------------------------*
 *    ChannelSpeed                                      *
 *------------------------------------------------------*/

#define VDEC_SPEED_NORMAL       0       /* Normal playback                */
#define VDEC_SPEED_REFONLY      1       /* Decode reference pictures only */
#define VDEC_SPEED_IPONLY       2       /* Decode IP pictures only        */
#define VDEC_SPEED_IONLY        3       /* Decode I pictures only         */

typedef struct
{
   uint32_t  command; /* 0x73760004 */
   uint32_t  channel_number;
   uint32_t  speed;
} BXVD_Cmd_ChannelSpeed;

typedef struct
{
   uint32_t  command;
   uint32_t  status;
} BXVD_Rsp_ChannelSpeed;


/*------------------------------------------------------*
 *    ChannelStart                                      *
 *------------------------------------------------------*/

#define VDEC_CHANNEL_MODE_SPARSE_NORMAL               0x0000  /* Normal playback                      */
#define VDEC_CHANNEL_MODE_SPARSE_NOSKIP               0x0001  /* Decode IPB pictures, no skip         */
#define VDEC_CHANNEL_MODE_SPARSE_REFONLY              0x0002  /* Decode Ref pictures only             */
#define VDEC_CHANNEL_MODE_SPARSE_IPONLY               0x0003  /* Decode IP pictures only              */
#define VDEC_CHANNEL_MODE_SPARSE_IONLY                0x0004  /* Decode I pictures only               */
#define VDEC_CHANNEL_MODE_NON_LEGACY                  0x0008  /* Non Legacy mode enable               */
#define VDEC_CHANNEL_MODE_CLEAN_HITS                  0x0010  /* Clean HITS mode enable               */
#define VDEC_CHANNEL_MODE_HITS                        0x0020  /* HITS mode enable                     */
#define VDEC_CHANNEL_MODE_ZERO_DELAY                  0x0040  /* Zero delay output mode               */
#define VDEC_CHANNEL_MODE_BLU_RAY_DECODE              0x0080  /* Blu-ray decode mode                  */
#define VDEC_CHANNEL_MODE_TIMESTAMP_DISPLAY_ORDER     0x0100  /* Timestamp decode mode                */
#define VDEC_CHANNEL_MODE_IFRAME_AS_RAP               0x0200  /* Treat I-Frame as RAP for AVC         */
#define VDEC_CHANNEL_MODE_ENA_ERROR_CONCEALMENT       0x0400  /* Enable AVC error concealment         */
#define VDEC_CHANNEL_MODE_ENA_IONLY_FIELD_OUTPUT      0x0800  /* Enable I Only Field output           */
#define VDEC_CHANNEL_MODE_DISABLE_DPB_OUTPUT_DELAY    0x1000  /* Ignore the DPB output delay syntax   */
#define VDEC_CHANNEL_MODE_ENA_SEI_FRAME_PACK          0x2000  /* Enable SEI Frame Packing             */
#define VDEC_CHANNEL_MODE_DISABLE_P_SKIP              0x4000  /* Disable P-Skip mode                  */
#define VDEC_CHANNEL_MODE_FILE_FORMAT                 0x8000  /* MKV File format hack h264            */
#define VDEC_CHANNEL_MODE_3D_SVC_DECODE              0x10000  /* Enable 3D SVC mode                   */
#define VDEC_CHANNEL_MODE_SW_COEF_AVC_DECODE         0x40000  /* Enable SW coefficient AVC decode     */
#define VDEC_CHANNEL_MODE_IGN_NUM_REORDR_FRM_ZERO    0x80000  /* Ignore num reorder frame equal zero  */
#define VDEC_CHANNEL_MODE_EARLY_PIC_DELIVERY        0x100000  /* Pre-mature picture delivery mode     */
#define VDEC_CHANNEL_MODE_ENA_UD_BTP                0x200000  /* Enable UD in BTP mode                */
#define VDEC_CHANNEL_MODE_NRT_ENABLE                0x400000  /* Enable NRT mode                      */
#define VDEC_CHANNEL_MODE_OUTPUT_ALL_10BIT_TO_8BIT 0x1000000  /* Enable 8 bit output for 10 bit data  */

#define BXVD_P_VEC_UNUSED 0xff

typedef struct
{
   uint32_t  command; /* 0x73760005 */
   uint32_t  protocol;
   uint32_t  channel_mode; /* See VDEC_CHANNEL_MODES defined above */
   uint32_t  vec_index;
   uint32_t  channel_number;

   uint32_t  rave_ctxt_base;
   uint32_t  rave_ctxt_base_ext;
} BXVD_Cmd_ChannelStart;

typedef struct
{
   uint32_t  command;
   uint32_t  status;
   uint32_t  frames_outstanding;
} BXVD_Rsp_ChannelStart;


/*------------------------------------------------------*
 *    ChannelStop                                       *
 *------------------------------------------------------*/

typedef struct
{
   uint32_t  command; /* 0x73760006 */
   uint32_t  channel_number;
} BXVD_Cmd_ChannelStop;

typedef struct
{
   uint32_t  command;
   uint32_t  status;
} BXVD_Rsp_ChannelStop;

/*------------------------------------------------------*
 *    ChannelConfig                                     *
 *------------------------------------------------------*/

typedef struct
{
   uint32_t  command; /* 0x73760007 */
   uint32_t  vec_index;
   uint32_t  interrupt_mask_0;
   uint32_t  interrupt_mask_1;
} BXVD_Cmd_Config;

typedef struct
{
   uint32_t  command;
   uint32_t  status;
} BXVD_Rsp_Config;

/*---------------------------------------------*
 *    Decoder Debug LOG Start/Stop Command     *
 *---------------------------------------------*/

typedef enum
{
   BXVD_DBGLOG_eSTOP = 0,
   BXVD_DBGLOG_eSTART = 1
} BXVD_DBG_LOG;

typedef struct
{
   unsigned int  command;  /* 0x73760008 */
   unsigned int  logStart;
   unsigned int  dbglog_memory_base; /* 4 byte aligned memory address */
   unsigned int  dbglog_memory_size; /* Size in bytes */

} BXVD_Cmd_DbgLogControl;

typedef struct
{
   unsigned int command;
   unsigned int status;
} BXVD_Rsp_DbgLogControl;


/*---------------------------------------------*
 *    Decoder DEBUG LOG Command                *
 *---------------------------------------------*/

typedef struct
{
   unsigned int  command;  /* 0x73760009 */
   char logCmd[40];
} BXVD_Cmd_DbgLogCommand;

typedef struct
{
   unsigned int command;
   unsigned int status;
} BXVD_Rsp_DbgLogCommand;

typedef struct
{
   unsigned int  command; /* 0x7376000a */
   unsigned int  ddr_stat_ctrl_reg;  /* MEMC_DDR_0/1_STAT_CONTROL */
   unsigned int  ddr_stat_ctrl_val;  /* This will have clock_gate set 0,
                                        and bits 07: 00 set to the client id */

   unsigned int  ddr_stat_ctrl_enable; /* stat_enable bit set to one  in CTRL register*/
   unsigned int  ddr_stat_timer_reg;   /* MEMC_DDR_0/1_STAT_TIMER */

   /* Set of 4 registers to read */

   unsigned int  client_read;   /* MEMC_DDR_0/1_STAT_CLIENT_SERVICE_TRANS_READ */
   unsigned int  cas;           /* MEMC_DDR_0_STAT_CAS_CLIENT_126. This is for client id 126. If the client ID passed in is different
                                   then we need a different client ID here */
   unsigned int  intra_penalty; /* MEMC_DDR_0/1_STAT_CLIENT_SERVICE_INTR_PENALTY */
   unsigned int  post_penalty;  /* MEMC_DDR_0/1_STAT_CLIENT_SERVICE_POST_PENALTY */
} BXVD_Cmd_DramPerf;

typedef struct
{
   unsigned int  command;
   unsigned int  status;
} BXVD_Rsp_DramPerf;

typedef union
{
      BXVD_Cmd cmd;
      BXVD_Cmd_Initialize    init;
      BXVD_Cmd_ChannelOpen   channelOpen;
      BXVD_Cmd_ChannelClose  channelClose;
      BXVD_Cmd_ChannelSpeed  channelSpeed;
      BXVD_Cmd_ChannelStart  channelStart;
      BXVD_Cmd_ChannelStop   channelStop;
      BXVD_Cmd_Config        config;
      BXVD_Cmd_DbgLogControl dbgLogControl;
      BXVD_Cmd_DbgLogCommand dbgLogCommand;
      BXVD_Cmd_DramPerf      dramPerf;
} BXVD_FW_Cmd;

typedef union
{
      BXVD_Rsp rsp;
      BXVD_Rsp_Initialize    init;
      BXVD_Rsp_ChannelOpen   channelOpen;
      BXVD_Rsp_ChannelClose  channelClose;
      BXVD_Rsp_ChannelSpeed  channelSpeed;
      BXVD_Rsp_ChannelStart  channelStart;
      BXVD_Rsp_ChannelStop   channelStop;
      BXVD_Rsp_Config        config;
      BXVD_Rsp_DbgLogControl dbgLogControl;
      BXVD_Rsp_DbgLogCommand dbgLogCommand;
      BXVD_Rsp_DramPerf      dramPerf;
} BXVD_FW_Rsp;


#endif /*__INC_BXVD_VDEC_API_H__ */
