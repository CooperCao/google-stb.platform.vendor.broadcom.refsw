/***************************************************************
**
** Broadcom Corp. Confidential
** Copyright 2003-2008 Broadcom Corp. All Rights Reserved.
**
** THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED
** SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM.
** YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT
** SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
**
** File:        mpod_test.c
** Description: Simple test utility for exercising the CableCard
**              Application Library.
**
** Created: 04/18/2001
**
** REVISION:
**
** $Log: $
**
**
****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "b_os_lib.h"
#include "mpod.h"
#include "mpod_list.h"
#include "mpod_ioctls.h"
#include "mpod_link.h"
#include "mpod_session.h"
#include "mpod_util.h"

#include "mpod_apinfo.h"
#include "mpod_ca.h"
#include "mpod_cp.h"
#include "test_cp.h"
#include "mpod_diag.h"
#include "mpod_download.h"
#include "mpod_ext_chan.h"
#include "mpod_feature.h"
#include "mpod_hc.h"
#include "mpod_headend_comm.h"
#include "mpod_homing.h"
#include "mpod_host_property.h"
#include "mpod_mmi.h"
#include "mpod_res.h"
#include "mpod_resrc_mgr.h"
#include "mpod_sas.h"
#include "mpod_snmp.h"
#include "mpod_systime.h"
#include "mpod_test_io.h"
#ifdef ESTB_CFG_SUPPORT
#include "b_estb_cfg_lib.h"
#endif

BDBG_MODULE(MPOD_TEST);

int fppod;

extern struct Session_Conn SessionLists;
extern MPOD_LINK_CONN MPOD_LinkConn;


B_MPOD_SAS_HANDLE SasConnections[32]; /* max 32 connections allowed by the Host 2.0 spec */
uint8_t SasData[8] = {0xde,0xad, 0xbe, 0xef, 0x01, 0x23, 0x45, 0x67};


extern MPOD_RET_CODE linkConnStart(
    MPOD_LINK_CONN *linkConn
    );

extern MPOD_RET_CODE linkConnStop(
    void
    );



/**************
** Apinfo & MMI
**************/
static uint8_t TransactionNumber = 0;
static uint16_t sessionNumber;


static void apInfoInfoChangedCb(
    uint8_t *apInfoLoop,
    uint32_t len
    )
{
    printf("apInfoInfoChangedCb Callback Called\n");
}


#define SERVER_QUERY_TAG 0x9f8022
#define SERVER_REPLY_TAG 0x9f8023
static void apInfoAPDURcvCb(
    uint16_t sessionNb,
    uint8_t *data,
    uint32_t tag,
    uint32_t len
    )
{
    BSTD_UNUSED(len);

    switch (tag)
    {
        case SERVER_REPLY_TAG:
        {
            uint32_t headerLength, fileLength;
            uint8_t *headerByte, *fileByte;

            /* forward URL contents to client */
            printf("External handler received SERVER_REPLY_TAG from MPOD.\n");

            printf("transactionNumber = %d\n", *(data++));
            printf("fileStatus = %d\n", *(data++));
            headerLength = ((uint16_t)(*data) << 8) | (uint16_t)*(data+1);
            printf("headerLength = %d\n", headerLength);

            data += 2;

            headerByte = data;
            data += headerLength;

            /* display for debug - careful since it's not NULL terminated */
            if(headerLength)
            {
                char last = headerByte[headerLength-1];

                headerByte[headerLength-1] = '\0';
                printf("HEADER STR = %s%c\n",headerByte, last);
                headerByte[headerLength-1] = last;
            }

            fileLength = ((uint16_t)(*data) << 8) | (uint16_t)*(data+1);
            data += 2;
            fileByte = data;
            data += fileLength;

            /* display for debug - careful since it's not NULL terminated */
            if(fileLength)
            {
                char last = fileByte[fileLength-1];

                fileByte[fileLength-1] = '\0';
                printf("FILE STR = %s%c\n",fileByte, last);
                fileByte[fileLength-1] = last;
            }
        }
        break;

        default:
            BDBG_ERR(("Invalid tag val APINFO data rcv'ed!!!"));
        break;
    }
}


static uint32_t dialogs[8];
#define MARK_AS_VALID(number)   (dialogs[(number) >> 5] |=  (1 << ((number) & 0x1f)))
#define MARK_AS_INVALID(number) (dialogs[(number) >> 5] &= ~(1 << ((number) & 0x1f)))
#define CHECK_IF_VALID(number)  (dialogs[(number) >> 5] &   (1 << ((number) & 0x1f)))


static void mmiHtmlRdyCb(
    uint8_t *html,
    uint16_t len,
    uint8_t dialogNb,
    bool hostDialog,
    uint8_t fileStatus
    )
{
    /* add NULL termination */
    html[len - 1] = '\0';
    printf("mmiHtmlRdyCb Callback Called for dialog number %d, file status %d\n", dialogNb, fileStatus);
    printf("This is a %s requested dialog\n", hostDialog ? "Host" : "Card");
}


static B_MPOD_MMI_OPEN_STATUS mmiDialogRequestCb(
    B_MPOD_MMI_DISPLAY_TYPE displayType,
    uint8_t dialogNb
    )
{
    printf("mmiDialogRequestCb Callback Called with dialog number %d for display type %d\n", dialogNb, (uint32_t)displayType);
    return B_MPOD_MMI_OK;
}


static void mmiDialogCloseCb(
    uint8_t dialogNb
    )
{
    printf("mmiDialogCloseCb Callback Called for dialog %d\n", dialogNb);
    MARK_AS_INVALID(dialogNb);
}


static void mmiAPDURcvCb(
    uint16_t sessionNb,
    uint8_t *data,
    uint32_t tag,
    uint32_t len
    )
{
    BSTD_UNUSED(data);
    BSTD_UNUSED(tag);
    BSTD_UNUSED(len);

    printf("mmiAPDURcvCb Callback Called\n");
}


static void mmiExternalHandlerRdy(
    void
    )
{
    printf("mmiExternalHandlerRdy Callback Called\n");
}


static void mmiSessionOpenCb(
    uint16_t sessionNb
    )
{
    BSTD_UNUSED(sessionNb);

    printf("mmiSessionOpenCb Callback Called\n");
}


static void mmiSessionCloseCb(
    uint16_t sessionNb
    )
{
    BSTD_UNUSED(sessionNb);

    printf("mmiSessionCloseCb Callback Called\n");
}


static MPOD_RET_CODE testServerQuery(
    char *urlData,
    uint16_t urlLen,
    uint8_t *transactionNb
    )
{
    uint8_t *data, *apduStart;
    uint32_t apduLen;

    if ( (data = Ap_Alloc_Buf(SERVER_QUERY_TAG, urlLen+5, &apduStart, &apduLen)) == NULL)
    {
        printf("Failed to alloc mem for SERVER_QUERY_TAG!!!\n");
        return MPOD_NO_MEMORY;
    }

    data[0] = TransactionNumber;
    data[1] = 0;  /* header length */
    data[2] = 0;  /* header length */
    data[3] = (uint8_t)((urlLen >> 8) & 0x00FF);  /* url length */
    data[4] = (uint8_t)(urlLen & 0x00FF);  /* url length */
    BKNI_Memcpy(data+5, urlData, urlLen);

    /* display for debug - careful since it's not NULL terminated */
    {
        char last = urlData[urlLen-1];
        urlData[urlLen-1] = '\0';
        printf("Sending ServerQuery %d for URL %s%c\n", TransactionNumber, urlData, last);
        urlData[urlLen-1] = last;
    }

    *transactionNb = TransactionNumber++;

    return B_Mpod_SessionSendData(sessionNumber, apduStart, apduLen);
}


void printApInfoMmiMenu(
    void
    )
{
    printf("ApInfo & MMI Test Menu:\n");
    printf("1 : Print out all card app menus\n");
    printf("2 : Close all host initiated MMI dialogs\n");
    printf("3 : Register an external MMI/APINFO handler\n");
    printf("4 : Unregister an external MMI/APINFO handler\n");
    printf("5 : As an external MMI/APINFO handler print out all card app menus \n");
    printf("0 : Quit this menu.\n");
}


void mpodApInfoMmiTest(
    void
    )
{
    char cmd[256];
    uint32_t end_loop = 0;


    while (!end_loop)
    {
        printApInfoMmiMenu();

        printf("\nEnter Option:");
        cmd[0] = GetInputChar();
        printf("\n");

        switch(cmd[0])
        {
            case '1':
            {
                /* display all of the CableCard application dialogs */
                char *appUrl, *appName;
                uint8_t type, numApps, i, j, appLen, urlLen, dialogNb;
                uint16_t version, manuId;
                MPOD_RET_CODE err;

                err  = B_Mpod_AppInfoGetManuID(&manuId);
                err |= B_Mpod_AppInfoGetVersionNum(&version);
                err |= B_Mpod_AppInfoGetNumApps(&numApps);

                if(err)
                {
                    printf("\n!!!!ERROR retrieving application dialogs\n\n");
                }
                else
                {
                    printf("CableCard ApInfo: Manufacturer ID = %x, Version = %d\n", manuId, version);

                    for(j = 0; j < 4; j++)
                    {
                        for(i = 0; i < numApps; i++)
                        {
                            err = B_Mpod_AppInfoGetType(i, &type);
                            if(err) printf("ERROR B_Mpod_AppInfoGetType\n");
                            err |= B_Mpod_AppInfoGetName(i, &appName, &appLen);
                            if(err) printf("ERROR B_Mpod_AppInfoGetName\n");
                            err |= B_Mpod_AppInfoGetURL(i, &appUrl, &urlLen);
                            if(err) printf("ERROR B_Mpod_AppInfoGetURL\n");

                            if(err)
                            {
                                printf("\n!!!!ERROR retrieving application dialogs\n\n");
                                break;
                            }
                            else
                            {
                                printf("Idx %d\n------\n\ttype %d\n\tName %s\n\tURL %s\n", i, type, appName, appUrl);
                                err = B_Mpod_MMIHostOpenDialog(appUrl, urlLen, &dialogNb);
                                if(err)
                                {
                                    printf("ERROR Opening Dialog\n");
                                }
                                else
                                {
                                    MARK_AS_VALID(dialogNb);
                                    printf("Dialog #%d opened\n", dialogNb);
                                }
                            }
                        }
                    }
                }
            }
            break;

            case '2':
            {
                /* close all dialogs */

                uint32_t i, j;

                printf("Closing all host initiated dialogs\n");

                for(i = 0; i < 8; i++)
                {
                    j = i * 32;

                    /* terminate all host initiated Dialogs */
                    while(dialogs[i])
                    {
                        BDBG_ASSERT(j < (i * 32) + 32);
                        if(CHECK_IF_VALID(j))
                        {
                            MARK_AS_INVALID(j);
                            printf("Closing Dialog #%d\n", j);
                            B_Mpod_MMIHostCloseDialog(j);
                        }
                        j++;
                    }
                }
            }
            break;

            case '3':
                B_Mpod_MMIRegisterExternalHandler();
            break;

            case '4':
                B_Mpod_MMIUnregisterExternalHandler();
            break;

            case '5':
            {
                /* send apdu to display all of the application dialogs */

                char *appUrl, *appName;
                uint8_t transactionNb;
                uint8_t type, numApps, i, j, appLen, urlLen;
                uint16_t version, manuId;
                MPOD_RET_CODE err;

                err  = B_Mpod_AppInfoGetManuID(&manuId);
                err |= B_Mpod_AppInfoGetVersionNum(&version);
                err |= B_Mpod_AppInfoGetNumApps(&numApps);

                if(err)
                {
                    printf("\n!!!!ERROR retrieving application dialogs\n\n");
                }
                else
                {
                    printf(" Manufacturer ID = %x, Version = %d\n", manuId, version);

                    for(j = 0; j < 4; j++)
                    {
                        for(i = 0; i < numApps; i++)
                        {
                            err = B_Mpod_AppInfoGetType(i, &type);
                            if(err) printf("ERROR B_Mpod_AppInfoGetType\n");
                            err |= B_Mpod_AppInfoGetName(i, &appName, &appLen);
                            if(err) printf("ERROR B_Mpod_AppInfoGetName\n");
                            err |= B_Mpod_AppInfoGetURL(i, &appUrl, &urlLen);
                            if(err) printf("ERROR B_Mpod_AppInfoGetURL\n");

                            if(err)
                            {
                                printf("\n!!!!ERROR retrieving application dialogs\n\n");
                                break;
                            }
                            else
                            {
                                printf("Idx %d\n------\n\ttype %d\n\tName %s\n\tURL %s\n", i, type, appName, appUrl);
                                err = testServerQuery(appUrl, urlLen, &transactionNb);

                                if(err)
                                {
                                    printf("ERROR Opening Dialog\n");
                                }
                                else
                                {
                                    printf("Dialog opened\n");
                                }
                            }
                        }
                    }
                }
            }

            break;
            case '0':
                end_loop = 1;
            break;

        }
    }
}



/**************
** CA & CP
**************/

#define MPEG_CA_DESCRIPTOR_TAG 0x09
#define MPEG_CRC_LENGTH 4
#define PMT_BASE_LEN 0x0c
#define PMT_ES_BASE_LEN 0x05

#define CA_PMT_BASE_LEN 0x0a
#define CA_PMT_CMD_ID_LEN 0x01
#define CA_PMT_ES_BASE_LEN 0x05

#define CA_ENA_FLAG_MASK 0x80

void caInfoCb(
    uint16_t *caSystemId,
    uint32_t numSystemId
    )
{
    uint32_t i;
    printf("%s\n", __FUNCTION__);
    for(i = 0; i < numSystemId; i++)
        printf("CA System Id [%d]: %x\n", i, caSystemId[i]);
}


void caPmtUpdateReply(
    B_MPOD_CA_PMT_UPDATE_INFO *replyInfo
    )
{
    uint32_t i;
    uint16_t pid;
    bool caEnableFlag;
    B_MPOD_CA_ENABLE caEnable;

    printf("\nProg Idx: %d  Prog Num: %d, Src Id: %d\n",
            replyInfo->progIndex, replyInfo->progNum, replyInfo->srcId);
    printf("Trans Id: %d  LTSID: %d\n", replyInfo->transId, replyInfo->ltsid);
    if(replyInfo->caEnaFlag)
        printf("Program Level CA Enable: %d\n", replyInfo->caEna);
    else
        printf("No Program Level CA Enable\n");

    for(i = 0; i < replyInfo->numElem; i++)
    {
        B_MPOD_CA_GET_PMT_ES_INFO(replyInfo->esInfo, i, pid, caEnableFlag, caEnable);
        if(caEnableFlag)
            printf("Es Level CA Enable:  pid:%02x  CA Enable:%d\n", pid, (uint8_t)caEnable);
        else
            printf("No Es Level CA Enable for pid %02x", pid);
    }
}


void caPmtUpdate(
    B_MPOD_CA_PMT_UPDATE_INFO *updateInfo
    )
{
    printf("%s\n", __FUNCTION__);
    caPmtUpdateReply(updateInfo);
}


void caPmtReply(
    B_MPOD_CA_PMT_REPLY_INFO *replyInfo
    )
{
    printf("%s\n", __FUNCTION__);
    caPmtUpdateReply(replyInfo);
}


/* we use this function to fake a PMT to send to CA resource. */
static void caPmtTest(
    void
    )
{
    uint32_t num_es, i;
    uint32_t size;
    char inputs[256];
    uint32_t temp;
    uint8_t CA_desc[] = {MPEG_CA_DESCRIPTOR_TAG,0x8,0x00,0x01,0xe8,0x00,0x55,0x66,0x77,0x88};
    uint8_t any_desc[] = {0x02,0x9,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9};
    uint8_t *pmt, *ptr;
    uint8_t prog_indx, ltsid;
    uint16_t source_id, program_num, ecmpid;

    printf("Please enter ltsid 0x");
    GetInputString(inputs);
    printf("\n");
    sscanf(inputs,"%x",&temp);
    ltsid = (uint8_t)temp;

    printf("Please enter source id 0x");
    GetInputString(inputs);
    printf("\n");
    sscanf(inputs,"%x",&temp);
    source_id = (uint8_t)temp;

    printf("Please enter program index 0x");
    GetInputString(inputs);
    printf("\n");
    sscanf(inputs,"%x",&temp);
    prog_indx = (uint8_t)temp;

    printf("Please enter ecm pid 0x");
    GetInputString(inputs);
    printf("\n");
    sscanf(inputs,"%x",&temp);
    ecmpid = (uint16_t)temp;

    printf("Please enter the number of ES streams 0x");
    GetInputString(inputs);
    printf("\n");
    sscanf(inputs,"%x",&num_es);
    size = num_es * (PMT_ES_BASE_LEN+sizeof(CA_desc)+sizeof(any_desc)) +
            (PMT_BASE_LEN+sizeof(CA_desc)+sizeof(any_desc)) + MPEG_CRC_LENGTH ;

    if ((pmt = BKNI_Malloc(size)) == NULL)
    {
        printf("Failed to alloc mem for PMT!!!\n");
        return;
    }

    BKNI_Memset(pmt, 0, size);

    /* don't care about most of the fields, just add the essential ones */

    /* section length + ssi */
    pmt[1] = 0xb0 | (uint8_t)(((size-3) & 0xf00) >> 8);
    pmt[2] = (uint8_t)((size-3) & 0xff);

    printf("Please enter the Program Number 0x");
    GetInputString(inputs);
    printf("\n");
    sscanf(inputs,"%x",&temp);

    /* program number */
    pmt[3] = (uint8_t)((temp & 0xff00) >> 8);
    pmt[4] = (uint8_t)(temp & 0xff);

    program_num = (uint16_t)temp;

    /* program_info_length */
    pmt[10] = 0xf0 | (uint8_t)(((sizeof(CA_desc) + sizeof(any_desc)) & 0xf00) >> 8);
    pmt[11] = (uint8_t)((sizeof(CA_desc) + sizeof(any_desc)) & 0xff);

    /* copy descriptors */
    BKNI_Memcpy(pmt + PMT_BASE_LEN, CA_desc, sizeof(CA_desc));
    BKNI_Memcpy(pmt + PMT_BASE_LEN + sizeof(CA_desc), any_desc, sizeof(any_desc));

    ptr = pmt + PMT_BASE_LEN + sizeof(CA_desc) + sizeof(any_desc);

    for (i=0; i<num_es; i++)
    {
        printf("Please enter the stream type(0x2 for video, 0x4 for audio) 0x");
        GetInputString(inputs);
        printf("\n");
        sscanf(inputs,"%x",&temp);

        /* stream_id */
        ptr[0] = (uint8_t)temp;

        printf("Please enter the stream pid 0x");
        GetInputString(inputs);
        printf("\n");
        sscanf(inputs,"%x",&temp);

        /* pid */
        temp |= 0xe000;
        ptr[1] = (uint8_t)((temp & 0xff00) >> 8);
        ptr[2] = (uint8_t)(temp & 0xff);

        /* ES_info_len */
        ptr[3] = (uint8_t)(((sizeof(CA_desc)+sizeof(any_desc)) & 0xff00) >> 8);
        ptr[4] = (uint8_t)((sizeof(CA_desc)+sizeof(any_desc)) & 0xff);

        BKNI_Memcpy(ptr+PMT_ES_BASE_LEN, CA_desc, sizeof(CA_desc));
        BKNI_Memcpy(ptr+PMT_ES_BASE_LEN+sizeof(CA_desc), any_desc, sizeof(any_desc));

        ptr += (PMT_ES_BASE_LEN+sizeof(CA_desc)+sizeof(any_desc));
    }

    printf("pmt size = 0x%x, data=\n", size);
    for (i=0; i<size; i++)
    {
        printf("0x%02x, ", pmt[i]);
        if (i%16==0)
            printf("\n");
    }

    B_Mpod_CpAddProgram(prog_indx, program_num, ecmpid, ltsid);
    B_Mpod_CaSendPmt(pmt, B_MPOD_CA_OK_DESCRAMBLE, prog_indx, ltsid, source_id);
}


void getAuthKeyCb(
    uint8_t *authKey,
    bool *authKeyExists
    )
{
    B_Mpod_TestCpGetAuthKey(authKey, authKeyExists);
}

void getIDCb(
	uint8_t * hostId,
	uint8_t * cardId)
{
	B_Mpod_TestCpGetID(hostId, cardId);
}


void cardAuthMsgCb(
    uint8_t *cardDevCert,
    uint8_t *cardManCert,
    uint8_t *dhPubKeyC,
    uint8_t *signC,
    uint8_t *hostDevCert,
    uint8_t *hostManCert,
    uint8_t *dhPubKeyH,
    uint8_t *signH
    )
{
    printf("Getting Host CP Authentication Parameters\n");
    B_Mpod_TestCpGetHostAuthParams(hostDevCert, hostManCert, dhPubKeyH, signH);
    printf("Checking Card CP Authorization, generating AuthKey\n");
    B_Mpod_TestCpGenAuthKeyH(cardDevCert, cardManCert, dhPubKeyC, signC);
}


void getNonceCb(
    uint8_t *nonce
    )
{
    B_Mpod_TestCpGenerateNonce(nonce);
}


void cpkeyGenCb(
    uint8_t *nHost,
    uint8_t *nCard,
    uint8_t *cpKeyA,
    uint8_t *cpKeyB
    )
{
    printf("Generating New CPKey");

    B_Mpod_TestCpGenCPKey(nHost, nCard, cpKeyA, cpKeyB);
}


void removeKeyCb(
    uint16_t programNumber,
    uint8_t ltsid
    )
{
    printf("Removing CPKey for program number %d, ltsid %d\n", programNumber, ltsid);
}


void progKeyCb(
    uint8_t *desABAKey,
    uint16_t programNumber,
    uint8_t ltsid
    )
{
    uint32_t i;

    printf("Programming the CPkey for program number %d, ltsid %d\n", programNumber, ltsid);
    printf("Des ABA key is ");
    for(i = 0; i < 16; i++) printf("%d ", desABAKey[i]);
    printf("\n");
}


void calcCciAckCb(
    uint8_t cpVersion,
    uint8_t cci,
    uint8_t *cpKeyA,
    uint8_t *cpKeyB,
    uint8_t *cciNCard,
    uint8_t *cciNHost,
    uint16_t programNumber,
    uint8_t ltsid,
    uint16_t ecmpid,
    uint8_t *cciAuth,
    uint8_t *cciAck
    )
{
    printf("Calculating new CP cci ack value\n");
    B_Mpod_TestCpGenCciAck(cpVersion, cci, cpKeyA, cpKeyB, cciNCard,
                cciNHost, programNumber, ltsid, ecmpid, cciAuth, cciAck);
}


void enforceCciCb(
    uint8_t cci,
    uint16_t programNumber,
    uint8_t ltsid)
{
    BSTD_UNUSED(cci);
    BSTD_UNUSED(programNumber);
    BSTD_UNUSED(ltsid);

    printf("Received new CP cci status from the card, cci is %d\n", cci);
}


void newValidationStatusCb(
    B_MPOD_CP_VALIDATION_STATUS validationStatus
    )
{
    printf("Received new CP validation status from the card, status is %d\n", validationStatus);
}



/*****************
** Diags
*****************/
/* sample diagnostic status/result for testing */

/****** Memory Diag */
B_MPOD_DIAG_MEM_ELEMENT memElement[] =
{
    {B_MPOD_DIAG_MEM_TYPE_DRAM, 128 * 1024},
    {B_MPOD_DIAG_MEM_TYPE_FLASH, 32 * 1024}
};

B_MPOD_DIAG_MEM_REPORT memoryStatus = {2, memElement};


/****** Sw Ver Diag */
B_MPOD_DIAG_APP_VER swVer =
{
            11, /* version */
            "app version",
            B_MPOD_DIAG_APP_STATUS_DOWNLOADING, /* status flag */
            8,
            "app name",
            8,
            "app sign"
};

B_MPOD_DIAG_SW_VER_REPORT swVerStatus =
{
        1, /* number of applications */
        &swVer
};


/****** firmware Diag */
B_MPOD_DIAG_FW_REPORT firmwareStatus =
{
    16, "firmware version", /* firmware version */
    2008, 1, 1, /* firmware date */
};


/****** Mac Address Diag */
uint8_t macAddr[] = { 0,1,2,3,4,5 };

B_MPOD_DIAG_MAC_DEV macDevice =
{
        B_MPOD_DIAG_ADDR_TYPE_HOST, /* type */
        6, /* numer of byte */
        macAddr
};

B_MPOD_DIAG_MAC_ADDR_REPORT macAddrStatus =
{
    1, /* numner of MAC */
    &macDevice
};


/****** FAT Status Diag */
B_MPOD_DIAG_FAT_STATUS_REPORT fatStatus =
{
    B_MPOD_DIAG_FAT_PCR_LOCK |
    B_MPOD_DIAG_FAT_MOD_QAM64 |
    B_MPOD_DIAG_FAT_CARR_LOCK,
    12345, /* fat snr */
    40
};


/****** FDC Status Diag */
B_MPOD_DIAG_FDC_STATUS_REPORT fdcStatus =
{
    12345, /* fdc_ceter_freq */
    B_MPOD_DIAG_FDC_LOCK
};


/****** Current Channel Diag */
B_MPOD_DIAG_CURR_CHANNEL_REPORT curChannelStatus =
{

    B_MPOD_DIAG_CHAN_DIGITAL |
    B_MPOD_DIAG_CHAN_PURCHASED |
    B_MPOD_DIAG_CHAN_PREVIEW |
    B_MPOD_DIAG_CHAN_PARENTAL_BLOCK,
    123
};


/****** 1394 Port Diag */
B_MPOD_DIAG_1394_CON_DEV e1394ConDevStatus =
{
    B_MPOD_DIAG_1394_CAMERA_STORAGE |
    B_MPOD_DIAG_1394_A2D_SRC_SEL_SUPP,
    "12345678"
};


B_MPOD_DIAG_1394_PORT_REPORT e1394PortStatus =
{
    B_MPOD_DIAG_1394_LOOP_EXIST |
    B_MPOD_DIAG_1394_IS_ROOT |
    B_MPOD_DIAG_1394_CYCLE_MASTER |
    B_MPOD_DIAG_1394_AD_SOURCE |
    B_MPOD_DIAG_1394_PORT_1_CONN |
    B_MPOD_DIAG_1394_PORT_2_CONN,
    1, /* total number of nodes */
    1, /* number of connected devices */
    &e1394ConDevStatus
};


/****** DVI Status Diag */
B_MPOD_DIAG_DVI_STATUS_REPORT dviStatus =
{
    B_MPOD_DIAG_DVI_CONN_REPEATER,
    480,
    640,
    B_MPOD_DIAG_HZ29_97,
    B_MPOD_DIAG_DVI_ASPECT4_3 |
    B_MPOD_DIAG_DVI_INTER
};


/****** ECM Status Diag */
B_MPOD_DIAG_ECM_STATUS_REPORT ecmStatus =
{
    123,
    456,
    B_MPOD_DIAG_ECM_DS_LOCKED |
    B_MPOD_DIAG_ECM_S_CDMA_TDMA |
    B_MPOD_DIAG_ECM_US_MOD_QAM64,
    789,
    123,
    B_MPOD_DIAG_MSPS_0_64
};


/****** HDMI Status Diag */
B_MPOD_DIAG_HDMI_STATUS_REPORT hdmiStatus =
{
    B_MPOD_DIAG_HDMI_COLOR_SPACE_YCC422 |
    B_MPOD_DIAG_HDMI_CONN_NOREPEATER |
    B_MPOD_DIAG_HDMI_HOST_HDCP_EN |
    B_MPOD_DIAG_HDMI_DEV_HDCP_COMP,
    640,
    480,
    B_MPOD_DIAG_HZ29_97,
    B_MPOD_DIAG_HDMI_ASPECT4_3 |
    B_MPOD_DIAG_HDMI_PROG,
    B_MPOD_DIAG_AUDIO_SAMPLE_SIZE_16 |
    B_MPOD_DIAG_AUDIO_FORMAT_PCM |
    B_MPOD_DIAG_AUDIO_SAMPLE_FREQ_48
};


/****** RDC Status Diag */
B_MPOD_DIAG_RDC_STATUS_REPORT rdcStatus =
{
    1235,
    56,
    B_MPOD_DIAG_KBPS1544
};


/****** Network Address Diag */
uint8_t netAddr[] = {18, 52, 86, 126};
uint8_t subNetAddr[] = {255 ,255 ,255 ,0};

B_MPOD_DIAG_OCHD2_DEV ochd2Dev =
{
        B_MPOD_DIAG_ADDR_TYPE_DOCSIS,
        4,
        netAddr,
        4,
        subNetAddr
};

B_MPOD_DIAG_NET_ADDR_REPORT netAddrStatus =
{
    1,
    &ochd2Dev
};


/****** Home Network Diag */
uint8_t clientIPAddr[] = {192,168,1,1};

B_MPOD_DIAG_HOME_NET_CLIENT homeNetworkClient =
{
        {0x00, 0x10, 0x18, 0xde ,0xad, 0xbe},
        4,
        clientIPAddr,
        B_MPOD_DIAG_HOME_NET_NO_CLIENT_TRUST
};

B_MPOD_DIAG_HOME_NET_REPORT homeNetworkStatus =
{
    2, /* max clients */
    B_MPOD_DIAG_HOME_NET_HOST_CLIENT_DRM, /* Host DRM Status */
    1, /* connected clients */
    &homeNetworkClient
};


/****** Host Info Diag */
B_MPOD_DIAG_HOST_INFO_REPORT hostInfoStatus =
{
    8,  "Broadcom",
    6,  "SetTop",
    0x123456,
    0x789abcde,
    0x9876
};



void diagReqCb(
    uint8_t numDiags,
    B_MPOD_DIAG_DATA *diagReqests
    )
{
    uint8_t i;
    printf("received %s\n", __FUNCTION__);

    for(i = 0; i < numDiags; i++)
    {

        if(diagReqests[i].id == B_MPOD_DIAG_ID_MEMORY) diagReqests[i].report.memory = &memoryStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_SW_VER) diagReqests[i].report.swVer = &swVerStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_FW_VER) diagReqests[i].report.firmware = &firmwareStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_MAC_ADDR) diagReqests[i].report.macAddr = &macAddrStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_FAT_STATUS) diagReqests[i].report.fatStatus = &fatStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_FDC_STATUS) diagReqests[i].report.fdcStatus = &fdcStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_CURR_CHANNEL) diagReqests[i].report.curChannel = &curChannelStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_1394_PORT) diagReqests[i].report.e1394Port = &e1394PortStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_DVI_STATUS) diagReqests[i].report.dviStatus = &dviStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_ECM_STATUS) diagReqests[i].report.ecmStatus = &ecmStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_HDMI_PORT_STATUS) diagReqests[i].report.hdmiStatus = &hdmiStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_RDC_STATUS) diagReqests[i].report.rdcStatus = &rdcStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_NET_ADDRESS) diagReqests[i].report.netAddr = &netAddrStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_HOME_NETWORK) diagReqests[i].report.homeNetwork = &homeNetworkStatus;
        if(diagReqests[i].id == B_MPOD_DIAG_ID_HOST_INFO) diagReqests[i].report.hostInfo = &hostInfoStatus;

        diagReqests[i].ltsid = 0xaa;
        diagReqests[i].status = B_MPOD_DIAG_GRANTED;
    }
}



/****************
** Download
****************/
#ifdef INTEGRATE_BCM_CDL
static void dlNewCodeVersionTableCb(
    uint8_t *data,
    uint32_t len,
    B_MPOD_DL_CVT_HOST_RESPONSE *cvtResponse
    )
{
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);
    printf("MPOD_TEST-DL: Got new code version table callback");
    *cvtResponse = B_MPOD_DL_ACT_NO_ERROR;
}

static void dlNewCodeVersionTable2Cb(
    uint8_t *data,
    uint32_t len,
    B_MPOD_DL_CVT_HOST_RESPONSE *cvtResponse
    )
{
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);
    printf("MPOD_TEST-DL: Got new code version table ver 2 callback");
    *cvtResponse = B_MPOD_DL_ACT_NO_ERROR;
}

#else
static void dlNewCodeVersionTableCb(
    B_MPOD_DL_CVT *cvt,
    B_MPOD_DL_CVT_HOST_RESPONSE *cvtResponse
    )
{
    BSTD_UNUSED(cvt);
    printf("MPOD_TEST-DL: Got new code version table callback");
    *cvtResponse = B_MPOD_DL_ACT_NO_ERROR;
}
#endif

uint8_t descriptorBlock[] =
/* tag length                      data                       */
{
    0,  12,      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
    1,  11,     12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    2,  10,     23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    3,   9,     33, 34, 35, 36, 37, 38, 39, 40, 41,
    4,   8,     42, 43, 44, 45, 46, 47, 48, 49,
    5,   7,     50, 51, 52, 53, 54, 55, 56,
    6,   6,     57, 58, 59, 60, 61, 62,
    7,   5,     63, 64, 65, 66, 67,
    8,   4,     68, 69, 70, 71,
    9,   3,     72, 73, 74,
    10,  2,     75, 76,
    11,  1,     77
};


static void dlHostInfoCb(
    B_MPOD_DL_SUPPORTED_DL_TYPE supportedDlType,
    B_MPOD_DL_HOST_INFO *hostInfo
    )
{
    BSTD_UNUSED(supportedDlType);
    printf("MPOD_TEST-DL: Got Host Info callback");

    hostInfo->vendorId = 0x3a3a3a;
    hostInfo->hardwareVersionId = 0x8c8c8c8c;
    hostInfo->numDescriptors = 12;
    hostInfo->descriptors = descriptorBlock;
}



/********************
** Extended Ch & DSG
********************/
void printExtChanMenu(
    void
    )
{
    printf("MPOD Extended Channel Test Menu:\n");
    printf("1 : Open an MPEG flow.\n");
    printf("2 : Open a IP_U flow. (For OOB mode ONLY!)\n");
    printf("3 : Open a IP_M flow. (For OOB mode ONLY!)\n");
    printf("4 : Delete a flow. \n");
    printf("5 : Send lost flow indication \n");
    printf("6 : Send DSG Message using DSG resource\n");
    printf("7 : Send DCD Info using DSG resource\n");
    printf("8 : Send DSG Message using Ext Channel resource\n");
    printf("9 : Send DCD Info using Ext Channel resource\n");
    printf("a : Send flow data\n");

    printf("0 : Quit this menu.\n");
}

void mpodExtChanTest(
    void
    )
{
    char cmd[256], inputs[256];
    uint32_t end_loop = 0;
    uint32_t temp;
    uint16_t pid;

    while (!end_loop)
    {
        printExtChanMenu();

        printf("\nEnter Option:");
        cmd[0] = GetInputChar();
        printf("\n");

        switch(cmd[0])
        {
            case '1':
                printf("Please enter the PID for the flow in hex: 0x");
                GetInputString(inputs);
                printf("\n");
                sscanf(inputs,"%x",&temp);
                pid = temp;

                B_Mpod_ExtChOpenMpegFlow(pid);
            break;

            case '2':
            {
                uint8_t macAddr[6] = {0x00, 0x10, 0x18, 0xc0, 0x10, 0x00};

                B_Mpod_ExtChOpenIpUnicastFlow(macAddr, 0, NULL);
            }
            break;

            case '3':
            {
                uint32_t mgid = 0x01234567;

                B_Mpod_ExtChOpenIpMulticastFlow(mgid);
            }
            break;

            case '4':
                printf("Please enter the flow id in hex: 0x");
                GetInputString(inputs);
                printf("\n");
                sscanf(inputs,"%x",&temp);

                B_Mpod_ExtChDelFlow(temp);
            break;

            case '5':
                printf("Please enter the flow id in hex: 0x");
                GetInputString(inputs);
                printf("\n");
                sscanf(inputs,"%x",&temp);

                B_Mpod_ExtChLostFlow(temp,  B_MPOD_EXT_LOST_FLOW_UNKNOWN);
            break;

            case '6':
            {
#ifndef INTEGRATE_BCM_DSG_HOST
                B_MPOD_DSG_MESSAGE dsgMessage;

                printf("Sending dsg_message()\n");

                dsgMessage.ucid = 129;
                dsgMessage.initType = B_MPOD_DSG_PERFORM_BCAST_RANGING;
                dsgMessage.disabledForwardingType = B_MPOD_DSG_FORWARD_IF_DOWN;

                dsgMessage.messageType = B_MPOD_DSG_APP_TUNNEL_REQ;
                B_Mpod_DsgSendDSGMessage(&dsgMessage);

                dsgMessage.messageType = B_MPOD_DSG_TWO_WAY_OK;
                B_Mpod_DsgSendDSGMessage(&dsgMessage);

                dsgMessage.messageType = B_MPOD_DSG_ENTERING_ONE_WAY;
                B_Mpod_DsgSendDSGMessage(&dsgMessage);

                dsgMessage.messageType = B_MPOD_DSG_DS_SCAN_COMPLETE;
                B_Mpod_DsgSendDSGMessage(&dsgMessage);

                dsgMessage.messageType = B_MPOD_DSG_DYNAMIC_CH_CHANGE;
                B_Mpod_DsgSendDSGMessage(&dsgMessage);

                dsgMessage.messageType = B_MPOD_DSG_ECM_RESET;
                B_Mpod_DsgSendDSGMessage(&dsgMessage);

                dsgMessage.messageType = B_MPOD_DSG_BAD_NUM_MAC_ADDR;
                B_Mpod_DsgSendDSGMessage(&dsgMessage);

                dsgMessage.messageType = B_MPOD_DSG_PROVISIONING_LIMIT;
                B_Mpod_DsgSendDSGMessage(&dsgMessage);
#endif
            }
            break;

            case '7':
            {
                uint8_t dcdInfo[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
                uint32_t len = sizeof(dcdInfo);

                B_Mpod_DsgSendDcdInfo(dcdInfo, len);
            }
            break;

            case '8':
            {
#ifndef INTEGRATE_BCM_DSG_HOST
                B_MPOD_EXT_DSG_MESSAGE extDsgMessage;
                uint16_t appIds[] = {0xff00, 0x00ff};

                printf("Sending dsg_message()\n");

                extDsgMessage.ucid = 129;
                extDsgMessage.initType = B_MPOD_DSG_PERFORM_BCAST_RANGING;
                extDsgMessage.numAppIds = 2;
                extDsgMessage.appIds = appIds;

                extDsgMessage.messageType = B_MPOD_DSG_APP_TUNNEL_REQ;
                B_Mpod_ExtChSendDSGMessage(&extDsgMessage);

                extDsgMessage.messageType = B_MPOD_DSG_TWO_WAY_OK;
                B_Mpod_ExtChSendDSGMessage(&extDsgMessage);

                extDsgMessage.messageType = B_MPOD_DSG_ENTERING_ONE_WAY;
                B_Mpod_ExtChSendDSGMessage(&extDsgMessage);

                extDsgMessage.messageType = B_MPOD_DSG_DS_SCAN_COMPLETE;
                B_Mpod_ExtChSendDSGMessage(&extDsgMessage);

                extDsgMessage.messageType = B_MPOD_DSG_DYNAMIC_CH_CHANGE;
                B_Mpod_ExtChSendDSGMessage(&extDsgMessage);

                extDsgMessage.messageType = B_MPOD_DSG_ECM_RESET;
                B_Mpod_ExtChSendDSGMessage(&extDsgMessage);
#endif
            }
            break;

            case '9':
            {
                uint8_t dcdInfo[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
                uint32_t len = sizeof(dcdInfo);

                B_Mpod_ExtChSendDcdInfo(dcdInfo, len);
            }
            break;

            case 'a':
            {
                uint8_t flowData[] = {0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf};
                uint32_t flowDataLen = sizeof(flowData);

                printf("Please enter the flow id in hex: 0x");
                GetInputString(inputs);
                printf("\n");
                sscanf(inputs,"%x",&temp);

                B_MPOD_ExtChSendFlowData(temp, flowData, flowDataLen);
            }
            break;

            case '0':
                end_loop = 1;
            break;

        }
    }
}


static B_EventHandle flowReqEvent;
static bool uniReq = false;
static bool sockReq = false;
static uint32_t flowID = 0x0;
static bool ThreadExit = false;

/* there can only ever be one flow outstanding (by design) so no need to lock the global vars */
static void handleFlowReq(
    void *unused
    )
{
    BSTD_UNUSED(unused);

    while(1)
    {
        B_Event_Wait(flowReqEvent, B_WAIT_FOREVER);
        if(ThreadExit) break;

        if(uniReq)
        {
            uint8_t ipAddr[16] = {0,0,0,0,0,0,0,0,0,0,0,0,192,168,1,10};
            B_Mpod_ExtChIpUnicastFlowCnf(flowID, B_MPOD_EXT_NEW_FLOW_GRANTED, ipAddr, 0, 1, 1500, 0, NULL);
        }
        else if(sockReq)
        {
            B_Mpod_ExtChSocketFlowCnf(flowID, B_MPOD_EXT_NEW_FLOW_GRANTED, 1500, 0, NULL);
            sockReq = false;
        }
        uniReq = sockReq = false;
    }
    B_Event_Destroy(flowReqEvent);
}


#ifdef INTEGRATE_BCM_DSG_HOST
static void reqIpUnicastFlowCb(
    uint32_t flowId,
    uint8_t *data,
    uint32_t len
    )
{
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);

    printf("received %s\n", __FUNCTION__);

    flowID = flowId;
    uniReq = true;
    B_Event_Set(flowReqEvent);
}


static void reqSocketFlowCb(
    uint32_t flowId,
    uint8_t *data,
    uint32_t len
    )
{
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);

    printf("received %s\n", __FUNCTION__);

    flowID = flowId;
    sockReq = true;
    B_Event_Set(flowReqEvent);
}


static void rcvSetDsgModeCb(
    uint8_t *data,
    uint32_t len
    )
{
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);

    printf("received %s\n", __FUNCTION__);
}


static void dsgErrorCb(
    uint8_t *data,
    uint32_t len
    )
{
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);

    printf("received %s\n", __FUNCTION__);
}


static void configAdvDsgCb(
    uint8_t *data,
    uint32_t len
    )
{
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);

    printf("received %s\n", __FUNCTION__);
}

#else

static void reqIpUnicastFlowCb(
    uint32_t flowId,
    B_MPOD_EXT_IP_U_REQ *ipReqInfo
    )
{
    BSTD_UNUSED(ipReqInfo);
    printf("received %s\n", __FUNCTION__);

    flowID = flowId;
    uniReq = true;
    B_Event_Set(flowReqEvent);
}


static void reqSocketFlowCb(
    uint32_t flowId,
    B_MPOD_EXT_SOCKET_REQ *socketReqInfo
    )
{
    BSTD_UNUSED(socketReqInfo);
    printf("received %s\n", __FUNCTION__);

    flowID = flowId;
    sockReq = true;
    B_Event_Set(flowReqEvent);
}


static void rcvSetDsgModeCb(
    B_MPOD_DSG_MODE_INFO *dsgModeInfo
    )
{
    BSTD_UNUSED(dsgModeInfo);

    printf("received %s\n", __FUNCTION__);
}


static void dsgErrorCb(
    B_MPOD_DSG_ERR err
    )
{
    BSTD_UNUSED(err);

    printf("received %s\n", __FUNCTION__);
}


static void configAdvDsgCb(
    B_MPOD_EXT_ADV_DSG_CONFIG *advDsgConfig
    )
{
    BSTD_UNUSED(advDsgConfig);

    printf("received %s\n", __FUNCTION__);
}
#endif


static B_MPOD_EXT_NEW_FLOW_CNF_STAT reqIpMulticastFlowCb(
    uint32_t flowId,
    uint32_t mgid
    )
{
    BSTD_UNUSED(flowId);
    BSTD_UNUSED(mgid);

    printf("received %s\n", __FUNCTION__);
    return B_MPOD_EXT_NEW_FLOW_GRANTED;
}


static B_MPOD_EXT_NEW_FLOW_CNF_STAT reqDsgFlowCb(
    uint32_t flowId
    )
{
    BSTD_UNUSED(flowId);

    printf("received %s\n", __FUNCTION__);
    return B_MPOD_EXT_NEW_FLOW_GRANTED;
}


static void flowReqFailedCb(
    B_MPOD_EXT_SERV_TYPE serviceType,
    B_MPOD_EXT_NEW_FLOW_CNF_STAT status
    )
{
    BSTD_UNUSED(serviceType);
    BSTD_UNUSED(status);

    printf("received %s\n", __FUNCTION__);
}


static void newFlowCnfCb(
    uint32_t flowId,
    B_MPOD_EXT_SERV_TYPE serviceType,
    uint16_t pid /* only valid for MPEG flows */
    )
{
	BSTD_UNUSED(flowId);
    printf("received %s flowId=%d serviceType=%d pid=%d\n", __FUNCTION__, flowID, (uint32_t) serviceType, pid);
}


static B_MPOD_EXT_DEL_FLOW_CNF_STAT delFlowReqCb(
    uint32_t flowId,
    B_MPOD_EXT_SERV_TYPE serviceType
    )
{
    BSTD_UNUSED(flowId);
    BSTD_UNUSED(serviceType);

    printf("received %s\n", __FUNCTION__);
    return B_MPOD_EXT_DEL_FLOW_GRANTED;
}


static void delFlowCnfCb(
    uint32_t flowId,
    B_MPOD_EXT_SERV_TYPE serviceType
    )
{
    BSTD_UNUSED(flowId);
    BSTD_UNUSED(serviceType);

    printf("received %s\n", __FUNCTION__);
}


static void lostFlowIndCb(
    uint32_t flowId,
    B_MPOD_EXT_SERV_TYPE serviceType,
    B_MPOD_EXT_LOST_FLOW_REAS reason
    )
{
    BSTD_UNUSED(flowId);
    BSTD_UNUSED(serviceType);
    BSTD_UNUSED(reason);

    printf("received %s\n", __FUNCTION__);
}


static void rcvFlowDataCb(
    uint32_t flowId,
    B_MPOD_EXT_SERV_TYPE serviceType,
    uint8_t *data,
    uint32_t len
    )
{
    static const char *FlowTypeString[] = {"MPEG", "IP Unicast", "IP Multicast"};

    printf("received %s\n", __FUNCTION__);

    if(flowId > 3)
    {
        printf("ERROR, received flow data for flow type %s from the card\n", FlowTypeString[serviceType]);
    }
    else
    {
        printf("received data for flow type %s, flow id %#x\n", FlowTypeString[serviceType], flowId);
        while(len--) printf("%02x ", *data++);
        printf("\n");
    }
}


#ifdef INTEGRATE_BCM_DSG_HOST

static void rcvDsgDirectoryCb(
    uint8_t *data,
    uint32_t len
    )
{
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);
    printf("received %s\n", __FUNCTION__);
}


#else

static void rcvDsgDirectoryCb(
    B_MPOD_DSG_DIRECTORY *dsgDirectoryInfo
    )
{
    BSTD_UNUSED(dsgDirectoryInfo);

    printf("received %s\n", __FUNCTION__);
}

#endif



/**************
** Feature
**************/
static const B_MPOD_FEATURE_ID TestFeatureList[] =
{
    B_MPOD_FEATURE_ID_RF_OUTPUT_CHANNEL,
    B_MPOD_FEATURE_ID_PARENTIAL_CTL_PIN,
    B_MPOD_FEATURE_ID_PARENTIAL_CTL_SETTING,
    B_MPOD_FEATURE_ID_IPPV_PIN,
    B_MPOD_FEATURE_ID_TIME_ZONE,
    B_MPOD_FEATURE_ID_DAYLIGHT_SAVING,
    B_MPOD_FEATURE_ID_AC_OUTLET,
    B_MPOD_FEATURE_ID_LANGUAGE,
    B_MPOD_FEATURE_ID_RATING_REGION,
    B_MPOD_FEATURE_ID_RESET_PINS,
    B_MPOD_FEATURE_ID_CABLE_URL,
    B_MPOD_FEATURE_ID_EAS_LOCATION_CODE,
    B_MPOD_FEATURE_ID_VCT_ID,
    B_MPOD_FEATURE_ID_TURN_ON_CHANNEL,
    B_MPOD_FEATURE_ID_TERMINAL_ASSOC,
    B_MPOD_FEATURE_ID_DOWNLOAD_GRP_ID,
    B_MPOD_FEATURE_ID_ZIP_CODE
};


static void featureReqHostListCb(
    B_MPOD_FEATURE_ID *hostFeatures,
    uint8_t *hostNumFeatures
    )
{
    *hostNumFeatures = sizeof(TestFeatureList)/sizeof(TestFeatureList[0]);
    BKNI_Memcpy(hostFeatures, TestFeatureList, sizeof(TestFeatureList));
}


static B_MPOD_FEATURE_PARAM TestParams[B_MPOD_FEATURE_ID_MAX]; /* adjust for 0 based below */
static B_MPOD_FEATURE_PARAM *RfOutput          = &TestParams[B_MPOD_FEATURE_ID_RF_OUTPUT_CHANNEL-1];
static B_MPOD_FEATURE_PARAM *ParentalPin       = &TestParams[B_MPOD_FEATURE_ID_PARENTIAL_CTL_PIN-1];
static B_MPOD_FEATURE_PARAM *ParentalSettings  = &TestParams[B_MPOD_FEATURE_ID_PARENTIAL_CTL_SETTING-1];
static B_MPOD_FEATURE_PARAM *PurchasePin       = &TestParams[B_MPOD_FEATURE_ID_IPPV_PIN-1];
static B_MPOD_FEATURE_PARAM *TimeZone          = &TestParams[B_MPOD_FEATURE_ID_TIME_ZONE-1];
static B_MPOD_FEATURE_PARAM *DaylightSavings   = &TestParams[B_MPOD_FEATURE_ID_DAYLIGHT_SAVING-1];
static B_MPOD_FEATURE_PARAM *AcOutlet          = &TestParams[B_MPOD_FEATURE_ID_AC_OUTLET-1];
static B_MPOD_FEATURE_PARAM *Language          = &TestParams[B_MPOD_FEATURE_ID_LANGUAGE-1];
static B_MPOD_FEATURE_PARAM *RatingRegion      = &TestParams[B_MPOD_FEATURE_ID_RATING_REGION-1];
static B_MPOD_FEATURE_PARAM *ResetPin          = &TestParams[B_MPOD_FEATURE_ID_RESET_PINS-1];
static B_MPOD_FEATURE_PARAM *CableUrls         = &TestParams[B_MPOD_FEATURE_ID_CABLE_URL-1];
static B_MPOD_FEATURE_PARAM *EmergencyAlertLoc = &TestParams[B_MPOD_FEATURE_ID_EAS_LOCATION_CODE-1];
static B_MPOD_FEATURE_PARAM *VirtualChannel    = &TestParams[B_MPOD_FEATURE_ID_VCT_ID-1];
static B_MPOD_FEATURE_PARAM *TurnOnChan        = &TestParams[B_MPOD_FEATURE_ID_TURN_ON_CHANNEL-1];
static B_MPOD_FEATURE_PARAM *TerminalAssoc     = &TestParams[B_MPOD_FEATURE_ID_TERMINAL_ASSOC-1];
static B_MPOD_FEATURE_PARAM *CommonDownload    = &TestParams[B_MPOD_FEATURE_ID_DOWNLOAD_GRP_ID-1];
static B_MPOD_FEATURE_PARAM *ZipCode           = &TestParams[B_MPOD_FEATURE_ID_ZIP_CODE-1];

static B_MPOD_FEATURE_VIRTUAL_CHANNEL VirtualChannels[10];
static B_MPOD_FEATURE_CABLE_URL Urls[3];


static void featureReqParamsCb(
    void
    )
{
    uint32_t i;
    int len;

    /* set up the ID's */
    for (i = 0; i < B_MPOD_FEATURE_ID_MAX; i++) TestParams[i].feature = i+1;
    RfOutput->param.rfOutput.channel = 0x3;
    RfOutput->param.rfOutput.channelUi = 0x1;

    ParentalPin->param.parentalPin.chr = "09080706";
    ParentalPin->param.parentalPin.length = 8;

    ParentalSettings->param.parentalSettings.factoryReset = 0;
    ParentalSettings->param.parentalSettings.chanCount = 10;
    ParentalSettings->param.parentalSettings.virtualChannels = VirtualChannels;

    for (i = 0; i < ParentalSettings->param.parentalSettings.chanCount; i++)
    {
        ParentalSettings->param.parentalSettings.virtualChannels[i].channelMajorMinor[2] = i;
        ParentalSettings->param.parentalSettings.virtualChannels[i].channelMajorMinor[1] = i * 5;
        ParentalSettings->param.parentalSettings.virtualChannels[i].channelMajorMinor[0] = i * 10;
    }

    PurchasePin->param.purchasePin.chr = "0504030";
    PurchasePin->param.purchasePin.length = 7;

    TimeZone->param.timeZone.offset = 27;

    DaylightSavings->param.daylightSavings.ctrl = 2; /* use daylight savings */
    DaylightSavings->param.daylightSavings.delta = 128;
    DaylightSavings->param.daylightSavings.entry = 0xffff0000;
    DaylightSavings->param.daylightSavings.exit =0x0000ffff;

    AcOutlet->param.acOutlet.ctrl = 2;

    Language->param.language.ctrl[0] = 'a';
    Language->param.language.ctrl[1] = 'b';
    Language->param.language.ctrl[2] = 'c';

    RatingRegion->param.ratingRegion.region = 1;

    ResetPin->param.resetPin.ctrl = 3;

    CableUrls->param.cableUrls.numberOfUrls = 3;
    CableUrls->param.cableUrls.urls = Urls;

    CableUrls->param.cableUrls.urls[0].length = 23;
    CableUrls->param.cableUrls.urls[0].type = 1; /* Web Portal URL */
    CableUrls->param.cableUrls.urls[0].url = "http://www.broadcom.com";

    CableUrls->param.cableUrls.urls[1].length = 24;
    CableUrls->param.cableUrls.urls[1].type = 2; /* EPG URL */
    CableUrls->param.cableUrls.urls[1].url = "http://epg.broadcomm.com";

    CableUrls->param.cableUrls.urls[2].length = 25;
    CableUrls->param.cableUrls.urls[2].type = 3; /* VOD URL */
    CableUrls->param.cableUrls.urls[2].url = "http://vod.broadcommm.com";

    EmergencyAlertLoc->param.emergencyAlertLoc.stateCode = 101;
    EmergencyAlertLoc->param.emergencyAlertLoc.countySubdivision = 102;
    EmergencyAlertLoc->param.emergencyAlertLoc.countyCode = 103;

    VirtualChannel->param.virtualChannel.vctId = 234;

    TurnOnChan->param.turnOnChan.virtualChannel = 88;
    TurnOnChan->param.turnOnChan.defined  = 1;

    TerminalAssoc->param.terminalAssoc.length = 19;
    TerminalAssoc->param.terminalAssoc.identifier = "This Is My Terminal";

    CommonDownload->param.commonDownload.groupId = 56;

    ZipCode->param.zipCode.chr = "95118-9446";
    ZipCode->param.zipCode.length = 10;

    B_Mpod_FeatureSendUpdatedParams(TestParams, 17);
}


static const char *FeatureString[] =
{
    "dummy", /* features are 1-based */
    "rf_output_channel",
    "parental_control_pin",
    "parental_control_settings",
    "purchase_pin",
    "time_zone",
    "daylight_savings",
    "ac_outlet",
    "language",
    "rating_region",
    "reset_pin",
    "cable_URLs",
    "EAS_location_code",
    "VCT_ID",
    "turn_on_channel",
    "terminal_association",
    "download_group_id",
    "zip_code"
};

static void featureRcvCardListCb(
    B_MPOD_FEATURE_ID *cardFeatures,
    uint8_t cardNumFeatures
    )
{
    uint32_t i;

    printf("%s list of Card supported features\n\n", __FUNCTION__);
    printf("%d\n", cardNumFeatures);
    for(i = 0; i < cardNumFeatures; i++) printf("Feature %d = %s\n", i, FeatureString[cardFeatures[i]]);
    printf("\n\n\n");
}


static void featureRcvParamsCb(
    B_MPOD_FEATURE_PARAM *featureParams,
    uint8_t numFeatures
    )
{
    uint32_t i, j;

    printf("Received the following feature params from the CableCard\n\n");

    for(i = 0; i < numFeatures; i++)
    {
        switch(featureParams[i].feature)
        {
            case B_MPOD_FEATURE_ID_RF_OUTPUT_CHANNEL:
                printf("RF_OUTPUT_CHANNEL\n\n");
                printf("Channel = %d, Channel UI is %s\n",
                featureParams[i].param.rfOutput.channel,
                (featureParams[i].param.rfOutput.channelUi) == 0x1 ? "enabled" : "disabled");
            break;

            case B_MPOD_FEATURE_ID_PARENTIAL_CTL_PIN:
                printf("PARENTIAL_CTL_PIN\n\n");
                featureParams[i].param.parentalPin.chr[featureParams[i].param.parentalPin.length + 1] = '\0';
                printf("Pin is %s\n", featureParams[i].param.parentalPin.chr);
            break;

            case B_MPOD_FEATURE_ID_PARENTIAL_CTL_SETTING:
                printf("PARENTIAL_CTL_SETTING\n\n");
                printf("%s Factory Reset, Channel Count = %d\n",
                (featureParams[i].param.parentalSettings.factoryReset == 0xa7) ? "Perform" : "Don't Perform",
                featureParams[i].param.parentalSettings.chanCount);
                for(j = 0; j < featureParams[i].param.parentalSettings.chanCount; j++)
                {
                    uint16_t major, minor;

                    major = ((featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[0] & 0xf) << 6) |
                            ((featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[1] & 0xfc) >> 2);
                    minor = ((featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[1] & 0x3) << 8) |
                            featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[2];

                    printf("Virtual Channel %d %d included\n", major, minor);
                }
            break;

            case B_MPOD_FEATURE_ID_IPPV_PIN:
                printf("IPPV_PIN\n\n");
                featureParams[i].param.purchasePin.chr[featureParams[i].param.purchasePin.length + 1] = '\0';
                printf("Pin is %s\n", featureParams[i].param.purchasePin.chr);
            break;

            case B_MPOD_FEATURE_ID_TIME_ZONE:
                printf("TIME_ZONE\n\n");
                printf("Time Zone Offset = %d\n", featureParams[i].param.timeZone.offset);
            break;

            case B_MPOD_FEATURE_ID_DAYLIGHT_SAVING:
                printf("DAYLIGHT_SAVING\n\n");
                printf("%s use Daylight Savings\n",
                    (featureParams[i].param.daylightSavings.ctrl == B_MPOD_FEATURE_USE_DST) ? "Do" : "Don't");
                if(featureParams[i].param.daylightSavings.ctrl == B_MPOD_FEATURE_USE_DST)
                {
                    printf("Delta = %d, Entry = %d, Exit = %d\n",
                    featureParams[i].param.daylightSavings.delta,
                    featureParams[i].param.daylightSavings.entry,
                    featureParams[i].param.daylightSavings.exit);
                }
            break;

            case B_MPOD_FEATURE_ID_AC_OUTLET:
            {
                char *ACOutletStrings[] = {"Use User Setting",
                                           "Switched AC Outlet",
                                           "Unswitched AC Outlet",
                                           "Reserved"};
                printf("AC_OUTLET\n\n");
                printf("AC Outlet setting %s\n", ACOutletStrings[featureParams[i].param.acOutlet.ctrl & 0x3]);
            }
            break;

            case B_MPOD_FEATURE_ID_LANGUAGE:
                printf("LANGUAGE\n\n");
                printf("Language code is %d %d %d\n",
                featureParams[i].param.language.ctrl[0],
                featureParams[i].param.language.ctrl[1],
                featureParams[i].param.language.ctrl[2]);
            break;

            case B_MPOD_FEATURE_ID_RATING_REGION:
                printf("RATING_REGION\n\n");
                printf("Rating Region is %d\n",
                featureParams[i].param.ratingRegion.region);
            break;

            case B_MPOD_FEATURE_ID_RESET_PINS:
            {
                char * resetPinsString[] = {"Don't reset any pin",
                                            "Reset parental control pin",
                                            "Reset purchase pin",
                                            "Reset parental control and purchase pin"};
                printf("RESET_PINS\n\n");
                printf("Reset Pin Setting is %s\n",
                resetPinsString[featureParams[i].param.resetPin.ctrl & 0x3]);
            }
            break;

            case B_MPOD_FEATURE_ID_CABLE_URL:
            {
                printf("CABLE_URL\n\n");

                /* populate the array with type, length and the pointer to the url */
                for (j = 0; j < featureParams[i].param.cableUrls.numberOfUrls; j++)
                {
                    char *urlTypeString[] = {"undefined", "Web Portal URL", "EPG URL", "VOD URL"};

                    featureParams[i].param.cableUrls.urls[j].url[featureParams[i].param.cableUrls.urls[j].length + 1] = '\0';

                    printf("Type %s, URL = %s\n",
                    urlTypeString[featureParams[i].param.cableUrls.urls[j].type & 0x3],
                    featureParams[i].param.cableUrls.urls[j].url);
                }
            }
            break;

            case B_MPOD_FEATURE_ID_EAS_LOCATION_CODE:
                printf("EAS_LOCATION_CODE\n\n");
                printf("State Code = %d, County Subdvsn = %d, County Code = %d\n",
                featureParams[i].param.emergencyAlertLoc.stateCode,
                featureParams[i].param.emergencyAlertLoc.countySubdivision,
                featureParams[i].param.emergencyAlertLoc.countyCode);
            break;

            case B_MPOD_FEATURE_ID_VCT_ID:
                printf("VCT_ID\n\n");
                printf("Virtual Channel ID = %d\n",
                    featureParams[i].param.virtualChannel.vctId);
            break;

            case B_MPOD_FEATURE_ID_TURN_ON_CHANNEL:
                printf("TURN_ON_CHANNEL\n\n");
                printf("Turn-On Channel %d is %s\n", featureParams[i].param.turnOnChan.virtualChannel,
                featureParams[i].param.turnOnChan.defined ? "defined" : "not defined");
            break;

            case B_MPOD_FEATURE_ID_TERMINAL_ASSOC:
                printf("TERMINAL_ASSOCIATION\n\n");
                featureParams[i].param.terminalAssoc.identifier[featureParams[i].param.terminalAssoc.length + 1] = '\0';
                printf("Terminal Association ID = %s\n", featureParams[i].param.terminalAssoc.identifier);
            break;

            case B_MPOD_FEATURE_ID_DOWNLOAD_GRP_ID:
                printf("DOWNLOAD_GROUP_ID\n\n");
                printf("Download Group ID is %d\n", featureParams[i].param.commonDownload.groupId);
            break;

            case B_MPOD_FEATURE_ID_ZIP_CODE:
                printf("ZIP_CODE\n\n");
                featureParams[i].param.zipCode.chr[featureParams[i].param.zipCode.length + 1] = '\0';
                printf("Zip Code is %s\n", featureParams[i].param.zipCode.chr);
            break;

            default:
                printf("What kind of feature is that??\n");
            break;
        }

        featureParams[i].featureStatus = 0; /* feature param accepted */

        printf("\n\n\n");

    }
}


static void featureParamDeniedCb(
    B_MPOD_FEATURE_ID feature,
    B_MPOD_FEATURE_STATUS status
    )
{
    printf("Param %s denied with status %d",FeatureString[feature], status );
}



/**************
** Headend Comm
**************/
void rcvHostResetVectorCb(
    B_MPOD_HEADEND_HOST_RESET_VECTOR *hostResetVector
    )
{
    printf("%s\n\n", __FUNCTION__);

    printf("Delay = %d\n", hostResetVector->delay);

    printf("resetEcm = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_ECM) ? "true" : "false");
    printf("resetSecurityElem = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_SECURITY_ELEM) ? "true" : "false");
    printf("resetHost = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_HOST) ? "true" : "false");
    printf("resetExternalDevices = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_EXTERNAL_DEVICES) ? "true" : "false");
    printf("resetAll = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_ALL) ? "true" : "false");

    printf("restartOcapStack = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESTART_FIELD(hostResetVector, B_MPOD_HEADEND_RESTART_OCAP_STACK) ? "true" : "false");
    printf("restartAll = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESTART_FIELD(hostResetVector, B_MPOD_HEADEND_RESTART_ALL) ? "true" : "false");

    printf("reloadAllOcapApps = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RELOAD_APP_FIELD(hostResetVector, B_MPOD_HEADEND_RELOAD_ALL_OCAP_APPS) ? "true" : "false");
    printf("reloadOcapStack = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RELOAD_APP_FIELD(hostResetVector, B_MPOD_HEADEND_RELOAD_OCAP_STACK) ? "true" : "false");

    printf("reloadHostFirmware = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RELOAD_FW_FIELD(hostResetVector, B_MPOD_HEADEND_RELOAD_HOST_FIRMWARE) ? "true" : "false");

    printf("clearPersistentGetFeatParams = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_PERSISTENT_GEN_FEAT_PARAMS) ? "true" : "false");
    printf("clearOrgDvbPersistentFs = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_ORG_DVB_PERSISTENT_FS) ? "true" : "false");
    printf("clearCachedUnboundApps = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_CACHED_UNBOUND_APPS) ? "true" : "false");
    printf("clearRegisteredLibs = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_REGISTERED_LIBS) ? "true" : "false");
    printf("clearPersistentHostMem = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_PERSISTENT_HOST_MEM) ? "true" : "false");
    printf("clearSecElemPassedValues = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_SEC_ELEM_PASSED_VALUES) ? "true" : "false");
    printf("clearNonAsdDvrContent = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_NON_ASD_DVR_CONTENT) ? "true" : "false");
    printf("clearAsdDvrContent = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_ASD_DVR_CONTENT) ? "true" : "false");
    printf("clearNetworkDvrContent = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_NETWORK_DVR_CONTENT) ? "true" : "false");
    printf("clearMediaVolInternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_MEDIA_VOL_INTERNAL_HDD) ? "true" : "false");
    printf("clearMediaVolExternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_MEDIA_VOL_EXTERNAL_HDD) ? "true" : "false");
    printf("clearGpfsInternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_GPFS_INTERNAL_HDD) ? "true" : "false");
    printf("clearGpfsExternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_GPFS_EXTERNAL_HDD) ? "true" : "false");
    printf("clearAllStorage = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_ALL_STORAGE) ? "true" : "false");

    printf("\n");
}



/**************
** Homing
**************/
static void homingTimeoutCb(
    void
    )
{
    printf("received %s\n", __FUNCTION__);
    printf("Resetting CableCard\n");
}


static void delayedDownloadReqCb(
    void
    )
{
    printf("received %s\n", __FUNCTION__);
}


static void homingCompleteCb(
    void
    )
{
    printf("received %s\n", __FUNCTION__);
}


/* the host should NOT interrupt the download */
static void downloadStartingCb(
    B_MPOD_HOMING_DOWNLOAD_INFO *downloadInfo
    )
{
    char notifyString[257];
    char *sourceStrings[] = {"unknown", "QAM Inband", "QPSK OOB", "reserved"};
    char *timeoutTypeStrings[] = {"both timeouts", "transport timeout",
                                            "download timeout", "no_timeout"};

    printf("received %s\n", __FUNCTION__);

    if(downloadInfo->notifyTextLength)
    {
        strncpy(notifyString, downloadInfo->notifyText, downloadInfo->notifyTextLength);
        notifyString[downloadInfo->notifyTextLength] = '\0';
        printf("Notify Message: %s\n", notifyString);
    }

    printf("Upgrade Source: %s,  Download Time %d\n",
            sourceStrings[downloadInfo->source & 0x3],
            downloadInfo->downloadTime);

    printf("Timeout Type: %s,  Timeout Period: %d\n",
            timeoutTypeStrings[downloadInfo->timeoutType & 0x3],
            downloadInfo->downloadTimeoutPeriod);
}


static void downloadCompleteCb(
    B_MPOD_HOMING_UPGRADE_RESET_REQUEST resetType
    )
{
    char *resetTypeStrings[] = {"PCMCIA Reset", "Card Reset", "No Reset", "Reserved"};

    printf("received %s\n", __FUNCTION__);
    printf("requested %s\n", resetTypeStrings[resetType & 0x3]);
}


static void printHomingMenu(
    void
    )
{
    printf("MPOD Homing Test Menu:\n");
    printf("1 : Simulate 'Go To Standby' - send open_homing().\n");
    printf("2 : Cancel homing (send homing_cancelled()\n");

    printf("0 : Quit this menu.\n");
}


static void mpodHomingTest(
    void
    )
{
    char cmd[256];
    uint32_t end_loop = 0;


    while (!end_loop)
    {
        printHomingMenu();

        printf("\nEnter Option:");
        cmd[0] = GetInputChar();
        printf("\n");

        switch(cmd[0])
        {
            case '1':
                B_Mpod_HomingOpen();
                printf("Sent open_homing() to the Card\n");\
            break;

            case '2':
            {
                B_Mpod_HomingCancel();
                printf("Cancelling Homing\n");
            }
            break;

            case '0':
                end_loop = 1;
            break;
        }
    }
}



/**************
** Host Control
**************/
static B_MPOD_HC_IB_TUNE_STATUS inbandTuneCb(
    uint32_t freqHz,
    B_MPOD_HC_IB_MOD_TYPE modulation,
    uint8_t *ltsid
    )
{
    char *modStrings[] = {"QAM64","QAM256"};
    printf("received %s\n", __FUNCTION__);

    printf("Freq: %dHz  modulation: %s\n",
            freqHz, modStrings[modulation & 0x1]);

    if(B_Mpod_HomingIsHomingActive())
    {
        *ltsid = 0x80;
        return B_MPOD_HC_IB_TUNE_ACCEPTED;
    }

    return B_MPOD_HC_IB_TUNER_BUSY;
}


static B_MPOD_HC_OOB_TX_TUNE_STATUS oobTxTuneCb(
    uint32_t freqHz,
    uint32_t powerLevel,
    uint32_t rate
    )
{
    printf("received %s\n", __FUNCTION__);
    printf("Freq: %dHz  PowerLevel: %d.%ddBmV  Rate: %dbaud\n",
            freqHz, powerLevel >> 1, (powerLevel & 0x1) ? 5 : 0, rate);

    return B_MPOD_HC_OOB_TX_TUNE_GRANTED;
}


static B_MPOD_HC_OOB_RX_TUNE_STATUS oobRxTuneCb(
    uint32_t freqHz,
    uint32_t rate,
    bool spectralInv
    )
{
    printf("received %s\n", __FUNCTION__);
    printf("Freq: %dHz  Rate: %dbps  Spectral Inversion: %s\n",
            freqHz, rate, spectralInv ? "true" : "false");

    return B_MPOD_HC_OOB_RX_TUNE_GRANTED;
}


static void sourceIdToFreqCb(
    uint16_t sourceId,
    uint32_t *freqHz,
    B_MPOD_HC_IB_MOD_TYPE *modulation
    )
{
    BSTD_UNUSED(sourceId);

    printf("received %s\n", __FUNCTION__);
    *freqHz = 103000000;
    *modulation = B_MPOD_HC_IB_QAM256;
}



/*****************
** Host Properties
*****************/
void hostPropertiesReplyCb(
    B_MPOD_PROP_HOST_PROPS *hostProperties
    )
{
    int i, j;

    printf("%s\n\n", __FUNCTION__);
    printf("%d properties sent from the card\n\n", hostProperties->numOfProperties);

    for(i = 0; i < hostProperties->numOfProperties; i++)
    {
        printf("Key: ");
        for(j = 0; j < hostProperties->properties[i].keyLength; j++)
            printf("%02x ", hostProperties->properties[i].keyByte[j]);

        printf("= ");
        for(j = 0; j < hostProperties->properties[i].valueLength; j++)
            printf("%02x ", hostProperties->properties[i].valueByte[j]);

        printf("\n");
    }
}



/***************
** Card Resource
***************/
void resProfileInfoCb(
    uint8_t numStreams,
    uint8_t numProgs,
    uint8_t numEs
    )
{
   printf("%s streams=%d  progs=%d  elem streams=%d\n", __FUNCTION__, numStreams, numProgs, numEs);
}



/**************
** SAS
**************/
static void printSasMenu(
    void
    )
{
    printf("POD SAS Test Menu:\n");
    printf("1 : send SAS connect.\n");
    printf("2 : send SAS data rqst.\n");
    printf("3 : send SAS data with syncrhonous handshake.\n");
    printf("4 : send SAS data asynchronously.\n");
    printf("5 : register external handler for private app.\n");
    printf("6 : unregister external handler for private app.\n");
    printf("0 : Quit this menu.\n");
}


static void sasExternalHandlerGoCallback(
    uint8_t *privateAppId,
    uint16_t sessionNb,
    bool connected
    )
{
    BSTD_UNUSED(sessionNb);

    printf("\nGot Ext Handler go for private app %02x %02x %02x %02x %02x %02x %02x %02x\n",
        privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]);
}


static void sasExternalRcvCallback(
    uint8_t *privateAppId,
    uint16_t sessionNb,
    uint8_t *data,
    uint32_t tag,
    uint32_t len
    )
{
    char *ApduDesc[] =
    {
        "",
        "",
        "data_rqst()",
        "data_av()",
        "data_av_cnf()",
        "server_query()",
        "server_reply()",
        "async_msg()"
    };

    BSTD_UNUSED(data);
    BSTD_UNUSED(len);

    printf("\nGot %s APDU for private app %02x %02x %02x %02x %02x %02x %02x %02x\n",
        ApduDesc[tag & 0xf], privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]);
}


/* tells the app that the requested connection for privateAppId has been established */
static void sasConnectCnfCallback(
    uint8_t *privateAppId,
    B_MPOD_SAS_HANDLE newSasConnection
    )
{
    int i;

    for(i = 0; i < 32; i++)
    {
        if(SasConnections[i] == 0)
        {
            SasConnections[i] = newSasConnection;
            printf("SAS connection %d assigned for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    i, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
                    privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]);
            return;
        }
    }

    printf("Unable to open new SAS Connection. Connection limit has been reached\n");
}


/* tells the app that the card is ready for syncrhonous communication for this privateAppId */
static void sasConnectionRdyCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId
    )
{

    printf("Recieved connection rdy for connection %#x assigned for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
        (uint32_t)sasConnection, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    );
}


/* delivers data to the application from the card */
static void sasSynchDataRcvCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId,
    uint8_t *msg,
    uint32_t len
    )
{
    uint32_t i = 0;

    BSTD_UNUSED(sasConnection);

    printf("Received new data through synchronous transmission from private app %02x %02x %02x %02x %02x %02x %02x %02x\n",
        privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    );

    printf("msg: \n");
    while(len--) printf("%x ", msg[i++]);
    printf("\n");
}


/* delivers data to the application from the card */
static void sasAsynchDataRcvCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId,
    uint8_t *msg,
    uint32_t len)
{
    uint32_t i = 0;

    BSTD_UNUSED(sasConnection);

    printf("Received new data through asynchronous transmission from private app %02x %02x %02x %02x %02x %02x %02x %02x\n",
        privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    );

    printf("msg: \n");
    while(len--) printf("%x ", msg[i++]);
    printf("\n");

}


/* retrieves data from the app to be sent to the card (app previously requested a syncrhonous transfer of data) */
static void sasGetSynchDataCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId,
    uint8_t transactionNb,
    uint8_t **sasMsg,
    uint32_t *sasLen
    )
{
    BSTD_UNUSED(sasConnection);

    printf("Received request for data for transaction number %d for private app %02x %02x %02x %02x %02x %02x %02x %02x\n",
        transactionNb, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    );

    /* Preliminary handshake is over, allow the data to be transmitted */
    *sasMsg = SasData;
    *sasLen = 8;
}


/* tells the app that the card has closed the connection for privateAppId */
static void sasConnectionClosedCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId
    )
{
    int i;

    for(i = 0; i < 32; i++)
    {
        if(SasConnections[i] == sasConnection)
        {
            SasConnections[i] = 0;
            printf("Closing SAS connection %#x assigned for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    (uint32_t)sasConnection, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
                    privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
            );
            return;
        }
    }

    printf("Unable to close SAS Connection. Connection not found\n");
}


static void mpodSasTest(
    void
    )
{
    char cmd[256], inputs[256];
    unsigned long end_loop = 0,i;
    uint32_t temp;
    uint8_t connectionNb, transactionNb;
    unsigned char appid[8];


    while (!end_loop)
    {
        printSasMenu();

        printf("\nEnter Option:\n");
        cmd[0] = GetInputChar();
        printf("\n");

        switch (cmd[0])
        {
            case '1': /* Open SAS connection */

                printf("enter the Private SAS App ID MSB-LSB separated by CR in hex:\n");

                for (i=0; i<8; i++)
                {
                    GetInputString(inputs);
                    sscanf(inputs,"%x",&temp);
                    appid[i] = (unsigned char)temp;
                    printf("\n");
                }

                printf("Requesting SAS connection for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    appid[0], appid[1], appid[2], appid[3], appid[4], appid[5], appid[6], appid[7]);

                if(B_Mpod_SasConnectRqst(appid))
                {
                    printf("ERROR requesting new SAS connection\n");
                }

            break;

            case '2': /* Send Rdy To Recv */
                do
                {
                    printf("enter valid SAS connection number:\n");
                    GetInputString(inputs);
                    sscanf(inputs,"%u",&temp);
                    connectionNb = (uint8_t)temp;
                    printf("\n");
                }
                while((connectionNb > 31) || (!SasConnections[connectionNb]));

                if(B_Mpod_SasConnectionRdyToRcv(SasConnections[connectionNb]))
                {
                    printf("ERROR setting Rdy To Recv\n");
                }
            break;

            case '3': /* Send data using synchronous transmission */
                do
                {
                    printf("enter valid SAS connection number:\n");
                    GetInputString(inputs);
                    sscanf(inputs,"%u",&temp);
                    connectionNb = (uint8_t)temp;
                    printf("\n");
                }
                while((connectionNb > 31) || (!SasConnections[connectionNb]));

                if(B_Mpod_SasSendSynchMsg(SasConnections[connectionNb], &transactionNb))
                {
                    printf("ERROR sending SAS data synchronously\n");
                }
            break;


            case '4': /* Send data using asynchronous transmission */
                do
                {
                    printf("enter valid SAS connection number:\n");
                    GetInputString(inputs);
                    sscanf(inputs,"%u",&temp);
                    connectionNb = (uint8_t)temp;
                    printf("\n");
                }
                while((connectionNb > 31) || (!SasConnections[connectionNb]));

                if(B_Mpod_SasSendAsynchMsg(SasConnections[connectionNb], SasData, 8))
                {
                    printf("ERROR sending SAS data asynchronously\n");
                }
            break;

            case '5': /* unregister external handler for private app */

                printf("enter the Private SAS App ID MSB-LSB separated by CR in hex:\n");

                for (i=0; i<8; i++)
                {
                    GetInputString(inputs);
                    sscanf(inputs,"%x",&temp);
                    appid[i] = (unsigned char)temp;
                    printf("\n");
                }

                printf("Registering external handler for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    appid[0], appid[1], appid[2], appid[3], appid[4], appid[5], appid[6], appid[7]);

                if(B_Mpod_SasRegisterExternalHandler(appid))
                {
                    printf("ERROR registering external handler for private app\n");
                }

            break;


            case '6': /* unregister external handler for private app */

                printf("enter the Private SAS App ID MSB-LSB separated by CR in hex:\n");

                for (i=0; i<8; i++)
                {
                    GetInputString(inputs);
                    sscanf(inputs,"%x",&temp);
                    appid[i] = (unsigned char)temp;
                    printf("\n");
                }

                printf("Unregistering external handler for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    appid[0], appid[1], appid[2], appid[3], appid[4], appid[5], appid[6], appid[7]);

                if(B_Mpod_SasUnregisterExternalHandler(appid))
                {
                    printf("ERROR unregistering external handler for private app\n");
                }

            break;

            case '0':
                end_loop = 1;
            break;
        }
    }
}



/**************
** SNMP
**************/
void snmpRcvRootOidCb(
    uint8_t *data,
    uint32_t len
    )
{
    uint32_t i;

    printf("%s\n", __FUNCTION__);

    for (i = 0; i < len; i++) printf("%d ", data[i]);
    printf("\n");
}


void snmpRcvReplyCb(
    uint8_t *data,
    uint32_t len
    )
{
    uint32_t i;

    printf("%s\n", __FUNCTION__);

    for (i = 0; i < len; i++) printf("%d ", data[i]);
    printf("\n");
}



/**************
** General
**************/
void cardInsertedCb(
    void
    )
{
    printf("%s\n", __FUNCTION__);
}

void cardRemovedCb(
    void
    )
{
    printf("%s\n", __FUNCTION__);
}

void cardErrorCb(
    B_MPOD_IF_ERROR error
    )
{
    char *strings[] = {"B_MPOD_IF_ERROR_CARD_UNKNOWN","B_MPOD_IF_ERROR_CARD_RCV","B_MPOD_IF_ERROR_CARD_TX"};
    BDBG_ASSERT(error < B_MPOD_IF_ERROR_CARD_MAX);

    printf("%s %s\n", __FUNCTION__, strings[error]);
}

void cardResetCb(
    void
    )
{
    printf("%s\n", __FUNCTION__);
}

static void printMainMenu(
    void
    )
{
    printf("MPOD Test Menu:\n");
    printf("1 : Reset the MPOD module. \n");
    printf("2 : Show All Opened Sessions. \n");
    printf("3 : Apinfo & MMI menu.\n");
    printf("4 : Extended Channel menu.\n");
    printf("5 : Homing Menu. \n");
    printf("6 : SAS menu\n");
    printf("7 : send CA_PMT.\n");
    printf("8 : send Download host_dl_ctrl.\n");
    printf("q : Close All Connections and Quit.\n");
}


#if BDBG_DEBUG_BUILD
/* taken directly from bsettop_board.c */
static void setModuleDebugLevel(const char *modulelist, BDBG_Level level)
{

	while (modulelist && *modulelist) {
		const char *end = strchr(modulelist, ',');
		if (!end) {
			BDBG_SetModuleLevel(modulelist, level);
			break;
		}
		else {
			int n = end-modulelist;
			char *buf = (char *)BKNI_Malloc(n+1);
			/* NOTE: memory leak, but this is debug */
			strncpy(buf, modulelist, n);
			buf[n] = 0;
			BDBG_SetModuleLevel(buf, level);
		}
		modulelist = ++end;
	}
}
#endif


int main(
    void
    )
{
    char cmd[256];
    unsigned long end_loop = 0;
    int err;
    MPOD_RET_CODE ret;

    B_ThreadHandle flowReqThread;


    B_MPOD_APINFO_SETTINGS apInfoSettings;

    B_MPOD_MMI_SETTINGS mmiSettings = {
        &mmiDialogRequestCb,
        &mmiDialogCloseCb,
        &mmiHtmlRdyCb,
        &mmiExternalHandlerRdy,
        &mmiAPDURcvCb,
        &mmiSessionOpenCb,
        &mmiSessionCloseCb
        };

    B_MPOD_SAS_SETTINGS sasSettings = {
        &sasConnectCnfCallback,
        &sasConnectionRdyCallback,
        &sasSynchDataRcvCallback,
        &sasAsynchDataRcvCallback,
        &sasGetSynchDataCallback,
        &sasExternalHandlerGoCallback,
        &sasExternalRcvCallback,
        &sasConnectionClosedCallback
        };

    B_MPOD_DL_SETTINGS dlSettings = {
        &dlHostInfoCb,
        &dlNewCodeVersionTableCb,
        &dlNewCodeVersionTable2Cb
        };

    B_MPOD_FEATURE_SETTINGS featureSettings = {
        &featureReqHostListCb,
        &featureReqParamsCb,
        &featureRcvCardListCb,
        &featureRcvParamsCb,
        &featureParamDeniedCb
        };

    B_MPOD_EXT_CH_SETTINGS extChSettings = {
        &reqIpUnicastFlowCb,
        &reqIpMulticastFlowCb,
        &reqDsgFlowCb,
        &reqSocketFlowCb,
        flowReqFailedCb,
        &newFlowCnfCb,
        &delFlowReqCb,
        &delFlowCnfCb,
        &lostFlowIndCb,
        &rcvSetDsgModeCb,
        &dsgErrorCb,
        &configAdvDsgCb,
        &rcvFlowDataCb
        };

    B_MPOD_DSG_SETTINGS dsgSettings = {
        &dsgErrorCb,
        &rcvSetDsgModeCb,
        &rcvDsgDirectoryCb
        };

    B_MPOD_HOST_CONTROL_SETTNINGS hostControlSettings = {
        &inbandTuneCb,
        &oobTxTuneCb,
        &oobRxTuneCb,
        &sourceIdToFreqCb
        };

    B_MPOD_HOMING_SETTINGS homingSettings = {
        &homingTimeoutCb,
        &delayedDownloadReqCb,
        &homingCompleteCb,
        &downloadStartingCb,
        &downloadCompleteCb
        };

    B_MPOD_DIAG_SETTINGS diagSettings = {
        &diagReqCb
        };

    B_MPOD_SYSTIME_SETTINGS systimeSettings = {NULL};

    B_MPOD_RES_SETTINGS resSettings = {
        5 /* max simul xpt streams*/,
        &resProfileInfoCb
        };

    B_MPOD_CA_SETTINGS caSettings = {
        &caInfoCb,
        &caPmtUpdate,
        &caPmtReply
        };

    B_MPOD_CP_SETTINGS cpSettings = {
        &getAuthKeyCb,
        &cardAuthMsgCb,
        &getNonceCb,
        &cpkeyGenCb,
        &removeKeyCb,
        &progKeyCb,
        &calcCciAckCb,
        &enforceCciCb,
        &newValidationStatusCb,
        &getIDCb
        };

    B_MPOD_SNMP_SETTINGS snmpSettings = {
        snmpRcvRootOidCb,
        snmpRcvReplyCb
        };

    B_MPOD_HEADEND_COMM_SETTINGS headendCommSettings = {
        &rcvHostResetVectorCb
        };

    B_MPOD_PROP_HOST_PROPS_SETTINGS hostPropertySettings = {
        &hostPropertiesReplyCb
        };

    B_MPOD_IF_SETTINGS ifSettings = {
        &cardInsertedCb,
        &cardRemovedCb,
        &cardErrorCb,
        &cardResetCb
        };


    BKNI_Init();
    BDBG_Init();

#if BDBG_DEBUG_BUILD
	BDBG_SetLevel(BDBG_eWrn);

	setModuleDebugLevel(getenv("trace_modules"), BDBG_eTrace);
	setModuleDebugLevel(getenv("msg_modules"), BDBG_eMsg);
	setModuleDebugLevel(getenv("wrn_modules"), BDBG_eWrn);
#endif

    B_Os_Init();

#ifdef ESTB_CFG_SUPPORT
	/*
	 *  load the estb_cfg
	 */
	B_Estb_cfg_Init("/perm");
	B_Estb_cfg_Init("/dyn");
	B_Estb_cfg_Open("/perm", "./perm.bin");
	B_Estb_cfg_Open("/dyn", "./dyn.bin");
#endif


    fppod = MPOD_open("/dev/pod");
    if(fppod<0)
    {
        BDBG_ERR(("Could not open MPOD Device!!!"));
        return fppod;
    }

    printf("fppod %d\n", fppod);

    B_Mpod_Init(&ifSettings);

    if((flowReqEvent = B_Event_Create(NULL)) == NULL)
    {
        printf("Unable to create flow req event\n");
        exit(1);
    }

    if((flowReqThread = B_Thread_Create(NULL, handleFlowReq, (void *)0, NULL)) == NULL)
    {
        printf("Unable to create flow request thread\n");
        exit(1);
    }


    /* clear out dialog tracking array */
    BKNI_Memset(dialogs, 0, sizeof(dialogs));
    BKNI_Memset(SasConnections, 0, sizeof(SasConnections));

    /* set up ApInfo resource */
    B_Mpod_AppInfoGetDefaultCapabilities(&apInfoSettings);
    apInfoSettings.apInfoExternalHandlerRcvCb   = &apInfoAPDURcvCb;
    apInfoSettings.apInfoChangedCb              = &apInfoInfoChangedCb;



    ret = MPOD_SUCCESS;
    ret |= B_Mpod_ResrcMgrInit();
    ret |= B_Mpod_AppInfoInit(&apInfoSettings);
    ret |= B_Mpod_MmiInit(&mmiSettings);
    ret |= B_Mpod_SasInit(&sasSettings);
    ret |= B_Mpod_DownloadInit(&dlSettings);
    ret |= B_Mpod_FeatureInit(&featureSettings);
    ret |= B_Mpod_ExtendedChInit(&extChSettings);
    ret |= B_Mpod_DsgInit(&dsgSettings);
    ret |= B_Mpod_HostControlInit(&hostControlSettings);
    ret |= B_Mpod_HomingInit(&homingSettings);
    ret |= B_Mpod_DiagsInit(&diagSettings);
    ret |= B_Mpod_SystimeInit(&systimeSettings);
    ret |= B_Mpod_CaInit(&caSettings);
    ret |= B_Mpod_CpInit(&cpSettings);
    ret |= B_Mpod_ResInit(&resSettings);
    ret |= B_Mpod_SnmpInit(&snmpSettings);
    ret |= B_Mpod_HeadendCommInit(&headendCommSettings);
    ret |= B_Mpod_HostPropertiesInit(&hostPropertySettings);

    if(ret != MPOD_SUCCESS)
    {
        printf("Unable to initialize Host resources\n");
        exit(1);
    }

    B_Mpod_TestCpInit();
    B_Mpod_Go(fppod);

    while (!end_loop)
    {
        printMainMenu();

        printf("\nEnter Option:");
        cmd[0] = GetInputChar();
        printf("\n");

        switch (cmd[0])
        {

            case '1':
            {
                MPOD_LINK_CONN *link_conn = &MPOD_LinkConn;
                linkConnStop();

                BDBG_ERR(("Reseting the MPOD interface ..."));
                err = ioctl(link_conn->driver,IOCTL_MPOD_INTERF_RESET, 0);
                if (err<0)
                {
                    BDBG_ERR(("IOCTL_POD_INTERF_RESET failed!"));
                    break;
                }
                linkConnStart(link_conn);

            }
            break;

            case '2':
                B_Mpod_SessionShowOpenSessions();
            break;

            case '3':
                mpodApInfoMmiTest();
            break;

            case '4':
                mpodExtChanTest();
            break;

            case '5':
                mpodHomingTest();
            break;

            case '6':
                mpodSasTest();
            break;


            case '7':
                caPmtTest();
            break;

            case '8':
            {
                uint32_t temp;
                uint8_t hostCommand;
                char inputs[256];

                do
                {
                    printf("enter host command [0-4]:\n");
                    printf("0 - Download Started\n");
                    printf("1 - Download Completed\n");
                    printf("2 - Notify Headend\n");
                    printf("3 - Download Max Retry\n");
                    printf("4 - Image Damaged\n");
                    printf("5 - Certificate Failure\n");
                    printf("6 - Reboot Max Retry\n");

                    GetInputString(inputs);
                    sscanf(inputs,"%u",&temp);
                    hostCommand = (uint8_t)temp;
                    printf("\n");
                }
                while(hostCommand > 6);
                B_Mpod_DownloadSendHostDlCtrl((B_MPOD_DL_HOST_COMMAND)hostCommand);
            }

            break;

            case 'q':
               goto done;

            default:
            break;
        }
    }

done:

    B_Mpod_Done(fppod);
    B_Mpod_TestCpDone();
    B_Mpod_AppInfoShutdown();
    B_Mpod_MmiShutdown();
    B_Mpod_SasShutdown();
    B_Mpod_DownloadShutdown();
    B_Mpod_FeatureShutdown();
    B_Mpod_ExtendedChShutdown();
    B_Mpod_DsgShutdown();
    B_Mpod_HostControlShutdown();
    B_Mpod_HomingShutdown();
    B_Mpod_DiagsShutdown();
    B_Mpod_SystimeShutdown();
    B_Mpod_CaShutdown();
    B_Mpod_CpShutdown();
    B_Mpod_SnmpShutdown();
    B_Mpod_HeadendCommShutdown();
    B_Mpod_HostPropertiesShutdown();

    ThreadExit = true;
    B_Event_Set(flowReqEvent);

    close(fppod);

#ifdef ESTB_CFG_SUPPORT
	/*
	 *  load the estb_cfg
	 */
	B_Estb_cfg_Close("/perm");
	B_Estb_cfg_Close("/dyn");
	B_Estb_cfg_Uninit("/perm");
	B_Estb_cfg_Uninit("/dyn");
#endif
    BDBG_MSG(("Finished!!!"));
    return 0;
}
