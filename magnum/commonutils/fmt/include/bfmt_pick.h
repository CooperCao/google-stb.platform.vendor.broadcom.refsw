/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 *   Video format module header file
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BFMT_PICKH__
#define BFMT_PICKH__

#ifdef __cplusplus
extern "C" {
#endif

#define BFMT_PICK_NO  0

/***************************************************************
 *
 * I. define PICK for display format
 *
 ***************************************************************/

#if !defined(BFMT_PICK_eNTSC)
    #define BFMT_PICK_eNTSC  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eNTSC_J)
    #define BFMT_PICK_eNTSC_J  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eNTSC_443)
    #define BFMT_PICK_eNTSC_443  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_B)
    #define BFMT_PICK_ePAL_B  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_B1)
    #define BFMT_PICK_ePAL_B1  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_D)
    #define BFMT_PICK_ePAL_D  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_D1)
    #define BFMT_PICK_ePAL_D1  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_G)
    #define BFMT_PICK_ePAL_G  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_H)
    #define BFMT_PICK_ePAL_H  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_K)
    #define BFMT_PICK_ePAL_K  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_I)
    #define BFMT_PICK_ePAL_I  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_M)
    #define BFMT_PICK_ePAL_M  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_N)
    #define BFMT_PICK_ePAL_N  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_NC)
    #define BFMT_PICK_ePAL_NC  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_ePAL_60)
    #define BFMT_PICK_ePAL_60  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eSECAM_L)
    #define BFMT_PICK_eSECAM_L  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eSECAM_B)
    #define BFMT_PICK_eSECAM_B  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eSECAM_G)
    #define BFMT_PICK_eSECAM_G  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eSECAM_D)
    #define BFMT_PICK_eSECAM_D  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eSECAM_K)
    #define BFMT_PICK_eSECAM_K  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eSECAM_H)
    #define BFMT_PICK_eSECAM_H  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080i)
    #define BFMT_PICK_e1080i  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080p)
    #define BFMT_PICK_e1080p  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e720p)
    #define BFMT_PICK_e720p  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e720p_60Hz_3DOU_AS)
    #define BFMT_PICK_e720p_60Hz_3DOU_AS  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e720p_50Hz_3DOU_AS)
    #define BFMT_PICK_e720p_50Hz_3DOU_AS  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e720p_30Hz_3DOU_AS)
    #define BFMT_PICK_e720p_30Hz_3DOU_AS  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e720p_24Hz_3DOU_AS)
    #define BFMT_PICK_e720p_24Hz_3DOU_AS  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e480p)
    #define BFMT_PICK_e480p  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080i_50Hz)
    #define BFMT_PICK_e1080i_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080p_24Hz_3DOU_AS)
    #define BFMT_PICK_e1080p_24Hz_3DOU_AS  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080p_30Hz_3DOU_AS)
    #define BFMT_PICK_e1080p_30Hz_3DOU_AS  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080p_60Hz_3DOU_AS)
    #define BFMT_PICK_e1080p_60Hz_3DOU_AS  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080p_60Hz_3DLR)
    #define BFMT_PICK_e1080p_60Hz_3DLR  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080p_24Hz)
    #define BFMT_PICK_e1080p_24Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080p_25Hz)
    #define BFMT_PICK_e1080p_25Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080p_30Hz)
    #define BFMT_PICK_e1080p_30Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080p_50Hz)
    #define BFMT_PICK_e1080p_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080p_100Hz)
    #define BFMT_PICK_e1080p_100Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1080p_120Hz)
    #define BFMT_PICK_e1080p_120Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1250i_50Hz)
    #define BFMT_PICK_e1250i_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e720p_24Hz)
    #define BFMT_PICK_e720p_24Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e720p_25Hz)
    #define BFMT_PICK_e720p_25Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e720p_30Hz)
    #define BFMT_PICK_e720p_30Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e720p_50Hz)
    #define BFMT_PICK_e720p_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e576p_50Hz)
    #define BFMT_PICK_e576p_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e240p_60Hz)
    #define BFMT_PICK_e240p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e288p_50Hz)
    #define BFMT_PICK_e288p_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1440x480p_60Hz)
    #define BFMT_PICK_e1440x480p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e1440x576p_50Hz)
    #define BFMT_PICK_e1440x576p_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e3840x2160p_24Hz)
    #define BFMT_PICK_e3840x2160p_24Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e3840x2160p_25Hz)
    #define BFMT_PICK_e3840x2160p_25Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e3840x2160p_30Hz)
    #define BFMT_PICK_e3840x2160p_30Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e3840x2160p_50Hz)
    #define BFMT_PICK_e3840x2160p_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e3840x2160p_60Hz)
    #define BFMT_PICK_e3840x2160p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e4096x2160p_24Hz)
    #define BFMT_PICK_e4096x2160p_24Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e4096x2160p_25Hz)
    #define BFMT_PICK_e4096x2160p_25Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e4096x2160p_30Hz)
    #define BFMT_PICK_e4096x2160p_30Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e4096x2160p_50Hz)
    #define BFMT_PICK_e4096x2160p_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e4096x2160p_60Hz)
    #define BFMT_PICK_e4096x2160p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eCUSTOM1920x2160i_48Hz)
    #define BFMT_PICK_eCUSTOM1920x2160i_48Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eCUSTOM1920x2160i_60Hz)
    #define BFMT_PICK_eCUSTOM1920x2160i_60Hz  BFMT_PICK_NO
#endif


#if !defined(BFMT_PICK_eCUSTOM_1440x240p_60Hz)
    #define BFMT_PICK_eCUSTOM_1440x240p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eCUSTOM_1440x288p_50Hz)
    #define BFMT_PICK_eCUSTOM_1440x288p_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eCUSTOM_1366x768p)
    #define BFMT_PICK_eCUSTOM_1366x768p  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eCUSTOM_1366x768p_50Hz)
    #define BFMT_PICK_eCUSTOM_1366x768p_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x480p)
    #define BFMT_PICK_eDVI_640x480p  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x480p_CVT)
    #define BFMT_PICK_eDVI_640x480p_CVT  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_800x600p)
    #define BFMT_PICK_eDVI_800x600p  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1024x768p)
    #define BFMT_PICK_eDVI_1024x768p  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x768p)
    #define BFMT_PICK_eDVI_1280x768p  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x768p_Red)
    #define BFMT_PICK_eDVI_1280x768p_Red  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x720p_50Hz)
    #define BFMT_PICK_eDVI_1280x720p_50Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x720p)
    #define BFMT_PICK_eDVI_1280x720p  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x720p_Red)
    #define BFMT_PICK_eDVI_1280x720p_Red  BFMT_PICK_NO
#endif


	/* Added for HDMI/HDDVI input support!  VEC does not support these timing format!
	 * Convention: BFMT_VideoFmt_eDVI_{av_width}x{av_height}{i/p}_{RefreshRateInHz}.
	 * Eventually VEC can output all these timing formats when we get the microcodes
	 * for it.  Currently there are no microcode for these yet. */
#if !defined(BFMT_PICK_eDVI_640x350p_60Hz)
    #define BFMT_PICK_eDVI_640x350p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x350p_70Hz)
    #define BFMT_PICK_eDVI_640x350p_70Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x350p_72Hz)
    #define BFMT_PICK_eDVI_640x350p_72Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x350p_75Hz)
    #define BFMT_PICK_eDVI_640x350p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x350p_85Hz)
    #define BFMT_PICK_eDVI_640x350p_85Hz  BFMT_PICK_NO
#endif


#if !defined(BFMT_PICK_eDVI_640x400p_60Hz)
    #define BFMT_PICK_eDVI_640x400p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x400p_70Hz)
    #define BFMT_PICK_eDVI_640x400p_70Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x400p_72Hz)
    #define BFMT_PICK_eDVI_640x400p_72Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x400p_75Hz)
    #define BFMT_PICK_eDVI_640x400p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x400p_85Hz)
    #define BFMT_PICK_eDVI_640x400p_85Hz  BFMT_PICK_NO
#endif


#if !defined(BFMT_PICK_eDVI_640x480p_66Hz)
    #define BFMT_PICK_eDVI_640x480p_66Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x480p_70Hz)
    #define BFMT_PICK_eDVI_640x480p_70Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x480p_72Hz)
    #define BFMT_PICK_eDVI_640x480p_72Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x480p_75Hz)
    #define BFMT_PICK_eDVI_640x480p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_640x480p_85Hz)
    #define BFMT_PICK_eDVI_640x480p_85Hz  BFMT_PICK_NO
#endif


#if !defined(BFMT_PICK_eDVI_720x400p_60Hz)
    #define BFMT_PICK_eDVI_720x400p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_720x400p_70Hz)
    #define BFMT_PICK_eDVI_720x400p_70Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_720x400p_72Hz)
    #define BFMT_PICK_eDVI_720x400p_72Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_720x400p_75Hz)
    #define BFMT_PICK_eDVI_720x400p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_720x400p_85Hz)
    #define BFMT_PICK_eDVI_720x400p_85Hz  BFMT_PICK_NO
#endif


#if !defined(BFMT_PICK_eDVI_800x600p_56Hz)
    #define BFMT_PICK_eDVI_800x600p_56Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_800x600p_59Hz_Red)
    #define BFMT_PICK_eDVI_800x600p_59Hz_Red  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_800x600p_70Hz)
    #define BFMT_PICK_eDVI_800x600p_70Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_800x600p_72Hz)
    #define BFMT_PICK_eDVI_800x600p_72Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_800x600p_75Hz)
    #define BFMT_PICK_eDVI_800x600p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_800x600p_85Hz)
    #define BFMT_PICK_eDVI_800x600p_85Hz  BFMT_PICK_NO
#endif


#if !defined(BFMT_PICK_eDVI_1024x768p_66Hz)
    #define BFMT_PICK_eDVI_1024x768p_66Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1024x768p_70Hz)
    #define BFMT_PICK_eDVI_1024x768p_70Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1024x768p_72Hz)
    #define BFMT_PICK_eDVI_1024x768p_72Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1024x768p_75Hz)
    #define BFMT_PICK_eDVI_1024x768p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1024x768p_85Hz)
    #define BFMT_PICK_eDVI_1024x768p_85Hz  BFMT_PICK_NO
#endif


#if !defined(BFMT_PICK_eDVI_1280x720p_70Hz)
    #define BFMT_PICK_eDVI_1280x720p_70Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x720p_72Hz)
    #define BFMT_PICK_eDVI_1280x720p_72Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x720p_75Hz)
    #define BFMT_PICK_eDVI_1280x720p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x720p_85Hz)
    #define BFMT_PICK_eDVI_1280x720p_85Hz  BFMT_PICK_NO
#endif


	/* New DVI or PC vdec input support */
#if !defined(BFMT_PICK_eDVI_1024x768i_87Hz)
    #define BFMT_PICK_eDVI_1024x768i_87Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1152x864p_75Hz)
    #define BFMT_PICK_eDVI_1152x864p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x768p_75Hz)
    #define BFMT_PICK_eDVI_1280x768p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x768p_85Hz)
    #define BFMT_PICK_eDVI_1280x768p_85Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x800p_60Hz)
    #define BFMT_PICK_eDVI_1280x800p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x960p_60Hz)
    #define BFMT_PICK_eDVI_1280x960p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x960p_85Hz)
    #define BFMT_PICK_eDVI_1280x960p_85Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x1024p_60Hz)
    #define BFMT_PICK_eDVI_1280x1024p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x1024p_69Hz)
    #define BFMT_PICK_eDVI_1280x1024p_69Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x1024p_75Hz)
    #define BFMT_PICK_eDVI_1280x1024p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1280x1024p_85Hz)
    #define BFMT_PICK_eDVI_1280x1024p_85Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_832x624p_75Hz)
    #define BFMT_PICK_eDVI_832x624p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1360x768p_60Hz)
    #define BFMT_PICK_eDVI_1360x768p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1366x768p_60Hz)
    #define BFMT_PICK_eDVI_1366x768p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1400x1050p_60Hz)
    #define BFMT_PICK_eDVI_1400x1050p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1400x1050p_60Hz_Red)
    #define BFMT_PICK_eDVI_1400x1050p_60Hz_Red  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1400x1050p_75Hz)
    #define BFMT_PICK_eDVI_1400x1050p_75Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1600x1200p_60Hz)
    #define BFMT_PICK_eDVI_1600x1200p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1920x1080p_60Hz_Red)
    #define BFMT_PICK_eDVI_1920x1080p_60Hz_Red  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_848x480p_60Hz)
    #define BFMT_PICK_eDVI_848x480p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1064x600p_60Hz)
    #define BFMT_PICK_eDVI_1064x600p_60Hz  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eDVI_1440x900p_60Hz)
    #define BFMT_PICK_eDVI_1440x900p_60Hz  BFMT_PICK_NO
#endif


	/* SW7435-276: New format enums for 482/483 */
#if !defined(BFMT_PICK_e720x482_NTSC)
    #define BFMT_PICK_e720x482_NTSC  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e720x482_NTSC_J)
    #define BFMT_PICK_e720x482_NTSC_J  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_e720x483p)
    #define BFMT_PICK_e720x483p  BFMT_PICK_NO
#endif


	/* statics: custom formats */
#if !defined(BFMT_PICK_eCustom0)
    #define BFMT_PICK_eCustom0  BFMT_PICK_NO
#endif

#if !defined(BFMT_PICK_eCustom1)
    #define BFMT_PICK_eCustom1  BFMT_PICK_NO
#endif


	/* dynamics: custom format */
#if !defined(BFMT_PICK_eCustom2)
    #define BFMT_PICK_eCustom2  BFMT_PICK_NO
#endif


	/* Must be last */
#if !defined(BFMT_PICK_eMaxCount)
    #define BFMT_PICK_eMaxCount  BFMT_PICK_NO
#endif

/***************************************************************
 *
 * II. define PICK for PXL_frq accroding to the PICK of display format
 *
 ***************************************************************/

#if BFMT_PICK_eNTSC
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_27MHz_MUL_1_001
        #define BFMT_PICK_PXL_27MHz_MUL_1_001  1
    #endif
#endif

#if BFMT_PICK_eNTSC_J
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_27MHz_MUL_1_001
        #define BFMT_PICK_PXL_27MHz_MUL_1_001  1
    #endif
#endif

#if BFMT_PICK_eNTSC_443
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_27MHz_MUL_1_001
        #define BFMT_PICK_PXL_27MHz_MUL_1_001  1
    #endif
#endif

#if BFMT_PICK_ePAL_B
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_ePAL_B1
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_ePAL_D
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_ePAL_D1
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_ePAL_G
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_ePAL_H
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_ePAL_K
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_ePAL_I
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_ePAL_M
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_27MHz_MUL_1_001
        #define BFMT_PICK_PXL_27MHz_MUL_1_001  1
    #endif
#endif

#if BFMT_PICK_ePAL_N
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_ePAL_NC
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_ePAL_60
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_27MHz_MUL_1_001
        #define BFMT_PICK_PXL_27MHz_MUL_1_001  1
    #endif
#endif

#if BFMT_PICK_eSECAM_L
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_eSECAM_B
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_eSECAM_G
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_eSECAM_D
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_eSECAM_K
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_eSECAM_H
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_e1080i
    #ifndef BFMT_PICK_PXL_74_25MHz_DIV_1_001
        #define BFMT_PICK_PXL_74_25MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
#endif

#if BFMT_PICK_e1080p
    #ifndef BFMT_PICK_PXL_148_5MHz_DIV_1_001
        #define BFMT_PICK_PXL_148_5MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_148_5MHz
        #define BFMT_PICK_PXL_148_5MHz  1
    #endif
#endif

#if BFMT_PICK_e720p
    #ifndef BFMT_PICK_PXL_74_25MHz_DIV_1_001
        #define BFMT_PICK_PXL_74_25MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
#endif

#if BFMT_PICK_e720p_60Hz_3DOU_AS
    #ifndef BFMT_PICK_PXL_148_5MHz_DIV_1_001
        #define BFMT_PICK_PXL_148_5MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_148_5MHz
        #define BFMT_PICK_PXL_148_5MHz  1
    #endif
#endif

#if BFMT_PICK_e720p_50Hz_3DOU_AS
    #ifndef BFMT_PICK_PXL_148_5MHz
        #define BFMT_PICK_PXL_148_5MHz  1
    #endif
#endif

#if BFMT_PICK_e720p_30Hz_3DOU_AS
    #ifndef BFMT_PICK_PXL_148_5MHz_DIV_1_001
        #define BFMT_PICK_PXL_148_5MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_148_5MHz
        #define BFMT_PICK_PXL_148_5MHz  1
    #endif
#endif

#if BFMT_PICK_e720p_24Hz_3DOU_AS
    #ifndef BFMT_PICK_PXL_148_5MHz_DIV_1_001
        #define BFMT_PICK_PXL_148_5MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_148_5MHz
        #define BFMT_PICK_PXL_148_5MHz  1
    #endif
#endif

#if BFMT_PICK_e480p
    #ifndef BFMT_PICK_PXL_27MHz_MUL_1_001
        #define BFMT_PICK_PXL_27MHz_MUL_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
	/* for 480p double sampling */
    #ifndef BFMT_PICK_PXL_54MHz_MUL_1_001
        #define BFMT_PICK_PXL_54MHz_MUL_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_54MHz
        #define BFMT_PICK_PXL_54MHz  1
    #endif
#endif

#if BFMT_PICK_e1080i_50Hz
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
#endif

#if BFMT_PICK_e1080p_24Hz_3DOU_AS
    #ifndef BFMT_PICK_PXL_148_5MHz_DIV_1_001
        #define BFMT_PICK_PXL_148_5MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_148_5MHz
        #define BFMT_PICK_PXL_148_5MHz  1
    #endif
#endif

#if BFMT_PICK_e1080p_30Hz_3DOU_AS
    #ifndef BFMT_PICK_PXL_148_5MHz_DIV_1_001
        #define BFMT_PICK_PXL_148_5MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_148_5MHz
        #define BFMT_PICK_PXL_148_5MHz  1
    #endif
#endif

#if BFMT_PICK_e1080p_60Hz_3DOU_AS
    #ifndef BFMT_PICK_PXL_297MHz_DIV_1_001
        #define BFMT_PICK_PXL_297MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_297MHz
        #define BFMT_PICK_PXL_297MHz  1
    #endif
#endif

#if BFMT_PICK_e1080p_60Hz_3DLR
    #ifndef BFMT_PICK_PXL_297MHz_DIV_1_001
        #define BFMT_PICK_PXL_297MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_297MHz
        #define BFMT_PICK_PXL_297MHz  1
    #endif
#endif

#if BFMT_PICK_e1080p_24Hz
    #ifndef BFMT_PICK_PXL_74_25MHz_DIV_1_001
        #define BFMT_PICK_PXL_74_25MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
#endif

#if BFMT_PICK_e1080p_25Hz
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
#endif

#if BFMT_PICK_e1080p_30Hz
    #ifndef BFMT_PICK_PXL_74_25MHz_DIV_1_001
        #define BFMT_PICK_PXL_74_25MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
#endif

#if BFMT_PICK_e1080p_50Hz
    #ifndef BFMT_PICK_PXL_148_5MHz
        #define BFMT_PICK_PXL_148_5MHz  1
    #endif
#endif

#if BFMT_PICK_e1080p_100Hz
    #ifndef BFMT_PICK_PXL_297MHz
        #define BFMT_PICK_PXL_297MHz  1
    #endif
#endif

#if BFMT_PICK_e1080p_120Hz
    #ifndef BFMT_PICK_PXL_297MHz
        #define BFMT_PICK_PXL_297MHz  1
    #endif
#endif

#if BFMT_PICK_e1250i_50Hz
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
#endif

#if BFMT_PICK_e720p_24Hz
    #ifndef BFMT_PICK_PXL_74_25MHz_DIV_1_001
        #define BFMT_PICK_PXL_74_25MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
#endif

#if BFMT_PICK_e720p_25Hz
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
#endif

#if BFMT_PICK_e720p_30Hz
    #ifndef BFMT_PICK_PXL_74_25MHz_DIV_1_001
        #define BFMT_PICK_PXL_74_25MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
#endif

#if BFMT_PICK_e720p_50Hz
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
#endif

#if BFMT_PICK_e576p_50Hz
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
	/* for 576p_50Hz double sampling */
    #ifndef BFMT_PICK_PXL_54MHz
        #define BFMT_PICK_PXL_54MHz  1
    #endif
#endif

#if BFMT_PICK_e240p_60Hz
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_e288p_50Hz
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_e1440x480p_60Hz
    #ifndef BFMT_PICK_PXL_54MHz_MUL_1_001
        #define BFMT_PICK_PXL_54MHz_MUL_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_54MHz
        #define BFMT_PICK_PXL_54MHz  1
    #endif
#endif

#if BFMT_PICK_e1440x576p_50Hz
    #ifndef BFMT_PICK_PXL_54MHz
        #define BFMT_PICK_PXL_54MHz  1
    #endif
#endif

#if BFMT_PICK_e3840x2160p_24Hz
    #ifndef BFMT_PICK_PXL_297MHz_DIV_1_001
        #define BFMT_PICK_PXL_297MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_297MHz
        #define BFMT_PICK_PXL_297MHz  1
    #endif
#endif

#if BFMT_PICK_e3840x2160p_25Hz
    #ifndef BFMT_PICK_PXL_297MHz
        #define BFMT_PICK_PXL_297MHz  1
    #endif
#endif

#if BFMT_PICK_e3840x2160p_30Hz
    #ifndef BFMT_PICK_PXL_297MHz_DIV_1_001
        #define BFMT_PICK_PXL_297MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_297MHz
        #define BFMT_PICK_PXL_297MHz  1
    #endif
#endif

#if BFMT_PICK_e3840x2160p_50Hz
    #ifndef BFMT_PICK_PXL_594MHz
        #define BFMT_PICK_PXL_594MHz  1
    #endif
#endif

#if BFMT_PICK_e3840x2160p_60Hz
    #ifndef BFMT_PICK_PXL_594MHz_DIV_1_001
        #define BFMT_PICK_PXL_594MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_594MHz
        #define BFMT_PICK_PXL_594MHz  1
    #endif
#endif

#if BFMT_PICK_e4096x2160p_24Hz
    #ifndef BFMT_PICK_PXL_297MHz_DIV_1_001
        #define BFMT_PICK_PXL_297MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_297MHz
        #define BFMT_PICK_PXL_297MHz  1
    #endif
#endif

#if BFMT_PICK_e4096x2160p_25Hz
    #ifndef BFMT_PICK_PXL_297MHz
        #define BFMT_PICK_PXL_297MHz  1
    #endif
#endif

#if BFMT_PICK_e4096x2160p_30Hz
    #ifndef BFMT_PICK_PXL_297MHz_DIV_1_001
        #define BFMT_PICK_PXL_297MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_297MHz
        #define BFMT_PICK_PXL_297MHz  1
    #endif
#endif

#if BFMT_PICK_e4096x2160p_50Hz
    #ifndef BFMT_PICK_PXL_594MHz
        #define BFMT_PICK_PXL_594MHz  1
    #endif
#endif

#if BFMT_PICK_e4096x2160p_60Hz
    #ifndef BFMT_PICK_PXL_594MHz_DIV_1_001
        #define BFMT_PICK_PXL_594MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_594MHz
        #define BFMT_PICK_PXL_594MHz  1
    #endif
#endif

#if BFMT_LEGACY_3DTV_SUPPORT
#if BFMT_PICK_eCUSTOM1920x2160i_48Hz
    #ifndef BFMT_PICK_PXL_148_5MHz_DIV_1_001
        #define BFMT_PICK_PXL_148_5MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_148_5MHz
        #define BFMT_PICK_PXL_148_5MHz  1
    #endif
#endif

#if BFMT_PICK_eCUSTOM1920x2160i_60Hz
    #ifndef BFMT_PICK_PXL_148_5MHz_DIV_1_001
        #define BFMT_PICK_PXL_148_5MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_148_5MHz
        #define BFMT_PICK_PXL_148_5MHz  1
    #endif
#endif
#endif

#if BFMT_PICK_eCUSTOM_1440x240p_60Hz
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_eCUSTOM_1440x288p_50Hz
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

#if BFMT_PICK_eCUSTOM_1366x768p
    #ifndef BFMT_PICK_PXL_67_565MHz_DIV_1_001
        #define BFMT_PICK_PXL_67_565MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_67_565MHz
        #define BFMT_PICK_PXL_67_565MHz  1
    #endif
#endif

#if BFMT_PICK_eCUSTOM_1366x768p_50Hz
    #ifndef BFMT_PICK_PXL_56_304MHz
        #define BFMT_PICK_PXL_56_304MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x480p
    #ifndef BFMT_PICK_PXL_25_2MHz_DIV_1_001
        #define BFMT_PICK_PXL_25_2MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_25_2MHz
        #define BFMT_PICK_PXL_25_2MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x480p_CVT
    #ifndef BFMT_PICK_PXL_23_75MHz
        #define BFMT_PICK_PXL_23_75MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_23_75MHz_DIV_1_001
        #define BFMT_PICK_PXL_23_75MHz_DIV_1_001  1
    #endif
#endif

	/* The following VESA display formats complies with
	   http://www.sj.broadcom.com/projects/dvt/Chip_Architecture/Clocking/Released/DVT_format_support.doc;
	   Note: 59.94/60 Hz support frame rate tracking for TV formats input; */
#if BFMT_PICK_eDVI_800x600p
    #ifndef BFMT_PICK_PXL_39_79MHz
        #define BFMT_PICK_PXL_39_79MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_39_79MHz_DIV_1_001
        #define BFMT_PICK_PXL_39_79MHz_DIV_1_001  1
    #endif
#endif

#if BFMT_PICK_eDVI_1024x768p
    #ifndef BFMT_PICK_PXL_65MHz
        #define BFMT_PICK_PXL_65MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_65MHz_DIV_1_001
        #define BFMT_PICK_PXL_65MHz_DIV_1_001  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x768p
    #ifndef BFMT_PICK_PXL_65_286MHz
        #define BFMT_PICK_PXL_65_286MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_65_286MHz_DIV_1_001
        #define BFMT_PICK_PXL_65_286MHz_DIV_1_001  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x768p_Red
    #ifndef BFMT_PICK_PXL_65_286MHz
        #define BFMT_PICK_PXL_65_286MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_65_286MHz_DIV_1_001
        #define BFMT_PICK_PXL_65_286MHz_DIV_1_001  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x720p_50Hz
    #ifndef BFMT_PICK_PXL_60_4656MHz
        #define BFMT_PICK_PXL_60_4656MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x720p
    #ifndef BFMT_PICK_PXL_74_25MHz
        #define BFMT_PICK_PXL_74_25MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_74_25MHz_DIV_1_001
        #define BFMT_PICK_PXL_74_25MHz_DIV_1_001  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x720p_Red
    #ifndef BFMT_PICK_PXL_64_022MHz
        #define BFMT_PICK_PXL_64_022MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_64_022MHz_DIV_1_001
        #define BFMT_PICK_PXL_64_022MHz_DIV_1_001  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x350p_60Hz
    #ifndef BFMT_PICK_PXL_25_2MHz_DIV_1_001
        #define BFMT_PICK_PXL_25_2MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_25_2MHz
        #define BFMT_PICK_PXL_25_2MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x350p_70Hz
    #ifndef BFMT_PICK_PXL_25_2MHz_DIV_1_001
        #define BFMT_PICK_PXL_25_2MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_25_2MHz
        #define BFMT_PICK_PXL_25_2MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x350p_72Hz
    #ifndef BFMT_PICK_PXL_25_2MHz_DIV_1_001
        #define BFMT_PICK_PXL_25_2MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_25_2MHz
        #define BFMT_PICK_PXL_25_2MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x350p_75Hz
    #ifndef BFMT_PICK_PXL_25_2MHz_DIV_1_001
        #define BFMT_PICK_PXL_25_2MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_25_2MHz
        #define BFMT_PICK_PXL_25_2MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x350p_85Hz
    #ifndef BFMT_PICK_PXL_31_50MHz
        #define BFMT_PICK_PXL_31_50MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x400p_60Hz
    #ifndef BFMT_PICK_PXL_25_2MHz_DIV_1_001
        #define BFMT_PICK_PXL_25_2MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_25_2MHz
        #define BFMT_PICK_PXL_25_2MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x400p_70Hz
    #ifndef BFMT_PICK_PXL_25_2MHz_DIV_1_001
        #define BFMT_PICK_PXL_25_2MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_25_2MHz
        #define BFMT_PICK_PXL_25_2MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x400p_72Hz
    #ifndef BFMT_PICK_PXL_25_2MHz_DIV_1_001
        #define BFMT_PICK_PXL_25_2MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_25_2MHz
        #define BFMT_PICK_PXL_25_2MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x400p_75Hz
    #ifndef BFMT_PICK_PXL_25_2MHz_DIV_1_001
        #define BFMT_PICK_PXL_25_2MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_25_2MHz
        #define BFMT_PICK_PXL_25_2MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x400p_85Hz
    #ifndef BFMT_PICK_PXL_31_50MHz
        #define BFMT_PICK_PXL_31_50MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x480p_66Hz
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x480p_70Hz
    #ifndef BFMT_PICK_PXL_25_2MHz_DIV_1_001
        #define BFMT_PICK_PXL_25_2MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_25_2MHz
        #define BFMT_PICK_PXL_25_2MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x480p_72Hz
    #ifndef BFMT_PICK_PXL_31_50MHz
        #define BFMT_PICK_PXL_31_50MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x480p_75Hz
    #ifndef BFMT_PICK_PXL_31_50MHz
        #define BFMT_PICK_PXL_31_50MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_640x480p_85Hz
    #ifndef BFMT_PICK_PXL_36_00MHz
        #define BFMT_PICK_PXL_36_00MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_720x400p_60Hz
    #ifndef BFMT_PICK_PXL_40MHz
        #define BFMT_PICK_PXL_40MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_720x400p_70Hz
    #ifndef BFMT_PICK_PXL_31_50MHz
        #define BFMT_PICK_PXL_31_50MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_720x400p_72Hz
    #ifndef BFMT_PICK_PXL_40MHz
        #define BFMT_PICK_PXL_40MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_720x400p_75Hz
    #ifndef BFMT_PICK_PXL_40MHz
        #define BFMT_PICK_PXL_40MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_720x400p_85Hz
    #ifndef BFMT_PICK_PXL_35_50MHz
        #define BFMT_PICK_PXL_35_50MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_800x600p_56Hz
    #ifndef BFMT_PICK_PXL_36_00MHz
        #define BFMT_PICK_PXL_36_00MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_800x600p_59Hz_Red
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_800x600p_70Hz
    #ifndef BFMT_PICK_PXL_50_00MHz
        #define BFMT_PICK_PXL_50_00MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_800x600p_72Hz
    #ifndef BFMT_PICK_PXL_50_00MHz
        #define BFMT_PICK_PXL_50_00MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_800x600p_75Hz
    #ifndef BFMT_PICK_PXL_49_50MHz
        #define BFMT_PICK_PXL_49_50MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_800x600p_85Hz
    #ifndef BFMT_PICK_PXL_56_25MHz
        #define BFMT_PICK_PXL_56_25MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1024x768p_66Hz
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1024x768p_70Hz
    #ifndef BFMT_PICK_PXL_75_00MHz
        #define BFMT_PICK_PXL_75_00MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1024x768p_72Hz
    #ifndef BFMT_PICK_PXL_65MHz
        #define BFMT_PICK_PXL_65MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1024x768p_75Hz
    #ifndef BFMT_PICK_PXL_78_75MHz
        #define BFMT_PICK_PXL_78_75MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1024x768p_85Hz
    #ifndef BFMT_PICK_PXL_94_50MHz
        #define BFMT_PICK_PXL_94_50MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x720p_70Hz
    #ifndef BFMT_PICK_PXL_74_375MHz
        #define BFMT_PICK_PXL_74_375MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x720p_72Hz
    #ifndef BFMT_PICK_PXL_74_375MHz
        #define BFMT_PICK_PXL_74_375MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x720p_75Hz
    #ifndef BFMT_PICK_PXL_74_375MHz
        #define BFMT_PICK_PXL_74_375MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x720p_85Hz
    #ifndef BFMT_PICK_PXL_74_375MHz
        #define BFMT_PICK_PXL_74_375MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1024x768i_87Hz
    #ifndef BFMT_PICK_PXL_44_900MHz
        #define BFMT_PICK_PXL_44_900MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1152x864p_75Hz
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x768p_75Hz
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x768p_85Hz
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x800p_60Hz
    #ifndef BFMT_PICK_PXL_83_5MHz
        #define BFMT_PICK_PXL_83_5MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_83_5MHz_DIV_1_001
        #define BFMT_PICK_PXL_83_5MHz_DIV_1_001  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x960p_60Hz
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x960p_85Hz
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x1024p_60Hz
    #ifndef BFMT_PICK_PXL_108MHz
        #define BFMT_PICK_PXL_108MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_108MHz_DIV_1_001
        #define BFMT_PICK_PXL_108MHz_DIV_1_001  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x1024p_69Hz
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x1024p_75Hz
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1280x1024p_85Hz
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_832x624p_75Hz
    #ifndef BFMT_PICK_PXL_56_25MHz
        #define BFMT_PICK_PXL_56_25MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1360x768p_60Hz
    #ifndef BFMT_PICK_PXL_85_5MHz
        #define BFMT_PICK_PXL_85_5MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1366x768p_60Hz
    #ifndef BFMT_PICK_PXL_85_5MHz
        #define BFMT_PICK_PXL_85_5MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_85_5MHz_DIV_1_001
        #define BFMT_PICK_PXL_85_5MHz_DIV_1_001  1
    #endif
#endif

#if BFMT_PICK_eDVI_1400x1050p_60Hz
    #ifndef BFMT_PICK_PXL_121_75MHz
        #define BFMT_PICK_PXL_121_75MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1400x1050p_60Hz_Red
    #ifndef BFMT_PICK_PXL_101MHz
        #define BFMT_PICK_PXL_101MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1400x1050p_75Hz
    #ifndef BFMT_PICK_PXL_156MHz
        #define BFMT_PICK_PXL_156MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1600x1200p_60Hz
    #ifndef BFMT_PICK_PXL_162MHz
        #define BFMT_PICK_PXL_162MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1920x1080p_60Hz_Red
    #ifndef BFMT_PICK_PXL_138_625MHz
        #define BFMT_PICK_PXL_138_625MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_848x480p_60Hz
    #ifndef BFMT_PICK_PXL_31_50MHz
        #define BFMT_PICK_PXL_31_50MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1064x600p_60Hz
    #ifndef BFMT_PICK_PXL_36_00MHz
        #define BFMT_PICK_PXL_36_00MHz  1
    #endif
#endif

#if BFMT_PICK_eDVI_1440x900p_60Hz
    #ifndef BFMT_PICK_PXL_106_5MHz
        #define BFMT_PICK_PXL_106_5MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_106_5MHz_DIV_1_001
        #define BFMT_PICK_PXL_106_5MHz_DIV_1_001  1
    #endif
#endif

	/* SW7435-276: New format enums for 482/483 */
#if BFMT_PICK_e720x482_NTSC
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_27MHz_MUL_1_001
        #define BFMT_PICK_PXL_27MHz_MUL_1_001  1
    #endif
#endif

#if BFMT_PICK_e720x482_NTSC_J
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
    #ifndef BFMT_PICK_PXL_27MHz_MUL_1_001
        #define BFMT_PICK_PXL_27MHz_MUL_1_001  1
    #endif
#endif

#if BFMT_PICK_e720x483p
    #ifndef BFMT_PICK_PXL_27MHz_MUL_1_001
        #define BFMT_PICK_PXL_27MHz_MUL_1_001  1
    #endif
    #ifndef BFMT_PICK_PXL_27MHz
        #define BFMT_PICK_PXL_27MHz  1
    #endif
#endif

	/* custom formats placeholders */
#if BFMT_PICK_eCustom0
    #ifndef BFMT_PXL_148_5MHz_DIV_1_001
        #define BFMT_PXL_148_5MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PXL_148_5MHz
        #define BFMT_PXL_148_5MHz  1
    #endif
#endif

#if BFMT_PICK_eCustom1
    #ifndef BFMT_PXL_148_5MHz_DIV_1_001
        #define BFMT_PXL_148_5MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PXL_148_5MHz
        #define BFMT_PXL_148_5MHz  1
    #endif
#endif

#if BFMT_PICK_eCustom2
    #ifndef BFMT_PXL_148_5MHz_DIV_1_001
        #define BFMT_PXL_148_5MHz_DIV_1_001  1
    #endif
    #ifndef BFMT_PXL_148_5MHz
        #define BFMT_PXL_148_5MHz  1
    #endif
#endif

	/* Must be last */
	/*BFMT_P_MAKE_BLANK(eMaxCount)*/

/***************************************************************
 *
 * III. define default PICK for unpicked PXL_frq
 *
 ***************************************************************/

#ifndef BFMT_PICK_PXL_25_2MHz
    #define BFMT_PICK_PXL_25_2MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_25_2MHz_MUL_1_001
    #define BFMT_PICK_PXL_25_2MHz_MUL_1_001  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_25_2MHz_DIV_1_001
    #define BFMT_PICK_PXL_25_2MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* NTSC and 480p, NTSC is upsampled in VEC from 13.5MHz */
#ifndef BFMT_PICK_PXL_27MHz
    #define BFMT_PICK_PXL_27MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_27MHz_MUL_1_001
    #define BFMT_PICK_PXL_27MHz_MUL_1_001  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_27MHz_DIV_1_001
    #define BFMT_PICK_PXL_27MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* 1080i and 720p */
#ifndef BFMT_PICK_PXL_74_25MHz
    #define BFMT_PICK_PXL_74_25MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_74_25MHz_MUL_1_001
    #define BFMT_PICK_PXL_74_25MHz_MUL_1_001  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_74_25MHz_DIV_1_001
    #define BFMT_PICK_PXL_74_25MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* 1080p60 3D
 * Reuse bits. BFMT_PICK_PXL_39_79MHz and BFMT_PICK_PXL_39_79MHz_DIV_1_001
 * are used for input only */
#ifndef BFMT_PICK_PXL_297MHz
    #define BFMT_PICK_PXL_297MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_297MHz_DIV_1_001
    #define BFMT_PICK_PXL_297MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* UHD */
#ifndef BFMT_PICK_PXL_594MHz
    #define BFMT_PICK_PXL_594MHz  BFMT_PICK_NO
#endif

#ifndef BFMT_PICK_PXL_594MHz_DIV_1_001
	#define BFMT_PICK_PXL_594MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* VESA mode: 1024x768p @ 60/59.94Hz */
#ifndef BFMT_PICK_PXL_65MHz
    #define BFMT_PICK_PXL_65MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_65MHz_DIV_1_001
    #define BFMT_PICK_PXL_65MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* VESA mode: 1280x720p @ 50Hz */
#ifndef BFMT_PICK_PXL_60_4656MHz
    #define BFMT_PICK_PXL_60_4656MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_60_375MHz
    #define BFMT_PICK_PXL_60_375MHz  BFMT_PICK_NO
#endif

/* 480P output, with 54 MHz pixel sampling with pixel doubling */
#ifndef BFMT_PICK_PXL_54MHz
    #define BFMT_PICK_PXL_54MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_54MHz_MUL_1_001
    #define BFMT_PICK_PXL_54MHz_MUL_1_001  BFMT_PICK_NO
#endif

/* VESA mode: 800x600p @ 60/59.94Hz */
#ifndef BFMT_PICK_PXL_40MHz
    #define BFMT_PICK_PXL_40MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_39_79MHz
    #define BFMT_PICK_PXL_39_79MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_39_79MHz_DIV_1_001
    #define BFMT_PICK_PXL_39_79MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* VESA mode: 640x480p_CVT @ 60Hz */
#ifndef BFMT_PICK_PXL_23_75MHz
    #define BFMT_PICK_PXL_23_75MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_23_75MHz_DIV_1_001
    #define BFMT_PICK_PXL_23_75MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* VESA mode: 1280x800p @ 60Hz */
#ifndef BFMT_PICK_PXL_83_5MHz
    #define BFMT_PICK_PXL_83_5MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_83_5MHz_DIV_1_001
    #define BFMT_PICK_PXL_83_5MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* VESA mode: 1280x1024p @ 60Hz */
#ifndef BFMT_PICK_PXL_108MHz
    #define BFMT_PICK_PXL_108MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_108MHz_DIV_1_001
    #define BFMT_PICK_PXL_108MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* VESA mode: 1440x900p @ 60Hz */
#ifndef BFMT_PICK_PXL_106_5MHz
    #define BFMT_PICK_PXL_106_5MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_106_5MHz_DIV_1_001
    #define BFMT_PICK_PXL_106_5MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* VESA mode: 1360x768p @ 60Hz */
#ifndef BFMT_PICK_PXL_85_5MHz
    #define BFMT_PICK_PXL_85_5MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_85_5MHz_DIV_1_001
    #define BFMT_PICK_PXL_85_5MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* 1080p */
#ifndef BFMT_PICK_PXL_148_5MHz
    #define BFMT_PICK_PXL_148_5MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_148_5MHz_DIV_1_001
    #define BFMT_PICK_PXL_148_5MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* 1600x1200p_60Hz */
#ifndef BFMT_PICK_PXL_162MHz
    #define BFMT_PICK_PXL_162MHz  BFMT_PICK_NO
#endif

/* These are low priority, using the 4 msb bits as count instead of mask */
/* DTV: 1366x768p  */
#ifndef BFMT_PICK_PXL_56_304MHz
    #define BFMT_PICK_PXL_56_304MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_67_565MHz
    #define BFMT_PICK_PXL_67_565MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_67_565MHz_MUL_1_001
    #define BFMT_PICK_PXL_67_565MHz_MUL_1_001  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_67_565MHz_DIV_1_001
    #define BFMT_PICK_PXL_67_565MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* VESA mode: 1280x720p @ 60/59.94Hz Reduced Blanking */
#ifndef BFMT_PICK_PXL_64MHz
    #define BFMT_PICK_PXL_64MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_64_022MHz
    #define BFMT_PICK_PXL_64_022MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_64_022MHz_DIV_1_001
    #define BFMT_PICK_PXL_64_022MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* VESA mode: 1280x768p @ 60/59.94Hz */
#ifndef BFMT_PICK_PXL_65_286MHz
    #define BFMT_PICK_PXL_65_286MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_65_286MHz_DIV_1_001
    #define BFMT_PICK_PXL_65_286MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* VESA mode: 1280x720p @ 60/59.94Hz */
#ifndef BFMT_PICK_PXL_74_375MHz
    #define BFMT_PICK_PXL_74_375MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_74_48MHz
    #define BFMT_PICK_PXL_74_48MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_74_48MHz_DIV_1_001
    #define BFMT_PICK_PXL_74_48MHz_DIV_1_001  BFMT_PICK_NO
#endif

/* More PC mode pixel rate */
#ifndef BFMT_PICK_PXL_31_50MHz
    #define BFMT_PICK_PXL_31_50MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_35_50MHz
    #define BFMT_PICK_PXL_35_50MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_36_00MHz
    #define BFMT_PICK_PXL_36_00MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_49_50MHz
    #define BFMT_PICK_PXL_49_50MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_50_00MHz
    #define BFMT_PICK_PXL_50_00MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_56_25MHz
    #define BFMT_PICK_PXL_56_25MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_75_00MHz
    #define BFMT_PICK_PXL_75_00MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_78_75MHz
    #define BFMT_PICK_PXL_78_75MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_94_50MHz
    #define BFMT_PICK_PXL_94_50MHz  BFMT_PICK_NO
#endif

#ifndef BFMT_PICK_PXL_101MHz
    #define BFMT_PICK_PXL_101MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_121_75MHz
    #define BFMT_PICK_PXL_121_75MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_156MHz
    #define BFMT_PICK_PXL_156MHz  BFMT_PICK_NO
#endif

/* VESA mode: 1920x1080p@60Hz_Red */
#ifndef BFMT_PICK_PXL_138_625MHz
    #define BFMT_PICK_PXL_138_625MHz  BFMT_PICK_NO
#endif

/* 1366x768@60 */
#ifndef BFMT_PICK_PXL_72_014MHz
    #define BFMT_PICK_PXL_72_014MHz  BFMT_PICK_NO
#endif

/* 1024x768i@87 */
#ifndef BFMT_PICK_PXL_44_900MHz
    #define BFMT_PICK_PXL_44_900MHz  BFMT_PICK_NO
#endif

#ifndef BFMT_PICK_PXL_75_5_MHz
    #define BFMT_PICK_PXL_75_5_MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_75_5_MHz_DIV_1_001
    #define BFMT_PICK_PXL_75_5_MHz_DIV_1_001  BFMT_PICK_NO
#endif

#ifndef BFMT_PICK_PXL_81_0MHz
    #define BFMT_PICK_PXL_81_0MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_81_0MHz_MUL_1_001
    #define BFMT_PICK_PXL_81_0MHz_MUL_1_001  BFMT_PICK_NO
#endif

#ifndef BFMT_PICK_PXL_222_75MHz
    #define BFMT_PICK_PXL_222_75MHz  BFMT_PICK_NO
#endif
#ifndef BFMT_PICK_PXL_222_75MHz_DIV_1_001
    #define BFMT_PICK_PXL_222_75MHz_DIV_1_001  BFMT_PICK_NO
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BFMT_PICKH__ */
/* End of File */
