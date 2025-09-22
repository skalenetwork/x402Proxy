#include "X402Handler.h"

#include <folly/Conv.h>
#include <folly/String.h>
#include <folly/json.h>

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
  auto* headerTable = req->getHeaders();
  if (!headerTable) return false;

  std::string payment = headerTable->getSingle("X-PAYMENT");
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
    ResponseBuilder(downstream_)
        .status(200, "OK")
        .header("Content-Type", "application/json")
        .header("X-PAYMENT-RESPONSE", settlementInfo)
        .body(R"({"ok":true,"message":"paid content here"})")
        .sendWithEOM();
    return;
  }
  // Otherwise, tell the client how to pay.
  reply402();
}
