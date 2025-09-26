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

    bool operator==(const Authorization& other) const {
        return from == other.from &&
               to == other.to &&
               value == other.value &&
               validAfter == other.validAfter &&
               validBefore == other.validBefore &&
               nonce == other.nonce;
    }
    friend std::ostream& operator<<(std::ostream& os, const Authorization& a) {
        os << "{from: " << a.from
           << ", to: " << a.to
           << ", value: " << a.value
           << ", validAfter: " << a.validAfter
           << ", validBefore: " << a.validBefore
           << ", nonce: " << a.nonce << "}";
        return os;
    }
};

// Define the non-intrusive serialization/deserialization for Authorization
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Authorization, from, to, value, validAfter, validBefore, nonce)

/**
 * @brief Represents the core signed data of the payment payload.
 */
struct Payload {
    std::string signature;
    Authorization authorization; // Nested struct for authorization details

    bool operator==(const Payload& other) const {
        return signature == other.signature &&
               authorization == other.authorization;
    }
    friend std::ostream& operator<<(std::ostream& os, const Payload& p) {
        os << "{signature: " << p.signature
           << ", authorization: " << p.authorization << "}";
        return os;
    }
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