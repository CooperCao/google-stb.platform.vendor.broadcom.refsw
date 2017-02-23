/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/

%module PerlExtLib
%{
#include "PerlExtLib.h"
%}

/***********************************************************************func*
 * bcmUseI2CParallel
 *
 * INPUT: pchDeviceName - the name of the device on the other side of the
 *                        I2C bus. (i.e. "7115")
 *        usPortAddr    - The port address of the parallel port the I2C
 *                        card is on. (0x378, 0x278, 0x3BC or 0x2BC)
 *        usSlaveAddr   - The slave address of the I2C slave device.
 *        ulUseCOM      - Non-zero if the BBS COM interface to the I2C card
 *                        is to be used.
 *
 * FUNCTION: Any subsequent calls to this .dll will be re-directed to the
 *           I2C parallel port card. This must be called before any other
 *           call to the .dll for proper functionality. After a call to
 *           bcmCloseDevice, this function must be called again to use I2C.
 * RETURN: NULL if failed to open the device, non-NULL to indicate success.
 */
const char * bcmUseI2CParallel(
	const char * pchDeviceName,
	const unsigned short usPortAddr,
	const unsigned short usSlaveAddr,
	const unsigned char uchSpeed,
	const unsigned long ulUseCOM);


/***********************************************************************func*
 * bcmUseSocket
 *
 * INPUT: pchServerName - The TCP/IP name of the Server that will
 *                        translate into an IP.
 *        usPortNum     - The port number that the server is listening.
 *
 * FUNCTION: Any subsequent calls to this .dll will be re-directed to the
 *           TCP/IP Socket. This must be called before any other
 *           call to the .dll for proper functionality. After a call to
 *           bcmCloseDevice, this function must be called again to use I2C.
 * RETURN: NULL if failed to open the device, non-NULL to indicate success.
 */
const char * bcmUseSocket(
	const char * pchServerName,
	const unsigned short usPortNum);


/***********************************************************************func*
 * bcmOpenDevice
 *
 * INPUT: none
 * FUNCTION: Open the device.
 * RETURN: NULL if failed to open the device, non-NULL to indicate success.
 */
const char* bcmOpenDevice(
	void);


/***********************************************************************func*
 * bcmCloseDevice
 *
 * INPUT: none
 * FUNCTION: Close Device
 * RETURN: NULL if failed to close the device, non-NULL to indicate success.
 */
const char* bcmCloseDevice(
	void);


/***********************************************************************func*
 * bcmRead
 *
 * INPUT: ulAddr - an offset address of a register
 * FUNCTION: Single register read
 * RETURN:  NULL if failed to read the device.  Otherwise return the
 *   value string in decimal.
 */
const char* bcmRead(
	const unsigned long ulAddr);


/***********************************************************************func*
 * bcmWrite
 *
 * INPUT: ulAddr - an offset address of a register
 *        ulData - a value to write to a register
 * FUNCTION: Single register write
 * RETURN:  NULL if failed to write the device.  Otherwise return the
 *   value string in decimal.
 */
const char* bcmWrite(
	const unsigned long ulAddr,
	const unsigned long ulData);


/************************************************************************func
 * Name:
 *   bcmRead64() - read a 64-bit register
 *
 * Input:
 *   a valid register address.
 *
 * Return:
 *   a register's dword if success, or NULL (undefined in perl).
 *
 * Description:
 *   read a register content of the given address.
 ****************************************************************************/
const char* bcmRead64
    ( const unsigned long ulAddr );


/************************************************************************func
 * Name:
 *   bcmWrite64() - write to a 64-bit register
 *
 * Input:
 *   a valid register address, data.
 *
 * Return:
 *   true for success, and false (undefined in perl) for failed.
 *
 * Description:
 *   read a register content of the given address.
 ****************************************************************************/
const char* bcmWrite64
    ( const unsigned long      ulAddr,
      const unsigned long long ulData );

/***********************************************************************func*
 * bcmReadMem
 *
 * INPUT: ulAddr - a offset address of memory
 * FUNCTION: Single memory read
 * RETURN:  NULL if failed to read the device.  Otherwise return the
 *   32-bit value string in decimal.
 ****************************************************************************/
const char* bcmReadMem
    ( const unsigned long long ulAddr );

/***********************************************************************func*
 * bcmWriteMem
 *
 * INPUT: ulAddr  - a offset address of memory
 *        ulData  - a 32-bit value to write to memroy
 *        ulCount - how many to fill
 * FUNCTION: Multiple memory write
 * RETURN:  NULL if failed to write the device.  Otherwise return the
 *   ulCount wrote.
 ****************************************************************************/
const char* bcmWriteMem
    ( const unsigned long long ulAddr,
      unsigned long            ulData,
      const unsigned long      ulCount );

/************************************************************************func
 * Name:
 *   bcmReadMemBlkToFile() - read block of SDRAM and store in a file.
 *
 * Input:
 *   ulAddr - a valid SDRAM address
 *   ulCount - number of dword to read.
 *   pchMemfile - file containing the dwords to write
 *
 * Return:
 *   1 for success, and NULL for failed.
 *
 * Description:
 *   result are in pulData.  It returns number of SDRAM dwords read.
 ****************************************************************************/
const char* bcmReadMemBlkToFile
    ( const unsigned long long  ulAddr,
      const unsigned long       ulCount,
      const char               *pchMemfile );

/************************************************************************func
 * Name:
 *   bcmWriteMemBlkFromFile() - write to a block of SDRAM with values from a file
 *
 * Input:
 *   ulAddr - a valid SDRAM address
 *   pchMemfile - file containing the dwords to write
 *
 * Return:
 *   true for number of bytes wrote, or NULL for failure.
 *
 * Description:
 *   write the content of file to sdram (raw data).
 ****************************************************************************/
const char* bcmWriteMemBlkFromFile
    ( const unsigned long long  ulAddr,
      const char               *pchMemfile);

/************************************************************************func
 * Name:
 *   bcmReadMemBlkToFileFormatted() - read block of SDRAM and store in a file in
 *                                    memview format.
 *
 * Input:
 *   ulAddr - a valid SDRAM address
 *   ulCount - number of dword to read.
 *   pchMemfile - file containing the dwords to write
 *
 * Return:
 *   1 for success, and NULL for failed.
 *
 * Description:
 *   result are in pulData.  It returns number of SDRAM dwords read.
 ****************************************************************************/
const char* bcmReadMemBlkToFileFormatted
    ( const unsigned long long  ulAddr,
      const unsigned long       ulCount,
      const char               *pchMemfile );

/************************************************************************func
 * Name:
 *   bcmWriteMemBlkFromFileFormatted() - write to a block of SDRAM with values
 *                                       from a file in memview format
 *
 * Input:
 *   ulAddr - a valid SDRAM address
 *   pchMemfile - file containing the dwords to write
 *
 * Return:
 *   true for number of bytes wrote, or NULL for failure.
 *
 * Description:
 *   write the content of file to sdram (raw data).
 ****************************************************************************/
const char* bcmWriteMemBlkFromFileFormatted
    ( const unsigned long long  ulAddr,
      const char               *pchMemfile);

/***********************************************************************func*
 * bcmReadI2C
 *
 * INPUT: uchChipAddr - i2c device address on i2c bus
 *        uchSubAddr  - i2c device's subaddress
 * FUNCTION: i2c device read
 * RETURN:  NULL if failed to read the i2c device.
 */
const char* bcmReadI2C(
	const unsigned char uchChipAddr,
	const unsigned char uchSubAddr);


/***********************************************************************func*
 * bcmWriteI2C
 *
 * INPUT: uchChipAddr - i2c device address on i2c bus
 *        uchSubAddr  - i2c device's subaddress
 *        uchData     - byte to be written to i2c device
 * FUNCTION: i2c device write
 * RETURN:  NULL if failed to write the i2c device.
 */
const char* bcmWriteI2C(
	const unsigned char uchChipAddr,
	const unsigned char uchSubAddr,
	const unsigned char uchData);


/***********************************************************************func*
 * bcmRunTCS
 *
 * INPUT: achFilename - the filename of the TCS script.
 *        ulRegOffset - the register offset to be added to all register values.
 * FUNCTION: parses and runs a TCS script
 * RETURN:  NULL if failed.
 */
const char* bcmRunTCS(
	const char achFilename[],
	const unsigned long ulRegOffset);



/* End of file */
