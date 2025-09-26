#pragma once

#include <string>
#include <optional>
#include "nlohmann/json.hpp"

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
};

// Add these declarations for ADL:
void to_json(json& j, const PaymentRequirements& p);
void from_json(const json& j, PaymentRequirements& p);
