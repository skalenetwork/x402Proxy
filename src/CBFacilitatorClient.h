#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

/// Minimal client for Coinbase x402 facilitator (REST JSON API).
/// Depends on: libcurl, nlohmann::json
///
///
//        std::string cdp_url = "https://api.cdp.coinbase.com/platform/v2/x402",
//        std::string cdpAuth = "Bearer XYZ", // Replace with real token if needed
///
class CBFacilitatorClient {
public:
    explicit CBFacilitatorClient(
        std::string base_url = "https://x402.org/facilitator/",
        std::string auth = "", // Optional Bearer token if needed
        long connect_timeout_ms = 5000,
        long total_timeout_ms   = 15000);

    // POST /verify — validates the payment payload (no chain call)
    nlohmann::json verify(const nlohmann::json& paymentInstruction,
                          const nlohmann::json& paymentPayload) const;

    // POST /settle — performs the on-chain transfer (gas sponsored by facilitator)
    nlohmann::json settle(const nlohmann::json& paymentInstruction,
                          const nlohmann::json& paymentPayload) const;

private:
    static void ensureCurlGlobalInit();
    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
    static std::string joinUrl(const std::string& base, const std::string& path);

    nlohmann::json postJson(const std::string& path, const nlohmann::json& body) const;

private:
    std::string base_url_;
    std::string auth_header_value_;
    long connect_timeout_ms_;
    long total_timeout_ms_;
    std::string proxy_url_;
    std::vector<std::string> extra_headers_;
};
