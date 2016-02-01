#include "dvr_test.h"


void print_media_attributes(unsigned attributeVal)
{
    int attribute = 32;

    do {
        if (attribute & attributeVal)
        {
            switch (attribute)
            {
                case B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM:
                    printf("Segmented_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_ENCRYPTED_STREAM:
                    printf("Encrypted_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_HD_STREAM:
                    printf("HD_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_AUDIO_ONLY_STREAM:
                    printf("Audio_Only_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_HITS_STREAM:
                    printf("HITS_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS:
                    printf("Recording_In_Progress ");
                break;
                default:
                    printf("Unknown_Attribute_Value ");
                break;
            }
        }
        attribute = attribute >> 1;
    } while (attribute > 0);
}


void B_DVR_Recordings_List(int volIndex, char *subDir, CuTest * tc)
{
    char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];

    unsigned recordingCount;
    unsigned index;
    unsigned esStreamIndex;
    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    unsigned keyLength;
    unsigned count=0;
    unsigned char key[16];


    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.programName = NULL;
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
    mediaNodeSettings.volumeIndex = volIndex;
    
    recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);

    if(recordingCount)
    {
        printf("\n******************************************");
        printf("\nList of Recordings (%s)", subDir);
        printf("\n******************************************");
        recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
        CuAssertPtrNotNull(tc, recordingList);
        B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);

        for(index=0;index<recordingCount;index++)
        {
            printf("\n------------------------------------------");
            printf("\n program index: %d", index);
            printf("\n program name: %s", recordingList[index]);
            mediaNodeSettings.programName = recordingList[index];
            B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
            printf("\n program metaData file: %s",media.mediaNodeFileName);
            printf("\n media metaData file: %s",media.mediaFileName);
            printf("\n nav metaData file: %s",media.navFileName);
            printf("\n media size: %u",(unsigned)(media.mediaLinearEndOffset-media.mediaLinearStartOffset));
            printf("\n media attributes: ");
            print_media_attributes(media.mediaAttributes);
            printf("\n nav size: %u",(unsigned)(media.navLinearEndOffset - media.navLinearStartOffset));
            printf("\n media time (seconds): %u",(unsigned)((media.mediaEndTime - media.mediaStartTime)/1000));
            printf("\n [ PID Info ]");
            for(esStreamIndex=0;esStreamIndex < media.esStreamCount;esStreamIndex++) 
            {
                if(media.esStreamInfo[esStreamIndex].pidType == eB_DVR_PidTypeVideo)
                {
                    printf("\n %u Video PID:",esStreamIndex);
                }
                else
                {
                    printf("\n %u Audio PID:",esStreamIndex);
                }
                printf(" %u",media.esStreamInfo[esStreamIndex].pid);
            }
            if (!(media.mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_ENCRYPTED_STREAM)) 
            {
                printf("\n media stream is not encrypted\n");
            }
            else 
            {
                printf("\n media stream is encrypted");
                if (media.drmServiceType == eB_DVR_DRMServiceTypeBroadcastMedia)
                {
                    printf("\n drm service type: Broadcast Media");
                }
                else if (media.drmServiceType == eB_DVR_DRMServiceTypeContainerMedia)
                {
                    printf("\n drm service type: Container Media");
                }
                if (media.drmServiceKeyType == eB_DVR_DRMServiceKeyTypeProtected)
                {
                    printf("\n drm service key type: Protected");
                }
                else if (media.drmServiceKeyType == eB_DVR_DRMServiceKeyTypeClear)
                {
                    printf("\n drm service key type: Clear");
                }
                else if (media.drmServiceKeyType == eB_DVR_DRMServiceKeyTypeMax)
                {
                    printf("\n drm service key type: Not Available");
                }

                B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keyLength);
                B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,media.esStreamInfo[0].pid,key);
            
                printf("\n key length = %d\n", keyLength);
                printf(" key = ");
                for(count=0;count<keyLength;count++)
                {
                    printf("0x%x ", key[count]);
                }
            }
            printf("\n------------------------------------------\n");

        }
        BKNI_Free(recordingList);
    }
    else
    {
        printf("\n No recordings found under %s directory!!!\n", subDir);
    }
}



void DvrTestUtilityContentList(CuTest * tc)
{
    unsigned volumeIndex = 0;


    B_DVR_Recordings_List(volumeIndex, "tsbConv", tc);
    B_DVR_Recordings_List(volumeIndex, "bgrec", tc);
    B_DVR_Recordings_List(volumeIndex, "transcode", tc);
}

