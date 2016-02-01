/****************************************************************************************************
 * WVDictionaryKeys.h
 *
 * Key and value constants for dictionaries
 *
 * (c) Copyright 2011 Google, Inc.
 ***************************************************************************************************/

#ifndef __WVDICTIONARYKEYS_H__
#define __WVDICTIONARYKEYS_H__


// Keys for Credentials dictionary
#define kCredentialsKey_DeviceId	"CredentialsKey_DeviceId"	 // Device ID (WVString)
#define kCredentialsKey_ClientId	"CredentialsKey_ClientId"	 // Client ID (WVString)
#define kCredentialsKey_StreamId	"CredentialsKey_StreamId"	 // Stream ID (WVString)
#define kCredentialsKey_SessionId	"CredentialsKey_SessionId"	 // Session ID (WVString)
#define kCredentialsKey_ClientIp	"CredentialsKey_ClientIp"	 // Client IP (WVString)
#define kCredentialsKey_Portal		"CredentialsKey_Portal"		 // Media portal (WVString)
#define kCredentialsKey_Storefront	"CredentialsKey_Storefront"	 // Media storefront (WVString)
#define kCredentialsKey_OptionalData	"CredentialsKey_OptionalData"	 // Optional data (WVString)
#define kCredentialsKey_AssetRegistryId	"CredentialsKey_AssetRegistryId" // Asset Registry ID to store/retrieve licenses (WVString)
#define kCredentialsKey_DrmServerUrl	"CredentialsKey_DrmServerUrl"	 // DRM server URL to retrieve entitlements (WVString)
#define kCredentialsKey_DrmAckServerUrl	"CredentialsKey_DrmAckServerUrl" // DRM ACK server URL (WVString)
#define kCredentialsKey_HeartbeatUrl	"CredentialsKey_HeartbeatUrl"	 // Heartbeat sever URL (WVString)
#define kCredentialsKey_DrmLicenseUsageUrl	"CredentialsKey_DrmLicenseUsageUrl" // DRM License usage URL (WVString)
#define kCredentialsKey_SignOnUrl	"CredentialsKey_SignOnUrl"	 // Sign-on server URL (WVString)
#define kCredentialsKey_AppSignatureUrl	"CredentialsKey_AppSignatureUrl" // DCP application signature URL (WVString)
#define kCredentialsKey_DcpLogUrl	"CredentialsKey_DcpLogUrl" 	 // DCP logging URL (WVString)
#define kCredentialsKey_HeartbeatPeriod	"CredentialsKey_HeartbeatPeriod" // Heartbeat period (WVUnsignedInt)
#define kCredentialsKey_UserAgent	"CredentialsKey_UserAgent" 	 // UserAgent for HTTP requests (WVString)
#define kCredentialsKey_PreloadTimeout  "CredentialsKey_PreloadTimeout"  // Preload timeout in microseconds (WVUnsignedInt)
#define kCredentialsKey_CacheSize  	"CredentialsKey_CacheSize"  	 // Size of donwload/adaptive cache in bytes (WVUnsignedInt).
#define kCredentialsKey_DataStorePath   "CredentialsKey_DataStorePath"   // Optional path to license data store (WVString)
#define kCredentialsKey_PlaybackMode    "CredentialsKey_PlaybackMode"    // Playback mode (WVUnsignedInt), see kPlaybackMode constants
#define kCredentialsKey_CaToken    	"CredentialsKey_CaToken"	 // Conditional access token
#define kCredentialsKey_CommandChannel  "CredentialsKey_CommandChannel"  // Enable control back channel (WVBoolean).
#define kCredentialsKey_Pda  		"CredentialsKey_Pda"  	 	 // Enable player-driven adaptation (WVBoolean)
#define kCredentialsKey_AssetFsRoot	"CredentialsKey_AssetFsRoot"	 // File system root directory for assets (WVString)
#define kCredentialsKey_UseJSON         "CredentialsKey_UseJSON"         // Use JSON format for EMM (WVBoolean)

// Keys for Asset Registry query results dictionary
#define kAssetRegistryKey_AssetPath			"AssetRegistryKey_AssetPath"			// Asset path (WVString)
#define kAssetRegistryKey_AssetStatus			"AssetRegistryKey_AssetStatus"			// Asset status (WVUnsignedInt)


// Keys for asset information containing dictionaries
#define kAssetInfoKey_SystemId				"AssetInfoKey_SystemId"				// CA system ID (WVUnsignedInt)
#define kAssetInfoKey_AssetId				"AssetInfoKey_AssetId"				// CA asset ID (WVUnsignedInt)
#define kAssetInfoKey_KeyIndex				"AssetInfoKey_KeyIndex"				// CA key index (WVUnsignedInt)
#define kAssetInfoKey_DistributionTimeRemaining		"AssetInfoKey_DistributionTimeRemaining"	// Distribution time remaining (WVUnsignedInt)
#define kAssetInfoKey_PurchaseTimeRemaining		"AssetInfoKey_PurchaseTimeRemaining"		// Purchase time reaminng (WVUnsignedInt)
#define kAssetInfoKey_LicenseTimeRemaining		"AssetInfoKey_LicenseTimeRemaining"		// License time remaining (WVUnsignedInt)
#define kAssetInfoKey_LicenseExpiryTimeRemaining  	"AssetInfoKey_LicenseExpiryTimeRemaining"	// Overall time remaining before license expires (WVUnsignedInt)
#define kAssetInfoKey_TimeSinceFirstPlayback		"AssetInfoKey_TimeSinceFirstPlayback"		// Time since first playback (WVUnsignedInt)
#define kAssetInfoKey_CAError				"AssetInfoKey_CAError"	       			// License retrieval error (WVUnsignedInt)
#define kAssetInfoKey_MaxBitrate			"AssetInfoKey_MaxBitrate"	       		// Max bitrate (WVUnsignedInt)
#define kAssetInfoKey_MinBitrate			"AssetInfoKey_MinBitrate"	       		// Min bitrate (WVUnsignedInt)

// Keys for error information containing dictionaries
#define kErrorKey_WVStatus			"ErrorKey_WVStatus"			// WVStatus code (WVUnsignedInt)
#define kErrorKey_AdditionalInfo		"ErrorKey_AdditionalInfo"		// Error additional information (WVString)


// Keys for track information
#define kTrackInfoKey_CypherVersion		"TrackInfoKey_CypherVersion"		// Cypher version (WVString)
#define kTrackInfoKey_Id			"TrackInfoKey_Id"			// Track ID (WVUnsignedInt)
#define kTrackInfoKey_Version     		"TrackInfoKey_Version"			// Track version (WVUnsignedInt) INTERNAL
#define kTrackInfoKey_BitRate			"TrackInfoKey_BitRate"			// Required bandwidth it bits/sec (WVUnsignedInt)
#define kTrackInfoKey_Offset			"TrackInfoKey_Offset"			// Track byte offset (WVUnsignedLong) INTERNAL
#define kTrackInfoKey_Size			"TrackInfoKey_Size"			// Track size (WVUnsignedLong) INTERNAL
#define kTrackInfoKey_Duration			"TrackInfoKey_Duration"			// Track duration in us (WVUnsignedLong)
#define kTrackInfoKey_AdaptationInterval	"TrackInfoKey_AdaptationInterval"	// Nominal adaptation interval, in seconds (WVFloat)
#define kTrackInfoKey_TrickPlayRate		"TrackInfoKey_TrickPlayRate"		// Trick-play rate (WVSignedInt)
#define kTrackInfoKey_Flags			"TrackInfoKey_Flags"			// Track flags (WVUnsignedInt)
#define kTrackInfoKey_Source			"TrackInfoKey_Source"			// Source for media (WVString)
#define kTrackInfoKey_TimingOffset		"TrackInfoKey_TimingOffset"		// Track timing adjustment in seconds (WVFloat)

// Video track information
#define kTrackInfoKey_VideoFormat		"TrackInfoKey_VideoFormat"		// Type of video, valid values below (WVUnsignedInt)
#define kTrackInfoKey_VideoProfile		"TrackInfoKey_VideoProfile"		// Video profile indicator (WVUnsignedInt)
#define kTrackInfoKey_VideoLevel		"TrackInfoKey_VideoLevel"		// Video leve indicator (WVUnsignedInt)
#define kTrackInfoKey_VideoWidth		"TrackInfoKey_VideoWidth"      		// Video width, in pixels (WVUnsignedInt)
#define kTrackInfoKey_VideoHeight		"TrackInfoKey_VideoHeight"		// Video height, in lines (WVUnsignedInt)
#define kTrackInfoKey_VideoPixelWidth		"TrackInfoKey_VideoPixelWidth"		// Video pixel aspect ratio width (WVUnsignedInt)
#define kTrackInfoKey_VideoPixelHeight		"TrackInfoKey_VideoPixelHeight"		// Video pixel aspect ratio height (WVUnsignedInt)
#define kTrackInfoKey_VideoFrameRate		"TrackInfoKey_VideoFrameRate"		// Video frame rate (WVFloat)
#define kTrackInfoKey_VideoBitRate		"TrackInfoKey_VideoBitRate"		// Video bit rate, bits/sec (WVUnsignedInt)
#define kTrackInfoKey_VideoMpegStreamType	"TrackInfoKey_VideoMpegStreamType"	// Video MPEG ES stream type (WVUnsignedInt)
#define kTrackInfoKey_VideoMpegStreamId		"TrackInfoKey_VideoMpegStreamId"	// Video MPEG PES stream ID (WVUnsignedInt)
#define kTrackInfoKey_VideoMpegStreamPid	"TrackInfoKey_VideoMpegStreamPid"	// Video MPEG2-TS PID (WVUnsignedInt)
#define kTrackInfoKey_AVCDecoderConfigurationRecord	"TrackInfoKey_AVCDecoderConfigurationRecord"	// h.264 AVCDecoderConfigurationRecord (WVDataBlob)

// Audio track information
#define kTrackInfoKey_AudioFormat		 "TrackInfoKey_AudioFormat"		// Type of audio, valid values below (WVUnsignedInt)
#define kTrackInfoKey_AudioProfile    		"TrackInfoKey_AudioProfile"		// Audio profile indicator (WVUnsignedInt)
#define kTrackInfoKey_AudioNumChannels 		"TrackInfoKey_AudioNumChannels"		// Number of audio channels (WVUnsignedInt)
#define kTrackInfoKey_AudioSampleFrequency     	"TrackInfoKey_AudioSampleFrequency"    	// Audio sampling frequency, in hertz (WVUnsignedInt)
#define kTrackInfoKey_AudioSampleSize     	"TrackInfoKey_AudioSampleSize"    	// Audio sample size, in bits (WVUnsignedInt)
#define kTrackInfoKey_AudioBitRate		"TrackInfoKey_AudioBitRate"		// Audio bit rate (WVUnsignedInt)
#define kTrackInfoKey_AudioMpegStreamType	"TrackInfoKey_AudioMpegStreamType"	// Audio MPEG ES stream type (WVUnsignedInt)
#define kTrackInfoKey_AudioMpegStreamId		"TrackInfoKey_AudioMpegStreamId"	// Audio MPEG PES stream ID (WVUnsignedInt)
#define kTrackInfoKey_AudioMpegStreamPid	"TrackInfoKey_AudioMpegStreamPid"	// Audio MPEG2-TS PID (WVUnsignedInt)
#define kTrackInfoKey_AudioEsDescriptor		"TrackInfoKey_AudioEsDescriptor"	// Audio ES_Descriptor (WVDataBlob)
#define kTrackInfoKey_AudioEC3SpecificData     	"TrackInfoKey_AudioEC3SpecificData"	// E-AC3 audio contents of EC3SpecificBox (WVDataBlob)
#define kTrackInfoKey_AudioDtsSpecificData     	"TrackInfoKey_AudioEC3SpecificData"	// DTS/DCA audio contents of DTSSpecificBox (WVDataBlob)
#define kTrackInfoKey_AudioIdentifier		"TrackInfoKey_AudioIdentifier"		// Audio track indentifier/language code (WVString)

// Subtitle track information
#define kTrackInfoKey_SubtitleFormat		"TrackInfoKey_SubtitleFormat" 		// Type of subtitle, valid values below (WVUnsignedInt)
#define kTrackInfoKey_SubtitleIdentifier	"TrackInfoKey_SubtitleIdentifier"	// Subtitle track identifier/language code (WVString)

// Keys for chapter information
#define kChapterInfoKey_SeqNum			"ChapterInfoKey_SeqNum"			// Chapter sequence number (WVUnsignedInt, 0 start)
#define kChapterInfoKey_ChapterTimeIndex	"ChapterInfoKey_StartTimeIndex"		// Chapter start time index (WVUnsignedInt, milliseconds)
#define kChapterInfoKey_Title			"ChapterInfoKey_Title"			// Chapter title (WVString)
#define kChapterInfoKey_Thumbnail		"ChapterInfoKey_Thumbnail"		// Chapter thumbnail image (WVDataBlob, JPEG)

// Keys for copy control information
#define kCopyControlInfoKey    			"CopyControlInfoKey"    		// CCI Information (WVUnsignedInt)
#define kCopyControlInfoKey_EMI			"CopyControlInfoKey_EMI"		// EMI restrictions (WVUnsignedInt, below)
#define kCopyControlInfoKey_APS			"CopyControlInfoKey_APS"		// APS restrictions (WVUnsignedInt, below)
#define kCopyControlInfoKey_CIT			"CopyControlInfoKey_CIT"		// Constrain Image Trigger enabled (WVBoolean)
#define kCopyControlInfoKey_HDCP		"CopyControlInfoKey_HDCP"		// HDCP enabled (WVBoolean)
#define kCopyControlInfoKey_Heartbeats		"CopyControlInfoKey_Heartbeats"		// Heartbeats enabled (WVBoolean)

// Keys for DCP flags (please use enums and inlines in WVEmm.h to extract fields)
#define kDCPKey_DCPFlags                "DCPKey_DCPFlags"               // DCP Flags (WVUnsignedInt)

// Key to indicate whether to check if the license needs to be refreshed
#define kLicenseKey_CheckLicenseRefresh     "LicenseKey_CheckLicenseRefresh"               // License refresh flag (WVBoolean)

// Keys for stats query
#define kStatsKey_VideoStreamingBufferSize	"StatsKey_VideoStreamingBufferSize"	 // Size of adaptive video streaming buffer (WVUnsignedInt)
#define kStatsKey_VideoStreamingBufferUsed	"StatsKey_VideoStreamingBufferUsed"	 // Number of bytes in video streaming buffer (WVUnsignedInt)
#define kStatsKey_VideoTimeBuffered		"StatsKey_VideoTimeBuffered"		 // Duration of currently buffered video, in seconds (WVFloat)
#define kStatsKey_AudioStreamingBufferSize	"StatsKey_AudioStreamingBufferSize"	 // Size of separate audio streaming buffer (WVUnsignedInt)
#define kStatsKey_AudioStreamingBufferUsed	"StatsKey_AudioStreamingBufferUsed"	 // Number of bytes in separate audio streaming buffer (WVUnsignedInt)
#define kStatsKey_AudioTimeBuffered		 "StatsKey_AudioTimeBuffered"		 // Duration of currently buffered separate audio, in seconds (WVFloat)
#define kStatsKey_MeasuredBandwidth		"StatsKey_MeasuredBandwidth"		// Last measured bandwidth, in bits/sec (WVUnsignedInt)
#define kStatsKey_VideoBitRate			"StatsKey_VideoBitRate"			// Current video bit rate in bits/sec (WVUnsignedInt)
#define kStatsKey_AudioBitRate			"StatsKey_AudioBitRate"			// Current audio bit rate in bits/sec (WVUnsignedInt)
#define kStatsKey_AudioIdentifier		"StatsKey_AudioIdentifier"		// Identifier of current audio track (WVString)


// Constant values for kCredentialsKey_PlaybackMode
#define kPlaybackModeValue_Streaming 		WVUnsignedInt(1)	// Streaming playback
#define kPlaybackModeValue_Offline		WVUnsignedInt(2)	// Playback from local file
#define kPlaybackMode_OfflineAndStreaming 	WVUnsignedInt(3)	// Streaming and local file playback


// Constant values for kCopyControlInfoKey_EMI
#define kCopyControlInfoValue_EMI_NoRestricions 			WVUnsignedInt(0)
#define kCopyControlInfoValue_EMI_NoFurtherCopying 			WVUnsignedInt(1)
#define kCopyControlInfoValue_EMI_OneGenerationCopyingPermitted 	WVUnsignedInt(2)
#define kCopyControlInfoValue_EMI_NoCopy				WVUnsignedInt(3)


// Constant values for kCopyControlInfoKey_APS
#define kCopyControlInfoValue_APS_NoRestricions 			WVUnsignedInt(0)
#define kCopyControlInfoValue_APS_ACGOn 				WVUnsignedInt(1)
#define kCopyControlInfoValue_APS_ACGOn2 				WVUnsignedInt(2)
#define kCopyControlInfoValue_APS_ACGOn4 				WVUnsignedInt(3)


// Constant values for kTrackInfoKey_VideoFormat
#define kVideoFormatValue_H264			WVUnsignedInt(1)


// Constant values for kTrackInfoKey_AudioFormat
#define kAudioFormatValue_AAC			WVUnsignedInt(1)
#define kAudioFormatValue_EAC3			WVUnsignedInt(2)
#define kAudioFormatValue_DTS			WVUnsignedInt(3)

// Constant values for DTS audio profile
#define kDtsProfile_PRE_HD			WVUnsignedInt(0)		// Pre-HD format
#define kDtsProfile_HD_CBR			WVUnsignedInt(1)		// HD CBR
#define kDtsProfile_HD_Lossless			WVUnsignedInt(2)		// HD Lossless
#define kDtsProfile_LBR				WVUnsignedInt(3)		// Low bit-rate


// Constant values for kTrackInfoKey_SubtitleFormat
#define kSubtitleFormatValue_TTML		WVUnsignedInt(1)


#endif // __WVDICTIONARYKEYS_H__
