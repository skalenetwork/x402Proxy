#pragma once

#include <string>
#include <optional>
#include "nlohmann/json.hpp"
#include <ostream>

/**
 * @brief Represents the payment requirements object for an x402 transaction.
 *
 * All string fields use std::string, and the timeout is an integer.
 * Note: maxAmountRequired is stored as a string as it represents a
 * large, unit-less integer (smallest token denomination) which could
 * exceed standard 64-bit integer limits.
 */

/**
 * @brief Represents the full x402 Payment Requirements object,
 * including optional fields like 'outputSchema' and 'extra'.
 */

using json = nlohmann::json;

struct PaymentRequirements {
    std::string scheme;
    std::string network;
    std::string maxAmountRequired; // uint256 as string
    std::string resource;
    std::string description;
    std::string mimeType;
    std::optional<json> outputSchema; // Now optional
    std::string payTo;
    int maxTimeoutSeconds;         // number (seconds)
    std::string asset;
    json extra;                    // New: object | null

    bool operator==(const PaymentRequirements& other) const {
        return scheme == other.scheme &&
               network == other.network &&
               maxAmountRequired == other.maxAmountRequired &&
               resource == other.resource &&
               description == other.description &&
               mimeType == other.mimeType &&
               outputSchema == other.outputSchema &&
               payTo == other.payTo &&
               maxTimeoutSeconds == other.maxTimeoutSeconds &&
               asset == other.asset &&
               extra == other.extra;
    }
    friend std::ostream& operator<<(std::ostream& os, const PaymentRequirements& p) {
        os << "{scheme: " << p.scheme
           << ", network: " << p.network
           << ", maxAmountRequired: " << p.maxAmountRequired
           << ", resource: " << p.resource
           << ", description: " << p.description
           << ", mimeType: " << p.mimeType
           << ", outputSchema: " << (p.outputSchema ? p.outputSchema->dump() : "null")
           << ", payTo: " << p.payTo
           << ", maxTimeoutSeconds: " << p.maxTimeoutSeconds
           << ", asset: " << p.asset
           << ", extra: " << p.extra.dump()
           << "}";
        return os;
    }
};

// Add these declarations for ADL:
void to_json(json& j, const PaymentRequirements& p);
void from_json(const json& j, PaymentRequirements& p);
