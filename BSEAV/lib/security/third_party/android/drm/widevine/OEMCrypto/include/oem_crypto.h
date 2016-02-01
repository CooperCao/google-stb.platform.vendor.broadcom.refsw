/*
 * oem_crypto.h
 *
 *  Created on: Oct 22, 2012
 *      Author: cdisc
 */

#ifndef OEM_CRYPTO_H_
#define OEM_CRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "oem_crypto_types.h"

#define OEMCrypto_EncryptAndStoreKeybox _oec02
#define OEMCrypto_IdentifyDevice _oec03
#define OEMCrypto_GetKeyboxData _oec01
#define OEMCrypto_GetRandom _oec04


/******************************************************************************
** FUNCTION
**   OEMCrypto_Initialize
**
** DESCRIPTION:
**    Initializes the crypto hardware
**
** PARAMETERS:
**   void
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_INIT_FAILED
**
******************************************************************************/
OEMCryptoResult OEMCrypto_Initialize(void);


/******************************************************************************
** FUNCTION
**   OEMCrypto_Terminate
**
** DESCRIPTION:
**    Close crypto engine and release resources
**
** PARAMETERS:
**   void
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_TERMINATE_FAILED
**
******************************************************************************/
OEMCryptoResult OEMCrypto_Terminate(void);


/******************************************************************************
** FUNCTION
**   OEMCrypto_Open
**
** DESCRIPTION:
**    Open the security engine and provide a block of memory that can be used
**    for crypto operations
**
** PARAMETERS:
**   buffer - address of a valid pointer
**   bufferSize - on return, size of the allocated buffer
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_OPEN_FAILED
**
******************************************************************************/
OEMCryptoResult OEMCrypto_Open(
		OEMCrypto_UINT8** buffer,
		OEMCrypto_UINT32 *bufferSize);



/******************************************************************************
** FUNCTION
**   OEMCrypto_Close
**
** DESCRIPTION:
**    Free the memory allocated in OEMCrypto_Open
**
** PARAMETERS:
**   void
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_CLOSE_FAILED
**
******************************************************************************/
OEMCryptoResult OEMCrypto_Close(void);


/******************************************************************************
** FUNCTION
**   OEMCrypto_SetEntitlementKey
**
** DESCRIPTION:
**    Decrypt Emm (or asset key) using the encrypted Device Key found in the keybox.
**
** PARAMETERS:
**   emmKey - pointer to the encrypted assert key
**   emmKeyLength - size of the emm key
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_NO_DEVICE_KEY: failed to decrypt device key
**   OEMCrypto_ERROR_KEYBOX_INVALID
**   OEMCrypto_ERROR_NO_ASSET_KEY: failed to decrypt asset key
**
******************************************************************************/
OEMCryptoResult OEMCrypto_SetEntitlementKey(
		const OEMCrypto_UINT8* emmKey,
		const OEMCrypto_UINT32 emmKeyLength);


/******************************************************************************
** FUNCTION
**   OEMCrypto_DeriveControlWord
**
** DESCRIPTION:
**    using active keyladder from SetEntitlementKey, decrypt ECM,
**    return 4-byte ID and load decrypted content key for future payload decryption
**
** PARAMETERS:
**   ecmKey - pointer to the encrypted content key
**   ecmKeyLength - size of the ecm key
**   flags - to contain 4-byte ID value
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_NO_CW: failed to load the Control Word
**
******************************************************************************/
OEMCryptoResult OEMCrypto_DeriveControlWord(
		const OEMCrypto_UINT8* ecmKey,
		const OEMCrypto_UINT32 ecmKeyLength,
		OEMCrypto_UINT32* flags);


/******************************************************************************
** FUNCTION
**   OEMCrypto_Decrypt
**
** DESCRIPTION:
**    Decrypt a buffer using AES-CBC (128bit) algorithm with CTS mode
**
** PARAMETERS:
**   input - input source
**   output - buffer to return clear payload. The size of the output buffer
**            must be >= size of the inputLength
**   inputLength - size of the input (encrypted) source buffer
**   initIvFlag - indicates whether or not to initialize the IV to all zeros
**   				(i.e. 1 = required, 0 = continue)
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_DECRYPT_FAILED
**
******************************************************************************/
OEMCryptoResult OEMCrypto_Decrypt(
		OEMCrypto_UINT8* input,
		OEMCrypto_UINT8* output,
		const OEMCrypto_UINT32 inputLength,
		OEMCrypto_UINT8 initIvFlag);


/******************************************************************************
** FUNCTION
**   OEMCrypto_InstallKeybox
**
** DESCRIPTION:
**	Decrypt and load keybox into security processor
**
** PARAMETERS:
**   keybox - pointer to encrypted keybox data
**   keyboxLength - length of keybox data in bytes
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_WRITE_KEYBOX
**
******************************************************************************/
OEMCryptoResult OEMCrypto_InstallKeybox(
		OEMCrypto_UINT8* keybox,
		OEMCrypto_UINT32 keyboxLength);


/******************************************************************************
** FUNCTION
**   OEMCrypto_EncryptAndStoreKeybox
**
** DESCRIPTION:
**	Encrypt and store keybox to persistent memory
**
** PARAMETERS:
**   keybox - pointer to encrypted keybox data
**   keyboxLength - length of keybox data in bytes
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_ENCRYPT_STORE_KEYBOX
**
******************************************************************************/
OEMCryptoResult OEMCrypto_EncryptAndStoreKeybox(
		OEMCrypto_UINT8* keybox,
		OEMCrypto_UINT32 keyboxLength);


/******************************************************************************
** FUNCTION
**   OEMCrypto_isKeyBoxValid
**
** DESCRIPTION:
**	Validates keybox loaded into security processor
**
** PARAMETERS:
**   void
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_BAD_MAGIC
**   OEMCrypto_ERROR_BAD_CRC
**
******************************************************************************/
OEMCryptoResult OEMCrypto_isKeyBoxValid(void);


/******************************************************************************
** FUNCTION
**   OEMCrypto_GetDeviceID
**
** DESCRIPTION:
**	return the Device ID field of the keybox
**
** PARAMETERS:
**   deviceId[out] - pointer to the buffer that receives the Device ID
**   idLength[in/out] - on input, size of the caller's Id buffer
**   			on output, the number of bytes written into the buffer
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_SHORT_BUFFER: if buffer is too small to return device Id
**   OEMCrypto_ERROR_NO_DEVICEID: failed to return device ID
**
******************************************************************************/
OEMCryptoResult OEMCrypto_GetDeviceID(
		OEMCrypto_UINT8* deviceId,
		OEMCrypto_UINT32* idLength);


/******************************************************************************
** FUNCTION
**   OEMCrypto_IdentifyDevice
**
** DESCRIPTION:
**	Return device unique identifier (not from the keybox)
**
** PARAMETERS:
**   deviceId[out] - pointer to the buffer that receives the unique id
**   idLength[in] - length of unique identifier, max 32 bytes
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_IDENTIFY_DEVICE
**
******************************************************************************/
OEMCryptoResult OEMCrypto_IdentifyDevice(
		OEMCrypto_UINT8* deviceId,
		OEMCrypto_UINT32 idLength);



/******************************************************************************
** FUNCTION
**   OEMCrypto_GetKeyData
**
** DESCRIPTION:
**	Return key data field from KeyBox
**
** PARAMETERS:
**   keyData[out] - pointer to the buffer that receives the Key Data
**   keyDataLength[in/out] - on input, size of the caller's Key Data buffer
**   			on output, the number of bytes written into the buffer
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_GET_KEY_DATA
**
******************************************************************************/
OEMCryptoResult OEMCrypto_GetKeyData(
		OEMCrypto_UINT8* keyData,
		OEMCrypto_UINT32* keyDataLength);


/******************************************************************************
** FUNCTION
**   OEMCrypto_GetKeyboxData
**
** DESCRIPTION:
**	 decrypt Keybox and return a range of bytes.
**
** PARAMETERS:
**   buffer - pointer to buffer that will receive data
**   offset - byte offset from the beginning of the keybox of the first byte
**   		  to return
**   length - number of bytes of data to return
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_GET_KEYBOX_DATA
**
******************************************************************************/
OEMCryptoResult OEMCrypto_GetKeyboxData(
		OEMCrypto_UINT8* buffer,
		OEMCrypto_UINT32 offset,
		OEMCrypto_UINT32 length);



/******************************************************************************
** FUNCTION
**   OEMCrypto_GetRandom
**
** DESCRIPTION:
**	 Fill a buffer with HW generated bytes
**
** PARAMETERS:
**   randomData - buffer to store the random data
**   dataLength - length of random data buffer in bytes
**
** RETURNS:
**   OEMCrypto_SUCCESS
**   OEMCrypto_ERROR_RNG_FAILED
**   OEMCrypto_ERROR_RNG_NOT_SUPPORTED
**
******************************************************************************/
OEMCryptoResult OEMCrypto_GetRandom(
		OEMCrypto_UINT8* randomData,
		OEMCrypto_UINT32 dataLength);

#ifdef __cplusplus
}
#endif

#endif /* OEM_CRYPTO_H_ */
