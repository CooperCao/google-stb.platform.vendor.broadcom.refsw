#
#  Broadcom USB RPC - Decoder support
#
#  Broadcom Proprietary and Confidential. Copyright (C) 2017,
#  All Rights Reserved.
#  
#  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
#  the contents of this file may not be disclosed to third parties, copied
#  or duplicated in any form, in whole or in part, without the prior
#  written permission of Broadcom.
#
#  $Id: BCMRPC_Common.inc,v 1.1 2008-11-18 01:13:16 $
#

set ModuleType  = "DataBlock Decoder";
set OutputType	= "BCM RPC Message";

#set ModuleType  = "Transaction Decoder";
#set InputType ??? what should this be ???
#  Looks like InputType should be an OutputType string if this
#  ModuleType is "Transaction Decoder" and operates on the output of some other decoder.

set UsbClassCode	= 0xFF; # Vendor Specific
#set UsbSubClass		= 0;
set UsbScriptDecodeType  = 2; # Combined decoding for Cells and Text decoding  

%include "BCMRPC_Call_IDs.inc"

# d11txh struct values
const	D11_TXH_LEN		= 0x68;		# byte length
const	txh_fid_bit_offset	= 38 * 16;	# bit offset

const	TXSTATUS_LEN		= 16;		# byte length
const	txs_fid_bit_offset	= 2 * 16;	# bit offset

ProcessTPBuffer()
{
	len = in.DataLength;
	GetNBits(in.Data, 0, 0);

	if (len == 0) {
		AddPLCellMonoNoDlg("NO DATA", "Payload is empy", "EMPTY", _WARNING);
		return Complete();
	}

	while (len >= 4) {
		rpc_len = NextDataDWord_();
		SetCellColor(RGB(0, 0, 128));

		AddPayLoadCell("RPC Msg", Format("%d bytes", rpc_len), 
		               "RPC Length\n" + 
		               "Length of the encapsulated RPC message not including this header.");

		RestoreCellColor();

                out.Decoded += 4;
                len -= 4;

		if (len < rpc_len) {
			# add an error cell
			ShowReDataBlockVS(rpc_len);
			len = 0;
		} else {
			#ShowReDataBlockVS(rpc_len);
			ProcessRPCBuffer(rpc_len);
	                len -= rpc_len;
		}
	
		if (len >= 4)
			AddDecSeparator();
	}

	if (len > 0) {
		AddPLCellMonoNoDlg("Error", "Illegal Payload",
			"Nothing is being decoded\nSee 'Data' for Raw Bytes", _ERROR);
		ShowDataBlockReOrder(in.DataLength - out.Decoded);

		out.Decoded += len;
        }

	return Complete();
}

const RPC_TYPE_DATA = 0;
const RPC_TYPE_RTN = 1;
const RPC_TYPE_MGN = 2;

set RPC_Message_Types =
[
	[ 0xFF, "Illegal"	],
	[ 0x00, "Data"		],
	[ 0x01, "Return"	],
	[ 0x02, "Mng"		]
];

ProcessRPCBuffer(rpc_len)
{
	if (rpc_len == 0) {
		# Filler token
	} else if (rpc_len < 4) {
		# Error
	} else {
		if (0) {
			TID = NextDataWord_();
			TID = (TID & 0xFF) << 8 + (TID >> 8);

			AddPayLoadCell("RPC Msg #", Format("0x%x", TID), 
				       "RPC Message Transaction Number", RGB(0, 0, 200));

			Session = NextNBits_(8);

			AddPayLoadCell("RPC Session", Format("%d", Session), "RPC Session Number");

			Type = NextNBits_(8);
			_type_row = Find(RPC_Message_Types, Type);
			Type_name = _type_row[1];

			AddPayLoadCell("RPC Msg Type", Type_name, 
				       "RPC Message Type\n" + 
				       "0: Data\n1: Return\n2: Management");
		} else {
			TID = NextNBits(8);
			TID += (NextNBits(8) << 8);
			Session = NextNBits(8);
			Type = NextNBits(8);

			# Update the field offsets to cover the 4 byte RPC header
			IncFieldOffset(4 * 8);

			_type_row = Find(RPC_Message_Types, Type);
			Type_name = _type_row[1];

			AddPayLoadCell("RPC Header",
			               Format("%s: ", Type_name) + Format("%d:", Session) + Format("%4X", TID), 
			               "RPC Message Header\nType: Session:Transaction#");
		}
		
                out.Decoded += 4;
		rpc_len -= 4;

		if (Type == RPC_TYPE_DATA) {
			ProcessRPCCall(rpc_len);
		} else {
			ShowReDataBlockVS(rpc_len);
		}
	}
}

ProcessRPCCall(len)
{
	if (len < 4) {
		#error
		return 0;
	}

	RPC_call_id = NextDataDWord_();
	RPC_call_name = _row_id[1];

	_row_id = Find(RPC_Function_IDs, RPC_call_id);
	AddPayLoadCell("Call", _row_id[1], "RPC Data Call ID");
		
	out.Decoded += 4;
	len -= 4;

	#Trace("hello \n");

	if (RPC_call_id == RPC_WLC_BMAC_TXFIFO_ID) {

		#
		# Decode the wlc_bmac_txfifo() parameters
		#

		fifo = NextDataDWord_();
		AddPayLoadCell("fifo", fifo, "wlc_bmac_txfifo() 'fifo' param");

		commit = NextDataDWord_();
		AddPayLoadCell("commit", commit, "wlc_bmac_txfifo() 'commit' param");

		fid = NextDataDWord_();
		if (fid == 0xFFFF) {
			fid = (0 - 1); # -1 as a literal does not work
			AddPayLoadCell("fid", fid, "wlc_bmac_txfifo() 'frameid' param");
		} else {
			AddPayLoadCell("fid", Format("%X", fid), "wlc_bmac_txfifo() 'frameid' param");
		}
		txpend = NextDataDWord_();
		AddPayLoadCell("txpend", txpend, "wlc_bmac_txfifo() 'txpend' param");

		pktflags = NextDataDWord_();
		AddPayLoadCell("pktflags", Format("%04X", pktflags), "wlc_bmac_txfifo() 'pktflags' param");

		pktlen = NextDataDWord_();
		AddPayLoadCell("pktlen", pktlen, "wlc_bmac_txfifo() len of 'p' param");

		# account for the 6 params above
		out.Decoded += 24;
		len -= 24;

		#
		# Decode the txfifo packet data
		#

		# packet starts with a d11txh struct
		# peek ahead to extract the TxFrameID, but have the cell cover the whole d11txh

		#save the current read pointer
		current_bit_offset = GetBitOffset();

		# read the txh.frameid without updating the Field offsets
		txframeid = GetNBits(in.Data, current_bit_offset + txh_fid_bit_offset, 8);
		txframeid += (NextNBits(8) << 8);

		# reset the read pointer to past the d11txh
		GetNBits(in.Data, current_bit_offset + (D11_TXH_LEN * 8), 0);

		# Update the field offsets to cover the d11hxh struct
		IncFieldOffset(D11_TXH_LEN * 8);

		AddPayLoadCell("d11txh",
		               Format("TxFrameID: %04X", txframeid), 
		               "d11txh");

		out.Decoded += D11_TXH_LEN;
		len -= D11_TXH_LEN;

		# display the wlc_bmac_txfifo() packet as a data block
		ShowReDataBlockVS(len);

	} else if (RPC_call_id == RPC_WLC_DOTXSTATUS_ID) {

		#save the current read pointer
		current_bit_offset = GetBitOffset();

		# read the txh.frameid without updating the Field offsets
		txstatusfid = GetNBits(in.Data, current_bit_offset + txs_fid_bit_offset, 8);
		txstatusfid += (NextNBits(8) << 8);

		Trace("txstatusfid = ", txstatusfid, "\n");

		# restore the read pointer
		GetNBits(in.Data, current_bit_offset, 0);
		
		# display the wlc_bmac_txfifo() packet as a data block
		BeginCellBlock("tx_status", Format("TxFrameID: %04X", txstatusfid));
		
		AddCell("tx_status fields", "", "tx_status data", FieldColor, _EXPANDED);
		
		EndCellBlock();

		ShowReDataBlockVS(len);
	} else {
		ShowReDataBlockVS(len);
	}
}
