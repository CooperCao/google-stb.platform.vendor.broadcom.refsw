/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/


#ifndef BSP_S_COMMANDS_H__
#define BSP_S_COMMANDS_H__


typedef enum BCMD_cmdType_e
{
    BCMD_cmdType_eRESERVED_0                        = 0x0,
    BCMD_cmdType_eSESSION_INIT_KEYSLOT              = 0x1,
    BCMD_cmdType_eSESSION_CONFIG_PIDKEYPOINTERTABLE = 0x2,
    BCMD_cmdType_eRESERVED_3                        = 0x3,
    BCMD_cmdType_eSESSION_LOAD_ROUTE_USERKEY        = 0x4,
    BCMD_cmdType_eSESSION_INVALIDATE_KEY            = 0x5,
    BCMD_cmdType_eReserved6 = 0x6,
    BCMD_cmdType_eRESERVED_6                        = 0x6,
    BCMD_cmdType_eSESSION_GENERATE_ROUTE_KEY        = 0x7,
    BCMD_cmdType_eReserved8                         = 0x8,


    BCMD_cmdType_eRESERVED_8                        = 0x8,
    BCMD_cmdType_eSESSION_MEM_AUTH                  = 0x9,
    BCMD_cmdType_eReserved10  =  0xa,
    BCMD_cmdType_eRESERVED_10                       = 0xa,
    BCMD_cmdType_eReserved11  =  0x0b,
    BCMD_cmdType_eRESERVED_11                       = 0x0b,
    BCMD_cmdType_eRESERVED_12                       = 0x0c,
    BCMD_cmdType_eUSER_RANDOM_NUMBER                = 0x0d,
    BCMD_cmdType_eUSER_RSA                          = 0x0e,
    BCMD_cmdType_eReserved15  =  0x0f,
    BCMD_cmdType_eReserved16  =  0x10,
    BCMD_cmdType_eRESERVED_15 = 0x0f,
    BCMD_cmdType_eRESERVED_16 = 0x10,
    BCMD_cmdType_eUSER_AES_DES                      = 0x11,
    BCMD_cmdType_eUSER_SHA1                         = 0x12,
    BCMD_cmdType_eReserved19  =  0x13,
    BCMD_cmdType_eRESERVED_19 = 0x13,
    BCMD_cmdType_eReserved20  =  0x14,
    BCMD_cmdType_eReserved21  =  0x15,
    BCMD_cmdType_eRESERVED_20 = 0x14,
    BCMD_cmdType_eRESERVED_21 = 0x15,
    BCMD_cmdType_eReserved22  =  0x16,
    BCMD_cmdType_eRESERVED_22                       = 0x16,
    BCMD_cmdType_eOFFLINE_STATUS                    = 0x17,
    BCMD_cmdType_eReserved24  =  0x18,
    BCMD_cmdType_eRESERVED_24                       = 0x18,
    BCMD_cmdType_eOFFLINE_OTP_READ                  = 0x19,
    BCMD_cmdType_eOFFLINE_PROG_MSP                  = 0x1a,
    BCMD_cmdType_eOFFLINE_MSP_READ                  = 0x1b,
    BCMD_cmdType_eUSER_ENABLE_DISABLE_SDRAM_ENCRYPTION  = 0x1c,
    BCMD_cmdType_eCONFIG_MULTI2                     = 0x1d,
    BCMD_cmdType_eOTP_DATA_SECTION_READ             = 0x1e,
    BCMD_cmdType_eOTP_DATA_SECTION_PROG             = 0x1f,
    BCMD_cmdType_eReserved32  =  0x20,
    BCMD_cmdType_eRESERVED_32 = 0x20,
    BCMD_cmdType_eReserved33  =  0x21,
    BCMD_cmdType_eRESERVED_33                       = 0x21,
    BCMD_cmdType_eOFFLINE_SetMiscBits_COMMAND       = 0x22,
    BCMD_cmdType_eReserved35  =  0x23,
    BCMD_cmdType_eReserved36  =  0x24,
    BCMD_cmdType_eRESERVED_35                       = 0x23,
    BCMD_cmdType_eRESERVED_36                       = 0x24,
    BCMD_cmdType_eOFFLINE_ARCH_COMMAND              = 0x25,
    BCMD_cmdType_eReserved38  =  0x26,
    BCMD_cmdType_eReserved39  =  0x27,
    BCMD_cmdType_eRESERVED_38                       = 0x26,
    BCMD_cmdType_eRESERVED_39                       = 0x27,
    BCMD_cmdType_eReserved40  =  0x28,
    BCMD_cmdType_eRESERVED_40                       = 0x28,
    BCMD_cmdType_eRESERVED_41                       = 0x29,
    BCMD_cmdType_eDEBUG_DUMP_EXC                    = 0x2a,
    BCMD_cmdType_eOFFLINE_SET_VICH_REG_PAR          = 0x2b,
    BCMD_cmdType_eRESERVED_44                       = 0x2c,
    BCMD_cmdType_eReserved45  =  0x2d,
    BCMD_cmdType_eReserved46  =  0x2e,
    BCMD_cmdType_eRESERVED_45                       = 0x2d,
    BCMD_cmdType_eRESERVED_46                       = 0x2e,
    BCMD_cmdType_eRESERVED_47                       = 0x2f,
    BCMD_cmdType_eRESERVED_48                       = 0x30,
    BCMD_cmdType_eRESERVED_49                       = 0x31,
    BCMD_cmdType_eRESERVED_50                       = 0x32,
    BCMD_cmdType_eRESERVED_51  		                = 0x33,
    BCMD_cmdType_eRESERVED_52                       = 0x34,
    BCMD_cmdType_eSTART_AVD                         = 0x35,
    BCMD_cmdType_eSECOND_TIER_KEY_VERIFY            = 0x36,
    BCMD_cmdType_eSECOND_STAGE_CODE_LOAD            = 0x37,
    BCMD_cmdType_eRESERVED_56                       = 0x38,
    BCMD_cmdType_eRESERVED_57                       = 0x39,
    BCMD_cmdType_eRESERVED_58                       = 0x3a,
    BCMD_cmdType_eRESERVED_59                       = 0x3b,
    BCMD_cmdType_eRESERVED_60                       = 0x3c,
    BCMD_cmdType_eRESERVED_61                       = 0x3d,
    BCMD_cmdType_eRESERVED_62                       = 0x3e,
    BCMD_cmdType_eRESERVED_63                       = 0x3f,
    BCMD_cmdType_eRESERVED_64                       = 0x40,
    BCMD_cmdType_eRESERVED_65                       = 0x41,
    BCMD_cmdType_eReserved66  =  0x42,
    BCMD_cmdType_eReserved67  =  0x43,
    BCMD_cmdType_eReserved68  =  0x44,
    BCMD_cmdType_eReserved69  =  0x45,
    BCMD_cmdType_eReserved70  =  0x46,
    BCMD_cmdType_eReserved71  =  0x47,
    BCMD_cmdType_eReserved72  =  0x48,
    BCMD_cmdType_eRESERVED_66                       = 0x42,
    BCMD_cmdType_eRESERVED_67                       = 0x43,
    BCMD_cmdType_eRESERVED_68                       = 0x44,
    BCMD_cmdType_eRESERVED_69                       = 0x45,
    BCMD_cmdType_eRESERVED_70                       = 0x46,
    BCMD_cmdType_eRESERVED_71                       = 0x47,
    BCMD_cmdType_eRESERVED_72                       = 0x48,
    BCMD_cmdType_eRESERVED_73                       = 0x49,
    BCMD_cmdType_eRESERVED_74                       = 0x4a,
    BCMD_cmdType_ePKE_Cmd_Poll_Status               = 0x4b,
    BCMD_cmdType_ROUTE_OTP_DATA_SECTION_KEY         = 0x4c,
    BCMD_cmdType_eRESERVED_77                       = 0x4d,
    BCMD_cmdType_eRESERVED_78                       = 0x4e,
    BCMD_cmdType_ePowerMgmtOp                       = 0x4f,
    BCMD_cmdType_eReserved80  =  0x50,
    BCMD_cmdType_eRESERVED_80                       = 0x50,
    BCMD_cmdType_eOTP_ProgPatternSequence           = 0x51,
    BCMD_cmdType_eRESERVED_82                       = 0x52,
    BCMD_cmdType_eReserved83  =  0x53,
    BCMD_cmdType_eReserved84  =  0x54,
    BCMD_cmdType_eRESERVED_83 = 0x53,
    BCMD_cmdType_eRESERVED_84 = 0x54,
    BCMD_cmdType_eRESERVED_85                       = 0x55,
    BCMD_cmdType_eRESERVED_86                       = 0x56,
    BCMD_cmdType_eReserved87  =  0x57,
    BCMD_cmdType_eRESERVED_87                       = 0x57,
    BCMD_cmdType_eVerifyDramParams                  = 0x58,
    BCMD_cmdType_eReserved89  =  0x59,
    BCMD_cmdType_eReserved90  =  0x5a,
    BCMD_cmdType_eRESERVED_89                       = 0x59,
    BCMD_cmdType_eRESERVED_90                       = 0x5a,
    BCMD_cmdType_eKLADChallengeCmd                  = 0x5b,
    BCMD_cmdType_eKLADResponseCmd                   = 0x5c,
    BCMD_cmdType_eReserved93  =  0x5d,
    BCMD_cmdType_eRESERVED_93                       = 0x5d,
    BCMD_cmdType_eReserved94  =  0x5e,
    BCMD_cmdType_eRESERVED_94 = 0x5e,
	BCMD_cmdType_eRESERVED_95 					    = 0x5f,
    BCMD_cmdType_eReserved96  =  0x60,
    BCMD_cmdType_eRESERVED_96 = 0x60,
    BCMD_cmdType_eAllocateKeySlot                   = 0x61,
    BCMD_cmdType_eReserved98  =  0x62,
    BCMD_cmdType_eRESERVED_98 = 0x62,
    BCMD_cmdType_eOTP_ROLLOVER_COUNTER_Op           = 0x63,
    BCMD_cmdType_eOTP_ReadMSP32                     = 0x64,


    BCMD_cmdType_eMAX_COMMAND
} BCMD_cmdType_e;

typedef enum BCMD_CommonBufferFields_e
{
    BCMD_CommonBufferFields_eVerNum             = (0<<2) + 0,
    BCMD_CommonBufferFields_eChipNum            = (1<<2) + 0,
    BCMD_CommonBufferFields_eChipRev            = (1<<2) + 1,
    BCMD_CommonBufferFields_eVersionScheme      = (1<<2) + 2,
    BCMD_CommonBufferFields_eOwnerId            = (1<<2) + 3,
    BCMD_CommonBufferFields_eContMode           = (2<<2) + 3,
    BCMD_CommonBufferFields_eTagId              = (3<<2) + 3,
    BCMD_CommonBufferFields_eParamLen           = (4<<2) + 2,
    BCMD_CommonBufferFields_eParamLenPlus1      = (4<<2) + 3,
    BCMD_CommonBufferFields_eParamStart         = (5<<2) + 0,
    BCMD_CommonBufferFields_eStatus             = (5<<2) + 3,
    BCMD_CommonBufferFields_eMax

} BCMD_CommonBufferFields_e;


typedef enum BCMD_CommonBufferFieldsExpanded_e
{
    BCMD_CommonBufferFields_eContModeExpanded   = (2<<2) + 0,
    BCMD_CommonBufferFields_eTagIdExpanded      = (3<<2) + 0,
    BCMD_CommonBufferFields_eParamLenExpanded   = (4<<2) + 0,
    BCMD_CommonBufferFieldsExpanded_eMax
} BCMD_CommonBufferFieldsExpanded_e;


typedef enum BCMD_ContMode_e{
    CONT_MODE_ALL_DATA   = 0x00,
    CONT_MODE_MORE_DATA  = 0x01,
    CONT_MODE_LAST_DATA  = 0x10,
    CONT_MODE_FIRST_DATA = 0x11,
    CONT_MODE_eMax
}BCMD_ContMode_e;

#define FEATURE_TABLE_BUFFER (aucGISB_Write_Access_Blocked_Region)
#define FEATURE_TABLE_BYTE_INDEX	(0x13C)
#define IP_LICENSING_BYTE_INDEX  	(0x160)
#define R_REGION_BYTE_INDEX      	(0x174)
#define DRM_KEYS_BUFFER (aucGISB_Write_Access_Blocked_Region)
#define DRM_KEYS_BYTE_INDEX           	(0x00)

#define SHA_1_DIGEST_SIZE_IN_BYTES	   20
#define DRM_KEY_DATA_CLEAR_DRM_DATA_SIZE   304
#define IP_LICENSING_BIT_MAP_SIZE_IN_BYTES 28
#define DRM_KEYS_BUFFER_SIZE 312

#define BCMD_BUFFER_BYTE_SIZE               384
#define BCMD_BUFFER_WORD_SIZE               BCMD_BUFFER_BYTE_SIZE>>2
#define BCMD_NON_HEADER_BUFFER_BYTE_SIZE    364
#define BCMD_HEADER_BYTE_SIZE               20

#define BCMD_CR_IN_BUFFER_BYTE_SIZE  56
#define BCMD_CR_OUT_BUFFER_BYTE_SIZE 40



#define CONTINUAL_MODE_CONSTANT         0xABCDEF00UL
#define CONTINUAL_MODE_CONSTANT_MASK    0xFFFFFF00UL

#define TAG_ID_CONSTANT                 0x0055AA00UL
#define TAG_ID_CONSTANT_MASK            0x00FFFF00UL

#define PARAMETER_LENGTH_CONSTANT       0x789A0000UL
#define PARAMETER_LENGTH_CONSTANT_MASK  0xFFFF0000UL

#define BSP_CMDBUF_BYTE_SIZE
#define BSP_CMDBUF_INPUT1_START_ADDR    (BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE)
#define BSP_CMDBUF_OUTPUT1_START_ADDR   (BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + (BCMD_BUFFER_BYTE_SIZE * 1))
#define BSP_CMDBUF_INPUT2_START_ADDR    (BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + (BCMD_BUFFER_BYTE_SIZE * 2))
#define BSP_CMDBUF_OUTPUT2_START_ADDR   (BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + (BCMD_BUFFER_BYTE_SIZE * 3))

#endif
