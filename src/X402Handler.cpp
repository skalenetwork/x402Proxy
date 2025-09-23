#include "common.h"
#include "X402Handler.h"
#include <folly/json.h>
#include <curl/curl.h>

using namespace proxygen;

namespace {
    // Demo payment requirements JSON (normally dynamic / per-request).
    folly::dynamic paymentRequirements() {
        folly::dynamic req = folly::dynamic::object;
        req["version"] = 1;
        req["scheme"] = "exact"; // client must pay exact amount
        req["network"] = "base-mainnet"; // demo value
        req["asset"] = folly::dynamic::object
                ("symbol", "USDC")
                ("chainId", 8453)
                ("decimals", 6);
        req["amount"] = "0.25"; // 0.25 USDC
        req["payTo"] = "0x000000000000000000000000000000000000dead"; // demo addr
        req["nonce"] = "demo-nonce-123"; // include in client payload to prevent replay
        req["expiresAt"] = "2099-01-01T00:00:00Z"; // demo
        req["description"] = "Pay-per-call demo: access the /paid resource.";
        return req;
    }
}

void X402Handler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
    reqHeaders = std::move(headers);
}

void X402Handler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
    if (!body) return;
    body->coalesce();
    bodyBuffer.append(reinterpret_cast<const char *>(body->data()), body->length());
}

bool X402Handler::hasValidPaymentHeader(const HTTPMessage *req, std::string &paymentInfo) {
    // TODO: Parse and verify real X-PAYMENT header.
    // This stub accepts "demo-ok".
    const auto &headerTable = req->getHeaders();

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


void X402Handler::proxyToBackEnd(std::string settlementInfo) {
    static thread_local std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curlThreadLocal(nullptr, &curl_easy_cleanup);

    if (!curlThreadLocal) {
        auto curlObject = curl_easy_init();
        if (!curlObject) {
            LOG(ERROR) << "Could not initialize CURL object";
            ResponseBuilder(downstream_)
                    .status(502, "Bad Gateway")
                    .header("Content-Type", "text/plain")
                    .body("Could not initialize CURL object")
                    .sendWithEOM();
            return;
        }
        curlThreadLocal.reset(curlObject);
    }
    // Fetch content from the external URL
    CHECK_STATE(curlThreadLocal);
    auto* curl = curlThreadLocal.get();
    curl_easy_reset(curl);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    std::string proxyBody;

    curl_easy_setopt(curl, CURLOPT_URL, "https://jsonplaceholder.typicode.com/posts/1");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
                     auto* str = static_cast<std::string*>(userdata);
                     str->append(ptr, size * nmemb);
                     return size * nmemb;
                     });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &proxyBody);

    auto result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        LOG(ERROR) << "CURL error: " << curl_easy_strerror(result);
        // Failed to fetch content, reply with 502
        ResponseBuilder(downstream_)
                .status(502, "Bad Gateway")
                .header("Content-Type", "text/plain")
                .body("Failed to fetch content from upstream service.")
                .sendWithEOM();
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_cleanup(curl);

    ResponseBuilder(downstream_)
            .status(200, "OK")
            .header("Content-Type", "text/plain")
            .header("X-PAYMENT-RESPONSE", settlementInfo)
            .body(proxyBody)
            .sendWithEOM();
    return;
}

void X402Handler::onEOM() noexcept {
    std::string settlementInfo;
    if (hasValidPaymentHeader(reqHeaders.get(), settlementInfo)) {
        proxyToBackEnd(settlementInfo);
    } else {
        reply402();
    }
}
