#include "PaymentRequirements.h"
#include "nlohmann/json.hpp"
#include <boost/test/unit_test.hpp>

using json = nlohmann::json;

BOOST_AUTO_TEST_CASE(deserialize_payment_requirements) {
    const std::string json_string = R"(
{
    "paymentRequirements": {
        "scheme": "exact",
        "network": "base-sepolia",
        "maxAmountRequired": "1000",
        "resource": "https://api.example.com/premium/data",
        "description": "Test API data",
        "mimeType": "application/json",
        "payTo": "0x2222222222222222222222222222222222222222",
        "maxTimeoutSeconds": 10,
        "asset": "0x036CbD53842c5426634e7929541eC2318f3dCF7e"
    }
}
)";

    json j_data = json::parse(json_string);
    json j_req = j_data["paymentRequirements"];
    PaymentRequirements requirements = j_req.get<PaymentRequirements>();

    BOOST_TEST(requirements.scheme == "exact");
    BOOST_TEST(requirements.network == "base-sepolia");
    BOOST_TEST(requirements.maxAmountRequired == "1000");
    BOOST_TEST(requirements.resource == "https://api.example.com/premium/data");
    BOOST_TEST(requirements.description == "Test API data");
    BOOST_TEST(requirements.mimeType == "application/json");
    BOOST_TEST(requirements.payTo == "0x2222222222222222222222222222222222222222");
    BOOST_TEST(requirements.maxTimeoutSeconds == 10);
    BOOST_TEST(requirements.asset == "0x036CbD53842c5426634e7929541eC2318f3dCF7e");
}

BOOST_AUTO_TEST_CASE(serialize_payment_requirements) {
    PaymentRequirements new_req;
    new_req.scheme = "streaming";
    new_req.network = "base";
    new_req.maxAmountRequired = "50000000000000000";
    new_req.resource = "https://api.prod.com/stream/video";
    new_req.description = "Production Video Stream";
    new_req.mimeType = "video/mp4";
    new_req.payTo = "0xAAAAAAAAAAAAAA...AAAAAAAAAAAAA";
    new_req.maxTimeoutSeconds = 600;
    new_req.asset = "0x833589fCD6eDb6E08f4c7C32D4f71b54bdA02913";

    json j_output;
    j_output["paymentRequirements"] = new_req;

    BOOST_TEST(j_output["paymentRequirements"]["scheme"] == "streaming");
    BOOST_TEST(j_output["paymentRequirements"]["network"] == "base");
    BOOST_TEST(j_output["paymentRequirements"]["maxAmountRequired"] == "50000000000000000");
    BOOST_TEST(j_output["paymentRequirements"]["resource"] == "https://api.prod.com/stream/video");
    BOOST_TEST(j_output["paymentRequirements"]["description"] == "Production Video Stream");
    BOOST_TEST(j_output["paymentRequirements"]["mimeType"] == "video/mp4");
    BOOST_TEST(j_output["paymentRequirements"]["payTo"] == "0xAAAAAAAAAAAAAA...AAAAAAAAAAAAA");
    BOOST_TEST(j_output["paymentRequirements"]["maxTimeoutSeconds"] == 600);
    BOOST_TEST(j_output["paymentRequirements"]["asset"] == "0x833589fCD6eDb6E08f4c7C32D4f71b54bdA02913");
}