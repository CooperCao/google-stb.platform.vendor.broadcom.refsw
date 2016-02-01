/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 ***************************************************************************/


/*= Module Overview *********************************************************
<verbatim>

Overview
The MHL (Mobile High-Definition Link) API is a library used to
provide MHL functionality between Broadcom MHL Tx core and connected MHL receivers.

The API includes support for associated HDMI functionality such as parsing
of the EDID (Extended Display Identification Data) contained in the HDMI/DVI
monitors as well as support for HDCP (High-bandwidth Data Content Protection)

Additional support for reading CEC (Consumer Electronic Control) will also be
provided.


</verbatim>
****************************************************************************/

#ifndef BHDM_MHL_PRIV_H__
#define BHDM_MHL_PRIV_H__

#include "bstd.h"
#include "blst_queue.h"

#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "btmr.h"   	/* Timer Handle  */

#include "berr_ids.h"   /* Error codes */
#include "bdbg.h"       /* Debug Support */

#include "bhdm.h"
#include "bhdm_mhl.h"
#include "bchp_mpm_cpu_ctrl.h"
#include "bchp_int_id_cbus_intr2_0.h"
#include "bchp_int_id_cbus_intr2_1.h"
#include "bchp_int_id_mpm_host_l2.h"
#include "bchp_hif_cpu_intr1.h"
#include "bchp_hdmi_tx_intr2.h"
#include "bchp_hdmi_tx_phy.h"
#include "bchp_hdmi.h"
#include "bchp_aon_pin_ctrl.h"

#include "bhdm_mhl_const_priv.h"
#include "bhdm_mhl_common_priv.h"
#include "bhdm_mhl_debug_priv.h"
#include "bhdm_mhl_req_priv.h"
#include "bhdm_mhl_msc_resp_priv.h"
#include "bhdm_mhl_msc_req_priv.h"
#include "bhdm_mhl_ddc_req_priv.h"
#include "bhdm_mhl_cbus_priv.h"
#include "bhdm_mhl_cbus_cmd_priv.h"
#include "bhdm_mhl_mailbox_priv.h"
#include "bhdm_mhl_host_priv.h"


#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(HDMI_MHL);


/* Broadcom's adopter ID allocated by MHL LLC */
#define BHDM_P_MHL_BCM_ID                             1634

/* Some made up vendor ID */
#define BHDM_P_MHL_VENDOR_ID                          (0xd0)

/* Some made up device ID */
#define BHDM_P_MHL_DEVICE_ID                          (0xbeef)


#define TIMER_INTR_ENABLE_BIT (1 << 0)
#define TIMER_NOT_HALTED_BIT (1 << 1)

#if 0
#define ENABLE_HEARTBEAT	_sr(TIMER_INTR_ENABLE_BIT|TIMER_NOT_HALTED_BIT,ARC600_TIMER1_CONTROL)
#define DISABLE_HEARTBEAT	_sr(DISABLE,ARC600_TIMER1_CONTROL)

#define SETUP_HEARTBEAT_LIMIT	_sr(CBUS_HEARTBEAT_TICKS,ARC600_TIMER1_LIMIT)
#define RESET_HEARTBEAT     	_sr(0,ARC600_TIMER1_COUNT)
#endif

#if BHDM_P_MHL_ENABLE_HEARTBEAT_TIMER
#define ENABLE_HEARTBEAT
#define DISABLE_HEARTBEAT

#define SETUP_HEARTBEAT_LIMIT
#define RESET_HEARTBEAT
#endif

typedef struct BHDM_P_Mhl_Settings
{
	bool bInit;
} BHDM_P_Mhl_Settings;

typedef enum BHDM_P_MhlPowerLimit
{
  BHDM_P_MhlPowerLimit_e500mA,           /* Device delivers at least 500 mA */
  BHDM_P_MhlPowerLimit_e900mA,           /* Device delivers at least 900 mA */
  BHDM_P_MhlPowerLimit_e1500mA,          /* Device delivers at least 1.5 A */
  BHDM_P_MhlPowerLimit_e100mA            /* Device delivers at least 100 mA */
} BHDM_P_MhlPowerLimit;

/******************************************************************************
Summary:
Type for MHL device capabilities

Description:
The structure indicates the MHL capabilities of a MHL-capable Rx device.
This is typically used in conjunction with the EDID and power management.


See Also:
	o BHDM_P_Mhl_GetDeviceCapabilities
*******************************************************************************/
typedef struct BHDM_P_Mhl_DeviceCapabilities
{
	BHDM_P_MhlPowerLimit ePwrLimit;   /* Indicates the minimum VBUS power required */
	bool bSupportsRgb444;            /* Device supports RGB 444 */
	bool bSupportsYCbCr444;          /* Device supports YCbCr 444 */
	bool bSupportsYCbCr422;          /* Device supports YCbCr 422 */
	bool bSupportsPackedPixel;       /* Device supports Packed Pixel. Only applies
										to 1080p and 1080p50. */
	bool bSupportsDataIslands;       /* Device supports islands */
	bool bSupportsVga;               /* Device supports VGA */
} BHDM_P_Mhl_DeviceCapabilities;

/*******************************************************************************
Private HDMI MHL Object Declaration
*******************************************************************************/
typedef struct BHDM_P_Mhl_Object
{
	BDBG_OBJECT(HDMI_MHL)

	BHDM_P_Mhl_Settings stDeviceSettings;

	BCHP_Handle   hChip;
	BREG_Handle   hRegister;
	BINT_Handle   hInterrupt;

	BHDM_Handle   hHdmi;
	BTMR_Handle   hTmr;
	BTMR_TimerHandle   hMhlTimer;

	BINT_CallbackHandle hCbus0Callback[MAKE_MHL_CBUS_INTR_0_ENUM(LAST)];
	BINT_CallbackHandle hCbus1Callback[MAKE_MHL_CBUS_INTR_1_ENUM(LAST)];
	BINT_CallbackHandle hMpmHostCallback[MAKE_MHL_MPM_HOST_INTR_ENUM(LAST)];

	BKNI_EventHandle hEventStandby;

	BHDM_P_Mhl_MscReq_Handle  hMscReq;
	BHDM_P_Mhl_MscResp_Handle hMscResp;
	BHDM_P_Mhl_DdcReq_Handle  hDdcReq;


	/* CBUS state "registers" */
	BHDM_P_Mhl_CbusState stCbusState;

	BHDM_P_Mhl_HostCpuState hostState;

	/* Hardware interrupt status */
	BHDM_P_Mhl_CbusIntr    stCbusIntr;
	BHDM_P_Mhl_MpmHostIntr stMpmHostIntr;

	BHDM_P_Mhl_CbusPathEnSentState ePathEnSentState;
	uint8_t                        ucHostFcbVersion;

	/* Activity */
	bool bStopPending;      /* Host has requested stop */
	bool bStopReady;        /* We can hand over */

	bool bSinkPlimValid; /* If true Sink's PLIM from DCAP is ready */

	bool bScratchPadValid;

	bool bDisconnectReceived;

	bool bChipIsInS3;

	bool bVbusNotRequired;

	/* indicates if Host has completed it's S3-to-S0 process */
	bool bMhlLinkEstablished;

	/* Heartbeat states */
	bool bHeartbeatQueued; /* HB queued? */
	uint8_t ucHeartbeatDeadPkts; /* No. of dead HB packets */
	int  iHeartbeatTimer; /* Heartbeat interval */

	/* Timers */
	uint32_t ulHeartbeatCount;
	uint32_t ulTickCount;

	/* User defined heartbeat command */
	const BHDM_P_Mhl_CbusPkt *pstHbPkts;
	uint32_t ulNumHbPkts;
	uint32_t ulNumHbFailed;

	/* Colorspace and color depth */
	BHDM_Video_Settings   stVideoSettings;

	/* HDMI MHL CTL MODE */
	BHDM_P_Mhl_CtrlMode    eMhlCtrlMode;
	BHDM_P_Mhl_ClkMode     eMhlClkMode;

	/* Hotplug callback */
	BHDM_CallbackFunc pfHotplugChangeCallback;
	void *pvHotplugChangeParm1;
	int iHotplugChangeParm2;

	/* Standby callback */
	BKNI_EventHandle BHDM_MHL_EventStandby;
	BHDM_CallbackFunc pfStandbyCallback;
	void *pvStandbyParm1;
	int iStandbyParm2;

#if BHDM_MHL_CTS
	BHDM_P_Mhl_CbusDiscovery eDiscovery;
#endif

}BHDM_P_Mhl_Object;

typedef BHDM_P_Mhl_Object *BHDM_P_Mhl_Handle;

BERR_Code BHDM_P_Mhl_Create
	( BHDM_P_Mhl_Handle          *phMhl,
	  BCHP_Handle                 hChip,
	  BREG_Handle                 hRegister,
	  BINT_Handle                 hInterrupt,
	  BTMR_Handle                 hTimer,
	  const BHDM_P_Mhl_Settings  *pSettings );

BERR_Code BHDM_P_Mhl_Destroy
	( BHDM_P_Mhl_Handle     hMhl );

/* Initialisation */
BERR_Code BHDM_P_Mhl_Init
	( BHDM_P_Mhl_Handle     hMhl,
	  BHDM_Handle           hHdmi);

void BHDM_P_Mhl_EnableCbus0Interrupts
	( const BHDM_P_Mhl_Handle hMhl );


void BHDM_P_Mhl_EnableCbus1Interrupts
	( const BHDM_P_Mhl_Handle hMhl );

void BHDM_P_Mhl_DisableCbus0Interrupts
	( const BHDM_P_Mhl_Handle hMhl );

void BHDM_P_Mhl_DisableCbus1Interrupts
	( const BHDM_P_Mhl_Handle hMhl );

void BHDM_P_Mhl_HandleCbus0Interrupt_isr
	( void                 *pParam1,
	  int                   parm2 );

void BHDM_P_Mhl_HandleCbus1Interrupt_isr
	( void                 *pParam1,
	  int                   parm2 );

void BHDM_P_Mhl_EnableMpmHostInterrupts
	( const BHDM_P_Mhl_Handle hMhl );

void BHDM_P_Mhl_DisableMpmHostInterrupts
	( const BHDM_P_Mhl_Handle hMhl );

void BHDM_P_Mhl_HandleMpmHostInterrupt_isr
	( void                 *pParam1,
	  int                   parm2 );

void BHDM_P_Mhl_HandleHandoverToMpm_isr
	( BHDM_P_Mhl_Handle      hMhl );

void BHDM_P_Mhl_WakeupAndDiscovery_isr
	( BHDM_P_Mhl_Handle      hMhl );

BERR_Code BHDM_P_Mhl_ConfigLink
	( BHDM_P_Mhl_Handle      hMhl,
	  BFMT_VideoFmt          eVideoFmt,
	  BAVC_HDMI_BitsPerPixel eBpp );

void BHDM_P_Mhl_EnableMhlLink
	( BHDM_P_Mhl_Handle      hMhl,
	  BFMT_VideoFmt          eFormat );

BERR_Code BHDM_MHL_P_OpenMhl
	( BHDM_Handle            hHdm );

BERR_Code BHDM_MHL_P_CloseMhl
	( BHDM_Handle            hHdm );

BERR_Code BHDM_MHL_P_ConfigMhlLink
	( BHDM_Handle            hHdm );

BERR_Code BHDM_MHL_P_ConfigPreemphasis
	( const BHDM_Handle               hHdm,
	  const BHDM_Settings            *pstNewHdmiSettings,
	  BHDM_PreEmphasis_Configuration *pstNewPreEmphasisConfig);

void BHDM_MHL_P_EnableTmdsData_isr
	( const BHDM_Handle      hHdm );

void BHDM_MHL_P_ClearHpdState_isr
	( BHDM_Handle            hHdm );

void BHDM_MHL_P_GetSupportedVideoFormats
	( BHDM_Handle            hHdm,
	  bool                  *bMhlSupportedVideoFormats );

BERR_Code BHDM_MHL_P_ValidatePreferredFormat
	( BHDM_Handle            hHDMI,
	  BFMT_VideoFmt          eFmt );
#ifdef __cplusplus
}
#endif

#endif /* BHDM_MHL_PRIV_H__ */
