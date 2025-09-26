#pragma once
#include "Authorization.h"
#include "nlohmann/json.hpp"
#include <string>
#include <ostream>
using json = nlohmann::json;

struct Payload {
    std::string signature;
    Authorization authorization;

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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Payload, signature, authorization)

