// X402HandlerBoostTest.cpp
#define BOOST_TEST_MODULE X402HandlerSelfTest
#include <boost/test/included/unit_test.hpp> // or <boost/test/unit_test.hpp> if using dynamic link
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <stdexcept>

#include <folly/init/Init.h>
#include <folly/SocketAddress.h>
#include <proxygen/httpserver/HTTPServer.h>

#include "ServerFactory.h"

// ---- libcurl helper ---------------------------------------------------------
#include <curl/curl.h>

namespace {
size_t writeBody(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto* out = static_cast<std::string*>(userdata);
  out->append(ptr, size * nmemb);
  return size * nmemb;
}
size_t writeHeader(char* buffer, size_t size, size_t nitems, void* userdata) {
  auto* headers = static_cast<std::vector<std::string>*>(userdata);
  headers->emplace_back(buffer, size * nitems);
  return size * nitems;
}

struct HttpResponse {
  long status = 0;
  std::string body;
  std::vector<std::string> headers;
};

HttpResponse httpGet(const std::string& url,
                     const std::vector<std::string>& extraHeaders = {}) {
  CURL* curl = curl_easy_init();
  if (!curl) throw std::runtime_error("curl_easy_init failed");

  struct curl_slist* hdrs = nullptr;
  for (auto& h : extraHeaders) hdrs = curl_slist_append(hdrs, h.c_str());

  HttpResponse resp;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBody);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp.body);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeader);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp.headers);
  if (hdrs) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);

  auto rc = curl_easy_perform(curl);
  if (rc != CURLE_OK) {
    if (hdrs) curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
    throw std::runtime_error(std::string("curl perform error: ") + curl_easy_strerror(rc));
  }
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp.status);

  if (hdrs) curl_slist_free_all(hdrs);
  curl_easy_cleanup(curl);
  return resp;
}
} // namespace

// ---- Test fixture that starts/stops the proxygen server ---------------------
struct X402ServerFixture {
  X402ServerFixture() {
    int argc = 0; char** argv = nullptr;

    FLAGS_logtostderr = 1;
    folly::init(&argc, &argv);

    server_ = ServerFactory().createServerInstance("0.0.0.0", 8080);

    srvThread_ = std::thread([this] {
      server_->start(); //
    });

    // tiny wait to ensure acceptors are ready (bind happened already)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  ~X402ServerFixture() {
    if (server_) server_->stop();
    if (srvThread_.joinable()) srvThread_.join();
  }

  std::string baseUrl() const {
    return "http://0.0.0.0/" + std::to_string( 8080);
  }

  std::shared_ptr<proxygen::HTTPServer> server_;
  std::thread srvThread_;
  uint16_t port_{0};
};

// Use the fixture for all tests in this suite
BOOST_FIXTURE_TEST_SUITE(X402Suite, X402ServerFixture)

BOOST_AUTO_TEST_CASE(Returns402WhenNoPaymentHeader) {
  const auto url = baseUrl() + "/paid";
  auto resp = httpGet(url);

  BOOST_TEST(resp.status == 402);

  // Basic body sanity checks
  BOOST_TEST(resp.body.find("\"scheme\":\"exact\"") != std::string::npos);
  BOOST_TEST(resp.body.find("\"asset\":{\"symbol\":\"USDC\"") != std::string::npos);

  // Check JSON content-type
  bool sawJson = false;
  for (auto& h : resp.headers) {
    if (h.rfind("Content-Type:", 0) == 0 && h.find("application/json") != std::string::npos) {
      sawJson = true; break;
    }
  }
  BOOST_TEST(sawJson);
}

BOOST_AUTO_TEST_CASE(Returns200WhenPaymentHeaderPresent) {
  const auto url = baseUrl() + "/paid";
  auto resp = httpGet(url, {"X-PAYMENT: demo-ok"});

  BOOST_TEST(resp.status == 200);
  BOOST_TEST(resp.body.find("\"ok\":true") != std::string::npos);

  bool sawPaymentResp = false;
  for (auto& h : resp.headers) {
    if (h.rfind("X-PAYMENT-RESPONSE:", 0) == 0) {
      sawPaymentResp = true;
      BOOST_TEST(h.find("txHash") != std::string::npos);
      break;
    }
  }
  BOOST_TEST(sawPaymentResp);
}

BOOST_AUTO_TEST_SUITE_END()
