/***************************************************************************
 * Copyright (C) 2019 Broadcom.
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
 *
 * Module Description:
 *
 ***************************************************************************/


#include "bavc.h"
#include "bavc_hdmi.h"


BDBG_MODULE(BAVC_HDMI) ;


static const char EnumError[] = BDBG_STRING_INLINE("Enum Error") ;
static const char Reserved[]  = BDBG_STRING_INLINE("Reserved") ;
static const char Unknown[]   = BDBG_STRING_INLINE("Unknown") ;

typedef struct
{
    BAVC_HDMI_PacketType ePacketType ;
    const char *PacketName ;
} BAVC_HDMI_PacketTypes ;

static const BAVC_HDMI_PacketTypes PacketTypes_Text[] =
{
    {BAVC_HDMI_PacketType_eAudioClockRegeneration, BDBG_STRING("Audio Clock Regeneration")},
    {BAVC_HDMI_PacketType_eAudioSample, BDBG_STRING("Audio Sample")},
    {BAVC_HDMI_PacketType_eGeneralControl, BDBG_STRING("General Control")},
    {BAVC_HDMI_PacketType_eAudioContentProtection, BDBG_STRING("ACP")},
    {BAVC_HDMI_PacketType_eISRC1, BDBG_STRING("ISRC1")},
    {BAVC_HDMI_PacketType_eISRC2, BDBG_STRING("ISRC2")},
    {BAVC_HDMI_PacketType_eDirectStream, BDBG_STRING("One Bit Audio Stream")},
    {BAVC_HDMI_PacketType_eHbrAudioPacket, BDBG_STRING("High Bitrate (HBR) Audio")},

#if BAVC_HDMI_1_3_SUPPORT
    {BAVC_HDMI_PacketType_eGamutMetadataPacket, BDBG_STRING("Gamut Metadata")},
#endif

    {BAVC_HDMI_PacketType_eVendorSpecificInfoframe, BDBG_STRING("Vendor Specific InfoFrame")},
    {BAVC_HDMI_PacketType_eAviInfoFrame, BDBG_STRING("AVI InfoFrame")},
    {BAVC_HDMI_PacketType_eSpdInfoFrame, BDBG_STRING("SPD InfoFrame")},
    {BAVC_HDMI_PacketType_eAudioInfoFrame, BDBG_STRING("Audio InfoFrame")},
    {BAVC_HDMI_PacketType_eDrmInfoFrame, BDBG_STRING("Dynamic Range Metadata InfoFrame")},
    {BAVC_HDMI_PacketType_eMpegInfoFrame, BDBG_STRING("MPEG Source InfoFrame")}
} ;



static const char * const BAVC_HDMI_AviInfoFrame_Text_Y1Y0[] =
{
    BDBG_STRING("RGB (Default)"),
    BDBG_STRING("YCbCr 4:2:2"),
    BDBG_STRING("YCbCr 4:4:4"),
    BDBG_STRING("YCbCr 4:2:0")
} ;

static const char * const BAVC_HDMI_AviInfoFrame_Text_A0[] =
{
    BDBG_STRING("No Data"),
    BDBG_STRING("Valid Data")
} ;

static const char * const BAVC_HDMI_AviInfoFrame_Text_B1B0[] =
{
    BDBG_STRING("Invalid Bar Data"),
    BDBG_STRING("Valid Vert Bar Info"),
    BDBG_STRING("Vaild Horz Bar Info"),
    BDBG_STRING("Valid Bar Info")
} ;


static const char * const BAVC_HDMI_AviInfoFrame_Text_S1S0[] =
{
    BDBG_STRING("No Data"),
    BDBG_STRING("Overscan"),
    BDBG_STRING("Underscan"),
    BDBG_STRING("Future")
} ;


static const char * const BAVC_HDMI_AviInfoFrame_Text_C1C0[] =
{
    BDBG_STRING("No Data"),
    BDBG_STRING("ITU601"),
    BDBG_STRING("ITU709"),
    BDBG_STRING("Extended Colorimetry Info Valid")
} ;


static const char * const BAVC_HDMI_AviInfoFrame_Text_M1M0[] =
{
    BDBG_STRING("No Data"),
    BDBG_STRING("4:3"),
    BDBG_STRING("16:9"),
    BDBG_STRING("Future")
} ;


static const char * const BAVC_HDMI_AviInfoFrame_Text_R3_R0[] =
{
    BDBG_STRING("Same as Picture AR"),
    BDBG_STRING("4:3 Center"),
    BDBG_STRING("16:9 Center"),
    BDBG_STRING("14:9 Center"),
    BDBG_STRING("4:3 Alternative 14:9 Center"),
    BDBG_STRING("16:9 Alternative 14:9 Center"),
    BDBG_STRING("16:9 Alternative 4:3 Center")
} ;


static const char * const BAVC_HDMI_AviInfoFrame_Text_ITC[] =
{
    BDBG_STRING("No Data"),
    BDBG_STRING("IT content"),
} ;


static const char * const BAVC_HDMI_AviInfoFrame_Text_EC2_EC0[] =
{
    BDBG_STRING("xvYCC 601"),
    BDBG_STRING("xvYCC 709"),
    BDBG_STRING("sYCC 601"),
    BDBG_STRING("AdobeYCC 601"),
    BDBG_STRING("AdobeYCC RGB"),
    BDBG_STRING("ITU-R BT.2020 Y'C'bcC'rc"),
    BDBG_STRING("ITU-R BT.2020 RGB/Y'C'bC'r"),
    BDBG_STRING("Reserved")
} ;


static const char * const BAVC_HDMI_AviInfoFrame_Text_Q1Q0[] =
{
    BDBG_STRING("Based on format"),
    BDBG_STRING("Limited Range"),
    BDBG_STRING("Full Range"),
    BDBG_STRING("Reserved")
} ;


static const char * const BAVC_HDMI_AviInfoFrame_Text_SC1SC0[] =
{
    BDBG_STRING("No Known non-uniform scaling"),
    BDBG_STRING("Picture Horizontally Scaled"),
    BDBG_STRING("Picture Vertically Scaled"),
    BDBG_STRING("Picute Scaled Vertically and Horizontally")
} ;

static const char * const BAVC_HDMI_AviInfoFrame_Text_VIC[] =
{
    BDBG_STRING("Unknown/IT Format"),
    BDBG_STRING("640x480p  59.94Hz/60Hz"),
    BDBG_STRING("720x480p  59.94Hz/60Hz"),
    BDBG_STRING("720x480p  59.94Hz/60Hz"),
    BDBG_STRING("1280x720p  59.94Hz/60Hz"),
    BDBG_STRING("1920x1080i  59.94Hz/60Hz"),
    BDBG_STRING("720(1440)x480i 59.94Hz/60Hz"),
    BDBG_STRING("720(1440)x480i 59.94Hz/60Hz"),
    BDBG_STRING("720(1440)x240p 59.94Hz/60Hz"),
    BDBG_STRING("720(1440)x240p 59.94Hz/60Hz"),
    BDBG_STRING("2880x480i  59.94Hz/60Hz"),
    BDBG_STRING("2880x480i  59.94Hz/60Hz"),
    BDBG_STRING("2880x240p  59.94Hz/60Hz"),
    BDBG_STRING("2880x240p  59.94Hz/60Hz"),
    BDBG_STRING("1440x480p  59.94Hz/60Hz"),
    BDBG_STRING("1440x480p  59.94Hz/60Hz"),
    BDBG_STRING("1920x1080p 59.94Hz/60Hz"),
    BDBG_STRING("720x576p 50Hz"),
    BDBG_STRING("720x576p 50Hz"),
    BDBG_STRING("1280x720p 50Hz"),
    BDBG_STRING("1920x1080i 50Hz"),
    BDBG_STRING("720(1440)x576i 50Hz"),
    BDBG_STRING("720(1440)x576i 50Hz"),
    BDBG_STRING("720(1440)x288p 50Hz"),
    BDBG_STRING("720(1440)x288p 50Hz"),
    BDBG_STRING("2880x576i 50Hz"),
    BDBG_STRING("2880x576i 50Hz"),
    BDBG_STRING("2880x288p 50Hz"),
    BDBG_STRING("2880x288p 50Hz"),
    BDBG_STRING("1440x576p 50Hz"),
    BDBG_STRING("1440x576p 50Hz"),
    BDBG_STRING("1920x1080p 50Hz"),
    BDBG_STRING("1920x1080p 23.97Hz/24Hz"),
    BDBG_STRING("1920x1080p 25Hz"),
    BDBG_STRING("1920x1080p 29.97Hz/30Hz"),
    BDBG_STRING("2880x480p 59.94Hz/60Hz"),
    BDBG_STRING("2880x480p 59.94Hz/60Hz"),
    BDBG_STRING("2880x576p 50Hz"),
    BDBG_STRING("2880x576p 50Hz"),
    BDBG_STRING("1920x1080i (1250 total) 50Hz"),
    BDBG_STRING("1920x1080i 100Hz"),
    BDBG_STRING("1280x720p  100Hz"),
    BDBG_STRING("720x576p 100Hz"),
    BDBG_STRING("720x576p 100Hz"),
    BDBG_STRING("720(1440)x576i 100Hz"),
    BDBG_STRING("720(1440)x576i 100Hz"),
    BDBG_STRING("1920x1080i 119.88/120Hz"),
    BDBG_STRING("1280x720p  119.88/120Hz"),
    BDBG_STRING("720x480p   119.88/120Hz"),
    BDBG_STRING("720x480p   119.88/120Hz"),
    BDBG_STRING("720(1440)x480i 119.88/120Hz"),
    BDBG_STRING("720(1440)x480i 119.88/120Hz"),
    BDBG_STRING("720x576p 200Hz"),
    BDBG_STRING("720x576p 200Hz"),
    BDBG_STRING("720(1440)x576i 200Hz"),
    BDBG_STRING("720(1440)x576i 200Hz"),
    BDBG_STRING("720x480p 239.76/240Hz"),
    BDBG_STRING("720x480p 239.76/240Hz"),
    BDBG_STRING("720(1440)x480i 239.76/240Hz"),
    BDBG_STRING("720(1440)x480i 239.76/240Hz"),
    BDBG_STRING("1280x720p 23.97Hz/24Hz"),
    BDBG_STRING("1280x720p 25Hz"),
    BDBG_STRING("1280x720p 29.97Hz/30Hz"),
    BDBG_STRING("1920x1080p 119.88/120Hz"),
    BDBG_STRING("1920x1080p 100Hz"),
    BDBG_STRING("1280x720p 23.98Hz/24Hz"),
    BDBG_STRING("1280x720p 25Hz"),
    BDBG_STRING("1280x720p 29.97Hz/30Hz"),
    BDBG_STRING("1280x720p 50Hz"),
    BDBG_STRING("1280x720p 59.94Hz/60Hz"),
    BDBG_STRING("1280x720p 100Hz"),
    BDBG_STRING("1280x720p 119.88/120Hz"),
    BDBG_STRING("1920x1080p 23.98Hz/24Hz"),
    BDBG_STRING("1920x1080p 25Hz"),
    BDBG_STRING("1920x1080p 29.97Hz/30Hz"),
    BDBG_STRING("1920x1080p 50Hz"),
    BDBG_STRING("1920x1080p 59.94Hz/60Hz"),
    BDBG_STRING("1920x1080p 100Hz"),
    BDBG_STRING("1920x1080p 119.88/120Hz"),
    BDBG_STRING("1680x720p 23.98Hz/24Hz"),
    BDBG_STRING("1680x720p 25Hz"),
    BDBG_STRING("1680x720p 29.97Hz/30Hz"),
    BDBG_STRING("1680x720p 50Hz"),
    BDBG_STRING("1680x720p 59.94Hz/60Hz"),
    BDBG_STRING("1680x720p 100Hz"),
    BDBG_STRING("1680x720p 119.88/120Hz"),
    BDBG_STRING("2560x1080p 23.98Hz/24Hz"),
    BDBG_STRING("2560x1080p 25Hz"),
    BDBG_STRING("2560x1080p 29.97Hz/30Hz"),
    BDBG_STRING("2560x1080p 50Hz"),
    BDBG_STRING("2560x1080p 59.94Hz/60Hz"),
    BDBG_STRING("2560x1080p 100Hz"),
    BDBG_STRING("2560x1080p 119.88/120Hz"),
    BDBG_STRING("3840x2160p 23.98Hz/24Hz"),
    BDBG_STRING("3840x2160p 25Hz"),
    BDBG_STRING("3840x2160p 29.97Hz/30Hz"),
    BDBG_STRING("3840x2160p 50Hz"),
    BDBG_STRING("3840x2160p 59.94Hz/60Hz"),
    BDBG_STRING("4096x2160p 23.98Hz/24Hz"),
    BDBG_STRING("4096x2160p 25Hz"),
    BDBG_STRING("4096x2160p 29.97Hz/30Hz"),
    BDBG_STRING("4096x2160p 50Hz"),
    BDBG_STRING("4096x2160p 59.94Hz/60Hz"),
    BDBG_STRING("3840x2160p 23.98Hz/24Hz"),
    BDBG_STRING("3840x2160p 25Hz"),
    BDBG_STRING("3840x2160p 29.97Hz/30Hz"),
    BDBG_STRING("3840x2160p 50Hz"),
    BDBG_STRING("3840x2160p 59.94Hz/60Hz"),
} ;


static const char * const BAVC_HDMI_AviInfoFrame_Text_YQ1YQ0[] =
{
    BDBG_STRING("Limited Range"),
    BDBG_STRING("Full Range"),
    BDBG_STRING("Reserved (2)"),
    BDBG_STRING("Reserved(3)")
} ;


static const char * const BAVC_HDMI_AviInfoFrame_Text_CN1CN0[] =
{
    BDBG_STRING("Graphics"),
    BDBG_STRING("Photo"),
    BDBG_STRING("Cinema"),
    BDBG_STRING("Game")
} ;



/************************************* Audio Info Frame ****************************************
***********************************************************************************************/

static const char * const BAVC_HDMI_AudioInfoFrame_Text_CT3_CT0[] =
{
    BDBG_STRING("Refer To Stream Header"),
    BDBG_STRING("PCM [24, 25]"),
    BDBG_STRING("AC3"),
    BDBG_STRING("MPEG1"),
    BDBG_STRING("MPEG 2 [multi-chan]"),
    BDBG_STRING("AAC"),
    BDBG_STRING("DTS"),
    BDBG_STRING("ATRAC"),
    BDBG_STRING("One-bit"),
    BDBG_STRING("DDP"),
    BDBG_STRING("DTS-HD"),
    BDBG_STRING("MAT[MLP]"),
    BDBG_STRING("DST"),
    BDBG_STRING("WMA Pro"),
    BDBG_STRING("Reserved")
} ;


static const char * const BAVC_HDMI_AudioInfoFrame_Text_CC2_CC0[] =
{
    BDBG_STRING("Refer To Stream Header"),  BDBG_STRING("2 ch"),
    BDBG_STRING("3 ch"),    BDBG_STRING("4 ch"),
    BDBG_STRING("5 ch"),    BDBG_STRING("6 ch"),
    BDBG_STRING("7 ch"),    BDBG_STRING("8 ch")
} ;

static const char * const BAVC_HDMI_AudioInfoFrame_Text_SF2_SF0[] =
{
    BDBG_STRING("Refer To Stream Header"),
    BDBG_STRING("32 kHz"),
    BDBG_STRING("44.1 kHz (CD)"),
    BDBG_STRING("48 kHz"),
    BDBG_STRING("88.2 kHz"),
    BDBG_STRING("96 kHz"),
    BDBG_STRING("176.4 kHz"),
    BDBG_STRING("192 kHz"),
} ;

static const char * const BAVC_HDMI_AudioInfoFrame_Text_SS1SS0[] =
{
    BDBG_STRING("Refer To Stream Header"),
    BDBG_STRING("16 bit"),
    BDBG_STRING("20 bit"),
    BDBG_STRING("24 bit")
} ;

static const char * const BAVC_HDMI_AudioInfoFrame_Text_LSV3_LSV0[] =
{
    BDBG_STRING("0 db"),    BDBG_STRING("1 db"),    BDBG_STRING("2 db"),    BDBG_STRING("3 db"),
    BDBG_STRING("4 db"),    BDBG_STRING("5 db"),    BDBG_STRING("6 db"),    BDBG_STRING("7 db"),
    BDBG_STRING("8 db"),    BDBG_STRING("9 db"),    BDBG_STRING("10 db"),   BDBG_STRING("11 db"),
    BDBG_STRING("12 db"),   BDBG_STRING("13 db"),   BDBG_STRING("14 db"),   BDBG_STRING("15 db")
} ;

static const char * const BAVC_HDMI_AudioInfoFrame_Text_DM[] =
{
    BDBG_STRING("Permitted"),
    BDBG_STRING("Prohibited")
} ;

static const char * const BAVC_HDMI_SpdInfoFrame_Text_SourceType[] =
{
    BDBG_STRING("unknown"),
    BDBG_STRING("Digital STB"),
    BDBG_STRING("DVD player"),
    BDBG_STRING("D-VHS"),
    BDBG_STRING("HDD Videorecorder"),
    BDBG_STRING("DVC"),
    BDBG_STRING("DSC"),
    BDBG_STRING("Video CD"),
    BDBG_STRING("Game"),
    BDBG_STRING("PC general"),
    BDBG_STRING("Blu-Ray Disc (BD)"),
    BDBG_STRING("Super Audio CD"),
    BDBG_STRING("HD DVD"),
    BDBG_STRING("PMP")
} ;


static const char * const BAVC_HDMI_VsInfoFrame_Text_HdmiVideoFormat[] =
{
    BDBG_STRING("No Add'l HDMI Format"),
    BDBG_STRING("Extended Resolution Format"),
    BDBG_STRING("3D Format")
} ;


static const char * const BAVC_HDMI_VsInfoFrame_Text_3DStructure[]=
{
    BDBG_STRING("FramePacking"),
    BDBG_STRING("FieldAlternative"),
    BDBG_STRING("LineAlternative"),
    BDBG_STRING("SidexSideFull"),
    BDBG_STRING("LDepth"),
    BDBG_STRING("LDepthGraphics"),
    BDBG_STRING("Top-and-Bottom"),
    BDBG_STRING("Reserved for future use"),
    BDBG_STRING("SidexSideHalf"),
    BDBG_STRING("3D Structure MAX")
} ;

static const char * const BAVC_HDMI_VsInfoFrame_Text_3DExtData[]=
{
    BDBG_STRING("None/HorzOLOR = 0"),
    BDBG_STRING("HorzOLER"),
    BDBG_STRING("HorzELOR"),
    BDBG_STRING("HorzELER"),
    BDBG_STRING("QuinOLOR"),
    BDBG_STRING("QuinOLER"),
    BDBG_STRING("QuinELOR"),
    BDBG_STRING("QuinELER"),
    BDBG_STRING("Max")
} ;


static const char * const BAVC_HDMI_VsInfoFrame_Text_HDMIVIC[]=
{
    BDBG_STRING("Reserved"),
    BDBG_STRING("4Kx2K_2997_30Hz"),
    BDBG_STRING("4Kx2K_25Hz"),
    BDBG_STRING("4Kx2K_2398_24Hz"),
    BDBG_STRING("4Kx2K_SMPTE_24Hz"),
    BDBG_STRING("MAX HDMI VICs")
} ;

#if !B_REFSW_MINIMAL
static const char * const BAVC_HDMI_GCP_ColorDepth_Text[]=
{
    BDBG_STRING("24 bpp*"),
    BDBG_STRING("24 bpp"),
    BDBG_STRING("30 bpp"),
    BDBG_STRING("36 bpp"),
    BDBG_STRING("48 bpp"),
    BDBG_STRING("Unknown bpp")
} ;
#endif

static const char * const BAVC_HDMI_BitsPerPixel_Text[]=
{
    BDBG_STRING("24 bpp"),
    BDBG_STRING("30 bpp"),
    BDBG_STRING("36 bpp"),
    BDBG_STRING("48 bpp")
} ;

static const char * const BAVC_HDMI_DRMInfoFrame_Text_EOTF[] =
{
    BDBG_STRING("SDR"),
    BDBG_STRING("HDR Gamma"),
    BDBG_STRING("HDR10 PQ"),
    BDBG_STRING("HLG")
} ;


static const char * const BAVC_HDMI_DRMInfoFrame_Text_DescriptorId[] =
{
    BDBG_STRING("Static Metadata Type 1")
} ;


static const char * const BAVC_HDMI_EDID_CeaTagName[] =
{
	"Reserved0  ",
	"Audio    DB",
	"Video    DB",
	"Vendor Specific DB",
	"Speaker  DB",
	"Reserved5  ",
	"Reserved6  ",
	"Extended DB"
} ;


typedef struct
{
	uint8_t uiTagId ;
	const char *ucName ;
} BAVC_HDMI_EDID_CeaExtendedDB ;

static const BAVC_HDMI_EDID_CeaExtendedDB BAVC_HDMI_EDID_ExtendedDataBlocks[] =
{
	{0, BDBG_STRING("Video Capability Data Block")},
	{1, BDBG_STRING("Vendor-Specific Video Data Block")},
	{2, BDBG_STRING("VESA Display Device Data Block")},
	{3, BDBG_STRING("VESA Video Timing Block Extension")},
	{4, BDBG_STRING("Reserved for HDMI Video Data Block")},
	{5, BDBG_STRING("Colorimetry Data Block")},
	{6, BDBG_STRING("HDR Static Metadata Data Block")},
	{7, BDBG_STRING("Reserved")}, /* 7 */
	{8, BDBG_STRING("Reserved")},
	{9, BDBG_STRING("Reserved")},
	{10, BDBG_STRING("Reserved")},
	{11, BDBG_STRING("Reserved")},
	{12, BDBG_STRING("Reserved")}, /* 12 */
	{13, BDBG_STRING("Video Format Preference Data Block")},
	{14, BDBG_STRING("YCbCr 4:2:0 Video Data Block")},
	{15, BDBG_STRING("YCbCr 4:2:0 Capability Map Data Block")},
	{16, BDBG_STRING("Reserved for CEA Miscellaneous Audio Fields")},
	{17, BDBG_STRING("Vendor-Specific Audio Data Block")},
	{18, BDBG_STRING("Reserved for HDMI Audio Data Block")},
	{120, BDBG_STRING("EDID Extension Override Data Block")},
	{121, BDBG_STRING("HF Sink Capability Data Block")}
} ;


typedef struct
{
	BAVC_HDMI_IEEERegId id ;
	uint8_t ucpOUI[3] ;
} BAVC_HDMI_IEEE_RegID ;

static const BAVC_HDMI_IEEE_RegID IEEE_RegistrationIDs[] =
{
	/* LSB.. MSB */
	{BAVC_HDMI_IEEERegId_eUnknown, { 0x00, 0x00, 0x00 }},  /* Unknown */
	{BAVC_HDMI_IEEERegId_eVSDB,    { 0x03, 0x0C, 0x00 }},  /* HDMI */
	{BAVC_HDMI_IEEERegId_eHF_VSDB, { 0xD8, 0x5D, 0xC4 }},  /* HDMI Forum */
	{BAVC_HDMI_IEEERegId_eDolby,   { 0x46, 0xD0, 0x00 }},  /* Dolby */
	{BAVC_HDMI_IEEERegId_eHDR10P,  { 0x8B, 0x84, 0x90 }},  /* HDR10+ */
} ;


#if !B_REFSW_MINIMAL
void BAVC_HDMI_GetDefaultEDID(BAVC_HDMI_EDID *pstEdid)
{
    BSTD_UNUSED(pstEdid) ;
    BDBG_ERR(("BAVC_HDMI_GetDefaultEDID is deprecated")) ;
}
#endif

/******************************************************************************
Summary:
Return character string for Colorspace (Y1Y0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(
    BAVC_HDMI_AviInfoFrame_Colorspace uiY1Y0)
{
    uint8_t entries ;

    entries =
        sizeof(BAVC_HDMI_AviInfoFrame_Text_Y1Y0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_Y1Y0) ;

    if (uiY1Y0 < entries)
        return BAVC_HDMI_AviInfoFrame_Text_Y1Y0[uiY1Y0] ;
    else
        return EnumError ;
}


/******************************************************************************
Summary:
Return character string for Active Format (A0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_ActiveFormatToStr(
    BAVC_HDMI_AviInfoFrame_ActiveInfo uiA0)
{
    uint8_t entries ;

    entries =
        sizeof(BAVC_HDMI_AviInfoFrame_Text_A0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_A0) ;

    if (uiA0 < entries)
        return BAVC_HDMI_AviInfoFrame_Text_A0[uiA0] ;
    else
        return EnumError ;
}


/******************************************************************************
Summary:
Return character string for Bar Info (B1B0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_BarInfoToStr(
    BAVC_HDMI_AviInfoFrame_BarInfo uiB1B0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_B1B0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_B1B0) ;

    if (uiB1B0 < entries)
        return BAVC_HDMI_AviInfoFrame_Text_B1B0[uiB1B0] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Scan Information (S1S0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_ScanInfoToStr(
    BAVC_HDMI_AviInfoFrame_ScanInfo uiS1S0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_S1S0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_S1S0) ;

    if (uiS1S0 < entries)
        return BAVC_HDMI_AviInfoFrame_Text_S1S0[uiS1S0] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Colorimetry (C1C0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_ColorimetryToStr(
    BAVC_HDMI_AviInfoFrame_Colorimetry uiC1C0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_C1C0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_C1C0) ;

    if (uiC1C0 < entries)
        return BAVC_HDMI_AviInfoFrame_Text_C1C0[uiC1C0] ;
    else
        return EnumError;
}

/******************************************************************************
Summary:
Return character string for Picture Aspect Ratio (M1M0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_PictureAspectRatioToStr(
    BAVC_HDMI_AviInfoFrame_PictureAspectRatio uiM1M0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_M1M0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_M1M0) ;

    if (uiM1M0 < entries)
        return BAVC_HDMI_AviInfoFrame_Text_M1M0[uiM1M0] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Active Format Aspect Ratio (R3_R0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(
    BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio uiR3_R0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_R3_R0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_R3_R0) ;

    if (uiR3_R0 < entries)
        return BAVC_HDMI_AviInfoFrame_Text_R3_R0[uiR3_R0] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for IT Content (ITC) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_ITContentToStr(
    BAVC_HDMI_AviInfoFrame_ITContent uiITC)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_ITC) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_ITC) ;

    if (uiITC < entries)
        return BAVC_HDMI_AviInfoFrame_Text_ITC[uiITC] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Extended Colorimetry (EC2_EC0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(
    BAVC_HDMI_AviInfoFrame_ExtendedColorimetry uiEC2_EC0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_EC2_EC0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_EC2_EC0) ;

    if (uiEC2_EC0 < entries)
        return BAVC_HDMI_AviInfoFrame_Text_EC2_EC0[uiEC2_EC0] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for RGB Quantization Range (Q1Q0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_RGBQuantizationRangeToStr(
    BAVC_HDMI_AviInfoFrame_RGBQuantizationRange uiQ1Q0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_Q1Q0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_Q1Q0) ;

    if (uiQ1Q0 < entries)
        return BAVC_HDMI_AviInfoFrame_Text_Q1Q0[uiQ1Q0] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Scaling (SC1SC0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_ScalingToStr(
    BAVC_HDMI_AviInfoFrame_Scaling uiSC1SC0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_SC1SC0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_SC1SC0) ;

    if (uiSC1SC0 < entries)
        return BAVC_HDMI_AviInfoFrame_Text_SC1SC0[uiSC1SC0] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Video ID Code (VIC) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr_isrsafe(
    BAVC_HDMI_AviInfoFrame_Scaling uiVIC)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_VIC) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_VIC) ;

    if (uiVIC < entries)
        return BAVC_HDMI_AviInfoFrame_Text_VIC[uiVIC] ;
    else
        return EnumError;
}



/******************************************************************************
Summary:
Return character string for IT Content Type enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_ContentTypeToStr(
    BAVC_HDMI_AviInfoFrame_ContentType eContentType)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_CN1CN0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_CN1CN0) ;

    if (eContentType < entries)
        return BAVC_HDMI_AviInfoFrame_Text_CN1CN0[eContentType] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Ycc Quantization Range enumeration
*******************************************************************************/
const char * BAVC_HDMI_AviInfoFrame_YccQuantizationRangeToStr(
    BAVC_HDMI_AviInfoFrame_ContentType eYccQuantizationRange)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AviInfoFrame_Text_YQ1YQ0) /
        sizeof(*BAVC_HDMI_AviInfoFrame_Text_YQ1YQ0) ;

    if (eYccQuantizationRange < entries)
        return BAVC_HDMI_AviInfoFrame_Text_YQ1YQ0[eYccQuantizationRange] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Audio Coding Type (CT3_CT0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AudioInfoFrame_CodingTypeToStr(
    BAVC_HDMI_AudioInfoFrame_CodingType uiCT3_CT0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AudioInfoFrame_Text_CT3_CT0) /
        sizeof(*BAVC_HDMI_AudioInfoFrame_Text_CT3_CT0) ;

    if (uiCT3_CT0 < entries)
        return BAVC_HDMI_AudioInfoFrame_Text_CT3_CT0[uiCT3_CT0] ;
    else
        return EnumError;
}

/******************************************************************************
Summary:
Return character string for Audio Channel Count (CC2_CC0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AudioInfoFrame_ChannelCountToStr(
    BAVC_HDMI_AudioInfoFrame_ChannelCount uiCC2_CC0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AudioInfoFrame_Text_CC2_CC0) /
        sizeof(*BAVC_HDMI_AudioInfoFrame_Text_CC2_CC0) ;

    if (uiCC2_CC0 < entries)
        return BAVC_HDMI_AudioInfoFrame_Text_CC2_CC0[uiCC2_CC0] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Sampling Frequency (SF2_SF0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AudioInfoFrame_SampleFrequencyToStr(
    BAVC_HDMI_AudioInfoFrame_SampleFrequency uiSF2_SF0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AudioInfoFrame_Text_SF2_SF0) /
        sizeof(*BAVC_HDMI_AudioInfoFrame_Text_SF2_SF0) ;

    if (uiSF2_SF0 < entries)
        return BAVC_HDMI_AudioInfoFrame_Text_SF2_SF0[uiSF2_SF0] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Sample Size (SS1SS0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AudioInfoFrame_SampleSizeToStr(
    BAVC_HDMI_AudioInfoFrame_SampleSize uiSS1SS0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AudioInfoFrame_Text_SS1SS0) /
        sizeof(*BAVC_HDMI_AudioInfoFrame_Text_SS1SS0) ;

    if (uiSS1SS0 < entries)
        return BAVC_HDMI_AudioInfoFrame_Text_SS1SS0[uiSS1SS0] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Level Shift Values (LSV3_LSV0) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AudioInfoFrame_LevelShiftToStr(
    BAVC_HDMI_AudioInfoFrame_LevelShift uiLSV3_LSV0)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AudioInfoFrame_Text_LSV3_LSV0) /
        sizeof(*BAVC_HDMI_AudioInfoFrame_Text_LSV3_LSV0) ;

    if (uiLSV3_LSV0 < entries)
        return BAVC_HDMI_AudioInfoFrame_Text_LSV3_LSV0[uiLSV3_LSV0] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for Colorspace (DM) enumeration
*******************************************************************************/
const char * BAVC_HDMI_AudioInfoFrame_DownMixInhibitToStr(
    BAVC_HDMI_AudioInfoFrame_DownMixInhibit uiDM)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_AudioInfoFrame_Text_DM) /
        sizeof(*BAVC_HDMI_AudioInfoFrame_Text_DM) ;

    if (uiDM < entries)
        return BAVC_HDMI_AudioInfoFrame_Text_DM[uiDM] ;
    else
        return EnumError;
}


/******************************************************************************
Summary:
Return character string for HDMI Packet Type
*******************************************************************************/
const char * BAVC_HDMI_PacketTypeToStr_isrsafe(
    BAVC_HDMI_PacketType ePacketType)
{
    uint8_t entries, i ;


    entries =
        sizeof(PacketTypes_Text) /
        sizeof(*PacketTypes_Text) ;

    for (i = 0 ; i < entries; i++)
    {
        if (ePacketType == PacketTypes_Text[i].ePacketType)
            return PacketTypes_Text[i].PacketName;
    }
    return Unknown ;
}


/******************************************************************************
Summary:
Return character string for SPD Device Type enumeration
*******************************************************************************/
const char * BAVC_HDMI_SpdInfoFrame_SourceTypeToStr(
    BAVC_HDMI_SpdInfoFrame_SourceType eSourceType)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_SpdInfoFrame_Text_SourceType) /
        sizeof(*BAVC_HDMI_SpdInfoFrame_Text_SourceType) ;

    if (eSourceType < entries)
        return BAVC_HDMI_SpdInfoFrame_Text_SourceType[eSourceType] ;
    else
        return Reserved ;
}


/******************************************************************************
Summary:
Return character string for VS Hdmi Video Format  enumeration
*******************************************************************************/
const char * BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr_isrsafe(
    BAVC_HDMI_VSInfoFrame_HDMIVideoFormat eFormatType)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_VsInfoFrame_Text_HdmiVideoFormat   ) /
        sizeof(*BAVC_HDMI_VsInfoFrame_Text_HdmiVideoFormat) ;

    if (eFormatType < entries)
        return BAVC_HDMI_VsInfoFrame_Text_HdmiVideoFormat[eFormatType] ;
    else
        return Reserved ;
}

/******************************************************************************
Summary:
Return character string for VS 3D Structure
******************************************************************************/
const char * BAVC_HDMI_VsInfoFrame_3DStructureToStr(
    BAVC_HDMI_VSInfoFrame_3DStructure e3DStructure)
{
    uint8_t entries ;

    entries =
        sizeof(BAVC_HDMI_VsInfoFrame_Text_3DStructure) /
        sizeof(*BAVC_HDMI_VsInfoFrame_Text_3DStructure) ;

    if (e3DStructure < entries)
        return BAVC_HDMI_VsInfoFrame_Text_3DStructure[e3DStructure] ;
    else
        return Reserved ;
}


/******************************************************************************
Summary:
Return character string for VS 3D Ext Data enumeration
*******************************************************************************/
const char * BAVC_HDMI_VsInfoFrame_3DExtDataToStr(
    BAVC_HDMI_VSInfoFrame_3DExtData e3DExtType)
{
    uint8_t entries ;

    entries =
        sizeof(BAVC_HDMI_VsInfoFrame_Text_3DExtData) /
        sizeof(*BAVC_HDMI_VsInfoFrame_Text_3DExtData) ;

    if (e3DExtType < entries)
        return BAVC_HDMI_VsInfoFrame_Text_3DExtData[e3DExtType] ;
    else
        return Reserved ;
}


/******************************************************************************
Summary:
Return character string for VS 3D Ext Data enumeration
*******************************************************************************/
const char * BAVC_HDMI_VsInfoFrame_HdmiVicToStr(
    BAVC_HDMI_VSInfoFrame_HDMIVIC  eHdmiVic)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_VsInfoFrame_Text_HDMIVIC) /
        sizeof(*BAVC_HDMI_VsInfoFrame_Text_HDMIVIC) ;

    if (eHdmiVic < entries)
        return BAVC_HDMI_VsInfoFrame_Text_HDMIVIC[eHdmiVic] ;
    else
        return Reserved ;
}



const char * BAVC_HDMI_DRMInfoFrame_EOTFToStr(const BAVC_HDMI_DRM_EOTF eEOTF)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_DRMInfoFrame_Text_EOTF) /
        sizeof(*BAVC_HDMI_DRMInfoFrame_Text_EOTF) ;

    if (eEOTF < entries)
        return BAVC_HDMI_DRMInfoFrame_Text_EOTF[eEOTF] ;
    else
        return Reserved ;
}


const char * BAVC_HDMI_DRMInfoFrame_DescriptorIdToStr(const BAVC_HDMI_DRM_DescriptorId eDescriptorId)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_DRMInfoFrame_Text_DescriptorId) /
        sizeof(*BAVC_HDMI_DRMInfoFrame_Text_DescriptorId) ;

    if (eDescriptorId < entries)
        return BAVC_HDMI_DRMInfoFrame_Text_DescriptorId[eDescriptorId] ;
    else
        return Reserved ;
}


const char * BAVC_HDMI_EDID_CeaTagToStr(const uint8_t uiCeaTagId)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_EDID_CeaTagName) /
        sizeof(*BAVC_HDMI_EDID_CeaTagName) ;

    if (uiCeaTagId < entries)
        return BAVC_HDMI_EDID_CeaTagName[uiCeaTagId] ;
    else
        return Unknown ;
}

const char * BAVC_HDMI_EDID_CeaExtendedTagToStr(const uint8_t uiCeaExtTagId)
{
	unsigned i, entries ;

	entries =
		sizeof(BAVC_HDMI_EDID_ExtendedDataBlocks) /
		sizeof(*BAVC_HDMI_EDID_ExtendedDataBlocks) ;

	for (i=0 ; i < entries ; i++)
	{
		if (uiCeaExtTagId == BAVC_HDMI_EDID_ExtendedDataBlocks[i].uiTagId)
		{
			return BAVC_HDMI_EDID_ExtendedDataBlocks[i].ucName ;
		}
	}
	return Unknown ;
}

const uint8_t * BAVC_HDMI_GetOuiId(const BAVC_HDMI_IEEERegId eRegId)
{
    uint8_t i, entries ;

    entries=
        sizeof(IEEE_RegistrationIDs) /
        sizeof(*IEEE_RegistrationIDs) ;

	for (i = 0 ; i < entries ; i++)
	{
		if (eRegId == IEEE_RegistrationIDs[i].id)
		{
	        return IEEE_RegistrationIDs[eRegId].ucpOUI;
		}
	}

	/* return unknown OUI */
    return IEEE_RegistrationIDs[BAVC_HDMI_IEEERegId_eUnknown].ucpOUI;
}


#if !B_REFSW_MINIMAL
const char * BAVC_HDMI_GCP_ColorDepthToStr(
    BAVC_HDMI_GCP_ColorDepth  eColorDepth)
{
    uint8_t entries ;
    BAVC_HDMI_GCP_ColorDepth tableTextId ;

    entries=
        sizeof(BAVC_HDMI_GCP_ColorDepth_Text) /
        sizeof(*BAVC_HDMI_GCP_ColorDepth_Text) ;

    if ((eColorDepth == BAVC_HDMI_GCP_ColorDepth_eDepthNotIndicated)
    || (eColorDepth < 4))
        tableTextId = BAVC_HDMI_GCP_ColorDepth_eDepthNotIndicated ;
    else if ((eColorDepth >= 4) && (eColorDepth <= 7))
        tableTextId =  eColorDepth - 3 ;
    else /* eColorDepth > 7 */
        tableTextId = BAVC_HDMI_GCP_ColorDepth_eUnknown ;

    if (tableTextId < entries)
        return BAVC_HDMI_GCP_ColorDepth_Text[tableTextId] ;
    else
        return Reserved ;
}
#endif

const char * BAVC_HDMI_BitsPerPixelToStr(
    BAVC_HDMI_BitsPerPixel  eBitsPerPixel)
{
    uint8_t entries ;

    entries=
        sizeof(BAVC_HDMI_BitsPerPixel_Text) /
        sizeof(*BAVC_HDMI_BitsPerPixel_Text) ;


    if (eBitsPerPixel < entries)
        return BAVC_HDMI_BitsPerPixel_Text[eBitsPerPixel] ;
    else
        return Reserved ;
}

#if !B_REFSW_MINIMAL
/******************************************************************************
unsigned int BAVC_HDMI_HDCP_NumberOfSetBits
Summary: Get the number of Set Bits
*******************************************************************************/
unsigned int BAVC_HDMI_HDCP_NumberOfSetBits(const unsigned char *bytes, int nbytes)
{
    int i, j ;
    int bit ;
    int count = 0 ;
    uint8_t byteToCheck;

    count = 0 ;
    for (i = 0; i < nbytes; i++)
    {
        bit = 1 ;
        byteToCheck = bytes[i];
        for (j = 0; j < 8 ; j++)
        {
            if (bit & byteToCheck)
                count++ ;
            bit = bit << 1 ;
        }
    }
    return count ;
} /* end BAVC_HDMI_HDCP_NumberOfSetBits */
#endif
