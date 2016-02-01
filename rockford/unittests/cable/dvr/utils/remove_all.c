#include "dvr_test.h"


struct recordfile_list {
    unsigned volume_index;
    char sub_dir[B_DVR_MAX_FILE_NAME_LENGTH];
    char program_name[B_DVR_MAX_FILE_NAME_LENGTH];
    struct recordfile_list *next;
};

int total_recordings = 0;


B_DVR_ERROR recordfile_list_insert(int volumeIndex, char *subDir, char *programName,
    struct recordfile_list *record_start)
{
    struct recordfile_list *record_new_node, *record_node, *record_previous;

    record_node = record_start->next;
    record_previous= record_start;

    while (record_node != NULL) {
        record_previous = record_node;
        record_node = record_node->next;
    }
    if ((record_new_node = (struct recordfile_list *)
            calloc(1, sizeof(struct recordfile_list))) == NULL)
    {
        return B_DVR_OUT_OF_SYSTEM_MEMORY;
    }

    record_new_node->volume_index= volumeIndex;
    sprintf(record_new_node->sub_dir, subDir);
    sprintf(record_new_node->program_name, programName);
    record_new_node->next = record_node;
    record_previous->next = record_new_node;

    return B_DVR_SUCCESS;
}

void recordfile_list_print(struct recordfile_list *record_start)
{
    struct recordfile_list *record_node;

    record_node = record_start->next;

    printf("\n----------------------------------------------\n");
    printf(" volume index | sub directory | program name \n");
    printf("----------------------------------------------\n");
    while (record_node != NULL)
    {
        printf("----------------------------------------------\n");
        printf("       %d      |     %s     |   %s       \n",
                record_node->volume_index, record_node->sub_dir, record_node->program_name);

        record_node = record_node->next;
    }
    printf("----------------------------------------------\n");
}

void recordfile_list_free(struct recordfile_list *record_start)
{
    struct recordfile_list *record_node, *record_next;

    for (record_node = record_start; record_node != NULL; record_node = record_next)
    {
        record_next = record_node->next;
        free(record_node);
    }
}

B_DVR_ERROR DVR_delete_record(int volIndex, char *subDir, char *programName)
{

    B_DVR_MediaNodeSettings *mediaNodeSettings;
    B_DVR_ERROR err;
    mediaNodeSettings = malloc(sizeof(B_DVR_MediaNodeSettings));
    
    mediaNodeSettings->subDir = subDir;
    mediaNodeSettings->programName = programName;
    mediaNodeSettings->volumeIndex = volIndex;
    mediaNodeSettings->recordingType = eB_DVR_RecordingPermanent;

    printf(" deleting volume: %u directory: %s program: %s\n",mediaNodeSettings->volumeIndex, mediaNodeSettings->subDir, mediaNodeSettings->programName);

    err = B_DVR_Manager_DeleteRecording(B_DVR_Manager_GetHandle(),mediaNodeSettings);
    if (err!=B_DVR_SUCCESS)
    {
        printf("check the file name!\n");
    }
    free(mediaNodeSettings);
    return err;
}


void DVR_recordings_list(int volIndex, char *subDir, struct recordfile_list *record_start)
{
    char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];

    unsigned recordingCount;
    unsigned index;
    B_DVR_MediaNodeSettings mediaNodeSettings;


    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.programName = NULL;
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
    mediaNodeSettings.volumeIndex = volIndex;
    
    recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);

    total_recordings += recordingCount;

    if(recordingCount)
    {
        recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
        B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);

        for(index=0;index<recordingCount;index++)
        {
            B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
            recordfile_list_insert(volIndex, subDir, recordingList[index], record_start);
        }
        BKNI_Free(recordingList);
    }
    else
    {
        printf("\n No recordings found under %s directory!!!\n", subDir);
    }
}


void DvrTestUtilityRemoveAllRecordings(CuTest * tc)
{
    unsigned volumeIndex = 0;
    struct recordfile_list *record_start, *record_node;


    if ((record_start = (struct recordfile_list *)
        calloc(1, sizeof(struct recordfile_list))) == NULL)
    {
        printf("Unable to allocate memory for the record file list\n");
        CuFail(tc, "There is a problem to allocate memory for the list of recordings");
    }
    record_start->next = NULL;

    DVR_recordings_list(volumeIndex, "tsbConv", record_start);
    DVR_recordings_list(volumeIndex, "bgrec", record_start);
    DVR_recordings_list(volumeIndex, "transcode", record_start);

    if (total_recordings)
    {
        printf("\nFollowing recordings will be removed from the disk\n");
        recordfile_list_print(record_start);

        for (record_node = record_start->next; record_node != NULL; record_node = record_node->next)
        {
            DVR_delete_record(record_node->volume_index, record_node->sub_dir, record_node->program_name);
        }
    }
    recordfile_list_free(record_start);
}

