#pragma once


#include <string>
#include <ostream>
#include "Payload.h"
#include "nlohmann/json.hpp"

// Use the nlohmann::json namespace alias
using json = nlohmann::json;


/**
 * @brief Represents the full payment payload structure.
 */
struct PaymentPayload {
    int x402Version;
    std::string scheme;
    std::string network;
    Payload payload; // Nested struct for core payload details

    bool operator==(const PaymentPayload& other) const {
        return x402Version == other.x402Version &&
               scheme == other.scheme &&
               network == other.network &&
               payload == other.payload;
    }
    friend std::ostream& operator<<(std::ostream& os, const PaymentPayload& p) {
        os << "{x402Version: " << p.x402Version
           << ", scheme: " << p.scheme
           << ", network: " << p.network
           << ", payload: " << p.payload << "}";
        return os;
    }
};

// Define the non-intrusive serialization/deserialization for PaymentPayload
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PaymentPayload, x402Version, scheme, network, payload)