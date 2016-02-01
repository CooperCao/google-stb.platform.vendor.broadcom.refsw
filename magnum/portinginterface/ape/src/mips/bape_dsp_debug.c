/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
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
#include "bmem.h"
#include "belf.h"
#include "bape_priv.h"

#if BDBG_DEBUG_BUILD

BDBG_MODULE(bape_dsp_debug);

/* Offsets in 4-byte increments */
#define OFFSET_GP (0)
#define OFFSET_C0 (32)
#define RESERVED_MEM_START (0xc00)

#define NUM_REGS (sizeof(g_pRegNames)/sizeof(const char *))

static const char *g_pRegNames[] =
{
    "zero",  "AT",  "v0",  "v1",  "a0",  "a1",  "a2", "a3",  
    "t0",    "t1",  "t2",  "t3",  "t4",  "t5",  "t6", "t7", 
    "s0",    "s1",  "s2",  "s3",  "s4",  "s5",  "s6", "s7", 
    "t8",    "t9",  "k0",  "k1",  "gp",  "sp",  "s8", "ra",
    "BADVA", "STATUS", "CAUSE", "EPC"
};

static const uint16_t g_pMemOffsets[] =
{
     0*4,  1*4,  2*4,  3*4,  4*4,  5*4,  6*4,  7*4,  8*4,  9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4, 
    16*4, 17*4, 18*4, 19*4, 20*4, 21*4, 22*4, 23*4, 24*4, 25*4, 26*4, 27*4, 28*4, 29*4, 30*4, 31*4,
    40*4, 44*4, 45*4, 46*4
};

static uint32_t g_pRegisters[NUM_REGS];

const char *BAPE_Dsp_P_ImgIdToName(BAPE_Img_Id id)
{
    switch ( id )
    {
    case BAPE_Img_Id_eBkernel: return "bsoftaudio.elf";
    case BAPE_Img_Id_eAacDecode: return "aac_1_decoder.elf";
    case BAPE_Img_Id_eDdpDecode: return "ddp_2_converter.elf";
    case BAPE_Img_Id_eMpegDecode: return "mpeg_1_decoder.elf";
    case BAPE_Img_Id_eWmaDecode: return "wma_1_decoder.elf";
    case BAPE_Img_Id_eAc3Decode: return "ac3_decoder.elf";
    case BAPE_Img_Id_eAc3Passthrough: return "ac3_passthrough.elf";
    case BAPE_Img_Id_ePcmWav: return "pcm_wav.elf";
    case BAPE_Img_Id_eMax: break;
    }
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return "unknown";
}

const char *BAPE_Dsp_P_GetLocationString(BAPE_DspHandle dsp, uint32_t value)
{
    unsigned i;
    static char locationBuf[512];
    void *pAddr;
    BERR_Code errCode;
    
    BKNI_Snprintf(locationBuf, 512, "N/A");

    /* If location is in KSEG1, convert to KSEG0 */
    if ( value >= 0xa0000000 && value < 0xc0000000 )
    {
        value &= ~0x20000000;
    }
    /* If value is not in KSEG0, bail out */
    if ( value < 0x80000000 || value >= 0xc0000000 )
    {
        return locationBuf;
    }
    /* Now, convert to a cached address so we can compare to the sections */
    value &= ~0x80000000;
    errCode = BMEM_ConvertOffsetToAddress(dsp->deviceHandle->memHandle, value, &pAddr);
    if ( errCode )
    {
        return locationBuf;
    }
    errCode = BMEM_ConvertAddressToCached(dsp->deviceHandle->memHandle, pAddr, &pAddr);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return locationBuf;
    }

    for ( i = 0; i < BAPE_Img_Id_eMax; i++ )
    {
        if ( dsp->images[i].elf )
        {
            unsigned j;
            BELF_ImageDetails imageDetails;
            BELF_SectionDetails sectionDetails;
            errCode = BELF_GetImageDetails(dsp->images[i].elf, &imageDetails);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                return locationBuf;
            }
            for ( j = 1; j < imageDetails.numSections; j++ )
            {
                errCode = BELF_LookupSectionByIndex(dsp->images[i].elf, j, &sectionDetails);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                    return locationBuf;
                }
                if ( pAddr >= sectionDetails.pAddress && pAddr < (void *)((uint8_t *)sectionDetails.pAddress + sectionDetails.size) )
                {
                    BELF_SymbolDetails symbolDetails;

                    errCode = BELF_LookupNearestSymbolByAddress(dsp->images[i].elf, pAddr, &symbolDetails);
                    if ( BERR_SUCCESS == errCode )
                    {
                        BKNI_Snprintf(locationBuf, 512, "%s: %s+0x%x [%s+0x%x]", 
                                      BAPE_Dsp_P_ImgIdToName(i), 
                                      sectionDetails.pName, 
                                      (uint8_t *)pAddr - (uint8_t *)sectionDetails.pAddress, 
                                      symbolDetails.pName,
                                      (uint8_t *)pAddr - (uint8_t *)symbolDetails.pAddress);
                    }
                    else
                    {
                        BKNI_Snprintf(locationBuf, 512, "%s: %s+0x%x", 
                                      BAPE_Dsp_P_ImgIdToName(i), 
                                      sectionDetails.pName, 
                                      (uint8_t *)pAddr - (uint8_t *)sectionDetails.pAddress);
                    }
                    return locationBuf;
                }
            }
        }
    }

    return locationBuf;
}

static bool BAPE_Dsp_P_IsFunction(BAPE_DspHandle dsp, uint32_t value)
{
    unsigned i;
    void *pAddr;
    BERR_Code errCode;

    /* If location is in KSEG1, convert to KSEG0 */
    if ( value >= 0xa0000000 && value < 0xc0000000 )
    {
        value &= ~0x20000000;
    }
    /* If value is not in KSEG0, bail out */
    if ( value < 0x80000000 || value >= 0xc0000000 )
    {
        return false;
    }
    /* If value is not aligned, bail out */
    if ( value & 0x3 )
    {
        return false;
    }
    /* Now, convert to a cached address so we can compare to the sections */
    value &= ~0x80000000;
    errCode = BMEM_ConvertOffsetToAddress(dsp->deviceHandle->memHandle, value, &pAddr);
    if ( errCode )
    {
        return false;
    }
    errCode = BMEM_ConvertAddressToCached(dsp->deviceHandle->memHandle, pAddr, &pAddr);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return false;
    }

    for ( i = 0; i < BAPE_Img_Id_eMax; i++ )
    {
        if ( dsp->images[i].pBase )
        {
            if ( ((uint32_t)pAddr >= (uint32_t)dsp->images[i].pTextBase) &&
                 ((uint32_t)pAddr < (uint32_t)dsp->images[i].pTextBase + dsp->images[i].textSize) )
            {
                return true;
            }
        }
    }

    return false;
}

void BAPE_Dsp_P_Backtrace(BAPE_DspHandle dsp)
{
    BAPE_Handle handle;
    unsigned i;
    BERR_Code errCode;
    uint32_t *pStack, *pStackTop;
    void *pAddr;

    BDBG_OBJECT_ASSERT(dsp, BAPE_Dsp);

    handle = dsp->deviceHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    if ( NULL == handle->settings.readReserved )
    {
        BDBG_ERR(("No read reserved handler installed.  Cannot backtrace."));
        return;
    }

    BDBG_MSG(("Retrieving exception log information"));
    for ( i = 0; i < NUM_REGS; i++ )
    {
        g_pRegisters[i] = handle->settings.readReserved(handle->settings.pParam1, handle->settings.param2, RESERVED_MEM_START+g_pMemOffsets[i]);
    }
    BDBG_MSG(("Done"));

    BDBG_ERR(("Registers:"));
    for ( i = 0; i < NUM_REGS; i++ )
    {
        BDBG_ERR(("%6s: 0x%08x (%s)", g_pRegNames[i], g_pRegisters[i], 
                  BAPE_Dsp_P_GetLocationString(dsp, g_pRegisters[i])));
    }

    BDBG_ERR((""));
    BDBG_ERR(("Backtrace:"));
    pStack = (uint32_t *)g_pRegisters[29];
    if ( g_pRegisters[29] & 0x3 )
    {
        BDBG_ERR(("Corrupt Stack (misaligned)"));
        return;
    }
    if ( g_pRegisters[29] >= 0xa0000000 || g_pRegisters[29] < 0x80000000 )
    {
        BDBG_ERR(("Corrupt Stack (invalid address range)"));
        return;
    }
    errCode = BMEM_ConvertOffsetToAddress(handle->memHandle, g_pRegisters[29] & ~0x80000000 /* Convert to offset */, &pAddr);
    if ( errCode )
    {
        BDBG_ERR(("Corrupt Stack (unable to convert to address)"));
        return;
    }
    errCode = BMEM_ConvertAddressToCached(handle->memHandle, pAddr, &pAddr);
    if ( errCode )
    {
        BDBG_ERR(("Corrupt Stack (unable to convert to cached)"));
        return;
    }
    pStack = pAddr;
    errCode = BMEM_ConvertAddressToCached(handle->memHandle,
                                          (uint8_t *)dsp->images[BAPE_Img_Id_eBkernel].pBase + dsp->images[BAPE_Img_Id_eBkernel].size + dsp->settings.heapSize + dsp->settings.stackSize + dsp->settings.sharedMemorySize, 
                                          &pAddr);
    if ( errCode )
    {
        BDBG_ERR(("Unable to find top of stack"));
        return;
    }
    pStackTop = pAddr;
    if ( pStack > pStackTop )
    {
        BDBG_ERR(("Corrupt Stack (above top %#x)", pStackTop));
    }
    if ( pStack < (pStackTop - dsp->settings.stackSize) )
    {
        BDBG_ERR(("Stack overflow detected.  SP %#x, Stack Top %#x size %d (%#x)", pStack, pStackTop, dsp->settings.stackSize, dsp->settings.stackSize));
        return;
    }
    /* Walk back up stack to top */
    BDBG_ERR(("Stack %#x Top %#x", pStack, pStackTop));
    for ( ; pStack <= pStackTop; pStack++ )
    {
        if ( BAPE_Dsp_P_IsFunction(dsp, *pStack) )
        {
            BDBG_ERR(("%#x: %#x (%s)", pStack, *pStack, BAPE_Dsp_P_GetLocationString(dsp, *pStack)));
        }
    }
    BDBG_ERR(("Backtrace Complete"));
}

#endif

