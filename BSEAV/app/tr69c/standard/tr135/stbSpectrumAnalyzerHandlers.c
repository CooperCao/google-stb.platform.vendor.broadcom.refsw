/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "bstd.h"

#include "../inc/tr69cdefs.h"
#include "bcmTR135Objs.h"
#include "utils.h"


BDBG_MODULE(spectrumAnalyzer);


extern TRxObjNode  measurementTableDesc[];
extern TRxObjNode  comparisonTableDesc[];
extern int getInstanceCount( TRxObjNode *n);
int skipID = 0;
static uint32_t *dataPointer=NULL;
uint32_t *currentDataPointer;
unsigned totalDataSamplesRead;
static measurementTable *table = NULL;
/*static BKNI_EventHandle spectrumEvent = NULL;*/



static void spectrum_data_ready_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend;
    BKNI_EventHandle spectrumEvent = (BKNI_EventHandle)param;

    BDBG_ASSERT(NULL != context);
    frontend = (NEXUS_FrontendHandle)context;
    BKNI_SetEvent(spectrumEvent);
}

/* Device.X_BROADCOM_COM_SpectrumAnalyzer.MeasurementTable.{i} */

int addMeasurementTableInstance(char *info)
{
    TRxObjNode *n;
    InstanceDesc *idp;

    n = measurementTableDesc;
    idp = getNewInstanceDesc(n, NULL, 1);

    if (idp != NULL) {
        idp->hwUserData = info;
        return 1;
    }
    return 0;
}

int initMeasurementTable(void)
{
	NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendAcquireSettings settings;
	NEXUS_FrontendHandle frontend = NULL;
    BKNI_EventHandle spectrumEvent = NULL;
    NEXUS_FrontendSpectrumSettings spectrumSettings;
	int numSamples = 0;
	int measurementsPerBin = 0;
	int rangeOfBin = 0;
	unsigned firstReferenceFreq = 0;
	unsigned firstStopFreq = 0;
	spectrumAnalyzer *spectrum;
	int i = 0;
	int j, k, m;
	int numOfEntries = 0;
	int32_t tmp[MAX_NUMSAMPLES*DEFAULT_NUMENTRIES];
	int numAverages = 0;

	spectrum = getSpectrumAnalyzer();
	numSamples = spectrum->numSamples;
	measurementsPerBin = spectrum->measurementsPerBin;
	numOfEntries = spectrum->measurementTableNumberOfEntries;
	rangeOfBin = (MAX_FREQ - MIN_FREQ)/numOfEntries;
	firstReferenceFreq = MIN_FREQ + rangeOfBin/2;
	firstStopFreq = MIN_FREQ + rangeOfBin;
	numAverages = spectrum->numAverages;

	if (table == NULL) {
		NEXUS_Memory_Allocate(numOfEntries*sizeof(table)*numSamples*2, NULL, (void *)&table);
		if (table == NULL) {
			printf("NEXUS_Memory_Allocate for table failed !!!");
			return -1;
		}
	}

	BKNI_Memset(table, 0, numOfEntries*sizeof(table)*numSamples*2);
	NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.qam = true;
    frontend = NEXUS_Frontend_Acquire(&settings);

    if (!frontend) {
        printf("Unable to find capable frontend. Run nxserver with -frontend option.");
        return -1;
    }
	BKNI_CreateEvent(&spectrumEvent);

	if (dataPointer == NULL) {
		NEXUS_MemoryAllocationSettings allocSettings;
		NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
		/* select a heap with driver-side mmap */
		NEXUS_ClientConfiguration clientConfig;
		NEXUS_Platform_GetClientConfiguration(&clientConfig);
		allocSettings.heap = clientConfig.heap[NXCLIENT_FULL_HEAP];
		NEXUS_Memory_Allocate(numSamples*4, &allocSettings, (void*)&dataPointer);

		if (dataPointer==NULL) {
			printf("NEXUS_Memory_Allocate failed !!!");
			return -1;
		}
	}

	BKNI_Memset(dataPointer, 0, numSamples*numOfEntries*4);
    currentDataPointer = dataPointer;
	spectrumSettings.data = dataPointer;
    spectrumSettings.numSamples = spectrum->numSamples;
    spectrumSettings.startFrequency = 0;
    spectrumSettings.stopFrequency = 0;
    spectrumSettings.fftSize = 1024;
    spectrumSettings.dataReadyCallback.callback = spectrum_data_ready_callback;
    spectrumSettings.dataReadyCallback.context = frontend;
    spectrumSettings.dataReadyCallback.param = (int)spectrumEvent;

	/*Run entire spectrum "numAverages" times then take average*/

	BDBG_MSG(("\n\tnumAverages = %d \n\tnumSamples = %d \n\tnunEntries = %d \n\tmeasurementPerBin = %d \n",
			spectrum->numAverages, spectrum->numSamples, spectrum->measurementTableNumberOfEntries, spectrum->measurementsPerBin));

	for (m = 0; m < numAverages; m++){
		spectrumSettings.startFrequency = 0;
		spectrumSettings.stopFrequency = 0;

		for (j = 0; j< numOfEntries && spectrumSettings.stopFrequency < MAX_FREQ; j++) /*Scan through the whole spectrum then take average*/
		{
			spectrumSettings.data = dataPointer;
		    spectrumSettings.numSamples = numSamples;
			BKNI_Memset(&tmp, 0, MAX_NUMSAMPLES*DEFAULT_NUMENTRIES);

			if (j==0){
				spectrumSettings.startFrequency = 0 ;
			}
			else
				spectrumSettings.startFrequency += rangeOfBin;
			spectrumSettings.stopFrequency += rangeOfBin;
		    spectrumSettings.fftSize = 1024;
		    spectrumSettings.dataReadyCallback.callback = spectrum_data_ready_callback;
		    spectrumSettings.dataReadyCallback.context = frontend;
		    spectrumSettings.dataReadyCallback.param = (int)spectrumEvent;

			/*Run each segment "measurementsPerBin" (4 in this case) times then take average*/
			for (k = 0; k < measurementsPerBin; k++){
			    rc = NEXUS_Frontend_RequestSpectrumData(frontend, &spectrumSettings);
			    if (rc){rc = BERR_TRACE(rc);  goto done;}

			    rc = BKNI_WaitForEvent(spectrumEvent, 5000);
			    if (rc == NEXUS_TIMEOUT) {
			        BDBG_ERR(("Spectrum data retrieval time out\n"));
			        goto done;
			    }
				/*Collect data 256 samples/segment*/
				for (i = 0 ;i < numSamples ; i++)
				{
					tmp[i] += *(dataPointer+i);
				}
			}
			/* Take average */
			for (i = 0 ;i < numSamples ; i++)
			{
				tmp[i] = tmp[i] / measurementsPerBin;
			}

			/*  Store data in final table */
			for (i = 0 ;i < numSamples ; i++)
			{
				table[i + (numSamples * j)].amplitudeData += tmp[i];
			}
		}

	}

	/* Take average after scanning the whole spectrum "numAverages" times */

	for (i = 0 ;i < numSamples * numOfEntries; i++) {
		table[i].amplitudeData = table[i].amplitudeData / numAverages;
	}

	done:

    if (spectrumEvent) {
        BKNI_DestroyEvent(spectrumEvent);
    }
    NEXUS_Frontend_Untune(frontend);
	NEXUS_Frontend_Release(frontend);
	return 0;
}

TRX_STATUS getReferenceFrequency(char **value)
{
	int numSamples = 0;
	int measurementsPerBin = 0;
	int32_t span = 0;
	int32_t referenceFreq = 0;
	unsigned firstStopFreq = 0;
	spectrumAnalyzer *spectrum;
	int i = 0;
	int numOfEntries = 0;
	char tmp[10];
	char *sep = ", ";
	char referenceFreqArray[DEFAULT_NUMENTRIES*2*10] = "\0";

	spectrum = getSpectrumAnalyzer();
	numSamples = spectrum->numSamples;
	measurementsPerBin = spectrum->measurementsPerBin;
	numOfEntries = spectrum->measurementTableNumberOfEntries;
	span = (MAX_FREQ - MIN_FREQ)/numOfEntries;
	referenceFreq = MIN_FREQ + span/2;
	firstStopFreq = MIN_FREQ + span;

	if (table == NULL)
	{
		NEXUS_Memory_Allocate(numOfEntries*sizeof(table)*numSamples, NULL, (void *)&table);
		if(table == NULL) {
			printf("NEXUS_Memory_Allocate for table failed !!!");
			return -1;
		}
	}

	for (i = 0; i< numOfEntries; i++)
	{
		table[i].refeferenceFrequency = referenceFreq;
		snprintf(tmp, 10, "%d", table[i].refeferenceFrequency);
		strncat(referenceFreqArray, tmp, strlen(tmp));
		if (i < numOfEntries -1)
		{
			strncat(referenceFreqArray, sep, strlen(sep));
		}
		referenceFreq += span;
	}
	*value = strdup(referenceFreqArray);
	return TRX_OK;
}

TRX_STATUS getAmplitudeData(char **value)
{
	int numSamples = 0;
	spectrumAnalyzer *spectrum;
	int i = 0;
	int numOfEntries = 0;
	char tmp[10];
	char *sep = ", ";
	char amplitude[MAX_NUMSAMPLES*DEFAULT_NUMENTRIES*2*10] = "\0";

	spectrum = getSpectrumAnalyzer();
	numSamples = spectrum->numSamples;
	numOfEntries = spectrum->measurementTableNumberOfEntries;

	initMeasurementTable();

	for (i = 0; i< numOfEntries * numSamples; i++)
	{
		BDBG_MSG(("GetAmplitudeData @%d raw = %d, in dBm %d \n", i, table[i].amplitudeData, table[i].amplitudeData/256));

		/* divide raw data from NEXUS api by 256 to get value in dBm */
		snprintf(tmp, 10, "%d", (table[i].amplitudeData/256));
		strncat(amplitude, tmp, strlen(tmp));
		if (i < (numOfEntries*numSamples -1)){
			strncat(amplitude, sep, strlen(sep));
		}
	}
	*value = strdup(amplitude);
	return TRX_OK;
}

void dataCleanUp(void)
{
	if (dataPointer) {
        NEXUS_Memory_Free(dataPointer);
		dataPointer = NULL;
    }
	if (table) {
        NEXUS_Memory_Free(table);
		table = NULL;
    }
}

/* Device.X_BROADCOM_COM_SpectrumAnalyzer.ComparisonTable.{i} */


int getNewComparisonTableInstanceId(int reqId)
{
	static int ComparisonTableInstance = 0;

    /* This function returns either the requested ID */
    /* or the next allocated ID. */
    if ( reqId == 0)
        return ComparisonTableInstance;

    if (reqId > ComparisonTableInstance)
    {
		ComparisonTableInstance = reqId;
        return ComparisonTableInstance;
    }
    else
    {
        return ++ComparisonTableInstance;
    }
}

int getComparisonTableInstanceIndex(InstanceDesc *idp)
{
	#if 0
    int currentIndex;

    if (idp == NULL )
        return 0;

    currentIndex = BfcTr69Api_SnmpToTr69Index(idp->hwUserData);
	#endif
	int currentIndex =1;
	if (idp == NULL )
        return 0;
	return currentIndex;
}


/*
* This function creates corresponding ComparisonTable entry in SNMP.
* There is one-to-one mapping between tr69 and SNMP for ComparisonTable entries.
*/

int Bfc_AddComparisonTableInstance(char *info)
{
	TRxObjNode *n;
    InstanceDesc *idp;
    int num;
	bool instanceExist = false;
    int i,j;
    n = comparisonTableDesc;

    /*Check if there is existing idp correspondig to instance ID */
    j = getNewComparisonTableInstanceId(0);

    for(i = 1; i <= j; i++)
    {
        instanceExist = false;

        if( (idp = findInstanceDescNoPathCheck(n, i)) == NULL )
        {
            /*create new instance entry for this id            */
            idp = getNewInstanceDesc(n, NULL, i);
            if (idp != NULL)
            {
                idp->hwUserData = info;
                return 1;
            }
        }
        else
        {
            if(i == skipID)
            {
                instanceExist = true;
                break;
            }
        }
    }

    /* if there is no instance create new instance entry   */
    if (instanceExist == false)
    {
        num = getNewComparisonTableInstanceId(1);
        idp = getNewInstanceDesc(n, NULL, num);

        if (idp != NULL)
        {
            idp->hwUserData = info;
            return 1;
        }
    }
    skipID = 0;

    return 0;
}


int Bfc_DeleteComparisonTableInstance(char *info)
{
    TRxObjNode *n;
    InstanceDesc *idp;
    int deleteIndex;
    int currentIndex;
    void * idpValue = info;
	int i,j;

    n = comparisonTableDesc;
	j = getNewComparisonTableInstanceId(0);
    /*deleteIndex = BfcTr69Api_SnmpToTr69Index(idpValue);*/
	deleteIndex = 1;
	BSTD_UNUSED(idpValue);

    for(i = 1; i <= j; i++)
    {
		if( (idp = findInstanceDescNoPathCheck(n, i)) != NULL )
        {
            if (idp->hwUserData != NULL)
            {
                /*currentIndex = BfcTr69Api_SnmpToTr69Index(idp->hwUserData);               */
				currentIndex = 1;
                if (currentIndex == deleteIndex)
                {
                    if ( !deleteInstanceDesc(n, idp->instanceID) )
                    {
                        return 1;
                    }
                    else
                    {
						printf("\nFailed to delete ComparisonTableInstace: ");
                    }
                }
            }
        }
    }

    return 0;
}

TRX_STATUS getCompEnable(char **value)
{
    InstanceDesc *idp;
    /*int status = 0;*/
    char *compEnable = "false";

    if ((idp = getCurrentInstanceDesc()) == NULL )
        return TRX_ERR;
	#if 0
    if (idp->hwUserData != NULL)
    {
        if((BfcTr69Api_GetIntFromSnmp(kOID_cmReferenceSpectrumCompStatus, idp->hwUserData, &status)) != 0)
        {
            if (status == SNMP_STATUS_ACTIVE)
				compEnable = "true";
        }
    }
	#endif
    *value = strdup(compEnable);

    return TRX_OK;
}

TRX_STATUS setCompEnable(char *value)
{
    InstanceDesc *idp;
    TRX_STATUS retVal = TRX_ERR;
	#if 0
    int disableSnmpEntry = 0/*SNMP_STATUS_NOT_IN_SERVICE*/;
    int enableSnmpEntry = 1/*SNMP_STATUS_ACTIVE*/;
	#endif

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

    if ((idp = getCurrentInstanceDesc()) == NULL )
        return TRX_ERR;

    if ((strcmp(value, "true") == 0) || (strcmp(value, "1") == 0))
    {
        if (idp->hwUserData != NULL)
        {
            /*BfcTr69Api_SnmpSetInt(kOID_cmReferenceSpectrumCompStatus, idp->hwUserData, enableSnmpEntry);*/
            retVal = TRX_OK;
        }
        else
        {
            printf("\nEnter CompFrequency using setObject to enable the entry.\n");
            retVal = TRX_ERR;
        }
    }

    if ((strcmp(value, "false") == 0) || (strcmp(value, "0") == 0))
    {
        /*BfcTr69Api_SnmpSetInt(kOID_cmReferenceSpectrumCompStatus, idp->hwUserData, disableSnmpEntry); */
        /*idp->hwUserData = NULL; */
        retVal = TRX_OK;
    }

    return retVal;
}

TRX_STATUS getCompStatus(char **value)
{
    char *compSatus = "DISABLED";

    *value = strdup(compSatus);
    return TRX_OK;
}

TRX_STATUS getCompFrequency(char **value)
{
	InstanceDesc *idp;
	spectrumAnalyzer *spectrum;
	char tmp[15];

	if ((idp = getCurrentInstanceDesc()) == NULL )
		return TRX_ERR;

    if(idp->hwUserData == NULL)
    {
        *value = strdup((char *) "");
        return TRX_OK;
    }
	spectrum = getSpectrumAnalyzer();
	snprintf(tmp, 10, "%d", spectrum->centerFrequency);
	*value = strdup(tmp);
	return TRX_OK;
}

TRX_STATUS setCompFrequency(char *value)
{
    InstanceDesc *idp;
	spectrumAnalyzer *spectrum;
	int freq;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	freq = atoi(value);

    if ((idp = getCurrentInstanceDesc()) == NULL )
        return TRX_ERR;
	/*getSpectrumAnalyzer(&spectrum, &numEntries);*/
	spectrum = getSpectrumAnalyzer();
	setCenterFreq(spectrum, freq);
	return TRX_OK;
}


TRX_STATUS getCompWidth(char **value)
{
	InstanceDesc *idp;
	spectrumAnalyzer *spectrum;
	char tmp[15];

	if ((idp = getCurrentInstanceDesc()) == NULL )
		return TRX_ERR;

    if(idp->hwUserData == NULL)
    {
        *value = strdup((char *) "");
        return TRX_OK;
    }
	spectrum = getSpectrumAnalyzer();
	snprintf(tmp, 10, "%d", spectrum->span);
	*value = strdup(tmp);

	return TRX_OK;
}

TRX_STATUS setCompWidth(char *value)
{
    InstanceDesc *idp;
	spectrumAnalyzer *spectrum;
    int compWidth = 0;

	compWidth = atoi(value);
    if ((idp = getCurrentInstanceDesc()) == NULL )
        return TRX_ERR;
	if ((compWidth >= MIN_SPAN) &&  (compWidth <= MAX_SPAN ))
    {
		spectrum = getSpectrumAnalyzer();
		spectrum->span = compWidth;
        return TRX_OK;
    }
    return TRX_ERR;
}


TRX_STATUS getCompStoredPower(char **value)
{
	InstanceDesc *idp;

	if ((idp = getCurrentInstanceDesc()) == NULL )
		return TRX_ERR;

    if(idp->hwUserData == NULL)
    {
        *value = strdup((char *) "");
        return TRX_OK;
    }
	return TRX_OK;
}


TRX_STATUS getCompCurPower(char **value)
{
	InstanceDesc *idp;

	if ((idp = getCurrentInstanceDesc()) == NULL )
		return TRX_ERR;

    if(idp->hwUserData == NULL)
    {
        *value = strdup((char *) "");
        return TRX_OK;
    }
	return TRX_OK;
}


TRX_STATUS getCompThresholdExceeded(char **value)
{
	InstanceDesc *idp;

	if ((idp = getCurrentInstanceDesc()) == NULL )
		return TRX_ERR;

    if(idp->hwUserData == NULL)
    {
        *value = strdup((char *) "");
        return TRX_OK;
    }
	return TRX_OK;
}


TRX_STATUS addComparisonTableInstance(char **value)
{
	InstanceDesc *idp;
    TRxObjNode *n;
    int entryId = 0;

    n = comparisonTableDesc;
    entryId = getNewComparisonTableInstanceId(1);
    idp = getNewInstanceDesc(n, NULL, entryId);

    if ( idp != NULL )
    {
        idp->hwUserData = NULL;
        *value = strdup((char *) itoa(idp->instanceID));
        return TRX_OK;
    }

    return TRX_ERR;
}

/*
* This function deletes corresponding ComparisonTable entry from SNMP.
* There is one-to-one mapping between tr69 and SNMP for ComparisonTable entries.
*/

#if 0

void DestroySnmpEntry (unsigned int CenterFrequencyHz)
{
    BcmMibTable *pTable = CmSnmpAgent::Singleton().FindTable(BcmObjectId(kOID_cmReferenceSpectrumComparisonTable));

    if (pTable != NULL)
        pTable->DestroyEntry(BcmObjectId(CenterFrequencyHz));
}
#endif

TRX_STATUS deleteComparisonTableInstance(char *value)
{
	BSTD_UNUSED(value);
	return TRX_OK;
}

/* Device.X_BROADCOM_COM_SpectrumAnalyzer. */

TRX_STATUS getStartFreq(char **value)
{
	spectrumAnalyzer *spectrum;
	char tmp[15];

	spectrum = getSpectrumAnalyzer();
	snprintf(tmp, 10, "%d", spectrum->startFrequency);
	*value = strdup(tmp);
	return TRX_OK;
}

TRX_STATUS setStartFreq(char *value)
{
	spectrumAnalyzer *spectrum;
    unsigned startFreq;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	startFreq = atoi(value);

    if (((int)startFreq < MIN_FREQ) || ((int)startFreq > MAX_FREQ))
        return TRX_ERR;
	else
	{
		spectrum = getSpectrumAnalyzer();
		spectrum->startFrequency= startFreq;
		return TRX_OK;
	}
}

TRX_STATUS getStopFreq(char **value)
{
	spectrumAnalyzer *spectrum;
	char tmp[15];

	spectrum = getSpectrumAnalyzer();
	snprintf(tmp, 10, "%d", spectrum->stopFrequency);
	*value = strdup(tmp);
	return TRX_OK;
}

TRX_STATUS setStopFreq(char *value)
{
	spectrumAnalyzer *spectrum;
    unsigned stopFreq;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	stopFreq = atoi(value);

    if (((int)stopFreq < MIN_FREQ) || ((int)stopFreq > MAX_FREQ))
        return TRX_ERR;
	else
	{
		spectrum = getSpectrumAnalyzer();
		spectrum->stopFrequency= stopFreq;
		return TRX_OK;
	}
}

TRX_STATUS getNumSamples(char **value)
{
	spectrumAnalyzer *spectrum;
	char tmp[15];

	spectrum = getSpectrumAnalyzer();
	snprintf(tmp, 10, "%d", spectrum->numSamples);
	*value = strdup(tmp);
	return TRX_OK;
}

TRX_STATUS setNumSamples(char *value)
{
	spectrumAnalyzer *spectrum;
    int numSamples;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	numSamples = atoi(value);

    if ((numSamples <= MIN_NUMSAMPLES) || (numSamples > MAX_NUMSAMPLES))
        return TRX_ERR;
	else
	{
		spectrum = getSpectrumAnalyzer();
		spectrum->numSamples = numSamples;
		return TRX_OK;
	}
}

TRX_STATUS getMeasurementsPerBin(char **value)
{
	spectrumAnalyzer *spectrum;
	char tmp[15];

	spectrum = getSpectrumAnalyzer();
	snprintf(tmp, 10, "%d", spectrum->measurementsPerBin);
	*value = strdup(tmp);
	return TRX_OK;
}


TRX_STATUS setMeasurementsPerBin(char *value)
{
	spectrumAnalyzer *spectrum;
	int measurementsPerBin = 0;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

    measurementsPerBin = atoi(value);

    if ((measurementsPerBin >= MIN_MEASUREMENTSPERBIN)
    &&  (measurementsPerBin <= MAX_MEASUREMENTSPERBIN))
    {
		spectrum = getSpectrumAnalyzer();
		spectrum->measurementsPerBin = measurementsPerBin;
        return TRX_OK;
    }
    return TRX_ERR;
}


TRX_STATUS getNumAverages(char **value)
{
	spectrumAnalyzer *spectrum;
	char tmp[15];

	spectrum = getSpectrumAnalyzer();
	snprintf(tmp, 10, "%d", spectrum->numAverages);
	*value = strdup(tmp);
	return TRX_OK;
}


TRX_STATUS setNumAverages(char *value)
{
	spectrumAnalyzer *spectrum;
    int numAverages;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	numAverages = atoi(value);

	if(value == NULL)
		return TRX_ERR;
    if ((numAverages >= MIN_NUMAVERAGES) && (numAverages <= MAX_NUMAVERAGES))
    {
		spectrum = getSpectrumAnalyzer();
		spectrum->numAverages = numAverages;
        return TRX_OK;
    }
	return TRX_ERR;
}


TRX_STATUS getMeasureAndStore(char **value)
{
	spectrumAnalyzer *spectrum;

	spectrum = getSpectrumAnalyzer();
	if (spectrum->measureAndStore)
		*value = strdup("true");
	*value = strdup("false");
	return TRX_OK;
}


TRX_STATUS setMeasureAndStore(char *value)
{
	spectrumAnalyzer *spectrum;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	spectrum = getSpectrumAnalyzer();
    if (strcmp(value, (char *) "false") == 0)
    {
		spectrum->measureAndStore = false;
        return TRX_OK;
    }

    if (strcmp(value, (char *) "true") == 0)
    {
		spectrum->measureAndStore = true;
        return TRX_OK;
    }
    return TRX_ERR;
}


TRX_STATUS getComparisonThreshold(char **value)
{
	BSTD_UNUSED(value);
	return TRX_OK;
}


TRX_STATUS setComparisonThreshold(char *value)
{
	int numComparisonThreshold;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

    numComparisonThreshold = atoi(value);

    if ((numComparisonThreshold > MIN_SPECTRUMCOMPARISONTHRESHOLD)
    &&  (numComparisonThreshold < MAX_SPECTRUMCOMPARISONTHRESHOLD))
        return TRX_OK;
    return TRX_ERR;
}


TRX_STATUS getComparisonInterval(char **value)
{
	BSTD_UNUSED(value);
	return TRX_OK;
}


TRX_STATUS setComparisonInterval(char *value)
{
	int numComparisonInterval;
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

    numComparisonInterval = atoi(value);
    int const MIN_COMPARISONINTERVAL = 0;
    int const MAX_COMPARISONINTERVAL = 2147483647;

    if ((numComparisonInterval > MIN_COMPARISONINTERVAL)
    &&  (numComparisonInterval < MAX_COMPARISONINTERVAL))
        return TRX_OK;
    return TRX_ERR;
}


TRX_STATUS getMeasurementTableNumberofEntries(char **value)
{
	spectrumAnalyzer *spectrum;
	int numOfEntries = 0;
	char tmp[4];

	spectrum = getSpectrumAnalyzer();
	numOfEntries = spectrum->measurementTableNumberOfEntries;
	snprintf(tmp, 4, "%d", numOfEntries);
	*value = strdup(tmp);
	return TRX_OK;
}


TRX_STATUS getComparisonTableNumberofEntries(char **value)
{
	int num = 0;
    num = getInstanceCount(comparisonTableDesc);

    /* If there are no entries, add the one and only*/
    if (num == 0) {
        /*Bfc_AddComparisonTableInstance(NULL);*/
        num = getInstanceCount(comparisonTableDesc);
    }

    *value = strdup((char *) itoa(num));

	return TRX_OK;
}
