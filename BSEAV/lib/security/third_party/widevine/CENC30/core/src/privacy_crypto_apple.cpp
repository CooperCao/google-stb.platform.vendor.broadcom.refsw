// Copyright 2015 Google Inc. All Rights Reserved.
//
// Description:
//   Privacy crypto implementation for iOS.  This fully implements the
//   privacy_crypto methods.  This assumes this is compiled on a Mac and is
//   being compiled for iOS.  Requires iOS 2.0 or later.
//
//   This is never included in the default builds.  If compiling using the gyp
//   files, setting privacy_crypto_impl to "apple" with use this file rather
//   than the openssl version.

#include "privacy_crypto.h"

#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#include <CoreFoundation/CoreFoundation.h>
#include <memory>
#include <Security/Security.h>
#include <Security/SecKey.h>

#include "string_conversions.h"
#include "log.h"

#define KEYSTORE_NAME "com.google.widevine.publicKey"

namespace {

const int kPssSaltLength = 20;
const int kOaepMinPadding = 2 * CC_SHA1_DIGEST_LENGTH + 1;

template<typename T>
struct CFDeleter {
  void operator()(T arg) {
    CFRelease(arg);
  }
};

template<typename T>
using CF =
    std::unique_ptr<typename std::remove_pointer<T>::type, CFDeleter<T> >;

SecKeyRef ImportPublicKey(const std::string& key) {
  std::string peerStr = KEYSTORE_NAME;
  CF<CFDataRef> peerData(CFDataCreate(NULL,
      reinterpret_cast<const UInt8*>(peerStr.c_str()), peerStr.length()));
  CF<CFDataRef> keyData(CFDataCreate(NULL,
      reinterpret_cast<const UInt8*>(key.c_str()), key.length()));

  // Create a selector and delete all old keys.
  CF<CFMutableDictionaryRef> deleteAttributes(CFDictionaryCreateMutable(
      NULL, 0, NULL, NULL));
  CFDictionarySetValue(deleteAttributes.get(), kSecClass, kSecClassKey);
  CFDictionarySetValue(deleteAttributes.get(), kSecAttrKeyType,
      kSecAttrKeyTypeRSA);
  CFDictionarySetValue(deleteAttributes.get(), kSecAttrApplicationTag,
      peerData.get());
  SecItemDelete(deleteAttributes.get());

  // Create attributes to add to the keystore.
  CF<CFMutableDictionaryRef> addAttributes(CFDictionaryCreateMutable(
      NULL, 0, NULL, NULL));
  CFDictionarySetValue(addAttributes.get(), kSecClass, kSecClassKey);
  CFDictionarySetValue(addAttributes.get(), kSecAttrKeyType,
      kSecAttrKeyTypeRSA);
  CFDictionarySetValue(addAttributes.get(), kSecAttrApplicationTag,
      peerData.get());
  CFDictionarySetValue(addAttributes.get(), kSecValueData,
      keyData.get());
  CFDictionarySetValue(addAttributes.get(), kSecAttrKeyClass,
      kSecAttrKeyClassPublic);
  CFDictionarySetValue(addAttributes.get(), kSecReturnPersistentRef,
      kCFBooleanTrue);

  // Add the key to the keystore.
  CFTypeRef temp;
  OSStatus status = SecItemAdd(addAttributes.get(), &temp);
  CF<CFTypeRef> peer(temp);
  if (!peer || (status != noErr && status != errSecDuplicateItem)) {
    LOGE("RsaPublicKey::Init: Error adding key to keychain %d", status);
    return NULL;
  }

  // Create attributes for for the query.
  CF<CFMutableDictionaryRef> queryAttributes(CFDictionaryCreateMutable(
      NULL, 0, NULL, NULL));
  CFDictionarySetValue(queryAttributes.get(), kSecClass, kSecClassKey);
  CFDictionarySetValue(queryAttributes.get(), kSecAttrApplicationTag,
      peerData.get());
  CFDictionarySetValue(queryAttributes.get(), kSecAttrKeyType,
      kSecAttrKeyTypeRSA);
  CFDictionarySetValue(queryAttributes.get(), kSecAttrKeyClass,
      kSecAttrKeyClassPublic);
  CFDictionarySetValue(queryAttributes.get(), kSecReturnRef, kCFBooleanTrue);

  // Query the keychain to get the public key ref.
  CFTypeRef keyRef = NULL;
  status = SecItemCopyMatching(queryAttributes.get(), &keyRef);
  if (status != noErr) {
    LOGE("RsaPublicKey::Init: Error getting key from keystore %d", status);
    return NULL;
  }

  return reinterpret_cast<SecKeyRef>(const_cast<void*>(keyRef));
}

// Apply a custom mask generation function (MGF) using the hash function SHA1,
// this is from OpenSSL.
void ApplyMGF1_SHA1(uint8_t *output, size_t outputLength,
                    const uint8_t* seed, size_t seedLength) {
  size_t outputIndex = 0;
  for (int i = 0; outputIndex < outputLength; i++) {
    uint8_t extra[4];
    extra[0] = (uint8_t)((i >> 24) & 0xFF);
    extra[1] = (uint8_t)((i >> 16) & 0xFF);
    extra[2] = (uint8_t)((i >>  8) & 0xFF);
    extra[3] = (uint8_t)(i & 0xFF);

    CC_SHA1_CTX ctx;
    CC_SHA1_Init(&ctx);
    CC_SHA1_Update(&ctx, seed, seedLength);
    CC_SHA1_Update(&ctx, extra, 4);

    if (outputIndex + CC_SHA1_DIGEST_LENGTH <= outputLength) {
      CC_SHA1_Final(output + outputIndex, &ctx);
    } else {
      uint8_t temp[CC_SHA1_DIGEST_LENGTH];
      CC_SHA1_Final(temp, &ctx);
      memcpy(output + outputIndex, temp, outputLength - outputIndex);
    }
    outputIndex += CC_SHA1_DIGEST_LENGTH;
  }
}

std::string ApplyOAEPPadding(const std::string& messageStr, size_t rsaSize) {
  if (messageStr.length() > rsaSize - kOaepMinPadding ) {
    LOGE("RsaPublicKey::Encrypt: message too large to be encrypted (actual %d",
         " max allowed %d)", messageStr.size(),
         rsaSize - kOaepMinPadding );
    return "";
  }

  // https://tools.ietf.org/html/rfc2437#section-9.1.1.2
  //
  // result         db
  // |------------------------------------------------------------------------|
  // |0|    seed    |   pHash    |000000000|1|             M                  |
  // |------------------------------------------------------------------------|
  // | |<-mdLength->|<-mdLength->|<-psLen->| |<-------messageLength---------->|
  // |<------------paddingLength------------>|

  std::string ret;
  ret.resize(rsaSize);
  size_t messageLength = messageStr.length();
  size_t paddingLength = rsaSize - messageLength;
  size_t psLen = paddingLength - kOaepMinPadding;
  const uint8_t *message = reinterpret_cast<const uint8_t*>(messageStr.data());
  uint8_t *result = reinterpret_cast<uint8_t*>(&ret[0]);
  uint8_t *seed = result + 1;
  uint8_t *db = result + CC_SHA1_DIGEST_LENGTH + 1;

  // Initialize db and message
  CC_SHA1(NULL, 0, db); // Hash of empty string.
  result[rsaSize - messageLength - 1] = 0x1;
  memcpy(result + paddingLength, message, messageLength);

  // Initialize seed
  if (SecRandomCopyBytes(kSecRandomDefault, CC_SHA1_DIGEST_LENGTH, seed)) {
    LOGE("RsaPublicKey::Encrypt: unable to get random data %d", errno);
    return "";
  }

  // Create the first mask
  std::vector<uint8_t> dbmask;
  dbmask.resize(rsaSize - CC_SHA1_DIGEST_LENGTH - 1);
  ApplyMGF1_SHA1(dbmask.data(), dbmask.size(), seed, CC_SHA1_DIGEST_LENGTH);
  for (int i = 0; i < dbmask.size(); i++) {
    db[i] ^= dbmask[i];
  }

  // Create the second mask
  uint8_t seedmask[CC_SHA1_DIGEST_LENGTH];
  ApplyMGF1_SHA1(seedmask, CC_SHA1_DIGEST_LENGTH, db, dbmask.size());
  for (int i = 0; i < CC_SHA1_DIGEST_LENGTH; i++) {
    seed[i] ^= seedmask[i];
  }

  return ret;
}

bool PSSVerify(const uint8_t *message, size_t messageLength,
               const uint8_t *encodedMessage, size_t encodedMessageLength) {
  // https://tools.ietf.org/html/rfc3447#section-9.1.2
  //
  // M'
  // |---------------------------------------------------|
  // | 00 00 00 00 00 00 00 00 |    mHash    |   salt    |
  // |---------------------------------------------------|
  //
  // H = hash(M')
  // dbMask = MGF(H)
  //
  // db
  // |------------------------|
  // | 00 00 ... 00 01 | salt |
  // |------------------------|
  // |<----messageLength----->|
  //
  // maskedDb = db ^ dbMask
  // encodedMessage
  // |--------------------------------------------------|
  // |          maskedDb      |        H           | bc |
  // |--------------------------------------------------|

  if (encodedMessage[encodedMessageLength - 1] != 0xbc) {
    return false;
  }

  const uint8_t *maskedDb = encodedMessage;
  size_t dbLength = encodedMessageLength - CC_SHA1_DIGEST_LENGTH - 1;
  const uint8_t *H = maskedDb + dbLength;

  // Decode db
  std::vector<uint8_t> dbMask;
  dbMask.resize(dbLength);
  ApplyMGF1_SHA1(dbMask.data(), dbMask.size(), H, CC_SHA1_DIGEST_LENGTH);
  for (int i = 0; i < dbLength; i++) {
    dbMask[i] ^= maskedDb[i];
  }

  // Verify db
  dbMask[0] &= 0x7F;
  for (int i = 0; i < dbLength - kPssSaltLength - 1; i++) {
    if (dbMask[i] != 0) {
      return false;
    }
  }
  if (dbMask[dbLength - kPssSaltLength - 1] != 0x01) {
    return false;
  }

  uint8_t *salt = dbMask.data() + (dbLength - kPssSaltLength);
  uint8_t mHash[CC_SHA1_DIGEST_LENGTH];
  CC_SHA1(message, messageLength, mHash);

  // Create our version of the message data (M')
  std::vector<uint8_t> dataVec;
  dataVec.resize(8 + CC_SHA1_DIGEST_LENGTH + kPssSaltLength);
  uint8_t *data = dataVec.data();
  memcpy(data + 8, mHash, CC_SHA1_DIGEST_LENGTH);
  memcpy(data + 8 + CC_SHA1_DIGEST_LENGTH, salt, kPssSaltLength);

  // Verify the hash of the message data.
  uint8_t H2[CC_SHA1_DIGEST_LENGTH];
  CC_SHA1(data, dataVec.size(), H2);
  return !memcmp(H, H2, CC_SHA1_DIGEST_LENGTH);
}

}  // namespace

namespace wvcdm {

AesCbcKey::AesCbcKey() {}

AesCbcKey::~AesCbcKey() {}

bool AesCbcKey::Init(const std::string& key) {
  assert(key.size() == kCCBlockSizeAES128);
  this->key_ = key;
  return true;
}

bool AesCbcKey::Encrypt(const std::string& in, std::string* out,
                        std::string* iv) {
  assert(!in.empty());
  assert(iv != NULL);
  assert(iv->size() == kCCBlockSizeAES128);
  assert(out != NULL);
  assert(!key_.empty());

  std::string temp;
  temp.resize(in.length() + kCCBlockSizeAES128);
  size_t length;
  CCCryptorStatus result = CCCrypt(kCCEncrypt, kCCAlgorithmAES128,
      kCCOptionPKCS7Padding, key_.c_str(), key_.length(), iv->c_str(),
      in.c_str(), in.length(), &temp[0], temp.size(), &length);

  if (result != kCCSuccess) {
    LOGE("AesCbcKey::Encrypt: Encryption failure: %d", result);
    return false;
  }

  out->assign(temp, 0, length);
  return true;
}

RsaPublicKey::RsaPublicKey() {}

RsaPublicKey::~RsaPublicKey() {}

bool RsaPublicKey::Init(const std::string& serialized_key) {
  assert(!serialized_key.empty());

  this->serialized_key_ = serialized_key;
  return true;
}

bool RsaPublicKey::Encrypt(const std::string& clear_message,
                           std::string* encrypted_message) {
  assert(!clear_message.empty());
  assert(encrypted_message != NULL);

  SecKeyRef key = ImportPublicKey(serialized_key_);
  if (!key) {
    return false;
  }

  size_t rsaSize = SecKeyGetBlockSize(key);
  std::string paddedMessage = ApplyOAEPPadding(clear_message, rsaSize);
  if (paddedMessage.empty()) {
    return false;
  }

  size_t size = paddedMessage.length();
  std::string buffer;
  buffer.resize(size);
  OSStatus status = SecKeyEncrypt(key, kSecPaddingNone,
      reinterpret_cast<const uint8_t*>(paddedMessage.c_str()),
      paddedMessage.length(),
      reinterpret_cast<uint8_t*>(&buffer[0]), &size);
  if (status != errSecSuccess) {
    LOGE("RsaPublicKey::Encrypt: Unable to encrypt data %d", status);
    return false;
  }

  encrypted_message->assign(buffer, 0, size);
  return true;
}

bool RsaPublicKey::VerifySignature(const std::string& message,
                                   const std::string& signature) {
  assert(!message.empty());
  assert(!signature.empty());

  SecKeyRef key = ImportPublicKey(serialized_key_);
  if (!key) {
    return false;
  }

  // "decrypt" the signature
  std::vector<uint8_t> buffer;
  buffer.resize(signature.length());
  size_t size = buffer.size();
  OSStatus status = SecKeyEncrypt(key, kSecPaddingNone,
          reinterpret_cast<const uint8_t*>(signature.c_str()),
          signature.length(),
          buffer.data(), &size);
  if (status != errSecSuccess) {
    LOGE("RsaPublicKey::VerifySignature: Unable to decrypt signature %d",
            status);
    return false;
  }

  // Verify the signature
  if (!PSSVerify(reinterpret_cast<const uint8_t*>(message.c_str()),
                message.length(),
                buffer.data(), buffer.size())) {
    LOGE("RsaPublicKey::VerifySignature: Unable to verify signature %d",
         status);
    return false;
  }

  return true;
}

}  // namespace wvcdm
