#pragma once

#include <string>
#include "nlohmann/json.hpp"

/**
 * @brief Represents the payment requirements object for an x402 transaction.
 *
 * All string fields use std::string, and the timeout is an integer.
 * Note: maxAmountRequired is stored as a string as it represents a
 * large, unit-less integer (smallest token denomination) which could
 * exceed standard 64-bit integer limits.
 */

struct PaymentRequirements {
    std::string scheme;
    std::string network;
    std::string maxAmountRequired;
    std::string resource;
    std::string description;
    std::string mimeType;
    std::string payTo;
    int maxTimeoutSeconds; // Could also be uint32_t or similar, but int is sufficient for typical values
    std::string asset;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PaymentRequirements,
    scheme,
    network,
    maxAmountRequired,
    resource,
    description,
    mimeType,
    payTo,
    maxTimeoutSeconds,
    asset
)
