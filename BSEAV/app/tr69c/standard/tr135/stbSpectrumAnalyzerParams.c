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




#include "sharedparams.h"
#include "stbSpectrumAnalyzerParams.h"


/* Device.X_BROADCOM_COM_SpectrumAnalyzer.MeasurementTable.{i} */

TRXGFUNC(getReferenceFrequency);
TRXGFUNC(getAmplitudeData);

TRxObjNode  measurementTableInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {ReferenceFrequency, {{tInt, 0, 0}}, NULL, getReferenceFrequency, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AmplitudeData, {{tObject, 0, 0}}, NULL, getAmplitudeData, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0,0,0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {ReferenceFrequency, {{tInt, 0, 0}}, NULL, getReferenceFrequency, NULL, NULL},
    {AmplitudeData, {{tObject, 0, 0}}, NULL, getAmplitudeData, NULL, NULL},
	{NULL, {{0,0,0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.X_BROADCOM_COM_SpectrumAnalyzer.MeasurementTable */
TRxObjNode  measurementTableDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK, {{0, 0, 0}}, NULL, NULL, measurementTableInstanceDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0,0,0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {instanceIDMASK, {{0, 0, 0}}, NULL, NULL, measurementTableInstanceDesc, NULL},
    {NULL, {{0,0,0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.X_BROADCOM_COM_SpectrumAnalyzer.ComparisonTable.{i} */

TRXGFUNC(getCompEnable);
TRXSFUNC(setCompEnable);
TRXGFUNC(getCompStatus);
TRXGFUNC(getCompFrequency);
TRXSFUNC(setCompFrequency);
TRXGFUNC(getCompWidth);
TRXSFUNC(setCompWidth);
TRXGFUNC(getCompStoredPower);
TRXGFUNC(getCompCurPower);
TRXGFUNC(getCompThresholdExceeded);

TRxObjNode  comparisonTableInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompEnable, getCompEnable, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ssvStatus, {{tString, 0, 0}}, NULL, getCompStatus, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {CompFrequency, {{tInt, 0, 0}}, setCompFrequency, getCompFrequency, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {CompWidth, {{tInt, 0, 0}}, setCompWidth, getCompWidth, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {CompStoredPower, {{tInt, 0, 0}}, NULL, getCompStoredPower, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {CompCurPower, {{tInt, 0, 0}}, NULL, getCompCurPower, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {CompThresholdExceeded, {{tBool, 0, 0}}, NULL, getCompThresholdExceeded, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0,0,0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompEnable, getCompEnable, NULL, NULL},
    {ssvStatus, {{tString, 0, 0}}, NULL, getCompStatus, NULL, NULL},
    {CompFrequency, {{tInt, 0, 0}}, setCompFrequency, getCompFrequency, NULL, NULL},
    {CompWidth, {{tInt, 0, 0}}, setCompWidth, getCompWidth, NULL, NULL},
    {CompStoredPower, {{tInt, 0, 0}}, NULL, getCompStoredPower, NULL, NULL},
    {CompCurPower, {{tInt, 0, 0}}, NULL, getCompCurPower, NULL, NULL},
    {CompThresholdExceeded, {{tBool, 0, 0}}, NULL, getCompThresholdExceeded, NULL, NULL},
    {NULL, {{0,0,0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.X_BROADCOM_COM_SpectrumAnalyzer.ComparisonTable */

TRXGFUNC(addComparisonTableInstance);
TRXSFUNC(deleteComparisonTableInstance);

TRxObjNode  comparisonTableDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK, {{0, 0, 0}}, deleteComparisonTableInstance, addComparisonTableInstance, comparisonTableInstanceDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0,0,0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {instanceIDMASK, {{0, 0, 0}}, deleteComparisonTableInstance, addComparisonTableInstance, comparisonTableInstanceDesc, NULL},
    {NULL, {{0,0,0}}, NULL, NULL, NULL, NULL}
#endif
};


/* Device.X_BROADCOM_COM_SpectrumAnalyzer. */

TRXGFUNC(getNumSamples);
TRXSFUNC(setNumSamples);
TRXGFUNC(getMeasurementsPerBin);
TRXSFUNC(setMeasurementsPerBin);
TRXGFUNC(getNumAverages);
TRXSFUNC(setNumAverages);
TRXGFUNC(getMeasureAndStore);
TRXSFUNC(setMeasureAndStore);
TRXGFUNC(getComparisonThreshold);
TRXSFUNC(setComparisonThreshold);
TRXGFUNC(getComparisonInterval);
TRXSFUNC(setComparisonInterval);
TRXGFUNC(getMeasurementTableNumberofEntries);
TRXGFUNC(getComparisonTableNumberofEntries);
TRXGFUNC(setStartFreq);
TRXGFUNC(getStartFreq);
TRXGFUNC(setStopFreq);
TRXGFUNC(getStopFreq);



TRxObjNode  X_BROADCOM_COM_spectrumAnalyzerDesc[] = {
#ifdef XML_DOC_SUPPORT
    {NumSamples, {{tInt, 0, 0}}, setNumSamples, getNumSamples, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MeasurementsPerBin, {{tInt, 0, 0}}, setMeasurementsPerBin, getMeasurementsPerBin, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NumAverages, {{tInt, 0, 0}}, setNumAverages, getNumAverages, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MeasureAndStore, {{tBool, 0, 0}}, setMeasureAndStore, getMeasureAndStore, NULL, NULL, 1, 0, 0, 0, NULL, true},
#ifdef SUPPORT
	{startFrequency, {{tInt, 0, 0}}, setStartFreq, getStartFreq, NULL, NULL, 1, 0, 0, 0, NULL, true},
	{stopFrequency, {{tInt, 0, 0}}, setStopFreq, getStopFreq, NULL, NULL, 1, 0, 0, 0, NULL, true},
#endif
    {ComparisonThreshold, {{tInt, 0, 0}}, setComparisonThreshold, getComparisonThreshold, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ComparisonInterval, {{tInt, 0, 0}}, setComparisonInterval, getComparisonInterval, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MeasurementTableNumberofEntries, {{tUnsigned, 0, 0}}, NULL, getMeasurementTableNumberofEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ComparisonTableNumberofEntries, {{tUnsigned, 0, 0}}, NULL, getComparisonTableNumberofEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MeasurementTable, {{tInstance,0,0}}, NULL, NULL, measurementTableDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {ComparisonTable, {{tInstance,0,0}}, NULL, NULL, comparisonTableDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0,0,0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {NumSamples, {{tInt, 0, 0}}, setNumSamples, getNumSamples, NULL, NULL},
    {MeasurementsPerBin, {{tInt, 0, 0}}, setMeasurementsPerBin, getMeasurementsPerBin, NULL, NULL},
    {NumAverages, {{tInt, 0, 0}}, setNumAverages, getNumAverages, NULL, NULL},
    {MeasureAndStore, {{tBool, 0, 0}}, setMeasureAndStore, getMeasureAndStore, NULL, NULL},
#ifdef SUPPORT
	{startFrequency, {{tInt, 0, 0}}, setStartFreq, getStartFreq, NULL, NULL},
	{stopFrequency, {{tInt, 0, 0}}, setStopFreq, getStopFreq, NULL, NULL},
#endif
    {ComparisonThreshold, {{tInt, 0, 0}}, setComparisonThreshold, getComparisonThreshold, NULL, NULL},
    {ComparisonInterval, {{tInt, 0, 0}}, setComparisonInterval, getComparisonInterval, NULL, NULL},
    {MeasurementTableNumberofEntries, {{tUnsigned, 0, 0}}, NULL, getMeasurementTableNumberofEntries, NULL, NULL},
    {ComparisonTableNumberofEntries, {{tUnsigned, 0, 0}}, NULL, getComparisonTableNumberofEntries, NULL, NULL},
    {MeasurementTable, {{tInstance,0,0}}, NULL, NULL, measurementTableDesc, NULL},
    {ComparisonTable, {{tInstance,0,0}}, NULL, NULL, comparisonTableDesc, NULL},
    {NULL, {{0,0,0}}, NULL, NULL, NULL, NULL}
#endif
};
