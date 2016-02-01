#include "dvr_test.h"
BDBG_MODULE(hnrecord);

static char hnrecordtxt[] = "hnrecord.txt";

static int recordmodetest(DvrTestHandle dvrtesthandle)
{
     char programName[B_DVR_MAX_FILE_NAME_LENGTH];
     char nfsFileName[B_DVR_MAX_FILE_NAME_LENGTH];
     B_DVR_MediaFileOpenMode openMode;
     B_DVR_MediaFilePlayOpenSettings openSettings;
     int nfsFileID;
     unsigned long bufSize;
     void *buffer;
     int rc = 0;

     strcpy(nfsFileName,dvrtesthandle->hnRecPara.nfsSourceFileName);
     if ((nfsFileID = open(nfsFileName, O_RDONLY,0666)) < 0)
     {
         printf("\n Unable to open %s",nfsFileName);
         rc = -1;
         goto errorrecordmodetest;
     }
     strcpy(programName,dvrtesthandle->hnRecPara.programName);
     strcpy(openSettings.subDir,dvrtesthandle->hnRecPara.programSubDir);
     openMode = eB_DVR_MediaFileOpenModeRecord;
     openSettings.playpumpIndex = 0;
     openSettings.volumeIndex = dvrtesthandle->hnRecPara.StreamPathIndex;
     bufSize = BUFFER_LEN;
     buffer  = BKNI_Malloc(bufSize + B_DVR_IO_BLOCK_SIZE);
     if(!buffer)
     {
         printf("\n unable to allocate source read buffer");
         rc= -1;
         close(nfsFileID);
         goto errorrecordmodetest;
     }
     dvrtesthandle->buffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)buffer);
     dvrtesthandle->mediaFile = B_DVR_MediaFile_Open(programName,openMode,&openSettings);

     if(!dvrtesthandle->mediaFile)
     {
         printf("\n unable to open dvr mediaFile %s",programName);
         rc = -1;
         free(buffer);
         close(nfsFileID);
         goto errorrecordmodetest;
     }
                
     while (read(nfsFileID,dvrtesthandle->buffer,bufSize)>0)
     {
         B_DVR_MediaFile_Write(dvrtesthandle->mediaFile,dvrtesthandle->buffer,bufSize);
     }
     printf("\n writing the nfs file to multiple file segments");
     B_DVR_MediaFile_Close(dvrtesthandle->mediaFile);
     close(nfsFileID);
     BKNI_Free(buffer);

errorrecordmodetest:
     return rc;
}

void InitHnRecordpara(void)
{
    g_dvrTest->hnRecPara.programSubDir[0] = '\0';
    g_dvrTest->hnRecPara.programName[0] = '\0';
    g_dvrTest->hnRecPara.StreamPathIndex = 0;
    g_dvrTest->hnRecPara.nfsSourceFilePath[0] = '\0';
    g_dvrTest->hnRecPara.nfsSourceFileName[0] = '\0';
}

void CheckRecordTestName(char *buffer)
{
    char testpara[512];
    char strpara[512];
    int  paravalue = 0;
    int  n = 0, m = 0;

    n = sscanf(buffer, "%s\n", testpara);
    if (n > 0)
    {
       if(strcmp(testpara, "ProgramSubDir") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnRecPara.programSubDir,strpara);
       }
       else if(strcmp(testpara, "ProgramName") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnRecPara.programName,strpara);
       }
       else if(strcmp(testpara, "StreamPathIndex") == 0)
       {
          m = sscanf(buffer, "%s %d\n", testpara,&paravalue);
          if(m > 0)
            g_dvrTest->hnRecPara.StreamPathIndex = paravalue;
       }
       else if(strcmp(testpara, "NFSSourceFilePath") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnRecPara.nfsSourceFilePath,strpara);
       }
       else if(strcmp(testpara, "NFSSourceFileName") == 0)
       {
          m = sscanf(buffer, "%s %s\n", testpara,strpara);
          if(m > 0)
            strcpy(g_dvrTest->hnRecPara.nfsSourceFileName,strpara);
       }
       else
          BDBG_WRN(("Parameter not recognized: %s\n", testpara));
    }
}


int HnRecordLoadTxt(void)
{
    char buffer[512];
    FILE *fd;
    int rc = 0;

    printf("\n Load Hn Record input txt file \n");
    fd= fopen(hnrecordtxt, "r");
    if(fd == NULL)
    {
        BDBG_MSG(("No input file found.\n"));
        rc = -1;
        return rc;
    }
	
    while (fgets(buffer, 512, fd) != NULL)
    {
      if (buffer[0] != '#')
      {
         CheckRecordTestName(buffer);
      }
    }
    fclose(fd);
    printf("\n ProgramSubDir =%s",g_dvrTest->hnRecPara.programSubDir);
    printf("\n ProgramName =%s",g_dvrTest->hnRecPara.programName);
    printf("\n NFSSourceFileName =%s",g_dvrTest->hnRecPara.nfsSourceFileName);
    printf("\n NFSSourceFilePath =%s",g_dvrTest->hnRecPara.nfsSourceFilePath);
    printf("\n StreamPathIndex =%d",g_dvrTest->hnRecPara.StreamPathIndex);

    return rc;
}

int hnRecordTest(CuTest * tc)
{
    int returnvalue = 0;

    printf("\n Enter hnRecordTest Main function : \n");
    InitHnRecordpara();
    returnvalue = HnRecordLoadTxt();
    if (returnvalue != 0)
    {
       printf("\n Missing input file\n");
       goto endofhnRecordTest;
    }

    g_dvrTest->pathIndex = g_dvrTest->hnRecPara.StreamPathIndex;
    returnvalue = recordmodetest(g_dvrTest);
endofhnRecordTest:
    CuAssertTrue(tc, returnvalue == 0);
    return returnvalue;
}
