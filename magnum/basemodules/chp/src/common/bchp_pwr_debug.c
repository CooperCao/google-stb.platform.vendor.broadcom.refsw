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

struct BCHP_PWR_CoreResources
{
    const char *pResourceName;
    unsigned resource;
};

struct BCHP_PWR_Cores {
    const char *pCoreName;
    const struct BCHP_PWR_CoreResources *map;
};

const struct BCHP_PWR_CoreResources AVD[] =
{
#ifdef BCHP_PWR_RESOURCE_AVD0_CLK
    {
        "Video Decoder 0 Clock",
        BCHP_PWR_RESOURCE_AVD0_CLK,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_AVD0_PWR
    {
        "Video Decoder 0 Sram",
        BCHP_PWR_RESOURCE_AVD0_PWR,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_AVD1_CLK
    {
        "Video Decoder 1 Clock",
        BCHP_PWR_RESOURCE_AVD1_CLK,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_AVD1_PWR
    {
        "Video Decoder 1 Sram",
        BCHP_PWR_RESOURCE_AVD1_PWR,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_AVD2_CLK
    {
        "Video Decoder 2 Clock",
        BCHP_PWR_RESOURCE_AVD2_CLK,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_AVD2_PWR
    {
        "Video Decoder 2 Sram",
        BCHP_PWR_RESOURCE_AVD2_PWR,
    },
#endif
    {0, 0}
};

const struct BCHP_PWR_CoreResources VICE[] =
{
#ifdef BCHP_PWR_RESOURCE_VICE0
    {
        "Video Encoder 0 Clock",
        BCHP_PWR_RESOURCE_VICE0_CLK,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_VICE0_PWR
    {
        "Video Encoder 0 Sram",
        BCHP_PWR_RESOURCE_VICE0_PWR,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_VICE1
    {
        "Video Encoder 1 Clock",
        BCHP_PWR_RESOURCE_VICE1_CLK,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_VICE1_PWR
    {
        "Video Encoder 1 Sram",
        BCHP_PWR_RESOURCE_VICE1_PWR,
    },
#endif
    {0, 0}
};

const struct BCHP_PWR_CoreResources AUD[] =
{
#ifdef BCHP_PWR_RESOURCE_AUD_AIO
    {
        "Aio Clock and Sram",
        BCHP_PWR_RESOURCE_AUD_AIO,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_DAC
    {
        "Audio DAC",
        BCHP_PWR_RESOURCE_AUD_DAC,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_RAAGA0
    {
        "Raaga 0 Clock",
        BCHP_PWR_RESOURCE_RAAGA0,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_RAAGA1
    {
        "Raaga 1 Clock",
        BCHP_PWR_RESOURCE_RAAGA1,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_RAAGA_SRAM
    {
        "Raaag Sram",
        BCHP_PWR_RESOURCE_RAAGA_SRAM,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL0
    {
        "Aud Pll 0",
        BCHP_PWR_RESOURCE_AUD_PLL0,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL1
    {
        "Aud Pll 1",
        BCHP_PWR_RESOURCE_AUD_PLL1,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL2
    {
        "Aud Pll 2",
        BCHP_PWR_RESOURCE_AUD_PLL2,
    },
#endif
    {0, 0}
};

const struct BCHP_PWR_CoreResources VDC[] =
{
#ifdef BCHP_PWR_RESOURCE_BVN
    {
        "BVN Clock and Sram",
        BCHP_PWR_RESOURCE_BVN,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_DAC
    {
        "Video Dac",
        BCHP_PWR_RESOURCE_VDC_DAC,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
    {
        "Vec Clock and Sram",
        BCHP_PWR_RESOURCE_VDC_VEC,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_656_OUT
    {
        "656 Out",
        BCHP_PWR_RESOURCE_VDC_656_OUT,
    },
#endif
    {0, 0}
};

const struct BCHP_PWR_CoreResources XPT[] =
{
#ifdef BCHP_PWR_RESOURCE_XPT_XMEMIF
    {
        "Transport Clock",
        BCHP_PWR_RESOURCE_XPT_XMEMIF,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
    {
        "Remux Clock",
        BCHP_PWR_RESOURCE_XPT_REMUX,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_XPT_WAKEUP
    {
        "Transport Wakeup",
        BCHP_PWR_RESOURCE_XPT_WAKEUP,
    },
#endif
    {0, 0}
};
const struct BCHP_PWR_CoreResources HDMI[] =
{
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CLK
    {
        "HDMI Tx 0 Clock",
        BCHP_PWR_RESOURCE_HDMI_TX_CLK,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_PHY
    {
        "HDMI Tx 0 Phy",
        BCHP_PWR_RESOURCE_HDMI_TX_PHY,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_1_CLK
    {
        "HDMI Tx 1 Clock",
        BCHP_PWR_RESOURCE_HDMI_TX_1_CLK,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_1_PHY
    {
        "HDMI Tx 1 Phy",
        BCHP_PWR_RESOURCE_HDMI_TX_1_PHY,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_HDMI_RX0_CLK
    {
        "HDMI Rx 0 Clock",
        BCHP_PWR_RESOURCE_HDMI_RX0_CLK,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_HDMI_RX0_PHY
    {
        "HDMI Rx 1 Clock",
        BCHP_PWR_RESOURCE_HDMI_RX0_PHY
    },
#endif
    {0, 0}
};

const struct BCHP_PWR_CoreResources GFX[] =
{
#ifdef BCHP_PWR_RESOURCE_M2MC0
    {
        "Graphics 2D 0 Clock",
        BCHP_PWR_RESOURCE_M2MC0,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_M2MC0_SRAM
    {
        "Graphics 2D 0 Sram",
        BCHP_PWR_RESOURCE_M2MC0_SRAM,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_M2MC1
    {
        "Graphics 2D 1 Clock",
        BCHP_PWR_RESOURCE_M2MC1,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_M2MC1_SRAM
    {
        "Graphics 2D 1 Sram",
        BCHP_PWR_RESOURCE_M2MC1_SRAM,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_GRAPHICS3D
    {
        "Graphics 3D Clock",
        BCHP_PWR_RESOURCE_GRAPHICS3D,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM
    {
        "Graphics 3D Sram",
        BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM,
    },
#endif
    {0, 0}
};

const struct BCHP_PWR_CoreResources SID[] =
{
#ifdef BCHP_PWR_RESOURCE_SID
    {
        "Picture Decoder Clock",
        BCHP_PWR_RESOURCE_SID,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_SID_SRAM
    {
        "Picture Decoder Sram",
        BCHP_PWR_RESOURCE_SID_SRAM,
    },
#endif
    {0, 0}
};

const struct BCHP_PWR_CoreResources SCD[] =
{
#ifdef BCHP_PWR_RESOURCE_SMARTCARD0
    {
        "Smart Card 0",
        BCHP_PWR_RESOURCE_SMARTCARD0,
    },
#endif
#ifdef BCHP_PWR_RESOURCE_SMARTCARD1
    {
        "Smart Card 1",
        BCHP_PWR_RESOURCE_SMARTCARD1,
    },
#endif
    {0, 0}
};

const struct BCHP_PWR_CoreResources RFM[] =
{
#ifdef BCHP_PWR_RESOURCE_RFM
    {
        "Rfm Clock and Phy",
        BCHP_PWR_RESOURCE_RFM,
    },
#endif
    {0, 0}
};

const struct BCHP_PWR_CoreResources UHF[] =
{
#ifdef BCHP_PWR_RESOURCE_UHF_INPUT
    {
        "Uhf Clock and Sram",
        BCHP_PWR_RESOURCE_UHF_INPUT,
    },
#endif
    {0, 0}
};

const struct BCHP_PWR_Cores cores[] =
{
    {"Video Decoder", AVD},
    {"Video Encoder", VICE},
    {"Audio", AUD},
    {"Display", VDC},
    {"Transport", XPT},
    {"Hdmi", HDMI},
    {"Graphics", GFX},
    {"Picture Decoder", SID},
    {"Smartcard", SCD},
    {"Rfm", RFM},
    {"Uhf", UHF}
};
