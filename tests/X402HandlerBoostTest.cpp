// X402HandlerBoostTest.cpp
#define BOOST_TEST_MODULE X402HandlerSelfTest

#include "../src/common.h"

#include <boost/test/included/unit_test.hpp> // or <boost/test/unit_test.hpp> if using dynamic link

#include <folly/init/Init.h>
#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <proxygen/httpserver/HTTPServer.h>

#include "ServerFactory.h"
#include "X402Client.h"

// ---- libcurl helper ---------------------------------------------------------
#include <curl/curl.h>
#include <nlohmann/json.hpp>


const std::string CONNECT_IP = "127.0.0.1";

// ---- Test fixture that starts/stops the proxygen server ---------------------
struct X402ServerFixture {
    X402ServerFixture() {
        int argc = 1;
        char *arg_array[] = {const_cast<char *>("X402HandlerBoostTest"), nullptr};
        char **argv = arg_array;
        FLAGS_logtostderr = 1;
        static folly::Init follyInit(&argc, &argv, {});

        server_ = ServerFactory().createServerInstance(CONNECT_IP, DEFAULT_TEST_PORT);

        client_ = std::make_shared<X402Client>(CONNECT_IP, DEFAULT_TEST_PORT);

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


    std::shared_ptr<proxygen::HTTPServer> server_;
    std::shared_ptr<X402Client> client_;

    std::thread srvThread_;
    uint16_t port_{0};
};


// Use the fixture for all tests in this suite
BOOST_FIXTURE_TEST_SUITE(X402Suite, X402ServerFixture)

    BOOST_AUTO_TEST_CASE(Returns402WhenNoPaymentHeader) {
        auto [headersMap, statusLine, resp] = client_->sendRequestAndParseResult(
            "paid", {});


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
    auto [headersMap, statusLine, resp] = client_->sendRequestAndParseResult(  "paid",
        {"X-PAYMENT: demo-ok"});
        BOOST_TEST(resp.status == 200);
        auto xPaymentTesponse = headersMap.at("X-PAYMENT-RESPONSE");
        BOOST_TEST(xPaymentTesponse.find("txHash") != std::string::npos);
        BOOST_TEST(resp.body.size() > 0);
    }

BOOST_AUTO_TEST_SUITE_END()
