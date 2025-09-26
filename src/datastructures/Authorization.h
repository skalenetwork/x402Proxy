#pragma once
#include <string>
#include <ostream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

struct Authorization {
    std::string from;
    std::string to;
    std::string value;
    std::string validAfter;
    std::string validBefore;
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Authorization, from, to, value, validAfter, validBefore, nonce)

