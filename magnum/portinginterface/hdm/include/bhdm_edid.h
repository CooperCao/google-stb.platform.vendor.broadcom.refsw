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
 *
 * Module Description:
 *
 ***************************************************************************/
#ifndef BHDM_EDID_H__
#define BHDM_EDID_H__

#include "bhdm.h"
#include "bavc.h"

/*=************************ Module Overview ********************************
  The EDID (Enhanced Display Identification Data) functions provide support
  for reading/interpretting the EDID prom contained in the DVI/HDMI Receiver.

  The PROM has an I2C address of 0xA0.  These functions support the Enhanced
  DDC protocol as needed.
***************************************************************************/

/************************************************************************
Memory Requirements

The Memory Requirements
TBD
***************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#define BHDM_EDID_I2C_ADDR 0x50

#define BHDM_EDID_NOT_IMPLEMENTED         BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+  0)
#define BHDM_EDID_NOT_FOUND               BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+  1)
#define BHDM_EDID_CHECKSUM_ERROR          BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+  2)
#define BHDM_EDID_BLOCK_NOT_FOUND         BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+  3)
#define BHDM_EDID_DETAILTIMING_NOT_SUPPORTED  BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+  4)
#define BHDM_EDID_DESCRIPTOR_NOT_FOUND    BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+  5)
#define BHDM_EDID_INVALID_DESCRIPTOR_FOUND \
                                          BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+  6)
#define BHDM_EDID_MONITOR_RANGE_NOT_FOUND BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+  7)
#define BHDM_EDID_ERROR                   BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+  8)
#define BHDM_EDID_EXT_VERSION_NOT_SUPPORTED \
                                          BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+  9)
#define BERR_EDID_NOT_ENOUGH_MEM          BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+ 10)

#define BHDM_EDID_HDMI_VSDB_REGID_ERROR   BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+ 11)
#define BHDM_EDID_HDMI_UNKNOWN_CEA_TAG 	  BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+ 12)
#define BHDM_EDID_HDMI_UNKNOWN_BIT_RATE   BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+ 13)
#define BHDM_EDID_HDMI_AUDIO_FORMAT_UNSUPPORTED \
                                          BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+ 14)
#define BHDM_EDID_HDMI_VIDEO_FORMAT_UNSUPPORTED \
                                          BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+ 15)
#define BHDM_EDID_HDMI_NOT_SUPPORTED      BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+ 16)
#define BHDM_EDID_VIDEO_FORMATS_UNAVAILABLE \
                                          BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+ 18)
#define BHDM_EDID_AUDIO_FORMATS_UNAVAILABLE \
                                          BERR_MAKE_CODE(BERR_HDM_ID, BHDM_EDID_ERRS+ 19)







#define BHDM_EDID_BLOCKSIZE  128

#define BHDM_EDID_HEADER           0x00
#define BHDM_EDID_HEADER_SIZE      0x08

#define BHDM_EDID_VENDOR_ID        0x08
#define BHDM_EDID_PRODUCT_ID       0x0A
#define BHDM_EDID_SERIAL_NO        0x0C
#define BHDM_EDID_MANUFACTURE_WEEK 0x10
#define BHDM_EDID_MANUFACTURE_YEAR 0x11
#define BHDM_EDID_VERSION     0x12
#define BHDM_EDID_REVISION    0x13
#define BHDM_EDID_VIDEO_DEF        0x14
#define BHDM_EDID_MAX_HORIZ_SIZE   0x15
#define BHDM_EDID_MAX_VERT_SIZE    0x16
#define BHDM_EDID_GAMMA            0x17
#define BHDM_EDID_FEATURE_SUPPORT  0x18

#define BHDM_EDID_ESTABLISHED_TIMINGS1 0x23
	#define BHDM_EDID_ESTABLISHED_TIMINGS_1_800x600_60HZ   0x01
	#define BHDM_EDID_ESTABLISHED_TIMINGS_1_800x600_56HZ   0x02
	#define BHDM_EDID_ESTABLISHED_TIMINGS_1_640x480_75HZ   0x04
	#define BHDM_EDID_ESTABLISHED_TIMINGS_1_640x480_72HZ   0x08
	#define BHDM_EDID_ESTABLISHED_TIMINGS_1_640x480_67HZ   0x10
	#define BHDM_EDID_ESTABLISHED_TIMINGS_1_640x480_60HZ   0x20
	#define BHDM_EDID_ESTABLISHED_TIMINGS_1_720x400_88HZ   0x40
	#define BHDM_EDID_ESTABLISHED_TIMINGS_1_720x400_70HZ   0x80

#define BHDM_EDID_ESTABLISHED_TIMINGS2 0x24
	#define BHDM_EDID_ESTABLISHED_TIMINGS_2_1280x1024_75HZ 0x01
	#define BHDM_EDID_ESTABLISHED_TIMINGS_2_1024x768_75HZ  0x02
	#define BHDM_EDID_ESTABLISHED_TIMINGS_2_1024x768_70HZ  0x04
	#define BHDM_EDID_ESTABLISHED_TIMINGS_2_1024x768_60HZ  0x08
	#define BHDM_EDID_ESTABLISHED_TIMINGS_2_1024x768_87HZI 0x10
	#define BHDM_EDID_ESTABLISHED_TIMINGS_2_832x624_75HZ   0x20
	#define BHDM_EDID_ESTABLISHED_TIMINGS_2_800x600_75HZ   0x40
	#define BHDM_EDID_ESTABLISHED_TIMINGS_2_800x600_72HZ   0x80

#define BHDM_EDID_RESERVED_TIMINGS    0x25

#define BHDM_EDID_STD_TIMING_1        0x26
#define BHDM_EDID_STD_TIMING_2        0x28
#define BHDM_EDID_STD_TIMING_3        0x2A
#define BHDM_EDID_STD_TIMING_4        0x2C
#define BHDM_EDID_STD_TIMING_5        0x2E
#define BHDM_EDID_STD_TIMING_6        0x30
#define BHDM_EDID_STD_TIMING_7        0x32
#define BHDM_EDID_STD_TIMING_8        0x34

#define BHDM_EDID_MONITOR_DESC_1      0x36
#define BHDM_EDID_MONITOR_DESC_SIZE   18

	/* if descriptor is not DETAILED TIMING initial first bytes are:
	 *
	 *       0x00  0x01  0x02  0x03  0x04
	 *       ----  ----  ----  ----  ----
	 *
	 *		{0x00, 0x00, 0x00, 0xFF, 0x00}, // Monitor Serial Number
	 *		{0x00, 0x00, 0x00, 0xFE, 0x00}, // ASCII String
	 *		{0x00, 0x00, 0x00, 0xFD, 0x00}, // Monitor Range Limits
	 *		{0x00, 0x00, 0x00, 0xFC, 0x00}, // Monitor Name (ASCII)
	 *		{0x00, 0x00, 0x00, 0xFB, 0x00}, // Color Point Data
	 *		{0x00, 0x00, 0x00, 0xFA, 0x00}, // Additional Standard Timings
	 *				                        // F9 - 11 Unused
	 *			                            // 0F - 00 Manufacturer Descriptor
	 *		{0x00, 0x00, 0x00, 0x10, 0x00}, // Additional Standard Timings
	 */

	#define BHDM_EDID_DESC_TAG 0x03
			#define BHDM_EDID_TAG_MONITOR_NAME  0xFC
			#define BHDM_EDID_TAG_MONITOR_ASCII 0xFE  /* ASCII String */
			#define BHDM_EDID_TAG_MONITOR_SN    0xFF  /* Serial Number */

			#define BHDM_EDID_TAG_MONITOR_RANGE 0xFD
			#define BHDM_EDID_TAG_DUMMY_DESC 0x10	/* Dummy descriptor, indicate that the descriptor space is unsed */

	#define BHDM_EDID_DESC_DATA 0x05
	#define BHDM_EDID_DESC_ASCII_STRING_LEN 13
	#define BHDM_EDID_DESC_HEADER_LEN 0x05


	#define BHDM_EDID_DESC_PIXEL_CLOCK_LO  0x00
	#define BHDM_EDID_DESC_PIXEL_CLOCK_HI  0x01

	#define BHDM_EDID_DESC_HACTIVE_PIXELS_LSB   0x02
	#define BHDM_EDID_DESC_HBLANK_PIXELS_LSB    0x03
	#define BHDM_EDID_DESC_HACTIVE_PIXELS_UN_F0 0x04
	#define BHDM_EDID_DESC_HBLANK_PIXELS_UN_0F  0x04

	#define BHDM_EDID_DESC_VACTIVE_LINES_LSB   0x05
	#define BHDM_EDID_DESC_VBLANK_LINES_LSB    0x06
	#define BHDM_EDID_DESC_VACTIVE_LINES_UN_F0 0x07
	#define BHDM_EDID_DESC_VBLANK_LINES_UN_0F  0x07

	#define BHDM_EDID_DESC_HSYNC_OFFSET_LSB    0x08
	#define BHDM_EDID_DESC_HSYNC_WIDTH_LSB     0x09
	#define BHDM_EDID_DESC_VSYNC_OFFSET_LN_F0  0x0A
	#define BHDM_EDID_DESC_VSYNC_WIDTH_LN_0F   0x0A

	#define BHDM_EDID_DESC_HSYNC_OFFSET_U2_C0  0x0B
	#define BHDM_EDID_DESC_HSYNC_WIDTH_U2_30   0x0B
	#define BHDM_EDID_DESC_VSYNC_OFFSET_U2_0C  0x0B
	#define BHDM_EDID_DESC_VSYNC_WIDTH_U2_03   0x0B

	#define BHDM_EDID_DESC_HSIZE_MM_LSB        0x0C
	#define BHDM_EDID_DESC_VSIZE_MM_LSB        0x0D
	#define BHDM_EDID_DESC_HSIZE_UN_F0         0x0E
	#define BHDM_EDID_DESC_VSIZE_UN_0F         0x0E

	#define BHDM_EDID_DESC_HBORDER_PIXELS      0x0F
	#define BHDM_EDID_DESC_VBORDER_LINES       0x10
	#define BHDM_EDID_DESC_PREFERRED_FLAGS     0x11

	/* MONITOR RANGE DESCRIPTOR OFFSETS */
	#define BHDM_EDID_DESC_RANGE_MIN_V_RATE    0x05
	#define BHDM_EDID_DESC_RANGE_MAX_V_RATE    0x06
	#define BHDM_EDID_DESC_RANGE_MIN_H_RATE    0x07
	#define BHDM_EDID_DESC_RANGE_MAX_H_RATE    0x08
	#define BHDM_EDID_DESC_RANGE_MAX_PCLOCK    0x09

	#define EDID_DESC_TIMING_FORMULA      0x0A

#define BHDM_EDID_EXTENSION               0x7E
#define BHDM_EDID_CHECKSUM                0x7F   /* Checksum 7F */

#define BHDM_EDID_EXT_TAG            0x00
	#define BHDM_EDID_EXT_LCD_TIMINGS          0x01
	#define BHDM_EDID_EXT_TIMING_DATA 	       0x02
	#define BHDM_EDID_EXT_EDID_2_0             0x20
	#define BHDM_EDID_EXT_COLOR_INFO           0x30
	#define BHDM_EDID_EXT_DVI_FEATURE          0x40
	#define BHDM_EDID_EXT_TOUCH_SCREEN         0x50
	#define BHDM_EDID_EXT_BLOCK_MAP            0xF0
	#define BHDM_EDID_EXT_MANUFACTURER         0xFF

#define BHDM_EDID_MAX_BLOCK_MAP_EXTENSIONS	0xFF

#define BHDM_EDID_EXT_VERSION                  0x01
	#define BHDM_EDID_TIMING_VERSION_1             0x01
	#define BHDM_EDID_TIMING_VERSION_2             0x02
	#define BHDM_EDID_TIMING_VERSION_3             0x03
	#define BHDM_EDID_TIMING_VERSION_4             0x04
#define BHDM_EDID_EXT_DATA_OFFSET              0x02
#define BHDM_EDID_EXT_MONITOR_SUPPORT          0x03
#define BHDM_EDID_EXT_DATA_BLOCK_COLLECTION    0x04

#define BHDM_EDID_EXT_CHECKSUM                 0x7F

#define BHDM_EDID_MAX_CEA_VIDEO_ID_CODES 64




/******************************************************************************
Summary:
Data structure containing Detail Timing data contained in a monitor's EDID
PROM.

Description:
Each monitor has an embedded PROM at I2C address 0x50.  The data can contain
several detail timing descriptions which describe what timings the monitor
can support.  The timing data is parsed and stored in this structure.

See Also:
	o BHDM_EDID_GetDetailTiming
*******************************************************************************/
typedef struct BHDM_EDID_DetailTiming
{
   uint16_t PixelClock ;          /* Pixel Clock / 10000 */

   uint16_t HorizActivePixels ;	  /* Horizontal Active Pixels */
   uint16_t HorizBlankingPixels ; /* Horizontal Blanking Pixels */

   /* vertical arguments are lines, not pixels */
   /* keep old names for compilation purposes */
   uint16_t VerticalActiveLines ;	 /* Vertical Active Lines */
   uint16_t VerticalBlankingLines ; /* Vertical Blanking Lines */

   uint16_t VerticalActivePixels ;	 /* Vertical Active Pixels */
   uint16_t VerticalBlankingPixels ; /* Vertical Blanking Pixels */

   uint16_t HSyncOffset ;  /* Pixels, from blanking starts */
   uint16_t HSyncWidth ;   /* Horzontal Sync Pulse width */

   uint16_t VSyncOffset ;  /* Vertical Sync Offset */
   uint16_t VSyncWidth ;   /* Vertical Sync Pulse width */

   uint16_t HSize_mm ;     /* Horizontal Image Size (mm) */
   uint16_t VSize_mm ;     /* Vertical Image Size (mm) */

   uint16_t HorizBorderPixels ;   /* Horizinatal Border in Pixels */
   uint16_t VerticalBorderLines ; /* Vertical Border in Lines */

   uint16_t Mode ; /* Additional Flags: Interlaced, Stereo, Analog, etc. */
} BHDM_EDID_DetailTiming ;


/******************************************************************************
Summary:
Data structure containing Monitor Ranges data contained in a monitor's EDID
PROM.

Description:
Each monitor has an embedded PROM at I2C address 0x50.  The data must contain
ranges which specify the min and maximum timings. The timing data is parsed
and stored in this structure.

See Also:
	o BHDM_EDID_GetMonitorRange
*******************************************************************************/
typedef struct BHDM_EDID_MonitorRange
{
	uint16_t MinVertical ;    /* Minimum Vertical Rate (Hz) */
	uint16_t MaxVertical ;    /* Maximum Vertical Rate (Hz) */
	uint16_t MinHorizontal ;  /* Minimum Horizontal Rate (kHz) */
	uint16_t MaxHorizontal ;  /* Maximum Horizontal Rate (kHz) */
	uint16_t MaxPixelClock ;  /* Maximum Supported Pixel Clock Rate (MHz) */
	uint8_t SecondaryTiming ; /* Secondary Timing Formula (if supported) */
	uint8_t SecondaryTimingParameters[7] ;  /* Secondary Timing Formula Params */
} BHDM_EDID_MonitorRange  ;


/******************************************************************************
Summary:
Data structure containing basic data contained in the monitor's EDID PROM.

Description:
Each monitor has an embedded PROM at I2C address 0x50.  The data contained at
this address is parsed and stored in this structure.

See Also:
	o BHDM_EDID_GetBasicData
*******************************************************************************/
typedef struct BHDM_EDID_BasicData
{
   uint8_t VendorID[2] ;  /* Vendor ID from Microsoft; compressed ASCII */
   uint8_t ProductID[2] ; /* Product ID assigned by Vendor */
   uint8_t SerialNum[4] ; /* Serial # assigned by Vendor; may be undefined */
   uint8_t ManufWeek ;    /* Week of Manufture (1..53)          */
   uint16_t ManufYear ;   /* Year of Manufacture + 1990         */
   uint8_t EdidVersion ;  /* Version of Edid (version 1 or 2)   */
   uint8_t EdidRevision ; /* Revision of Edid 1.3 or 2.0        */
   uint8_t MaxHorizSize ; /* Max Horizontal Image size in cm.   */
   uint8_t MaxVertSize ;  /* Max Vertical Image size in cm.     */
   uint8_t Extensions ;   /* Number of 128 byte EDID extensions */

   uint8_t features;      /* Features are not supported; standby,suspend, etc */

	char monitorName[14];              /* NULL-terminated string for the monitor name */
	BHDM_EDID_MonitorRange MonitorRange ;
	BFMT_VideoFmt eVideoFmt ;

} BHDM_EDID_BasicData ;


/******************************************************************************
Summary:
Enumerated type containing the available descriptor tags that can be found in
a monitor's EDID PROM.

Description:
Each monitor has an embedded PROM at I2C address 0x50.  The PROM may contain
descriptors (name, serial number, etc.).  The available descriptor types are
specified in this enumeration type.

See Also:
	o BHDM_EDID_GetMonitorRange
*******************************************************************************/
typedef enum BHDM_EDID_Tag
{
	BHDM_EDID_Tag_eMONITOR_NAME,  /* Monitor Name stored as ASCII */
	BHDM_EDID_Tag_eMONITOR_ASCII, /* ASCII Text */
	BHDM_EDID_Tag_eMONITOR_SN     /* Monitor Serial Number stored as ASCII */
} BHDM_EDID_Tag ;


/* DEFINES for 3D_Structure_ALL */
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FRAME_PACKING 0x0001
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FIELD_ALTERNATIVE 0x0002
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LINE_ALTERNATIVE 0x0004
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_FULL	0x0008

#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LDEPTH	0x0010
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LDEPTH_GFX	0x0020
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_TOP_BOTTOM	0x0040
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_RESERVED_07  0x0080

#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_HALF_HORIZ	0x0100
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_RESERVED_09  0x0200
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_RESERVED_10  0x0400
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_RESERVED_11  0x0800

#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_RESERVED_12  0x1000
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_RESERVED_13  0x2000
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_RESERVED_14  0x4000
#define BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_HALF_QUINC	0x8000


/* DEFINES for 3D_Structure_X */
#define BHDM_EDID_VSDB_3D_STRUCTURE_X_FRAME_PACKING 0
#define BHDM_EDID_VSDB_3D_STRUCTURE_X_FIELD_ALTERNATIVE 1
#define BHDM_EDID_VSDB_3D_STRUCTURE_X_LINE_ALTERNATIVE 2
#define BHDM_EDID_VSDB_3D_STRUCTURE_X_SBS_FULL	3
#define BHDM_EDID_VSDB_3D_STRUCTURE_X_LDEPTH	4
#define BHDM_EDID_VSDB_3D_STRUCTURE_X_LDEPTH_GFX	5
#define BHDM_EDID_VSDB_3D_STRUCTURE_X_TOP_BOTTOM	6
#define BHDM_EDID_VSDB_3D_STRUCTURE_X_RESERVED  	7
#define BHDM_EDID_VSDB_3D_STRUCTURE_X_SBS_HALF 8


/* DEFINES for 3D_Ext_Data */
/* values of 0000, 0001, 0010, 0011 are all Horizontal sub_sampling */
#define BHDM_EDID_VSDB_3D_EXT_DATA_HORIZ_SUB_SAMPLING 3

/* values of 0100, 0101, 0110, 0111  are all Quincunx matrix */
#define BHDM_EDID_VSDB_3D_EXT_DATA_QUINCUNX_MATRIX 7

/* values 1000 ~ 1111 are reserved */
#define BHDM_EDID_VSDB_3D_EXT_DATA_HORIZ_RESERVED 8


typedef uint16_t BHDM_EDID_3D_Structure_ALL ;

/******************************************************************************
Summary:
Data structure containing HDMI parameters contained in the HDMI Receiver EDID
PROM.

Description:
Each monitor has an embedded PROM at I2C address 0x50.  The PROM may contain
information describing HDMI functionality supported by the monitor (audio, video
etc.)

See Also:
	o BHDM_EDID_IsRxDeviceHdmi
	o BHDM_EDID_CheckRxHdmiAudioSupport
	o BHDM_EDID_CheckRxHdmiVideoSupport
*******************************************************************************/
typedef struct BHDM_EDID_RxVendorSpecificDB
{
	bool valid ;
	/* Vendor Specific Data Block */
	uint8_t version ;
	uint8_t VsCode ;      /* Vendor Specific Tag Code = 3 */
	uint8_t ucpIEEE_RegId[3] ; /* Registration Identified 0x000C03 */


	uint8_t PhysAddr_A ;
	uint8_t PhysAddr_B ;
	uint8_t PhysAddr_C ;
	uint8_t PhysAddr_D ;

	bool SupportsAI ;
	bool DeepColor_30bit;
	bool DeepColor_36bit;
	bool DeepColor_48bit;
	bool DeepColor_Y444;
	bool DVI_Dual;

	uint16_t Max_TMDS_Clock_Rate;

	bool Latency_Fields_Present;
	bool Interlaced_Latency_Fields_Present;
	bool HDMI_Video_Present ;
	bool SupportedContentTypeGraphicsText ;
	bool SupportedContentTypePhoto ;
	bool SupportedContentTypeCinema ;
	bool SupportedContentTypeGame ;

	uint8_t Video_Latency;
	uint8_t Audio_Latency;
	uint8_t Interlaced_Video_Latency;
	uint8_t Interlaced_Audio_Latency;

	bool HDMI_3D_Present ;
	uint8_t HDMI_3D_Multi_Present ;

	uint8_t HDMI_Image_Size ;
	uint8_t HDMI_VIC_Len ;
	uint8_t HDMI_3D_Len ;

	/* Functions Supported */
	bool Underscan ;
	bool Audio ;
	bool YCbCr444 ;
	bool YCbCr422 ;

	uint8_t NativeFormatsInDescriptors ;

} BHDM_EDID_RxVendorSpecificDB ;


typedef struct BHDM_EDID_RxHfVsdb
{
	bool exists ;

	/* Vendor Specific Data Block */
	uint8_t VsCode ;      /* Vendor Specific Tag Code = 3 */
	uint8_t Version ;

	uint16_t Max_TMDS_Clock_Rate;

	bool _3D_OSD_Disparity ;
	bool DualView ;
	bool IndependentView ;
	bool LTE_340MScrambleSupport ; /* Sink supporst scrambling for pixel clocks < 340MHz */
	bool RRCapable ; /* Sink can initiate SCDC Read Request */
	bool SCDCSupport ;

	bool DeepColor_420_30bit ;
	bool DeepColor_420_36bit ;
	bool DeepColor_420_48bit ;

} BHDM_EDID_RxHfVsdb ;

/******************************************************************************
Summary:
Data structure containing HDMI Speaker Data Block informationcontained in the HDMI Receiver
EDID  PROM.


See Also:
	o BHDM_EDID_IsRxDeviceHdmi
	o BHDM_EDID_CheckRxHdmiAudioSupport
	o BHDM_EDID_CheckRxHdmiVideoSupport
*******************************************************************************/
typedef struct BHDM_EDID_SpeakerDB
{
	uint8_t uiSpeakerBlock ;
	bool bHasRLC_RRC ;
	bool bHasFLC_FRC ;
	bool bHasRC ;
	bool bHasRL_RR ;
	bool bHasFC ;
	bool bHasLFE ;
	bool bHasFL_FR ;
} BHDM_EDID_SpeakerDB ;


/******************************************************************************
Summary:
Enumeration type containing CEA data tags that are contained in Data Blocks
inside Version 3 Timing Extensions

Description:
Each monitor has an embedded PROM at I2C address 0x50.  The PROM may contain
information describing HDMI functionality supported by the monitor (audio, video
etc.)  These are the Data Block Tag IDs

See Also:
	o BHDM_EDID_IsRxDeviceHdmi
	o BHDM_EDID_CheckRxHdmiAudioSupport
	o BHDM_EDID_CheckRxHdmiVideoSupport
*******************************************************************************/
typedef enum BHDM_EDID_CeaDataBlockTag
{
	BHDM_EDID_CeaDataBlockTag_eReserved0,
	BHDM_EDID_CeaDataBlockTag_eAudioDB,
	BHDM_EDID_CeaDataBlockTag_eVideoDB,
	BHDM_EDID_CeaDataBlockTag_eVSDB,
	BHDM_EDID_CeaDataBlockTag_eSpeakerDB,
	BHDM_EDID_CeaDataBlockTag_eReserved5,
	BHDM_EDID_CeaDataBlockTag_eReserved6,
	BHDM_EDID_CeaDataBlockTag_eExtendedDB
} BHDM_EDID_CeaDataBlockTag ;


/******************************************************************************
Summary:
Enumeration type containing CEA Extended Data Block tags that are contained in Extended Data Blocks


Description:
Each monitor has an embedded PROM at I2C address 0x50.  The PROM may contain
information describing HDMI functionality supported by the monitor (audio, video
etc.)  These are the Data Block Tag IDs

See Also:
	o BHDM_EDID_CeaDataBlockTag
*******************************************************************************/
typedef enum BHDM_EDID_CeaExtendedDBTag
{
	BHDM_EDID_CeaExtendedDBTag_eVideoCapabilityDB,
	BHDM_EDID_CeaExtendedDBTag_eVendorSpecificVideoDB,
	BHDM_EDID_CeaExtendedDBTag_eVESADisplayDeviceDB,
	BHDM_EDID_CeaExtendedDBTag_eVESAVideoTimingBlockExtension,
	BHDM_EDID_CeaExtendedDBTag_eColorimetryDB = 5,
	BHDM_EDID_CeaExtendedDBTag_eHDRStaticMetaDB,
	BHDM_EDID_CeaExtendedDBTag_eVideoFormatPreferenceDB = 13,
	BHDM_EDID_CeaExtendedDBTag_eYCBCR420VideoDB,
	BHDM_EDID_CeaExtendedDBTag_eYCBCR420CapabilityMapDB,
	BHDM_EDID_CeaExtendedDBTag_eVendorSpecificAudioDB = 18,
	BHDM_EDID_CeaExtendedDBTag_eInfoFrameDB = 32
} BHDM_EDID_CeaExtendedDBTag ;


/******************************************************************************
Summary:
Type Definition containing CEA Short Audio Descriptor Information contained in
Data Blocks inside Version 3 Timing Extensions

Description:
CEA861B Short Audio Descriptors contain
	Audio Format
	Audio Channels
	Audio Bits or Bit Rate


See Also:
	o BHDM_EDID_GetRxHdmiAudioSupport
	o BHDM_EDID_CheckRxHdmiAudioSupport
*******************************************************************************/
typedef struct _BHDM_EDID_AUDIO_DESCRIPTOR
{
	bool Supported ;
	uint8_t AudioChannels ;
	uint8_t ucCeaSampleRates ;
	uint8_t ucCeaNBits_BitRate ;  /* number bits or bit rate */
} BHDM_EDID_AudioDescriptor ;


typedef enum _BHDM_EDID_COLORIMETRY_DB_EXTENDED_SUPPORT_
{
	BHDM_EDID_ColorimetryDbExtendedSupport_exvYCC601,
	BHDM_EDID_ColorimetryDbExtendedSupport_exvYCC709,
	BHDM_EDID_ColorimetryDbExtendedSupport_esYCC601,
	BHDM_EDID_ColorimetryDbExtendedSupport_eAdobeYCC601,
	BHDM_EDID_ColorimetryDbExtendedSupport_eAdobeRGB,
	BHDM_EDID_ColorimetryDbExtendedSupport_eBT2020cYCC,
	BHDM_EDID_ColorimetryDbExtendedSupport_eBT2020YCC,
	BHDM_EDID_ColorimetryDbExtendedSupport_eBT2020RGB,
	BHDM_EDID_ColorimetryDbExtendedSupport_eMax
}  BHDM_EDID_COLORIMETRY_DB_EXTENDED_SUPPORT ;


typedef enum _BHDM_EDID_COLORIMETRY_DB_METADATA_PROFILE_
{
	BHDM_EDID_ColorimetryDbMetadataProfile_eMD0,
	BHDM_EDID_ColorimetryDbMetadataProfile_eMD1,
	BHDM_EDID_ColorimetryDbMetadataProfile_eMD2,
	BHDM_EDID_ColorimetryDbMetadataProfile_eMD3,
	BHDM_EDID_ColorimetryDbMetadataProfile_eMax

} BHDM_EDID_COLORIMETRY_DB_METADATA_PROFILE ;


/******************************************************************************
Summary:
Type Definition containing CEA Colorimetry data contained in Colorimetry
Data Blocks inside Version 3 Timing Extensions

Description:
CEA861D Colorimetry Data Block contains
	xvYCC709 and xvYCC601 flags
	MetadataProfile0, MetadataProfile1 and MetadataProfile2 flags

*******************************************************************************/
typedef struct _BHDM_EDID_COLORIMETRY_DATA_BLOCK
{
	bool bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eMax] ;
	bool bMetadataProfile[BHDM_EDID_ColorimetryDbMetadataProfile_eMax];
} BHDM_EDID_ColorimetryDataBlock;


typedef enum _BHDM_EDID_VDCB_CE_BEHAVIOR_
{
	BHDM_EDID_VCDBCeBehavior_eNotSupported,
	BHDM_EDID_VCDBCeBehavior_eAlwaysOverscanned,
	BHDM_EDID_VCDBCeBehavior_eAlwaysUnderscanned,
	BHDM_EDID_VCDBCeBehavior_eOverUnderSupport

} BHDM_EDID_VDCB_CE_BEHAVIOR ;


typedef enum _BHDM_EDID_VDCB_IT_BEHAVIOR_
{
	BHDM_EDID_VCDBItBehavior_eNotSupported,
	BHDM_EDID_VCDBItBehavior_eAlwaysOverscanned,
	BHDM_EDID_VCDBItBehavior_eAlwaysUnderscanned,
	BHDM_EDID_VCDBItBehavior_eOverUnderSupport

} BHDM_EDID_VDCB_IT_BEHAVIOR ;


typedef enum _BHDM_EDID_VDCB_PT_BEHAVIOR_
{
	BHDM_EDID_VCDBPtBehavior_eNotSupported,
	BHDM_EDID_VCDBPtBehavior_eAlwaysOverscanned,
	BHDM_EDID_VCDBPtBehavior_eAlwaysUnderscanned,
	BHDM_EDID_VCDBPtBehavior_eOverUnderSupport

} BHDM_EDID_VDCB_PT_BEHAVIOR ;



typedef enum _BHDM_EDID_HDR_DB_EOTF_SUPPORT_
{
	BHDM_EDID_HdrDbEotfSupport_eSDR,
	BHDM_EDID_HdrDbEotfSupport_eHDR,
	BHDM_EDID_HdrDbEotfSupport_eSMPTESt2084,
	BHDM_EDID_HdrDbEotfSupport_eHLG,
	BHDM_EDID_HdrDbEotfSupport_eMax
}  BHDM_EDID_HDR_DB_EOTF_SUPPORT ;



typedef enum _BHDM_EDID_HDR_DB_STATIC_METADATA_SUPPORT_
{
	BHDM_EDID_HdrDbStaticMetadataSupport_eType1,
	BHDM_EDID_HdrDbStaticMetadataSupport_eMax
}  BHDM_EDID_HDR_DB_STATIC_METADATA_SUPPORT ;


typedef struct BHDM_EDID_HDRStaticDB
{
	bool valid ;

	bool bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eMax] ;
	bool bMetadataSupport[BHDM_EDID_HdrDbStaticMetadataSupport_eMax] ;

	uint8_t MinLuminance ;
	bool MinLuminanceValid ;

	uint8_t AverageLuminance ;
	bool AverageLuminanceValid ;

	uint8_t MaxLuminance ;
	bool MaxLuminanceValid ;

} BHDM_EDID_HDRStaticDB ;


/******************************************************************************
Summary:
Type Definition containing CEA Video Capabilities
The Video Capability Data Block is found inside Version 3 Timing Extensions

*******************************************************************************/
typedef struct _BHDM_EDID_VideoCapability_DATA_BLOCK
{
	bool valid ;
	BHDM_EDID_VDCB_CE_BEHAVIOR	eCeBehavior ;
	BHDM_EDID_VDCB_IT_BEHAVIOR eItBehavior ;
	BHDM_EDID_VDCB_PT_BEHAVIOR ePtBehavior ;
	bool bQuantizationSelectatbleRGB ;
	bool bQuantizationSelectatbleYCC ;
} BHDM_EDID_VideoCapabilityDataBlock;


/******************************************************************************
Summary:
Type definition for indicating extended xvYCC colorimetries supported by the
attached monitor.

Description:
Struct contains following (for now): xvYCC709 and xvYCC601 flags
Structure will be DEPRACATED; see BHDM_EDID_ColorimetryDataBlock

See Also:
	o BHDM_EDID_ColorimetryDataBlock
*******************************************************************************/
typedef struct _BHDM_EDID_EXTENDED_COLORIMETRY
{
	bool bxvYCC601;
	bool bxvYCC709;
} BHDM_EDID_ExtendedColorimetry ;



/******************************************************************************
Summary:
Type definition for indicating xvYCC colorimetries supported by the
attached monitor.

Description:
Struct contains following (for now):
	xvYCC709 and xvYCC601 flags

See Also:
*******************************************************************************/
typedef struct _BHDM_EDID_COLOR_DEPTH
{
	bool bColorDepth24bit;
	bool bColorDepth30bit;
	bool bColorDepth36bit;
	bool bColorDepth48bit;

} BHDM_EDID_ColorDepth;


/***************************************************************************
Summary:
	Holds related information of the attached receiver supported video formats.

See Also
	o BHDM_EDID_GetSupportedVideoInfo
****************************************************************************/
typedef struct _BHDM_EDID_VideoDescriptorInfo_
{
	uint8_t VideoIDCode[BHDM_EDID_MAX_CEA_VIDEO_ID_CODES] ;
	uint8_t numDescriptors;
} BHDM_EDID_VideoDescriptorInfo ;


/******************************************************************************
Summary:
Retrieve the Nth 128-byte EDID block (block could be from cache)

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	iNthBlock - Number of the EDID Block to return

	iBufSize - Size of the buffer to hold the EDID block...
	          should be 128 bytes

Output:
	pBuffer - pointer to previously allocated 128 byte buffer to hold the
	128-byte EDID block

Returns:
	BERR_SUCCESS              - EDID block retrieved
	BERR_INVALID_PARAMETER    - Invalid function parameter
	BHDM_EDID_NOT_ENOUGH_MEM  - Not enough memory to hold EDID block
	BHDM_EDID_NOT_FOUND       - No EDID Available
	BHDM_EDID_CHECKSUM_ERROR  - No EDID Checksum Error
	BHDM_EDID_BLOCK_NOT_FOUND - Requested Block is not available


See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_EDID_GetNthBlock(
   BHDM_Handle hHDMI,  /* [in] HDMI handle */
   uint8_t iNthBlock, /* [in] EDID Block Number to retrieve */
   uint8_t *pBuffer,   /* [out] pointer to input buffer */
   uint16_t iBufSize   /* [in] Size of input buffer to write EDID to */
) ;


/******************************************************************************
Summary:
Read the Nth 128-byte EDID block (direct from hardware)

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	iNthBlock - Number of the EDID Block to return

	iBufSize - Size of the buffer to hold the EDID block...
	          should be 128 bytes

Output:
	pBuffer - pointer to previously allocated 128 byte buffer to hold the
	128-byte EDID block

Returns:
	BERR_SUCCESS              - EDID block retrieved
	BERR_INVALID_PARAMETER    - Invalid function parameter
	BHDM_EDID_NOT_ENOUGH_MEM  - Not enough memory to hold EDID block
	BHDM_EDID_NOT_FOUND       - No EDID Available
	BHDM_EDID_CHECKSUM_ERROR  - No EDID Checksum Error
	BHDM_EDID_BLOCK_NOT_FOUND - Requested Block is not available


See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_EDID_ReadNthBlock(
   BHDM_Handle hHDMI,  /* [in] HDMI handle */
   uint8_t iNthBlock, /* [in] EDID Block Number to retrieve */
   uint8_t *pBuffer,   /* [out] pointer to input buffer */
   uint16_t iBufSize   /* [in] Size of input buffer to write EDID to */
) ;


/******************************************************************************
Summary:
Retrieve the basic EDID data

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pMonitorData - pointer to BHDM_EDID_BasicData structure to hold the
	Detail Timing data

Returns:
	BERR_SUCCESS            - Basic EDID Data successfully retrieved
	BERR_INVALID_PARAMETER  - Invalid function parameter.
	BHDM_EDID_NOT_FOUND      - No EDID Available
	BHDM_EDID_CHECKSUM_ERROR - EDID Checksum Error
	BHDM_EDID_BLOCK_NOT_FOUND	- EDID does not contain requested block

See Also:
	o BHDM_Handle
	o BHDM_EDID_BasicData

*******************************************************************************/
BERR_Code BHDM_EDID_GetBasicData(
   BHDM_Handle hHDMI,               /* [in] HDMI handle */
   BHDM_EDID_BasicData *pMonitorData /* [out] pointer to structure to hold Basic
                                            EDID Data */
) ;


/******************************************************************************
Summary:
Retrieve EDID Detail Timing Data

Description:
This function can be used to read the preferred Video Format from the EDID
of the attached monitor.


Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	NthTimingRequested - The number of the Detail Timing to retrieve


Output:
	pBHDM_EDID_DetailTiming - pointer to structure to hold the Detail Timing data
	eVideoFmt - the preferred Video Format in BFMT_xxxxx

Returns:
	BERR_SUCCESS                   - Detail  Timing Information retrieved
	BERR_INVALID_PARAMETER         - Invalid function parameter.
	BHDM_EDID_NOT_FOUND             - No EDID Available
	BHDM_EDID_CHECKSUM_ERROR        - EDID Checksum Error
	BHDM_EDID_DETAILTIMING_NOT_SUPPORTED - Requested Detail Timing is not supported.

See Also:
	o BHDM_Handle
	o BHDM_EDID_DetailTiming
	o BFMT_VideoFmt


*******************************************************************************/
BERR_Code BHDM_EDID_GetDetailTiming(
	BHDM_Handle hHDMI,			/* [in] HDMI handle */
	uint8_t NthTimingRequested, /* [in] number of the Detail Block to get */
	BHDM_EDID_DetailTiming
		*pBHDM_EDID_DetailTiming, /* [out] pointer to a BHDM_EDID_DetailTiming
								   structure to hold the data */
	BFMT_VideoFmt *eVideoFmt
) ;


/******************************************************************************
Summary:
Retrieve a EDID Video Descriptor

Description:
EDID V3 Timing Extensions contain short Video Descriptors which provide
information on supported formats

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.
	NthIdDescriptor - number of the descriptor to get


Output:
	VideoIdCode - pointer to the buffer to hold the Video ID Code (see CEA-861B)
	BCM_VideoFmt - associated BCM_VideoFmt with VideoIdCode
	NativeFormat - indicates if Video Format is a native format os the monitor

Returns:
	BERR_SUCCESS                 - Requested tag successfully found and copied
	BERR_INVALID_PARAMETER       - Invalid function parameter.


See Also:
	o BHDM_Handle
*******************************************************************************/
BERR_Code BHDM_EDID_GetVideoDescriptor(
	BHDM_Handle hHDMI,			 /* [in] HDMI handle */
	uint8_t NthIdCode,           /* [in] number of the Video ID Code to get */
	uint8_t *VideoIdCode,        /* [out] 861B Video ID Code  */
	BFMT_VideoFmt *BCM_VideoFmt, /* [out] associated BFMT */
	uint8_t *NativeFormat        /* [out] Native Monitor Format y/n */
)  ;


/******************************************************************************
Summary:
Retrieve a specified EDID descriptor

Description:
EDIDs may or may not contain ASCII descriptors which provide additional
information about a monitor

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	tag - Type of Descriptor to search for

	o BHDM_EDID_Tag_eMONITOR_NAME
	o BHDM_EDID_Tag_eMONITOR_ASCII
	o BHDM_EDID_Tag_eMONITOR_SN

	iBufSize - size of buffer pointed to by pDescriptorText


Output:
	pDescriptorText - pointer to buffer to hold descriptor data.
	buffer must be <= 13 bytes

Returns:
	BERR_SUCCESS                 - Requested tag successfully found and copied
	BERR_INVALID_PARAMETER       - Invalid function parameter.
	BHDM_EDID_NOT_FOUND          - No EDID Available
	BHDM_EDID_CHECKSUM_ERROR     - EDID Checksum Error
	BHDM_EDID_DESCRIPTOR_NOT_FOUND - Requested Descriptor is not available
	BERR_EDID_NOT_ENOUGH_MEM     - Not enough memory to hold the EDID descriptor
	BHDM_EDID_EXT_VERSION_NOT_SUPPORTED      - Requested EDID information is not supported

Note:
	The calling function must allocate and free the 13 byte buffer pointed to by
	pDescriptorText

See Also:
	o BHDM_Handle
	o BHDM_EDID_Tag
*******************************************************************************/
BERR_Code BHDM_EDID_GetDescriptor(
   BHDM_Handle hHDMI, /* [in] HDMI handle */
   BHDM_EDID_Tag tag, /* [in] id of the descriptor tag to retrieve */
   uint8_t *pDescriptorText,    /* [out] pointer to memory to hold retrieved tag data */
   uint8_t iBufSize       /* [in ] mem size in bytes of pDescriptorText */
) ;


BERR_Code BHDM_EDID_GetMonitorName(
   BHDM_Handle hHDMI, /* [in] HDMI handle */
   uint8_t *pDescriptorText    /* [out] pointer to memory to hold retrieved tag data */
) ;


/******************************************************************************
Summary:
Retrieve the monitor ranges supported as specified in the EDID

Description:
The EDID may contain a Descriptor that defines the Monitor Range Limits.
This function can be used to extract those ranges and place them in the
BHDM_EDID_MoniorRange structure.


Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pMonitorRange - pointer to previously allocated structure to hold the
	Monitor Range information

Returns:
	BERR_SUCCESS                   - Monitor Range Retrieved
	BERR_INVALID_PARAMETER         - Invalid function parameter.
	BHDM_EDID_NOT_FOUND             - EDID is Not Available
	BHDM_EDID_CHECKSUM_ERROR        - EDID Checksum Error
	BHDM_EDID_MONITOR_RANGE_NOT_FOUND - Monitor Range Not Available in EDID


See Also:
	o BHDM_Handle
	o BHDM_EDID_MonitorRange

*******************************************************************************/
BERR_Code BHDM_EDID_GetMonitorRange(
   BHDM_Handle hHDMI,                    /* [in] HDMI handle */
   BHDM_EDID_MonitorRange *pMonitorRange /* [out] pointer to BHDM_EDID_MonitorRange
										    to hold the retrieved data */
);



/******************************************************************************
Summary:
Check if the connected Receiver Device is HDMI Capable

Description:
The EDID may contain a CEA-861B Vendor Specific Data Block (VSDB) in the first
Version 3 Timing Block.  This HDMI VSDB indicates the receiver is HDMI Capable
If the Receiver is HDMI capable, this function can be used to extract the
VSDB information and other functionality supported by the Receiver.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pRxVSDB - pointer to previously allocated structure to hold information
	contained in the Vendor Specific Data Block and other HDMI related data
	bHdmiDeive - pointer to boolean true to indicate Rx device supports HDMI
	               false to indicate device supports DVI only

Returns:
	BERR_SUCCESS                    - Monitor is HDMI Capable and data retrieved
	BERR_INVALID_PARAMETER          - Invalid function parameter.
	BHDM_EDID_HDMI_VSDB_NOT_FOUND   - VSDB not found
	BHDM_EDID_HDMI_VSDB_REGID_ERROR - Registration ID error in VSCB

Note:
	All returns except BERR_SUCCESS indicate the attached device supports DVI
	only.

See Also:
	o BHDM_Handle
	o BHDM_EDID_HdmiData

*******************************************************************************/
BERR_Code BHDM_EDID_IsRxDeviceHdmi(
   BHDM_Handle hHDMI,                    /* [in] HDMI handle */
   BHDM_EDID_RxVendorSpecificDB *pRxVSDB, /* [out] pointer to HDMI data structure
                                             to hold the retrieved data */
   bool *bHdmiDevice                     /* [out] pointer to bool indicating
										      attached device supports HDMI */
) ;


/******************************************************************************
Summary:
Get content of HDMI Rx VSDB

Description:
The EDID may contain a CEA-861B Vendor Specific Data Block (VSDB) in the first
Version 3 Timing Block.  This HDMI VSDB indicates the receiver is HDMI Capable
If the Receiver is HDMI capable, this function can be used to extract the
VSDB information and other functionality supported by the Receiver.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pRxVSDB - pointer to previously allocated structure to hold information
	contained in the Vendor Specific Data Block and other HDMI related data

Returns:
	BERR_SUCCESS                    - Monitor is HDMI Capable and data retrieved
	BERR_INVALID_PARAMETER          - Invalid function parameter.
	BHDM_EDID_HDMI_VSDB_NOT_FOUND   - VSDB not found
	BHDM_EDID_HDMI_VSDB_REGID_ERROR - Registration ID error in VSCB

Note:
	All returns except BERR_SUCCESS indicate the attached device supports DVI
	only.

See Also:
	o BHDM_Handle
	o BHDM_EDID_HdmiData

*******************************************************************************/
BERR_Code BHDM_EDID_GetHdmiVsdb(
   BHDM_Handle hHDMI,                  /* [in] HDMI handle */
   BHDM_EDID_RxVendorSpecificDB *RxVSDB  /* [out] pointer to Vendor Specific Data
                                            Block to hold the retrieved data */
) ;


/******************************************************************************
Summary:
Get content of HDMI Forum VSDB

Description:
The EDID may contain a CEA-861B HDMI Forum Vendor Specific Data Block (RxHdmiForumVSDB) in the first
Version 3 Timing Block.  This HDMI Forum VSDB indicates additional HDMI Capabilities for HDMI 2.0 features

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pRxHdmiForumVSDB - pointer to previously allocated structure to hold information

Returns:
	BERR_SUCCESS                    - Monitor is HDMI Capable and data retrieved
	BERR_INVALID_PARAMETER          - Invalid function parameter.
	BHDM_EDID_HDMI_VSDB_NOT_FOUND   - VSDB not found
	BHDM_EDID_HDMI_VSDB_REGID_ERROR - Registration ID error in VSCB

Note:
	All returns except BERR_SUCCESS indicate the attached device supports DVI
	only.

See Also:
	o BHDM_Handle
	o BHDM_EDID_HdmiData

*******************************************************************************/
BERR_Code BHDM_EDID_GetHdmiForumVsdb(
   BHDM_Handle hHDMI,                  /* [in] HDMI handle */
   BHDM_EDID_RxHfVsdb *RxHdmiForumVSDB  /* [out] pointer to Vendor Specific Data
                                            Block to hold the retrieved data */
) ;


/******************************************************************************
Summary:
Get content of HDR Static MetaData Block

Description:
The EDID may contain a CEA-861B HDR Metadata Block  in the first
Version 3 Timing Block.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pRxHdrDB - pointer to previously allocated structure to hold information

Returns:
	BERR_SUCCESS                    - Monitor is HDMI Capable and data retrieved
	BERR_INVALID_PARAMETER          - Invalid function parameter.

Note:

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_EDID_GetHdrStaticMetadatadb(
   const BHDM_Handle hHDMI,                  /* [in] HDMI handle */
   BHDM_EDID_HDRStaticDB *pRxHdrDB  /* [out] pointer to HDR Data
                                            Block to hold the retrieved data */
) ;


/******************************************************************************
Summary:
Check if the connected HDMI Receiver can support specified audio parameters

Description:
The EDID may contain CEA-861B Audio Descriptors contained within Version 3
Timing Extensions of the EDID.
This function will check if certain audio parameters are specified as being
supported by the HDMI Receiver.

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.
	eAudioFormat - format of Audio (PCM, AC3, AAC, etc)
	eAudioSamplingRate - sampling rate of the Audio (32kHz, 44.1kHz, etc.)
	eAudioBits  - number of bits in each Audio Sample (16, 20, etc.)
	iCommpressedBitRate  - Compressed Bit Rate Supported by the HDMI Receiver


Output:
	iSupported - Specified audio format is reported as supported by EDID

Returns:
	BERR_SUCCESS                    - Monitor is HDMI Capable and data retrieved
	BERR_INVALID_PARAMETER          - Invalid function parameter.
	BHDM_EDID_HDMI_VSDB_NOT_FOUND   - VSDB not found
	BHDM_EDID_HDMI_UNKNOWN_CEA_TAG  - Unknown CEA Tag
	BHDM_EDID_HDMI_UNKNOWN_BIT_RATE - Unknown Bit Rate extracted from Audio Descriptor
	BHDM_EDID_HDMI_AUDIO_UNSUPPORTED - Rx does not support requested Audio Parameters

Note:
	eAudioBits and iCompressedBitRate are mutually exclusive.
	The eAudioBits 	enumeration is to be used with eAudioFormat
	BAVC_AudioFormat_ePCM (uncompressed).
	The iCompressedBitRate is to be used with all other eAudioFormat
	enumerations.  The function uses the eAudioFormat to determie which
	argument to use.

	Calling function should assume DVI if return value is not BERR_SUCCESS

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_EDID_CheckRxHdmiAudioSupport(
   BHDM_Handle hHDMI,                   /* [in] HDMI handle  */
   BAVC_AudioFormat       eAudioFormat, /* [in] Audio Format */
   BAVC_AudioSamplingRate eAudioSamplingRate, /* [in] Audio Rate to check for */
   BAVC_AudioBits         eAudioBits,   /* [in] Number of Bits if uncompressed */
   uint16_t         iCompressedBitRate, /* [in] Bit Rate if compressed audio */
   uint8_t *iSupported
)  ;


/******************************************************************************
Summary:
Check if the connected HDMI Receiver can support specified video parameters

Description:
The EDID may contain CEA-861B Video Descriptors contained within Version 3
Timing Extensions of the EDID.
This function will check if certain video parameters are specified as being
supported by the HDMI Receiver.

Input:
	hHDMI - The HDMI device handle that the application created earlier

	HorizontalPixels - Number of active horizontal pixels
	VerticalPixels   - Number of active vertical pixels
	eScanType        - Enumeration of Scan Types (progressive, interlaced)
	eFrameRateCode   - Enumeration of Vertical Frequency (59.94 / 60Hz etc)
	eAspectRatio     - Enumeration of Aspect Ratio (16:9, 4:3 etc)


Output:
	*pNativeFormat	 - specified input format is native to display (0, 1)

Returns:
	BERR_SUCCESS                    - Monitor is HDMI Capable and data retrieved
	BERR_INVALID_PARAMETER          - Invalid function parameter.
	BHDM_EDID_HDMI_VSDB_NOT_FOUND   - VSDB not found; HDMI Unsupported
	BHDM_EDID_HDMI_UNKNOWN_CEA_TAG  - Unknown CEA Tag
	BHDM_EDID_HDMI_VIDEO_FORMAT_UNSUPPORTED - Rx does not support requested Video Parameters

Note:
	Calling function should assume DVI if return value is not BERR_SUCCESS

See Also:
	o BHDM_Handle
	o BHDM_EDID_HdmiData

*******************************************************************************/
BERR_Code BHDM_EDID_CheckRxHdmiVideoSupport(
	BHDM_Handle hHDMI,                    /* [in] HDMI handle  */
	uint16_t           HorizontalPixels,  /* [in] Horiz Active Pixels */
	uint16_t           VerticalPixels,    /* [in] Vertical Active Pixels */
	BAVC_ScanType      eScanType,         /* [in] Progressive, Interlaced */
	BAVC_FrameRateCode eFrameRateCode,    /* [in] Vertical Frequency */
	BFMT_AspectRatio   eAspectRatio,      /* [in] Horiz  to Vertical Ratio */
	uint8_t            *pNativeFormat     /* [out] Requested format is a
										      native format to the monitor */
) ;



/******************************************************************************
Summary:
Check if the connected HDMI Receiver can support the requested format

Description:
Check if the attached monitor can support the requested BFMT_VideoFmt.  The
table containing the supported formats are loaded at hot plug events

Input:
	hHDMI - The HDMI device handle that the application created earlier
	BFMT_VideoFmt - Requested Video Format

Output:
	*pSupported   - flag specifying if requested format is supported

Returns:
	BERR_SUCCESS                    - Monitor is HDMI Capable and data retrieved
	BERR_INVALID_PARAMETER          - Invalid function parameter.

Note:
	Calling function should assume the format is not supported if return value
	is not BERR_SUCCESS

See Also:
	o BHDM_Handle
	o BFMT_VideoFmt

*******************************************************************************/
BERR_Code BHDM_EDID_VideoFmtSupported(
	BHDM_Handle hHDMI,        /* [in] HDMI handle  */
	BFMT_VideoFmt eVideoFmt,  /* requested video format */
	uint8_t *Supported        /* [out] true/false Requested format is
							    supported by the monitor */
) ;


/******************************************************************************
Summary:
Return all video formats supported by the attached monitor.

Description:
This function will fill an bool array matching the number of BFMT_VideoFmts that are
supported by the attached monitor.  True for supported; False if not.

Input:
	hHDMI - The HDMI device handle that the application created earlier

Output:
	*bVideoFormats - bool array of size BFMT_VideoFmt_eMaxCount indicating what
	                 BFMTs are supported by the display

Returns:
	BERR_SUCCESS                    - Monitor is HDMI Capable and data retrieved
	BERR_INVALID_PARAMETER          - Invalid function parameter.
	BHDM_EDID_HDMI_VSDB_NOT_FOUND   - VSDB not found; HDMI Unsupported
	BHDM_EDID_HDMI_UNKNOWN_CEA_TAG  - Unknown CEA Tag
	BHDM_EDID_HDMI_VIDEO_FORMAT_UNSUPPORTED - Rx does not support requested Video Parameters

Note:
	This function will check both EDID Version 3 Timing Extensions and
	Preferred Detailed Timing Descriptors.  While the V3 extensions include
	the formats contained in the Preferred Extensions, both are searched due
	to DVI monitors are not required to have V3 Timing Extensions.

See Also:
	o BHDM_Handle
	o BHDM_EDID_GetSupportedVideoFormats
	o BHDM_EDID_CheckRxHdmiVideoSupport

*******************************************************************************/
BERR_Code BHDM_EDID_GetSupportedVideoFormats(
	BHDM_Handle hHDMI,                    /* [in] HDMI handle  */
	bool *bVideoFormats                   /* [out] supported true/false */
) ;


BERR_Code BHDM_EDID_GetSupported420VideoFormats(
	BHDM_Handle hHDMI,                    /* [in] HDMI handle  */
	bool *bVideoFormats                   /* [out] supported true/false */
) ;


BERR_Code BHDM_EDID_GetSupported3DFormats(
	BHDM_Handle hHDMI,					 /* [in] HDMI handle  */
	BHDM_EDID_3D_Structure_ALL *_3DFormats					 /* [out] supported true/false */
);


/******************************************************************************
Summary:
Return information (video formats, video ID code, etc.) of all formats supported by the attached monitor.

Description:
This function will return a structure contain a boolean array matching the number of
BFMT_VideoFmts that are supported by the attached monitor, a list of video ID code of the
support formats as well as the number of supported formats.

Input:
	hHDMI - The HDMI device handle that the application created earlier

Output:
	*stSupportedVideoInfo - structure holding supported video information

Returns:
	BERR_SUCCESS					- Monitor is HDMI Capable and data retrieved
	BHDM_EDID_NOT_FOUND			- No EDID found
	BHDM_EDID_VIDEO_FORMATS_UNAVAILABLE		- No supported formats were found

See Also:
	o BHDM_Handle
	o BHDM_EDID_GetSupportedVideoFormats

*******************************************************************************/
BERR_Code BHDM_EDID_GetSupportedVideoInfo(
	BHDM_Handle hHDMI,					 /* [in] HDMI handle  */
	BHDM_EDID_VideoDescriptorInfo *pSupportedVideoInfo		/* [out] supported true/false */
);


/******************************************************************************
Summary:
Return all audio formats supported by the attached monitor.

Description:
This function will fill an bool array matching the number of BFMT_VideoFmts that are
supported by the attached monitor.  True for supported; False if not.

Input:
	hHDMI - The HDMI device handle that the application created earlier

Output:
	BcmAudioFormats - array of size BAVC_AudioFmt_eMaxCount indicating what
	                  audio are supported by the HDMI Rx

Returns:
	BERR_SUCCESS                    - Monitor is HDMI Capable and data retrieved
	BERR_INVALID_PARAMETER          - Invalid function parameter.


See Also:
	o BHDM_Handle
	o BHDM_EDID_GetSupportedAudioFormats
	o BHDM_EDID_CheckRxHdmiAudioSupport

*******************************************************************************/
BERR_Code BHDM_EDID_GetSupportedAudioFormats(
	BHDM_Handle hHDMI,                          /* [in] HDMI handle  */
	BHDM_EDID_AudioDescriptor *BcmAudioFormats  /* [out] supported formats */
) ;

/******************************************************************************
Summary:
Dump all EDID blocks of the attached DVI/HDMI Receiver

Description:
This function will fill an bool array matching the number of BFMT_VideoFmts that are
supported by the attached monitor.  True for supported; False if not.

Input:
	hHDMI - The HDMI device handle that the application created earlier

Returns:
	BERR_SUCCESS                    - Monitor is HDMI Capable and data retrieved

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_EDID_DumpRawEDID(
	BHDM_Handle hHDMI                    /* [in] HDMI handle  */
) ;


/******************************************************************************
Summary:
Initialize the EDID information by re-reading the EDID in the monitor.

Description:
This function will initialize EDID information read from an attached receiver
This function is useful for reading EDID after a Hot Plug event

Input:
	hHDMI - The HDMI device handle that the application created earlier

Returns:

See Also:
	o BHDM_Handle
	o BHDM_EDID_GetSupportedFormats
	o BHDM_EDID_GetMonitorRange

*******************************************************************************/
BERR_Code BHDM_EDID_Initialize(
	BHDM_Handle hHDMI                    /* [in] HDMI handle  */
) ;


typedef struct BHDM_EDID_ColorimetryParams
{
    BFMT_VideoFmt eVideoFmt ;
    bool xvYccEnabled ;
} BHDM_EDID_ColorimetryParams ;


/******************************************************************************
Summary:
Get the STB's preferred Colorimetry to use based on the current format, xvYcc setting and EDID information
from the monitor

Description:
This function will get the colorimetry to use based on the selected format, xvYcc setting and the capanbilities
of the attached HDMI monitor


This function must be used after the BHDM_EDID_Initialize function

Input:
	hHDMI - The HDMI device handle that the application created earlier
	eOutputFormat - output (encoding) format sent to display (DVI vs HDMI mode)
	eVideoFmt - output display format: SD, HD, or PC format
	eColorimetry - Recommended and Supported Colorimetry

Returns:

See Also:
	o BHDM_Handle
	o BHDM_EDID_ColorimetryParams
	o BAVC_MatrixCoefficients


*******************************************************************************/
BERR_Code BHDM_EDID_GetPreferredColorimetry(
	BHDM_Handle hHDMI,
	const BHDM_EDID_ColorimetryParams *parameters,
	BAVC_MatrixCoefficients *eColorimetry) ;


/******************************************************************************
Summary:
Get the Recommended/Supported Colorimetry to use based on the EDID information in the
monitor

Description:
This function will get the colorimetry to use based on the selected format and output mode.  The
function make sure the format it selects is supported by the attached HDMI monitor

This function must be used after the BHDM_EDID_Initialize function

Input:
	hHDMI - The HDMI device handle that the application created earlier
	eOutputFormat - output (encoding) format sent to display (DVI vs HDMI mode)
	eVideoFmt - output display format: SD, HD, or PC format
	eColorimetry - Recommended and Supported Colorimetry

Returns:

See Also:
	o BHDM_Handle
	o BHDM_OutputFormat
	o BFMT_VideoFmt


*******************************************************************************/
BERR_Code BHDM_EDID_GetSupportedColorimetry(
	BHDM_Handle hHDMI, BHDM_OutputFormat eOutputFormat,
	BFMT_VideoFmt eVideoFmt, BAVC_MatrixCoefficients *eColorimetry) ;


/******************************************************************************
Summary:
Get the Video Capabilities Data Block based on the EDID information of the
attached monitor.

Description:
This function will return the Video Capabiliites of the attached HDMI monitor

This function must be used after the BHDM_EDID_Initialize function

Input:
	hHDMI - The HDMI device handle that the application created earlier

Output:
	pVideoCapabilityDataBlock - pointer to structure to add the Video Capabilities

Returns:

See Also:
	o BHDM_Handle
	o BHDM_EDID_VideoCapabilityDataBlock
*******************************************************************************/
BERR_Code BHDM_EDID_GetVideoCapabilityDB(
	BHDM_Handle hHDMI,
	BHDM_EDID_VideoCapabilityDataBlock *pVideoCapabilityDataBlock) ;


/******************************************************************************
Summary:
Get the supported Extended Colorimetry Data Block based on the EDID information of the
attached monitor.

Description:
This function will return whether or not the attached HDMI monitor
supports the xvYCC (601 or 709 or both) colorimetry.

This function must be used after the BHDM_EDID_Initialize function

Input:
	hHDMI - The HDMI device handle that the application created earlier

Output:
	pExtendedColorimetry - pointer to structure indicating xvYCC supported colorimetries

Returns:

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_EDID_GetColorimetryDB(
	BHDM_Handle hHDMI,
	BHDM_EDID_ColorimetryDataBlock *pColorimetryDataBlock) ;



/******************************************************************************
Summary:
Get the supported deep color mode based on the EDID information of the
attached monitor.

Description:
This function will return the supported deep color modes (if any) of the attached HDMI sink
This function must be used after the BHDM_EDID_Initialize function

Input:
	hHDMI - The HDMI device handle that the application created earlier

Output:
	stSuppotedColorDepth - pointer to structure indicating supported color depth
	bYCbCrPixelEncoding - indicate YCbCr 4:4:4 pixel encoding support for the supported
						color depth

Returns:

See Also:
	o BHDM_Handle
	o BHDM_EDID_ExtendedColorimetry

*******************************************************************************/
BERR_Code BHDM_EDID_GetSupportedColorDepth(
	BHDM_Handle hHDMI,
	BHDM_EDID_ColorDepth *stSuppotedColorDepth,	/* [out] */
	bool *bYCbCrPixelEncoding 	/* [out] */
);


/******************************************************************************
Summary:
Get CEC Physical Address based on the EDID information of the
attached sink.

Description:
This function will return the CEC Physical Address assigned to
us (Tx) by the attached sink.

This function must be used after the BHDM_EDID_Initialize function

Input:
	hHDMI - The HDMI device handle that the application created earlier

Output:
	pMyPhysicalAddr - pointer to 2-byte array for receiving address

Returns:
	BERR_SUCCESS - Successfully returned the CEC Physical Addresses
        BHDM_EDID_NOT_FOUND

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_EDID_GetMyCecPhysicalAddr(
   BHDM_Handle hHDMI,         /* [in] HDMI handle */
   uint8_t *pMyPhysicalAddr); /* [out] ptr to uint8 to hold Physical Addr */


#ifdef __cplusplus
}
#endif

#endif /* BHDM_EDID_H__ */
