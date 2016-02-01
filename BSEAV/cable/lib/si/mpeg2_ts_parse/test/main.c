/***************************************************************************
 *     Copyright (c) 2003-2007, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <stdio.h>
#include <memory.h>

#include "bstd.h"
#include "ts_priv.h"
#include "ts_packet.h"

#include "ts_psi.h"
#include "ts_pat.h"
#include "ts_pmt.h"
#include "psip_mgt.h"
#include "psip_vct.h"
#include "psip_eit.h"
#include "psip_mss.h"
#include "psip_ett.h"
#include "psip_stt.h"
#include "psip_rrt.h"


#define MAX_MESSAGE_SIZE	4096
#define MAX_PIDS			64
#define MAX_STRING_SIZE		(8*1024)

struct pid_info_t
{
	bool		inUse;
	uint16_t	PID;
	uint8_t		contCount;
	bool		foundFirstPusi;
	uint8_t		messageBuf[MAX_MESSAGE_SIZE];
	int			messageSize;
};

static struct pid_info_t	gPidArray[MAX_PIDS];

static void printString( PSIP_MSS_string mss, char * pre, char * post )
{
	int j;
	static char string[MAX_STRING_SIZE];
	static int stringSize = MAX_STRING_SIZE;
	char *pLangCode;

	for(j=0; PSIP_MSS_getString(mss, j, &stringSize, string)==BERR_SUCCESS; stringSize = MAX_STRING_SIZE, j++ )
	{
		PSIP_MSS_getCode( mss, j, &pLangCode );
		if( stringSize != 0 )
		{
			printf("%s(%c%c%c): %s%s", pre, pLangCode[0], pLangCode[1], pLangCode[2], string, post);
		}
	}
}

void processDescriptor( TS_PSI_descriptor descriptor )
{
	switch( descriptor[0] )
	{
	case 2:
		printf("video_stream_descriptor\n");
		break;
	case 3:
		printf("audio_stream_descriptor\n");
		break;
	case 4:
		printf("hierarchy_descriptor\n");
		break;
	case 5:
		printf("registration_descriptor\n");
		break;
	case 6:
		printf("data_stream_alignment_descriptor\n");
		break;
	case 7:
		printf("target_background_grid_descriptor\n");
		break;
	case 8:
		printf("Video_window_descriptor\n");
		break;
	case 9:
		printf("CA_descriptor\n");
		break;
	case 10:
		printf("ISO_639_language_descriptor\n");
		break;
	case 11:
		printf("System_clock_descriptor\n");
		break;
	case 12:
		printf("Multiplex_buffer_utilization_descriptor\n");
		break;
	case 13:
		printf("Copyright_descriptor\n");
		break;
	case 14:
		printf("Maximum_bitrate_descriptor\n");
		break;
	case 15:
		printf("Private_data_indicator_descriptor\n");
		break;
	case 16:
		printf("Smoothing_buffer_descriptor\n");
		break;
	case 17:
		printf("STD_descriptor\n");
		break;
	case 18:
		printf("IBP_descriptor\n");
		break;
	case 27:
		printf("MPEG-4_video_descriptor\n");
		break;
	case 28:
		printf("MPEG-4_audio_descriptor\n");
		break;
	case 29:
		printf("IOD_descriptor\n");
		break;
	case 30:
		printf("SL_descriptor\n");
		break;
	case 31:
		printf("FMC_descriptor\n");
		break;
	case 32:
		printf("External_ES_ID_descriptor\n");
		break;
	case 33:
		printf("MuxCode_descriptor\n");
		break;
	case 34:
		printf("FmxBufferSize_descriptor\n");
		break;
	case 35:
		printf("MultiplexBuffer_descriptor\n");
		break;
	case 0x80:
		printf("PSIP stuffing descriptor\n");
		break;
	case 0x81:
		printf("PSIP AC-3 audio descriptor\n");
		break;
	case 0x86:
		printf("PSIP caption service descriptor\n");
		break;
	case 0x87:
		printf("PSIP content advisory descriptor\n");
		break;
	case 0xA0:
		printf("PSIP extended channel name descriptor\n");
		break;
	case 0xA1:
		printf("PSIP service location descriptor\n");
		break;
	case 0xA2:
		printf("PSIP time-shifted service descriptor\n");
		break;
	case 0xA3:
		printf("PSIP component name descriptor\n");
		break;
	case 0xA8:
		printf("PSIP DCC departing request descriptor\n");
		break;
	case 0xA9:
		printf("PSIP arriving request descriptor\n");
		break;
	case 0xAA:
		printf("PSIP redistribution control descriptor\n");
		break;
	default:
		printf("Unknown descriptor id %d\n", descriptor[0] );
		break;
	}
}

void processMessages( const uint8_t *buf, int messageSize )
{
	int i, j;
	TS_PSI_header header;

	TS_PSI_getSectionHeader( buf, &header );
	printf("id: 0x%02X, ssi: %d, pi: %d, len: %d, ext: %d, ver: %d, cni: %d, #: %d, l#: %d\n", header.table_id, header.section_syntax_indicator, header.private_indicator, header.section_length, header.table_id_extension, header.version_number, header.current_next_indicator, header.section_number, header.last_section_number );

	switch( buf[0] )
	{
	case 0x00:	/* PAT */
		{
			TS_PAT_program program;
			printf("\tPAT - %d Programs\n", TS_PAT_getNumPrograms(buf));
			for(i=0; TS_PAT_getProgram(buf,messageSize,i,&program)==BERR_SUCCESS; i++ )
			{
				printf("\tprogram_number: %d, PID: 0x%04X\n", program.program_number, program.PID);
			}
		}
		break;
	case 0x01:	/* CAT */
		printf("\tCAT\n");
		break;
	case 0x02:	/* PMT */
		{
			TS_PMT_stream stream;
			TS_PSI_descriptor descriptor;

			printf("\tPMT - PCR PID: 0x%04X - Num Streams: %d\n", 
				TS_PMT_getPcrPid(buf,messageSize), TS_PMT_getNumStreams(buf,messageSize) );
			for(j=0; (descriptor=TS_PMT_getDescriptor(buf,messageSize,j))!=NULL; j++ )
			{
				printf("\t\t");
				processDescriptor( descriptor );
			}
			
			for(i=0; TS_PMT_getStream(buf, messageSize, i, &stream)==BERR_SUCCESS; i++ )
			{
				printf("\tstream_type: 0x%02X, PID: 0x%04X\n", stream.stream_type, stream.elementary_PID);
				for(j=0; (descriptor=TS_PMT_getStreamDescriptor(buf,messageSize,i,j))!=NULL; j++ )
				{
					printf("\t\t");
					processDescriptor( descriptor );
				}
			}
		}
		break;
	case 0xC7:	/* MGT */
		{
			PSIP_MGT_table table;
			TS_PSI_descriptor descriptor;

			printf("\tMGT\n");
			for(j=0; (descriptor=PSIP_MGT_getAdditionalDescriptor(buf,j))!=NULL; j++ )
			{
				printf("\t\t");
				processDescriptor( descriptor );
			}
			for(i=0; PSIP_MGT_getTable(buf, i, &table)==BERR_SUCCESS; i++ )
			{

typedef struct
{
	const char * name;
	uint16_t	typeStart;
	uint16_t	typeEnd;
	uint16_t	subValue;
} MGT_Table_Types;			

const MGT_Table_Types mgtTypes[] = {

	{"VCT CNI=1", 0x0,    0x0,    0},
	{"VCT CNI=0", 0x1,    0x1,    1},
	{"VCT CNI=1", 0x2,    0x2,    2},
	{"VCT CNI=0", 0x3,	  0x3,    3},
	{"Chan ETT ", 0x4,	  0x4,    4},
	{"DCCSCT   ", 0x5,    0x5,    5},
	{"EIT      ", 0x100,  0x17F,  0x100},
	{"Event ETT", 0x200,  0x27F,  0x200},
	{"RRT      ", 0x301,  0x3FF,  0x300},
	{"DCCT     ", 0x1400, 0x14FF, 0x1400},
	{"Reserved ", 0,      0xFFFF, 0}
};

				for(j=0; !(table.table_type >= mgtTypes[j].typeStart && table.table_type <= mgtTypes[j].typeEnd); j++ ) {}

				printf("\t%s (%d), PID: 0x%04X, ver: %d, bytes: %ld\n", mgtTypes[j].name, table.table_type-mgtTypes[j].subValue, table.table_type_PID, table.table_type_version_number, table.number_bytes );
				for(j=0; (descriptor=PSIP_MGT_getTableDescriptor(buf,i,j))!=NULL; j++ )
				{
					printf("\t\t");
					processDescriptor( descriptor );
				}
			}
		}
		break;
	case 0xC8:	/* TVCT */
		/* Fallthrough */
	case 0xC9:	/* CVCT */
		{
			PSIP_VCT_channel channel;
			TS_PSI_descriptor descriptor;

			printf("\tTVCT / CVCT\n");
			for(j=0; (descriptor=PSIP_VCT_getAdditionalDescriptor(buf,j))!=NULL; j++ )
			{
				printf("\t\t");
				processDescriptor( descriptor );
			}
			for(i=0; PSIP_VCT_getChannel(buf, i, &channel)==BERR_SUCCESS; i++ )
			{
				wprintf(L"\t%.7s: %d.%d\n", channel.short_name, channel.major_channel_number, channel.minor_channel_number);
				for(j=0; (descriptor=PSIP_VCT_getChannelDescriptor(buf,i,j))!=NULL; j++ )
				{
					printf("\t\t");
					processDescriptor( descriptor );
				}
			}
		}
		break;
	case 0xCA:	/* RRT */
		{
			PSIP_RRT_header header;
			PSIP_RRT_dimension dimension;
			PSIP_RRT_value value;

			PSIP_RRT_getHeader( buf, &header );

			printf("\tRRT: %d dimensions\n",header.dimensions_defined);
			
			printString( header.p_rating_region_name_text, "\t", "\n" );
				
			for(i=0; PSIP_RRT_getDimension(buf, i, &dimension)==BERR_SUCCESS; i++ )
			{
				printf("\tDimension %d - graduated_scale: %d, values: %d\n", i, dimension.graduated_scale, dimension.values_defined);
				printString( dimension.p_dimension_name_text, "\t\t", "\n" );

				for(j=0; PSIP_RRT_getValue(buf, i, j, &value)==BERR_SUCCESS; j++ )
				{
					printString( value.p_abbrev_rating_value_text, "\t\t\t", ": " );
					printString( value.p_rating_value_text, "", "\n" );
				}
			}
		}
		break;
	case 0xCB:	/* EIT */
		{
			PSIP_EIT_event event;
			TS_PSI_descriptor descriptor;

			printf("\tEIT\n");
			for(i=0; PSIP_EIT_getEvent(buf, i, &event)==BERR_SUCCESS; i++ )
			{
				printf("\tevent_id: 0x%X, start_time: %u, ETM_loc: %d, sec: %d\n", event.event_id, event.start_time, event.ETM_location, event.length_in_seconds );
				printString( event.p_title_text, "\t\t", "\n" );
				for(j=0; (descriptor=PSIP_EIT_getEventDescriptor(buf,i,j))!=NULL; j++ )
				{
					printf("\t\t\t");
					processDescriptor( descriptor );
				}
			}
		}
		break;
	case 0xCC:	/* ETT */
		{
			PSIP_ETT_header header;

			printf("\tETT\n");
			PSIP_ETT_getHeader( buf, &header );

			printf("\ttype: %d, ETM_source_id: 0x%04X, ETM_event_id: 0x%04X\n", header.ETM_id.id_type, header.ETM_id.source_id, header.ETM_id.event_id );
			printString( header.p_extended_text_message, "\t\t", "\n" );
		}
		break;
	case 0xCD:	/* STT */
		{
			PSIP_STT_header header;
			TS_PSI_descriptor descriptor;

			printf("\tSTT\n");
			PSIP_STT_getHeader( buf, &header );

			printf("\ttime: %u, offset: %d, DS_status: %d, DS_dom: %d, DS_hour: %d\n", header.system_time, header.GPS_UTC_offset, header.daylight_savings.DS_status, header.daylight_savings.DS_day_of_month, header.daylight_savings.DS_hour );
			for(j=0; (descriptor=PSIP_STT_getDescriptor(buf,j))!=NULL; j++ )
			{
				printf("\t\t");
				processDescriptor( descriptor );
			}
		}
		break;
	case 0xD3:	/* DCCT */
		printf("\tDCCT\n");
		break;
	case 0xD4:	/* DCCSCT */
		printf("\tDCCSCT\n");
		break;
	default:
		printf("Unknown Table ID 0x%2X\n", buf[0]);
		break;
	}
}

void processPid( const TS_packet *p_packet )
{
	int i;

	for( i=0; i<MAX_PIDS; i++ )
	{
		if( !gPidArray[i].inUse )
			break;

		if( gPidArray[i].PID == p_packet->PID )
			break;
	}

	if( i == MAX_PIDS )
	{
		printf("Too many PIDS found.  Discarding PID 0x%04X\n", p_packet->PID );
		return;
	}

	if( !gPidArray[i].inUse )
	{
		printf("PID 0x%04X Detected.\n", p_packet->PID );
		gPidArray[i].inUse = true;
		gPidArray[i].PID = p_packet->PID;
		gPidArray[i].contCount = (uint8_t)(p_packet->continuity_counter-1);
	}

	if( p_packet->continuity_counter != ((gPidArray[i].contCount+1)&0xF) )
	{
		printf( "continuity_counter error detected on PID 0x%04X\n", p_packet->PID );
		gPidArray[i].messageSize = 0;
		gPidArray[i].foundFirstPusi = false;
		return;
	}

	gPidArray[i].contCount = p_packet->continuity_counter;

	if( !gPidArray[i].foundFirstPusi )
	{
		if( p_packet->payload_unit_start_indicator )
			gPidArray[i].foundFirstPusi = true;
		else
			return;
	}

	if( p_packet->payload_unit_start_indicator && ((TS_READ_32( p_packet->p_data_byte ) & 0xFFFFFF00) == 0x00000100) )
	{
		/* This packet contains PES data (non PSI), so ignore it... */
		gPidArray[i].foundFirstPusi = false;
		return;
	}

	if( gPidArray[i].messageSize != 0 )
	{
		/* Continue copying message(s) */
		if( p_packet->payload_unit_start_indicator )
		{
			/* only copy data until we hit start of next message */
			memcpy( &gPidArray[i].messageBuf[gPidArray[i].messageSize], &p_packet->p_data_byte[1], p_packet->p_data_byte[0] );
			gPidArray[i].messageSize += p_packet->p_data_byte[0];
		}
		else
		{
			memcpy( &gPidArray[i].messageBuf[gPidArray[i].messageSize], p_packet->p_data_byte, p_packet->data_size );
			gPidArray[i].messageSize += p_packet->data_size;
		}
	}

	if( p_packet->payload_unit_start_indicator )
	{
		/* We are starting a new message */
		if( gPidArray[i].messageSize != 0 )
		{
			processMessages( gPidArray[i].messageBuf, gPidArray[i].messageSize );
		}

		/* Copy our new message(s) */
		memcpy( gPidArray[i].messageBuf, &p_packet->p_data_byte[1+p_packet->p_data_byte[0]], p_packet->data_size-(1+p_packet->p_data_byte[0]) );
		gPidArray[i].messageSize = p_packet->data_size-(1+p_packet->p_data_byte[0]);

		/* Check if we have a complete message */
		if( (TS_PSI_GET_SECTION_LENGTH(gPidArray[i].messageBuf)+3) <= gPidArray[i].messageSize )
		{
			processMessages( gPidArray[i].messageBuf, gPidArray[i].messageSize );
			gPidArray[i].messageSize = 0;
			gPidArray[i].foundFirstPusi = false;
		}
	}
}

#include "psip_mss.h"
/* These are compressed buffers passed to the function
 *
 * void PSIP_MSS_P_decompressString( const uint8_t *p_decodeTree, 
 *         const uint8_t *p_compressedString, uint8_t numberBytes, int *p_stringSize, char *p_string )
 *
 * p_decodeTree = PSIP_decode_tree_02
 */
unsigned char g_compressed_data_0[39] = 
{
	0xd1, 	0x5b, 	0x9d, 	0x36, 	0x55, 	0xa3, 	0x3d, 	0xfb, 	0x2c, 	0x71, 	0xeb, 	0xa9, 	0x89, 	0xaf, 	0x2f, 	0x7b, 
	0xfb, 	0xe2, 	0x5f, 	0xf5, 	0x9b, 	0xb8, 	0x0e, 	0xeb, 	0xff, 	0xdd, 	0xf5, 	0x55, 	0xb4, 	0xfa, 	0x94, 	0x39, 
	0x5d, 	0x8a, 	0xdd, 	0xdc, 	0xb5, 	0xbf, 	0x24, 
};

unsigned char g_compressed_data_1[34] =
{
	0x5d, 	0xc9, 	0x88, 	0xea, 	0x25, 	0xb3, 	0xa1, 	0xcc, 	0xb9, 	0x34, 	0x59, 	0x7a, 	0xa2, 	0x4b, 	0xff, 	0xa7, 
	0xc0, 	0x61, 	0x51, 	0x10, 	0xb4, 	0xff, 	0x4e, 	0x2f, 	0xff, 	0x52, 	0x0d, 	0x43, 	0x0c, 	0x88, 	0x2f, 	0xb9, 
	0x66, 	0x20,   
};

unsigned char g_compressed_data_2[29] = 
{
	0x5d, 	0xfd, 	0x22, 	0x5f, 	0xfe, 	0x4b, 	0xaa, 	0x2e, 	0x17, 	0x34, 	0x7f, 	0xad, 	0xef, 	0xbc, 	0x43, 	0x64, 
	0xea, 	0xbd, 	0x69, 	0xd4, 	0xe7, 	0x10, 	0xf5, 	0x8b, 	0xea, 	0x15, 	0x1a, 	0xb1, 	0x00, 
};

unsigned char g_compressed_data_3[143] = 
{
	0x4c, 	0xd7, 	0xc4, 	0xfa, 	0x1c, 	0x41, 	0x4c, 	0x77, 	0xee, 	0xff, 	0xf7, 	0x73, 	0x72, 	0xe6, 	0x65, 	0xeb, 
	0xbd, 	0xde, 	0xb5, 	0xcb, 	0x3e, 	0x9d, 	0x77, 	0xbd, 	0x31, 	0x68, 	0x7f, 	0xb7, 	0x75, 	0x2a, 	0xb1, 	0xd5, 
	0x2a, 	0xda, 	0x7d, 	0x42, 	0x57, 	0xb2, 	0x66, 	0xe7, 	0x17, 	0x33, 	0xfc, 	0xb1, 	0xae, 	0x68, 	0xc5, 	0xcf, 
	0x53, 	0xb6, 	0x33, 	0xe6, 	0x73, 	0x6f, 	0x0c, 	0x5b, 	0x9c, 	0xcb, 	0xd7, 	0x7c, 	0xed, 	0x11, 	0xb6, 	0xfe, 
	0xef, 	0xcd, 	0xdf, 	0x30, 	0xe2, 	0x42, 	0xd3, 	0x70, 	0x56, 	0xc2, 	0xd1, 	0x17, 	0x6d, 	0x39, 	0x0e, 	0xd1, 
	0x54, 	0x7f, 	0x26, 	0xdf, 	0xdd, 	0xfb, 	0xfc, 	0xb0, 	0x8b, 	0x4f, 	0xfb, 	0x26, 	0x6e, 	0x7e, 	0x90, 	0xdb, 
	0xa7, 	0x92, 	0x0a, 	0xd7, 	0xa8, 	0x9c, 	0xd7, 	0xea, 	0xe9, 	0xf1, 	0xe3, 	0x1d, 	0xca, 	0xb2, 	0xef, 	0xf5, 
	0x1f, 	0x73, 	0xfd, 	0xb0, 	0xb3, 	0xd2, 	0x86, 	0x29, 	0x89, 	0xd0, 	0x33, 	0xd0, 	0xdc, 	0x85, 	0xa7, 	0xec, 
	0xe8, 	0x11, 	0xfe, 	0x2d, 	0x46, 	0x9f, 	0xa5, 	0xd0, 	0x4d, 	0xde, 	0x9f, 	0x34, 	0x02, 	0x31, 	0x00, 
};

extern void PSIP_MSS_P_decompressString( const uint8_t *p_decodeTree, const uint8_t *p_compressedString, uint8_t numberBytes, int *p_stringSize, char *p_string );
extern const uint8_t PSIP_decode_tree_01[];
extern const uint8_t PSIP_decode_tree_02[];

int test_huffman(void)
{
	#define TEST_STRING_SIZE (8*1024)
	uint8_t *str = malloc(TEST_STRING_SIZE);
	int size;
	
	size = TEST_STRING_SIZE;
	PSIP_MSS_P_decompressString(PSIP_decode_tree_02, g_compressed_data_0, 39, &size, str);
	if( size != 91 )
		return 10;
	printf("%d: %s\n", size, str);

	size = TEST_STRING_SIZE;
	PSIP_MSS_P_decompressString(PSIP_decode_tree_02, g_compressed_data_1, 34, &size, str);
	if( size != 68 )
		return 11;
	printf("%d: %s\n", size, str);

	size = TEST_STRING_SIZE;
	PSIP_MSS_P_decompressString(PSIP_decode_tree_02, g_compressed_data_2, 29, &size, str);
	if( size != 58 )
		return 12;
	printf("%d: %s\n", size, str);

	size = TEST_STRING_SIZE;
	PSIP_MSS_P_decompressString(PSIP_decode_tree_02, g_compressed_data_3, 143, &size, str);
	if( size != 303 )
		return 13;
	printf("%d: %s\n", size, str);
	
	return 0;
}
unsigned char g_message_data_0[75] = {

	0xcb, // EIT 	
  0xf0, 0x4b, // Length = 0x4B 
  0x00, 0x07, // Source 	
  0xdf, // ? 2b0, 5bVersion, 1b1	
  0x00, // Section # 	
  0x00, // Last section 	
  0x00, // Protocol 	
  0x02, // Num events 	
  // Event 1
  0xc3, 0xda, 	// 2b1 + Event ID 	
  0x33, 	0x0e, 0xa7, 0x8d, // Start time 	
  0xc0, 	0x70, 0x80, // ? 2b1, 2bETM, 20bSeconds
  0x0c, 	// Title len (12)
  // compr text
  0x01, 	0x65, 	0x6e, 	0x67, 	0x01, 	0x01, 	0x00, 	0x04, 	0x2a, 	0xf0, 	0x60, 	0xba,
	0xf0, 	0x08, // descr len (8) 	
  0x81, 	0x06, 	0x08, 	0xbc, 	0x1b, 	0x09, 	0x1f, 	0x00, 	
  // Event 2
  0xc3, 	0xdd, // Event id 	
  0x33, 	0x0f, 	0x18, 	0x0d, // start time 
	0xc0, 	0x70, 	0x80, // ? 2b1, 2bETM, 20bSeconds
  0x0c, 	// Title len (12)
  // compr text
  0x01, 	0x65, 	0x6e, 	0x67, 	0x01, 	0x01, 	0x00, 	0x04, 	0x2a, 	0xf0, 	0x60, 	0xba, 
	0xf0, 	0x08, // desc len (8) 	
  0x81, 	0x06, 	0x08, 	0xbc, 	0x1b, 	0x09, 	0x1f, 	0x00, 	
  0xbe, // missing CRC bytes? 
  };
unsigned char g_message_data_1[75] = {

	0xcb, 	0xf0, 	0x4b, 	0x00, 	0x07, 	0xdf, 	0x00, 	0x00, 	0x00, 	0x02, 	0xc3, 	0xda, 	0x33, 	0x0e, 	0xa7, 	0x8d, 
	0xc0, 	0x70, 	0x80, 	0x0c, 	0x01, 	0x65, 	0x6e, 	0x67, 	0x01, 	0x01, 	0x00, 	0x04, 	0x2a, 	0xf0, 	0x60, 	0xba, 
	0xf0, 	0x08, 	0x81, 	0x06, 	0x08, 	0xbc, 	0x1b, 	0x09, 	0x1f, 	0x00, 	0xc3, 	0xdd, 	0x33, 	0x0f, 	0x18, 	0x0d, 
	0xc0, 	0x70, 	0x80, 	0x0c, 	0x01, 	0x65, 	0x6e, 	0x67, 	0x01, 	0x01, 	0x00, 	0x04, 	0x2a, 	0xf0, 	0x60, 	0xba, 
	0xf0, 	0x08, 	0x81, 	0x06, 	0x08, 	0xbc, 	0x1b, 	0x09, 	0x1f, 	0x00, 	0xbe, };
unsigned char g_message_data_2[165] = {

	0xcc, // ETT 	
  0xf0,  0xa5, // len 	
  0x00, 	0x00, // 0x00 	
  0xc7, // 2b1, 5bVer, 1b1 	
  0x00, // sec num 	
  0x00, // last sec num 	
  0x00, // protocol ver 	
  0x00, 	0x16, 	0x65, 	0xe6, // etm id 
  // Text start...
  0x01, // num strings 	
  0x65, 	0x6e, 0x67, // iso lang code
  0x01, // num segs 	
  0x02, // compr type 	
  0x00, // mode 	
  0x8f, // num bytes 	
  0x4c, 	0xd7, 	0xc4, 	0xfa, 	0x1c, 	0x41, 	0x4c, 	0x77, 	0xee, 	0xff, 	0xf7, 
	0x73, 	0x72, 	0xe6, 	0x65, 	0xeb, 	0xbd, 	0xde, 	0xb5, 	0xcb, 	0x3e, 	0x9d, 	0x77, 	0xbd, 	0x31, 	0x68, 	0x7f, 
	0xb7, 	0x75, 	0x2a, 	0xb1, 	0xd5, 	0x2a, 	0xda, 	0x7d, 	0x42, 	0x57, 	0xb2, 	0x66, 	0xe7, 	0x17, 	0x33, 	0xfc, 
	0xb1, 	0xae, 	0x68, 	0xc5, 	0xcf, 	0x53, 	0xb6, 	0x33, 	0xe6, 	0x73, 	0x6f, 	0x0c, 	0x5b, 	0x9c, 	0xcb, 	0xd7, 
	0x7c, 	0xed, 	0x11, 	0xb6, 	0xfe, 	0xef, 	0xcd, 	0xdf, 	0x30, 	0xe2, 	0x42, 	0xd3, 	0x70, 	0x56, 	0xc2, 	0xd1, 
	0x17, 	0x6d, 	0x39, 	0x0e, 	0xd1, 	0x54, 	0x7f, 	0x26, 	0xdf, 	0xdd, 	0xfb, 	0xfc, 	0xb0, 	0x8b, 	0x4f, 	0xfb, 
	0x26, 	0x6e, 	0x7e, 	0x90, 	0xdb, 	0xa7, 	0x92, 	0x0a, 	0xd7, 	0xa8, 	0x9c, 	0xd7, 	0xea, 	0xe9, 	0xf1, 	0xe3, 
	0x1d, 	0xca, 	0xb2, 	0xef, 	0xf5, 	0x1f, 	0x73, 	0xfd, 	0xb0, 	0xb3, 	0xd2, 	0x86, 	0x29, 	0x89, 	0xd0, 	0x33, 
	0xd0, 	0xdc, 	0x85, 	0xa7, 	0xec, 	0xe8, 	0x11, 	0xfe, 	0x2d, 	0x46, 	0x9f, 	0xa5, 	0xd0, 	0x4d, 	0xde, 	0x9f, 
	0x34, 	
  0x02, 	0x31, 	0x00, 	0x90, // CRC
  };
unsigned char g_message_data_3[43] = {

	0xcb, // EIT 	
  0xf0, 	0x2b, // len 	
  0x00, 	0x07, // source id 	
  0xcb, // ver 	
  0x00, // sec num 
  0x00, // last sec	
  0x00, // protocol	
  0x01, // num events	
  0xc3, 	0xdd, // event id 	
  0x33, 	0x0f, 	0x18, 	0x0d, // start time 
	0xc0, 	0x70, 	0x80, 	0x0c, 	0x01, 	0x65, 	0x6e, 	0x67, 	0x01, 	0x01, 	0x00, 	0x04, 	0x2a, 	0xf0, 	0x60, 	0xba, 
	0xf0, 	0x08, 	0x81, 	0x06, 	0x08, 	0xbc, 	0x1b, 	0x09, 	0x1f, 	0x00, 	0x83, };
unsigned char g_message_data_4[104] = {

	0xcc, 	0xf0, 	0x68, 	0x00, 	0x00, 	0xcb, 	0x00, 	0x00, 	0x00, 	0x00, 	0x15, 	0x54, 	0x0a, 	0x01, 	0x65, 	0x6e, 
	0x67, 	0x01, 	0x02, 	0x00, 	0x52, 	0x67, 	0x31, 	0x77, 	0xd4, 	0xc3, 	0x67, 	0x7b, 	0xd3, 	0xed, 	0xcb, 	0xc0, 
	0x14, 	0x33, 	0x19, 	0xbf, 	0x25, 	0xc2, 	0x44, 	0x80, 	0xc9, 	0x7d, 	0x0f, 	0x3e, 	0x58, 	0x7c, 	0x06, 	0x72, 
	0x09, 	0x70, 	0xa3, 	0x0a, 	0xbe, 	0xf2, 	0x0e, 	0x98, 	0xde, 	0x51, 	0xe9, 	0xa1, 	0x62, 	0xf3, 	0x8a, 	0x12, 
	0x8e, 	0xdd, 	0x38, 	0x75, 	0xf6, 	0xf6, 	0x69, 	0x8f, 	0xfc, 	0x07, 	0x5e, 	0x3f, 	0xed, 	0x75, 	0x3d, 	0xa5, 
	0x21, 	0xec, 	0x33, 	0xaa, 	0xc3, 	0x28, 	0x89, 	0xfe, 	0x0e, 	0x1c, 	0xa2, 	0xfb, 	0x13, 	0x6a, 	0x8e, 	0xa6, 
	0xbf, 	0xa9, 	0x15, 	0x99, 	0xde, 	0xeb, 	0x00, 	0xaf, };
unsigned char g_message_data_5[75] = {

	0xcb, 	0xf0, 	0x4b, 	0x00, 	0x07, 	0xcb, 	0x00, 	0x00, 	0x00, 	0x02, 	0xc3, 	0xe0, 	0x33, 	0x0f, 	0x88, 	0x8d, 
	0xc0, 	0x70, 	0x80, 	0x0c, 	0x01, 	0x65, 	0x6e, 	0x67, 	0x01, 	0x01, 	0x00, 	0x04, 	0x2a, 	0xf0, 	0x60, 	0xba, 
	0xf0, 	0x08, 	0x81, 	0x06, 	0x08, 	0xbc, 	0x1b, 	0x09, 	0x1f, 	0x00, 	0xc3, 	0xe3, 	0x33, 	0x0f, 	0xf9, 	0x0d, 
	0xc0, 	0x70, 	0x80, 	0x0c, 	0x01, 	0x65, 	0x6e, 	0x67, 	0x01, 	0x01, 	0x00, 	0x04, 	0x2a, 	0xf0, 	0x60, 	0xba, 
	0xf0, 	0x08, 	0x81, 	0x06, 	0x08, 	0xbc, 	0x1b, 	0x09, 	0x1f, 	0x00, 	0x92, };
unsigned char g_message_data_6[65] = {

	0xcc, 	0xf0, 	0x41, 	0x00, 	0x00, 	0xcb, 	0x00, 	0x00, 	0x00, 	0x00, 	0x16, 	0x66, 	0x7a, 	0x01, 	0x65, 	0x6e, 
	0x67, 	0x01, 	0x02, 	0x00, 	0x2b, 	0x9f, 	0xda, 	0x2f, 	0x92, 	0x40, 	0xc6, 	0xe8, 	0xdf, 	0xe5, 	0x88, 	0x17, 
	0xb3, 	0xa8, 	0x87, 	0xde, 	0x8f, 	0xf8, 	0xc5, 	0x77, 	0x6e, 	0xfc, 	0x9c, 	0x23, 	0xef, 	0xb9, 	0x5c, 	0xc3, 
	0x75, 	0x4e, 	0x0d, 	0xb7, 	0x7b, 	0x8f, 	0x2c, 	0x29, 	0xce, 	0x21, 	0xeb, 	0x17, 	0xd4, 	0x2a, 	0x35, 	0x62, 
	0xd6, };

unsigned char g_message_data_7[] = {
/* ###  chan_mgr: ETT: 0x1d01(0) <seg1/8><seg3/8><seg5/8><seg7/8> */
0xcc, 0xf0, 0x6b, 0x00, 0x00, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x0e, 
0x01, 0x65, 0x6e, 0x67, 0x08, 
0x00, 0x00, 0x08, 0x3c, 0x73, 0x65, 0x67, 0x31, 0x2f, 0x38, 0x3e, 
0x00, 0x01, 0x08, 0x3c, 0x73, 0x65, 0x67, 0x32, 0x2f, 0x38, 0x3e, 
0x00, 0x00, 0x08, 0x3c, 0x73, 0x65, 0x67, 0x33, 0x2f, 0x38, 0x3e, 
0x00, 0x01, 0x08, 0x3c, 0x73, 0x65, 0x67, 0x34, 0x2f, 0x38, 0x3e, 
0x00, 0x00, 0x08, 0x3c, 0x73, 0x65, 0x67, 0x35, 0x2f, 0x38, 0x3e, 
0x00, 0x01, 0x08, 0x3c, 0x73, 0x65, 0x67, 0x36, 0x2f, 0x38, 0x3e, 
0x00, 0x00, 0x08, 0x3c, 0x73, 0x65, 0x67, 0x37, 0x2f, 0x38, 0x3e, 
0x00, 0x01, 0x08, 0x3c, 0x73, 0x65, 0x67, 0x38, 0x2f, 0x38, 0x3e, 
0x83, 
};

unsigned char g_message_data_8[] = {
/* ###  chan_mgr: ETT: 0x1d01(0) */ 
0xcc, 0xf0, 0x26, 0x00, 0x00, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x22, 0x00, 0x12, 
0x01, 0x65, 0x6e, 0x67, 0x01, 
0x01, 0x00, 0x10, 0xcb, 0x68, 0xe7, 0x3f, 0x7e, 0x5b, 0x6e, 0xc8, 0x5c, 0x81, 0x31, 0x11, 0xbe, 0x26, 0x9c, 0x00, 
0x72, 
};

int test_message(void)
{
	processMessages( g_message_data_0, sizeof(g_message_data_0) );
	processMessages( g_message_data_1, sizeof(g_message_data_1) );
	processMessages( g_message_data_2, sizeof(g_message_data_2) );
	processMessages( g_message_data_3, sizeof(g_message_data_3) );
	processMessages( g_message_data_4, sizeof(g_message_data_4) );
	processMessages( g_message_data_5, sizeof(g_message_data_5) );
	processMessages( g_message_data_6, sizeof(g_message_data_6) );
	processMessages( g_message_data_7, sizeof(g_message_data_7) );
	processMessages( g_message_data_8, sizeof(g_message_data_8) );
	
	return 0;
}


int main(int argc, char* argv[])
{
	static uint8_t buf[188];
	FILE	*source;

	/* Stream Statistics */
	int		nullPacketCount = 0;
	int		packetCount = 0;

	/* Packet structures */
	TS_packet ts_packet;

	if( argc != 2 )
	{
		printf("USAGE:\n");
		printf("psip [sourceFile|HUFFMAN|MESSAGE]\n\n");
		return 1;
	}
	
	if( strcmp(argv[1], "HUFFMAN") == 0)
	{
		return test_huffman();
	}

	if( strcmp(argv[1], "MESSAGE") == 0)
	{
		return test_message();
	}

	if( (source=fopen(argv[1], "rb"))==NULL )
	{
		printf("Unable to open source file: %s", argv[1]);
		return 2;
	}

	while( fread( buf, 1, 188, source ) )
	{
		packetCount++;
		TS_parseTsPacket( buf, &ts_packet );

		if( ts_packet.PID == 0x1FFF )
		{
			nullPacketCount++;
		}
		else
		{
			processPid( &ts_packet );
		}
	}

	fclose(source);
	printf("Packet Count = %d\n", packetCount);
	printf("Null Packet Count = %d\n", nullPacketCount);

	return 0;
}
