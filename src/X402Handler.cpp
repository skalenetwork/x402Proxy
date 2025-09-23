#include "X402Handler.h"

#include <folly/Conv.h>
#include <folly/String.h>
#include <folly/json.h>

#include <curl/curl.h>

using namespace proxygen;

namespace {
// Demo payment requirements JSON (normally dynamic / per-request).
folly::dynamic paymentRequirements() {
  folly::dynamic req = folly::dynamic::object;
  req["version"] = 1;
  req["scheme"] = "exact";        // client must pay exact amount
  req["network"] = "base-mainnet"; // demo value
  req["asset"] = folly::dynamic::object
    ("symbol", "USDC")
    ("chainId", 8453)
    ("decimals", 6);
  req["amount"] = "0.25";          // 0.25 USDC
  req["payTo"] = "0x000000000000000000000000000000000000dead"; // demo addr
  req["nonce"] = "demo-nonce-123"; // include in client payload to prevent replay
  req["expiresAt"] = "2099-01-01T00:00:00Z"; // demo
  req["description"] = "Pay-per-call demo: access the /paid resource.";
  return req;
}
}

void X402Handler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
  reqHeaders_ = std::move(headers);
}

void X402Handler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (!body) return;
  body->coalesce();
  bodyBuffer_.append(reinterpret_cast<const char*>(body->data()), body->length());
}

bool X402Handler::hasValidPaymentHeader(const HTTPMessage* req, std::string& paymentInfo) {
  // TODO: Parse and verify real X-PAYMENT header.
  // This stub accepts "demo-ok".
  const auto& headerTable = req->getHeaders();

  std::string payment = headerTable.getSingleOrEmpty("X-PAYMENT");
  if (payment.empty()) return false;

  if (payment == "demo-ok") {
    paymentInfo = R"({"txHash":"0xabc123...","amount":"0.25","asset":"USDC","network":"base-mainnet"})";
    return true;
  }
  return false;
}

void X402Handler::reply402() {
  auto json = folly::toJson(paymentRequirements());
  ResponseBuilder(downstream_)
      .status(402, "Payment Required")
      .header("Content-Type", "application/json")
      .body(json)
      .sendWithEOM();
}

void X402Handler::onEOM() noexcept {
  std::string settlementInfo;
  if (hasValidPaymentHeader(reqHeaders_.get(), settlementInfo)) {
    // Fetch content from the external URL
    CURL* curl = curl_easy_init();
    std::string proxyBody;
    if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/skalenetwork/skaled/refs/heads/develop/dummy.txt");
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
          auto* str = static_cast<std::string*>(userdata);
          str->append(ptr, size * nmemb);
          return size * nmemb;
      });
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &proxyBody);
      curl_easy_perform(curl);
      curl_easy_cleanup(curl);
    }
    ResponseBuilder(downstream_)
        .status(200, "OK")
        .header("Content-Type", "text/plain")
        .header("X-PAYMENT-RESPONSE", settlementInfo)
        .body(proxyBody)
        .sendWithEOM();
    return;
  }
  // Otherwise, tell the client how to pay.
  reply402();
}
