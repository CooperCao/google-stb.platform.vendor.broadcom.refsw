/***************************************************************************
 *     Copyright (c) 2005-2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * OTP CMD layer abstraction.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * Broadcom Security Processor Code
 ***************************************************************************/
 
#ifndef BSP_S_OTP_COMMON_H__
#define BSP_S_OTP_COMMON_H__

typedef enum BCMD_Otp_CmdMsp_e      
{

    BCMD_Otp_CmdMsp_eReserved0           = 0,
    BCMD_Otp_CmdMsp_eReserved1           = 1,
    
    BCMD_Otp_CmdMsp_eBackgroundCheckEn   = 2,
    BCMD_Otp_CmdMsp_eFlashRegion0_2      = 3,  /* 3 bits */
    BCMD_Otp_CmdMsp_eBootMemChkEn        = 4,
    BCMD_Otp_CmdMsp_eExportCntrlEnable   = 5,
    BCMD_Otp_CmdMsp_ePubKeyIndx0_3       = 6,  /* 4 bits */
    
    BCMD_Otp_CmdMsp_eStopB               = 7,
    BCMD_Otp_CmdMsp_eStartB              = 8,
    BCMD_Otp_CmdMsp_eStopA               = 9,
    BCMD_Otp_CmdMsp_eStartA              = 10,
    
    BCMD_Otp_CmdMsp_eReserved11          = 11,
    BCMD_Otp_CmdMsp_eReserved12          = 12,
    
    BCMD_Otp_CmdMsp_eLockByte0_7         = 13,  /* 8 bits */
    
    BCMD_Otp_CmdMsp_eLockBitC            = 14,
    BCMD_Otp_CmdMsp_eCrOtpBit0           = 15,
    BCMD_Otp_CmdMsp_eCrOtpBit1           = 16,
    BCMD_Otp_CmdMsp_ePciHostProtectWithCR = 17,
    BCMD_Otp_CmdMsp_ePciClientProtectWithCR = 18,
    BCMD_Otp_CmdMsp_eTestPortProtectWithCR = 19,
    BCMD_Otp_CmdMsp_eForceDramScrambler  = 20,
    
    BCMD_Otp_CmdMsp_eReserved21          = 21,
    BCMD_Otp_CmdMsp_eReserved22          = 22,
    BCMD_Otp_CmdMsp_eReserved23          = 23,
    BCMD_Otp_CmdMsp_eReserved24          = 24,

    BCMD_Otp_CmdMsp_eForceSha256         = 25,
    BCMD_Otp_CmdMsp_eOtpProductId0_3     = 26,  /* 4 bits*/
    
    BCMD_Otp_CmdMsp_eOtpCtrlBit0_30      = 27,  /* 31 bits*/
    
    BCMD_Otp_CmdMsp_eReserved28          = 28,
    BCMD_Otp_CmdMsp_eReserved29          = 29, /* 4 bits */
    BCMD_Otp_CmdMsp_eReserved30          = 30, /* 4 bits */

    BCMD_Otp_CmdMsp_eReserved31          = 31, /* 32 bits */
    
    BCMD_Otp_CmdMsp_eReserved32          = 32, /* 32 bits */
    
    BCMD_Otp_CmdMsp_eReserved33          = 33,
    BCMD_Otp_CmdMsp_eReserved34          = 34,

    BCMD_Otp_CmdMsp_eReserved35          = 35,

    BCMD_Otp_CmdMsp_eOtpCtrlBitSet2      = 36, /* 12 bits */

    BCMD_Otp_CmdMsp_eReserved37          = 37,
    BCMD_Otp_CmdMsp_eReserved38 		 = 38, /* 4 bits */ 
   
    BCMD_Otp_CmdMsp_eOtpReserved39       = 39, 
   
    BCMD_Otp_CmdMsp_eReserved40          = 40,

    BCMD_Otp_CmdMsp_eOtpCtrlBit44_eDisableCaKeyLadder = 41, /* 1 bit */

    BCMD_Otp_CmdMsp_eReserved42          = 42,

    BCMD_Otp_CmdMsp_eReserved43          = 43, /* 1 bit */

    BCMD_Otp_CmdMsp_eRaveVerifyEnable    = 44, /* 1 bit */
    BCMD_Otp_CmdMsp_eReserved44          = 44,
    BCMD_Otp_CmdMsp_eReserved45 		 = 45, /* 1 bits */ 


    BCMD_Otp_CmdMsp_eDisableMICH         = 46, /* 1 bit */
    
    BCMD_Otp_CmdMsp_eReserved47          = 47, /* 1 bit */
    BCMD_Otp_CmdMsp_eReserved48          = 48, /* 1 bit */
    BCMD_Otp_CmdMsp_eReserved49          = 49, /* 1 bit */
    BCMD_Otp_CmdMsp_eReserved50 		 = 50, /* 1 bits */ 
    
    BCMD_Otp_CmdMsp_eM2mWmdrmPd          = 51, /* 1 bit */
    BCMD_Otp_CmdMsp_eReserved52          = 52, /* 1 bit */
    BCMD_Otp_CmdMsp_eOtpCtrlBitSet3      = 53, /* 10 bits */
    
    BCMD_Otp_CmdMsp_eFlashPublicKeyAuthenticate = 54, /* 54 - 1 bit */
    BCMD_Otp_CmdMsp_eReserved55          = 55, /* 55 - 1 bit */
    
#ifndef BSP_LOTP
    BCMD_Otp_CmdMsp_eHashCodeDigest0     = 56, /* 56 - 32 bits */
    BCMD_Otp_CmdMsp_eHashCodeDigest1     = 57, /* 57 - 32 bits */
    BCMD_Otp_CmdMsp_eHashCodeDigest2     = 58, /* 58 - 32 bits */
    BCMD_Otp_CmdMsp_eHashCodeDigest3     = 59, /* 59 - 32 bits */
    BCMD_Otp_CmdMsp_eHashCodeDigest4     = 60, /* 60 - 32 bits */
#else
    BCMD_Otp_CmdMsp_eReserved56          = 56, /* 56 - 32 bits */
    BCMD_Otp_CmdMsp_eReserved57          = 57, /* 57 - 32 bits */
    BCMD_Otp_CmdMsp_eReserved58          = 58, /* 58 - 32 bits */
    BCMD_Otp_CmdMsp_eReserved59          = 59, /* 59 - 32 bits */
    BCMD_Otp_CmdMsp_eReserved60          = 60, /* 60 - 32 bits */
#endif
    BCMD_Otp_CmdMsp_eReserved61          = 61, /* 61 - 1 bit */

    BCMD_Otp_CmdMsp_eOtpFullRomDisable   = 62, /* 62 - 1 bit */
    BCMD_Otp_CmdMsp_eOtpBootRomCrcEn     = 63, /* 63 - 1 bit */
    BCMD_Otp_CmdMsp_eOtpBspCodeBckgrdChkEnable = 64, /* 64 - 1 bit */
    BCMD_Otp_CmdMsp_eAesprime_KeySelect  = 65, /* 65 - 6 bits */
    BCMD_Otp_CmdMsp_eOtpSecurityMode     = 66, /* 66 - 5 bits */
    BCMD_Otp_CmdMsp_eReserved67          = 67, /* 67 - 2 bits */
    BCMD_Otp_CmdMsp_eMarketId            = 68, /* 68 - 32 bits */
    BCMD_Otp_CmdMsp_eCurrentEpoch        = 69, /* 69 - 8 bits */
    BCMD_Otp_CmdMsp_eKey0PrimeSigningRules = 70, /* 70 - 4 bits */
    BCMD_Otp_CmdMsp_eKey0SigningRules    = 71, /* 71 - 4 bits */
    BCMD_Otp_CmdMsp_eReserved72 		 = 72, /* 72 - 1 bit */ 
    BCMD_Otp_CmdMsp_eReserved73 		 = 73, /* 73 - 1 bit */ 
    BCMD_Otp_CmdMsp_eExtendedBootProtect = 74, /* 74 - 1 bit */
    BCMD_Otp_CmdMsp_eErrorMipsOutResetDisable = 75, /* 75 - 1 bit */
    
    BCMD_Otp_CmdMsp_eGenericPinBoundaryScanProtect = 76, /* 76 - 1 bit */
    BCMD_Otp_CmdMsp_eReserved77 		 = 77, /* 77 - 1 bit */ 
     
    BCMD_Otp_CmdMsp_eMichEnable          = 78, /* 78 - 1 bit */
    BCMD_Otp_CmdMsp_eVideoVerifyEnable   = 79, /* 79 - 1 bit */
    BCMD_Otp_CmdMsp_eAudioVerifyEnable   = 80, /* 80 - 1 bit */
    
	BCMD_Otp_CmdMsp_eReserved81          = 81, /* 81 - 2 bits */
	BCMD_Otp_CmdMsp_eReserved82          = 82, /* 82 - 1 bit */
	BCMD_Otp_CmdMsp_eReserved83          = 83, /* 83 - 1 bit */
	BCMD_Otp_CmdMsp_eReserved84          = 84, /* 84 - 2 bits */
    BCMD_Otp_CmdMsp_eReserved85          = 85, /* 85 - 1 bit */
	BCMD_Otp_CmdMsp_eSpdifI2SDisable     = 86, /* 86 - 1 bit */
    
	BCMD_Otp_CmdMsp_eOtpProgRights       = 87, /* 87 - 2 bits */
    BCMD_Otp_CmdMsp_eConformanceDisable  = 88, /* 88 - 1 bit */
    BCMD_Otp_CmdMsp_eDesParityCheckEn    = 89, /* 89 - 1 bit */
    BCMD_Otp_CmdMsp_eOtpKeyDeobfuscationEn = 90, /* 90 - 1 bit */
    BCMD_Otp_CmdMsp_eOtpObfuscatedKeyHashReadDisable = 91, /* 91 - 1 bit */
    BCMD_Otp_CmdMsp_eReserved92         = 92, /* 92 - 1 bit */
	BCMD_Otp_CmdMsp_eForce_3DES_Key_Ladder = 93, /* 93 - 1 bit */

#ifdef BSP_LOTP
    BCMD_Otp_CmdMsp_eStartHash           = 94,  /* 94 - 1 bit */
    BCMD_Otp_CmdMsp_eStopHash            = 95,  /* 95 - 1 bit */
#else
    BCMD_Otp_CmdMsp_eReserved94          = 94,  /* 94 - 1 bit */
    BCMD_Otp_CmdMsp_eReserved95          = 95,  /* 95 - 1 bit */
#endif

    BCMD_Otp_CmdMsp_eReserved96 		 = 96,  /* 96 - 2 bits */ 
    BCMD_Otp_CmdMsp_eReserved97 		 = 97,  /* 97 - 1 bit */ 
    BCMD_Otp_CmdMsp_eEnableASKM          = 98,  /* 98 - 1 bit */
    BCMD_Otp_CmdMsp_eOTPSecurePSEQ       = 99,  /* 99 - 1 bit */
    BCMD_Otp_CmdMsp_eOTPChipPwrMgmtDisable = 100, /* 100 - 1 bit */
    BCMD_Otp_CmdMsp_ePCIeHostProtectCR   = 101, /* 101 - 1 bit */
    BCMD_Otp_CmdMsp_ePCIeClientProtectCR = 102, /* 102 - 1 bit */
    BCMD_Otp_CmdMsp_eReserved103 		 = 103,	/* 103 - 1 bit */ 
    BCMD_Otp_CmdMsp_eReserved104 		 = 104,	/* 104 - 1 bit */ 
    BCMD_Otp_CmdMsp_eMOCAScbDisable      = 105, /* 105 - 1 bit */
    BCMD_Otp_CmdMsp_eCpKeyLadderDisable  = 106, /* 106 - 1 bit */
    BCMD_Otp_CmdMsp_ePCIUnsecuredModeDisable = 107, /* 107 - 1 bit */
    BCMD_Otp_CmdMsp_eAskmStbOwnerId      = 108, /* 108 - 16 bit */
    BCMD_Otp_CmdMsp_eBseckFwEpoch        = 109, /* 109 - 8 bits */
    BCMD_Otp_CmdMsp_eReserved110 		      = 110, /* 110 - 1 bit */ 

    BCMD_Otp_CmdMsp_eDataSectionReadProtectBits = 111, /* 111 - 4 bits */
    BCMD_Otp_CmdMsp_eOtpCtrlBit31_eDisableCustKeyToCaKeyLadder = 112, /* 112 - 1 bit */    
    BCMD_Otp_CmdMsp_eFlashType           = 113, /* 3 bits */
    BCMD_Otp_CmdMsp_eMipsBootFromFlash   = 114, /* 1 bit */
    BCMD_Otp_CmdMsp_eUserReg             = 115, /* 32 bits */
    BCMD_Otp_CmdMsp_eBootCodeHideEn      = 116, /* 1 bit */
    BCMD_Otp_CmdMsp_eFinal3DesRouteKeyK1K2Check  = 117, /*1 bit */ 
    BCMD_Otp_CmdMsp_eReserved118 		      = 118, /*1 bit */ 
          
    BCMD_Otp_CmdMsp_eBcmBootSequence2    = 119, /* 1 bit */
    BCMD_Otp_CmdMsp_eOtpCtrlBit30_eDisableCustKeyToCpKeyLadder = 120, /* 1 bit */    
    BCMD_Otp_CmdMsp_eAesKeySelectForBootCodeDecrypt = 121, /* 4 bits */
    BCMD_Otp_CmdMsp_eDHRootKeyDisable    = 122, /* 1 bit */
    BCMD_Otp_CmdMsp_eDHGyMustSign        = 123, /* 1 bit */
    BCMD_Otp_CmdMsp_eCaRsaRootDisable    = 124, /* 1 bit */
    BCMD_Otp_CmdMsp_eCpRsaRootDisable    = 125, /* 1 bit */
    BCMD_Otp_CmdMsp_eEncKey3MustSign     = 126, /* 1 bit */
    BCMD_Otp_CmdMsp_eReserved127 		 = 127, /*1 bit */ 
    BCMD_Otp_CmdMsp_eReserved128 		 = 128, /*1 bit */ 

    BCMD_Otp_CmdMsp_eNandConfigViaOtp    = 129, /* 1 bit */
    BCMD_Otp_CmdMsp_eNandFlashDeviceSize = 130, /* 1 bit */
    BCMD_Otp_CmdMsp_eNandFlashBlockSize  = 131, /* 1 bit */
    BCMD_Otp_CmdMsp_eNandFlashPageSize   = 132, /* 1 bit */
    BCMD_Otp_CmdMsp_eIpLicensingCheckEnable = 133, /* 1 bit */ 
    BCMD_Otp_CmdMsp_eIpLicensingRightsClass = 134, /* 4 bits */ 
    BCMD_Otp_CmdMsp_eSize		         = 135 /* Max */	
} BCMD_Otp_CmdMsp_e ;
    
#endif /* BSP_S_OTP_COMMON_H__ */
