
#include "PaymentRequirements.h"

/**
 * @brief Serializes a PaymentRequirements struct into a nlohmann::json object.
 * Handles 'outputSchema' conditionally: the key is omitted if the optional is empty.
 */
void to_json(json& j, const PaymentRequirements& p) {
    // Mandatory Fields
    j["scheme"] = p.scheme;
    j["network"] = p.network;
    j["maxAmountRequired"] = p.maxAmountRequired;
    j["resource"] = p.resource;
    j["description"] = p.description;
    j["mimeType"] = p.mimeType;
    j["payTo"] = p.payTo;
    j["maxTimeoutSeconds"] = p.maxTimeoutSeconds;
    j["asset"] = p.asset;
    j["extra"] = p.extra; // json type handles null/object automatically

    // Optional Field: outputSchema
    if (p.outputSchema.has_value()) {
        j["outputSchema"] = p.outputSchema.value();
    }
}

// --- Custom JSON Deserialization (from_json) ---
/**
 * @brief Deserializes a nlohmann::json object into a PaymentRequirements struct.
 * Handles 'outputSchema' conditionally: sets the optional to nullopt if the key is missing.
 */
void from_json(const json& j, PaymentRequirements& p)
{
    // Mandatory Fields (using at() for strict access)
    j.at("scheme").get_to(p.scheme);
    j.at("network").get_to(p.network);
    j.at("maxAmountRequired").get_to(p.maxAmountRequired);
    j.at("resource").get_to(p.resource);
    j.at("description").get_to(p.description);
    j.at("mimeType").get_to(p.mimeType);
    j.at("payTo").get_to(p.payTo);
    j.at("maxTimeoutSeconds").get_to(p.maxTimeoutSeconds);
    j.at("asset").get_to(p.asset);
    j.at("extra").get_to(p.extra);

    // Optional Field: outputSchema (using contains() check)
    if (j.contains("outputSchema")) {
        p.outputSchema = j.at("outputSchema").get<json>();
    } else {
        p.outputSchema = std::nullopt;
    }
}