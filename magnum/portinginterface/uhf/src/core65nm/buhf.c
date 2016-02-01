/*************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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
 * Module Description: Porting Interface for the UHF Receiver Module.
 * Refer http://twiki-01.broadcom.com/bin/view/Bseavsw/UHFPI
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 * ***************************************************************************/

#include "buhf.h"
#include "bchp_uhfr_1.h"
#include "bchp_int_id_uhfr_1.h"
#if ((BCHP_CHIP == 7400) || (BCHP_CHIP == 7420))
#include "bchp_uhfr_2.h"
#include "bchp_int_id_uhfr_2.h"
#endif

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if ((BCHP_CHIP == 7400 && BCHP_VER >= BCHP_VER_B0) || (BCHP_CHIP == 7405) || \
     (BCHP_CHIP == 7325) || (BCHP_CHIP == 7335) || (BCHP_CHIP == 7340) || (BCHP_CHIP == 7342) || \
     (BCHP_CHIP == 7420) || (BCHP_CHIP == 7413) || (BCHP_CHIP == 7408))
#define BUHF_65nm 1  /* Define this macro for all 65 nm chips */
#endif

#if ((BCHP_CHIP == 7420)|| (BCHP_CHIP == 7340) || (BCHP_CHIP == 7342) || (BCHP_CHIP == 7408))
  #define BUHF_7420_FAMILY 1
#else
  #define BUHF_7420_FAMILY 0
#endif

BDBG_MODULE (uhf);


#define ABSOLUTE_VALUE(x)    ((x) < 0 ? -(x) : (x))


/* If you want to the Frequency adjustment algo to print values,
 * use 
 *      #define UHFR_DEBUG_FREQ_ADJUSTMENT printf x 
 * else use
 *      #define UHFR_DEBUG_FREQ_ADJUSTMENT  */

#define UHFR_DEBUG_FREQ_ADJUSTMENT(x) BDBG_MSG(x)


/***************************************************************************
Summary:  Maintains status values of different fields required for the Freq 
offset adjustment calculcations
****************************************************************************/
typedef struct BUHF_P_MovingAvg {
    unsigned int    logCount;       /* Current no. of entries in the log */     
    unsigned char   overflowFlag;   /* Whether log has overflown */ 
    int             prDcOffset[BUHF_MAX_LOG_SIZE];    /* Preamble DC offset: 
                                                         UHFR_COR13.dc_level */
    int             prDcOffsetSum;  /* Sum of DC offset values */
    int             prDcOffsetAvg;  /* Running Average of DC offset values */
} BUHF_P_MovingAvg;


/* Maintains the configuration of the UHFR device */
typedef struct BUHF_P_Device {
    BCHP_Handle         hChip;
    BREG_Handle         hRegister;
    BINT_Handle         hInterrupt;
    BKNI_EventHandle    hDataReadyEvent; /* Event to be set by the UHF module when data is ready */
    BINT_CallbackHandle hCallback1; /* BINT callbacks */
    BINT_CallbackHandle hCallback2;
    unsigned int        uiIndex;    /* UHFR device 0 or 1 */
    uint32_t            ui32Offset; /* Register offset of this UHFR device base
                                       address from UHFR 0 base address */
    BUHF_Settings       settings;   /* Current settings of the UHFR device */
   
    /* The following are used ONLY IF freqAdjust is TRUE */
    unsigned int        origFcw;        /* Original FCW in first iteration of 
                                           Freq Adjust algo */
    unsigned int        adjustedFcw;    /* New FCW in final iteration of Freq 
                                           Adjust algo ie after the freq has 
                                           converged.*/
    BUHF_P_MovingAvg    *pMovingAvg;    /* Data structure used to calculate
                                           moving average of the DC level */
    
    /* Used internally */
    volatile bool       intFlag;        /* 1= Data waiting to be processed. */ 
    BUHF_Data           data;           /* value in the COR12 register and the preamble type */

    /* Temporary: only for diagnostic purposes. Remove later */
    unsigned int        totalPkt;       /* Number of packets entering the system */
    unsigned int        errPkt;         /* Number of incoming packets that have BCH error */
    unsigned int        correctedErrPkt;    /* Number of packets with BCH error
                                               that have been corrected */
    uint32_t            ui32IntStatus;  /* Value of BCHP_UHFR_1_ISTS is saved into it */
    bool                bStandbyState;  /* True if in Standby mode, false if in normal mode */
    bool                bFullPowerdown; /* True if the device is completely powered down */

    BUHF_Callback       uhfCb;          /* callback function */
    void                *pData;         /* data passed to callback function */
} BUHF_P_Device;



/* The interrupt handler checks which UHFR interrupt has been received and services
 * it accordingly. If an input key has been received, it calls the fsk input handler.
 */

static void BUHF_P_Isr(void *hUhfr, int iParam2);
static void BUHF_P_Cust2Isr1(void *hUhfr, int iParam2);
static void BUHF_P_Cust2Isr2(void *hUhfr, int iParam2);

BERR_Code BUHF_P_SelectRfChan(BUHF_Handle hUhfr);

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BUHF_Settings defUhfrSettings = 
{
    BUHF_Mode_eAdvanced,     /* mode = Advanced */      
    BUHF_ChanNum_eChan1,     /* channel = BUHF_ChanNum_eChan1; */
    BUHF_InjType_eLow,       /* injection = Low side */
    BUHF_eLPF,               /* filter = BUHF_eLPF */
    false,                   /* BCH error correction is disabled */
    false,                   /* Frequency adjustment is disabled */
    false,                   /* Shift IF is disabled */
    0                        /* No shift */
};

static const BUHF_StandbySettings defStandbySettings = 
{
    true,       /* bEnableWakeup */
    true,       /* bHwKeyFiltering */
    0x17700BB8  /* ui32KeyPatternValue */
};

#if 0
static uint32_t ui32KeyFilterPattern_cust1[BUHF_KeyFilter_eMax] = { 
    0x0880DCC0, /*BUHF_KeyFilter_ePower pattern*/
    0xffffffff
}

static uint32_t ui32KeyFilterPattern_cust2[BUHF_KeyFilter_eMax] = { 
    0x1E240107, /*BUHF_KeyFilter_ePower pattern*/
    0xffffffff
}
#endif


/*#if ((BCHP_CHIP == 7401) || (BCHP_CHIP == 7403))*/
#if 1 /* all chips now have 107 filter taps*/


/* Low pass filter coeffs for 107 taps for Cust2 ie Channel 9 */
static unsigned int lowPassFilterCoeffCust2[BUHF_NUM_FILTER_COEFF] = {
           0x00010000,    
           0xFFFF0000,
           0x00030000,
           0xFFFB0000,
           0x00080000,
           0xFFF30000,
           0x00130000,
           0xFFE50000,
           0x00250000,
           0xFFCF0000,
           0x00420000,
           0xFFAB0000,
           0x006E0000,
           0xFF740000,
           0x00B00000,
           0xFF240000,
           0x01110000,
           0xFEAF0000,
           0x01A00000,
           0xFDFD0000,
           0x02810000,
           0xFCD80000,
           0x04110000,
           0xFA8B0000,
           0x07E20000,
           0xF2940000,
           0x28B04000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000
};

/* Low pass filter coeffs for 107 taps for Cust1 ie Channel 1-8 */
static unsigned int lowPassFilterCoeffCust1[BUHF_NUM_FILTER_COEFF] = {
           0xFFFE0004,
           0x00050000,
           0xFFF9FFFA,
           0x0007000E,
           0x0000FFEC,
           0xFFF20010,
           0x001F0000,
           0xFFD7FFE3,
           0x0021003D,
           0x0000FFB2,
           0xFFCA003C,
           0x006D0000,
           0xFF7AFFA4,
           0x006500B6,
           0x0000FF23,
           0xFF6900A5,
           0x01270000,
           0xFE9DFF0E,
           0x010901D9,
           0x0000FDC0,
           0xFE7401B6,
           0x031B0000,
           0xFC0BFD33,
           0x033F0639,
           0x0000F679,
           0xF8170BEB,
           0x26B53334,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000
};

#else  /* For chips which have 161 taps */

/* Low pass filter coeffs for 161 taps for Cust2 ie Channel 9 */
unsigned int lowPassFilterCoeffCust2[BUHF_NUM_FILTER_COEFF] = {
             0x00000000,
             0x00000000,
             0x00000000,
             0x00000000,
             0x00000000,
             0x00000000,
             0x00000000,
             0x00000001,
             0x0000FFFE,
             0x00000003,
             0x0000FFFC,
             0x00000005,
             0x0000FFF8,
             0x0000000A,
             0x0000FFF2,
             0x00000012,
             0x0000FFE8,
             0x0000001F,
             0x0000FFD9,
             0x00000031,
             0x0000FFC3,
             0x0000004B,
             0x0000FFA4,
             0x00000070,
             0x0000FF79,
             0x000000A2,
             0x0000FF3E,
             0x000000E8,
             0x0000FEEB,
             0x00000149,
             0x0000FE77,
             0x000001D5,
             0x0000FDCB,
             0x000002AE,
             0x0000FCB0,
             0x00000434,
             0x0000FA70,
             0x000007F6,
             0x0000F288,
             0x000028B4,
             0x3FFF0000
};

/* Low pass filter coeffs for 161 taps for Cust1 ie Channel 1-8 */
unsigned int lowPassFilterCoeffCust1[BUHF_NUM_FILTER_COEFF] = {
             0x00000000,
             0x00000000,
             0x00000000,
             0x00000000,
             0x00000001,
             0x0000FFFF,
             0xFFFF0001,
             0x00030000,
             0xFFFCFFFD,
             0x00030006,
             0x0000FFF7,
             0xFFFA0007,
             0x000D0000,
             0xFFEFFFF5,
             0x000D0018,
             0x0000FFE2,
             0xFFEC0017,
             0x00290000,
             0xFFCEFFDE,
             0x00250042,
             0x0000FFB0,
             0xFFCB003B,
             0x00670000,
             0xFF86FFAE,
             0x0059009C,
             0x0000FF49,
             0xFF860084,
             0x00E80000,
             0xFEF1FF4C,
             0x00C30156,
             0x0000FE6F,
             0xFEF30124,
             0x02030000,
             0xFD99FE5F,
             0x01CC033A,
             0x0000FBF1,
             0xFD25034D,
             0x064A0000,
             0xF66DF812,
             0x0BF026B8,
             0x33320000
};
#endif

/* Band pass filter coeffs  */
static unsigned int bandPassFilterCoeff[BUHF_NUM_FILTER_COEFF] = {
                                     0xfff9015e, /* 0 */   
                                     0x006f0055, /* 1 */
                                     0x002d0000, /* 2 */
                                     0xffe1ffd9, /* 3 */
                                     0xffeb000b, /* 4 */
                                     0x00260029, /* 5 */
                                     0x000cffd2, /* 6 */
                                     0xff8eff58, /* 7 */
                                     0xff47ff62, /* 8 */
                                     0xffa2ffef, /* 9 */
                                     0x002b0041, /* 10 */
                                     0x002cfff9, /* 11 */
                                     0xffc8ffb7, /* 12 */
                                     0xffdb0031, /* 13 */
                                     0x00a10102, /* 14 */
                                     0x012f0114, /* 15 */
                                     0x00b7003c, /* 16 */
                                     0xffd0ff9a, /* 17 */
                                     0xffadfff6, /* 18 */
                                     0x004b0077, /* 19 */
                                     0x0053ffd9, /* 20 */
                                     0xff27fe7b, /* 21 */
                                     0xfe15fe20, /* 22 */
                                     0xfe9dff61, /* 23 */
                                     0x00220099, /* 24 */
                                     0x009d0037, /* 25 */
                                     0xffa5ff3f, /* 26 */
                                     0xff530001, /* 27 */
                                     0x01290270, /* 28 */
                                     0x035f0398, /* 29 */
                                     0x02f501a8, /* 30 */
                                     0x0028ff04, /* 31 */
                                     0xfeadff3e, /* 32 */
                                     0x0065017c, /* 33 */
                                     0x01bd0097, /* 34 */
                                     0xfdf4fa54, /* 35 */
                                     0xf6bef475, /* 36 */
                                     0xf48df789, /* 37 */
                                     0xfd1b0428, /* 38 */
                                     0x0b0b100c, /* 39 */
                                     0x11e0100c  /* 40 */
};

#ifndef BUHF_65nm
/* ANA3 register values for the channels */
static unsigned int channelTable[2][BUHF_NUM_CHANNELS] = { 
                             {0x51C60335 , /* 358.8 Mhz VCO 1*/  /* Values for low side injection */
                              0x51Cf2c35 , /* 360.4 Mhz VCO 2*/
                              0x51C09036 , /* 364.6 Mhz VCO 3*/
                              0x51C98d36 , /* 366.2 Mhz VCO 4*/
                              0x50C22b38 , /* 377.6 Mhz VCO 5*/
                              0x51CF2C38 , /* 380.8 Mhz VCO 6*/
                              0x50C61F39 , /* 383.6 Mhz VCO 7*/
                              0x51C26539 , /* 385.2 Mhz VCO 8*/
#if (BCHP_CHIP == 7401 && BCHP_VER==BCHP_VER_A0)    
                              0x5197191F   /* 423.22 Mhz : for Cust2*/
#else
                              0x50c9c73f 
#endif                              
                              }, 
                             {0x51Cbeb38 , /* 380.2 Mhz VCO 1*/  /* Values for high side injection */
                              0x50Cfb439 , /* 381.8 Mhz VCO 2*/
                              0x51C6a739 , /* 386.0 Mhz VCO 3*/
                              0x51Cf2c39 , /* 387.6 Mhz VCO 4*/
                              0x51C3db3b , /* 399.0 Mhz VCO 5*/
                              0x50Ce293c , /* 402.2 Mhz VCO 6*/
                              0x50C0003c , /* 405.0 Mhz VCO 7*/
                              0x51C8173c , /* 406.6 Mhz VCO 8*/
#if (BCHP_CHIP == 7401 && BCHP_VER==BCHP_VER_A0)    
                              0x50840c21   /* 444.62 Mhz : For Cust2 */
#else                              
                              0x50c40c42 
#endif                              
                             }
                           };
#endif

#ifdef BUHF_65nm
/* Tables with values of various registers depending on Frequency
 ANA 1 values for different frequencies = 0x40402204 for all freq */

#if(BCHP_CHIP == 7325 )
   #define BUHF_ANA1_VALUE 0x20401204 /* 7325 use a  27MHz crystal */
   #define BUHF_ANA1_VALUE_CH9 0x60501104 /* 7325 use a  27MHz crystal */   /*value for Cust2 pll setting*/
#else
   #define BUHF_ANA1_VALUE 0x20402204  /* Other chips use a 54MHz crystal */
   #define BUHF_ANA1_VALUE_CH9 0x60502104  /* Other chips use a 54MHz crystal */  /*value for Cust2 pll setting*/
#endif
   #define BUHF_ANA2_VALUE 0x15202C28
   #define BUHF_ANA2_VALUE_CH9 0x15382c28/*value for Cust2 pll setting*/

#if 1

/* Carrier Frequency for channels 1 to 8 ie for Cust 1 in KHz*/
static unsigned int channelFreq[BUHF_NUM_CHANNELS-1] = {
    369500,
    371100,
    375300,
    376900,
    388300,
    391500,
    394300,
    395900
};

/* NOTE: This table is valid only if there are 2 UHF devices */
static unsigned int ndivInt[2/*device number*/][2/*injection_type*/][BUHF_NUM_CHANNELS-1] = {
{{0x01A,0x01A,0x01B,0x01B,0x01B,0x01C,0x01C,0x01C}, /* ndivInt values for channel 1-8, device 0, low side injection */
{0x01C,0x01C,0x01C,0x01C,0x01D,0x01D,0x01D,0x01E}}, /* ndivInt values for channel 1-8, device 0, high side injection */
{{0x01A,0x01A,0x01B/*(01A if shift > 200kHz)*/,0x01B,0x01B,0x01C,0x01C,0x01C},/* ndivInt values for channel 1-8, device 1, low side injection */
{0x01C,0x01C,0x01C,0x01C,0x01D,0x01D,0x01E,0x01E}}  /* ndivInt values for channel 1-8, device 1, high side injection */ 
};

#endif


/* ANA 3 values for different frequencies */ /*change for the cust2 pll setting*/
static unsigned int ana3Table[2][BUHF_NUM_CHANNELS] = { 
                             /* Values for low side injection */
                             {0x0293E93F , /* 358.8 Mhz VCO 1*/  
                              0x02B24079 , /* 360.4 Mhz VCO 2*/
                              0x0201E574 , /* 364.6 Mhz VCO 3*/
                              0x02203CAE , /* 366.2 Mhz VCO 4*/
                              0x02F86A31 , /* 377.6 Mhz VCO 5*/
                              0x023518A7 , /* 380.8 Mhz VCO 6*/
                              0x026A314E , /* 383.6 Mhz VCO 7*/
                              0x02888889 , /* 385.2 Mhz VCO 8*/
                              0x02B302A8   /* 423.22 Mhz : for Cust2*/                            
                              }, 
                             {0x0229B7F1 , /* 380.2 Mhz VCO 1*/  /* Values for high side injection */
                              0x02480F2C , /* 381.8 Mhz VCO 2*/
                              0x0297B426 , /* 386.0 Mhz VCO 3*/
                              0x02B60B61 , /* 387.6 Mhz VCO 4*/
                              0x028E38E4 , /* 399.0 Mhz VCO 5*/
                              0x02CAE759 , /* 402.2 Mhz VCO 6*/
                              0x02000000 , /* 405.0 Mhz VCO 7*/
                              0x021E573B , /* 406.6 Mhz VCO 8*/  
                              0x02DEA00C   /* 444.62 Mhz : For Cust2 */
                             }
                           };

/* ANA 4 values for different frequencies */
static unsigned int ana4Table[2][BUHF_NUM_CHANNELS] = { 
                             /* Values for low side injection */
                             {0x87b6c1A7 , /* 358.8 Mhz VCO 1*/  
                              0x87b6c1A7 , /* 360.4 Mhz VCO 2*/
                              0x87b6c1B7 , /* 364.6 Mhz VCO 3*/
                              0x87b6c1B7 , /* 366.2 Mhz VCO 4*/
                              0x87b6c1B7 , /* 377.6 Mhz VCO 5*/
                              0x87b6c1C7 , /* 380.8 Mhz VCO 6*/
                              0x87b6c1C7 , /* 383.6 Mhz VCO 7*/
                              0x87b6c1C7 , /* 385.2 Mhz VCO 8*/
                              0x87b6c3E7   /* 423.22 Mhz : for Cust2*/                            
                              }, 
                             {0x87b6c1C7 , /* 380.2 Mhz VCO 1*/  /* Values for high side injection */
                              0x87b6c1C7 , /* 381.8 Mhz VCO 2*/
                              0x87b6c1C7 , /* 386.0 Mhz VCO 3*/
                              0x87b6c1C7 , /* 387.6 Mhz VCO 4*/
                              0x87b6c1D7 , /* 399.0 Mhz VCO 5*/
                              0x87b6c1D7 , /* 402.2 Mhz VCO 6*/
                              0x87b6c1E7 , /* 405.0 Mhz VCO 7*/
                              0x87b6c1E7 , /* 406.6 Mhz VCO 8*/  
                              0x87b6c417   /* 444.62 Mhz : For Cust2 */
                             }
                           };

#if 0 
/* ANA 3 values for different frequencies */
static unsigned int ana3Table[2][BUHF_NUM_CHANNELS] = { 
                             /* Values for low side injection */
                             {0x0393E93F , /* 358.8 Mhz VCO 1*/  
                              0x03B24079 , /* 360.4 Mhz VCO 2*/
                              0x0301E574 , /* 364.6 Mhz VCO 3*/
                              0x03203CAE , /* 366.2 Mhz VCO 4*/
                              0x03F86A31 , /* 377.6 Mhz VCO 5*/
                              0x033518A7 , /* 380.8 Mhz VCO 6*/
                              0x036A314E , /* 383.6 Mhz VCO 7*/
                              0x03888889 , /* 385.2 Mhz VCO 8*/
                              0x03598154   /* 423.22 Mhz : for Cust2*/                            
                              }, 
                             {0x0329B7F1 , /* 380.2 Mhz VCO 1*/  /* Values for high side injection */
                              0x03480F2C , /* 381.8 Mhz VCO 2*/
                              0x0397B426 , /* 386.0 Mhz VCO 3*/
                              0x03B60B61 , /* 387.6 Mhz VCO 4*/
                              0x038E38E4 , /* 399.0 Mhz VCO 5*/
                              0x03CAE759 , /* 402.2 Mhz VCO 6*/
                              0x03000000 , /* 405.0 Mhz VCO 7*/
                              0x031E573B , /* 406.6 Mhz VCO 8*/  
                              0x03EF5006   /* 444.62 Mhz : For Cust2 */
                             }
                           };

/* ANA 4 values for different frequencies */
static unsigned int ana4Table[2][BUHF_NUM_CHANNELS] = { 
                             /* Values for low side injection */
                             {0x87b6c1A7 , /* 358.8 Mhz VCO 1*/  
                              0x87b6c1A7 , /* 360.4 Mhz VCO 2*/
                              0x87b6c1B7 , /* 364.6 Mhz VCO 3*/
                              0x87b6c1B7 , /* 366.2 Mhz VCO 4*/
                              0x87b6c1B7 , /* 377.6 Mhz VCO 5*/
                              0x87b6c1C7 , /* 380.8 Mhz VCO 6*/
                              0x87b6c1C7 , /* 383.6 Mhz VCO 7*/
                              0x87b6c1C7 , /* 385.2 Mhz VCO 8*/
                              0x87b6c1F7   /* 423.22 Mhz : for Cust2*/                            
                              }, 
                             {0x87b6c1C7 , /* 380.2 Mhz VCO 1*/  /* Values for high side injection */
                              0x87b6c1C7 , /* 381.8 Mhz VCO 2*/
                              0x87b6c1C7 , /* 386.0 Mhz VCO 3*/
                              0x87b6c1C7 , /* 387.6 Mhz VCO 4*/
                              0x87b6c1D7 , /* 399.0 Mhz VCO 5*/
                              0x87b6c1D7 , /* 402.2 Mhz VCO 6*/
                              0x87b6c1E7 , /* 405.0 Mhz VCO 7*/
                              0x87b6c1E7 , /* 406.6 Mhz VCO 8*/  
                              0x87b6c207   /* 444.62 Mhz : For Cust2 */
                             }
                           };
#endif
#endif /* BUHF_65nm */

/* BCH Error Correction related macros and arrays */
#define GENERATOR_CODE (0x00000769)
#define CRC_MASK (0x000003ff)
#define CRC_SIZE (10)
#define KEY_DATA_MASK (0x03FFFC00)
#define KEY_DATA_SIZE (16)

static const unsigned long syndrome_table[] = {
0x00000000, 0x00000001, 0x00000002, 0x00000003, 0x00000004, 
0x00000005, 0x00000006, 0x00000000, 0x00000008, 0x00000009, 
0x0000000a, 0x00000000, 0x0000000c, 0x00000000, 0x00000000, 
0x00000000, 0x00000010, 0x00000011, 0x00000012, 0x00480000, 
0x00000014, 0x00000000, 0x00000000, 0x00000000, 0x00000018, 
0x01000800, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00001400, 0x00000020, 0x00000021, 0x00000022, 
0x00000000, 0x00000024, 0x00000000, 0x00900000, 0x01002000, 
0x00000028, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x02000400, 0x00000000, 0x00000000, 0x00000030, 0x00000000, 
0x02001000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00002800, 0x00000000, 0x00000040, 
0x00000041, 0x00000042, 0x00000000, 0x00000044, 0x00000000, 
0x00000000, 0x00000000, 0x00000048, 0x00000000, 0x00000000, 
0x00000000, 0x01200000, 0x00000000, 0x02004000, 0x00000000, 
0x00000050, 0x00800080, 0x00000000, 0x00040200, 0x00000000, 
0x00200800, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000060, 0x00000000, 0x00000000, 0x00004400, 
0x00000000, 0x00000000, 0x00000000, 0x00030000, 0x00000000, 
0x00000000, 0x00000000, 0x00202000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00100080, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00005000, 
0x00008100, 0x00000000, 0x00000000, 0x00000080, 0x00000081, 
0x00000082, 0x00000000, 0x00000084, 0x00002100, 0x00000000, 
0x00000000, 0x00000088, 0x00000000, 0x00000000, 0x02080000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000090, 
0x00800040, 0x00000000, 0x00208000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x02400000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00020200, 0x00000000, 0x00000000, 
0x000000a0, 0x00000000, 0x01000100, 0x00000000, 0x00000000, 
0x00000000, 0x00080400, 0x00000000, 0x00000000, 0x00050000, 
0x00401000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00400400, 0x00000000, 0x00100040, 0x00000000, 
0x00081000, 0x00000000, 0x00000900, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x000000c0, 0x00800010, 0x00000000, 
0x00000000, 0x00000000, 0x00084000, 0x00008800, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00060000, 0x00000000, 0x00800001, 0x00800000, 
0x00000000, 0x00800002, 0x00000000, 0x00800004, 0x00404000, 
0x00100020, 0x00000000, 0x00800008, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x01008000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00100010, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00200100, 0x00000000, 
0x00000000, 0x00800020, 0x00000000, 0x00100004, 0x00000000, 
0x00100002, 0x00100001, 0x00100000, 0x0000a000, 0x00000000, 
0x00010200, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00100008, 0x00000100, 0x00000101, 0x00000102, 0x00000000, 
0x00000104, 0x00002080, 0x00000000, 0x00000000, 0x00000108, 
0x00000000, 0x00004200, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000110, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x02040000, 
0x00000000, 0x00300000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000120, 0x00000000, 
0x01000080, 0x00000000, 0x00000000, 0x00041000, 0x00410000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00090000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00040400, 
0x00000880, 0x00000000, 0x00008040, 0x00000000, 0x00a00000, 
0x00000140, 0x00420000, 0x00000000, 0x00000000, 0x02000200, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00100800, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x000a0000, 0x00000000, 
0x00802000, 0x01100000, 0x00000000, 0x00000000, 0x00000000, 
0x00044000, 0x00000000, 0x00000000, 0x00000000, 0x00008020, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000600, 0x00800800, 0x00000000, 0x00000000, 
0x00008010, 0x00200080, 0x00000000, 0x00000000, 0x00000000, 
0x00102000, 0x01800000, 0x00000000, 0x00008008, 0x00001200, 
0x00000000, 0x00000000, 0x00008004, 0x00000000, 0x00000000, 
0x00008001, 0x00008000, 0x00000000, 0x00008002, 0x00000180, 
0x00002004, 0x01000020, 0x00000000, 0x00002001, 0x00002000, 
0x00000000, 0x00002002, 0x00000000, 0x00000000, 0x00108000, 
0x00000000, 0x00011000, 0x00002008, 0x00000000, 0x00440000, 
0x00000000, 0x00000000, 0x00000000, 0x00010400, 0x00000000, 
0x00002010, 0x00000000, 0x00024000, 0x00000000, 0x00000000, 
0x00000000, 0x00000820, 0x000c0000, 0x00000000, 0x00000000, 
0x00000000, 0x01000002, 0x00000000, 0x01000000, 0x01000001, 
0x00000000, 0x00002020, 0x01000004, 0x00000000, 0x00000000, 
0x00000000, 0x01000008, 0x00000810, 0x00808000, 0x00000000, 
0x00200040, 0x00000000, 0x00000000, 0x00000000, 0x01000010, 
0x00000808, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000802, 0x00000801, 0x00000800, 0x00000000, 
0x00000000, 0x02010000, 0x00000804, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00002040, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00200020, 0x00080200, 0x00000000, 
0x00800100, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x02020000, 0x00000000, 
0x00000000, 0x00400200, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x01000040, 0x00000000, 0x00000000, 
0x00000000, 0x00200008, 0x00000000, 0x00000000, 0x00000000, 
0x00200004, 0x00021000, 0x00200002, 0x00000000, 0x00200000, 
0x00200001, 0x00014000, 0x00000000, 0x00000000, 0x00000000, 
0x00020400, 0x00000000, 0x00000000, 0x00100100, 0x00000000, 
0x00000000, 0x00000000, 0x00000840, 0x00000000, 0x00008080, 
0x00200010, 0x00000000, 0x00000200, 0x00000201, 0x00000202, 
0x00000000, 0x00000204, 0x00000000, 0x00000000, 0x00000000, 
0x00000208, 0x00000000, 0x00004100, 0x00009000, 0x00000000, 
0x00110000, 0x00000000, 0x00000000, 0x00000210, 0x00000000, 
0x00000000, 0x00040040, 0x00008400, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00020080, 0x00000000, 0x00000000, 0x00000220, 
0x00280000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00810000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00600000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x02008000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000240, 0x00000000, 0x00000000, 0x00040010, 
0x02000100, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00082000, 0x00000000, 0x00820000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00040002, 0x00040001, 
0x00040000, 0x00000000, 0x00000000, 0x00000000, 0x00040004, 
0x00000000, 0x00402000, 0x00000000, 0x00040008, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00400800, 0x00000000, 0x00000500, 0x00120000, 0x00000000, 
0x00000000, 0x01080000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00040020, 0x00080800, 0x00000000, 
0x00001100, 0x0000c000, 0x00000000, 0x00000000, 0x00010080, 
0x00000000, 0x00000000, 0x00000000, 0x01400000, 0x00000000, 
0x00000280, 0x00000000, 0x00840000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00200400, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00020010, 0x00000000, 
0x00006000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00020008, 0x00000000, 0x00000000, 0x00201000, 
0x00020004, 0x00000000, 0x00000000, 0x00020001, 0x00020000, 
0x00000000, 0x00020002, 0x00000000, 0x00408000, 0x00000000, 
0x00000000, 0x00140000, 0x00000000, 0x00000000, 0x00000000, 
0x01004000, 0x00000000, 0x02200000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00004800, 
0x00088000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00010040, 0x00000000, 
0x00000000, 0x00020020, 0x00000000, 0x00000000, 0x00000000, 
0x02002000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x01000400, 0x00000000, 0x00001800, 0x00000000, 0x00080100, 
0x00000000, 0x00800200, 0x00000c00, 0x00040080, 0x01001000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00010020, 0x00000000, 0x00400100, 0x00020040, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00204000, 0x00000000, 0x03000000, 0x00000000, 0x00000000, 
0x00000000, 0x00010010, 0x00000000, 0x00002400, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00010008, 
0x00003000, 0x00000000, 0x00000000, 0x00000000, 0x00100200, 
0x00010002, 0x00000000, 0x00010000, 0x00010001, 0x00000000, 
0x00000000, 0x00010004, 0x02000800, 0x00000300, 0x00000000, 
0x00004008, 0x00000000, 0x02000040, 0x00000000, 0x00000000, 
0x00000000, 0x00004002, 0x00000000, 0x00004000, 0x00004001, 
0x00000000, 0x00c00000, 0x00004004, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00210000, 0x00000000, 
0x00000000, 0x00000000, 0x00022000, 0x00000000, 0x00004010, 
0x00000000, 0x00000000, 0x00000000, 0x00880000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00020800, 0x00000000, 0x00000000, 0x00000440, 
0x00004020, 0x00500000, 0x00000000, 0x00000000, 0x00048000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00001040, 0x00000000, 0x00180000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x01020000, 0x02000004, 0x00010800, 0x00000000, 
0x00000000, 0x02000000, 0x02000001, 0x02000002, 0x00000000, 
0x00000000, 0x00000420, 0x00004040, 0x00000000, 0x02000008, 
0x00000000, 0x00000000, 0x00080080, 0x00000000, 0x00000000, 
0x00000000, 0x00040100, 0x02000010, 0x00000000, 0x00001020, 
0x00000000, 0x01010000, 0x00000000, 0x00000000, 0x00000000, 
0x00400080, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000408, 0x00000000, 0x00000000, 0x02000020, 0x00000000, 
0x00001010, 0x00000000, 0x00000401, 0x00000400, 0x00000000, 
0x00000402, 0x00000000, 0x00000404, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00001004, 0x00220000, 0x00001002, 
0x00000000, 0x00001000, 0x00001001, 0x00000000, 0x00000410, 
0x00000000, 0x00000000, 0x00000000, 0x00008200, 0x00001008, 
0x00012000, 0x00000000, 0x00101000, 0x00000000, 0x00000000, 
0x00000000, 0x00002200, 0x00000000, 0x00018000, 0x00000000, 
0x00000000, 0x00004080, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00080040, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x02800000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00400040, 
0x00020100, 0x00100400, 0x00000000, 0x00000000, 0x00000000, 
0x01000200, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00801000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x02100000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00800400, 0x00000000, 0x00000000, 
0x00000a00, 0x00000000, 0x00240000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x02000080, 
0x00000000, 0x00000000, 0x00080008, 0x00000000, 0x00000000, 
0x00000000, 0x00080004, 0x00400010, 0x00080002, 0x00080001, 
0x00080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00400008, 0x00000000, 0x00042000, 0x00000000, 0x00400004, 
0x00000000, 0x00000000, 0x00804000, 0x00400000, 0x00400001, 
0x00400002, 0x00080010, 0x00028000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00040800, 0x00000480, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00200200, 0x00080020, 0x00000000, 0x01040000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00001080, 
0x00000000, 0x00000000, 0x00000000, 0x00010100, 0x00000000, 
0x00400020, 0x00104000};

#define SYNDROME_TABLE_SIZE (sizeof(syndrome_table) / 4)

static BERR_Code BUHF_P_BchErrorCorrection (BUHF_Handle hUhfr, unsigned int * packet);

/* BCH Error Correction related additions - end */


/* Static functions ie local functions not visible to user */
static BERR_Code BUHF_P_InitReg (BUHF_Handle hUhfr);
static BERR_Code BUHF_P_SetFilterCoeff (BUHF_Handle hUhfr, BUHF_Filter filter);

#ifndef BUHF_65nm
static BERR_Code BUHF_P_CalculateMovingAvg (BUHF_Handle hUhfr, int value);
static BERR_Code BUHF_P_SetFreqOffset (BUHF_Handle hUhfr, unsigned int bchError);
#endif

/******************************************************************************
 * Summary: 
 *      Sets the filter coefficients.        
 *
 * Input Parmameters:
 *      BUHF_Handle hUhfr : Pointer to UHFR device 
 *      BUHF_Filter filter   : Which filter to select.  
 *                             Values = BUHF_eLPF or BUHF_eBPF
 *
 * Returns:
 *      BERR_SUCCESS on Success
 *      Error value on failure.
 *
 * Implementation:
 *      Filter coefficients for Low Pass and High Pass filter
 *      are taken from the array filterCoeff[][].
 *
 *****************************************************************************/
static BERR_Code BUHF_P_SetFilterCoeff 
(
    BUHF_Handle hUhfr,
    BUHF_Filter filter
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t val;
    unsigned int i;

    /* Check parameters passed to this function */
    BDBG_ASSERT(hUhfr);

    hUhfr->settings.filter = filter;
    
    /* Enable auto increment */
    val = BCHP_UHFR_1_ADDR_LPF_COEF_ADDR_INC_EN_MASK; 
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ADDR + hUhfr->ui32Offset), val);         

    if (filter == BUHF_eLPF) 
    {
        if (hUhfr->settings.channel == BUHF_ChanNum_eChan9)
        { /* Load LPF coeffs for Cust 2 */
            for (i=0; i<BUHF_NUM_FILTER_COEFF; i++)
            {
                val = lowPassFilterCoeffCust2[i]; 
                BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COEF + hUhfr->ui32Offset), val);            
            }
        }        
        else 
        {   /* Load LPF coeffs for Channels 1-8 ie Cust 1 */
            for (i=0; i<BUHF_NUM_FILTER_COEFF; i++)
            {
                val = lowPassFilterCoeffCust1[i]; 
                BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COEF + hUhfr->ui32Offset), val);            
            }
        }
    }
    else if (filter == BUHF_eBPF)
    { /* For Band Pass Filter */
        for (i=0; i<BUHF_NUM_FILTER_COEFF; i++)
        {
            val = bandPassFilterCoeff[i]; 
            BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COEF + hUhfr->ui32Offset), val);         
        }
    }                        
    return ret;
}


/******************************************************************************
 * static BERR_Code BUHF_P_InitReg (BUHF_Handle hUhfr, BUHF_Mode mode)
 *
 * Function:
 *      Initializes all UHFR registers according to mode of 
 *      operation (Legacy or Advanced)
 *
 * Input Parmameters:
 *      SBcmDevice *pDevice: Pointer to UHFR device
 *
 * Returns:
 *      BERR_SUCCESS on Success
 *      Error on failure
 *
 * Implementation:
 *      ANA3 is not set here. To set it, BUHF_P_SelectRfChan() has to be called 
 *      explictly by the user. 
 *      In Legacy mode, disable all UHFR intrpts. 
 *      In Advanced mode, enable only BCHP_UHFR_1_ISTS_CORR_BCH_ERR_MASK,
 *      CORR_DECODE_PR1_END_MASK and CORR_DECODE_PR2_END_MASK 
 *
 *****************************************************************************/
static BERR_Code BUHF_P_InitReg (
        BUHF_Handle hUhfr
)
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int val;
    unsigned int offset = 0;
    offset = hUhfr->ui32Offset;

    /* Check parameters passed to this function */
    BDBG_ASSERT(hUhfr);     

    BDBG_MSG(("BUHF_P_InitReg"));

    /* always program BCHP_UHFR_GLBL_AUTOPD1 for UHF1 or UHF2 */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD1), 0x00000000);         
    
    /* reset all blocks */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_RST + offset), 0xffffffff);         
    
#ifndef BUHF_65nm 
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANADIR), 0x000c0101); 
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANADIR), 0x000c0100); 
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANADIR + offset), 0x000c0101);        
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANADIR + offset), 0x000c0100); 
#else
    val = 0x004b0101;
    val &= ~(BCHP_MASK (UHFR_1_ANADIR, RF_BAND)); /* clear RF_BAND field */  

    switch (hUhfr->settings.channel)
    {
        case BUHF_ChanNum_eChan1:
        case BUHF_ChanNum_eChan2:
        case BUHF_ChanNum_eChan3:
        case BUHF_ChanNum_eChan4:      
        case BUHF_ChanNum_eChan5:      
        case BUHF_ChanNum_eChan6:      
        case BUHF_ChanNum_eChan7:      
        case BUHF_ChanNum_eChan8:                  
            val |= BCHP_FIELD_DATA (UHFR_1_ANADIR, RF_BAND, 1);
            break;            
        case BUHF_ChanNum_eChan9:                  
            val |= BCHP_FIELD_DATA (UHFR_1_ANADIR, RF_BAND, 3);
            break;    
        default:
            BDBG_ERR(("Invalid channel %d ", hUhfr->settings.channel));
            return BERR_INVALID_PARAMETER;
    }

    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANADIR), val); /* for dev 1 */ 
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANADIR + offset), val); /* will take care of dev 2 */ 
    BDBG_MSG(("BCHP_UHFR_1_ANADIR=0x%08x", val));
    
    val &= ~(BCHP_MASK (UHFR_1_ANADIR, RX_RESET_DIR)); /* clear RX_RESET_DIR bit */    
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANADIR), val); /* for dev 1 */ 
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANADIR + offset), val); /* will take care of dev 2 */ 
    BDBG_MSG(("BCHP_UHFR_1_ANADIR=0x%08x", val));
#endif

#ifdef BUHF_65nm
    /*  When you select uhfr_2, you also need to write to uhfr_1.ANA0, 
    uhfr_1.ANA4, and uhfr_1.ANA5. */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA0 ), 0x4a492492);         
#endif
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA0 + offset), 0x4a492492);         

#ifndef BUHF_65nm
    /* ADC Power down for both modes */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA1 + offset), 0x4d000000);         
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA2 + offset), 0x0000879e);         
    /* ANA1 has to be programmed for both UHF 1 and UHF 2 to be functional */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA1), 0x4d000000);  
#else
    if (hUhfr->settings.channel == BUHF_ChanNum_eChan9) /*change for the cust2 pll setting*/
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA2 + offset), BUHF_ANA2_VALUE_CH9);
    }
    else
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA2 + offset), BUHF_ANA2_VALUE);
    }
    /*  When you select uhfr_2, you also need to write to uhfr_1.ANA0, 
    uhfr_1.ANA4, and uhfr_1.ANA5.*/    
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA5), 1);  
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA5 + offset), 1);       
#endif
    
    /* ANACTL has to be programmed after ANA1 for ANA1 to take effect */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANACTL), 0x1);      

#if ((BCHP_CHIP == 7401) || (BCHP_CHIP == 7403))
    /* 7401 and 7403 are decimated by 108 */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_FILT + offset), 0x006a0104);   
    val = (hUhfr->settings.channel == BUHF_ChanNum_eChan9)? 0x00cd462a: 0x0032b9d6;
#elif ((BUHF_7420_FAMILY == 1))
    /* 7420 are decimated by 108 */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_FILT + offset), 0x006a0104);   
    val = (hUhfr->settings.channel == BUHF_ChanNum_eChan9)? 0x00cd462a: 0x0032b9d6; /* ~(0x0032b9d6)+1 = FFCD462A  , But programming 0x00CD462A*/
#else    
    /* All other chips are decimated by 162 */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_FILT + offset), 0x006a0108);   
    val = (hUhfr->settings.channel == BUHF_ChanNum_eChan9)? ~(0x0021d13a)+1: 0x0021d13a;
#endif

#if ((BUHF_7420_FAMILY == 1))
    if ((hUhfr->settings.bShiftIf==true) && (hUhfr->settings.channel != BUHF_ChanNum_eChan9))
    {  /* If SHIFT IF is selected, use different FCW values for UHF1 and UHF 2 */
        if (hUhfr->uiIndex == 0)  
        {
            val = 0x00322833; /*in 81 MHZ domain its value is 0x00217022 And 81*(0x00217022/2^24)=10.58MHz . 
                                        So in 54Mhz domain (10.58/54)*2^24=0x00322833*/
        }
        else if (hUhfr->uiIndex == 1)  
        {
            val = 0x00334B78; /*in 81 MHZ domain its value is 0x00223251 And 81*(0x00223251/2^24)=10.58MHz . 
                                        So in 54Mhz domain (10.58/54)*2^24=0x00334B78*/
        }
    }
#else
    if ((hUhfr->settings.bShiftIf==true) && (hUhfr->settings.channel != BUHF_ChanNum_eChan9))
    {  /* If SHIFT IF is selected, use different FCW values for UHF1 and UHF 2 */
        if (hUhfr->uiIndex == 0)  
            val = 0x00217022;
        if (hUhfr->uiIndex == 1)  
            val = 0x00223251;
    }
#endif
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_FCW + offset), val);         

    BUHF_P_SetFilterCoeff (hUhfr, hUhfr->settings.filter); 

    val = (hUhfr->settings.mode == BUHF_Mode_eLegacy)? 0x00010a04: 0x00060a05;
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_IRPH + offset), val);         
    
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_IRDC + offset), 0x260b0f0a);         

    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_FRZMM + offset), 0x02020202);         
    
    val = (hUhfr->settings.mode == BUHF_Mode_eLegacy)? 0x00100100: 0x00000100;
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_FRZDC + offset), val);         

    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_DCINT+ offset), 0x00000000);         

    val = (hUhfr->settings.mode == BUHF_Mode_eLegacy)? 0xf2010b0a: 0xf2010c0a;
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_PDAS + offset), val);         

    val = (hUhfr->settings.mode == BUHF_Mode_eLegacy)? 0x00048000: 0x00010000;
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_THRAS + offset), val);         
    
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_THHYS + offset), 0x00000020);         
   
    if ((hUhfr->settings.channel == BUHF_ChanNum_eChan9))
    {
        /* For Cust2 in Legacy/Advanced Mode*/
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ALTCFG + offset), 0x20f40160);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ALTCFG2 + offset), 0xff10785f);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR1 + offset), 0x005b0c8e);       
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR4 + offset), 0x00000d6a);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR5 + offset), 0x0001ff6b);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR6 + offset), 0x0001ff6b);  
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR7 + offset), 0x003FD879);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR8 + offset), 0);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR9 + offset), 0);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR10 + offset), 0x00001919);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR16 + offset), 0x000FFFFF);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR11 + offset), 0x0708010F);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR3 + offset), 0x0c001f00);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR2 + offset), 0x00140801);  
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_IMSK + offset), 0x80000500);       
        BDBG_MSG(("BCHP_UHFR_1_IMSK=0x80000500"));    
    }

    else if (hUhfr->settings.mode == BUHF_Mode_eAdvanced)
    {
        /* For Cust1 in Advanced mode */
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR1 + offset), 0x00000D8e);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR4 + offset), 0x003FD861);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR5 + offset), 0x0000FF7B);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR6 + offset), 0x0000FFE7);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR7 + offset), 0x003FD879);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR8 + offset), 0x00007fdf);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR9 + offset), 0x00003FFF);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR10 + offset), 0x19191919);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR16 + offset), 0x000FFFFF);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR11 + offset), 0x0708010F);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR3 + offset), 0x06001f00);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR2 + offset), 0x00151400);     
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_IMSK + offset), 0x00818000);    
        BDBG_MSG(("BCHP_UHFR_1_IMSK=0x00818000"));

    }
    
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_CTL1 + offset), 0x00000401);         
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_RST + offset), 0x00010000);         

    if (hUhfr->settings.mode == BUHF_Mode_eLegacy)
    {
        /* For Cust1/Cust2 in legacy mode */
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR5 + offset), 0x003FFF7B);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR8 + offset), 0x000FFFFF);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR3 + offset), 0x26000E07);         
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_IMSK + offset), 0x0);    
        BDBG_MSG(("BCHP_UHFR_1_IMSK=0"));            
    }
    else
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_CTL1 + offset), 0x40000401);         
    }

#if ((BUHF_7420_FAMILY == 1))
    if(hUhfr->settings.channel == BUHF_ChanNum_eChan9)
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD4),0x1E240107);        
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD3),0x20D01068);        
    }
    else
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD4),0x0880DCC0);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD3),0x17700BB8);                
    } 

    val = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD5));
    val &= ~( (BCHP_MASK (UHFR_GLBL_AUTOPD5, REPEAT_PKT_TIMEOUT)));
    val |= (BCHP_FIELD_DATA (UHFR_GLBL_AUTOPD5, REPEAT_PKT_TIMEOUT,0xFA0));                         
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD5), val );       
#endif
    return ret;  
}  
    
/******************************************************************************
 * static BERR_Code BUHF_P_SetFreqOffset 
 *                          (                 
 *                              SBcmDevice  *pDevice, 
 *                              unsigned int bchError 
 *                          )
 *
 * Function:
 *      When a packet has been recieved this function is used to apply a 
 *      frequency offset adjustment to the nominal LO frequency programmed 
 *      in BUHF_P_SelectRfChan(). This will be used to compensate the 
 *      carrier offset. The UHFR_ANA3.FCW field should be modified to achieve 
 *      this offset.
 *
 * Input Parmameters:
 *      BUHF_Handle hUhfr: Pointer to UHFR device
 *      unsigned int bchError: UHFR_ISTS[23]
 *                               Values = 0 (no BCH error)
 *                                        1 (BCH error)
 *
 * Returns:
 *      BERR_SUCCESS on Success
 *      Error on failure
 *      
 * Implementation:
 *  TODO: Describe algo. 
 * 
 *
 *****************************************************************************/
#ifndef BUHF_65nm
static BERR_Code BUHF_P_SetFreqOffset
(
    BUHF_Handle hUhfr, 
    unsigned int bchError  /* 0=> no BCH error. 1=> BCH error. UHFR_ISTS[23] */
)
{
    BUHF_P_MovingAvg * pMovingAvg;
    BERR_Code   ret = BERR_SUCCESS;
    unsigned int offset=0, mask=0, regVal=0, shift=0;
   
    int  freqOffset=0, dcLevel=0, deltaFcw=0, minStep=0;
    unsigned int prCorrPeak=0, divN=0, fcw=0, rotDir=0; 
    
    int i=0;

    BDBG_ASSERT(hUhfr);

    offset = hUhfr->ui32Offset;
    pMovingAvg = hUhfr->pMovingAvg;
    
    /* Get Preamble Peak */
    regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_COR13+offset));

    if (regVal == 0) 
    {
       /* Shud we also check if Data == 0?? */
        /* Sometimes a spurios pkt end intrpt may occur and cause this fnc to be
         * called. In such cases COR13 will be 0. Do not take any action in such
         * cases */ 
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    mask =  BCHP_UHFR_1_COR13_pr_corr_peak_MASK;
    shift = BCHP_UHFR_1_COR13_pr_corr_peak_SHIFT; 
    prCorrPeak = (regVal & mask) >> shift;
    UHFR_DEBUG_FREQ_ADJUSTMENT(("Preamble Peak=0x%x, BCH Error=%d\n", prCorrPeak, bchError));

    if ((bchError==0) && (prCorrPeak > BUHF_PR_CORR_PEAK_THRES))
    {
        /* Only when there is no BCH error (UHISTS[23]=0) and pr_corr_peak > 0xf00 (this value may change),
         * you take dc_level for averaging. */   

        /* Get Preamble DC offset */
        mask = BCHP_UHFR_1_COR13_dc_level_MASK;    
        shift = BCHP_UHFR_1_COR13_dc_level_SHIFT; 
        dcLevel = (int) ((short int) (regVal & mask) >> shift); /* Sign extend */
        UHFR_DEBUG_FREQ_ADJUSTMENT(("Signed DC level offset=%d \n", dcLevel)); 

        BUHF_P_CalculateMovingAvg (hUhfr, dcLevel); 
    } 

if (pMovingAvg->overflowFlag == 1) 
{
    /* Starting of Step2 of Freq offset adjustment algorithm */
    freqOffset = 0 - ((BUHF_FREQ_RATIO_NUM * pMovingAvg->prDcOffsetAvg)/BUHF_FREQ_RATIO_DEN);
    UHFR_DEBUG_FREQ_ADJUSTMENT(("freq Offset=%d \n", freqOffset));

    if ((ABSOLUTE_VALUE(freqOffset)) > BUHF_FREQ_OFFSET_THRES) 
    {
        UHFR_DEBUG_FREQ_ADJUSTMENT(("Case1: Adjusting FCW  "));
        /* Disable UHFR by writing UHCTL1[0]=0 and UHCTL1[30]=0 */
        regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_CTL1 + offset));
        regVal &= ~(BCHP_UHFR_1_CTL1_UHFR_ENBL_MASK | BCHP_UHFR_1_CTL1_CORR_ENBL_MASK);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_CTL1 + offset), regVal);

        /* From ANA3 get coarse freqency divider(divN), the frequency control word for fine tuning, 
         * fine tuning direction (rotDir) */
        regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA3 + offset));

        mask =  BCHP_UHFR_1_ANA3_DIV_N_MASK;
        shift = BCHP_UHFR_1_ANA3_DIV_N_SHIFT; 
        divN = (regVal & mask) >> shift;
           
        mask =  BCHP_UHFR_1_ANA3_FCW_MASK;
        shift =  BCHP_UHFR_1_ANA3_FCW_SHIFT; 
        fcw = (regVal & mask) >> shift;
            
        mask = BCHP_UHFR_1_ANA3_ROT_DIR_MASK;
        shift = BCHP_UHFR_1_ANA3_ROT_DIR_SHIFT; 
        rotDir = (regVal & mask) >> shift;

        /* Calculate change in FCW */
        minStep = (int) ((BUHF_MIN_STEP_FACTOR * divN)/100);

        deltaFcw = (int) ((4*freqOffset)/minStep); 

        UHFR_DEBUG_FREQ_ADJUSTMENT(("divN=%d, ", divN));
        UHFR_DEBUG_FREQ_ADJUSTMENT(("minStep=%d, ", minStep));
        UHFR_DEBUG_FREQ_ADJUSTMENT(("deltaFcw=%d, ", deltaFcw));
        UHFR_DEBUG_FREQ_ADJUSTMENT(("Orig fcw=%d, ", fcw));
        UHFR_DEBUG_FREQ_ADJUSTMENT(("Orig rotDir=%d, ", rotDir));

        if (hUhfr->settings.injection == BUHF_InjType_eHigh) 
        {
            deltaFcw = 0 - deltaFcw;
        }

        if (fcw == 0)
        {
            if (deltaFcw < 0)
            {
                fcw = fcw - deltaFcw;
                rotDir = 0;
            }
            else 
            {
                fcw = fcw + deltaFcw;
                rotDir = 1;
            }
        }
        else if (rotDir == 1) 
        {
            if ( ((signed)fcw + deltaFcw) < 0)
            { 
                fcw = 0 - ((signed)fcw + deltaFcw); 
                rotDir = 0;
            }
            else 
            {
                fcw = fcw + deltaFcw;
            }
        }
        else 
        {
            if ( ((signed)fcw - deltaFcw) < 0)
            { 
                fcw = 0 - ((signed)fcw - deltaFcw);
                rotDir = 1;
            }
            else 
            {
                fcw = fcw - deltaFcw;
            }
        }
        UHFR_DEBUG_FREQ_ADJUSTMENT(("New fcw=%d, ", fcw));
        UHFR_DEBUG_FREQ_ADJUSTMENT(("New rotDir=%d\n", rotDir));

        /* Save new fcw in adjustedFcw */
        hUhfr->adjustedFcw = fcw;

        /* Set the new fcw value to UHANA3[7:0] and make UHANA3[28](pll_reset) "1" */
        regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA3 + offset));
        mask = BCHP_UHFR_1_ANA3_FCW_MASK; 
        regVal &= ~ mask; /* Clear FCW bits */
        shift = BCHP_UHFR_1_ANA3_FCW_SHIFT; 
        regVal |= (fcw << shift); /* Set new FCW value */
        mask = BCHP_UHFR_1_ANA3_ROT_DIR_MASK;
        regVal &= ~ mask; /* Clear rotDir bits */
        shift = BCHP_UHFR_1_ANA3_ROT_DIR_SHIFT;
        regVal |= (rotDir << shift); 
 
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA3 + offset), regVal);         
    
        regVal = 0x00000001; /* start serial shift to analog */
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANACTL + offset), regVal);         
       
        /* Issue soft reset by setting UHRST[16]=1 */
        regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_RST + offset));
        mask = BCHP_UHFR_1_RST_SOFT_RST_MASK; 
        regVal |= mask;
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_RST + offset), regVal); 

        /* Enable UHFR by writing UHCTL1[0]=1 and UHCTL1[30]=1 */
        regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_CTL1 + offset));
        regVal |= (BCHP_UHFR_1_CTL1_UHFR_ENBL_MASK | BCHP_UHFR_1_CTL1_CORR_ENBL_MASK);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_CTL1 + offset), regVal );            

        /* Discard old DC level values. Reset counter, sum etc */
        pMovingAvg->logCount = 0;
        pMovingAvg->overflowFlag = 0;
        pMovingAvg->prDcOffsetSum = 0;
        pMovingAvg->prDcOffsetAvg = 0;
        for (i=0; i < BUHF_MAX_LOG_SIZE; i++)
        {
            pMovingAvg->prDcOffset[i] = 0;  
        }       
    }
    else if (((ABSOLUTE_VALUE(freqOffset)) < BUHF_FREQ_OFFSET_THRES) && (hUhfr->settings.filter == BUHF_eLPF))
    { 
        UHFR_DEBUG_FREQ_ADJUSTMENT(("Case2: Switch from LPF to BPF \n"));
        
        /* Disable UHFR by writing UHCTL1[0]=0 and UHCTL1[30]=0 */
        regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_CTL1 + offset));
        regVal &= ~(BCHP_UHFR_1_CTL1_UHFR_ENBL_MASK | BCHP_UHFR_1_CTL1_CORR_ENBL_MASK);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_CTL1 + offset), regVal );

        /* Set BPF Filter. */
        ret = BUHF_P_SetFilterCoeff (hUhfr, BUHF_eBPF);
        if (ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BUHF_P_SetFreqOffset: Changing to BPF Failed.\n"));
            return BERR_TRACE(ret);
        }  

        /* Issue soft reset by setting UHRST[16]=1 */
        regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_RST + offset));
        mask = BCHP_UHFR_1_RST_SOFT_RST_MASK; 
        regVal |= mask;
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_RST + offset), regVal); 

        /* Enable UHFR by writing UHCTL1[0]=1 and UHCTL1[30]=1 */
        regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_CTL1 + offset));
        regVal |= (BCHP_UHFR_1_CTL1_UHFR_ENBL_MASK | BCHP_UHFR_1_CTL1_CORR_ENBL_MASK);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_CTL1 + offset), regVal );        
        
        /* Discard old DC level values. Reset counter, sum etc */
        pMovingAvg->logCount = 0;
        pMovingAvg->overflowFlag = 0;
        pMovingAvg->prDcOffsetSum = 0;
        pMovingAvg->prDcOffsetAvg = 0;
        for (i=0; i < BUHF_MAX_LOG_SIZE; i++)
        {
            pMovingAvg->prDcOffset[i] = 0;  
        }      
    }
    else if (((ABSOLUTE_VALUE(freqOffset)) < BUHF_FREQ_OFFSET_THRES) && (hUhfr->settings.filter == BUHF_eBPF))
    { 
        UHFR_DEBUG_FREQ_ADJUSTMENT(("Case3: Do nothing \n"));
    }
    else 
    {
        BDBG_ERR(("Error!!! no condition met.\n"));
    }
}

    return BERR_SUCCESS;
} 

#endif

#ifndef BUHF_65nm
/******************************************************************************
 * static BERR_Code BUHF_P_CalculateMovingAvg 
 *                          (
 *                              SBcmDevice *pDevice, 
 *                              int value  
 *                          )
 *
 * Function:
 *
 * Input Parmameters:
 *      BUHF_Handle hUhfr: Pointer to UHFR device
 *      int value          : Value of new DC Level to be added to the log and
 *                           used to calculate average
 *                           
 * Output:
 *     The calculcate value of average DC level is saved in 
 *     pMovingAvg->prDcOffsetAvg for later references.
 * 
 * Returns:
 *      BERR_SUCCESS on Success
 *      Error value on failure
 *      
 * Implementation:
 *      Wait till we've accumulated BUHF_MAX_LOG_SIZE values, then calculate the
 *      average. After that, for each new value, keep over-writing the buffer
 *      and recompute the average each time.
 *
 *****************************************************************************/

static BERR_Code BUHF_P_CalculateMovingAvg 
(
    BUHF_Handle hUhfr, 
    int value  /* Data to be added to the log, and used in calculating the average */
)
{
    BUHF_P_MovingAvg * pMovingAvg;

    BDBG_ASSERT(hUhfr);

    pMovingAvg = hUhfr->pMovingAvg;

    /* Wait till we've accumulated BUHF_MAX_LOG_SIZE values, then calculate the avg.
     * AFter that, for each new value, keep over-writing the buffer and
     * recompute the avg each time.
     */
    
    pMovingAvg->prDcOffsetSum += ( value - pMovingAvg->prDcOffset[pMovingAvg->logCount]); /* sum = sum - old_value + new_value */
    pMovingAvg->prDcOffset[pMovingAvg->logCount] = value; /* Save new value */

    pMovingAvg->logCount++;
    if (pMovingAvg->logCount == BUHF_MAX_LOG_SIZE)
    {
        pMovingAvg->logCount = 0;
        pMovingAvg->overflowFlag = 1;
    }

    /* Wait till we have BUHF_MAX_LOG_SIZE samples before we take the average. */
    if (pMovingAvg->overflowFlag == 1)
    {
          pMovingAvg->prDcOffsetAvg = (pMovingAvg->prDcOffsetSum)/BUHF_MAX_LOG_SIZE;
    }
    
    UHFR_DEBUG_FREQ_ADJUSTMENT(("Calculating avg: log_cnt=%d, overflow_flag=%d, val=%d, sum=%d, avg=%d\n", 
        pMovingAvg->logCount, pMovingAvg->overflowFlag, value, pMovingAvg->prDcOffsetSum, pMovingAvg->prDcOffsetAvg));
    return BERR_SUCCESS;
} 

#endif

/* For BCH error Correction.  */
/*
 *  This function strips the transmitted 10 bit
 *  crc off the 26 bit packet and replaces the crc with zeros.  
 *  The new crc is calculated and compared with the transmitted crc.  
 *  If the crc's don't match, bit errors have occured.  Using the 
 *  difference between the crc's and the syndrome look up table, up
 *  to two bit errors can be corrected.  After an attempt to correct
 *  errors has been made, the crc will be calculated again to verify
 *  the packet has been repaired properly.
 *  
 *
 *  \param input - 26 bit fsk packet
 *
 *  \return  eDeviceExtError for error, BERR_SUCCESS for OK
 */
static BERR_Code BUHF_P_BchErrorCorrection 
(   
    BUHF_Handle hUhfr, 
    unsigned int *packet
)
{
    unsigned long calc_crc;
    unsigned long syndrome;
    unsigned long generator = GENERATOR_CODE << KEY_DATA_SIZE;
    int the_index = KEY_DATA_SIZE + CRC_SIZE;
    unsigned int temp_pkt;
    unsigned int offset=0, mask=0, regVal=0, shift=0;
       
    BDBG_ASSERT(hUhfr);
    offset = hUhfr->ui32Offset;
    
    temp_pkt = *packet;
    
    /* Get syndrome from COR14 */
    regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_COR14+offset));
    mask = BCHP_UHFR_1_COR14_bch_synd_MASK;
    shift = BCHP_UHFR_1_COR14_bch_synd_SHIFT; 
    syndrome = (regVal & mask) >> shift;

    if (syndrome) /* error detected */
    {
        hUhfr->errPkt++;
        
        if (syndrome < SYNDROME_TABLE_SIZE)
        {
            /* Right shift packet by 6 */
            temp_pkt = ((temp_pkt) >> 6);
      
            temp_pkt ^= syndrome_table[syndrome]; /* attempt repair */
         
            the_index = KEY_DATA_SIZE + CRC_SIZE;
            generator = GENERATOR_CODE << KEY_DATA_SIZE;
            calc_crc = temp_pkt;    /* Don't strip off transmitted crc */

            /* re - calculate crc - result should be zero if packet is fixed */
            while (the_index >= CRC_SIZE)    
            {
                if (calc_crc & (1 << the_index)) 
                {
                    calc_crc ^= generator;
                }
                generator >>= 1;
                the_index--;
            }
         
            if (calc_crc &= CRC_MASK)
            {
                BDBG_MSG(("BUHF_P_BchErrorCorrection: BCH error corrections failed.\n"));
                return BERR_TRACE(BERR_UNKNOWN);
            }
            else
            {   
                /* Left shift packet by 6 */
                temp_pkt = ((temp_pkt) << 6);              

                /* Copy back to orignal packet memory */
                *packet = temp_pkt;
        
                hUhfr->correctedErrPkt++;
                
                BDBG_MSG(("BUHF_P_BchErrorCorrection: BCH error corrected.\n"));
                return BERR_SUCCESS;
            }
        
        }
        BDBG_MSG(("BUHF_P_BchErrorCorrection: BCH error corrections failed. syndrome > SYNDROME_TABLE_SIZE \n"));
        return BERR_TRACE(BERR_UNKNOWN);
   }
   return BERR_SUCCESS;
}


BERR_Code BUHF_Open(
    BUHF_Handle *phUhfr,                /* [out] Returns Device handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle */
    unsigned int devNum,                /* [in] UHFR no. 0 or 1 */
    const BUHF_Settings *pSettings      /* [in] Device settings */
)
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int i;
    BUHF_Handle hUhfr;    

    /* Check parameters passed to this function */
    BDBG_ASSERT(phUhfr);
    BDBG_ASSERT(hChip);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(hInterrupt);
    BDBG_ASSERT(pSettings);

    BDBG_MSG(("devNum=%d", devNum));
    BDBG_MSG(("mode=%d", pSettings->mode));
    BDBG_MSG(("channel=%d", pSettings->channel));
    BDBG_MSG(("injection=%d", pSettings->injection));
    BDBG_MSG(("filter=%d", pSettings->filter));
    BDBG_MSG(("bchErrorCorrection=%d", pSettings->bchErrorCorrection));
    BDBG_MSG(("bFreqAdjust=%d", pSettings->bFreqAdjust));
    BDBG_MSG(("bShiftIf=%d", pSettings->bShiftIf));
    BDBG_MSG(("uiFreqShift=%d", pSettings->uiFreqShift));
    
    /* Make sure device number is valid, and get offset */
    if (devNum >= BUHF_MAX_DEVICES) 
    {
        BDBG_ERR(("BUHF_Open: Invalid Device number.\n"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Shift IF can be enabled only for 65nm, Dual UHFR chips. */
#if !((BUHF_65nm==1) && (BUHF_MAX_DEVICES==2))
    if (pSettings->bShiftIf == true)
    {
        BDBG_ERR(("%d is not a 65nm chip with 2 UHF devices. Please disable shift IF. ", BCHP_CHIP));
        return BERR_INVALID_PARAMETER;        
    }
#endif
    if ((pSettings->bShiftIf == true) && (pSettings->channel==BUHF_ChanNum_eChan9))
    {
        BDBG_ERR(("Shift IF is not valid for Channel9. Please disable shift IF. "));
        return BERR_INVALID_PARAMETER;        
    }

    /* Allocate the memory for the Device handle */
    hUhfr = (BUHF_Handle) BKNI_Malloc( sizeof(BUHF_P_Device));
    if(hUhfr == NULL)
    {
        ret = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return ret;
    }
    /* Reset the entire structure */
    BKNI_Memset (hUhfr, 0, sizeof(BUHF_P_Device));
    *phUhfr = hUhfr;
  
    /* Initialise UHFR Handle */
    hUhfr->hChip = hChip;
    hUhfr->hRegister = hRegister;
    hUhfr->hInterrupt = hInterrupt;
    hUhfr->settings = *pSettings;
    hUhfr->uiIndex = devNum;

#if (BUHF_MAX_DEVICES > 1)
    /* If htere are more than 1 UHF devices, get correct offset for all*/
    hUhfr->ui32Offset = devNum *(BCHP_UHFR_2_RST- BCHP_UHFR_1_RST); 
#else
    hUhfr->ui32Offset = 0; /* Since only 1 UHFR device*/
#endif    

    hUhfr->origFcw = 0;
    hUhfr->adjustedFcw = 0;
    hUhfr->intFlag = 0;
    hUhfr->data.value = 0;
    hUhfr->data.prType = BUHF_PrType_eNone;
    hUhfr->totalPkt = 0;
    hUhfr->errPkt = 0;
    hUhfr->correctedErrPkt = 0;
    hUhfr->bStandbyState = false;

    /* Create event handler */
    ret = BKNI_CreateEvent( &(hUhfr->hDataReadyEvent));
    if (ret != BERR_SUCCESS)
    {
         BDBG_ERR(("Could not install event handler"));
         BKNI_Free(hUhfr); /* free the memory allocated for handle before returning here*/         
         return BERR_TRACE(ret);
    }
    
    /* Install ISR */

#ifdef BCHP_PWR_RESOURCE_UHF_INPUT
    BCHP_PWR_AcquireResource(hUhfr->hChip, BCHP_PWR_RESOURCE_UHF_INPUT);
#endif

#if (BUHF_MAX_DEVICES == 1)
    if (hUhfr->settings.mode == BUHF_Mode_eAdvanced)    
    {
        if (hUhfr->settings.channel == BUHF_ChanNum_eChan9) 
        {
            ret = BINT_CreateCallback (
                &hUhfr->hCallback1,
                hUhfr->hInterrupt,
                BCHP_INT_ID_IMSK_ALT_PACKET_END, 
                BUHF_P_Cust2Isr1,
                (void*)hUhfr,
                0 /* Not used */);
            ret = BINT_EnableCallback (hUhfr->hCallback1);

            ret = BINT_CreateCallback (
                &hUhfr->hCallback2,
                hUhfr->hInterrupt,
                BCHP_INT_ID_IMSK_ALT_SFR, 
                BUHF_P_Cust2Isr2,
                (void*)hUhfr,
                0 /* Not used */);
            ret = BINT_EnableCallback (hUhfr->hCallback2);  
        }
        else    
        {
            ret = BINT_CreateCallback (
                &hUhfr->hCallback1,
                hUhfr->hInterrupt,
                BCHP_INT_ID_IMSK_CORR_DECODE_PR1_END, 
                BUHF_P_Isr,
                (void*)hUhfr,
                0 /* Not used */);
            ret = BINT_EnableCallback (hUhfr->hCallback1);
    
            ret = BINT_CreateCallback (
                &hUhfr->hCallback2,
                hUhfr->hInterrupt,
                BCHP_INT_ID_IMSK_CORR_DECODE_PR2_END, 
                BUHF_P_Isr,
                (void*)hUhfr,
                0 /* Not used */);
            ret = BINT_EnableCallback (hUhfr->hCallback2);
        }
    }
#else
    if (hUhfr->uiIndex == 0)
    {
        if (hUhfr->settings.mode == BUHF_Mode_eAdvanced)    
        {
            if (hUhfr->settings.channel == BUHF_ChanNum_eChan9) 
            {
                ret = BINT_CreateCallback (
                    &hUhfr->hCallback1,
                    hUhfr->hInterrupt,
                    BCHP_INT_ID_UHFR_1_IMSK_ALT_PACKET_END, 
                    BUHF_P_Cust2Isr1,
                    (void*)hUhfr,
                    0 /* Not used */);
                ret = BINT_EnableCallback (hUhfr->hCallback1);

                ret = BINT_CreateCallback (
                    &hUhfr->hCallback2,
                    hUhfr->hInterrupt,
                    BCHP_INT_ID_UHFR_1_IMSK_ALT_SFR, 
                    BUHF_P_Cust2Isr2,
                    (void*)hUhfr,
                    0 /* Not used */);
                ret = BINT_EnableCallback (hUhfr->hCallback2);  
            }
            else    
            {
                ret = BINT_CreateCallback (
                    &hUhfr->hCallback1,
                    hUhfr->hInterrupt,
                    BCHP_INT_ID_UHFR_1_IMSK_CORR_DECODE_PR1_END, 
                    BUHF_P_Isr,
                    (void*)hUhfr,
                    0 /* Not used */);
                ret = BINT_EnableCallback (hUhfr->hCallback1);
        
                ret = BINT_CreateCallback (
                    &hUhfr->hCallback2,
                    hUhfr->hInterrupt,
                    BCHP_INT_ID_UHFR_1_IMSK_CORR_DECODE_PR2_END, 
                    BUHF_P_Isr,
                    (void*)hUhfr,
                    0 /* Not used */);
                ret = BINT_EnableCallback (hUhfr->hCallback2);
            }
        }
    }
    else if (hUhfr->uiIndex == 1)
    {
        if (hUhfr->settings.mode == BUHF_Mode_eAdvanced)    
        {
            if (hUhfr->settings.channel == BUHF_ChanNum_eChan9) 
            {
                ret = BINT_CreateCallback (
                    &hUhfr->hCallback1,
                    hUhfr->hInterrupt,
                    BCHP_INT_ID_UHFR_2_IMSK_ALT_PACKET_END, 
                    BUHF_P_Cust2Isr1,
                    (void*)hUhfr,
                    0 /* Not used */);
                ret = BINT_EnableCallback (hUhfr->hCallback1);

                ret = BINT_CreateCallback (
                    &hUhfr->hCallback2,
                    hUhfr->hInterrupt,
                    BCHP_INT_ID_UHFR_2_IMSK_ALT_SFR, 
                    BUHF_P_Cust2Isr2,
                    (void*)hUhfr,
                    0 /* Not used */);
                ret = BINT_EnableCallback (hUhfr->hCallback2);  
            }
            else    
            {
                ret = BINT_CreateCallback (
                    &hUhfr->hCallback1,
                    hUhfr->hInterrupt,
                    BCHP_INT_ID_UHFR_2_IMSK_CORR_DECODE_PR1_END, 
                    BUHF_P_Isr,
                    (void*)hUhfr,
                    0 /* Not used */);
                ret = BINT_EnableCallback (hUhfr->hCallback1);
        
                ret = BINT_CreateCallback (
                    &hUhfr->hCallback2,
                    hUhfr->hInterrupt,
                    BCHP_INT_ID_UHFR_2_IMSK_CORR_DECODE_PR2_END, 
                    BUHF_P_Isr,
                    (void*)hUhfr,
                    0 /* Not used */);
                ret = BINT_EnableCallback (hUhfr->hCallback2);
            }
        }
    }
#endif

    BDBG_MSG(("UHF interrupt callbacks installed for UHF device %d", hUhfr->uiIndex)); 
 
    hUhfr->pMovingAvg =  (BUHF_P_MovingAvg *)BKNI_Malloc (sizeof(BUHF_P_MovingAvg)); 
    if (hUhfr->pMovingAvg == NULL)
    {
        BDBG_ERR(("BUHF_Open: Memory allocation for UHFR Moving Avg data structure failed.\n"));
        ret = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BKNI_Free(hUhfr); /* free the memory allocated for handle before returning here*/
        return ret; 
    }
        
    hUhfr->pMovingAvg->logCount = 0;
    hUhfr->pMovingAvg->overflowFlag = 0;
    hUhfr->pMovingAvg->prDcOffsetAvg = 0;
    hUhfr->pMovingAvg->prDcOffsetSum = 0;
    for (i=0; i < BUHF_MAX_LOG_SIZE; i++)
    {
        hUhfr->pMovingAvg->prDcOffset[i] = 0;
    }
        
    /* Initialise UHFR Registers */
    ret = BUHF_P_InitReg (hUhfr);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BUHF_Open: Failed to initialise UHFR registers correctly.\n"));
        BKNI_Free(hUhfr->pMovingAvg); 
        BKNI_Free(hUhfr); /* free the memory allocated for handle before returning here*/
        return BERR_TRACE(ret);
    }

    hUhfr->uhfCb = NULL; /* no callback until they register one */
    hUhfr->pData = NULL;

    BUHF_P_SelectRfChan (hUhfr);

     return ret;
}


BERR_Code BUHF_Close
(
    BUHF_Handle hUhfr   
)
{
    /* Check parameters passed to this function */
    BDBG_ASSERT(hUhfr); 
  
    if (hUhfr->uhfCb)
        BUHF_UnregisterCallback(hUhfr);

    if (hUhfr->hCallback1 != 0)
    {
        BINT_DisableCallback (hUhfr->hCallback1);
        BINT_DestroyCallback (hUhfr->hCallback1);
    }   
    if (hUhfr->hCallback2 != 0)
    {
        BINT_DisableCallback (hUhfr->hCallback2);
        BINT_DestroyCallback (hUhfr->hCallback2);
    }

    BKNI_DestroyEvent(hUhfr->hDataReadyEvent);
    
#ifdef BCHP_PWR_RESOURCE_UHF_INPUT
    BCHP_PWR_ReleaseResource(hUhfr->hChip, BCHP_PWR_RESOURCE_UHF_INPUT);
#endif
    
    BKNI_Free(hUhfr->pMovingAvg);
    BKNI_Free(hUhfr);
    
    return BERR_SUCCESS;
}

BERR_Code BUHF_GetEventHandle(
    BUHF_Handle hUhfr,          /* [in] Device  handle */
    BKNI_EventHandle *phEvent   /* [out] Returns event handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(phEvent);

    *phEvent = hUhfr->hDataReadyEvent;
    return( retCode );
}

void BUHF_RegisterCallback (
    BUHF_Handle hUhfr,      /* [in] UHFR Device handle */
    BUHF_Callback callback, /* [in] Callback function to register */
    void *pData             /* [in] Data passed to callback function */
)
{
    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(callback);

    hUhfr->uhfCb = callback;
    hUhfr->pData = pData;
    return;
}

void BUHF_UnregisterCallback (
    BUHF_Handle hUhfr       /* [in] UHFR Device handle */
)
{
    BDBG_ASSERT(hUhfr);

    hUhfr->uhfCb = NULL;
    hUhfr->pData = 0;
    return;
}


/******************************************************************************
 *
 * Function:
 *      The interrupt handler checks which UHFR interrupt has been received 
 *      and services it accordingly. The interrupt bit is cleared by writing 
 *      a 1 to it. 
 * 
 * Returns:
 *      BERR_SUCCESS on Success
 *      Error value on other errors.
 *
 * Implementation:
 *      When a eUhfrIntCorrDecodePr1End or eUhfrIntCorrDecodePr2End 
 *      UHFR interrupt has occurred => a packet has been recieved. 
 *      On recieving a packet, the interrupt flag in UHFR configuration 
 *      data structure is set to indicate that there is data pending to be read; 
 *      this data is saved in pUhfrConfig->data to prevent loss on reset (soft
 *      resets are done by the frequency offset adjustment algorithm), and the 
 *      frequency offset adjustment algorithm is invoked.
 * 
 *****************************************************************************/
/* TODO: need to clean up this ISR */
static void BUHF_P_Isr
(
    void *pUhfr,      /* [in] Handle to UHFR device */
    int  iParam2      /* Not used */
)
{
#ifndef BUHF_65nm
    BERR_Code   ret = BERR_SUCCESS;
#endif
    unsigned int irq_status, mask, temp;
    BUHF_Handle hUhfr;
    unsigned int offset;
    unsigned int bchError=0;

    BSTD_UNUSED(iParam2);
    BDBG_ASSERT(pUhfr);
    
    hUhfr = (BUHF_Handle) pUhfr;
    offset = hUhfr->ui32Offset;
       
    irq_status = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_ISTS+offset));
    hUhfr->ui32IntStatus = irq_status;

    mask = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_IMSK+offset));
    
    BDBG_MSG(("BCHP_UHFR_1_ISTS=0x%08x, BCHP_UHFR_1_IMSK=0x%08x", irq_status, mask));    
    
    /* If the interrupt is enabled and is set in the interrupt status register,
     * clear it and set the flag */
    temp = irq_status & mask;
    
    if (temp & BCHP_UHFR_1_ISTS_CORR_DECODE_PR1_END_MASK)
    {
        BDBG_MSG(("bcmUhfrIsr: CORR_DECODE_PR1_END interrupt.\n"));
        irq_status |= (BCHP_UHFR_1_ISTS_CORR_DECODE_PR1_END_MASK); /* Clear the intrpt status bit by writing 1 to it */
        hUhfr->intFlag = 1; 
        hUhfr->data.prType = BUHF_PrType_e1;
    }

    if (temp & BCHP_UHFR_1_ISTS_CORR_DECODE_PR2_END_MASK)
    {
        BDBG_MSG(("bcmUhfrIsr: CORR_DECODE_PR2_END interrupt.\n"));
        irq_status |= (BCHP_UHFR_1_ISTS_CORR_DECODE_PR2_END_MASK); /* Clear the intrpt status bit by writing 1 to it */
        hUhfr->intFlag = 1; 
        hUhfr->data.prType = BUHF_PrType_e2;
    }

    if (hUhfr->intFlag == 1)
    {
        /* The PLL workaround and the freq offset adjustment algo etc require a soft
         * reset. Soft reset also clears out COR12 => data is lost on a soft reset. 
         * Therefore, in the ISR, we save the data into hUhfr->data and read
         * it from there always. */
        /* Save the packet */
        hUhfr->data.value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_COR12+offset));

        BDBG_MSG(("data.value = %x",hUhfr->data.value));

        /* Increment pkt count */
        hUhfr->totalPkt++;
        
        bchError = (temp & BCHP_UHFR_1_ISTS_CORR_BCH_ERR_MASK) >> BCHP_UHFR_1_ISTS_CORR_BCH_ERR_SHIFT;
       
        if (bchError == 1) 
        { 
            if (hUhfr->settings.bchErrorCorrection == 1) 
            {  
               /* If enabled do BCH error correction on the recieved data*/
               temp = BUHF_P_BchErrorCorrection (hUhfr, &(hUhfr->data.value));

                /* If BCH error correction was successful, clear intrpt */
                if (temp == BERR_SUCCESS)
                {
                    irq_status |= (BCHP_UHFR_1_ISTS_CORR_BCH_ERR_MASK); /* Clear the intrpt status bit by writing 1 to it */
                }
            }
        }

#ifndef BUHF_65nm
        /* If a packet has been recieved, calculate and adjust frequency offset */  
        if (hUhfr->settings.bFreqAdjust == 1) 
        {
            /* Sometimes a spurios pkt end intrpt may occur and cause this fnc to be
             * called. In such cases COR13 will be 0. Do not take any action */ 
            ret = BUHF_P_SetFreqOffset(hUhfr, bchError); 
        }
#endif

        /* Call back to the registered callback function */
        if (hUhfr->uhfCb)
            hUhfr->uhfCb(hUhfr, hUhfr->pData);

        BKNI_SetEvent (hUhfr->hDataReadyEvent);
    }  

    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ISTS + offset), irq_status);
    return;
}

static void BUHF_P_Cust2Isr1
(
    void *pUhfr,      /* [in] Handle to UHFR device */
    int  iParam2      /* Not used */
)
{
    unsigned int irq_status, mask, temp;
    unsigned int offset = 0;
    BUHF_Handle hUhfr;
    bool bError=false;

    BDBG_ASSERT(pUhfr);
    BSTD_UNUSED(iParam2);
    
    hUhfr = (BUHF_Handle) pUhfr;
    offset = hUhfr->ui32Offset;

    irq_status = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_ISTS+offset));
    
    hUhfr->ui32IntStatus = irq_status;

    mask = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_IMSK+offset));
    
    /* If the interrupt is enabled and is set in the interrupt status register, clear it and set the flag */
    temp = irq_status & mask;
    
    if  ((temp & (BCHP_UHFR_1_ISTS_ALT_PACKET_END_MASK |
                               BCHP_UHFR_1_ISTS_CORR_ALT_CHSM_ERR_MASK |
                               BCHP_UHFR_1_ISTS_ALT_SFR_MASK))
                               != BCHP_UHFR_1_ISTS_ALT_PACKET_END_MASK)    
    {
        bError = true;
        BDBG_MSG(("BUHF_P_Cust2Isr1: BAD interrupt occured. Decode aborted early. Data not valid.\n"));
        if (temp & BCHP_UHFR_1_ISTS_CORR_ALT_CHSM_ERR_MASK)
        {
            BDBG_MSG(("BUHF_P_Cust2Isr1: Checksum error interrupt occured.\n"));
            irq_status |= (BCHP_UHFR_1_ISTS_CORR_ALT_CHSM_ERR_MASK); /* Clear the intrpt status bit by writing 1 to it */
        }
         if (temp & BCHP_UHFR_1_ISTS_ALT_SFR_MASK)
        {
            BDBG_MSG(("BUHF_P_Cust2Isr1: ALT_SFR interrupt occured. Decode aborted early. Data not valid.\n"));
            irq_status |= (BCHP_UHFR_1_ISTS_ALT_SFR_MASK); /* Clear the intrpt status bit by writing 1 to it */
        }       
    }        
    else
    {
        hUhfr->intFlag = 1; 
        hUhfr->data.value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_ALTDTM+offset));
        hUhfr->data.prType = BUHF_PrType_eNone;
    }

    irq_status |= (BCHP_UHFR_1_ISTS_ALT_PACKET_END_MASK); /* Clear the intrpt status bit by writing 1 to it */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ISTS + offset), irq_status);

    if(bError == false) {
        if (hUhfr->uhfCb)
            hUhfr->uhfCb(hUhfr, hUhfr->pData);
        BKNI_SetEvent (hUhfr->hDataReadyEvent);
	}
}


static void BUHF_P_Cust2Isr2
(
    void *pUhfr,      /* [in] Handle to UHFR device */
    int  iParam2      /* Not used */
)
{
    unsigned int irq_status;
    unsigned int offset = 0;
    BUHF_Handle hUhfr;

    BDBG_ASSERT(pUhfr);
    BSTD_UNUSED(iParam2);
    
    hUhfr = (BUHF_Handle) pUhfr;
    offset = hUhfr->ui32Offset;    

    irq_status = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_ISTS+offset));
    
    hUhfr->ui32IntStatus = irq_status;

    BDBG_MSG(("BUHF_P_Cust2Isr2: ALT_SFR interrupt occured. Decode aborted early. Data not valid.\n"));

    irq_status |= (BCHP_UHFR_1_ISTS_ALT_SFR_MASK); /* Clear the intrpt status bit by writing 1 to it */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ISTS + offset), irq_status);
}

BERR_Code BUHF_IsDataReady
(
    BUHF_Handle hUhfr,                  /* [in] UHFR Device handle */
    unsigned char    *pFlag             /* [out] flag indicating data is available */
)
{
    BERR_Code   ret = BERR_SUCCESS;
    unsigned int offset = 0;
    
    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(pFlag);

    /* This function is available only in advanced mode */
    if (hUhfr->settings.mode == BUHF_Mode_eLegacy)
    {
        BDBG_ERR(("BUHF_IsDataReady: not Available in UHFR legacy mode.\n"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
     
    BDBG_MSG(("BUHF_IsDataReady: Checking if there's data to be read ...\n")); 

    offset = hUhfr->ui32Offset;
    *pFlag = 0; /* Assume no data is ready */
    
    if (hUhfr->intFlag == 1)
    {
        *pFlag = 1; 
    }
    return ret;
}


BERR_Code BUHF_Read
(
    BUHF_Handle hUhfr,              /* [in] UHFR Device handle */
    BUHF_Data   *pData              /* [out] ptr to where data is to be saved */
)
{
    BERR_Code   ret = BERR_SUCCESS;
    
    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(pData);

    /* This function is available only in advanced mode */
    if (hUhfr->settings.mode == BUHF_Mode_eLegacy)
    {
        BDBG_ERR(("BUHF_IsDataReady: not Available in UHFR legacy mode.\n"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("bcmUhfrRead: Reading Data...\n")); 

    /* The PLL workaround and the freq offset adjustment algo etc require a soft
     * reset. Soft reset also clears out COR12 => data is lost on a soft reset. 
     * Therefore, in the ISR, we save the data into pUhfrConfig->data and read
     * it from there always. */
    *pData = hUhfr->data;

    /* Clear the intrpt flag and data */
    hUhfr->intFlag = 0;
    hUhfr->data.value = 0;
    hUhfr->data.prType = BUHF_PrType_eNone;

    return ret;
}  

BERR_Code BUHF_GetDefaultSettings (
    BUHF_Settings *pDefSettings     /* [out] Returns default settings */
)
{
    *pDefSettings = defUhfrSettings;
    return BERR_SUCCESS;
}

BERR_Code BUHF_GetCurrentSettings (
    BUHF_Handle     hUhfr,          /* [in] UHFR Device handle */
    BUHF_Settings   *pSettings      /* [out] Returns current settings */
)
{
    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(pSettings);

    *pSettings = hUhfr->settings;
    return BERR_SUCCESS;
}

BERR_Code BUHF_GetStatus 
(
    BUHF_Handle   hUhfr,    /* [in] UHFR Device handle */
    BUHF_Status *pStatus    /* [out] ptr to where Status is to be saved */
)
{
    BERR_Code   ret = BERR_SUCCESS;
    unsigned int offset =0, mask=0, regVal=0, shift=0;    
    
    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(pStatus);
 
    offset = hUhfr->ui32Offset;
     
    /* This function is available only in advanced mode */
    if (hUhfr->settings.mode == BUHF_Mode_eLegacy)
    {
        BDBG_ERR(("BUHF_IsDataReady: not Available in UHFR legacy mode.\n"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("BUHF_GetStatus: Reading Status...\n")); 

    /* Get BCH error status bit */
    regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_ISTS+offset));
    mask = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_IMSK+offset));

    regVal = regVal & mask; /* Read the bit only if this interrupt has been enable in IMSK */
    mask = BCHP_UHFR_1_ISTS_CORR_BCH_ERR_MASK;
    shift = BCHP_UHFR_1_ISTS_CORR_BCH_ERR_SHIFT; 
    
    pStatus->bchError = (regVal & mask) >> shift;

    /* Read COR 13 */
    regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_COR13+offset));

    /* Get Preamble Peak */
    mask = BCHP_UHFR_1_COR13_pr_corr_peak_MASK;
    shift = BCHP_UHFR_1_COR13_pr_corr_peak_SHIFT; 
    pStatus->prCorrPeak = (regVal & mask) >> shift;

    /* Get Preamble DC offset */
    mask = BCHP_UHFR_1_COR13_dc_level_MASK;    
    shift = BCHP_UHFR_1_COR13_dc_level_SHIFT; 
    pStatus->dcLevel = (int) ((short int) (regVal & mask) >> shift); /* Sign extend */

    BDBG_MSG(("BCH Error=%d, Preamble Peak=0x%x, DC Level=%d, Slow RSSI out=%d \n",
        pStatus->bchError, pStatus->prCorrPeak, pStatus->dcLevel, pStatus->slowRssiOut));

    pStatus->totalPkt = hUhfr->totalPkt;
    pStatus->errPkt = hUhfr->errPkt;
    pStatus->correctedErrPkt = hUhfr->correctedErrPkt;
    pStatus->ui32IntStatus = hUhfr->ui32IntStatus;

    return ret;
}

/******************************************************************************
Summary:
      Configures to one of the 9 RF channels defined .

 Description:
      Depending on the channel selected, we set the values for UHFR Analog 
      Control Register 3 ie fRF, fLO, fVCO, fPI, vco_div, div2, rot_dir, 
      divN[7:0] and fcw[13:0]. 

 Returns:
      BERR_SUCCESS on Success
      eDeviceBadDevice if invalid device number is given
      eDeviceBadParam if invalid channel number or injection value is given
      Error value on other errors.

Returns:
    BERR_SUCCESS - if open is successful.
    Error value  - if not successful. *
*****************************************************************************/
#ifndef BUHF_65nm

BERR_Code BUHF_P_SelectRfChan
(
    BUHF_Handle     hUhfr           /* [in] UHFR Device handle */
)
{
    unsigned int offset=0, mask=0, shift=0;
    unsigned int value, ana3_val;

    BDBG_ASSERT(hUhfr);
    
    offset = hUhfr->ui32Offset;
    
    value = channelTable[hUhfr->settings.injection][hUhfr->settings.channel];
    ana3_val = value;
    value = ana3_val | 0x30000000;  /* assert fil_rst and syn_rst */
    
    /* Save the FCW from the ANA3 as origFcw */
    mask =  BCHP_UHFR_1_ANA3_FCW_MASK;
    shift =  BCHP_UHFR_1_ANA3_FCW_SHIFT; 
    hUhfr->origFcw = (value & mask) >> shift;

    value = ana3_val | 0x30000000;  /* assert fil_rst and syn_rst */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA3 + offset), value);       
    
    value = 0x00000001; /* start serial shift to analog */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANACTL + offset), value);         
        
    BKNI_Delay(20);

    value = ana3_val & 0xcfffffff;  /* deassert fil_rst and syn_rst */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA3 + offset), value);       
    
    value = 0x00000001; /* start serial shift to analog */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANACTL + offset), value);         

    if (hUhfr->settings.injection == BUHF_InjType_eHigh) 
    {
        if (hUhfr->settings.mode == BUHF_Mode_eLegacy) /* legacy mode regardless of Cust1 or Cust2 */
        {   value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_FCW + offset));
            value = ~(value) + 1; 
            BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_FCW + offset), value);
        }
        /* COR1, COR4, and COR7 are changed in advanced mode regardless of packet type;
           these 3 registers do not affect legacy mode*/
        value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_COR1 + offset));         
        value ^= 0x00000100; 
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR1 + offset), value);         
        
        value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_COR4 + offset));         
        value = ~(value); 
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR4 + offset), value);   
       
        value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_COR7 + offset));
        value = ~(value);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR7 + offset), value);         
    }

    return BERR_SUCCESS;
}

#else

/* For 65 nm chips */
BERR_Code BUHF_P_SelectRfChan
(
    BUHF_Handle     hUhfr           /* [in] UHFR Device handle */
)
{
    unsigned int offset=0;
    uint32_t value, ana3_val, ana4_val;
    uint32_t fcw=0, half_shift=0, fcw_int=0;
    uint32_t ndiv_frac=0, ndiv_frac_int=0, ndiv_int=0;
    
    BDBG_ASSERT(hUhfr);
    
    offset = hUhfr->ui32Offset;

    if ((hUhfr->settings.bShiftIf == false) || (hUhfr->settings.uiFreqShift == 0) || (hUhfr->settings.channel == BUHF_ChanNum_eChan9))
    {
        value=ana3_val = ana3Table[hUhfr->settings.injection][hUhfr->settings.channel];
        ana4_val = ana4Table[hUhfr->settings.injection][hUhfr->settings.channel];
        half_shift = 10700;
    }
    else
    {   
        /* All frequency calculations with be in kHz domain */
        ana3_val=0x02000000; 
        ana4_val=0x17b6c007;
        fcw = 0;
       
        if (hUhfr->uiIndex == 0)
        {
            half_shift = 10700 - (hUhfr->settings.uiFreqShift/2); 
            ndiv_int =  ndivInt[0][hUhfr->settings.injection][hUhfr->settings.channel];
        }
        else if (hUhfr->uiIndex ==1)
        {
            half_shift = 10700 + (hUhfr->settings.uiFreqShift/2); 
            ndiv_int =  ndivInt[1][hUhfr->settings.injection][hUhfr->settings.channel];
        }     
        BDBG_MSG(("half_shift=0x%x", half_shift));
        BDBG_MSG(("hUhfr->settings.uiFreqShift=0x%x", hUhfr->settings.uiFreqShift));        
        if ((hUhfr->uiIndex ==1) && (hUhfr->settings.injection == BUHF_InjType_eLow) &&
            (hUhfr->settings.channel==BUHF_ChanNum_eChan3) && (hUhfr->settings.uiFreqShift > 200))
        {
            ndiv_int = 0x01A;
        }     

        if (hUhfr->settings.injection == BUHF_InjType_eLow) 
        {     
            ndiv_frac_int = ((channelFreq[hUhfr->settings.channel]-half_shift)<<2) - (ndiv_int*54000);
        }
        else if (hUhfr->settings.injection == BUHF_InjType_eHigh) 
        {
            ndiv_frac_int = ((channelFreq[hUhfr->settings.channel]+half_shift)<<2) - (ndiv_int*54000);
        }
        ndiv_frac = (ndiv_frac_int * 19884 + (ndiv_frac_int >> 3) - (ndiv_frac_int >> 6) - (ndiv_frac_int >> 9) + 32)  >> 6;

        ana3_val |= (BCHP_FIELD_DATA (UHFR_1_ANA3, PLL_NDIV_FRAC, (uint32_t)ndiv_frac));
        ana4_val |= (BCHP_FIELD_DATA (UHFR_1_ANA4, PLL_NDIV_INT, (uint32_t)ndiv_int));

        BDBG_MSG(("ndiv_int=0x%x", ndiv_int)); 
        BDBG_MSG(("ana4_val=0x%x", ana4_val));
        BDBG_MSG(("ndiv_frac=0x%x", ndiv_frac));       
        BDBG_MSG(("ana3_val=0x%x", ana3_val));     
        BDBG_MSG(("fcw=0x%x", fcw));
        
    }

    fcw_int = (half_shift * 212097 + (half_shift >> 3) + (half_shift >> 6) + 512) >> 10;    /* Fsample = 81MHz */
#if ((BUHF_7420_FAMILY == 1))
        fcw = (fcw_int * 3) >> 1;   /* Fsample = 54MHz */
#else
        fcw = fcw_int;              /* Fsample = 81MHz */
#endif
    if (hUhfr->settings.channel == BUHF_ChanNum_eChan9) {
      fcw = ~fcw + 1;
    }
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_FCW + offset), fcw);
    
    value = ana3_val;

    /* assert both areset and dreset */
    value &= ~(BCHP_MASK (UHFR_1_ANA3, PLL_ARESET));
    value |= (BCHP_FIELD_DATA (UHFR_1_ANA3, PLL_ARESET, 1));
    value &= ~(BCHP_MASK (UHFR_1_ANA3, PLL_DRESET));
    value |= (BCHP_FIELD_DATA (UHFR_1_ANA3, PLL_DRESET, 1));    
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA3 + offset), value);       
    BDBG_MSG(("BCHP_UHFR_1_ANA3=0x%08x", value));    

    if (hUhfr->settings.channel == BUHF_ChanNum_eChan9) /*change for the cust2 pll setting*/
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA1 + offset), BUHF_ANA1_VALUE_CH9);  
        BDBG_ERR(("BCHP_UHFR_1_ANA1=0x%08x", BUHF_ANA1_VALUE_CH9));    
    }
    else
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA1 + offset), BUHF_ANA1_VALUE);
        BDBG_MSG(("BCHP_UHFR_1_ANA1=0x%08x", BUHF_ANA1_VALUE));
        /*  When you select uhfr_2, you also need to write to uhfr_1.ANA0, uhfr_1.ANA4, and uhfr_1.ANA5.*/
    }
    if (hUhfr->uiIndex ==0)
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA4), ana4_val);       
        BDBG_MSG(("BCHP_UHFR_1_ANA4=0x%08x", ana4_val));        
    }
    else
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA4 + offset), ana4_val);  
        BDBG_MSG(("BCHP_UHFR_2_ANA4=0x%08x", ana4_val));  

        /* read-modify-write device 0 ana4. */
        value=BREG_Read32(hUhfr->hRegister, BCHP_UHFR_1_ANA4);
        BDBG_MSG(("BCHP_UHFR_1_ANA4 was =0x%08x", value)); 
        
        /* write to only the shared bits of BCHP_UHFR_1_ANA4 ie ANA4[15], ANA4[3:0]*/
        value &= ~(BCHP_MASK (UHFR_1_ANA4, REGCTRL_25)); /* ANA4[3:0] */
        value |= (BCHP_FIELD_DATA (UHFR_1_ANA4, REGCTRL_25, 7));
        value &= ~(BCHP_MASK (UHFR_1_ANA4, RXCLKSEL)); /* ANA4[15] */
        value |= (BCHP_FIELD_DATA (UHFR_1_ANA4, RXCLKSEL, 1));        
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA4 ), value);      
        BDBG_MSG(("BCHP_UHFR_1_ANA4 modified to = 0x%08x", value));         
    }
    
    value = 0x00000001; /* start serial shift to analog */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANACTL + offset), value);           
    BKNI_Delay(20); /* TODO: get correct value */

    value = ana3_val & ~(BCHP_MASK (UHFR_1_ANA3, PLL_DRESET));   
    value |= (BCHP_FIELD_DATA (UHFR_1_ANA3, PLL_DRESET, 1)); 

    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA3 + offset), value);           
    value = 0x00000001; /* start serial shift to analog */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANACTL + offset), value);         
    BKNI_Delay(20); /* TODO: get correct value */
    

    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA3 + offset), ana3_val);           
    value = 0x00000001; /* start serial shift to analog */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANACTL + offset), value);         
    BKNI_Delay(20); /* TODO: get correct value */

    if (hUhfr->settings.channel == BUHF_ChanNum_eChan9) /*change for Cust2 pll setting*/
    {
        value=BUHF_ANA1_VALUE_CH9 & 0xffbfffff; /* enable pll clkout */
    }
    else
    {
        value=BUHF_ANA1_VALUE & 0xffbfffff; /* enable pll clkout */
    }
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANA1 + offset), value);       
    value = 0x00000001; /* start serial shift to analog */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANACTL + offset), value);         

    if (hUhfr->uiIndex == 1)
    {
        /* write to UHFR_1.ANACTL only perform when device 1 is programmed
        but not device 0*/
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_ANACTL), value);   
    }
    BKNI_Delay(20); /* TODO: get correct value */

    if (hUhfr->settings.injection == BUHF_InjType_eHigh) 
    {
        if (hUhfr->settings.mode == BUHF_Mode_eLegacy) /* legacy mode regardless of Cust1 or Cust2 */
        {   
            value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_FCW + offset));
            value = ~(value) + 1; 
            BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_FCW + offset), value);
        }
        /* COR1, COR4, and COR7 are changed in advanced mode regardless of packet type;
           these 3 registers do not affect legacy mode*/
        value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_COR1 + offset));         
        value ^= 0x00000100; 
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR1 + offset), value);         
        
        value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_COR4 + offset));         
        value = ~(value); 
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR4 + offset), value);   
       
        value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_1_COR7 + offset));
        value = ~(value);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_1_COR7 + offset), value);         
    }

    return BERR_SUCCESS;
}


#endif

BERR_Code BUHF_GetDefaultStandbySettings(
    BUHF_Handle hUhf,
    BUHF_StandbySettings *pSettings
    )
{
    BDBG_ASSERT(hUhf);
    BDBG_ASSERT(pSettings);
    if(hUhf->settings.channel == BUHF_ChanNum_eChan9)
    {
        *pSettings = defStandbySettings;
        pSettings->ui32KeyPatternValue = 0x20D01068;
    }
    else
    {
        *pSettings = defStandbySettings;
    }
    return BERR_SUCCESS;
}

BERR_Code BUHF_Standby(
    BUHF_Handle hUhfr,
    const BUHF_StandbySettings *pSettings
    )
{
    BERR_Code   ret = BERR_SUCCESS;
    uint32_t    ui32RegVal;
    BSTD_UNUSED(ui32RegVal);

    BDBG_ENTER(BUHF_Standby);
    
    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(pSettings);
#if (BUHF_7420_FAMILY ==0)
    BSTD_UNUSED(hUhfr);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Standby feature not supported for this chip"));
    ret =BERR_NOT_SUPPORTED;
    goto end;
#else
    if(hUhfr->bStandbyState == true)
    {
        BDBG_WRN(("Already in Standby State. Doing nothing"));
        ret = BERR_SUCCESS;
        goto end;
    }
#if 0
    if((pSettings->bHwKeyFiltering == true)
        &&(pSettings->eKeyFilter >= BUHF_KeyFilter_eMax))
    {
        BDBG_ERR(("Not a valid Key Filter (%d) when Hw filtering is enabled",pSettings->eKeyFilter));
        ret = BERR_INVALID_PARAMETER;
        goto end;
    }
#endif

    if (!pSettings->bEnableWakeup) {
        /* power down everything */
        hUhfr->bFullPowerdown = true;       
#ifdef BCHP_PWR_RESOURCE_UHF_INPUT
        BCHP_PWR_ReleaseResource(hUhfr->hChip, BCHP_PWR_RESOURCE_UHF_INPUT);
#endif
    }
    else {
        /* otherwise, we must leave everything running, but lower the power duty cycle */
        hUhfr->bFullPowerdown = false;

        /* Enable automatic analog power down and long duty cycle power down for standby with wakeup 
           Note, this is not a separate BCHP_PWR node because analog power is still on, but managed automatically by the HW */
        ui32RegVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD1));
        ui32RegVal &= ~( (BCHP_MASK (UHFR_GLBL_AUTOPD1, AUTO_PD_EN)) 
                       | (BCHP_MASK (UHFR_GLBL_AUTOPD1, LDC_EN_AUTO_PD)));
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_GLBL_AUTOPD1, AUTO_PD_EN, 1));    
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_GLBL_AUTOPD1, LDC_EN_AUTO_PD, 1));
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD1), ui32RegVal );

        if (pSettings->bHwKeyFiltering == true)
        {
            ui32RegVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD5));
            ui32RegVal &= ~( (BCHP_MASK (UHFR_GLBL_AUTOPD5, DIS_PKT_FILTER))
                                        |(BCHP_MASK (UHFR_GLBL_AUTOPD5, EN_REPEAT_EN_INT)));
            ui32RegVal |= (BCHP_FIELD_DATA (UHFR_GLBL_AUTOPD5, DIS_PKT_FILTER, 0));
            ui32RegVal |= (BCHP_FIELD_DATA (UHFR_GLBL_AUTOPD5, EN_REPEAT_EN_INT, 0));
            BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD5), ui32RegVal );

            BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD4), pSettings->ui32KeyPatternValue);

#if 0        
            if(hUhfr->settings.channel == BUHF_ChanNum_eChan9)
            {
                BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD4), ui32KeyFilterPattern_cust2[pSettings->eKeyFilter]);
            }
            else
            {
                BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD4), ui32KeyFilterPattern_cust2[pSettings->eKeyFilter]);
            }            
#endif        
        }
        else
        {
            ui32RegVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD5));
            ui32RegVal &= ~( (BCHP_MASK (UHFR_GLBL_AUTOPD5, DIS_PKT_FILTER))
                            |(BCHP_MASK (UHFR_GLBL_AUTOPD5, EN_REPEAT_EN_INT)));
            ui32RegVal |= (BCHP_FIELD_DATA (UHFR_GLBL_AUTOPD5, DIS_PKT_FILTER, 1));
            ui32RegVal |= (BCHP_FIELD_DATA (UHFR_GLBL_AUTOPD5, EN_REPEAT_EN_INT, 0));
            BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD5), ui32RegVal );   
        }
    }

    hUhfr->bStandbyState = true;
#endif
end:
    BDBG_LEAVE(BUHF_Standby);
    return ret;
}


BERR_Code BUHF_Resume(BUHF_Handle hUhfr)
{
    BERR_Code   ret = BERR_SUCCESS;
    uint32_t    ui32RegVal;
    BSTD_UNUSED(ui32RegVal);
    BDBG_ENTER(BUHF_Resume);
    
    BDBG_ASSERT(hUhfr);
#if (BUHF_7420_FAMILY ==0)
    BSTD_UNUSED(hUhfr);
    BDBG_WRN(("Resume feature not supported for this chip"));
    ret =BERR_NOT_SUPPORTED;
    goto end;
#else
    if(hUhfr->bStandbyState == false)
    {
        BDBG_WRN(("Already in Resume State. Doing nothing"));
        ret = BERR_SUCCESS;
        goto end;
    }

    if (hUhfr->bFullPowerdown) {
        /* power up everything */
        hUhfr->bFullPowerdown = false;
#ifdef BCHP_PWR_RESOURCE_UHF_INPUT
        BCHP_PWR_AcquireResource(hUhfr->hChip, BCHP_PWR_RESOURCE_UHF_INPUT);
#endif
    }
    else {
        /* otherwise, just turn off automatic analog power down, which restores full power state */
        ui32RegVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD1));
        ui32RegVal &= ~( (BCHP_MASK (UHFR_GLBL_AUTOPD1, AUTO_PD_EN)) 
                       | (BCHP_MASK (UHFR_GLBL_AUTOPD1, LDC_EN_AUTO_PD)));
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_GLBL_AUTOPD1, AUTO_PD_EN, 0));
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_GLBL_AUTOPD1, LDC_EN_AUTO_PD, 0));
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD1), ui32RegVal );

        ui32RegVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD5));
        ui32RegVal &= ~( (BCHP_MASK (UHFR_GLBL_AUTOPD5, DIS_PKT_FILTER))
                            |(BCHP_MASK (UHFR_GLBL_AUTOPD5, EN_REPEAT_EN_INT)));
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_GLBL_AUTOPD5, DIS_PKT_FILTER, 1));
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_GLBL_AUTOPD5, EN_REPEAT_EN_INT, 1));        
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_GLBL_AUTOPD5), ui32RegVal );
    }

    hUhfr->bStandbyState = false;
#endif
end:
    BDBG_LEAVE(BUHF_Resume);
    return ret;
}

