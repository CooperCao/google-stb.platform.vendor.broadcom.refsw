/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
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
#ifndef TS_PACKET_H__
#define TS_PACKET_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	bool		transport_error_indicator;
	bool		payload_unit_start_indicator;
	bool		transport_priority;
	uint16_t	PID;
	uint8_t		transport_scambling_control;
	uint8_t		adaptation_field_control;
	uint8_t		continuity_counter;
	struct
	{
		bool		discontinuity_indicator;
		bool		random_access_indicator;
		bool		elementary_stream_priority_indicator;
		bool		PCR_flag;
			uint64_t	program_clock_reference_base;	
			uint16_t	program_clock_reference_extension;
		bool		OPCR_flag;
			uint64_t	original_program_clock_reference_base;
			uint16_t	original_program_clock_reference_extension;
		bool		splicing_point_flag;
			uint8_t		splice_countdown;
		bool		transport_private_data_flag;
			uint8_t		transport_private_data_length;
			const uint8_t *p_private_data_byte;
		bool		adaptation_field_extension_flag;
			bool		ltw_flag;
				bool		ltw_valid_flag;
				uint16_t	ltw_offset;
			bool		piecewise_rate_flag;
				uint32_t	piecewise_rate;
			bool		seamless_splice_flag;
				uint8_t		splice_type;
				uint64_t	DTS_next_AU;
	} adaptation_field;
	const uint8_t *p_data_byte;
	uint8_t		data_size;
} TS_packet;

void TS_parseTsPacket( const uint8_t *buf, TS_packet *p_packet );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
