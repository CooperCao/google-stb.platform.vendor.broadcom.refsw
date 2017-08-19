/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#ifdef SNMP_SUPPORT

#include "tuner.h"
#include "tuner_oob.h"
#include "tuner_upstream.h"
#include "snmp.h"
#ifdef CDL_SUPPORT
#include "b_cdl_lib.h"
#endif
#ifdef ESTB_CFG_SUPPORT
#include "b_estb_cfg_lib.h"
#endif
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

CTunerOob * g_pTunerOob = NULL;
CTunerUpstream * g_pTunerUpstream = NULL;
CControl * g_pControl = NULL;
CSnmp * g_pSnmp = NULL;

BDBG_MODULE(snmp);

#define ELAPSED_TIME 50 /* 500 ms, in unit of 10ms */
#define LONG_ELAPSED_TIME 300 /* 3 seconds, in unit of 10ms */
#define MAX_DECODER	2

static containerDumpTrapInfo dumpTrapInfo = { 0, false, 5, "./dumpfile" };

CSnmp::CSnmp():
	CMvcModel("snmp"),
    snmp_thread(0)
{
}

CSnmp::~CSnmp()
{
}

eRet CSnmp::snmp_save_oob(CTunerOob * pTuner)
{
    eRet ret = eRet_Ok;
    g_pTunerOob = pTuner;
    return ret;
}

eRet CSnmp::snmp_save_upstream(CTunerUpstream * pTuner)
{
    eRet ret = eRet_Ok;
    g_pTunerUpstream = pTuner;
    return ret;
}

eRet CSnmp::snmp_save_control(CControl * pControl)
{
    eRet ret = eRet_Ok;
    g_pControl = pControl;
    return ret;
}

eRet CSnmp::snmp_init(CModel * pModel)
{
    eRet ret = eRet_Ok;
    int rc = 0;

	BDBG_ASSERT(pModel != NULL);
    _pModel = pModel;

    B_SNMP_Init();
    snmp_proxy_mibs_open();
    snmp_proxy_mibs_register();

    BDBG_MSG(("Lauching snmp proxy thread"));
    rc = pthread_create(&snmp_thread, NULL, (void *(*)(void *))(B_SNMP_Main), NULL);
	BDBG_ASSERT(!rc);

	g_pSnmp = this;

    return ret;
}

eRet CSnmp::snmp_uninit(void)
{
    eRet ret = eRet_Ok;

    snmp_proxy_mibs_unregister();
    snmp_proxy_mibs_close();
    B_SNMP_Shutdown();
    g_pTunerOob = NULL;
    g_pTunerUpstream = NULL;
    g_pControl = NULL;

    return ret;
}

bool CSnmp::snmp_check_display_connection(eBoardResource type)
{
    CSnmp * pSnmp = g_pSnmp;
    MList <CDisplay> * pListDisplays = NULL;
    CDisplay * pDisplay = NULL;
    MList <COutput> * pListOutputs = NULL;
    COutput * pOutput = NULL;
    bool connected = false;

    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    if ((pListDisplays = pSnmp->_pModel->getDisplayList()) != NULL)
    {
        for (pDisplay = pListDisplays->first(); pDisplay; pDisplay = pListDisplays->next())
        {
            pListOutputs = pDisplay->getOutputList();
            for (pOutput = pListOutputs->first(); pOutput; pOutput = pListOutputs->next())
            {
                switch (pOutput->getType())
                {
                    case eBoardResource_outputComponent:
                        BDBG_MSG(("Component -> Display(%d)", pDisplay->getNumber()));
                        if (type == eBoardResource_outputComponent)
                        {
                            if(!connected) connected = true;
                            else BDBG_WRN(("already connected!"));
                        }
                        break;
                    case eBoardResource_outputSVideo:
                        BDBG_MSG(("SVideo -> Display(%d)", pDisplay->getNumber()));
                        if (type == eBoardResource_outputSVideo)
                        {
                            if(!connected) connected = true;
                            else BDBG_WRN(("already connected!"));
                        }
                        break;
                    case eBoardResource_outputComposite:
                        BDBG_MSG(("Composite -> Display(%d)", pDisplay->getNumber()));
                        if (type == eBoardResource_outputComposite)
                        {
                            if(!connected) connected = true;
                            else BDBG_WRN(("already connected!"));
                        }
                        break;
                    case eBoardResource_outputRFM:
                        BDBG_MSG(("RFM -> Display(%d)", pDisplay->getNumber()));
                        if (type == eBoardResource_outputRFM)
                        {
                            if(!connected) connected = true;
                            else BDBG_WRN(("already connected!"));
                        }
                        break;
                    case eBoardResource_outputHdmi:
                        BDBG_MSG(("HDMI/DVI -> Display(%d)", pDisplay->getNumber()));
                        if (type == eBoardResource_outputHdmi)
                        {
                            if(!connected) connected = true;
                            else BDBG_WRN(("already connected!"));
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return connected;
}

void CSnmp::snmp_get_hw_indentifiers(void *context, int param)
{
	mibObjectHwIdentifiers *pObj = (mibObjectHwIdentifiers *) context;
	unsigned char buf[5];
	int length;

	BSTD_UNUSED(param);

#ifdef ESTB_CFG_SUPPORT
	B_Estb_cfg_Get_bin((char *)"/dyn/estb/host_id", buf, &length);
    if (length != 5) BKNI_Memset(buf, 0, 5);
#endif
	sprintf(pObj->container.serialNumber, "%s", "BROADCOM-BCM974XX");
	sprintf(pObj->container.hostId, "%s", "0-000-000-000-000");
	pObj->container.capabilities = oc_hostCapabilities_ochd21;
	pObj->container.avcSupport = oc_true;

	return;
}

void CSnmp::snmp_get_ieee1394_table(void *context, int param)
{
	mibObjectIeee1394Table *pObj = (mibObjectIeee1394Table *) context;

	pObj->pContainer[param]->activeNodes = 0;
	pObj->pContainer[param]->dataXMission = false;
	pObj->pContainer[param]->dtcpStatus = false;
	pObj->pContainer[param]->loopStatus = false;
	pObj->pContainer[param]->rootStatus = false;
	pObj->pContainer[param]->cycleIsMaster = false;
	pObj->pContainer[param]->irmStatus = false;
	pObj->pContainer[param]->audioMuteStatus = false;
	pObj->pContainer[param]->videoMuteStatus = false;

    return;
}

void CSnmp::snmp_get_ieee1394_connected_devices_table(void *context, int param)
{
	mibObjectIeee1394ConnectedDevicesTable *pObj = (mibObjectIeee1394ConnectedDevicesTable *) context;

	pObj->pContainer[param]->subUnitType = oc_1394ConnectedDevicesSubUnitType_other;

	pObj->pContainer[param]->eui64[0] = 0x12;
	pObj->pContainer[param]->eui64[1] = 0x34;
	pObj->pContainer[param]->eui64[2] = 0x56;
	pObj->pContainer[param]->eui64[3] = 0x78;
	pObj->pContainer[param]->eui64[4] = 0x9a;
	pObj->pContainer[param]->eui64[5] = 0xbc;
	pObj->pContainer[param]->eui64[6] = 0xde;
	pObj->pContainer[param]->eui64[7] = 0xff;

	pObj->pContainer[param]->analogDigitalSourceSelection = false;

    return;
}

void CSnmp::snmp_get_dvi_hdmi_table(void *context, int param)
{
#if NEXUS_NUM_HDMI_OUTPUTS
	mibObjectDviHdmiTable *pObj = (mibObjectDviHdmiTable *) context;
	NEXUS_PlatformConfiguration configuration;
	NEXUS_HdmiOutputStatus hdmiStatus;
	NEXUS_HdmiOutputHdcpStatus hdcpStatus;
	NEXUS_HdmiOutputHdcpSettings hdcpSettings;
	NEXUS_HdmiOutputEdidBlock edidBlock;
	NEXUS_HdmiOutputBasicEdidData basicEdidData;
	NEXUS_HdmiOutputSupportedVideoInfo supportedVideoInfo;
	NEXUS_AudioOutputSettings audioOutputSettings;
	NEXUS_VideoDecoderStatus videoDecoderStatus;
	NEXUS_DisplaySettings displaySettings;
	NEXUS_VideoFormatInfo videoFormatInfo;
	NEXUS_VideoFrameRate frameRate = NEXUS_VideoFrameRate_eUnknown;
	NEXUS_Error retVal = BERR_SUCCESS;

    CSnmp * pSnmp = g_pSnmp;
    CSimpleAudioDecode * pSimpleAudioDecode = NULL;
    CSimpleVideoDecode * pSimpleVideoDecode = NULL;
    CVideoDecode * pVideoDecode = NULL;
    CDisplay * pDisplay = NULL;

    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    BKNI_Memset(&hdmiStatus, 0, sizeof(NEXUS_HdmiOutputStatus));
    BKNI_Memset(&hdcpStatus, 0, sizeof(NEXUS_HdmiOutputHdcpStatus));
    BKNI_Memset(&hdcpSettings, 0, sizeof(NEXUS_HdmiOutputHdcpSettings));
    BKNI_Memset(&edidBlock, 0, sizeof(NEXUS_HdmiOutputEdidBlock));
	BKNI_Memset(&basicEdidData, 0, sizeof(NEXUS_HdmiOutputBasicEdidData));
	BKNI_Memset(&supportedVideoInfo, 0, sizeof(NEXUS_HdmiOutputSupportedVideoInfo));
	BKNI_Memset(&audioOutputSettings, 0, sizeof(NEXUS_AudioOutputSettings));
    BKNI_Memset(&videoDecoderStatus, 0, sizeof(NEXUS_VideoDecoderStatus));
	BKNI_Memset(&displaySettings, 0, sizeof(NEXUS_DisplaySettings));
	BKNI_Memset(&videoFormatInfo, 0, sizeof(NEXUS_VideoFormatInfo));
    BKNI_Memset(pObj->pContainer[param], 0, sizeof(containerDviHdmiTable));

    if ((pSimpleAudioDecode = pSnmp->_pModel->getSimpleAudioDecode(pSnmp->_pModel->getFullScreenWindowType())) != NULL)
    {
        pObj->audioDecoder[0] = pSimpleAudioDecode->getDecoder();
    }
    else
    {
        pObj->audioDecoder[0] = NULL;
    }

    if ((pSimpleVideoDecode = pSnmp->_pModel->getSimpleVideoDecode(pSnmp->_pModel->getFullScreenWindowType())) != NULL)
    {
        if ((pVideoDecode = pSimpleVideoDecode->getVideoDecoder()) != NULL)
        {
            pObj->videoDecoder[0] = pVideoDecode->getDecoder();
        }
        else
        {
            pObj->videoDecoder[0] = NULL;
        }
    }
    else
    {
        pObj->videoDecoder[0] = NULL;
    }


    if ((pDisplay = pSnmp->_pModel->getDisplay(0)) != NULL)
    {
        pObj->display[0] = pDisplay->getDisplay();
    }
    else
    {
        pObj->display[0] = NULL;
    }

	NEXUS_Platform_GetConfiguration(&configuration);
	NEXUS_HdmiOutput_GetStatus(configuration.outputs.hdmi[param], &hdmiStatus);
	if (hdmiStatus.hdmiDevice)
	{
		NEXUS_HdmiOutput_GetHdcpSettings(configuration.outputs.hdmi[param], &hdcpSettings);
		retVal = NEXUS_HdmiOutput_GetHdcpStatus(configuration.outputs.hdmi[param], &hdcpStatus);
		NEXUS_HdmiOutput_GetEdidBlock(configuration.outputs.hdmi[param], 0, &edidBlock);
		NEXUS_HdmiOutput_GetBasicEdidData(configuration.outputs.hdmi[param], &basicEdidData);
		NEXUS_HdmiOutput_GetSupportedVideoInfo(configuration.outputs.hdmi[param], &supportedVideoInfo);
	}
    if (pObj->videoDecoder[param])
	    NEXUS_VideoDecoder_GetStatus(pObj->videoDecoder[param], &videoDecoderStatus);
    if (pObj->display[param])
    {
        NEXUS_Display_GetSettings(pObj->display[param], &displaySettings);
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
        NEXUS_LookupFrameRate(videoFormatInfo.verticalFreq*10, &frameRate);
    }
	NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(configuration.outputs.hdmi[param]), &audioOutputSettings);

	pObj->pContainer[param]->hdmiDevice = hdmiStatus.hdmiDevice;
	pObj->pContainer[param]->connectionStatus = hdmiStatus.connected;
	pObj->pContainer[param]->repeaterStatus = (retVal == BERR_SUCCESS) ? hdcpStatus.isHdcpRepeater : false;
	pObj->pContainer[param]->videoXMissionStatus = hdmiStatus.connected && videoDecoderStatus.started;
	pObj->pContainer[param]->hdcpStatus = (retVal == BERR_SUCCESS) ? hdcpStatus.transmittingEncrypted : false;
	pObj->pContainer[param]->videoMuteStatus = videoDecoderStatus.muted;
	pObj->pContainer[param]->outputFormat = hdmiStatus.videoFormat;
	pObj->pContainer[param]->aspectRatio = hdmiStatus.aspectRatio;
	pObj->pContainer[param]->hostDeviceHdcpStatus = retVal == BERR_SUCCESS;
	pObj->pContainer[param]->audioFormat = hdmiStatus.audioFormat;
	pObj->pContainer[param]->audioSampleRate = hdmiStatus.audioSamplingRate;
	pObj->pContainer[param]->audioChannelCount = hdmiStatus.maxAudioPcmChannels;
	pObj->pContainer[param]->audioMuteStatus = audioOutputSettings.muted;
	pObj->pContainer[param]->audioSampleSize = hdmiStatus.audioSamplingSize;
	pObj->pContainer[param]->colorSpace = hdmiStatus.colorSpace;
	pObj->pContainer[param]->frameRate = frameRate;
	pObj->pContainer[param]->attachedDeviceType = oc_attachedDeviceType_tv;
	BKNI_Memcpy(pObj->pContainer[param]->edid, edidBlock.data, 128);
	pObj->pContainer[param]->cecFeatures = oc_cecFeatures_systemStandby;
	pObj->pContainer[param]->features = oc_hdmiFeatures_cec;
	pObj->pContainer[param]->lipSyncDelay = 0; /* Auto Lipsync Correction Feature is not supported */
#if BCHP_CHIP == 7420
	pObj->pContainer[param]->maxDeviceCount = hdcpSettings.maxDeviceCountSupported;
#else
	pObj->pContainer[param]->maxDeviceCount = 127;
#endif
	sprintf(pObj->pContainer[param]->edidVersion, "%02X.%02X", basicEdidData.edidVersion, basicEdidData.edidRevision);
	pObj->pContainer[param]->threeDCompatibilityControl = oc_3DCompatibilityControl_other;
	pObj->pContainer[param]->threeDCompatibilityMsgDisplay = false;

	/*
	** The Host2.1 CFR requires HDMI 1.3a that in turn REQUIRES the HDMI Sink to use the Short Video Descriptor (SVD)
	** that encapsulates the VIC for the CEA-861 video formats.  The Sink MAY also use the detailed timing descriptor
	** which allows for PC formats. CEA-861-E states the SVDs shall be listed in order of priority with an informative
	** section clarifying with "The preferred SVD is listed first".
	** Keep in mind there is no guarantee the first VIC is the preferred code.
	** As we have seen at Plugfests, some manufacturers still do not specify the VIC in preferred order.
	** In practice this is not always the case so be prepared if the codes are not in preferred order.
	** We have seen this error at prior Plugfests.
	**/
	pObj->pContainer[param]->preferredVideoFormat = hdmiStatus.hdmiDevice ? supportedVideoInfo.supportedVideoIDCode[0] : 0;
#else
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
#endif
	return;
}

void CSnmp::snmp_get_dvi_hdmi_available_video_format_table(void *context, int param)
{
#if NEXUS_NUM_HDMI_OUTPUTS
	mibObjectDviHdmiAvailableVideoFormatTable *pObj = (mibObjectDviHdmiAvailableVideoFormatTable *) context;
	NEXUS_PlatformConfiguration configuration;
	NEXUS_HdmiOutputStatus hdmiStatus;
	NEXUS_HdmiOutputSupportedVideoInfo supportedVideoInfo;
	NEXUS_HdmiAviInfoFrame aviInfoFrame;
	short supported3DStructures = 0;
	int active3DStructure = oc_active3DStructures_na;
	int i;
	NEXUS_Error retVal = BERR_SUCCESS;

	BKNI_Memset(&hdmiStatus, 0, sizeof(NEXUS_HdmiOutputStatus));
	BKNI_Memset(&supportedVideoInfo, 0, sizeof(NEXUS_HdmiOutputSupportedVideoInfo));
	BKNI_Memset(&aviInfoFrame, 0, sizeof(NEXUS_HdmiAviInfoFrame));
    BKNI_Memset(pObj->pContainer[param], 0, sizeof(containerDviHdmiAvailableVideoFormatTable));

	NEXUS_Platform_GetConfiguration(&configuration);
	NEXUS_HdmiOutput_GetStatus(configuration.outputs.hdmi[0], &hdmiStatus);
	if (hdmiStatus.hdmiDevice)
	{
		NEXUS_HdmiOutput_GetSupportedVideoInfo(configuration.outputs.hdmi[0], &supportedVideoInfo);
		retVal = NEXUS_HdmiOutput_GetAviInfoFrame(configuration.outputs.hdmi[0], &aviInfoFrame);

		for (i=0; i < NEXUS_VideoFormat_eMax; i++)
		{
			switch (hdmiStatus.hdmi3DFormatsSupported[i])
			{
				case NEXUS_HdmiOutput_3DStructure_FramePacking:
					supported3DStructures |= oc_supported3DStructures_framePacking;
					break;
				case NEXUS_HdmiOutput_3DStructure_FieldAlternative:
					supported3DStructures |= oc_supported3DStructures_fieldAlternative;
					break;
				case NEXUS_HdmiOutput_3DStructure_LineAlternative:
					supported3DStructures |= oc_supported3DStructures_lineAlternative;
					break;
				case NEXUS_HdmiOutput_3DStructure_SideBySideFull:
					supported3DStructures |= oc_supported3DStructures_sideBySideFull;
					break;
				case NEXUS_HdmiOutput_3DStructure_LDepth:
					supported3DStructures |= oc_supported3DStructures_leftPlusDepth;
					break;
				case NEXUS_HdmiOutput_3DStructure_LDepthGraphics:
					supported3DStructures |= oc_supported3DStructures_leftPlusDepthPlusGraphicsPlusGraphicsDepth;
					break;
				case NEXUS_HdmiOutput_3DStructure_TopAndBottom:
					supported3DStructures |= oc_supported3DStructures_topAndBottom;
					break;
				case NEXUS_HdmiOutput_3DStructure_SideBySideHalfHorizontal:
					supported3DStructures |= oc_supported3DStructures_sideBySideHalf;
					break;
				case NEXUS_HdmiOutput_3DStructure_SideBySideHalfQuincunx:
					supported3DStructures |= oc_supported3DStructures_sideBySideHalfQuincunx;
					break;
				case NEXUS_HdmiOutput_3DStructure_Reserved07:
				case NEXUS_HdmiOutput_3DStructure_Reserved09:
				case NEXUS_HdmiOutput_3DStructure_Reserved10:
				case NEXUS_HdmiOutput_3DStructure_Reserved11:
				case NEXUS_HdmiOutput_3DStructure_Reserved12:
				case NEXUS_HdmiOutput_3DStructure_Reserved13:
				case NEXUS_HdmiOutput_3DStructure_Reserved14:
					BDBG_WRN(("Huh? Why did we get a reserved 3D structure from Nexus?"));
				default:
					break;

			}
		}

		if (supported3DStructures)
		{
			if (param == aviInfoFrame.videoIdCode)
			{
				switch (hdmiStatus.hdmi3DFormatsSupported[hdmiStatus.videoFormat])
				{
					case NEXUS_HdmiOutput_3DStructure_FramePacking:
						active3DStructure = oc_active3DStructures_framePacking;
						break;
					case NEXUS_HdmiOutput_3DStructure_FieldAlternative:
						active3DStructure = oc_active3DStructures_fieldAlternative;
						break;
					case NEXUS_HdmiOutput_3DStructure_LineAlternative:
						active3DStructure = oc_active3DStructures_lineAlternative;
						break;
					case NEXUS_HdmiOutput_3DStructure_SideBySideFull:
						active3DStructure = oc_active3DStructures_sideBySideFull;
						break;
					case NEXUS_HdmiOutput_3DStructure_LDepth:
						active3DStructure = oc_active3DStructures_leftPlusDepth;
						break;
					case NEXUS_HdmiOutput_3DStructure_LDepthGraphics:
						active3DStructure = oc_active3DStructures_leftPlusDepthPlusGraphicsPlusGraphicsDepth;
						break;
					case NEXUS_HdmiOutput_3DStructure_TopAndBottom:
						active3DStructure = oc_active3DStructures_topAndBottom;
						break;
					case NEXUS_HdmiOutput_3DStructure_SideBySideHalfHorizontal:
						active3DStructure = oc_active3DStructures_sideBySideHalf;
						break;
					case NEXUS_HdmiOutput_3DStructure_SideBySideHalfQuincunx:
						active3DStructure = oc_active3DStructures_sideBySideHalfQuincunx;
						break;
					case NEXUS_HdmiOutput_3DStructure_Reserved07:
					case NEXUS_HdmiOutput_3DStructure_Reserved09:
					case NEXUS_HdmiOutput_3DStructure_Reserved10:
					case NEXUS_HdmiOutput_3DStructure_Reserved11:
					case NEXUS_HdmiOutput_3DStructure_Reserved12:
					case NEXUS_HdmiOutput_3DStructure_Reserved13:
					case NEXUS_HdmiOutput_3DStructure_Reserved14:
						BDBG_WRN(("Huh? Why did we get a reserved 3D structure from Nexus?"));
					default:
						active3DStructure = oc_active3DStructures_other;
						break;
				}
			}
		}
		else
		{
			if (retVal != BERR_SUCCESS) /* no InfoFrame is obtained */
			{
				active3DStructure = oc_active3DStructures_infoFrameNotAvailable;
			}
			else
			{
				switch (aviInfoFrame.videoIdCode)
				{
					case 0:
						active3DStructure = oc_active3DStructures_noAdditionalHDMIInfo;
						break;
					case 1:
						active3DStructure = oc_active3DStructures_no3DInforamtion;
						break;
					default:
						break;
				}
			}
		}

		pObj->pContainer[param]->numSupportedVideoDescriptors = supportedVideoInfo.numSupportedVideoDescriptors;
		pObj->pContainer[param]->availableVideoFormat = (param < supportedVideoInfo.numSupportedVideoDescriptors) ? supportedVideoInfo.supportedVideoIDCode[param] : 0;
		pObj->pContainer[param]->supported3DStructures = supported3DStructures;
		pObj->pContainer[param]->active3DStructure = active3DStructure;
	}
	else
	{
		pObj->pContainer[param]->numSupportedVideoDescriptors = 0;
		pObj->pContainer[param]->availableVideoFormat = 0;
		pObj->pContainer[param]->supported3DStructures = 0;
		pObj->pContainer[param]->active3DStructure = oc_active3DStructures_na;
	}
#else
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
#endif
	return;
}

void CSnmp::snmp_get_component_video_table(void *context, int param)
{
#if NEXUS_NUM_COMPONENT_OUTPUTS
	mibObjectComponentVideoTable *pObj = (mibObjectComponentVideoTable *) context;
	NEXUS_PlatformConfiguration configuration;
	NEXUS_ComponentOutputSettings componentOutputSettings;
	NEXUS_DisplaySettings displaySettings;

    CSnmp * pSnmp = g_pSnmp;
    CDisplay * pDisplay = NULL;

    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    BKNI_Memset(pObj->pContainer[param], 0, sizeof(containerComponentVideoTable));

    if ((pDisplay = pSnmp->_pModel->getDisplay(0)) != NULL)
    {
        pObj->display[0] = pDisplay->getDisplay();
    }
    else
    {
        pObj->display[0] = NULL;
    }

    BKNI_Memset(&componentOutputSettings, 0, sizeof(NEXUS_ComponentOutputSettings));
    BKNI_Memset(&displaySettings, 0, sizeof(NEXUS_DisplaySettings));

	NEXUS_Platform_GetConfiguration(&configuration);
	NEXUS_Display_GetSettings(pObj->display[param], &displaySettings);
	NEXUS_ComponentOutput_GetSettings(configuration.outputs.component[param], &componentOutputSettings);

	pObj->pContainer[param]->constrainedStatus = componentOutputSettings.mpaaDecimationEnabled;
	pObj->pContainer[param]->outputFormat = displaySettings.format;
	pObj->pContainer[param]->aspectRatio = displaySettings.aspectRatio;
	pObj->pContainer[param]->videoMuteStatus = !pSnmp->snmp_check_display_connection(eBoardResource_outputComponent);
#else
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
#endif
	return;
}

void CSnmp::snmp_get_rf_channel_out_table(void *context, int param)
{
#if NEXUS_NUM_RFM_OUTPUTS
	mibObjectRfChannelOutTable *pObj = (mibObjectRfChannelOutTable *) context;
	NEXUS_PlatformConfiguration configuration;
	NEXUS_RfmSettings rfmSettings;

    CSnmp * pSnmp = g_pSnmp;

    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    BKNI_Memset(pObj->pContainer[param], 0, sizeof(containerRfChannelOutTable));

	NEXUS_Platform_GetConfiguration(&configuration);
	NEXUS_Rfm_GetSettings(configuration.outputs.rfm[param], &rfmSettings);

	pObj->pContainer[param]->channelOut = rfmSettings.channel;
	pObj->pContainer[param]->audioMuteStatus = rfmSettings.muted;
	pObj->pContainer[param]->videoMuteStatus = !pSnmp->snmp_check_display_connection(eBoardResource_outputRFM);
#else
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
#endif
	return;
}

void CSnmp::snmp_get_in_band_tuner_table(void *context, int param)
{
	mibObjectInBandTunerTable *pObj = (mibObjectInBandTunerTable *) context;
	NEXUS_FrontendCapabilities capabilities;
	NEXUS_TimebaseStatus timebaseStatus;
	NEXUS_VideoDecoderStatus videoDecoderStatus;
	NEXUS_AudioDecoderStatus audioDecoderStatus;

    CSnmp * pSnmp = g_pSnmp;
    eWindowType screenType;
    CSimpleAudioDecode * pSimpleAudioDecode = NULL;
    CSimpleVideoDecode * pSimpleVideoDecode = NULL;
    CVideoDecode * pVideoDecode = NULL;
    CChannel * pChannel = NULL;
    CTuner * pTuner = NULL;
    tuner_status tunerStatus;

    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

	BKNI_Memset(&capabilities, 0, sizeof(NEXUS_FrontendCapabilities));
	BKNI_Memset(&timebaseStatus, 0, sizeof(NEXUS_TimebaseStatus));
	BKNI_Memset(&videoDecoderStatus, 0, sizeof(NEXUS_VideoDecoderStatus));
	BKNI_Memset(&audioDecoderStatus, 0, sizeof(NEXUS_AudioDecoderStatus));
    BKNI_Memset(&tunerStatus, 0, sizeof(tuner_status));
    BKNI_Memset(pObj->pContainer[param], 0, sizeof(containerInBandTunerTable));

    if (param < MAX_DECODER)
    {
        if (param == 0)
        {
            screenType = pSnmp->_pModel->getFullScreenWindowType();
        }
        else if (param == 1)
        {
            screenType = pSnmp->_pModel->getPipScreenWindowType();
        }

        if ((pSimpleAudioDecode = pSnmp->_pModel->getSimpleAudioDecode(screenType)) != NULL)
        {
            pObj->audioDecoder[param] = pSimpleAudioDecode->getDecoder();
        }
        else
        {
            pObj->audioDecoder[param] = NULL;
        }

        if ((pSimpleVideoDecode = pSnmp->_pModel->getSimpleVideoDecode(screenType)) != NULL)
        {
            if ((pVideoDecode = pSimpleVideoDecode->getVideoDecoder()) != NULL)
            {
                pObj->videoDecoder[param] = pVideoDecode->getDecoder();
                pObj->timebase[param] = (NEXUS_Timebase) pVideoDecode->getNumber();
            }
            else
            {
                pObj->videoDecoder[param] = NULL;
                pObj->timebase[param] = NEXUS_Timebase_eInvalid;
            }
        }
        else
        {
            pObj->videoDecoder[param] = NULL;
            pObj->timebase[param] = NEXUS_Timebase_eInvalid;
        }

        if ((pChannel = pSnmp->_pModel->getCurrentChannel(screenType)) != NULL)
        {
            if ((pTuner = pChannel->getTuner()) != NULL)
            {
                MStringHash * pProperties;
                pProperties = new MStringHash;
                pTuner->getProperties(pProperties);
                delete pProperties;
                pTuner->getStatus(&tunerStatus);

                pObj->tuner[param] = pTuner->getFrontend();
            }
            else
            {
                pObj->tuner[param] = NULL;
            }
        }
        else
        {
            pObj->tuner[param] = NULL;
        }
    }

	if (!pObj->tuner[param])
	{
        pObj->pContainer[param]->modulationMode = NEXUS_FrontendQamMode_eMax;
        pObj->pContainer[param]->frequency = 0;
        pObj->pContainer[param]->interleaver = -1;
        pObj->pContainer[param]->tunerPower = 0;
        pObj->pContainer[param]->agcValue = 0;
        pObj->pContainer[param]->snrValue = 0;
        pObj->pContainer[param]->unerroreds = 0;
        pObj->pContainer[param]->correcteds = 0;
        pObj->pContainer[param]->uncorrectables = 0;
        pObj->pContainer[param]->carrierLockLost = 0;
        pObj->pContainer[param]->pcrErrors = 0;
        pObj->pContainer[param]->ptsErrors = 0;
        pObj->pContainer[param]->state = tuner_idle;
		pObj->pContainer[param]->ber = 1;
        pObj->pContainer[param]->secSinceLock = 0;
        pObj->pContainer[param]->eqGain = 0;
        pObj->pContainer[param]->mainTap = 0;
        pObj->pContainer[param]->totalTuneCount = 0;
        pObj->pContainer[param]->tuneFailureCount = 0;
        pObj->pContainer[param]->tuneFailFreq= 0;
        pObj->pContainer[param]->bandwidth = 0;

		return;
	}

    NEXUS_Timebase_GetStatus(pObj->timebase[param], &timebaseStatus);

    if (pObj->tuner[param])
	{
		NEXUS_Frontend_GetCapabilities(pObj->tuner[param], &capabilities);
	}
	if (pObj->videoDecoder[param])
	{
		NEXUS_VideoDecoder_GetStatus(pObj->videoDecoder[param], &videoDecoderStatus);
	}
	if (pObj->audioDecoder[param])
	{
		NEXUS_AudioDecoder_GetStatus(pObj->audioDecoder[param], &audioDecoderStatus);
	}

	pObj->qam = capabilities.qam;
	pObj->analog = capabilities.analog;

	if (pObj->qam)
	{
		if(pObj->tuner[param])
		{
			pObj->annex = tunerStatus.status.qam.settings.annex;
			pObj->pContainer[param]->modulationMode = tunerStatus.status.qam.settings.mode;
			pObj->pContainer[param]->frequency = tunerStatus.status.qam.settings.frequency;
			pObj->pContainer[param]->interleaver = tunerStatus.status.qam.interleaveDepth;
			pObj->pContainer[param]->tunerPower = tunerStatus.status.qam.dsChannelPower;
			pObj->pContainer[param]->agcValue = tunerStatus.status.qam.ifAgcLevel;
			pObj->pContainer[param]->snrValue = tunerStatus.status.qam.snrEstimate;
			pObj->pContainer[param]->unerroreds = tunerStatus.unerrored;
			pObj->pContainer[param]->correcteds = tunerStatus.status.qam.fecCorrected;
			pObj->pContainer[param]->uncorrectables = tunerStatus.status.qam.fecUncorrected;
			pObj->pContainer[param]->carrierLockLost = tunerStatus.unlock_count;
			pObj->pContainer[param]->pcrErrors = timebaseStatus.pcrErrors;
			pObj->pContainer[param]->ptsErrors = videoDecoderStatus.ptsErrorCount + audioDecoderStatus.ptsErrorCount;
			pObj->pContainer[param]->state = (tunerState)tunerStatus.state;
			if (tunerStatus.status.qam.fecCorrected + tunerStatus.status.qam.fecUncorrected == 0 || tunerStatus.unerrored + tunerStatus.status.qam.fecCorrected + tunerStatus.status.qam.fecUncorrected == 0)
			{
				pObj->pContainer[param]->ber = 0;
			}
			else
			{
				double fBER = log(((double)tunerStatus.status.qam.fecCorrected + (double)tunerStatus.status.qam.fecUncorrected) / ((double)tunerStatus.unerrored + (double)tunerStatus.corrected + (double)tunerStatus.uncorrected)) / log(10.0);
				pObj->pContainer[param]->ber = (int) (fBER > 0) ? (fBER + 0.5) : (fBER - 0.5);
			}
            pObj->pContainer[param]->secSinceLock = tunerStatus.elapsed_time;
            pObj->pContainer[param]->eqGain = tunerStatus.status.qam.equalizerGain;
            pObj->pContainer[param]->mainTap = tunerStatus.status.qam.mainTap;
            pObj->pContainer[param]->totalTuneCount = tunerStatus.acquire_count;
            pObj->pContainer[param]->tuneFailureCount = tunerStatus.failure_count;
            pObj->pContainer[param]->tuneFailFreq= tunerStatus.failure_freq;
            pObj->pContainer[param]->bandwidth = oc_tunerBandwidth_mHz1002;
		}
	}

	return;
}

void CSnmp::snmp_get_analog_video_table(void *context, int param)
{
	mibObjectAnalogVideoTable *pObj = (mibObjectAnalogVideoTable *) context;
	NEXUS_DisplayVbiSettings displayVbiSettings;

    CSnmp * pSnmp = g_pSnmp;
    CDisplay * pDisplay = NULL;

    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    BKNI_Memset(&displayVbiSettings, 0, sizeof(NEXUS_DisplayVbiSettings));

    if ((pDisplay = pSnmp->_pModel->getDisplay(0)) != NULL)
    {
        pObj->display[0] = pDisplay->getDisplay();
    }
    else
    {
        pObj->display[0] = NULL;
    }

    if ((pDisplay = pSnmp->_pModel->getDisplay(1)) != NULL)
    {
        pObj->display[1] = pDisplay->getDisplay();
    }
    else
    {
        pObj->display[1] = NULL;
    }

	/*
	 * Component and Display connection is platform dependent and display mode dependent.
	 * In 1080p, display1 drives component and composite and in other display modes,
	 * display drives the component and HDMI both.
	 * Currently, assuming that display drives the component.
	 */
#if NEXUS_NUM_COMPONENT_OUTPUTS
	if (pObj->pContainer[param]->avIfType == ocStbHostComponentOut)
	{
		if (pObj->display[0])
		{
			NEXUS_Display_GetVbiSettings(pObj->display[0], &displayVbiSettings);
		}
		else
		{
			displayVbiSettings.macrovisionEnabled = false;
		}
	}
#endif
	/*
	 * Composite and Display connection is platform dependent.
	 * Currently, assuming that display1 drives the composite.
	 */
#if NEXUS_NUM_COMPOSITE_OUTPUTS
	if (pObj->pContainer[param]->avIfType == ocStbHostBbVideoOut)
	{
		if (pObj->display[1])
		{
			NEXUS_Display_GetVbiSettings(pObj->display[1], &displayVbiSettings);
		}
		else
		{
			displayVbiSettings.macrovisionEnabled = false;
		}
	}
#endif
	/*
	 * SVIDEO and Display connection is platform dependent.
	 * Currently, assuming that display1 drives the SVIDEO.
	 */
#if NEXUS_NUM_SVIDEO_OUTPUTS
	if (pObj->pContainer[param]->avIfType == ocStbHostSVideoOut)
	{
		if (pObj->display[2])
		{
			NEXUS_Display_GetVbiSettings(pObj->display[2], &displayVbiSettings);
		}
		else
		{
			displayVbiSettings.macrovisionEnabled = false;
		}
	}
#endif
	pObj->pContainer[param]->potectionStatus = displayVbiSettings.macrovisionEnabled;

	return;

}

void CSnmp::snmp_get_mpeg2_content_table(void *context, int param)
{
    mibObjectMpeg2ContentTable *pObj = (mibObjectMpeg2ContentTable *) context;
    NEXUS_VideoDecoderStatus VideoDecStatus;
    NEXUS_TimebaseStatus timebaseStatus;
    uint8_t cci = 0;

    eWindowType screenType;
    CSimpleVideoDecode * pSimpleVideoDecode = NULL;
    CVideoDecode * pVideoDecode = NULL;
    CChannel * pChannel = NULL;
    CHANNEL_INFO_T channelInfo;
    PROGRAM_INFO_T * pProgramInfo = NULL;
    uint32_t ccErrorCount = 0;
    uint32_t teiErrorCount = 0;
    unsigned program_number = 0;
    bool isMain = false;
    bool isPip = false;

    CSnmp * pSnmp = g_pSnmp;

    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    BKNI_Memset(&VideoDecStatus, 0, sizeof(NEXUS_VideoDecoderStatus));
    BKNI_Memset(&timebaseStatus, 0, sizeof(NEXUS_TimebaseStatus));
    BKNI_Memset(&channelInfo, 0, sizeof(CHANNEL_INFO_T));
    BKNI_Memset(pObj->pContainer[param], 0, sizeof(containerMpeg2ContentTable));

    if (param < MAX_DECODER)
    {
        if (param == 0)
        {
            isMain = true;
            screenType = pSnmp->_pModel->getFullScreenWindowType();
        }
        else if (param == 1)
        {
            isPip = pSnmp->_pModel->getPipEnabled() && pSnmp->_pModel->getPipState();
            screenType = pSnmp->_pModel->getPipScreenWindowType();
        }

        if ((pSimpleVideoDecode = pSnmp->_pModel->getSimpleVideoDecode(screenType)) != NULL)
        {
            if ((pVideoDecode = pSimpleVideoDecode->getVideoDecoder()) != NULL)
            {
                pObj->video_decode[param] = pVideoDecode->getDecoder();
                pObj->timebase[param] = (NEXUS_Timebase) pVideoDecode->getNumber();
            }
            else
            {
                pObj->video_decode[param] = NULL;
                pObj->timebase[param] = NEXUS_Timebase_eInvalid;
            }
        }
        else
        {
            pObj->video_decode[param] = NULL;
            pObj->timebase[param] = NEXUS_Timebase_eInvalid;
        }

        if ((pChannel = pSnmp->_pModel->getCurrentChannel(screenType)) != NULL)
        {
            pChannel->getChannelInfo(&channelInfo, true);
            program_number = pChannel->getMinor() - 1;
            pProgramInfo = &channelInfo.program_info[program_number];
            ccErrorCount = pChannel->getCcError();
            teiErrorCount = pChannel->getTeiError();
            cci = pChannel->getCci();
        }
    }

    if ((isMain && param == 0) || (isPip && param == 1))
    {
        if (pObj->timebase[param] != NEXUS_Timebase_eInvalid)
        {
            NEXUS_Timebase_GetStatus(pObj->timebase[param], &timebaseStatus);
        }

        if (pObj->video_decode[param])
        {
            NEXUS_VideoDecoder_GetStatus(pObj->video_decode[param], &VideoDecStatus);
        }

        pObj->pContainer[param]->programNumber = program_number;
        pObj->pContainer[param]->transportStreamId = channelInfo.transport_stream_id;
        pObj->pContainer[param]->totalStreams = (pProgramInfo) ? (pProgramInfo->num_audio_pids + pProgramInfo->num_video_pids + pProgramInfo->num_other_pids) : 0;
        pObj->pContainer[param]->selectedVideoPid  = (pProgramInfo) ? pProgramInfo->video_pids[0].pid : -1;
        pObj->pContainer[param]->selectedAudioPid = (pProgramInfo) ? pProgramInfo->audio_pids[0].pid : -1;
        pObj->pContainer[param]->otherAudioPids = (pProgramInfo) ? ((pProgramInfo->num_audio_pids > 1) ? true : false) : false;
        pObj->pContainer[param]->cciValue = (Mpeg2ContentTable_cciValue)(cci & 0x3);
        pObj->pContainer[param]->apsValue = (cci & 0x6) >> 2;
        pObj->pContainer[param]->citStatus = ((cci & 0x10) >> 4) ? true : false;
        pObj->pContainer[param]->broadcastFlagStatus = (pProgramInfo) ? (pProgramInfo->broadcast_flag ? true : false) : false;
        pObj->pContainer[param]->epnStatus = ((cci & 0x20) >> 5) ? true : false;
        pObj->pContainer[param]->pcrPid = (pProgramInfo) ? pProgramInfo->pcr_pid : -1;
        pObj->pContainer[param]->pcrLockStatus = timebaseStatus.pcrValid;
        pObj->pContainer[param]->decoderPts = VideoDecStatus.pts;
        pObj->pContainer[param]->discontinuities = ccErrorCount;
        pObj->pContainer[param]->pktErrors = teiErrorCount;
        pObj->pContainer[param]->pipelineErrors = VideoDecStatus.numDecodeErrors;
        pObj->pContainer[param]->decoderRestarts = VideoDecStatus.numWatchdogs;
    }
    else
    {
        pObj->pContainer[param]->programNumber = 0;
        pObj->pContainer[param]->transportStreamId = 0;
        pObj->pContainer[param]->totalStreams = 0;
        pObj->pContainer[param]->selectedVideoPid  = -1;
        pObj->pContainer[param]->selectedAudioPid = -1;
        pObj->pContainer[param]->otherAudioPids = false;
        pObj->pContainer[param]->cciValue = -1;
        pObj->pContainer[param]->apsValue = -1;
        pObj->pContainer[param]->citStatus = false;
        pObj->pContainer[param]->broadcastFlagStatus = false;
        pObj->pContainer[param]->epnStatus = false;
        pObj->pContainer[param]->pcrPid = -1;
        pObj->pContainer[param]->pcrLockStatus = 0;
        pObj->pContainer[param]->decoderPts = 0;
        pObj->pContainer[param]->discontinuities = 0;
        pObj->pContainer[param]->pktErrors = 0;
        pObj->pContainer[param]->pipelineErrors = 0;
        pObj->pContainer[param]->decoderRestarts = 0;
    }

    return;

}

/*Get the active decode channel in the case of dual decode and no pip */
void CSnmp::snmp_get_program_status_table(void *context, int param)
{
    mibObjectProgramStatusTable *pObj = (mibObjectProgramStatusTable *) context;

    CSnmp * pSnmp = g_pSnmp;
    eWindowType screenType;
    CSimpleVideoDecode * pSimpleVideoDecode = NULL;
    CVideoDecode * pVideoDecode = NULL;

    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    if (param < MAX_DECODER)
    {
        if (param == 0)
        {
            screenType = pSnmp->_pModel->getFullScreenWindowType();
        }
        else if (param == 1)
        {
            if (!pSnmp->_pModel->getPipEnabled() || !pSnmp->_pModel->getPipState())
            {
                return;
            }
            screenType = pSnmp->_pModel->getPipScreenWindowType();
        }

        pSimpleVideoDecode = pSnmp->_pModel->getSimpleVideoDecode(screenType);
        pVideoDecode = pSimpleVideoDecode ? pSimpleVideoDecode->getVideoDecoder() : NULL;
    }

	if (pVideoDecode)
	{
        if (pVideoDecode->getDecoder())
        {
            pObj->decode_index = param + 1;
        }
	}

    return;
}

void CSnmp::snmp_get_qpsk_objects(void *context, int param)
{
	mibObjectQpskObjects *pObj = (mibObjectQpskObjects *) context;
	NEXUS_FrontendCapabilities capabilities;
    NEXUS_FrontendUpstreamStatus upStreamStatus;
    tuner_status tunerStatus;

    CTunerOob * pTunerOob = g_pTunerOob;
    CTunerUpstream * pTunerUpstream = g_pTunerUpstream;

	BSTD_UNUSED(param);

    BKNI_Memset(&capabilities, 0, sizeof(NEXUS_FrontendCapabilities));
    BKNI_Memset(&upStreamStatus, 0, sizeof(NEXUS_FrontendUpstreamStatus));
    BKNI_Memset(&tunerStatus, 0, sizeof(tuner_status));
    BKNI_Memset(&pObj->container, 0, sizeof(containerQpskObjects));

    if (pTunerOob)
    {
        MStringHash * pProperties;
        pProperties = new MStringHash;
        pTunerOob->getProperties(pProperties);
        delete pProperties;
        pTunerOob->getStatus(&tunerStatus);

        pObj->oobTuner = pTunerOob->getFrontend();
    }
    else
    {
        pObj->oobTuner = NULL;
    }

    if (pTunerUpstream)
    {
        pObj->upstreamTuner = pTunerUpstream->getFrontend();
    }
    else
    {
        pObj->upstreamTuner = NULL;
    }

    if (pObj->oobTuner)
    {
	    NEXUS_Frontend_GetCapabilities(pObj->oobTuner, &capabilities);
    }
    if (pObj->upstreamTuner)
    {
        NEXUS_Frontend_GetUpstreamStatus(pObj->upstreamTuner, &upStreamStatus);
    }

	if (capabilities.outOfBand && capabilities.upstream)
	{
		pObj->container.fdcFreq = tunerStatus.status.oob.settings.frequency;
		pObj->container.rdcFreq = upStreamStatus.frequency;
		pObj->container.fdcBer = (pObj->container.fdcFreq != 0) ? tunerStatus.status.oob.postRsBer : 1;
		pObj->container.state = (tunerState) tunerStatus.state;
		pObj->container.fdcBytesRead = tunerStatus.total_byte;
		pObj->container.fdcPower = tunerStatus.status.oob.fdcChannelPower;
		pObj->container.lockedTime = tunerStatus.elapsed_time;
		pObj->container.fdcSNR = tunerStatus.status.oob.snrEstimate;
		pObj->container.agc = tunerStatus.status.oob.agcExtLevel;
		pObj->container.rdcPower = upStreamStatus.powerLevel / 10;
		pObj->container.rdcDataRate = upStreamStatus.symbolRate;
	}
    else
    {
		pObj->container.fdcFreq = 0;
		pObj->container.rdcFreq = 0;
		pObj->container.fdcBer = 1;
		pObj->container.state = tuner_idle;
		pObj->container.fdcBytesRead = 0;
		pObj->container.fdcPower = 0;
		pObj->container.lockedTime = 0;
		pObj->container.fdcSNR = 0;
		pObj->container.agc = 0;
		pObj->container.rdcPower = 0;
		pObj->container.rdcDataRate = 0;
    }

	return;
}

void CSnmp::snmp_get_spdif_table(void *context, int param)
{
#if NEXUS_NUM_SPDIF_OUTPUTS
    mibObjectSPDIfTable *pObj = (mibObjectSPDIfTable*)context;
    NEXUS_AudioOutputSettings settings;

    CSnmp * pSnmp = g_pSnmp;
    CSimpleAudioDecode * pSimpleAudioDecode = NULL;
    COutputSpdif * pOutputSpdif = NULL;
    NEXUS_AudioDecoderStatus status;

    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    BKNI_Memset(&status, 0, sizeof(NEXUS_AudioDecoderStatus));
    BKNI_Memset(pObj->pContainer[param], 0, sizeof(containerSPDIfTable));

    if ((pSimpleAudioDecode = pSnmp->_pModel->getSimpleAudioDecode(pSnmp->_pModel->getFullScreenWindowType())) != NULL)
    {
        if ((pOutputSpdif = pSimpleAudioDecode->getOutputSpdif()) != NULL)
        {
            pObj->audio_output[0] = pOutputSpdif->getConnectorA();
        }
        else
        {
            pObj->audio_output[0] = NULL;
        }

        pSimpleAudioDecode->getStatus(&status);
    }
    else
    {
        pObj->audio_output[0] = NULL;
    }

    if (pObj->audio_output[param])
    {
		NEXUS_AudioOutput_GetSettings(pObj->audio_output[param], &settings);
    }

    pObj->pContainer[param]->muteStatus = settings.muted;
    pObj->pContainer[param]->audioFormat = status.codec;
#else
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
#endif
    return;
}

void CSnmp::snmp_get_eas_codes(void *context, int param)
{
	mibObjectEasCodes *pObj = (mibObjectEasCodes*)context;

	BSTD_UNUSED(param);

	pObj->container.countyCode = 1;
	pObj->container.stateCode = 2;
	pObj->container.subDivisionCode = 3;

	return;
}

void CSnmp::snmp_get_device_software_base(void *context, int param)
{
	mibObjectDeviceSoftwareBase *pObj = (mibObjectDeviceSoftwareBase*)context;
	char releaseDate[8] = {(char)0x07, (char)0xD9, (char)0x0A, (char)0x0F, 0, 0, 0, 0}; /* 2009-10-15,00:00:00.0 */

	BSTD_UNUSED(param);

	sprintf(pObj->container.firmwareVersion, "Firmware-1.1");
	sprintf(pObj->container.OCAPVersion, "OCAP1.1");
	BKNI_Memcpy(pObj->container.releaseDate, releaseDate, 8);

	return;
}

void CSnmp::snmp_get_fw_download_status(void *context, int param)
{
	mibObjectFwDownloadStatus *pObj = (mibObjectFwDownloadStatus *) context;

	BSTD_UNUSED(param);

#ifdef CDL_SUPPORT
	int downloadFailedCount = 0;
	int groupId = 0;
	int ret = 0;
    B_Cdl_Status status;
	if (B_Cdl_Get_Status(&status) < 0) {
        pObj->container.imageStatus = oc_imageStatus_imageAuthorized;
        pObj->container.codeDownloadStatus = oc_codeDownloadStatus_dowloadingComplete;
        sprintf(pObj->container.name, "%s", "firmware_code");
        pObj->container.downloadFailedStatus = oc_downloadFailedStatus_cdlError1;
		pObj->container.downloadFailedCount = 0;
		pObj->container.downloadGroupId = 0;
        return;
    }

	pObj->container.imageStatus = status.imageStatus;
	pObj->container.codeDownloadStatus = status.codeDownloadStatus;
	sprintf(pObj->container.name, "%s", status.name);
	pObj->container.downloadFailedStatus = status.downloadFailedStatus;
#ifdef ESTB_CFG_SUPPORT
	ret = B_Estb_cfg_Get_uint32((char *)"/dyn/estb/download_count", (unsigned int*)&downloadFailedCount);
#endif
	pObj->container.downloadFailedCount = ret ? 0 : downloadFailedCount;
#ifdef ESTB_CFG_SUPPORT
	ret = B_Estb_cfg_Get_uint32((char *)"/dyn/estb/group_id", (unsigned int*)&groupId);
#endif
	pObj->container.downloadGroupId = groupId;
#else
	/* cdl_get_status() */
	pObj->container.imageStatus = oc_imageStatus_imageAuthorized;
	pObj->container.codeDownloadStatus = oc_codeDownloadStatus_dowloadingComplete;
	sprintf(pObj->container.name, "%s", "firmware_code");
	pObj->container.downloadFailedStatus = oc_downloadFailedStatus_cdlError1;
	pObj->container.downloadFailedCount = 0;
	pObj->container.downloadGroupId = 0;
#endif

	return;
}

void CSnmp::snmp_get_sw_app_info(void *context, int param)
{
	mibObjectSwAppInfo *pObj = (mibObjectSwAppInfo *) context;
	char lastReceivedTime[8] = {(char)0x07, (char)0xD9, (char)0x0A, (char)0x0F, 0, 0, 0, 0}; /* 2009-10-15,00:00:00.0 */

	BSTD_UNUSED(param);

	BKNI_Memcpy(pObj->container.sigLastReceivedTime, lastReceivedTime, 8);
	pObj->container.sigLastReadStatus = oc_sigLastReadStatus_okay;
	pObj->container.sigLastNetworkVersionRead = -1;
	pObj->container.sigVersionInUse = -1;

	return;
}

void CSnmp::snmp_get_sw_app_info_table(void *context, int param)
{
	mibObjectSwAppInfoTable *pObj = (mibObjectSwAppInfoTable *) context;

	switch (param)
	{
		case 0:
			sprintf(pObj->pContainer[0]->name, "%s", "rnonvolhost");
			sprintf(pObj->pContainer[0]->version, "%s", "1.18");

			pObj->pContainer[0]->orgId[0] = 0x80;
			pObj->pContainer[0]->orgId[1] = 0xC1;
			pObj->pContainer[0]->orgId[2] = 0xD2;
			pObj->pContainer[0]->orgId[3] = 0x73;

			pObj->pContainer[0]->appId[0] = 0xAB;
			pObj->pContainer[0]->appId[1] = 0x01;
			break;
		case 1:
			sprintf(pObj->pContainer[1]->name, "%s", "hostboot");
			sprintf(pObj->pContainer[1]->version, "%s", "1.18");

			pObj->pContainer[1]->orgId[0] = 0x14;
			pObj->pContainer[1]->orgId[1] = 0xA5;
			pObj->pContainer[1]->orgId[2] = 0x56;
			pObj->pContainer[1]->orgId[3] = 0xB7;

			pObj->pContainer[1]->appId[0] = 0xBC;
			pObj->pContainer[1]->appId[1] = 0x03;
			break;
		case 2:
			sprintf(pObj->pContainer[2]->name, "%s", "atlas");
			sprintf(pObj->pContainer[2]->version, "%s", "1.0");

			pObj->pContainer[2]->orgId[0] = 0xE8;
			pObj->pContainer[2]->orgId[1] = 0x29;
			pObj->pContainer[2]->orgId[2] = 0x9A;
			pObj->pContainer[2]->orgId[3] = 0xEB;

			pObj->pContainer[2]->appId[0] = 0xCD;
			pObj->pContainer[2]->appId[1] = 0x05;
			break;
		case 3:
			sprintf(pObj->pContainer[3]->name, "%s", "estbsnmpagent");
			sprintf(pObj->pContainer[3]->version, "%s", "2.0.4");

			pObj->pContainer[3]->orgId[0] = 0xFC;
			pObj->pContainer[3]->orgId[1] = 0x6D;
			pObj->pContainer[3]->orgId[2] = 0xAE;
			pObj->pContainer[3]->orgId[3] = 0x9F;

			pObj->pContainer[3]->appId[0] = 0xDE;
			pObj->pContainer[3]->appId[1] = 0x07;
			break;
	}

	return;
}

void CSnmp::snmp_get_security_subsystem(void *context, int param)
{
	mibObjectSecuritySubSystem *pObj = (mibObjectSecuritySubSystem *) context;
#ifdef MPOD_SUPPORT
    CSnmp * pSnmp = g_pSnmp;
    CableCARDInfo *info = NULL;
    CCablecard * pCablecard = NULL;
    cablecard_t cablecard = NULL;
#endif

	BSTD_UNUSED(param);

    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    sprintf(pObj->container.caId, "%s", "0x0000");
    pObj->container.caType = oc_hostSecurityType_other;

#ifdef MPOD_SUPPORT
    pCablecard = pSnmp->_pModel->getCableCard();
    cablecard = cablecard_get_instance();
    if (pCablecard && cablecard && cablecard->cablecard_in)
    {
        info = pCablecard->cablecard_get_info();
        if (info)
        {
            sprintf(pObj->container.caId, "0x%04x", info->CAId);
            pObj->container.caType = oc_hostSecurityType_cablecard;
        }
    }
#endif

	return;
}

void CSnmp::snmp_get_power(void *context, int param)
{
	mibObjectPower *pObj = (mibObjectPower *) context;
    ePowerMode mode;

    CControl * pControl = g_pControl;

    BSTD_UNUSED(param);

    mode = pControl ? pControl->getPowerMode() : ePowerMode_Max;

    switch (mode)
    {
        case ePowerMode_S1:
        case ePowerMode_S2:
        case ePowerMode_S3:
            pObj->container.powerStatus = oc_powerStatus_standby;
            break;
        case ePowerMode_S0:
        case ePowerMode_Max:
        default:
            pObj->container.powerStatus = oc_powerStatus_powerOn;
            break;
    }

	return;
}

void CSnmp::snmp_get_user_settings(void *context, int param)
{
	mibObjectUserSettings *pObj = (mibObjectUserSettings *) context;

	BSTD_UNUSED(param);

	/* ISO 639.2 Codes */
	/* http://www.loc.gov/standards/iso639-2/langhome.html */
	sprintf(pObj->container.preferredLanguage, "%s", "eng");

	return;
}

void CSnmp::snmp_get_system_memory_report_table(void *context, int param)
{
	mibObjectSystemMemoryReportTable *pObj = (mibObjectSystemMemoryReportTable *) context;

	switch (param)
	{
		case 0:
			pObj->pContainer[0]->memoryType = oc_memoryType_dram;
			pObj->pContainer[0]->memorySize = 512*1024; /* kilobytes */
			break;
		case 1:
			pObj->pContainer[1]->memoryType = oc_memoryType_flash;
			pObj->pContainer[1]->memorySize = 256*1024; /* kilobytes */
			break;
		case 2:
			pObj->pContainer[2]->memoryType = oc_memoryType_internalHardDrive;
			pObj->pContainer[2]->memorySize = 20*1024*1024; /* kilobytes */
			break;
	}

	return;
}

#ifdef MPOD_SUPPORT
/* map CCCP2.0 Card validation status to OC-HOST MIB CardBindingStatus */
const int CardBindStatus[B_MPOD_CP_MAX_AUTH_STATUS]  = {
	oc_cardBinding_unknown,
	oc_cardBinding_otherAuthFailure,
	oc_cardBinding_invalidCertificate,
	oc_cardBinding_otherAuthFailure,
	oc_cardBinding_otherAuthFailure,
	oc_cardBinding_otherAuthFailure,
	oc_cardBinding_bound,
	oc_cardBinding_bound,
	oc_cardBinding_otherAuthFailure
};
#endif

void CSnmp::snmp_get_card_info(void *context, int param)
{
	mibObjectCardInfo *pObj = (mibObjectCardInfo*) context;
#ifdef MPOD_SUPPORT
    CSnmp * pSnmp = g_pSnmp;
    CableCARDInfo *info = NULL;
    unsigned char *ptr = NULL;
    CCablecard * pCablecard = NULL;
    cablecard_t cablecard = NULL;
#endif

	BSTD_UNUSED(param);

    BKNI_Memset(&pObj->container, 0, sizeof(containerCardInfo));
    pObj->container.cardBindingStatus = oc_cardBinding_unknown;

#ifdef MPOD_SUPPORT
    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    pCablecard = pSnmp->_pModel->getCableCard();
    cablecard = cablecard_get_instance();
    if (pCablecard && cablecard && cablecard->cablecard_in)
    {
        info = pCablecard->cablecard_get_info();
        if (info)
        {
            int i;
            /* TODO:: convert info->cardID into DisplayString */
            sprintf(pObj->container.cardId, "%s", "1-111-111-111-111");
            BKNI_Memcpy(pObj->container.macAddr, info->macaddr, 6);
            BKNI_Memset(pObj->container.ipAddr, 0, 16);
            pObj->container.ipType = oc_inetAddrType_unknown;
            for (i=0;i<16;i++)
            {
                if (info->ipaddr[i])
                {
                    BKNI_Memcpy(pObj->container.ipAddr, info->ipaddr,16);
                    pObj->container.ipType = oc_inetAddrType_ipv4; /* TODO:: hardcoded as ipv4 */
                    break;
                }
            }
            pObj->container.cardBindingStatus = CardBindStatus[info->status];

            ptr = (unsigned char *) &info->openedResrc;
            pObj->container.opendGenericResource[0] = *(ptr+3);
            pObj->container.opendGenericResource[1] = *(ptr+2);
            pObj->container.opendGenericResource[2] = *(ptr+1);
            pObj->container.opendGenericResource[3] = *(ptr);

            pObj->container.timeZoneOffset = info->timezoneOffset;
            pObj->container.daylightSavingsTimeDelta = info->DSTimeDelta & 0xff;
            pObj->container.daylightSavingsTimeEntry = info->DSTimeEntry;
            pObj->container.daylightSavingsTimeExit = info->DSTimeExit;
            BKNI_Memcpy(pObj->container.eaLocationCode, info->EALocCode, 3);

            ptr = (unsigned char *) &info->vctId;
            pObj->container.vctId[0] = *(ptr+1);
            pObj->container.vctId[1] = *(ptr);
        }
    }
 #endif

	return;
}

void CSnmp::snmp_get_card_cp_info(void *context, int param)
{
	mibObjectCardCpInfo *pObj = (mibObjectCardCpInfo*) context;
#ifdef MPOD_SUPPORT
    CSnmp * pSnmp = g_pSnmp;
	CableCARDInfo *info = NULL;
	unsigned char *ptr = NULL;
    CCablecard * pCablecard = NULL;
    cablecard_t cablecard = NULL;
#endif

	BSTD_UNUSED(param);

    BKNI_Memset(&pObj->container, 0, sizeof(containerCardCpInfo));

#ifdef MPOD_SUPPORT
    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    pCablecard = pSnmp->_pModel->getCableCard();
    cablecard = cablecard_get_instance();
    if (pCablecard && cablecard && cablecard->cablecard_in)
    {
        info = pCablecard->cablecard_get_info();
        if (info)
        {
            pObj->container.authKeyStatus = (info->authKeyStatus == true) ? oc_cardCpAuthKeyStatus_ready : oc_cardCpAuthKeyStatus_notReady;
            pObj->container.certCheck = (info->certCheck == true) ? oc_cardCpCertCheck_failed : oc_cardCpCertCheck_ok;
            pObj->container.cciCount = info->cciCount;
            pObj->container.keyGenReqCount = info->CPkeyCount;

            ptr = (unsigned char *) &info->CPId;
            pObj->container.idList[0] = *(ptr+3);
            pObj->container.idList[1] = *(ptr+2);
            pObj->container.idList[2] = *(ptr+1);
            pObj->container.idList[3] = *(ptr);
        }
    }
#endif

	return;
}

#ifdef MPOD_SUPPORT
static char * ccmmi_html = NULL;
static unsigned ccmmi_len = 0;
/* To recieve the various App info thro a callback for the URLs sent  */
static void snmp_receive_app_info_query(uint8_t *html, unsigned len, uint8_t dialogNb, uint8_t fileStatus)
{
	BSTD_UNUSED(dialogNb);
	BSTD_UNUSED(fileStatus);

	if (len)
	{
		if (!ccmmi_html || ccmmi_len < len)
        {
            ccmmi_html = (char *) realloc(ccmmi_html, len);
		}

		BKNI_Memcpy(ccmmi_html, html, len);
	}

	ccmmi_len = len;
}
#endif

void CSnmp::snmp_get_cc_app_info_table(void *context, int param)
{
	mibObjectCCAppInfoTable *pObj = (mibObjectCCAppInfoTable *) context;
#ifdef MPOD_SUPPORT
    CSnmp * pSnmp = g_pSnmp;
    CCablecard * pCablecard = NULL;
    cablecard_t cablecard = NULL;
#endif

	if (pObj->pContainer[param]->appInfoPage) free(pObj->pContainer[param]->appInfoPage);
	BKNI_Memset(pObj->pContainer[param], 0, sizeof(containerCCAppInfoTable));

#ifdef MPOD_SUPPORT
    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    pCablecard = pSnmp->_pModel->getCableCard();
    cablecard = cablecard_get_instance();
	if (pCablecard && cablecard && cablecard->cablecard_in)
	{
		unsigned short length = 0;
		char url_query[256];
		char *url_ptr;
		uint8_t dialogNb;
		uint8_t urlLen;
        uint8_t max_num_pages;
		MPOD_RET_CODE err;
		uint8_t type;
		unsigned version;
		uint8_t appLen;
		char *appName;
		int timeout = 20;

		err = B_Mpod_AppInfoGetNumApps(&max_num_pages);
		if(err || param >= max_num_pages) return;

        err = B_Mpod_AppInfoGetType(param, &type);
        if(err) return;
        err = B_Mpod_AppInfoGetVersion(param, &version);
        if(err) return;
        err = B_Mpod_AppInfoGetName(param, &appName, &appLen);
        if(err) return;
        err = B_Mpod_AppInfoGetURL(param, &url_ptr, &urlLen);
        if(err) return;
		strcpy(url_query, url_ptr);
		length = strlen(url_query);
		pCablecard->cablecard_set_html_callback(cablecard, snmp_receive_app_info_query);
		ccmmi_len = 0;
		err = B_Mpod_MMIHostOpenDialog(url_query, length, &dialogNb);
        if(err) return;
		pObj->pContainer[param]->appInfoIndex = param;
		BKNI_Memcpy(pObj->pContainer[param]->appName, appName, appLen);
		pObj->pContainer[param]->appName[appLen] = '\0';
		pObj->pContainer[param]->appType = type;
		pObj->pContainer[param]->appVersion = version;
		while ( timeout-- && ccmmi_len == 0)
		{
			BKNI_Sleep(50);
		}
		if (ccmmi_len)
		{
			pObj->pContainer[param]->appInfoPage = (char*)malloc(ccmmi_len);
			if (pObj->pContainer[param]->appInfoPage)
			{
				BKNI_Memcpy(pObj->pContainer[param]->appInfoPage, ccmmi_html, ccmmi_len);
				pObj->pContainer[param]->appInfoLen = ccmmi_len;
			}
		}
	}
#endif

	return;
}

void CSnmp::snmp_get_snmpproxy_info(void *context, int param)
{
	mibObjectSnmpProxyInfo *pObj = (mibObjectSnmpProxyInfo*) context;
#ifdef MPOD_SUPPORT
    CSnmp * pSnmp = g_pSnmp;
	CableCARDInfo *info = NULL;
	unsigned char *ptr = NULL;
    CCablecard * pCablecard = NULL;
    cablecard_t cablecard = NULL;
#endif

	BSTD_UNUSED(param);

    BKNI_Memset(&pObj->container, 0, sizeof(containerSnmpProxyInfo));

#ifdef MPOD_SUPPORT
    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    pCablecard = pSnmp->_pModel->getCableCard();
    cablecard = cablecard_get_instance();
	if (pCablecard && cablecard && cablecard->cablecard_in)
	{
        info = pCablecard->cablecard_get_info();
        if (info)
        {
            ptr = (unsigned char *) &info->vendorId;
            pObj->container.cardMfgId[0] = *(ptr+1);
            pObj->container.cardMfgId[1] = *(ptr);

            ptr = (unsigned char *) &info->version;
            pObj->container.cardVersion[0] = *(ptr+1);
            pObj->container.cardVersion[1] = *(ptr);

            if (info->rootoidLen && info->rootoidLen <= MAX_OID_LEN)
            {
                pObj->container.rootOidLen = info->rootoidLen;
                BKNI_Memcpy(pObj->container.cardRootOid, info->rootoid, info->rootoidLen*sizeof(oid));
            }
            else
            {
                pObj->container.rootOidLen = 0;
                BDBG_MSG(("Card Root OID error length %d", info->rootoidLen));
            }

            if (info->serialLen)
            {
                BKNI_Memcpy(pObj->container.cardSerialNum, info->serialNum, info->serialLen);
                pObj->container.cardSerialNum[info->serialLen] = '\0';
            }
            else
            {
                sprintf(pObj->container.cardSerialNum, "%s", "unknown");
            }
        }
	}
#endif

	return;
}

void CSnmp::snmp_set_snmpproxy_info(void *context, int param)
{
	mibObjectSnmpProxyInfo *pObj = (mibObjectSnmpProxyInfo*) context;

	pObj->container.discardCardSnmpAccessControl = param;

	if (pObj->container.discardCardSnmpAccessControl)
	{
		BDBG_MSG(("Discard SNMP messages to the Card subtree OID"));
	}
	else
	{
		BDBG_MSG(("Forward SNMP messages to the Card subtree OID"));
	}

	return;
}

static int snmp_get_estb_ip_addr(char *if_name, unsigned char *ip_addr, unsigned char *mask)
{
    struct ifreq ifr;
    int sock, status;
    struct sockaddr_in *sin = (struct sockaddr_in *)&(ifr.ifr_addr);

    /* Query the existing IP address and subnet mask. */
    BKNI_Memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_ifrn.ifrn_name, if_name);
    sin->sin_family = AF_INET;

    sock = socket (AF_INET, SOCK_PACKET, 0);
    if (sock < 0)
    {
        BDBG_MSG((" Failed to open socket!"));
        goto err;
    }
    status = ioctl(sock, SIOCGIFADDR, &ifr) ;
    if( status )
    {
        BDBG_MSG((" ERROR- SIOCGIFADDR: %s  %s errno = [%d]", if_name, strerror(errno), errno));
        goto err;
    }
    BKNI_Memcpy( ip_addr, (uint8_t*)&(sin->sin_addr.s_addr), sizeof(unsigned int));

    status = ioctl(sock, SIOCGIFNETMASK, &ifr);
    if( status )
    {
        BDBG_MSG((" ERROR- SIOCGIFNETMASK: %s  %s errno = [%d]", if_name, strerror(errno), errno));
        goto err;
    }
    BKNI_Memcpy( mask, (uint8_t*)&(sin->sin_addr.s_addr), sizeof(unsigned int));
    close(sock);
    return 0;
err:
    return -1;
}

void CSnmp::snmp_get_host_info(void *context, int param)
{
	mibObjectHostInfo *pObj = (mibObjectHostInfo *) context;
#if BCHP_CHIP == 7420
	char iface[]="eth2";
#elif (BCHP_CHIP == 7425 || BCHP_CHIP == 7429 || BCHP_CHIP == 7435 || BCHP_CHIP == 7445 )
	char iface[]="eth0";
#elif BCHP_CHIP == 7125
	char iface[]="bcm0";
#else
	char iface[]="eth1";
#endif
	unsigned char ipAddr[16] = { 0 };
#ifdef MPOD_SUPPORT
    CSnmp * pSnmp = g_pSnmp;
    CableCARDInfo *info = NULL;
    CCablecard * pCablecard = NULL;
    cablecard_t cablecard = NULL;
#endif

	BSTD_UNUSED(param);

	snmp_get_estb_ip_addr(iface, ipAddr, (unsigned char*) pObj->container.subNetMask);
	pObj->container.ipType = oc_inetAddrType_ipv4;
    pObj->container.oobMessageMode = oc_hostOOBType_other;

#ifdef MPOD_SUPPORT
    BDBG_ASSERT(pSnmp != NULL);
	BDBG_ASSERT(pSnmp->_pModel != NULL);

    pCablecard = pSnmp->_pModel->getCableCard();
    cablecard = cablecard_get_instance();
	if (pCablecard && cablecard && cablecard->cablecard_in)
	{
        info = pCablecard->cablecard_get_info();
        if (info)
        {
            pObj->container.oobMessageMode = (info->mode == B_MPOD_EXT_OOB) ? oc_hostOOBType_scte55 : oc_hostOOBType_dsg;
        }
	}
#endif

    pObj->container.bootStatus= oc_bootStatus_completedSuccessfully;

	return;
}

void CSnmp::snmp_get_dump_trap_info(void *context, int param)
{
	mibObjectDumpTrapInfo *pObj = (mibObjectDumpTrapInfo *) context;

	BSTD_UNUSED(param);

	pObj->container.dumpEventCount = dumpTrapInfo.dumpEventCount;
	pObj->container.dumpNow = dumpTrapInfo.dumpNow;
	pObj->container.dumpEventTimeout = dumpTrapInfo.dumpEventTimeout;
	sprintf(pObj->container.dumpFilePath, "%s", dumpTrapInfo.dumpFilePath);

	return;
}

void CSnmp::snmp_set_dump_trap_info(void *context, int param)
{
	char ipaddr[] = "127.0.0.1";

	switch(param)
	{
		case DUMP_EVENT_COUNT:
			dumpTrapInfo.dumpEventCount = *((int *) context);
			break;
		case DUMP_NOW:
			/* add code to force immediate dump */
			BDBG_WRN(("!!!!!!!! Got DUMP NOW !!!!!!!!"));
			BDBG_WRN(("!!!!!!!! Prepare the dump file and send to NMS !!!!!!!!"));
			B_SNMP_Trap(ipaddr, "ocStbPanicDumpTrap",
				"ocStbHostDumpFilePath.0", B_SNMP_TYPE_OCTETSTRING, dumpTrapInfo.dumpFilePath);
			BDBG_WRN(("!!!!!!!! DUMP TRAP is Sent to %s !!!!!!!!",ipaddr ));
			break;
		case DUMP_EVENT_TIMEOUT:
			dumpTrapInfo.dumpEventTimeout = *((unsigned *) context);
			break;
	}

	return;
}

void CSnmp::snmp_get_spec_info(void *context, int param)
{
	mibObjectSpecInfo *pObj = (mibObjectSpecInfo *) context;

	BSTD_UNUSED(param);

	sprintf(pObj->container.cfrSpecIssue, "%s", "OC-SP-HOST2.1-CFR-I11-100507");
	sprintf(pObj->container.mibSpecIssue, "%s", "OC-SP-MIB-HOST2.X-I11-110204");

	return;
}

void CSnmp::snmp_get_content_error_summary_info(void *context, int param)
{
	mibObjectContentErrorSummaryInfo *pObj = (mibObjectContentErrorSummaryInfo *) context;
	int patTimeoutCount, pmtTimeoutCount;

	BSTD_UNUSED(param);

	tsPsi_getTimeoutCnt(&patTimeoutCount, &pmtTimeoutCount);

	pObj->container.patTimeoutCount = patTimeoutCount;
	pObj->container.pmtTimeoutCount = pmtTimeoutCount;

#ifdef CDL_SUPPORT
    B_Cdl_Status status;
	if (B_Cdl_Get_Status(&status) >= 0) {
		pObj->container.oobCarouselTimeoutCount = status.inbandCarouselTimeoutUs;
		pObj->container.inbandCarouselTimeoutCount = status.inbandCarouselTimeoutCount;
    }
#else
	pObj->container.oobCarouselTimeoutCount = 0;
	pObj->container.inbandCarouselTimeoutCount = 0;
#endif

	return;
}

void CSnmp::snmp_set_reboot_info(void *context, int param)
{
	int reboot_type = oc_rebootType_user;

	BSTD_UNUSED(context);
	BSTD_UNUSED(param);

	/* save reboot reason code in non-vol*/
	if (reboot_type < oc_rebootType_unknown || reboot_type > oc_rebootType_cablecardError)
	{
		BDBG_ERR(("Unknown ocStbHostRebootType"));
	}
#ifdef ESTB_CFG_SUPPORT
	else
		B_Estb_cfg_Set_uint32((char *)"/sys/estb/reboot_type", reboot_type);
#endif
	/* add code to do system reset here */
	printf("!!!!!!!! System will shut down !!!!!!!!\n");
	/* close current PVR operation, clean up files */
	sleep(1);
	system("reboot");

	return;
}

void CSnmp::snmp_get_reboot_info(void *context, int param)
{
	mibObjectRebootInfo *pObj = (mibObjectRebootInfo *) context;
	int reboot_type = oc_rebootType_unknown;
#ifdef ESTB_CFG_SUPPORT
	int ret;
#endif

	BSTD_UNUSED(param);

	/* read the reboot type from the last reboot if any */
#ifdef ESTB_CFG_SUPPORT
	ret = B_Estb_cfg_Get_uint32((char *)"/sys/estb/reboot_type", (unsigned int*)&reboot_type);
#endif

	if (
#ifdef ESTB_CFG_SUPPORT
        ret ||
#endif
        reboot_type < oc_rebootType_unknown || reboot_type > oc_rebootType_cablecardError)
	{
		reboot_type = oc_rebootType_unknown;
	}
	pObj->container.rebootType = reboot_type;

	return;
}

void CSnmp::snmp_get_jvm_info(void *context, int param)
{
	mibObjectJvmInfo *pObj = (mibObjectJvmInfo *) context;

	BSTD_UNUSED(param);

	pObj->container.heapSize = 10*1024;
	pObj->container.availHeap = 10*1024;
	pObj->container.liveObjects = 0;
	pObj->container.deadObjects = 0;

	return;
}

void CSnmp::snmp_proxy_mibs_open(void)
{
	B_SNMP_ProxyMibInit();
}

void CSnmp::snmp_proxy_mibs_close(void)
{
	B_SNMP_ProxyMibUninit();
}

void CSnmp::snmp_proxy_mibs_register(void)
{
	mibObjectHwIdentifiers *pHwIdentifiers;
	mibObjectIeee1394Table *pIeee1394Table;
	mibObjectIeee1394ConnectedDevicesTable *pIeee1394ConnectedDevicesTable;
	mibObjectDviHdmiTable *pDviHdmiTable;
	mibObjectDviHdmiAvailableVideoFormatTable *pDviHdmiAvailableVideoFormatTable;
	mibObjectComponentVideoTable *pComponentVideoTable;
	mibObjectRfChannelOutTable *pRfChannelOutTable;
	mibObjectInBandTunerTable *pInBandTunerTable;
	mibObjectProgramStatusTable *pProgramStatusTable;
	mibObjectMpeg2ContentTable *pMpeg2ContentTable;
	mibObjectAnalogVideoTable *pAnalogVideoTable;
	mibObjectQpskObjects *pQpskObjects;
	mibObjectSPDIfTable *pSPDifTable;
	mibObjectEasCodes *pEasCodes;
	mibObjectDeviceSoftwareBase *pDeviceSoftwareBase;
	mibObjectFwDownloadStatus *pFwDownloadStatus;
	mibObjectSwAppInfo *pSwAppInfo;
	mibObjectSwAppInfoTable *pSwAppInfoTable;
	mibObjectSecuritySubSystem *pSecuritySubSystem;
	mibObjectPower *pPower;
	mibObjectUserSettings *pUserSettings;
	mibObjectSystemMemoryReportTable *pSystemMemoryReportTable;
	mibObjectCardInfo *pCardInfo;
	mibObjectCardCpInfo *pCardCpInfo;
	mibObjectCCAppInfoTable *pCCAppInfoTable;
	mibObjectSnmpProxyInfo *pSnmpproxyInfo;
	mibObjectHostInfo *pHostInfo;
	mibObjectDumpTrapInfo *pDumpTrapInfo;
	mibObjectSpecInfo *pSpecInfo;
	mibObjectContentErrorSummaryInfo *pContentErrorSummaryInfo;
	mibObjectRebootInfo *pRebootInfo;
	mibObjectJvmInfo *pJvmInfo;

	B_SNMP_ProxyMibGetDefault(ocStbHostHWIdentifiers, (void **)&pHwIdentifiers);
	pHwIdentifiers->callback = snmp_get_hw_indentifiers;
	B_SNMP_ProxyMibRegister(ocStbHostHWIdentifiers);

	B_SNMP_ProxyMibRegister(ocStbHostAVInterfaceTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostIEEE1394Table, (void **)&pIeee1394Table);
	pIeee1394Table->callback = snmp_get_ieee1394_table;
	B_SNMP_ProxyMibRegister(ocStbHostIEEE1394Table);

	B_SNMP_ProxyMibGetDefault(ocStbHostIEEE1394ConnectedDevicesTable, (void **)&pIeee1394ConnectedDevicesTable);
	pIeee1394ConnectedDevicesTable->callback = snmp_get_ieee1394_connected_devices_table;
	B_SNMP_ProxyMibRegister(ocStbHostIEEE1394ConnectedDevicesTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostDVIHDMITable, (void **)&pDviHdmiTable);
	pDviHdmiTable->callback = snmp_get_dvi_hdmi_table;
	B_SNMP_ProxyMibRegister(ocStbHostDVIHDMITable);

	B_SNMP_ProxyMibGetDefault(ocStbHostDVIHDMIAvailableVideoFormatTable, (void **)&pDviHdmiAvailableVideoFormatTable);
	pDviHdmiAvailableVideoFormatTable->callback = snmp_get_dvi_hdmi_available_video_format_table;
	B_SNMP_ProxyMibRegister(ocStbHostDVIHDMIAvailableVideoFormatTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostComponentVideoTable, (void **)&pComponentVideoTable);
	pComponentVideoTable->callback = snmp_get_component_video_table;
	B_SNMP_ProxyMibRegister(ocStbHostComponentVideoTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostRFChannelOutTable, (void **)&pRfChannelOutTable);
	pRfChannelOutTable->callback = snmp_get_rf_channel_out_table;
	B_SNMP_ProxyMibRegister(ocStbHostRFChannelOutTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostInBandTunerTable, (void **)&pInBandTunerTable);
	pInBandTunerTable->callback = snmp_get_in_band_tuner_table;
	B_SNMP_ProxyMibRegister(ocStbHostInBandTunerTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostProgramStatusTable, (void **)&pProgramStatusTable);
	pProgramStatusTable->callback = snmp_get_program_status_table;
	B_SNMP_ProxyMibRegister(ocStbHostProgramStatusTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostMpeg2ContentTable, (void **)&pMpeg2ContentTable);
	pMpeg2ContentTable->callback = snmp_get_mpeg2_content_table;
	B_SNMP_ProxyMibRegister(ocStbHostMpeg2ContentTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostAnalogVideoTable, (void **)&pAnalogVideoTable);
	pAnalogVideoTable->callback = snmp_get_analog_video_table;
	B_SNMP_ProxyMibRegister(ocStbHostAnalogVideoTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostQpskObjects, (void **)&pQpskObjects);
	pQpskObjects->callback = snmp_get_qpsk_objects;
	B_SNMP_ProxyMibRegister(ocStbHostQpskObjects);

	B_SNMP_ProxyMibGetDefault(ocStbHostSPDIfTable, (void **)&pSPDifTable);
    pSPDifTable->callback= snmp_get_spdif_table;
    B_SNMP_ProxyMibRegister(ocStbHostSPDIfTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostEasCodes , (void **)&pEasCodes);
	pEasCodes->callback = snmp_get_eas_codes;
	B_SNMP_ProxyMibRegister(ocStbHostEasCodes);

	B_SNMP_ProxyMibGetDefault(ocStbHostDeviceSoftwareBase , (void **)&pDeviceSoftwareBase);
	pDeviceSoftwareBase->callback = snmp_get_device_software_base;
	B_SNMP_ProxyMibRegister(ocStbHostDeviceSoftwareBase);

	B_SNMP_ProxyMibGetDefault(ocStbHostFirmwareDownloadStatus, (void **)&pFwDownloadStatus);
	pFwDownloadStatus->callback = snmp_get_fw_download_status;
	B_SNMP_ProxyMibRegister(ocStbHostFirmwareDownloadStatus);

	B_SNMP_ProxyMibGetDefault(ocStbHostSoftwareApplicationInfo, (void **)&pSwAppInfo);
	pSwAppInfo->callback = snmp_get_sw_app_info;
	B_SNMP_ProxyMibRegister(ocStbHostSoftwareApplicationInfo);

	B_SNMP_ProxyMibGetDefault(ocStbHostSoftwareApplicationInfoTable, (void **)&pSwAppInfoTable);
	pSwAppInfoTable->callback = snmp_get_sw_app_info_table;
	B_SNMP_ProxyMibRegister(ocStbHostSoftwareApplicationInfoTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostSecuritySubSystem, (void **)&pSecuritySubSystem);
	pSecuritySubSystem->callback = snmp_get_security_subsystem;
	B_SNMP_ProxyMibRegister(ocStbHostSecuritySubSystem);

	B_SNMP_ProxyMibGetDefault(ocStbHostPower, (void **)&pPower);
	pPower->callback = snmp_get_power;
	B_SNMP_ProxyMibRegister(ocStbHostPower);

	B_SNMP_ProxyMibGetDefault(ocStbHostUserSettings, (void **)&pUserSettings);
	pUserSettings->callback = snmp_get_user_settings;
	B_SNMP_ProxyMibRegister(ocStbHostUserSettings);

	B_SNMP_ProxyMibGetDefault(ocStbHostSystemMemoryReportTable, (void **)&pSystemMemoryReportTable);
	pSystemMemoryReportTable->callback = snmp_get_system_memory_report_table;
	B_SNMP_ProxyMibRegister(ocStbHostSystemMemoryReportTable);

	B_SNMP_ProxyMibGetDefault(ocStbCardInfo, (void **)&pCardInfo);
	pCardInfo->callback = snmp_get_card_info;
	B_SNMP_ProxyMibRegister(ocStbCardInfo);

	B_SNMP_ProxyMibGetDefault(ocStbHostCardCpInfo, (void **)&pCardCpInfo);
	pCardCpInfo->callback = snmp_get_card_cp_info;
	B_SNMP_ProxyMibRegister(ocStbHostCardCpInfo);

	B_SNMP_ProxyMibGetDefault(ocStbHostCCAppInfoTable, (void **)&pCCAppInfoTable);
	pCCAppInfoTable->callback = snmp_get_cc_app_info_table;
	B_SNMP_ProxyMibRegister(ocStbHostCCAppInfoTable);

	B_SNMP_ProxyMibGetDefault(ocStbHostSnmpProxyInfo, (void **)&pSnmpproxyInfo);
	pSnmpproxyInfo->get_callback = snmp_get_snmpproxy_info;
	pSnmpproxyInfo->set_callback = snmp_set_snmpproxy_info;
	B_SNMP_ProxyMibRegister(ocStbHostSnmpProxyInfo);

	B_SNMP_ProxyMibGetDefault(ocStbHostInfo, (void **)&pHostInfo);
	pHostInfo->callback = snmp_get_host_info;
	B_SNMP_ProxyMibRegister(ocStbHostInfo);

	B_SNMP_ProxyMibGetDefault(ocStbHostDumpTrapInfo, (void **)&pDumpTrapInfo);
	pDumpTrapInfo->get_callback = snmp_get_dump_trap_info;
	pDumpTrapInfo->set_callback = snmp_set_dump_trap_info;
	B_SNMP_ProxyMibRegister(ocStbHostDumpTrapInfo);

	B_SNMP_ProxyMibGetDefault(ocStbHostSpecificationsInfo, (void **)&pSpecInfo);
	pSpecInfo->callback = snmp_get_spec_info;
	B_SNMP_ProxyMibRegister(ocStbHostSpecificationsInfo);

	B_SNMP_ProxyMibGetDefault(ocStbHostContentErrorSummaryInfo, (void **)&pContentErrorSummaryInfo);
	pContentErrorSummaryInfo->callback = snmp_get_content_error_summary_info;
	B_SNMP_ProxyMibRegister(ocStbHostContentErrorSummaryInfo);

	B_SNMP_ProxyMibGetDefault(ocStbHostRebootInfo, (void **)&pRebootInfo);
	pRebootInfo->set_callback = snmp_set_reboot_info;
	pRebootInfo->get_callback = snmp_get_reboot_info;
	B_SNMP_ProxyMibRegister(ocStbHostRebootInfo);

	B_SNMP_ProxyMibRegister(ocStbHostMemoryInfo);

	B_SNMP_ProxyMibGetDefault(ocStbHostJVMInfo, (void **)&pJvmInfo);
	pJvmInfo->callback = snmp_get_jvm_info;
	B_SNMP_ProxyMibRegister(ocStbHostJVMInfo);

	return;
}

void CSnmp::snmp_proxy_mibs_unregister(void)
{
	B_SNMP_ProxyMibUnregister(ocStbHostHWIdentifiers);
	B_SNMP_ProxyMibUnregister(ocStbHostAVInterfaceTable);
	B_SNMP_ProxyMibUnregister(ocStbHostIEEE1394Table);
	B_SNMP_ProxyMibUnregister(ocStbHostIEEE1394ConnectedDevicesTable);
	B_SNMP_ProxyMibUnregister(ocStbHostDVIHDMITable);
	B_SNMP_ProxyMibUnregister(ocStbHostDVIHDMIAvailableVideoFormatTable);
	B_SNMP_ProxyMibUnregister(ocStbHostComponentVideoTable);
	B_SNMP_ProxyMibUnregister(ocStbHostRFChannelOutTable);
	B_SNMP_ProxyMibUnregister(ocStbHostInBandTunerTable);
	B_SNMP_ProxyMibUnregister(ocStbHostProgramStatusTable);
	B_SNMP_ProxyMibUnregister(ocStbHostMpeg2ContentTable);
	B_SNMP_ProxyMibUnregister(ocStbHostAnalogVideoTable);
	B_SNMP_ProxyMibUnregister(ocStbHostQpskObjects);
	B_SNMP_ProxyMibUnregister(ocStbHostSPDIfTable);
	B_SNMP_ProxyMibUnregister(ocStbHostEasCodes);
	B_SNMP_ProxyMibUnregister(ocStbHostDeviceSoftwareBase);
	B_SNMP_ProxyMibUnregister(ocStbHostFirmwareDownloadStatus);
	B_SNMP_ProxyMibUnregister(ocStbHostSoftwareApplicationInfo);
	B_SNMP_ProxyMibUnregister(ocStbHostSoftwareApplicationInfoTable);
	B_SNMP_ProxyMibUnregister(ocStbHostSecuritySubSystem);
	B_SNMP_ProxyMibUnregister(ocStbHostPower);
	B_SNMP_ProxyMibUnregister(ocStbHostUserSettings);
	B_SNMP_ProxyMibUnregister(ocStbHostSystemMemoryReportTable);
	B_SNMP_ProxyMibUnregister(ocStbCardInfo);
	B_SNMP_ProxyMibUnregister(ocStbHostCardCpInfo);
	B_SNMP_ProxyMibUnregister(ocStbHostCCAppInfoTable);
	B_SNMP_ProxyMibUnregister(ocStbHostSnmpProxyInfo);
	B_SNMP_ProxyMibUnregister(ocStbHostInfo);
	B_SNMP_ProxyMibUnregister(ocStbHostDumpTrapInfo);
	B_SNMP_ProxyMibUnregister(ocStbHostSpecificationsInfo);
	B_SNMP_ProxyMibUnregister(ocStbHostContentErrorSummaryInfo);
	B_SNMP_ProxyMibUnregister(ocStbHostRebootInfo);
	B_SNMP_ProxyMibUnregister(ocStbHostMemoryInfo);
	B_SNMP_ProxyMibUnregister(ocStbHostJVMInfo);

	return;
}

#endif
