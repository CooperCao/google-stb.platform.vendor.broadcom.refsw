/***************************************************************************
 *     Copyright (c) 2009-2013, Broadcom Corporation
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
 ****************************************************************************/
#include "dvr_test.h"

#define All_DvrTestMediaStorage 0x00000001
#define All_DvrTestTsb          0x00000002
#define All_DvrTestUtility      0x00000004
#define All_DvrTestRecord       0x00000008
#define All_DvrTestPlayback     0x00000010
#define All_DvrTestHn           0x00000020
#define All_DvrTestDrm           0x00000040

BDBG_MODULE(dvr_auto_test);
/* global variables */
DvrTest_ChannelInfo channelInfo[MAX_STATIC_CHANNELS] =
{
    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,429000000,"hockey",1,1,{0x11},
    {0x14},{0x11},{NEXUS_VideoCodec_eMpeg2},{NEXUS_AudioCodec_eAc3}},

    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,435000000,"color beat",1,1,{0x11},
    {0x14},{0x11},{NEXUS_VideoCodec_eMpeg2},{NEXUS_AudioCodec_eAc3}},

    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,447000000,"hockey match", 3,3,{0x11,0x21,0x41},
    {0x14,0x24,0x44},{0x11,0x21,0x41},{NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2},
    {NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3}},
    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e256,5360537,525000000,"sc_bugs_baywatch",3,3,{0x31,0x11,0x21},
    {0x34,0x14,0x24},{0x31,0x11,0x21},{NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2},
    {NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3}},
};
DvrTestHandle g_dvrTest;


/* static variables */
static char testlist_fn[] = "testlist.txt";
B_DVR_ERROR dvrError = B_DVR_SUCCESS;
unsigned volumeIndex;
B_DVR_MediaStorageStatus mediaStorageStatus;

/* global definition */
extern int g_CuTestDryRun;

CuString *g_cutest_output = NULL;
CuSuite* g_cutest_suite = NULL;
extern void DvrTestInit(CuTest *);
extern void DvrTestExit(CuTest * tc);
extern void DvrTestMediaStorageMountVolume(CuTest *);
extern void DvrTestMediaStorageFormatVolume(CuTest *);
extern void hnMulPFToMulStreamNetTest(CuTest * tc);



void DvrTestMediaStorageMountVolume(CuTest* tc) 
{

    int i;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    BDBG_MSG(("%s:", __FUNCTION__));

    system("umount /dev/sda1");
    system("umount /dev/sda2");
    system("umount /dev/sda3"); 

    if (g_dvrTest) {
        BKNI_Free(g_dvrTest);
    }
    g_dvrTest = BKNI_Malloc(sizeof(*g_dvrTest));
    BKNI_Memset(g_dvrTest,0,sizeof(*g_dvrTest));

    mediaStorageOpenSettings.storageType = eB_DVR_MediaStorageTypeBlockDevice;
    g_dvrTest->mediaStorage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    CuAssertPtrNotNullMsg(tc, "B_DVR_MediaStorage_Open failed", g_dvrTest->mediaStorage);

    volumeIndex=0;

    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);
    /* mount volume */
    volumeIndex=0;

    /* check volume status */
    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);     
    if (mediaStorageStatus.numMountedVolumes >= 1) {
        BDBG_WRN(("%s: Unmount volume %d first\n", __FUNCTION__, volumeIndex));
        dvrError = B_DVR_MediaStorage_UnmountVolume(g_dvrTest->mediaStorage, volumeIndex);
        CuAssertTrue(tc, B_DVR_SUCCESS == dvrError);
    }
    BDBG_WRN(("%s: Mount volume\n", __FUNCTION__));
    BDBG_WRN(("Volume index %d", volumeIndex));
    dvrError = B_DVR_MediaStorage_MountVolume(g_dvrTest->mediaStorage, volumeIndex);

    CuAssertTrue(tc, B_DVR_SUCCESS == dvrError);

    /* check volume status */
    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);     
    BDBG_MSG(("Number of registered volumes:%d\n", mediaStorageStatus.numRegisteredVolumes));
    BDBG_MSG(("Number of mounted volumes:%d\n", mediaStorageStatus.numMountedVolumes));
    for (i=0; i<B_DVR_MEDIASTORAGE_MAX_VOLUME; i++) 
    {
        if (!mediaStorageStatus.volumeInfo[i].registered) 
            continue;
        BDBG_MSG(("Volume %d ",i));
        BDBG_MSG(("\n %s", mediaStorageStatus.volumeInfo[i].mounted?"Mounted":"Not Mounted"));
        BDBG_MSG(("\n\tdevice: %s", mediaStorageStatus.volumeInfo[i].device));
        BDBG_MSG(("\n\tname: %s", mediaStorageStatus.volumeInfo[i].name));
        BDBG_MSG(("\n\tmedia segment size: %d\n", mediaStorageStatus.volumeInfo[i].mediaSegmentSize));
    }
    CuAssertTrue(tc, 1 == mediaStorageStatus.numMountedVolumes );
    CuAssertTrue(tc, B_DVR_SUCCESS == dvrError);

    B_DVR_MediaStorage_Close(g_dvrTest->mediaStorage);
    BKNI_Free(g_dvrTest);
    g_dvrTest = NULL;

}

void DvrTestMediaStorageFormatVolume(CuTest* tc) 
{

    BDBG_MSG(("%s:", __FUNCTION__));

    system("umount /dev/sda1");
    system("umount /dev/sda2");
    system("umount /dev/sda3"); 

    printf("sgdisk -Z to destroy GPT and MBR\n");
    system("sgdisk -Z /dev/sda");

    if (g_dvrTest) {
        BKNI_Free(g_dvrTest);
    }
    g_dvrTest = BKNI_Malloc(sizeof(*g_dvrTest));
    BKNI_Memset(g_dvrTest,0,sizeof(*g_dvrTest));

    g_dvrTest->mediaStorage = B_DVR_MediaStorage_Open(NULL);
    CuAssertPtrNotNullMsg(tc, "B_DVR_MediaStorage_Open failed", g_dvrTest->mediaStorage);

    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);

    volumeIndex=0;

    /* check volume status */
    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);     

    /* Register volume */
    if (mediaStorageStatus.volumeInfo[volumeIndex].registered) {
        BDBG_WRN(("%s: Unregister volume %d\n", __FUNCTION__, volumeIndex));
        dvrError = B_DVR_MediaStorage_UnregisterVolume(g_dvrTest->mediaStorage,volumeIndex);
        if (dvrError>0)
            printf("Unregister error\n");
        else
            printf("Unregister success\n");
        CuAssertTrue(tc, B_DVR_SUCCESS == dvrError);
    }

    sprintf(g_dvrTest->mediaStorageRegisterSettings.device,"/dev/sda");
    g_dvrTest->mediaStorageRegisterSettings.startSec = 0;
    g_dvrTest->mediaStorageRegisterSettings.length = 0;
    dvrError = B_DVR_MediaStorage_RegisterVolume(g_dvrTest->mediaStorage,&g_dvrTest->mediaStorageRegisterSettings,&volumeIndex);
    if (dvrError >0)
        printf("\n Register media volume error");
    else
        printf("\n Register media volume success vol:%d\n",volumeIndex);
    
    CuAssertTrue(tc, B_DVR_SUCCESS == dvrError);
    /* format volume */
    dvrError = B_DVR_MediaStorage_FormatVolume(g_dvrTest->mediaStorage,volumeIndex);
    CuAssertTrue(tc, B_DVR_SUCCESS == dvrError);

    B_DVR_MediaStorage_Close(g_dvrTest->mediaStorage);
    BKNI_Free(g_dvrTest);
    g_dvrTest = NULL;
}
void addTestByNameMediaStorage(CuSuite* suite, char *testName) {

    static unsigned int en = 0;

    if (strcmp(testName, "Enable_DryRun") == 0) {
        g_CuTestDryRun = 1;
    }
    if  (strcmp(testName, "All_DvrTest") == 0) {
        en |= 0xffffffff;
    }
    if  (strcmp(testName, "All_DvrTestMediaStorage") == 0) {
        en |= All_DvrTestMediaStorage;
    }

    if  (strcmp(testName, "DvrTestMediaStorageFormatVolume") == 0 || (en & All_DvrTestMediaStorage))
        SUITE_ADD_TEST(suite, DvrTestMediaStorageFormatVolume);
    if  (strcmp(testName, "DvrTestMediaStorageMountVolume") == 0 || (en & All_DvrTestMediaStorage))
        SUITE_ADD_TEST(suite, DvrTestMediaStorageMountVolume);
}

void addTestByName(CuSuite* suite, char *testName) {
    static unsigned int en = 0;

    if (strcmp(testName, "Enable_DryRun") == 0) {
        g_CuTestDryRun = 1;
    }
    if  (strcmp(testName, "All_DvrTest") == 0) {
        en |= 0xffffffff;
    }
    if  (strcmp(testName, "All_DvrTestMediaStorage") == 0) {
        en |= All_DvrTestMediaStorage;
    }
    if  (strcmp(testName, "All_DvrTestTsb") == 0) {
        en |= All_DvrTestTsb;
    }
    if  (strcmp(testName, "All_DvrTestUtility") == 0) {
        en |= All_DvrTestUtility;
    }
    if  (strcmp(testName, "All_DvrTestRecord") == 0) {
        en |= All_DvrTestRecord;
    }
    if  (strcmp(testName, "All_DvrTestPlayback") == 0) {
        en |= All_DvrTestPlayback;
    }
    if  (strcmp(testName, "All_DvrTestHn") == 0) {
        en |= All_DvrTestHn;
    }
    if  (strcmp(testName, "All_DvrTestDrm") == 0) {
        en |= All_DvrTestDrm;
    }
	if	(strcmp(testName, "hnMulPFToMulStreamNetTest") == 0 || (en & All_DvrTestHn))
  	    SUITE_ADD_TEST(suite, hnMulPFToMulStreamNetTest);
}

void LoadTestLists(void)
{
    char buffer[512];
    char testcase[512];
    FILE *	f;
    int n = 0;

	g_cutest_output = CuStringNew();
	g_cutest_suite = CuSuiteNew();

    /* medianode test needs to be run before DvrTestInit */
    f = fopen(testlist_fn, "r");
    if (f == NULL)
	{
        BDBG_MSG(("No testlist found.\n"));
        return;
    }
	
    while (fgets (buffer, 512, f) != NULL)
	{
        if (buffer[0] != '#')
		{
            n = sscanf(buffer, "%s\n", testcase);
            if (n > 0)
                addTestByNameMediaStorage(g_cutest_suite, testcase);
        }
    }
	
    fclose(f);

    /*test  always enabled */
    SUITE_ADD_TEST(g_cutest_suite, DvrTestInit);

    /* additional test read from file */
    f = fopen(testlist_fn, "r");
    if (f == NULL)
	{
        BDBG_MSG(("No testlist found.\n"));
        return;
    }
	
    while (fgets (buffer, 512, f) != NULL)
	{
        if (buffer[0] != '#')
		{
            n = sscanf(buffer, "%s\n", testcase);
            if (n > 0)
                addTestByName(g_cutest_suite, testcase);
        }
    }
	
    fclose(f);

    SUITE_ADD_TEST(g_cutest_suite, DvrTestExit);

}

void RunAllTests(void)
{
	CuSuiteRun(g_cutest_suite);
	CuSuiteSummary(g_cutest_suite, g_cutest_output);
	CuSuiteDetails(g_cutest_suite, g_cutest_output);
	printf("%s\n", g_cutest_output->buffer);
}


int main(void)
{
    LoadTestLists();
    RunAllTests();
    return 0;
}
