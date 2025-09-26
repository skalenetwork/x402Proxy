#pragma once

const std::string EXACT_UCDC_PAYMENT_REQ_CB_SEPOLIA = R"(
{
        "scheme": "exact",
        "network": "base-sepolia",
        "maxAmountRequired": "1000",
        "resource": "https://api.example.com/premium/data",
        "description": "Test API data",
        "mimeType": "application/json",
        "payTo": "0x2222222222222222222222222222222222222222",
        "maxTimeoutSeconds": 10,
        "asset": "0x036CbD53842c5426634e7929541eC2318f3dCF7e",
        "extra": {
            "name": "USDC",
            "version": "2"
        }
}
)";


const std::string EXACT_UCDC_PAYMENT_PAYLOAD_CB_SEPOLIA = R"(
{
    "paymentPayload": {
        "x402Version": 1,
        "scheme": "exact",
        "network": "base-sepolia",
        "payload": {
            "signature": "0xdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef1b",
            "authorization": {
                "from": "0x1111111111111111111111111111111111111111",
                "to": "0x2222222222222222222222222222222222222222",
                "value": "1000",
                "validAfter": "1716150000",
                "validBefore": "1716153600",
                "nonce": "0x1234567890abcdef"
            }
        }
    }
}
)";