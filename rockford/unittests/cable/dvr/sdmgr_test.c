#include "stdio.h"
#include "stdlib.h"

#include "nexus_platform.h"
#include "b_dvr_mediastorage.h"
#include "b_dvr_manager.h"
#include "msutil.h"
#include "msdiag.h"
#include "b_dvr_prof.h"

void printMenu(void)
{
    printf("10: format\n");
    printf("11: register\n");
    printf("12: Unregister\n");
    printf("13: volume status\n");
    printf("14: format async\n");
    printf("15: Check new device\n");
    printf("20: mount\n");
    printf("21: unmount\n");
    printf("22: set quota\n");
    printf("23: get quota\n");
    printf("30: allocate media segment for record\n");
    printf("31: free a media segment from record\n");
    printf("40: allocate nav segment for record\n");
    printf("41: free a nav segment from record\n");
    printf("50: get metadata folder path\n");
    printf("55: print segment status\n");
    printf("56: check and recover volume\n");
    printf("57: set diag print level\n");
    printf("60: create a recording for test\n");
    printf("61: delete a recording\n");
    printf("62: move a recording\n");
    printf("70: enable label\n");
    printf("80: print profile data\n");
    printf("99: exit\n");
}

void sdMgrCallback(void * appContext, unsigned index, B_DVR_Event event, B_DVR_Service service)

{
    B_DVR_ManagerHandle dvrManager;

    printf("sdMgrCallback: %s: index %d, event %d, service %d",(char*)appContext, index, event,service);

    dvrManager=B_DVR_Manager_GetHandle();

    switch (event) {
        case eB_DVR_EventMountSuccss:
            B_DVR_Manager_CreateMediaNodeList(dvrManager,index);
            break;
        default:
            break;
    }
}

#if 1
void setQuota(B_DVR_MediaStorageHandle mediaStorage)
{
    char subDir [ B_DVR_MAX_FILE_NAME_LENGTH ];
    unsigned volIndex;
    uint64_t sizeKbytes;
    B_DVR_ERROR err;
    
    printf("volumeIndex?: ");
    scanf("%u",&volIndex);
    printf("subdir to set quota: ");
    scanf("%s",subDir);    
    if(subDir[0]=='/') subDir[0]='\0';
    printf("Size in Kbytes?: ");
    scanf("%llu",&sizeKbytes);

    printf("subdir %s, size = %llu\n", subDir,sizeKbytes);

    err = B_DVR_MediaStorage_SetDirQuota(mediaStorage,volIndex,subDir,sizeKbytes);
    printf("B_DVR_MediaStorage_SetDirQuota returned %d\n", err);
}

void getQuota(B_DVR_MediaStorageHandle mediaStorage)
{
    char subDir [ B_DVR_MAX_FILE_NAME_LENGTH ];
    unsigned volIndex;
    uint64_t sizeKbytes;
    uint64_t freeKbytes;
    B_DVR_ERROR err;
    
    printf("volumeIndex?: ");
    scanf("%u",&volIndex);
    printf("subdir to set quota: ");
    scanf("%s",subDir);    
    if(subDir[0]=='/') subDir[0]='\0';

    err = B_DVR_MediaStorage_GetDirQuota(mediaStorage,volIndex,subDir,&sizeKbytes,&freeKbytes);

    if (err){
        printf("Quota not set for %s\n", subDir);
    } else {
        printf("Quota = %llu\n", sizeKbytes);
    }

    printf("B_DVR_MediaStorage_GetDirQuota returned %d\n", err);
}
#endif

void createRecording(void)
{
    char programName[256];
    unsigned numSegs;
    char subDir [ B_DVR_MAX_FILE_NAME_LENGTH ];
    unsigned volIndex;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    
    printf("volumeIndex?: ");
    scanf("%u",&volIndex);
    printf("program name to create: ");
    scanf("%s",programName);    
    printf("subdir: ");
    scanf("%s",subDir);    
    if(subDir[0]=='/') subDir[0]='\0';
    printf("how many segments?: ");
    scanf("%u",&numSegs);
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.volumeIndex = volIndex;
    mediaNodeSettings.programName = programName;
    rc = B_DVR_Manager_AllocSegmentedFileRecord(B_DVR_Manager_GetHandle(),
                                                &mediaNodeSettings,
                                                numSegs);
    if(rc!=B_DVR_SUCCESS)
    {
        printf("\n create recording failed");
    }
}

void deleteRecording(void)
{
    char programName[256];
    unsigned volIndex;
    char *subDir;
    B_DVR_MediaNodeSettings *mediaNodeSettings;
    B_DVR_ERROR err;

    mediaNodeSettings = malloc(sizeof(B_DVR_MediaNodeSettings));
    subDir = malloc(100);
    
    printf("volumeIndex?: ");
    scanf("%u",&volIndex);
    printf("program name to delete: ");
    scanf("%s",programName);    
    printf("subdir: ");
    scanf("%s",subDir);    
    if(subDir[0]=='/') subDir[0]='\0';
    
    mediaNodeSettings->programName =programName;
    mediaNodeSettings->subDir = subDir;
    mediaNodeSettings->volumeIndex = volIndex;
    mediaNodeSettings->recordingType = eB_DVR_RecordingPermanent;

    printf("deleting volume %u:%s\n",mediaNodeSettings->volumeIndex, mediaNodeSettings->programName);

    err = B_DVR_Manager_DeleteRecording(B_DVR_Manager_GetHandle(),mediaNodeSettings);
    if (err!=B_DVR_SUCCESS) 
        printf("check the file name!\n");

    free(subDir);
    free(mediaNodeSettings);
}

void moveRecording(void)
{
    char fileName[256];
    char srcDir[256];
    char destDir[256];
    unsigned volIndex;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_ERROR err;
    
    printf("volumeIndex?: ");
    scanf("%u",&volIndex);
    printf("filename to move: ");
    scanf("%s",fileName);    
    printf("source dir: ");
    scanf("%s",srcDir);    
    if(srcDir[0]=='/') srcDir[0]='\0';
    printf("dest dir: ");
    scanf("%s",destDir);    
    if(destDir[0]=='/') destDir[0]='\0';

    mediaNodeSettings.programName =fileName;
    mediaNodeSettings.subDir = srcDir;
    mediaNodeSettings.volumeIndex = volIndex;
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;

    printf("moving %s from %s to %s\n",fileName, srcDir, destDir);

    err = B_DVR_Manager_MoveRecording(B_DVR_Manager_GetHandle(),&mediaNodeSettings,destDir);
    if (err!=B_DVR_SUCCESS) 
        printf("check the file name!\n");

}

void checkDevice(B_DVR_MediaStorageHandle mediaStorage)
{
    char device[256];
    B_DVR_ERROR dvrErr;
    int volIndex;
    
    printf("device to check: ");
    scanf("%s",device);    
    printf("checking %s\n",device);

    dvrErr = B_DVR_MediaStorage_CheckDevice(mediaStorage,device,&volIndex);

    if (dvrErr != B_DVR_SUCCESS) {
        printf("not a valid media volume. register to use.\n");
        return ;
    }

    printf("Device %s registered as volume %d\n",device,volIndex);
}

void uncheckDevice(B_DVR_MediaStorageHandle mediaStorage)
{
    char device[256];
    B_DVR_ERROR dvrErr;
    int volIndex;
    
    printf("device to check: ");
    scanf("%s",device);    
    printf("checking %s\n",device);

    dvrErr = B_DVR_MediaStorage_UncheckDevice(mediaStorage,device,&volIndex);

    if (dvrErr != B_DVR_SUCCESS) {
        printf("B_DVR_MediaStorage_UncheckDevice failed.\n");
        return ;
    }

    printf("Device %s registered as volume %d\n",device,volIndex);
}

void IncreadRefCnt(B_DVR_MediaStorageHandle mediaStorage)
{
    B_DVR_ERROR dvrErr;
    unsigned volIndex;
    char mediaSegment[B_DVR_MAX_FILE_NAME_LENGTH];
    char subDir[256];

    printf("volumeIndex?: ");
    scanf("%u",&volIndex);
    printf("sub dir: ");
    scanf("%s",subDir);    
    if(subDir[0]=='/') subDir[0]='\0';
    printf("media segment name: ");
    scanf("%s",mediaSegment);    

    dvrErr = B_DVR_MediaStorage_IncreaseMediaSegmentRefCount(mediaStorage,volIndex, subDir, mediaSegment);
    if (dvrErr) {
        printf("failed to incread media Segment refcnt!\n");
    }
    else {
        printf("refcnt increased for :%s\n", mediaSegment);
    }
}

void allocMediaSegments(B_DVR_MediaStorageHandle mediaStorage)
{
    B_DVR_ERROR dvrErr;
    unsigned volIndex;
    char mediaSegment[B_DVR_MAX_FILE_NAME_LENGTH];
    char subDir[256];

    printf("volumeIndex?: ");
    scanf("%u",&volIndex);
    printf("sub dir: ");
    scanf("%s",subDir);    
    if(subDir[0]=='/') subDir[0]='\0';

    dvrErr = B_DVR_MediaStorage_AllocateMediaSegment(mediaStorage,volIndex, subDir, mediaSegment);
    if (dvrErr) {
        printf("failed to allocate media Segment!\n");
    }
    else {
        printf("media segment allocated for record:%s\n", mediaSegment);
    }
}

void freeMediaSegments(B_DVR_MediaStorageHandle mediaStorage)
{
    B_DVR_ERROR dvrErr;
    unsigned volIndex;
    char mediaSegment[B_DVR_MAX_FILE_NAME_LENGTH];
    char subDir[256];

    printf("volumeIndex?: ");
    scanf("%u",&volIndex);
    printf("sub dir: ");
    scanf("%s",subDir);    
    if(subDir[0]=='/') subDir[0]='\0';
    printf("record media segment file name to free?:");
    scanf("%s",mediaSegment);

    dvrErr = B_DVR_MediaStorage_FreeMediaSegment(mediaStorage, volIndex, subDir, mediaSegment);
    
    if (dvrErr) {
        printf("failed to free media Segment!\n");
    }
    else {
        printf("record media segment moved to depot:%s\n", mediaSegment);
    }

}

unsigned char testLabel[1024]={0,0,0,0,0,0,0,0,0x11,0x22,0x33,0x44,0x55,0x66,0x00};

void enableLabel(B_DVR_MediaStorageHandle mediaStorage)
{
    B_DVR_MediaStorageSettings settings;

    settings.labelSupport = true;
    settings.label = testLabel;
    settings.labelSize = 1024;

    B_DVR_MediaStorage_SetSettings(mediaStorage,&settings);
}

void disableLabel(B_DVR_MediaStorageHandle mediaStorage)
{
    B_DVR_MediaStorageSettings settings;

    settings.labelSupport = false;

    B_DVR_MediaStorage_SetSettings(mediaStorage,&settings);
}

#define NUM_PROFILER 4

void printProfData(void)
{
    int i;
    B_DVR_ProfStatus status;

    for(i=0; i<NUM_PROFILER; i++) {
        B_DVR_Prof_GetStatus(i,&status);
        printf("%s\n",status.name);
        printf("number of calls:          %lu\n",status.calls);
        printf("total execution time:     %ld.%09ld seconds\n",status.totalTime.tv_sec,status.totalTime.tv_nsec);
        printf("mean execution time:      %ld.%09ld seconds\n",status.meanTime.tv_sec,status.meanTime.tv_nsec);
        printf("peak high execution time: %ld.%09ld seconds\n",status.peakHigh.tv_sec,status.peakHigh.tv_nsec);
        printf("peak low execution time:  %ld.%09ld seconds\n",status.peakLow.tv_sec,status.peakLow.tv_nsec);
    }
}

int main(void)
{
    B_DVR_MediaStorageHandle mstorage;
     B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
    B_DVR_MediaStorageStatus mstorageStatus;
    char navSegment[B_DVR_MAX_FILE_NAME_LENGTH];
    char path[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_ERROR dvrErr;
    int cmd = 1;
    char cmdstr[100];
    unsigned volumeIndex = 0, index;
    int i;
    char *test="SD manager test";
    B_DVR_ManagerHandle dvrManager;
    B_DVR_ProfSettings profSettings;

    system("umount /mnt/dvr*");
    system("umount /tmp/dvr*");

/*    NEXUS_Platform_Init(NULL); */

    B_DVR_Prof_Create(NUM_PROFILER);

    sprintf(profSettings.name,"%s","ms_update_registry");
    B_DVR_Prof_SetSettings(0,&profSettings);

    sprintf(profSettings.name,"%s","B_DVR_File_Close");
    B_DVR_Prof_SetSettings(3,&profSettings);

    mediaStorageOpenSettings.storageType = eB_DVR_MediaStorageTypeBlockDevice;
    mstorage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);

    dvrManager = B_DVR_Manager_Init(NULL);

    enableLabel(mstorage);

    B_DVR_MediaStorage_InstallCallback(mstorage,sdMgrCallback,test);
    
    if(!mstorage) {
        printf("check registry.\n");
        return -1;
    }
    
    B_DVR_MediaStorage_GetStatus(mstorage,&mstorageStatus);

    printMenu();

    while (cmd != 99) {
        fgets(cmdstr, 256, stdin);
        cmd = atoi(cmdstr);
        switch (cmd) {
            case 10:
                printf("volume index: ");
                scanf("%u",&volumeIndex);
                B_DVR_MediaStorage_FormatVolume(mstorage,volumeIndex);
                break;
            case 11:
                printf("device to register: ");
                scanf("%s",path);
                sprintf(registerSettings.device,path);
                registerSettings.startSec=0;
                registerSettings.length=0;
                if (B_DVR_MediaStorage_RegisterVolume(mstorage,&registerSettings,&index) >0)
                    printf("Register error\n");
                else
                    printf("Register success. volume index is %d\n", index);
                break;
            case 12:
                printf("volume index: ");
                scanf("%u",&volumeIndex);
                if (B_DVR_MediaStorage_UnregisterVolume(mstorage,(unsigned)cmd) >0)
                    printf("Unregister error\n");
                else
                    printf("Unregister success\n");
                break;
            case 13:
                B_DVR_MediaStorage_GetStatus(mstorage,&mstorageStatus);        
                printf("Number of registered volumes:%d\n", mstorageStatus.numRegisteredVolumes);
                printf("Number of mounted volumes:%d\n", mstorageStatus.numMountedVolumes);
                for (i=0; i<B_DVR_MEDIASTORAGE_MAX_VOLUME; i++) {
                    if (!mstorageStatus.volumeInfo[i].registered) continue;
                    printf("Volume %d ",i);
                    printf("%s\n", mstorageStatus.volumeInfo[i].mounted?"Mounted":"Not Mounted");
                    printf("\tname: %s\n", mstorageStatus.volumeInfo[i].name);
                    printf("\tdevice: %s\n", mstorageStatus.volumeInfo[i].device);
                    printf("\tmedia segment size: %d\n", mstorageStatus.volumeInfo[i].mediaSegmentSize);
                }
                break;
            case 14:
                printf("volume index: ");
                scanf("%u",&volumeIndex);
                B_DVR_MediaStorage_FormatVolumeAsync(mstorage,volumeIndex);
                break;
            case 15:
                checkDevice(mstorage);
                break;
            case 16:
                uncheckDevice(mstorage);
                break;
            case 20:
                printf("volume index: ");
                scanf("%u",&volumeIndex);
                dvrErr = B_DVR_MediaStorage_MountVolumeAsync(mstorage, volumeIndex);
                break;
            case 21:
                printf("volume index: ");
                scanf("%u",&volumeIndex);
                B_DVR_MediaStorage_UnmountVolume(mstorage,volumeIndex);
                break;
#if 1
            case 22:
                setQuota(mstorage);
                break;
            case 23:
                getQuota(mstorage);
                break;
#endif
            case 30:
                allocMediaSegments(mstorage);
                break;
            case 31:
                freeMediaSegments(mstorage);
                break;
            case 32:
                IncreadRefCnt(mstorage);
                break;

            case 40:
                printf("volume index: ");
                scanf("%u",&volumeIndex);
                dvrErr = B_DVR_MediaStorage_AllocateNavSegment(mstorage,volumeIndex, navSegment);
                if (dvrErr) {
                    printf("failed to allocate nav Segment!\n");
                }
                else {
                    printf("nav segment allocated for record:%s\n", navSegment);
                }
                break;
            case 41:
                printf("volume index: ");
                scanf("%u",&volumeIndex);
                printf("record nav segment file name to free?:");
                scanf("%s",navSegment);
                dvrErr = B_DVR_MediaStorage_FreeNavSegment(mstorage,volumeIndex, navSegment);
                if (dvrErr) {
                    printf("failed to free nav Segment!\n");
                }
                else {
                    printf("nav segment freed to depot:%s\n", navSegment);
                }
                break;
            case 50:
                printf("volume index: ");
                scanf("%u",&volumeIndex);
                B_DVR_MediaStorage_GetMetadataPath(mstorage,volumeIndex, path);
                printf("volume %d metadata path: %s\n", volumeIndex, path);
                break;
            case 55:
                printf("mountname: ");
                scanf("%s",path);
                ms_dump_volume_status(path);
                break;
            case 56:
                printf("mountname: ");
                scanf("%s",path);
                ms_recover_volume(path,1);
                break;
            case 57:
                {
                    int level;
                    
                    printf("print level: ");
                    scanf("%d",&level);
                    ms_set_level(level);
                }
                break;
            case 60:
                createRecording();
                break;

            case 61:
                deleteRecording();
                break;

            case 62:
                moveRecording();
                break;

            case 70:
                enableLabel(mstorage);
                break;
            case 71:
                disableLabel(mstorage);
                break;
            case 80:
                printProfData();
                break;
            default:
                printMenu();
                break;
        }
    }

    
    return 0;    
}
