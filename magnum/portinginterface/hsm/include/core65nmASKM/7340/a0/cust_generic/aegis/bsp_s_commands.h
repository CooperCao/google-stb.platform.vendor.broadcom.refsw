/***************************************************************************
 *     Copyright (c) 2005-2009, Broadcom Corporation
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
 * Security Processor Code for Register Access
 ***************************************************************************/

#ifndef BSP_S_COMMANDS_H__
#define BSP_S_COMMANDS_H__


/* Commands accepted by AMCM */
typedef enum BCMD_cmdType_e
{
    /* command 0x00 */
    BCMD_cmdType_eMODE_CONFIG = 0x0,
    BCMD_cmdType_eSESSION_INIT_KEYSLOT = 0x1,
    BCMD_cmdType_eSESSION_CONFIG_PIDKEYPOINTERTABLE =0x2,
    BCMD_cmdType_eSESSION_CONFIG_ALGORITHM =0x3,
    
    /* command 0x04 */
    BCMD_cmdType_eSESSION_LOAD_ROUTE_USERKEY=0x4,
    BCMD_cmdType_eSESSION_INVALIDATE_KEY=0x5,
    BCMD_cmdType_eRESERVED_6=0x6,
    BCMD_cmdType_eSESSION_GENERATE_ROUTE_KEY = 0x7,
    
    /* command 0x08 */
    BCMD_cmdType_eSESSION_BRCM_CANCEL_COMMAND= 0x8,
    BCMD_cmdType_eSESSION_MEM_AUTH= 0x9,
    BCMD_cmdType_eRESERVED_10= 0xa,
    BCMD_cmdType_eRESERVED_11 = 0x0b,
    
    /* command 0x0c */
    BCMD_cmdType_eSESSION_PROC_OUT_CMD = 0x0c,
    BCMD_cmdType_eUSER_RANDOM_NUMBER= 0x0d,
    BCMD_cmdType_eUSER_RSA= 0x0e,
    BCMD_cmdType_eUSER_DSA= 0x0f,
    
    /* command 0x10 */
    BCMD_cmdType_eUSER_DH= 0x10,
    BCMD_cmdType_eRESERVED_17 = 0x11,
    BCMD_cmdType_eUSER_SHA1= 0x12,
    BCMD_cmdType_eUSER_TIMER= 0x13,
    
    /* command 0x14 */
    BCMD_cmdType_eUSER_GET_OTP_ID_CHALLENGE= 0x14,
    BCMD_cmdType_eUSER_SEND_AUTH_DATA= 0x15,
    BCMD_cmdType_eRESERVED_22= 0x16,   
 
    BCMD_cmdType_eOFFLINE_STATUS = 0x17,
    /* command 0x18 */
    BCMD_cmdType_eRESERVED_24     = 0x18,	

    BCMD_cmdType_eOFFLINE_OTP_READ= 0x19,
    BCMD_cmdType_eOFFLINE_PROG_MSP= 0x1a,
    BCMD_cmdType_eOFFLINE_MSP_READ= 0x1b,
    
    /* command 0x1c */
    BCMD_cmdType_eUSER_ENABLE_SDRAM_ENCRYPTION= 0x1c,
    BCMD_cmdType_eCONFIG_MULTI2= 0x1d,
    BCMD_cmdType_eOTP_DATA_SECTION_READ= 0x1e,
    BCMD_cmdType_eOTP_DATA_SECTION_PROG= 0x1f,    
    /* command 0x20 */
    BCMD_cmdType_eRESERVED_32	      = 0x20,	
    BCMD_cmdType_eUSER_SC_ARRAY_STATUS= 0x21,
    BCMD_cmdType_eOFFLINE_RAVE_COMMAND= 0x22,
    BCMD_cmdType_eRESERVED_35= 0x23,
    BCMD_cmdType_eRESERVED_36= 0x24,
    BCMD_cmdType_eOFFLINE_ARCH_COMMAND = 0x25,
    BCMD_cmdType_eRESERVED_38= 0x26,
    BCMD_cmdType_eRESERVED_39= 0x27,    

   /* command 0x28 */
   BCMD_cmdType_eRESERVED_40 = 0x28,  

    BCMD_cmdType_eRESERVED_41 = 0x29,

   BCMD_cmdType_eDEBUG_DUMP_EXC = 0x2a,
   BCMD_cmdType_eOFFLINE_SET_VICH_REG_PAR = 0x2b,
#if defined (DOCSISMIPS)
   BCMD_cmdType_eDOCSIS_MIPS_BOOT = 0x2c,
#else
  BCMD_cmdType_eRESERVED_44 = 0x2c,  
#endif
  BCMD_cmdType_eOFFLINE_SetPCIMemWin = 0x2d,
  BCMD_cmdType_eREMUX_FORCE_ENCRYPTION      = 0x2e,
  BCMD_cmdType_eUNWRAP_CONTENT_KEYS         = 0x2f,
  BCMD_cmdType_eCALC_MEDIA_KEY              = 0x30,
  BCMD_cmdType_eCALC_SEQUENCE_KEY           = 0x31,
  BCMD_cmdType_eLOAD_SECRET_KEY             = 0x32,
  BCMD_cmdType_eLOAD_KEYS_AFTER_OPERATION   = 0x33,
  BCMD_cmdType_eUPDATE_SECRET_KEYS          = 0x34,
  BCMD_cmdType_eSTART_AVD = 0x35,

    BCMD_cmdType_eRESERVED_54 = 0x36,
    BCMD_cmdType_eRESERVED_55 = 0x37,

    BCMD_cmdType_eRESERVED_56 = 0x38,
    BCMD_cmdType_eRESERVED_57 = 0x39,
    BCMD_cmdType_eRESERVED_58 = 0x3a,
    BCMD_cmdType_eRESERVED_59 = 0x3b,
 #if  (defined(BSP_DCAS)) 
  	BCMD_cmdType_eDCAS_INIT_SECURITY_PACKAGE_DLDU   = 0x3c, 
  	BCMD_cmdType_eDCAS_CW_SECURITY_PACKAGE_DLDU     = 0x3d, 
  	BCMD_cmdType_eDCAS_ASD_SECURITY_PACKAGE_DLDU    = 0x3e, 
  	BCMD_cmdType_eDCAS_INSTALL_TP_DATA_DLDU         = 0x3f, 
  	BCMD_cmdType_eDCAS_SECURE_TP_INFO_REQUEST_DLDU  = 0x40, 
  	BCMD_cmdType_eDCAS_RESERVED_65      = 0x41,
 #endif 
  	BCMD_cmdType_eDCAS_RESERVED_60      = 0x3c, 
  	BCMD_cmdType_eDCAS_RESERVED_61      = 0x3d, 
  	BCMD_cmdType_eDCAS_RESERVED_62      = 0x3e, 
  	BCMD_cmdType_eDCAS_RESERVED_63      = 0x3f,
  	BCMD_cmdType_eDCAS_RESERVED_64      = 0x40,
  	BCMD_cmdType_eDCAS_RESERVED_65      = 0x41,

#ifdef BSP_MSIPTV
  	/* MSIPTV commands */
  	BCMD_cmdType_eMSIPTV_GENERATE_SESSION_KEY   = 0x42,
  	BCMD_cmdType_eMSIPTV_GENERATE_BOUNDARY_KEY  = 0x43,
  	BCMD_cmdType_eMSIPTV_DECRYPT_AV_PACKET	    = 0x44,
  	BCMD_cmdType_eMSIPTV_USER_AES_OMAC1	= 0x45,
  	BCMD_cmdType_eMSIPTV_RSA  		= 0x46,
  	BCMD_cmdType_eMSIPTV_INIT		= 0x47,
  	BCMD_cmdType_eMSIPTV_LoadGRCMembToken   = 0x48,
  	BCMD_cmdType_eMSIPTV_RESERVED_67   = 0x43,
  	BCMD_cmdType_eMSIPTV_RESERVED_68   = 0x44,
  	BCMD_cmdType_eMSIPTV_RESERVED_69   = 0x45,
  	BCMD_cmdType_eMSIPTV_RESERVED_70   = 0x46,
  	BCMD_cmdType_eMSIPTV_RESERVED_71   = 0x47,
  	BCMD_cmdType_eMSIPTV_RESERVED_72   = 0x48,
  	BCMD_cmdType_eMSIPTV_RESERVED_73   = 0x49,
  	BCMD_cmdType_eMSIPTV_RESERVED_74   = 0x4a,
#else /* BSP_MSIPTV */
  	BCMD_cmdType_eMSIPTV_RESERVED_66   = 0x42,
  	BCMD_cmdType_eMSIPTV_RESERVED_67   = 0x43,
  	BCMD_cmdType_eMSIPTV_RESERVED_68   = 0x44,
  	BCMD_cmdType_eMSIPTV_RESERVED_69   = 0x45,
  	BCMD_cmdType_eMSIPTV_RESERVED_70   = 0x46,
  	BCMD_cmdType_eMSIPTV_RESERVED_71   = 0x47,
  	BCMD_cmdType_eMSIPTV_RESERVED_72   = 0x48,
  	BCMD_cmdType_eMSIPTV_RESERVED_73   = 0x49,
  	BCMD_cmdType_eMSIPTV_RESERVED_74   = 0x4a,
#endif /* BSP_MSIPTV */

#ifdef BSP_PKE_POLLING_CMD
  	BCMD_cmdType_ePKE_Cmd_Poll_Status   = 0x4b,
#else
  	BCMD_cmdType_eMSIPTV_RESERVED_75   = 0x4b,
#endif
  	BCMD_cmdType_ROUTE_OTP_DATA_SECTION_KEY   = 0x4c,

    BCMD_cmdType_eRESERVED_77 = 0x4d, /* BCMD_cmdType_ePCI_GISB_LOCKDOWN */

    BCMD_cmdType_eRESERVED_78 = 0x4e, 
#if defined (POWER_MGMT)  
    BCMD_cmdType_ePowerMgmtOp = 0x4f, /* BCMD_cmdType_ePowerMgmtOp = 0x4f */
#else
    BCMD_cmdType_eRESERVED_79 = 0x4f, /* 0x4f */
#endif    

#if defined (BSP_OTP_PROG_PASSWD_PROTECT)
    BCMD_cmdType_eOTP_KeyProgPasswdCheck = 0x50,
#else
    BCMD_cmdType_eRESERVED_80 = 0x50, /* 0x50 */
#endif

#if defined (BSP_OTP_PROG_PATTERN_SEQUENCE)
    BCMD_cmdType_eOTP_ProgPatternSequence = 0x51,
#else
    BCMD_cmdType_eRESERVED_81 = 0x51, /* 0x51 */
#endif

    BCMD_cmdType_eRebootWithEJTAGDebug = 0x52,

#if defined(BSP_SECURE_RSA)
  /* Secure RSA commands */
  BCMD_cmdType_eSecureRSA_Init                = 0x53,
  BCMD_cmdType_eSecureRSA_Operation           = 0x54,
  BCMD_cmdType_eSecureRSA_Dh_Operation        = 0x55,
  BCMD_cmdType_eSecureRSA_Dh_GenerateK3       = 0x56,
#else    
    BCMD_cmdType_eRESERVED_83                   = 0x53,
    BCMD_cmdType_eRESERVED_84                   = 0x54,
    BCMD_cmdType_eRESERVED_85                   = 0x55,
    BCMD_cmdType_eRESERVED_86                   = 0x56,
#endif    
#if defined(DRM_DATA_OP_COMMAND)
    BCMD_cmdType_eOTP_DS_Decrypt_DRM_Data_IP_Licensing       = 0x57,
#else  
    BCMD_cmdType_eRESERVED_87                   = 0x57,
#endif


    /*
     * **********************************
     * NOTE: To add one command for either FullROM or BSECK: 
     *
     *      Because FullROM and BSECK use same share file,
     *      BCMD_cmdType_eMAX_COMMAND will use same number, 
     *      whenever a new command added, need to update 
     *      BSECK and FullROM bcmd_command_table.h. If command 
     *      doesnt apply for FullROM, call back table should use 
     *      InvalidCmd for FullROM.
     *
     * **********************************
     */
    /* MAX for production code */
    BCMD_cmdType_eMAX_COMMAND
} BCMD_cmdType_e;

/*

General Command Header - Every command sent from the host contains the
   following 24 bytes header.

Parameters passed:
eVerNum - Version Number of command used by host.
eOwnerId - Id from host to identify output response from Aegis.
eContMode - Indication from host if command requires multiple input
   buffers before returning output.
eTagId - Command to run.
eParamLen - upper 8 bits of command length following Param length field.
ePrarmLenPlus1 - lower 8 bits of command length.
eStatus - used only in output buffer to inidicate status of input command.

BCMD_CommonBufferFields_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0                 | 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eVerNum - 8      | 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eOwnerId - 8     | 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eContMode - 8        | 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eTagId - 8       | 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | eParamLen - 8     | eParamLenPlus1 - 8| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eStatus - 8      | 
|-------------------|-------------------|-------------------|-------------------| 

*/
typedef enum BCMD_CommonBufferFields_e
{
    BCMD_CommonBufferFields_eVerNum             = (0<<2) + 0, /* 32 bit index */
    BCMD_CommonBufferFields_eOwnerId            = (1<<2) + 3, /* 8 bit index */
    BCMD_CommonBufferFields_eContMode           = (2<<2) + 3, /* 8 bit index */
    BCMD_CommonBufferFields_eTagId              = (3<<2) + 3, /* 8 bit index */
    BCMD_CommonBufferFields_eParamLen           = (4<<2) + 2, /* 16 bit index */
    BCMD_CommonBufferFields_eParamLenPlus1      = (4<<2) + 3, 
    BCMD_CommonBufferFields_eParamStart         = (5<<2) + 0, 
    BCMD_CommonBufferFields_eStatus             = (5<<2) + 3 /* 8 bit index */

} BCMD_CommonBufferFields_e;

#if defined(GLITCH_HARDENED_FW_CMD) || defined(GLITCH_HARDENED_FW_MISC)
/* Define new command header with the expanded glitch-hardened fields above */

typedef enum BCMD_CommonBufferFieldsExpanded_e
{
    BCMD_CommonBufferFields_eContModeExpanded   = (2<<2) + 0, /* 32 bit index */
    BCMD_CommonBufferFields_eTagIdExpanded      = (3<<2) + 0, /* 32 bit index */
    BCMD_CommonBufferFields_eParamLenExpanded   = (4<<2) + 0, /* 32 bit index */
} BCMD_CommonBufferFieldsExpanded_e;

#endif /* GLITCH_HARDENED_FW_CMD || GLITCH_HARDENED_FW_MISC */

/* CONT MODES */
typedef enum BCMD_ContMode_e{
    CONT_MODE_ALL_DATA   = 0x00,
    CONT_MODE_MORE_DATA  = 0x01,
    CONT_MODE_LAST_DATA  = 0x10,
    CONT_MODE_FIRST_DATA = 0x11
}BCMD_ContMode_e;


#define BCMD_BUFFER_BYTE_SIZE               384
#define BCMD_BUFFER_WORD_SIZE               BCMD_BUFFER_BYTE_SIZE>>2
#define BCMD_NON_HEADER_BUFFER_BYTE_SIZE    364
#define BCMD_HEADER_BYTE_SIZE               20

/*****************************************************************************
Largest size of the Challange response input is 56 bytes (for response when 
using cust key and key var) and the output buffer is 40 bytes.  So it needs 
96 bytes for the input/output buffer pair.  This will be placed at the 
top of non-secure memory.
 *****************************************************************************/
#define BCMD_CR_IN_BUFFER_BYTE_SIZE  56
#define BCMD_CR_OUT_BUFFER_BYTE_SIZE 40


#if defined(GLITCH_HARDENED_FW_CMD)

/* For glitch hardening, the command id, parameter length and continual mode 
 * command fields will be expanded to include unique constants as follows:
 * |-------------------|-------------------|-------------------|-------------------| 
 * |  31..24           | 23..16            | 15..8             | 7..0              | 
 * |-------------------|-------------------|-------------------|-------------------| 
 * |                  Continual Mode Constant                  | Continual Mode    |
 * |-------------------|-------------------|-------------------|-------------------| 
 * |  ~(Tag Id)        |             Tag Id Constant           |   Tag Id          |
 * |-------------------|-------------------|-------------------|-------------------| 
 * |   Parameter Length Constant           |        Parameter Length               |
 * |-------------------|-------------------|-------------------|-------------------| 
 *
 * These constants are defined below 
 */

#define CONTINUAL_MODE_CONSTANT         0xABCDEF00UL
#define CONTINUAL_MODE_CONSTANT_MASK    0xFFFFFF00UL

#define TAG_ID_CONSTANT                 0x0055AA00UL
#define TAG_ID_CONSTANT_MASK            0x00FFFF00UL

#define PARAMETER_LENGTH_CONSTANT       0x789A0000UL
#define PARAMETER_LENGTH_CONSTANT_MASK  0xFFFF0000UL

#endif /* GLITCH_HARDENED_FW_CMD */


#endif /* BSP_S_COMMANDS_H__ */

