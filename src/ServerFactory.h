#pragma once


#include <string>
#include <cstdint>
#include <memory>

namespace proxygen {
    class HTTPServer;
}

constexpr uint32_t DEFAULT_TEST_PORT = 8080;

class ServerFactory {
public:
    static std::shared_ptr<proxygen::HTTPServer> createServerInstance(
        std::string bindIP, uint64_t bindPort);
};
