/***************************************************************************
 *     Copyright (c) 2007-2010, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/


#ifndef _BVSB_3548_H__
#define _BVSB_3548_H__				   

#include "bvsb.h"
#include "breg_i2c.h"

/***************************************************************************
Summary:
	Defines addresses in host config space

Description:
   The following macros define addresses of parameters in the host 
   configuration space.  These are passed into BVSB_WriteConfig/BVSB_ReadConfig
   functions 

See Also:
	None.

****************************************************************************/
#define BVSB_CONFIG_PAD_CTRL_1         0x0012 
#define BVSB_CONFIG_PAD_CTRL_2         0x0013 
#define BVSB_CONFIG_PAD_CTRL_3         0x0014 
#define BVSB_CONFIG_PAD_CTRL_4         0x0015 
#define BVSB_CONFIG_PAD_CTRL_5         0x0016 
#define BVSB_CONFIG_PAD_CTRL_6         0x0017 
#define BVSB_CONFIG_PIN_CTRL_1         0x0018
#define BVSB_CONFIG_PIN_CTRL_2         0x0019
#define BVSB_CONFIG_PIN_CTRL_3         0x001A
#define BVSB_CONFIG_PIN_CTRL_4         0x001B
#define BVSB_CONFIG_XPORT_CTRL         0x001C
#define BVSB_CONFIG_VSB_SYM_RATE       0x0020
#define BVSB_CONFIG_QAM_A_SYM_RATE     0x0024
#define BVSB_CONFIG_QAM_64B_SYM_RATE   0x0028
#define BVSB_CONFIG_QAM_256B_SYM_RATE  0x002C
#define BVSB_CONFIG_QAM_1024B_SYM_RATE 0x0030
#define BVSB_CONFIG_INBAND_IF_FREQ     0x0034
#define BVSB_CONFIG_ACQUIRE_MODE       0x0049
#define BVSB_CONFIG_ACQ_PARAM1         0x0050
#define BVSB_CONFIG_ACQ_PARAM2         0x0054
#define BVSB_CONFIG_ACQ_PARAM3         0x0058
#define BVSB_CONFIG_ACQ_PARAM4         0x005C
#define BVSB_CONFIG_ACQ_PARAM5         0x0060
#define BVSB_CONFIG_ACQ_PARAM6         0x0064
#define BVSB_CONFIG_ACQ_PARAM7         0x0068
#define BVSB_CONFIG_ACQ_PARAM8         0x006C
#define BVSB_CONFIG_ACQ_PARAM9         0x0070
#define BVSB_CONFIG_ACQ_PARAM10        0x0074
#define BVSB_CONFIG_ACQ_PARAM11        0x0078
#define BVSB_CONFIG_ACQ_PARAM12        0x007C
#define BVSB_CONFIG_ACQ_PARAM13        0x0080
#define BVSB_CONFIG_ACQ_PARAM14        0x0084
#define BVSB_CONFIG_ACQ_PARAM15        0x0088
#define BVSB_CONFIG_ACQ_PARAM16        0x008C
#define BVSB_CONFIG_ACQ_PARAM17        0x0090
#define BVSB_CONFIG_ACQ_PARAM18        0x0094
#define BVSB_CONFIG_ACQ_PARAM19        0x0098
#define BVSB_CONFIG_ACQ_PARAM20        0x009C
#define BVSB_CONFIG_ACQ_PARAM21        0x00A0
#define BVSB_CONFIG_ACQ_PARAM22        0x00A4
#define BVSB_CONFIG_ACQ_PARAM23        0x00A8
#define BVSB_CONFIG_ACQ_PARAM24        0x00AC
#define BVSB_CONFIG_ACQ_PARAM25        0x00B0
#define BVSB_CONFIG_ACQ_PARAM26        0x00B4
#define BVSB_CONFIG_ACQ_PARAM27        0x00B8
#define BVSB_CONFIG_ACQ_PARAM28        0x00BC
#define BVSB_CONFIG_ACQ_PARAM29        0x00C0
#define BVSB_CONFIG_ACQ_PARAM30        0x00C4
#define BVSB_CONFIG_ACQ_PARAM_BYTE1    0x00C8
#define BVSB_CONFIG_ACQ_PARAM_BYTE2    0x00C9
#define BVSB_CONFIG_ACQ_PARAM_BYTE3    0x00CA
#define BVSB_CONFIG_ACQ_PARAM_BYTE4    0x00CB
#define BVSB_CONFIG_ACQ_PARAM_BYTE5    0x00CC
#define BVSB_CONFIG_ACQ_PARAM_BYTE6    0x00CD
#define BVSB_CONFIG_ACQ_PARAM_BYTE7    0x00CE
#define BVSB_CONFIG_ACQ_PARAM_BYTE8    0x00CF
#define BVSB_CONFIG_VSB_CABLE_SETUP1   0x00D0
#define BVSB_CONFIG_VSB_CABLE_SETUP2   0x00D1
#define BVSB_CONFIG_VSB_CABLE_SETUP3   0x00D2
#define BVSB_CONFIG_VSB_CABLE_SETUP4   0x00D3
#define BVSB_CONFIG_VSB_CABLE_SETUP5   0x00D4
#define BVSB_CONFIG_VSB_CABLE_SETUP6   0x00D5
#define BVSB_CONFIG_VSB_CABLE_SETUP7   0x00D6
#define BVSB_CONFIG_VSB_CABLE_SETUP8   0x00D7
#define BVSB_CONFIG_VSB_CABLE_SETUP9   0x00D8
#define BVSB_CONFIG_VSB_CABLE_SETUP10  0x00D9
#define BVSB_CONFIG_VSB_CABLE_SETUP11  0x00DA
#define BVSB_CONFIG_VSB_CABLE_SETUP12  0x00DB
#define BVSB_CONFIG_VSB_TER_SETUP1     0x00DC
#define BVSB_CONFIG_VSB_TER_SETUP2     0x00DD
#define BVSB_CONFIG_VSB_TER_SETUP3     0x00DE
#define BVSB_CONFIG_VSB_TER_SETUP4     0x00DF
#define BVSB_CONFIG_VSB_TER_SETUP5     0x00E0
#define BVSB_CONFIG_VSB_TER_SETUP6     0x00E1
#define BVSB_CONFIG_VSB_TER_SETUP7     0x00E2
#define BVSB_CONFIG_VSB_TER_SETUP8     0x00E3
#define BVSB_CONFIG_VSB_TER_SETUP9     0x00E4
#define BVSB_CONFIG_VSB_TER_SETUP10    0x00E5
#define BVSB_CONFIG_VSB_TER_SETUP11    0x00E6
#define BVSB_CONFIG_VSB_TER_SETUP12    0x00E7
#define BVSB_CONFIG_QAM_CABLE_SETUP1   0x00E8
#define BVSB_CONFIG_QAM_CABLE_SETUP2   0x00E9
#define BVSB_CONFIG_QAM_CABLE_SETUP3   0x00EA
#define BVSB_CONFIG_QAM_CABLE_SETUP4   0x00EB
#define BVSB_CONFIG_QAM_CABLE_SETUP5   0x00EC
#define BVSB_CONFIG_QAM_CABLE_SETUP6   0x00ED
#define BVSB_CONFIG_QAM_CABLE_SETUP7   0x00EE
#define BVSB_CONFIG_QAM_CABLE_SETUP8   0x00EF
#define BVSB_CONFIG_QAM_CABLE_SETUP9   0x00F0
#define BVSB_CONFIG_QAM_CABLE_SETUP10  0x00F1
#define BVSB_CONFIG_QAM_CABLE_SETUP11  0x00F2
#define BVSB_CONFIG_QAM_CABLE_SETUP12  0x00F3
#define BVSB_CONFIG_QAM_TER_SETUP1     0x00F4
#define BVSB_CONFIG_QAM_TER_SETUP2     0x00F5
#define BVSB_CONFIG_QAM_TER_SETUP3     0x00F6
#define BVSB_CONFIG_QAM_TER_SETUP4     0x00F7
#define BVSB_CONFIG_QAM_TER_SETUP5     0x00F8
#define BVSB_CONFIG_QAM_TER_SETUP6     0x00F9
#define BVSB_CONFIG_QAM_TER_SETUP7     0x00FA
#define BVSB_CONFIG_QAM_TER_SETUP8     0x00FB
#define BVSB_CONFIG_QAM_TER_SETUP9     0x00FC
#define BVSB_CONFIG_QAM_TER_SETUP10    0x00FD
#define BVSB_CONFIG_QAM_TER_SETUP11    0x00FE
#define BVSB_CONFIG_QAM_TER_SETUP12    0x00FF
#define BVSB_CONFIG_NTSC_CABLE_SETUP1  0x0100
#define BVSB_CONFIG_NTSC_CABLE_SETUP2  0x0101
#define BVSB_CONFIG_NTSC_CABLE_SETUP3  0x0102
#define BVSB_CONFIG_NTSC_CABLE_SETUP4  0x0103
#define BVSB_CONFIG_NTSC_CABLE_SETUP5  0x0104
#define BVSB_CONFIG_CBL_IF_TOP         0x0104
#define BVSB_CONFIG_NTSC_CABLE_SETUP6  0x0105
#define BVSB_CONFIG_NTSC_CABLE_SETUP7  0x0106
#define BVSB_CONFIG_NTSC_CABLE_SETUP8  0x0107
#define BVSB_CONFIG_NTSC_CABLE_SETUP9  0x0108
#define BVSB_CONFIG_NTSC_CABLE_SETUP10 0x0109
#define BVSB_CONFIG_NTSC_CABLE_SETUP11 0x010A
#define BVSB_CONFIG_NTSC_CABLE_SETUP12 0x010B
#define BVSB_CONFIG_NTSC_TER_SETUP1    0x010C
#define BVSB_CONFIG_NTSC_TER_SETUP2    0x010D
#define BVSB_CONFIG_NTSC_TER_SETUP3    0x010E
#define BVSB_CONFIG_NTSC_TER_SETUP4    0x010F
#define BVSB_CONFIG_NTSC_TER_SETUP5    0x0110
#define BVSB_CONFIG_TER_IF_TOP         0x0110
#define BVSB_CONFIG_NTSC_TER_SETUP6    0x0111
#define BVSB_CONFIG_NTSC_TER_SETUP7    0x0112
#define BVSB_CONFIG_NTSC_TER_SETUP8    0x0113
#define BVSB_CONFIG_NTSC_TER_SETUP9    0x0114
#define BVSB_CONFIG_NTSC_TER_SETUP10   0x0115
#define BVSB_CONFIG_NTSC_TER_SETUP11   0x0116
#define BVSB_CONFIG_NTSC_TER_SETUP12   0x0117
#define BVSB_CONFIG_AGC_TEST1          0x0118
#define BVSB_CONFIG_PIX_CARRIER        0x0125
#define BVSB_CONFIG_WEAK_THRESHOLD_ON  0x016D
#define BVSB_CONFIG_WEAK_THRESHOLD_OFF 0x016E
#define BVSB_CONFIG_VIDEO_THRESHOLD_ON  0x016F
#define BVSB_CONFIG_VIDEO_THRESHOLD_OFF 0x0170

#define BVSB_CONFIG_IFD_CONFIG1			0x0171
#define BVSB_CONFIG_IFD_CONFIG2			0x0172
#define BVSB_CONFIG_IFD_CONFIG3			0x0173
#define BVSB_CONFIG_IFD_CONFIG4			0x0174
#define BVSB_CONFIG_IFD_CONFIG5			0x0175
#define BVSB_CONFIG_IFD_CONFIG6			0x0176
#define BVSB_CONFIG_IFD_CONFIG7			0x0177
#define BVSB_CONFIG_IFD_CONFIG8			0x0178

#define BVSB_CONFIG_SCAN_QAM_PARAM4		0x0188
#define BVSB_CONFIG_SCAN_AGF				0x018C
#define BVSB_CONFIG_SCAN_AGC				0x018E

#define BVSB_MODE_VSB 0xA
#define BVSB_MODE_QAM 0xB
#define BVSB_MODE_NTSC 0xD
/***************************************************************************
Summary:
	This function returns the default settings for 3548 VSB module.

Description:
	This function is responsible for returns the default setting for 
	3548 VSB module. The returning default setting should be used when
	opening the device.

Returns:
	TODO:

See Also:
	None.

****************************************************************************/
BERR_Code BVSB_3548_GetDefaultSettings(
	BVSB_Settings *		/* [in] Default settings */
);
/***************************************************************************
Summary:
	Structure for pad config of 3548/3560/35xx DTV chip

Description:
	This structure contains pad congrol configuration parameters.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_3548_PadConfig
{
   bool bOpenDrain_gm;
   bool bOpenDrain_om;
   bool bOpenDrain_im;
   bool bOpenDrain_am;
   uint8_t agcgpsel; 
   uint8_t agcisel; 
   uint8_t agctsel;
} BVSB_3548_PadConfig;
 

#ifdef __cplusplus
}
#endif

#endif /* BVSB_3548_H__ */

