$Id: README.txt,v 1.2 2010-05-28 22:05:29 $

Broadcom USB RPC Decoder Scripts

The BCMRPC directory has the USB CATC Scripting Language based custom decoders
to interpret BCM RPC traffic passed over the USB Bulk endpoints. The script
files were developed for the LeCroy UsbTracer v2.60.444 application.

Make use of the scripts, first copy the BCMRPC directory to the
USBTracer/Scripts/ directory. When USBTracer starts up it will search in the
Scripts/BCMRPC directory and find and catalog the decoder scripts. When you have
opened a USB Trace, use the View -> "Transfer Level" menu item show the Bulk
traffic collected into USB transfers. Then use the View -> "Apply Decoding
Scripts" menu item to associate the BCM RPC Message decoder with Bulk
traffic. The menu item will open a dialog window. Select the Endpoints tab, then
select the Bulk IN endpoint line, and pick BCM RPC Message from the Class/Vendor
Endpoint Decoding drop-down menu. Do the same for the Bulk OUT endpoint
line. After doing this, the transfers should be decoded with "RPC Msg" cells.

The RPC IDs are different from branch to branch, and change over time. The
BCMRPC_Call_IDs.inc file has the mapping of RPC IDs to displayed names and to
constants. Update the file as needed and check back into source control
associated with the appropriate code branch.

The script gen_call_ids.awk will generate an updated BCMRPC_Call_IDs.inc
from the RPC header wlc_rpc.h. Directions and description of the script
can be found in the script file.
