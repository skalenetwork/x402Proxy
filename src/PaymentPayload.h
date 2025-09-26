#pragma once


#include <string>
#include "nlohmann/json.hpp" // Ensure you have nlohmann/json.hpp in your environment

// Use the nlohmann::json namespace alias
using json = nlohmann::json;

/**
 * @brief Represents the authorization details within the payment payload.
 *
 * All fields are stored as strings to handle large token values (like 'value')
 * and large timestamps (like 'validAfter').
 */
struct Authorization {
    std::string from;
    std::string to;
    std::string value;
    std::string validAfter;  // Unix timestamp (string)
    std::string validBefore; // Unix timestamp (string)
    std::string nonce;
};

// Define the non-intrusive serialization/deserialization for Authorization
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Authorization, from, to, value, validAfter, validBefore, nonce)

/**
 * @brief Represents the core signed data of the payment payload.
 */
struct Payload {
    std::string signature;
    Authorization authorization; // Nested struct for authorization details
};

// Define the non-intrusive serialization/deserialization for Payload
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Payload, signature, authorization)

/**
 * @brief Represents the full payment payload structure.
 */
struct PaymentPayload {
    int x402Version;
    std::string scheme;
    std::string network;
    Payload payload; // Nested struct for core payload details
};

// Define the non-intrusive serialization/deserialization for PaymentPayload
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PaymentPayload, x402Version, scheme, network, payload)