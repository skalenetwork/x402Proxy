// X402HandlerBoostTest.cpp
#define BOOST_TEST_MODULE X402HandlerSelfTest

#include "common.h"

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

#include "../datastructures/PaymentRequirements.h"
#include "../examples/PaymentExamples.h"


const std::string BIND_IP = "0.0.0.0";
const std::string CONNECT_HOST = "localhost";
constexpr uint32_t DEFAULT_TEST_PORT = 8080;


// ---- Test fixture that starts/stops the proxygen server ---------------------
struct X402ServerFixture {
    X402ServerFixture() {
        int argc = 1;
        char *arg_array[] = {const_cast<char *>("X402HandlerBoostTest"), nullptr};
        char **argv = arg_array;
        FLAGS_logtostderr = 1;
        static folly::Init follyInit(&argc, &argv, {});

        server = ServerFactory().createServerInstance(BIND_IP, DEFAULT_TEST_PORT);

        client = std::make_shared<X402Client>(CONNECT_HOST, DEFAULT_TEST_PORT);

        srvThread = std::thread([this] {
            server->start(); //
        });

        // tiny wait to ensure acceptors are ready (bind happened already)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    ~X402ServerFixture() {
        if (server) server->stop();
        if (srvThread.joinable()) srvThread.join();
    }


    std::shared_ptr<proxygen::HTTPServer> server;
    std::shared_ptr<X402Client> client;

    std::thread srvThread;
    uint16_t port{0};
};


// Use the fixture for all tests in this suite
BOOST_FIXTURE_TEST_SUITE(X402Suite, X402ServerFixture)

    BOOST_AUTO_TEST_CASE(Returns402WhenNoPaymentHeader) {
        auto [headersMap, statusLine, resp] = client->sendRequestAndParseResult(
            "paid", {});


        BOOST_TEST(resp.status == 402);
        BOOST_TEST(statusLine == "HTTP/1.1 402 Payment Required");
        BOOST_TEST(headersMap["Content-Type"] == "application/json");


        auto req = nlohmann::json::parse(resp.body).get<PaymentRequirements>();
        auto expected = nlohmann::json::parse(EXACT_UCDC_PAYMENT_REQ_CB_SEPOLIA).get<PaymentRequirements>();
        BOOST_TEST(req == expected);
    }

    BOOST_AUTO_TEST_CASE(Returns200WhenPaymentHeaderPresent) {
    auto [headersMap, statusLine, resp] = client->sendRequestAndParseResult(  "paid",
        {"X-PAYMENT: demo-ok"});
        BOOST_TEST(resp.status == 200);
        auto xPaymentTesponse = headersMap.at("X-PAYMENT-RESPONSE");
        BOOST_TEST(xPaymentTesponse.find("txHash") != std::string::npos);
        BOOST_TEST(resp.body.size() > 0);
    }

BOOST_AUTO_TEST_SUITE_END()
