/***************************************************************************
 *     Copyright (c) 2004-2013, Broadcom Corporation
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
 * Implementation of the Realtime Memory Monitor for 7038
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BMRC_MONITOR_PRIV_H_
#define BMRC_MONITOR_PRIV_H_
#ifdef __cplusplus
extern "C" {
#endif

/* TODO: Make this detect version at runtime */
#if (BCHP_CHIP==3390) || (BCHP_CHIP==7145) || (BCHP_CHIP==7250) || (BCHP_CHIP==7252) || (BCHP_CHIP==7364)\
 || (BCHP_CHIP==7366) || (BCHP_CHIP==7399) || (BCHP_CHIP==7439) || (BCHP_CHIP==7445)
/* Actually protocol 0x50 */
#define BMRC_MONITOR_P_SCB_PROTOCOL_VER 0x42
#elif (BCHP_CHIP==7422) || (BCHP_CHIP==7425) || (BCHP_CHIP==7435) || (BCHP_CHIP==7325) || (BCHP_CHIP==7344)
#define BMRC_MONITOR_P_SCB_PROTOCOL_VER 0x42
#else
#define BMRC_MONITOR_P_SCB_PROTOCOL_VER 0x01
#endif

/* SCB Protocol Specifications the following are derived from are available at
   http://www.blr.broadcom.com/projects/DVT_BLR/Memc_Arch/.  */
typedef enum 
{
	BMRC_P_Monitor_ScbCommand_eLR = 1,
	BMRC_P_Monitor_ScbCommand_eLW = 2,
	BMRC_P_Monitor_ScbCommand_eREF = 3,
	BMRC_P_Monitor_ScbCommand_eMRS = 4,
	BMRC_P_Monitor_ScbCommand_eEMRS = 5,
	BMRC_P_Monitor_ScbCommand_ePALL = 6,
	BMRC_P_Monitor_ScbCommand_eDR = 7,
	BMRC_P_Monitor_ScbCommand_eDW = 8,
	BMRC_P_Monitor_ScbCommand_eMR = 9,
	BMRC_P_Monitor_ScbCommand_eMW = 10,
	BMRC_P_Monitor_ScbCommand_eCR4 = 11,
	BMRC_P_Monitor_ScbCommand_eCR8 = 12,
	BMRC_P_Monitor_ScbCommand_eUnknown = 0
}BMRC_P_Monitor_ScbCommand;


typedef struct BMRC_P_Monitor_FileClientInfo
{
	const char *prefix;
	const BMRC_Client *clients;
} BMRC_P_Monitor_FileClientInfo;

typedef struct BMRC_P_Monitor_ScbCommandInfo {
	BMRC_P_Monitor_ScbCommand eScbCommand;
	uint32_t ulCommand;
	uint32_t ulMask;
	const char *pName;
}BMRC_P_Monitor_ScbCommandInfo;

#define BMRC_P_MONITOR_SCB_MEM_ACCESS_ALIGN      ~0x000000FF /* some clients have J-word min access length */
#define BMRC_P_MONITOR_SCB_CHECKER_SIZE_MASK     ~0x00000007
#define BMRC_P_MONITOR_CHECKER_ADDR_ALIGN         BMRC_P_MONITOR_SCB_MEM_ACCESS_ALIGN


/* SCB defines */

#if (BMRC_MONITOR_P_SCB_PROTOCOL_VER >= 0x42)
#define BMRC_P_MONITOR_SCB_TRANSFER_ACCESS_MASK   0x1E0
#define BMRC_P_MONITOR_SCB_TRANSFER_SIZE_MASK     0x01F
#else
#define BMRC_P_MONITOR_SCB_TRANSFER_ACCESS_MASK   0x1F0
#define BMRC_P_MONITOR_SCB_TRANSFER_SIZE_MASK     0x00F
#endif

#define BMRC_P_MONITOR_SCB_INTERNAL_MASK          0x1FF
#define BMRC_P_MONITOR_SCB_MPEG_BLOCK_ACCESS_MASK 0x180
#define BMRC_P_MONITOR_SCB_CACHE_ACCESS_MASK      0x1FF

#define BMRC_P_MONITOR_SCB_TRANSFER_SIZE_MAX      (BMRC_P_MONITOR_SCB_TRANSFER_SIZE_MASK + 1)

/* SCB MPEG block command fields */
#define BMRC_P_MONITOR_SCB_MPEG_X_BIT             0x080
#define BMRC_P_MONITOR_SCB_MPEG_Y_MASK            0x03E
#define BMRC_P_MONITOR_SCB_MPEG_T_BIT             0x001

#define BMRC_P_MONITOR_SCB_YLINES_MAX             (64)

#if (BMRC_MONITOR_P_SCB_PROTOCOL_VER >= 0x42)
#define BMRC_P_MONITOR_SCB_COMMAND_LR   0x000
#define BMRC_P_MONITOR_SCB_COMMAND_LW   0x020
#define BMRC_P_MONITOR_SCB_COMMAND_DR   0x180
#define BMRC_P_MONITOR_SCB_COMMAND_DW   0x1A0
#define BMRC_P_MONITOR_SCB_COMMAND_CR4  0x044
#define BMRC_P_MONITOR_SCB_COMMAND_CR8  0x048

#else
#define BMRC_P_MONITOR_SCB_COMMAND_LR   0x000
#define BMRC_P_MONITOR_SCB_COMMAND_LW   0x010
#define BMRC_P_MONITOR_SCB_COMMAND_DR   0x060
#define BMRC_P_MONITOR_SCB_COMMAND_DW   0x070
#define BMRC_P_MONITOR_SCB_COMMAND_CR4  0x024
#define BMRC_P_MONITOR_SCB_COMMAND_CR8  0x028
#endif

#define BMRC_P_MONITOR_SCB_COMMAND_REF  0x05C
#define BMRC_P_MONITOR_SCB_COMMAND_MRS  0x05D
#define BMRC_P_MONITOR_SCB_COMMAND_EMRS 0x05E
#define BMRC_P_MONITOR_SCB_COMMAND_PALL 0x05F

#define BMRC_P_MONITOR_SCB_COMMAND_MR   0x080
#define BMRC_P_MONITOR_SCB_COMMAND_MW   0x100



struct BMRC_P_ClientMapEntry {
    BCHP_MemcClient client;
    BMRC_Monitor_HwBlock block;
};

struct BMRC_P_MonitorClientMap {
    uint8_t fileBlocks[BMRC_Monitor_HwBlock_eMax]; /* clients protected based on allocation place */
    struct BMRC_P_ClientMapEntry clientMap[BCHP_MemcClient_eMax+1]; /* this array sorted by value of memcClient and it's used to get list of MEMC clients by HW Block */
    uint16_t clientMapEntry[BMRC_Monitor_HwBlock_eMax]; /* this array has entry in sorted array of clientMap */
    uint16_t clientToBlock[BCHP_MemcClient_eMax]; /* map that translates BCHP_MemcClient to BMRC_Monitor_HwBlock */
};

struct BMRC_Monitor_P_ClientList {
    bool clients[BCHP_MemcClient_eMax];
};


void BMRC_Monitor_P_MapInit(struct BMRC_P_MonitorClientMap *map);
void BMRC_Monitor_P_MapGetClientsByFileName(const struct BMRC_P_MonitorClientMap *map, const char *fname, struct BMRC_Monitor_P_ClientList *clientList);
void BMRC_Monitor_P_MapGetHwClients(struct BMRC_P_MonitorClientMap *map, struct BMRC_Monitor_P_ClientList *clientList);
void BMRC_Monitor_P_PrintBitmap(const struct BMRC_P_MonitorClientMap *map, const uint32_t *bitmap, size_t bitmapSize, const char *blockedType);
void BMRC_Monitor_P_SetHwBlocks(const struct BMRC_P_MonitorClientMap *map, const uint8_t *hwBlocks, struct BMRC_Monitor_P_ClientList *clientList, bool add);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* BMRC_MONITOR_PRIV_H_ */

/* End of File */
