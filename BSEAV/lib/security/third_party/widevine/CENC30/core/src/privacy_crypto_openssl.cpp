// Copyright 2013 Google Inc. All Rights Reserved.
//
// Description:
//   Definition of classes representing RSA public keys used
//   for signature verification and encryption and decryption.
//

#include "privacy_crypto.h"

#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include "log.h"

namespace {
const int kPssSaltLength = 20;
const int kRsaPkcs1OaepPaddingLength = 41;

RSA* GetKey(const std::string& serialized_key) {
  BIO* bio = BIO_new_mem_buf(const_cast<char*>(serialized_key.data()),
                             serialized_key.size());
  if (bio == NULL) {
    LOGE("GetKey: BIO_new_mem_buf returned NULL");
    return NULL;
  }
  RSA* key = d2i_RSAPublicKey_bio(bio, NULL);
  BIO_free(bio);

  if (key == NULL) {
    LOGE("GetKey: RSA key deserialization failure: %s",
         ERR_error_string(ERR_get_error(), NULL));
    return NULL;
  }

  return key;
}

void FreeKey(RSA* key) {
  if (key != NULL) {
    RSA_free(key);
  }
}

}  // namespace

namespace wvcdm {

AesCbcKey::AesCbcKey() {}

AesCbcKey::~AesCbcKey() {}

bool AesCbcKey::Init(const std::string& key) {
  if (key.size() != AES_BLOCK_SIZE) {
    LOGE("AesCbcKey::Init: unexpected key size: %d", key.size());
    return false;
  }

  key_ = key;
  return true;
}

bool AesCbcKey::Encrypt(const std::string& in, std::string* out,
                        std::string* iv) {
  if (in.empty()) {
    LOGE("AesCbcKey::Encrypt: no cleartext provided");
    return false;
  }
  if (iv == NULL) {
    LOGE("AesCbcKey::Encrypt: initialization vector destination not provided");
    return false;
  }
  if (iv->size() != AES_BLOCK_SIZE) {
    LOGE("AesCbcKey::Encrypt: invalid iv size: %d", iv->size());
    return false;
  }
  if (out == NULL) {
    LOGE("AesCbcKey::Encrypt: crypttext destination not provided");
    return false;
  }
  if (key_.empty()) {
    LOGE("AesCbcKey::Encrypt: AES key not initialized");
    return false;
  }

  EVP_CIPHER_CTX ctx;
  if (EVP_EncryptInit(&ctx, EVP_aes_128_cbc(),
                      reinterpret_cast<uint8_t*>(&key_[0]),
                      reinterpret_cast<uint8_t*>(&(*iv)[0])) == 0) {
    LOGE("AesCbcKey::Encrypt: AES CBC setup failure: %s",
         ERR_error_string(ERR_get_error(), NULL));
    return false;
  }

  out->resize(in.size() + AES_BLOCK_SIZE);
  int out_length = out->size();
  if (EVP_EncryptUpdate(
          &ctx, reinterpret_cast<uint8_t*>(&(*out)[0]), &out_length,
          reinterpret_cast<uint8_t*>(const_cast<char*>(in.data())),
          in.size()) == 0) {
    LOGE("AesCbcKey::Encrypt: encryption failure: %s",
         ERR_error_string(ERR_get_error(), NULL));
    return false;
  }

  int padding = 0;
  if (EVP_EncryptFinal_ex(&ctx, reinterpret_cast<uint8_t*>(&(*out)[out_length]),
                          &padding) == 0) {
    LOGE("AesCbcKey::Encrypt: PKCS7 padding failure: %s",
         ERR_error_string(ERR_get_error(), NULL));
    return false;
  }

  out->resize(out_length + padding);
  return true;
}

RsaPublicKey::RsaPublicKey() {}

RsaPublicKey::~RsaPublicKey() {}

bool RsaPublicKey::Init(const std::string& serialized_key) {
  if (serialized_key.empty()) {
    LOGE("RsaPublicKey::Init: no serialized key provided");
    return false;
  }

  serialized_key_ = serialized_key;
  return true;
}

bool RsaPublicKey::Encrypt(const std::string& clear_message,
                           std::string* encrypted_message) {
  if (clear_message.empty()) {
    LOGE("RsaPublicKey::Encrypt: message to be encrypted is empty");
    return false;
  }
  if (encrypted_message == NULL) {
    LOGE("RsaPublicKey::Encrypt: no encrypt message buffer provided");
    return false;
  }
  if (serialized_key_.empty()) {
    LOGE("RsaPublicKey::Encrypt: RSA key not initialized");
    return false;
  }

  RSA* key = GetKey(serialized_key_);
  if (key == NULL) {
    // Error already logged by GetKey.
    return false;
  }

  int rsa_size = RSA_size(key);
  if (static_cast<int>(clear_message.size()) >
      rsa_size - kRsaPkcs1OaepPaddingLength) {
    LOGE("RsaPublicKey::Encrypt: message too large to be encrypted (actual %d",
         " max allowed %d)", clear_message.size(),
         rsa_size - kRsaPkcs1OaepPaddingLength);
    FreeKey(key);
    return false;
  }

  encrypted_message->assign(rsa_size, 0);
  if (RSA_public_encrypt(
          clear_message.size(),
          const_cast<unsigned char*>(
              reinterpret_cast<const unsigned char*>(clear_message.data())),
          reinterpret_cast<unsigned char*>(&(*encrypted_message)[0]), key,
          RSA_PKCS1_OAEP_PADDING) != rsa_size) {
    LOGE("RsaPublicKey::Encrypt: encrypt failure: %s",
         ERR_error_string(ERR_get_error(), NULL));
    FreeKey(key);
    return false;
  }

  return true;
}

// LogOpenSSLError is a callback from OpenSSL which is called with each error
// in the thread's error queue.
static int LogOpenSSLError(const char *msg, size_t /* len */, void */* ctx */) {
  LOGE("  %s", msg);
  return 1;
}

static bool VerifyPSSSignature(EVP_PKEY *pkey, const std::string &message,
                               const std::string &signature) {
  EVP_MD_CTX ctx;
  EVP_MD_CTX_init(&ctx);
  EVP_PKEY_CTX *pctx = NULL;

  if (EVP_DigestVerifyInit(&ctx, &pctx, EVP_sha1(), NULL /* no ENGINE */,
                           pkey) != 1) {
    LOGE("EVP_DigestVerifyInit failed in VerifyPSSSignature");
    goto err;
  }

  if (EVP_PKEY_CTX_set_signature_md(pctx, EVP_sha1()) != 1) {
    LOGE("EVP_PKEY_CTX_set_signature_md failed in VerifyPSSSignature");
    goto err;
  }

  if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PSS_PADDING) != 1) {
    LOGE("EVP_PKEY_CTX_set_rsa_padding failed in VerifyPSSSignature");
    goto err;
  }

  if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, kPssSaltLength) != 1) {
    LOGE("EVP_PKEY_CTX_set_rsa_pss_saltlen failed in VerifyPSSSignature");
    goto err;
  }

  if (EVP_DigestVerifyUpdate(&ctx, message.data(), message.size()) != 1) {
    LOGE("EVP_DigestVerifyUpdate failed in VerifyPSSSignature");
    goto err;
  }

  if (EVP_DigestVerifyFinal(
          &ctx, const_cast<uint8_t *>(
                    reinterpret_cast<const uint8_t *>(signature.data())),
          signature.size()) != 1) {
    LOGE(
        "EVP_DigestVerifyFinal failed in VerifyPSSSignature. (Probably a bad "
        "signature.)");
    goto err;
  }

  EVP_MD_CTX_cleanup(&ctx);
  return true;

err:
  ERR_print_errors_cb(LogOpenSSLError, NULL);
  EVP_MD_CTX_cleanup(&ctx);
  return false;
}

bool RsaPublicKey::VerifySignature(const std::string& message,
                                   const std::string& signature) {
  if (serialized_key_.empty()) {
    LOGE("RsaPublicKey::VerifySignature: RSA key not initialized");
    return false;
  }
  if (message.empty()) {
    LOGE("RsaPublicKey::VerifySignature: signed message is empty");
    return false;
  }
  RSA* rsa_key = GetKey(serialized_key_);
  if (rsa_key == NULL) {
    // Error already logged by GetKey.
    return false;
  }
  EVP_PKEY *pkey = EVP_PKEY_new();
  if (pkey == NULL ||
      EVP_PKEY_set1_RSA(pkey, rsa_key) != 1) {
    FreeKey(rsa_key);
    LOGE("RsaPublicKey::VerifySignature: failed to wrap key in an EVP_PKEY");
    return false;
  }
  FreeKey(rsa_key);

  const bool ok = VerifyPSSSignature(pkey, message, signature);
  EVP_PKEY_free(pkey);

  if (!ok) {
    LOGE("RsaPublicKey::VerifySignature: RSA verify failure");
    return false;
  }

  return true;
}

}  // namespace wvcdm
