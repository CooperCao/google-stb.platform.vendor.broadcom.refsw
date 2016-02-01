#include "dvr_test.h"


struct recordfile_list {
    int program_index;
    int record_type;
    unsigned volume_index;
    char program_name[B_DVR_MAX_FILE_NAME_LENGTH];
    int video_pid;
    struct recordfile_list *next;
};

struct metadata_list {
    int index;
    char mediaSeg[B_DVR_MAX_FILE_NAME_LENGTH];
    char navSeg[B_DVR_MAX_FILE_NAME_LENGTH];
    struct metadata_list *next;
};


unsigned insertComplete = 0;

B_DVR_ERROR dumpMetaDataFile(char *fileName, struct metadata_list *start, int metadataType);
B_DVR_ERROR insert_metadata_list_media(char *mediaSegFile, int indexVal, struct metadata_list *start);
B_DVR_ERROR insert_metadata_list_nav(char *navSegFile, int indexVal, struct metadata_list *start);
void free_metadata_list(struct metadata_list *start);


void print_recordfile_list(struct recordfile_list *record_start);
void free_recordfile_list(struct recordfile_list *record_start);



B_DVR_ERROR dumpMetaDataFile(char *fileName, struct metadata_list *start, int metadataType)
{
    B_DVR_SegmentedFileNode fileNode;
    off_t returnOffset, fileNodeOffset;
    ssize_t sizeRead=0;
    unsigned index=0;
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_File dvrFile;
    B_DVR_FileSettings settings;


    settings.fileIO = eB_DVR_FileIORead;
    settings.directIO = false;
    B_DVR_File_Open(&dvrFile,fileName, &settings);

    do
    {
        fileNodeOffset = index*sizeof(fileNode);
        returnOffset = dvrFile.fileInterface.io.readFD.seek(
            &dvrFile.fileInterface.io.readFD,
            fileNodeOffset,SEEK_SET);

        if(returnOffset != fileNodeOffset)
        {
            break;
        }
         sizeRead = dvrFile.fileInterface.io.readFD.read(
             &dvrFile.fileInterface.io.readFD,&fileNode,sizeof(fileNode));

        if(sizeRead != sizeof(fileNode))
        {

        }
        else
        {
            if (metadataType) /* media segmentation */
            {
                insert_metadata_list_media(fileNode.fileName, index, start);
            }
            else    /* nav segmentation */
            {
                insert_metadata_list_nav(fileNode.fileName, index, start);
            }
        }
        index++;
    }while(sizeRead);

    B_DVR_File_Close(&dvrFile);
    insertComplete = 1;
    return rc;
}


B_DVR_ERROR insert_metadata_list_media(char *mediaSegFile, int indexVal, struct metadata_list *start)
{
    struct metadata_list *new_node, *node, *previous;

    node = start->next;
    previous= start;

    while (node != NULL) {
        previous = node;
        node = node->next;
    }
    if ((new_node = (struct metadata_list *)
            calloc(1, sizeof(struct metadata_list))) == NULL)
    {
        return B_DVR_OUT_OF_SYSTEM_MEMORY;
    }
    new_node->index = indexVal;
    sprintf(new_node->mediaSeg, mediaSegFile);
    new_node->next = node;
    previous->next = new_node;

    return B_DVR_SUCCESS;
}

B_DVR_ERROR insert_metadata_list_nav(char *navSegFile, int indexVal, struct metadata_list *start)
{
    struct metadata_list *node;

    node = start->next;

    while ((node != NULL) && (node->index < indexVal))
        node = node->next;

    sprintf(node->navSeg, navSegFile);

    return B_DVR_SUCCESS;
}

void print_metadata_list(struct metadata_list *start)
{
    struct metadata_list *node;

    node = start->next;

    printf("\n---------------------------------------------\n");
    printf(" index | media segments   | nav segments  \n");
    printf("---------------------------------------------\n");
    while (node != NULL)
    {
        printf("---------------------------------------------\n");
        printf("   %d   | %s     | %s\n", node->index, node->mediaSeg, node->navSeg);
        node = node->next;
    }
    printf("---------------------------------------------\n");
}


void regenerate_nav_segment(struct metadata_list *start, int volIndex, int videoPid)
{
    struct metadata_list *node;
    char command[B_DVR_MAX_FILE_NAME_LENGTH];

    node = start->next;

    while (node != NULL)
    {
        sprintf(command, "createindex /mnt/dvr%d-media/record/%s /mnt/dvr%d-nav/record/%s %d -in=ts -out=bcm", volIndex, node->mediaSeg, volIndex, node->navSeg, videoPid);
        system(command);
        node = node->next;
    }
}

B_DVR_ERROR create_metadata_list(unsigned volIndex, char *subDir, char *programName, int videoPid)
{
    char metadataFile[B_DVR_MAX_FILE_NAME_LENGTH];
    struct metadata_list *start, *node;
    B_DVR_ERROR rc=B_DVR_SUCCESS;

    if ((start = (struct metadata_list *)
        calloc(1, sizeof(struct metadata_list))) == NULL)
    {
        printf("Unable to allocate memory for the metadata list\n");
        return B_DVR_OUT_OF_SYSTEM_MEMORY;
    }
    node = start;
    node->next = NULL;

    sprintf(metadataFile, "/mnt/dvr%d-metadata/%s/%s%s", volIndex, subDir, programName,
        B_DVR_MEDIA_FILE_EXTENTION);

    rc = dumpMetaDataFile(metadataFile, start, 1);

    do {
        if ((rc == B_DVR_SUCCESS) && (insertComplete))
            break;
    } while (1);
    insertComplete = 0;

    sprintf(metadataFile, "/mnt/dvr%d-metadata/%s/%s%s", volIndex, subDir, programName,
    B_DVR_NAVIGATION_FILE_EXTENTION);

    rc = dumpMetaDataFile(metadataFile, start, 0);

    do {
        if ((rc == B_DVR_SUCCESS) && (insertComplete))
            break;
    } while (1);
    insertComplete = 0;

    print_metadata_list(start);

    regenerate_nav_segment(start, volIndex, videoPid);

    free_metadata_list(start);
    return B_DVR_SUCCESS;
}


void free_metadata_list(struct metadata_list *start)
{
    struct metadata_list *node, *next;

    for (node = start; node != NULL; node = next)
    {
        next = node->next;
        free(node);
    }
}


B_DVR_ERROR insert_recordfile_list(int index, int recordType, int volumeIndex, char *programName,
    unsigned VideoPid, struct recordfile_list *record_start)
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

    record_new_node->program_index = index;
    record_new_node->record_type = recordType;
    record_new_node->volume_index= volumeIndex;
    sprintf(record_new_node->program_name, programName);
    record_new_node->video_pid = VideoPid;
    record_new_node->next = record_node;
    record_previous->next = record_new_node;

    return B_DVR_SUCCESS;
}



void print_recordfile_list(struct recordfile_list *record_start)
{
    struct recordfile_list *record_node;

    record_node = record_start->next;

    printf("\n-------------------------------------------------------------------------\n");
    printf(" program index | record type | volume index | program name   | video pid\n");
    printf("-------------------------------------------------------------------------\n");
    while (record_node != NULL)
    {
        printf("-------------------------------------------------------------------------\n");
        if (record_node->record_type)
        {
            printf("       %d       |   %s   |      %d       | %s          | %d\n", record_node->program_index, "TSBConv",
                record_node->volume_index, record_node->program_name, record_node->video_pid);
        }
        else
        {
            printf("       %d       |   %s    |      %d       | %s        | %d\n", record_node->program_index, "Record",
                record_node->volume_index, record_node->program_name, record_node->video_pid);
        }
        record_node = record_node->next;
    }
    printf("-------------------------------------------------------------------------\n");
}



void free_recordfile_list(struct recordfile_list *record_start)
{
    struct recordfile_list *record_node, *record_next;

    for (record_node = record_start; record_node != NULL; record_node = record_next)
    {
        record_next = record_node->next;
        free(record_node);
    }
}


B_DVR_ERROR DVR_Recordings_List_Create(int volIndex, char *subDir, struct recordfile_list *record_start, CuTest * tc)
{
    char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];

    unsigned recordingCount;
    unsigned index;
    unsigned esStreamIndex;
    unsigned VideoPID;
    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    unsigned record_type;


    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.programName = NULL;
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
    mediaNodeSettings.volumeIndex = volIndex;

    recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);

    if(recordingCount)
    {
		recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
        CuAssertPtrNotNull(tc, recordingList);
        B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);

        for(index=0;index<recordingCount;index++)
        {

            B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);

            mediaNodeSettings.programName = recordingList[index];
            B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);

            for(esStreamIndex=0;esStreamIndex < media.esStreamCount;esStreamIndex++) 
            {
                if(media.esStreamInfo[esStreamIndex].pidType == eB_DVR_PidTypeVideo)
                {
                   VideoPID = media.esStreamInfo[esStreamIndex].pid;
                }
            }
            if (strcmp(subDir, "tsbConv"))
                record_type = 0;
            else
                record_type = 1;

            insert_recordfile_list(index, record_type, volIndex, recordingList[index], VideoPID, record_start);
        }

        BKNI_Free(recordingList);
    }
    else
    {
        printf("\n No recordings found under %s directory!!!\n", subDir);
    }

    return B_DVR_SUCCESS;
}


void DvrTestUtilityNavRegenerator(CuTest * tc)
{
    unsigned volumeIndex = 0;
    struct recordfile_list *record_start, *record_node;

    if ((record_start = (struct recordfile_list *)
        calloc(1, sizeof(struct recordfile_list))) == NULL)
    {
        printf("Unable to allocate memory for the record file list\n");
    }
    record_start->next = NULL;


    DVR_Recordings_List_Create(volumeIndex, "tsbConv", record_start, tc);
    DVR_Recordings_List_Create(volumeIndex, "bgrec", record_start, tc);

    print_recordfile_list(record_start);

    for (record_node = record_start->next; record_node != NULL; record_node = record_node->next)
    {
        if (record_node->record_type)
            create_metadata_list(record_node->volume_index, "tsbConv", record_node->program_name, record_node->video_pid);
        else
            create_metadata_list(record_node->volume_index, "bgrec", record_node->program_name, record_node->video_pid);
    }
    
    free_recordfile_list(record_start);

}

