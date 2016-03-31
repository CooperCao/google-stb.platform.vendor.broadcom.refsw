// Copyright 2015 Google Inc. All Rights Reserved.
//
// This source file provides a basic set of unit tests for the Content
// Decryption Module (CDM).

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cdm.h"
#include "cdm_test_printers.h"
#include "license_request.h"
#include "log.h"
#include "OEMCryptoCENC.h"
#include "override.h"
#include "properties_ce.h"
#include "scoped_ptr.h"
#include "string_conversions.h"
#include "test_host.h"
#include "url_request.h"

using namespace testing;
using namespace wvcdm;

namespace widevine {

namespace {

const int kHttpOk = 200;

const int kRenewalTestDelayMs = 3 * 60 * 1000;
const int kExpirationTestDelayMs = 5 * 60 * 1000;

const Cdm::SessionType kBogusSessionType = static_cast<Cdm::SessionType>(-1);
const Cdm::InitDataType kBogusInitDataType = static_cast<Cdm::InitDataType>(-1);
const std::string kBogusSessionId = "asdf";

const std::string kDefaultServerCertificate = a2bs_hex(
    "0ABF020803121028703454C008F63618ADE7443DB6C4C8188BE7F99005228E023082010A02"
    "82010100B52112B8D05D023FCC5D95E2C251C1C649B4177CD8D2BEEF355BB06743DE661E3D"
    "2ABC3182B79946D55FDC08DFE95407815E9A6274B322A2C7F5E067BB5F0AC07A89D45AEA94"
    "B2516F075B66EF811D0D26E1B9A6B894F2B9857962AA171C4F66630D3E4C602718897F5E1E"
    "F9B6AAF5AD4DBA2A7E14176DF134A1D3185B5A218AC05A4C41F081EFFF80A3A040C50B09BB"
    "C740EEDCD8F14D675A91980F92CA7DDC646A06ADAD5101F74A0E498CC01F00532BAC217850"
    "BD905E90923656B7DFEFEF42486767F33EF6283D4F4254AB72589390BEE55808F1D668080D"
    "45D893C2BCA2F74D60A0C0D0A0993CEF01604703334C3638139486BC9DAF24FD67A07F9AD9"
    "4302030100013A1273746167696E672E676F6F676C652E636F6D128003983E30352675F40B"
    "A715FC249BDAE5D4AC7249A2666521E43655739529721FF880E0AAEFC5E27BC980DAEADABF"
    "3FC386D084A02C82537848CC753FF497B011A7DA97788A00E2AA6B84CD7D71C07A48EBF616"
    "02CCA5A3F32030A7295C30DA915B91DC18B9BC9593B8DE8BB50F0DEDC12938B8E9E039CDDE"
    "18FA82E81BB032630FE955D85A566CE154300BF6D4C1BD126966356B287D657B18CE63D0EF"
    "D45FC5269E97EAB11CB563E55643B26FF49F109C2101AFCAF35B832F288F0D9D45960E259E"
    "85FB5D24DBD2CF82764C5DD9BF727EFBE9C861F869321F6ADE18905F4D92F9A6DA6536DB84"
    "75871D168E870BB2303CF70C6E9784C93D2DE845AD8262BE7E0D4E2E4A0759CEF82D109D25"
    "92C72429F8C01742BAE2B3DECADBC33C3E5F4BAF5E16ECB74EADBAFCB7C6705F7A9E3B6F39"
    "40383F9C5116D202A20C9229EE969C2519718303B50D0130C3352E06B014D838540F8A0C22"
    "7C0011E0F5B38E4E298ED2CB301EB4564965F55C5D79757A250A4EB9C84AB3E6539F6B6FDF"
    "56899EA29914");

const std::string kLicenseServer = "http://widevine-proxy.appspot.com/proxy";
const std::string kCencInitData = a2bs_hex(
    "00000042"                          // blob size
    "70737368"                          // "pssh"
    "00000000"                          // flags
    "edef8ba979d64acea3c827dcd51d21ed"  // Widevine system id
    "00000022"                          // pssh data size
    // pssh data:
    "08011a0d7769646576696e655f746573"
    "74220f73747265616d696e675f636c69"
    "7031");
const std::string kCencPersistentInitData = a2bs_hex(
    "00000040"                          // blob size
    "70737368"                          // "pssh"
    "00000000"                          // flags
    "edef8ba979d64acea3c827dcd51d21ed"  // Widevine system id
    "00000020"                          // pssh data size
    // pssh data:
    "08011a0d7769646576696e655f746573"
    "74220d6f66666c696e655f636c697032");
const std::string kInvalidCencInitData = a2bs_hex(
    "0000000c"                          // blob size
    "61736466"                          // "asdf" (wrong box type)
    "01020304");                        // nonsense
const std::string kNonWidevineCencInitData = a2bs_hex(
    "00000020"                          // blob size
    "70737368"                          // "pssh"
    "00000000"                          // flags
    "000102030405060708090a0b0c0d0e0f"  // unknown system id
    "00000000");                        // pssh data size
const std::string kWebMInitData = a2bs_hex("deadbeefdeadbeefdeadbeefdeadbeef");
const std::string kKeyIdsInitData =
    "{\"kids\":[\"67ef0gd8pvfd0\",\"77ef0gd8pvfd0\"]}";

// Dummy encrypted data.
const std::vector<uint8_t> kKeyId1 = a2b_hex(
    "371ea35e1a985d75d198a7f41020dc23");
const std::vector<uint8_t> kInput1 = a2b_hex(
    "64ab17b3e3dfab47245c7cce4543d4fc7a26dcf248f19f9b59f3c92601440b36"
    "17c8ed0c96c656549e461f38708cd47a434066f8df28ccc28b79252eee3f9c2d"
    "7f6c68ebe40141fe818fe082ca523c03d69ddaf183a93c022327fedc5582c5ab"
    "ca9d342b71263a67f9cb2336f12108aaaef464f17177e44e9b0c4e56e61da53c"
    "2150b4405cc82d994dfd9bf4087c761956d6688a9705db4cf350381085f383c4"
    "9666d4aed135c519c1f0b5cba06e287feea96ea367bf54e7368dcf998276c6e4"
    "6497e0c50e20fef74e42cb518fe7f22ef27202428688f86404e8278587017012"
    "c1d65537c6cbd7dde04aae338d68115a9f430afc100ab83cdadf45dca39db685");
const std::vector<uint8_t> kIv1 = a2b_hex(
    "f6f4b1e600a5b67813ed2bded913ba9f");
const std::vector<uint8_t> kOutput1 = a2b_hex(
    "217ce9bde99bd91e9733a1a00b9b557ac3a433dc92633546156817fae26b6e1c"
    "942ac20a89ff79f4c2f25fba99d6a44618a8c0420b27d54e3da17b77c9d43cca"
    "595d259a1e4a8b6d7744cd98c5d3f921adc252eb7d8af6b916044b676a574747"
    "8df21fdc42f166880d97a2225cd5c9ea5e7b752f4cf81bbdbe98e542ee10e1c6"
    "ad868a6ac55c10d564fc23b8acff407daaf4ed2743520e02cda9680d9ea88e91"
    "029359c4cf5906b6ab5bf60fbb3f1a1c7c59acfc7e4fb4ad8e623c04d503a3dd"
    "4884604c8da8a53ce33db9ff8f1c5bb6bb97f37b39906bf41596555c1bcce9ed"
    "08a899cd760ff0899a1170c2f224b9c52997a0785b7fe170805fd3e8b1127659");

const std::string kValue = "A Value";
const std::string kNewValue = "A New Value";

const std::string kParamName = "PARAM";
const std::string kParamName2 = "PARAM2";


class CdmTest : public Test,
                public Cdm::IEventListener {
 public:
  CdmTest() {}
  virtual ~CdmTest() {}

  // IEventListener mocks:
  MOCK_METHOD3(onMessage,
      void(const std::string& session_id,
           Cdm::MessageType message_type,
           const std::string& message));
  MOCK_METHOD1(onKeyStatusesChange,
      void(const std::string& session_id));
  MOCK_METHOD1(onRemoveComplete,
      void(const std::string& session_id));

 protected:
  virtual void SetUp() OVERRIDE {
    // Clear anything stored, load default device cert.
    g_host->Reset();

    // Clear anything stored by OEMCrypto.
    ASSERT_EQ(OEMCrypto_SUCCESS, OEMCrypto_Initialize());
    ASSERT_EQ(OEMCrypto_SUCCESS, OEMCrypto_DeleteUsageTable());
    ASSERT_EQ(OEMCrypto_SUCCESS, OEMCrypto_Terminate());

    // Reinit the library.
    Cdm::DeviceCertificateRequest cert_request;
    Cdm::Status status = Cdm::initialize(
        Cdm::kNoSecureOutput, PropertiesCE::GetClientInfo(),
        g_host, g_host, g_host, &cert_request,
        static_cast<Cdm::LogLevel>(g_cutoff));
    ASSERT_EQ(Cdm::kSuccess, status);

    // Make a fresh CDM.
    RecreateCdm(true /* privacy_mode */);
    SetDefaultServerCertificate();
  }

  virtual void TearDown() OVERRIDE {
    // So the OEMCrypto nonce flood check does not trigger.
    // A 500ms delay allows up to 10 nonces to be generated per test without
    // triggering an OEMCrypto error.
    usleep(500 * 1000);
  }

  void RecreateCdm(bool privacy_mode) {
    CreateAdditionalCdm(privacy_mode, &cdm_);
  }

  void CreateAdditionalCdm(bool privacy_mode, scoped_ptr<Cdm>* cdm) {
    cdm->reset(Cdm::create(this, privacy_mode));
    ASSERT_NE((Cdm*)0, cdm->get());
  }

  void SetDefaultServerCertificate() {
    // Set the default server certificate.
    Cdm::Status status = cdm_->setServerCertificate(kDefaultServerCertificate);
    ASSERT_EQ(Cdm::kSuccess, status);
  }

  bool Fetch(const std::string& url,
             const std::string& message,
             std::string* response,
             int* status_code) {
    UrlRequest url_request(url);
    EXPECT_TRUE(url_request.is_connected());
    if (!url_request.is_connected()) {
      return false;
    }

    url_request.PostRequest(message);
    std::string http_response;
    url_request.GetResponse(&http_response);

    // Some license servers return 400 for invalid message, some
    // return 500; treat anything other than 200 as an invalid message.
    int http_status_code = url_request.GetStatusCode(http_response);
    if (status_code) {
      *status_code = http_status_code;
    }

    if (response) {
      if (http_status_code == kHttpOk) {
        // Parse out HTTP and server headers and return the body only.
        std::string reply_body;
        LicenseRequest lic_request;
        lic_request.GetDrmMessage(http_response, reply_body);
        *response = reply_body;
      } else {
        *response = http_response;
      }
      LOGV("Reply body: %s", b2a_hex(*response).c_str());
    }
    return true;
  }

  void FetchCertificate(const std::string& url,
                        std::string* response) {
    int status_code;
    bool ok = Fetch(url, "", response, &status_code);
    ASSERT_TRUE(ok);
    if (ok) ASSERT_EQ(kHttpOk, status_code);
  }

  void FetchLicense(const std::string& message,
                    std::string* response) {
    int status_code;
    bool ok = Fetch(kLicenseServer, message, response, &status_code);
    ASSERT_TRUE(ok);
    if (ok) ASSERT_EQ(kHttpOk, status_code);
  }

  void FetchLicenseFailure(const std::string& message,
                           int expected_status_code) {
    int status_code;
    bool ok = Fetch(kLicenseServer, message, NULL, &status_code);
    ASSERT_TRUE(ok);
    if (ok) ASSERT_EQ(expected_status_code, status_code);
  }

  void CreateSessionAndGenerateRequest(Cdm::SessionType session_type,
                                       std::string* session_id,
                                       std::string* message) {
    Cdm::Status status = cdm_->createSession(session_type, session_id);
    ASSERT_EQ(Cdm::kSuccess, status);

    std::string init_data;
    if (session_type == Cdm::kTemporary) {
      init_data = kCencInitData;
    } else {
      init_data = kCencPersistentInitData;
    }

    EXPECT_CALL(*this, onMessage(*session_id, Cdm::kLicenseRequest, _)).
        WillOnce(SaveArg<2>(message));
    status = cdm_->generateRequest(*session_id, Cdm::kCenc, init_data);
    ASSERT_EQ(Cdm::kSuccess, status);
    Mock::VerifyAndClear(this);
  }

  void CreateSessionAndFetchLicense(Cdm::SessionType session_type,
                                    std::string* session_id,
                                    std::string* response) {
    std::string message;
    ASSERT_NO_FATAL_FAILURE(CreateSessionAndGenerateRequest(
        session_type, session_id, &message));
    FetchLicense(message, response);
  }

  void CreateSessionAndUpdate(Cdm::SessionType session_type,
                              std::string* session_id) {
    std::string response;
    ASSERT_NO_FATAL_FAILURE(CreateSessionAndFetchLicense(
        session_type, session_id, &response));
    EXPECT_CALL(*this, onKeyStatusesChange(*session_id));
    Cdm::Status status = cdm_->update(*session_id, response);
    ASSERT_EQ(Cdm::kSuccess, status);
    Mock::VerifyAndClear(this);
  }

  void FetchLicenseAndUpdate(const std::string& session_id,
                             const std::string& message) {
    // Acquire a license.
    std::string response;
    ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

    // This license should be accepted, but the keys are not expected to change.
    EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(0);
    Cdm::Status status = cdm_->update(session_id, response);
    ASSERT_EQ(Cdm::kSuccess, status);
    Mock::VerifyAndClear(this);
  }

  scoped_ptr<Cdm> cdm_;
};


class MockTimerClient : public Cdm::ITimer::IClient {
 public:
  MockTimerClient() {}
  virtual ~MockTimerClient() {}

  MOCK_METHOD1(onTimerExpired, void(void*));
};

}  // namespace


TEST_F(CdmTest, TestHostTimer) {
  // Validate that the TestHost timers are processed in the correct order.
  const int64_t kTimerDelayMs = 1000;
  void* kCtx1 = reinterpret_cast<void*>(0x1);
  void* kCtx2 = reinterpret_cast<void*>(0x2);

  MockTimerClient client;

  g_host->setTimeout(kTimerDelayMs * 1, &client, kCtx1);
  g_host->setTimeout(kTimerDelayMs * 2, &client, kCtx2);

  EXPECT_CALL(client, onTimerExpired(kCtx1));
  g_host->ElapseTime(kTimerDelayMs);
  Mock::VerifyAndClear(&client);

  EXPECT_CALL(client, onTimerExpired(kCtx2));
  g_host->ElapseTime(kTimerDelayMs);
  Mock::VerifyAndClear(&client);

  EXPECT_CALL(client, onTimerExpired(_)).Times(0);
  g_host->ElapseTime(kTimerDelayMs);
  Mock::VerifyAndClear(&client);
}

TEST_F(CdmTest, Initialize) {
  Cdm::DeviceCertificateRequest cert_request;
  Cdm::Status status;

  // Try with an invalid output type.
  status = Cdm::initialize(
      static_cast<Cdm::SecureOutputType>(-1), PropertiesCE::GetClientInfo(),
      g_host, g_host, g_host, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kTypeError, status);

  // Try with various client info properties missing.
  Cdm::ClientInfo working_client_info = PropertiesCE::GetClientInfo();
  Cdm::ClientInfo broken_client_info;

  broken_client_info = working_client_info;
  broken_client_info.product_name.clear();
  status = Cdm::initialize(
      Cdm::kNoSecureOutput, broken_client_info,
      g_host, g_host, g_host, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kTypeError, status);

  broken_client_info = working_client_info;
  broken_client_info.company_name.clear();
  status = Cdm::initialize(
      Cdm::kNoSecureOutput, broken_client_info,
      g_host, g_host, g_host, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kTypeError, status);

  broken_client_info = working_client_info;
  broken_client_info.device_name.clear();  // Not required
  status = Cdm::initialize(
      Cdm::kNoSecureOutput, broken_client_info,
      g_host, g_host, g_host, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kSuccess, status);

  broken_client_info = working_client_info;
  broken_client_info.model_name.clear();
  status = Cdm::initialize(
      Cdm::kNoSecureOutput, broken_client_info,
      g_host, g_host, g_host, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kTypeError, status);

  broken_client_info = working_client_info;
  broken_client_info.arch_name.clear();  // Not required
  status = Cdm::initialize(
      Cdm::kNoSecureOutput, broken_client_info,
      g_host, g_host, g_host, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kSuccess, status);

  broken_client_info = working_client_info;
  broken_client_info.build_info.clear();  // Not required
  status = Cdm::initialize(
      Cdm::kNoSecureOutput, broken_client_info,
      g_host, g_host, g_host, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kSuccess, status);

  // Try with various host interfaces missing.
  status = Cdm::initialize(
      Cdm::kNoSecureOutput, working_client_info,
      NULL, g_host, g_host, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kTypeError, status);

  status = Cdm::initialize(
      Cdm::kNoSecureOutput, working_client_info,
      g_host, NULL, g_host, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kTypeError, status);

  status = Cdm::initialize(
      Cdm::kNoSecureOutput, working_client_info,
      g_host, g_host, NULL, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kTypeError, status);

  status = Cdm::initialize(
      Cdm::kNoSecureOutput, working_client_info,
      g_host, g_host, g_host, NULL,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kTypeError, status);

  // One last init with everything correct and working.
  status = Cdm::initialize(
      Cdm::kNoSecureOutput, working_client_info,
      g_host, g_host, g_host, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));
  EXPECT_EQ(Cdm::kSuccess, status);
}

TEST_F(CdmTest, DeviceCertificateRequest) {
  uint32_t nonce = 0;
  uint8_t buffer[1];
  size_t size = 0;
  ASSERT_EQ(OEMCrypto_SUCCESS, OEMCrypto_Initialize());
  int result = OEMCrypto_RewrapDeviceRSAKey(
      0, buffer, 0, buffer, 0, &nonce, buffer, 0, buffer, buffer, &size);
  ASSERT_EQ(OEMCrypto_SUCCESS, OEMCrypto_Terminate());
  if (result == OEMCrypto_ERROR_NOT_IMPLEMENTED) {
    LOGW("WARNING: Skipping DeviceCertificateRequest because the device does "
         "not support provisioning. If you are using a baked-in certificate, "
         "this is expected. Otherwise, something is wrong.");
    return;
  }

  // Clear any existing certificates.
  g_host->remove("cert.bin");

  // Reinit the library without a device cert.
  Cdm::DeviceCertificateRequest cert_request;
  Cdm::Status status = Cdm::initialize(
      Cdm::kNoSecureOutput, PropertiesCE::GetClientInfo(),
      g_host, g_host, g_host, &cert_request,
      static_cast<Cdm::LogLevel>(g_cutoff));

  // This should succeed, but indicate that we need a cert still.
  EXPECT_EQ(Cdm::kSuccess, status);
  EXPECT_TRUE(cert_request.needed);
  EXPECT_FALSE(cert_request.url.empty());

  // Creating a session should fail.
  std::string session_id;
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  EXPECT_EQ(Cdm::kNeedsDeviceCertificate, status);

  // Loading a session should fail.
  status = cdm_->load(kBogusSessionId);
  EXPECT_EQ(Cdm::kNeedsDeviceCertificate, status);

  // We should be able to provision.
  std::string reply;
  ASSERT_NO_FATAL_FAILURE(FetchCertificate(cert_request.url, &reply));
  status = cert_request.acceptReply(reply);
  ASSERT_EQ(Cdm::kSuccess, status);

  // We should now be able to create a session.
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  EXPECT_EQ(Cdm::kSuccess, status);
}

TEST_F(CdmTest, SetServerCertificate) {
  // Can't set a server certificate if privacy mode is disabled.
  ASSERT_NO_FATAL_FAILURE(RecreateCdm(false /* privacy_mode */));
  Cdm::Status status = cdm_->setServerCertificate(kDefaultServerCertificate);
  EXPECT_EQ(Cdm::kNotSupported, status);

  // Can set a server certificate if privacy mode is enabled.
  ASSERT_NO_FATAL_FAILURE(RecreateCdm(true /* privacy_mode */));
  status = cdm_->setServerCertificate(kDefaultServerCertificate);
  EXPECT_EQ(Cdm::kSuccess, status);

  // It is invalid to set an empty cert.
  status = cdm_->setServerCertificate("");
  EXPECT_EQ(Cdm::kTypeError, status);

  // It is invalid to set a malformed cert.
  status = cdm_->setServerCertificate("asdf");
  EXPECT_EQ(Cdm::kTypeError, status);
}

TEST_F(CdmTest, CreateSession) {
  // Create a temporary session.
  std::string session_id;
  Cdm::Status status = cdm_->createSession(Cdm::kTemporary, &session_id);
  EXPECT_EQ(Cdm::kSuccess, status);
  EXPECT_FALSE(session_id.empty());

  // Create another using the same pointer to an already-filled-out string,
  // and expect the session ID to change.
  std::string original_session_id = session_id;
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  EXPECT_EQ(Cdm::kSuccess, status);
  EXPECT_NE(original_session_id, session_id);

  // Create a persistent session.
  status = cdm_->createSession(Cdm::kPersistentLicense, &session_id);
  EXPECT_EQ(Cdm::kSuccess, status);

  // Try a NULL pointer for session ID.
  status = cdm_->createSession(Cdm::kTemporary, NULL);
  EXPECT_EQ(Cdm::kTypeError, status);

  // Try a bogus session type.
  status = cdm_->createSession(kBogusSessionType, &session_id);
  EXPECT_EQ(Cdm::kNotSupported, status);
}

TEST_F(CdmTest, GenerateRequest) {
  std::string session_id;
  Cdm::Status status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  // Generate a license request for CENC.
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _));
  status = cdm_->generateRequest(session_id, Cdm::kCenc, kCencInitData);
  EXPECT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Can't call generateRequest more than once on a session.
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).Times(0);
  status = cdm_->generateRequest(session_id, Cdm::kCenc, kCencInitData);
  EXPECT_EQ(Cdm::kInvalidState, status);
  Mock::VerifyAndClear(this);

  // Create a new session and generate a license request for WebM.
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _));
  status = cdm_->generateRequest(session_id, Cdm::kWebM, kWebMInitData);
  EXPECT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Create a new session and try the as-yet-unsupported key-ids format.
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).Times(0);
  status = cdm_->generateRequest(session_id, Cdm::kKeyIds, kKeyIdsInitData);
  EXPECT_EQ(Cdm::kNotSupported, status);
  Mock::VerifyAndClear(this);

  // Create a new session and try a bogus init data type.
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).Times(0);
  status = cdm_->generateRequest(session_id, kBogusInitDataType, "asdf");
  EXPECT_EQ(Cdm::kTypeError, status);
  Mock::VerifyAndClear(this);

  // This same session should still be usable with a supported init data type
  // after failing with an unsupported or bogus type.
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _));
  status = cdm_->generateRequest(session_id, Cdm::kCenc, kCencInitData);
  EXPECT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Create a new session and try to pass empty init data.
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).Times(0);
  status = cdm_->generateRequest(session_id, Cdm::kCenc, "");
  EXPECT_EQ(Cdm::kTypeError, status);
  Mock::VerifyAndClear(this);

  // Try to pass invalid CENC init data.
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).Times(0);
  status = cdm_->generateRequest(session_id, Cdm::kCenc, kInvalidCencInitData);
  EXPECT_EQ(Cdm::kNotSupported, status);
  Mock::VerifyAndClear(this);

  // Try to pass non-Widevine CENC init data.
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).Times(0);
  status = cdm_->generateRequest(session_id, Cdm::kCenc,
                                 kNonWidevineCencInitData);
  EXPECT_EQ(Cdm::kNotSupported, status);
  Mock::VerifyAndClear(this);

  // Try a bogus session ID.
  EXPECT_CALL(*this, onMessage(_, _, _)).Times(0);
  status = cdm_->generateRequest(kBogusSessionId, Cdm::kCenc, kCencInitData);
  EXPECT_EQ(Cdm::kSessionNotFound, status);
  Mock::VerifyAndClear(this);
}

TEST_F(CdmTest, Update) {
  std::string session_id;
  std::string message;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndGenerateRequest(
      Cdm::kTemporary, &session_id, &message));

  // Acquire a license.
  std::string response;
  ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

  // Update the session.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  Cdm::Status status = cdm_->update(session_id, response);
  EXPECT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Try updating a bogus session ID.
  status = cdm_->update(kBogusSessionId, response);
  EXPECT_EQ(Cdm::kSessionNotFound, status);

  // Try updating with an empty response.
  status = cdm_->update(session_id, "");
  EXPECT_EQ(Cdm::kTypeError, status);

  // Create a new session and try updating before generating a request.
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kInvalidState, status);
}

TEST_F(CdmTest, Close) {
  // Create a temporary session.
  std::string session_id;
  Cdm::Status status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  // Close it.
  status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  // Can't generate a license request after close.
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).Times(0);
  status = cdm_->generateRequest(session_id, Cdm::kCenc, kCencInitData);
  EXPECT_EQ(Cdm::kSessionNotFound, status);
  Mock::VerifyAndClear(this);

  // Try to close the same session again.
  status = cdm_->close(session_id);
  EXPECT_EQ(Cdm::kSessionNotFound, status);

  // Try to close a bogus session.
  status = cdm_->close(kBogusSessionId);
  EXPECT_EQ(Cdm::kSessionNotFound, status);
}

TEST_F(CdmTest, LoadTemporary) {
  std::string session_id;
  std::string response;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndFetchLicense(
      Cdm::kTemporary, &session_id, &response));

  // Update the temporary session.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  Cdm::Status status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Close the session.
  status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  // Can't load a temporary session.
  status = cdm_->load(session_id);
  EXPECT_EQ(Cdm::kSessionNotFound, status);
}

TEST_F(CdmTest, LoadPersistent) {
  std::string session_id;
  std::string response;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndFetchLicense(
      Cdm::kPersistentLicense, &session_id, &response));

  // Update the persistent session.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  Cdm::Status status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Should be able to load the session again after closing it.
  status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  status = cdm_->load(session_id);
  EXPECT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Should be able to load the session again after recreating the CDM.
  ASSERT_NO_FATAL_FAILURE(RecreateCdm(true /* privacy_mode */));
  ASSERT_NO_FATAL_FAILURE(SetDefaultServerCertificate());
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  status = cdm_->load(session_id);
  EXPECT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Should not be able to load the session again clearing storage.
  status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  g_host->Reset();
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(0);
  status = cdm_->load(session_id);
  EXPECT_EQ(Cdm::kSessionNotFound, status);
  Mock::VerifyAndClear(this);
}

TEST_F(CdmTest, LoadUsageRecord) {
  std::string session_id;
  std::string response;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndFetchLicense(
      Cdm::kPersistentUsageRecord, &session_id, &response));

  // Update the session.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  Cdm::Status status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Should be able to load the session again after closing it.
  status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  // There should be no usable keys after loading this session.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(0);
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRelease, _));
  status = cdm_->load(session_id);
  EXPECT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Should be able to load the session again after recreating the CDM.
  ASSERT_NO_FATAL_FAILURE(RecreateCdm(true /* privacy_mode */));
  ASSERT_NO_FATAL_FAILURE(SetDefaultServerCertificate());
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(0);
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRelease, _));
  status = cdm_->load(session_id);
  EXPECT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Should not be able to load the session again clearing storage.
  status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  g_host->Reset();
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(0);
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRelease, _)).Times(0);
  status = cdm_->load(session_id);
  EXPECT_EQ(Cdm::kSessionNotFound, status);
  Mock::VerifyAndClear(this);
}

TEST_F(CdmTest, LoadBogus) {
  EXPECT_CALL(*this, onKeyStatusesChange(_)).Times(0);
  Cdm::Status status = cdm_->load(kBogusSessionId);
  EXPECT_EQ(Cdm::kSessionNotFound, status);
}

TEST_F(CdmTest, GetKeyStatuses) {
  std::string session_id;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndUpdate(Cdm::kTemporary, &session_id));

  // We should be able to query status and see a usable key.
  Cdm::KeyStatusMap map;
  Cdm::Status status = cdm_->getKeyStatuses(session_id, &map);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_EQ(Cdm::kUsable, map.begin()->second);

  // The key ID should be the one we are expecting.
  const std::string expected_key_id(
      reinterpret_cast<const char*>(kKeyId1.data()), kKeyId1.size());
  EXPECT_EQ(expected_key_id, map.begin()->first);

  // Let the key expire.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRenewal, _)).Times(
      AtLeast(1));
  g_host->ElapseTime(kExpirationTestDelayMs);
  Mock::VerifyAndClear(this);

  // We should see expiration reflected in the map.
  status = cdm_->getKeyStatuses(session_id, &map);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_EQ(Cdm::kExpired, map.begin()->second);

  // We can't get status after closing a session.
  status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  status = cdm_->getKeyStatuses(session_id, &map);
  ASSERT_EQ(Cdm::kSessionNotFound, status);
}

TEST_F(CdmTest, GetExpiration) {
  std::string session_id;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndUpdate(Cdm::kTemporary, &session_id));

  // We should be able to query expiration and get a value in the future.
  int64_t expiration;
  Cdm::Status status = cdm_->getExpiration(session_id, &expiration);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_GT(expiration, g_host->now());
  int64_t original_expiration = expiration;

  // Let the key expire.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRenewal, _)).Times(
      AtLeast(1));
  g_host->ElapseTime(kExpirationTestDelayMs);
  Mock::VerifyAndClear(this);

  // We should see expiration in the past now.
  status = cdm_->getExpiration(session_id, &expiration);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_LE(expiration, g_host->now());
  // Expiration should not have changed.
  EXPECT_EQ(original_expiration, expiration);

  // We can't get expiration after closing a session.
  status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  status = cdm_->getExpiration(session_id, &expiration);
  ASSERT_EQ(Cdm::kSessionNotFound, status);
}

TEST_F(CdmTest, Remove) {
  std::string session_id;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndUpdate(
      Cdm::kPersistentLicense, &session_id));

  // Remove the session.  This causes a release message to be generated.
  std::string message;
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRelease, _)).WillOnce(
      SaveArg<2>(&message));
  Cdm::Status status = cdm_->remove(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // The keys should already be unusable.
  Cdm::KeyStatusMap map;
  status = cdm_->getKeyStatuses(session_id, &map);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_EQ(Cdm::kReleased, map.begin()->second);

  // Post the release message to the license server.
  std::string response;
  ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

  // Update the session.
  EXPECT_CALL(*this, onRemoveComplete(session_id));
  status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // The session is now completely gone.
  status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSessionNotFound, status);
  status = cdm_->load(session_id);
  ASSERT_EQ(Cdm::kSessionNotFound, status);

  // Try a bogus session ID.
  status = cdm_->remove(kBogusSessionId);
  EXPECT_EQ(Cdm::kSessionNotFound, status);

  // Try a new session.
  status = cdm_->createSession(Cdm::kPersistentLicense, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  status = cdm_->remove(session_id);
  EXPECT_EQ(Cdm::kInvalidState, status);

  // Try a temporary session.
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndUpdate(Cdm::kTemporary, &session_id));
  status = cdm_->remove(session_id);
  EXPECT_EQ(Cdm::kRangeError, status);
}

TEST_F(CdmTest, RemoveUsageRecord) {
  std::string session_id;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndUpdate(
      Cdm::kPersistentUsageRecord, &session_id));

  // Remove the session.  This causes a release message to be generated.
  std::string message;
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRelease, _)).WillOnce(
      SaveArg<2>(&message));
  Cdm::Status status = cdm_->remove(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // The keys should already be unusable.
  Cdm::KeyStatusMap map;
  status = cdm_->getKeyStatuses(session_id, &map);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_EQ(Cdm::kReleased, map.begin()->second);

  // Post the release message to the license server.
  std::string response;
  ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

  // Update the session.
  EXPECT_CALL(*this, onRemoveComplete(session_id));
  status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // The session is now completely gone.
  status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSessionNotFound, status);
}

TEST_F(CdmTest, RemoveIncomplete) {
  std::string session_id;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndUpdate(
      Cdm::kPersistentLicense, &session_id));

  // Remove the session.  This causes a release message to be generated.
  std::string message;
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRelease, _)).WillOnce(
      SaveArg<2>(&message));
  Cdm::Status status = cdm_->remove(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // The keys should already be unusable, but they should still exist.
  Cdm::KeyStatusMap map;
  status = cdm_->getKeyStatuses(session_id, &map);
  ASSERT_EQ(Cdm::kSuccess, status);
  ASSERT_FALSE(map.empty());
  EXPECT_EQ(Cdm::kReleased, map.begin()->second);

  // Recreate the CDM.
  ASSERT_NO_FATAL_FAILURE(RecreateCdm(true /* privacy_mode */));
  ASSERT_NO_FATAL_FAILURE(SetDefaultServerCertificate());

  // Load the partially removed session, which will immediately generate a
  // release message.
  message.clear();
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(0);
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRelease, _)).WillOnce(
      SaveArg<2>(&message));
  status = cdm_->load(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  ASSERT_FALSE(message.empty());
  Mock::VerifyAndClear(this);

  // This session has no keys.
  status = cdm_->getKeyStatuses(session_id, &map);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_TRUE(map.empty());

  // Post the release message to the license server.
  std::string response;
  ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

  // Update the session.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(0);
  EXPECT_CALL(*this, onRemoveComplete(session_id));
  status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // The session is now completely gone.
  status = cdm_->load(session_id);
  ASSERT_EQ(Cdm::kSessionNotFound, status);
}

TEST_F(CdmTest, RemoveUsageRecordIncomplete) {
  std::string session_id;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndUpdate(
      Cdm::kPersistentUsageRecord, &session_id));

  // Remove the session.  This causes a release message to be generated.
  std::string message;
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRelease, _)).WillOnce(
      SaveArg<2>(&message));
  Cdm::Status status = cdm_->remove(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // The keys should already be unusable, but they should still exist.
  Cdm::KeyStatusMap map;
  status = cdm_->getKeyStatuses(session_id, &map);
  ASSERT_EQ(Cdm::kSuccess, status);
  ASSERT_FALSE(map.empty());
  EXPECT_EQ(Cdm::kReleased, map.begin()->second);

  // Recreate the CDM.
  ASSERT_NO_FATAL_FAILURE(RecreateCdm(true /* privacy_mode */));
  ASSERT_NO_FATAL_FAILURE(SetDefaultServerCertificate());

  // Load the partially removed session, which will immediately generate a
  // release message.
  message.clear();
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(0);
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRelease, _)).WillOnce(
      SaveArg<2>(&message));
  status = cdm_->load(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  ASSERT_FALSE(message.empty());
  Mock::VerifyAndClear(this);

  // This session has no keys.
  status = cdm_->getKeyStatuses(session_id, &map);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_TRUE(map.empty());

  // Post the release message to the license server.
  std::string response;
  ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

  // Update the session.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(0);
  EXPECT_CALL(*this, onRemoveComplete(session_id));
  status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // The session is now completely gone.
  status = cdm_->load(session_id);
  ASSERT_EQ(Cdm::kSessionNotFound, status);
}

TEST_F(CdmTest, RemoveNotLoaded) {
  // Create a persistent session and then close it.
  std::string session_id;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndUpdate(
      Cdm::kPersistentLicense, &session_id));
  Cdm::Status status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  // A session must be loaded before removing it.  Remove only works on active
  // sessions.
  status = cdm_->remove(session_id);
  EXPECT_EQ(Cdm::kSessionNotFound, status);
}

TEST_F(CdmTest, DecryptClear) {
  Cdm::InputBuffer input;
  Cdm::OutputBuffer output;

  input.key_id = kKeyId1.data();
  input.key_id_length = kKeyId1.size();
  input.iv = kIv1.data();
  input.iv_length = kIv1.size();
  input.data = kInput1.data();
  input.data_length = kInput1.size();
  input.is_encrypted = true;

  std::vector<uint8_t> output_buffer(input.data_length);
  output.data = &(output_buffer[0]);
  output.data_length = output_buffer.size();

  // Decrypt without keys loaded should fail.
  Cdm::Status status = cdm_->decrypt(input, output);
  ASSERT_EQ(Cdm::kNoKey, status);

  // Create a session with the right keys.
  std::string session_id;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndUpdate(Cdm::kTemporary, &session_id));

  // Decrypt should now succeed.
  status = cdm_->decrypt(input, output);
  ASSERT_EQ(Cdm::kSuccess, status);
  EXPECT_EQ(kOutput1, output_buffer);
}
// TODO: add infrastructure to test secure buffer decrypt for some platforms

TEST_F(CdmTest, RequestPersistentLicenseWithWrongInitData) {
  // Generate a request for a persistent license without using the correct
  // persistent content init data.
  std::string session_id;
  Cdm::Status status = cdm_->createSession(Cdm::kPersistentLicense,
                                           &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  std::string message;
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).WillOnce(
      SaveArg<2>(&message));
  status = cdm_->generateRequest(session_id, Cdm::kCenc, kCencInitData);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // The license server will reject this.
  FetchLicenseFailure(message, 500);
}

TEST_F(CdmTest, RequestTemporaryLicenseWithWrongInitData) {
  // Generate a request for a temporary license using persistent init data.
  std::string session_id;
  Cdm::Status status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  std::string message;
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).WillOnce(
      SaveArg<2>(&message));
  status = cdm_->generateRequest(session_id, Cdm::kCenc,
                                 kCencPersistentInitData);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Acquire a license.
  std::string response;
  ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

  // This license should not be accepted.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id));
  status = cdm_->update(session_id, response);
  EXPECT_EQ(Cdm::kRangeError, status);
  Mock::VerifyAndClear(this);
}

TEST_F(CdmTest, Renewal) {
  std::string session_id;
  ASSERT_NO_FATAL_FAILURE(CreateSessionAndUpdate(Cdm::kTemporary, &session_id));

  // We should have a timer.
  EXPECT_NE(0, g_host->NumTimers());

  // When we elapse time, we should get a renewal message.
  std::string message;
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRenewal, _)).WillOnce(
      SaveArg<2>(&message));
  g_host->ElapseTime(kRenewalTestDelayMs);
  Mock::VerifyAndClear(this);
  ASSERT_FALSE(message.empty());  // Stop the test if no message came through.

  // When should still have a timer.
  EXPECT_NE(0, g_host->NumTimers());

  // We should be able to update the session.
  ASSERT_NO_FATAL_FAILURE(FetchLicenseAndUpdate(session_id, message));

  // After closing the session, there should be no more renewals.
  Cdm::Status status = cdm_->close(session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRenewal, _)).Times(0);
  g_host->ElapseTime(kRenewalTestDelayMs * 10);
  Mock::VerifyAndClear(this);
}

TEST_F(CdmTest, ServerCertificateProvisioning) {
  // Do not set a server cert.  Provisioning will be required.
  ASSERT_NO_FATAL_FAILURE(RecreateCdm(true /* privacy_mode */));

  // Create a session.
  std::string session_id;
  Cdm::Status status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  // Expect a license request type message, but this is actually a server cert
  // provisioning request.
  std::string message;
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).
      WillOnce(SaveArg<2>(&message));
  status = cdm_->generateRequest(session_id, Cdm::kCenc, kCencInitData);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Relay it to the server.
  std::string response;
  ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

  // No keys will change, since this wasn't a license.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(0);
  // We should get another license request generated during update.
  message.clear();
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).WillOnce(
      SaveArg<2>(&message));
  status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // There are no keys yet.
  Cdm::KeyStatusMap map;
  status = cdm_->getKeyStatuses(session_id, &map);
  ASSERT_EQ(Cdm::kSuccess, status);
  ASSERT_TRUE(map.empty());

  // Relay the license request to the server.
  ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

  // Update the session.  The keys will change now.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(1);
  status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // The keys should be usable.
  status = cdm_->getKeyStatuses(session_id, &map);
  ASSERT_EQ(Cdm::kSuccess, status);
  ASSERT_FALSE(map.empty());
  EXPECT_EQ(Cdm::kUsable, map.begin()->second);


  // Create another session.  This one should not require server certificate
  // provisioning.
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  // Expect a license request.
  message.clear();
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).WillOnce(
      SaveArg<2>(&message));
  status = cdm_->generateRequest(session_id, Cdm::kCenc, kCencInitData);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Relay it to the server.
  ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

  // Keys will change, since this was an actual license.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(1);
  status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);


  // Create a second CDM instance.
  scoped_ptr<Cdm> cdm2;
  CreateAdditionalCdm(true /* privacy_mode */, &cdm2);

  // Create a session on the second CDM instance.  This one should require
  // provisioning, since provisioned certs should not be shared across CDM
  // instances.
  status = cdm2->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  message.clear();
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).WillOnce(
      SaveArg<2>(&message));
  status = cdm2->generateRequest(session_id, Cdm::kCenc, kCencInitData);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Relay it to the server.
  ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

  // No keys will change, since this wasn't a license.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(0);
  // We should get another license request generated during update.
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _));
  status = cdm2->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);


  // Create another session on the first CDM.  This one should not require
  // server certificate provisioning.  This proves that the creation of the
  // second CDM instance did not affect the state of the first.
  status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);
  message.clear();
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).WillOnce(
      SaveArg<2>(&message));
  status = cdm_->generateRequest(session_id, Cdm::kCenc, kCencInitData);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);

  // Relay it to the server.
  ASSERT_NO_FATAL_FAILURE(FetchLicense(message, &response));

  // Keys will change, since this was an actual license.
  EXPECT_CALL(*this, onKeyStatusesChange(session_id)).Times(1);
  status = cdm_->update(session_id, response);
  ASSERT_EQ(Cdm::kSuccess, status);
  Mock::VerifyAndClear(this);
}

TEST_F(CdmTest, SetAppParameters) {
  // Must use privacy_mode = false to ensure that the message is in plain-text.
  std::string session_id;
  ASSERT_NO_FATAL_FAILURE(RecreateCdm(false /* privacy_mode */));
  Cdm::Status status = cdm_->createSession(Cdm::kTemporary, &session_id);
  ASSERT_EQ(Cdm::kSuccess, status);

  // Set a new app parameter, and check by getting.
  std::string result;
  status = cdm_->setAppParameter(kParamName, kValue);
  ASSERT_EQ(Cdm::kSuccess, status);
  status = cdm_->getAppParameter(kParamName, &result);
  ASSERT_EQ(Cdm::kSuccess, status);
  ASSERT_EQ(kValue, result);

  // Try to get using a null result.
  status = cdm_->getAppParameter(kParamName, NULL);
  ASSERT_EQ(Cdm::kTypeError, status);

  // Try to get using an empty key.
  status = cdm_->getAppParameter("", &result);
  ASSERT_EQ(Cdm::kTypeError, status);

  // Try to set using an empty key.
  status = cdm_->setAppParameter("", kValue);
  ASSERT_EQ(Cdm::kTypeError, status);

  // Try to remove using an empty key.
  status = cdm_->removeAppParameter("");
  ASSERT_EQ(Cdm::kTypeError, status);

  // Change an existing app parameter.
  status = cdm_->setAppParameter(kParamName, kNewValue);
  ASSERT_EQ(Cdm::kSuccess, status);
  status = cdm_->getAppParameter(kParamName, &result);
  ASSERT_EQ(Cdm::kSuccess, status);
  ASSERT_EQ(kNewValue, result);

  // Remove an existing app parameter, check for invalid access when it's gone.
  status = cdm_->removeAppParameter(kParamName);
  ASSERT_EQ(Cdm::kSuccess, status);
  status = cdm_->getAppParameter(kParamName, &result);
  ASSERT_EQ(Cdm::kTypeError, status);

  // Try to remove an absent value.
  status = cdm_->removeAppParameter(kParamName2);
  ASSERT_EQ(Cdm::kTypeError, status);

  // Set some values to check for.
  status = cdm_->setAppParameter(kParamName, kValue);
  ASSERT_EQ(Cdm::kSuccess, status);
  status = cdm_->setAppParameter(kParamName2, kNewValue);
  ASSERT_EQ(Cdm::kSuccess, status);

  // Send a generate request to ensure the parameter is in the message.
  std::string message;
  EXPECT_CALL(*this, onMessage(session_id, Cdm::kLicenseRequest, _)).WillOnce(
      SaveArg<2>(&message));
  status = cdm_->generateRequest(session_id, Cdm::kCenc, kCencInitData);
  EXPECT_EQ(Cdm::kSuccess, status);
  EXPECT_TRUE(!message.empty() && message.find(kValue) != std::string::npos);
  Mock::VerifyAndClear(this);

  // Ensure that the value is still present and correct.
  status = cdm_->getAppParameter(kParamName, &result);
  ASSERT_EQ(Cdm::kSuccess, status);
  ASSERT_EQ(kValue, result);
  status = cdm_->getAppParameter(kParamName2, &result);
  ASSERT_EQ(Cdm::kSuccess, status);
  ASSERT_EQ(kNewValue, result);

  // Clear all the parameters.
  status = cdm_->clearAppParameters();
  ASSERT_EQ(Cdm::kSuccess, status);
  status = cdm_->getAppParameter(kParamName, &result);
  ASSERT_EQ(Cdm::kTypeError, status);
  status = cdm_->getAppParameter(kParamName2, &result);
  ASSERT_EQ(Cdm::kTypeError, status);
}

}  // namespace widevine
