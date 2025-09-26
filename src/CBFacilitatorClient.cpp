#include "common.h"
#include "CBFacilitatorClient.h"

#include <stdexcept>
#include <mutex>
#include <curl/curl.h>

CBFacilitatorClient::CBFacilitatorClient(
    std::string base_url,
    std::string auth,
    long connect_timeout_ms,
    long total_timeout_ms)
    : base_url_(std::move(base_url)),
      auth_header_value_(std::move(auth)),
      connect_timeout_ms_(connect_timeout_ms),
      total_timeout_ms_(total_timeout_ms) {
    ensureCurlGlobalInit();
}

void CBFacilitatorClient::ensureCurlGlobalInit() {
    static std::once_flag once;
    std::call_once(once, []() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    });
}

size_t CBFacilitatorClient::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    const size_t real_size = size * nmemb;
    auto* buf = static_cast<std::string*>(userdata);
    buf->append(ptr, real_size);
    return real_size;
}

std::string CBFacilitatorClient::joinUrl(const std::string& base, const std::string& path) {
    if (base.empty()) return path;
    if (path.empty()) return base;
    const bool b = base.back() == '/';
    const bool p = path.front() == '/';
    if (b && p)   return base + path.substr(1);
    if (!b && !p) return base + "/" + path;
    return base + path;
}
const std::string VERIFY_PAYLOAD_EXAMPLE = R"JSON(
{
    "x402Version": 1,
    "paymentPayload": {
        "x402Version": 1,
        "scheme": "exact",
        "network": "base-sepolia",
        "payload": {
            "signature": "0xdeadbeef...",
            "authorization": {
                "from": "0x1111111111111111111111111111111111111111",
                "to": "0x2222222222222222222222222222222222222222",
                "value": "1000",
                "validAfter": "1716150000",
                "validBefore": "1716153600",
                "nonce": "0x1234567890abcdef"
            }
        }
    },
    "paymentRequirements": {
        "scheme": "exact",
        "network": "base-sepolia",
        "maxAmountRequired": "1000",
        "resource": "https://api.example.com/premium/data",
        "description": "Test API data",
        "mimeType": "application/json",
        "payTo": "0x2222222222222222222222222222222222222222",
        "maxTimeoutSeconds": 10,
        "asset": "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48"
    }
}
)JSON";





nlohmann::json CBFacilitatorClient::verify(const nlohmann::json& paymentInstruction,
                                           const nlohmann::json& paymentPayload) const {
    nlohmann::json body;
    body["paymentInstruction"] = paymentInstruction;
    body["paymentPayload"]     = paymentPayload;
    return postJson("/verify", body);
}

nlohmann::json CBFacilitatorClient::settle(const nlohmann::json& paymentInstruction,
                                           const nlohmann::json& paymentPayload) const {
    nlohmann::json body;
    body["paymentInstruction"] = paymentInstruction;
    body["paymentPayload"]     = paymentPayload;
    return postJson("/settle", body);
}

nlohmann::json CBFacilitatorClient::postJson(const std::string& path, const nlohmann::json& body) const {
    const std::string url = joinUrl(base_url_, path);
    std::string payload = body.dump();

    payload = VERIFY_PAYLOAD_EXAMPLE;

    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init CURL easy handle");

    std::string response_data;
    long http_code = 0;

    // Build headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!auth_header_value_.empty()) {
        std::string auth = "Authorization: " + auth_header_value_;
        headers = curl_slist_append(headers, auth.c_str());
    }
    for (const auto& h : extra_headers_) {
        headers = curl_slist_append(headers, h.c_str());
    }

    // Set options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.size());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CBFacilitatorClient/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CBFacilitatorClient::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connect_timeout_ms_);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, total_timeout_ms_);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (!proxy_url_.empty()) {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy_url_.c_str());
    }

    // Perform
    CURLcode res = curl_easy_perform(curl);

    // Collect HTTP code before cleanup
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    // Cleanup
    if (headers) curl_slist_free_all(headers);
    curl_easy_cleanup(curl);


    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("CURL error: ") + curl_easy_strerror(res));
    }


    if (http_code < 200 || http_code >= 300) {
        std::string error_explanation;
        switch (http_code) {
        case 400: error_explanation = "Bad Request"; break;
        case 401: error_explanation = "Unauthorized"; break;
        case 403: error_explanation = "Forbidden"; break;
        case 404: error_explanation = "Not Found"; break;
        case 500: error_explanation = "Internal Server Error"; break;
        case 502: error_explanation = "Bad Gateway"; break;
        case 503: error_explanation = "Service Unavailable"; break;
        default:  error_explanation = "Unknown Error"; break;
        }
        throw std::runtime_error("HTTP " + std::to_string(http_code) + " (" + error_explanation + ") error at "
            + url + ": " + response_data + "\n | Payload: " + payload);
    }

    if (response_data.empty()) {
        throw std::runtime_error("HTTP server at " + url + " returned empty response (HTTP " +
            std::to_string(http_code) + ")");
    }

    // Parse response
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(response_data);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse JSON (HTTP " + std::to_string(http_code) +
                                 ") from " + url + ": " + std::string(e.what()) + " | Raw: " + response_data);
    }



    return j;
}
