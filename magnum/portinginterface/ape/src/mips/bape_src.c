/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_src_ctrl0.h"

BDBG_MODULE(bape_src);

BERR_Code BAPE_P_InitSrc(
    BAPE_Handle handle
    )
{
    unsigned i;
    uint32_t regAddr, regVal, stride, mem=0;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    #if BCHP_CHIP == 7408 && BCHP_VER <= BCHP_VER_B0
    /* 7408 B0 has a HW bug in the SRC block that requires us to ignore the middle 4 SRCs, they can not use L_INT mode. */
    for ( i = 4; i < 8; i++ )
    {
        handle->srcAllocated[i] = true;
    }
    #endif

    /* We only use LINT mode, so we need to give each SRC 4 bytes of memory to work with. */
    regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0;
    stride = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG1 - BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0;
    for ( i = 0; i < BAPE_CHIP_MAX_SRCS; i++ )
    {
        regVal = 0;

        #if (BCHP_CHIP == 7408 && BCHP_VER <= BCHP_VER_B0)
        if ( i < 4 )
        {
            mem=i*4;
        }
        else if ( i < 8 )
        {
            mem = 0x20; /* non-functional address */
        }
        else 
        {
            switch ( i )
            {
            default:
            case 8:
                mem=0x14;
                break;
            case 9:
                mem=0x1c;
                break;
            case 10:
                mem=0x2c;
                break;
            case 11:
                mem=0x3c;
                break;
            }
        }
        #endif

        regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFG0, BASE_ADDR, mem);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_SRC_CFG0, TYPE, Bypass);
        BREG_Write32(handle->regHandle, regAddr, regVal);
        regAddr += stride;

        #if !(BCHP_CHIP == 7408 && BCHP_VER <= BCHP_VER_B0)
        mem += 4;
        #endif
    }

    /* Reset group IDs to default */
    regAddr = BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE;
    for ( i = 0; i < BAPE_CHIP_MAX_SRCS; i++ )
    {
        BREG_Write32(handle->regHandle, regAddr, BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, GROUP, i));
        regAddr += (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_ELEMENT_SIZE/8);
        BAPE_P_ProgramSRC_isr(handle, i, 0, 0);
    }

    for ( i = 0; i <= BCHP_AUD_FMM_SRC_CTRL0_RSRVA_i_ARRAY_END; i++ )
    {
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_RSRVA_i_ARRAY_BASE+(i*4), 0);
    }

    return BERR_SUCCESS;
}

typedef struct BAPE_SrcValues
{
    uint32_t type;
    uint32_t num;
    uint32_t den;
    uint32_t denScale;
} BAPE_SrcValues;

#define TYPE_LINT BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0_TYPE_L_Int
#define TYPE_NONE BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0_TYPE_Bypass
#define DEN_SCALE(d) ((1<<22)/(d))

static const BAPE_SrcValues g_pTable32[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 32k->32k */
    {TYPE_LINT, 320, 441, DEN_SCALE(441)},       /* 32k->44.1k */
    {TYPE_LINT, 2, 3, DEN_SCALE(3)},             /* 32k->48k */
    {TYPE_LINT, 1, 3, DEN_SCALE(3)},             /* 32k->96k */
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 32k->16k */
    {TYPE_LINT, 3200, 2205, DEN_SCALE(2205)},    /* 32k->22.05k */
    {TYPE_LINT, 4, 3, DEN_SCALE(3)},             /* 32k->24k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},             /* 32k->64k */
    {TYPE_LINT, 160, 441, DEN_SCALE(441)},       /* 32k->88_2k */
    {TYPE_LINT, 1, 4, DEN_SCALE(4)},             /* 32k->128k */
    {TYPE_LINT, 80, 441, DEN_SCALE(441)},        /* 32k->176_4k */
    {TYPE_LINT, 1, 6, DEN_SCALE(6)},             /* 32k->192k */
    {TYPE_LINT, 4, 1, DEN_SCALE(1)},             /* 32k->8k */
    {TYPE_LINT, 8, 3, DEN_SCALE(3)},             /* 32k->12k */
    {TYPE_LINT, 32000, 11025, DEN_SCALE(11025)}, /* 32k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 32k->Unknown */
};

static const BAPE_SrcValues g_pTable44_1[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 441, 320, DEN_SCALE(320)},       /* 44.1k->32k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 44.1k->44.1k */
    {TYPE_LINT, 441, 480, DEN_SCALE(480)},       /* 44.1k->48k */
    {TYPE_LINT, 441, 960, DEN_SCALE(960)},       /* 44.1k->96k */
    {TYPE_LINT, 441, 160, DEN_SCALE(160)},       /* 44.1k->16k */
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 44.1k->22.05k */
    {TYPE_LINT, 441, 240, DEN_SCALE(240)},       /* 44.1k->24k */
    {TYPE_LINT, 441, 640, DEN_SCALE(640)},       /* 44.1k->64k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},             /* 44.1k->88_2k */
    {TYPE_LINT, 441, 1280, DEN_SCALE(1280)},     /* 44.1k->128k */
    {TYPE_LINT, 1, 4, DEN_SCALE(4)},             /* 44.1k->176_4k */
    {TYPE_LINT, 441, 1920, DEN_SCALE(1920)},     /* 44.1k->192k */
    {TYPE_LINT, 441, 80, DEN_SCALE(80)},         /* 44.1k->8k */
    {TYPE_LINT, 441, 120, DEN_SCALE(120)},       /* 44.1k->12k */
    {TYPE_LINT, 4, 1, DEN_SCALE(1)},             /* 44.1k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 44.1k->Unknown */
};

static const BAPE_SrcValues g_pTable48[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 3, 2, DEN_SCALE(2)},             /* 48k->32k */
    {TYPE_LINT, 480, 441, DEN_SCALE(441)},       /* 48k->44.1k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 48k->48k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},             /* 48k->96k */
    {TYPE_LINT, 4, 1, DEN_SCALE(1)},             /* 48k->16k */
    {TYPE_LINT, 4800, 2205, DEN_SCALE(2205)},    /* 48k->22.05k */
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 48k->24k */
    {TYPE_LINT, 3, 4, DEN_SCALE(4)},             /* 48k->64k */
    {TYPE_LINT, 240, 441, DEN_SCALE(441)},       /* 48k->88_2k */
    {TYPE_LINT, 3, 8, DEN_SCALE(8)},             /* 48k->128k */
    {TYPE_LINT, 120, 441, DEN_SCALE(441)},       /* 48k->176_4k */
    {TYPE_LINT, 1, 4, DEN_SCALE(4)},             /* 48k->192k */
    {TYPE_LINT, 6, 1, DEN_SCALE(1)},             /* 48k->8k */
    {TYPE_LINT, 4, 1, DEN_SCALE(1)},             /* 48k->12k */
    {TYPE_LINT, 48000, 11025, DEN_SCALE(11025)}, /* 48k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 48k->Unknown */
};

static const BAPE_SrcValues g_pTable96[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 3, 1, DEN_SCALE(1)},             /* 96k->32k */
    {TYPE_LINT, 960, 441, DEN_SCALE(441)},       /* 96k->44.1k */
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 96k->48k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 96k->96k */
    {TYPE_LINT, 6, 1, DEN_SCALE(1)},             /* 96k->16k */
    {TYPE_LINT, 9600, 2205, DEN_SCALE(2205)},    /* 96k->22.05k */
    {TYPE_LINT, 4, 1, DEN_SCALE(1)},             /* 96k->24k */
    {TYPE_LINT, 3, 2, DEN_SCALE(2)},             /* 96k->64k */
    {TYPE_LINT, 480, 441, DEN_SCALE(441)},       /* 96k->88_2k */
    {TYPE_LINT, 3, 4, DEN_SCALE(4)},             /* 96k->128k */
    {TYPE_LINT, 240, 441, DEN_SCALE(441)},       /* 96k->176_4k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},             /* 96k->192k */
    {TYPE_LINT, 12, 1, DEN_SCALE(1)},            /* 96k->8k */
    {TYPE_LINT, 8, 1, DEN_SCALE(1)},             /* 96k->12k */
    {TYPE_LINT, 96000, 11025, DEN_SCALE(11025)}, /* 96k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 96k->Unknown */
};

static const BAPE_SrcValues g_pTable16[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},             /* 16k->32k */
    {TYPE_LINT, 160, 441, DEN_SCALE(441)},       /* 16k->44.1k */
    {TYPE_LINT, 1, 3, DEN_SCALE(3)},             /* 16k->48k */
    {TYPE_LINT, 1, 6, DEN_SCALE(6)},             /* 16k->96k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 16k->16k */
    {TYPE_LINT, 1600, 2205, DEN_SCALE(2205)},    /* 16k->22.05k */
    {TYPE_LINT, 2, 3, DEN_SCALE(3)},             /* 16k->24k */
    {TYPE_LINT, 1, 4, DEN_SCALE(4)},             /* 16k->64k */
    {TYPE_LINT, 80, 441, DEN_SCALE(441)},        /* 16k->88_2k */
    {TYPE_LINT, 1, 8, DEN_SCALE(8)},             /* 16k->128k */
    {TYPE_LINT, 40, 441, DEN_SCALE(441)},        /* 16k->176_4k */
    {TYPE_LINT, 1, 12, DEN_SCALE(12)},           /* 16k->192k */
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 16k->8k */
    {TYPE_LINT, 4, 3, DEN_SCALE(3)},             /* 16k->12k */
    {TYPE_LINT, 16000, 11025, DEN_SCALE(11025)}, /* 16k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 16k->Unknown */
};

static const BAPE_SrcValues g_pTable22_05[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 2205, 3200, DEN_SCALE(3200)},    /* 22.05k->32k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},             /* 22.05k->44.1k */
    {TYPE_LINT, 2205, 4800, DEN_SCALE(4800)},    /* 22.05k->48k */
    {TYPE_LINT, 2205, 9600, DEN_SCALE(9600)},    /* 22.05k->96k */
    {TYPE_LINT, 2205, 1600, DEN_SCALE(1600)},    /* 22.05k->16k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 22.05k->22.05k */
    {TYPE_LINT, 2205, 2400, DEN_SCALE(2400)},    /* 22.05k->24k */
    {TYPE_LINT, 2205, 6400, DEN_SCALE(6400)},    /* 22.05k->64k */
    {TYPE_LINT, 1, 4, DEN_SCALE(4)},             /* 22.05k->88_2k */
    {TYPE_LINT, 2205, 12800, DEN_SCALE(12800)},  /* 22.05k->128k */
    {TYPE_LINT, 1, 8, DEN_SCALE(8)},             /* 22.05k->176_4k */
    {TYPE_LINT, 2205, 19200, DEN_SCALE(19200)},  /* 22.05k->192k */
    {TYPE_LINT, 2205, 800, DEN_SCALE(800)},      /* 22.05k->8k */
    {TYPE_LINT, 2205, 1200, DEN_SCALE(1200)},    /* 22.05k->12k */
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 22.05k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 22.05k->Unknown */
};

static const BAPE_SrcValues g_pTable24[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 3, 4, DEN_SCALE(4)},             /* 24k->32k */
    {TYPE_LINT, 240, 441, DEN_SCALE(441)},       /* 24k->44.1k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},             /* 24k->48k */
    {TYPE_LINT, 1, 4, DEN_SCALE(4)},             /* 24k->96k */
    {TYPE_LINT, 3, 2, DEN_SCALE(2)},             /* 24k->16k */
    {TYPE_LINT, 2400, 2205, DEN_SCALE(2205)},    /* 24k->22.05k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 24k->24k */
    {TYPE_LINT, 3, 8, DEN_SCALE(8)},             /* 24k->64k */
    {TYPE_LINT, 120, 441, DEN_SCALE(441)},       /* 24k->88_2k */
    {TYPE_LINT, 3, 16, DEN_SCALE(16)},           /* 24k->128k */
    {TYPE_LINT, 60, 441, DEN_SCALE(441)},        /* 24k->176_4k */
    {TYPE_LINT, 1, 8, DEN_SCALE(8)},             /* 24k->192k */
    {TYPE_LINT, 3, 1, DEN_SCALE(1)},             /* 24k->8k */
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 24k->12k */
    {TYPE_LINT, 24000, 11025, DEN_SCALE(11025)}, /* 24k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 24k->Unknown */
};

static const BAPE_SrcValues g_pTable64[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 64k->32k */
    {TYPE_LINT, 640, 441, DEN_SCALE(441)},       /* 64k->44.1k */
    {TYPE_LINT, 4, 3, DEN_SCALE(3)},             /* 64k->48k */
    {TYPE_LINT, 2, 3, DEN_SCALE(3)},             /* 64k->96k */
    {TYPE_LINT, 4, 1, DEN_SCALE(1)},             /* 64k->16k */
    {TYPE_LINT, 6400, 2205, DEN_SCALE(2205)},    /* 64k->22.05k */
    {TYPE_LINT, 8, 3, DEN_SCALE(3)},             /* 64k->24k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 64k->64k */
    {TYPE_LINT, 320, 441, DEN_SCALE(441)},       /* 64k->88_2k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},             /* 64k->128k */
    {TYPE_LINT, 160, 441, DEN_SCALE(441)},       /* 64k->176_4k */
    {TYPE_LINT, 1, 3, DEN_SCALE(3)},             /* 64k->192k */
    {TYPE_LINT, 8, 1, DEN_SCALE(1)},             /* 64k->8k */
    {TYPE_LINT, 16, 3, DEN_SCALE(3)},            /* 64k->12k */
    {TYPE_LINT, 64000, 11025, DEN_SCALE(11025)}, /* 64k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 64k->Unknown */
};

static const BAPE_SrcValues g_pTable88_2[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 88.2k->44.1k */
    {TYPE_LINT, 441, 160, DEN_SCALE(160)},       /* 88.2k->32k */
    {TYPE_LINT, 441, 240, DEN_SCALE(240)},       /* 88.2k->48k */
    {TYPE_LINT, 441, 480, DEN_SCALE(480)},       /* 88.2k->96k */
    {TYPE_LINT, 441, 80, DEN_SCALE(80)},         /* 88.2k->16k */
    {TYPE_LINT, 4, 1, DEN_SCALE(1)},             /* 88.2k->22.05k */
    {TYPE_LINT, 441, 120, DEN_SCALE(120)},       /* 88.2k->24k */
    {TYPE_LINT, 441, 320, DEN_SCALE(320)},       /* 88.2k->64k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 88.2k->88.2k */
    {TYPE_LINT, 441, 640, DEN_SCALE(640)},       /* 88.2k->128k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},             /* 88.2k->176_4k */
    {TYPE_LINT, 441, 960, DEN_SCALE(960)},       /* 88.2k->192k */
    {TYPE_LINT, 441, 40, DEN_SCALE(40)},         /* 88.2k->8k */
    {TYPE_LINT, 441, 60, DEN_SCALE(60)},         /* 88.2k->12k */
    {TYPE_LINT, 8, 1, DEN_SCALE(1)},             /* 88.2k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 88.2k->Unknown */
};

static const BAPE_SrcValues g_pTable128[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 4, 1, DEN_SCALE(1)},             /* 128k->32k */
    {TYPE_LINT, 1280, 441, DEN_SCALE(441)},      /* 128k->44.1k */
    {TYPE_LINT, 8, 3, DEN_SCALE(3)},             /* 128k->48k */
    {TYPE_LINT, 4, 3, DEN_SCALE(3)},             /* 128k->96k */
    {TYPE_LINT, 8, 1, DEN_SCALE(1)},             /* 128k->16k */
    {TYPE_LINT, 12800, 2205, DEN_SCALE(2205)},   /* 128k->22.05k */
    {TYPE_LINT, 16, 3, DEN_SCALE(3)},            /* 128k->24k */
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 128k->64k */
    {TYPE_LINT, 640, 441, DEN_SCALE(441)},       /* 128k->88_2k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 128k->128k */
    {TYPE_LINT, 320, 441, DEN_SCALE(441)},       /* 128k->176_4k */
    {TYPE_LINT, 2, 3, DEN_SCALE(3)},             /* 128k->192k */
    {TYPE_LINT, 16, 1, DEN_SCALE(1)},            /* 128k->8k */
    {TYPE_LINT, 32, 3, DEN_SCALE(3)},            /* 128k->12k */
    {TYPE_LINT, 128000, 11025, DEN_SCALE(11025)},/* 128k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 128k->Unknown */
};

static const BAPE_SrcValues g_pTable176_4[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 4, 1, DEN_SCALE(1)},             /* 176.4k->44.1k */
    {TYPE_LINT, 441, 80, DEN_SCALE(80)},         /* 176.4k->32k */
    {TYPE_LINT, 441, 120, DEN_SCALE(120)},       /* 176.4k->48k */
    {TYPE_LINT, 441, 240, DEN_SCALE(240)},       /* 176.4k->96k */
    {TYPE_LINT, 441, 40, DEN_SCALE(40)},         /* 176.4k->16k */
    {TYPE_LINT, 8, 1, DEN_SCALE(1)},             /* 176.4k->22.05k */
    {TYPE_LINT, 441, 60, DEN_SCALE(60)},         /* 176.4k->24k */
    {TYPE_LINT, 441, 160, DEN_SCALE(160)},       /* 176.4k->64k */
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 176.4k->88.2k */
    {TYPE_LINT, 441, 320, DEN_SCALE(320)},       /* 176.4k->128k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 176.4k->176.4k */
    {TYPE_LINT, 441, 480, DEN_SCALE(480)},       /* 176.4k->192k */
    {TYPE_LINT, 441, 20, DEN_SCALE(20)},         /* 176.4k->8k */
    {TYPE_LINT, 441, 30, DEN_SCALE(30)},         /* 176.4k->12k */
    {TYPE_LINT, 16, 1, DEN_SCALE(1)},            /* 176.4k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 176.4k->Unknown */
};

static const BAPE_SrcValues g_pTable192[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 6, 1, DEN_SCALE(1)},             /* 192k->32k */
    {TYPE_LINT, 1920, 441, DEN_SCALE(441)},      /* 192k->44.1k */
    {TYPE_LINT, 4, 1, DEN_SCALE(1)},             /* 192k->48k */
    {TYPE_LINT, 2, 1, DEN_SCALE(1)},             /* 192k->96k */
    {TYPE_LINT, 8, 1, DEN_SCALE(1)},             /* 192k->16k */
    {TYPE_LINT, 19200, 2205, DEN_SCALE(2205)},   /* 192k->22.05k */
    {TYPE_LINT, 8, 1, DEN_SCALE(1)},             /* 192k->24k */
    {TYPE_LINT, 6, 2, DEN_SCALE(2)},             /* 192k->64k */
    {TYPE_LINT, 960, 441, DEN_SCALE(441)},       /* 192k->88_2k */
    {TYPE_LINT, 3, 2, DEN_SCALE(2)},             /* 192k->128k */
    {TYPE_LINT, 480, 441, DEN_SCALE(441)},       /* 192k->176_4k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 192k->192k */
    {TYPE_LINT, 24, 1, DEN_SCALE(1)},            /* 192k->8k */
    {TYPE_LINT, 16, 1, DEN_SCALE(1)},            /* 192k->12k */
    {TYPE_LINT, 192000, 11025, DEN_SCALE(11025)},/* 192k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 192k->Unknown */
};

static const BAPE_SrcValues g_pTable8[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 1, 4, DEN_SCALE(4)},             /* 8k->32k */
    {TYPE_LINT, 80, 441, DEN_SCALE(441)},        /* 8k->44.1k */
    {TYPE_LINT, 1, 6, DEN_SCALE(6)},             /* 8k->48k */
    {TYPE_LINT, 1, 12, DEN_SCALE(12)},           /* 8k->96k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},             /* 8k->16k */
    {TYPE_LINT, 800, 2205, DEN_SCALE(2205)},     /* 8k->22.05k */
    {TYPE_LINT, 2, 6, DEN_SCALE(6)},             /* 8k->24k */
    {TYPE_LINT, 1, 8, DEN_SCALE(8)},             /* 8k->64k */
    {TYPE_LINT, 40, 441, DEN_SCALE(441)},        /* 8k->88_2k */
    {TYPE_LINT, 1, 16, DEN_SCALE(16)},           /* 8k->128k */
    {TYPE_LINT, 20, 441, DEN_SCALE(441)},        /* 8k->176_4k */
    {TYPE_LINT, 1, 24, DEN_SCALE(24)},           /* 8k->192k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 8k->8k */
    {TYPE_LINT, 2, 3, DEN_SCALE(3)},             /* 8k->12k */
    {TYPE_LINT, 8000, 11025, DEN_SCALE(11025)},  /* 8k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 8k->Unknown */
};

static const BAPE_SrcValues g_pTable12[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 3, 8, DEN_SCALE(8)},             /* 12k->32k */
    {TYPE_LINT, 120, 441, DEN_SCALE(441)},       /* 12k->44.1k */
    {TYPE_LINT, 1, 4, DEN_SCALE(4)},             /* 12k->48k */
    {TYPE_LINT, 1, 8, DEN_SCALE(8)},             /* 12k->96k */
    {TYPE_LINT, 3, 4, DEN_SCALE(4)},             /* 12k->16k */
    {TYPE_LINT, 1200, 2205, DEN_SCALE(2205)},    /* 12k->22.05k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},             /* 12k->24k */
    {TYPE_LINT, 3, 16, DEN_SCALE(16)},           /* 12k->64k */
    {TYPE_LINT, 60, 441, DEN_SCALE(441)},        /* 12k->88_2k */
    {TYPE_LINT, 3, 32, DEN_SCALE(32)},           /* 12k->128k */
    {TYPE_LINT, 30, 441, DEN_SCALE(441)},        /* 12k->176_4k */
    {TYPE_LINT, 1, 16, DEN_SCALE(16)},           /* 12k->192k */
    {TYPE_LINT, 3, 2, DEN_SCALE(2)},             /* 12k->8k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 12k->12k */
    {TYPE_LINT, 12000, 11025, DEN_SCALE(11025)}, /* 12k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},             /* 12k->Unknown */
};

static const BAPE_SrcValues g_pTable11_025[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 11025, 32000, DEN_SCALE(32000)},    /* 11.025k->32k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},                /* 11.025k->44.1k */
    {TYPE_LINT, 11025, 48000, DEN_SCALE(48000)},    /* 11.025k->48k */
    {TYPE_LINT, 11025, 96000, DEN_SCALE(96000)},    /* 11.025k->96k */
    {TYPE_LINT, 11025, 16000, DEN_SCALE(16000)},    /* 11.025k->16k */
    {TYPE_LINT, 1, 2, DEN_SCALE(2)},                /* 11.025k->22.05k */
    {TYPE_LINT, 11025, 24000, DEN_SCALE(24000)},    /* 11.025k->24k */
    {TYPE_LINT, 11025, 64000, DEN_SCALE(64000)},    /* 11.025k->64k */
    {TYPE_LINT, 1, 8, DEN_SCALE(8)},                /* 11.025k->88_2k */
    {TYPE_LINT, 11025, 128000, DEN_SCALE(128000)},  /* 11.025k->128k */
    {TYPE_LINT, 1, 16, DEN_SCALE(16)},              /* 11.025k->176_4k */
    {TYPE_LINT, 11025, 192000, DEN_SCALE(192000)},  /* 11.025k->192k */
    {TYPE_LINT, 11025, 8000, DEN_SCALE(8000)},      /* 11.025k->8k */
    {TYPE_LINT, 11025, 12000, DEN_SCALE(12000)},    /* 11.025k->12k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* 11.025k->11.025k */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* 11.025k->Unknown */
};

static const BAPE_SrcValues g_pTableUnknown[BAVC_AudioSamplingRate_eUnknown+1] =
{
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
    {TYPE_LINT, 1, 1, DEN_SCALE(1)},                /* Unknown->Unknown */
};

static const BAPE_SrcValues *g_pSrcTable[BAVC_AudioSamplingRate_eUnknown+1] = 
{
    g_pTable32,
    g_pTable44_1,
    g_pTable48,
    g_pTable96,
    g_pTable16,
    g_pTable22_05,
    g_pTable24,
    g_pTable64,
    g_pTable88_2,
    g_pTable128,
    g_pTable176_4,
    g_pTable192,
    g_pTable8,
    g_pTable12,
    g_pTable11_025,
    g_pTableUnknown
};

void BAPE_P_ProgramSRC_isr(BAPE_Handle handle, uint8_t srcId, BAVC_AudioSamplingRate inputRate, BAVC_AudioSamplingRate outputRate)
{
    uint32_t regVal, regAddr;
    
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(srcId < BAPE_CHIP_MAX_SRCS);

    BDBG_MSG(("SRC Programming - Input Rate %s Output Rate %s", BAPE_P_GetSampleRateString_isrsafe(inputRate), BAPE_P_GetSampleRateString_isrsafe(outputRate)));
    BDBG_MSG(("Programming SRC %u to mode %u num %u den %u scale %u", srcId,
              g_pSrcTable[inputRate][outputRate].type,
              g_pSrcTable[inputRate][outputRate].num,
              g_pSrcTable[inputRate][outputRate].den,
              g_pSrcTable[inputRate][outputRate].denScale));

    regAddr = BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0 + (((uint32_t) srcId)*(BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG1-BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0));
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFG0, TYPE);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFG0, TYPE, g_pSrcTable[inputRate][outputRate].type);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    regAddr += BCHP_AUD_FMM_SRC_CTRL0_LI_NUM0-BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0;
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_LI_NUM0, NUMERATOR);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_NUM0, NUMERATOR, g_pSrcTable[inputRate][outputRate].num);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    regAddr += BCHP_AUD_FMM_SRC_CTRL0_LI_DEN0-BCHP_AUD_FMM_SRC_CTRL0_LI_NUM0;
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_LI_DEN0, DENOMINATOR);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_DEN0, DENOMINATOR, g_pSrcTable[inputRate][outputRate].den);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    regAddr += BCHP_AUD_FMM_SRC_CTRL0_LI_DEN_SCALE0-BCHP_AUD_FMM_SRC_CTRL0_LI_DEN0;
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_SRC_CTRL0_LI_DEN_SCALE0, DEN_SCALE);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_DEN_SCALE0, DEN_SCALE, g_pSrcTable[inputRate][outputRate].denScale);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
}

