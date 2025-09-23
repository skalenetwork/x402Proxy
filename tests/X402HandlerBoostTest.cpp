// X402HandlerBoostTest.cpp
#define BOOST_TEST_MODULE X402HandlerSelfTest

#include "../src/common.h"

#include <boost/test/included/unit_test.hpp> // or <boost/test/unit_test.hpp> if using dynamic link

#include <folly/init/Init.h>
#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <proxygen/httpserver/HTTPServer.h>

#include "ServerFactory.h"

// ---- libcurl helper ---------------------------------------------------------
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

const std::string CONNECT_IP = "127.0.0.1";

namespace {
    size_t writeBody(char *ptr, size_t size, size_t nmemb, void *userdata) {
        auto *out = static_cast<std::string *>(userdata);
        out->append(ptr, size * nmemb);
        return size * nmemb;
    }

    size_t writeHeader(char *buffer, size_t size, size_t nitems, void *userdata) {
        auto *headers = static_cast<std::vector<std::string> *>(userdata);
        headers->emplace_back(buffer, size * nitems);
        return size * nitems;
    }

    struct HttpResponse {
        long status = 0;
        std::string body;
        std::vector<std::string> headers;
    };

    HttpResponse httpGet(const std::string &_baseURL, const std::string &_location,
                         const std::vector<std::string> &extraHeaders) {
        CURL *curl = curl_easy_init();
        if (!curl) throw std::runtime_error("curl_easy_init failed");

        std::string url = _baseURL + ":" + std::to_string(DEFAULT_TEST_PORT) + "/" + _location;

        struct curl_slist *hdrs = nullptr;
        for (auto &h: extraHeaders) hdrs = curl_slist_append(hdrs, h.c_str());

        HttpResponse resp;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBody);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp.body);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeader);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp.headers);
        if (hdrs)
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);

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
        int argc = 1;
        char *arg_array[] = {const_cast<char *>("X402HandlerBoostTest"), nullptr};
        char **argv = arg_array;
        FLAGS_logtostderr = 1;
        static folly::Init follyInit(&argc, &argv, {});

        server_ = ServerFactory().createServerInstance("127.0.0.1", DEFAULT_TEST_PORT);

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
        return "http://" + CONNECT_IP;
    }

    //return status line and parse headers into map
    std::string parseStatusLineAndHeaders(const std::vector<std::string> &headersVector,
                                          std::map<std::string, std::string> &headersMap) {
        std::string statusLine;
        BOOST_TEST(headersVector.size() > 0); // At least status line must be present
        if (!headersVector.empty()) {
            statusLine = headersVector[0];
            statusLine.erase(statusLine.find_last_not_of(" \t\r\n") + 1);
        }
        for (size_t i = 1; i < headersVector.size(); ++i) {
            auto pos = headersVector[i].find(':');
            if (pos != std::string::npos) {
                std::string key = headersVector[i].substr(0, pos);
                std::string value = headersVector[i].substr(pos + 1);
                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                headersMap[key] = value;
            }
        }

        return statusLine;
    }

    std::tuple<std::map<std::string, std::string>, std::string, HttpResponse>
    sendRequestAndParseResult(std::string baseUrl, std::string location, const std::vector<std::string> &extraHeaders) {
        auto resp = httpGet( baseUrl, location, extraHeaders);
        auto headersVector = resp.headers;
        std::map<std::string, std::string> headersMap;
        auto statusLine = parseStatusLineAndHeaders(headersVector, headersMap);
        LOG(INFO) << "STATUS::" << statusLine;
        for (const auto &[key, value]: headersMap) {
            LOG(INFO) << key << ": " << value;
        }
        LOG(INFO) << "BODY::" << resp.body;
        return {headersMap, statusLine, resp};
    }

    std::shared_ptr<proxygen::HTTPServer> server_;
    std::thread srvThread_;
    uint16_t port_{0};
};


// Use the fixture for all tests in this suite
BOOST_FIXTURE_TEST_SUITE(X402Suite, X402ServerFixture)

    BOOST_AUTO_TEST_CASE(Returns402WhenNoPaymentHeader) {
        auto [headersMap, statusLine, resp] = sendRequestAndParseResult( baseUrl(), "paid", {});


        BOOST_TEST(resp.status == 402);
        BOOST_TEST(statusLine == "HTTP/1.1 402 Payment Required");
        BOOST_TEST(headersMap["Content-Type"] == "application/json");

        nlohmann::json bodyJson;

        try {
            bodyJson = nlohmann::json::parse(resp.body, nullptr, true);
        } catch (const nlohmann::json::parse_error &e) {
            BOOST_FAIL("JSON parsing failed: " + std::string(e.what()));
        }

        // Basic body sanity checks
        BOOST_TEST(bodyJson["scheme"] == "exact");
        BOOST_TEST(bodyJson["asset"]["symbol"] == "USDC");
    }

    BOOST_AUTO_TEST_CASE(Returns200WhenPaymentHeaderPresent) {
    auto [headersMap, statusLine, resp] = sendRequestAndParseResult( baseUrl(), "paid",
        {"X-PAYMENT: demo-ok"});
        BOOST_TEST(resp.status == 200);
        auto xPaymentTesponse = headersMap.at("X-PAYMENT-RESPONSE");
        BOOST_TEST(xPaymentTesponse.find("txHash") != std::string::npos);
        BOOST_TEST(resp.body.size() > 0);
    }

BOOST_AUTO_TEST_SUITE_END()
